/**************************************************************************
  It is a simple calss which used to convert paths between local and 
  remote.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/25
    Website : http://ilovecpp.com

**************************************************************************/


#pragma once

#include <windows.h>
#include <strsafe.h>
#include "utils.h"

class PathMgr
{
public:
    PathMgr() {}

    static HRESULT SSHPathToLocalPath(LPCSTR sshpath, LPWSTR localpath, UINT len);

    static HRESULT LocalPathToSSHPath(LPCWSTR localpath, LPSTR sshpath, UINT len);

    static LPCSTR sshroot;

    static LPCWSTR localroot;

};
