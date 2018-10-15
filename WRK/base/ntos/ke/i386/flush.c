/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    flush.c

Abstract:

    This module implements i386 machine dependent kernel functions to flush
    the data and instruction caches and to stall processor execution.

--*/

#include "ki.h"

//
// Prototypes
//

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

extern ULONG KeI386CpuType;

BOOLEAN
KeInvalidateAllCaches (
    VOID
    )

/*++

Routine Description:

    This function writes back and invalidates the cache on all processors
    in the host configuration.

Arguments:

    None.

Return Value:

    TRUE if the invalidation was done, FALSE otherwise.

--*/

{

#ifndef NT_UP

    KIRQL OldIrql;
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

#endif

    //
    // Support for wbinvd on Pentium based platforms is vendor dependent.
    // Check for family first and support on Pentium Pro based platforms
    // onward.
    //

    if (KeI386CpuType < 6 ) {
        return FALSE;
    }

    //
    // Raise IRQL and compute target set of processors.
    //


#ifndef NT_UP

    //
    // Synchronize with other IPI functions which may stall
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    KeAcquireSpinLockAtDpcLevel (&KiReverseStallIpiLock);

    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;

    //
    // If any target processors are specified, then send writeback
    // invalidate packet to the target set of processors.
    //

    if (TargetProcessors != 0) {
        KiIpiSendSynchronousPacket(Prcb,
                                   TargetProcessors,
                                   KiInvalidateAllCachesTarget,
                                   (PVOID)&Prcb->ReverseStall,
                                   NULL,
                                   NULL);

        KiIpiStallOnPacketTargets(TargetProcessors);
    }

#endif

    //
    // All target processors have written back and invalidated caches and
    // are waiting to proceed. Write back invalidate current cache and
    // then continue the execution of target processors.
    //

    _asm {
        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h
    }

    //
    // Wait until all target processors have finished and completed packet.
    //

#ifndef NT_UP

    if (TargetProcessors != 0) {
        Prcb->ReverseStall += 1;
    }

    //
    // Drop reverse IPI lock and Lower IRQL to its previous value.
    //

    KeReleaseSpinLockFromDpcLevel (&KiReverseStallIpiLock);

    KeLowerIrql(OldIrql);

#endif

    return TRUE;
}

#if !defined(NT_UP)

VOID
KiInvalidateAllCachesTarget (
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Proceed,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for writing back and invalidating the cache.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed.

    Proceed - pointer to flag to synchronize with

  Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER (Parameter2);
    UNREFERENCED_PARAMETER (Parameter3);

    //
    // Write back invalidate current cache
    //

    _asm {
        ;
        ; wbinvd
        ;

        _emit 0Fh
        _emit 09h

    }

    KiIpiSignalPacketDoneAndStall (SignalDone, Proceed);
}

#endif

