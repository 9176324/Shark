/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    queueobj.c

Abstract:

    This module implements the kernel queue object. Functions are provided
    to initialize, read, insert, and remove queue objects.

--*/

#include "ki.h"

VOID
KeInitializeQueue (
    __out PRKQUEUE Queue,
    __in ULONG Count
    )

/*++

Routine Description:

    This function initializes a kernel queue object.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type event.

    Count - Supplies the target maximum number of threads that should
        be concurrently active. If this parameter is specified as zero,
        then the number of processors is used.

Return Value:

    None.

--*/

{

    //
    // Initialize standard dispatcher object header and set initial
    // state of queue object.
    //

    Queue->Header.Type = QueueObject;
    Queue->Header.Size = sizeof(KQUEUE) / sizeof(LONG);
    Queue->Header.SignalState = 0;
    InitializeListHead(&Queue->Header.WaitListHead);

    //
    // Initialize queue listhead, the thread list head, the current number
    // of threads, and the target maximum number of threads.
    //

    InitializeListHead(&Queue->EntryListHead);
    InitializeListHead(&Queue->ThreadListHead);
    Queue->CurrentCount = 0;
    if (ARGUMENT_PRESENT(Count)) {
        Queue->MaximumCount = Count;

    } else {
        Queue->MaximumCount = KeNumberProcessors;
    }

    return;
}

LONG
KeReadStateQueue (
    __in PRKQUEUE Queue
    )

/*++

Routine Description:

    This function reads the current signal state of a Queue object.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

Return Value:

    The current signal state of the Queue object.

--*/

{

    ASSERT_QUEUE(Queue);

    return Queue->Header.SignalState;
}

LONG
KeInsertQueue (
    __inout PRKQUEUE Queue,
    __inout PLIST_ENTRY Entry
    )

/*++

Routine Description:

    This function inserts the specified entry in the queue object entry
    list and attempts to satisfy the wait of a single waiter.

    N.B. The wait discipline for Queue object is FIFO.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

    Entry - Supplies a pointer to a list entry that is inserted in the
        queue object entry list.

Return Value:

    The previous signal state of the Queue object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH level and lock the dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // Insert the specified entry in the queue object entry list.
    //

    OldState = KiInsertQueue(Queue, Entry, FALSE);

    //
    // Unlock the dispatcher database, exit the dispatcher, and return the
    // signal state of queue object.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return OldState;
}

LONG
KeInsertHeadQueue (
    __inout PRKQUEUE Queue,
    __inout PLIST_ENTRY Entry
    )

/*++

Routine Description:

    This function inserts the specified entry in the queue object entry
    list and attempts to satisfy the wait of a single waiter.

    N.B. The wait discipline for Queue object is LIFO.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

    Entry - Supplies a pointer to a list entry that is inserted in the
        queue object entry list.

Return Value:

    The previous signal state of the Queue object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH level and lock the dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // Insert the specified entry in the queue object entry list.
    //

    OldState = KiInsertQueue(Queue, Entry, TRUE);

    //
    // Unlock the dispatcher database, exit the dispatcher, and return the
    // signal state of queue object.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return OldState;
}

//
// The following macro initializes thread local variables for the wait
// for single object kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeRemoveQueue()                                             \
    Thread->WaitBlockList = WaitBlock;                                      \
    WaitBlock->Object = (PVOID)Queue;                                       \
    WaitBlock->WaitKey = (CSHORT)(STATUS_SUCCESS);                          \
    WaitBlock->WaitType = WaitAny;                                          \
    WaitBlock->Thread = Thread;                                             \
    Thread->WaitStatus = 0;                                                 \
    if (ARGUMENT_PRESENT(Timeout)) {                                        \
        KiSetDueTime(Timer, *Timeout, &Hand);                               \
        DueTime.QuadPart = Timer->DueTime.QuadPart;                         \
        WaitBlock->NextWaitBlock = WaitTimer;                               \
        WaitTimer->NextWaitBlock = WaitBlock;                               \
        Timer->Header.WaitListHead.Flink = &WaitTimer->WaitListEntry;       \
        Timer->Header.WaitListHead.Blink = &WaitTimer->WaitListEntry;       \
    } else {                                                                \
        WaitBlock->NextWaitBlock = WaitBlock;                               \
    }                                                                       \
    Thread->Alertable = FALSE;                                              \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = WrQueue;                                           \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime= KiQueryLowTickCount()

PLIST_ENTRY
KeRemoveQueue (
    __inout PRKQUEUE Queue,
    __in KPROCESSOR_MODE WaitMode,
    __in_opt PLARGE_INTEGER Timeout
    )

/*++

Routine Description:

    This function removes the next entry from the Queue object entry
    list. If no list entry is available, then the calling thread is
    put in a wait state.

    N.B. The wait discipline for Queue object LIFO.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

    Timeout - Supplies a pointer to an optional absolute of relative time over
        which the wait is to occur.

Return Value:

    The address of the entry removed from the Queue object entry list or
    STATUS_TIMEOUT.

    N.B. These values can easily be distinguished by the fact that all
         addresses in kernel mode have the high order bit set.

--*/

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    PLIST_ENTRY Entry;
    ULONG Hand;
    LARGE_INTEGER NewTime;
    PRKQUEUE OldQueue;
    PLARGE_INTEGER OriginalTime;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;
    LONG_PTR WaitStatus;
    PRKWAIT_BLOCK WaitTimer;

    ASSERT_QUEUE(Queue);

    //
    // Set constant variables.
    //

    Hand = 0;
    OriginalTime = Timeout;
    Thread = KeGetCurrentThread();
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[0];
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If the dispatcher database lock is already held, then initialize the
    // local variables. Otherwise, raise IRQL to SYNCH_LEVEL, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (Thread->WaitNext) {
        Thread->WaitNext = FALSE;
        InitializeRemoveQueue();

    } else {
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeRemoveQueue();
        KiLockDispatcherDatabaseAtSynchLevel();
    }

    //
    // Check if the thread is currently processing a queue entry and whether
    // the new queue is the same as the old queue.
    //

    OldQueue = Thread->Queue;
    Thread->Queue = Queue;
    if (Queue != OldQueue) {

        //
        // If the thread was previously associated with a queue, then remove
        // the thread from the old queue object thread list and attempt to
        // activate another thread.
        //

        Entry = &Thread->QueueListEntry;
        if (OldQueue != NULL) {
            RemoveEntryList(Entry);
            KiActivateWaiterQueue(OldQueue);
        }

        //
        // Insert thread in the thread list of the new queue that the thread
        // will be associate with.
        //

        InsertTailList(&Queue->ThreadListHead, Entry);

    } else {

        //
        // The previous and current queue are the same queue - decrement the
        // current number of threads.
        //

        Queue->CurrentCount -= 1;
    }

    //
    // Start of wait loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the
    // middle of the wait or a kernel APC is pending on the first attempt
    // through the loop.
    //
    // If the Queue object entry list is not empty, then remove the next
    // entry from the Queue object entry list. Otherwise, wait for an entry
    // to be inserted in the queue.
    //

    do {

        //
        // Check if there is a queue entry available and the current
        // number of active threads is less than target maximum number
        // of threads.
        //

        Entry = Queue->EntryListHead.Flink;
        if ((Entry != &Queue->EntryListHead) &&
            (Queue->CurrentCount < Queue->MaximumCount)) {

            //
            // Decrement the number of entries in the queue object entry list,
            // increment the number of active threads, remove the next entry
            // from the list, and set the forward link to NULL.
            //

            Queue->Header.SignalState -= 1;
            Queue->CurrentCount += 1;
            if ((Entry->Flink == NULL) || (Entry->Blink == NULL)) {
                KeBugCheckEx(INVALID_WORK_QUEUE_ITEM,
                             (ULONG_PTR)Entry,
                             (ULONG_PTR)Queue,
                             (ULONG_PTR)&ExWorkerQueue[0],
                             (ULONG_PTR)((PWORK_QUEUE_ITEM)Entry)->WorkerRoutine);
            }

            RemoveEntryList(Entry);
            Entry->Flink = NULL;
            break;

        } else {

            //
            // Test to determine if a kernel APC is pending.
            //
            // If a kernel APC is pending, the special APC disable count is
            // zero, and the previous IRQL was less than APC_LEVEL, then a
            // kernel APC was queued by another processor just after IRQL was
            // raised to DISPATCH_LEVEL, but before the dispatcher database
            // was locked.
            //
            // N.B. This can only happen in a multiprocessor system.
            //

            if (Thread->ApcState.KernelApcPending &&
                (Thread->SpecialApcDisable == 0) &&
                (Thread->WaitIrql < APC_LEVEL)) {

                //
                // Increment the current thread count, unlock the dispatcher
                // database, and exit the dispatcher. An APC interrupt will 
                // immediately occur which will result in the delivery of the
                // kernel APC, if possible.
                //

                Queue->CurrentCount += 1;
                KiUnlockDispatcherDatabaseFromSynchLevel();
                KiExitDispatcher(Thread->WaitIrql);

            } else {

                //
                // Test if a user APC is pending.
                //

                if ((WaitMode != KernelMode) && (Thread->ApcState.UserApcPending)) {
                    Entry = (PLIST_ENTRY)ULongToPtr(STATUS_USER_APC);
                    Queue->CurrentCount += 1;
                    break;
                }

                //
                // Check to determine if a timeout value is specified.
                //

                if (ARGUMENT_PRESENT(Timeout)) {

                    //
                    // Check if the timer has already expired.
                    //
                    // N.B. The constant fields of the timer wait block are
                    //      initialized when the thread is initialized. The
                    //      constant fields include the wait object, wait key,
                    //      wait type, and the wait list entry link pointers.
                    //

                    if (KiCheckDueTime(Timer) == FALSE) {
                        Entry = (PLIST_ENTRY)ULongToPtr(STATUS_TIMEOUT);
                        Queue->CurrentCount += 1;
                        break;
                    }
                }

                //
                // Insert wait block in object wait list.
                //

                InsertTailList(&Queue->Header.WaitListHead, &WaitBlock->WaitListEntry);

                //
                // Set the thread wait parameters, set the thread dispatcher
                // state to Waiting, and insert the thread in the wait list.
                //

                CurrentPrcb = KeGetCurrentPrcb();
                Thread->State = Waiting;
                if (StackSwappable != FALSE) {
                    InsertTailList(&CurrentPrcb->WaitListHead, &Thread->WaitListEntry);
                }

                //
                // Set swap busy for the current thread, unlock the dispatcher
                // database, and switch to a new thread.
                //
                // Control is returned at the original IRQL.
                //

                ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

                KiSetContextSwapBusy(Thread);

                if (ARGUMENT_PRESENT(Timeout)) {
                    KiInsertOrSignalTimer(Timer, Hand);

                } else {
                    KiUnlockDispatcherDatabaseFromSynchLevel();
                }

                WaitStatus = KiSwapThread(Thread, CurrentPrcb);

                //
                // If the thread was not awakened to deliver a kernel mode APC,
                // then return wait status.
                //

                Thread->WaitReason = 0;
                if (WaitStatus != STATUS_KERNEL_APC) {
                    return (PLIST_ENTRY)WaitStatus;
                }

                if (ARGUMENT_PRESENT(Timeout)) {

                    //
                    // Reduce the amount of time remaining before timeout occurs.
                    //

                    Timeout = KiComputeWaitInterval(OriginalTime,
                                                    &DueTime,
                                                    &NewTime);
                }
            }

            //
            // Raise IRQL to SYNCH level, initialize the local variables,
            // lock the dispatcher database, and decrement the count of
            // active threads.
            //

            Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
            InitializeRemoveQueue();
            KiLockDispatcherDatabaseAtSynchLevel();
            Queue->CurrentCount -= 1;
        }

    } while (TRUE);

    //
    // Unlock the dispatcher database, exit the dispatcher, and return the
    // list entry address or a status of timeout.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(Thread->WaitIrql);
    return Entry;
}

PLIST_ENTRY
KeRundownQueue (
    __inout PRKQUEUE Queue
    )

/*++

Routine Description:

    This function runs down the specified queue by removing the listhead
    from the queue list, removing any associated threads from the thread
    list, and returning the address of the first entry.


Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

Return Value:

    If the queue list is not empty, then the address of the first entry in
    the queue is returned as the function value. Otherwise, a value of NULL
    is returned.

--*/

{

    PLIST_ENTRY Entry;
    PLIST_ENTRY FirstEntry;
    KIRQL OldIrql;
    PKTHREAD Thread;

    ASSERT_QUEUE(Queue);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(IsListEmpty(&Queue->Header.WaitListHead));

    //
    // Raise IRQL to SYNCH level and lock the dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // Get the address of the first entry in the queue and check if the
    // list is empty or contains entries that should be flushed. If there
    // are no entries in the list, then set the return value to NULL.
    // Otherwise, set the return value to the address of the first list
    // entry and remove the listhead from the list.
    //

    FirstEntry = Queue->EntryListHead.Flink;
    if (FirstEntry == &Queue->EntryListHead) {
        FirstEntry = NULL;

    } else {
        RemoveEntryList(&Queue->EntryListHead);
    }

    //
    // Remove all associated threads from the thread list of the queue.
    //

    while (Queue->ThreadListHead.Flink != &Queue->ThreadListHead) {
        Entry = Queue->ThreadListHead.Flink;
        Thread = CONTAINING_RECORD(Entry, KTHREAD, QueueListEntry);
        Thread->Queue = NULL;
        RemoveEntryList(Entry);
    }

#if DBG

    Queue->EntryListHead.Flink = Queue->EntryListHead.Blink = NULL;
    Queue->ThreadListHead.Flink = Queue->ThreadListHead.Blink = NULL;
    Queue->Header.WaitListHead.Flink = Queue->Header.WaitListHead.Blink = NULL;

#endif

    //
    // Unlock the dispatcher database, exit the dispatcher, and return the
    // function value.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KiExitDispatcher(OldIrql);
    return FirstEntry;
}

LONG
FASTCALL
KiInsertQueue (
    __inout PRKQUEUE Queue,
    __inout PLIST_ENTRY Entry,
    __in BOOLEAN Head
    )

/*++

Routine Description:

    This function inserts the specified entry in the queue object entry
    list and attempts to satisfy the wait of a single waiter.

    N.B. The wait discipline for Queue object is LIFO.

Arguments:

    Queue - Supplies a pointer to a dispatcher object of type Queue.

    Entry - Supplies a pointer to a list entry that is inserted in the
        queue object entry list.

    Head - Supplies a boolean value that determines whether the queue
        entry is inserted at the head or tail of the queue if it can
        not be immediately dispatched.

Return Value:

    The previous signal state of the Queue object.

--*/

{

    LONG OldState;
    PRKTHREAD Thread;
    PKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    PLIST_ENTRY WaitEntry;

    ASSERT_QUEUE(Queue);

    //
    // Capture the current signal state of queue object and check if there
    // is a thread waiting on the queue object, the current number of active
    // threads is less than the target number of threads, and the wait reason
    // of the current thread is not queue wait or the wait queue is not the
    // same queue as the insertion queue. If these conditions are satisfied,
    // then satisfy the thread wait and pass the thread the address of the
    // queue entry as the wait status. Otherwise, set the state of the queue
    // object to signaled and insert the specified entry in the queue object
    // entry list.
    //

    OldState = Queue->Header.SignalState;
    Thread = KeGetCurrentThread();
    WaitEntry = Queue->Header.WaitListHead.Blink;
    if ((WaitEntry != &Queue->Header.WaitListHead) &&
        (Queue->CurrentCount < Queue->MaximumCount) &&
        ((Thread->Queue != Queue) ||
        (Thread->WaitReason != WrQueue))) {

        //
        // Remove the last wait block from the wait list and get the address
        // of the waiting thread object.
        //

        RemoveEntryList(WaitEntry);
        WaitBlock = CONTAINING_RECORD(WaitEntry, KWAIT_BLOCK, WaitListEntry);
        Thread = WaitBlock->Thread;

        //
        // Set the wait completion status, remove the thread from its wait
        // list, increment the number of active threads, and clear the wait
        // reason.
        //

        Thread->WaitStatus = (LONG_PTR)Entry;
        if (Thread->WaitListEntry.Flink != NULL) {
            RemoveEntryList(&Thread->WaitListEntry);
        }

        Queue->CurrentCount += 1;
        Thread->WaitReason = 0;

        //
        // If thread timer is still active, then cancel thread timer.
        //

        Timer = &Thread->Timer;
        if (Timer->Header.Inserted == TRUE) {
            KiRemoveTreeTimer(Timer);
        }

        //
        // Ready the thread for execution.
        //

        KiReadyThread(Thread);

    } else {
        Queue->Header.SignalState += 1;
        if (Head != FALSE) {
            InsertHeadList(&Queue->EntryListHead, Entry);

        } else {
            InsertTailList(&Queue->EntryListHead, Entry);
        }
    }

    return OldState;
}

