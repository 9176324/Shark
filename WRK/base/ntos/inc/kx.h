/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kx.h

Abstract:

    This module contains the public (external) header file for the kernel
    that must be included after all other header files.

    WARNING: There is code in windows\core\ntgdi\gre\i386\locka.asm that
             mimics the functions to enter and leave critical regions.
             Any changes to the subject routines must be reflected in locka.asm also.

--*/

#ifndef _KX_
#define _KX_

NTKERNELAPI
VOID
KiCheckForKernelApcDelivery (
    VOID
    );

VOID
FASTCALL
KiAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    );

FORCEINLINE
VOID
KeEnterGuardedRegionThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function disables special kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    Thread - Supplies a pointer to the current thread.

    N.B. This must be a pointer to the current thread.

Return Value:

    None.

--*/

{

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT((Thread->SpecialApcDisable <= 0) && (Thread->SpecialApcDisable != -32768));

    Thread->SpecialApcDisable -= 1;
    KeMemoryBarrierWithoutFence();
    return;
}

FORCEINLINE
VOID
KeEnterGuardedRegion (
    VOID
    )

/*++

Routine Description:

    This function disables special kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KeEnterGuardedRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeLeaveGuardedRegionThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function enables special kernel APC's.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    Thread - Supplies a pointer to the current thread.

    N.B. This must be a pointer to the current thread.

Return Value:

    None.

--*/

{

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT(Thread->SpecialApcDisable < 0);

    KeMemoryBarrierWithoutFence();
    if ((Thread->SpecialApcDisable += 1) == 0) { 
        KeMemoryBarrierWithoutFence();
        if (Thread->ApcState.ApcListHead[KernelMode].Flink !=       
                                &Thread->ApcState.ApcListHead[KernelMode]) {

            KiCheckForKernelApcDelivery();
        }                                                             
    }                                                                 

    return;
}

FORCEINLINE
VOID
KeLeaveGuardedRegion (
    VOID
    )

/*++

Routine Description:

    This function enables special kernel APC's.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KeLeaveGuardedRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeEnterCriticalRegionThread (
    PKTHREAD Thread
    )

/*++

Routine Description:

    This function disables kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    Thread - Supplies a pointer to the current thread.

    N.B. This must be a pointer to the current thread.

Return Value:

    None.

--*/

{

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT((Thread->KernelApcDisable <= 0) && (Thread->KernelApcDisable != -32768));

    Thread->KernelApcDisable -= 1;
    KeMemoryBarrierWithoutFence();
    return;
}

FORCEINLINE
VOID
KeEnterCriticalRegion (
    VOID
    )

/*++

Routine Description:

    This function disables kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KeEnterCriticalRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
VOID
KeLeaveCriticalRegionThread (
    IN PKTHREAD Thread
    )

/*++

Routine Description:

    This function enables normal kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    Thread - Supplies a pointer to the current thread.

    N.B. This must be a pointer to the current thread.

Return Value:

    None.

--*/

{

    ASSERT(Thread == KeGetCurrentThread());

    ASSERT(Thread->KernelApcDisable < 0);

    KeMemoryBarrierWithoutFence();
    if ((Thread->KernelApcDisable += 1) == 0) {
        KeMemoryBarrierWithoutFence();
        if (Thread->ApcState.ApcListHead[KernelMode].Flink !=         
                                &Thread->ApcState.ApcListHead[KernelMode]) {

            if (Thread->SpecialApcDisable == 0) {
                KiCheckForKernelApcDelivery();
            }
        }                                                               
    }

    return;
}

FORCEINLINE
VOID
KeLeaveCriticalRegion (
    VOID
    )

/*++

Routine Description:

    This function enables normal kernel APC's for the current thread.

    N.B. The following code does not require any interlocks. There are
         two cases of interest: 1) On an MP system, the thread cannot
         be running on two processors as once, and 2) if the thread is
         is interrupted to deliver a kernel mode APC which also calls
         this routine, the values read and stored will stack and unstack
         properly.

Arguments:

    None.

Return Value:

    None.

--*/

{

    KeLeaveCriticalRegionThread(KeGetCurrentThread());
    return;
}

FORCEINLINE
BOOLEAN
KeAreApcsDisabled (
    VOID
    )

/*++

Routine description:

    This function returns whether kernel are disabled for the current thread.

Arguments:

    None.

Return Value:

    If either the kernel or special APC disable count is nonzero, then a value
    of TRUE is returned. Otherwise, a value of FALSE is returned.

--*/

{

    return (BOOLEAN)(KeGetCurrentThread()->CombinedApcDisable != 0);
}

FORCEINLINE
BOOLEAN
KeAreAllApcsDisabled (
    VOID
    )

/*++

Routine description:

    This function returns whether all APCs are disabled for the current thread.

Arguments:

    None.

Return Value:

    If either the special APC disable count is nonzero or the IRQL is greater
    than or equal to APC_LEVEL, then a value of TRUE is returned. Otherwise,
    a value of FALSE is returned.

--*/

{

    return (BOOLEAN)((KeGetCurrentThread()->SpecialApcDisable != 0) ||
                     (KeGetCurrentIrql() >= APC_LEVEL));
}

FORCEINLINE
VOID
FASTCALL
KeInitializeGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function initializes a guarded mutex.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    Mutex->Owner = NULL;
    Mutex->Count = GM_LOCK_BIT;
    Mutex->Contention = 0;
    KeInitializeGate(&Mutex->Gate);
    return;
}

FORCEINLINE
VOID
FASTCALL
KeAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function enters a guarded region and acquires ownership of a guarded
    mutex.

Arguments:

    Mutex  - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    PKTHREAD Thread;

    //
    // Enter a guarded region and attempt to acquire ownership of the
    // guarded mutex.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    Thread = KeGetCurrentThread();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Mutex->Owner != Thread);

    KeEnterGuardedRegionThread(Thread);
    if (!InterlockedBitTestAndReset(&Mutex->Count, GM_LOCK_BIT_V)) {

        //
        // The guarded mutex is owned - take the slow path.
        //

        KiAcquireGuardedMutex(Mutex);
    }

    //
    // Grant ownership of the guarded mutex to the current thread.
    //

    Mutex->Owner = Thread;

#if DBG

    Mutex->SpecialApcDisable = Thread->SpecialApcDisable;

#endif

    return;
}

FORCEINLINE
VOID
FASTCALL
KeReleaseGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function releases ownership of a guarded mutex and leaves a guarded
    region.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    LONG NewValue;
    LONG OldValue;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    ASSERT(Mutex->Owner == KeGetCurrentThread());

    ASSERT(KeGetCurrentThread()->SpecialApcDisable == Mutex->SpecialApcDisable);

    //
    // Clear the owner thread and attempt to wake a waiter.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    Mutex->Owner = NULL;
    OldValue = InterlockedExchangeAdd(&Mutex->Count, GM_LOCK_BIT);

    ASSERT((OldValue & GM_LOCK_BIT) == 0);

    //
    // If there are no waiters or a waiter has already been woken, then
    // release the mutex. Otherwise, attempt to wake a waiter.
    //

    if ((OldValue != 0) &&
        ((OldValue & GM_LOCK_WAITER_WOKEN) == 0)) {

        //
        // There must be at least one waiter that needs to be woken. Set the
        // woken waiter bit and decrement the waiter count. If the exchange
        // fails, then another thread will do the wake.
        //

        OldValue = OldValue + GM_LOCK_BIT;
        NewValue = OldValue + GM_LOCK_WAITER_WOKEN - GM_LOCK_WAITER_INC;
        if (InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue) == OldValue) {

            //
            // Wake one waiter.
            //

            KeSignalGateBoostPriority(&Mutex->Gate);
        }
    }

    //
    // Leave guarded region.
    //

    KeLeaveGuardedRegion();
    return;
}

FORCEINLINE
BOOLEAN
FASTCALL
KeTryToAcquireGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function attempts to acquire ownership of a guarded mutex, and if
    successful, enters a guarded region.

Arguments:

    Mutex  - Supplies a pointer to a guarded mutex.

Return Value:

    If the guarded mutex was successfully acquired, then a value of TRUE
    is returned as the function value. Otherwise, a value of FALSE is
    returned.

--*/

{

    PKTHREAD Thread;

    //
    // Enter a guarded region and attempt to acquire ownership of the
    // guarded mutex.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    Thread = KeGetCurrentThread();

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    KeEnterGuardedRegionThread(Thread);
    if (!InterlockedBitTestAndReset(&Mutex->Count, GM_LOCK_BIT_V)) {

        //
        // The guarded mutex is owned - leave the guarded region and return
        // FALSE.
        //

        KeLeaveGuardedRegionThread(Thread);
        KeYieldProcessor();
        return FALSE;

    } else {

        //
        // Grant ownership of the guarded mutex to the current thread and
        // return TRUE.
        //

        Mutex->Owner = Thread;

#if DBG

        Mutex->SpecialApcDisable = Thread->SpecialApcDisable;

#endif

        return TRUE;
    }
}

FORCEINLINE
VOID
FASTCALL
KeAcquireGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function acquires ownership of a guarded mutex, but does enter a
    guarded region.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    PKTHREAD Thread;

    //
    // Attempt to acquire ownership of the guarded mutex.
    //
    // N.B. The first operation is a write to the guarded mutex.
    //

    Thread = KeGetCurrentThread();

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (Thread->SpecialApcDisable < 0) ||
           (Thread->Teb == NULL) ||
           (Thread->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(Mutex->Owner != Thread);

    if (!InterlockedBitTestAndReset(&Mutex->Count, GM_LOCK_BIT_V)) {

        //
        // The guarded mutex is already owned - take the slow path.
        //

        KiAcquireGuardedMutex(Mutex);
    }

    //
    // Grant ownership of the guarded mutex to the current thread.
    //

    Mutex->Owner = Thread;
    return;
}

FORCEINLINE
VOID
FASTCALL
KeReleaseGuardedMutexUnsafe (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function releases ownership of a guarded mutex, and does not leave
    a guarded region.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    None.

--*/

{

    LONG NewValue;
    LONG OldValue;

    ASSERT((KeGetCurrentIrql() == APC_LEVEL) ||
           (KeGetCurrentThread()->SpecialApcDisable < 0) ||
           (KeGetCurrentThread()->Teb == NULL) ||
           (KeGetCurrentThread()->Teb >= MM_SYSTEM_RANGE_START));

    ASSERT(Mutex->Owner == KeGetCurrentThread());

    //
    // Clear the owner thread and attempt to wake a waiter.
    //
    // N.B. The first operation performed on the mutex is a write.
    //

    Mutex->Owner = NULL;
    OldValue = InterlockedExchangeAdd(&Mutex->Count, GM_LOCK_BIT);

    ASSERT((OldValue & GM_LOCK_BIT) == 0);

    //
    // If there are no waiters or a waiter has already been woken, then
    // release the mutex. Otherwise, attempt to wake a waiter.
    //

    if ((OldValue != 0) &&
        ((OldValue & GM_LOCK_WAITER_WOKEN) == 0)) {

        //
        // There must be at least one waiter that needs to be woken. Set the
        // woken waiter bit and decrement the waiter count. If the exchange
        // fails, then another thread will do the wake.
        //

        OldValue = OldValue + GM_LOCK_BIT;
        NewValue = OldValue + GM_LOCK_WAITER_WOKEN - GM_LOCK_WAITER_INC;
        if (InterlockedCompareExchange(&Mutex->Count, NewValue, OldValue) == OldValue) {

            //
            // Wake one waiter.
            //

            KeSignalGateBoostPriority(&Mutex->Gate);
        }
    }

    return;
}

FORCEINLINE
PKTHREAD
FASTCALL
KeGetOwnerGuardedMutex (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function returns the owner of the specified guarded mutex.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    If the guarded mutex is owned, then a pointer to the owner thread is
    returned. Otherwise, NULL is returned.

--*/

{
    return Mutex->Owner;
}

FORCEINLINE
BOOLEAN
FASTCALL
KeIsGuardedMutexOwned (
    IN PKGUARDED_MUTEX Mutex
    )

/*++

Routine Description:

    This function tests whether the specified guarded mutex is owned.

Arguments:

    Mutex - Supplies a pointer to a guarded mutex.

Return Value:

    A value of TRUE is returned if the guarded mutex is owned. Otherwise,
    a value of FALSE is returned.

--*/

{
    return (BOOLEAN)((Mutex->Count & GM_LOCK_BIT) == 0);
}

#endif
