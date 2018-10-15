/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    WmiData.c

Abstract:

    Define storage for Guids and common global structures

--*/

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4324)   // alignment sensitive to declspec
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4115)   // named type definition in parentheses

#include <initguid.h>
#include <ntos.h>

//
// Time out constants
//
const LARGE_INTEGER WmiOneSecond = {(ULONG)(-1 * 1000 * 1000 * 10), -1};
const LARGE_INTEGER WmiShortTime = {(ULONG)(-10 * 1000 * 10), -1}; // 10 milliseconds

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
#include <wmistr.h>
#include <wmiguid.h>
#define _WMIKM_
#include <evntrace.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

