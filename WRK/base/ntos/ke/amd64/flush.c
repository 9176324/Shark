/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    flush.c

Abstract:

    This module implements AMD64 machine dependent kernel functions to
    flush the data and instruction caches on all processors.

--*/

#include "ki.h"

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

    TRUE is returned as the function value.

--*/

{


#if !defined(NT_UP)

    PKAFFINITY Barrier;
    KIRQL OldIrql;
    PKPRCB Prcb;
    KAFFINITY TargetProcessors;

    //
    // Raise IRQL to SYNCH level.
    //
    // Send request to target processors, if any, invalidate the current cache,
    // and wait for the IPI request barrier.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    Prcb = KeGetCurrentPrcb();
    TargetProcessors = KeActiveProcessors & ~Prcb->SetMember;
    if (TargetProcessors != 0) {
        Barrier = KiIpiSendRequest(TargetProcessors, 0, 0, IPI_INVALIDATE_ALL);
        WritebackInvalidate();
        KiIpiWaitForRequestBarrier(Barrier);

    } else {
        WritebackInvalidate();
    }

    //
    // Lower IRQL to its previous value.
    //

    KeLowerIrql(OldIrql);

#else

    WritebackInvalidate();

#endif

    return TRUE;
}

