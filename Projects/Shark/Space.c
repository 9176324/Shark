/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original Code is blindtiger.
*
*/

#include <defs.h>

#include "Space.h"

#include "Reload.h"
#include "Rtx.h"

void
NTAPI
FlushSingleTb(
    __in ptr VirtualAddress
)
{
    IpiSingleCall(
        (PGKERNEL_ROUTINE)NULL,
        (PGSYSTEM_ROUTINE)NULL,
        (PGRUNDOWN_ROUTINE)_FlushSingleTb,
        (PGNORMAL_ROUTINE)VirtualAddress);
}

void
NTAPI
FlushMultipleTb(
    __in ptr VirtualAddress,
    __in u RegionSize,
    __in b AllProcesors
)
{
    if (FALSE != AllProcesors) {
        IpiGenericCall(
            (PGKERNEL_ROUTINE)NULL,
            (PGSYSTEM_ROUTINE)_FlushMultipleTb,
            (PGRUNDOWN_ROUTINE)VirtualAddress,
            (PGNORMAL_ROUTINE)RegionSize);
    }
    else {
        IpiSingleCall(
            (PGKERNEL_ROUTINE)NULL,
            (PGSYSTEM_ROUTINE)_FlushMultipleTb,
            (PGRUNDOWN_ROUTINE)VirtualAddress,
            (PGNORMAL_ROUTINE)RegionSize);
    }
}
