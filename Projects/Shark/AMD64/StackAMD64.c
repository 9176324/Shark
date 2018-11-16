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

#include "Stack.h"

DECLSPEC_NOINLINE
ULONG
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in ULONG Count
)
{
    CONTEXT ContextRecord = { 0 };
    PVOID HandlerData = NULL;
    ULONG Index = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    ULONG64 ImageBase = 0;
    ULONG64 EstablisherFrame = 0;

    RtlCaptureContext(&ContextRecord);

    __try {
        while (Index < Count && 0 != ContextRecord.Rip) {
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

                Callers[Index].Establisher = (PVOID)ContextRecord.Rip;
                Callers[Index].EstablisherFrame = (PVOID *)EstablisherFrame;

                Index += 1;
            }
            else {
                break;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        RTL_SOFT_ASSERT(NT_SUCCESS(GetExceptionCode()));

        Index = 0;
    }

    return Index;
}
