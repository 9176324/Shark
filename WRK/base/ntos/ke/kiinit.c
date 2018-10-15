/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kiinit.c

Abstract:

    This module implements architecture independent kernel initialization.

--*/

#include "ki.h"

//
// External data.
//

extern ALIGNED_SPINLOCK AfdWorkQueueSpinLock;
extern ALIGNED_SPINLOCK CcBcbSpinLock;
extern ALIGNED_SPINLOCK CcMasterSpinLock;
extern ALIGNED_SPINLOCK CcVacbSpinLock;
extern ALIGNED_SPINLOCK CcWorkQueueSpinLock;
extern ALIGNED_SPINLOCK IopCancelSpinLock;
extern ALIGNED_SPINLOCK IopCompletionLock;
extern ALIGNED_SPINLOCK IopDatabaseLock;
extern ALIGNED_SPINLOCK IopVpbSpinLock;
extern ALIGNED_SPINLOCK NtfsStructLock;
extern ALIGNED_SPINLOCK MmPfnLock;
extern ALIGNED_SPINLOCK NonPagedPoolLock;
extern ALIGNED_SPINLOCK MmNonPagedPoolLock;
extern ALIGNED_SPINLOCK MmSystemSpaceLock;

#if defined(KE_MULTINODE)

extern PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode;
extern PHALNUMAPAGETONODE MmPageToNode;

#endif

//
// Put all code for kernel initialization in the INIT section. It will be
// deallocated by memory management when phase 1 initialization is completed.
//

#pragma alloc_text(INIT, KeInitSystem)
#pragma alloc_text(INIT, KiInitSpinLocks)
#pragma alloc_text(INIT, KiInitSystem)
#pragma alloc_text(INIT, KeNumaInitialize)

BOOLEAN
KeInitSystem (
    VOID
    )

/*++

Routine Description:

    This function initializes executive structures implemented by the
    kernel.

    N.B. This function is only called during phase 1 initialization.

Arguments:

    None.

Return Value:

    A value of TRUE is returned if initialization is successful. Otherwise,
    a value of FALSE is returned.

--*/

{

    HANDLE Handle;
    ULONG Index;
    ULONG Limit;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKPRCB Prcb;
    NTSTATUS Status;

    //
    // If threaded DPCs are enabled for the host system, then create a DPC
    // thread for each processor.
    //

    if (KeThreadDpcEnable != FALSE) {
        Index = 0;
        Limit = (ULONG)KeNumberProcessors;
        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL);
        do {
            Prcb = KiProcessorBlock[Index];
            KeInitializeEvent(&Prcb->DpcEvent, SynchronizationEvent, FALSE);
            InitializeListHead(&Prcb->DpcData[DPC_THREADED].DpcListHead);
            KeInitializeSpinLock(&Prcb->DpcData[DPC_THREADED].DpcLock);
            Prcb->DpcData[DPC_THREADED].DpcQueueDepth = 0;
            Status = PsCreateSystemThread(&Handle,
                                          THREAD_ALL_ACCESS,
                                          &ObjectAttributes,
                                          NULL,
                                          NULL,
                                          KiExecuteDpc,
                                          Prcb);

            if (!NT_SUCCESS(Status)) {
                return FALSE;
            }

            ZwClose(Handle);
            Index += 1;
        } while (Index < Limit);
    }

    //
    // Perform platform dependent initialization.
    //

    return KiInitMachineDependent();
}

VOID
KiInitSpinLocks (
    PKPRCB Prcb,
    ULONG Number
    )

/*++

Routine Description:

    This function initializes the spinlock structures in the per processor
    PRCB. This function is called once for each processor.

Arguments:

    Prcb - Supplies a pointer to a PRCB.

    Number - Supplies the number of respective processor.

Return Value:

    None.

--*/

{

    ULONG Index;

    //
    // Initialize dispatcher ready queue list heads, the ready summary, and
    // the deferred ready list head.
    //

    Prcb->QueueIndex = 1;
    Prcb->ReadySummary = 0;
    Prcb->DeferredReadyListHead.Next = NULL;
    for (Index = 0; Index < MAXIMUM_PRIORITY; Index += 1) {
        InitializeListHead(&Prcb->DispatcherReadyListHead[Index]);
    }

    //
    // Initialize the normal DPC data.
    //

    InitializeListHead(&Prcb->DpcData[DPC_NORMAL].DpcListHead);
    KeInitializeSpinLock(&Prcb->DpcData[DPC_NORMAL].DpcLock);
    Prcb->DpcData[DPC_NORMAL].DpcQueueDepth = 0;
    Prcb->DpcData[DPC_NORMAL].DpcCount = 0;
    Prcb->DpcRoutineActive = 0;
    Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
    Prcb->MinimumDpcRate = KiMinimumDpcRate;
    Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

    //
    // Initialize the generic call DPC structure, set the target processor
    // number, and set the DPC importance.
    //

    KeInitializeDpc(&Prcb->CallDpc, NULL, NULL);
    KeSetTargetProcessorDpc(&Prcb->CallDpc, (CCHAR)Number);
    KeSetImportanceDpc(&Prcb->CallDpc, HighImportance);

    //
    // Initialize wait list.
    //

    InitializeListHead(&Prcb->WaitListHead);

    //
    // Initialize general numbered queued spinlock structures.
    //

#if defined(_AMD64_)

    KeGetPcr()->LockArray = &Prcb->LockQueue[0];

#endif

    Prcb->LockQueue[LockQueueDispatcherLock].Next = NULL;
    Prcb->LockQueue[LockQueueDispatcherLock].Lock = &KiDispatcherLock;

    Prcb->LockQueue[LockQueueUnusedSpare1].Next = NULL;
    Prcb->LockQueue[LockQueueUnusedSpare1].Lock = NULL;

    Prcb->LockQueue[LockQueuePfnLock].Next = NULL;
    Prcb->LockQueue[LockQueuePfnLock].Lock = &MmPfnLock;

    Prcb->LockQueue[LockQueueSystemSpaceLock].Next = NULL;
    Prcb->LockQueue[LockQueueSystemSpaceLock].Lock = &MmSystemSpaceLock;

    Prcb->LockQueue[LockQueueBcbLock].Next = NULL;
    Prcb->LockQueue[LockQueueBcbLock].Lock = &CcBcbSpinLock;

    Prcb->LockQueue[LockQueueMasterLock].Next = NULL;
    Prcb->LockQueue[LockQueueMasterLock].Lock = &CcMasterSpinLock;

    Prcb->LockQueue[LockQueueVacbLock].Next = NULL;
    Prcb->LockQueue[LockQueueVacbLock].Lock = &CcVacbSpinLock;

    Prcb->LockQueue[LockQueueWorkQueueLock].Next = NULL;
    Prcb->LockQueue[LockQueueWorkQueueLock].Lock = &CcWorkQueueSpinLock;

    Prcb->LockQueue[LockQueueNonPagedPoolLock].Next = NULL;
    Prcb->LockQueue[LockQueueNonPagedPoolLock].Lock = &NonPagedPoolLock;

    Prcb->LockQueue[LockQueueMmNonPagedPoolLock].Next = NULL;
    Prcb->LockQueue[LockQueueMmNonPagedPoolLock].Lock = &MmNonPagedPoolLock;

    Prcb->LockQueue[LockQueueIoCancelLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoCancelLock].Lock = &IopCancelSpinLock;

    Prcb->LockQueue[LockQueueIoVpbLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoVpbLock].Lock = &IopVpbSpinLock;

    Prcb->LockQueue[LockQueueIoDatabaseLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoDatabaseLock].Lock = &IopDatabaseLock;

    Prcb->LockQueue[LockQueueIoCompletionLock].Next = NULL;
    Prcb->LockQueue[LockQueueIoCompletionLock].Lock = &IopCompletionLock;

    Prcb->LockQueue[LockQueueNtfsStructLock].Next = NULL;
    Prcb->LockQueue[LockQueueNtfsStructLock].Lock = &NtfsStructLock;

    Prcb->LockQueue[LockQueueAfdWorkQueueLock].Next = NULL;
    Prcb->LockQueue[LockQueueAfdWorkQueueLock].Lock = &AfdWorkQueueSpinLock;

    Prcb->LockQueue[LockQueueUnusedSpare16].Next = NULL;
    Prcb->LockQueue[LockQueueUnusedSpare16].Lock = NULL;

    //
    // Initialize the timer table numbered queued spinlock structures.
    //

    for (Index = 0; Index < LOCK_QUEUE_TIMER_TABLE_LOCKS; Index += 1) {
        KeInitializeSpinLock(&KiTimerTableLock[Index].Lock);
        Prcb->LockQueue[LockQueueTimerTableLock + Index].Next = NULL;
        Prcb->LockQueue[LockQueueTimerTableLock + Index].Lock = &KiTimerTableLock[Index].Lock;
    }

    //
    // Initialize processor control block lock.
    //

    KeInitializeSpinLock(&Prcb->PrcbLock);

    //
    // If this is processor zero, then also initialize the queued spin lock
    // home address.
    //

    if (Number == 0) {
        KeInitializeSpinLock(&KiDispatcherLock);
        KeInitializeSpinLock(&KiReverseStallIpiLock);
        KeInitializeSpinLock(&MmPfnLock);
        KeInitializeSpinLock(&MmSystemSpaceLock);
        KeInitializeSpinLock(&CcBcbSpinLock);
        KeInitializeSpinLock(&CcMasterSpinLock);
        KeInitializeSpinLock(&CcVacbSpinLock);
        KeInitializeSpinLock(&CcWorkQueueSpinLock);
        KeInitializeSpinLock(&IopCancelSpinLock);
        KeInitializeSpinLock(&IopCompletionLock);
        KeInitializeSpinLock(&IopDatabaseLock);
        KeInitializeSpinLock(&IopVpbSpinLock);
        KeInitializeSpinLock(&NonPagedPoolLock);
        KeInitializeSpinLock(&MmNonPagedPoolLock);
        KeInitializeSpinLock(&NtfsStructLock);
        KeInitializeSpinLock(&AfdWorkQueueSpinLock);
    }

    return;
}

VOID
KiInitSystem (
    VOID
    )

/*++

Routine Description:

    This function initializes architecture independent kernel structures.

    N.B. This function is only called on processor 0.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG Index;

    //
    // Initialize bugcheck callback listhead and spinlock.
    //

    InitializeListHead(&KeBugCheckCallbackListHead);
    InitializeListHead(&KeBugCheckReasonCallbackListHead);
    KeInitializeSpinLock(&KeBugCheckCallbackLock);

    //
    // Initialize the timer expiration DPC object and set the destination
    // processor to processor zero.
    //

    KeInitializeDpc(&KiTimerExpireDpc, KiTimerExpiration, NULL);
    KeSetTargetProcessorDpc(&KiTimerExpireDpc, 0);

    //
    // Initialize the profile listhead and profile locks
    //

    KeInitializeSpinLock(&KiProfileLock);
    InitializeListHead(&KiProfileListHead);

    //
    // Initialize the active profile source listhead.
    //

    InitializeListHead(&KiProfileSourceListHead);

    //
    // Initialize the timer table and the timer table due time table.
    //
    // N.B. Each entry in the timer table due time table is set to an infinite
    //      absolute due time.
    //

    for (Index = 0; Index < TIMER_TABLE_SIZE; Index += 1) {
        InitializeListHead(&KiTimerTableListHead[Index].Entry);
        KiTimerTableListHead[Index].Time.HighPart = 0xffffffff;
        KiTimerTableListHead[Index].Time.LowPart = 0;
    }

    //
    // Initialize the swap event, the process inswap listhead, the
    // process outswap listhead, and the kernel stack inswap listhead.
    //

    KeInitializeEvent(&KiSwapEvent,
                      SynchronizationEvent,
                      FALSE);

    KiProcessInSwapListHead.Next = NULL;
    KiProcessOutSwapListHead.Next = NULL;
    KiStackInSwapListHead.Next = NULL;

    //
    // Initialize the generic DPC call fast mutex.
    //

    ExInitializeFastMutex(&KiGenericCallDpcMutex);

    //
    // Initialize the system service descriptor table.
    //

    KeServiceDescriptorTable[0].Base = &KiServiceTable[0];
    KeServiceDescriptorTable[0].Count = NULL;
    KeServiceDescriptorTable[0].Limit = KiServiceLimit;
    KeServiceDescriptorTable[0].Number = KiArgumentTable;
    for (Index = 1; Index < NUMBER_SERVICE_TABLES; Index += 1) {
        KeServiceDescriptorTable[Index].Limit = 0;
    }

    //
    // Copy the system service descriptor table to the shadow table
    // which is used to record the Win32 system services.
    //

    RtlCopyMemory(KeServiceDescriptorTableShadow,
                  KeServiceDescriptorTable,
                  sizeof(KeServiceDescriptorTable));

    return;
}

ULARGE_INTEGER
KeComputeReciprocal (
    IN LONG Divisor,
    OUT PCCHAR Shift
    )

/*++

Routine Description:

    This function computes the 64-bit reciprocal of the specified value.

Arguments:

    Divisor - Supplies the value for which the large integer reciprocal is
        computed.

    Shift - Supplies a pointer to a variable that receives the computed
        shift count.

Return Value:

    The 64-bit reciprocal is returned as the function value.

--*/

{

    LARGE_INTEGER Fraction;
    LONG NumberBits;
    LONG Remainder;

    //
    // Compute the large integer reciprocal of the specified value.
    //

    NumberBits = 0;
    Remainder = 1;
    Fraction.LowPart = 0;
    Fraction.HighPart = 0;
    while (Fraction.HighPart >= 0) {
        NumberBits += 1;
        Fraction.HighPart = (Fraction.HighPart << 1) | (Fraction.LowPart >> 31);
        Fraction.LowPart <<= 1;
        Remainder <<= 1;
        if (Remainder >= Divisor) {
            Remainder -= Divisor;
            Fraction.LowPart |= 1;
        }
    }

    if (Remainder != 0) {
        if ((Fraction.LowPart == 0xffffffff) && (Fraction.HighPart == 0xffffffff)) {
            Fraction.LowPart = 0;
            Fraction.HighPart = 0x80000000;
            NumberBits -= 1;

        } else {
            if (Fraction.LowPart == 0xffffffff) {
                Fraction.LowPart = 0;
                Fraction.HighPart += 1;

            } else {
                Fraction.LowPart += 1;
            }
        }
    }

    //
    // Compute the shift count value and return the reciprocal fraction.
    //

    *Shift = (CCHAR)(NumberBits - 64);
    return *((PULARGE_INTEGER)&Fraction);
}

ULONG
KeComputeReciprocal32 (
    IN LONG Divisor,
    OUT PCCHAR Shift
    )

/*++

Routine Description:

    This function computes the 32-bit reciprocal of the specified value.

Arguments:

    Divisor - Supplies the value for which the large integer reciprocal is
        computed.

    Shift - Supplies a pointer to a variable that receives the computed
        shift count.

Return Value:

    The 32-bit reciprocal is returned as the function value.

--*/

{

    LONG Fraction;
    LONG NumberBits;
    LONG Remainder;

    //
    // Compute the 32-bit reciprocal of the specified value.
    //

    NumberBits = 0;
    Remainder = 1;
    Fraction = 0;
    while (Fraction >= 0) {
        NumberBits += 1;
        Fraction <<= 1;
        Remainder <<= 1;
        if (Remainder >= Divisor) {
            Remainder -= Divisor;
            Fraction |= 1;
        }
    }

    if (Remainder != 0) {
        if (Fraction == 0xffffffff) {
            Fraction = 0x80000000;
            NumberBits -= 1;

        } else {
            Fraction += 1;
        }
    }

    //
    // Compute the shift count value and return the reciprocal fraction.
    //

    *Shift = (CCHAR)(NumberBits - 32);
    return Fraction;
}

VOID
KeNumaInitialize (
    VOID
    )

/*++

Routine Description:

    Initialize the kernel structures needed to support NUMA systems.

Arguments:

    None.

Return Value:

    None.

--*/

{

#if defined(KE_MULTINODE)

    ULONG Length;
    HAL_NUMA_TOPOLOGY_INTERFACE NumaInformation;
    NTSTATUS Status;

    //
    // Query the NUMA topology of the system.
    //

    Status = HalQuerySystemInformation(HalNumaTopologyInterface,
                                       sizeof(HAL_NUMA_TOPOLOGY_INTERFACE),
                                       &NumaInformation,
                                       &Length);

    //
    // If the query was successful and the number of nodes in the host
    // system is greater than one, then set the number of nodes and the
    // address of the page to node and query processor functions.
    //

    if (NT_SUCCESS(Status)) {

        ASSERT(Length == sizeof(HAL_NUMA_TOPOLOGY_INTERFACE));
        ASSERT(NumaInformation.NumberOfNodes <= MAXIMUM_CCNUMA_NODES);
        ASSERT(NumaInformation.QueryProcessorNode != NULL);
        ASSERT(NumaInformation.PageToNode != NULL);

        if (NumaInformation.NumberOfNodes > 1) {
            KeNumberNodes = (UCHAR)NumaInformation.NumberOfNodes;
            MmPageToNode = NumaInformation.PageToNode;
            KiQueryProcessorNode = NumaInformation.QueryProcessorNode;
        }
    }

#endif

    return;
}

