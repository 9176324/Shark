/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    stubs.c

Abstract:

    This module implements kernel debugger synchronization routines.

--*/

#include "ki.h"

//
// KiDebugRoutine - This is the address of the kernel debugger. Initially
//      this is filled with the address of a routine that just returns. If
//      the system debugger is present in the system, then it sets this
//      location to the address of the system debugger's routine.
//

PKDEBUG_ROUTINE KiDebugRoutine = &KdpStub;

#if defined(_AMD64_)
#define IDBG    0
#else
#define IDBG    1
#endif


//
// Define flags bits.
//

#define FREEZE_ACTIVE           0x20

//
// Define local storage to save the old IRQL.
//

KIRQL KiOldIrql;                                  

#ifndef NT_UP
PKPRCB KiFreezeOwner;
#endif

BOOLEAN
KeFreezeExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function freezes the execution of all other processors in the host
    configuration and then returns to the caller.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame that describes the
        trap.

    ExceptionFrame - Supplies a pointer to an exception frame that
        describes the trap.

Return Value:

    Previous interrupt enable.

--*/

{

    BOOLEAN Enable;

#if !defined(NT_UP)

    PKPRCB Prcb;
    KAFFINITY TargetSet;
    ULONG BitNumber;
    KIRQL OldIrql;

#if IDBG

    ULONG Count = 30000;

#endif

#if !defined(_AMD64_)
    BOOLEAN Flag;
#endif

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif

    //
    // Disable interrupts.
    //

    Enable = KeDisableInterrupts();
    KiFreezeFlag = FREEZE_FROZEN;

    //
    // Raise IRQL to HIGH_LEVEL.
    //

#if !defined(NT_UP)

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    if (FrozenState(KeGetCurrentPrcb()->IpiFrozen) == FREEZE_OWNER) {

        //
        // This processor already owns the freeze lock.
        // Return without trying to re-acquire lock or without
        // trying to IPI the other processors again
        //

        return Enable;
    }

#if defined(_AMD64_)

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

    //
    // Acquire the KiFreezeExecutionLock.  No need to check for FREEZE
    // IPIs as these come in as NMIs anyway.
    //

    KeAcquireSpinLockAtDpcLevel(&KiFreezeExecutionLock);

#else

    //
    // Try to acquire the KiFreezeExecutionLock before sending the request.
    // To prevent deadlock from occurring, we need to accept and process
    // incoming FreezeExecution requests while we are waiting to acquire
    // the FreezeExecutionFlag.
    //

    while (KeTryToAcquireSpinLockAtDpcLevel(&KiFreezeExecutionLock) == FALSE) {

        //
        // FreezeExecutionLock is busy.  Another processor may be trying
        // to IPI us - go service any IPI.
        //

        KeEnableInterrupts(Enable);
        Flag = KiIpiServiceRoutine((PVOID)TrapFrame, (PVOID)ExceptionFrame);
        KeDisableInterrupts();

#if IDBG

        if (Flag != FALSE) {
            Count = 30000;
            continue;
        }

        KeStallExecutionProcessor (100);
        if (!Count--) {
            Count = 30000;
            if (KeTryToAcquireSpinLockAtDpcLevel(&KiFreezeLockBackup) == TRUE) {
                KiFreezeFlag |= FREEZE_BACKUP;
                break;
            }
        }

#endif

    }

#endif

    //
    // After acquiring the lock flag, we send Freeze request to each processor
    // in the system (other than us) and wait for it to become frozen.
    //

    Prcb = KeGetCurrentPrcb();  // Do this after spinlock is acquired.
    TargetSet = KeActiveProcessors & ~(AFFINITY_MASK(Prcb->Number));
    if (TargetSet) {

#if IDBG
        Count = 4000;
#endif

        KiFreezeOwner = Prcb;
        Prcb->IpiFrozen = FREEZE_OWNER | FREEZE_ACTIVE;
        Prcb->SkipTick  = TRUE;
        KiSendFreeze(&TargetSet, (KeBugCheckActive != 2));
        while (TargetSet != 0) {
            KeFindFirstSetLeftAffinity(TargetSet, &BitNumber);
            ClearMember(BitNumber, TargetSet);
            Prcb = KiProcessorBlock[BitNumber];

#if IDBG

            while (Prcb->IpiFrozen != TARGET_FROZEN) {
                if (Count == 0) {
                    KiFreezeFlag |= FREEZE_SKIPPED_PROCESSOR;
                    break;
                }

                KeStallExecutionProcessor (10000);
                Count--;
            }

#else

            while (Prcb->IpiFrozen != TARGET_FROZEN) {
                KeYieldProcessor();
            }
#endif

        }
    }

    //
    // Save the old IRQL.
    //

    KiOldIrql = OldIrql;

#else

    //
    // Save the current IRQL.
    //

    KiOldIrql = KeGetCurrentIrql();

#endif      // !defined(NT_UP)

    //
    // Return whether interrupts were previous enabled.
    //

    return Enable;
}

VOID
KiFreezeTargetExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function freezes the execution of the current running processor.
    If a trapframe is supplied to current state is saved into the prcb
    for the debugger.

Arguments:

    TrapFrame - Supplies a pointer to the trap frame that describes the
        trap.

    ExceptionFrame - Supplies a pointer to the exception frame that
        describes the trap.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    KIRQL OldIrql;
    PKPRCB Prcb;
    BOOLEAN Enable;
    KCONTINUE_STATUS Status;
    EXCEPTION_RECORD ExceptionRecord;

    //
    // If the freeze lock is not held, then a debugger freeze was missed.
    //

    if ((KiFreezeExecutionLock == 0) && 
        (KiFreezeLockBackup == 0) &&
        (KeBugCheckActive == FALSE)) {
#if DBG

        DbgBreakPoint();

#endif

        return;
    }

    Enable = KeDisableInterrupts();
    KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    Prcb = KeGetCurrentPrcb();
    Prcb->IpiFrozen = TARGET_FROZEN;
    Prcb->SkipTick  = TRUE;
    if (TrapFrame != NULL) {
        KiSaveProcessorState(TrapFrame, ExceptionFrame);
    }

    //
    // Sweep the data cache in case this is a system crash and the bugcheck
    // code is attempting to write a crash dump file.
    //

    KeSweepCurrentDcache();

    //
    //  Wait for person requesting us to freeze to
    //  clear our frozen flag
    //

    while (FrozenState(Prcb->IpiFrozen) == TARGET_FROZEN) {
        if (Prcb->IpiFrozen & FREEZE_ACTIVE) {

            //
            // This processor has been made the active processor
            //
            if (TrapFrame) {
                RtlZeroMemory (&ExceptionRecord, sizeof ExceptionRecord);
                ExceptionRecord.ExceptionCode = STATUS_WAKE_SYSTEM_DEBUGGER;
                ExceptionRecord.ExceptionRecord  = &ExceptionRecord;
                ExceptionRecord.ExceptionAddress =
                    (PVOID)CONTEXT_TO_PROGRAM_COUNTER (&Prcb->ProcessorState.ContextFrame);

                Status = (KiDebugSwitchRoutine) (
                            &ExceptionRecord,
                            &Prcb->ProcessorState.ContextFrame,
                            FALSE
                            );

            } else {
                Status = ContinueError;
            }

            //
            // If status is anything other then, continue with next
            // processor then reselect master
            //

            if (Status != ContinueNextProcessor) {
                Prcb->IpiFrozen &= ~FREEZE_ACTIVE;
                KiFreezeOwner->IpiFrozen |= FREEZE_ACTIVE;
            }
        }
        KeYieldProcessor();
    }

    if (TrapFrame != NULL) {
        KiRestoreProcessorState(TrapFrame, ExceptionFrame);
    }

    Prcb->IpiFrozen = RUNNING;
    KeFlushCurrentTb();
    KeSweepCurrentIcache();
    KeLowerIrql(OldIrql);
    KeEnableInterrupts(Enable);

#else

    UNREFERENCED_PARAMETER(TrapFrame);
    UNREFERENCED_PARAMETER(ExceptionFrame);

#endif      // !define(NT_UP)

    return;
}

KCONTINUE_STATUS
KeSwitchFrozenProcessor (
    IN ULONG ProcessorNumber
    )
{

#if !defined(NT_UP)

    PKPRCB TargetPrcb, CurrentPrcb;

    //
    // If Processor number is out of range, reselect current processor
    //

    if (ProcessorNumber >= (ULONG) KeNumberProcessors) {
        return ContinueProcessorReselected;
    }

    TargetPrcb = KiProcessorBlock[ProcessorNumber];
    CurrentPrcb = KeGetCurrentPrcb();

    //
    // Move active flag to correct processor.
    //

    CurrentPrcb->IpiFrozen &= ~FREEZE_ACTIVE;
    TargetPrcb->IpiFrozen  |= FREEZE_ACTIVE;

    //
    // If this processor is frozen in KiFreezeTargetExecution, return to it
    //

    if (FrozenState(CurrentPrcb->IpiFrozen) == TARGET_FROZEN) {
        return ContinueNextProcessor;
    }

    //
    // This processor must be FREEZE_OWNER, wait to be reselected as the
    // active processor
    //

    if (FrozenState(CurrentPrcb->IpiFrozen) != FREEZE_OWNER) {
        return ContinueError;
    }

    while (!(CurrentPrcb->IpiFrozen & FREEZE_ACTIVE)) {
        KeYieldProcessor();
    }

#else

    UNREFERENCED_PARAMETER(ProcessorNumber);

#endif  // !defined(NT_UP)

    //
    // Reselect this processor
    //

    return ContinueProcessorReselected;
}

VOID
KeThawExecution (
    IN BOOLEAN Enable
    )

/*++

Routine Description:

    This function thaws the execution of all other processors in the host
    configuration and then returns to the caller. It is intended for use by
    the kernel debugger.

Arguments:

    Enable - Supplies the previous interrupt enable that is to be restored
        after having thawed the execution of all other processors.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    KIRQL OldIrql;

#if IDBG
    ULONG Flag;
#endif

    KiSendThawExecution (TRUE);

    //
    // Capture the previous IRQL before releasing the freeze lock.
    //

    OldIrql = KiOldIrql;

#if IDBG

    Flag = KiFreezeFlag;
    KiFreezeFlag = 0;

    if ((Flag & FREEZE_BACKUP) != 0) {
        KiReleaseSpinLock(&KiFreezeLockBackup);
    } else {
        KiReleaseSpinLock(&KiFreezeExecutionLock);
    }

#else

    KiFreezeFlag = 0;
    KiReleaseSpinLock(&KiFreezeExecutionLock);

#endif
#endif  // !defined (NT_UP)


    //
    // Flush the current TB, instruction cache, and data cache.
    //

    KeFlushCurrentTb();
    KeSweepCurrentIcache();
    KeSweepCurrentDcache();

    //
    // Lower IRQL and restore interrupt enable
    //

#if !defined(NT_UP)
    KeLowerIrql(OldIrql);
#endif
    KeEnableInterrupts(Enable);
    return;
}

VOID
KiPollFreezeExecution(
    VOID
    )

/*++

Routine Description:

    This routine is called from code that is spinning with interrupts
    disabled, waiting for something to happen, when there is some
    (possibly extremely small) chance that that thing will not happen
    because a system freeze has been initiated.

    N.B. Interrupts are disabled.

Arguments:

    None.

Return Value:

    None.

--*/

{

#if defined(_AMD64_)

    KeYieldProcessor();

#else

    //
    // Check to see if a freeze is pending for this processor.
    //

    PKPRCB Prcb = KeGetCurrentPrcb();

    if ((Prcb->RequestSummary & IPI_FREEZE) != 0) {

        //
        // Clear the freeze request and freeze this processor.
        //

        InterlockedExchangeAdd((PLONG)&Prcb->RequestSummary, -(IPI_FREEZE));
        KiFreezeTargetExecution(NULL, NULL);

    } else {

        //
        // No freeze pending, assume this processor is spinning.
        //

        KeYieldProcessor();
    }

#endif

}

