#include "PathMgr.h"
#include "utils.h"

HRESULT PathMgr::SSHPathToLocalPath(LPCSTR sshpath, LPWSTR localpath, UINT len)
{
    HRESULT hr = S_OK;

    ZeroMemory(localpath, len);

    hr = StringCchCopy(localpath, len, localroot);

    if (SUCCEEDED(hr))
    {
        LPWSTR tmpwstr = UTF8ToUnicode(sshpath);

        hr = StringCchCat(localpath, lstrlen(tmpwstr), tmpwstr);

        CoTaskMemFree(tmpwstr);

    }

    return hr;
}

HRESULT PathMgr::LocalPathToSSHPath(LPCWSTR localpath, LPSTR sshpath, UINT len)
{
    HRESULT hr = S_OK;

    ZeroMemory(sshpath, len);

    hr = StringCchCopyA(sshpath, len, sshroot);

    if (SUCCEEDED(hr))
    {
        LPSTR tmpstr = ::UnicodeToUTF8(localpath);

        hr = StringCchCatA(sshpath, strlen(tmpstr), tmpstr);

        CoTaskMemFree(tmpstr);
    }


    return hr;
}

LPCSTR PathMgr::sshroot    = "/home/eric/";

LPCWSTR PathMgr::localroot = L"D:\\map\\";