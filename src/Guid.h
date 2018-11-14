/**************************************************************************
  GUIDs( CLSID and Property Keys used) are defined in this file.

    Created : baixiangcpp@gmail.com
    Date    : 2018/10/27
    Website : http://ilovecpp.com

**************************************************************************/
#pragma once

#define INITGUID

#include <guiddef.h>
#include <propkeydef.h>

// {3BC89210-463B-4F1C-BAEF-BA17F6C1F282}
DEFINE_GUID(STARFILECLSID,
    0x3bc89210, 0x463b, 0x4f1c, 0xba, 0xef, 0xba, 0x17, 0xf6, 0xc1, 0xf2, 0x82);

// {561CD2E7-3AE4-40B0-A72A-D66C2260C607}
DEFINE_GUID(SHORTCUTMENUCLSID,
    0x561cd2e7, 0x3ae4, 0x40b0, 0xa7, 0x2a, 0xd6, 0x6c, 0x22, 0x60, 0xc6, 0x07);

// Col 2
// {6569520D-FCB8-47EE-ABED-F59DD25D5215}
DEFINE_PROPERTYKEY(SIZEPROPERTYKEY,
    0x6569520d, 0xfcb8, 0x47ee, 0xab, 0xed, 0xf5, 0x9d, 0xd2, 0x5d, 0x52, 0x15, PID_FIRST_USABLE);

// Col 3
// {EAED6CC2-2C0D-4CB4-96BA-E50066E2B9AF}
DEFINE_PROPERTYKEY(TIMEPROPERTYKEY,
    0xeaed6cc2, 0x2c0d, 0x4cb4, 0x96, 0xba, 0xe5, 0x0, 0x66, 0xe2, 0xb9, 0xaf, PID_FIRST_USABLE + 1);

// Col 4
// {0FFE036C-6CDE-4C39-BA1A-596A4E01A84B}
DEFINE_PROPERTYKEY(STATEPROPERTYKEY,
    0xffe036c, 0x6cde, 0x4c39, 0xba, 0x1a, 0x59, 0x6a, 0x4e, 0x1, 0xa8, 0x4b, PID_FIRST_USABLE + 2);

