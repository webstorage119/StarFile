#include "SSHShellFolder.h"
#include <strsafe.h>
#include <windows.h>
#include <shlobj.h>
#include <ctime>
#include <sstream>
#include "Utils.h"
#include "shlwapi.h"


HRESULT TimeToString(PWSTR strDateStr, UINT len, INT64 time)
{
    memset(strDateStr, 0, len * sizeof(WCHAR));

    struct tm t;
    localtime_s(&t,&time);
    t.tm_year = t.tm_year + 1900;
    t.tm_mon = t.tm_mon + 1;

    StringCchPrintf(strDateStr, len, L"%04d-%02d-%02d %02d:%02d:%02d",
        t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

    return S_OK;
}

VOID RemoveSurplusZero(LPCWSTR in,LPWSTR out)
{
    UINT len = lstrlen(in),index = 0;
    
    for (INT i = 0; i < len; )
    {
        if (in[i] == L'.')
        {
            if (in[i + 2] == L'0' )
            {
                if (in[i + 1] == L'0')
                {
                    i += 3;
                }
                else
                {
                    out[index++] = in[i++];
                    out[index++] = in[i++];
                    ++i;
                }

                continue;
            }

        }

        out[index++] = in[i++];
    }

}

HRESULT ReadableFileSize(INT64 size, LPWSTR str, UINT len)
{
    HRESULT hr = S_OK;
    PCWSTR tail = NULL;
    FLOAT readbleSize = 0;
    WCHAR tmp[MAX_PATH] = { 0 };

    if (size <= 1024) {
        readbleSize = size;
        tail = L"B";
    }
    else if (size > 1024 && size <= 1024 * 1024)
    {
        readbleSize = (DOUBLE)size / 1024;
        tail = L"KB";
    }
    else if (size > 1024 * 1024 && size <= 1024 * 1024 * 1024)
    {
        readbleSize = ((DOUBLE)size / 1024) / 1024;
        tail = L"MB";
    }
    else if (size > 1024 * 1024 * 1024 && size <= (INT64)1024 * 1024 * 1024 * 1024)
    {
        readbleSize = ((DOUBLE)size / 1024 / 1024) / 1024;
        tail = L"GB";
    }
    else if (size > (INT64)1024 * 1024 * 1024 * 1024)
    {
        readbleSize = ((DOUBLE)size / 1024 / 1024 / 1024) / 1024;
        tail = L"TB";
    }

    hr =  StringCchPrintf(tmp, len, L"%.2f %s", readbleSize, tail);

    RemoveSurplusZero(tmp,str);

    return hr;
}

STDAPI StringToStrRet(PCWSTR pszName, STRRET *pStrRet)
{
    pStrRet->uType = STRRET_WSTR;
    return SHStrDup(pszName, &pStrRet->pOleStr);
}

HRESULT HandleItem(IShellItemArray *psia, HWND hwnd)
{
    // Get the first ShellItem and display its name
    IShellItem *psi;
    HRESULT hr = psia->GetItemAt(0, &psi);
    if (SUCCEEDED(hr))
    {
        PWSTR pszDisplayName;
        hr = psi->GetDisplayName(SIGDN_NORMALDISPLAY, &pszDisplayName);
        if (SUCCEEDED(hr))
        {
            //TODO: shortcut menu here

            CoTaskMemFree(pszDisplayName);
        }
        psi->Release();
    }
    return hr;
}

LPWSTR UTF8ToUnicode(CONST CHAR* str)
{
	INT textlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	INT bytelen = (textlen + 1) * sizeof(WCHAR);
	LPWSTR wstr = (LPWSTR)CoTaskMemAlloc(bytelen);
	ZeroMemory(wstr, bytelen);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, (LPWSTR)wstr, textlen);
	return wstr;
}

LPSTR UnicodeToUTF8(LPCWSTR str)
{
	INT textlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	INT bytelen = (textlen + 1) * sizeof(CHAR);
	LPSTR ustr = (LPSTR)CoTaskMemAlloc(bytelen);
	ZeroMemory(ustr, bytelen);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, ustr, textlen, NULL, NULL);
	return ustr;
}

PCSSHITEMID _IsValid(PCUIDLIST_RELATIVE pidl)
{
	PCSSHITEMID pidmine = NULL;
	if (pidl)
	{
		pidmine = (PCSSHITEMID)pidl;
		if (!(pidmine->cb && MAGICID == pidmine->magicid))
		{
			pidmine = NULL;
		}
	}
	return pidmine;
}

HRESULT ParseReadablePath(LPCITEMIDLIST pidl, LPWSTR path,BOOL reservedSlash)
{
	HRESULT hr = S_FALSE;
	//LPCITEMIDLIST tmppidl = pidl;
	PCSSHITEMID pmyidl = NULL;
	std::wstring p;

	while (!ILIsEmpty(pidl))
	{
		pmyidl = _IsValid(pidl);

		if (!pmyidl)
		{
			pidl = ILNext(pidl);
			continue;
		}

		p += pmyidl->name;
		p += '/';

		pidl = ILNext(pidl);
	}

	if (p.length())
	{
		StringCchCopy(path, reservedSlash ? p.length():p.length()-1 , p.c_str());
	}

	return hr;
}

void SaveFile(LPCWSTR path,std::string content)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    hFile = CreateFile( path,
                        GENERIC_WRITE,
                        NULL,
                        NULL,
                        OPEN_ALWAYS,
                        NULL,
                        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        OutputDebugString(path);
        OutputDebugString(L"Create File failed.\n");
        return;
    }


    DWORD dwWriteSize = 0,offset = 0;
    BOOL bRet;
    do
    {

        bRet = WriteFile(hFile, content.c_str()+ offset, content.length(), &dwWriteSize, NULL);
        offset += dwWriteSize;

        if (!bRet)
        {
            OutputDebugString(L"Write File failed\n");
        }

    } while (offset < content.length());
    
    FlushFileBuffers(hFile);

    CloseHandle(hFile);
}

