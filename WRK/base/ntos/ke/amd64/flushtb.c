/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    flushtb.c

Abstract:

    This module implements machine dependent functions to flush the TB
    for an AMD64 system.

--*/

#include "ki.h"

VOID
KxFlushEntireTb (
    VOID
    )

/*++

Routine Description:

    This function flushes the entire translation buffer (TB) on all
    processors in the host configuration.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KIRQL OldIrql;

#if !defined(NT_UP)

    PKAFFINITY Barrier;
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

#endif

    //
    // Raise IRQL to SYNCH level and set TB flush time stamp busy.
    //
    // Send request to target processors, if any, flush the current TB, and
    // wait for the IPI request barrier.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();

#if !defined(NT_UP)

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;
    KiSetTbFlushTimeStampBusy();
    if (TargetProcessors != 0) {
        Barrier = KiIpiSendRequest(TargetProcessors, 0, 0, IPI_FLUSH_ALL);
        KeFlushCurrentTb();
        KiIpiWaitForRequestBarrier(Barrier);

    } else {
        KeFlushCurrentTb();
    }

#else

    KeFlushCurrentTb();

#endif

    //
    // Clear the TB flush time stamp busy and lower IRQL to its previous value.
    //

    KiClearTbFlushTimeStampBusy();
    KeLowerIrql(OldIrql);
    return;
}

#if !defined(NT_UP)

VOID
KeFlushProcessTb (
    VOID
    )

/*++

Routine Description:

    This function flushes the non-global translation buffer on all processors
    that are currently running threads which are child of the current process.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKAFFINITY Barrier;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

    //
    // Compute the target set of processors, disable context switching,
    // and send the flush entire parameters to the target processors,
    // if any, for execution.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    Process = Prcb->CurrentThread->ApcState.Process;
    TargetProcessors = Process->ActiveProcessors;
    TargetProcessors &= ~Prcb->SetMember;

    //
    // Send request to target processors, if any, flush the current process
    // TB, and wait for the IPI request barrier.
    //

    if (TargetProcessors != 0) {
        Barrier = KiIpiSendRequest(TargetProcessors, 0, 0, IPI_FLUSH_PROCESS);
        KiFlushProcessTb();
        KiIpiWaitForRequestBarrier(Barrier);

    } else {
        KiFlushProcessTb();
    }

    //
    // Lower IRQL to its previous value.
    //

    KeLowerIrql(OldIrql);
    return;
}

VOID
KeFlushMultipleTb (
    IN ULONG Number,
    IN PVOID *Virtual,
    IN BOOLEAN AllProcessors
    )

/*++

Routine Description:

    This function flushes multiple entries from the translation buffer
    on all processors that are currently running threads which are
    children of the current process or flushes a multiple entries from
    the translation buffer on all processors in the host configuration.

Arguments:

    Number - Supplies the number of TB entries to flush.

    Virtual - Supplies a pointer to an array of virtual addresses that
        are within the pages whose translation buffer entries are to be
        flushed.

    AllProcessors - Supplies a boolean value that determines which
        translation buffers are to be flushed.

Return Value:

    The previous contents of the specified page table entry is returned
    as the function value.

--*/

{

    PKAFFINITY Barrier;
    PVOID *End;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

    ASSERT((Number != 0) && (Number <= FLUSH_MULTIPLE_MAXIMUM));

    //
    // Compute target set of processors.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (AllProcessors != FALSE) {
        TargetProcessors = KeActiveProcessors;

    } else {
        Process = Prcb->CurrentThread->ApcState.Process;
        TargetProcessors = Process->ActiveProcessors;
    }

    //
    // Send request to target processors, if any, flush multiple entries in
    // current TB, and wait for the IPI request barrier.
    //

    End = Virtual + Number;
    TargetProcessors &= ~Prcb->SetMember;
    if (TargetProcessors != 0) {
        Barrier = KiIpiSendRequest(TargetProcessors,
                                   (LONG64)Virtual,
                                   Number,
                                   IPI_FLUSH_MULTIPLE);

        do {
            KiFlushSingleTb(*Virtual);
            Virtual += 1;
        } while (Virtual < End);

        KiIpiWaitForRequestBarrier(Barrier);

    } else {
        do {
            KiFlushSingleTb(*Virtual);
            Virtual += 1;
        } while (Virtual < End);
    }

    //
    // Lower IRQL to its previous value.
    //

    KeLowerIrql(OldIrql);
    return;
}

VOID
FASTCALL
KeFlushSingleTb (
    IN PVOID Virtual,
    IN BOOLEAN AllProcessors
    )

/*++

Routine Description:

    This function flushes a single entry from translation buffer (TB)
    on all processors that are currently running threads which are
    children of the current process.

Arguments:

    Virtual - Supplies a virtual address that is within the page whose
        translation buffer entry is to be flushed.

    AllProcessors - Supplies a boolean value that determines which
        translation buffers are to be flushed.

Return Value:

    The previous contents of the specified page table entry is returned
    as the function value.

--*/

{

    PKAFFINITY Barrier;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

    //
    // Compute the target set of processors and send the flush single
    // parameters to the target processors, if any, for execution.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    if (AllProcessors != FALSE) {
        TargetProcessors = KeActiveProcessors;

    } else {
        Process = Prcb->CurrentThread->ApcState.Process;
        TargetProcessors = Process->ActiveProcessors;
    }

    //
    // Send request to target processors, if any, flush the single entry from
    // the current TB, and wait for the IPI request barrier.
    //

    TargetProcessors &= ~Prcb->SetMember;
    if (TargetProcessors != 0) {
        Barrier = KiIpiSendRequest(TargetProcessors, (LONG64)Virtual, 0, IPI_FLUSH_SINGLE);
        KiFlushSingleTb(Virtual);
        KiIpiWaitForRequestBarrier(Barrier);

    } else {
        KiFlushSingleTb(Virtual);
    }

    //
    // Lower IRQL to its previous value.
    //

    KeLowerIrql(OldIrql);
    return;
}

#endif

