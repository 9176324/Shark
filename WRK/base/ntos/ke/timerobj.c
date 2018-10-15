/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    timerobj.c

Abstract:

    This module implements the kernel timer object. Functions are
    provided to initialize, read, set, and cancel timer objects.

--*/

#include "ki.h"

//
// Flags controlling the nature of timer checks. This checks are triggered
// when driver verifier is active and default behavior is to check for active timers
// involved in pool free and driver unload operations.
//

#define KE_TIMER_CHECK_FREES 0x1
ULONG KeTimerCheckFlags = KE_TIMER_CHECK_FREES;

//
// The following assert macro is used to check that an input timer is
// really a ktimer and not something else, like deallocated pool.
//

#define ASSERT_TIMER(E) {                                                    \
    ASSERT(((E)->Header.Type == TimerNotificationObject) ||                  \
           ((E)->Header.Type == TimerSynchronizationObject));                \
}

VOID
KeInitializeTimer (
    __out PKTIMER Timer
    )

/*++

Routine Description:

    This function initializes a kernel timer object.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    None.

--*/

{

    //
    // Initialize extended timer object with a type of notification and a
    // period of zero.
    //

    KeInitializeTimerEx(Timer, NotificationTimer);
    return;
}

VOID
KeInitializeTimerEx (
    __out PKTIMER Timer,
    __in TIMER_TYPE Type
    )

/*++

Routine Description:

    This function initializes an extended kernel timer object.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

    Type - Supplies the type of timer object; NotificationTimer or
        SynchronizationTimer;

Return Value:

    None.

--*/

{

    //
    // Initialize standard dispatcher object header and set initial
    // state of timer.
    //

    Timer->Header.Type = (UCHAR)(TimerNotificationObject + Type);
    Timer->Header.Inserted = FALSE;
    Timer->Header.Size = sizeof(KTIMER) / sizeof(LONG);
    Timer->Header.SignalState = FALSE;

#if DBG

    Timer->TimerListEntry.Flink = NULL;
    Timer->TimerListEntry.Blink = NULL;

#endif

    InitializeListHead(&Timer->Header.WaitListHead);
    Timer->DueTime.QuadPart = 0;
    Timer->Period = 0;
    return;
}

VOID
KeClearTimer (
    __inout PKTIMER Timer
    )

/*++

Routine Description:

    This function clears the signal state of an timer object.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    None.

--*/

{

    ASSERT_TIMER(Timer);

    //
    // Clear signal state of timer object.
    //

    Timer->Header.SignalState = 0;
    return;
}

BOOLEAN
KeCancelTimer (
    __inout PKTIMER Timer
    )

/*++

Routine Description:

    This function cancels a timer that was previously set to expire at
    a specified time. If the timer is not currently set, then no operation
    is performed. Canceling a timer does not set the state of the timer to
    Signaled.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    A boolean value of TRUE is returned if the the specified timer was
    currently set. Else a value of FALSE is returned.

--*/

{

    BOOLEAN Inserted;
    KIRQL OldIrql;

    ASSERT_TIMER(Timer);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level, lock the dispatcher database, and
    // capture the timer inserted status. If the timer is currently set,
    // then remove it from the timer list.
    //

    KiLockDispatcherDatabase(&OldIrql);
    Inserted = Timer->Header.Inserted;
    if (Inserted != FALSE) {
        KiRemoveTreeTimer(Timer);
    }

    //
    // Unlock the dispatcher database, lower IRQL to its previous value, and
    // return boolean value that signifies whether the timer was set of not.
    //

    KiUnlockDispatcherDatabase(OldIrql);
    return Inserted;
}

BOOLEAN
KeReadStateTimer (
    __in PKTIMER Timer
    )

/*++

Routine Description:

    This function reads the current signal state of a timer object.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    The current signal state of the timer object.

--*/

{

    ASSERT_TIMER(Timer);

    //
    // Return current signal state of timer object.
    //

    return (BOOLEAN)Timer->Header.SignalState;
}

BOOLEAN
KeSetTimer (
    __inout PKTIMER Timer,
    __in LARGE_INTEGER DueTime,
    __in_opt PKDPC Dpc
    )

/*++

Routine Description:

    This function sets a timer to expire at a specified time. If the timer is
    already set, then it is implicitly canceled before it is set to expire at
    the specified time. Setting a timer causes its due time to be computed,
    its state to be set to Not-Signaled, and the timer object itself to be
    inserted in the timer list.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

    DueTime - Supplies an absolute or relative time at which the timer
        is to expire.

    Dpc - Supplies an optional pointer to a control object of type DPC.

Return Value:

    A boolean value of TRUE is returned if the the specified timer was
    currently set. Else a value of FALSE is returned.

--*/

{

    //
    // Set the timer with a period of zero.
    //

    return KeSetTimerEx(Timer, DueTime, 0, Dpc);
}

BOOLEAN
KeSetTimerEx (
    __inout PKTIMER Timer,
    __in LARGE_INTEGER DueTime,
    __in LONG Period,
    __in_opt PKDPC Dpc
    )

/*++

Routine Description:

    This function sets a timer to expire at a specified time. If the timer is
    already set, then it is implicitly canceled before it is set to expire at
    the specified time. Setting a timer causes its due time to be computed,
    its state to be set to Not-Signaled, and the timer object itself to be
    inserted in the timer list.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

    DueTime - Supplies an absolute or relative time at which the timer
        is to expire.

    Period - Supplies an optional period for the timer in milliseconds.

    Dpc - Supplies an optional pointer to a control object of type DPC.

Return Value:

    A boolean value of TRUE is returned if the the specified timer was
    currently set. Otherwise, a value of FALSE is returned.

--*/

{

    ULONG Hand;
    BOOLEAN Inserted;
    KIRQL OldIrql;
    BOOLEAN RequestInterrupt;

    ASSERT_TIMER(Timer);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //
    // Capture the timer inserted status and if the timer is currently
    // set, then remove it from the timer list.
    //

    KiLockDispatcherDatabase(&OldIrql);
    Inserted = Timer->Header.Inserted;
    if (Inserted != FALSE) {
        KiRemoveTreeTimer(Timer);
    }

    //
    // Set the DPC address, set the period, and compute the timer due time.
    // If the timer has already expired, then signal the timer. Otherwise,
    // set the signal state to false and attempt to insert the timer in the
    // timer table.
    //
    // N.B. The signal state must be cleared before it is inserted in the
    //      timer table in case the period is not zero.
    //

    Timer->Dpc = Dpc;
    Timer->Period = Period;
    if (KiComputeDueTime(Timer, DueTime, &Hand) == FALSE) {
        RequestInterrupt = KiSignalTimer(Timer);
        KiUnlockDispatcherDatabaseFromSynchLevel();
        if (RequestInterrupt == TRUE) {
            KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
        }

    } else {
        Timer->Header.SignalState = FALSE;
        KiInsertOrSignalTimer(Timer, Hand);
    }

    KiExitDispatcher(OldIrql);

    //
    // Return boolean value that signifies whether the timer was set or
    // not.
    //

    return Inserted;
}

ULONGLONG
KeQueryTimerDueTime (
    __in PKTIMER Timer
    )

/*++

Routine Description:

    This function returns the interrupt time at which the specified timer is
    due to expire. If the timer is not pending, then zero is returned.

    N.B. This function may only be called by the system sleep code.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    Returns the amount of time remaining on the timer, or 0 if the
    timer is not pending.

--*/

{

    ULONGLONG DueTime;
    KIRQL OldIrql;

    ASSERT_TIMER(Timer);


    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //
    // If the timer is currently pending, then return the due time. Otherwise,
    // return zero.
    //

    DueTime = 0;
    KiLockDispatcherDatabase(&OldIrql);
    if (Timer->Header.Inserted) {
        DueTime = Timer->DueTime.QuadPart;
    }

    //
    // Unlock the dispatcher database, lower IRQL to its previous value, and
    // return the due time.
    //

    KiUnlockDispatcherDatabase(OldIrql);
    return DueTime;
}

VOID
KeCheckForTimer (
    __in_bcount(BlockSize) PVOID BlockStart,
    __in SIZE_T BlockSize
    )

/*++

Routine Description:

    This function checks to detemine if the specified block of memory
    overlaps the range of an active timer.

Arguments:

    BlockStart - Supplies a pointer to the block of memory to check.

    BlockSize - Supplies the size, in bytes, of the block of memory to check.

Return Value:

    None.

--*/

{

    PUCHAR Address;
    PUCHAR End;
    ULONG Index;
    PLIST_ENTRY ListHead;
    PKSPIN_LOCK_QUEUE LockQueue;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKTIMER Timer;
    PUCHAR Start;

    //
    // Make sure timer checks are enabled before proceeding.
    //

    if ((KeTimerCheckFlags & KE_TIMER_CHECK_FREES) == 0) {
        return;
    }

    //
    // Compute the ending memory location.
    //

    Start = (PUCHAR)BlockStart;
    End = Start + BlockSize;

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //
    // Scan the timer database and check for any timers in the specified
    // memory block.
    //

    KiLockDispatcherDatabase(&OldIrql);
    Index = 0;
    do {
        ListHead = &KiTimerTableListHead[Index].Entry;
        LockQueue = KiAcquireTimerTableLock(Index);
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            Address = (PUCHAR)Timer;
            NextEntry = NextEntry->Flink;

            //
            // Check that the timer object is not in the range.
            //

            if ((Address > (Start - sizeof(KTIMER))) &&
                (Address < End)) {
                KeBugCheckEx(TIMER_OR_DPC_INVALID,
                             0x0,
                             (ULONG_PTR)Address,
                             (ULONG_PTR)Start,
                             (ULONG_PTR)End);
            }

            if (Timer->Dpc) {

                //
                // Check that the timer DPC object is not in the range.
                //

                Address = (PUCHAR)Timer->Dpc;
                if ((Address > (Start - sizeof(KDPC))) &&
                    (Address < End)) {
                    KeBugCheckEx(TIMER_OR_DPC_INVALID,
                                 0x1,
                                 (ULONG_PTR)Address,
                                 (ULONG_PTR)Start,
                                 (ULONG_PTR)End);
                }

                //
                // Check that the timer DPC routine is not in the range.
                //

                Address = (PUCHAR)(ULONG_PTR) Timer->Dpc->DeferredRoutine;
                if (Address >= Start && Address < End) {
                    KeBugCheckEx(TIMER_OR_DPC_INVALID,
                                 0x2,
                                 (ULONG_PTR)Address,
                                 (ULONG_PTR)Start,
                                 (ULONG_PTR)End);
                }
            }
        }

        KiReleaseTimerTableLock(LockQueue);
        Index += 1;
    } while(Index < TIMER_TABLE_SIZE);

    //
    // Unlock the dispatcher database and lower IRQL to its previous value
    //

    KiUnlockDispatcherDatabase(OldIrql);
}

