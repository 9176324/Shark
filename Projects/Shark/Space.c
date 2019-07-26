/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#include "Reload.h"
#include "Rtx.h"

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
    if (FALSE != AllProcesors) {
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
