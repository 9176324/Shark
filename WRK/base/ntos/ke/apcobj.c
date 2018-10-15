/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    apcobj.c

Abstract:

    This module implements the kernel APC object. Functions are provided
    to initialize, flush, insert, and remove APC objects.

--*/

#include "ki.h"

//
// The following assert macro is used to check that an input apc is
// really a kapc and not something else, like deallocated pool.
//

#define ASSERT_APC(E) ASSERT((E)->Type == ApcObject)

VOID
KeInitializeApc (
    __out PRKAPC Apc,
    __in PRKTHREAD Thread,
    __in KAPC_ENVIRONMENT Environment,
    __in PKKERNEL_ROUTINE KernelRoutine,
    __in_opt PKRUNDOWN_ROUTINE RundownRoutine,
    __in_opt PKNORMAL_ROUTINE NormalRoutine,
    __in_opt KPROCESSOR_MODE ApcMode,
    __in_opt PVOID NormalContext
    )

/*++

Routine Description:

    This function initializes a kernel APC object. The thread, kernel
    routine, and optionally a normal routine, processor mode, and normal
    context parameter are stored in the APC object.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.

    Thread - Supplies a pointer to a dispatcher object of type thread.

    Environment - Supplies the environment in which the APC will execute.
        Valid values for this parameter are: OriginalApcEnvironment,
        AttachedApcEnvironment, CurrentApcEnvironment, or InsertApcEnvironment

    KernelRoutine - Supplies a pointer to a function that is to be
        executed at IRQL APC_LEVEL in kernel mode.

    RundownRoutine - Supplies an optional pointer to a function that is to be
        called if the APC is in a thread's APC queue when the thread terminates.

    NormalRoutine - Supplies an optional pointer to a function that is
        to be executed at IRQL 0 in the specified processor mode. If this
        parameter is not specified, then the ProcessorMode and NormalContext
        parameters are ignored.

    ApcMode - Supplies the processor mode in which the function specified
        by the NormalRoutine parameter is to be executed.

    NormalContext - Supplies a pointer to an arbitrary data structure which is
        to be passed to the function specified by the NormalRoutine parameter.

Return Value:

    None.

--*/

{

    ASSERT(Environment <= InsertApcEnvironment);

    //
    // Initialize standard control object header.
    //

    Apc->Type = ApcObject;
    Apc->Size = sizeof(KAPC);

    //
    // Initialize the APC environment, thread address, kernel routine address,
    // rundown routine address, normal routine address, processor mode, and
    // normal context parameter. If the normal routine address is null, then
    // the processor mode is defaulted to KernelMode and the APC is a special
    // APC. Otherwise, the processor mode is taken from the argument list.
    //

    if (Environment == CurrentApcEnvironment) {
        Apc->ApcStateIndex = Thread->ApcStateIndex;

    } else {

        ASSERT((Environment <= Thread->ApcStateIndex) || (Environment == InsertApcEnvironment));

        Apc->ApcStateIndex = (CCHAR)Environment;
    }

    Apc->Thread = Thread;
    Apc->KernelRoutine = KernelRoutine;
    Apc->RundownRoutine = RundownRoutine;
    Apc->NormalRoutine = NormalRoutine;
    if (ARGUMENT_PRESENT(NormalRoutine)) {
        Apc->ApcMode = ApcMode;
        Apc->NormalContext = NormalContext;

    } else {
        Apc->ApcMode = KernelMode;
        Apc->NormalContext = NIL;
    }

    Apc->Inserted = FALSE;
    return;
}

PLIST_ENTRY
KeFlushQueueApc (
    __inout PKTHREAD Thread,
    __in KPROCESSOR_MODE ApcMode
    )

/*++

Routine Description:

    This function flushes the APC queue selected by the specified processor
    mode for the specified thread. An APC queue is flushed by removing the
    listhead from the list, scanning the APC entries in the list, setting
    their inserted variables to FALSE, and then returning the address of the
    doubly linked list as the function value.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

    ApcMode - Supplies the processor mode of the APC queue that is to
        be flushed.

Return Value:

    The address of the first entry in the list of APC objects that were flushed
    from the specified APC queue.

--*/

{

    PKAPC Apc;
    PLIST_ENTRY FirstEntry;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the APC mode is user mode, then acquire the thread APC queue lock
    // to ensure that no further APCs are queued after a possible setting of
    // the thread APC queueable state.
    //

    if (ApcMode == UserMode) {
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
        if (IsListEmpty(&Thread->ApcState.ApcListHead[ApcMode])) {
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            return NULL;
        }

    } else {
        if (IsListEmpty(&Thread->ApcState.ApcListHead[ApcMode])) {
            return NULL;

        } else {
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);
        }
    }

    //
    // Get address of first APC in the list and check if the list is
    // empty or contains entries that should be flushed. If entries
    // should be flushed, then scan the list of APC objects and set their
    // inserted state to FALSE.
    //

    FirstEntry = Thread->ApcState.ApcListHead[ApcMode].Flink;
    if (FirstEntry == &Thread->ApcState.ApcListHead[ApcMode]) {
        FirstEntry = (PLIST_ENTRY)NULL;

    } else {
        RemoveEntryList(&Thread->ApcState.ApcListHead[ApcMode]);
        NextEntry = FirstEntry;
        do {
            Apc = CONTAINING_RECORD(NextEntry, KAPC, ApcListEntry);
            Apc->Inserted = FALSE;
            NextEntry = NextEntry->Flink;
        } while (NextEntry != FirstEntry);

        //
        // Reinitialize the header so the current thread may safely attach
        // to another process.
        //

        InitializeListHead(&Thread->ApcState.ApcListHead[ApcMode]);
    }

    //
    // Unlock the thread APC queue lock, lower IRQL to its previous value,
    // and return address of the first entry in list of APC objects that
    // were flushed.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return FirstEntry;
}

BOOLEAN
KeInsertQueueApc (
    __inout PRKAPC Apc,
    __in_opt PVOID SystemArgument1,
    __in_opt PVOID SystemArgument2,
    __in KPRIORITY Increment
    )

/*++

Routine Description:

    This function inserts an APC object into the APC queue specifed by the
    thread and processor mode fields of the APC object. If the APC object
    is already in an APC queue or APC queuing is disabled, then no operation
    is performed. Otherwise the APC object is inserted in the specified queue
    and appropriate scheduling decisions are made.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.

    SystemArgument1, SystemArgument2 - Supply a set of two arguments that
        contain untyped data provided by the executive.

    Increment - Supplies the priority increment that is to be applied if
        queuing the APC causes a thread wait to be satisfied.

Return Value:

    If the APC object is already in an APC queue or APC queuing is disabled,
    then a value of FALSE is returned. Otherwise a value of TRUE is returned.

--*/

{

    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_APC(Apc);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the thread APC queue lock.
    //

    Thread = Apc->Thread;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);

    //
    // If APC queuing is disabled or the APC is already inserted, then set
    // inserted to FALSE. Otherwise, set the system  parameter values in the
    // APC object, insert the APC in the thread APC queue, and set inserted to
    // true.
    //

    if ((Thread->ApcQueueable == FALSE) ||
        (Apc->Inserted == TRUE)) {
        Inserted = FALSE;

    } else {
        Apc->Inserted = TRUE;
        Apc->SystemArgument1 = SystemArgument1;
        Apc->SystemArgument2 = SystemArgument2;
        KiInsertQueueApc(Apc, Increment);
        Inserted = TRUE;
    }

    //
    // Unlock the thread APC queue lock, exit the scheduler, and return
    // whether the APC was inserted.
    //

    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return Inserted;
}

BOOLEAN
KeRemoveQueueApc (
    __inout PKAPC Apc
    )

/*++

Routine Description:

    This function removes an APC object from an APC queue. If the APC object
    is not in an APC queue, then no operation is performed. Otherwise the
    APC object is removed from its current queue and its inserted state is
    set FALSE.

Arguments:

    Apc - Supplies a pointer to a control object of type APC.

Return Value:

    If the APC object is not in an APC queue, then a value of FALSE is returned.
    Otherwise a value of TRUE is returned.

--*/

{

    PKAPC_STATE ApcState;
    BOOLEAN Inserted;
    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_APC(Apc);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the thread APC queue lock.
    //

    Thread = Apc->Thread;
    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock, &LockHandle);

    //
    // If the APC object is in an APC queue, then remove it from the queue
    // and set its inserted state to FALSE. If the queue becomes empty, set
    // the APC pending state to FALSE.
    //

    Inserted = Apc->Inserted;
    if (Inserted != FALSE) {
        Apc->Inserted = FALSE;
        ApcState = Thread->ApcStatePointer[Apc->ApcStateIndex];
        KiLockDispatcherDatabaseAtSynchLevel();
        if (RemoveEntryList(&Apc->ApcListEntry) != FALSE) {
            if (Apc->ApcMode == KernelMode) {
                ApcState->KernelApcPending = FALSE;

            } else {
                ApcState->UserApcPending = FALSE;
            }
        }

        KiUnlockDispatcherDatabaseFromSynchLevel();
    }

    //
    // Release the thread APC queue lock, lower IRQL to its previous value,
    // and return whether an APC object was removed from the APC queue.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return Inserted;
}

