/**************************************************************************
  StarFile is a windows shell extension which creates a virtual folder
  on My Computer. It allows the user to interact with his SSH server
  through Windows Explorer.

  The most important interface SSHFolderImpl defined in this file.


	Created : baixiangcpp@gmail.com
	Date    : 2018/10/27
	Website : http://ilovecpp.com

**************************************************************************/
#pragma once

//ssh.h must be placed before windows.h ,
//https://stackoverflow.com/questions/21399650/cannot-include-both-files-winsock2-windows-h

#include "SSHOperation.h"
#include <windows.h>
#include <ShlObj.h>
#include <memory>
#include "SSHPIDL.h"


class SSHFolderImpl : public IShellFolder2,
	                  public IPersistFolder2,
                      public IExplorerPaneVisibility
{
public:
	SSHFolderImpl();

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IShellFolder
	IFACEMETHODIMP ParseDisplayName(HWND hwnd, IBindCtx *pbc, PWSTR pszName,
		ULONG *pchEaten, PIDLIST_RELATIVE *ppidl, ULONG *pdwAttributes);

	IFACEMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);

	IFACEMETHODIMP BindToObject(PCUIDLIST_RELATIVE pidl, IBindCtx *pbc, REFIID riid, void **ppv);

	IFACEMETHODIMP BindToStorage(PCUIDLIST_RELATIVE pidl, IBindCtx *pbc, REFIID riid, void **ppv);

	IFACEMETHODIMP CompareIDs(LPARAM lParam, PCUIDLIST_RELATIVE pidl1, PCUIDLIST_RELATIVE pidl2);

	IFACEMETHODIMP CreateViewObject(HWND hwnd, REFIID riid, void **ppv);

	IFACEMETHODIMP GetAttributesOf(UINT cidl, PCUITEMID_CHILD_ARRAY apidl, ULONG *rgfInOut);

	IFACEMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, PCUITEMID_CHILD_ARRAY apidl,
		REFIID riid, UINT* prgfInOut, void **ppv);

	IFACEMETHODIMP GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF shgdnFlags, STRRET *pName);

	IFACEMETHODIMP SetNameOf(HWND hwnd, PCUITEMID_CHILD pidl, PCWSTR pszName, DWORD uFlags, PITEMID_CHILD * ppidlOut);

	// IShellFolder2
	IFACEMETHODIMP GetDefaultSearchGUID(GUID *pGuid);

	IFACEMETHODIMP EnumSearches(IEnumExtraSearch **ppenum);

	IFACEMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay);

	IFACEMETHODIMP GetDefaultColumnState(UINT iColumn, SHCOLSTATEF *pbState);

	IFACEMETHODIMP GetDetailsEx(PCUITEMID_CHILD pidl, CONST PROPERTYKEY *pkey, VARIANT *pv);

	IFACEMETHODIMP GetDetailsOf(PCUITEMID_CHILD pidl, UINT iColumn, SHELLDETAILS *pDetails);

	IFACEMETHODIMP MapColumnToSCID(UINT iColumn, PROPERTYKEY *pkey);

	// IPersist
	IFACEMETHODIMP GetClassID(CLSID *pClassID);

	// IPersistFolder
	IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidl);

	// IPersistFolder2
	IFACEMETHODIMP GetCurFolder(PIDLIST_ABSOLUTE *ppidl);

    //IExplorerPaneVisibility
    HRESULT GetPaneState(REFEXPLORERPANE  ep,EXPLORERPANESTATE *peps);

	// IDList constructor for the enumerator object
	HRESULT CreateChildID(std::shared_ptr<LIBSSH::SSHItem> dir, PITEMID_CHILD *ppidl);

    VOID SetMapPath(std::wstring path) { curmappath_ = path; }

private:
	~SSHFolderImpl();

	HRESULT _GetName(PCUIDLIST_RELATIVE pidl, PWSTR pszName, INT cchMax);

	HRESULT _GetName(PCUIDLIST_RELATIVE pidl, PWSTR *pszName);

    HRESULT _GetPerm(PCUIDLIST_RELATIVE pidl, PWSTR *pszPerm);

	HRESULT _GetSize(PCUIDLIST_RELATIVE pidl, INT* pSize);

    HRESULT _GetTime(PCUIDLIST_RELATIVE pidl, UINT64* time);

	HRESULT _GetFolderness(PCUIDLIST_RELATIVE pidl, BOOL* pfIsFolder);

	HRESULT _ValidatePidl(PCUIDLIST_RELATIVE pidl);

	PCSSHITEMID _IsValid(PCUIDLIST_RELATIVE pidl);

	HRESULT _GetColumnDisplayName( PCUITEMID_CHILD pidl, 
								   CONST PROPERTYKEY* pkey, 
								   VARIANT* pv,
								   WCHAR* pszRet, 
								   UINT cch);

	// current positon this folder in the name space
	// it is initialized in IPersistFolder::Initialize()

	PIDLIST_ABSOLUTE    curpidl_;

    std::wstring        curmappath_;

	LONG                ref_;
};

