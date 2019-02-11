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

#include "Detours.h"
#include "Reload.h"
#include "Scan.h"

PKAPC_STATE
NTAPI
GetApcStateThread(
    __in PKTHREAD Thread
)
{
    return CONTAINING_RECORD(
        (ULONG_PTR)Thread + ReloaderBlock->DebuggerDataBlock.OffsetKThreadApcProcess,
        KAPC_STATE,
        Process);
}
