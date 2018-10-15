/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    eventobj.c

Abstract:

    This module implements the kernel event objects. Functions are
    provided to initialize, pulse, read, reset, and set event objects.

--*/

#include "ki.h"

#pragma alloc_text (PAGE, KeInitializeEventPair)

#undef KeClearEvent

//
// The following assert macro is used to check that an input event is
// really a kernel event and not something else, like deallocated pool.
//

#define ASSERT_EVENT(E) {                             \
    ASSERT((E)->Header.Type == NotificationEvent ||   \
           (E)->Header.Type == SynchronizationEvent); \
}

//
// The following assert macro is used to check that an input event is
// really a kernel event pair and not something else, like deallocated
// pool.
//

#define ASSERT_EVENT_PAIR(E) {                        \
    ASSERT((E)->Type == EventPairObject);             \
}

#undef KeInitializeEvent

VOID
KeInitializeEvent (
    __out PRKEVENT Event,
    __in EVENT_TYPE Type,
    __in BOOLEAN State
    )

/*++

Routine Description:

    This function initializes a kernel event object. The initial signal
    state of the object is set to the specified value.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

    Type - Supplies the type of event; NotificationEvent or
        SynchronizationEvent.

    State - Supplies the initial signal state of the event object.

Return Value:

    None.

--*/

{

    //
    // Initialize standard dispatcher object header, set initial signal
    // state of event object, and set the type of event object.
    //

    Event->Header.Type = (UCHAR)Type;
    Event->Header.Size = sizeof(KEVENT) / sizeof(LONG);
    Event->Header.SignalState = State;
    InitializeListHead(&Event->Header.WaitListHead);
    return;
}

VOID
KeInitializeEventPair (
    __inout PKEVENT_PAIR EventPair
    )

/*++

Routine Description:

    This function initializes a kernel event pair object. A kernel event
    pair object contains two separate synchronization event objects that
    are used to provide a fast interprocess synchronization capability.

Arguments:

    EventPair - Supplies a pointer to a control object of type event pair.

Return Value:

    None.

--*/

{

    //
    // Initialize the type and size of the event pair object and initialize
    // the two event object as synchronization events with an initial state
    // of FALSE.
    //

    EventPair->Type = (USHORT)EventPairObject;
    EventPair->Size = sizeof(KEVENT_PAIR);
    KeInitializeEvent(&EventPair->EventLow, SynchronizationEvent, FALSE);
    KeInitializeEvent(&EventPair->EventHigh, SynchronizationEvent, FALSE);
    return;
}

VOID
KeClearEvent (
    __inout PRKEVENT Event
    )

/*++

Routine Description:

    This function clears the signal state of an event object.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

Return Value:

    None.

--*/

{

    ASSERT_EVENT(Event);

    //
    // Clear signal state of event object.
    //

    Event->Header.SignalState = 0;
    return;
}

LONG
KePulseEvent (
    __inout PRKEVENT Event,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    This function atomically sets the signal state of an event object to
    signaled, attempts to satisfy as many waits as possible, and then resets
    the signal state of the event object to Not-Signaled. The previous signal
    state of the event object is returned as the function value.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

    Increment - Supplies the priority increment that is to be applied
       if setting the event causes a Wait to be satisfied.

    Wait - Supplies a boolean value that signifies whether the call to
       KePulseEvent will be immediately followed by a call to one of the
       kernel Wait functions.

Return Value:

    The previous signal state of the event object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // If the current state of the event object is Not-Signaled and
    // the wait queue is not empty, then set the state of the event
    // to Signaled, satisfy as many Waits as possible, and then reset
    // the state of the event to Not-Signaled.
    //

    OldState = ReadForWriteAccess(&Event->Header.SignalState);
    if ((OldState == 0) &&
        (IsListEmpty(&Event->Header.WaitListHead) == FALSE)) {

        Event->Header.SignalState = 1;
        KiWaitTest(Event, Increment);
    }

    Event->Header.SignalState = 0;

    //
    // If the value of the Wait argument is TRUE, then return to the
    // caller with IRQL raised and the dispatcher database locked. Else
    // release the dispatcher database lock and lower IRQL to the
    // previous value.
    //

    if (Wait != FALSE) {
        Thread = KeGetCurrentThread();
        Thread->WaitIrql = OldIrql;
        Thread->WaitNext = Wait;

    } else {
       KiUnlockDispatcherDatabase(OldIrql);
    }

    //
    // Return previous signal state of event object.
    //

    return OldState;
}

LONG
KeReadStateEvent (
    __in PRKEVENT Event
    )

/*++

Routine Description:

    This function reads the current signal state of an event object.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

Return Value:

    The current signal state of the event object.

--*/

{

    ASSERT_EVENT(Event);

    //
    // Return current signal state of event object.
    //

    return Event->Header.SignalState;
}

LONG
KeResetEvent (
    __inout PRKEVENT Event
    )

/*++

Routine Description:

    This function resets the signal state of an event object to
    Not-Signaled. The previous state of the event object is returned
    as the function value.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

Return Value:

    The previous signal state of the event object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // Capture the current signal state of event object and then reset
    // the state of the event object to Not-Signaled.
    //

    OldState = ReadForWriteAccess(&Event->Header.SignalState);
    Event->Header.SignalState = 0;

    //
    // Unlock the dispatcher database and lower IRQL to its previous
    // value.

    KiUnlockDispatcherDatabase(OldIrql);

    //
    // Return previous signal state of event object.
    //

    return OldState;
}

LONG
KeSetEvent (
    __inout PRKEVENT Event,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    This function sets the signal state of an event object to signaled
    and attempts to satisfy as many waits as possible. The previous
    signal state of the event object is returned as the function value.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

    Increment - Supplies the priority increment that is to be applied
       if setting the event causes a Wait to be satisfied.

    Wait - Supplies a boolean value that signifies whether the call to
       set event will be immediately followed by a call to one of the
       kernel Wait functions.

Return Value:

    The previous signal state of the event object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_EVENT(Event);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the event is a notification event, the event is already signaled,
    // and wait is false, then there is no need to set the event.
    //

    if ((Event->Header.Type == EventNotificationObject) &&
        (Event->Header.SignalState == 1) &&
        (Wait == FALSE)) {

        return 1;
    }

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // Capture the old state and set the new state to signaled.
    //
    // If the old state is not-signaled and the wait list is not empty,
    // then satisfy as many waits as possible.
    //

    OldState = ReadForWriteAccess(&Event->Header.SignalState);
    Event->Header.SignalState = 1;
    if ((OldState == 0) &&
        (IsListEmpty(&Event->Header.WaitListHead) == FALSE)) {

        if (Event->Header.Type == EventNotificationObject) {
            KiWaitTestWithoutSideEffects(Event, Increment);

        } else {
            KiWaitTestSynchronizationObject(Event, Increment);
        }
    }

    //
    // If the value of the Wait argument is TRUE, then return to the
    // caller with IRQL raised and the dispatcher database locked. Else
    // release the dispatcher database lock and lower IRQL to its
    // previous value.
    //

    if (Wait != FALSE) {
       Thread = KeGetCurrentThread();
       Thread->WaitNext = Wait;
       Thread->WaitIrql = OldIrql;

    } else {
       KiUnlockDispatcherDatabase(OldIrql);
    }

    //
    // Return previous signal state of event object.
    //

    return OldState;
}

VOID
KeSetEventBoostPriority (
    __inout PRKEVENT Event,
    __in_opt PRKTHREAD *Thread
    )

/*++

Routine Description:

    This function conditionally sets the signal state of an event object
    to signaled, and attempts to unwait the first waiter, and optionally
    returns the thread address of the unwaited thread.

    N.B. This function can only be called with synchronization events.

Arguments:

    Event - Supplies a pointer to a dispatcher object of type event.

    Thread - Supplies an optional pointer to a variable that receives
        the address of the thread that is awakened.

Return Value:

    None.

--*/

{

    PKTHREAD CurrentThread;
    KIRQL OldIrql;
    PKWAIT_BLOCK WaitBlock;
    PRKTHREAD WaitThread;

    ASSERT(Event->Header.Type == SynchronizationEvent);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //

    CurrentThread = KeGetCurrentThread();
    KiLockDispatcherDatabase(&OldIrql);

    //
    // If the the wait list is not empty, then satisfy the wait of the
    // first thread in the wait list. Otherwise, set the signal state
    // of the event object.
    //

    if (IsListEmpty(&Event->Header.WaitListHead) != FALSE) {
        Event->Header.SignalState = 1;

    } else {

        //
        // Get the address of the first wait block in the event list.
        // If the wait is a wait any, then set the state of the event
        // to signaled and attempt to satisfy as many waits as possible.
        // Otherwise, unwait the first thread and apply an appropriate
        // priority boost to help prevent lock convoys from forming.
        //
        // N.B. Internal calls to this function for resource and fast
        //      mutex boosts NEVER call with a possibility of having
        //      a wait type of WaitAll. Calls from the NT service to
        //      set event and boost priority are restricted as to the
        //      event type, but not the wait type.
        //

        WaitBlock = CONTAINING_RECORD(Event->Header.WaitListHead.Flink,
                                      KWAIT_BLOCK,
                                      WaitListEntry);

        if (WaitBlock->WaitType == WaitAll) {
            Event->Header.SignalState = 1;
            KiWaitTestSynchronizationObject(Event, EVENT_INCREMENT);

        } else {

            //
            // Get the address of the waiting thread and return the address
            // if requested.
            //

            WaitThread = WaitBlock->Thread;
            if (ARGUMENT_PRESENT(Thread)) {
                *Thread = WaitThread;
            }

            //
            // Compute the new thread priority.
            //

            CurrentThread->Priority = KiComputeNewPriority(CurrentThread, 0);

            //
            // Unlink the thread from the appropriate wait queues and set
            // the wait completion status.
            //

            KiUnlinkThread(WaitThread, STATUS_SUCCESS);

            //
            // Set unwait priority adjustment parameters.
            //

            WaitThread->AdjustIncrement = CurrentThread->Priority;
            WaitThread->AdjustReason = (UCHAR)AdjustBoost;

            //
            // Ready the thread for execution.
            //

            KiReadyThread(WaitThread);
        }
    }

    //
    // Unlock dispatcher database lock and lower IRQL to its previous
    // value.
    //

    KiUnlockDispatcherDatabase(OldIrql);
    return;
}

