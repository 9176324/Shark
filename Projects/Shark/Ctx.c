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

#include "Ctx.h"

#include "Jump.h"
#include "Scan.h"

PKAPC_STATE
NTAPI
GetApcStateThread(
    __in PKTHREAD Thread
)
{
    PKAPC_STATE ApcState = NULL;
    PULONG_PTR Buffer = NULL;
    ULONG Index = 0;

    static SIZE_T Offset;

    if (0 == Offset) {
        for (Index = 0;
            Index < PAGE_SIZE;
            Index += sizeof(PVOID)) {
            Buffer = (PULONG_PTR)((ULONG_PTR)Thread + Index);

            if ((ULONG_PTR)*Buffer == (ULONG_PTR)PsGetThreadProcess(Thread)) {
                ApcState = CONTAINING_RECORD(
                    (ULONG_PTR)Thread + Index,
                    KAPC_STATE,
                    Process);

                Offset = (ULONG_PTR)ApcState - (ULONG_PTR)Thread;

                break;
            }
        }
    }
    else {
        ApcState = (PKAPC_STATE)((ULONG_PTR)Thread + Offset);
    }

    return ApcState;
}
