/**************************************************************************
  A implementation of  shortcut context menu in shell namespace extension.
  Ensure not change the default behavior of the explorer,so do not 
  register default menu(when the user double-clicks,it would be triggered£©.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/26
    Website : http://ilovecpp.com

**************************************************************************/

#pragma once

#include <shlobj.h>
#include <shlwapi.h>
#include "Utils.h"

class CFolderViewImplContextMenu : public IContextMenu
	, public IShellExtInit
	, public IObjectWithSite
{
public:
	CFolderViewImplContextMenu() : _cRef(1), _punkSite(NULL), _pdtobj(NULL)
	{
		DllAddRef();
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		static const QITAB qit[] = {
			QITABENT(CFolderViewImplContextMenu, IContextMenu),
			QITABENT(CFolderViewImplContextMenu, IShellExtInit),
			QITABENT(CFolderViewImplContextMenu, IObjectWithSite),
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
		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}


	// IContextMenu
	IFACEMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
	IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
	IFACEMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pRes, LPSTR pszName, UINT cchMax);

	// IShellExtInit
	IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown *punkSite);
	IFACEMETHODIMP GetSite(REFIID riid, void **ppvSite);

private:
	long    _cRef;
	IDataObject *_pdtobj;
	IUnknown *_punkSite;

	~CFolderViewImplContextMenu()
	{
		// _punkSite should be NULL due to SetSite(NULL).
		if (_pdtobj)
		{
			_pdtobj->Release();
		}
		DllRelease();
	}
};
