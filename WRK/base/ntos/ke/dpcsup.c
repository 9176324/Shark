/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    dpcsup.c

Abstract:

    This module contains the support routines for the system DPC objects.
    Functions are provided to process quantum end, the power notification
    queue, and timer expiration.

Environment:

    IRQL DISPATCH_LEVEL.

--*/

#include "ki.h"

//
// Define DPC entry structure and maximum DPC List size.
//

#define MAXIMUM_DPC_TABLE_SIZE 16

typedef struct _DPC_ENTRY {
    PRKDPC Dpc;
    PKDEFERRED_ROUTINE Routine;
    PVOID Context;
} DPC_ENTRY, *PDPC_ENTRY;

//
// Define maximum number of timers that can be examined or processed before
// dropping the dispatcher database lock.
//

#define MAXIMUM_TIMERS_EXAMINED 24
#define MAXIMUM_TIMERS_PROCESSED 4

VOID
KiExecuteDpc (
    IN PVOID Context
    )

/*++

Routine Description:

    This function is executed by the DPC thread for each processor. DPC
    threads are started during kernel initialization after having started
    all processors and it is determined that the host configuation should
    execute threaded DPCs in a DPC thread.
    
Arguments:

    Context - Supplies a pointer to the processor control block for the
        processor on which the DPC thread is to run.

Return Value:

    None.

--*/

{

    PKDPC Dpc;
    PVOID DeferredContext;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PERFINFO_DPC_INFORMATION DpcInformation;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    LOGICAL Logging;
    KIRQL OldIrql;
    PKPRCB Prcb;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PKTHREAD Thread;
    LARGE_INTEGER TimeStamp = {0};

    //
    // Get PRCB and set the DPC thread address.
    //

    Prcb = Context; 
    Thread = KeGetCurrentThread();
    Prcb->DpcThread = Thread;

    //
    // Set the DPC thread priority to the highest level, set the thread
    // affinity, and enable threaded DPCs on this processor.
    //

    KeSetPriorityThread(Thread, HIGH_PRIORITY);
    KeSetSystemAffinityThread(Prcb->SetMember);
    Prcb->ThreadDpcEnable = TRUE;

    //
    // Loop processing DPC list entries until the specified DPC list is empty.
    //
    // N.B. This following code appears to have a redundant loop, but it does
    //      not. The point of this code is to avoid as many dispatch interrupts
    //      as possible.
    //

    ListHead = &Prcb->DpcData[DPC_THREADED].DpcListHead;
    do {
        Prcb->DpcThreadActive = TRUE;

        //
        // If the DPC list is not empty, then process the DPC list.
        //

        if (Prcb->DpcData[DPC_THREADED].DpcQueueDepth != 0) {

            //
            // Acquire the DPC lock for the current processor and check if
            // the DPC list is empty. If the DPC list is not empty, then
            // remove the first entry from the DPC list, capture the DPC
            // parameters, set the DPC inserted state false, decrement the
            // DPC queue depth, release the DPC lock, enable interrupts, and
            // call the specified DPC routine. Otherwise, release the DPC
            // lock and enable interrupts.
            //

            Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);
            do {
                KeRaiseIrql(HIGH_LEVEL, &OldIrql);
                KeAcquireSpinLockAtDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                Entry = ListHead->Flink;
                if (Entry != ListHead) {
                    RemoveEntryList(Entry);
                    Dpc = CONTAINING_RECORD(Entry, KDPC, DpcListEntry);
                    DeferredRoutine = Dpc->DeferredRoutine;
                    DeferredContext = Dpc->DeferredContext;
                    SystemArgument1 = Dpc->SystemArgument1;
                    SystemArgument2 = Dpc->SystemArgument2;
                    Dpc->DpcData = NULL;
                    Prcb->DpcData[DPC_THREADED].DpcQueueDepth -= 1;
                    KeReleaseSpinLockFromDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                    KeLowerIrql(OldIrql);

                    //
                    // If event tracing is enabled, capture the start time.
                    //

                    if (Logging != FALSE) {
                        PerfTimeStamp(TimeStamp);
                    }

                    //
                    // Call the DPC routine.
                    //

                    (DeferredRoutine)(Dpc,
                                      DeferredContext,
                                      SystemArgument1,
                                      SystemArgument2);

                    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

                    ASSERT(Thread->Affinity == Prcb->SetMember);

                    ASSERT(Thread->Priority == HIGH_PRIORITY);

                    //
                    // If event tracing is enabled, then log the start time
                    // and routine address.
                    //

                    if (Logging != FALSE) {
                        DpcInformation.InitialTime = TimeStamp.QuadPart;
                        DpcInformation.DpcRoutine = (PVOID)(ULONG_PTR)DeferredRoutine;
                        PerfInfoLogBytes(PERFINFO_LOG_TYPE_DPC,
                                         &DpcInformation,
                                         sizeof(DpcInformation));
                    }

                } else {

                    ASSERT(Prcb->DpcData[DPC_THREADED].DpcQueueDepth == 0);

                    KeReleaseSpinLockFromDpcLevel(&Prcb->DpcData[DPC_THREADED].DpcLock);
                    KeLowerIrql(OldIrql);
                }

            } while (Prcb->DpcData[DPC_THREADED].DpcQueueDepth != 0);
        }

        Prcb->DpcThreadActive = FALSE;
        Prcb->DpcThreadRequested = FALSE;
        KeMemoryBarrier();

        //
        // If the thread DPC list is empty, then wait until the DPC event
        // for the current processor is set.
        //

        if (Prcb->DpcData[DPC_THREADED].DpcQueueDepth == 0) {
            KeWaitForSingleObject(&Prcb->DpcEvent, 
                                  Suspended,               
                                  KernelMode, 
                                  FALSE,
                                  NULL);
     
        }

    } while (TRUE);

    return;
}

VOID
KiQuantumEnd (
    VOID
    )

/*++

Routine Description:

    This function is called when a quantum end event occurs on the current
    processor. Its function is to determine whether the thread priority should
    be decremented and whether a redispatch of the processor should occur.

    N.B. This function is called at DISPATCH level and returns at DISPATCH
         level.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;
    PKPROCESS Process;
    PRKTHREAD Thread;
    PRKTHREAD NewThread;

    //
    // If DPC thread activation is requested, then set the DPC event.
    //

    Prcb = KeGetCurrentPrcb();
    Thread = KeGetCurrentThread();
    if (InterlockedExchange(&Prcb->DpcSetEventRequest, FALSE) == TRUE) {
        KeSetEvent(&Prcb->DpcEvent, 0, FALSE);
    }

    //
    // Raise IRQL to SYNCH level, acquire the thread lock, and acquire the
    // PRCB lock.
    //
    // If the quantum has expired for the current thread, then update its
    // quantum and priority.
    //

    KeRaiseIrqlToSynchLevel();
    KiAcquireThreadLock(Thread);
    KiAcquirePrcbLock(Prcb);
    if (Thread->Quantum <= 0) {

        //
        // If quantum runout is disabled for the thread's process and
        // the thread is running at a realtime priority, then set the
        // thread quantum to the highest value and do not round robin
        // at the thread's priority level. Otherwise, reset the thread
        // quantum and decay the thread's priority as appropriate.
        //

        Process = Thread->ApcState.Process;
        if ((Process->DisableQuantum != FALSE) &&
            (Thread->Priority >= LOW_REALTIME_PRIORITY)) {

            Thread->Quantum = MAXCHAR;

        } else {
            Thread->Quantum = Thread->QuantumReset;

            //
            // Compute the new thread priority and attempt to reschedule the
            // current processor.
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
    // Release the thread lock.
    //
    // If a thread was scheduled for execution on the current processor, then
    // acquire the PRCB lock, set the current thread to the new thread, set
    // next thread to NULL, set the thread state to running, release the PRCB
    // lock, set the wait reason, ready the old thread, and swap context to
    // the new thread.
    //

    KiReleaseThreadLock(Thread);
    if (Prcb->NextThread != NULL) {
        KiSetContextSwapBusy(Thread);
        NewThread = Prcb->NextThread;
        Prcb->NextThread = NULL;
        Prcb->CurrentThread = NewThread;
        NewThread->State = Running;
        Thread->WaitReason = WrQuantumEnd;
        KxQueueReadyThread(Thread, Prcb);
        Thread->WaitIrql = APC_LEVEL;
        KiSwapContext(Thread, NewThread);

    } else {
        KiReleasePrcbLock(Prcb);
    }

    //
    // Lower IRQL to DISPATCH level and return.
    //

    KeLowerIrql(DISPATCH_LEVEL);
    return;
}

#if DBG

VOID
KiCheckTimerTable (
    IN ULARGE_INTEGER CurrentTime
    )

{

    ULONG Index;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    PKTIMER Timer;

    //
    // Raise IRQL to highest level and scan timer table for timers that
    // have expired.
    //

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    Index = 0;
    do {
        ListHead = &KiTimerTableListHead[Index].Entry;
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead) {
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            NextEntry = NextEntry->Flink;
            if (Timer->DueTime.QuadPart <= CurrentTime.QuadPart) {

                //
                // If the timer expiration DPC is queued, then the time has
                // been change and the DPC has not yet had the chance to run
                // and clear out the expired timers.
                //

                if ((KeGetCurrentPrcb()->TimerRequest == 0) &&
                    *((volatile PKSPIN_LOCK *)(&KiTimerExpireDpc.DpcData)) == NULL) {
                    DbgBreakPoint();
                }
            }
        }

        Index += 1;
    } while(Index < TIMER_TABLE_SIZE);

    //
    // Lower IRQL to the previous level.
    //

    KeLowerIrql(OldIrql);
    return;
}

#endif

FORCEINLINE
VOID
KiProcessTimerDpcTable (
    IN PULARGE_INTEGER SystemTime,
    IN PDPC_ENTRY DpcTable,
    IN ULONG Count
    )

/**++

Routine Description:

    This function processes the time DPC table which is a array of DPCs that
    are to be called on the current processor.

    N.B. This routine is entered with the dispatcher database locked.

    N.B. This routine returns with the dispatcher database unlocked.

Arguments:

    SystemTime - Supplies a pointer to the timer expiration time.

    DpcTable - Supplies a pointer to an array of DPC entries.

    Count - Supplies a count of the number of entries in the DPC table.

Return Value:

    None.

--*/

{

    PERFINFO_DPC_INFORMATION DpcInformation;
    LOGICAL Logging;
    LARGE_INTEGER TimeStamp = {0};

    //
    // Unlock the dispatcher database and lower IRQL to dispatch level.
    //

    KiUnlockDispatcherDatabase(DISPATCH_LEVEL);

    //
    // Process DPC table entries.
    //

    Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);
    while (Count != 0) {

        //
        // Reset the debug DPC count to avoid a timeout and breakpoint.
        //

#if DBG

        KeGetCurrentPrcb()->DebugDpcTime = 0;

#endif

        //
        // If event tracing is enabled, capture the start time.
        //

        if (Logging != FALSE) {
            PerfTimeStamp(TimeStamp);
        }

        //
        // Call the DPC routine.
        //

        (DpcTable->Routine)(DpcTable->Dpc,
                            DpcTable->Context,
                            ULongToPtr(SystemTime->LowPart),
                            ULongToPtr(SystemTime->HighPart));

        //
        // If event tracing is enabled, then log the start time and
        // routine address.
        //

        if (Logging != FALSE) {
            DpcInformation.InitialTime = TimeStamp.QuadPart;
            DpcInformation.DpcRoutine = (PVOID)(ULONG_PTR)DpcTable->Routine;
            PerfInfoLogBytes(PERFINFO_LOG_TYPE_TIMERDPC,
                             &DpcInformation,
                             sizeof(DpcInformation));
        }

        DpcTable += 1;
        Count -= 1;
    }

    return;
}

VOID
KiTimerExpiration (
    IN PKDPC TimerDpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function is called when the clock interupt routine discovers that
    a timer has expired.

    N.B. This function executes on the same processor that receives clock
         interrupts.

Arguments:

    TimerDpc - Not used.

    DeferredContext - Not used.

    SystemArgument1 - Supplies the starting timer table index value to
        use for the timer table scan.

    SystemArgument2 - Not used.

Return Value:

    None.

--*/

{

    ULARGE_INTEGER CurrentTime;
    ULONG DpcCount;
    PKDPC Dpc;
    DPC_ENTRY DpcTable[MAXIMUM_TIMERS_PROCESSED];
    KIRQL DummyIrql;
    LONG HandLimit;
    LONG Index;
    LARGE_INTEGER Interval;
    PLIST_ENTRY ListHead;
    PKSPIN_LOCK_QUEUE LockQueue;
    PLIST_ENTRY NextEntry;
    KIRQL OldIrql;
    LONG Period;

#if !defined(NT_UP) || defined(_WIN64)

    PKPRCB Prcb = KeGetCurrentPrcb();

#endif

    ULARGE_INTEGER SystemTime;
    PKTIMER Timer;
    ULONG TimersExamined;
    ULONG TimersProcessed;

    UNREFERENCED_PARAMETER(TimerDpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument2);

    //
    // Capture the timer expiration time, the current interrupt time, and
    // the low tick count.
    //
    // N.B. Interrupts are disabled to ensure that interrupt activity on the
    //      current processor does not cause the values read to be skewed.
    //

    _disable();
    KiQuerySystemTime((PLARGE_INTEGER)&SystemTime);
    KiQueryInterruptTime((PLARGE_INTEGER)&CurrentTime);
    HandLimit = (LONG)KiQueryLowTickCount();
    _enable();

    //
    // If the timer table has not wrapped, then start with the specified
    // timer table index value, and scan for timer entries that have expired.
    // Otherwise, start with the specified timer table index value and scan
    // the entire table for timer entries that have expired.
    //
    // N.B. This later condition exists when DPC processing is blocked for a
    //      period longer than one round trip throught the timer table.
    //
    // N.B. The current instance of the timer expiration execution will only
    //      process the timer queue entries specified by the computed index
    //      and hand limit. If another timer expires while the current scan
    //      is in progress, then another scan will occur when the current one
    //      is finished.
    //

    Index = PtrToLong(SystemArgument1);
    if ((ULONG)(HandLimit - Index) >= TIMER_TABLE_SIZE) {
        HandLimit = Index + TIMER_TABLE_SIZE - 1;
    }

    Index -= 1;
    HandLimit &= (TIMER_TABLE_SIZE - 1);

    //
    // Acquire the dispatcher database lock and read the current interrupt
    // time to determine which timers have expired.
    //

    DpcCount = 0;
    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;
    KiLockDispatcherDatabase(&OldIrql);
    do {
        Index = (Index + 1) & (TIMER_TABLE_SIZE - 1);
        ListHead = &KiTimerTableListHead[Index].Entry;
        while (ListHead != ListHead->Flink) {
            LockQueue = KiAcquireTimerTableLock(Index);
            NextEntry = ListHead->Flink;
            Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);
            TimersExamined -= 1;
            if ((NextEntry != ListHead) &&
                (Timer->DueTime.QuadPart <= CurrentTime.QuadPart)) {

                //
                // The next timer in the current timer list has expired.
                // Remove the entry from the timer tree and set the signal
                // state of the timer.
                //

                TimersProcessed -= 1;
                KiRemoveEntryTimer(Timer);
                Timer->Header.Inserted = FALSE;
                KiReleaseTimerTableLock(LockQueue);
                Timer->Header.SignalState = 1;

                //
                // Capture the DPC and period fields from the timer object.
                // Once wait test is called, the timer must not be touched
                // again unless it is periodic. The reason for this is that
                // a thread may allocate a timer on its local stack and wait
                // on it. Wait test can cause that thread to immediately
                // start running on another processor on an MP system. If
                // the thread returns, then the timer will be corrupted.
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
                // If the timer is periodic, then compute the next interval
                // time and reinsert the timer in the timer tree.
                //
                // N.B. Even though the timer insertion is relative, it can
                //      still fail if the period of the timer elapses in
                //      between computing the time and inserting the timer.
                //      If this happens, then the insertion is retried.
                //

                if (Period != 0) {
                    Interval.QuadPart = Int32x32To64(Period, - 10 * 1000);
                    do {
                    } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
                }

                //
                // If a DPC is specified, then insert it in the target
                // processor's DPC queue or capture the parameters in
                // the DPC table for subsequent execution on the current
                // processor.
                //

                if (Dpc != NULL) {

#if defined(NT_UP)

                    DpcTable[DpcCount].Dpc = Dpc;
                    DpcTable[DpcCount].Routine = Dpc->DeferredRoutine;
                    DpcTable[DpcCount].Context = Dpc->DeferredContext;
                    DpcCount += 1;

#else

                    if (((Dpc->Number >= MAXIMUM_PROCESSORS) &&
                         (((LONG)Dpc->Number - MAXIMUM_PROCESSORS) != Prcb->Number)) ||
                        ((Dpc->Type == (UCHAR)ThreadedDpcObject) &&
                         (Prcb->ThreadDpcEnable != FALSE))) {

                        KeInsertQueueDpc(Dpc,
                                         ULongToPtr(SystemTime.LowPart),
                                         ULongToPtr(SystemTime.HighPart));
        
                    } else {
                        DpcTable[DpcCount].Dpc = Dpc;
                        DpcTable[DpcCount].Routine = Dpc->DeferredRoutine;
                        DpcTable[DpcCount].Context = Dpc->DeferredContext;
                        DpcCount += 1;
                    }

#endif

                }

                //
                // If the maximum number of timers have been processed or
                // the maximum number of timers have been examined, then
                // drop the dispatcher lock and process the DPC table.
                //

                if ((TimersProcessed == 0) || (TimersExamined == 0)) {
                    KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);

                    //
                    // Initialize the DPC count, the scan counters, and
                    // acquire the dispatcher database lock.
                    //
                    // N.B. Control is returned with the dispatcher database
                    //      unlocked.
                    //

                    DpcCount = 0;
                    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
                    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;

#if defined(_WIN64)

                    if (KiTryToLockDispatcherDatabase(&DummyIrql) == FALSE) {
                        Prcb->TimerHand = 0x100000000I64 + Index;
                        return;
                    }

#else

                    KiLockDispatcherDatabase(&DummyIrql);

#endif

                }

            } else {

                //
                // If the timer table list is not empty, then set the due time
                // to the first entry in the respective timer table entry.
                //
                // N.B. On the x86 the write of the due time is not atomic.
                //      Therefore, interrupts must be disabled to synchronize
                //      with the clock interrupt so the clock interupt code
                //      does not observe a partial update of the due time.
                //
                // Release the timer table lock.
                //

                if (NextEntry != ListHead) {

                    ASSERT(KiTimerTableListHead[Index].Time.QuadPart <= Timer->DueTime.QuadPart);

#if defined(_X86_)

                    _disable();
                    KiTimerTableListHead[Index].Time.QuadPart = Timer->DueTime.QuadPart;
                    _enable();

#else

                    KiTimerTableListHead[Index].Time.QuadPart = Timer->DueTime.QuadPart;

#endif

                }

                KiReleaseTimerTableLock(LockQueue);

                //
                // If the maximum number of timers have been scanned, then
                // drop the dispatcher lock and process the DPC table.
                //

                if (TimersExamined == 0) {
                    KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);

                    //
                    // Initialize the DPC count, the scan counters, and
                    // acquire the dispatcher database lock.
                    //
                    // N.B. Control is returned with the dispatcher database
                    //      unlocked.
                    //

                    DpcCount = 0;
                    TimersExamined = MAXIMUM_TIMERS_EXAMINED;
                    TimersProcessed = MAXIMUM_TIMERS_PROCESSED;

#if defined(_WIN64)

                    if (KiTryToLockDispatcherDatabase(&DummyIrql) == FALSE) {
                        Prcb->TimerHand = 0x100000000I64 + Index;
                        return;
                    }

#else

                    KiLockDispatcherDatabase(&DummyIrql);

#endif

                }

                break;
            }
        }

    } while(Index != HandLimit);

#if DBG

    if (KeNumberProcessors == 1) {
        KiCheckTimerTable(CurrentTime);
    }

#endif

    //
    // If the DPC table is not empty, then process the remaining DPC table
    // entries and lower IRQL. Otherwise, unlock the dispatcher database.
    //
    // N.B. Control is returned from the DPC processing routine with the
    //      dispatcher database unlocked.
    //

    if (DpcCount != 0) {
        KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], DpcCount);
        if (OldIrql != DISPATCH_LEVEL) {
            KeLowerIrql(OldIrql);
        }

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

    return;
}

VOID
FASTCALL
KiTimerListExpire (
    IN PLIST_ENTRY ExpiredListHead,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This function is called to process a list of timers that have expired.

    N.B. This function is called with the dispatcher database locked and
        returns with the dispatcher database unlocked.

Arguments:

    ExpiredListHead - Supplies a pointer to a list of timers that have
        expired.

    OldIrql - Supplies the previous IRQL.

Return Value:

    None.

--*/

{

    LONG Count;
    PKDPC Dpc;
    DPC_ENTRY DpcTable[MAXIMUM_DPC_TABLE_SIZE];
    LARGE_INTEGER Interval;
    KIRQL OldIrql1;

#if !defined(NT_UP)

    PKPRCB Prcb = KeGetCurrentPrcb();

#endif

    ULARGE_INTEGER SystemTime;
    PKTIMER Timer;
    LONG Period;

    //
    // Capture the timer expiration time.
    //

    KiQuerySystemTime((PLARGE_INTEGER)&SystemTime);

    //
    // Remove the next timer from the expired timer list, set the state of
    // the timer to signaled, reinsert the timer in the timer tree if it is
    // periodic, and optionally call the DPC routine if one is specified.
    //

RestartScan:
    Count = 0;
    while (ExpiredListHead->Flink != ExpiredListHead) {
        Timer = CONTAINING_RECORD(ExpiredListHead->Flink, KTIMER, TimerListEntry);
        RemoveEntryList(&Timer->TimerListEntry);

#if DBG

        Timer->TimerListEntry.Flink = NULL;
        Timer->TimerListEntry.Blink = NULL;

#endif

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
        // If the timer is periodic, then compute the next interval time
        // and reinsert the timer in the timer tree.
        //
        // N.B. Even though the timer insertion is relative, it can still
        //      fail if the period of the timer elapses in between computing
        //      the time and inserting the timer. If this happens, then the
        //      insertion is retried.
        //

        if (Period != 0) {
            Interval.QuadPart = Int32x32To64(Period, - 10 * 1000);
            do {
            } while (KiInsertTreeTimer(Timer, Interval) == FALSE);
        }

        //
        // If a DPC is specified, then insert it in the target  processor's
        // DPC queue or capture the parameters in the DPC table for subsequent
        // execution on the current processor.
        //

        if (Dpc != NULL) {

            //
            // If the DPC is explicitly targeted to another processor, then
            // queue the DPC to the target processor. Otherwise, capture the
            // DPC parameters for execution on the current processor.
            //

#if defined(NT_UP)

            DpcTable[Count].Dpc = Dpc;
            DpcTable[Count].Routine = Dpc->DeferredRoutine;
            DpcTable[Count].Context = Dpc->DeferredContext;
            Count += 1;
            if (Count == MAXIMUM_DPC_TABLE_SIZE) {
                break;
            }

#else

            if (((Dpc->Number >= MAXIMUM_PROCESSORS) &&
                 (((LONG)Dpc->Number - MAXIMUM_PROCESSORS) != Prcb->Number)) ||
                ((Dpc->Type == (UCHAR)ThreadedDpcObject) &&
                 (Prcb->ThreadDpcEnable != FALSE))) {

                KeInsertQueueDpc(Dpc,
                                 ULongToPtr(SystemTime.LowPart),
                                 ULongToPtr(SystemTime.HighPart));
        
            } else {
                DpcTable[Count].Dpc = Dpc;
                DpcTable[Count].Routine = Dpc->DeferredRoutine;
                DpcTable[Count].Context = Dpc->DeferredContext;
                Count += 1;
                if (Count == MAXIMUM_DPC_TABLE_SIZE) {
                    break;
                }
            }

#endif

        }
    }

    //
    // Unlock the dispatcher database and process DPC list entries.
    //

    if (Count != 0) {
        KiProcessTimerDpcTable(&SystemTime, &DpcTable[0], Count);

        //
        // If processing of the expired timer list was terminated because
        // the DPC List was full, then process any remaining entries.
        //

        if (Count == MAXIMUM_DPC_TABLE_SIZE) {
            KiLockDispatcherDatabase(&OldIrql1);
            goto RestartScan;
        }

        KeLowerIrql(OldIrql);

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

    return;
}

VOID
FASTCALL
KiRetireDpcList (
    PKPRCB Prcb
    )

/*++

Routine Description:

    This function processes the DPC list for the specified processor,
    processes timer expiration, and processes the deferred ready list.

    N.B. This function is entered with interrupts disabled and exits with
         interrupts disabled.

Arguments:

    Prcb - Supplies the address of the processor block.

Return Value:

    None.

--*/

{

    PKDPC Dpc;
    PKDPC_DATA DpcData;
    PVOID DeferredContext;
    PKDEFERRED_ROUTINE DeferredRoutine;
    PERFINFO_DPC_INFORMATION DpcInformation;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    LOGICAL Logging;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    ULONG_PTR TimerHand;
    LARGE_INTEGER TimeStamp = {0};

    //
    // Loop processing DPC list entries until the specified DPC list is empty.
    //
    // N.B. This following code appears to have a redundant loop, but it does
    //      not. The point of this code is to avoid as many dispatch interrupts
    //      as possible.
    //

    DpcData = &Prcb->DpcData[DPC_NORMAL];
    ListHead = &DpcData->DpcListHead;
    Logging = PERFINFO_IS_GROUP_ON(PERF_DPC);
    do {
        Prcb->DpcRoutineActive = TRUE;

        //
        // If the timer hand value is nonzero, then process expired timers.
        //

        if (Prcb->TimerRequest != 0) {
            TimerHand = Prcb->TimerHand;
            Prcb->TimerRequest = 0;
            _enable();
            KiTimerExpiration(NULL, NULL, (PVOID) TimerHand, NULL);
            _disable();
        }

        //
        // If the DPC list is not empty, then process the DPC list.
        //

        if (DpcData->DpcQueueDepth != 0) {

            //
            // Acquire the DPC lock for the current processor and check if
            // the DPC list is empty. If the DPC list is not empty, then
            // remove the first entry from the DPC list, capture the DPC
            // parameters, set the DPC inserted state false, decrement the
            // DPC queue depth, release the DPC lock, enable interrupts, and
            // call the specified DPC routine. Otherwise, release the DPC
            // lock and enable interrupts.
            //

            do {
                KeAcquireSpinLockAtDpcLevel(&DpcData->DpcLock);
                Entry = ListHead->Flink;
                if (Entry != ListHead) {
                    RemoveEntryList(Entry);
                    Dpc = CONTAINING_RECORD(Entry, KDPC, DpcListEntry);
                    DeferredRoutine = Dpc->DeferredRoutine;
                    DeferredContext = Dpc->DeferredContext;
                    SystemArgument1 = Dpc->SystemArgument1;
                    SystemArgument2 = Dpc->SystemArgument2;
                    Dpc->DpcData = NULL;
                    DpcData->DpcQueueDepth -= 1;

#if DBG

                    Prcb->DebugDpcTime = 0;

#endif

                    KeReleaseSpinLockFromDpcLevel(&DpcData->DpcLock);
                    _enable();

                    //
                    // If event tracing is enabled, capture the start time.
                    //

                    if (Logging != FALSE) {
                        PerfTimeStamp(TimeStamp);
                    }

                    //
                    // Call the DPC routine.
                    //

                    (DeferredRoutine)(Dpc,
                                      DeferredContext,
                                      SystemArgument1,
                                      SystemArgument2);

                    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

                    //
                    // If event tracing is enabled, then log the start time
                    // and routine address.
                    //

                    if (Logging != FALSE) {
                        DpcInformation.InitialTime = TimeStamp.QuadPart;
                        DpcInformation.DpcRoutine = (PVOID) (ULONG_PTR) DeferredRoutine;
                        PerfInfoLogBytes(PERFINFO_LOG_TYPE_DPC,
                                         &DpcInformation,
                                         sizeof(DpcInformation));
                    }

                    _disable();

                } else {

                    ASSERT(DpcData->DpcQueueDepth == 0);

                    KeReleaseSpinLockFromDpcLevel(&DpcData->DpcLock);
                }

            } while (DpcData->DpcQueueDepth != 0);
        }

        Prcb->DpcRoutineActive = FALSE;
        Prcb->DpcInterruptRequested = FALSE;
        KeMemoryBarrier();

        //
        // Process the deferred ready list if the list is not empty.
        //

#if !defined(NT_UP)

        if (Prcb->DeferredReadyListHead.Next != NULL) {

            KIRQL OldIrql;

            _enable();
            OldIrql = KeRaiseIrqlToSynchLevel();
            KiProcessDeferredReadyList(Prcb);
            KeLowerIrql(OldIrql);
            _disable();
        }
#endif

#if defined(_WIN64)

    } while ((DpcData->DpcQueueDepth != 0) || (Prcb->TimerRequest != 0));

#else

    } while (DpcData->DpcQueueDepth != 0);

#endif

    return;
}

