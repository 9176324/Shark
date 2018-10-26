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

#include <Defs.h>

#include "Ctx.h"

VOID
NTAPI
RemoteDispatcher(
    __in PKAPC Apc,
    __in PKNORMAL_ROUTINE * NormalRoutine,
    __in PVOID * NormalContext,
    __in PVOID * SystemArgument1,
    __in PVOID * SystemArgument2
)
{
    PATX Atx = NULL;

    Atx = CONTAINING_RECORD(Apc, ATX, Apc);

    if (KernelMode == Atx->Ctx.Mode) {
        Atx->Ctx.ReturnedStatus = Atx->Ctx.StartRoutine(Atx->Ctx.StartContext);
    }
    else {
    }

    KeSetEvent(&Atx->Notify, LOW_PRIORITY, FALSE);
}

NTSTATUS
NTAPI
RemoteCall(
    __in HANDLE UniqueThread,
    __in USHORT Platform,
    __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
    __in_opt PVOID StartContext,
    __in KPROCESSOR_MODE Mode
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PETHREAD Thread = NULL;
    ATX Atx = { 0 };

    __try {
        Status = PsLookupThreadByThreadId(
            UniqueThread,
            &Thread);

        if (NT_SUCCESS(Status)) {
            if ((ULONG_PTR)KeGetCurrentThread() != (ULONG_PTR)Thread) {
                KeInitializeEvent(
                    &Atx.Notify,
                    SynchronizationEvent,
                    FALSE);

                KeInitializeApc(
                    &Atx.Apc,
                    Thread,
                    OriginalApcEnvironment,
                    RemoteDispatcher,
                    NULL,
                    NULL,
                    KernelMode,
                    NULL);

                Atx.Ctx.Platform = Platform;
                Atx.Ctx.StartRoutine = StartRoutine;
                Atx.Ctx.StartContext = StartContext;
                Atx.Ctx.Mode = Mode;

                if (FALSE != KeInsertQueueApc(
                    &Atx.Apc,
                    NULL,
                    NULL,
                    LOW_PRIORITY)) {
                    Status = KeWaitForSingleObject(
                        &Atx.Notify,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL);

                    if (STATUS_SUCCESS == Status) {
                        Status = Atx.Ctx.ReturnedStatus;
                    }
                }
                else {
                    Status = STATUS_UNSUCCESSFUL;
                }
            }
            else {
                KeInitializeEvent(
                    &Atx.Notify,
                    SynchronizationEvent,
                    FALSE);

                Atx.Ctx.StartRoutine = StartRoutine;
                Atx.Ctx.StartContext = StartContext;

                Atx.Ctx.Platform = Platform;

                RemoteDispatcher(&Atx.Apc, NULL, NULL, NULL, NULL);

                Status = Atx.Ctx.ReturnedStatus;
            }

            ObDereferenceObject(Thread);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();

        RTL_SOFT_ASSERT(NT_SUCCESS(Status));
    }

    return Status;
}
