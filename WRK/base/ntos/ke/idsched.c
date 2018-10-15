/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    idsched.c

Abstract:

    This module implement idle scheduling. Idle scheduling is performed when
    the current thread is entering a wait state or must be rescheduled for
    any other reason (e.g., affinity change), and a suitable thread cannot be
    found after searching a suitable subset of the processor ready queues.

--*/

#include "ki.h"

#if !defined(NT_UP)

PKTHREAD
FASTCALL
KiIdleSchedule (
    PKPRCB CurrentPrcb
    )

/*++

Routine Description:

    This function is called when the idle schedule flag is set in the current
    PRCB. This flag is set when the current thread is entering a wait state
    or must be rescheduled for any other reason (e.g., affinity change) and
    a suitable thread cannot be found after searching all the processor ready
    queues outside the dispatcher database lock. A second pass over the ready
    queues is required since the processor was not idle during the first scan,
    and therefore, a potential candidate thread may have been missed.

Arguments:

    CurrentPrcb - Supplies a pointer to the current processor block.

Return Value:

    If a thread is found to run, then a pointer to the thread is returned.
    Otherwise, NULL is returned.

    N.B. If a thread is found, then IRQL is returned at SYNCH_LEVEL. If a
         thread is not found, then IRQL is returned at DISPATCH_LEVEL.

--*/

{

    LONG Index;
    LONG Limit;
    LONG Number;
    PKTHREAD NewThread;
    ULONG Processor;
    PKPRCB TargetPrcb;

    ASSERT (CurrentPrcb == KeGetCurrentPrcb());

    //
    // Raise IRQL to SYNCH_LEVEL, acquire the current PRCB lock, and clear
    // idle schedule.
    //

    KeRaiseIrqlToSynchLevel();
    KiAcquirePrcbLock(CurrentPrcb);
    CurrentPrcb->IdleSchedule = FALSE;

    //
    // If the idle thread has been selected to run on current processor, then
    // clear the idle thread selection.
    //
    // N.B. This condition can occur when another thread is scheduled to run
    //      on the current processor, then becomes ineligible to run and the
    //      processor goes idle again.
    //

    if (CurrentPrcb->NextThread == CurrentPrcb->IdleThread) {
        CurrentPrcb->NextThread = NULL;
    }

    //
    // If a thread has already been selected to run on the current processor,
    // then select that thread. Otherwise, attempt to select a thread from
    // the current processor dispatcher ready queues.
    //

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

        NewThread = KiSelectReadyThread(0, CurrentPrcb);
        if (NewThread != NULL) {
            CurrentPrcb->NextThread = NULL;
            CurrentPrcb->CurrentThread = NewThread;
            NewThread->State = Running;

        } else {

            //
            // Release the current PRCB lock and attempt to select a thread
            // from any processor dispatcher ready queues.
            //
            // If this is a multinode system, then start with the processors
            // on the same node. Otherwise, start with the current processor.
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
                                CurrentPrcb->NextThread = NULL;
                                CurrentPrcb->CurrentThread = NewThread;

                                //
                                // Clear idle on the current processor
                                // and update the idle SMT summary set to
                                // indicate the set is not idle.
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
        }
    }

    //
    // If a new thread has been selected for execution, then release the
    // PRCB lock and acquire the idle thread lock. Otherwise, lower IRQL
    // to DISPATCH_LEVEL.
    //

ThreadFound:;
    if (NewThread == NULL) {
        KeLowerIrql(DISPATCH_LEVEL);

    } else {
        KiSetContextSwapBusy(CurrentPrcb->IdleThread);
        KiReleasePrcbLock(CurrentPrcb);
    }

    return NewThread;
}

#endif

