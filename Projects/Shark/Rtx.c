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

#include "Rtx.h"

#include "Ctx.h"
#include "Guard.h"
#include "Scan.h"

void
NTAPI
AsyncDispatcher(
    __in PKAPC Apc,
    __in PKNORMAL_ROUTINE * NormalRoutine,
    __in ptr * NormalContext,
    __in ptr * SystemArgument1,
    __in ptr * SystemArgument2
)
{
    PATX Atx = NULL;

    Atx = CONTAINING_RECORD(Apc, ATX, Apc);

    Atx->Rtx.Routines.Result = GuardCall(
        Atx->Rtx.Routines.KernelRoutine,
        Atx->Rtx.Routines.SystemRoutine,
        Atx->Rtx.Routines.RundownRoutine,
        Atx->Rtx.Routines.NormalRoutine);

    KeSetEvent(&Atx->Rtx.Notify, LOW_PRIORITY, FALSE);
}

status
NTAPI
AsyncCall(
    __in ptr UniqueThread,
    __in_opt PGKERNEL_ROUTINE KernelRoutine,
    __in_opt PGSYSTEM_ROUTINE SystemRoutine,
    __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
    __in_opt PGNORMAL_ROUTINE NormalRoutine
)
{
    status Status = STATUS_SUCCESS;
    PETHREAD Thread = NULL;
    ATX Atx = { 0 };
    LARGE_INTEGER Timeout = { 0 };

    Status = PsLookupThreadByThreadId(
        UniqueThread,
        &Thread);

    if (TRACE(Status)) {
        Atx.Rtx.Routines.KernelRoutine = KernelRoutine;
        Atx.Rtx.Routines.SystemRoutine = SystemRoutine;
        Atx.Rtx.Routines.RundownRoutine = RundownRoutine;
        Atx.Rtx.Routines.NormalRoutine = NormalRoutine;

        KeInitializeEvent(
            &Atx.Rtx.Notify,
            SynchronizationEvent,
            FALSE);

        if ((u)KeGetCurrentThread() != (u)Thread) {
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

void
NTAPI
IpiDispatcher(
    __in PRTX Rtx
)
{
    if (-1 == Rtx->Processor) {
        GuardCall(
            Rtx->Routines.KernelRoutine,
            Rtx->Routines.SystemRoutine,
            Rtx->Routines.RundownRoutine,
            Rtx->Routines.NormalRoutine);
    }
    else {
        if (KeGetCurrentProcessorNumber() == Rtx->Processor) {
            Rtx->Routines.Result = GuardCall(
                Rtx->Routines.KernelRoutine,
                Rtx->Routines.SystemRoutine,
                Rtx->Routines.RundownRoutine,
                Rtx->Routines.NormalRoutine);
        }
    }
}

u
NTAPI
IpiSingleCall(
    __in_opt PGKERNEL_ROUTINE KernelRoutine,
    __in_opt PGSYSTEM_ROUTINE SystemRoutine,
    __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
    __in_opt PGNORMAL_ROUTINE NormalRoutine
)
{
    u Result = 0;
    RTX Rtx = { 0 };

    Rtx.Processor = KeGetCurrentProcessorNumber();

    Rtx.Routines.KernelRoutine = KernelRoutine;
    Rtx.Routines.SystemRoutine = SystemRoutine;
    Rtx.Routines.RundownRoutine = RundownRoutine;
    Rtx.Routines.NormalRoutine = NormalRoutine;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (u)&Rtx);

    Result = Rtx.Routines.Result;

    return Result;
}

void
NTAPI
IpiGenericCall(
    __in_opt PGKERNEL_ROUTINE KernelRoutine,
    __in_opt PGSYSTEM_ROUTINE SystemRoutine,
    __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
    __in_opt PGNORMAL_ROUTINE NormalRoutine
)
{
    RTX Rtx = { 0 };

    Rtx.Processor = -1;

    Rtx.Routines.KernelRoutine = KernelRoutine;
    Rtx.Routines.SystemRoutine = SystemRoutine;
    Rtx.Routines.RundownRoutine = RundownRoutine;
    Rtx.Routines.NormalRoutine = NormalRoutine;

    KeIpiGenericCall(
        (PKIPI_BROADCAST_WORKER)IpiDispatcher,
        (u)&Rtx);
}
