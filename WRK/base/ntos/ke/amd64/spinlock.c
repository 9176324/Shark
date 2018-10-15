/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    spinlock.c

Abstract:

    This module implements the platform specific functions for acquiring
    and releasing spin locks.

--*/

#include "ki.h"

VOID
KiAcquireSpinLockCheckForFreeze (
    IN PKSPIN_LOCK SpinLock,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function acquires a spin lock from while at high priority.
    While the lock is not available, a check is made to see if another
    processor has requested this processor to freeze execution.
    
    N.B. This function must be called with IRQL at or above DISPATCH
         level, or with interrupts disabled.

Arguments:

    SpinLock - Supplies a pointer to a spin lock.

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    //
    // Attempt to acquire the queued spin lock.
    //
    // If the previous value of the spinlock is NULL, then the lock has
    // been acquired. Otherwise wait for lock ownership to be granted
    // while checking for a freeze request.
    //

    do {
        if (KxTryToAcquireSpinLock(SpinLock) != FALSE) {
            break;
        }

        do {

            //
            // Check for freeze request while waiting for spin lock to
            // become free.
            //

            KiCheckForFreezeExecution(TrapFrame, ExceptionFrame);
        } while (*(volatile LONG64 *)SpinLock != 0);

    } while (TRUE);

#else

        UNREFERENCED_PARAMETER(SpinLock);
        UNREFERENCED_PARAMETER(TrapFrame);
        UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    return;
}

DECLSPEC_NOINLINE
ULONG64
KxWaitForSpinLockAndAcquire (
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function is called when the first attempt to acquire a spin lock
    fails. A spin loop is executed until the spin lock is free and another
    attempt to acquire is made. If the attempt fails, then another wait
    for the spin lock to become free is initiated.

Arguments:

    SpinLock - Supplies the address of a spin lock.

Return Value:

    The number of wait loops that were executed.

--*/

{

    ULONG64 SpinCount = 0;

#if DBG

    LONG64 Thread = (LONG64)KeGetCurrentThread() + 1;

#endif

    //
    // Wait for spin lock to become free.
    //

    do {
        do {
            KeYieldProcessor();
        } while (*(volatile LONG64 *)SpinLock != 0);

#if DBG

    } while (InterlockedCompareExchange64((LONG64 *)SpinLock, Thread, 0) != 0);

#else

    } while(InterlockedBitTestAndSet64((LONG64 *)SpinLock, 0));

#endif

    return SpinCount;
}

