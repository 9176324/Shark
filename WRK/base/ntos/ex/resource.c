/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    resource.c

Abstract:

    This module implements the executive functions to acquire and release
    a shared resource.


Environment:

    Substantially rewritten to make fastlock optimizations portable
    across all platforms and to improve the algorithms used to be
    perfectly synchronized.

--*/

#include "exp.h"
#pragma hdrstop
#include "nturtl.h"

//
// Define local macros to test resource state.
//

#define IsExclusiveWaiting(a) ((a)->NumberOfExclusiveWaiters != 0)
#define IsSharedWaiting(a) ((a)->NumberOfSharedWaiters != 0)
#define IsOwnedExclusive(a) (((a)->Flag & ResourceOwnedExclusive) != 0)
#define IsBoostAllowed(a) (((a)->Flag & DisablePriorityBoost) == 0)

//
// Define priority boost flags.
//

#define DisablePriorityBoost 0x08

LARGE_INTEGER ExShortTime = {(ULONG)(-10 * 1000 * 10), -1}; // 10 milliseconds

#define EX_RESOURCE_CHECK_FREES   0x1
#define EX_RESOURCE_CHECK_ORPHANS 0x2

ULONG ExResourceCheckFlags = EX_RESOURCE_CHECK_FREES|EX_RESOURCE_CHECK_ORPHANS;


//
// Define resource assertion macro.
//

#if DBG

VOID
ExpAssertResource(
    IN PERESOURCE Resource
    );

#define ASSERT_RESOURCE(_Resource) ExpAssertResource(_Resource)

#else

#define ASSERT_RESOURCE(_Resource)

#endif

//
// Define locking primitives. 
// On UP systems, fastlocks are used.
// On MP systems, a queued spinlock is used.
//

#if defined(NT_UP)
#define EXP_LOCK_HANDLE KIRQL
#define PEXP_LOCK_HANDLE PKIRQL
#define EXP_LOCK_RESOURCE(_resource_, _plockhandle_)  UNREFERENCED_PARAMETER(_plockhandle_); ExAcquireFastLock(&(_resource_)->SpinLock, (_plockhandle_))
#define EXP_UNLOCK_RESOURCE(_resource_, _plockhandle_) ExReleaseFastLock(&(_resource_)->SpinLock, *(_plockhandle_))
#define EXP_LOCK_RESOURCE_RAISE   EXP_LOCK_RESOURCE
#define EXP_UNLOCK_RESOURCE_RAISE EXP_UNLOCK_RESOURCE
#else

#define EXP_LOCK_HANDLE KLOCK_QUEUE_HANDLE
#define PEXP_LOCK_HANDLE PKLOCK_QUEUE_HANDLE

#if defined(_AMD64_)

FORCEINLINE
VOID
EXP_LOCK_RESOURCE (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )
{

#if DBG
    LockHandle->OldIrql = DISPATCH_LEVEL;
#else
    UNREFERENCED_PARAMETER(LockHandle);
#endif

    do {
        _disable();
        if (InterlockedBitTestAndSet64((LONG64 *)&Resource->SpinLock,0) == FALSE) {
            PrefetchForWrite(&Resource->ActiveCount);
            break;
        }
        _enable();
        while (*(LONG64 volatile *)&Resource->SpinLock != 0) {
            KeYieldProcessor();
        }
    } while (1);
}

FORCEINLINE
EXP_UNLOCK_RESOURCE (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )
{
    UNREFERENCED_PARAMETER(LockHandle);

    InterlockedAnd64((LONG64 *)&Resource->SpinLock,0);
    _enable();
}

FORCEINLINE
VOID
EXP_LOCK_RESOURCE_RAISE (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )
{
    LockHandle->OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    while (InterlockedBitTestAndSet64((LONG64 *)&Resource->SpinLock,0) != FALSE) {
        while (*(LONG64 volatile *)&Resource->SpinLock != 0) {
            KeYieldProcessor();
        }
    }
}

FORCEINLINE
EXP_UNLOCK_RESOURCE_RAISE (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )
{
    InterlockedAnd64((LONG64 *)&Resource->SpinLock,0);
    KeLowerIrql(LockHandle->OldIrql);
}

#else

#define EXP_LOCK_RESOURCE(_resource_, _plockhandle_) KeAcquireInStackQueuedSpinLock(&(_resource_)->SpinLock, (_plockhandle_))
#define EXP_UNLOCK_RESOURCE(_resource_, _plockhandle_) KeReleaseInStackQueuedSpinLock(_plockhandle_)

#define EXP_LOCK_RESOURCE_RAISE   EXP_LOCK_RESOURCE
#define EXP_UNLOCK_RESOURCE_RAISE EXP_UNLOCK_RESOURCE

#endif
#endif

//
// Define private function prototypes.
//

VOID
FASTCALL
ExpWaitForResource (
    IN PERESOURCE Resource,
    IN PVOID Object
    );

VOID
ExpExpandResourceOwnerTable (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    );

POWNER_ENTRY
FASTCALL
ExpFindCurrentThread(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD CurrentThread,
    IN PEXP_LOCK_HANDLE LockHandle OPTIONAL
    );

POWNER_ENTRY
FASTCALL
ExpFindEmptyEntry(
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle OPTIONAL
    );



//
// Resource wait time out value.
//

LARGE_INTEGER ExpTimeout;

//
// Consecutive time outs before message.  Note this is registry-settable.
//

ULONG ExResourceTimeoutCount = 648000;

//
// Global spinlock to guard access to resource lists.
//

extern ALIGNED_SPINLOCK ExpResourceSpinLock;

//
// Resource list used to record all resources in the system.
//

LIST_ENTRY ExpSystemResourcesList;

#define ExpIncrementCounter(Member)



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpResourceInitialization)
#pragma alloc_text(PAGELK, ExQuerySystemLockInformation)
#endif

//
// Resource strict verification (checked builds only)
//
// When acquiring a resource while running in a thread that is not a system
// thread and runs at passive level we need to disable kernel APCs first
// (KeEnterCriticalRegion()). Otherwise any user mode code can call
// NtSuspendThread() which is implemented using kernel APCs and can
// suspend the thread while having a resource acquired.
// This will potentially deadlock the whole system.
//

#if DBG

ULONG ExResourceStrict = 1;

VOID
ExCheckIfKernelApcsShouldBeDisabled (
    IN KIRQL Irql,
    IN PVOID Resource,
    IN PKTHREAD Thread)
{
    if ((ExResourceStrict == 0) ||
        (Irql >= APC_LEVEL) ||
        (IS_SYSTEM_THREAD((PETHREAD)Thread)) ||
        (Thread->CombinedApcDisable != 0)) {

        return;
    }

    DbgPrint ("EX: resource: APCs still enabled before resource %p acquire !!!\n", Resource);
    DbgBreakPoint ();
}

#define EX_ENSURE_APCS_DISABLED(Irql, Resource, Thread) \
            ExCheckIfKernelApcsShouldBeDisabled (Irql, Resource, Thread);

#else

#define EX_ENSURE_APCS_DISABLED(Irql, Resource, Thread)

#endif // DBG


BOOLEAN
ExpResourceInitialization(
    VOID
    )

/*++

Routine Description:

    This function initializes global data during system initialization.

Arguments:

    None.

Return Value:

    BOOLEAN - TRUE

--*/

{
    //
    // Initialize resource timeout value, the system resource listhead,
    // and the resource spinlock.
    //

    ExpTimeout.QuadPart = Int32x32To64(4 * 1000, -10000);
    InitializeListHead(&ExpSystemResourcesList);
    KeInitializeSpinLock(&ExpResourceSpinLock);

    return TRUE;
}

VOID
ExpAllocateExclusiveWaiterEvent (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

/*++

Routine Description:

    This function allocates and initializes the exclusive waiter event
    for a resource.

    N.B. The resource spin lock is held on entry and exit of this routine.

Arguments:

    Resource - Supplies a pointer to the resource.

    LockHandle - Supplies a pointer to a lock handle.

Return Value:

    None.

--*/

{

    PKEVENT Event;

    //
    // Allocate an exclusive wait event and retry the acquire operation.
    //

    EXP_UNLOCK_RESOURCE(Resource, LockHandle);
    do {
        Event = ExAllocatePoolWithTag(NonPagedPool,
                                      sizeof(KEVENT),
                                      'vEeR');

        if (Event != NULL) {
            KeInitializeEvent(Event, SynchronizationEvent, FALSE);
            if (InterlockedCompareExchangePointer(&Resource->ExclusiveWaiters,
                                                  Event,
                                                  NULL) != NULL) {

                ExFreePool(Event);
            }

            break;
        }

        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } while (TRUE);

    EXP_LOCK_RESOURCE(Resource, LockHandle);
    return;
}

VOID
ExpAllocateSharedWaiterSemaphore (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

/*++

Routine Description:

    This function allocates and initializes the shared waiter semaphore
    for a resource.

    N.B. The resource spin lock is held on entry and exit of this routine.

Arguments:

    Resource - Supplies a pointer to the resource.

    LockHandle - Supplies a pointer to a lock handle.

Return Value:

    None.

--*/

{

    PKSEMAPHORE Semaphore;

    //
    // Allocate and initialize a shared wait semaphore for the specified
    // resource.
    //

    EXP_UNLOCK_RESOURCE(Resource, LockHandle);
    do {
        Semaphore = ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(KSEMAPHORE),
                                          'eSeR');

        if (Semaphore != NULL) {
            KeInitializeSemaphore(Semaphore, 0, MAXLONG);
            if (InterlockedCompareExchangePointer(&Resource->SharedWaiters,
                                                  Semaphore,
                                                  NULL) != NULL) {
                ExFreePool(Semaphore);
            }

            break;
        }

        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } while (TRUE);

    EXP_LOCK_RESOURCE(Resource, LockHandle);
    return;
}

NTSTATUS
ExInitializeResourceLite(
    __out PERESOURCE Resource
    )

/*++

Routine Description:

    This routine initializes the specified resource.

Arguments:

    Resource - Supplies a pointer to the resource to initialize.

Return Value:

    STATUS_SUCCESS.

--*/

{
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT(MmDeterminePoolType(Resource) == NonPagedPool);

    //
    // Initialize the specified resource.
    //
    // N.B. All fields are initialized to zero (NULL pointers) except
    //      the list entry and spinlock.
    //

    RtlZeroMemory(Resource, sizeof(ERESOURCE));
    KeInitializeSpinLock(&Resource->SpinLock);

    if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {
        Resource->CreatorBackTraceIndex = RtlLogStackBackTrace();
    }else {
        Resource->CreatorBackTraceIndex = 0;
    }

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    InsertTailList (&ExpSystemResourcesList, &Resource->SystemResourcesList);

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return STATUS_SUCCESS;
}

NTSTATUS
ExReinitializeResourceLite(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine reinitializes the specified resource.

Arguments:

    Resource - Supplies a pointer to the resource to initialize.

Return Value:

    STATUS_SUCCESS.

--*/

{

    PKEVENT Event;
    ULONG Index;
    POWNER_ENTRY OwnerTable;
    PKSEMAPHORE Semaphore;
    ULONG TableSize;

    ASSERT(MmDeterminePoolType(Resource) == NonPagedPool);

    //
    // If the resource has an owner table, then zero the owner table.
    //

    OwnerTable = Resource->OwnerTable;
    if (OwnerTable != NULL) {
        TableSize = OwnerTable->TableSize;
        for (Index = 1; Index < TableSize; Index += 1) {
            OwnerTable[Index].OwnerThread = 0;
            OwnerTable[Index].OwnerCount = 0;
        }
    }

    //
    // Set the active count and flags to zero.
    //

    Resource->ActiveCount = 0;
    Resource->Flag = 0;

    //
    // If the resource has a shared waiter semaphore, then reinitialize
    // it.
    //

    Semaphore = Resource->SharedWaiters;
    if (Semaphore != NULL) {
        KeInitializeSemaphore(Semaphore, 0, MAXLONG);
    }

    //
    // If the resource has a exclusive waiter event, then reinitialize
    // it.
    //

    Event = Resource->ExclusiveWaiters;
    if (Event != NULL) {
        KeInitializeEvent(Event, SynchronizationEvent, FALSE);
    }

    //
    // Initialize the builtin owner table.
    //

    Resource->OwnerThreads[0].OwnerThread = 0;
    Resource->OwnerThreads[0].OwnerCount = 0;
    Resource->OwnerThreads[1].OwnerThread = 0;
    Resource->OwnerThreads[1].OwnerCount = 0;

    //
    // Set the contention count, number of shared waiters, and number
    // of exclusive waiters to zero.
    //

    Resource->ContentionCount = 0;
    Resource->NumberOfSharedWaiters = 0;
    Resource->NumberOfExclusiveWaiters = 0;

    return STATUS_SUCCESS;
}

VOID
ExDisableResourceBoostLite(
    __in PERESOURCE Resource
    )

/*++

Routine Description:

    This routine disables priority inversion boosting for the specified
    resource.

Arguments:

    Resource - Supplies a pointer to the resource for which priority
        boosting is disabled.

Return Value:

    None.

--*/

{

    EXP_LOCK_HANDLE LockHandle;

    ASSERT_RESOURCE(Resource);

    //
    // Disable priority boosts for the specified resource.
    //

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    Resource->Flag |= DisablePriorityBoost;

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

BOOLEAN
ExAcquireResourceExclusiveLite(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    The routine acquires the specified resource for exclusive access.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for exclusive access.

    Wait - A boolean value that specifies whether to wait for the
        resource to become available if access cannot be granted
        immediately.

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    BOOLEAN Result;

    ASSERT((Resource->Flag & ResourceNeverExclusive) == 0);

    //
    // Acquire exclusive access to the specified resource.
    //

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    //
    // Resource acquisition must be protected from thread suspends.
    //

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    ExpIncrementCounter(ExclusiveAcquire);

    //
    // If the active count of the resource is zero, then there is neither
    // an exclusive owner nor a shared owner and access to the resource can
    // be immediately granted. Otherwise, there is either a shared owner or
    // an exclusive owner.
    //

retry:
    if (Resource->ActiveCount != 0) {

        //
        // The resource is either owned exclusive or shared.
        //
        // If the resource is owned exclusive and the current thread is the
        // owner, then increment the recursion count.
        //

        if (IsOwnedExclusive(Resource) &&
            (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            Result = TRUE;

        } else {

            //
            // The resource is either owned exclusive by some other thread,
            // or owned shared.
            //
            // If wait is not specified, then return that the resource was
            // not acquired. Otherwise, wait for exclusive access to the
            // resource to be granted.
            //

            if (Wait == FALSE) {
                Result = FALSE;

            } else {

                //
                // If the exclusive wait event has not yet been allocated,
                // then the long path code must be taken.
                //

                if (Resource->ExclusiveWaiters == NULL) {
                    ExpAllocateExclusiveWaiterEvent(Resource, &LockHandle);
                    goto retry;
                }

                //
                // Wait for exclusive access to the resource to be granted
                // and set the owner thread.
                //

                Resource->NumberOfExclusiveWaiters += 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                ExpWaitForResource(Resource, Resource->ExclusiveWaiters);

                //
                // N.B. It is "safe" to store the owner thread without
                //      obtaining any locks since the thread has already
                //      been granted exclusive ownership.
                //

                Resource->OwnerThreads[0].OwnerThread = (ERESOURCE_THREAD)PsGetCurrentThread();
                return TRUE;
            }
        }

    } else {

        //
        // The resource is not owned.
        //

        Resource->Flag |= ResourceOwnedExclusive;
        Resource->OwnerThreads[0].OwnerThread = CurrentThread;
        Resource->OwnerThreads[0].OwnerCount = 1;
        Resource->ActiveCount = 1;
        Result = TRUE;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return Result;
}

PVOID
ExEnterCriticalRegionAndAcquireResourceExclusive(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    The routine enters a critical region and acquires the specified resource
    for exclusive access.

    N.B. This is a win32k accelerator routine.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired for
        exclusive access.

Return Value:

    The address of the current win32 thread is returned as the function value.

--*/

{
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(Resource, TRUE);
    return _PsGetCurrentThread()->Tcb.Win32Thread;
}

BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    The routine attempts to acquire the specified resource for exclusive
    access.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for exclusive access.

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    BOOLEAN Result;

    ASSERT((Resource->Flag & ResourceNeverExclusive) == 0);

    //
    // Attempt to acquire exclusive access to the specified resource.
    //

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    //
    // If the active count of the resource is zero, then there is neither
    // an exclusive owner nor a shared owner and access to the resource can
    // be immediately granted. Otherwise, if the resource is owned exclusive
    // and the current thread is the owner, then access to the resource can
    // be immediately granted. Otherwise, access cannot be granted.
    //

    Result = FALSE;
    if (Resource->ActiveCount == 0) {
        ExpIncrementCounter(ExclusiveAcquire);
        Resource->Flag |= ResourceOwnedExclusive;
        Resource->OwnerThreads[0].OwnerThread = CurrentThread;
        Resource->OwnerThreads[0].OwnerCount = 1;
        Resource->ActiveCount = 1;
        Result = TRUE;

    } else if (IsOwnedExclusive(Resource) &&
        (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
        ExpIncrementCounter(ExclusiveAcquire);
        Resource->OwnerThreads[0].OwnerCount += 1;
        Result = TRUE;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return Result;
}

BOOLEAN
ExAcquireResourceSharedLite(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    The routine acquires the specified resource for shared access.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for shared access.

    Wait - A boolean value that specifies whether to wait for the
        resource to become available if access cannot be granted
        immediately.

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

    //
    // Acquire exclusive access to the specified resource.
    //

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    //
    // Resource acquisition must be protected from thread suspends.
    //

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    ExpIncrementCounter(SharedFirstLevel);

    //
    // If the active count of the resource is zero, then there is neither
    // an exclusive owner nor a shared owner and access to the resource can
    // be immediately granted.
    //

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

    //
    // The resource is either owned exclusive or shared.
    //
    // If the resource is owned exclusive and the current thread is the
    // owner, then treat the shared request as an exclusive request and
    // increment the recursion count. Otherwise, it is owned shared.
    //

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

        //
        // Find an empty entry in the thread array.
        //

        OwnerEntry = ExpFindEmptyEntry(Resource, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

        //
        // The resource is owned shared.
        //
        // If the current thread already has acquired the resource for
        // shared access, then increment the recursion count. Otherwise
        // grant shared access if there are no exclusive waiters.
        //

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

        if (OwnerEntry->OwnerThread == CurrentThread) {
            OwnerEntry->OwnerCount += 1;

            ASSERT(OwnerEntry->OwnerCount != 0);

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

        //
        // If there are no exclusive waiters, then grant shared access
        // to the resource. Otherwise, wait for the resource to become
        // available.
        //

        if (IsExclusiveWaiting(Resource) == FALSE) {
            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            Resource->ActiveCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }
    }

    //
    // The resource is either owned exclusive by some other thread, or
    // owned shared by some other threads, but there is an exclusive
    // waiter and the current thread does not already have shared access
    // to the resource.
    //
    // If wait is not specified, then return that the resource was
    // not acquired.
    //

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

    //
    // If the shared wait semaphore has not yet been allocated, then the
    // long path must be taken.
    //

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

    //
    // Wait for shared access to the resource to be granted and increment
    // the recursion count.
    //

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

PVOID
ExEnterCriticalRegionAndAcquireResourceShared(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine enters a critical region and acquires the specified resource
    for shared access.

     N.B. This is a win32k accelerator routine.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired for shared
        access.

Return Value:

    The address of the current win32 thread is returned as the function value.

--*/

{

    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(Resource, TRUE);
    return _PsGetCurrentThread()->Tcb.Win32Thread;
}

BOOLEAN
ExAcquireSharedStarveExclusive(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    )
/*++

Routine Description:

    This routine acquires the specified resource for shared access and
    does not wait for any pending exclusive owners.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for shared access.

    Wait - A boolean value that specifies whether to wait for the
        resource to become available if access cannot be granted
        immediately.

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

    //
    // Acquire exclusive access to the specified resource.
    //

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    ExpIncrementCounter(StarveFirstLevel);

    //
    // If the active count of the resource is zero, then there is neither
    // an exclusive owner nor a shared owner and access to the resource can
    // be immediately granted.
    //

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

    //
    // The resource is either owned exclusive or shared.
    //
    // If the resource is owned exclusive and the current thread is the
    // owner, then treat the shared request as an exclusive request and
    // increment the recursion count. Otherwise, it is owned shared.
    //

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

        //
        // Find an empty entry in the thread array.
        //

        OwnerEntry = ExpFindEmptyEntry(Resource, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

        //
        // The resource is owned shared.
        //
        // If the current thread already has acquired the resource for
        // shared access, then increment the recursion count. Otherwise
        // grant shared access to the current thread.
        //

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

        if (OwnerEntry->OwnerThread == CurrentThread) {
            OwnerEntry->OwnerCount += 1;

            ASSERT(OwnerEntry->OwnerCount != 0);

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

        //
        // Grant the current thread shared access to the resource.
        //

        OwnerEntry->OwnerThread = CurrentThread;
        OwnerEntry->OwnerCount = 1;
        Resource->ActiveCount += 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

    //
    // The resource is owned exclusive by some other thread.
    //
    // If wait is not specified, then return that the resource was
    // not acquired.
    //

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

    //
    // If the shared wait semaphore has not yet been allocated, then the
    // long path must be taken.
    //

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

    //
    // Wait for shared access to the resource to be granted and increment
    // the recursion count.
    //

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

BOOLEAN
ExAcquireSharedWaitForExclusive(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    )

/*++

Routine Description:

    This routine acquires the specified resource for shared access, but
    waits for any pending exclusive owners.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for shared access.

    Wait - A boolean value that specifies whether to wait for the
        resource to become available if access cannot be granted
        immediately.

Return Value:

    BOOLEAN - TRUE if the resource is acquired and FALSE otherwise.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry;

    //
    // Acquire exclusive access to the specified resource.
    //

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    ExpIncrementCounter(WaitForExclusive);

    //
    // If the active count of the resource is zero, then there is neither
    // an exclusive owner nor a shared owner and access to the resource can
    // be immediately granted.
    //

retry:
    if (Resource->ActiveCount == 0) {
        Resource->OwnerThreads[1].OwnerThread = CurrentThread;
        Resource->OwnerThreads[1].OwnerCount = 1;
        Resource->ActiveCount = 1;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return TRUE;
    }

    //
    // The resource is either owned exclusive or shared.
    //
    // If the resource is owned exclusive and the current thread is the
    // owner, then treat the shared request as an exclusive request and
    // increment the recursion count. Otherwise, it is owned shared.
    //

    if (IsOwnedExclusive(Resource)) {
        if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            Resource->OwnerThreads[0].OwnerCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }

        //
        // Find an empty entry in the thread array.
        //

        OwnerEntry = ExpFindEmptyEntry(Resource, &LockHandle);
        if (OwnerEntry == NULL) {
            goto retry;
        }

    } else {

        //
        // The resource is owned shared.
        //
        // If there is an exclusive waiter, then wait for the exclusive
        // waiter to gain access to the resource, then acquire the resource
        // shared without regard to exclusive waiters. Otherwise, if the
        // current thread already has acquired the resource for shared access,
        // then increment the recursion count. Otherwise grant shared access
        // to the current thread.
        //

        if (IsExclusiveWaiting(Resource)) {

            //
            // The resource is shared, but there is an exclusive waiter.
            //
            // It doesn't matter if this thread is already one of the shared
            // owner(s) - if TRUE is specified, this thread must block - an APC
            // will release the resource to unjam things and callers count on
            // this behavior.
            //

            //
            // If wait is not specified, then return that the resource was
            // not acquired.
            //

            if (Wait == FALSE) {
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                return FALSE;
            }

            //
            // If the shared wait semaphore has not yet been allocated, then
            // allocate and initialize it.
            //

            if (Resource->SharedWaiters == NULL) {
                ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
                goto retry;
            }

            //
            // Increment the number of shared waiters and wait for shared
            // access to the resource to be granted to some other set of
            // threads, and then acquire the resource shared without regard
            // to exclusive access.
            //
            // N.B. The resource is left in a state such that the calling
            //      thread does not have a reference in the owner table
            //      for the requested access even though the active count
            //      is incremented when control is returned. However, the
            //      resource is owned shared at this point, so an owner
            //      entry can simply be allocated and the owner count set
            //      to one.
            //

            Resource->NumberOfSharedWaiters += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            ExpWaitForResource(Resource, Resource->SharedWaiters);

            //
            // Reacquire the resource spin lock, allocate an owner entry,
            // and initialize the owner count to one. The active count
            // was already incremented when shared access was granted.
            //

            EXP_LOCK_RESOURCE(Resource, &LockHandle);
            do {
            } while ((OwnerEntry = ExpFindCurrentThread(Resource,
                                                        CurrentThread,
                                                        &LockHandle)) == NULL);

            ASSERT(IsOwnedExclusive(Resource) == FALSE);
            ASSERT(Resource->ActiveCount > 0);
            ASSERT(OwnerEntry->OwnerThread != CurrentThread);

            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;

        } else {
            OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, &LockHandle);
            if (OwnerEntry == NULL) {
                goto retry;
            }

            if (OwnerEntry->OwnerThread == CurrentThread) {
                OwnerEntry->OwnerCount += 1;

                ASSERT(OwnerEntry->OwnerCount != 0);

                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                return TRUE;
            }

            //
            // Grant the current thread shared access to the resource.
            //

            OwnerEntry->OwnerThread = CurrentThread;
            OwnerEntry->OwnerCount = 1;
            Resource->ActiveCount += 1;
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return TRUE;
        }
    }

    //
    // The resource is owned exclusive by some other thread.
    //
    // If wait is not specified, then return that the resource was
    // not acquired.
    //

    if (Wait == FALSE) {
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        return FALSE;
    }

    //
    // If the shared wait semaphore has not yet been allocated, then allocate
    // and initialize it.
    //

    if (Resource->SharedWaiters == NULL) {
        ExpAllocateSharedWaiterSemaphore(Resource, &LockHandle);
        goto retry;
    }

    //
    // Wait for shared access to the resource to be granted and increment
    // the recursion count.
    //

    OwnerEntry->OwnerThread = CurrentThread;
    OwnerEntry->OwnerCount = 1;
    Resource->NumberOfSharedWaiters += 1;
    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    ExpWaitForResource(Resource, Resource->SharedWaiters);
    return TRUE;
}

PVOID
ExEnterCriticalRegionAndAcquireSharedWaitForExclusive(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine acquires the specified resource for shared access, but
    waits for any pending exclusive owners.

    N.B. This is a win32k accelerator routine.

Arguments:

    Resource - Supplies a pointer to the resource that is acquired
        for shared access.

    Wait - A boolean value that specifies whether to wait for the
        resource to become available if access cannot be granted
        immediately.

Return Value:

    The address of the current win32 thread is returned as the function value..

--*/

{

    KeEnterCriticalRegion();
    ExAcquireSharedWaitForExclusive(Resource, TRUE);
    return _PsGetCurrentThread()->Tcb.Win32Thread;
}

VOID
FASTCALL
ExReleaseResourceLite(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine releases the specified resource for the current thread
    and decrements the recursion count. If the count reaches zero, then
    the resource may also be released.

Arguments:

    Resource - Supplies a pointer to the resource to release.

Return Value:

    None.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    ULONG Index;
    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, OwnerEnd;

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

    //
    // Acquire exclusive access to the specified resource.
    //

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    //
    // Resource release must be protected from thread suspends.
    //

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    //
    // If the resource is exclusively owned, then release exclusive
    // ownership. Otherwise, release shared ownership.
    //
    // N.B. The two release paths are split since this is such a high
    //      frequency function.
    //

    if (IsOwnedExclusive(Resource)) {

#if DBG
        //
        // This can only be enabled in checked builds because this (unusual)
        // behavior might have worked in earlier releases of NT.  However,
        // in the checked builds, this can be enabled because callers really
        // should convert to using ExReleaseResourceForThreadLite instead.
        //

        if (Resource->OwnerThreads[0].OwnerThread != CurrentThread) {
            KeBugCheckEx(RESOURCE_NOT_OWNED,
                         (ULONG_PTR)Resource,
                         (ULONG_PTR)CurrentThread,
                         (ULONG_PTR)Resource->OwnerTable,
                         0x1);
        }
#endif

        //
        // Decrement the recursion count and check if ownership can be
        // released.
        //

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        if (--Resource->OwnerThreads[0].OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

        //
        // Clear the owner thread.
        //

        Resource->OwnerThreads[0].OwnerThread = 0;

        //
        // The thread recursion count reached zero so decrement the resource
        // active count. If the active count reaches zero, then the resource
        // is no longer owned and an attempt should be made to grant access to
        // another thread.
        //

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

            //
            // If there are shared waiters, then grant shared access to the
            // resource. Otherwise, grant exclusive ownership if there are
            // exclusive waiters.
            //

            if (IsSharedWaiting(Resource)) {
                Resource->Flag &= ~ResourceOwnedExclusive;
                Number = Resource->NumberOfSharedWaiters;
                Resource->ActiveCount =  (SHORT)Number;
                Resource->NumberOfSharedWaiters = 0;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
                return;

            } else if (IsExclusiveWaiting(Resource)) {
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);
                return;
            }

            Resource->Flag &= ~ResourceOwnedExclusive;
        }

    } else {
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[1];

        } else if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[0];

        } else {
            Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;
            OwnerEntry = Resource->OwnerTable;

            if (OwnerEntry == NULL) {
                KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x2);
            }

            //
            // If the resource hint is not within range or the resource
            // table entry does match the current thread, then search
            // the owner table for a match.
            //

            if ((Index >= OwnerEntry->TableSize) ||
                (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                OwnerEnd = &OwnerEntry[OwnerEntry->TableSize];
                while (1) {
                    OwnerEntry += 1;
                    if (OwnerEntry >= OwnerEnd) {
                       KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x3);
                    }
                    if (OwnerEntry->OwnerThread == CurrentThread) {
                        break;
                    }
                }

            } else {
                OwnerEntry = &OwnerEntry[Index];
            }
        }

        //
        // Decrement the recursion count and check if ownership can be
        // released.
        //

        ASSERT(OwnerEntry->OwnerThread == CurrentThread);
        ASSERT(OwnerEntry->OwnerCount > 0);

        if (--OwnerEntry->OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

        //
        // Clear the owner thread.
        //

        OwnerEntry->OwnerThread = 0;

        //
        // The thread recursion count reached zero so decrement the resource
        // active count. If the active count reaches zero, then the resource
        // is no longer owned and an attempt should be made to grant access to
        // another thread.
        //

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

            //
            // If there are exclusive waiters, then grant exclusive access
            // to the resource.
            //

            if (IsExclusiveWaiting(Resource)) {
                Resource->Flag |= ResourceOwnedExclusive;
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }
        }
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
FASTCALL
ExReleaseResourceAndLeaveCriticalRegion(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine releases the specified resource for the current thread,
    decrements the recursion count, and leave a critical region.. If the
    count reaches zero, then the resource may also be released.

Arguments:

    Resource - Supplies a pointer to the resource to release.

Return Value:

    None.

--*/

{
    ExReleaseResourceLite(Resource);
    KeLeaveCriticalRegion();
    return;
}

VOID
ExReleaseResourceForThreadLite(
    __inout PERESOURCE Resource,
    __in ERESOURCE_THREAD CurrentThread
    )

/*++

Routine Description:

    This routine release the specified resource for the specified thread
    and decrements the recursion count. If the count reaches zero, then
    the resource may also be released.

Arguments:

    Resource - Supplies a pointer to the resource to release.

    Thread - Supplies the thread that originally acquired the resource.

Return Value:

    None.

--*/

{

    ULONG Index;
    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, OwnerEnd;

    ASSERT(CurrentThread != 0);

    ASSERT_RESOURCE(Resource);

    //
    // Acquire exclusive access to the specified resource.
    //

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    //
    // Resource release must be protected from thread suspends.
    //

    EX_ENSURE_APCS_DISABLED (LockHandle.OldIrql,
                             Resource,
                             KeGetCurrentThread());

    //
    // If the resource is exclusively owned, then release exclusive
    // ownership. Otherwise, release shared ownership.
    //
    // N.B. The two release paths are split since this is such a high
    //      frequency function.
    //

    if (IsOwnedExclusive(Resource)) {

        ASSERT(Resource->OwnerThreads[0].OwnerThread == CurrentThread);

        //
        // Decrement the recursion count and check if ownership can be
        // released.
        //

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        if (--Resource->OwnerThreads[0].OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

        //
        // Clear the owner thread.
        //

        Resource->OwnerThreads[0].OwnerThread = 0;

        //
        // The thread recursion count reached zero so decrement the resource
        // active count. If the active count reaches zero, then the resource
        // is no longer owned and an attempt should be made to grant access to
        // another thread.
        //

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

            //
            // If there are shared waiters, then grant shared access to the
            // resource. Otherwise, grant exclusive ownership if there are
            // exclusive waiters.
            //

            if (IsSharedWaiting(Resource)) {
                Resource->Flag &= ~ResourceOwnedExclusive;
                Number = Resource->NumberOfSharedWaiters;
                Resource->ActiveCount =  (SHORT)Number;
                Resource->NumberOfSharedWaiters = 0;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
                return;

            } else if (IsExclusiveWaiting(Resource)) {
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }

            Resource->Flag &= ~ResourceOwnedExclusive;
        }

    } else {
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[1];

        } else if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
            OwnerEntry = &Resource->OwnerThreads[0];

        } else {

            //
            // If the specified current thread is an owner address (low
            // bits are nonzero), then set the hint index to the first
            // entry. Otherwise, set the hint index from the owner thread.
            //

            Index = 1;
            if (((ULONG)CurrentThread & 3) == 0) {
                Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;
            }

            OwnerEntry = Resource->OwnerTable;

            ASSERT(OwnerEntry != NULL);

            //
            // If the resource hint is not within range or the resource
            // table entry does match the current thread, then search
            // the owner table for a match.
            //

            if ((Index >= OwnerEntry->TableSize) ||
                (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                OwnerEnd = &OwnerEntry[OwnerEntry->TableSize];
                while (1) {
                    OwnerEntry += 1;
                    if (OwnerEntry >= OwnerEnd) {
                       KeBugCheckEx(RESOURCE_NOT_OWNED,
                             (ULONG_PTR)Resource,
                             (ULONG_PTR)CurrentThread,
                             (ULONG_PTR)Resource->OwnerTable,
                             0x3);
                    }
                    if (OwnerEntry->OwnerThread == CurrentThread) {
                        break;
                    }
                }

            } else {
                OwnerEntry = &OwnerEntry[Index];
            }
        }

        //
        // Decrement the recursion count and check if ownership can be
        // released.
        //

        ASSERT(OwnerEntry->OwnerThread == CurrentThread);
        ASSERT(OwnerEntry->OwnerCount > 0);

        if (--OwnerEntry->OwnerCount != 0) {
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
            return;
        }

        //
        // Clear the owner thread.
        //

        OwnerEntry->OwnerThread = 0;

        //
        // The thread recursion count reached zero so decrement the resource
        // active count. If the active count reaches zero, then the resource
        // is no longer owned and an attempt should be made to grant access to
        // another thread.
        //

        ASSERT(Resource->ActiveCount > 0);

        if (--Resource->ActiveCount == 0) {

            //
            // If there are exclusive waiters, then grant exclusive access
            // to the resource.
            //

            if (IsExclusiveWaiting(Resource)) {
                Resource->Flag |= ResourceOwnedExclusive;
                Resource->OwnerThreads[0].OwnerThread = 1;
                Resource->OwnerThreads[0].OwnerCount = 1;
                Resource->ActiveCount = 1;
                Resource->NumberOfExclusiveWaiters -= 1;
                EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
                KeSetEventBoostPriority(Resource->ExclusiveWaiters,
                                        (PRKTHREAD *)&Resource->OwnerThreads[0].OwnerThread);

                return;
            }
        }
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
ExSetResourceOwnerPointer(
    __inout PERESOURCE Resource,
    __in PVOID OwnerPointer
    )

/*++

Routine Description:

    This routine locates the owner entry for the current thread and stores
    the specified owner address as the owner thread. Subsequent to calling
    this routine, the only routine which may be called for this resource is
    ExReleaseResourceForThreadLite, supplying the owner address as the "thread".

    Owner addresses must obey the following rules:

        They must be a unique pointer to a structure allocated in system space,
        and they must point to a structure which remains allocated until after
        the call to ExReleaseResourceForThreadLite. This is to eliminate aliasing
        with a thread or other owner address.

        The low order two bits of the owner address must be set by the caller,
        so that other routines in the resource package can distinguish owner
        address from thread addresses.

Arguments:

    Resource - Supplies a pointer to the resource to release.

    OwnerPointer - Supplies a pointer to an allocated structure with the low
        order two bits set.

Return Value:

    None.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    EXP_LOCK_HANDLE LockHandle;
    POWNER_ENTRY OwnerEntry, ExistingOwnerEntry;

    ASSERT((OwnerPointer != 0) && (((ULONG_PTR)OwnerPointer & 3) == 3));

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

    //
    // Acquire exclusive access to the specified resource.
    //

    EXP_LOCK_RESOURCE(Resource, &LockHandle);


    //
    // If the resource is exclusively owned, then it is the first owner entry.
    //

    if (IsOwnedExclusive(Resource)) {

        ASSERT(Resource->OwnerThreads[0].OwnerThread == CurrentThread);

        //
        // Set the owner address.
        //

        ASSERT(Resource->OwnerThreads[0].OwnerCount > 0);

        Resource->OwnerThreads[0].OwnerThread = (ULONG_PTR)OwnerPointer;

    //
    //  For shared access we have to search for the current thread to set
    //  the owner address.
    //

    } else {

        ExistingOwnerEntry = ExpFindCurrentThread(Resource, (ERESOURCE_THREAD)OwnerPointer, NULL);

        OwnerEntry = ExpFindCurrentThread(Resource, CurrentThread, NULL);

        if (OwnerEntry == NULL) {
            KeBugCheckEx(RESOURCE_NOT_OWNED,
                         (ULONG_PTR)Resource,
                         (ULONG_PTR)CurrentThread,
                         (ULONG_PTR)Resource->OwnerTable,
                         0x3);
        }

        if (ExistingOwnerEntry != NULL) {
            ExistingOwnerEntry->OwnerCount += OwnerEntry->OwnerCount;
            OwnerEntry->OwnerCount = 0;
            OwnerEntry->OwnerThread = 0;
            ASSERT (Resource->ActiveCount >= 2);
            Resource->ActiveCount -= 1;
        } else {
            OwnerEntry->OwnerThread = (ERESOURCE_THREAD)OwnerPointer;
        }

    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

VOID
ExConvertExclusiveToSharedLite(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine converts the specified resource from acquired for exclusive
    access to acquired for shared access.

Arguments:

    Resource - Supplies a pointer to the resource to acquire for shared access. it

Return Value:

    None.

--*/

{

    ULONG Number;
    EXP_LOCK_HANDLE LockHandle;

    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    ASSERT(IsOwnedExclusive(Resource));
    ASSERT(Resource->OwnerThreads[0].OwnerThread == (ERESOURCE_THREAD)PsGetCurrentThread());

    //
    // Acquire exclusive access to the specified resource.
    //

    EXP_LOCK_RESOURCE(Resource, &LockHandle);

    //
    // Convert the granted access from exclusive to shared.
    //

    Resource->Flag &= ~ResourceOwnedExclusive;

    //
    // If there are any shared waiters, then grant them shared access.
    //

    if (IsSharedWaiting(Resource)) {
        Number = Resource->NumberOfSharedWaiters;
        Resource->ActiveCount = (SHORT)(Resource->ActiveCount + Number);
        Resource->NumberOfSharedWaiters = 0;
        EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        KeReleaseSemaphore(Resource->SharedWaiters, 0, Number, FALSE);
        return;
    }

    EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
    return;
}

NTSTATUS
ExDeleteResourceLite(
    __inout PERESOURCE Resource
    )

/*++

Routine Description:

    This routine deallocates any pool allocated to support the specified
    resource.


Arguments:

    Resource - Supplies a pointer to the resource whose allocated pool
        is freed.

Return Value:

    STATUS_SUCCESS.

--*/

{
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT(IsSharedWaiting(Resource) == FALSE);
    ASSERT(IsExclusiveWaiting(Resource) == FALSE);
    ASSERT(KeIsExecutingDpc() == FALSE);
    ASSERT_RESOURCE(Resource);

    //
    // Acquire the executive resource spinlock and remove the resource from
    // the system resource list.
    //

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    RemoveEntryList(&Resource->SystemResourcesList);

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    //
    // If an owner table was allocated, then free it to pool.
    //

    if (Resource->OwnerTable != NULL) {
        ExFreePool(Resource->OwnerTable);
    }

    //
    // If a semaphore was allocated, then free it to pool.
    //

    if (Resource->SharedWaiters) {
        ExFreePool(Resource->SharedWaiters);
    }

    //
    // If an event was allocated, then free it to pool.
    //

    if (Resource->ExclusiveWaiters) {
        ExFreePool(Resource->ExclusiveWaiters);
    }

    return STATUS_SUCCESS;
}

ULONG
ExGetExclusiveWaiterCount(
    __in PERESOURCE Resource
    )

/*++

Routine Description:

    This routine returns the exclusive waiter count.


Arguments:

    Resource - Supplies a pointer to and executive resource.

Return Value:

    The current number of exclusive waiters is returned as the function
    value.

--*/

{
    return Resource->NumberOfExclusiveWaiters;
}

ULONG
ExGetSharedWaiterCount(
    __in PERESOURCE Resource
    )

/*++

Routine Description:

    This routine returns the shared waiter count.


Arguments:

    Resource - Supplies a pointer to and executive resource.

Return Value:

    The current number of shared waiters is returned as the function
    value.

--*/

{
    return Resource->NumberOfSharedWaiters;
}



BOOLEAN
ExIsResourceAcquiredExclusiveLite(
    __in PERESOURCE Resource
    )

/*++

Routine Description:

    This routine determines if a resource is acquired exclusive by the
    calling thread.

Arguments:

    Resource - Supplies a pointer the resource to query.

Return Value:

    If the current thread has acquired the resource exclusive, a value of
    TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    BOOLEAN Result;

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    ASSERT_RESOURCE(Resource);

    //
    // If the resource is owned exclusive and the current thread is the
    // owner, then set the return value of TRUE. Otherwise, set the return
    // value to FALSE.
    //

    Result = FALSE;
    if ((IsOwnedExclusive(Resource)) &&
        (Resource->OwnerThreads[0].OwnerThread == CurrentThread)) {
        Result = TRUE;
    }

    return Result;
}



ULONG
ExIsResourceAcquiredSharedLite(
    __in PERESOURCE Resource
    )

/*++

Routine Description:

    This routine determines if a resource is acquired either shared or
    exclusive by the calling thread.

Arguments:

    Resource - Supplies a pointer to the resource to query.

Return Value:

    If the current thread has not acquired the resource a value of zero
    is returned. Otherwise, the thread's acquire count is returned.

--*/

{

    ERESOURCE_THREAD CurrentThread;
    ULONG Index;
    ULONG Number;
    POWNER_ENTRY OwnerEntry;
    ULONG Result;
    EXP_LOCK_HANDLE LockHandle;

    ASSERT_RESOURCE(Resource);


    //
    // If nobody owns this resource then exit early.
    //
    if (Resource->ActiveCount == 0) {
        return 0;
    }

    CurrentThread = (ERESOURCE_THREAD)PsGetCurrentThread();

    //
    // Find the current thread in the thread array and return the count.
    //
    // N.B. If the thread is not found a value of zero will be returned.
    //


    if (Resource->OwnerThreads[0].OwnerThread == CurrentThread) {
        Result = Resource->OwnerThreads[0].OwnerCount;

    } else {
        //
        // If we are not in the first slot and the resource is owned exclusive
        // then we can't own it at all
        //
        if (IsOwnedExclusive(Resource)) {
            return 0;
        }

        //
        // Check slot 2 and other slots in the owner table
        //
        if (Resource->OwnerThreads[1].OwnerThread == CurrentThread) {
            Result = Resource->OwnerThreads[1].OwnerCount;
        } else {

            Result = 0;
            Index = ((PKTHREAD)(CurrentThread))->ResourceIndex;

            EXP_LOCK_RESOURCE(Resource, &LockHandle);

            //
            // If the resource hint is not within range or the resource table
            // entry does not match the current thread, then search the owner
            // table for a match.
            //

            OwnerEntry = Resource->OwnerTable;
            if (OwnerEntry != NULL) {
                Number = OwnerEntry->TableSize;
                if ((Index >= Number) ||
                    (OwnerEntry[Index].OwnerThread != CurrentThread)) {
                    for (Index = 1; Index < Number; Index += 1) {
                        OwnerEntry += 1;
                        if (OwnerEntry->OwnerThread == CurrentThread) {
                            Result = OwnerEntry->OwnerCount;
                            break;
                        }
                    }

                } else {
                    Result = OwnerEntry[Index].OwnerCount;
                }
            }

            //
            // Release exclusive access to the specified resource.
            //

            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
        }
    }

    return Result;
}

NTSTATUS
ExQuerySystemLockInformation(
    __out_bcount(LockInformationLength) PRTL_PROCESS_LOCKS LockInformation,
    __in ULONG LockInformationLength,
    __out_opt PULONG ReturnLength
    )

{

    NTSTATUS Status;
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG RequiredLength;
    PLIST_ENTRY Head, Next;
    PRTL_PROCESS_LOCK_INFORMATION LockInfo;
    PERESOURCE Resource;
    PETHREAD OwningThread;

    RequiredLength = FIELD_OFFSET(RTL_PROCESS_LOCKS, Locks);
    if (LockInformationLength < RequiredLength) {
        Status = STATUS_INFO_LENGTH_MISMATCH;

    } else {
        Status = STATUS_SUCCESS;

        LockInformation->NumberOfLocks = 0;
        LockInfo = &LockInformation->Locks[0];
        Head = &ExpSystemResourcesList;

        KeAcquireInStackQueuedSpinLock(&ExpResourceSpinLock, &LockHandle);

        Next = Head->Flink;
        while (Next != Head) {
            Resource = CONTAINING_RECORD(Next,
                                         ERESOURCE,
                                         SystemResourcesList);

            LockInformation->NumberOfLocks += 1;
            RequiredLength += sizeof(RTL_PROCESS_LOCK_INFORMATION);

            if (LockInformationLength < RequiredLength) {
                Status = STATUS_INFO_LENGTH_MISMATCH;

            } else {
                LockInfo->Address = Resource;
                LockInfo->Type = RTL_RESOURCE_TYPE;
                LockInfo->CreatorBackTraceIndex = 0;
#if i386 && !FPO
                LockInfo->CreatorBackTraceIndex = (USHORT)Resource->CreatorBackTraceIndex;
#endif // i386 && !FPO

                 if ((Resource->OwnerThreads[0].OwnerThread != 0) &&
                    ((Resource->OwnerThreads[0].OwnerThread & 3) == 0)) {
                    OwningThread = (PETHREAD)(Resource->OwnerThreads[0].OwnerThread);
                    LockInfo->OwningThread = OwningThread->Cid.UniqueThread;

                } else {
                    LockInfo->OwningThread = 0;
                }

                LockInfo->LockCount = Resource->ActiveCount;
                LockInfo->ContentionCount = Resource->ContentionCount;
                LockInfo->NumberOfWaitingShared = Resource->NumberOfSharedWaiters;
                LockInfo->NumberOfWaitingExclusive = Resource->NumberOfExclusiveWaiters;
                LockInfo += 1;
            }
            Next = Next->Flink;
        }

        KeReleaseInStackQueuedSpinLock(&LockHandle);
    }

    if (ARGUMENT_PRESENT(ReturnLength)) {
        *ReturnLength = RequiredLength;
    }

    return Status;
}

VOID
FASTCALL
ExpBoostOwnerThread (
    IN PKTHREAD CurrentThread,
    IN PKTHREAD OwnerThread
    )
/*++

Routine Description:

    This function boots the priority of the specified owner thread iff
    its priority is less than that of the current thread and is also
    less than fourteen.

    N.B. this function is called with the dispatcher database lock held.

Arguments:

    CurrentThread - Supplies a pointer to the current thread object.

    OwnerThread - Supplies a pointer to the owner thread object.

Return Value:

    None.

--*/

{

    //
    // If the owner thread is lower priority than the current thread, the
    // current thread is running at a priority less than 14, then boost the
    // priority of the owner thread for a quantum.
    //
    // N.B. A thread that has already been boosted may be reboosted to allow
    //      it to execute and release resources. When the boost is removed,
    //      the thread will return to its priority before any boosting.
    //

    if (((ULONG_PTR)OwnerThread & 0x3) == 0) {
        if ((OwnerThread->Priority < CurrentThread->Priority) &&
            (OwnerThread->Priority < 14)) {

            KiAcquireThreadLock(OwnerThread);
            OwnerThread->PriorityDecrement += 14 - OwnerThread->Priority;
            OwnerThread->Quantum = OwnerThread->QuantumReset;
            KiSetPriorityThread(OwnerThread, 14);
            KiReleaseThreadLock(OwnerThread);
        }
    }

    return;
}

VOID
FASTCALL
ExpWaitForResource (
    IN PERESOURCE Resource,
    IN PVOID Object
    )
/*++

Routine Description:

    The routine waits for the specified resource object to be set. If the
    wait is too long the priority of the current owners of the resource
    are boosted.

Arguments:

    Resource - Supplies a pointer to the resource to wait for.

    Object - Supplies a pointer to an event (exclusive) or semaphore
       (shared) to wait for.

Return Value:

    None.

--*/

{

    ULONG Index;
    ULONG Limit;
    ULONG Number;
    POWNER_ENTRY OwnerEntry;
    PKTHREAD OwnerThread;
    NTSTATUS Status;
    PKTHREAD CurrentThread;
    LARGE_INTEGER Timeout;
#if DBG
    EXP_LOCK_HANDLE LockHandle;
#endif

    //
    // Increment the contention count for the resource, set the initial
    // timeout value, and wait for the specified object to be signaled
    // or a timeout to occur.
    //

    Limit = 0;
    Resource->ContentionCount += 1;
    Timeout.QuadPart = 500 * -10000;
    do {
        Status = KeWaitForSingleObject (
                        Object,
                        WrResource,
                        KernelMode,
                        FALSE,
                        &Timeout );

        if (Status != STATUS_TIMEOUT) {
            break;
        }

        //
        // The limit has been exceeded, then output status information.
        //

        Limit += 1;
        Timeout = ExpTimeout;
        if (Limit > ExResourceTimeoutCount) {
            Limit = 0;

#if DBG
            //
            // Output information for the specified resource.
            //

            EXP_LOCK_RESOURCE(Resource, &LockHandle);
            DbgPrint("Resource @ %p\n", Resource);
            DbgPrint(" ActiveCount = %04lx  Flags = %s%s%s\n",
                     Resource->ActiveCount,
                     IsOwnedExclusive(Resource) ? "IsOwnedExclusive " : "",
                     IsSharedWaiting(Resource) ? "SharedWaiter "     : "",
                     IsExclusiveWaiting(Resource) ? "ExclusiveWaiter "  : "");

            DbgPrint(" NumberOfExclusiveWaiters = %04lx\n", Resource->NumberOfExclusiveWaiters);

            DbgPrint("  Thread = %p, Count = %02x\n",
                     Resource->OwnerThreads[0].OwnerThread,
                     Resource->OwnerThreads[0].OwnerCount);

            DbgPrint("  Thread = %p, Count = %02x\n",
                     Resource->OwnerThreads[1].OwnerThread,
                     Resource->OwnerThreads[1].OwnerCount);

            OwnerEntry = Resource->OwnerTable;
            if (OwnerEntry != NULL) {
                Number = OwnerEntry->TableSize;
                for(Index = 1; Index < Number; Index += 1) {
                    OwnerEntry += 1;
                    DbgPrint("  Thread = %p, Count = %02x\n",
                             OwnerEntry->OwnerThread,
                             OwnerEntry->OwnerCount);
                }
            }

            DbgBreakPoint();
            DbgPrint("EX - Rewaiting\n");
            EXP_UNLOCK_RESOURCE(Resource, &LockHandle);
#endif
        }

        //
        // If priority boosts are allowed, then attempt to boost the priority
        // of owner threads.
        //

        if (IsBoostAllowed(Resource)) {

            //
            // Get the current thread address, lock the dispatcher database,
            // and set wait next in the current thread so the dispatcher
            // database lock does not need to be released before waiting
            // for the resource.
            //
            // N.B. Since the dispatcher database lock instead of the resource
            //      lock is being used to synchronize access to the resource,
            //      it is possible for the information being read from the
            //      resource to be stale. However, the important thing that
            //      cannot change is a valid thread address. Thus a thread
            //      could possibly get boosted that actually has dropped its
            //      access to the resource, but it guaranteed that the thread
            //      cannot be terminated or otherwise deleted.
            //
            // N.B. The dispatcher lock is released by the wait at the top of
            //      loop.
            //

            CurrentThread = KeGetCurrentThread();

            KiLockDispatcherDatabase(&CurrentThread->WaitIrql);
            CurrentThread->WaitNext = TRUE;

            //
            // Attempt to boost the one owner that can be shared or exclusive.
            //

            OwnerThread = (PKTHREAD)Resource->OwnerThreads[0].OwnerThread;
            if (OwnerThread != NULL) {
                ExpBoostOwnerThread(CurrentThread, OwnerThread);
            }

            //
            // If the specified resource is not owned exclusive, then attempt
            // to boost all the owning shared threads priority.
            //

            if (!IsOwnedExclusive(Resource)) {
                OwnerThread = (PKTHREAD)Resource->OwnerThreads[1].OwnerThread;
                if (OwnerThread != NULL) {
                    ExpBoostOwnerThread(CurrentThread, OwnerThread);
                }

                OwnerEntry = Resource->OwnerTable;
                if (OwnerEntry != NULL) {
                    Number = OwnerEntry->TableSize;
                    for(Index = 1; Index < Number; Index += 1) {
                        OwnerEntry += 1;
                        OwnerThread = (PKTHREAD)OwnerEntry->OwnerThread;
                        if (OwnerThread != NULL) {
                            ExpBoostOwnerThread(CurrentThread, OwnerThread);
                        }
                    }
                }
            }
        }

    } while (TRUE);

    return;
}


POWNER_ENTRY
FASTCALL
ExpFindEmptyEntry(
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

/*++

Routine Description:

    This function searches for a free entry in the resource thread array.

    N.B. This routine is entered with the resource lock held and returns
         with the resource lock held. If the resource lock is released
         to expand the owner table, then the return value will be NULL.
         This is a signal to the caller that the complete operation must
         be repeated. This is done to avoid holding the resource lock
         while memory is allocated and freed.

Arguments:

    Resource - Supplies a pointer to the resource for which the search
        is performed.

    LockHandle - Supplies a pointer to a lock handle.

Return Value:

    A pointer to an owner entry is returned or NULL if one could not be
    allocated.

--*/

{

    POWNER_ENTRY OwnerBound;
    POWNER_ENTRY OwnerEntry;

    //
    // Search the owner threads for a free entry.
    //

    ASSERT(LockHandle != NULL);
    ASSERT(Resource->OwnerThreads[0].OwnerThread != 0);

    if (Resource->OwnerThreads[1].OwnerThread == 0) {
        return &Resource->OwnerThreads[1];
    }

    OwnerEntry = Resource->OwnerTable;
    if (OwnerEntry != NULL) {

        OwnerBound = &OwnerEntry[OwnerEntry->TableSize];
        OwnerEntry += 1;

        do {
            if (OwnerEntry->OwnerThread == 0) {
                KeGetCurrentThread()->ResourceIndex = (UCHAR)(OwnerEntry - Resource->OwnerTable);
                return OwnerEntry;
            }

            OwnerEntry += 1;
        } while (OwnerEntry != OwnerBound);
    }

    ExpExpandResourceOwnerTable(Resource,LockHandle);
    return NULL;
}


POWNER_ENTRY
FASTCALL
ExpFindCurrentThread(
    IN PERESOURCE Resource,
    IN ERESOURCE_THREAD CurrentThread,
    IN PEXP_LOCK_HANDLE LockHandle OPTIONAL
    )

/*++

Routine Description:

    This function searches for the specified thread in the resource
    thread array. If the thread is located, then a pointer to the
    array entry is returned as the function value. Otherwise, a pointer
    to a free entry is returned.

    N.B. This routine is entered with the resource lock held and returns
         with the resource lock held. If the resource lock is released
         to expand the owner table, then the return value will be NULL.
         This is a signal to the caller that the complete operation must
         be repeated. This is done to avoid holding the resource lock
         while memory is allocated and freed.

Arguments:

    Resource - Supplies a pointer to the resource for which the search
        is performed.

    CurrentThread - Supplies the identification of the thread to search
        for.

    LockHandle - Supplies a pointer to a lock handle.  If NULL, then the
        caller just wants to know if the requested thread is an owner of
        this resource.  No free entry index is returned and no table
        expansion is performed.  Instead NULL is returned if the requested
        thread cannot be found in the table.

Return Value:

    A pointer to an owner entry is returned or NULL if one could not be
    allocated.

--*/

{

    POWNER_ENTRY FreeEntry;
    POWNER_ENTRY OwnerBound;
    POWNER_ENTRY OwnerEntry;

    //
    // Search the owner threads for the specified thread and return either
    // a pointer to the found thread or a pointer to a free thread table
    // entry.
    //

    OwnerEntry = &Resource->OwnerThreads[0];
    if (OwnerEntry->OwnerThread == CurrentThread) {
        return OwnerEntry;
    }

    OwnerEntry += 1;
    if (OwnerEntry->OwnerThread == CurrentThread) {
        return OwnerEntry;
    }

    if (OwnerEntry->OwnerThread == 0) {
        FreeEntry = OwnerEntry;
    } else {
        FreeEntry = NULL;
    }

    OwnerEntry = Resource->OwnerTable;
    if (OwnerEntry != NULL) {
        OwnerBound = &OwnerEntry[OwnerEntry->TableSize];
        OwnerEntry += 1;
        do {
            if (OwnerEntry->OwnerThread == CurrentThread) {
                KeGetCurrentThread()->ResourceIndex = (UCHAR)(OwnerEntry - Resource->OwnerTable);
                return OwnerEntry;
            }

            if ((FreeEntry == NULL) &&
                (OwnerEntry->OwnerThread == 0)) {
                FreeEntry = OwnerEntry;
            }

            OwnerEntry += 1;
        } while (OwnerEntry != OwnerBound);
    }

    if (!ARGUMENT_PRESENT(LockHandle)) {

        //
        // No argument indicates the caller does not want a free entry or
        // automatic table expansion.  The caller just wants to know if the
        // requested thread is a resource owner.  And clearly the answer is
        // NO at this point.
        //

        return NULL;
    }

    //
    // If a free entry was found in the table, then return the address of the
    // free entry. Otherwise, expand the size of the owner thread table.
    //

    if (FreeEntry != NULL) {
        KeGetCurrentThread()->ResourceIndex = (UCHAR)(FreeEntry - Resource->OwnerTable);
        return FreeEntry;
    }

    ExpExpandResourceOwnerTable(Resource,LockHandle);
    return NULL;
}



VOID
ExpExpandResourceOwnerTable (
    IN PERESOURCE Resource,
    IN PEXP_LOCK_HANDLE LockHandle
    )

/*++

Routine Description:

    This function expands the owner table associated with a resource.  It
    is entered and exited with the resource lock held, however it will
    release the resource lock while performing the memory allocation.

Arguments:

    Resource - Supplies a pointer to the resource to wait for.

    Object - Supplies a pointer to an event (exclusive) or semaphore
       (shared) to wait for.

Return Value:

    None.

--*/

{
    ULONG NewSize;
    KIRQL OldIrql;
    ULONG OldSize;
    POWNER_ENTRY OldTable;
    POWNER_ENTRY OwnerTable;

    //
    // Save previous owner table address and allocate an expanded owner table.
    //

    ExpIncrementCounter(OwnerTableExpands);

    OldTable = Resource->OwnerTable;
    if (OldTable == NULL) {
        OldSize = 0;
        NewSize = 3;
    } else {
        OldSize = OldTable->TableSize;
        NewSize = OldSize + 4;
    }

    EXP_UNLOCK_RESOURCE(Resource, LockHandle);

    OwnerTable = ExAllocatePoolWithTag(NonPagedPool,
                                       NewSize * sizeof(OWNER_ENTRY),
                                       'aTeR');

    if (OwnerTable == NULL) {
        KeDelayExecutionThread(KernelMode, FALSE, &ExShortTime);

    } else {

        //
        // Zero the expansion area of the new owner table.
        //

        RtlZeroMemory(OwnerTable + OldSize,
                      (NewSize - OldSize) * sizeof(OWNER_ENTRY));

        //
        // Acquire the resource lock and determine if the owner table
        // has been expanded by another thread while the new owner table
        // was being allocated. If the owner table has been expanded by
        // another thread, then release the new owner table. Otherwise,
        // copy the owner table to the new owner table and establish the
        // new owner table as the owner table.
        //

        EXP_LOCK_RESOURCE_RAISE(Resource, LockHandle);
        if ((OldTable != Resource->OwnerTable) ||
            ((OldTable != NULL) && (OldSize != OldTable->TableSize))) {
            EXP_UNLOCK_RESOURCE_RAISE(Resource, LockHandle);
            ExFreePool(OwnerTable);

        } else {
            RtlCopyMemory(OwnerTable,
                          OldTable,
                          OldSize * sizeof(OWNER_ENTRY));

            //
            // Swapping of the owner table must be done while owning the
            // dispatcher lock to prevent a priority boost scan from occurring
            // while the table is being changed. The priority boost scan is
            // done when a time out occurs on a specific resource.
            //

            KiLockDispatcherDatabase(&OldIrql);
            OwnerTable->TableSize = NewSize;
            Resource->OwnerTable = OwnerTable;
            KiUnlockDispatcherDatabase(OldIrql);

            ASSERT_RESOURCE(Resource);

            //
            // Release the resource lock and free the old owner table.
            //

            EXP_UNLOCK_RESOURCE_RAISE(Resource, LockHandle);
            if (OldTable != NULL) {
                ExFreePool(OldTable);
            }

            if (OldSize == 0) {
                OldSize = 1;
            }
        }
    }

    //
    // Set the hint index, acquire the resource lock, and return NULL
    // as the function value. This will force a reevaluation of the
    // calling resource function.
    //

    KeGetCurrentThread()->ResourceIndex = (CCHAR)OldSize;
    EXP_LOCK_RESOURCE(Resource, LockHandle);
}


#if DBG

VOID
ExpAssertResource (
    IN PERESOURCE Resource
    )

{
    //
    //  Assert that resource structure is correct.
    //
    // N.B. This routine is called with the resource lock held.
    //

    //
    // Check resource is aligned naturally.
    //

    ASSERT((((ULONG_PTR)Resource) & (sizeof(ULONG_PTR) - 1)) == 0);

    ASSERT(!Resource->SharedWaiters ||
           Resource->SharedWaiters->Header.Type == SemaphoreObject);

    ASSERT(!Resource->SharedWaiters ||
           Resource->SharedWaiters->Header.Size == (sizeof(KSEMAPHORE) / sizeof(ULONG)));

    ASSERT(!Resource->ExclusiveWaiters ||
           Resource->ExclusiveWaiters->Header.Type == SynchronizationEvent);

    ASSERT(!Resource->ExclusiveWaiters ||
           Resource->ExclusiveWaiters->Header.Size == (sizeof(KEVENT) / sizeof(ULONG)));
}

#endif

PVOID
ExpCheckForResource (
    IN PVOID p,
    IN SIZE_T Size
    )

{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY Head, Next;
    volatile PLIST_ENTRY Last=NULL, Last1=NULL;
    PERESOURCE Resource;
    PCHAR BeginBlock;
    PCHAR EndBlock;

    //
    // This can cause a deadlock on MP machines.
    //

    if (KeNumberProcessors > 1) {
        return NULL;
    }

    if ((ExResourceCheckFlags&EX_RESOURCE_CHECK_FREES) == 0) {
        return NULL;
    }


    BeginBlock = (PCHAR)p;
    EndBlock = (PCHAR)p + Size;

    Head = &ExpSystemResourcesList;

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);
    Next = Head->Flink;
    while (Next != Head) {
        Resource = CONTAINING_RECORD(Next,
                                     ERESOURCE,
                                     SystemResourcesList);

        if ((PCHAR)Resource >= BeginBlock && (PCHAR)Resource < EndBlock) {
            DbgPrint("EX: ExFreePool( %p, %lx ) contains an ERESOURCE structure that has not been ExDeleteResourced\n",
                     p,
                     Size);

            DbgBreakPoint ();

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return (PVOID)Resource;
        }

        //
        //  Save the last ptr in a volatile variable for debugging when a flink is bad
        //  

        Last1 = Last;
        Last = Next;
        Next = Next->Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    return NULL;
}

#if DBG

VOID
ExCheckIfResourceOwned (
    VOID
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY Head, Next;
    volatile PLIST_ENTRY Last=NULL, Last1=NULL;
    PERESOURCE Resource;

    if ((ExResourceCheckFlags&EX_RESOURCE_CHECK_ORPHANS) == 0) {
        return;
    }

    Head = &ExpSystemResourcesList;

    KeAcquireInStackQueuedSpinLock (&ExpResourceSpinLock, &LockHandle);

    Next = Head->Flink;
    while (Next != Head) {
        Resource = CONTAINING_RECORD(Next,
                                     ERESOURCE,
                                     SystemResourcesList);

        if (ExIsResourceAcquiredSharedLite (Resource) != 0) {
            DbgPrint("EX: Resource %p held in a position where all resources acquires should have been released\n",
                     Resource);

            DbgBreakPoint ();

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return;
        }

        //
        //  Save the last ptr in a volatile variable for debugging when a flink is bad
        //  

        Last1 = Last;
        Last = Next;
        Next = Next->Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);
    return;    
}
#endif

