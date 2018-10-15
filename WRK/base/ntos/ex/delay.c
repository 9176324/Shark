/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    delay.c

Abstract:

   This module implements the executive delay execution system service.

--*/

#include "exp.h"

#pragma alloc_text(PAGE, NtDelayExecution)

NTSTATUS
NtDelayExecution (
    __in BOOLEAN Alertable,
    __in PLARGE_INTEGER DelayInterval
    )

/*++

Routine Description:

    This function delays the execution of the current thread for the specified
    interval of time.

Arguments:

    Alertable - Supplies a boolean value that specifies whether the delay
        is alertable.

    DelayInterval - Supplies the absolute of relative time over which the
        delay is to occur.

Return Value:

    NTSTATUS.

--*/

{

    LARGE_INTEGER Interval;
    KPROCESSOR_MODE PreviousMode;

    //
    // Get previous processor mode and probe delay interval address if
    // necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForReadSmallStructure(DelayInterval, sizeof(LARGE_INTEGER), sizeof(ULONG));
            Interval = *DelayInterval;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {
        Interval = *DelayInterval;
    }

    //
    // Delay execution for the specified amount of time.
    //

    return KeDelayExecutionThread(PreviousMode, Alertable, &Interval);
}

