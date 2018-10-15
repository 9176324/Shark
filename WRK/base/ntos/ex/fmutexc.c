/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    fmutexc.c

Abstract:

    This module implements the code necessary to acquire and release fast
    mutexes.

--*/

#include "exp.h"

#undef ExAcquireFastMutex

VOID
FASTCALL
ExAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
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

    xxAcquireFastMutex(FastMutex);
    return;
}

#undef ExReleaseFastMutex

VOID
FASTCALL
ExReleaseFastMutex (
    __inout PFAST_MUTEX FastMutex
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

    xxReleaseFastMutex(FastMutex);
    return;
}

#undef ExTryToAcquireFastMutex

BOOLEAN
FASTCALL
ExTryToAcquireFastMutex (
    __inout PFAST_MUTEX FastMutex
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

    return xxTryToAcquireFastMutex(FastMutex);
}

#undef ExAcquireFastMutexUnsafe

VOID
FASTCALL
ExAcquireFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
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

    xxAcquireFastMutexUnsafe(FastMutex);
    return;
}

VOID
FASTCALL
ExEnterCriticalRegionAndAcquireFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function enters a critical region, acquires ownership of a fast
    mutex, but does not raise IRQL to APC Level.

    N.B. This is a win32 accelerator routine.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    KeEnterCriticalRegion();
    xxAcquireFastMutexUnsafe(FastMutex);
    return;
}

#undef ExReleaseFastMutexUnsafe

VOID
FASTCALL
ExReleaseFastMutexUnsafe (
    __inout PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function releases ownership to a fast mutex, and does not restore
    IRQL to its previous level.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    xxReleaseFastMutexUnsafe(FastMutex);
    return;
}

VOID
FASTCALL
ExReleaseFastMutexUnsafeAndLeaveCriticalRegion (
    __inout PFAST_MUTEX FastMutex
    )

/*++

Routine Description:

    This function releases ownership to a fast mutex, does not restore IRQL
    to its previous level, and leaves a critical region.

    N.B. This is a win32 accelerator routine.

Arguments:

    FastMutex - Supplies a pointer to a fast mutex.

Return Value:

    None.

--*/

{

    xxReleaseFastMutexUnsafe(FastMutex);
    KeLeaveCriticalRegion();
    return;
}

