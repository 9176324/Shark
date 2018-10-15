/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    spinlock.c

Abstract:

    This module implements the platform specific functions for acquiring
    and releasing spin locks.

--*/

#include "ki.h"

DECLSPEC_NOINLINE
PKSPIN_LOCK_QUEUE
KxWaitForLockChainValid (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*++

Routine Description:

    This function is called when the attempt to release a queued spin lock
    fails. A spin loop is executed to wait until the next owner fills in
    the next pointer in the specified lock queue entry.

Arguments:

    LockQueue - Supplies the address of a lock queue entry.

Return Value;

    The address of the next lock queue entry is returned as the function
    value.

--*/

{

    PKSPIN_LOCK_QUEUE NextQueue;

    //
    // Wait for lock chain to become valid.
    //

    do {
        KeYieldProcessor();
    } while ((NextQueue = LockQueue->Next) == NULL);

    return NextQueue;
}

DECLSPEC_NOINLINE
ULONG64
KxWaitForLockOwnerShip (
    __inout PKSPIN_LOCK_QUEUE LockQueue,
    __inout PKSPIN_LOCK_QUEUE TailQueue
    )

/*++

Routine Description:

Arguments:

    LockQueue - Supplies the address of the lock queue entry that is now
        the last entry in the lock queue.

    TailQueue - Supplies the address of the previous last entry in the lock
        queue.

Return Value:

    The number of wait loops that were executed.

--*/

{

    ULONG64 SpinCount;

    //
    // Set the wait bit in the acquiring lock queue entry and set the next
    // lock queue entry in the last lock queue entry.
    //

    *((ULONG64 volatile *)&LockQueue->Lock) |= LOCK_QUEUE_WAIT;
    TailQueue->Next = LockQueue;

    //
    // Wait for lock ownership to be passed.
    //

    SpinCount = 0;
    do {
        KeYieldProcessor();
    } while ((*((ULONG64 volatile *)&LockQueue->Lock) & LOCK_QUEUE_WAIT) != 0);

    KeMemoryBarrier();
    return SpinCount;
}

__forceinline
VOID
KxAcquireQueuedSpinLock (
    __inout PKSPIN_LOCK_QUEUE LockQueue,
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function acquires a queued spin lock at the current IRQL.

Arguments:

    LockQueue - Supplies a pointer to a spin lock queue.

    SpinLock - Supplies a pointer to the spin lock associated with the lock
        queue.

Return Value:

    None.

--*/

{

    //
    // Insert the specified lock queue entry at the end of the lock queue
    // list. If the list was previously empty, then lock ownership is
    // immediately granted. Otherwise, wait for ownership of the lock to
    // be granted.
    //

#if !defined(NT_UP)

    PKSPIN_LOCK_QUEUE TailQueue;

    TailQueue = InterlockedExchangePointer((PVOID *)SpinLock, LockQueue);
    if (TailQueue != NULL) {
        KxWaitForLockOwnerShip(LockQueue, TailQueue);
    }

#else

    UNREFERENCED_PARAMETER(LockQueue);
    UNREFERENCED_PARAMETER(SpinLock);

#endif

    return;
}

__forceinline
LOGICAL
KxTryToAcquireQueuedSpinLock (
    __inout PKSPIN_LOCK_QUEUE LockQueue,
    __inout PKSPIN_LOCK SpinLock
    )

/*++

Routine Description:

    This function attempts to acquire the specified queued spin lock at
    the current IRQL.

Arguments:

    LockQueue - Supplies a pointer to a spin lock queue.

    SpinLock - Supplies a pointer to the spin lock associated with the lock
        queue.

Return Value:

    A value of TRUE is returned is the specified queued spin lock is
    acquired. Otherwise, a value of FALSE is returned.

--*/

{

    //
    // Insert the specified lock queue entry at the end of the lock queue
    // list iff the lock queue list is currently empty. If the lock queue
    // was empty, then lock ownership is granted and TRUE is returned.
    // Otherwise, FALSE is returned.
    //

#if !defined(NT_UP)

    if ((ReadForWriteAccess(SpinLock) != 0) ||
        (InterlockedCompareExchangePointer((PVOID *)SpinLock,
                                                  LockQueue,
                                                  NULL) != NULL)) {

        KeYieldProcessor();
        return FALSE;

    }

#else

    UNREFERENCED_PARAMETER(LockQueue);
    UNREFERENCED_PARAMETER(SpinLock);

#endif

    return TRUE;
}

__forceinline
VOID
KxReleaseQueuedSpinLock (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*++

Routine Description:

    The function release a queued spin lock at the current IRQL.

Arguments:

    LockQueue - Supplies a pointer to a spin lock queue.

Return Value:

    None.

--*/

{

    //
    // Attempt to release the lock. If the lock queue is not empty, then wait
    // for the next entry to be written in the lock queue entry and then grant
    // ownership of the lock to the next lock queue entry.
    //

#if !defined(NT_UP)

    PKSPIN_LOCK_QUEUE NextQueue;

    NextQueue = ReadForWriteAccess(&LockQueue->Next);
    if (NextQueue == NULL) {
        if (InterlockedCompareExchangePointer((PVOID *)LockQueue->Lock,
                                              NULL,
                                              LockQueue) == LockQueue) {
            return;
        }

        NextQueue = KxWaitForLockChainValid(LockQueue);
    }

    ASSERT(((ULONG64)NextQueue->Lock & LOCK_QUEUE_WAIT) != 0);

    InterlockedXor64((LONG64 volatile *)&NextQueue->Lock, LOCK_QUEUE_WAIT);
    LockQueue->Next = NULL;

#else

    UNREFERENCED_PARAMETER(LockQueue);

#endif

    return;
}

#undef KeAcquireQueuedSpinLock

KIRQL
KeAcquireQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number
    )

/*++

Routine Description:

    This function raises IRQL to DISPATCH_LEVEL and acquires the specified
    numbered queued spin lock.

Arguments:

    Number  - Supplies the queued spin lock number.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
    PKSPIN_LOCK SpinLock;

    //
    // Raise IRQL to DISPATCH_LEVEL and acquire the specified queued spin
    // lock.
    //

    OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    LockQueue = &KiGetLockQueue()[Number];
    SpinLock = LockQueue->Lock;
    KxAcquireQueuedSpinLock(LockQueue, SpinLock);
    return OldIrql;
}

#undef KeAcquireQueuedSpinLockRaiseToSynch

KIRQL
KeAcquireQueuedSpinLockRaiseToSynch (
    __in KSPIN_LOCK_QUEUE_NUMBER Number
    )

/*++

Routine Description:

    This function raises IRQL to SYNCH_LEVEL and acquires the specified
    numbered queued spin lock.

Arguments:

    Number - Supplies the queued spinlock number.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
    PKSPIN_LOCK SpinLock;

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the specified queued spin
    // lock.
    //

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    LockQueue = &KiGetLockQueue()[Number];
    SpinLock = LockQueue->Lock;
    KxAcquireQueuedSpinLock(LockQueue, SpinLock);
    return OldIrql;
}

#undef KeAcquireQueuedSpinLockAtDpcLevel

VOID
KeAcquireQueuedSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*++

Routine Description:

    This function acquires the specified queued spin lock at the current IRQL.

Arguments:

    LockQueue - Supplies a pointer to the lock queue entry for the specified
        queued spin lock.

Return Value:

    None.

--*/

{

    //
    // Acquire the specified queued spin lock at the current IRQL.
    //

    KxAcquireQueuedSpinLock(LockQueue, LockQueue->Lock);
    return;
}

#undef KeTryToAcquireQueuedSpinLock

LOGICAL
KeTryToAcquireQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number,
    __out PKIRQL OldIrql
    )

/*++

Routine Description:

    This function raises IRQL to DISPATCH_LEVEL and attempts to acquire the
    specified numbered queued spin lock. If the spin lock is already owned,
    then IRQL is restored to its previous value and FALSE is returned.
    Otherwise, the spin lock is acquired and TRUE is returned.

Arguments:

    Number - Supplies the queued spinlock number.

    OldIrql - Supplies a pointer to the variable to receive the old IRQL.

Return Value:

    If the spin lock is acquired a value TRUE is returned. Otherwise, FALSE
    is returned as the function value.

--*/

{
    PKSPIN_LOCK_QUEUE LockQueue;
    PKSPIN_LOCK SpinLock;

    //
    // Try to acquire the specified queued spin lock at DISPATCH_LEVEL.
    //

    *OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    LockQueue = &KiGetLockQueue()[Number];
    SpinLock = LockQueue->Lock;
    if (KxTryToAcquireQueuedSpinLock(LockQueue, SpinLock) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;

    }

    return TRUE;
}

#undef KeTryToAcquireQueuedSpinLockRaiseToSynch

LOGICAL
KeTryToAcquireQueuedSpinLockRaiseToSynch (
    __in  KSPIN_LOCK_QUEUE_NUMBER Number,
    __out PKIRQL OldIrql
    )

/*++

Routine Description:

    This function raises IRQL to SYNCH_LEVEL and attempts to acquire the
    specified numbered queued spin lock. If the spin lock is already owned,
    then IRQL is restored to its previous value and FALSE is returned.
    Otherwise, the spin lock is acquired and TRUE is returned.

Arguments:

    Number - Supplies the queued spinlock number.

    OldIrql - Supplies a pointer to the variable to receive the old IRQL.

Return Value:

    If the spin lock is acquired a value TRUE is returned. Otherwise, FALSE
    is returned as the function value.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    PKSPIN_LOCK SpinLock;

    //
    // Try to acquire the specified queued spin lock at SYNCH_LEVEL.
    //

    *OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    LockQueue = &KiGetLockQueue()[Number];
    SpinLock = LockQueue->Lock;
    if (KxTryToAcquireQueuedSpinLock(LockQueue, SpinLock) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;

    }

    return TRUE;
}

#undef KeTryToAcquireQueuedSpinLockAtRaisedIrql

LOGICAL
KeTryToAcquireQueuedSpinLockAtRaisedIrql (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*++

Routine Description:

    This function attempts to acquire the specified queued spin lock at the
    current IRQL.

Arguments:

    LockQueue - Supplies a pointer to a lock queue entry.

Return Value:

    If the spin lock is acquired a value TRUE is returned as the function
    value. Otherwise, FALSE is returned as the function value.

--*/

{

    //
    // Try to acquire the specified queued spin lock at the current IRQL.
    //

    return KxTryToAcquireQueuedSpinLock(LockQueue, LockQueue->Lock);
}

#undef KeReleaseQueuedSpinLock

VOID
KeReleaseQueuedSpinLock (
    __in KSPIN_LOCK_QUEUE_NUMBER Number,
    __in KIRQL OldIrql
    )

/*++

Routine Description:

    This function releases a numbered queued spin lock and lowers the IRQL to
    its previous value.

Arguments:

    Number - Supplies the queued spinlock number.

    OldIrql  - Supplies the previous IRQL value.

Return Value:

    None.

--*/

{

    //
    // Release the specified queued spin lock and lower IRQL.
    //

    KxReleaseQueuedSpinLock(&KiGetLockQueue()[Number]);
    KeLowerIrql(OldIrql);
    return;
}

#undef KeReleaseQueuedSpinLockFromDpcLevel

VOID
KeReleaseQueuedSpinLockFromDpcLevel (
    __inout PKSPIN_LOCK_QUEUE LockQueue
    )

/*

Routine Description:

    This function releases a queued spinlock from the current IRQL.

Arguments:

    LockQueue - Supplies a pointer to a lock queue entry.

Return Value:

    None.

--*/

{

    //
    // Release the specified queued spin lock at the current IRQL.
    //

    KxReleaseQueuedSpinLock(LockQueue);
    return;
}

VOID
KeAcquireInStackQueuedSpinLock (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    )

/*++

Routine Description:

    This function raises IRQL to DISPATCH_LEVEL and acquires the specified
    in stack queued spin lock.

Arguments:

    SpinLock - Supplies the home address of the queued spin lock.

    LockHandle - Supplies the address of a lock queue handle.

Return Value:

    None.

--*/

{

    //
    // Raise IRQL to DISPATCH_LEVEL and acquire the specified in stack
    // queued spin lock.
    //

#if !defined(NT_UP)

    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif

    LockHandle->OldIrql = KfRaiseIrql(DISPATCH_LEVEL);
    KxAcquireQueuedSpinLock(&LockHandle->LockQueue, SpinLock);
    return;
}

VOID
KeAcquireInStackQueuedSpinLockRaiseToSynch (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    )

/*++

Routine Description:

    This functions raises IRQL to SYNCH_LEVEL and acquires the specified
    in stack queued spin lock.

Arguments:

    SpinLock - Supplies the home address of the queued spin lock.

    LockHandle - Supplies the address of a lock queue handle.

Return Value:

    None.

--*/

{

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the specified in stack
    // queued spin lock.
    //

#if !defined(NT_UP)

    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif    

    LockHandle->OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    KxAcquireQueuedSpinLock(&LockHandle->LockQueue, SpinLock);
    return;
}

VOID
KeAcquireInStackQueuedSpinLockAtDpcLevel (
    __inout PKSPIN_LOCK SpinLock,
    __out PKLOCK_QUEUE_HANDLE LockHandle
    )

/*++

Routine Description:

    This function acquires the specified in stack queued spin lock at the
    current IRQL.

Arguments:

    SpinLock - Supplies a pointer to thehome address of a spin lock.

    LockHandle - Supplies the address of a lock queue handle.

Return Value:

    None.

--*/

{

    //
    // Acquire the specified in stack queued spin lock at the current
    // IRQL.
    //

#if !defined(NT_UP)

    LockHandle->LockQueue.Lock = SpinLock;
    LockHandle->LockQueue.Next = NULL;

#else

    UNREFERENCED_PARAMETER(SpinLock);

#endif

    KxAcquireQueuedSpinLock(&LockHandle->LockQueue, SpinLock);
    return;
}

VOID
KeReleaseInStackQueuedSpinLock (
    __in PKLOCK_QUEUE_HANDLE LockHandle
    )

/*++

Routine Description:

    This function releases an in stack queued spin lock and lowers the IRQL
    to its previous value.

Arguments:

    LockHandle - Supplies the address of a lock queue handle.

Return Value:

    None.

--*/

{

    //
    // Release the specified in stack queued spin lock and lower IRQL.
    //

    KxReleaseQueuedSpinLock(&LockHandle->LockQueue);
    KeLowerIrql(LockHandle->OldIrql);
    return;
}

VOID
KeReleaseInStackQueuedSpinLockFromDpcLevel (
    __in PKLOCK_QUEUE_HANDLE LockHandle
    )

/*++

Routine Description:

    This function releases an in stack queued spinlock at the current IRQL.

Arguments:

   LockHandle - Supplies a pointer to lock queue handle.

Return Value:

    None.

--*/

{

    //
    // Release the specified in stack queued spin lock at the current IRQL.
    //

    KxReleaseQueuedSpinLock(&LockHandle->LockQueue);
    return;
}

KIRQL
KiAcquireDispatcherLockRaiseToSynch (
    VOID
    )

/*++

Routine Description:

    This function raises IRQL to SYNCH_LEVEL and acquires the dispatcher
    database queued spin lock.

Arguments:

    None.

Return Value:

    The previous IRQL is returned as the function value.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    KIRQL OldIrql;
    PKSPIN_LOCK SpinLock;

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the dispatcher database queued
    // spin lock.
    //

    OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    LockQueue = &KiGetLockQueue()[LockQueueDispatcherLock];
    SpinLock = LockQueue->Lock;
    KxAcquireQueuedSpinLock(LockQueue, SpinLock);
    return OldIrql;
}

VOID
KiAcquireDispatcherLockAtSynchLevel (
    VOID
    )

/*++

Routine Description:

    This function acquires the dispatcher database queued spin lock at the
    current IRQL.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    PKSPIN_LOCK SpinLock;

    //
    // Acquire the dispatcher database queued spin lock at the current IRQL.
    //

    LockQueue = &KiGetLockQueue()[LockQueueDispatcherLock];
    SpinLock = LockQueue->Lock;
    KxAcquireQueuedSpinLock(LockQueue, SpinLock);
    return;
}

VOID
KiReleaseDispatcherLockFromSynchLevel (
    VOID
    )

/*

Routine Description:

    This function releases the dispatcher database queued spinlock from the
    current IRQL.

Arguments:

    None.

Return Value:

    None.

--*/

{

    //
    // Release the dispatcher databsse queued spin lock at the current IRQL.
    //

    KxReleaseQueuedSpinLock(&KiGetLockQueue()[LockQueueDispatcherLock]);
    return;
}

LOGICAL
KiTryToAcquireDispatcherLockRaiseToSynch (
    __out PKIRQL OldIrql
    )

/*++

Routine Description:

    This function raises IRQL to SYNCH_LEVEL and attempts to acquire the
    dispatcher database spin lock. If the dispatcher database lock is
    already owned, then IRQL is restored to its previous value and FALSE
    is returned. Otherwise, the dispatcher database lock is acquired and
    TRUE is returned.

Arguments:

    OldIrql - Supplies a pointer to the variable to receive the old IRQL.

Return Value:

    If the dispatcher database lock is acquired a value TRUE is returned.
    Otherwise, FALSE is returned as the function value.

--*/

{

    PKSPIN_LOCK_QUEUE LockQueue;
    PKSPIN_LOCK SpinLock;

    //
    // Try to acquire the dispatcher database queued spin lock at SYNCH_LEVEL.
    //

    *OldIrql = KfRaiseIrql(SYNCH_LEVEL);
    LockQueue = &KiGetLockQueue()[LockQueueDispatcherLock];
    SpinLock = LockQueue->Lock;
    if (KxTryToAcquireQueuedSpinLock(LockQueue, SpinLock) == FALSE) {
        KeLowerIrql(*OldIrql);
        return FALSE;

    }

    return TRUE;
}

