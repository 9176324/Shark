/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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
#include "Detour.h"
#include "Scan.h"

VOID
NTAPI
AsyncDispatcher(
    __in PKAPC Apc,
    __in PKNORMAL_ROUTINE * NormalRoutine,
    __in PVOID * NormalContext,
    __in PVOID * SystemArgument1,
    __in PVOID * SystemArgument2
)
{
    PATX Atx = NULL;

    Atx = CONTAINING_RECORD(Apc, ATX, Apc);

    Atx->Rtx.Routines.Result = MultipleDispatcher(
        Atx->Rtx.Routines.ApcRoutine,
        Atx->Rtx.Routines.SystemRoutine,
        Atx->Rtx.Routines.StartRoutine,
        Atx->Rtx.Routines.StartContext);

    KeSetEvent(&Atx->Rtx.Notify, LOW_PRIORITY, FALSE);
}

NTSTATUS
NTAPI
AsyncCall(
    __in HANDLE UniqueThread,
    __in_opt PPS_APC_ROUTINE ApcRoutine,
    __in_opt PKSYSTEM_ROUTINE SystemRoutine,
    __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
    __in_opt PVOID StartContext
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PETHREAD Thread = NULL;
    ATX Atx = { 0 };
    LARGE_INTEGER Timeout = { 0 };

    Status = PsLookupThreadByThreadId(
        UniqueThread,
        &Thread);

    if (TRACE(Status)) {
        Atx.Rtx.Routines.ApcRoutine = ApcRoutine;
        Atx.Rtx.Routines.SystemRoutine = SystemRoutine;
        Atx.Rtx.Routines.StartRoutine = StartRoutine;
        Atx.Rtx.Routines.StartContext = StartContext;

        KeInitializeEvent(
            &Atx.Rtx.Notify,
            SynchronizationEvent,
            FALSE);

        if ((ULONG_PTR)KeGetCurrentThread() != (ULONG_PTR)Thread) {
            KeInitializeApc(
                &Atx.Apc,
                Thread,
                OriginalApcEnvironment,
                AsyncDispatcher,
                NULL,
                NULL,
                KernelMode,
                NULL);

            Timeout.QuadPart = Int32x32To64(10, -10 * 1000 * 1000);

            if (FALSE != KeInsertQueueApc(
                &Atx.Apc,
                NULL,
                NULL,
                LOW_PRIORITY)) {
                Status = KeWaitForSingleObject(
                    &Atx.Rtx.Notify,
                    Executive,
                    KernelMode,
                    FALSE,
                    &Timeout);

                if (STATUS_SUCCESS == Status) {
                    Status = Atx.Rtx.Routines.Result;
                }
            }
            else {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
        else {
            AsyncDispatcher(&Atx.Apc, NULL, NULL, NULL, NULL);

            Status = Atx.Rtx.Routines.Result;
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
        MultipleDispatcher(
            Rtx->Routines.ApcRoutine,
            Rtx->Routines.SystemRoutine,
            Rtx->Routines.StartRoutine,
            Rtx->Routines.StartContext);
    }
    else {
        if (KeGetCurrentProcessorNumber() == Rtx->Processor) {
            Rtx->Routines.Result = MultipleDispatcher(
                Rtx->Routines.ApcRoutine,
                Rtx->Routines.SystemRoutine,
                Rtx->Routines.StartRoutine,
                Rtx->Routines.StartContext);
        }
    }
}

ULONG_PTR
NTAPI
IpiSingleCall(
    __in_opt PPS_APC_ROUTINE ApcRoutine,
    __in_opt PKSYSTEM_ROUTINE SystemRoutine,
    __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
    __in_opt PVOID StartContext
)
{
    ULONG_PTR Result = 0;
    RTX Rtx = { 0 };

    Rtx.Processor = KeGetCurrentProcessorNumber();

    Rtx.Routines.ApcRoutine = ApcRoutine;
    Rtx.Routines.SystemRoutine = SystemRoutine;
    Rtx.Routines.StartRoutine = StartRoutine;
    Rtx.Routines.StartContext = StartContext;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (ULONG_PTR)&Rtx);

    Result = Rtx.Routines.Result;

    return Result;
}

VOID
NTAPI
IpiGenericCall(
    __in_opt PPS_APC_ROUTINE ApcRoutine,
    __in_opt PKSYSTEM_ROUTINE SystemRoutine,
    __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
    __in_opt PVOID StartContext
)
{
    RTX Rtx = { 0 };

    Rtx.Processor = -1;

    Rtx.Routines.ApcRoutine = ApcRoutine;
    Rtx.Routines.SystemRoutine = SystemRoutine;
    Rtx.Routines.StartRoutine = StartRoutine;
    Rtx.Routines.StartContext = StartContext;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (ULONG_PTR)&Rtx);
}
