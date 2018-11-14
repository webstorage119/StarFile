#include "SSHOperation.h"
#include <windows.h>
#include <shlobj.h>
#include <propkey.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shellapi.h>
#include <new>
#include <vector>
#include "SSHShellFolder.h"
#include "SSHIDListEnumer.h"
#include "Utils.h"
#include "Guid.h"
#include "SSHDataObject.h"


HRESULT CreateDataObjectInstance(LPCWSTR path, REFIID riid, void **ppv);

HRESULT CreateSSHShellFolderInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;
    SSHFolderImpl* pFolderViewImplShellFolder = new (std::nothrow) SSHFolderImpl();
    HRESULT hr = pFolderViewImplShellFolder ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pFolderViewImplShellFolder->QueryInterface(riid, ppv);
        pFolderViewImplShellFolder->Release();
    }
    return hr;
}

SSHFolderImpl::SSHFolderImpl() : ref_(1), curpidl_(NULL)
{
    DllAddRef();
}

SSHFolderImpl::~SSHFolderImpl()
{
    CoTaskMemFree(curpidl_);
    DllRelease();
}

HRESULT SSHFolderImpl::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(SSHFolderImpl, IShellFolder),
        QITABENT(SSHFolderImpl, IShellFolder2),
        QITABENT(SSHFolderImpl, IPersist),
        QITABENT(SSHFolderImpl, IPersistFolder),
        QITABENT(SSHFolderImpl, IPersistFolder2),
        QITABENT(SSHFolderImpl, IExplorerPaneVisibility),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG SSHFolderImpl::AddRef()
{
    return InterlockedIncrement(&ref_);
}

ULONG SSHFolderImpl::Release()
{
    long cRef = InterlockedDecrement(&ref_);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT SSHFolderImpl::ParseDisplayName(HWND hwnd, IBindCtx *pbc, PWSTR pszName,
                                                ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;
    return hr;
}

HRESULT SSHFolderImpl::EnumObjects(HWND /* hwnd */, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
	HRESULT hr;

	SSHIDListEnumerImpl *penum = new (std::nothrow) SSHIDListEnumerImpl(grfFlags, this);
	hr = penum ? S_OK : E_OUTOFMEMORY;
	if (SUCCEEDED(hr))
	{
		hr = penum->Initialize();
		if (SUCCEEDED(hr))
		{
			hr = penum->QueryInterface(IID_PPV_ARGS(ppenumIDList));
		}
		penum->Release();
	}


	return hr;
}


HRESULT SSHFolderImpl::BindToObject(PCUIDLIST_RELATIVE pidl,
                                            IBindCtx *pbc, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = _ValidatePidl(pidl);
    if (SUCCEEDED(hr))
    {
        SSHFolderImpl* pCFolderViewImplFolder = new (std::nothrow) SSHFolderImpl();
        hr = pCFolderViewImplFolder ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            PITEMID_CHILD pidlFirst = ILCloneFirst(pidl);
            hr = pidlFirst ? S_OK : E_OUTOFMEMORY;
            if (SUCCEEDED(hr))
            {
                PIDLIST_ABSOLUTE pidlBind = ILCombine(curpidl_, pidlFirst);
                hr = pidlBind ? S_OK : E_OUTOFMEMORY;
                if (SUCCEEDED(hr))
                {
                    hr = pCFolderViewImplFolder->Initialize(pidlBind);
                    if (SUCCEEDED(hr))
                    {
                        PCUIDLIST_RELATIVE pidlNext = ILNext(pidl);
                        if (ILIsEmpty(pidlNext))
                        {
                            // If we're reached the end of the idlist, return the interfaces we support for this item.
                            hr = pCFolderViewImplFolder->QueryInterface(riid, ppv);
                        }
                        else
                        {
                            // Otherwise we delegate to our child folder to let it bind to the next level.
                            hr = pCFolderViewImplFolder->BindToObject(pidlNext, pbc, riid, ppv);
                        }
                    }
                    CoTaskMemFree(pidlBind);
                }
                ILFree(pidlFirst);
            }
            pCFolderViewImplFolder->Release();
        }
    }
    return hr;
}

HRESULT SSHFolderImpl::BindToStorage(PCUIDLIST_RELATIVE pidl,
                                             IBindCtx *pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}


//  Helper function to help compare relative IDs.
HRESULT _ILCompareRelIDs(IShellFolder *psfParent, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2,
                         LPARAM lParam)
{
    HRESULT hr;
    PCUIDLIST_RELATIVE pidlRel1 = ILNext(pidl1);
    PCUIDLIST_RELATIVE pidlRel2 = ILNext(pidl2);

    if (ILIsEmpty(pidlRel1))
    {
        if (ILIsEmpty(pidlRel2))
        {
            hr = ResultFromShort(0);  // Both empty
        }
        else
        {
            hr = ResultFromShort(-1);   // 1 is empty, 2 is not.
        }
    }
    else
    {
        if (ILIsEmpty(pidlRel2))
        {
            hr = ResultFromShort(1);  // 2 is empty, 1 is not
        }
        else
        {
            // pidlRel1 and pidlRel2 point to something, so:
            //  (1) Bind to the next level of the IShellFolder
            //  (2) Call its CompareIDs to let it compare the rest of IDs.
            PIDLIST_RELATIVE pidlNext = ILCloneFirst(pidl1);    // pidl2 would work as well
            hr = pidlNext ? S_OK : E_OUTOFMEMORY;
            if (pidlNext)
            {
                IShellFolder *psfNext;
                hr = psfParent->BindToObject(pidlNext, NULL, IID_PPV_ARGS(&psfNext));
                if (SUCCEEDED(hr))
                {
                    // We do not want to pass the lParam is IShellFolder2 isn't supported.
                    // Although it isn't important for this example it shoud be considered
                    // if you are implementing this for other situations.
                    IShellFolder2 *psf2;
                    if (SUCCEEDED(psfNext->QueryInterface(&psf2)))
                    {
                        psf2->Release();  // We can use the lParam
                    }
                    else
                    {
                        lParam = 0;       // We can't use the lParam
                    }

                    // Also, the column mask will not be relevant and should never be passed.
                    hr = psfNext->CompareIDs((lParam & ~SHCIDS_COLUMNMASK), pidlRel1, pidlRel2);
                    psfNext->Release();
                }
                CoTaskMemFree(pidlNext);
            }
        }
    }
    return hr;
}

//  Called to determine the equivalence and/or sort order of two idlists.
HRESULT SSHFolderImpl::CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2)
{
	HRESULT hr;
	if (lParam & (SHCIDS_CANONICALONLY | SHCIDS_ALLFIELDS))
	{
		// First do a "canonical" comparison, meaning that we compare with the intent to determine item
		// identity as quickly as possible.  The sort order is arbitrary but it must be consistent.
		PWSTR psz1;
		hr = _GetName(pidl1, &psz1);
		if (SUCCEEDED(hr))
		{
			PWSTR psz2;
			hr = _GetName(pidl2, &psz2);
			if (SUCCEEDED(hr))
			{
				hr = ResultFromShort(StrCmp(psz1, psz2));
				CoTaskMemFree(psz2);
			}
			CoTaskMemFree(psz1);
		}

		// If we've been asked to do an all-fields comparison, test for any other fields that
		// may be different in an item that shares the same identity.  For example if the item
		// represents a file, the identity may be just the filename but the other fields contained
		// in the idlist may be file size and file modified date, and those may change over time.
		// In our example let's say that "level" is the data that could be different on the same item.
		if ((ResultFromShort(0) == hr) && (lParam & SHCIDS_ALLFIELDS))
		{
			int cLevel1 = 0, cLevel2 = 0;
			//hr = _GetLevel(pidl1, &cLevel1);
			if (SUCCEEDED(hr))
			{
				//hr = _GetLevel(pidl2, &cLevel2);
				if (SUCCEEDED(hr))
				{
					hr = ResultFromShort(cLevel1 - cLevel2);
				}
			}
		}
	}
	else
	{
		//folder first
		BOOL left = FALSE, right = FALSE;
		hr = _GetFolderness(pidl1, &left);
		if (SUCCEEDED(hr))
		{
			hr = _GetFolderness(pidl2, &right);
			if (SUCCEEDED(hr))
			{
				hr = ResultFromShort(right - left);
			}
		}

		if (SUCCEEDED(hr) && (right != left))
			return hr;



		// Compare child ids by column data (lParam & SHCIDS_COLUMNMASK).
		hr = ResultFromShort(0);
		switch (lParam & SHCIDS_COLUMNMASK)
		{
		case 0: // Column one, Name.
		{
			PWSTR psz1;
			hr = _GetName(pidl1, &psz1);
			if (SUCCEEDED(hr))
			{
				PWSTR psz2;
				hr = _GetName(pidl2, &psz2);
				if (SUCCEEDED(hr))
				{
					hr = ResultFromShort(StrCmp(psz1, psz2));
					CoTaskMemFree(psz2);
				}
				CoTaskMemFree(psz1);
			}

			break;
		}
		case 1: // Column two, Size.
		{
			int nSize1 = 0, nSize2 = 0;
			hr = _GetSize(pidl1, &nSize1);
			if (SUCCEEDED(hr))
			{
				hr = _GetSize(pidl2, &nSize2);
				if (SUCCEEDED(hr))
				{
					hr = ResultFromShort(nSize1 - nSize2);
				}
			}
			break;
		}
		case 2: // Column Three, Sides.
		{
			int nSides1 = 0, nSides2 = 0;
			//hr = _GetSides(pidl1, &nSides1);
			if (SUCCEEDED(hr))
			{
				//  hr = _GetSides(pidl2, &nSides2);
				if (SUCCEEDED(hr))
				{
					hr = ResultFromShort(nSides1 - nSides2);
				}
			}
			break;
		}
		case 3: // Column four, Level.
		{
			int cLevel1 = 0, cLevel2 = 0;
			// hr = _GetLevel(pidl1, &cLevel1);
			if (SUCCEEDED(hr))
			{
				//   hr = _GetLevel(pidl2, &cLevel2);
				if (SUCCEEDED(hr))
				{
					hr = ResultFromShort(cLevel1 - cLevel2);
				}
			}
			break;
		}
		default:
		{
			hr = ResultFromShort(1);
		}
		}
	}

	if (ResultFromShort(0) == hr)
	{
		// Continue on by binding to the next level.
		hr = _ILCompareRelIDs(this, pidl1, pidl2, lParam);
	}
	return hr;
}

//  Called by the Shell to create the View Object and return it.
HRESULT SSHFolderImpl::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_NOINTERFACE;
    if (riid == IID_IShellView)
    {
        //TODO:IShellFolderViewCB
        SFV_CREATE sfv = {0};
        sfv.cbSize = sizeof(SFV_CREATE);
        sfv.psfvcb = NULL;
        sfv.psvOuter = NULL; 

        hr = QueryInterface(IID_IShellFolder, (void**)&sfv.pshf);
        if (SUCCEEDED(hr))
        {
            hr = SHCreateShellFolderView(&sfv, (IShellView**)ppv);

            sfv.pshf->Release();
        }
    }
    else if (riid == IID_ICategoryProvider)
    {
		//TODO:IID_ICategoryProvider
    }
    else if (riid == IID_IContextMenu)
    {
        // This is the background context menu for the folder itself, not the context menu on items within it.
        DEFCONTEXTMENU dcm = { hwnd, NULL, curpidl_, static_cast<IShellFolder2 *>(this), 0, NULL, NULL, 0, NULL };
        hr = SHCreateDefaultContextMenu(&dcm, riid, ppv);
    }
    else if (riid == IID_IExplorerCommandProvider)
    {
		//TODO:IID_IExplorerCommandProvider
    }
	else if (riid == IID_IExtractIcon)
	{
		//TODO:IID_IExtractIcon
	}

    return hr;
}

//  Retrieves the attributes of one or more file objects or subfolders.
HRESULT SSHFolderImpl::GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, ULONG *rgfInOut)
{
    // If SFGAO_FILESYSTEM is returned, GetDisplayNameOf(SHGDN_FORPARSING) on that item MUST return a filesystem path.
	HRESULT hr = S_FALSE;
	BOOL isfolder = FALSE;
	hr = _GetFolderness(apidl[0], &isfolder);
	if (SUCCEEDED(hr))
	{
		DWORD dwAttribs = SFGAO_FILESYSTEM;
		if (isfolder)
		{
			dwAttribs |= SFGAO_FOLDER;
			dwAttribs |= SFGAO_HASSUBFOLDER;
		}
		
        *rgfInOut &= dwAttribs;

	}
    return hr;
}



//  Retrieves an OLE interface that can be used to carry out
//  actions on the specified file objects or folders.
HRESULT SSHFolderImpl::GetUIObjectOf(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
                                             REFIID riid, UINT * /* prgfInOut */, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;

    if (riid == IID_IContextMenu)
    {
        DEFCONTEXTMENU const dcm = { hwnd, NULL, curpidl_, static_cast<IShellFolder2 *>(this),
                               cidl, apidl, NULL, 0, NULL };
        hr = SHCreateDefaultContextMenu(&dcm, riid, ppv);
    }
    else if (riid == IID_IExtractIcon)
    {
        //TODO:Querty default Extract Icon
    }
    else if (riid == IID_IDataObject)
    {
        PCSSHITEMID abspidl = (PCSSHITEMID)ILCombine((PCIDLIST_ABSOLUTE)curpidl_,(PCUIDLIST_RELATIVE)apidl[0]);
        WCHAR path[200];
        ParseReadablePath((LPCITEMIDLIST)abspidl,path);
        ILFree((PIDLIST_ABSOLUTE)abspidl);

        if (cidl == 1)
        {
            BOOL flag = FALSE;
            _GetFolderness(apidl[0],&flag);
            if(flag)
                return SHCreateDataObject(curpidl_, cidl, apidl, NULL, riid, ppv);
            else
            {
                return  CreateDataObjectInstance(path,riid, ppv);
            }

        }        
    }
    else if (riid == IID_IQueryAssociations)
    {
		//TODO : Query Associations
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    return hr;
}

//  Retrieves the display name for the specified file object or subfolder.
HRESULT SSHFolderImpl::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF shgdnFlags, STRRET *pName)
{
    HRESULT hr = S_OK;
    if (shgdnFlags & SHGDN_FORPARSING)
    {
        WCHAR szDisplayName[MAX_PATH];
        if (shgdnFlags & SHGDN_INFOLDER)
        {
            // This form of the display name needs to be handled by ParseDisplayName.
            hr = _GetName(pidl, szDisplayName, ARRAYSIZE(szDisplayName));
        }
        else
        {
            PWSTR pszThisFolder;
            hr = SHGetNameFromIDList(curpidl_, (shgdnFlags & SHGDN_FORADDRESSBAR) ? SIGDN_DESKTOPABSOLUTEEDITING : SIGDN_DESKTOPABSOLUTEPARSING, &pszThisFolder);
            if (SUCCEEDED(hr))
            {
                StringCchCopy(szDisplayName, ARRAYSIZE(szDisplayName), pszThisFolder);
                StringCchCat(szDisplayName, ARRAYSIZE(szDisplayName), L"\\");

                WCHAR name[MAX_PATH];
                hr = _GetName(pidl, name, ARRAYSIZE(name));
                if (SUCCEEDED(hr))
                {
                    StringCchCat(szDisplayName, ARRAYSIZE(szDisplayName), name);
                }
                CoTaskMemFree(pszThisFolder);
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = StringToStrRet(szDisplayName, pName);
        }
    }
    else
    {
        PWSTR pszName;
        hr = _GetName(pidl, &pszName);
        if (SUCCEEDED(hr))
        {
            hr = StringToStrRet(pszName, pName);
            CoTaskMemFree(pszName);
        }
    }
    return hr;
}

//  Sets the display name of a file object or subfolder, changing
//  the item identifier in the process.
HRESULT SSHFolderImpl::SetNameOf(HWND /* hwnd */, PCUITEMID_CHILD /* pidl */,
                                         PCWSTR /* pszName */,  DWORD /* uFlags */, PITEMID_CHILD *ppidlOut)
{
    HRESULT hr = E_NOTIMPL;
    *ppidlOut = NULL;
    return hr;
}

//  IPersist method
HRESULT SSHFolderImpl::GetClassID(CLSID *pClassID)
{
    *pClassID = STARFILECLSID;
    return S_OK;
}

//  IPersistFolder method
HRESULT SSHFolderImpl::Initialize(PCIDLIST_ABSOLUTE pidl)
{
    curpidl_ = ILCloneFull(pidl);
    return curpidl_ ? S_OK : E_FAIL;
}

//  IShellFolder2 methods
HRESULT SSHFolderImpl::EnumSearches(IEnumExtraSearch **ppEnum)
{
    *ppEnum = NULL;
    return E_NOINTERFACE;
}

//  Retrieves the default sorting and display column (indices from GetDetailsOf).
HRESULT SSHFolderImpl::GetDefaultColumn(DWORD /* dwRes */,
                                                ULONG *pSort,
                                                ULONG *pDisplay)
{
    *pSort = 0;
    *pDisplay = 0;
    return S_OK;
}

//  Retrieves the default state for a specified column.
HRESULT SSHFolderImpl::GetDefaultColumnState(UINT iColumn, SHCOLSTATEF *pcsFlags)
{
    HRESULT hr = (iColumn < 3) ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *pcsFlags = SHCOLSTATE_ONBYDEFAULT | SHCOLSTATE_TYPE_STR;
    }
    return hr;
}

//  Requests the GUID of the default search object for the folder.
HRESULT SSHFolderImpl::GetDefaultSearchGUID(GUID * /* pguid */)
{
    return E_NOTIMPL;
}

//  Helper function for getting the display name for a column.
//  IMPORTANT: If cch is set to 0 the value is returned in the VARIANT.
HRESULT SSHFolderImpl::_GetColumnDisplayName( PCUITEMID_CHILD pidl,
												  CONST PROPERTYKEY* pkey,
												  VARIANT* pv,
												  WCHAR* pszRet,
												  UINT cch )
{
    BOOL isfolder = FALSE;
    HRESULT hr = _GetFolderness(pidl, &isfolder);
    if (SUCCEEDED(hr))
    {
        if (IsEqualPropertyKey(*pkey, PKEY_ItemNameDisplay))
        {
            PWSTR pszName;
            hr = _GetName(pidl, &pszName);
            if (SUCCEEDED(hr))
            {
                if (pv != NULL)
                {
                    pv->vt = VT_BSTR;
                    pv->bstrVal = SysAllocString(pszName);
                    hr = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
                }
                else
                {
                    hr = StringCchCopy(pszRet, cch, pszName);
                }
                CoTaskMemFree(pszName);
            }
        }
        else if ((IsEqualPropertyKey(*pkey, SIZEPROPERTYKEY)) &&
                 (!isfolder))
        {
            int size = 0;
            hr = _GetSize(pidl, &size);
            if (SUCCEEDED(hr))
            {
                WCHAR sizeStr[128] = { 0 };
                hr = ReadableFileSize(size, sizeStr, ARRAYSIZE(sizeStr));
                if (SUCCEEDED(hr))
                {
                    if (pv)
                    {
                        pv->vt = VT_BSTR;
                        pv->bstrVal = SysAllocString(sizeStr);
                        hr = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = StringCchCopy(pszRet, cch, sizeStr);
                    }
                }
            }
        }
        else if (IsEqualPropertyKey(*pkey, TIMEPROPERTYKEY))
        {
            UINT64 time = 0;
            hr = _GetTime(pidl, &time);
            if (SUCCEEDED(hr))
            {
                WCHAR timeStr[128] = { 0 };
                hr = ::TimeToString(timeStr, ARRAYSIZE(timeStr), time);
                if (SUCCEEDED(hr))
                {
                    if (pv)
                    {
                        VariantInit(pv);
                        hr = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = StringCchCopy(pszRet, cch, timeStr);
                    }
                }

            }
        }
        else if (IsEqualPropertyKey(*pkey, STATEPROPERTYKEY))
        {
            PWSTR pszPerm = NULL;
            hr = _GetPerm(pidl, &pszPerm);
            if (SUCCEEDED(hr))
            {
                if (pv)
                {
                    pv->vt = VT_BSTR;
                    pv->bstrVal = SysAllocString(pszPerm);
                    hr = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
                }
                else
                {
                    hr = StringCchCopy(pszRet, cch, pszPerm);
                }
            }
            CoTaskMemFree(pszPerm);
        }
        else
        {
            if (pv)
            {
                VariantInit(pv);
            }

            if (pszRet)
            {
                *pszRet = '\0';
            }
        }
    }
    return hr;
}

//  Retrieves detailed information, identified by a
//  property set ID (FMTID) and property ID (PID),
//  on an item in a Shell folder.
HRESULT SSHFolderImpl::GetDetailsEx(PCUITEMID_CHILD pidl,
                                            const PROPERTYKEY *pkey,
                                            VARIANT *pv)
{
    BOOL pfIsFolder = FALSE;
    HRESULT hr = _GetFolderness(pidl, &pfIsFolder);
    if (SUCCEEDED(hr))
    {
        if (!pfIsFolder && IsEqualPropertyKey(*pkey, PKEY_PropList_PreviewDetails))
        {
            // This proplist indicates what properties are shown in the details pane at the bottom of the explorer browser.
            pv->vt = VT_BSTR;
            pv->bstrVal = SysAllocString(L"prop:Microsoft.SDKSample.AreaSize;Microsoft.SDKSample.NumberOfSides;Microsoft.SDKSample.DirectoryLevel");
            hr = pv->bstrVal ? S_OK : E_OUTOFMEMORY;
        }
        else
        {
            hr = _GetColumnDisplayName(pidl, pkey, pv, NULL, 0);
        }
    }
    return hr;
}

//  Retrieves detailed information, identified by a
//  column index, on an item in a Shell folder.
HRESULT SSHFolderImpl::GetDetailsOf(PCUITEMID_CHILD pidl,
                                            UINT iColumn,
                                            SHELLDETAILS *pDetails)
{
    PROPERTYKEY key;
    HRESULT hr = MapColumnToSCID(iColumn, &key);
    pDetails->cxChar = 24;
    WCHAR szRet[MAX_PATH];

    if (!pidl)
    {
        // No item means we're returning information about the column itself.
        switch (iColumn)
        {
        case 0:
            pDetails->fmt = LVCFMT_LEFT;
            hr = StringCchCopy(szRet, ARRAYSIZE(szRet), L"Name");
            break;
        case 1:
            pDetails->fmt = LVCFMT_LEFT;
            hr = StringCchCopy(szRet, ARRAYSIZE(szRet), L"Permission");
            break;
        case 2:
            pDetails->fmt = LVCFMT_LEFT;
            hr = StringCchCopy(szRet, ARRAYSIZE(szRet), L"Time");
            break;
        case 3:
            pDetails->fmt = LVCFMT_RIGHT;
            hr = StringCchCopy(szRet, ARRAYSIZE(szRet), L"Size");
            break;
        default:
            // GetDetailsOf is called with increasing column indices until failure.
            hr = E_FAIL;
            break;
        }
    }
    else if (SUCCEEDED(hr))
    {
        hr = _GetColumnDisplayName(pidl, &key, NULL, szRet, ARRAYSIZE(szRet));
    }

    if (SUCCEEDED(hr))
    {
        hr = StringToStrRet(szRet, &pDetails->str);
    }

    return hr;
}

//  Converts a column name to the appropriate
//  property set ID (FMTID) and property ID (PID).
HRESULT SSHFolderImpl::MapColumnToSCID(UINT iColumn, PROPERTYKEY *pkey)
{
    // The property keys returned here are used by the categorizer.
    HRESULT hr = S_OK;
    switch (iColumn)
    {
    case 0:
        *pkey = PKEY_ItemNameDisplay;
        break;
    case 1:
        *pkey = STATEPROPERTYKEY;
        break;
    case 2:
        *pkey = TIMEPROPERTYKEY;
        break;
    case 3:
        *pkey = SIZEPROPERTYKEY;
        break;
    default:
        hr = E_FAIL;
        break;
    }
    return hr;
}

//IPersistFolder2 methods
//  Retrieves the PIDLIST_ABSOLUTE for the folder object.
HRESULT SSHFolderImpl::GetCurFolder(PIDLIST_ABSOLUTE *ppidl)
{
    *ppidl = NULL;
    HRESULT hr = curpidl_ ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        *ppidl = ILCloneFull(curpidl_);
        hr = *ppidl ? S_OK : E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT SSHFolderImpl::GetPaneState(REFEXPLORERPANE  ep, EXPLORERPANESTATE *peps)
{
    HRESULT hr = S_OK;
    
    if (ep == EP_Ribbon)
    {
        *peps = EPS_FORCE | EPS_DEFAULT_ON;
    }

    return hr;

}

// Item idlists passed to folder methods are guaranteed to have accessible memory as specified
// by the cbSize in the itemid.  However they may be loaded from a persisted form (for example
// shortcuts on disk) where they could be corrupted.  It is the shell folder's responsibility
// to make sure it's safe against corrupted or malicious itemids.
PCSSHITEMID SSHFolderImpl::_IsValid(PCUIDLIST_RELATIVE pidl)
{
    PCSSHITEMID pidmine = NULL;
    if (pidl)
    {
        pidmine = (PCSSHITEMID)pidl;
        if (!(pidmine->cb && MAGICID == pidmine->magicid ))
        {
            pidmine = NULL;
        }
    }
    return pidmine;
}

HRESULT SSHFolderImpl::_GetPerm(PCUIDLIST_RELATIVE pidl, PWSTR *pszPerm)
{
    PCSSHITEMID pMyObj = _IsValid(pidl);
    HRESULT hr = pMyObj ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *pszPerm = (PWSTR)CoTaskMemAlloc(sizeof(pMyObj->perm));
        hr = *pszPerm ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            StringCchCopy(*pszPerm,sizeof(pMyObj->perm),pMyObj->perm);
        }
    }
    return hr;
}

HRESULT SSHFolderImpl::_GetTime(PCUIDLIST_RELATIVE pidl, UINT64* time)
{
    PCSSHITEMID sshpidl = _IsValid(pidl);
    HRESULT hr = sshpidl ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *time = sshpidl->mtime;
    }
    return hr;
}

HRESULT SSHFolderImpl::_GetName(PCUIDLIST_RELATIVE pidl, PWSTR pszName, INT cchMax)
{
    PCSSHITEMID pMyObj = _IsValid(pidl);
    HRESULT hr = pMyObj ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        // StringCchCopy requires aligned strings, and itemids are not necessarily aligned.
        int i = 0;
        for ( ; i < cchMax; i++)
        {
            pszName[i] = pMyObj->name[i];
            if (0 == pszName[i])
            {
                break;
            }
        }

        // Make sure the string is null-terminated.
        if (i == cchMax)
        {
            pszName[cchMax - 1] = 0;
        }
    }
    return hr;
}

HRESULT SSHFolderImpl::_GetName(PCUIDLIST_RELATIVE pidl, PWSTR *ppsz)
{
    *ppsz = 0;
    PCSSHITEMID pMyObj = _IsValid(pidl);
    HRESULT hr = pMyObj ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *ppsz = (PWSTR)CoTaskMemAlloc(MAX_PATH);
        hr = *ppsz ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = _GetName(pidl, *ppsz, MAX_PATH);
        }
    }
    return hr;
}

HRESULT SSHFolderImpl::_GetSize(PCUIDLIST_RELATIVE pidl, INT *pSize)
{
    PCSSHITEMID pMyObj = _IsValid(pidl);
    HRESULT hr = pMyObj ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *pSize = pMyObj->msize;
    }
    return hr;
}

HRESULT SSHFolderImpl::_GetFolderness(PCUIDLIST_RELATIVE pidl, BOOL *pfIsFolder)
{
    PCSSHITEMID pMyObj = _IsValid(pidl);
    HRESULT hr = pMyObj ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        *pfIsFolder = pMyObj->isfolder;
    }
    return hr;
}

HRESULT SSHFolderImpl::_ValidatePidl(PCUIDLIST_RELATIVE pidl)
{
    PCSSHITEMID pMyObj = _IsValid(pidl);
    return pMyObj ? S_OK : E_INVALIDARG;
}

HRESULT SSHFolderImpl::CreateChildID(std::shared_ptr<LIBSSH::SSHItem> dir, PITEMID_CHILD *ppidl)
{
    // Sizeof an object plus the next cb plus the characters in the string.
    UINT nIDSize = sizeof(SSHITEMID) +
                   sizeof(USHORT);

    // Allocate and zero the memory.
    SSHITEMID *lpMyObj = (SSHITEMID *)CoTaskMemAlloc(nIDSize);

    HRESULT hr = lpMyObj ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        ZeroMemory(lpMyObj, nIDSize);
		lpMyObj->cb = sizeof(SSHITEMID);
        
		lpMyObj->magicid    = MAGICID;
        
        lpMyObj->msize      = dir->filesize;
        
        lpMyObj->isfolder  = (BOOL)dir->isfolder;

        lpMyObj->mtime = dir->mtime;

        hr = StringCchCopy(lpMyObj->perm, PERM_LEN, dir->perm.c_str());
        if (SUCCEEDED(hr))
        {
            hr = StringCchCopy(lpMyObj->name, MAX_PATH, dir->name.c_str());
            if (SUCCEEDED(hr))
            {
                *ppidl = (PITEMID_CHILD)lpMyObj;
            }
        }

    }
    return hr;
}
