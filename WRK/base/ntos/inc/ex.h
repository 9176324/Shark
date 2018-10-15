/*++ BUILD Version: 0007    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ex.h

Abstract:

    Public executive data structures and procedure prototypes.

--*/

#ifndef _EX_
#define _EX_

//
// Define caller count hash table structures and function prototypes.
//

#define CALL_HASH_TABLE_SIZE 64

typedef struct _CALL_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID CallersAddress;
    PVOID CallersCaller;
    ULONG CallCount;
} CALL_HASH_ENTRY, *PCALL_HASH_ENTRY;

typedef struct _CALL_PERFORMANCE_DATA {
    KSPIN_LOCK SpinLock;
    LIST_ENTRY HashTable[CALL_HASH_TABLE_SIZE];
} CALL_PERFORMANCE_DATA, *PCALL_PERFORMANCE_DATA;

VOID
ExInitializeCallData(
    IN PCALL_PERFORMANCE_DATA CallData
    );

VOID
ExRecordCallerInHashTable(
    IN PCALL_PERFORMANCE_DATA CallData,
    IN PVOID CallersAddress,
    IN PVOID CallersCaller
    );

#define RECORD_CALL_DATA(Table)                                            \
    {                                                                      \
        PVOID CallersAddress;                                              \
        PVOID CallersCaller;                                               \
        RtlGetCallersAddress(&CallersAddress, &CallersCaller);             \
        ExRecordCallerInHashTable((Table), CallersAddress, CallersCaller); \
    }

//
// Define executive event pair object structure.
//

typedef struct _EEVENT_PAIR {
    KEVENT_PAIR KernelEventPair;
} EEVENT_PAIR, *PEEVENT_PAIR;

//
// empty struct def so we can forward reference ETHREAD
//

struct _ETHREAD;

//
// System Initialization procedure for EX subcomponent of NTOS (in exinit.c)
//

NTKERNELAPI
BOOLEAN
ExInitSystem(
    VOID
    );

VOID
ExInitSystemPhase2 (
    VOID
    );

VOID
ExInitPoolLookasidePointers (
    VOID
    );

ULONG
ExComputeTickCountMultiplier (
    IN ULONG TimeIncrement
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntndis begin_ntosp
//
// Pool Allocation routines (in pool.c)
//

typedef enum _POOL_TYPE {
    NonPagedPool,
    PagedPool,
    NonPagedPoolMustSucceed,
    DontUseThisType,
    NonPagedPoolCacheAligned,
    PagedPoolCacheAligned,
    NonPagedPoolCacheAlignedMustS,
    MaxPoolType

    // end_wdm
    ,
    //
    // Note these per session types are carefully chosen so that the appropriate
    // masking still applies as well as MaxPoolType above.
    //

    NonPagedPoolSession = 32,
    PagedPoolSession = NonPagedPoolSession + 1,
    NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
    DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
    NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
    PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
    NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,

    // begin_wdm

    } POOL_TYPE;

#define POOL_COLD_ALLOCATION 256     // Note this cannot encode into the header.

// end_ntddk end_wdm end_nthal end_ntifs end_ntndis begin_ntosp

//
// The following two definitions control the raising of exceptions on quota
// and allocation failures.
//

#define POOL_QUOTA_FAIL_INSTEAD_OF_RAISE 8
#define POOL_RAISE_IF_ALLOCATION_FAILURE 16               // ntifs
#define POOL_MM_ALLOCATION 0x80000000     // Note this cannot encode into the header.


// end_ntosp

VOID
InitializePool(
    IN POOL_TYPE PoolType,
    IN ULONG Threshold
    );

//
// These routines are private to the pool manager and the memory manager.
//

VOID
ExInsertPoolTag (
    ULONG Tag,
    PVOID Va,
    SIZE_T NumberOfBytes,
    POOL_TYPE PoolType
    );

VOID
ExAllocatePoolSanityChecks(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes
    );

VOID
ExFreePoolSanityChecks(
    IN PVOID P
    );

// begin_ntddk begin_nthal begin_ntifs begin_wdm begin_ntosp

DECLSPEC_DEPRECATED_DDK                     // Use ExAllocatePoolWithTag
NTKERNELAPI
__bcount(NumberOfBytes) 
PVOID
ExAllocatePool(
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes
    );

DECLSPEC_DEPRECATED_DDK                     // Use ExAllocatePoolWithQuotaTag
NTKERNELAPI
__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithQuota(
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes
    );

NTKERNELAPI
__bcount(NumberOfBytes) 
PVOID
NTAPI
ExAllocatePoolWithTag(
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag
    );

//
// _EX_POOL_PRIORITY_ provides a method for the system to handle requests
// intelligently in low resource conditions.
//
// LowPoolPriority should be used when it is acceptable to the driver for the
// mapping request to fail if the system is low on resources.  An example of
// this could be for a non-critical network connection where the driver can
// handle the failure case when system resources are close to being depleted.
//
// NormalPoolPriority should be used when it is acceptable to the driver for the
// mapping request to fail if the system is very low on resources.  An example
// of this could be for a non-critical local filesystem request.
//
// HighPoolPriority should be used when it is unacceptable to the driver for the
// mapping request to fail unless the system is completely out of resources.
// An example of this would be the paging file path in a driver.
//
// SpecialPool can be specified to bound the allocation at a page end (or
// beginning).  This should only be done on systems being debugged as the
// memory cost is expensive.
//
// N.B.  These values are very carefully chosen so that the pool allocation
//       code can quickly crack the priority request.
//

typedef enum _EX_POOL_PRIORITY {
    LowPoolPriority,
    LowPoolPrioritySpecialPoolOverrun = 8,
    LowPoolPrioritySpecialPoolUnderrun = 9,
    NormalPoolPriority = 16,
    NormalPoolPrioritySpecialPoolOverrun = 24,
    NormalPoolPrioritySpecialPoolUnderrun = 25,
    HighPoolPriority = 32,
    HighPoolPrioritySpecialPoolOverrun = 40,
    HighPoolPrioritySpecialPoolUnderrun = 41

    } EX_POOL_PRIORITY;

NTKERNELAPI
__bcount(NumberOfBytes) 
PVOID
NTAPI
ExAllocatePoolWithTagPriority(
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag,
    __in EX_POOL_PRIORITY Priority
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif //POOL_TAGGING

NTKERNELAPI
__bcount(NumberOfBytes) 
PVOID
ExAllocatePoolWithQuotaTag(
    __in POOL_TYPE PoolType,
    __in SIZE_T NumberOfBytes,
    __in ULONG Tag
    );

#ifndef POOL_TAGGING
#define ExAllocatePoolWithQuotaTag(a,b,c) ExAllocatePoolWithQuota(a,b)
#endif //POOL_TAGGING

NTKERNELAPI
VOID
NTAPI
ExFreePool(
    __in PVOID P
    );

// end_wdm

#if defined(POOL_TAGGING)
#define ExFreePool(a) ExFreePoolWithTag(a,0)
#endif

//
// If high order bit in Pool tag is set, then must use ExFreePoolWithTag to free
//

#define PROTECTED_POOL 0x80000000

// begin_wdm

NTKERNELAPI
VOID
ExFreePoolWithTag(
    __in PVOID P,
    __in ULONG Tag
    );

// end_ntddk end_wdm end_nthal end_ntifs


#ifndef POOL_TAGGING
#define ExFreePoolWithTag(a,b) ExFreePool(a)
#endif //POOL_TAGGING

// end_ntosp


NTKERNELAPI
KIRQL
ExLockPool(
    __in POOL_TYPE PoolType
    );

NTKERNELAPI
VOID
ExUnlockPool(
    __in POOL_TYPE PoolType,
    __in KIRQL LockHandle
    );

// begin_ntosp
NTKERNELAPI                                     // ntifs
SIZE_T                                          // ntifs
ExQueryPoolBlockSize (                          // ntifs
    __in PVOID PoolBlock,                       // ntifs
    __out PBOOLEAN QuotaCharged                 // ntifs
    );                                          // ntifs
// end_ntosp

NTKERNELAPI
VOID
ExQueryPoolUsage(
    __out PULONG PagedPoolPages,
    __out PULONG NonPagedPoolPages,
    __out PULONG PagedPoolAllocs,
    __out PULONG PagedPoolFrees,
    __out PULONG PagedPoolLookasideHits,
    __out PULONG NonPagedPoolAllocs,
    __out PULONG NonPagedPoolFrees,
    __out PULONG NonPagedPoolLookasideHits
    );

VOID
ExReturnPoolQuota (
    IN PVOID P
    );

// begin_ntifs begin_ntddk begin_wdm begin_nthal begin_ntosp
//
// Routines to support fast mutexes.
//

typedef struct _FAST_MUTEX {

#define FM_LOCK_BIT          0x1 // Actual lock bit, 1 = Unlocked, 0 = Locked
#define FM_LOCK_BIT_V        0x0 // Lock bit as a bit number
#define FM_LOCK_WAITER_WOKEN 0x2 // A single waiter has been woken to acquire this lock
#define FM_LOCK_WAITER_INC   0x4 // Increment value to change the waiters count

    LONG Count;
    PKTHREAD Owner;
    ULONG Contention;
    KEVENT Gate;
    ULONG OldIrql;
} FAST_MUTEX, *PFAST_MUTEX;

#define ExInitializeFastMutex(_FastMutex)                                    \
    (_FastMutex)->Count = FM_LOCK_BIT;                                       \
    (_FastMutex)->Owner = NULL;                                              \
    (_FastMutex)->Contention = 0;                                            \
    KeInitializeEvent(&(_FastMutex)->Gate,                                   \
                      SynchronizationEvent,                                  \
                      FALSE);

// end_ntifs end_ntddk end_wdm end_nthal end_ntosp

C_ASSERT(sizeof(FAST_MUTEX) == sizeof(KGUARDED_MUTEX));

#if !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

VOID
FASTCALL
KiAcquireFastMutex (
    IN PFAST_MUTEX Mutex
    );

FORCEINLINE
VOID
xxAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function acquires ownership of a fast mutex and raises IRQL to
    APC Level.

Arguments:

    FastMutex  - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    KIRQL OldIrql;

    //
    // Raise IRQL to APC_LEVEL and attempt to acquire ownership of the fast
    // mutex.
    //

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    OldIrql = KfRaiseIrql(APC_LEVEL);

#if defined (_X86_)

    if (InterlockedDecrementAcquire(&FastMutex->Count) != 0) {

#else

    if (!InterlockedBitTestAndReset(&FastMutex->Count, FM_LOCK_BIT_V)) {

#endif

        KiAcquireFastMutex(FastMutex);
    }

    //
    // Grant ownership of the fast mutex to the current thread.
    //

    FastMutex->Owner = KeGetCurrentThread();
    FastMutex->OldIrql = OldIrql;
    return;
}

FORCEINLINE
VOID
xxReleaseFastMutex (
    IN PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function releases ownership to a fast mutex and lowers IRQL to
    its previous level.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

#if !defined (_X86_)

    LONG NewValue;
    LONG OldValue;

#endif

    KIRQL OldIrql;

    //
    // N.B. On x86 this code may need to be compatible with the legacy fast
    //      mutex code in an OEM HAL.  The HAL code stores the owning stack
    //      pointer and not the owning thread in the fast mutex owner field.
    //

#if !defined (_X86_)

    ASSERT(FastMutex->Owner == KeGetCurrentThread());

#endif

    ASSERT(KeGetCurrentIrql() == APC_LEVEL);

    //
    // Clear the owner thread.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    FastMutex->Owner = NULL;

    //
    // Save the old IRQL and attempt to release the fast mutex. 
    //

    OldIrql = (KIRQL)FastMutex->OldIrql;

#if defined (_X86_)

    if (InterlockedIncrementRelease(&FastMutex->Count) <= 0) {
        KeSetEventBoostPriority(&FastMutex->Gate, NULL);
    }

#else

    OldValue = InterlockedExchangeAdd(&FastMutex->Count, FM_LOCK_BIT);

    ASSERT((OldValue & FM_LOCK_BIT) == 0);

    //
    // If there are no waiters or a waiter has already been woken, then
    // release the fast mutex. Otherwise, attempt to wake a waiter.
    //

    if ((OldValue != 0) &&
        ((OldValue & FM_LOCK_WAITER_WOKEN) == 0)) {

        //
        // There must be at least one waiter that needs to be woken. Set the
        // woken waiter bit and decrement the waiter count. If the exchange
        // fails, then another thread will do the wake.
        //

        OldValue = OldValue + FM_LOCK_BIT;
        NewValue = OldValue + FM_LOCK_WAITER_WOKEN - FM_LOCK_WAITER_INC;
        if (InterlockedCompareExchange(&FastMutex->Count, NewValue, OldValue) == OldValue) {

            //
            // There are one or more threads waiting for ownership of the
            // mutex.
            //

            KeSignalGateBoostPriority((PKGATE)&FastMutex->Gate);
        }
    }
#endif

    //
    // Lower IRQL to its previous value.
    //

    KeLowerIrql(OldIrql);
    return;
}

FORCEINLINE
BOOLEAN
xxTryToAcquireFastMutex (
    IN PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function attempts to acquire ownership of a fast mutex, and if
    successful, raises IRQL to APC level.

Arguments:

    FastMutex  - Supplies a pointer to a fast mutex.

Return Value:

    If the fast mutex was successfully acquired, then a value of TRUE
    is returned as the function value. Otherwise, a value of FALSE is
    returned.

--*/

{

    KIRQL OldIrql;

    //
    // Raise IRQL to APC_LEVEL and attempt to acquire ownership of the fast
    // mutex.
    //

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    OldIrql = KfRaiseIrql(APC_LEVEL);

#if defined (_X86_)

    if (InterlockedCompareExchangeAcquire(&FastMutex->Count, 0, 1) != 1) {

#else

    if (!InterlockedBitTestAndReset(&FastMutex->Count, FM_LOCK_BIT_V)) {

#endif
        //
        // The fast mutex is owned - lower IRQL to its previous value and
        // return FALSE.
        //

        KeLowerIrql(OldIrql);
        KeYieldProcessor();
        return FALSE;

    } else {

        //
        // Grant ownership of the fast mutex to the current thread and
        // return TRUE.
        //

        FastMutex->Owner = KeGetCurrentThread();
        FastMutex->OldIrql = OldIrql;
        return TRUE;
    }
}

FORCEINLINE
VOID
xxAcquireFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function acquires ownership of a fast mutex, but does not raise
    IRQL to APC Level.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    PKTHREAD Thread;

    //
    // Attempt to acquire ownership of the fast mutex.
    //

    Thread = KeGetCurrentThread();

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (Thread->CombinedApcDisable != 0) ||
           (Thread->Teb == NULL) ||
           (Thread->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(FastMutex->Owner != Thread);

#if defined (_X86_)

    if (InterlockedDecrementAcquire(&FastMutex->Count) != 0) {

#else

    if (!InterlockedBitTestAndReset(&FastMutex->Count, FM_LOCK_BIT_V)) {

#endif

        KiAcquireFastMutex(FastMutex);
    }

    //
    // Grant ownership of the fast mutex to the current thread.
    //

    FastMutex->Owner = Thread;
    return;
}

FORCEINLINE
VOID
xxReleaseFastMutexUnsafe (
    IN PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function releases ownership of a fast mutex, and does not restore
    IRQL to its previous level.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

#if !defined (_X86_)

    LONG NewValue;
    LONG OldValue;

#endif

    KIRQL OldIrql;

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (KeGetCurrentThread()->CombinedApcDisable != 0) ||
           (KeGetCurrentThread()->Teb == NULL) ||
           (KeGetCurrentThread()->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(FastMutex->Owner == KeGetCurrentThread());

    //
    // Clear the owner thread.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    FastMutex->Owner = NULL;

    //
    // Save the old IRQL and attempt to release the fast mutex.
    //

    OldIrql = (KIRQL)FastMutex->OldIrql;

#if defined (_X86_)

    if (InterlockedIncrementRelease(&FastMutex->Count) <= 0) {
        KeSetEventBoostPriority(&FastMutex->Gate, NULL);
    }

#else


    OldValue = InterlockedExchangeAdd(&FastMutex->Count, FM_LOCK_BIT);

    ASSERT((OldValue & FM_LOCK_BIT) == 0);

    //
    // If there are no waiters or a waiter has already been woken, then
    // release the fast mutex. Otherwise, attempt to wake a waiter.
    //

    if ((OldValue != 0) &&
        ((OldValue & FM_LOCK_WAITER_WOKEN) == 0)) {

        //
        // There must be at least one waiter that needs to be woken. Set the
        // woken waiter bit and decrement the waiter count. If the exchange
        // fails, then another thread will do the wake.
        //

        OldValue = OldValue + FM_LOCK_BIT;
        NewValue = OldValue + FM_LOCK_WAITER_WOKEN - FM_LOCK_WAITER_INC;
        if (InterlockedCompareExchange(&FastMutex->Count, NewValue, OldValue) == OldValue) {

            //
            // There are one or more threads waiting for ownership of the
            // mutex
            //

            KeSignalGateBoostPriority((PKGATE)&FastMutex->Gate);
        }
    }

#endif

    return;
}

//
// The EX_SPIN_LOCK could be made a CHAR to save space - this would limit
// support to 127 processors since the high bit is used to denote exclusive.
// Thus we wouldn't want to export it this way, but could use this internally
// for structures that are tight on space (may get a lot of false cacheline
// pinging due to sharing though).
//
// At some point we may also want to do cache aware versions of these APIs.
//

typedef LONG EX_SPIN_LOCK, *PEX_SPIN_LOCK;

#if !defined (NT_UP)

#define EXP_SPIN_LOCK_EXCLUSIVE 0x80000000

FORCEINLINE
KIRQL
ExAcquireSpinLockShared (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    KIRQL OldIrql;
    EX_SPIN_LOCK LockContents;
    EX_SPIN_LOCK NewLockContents;

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    do {

        LockContents = *(volatile EX_SPIN_LOCK *)SpinLock;

        //
        // If the lock is not being sought exclusive by anyone then try for
        // it shared now.
        //

        if ((LockContents & EXP_SPIN_LOCK_EXCLUSIVE) == 0) {

            NewLockContents = LockContents + 1;

            if (InterlockedCompareExchangeAcquire (SpinLock,
                                                   NewLockContents,
                                                   LockContents) == LockContents) {
                return OldIrql;
            }
        }

        KeYieldProcessor();

    } while (TRUE);
}

FORCEINLINE
VOID
ExReleaseSpinLockShared (
    IN PEX_SPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )
{
    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ASSERT (OldIrql <= DISPATCH_LEVEL);
    ASSERT (*SpinLock != 0);

    InterlockedDecrementRelease (SpinLock);
    KeLowerIrql (OldIrql);

    return;
}

FORCEINLINE
LOGICAL
ExTryAcquireSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    EX_SPIN_LOCK LockContents;
    EX_SPIN_LOCK NewLockContents;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    do {

        LockContents = *(volatile EX_SPIN_LOCK *)SpinLock;

        ASSERT (LockContents != 0);

        //
        // If the big pool tag table is already held exclusive, then it cannot
        // possibly be by the current thread - it must be another thread.
        // Release our thread's shared reference and inform our caller so
        // we don't cause the exclusive thread to spin.
        //
        // Otherwise it's safe to try to acquire exclusive ourselves.
        //

        if (LockContents & EXP_SPIN_LOCK_EXCLUSIVE) {
            return FALSE;
        }

        NewLockContents = (LockContents | EXP_SPIN_LOCK_EXCLUSIVE);

        if (InterlockedCompareExchangeAcquire (SpinLock,
                                               NewLockContents,
                                               LockContents) == LockContents) {

            //
            // We are the winner of exclusive now.  However, we must first
            // wait for any straggling threads on other processors to release
            // their references.
            //

            while (*(volatile EX_SPIN_LOCK *)SpinLock != (EXP_SPIN_LOCK_EXCLUSIVE | 0x1)) {
                KeYieldProcessor();
                NOTHING;
            }

            //
            // Now we finally own the lock exclusively.
            //

            return TRUE;
        }

    } while (TRUE);
}

FORCEINLINE
KIRQL
ExAcquireSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    KIRQL OldIrql;

    do {

        //
        // First acquire it shared (so we get a reference).
        //

        OldIrql = ExAcquireSpinLockShared (SpinLock);
    
        //
        // Now try to acquire the lock exclusive.  If another thread wins,
        // then we must release and retry.
        //
    
        if (ExTryAcquireSpinLockExclusive (SpinLock) == TRUE) {
            return OldIrql;
        }

        ExReleaseSpinLockShared (SpinLock, OldIrql);

        KeYieldProcessor();

    } while (TRUE);
}

FORCEINLINE
VOID
ExReleaseSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )
{
    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ASSERT (OldIrql <= DISPATCH_LEVEL);
    ASSERT (*SpinLock == (EXP_SPIN_LOCK_EXCLUSIVE | 0x1));

    KeMemoryBarrierWithoutFence();
    *((EX_SPIN_LOCK volatile *)SpinLock) = 0;
    KeLowerIrql (OldIrql);

    return;
}

#else // (NT_UP)

FORCEINLINE
KIRQL
ExAcquireSpinLockShared (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER (SpinLock);

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    return OldIrql;
}

FORCEINLINE
KIRQL
ExAcquireSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER (SpinLock);

    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

    return OldIrql;
}

FORCEINLINE
LOGICAL
ExTryAcquireSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock
    )
{
    UNREFERENCED_PARAMETER (SpinLock);

    return TRUE;
}

FORCEINLINE
VOID
ExReleaseSpinLockShared (
    IN PEX_SPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )
{
    UNREFERENCED_PARAMETER (SpinLock);

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ASSERT (OldIrql <= DISPATCH_LEVEL);

    KeLowerIrql (OldIrql);

    return;
}

FORCEINLINE
VOID
ExReleaseSpinLockExclusive (
    IN PEX_SPIN_LOCK SpinLock,
    IN KIRQL OldIrql
    )
{
    UNREFERENCED_PARAMETER (SpinLock);

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ASSERT (OldIrql <= DISPATCH_LEVEL);

    KeLowerIrql (OldIrql);

    return;
}

#endif
#endif // !(defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_)) && !defined(_BLDR_)

#if defined(_NTDRIVER_) || defined(_NTIFS_) || defined(_NTDDK_) || defined(_NTHAL_) || defined(_NTOSP_)

// begin_ntifs begin_ntddk begin_wdm begin_nthal begin_ntosp

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
    );

// end_ntifs end_ntddk end_wdm end_nthal

NTKERNELAPI
VOID
FASTCALL
ExEnterCriticalRegionAndAcquireFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutexUnsafeAndLeaveCriticalRegion (
    __inout PFAST_MUTEX FastMutex
    );

// end_ntosp

#else

#define ExAcquireFastMutexUnsafe(FastMutex) xxAcquireFastMutexUnsafe(FastMutex)

#define ExReleaseFastMutexUnsafe(FastMutex) xxReleaseFastMutexUnsafe(FastMutex)

#endif

#if defined(_NTDRIVER_) || defined(_NTIFS_) || defined(_NTDDK_) || defined(_NTOSP_) || (defined(_X86_) && !defined(_APIC_TPR_))

// begin_ntifs begin_ntddk begin_wdm begin_nthal begin_ntosp

#if defined(_NTHAL_) && defined(_X86_)

NTKERNELAPI
VOID
FASTCALL
ExiAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExiReleaseFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExiTryToAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

#define ExAcquireFastMutex(FastMutex) ExiAcquireFastMutex(FastMutex)

#define ExReleaseFastMutex(FastMutex) ExiReleaseFastMutex(FastMutex)

#define ExTryToAcquireFastMutex(FastMutex) ExiTryToAcquireFastMutex(FastMutex)


#else

NTKERNELAPI
VOID
FASTCALL
ExAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
    );

#endif

// end_ntifs end_ntddk end_wdm end_nthal end_ntosp

#else

#define ExAcquireFastMutex(FastMutex) xxAcquireFastMutex(FastMutex)

#define ExReleaseFastMutex(FastMutex) xxReleaseFastMutex(FastMutex)

#define ExTryToAcquireFastMutex(FastMutex) xxTryToAcquireFastMutex(FastMutex)

#endif

#if defined (_X86_)

#define ExIsFastMutexOwned(_FastMutex) ((_FastMutex)->Count != 1)

#else

#define ExIsFastMutexOwned(_FastMutex) (((_FastMutex)->Count&FM_LOCK_BIT) == 0)

#endif

//
// Interlocked support routine definitions.
//
// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntndis begin_ntosp
//

#if defined(_WIN64)

#define ExInterlockedAddLargeStatistic(Addend, Increment)                   \
    (VOID) InterlockedAdd64(&(Addend)->QuadPart, Increment)

#else

#ifdef __cplusplus
extern "C" {
#endif

LONG
_InterlockedAddLargeStatistic (
    IN PLONGLONG Addend,
    IN ULONG Increment
    );

#ifdef __cplusplus
}
#endif

#pragma intrinsic (_InterlockedAddLargeStatistic)

#define ExInterlockedAddLargeStatistic(Addend,Increment)                     \
    (VOID) _InterlockedAddLargeStatistic ((PLONGLONG)&(Addend)->QuadPart, Increment)

#endif

// end_ntndis

NTKERNELAPI
LARGE_INTEGER
ExInterlockedAddLargeInteger (
    __inout PLARGE_INTEGER Addend,
    __in LARGE_INTEGER Increment,
    __inout PKSPIN_LOCK Lock
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

#if defined(NT_UP) && !defined(_NTHAL_) && !defined(_NTDDK_) && !defined(_NTIFS_)

#undef ExInterlockedAddUlong
#define ExInterlockedAddUlong(x, y, z) InterlockedExchangeAdd((PLONG)(x), (LONG)(y))

#else

// begin_wdm begin_ntddk begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
ULONG
FASTCALL
ExInterlockedAddUlong (
    __inout PULONG Addend,
    __in ULONG Increment,
    __inout PKSPIN_LOCK Lock
    );

// end_wdm end_ntddk end_nthal end_ntifs end_ntosp

#endif

// begin_wdm begin_ntddk begin_nthal begin_ntifs begin_ntosp

#if defined(_AMD64_)

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    InterlockedCompareExchange64(Destination, *(Exchange), *(Comperand))

#else

#define ExInterlockedCompareExchange64(Destination, Exchange, Comperand, Lock) \
    ExfInterlockedCompareExchange64(Destination, Exchange, Comperand)

#endif

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertHeadList (
    __inout PLIST_ENTRY ListHead,
    __inout PLIST_ENTRY ListEntry,
    __inout PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedInsertTailList (
    __inout PLIST_ENTRY ListHead,
    __inout PLIST_ENTRY ListEntry,
    __inout PKSPIN_LOCK Lock
    );

NTKERNELAPI
PLIST_ENTRY
FASTCALL
ExInterlockedRemoveHeadList (
    __inout PLIST_ENTRY ListHead,
    __inout PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPopEntryList (
    __inout PSINGLE_LIST_ENTRY ListHead,
    __inout PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSINGLE_LIST_ENTRY
FASTCALL
ExInterlockedPushEntryList (
    __inout PSINGLE_LIST_ENTRY ListHead,
    __inout PSINGLE_LIST_ENTRY ListEntry,
    __inout PKSPIN_LOCK Lock
    );

// end_wdm end_ntddk end_nthal end_ntifs end_ntosp
//
// Define non-blocking interlocked queue functions.
//
// A non-blocking queue is a singly link list of queue entries with a
// head pointer and a tail pointer. The head and tail pointers use
// sequenced pointers as do next links in the entries themselves. The
// queueing discipline is FIFO. New entries are inserted at the tail
// of the list and current entries are removed from the front of the
// list.
//
// Non-blocking queues require a descriptor for each entry in the queue.
// A descriptor consists of a sequenced next pointer and a PVOID data
// value. Descriptors for a queue must be preallocated and inserted in
// an SLIST before calling the function to initialize a non-blocking
// queue header. The SLIST should have as many entries as required for
// the respective queue.
//

typedef struct _NBQUEUE_BLOCK {
    ULONG64 Next;
    ULONG64 Data;
} NBQUEUE_BLOCK, *PNBQUEUE_BLOCK;

PVOID
ExInitializeNBQueueHead (
    IN PSLIST_HEADER SlistHead
    );

BOOLEAN
ExInsertTailNBQueue (
    IN PVOID Header,
    IN ULONG64 Value
    );

BOOLEAN
ExRemoveHeadNBQueue (
    IN PVOID Header,
    OUT PULONG64 Value
    );

// begin_wdm begin_ntddk begin_nthal begin_ntifs begin_ntosp begin_ntndis
//
// Define interlocked sequenced listhead functions.
//
// A sequenced interlocked list is a singly linked list with a header that
// contains the current depth and a sequence number. Each time an entry is
// inserted or removed from the list the depth is updated and the sequence
// number is incremented. This enables AMD64, IA64, and Pentium and later
// machines to insert and remove from the list without the use of spinlocks.
//

#if !defined(_WINBASE_)

/*++

Routine Description:

    This function initializes a sequenced singly linked listhead.

Arguments:

    SListHead - Supplies a pointer to a sequenced singly linked listhead.

Return Value:

    None.

--*/

#if defined(_WIN64) && (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
VOID
InitializeSListHead (
    __out PSLIST_HEADER SListHead
    );

#else

__inline
VOID
InitializeSListHead (
    __out PSLIST_HEADER SListHead
    )

{

#ifdef _WIN64

    //
    // Slist headers must be 16 byte aligned.
    //

    if ((ULONG_PTR) SListHead & 0x0f) {

        DbgPrint( "InitializeSListHead unaligned Slist header.  Address = %p, Caller = %p\n", SListHead, _ReturnAddress());
        RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
    }

#endif

    SListHead->Alignment = 0;

    return;
}

#endif

#endif // !defined(_WINBASE_)

#define ExInitializeSListHead InitializeSListHead

PSLIST_ENTRY
FirstEntrySList (
    IN const SLIST_HEADER *SListHead
    );

/*++

Routine Description:

    This function queries the current number of entries contained in a
    sequenced single linked list.

Arguments:

    SListHead - Supplies a pointer to the sequenced listhead which is
        be queried.

Return Value:

    The current number of entries in the sequenced singly linked list is
    returned as the function value.

--*/

#if defined(_WIN64)

#if (defined(_NTDRIVER_) || defined(_NTDDK_) || defined(_NTIFS_) || defined(_NTHAL_) || defined(_NTOSP_))

NTKERNELAPI
USHORT
ExQueryDepthSList (
    __in PSLIST_HEADER SListHead
    );

#else

__inline
USHORT
ExQueryDepthSList (
    __in PSLIST_HEADER SListHead
    )

{

    return (USHORT)(SListHead->Alignment & 0xffff);
}

#endif

#else

#define ExQueryDepthSList(_listhead_) (_listhead_)->Depth

#endif

#if defined(_WIN64)

#define ExInterlockedPopEntrySList(Head, Lock) \
    ExpInterlockedPopEntrySList(Head)

#define ExInterlockedPushEntrySList(Head, Entry, Lock) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define ExInterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#if !defined(_WINBASE_)

#define InterlockedPopEntrySList(Head) \
    ExpInterlockedPopEntrySList(Head)

#define InterlockedPushEntrySList(Head, Entry) \
    ExpInterlockedPushEntrySList(Head, Entry)

#define InterlockedFlushSList(Head) \
    ExpInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif // !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPopEntrySList (
    __inout PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedPushEntrySList (
    __inout PSLIST_HEADER ListHead,
    __inout PSLIST_ENTRY ListEntry
    );

NTKERNELAPI
PSLIST_ENTRY
ExpInterlockedFlushSList (
    __inout PSLIST_HEADER ListHead
    );

#else

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPopEntrySList (
    __inout PSLIST_HEADER ListHead,
    __inout PKSPIN_LOCK Lock
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedPushEntrySList (
    __inout PSLIST_HEADER ListHead,
    __inout PSLIST_ENTRY ListEntry,
    __inout PKSPIN_LOCK Lock
    );

#else

#define ExInterlockedPopEntrySList(ListHead, Lock) \
    InterlockedPopEntrySList(ListHead)

#define ExInterlockedPushEntrySList(ListHead, ListEntry, Lock) \
    InterlockedPushEntrySList(ListHead, ListEntry)

#endif

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
ExInterlockedFlushSList (
    __inout PSLIST_HEADER ListHead
    );

#if !defined(_WINBASE_)

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPopEntrySList (
    __inout PSLIST_HEADER ListHead
    );

NTKERNELAPI
PSLIST_ENTRY
FASTCALL
InterlockedPushEntrySList (
    __inout PSLIST_HEADER ListHead,
    __inout PSLIST_ENTRY ListEntry
    );

#define InterlockedFlushSList(Head) \
    ExInterlockedFlushSList(Head)

#define QueryDepthSList(Head) \
    ExQueryDepthSList(Head)

#endif // !defined(_WINBASE_)

#endif // defined(_WIN64)

// end_ntddk end_wdm end_ntosp


PSLIST_ENTRY
FASTCALL
InterlockedPushListSList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY List,
    IN PSLIST_ENTRY ListEnd,
    IN ULONG Count
    );


//
// Define interlocked lookaside list structure and allocation functions.
//

VOID
ExAdjustLookasideDepth (
    VOID
    );

// begin_ntddk begin_wdm begin_ntosp

typedef
PVOID
(*PALLOCATE_FUNCTION) (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

typedef
VOID
(*PFREE_FUNCTION) (
    IN PVOID Buffer
    );

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _GENERAL_LOOKASIDE {

#else

typedef struct DECLSPEC_CACHEALIGN _GENERAL_LOOKASIDE {

#endif

    SLIST_HEADER ListHead;
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };

    ULONG TotalFrees;
    union {
        ULONG FreeMisses;
        ULONG FreeHits;
    };

    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    PALLOCATE_FUNCTION Allocate;
    PFREE_FUNCTION Free;
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };

    ULONG Future[2];
} GENERAL_LOOKASIDE, *PGENERAL_LOOKASIDE;

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_) || defined(_NDIS_))

typedef struct _NPAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _NPAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_)

    KSPIN_LOCK Lock__ObsoleteButDoNotDelete;

#endif

} NPAGED_LOOKASIDE_LIST, *PNPAGED_LOOKASIDE_LIST;

NTKERNELAPI
VOID
ExInitializeNPagedLookasideList (
    __out PNPAGED_LOOKASIDE_LIST Lookaside,
    __in_opt PALLOCATE_FUNCTION Allocate,
    __in_opt PFREE_FUNCTION Free,
    __in ULONG Flags,
    __in SIZE_T Size,
    __in ULONG Tag,
    __in USHORT Depth
    );

NTKERNELAPI
VOID
ExDeleteNPagedLookasideList (
    __inout PNPAGED_LOOKASIDE_LIST Lookaside
    );

__inline
PVOID
ExAllocateFromNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

/*++

Routine Description:

    This function removes (pops) the first entry from the specified
    nonpaged lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a nonpaged lookaside list structure.

Return Value:

    If an entry is removed from the specified lookaside list, then the
    address of the entry is returned as the function value. Otherwise,
    NULL is returned.

--*/

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead,
                                       &Lookaside->Lock__ObsoleteButDoNotDelete);


#else

    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);

#endif

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

__inline
VOID
ExFreeToNPagedLookasideList(
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PVOID Entry
    )

/*++

Routine Description:

    This function inserts (pushes) the specified entry into the specified
    nonpaged lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a nonpaged lookaside list structure.

    Entry - Supples a pointer to the entry that is inserted in the
        lookaside list.

Return Value:

    None.

--*/

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSLIST_ENTRY)Entry,
                                    &Lookaside->Lock__ObsoleteButDoNotDelete);

#else

        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);

#endif

    }
    return;
}

// end_ntndis

#if !defined(_WIN64) && (defined(_NTDDK_) || defined(_NTIFS_)  || defined(_NDIS_))

typedef struct _PAGED_LOOKASIDE_LIST {

#else

typedef struct DECLSPEC_CACHEALIGN _PAGED_LOOKASIDE_LIST {

#endif

    GENERAL_LOOKASIDE L;

#if !defined(_AMD64_)

    FAST_MUTEX Lock__ObsoleteButDoNotDelete;

#endif

} PAGED_LOOKASIDE_LIST, *PPAGED_LOOKASIDE_LIST;

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp
//
// N.B. Nonpaged lookaside list structures and pages lookaside list
//      structures MUST be the same size for the system itself. The
//      per-processor lookaside lists for small pool and I/O are
//      allocated with one allocation.
//

#if defined(_WIN64) || (!defined(_NTDDK_) && !defined(_NTIFS_) && !defined(_NDIS_))

C_ASSERT(sizeof(NPAGED_LOOKASIDE_LIST) == sizeof(PAGED_LOOKASIDE_LIST));

#endif

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
VOID
ExInitializePagedLookasideList (
    __out PPAGED_LOOKASIDE_LIST Lookaside,
    __in_opt PALLOCATE_FUNCTION Allocate,
    __in_opt PFREE_FUNCTION Free,
    __in ULONG Flags,
    __in SIZE_T Size,
    __in ULONG Tag,
    __in USHORT Depth
    );

NTKERNELAPI
VOID
ExDeletePagedLookasideList (
    __inout PPAGED_LOOKASIDE_LIST Lookaside
    );

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
PVOID
ExAllocateFromPagedLookasideList(
    __inout PPAGED_LOOKASIDE_LIST Lookaside
    );

#else

__inline
PVOID
ExAllocateFromPagedLookasideList(
    __inout PPAGED_LOOKASIDE_LIST Lookaside
    )

/*++

Routine Description:

    This function removes (pops) the first entry from the specified
    paged lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a paged lookaside list structure.

Return Value:

    If an entry is removed from the specified lookaside list, then the
    address of the entry is returned as the function value. Otherwise,
    NULL is returned.

--*/

{

    PVOID Entry;

    Lookaside->L.TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->L.ListHead);
    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
    }

    return Entry;
}

#endif

#if defined(_WIN2K_COMPAT_SLIST_USAGE) && defined(_X86_)

NTKERNELAPI
VOID
ExFreeToPagedLookasideList(
    __inout PPAGED_LOOKASIDE_LIST Lookaside,
    __in PVOID Entry
    );

#else

__inline
VOID
ExFreeToPagedLookasideList(
    __inout PPAGED_LOOKASIDE_LIST Lookaside,
    __in PVOID Entry
    )

/*++

Routine Description:

    This function inserts (pushes) the specified entry into the specified
    paged lookaside list.

Arguments:

    Lookaside - Supplies a pointer to a nonpaged lookaside list structure.

    Entry - Supples a pointer to the entry that is inserted in the
        lookaside list.

Return Value:

    None.

--*/

{

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {
        InterlockedPushEntrySList(&Lookaside->L.ListHead,
                                  (PSLIST_ENTRY)Entry);
    }

    return;
}

#endif

// end_ntddk end_nthal end_ntifs end_wdm end_ntosp

VOID
ExInitializeSystemLookasideList (
    IN PGENERAL_LOOKASIDE Lookaside,
    IN POOL_TYPE Type,
    IN ULONG Size,
    IN ULONG Tag,
    IN USHORT Depth,
    IN PLIST_ENTRY ListHead
    );

//
// Define per processor nonpage lookaside list structures.
//

typedef enum _PP_NPAGED_LOOKASIDE_NUMBER {
    LookasideSmallIrpList,
    LookasideLargeIrpList,
    LookasideMdlList,
    LookasideCreateInfoList,
    LookasideNameBufferList,
    LookasideTwilightList,
    LookasideCompletionList,
    LookasideMaximumList
} PP_NPAGED_LOOKASIDE_NUMBER, *PPP_NPAGED_LOOKASIDE_NUMBER;

#if !defined(_CROSS_PLATFORM_)

FORCEINLINE
PVOID
ExAllocateFromPPLookasideList (
    IN PP_NPAGED_LOOKASIDE_NUMBER Number
    )

/*++

Routine Description:

    This function removes (pops) the first entry from the specified per
    processor lookaside list.

    N.B. It is possible to context switch during the allocation from a
         per processor nonpaged lookaside list, but this should happen
         infrequently and should not aversely effect the benefits of
         per processor lookaside lists.

Arguments:

    Number - Supplies the per processor nonpaged lookaside list number.

Return Value:

    If an entry is removed from the specified lookaside list, then the
    address of the entry is returned as the function value. Otherwise,
    NULL is returned.

--*/

{

    PVOID Entry;
    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    ASSERT((Number >= 0) && (Number < LookasideMaximumList));

    //
    // Attempt to allocate from the per processor lookaside list.
    //

    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[Number].P;
    Lookaside->TotalAllocates += 1;
    Entry = InterlockedPopEntrySList(&Lookaside->ListHead);

    //
    // If the per processor allocation attempt failed, then attempt to
    // allocate from the system lookaside list.
    //

    if (Entry == NULL) {
        Lookaside->AllocateMisses += 1;
        Lookaside = Prcb->PPLookasideList[Number].L;
        Lookaside->TotalAllocates += 1;
        Entry = InterlockedPopEntrySList(&Lookaside->ListHead);
        if (Entry == NULL) {
            Lookaside->AllocateMisses += 1;
            Entry = (Lookaside->Allocate)(Lookaside->Type,
                                          Lookaside->Size,
                                          Lookaside->Tag);
        }
    }

    return Entry;
}

FORCEINLINE
VOID
ExFreeToPPLookasideList (
    IN PP_NPAGED_LOOKASIDE_NUMBER Number,
    IN PVOID Entry
    )

/*++

Routine Description:

    This function inserts (pushes) the specified entry into the specified
    per processor lookaside list.

    N.B. It is possible to context switch during the free of a per
         processor nonpaged lookaside list, but this should happen
         infrequently and should not aversely effect the benefits of
         per processor lookaside lists.

Arguments:

    Number - Supplies the per processor nonpaged lookaside list number.

    Entry - Supples a pointer to the entry that is inserted in the per
        processor lookaside list.

Return Value:

    None.

--*/

{

    PGENERAL_LOOKASIDE Lookaside;
    PKPRCB Prcb;

    ASSERT((Number >= 0) && (Number < LookasideMaximumList));

    //
    // If the current depth is less than of equal to the maximum depth, then
    // free the specified entry to the per processor lookaside list. Otherwise,
    // free the entry to the system lookaside list;
    //
    //

    Prcb = KeGetCurrentPrcb();
    Lookaside = Prcb->PPLookasideList[Number].P;
    Lookaside->TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
        Lookaside->FreeMisses += 1;
        Lookaside = Prcb->PPLookasideList[Number].L;
        Lookaside->TotalFrees += 1;
        if (ExQueryDepthSList(&Lookaside->ListHead) >= Lookaside->Depth) {
            Lookaside->FreeMisses += 1;
            (Lookaside->Free)(Entry);
            return;
        }
    }

    InterlockedPushEntrySList(&Lookaside->ListHead,
                              (PSLIST_ENTRY)Entry);

    return;
}

#endif

#if i386 && !FPO

NTSTATUS
ExQuerySystemBackTraceInformation(
    OUT PRTL_PROCESS_BACKTRACES BackTraceInformation,
    IN ULONG BackTraceInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTKERNELAPI
USHORT
ExGetPoolBackTraceIndex(
    __in PVOID P
    );

#endif // i386 && !FPO

NTKERNELAPI
NTSTATUS
ExLockUserBuffer(
    __inout_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in KPROCESSOR_MODE ProbeMode,
    __in LOCK_OPERATION LockMode,
    __deref_out PVOID *LockedBuffer,
    __deref_out PVOID *LockVariable
    );

NTKERNELAPI
VOID
ExUnlockUserBuffer(
    __inout PVOID LockVariable
    );

// begin_ntddk begin_wdm begin_ntifs

#if defined(_NTDDK_) || defined(_NTIFS_)

NTKERNELAPI
VOID
NTAPI
ProbeForRead (
    __in_bcount(Length) VOID *Address,
    __in SIZE_T Length,
    __in ULONG Alignment
    );

#endif

// begin_ntosp
//
// Raise status from kernel mode.
//

NTKERNELAPI
DECLSPEC_NORETURN
VOID
NTAPI
ExRaiseStatus (
    __in NTSTATUS Status
    );

// end_wdm

NTKERNELAPI
DECLSPEC_NORETURN
VOID
ExRaiseDatatypeMisalignment (
    VOID
    );

NTKERNELAPI
DECLSPEC_NORETURN
VOID
ExRaiseAccessViolation (
    VOID
    );

// end_ntddk end_ntifs
//
// Probe function definitions
//
// Probe for read functions.
//
//++
//
// VOID
// ProbeForRead (
//     IN PVOID Address,
//     IN ULONG Length,
//     IN ULONG Alignment
//     )
//
//--

#define ProbeForRead(Address, Length, Alignment) {                           \
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
           ((Alignment) == 4) || ((Alignment) == 8) ||                       \
           ((Alignment) == 16));                                             \
                                                                             \
    if ((Length) != 0) {                                                     \
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
            ExRaiseDatatypeMisalignment();                                   \
                                                                             \
        }                                                                    \
        if ((((ULONG_PTR)(Address) + (Length)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS) || \
            (((ULONG_PTR)(Address) + (Length)) < (ULONG_PTR)(Address))) {    \
            *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;              \
        }                                                                    \
    }                                                                        \
}

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForReadSmallStructure (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG Alignment
    )

/*++

Routine Description:

    Probes a structure for read access whose size is known at compile time.

    N.B. A NULL structure address is not allowed.

Arguments:

    Address - Supples a pointer to the structure.

    Size - Supplies the size of the structure.

    Alignment - Supplies the alignment of structure.

Return Value:

    None

--*/

{

    ASSERT((Alignment == 1) || (Alignment == 2) ||
           (Alignment == 4) || (Alignment == 8) ||
           (Alignment == 16));

    if ((Size == 0) || (Size >= 0x10000)) {

        ASSERT(0);

        ProbeForRead(Address, Size, Alignment);

    } else {
        if (((ULONG_PTR)Address & (Alignment - 1)) != 0) {
            ExRaiseDatatypeMisalignment();
        }

        if ((PUCHAR)Address >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {
            Address = (UCHAR * const)MM_USER_PROBE_ADDRESS;
        }

        _ReadWriteBarrier();
        *(volatile UCHAR *)Address;
    }
}

#else

#define ProbeForReadSmallStructure(Address, Size, Alignment) {               \
    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||                       \
           ((Alignment) == 4) || ((Alignment) == 8) ||                       \
           ((Alignment) == 16));                                             \
    if ((Size == 0) || (Size > 0x10000)) {                                   \
        ASSERT(0);                                                           \
        ProbeForRead(Address, Size, Alignment);                              \
    } else {                                                                 \
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {               \
            ExRaiseDatatypeMisalignment();                                   \
        }                                                                    \
        if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {      \
            *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;              \
        }                                                                    \
    }                                                                        \
}

#endif

//++
//
// BOOLEAN
// ProbeAndReadBoolean (
//     IN PBOOLEAN Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
BOOLEAN
ProbeAndReadBoolean (
    PBOOLEAN Address
    )

{

    if (Address >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {
        Address = (BOOLEAN * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile BOOLEAN *)Address);
}

#else

#define ProbeAndReadBoolean(Address) \
    (((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS) : (*(volatile BOOLEAN *)(Address)))

#endif

//++
//
// CHAR
// ProbeAndReadChar (
//     IN PCHAR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
CHAR
ProbeAndReadChar (
    PCHAR Address
    )

{

    if (Address >= (CHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (CHAR * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile CHAR *)Address);
}

#else

#define ProbeAndReadChar(Address) \
    (((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CHAR * const)MM_USER_PROBE_ADDRESS) : (*(volatile CHAR *)(Address)))

#endif

//++
//
// UCHAR
// ProbeAndReadUchar (
//     IN PUCHAR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
UCHAR
ProbeAndReadUchar (
    PUCHAR Address
    )

{

    if (Address >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (UCHAR * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile UCHAR *)Address);
}

#else

#define ProbeAndReadUchar(Address) \
    (((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UCHAR * const)MM_USER_PROBE_ADDRESS) : (*(volatile UCHAR *)(Address)))

#endif

//++
//
// SHORT
// ProbeAndReadShort(
//     IN PSHORT Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
SHORT
ProbeAndReadShort (
    PSHORT Address
    )

{

    if (Address >= (SHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (SHORT * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile SHORT *)Address);
}

#else

#define ProbeAndReadShort(Address) \
    (((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile SHORT * const)MM_USER_PROBE_ADDRESS) : (*(volatile SHORT *)(Address)))

#endif

//++
//
// USHORT
// ProbeAndReadUshort (
//     IN PUSHORT Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
USHORT
ProbeAndReadUshort (
    PUSHORT Address
    )

{

    if (Address >= (USHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (USHORT * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile USHORT *)Address);
}

#else

#define ProbeAndReadUshort(Address) \
    (((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile USHORT * const)MM_USER_PROBE_ADDRESS) : (*(volatile USHORT *)(Address)))

#endif

//++
//
// HANDLE
// ProbeAndReadHandle (
//     IN PHANDLE Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
HANDLE
ProbeAndReadHandle (
    PHANDLE Address
    )

{

    if (Address >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {
        Address = (HANDLE * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile HANDLE *)Address);
}

#else

#define ProbeAndReadHandle(Address) \
    (((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile HANDLE * const)MM_USER_PROBE_ADDRESS) : (*(volatile HANDLE *)(Address)))

#endif

//++
//
// PVOID
// ProbeAndReadPointer (
//     IN PVOID *Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
PVOID
ProbeAndReadPointer (
    PVOID *Address
    )

{

    if (Address >= (PVOID * const)MM_USER_PROBE_ADDRESS) {
        Address = (PVOID * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile PVOID *)Address);
}

#else

#define ProbeAndReadPointer(Address) \
    (((Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile PVOID * const)MM_USER_PROBE_ADDRESS) : (*(volatile PVOID *)(Address)))

#endif

//++
//
// LONG
// ProbeAndReadLong (
//     IN PLONG Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
LONG
ProbeAndReadLong (
    PLONG Address
    )

{

    if (Address >= (LONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (LONG * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile LONG *)Address);
}

#else

#define ProbeAndReadLong(Address) \
    (((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile LONG * const)MM_USER_PROBE_ADDRESS) : (*(volatile LONG *)(Address)))

#endif

//++
//
// ULONG
// ProbeAndReadUlong (
//     IN PULONG Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
ULONG
ProbeAndReadUlong (
    PULONG Address
    )

{

    if (Address >= (ULONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile ULONG *)Address);
}

#else

#define ProbeAndReadUlong(Address) \
    (((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULONG * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULONG *)(Address)))

#endif

//++
//
// ULONG_PTR
// ProbeAndReadUlong_ptr (
//     IN PULONG_PTR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
ULONG_PTR
ProbeAndReadUlong_ptr (
    PULONG_PTR Address
    )

{

    if (Address >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG_PTR * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile ULONG_PTR *)Address);
}

#else

#define ProbeAndReadUlong_ptr(Address) \
    (((Address) >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULONG_PTR * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULONG_PTR *)(Address)))

#endif

//++
//
// QUAD
// ProbeAndReadQuad (
//     IN PQUAD Address
//     )
//
//--

#if defined(_AMD64_) && !defined(__cplusplus)

FORCEINLINE
QUAD
ProbeAndReadQuad (
    PQUAD Address
    )

{

    if (Address >= (QUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (QUAD * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile QUAD *)Address);
}

#else

#define ProbeAndReadQuad(Address) \
    (((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile QUAD * const)MM_USER_PROBE_ADDRESS) : (*(volatile QUAD *)(Address)))

#endif

//++
//
// UQUAD
// ProbeAndReadUquad (
//     IN PUQUAD Address
//     )
//
//--

#if defined(_AMD64_) && !defined(__cplusplus)

FORCEINLINE
UQUAD
ProbeAndReadUquad (
    PUQUAD Address
    )

{

    if (Address >= (UQUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (UQUAD * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile UQUAD *)Address);
}

#else

#define ProbeAndReadUquad(Address) \
    (((Address) >= (UQUAD * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UQUAD * const)MM_USER_PROBE_ADDRESS) : (*(volatile UQUAD *)(Address)))

#endif

//++
//
// LARGE_INTEGER
// ProbeAndReadLargeInteger(
//     IN PLARGE_INTEGER Source
//     )
//
//--

#if defined(_AMD64_) && !defined(__cplusplus)

FORCEINLINE
LARGE_INTEGER
ProbeAndReadLargeInteger (
    PLARGE_INTEGER Address
    )

{

    if (Address >= (LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) {
        Address = (LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile LARGE_INTEGER *)Address);
}

#else

#define ProbeAndReadLargeInteger(Source)  \
    (((Source) >= (LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile LARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) : (*(volatile LARGE_INTEGER *)(Source)))

#endif

//++
//
// ULARGE_INTEGER
// ProbeAndReadUlargeInteger (
//     IN PULARGE_INTEGER Source
//     )
//
//--

#if defined(_AMD64_) && !defined(__cplusplus)

FORCEINLINE
ULARGE_INTEGER
ProbeAndReadUlargeInteger (
    PULARGE_INTEGER Address
    )

{

    if (Address >= (ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    return *((volatile ULARGE_INTEGER *)Address);
}

#else

#define ProbeAndReadUlargeInteger(Source)  \
    (((Source) >= (ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile ULARGE_INTEGER * const)MM_USER_PROBE_ADDRESS) : (*(volatile ULARGE_INTEGER *)(Source)))

#endif

//++
//
// VOID
// ProbeAndReadUnicodeStringEx (
//     OUT PUNICODE_STRING Destination,
//     IN PUNICODE_STRING Source
//     )
//
//--

#if !defined(__cplusplus)

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndReadUnicodeStringEx (
    OUT PUNICODE_STRING Destination,
    IN PUNICODE_STRING Source
    )

{

    if (Source >= (UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) {
        Source = (UNICODE_STRING * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    *Destination = *((volatile UNICODE_STRING *)Source);
    return;
}

#else

#define ProbeAndReadUnicodeStringEx(Dst, Src) *(Dst) = ProbeAndReadUnicodeString(Src)

#endif

#endif

//++
//
// UNICODE_STRING
// ProbeAndReadUnicodeString (
//     IN PUNICODE_STRING Source
//     )
//
//--

#if !defined(__cplusplus)

#define ProbeAndReadUnicodeString(Source)  \
    (((Source) >= (UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile UNICODE_STRING * const)MM_USER_PROBE_ADDRESS) : (*(volatile UNICODE_STRING *)(Source)))

#endif

//++
//
// VOID
// ProbeAndReadStructureEx (
//     IN P<STRUCTURE> Destination,
//     IN P<STRUCTURE> Source,
//     <STRUCTURE>
//     )
//
//--

#if defined(_AMD64_)

#define ProbeAndReadStructureEx(Dst, Src, STRUCTURE)                         \
    ProbeAndReadStructureWorker(&(Dst), Src, sizeof(STRUCTURE))

FORCEINLINE
VOID
ProbeAndReadStructureWorker (
    IN PVOID Destination,
    IN PVOID Source,
    IN SIZE_T Size
    )

{

    if (Source >= (VOID * const)MM_USER_PROBE_ADDRESS) {
        Source = (VOID * const)MM_USER_PROBE_ADDRESS;
    }

    _ReadWriteBarrier();
    memcpy(Destination, Source, Size);
    return;
}

#else

#define ProbeAndReadStructureEx(Dst, Src, STRUCTURE)                         \
    (Dst) = ProbeAndReadStructure(Src, STRUCTURE)

#endif

//++
//
// <STRUCTURE>
// ProbeAndReadStructure (
//     IN P<STRUCTURE> Source
//     <STRUCTURE>
//     )
//
//--

#define ProbeAndReadStructure(Source, STRUCTURE)                             \
    (((Source) >= (STRUCTURE * const)MM_USER_PROBE_ADDRESS) ?                \
        (*(STRUCTURE * const)MM_USER_PROBE_ADDRESS) : (*(STRUCTURE *)(Source)))

//
// Probe for write functions definitions.
//
//++
//
// VOID
// ProbeForWriteBoolean (
//     IN PBOOLEAN Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteBoolean (
    IN PBOOLEAN Address
    )

{

    if (Address >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {
        Address = (BOOLEAN * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile BOOLEAN *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteBoolean(Address) {                                      \
    if ((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {               \
        *(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS = 0;                \
    }                                                                        \
                                                                             \
    *(volatile BOOLEAN *)(Address) = *(volatile BOOLEAN *)(Address);         \
}

#endif

//++
//
// VOID
// ProbeForWriteChar (
//     IN PCHAR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteChar (
    IN PCHAR Address
    )

{

    if (Address >= (CHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (CHAR * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile CHAR *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteChar(Address) {                                         \
    if ((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile CHAR * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(volatile CHAR *)(Address) = *(volatile CHAR *)(Address);               \
}

#endif

//++
//
// VOID
// ProbeForWriteUchar (
//     IN PUCHAR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteUchar (
    IN PUCHAR Address
    )

{

    if (Address >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (UCHAR * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile UCHAR *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteUchar(Address) {                                        \
    if ((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile UCHAR *)(Address) = *(volatile UCHAR *)(Address);             \
}

#endif

//++
//
// VOID
// ProbeForWriteIoStatus (
//     IN PIO_STATUS_BLOCK Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteIoStatus (
    IN PIO_STATUS_BLOCK Address
    )

{

    if (Address >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {
        Address = (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS;
    }

    ((volatile IO_STATUS_BLOCK *)Address)->Status = Address->Status;
    ((volatile IO_STATUS_BLOCK *)Address)->Information = Address->Information;
    return;
}

#else

#define ProbeForWriteIoStatus(Address) {                                     \
    if ((Address) >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {       \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile IO_STATUS_BLOCK *)(Address) = *(volatile IO_STATUS_BLOCK *)(Address); \
}

#endif

#if defined(_WIN64)

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteIoStatusEx (
    IN PIO_STATUS_BLOCK Address,
    IN ULONG64 Cookie
    )

{

    if (Address >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {
        Address = (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS;
    }

    if ((Cookie & 1) != 0) {
        ((volatile IO_STATUS_BLOCK32 *)Address)->Status =
                                    ((IO_STATUS_BLOCK32 *)Address)->Status;

        ((volatile IO_STATUS_BLOCK32 *)Address)->Information =
                                    ((IO_STATUS_BLOCK32 *)Address)->Information;

    } else {
        ((volatile IO_STATUS_BLOCK *)Address)->Status = Address->Status;
        ((volatile IO_STATUS_BLOCK *)Address)->Information = Address->Information;
    }

    return;
}

#else

#define ProbeForWriteIoStatusEx(Address, Cookie) {                                          \
    if ((Address) >= (IO_STATUS_BLOCK * const)MM_USER_PROBE_ADDRESS) {                      \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                                 \
    }                                                                                       \
    if ((ULONG_PTR)(Cookie) & (ULONG)1) {                                                   \
        *(volatile IO_STATUS_BLOCK32 *)(Address) = *(volatile IO_STATUS_BLOCK32 *)(Address);\
    } else {                                                                                \
        *(volatile IO_STATUS_BLOCK *)(Address) = *(volatile IO_STATUS_BLOCK *)(Address);    \
    }                                                                                       \
}

#endif

#else

#define ProbeForWriteIoStatusEx(Address, Cookie) ProbeForWriteIoStatus(Address)

#endif

//++
//
// VOID
// ProbeForWriteShort (
//     IN PSHORT Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteShort (
    IN PSHORT Address
    )

{

    if (Address >= (SHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (SHORT * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile SHORT *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteShort(Address) {                                        \
    if ((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile SHORT * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile SHORT *)(Address) = *(volatile SHORT *)(Address);             \
}

#endif

//++
//
// VOID
// ProbeForWriteUshort (
//     IN PUSHORT Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteUshort (
    IN PUSHORT Address
    )

{

    if (Address >= (USHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (USHORT * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile USHORT *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteUshort(Address) {                                       \
    if ((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile USHORT * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile USHORT *)(Address) = *(volatile USHORT *)(Address);           \
}

#endif

//++
//
// VOID
// ProbeForWriteHandle (
//     IN PHANDLE Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteHandle (
    IN PHANDLE Address
    )

{

    if (Address >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {
        Address = (HANDLE * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile HANDLE *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteHandle(Address) {                                       \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile HANDLE *)(Address) = *(volatile HANDLE *)(Address);           \
}

#endif

//++
//
// VOID
// ProbeAndZeroHandle (
//     IN PHANDLE Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndZeroHandle (
    IN PHANDLE Address
    )

{

    if (Address >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {
        Address = (HANDLE * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile HANDLE *)Address) = 0;
    return;
}

#else

#define ProbeAndZeroHandle(Address) {                                        \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(volatile HANDLE *)(Address) = 0;                                       \
}

#endif

//++
//
// VOID
// ProbeForWritePointer (
//     IN PVOID *Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWritePointer (
    IN PVOID *Address
    )

{

    if (Address >= (PVOID * const)MM_USER_PROBE_ADDRESS) {
        Address = (PVOID * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile PVOID *)Address) = *Address;
    return;
}

#else

#define ProbeForWritePointer(Address) {                                      \
    if ((PVOID *)(Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) {        \
        *(volatile PVOID * const)MM_USER_PROBE_ADDRESS = NULL;               \
    }                                                                        \
                                                                             \
    *(volatile PVOID *)(Address) = *(volatile PVOID *)(Address);             \
}

#endif

//++
//
// VOID
// ProbeAndNullPointer (
//     IN PVOID *Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndNullPointer (
    IN PVOID *Address
    )

{

    if (Address >= (PVOID * const)MM_USER_PROBE_ADDRESS) {
        Address = (PVOID * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile PVOID *)Address) = NULL;
    return;
}

#else

#define ProbeAndNullPointer(Address) {                                       \
    if ((PVOID *)(Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) {        \
        *(volatile PVOID * const)MM_USER_PROBE_ADDRESS = NULL;               \
    }                                                                        \
                                                                             \
    *(volatile PVOID *)(Address) = NULL;                                     \
}

#endif

//++
//
// VOID
// ProbeForWriteLong (
//     IN PLONG Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteLong (
    IN PLONG Address
    )

{

    if (Address >= (LONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (LONG * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile LONG *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteLong(Address) {                                        \
    if ((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                       \
                                                                            \
    *(volatile LONG *)(Address) = *(volatile LONG *)(Address);              \
}

#endif

//++
//
// VOID
// ProbeForWriteUlong (
//     IN PULONG Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteUlong (
    IN PULONG Address
    )

{

    if (Address >= (ULONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile ULONG *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteUlong(Address) {                                        \
    if ((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile ULONG *)(Address) = *(volatile ULONG *)(Address);             \
}

#endif

//++
//
// VOID
// ProbeForWriteUlongAligned32 (
//     IN PULONG Address
//     )
//
//--

FORCEINLINE
VOID
ProbeForWriteUlongAligned32 (
    IN PULONG Address
    )

{

    if (((ULONG_PTR)Address & (sizeof(ULONG) - 1)) != 0) {
        ExRaiseDatatypeMisalignment();
    }

    ProbeForWriteUlong(Address);
    return;
}

//++
//
// VOID
// ProbeForWriteUlong_ptr (
//     IN PULONG_PTR Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteUlong_ptr (
    IN PULONG_PTR Address
    )

{

    if (Address >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG_PTR * const)MM_USER_PROBE_ADDRESS;
    }

    *((volatile ULONG_PTR *)Address) = *Address;
    return;
}

#else

#define ProbeForWriteUlong_ptr(Address) {                                    \
    if ((Address) >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {             \
        *(volatile ULONG_PTR * const)MM_USER_PROBE_ADDRESS = 0;              \
    }                                                                        \
                                                                             \
    *(volatile ULONG_PTR *)(Address) = *(volatile ULONG_PTR *)(Address);     \
}

#endif

//++
//
// VOID
// ProbeForWriteQuad (
//     IN PQUAD Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteQuad (
    IN PQUAD Address
    )

{

    if (Address >= (QUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (QUAD * const)MM_USER_PROBE_ADDRESS;
    }

    ((volatile QUAD *)Address)->UseThisFieldToCopy = Address->UseThisFieldToCopy;
    return;
}

#else

#define ProbeForWriteQuad(Address) {                                         \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(volatile QUAD *)(Address) = *(volatile QUAD *)(Address);               \
}

#endif

//++
//
// VOID
// ProbeForWriteUquad (
//     IN PUQUAD Address
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeForWriteUquad (
    IN PUQUAD Address
    )

{

    if (Address >= (UQUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (UQUAD * const)MM_USER_PROBE_ADDRESS;
    }

    ((volatile UQUAD *)Address)->UseThisFieldToCopy = Address->UseThisFieldToCopy;
    return;
}

#else

#define ProbeForWriteUquad(Address) {                                        \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile UQUAD *)(Address) = *(volatile UQUAD *)(Address);             \
}

#endif

//
// Probe and write functions definitions.
//
//++
//
// VOID
// ProbeAndWriteBoolean (
//     IN PBOOLEAN Address,
//     IN BOOLEAN Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteBoolean (
    IN PBOOLEAN Address,
    IN BOOLEAN Value
    )

{

    if (Address >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {
        Address = (BOOLEAN * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteBoolean(Address, Value) {                               \
    if ((Address) >= (BOOLEAN * const)MM_USER_PROBE_ADDRESS) {               \
        *(volatile BOOLEAN * const)MM_USER_PROBE_ADDRESS = 0;                \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteChar (
//     IN PCHAR Address,
//     IN CHAR Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteChar (
    IN PCHAR Address,
    IN CHAR Value
    )

{

    if (Address >= (CHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (CHAR * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteChar(Address, Value) {                                  \
    if ((Address) >= (CHAR * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile CHAR * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteUchar (
//     IN PUCHAR Address,
//     IN UCHAR Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteUchar (
    IN PUCHAR Address,
    IN UCHAR Value
    )

{

    if (Address >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {
        Address = (UCHAR * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteUchar(Address, Value) {                                 \
    if ((Address) >= (UCHAR * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteShort (
//     IN PSHORT Address,
//     IN SHORT Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteShort (
    IN PSHORT Address,
    IN SHORT Value
    )

{

    if (Address >= (SHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (SHORT * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteShort(Address, Value) {                                 \
    if ((Address) >= (SHORT * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile SHORT * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteUshort (
//     IN PUSHORT Address,
//     IN USHORT Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteUshort (
    IN PUSHORT Address,
    IN USHORT Value
    )

{

    if (Address >= (USHORT * const)MM_USER_PROBE_ADDRESS) {
        Address = (USHORT * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteUshort(Address, Value) {                                \
    if ((Address) >= (USHORT * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile USHORT * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteHandle (
//     IN PHANDLE Address,
//     IN HANDLE Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteHandle (
    IN PHANDLE Address,
    IN HANDLE Value
    )

{

    if (Address >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {
        Address = (HANDLE * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteHandle(Address, Value) {                                \
    if ((Address) >= (HANDLE * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile HANDLE * const)MM_USER_PROBE_ADDRESS = 0;                 \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteLong (
//     IN PLONG Address,
//     IN LONG Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteLong (
    IN PLONG Address,
    IN LONG Value
    )

{

    if (Address >= (LONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (LONG * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteLong(Address, Value) {                                  \
    if ((Address) >= (LONG * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteUlong (
//     IN PULONG Address,
//     IN ULONG Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteUlong (
    IN PULONG Address,
    IN ULONG Value
    )

{

    if (Address >= (ULONG * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteUlong(Address, Value) {                                 \
    if ((Address) >= (ULONG * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteUlong_ptr (
//     IN PULONG_PTR Address,
//     IN ULONG_PTR Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteUlong_ptr (
    IN PULONG_PTR Address,
    IN ULONG_PTR Value
    )

{

    if (Address >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {
        Address = (ULONG_PTR * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWriteUlong_ptr(Address, Value) {                             \
    if ((Address) >= (ULONG_PTR * const)MM_USER_PROBE_ADDRESS) {             \
        *(volatile ULONG_PTR * const)MM_USER_PROBE_ADDRESS = 0;              \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWritePointer (
//     IN PVOID *Address,
//     IN PVOID Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWritePointer (
    IN PVOID *Address,
    IN PVOID Value
    )

{

    if (Address >= (PVOID * const)MM_USER_PROBE_ADDRESS) {
        Address = (PVOID * const)MM_USER_PROBE_ADDRESS;
    }

    *Address = Value;
    return;
}

#else

#define ProbeAndWritePointer(Address, Value) {                               \
    if ((Address) >= (PVOID * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteQuad (
//     IN PQUAD Address,
//     IN QUAD Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteQuad (
    IN PQUAD Address,
    IN QUAD Value
    )

{

    if (Address >= (QUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (QUAD * const)MM_USER_PROBE_ADDRESS;
    }

    Address->UseThisFieldToCopy = Value.UseThisFieldToCopy;
    return;
}

#else

#define ProbeAndWriteQuad(Address, Value) {                                  \
    if ((Address) >= (QUAD * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile LONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteUquad (
//     IN PUQUAD Address,
//     IN UQUAD Value
//     )
//
//--

#if defined(_AMD64_)

FORCEINLINE
VOID
ProbeAndWriteUquad (
    IN PUQUAD Address,
    IN UQUAD Value
    )

{

    if (Address >= (UQUAD * const)MM_USER_PROBE_ADDRESS) {
        Address = (UQUAD * const)MM_USER_PROBE_ADDRESS;
    }

    Address->UseThisFieldToCopy = Value.UseThisFieldToCopy;
    return;
}

#else

#define ProbeAndWriteUquad(Address, Value) {                                 \
    if ((Address) >= (UQUAD * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

//++
//
// VOID
// ProbeAndWriteStructure (
//     IN P<STRUCTURE> Address,
//     IN <STRUCTURE> Value,
//     <STRUCTURE>
//     )
//
//--

#if defined(_AMD64_)

#define ProbeAndWriteStructure(Address, Value, STRUCTURE)                    \
    ProbeAndWriteStructureWorker(Address, &(Value), sizeof(STRUCTURE))

FORCEINLINE
VOID
ProbeAndWriteStructureWorker (
    IN PVOID Address,
    IN PVOID Value,
    IN SIZE_T Size
    )

{

    if (Address >= (VOID * const)MM_USER_PROBE_ADDRESS) {
        Address = (VOID * const)MM_USER_PROBE_ADDRESS;
    }

    memcpy(Address, Value, Size);
    return;
}

#else

#define ProbeAndWriteStructure(Address, Value, STRUCTURE) {                  \
    if ((STRUCTURE * const)(Address) >= (STRUCTURE * const)MM_USER_PROBE_ADDRESS) { \
        *((volatile UCHAR * const)MM_USER_PROBE_ADDRESS) = 0;                \
    }                                                                        \
                                                                             \
    *(Address) = (Value);                                                    \
}

#endif

// begin_ntifs begin_ntddk begin_wdm
//
// Common probe for write functions.
//

NTKERNELAPI
VOID
NTAPI
ProbeForWrite (
    __inout_bcount(Length) PVOID Address,
    __in SIZE_T Length,
    __in ULONG Alignment
    );

// end_ntifs end_ntddk end_wdm end_ntosp

//
// Timer Rundown
//

NTKERNELAPI
VOID
ExTimerRundown (
    VOID
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp
//
// Worker Thread
//

typedef enum _WORK_QUEUE_TYPE {
    CriticalWorkQueue,
    DelayedWorkQueue,
    HyperCriticalWorkQueue,
    MaximumWorkQueue
} WORK_QUEUE_TYPE;

typedef
VOID
(*PWORKER_THREAD_ROUTINE)(
    IN PVOID Parameter
    );

typedef struct _WORK_QUEUE_ITEM {
    LIST_ENTRY List;
    PWORKER_THREAD_ROUTINE WorkerRoutine;
    PVOID Parameter;
} WORK_QUEUE_ITEM, *PWORK_QUEUE_ITEM;

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeWorkItem)    // Use IoAllocateWorkItem
#endif
#define ExInitializeWorkItem(Item, Routine, Context) \
    (Item)->WorkerRoutine = (Routine);               \
    (Item)->Parameter = (Context);                   \
    (Item)->List.Flink = NULL;

DECLSPEC_DEPRECATED_DDK                     // Use IoQueueWorkItem
NTKERNELAPI
VOID
ExQueueWorkItem(
    __inout PWORK_QUEUE_ITEM WorkItem,
    __in WORK_QUEUE_TYPE QueueType
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

VOID
ExSwapinWorkerThreads(
    IN BOOLEAN AllowSwap
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs begin_ntosp

NTKERNELAPI
BOOLEAN
ExIsProcessorFeaturePresent(
    __in ULONG ProcessorFeature
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

//
// QueueDisabled indicates that the queue is being shut down, and new
// workers should not join the queue.  WorkerCount indicates the total
// number of worker threads processing items in this queue.  These two
// pieces of information need to do a RMW together, so it's simpler to
// smush them together than to use a lock.
//

typedef union {
    struct {

#define EX_WORKER_QUEUE_DISABLED    0x80000000

        ULONG QueueDisabled :  1;

        //
        // MakeThreadsAsNecessary indicates whether this work queue is eligible
        // for dynamic creation of threads not just for deadlock detection,
        // but to ensure that the CPUs are all kept busy clearing any work
        // item backlog.
        //

        ULONG MakeThreadsAsNecessary : 1;

        ULONG WaitMode : 1;

        ULONG WorkerCount   : 29;
    };
    LONG QueueWorkerInfo;
} EX_QUEUE_WORKER_INFO;

typedef struct _EX_WORK_QUEUE {

    //
    // Queue objects that that are used to hold work queue entries and
    // synchronize worker thread activity.
    //

    KQUEUE WorkerQueue;

    //
    // Number of dynamic worker threads that have been created "on the fly"
    // as part of worker thread deadlock prevention
    //

    ULONG DynamicThreadCount;

    //
    // Count of the number of work items processed.
    //

    ULONG WorkItemsProcessed;

    //
    // Used for deadlock detection, WorkItemsProcessedLastPass equals the value
    // of WorkItemsProcessed the last time ExpDetectWorkerThreadDeadlock()
    // ran.
    //

    ULONG WorkItemsProcessedLastPass;

    //
    // QueueDepthLastPass is also part of the worker queue state snapshot
    // taken by ExpDetectWorkerThreadDeadlock().
    //

    ULONG QueueDepthLastPass;

    EX_QUEUE_WORKER_INFO Info;

} EX_WORK_QUEUE, *PEX_WORK_QUEUE;

extern EX_WORK_QUEUE ExWorkerQueue[];


// begin_ntddk begin_nthal begin_ntifs begin_ntosp
//
// Zone Allocation
//

typedef struct _ZONE_SEGMENT_HEADER {
    SINGLE_LIST_ENTRY SegmentList;
    PVOID Reserved;
} ZONE_SEGMENT_HEADER, *PZONE_SEGMENT_HEADER;

typedef struct _ZONE_HEADER {
    SINGLE_LIST_ENTRY FreeList;
    SINGLE_LIST_ENTRY SegmentList;
    ULONG BlockSize;
    ULONG TotalSegmentSize;
} ZONE_HEADER, *PZONE_HEADER;


DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInitializeZone(
    __out PZONE_HEADER Zone,
    __in ULONG BlockSize,
    __inout PVOID InitialSegment,
    __in ULONG InitialSegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExExtendZone(
    __inout PZONE_HEADER Zone,
    __inout PVOID Segment,
    __in ULONG SegmentSize
    );

DECLSPEC_DEPRECATED_DDK
NTKERNELAPI
NTSTATUS
ExInterlockedExtendZone(
    __inout PZONE_HEADER Zone,
    __inout PVOID Segment,
    __in ULONG SegmentSize,
    __inout PKSPIN_LOCK Lock
    );

//++
//
// PVOID
// ExAllocateFromZone(
//     IN PZONE_HEADER Zone
//     )
//
// Routine Description:
//
//     This routine removes an entry from the zone and returns a pointer to it.
//
// Arguments:
//
//     Zone - Pointer to the zone header controlling the storage from which the
//         entry is to be allocated.
//
// Return Value:
//
//     The function value is a pointer to the storage allocated from the zone.
//
//--
#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExAllocateFromZone)
#endif
#define ExAllocateFromZone(Zone) \
    (PVOID)((Zone)->FreeList.Next); \
    if ( (Zone)->FreeList.Next ) (Zone)->FreeList.Next = (Zone)->FreeList.Next->Next


//++
//
// PVOID
// ExFreeToZone(
//     IN PZONE_HEADER Zone,
//     IN PVOID Block
//     )
//
// Routine Description:
//
//     This routine places the specified block of storage back onto the free
//     list in the specified zone.
//
// Arguments:
//
//     Zone - Pointer to the zone header controlling the storage to which the
//         entry is to be inserted.
//
//     Block - Pointer to the block of storage to be freed back to the zone.
//
// Return Value:
//
//     Pointer to previous block of storage that was at the head of the free
//         list.  NULL implies the zone went from no available free blocks to
//         at least one free block.
//
//--

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExFreeToZone)
#endif
#define ExFreeToZone(Zone,Block)                                    \
    ( ((PSINGLE_LIST_ENTRY)(Block))->Next = (Zone)->FreeList.Next,  \
      (Zone)->FreeList.Next = ((PSINGLE_LIST_ENTRY)(Block)),        \
      ((PSINGLE_LIST_ENTRY)(Block))->Next                           \
    )

//++
//
// BOOLEAN
// ExIsFullZone(
//     IN PZONE_HEADER Zone
//     )
//
// Routine Description:
//
//     This routine determines if the specified zone is full or not.  A zone
//     is considered full if the free list is empty.
//
// Arguments:
//
//     Zone - Pointer to the zone header to be tested.
//
// Return Value:
//
//     TRUE if the zone is full and FALSE otherwise.
//
//--

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsFullZone)
#endif
#define ExIsFullZone(Zone) \
    ( (Zone)->FreeList.Next == (PSINGLE_LIST_ENTRY)NULL )

//++
//
// PVOID
// ExInterlockedAllocateFromZone(
//     IN PZONE_HEADER Zone,
//     IN PKSPIN_LOCK Lock
//     )
//
// Routine Description:
//
//     This routine removes an entry from the zone and returns a pointer to it.
//     The removal is performed with the specified lock owned for the sequence
//     to make it MP-safe.
//
// Arguments:
//
//     Zone - Pointer to the zone header controlling the storage from which the
//         entry is to be allocated.
//
//     Lock - Pointer to the spin lock which should be obtained before removing
//         the entry from the allocation list.  The lock is released before
//         returning to the caller.
//
// Return Value:
//
//     The function value is a pointer to the storage allocated from the zone.
//
//--

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedAllocateFromZone)
#endif
#define ExInterlockedAllocateFromZone(Zone,Lock) \
    (PVOID) ExInterlockedPopEntryList( &(Zone)->FreeList, Lock )

//++
//
// PVOID
// ExInterlockedFreeToZone(
//     IN PZONE_HEADER Zone,
//     IN PVOID Block,
//     IN PKSPIN_LOCK Lock
//     )
//
// Routine Description:
//
//     This routine places the specified block of storage back onto the free
//     list in the specified zone.  The insertion is performed with the lock
//     owned for the sequence to make it MP-safe.
//
// Arguments:
//
//     Zone - Pointer to the zone header controlling the storage to which the
//         entry is to be inserted.
//
//     Block - Pointer to the block of storage to be freed back to the zone.
//
//     Lock - Pointer to the spin lock which should be obtained before inserting
//         the entry onto the free list.  The lock is released before returning
//         to the caller.
//
// Return Value:
//
//     Pointer to previous block of storage that was at the head of the free
//         list.  NULL implies the zone went from no available free blocks to
//         at least one free block.
//
//--

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInterlockedFreeToZone)
#endif
#define ExInterlockedFreeToZone(Zone,Block,Lock) \
    ExInterlockedPushEntryList( &(Zone)->FreeList, ((PSINGLE_LIST_ENTRY) (Block)), Lock )


//++
//
// BOOLEAN
// ExIsObjectInFirstZoneSegment(
//     IN PZONE_HEADER Zone,
//     IN PVOID Object
//     )
//
// Routine Description:
//
//     This routine determines if the specified pointer lives in the zone.
//
// Arguments:
//
//     Zone - Pointer to the zone header controlling the storage to which the
//         object may belong.
//
//     Object - Pointer to the object in question.
//
// Return Value:
//
//     TRUE if the Object came from the first segment of zone.
//
//--

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExIsObjectInFirstZoneSegment)
#endif
#define ExIsObjectInFirstZoneSegment(Zone,Object) ((BOOLEAN)     \
    (((PUCHAR)(Object) >= (PUCHAR)(Zone)->SegmentList.Next) &&   \
     ((PUCHAR)(Object) < (PUCHAR)(Zone)->SegmentList.Next +      \
                         (Zone)->TotalSegmentSize))              \
)

// end_ntddk end_nthal end_ntifs end_ntosp



// begin_ntifs begin_ntddk begin_wdm begin_ntosp
//
//  Define executive resource data structures.
//

typedef ULONG_PTR ERESOURCE_THREAD;
typedef ERESOURCE_THREAD *PERESOURCE_THREAD;

typedef struct _OWNER_ENTRY {
    ERESOURCE_THREAD OwnerThread;
    union {
        LONG OwnerCount;
        ULONG TableSize;
    };

} OWNER_ENTRY, *POWNER_ENTRY;

typedef struct _ERESOURCE {
    LIST_ENTRY SystemResourcesList;
    POWNER_ENTRY OwnerTable;
    SHORT ActiveCount;
    USHORT Flag;
    PKSEMAPHORE SharedWaiters;
    PKEVENT ExclusiveWaiters;
    OWNER_ENTRY OwnerThreads[2];
    ULONG ContentionCount;
    USHORT NumberOfSharedWaiters;
    USHORT NumberOfExclusiveWaiters;
    union {
        PVOID Address;
        ULONG_PTR CreatorBackTraceIndex;
    };

    KSPIN_LOCK SpinLock;
} ERESOURCE, *PERESOURCE;
//
//  Values for ERESOURCE.Flag
//

#define ResourceNeverExclusive       0x10
#define ResourceReleaseByOtherThread 0x20
#define ResourceOwnedExclusive       0x80

#define RESOURCE_HASH_TABLE_SIZE 64

typedef struct _RESOURCE_HASH_ENTRY {
    LIST_ENTRY ListEntry;
    PVOID Address;
    ULONG ContentionCount;
    ULONG Number;
} RESOURCE_HASH_ENTRY, *PRESOURCE_HASH_ENTRY;

typedef struct _RESOURCE_PERFORMANCE_DATA {
    ULONG ActiveResourceCount;
    ULONG TotalResourceCount;
    ULONG ExclusiveAcquire;
    ULONG SharedFirstLevel;
    ULONG SharedSecondLevel;
    ULONG StarveFirstLevel;
    ULONG StarveSecondLevel;
    ULONG WaitForExclusive;
    ULONG OwnerTableExpands;
    ULONG MaximumTableExpand;
    LIST_ENTRY HashTable[RESOURCE_HASH_TABLE_SIZE];
} RESOURCE_PERFORMANCE_DATA, *PRESOURCE_PERFORMANCE_DATA;

//
// Define executive resource function prototypes.
//
NTKERNELAPI
NTSTATUS
ExInitializeResourceLite (
    __out PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExReinitializeResourceLite (
    __inout PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceSharedLite (
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    );

NTKERNELAPI
PVOID
ExEnterCriticalRegionAndAcquireResourceShared (
    __inout PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireResourceExclusiveLite (
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    );

NTKERNELAPI
PVOID
ExEnterCriticalRegionAndAcquireResourceExclusive (
    __inout PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedStarveExclusive(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    );

NTKERNELAPI
BOOLEAN
ExAcquireSharedWaitForExclusive(
    __inout PERESOURCE Resource,
    __in BOOLEAN Wait
    );

NTKERNELAPI
PVOID
ExEnterCriticalRegionAndAcquireSharedWaitForExclusive (
    __inout PERESOURCE Resource
    );

NTKERNELAPI
BOOLEAN
ExTryToAcquireResourceExclusiveLite(
    __inout PERESOURCE Resource
    );

//
//  VOID
//  ExReleaseResource(
//      IN PERESOURCE Resource
//      );
//

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExReleaseResource)       // Use ExReleaseResourceLite
#endif
#define ExReleaseResource(R) (ExReleaseResourceLite(R))

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceLite(
    __inout PERESOURCE Resource
    );

NTKERNELAPI
VOID
FASTCALL
ExReleaseResourceAndLeaveCriticalRegion(
    __inout PERESOURCE Resource
    );

NTKERNELAPI
VOID
ExReleaseResourceForThreadLite(
    __inout PERESOURCE Resource,
    __in ERESOURCE_THREAD ResourceThreadId
    );

NTKERNELAPI
VOID
ExSetResourceOwnerPointer(
    __inout PERESOURCE Resource,
    __in PVOID OwnerPointer
    );

NTKERNELAPI
VOID
ExConvertExclusiveToSharedLite(
    __inout PERESOURCE Resource
    );

NTKERNELAPI
NTSTATUS
ExDeleteResourceLite (
    __inout PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetExclusiveWaiterCount (
    __in PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExGetSharedWaiterCount (
    __in PERESOURCE Resource
    );

// end_ntddk end_wdm end_ntosp

NTKERNELAPI
VOID
ExDisableResourceBoostLite (
    __in PERESOURCE Resource
    );

#if DBG

VOID
ExCheckIfResourceOwned (
    VOID
    );

#endif

// begin_ntddk begin_wdm begin_ntosp
//
//  ERESOURCE_THREAD
//  ExGetCurrentResourceThread(
//      );
//

#define ExGetCurrentResourceThread() ((ULONG_PTR)PsGetCurrentThread())

NTKERNELAPI
BOOLEAN
ExIsResourceAcquiredExclusiveLite (
    __in PERESOURCE Resource
    );

NTKERNELAPI
ULONG
ExIsResourceAcquiredSharedLite (
    __in PERESOURCE Resource
    );

//
// An acquired resource is always owned shared, as shared ownership is a subset
// of exclusive ownership.
//
#define ExIsResourceAcquiredLite ExIsResourceAcquiredSharedLite

// end_wdm
//
//  ntddk.h stole the entrypoints we wanted so fix them up here.
//

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(ExInitializeResource)            // use ExInitializeResourceLite
#pragma deprecated(ExAcquireResourceShared)         // use ExAcquireResourceSharedLite
#pragma deprecated(ExAcquireResourceExclusive)      // use ExAcquireResourceExclusiveLite
#pragma deprecated(ExReleaseResourceForThread)      // use ExReleaseResourceForThreadLite
#pragma deprecated(ExConvertExclusiveToShared)      // use ExConvertExclusiveToSharedLite
#pragma deprecated(ExDeleteResource)                // use ExDeleteResourceLite
#pragma deprecated(ExIsResourceAcquiredExclusive)   // use ExIsResourceAcquiredExclusiveLite
#pragma deprecated(ExIsResourceAcquiredShared)      // use ExIsResourceAcquiredSharedLite
#pragma deprecated(ExIsResourceAcquired)            // use ExIsResourceAcquiredSharedLite
#endif
#define ExInitializeResource ExInitializeResourceLite
#define ExAcquireResourceShared ExAcquireResourceSharedLite
#define ExAcquireResourceExclusive ExAcquireResourceExclusiveLite
#define ExReleaseResourceForThread ExReleaseResourceForThreadLite
#define ExConvertExclusiveToShared ExConvertExclusiveToSharedLite
#define ExDeleteResource ExDeleteResourceLite
#define ExIsResourceAcquiredExclusive ExIsResourceAcquiredExclusiveLite
#define ExIsResourceAcquiredShared ExIsResourceAcquiredSharedLite
#define ExIsResourceAcquired ExIsResourceAcquiredSharedLite

// end_ntddk end_ntosp
#define ExDisableResourceBoost ExDisableResourceBoostLite
// end_ntifs

NTKERNELAPI
NTSTATUS
ExQuerySystemLockInformation(
    __out_bcount(LockInformationLength) struct _RTL_PROCESS_LOCKS *LockInformation,
    __in ULONG LockInformationLength,
    __out_opt PULONG ReturnLength
    );



// begin_ntosp

//
// Push lock definitions
//
typedef struct _EX_PUSH_LOCK {

//
// LOCK bit is set for both exclusive and shared acquires
//
#define EX_PUSH_LOCK_LOCK_V          ((ULONG_PTR)0x0)
#define EX_PUSH_LOCK_LOCK            ((ULONG_PTR)0x1)

//
// Waiting bit designates that the pointer has chained waiters
//

#define EX_PUSH_LOCK_WAITING         ((ULONG_PTR)0x2)

//
// Waking bit designates that we are either traversing the list
// to wake threads or optimizing the list
//

#define EX_PUSH_LOCK_WAKING          ((ULONG_PTR)0x4)

//
// Set if the lock is held shared by multiple owners and there are waiters
//

#define EX_PUSH_LOCK_MULTIPLE_SHARED ((ULONG_PTR)0x8)

//
// Total shared Acquires are incremented using this
//
#define EX_PUSH_LOCK_SHARE_INC       ((ULONG_PTR)0x10)
#define EX_PUSH_LOCK_PTR_BITS        ((ULONG_PTR)0xf)

    union {
        struct {
            ULONG_PTR Locked         : 1;
            ULONG_PTR Waiting        : 1;
            ULONG_PTR Waking         : 1;
            ULONG_PTR MultipleShared : 1;
            ULONG_PTR Shared         : sizeof (ULONG_PTR) * 8 - 4;
        };
        ULONG_PTR Value;
        PVOID Ptr;
    };
} EX_PUSH_LOCK, *PEX_PUSH_LOCK;

#if defined (NT_UP)
#define EX_CACHE_LINE_SIZE 16
#define EX_PUSH_LOCK_FANNED_COUNT 1
#else
#define EX_CACHE_LINE_SIZE 128
#define EX_PUSH_LOCK_FANNED_COUNT (PAGE_SIZE/EX_CACHE_LINE_SIZE)
#endif

//
// Define a fan out structure for n push locks each in its own cache line
//
typedef struct _EX_PUSH_LOCK_CACHE_AWARE {
    PEX_PUSH_LOCK Locks[EX_PUSH_LOCK_FANNED_COUNT];
} EX_PUSH_LOCK_CACHE_AWARE, *PEX_PUSH_LOCK_CACHE_AWARE;

//
// Define structure thats a push lock padded to the size of a cache line
//
typedef struct _EX_PUSH_LOCK_CACHE_AWARE_PADDED {
        EX_PUSH_LOCK Lock;
        union {
            UCHAR Pad[EX_CACHE_LINE_SIZE - sizeof (EX_PUSH_LOCK)];
            BOOLEAN Single;
        };
} EX_PUSH_LOCK_CACHE_AWARE_PADDED, *PEX_PUSH_LOCK_CACHE_AWARE_PADDED;

// begin_wdm begin_ntddk begin_ntifs 

//
// Rundown protection structure
//
typedef struct _EX_RUNDOWN_REF {

#define EX_RUNDOWN_ACTIVE      0x1
#define EX_RUNDOWN_COUNT_SHIFT 0x1
#define EX_RUNDOWN_COUNT_INC   (1<<EX_RUNDOWN_COUNT_SHIFT)
    union {
        ULONG_PTR Count;
        PVOID Ptr;
    };
} EX_RUNDOWN_REF, *PEX_RUNDOWN_REF;
          
//
//  Opaque cache-aware rundown ref structure
//

typedef struct _EX_RUNDOWN_REF_CACHE_AWARE  *PEX_RUNDOWN_REF_CACHE_AWARE;

// end_wdm end_ntddk end_ntifs

typedef struct _EX_RUNDOWN_REF_CACHE_AWARE {

    //
    //  Pointer to array of cache-line aligned rundown ref structures
    //

    PEX_RUNDOWN_REF RunRefs;

    //
    //  Points to pool of per-proc rundown refs that needs to be freed
    //

    PVOID PoolToFree;

    //
    //  Size of each padded rundown ref structure
    //

    ULONG RunRefSize;

    //
    //  Indicates # of entries in the array of rundown ref structures
    //

    ULONG Number;
} EX_RUNDOWN_REF_CACHE_AWARE, *PEX_RUNDOWN_REF_CACHE_AWARE;



//
//  The Ex/Ob handle table interface package (in handle.c)
//

//
//  The Ex/Ob handle table package uses a common handle definition.  The actual
//  type definition for a handle is a pvoid and is declared in sdk/inc.  This
//  package uses only the low 32 bits of the pvoid pointer.
//
//  For simplicity we declare a new typedef called an exhandle
//
//  The 2 bits of an EXHANDLE is available to the application and is
//  ignored by the system.  The next 24 bits store the handle table entry
//  index and is used to refer to a particular entry in a handle table.
//
//  Note that this format is immutable because there are outside programs with
//  hardwired code that already assumes the format of a handle.
//

typedef struct _EXHANDLE {

    union {

        struct {

            //
            //  Application available tag bits
            //

            ULONG TagBits : 2;

            //
            //  The handle table entry index
            //

            ULONG Index : 30;

        };

        HANDLE GenericHandleOverlay;

#define HANDLE_VALUE_INC 4 // Amount to increment the Value to get to the next handle

        ULONG_PTR Value;
    };

} EXHANDLE, *PEXHANDLE;
// end_ntosp

typedef struct _HANDLE_TABLE_ENTRY_INFO {


    //
    //  The following field contains the audit mask for the handle if one
    //  exists.  The purpose of the audit mask is to record all of the accesses
    //  that may have been audited when the handle was opened in order to
    //  support "per operation" based auditing.  It is computed by walking the
    //  SACL of the object being opened and keeping a record of all of the audit
    //  ACEs that apply to the open operation going on.  Each set bit corresponds
    //  to an access that would be audited.  As each operation takes place, its
    //  corresponding access bit is removed from this mask.
    //

    ACCESS_MASK AuditMask;

} HANDLE_TABLE_ENTRY_INFO, *PHANDLE_TABLE_ENTRY_INFO;

//
//  A handle table stores multiple handle table entries, each entry is looked
//  up by its exhandle.  A handle table entry has really two fields.
//
//  The first field contains a pointer object and is overloaded with the three
//  low order bits used by ob to denote inherited, protected, and audited
//  objects.   The upper bit used as a handle table entry lock.  Note, this
//  means that all valid object pointers must be at least longword aligned and
//  have their sign bit set (i.e., be negative).
//
//  The next field contains the access mask (sometimes in the form of a granted
//  access index, and creator callback trace) if the entry is in use or a
//  pointer in the free list if the entry is free.
//
//  Two things to note:
//
//  1. An entry is free if the object pointer is null, this means that the
//     following field contains the FreeTableEntryList.
//
//  2. An entry is unlocked if the object pointer is positive and locked if its
//     negative.  The handle package through callbacks and Map Handle to
//     Pointer will lock the entry (thus making the pointer valid) outside
//     routines can then read and reset the attributes field and the object
//     provided they don't unlock the entry.  When the callbacks return the
//     entry will be unlocked and the callers or MapHandleToPointer will need
//     to call UnlockHandleTableEntry explicitly.
//

typedef struct _HANDLE_TABLE_ENTRY {

    //
    //  The pointer to the object overloaded with three ob attributes bits in
    //  the lower order and the high bit to denote locked or unlocked entries
    //

    union {

        PVOID Object;

        ULONG ObAttributes;

        PHANDLE_TABLE_ENTRY_INFO InfoTable;

        ULONG_PTR Value;
    };

    //
    //  This field either contains the granted access mask for the handle or an
    //  ob variation that also stores the same information.  Or in the case of
    //  a free entry the field stores the index for the next free entry in the
    //  free list.  This is like a FAT chain, and is used instead of pointers
    //  to make table duplication easier, because the entries can just be
    //  copied without needing to modify pointers.
    //

    union {

        union {

            ACCESS_MASK GrantedAccess;

            struct {

                USHORT GrantedAccessIndex;
                USHORT CreatorBackTraceIndex;
            };
        };

        LONG NextFreeTableEntry;
    };

} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;


//
// Define a structure to track handle usage
//

#define HANDLE_TRACE_DB_MAX_STACKS 65536
#define HANDLE_TRACE_DB_MIN_STACKS 128
#define HANDLE_TRACE_DB_DEFAULT_STACKS 4096
#define HANDLE_TRACE_DB_STACK_SIZE 16

typedef struct _HANDLE_TRACE_DB_ENTRY {
    CLIENT_ID ClientId;
    HANDLE Handle;
#define HANDLE_TRACE_DB_OPEN    1
#define HANDLE_TRACE_DB_CLOSE   2
#define HANDLE_TRACE_DB_BADREF  3
    ULONG Type;
    PVOID StackTrace[HANDLE_TRACE_DB_STACK_SIZE];
} HANDLE_TRACE_DB_ENTRY, *PHANDLE_TRACE_DB_ENTRY;

typedef struct _HANDLE_TRACE_DEBUG_INFO {

    //
    // Reference count for this structure
    //
    LONG RefCount;

    //
    // Size of the trace table in entries
    //

    ULONG TableSize;

    //
    // this flag will clean the TraceDb.
    // once the TraceDb is cleaned, this flag will be reset.
    // it is needed for setting the HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE
    // dynamically via KD
    //
#define HANDLE_TRACE_DEBUG_INFO_CLEAN_DEBUG_INFO        0x1

    //
    // this flag will do the following: for each close
    // it will look for a matching open, remove the open
    // entry and compact TraceDb
    // NOTE: this should not be used for HANDLE_TRACE_DB_BADREF
    //      because you will not have the close trace
    //
#define HANDLE_TRACE_DEBUG_INFO_COMPACT_CLOSE_HANDLE    0x2

    //
    // setting this flag will break into debugger when the trace list
    // wraps around. This way you will have a chance to loot at old entries
    // before they are deleted
    //
#define HANDLE_TRACE_DEBUG_INFO_BREAK_ON_WRAP_AROUND    0x4

    //
    // these are status flags, do not set them explicitly
    //
#define HANDLE_TRACE_DEBUG_INFO_WAS_WRAPPED_AROUND      0x40000000
#define HANDLE_TRACE_DEBUG_INFO_WAS_SOMETIME_CLEANED    0x80000000

        ULONG BitMaskFlags;

        FAST_MUTEX CloseCompactionLock;

        //
        // Current index for the stack trace DB
        //
        ULONG CurrentStackIndex;

        //
        // Save traces of those who open and close handles
        //
        HANDLE_TRACE_DB_ENTRY TraceDb[1];

} HANDLE_TRACE_DEBUG_INFO, *PHANDLE_TRACE_DEBUG_INFO;


//
//  One handle table exists per process.  Unless otherwise specified, via a
//  call to RemoveHandleTable, all handle tables are linked together in a
//  global list.  This list is used by the snapshot handle tables call.
//


typedef struct _HANDLE_TABLE {

    //
    //  A pointer to the top level handle table tree node.
    //

    ULONG_PTR TableCode;

    //
    //  The process who is being charged quota for this handle table and a
    //  unique process id to use in our callbacks
    //

    struct _EPROCESS *QuotaProcess;
    HANDLE UniqueProcessId;


    //
    // These locks are used for table expansion and preventing the A-B-A problem
    // on handle allocate.
    //

#define HANDLE_TABLE_LOCKS 4

    EX_PUSH_LOCK HandleTableLock[HANDLE_TABLE_LOCKS];

    //
    //  The list of global handle tables.  This field is protected by a global
    //  lock.
    //

    LIST_ENTRY HandleTableList;

    //
    // Define a field to block on if a handle is found locked.
    //
    EX_PUSH_LOCK HandleContentionEvent;

    //
    // Debug info. Only allocated if we are debugging handles
    //
    PHANDLE_TRACE_DEBUG_INFO DebugInfo;

    //
    //  The number of pages for additional info.
    //  This counter is used to improve the performance
    //  in ExGetHandleInfo
    //
    LONG ExtraInfoPages;

    //
    //  This is a singly linked list of free table entries.  We don't actually
    //  use pointers, but have each store the index of the next free entry
    //  in the list.  The list is managed as a lifo list.  We also keep track
    //  of the next index that we have to allocate pool to hold.
    //

    ULONG FirstFree;

    //
    // We free handles to this list when handle debugging is on or if we see
    // that a thread has this handles bucket lock held. The allows us to delay reuse
    // of handles to get a better chance of catching offenders
    //

    ULONG LastFree;

    //
    // This is the next handle index needing a pool allocation. Its also used as a bound
    // for good handles.
    //

    ULONG NextHandleNeedingPool;

    //
    //  The number of handle table entries in use.
    //

    LONG HandleCount;

    //
    // Define a flags field
    //
    union {
        ULONG Flags;

        //
        // For optimization we reuse handle values quickly. This can be a problem for
        // some usages of handles and makes debugging a little harder. If this
        // bit is set then we always use FIFO handle allocation.
        //
        BOOLEAN StrictFIFO : 1;
    };

} HANDLE_TABLE, *PHANDLE_TABLE;

//
//  Routines for handle manipulation.
//

//
//  Function for unlocking handle table entries
//

NTKERNELAPI
VOID
ExUnlockHandleTableEntry (
    __inout PHANDLE_TABLE HandleTable,
    __inout PHANDLE_TABLE_ENTRY HandleTableEntry
    );

//
//  A global initialization function called on at system start up
//

NTKERNELAPI
VOID
ExInitializeHandleTablePackage (
    VOID
    );

//
//  Functions to create, remove, and destroy handle tables per process.  The
//  destroy function uses a callback.
//

NTKERNELAPI
PHANDLE_TABLE
ExCreateHandleTable (
    __in_opt struct _EPROCESS *Process
    );

VOID
ExSetHandleTableStrictFIFO (
    IN PHANDLE_TABLE HandleTable
    );

NTKERNELAPI
VOID
ExRemoveHandleTable (
    __inout PHANDLE_TABLE HandleTable
    );

NTKERNELAPI
NTSTATUS
ExEnableHandleTracing (
    __inout PHANDLE_TABLE HandleTable,
    __in ULONG Slots
    );

NTKERNELAPI
NTSTATUS
ExDisableHandleTracing (
    __inout PHANDLE_TABLE HandleTable
    );

VOID
ExDereferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable,
    IN PHANDLE_TRACE_DEBUG_INFO DebugInfo
    );

PHANDLE_TRACE_DEBUG_INFO
ExReferenceHandleDebugInfo (
    IN PHANDLE_TABLE HandleTable
    );


typedef VOID (*EX_DESTROY_HANDLE_ROUTINE)(
    IN HANDLE Handle
    );

NTKERNELAPI
VOID
ExDestroyHandleTable (
    __inout PHANDLE_TABLE HandleTable,
    __in EX_DESTROY_HANDLE_ROUTINE DestroyHandleProcedure
    );

//
//  A function to enumerate through the handle table of a process using a
//  callback.
//

typedef BOOLEAN (*EX_ENUMERATE_HANDLE_ROUTINE)(
    IN PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN HANDLE Handle,
    IN PVOID EnumParameter
    );

NTKERNELAPI
BOOLEAN
ExEnumHandleTable (
    __in PHANDLE_TABLE HandleTable,
    __in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    __in PVOID EnumParameter,
    __out_opt PHANDLE Handle
    );

NTKERNELAPI
VOID
ExSweepHandleTable (
    __in PHANDLE_TABLE HandleTable,
    __in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
    __in PVOID EnumParameter
    );

//
//  A function to duplicate the handle table of a process using a callback
//

typedef BOOLEAN (*EX_DUPLICATE_HANDLE_ROUTINE)(
    IN struct _EPROCESS *Process OPTIONAL,
    IN PHANDLE_TABLE OldHandleTable,
    IN PHANDLE_TABLE_ENTRY OldHandleTableEntry,
    IN PHANDLE_TABLE_ENTRY HandleTableEntry
    );

NTKERNELAPI
PHANDLE_TABLE
ExDupHandleTable (
    __inout_opt struct _EPROCESS *Process,
    __in PHANDLE_TABLE OldHandleTable,
    __in EX_DUPLICATE_HANDLE_ROUTINE DupHandleProcedure OPTIONAL,
    __in ULONG_PTR Mask
    );

//
//  A function that enumerates all the handles in all the handle tables
//  throughout the system using a callback.
//

typedef NTSTATUS (*PEX_SNAPSHOT_HANDLE_ENTRY)(
    IN OUT PSYSTEM_HANDLE_TABLE_ENTRY_INFO *HandleEntryInfo,
    IN HANDLE UniqueProcessId,
    IN PHANDLE_TABLE_ENTRY HandleEntry,
    IN HANDLE Handle,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

typedef NTSTATUS (*PEX_SNAPSHOT_HANDLE_ENTRY_EX)(
    IN OUT PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX *HandleEntryInfo,
    IN HANDLE UniqueProcessId,
    IN PHANDLE_TABLE_ENTRY HandleEntry,
    IN HANDLE Handle,
    IN ULONG Length,
    IN OUT PULONG RequiredLength
    );

NTKERNELAPI
NTSTATUS
ExSnapShotHandleTables (
    __in PEX_SNAPSHOT_HANDLE_ENTRY SnapShotHandleEntry,
    __inout PSYSTEM_HANDLE_INFORMATION HandleInformation,
    __in ULONG Length,
    __inout PULONG RequiredLength
    );

NTKERNELAPI
NTSTATUS
ExSnapShotHandleTablesEx (
    __in PEX_SNAPSHOT_HANDLE_ENTRY_EX SnapShotHandleEntry,
    __inout PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    __in ULONG Length,
    __inout PULONG RequiredLength
    );

//
//  Functions to create, destroy, and modify handle table entries the modify
//  function using a callback
//

NTKERNELAPI
HANDLE
ExCreateHandle (
    __inout PHANDLE_TABLE HandleTable,
    __in PHANDLE_TABLE_ENTRY HandleTableEntry
    );


NTKERNELAPI
BOOLEAN
ExDestroyHandle (
    __inout PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __inout_opt PHANDLE_TABLE_ENTRY HandleTableEntry
    );


typedef BOOLEAN (*PEX_CHANGE_HANDLE_ROUTINE) (
    IN OUT PHANDLE_TABLE_ENTRY HandleTableEntry,
    IN ULONG_PTR Parameter
    );

NTKERNELAPI
BOOLEAN
ExChangeHandle (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in PEX_CHANGE_HANDLE_ROUTINE ChangeRoutine,
    __in ULONG_PTR Parameter
    );

//
//  A function that takes a handle value and returns a pointer to the
//  associated handle table entry.
//

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointer (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle
    );

NTKERNELAPI
PHANDLE_TABLE_ENTRY
ExMapHandleToPointerEx (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in KPROCESSOR_MODE PreviousMode
    );

NTKERNELAPI
NTSTATUS
ExSetHandleInfo (
    __inout PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in PHANDLE_TABLE_ENTRY_INFO EntryInfo,
    __in BOOLEAN EntryLocked
    );

NTKERNELAPI
PHANDLE_TABLE_ENTRY_INFO
ExpGetHandleInfo (
    __in PHANDLE_TABLE HandleTable,
    __in HANDLE Handle,
    __in BOOLEAN EntryLocked
    );

#define ExGetHandleInfo(HT,H,E) \
    ((HT)->ExtraInfoPages ? ExpGetHandleInfo((HT),(H),(E)) : NULL)


//
//  Macros for resetting the owner of the handle table, and current
//  noop macro for setting fifo/lifo behavior of the table
//

#define ExSetHandleTableOwner(ht,id) {(ht)->UniqueProcessId = (id);}

#define ExSetHandleTableOrder(ht,or) {NOTHING;}


//
// Locally Unique Identifier Services
//

NTKERNELAPI
BOOLEAN
ExLuidInitialization (
    VOID
    );

//
// VOID
// ExAllocateLocallyUniqueId (
//     PLUID Luid
//     )
//
//*++
//
// Routine Description:
//
//     This function returns an LUID value that is unique since the system
//     was last rebooted. It is unique only on the system it is generated on
//     and not network wide.
//
//     N.B. A LUID is a 64-bit value and for all practical purposes will
//          never carry in the lifetime of a single boot of the system.
//          At an increment rate of 1ns, the value would carry to zero in
//          approximately 126 years.
//
// Arguments:
//
//     Luid - Supplies a pointer to a variable that receives the allocated
//          locally unique Id.
//
// Return Value:
//
//     The allocated LUID value.
//
// --*/


extern LARGE_INTEGER ExpLuid;
extern const LARGE_INTEGER ExpLuidIncrement;

__inline
VOID
ExAllocateLocallyUniqueId (
    IN OUT PLUID Luid
    )

{
    LARGE_INTEGER Initial;

#if defined (_WIN64) && !defined(_X86AMD64_)
    Initial.QuadPart = InterlockedAdd64 (&ExpLuid.QuadPart, ExpLuidIncrement.QuadPart);
#else
    LARGE_INTEGER Value;


    while (1) {
        Initial.QuadPart = ExpLuid.QuadPart;

        Value.QuadPart = Initial.QuadPart + ExpLuidIncrement.QuadPart;
        Value.QuadPart = InterlockedCompareExchange64(&ExpLuid.QuadPart,
                                                      Value.QuadPart,
                                                      Initial.QuadPart);
        if (Initial.QuadPart != Value.QuadPart) {
            continue;
        }
        break;
    }

#endif

    Luid->LowPart = Initial.LowPart;
    Luid->HighPart = Initial.HighPart;
    return;
}

// begin_ntddk begin_wdm begin_ntifs begin_ntosp
//
// Get previous mode
//

NTKERNELAPI
KPROCESSOR_MODE
ExGetPreviousMode(
    VOID
    );
// end_ntddk end_wdm end_ntifs end_ntosp

//
// Raise exception from kernel mode.
//

NTKERNELAPI
VOID
NTAPI
ExRaiseException (
    __in PEXCEPTION_RECORD ExceptionRecord
    );

// begin_ntosp

FORCEINLINE
VOID
ProbeForWriteSmallStructure (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG Alignment
    )

/*++

Routine Description:

    Probes a structure for write access whose size is known at compile time.

Arguments:

    Address - Supples a pointer to the structure.

    Size - Supplies the size of the structure.

    Alignment - Supplies the alignment of structure.

Return Value:

    None

--*/

{

    ASSERT((Alignment == 1) || (Alignment == 2) ||
           (Alignment == 4) || (Alignment == 8) ||
           (Alignment == 16));

    //
    // If the size of the structure is > 4k then call the standard routine.
    // wow64 uses a page size of 4k even on ia64.
    //

    if ((Size == 0) || (Size >= 0x1000)) {

        ASSERT(0);

        ProbeForWrite(Address, Size, Alignment);

    } else {
        if (((ULONG_PTR)(Address) & (Alignment - 1)) != 0) {
            ExRaiseDatatypeMisalignment();
        }

#if defined(_AMD64_)

        if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {
             Address = (UCHAR * const)MM_USER_PROBE_ADDRESS;
        }
    
        ((volatile UCHAR *)(Address))[0] = ((volatile UCHAR *)(Address))[0];
        ((volatile UCHAR *)(Address))[Size - 1] = ((volatile UCHAR *)(Address))[Size - 1];

#else

        if ((ULONG_PTR)(Address) >= (ULONG_PTR)MM_USER_PROBE_ADDRESS) {
             *((volatile UCHAR * const)MM_USER_PROBE_ADDRESS) = 0;
        }
    
        *(volatile UCHAR *)(Address) = *(volatile UCHAR *)(Address);
        if (Size > Alignment) {
            ((volatile UCHAR *)(Address))[(Size - 1) & ~(SIZE_T)(Alignment - 1)] =
                ((volatile UCHAR *)(Address))[(Size - 1) & ~(SIZE_T)(Alignment - 1)];
        }

#endif

    }
}

NTKERNELAPI
NTSTATUS
ExRaiseHardError(
    __in NTSTATUS ErrorStatus,
    __in ULONG NumberOfParameters,
    __in ULONG UnicodeStringParameterMask,
    __in_ecount(NumberOfParameters) PULONG_PTR Parameters,
    __in ULONG ValidResponseOptions,
    __out PULONG Response
    );

int
ExSystemExceptionFilter(
    VOID
    );

NTKERNELAPI
VOID
ExGetCurrentProcessorCpuUsage(
    __out PULONG CpuUsage
    );

NTKERNELAPI
VOID
ExGetCurrentProcessorCounts(
    __out PULONG IdleCount,
    __out PULONG KernelAndUser,
    __out PULONG Index
    );

// end_ntosp

extern BOOLEAN ExReadyForErrors;

//
// The following are global counters used by the EX component to indicate
// the amount of EventPair transactions being performed in the system.
//

extern ULONG EvPrSetHigh;
extern ULONG EvPrSetLow;


//
// Debug event logging facility
//

#define EX_DEBUG_LOG_FORMAT_NONE     (UCHAR)0
#define EX_DEBUG_LOG_FORMAT_ULONG    (UCHAR)1
#define EX_DEBUG_LOG_FORMAT_PSZ      (UCHAR)2
#define EX_DEBUG_LOG_FORMAT_PWSZ     (UCHAR)3
#define EX_DEBUG_LOG_FORMAT_STRING   (UCHAR)4
#define EX_DEBUG_LOG_FORMAT_USTRING  (UCHAR)5
#define EX_DEBUG_LOG_FORMAT_OBJECT   (UCHAR)6
#define EX_DEBUG_LOG_FORMAT_HANDLE   (UCHAR)7

#define EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES 4
#define EX_DEBUG_LOG_NUMBER_OF_BACK_TRACES 4

typedef struct _EX_DEBUG_LOG_TAG {
    UCHAR Format[ EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES ];
    PCHAR Name;
} EX_DEBUG_LOG_TAG, *PEX_DEBUG_LOG_TAG;

typedef struct _EX_DEBUG_LOG_EVENT {
    USHORT ThreadId;
    USHORT ProcessId;
    ULONG Time : 24;
    ULONG Tag : 8;
    ULONG BackTrace[ EX_DEBUG_LOG_NUMBER_OF_BACK_TRACES ];
    ULONG Data[ EX_DEBUG_LOG_NUMBER_OF_DATA_VALUES ];
} EX_DEBUG_LOG_EVENT, *PEX_DEBUG_LOG_EVENT;

typedef struct _EX_DEBUG_LOG {
    KSPIN_LOCK Lock;
    ULONG NumberOfTags;
    ULONG MaximumNumberOfTags;
    PEX_DEBUG_LOG_TAG Tags;
    ULONG CountOfEventsLogged;
    PEX_DEBUG_LOG_EVENT First;
    PEX_DEBUG_LOG_EVENT Last;
    PEX_DEBUG_LOG_EVENT Next;
} EX_DEBUG_LOG, *PEX_DEBUG_LOG;


NTKERNELAPI
PEX_DEBUG_LOG
ExCreateDebugLog(
    __in UCHAR MaximumNumberOfTags,
    __in ULONG MaximumNumberOfEvents
    );

NTKERNELAPI
UCHAR
ExCreateDebugLogTag(
    __in PEX_DEBUG_LOG Log,
    __in PCHAR Name,
    __in UCHAR Format1,
    __in UCHAR Format2,
    __in UCHAR Format3,
    __in UCHAR Format4
    );

NTKERNELAPI
VOID
ExDebugLogEvent(
    __in PEX_DEBUG_LOG Log,
    __in UCHAR Tag,
    __in ULONG Data1,
    __in ULONG Data2,
    __in ULONG Data3,
    __in ULONG Data4
    );

VOID
ExShutdownSystem(
    IN ULONG Phase
    );

BOOLEAN
ExAcquireTimeRefreshLock(
    IN BOOLEAN Wait
    );

VOID
ExReleaseTimeRefreshLock(
    VOID
    );

VOID
ExUpdateSystemTimeFromCmos (
    IN BOOLEAN UpdateInterruptTime,
    IN ULONG   MaxSepInSeconds
    );

VOID
ExGetNextWakeTime (
    OUT PULONGLONG      DueTime,
    OUT PTIME_FIELDS    TimeFields,
    OUT PVOID           *TimerObject
    );

// begin_ntddk begin_wdm begin_ntifs begin_ntosp
//
// Set timer resolution.
//

NTKERNELAPI
ULONG
ExSetTimerResolution (
    __in ULONG DesiredTime,
    __in BOOLEAN SetResolution
    );

//
// Subtract time zone bias from system time to get local time.
//

NTKERNELAPI
VOID
ExSystemTimeToLocalTime (
    __in PLARGE_INTEGER SystemTime,
    __out PLARGE_INTEGER LocalTime
    );

//
// Add time zone bias to local time to get system time.
//

NTKERNELAPI
VOID
ExLocalTimeToSystemTime (
    __in PLARGE_INTEGER LocalTime,
    __out PLARGE_INTEGER SystemTime
    );

// end_ntddk end_wdm end_ntifs end_ntosp

NTKERNELAPI
VOID
ExInitializeTimeRefresh(
    VOID
    );

// begin_ntddk begin_wdm begin_ntifs begin_nthal begin_ntminiport begin_ntosp

//
// Define the type for Callback function.
//

typedef struct _CALLBACK_OBJECT *PCALLBACK_OBJECT;

typedef VOID (*PCALLBACK_FUNCTION ) (
    __in_opt PVOID CallbackContext,
    __in_opt PVOID Argument1,
    __in_opt PVOID Argument2
    );


NTKERNELAPI
NTSTATUS
ExCreateCallback (
    __deref_out PCALLBACK_OBJECT *CallbackObject,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN Create,
    __in BOOLEAN AllowMultipleCallbacks
    );

NTKERNELAPI
PVOID
ExRegisterCallback (
    __inout PCALLBACK_OBJECT CallbackObject,
    __in PCALLBACK_FUNCTION CallbackFunction,
    __in_opt PVOID CallbackContext
    );

NTKERNELAPI
VOID
ExUnregisterCallback (
    __inout PVOID CallbackRegistration
    );

NTKERNELAPI
VOID
ExNotifyCallback (
    __in PVOID CallbackObject,
    __in_opt PVOID Argument1,
    __in_opt PVOID Argument2
    );


// end_ntddk end_wdm end_ntifs end_nthal end_ntminiport end_ntosp

//
// System lookaside list structure list.
//

extern LIST_ENTRY ExSystemLookasideListHead;

//
// The current bias from GMT to LocalTime
//

extern LARGE_INTEGER ExpTimeZoneBias;
extern LONG ExpLastTimeZoneBias;
extern LONG ExpAltTimeZoneBias;
extern ULONG ExpCurrentTimeZoneId;
extern ULONG ExpRealTimeIsUniversal;
extern ULONG ExCriticalWorkerThreads;
extern ULONG ExDelayedWorkerThreads;
extern ULONG ExpTickCountMultiplier;

//
// Used for cmos clock sanity
//
extern BOOLEAN ExCmosClockIsSane;

//
// The lock handle for PAGELK section, initialized in init\init.c
//

extern PVOID ExPageLockHandle;

//
// Global executive callbacks
//

extern PCALLBACK_OBJECT ExCbSetSystemTime;
extern PCALLBACK_OBJECT ExCbSetSystemState;
extern PCALLBACK_OBJECT ExCbPowerState;


// begin_ntosp


typedef
PVOID
(*PKWIN32_GLOBALATOMTABLE_CALLOUT) ( void );

extern PKWIN32_GLOBALATOMTABLE_CALLOUT ExGlobalAtomTableCallout;

// end_ntosp

// begin_ntddk begin_ntosp begin_ntifs

//
// UUID Generation
//

typedef GUID UUID;

NTKERNELAPI
NTSTATUS
ExUuidCreate(
    __out UUID *Uuid
    );

// end_ntddk end_ntosp end_ntifs

// begin_ntddk begin_wdm begin_ntifs
//
// suite support
//

NTKERNELAPI
BOOLEAN
ExVerifySuite(
    __in SUITE_TYPE SuiteType
    );

// end_ntddk end_wdm end_ntifs


// begin_wdm begin_ntddk begin_ntosp begin_ntifs

//
//  Rundown Locks
//

NTKERNELAPI
VOID
FASTCALL
ExInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionEx (
     __inout PEX_RUNDOWN_REF RunRef,
     __in ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionEx (
     __inout PEX_RUNDOWN_REF RunRef,
     __in ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExRundownCompleted (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExWaitForRundownProtectionRelease (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
PEX_RUNDOWN_REF_CACHE_AWARE
ExAllocateCacheAwareRundownProtection(
    __in POOL_TYPE PoolType,
    __in ULONG PoolTag
    );

NTKERNELAPI
SIZE_T
ExSizeOfRundownProtectionCacheAware(
    VOID
    );

NTKERNELAPI
VOID
ExInitializeRundownProtectionCacheAware(
    __out PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware,
    __in SIZE_T RunRefSize
    );

NTKERNELAPI
VOID
ExFreeCacheAwareRundownProtection(
    __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
    );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExAcquireRundownProtectionCacheAwareEx (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware,
     __in ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExReleaseRundownProtectionCacheAwareEx (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRef,
     __in ULONG Count
     );

NTKERNELAPI
VOID
FASTCALL
ExWaitForRundownProtectionReleaseCacheAware (
     __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRef
     );

NTKERNELAPI
VOID
FASTCALL
ExReInitializeRundownProtectionCacheAware (
    __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
    );

NTKERNELAPI
VOID
FASTCALL
ExRundownCompletedCacheAware (
    __inout PEX_RUNDOWN_REF_CACHE_AWARE RunRefCacheAware
    );


// end_wdm end_ntddk end_ntosp end_ntifs

NTKERNELAPI
VOID
FASTCALL
ExfInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     )
/*++

Routine Description:

    Initialize rundown protection structure

Arguments:

    RunRef - Rundown block to be referenced

Return Value:

    None

--*/
{
    RunRef->Count = 0;
}

//
// Reset a rundown protection block
//

NTKERNELAPI
VOID
FASTCALL
ExfReInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExReInitializeRundownProtection (
     __out PEX_RUNDOWN_REF RunRef
     )
/*++

Routine Description:

    Reinitialize rundown protection structure after its been rundown

Arguments:

    RunRef - Rundown block to be referenced

Return Value:

    None

--*/
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, NULL);
}


//
// Acquire rundown protection
//

NTKERNELAPI
BOOLEAN
FASTCALL
ExfAcquireRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
BOOLEAN
FORCEINLINE
FASTCALL
ExAcquireRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR Value, NewValue;

    Value = ReadForWriteAccess(&RunRef->Count) & ~EX_RUNDOWN_ACTIVE;
    NewValue = Value + EX_RUNDOWN_COUNT_INC;
    NewValue = (ULONG_PTR) InterlockedCompareExchangePointerAcquire (&RunRef->Ptr,
                                                                     (PVOID) NewValue,
                                                                     (PVOID) Value);
    if (NewValue == Value) {
        return TRUE;
    } else {
        return ExfAcquireRundownProtection (RunRef);
    }
}

//
// Release rundown protection
//
NTKERNELAPI
VOID
FASTCALL
ExfReleaseRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExReleaseRundownProtection (
     __inout PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR Value, NewValue;

    Value = ReadForWriteAccess(&RunRef->Count) & ~EX_RUNDOWN_ACTIVE;
    NewValue = Value - EX_RUNDOWN_COUNT_INC;
    NewValue = (ULONG_PTR) InterlockedCompareExchangePointerRelease (&RunRef->Ptr,
                                                                     (PVOID) NewValue,
                                                                     (PVOID) Value);
    if (NewValue != Value) {
        ExfReleaseRundownProtection (RunRef);
    } else {

       //
       // For cache-aware rundown protection it is possible,
       // that the value is zero at this point for this processor
       //

        ASSERT ((Value >= EX_RUNDOWN_COUNT_INC) || (KeNumberProcessors > 1));
    }
}

//
// Mark rundown block as rundown having been completed.
//

NTKERNELAPI
VOID
FASTCALL
ExfRundownCompleted (
     __out PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExRundownCompleted (
     __out PEX_RUNDOWN_REF RunRef
     )
/*++
Routine Description:

    Mark rundown block has having completed rundown so we can wait again safely.

Arguments:

    RunRef - Rundown block to be referenced

Return Value:

    None
--*/
{
    PAGED_CODE ();

    ASSERT ((RunRef->Count&EX_RUNDOWN_ACTIVE) != 0);
    InterlockedExchangePointer (&RunRef->Ptr, (PVOID) EX_RUNDOWN_ACTIVE);
}

//
// Wait for all protected acquires to exit
//
NTKERNELAPI
VOID
FASTCALL
ExfWaitForRundownProtectionRelease (
     __inout PEX_RUNDOWN_REF RunRef
     );

NTKERNELAPI
VOID
FORCEINLINE
FASTCALL
ExWaitForRundownProtectionRelease (
     __inout PEX_RUNDOWN_REF RunRef
     )
{
    ULONG_PTR OldValue;

    OldValue = (ULONG_PTR) InterlockedCompareExchangePointerAcquire (&RunRef->Ptr,
                                                                    (PVOID) EX_RUNDOWN_ACTIVE,
                                                                    (PVOID) 0);
    if (OldValue != 0 && OldValue != EX_RUNDOWN_ACTIVE) {
        ExfWaitForRundownProtectionRelease (RunRef);
    }
}

//
// Fast reference routines. See ntos\ob\fastref.c for algorithm description.
//
#if defined (_WIN64)
#define MAX_FAST_REFS 15
#else
#define MAX_FAST_REFS 7
#endif

typedef struct _EX_FAST_REF {
    union {
        PVOID Object;
#if defined (_WIN64)
        ULONG_PTR RefCnt : 4;
#else
        ULONG_PTR RefCnt : 3;
#endif
        ULONG_PTR Value;
    };
} EX_FAST_REF, *PEX_FAST_REF;


NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefCanBeReferenced (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to determine if the fast reference
    structure contains cached references.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    LOGICAL - TRUE: There were cached references in the object,
              FALSE: No cached references are available.

--*/
{
    return FastRef.RefCnt != 0;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefCanBeDereferenced (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to determine if the fast reference
    structure contains room for cached references.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    LOGICAL - TRUE: There is space for cached references in the object,
              FALSE: No space was available.

--*/
{
    return FastRef.RefCnt != MAX_FAST_REFS;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefIsLastReference (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to determine if the fast reference
    structure contains only 1 cached reference.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    LOGICAL - TRUE: There is only one cached reference in the object,
              FALSE: The is more or less than one cached reference available.

--*/
{
    return FastRef.RefCnt == 1;
}


NTKERNELAPI
PVOID
FORCEINLINE
ExFastRefGetObject (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to obtain the object pointer from a fast
    reference structure.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    PVOID - The contained object or NULL if there isn't one.

--*/
{
    return (PVOID) (FastRef.Value & ~MAX_FAST_REFS);
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExFastRefObjectNull (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to test of the specified fastref value
    has a null pointer

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    BOOLEAN - TRUE if the object is NULL FALSE otherwise

--*/
{
    return (BOOLEAN) (FastRef.Value == 0);
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExFastRefEqualObjects (
    __in EX_FAST_REF FastRef,
    __in PVOID Object
    )
/*++

Routine Description:

    This routine allows the caller to test of the specified fastref contains
    the specified object

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    BOOLEAN - TRUE if the object matches FALSE otherwise

--*/
{
    return (BOOLEAN)((FastRef.Value^(ULONG_PTR)Object) <= MAX_FAST_REFS);
}


NTKERNELAPI
ULONG
FORCEINLINE
ExFastRefGetUnusedReferences (
    __in EX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine allows the caller to obtain the number of cached references
    in the fast reference structure.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    ULONG - The number of cached references.

--*/
{
    return (ULONG) FastRef.RefCnt;
}


NTKERNELAPI
VOID
FORCEINLINE
ExFastRefInitialize (
    __out PEX_FAST_REF FastRef,
    __in_opt PVOID Object
    )
/*++

Routine Description:

    This routine initializes fast reference structure.

Arguments:

    FastRef - Fast reference block to be used
    Object  - Object pointer to be assigned to the fast reference

Return Value:

    None.

--*/
{
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    if (Object == NULL) {
       FastRef->Object = NULL;
    } else {
       FastRef->Value = (ULONG_PTR) Object | MAX_FAST_REFS;
    }
}

NTKERNELAPI
VOID
FORCEINLINE
ExFastRefInitializeEx (
    __out PEX_FAST_REF FastRef,
    __in_opt PVOID Object,
    __in ULONG AdditionalRefs
    )
/*++

Routine Description:

    This routine initializes fast reference structure with the specified additional references.

Arguments:

    FastRe f       - Fast reference block to be used
    Object         - Object pointer to be assigned to the fast reference
    AdditionalRefs - Number of additional refs to add to the object

Return Value:

    None

--*/
{
    ASSERT (AdditionalRefs <= MAX_FAST_REFS);
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    if (Object == NULL) {
       FastRef->Object = NULL;
    } else {
       FastRef->Value = (ULONG_PTR) Object + AdditionalRefs;
    }
}

NTKERNELAPI
ULONG
FORCEINLINE
ExFastRefGetAdditionalReferenceCount (
    VOID
    )
{
    return MAX_FAST_REFS;
}



NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastReference (
    __inout PEX_FAST_REF FastRef
    )
/*++

Routine Description:

    This routine attempts to obtain a fast (cached) reference from a fast
    reference structure.

Arguments:

    FastRef - Fast reference block to be used

Return Value:

    EX_FAST_REF - The old or current contents of the fast reference structure.

--*/
{
    EX_FAST_REF OldRef, NewRef;

    while (1) {
        //
        // Fetch the old contents of the fast ref structure
        //
        OldRef = ReadForWriteAccess(FastRef);
        //
        // If the object pointer is null or if there are no cached references
        // left then bail. In the second case this reference will need to be
        // taken while holding the lock. Both cases are covered by the single
        // test of the lower bits since a null pointer can never have cached
        // refs.
        //
        if (OldRef.RefCnt != 0) {
            //
            // We know the bottom bits can't underflow into the pointer for a
            // request that works so just decrement
            //
            NewRef.Value = OldRef.Value - 1;
            NewRef.Object = InterlockedCompareExchangePointerAcquire (&FastRef->Object,
                                                                      NewRef.Object,
                                                                      OldRef.Object);
            if (NewRef.Object != OldRef.Object) {
                //
                // The structured changed beneath us. Try the operation again
                //
                continue;
            }
        }
        break;
    }

    return OldRef;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefDereference (
    __inout PEX_FAST_REF FastRef,
    __in PVOID Object
    )
/*++

Routine Description:

    This routine attempts to release a fast reference from a fast ref
    structure. This routine could be called for a reference obtained
    directly from the object but presumably the chances of the pointer
    matching would be unlikely. The algorithm will work correctly in this
    case.

Arguments:

    FastRef - Fast reference block to be used

    Object - The original object that the reference was taken on.

Return Value:

    LOGICAL - TRUE: The fast dereference worked ok, FALSE: the
              dereference didn't.

--*/
{
    EX_FAST_REF OldRef, NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    ASSERT (Object != NULL);

    while (1) {
        //
        // Fetch the old contents of the fast ref structure
        //
        OldRef = ReadForWriteAccess(FastRef);

        //
        // If the reference cache is fully populated or the pointer has
        // changed to another object then just return the old value. The
        // caller can return the reference to the object instead.
        //
        if ((OldRef.Value^(ULONG_PTR)Object) >= MAX_FAST_REFS) {
            return FALSE;
        }
        //
        // We know the bottom bits can't overflow into the pointer so just
        // increment
        //
        NewRef.Value = OldRef.Value + 1;
        NewRef.Object = InterlockedCompareExchangePointerRelease (&FastRef->Object,
                                                                  NewRef.Object,
                                                                  OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
            //
            // The structured changed beneath us. Try the operation again
            //
            continue;
        }
        break;
    }
    return TRUE;
}

NTKERNELAPI
LOGICAL
FORCEINLINE
ExFastRefAddAdditionalReferenceCounts (
    __inout PEX_FAST_REF FastRef,
    __in PVOID Object,
    __in ULONG RefsToAdd
    )
/*++

Routine Description:

    This routine attempts to update the cached references on structure to
    allow future callers to run lock free. Callers must have already biased
    the object by the RefsToAdd reference count. This operation can fail at
    which point the caller should removed the extra references added and
    continue.

Arguments:

    FastRef - Fast reference block to be used

    Object - The original object that has had its reference count biased.

    RefsToAdd - The number of references to add to the cache

Return Value:

    LOGICAL - TRUE: The references where cached ok, FALSE: The references
              could not be cached.

--*/
{
    EX_FAST_REF OldRef, NewRef;

    ASSERT (RefsToAdd <= MAX_FAST_REFS);
    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);

    while (1) {
        //
        // Fetch the old contents of the fast ref structure
        //
        OldRef = ReadForWriteAccess(FastRef);

        //
        // If the count would push us above maximum cached references or
        // if the object pointer has changed the fail the request.
        //
        if (OldRef.RefCnt + RefsToAdd > MAX_FAST_REFS ||
            (ULONG_PTR) Object != (OldRef.Value & ~MAX_FAST_REFS)) {
            return FALSE;
        }
        //
        // We know the bottom bits can't overflow into the pointer so just
        // increment
        //
        NewRef.Value = OldRef.Value + RefsToAdd;
        NewRef.Object = InterlockedCompareExchangePointerAcquire (&FastRef->Object,
                                                                  NewRef.Object,
                                                                  OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
            //
            // The structured changed beneath us. Use the return value from the
            // exchange and try it all again.
            //
            continue;
        }
        break;
    }
    return TRUE;
}

NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastRefSwapObject (
    __inout PEX_FAST_REF FastRef,
    __in_opt PVOID Object
    )
/*++

Routine Description:

    This routine attempts to replace the current object with a new object.
    This routine must be called while holding the lock that protects the
    pointer field if concurrency with the slow ref path is possible.
    Its also possible to obtain and drop the lock after this operation has
    completed to force all the slow referencers from the slow reference path.

Arguments:

    FastRef - Fast reference block to be used

    Object - The new object that is to be placed in the structure. This
             object must have already had its reference count biased by
             the caller to account for the reference cache.

Return Value:

    EX_FAST_REF - The old contents of the fast reference structure.

--*/
{
    EX_FAST_REF OldRef;
    EX_FAST_REF NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    if (Object != NULL) {
        NewRef.Value = (ULONG_PTR) Object | MAX_FAST_REFS;
    } else {
        NewRef.Value = 0;
    }
    OldRef.Object = InterlockedExchangePointer (&FastRef->Object, NewRef.Object);
    return OldRef;
}

NTKERNELAPI
EX_FAST_REF
FORCEINLINE
ExFastRefCompareSwapObject (
    __inout PEX_FAST_REF FastRef,
    __in_opt PVOID Object,
    __in PVOID OldObject
    )
/*++

Routine Description:

    This routine attempts to replace the current object with a new object if
    the current object matches the old object.
    This routine must be called while holding the lock that protects the
    pointer field if concurrency with the slow ref path is possible.
    Its also possible to obtain and drop the lock after this operation has
    completed to force all the slow referencers from the slow reference path.

Arguments:

    FastRef - Fast reference block to be used

    Object - The new object that is to be placed in the structure. This
             object must have already had its reference count biased by
             the caller to account for the reference cache.

    OldObject - The object that must match the current object for the
                swap to occur.

Return Value:

    EX_FAST_REF - The old contents of the fast reference structure.

--*/
{
    EX_FAST_REF OldRef;
    EX_FAST_REF NewRef;

    ASSERT ((((ULONG_PTR)Object)&MAX_FAST_REFS) == 0);
    while (1) {
        //
        // Fetch the old contents of the fast ref structure
        //
        OldRef = ReadForWriteAccess(FastRef);

        //
        // Compare the current object to the old to see if a swap is possible.
        //
        if (!ExFastRefEqualObjects (OldRef, OldObject)) {
            return OldRef;
        }

        if (Object != NULL) {
            NewRef.Value = (ULONG_PTR) Object | MAX_FAST_REFS;
        } else {
            NewRef.Value = (ULONG_PTR) Object;
        }

        NewRef.Object = InterlockedCompareExchangePointerRelease (&FastRef->Object,
                                                                  NewRef.Object,
                                                                  OldRef.Object);
        if (NewRef.Object != OldRef.Object) {
            //
            // The structured changed beneath us. Try it all again.
            //
            continue;
        }
        break;
    }
    return OldRef;
}

// begin_ntosp

#if !defined(NONTOSPINTERLOCK)

VOID
FORCEINLINE
ExInitializePushLock (
     __out PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Initialize a push lock structure

Arguments:

    PushLock - Push lock to be initialized

Return Value:

    None

--*/
{
    PushLock->Value = 0;
}

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfReleasePushLock (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfReleasePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfAcquirePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfReleasePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExfTryAcquirePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
BOOLEAN
FASTCALL
ExfTryAcquirePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
     );

NTKERNELAPI
VOID
FASTCALL
ExfTryToWakePushLock (
     __inout PEX_PUSH_LOCK PushLock
     );

// end_ntosp

NTKERNELAPI
VOID
FASTCALL
ExfConvertPushLockExclusiveToShared (
     __inout PEX_PUSH_LOCK PushLock
     );


NTKERNELAPI
VOID
FORCEINLINE
ExAcquireReleasePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Acquire a push lock exclusively and immediately release it

Arguments:

    PushLock - Push lock to be acquired and released

Return Value:

    None

--*/
{
    ULONG_PTR Locked;

    KeMemoryBarrier ();
    Locked = PushLock->Locked;
    KeMemoryBarrier ();

    if (Locked) {
        ExfAcquirePushLockExclusive (PushLock);
        ASSERT (PushLock->Locked);
        ExfReleasePushLockExclusive (PushLock);
    }
}

NTKERNELAPI
BOOLEAN
FORCEINLINE
ExTryAcquireReleasePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Try to acquire a push lock exclusively and immediately release it

Arguments:

    PushLock - Push lock to be acquired and released

Return Value:

    BOOLEAN - TRUE: The lock was acquired, FALSE: The lock was not acquired

--*/
{
    ULONG_PTR Locked;

    KeMemoryBarrier ();
    Locked = PushLock->Locked;
    KeMemoryBarrier ();

    if (Locked) {
        return FALSE;
    } else {
        return TRUE;
    }
}

VOID
FORCEINLINE
ExConvertPushLockExclusiveToShared (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Attempts to convert an exclusive acquire to shared. If other shared waiters 
    are present at the end of the waiters chain they are released.

Arguments:

    PushLock - Push lock to be converted

Return Value:

    None.

--*/
{

#if DBG
    EX_PUSH_LOCK OldValue;

    OldValue = *PushLock;

    ASSERT (OldValue.Waiting || OldValue.Shared == 0);
    ASSERT (OldValue.Locked);

#endif

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID) (EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK),
                                           (PVOID) EX_PUSH_LOCK_LOCK) !=
                                               (PVOID) EX_PUSH_LOCK_LOCK) {
        ExfConvertPushLockExclusiveToShared (PushLock);
#if DBG
        OldValue = *PushLock;
        ASSERT (OldValue.Locked);
        ASSERT (OldValue.Waiting || OldValue.Shared > 0);
#endif
    }
}

// begin_ntosp

VOID
FORCEINLINE
ExAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Acquire a push lock exclusively

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    None

--*/
{
#if defined (_WIN64)
    if (InterlockedBitTestAndSet64 ((LONG64 *)&PushLock->Value, EX_PUSH_LOCK_LOCK_V))
#else
    if (InterlockedBitTestAndSet ((LONG *)&PushLock->Value, EX_PUSH_LOCK_LOCK_V))
#endif
    {
        ExfAcquirePushLockExclusive (PushLock);
    }
    ASSERT (PushLock->Locked);
}

BOOLEAN
FORCEINLINE
ExTryAcquirePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Try and acquire a push lock exclusively

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    BOOLEAN - TRUE: Acquire was successful, FALSE: Lock was already acquired

--*/
{
#if defined (_WIN64)
    if (!InterlockedBitTestAndSet64 ((LONG64 *)&PushLock->Value, EX_PUSH_LOCK_LOCK_V)) {
#else
    if (!InterlockedBitTestAndSet ((LONG *)&PushLock->Value, EX_PUSH_LOCK_LOCK_V)) {
#endif
        ASSERT (PushLock->Locked);
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
FORCEINLINE
ExTryAcquirePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Try to Acquire a push lock shared.

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    BOOLEAN - TRUE: Acquire was successful, FALSE: Lock was already acquired exclusively.

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue.Value = 0;
    NewValue.Value = EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK;

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        if (!ExfTryAcquirePushLockShared (PushLock)) {
            return FALSE;
        }
    }
#if DBG
    OldValue = *PushLock;
    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared > 0);
#endif
    return TRUE;
}

VOID
FORCEINLINE
ExAcquirePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Acquire a push lock shared

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue.Value = 0;
    NewValue.Value = EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK;

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfAcquirePushLockShared (PushLock);
    }
#if DBG
    OldValue = *PushLock;
    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared > 0);
#endif
}

VOID
FORCEINLINE
ExAcquirePushLockSharedAssumeNoOwner (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Acquire a push lock shared making the assumption that its not currently owned.

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    None

--*/
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID)(EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK),
                                           NULL) != NULL) {
        ExfAcquirePushLockShared (PushLock);
    }
}

BOOLEAN
FORCEINLINE
ExTryConvertPushLockSharedToExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Attempts to convert a shared acquire to exclusive. If other sharers or waiters are present
    the function fails.

Arguments:

    PushLock - Push lock to be converted

Return Value:

    BOOLEAN - TRUE: Conversion worked ok, FALSE: The conversion could not be achieved

--*/
{
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           (PVOID) EX_PUSH_LOCK_LOCK,
                                           (PVOID) (EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK)) ==
                                               (PVOID)(EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK)) {
        ASSERT (PushLock->Locked);
        return TRUE;
    } else {
        return FALSE;
    }
}


VOID
FORCEINLINE
ExReleasePushLock (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Release a push lock that was acquired exclusively or shared

Arguments:

    PushLock - Push lock to be released

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = ReadForWriteAccess (PushLock);

    ASSERT (OldValue.Locked);
    
    if (OldValue.Shared > 1) {
        NewValue.Value = OldValue.Value - EX_PUSH_LOCK_SHARE_INC;
    } else {
        NewValue.Value = 0;
    }

    if (OldValue.Waiting ||
        InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfReleasePushLock (PushLock);
    }
}

VOID
FORCEINLINE
ExReleasePushLockExclusive (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Release a push lock that was acquired exclusively

Arguments:

    PushLock - Push lock to be released

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue;

#if DBG
    OldValue = *PushLock;

    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared == 0);

#endif

#if defined (_WIN64)
    OldValue.Value = InterlockedExchangeAdd64 ((PLONG64)&PushLock->Value, -(LONG64)EX_PUSH_LOCK_LOCK);
#else
    OldValue.Value = InterlockedExchangeAdd ((PLONG)&PushLock->Value, -(LONG)EX_PUSH_LOCK_LOCK);
#endif

    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared == 0);

    if (!OldValue.Waiting || OldValue.Waking) {
        return;
    }

    ExfTryToWakePushLock (PushLock);
}

VOID
FORCEINLINE
ExReleasePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Release a push lock that was acquired shared

Arguments:

    PushLock - Push lock to be released

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

#if DBG

    OldValue = *PushLock;

    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared > 0);

#endif

    OldValue.Value = EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK;
    NewValue.Value = 0;

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) != OldValue.Ptr) {
        ExfReleasePushLockShared (PushLock);
    }
}

VOID
FORCEINLINE
ExReleasePushLockSharedAssumeSingleOwner (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Release a push lock that was acquired shared assuming that we are the only owner

Arguments:

    PushLock - Push lock to be released

Return Value:

    None

--*/
{
#if DBG
    EX_PUSH_LOCK OldValue;

    OldValue = *PushLock;

    ASSERT (OldValue.Locked);
    ASSERT (OldValue.Waiting || OldValue.Shared > 0);

#endif

    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NULL,
                                           (PVOID)(EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK)) !=
                       (PVOID)(EX_PUSH_LOCK_SHARE_INC|EX_PUSH_LOCK_LOCK)) {
        ExfReleasePushLockShared (PushLock);
    }
}

// end_ntosp

//
// This is a block held on the local stack of the waiting threads.
//
typedef  struct DECLSPEC_ALIGN(16) _EX_PUSH_LOCK_WAIT_BLOCK *PEX_PUSH_LOCK_WAIT_BLOCK;

typedef struct DECLSPEC_ALIGN(16) _EX_PUSH_LOCK_WAIT_BLOCK {
    union {
        KGATE WakeGate;
        KEVENT WakeEvent;
    };
    PEX_PUSH_LOCK_WAIT_BLOCK Next;
    PEX_PUSH_LOCK_WAIT_BLOCK Last;
    PEX_PUSH_LOCK_WAIT_BLOCK Previous;
    LONG ShareCount;

#define EX_PUSH_LOCK_FLAGS_EXCLUSIVE  (0x1)
#define EX_PUSH_LOCK_FLAGS_SPINNING_V (0x1)
#define EX_PUSH_LOCK_FLAGS_SPINNING   (0x2)
    LONG Flags;

#if DBG
    BOOLEAN Signaled;
    PVOID OldValue;
    PVOID NewValue;
    PEX_PUSH_LOCK PushLock;
#endif
} DECLSPEC_ALIGN(16) EX_PUSH_LOCK_WAIT_BLOCK;


NTKERNELAPI
VOID
FASTCALL
ExBlockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     );

NTKERNELAPI
VOID
FASTCALL
ExfUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout_opt PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     );

VOID
FORCEINLINE
ExUnblockPushLock (
     IN PEX_PUSH_LOCK PushLock,
     IN PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock OPTIONAL
     )
{
    KeMemoryBarrier ();
    if (WaitBlock != NULL || PushLock->Ptr != NULL) {
        ExfUnblockPushLock (PushLock, WaitBlock);
    }
}


NTKERNELAPI
VOID
FASTCALL
ExWaitForUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     );

NTKERNELAPI
NTSTATUS
FASTCALL
ExTimedWaitForUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock,
     __in_opt PLARGE_INTEGER Timeout
     );

// begin_ntosp

NTKERNELAPI
PEX_PUSH_LOCK_CACHE_AWARE
ExAllocateCacheAwarePushLock (
     VOID
     );

NTKERNELAPI
VOID
ExFreeCacheAwarePushLock (
     __inout PEX_PUSH_LOCK_CACHE_AWARE PushLock
     );

NTKERNELAPI
VOID
ExAcquireCacheAwarePushLockExclusive (
     __inout PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     );

NTKERNELAPI
VOID
ExReleaseCacheAwarePushLockExclusive (
     __inout PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     );

PEX_PUSH_LOCK
FORCEINLINE
ExAcquireCacheAwarePushLockShared (
     IN PEX_PUSH_LOCK_CACHE_AWARE CacheAwarePushLock
     )
/*++

Routine Description:

    Acquire a cache aware push lock shared.

Arguments:

    PushLock - Cache aware push lock to be acquired

Return Value:

    None

--*/
{
    PEX_PUSH_LOCK PushLock;
    //
    // Take a single one of the slots in shared mode.
    // Exclusive acquires must obtain all the slots exclusive.
    //
    PushLock = CacheAwarePushLock->Locks[KeGetCurrentProcessorNumber()%EX_PUSH_LOCK_FANNED_COUNT];
    ExAcquirePushLockSharedAssumeNoOwner (PushLock);
    return PushLock;
}

VOID
FORCEINLINE
ExReleaseCacheAwarePushLockShared (
     IN PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Acquire a cache aware push lock shared.

Arguments:

    PushLock - Part of cache aware push lock returned by ExAcquireCacheAwarePushLockShared

Return Value:

    None

--*/
{
    ExReleasePushLockSharedAssumeSingleOwner (PushLock);

    return;
}

#endif // !defined(NONTOSPINTERLOCK)

// end_ntosp


//
// Define low overhead callbacks for thread create etc
//

// begin_wdm begin_ntddk

//
// Define a block to hold the actual routine registration.
//
typedef NTSTATUS (*PEX_CALLBACK_FUNCTION ) (
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

// end_wdm end_ntddk

typedef struct _EX_CALLBACK_ROUTINE_BLOCK {
    EX_RUNDOWN_REF        RundownProtect;
    PEX_CALLBACK_FUNCTION Function;
    PVOID                 Context;
} EX_CALLBACK_ROUTINE_BLOCK, *PEX_CALLBACK_ROUTINE_BLOCK;

//
// Define a structure the caller uses to hold the callbacks
//
typedef struct _EX_CALLBACK {
    EX_FAST_REF RoutineBlock;
} EX_CALLBACK, *PEX_CALLBACK;

VOID
ExInitializeCallBack (
    IN OUT PEX_CALLBACK CallBack
    );

BOOLEAN
ExCompareExchangeCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK NewBlock,
    IN PEX_CALLBACK_ROUTINE_BLOCK OldBlock
    );

NTSTATUS
ExCallCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

PEX_CALLBACK_ROUTINE_BLOCK
ExAllocateCallBack (
    IN PEX_CALLBACK_FUNCTION Function,
    IN PVOID Context
    );

VOID
ExFreeCallBack (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

PEX_CALLBACK_ROUTINE_BLOCK
ExReferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack
    );

PEX_CALLBACK_FUNCTION
ExGetCallBackBlockRoutine (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

PVOID
ExGetCallBackBlockContext (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

VOID
ExDereferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

VOID
ExWaitForCallBacks (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    );

//
//  Hotpatch declarations
//

extern volatile LONG ExHotpSyncRenameSequence;
extern PKTHREAD ExSyncRenameOwner;

#endif /* _EX_ */
