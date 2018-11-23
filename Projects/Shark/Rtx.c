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

#include "Rtx.h"

#include "Ctx.h"

VOID
NTAPI
CtxSpecialApc(
    __in PKAPC Apc,
    __in PKNORMAL_ROUTINE * NormalRoutine,
    __in PVOID * NormalContext,
    __in PVOID * SystemArgument1,
    __in PVOID * SystemArgument2
)
{
    PKEVENT Notify = NULL;

    Notify = (PKEVENT)Apc->RundownRoutine;

    KeSetEvent(Notify, LOW_PRIORITY, FALSE);

    ExFreePool(Apc);
}

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

    if (KernelMode == Atx->Rtx.Mode) {
        Atx->Rtx.ReturnedStatus = Atx->Rtx.StartRoutine(Atx->Rtx.StartContext);
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

            Atx.Rtx.Platform = Platform;
            Atx.Rtx.StartRoutine = StartRoutine;
            Atx.Rtx.StartContext = StartContext;
            Atx.Rtx.Mode = Mode;

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
                    Status = Atx.Rtx.ReturnedStatus;
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

            Atx.Rtx.StartRoutine = StartRoutine;
            Atx.Rtx.StartContext = StartContext;

            Atx.Rtx.Platform = Platform;

            RemoteDispatcher(&Atx.Apc, NULL, NULL, NULL, NULL);

            Status = Atx.Rtx.ReturnedStatus;
        }

        ObDereferenceObject(Thread);
    }

    return Status;
}

VOID
NTAPI
IpiDispatcher(
    __in PRTX Rtx
)
{
    if (-1 == Rtx->Processor) {
        _IpiDispatcher(
            Rtx->ApcRoutine,
            Rtx->SystemRoutine,
            Rtx->StartRoutine,
            Rtx->StartContext);
    }
    else {
        if (KeGetCurrentProcessorNumber() == Rtx->Processor) {
            Rtx->Result = _IpiDispatcher(
                Rtx->ApcRoutine,
                Rtx->SystemRoutine,
                Rtx->StartRoutine,
                Rtx->StartContext);
        }
    }
}

ULONG_PTR
NTAPI
IpiSingleCall(
    __in PPS_APC_ROUTINE ApcRoutine,
    __in PKSYSTEM_ROUTINE SystemRoutine,
    __in PUSER_THREAD_START_ROUTINE StartRoutine,
    __in PVOID StartContext
)
{
    ULONG_PTR Result = 0;
    RTX Rtx = { 0 };

    Rtx.Processor = KeGetCurrentProcessorNumber();
    Rtx.ApcRoutine = ApcRoutine;
    Rtx.SystemRoutine = SystemRoutine;
    Rtx.StartRoutine = StartRoutine;
    Rtx.StartContext = StartContext;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (ULONG_PTR)&Rtx);

    Result = Rtx.Result;

    return Result;
}

VOID
NTAPI
IpiGenericCall(
    __in PPS_APC_ROUTINE ApcRoutine,
    __in PKSYSTEM_ROUTINE SystemRoutine,
    __in PUSER_THREAD_START_ROUTINE StartRoutine,
    __in PVOID StartContext
)
{
    RTX Rtx = { 0 };

    Rtx.Processor = -1;
    Rtx.ApcRoutine = ApcRoutine;
    Rtx.SystemRoutine = SystemRoutine;
    Rtx.StartRoutine = StartRoutine;
    Rtx.StartContext = StartContext;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (ULONG_PTR)&Rtx);
}
