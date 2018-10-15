/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    wait.c

Abstract:

    This module implements the generic kernel wait routines. Functions
    are provided to delay execution, wait for multiple objects, wait for
    a single object, and ot set a client event and wait for a server event.

    N.B. This module is written to be a fast as possible and not as small
        as possible. Therefore some code sequences are duplicated to avoid
        procedure calls. It would also be possible to combine wait for
        single object into wait for multiple objects at the cost of some
        speed. Since wait for single object is the most common case, the
        two routines have been separated.

--*/

#include "ki.h"

#pragma alloc_text(PAGE, KeIsWaitListEmpty)

//
// Test for alertable condition.
//
// If alertable is TRUE and the thread is alerted for a processor
// mode that is equal to the wait mode, then return immediately
// with a wait completion status of ALERTED.
//
// Else if alertable is TRUE, the wait mode is user, and the user APC
// queue is not empty, then set user APC pending, and return immediately
// with a wait completion status of USER_APC.
//
// Else if alertable is TRUE and the thread is alerted for kernel
// mode, then return immediately with a wait completion status of
// ALERTED.
//
// Else if alertable is FALSE and the wait mode is user and there is a
// user APC pending, then return immediately with a wait completion
// status of USER_APC.
//

#define TestForAlertPending(Alertable) \
    if (Alertable) { \
        if (Thread->Alerted[WaitMode] != FALSE) { \
            Thread->Alerted[WaitMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } else if ((WaitMode != KernelMode) && \
                  (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode])) == FALSE) { \
            Thread->ApcState.UserApcPending = TRUE; \
            WaitStatus = STATUS_USER_APC; \
            break; \
        } else if (Thread->Alerted[KernelMode] != FALSE) { \
            Thread->Alerted[KernelMode] = FALSE; \
            WaitStatus = STATUS_ALERTED; \
            break; \
        } \
    } else if (Thread->ApcState.UserApcPending & WaitMode) { \
        WaitStatus = STATUS_USER_APC; \
        break; \
    }

VOID
KiAdjustQuantumThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    If the current thread is not a time critical or real time thread, then
    adjust its quantum in accordance with the adjustment that would have
    occurred if the thread had actually waited.

    N.B. This routine is entered at SYNCH_LEVEL and exits at the wait
         IRQL of the subject thread after having exited the scheduler.

Arguments:

    Thread - Supplies a pointer to the current thread.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;
    PKTHREAD NewThread;

    //
    // Acquire the thread lock and the PRCB lock.
    //
    // If the thread is not a real time or time critical thread, then adjust
    // the thread quantum.
    //

    Prcb = KeGetCurrentPrcb();
    KiAcquireThreadLock(Thread);
    KiAcquirePrcbLock(Prcb);
    if ((Thread->Priority < LOW_REALTIME_PRIORITY) &&
        (Thread->BasePriority < TIME_CRITICAL_PRIORITY_BOUND)) {

        Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
        if (Thread->Quantum <= 0) {

            //
            // Quantum end has occurred. Adjust the thread priority.
            //

            Thread->Quantum = Thread->QuantumReset;

            //
            // Compute the new thread priority and attempt to reschedule the
            // current processor as if a quantum end had occurred.
            //
            // N.B. The new priority will never be greater than the previous
            //      priority.
            //

            Thread->Priority = KiComputeNewPriority(Thread, 1);
            if (Prcb->NextThread == NULL) {
                if ((NewThread = KiSelectReadyThread(Thread->Priority, Prcb)) != NULL) {
                    NewThread->State = Standby;
                    Prcb->NextThread = NewThread;
                }

            } else {
                Thread->Preempted = FALSE;
            }
        }
    }

    //
    // Release the thread lock, release the PRCB lock, exit the scheduler,
    // and return.
    //

    KiReleasePrcbLock(Prcb);
    KiReleaseThreadLock(Thread);
    KiExitDispatcher(Thread->WaitIrql);
    return;
}

FORCEINLINE
PLARGE_INTEGER
FASTCALL
KiComputeWaitInterval (
    IN PLARGE_INTEGER OriginalTime,
    IN PLARGE_INTEGER DueTime,
    IN OUT PLARGE_INTEGER NewTime
    )

/*++

Routine Description:

    This function recomputes the wait interval after a thread has been
    awakened to deliver a kernel APC.

Arguments:

    OriginalTime - Supplies a pointer to the original timeout value.

    DueTime - Supplies a pointer to the previous due time.

    NewTime - Supplies a pointer to a variable that receives the
        recomputed wait interval.

Return Value:

    A pointer to the new time is returned as the function value.

--*/

{

    //
    // If the original wait time was absolute, then return the same
    // absolute time. Otherwise, reduce the wait time remaining before
    // the time delay expires.
    //

    if (OriginalTime->HighPart >= 0) {
        return OriginalTime;

    } else {
        KiQueryInterruptTime(NewTime);
        NewTime->QuadPart -= DueTime->QuadPart;
        return NewTime;
    }
}

//
// The following macro initializes thread local variables for the delay
// execution thread kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeDelayExecution()                                          \
    Thread->WaitBlockList = WaitBlock;                                      \
    Thread->WaitStatus = 0;                                                 \
    KiSetDueTime(Timer, *Interval, &Hand);                                  \
    DueTime.QuadPart = Timer->DueTime.QuadPart;                             \
    WaitBlock->NextWaitBlock = WaitBlock;                                   \
    Timer->Header.WaitListHead.Flink = &WaitBlock->WaitListEntry;           \
    Timer->Header.WaitListHead.Blink = &WaitBlock->WaitListEntry;           \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = DelayExecution;                                    \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
DECLSPEC_NOINLINE
KeDelayExecutionThread (
    __in KPROCESSOR_MODE WaitMode,
    __in BOOLEAN Alertable,
    __in PLARGE_INTEGER Interval
    )

/*++

Routine Description:

    This function delays the execution of the current thread for the specified
    interval of time.

Arguments:

    WaitMode  - Supplies the processor mode in which the delay is to occur.

    Alertable - Supplies a boolean value that specifies whether the delay
        is alertable.

    Interval - Supplies a pointer to the absolute or relative time over which
        the delay is to occur.

Return Value:

    The wait completion status. A value of STATUS_SUCCESS is returned if
    the delay occurred. A value of STATUS_ALERTED is returned if the wait
    was aborted to deliver an alert to the current thread. A value of
    STATUS_USER_APC is returned if the wait was aborted to deliver a user
    APC to the current thread.

--*/

{

    LARGE_INTEGER DueTime;
    ULONG Hand;
    LARGE_INTEGER NewTime;
    PLARGE_INTEGER OriginalTime;
    PKPRCB Prcb;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;

    //
    // If the specfied wait interval is zero and the wait mode is not kernel
    // mode, then attempt to yield execution.
    //

    Hand = 0;
    Thread = KeGetCurrentThread();
    if ((Interval->QuadPart == 0) &&
        (WaitMode != KernelMode)) {

        if ((Alertable == FALSE) &&
            (Thread->ApcState.UserApcPending == FALSE)) {

            return NtYieldExecution();
        }
    }

    //
    // Set constant variables.
    //

    OriginalTime = Interval;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (ReadForWriteAccess(&Thread->WaitNext) == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeDelayExecution();

    //
    // Start of delay loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the delay or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. This can only happen in a multiprocessor system.
        //

        Thread->Preempted = FALSE;
        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

            //
            // Check if the timer has already expired.
            //
            // N.B. The constant fields of the timer wait block are
            //      initialized when the thread is initialized. The
            //      constant fields include the wait object, wait key,
            //      wait type, and the wait list entry link pointers.
            //

            Prcb = KeGetCurrentPrcb();
            if (KiCheckDueTime(Timer) == FALSE) {
                goto NoWait;
            }

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher
            // state to Waiting, and insert the thread in the wait list if
            // the kernel stack of the current thread is swappable.
            //

            Thread->State = Waiting;
            if (StackSwappable != FALSE) {
                InsertTailList(&Prcb->WaitListHead, &Thread->WaitListEntry);
            }

            //
            // Set swap busy for the current thread, unlock the dispatcher
            // database, and switch to a new thread.
            //
            // Control is returned at the original IRQL.
            //

            ASSERT(Thread->WaitIrql <= DISPATCH_LEVEL);

            KiSetContextSwapBusy(Thread);
            KiInsertOrSignalTimer(Timer, Hand);
            WaitStatus = (NTSTATUS)KiSwapThread(Thread, Prcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return the wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                if (WaitStatus == STATUS_TIMEOUT) {
                    WaitStatus = STATUS_SUCCESS;
                }

                return WaitStatus;
            }

            //
            // Reduce the time remaining before the time delay expires.
            //

            Interval = KiComputeWaitInterval(OriginalTime,
                                             &DueTime,
                                             &NewTime);
        }

        //
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:

        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeDelayExecution();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return the
    // wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // If the wait time is zero, then unlock the dispatcher database and
    // yield execution. Otherwise, unlock the dispatcher database, remain
    // at SYNCH level, adjust the thread quantum, exit the dispatcher, and
    // and return the wait completion status.
    //

NoWait:

    if (Interval->QuadPart == 0) {
        KiUnlockDispatcherDatabase(Thread->WaitIrql);
        return NtYieldExecution();

    } else {
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KiAdjustQuantumThread(Thread);
        return STATUS_SUCCESS;
    }
}

BOOLEAN
KeIsWaitListEmpty (
    __in PVOID Object
    )

/*++

Routine Description:

    This function tests to determine if the wait list of the specified
    dispatcher object is empty.

Arguments:

    Object - Supplies a pointer to a dispatcher object.

Return Value:

    If the wait list of the specified dispatcher object is empty, then a value
    of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    PKEVENT Event = Object;
    PLIST_ENTRY ListHead;

    ListHead = &Event->Header.WaitListHead;
    KeMemoryBarrier();
    return (BOOLEAN)(ListHead == ListHead->Flink);
}

//
// The following macro initializes thread local variables for the wait
// for multiple objects kernel service while context switching is disabled.
//
// N.B. IRQL must be raised to DPC level prior to the invocation of this
//      macro.
//
// N.B. Initialization is done in this manner so this code does not get
//      executed inside the dispatcher lock.
//

#define InitializeWaitMultiple()                                            \
    Thread->WaitBlockList = WaitBlockArray;                                 \
    Index = 0;                                                              \
    do {                                                                    \
        WaitBlock = &WaitBlockArray[Index];                                 \
        WaitBlock->Object = Object[Index];                                  \
        WaitBlock->WaitKey = (CSHORT)(Index);                               \
        WaitBlock->WaitType = WaitType;                                     \
        WaitBlock->Thread = Thread;                                         \
        WaitBlock->NextWaitBlock = &WaitBlockArray[Index + 1];              \
        Index += 1;                                                         \
    } while (Index < Count);                                                \
    WaitBlock->NextWaitBlock = &WaitBlockArray[0];                          \
    Thread->WaitStatus = 0;                                                 \
    if (ARGUMENT_PRESENT(Timeout)) {                                        \
        WaitTimer->NextWaitBlock = &WaitBlockArray[0];                      \
        KiSetDueTime(Timer, *Timeout, &Hand);                               \
        DueTime.QuadPart = Timer->DueTime.QuadPart;                         \
        InitializeListHead(&Timer->Header.WaitListHead);                    \
    }                                                                       \
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForMultipleObjects (
    __in ULONG Count,
    __in_ecount(Count) PVOID Object[],
    __in WAIT_TYPE WaitType,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout,
    __out_opt PKWAIT_BLOCK WaitBlockArray
    )

/*++

Routine Description:

    This function waits until the specified objects attain a state of
    Signaled. The wait can be specified to wait until all of the objects
    attain a state of Signaled or until one of the objects attains a state
    of Signaled. An optional timeout can also be specified. If a timeout
    is not specified, then the wait will not be satisfied until the objects
    attain a state of Signaled. If a timeout is specified, and the objects
    have not attained a state of Signaled when the timeout expires, then
    the wait is automatically satisfied. If an explicit timeout value of
    zero is specified, then no wait will occur if the wait cannot be satisfied
    immediately. The wait can also be specified as alertable.

Arguments:

    Count - Supplies a count of the number of objects that are to be waited
        on.

    Object[] - Supplies an array of pointers to dispatcher objects.

    WaitType - Supplies the type of wait to perform (WaitAll, WaitAny).

    WaitReason - Supplies the reason for the wait.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

    Alertable - Supplies a boolean value that specifies whether the wait is
        alertable.

    Timeout - Supplies a pointer to an optional absolute of relative time over
        which the wait is to occur.

    WaitBlockArray - Supplies an optional pointer to an array of wait blocks
        that are to used to describe the wait operation.

Return Value:

    The wait completion status. A value of STATUS_TIMEOUT is returned if a
    timeout occurred. The index of the object (zero based) in the object
    pointer array is returned if an object satisfied the wait. A value of
    STATUS_ALERTED is returned if the wait was aborted to deliver an alert
    to the current thread. A value of STATUS_USER_APC is returned if the
    wait was aborted to deliver a user APC to the current thread.

--*/

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    ULONG Hand;
    ULONG_PTR Index;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PRKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

    //
    // Set constant variables.
    //

    Hand = 0;
    Thread = KeGetCurrentThread();
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If a wait block array has been specified, then the maximum number of
    // objects that can be waited on is specified by MAXIMUM_WAIT_OBJECTS.
    // Otherwise the builtin wait blocks in the thread object are used and
    // the maximum number of objects that can be waited on is specified by
    // THREAD_WAIT_OBJECTS. If the specified number of objects is not within
    // limits, then bugcheck.
    //

    if (ARGUMENT_PRESENT(WaitBlockArray)) {
        if (Count > MAXIMUM_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

    } else {
        if (Count > THREAD_WAIT_OBJECTS) {
            KeBugCheck(MAXIMUM_WAIT_OBJECTS_EXCEEDED);
        }

        WaitBlockArray = &Thread->WaitBlock[0];
    }

    ASSERT(Count != 0);

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (ReadForWriteAccess(&Thread->WaitNext) == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitMultiple();

    //
    // Start of wait loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the wait or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. This can only happen in a multiprocessor system.
        //

        Thread->Preempted = FALSE;
        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // Construct wait blocks and check to determine if the wait is
            // already satisfied. If the wait is satisfied, then perform
            // wait completion and return. Else put current thread in a wait
            // state if an explicit timeout value of zero is not specified.
            //

            Index = 0;
            if (WaitType == WaitAny) {
                do {

                    //
                    // Test if wait can be satisfied immediately.
                    //
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                    //
                    // If the object is a mutant object and the mutant object
                    // has been recursively acquired MINLONG times, then raise
                    // an exception. Otherwise if the signal state of the mutant
                    // object is greater than zero, or the current thread is
                    // the owner of the mutant object, then satisfy the wait.
                    //

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Objectx->Header.SignalState > 0) ||
                            (Thread == Objectx->OwnerThread)) {
                            if (Objectx->Header.SignalState != MINLONG) {
                                KiWaitSatisfyMutant(Objectx, Thread);
                                WaitStatus = (NTSTATUS)(Index | Thread->WaitStatus);
                                goto NoWait;

                            } else {
                                KiUnlockDispatcherDatabase(Thread->WaitIrql);
                                ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                            }
                        }

                    //
                    // If the signal state is greater than zero, then satisfy
                    // the wait.
                    //

                    } else if (Objectx->Header.SignalState > 0) {
                        KiWaitSatisfyOther(Objectx);
                        WaitStatus = (NTSTATUS)(Index);
                        goto NoWait;
                    }

                    Index += 1;

                } while(Index < Count);

            } else {
                do {

                    //
                    // Test if wait can be satisfied.
                    //
    
                    Objectx = (PKMUTANT)Object[Index];
    
                    ASSERT(Objectx->Header.Type != QueueObject);
    
                    //
                    // If the object is a mutant object and the mutant object
                    // has been recursively acquired MINLONG times, then raise
                    // an exception. Otherwise if the signal state of the mutant
                    // object is less than or equal to zero and the current
                    // thread is not the  owner of the mutant object, then the
                    // wait cannot be satisfied.
                    //

                    if (Objectx->Header.Type == MutantObject) {
                        if ((Thread == Objectx->OwnerThread) &&
                            (Objectx->Header.SignalState == MINLONG)) {
                            KiUnlockDispatcherDatabase(Thread->WaitIrql);
                            ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);

                        } else if ((Objectx->Header.SignalState <= 0) &&
                                  (Thread != Objectx->OwnerThread)) {
                            break;
                        }

                    //
                    // If the signal state is less than or equal to zero, then
                    // the wait cannot be satisfied.
                    //

                    } else if (Objectx->Header.SignalState <= 0) {
                        break;
                    }

                    Index += 1;

                } while(Index < Count);

                //
                // If all objects have been scanned, then satisfy the wait.
                //

                if (Index == Count) {
                    WaitBlock = &WaitBlockArray[0];
                    do {
                        Objectx = (PKMUTANT)WaitBlock->Object;
                        KiWaitSatisfyAny(Objectx, Thread);
                        WaitBlock = WaitBlock->NextWaitBlock;
                    } while (WaitBlock != &WaitBlockArray[0]);

                    WaitStatus = (NTSTATUS)Thread->WaitStatus;
                    goto NoWait;
                }
            }

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

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
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }

                WaitBlock->NextWaitBlock = WaitTimer;
            }

            //
            // Insert wait blocks in object wait lists.
            //

            WaitBlock = &WaitBlockArray[0];
            do {
                Objectx = (PKMUTANT)WaitBlock->Object;
                InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);
                WaitBlock = WaitBlock->NextWaitBlock;
            } while (WaitBlock != &WaitBlockArray[0]);

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher state
            // to Waiting, and insert the thread in the wait list.
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

            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return the wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
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
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitMultiple();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return
    // the wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // Unlock the dispatcher database and remain at SYNCH level.
    //

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

    //
    // Adjust the thread quantum, exit the scheduler, and return the wait
    // completion status.
    //

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
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

#define InitializeWaitSingle()                                              \
    Thread->WaitBlockList = WaitBlock;                                      \
    WaitBlock->Object = Object;                                             \
    WaitBlock->WaitKey = (CSHORT)(STATUS_SUCCESS);                          \
    WaitBlock->WaitType = WaitAny;                                          \
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
    Thread->Alertable = Alertable;                                          \
    Thread->WaitMode = WaitMode;                                            \
    Thread->WaitReason = (UCHAR)WaitReason;                                 \
    Thread->WaitListEntry.Flink = NULL;                                     \
    StackSwappable = KiIsKernelStackSwappable(WaitMode, Thread);            \
    Thread->WaitTime = KiQueryLowTickCount()

NTSTATUS
KeWaitForSingleObject (
    __in PVOID Object,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    )

/*++

Routine Description:

    This function waits until the specified object attains a state of
    Signaled. An optional timeout can also be specified. If a timeout
    is not specified, then the wait will not be satisfied until the object
    attains a state of Signaled. If a timeout is specified, and the object
    has not attained a state of Signaled when the timeout expires, then
    the wait is automatically satisfied. If an explicit timeout value of
    zero is specified, then no wait will occur if the wait cannot be satisfied
    immediately. The wait can also be specified as alertable.

Arguments:

    Object - Supplies a pointer to a dispatcher object.

    WaitReason - Supplies the reason for the wait.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

    Alertable - Supplies a boolean value that specifies whether the wait is
        alertable.

    Timeout - Supplies a pointer to an optional absolute of relative time over
        which the wait is to occur.

Return Value:

    The wait completion status. A value of STATUS_TIMEOUT is returned if a
    timeout occurred. A value of STATUS_SUCCESS is returned if the specified
    object satisfied the wait. A value of STATUS_ALERTED is returned if the
    wait was aborted to deliver an alert to the current thread. A value of
    STATUS_USER_APC is returned if the wait was aborted to deliver a user
    APC to the current thread.

--*/

{

    PKPRCB CurrentPrcb;
    LARGE_INTEGER DueTime;
    ULONG Hand;
    LARGE_INTEGER NewTime;
    PKMUTANT Objectx;
    PLARGE_INTEGER OriginalTime;
    PRKQUEUE Queue;
    LOGICAL StackSwappable;
    PRKTHREAD Thread;
    PRKTIMER Timer;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;
    PKWAIT_BLOCK WaitTimer;

    ASSERT((PsGetCurrentThread()->StartAddress != (PVOID)(ULONG_PTR)KeBalanceSetManager) || (ARGUMENT_PRESENT(Timeout)));

    //
    // Set constant variables.
    //

    Hand = 0;
    Thread = KeGetCurrentThread();
    Objectx = (PKMUTANT)Object;
    OriginalTime = Timeout;
    Timer = &Thread->Timer;
    WaitBlock = &Thread->WaitBlock[0];
    WaitTimer = &Thread->WaitBlock[TIMER_WAIT_BLOCK];

    //
    // If the dispatcher database is already held, then initialize the thread
    // local variables. Otherwise, raise IRQL to DPC level, initialize the
    // thread local variables, and lock the dispatcher database.
    //

    if (ReadForWriteAccess(&Thread->WaitNext) == FALSE) {
        goto WaitStart;
    }

    Thread->WaitNext = FALSE;
    InitializeWaitSingle();

    //
    // Start of wait loop.
    //
    // Note this loop is repeated if a kernel APC is delivered in the middle
    // of the wait or a kernel APC is pending on the first attempt through
    // the loop.
    //

    do {

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // DISPATCH_LEVEL, but before the dispatcher database was locked.
        //
        // N.B. This can only happen in a multiprocessor system.
        //

        Thread->Preempted = FALSE;
        if (Thread->ApcState.KernelApcPending &&
            (Thread->SpecialApcDisable == 0) &&
            (Thread->WaitIrql < APC_LEVEL)) {

            //
            // Unlock the dispatcher database and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will result
            // in the delivery of the kernel APC if possible.
            //

            KiUnlockDispatcherDatabase(Thread->WaitIrql);

        } else {

            //
            // If the object is a mutant object and the mutant object has been
            // recursively acquired MINLONG times, then raise an exception.
            // Otherwise if the signal state of the mutant object is greater
            // than zero, or the current thread is the owner of the mutant
            // object, then satisfy the wait.
            //

            ASSERT(Objectx->Header.Type != QueueObject);

            if (Objectx->Header.Type == MutantObject) {
                if ((Objectx->Header.SignalState > 0) ||
                    (Thread == Objectx->OwnerThread)) {
                    if (Objectx->Header.SignalState != MINLONG) {
                        KiWaitSatisfyMutant(Objectx, Thread);
                        WaitStatus = (NTSTATUS)(Thread->WaitStatus);
                        goto NoWait;

                    } else {
                        KiUnlockDispatcherDatabase(Thread->WaitIrql);
                        ExRaiseStatus(STATUS_MUTANT_LIMIT_EXCEEDED);
                    }
                }

            //
            // If the signal state is greater than zero, then satisfy the wait.
            //

            } else if (Objectx->Header.SignalState > 0) {
                KiWaitSatisfyOther(Objectx);
                WaitStatus = (NTSTATUS)(0);
                goto NoWait;
            }

            //
            // Construct a wait block for the object.
            //

            //
            // Test for alert pending.
            //

            TestForAlertPending(Alertable);

            //
            // The wait cannot be satisfied immediately. Check to determine if
            // a timeout value is specified.
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
                    WaitStatus = (NTSTATUS)STATUS_TIMEOUT;
                    goto NoWait;
                }
            }

            //
            // Insert wait block in object wait list.
            //

            InsertTailList(&Objectx->Header.WaitListHead, &WaitBlock->WaitListEntry);

            //
            // If the current thread is processing a queue entry, then attempt
            // to activate another thread that is blocked on the queue object.
            //

            Queue = Thread->Queue;
            if (Queue != NULL) {
                KiActivateWaiterQueue(Queue);
            }

            //
            // Set the thread wait parameters, set the thread dispatcher state
            // to Waiting, and insert the thread in the wait list.
            //

            Thread->State = Waiting;
            CurrentPrcb = KeGetCurrentPrcb();
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

            WaitStatus = (NTSTATUS)KiSwapThread(Thread, CurrentPrcb);

            //
            // If the thread was not awakened to deliver a kernel mode APC,
            // then return wait status.
            //

            if (WaitStatus != STATUS_KERNEL_APC) {
                return WaitStatus;
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
        // Raise IRQL to SYNCH level, initialize the thread local variables,
        // and lock the dispatcher database.
        //

WaitStart:
        Thread->WaitIrql = KeRaiseIrqlToSynchLevel();
        InitializeWaitSingle();
        KiLockDispatcherDatabaseAtSynchLevel();

    } while (TRUE);

    //
    // The thread is alerted or a user APC should be delivered. Unlock the
    // dispatcher database, lower IRQL to its previous value, and return
    // the wait status.
    //

    KiUnlockDispatcherDatabase(Thread->WaitIrql);
    return WaitStatus;

    //
    // The wait has been satisfied without actually waiting.
    //
    // Unlock the dispatcher database and remain at SYNCH level.
    //

NoWait:

    KiUnlockDispatcherDatabaseFromSynchLevel();

    //
    // Adjust the thread quantum, exit the scheduler, and return the wait
    // completion status.
    //

    KiAdjustQuantumThread(Thread);
    return WaitStatus;
}

NTSTATUS
KiSetServerWaitClientEvent (
    __inout PKEVENT ServerEvent,
    __inout PKEVENT ClientEvent,
    __in ULONG WaitMode
    )

/*++

Routine Description:

    This function sets the specified server event and waits on specified
    client event. The wait is performed such that an optimal switch to
    the waiting thread occurs if possible. No timeout is associated with
    the wait, and thus, the issuing thread will wait until the client event
    is signaled or an APC is delivered.

Arguments:

    ServerEvent - Supplies a pointer to a dispatcher object of type event.

    ClientEvent - Supplies a pointer to a dispatcher object of type event.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

Return Value:

    The wait completion status. A value of STATUS_SUCCESS is returned if
    the specified object satisfied the wait. A value of STATUS_USER_APC is
    returned if the wait was aborted to deliver a user APC to the current
    thread.

--*/

{

    //
    // Set server event and wait on client event atomically.
    //

    KeSetEvent(ServerEvent, EVENT_INCREMENT, TRUE);
    return KeWaitForSingleObject(ClientEvent,
                                 WrEventPair,
                                 (KPROCESSOR_MODE)WaitMode,
                                 FALSE,
                                 NULL);
}

VOID
FASTCALL
KiAcquireFastMutex (
    IN PFAST_MUTEX Mutex
    )

/*++

Routine Description:

    This function is the slow path for fast mutex acquires.

Arguments:

    Mutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

#if !defined (_X86_)

    LONG BitsToChange;
    LONG NewValue;
    LONG OldValue;
    LONG WaitIncrement;

#endif

    //
    // Increment the contention count and wait or acquire fast mutex.
    //

    Mutex->Contention += 1;

#if defined (_X86_)

    KeWaitForSingleObject(&Mutex->Gate, WrMutex, KernelMode, FALSE, NULL);

#else

    BitsToChange = FM_LOCK_BIT;
    WaitIncrement = FM_LOCK_WAITER_INC;
    do {

        ASSERT((BitsToChange == FM_LOCK_BIT) ||
               (BitsToChange == (FM_LOCK_BIT | FM_LOCK_WAITER_WOKEN)));

        ASSERT((WaitIncrement == FM_LOCK_WAITER_INC) ||
               (WaitIncrement == FM_LOCK_WAITER_WOKEN));

        OldValue = Mutex->Count;
        do {
            if ((OldValue & FM_LOCK_BIT) != 0) {

                ASSERT((BitsToChange == FM_LOCK_BIT) ||
                       ((OldValue & FM_LOCK_WAITER_WOKEN) != 0));

                NewValue = OldValue ^ BitsToChange;
                if ((NewValue = InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue)) == OldValue) {
                    return;
                }

            } else {
                NewValue = OldValue + WaitIncrement;
                if ((NewValue = InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue)) == OldValue) {
                    break;
                }
            }

            OldValue = NewValue;

        } while (TRUE);

        //
        // Wait until woken.
        //

        KeWaitForGate((PKGATE)&Mutex->Gate, WrGuardedMutex, KernelMode);

        ASSERT((Mutex->Count & FM_LOCK_WAITER_WOKEN) != 0);

        //
        // Switch to trying to set the lock bit and clear the woken bit or
        // incrementing the waiters and clearing woken bit.
        //

        BitsToChange = FM_LOCK_BIT | FM_LOCK_WAITER_WOKEN;
        WaitIncrement = FM_LOCK_WAITER_WOKEN;

        ASSERT((FM_LOCK_WAITER_WOKEN * 2) == FM_LOCK_WAITER_INC);

    } while (TRUE);

#endif

    return;
}

VOID
FASTCALL
KiAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )
/*++

Routine Description:

    This function is the slow path for guarded mutex acquires.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    LONG BitsToChange;
    LONG NewValue;
    LONG OldValue;
    LONG WaitIncrement;

    //
    // Increment the contention count and wait or acquire the guarded mutex.
    //

    Mutex->Contention += 1;
    BitsToChange = GM_LOCK_BIT;
    WaitIncrement = GM_LOCK_WAITER_INC;
    do {

        ASSERT((BitsToChange == GM_LOCK_BIT) ||
               (BitsToChange == (GM_LOCK_BIT | GM_LOCK_WAITER_WOKEN)));

        ASSERT((WaitIncrement == GM_LOCK_WAITER_INC) ||
               (WaitIncrement == GM_LOCK_WAITER_WOKEN));

        OldValue = Mutex->Count;
        do {
            if ((OldValue & GM_LOCK_BIT) != 0) {

                ASSERT((BitsToChange == GM_LOCK_BIT) ||
                       ((OldValue & GM_LOCK_WAITER_WOKEN) != 0));

                NewValue = OldValue ^ BitsToChange;
                if ((NewValue = InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue)) == OldValue) {
                    return;
                }

            } else {
                NewValue = OldValue + WaitIncrement;
                if ((NewValue = InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue)) == OldValue) {
                    break;
                }
            }
            OldValue = NewValue;
        } while (TRUE);

        //
        // Wait until woken.
        //

        KeWaitForGate(&Mutex->Gate, WrGuardedMutex, KernelMode);

        ASSERT((Mutex->Count & GM_LOCK_WAITER_WOKEN) != 0);

        //
        // Revert to trying to set the lock bit and clear the woken bit or
        // incrementing the waiters and clearing woken bit.
        //

        BitsToChange = GM_LOCK_BIT | GM_LOCK_WAITER_WOKEN;
        WaitIncrement = GM_LOCK_WAITER_WOKEN;

        ASSERT((GM_LOCK_WAITER_WOKEN * 2) == GM_LOCK_WAITER_INC);

    } while (TRUE);

    return;
}

