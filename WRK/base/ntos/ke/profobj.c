/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    profobj.c

Abstract:

    This module implements the kernel Profile Object. Functions are
    provided to initialize, start, and stop profile objects and to set
    and query the profile interval.

--*/

#include "ki.h"

#pragma alloc_text(PAGE, KeQueryIntervalProfile)

//
// The following assert macro is used to check that an input profile object is
// really a kprofile and not something else, like deallocated pool.
//

#define ASSERT_PROFILE(E) {             \
    ASSERT((E)->Type == ProfileObject); \
}

//
// Structure representing an active profile source
//

typedef struct _KACTIVE_PROFILE_SOURCE {
    LIST_ENTRY ListEntry;
    KPROFILE_SOURCE Source;
    KAFFINITY Affinity;
    ULONG ProcessorCount[1];            // variable-sized, one per processor
} KACTIVE_PROFILE_SOURCE, *PKACTIVE_PROFILE_SOURCE;

//
// Prototypes for IPI target functions
//

VOID
KiStartProfileInterrupt (
    __inout PKIPI_CONTEXT SignalDone,
    __in PVOID Parameter1,
    __in PVOID Parameter2,
    __in PVOID Parameter3
    );

VOID
KiStopProfileInterrupt (
    __inout PKIPI_CONTEXT SignalDone,
    __in PVOID Parameter1,
    __in PVOID Parameter2,
    __in PVOID Parameter3
    );

VOID
KeInitializeProfile (
    __out PKPROFILE Profile,
    __in_opt PKPROCESS Process,
    __in_opt PVOID RangeBase,
    __in SIZE_T RangeSize,
    __in ULONG BucketSize,
    __in ULONG Segment,
    __in KPROFILE_SOURCE ProfileSource,
    __in KAFFINITY ProfileAffinity
    )

/*++

Routine Description:

    This function initializes a kernel profile object. The process,
    address range, bucket size, and buffer are set. The profile is
    set to the stopped state.

Arguments:

    Profile - Supplies a pointer to control object of type profile.

    Process - Supplies an optional pointer to a process object that
        describes the address space to profile. If not specified,
        then all address spaces are included in the profile.

    RangeBase - Supplies the address of the first byte of the address
        range for which profiling information is to be collected.

    RangeSize - Supplies the size of the address range for which profiling
        information is to be collected.  The RangeBase and RangeSize
        parameters are interpreted such that RangeBase <= address <
        RangeBase + RangeSize generates a profile hit.

    BucketSize - Supplies the log base 2 of the size of a profiling bucket.
        Thus, BucketSize = 2 yields 4-byte buckets, BucketSize = 7 yields
        128-byte buckets.

    Segment - Supplies the non-Flat code segment to profile.  If this
        is zero, then the flat profiling is done.  This will only
        be non-zero on an x86 machine.

    ProfileSource - Supplies the profile interrupt source.

    ProfileAffinity - Supplies the set of processor to count hits for.

Return Value:

    None.

--*/

{

#if !defined(i386)

    ASSERT(Segment == 0);

#endif

    //
    // Initialize the standard control object header.
    //

    Profile->Type = ProfileObject;
    Profile->Size = sizeof(KPROFILE);

    //
    // Initialize the process address space, range base, range limit,
    // bucket shift count, and set started FALSE.
    //

    if (ARGUMENT_PRESENT(Process)) {
        Profile->Process = Process;

    } else {
        Profile->Process = NULL;
    }

    Profile->RangeBase = RangeBase;
    Profile->RangeLimit = (PUCHAR)RangeBase + RangeSize;
    Profile->BucketShift = BucketSize - 2;
    Profile->Started = FALSE;
    Profile->Segment = Segment;
    Profile->Source = (CSHORT)ProfileSource;
    Profile->Affinity = ProfileAffinity & KeActiveProcessors;
    if (Profile->Affinity == 0) {
        Profile->Affinity = KeActiveProcessors;
    }

    return;
}

ULONG
KeQueryIntervalProfile (
    __in KPROFILE_SOURCE ProfileSource
    )

/*++

Routine Description:

    This function returns the profile sample interval the system is
    currently using.

Arguments:

    ProfileSource - Supplies the profile source to be queried.

Return Value:

    Sample interval in units of 100ns.

--*/

{

    HAL_PROFILE_SOURCE_INFORMATION ProfileSourceInfo;
    ULONG ReturnedLength;
    NTSTATUS Status;

    if (ProfileSource == ProfileTime) {

        //
        // Return the current sampling interval in 100ns units.
        //

        return KiProfileInterval;

    } else

    if (ProfileSource == ProfileAlignmentFixup) {
        return KiProfileAlignmentFixupInterval;

    } else {

        //
        // The HAL is responsible for tracking this profile interval.
        //

        ProfileSourceInfo.Source = ProfileSource;
        Status = HalQuerySystemInformation(HalProfileSourceInformation,
                                           sizeof(HAL_PROFILE_SOURCE_INFORMATION),
                                           &ProfileSourceInfo,
                                           &ReturnedLength);

        if (NT_SUCCESS(Status) && ProfileSourceInfo.Supported) {
            return ProfileSourceInfo.Interval;

        } else {
            return 0;
        }
    }
}

VOID
KeSetIntervalProfile (
    __in ULONG Interval,
    __in KPROFILE_SOURCE Source
    )

/*++

Routine Description:

    This function sets the profile sampling interval. The interval is in
    100ns units. The interval will actually be set to some value in a set
    of preset values (at least on pc based hardware), using the one closest
    to what the user asked for.

Arguments:

    Interval - Supplies the length of the sampling interval in 100ns units.

Return Value:

    None.

--*/

{

    HAL_PROFILE_SOURCE_INTERVAL ProfileSourceInterval;

    if (Source == ProfileTime) {

        //
        // If the specified sampling interval is less than the minimum
        // sampling interval, then set the sampling interval to the minimum
        // sampling interval.
        //

        if (Interval < MINIMUM_PROFILE_INTERVAL) {
            Interval = MINIMUM_PROFILE_INTERVAL;
        }

        //
        // Set the sampling interval.
        //

        KiProfileInterval = (ULONG)KeIpiGenericCall(HalSetProfileInterval, Interval);

    } else

    if (Source == ProfileAlignmentFixup) {
        KiProfileAlignmentFixupInterval = Interval;

    } else {

        //
        // The HAL is responsible for setting this profile interval.
        //

        ProfileSourceInterval.Source = Source;
        ProfileSourceInterval.Interval = Interval;
        HalSetSystemInformation(HalProfileSourceInterval,
                                sizeof(HAL_PROFILE_SOURCE_INTERVAL),
                                &ProfileSourceInterval);
    }

    return;
}

BOOLEAN
KeStartProfile (
    __inout PKPROFILE Profile,
    __out_opt PULONG Buffer
    )

/*++

Routine Description:

    This function starts profile data gathering on the specified profile
    object. The profile object is marked started, and is registered with
    the profile interrupt procedure.

    If the number of active profile objects was previously zero, then the
    profile interrupt is enabled.

    N.B. For the current implementation, an arbitrary number of profile
        objects may be active at once. This can present a large system
        overhead. It is assumed that the caller appropriately limits the
        the number of active profiles.

Arguments:

    Profile - Supplies a pointer to a control object of type profile.

    Buffer - Supplies a pointer to an optional array of counters which record
        the number of hits in the corresponding bucket.

Return Value:

    A value of TRUE is returned if profiling was previously stopped for
    the specified profile object. Otherwise, a value of FALSE is returned.

--*/

{

    KIRQL OldIrql;
    PKPROCESS Process;
    BOOLEAN Started;
    KAFFINITY TargetProcessors;
    PKPRCB Prcb;
    PKACTIVE_PROFILE_SOURCE ActiveSource = NULL;
    PKACTIVE_PROFILE_SOURCE CurrentActiveSource;
    PKACTIVE_PROFILE_SOURCE AllocatedPool;
    PLIST_ENTRY ListEntry;
    ULONG SourceSize;
    KAFFINITY AffinitySet;
    PULONG Reference;

    ASSERT_PROFILE(Profile);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Allocate pool that may be required before raising to PROFILE_LEVEL.
    //

    SourceSize =
        sizeof(KACTIVE_PROFILE_SOURCE) + sizeof(ULONG) * (KeNumberProcessors - 1);

    AllocatedPool = ExAllocatePoolWithTag(NonPagedPool, SourceSize, 'forP');
    if (AllocatedPool == NULL) {
        return(TRUE);
    }

    //
    // Raise IRQL to profile level and acquire the profile lock.
    //

    OldIrql = KfRaiseIrql(KiProfileIrql);
    KeAcquireSpinLockAtDpcLevel(&KiProfileLock);

    //
    // Assume object already started.
    //

    Started = FALSE;
    AffinitySet = 0L;
    TargetProcessors = 0L;

    //
    // If the specified profile object is not started, set started to TRUE,
    // set the address of the profile buffer, set the profile object to started,
    // insert the profile object in the appropriate profile list, and start
    // profile interrupts if the number of active profile objects was previously zero.
    //

    Prcb = KeGetCurrentPrcb();
    if (Profile->Started == FALSE) {
        Started = TRUE;
        Profile->Buffer = Buffer;
        Profile->Started = TRUE;
        Process = Profile->Process;

        //
        // If a profile buffer is specified, then profile either the process
        // or system. Otherwise, profile events.
        //

        if (Profile->Buffer != NULL) {
            if (Process != NULL) {
                InsertTailList(&Process->ProfileListHead, &Profile->ProfileListEntry);

            } else {
                InsertTailList(&KiProfileListHead, &Profile->ProfileListEntry);
            }

        } else {
            InitializeListHead(&Profile->ProfileListEntry);
        }

        //
        // Check the profile source list to see if this profile source is
        // already started. If so, update the reference counts. If not,
        // allocate a profile source object, initialize the reference
        // counts, and add it to the list.
        //

        ListEntry = KiProfileSourceListHead.Flink;
        while (ListEntry != &KiProfileSourceListHead) {
            CurrentActiveSource = CONTAINING_RECORD(ListEntry,
                                                    KACTIVE_PROFILE_SOURCE,
                                                    ListEntry);

            if (CurrentActiveSource->Source == Profile->Source) {
                ActiveSource = CurrentActiveSource;
                break;
            }

            ListEntry = ListEntry->Flink;
        }

        if (ActiveSource == NULL) {

            //
            // This source was not found, allocate and initialize a new entry and add
            // it to the head of the list.
            //

            ActiveSource = AllocatedPool;
            AllocatedPool = NULL;
            RtlZeroMemory(ActiveSource, SourceSize);
            ActiveSource->Source = Profile->Source;
            InsertHeadList(&KiProfileSourceListHead, &ActiveSource->ListEntry);
            if (Profile->Source == ProfileAlignmentFixup) {
                KiProfileAlignmentFixup = TRUE;
            }
        }

        //
        // Increment the reference counts for each processor in the
        // affinity set.
        //

        AffinitySet = Profile->Affinity;
        Reference = &ActiveSource->ProcessorCount[0];
        while (AffinitySet != 0) {
            if (AffinitySet & 1) {
                *Reference = *Reference + 1;
            }

            AffinitySet = AffinitySet >> 1;
            Reference = Reference + 1;
        }

        //
        // Compute the processors which the profile interrupt is
        // required and not already started
        //

        AffinitySet = Profile->Affinity & ~ActiveSource->Affinity;
        TargetProcessors = AffinitySet & ~Prcb->SetMember;

        //
        // Update set of processors on which this source is active.
        //

        ActiveSource->Affinity |= Profile->Affinity;
    }

    //
    // Release the profile lock, lower IRQL to its previous value, and
    // return whether profiling was started.
    //

    KeReleaseSpinLockFromDpcLevel(&KiProfileLock);
    KeLowerIrql(DISPATCH_LEVEL);

    //
    // Start profile interrupt on pending processors.
    //

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiStartProfileInterrupt,
                        (PVOID)(ULONG_PTR)Profile->Source,
                        NULL,
                        NULL);
    }

#endif

    if (AffinitySet & Prcb->SetMember) {
        if (Profile->Source == ProfileAlignmentFixup) {
            KiEnableAlignmentExceptions();
        }

        KfRaiseIrql(KiProfileIrql);
        HalStartProfileInterrupt(Profile->Source);
        KeLowerIrql(DISPATCH_LEVEL);
    }

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargetsPrcb(TargetProcessors, Prcb);
    }

#endif

    //
    // Lower to original IRQL
    //

    KeLowerIrql(OldIrql);

    //
    // If the allocated pool was not used, free it now.
    //

    if (AllocatedPool != NULL) {
        ExFreePool(AllocatedPool);
    }

    return Started;
}

BOOLEAN
KeStopProfile (
    __inout PKPROFILE Profile
    )

/*++

Routine Description:

    This function stops profile data gathering on the specified profile
    object. The object is marked stopped, and is removed from the active
    profile list.

    If the number of active profile objects goes to zero, then the profile
    interrupt is disabled.

Arguments:

    Profile - Supplies a pointer to a control object of type profile.

Return Value:

    A value of TRUE is returned if profiling was previously started for
    the specified profile object. Otherwise, a value of FALSE is returned.

--*/

{

    KIRQL OldIrql;
    BOOLEAN Stopped;
    KAFFINITY TargetProcessors;
    PKPRCB Prcb;
    PLIST_ENTRY ListEntry;
    PKACTIVE_PROFILE_SOURCE ActiveSource;
    PKACTIVE_PROFILE_SOURCE PoolToFree=NULL;
    KAFFINITY AffinitySet = 0;
    KAFFINITY CurrentProcessor;
    PULONG Reference;

    ASSERT_PROFILE(Profile);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Assume object already stopped
    //

    Stopped = FALSE;
    AffinitySet = 0L;
    TargetProcessors = 0L;

    //
    // Raise IRQL to profile level and acquire the profile lock.
    //

    OldIrql = KfRaiseIrql(KiProfileIrql);
    KeAcquireSpinLockAtDpcLevel(&KiProfileLock);

    //
    // If the specified profile object is not stopped, set stopped to TRUE, set
    // the profile object to stopped, remove the profile object object from the
    // appropriate profilelist, and stop profile interrupts if the number of
    // active profile objects is zero.
    //

    Prcb = KeGetCurrentPrcb();
    if (Profile->Started != FALSE) {
        Stopped = TRUE;
        Profile->Started = FALSE;
        if (!IsListEmpty(&Profile->ProfileListEntry)) {
            RemoveEntryList(&Profile->ProfileListEntry);
        }

        //
        // Search the profile source list to find the entry for this
        // profile source.
        //

        ListEntry = KiProfileSourceListHead.Flink;
        do {
            ASSERT(ListEntry != &KiProfileSourceListHead);
            ActiveSource = CONTAINING_RECORD(ListEntry,
                                             KACTIVE_PROFILE_SOURCE,
                                             ListEntry);

            ListEntry = ListEntry->Flink;
        } while (ActiveSource->Source != Profile->Source);

        //
        // Decrement the reference counts for each processor in the
        // affinity set and build up a mask of the processors that
        // now have a reference count of zero.
        //

        CurrentProcessor = 1;
        TargetProcessors = 0;
        AffinitySet = Profile->Affinity;
        Reference = &ActiveSource->ProcessorCount[0];
        while (AffinitySet != 0) {
            if (AffinitySet & 1) {
                *Reference = *Reference - 1;
                if (*Reference == 0) {
                    TargetProcessors = TargetProcessors | CurrentProcessor;
                }
            }

            AffinitySet = AffinitySet >> 1;
            Reference = Reference + 1;
            CurrentProcessor = CurrentProcessor << 1;
        }

        //
        // Compute the processors whose profile interrupt reference
        // count has dropped to zero.
        //

        AffinitySet = TargetProcessors;
        TargetProcessors = AffinitySet & ~Prcb->SetMember;

        //
        // Update set of processors on which this source is active.
        //

        ActiveSource->Affinity &= ~AffinitySet;

        //
        // Determine whether this profile source is stopped on all
        // processors. If so, remove it from the list and free it.
        //

        if (ActiveSource->Affinity == 0) {
            RemoveEntryList(&ActiveSource->ListEntry);
            PoolToFree = ActiveSource;
            if (Profile->Source == ProfileAlignmentFixup) {
                KiProfileAlignmentFixup = FALSE;
            }
        }
    }

    //
    // Release the profile lock, lower IRQL to its previous value, and
    // return whether profiling was stopped.
    //

    KeReleaseSpinLockFromDpcLevel(&KiProfileLock);
    KeLowerIrql(DISPATCH_LEVEL);

    //
    // Stop profile interrupt on pending processors
    //

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiSendPacket(TargetProcessors,
                        KiStopProfileInterrupt,
                        (PVOID)(ULONG_PTR)Profile->Source,
                        NULL,
                        NULL);
    }

#endif

    if (AffinitySet & Prcb->SetMember) {
        if (Profile->Source == ProfileAlignmentFixup) {
            KiDisableAlignmentExceptions();
        }

        KfRaiseIrql(KiProfileIrql);
        HalStopProfileInterrupt(Profile->Source);
        KeLowerIrql(DISPATCH_LEVEL);
    }

#if !defined(NT_UP)

    if (TargetProcessors != 0) {
        KiIpiStallOnPacketTargetsPrcb(TargetProcessors, Prcb);
    }

#endif

    //
    // Lower to original IRQL
    //

    KeLowerIrql(OldIrql);

    //
    // Now that IRQL has been lowered, free the profile source if
    // necessary.
    //

    if (PoolToFree != NULL) {
        ExFreePool(PoolToFree);
    }

    return Stopped;
}

#if !defined(NT_UP)

VOID
KiStopProfileInterrupt (
    __inout PKIPI_CONTEXT SignalDone,
    __in PVOID Parameter1,
    __in PVOID Parameter2,
    __in PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for stopping the profile interrupt on target
    processors.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed

    Parameter1 - Supplies the profile source

    Parameter2 - Parameter3 - not used

Return Value:

    None.

--*/

{

    KIRQL OldIrql;
    KPROFILE_SOURCE ProfileSource;

    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

    //
    // Stop the profile interrupt on the current processor and clear the
    // data cache packet address to signal the source to continue.
    //

    ProfileSource = (KPROFILE_SOURCE)PtrToUlong(Parameter1);
    if (ProfileSource == ProfileAlignmentFixup) {
        KiDisableAlignmentExceptions();
    }

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < KiProfileIrql) {
        KfRaiseIrql(KiProfileIrql);
    }

    HalStopProfileInterrupt(ProfileSource);
    KeLowerIrql(OldIrql);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

VOID
KiStartProfileInterrupt (
    __inout PKIPI_CONTEXT SignalDone,
    __in PVOID Parameter1,
    __in PVOID Parameter2,
    __in PVOID Parameter3
    )

/*++

Routine Description:

    This is the target function for stopping the profile interrupt on target
    processors.

Arguments:

    SignalDone - Supplies a pointer to a variable that is cleared when the
        requested operation has been performed

    Parameter1 - Supplies the profile source

    Parameter2 - Parameter3 - not used

Return Value:

    None.

--*/

{

    KIRQL OldIrql;
    KPROFILE_SOURCE ProfileSource;

    UNREFERENCED_PARAMETER(Parameter2);
    UNREFERENCED_PARAMETER(Parameter3);

    //
    // Start the profile interrupt on the current processor and clear the
    // data cache packet address to signal the source to continue.
    //

    ProfileSource = (KPROFILE_SOURCE)PtrToUlong(Parameter1);
    if (ProfileSource == ProfileAlignmentFixup) {
        KiEnableAlignmentExceptions();
    }

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < KiProfileIrql) {
        KfRaiseIrql(KiProfileIrql);
    }

    HalStartProfileInterrupt(ProfileSource);
    KeLowerIrql(OldIrql);
    KiIpiSignalPacketDone(SignalDone);
    return;
}

#endif

