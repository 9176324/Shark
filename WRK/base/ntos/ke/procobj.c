/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    procobj.c

Abstract:

    This module implements the machine independent functions to manipulate
    the kernel process object. Functions are provided to initialize, attach,
    detach, exclude, include, and set the base priority of process objects.

--*/

#include "ki.h"

#pragma alloc_text(PAGE, KeInitializeProcess)

//
// Define forward referenced function prototypes.
//

VOID
KiAttachProcess (
    __inout PRKTHREAD Thread,
    __in PRKPROCESS Process,
    __in PKLOCK_QUEUE_HANDLE LockHandle,
    __out PRKAPC_STATE SavedApcState
    );

VOID
KiMoveApcState (
    __in PKAPC_STATE Source,
    __out PKAPC_STATE Destination
    );

#if !defined(NT_UP)

FORCEINLINE
VOID
KiSetIdealNodeProcess (
    __inout PKPROCESS Process,
    __in KAFFINITY Affinity
    )

/*++

Routine Description:

    This function sets the ideal node for a process based on the specified
    affinity and the node generation seed.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    Affinity - Supplies the set of processors on which children threads
        of the process can execute.

Return Value:

    None.

--*/

{

    ULONG Index;
    PKNODE Node;
    ULONG NodeNumber;

    //
    // Select the ideal node for the process.
    //

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

    Process->IdealNode = (UCHAR)NodeNumber;
    Node = KeNodeBlock[NodeNumber];

    ASSERT((Node->ProcessorMask & Affinity) != 0);

    Process->ThreadSeed = (UCHAR)KeFindNextRightSetAffinity(Node->Seed,
                                                            Node->ProcessorMask & Affinity);

    Node->Seed = Process->ThreadSeed;
    return;
}

#endif

VOID
KeInitializeProcess (
    __out PRKPROCESS Process,
    __in KPRIORITY BasePriority,
    __in KAFFINITY Affinity,
    __in ULONG_PTR DirectoryTableBase[2],
    __in BOOLEAN Enable
    )

/*++

Routine Description:

    This function initializes a kernel process object. The base priority,
    affinity, and page frame numbers for the process page table directory
    and hyper space are stored in the process object.

    N.B. It is assumed that the process object is zeroed.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    BasePriority - Supplies the base priority of the process.

    Affinity - Supplies the set of processors on which children threads
        of the process can execute.

    DirectoryTableBase - Supplies a pointer to an array whose fist element
        is the value that is to be loaded into the Directory Table Base
        register when a child thread is dispatched for execution and whose
        second element contains the page table entry that maps hyper space.

    Enable - Supplies a boolean value that determines the default
        handling of data alignment exceptions for child threads. A value
        of TRUE causes all data alignment exceptions to be automatically
        handled by the kernel. A value of FALSE causes all data alignment
        exceptions to be actually raised as exceptions.

Return Value:

    None.

--*/

{

    //
    // Initialize the standard dispatcher object header and set the initial
    // signal state of the process object.
    //

    Process->Header.Type = ProcessObject;
    Process->Header.Size = sizeof(KPROCESS) / sizeof(LONG);
    InitializeListHead(&Process->Header.WaitListHead);

    //
    // Initialize the base priority, affinity, directory table base values,
    // autoalignment, and stack count.
    //

    Process->BasePriority = (SCHAR)BasePriority;
    Process->Affinity = Affinity;
    Process->AutoAlignment = Enable;
    Process->DirectoryTableBase[0] = DirectoryTableBase[0];
    Process->DirectoryTableBase[1] = DirectoryTableBase[1];

    //
    // Initialize the stack count, profile listhead, ready queue list head,
    // accumulated runtime, process quantum, thread quantum, and thread list
    // head.
    //

    InitializeListHead(&Process->ProfileListHead);
    InitializeListHead(&Process->ReadyListHead);
    InitializeListHead(&Process->ThreadListHead);
    Process->QuantumReset = THREAD_QUANTUM;

    //
    // Initialize the process state and set the thread processor selection
    // seed.
    //

    Process->State = ProcessInMemory;

    //
    // Select the ideal node for the process.
    //

#if !defined(NT_UP)

    KiSetIdealNodeProcess(Process, Affinity);

#endif

    //
    // Initialize IopmBase and Iopl flag for this process (i386 only)
    //

#if defined(_X86_)

    Process->IopmOffset = KiComputeIopmOffset(IO_ACCESS_MAP_NONE);

#endif // defined(_X86_)

    return;
}

VOID
KeAttachProcess (
    __inout PRKPROCESS Process
    )

/*++

Routine Description:

    This function attaches a thread to a target process' address space
    if, and only if, there is not already a process attached.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the target process is not the current process, then attach the
    // target process.
    //

    Thread = KeGetCurrentThread();
    if (Thread->ApcState.Process != Process) {

        //
        // If the current thread is already attached or executing a DPC, then
        // bugcheck.
        //
    
        if ((Thread->ApcStateIndex != 0) ||
            (KeIsExecutingDpc() != FALSE)) {
    
            KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                         (ULONG_PTR)Process,
                         (ULONG_PTR)Thread->ApcState.Process,
                         (ULONG)Thread->ApcStateIndex,
                         (ULONG)KeIsExecutingDpc());
        }
    
        //
        // Raise IRQL to SYNCH_LEVEL, acquire the thread APC queue lock,
        // acquire the dispatcher database lock, and attach to the specified
        // process.
        //
        // N.B. All lock are released by the internal attach routine.
        //

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();
        KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
    }

    return;
}

LOGICAL
KeForceAttachProcess (
    __inout PRKPROCESS Process
    )

/*++

Routine Description:

    This function forces an attach of a thread to a target process' address
    space if the process is not current being swapped into or out of memory.

    N.B. This function is for use by memory management ONLY.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the current thread is already attached or executing a DPC, then
    // bugcheck.
    //

    Thread = KeGetCurrentThread();
    if ((Thread->ApcStateIndex != 0) ||
        (KeIsExecutingDpc() != FALSE)) {

        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

    //
    // If the target process is not the current process, then attach the
    // target process if the process is not currently being swapped in or
    // out of memory.
    //

    if (Thread->ApcState.Process != Process) {

        //
        // Raise IRQL to SYNCH_LEVEL, acquire the thread APC queue lock, and
        // acquire the dispatcher database lock.
        //

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();

        //
        // If the target process is currently being swapped into or out of
        // memory, then return a value of FALSE. Otherwise, force the process
        // to be inswapped.
        //

        if ((Process->State == ProcessInSwap) ||
            (Process->State == ProcessInTransition) ||
            (Process->State == ProcessOutTransition) ||
            (Process->State == ProcessOutSwap)) {
            KiUnlockDispatcherDatabaseFromSynchLevel();
            KeReleaseInStackQueuedSpinLock(&LockHandle);
            return FALSE;

        } else {

            //
            // Force the process state to in memory and attach the target process.
            //
            // N.B. All lock are released by the internal attach routine.
            //

            Process->State = ProcessInMemory;
            KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
        }
    }

    return TRUE;
}

VOID
KeStackAttachProcess (
    __inout PRKPROCESS Process,
    __out PRKAPC_STATE ApcState
    )

/*++

Routine Description:

    This function attaches a thread to a target process' address space
    and returns information about a previous attached process.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the current thread is executing a DPC, then bugcheck.
    //

    Thread = KeGetCurrentThread();
    if (KeIsExecutingDpc() != FALSE) {
        KeBugCheckEx(INVALID_PROCESS_ATTACH_ATTEMPT,
                     (ULONG_PTR)Process,
                     (ULONG_PTR)Thread->ApcState.Process,
                     (ULONG)Thread->ApcStateIndex,
                     (ULONG)KeIsExecutingDpc());
    }

    //
    // If the target process is not the current process, then attach the
    // target process. Otherwise, return a distinguished process value to
    // indicate that an attach was not performed.
    //

    if (Thread->ApcState.Process == Process) {
        ApcState->Process = (PRKPROCESS)1;

    } else {

        //
        // Raise IRQL to SYNCH_LEVEL, acquire the thread APC queue lock, and
        // acquire the dispatcher database lock.
        //

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();

        //
        // If the current thread is attached to a process, then save the
        // current APC state in the callers APC state structure. Otherwise,
        // save the current APC state in the saved APC state structure, and
        // return a NULL process pointer.
        //
        // N.B. All lock are released by the internal attach routine.
        //

        if (Thread->ApcStateIndex != 0) {
            KiAttachProcess(Thread, Process, &LockHandle, ApcState);

        } else {
            KiAttachProcess(Thread, Process, &LockHandle, &Thread->SavedApcState);
            ApcState->Process = NULL;
        }
    }

    return;
}

VOID
KeDetachProcess (
    VOID
    )

/*++

Routine Description:

    This function detaches a thread from another process' address space.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the current thread is attached to another process, then detach
    // it.
    //

    Thread = KeGetCurrentThread();
    if (Thread->ApcStateIndex != 0) {

        //
        // Raise IRQL to SYNCH_LEVEL and acquire the thread APC queue lock.
        //

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

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

#if !defined(NT_UP)

        while (Thread->ApcState.KernelApcPending &&
               (Thread->SpecialApcDisable == 0) &&
               (LockHandle.OldIrql < APC_LEVEL)) {

            //
            // Unlock the thread APC lock and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will
            // result in the delivery of the kernel APC if possible.
            //

            KeReleaseInStackQueuedSpinLock(&LockHandle);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                       &LockHandle);
        }

#endif

        //
        // If a kernel APC is in progress, the kernel APC queue is not empty,
        // or the user APC queues is not empty, then bugcheck.
        //

#if DBG

        if ((Thread->ApcState.KernelApcInProgress) ||
            (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) ||
            (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE)) {

            KeBugCheck(INVALID_PROCESS_DETACH_ATTEMPT);
        }

#endif

        //
        // Lock the dispatcher database, unbias current process stack count,
        // and check if the process should be swapped out of memory.
        //

        Process = Thread->ApcState.Process;
        KiLockDispatcherDatabaseAtSynchLevel();

        ASSERT(Process->StackCount != 0);

        ASSERT(Process->State == ProcessInMemory);

        Process->StackCount -= 1;
        if ((Process->StackCount == 0) &&
            (IsListEmpty(&Process->ThreadListHead) == FALSE)) {

            Process->State = ProcessOutTransition;
            InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

        //
        // Unlock dispatcher database, but remain at SYNCH_LEVEL.
        //
    
        KiUnlockDispatcherDatabaseFromSynchLevel();

        //
        // Restore APC state and check whether the kernel APC queue contains
        // an entry. If the kernel APC queue contains an entry then set kernel
        // APC pending and request a software interrupt at APC_LEVEL.
        //

        KiMoveApcState(&Thread->SavedApcState, &Thread->ApcState);
        Thread->SavedApcState.Process = (PKPROCESS)NULL;
        Thread->ApcStatePointer[0] = &Thread->ApcState;
        Thread->ApcStatePointer[1] = &Thread->SavedApcState;
        Thread->ApcStateIndex = 0;

        //
        // Release the thread APC queue lock, swap the address space back to
        // the parent process, and exit the scheduler.
        //
    
        KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
        KiSwapProcess(Thread->ApcState.Process, Process);
        KiExitDispatcher(LockHandle.OldIrql);

        //
        // Initiate an APC interrupt if there are pending kernel APC's.
        //

        if (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {
            Thread->ApcState.KernelApcPending = TRUE;
            KiRequestSoftwareInterrupt(APC_LEVEL);
        }
    }

    return;
}

VOID
KeUnstackDetachProcess (
    __in PRKAPC_STATE ApcState
    )

/*++

Routine Description:

    This function detaches a thread from another process' address space
    and restores previous attach state.

Arguments:

    ApcState - Supplies a pointer to an APC state structure that was returned
        from a previous call to stack attach process.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PKPROCESS Process;
    PKTHREAD Thread;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the APC state has a distinguished process pointer value, then no
    // attach was performed on the paired call to stack attach process.
    //

    if (ApcState->Process != (PRKPROCESS)1) {

        //
        // Raise IRQL to SYNCH_LEVEL and acquire the thread APC queue lock.
        //

        Thread = KeGetCurrentThread();
        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   &LockHandle);

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

#if !defined(NT_UP)

        while (Thread->ApcState.KernelApcPending &&
               (Thread->SpecialApcDisable == 0) &&
               (LockHandle.OldIrql < APC_LEVEL)) {

            //
            // Unlock the thread APC lock and lower IRQL to its previous
            // value. An APC interrupt will immediately occur which will
            // result in the delivery of the kernel APC if possible.
            //

            KeReleaseInStackQueuedSpinLock(&LockHandle);
            KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                       &LockHandle);
        }

#endif

        //
        // If the APC state is the original APC state, a kernel APC is in
        // progress, the kernel APC is nbot empty, or the user APC queues is
        // not empty, then bugcheck.
        //

        if ((Thread->ApcStateIndex == 0) ||
             (Thread->ApcState.KernelApcInProgress) ||
             (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) ||
             (IsListEmpty(&Thread->ApcState.ApcListHead[UserMode]) == FALSE)) {

            KeBugCheck(INVALID_PROCESS_DETACH_ATTEMPT);
        }

        //
        // Lock the dispatcher database, unbias current process stack count,
        // and check if the process should be swapped out of memory.
        //

        Process = Thread->ApcState.Process;
        KiLockDispatcherDatabaseAtSynchLevel();

        ASSERT(Process->StackCount != 0);

        ASSERT(Process->State == ProcessInMemory);

        Process->StackCount -= 1;
        if ((Process->StackCount == 0) &&
            (IsListEmpty(&Process->ThreadListHead) == FALSE)) {

            Process->State = ProcessOutTransition;
            InterlockedPushEntrySingleList(&KiProcessOutSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

        //
        // Unlock dispatcher database, but remain at SYNCH_LEVEL.
        //
    
        KiUnlockDispatcherDatabaseFromSynchLevel();

        //
        // Restore APC state and check whether the kernel APC queue contains
        // an entry. If the kernel APC queue contains an entry then set kernel
        // APC pending and request a software interrupt at APC_LEVEL.
        //

        if (ApcState->Process != NULL) {
            KiMoveApcState(ApcState, &Thread->ApcState);

        } else {
            KiMoveApcState(&Thread->SavedApcState, &Thread->ApcState);
            Thread->SavedApcState.Process = (PKPROCESS)NULL;
            Thread->ApcStatePointer[0] = &Thread->ApcState;
            Thread->ApcStatePointer[1] = &Thread->SavedApcState;
            Thread->ApcStateIndex = 0;
        }

        //
        // Release the thread APC queue lock, swap the address space back to
        // the parent process, and exit the scheduler.
        //
    
        KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
        KiSwapProcess(Thread->ApcState.Process, Process);
        KiExitDispatcher(LockHandle.OldIrql);

        //
        // Initiate an APC interrupt if we need to
        //

        if (IsListEmpty(&Thread->ApcState.ApcListHead[KernelMode]) == FALSE) {
            Thread->ApcState.KernelApcPending = TRUE;
            KiRequestSoftwareInterrupt(APC_LEVEL);
        }
    }

    return;
}

ULONG
KeQueryRuntimeProcess (
    __in PKPROCESS Process,
    __out PULONG UserTime
    )

/*++

Routine Description:

    This function queries the user and kernel runtime for the specifed process.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    UserTime - Supplies a pointer to a variable that receives the total
        user time.

Return Value:

    The total kernel time is returned as the function value.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG KernelTime;
    PLIST_ENTRY NextEntry;
    PKTHREAD Thread;
    ULONG TotalTime;

    ASSERT_PROCESS(Process);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH level and acquire the process lock.
    //
    // Sum the process current kernel and user time with the kernel and user
    // time of all the process threads.
    //

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KernelTime = Process->KernelTime;
    TotalTime = Process->UserTime;
    NextEntry = Process->ThreadListHead.Flink;
    while (NextEntry != &Process->ThreadListHead) {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);
        KernelTime += Thread->KernelTime;
        TotalTime += Thread->UserTime;
        NextEntry = NextEntry->Flink;
    }

    //
    // Unlock the process lock and return the total user and kernel time.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    *UserTime = TotalTime;
    return KernelTime;
}

VOID
KeQueryValuesProcess (
    __in PKPROCESS Process,
    __out PKPROCESS_VALUES Values
    )

/*++

Routine Description:

    This function queries the user runtime, the kernel runtime, and the I/O
    statistics for the specifed process.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    Values - Supplies a pointer to a structure that will receive the runtime
        and I/O statistics values.

Return Value:

    None.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG KernelTime;
    PLIST_ENTRY NextEntry;
    PKTHREAD Thread;
    ULONG UserTime;

    ASSERT_PROCESS(Process);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH level and acquire the process lock.
    //
    // Sum the process current kernel time, user time, and I/O statistics
    // with the kernel time, user time, and I/O statistics for all of the
    // process threads.
    //

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KernelTime = Process->KernelTime;
    UserTime = Process->UserTime;
    Values->ReadOperationCount = ((PEPROCESS)Process)->ReadOperationCount.QuadPart;
    Values->WriteOperationCount = ((PEPROCESS)Process)->WriteOperationCount.QuadPart;
    Values->OtherOperationCount = ((PEPROCESS)Process)->OtherOperationCount.QuadPart;
    Values->ReadTransferCount = ((PEPROCESS)Process)->ReadTransferCount.QuadPart;
    Values->WriteTransferCount = ((PEPROCESS)Process)->WriteTransferCount.QuadPart;
    Values->OtherTransferCount = ((PEPROCESS)Process)->OtherTransferCount.QuadPart;
    NextEntry = Process->ThreadListHead.Flink;
    while (NextEntry != &Process->ThreadListHead) {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);
        KernelTime += Thread->KernelTime;
        UserTime += Thread->UserTime;

#if defined(_WIN64)

        Values->ReadOperationCount += Thread->ReadOperationCount;
        Values->WriteOperationCount += Thread->WriteOperationCount;
        Values->OtherOperationCount += Thread->OtherOperationCount;
        Values->ReadTransferCount += Thread->ReadTransferCount;
        Values->WriteTransferCount += Thread->WriteTransferCount;
        Values->OtherTransferCount += Thread->OtherTransferCount;

#endif

        NextEntry = NextEntry->Flink;
    }

    //
    // Unlock the process lock and return the total user and kernel time.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    Values->KernelTime = UInt32x32To64(KernelTime, KeMaximumIncrement);
    Values->UserTime = UInt32x32To64(UserTime, KeMaximumIncrement);
    return;
}

LONG
KeReadStateProcess (
    __in PRKPROCESS Process
    )

/*++

Routine Description:

    This function reads the current signal state of a process object.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

Return Value:

    The current signal state of the process object.

--*/

{

    ASSERT_PROCESS(Process);

    //
    // Return current signal state of process object.
    //

    return Process->Header.SignalState;
}

LONG
KeSetProcess (
    __inout PRKPROCESS Process,
    __in KPRIORITY Increment,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    This function sets the signal state of a process object to Signaled
    and attempts to satisfy as many Waits as possible. The previous
    signal state of the process object is returned as the function value.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    Increment - Supplies the priority increment that is to be applied
       if setting the process causes a Wait to be satisfied.

    Wait - Supplies a boolean value that signifies whether the call to
       KeSetProcess will be immediately followed by a call to one of the
       kernel Wait functions.

Return Value:

    The previous signal state of the process object.

--*/

{

    KIRQL OldIrql;
    LONG OldState;
    PRKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to dispatcher level and lock dispatcher database.
    //

    KiLockDispatcherDatabase(&OldIrql);

    //
    // If the previous state of the process object is Not-Signaled and
    // the wait queue is not empty, then satisfy as many Waits as
    // possible.
    //

    OldState = Process->Header.SignalState;
    Process->Header.SignalState = 1;
    if ((OldState == 0) &&
        (IsListEmpty(&Process->Header.WaitListHead) == FALSE)) {

        KiWaitTestWithoutSideEffects(Process, Increment);
    }

    //
    // If the value of the Wait argument is TRUE, then return to the
    // caller with IRQL raised and the dispatcher database locked. Else
    // release the dispatcher database lock and lower IRQL to its
    // previous value.
    //

    if (Wait) {
        Thread = KeGetCurrentThread();
        Thread->WaitNext = Wait;
        Thread->WaitIrql = OldIrql;

    } else {
        KiUnlockDispatcherDatabase(OldIrql);
    }

    //
    // Return previous signal state of process object.
    //

    return OldState;
}

KAFFINITY
KeSetAffinityProcess (
    __inout PKPROCESS Process,
    __in KAFFINITY Affinity
    )

/*++

Routine Description:

    This function sets the affinity of a process to the specified value and
    also sets the affinity of each thread in the process to the specified
    value.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    Affinity - Supplies the new of set of processors on which the threads
        in the process can run.

Return Value:

    The previous affinity of the specified process is returned as the function
    value.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    KAFFINITY OldAffinity;
    PKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT((Affinity & KeActiveProcessors) != 0);

    //
    // Raise IRQL to SYNCH_LEVEL, acquire the process lock, and acquire the
    // dispatcher databack lock at SYNCH_LEVEL.
    //

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

    //
    // Capture the current affinity of the specified process and set the
    // affinity of the process.
    //

    OldAffinity = Process->Affinity;
    Process->Affinity = Affinity;

    //
    // If the new affinity does not intersect with the process ideal node
    // affinity, then select a new process ideal node.
    //

#if !defined(NT_UP)

    if ((Affinity & KeNodeBlock[Process->IdealNode]->ProcessorMask) == 0) {
        KiSetIdealNodeProcess(Process, Affinity);
    }

#endif

    //
    // Set the affinity of all process threads.
    //

    NextEntry = Process->ThreadListHead.Flink;
    while (NextEntry != &Process->ThreadListHead) {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);
        KiSetAffinityThread(Thread, Affinity);
        NextEntry = NextEntry->Flink;
    }

    //
    // Unlock dispatcher database, unlock the process lock, exit the
    // scheduler, and return the previous process affinity.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldAffinity;
}

KPRIORITY
KeSetPriorityAndQuantumProcess (
    __inout PKPROCESS Process,
    __in KPRIORITY NewBase,
    __in SCHAR QuantumReset
    )

/*++

Routine Description:

    This function sets the base priority and quantum reset of a process to a
    new value and adjusts the priority, base priority, and quantum reset value
    of all child threads as appropriate. 

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    NewBase - Supplies the new base priority of the process.

    QuantumReset - Supplies the new quantum rest value if nonzero.

Return Value:

    The previous base priority of the process.

--*/

{

    KPRIORITY Adjustment;
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    KPRIORITY NewPriority;
    KPRIORITY OldBase;
    PKTHREAD Thread;

    ASSERT_PROCESS(Process);
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // If the new priority is equal to the old priority, then do not change
    // the process priority or the quantum reset value and return the old
    // priority.
    //
    // N.B. This check can be made without holding the dispatcher lock since
    // nothing needs to be protected, and any race condition that can exist
    // calling this routine exists with or without the lock being held.
    //

    if (Process->BasePriority == NewBase) {
        return NewBase;
    }

    //
    // If the new base priority is zero, then default it to one.
    //

    if (NewBase == 0) {
        NewBase = 1;
    }

    //
    // Raise IRQL to SYNCH level, acquire the process lock, and lock the
    // dispatcher database.
    //

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    KiLockDispatcherDatabaseAtSynchLevel();

    //
    // If the quantum reset value is nonzsro, then set the new quantum reset
    // value of the process.
    //

    if (QuantumReset != 0) {
        Process->QuantumReset = QuantumReset;
    }

    //
    // Save the current process base priority, set the new process base
    // priority, compute the adjustment value, and adjust the priority
    // and base priority of all child threads as appropriate.
    //

    OldBase = Process->BasePriority;
    Process->BasePriority = (SCHAR)NewBase;
    Adjustment = NewBase - OldBase;
    NextEntry = Process->ThreadListHead.Flink;
    if (NewBase >= LOW_REALTIME_PRIORITY) {
        while (NextEntry != &Process->ThreadListHead) {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

            //
            // If the quantum reset value is nonzsro, then set the new quantum
            // reset value of the thread.
            //
        
            if (QuantumReset != 0) {
                Thread->QuantumReset = QuantumReset;
            }
        
            //
            // Acquire the thread lock and compute the new base priority of
            // the thread.
            //

            KiAcquireThreadLock(Thread);
            NewPriority = Thread->BasePriority + Adjustment;

            //
            // If the new base priority is outside the realtime class,
            // then limit the change to the realtime class.
            //

            if (NewPriority < LOW_REALTIME_PRIORITY) {
                NewPriority = LOW_REALTIME_PRIORITY;

            } else if (NewPriority > HIGH_PRIORITY) {
                NewPriority = HIGH_PRIORITY;
            }

            //
            // Set the base priority and the current priority of the
            // thread to the appropriate value.
            //
            // N.B. If priority saturation occured the last time the thread
            //      base priority was set and the new process base priority
            //      is not crossing from variable to realtime, then it is not
            //      necessary to change the thread priority.
            //

            if ((Thread->Saturation == 0) || (OldBase < LOW_REALTIME_PRIORITY)) {
                if (Thread->Saturation > 0) {
                    NewPriority = HIGH_PRIORITY;

                } else if (Thread->Saturation < 0) {
                    NewPriority = LOW_REALTIME_PRIORITY;
                }

                Thread->BasePriority = (SCHAR)NewPriority;
                Thread->Quantum = Thread->QuantumReset;
                Thread->PriorityDecrement = 0;
                KiSetPriorityThread(Thread, NewPriority);
            }

            KiReleaseThreadLock(Thread);
            NextEntry = NextEntry->Flink;
        }

    } else {
        while (NextEntry != &Process->ThreadListHead) {
            Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);

            //
            // If the quantum reset value is nonzsro, then set the new quantum
            // reset value of the thread.
            //
        
            if (QuantumReset != 0) {
                Thread->QuantumReset = QuantumReset;
            }
        
            //
            // Acquire the thread lock and compute the new base priority of
            // the thread.
            //

            KiAcquireThreadLock(Thread);
            NewPriority = Thread->BasePriority + Adjustment;

            //
            // If the new base priority is outside the variable class,
            // then limit the change to the variable class.
            //

            if (NewPriority >= LOW_REALTIME_PRIORITY) {
                NewPriority = LOW_REALTIME_PRIORITY - 1;

            } else if (NewPriority <= LOW_PRIORITY) {
                NewPriority = 1;
            }

            //
            // Set the base priority and the current priority of the
            // thread to the computed value and reset the thread quantum.
            //
            // N.B. If priority saturation occured the last time the thread
            //      base priority was set and the new process base priority
            //      is not crossing from realtime to variable, then it is not
            //      necessary to change the thread priority.
            //

            if ((Thread->Saturation == 0) || (OldBase >= LOW_REALTIME_PRIORITY)) {
                if (Thread->Saturation > 0) {
                    NewPriority = LOW_REALTIME_PRIORITY - 1;

                } else if (Thread->Saturation < 0) {
                    NewPriority = 1;
                }

                Thread->BasePriority = (SCHAR)NewPriority;
                Thread->Quantum = Thread->QuantumReset;
                Thread->PriorityDecrement = 0;
                KiSetPriorityThread(Thread, NewPriority);
            }

            KiReleaseThreadLock(Thread);
            NextEntry = NextEntry->Flink;
        }
    }

    //
    // Unlock dispatcher database, unlock the process lock, exit the
    // scheduler, and return the previous base priority.
    //

    KiUnlockDispatcherDatabaseFromSynchLevel();
    KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    KiExitDispatcher(LockHandle.OldIrql);
    return OldBase;
}

VOID
KeSetQuantumProcess (
    __inout PKPROCESS Process,
    __in SCHAR QuantumReset
    )

/*++

Routine Description:

    This function sets the quantum for a process to a new value and sets
    the quantum reset value of all child threads.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    QuantumReset - Supplies the new quantum reset value of the process.

Return Value:

    The previous base priority of the process.

--*/

{

    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PKTHREAD Thread;

    ASSERT_PROCESS(Process);

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    //
    // Raise IRQL to SYNCH level and acquire the process lock.
    //
    // Set the new process quantum reset value and the quantum reset value
    // of all child threads.
    //

    KeAcquireInStackQueuedSpinLockRaiseToSynch(&Process->ProcessLock, &LockHandle);
    Process->QuantumReset = QuantumReset;
    NextEntry = Process->ThreadListHead.Flink;
    while (NextEntry != &Process->ThreadListHead) {
        Thread = CONTAINING_RECORD(NextEntry, KTHREAD, ThreadListEntry);
        Thread->QuantumReset = QuantumReset;
        NextEntry = NextEntry->Flink;
    }

    //
    // Unlock the process lock and return.
    //

    KeReleaseInStackQueuedSpinLock(&LockHandle);
    return;
}

LOGICAL
KeSetAutoAlignmentProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Enable
    )

/*++

Routine Description:

    This function sets the data alignment handling mode for the specified
    process.

Arguments:

    Process  - Supplies a pointer to a dispatcher object of type process.

    Enable - Supplies a boolean value that determines the handling of data
        alignment exceptions for the specified process.

Return Value:

    The previous value of auto alignment for the specified process is returned
    as the function value.

--*/

{

    ASSERT_PROCESS(Process);

    //
    // Capture the previous data alignment handling mode and set the specified
    // data alignment mode.
    //

    if (Enable != FALSE) {
        return InterlockedBitTestAndSet(&Process->ProcessFlags,
                                        KPROCESS_AUTO_ALIGNMENT_BIT);

    } else {
        return InterlockedBitTestAndReset(&Process->ProcessFlags,
                                          KPROCESS_AUTO_ALIGNMENT_BIT);
    }
}

LOGICAL
KeSetDisableBoostProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Disable
    )

/*++

Routine Description:

    This function disables or enables priority boosts for the specified
    process.

Arguments:

    Process - Supplies a pointer to a dispatcher object of type process.

    Disable - Supplies a logical value that determines whether priority
        boosts are enabled or disabled for the specfied process.

Return Value:

    The previous value of disable priority boost for the specified process
    is returned as the function value.

--*/

{

    ASSERT_PROCESS(Process);

    //
    // Capture the current state of disable boost and set its state to the
    // specified value.
    //

    if (Disable != FALSE) {
        return InterlockedBitTestAndSet(&Process->ProcessFlags,
                                        KPROCESS_DISABLE_BOOST_BIT);

    } else {
        return InterlockedBitTestAndReset(&Process->ProcessFlags,
                                          KPROCESS_DISABLE_BOOST_BIT);
    }
}

LOGICAL
KeSetDisableQuantumProcess (
    __inout PKPROCESS Process,
    __in LOGICAL Disable
    )

/*++

Routine Description:

    This function disables or enables quantum runout for realtime threads in
    the specified process.

Arguments:

    Process  - Supplies a pointer to a dispatcher object of type process.

    Disable - Supplies a logical value that determines whether quantum
        runout is disabled or enabled for the specified process.

Return Value:

    The previous value of the disable quantum state for the specified process
    is returned as the function value.

--*/

{

    ASSERT_PROCESS(Process);

    //
    // Capture the current state of disable quantum and set its state to the
    // specified value.
    //

    if (Disable != FALSE) {
        return InterlockedBitTestAndSet(&Process->ProcessFlags,
                                        KPROCESS_DISABLE_QUANTUM_BIT);

    } else {
        return InterlockedBitTestAndReset(&Process->ProcessFlags,
                                          KPROCESS_DISABLE_QUANTUM_BIT);
    }
}

VOID
KiAttachProcess (
    __inout PRKTHREAD Thread,
    __in PKPROCESS Process,
    __in PKLOCK_QUEUE_HANDLE LockHandle,
    __out PRKAPC_STATE SavedApcState
    )

/*++

Routine Description:

    This function attaches a thread to a target process' address space.

    N.B. The dispatcher database lock and the thread APC queue lock must be
         held when this routine is called.

Arguments:

    Thread - Supplies a pointer to the current thread object.

    Process - Supplies a pointer to the current process object.

    Lockhandle - Supplies the address of the lock handle that was used to
        acquire the thread APC lock.

    SavedApcState - Supplies a pointer to the APC state structure that
        receives the saved APC state.

Return Value:

    None.

--*/

{

    PLIST_ENTRY NextEntry;
    PRKTHREAD OutThread;

    ASSERT(Process != Thread->ApcState.Process);

    //
    // Bias the stack count of the target process to signify that a
    // thread exists in that process with a stack that is resident.
    //

    ASSERT(Process->StackCount != MAXULONG_PTR);

    Process->StackCount += 1;

    //
    // Save current APC state and initialize a new APC state.
    //

    KiMoveApcState(&Thread->ApcState, SavedApcState);
    InitializeListHead(&Thread->ApcState.ApcListHead[KernelMode]);
    InitializeListHead(&Thread->ApcState.ApcListHead[UserMode]);
    Thread->ApcState.KernelApcInProgress = FALSE;
    Thread->ApcState.KernelApcPending  = FALSE;
    Thread->ApcState.UserApcPending = FALSE;
    if (SavedApcState == &Thread->SavedApcState) {
        Thread->ApcStatePointer[0] = &Thread->SavedApcState;
        Thread->ApcStatePointer[1] = &Thread->ApcState;
        Thread->ApcStateIndex = 1;
    }

    //
    // If the target process is in memory, then immediately enter the
    // new address space by loading a new Directory Table Base. Otherwise,
    // insert the current thread in the target process ready list, inswap
    // the target process if necessary, select a new thread to run on the
    // the current processor and context switch to the new thread.
    //

    if (Process->State == ProcessInMemory) {
        Thread->ApcState.Process = Process;

        //
        // It is possible that the process is in memory, but there exist
        // threads in the process ready list. This can happen when memory
        // management forces a process attach.
        //

        NextEntry = Process->ReadyListHead.Flink;
        while (NextEntry != &Process->ReadyListHead) {
            OutThread = CONTAINING_RECORD(NextEntry, KTHREAD, WaitListEntry);
            RemoveEntryList(NextEntry);
            OutThread->ProcessReadyQueue = FALSE;
            KiReadyThread(OutThread);
            NextEntry = Process->ReadyListHead.Flink;
        }

        //
        // Unlock dispatcher database, unlock the thread APC lock, swap the
        // address space to the target process, and exit the scheduler.
        //
    
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSwapProcess(Process, SavedApcState->Process);
        KiExitDispatcher(LockHandle->OldIrql);

    } else {
        Thread->State = Ready;
        Thread->ProcessReadyQueue = TRUE;
        InsertTailList(&Process->ReadyListHead, &Thread->WaitListEntry);
        if (Process->State == ProcessOutOfMemory) {
            Process->State = ProcessInTransition;
            InterlockedPushEntrySingleList(&KiProcessInSwapListHead,
                                           &Process->SwapListEntry);

            KiSetInternalEvent(&KiSwapEvent, KiSwappingThread);
        }

        //
        // Set the current thread wait IRQL, release the thread APC lock,
        // set swap busy for the current thread, unlock the dispatcher
        // database, and swap context to a new thread.
        //

        Thread->WaitIrql = LockHandle->OldIrql;
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSetContextSwapBusy(Thread);
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KiSwapThread(Thread, KeGetCurrentPrcb());

        //
        // Acquire the APC lock, acquire the dispatcher database lock, set
        // the new process object address, unlock the dispatcher database,
        // unlock the APC lock, swap the address space to the target process,
        // and exit the scheduler.
        //

        KeAcquireInStackQueuedSpinLockRaiseToSynch(&Thread->ApcQueueLock,
                                                   LockHandle);

        KiLockDispatcherDatabaseAtSynchLevel();
        Thread->ApcState.Process = Process;
        KiUnlockDispatcherDatabaseFromSynchLevel();
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
        KiSwapProcess(Process, SavedApcState->Process);
        KiExitDispatcher(LockHandle->OldIrql);
    }

    return;
}

VOID
KiMoveApcState (
    __in PKAPC_STATE Source,
    __out PKAPC_STATE Destination
    )

/*++

Routine Description:

    This function moves the APC state from the source structure to the
    destination structure and reinitializes list headers as appropriate.

Arguments:

    Source - Supplies a pointer to the source APC state structure.

    Destination - Supplies a pointer to the destination APC state structure.


Return Value:

    None.

--*/

{

    PLIST_ENTRY First;
    PLIST_ENTRY Last;

    //
    // Copy the APC state from the source to the destination.
    //
    // N.B. Only the actual data in the APC state structure is moved without
    //      moving any of the padding.
    //

    RtlCopyMemory(Destination, Source, KAPC_STATE_ACTUAL_LENGTH);
    if (IsListEmpty(&Source->ApcListHead[KernelMode]) != FALSE) {
        InitializeListHead(&Destination->ApcListHead[KernelMode]);

    } else {
        First = Source->ApcListHead[KernelMode].Flink;
        Last = Source->ApcListHead[KernelMode].Blink;
        Destination->ApcListHead[KernelMode].Flink = First;
        Destination->ApcListHead[KernelMode].Blink = Last;
        First->Blink = &Destination->ApcListHead[KernelMode];
        Last->Flink = &Destination->ApcListHead[KernelMode];
    }

    if (IsListEmpty(&Source->ApcListHead[UserMode]) != FALSE) {
        InitializeListHead(&Destination->ApcListHead[UserMode]);

    } else {
        First = Source->ApcListHead[UserMode].Flink;
        Last = Source->ApcListHead[UserMode].Blink;
        Destination->ApcListHead[UserMode].Flink = First;
        Destination->ApcListHead[UserMode].Blink = Last;
        First->Blink = &Destination->ApcListHead[UserMode];
        Last->Flink = &Destination->ApcListHead[UserMode];
    }

    return;
}

