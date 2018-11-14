/**************************************************************************
  Some helper functions used by StarFile.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/26
    Website : http://ilovecpp.com

**************************************************************************/

#include <shlobj.h>
#include <string>
#pragma once

void SaveFile(LPCWSTR path, std::string content);

HRESULT TimeToString(PWSTR strDateStr, UINT len, INT64 time);

HRESULT ReadableFileSize(INT64 size, LPWSTR str, UINT len);

HRESULT ParseReadablePath(LPCITEMIDLIST pidl,LPWSTR path, BOOL reservedSlash = TRUE);

LPSTR UnicodeToUTF8(LPCWSTR str);

LPWSTR UTF8ToUnicode(CONST CHAR* str);

STDAPI StringToStrRet(PCWSTR pszName, STRRET *pStrRet);

HRESULT HandleItem(IShellItemArray *psia, HWND hwndParent);

#ifndef ResultFromShort
#define ResultFromShort(i)      MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(i))
#endif

__inline HRESULT ResultFromKnownLastError() { const DWORD err = GetLastError(); return err == ERROR_SUCCESS ? E_FAIL : HRESULT_FROM_WIN32(err); }

extern HINSTANCE g_hInst;

void DllAddRef();

void DllRelease();
