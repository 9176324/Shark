/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    xipi.c

Abstract:

    This module implements portable interprocessor interrupt routines.

--*/

#include "ki.h"

//
// Define forward reference function prototypes.
//

VOID
KiIpiGenericCallTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID BroadcastFunction,
    IN PVOID Context,
    IN PVOID Parameter3
    );

ULONG_PTR
KeIpiGenericCall (
    IN PKIPI_BROADCAST_WORKER BroadcastFunction,
    IN ULONG_PTR Context
    )

/*++

Routine Description:

    This function executes the specified function on every processor in
    the host configuration in a synchronous manner, i.e., the function
    is executed on each target in series with the execution of the source
    processor.

Arguments:

    BroadcastFunction - Supplies the address of function that is executed
        on each of the target processors.

    Context - Supplies the value of the context parameter that is passed
        to each function.

Return Value:

    The value returned by the specified function on the source processor
    is returned as the function value.

--*/

{

    volatile LONG Count;
    KIRQL OldIrql;
    ULONG_PTR Status;

#if !defined(NT_UP)

    KAFFINITY TargetProcessors;

#endif

    //
    // Raise IRQL to synchronization level and acquire the reverse stall spin
    // lock to synchronize with other reverse stall functions.
    //

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < SYNCH_LEVEL) {
        KfRaiseIrql(SYNCH_LEVEL);
    }

#if !defined(NT_UP)

    Count = KeNumberProcessors;
    TargetProcessors = KeActiveProcessors & ~KeGetCurrentPrcb()->SetMember;

#endif

    KeAcquireSpinLockAtDpcLevel(&KiReverseStallIpiLock);

    //
    // Initialize the broadcast packet, compute the set of target processors,
    // and sent the packet to the target processors for execution.
    //

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiIpiGenericCallTarget,
                        (PVOID)(ULONG_PTR)BroadcastFunction,
                        (PVOID)Context,
                        (PVOID)&Count);
    }

    //
    // Wait until all processors have entered the target routine and are
    // waiting.
    //

    while (Count != 1) {
        KeYieldProcessor();
    }

#endif

    //
    // Raise IRQL to IPI_LEVEL, signal all other processors to proceed, and
    // call the specified function on the source processor.
    //

    KfRaiseIrql(IPI_LEVEL);
    Count = 0;
    Status = BroadcastFunction(Context);

    //
    // Wait until all of the target processors have finished capturing the
    // function parameters.
    //

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

    //
    // Release reverse stall spin lock, lower IRQL to its previous level,
    // and return the function execution status.
    //

    KeReleaseSpinLockFromDpcLevel(&KiReverseStallIpiLock);
    KeLowerIrql(OldIrql);
    return Status;
}

#if !defined(NT_UP)

VOID
KiIpiGenericCallTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID BroadcastFunction,
    IN PVOID Context,
    IN PVOID Count
    )

/*++

Routine Description:

    This function is the target jacket function to execute a broadcast
    function on a set of target processors. The broadcast packet address
    is obtained, the specified parameters are captured, the broadcast
    packet address is cleared to signal the source processor to continue,
    and the specified function is executed.

Arguments:

    SignalDone Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    BroadcastFunction - Supplies the address of function that is executed
        on each of the target processors.

    Context - Supplies the value of the context parameter that is passed
        to each function.

    Count - Supplies the address of a down count synchronization variable.

Return Value:

    None

--*/

{

    //
    // Decrement the synchronization count variable and wait for the value
    // to go to zero.
    //

    InterlockedDecrement((volatile LONG *)Count);
    while ((*(volatile LONG *)Count) != 0) {
        
        //
        // Check for any other IPI such as the debugger
        // while we wait.  Note this routine does a YEILD.
        //

        KiPollFreezeExecution();
    }

    //
    // Execute the specified function.
    //

    ((PKIPI_BROADCAST_WORKER)(ULONG_PTR)(BroadcastFunction))((ULONG_PTR)Context);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

#endif

