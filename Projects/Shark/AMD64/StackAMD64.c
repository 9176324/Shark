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

#include "Stack.h"

#include "Except.h"

DECLSPEC_NOINLINE
u32
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in u32 Count
)
{
    CONTEXT ContextRecord = { 0 };
    ptr HandlerData = NULL;
    u32 Index = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    u64 ImageBase = 0;
    u64 EstablisherFrame = 0;
    u64 Top = 0;
    u64 Bottom = 0;

    RtlCaptureContext(&ContextRecord);
    IoGetStackLimits(&Bottom, &Top);

    while (Index < Count) {
        FunctionEntry = RtlLookupFunctionEntry(
            ContextRecord.Rip,
            &ImageBase,
            NULL);

        if (NULL != FunctionEntry) {
            RtlVirtualUnwind(
                UNW_FLAG_NHANDLER,
                ImageBase,
                ContextRecord.Rip,
                FunctionEntry,
                &ContextRecord,
                &HandlerData,
                &EstablisherFrame,
                NULL);

            if (EstablisherFrame >= Bottom &&
                EstablisherFrame < Top) {
                Callers[Index].Establisher = (ptr)ContextRecord.Rip;
                Callers[Index].EstablisherFrame = (ptr *)EstablisherFrame;
            }
            else {
                break;
            }

            Index += 1;
        }
        else {
            break;
        }
    }

    return Index;
}
