/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>

#include "Space.h"

#include "Jump.h"
#include "Reload.h"
#include "Rtx.h"
#include "Scan.h"

#define MM_PTE_NO_EXECUTE ((ULONG64)1 << 63)

ULONG64 ProtectToPteMask[32] = {
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_READONLY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_EXECUTE | MM_PTE_CACHE,
    MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
    MM_PTE_READWRITE | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOPY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
    MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_READONLY | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_READ,
    MM_PTE_NOCACHE | MM_PTE_READWRITE | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_WRITECOPY | MM_PTE_NO_EXECUTE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_READWRITE,
    MM_PTE_NOCACHE | MM_PTE_EXECUTE_WRITECOPY,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_READONLY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_EXECUTE | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_READ | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_READWRITE | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_WRITECOPY | MM_PTE_CACHE | MM_PTE_NO_EXECUTE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_READWRITE | MM_PTE_CACHE,
    MM_PTE_GUARD | MM_PTE_EXECUTE_WRITECOPY | MM_PTE_CACHE,
    MM_PTE_NOACCESS | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_READONLY | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_READ,
    MM_PTE_WRITECOMBINE | MM_PTE_READWRITE | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_WRITECOPY | MM_PTE_NO_EXECUTE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_READWRITE,
    MM_PTE_WRITECOMBINE | MM_PTE_EXECUTE_WRITECOPY
};

VOID
NTAPI
FlushSingleTb(
    __in PVOID VirtualAddress
)
{
    IpiSingleCall(
        (PPS_APC_ROUTINE)NULL,
        (PKSYSTEM_ROUTINE)NULL,
        (PUSER_THREAD_START_ROUTINE)_FlushSingleTb,
        (PVOID)VirtualAddress);
}

VOID
NTAPI
FlushMultipleTb(
    __in PVOID VirtualAddress,
    __in SIZE_T RegionSize,
    __in BOOLEAN AllProcesors
)
{
    if (AllProcesors) {
        IpiGenericCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)_FlushMultipleTb,
            (PUSER_THREAD_START_ROUTINE)VirtualAddress,
            (PVOID)RegionSize);
    }
    else {
        IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)_FlushMultipleTb,
            (PUSER_THREAD_START_ROUTINE)VirtualAddress,
            (PVOID)RegionSize);
    }
}
