/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    thredsup.c

Abstract:

    This module contains the support routines for the thread object. It
    contains functions to boost the priority of a thread, find a ready
    thread, select the next thread, ready a thread, set priority of a
    thread, and to suspend a thread.

Environment:

    All of the functions in this module execute in kernel mode except
    the function that raises a user mode alert condition.

--*/

#include "ki.h"

VOID
KiSuspendNop (
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    )

/*++

Routine Description:

    This function is the kernel routine for the builtin suspend APC for a
    thread. It is executed in kernel mode as the result of queuing the
    builtin suspend APC.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.

    NormalRoutine - not used

    NormalContext - not used

    SystemArgument1 - not used

    SystemArgument2 - not used

Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Apc);
    UNREFERENCED_PARAMETER(NormalRoutine);
    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    //
    // No operation is performed by this routine.
    //

    return;
}

VOID
KiSuspendRundown (
    IN PKAPC Apc
    )

/*++

Routine Description:

    This function is the rundown routine for the threads built in suspend APC.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.


Return Value:

    None.

--*/

{

    UNREFERENCED_PARAMETER(Apc);

    //
    // No operation is performed by this routine.
    //

    return;
}

VOID
FASTCALL
KiDeferredReadyThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function readies a thread for execution and attempts to dispatch the
    thread for execution by either assigning the thread to an idle processor
    or preempting another lower priority thread.

    If the thread can be assigned to an idle processor, then the thread enters
    the standby state and the target processor will switch to the thread on
    its next iteration of the idle loop.

    If a lower priority thread can be preempted, then the thread enters the
    standby state and the target processor is requested to dispatch.

    If the thread cannot be assigned to an idle processor and another thread
    cannot be preempted, then the specified thread is inserted at the head or
    tail of the dispatcher ready selected by its priority depending on whether
    it was preempted or not.

    N.B. This function is called at SYNCH level with no PRCB locks held.

    N.B. This function may be called with the dispatcher database lock held.

    N.B. Neither the priority nor the affinity of a thread in the deferred
         ready state can be changed outside the PRCB lock of the respective
         processor.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

Return Value:

    None.

--*/

{

    PKPRCB CurrentPrcb;
    BOOLEAN Preempted;
    KPRIORITY Priority;
    PKPROCESS Process;
    ULONG Processor;
    PKPRCB TargetPrcb;
    KPRIORITY ThreadPriority;
    PKTHREAD Thread1;

#if !defined(NT_UP)

    KAFFINITY Affinity;
    ULONG IdealProcessor;
    KAFFINITY IdleSummary;

#if defined(NT_SMT)

    KAFFINITY FavoredSMTSet;
    KAFFINITY IdleSMTSet;

#endif

    KAFFINITY IdleSet;
    PKNODE Node;

#endif

    ASSERT(Thread->State == DeferredReady);

    ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

    //
    // Check if a priority adjustment is requested.
    //

    if (Thread->AdjustReason == AdjustNone) {

        //
        // No priority adjustment.
        //

        NOTHING;

    } else if (Thread->AdjustReason == AdjustBoost) {

        //
        // Priority adjustment as the result of a set event boost priority.
        //
        // The current thread priority is stored in the adjust increment
        // field of the thread object.
        //
        // Acquire the thread lock.
        //
        // If the priority of the waiting thread is less than or equal
        // to the priority of the current thread and the waiting thread
        // priority is less than the time critical priority bound and
        // boosts are not disabled for the waiting thread, then boost
        // the priority of the waiting thread to the minimum of the
        // priority of the current thread priority plus one and the time
        // critical bound minus one. This boost will be taken away at
        // quantum end.
        //

        KiAcquireThreadLock(Thread);
        if ((Thread->Priority <= Thread->AdjustIncrement) &&
            (Thread->Priority < (TIME_CRITICAL_PRIORITY_BOUND - 1)) &&
            (Thread->DisableBoost == FALSE)) {

            //
            // Compute the new thread priority.
            //

            Priority = min(Thread->AdjustIncrement + 1,
                           TIME_CRITICAL_PRIORITY_BOUND - 1);

            ASSERT((Thread->PriorityDecrement >= 0) &&
                   (Thread->PriorityDecrement <= Thread->Priority));

            Thread->PriorityDecrement += ((SCHAR)Priority - Thread->Priority);

            ASSERT((Thread->PriorityDecrement >= 0) &&
                   (Thread->PriorityDecrement <= Priority));

            Thread->Priority = (SCHAR)Priority;
        }

        //
        // Make sure the thread has a quantum that is appropriate for
        // lock ownership and charge quantum.
        //

        if (Thread->Quantum < LOCK_OWNERSHIP_QUANTUM) {
            Thread->Quantum = LOCK_OWNERSHIP_QUANTUM;
        }

        Thread->Quantum -= WAIT_QUANTUM_DECREMENT;

        //
        // Release the thread lock and set the adjust reason to none.
        //

        ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

        KiReleaseThreadLock(Thread);
        Thread->AdjustReason = AdjustNone;

    } else if (Thread->AdjustReason == AdjustUnwait) {

        //
        // Priority adjustment as the result of an unwait operation.
        //
        // The priority increment is stored in the adjust increment field of
        // the thread object.
        //
        // Acquire the thread lock.
        //
        // If the thread runs at a realtime priority level, then reset the
        // thread quantum. Otherwise, compute the next thread priority and
        // charge the thread for the wait operation.
        //
    
        Process = Thread->ApcState.Process;
        KiAcquireThreadLock(Thread);
        if (Thread->Priority < LOW_REALTIME_PRIORITY) {

            //
            // If the thread base priority is time critical or higher, then
            // replenish the quantum.
            //

            if (Thread->BasePriority >= TIME_CRITICAL_PRIORITY_BOUND) {
                Thread->Quantum = Thread->QuantumReset;
    
            } else {

                //
                // If the thread has not received an unusual boost and the
                // priority increment is nonzero, then replenish the thread
                // quantum.
                //

                if ((Thread->PriorityDecrement == 0) && (Thread->AdjustIncrement > 0)) {
                    Thread->Quantum = Thread->QuantumReset;
                }

                //
                // If the thread was unwaited to execute a kernel APC,
                // then do not charge the thread any quantum. The wait
                // code will charge quantum after the kernel APC has
                // executed and the wait is actually satisfied. Otherwise,
                // reduce the thread quantum and compute the new thread
                // priority if quantum runout occurs.
                //
            
                if (Thread->WaitStatus != STATUS_KERNEL_APC) {
                    Thread->Quantum -= WAIT_QUANTUM_DECREMENT;
                    if (Thread->Quantum <= 0) {
                        Thread->Quantum = Thread->QuantumReset;
                        Thread->Priority = KiComputeNewPriority(Thread, 1);
                    }
                }
            }

            //
            // If the thread is not running with an unusual boost and boosts
            // are not disabled, then attempt to apply the specified priority
            // increment.
            //

            if ((Thread->PriorityDecrement == 0) &&
                (Thread->DisableBoost == FALSE)) {
    
                //
                // If the specified thread is from a process with a foreground
                // memory priority, then add the foreground boost separation.
                //

                ASSERT(Thread->AdjustIncrement >= 0);

                Priority = Thread->BasePriority + Thread->AdjustIncrement;
                if (((PEPROCESS)Process)->Vm.Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND) {
                    Priority += ((SCHAR)PsPrioritySeparation);
                }
    
                //
                // If the new thread priority is greater than the current
                // thread priority, then boost the thread priority, but not
                // above low real time minus one.
                //
    
                if (Priority > Thread->Priority) {
                    if (Priority >= LOW_REALTIME_PRIORITY) {
                        Priority = LOW_REALTIME_PRIORITY - 1;
                    }
    
                    //
                    // If the new thread priority is greater than the thread
                    // base priority plus the specified increment (i.e., the
                    // foreground separation was added), then set the priority
                    // decrement to remove the separation boost after one
                    // quantum.
                    //
    
                    if (Priority > (Thread->BasePriority + Thread->AdjustIncrement)) {
                        Thread->PriorityDecrement =
                            ((SCHAR)Priority - Thread->BasePriority - Thread->AdjustIncrement);
                    }

                    ASSERT((Thread->PriorityDecrement >= 0) &&
                           (Thread->PriorityDecrement <= Priority));

                    Thread->Priority = (SCHAR)Priority;
                }
            }
    
        } else {
            Thread->Quantum = Thread->QuantumReset;
        }

        //
        // Release the thread lock and set the adjust reason to none.
        //

        ASSERT((Thread->Priority >= 0) && (Thread->Priority <= HIGH_PRIORITY));

        KiReleaseThreadLock(Thread);
        Thread->AdjustReason = AdjustNone;

    } else {

        //
        // Invalid priority adjustment reason.
        //

        ASSERT(FALSE);

        Thread->AdjustReason = AdjustNone;
    }

    //
    // Save the value of thread's preempted flag and set thread preempted
    // FALSE,
    //

    Preempted = Thread->Preempted;
    Thread->Preempted = FALSE;

    //
    // If there is an idle processor, then schedule the thread on an
    // idle processor giving preference to:
    //
    // (a) the thread's ideal processor,
    //
    // (b) if the thread has a soft (preferred affinity set) and
    //     that set contains an idle processor, reduce the set to
    //     the intersection of the two sets.
    //
    // (c) if the processors are Simultaneous Multi Threaded, and the
    //     set contains physical processors with no busy logical
    //     processors, reduce the set to that subset.
    //
    // (d) if this thread last ran on a member of this remaining set,
    //     select that processor, otherwise,
    //
    // (e) if there are processors amongst the remainder which are
    //     not sleeping, reduce to that subset.
    //
    // (f) select the leftmost processor from this set.
    //

#if defined(NT_UP)

    Thread->NextProcessor = 0;
    TargetPrcb = KiProcessorBlock[0];
    if (KiIdleSummary != 0) {
        KiIdleSummary = 0;
        Thread->State = Standby;
        TargetPrcb->NextThread = Thread;
        return;
    }

    Processor = 0;
    CurrentPrcb = TargetPrcb;
    ThreadPriority = Thread->Priority;

#else

    //
    // Attempt to assign the thread on an idle processor.
    //

    CurrentPrcb = KeGetCurrentPrcb();

IdleAssignment:
    Affinity = Thread->Affinity;
    do {
        Processor = Thread->IdealProcessor;
        IdleSet = KiIdleSummary & Affinity;
        if (IdleSet != 0) {
            if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                //
                // Ideal processor is not available.
                //
                // If the intersection of the idle set and the node
                // affinity is nonzero, then reduce the set of idle
                // processors by the node affinity.
                //

                Node = KiProcessorBlock[Processor]->ParentNode;
                if ((IdleSet & Node->ProcessorMask) != 0) {
                    IdleSet &= Node->ProcessorMask;
                }

                //
                // If the intersection of the idle set and the SMT idle
                // set is nonzero, then reduce the set of idle processors
                // by the SMT idle set.
                //
    
#if defined(NT_SMT)

                IdleSMTSet = KiIdleSMTSummary;
                if ((IdleSet & IdleSMTSet) != 0) {
                    IdleSet &= IdleSMTSet;
                }
    
#endif
    
                //
                // If the last processor the thread ran on is included in
                // the idle set, then attempt to select that processor.
                //

                IdealProcessor = Processor;
                Processor = Thread->NextProcessor;
                if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                    //
                    // If the current processor is included in the idle,
                    // then attempt to select that processor. 
                    //

                    Processor = CurrentPrcb->Number;
                    if ((IdleSet & AFFINITY_MASK(Processor)) == 0) {

                        //
                        // If the intersection of the idle set and the
                        // logical processor set on the ideal processor
                        // node is nonzero, then reduce the set of idle
                        // processors by the logical processor set.
                        //
                        // Otherwise, if the intersection of the idle
                        // set and the logical processor set of the last
                        // processor node is nonzero, then reduce the set
                        // of idle processors by the logical processor set.
                        //
    
#if defined(NT_SMT)
    
                        FavoredSMTSet = KiProcessorBlock[IdealProcessor]->MultiThreadProcessorSet;
                        if ((IdleSet & FavoredSMTSet) != 0) {
                            IdleSet &= FavoredSMTSet;

                        } else {
                            FavoredSMTSet = KiProcessorBlock[Thread->NextProcessor]->MultiThreadProcessorSet;
                            if ((IdleSet & FavoredSMTSet) != 0) {
                                IdleSet &= FavoredSMTSet;
                            }
                        }
    
#endif
    
                        //
                        // Select an idle processor from the remaining
                        // set.
                        //

                        KeFindFirstSetLeftAffinity(IdleSet, &Processor);
                    }
                }
            }

            //
            // Acquire the current and target PRCB locks and ensure the
            // selected processor is still idle and the thread can still
            // run on the processor.
            //

            TargetPrcb = KiProcessorBlock[Processor];
            KiAcquireTwoPrcbLocks(CurrentPrcb, TargetPrcb);
            IdleSummary = ReadForWriteAccess(&KiIdleSummary);
            if (((IdleSummary & TargetPrcb->SetMember) != 0) &&
                ((Thread->Affinity & TargetPrcb->SetMember) != 0)) {

                //
                // Set the thread state to standby, set the processor
                // number the thread is being assigned to, and clear the
                // associated bit in idle summary.
                //

                Thread->State = Standby;
                Thread->NextProcessor = (UCHAR)Processor;
                KiClearIdleSummary(AFFINITY_MASK(Processor));
    
                ASSERT((TargetPrcb->NextThread == NULL) ||
                       (TargetPrcb->NextThread == TargetPrcb->IdleThread));
    
                TargetPrcb->NextThread = Thread;
    
                //
                // Update the idle SMT summary set to indicate that the
                // SMT set is not idle.
                //
    
                KiClearSMTSummary(TargetPrcb->MultiThreadProcessorSet);
                if ((TargetPrcb != CurrentPrcb) &&
                    (KeIsIdleHaltSet(TargetPrcb, Processor) != FALSE)) {

                    KiSendSoftwareInterrupt(AFFINITY_MASK(Processor), DISPATCH_LEVEL);
                }

                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                return;

            } else {
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                continue;
            }

        } else {
            break;
        }

    } while (TRUE);

    //
    // Select the ideal processor as the processor to preempt, if possible.
    //

    TargetPrcb = KiProcessorBlock[Processor];

    //
    // There are no suitable idle processors to run the thread. Acquire
    // the current and target PRCB locks and ensure the target processor
    // is not idle and the thread can still run on the processor.
    //

    KiAcquireTwoPrcbLocks(CurrentPrcb, TargetPrcb);
    ThreadPriority = Thread->Priority;
    if (((KiIdleSummary & TargetPrcb->SetMember) == 0) &&
        (Thread->IdealProcessor == Processor)) {

        ASSERT((Thread->Affinity & TargetPrcb->SetMember) != 0);

#endif

        Thread->NextProcessor = (UCHAR)Processor;
        if ((Thread1 = TargetPrcb->NextThread) != NULL) {

            ASSERT(Thread1->State == Standby);

            if (ThreadPriority > Thread1->Priority) {
                Thread1->Preempted = TRUE;
                Thread->State = Standby;
                TargetPrcb->NextThread = Thread;
                Thread1->State = DeferredReady;
                Thread1->DeferredProcessor = CurrentPrcb->Number;
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                KiDeferredReadyThread(Thread1);
                return;
            }

        } else {
            Thread1 = TargetPrcb->CurrentThread;
            if (ThreadPriority > Thread1->Priority) {
                if (Thread1->State == Running) {
                    Thread1->Preempted = TRUE;
                }

                Thread->State = Standby;
                TargetPrcb->NextThread = Thread;
                KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
                KiRequestDispatchInterrupt(Thread->NextProcessor);
                return;
            }
        }

#if !defined(NT_UP)

    } else {
        KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
        goto IdleAssignment;
    }

#endif

    //
    // No thread can be preempted.
    //
    // Insert the thread in the dispatcher queue selected by its priority.
    // If the thread was preempted, then insert the thread at the front of
    // the queue. Otherwise, insert the thread at the tail of the queue.
    //

    ASSERT((ThreadPriority >= 0) && (ThreadPriority <= HIGH_PRIORITY));

    Thread->State = Ready;
    Thread->WaitTime = KiQueryLowTickCount();
    if (Preempted != FALSE) {
        InsertHeadList(&TargetPrcb->DispatcherReadyListHead[ThreadPriority],
                       &Thread->WaitListEntry);

    } else {
        InsertTailList(&TargetPrcb->DispatcherReadyListHead[ThreadPriority],
                       &Thread->WaitListEntry);
    }

    TargetPrcb->ReadySummary |= PRIORITY_MASK(ThreadPriority);

    ASSERT(ThreadPriority == Thread->Priority);

    KiReleaseTwoPrcbLocks(CurrentPrcb, TargetPrcb);
    return;
}

#if !defined(NT_UP)

PKTHREAD
FASTCALL
KiFindReadyThread (
    IN ULONG Number,
    IN PKPRCB Prcb
    )

/*++

Routine Description:

    This function searches the dispatcher ready queues in an attempt to find
    a thread that can execute on the specified processor.

    N.B. This routine is called with the sources PRCB locked and the specified
         PRCB lock held and returns with both locks held.

    N.B. This routine is only called when it is known that the ready summary
         for the specified processor is nonzero.

Arguments:

    Number - Supplies the number of the processor to find a thread for.

    Prcb - Supplies a pointer to the processor control block whose ready
        queues are to be examined.

Return Value:

    If a thread is located that can execute on the specified processor, then
    the address of the thread object is returned. Otherwise a null pointer is
    returned.

--*/

{

    ULONG HighPriority;
    PRLIST_ENTRY ListHead;
    PRLIST_ENTRY NextEntry;
    ULONG PrioritySet;
    PKTHREAD Thread;

    //
    // Initialize the set of priority levels that should be scanned in an
    // attempt to find a thread that can run on the specified processor.
    //

    PrioritySet = Prcb->ReadySummary;

    ASSERT(PrioritySet != 0);

    KeFindFirstSetLeftMember(PrioritySet, &HighPriority);
    do {

        ASSERT((PrioritySet & PRIORITY_MASK(HighPriority)) != 0);
        ASSERT(IsListEmpty(&Prcb->DispatcherReadyListHead[HighPriority]) == FALSE);

        ListHead = &Prcb->DispatcherReadyListHead[HighPriority];
        NextEntry = ListHead->Flink;

        ASSERT(NextEntry != ListHead);

        //
        // Scan the specified dispatcher ready queue for a suitable
        // thread to execute.
        //
        // N.B. It is not necessary to attempt to find a better candidate
        //      on either multinode or non-multinode systems. For multinode
        //      systems, this routine is called sequentially specifying each
        //      processor on the current node before attempting to schedule
        //      from other processors. For non-multinode systems all threads
        //      run on a single node and there is no node distinction. In
        //      both cases threads are inserted in per-processor ready queues
        //      according to their ideal processor.
        //

        do {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
            if ((Thread->Affinity & AFFINITY_MASK(Number)) != 0) {

                ASSERT((Prcb->ReadySummary & PRIORITY_MASK(HighPriority)) != 0);
                ASSERT((KPRIORITY)HighPriority == Thread->Priority);
                ASSERT(Thread->NextProcessor == Prcb->Number);

                if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                    Prcb->ReadySummary ^= PRIORITY_MASK(HighPriority);
                }

                Thread->NextProcessor = (UCHAR)Number;
                return Thread;
            }

            NextEntry = NextEntry->Flink;
        } while (NextEntry != ListHead);

        PrioritySet ^= PRIORITY_MASK(HighPriority);
        KeFindFirstSetLeftMember(PrioritySet, &HighPriority);
    } while (PrioritySet != 0);

    //
    // No thread could be found, return a null pointer.
    //

    return NULL;
}

VOID
FASTCALL
KiProcessDeferredReadyList (
    IN PKPRCB CurrentPrcb
    )

/*++

Routine Description:

    This function is called to process the deferred ready list.

    N.B. This function is called at SYNCH level with no locks held.

    N.B. This routine is only called when it is known that the deferred
         ready list is not empty.

    N.B. The deferred ready list is a per processor list and items are
         only inserted and removed from the respective processor. Thus
         no synchronization of the list is required.

Arguments:

    CurrentPrcb - Supplies a pointer to the current processor's PRCB.

Return Value:

    None.

--*/

{

    PSINGLE_LIST_ENTRY NextEntry;
    PKTHREAD Thread;

    ASSERT(CurrentPrcb->DeferredReadyListHead.Next != NULL);

    //
    // Save the address of the first entry in the deferred ready list and
    // set the list to empty.
    //

    NextEntry = CurrentPrcb->DeferredReadyListHead.Next;
    CurrentPrcb->DeferredReadyListHead.Next = NULL;

    //
    // Process each entry in deferred ready list and ready the specified
    // thread for execution.
    //

    do {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, SwapListEntry);
        NextEntry = NextEntry->Next;
        KiDeferredReadyThread(Thread);
    } while (NextEntry != NULL);

    ASSERT(CurrentPrcb->DeferredReadyListHead.Next == NULL);

    return;
}

#endif

VOID
FASTCALL
KiQueueReadyThread (
    IN PKTHREAD Thread,
    IN PKPRCB Prcb
    )

/*++

Routine Description:

    This function inserts the specified thread in the appropriate dispatcher
    ready queue for the specified processor if the thread can run on the
    specified processor. Otherwise, the specified thread is readied for
    execution.

    N.B. This function is called with the specified PRCB lock held and returns
         with the PRCB lock not held.

Arguments:

    Thread - Supplies a pointer to a dsispatcher object of type thread.

    Prcb - Supplies a pointer to a processor control block.

Return Value:

    None.

--*/

{

    KxQueueReadyThread(Thread, Prcb);
    return;
}

VOID
FASTCALL
KiReadyThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function inserts the specified thread in the process ready list if
    the thread's process is currently not in memory, inserts the specified
    thread in the kernel stack in swap list if the thread's kernel stack is
    not resident, or inserts the thread in the deferred ready list.

    N.B. This function is called with the dispatcher database lock held and
         returns with the lock held.

    N.B. The deferred ready list is a per processor list and items are
         only inserted and removed from the respective processor. Thus
         no synchronization of the list is required.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

Return Value:

    None.

--*/

{

    PKPROCESS Process;

    //
    // If the thread's process is not in memory, then insert the thread in
    // the process ready queue and inswap the process.
    //

    Process = Thread->ApcState.Process;
    if (Process->State != ProcessInMemory) {
        Thread->State = Ready;
        Thread->ProcessReadyQueue = TRUE;
        InsertTailList(&Process->ReadyListHead, &Thread->WaitListEntry);
        if (Process->State == ProcessOutOfMemory) {
            Process->State = ProcessInTransition;
            InterlockedPushEntrySingleList(&KiProcessInSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

        return;

    } else if (Thread->KernelStackResident == FALSE) {

        //
        // The thread's kernel stack is not resident. Increment the process
        // stack count, set the state of the thread to transition, insert
        // the thread in the kernel stack inswap list, and set the kernel
        // stack inswap event.
        //

        ASSERT(Process->StackCount != MAXULONG_PTR);

        Process->StackCount += 1;

        ASSERT(Thread->State != Transition);

        Thread->State = Transition;
        InterlockedPushEntrySingleList(&KiStackInSwapListHead,
                                       &Thread->SwapListEntry);

        KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        return;

    } else {

        //
        // Insert the specified thread in the deferred ready list.
        //

        KiInsertDeferredReadyList(Thread);
        return;
    }
}

PKTHREAD
FASTCALL
KiSelectNextThread (
    IN PKPRCB Prcb
    )

/*++

Routine Description:

    This function selects the next thread to run on the specified processor.

    N.B. This function is called with the specified PRCB lock held and also
         returns with the lock held.

Arguments:

    Prcb - Supplies a  pointer to a processor block.

Return Value:

    The address of the selected thread object.

--*/

{

    PKTHREAD Thread;

    //
    // Find a ready thread to run from the specified PRCB dispatcher ready
    // queues.
    //

    if ((Thread = KiSelectReadyThread(0, Prcb)) == NULL) {

        //
        // A ready thread cannot be found in the specified PRCB dispatcher
        // ready queues. Select the idle thread and set idle schedule for
        // the specified processor.
        //
        // N.B. Selecting the idle thread with idle schedule set avoids doing
        //      a complete search of all the dispatcher queues for a suitable
        //      thread to run. A complete search will be performed by the idle
        //      thread outside the dispatcher lock.
        //
    
        Thread = Prcb->IdleThread;
        KiSetIdleSummary(Prcb->SetMember);
        Prcb->IdleSchedule = TRUE;
    
        //
        // If all logical processors of the physical processor are idle, then
        // update the idle SMT set summary.
        //
    
        if (KeIsSMTSetIdle(Prcb) == TRUE) {
            KiSetSMTSummary(Prcb->MultiThreadProcessorSet);
        }
    }

    ASSERT(Thread != NULL);

    //
    // Return address of selected thread object.
    //

    ASSERT((Thread->BasePriority == 0) || (Thread->Priority != 0));

    return Thread;
}

KAFFINITY
FASTCALL
KiSetAffinityThread (
    IN PKTHREAD Thread,
    IN KAFFINITY Affinity
    )

/*++

Routine Description:

    This function sets the affinity of a specified thread to a new value.
    If the new affinity is not a proper subset of the parent process affinity
    or is null, then a bugcheck occurs. If the specified thread is running on
    or about to run on a processor for which it is no longer able to run, then
    the target processor is rescheduled. If the specified thread is in a ready
    state and is not in the parent process ready queue, then it is rereadied
    to reevaluate any additional processors it may run on.

Arguments:

    Thread  - Supplies a pointer to a dispatcher object of type thread.

    Affinity - Supplies the new of set of processors on which the thread
        can run.

Return Value:

    The previous affinity of the specified thread is returned as the function
    value.

--*/

{

    KAFFINITY OldAffinity;
    PKPRCB Prcb;
    PKPROCESS Process;
    ULONG Processor;

#if !defined(NT_UP)

    ULONG IdealProcessor;
    ULONG Index;
    PKNODE Node;
    ULONG NodeNumber;

#endif

    BOOLEAN RequestInterrupt;
    PKTHREAD Thread1;

    //
    // Capture the current affinity of the specified thread and get address
    // of parent process object.
    //

    OldAffinity = Thread->UserAffinity;
    Process = Thread->Process;

    //
    // If new affinity is not a proper subset of the parent process affinity
    // or the new affinity is null, then bugcheck.
    //

    if (((Affinity & Process->Affinity) != (Affinity)) || (!Affinity)) {
        KeBugCheck(INVALID_AFFINITY_SET);
    }

    //
    // Set the thread user affinity to the specified value.
    //

    Thread->UserAffinity = Affinity;

    //
    // If the thread user ideal processor is not a member of the new affinity
    // set, then recompute the user ideal processor.
    //

#if !defined(NT_UP)

    if ((Affinity & AFFINITY_MASK(Thread->UserIdealProcessor)) == 0) {
        if (KeNumberNodes > 1) {
            NodeNumber = (KeProcessNodeSeed + 1) % KeNumberNodes;
            KeProcessNodeSeed = (UCHAR)NodeNumber;
            Index = 0;
            do {      
                if ((KeNodeBlock[NodeNumber]->ProcessorMask & Affinity) != 0) {
                    break;
                }
    
                Index += 1;
                NodeNumber += 1;
                if (NodeNumber >= KeNumberNodes) {
                    NodeNumber -= KeNumberNodes;
                }
    
            } while (Index < KeNumberNodes);
    
        } else {
            NodeNumber = 0;
        }
    
        Node = KeNodeBlock[NodeNumber];

        ASSERT((Node->ProcessorMask & Affinity) != 0);

        IdealProcessor = KeFindNextRightSetAffinity(Node->Seed,
                                                    Node->ProcessorMask & Affinity);
    
        Thread->UserIdealProcessor = (UCHAR)IdealProcessor;
        Node->Seed = (UCHAR)IdealProcessor;
    }

#endif

    //
    // If the thread is not current executing with system affinity active,
    // then set the thread current affinity and switch on the thread state.
    //

    if (Thread->SystemAffinityActive == FALSE) {

        //
        // Switch on the thread state.
        //

        KiAcquireThreadLock(Thread);
        do {
            switch (Thread->State) {

                //
                // Ready State.
                //
                // If the thread is not in the process ready queue, then
                // remove the thread from its current dispatcher ready
                // queue and ready the thread for execution.
                //

            case Ready:
                if (Thread->ProcessReadyQueue == FALSE) {
                    Processor = Thread->NextProcessor;
                    Prcb = KiProcessorBlock[Processor];
                    KiAcquirePrcbLock(Prcb);
                    if ((Thread->State == Ready) &&
                        (Thread->NextProcessor == Prcb->Number)) {

                        Thread->Affinity = Affinity;

#if !defined(NT_UP)

                        Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                        ASSERT((Prcb->ReadySummary & PRIORITY_MASK(Thread->Priority)) != 0);

                        if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                            Prcb->ReadySummary ^= PRIORITY_MASK(Thread->Priority);
                        }
        
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);

                    } else {
                        KiReleasePrcbLock(Prcb);
                        continue;
                    }

                } else {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                }

                break;
    
                //
                // Standby State.
                //
                // If the target processor is not in the new affinity set,
                // then select a new thread to run on the target processor,
                // and ready the thread for execution.
                //
    
            case Standby:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->NextThread) {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif
        
                    if ((Prcb->SetMember & Affinity) == 0) {
                        Thread1 = KiSelectNextThread(Prcb);
                        Thread1->State = Standby;
                        Prcb->NextThread = Thread1;
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);
        
                    } else {
                        KiReleasePrcbLock(Prcb);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;
    
                //
                // Running State.
                //
                // If the target processor is not in the new affinity set and
                // another thread has not already been selected for execution
                // on the target processor, then select a new thread for the
                // target processor, and cause the target processor to be
                // redispatched.
                //
    
            case Running:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                RequestInterrupt = FALSE;
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->CurrentThread) {
                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                    if (((Prcb->SetMember & Affinity) == 0) &&
                        (Prcb->NextThread == NULL)) {
        
                        Thread1 = KiSelectNextThread(Prcb);
                        Thread1->State = Standby;
                        Prcb->NextThread = Thread1;
                        RequestInterrupt = TRUE;
                    }

                    KiReleasePrcbLock(Prcb);
                    if (RequestInterrupt == TRUE) {
                        KiRequestDispatchInterrupt(Processor);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;

                //
                // Deferred Ready State:
                //
                // Set the affinity of the thread in a deferred ready state.
                //

            case DeferredReady:
                Processor = Thread->DeferredProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if ((Thread->State == DeferredReady) &&
                    (Thread->DeferredProcessor == Processor)) {

                    Thread->Affinity = Affinity;

#if !defined(NT_UP)

                    Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;

                //
                // Initialized, GateWait, Terminated, Waiting, Transition
                // case - For these states it is sufficient to just set the
                // new thread affinity.
                //
    
            default:
                Thread->Affinity = Affinity;

#if !defined(NT_UP)

                Thread->IdealProcessor = Thread->UserIdealProcessor;

#endif

                break;
            }

            break;

        } while (TRUE);

        KiReleaseThreadLock(Thread);
    }

    //
    // Return the previous user affinity.
    //

    return OldAffinity;
}

VOID
KiSetInternalEvent (
    IN PKEVENT Event,
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function sets an internal event or unwaits the specfied thread.

    N.B. The dispatcher lock must be held to call this routine.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PLIST_ENTRY WaitEntry;

    //
    // If the swap event wait queue is not empty, then unwait the swap
    // thread (there is only one swap thread). Otherwise, set the swap
    // event.
    //

    WaitEntry = Event->Header.WaitListHead.Flink;
    if (WaitEntry != &Event->Header.WaitListHead) {
        KiUnwaitThread(Thread, 0, BALANCE_INCREMENT);

    } else {
        Event->Header.SignalState = 1;
    }

    return;
}

VOID
FASTCALL
KiSetPriorityThread (
    __inout PKTHREAD Thread,
    __in KPRIORITY Priority
    )

/*++

Routine Description:

    This function set the priority of the specified thread to the specified
    value. If the thread is in the standby or running state, then the processor
    may be redispatched. If the thread is in the ready state, then some other
    thread may be preempted.

    N.B. The thread lock is held on entry and exit to this function.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

    Priority - Supplies the new thread priority value.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;
    ULONG Processor;
    BOOLEAN RequestInterrupt;
    KPRIORITY ThreadPriority;
    PKTHREAD Thread1;

    ASSERT((Priority >= 0) && (Priority <= HIGH_PRIORITY));

    //
    // If the new priority is not equal to the old priority, then set the
    // new priority of the thread and redispatch a processor if necessary.
    //

    if (Priority != Thread->Priority) {

        //
        //
        // Switch on the thread state.

        do {
            switch (Thread->State) {
    
                //
                // Ready State.
                //
                // If the thread is not in the process ready queue, then
                // remove the thread from its current dispatcher ready
                // queue and ready the thread for execution.
                //
    
            case Ready:
                if (Thread->ProcessReadyQueue == FALSE) {
                    Processor = Thread->NextProcessor;
                    Prcb = KiProcessorBlock[Processor];
                    KiAcquirePrcbLock(Prcb);
                    if ((Thread->State == Ready) &&
                        (Thread->NextProcessor == Prcb->Number)) {

                        ASSERT((Prcb->ReadySummary & PRIORITY_MASK(Thread->Priority)) != 0);

                        if (RemoveEntryList(&Thread->WaitListEntry) != FALSE) {
                            Prcb->ReadySummary ^= PRIORITY_MASK(Thread->Priority);
                        }

                        Thread->Priority = (SCHAR)Priority;
                        KiInsertDeferredReadyList(Thread);
                        KiReleasePrcbLock(Prcb);
    
                    } else {
                        KiReleasePrcbLock(Prcb);
                        continue;
                    }

                } else {
                    Thread->Priority = (SCHAR)Priority;
                }
    
                break;
    
                //
                // Standby State.
                // 
                // If the thread's priority is being lowered, then attempt
                // to find another thread to execute on the target processor.
                //
    
            case Standby:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->NextThread) {
                    ThreadPriority = Thread->Priority;
                    Thread->Priority = (SCHAR)Priority;
                    if (Priority < ThreadPriority) {
                        if ((Thread1 = KiSelectReadyThread(Priority + 1, Prcb)) != NULL) {
                            Thread1->State = Standby;
                            Prcb->NextThread = Thread1;
                            KiInsertDeferredReadyList(Thread);
                            KiReleasePrcbLock(Prcb);
    
                        } else {
                            KiReleasePrcbLock(Prcb);
                        }

                    } else {
                        KiReleasePrcbLock(Prcb);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;
    
                //
                // Running State.
                //
                // If the thread's priority is being lowered, then attempt
                // to find another thread to execute on the target processor.
                //
    
            case Running:
                Processor = Thread->NextProcessor;
                Prcb = KiProcessorBlock[Processor];
                RequestInterrupt = FALSE;
                KiAcquirePrcbLock(Prcb);
                if (Thread == Prcb->CurrentThread) {
                    ThreadPriority = Thread->Priority;
                    Thread->Priority = (SCHAR)Priority;
                    if ((Priority < ThreadPriority) &&
                        (Prcb->NextThread == NULL)) {

                        if ((Thread1 = KiSelectReadyThread(Priority + 1, Prcb)) != NULL) {
                            Thread1->State = Standby;
                            Prcb->NextThread = Thread1;
                            RequestInterrupt = TRUE;
                        }
                    }

                    KiReleasePrcbLock(Prcb);
                    if (RequestInterrupt == TRUE) {
                        KiRequestDispatchInterrupt(Processor);
                    }

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }
    
                break;

                //
                // Deferred Ready State:
                //
                // Set the priority of the thread in a deferred ready state.
                //

            case DeferredReady:
                Processor = Thread->DeferredProcessor;
                Prcb = KiProcessorBlock[Processor];
                KiAcquirePrcbLock(Prcb);
                if ((Thread->State == DeferredReady) &&
                    (Thread->DeferredProcessor == Processor)) {

                    Thread->Priority = (SCHAR)Priority;
                    KiReleasePrcbLock(Prcb);

                } else {
                    KiReleasePrcbLock(Prcb);
                    continue;
                }

                break;

                //
                // Initialized, GateWait, Terminated, Waiting, Transition
                // case - For these states it is sufficient to just set the
                // new thread priority.
                //
    
            default:
                Thread->Priority = (SCHAR)Priority;
                break;
            }

            break;

        } while(TRUE);
    }

    return;
}

VOID
KiSuspendThread (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function is the kernel routine for the builtin suspend APC of a
    thread. It is executed as the result of queuing the builtin suspend
    APC and suspends thread execution by waiting nonalerable on the thread's
    builtin suspend semaphore. When the thread is resumed, execution of
    thread is continued by simply returning.

Arguments:

    NormalContext - Not used.

    SystemArgument1 - Not used.

    SystemArgument2 - Not used.

Return Value:

    None.

--*/

{

    PKTHREAD Thread;

    UNREFERENCED_PARAMETER(NormalContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    //
    // Get the address of the current thread object and Wait nonalertable on
    // the thread's builtin suspend semaphore.
    //

    Thread = KeGetCurrentThread();
    KeWaitForSingleObject(&Thread->SuspendSemaphore,
                          Suspended,
                          KernelMode,
                          FALSE,
                          NULL);

    return;
}

LONG_PTR
FASTCALL
KiSwapThread (
    IN PKTHREAD OldThread,
    IN PKPRCB CurrentPrcb
    )

/*++

Routine Description:

    This function selects the next thread to run on the current processor
    and swaps thread context to the selected thread. When the execution
    of the current thread is resumed, the IRQL is lowered to its previous
    value and the wait status is returned as the function value.

    N.B. This function is called with no locks held.

Arguments:

    Thread - Supplies a pointer to the current thread object.

    CurrentPrcb - Supplies a pointer to the current PRCB.

Return Value:

    The wait completion status is returned as the function value.

--*/

{

    PKTHREAD NewThread;
    BOOLEAN Pending;
    KIRQL WaitIrql;
    LONG_PTR WaitStatus;

#if !defined(NT_UP)
      
    LONG Index;
    LONG Limit;
    LONG Number;
    ULONG Processor;
    PKPRCB TargetPrcb;

#endif

    //
    // If the deferred ready list is not empty, then process the list.
    //

#if !defined(NT_UP)

    if (CurrentPrcb->DeferredReadyListHead.Next != NULL) {
        KiProcessDeferredReadyList(CurrentPrcb);
    }

#endif

    //
    // Acquire the current PRCB lock and check if a thread has been already
    // selected to run of this processor.
    //
    // If a thread has already been selected to run on the current processor,
    // then select that thread. Otherwise, attempt to select a thread from
    // the current processor dispatcher ready queues.
    //

    KiAcquirePrcbLock(CurrentPrcb);
    if ((NewThread = CurrentPrcb->NextThread) != NULL) {

        //
        // Clear the next thread address, set the current thread address, and
        // set the thread state to running.
        //

        CurrentPrcb->NextThread = NULL;
        CurrentPrcb->CurrentThread = NewThread;
        NewThread->State = Running;

    } else {

        //
        // Attempt to select a thread from the current processor dispatcher
        // ready queues.
        //

        if ((NewThread = KiSelectReadyThread(0, CurrentPrcb)) != NULL) {
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;

        } else {

            //
            // A thread could not be selected from the current processor
            // dispatcher ready queues. Set the current processor idle while
            // attempting to select a ready thread from any other processor
            // dispatcher ready queue.
            //
            // Setting the current processor idle allows the old thread to
            // masquerade as the idle thread while scanning other processor
            // dispatcher ready queues and avoids forcing the idle thread
            // to perform a complete scan should no suitable thread be found.
            // 

            KiSetIdleSummary(CurrentPrcb->SetMember);

            //
            // On a UP system, select the idle thread as the new thread.
            //
            // On an MP system, attempt to select a thread from another
            // processor's dispatcher ready queues as the new thread.
            //

#if defined(NT_UP)

            NewThread = CurrentPrcb->IdleThread;
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;
        }
    }

#else

            //
            // If all logical processors of the physical processor are idle,
            // then update the idle SMT summary set.
            //

            if (KeIsSMTSetIdle(CurrentPrcb) == TRUE) {
                KiSetSMTSummary(CurrentPrcb->MultiThreadProcessorSet);
            }

            //
            // Release the current PRCB lock and attempt to select a thread
            // from any processor dispatcher ready queues.
            //
            // If this is a multinode system, then start with the processors
            // on the same node. Otherwise, start with the current processor.
            //
            // N.B. It is possible to perform the below loop with minimal
            //      releases of the current PRCB lock. However, this limits
            //      parallelism.
            //

            KiReleasePrcbLock(CurrentPrcb);
            Processor = CurrentPrcb->Number;
            Index = Processor;
            if (KeNumberNodes > 1) {
                KeFindFirstSetLeftAffinity(CurrentPrcb->ParentNode->ProcessorMask,
                                           (PULONG)&Index);
            }

            Limit = KeNumberProcessors - 1;
            Number = Limit;

            ASSERT(Index <= Limit);

            do {
                TargetPrcb = KiProcessorBlock[Index];
                if (CurrentPrcb != TargetPrcb) {
                    if (TargetPrcb->ReadySummary != 0) {

                        //
                        // Acquire the current and target PRCB locks.
                        //

                        KiAcquireTwoPrcbLocks(CurrentPrcb, TargetPrcb);

                        //
                        // If a new thread has not been selected to run on
                        // the current processor, then attempt to select a
                        // thread to run on the current processor.
                        //

                        if ((NewThread = CurrentPrcb->NextThread) == NULL) {
                            if ((TargetPrcb->ReadySummary != 0) &&
                                (NewThread = KiFindReadyThread(Processor,
                                                               TargetPrcb)) != NULL) {
    
                                //
                                // A new thread has been found to run on the
                                // current processor. 
                                //
    
                                NewThread->State = Running;
                                KiReleasePrcbLock(TargetPrcb);
                                CurrentPrcb->CurrentThread = NewThread;

                                //
                                // Clear idle on the current processor and
                                // update the idle summary SMT set to indicate
                                // the physical processor is not entirely idle.
                                //

                                KiClearIdleSummary(AFFINITY_MASK(Processor));
                                KiClearSMTSummary(CurrentPrcb->MultiThreadProcessorSet);
                                goto ThreadFound;

                            } else {
                                KiReleasePrcbLock(CurrentPrcb);
                                KiReleasePrcbLock(TargetPrcb);
                            }

                        } else {

                            //
                            // A thread has already been selected to run on
                            // the current processor. It is possible that
                            // the thread is the idle thread due to a state
                            // change that made a scheduled runable thread
                            // unrunable.
                            //
                            // N.B. If the idle thread is selected, then the
                            //      current processor is idle. Otherwise,
                            //      the current processor is not idle.
                            //

                            if (NewThread == CurrentPrcb->IdleThread) {
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->IdleSchedule = FALSE;
                                KiReleasePrcbLock(CurrentPrcb);
                                KiReleasePrcbLock(TargetPrcb);
                                continue;

                            } else {
                                NewThread->State = Running;
                                KiReleasePrcbLock(TargetPrcb);
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->CurrentThread = NewThread;
                                goto ThreadFound;
                            }
                        }
                    }
                }
        
                Index -= 1;
                if (Index < 0) {
                    Index = Limit;
                }
        
                Number -= 1;
            } while (Number >= 0);

            //
            // Acquire the current PRCB lock and if a thread has not been
            // selected to run on the current processor, then select the
            // idle thread.
            //

            KiAcquirePrcbLock(CurrentPrcb);
            if ((NewThread = CurrentPrcb->NextThread) != NULL) {
                CurrentPrcb->NextThread = NULL;

            } else {
                NewThread = CurrentPrcb->IdleThread;
            }

            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;
        }
    }

    //
    // If the new thread is not the idle thread, and the old thread is not
    // the new thread, and the new thread has not finished saving context,
    // then avoid a deadlock by scheduling the new thread via the idle thread.
    //

ThreadFound:;

    if ((NewThread != CurrentPrcb->IdleThread) &&
        (NewThread != OldThread) &&
        (NewThread->SwapBusy != FALSE)) {

        NewThread->State = Standby;
        CurrentPrcb->NextThread = NewThread;
        NewThread = CurrentPrcb->IdleThread;
        NewThread->State = Running;
        CurrentPrcb->CurrentThread = NewThread;
    }

#endif

    //
    // Release the current PRCB lock.
    //

    ASSERT(OldThread != CurrentPrcb->IdleThread);

    KiReleasePrcbLock(CurrentPrcb);

    //
    // If the old thread is the same as the new thread, then the current
    // thread has been readied for execution before the context was saved.
    // Release the old thread lock, and set the APC pending value. Otherwise,
    // swap context to the new thread.
    //

    WaitIrql = OldThread->WaitIrql;

#if !defined(NT_UP)

    if (OldThread == NewThread) {
        KiSetContextSwapIdle(OldThread);
        Pending = (BOOLEAN)((NewThread->ApcState.KernelApcPending != FALSE) &&
                            (NewThread->SpecialApcDisable == 0) &&
                            (WaitIrql == 0));

    } else {
        Pending = KiSwapContext(OldThread, NewThread);
    }

#else

    Pending = KiSwapContext(OldThread, NewThread);

#endif

    //
    // If a kernel APC should be delivered, then deliver it now.
    //

    WaitStatus = OldThread->WaitStatus;
    if (Pending != FALSE) {
        KeLowerIrql(APC_LEVEL);
        KiDeliverApc(KernelMode, NULL, NULL);

        ASSERT(WaitIrql == 0);
    }

    //
    // Lower IRQL to its level before the wait operation and return the wait
    // status.
    //

    KeLowerIrql(WaitIrql);
    return WaitStatus;
}

ULONG
KeFindNextRightSetAffinity (
    __in ULONG Number,
    __in KAFFINITY Set
    )

/*++

Routine Description:

    This function locates the left most set bit in the set immediately to
    the right of the specified bit. If no bits are set to the right of the
    specified bit, then the left most set bit in the complete set is located.

    N.B. Set must contain at least one bit.

Arguments:

    Number - Supplies the bit number from which the search to to begin.

    Set - Supplies the bit mask to search.

Return Value:

    The number of the found set bit is returned as the function value.

--*/

{

    KAFFINITY NewSet;
    ULONG Temp;

    ASSERT(Set != 0);

    //
    // Get a mask with all bits to the right of bit "Number" set.
    //

    NewSet = (AFFINITY_MASK(Number) - 1) & Set;

    //
    // If no bits are set to the right of the specified bit number, then use
    // the complete set.
    //

    if (NewSet == 0) {
        NewSet = Set;
    }

    //
    // Find leftmost bit in this set.
    //

    KeFindFirstSetLeftAffinity(NewSet, &Temp);
    return Temp;
}

