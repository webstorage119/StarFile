/**************************************************************************
  This is a shell folder the enumerator.First,it reads the directory from
  sftp server.Then map them to local (by creating empty file on disk.)

  when SFTPSession times out,reconnect it.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/24
    Website : http://ilovecpp.com

**************************************************************************/

#pragma once

#include "SSHOperation.h"
#include <Shlobj.h>
#include <vector>
#include <memory>

class SSHFolderImpl;

class SSHIDListEnumerImpl : public IEnumIDList
{
public:
    SSHIDListEnumerImpl(DWORD grfFlags, SSHFolderImpl *pFolderViewImplShellFolder);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IEnumIDList
    IFACEMETHODIMP Next(ULONG celt, PITEMID_CHILD *rgelt, ULONG *pceltFetched);
    IFACEMETHODIMP Skip(DWORD celt);
    IFACEMETHODIMP Reset();
    IFACEMETHODIMP Clone(IEnumIDList **ppenum);

    HRESULT Initialize();

private:
    HRESULT MapSSHPathToLocalPath(std::string);

    HRESULT CreateLocalFile(std::wstring name,BOOL isfolder = FALSE);

    std::wstring localpath_; //current local mapping path

private:
    ~SSHIDListEnumerImpl();

    long ref_;
    int index_;

    std::vector<std::shared_ptr<LIBSSH::SSHItem>> items_;
    SSHFolderImpl *folder_;
};
