/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    timersup.c

Abstract:

    This module contains the support routines for the timer object. It
    contains functions to insert and remove from the timer queue.

--*/

#include "ki.h"

VOID
FASTCALL
KiCompleteTimer (
    __inout PKTIMER Timer,
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*++

Routine Description:

    This function completes a timer that could not be inserted in the timer
    table because its due time has already passed.

    N.B. This function must be called with the corresponding timer table
         entry locked at raised IRQL.

    N.B. This function returns with no locks held at raised IRQL.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

    LockQueue - Supplies a pointer to a timer table lock queue entry.

Return Value:

    None.

-*/

{

    LIST_ENTRY ListHead;
    BOOLEAN RequestInterrupt;

    //
    // Remove the timer from the specified timer table list and insert
    // the timer in a dummy list in case the timer is cancelled while
    // releasing the timer table lock and acquiring the dispatcher lock.
    //

    KiRemoveEntryTimer(Timer);
    ListHead.Flink = &Timer->TimerListEntry;
    ListHead.Blink = &Timer->TimerListEntry;
    Timer->TimerListEntry.Flink = &ListHead;
    Timer->TimerListEntry.Blink = &ListHead;
    KiReleaseTimerTableLock(LockQueue);
    RequestInterrupt = FALSE;
    KiLockDispatcherDatabaseAtSynchLevel();

    //
    // If the timer has not been removed from the dummy list, then signal
    // the timer.
    //

    if (ListHead.Flink != &ListHead) {
        RequestInterrupt = KiSignalTimer(Timer);
    }

    //
    // Release the dispatcher lock and request a DPC interrupt if required.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    if (RequestInterrupt == TRUE) {
        KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
    }

    return;
}

LOGICAL
FASTCALL
KiInsertTimerTable (
    IN PKTIMER Timer,
    IN ULONG Hand
    )

/*++

Routine Description:

    This function inserts a timer object in the timer table.

    N.B. This routine assumes that the timer table lock has been acquired.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

    Hand - supplies the timer table hand value.

Return Value:

    If the timer has expired, than a value of TRUE is returned. Otherwise, a
    value of FALSE is returned.

--*/

{

    ULONG64 DueTime;
    LOGICAL Expired;
    ULARGE_INTEGER InterruptTime;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PKTIMER NextTimer;

    //
    // Set the signal state to FALSE if the period is zero.
    //
    // N.B. The timer state is set to inserted.
    //

    Expired = FALSE;
    if (Timer->Period == 0) {
        Timer->Header.SignalState = FALSE;
    }

    //
    // If the timer is due before the first entry in the computed list
    // or the computed list is empty, then insert the timer at the front
    // of the list and check if the timer has already expired. Otherwise,
    // insert then timer in the sorted order of the list searching from
    // the back of the list forward.
    //
    // N.B. The sequence of operations below is critical to avoid the race
    //      condition that exists between this code and the clock interrupt
    //      code that examines the timer table lists to detemine when timers
    //      expire.
    //

    DueTime = Timer->DueTime.QuadPart;

    ASSERT(Hand == KiComputeTimerTableIndex(DueTime));

    ListHead = &KiTimerTableListHead[Hand].Entry;
    NextEntry = ListHead->Blink;
    while (NextEntry != ListHead) {

        //
        // Compute the maximum search count.
        //

        NextTimer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
        if (DueTime >= (ULONG64)NextTimer->DueTime.QuadPart) {
            break;
        }

        NextEntry = NextEntry->Blink;
    }

    InsertHeadList(NextEntry, &Timer->TimerListEntry);
    if (NextEntry == ListHead) {

        //
        // The computed list is empty or the timer is due to expire before
        // the first entry in the list.
        //
        // Make sure the writes for the update of the due time table are done
        // before reading the interrupt time.
        //

        KiTimerTableListHead[Hand].Time.QuadPart = DueTime; 
        KeMemoryBarrier();
        KiQueryInterruptTime((PLARGE_INTEGER)&InterruptTime);
        if (DueTime <= (ULONG64)InterruptTime.QuadPart) {
            Expired = TRUE;
        }
    }

    return Expired;
}

BOOLEAN
FASTCALL
KiSignalTimer (
    __inout PKTIMER Timer
    )

/*++

Routine Description:

    This function signals a timer that could not be inserted in the timer
    table because its due time has already passed.

    N.B. This function must be called with the dispatcher database locked at
         at raised IRQL.

Arguments:

    Timer - Supplies a pointer to a dispatcher object of type timer.

Return Value:

    If a software interrupt should be requested, then TRUE is returned.
    Otherwise, a value of FALSE is returned.

-*/

{

    PKDPC Dpc;
    LARGE_INTEGER Interval;
    LONG Period;
    BOOLEAN RequestInterrupt;
    LARGE_INTEGER SystemTime;

    //
    // Set time timer state to not inserted and the signal state to signaled.
    //

    RequestInterrupt = FALSE;
    Timer->Header.Inserted = FALSE;
    Timer->Header.SignalState = 1;

    //
    // Capture the DPC and period fields from the timer object. Once wait
    // test is called, the timer must not be touched again unless it is
    // periodic. The reason for this is that a thread may allocate a timer
    // on its local stack and wait on it. Wait test can cause that thread
    // to immediately start running on another processor on an MP system.
    // If the thread returns, then the timer will be corrupted.
    // 

    Dpc = Timer->Dpc;
    Period = Timer->Period;
    if (IsListEmpty(&Timer->Header.WaitListHead) == FALSE) {
        if (Timer->Header.Type == TimerNotificationObject) {
            KiWaitTestWithoutSideEffects(Timer, TIMER_EXPIRE_INCREMENT);

        } else {
            KiWaitTestSynchronizationObject(Timer, TIMER_EXPIRE_INCREMENT);
        }
    }

    //
    // If the timer is periodic, then compute the next interval time and
    // reinsert the timer in the timer tree.
    //
    // N.B. Even though the timer insertion is relative, it can still fail
    //      if the period of the timer elapses in between computing the time
    //      and inserting the timer. If this happens, then the insertion is
    //      retried.
    //

    if (Period != 0) {
        Interval.QuadPart = Int32x32To64(Period, - 10 * 1000);
        do {
        } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
    }

    //
    // If a DPC is specified, then insert it in the target processor's DPC
    // queue or capture the parameters in the DPC table for subsequent
    // execution on the current processor.
    //
    // N.B. A dispatch interrupt must be forced on the current processor,
    //      however, this will occur very infrequently.
    //

    if (Dpc != NULL) {
        KiQuerySystemTime(&SystemTime);
        KeInsertQueueDpc(Dpc,
                         ULongToPtr(SystemTime.LowPart),
                         ULongToPtr(SystemTime.HighPart));

        RequestInterrupt = TRUE;
    }

    return RequestInterrupt;
}

