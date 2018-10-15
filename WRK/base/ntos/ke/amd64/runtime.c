/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    runtime.c

Abstract:

    This module implements code to update runtime, perform quantum end
    processing, and perform DPC moderation processing.

--*/

#include "ki.h"

VOID
KeUpdateRunTime (
    IN PKTRAP_FRAME TrapFrame,
    IN LONG Increment
    )

/*++

Routine Description:

    This routine is called as the result of the interval timer interrupt on
    all processors in the system. Its function is update the per processor
    tick count, update the runtime of the current thread, update the runtime
    of the current thread's process, perform DPC moderation, and decrement
    the current thread quantum if a full tick has expired. This routine also
    performs real time scheduling quantum end and next interval processing.
 
    N.B. This routine is executed on all processors in a multiprocessor
         system.

Arguments:

    TrapFrame - Supplies the address of a trap frame.

    Increment - Supplies the time increment value in 100 nanosecond units.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;
    PKTHREAD Thread;

    //
    // If the clock tick should be skipped, then clear the skip flag and
    // return.
    //

    Prcb = KeGetCurrentPrcb();

#if DBG

    if (Prcb->SkipTick != FALSE) {
        Prcb->SkipTick = FALSE;
        return;
    }

#endif

    //
    // Update the tick count offset and check if a full tick has expired.
    //

    if ((Prcb->TickOffset -= Increment) <= 0) {

        //
        // Update the appropriate time counter based on previous mode, IRQL,
        // and whether there is currently a DPC active.
        //

        Prcb->TickOffset += KeMaximumIncrement;
        Thread = KeGetCurrentThread();
        if ((TrapFrame->SegCs & MODE_MASK) == 0) {

            //
            // Update the total time spent in kernel mode.
            //
            // If the interrupt nesting level is one and a DPC routine is
            // active, then update the DPC time. Otherwise, if the interrupt
            // nesting level is greater than one, then update the interrupt
            // time. Otherwise, update the current thread kernel time.
            //

            Prcb->KernelTime += 1;
            if ((Prcb->NestingLevel == 1) && (Prcb->DpcRoutineActive != FALSE)) {
                Prcb->DpcTime += 1;

                //
                // If the kernel debugger is enabled and the time spent in
                // the current DPC exceeds the system DPC time out limit,
                // then print a warning message and break into the debugger.
                //

#if DBG

                if ((KdDebuggerEnabled != 0) &
                    ((Prcb->DebugDpcTime += 1) >= KiDPCTimeout)) {
            
                    DbgPrint("*** DPC execution time exceeds system limit\n");
                    DbgPrint("    This is NOT a break in update time\n");
                    DbgPrint("    This is an ERROR in a DPC routine\n");
                    DbgPrint("    Perform a stack trace to find the culprit\n"); 
                    DbgBreakPoint();
                    Prcb->DebugDpcTime = 0;
                }

#endif

            } else if (Prcb->NestingLevel > 1) {
                Prcb->InterruptTime += 1;

            } else {
                Thread->KernelTime += 1;
            }

        } else {

            //
            // Update the total time spent in user mode.
            //

            Prcb->UserTime += 1;
            Thread->UserTime += 1;
        }

        //
        // Update the DPC request rate which is computed as the average of
        // the previous rate and the current rate.
        //

        Prcb->DpcRequestRate += (Prcb->DpcData[DPC_NORMAL].DpcCount - Prcb->DpcLastCount);
        Prcb->DpcRequestRate >>= 1;
        Prcb->DpcLastCount = Prcb->DpcData[DPC_NORMAL].DpcCount;

        //
        // If the current DPC queue depth is not zero, a DPC routine is not
        // active, and a DPC interrupt has not been requested, then request
        // a dispatch interrupt, decrement the maximum DPC queue depth, and
        // reset the threshold counter if appropriate. Otherwise, count down
        // the adjustment threshold and if the count reaches zero, then
        // increment the maximum DPC queue depth, but not above the initial
        // value and reset the adjustment threshold value.
        //

        if ((Prcb->DpcData[DPC_NORMAL].DpcQueueDepth != 0) &&
            (Prcb->DpcRoutineActive == FALSE) &&
            (Prcb->DpcInterruptRequested == FALSE)) {

            KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
            Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;

            //
            // If the DPC request rate is less than the ideal rate and the
            // DPC queue depth is not one, then decrement the maximum queue
            // depth.
            //

            if ((Prcb->DpcRequestRate < KiIdealDpcRate) &&
                (Prcb->MaximumDpcQueueDepth != 1)) {

                Prcb->MaximumDpcQueueDepth -= 1;
            }

        } else {
            if ((Prcb->AdjustDpcThreshold -= 1) == 0) {
                Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
                if (Prcb->MaximumDpcQueueDepth != KiMaximumDpcQueueDepth) {
                    Prcb->MaximumDpcQueueDepth += 1;
                }
            }
        }

        //
        // If the current thread is not the idle thread, then decrement the
        // thread quantum.
        //
        // If the thread quantum is exhausted, then set the quantum end true
        // and request a dispatch interrupt on the current processor.
        //

        if ((Thread != Prcb->IdleThread) &&
            ((Thread->Quantum -= CLOCK_QUANTUM_DECREMENT) <= 0)) {

            Prcb->QuantumEnd = TRUE;
            KiRequestSoftwareInterrupt(DISPATCH_LEVEL);
        }

        //
        // If the debugger is enabled, check if a break is requested.
        //
        // N.B. A poll break in attempt only occurs on each processor when
        //      the poll slot matches the current processor number.
        //
    
        if (KdDebuggerEnabled != FALSE) {
            if (Prcb->PollSlot == Prcb->Number) {
                KdCheckForDebugBreak();
            }
    
            Prcb->PollSlot += 1;
            if (Prcb->PollSlot == KeNumberProcessors) {
                Prcb->PollSlot = 0;
            }
        }
    }

    return;
}

