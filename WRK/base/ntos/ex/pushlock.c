/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pushlock.c

Abstract:

    This module houses routines that do push locking.

    Push locks are capable of being acquired in both shared and exclusive mode.

    Properties include:

    They cannot be acquired recursively.
    They are small (the size of a pointer) and can be used when embedded in pageable data.
    Acquire and release is done lock free. On lock contention the waiters are chained
    through the lock and local stack space.

    This is the structure of a push lock:

    
    L  == Lock bit, This bit is set when the lock is held exclusive or shared
    W  == Waiters present
    K  == Waking threads or optimizing chains. This bit can't be set unless the waiters bit is set.
    M  == Multiple shared owners present. When they release these threads must walk the waitblock list
    SC == Share count
    P  == Pointer to wait block

    +--------------+---+---+---+---+
    |      SC      | M | K | W | L | L, W, K and M are single bits W == 0
    +--------------+---+---+---+---+

    +--------------+---+---+---+---+
    |      P       | M | K | W | L | W == 1. Pointer is the address of a chain of stack local wait blocks
    +--------------+---+---+---+---+

    The non-contented acquires and releases are trivial. Interlocked operations make the following
    transformations.

    Uncontended exclusive acquire. Notice that we can acquire even if there are waiters.

    (SC=0,L=0,W=w,K=k,M=0) === Exclusive acquire ===> (SC=0,L=1,W=w,K=k,M=0)

    There are two cases for shared acquire. Notice that we can only acquire
    in the absence of waiters. The first and later acquires:

    (SC=0,L=0,W=0,K=0,M=0) === First shared acquire ===> (SC=1,  L=1,W=0,K=0,M=0)
    (SC=n,L=1,W=0,K=0,M=0) === later shared acquire ===> (SC=n+1,L=1,W=0,K=0,M=0)

    Uncontended exclusive release:

    (SC=0,L=1,W=0,K=0,M=0) === Exclusive release ===> (SC=0,L=0,W=0,K=0,M=0)

    There are two cases for uncontended shared release. Last and non-last:

    (SC=n,L=1,W=0,K=0,M=0) === Non-last Shared release ===> (SC=n-1,L=1,W=0,K=0,M=0) n > 0
    (SC=1,L=1,W=0,K=0,M=0) ===   last Shared release   ===> (SC=0,  L=0,W=0,K=0,M=0)

    Contention causes the acquiring thread to produce a local stack based wait block and to
    enqueue it to the front of the list.

    Notice that after a contended acquire takes place the K bit is always set.
    It was either set by another thread or this thread. The thread that sets this bit has
    the right to walk the waiters chain and optimize it (place pointers to ending blocks etc).

    Owners of the K bit must either clear the K bit if the lock is still locked or clear the K bit and
    wake at least one thread (if the lock is not locked).

    (SC=n,L=1,W=0,K=0,M=0) === Exclusive acquire ===> (P=LWB(SSC=n,E=e),W=1,K=1,M=m)
        LWB = local wait block,
        SSC = Saved share count,
        E   = Exclusive waiter flag
        m   = Multiple shared owners. m=(n>1)?1:0

    (SC=0,L=1,W=0,K=0,M=0) === Shared acquire    ===> (P=LWB(SSC=0,E=0),W=1,K=1,M=0)
        LWB = local wait block,
        SSC = Saved share count.
        E   = Exclusive waiter flag

    After contention has caused one or more threads to queue up wait blocks releases are more
    complicated. The K bit arbitrates who is allowed to manipulate the waiters list but it does
    not guaranteed complete freedom. The following are the rights to manipulate the list:

    1) New waiters at any time may push themselves onto the list. If they are not the first
       waiter they will also attempt to set the K bit to perform list optimization.
    2) Shared releasers are allowed to walk the list (hopefully optimized by the waiters) to
       find the end of the chain where the share count lives. If they decrement the shared count
       to zero they behave like exclusive releasers and try to set the K bit.
    3) K bit owners, either waiters that pend and manage to set the bit or releasers who manage
       to set the bit have the right to walk the chain and break it and wake. They can't reorder
       without excluding shared walkers (we don't attempt to reorder just put in end pointers).

    The K bit is not a lock in the sense that we don't need to spin on it. Only one owner is needed
    at any time and failure to obtain K bit ownership means some other thread will wake etc.

    The M bit is needed for lock releasers who don't know if they own the lock shared. It also has the
    benefit that only when there are more than one shared owner do we need to traverse the wait list.


Revision History:

    Shifted to not pass on ownership in contended cases to reduce lock hold times.

--*/

#include "exp.h"

#pragma hdrstop

VOID
ExpInitializePushLocks (
    VOID
    );

#ifdef ALLOC_PRAGMA

//
// Since pushlocks are used as the working set synchronization by
// memory management, these routines cannot be pageable.
//
//#pragma alloc_text(PAGE, ExBlockPushLock)
//#pragma alloc_text(PAGE, ExfAcquirePushLockExclusive)
//#pragma alloc_text(PAGE, ExfAcquirePushLockShared)
//#pragma alloc_text(PAGE, ExAllocateCacheAwarePushLock)
//#pragma alloc_text(PAGE, ExFreeCacheAwarePushLock)
//#pragma alloc_text(PAGE, ExAcquireCacheAwarePushLockExclusive)
//#pragma alloc_text(PAGE, ExReleaseCacheAwarePushLockExclusive)

#pragma alloc_text(INIT, ExpInitializePushLocks)

#endif

#if defined (NT_UP)
const ULONG ExPushLockSpinCount = 0;
#else
ULONG ExPushLockSpinCount = 0;
#endif

#if !defined (NT_UP)
#define USE_EXP_BACKOFF
#endif

VOID
ExpInitializePushLocks (
    VOID
    )
/*++

Routine Description:

    Calculates the value of the pushlock spin count

Arguments:

    None

Return Value:

    None

--*/
{

#if defined(_AMD64_)
    EX_PUSH_LOCK ScratchPushLock;
#endif

#if !NT_UP
    if (KeNumberProcessors > 1) {
        ExPushLockSpinCount = 1024;
    } else {
        ExPushLockSpinCount = 0;
    }
#endif

    //
    // Exercise the following routines early, in order to patch any
    // prefetchw instructions therein.
    // 

#if defined(_AMD64_)

    ExInitializePushLock( &ScratchPushLock );

    ExfAcquirePushLockShared( &ScratchPushLock );
    ExfReleasePushLock( &ScratchPushLock );

    ExfAcquirePushLockShared( &ScratchPushLock );
    ExfReleasePushLockShared( &ScratchPushLock );

    ExfAcquirePushLockExclusive( &ScratchPushLock );
    ExfReleasePushLockExclusive( &ScratchPushLock );

#endif
}



NTKERNELAPI
VOID
FASTCALL
ExfWakePushLock (
    IN PEX_PUSH_LOCK PushLock,
    IN EX_PUSH_LOCK TopValue
    )
/*++

Routine Description:

    Walks the pushlock waiting list and wakes waiters if the lock is still unacquired.

Arguments:

    PushLock - Push lock to be walked

    TopValue - Start of the chain (*PushLock)

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;
    PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock, NextWaitBlock, FirstWaitBlock, PreviousWaitBlock;
    KIRQL OldIrql;

    OldValue = TopValue;

    while (1) {

        //
        // Nobody should be walking the list while we manipulate it.
        //

        ASSERT (!OldValue.MultipleShared);

        //
        // No point waking somebody to find a locked lock. Just clear the waking bit
        //

        while (OldValue.Locked) {
            NewValue.Value = OldValue.Value - EX_PUSH_LOCK_WAKING;
            ASSERT (!NewValue.Waking);
            ASSERT (NewValue.Locked);
            ASSERT (NewValue.Waiting);
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                return;
            }
            OldValue = NewValue;
        }

        WaitBlock = (PEX_PUSH_LOCK_WAIT_BLOCK)
           (OldValue.Value & ~(ULONG_PTR)EX_PUSH_LOCK_PTR_BITS);

        FirstWaitBlock = WaitBlock;

        while (1) {

            NextWaitBlock = WaitBlock->Last;
            if (NextWaitBlock != NULL) {
                WaitBlock = NextWaitBlock;
                break;
            }

            PreviousWaitBlock = WaitBlock;
            WaitBlock = WaitBlock->Next;
            WaitBlock->Previous = PreviousWaitBlock;
        }

        if (WaitBlock->Flags&EX_PUSH_LOCK_FLAGS_EXCLUSIVE &&
            (PreviousWaitBlock = WaitBlock->Previous) != NULL) {

            FirstWaitBlock->Last = PreviousWaitBlock;

            WaitBlock->Previous = NULL;

            ASSERT (FirstWaitBlock != WaitBlock);

            ASSERT (PushLock->Waiting);

#if defined (_WIN64)
            InterlockedAnd64 ((LONG64 *)&PushLock->Value, ~EX_PUSH_LOCK_WAKING);
#else
            InterlockedAnd ((LONG *)&PushLock->Value, ~EX_PUSH_LOCK_WAKING);
#endif

            break;
        } else {
            NewValue.Value = 0;
            ASSERT (!NewValue.Waking);
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                break;
            }
            OldValue = NewValue;
        }
    }

    //
    // If we are waking more than one thread then raise to DPC level to prevent us
    // getting rescheduled part way through the operation
    //

    OldIrql = DISPATCH_LEVEL;
    if (WaitBlock->Previous != NULL) {
        KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
    }

    while (1) {

        NextWaitBlock = WaitBlock->Previous;
#if DBG
        ASSERT (!WaitBlock->Signaled);
        WaitBlock->Signaled = TRUE;
#endif

        if (!InterlockedBitTestAndReset (&WaitBlock->Flags, EX_PUSH_LOCK_FLAGS_SPINNING_V)) {
            KeSignalGateBoostPriority (&WaitBlock->WakeGate);
        }

        WaitBlock = NextWaitBlock;
        if (WaitBlock == NULL) {
            break;
        }
    }

    if (OldIrql != DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

VOID
FASTCALL
ExfTryToWakePushLock (
    __inout PEX_PUSH_LOCK PushLock
    )
/*++

Routine Description:

    Tries to set the wake bit and wake the pushlock

Arguments:

    PushLock - Push lock to be woken

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = *PushLock;

    if (OldValue.Waking || OldValue.Locked || !OldValue.Waiting) {
        return;
    }

    NewValue.Value = OldValue.Value + EX_PUSH_LOCK_WAKING;
    if (InterlockedCompareExchangePointer (&PushLock->Ptr,
                                           NewValue.Ptr,
                                           OldValue.Ptr) == OldValue.Ptr) {
        ExfWakePushLock (PushLock, NewValue);
    }
}



VOID
FASTCALL
ExpOptimizePushLockList (
    IN PEX_PUSH_LOCK PushLock,
    IN EX_PUSH_LOCK TopValue
    )
/*++

Routine Description:

    Walks the pushlock waiting list during contention and optimizes it

Arguments:

    PushLock - Push lock to be walked

    TopValue - Start of the chain (*PushLock)

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;
    PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock, PreviousWaitBlock, FirstWaitBlock, NextWaitBlock;

    OldValue = TopValue;
    while (1) {
        if (!OldValue.Locked) {
            ExfWakePushLock (PushLock, OldValue);
            break;
        }

        WaitBlock = (PEX_PUSH_LOCK_WAIT_BLOCK)(OldValue.Value & ~(EX_PUSH_LOCK_PTR_BITS));

        FirstWaitBlock = WaitBlock;

        while (1) {

            NextWaitBlock = WaitBlock->Last;
            if (NextWaitBlock != NULL) {
                FirstWaitBlock->Last = NextWaitBlock;
                break;
            }

            PreviousWaitBlock = WaitBlock;
            WaitBlock = WaitBlock->Next;
            WaitBlock->Previous = PreviousWaitBlock;
        }

        NewValue.Value = OldValue.Value - EX_PUSH_LOCK_WAKING;
        ASSERT (NewValue.Locked);
        ASSERT (!NewValue.Waking);
        if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                               NewValue.Ptr,
                                                               OldValue.Ptr)) == OldValue.Ptr) {
            break;
        }
        OldValue = NewValue;
    }
}


NTKERNELAPI
VOID
DECLSPEC_NOINLINE
FASTCALL
ExfAcquirePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
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
    EX_PUSH_LOCK OldValue, NewValue, TopValue;
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;
    BOOLEAN Optimize;
#if defined (USE_EXP_BACKOFF)
    RTL_BACKOFF Backoff = {0};
#endif

    OldValue = ReadForWriteAccess (PushLock);

    while (1) {
        //
        // If the lock is already held exclusively/shared or there are waiters then
        // we need to wait.
        //
        if ((OldValue.Value&EX_PUSH_LOCK_LOCK) == 0) {
            NewValue.Value = OldValue.Value + EX_PUSH_LOCK_LOCK;

            ASSERT (NewValue.Locked);

            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                break;
            }

#if defined (USE_EXP_BACKOFF)

            //
            // Use backof to limit memory bandwidth
            //
            RtlBackoff (&Backoff);

            NewValue = *PushLock;

#endif
        } else {
            WaitBlock.Flags = EX_PUSH_LOCK_FLAGS_EXCLUSIVE | EX_PUSH_LOCK_FLAGS_SPINNING;
            WaitBlock.Previous = NULL;
            Optimize = FALSE;
 
            //
            // Move the sharecount to our wait block if need be.
            //
            if (OldValue.Waiting) {
                WaitBlock.Last = NULL;
                WaitBlock.Next = (PEX_PUSH_LOCK_WAIT_BLOCK)
                                     (OldValue.Value & ~EX_PUSH_LOCK_PTR_BITS);
                WaitBlock.ShareCount = 0;
                NewValue.Ptr = (PVOID)(((ULONG_PTR) &WaitBlock) |
                                    (OldValue.Value & EX_PUSH_LOCK_PTR_BITS) |
                                    EX_PUSH_LOCK_WAITING | EX_PUSH_LOCK_WAKING |
                                    EX_PUSH_LOCK_LOCK);
                if (!OldValue.Waking) {
                    Optimize = TRUE;
                }
            } else {
                WaitBlock.Last = &WaitBlock;
                WaitBlock.ShareCount = (ULONG) OldValue.Shared;
                if (WaitBlock.ShareCount > 1) {
                    NewValue.Ptr = (PVOID)(((ULONG_PTR) &WaitBlock) | EX_PUSH_LOCK_WAITING |
                                                                      EX_PUSH_LOCK_LOCK |
                                                                      EX_PUSH_LOCK_MULTIPLE_SHARED);
                } else {
                    WaitBlock.ShareCount = 0;
                    NewValue.Ptr = (PVOID)(((ULONG_PTR) &WaitBlock) | EX_PUSH_LOCK_WAITING |
                                                                      EX_PUSH_LOCK_LOCK);
                }
            }
             
#if DBG
            WaitBlock.Signaled = FALSE;
            WaitBlock.OldValue = OldValue.Ptr;
            WaitBlock.NewValue = NewValue.Ptr;
            WaitBlock.PushLock = PushLock;
#endif
            ASSERT (NewValue.Waiting);
            ASSERT (OldValue.Locked);
            ASSERT (NewValue.Locked);

            TopValue = NewValue;
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                ULONG i;

                //
                // If we set the waiting bit then optimize the list
                //
                if (Optimize) {
                    ExpOptimizePushLockList (PushLock, TopValue);
                }

                KeInitializeGate (&WaitBlock.WakeGate);

                for (i = ExPushLockSpinCount; i > 0; i--) {
                    if (((*(volatile LONG *)&WaitBlock.Flags)&EX_PUSH_LOCK_FLAGS_SPINNING) == 0) {
                        break;
                    }
                    KeYieldProcessor ();
                }

                if (InterlockedBitTestAndReset (&WaitBlock.Flags, EX_PUSH_LOCK_FLAGS_SPINNING_V)) {

                    KeWaitForGate (&WaitBlock.WakeGate, WrPushLock, KernelMode);
#if DBG
                    ASSERT (WaitBlock.Signaled);
#endif

                }
                ASSERT ((WaitBlock.ShareCount == 0));
            } else {

#if defined (USE_EXP_BACKOFF)
                //
                // Use backof to limit memory bandwidth
                //
                RtlBackoff (&Backoff);

                NewValue = *PushLock;
#endif
            }
        }
        OldValue = NewValue;
    }

}

NTKERNELAPI
VOID
DECLSPEC_NOINLINE
FASTCALL
ExfAcquirePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
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
    EX_PUSH_LOCK OldValue, NewValue, TopValue;
    EX_PUSH_LOCK_WAIT_BLOCK WaitBlock;
    BOOLEAN Optimize;
#if defined (USE_EXP_BACKOFF)
    RTL_BACKOFF Backoff = {0};
#endif

    OldValue = ReadForWriteAccess (PushLock);

    while (1) {
        //
        // If the lock is already held we need to wait if its not held shared
        //
        if (!OldValue.Locked || (!OldValue.Waiting && OldValue.Shared > 0)) {

            if (OldValue.Waiting) {
                NewValue.Value = OldValue.Value + EX_PUSH_LOCK_LOCK;
            } else {
                NewValue.Value = (OldValue.Value + EX_PUSH_LOCK_SHARE_INC) | EX_PUSH_LOCK_LOCK;
            }
            ASSERT (NewValue.Locked);
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                break;
            }

#if defined (USE_EXP_BACKOFF)

            //
            // Use backof to limit memory bandwidth
            //
            RtlBackoff (&Backoff);

            NewValue = *PushLock;

#endif

        } else {
            WaitBlock.Flags = EX_PUSH_LOCK_FLAGS_SPINNING;
            WaitBlock.ShareCount = 0;
            Optimize = FALSE;
            WaitBlock.Previous = NULL;
 
            //
            // Move the sharecount to our wait block if need be.
            //
            if (OldValue.Waiting) {
                WaitBlock.Last = NULL;
                WaitBlock.Next = (PEX_PUSH_LOCK_WAIT_BLOCK)
                                     (OldValue.Value & ~EX_PUSH_LOCK_PTR_BITS);
                NewValue.Ptr = (PVOID)(((ULONG_PTR) &WaitBlock) |
                                    (OldValue.Value & (EX_PUSH_LOCK_LOCK | EX_PUSH_LOCK_MULTIPLE_SHARED)) |
                                    EX_PUSH_LOCK_WAITING | EX_PUSH_LOCK_WAKING);
                if (!OldValue.Waking) {
                    Optimize = TRUE;
                }
            } else {
                WaitBlock.Last = &WaitBlock;
                NewValue.Ptr = (PVOID)(((ULONG_PTR) &WaitBlock) |
                                    (OldValue.Value & EX_PUSH_LOCK_PTR_BITS) |
                                    EX_PUSH_LOCK_WAITING);
            }
             
            ASSERT (NewValue.Waiting);

#if DBG
            WaitBlock.Signaled = FALSE;
            WaitBlock.OldValue = OldValue.Ptr;
            WaitBlock.NewValue = NewValue.Ptr;
            WaitBlock.PushLock = PushLock;
#endif
            TopValue = NewValue;
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);

            if (NewValue.Ptr == OldValue.Ptr) {
                ULONG i;

                //
                // If we set the waiting bit then optimize the list
                //
                if (Optimize) {
                    ExpOptimizePushLockList (PushLock, TopValue);
                }

                //
                // It is safe to initialize the gate here, as the interlocked operation below forces 
                // a gate signal to always follow gate initialization.
                //
                KeInitializeGate (&WaitBlock.WakeGate);

                for (i = ExPushLockSpinCount; i > 0; i--) {
                    if (((*(volatile LONG *)&WaitBlock.Flags)&EX_PUSH_LOCK_FLAGS_SPINNING) == 0) {
                        break;
                    }
                    KeYieldProcessor ();
                }

                if (InterlockedBitTestAndReset ((LONG*)&WaitBlock.Flags, EX_PUSH_LOCK_FLAGS_SPINNING_V)) {

                    KeWaitForGate (&WaitBlock.WakeGate, WrPushLock, KernelMode);
#if DBG
                    ASSERT (WaitBlock.Signaled);
#endif

                }

            } else {

#if defined (USE_EXP_BACKOFF)

                //
                // Use backof to limit memory bandwidth
                //
                RtlBackoff (&Backoff);

                NewValue = *PushLock;
#endif
            }

        }
        OldValue = NewValue;
    }

}

NTKERNELAPI
BOOLEAN
FASTCALL
ExfTryAcquirePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Try to acquire a push lock shared without blocking

Arguments:

    PushLock - Push lock to be acquired

Return Value:

    BOOLEAN - TRUE, The lock was acquired, FALSE otherwise

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = ReadForWriteAccess (PushLock);

    while (1) {
        //
        // If the lock is already held we need to wait if its not held shared
        //
        if (!OldValue.Locked || (!OldValue.Waiting && OldValue.Shared > 0)) {

            if (OldValue.Waiting) {
                NewValue.Value = OldValue.Value + EX_PUSH_LOCK_LOCK;
            } else {
                NewValue.Value = (OldValue.Value + EX_PUSH_LOCK_SHARE_INC) | EX_PUSH_LOCK_LOCK;
            }
            ASSERT (NewValue.Locked);
            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                return TRUE;
            }
        } else {
            return FALSE;
        }
        OldValue = NewValue;
    }

}

NTKERNELAPI
VOID
DECLSPEC_NOINLINE
FASTCALL
ExfReleasePushLockShared (
     __inout PEX_PUSH_LOCK PushLock
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
    EX_PUSH_LOCK OldValue, NewValue, TopValue;
    PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock, NextWaitBlock;

    OldValue = ReadForWriteAccess (PushLock);

    while (!OldValue.Waiting) {

        if (OldValue.Shared > 1) {
            NewValue.Value = OldValue.Value - EX_PUSH_LOCK_SHARE_INC;
        } else {
            NewValue.Value = 0;
        }

        if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                               NewValue.Ptr,
                                                               OldValue.Ptr)) == OldValue.Ptr) {
            return;
        }
        OldValue = NewValue;
    }

    if (OldValue.MultipleShared) {
        WaitBlock = (PEX_PUSH_LOCK_WAIT_BLOCK) (OldValue.Value & ~(ULONG_PTR)EX_PUSH_LOCK_PTR_BITS);

        while (1) {

            NextWaitBlock = WaitBlock->Last;
            if (NextWaitBlock != NULL) {
                WaitBlock = NextWaitBlock;
                break;
            }

            WaitBlock = WaitBlock->Next;
        }

        ASSERT (WaitBlock->ShareCount > 0);
        ASSERT (WaitBlock->Flags&EX_PUSH_LOCK_FLAGS_EXCLUSIVE);

        if (InterlockedDecrement (&WaitBlock->ShareCount) > 0) {
            return;
        }
    }

    while (1) {

        if (OldValue.Waking) {
            NewValue.Value = OldValue.Value & ~(EX_PUSH_LOCK_LOCK|EX_PUSH_LOCK_MULTIPLE_SHARED);
            ASSERT (NewValue.Waking && !NewValue.Locked && !NewValue.MultipleShared);
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                return;
            }
        } else {
            NewValue.Value = (OldValue.Value & ~(EX_PUSH_LOCK_LOCK |
                                                 EX_PUSH_LOCK_MULTIPLE_SHARED)) |
                                      EX_PUSH_LOCK_WAKING;
            ASSERT (NewValue.Waking && !NewValue.Locked && !NewValue.MultipleShared);
            TopValue = NewValue;
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                ExfWakePushLock (PushLock, TopValue);
                return;
            }
        }

        OldValue = NewValue;
    }
}

NTKERNELAPI
VOID
DECLSPEC_NOINLINE
FASTCALL
ExfReleasePushLockExclusive (
     __inout PEX_PUSH_LOCK PushLock
     )
/*++

Routine Description:

    Release a push lock that was acquired exclusive

Arguments:

    PushLock - Push lock to be released

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue, TopValue;

    OldValue = ReadForWriteAccess (PushLock);

    while (1) {

        ASSERT (OldValue.Locked);
        ASSERT (OldValue.Waiting || OldValue.Shared == 0);

        if (OldValue.Waiting && !OldValue.Waking) {
            NewValue.Value = OldValue.Value - EX_PUSH_LOCK_LOCK + EX_PUSH_LOCK_WAKING;
            ASSERT (NewValue.Waking && !NewValue.Locked);
            TopValue = NewValue;
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                ExfWakePushLock (PushLock, TopValue);
                break;
            }
        } else {
            NewValue.Value = OldValue.Value - EX_PUSH_LOCK_LOCK;
            ASSERT (NewValue.Waking || !NewValue.Waiting);
            if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                   NewValue.Ptr,
                                                                   OldValue.Ptr)) == OldValue.Ptr) {
                break;
            }
        }
        OldValue = NewValue;
    }
}

NTKERNELAPI
VOID
DECLSPEC_NOINLINE
FASTCALL
ExfReleasePushLock (
     __inout PEX_PUSH_LOCK PushLock
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
    EX_PUSH_LOCK OldValue, NewValue, TopValue;
    PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock, NextWaitBlock;

    OldValue = ReadForWriteAccess (PushLock);

    ASSERT (OldValue.Locked);

    while (1) {

        if (!OldValue.Waiting) {

            if (OldValue.Shared > 1) {
                NewValue.Value = OldValue.Value - EX_PUSH_LOCK_SHARE_INC;
            } else {
                NewValue.Value = 0;
            }

            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                              NewValue.Ptr,
                                                              OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                return;
            }
            OldValue = NewValue;
        } else {

            //
            // See if we need to walk the list to find the share count
            //

            if (OldValue.MultipleShared) {
                WaitBlock = (PEX_PUSH_LOCK_WAIT_BLOCK) (OldValue.Value & ~(ULONG_PTR)EX_PUSH_LOCK_PTR_BITS);

                while (1) {

                    NextWaitBlock = WaitBlock->Last;

                    if (NextWaitBlock != NULL) {
                        WaitBlock = NextWaitBlock;
                        break;
                    }

                    WaitBlock = WaitBlock->Next;
                }

                if (WaitBlock->ShareCount > 0) {
                    ASSERT (WaitBlock->Flags&EX_PUSH_LOCK_FLAGS_EXCLUSIVE);

                    if (InterlockedDecrement (&WaitBlock->ShareCount) > 0) {
                        return;
                    }
                }
            }

            while (1) {
                if (OldValue.Waking) {
                    NewValue.Value = OldValue.Value & ~(EX_PUSH_LOCK_LOCK |
                                                        EX_PUSH_LOCK_MULTIPLE_SHARED);
                    ASSERT (NewValue.Waking && !NewValue.Locked && !NewValue.MultipleShared);
                    if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                           NewValue.Ptr,
                                                                           OldValue.Ptr)) == OldValue.Ptr) {
                        return;
                    }
                } else {
                    NewValue.Value = (OldValue.Value & ~(EX_PUSH_LOCK_LOCK |
                                                         EX_PUSH_LOCK_MULTIPLE_SHARED)) |
                                           EX_PUSH_LOCK_WAKING;
                    ASSERT (NewValue.Waking && !NewValue.Locked && !NewValue.MultipleShared);
                    TopValue = NewValue;
                    if ((NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                                           NewValue.Ptr,
                                                                           OldValue.Ptr)) == OldValue.Ptr) {
                        ExfWakePushLock (PushLock, TopValue);
                        return;
                    }
                }
                OldValue = NewValue;
            }
        }
    }
}

NTKERNELAPI
VOID
FASTCALL
ExfConvertPushLockExclusiveToShared (
     __inout PEX_PUSH_LOCK PushLock
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
    EX_PUSH_LOCK OldValue, NewValue;

    OldValue = ReadForWriteAccess (PushLock);

    while (1) {
        ASSERT (OldValue.Locked);
        ASSERT (OldValue.Waiting || OldValue.Shared == 0);

        if (!OldValue.Waiting) {

            NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                               (PVOID) (EX_PUSH_LOCK_LOCK|EX_PUSH_LOCK_SHARE_INC),
                                               OldValue.Ptr);
            if (NewValue.Ptr == OldValue.Ptr) {
                return;
            }

            OldValue = NewValue;
        } else {
            return;
        }
    }
}

NTKERNELAPI
VOID
FASTCALL
ExBlockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     )
/*++

Routine Description:

    Block on a push lock

Arguments:

    PushLock  - Push lock to block on
    WaitBlock - Wait block to queue for waiting

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue, NewValue;

    //
    // Mark the wait block as not waiting yet
    //

    WaitBlock->Flags = EX_PUSH_LOCK_FLAGS_SPINNING;

    //
    // Push the wait block on the list. 
    //

    OldValue = ReadForWriteAccess (PushLock);
    while (1) {
        //
        // Chain the next block to us if there is one.
        //
        WaitBlock->Next = OldValue.Ptr;
        NewValue.Ptr = InterlockedCompareExchangePointer (&PushLock->Ptr,
                                                          WaitBlock,
                                                          OldValue.Ptr);
        if (NewValue.Ptr == OldValue.Ptr) {
            return;
        }
        OldValue = NewValue;
    }
}


NTKERNELAPI
VOID
FASTCALL
ExWaitForUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     )
{
    ExTimedWaitForUnblockPushLock (PushLock,
                                   WaitBlock,
                                   NULL);
}

NTKERNELAPI
NTSTATUS
FASTCALL
ExTimedWaitForUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock,
     __in_opt PLARGE_INTEGER Timeout
     )
{
    ULONG i;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER (PushLock);

    KeInitializeEvent (&WaitBlock->WakeEvent, SynchronizationEvent, FALSE);

    for (i = ExPushLockSpinCount; i > 0; i--) {
        if (((*(volatile LONG *)&WaitBlock->Flags)&EX_PUSH_LOCK_FLAGS_SPINNING) == 0) {
            return STATUS_SUCCESS;
        }
        KeYieldProcessor ();
    }


    if (InterlockedBitTestAndReset (&WaitBlock->Flags, EX_PUSH_LOCK_FLAGS_SPINNING_V)) {
        Status = KeWaitForSingleObject (&WaitBlock->WakeEvent,
                                        WrPushLock,
                                        KernelMode,
                                        FALSE,
                                        Timeout);
        if (Status != STATUS_SUCCESS) {
            ExfUnblockPushLock (PushLock,
                                WaitBlock);
        }
    } else {
        Status = STATUS_SUCCESS;
    }
    return Status;
}

NTKERNELAPI
VOID
FASTCALL
ExfUnblockPushLock (
     __inout PEX_PUSH_LOCK PushLock,
     __inout_opt PEX_PUSH_LOCK_WAIT_BLOCK WaitBlock
     )
/*++

Routine Description:

    Unblock on a push lock

Arguments:

    PushLock  - Push lock to block on
    WaitBlock - Wait block previously queued for waiting or NULL if there wasn't one

Return Value:

    None

--*/
{
    EX_PUSH_LOCK OldValue;
    PEX_PUSH_LOCK_WAIT_BLOCK tWaitBlock;
    KIRQL OldIrql;

    //
    // Pop the entire chain and wake them all up.
    //
    OldValue.Ptr = InterlockedExchangePointer (&PushLock->Ptr,
                                               NULL);

    tWaitBlock = OldValue.Ptr;

    //
    // If it looks like we will wake more than one thread then raise IRQL to prevent
    // rescheduling during the operation.
    //
    OldIrql = DISPATCH_LEVEL;
    if (tWaitBlock != NULL && tWaitBlock->Next != NULL) {
        KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
    }

    while (tWaitBlock != NULL) {
        OldValue.Ptr = tWaitBlock->Next;

        if (!InterlockedBitTestAndReset (&tWaitBlock->Flags, EX_PUSH_LOCK_FLAGS_SPINNING_V)) {
            KeSetEventBoostPriority (&tWaitBlock->WakeEvent, NULL);
        }

        tWaitBlock = OldValue.Ptr;
    }

    if (OldIrql != DISPATCH_LEVEL) {
        KeLowerIrql (OldIrql);
    }

    if (WaitBlock != NULL && (WaitBlock->Flags&EX_PUSH_LOCK_FLAGS_SPINNING) != 0) {
        ExWaitForUnblockPushLock (PushLock, WaitBlock);
    }
}

NTKERNELAPI
PEX_PUSH_LOCK_CACHE_AWARE
ExAllocateCacheAwarePushLock (
     VOID
     )
/*++

Routine Description:

    Allocate a cache aware (cache friendly) push lock

Arguments:

    None

Return Value:

    None

--*/
{
    PEX_PUSH_LOCK_CACHE_AWARE PushLockCacheAware;
    PEX_PUSH_LOCK_CACHE_AWARE_PADDED PaddedPushLock;
    ULONG i, j, MaxLine;

    PushLockCacheAware = ExAllocatePoolWithTag (PagedPool,
                                                sizeof (EX_PUSH_LOCK_CACHE_AWARE),
                                                'pclP');
    if (PushLockCacheAware != NULL) {
        //
        // If we are a non-numa machine then allocate the padded push locks as a single block
        //
        if (KeNumberNodes == 1) {
            PaddedPushLock = ExAllocatePoolWithTag (PagedPool,
                                                    sizeof (EX_PUSH_LOCK_CACHE_AWARE_PADDED)*
                                                       EX_PUSH_LOCK_FANNED_COUNT,
                                                    'lclP');
            if (PaddedPushLock == NULL) {
                ExFreePool (PushLockCacheAware);
                return NULL;
            }
            for (i = 0; i < EX_PUSH_LOCK_FANNED_COUNT; i++) {
                PaddedPushLock->Single = TRUE;
                ExInitializePushLock (&PaddedPushLock->Lock);
                PushLockCacheAware->Locks[i] = &PaddedPushLock->Lock;
                PaddedPushLock++;
            }
        } else {
            //
            // Allocate a different block for each lock and set affinity
            // so the allocation comes from that node's memory.
            //
            MaxLine = KeNumberProcessors;
            if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
                MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
            }

            for (i = 0; i < MaxLine; i++) {
                KeSetSystemAffinityThread (AFFINITY_MASK (i));
                PaddedPushLock = ExAllocatePoolWithTag (PagedPool,
                                                        sizeof (EX_PUSH_LOCK_CACHE_AWARE_PADDED),
                                                        'lclP');
                if (PaddedPushLock == NULL) {
                    for (j = 0; j < i; j++) {
                        ExFreePool (PushLockCacheAware->Locks[j]);
                    }
                    KeRevertToUserAffinityThread ();

                    ExFreePool (PushLockCacheAware);
                    return NULL;
                }
                PaddedPushLock->Single = FALSE;
                ExInitializePushLock (&PaddedPushLock->Lock);
                PushLockCacheAware->Locks[i] = &PaddedPushLock->Lock;
            }
            KeRevertToUserAffinityThread ();
        }
        
    }
    return PushLockCacheAware;
}

NTKERNELAPI
VOID
ExFreeCacheAwarePushLock (
     __inout PEX_PUSH_LOCK_CACHE_AWARE PushLock     
     )
/*++

Routine Description:

    Frees a cache aware (cache friendly) push lock

Arguments:

    PushLock - Cache aware push lock to be freed

Return Value:

    None

--*/
{
    ULONG i;
    ULONG MaxLine;

    if (!CONTAINING_RECORD (PushLock->Locks[0], EX_PUSH_LOCK_CACHE_AWARE_PADDED, Lock)->Single) {
        MaxLine = KeNumberProcessors;
        if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
            MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
        }

        for (i = 0; i < MaxLine; i++) {
            ExFreePool (PushLock->Locks[i]);
        }
    } else {
        ExFreePool (PushLock->Locks[0]);
    }
    ExFreePool (PushLock);
}


NTKERNELAPI
VOID
ExAcquireCacheAwarePushLockExclusive (
     __inout PEX_PUSH_LOCK_CACHE_AWARE PushLock
     )
/*++

Routine Description:

    Acquire a cache aware push lock exclusive.

Arguments:

    PushLock - Cache aware push lock to be acquired

Return Value:

    None

--*/
{
    PEX_PUSH_LOCK *Start, *End;
    ULONG MaxLine;

    //
    // Exclusive acquires must obtain all the slots exclusive.
    // Take the first slot exclusive and then we can take the
    // rest of the slots in any order we want.
    // There is no deadlock here. A->B->C does not deadlock with A->C->B.
    //
    Start = &PushLock->Locks[1];
    MaxLine = KeNumberProcessors;
    if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
        MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
    }
    End   = &PushLock->Locks[MaxLine - 1];

    ExAcquirePushLockExclusive (PushLock->Locks[0]);

    while (Start <= End) {
        if (ExTryAcquirePushLockExclusive (*Start)) {
            Start++;
        } else {
            ExAcquirePushLockExclusive (*End);
            End--;
        }
    }
}

NTKERNELAPI
VOID
ExReleaseCacheAwarePushLockExclusive (
     __inout PEX_PUSH_LOCK_CACHE_AWARE PushLock
     )
/*++

Routine Description:

    Release a cache aware push lock exclusive.

Arguments:

    PushLock - Cache aware push lock to be released

Return Value:

    None

--*/
{
    PEX_PUSH_LOCK *Start, *End;
    ULONG MaxLine;

    //
    // Release the locks in order
    //

    MaxLine = KeNumberProcessors;
    if (MaxLine > EX_PUSH_LOCK_FANNED_COUNT) {
        MaxLine = EX_PUSH_LOCK_FANNED_COUNT;
    }
    End   = &PushLock->Locks[MaxLine];
    for (Start = &PushLock->Locks[0];
         Start < End;
         Start++) {
        ExReleasePushLockExclusive (*Start);
    }
}

