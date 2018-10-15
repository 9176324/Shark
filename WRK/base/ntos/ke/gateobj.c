/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    gateobj.c

Abstract:

    This module implements the kernel gate object. Functions are provided to
    initialize, signal, and wait for gate objects.

--*/

#include "ki.h"

VOID
FASTCALL
KeInitializeGate (
    __out PKGATE Gate
    )

/*++

Routine Description:

    This function initializes a kernel gate object.

Arguments:

    Gate - Supplies a pointer to a dispatcher object of type gate.

Return Value:

    None.

--*/

{

    //
    // Initialize standard dispatcher object header.
    //

    Gate->Header.Type = (UCHAR)GateObject;
    Gate->Header.Size = sizeof(KGATE) / sizeof(LONG);
    Gate->Header.SignalState = 0;
    InitializeListHead(&Gate->Header.WaitListHead);
    return;
}

VOID
FASTCALL
KeSignalGateBoostPriority (
    __inout PKGATE Gate
    )

/*++

Routine Description:

    This function conditionally sets the signal state of a gate object and
    attempts to unwait the first waiter.

Arguments:

    Gate - Supplies a pointer to a dispatcher object of type gate.

Return Value:

    None.

--*/

{

    PKTHREAD CurrentThread;
    PLIST_ENTRY Entry;
    KIRQL OldIrql;
    SCHAR Priority;
    PKQUEUE Queue;
    PKWAIT_BLOCK WaitBlock;
    PKTHREAD WaitThread;

    ASSERT_GATE(Gate);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the object lock.
    //
    // If the object is not already signaled, then attempt to wake a waiter.
    //

    CurrentThread = KeGetCurrentThread();
    do {
        OldIrql = KeRaiseIrqlToSynchLevel();
        KiAcquireKobjectLock(Gate);
        if (Gate->Header.SignalState == 0) {
    
            //
            // If there are any waiters, then remove and ready the first
            // waiter. Otherwise, set the signal state.
            //
    
            if (IsListEmpty(&Gate->Header.WaitListHead) == FALSE) {
                Entry = Gate->Header.WaitListHead.Flink;
                WaitBlock = CONTAINING_RECORD(Entry, KWAIT_BLOCK, WaitListEntry);
                WaitThread = WaitBlock->Thread;

                //
                // Try to acquire the thread lock.
                //
                // If the thread lock cannot be acquired, then release the
                // object lock, lower IRQL to is previous value, and try to
                // signal the gate again. Otherwise, remove the entry from
                // the list, set the wait completion status to success, set
                // the deferred processor number, set the thread state to
                // deferred ready, release the thread lock, release the
                // object lock, compute the new thread priority, ready the
                // thread for execution, and exit the dispatcher.
                //

                if (KiTryToAcquireThreadLock(WaitThread)) {
                    RemoveEntryList(Entry);
                    WaitThread->WaitStatus = STATUS_SUCCESS;
                    WaitThread->State = DeferredReady;
                    WaitThread->DeferredProcessor = KeGetCurrentPrcb()->Number;
                    KiReleaseKobjectLock(Gate);
                    KiReleaseThreadLock(WaitThread);
                    Priority = CurrentThread->Priority;
                    if (Priority < LOW_REALTIME_PRIORITY) {
                        Priority = Priority - CurrentThread->PriorityDecrement;
                        if (Priority < CurrentThread->BasePriority) {
                            Priority = CurrentThread->BasePriority;
                        }

                        if (CurrentThread->PriorityDecrement != 0) {
                            CurrentThread->PriorityDecrement = 0;
                            CurrentThread->Quantum = CLOCK_QUANTUM_DECREMENT;
                        }
                    }

                    WaitThread->AdjustIncrement = Priority;
                    WaitThread->AdjustReason = (UCHAR)AdjustBoost;

                    //
                    // If the wait thread is associated with a queue, then
                    // increment the concurrency level.
                    //
                    // N.B. This can be done after dropping all other locks,
                    //      but must be done holding the dispatcher lock
                    //      since the concurrency count is not accessed with
                    //      interlocked operations elsewhere.
                    //    

                    if (WaitThread->Queue != NULL) {
                        KiLockDispatcherDatabaseAtSynchLevel();
                        if ((Queue = WaitThread->Queue) != NULL) {
                            Queue->CurrentCount += 1;
                        }

                        KiUnlockDispatcherDatabaseFromSynchLevel();
                    }

                    KiDeferredReadyThread(WaitThread);
                    KiExitDispatcher(OldIrql);
                    return;

                } else {
                    KiReleaseKobjectLock(Gate);
                    KeLowerIrql(OldIrql);
                    continue;
                }

    
            } else {
                Gate->Header.SignalState = 1;
                break;
            }

        } else {
            break;
        }

    } while (TRUE);

    //
    // Release the object lock and lower IRQL to its previous value.
    //

    KiReleaseKobjectLock(Gate);
    KeLowerIrql(OldIrql);
    return;
}

VOID
FASTCALL
KeWaitForGate (
    __inout PKGATE Gate,
    __in KWAIT_REASON WaitReason,
    __in KPROCESSOR_MODE WaitMode
    )

/*++

Routine Description:

    This function waits until the signal state of a gate object is set. If
    the state of the gate object is signaled when the wait is executed, then
    no wait will occur.

Arguments:

    Gate - Supplies a pointer to a dispatcher object of type Gate.

    WaitReason - Supplies the reason for the wait.

    WaitMode  - Supplies the processor mode in which the wait is to occur.

Return Value:

    None.

--*/

{

    PKTHREAD CurrentThread;
    KLOCK_QUEUE_HANDLE LockHandle;
    PKQUEUE Queue;
    PKWAIT_BLOCK WaitBlock;
    NTSTATUS WaitStatus;

    ASSERT_GATE(Gate);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the APC queue lock.
    //

    CurrentThread = KeGetCurrentThread();
    do {
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&CurrentThread->ApcQueueLock,
                                                   &LockHandle);

        //
        // Test to determine if a kernel APC is pending.
        //
        // If a kernel APC is pending, the special APC disable count is zero,
        // and the previous IRQL was less than APC_LEVEL, then a kernel APC
        // was queued by another processor just after IRQL was raised to
        // SYNCH_LEVEL, but before the APC queue lock was acquired.
        //
        // N.B. This can only happen in a multiprocessor system.
        //

        if (CurrentThread->ApcState.KernelApcPending &&
            (CurrentThread->SpecialApcDisable == 0) &&
            (LockHandle.OldIrql < APC_LEVEL)) {

            //
            // Unlock the APC queue lock and lower IRQL to its previous value.
            // An APC interrupt will immediately occur which will result in
            // the delivery of the kernel APC if possible.
            //

            KeReleaseInStackQueuedSpinLock(&LockHandle);
            continue;
        }

        //
        // If the current thread is associated with a queue object, then
        // acquire the dispatcher lock.
        //

        if ((Queue = CurrentThread->Queue) != NULL) {
            KiLockDispatcherDatabaseAtSynchLevel();
        }

        //
        // Acquire the thread lock and the object lock.
        //
        // If the object is already signaled, then clear the signaled state,
        // release the object lock, release the thread lock, and lower IRQL
        // to its previous value. Otherwise, set the thread state to gate
        // wait, set the address of the gate object, insert the thread in the
        // object wait list, set context swap busy, release the object lock,
        // release the thread lock, and switch to a new thread.
        //

        KiAcquireThreadLock(CurrentThread);
        KiAcquireKobjectLock(Gate);
        if (Gate->Header.SignalState != 0) {
            Gate->Header.SignalState = 0;
            KiReleaseKobjectLock(Gate);
            KiReleaseThreadLock(CurrentThread);
            if (Queue != NULL) {
                KiUnlockDispatcherDatabaseFromSynchLevel();
            }

            KeReleaseInStackQueuedSpinLock(&LockHandle);
            break;
    
        } else {
            WaitBlock = &CurrentThread->WaitBlock[0];
            WaitBlock->Object = Gate;
            WaitBlock->Thread = CurrentThread;
            CurrentThread->WaitMode = WaitMode;
            CurrentThread->WaitReason = WaitReason;
            CurrentThread->WaitIrql = LockHandle.OldIrql;
            CurrentThread->State = GateWait;
            CurrentThread->GateObject = Gate;
            InsertTailList(&Gate->Header.WaitListHead, &WaitBlock->WaitListEntry);
            KiReleaseKobjectLock(Gate);
            KiSetContextSwapBusy(CurrentThread);
            KiReleaseThreadLock(CurrentThread);

            //
            // If the current thread is associated with a queue object, then
            // activate another thread if possible.
            //

            if (Queue != NULL) {
                if ((Queue = CurrentThread->Queue) != NULL) {
                    KiActivateWaiterQueue(Queue);
                }

                KiUnlockDispatcherDatabaseFromSynchLevel();
            }

            KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
            WaitStatus = (NTSTATUS)KiSwapThread(CurrentThread, KeGetCurrentPrcb());
            if (WaitStatus == STATUS_SUCCESS) {
                return;
            }
        }

    } while (TRUE);

    return;
}         

