/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    iopm.c

Abstract:

    This module implements interfaces that support manipulation of i386
    i/o access maps (IOPMs).

    These entry points only exist on i386 machines.

--*/

#include "ki.h"

//
// Our notion of alignment is different, so force use of ours
//

#undef  ALIGN_UP
#undef  ALIGN_DOWN
#define ALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))
#define ALIGN_UP(address,amt) (ALIGN_DOWN( (address + (amt) - 1), (amt) ))

//
// Note on synchronization:
//
//  IOPM edits are always done by code running at DPC level on
//  the processor whose TSS (map) is being edited.
//
//  IOPM only affects user mode code.  User mode code can never interrupt
//  DPC level code, therefore, edits and user code never race.
//


//
// Define a structure to hold the map change info we pass to DPC's
//

typedef struct _MAPINFO {
    PVOID MapSource;
    PKPROCESS Process;
    ULONG MapNumber;
    USHORT MapOffset;
} MAPINFO, *PMAPINFO;

//
// Define forward referenced function prototypes.
//

VOID
KiSetIoMap(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
KiLoadIopmOffset(
    IN PKIPI_CONTEXT SignalDone,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

BOOLEAN
Ke386SetIoAccessMap (
    ULONG MapNumber,
    PKIO_ACCESS_MAP IoAccessMap
    )

/*++

Routine Description:

    The specified i/o access map will be set to match the
    definition specified by IoAccessMap (i.e. enable/disable
    those ports) before the call returns.  The change will take
    effect on all processors.

    Ke386SetIoAccessMap does not give any process enhanced I/O
    access, it merely defines a particular access map.

Arguments:

    MapNumber - Number of access map to set.  Map 0 is fixed.

    IoAccessMap - Pointer to bitvector (64K bits, 8K bytes) which
           defines the specified access map.  Must be in
           non-paged pool.

Return Value:

    TRUE if successful.  FALSE if failure (attempt to set a map
    which does not exist, attempt to set map 0)

--*/

{
    MAPINFO MapInfo;

    //
    // Reject illegal requests
    //

    if ((MapNumber > IOPM_COUNT) || (MapNumber == IO_ACCESS_MAP_NONE)) {
        return FALSE;
    }

    MapInfo.MapSource = IoAccessMap;
    MapInfo.MapNumber = MapNumber;
    MapInfo.Process   = KeGetCurrentThread()->ApcState.Process;

    KeGenericCallDpc (KiSetIoMap,
                      &MapInfo);

    return TRUE;
}

VOID
KiSetIoMap(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
/*++

Routine Description:

    copy the specified map into this processor's TSS.
    This procedure runs at IPI level.

Arguments:

    Dpc - DPC used to initiate this call
    DeferredContext - Context
    SystemArgument1 - System context, Used to signal completion of this call
    SystemArgument2 - System context

Return Value:

    none

--*/

{

    PKPROCESS CurrentProcess;
    PKPCR Pcr;
    PKPRCB Prcb;
    PVOID pt;
    PMAPINFO MapInfo;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

    MapInfo = DeferredContext;

    //
    // Copy the IOPM map and load the map for the current process.
    // We only do this if the current process is running on this processor.
    //

    Pcr = KiPcr ();
    Prcb = Pcr->Prcb;
    CurrentProcess = Prcb->CurrentThread->ApcState.Process;

    pt = &(Pcr->TSS->IoMaps[MapInfo->MapNumber-1].IoMap);
    RtlCopyMemory (pt, MapInfo->MapSource, IOPM_SIZE);
    Pcr->TSS->IoMapBase = CurrentProcess->IopmOffset;

    //
    // Signal that all processing has been done
    //

    KeSignalCallDpcDone (SystemArgument1);

    return;
}

BOOLEAN
Ke386QueryIoAccessMap (
    ULONG MapNumber,
    PKIO_ACCESS_MAP IoAccessMap
    )

/*++

Routine Description:

    The specified i/o access map will be dumped into the buffer.
    map 0 is a constant, but will be dumped anyway.

Arguments:

    MapNumber - Number of access map to set.  map 0 is fixed.

    IoAccessMap - Pointer to buffer (64K bits, 8K bytes) which
           is to receive the definition of the access map.
           Must be in non-paged pool.

Return Value:

    TRUE if successful.  FALSE if failure (attempt to query a map
    which does not exist)

--*/

{

    ULONG i;
    PVOID Map;
    KIRQL OldIrql;
    PUCHAR p;

    //
    // Reject illegal requests
    //

    if (MapNumber > IOPM_COUNT) {
        return FALSE;
    }


    //
    // Copy out the map
    //

    if (MapNumber == IO_ACCESS_MAP_NONE) {

        //
        // no access case, simply return a map of all 1s
        //

        p = (PUCHAR)IoAccessMap;
        for (i = 0; i < IOPM_SIZE; i++) {
            p[i] = (UCHAR)-1;
        }

    } else {

        //
        // Raise to DISPATCH_LEVEL to obtain read access to the structure
        //

        KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

        //
        // normal case, just copy the bits
        //

        Map = (PVOID)&(KiPcr ()->TSS->IoMaps[MapNumber-1].IoMap);
        RtlCopyMemory ((PVOID)IoAccessMap, Map, IOPM_SIZE);

        //
        // Restore IRQL.
        //

        KeLowerIrql (OldIrql);
    }

    return TRUE;
}

BOOLEAN
Ke386IoSetAccessProcess (
    PKPROCESS Process,
    ULONG MapNumber
    )
/*++

Routine Description:

    Set the i/o access map which controls user mode i/o access
    for a particular process.

Arguments:

    Process - Pointer to kernel process object describing the
    process which for which a map is to be set.

    MapNumber - Number of the map to set.  Value of map is
    defined by Ke386IoSetAccessProcess.  Setting MapNumber
    to IO_ACCESS_MAP_NONE will disallow any user mode i/o
    access from the process.

Return Value:

    TRUE if success, FALSE if failure (illegal MapNumber)

--*/

{
    MAPINFO MapInfo;
    USHORT MapOffset;

    //
    // Reject illegal requests
    //

    if (MapNumber > IOPM_COUNT) {
        return FALSE;
    }

    MapOffset = KiComputeIopmOffset (MapNumber);

    //
    // Do the update on all processors at DISPATCH_LEVEL
    //

    MapInfo.Process   = Process;
    MapInfo.MapOffset = MapOffset;

    KeGenericCallDpc (KiLoadIopmOffset,
                      &MapInfo);

    return TRUE;
}

VOID
KiLoadIopmOffset(
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    Edit IopmBase of Tss to match that of currently running process.

Arguments:

    Dpc - DPC used to initiate this call
    DeferredContext - Context
    SystemArgument1 - System context, Used to signal completion of this call
    SystemArgument2 - System context

Return Value:

    none

--*/

{
    PKPCR Pcr;
    PKPRCB Prcb;
    PKPROCESS CurrentProcess;
    PMAPINFO MapInfo;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument2);

    //
    // Update IOPM field in TSS from current process
    //

    MapInfo = DeferredContext;

    Pcr = KiPcr ();
    Prcb = Pcr->Prcb;
    CurrentProcess = Prcb->CurrentThread->ApcState.Process;

    //
    // Set the process IOPM offset first so its available to all.
    // Any context swaps after this point will pick up the new value
    // This store may occur multiple times but that doesn't matter
    //
    MapInfo->Process->IopmOffset = MapInfo->MapOffset;

    Pcr->TSS->IoMapBase = CurrentProcess->IopmOffset;

    //
    // Signal that all processing has been done
    //

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

VOID
Ke386SetIOPL(
    VOID
    )

/*++

Routine Description:

    Gives IOPL to the specified process.

    All threads created from this point on will get IOPL.  The current
    process will get IOPL.  Must be called from context of thread and
    process that are to have IOPL.

    Iopl (to be made a boolean) in KPROCESS says all
    new threads to get IOPL.

    Iopl (to be made a boolean) in KTHREAD says given
    thread to get IOPL.

    N.B.    If a kernel mode only thread calls this procedure, the
            result is (a) pointless and (b) will break the system.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKTHREAD    Thread;
    PKPROCESS   Process2;
    PKTRAP_FRAME    TrapFrame;
    CONTEXT     Context;

    //
    // get current thread and Process2, set flag for IOPL in both of them
    //

    Thread = KeGetCurrentThread();
    Process2 = Thread->ApcState.Process;

    Process2->Iopl = 1;
    Thread->Iopl = 1;

    //
    // Force IOPL to be on for current thread
    //

    TrapFrame = (PKTRAP_FRAME)((PUCHAR)Thread->InitialStack -
                ALIGN_UP(sizeof(KTRAP_FRAME),KTRAP_FRAME_ALIGN) -
                sizeof(FX_SAVE_AREA));

    Context.ContextFlags = CONTEXT_CONTROL;
    KeContextFromKframes(TrapFrame,
                         NULL,
                         &Context);

    Context.EFlags |= (EFLAGS_IOPL_MASK & -1);  // IOPL == 3

    KeContextToKframes(TrapFrame,
                       NULL,
                       &Context,
                       CONTEXT_CONTROL,
                       UserMode);

    return;
}

