/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    apcsup.c

Abstract:

    This module contains the support routines for the APC object. Functions
    are provided to insert in an APC queue and to deliver user and kernel
    mode APC's.

--*/

#include "ki.h"

VOID
KiCheckForKernelApcDelivery (
    VOID
    )

/*++

Routine Description:

    This function checks to detemine if a kernel APC can be delivered
    immediately to the current thread or a kernel APC interrupt should
    be requested. On entry to this routine the following conditions are
    true:

    1. Special kernel APCs are enabled for the current thread.

    2. Normal kernel APCs may also be enabled for the current thread.

    3. The kernel APC queue is not empty.

    N.B. This routine is only called by kernel code that leaves a guarded
         or critcial region.

Arguments:

    None.

Return Value:

    None.

--*/

{

    //
    // If the current IRQL is passive level, then a kernel APC can be
    // delivered immediately. Otherwise, an APC interrupt must be
    // requested.
    //

    if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
        KiDeliverApc(KernelMode, NULL, NULL);
        KeLowerIrql(PASSIVE_LEVEL);

    } else {
        KeGetCurrentThread()->ApcState.KernelApcPending = TRUE;                 
        KiRequestSoftwareInterrupt(APC_LEVEL);                      
    }

    return;
}

VOID
KiDeliverApc (
    IN KPROCESSOR_MODE PreviousMode,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function is called from the APC interrupt code and when one or
    more of the APC pending flags are set at system exit and the previous
    IRQL is zero. All special kernel APC's are delivered first, followed
    by normal kernel APC's if one is not already in progress, and finally
    if the user APC queue is not empty, the user APC pending flag is set,
    and the previous mode is user, then a user APC is delivered. On entry
    to this routine IRQL is set to APC_LEVEL.

    N.B. The exception frame and trap frame addresses are only guaranteed
         to be valid if, and only if, the previous mode is user.

Arguments:

    PreviousMode - Supplies the previous processor mode.

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

Return Value:

    None.

--*/

{

    PKAPC Apc;
    PKKERNEL_ROUTINE KernelRoutine;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PVOID NormalContext;
    PKNORMAL_ROUTINE NormalRoutine;
    PKTRAP_FRAME OldTrapFrame;
    PKPROCESS Process;
    PVOID SystemArgument1;
    PVOID SystemArgument2;
    PKTHREAD Thread;

    //
    // If the thread was interrupted in the middle of the SLIST pop code,
    // then back up the PC to the start of the SLIST pop. 
    //

    if (TrapFrame != NULL) {
        KiCheckForSListAddress(TrapFrame);
    }

    //
    // Save the current thread trap frame address and set the thread trap
    // frame address to the new trap frame. This will prevent a user mode
    // exception from being raised within an APC routine.
    //

    Thread = KeGetCurrentThread();
    OldTrapFrame = Thread->TrapFrame;
    Thread->TrapFrame = TrapFrame;

    //
    // If special APC are not disabled, then attempt to deliver one or more
    // APCs.
    //

    Process = Thread->ApcState.Process;
    Thread->ApcState.KernelApcPending = FALSE;
    if (Thread->SpecialApcDisable == 0) {

        //
        // If the kernel APC queue is not empty, then attempt to deliver a
        // kernel APC.
        //
        // N.B. The following test is not synchronized with the APC insertion
        //      code. However, when an APC is inserted in the kernel queue of
        //      a running thread an APC interrupt is requested. Therefore, if
        //      the following test were to falsely return that the kernel APC
        //      queue was empty, an APC interrupt would immediately cause this
        //      code to be executed a second time in which case the kernel APC
        //      queue would found to contain an entry.
        //

        KeMemoryBarrier();
        while (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {

            //
            // Raise IRQL to dispatcher level, lock the APC queue, and check
            // if any kernel mode APC's can be delivered.
            //
            // If the kernel APC queue is now empty because of the removal of
            // one or more entries, then release the APC lock, and attempt to
            // deliver a user APC.
            //

            KeAcquireInStackQueuedSpinLock(&Thread->ApcQueueLock, &LockHandle);
            NextEntry = Thread->ApcState.ApcListHead[KernelMode].Flink;
            if (NextEntry == &Thread->ApcState.ApcListHead[KernelMode]) {
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                break;
            }

            //
            // Clear kernel APC pending, get the address of the APC object,
            // and determine the type of APC.
            //
            // N.B. Kernel APC pending must be cleared each time the kernel
            //      APC queue is found to be non-empty.
            //

            Thread->ApcState.KernelApcPending = FALSE;
            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            ReadForWriteAccess(Apc);
            KernelRoutine = Apc->KernelRoutine;
            NormalRoutine = Apc->NormalRoutine;
            NormalContext = Apc->NormalContext;
            SystemArgument1 = Apc->SystemArgument1;
            SystemArgument2 = Apc->SystemArgument2;
            if (NormalRoutine == (PKNORMAL_ROUTINE)NULL) {
    
                //
                // First entry in the kernel APC queue is a special kernel APC.
                // Remove the entry from the APC queue, set its inserted state
                // to FALSE, release dispatcher database lock, and call the kernel
                // routine. On return raise IRQL to dispatcher level and lock
                // dispatcher database lock.
                //
    
                RemoveEntryList(NextEntry);
                Apc->Inserted = FALSE;
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                (KernelRoutine)(Apc,
                                &NormalRoutine,
                                &NormalContext,
                                &SystemArgument1,
                                &SystemArgument2);
    
#if DBG
    
                if (KeGetCurrentIrql() != LockHandle.OldIrql) {
                    KeBugCheckEx(IRQL_UNEXPECTED_VALUE,
                                 KeGetCurrentIrql() << 16 | LockHandle.OldIrql << 8,
                                 (ULONG_PTR)KernelRoutine,
                                 (ULONG_PTR)Apc,
                                 (ULONG_PTR)NormalRoutine);
                }
    
#endif

            } else {
    
                //
                // First entry in the kernel APC queue is a normal kernel APC.
                // If there is not a normal kernel APC in progress and kernel
                // APC's are not disabled, then remove the entry from the APC
                // queue, set its inserted state to FALSE, release the APC queue
                // lock, call the specified kernel routine, set kernel APC in
                // progress, lower the IRQL to zero, and call the normal kernel
                // APC routine. On return raise IRQL to dispatcher level, lock
                // the APC queue, and clear kernel APC in progress.
                //
    
                if ((Thread->ApcState.KernelApcInProgress == FALSE) &&
                   (Thread->KernelApcDisable == 0)) {

                    RemoveEntryList(NextEntry);
                    Apc->Inserted = FALSE;
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                    (KernelRoutine)(Apc,
                                    &NormalRoutine,
                                    &NormalContext,
                                    &SystemArgument1,
                                    &SystemArgument2);
    
#if DBG
    
                    if (KeGetCurrentIrql() != LockHandle.OldIrql) {
                        KeBugCheckEx(IRQL_UNEXPECTED_VALUE,
                                     KeGetCurrentIrql() << 16 | LockHandle.OldIrql << 8 | 1,
                                     (ULONG_PTR)KernelRoutine,
                                     (ULONG_PTR)Apc,
                                     (ULONG_PTR)NormalRoutine);
                    }
    
#endif
    
                    if (NormalRoutine != (PKNORMAL_ROUTINE)NULL) {
                        Thread->ApcState.KernelApcInProgress = TRUE;
                        KeLowerIrql(0);
                        (NormalRoutine)(NormalContext,
                                        SystemArgument1,
                                        SystemArgument2);
    
                        KeRaiseIrql(APC_LEVEL, &LockHandle.OldIrql);
                    }
    
                    Thread->ApcState.KernelApcInProgress = FALSE;
    
                } else {
                    KeReleaseInStackQueuedSpinLock(&LockHandle);
                    goto CheckProcess;
                }
            }
        }

        //
        // Kernel APC queue is empty. If the previous mode is user, user APC
        // pending is set, and the user APC queue is not empty, then remove
        // the first entry from the user APC queue, set its inserted state to
        // FALSE, clear user APC pending, release the dispatcher database lock,
        // and call the specified kernel routine. If the normal routine address
        // is not NULL on return from the kernel routine, then initialize the
        // user mode APC context and return. Otherwise, check to determine if
        // another user mode APC can be processed.
        //
        // N.B. There is no race condition associated with checking the APC
        //      queue outside the APC lock. User APCs are always delivered at
        //      system exit and never interrupt the execution of the thread
        //      in the kernel.
        //
    
        if ((PreviousMode == UserMode) &&
            (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE) &&
            (Thread->ApcState.UserApcPending != FALSE)) {

            //
            // Raise IRQL to dispatcher level, lock the APC queue, and deliver
            // a user mode APC.
            //

            KeAcquireInStackQueuedSpinLock(&Thread->ApcQueueLock, &LockHandle);

            //
            // If the user APC queue is now empty because of the removal of
            // one or more entries, then release the APC lock and exit.
            //

            Thread->ApcState.UserApcPending = FALSE;
            NextEntry = Thread->ApcState.ApcListHead[UserMode].Flink;
            if (NextEntry == &Thread->ApcState.ApcListHead[UserMode]) {
                KeReleaseInStackQueuedSpinLock(&LockHandle);
                goto CheckProcess;
            }

            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            ReadForWriteAccess(Apc);
            KernelRoutine = Apc->KernelRoutine;
            NormalRoutine = Apc->NormalRoutine;
            NormalContext = Apc->NormalContext;
            SystemArgument1 = Apc->SystemArgument1;
            SystemArgument2 = Apc->SystemArgument2;
            RemoveEntryList(NextEntry);
            Apc->Inserted = FALSE;
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            (KernelRoutine)(Apc,
                            &NormalRoutine,
                            &NormalContext,
                            &SystemArgument1,
                            &SystemArgument2);
    
            if (NormalRoutine == (PKNORMAL_ROUTINE)NULL) {
                KeTestAlertThread(UserMode);
    
            } else {
                KiInitializeUserApc(ExceptionFrame,
                                    TrapFrame,
                                    NormalRoutine,
                                    NormalContext,
                                    SystemArgument1,
                                    SystemArgument2);
            }
        }
    }

    //
    // Check if process was attached during the APC routine.
    //

CheckProcess:
    if (Thread->ApcState.Process != Process) {
        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

    //
    // Restore the previous thread trap frame address.
    //

    Thread->TrapFrame = OldTrapFrame;
    return;
}

VOID
FASTCALL
KiInsertQueueApc (
    IN PKAPC Apc,
    IN KPRIORITY Increment
    )

/*++

Routine Description:

    This function inserts an APC object into a thread's APC queue. The address
    of the thread object, the APC queue, and the type of APC are all derived
    from the APC object. If the APC object is already in an APC queue, then
    no operation is performed and a function value of FALSE is returned. Else
    the APC is inserted in the specified APC queue, its inserted state is set
    to TRUE, and a function value of TRUE is returned. The APC will actually
    be delivered when proper enabling conditions exist.

    N.B. The thread APC queue lock must be held when this routine is called.

    N.B. It is the responsibility of the caller to ensure that the APC is not
         already inserted in an APC queue and to set the Inserted field of
         the APC.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.

    Increment - Supplies the priority increment that is to be applied if
        queuing the APC causes a thread wait to be satisfied.

Return Value:

    None.

--*/

{

    KPROCESSOR_MODE ApcMode;
    PKAPC ApcEntry;
    PKAPC_STATE ApcState;
    PKGATE GateObject;
    PLIST_ENTRY ListEntry;
    PKQUEUE Queue;
    BOOLEAN RequestInterrupt;
    PKTHREAD Thread;
    KTHREAD_STATE ThreadState;

    //
    // Insert the APC object in the specified APC queue, set the APC inserted
    // state to TRUE, and check to determine if the APC should be delivered
    // immediately.
    //
    // For multiprocessor performance, the following code utilizes the fact
    // that kernel APC disable count is incremented before checking whether
    // the kernel APC queue is nonempty.
    //
    // See KeLeaveCriticalRegion().
    //

    Thread = Apc->Thread;
    if (Apc->ApcStateIndex == InsertApcEnvironment) {
        Apc->ApcStateIndex = Thread->ApcStateIndex;
    }

    ApcState = Thread->ApcStatePointer[Apc->ApcStateIndex];

    //
    // Insert the APC after all other special APC entries selected by
    // the processor mode if the normal routine value is NULL. Else
    // insert the APC object at the tail of the APC queue selected by
    // the processor mode unless the APC mode is user and the address
    // of the special APC routine is exit thread, in which case insert
    // the APC at the front of the list and set user APC pending.
    //

    ApcMode = Apc->ApcMode;

    ASSERT (Apc->Inserted == TRUE);

    if (Apc->NormalRoutine != NULL) {
        if ((ApcMode != KernelMode) && (Apc->KernelRoutine == PsExitSpecialApc)) {
            Thread->ApcState.UserApcPending = TRUE;
            InsertHeadList(&ApcState->ApcListHead[ApcMode],
                           &Apc->ApcListEntry);

        } else {
            InsertTailList(&ApcState->ApcListHead[ApcMode],
                           &Apc->ApcListEntry);
        }

    } else {
        ListEntry = ApcState->ApcListHead[ApcMode].Blink;
        while (ListEntry != &ApcState->ApcListHead[ApcMode]) {
            ApcEntry = CONTAINING_RECORD(ListEntry, KAPC, ApcListEntry);
            if (ApcEntry->NormalRoutine == NULL) {
                break;
            }

            ListEntry = ListEntry->Blink;
        }

        InsertHeadList(ListEntry, &Apc->ApcListEntry);
    }

    //
    // If the APC index from the APC object matches the APC Index of
    // the thread, then check to determine if the APC should interrupt
    // thread execution or sequence the thread out of a wait state.
    //

    if (Apc->ApcStateIndex == Thread->ApcStateIndex) {

        //
        // If the target thread is the current thread, then the thread state
        // is running and cannot change.
        //

        if (Thread == KeGetCurrentThread()) {

            ASSERT(Thread->State == Running);

            //
            // If the APC mode is kernel, then set kernel APC pending and
            // request an APC interrupt if special APC's are not disabled.
            //

            if (ApcMode == KernelMode) {
                Thread->ApcState.KernelApcPending = TRUE;
                if (Thread->SpecialApcDisable == 0) {
                    KiRequestSoftwareInterrupt(APC_LEVEL);
                }
            }

            return;
        }

        //
        // Lock the dispatcher database and test the processor mode.
        //
        // If the processor mode of the APC is kernel, then check if
        // the APC should either interrupt the thread or sequence the
        // thread out of a Waiting state. Else check if the APC should
        // sequence the thread out of an alertable Waiting state.
        //

        RequestInterrupt = FALSE;
        KiLockDispatcherDatabaseAtSynchLevel();
        if (ApcMode == KernelMode) {

            //
            // Thread transitions from the standby state to the running
            // state can occur from the idle thread without holding the
            // dispatcher lock. Reading the thread state after setting
            // the kernel APC pending flag prevents the code from not
            // delivering the APC interrupt in this case.
            //
            // N.B. Transitions from gate wait to running are synchronized
            //      using the thread lock. Transitions from running to gate
            //      wait are synchronized using the APC queue lock.
            //
            // N.B. If the target thread is found to be in the running state,
            //      then the APC interrupt request can be safely deferred to
            //      after the dispatcher lock is released even if the thread
            //      were to be switched to another processor, i.e., the APC
            //      would be delivered by the context switch code.
            //

            Thread->ApcState.KernelApcPending = TRUE;
            KeMemoryBarrier();
            ThreadState = Thread->State;
            if (ThreadState == Running) {
                RequestInterrupt = TRUE;

            } else if ((ThreadState == Waiting) &&
                       (Thread->WaitIrql == 0) &&
                       (Thread->SpecialApcDisable == 0) &&
                       ((Apc->NormalRoutine == NULL) ||
                        ((Thread->KernelApcDisable == 0) &&
                         (Thread->ApcState.KernelApcInProgress == FALSE)))) {

                KiUnwaitThread(Thread, STATUS_KERNEL_APC, Increment);

            } else if (Thread->State == GateWait) {
                KiAcquireThreadLock(Thread);
                if ((Thread->State == GateWait) &&
                    (Thread->WaitIrql == 0) &&
                    (Thread->SpecialApcDisable == 0) &&
                    ((Apc->NormalRoutine == NULL) ||
                     ((Thread->KernelApcDisable == 0) &&
                      (Thread->ApcState.KernelApcInProgress == FALSE)))) {

                    GateObject = Thread->GateObject;
                    KiAcquireKobjectLock(GateObject);
                    RemoveEntryList(&Thread->WaitBlock[0].WaitListEntry);
                    KiReleaseKobjectLock(GateObject);
                    if ((Queue = Thread->Queue) != NULL) {
                        Queue->CurrentCount += 1;
                    }

                    Thread->WaitStatus = STATUS_KERNEL_APC;
                    KiInsertDeferredReadyList(Thread);
                }

                KiReleaseThreadLock(Thread);
            }

        } else if ((Thread->State == Waiting) &&
                  (Thread->WaitMode == UserMode) &&
                  (Thread->Alertable || Thread->ApcState.UserApcPending)) {

            Thread->ApcState.UserApcPending = TRUE;
            KiUnwaitThread(Thread, STATUS_USER_APC, Increment);
        }

        //
        // Unlock the dispatcher database and request an APC interrupt if
        // required.
        //

        KiUnlockDispatcherDatabaseFromSynchLevel();
        if (RequestInterrupt == TRUE) {
            KiRequestApcInterrupt(Thread->NextProcessor);
        }
    }

    return;
}

