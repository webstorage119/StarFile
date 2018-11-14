#include "SSHIDListEnumer.h"
#include "SSHShellFolder.h"
#include <shlwapi.h>
#include "PathMgr.h"

SSHIDListEnumerImpl::SSHIDListEnumerImpl(DWORD grfFlags, SSHFolderImpl *pFolderViewImplShellFolder) :
	ref_(1), index_(0), folder_(pFolderViewImplShellFolder)
{
	folder_->AddRef();
}

SSHIDListEnumerImpl::~SSHIDListEnumerImpl()
{
	folder_->Release();
}

HRESULT SSHIDListEnumerImpl::MapSSHPathToLocalPath(std::string abspath)
{
    HRESULT hr = S_OK;

    std::string::size_type pos = abspath.find(PathMgr::sshroot);
    if (pos == std::string::npos)
    {
        hr = S_FALSE;
    }
    else
    {
        abspath = abspath.substr(pos + strlen(PathMgr::sshroot));

        LPCWSTR tmpstr = UTF8ToUnicode(abspath.c_str());

        localpath_ = std::wstring(PathMgr::localroot) + tmpstr;

        folder_->SetMapPath(localpath_);

        CoTaskMemFree((LPVOID*)tmpstr);
    }

    return hr;
}

HRESULT SSHIDListEnumerImpl::CreateLocalFile(std::wstring name,BOOL isfolder)
{
    HRESULT hr = S_OK;

    std::wstring filepathstring = localpath_ + name;

    LPCWSTR filepath = filepathstring.c_str();

    if (!PathFileExists(filepath))
    {
        WCHAR ppath[MAX_PATH] = { 0 };
        StringCchCopy(ppath, MAX_PATH, filepath);
        PathRemoveFileSpec(ppath);
        SHCreateDirectoryEx(NULL, ppath, NULL);
    }
    else
    {
        return S_OK;
    }

    if (!isfolder)
    {
        HANDLE hFile = CreateFile( filepath,
                                   GENERIC_WRITE,
                                   NULL,
                                   NULL,
                                   OPEN_ALWAYS,
                                   NULL,
                                   NULL );
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hr = S_FALSE;
            OutputDebugString(L"Create File failed.\n");
        }

        CloseHandle(hFile);
    }
    else
    {
        BOOL ret = CreateDirectory(filepath,NULL);
        if (!ret)
        {
            hr = S_FALSE;
            OutputDebugString(L"Create dir failed.\n");
        }

    }

    return hr;
}

HRESULT SSHIDListEnumerImpl::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] = {
		QITABENT(SSHIDListEnumerImpl, IEnumIDList),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

ULONG SSHIDListEnumerImpl::AddRef()
{
	return InterlockedIncrement(&ref_);
}

ULONG SSHIDListEnumerImpl::Release()
{
	long cRef = InterlockedDecrement(&ref_);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

//TODO: to make a ssh configure dialog
//TODO: reconnect when ssh session timeout
LIBSSH::SFTPSession session("47.96.93.28", 22, "eric", "XXXXXXXXXXXX");

// Initializes the enumerator
HRESULT SSHIDListEnumerImpl::Initialize()
{
	PSSHITEMID pidl = NULL;
	HRESULT hr = folder_->GetCurFolder((PIDLIST_ABSOLUTE*)&pidl);

	if (SUCCEEDED(hr))
	{
		std::string sshpath(PathMgr::sshroot);

		WCHAR p[MAX_PATH] = {0};
		ParseReadablePath((PIDLIST_ABSOLUTE)pidl,p);
        ILFree((PIDLIST_ABSOLUTE)pidl);

		if (lstrlen(p))
		{
			LPSTR pa = ::UnicodeToUTF8(p);
			sshpath += pa;
			CoTaskMemFree(pa);
		}

        MapSSHPathToLocalPath(sshpath+"\\");
		
		items_ = session.ListDirectory(sshpath.c_str());
	}
	

	return S_OK;
}

// Retrieves the specified number of item identifiers in the enumeration sequence and advances the current position
// by the number of items retrieved.
HRESULT SSHIDListEnumerImpl::Next(ULONG celt, PITEMID_CHILD *rgelt, ULONG *pceltFetched)
{
    ULONG celtFetched = 0;
    HRESULT hr = (pceltFetched || celt <= 1) ? S_OK : E_INVALIDARG;

    if (SUCCEEDED(hr))
    {
        ULONG i = 0;
        while (SUCCEEDED(hr) && i < celt && index_ < items_.size())
        {
            hr = CreateLocalFile(items_[index_]->name, items_[index_]->isfolder);

            if (SUCCEEDED(hr))
            {
                hr = folder_->CreateChildID(items_[index_], &rgelt[i]);

                if (SUCCEEDED(hr))
                {
                    celtFetched++;
                    i++;
                }
            }

            ++index_;
        }

    }

    if (pceltFetched)
    {
        *pceltFetched = celtFetched;
    }

    return (celtFetched == celt) ? S_OK : S_FALSE;
}

HRESULT SSHIDListEnumerImpl::Skip(DWORD celt)
{
	index_ += celt;
	return S_OK;
}

HRESULT SSHIDListEnumerImpl::Reset()
{
	index_ = 0;
	return S_OK;
}

HRESULT SSHIDListEnumerImpl::Clone(IEnumIDList **ppenum)
{
	// this method is rarely used and it's acceptable to not implement it.
	*ppenum = NULL;
	return E_NOTIMPL;
}
