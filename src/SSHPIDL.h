/**************************************************************************
  This file define the ItemID we used in StarFile.There is no standard
  definition for PIDL.The only requirement is `cb` holds the size of this
  structure.

  ITEMIDLIST should be terminated by a two-byte NULL.

  Windows Explorer assigns a PIDL to your root folder and passes the value
  to your extension during initialization.Your extension is then responsible
  for assigning a properly constructed PIDL to each of its objects and
  providing those PIDLs to Windows Explorer on request.	When the Shell uses
  a PIDL to identify one of your extension's objects,your extension must be
  able to interpret the PIDL and identify the particular object.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/27
    Website : http://ilovecpp.com

(https://docs.microsoft.com/zh-cn/windows/desktop/shell/nse-implement#handling-pidls)
**************************************************************************/

#pragma once

#include <windows.h>

#define PERM_LEN 16
#define MAGICID 0x0825

#pragma pack(1)
typedef struct tagObject
{
    //Because the length of abID is not defined and can vary, 
    //the `cb` member is set to the size of the SSHITEMID structure, in bytes.
    //Even if in this case ,its size is fixed.

    //So this structure can be passed to ILClone,ILNext,ILFirst ...
    USHORT  cb;

    //Magic number ,it used to derermine if the passed pidl defined by us.
    WORD    magicid;

    UINT64    msize;
    BOOL    isfolder;
    UINT    mtime;
    WCHAR   perm[PERM_LEN];
    WCHAR   name[MAX_PATH];
} SSHITEMID;
#pragma pack()



typedef UNALIGNED SSHITEMID *PSSHITEMID;
typedef CONST UNALIGNED SSHITEMID *PCSSHITEMID;

