/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    exinit.c

Abstract:

    The module contains the the initialization code for the executive
    component. It also contains the display string and shutdown system
    services.

--*/

#include "exp.h"
#include <zwapi.h>
#include <inbv.h>
#include "safeboot.h"

#pragma alloc_text(INIT, ExInitPoolLookasidePointers)
#pragma alloc_text(INIT, ExInitSystem)
#pragma alloc_text(INIT, ExpInitSystemPhase0)
#pragma alloc_text(INIT, ExpInitSystemPhase1)
#pragma alloc_text(INIT, ExComputeTickCountMultiplier)
#pragma alloc_text(PAGE, NtShutdownSystem)
#pragma alloc_text(PAGE, ExSystemExceptionFilter)
#pragma alloc_text(INIT, ExInitializeSystemLookasideList)

//
// Tick count multiplier.
//

ULONG ExpTickCountMultiplier;
ULONG ExpHydraEnabled;
static ULONG ExpSuiteMask;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

VOID
ExInitPoolLookasidePointers (
    VOID
    )

/*++

Routine Description:

    This function initializes the PRCB lookaside pointers to temporary
    values that will be updated during phase 1 initialization.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG Index;
    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    //
    // Initialize the paged and nonpaged small pool lookaside list
    // pointers in the PRCB to temporarily point to the global pool
    // lookaside lists. During phase 1 initialization per processor
    // lookaside lists are allocated and the pointer to these lists
    // are established in the PRCB of each processor.
    //

    Prcb = KeGetCurrentPrcb();
    for (Index = 0; Index < POOL_SMALL_LISTS; Index += 1) {
        Lookaside = &ExpSmallNPagedPoolLookasideLists[Index];
        ExInitializeSListHead(&Lookaside->ListHead);
        Prcb->PPNPagedLookasideList[Index].P = Lookaside;
        Prcb->PPNPagedLookasideList[Index].L = Lookaside;

        Lookaside = &ExpSmallPagedPoolLookasideLists[Index];
        ExInitializeSListHead(&Lookaside->ListHead);
        Prcb->PPPagedLookasideList[Index].P = Lookaside;
        Prcb->PPPagedLookasideList[Index].L = Lookaside;
    }

    return;
}

BOOLEAN
ExInitSystem (
    VOID
    )

/*++

Routine Description:

    This function initializes the executive component of the NT system.
    It will perform Phase 0 or Phase 1 initialization as appropriate.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization is success. Otherwise
    a value of FALSE is returned.

--*/

{
    switch ( InitializationPhase ) {

    case 0:
        (void) ExpSystemOK(0, FALSE);
        return ExpInitSystemPhase0();
    case 1:
        (void) ExpSystemOK(1, TRUE);
        return ExpInitSystemPhase1();
    default:
        KeBugCheckEx(UNEXPECTED_INITIALIZATION_CALL, 3, InitializationPhase, 0, 0);
    }
}

BOOLEAN
ExpInitSystemPhase0 (
    VOID
    )

/*++

Routine Description:

    This function performs Phase 0 initialization of the executive component
    of the NT system.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization is success. Otherwise
    a value of FALSE is returned.

--*/

{

    ULONG Index;
    BOOLEAN Initialized = TRUE;
    PGENERAL_LOOKASIDE Lookaside;

    //
    // Initialize Resource objects, currently required during SE
    // Phase 0 initialization.
    //

    if (ExpResourceInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Resource initialization failed\n"));
    }

    //
    // Initialize query/set environment variable synchronization fast
    // mutex.
    //

    ExInitializeFastMutex(&ExpEnvironmentLock);

    //
    // Initialize the key manipulation resource.
    //

    ExInitializeResourceLite(&ExpKeyManipLock);

    //
    // Initialize the paged and nonpaged small pool lookaside structures,
    //

    InitializeListHead(&ExPoolLookasideListHead);
    for (Index = 0; Index < POOL_SMALL_LISTS; Index += 1) {
        Lookaside = &ExpSmallNPagedPoolLookasideLists[Index];
        ExInitializeSystemLookasideList(Lookaside,
                                        NonPagedPool,
                                        (Index + 1) * sizeof (POOL_BLOCK),
                                        'looP',
                                        256,
                                        &ExPoolLookasideListHead);

        Lookaside = &ExpSmallPagedPoolLookasideLists[Index];
        ExInitializeSystemLookasideList(Lookaside,
                                        PagedPool,
                                        (Index + 1) * sizeof (POOL_BLOCK),
                                        'looP',
                                        256,
                                        &ExPoolLookasideListHead);
    }

    //
    // Initialize the nonpaged and paged system lookaside lists.
    //

    InitializeListHead(&ExNPagedLookasideListHead);
    KeInitializeSpinLock(&ExNPagedLookasideLock);
    InitializeListHead(&ExPagedLookasideListHead);
    KeInitializeSpinLock(&ExPagedLookasideLock);

    //
    // Initialize the system paged and nonpaged lookaside list.
    //

    InitializeListHead(&ExSystemLookasideListHead);

    //
    // Initialize the Firmware table provider list
    //
    InitializeListHead(&ExpFirmwareTableProviderListHead);

    //
    // Initialize the FirmwareTableProvider resource
    //
    ExInitializeResourceLite(&ExpFirmwareTableResource);
    
    return ExpSystemOK(2, Initialized);
}

BOOLEAN
ExpInitSystemPhase1 (
    VOID
    )

/*++

Routine Description:

    This function performs Phase 1 initialization of the executive component
    of the NT system.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if the initialization succeeded.  Otherwise
    a value of FALSE is returned.

--*/

{

    ULONG Index;
    BOOLEAN Initialized = TRUE;
    ULONG List;
    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    //
    // Initialize the ATOM package
    //

    RtlInitializeAtomPackage( 'motA' );

    //
    // Initialize pushlocks.
    //

    ExpInitializePushLocks ();

    //
    // Initialize the worker threads.
    //

    if (!NT_SUCCESS (ExpWorkerInitialization())) {
        Initialized = FALSE;
        KdPrint(("Executive: Worker thread initialization failed\n"));
    }

    //
    // Initialize the executive objects.
    //

    if (ExpEventInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Event initialization failed\n"));
    }

    if (ExpEventPairInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Event Pair initialization failed\n"));
    }

    if (ExpMutantInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Mutant initialization failed\n"));
    }
    if (ExpInitializeCallbacks() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Callback initialization failed\n"));
    }

    if (ExpSemaphoreInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Semaphore initialization failed\n"));
    }

    if (ExpTimerInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Timer initialization failed\n"));
    }

    if (ExpProfileInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Profile initialization failed\n"));
    }

    if (ExpUuidInitialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Uuid initialization failed\n"));
    }

    if (!NT_SUCCESS (ExpKeyedEventInitialization ())) {
        Initialized = FALSE;
        KdPrint(("Executive: Keyed event initialization failed\n"));
    }

    if (ExpWin32Initialization() == FALSE) {
        Initialized = FALSE;
        KdPrint(("Executive: Win32 initialization failed\n"));
    }

    //
    // Initialize per processor paged and nonpaged pool lookaside lists.
    //

    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
        Prcb = KiProcessorBlock[Index];

        //
        // Allocate all of the lookaside list structures at once so they will
        // be dense and aligned properly.
        //

        Lookaside =  ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(GENERAL_LOOKASIDE) * POOL_SMALL_LISTS * 2,
                                           'LooP');

        //
        // If the allocation succeeded, then initialize and fill in the new
        // per processor lookaside structures. Otherwise, use the default
        // structures initialized during phase zero.
        //

        if (Lookaside != NULL) {
            for (List = 0; List < POOL_SMALL_LISTS; List += 1) {
                ExInitializeSystemLookasideList(Lookaside,
                                                NonPagedPool,
                                                (List + 1) * sizeof (POOL_BLOCK),
                                                'LooP',
                                                256,
                                                &ExPoolLookasideListHead);
    
                Prcb->PPNPagedLookasideList[List].P = Lookaside;
                Lookaside += 1;
  
                ExInitializeSystemLookasideList(Lookaside,
                                                PagedPool,
                                                (List + 1) * sizeof (POOL_BLOCK),
                                                'LooP',
                                                256,
                                                &ExPoolLookasideListHead);
    
                Prcb->PPPagedLookasideList[List].P = Lookaside;
                Lookaside += 1;
            }
        }
    }

    return Initialized;
}

ULONG
ExComputeTickCountMultiplier (
    IN ULONG TimeIncrement
    )

/*++

Routine Description:

    This routine computes the tick count multiplier that is used to
    compute a tick count value.

Arguments:

    TimeIncrement - Supplies the clock increment value in 100ns units.

Return Value:

    A scaled integer/fraction value is returned as the function result.

--*/

{

    ULONG FractionPart;
    ULONG IntegerPart;
    ULONG Index;
    ULONG Remainder;

    //
    // Compute the integer part of the tick count multiplier.
    //
    // The integer part is the whole number of milliseconds between
    // clock interrupts. It is assumed that this value is always less
    // than 128.
    //

    IntegerPart = TimeIncrement / (10 * 1000);

    //
    // Compute the fraction part of the tick count multiplier.
    //
    // The fraction part is the fraction milliseconds between clock
    // interrupts and is computed to an accuracy of 24 bits.
    //

    Remainder = TimeIncrement - (IntegerPart * (10 * 1000));
    FractionPart = 0;
    for (Index = 0; Index < 24; Index += 1) {
        FractionPart <<= 1;
        Remainder <<= 1;
        if (Remainder >= (10 * 1000)) {
            Remainder -= (10 * 1000);
            FractionPart |= 1;
        }
    }

    //
    // The tick count multiplier is equal to the integer part shifted
    // left by 24 bits and added to the 24 bit fraction.
    //

    return (IntegerPart << 24) | FractionPart;
}

NTSTATUS
NtShutdownSystem (
    __in SHUTDOWN_ACTION Action
    )

/*++

Routine Description:

    This service is used to safely shutdown the system.

    N.B. The caller must have SeShutdownPrivilege to shut down the
        system.

Arguments:

    Action - Supplies an action that is to be taken after having shutdown.

Return Value:

    !NT_SUCCESS - The operation failed or the caller did not have appropriate
        privileges.

--*/

{
    POWER_ACTION        SystemAction;
    NTSTATUS            Status;

    //
    // Convert shutdown action to system action
    //

    switch (Action) {
        case ShutdownNoReboot:  SystemAction = PowerActionShutdown;         break;
        case ShutdownReboot:    SystemAction = PowerActionShutdownReset;    break;
        case ShutdownPowerOff:  SystemAction = PowerActionShutdownOff;      break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

    //
    // Bypass policy manager and pass directly to SetSystemPowerState
    //

    Status = NtSetSystemPowerState (
                SystemAction,
                PowerSystemSleeping3,
                POWER_ACTION_OVERRIDE_APPS | POWER_ACTION_DISABLE_WAKES | POWER_ACTION_CRITICAL
                );

    return Status;
}


int
ExSystemExceptionFilter( VOID )
{
    return( KeGetPreviousMode() != KernelMode ? EXCEPTION_EXECUTE_HANDLER
                                            : EXCEPTION_CONTINUE_SEARCH
          );
}



NTKERNELAPI
KPROCESSOR_MODE
ExGetPreviousMode(
    VOID
    )

/*++

Routine Description:

    Returns previous mode.  This routine is exported from the kernel so
    that drivers can call it, as they may have to do probing of
    embedded pointers to user structures on IOCTL calls that the I/O
    system can't probe for them on the FastIo path, which does not pass
    previous mode via the FastIo parameters.

Arguments:

    None.

Return Value:

    return-value - Either KernelMode or UserMode

--*/

{
    return KeGetPreviousMode();
}

VOID
ExInitializeSystemLookasideList (
    IN PGENERAL_LOOKASIDE Lookaside,
    IN POOL_TYPE Type,
    IN ULONG Size,
    IN ULONG Tag,
    IN USHORT Depth,
    IN PLIST_ENTRY ListHead
    )

/*++

Routine Description:

    This function initializes a system lookaside list structure and inserts
    the structure in the specified lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a nonpaged lookaside list structure.

    Type - Supplies the pool type of the lookaside list.

    Size - Supplies the size of the lookaside list entries.

    Tag - Supplies the pool tag for the lookaside list entries.

    Depth - Supplies the maximum depth of the lookaside list.

    ListHead - Supplies a pointer to the lookaside list into which the
        lookaside list structure is to be inserted.

Return Value:

    None.

--*/

{

    //
    // Initialize pool lookaside list structure and insert the structure
    // in the pool lookaside list.
    //

    ExInitializeSListHead(&Lookaside->ListHead);
    Lookaside->Allocate = &ExAllocatePoolWithTag;
    Lookaside->Free = &ExFreePool;
    Lookaside->Depth = 2;
    Lookaside->MaximumDepth = Depth;
    Lookaside->TotalAllocates = 0;
    Lookaside->AllocateHits = 0;
    Lookaside->TotalFrees = 0;
    Lookaside->FreeHits = 0;
    Lookaside->Type = Type;
    Lookaside->Tag = Tag;
    Lookaside->Size = Size;
    Lookaside->LastTotalAllocates = 0;
    Lookaside->LastAllocateHits = 0;
    InsertTailList(ListHead, &Lookaside->ListEntry);
    return;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

