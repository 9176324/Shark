/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    tbflush.c

Abstract:

    This module implements machine dependent functions to flush
    the translation buffers in an Intel x86 system.

--*/

#include "ki.h"

VOID
KiFlushTargetEntireTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetProcessTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetMultipleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiFlushTargetSingleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KxFlushEntireTb (
    VOID
    )

/*++

Routine Description:

    This function flushes the entire translation buffer on all processors.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KIRQL OldIrql;

#if !defined(NT_UP)

    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

#endif

    //
    // Compute the target set of processors and send the flush entire
    // parameters to the target processors, if any, for execution.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    KiSetTbFlushTimeStampBusy();

#if !defined(NT_UP)

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;

    //
    // Send packet to target processors.
    //

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetEntireTb,
                        NULL,
                        NULL,
                        NULL);

        IPI_INSTRUMENT_COUNT (Prcb->Number, FlushEntireTb);
    }

#endif

    //
    // Flush TB on current processor.
    //

    KeFlushCurrentTb();

    //
    // Wait until all target processors have finished and complete packet.
    //

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

    //
    // Clear the TB time stamp busy.
    //

    KiClearTbFlushTimeStampBusy();

    //
    // Lower IRQL to previous level.
    //

    KeLowerIrql(OldIrql);
    return;
}

#if !defined(NT_UP)

VOID
KiFlushTargetEntireTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for flushing the entire TB.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    Parameter1 - Parameter3 - Not used.

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

    //
    // Flush the entire TB on the current processor.
    //

    KiIpiSignalPacketDone(SignalDone);
    KeFlushCurrentTb();
    return;
}

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
    // Send packet to target processors.
    //

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetProcessTb,
                        NULL,
                        NULL,
                        NULL);

        IPI_INSTRUMENT_COUNT (Prcb->Number, FlushEntireTb);
    }

    //
    // Flush TB on current processor.
    //

    KiFlushProcessTb();

    //
    // Wait until all target processors have finished and complete packet.
    //

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    //
    // Lower IRQL and unlock as appropriate.
    //

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetProcessTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for flushing the non-global TB.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    Parameter1 - Parameter3 - Not used.

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

    //
    // Flush the non-global TB on the current processor.
    //

    KiIpiSignalPacketDone(SignalDone);
    KiFlushProcessTb();
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

    PVOID *End;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

    ASSERT(Number != 0);

    End = Virtual + Number; 

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

    TargetProcessors &= ~Prcb->SetMember;

    //
    // If any target processors are specified, then send a flush multiple
    // packet to the target set of processors.
    //

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetMultipleTb,
                        NULL,
                        (PVOID)End,
                        (PVOID)Virtual);

        IPI_INSTRUMENT_COUNT (Prcb->Number, FlushMultipleTb);
    }

    //
    // Flush the specified entries from the TB on the current processor.
    //

    do {
        KiFlushSingleTb(*Virtual);
        Virtual += 1;
    } while (Virtual < End);

    //
    // Wait until all target processors have finished and complete packet.
    //

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    //
    // Lower IRQL to its previous level.
    //

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetMultipleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID End,
    IN PVOID Virtual
    )

/*++

Routine Description:

    This is the target function for flushing multiple TB entries.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    Parameter1 - Not used.

    End - Supplies the a pointer to the ending address of the virtual
        address array.

    Virtual - Supplies a pointer to an array of virtual addresses that
        are within the pages whose translation buffer entries are to be
        flushed.

Return Value:

    None.

--*/

{

    PVOID *xEnd;
    PVOID *xVirtual;

    UNREFERENCED_PARAMETER(Parameter1);

    //
    // Flush the specified entries from the TB on the current processor and
    // signal pack done.
    //

    xEnd = (PVOID *)End;
    xVirtual = (PVOID *)Virtual;
    do {
        KiFlushSingleTb(*xVirtual);
        xVirtual += 1;
    } while (xVirtual < xEnd);

    KiIpiSignalPacketDone(SignalDone);
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

    This function flushes a single entry from translation buffer (TB) on all
    processors that are currently running threads which are child of the current
    process or flushes the entire translation buffer on all processors in the
    host configuration.

Arguments:

    Virtual - Supplies a virtual address that is within the page whose
        translation buffer entry is to be flushed.

    AllProcessors - Supplies a boolean value that determines which translation
        buffers are to be flushed.

Return Value:

    Returns the contents of the PtePointer before the new value
    is stored.

--*/

{

    KIRQL OldIrql;
    PKPRCB Prcb;
    PKPROCESS Process;
    KAFFINITY TargetProcessors;

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

    TargetProcessors &= ~Prcb->SetMember;

    //
    // If any target processors are specified, then send a flush single
    // packet to the target set of processors.
    //

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiFlushTargetSingleTb,
                        NULL,
                        (PVOID)Virtual,
                        NULL);

        IPI_INSTRUMENT_COUNT(Prcb->Number, FlushSingleTb);
    }

    //
    // Flush the specified entry from the TB on the current processor.
    //

    KiFlushSingleTb(Virtual);

    //
    // Wait until all target processors have finished and complete packet.
    //

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargets(TargetProcessors);
    }

    //
    // Lower IRQL to its previous level.
    //

    KeLowerIrql(OldIrql);
    return;
}

VOID
KiFlushTargetSingleTb (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID VirtualAddress,
    IN PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for flushing a single TB entry.

Arguments:

    SignalDone Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    Parameter1 - Not used.

    Virtual - Supplies a virtual address that is within the page whose
        translation buffer entry is to be flushed.

    Parameter3 - Not used.

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Parameter1);
    UNREFERENCED_PARAMETER(Parameter3);

    //
    // Flush a single entry from the TB on the current processor.
    //

    KiIpiSignalPacketDone(SignalDone);
    KiFlushSingleTb(VirtualAddress);
}

#endif

