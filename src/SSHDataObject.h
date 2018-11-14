/**************************************************************************
  Shell Data transfer class.Provide data to upper-level applications.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/27
    Website : http://ilovecpp.com

**************************************************************************/
#pragma once

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>
#include <new> 
#include <string>
#include <windows.h>
#include <algorithm>
#include <string>
#include "PathMgr.h"

void DllAddRef();
void DllRelease();

class CDataObject : public IDataObject
{
public:
	CDataObject(LPCWSTR path) : _cRef(1), _pdtobjShell(NULL),path_(path)
	{
		DllAddRef();
        extern LIBSSH::SFTPSession session;

        //download

        LPCSTR rltpath = UnicodeToUTF8(path_.c_str());
        servpath_ = std::string(PathMgr::sshroot) + rltpath;
        CoTaskMemFree((LPVOID*)rltpath);
        

        localpath_ = std::wstring(PathMgr::localroot) + path_;

        std::replace(localpath_.begin(), localpath_.end(), '/', '\\');

        

        //
        IShellFolder* DesktopPtr;

        HRESULT hr = SHGetDesktopFolder(&DesktopPtr);

        if (FAILED(hr))
            return ;

        LPITEMIDLIST pidlLocal = { 0 };
        LPITEMIDLIST abspidl = { 0 };

        hr = DesktopPtr->ParseDisplayName(NULL, NULL, (LPWSTR)localpath_.c_str(), NULL, &abspidl, NULL);
        if (FAILED(hr))
            return ;

        pidlLocal = ILClone(abspidl);
        abspidl = ILFindLastID(abspidl);
        ILRemoveLastID(pidlLocal);

        hr = SHCreateDataObject(pidlLocal, 1, (PCUITEMID_CHILD_ARRAY)&abspidl, NULL, IID_IDataObject, (void**)&p_);


	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		static const QITAB qit[] = {
			QITABENT(CDataObject, IDataObject),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&_cRef);
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		long cRef = InterlockedDecrement(&_cRef);
		if (0 == cRef)
		{
			delete this;
		}
		return cRef;
	}

	// IDataObject
	IFACEMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);

    IFACEMETHODIMP GetDataHere(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */);

	IFACEMETHODIMP QueryGetData(FORMATETC *pformatetc);

    IFACEMETHODIMP GetCanonicalFormatEtc(FORMATETC *pformatetcIn, FORMATETC *pFormatetcOut);

	IFACEMETHODIMP SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease);

	IFACEMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc);

    IFACEMETHODIMP DAdvise(FORMATETC* /* pformatetc */, DWORD /* advf */, IAdviseSink* /* pAdvSnk */, DWORD* /* pdwConnection */);

    IFACEMETHODIMP DUnadvise(DWORD /* dwConnection */);

    IFACEMETHODIMP EnumDAdvise(IEnumSTATDATA** /*ppenumAdvise */);

private:
	~CDataObject()
	{
		::DllRelease();
	}

	HRESULT _EnsureShellDataObject()
	{
		// the shell data object imptlements ::SetData() in a way that will store any format
		// this code delegates to that implementation to avoid having to implement ::SetData()
		return _pdtobjShell ? S_OK : SHCreateDataObject(NULL, 0, NULL, NULL, IID_PPV_ARGS(&_pdtobjShell));
	}

	long _cRef;

    std::wstring path_;

	IDataObject *_pdtobjShell;

    IDataObject* p_;

    std::string servpath_;

    std::wstring localpath_;
};

WCHAR const c_szText[] = L"Clipboard Contents";


