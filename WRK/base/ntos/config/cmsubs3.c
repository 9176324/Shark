/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmsubs3.c

Abstract:

    This module contains locking support routines for the configuration manager.

--*/

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpLockRegistry)
#pragma alloc_text(PAGE,CmpLockRegistryExclusive)
#pragma alloc_text(PAGE,CmpUnlockRegistry)

#if DBG
#pragma alloc_text(PAGE,CmpTestRegistryLock)
#pragma alloc_text(PAGE,CmpTestRegistryLockExclusive)
#endif

#pragma alloc_text(PAGE,CmpTryToLockHashEntryByIndexExclusive)
#pragma alloc_text(PAGE,CmpLockTwoHashEntriesExclusive)
#pragma alloc_text(PAGE,CmpLockTwoHashEntriesShared)
#pragma alloc_text(PAGE,CmpUnlockTwoHashEntries)

#if DBG
#pragma alloc_text(PAGE,CmpLockHiveFlusherShared)
#pragma alloc_text(PAGE,CmpLockHiveFlusherExclusive)
#pragma alloc_text(PAGE,CmpUnlockHiveFlusher)
#pragma alloc_text(PAGE,CmpTestHiveFlusherLockShared)
#pragma alloc_text(PAGE,CmpTestHiveFlusherLockExclusive)
#endif

#pragma alloc_text(PAGE,CmpKCBLockForceAcquireAllowed)
#endif


//
// Global registry lock
//

ERESOURCE   CmpRegistryLock;


LONG        CmpFlushStarveWriters = 0;
BOOLEAN     CmpFlushOnLockRelease = FALSE;
LONG        CmRegistryLogSizeLimit = -1;


#if DBG
PVOID       CmpRegistryLockCaller;
PVOID       CmpRegistryLockCallerCaller;
PVOID       CmpKCBLockCaller;
PVOID       CmpKCBLockCallerCaller;
#endif //DBG

extern BOOLEAN CmpSpecialBootCondition;

VOID
CmpLockRegistry(
    VOID
    )
/*++

Routine Description:

    Lock the registry for shared (read-only) access

Arguments:

    None.

Return Value:

    None, the registry lock will be held for shared access upon return.

--*/
{
#if DBG
    PVOID       Caller;
    PVOID       CallerCaller;
#endif

    KeEnterCriticalRegion();

    if( CmpFlushStarveWriters ) {
        //
        // a flush is in progress; starve potential writers
        //
        ExAcquireSharedStarveExclusive(&CmpRegistryLock, TRUE);
    } else {
        //
        // regular shared mode
        //
        ExAcquireResourceSharedLite(&CmpRegistryLock, TRUE);
    }

#if DBG
    RtlGetCallersAddress(&Caller, &CallerCaller);
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_LOCKING,"CmpLockRegistry: c, cc: %p  %p\n", Caller, CallerCaller));
#endif

}

VOID
CmpLockRegistryExclusive(
    VOID
    )
/*++

Routine Description:

    Lock the registry for exclusive (write) access.

Arguments:

    None.

Return Value:

    TRUE - Lock was acquired exclusively

    FALSE - Lock is owned by another thread.

--*/
{
    KeEnterCriticalRegion();
    
    ExAcquireResourceExclusiveLite(&CmpRegistryLock,TRUE);

    ASSERT( CmpFlushStarveWriters == 0 );

#if DBG
    RtlGetCallersAddress(&CmpRegistryLockCaller, &CmpRegistryLockCallerCaller);
#endif //DBG
}

VOID
CmpUnlockRegistry(
    )
/*++

Routine Description:

    Unlock the registry.

--*/
{
    ASSERT_CM_LOCK_OWNED();

    //
    // test if bit set to force flush; and we own the reglock exclusive and ownercount is 1
    //
    if( CmpFlushOnLockRelease && ExIsResourceAcquiredExclusiveLite(&CmpRegistryLock) && (CmpRegistryLock.OwnerThreads[0].OwnerCount == 1) ) {
        //
        // we need to flush now
        //
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
        CmpDoFlushAll(TRUE);
        CmpFlushOnLockRelease = FALSE;
    }
    
    ExReleaseResourceLite(&CmpRegistryLock);
    KeLeaveCriticalRegion();
}


#if DBG

BOOLEAN
CmpTestRegistryLock(VOID)
{
    BOOLEAN rc;

    rc = TRUE;
    if (ExIsResourceAcquiredShared(&CmpRegistryLock) == 0) {
        rc = FALSE;
    }
    return rc;
}

BOOLEAN
CmpTestRegistryLockExclusive(VOID)
{
    if (ExIsResourceAcquiredExclusiveLite(&CmpRegistryLock) == 0) {
        return(FALSE);
    }
    return(TRUE);
}

#endif

BOOLEAN CmpTryConvertKCBLockSharedToExclusive(PCM_KEY_CONTROL_BLOCK Kcb) 
/*++

Routine Description:

    Attempts to convert a shared acquire to exclusive. 

Arguments:

    Kcb - Kcb to be converted

Return Value:

    BOOLEAN - TRUE: Conversion worked ok, FALSE: The conversion could not be achieved

--*/
{
    BOOLEAN Result;

    ASSERT_KCB_LOCKED(Kcb);
    ASSERT( CmpIsKCBLockedExclusive(Kcb) == FALSE );
    Result = ExTryConvertPushLockSharedToExclusive(&(GET_KCB_HASH_ENTRY_LOCK((Kcb))));
    if( Result ) {
        GET_KCB_HASH_ENTRY_OWNER(Kcb) = KeGetCurrentThread();
    }
    return Result;
}

BOOLEAN
CmpTryToLockHashEntryByIndexExclusive(
    ULONG   Index
    )
/*++

Routine Description:

    Try to acquire the entry exclusive 

Arguments:

    None.

Return Value:

    TRUE if acquire was successfull; FALSE otherwise

--*/
{
    BOOLEAN Result;

    ASSERT_CM_LOCK_OWNED();

    Result = ExTryAcquirePushLockExclusive(&(CmpCacheTable[Index].Lock));

    if( Result ) {
        ASSERT( CmpCacheTable[Index].Owner == NULL );
    
        CmpCacheTable[Index].Owner = KeGetCurrentThread();
    }
    return Result;
}


VOID
CmpLockTwoHashEntriesExclusive(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    )
/*++

Routine Description:

    Lock two entries ordered by the index. deadlock avoidance.
    Takes care of not locking the same entry twice.

  NOTE: should be used in pair with UnlockTwoHashEntries

Arguments:

    None.

Return Value:

    None

--*/
{
    ULONG   Index1;
    ULONG   Index2;

    ASSERT_CM_LOCK_OWNED();

    Index1 = GET_HASH_INDEX(ConvKey1);
    Index2 = GET_HASH_INDEX(ConvKey2);

    if( Index1 < Index2 ) {
        CmpLockHashEntryExclusive(ConvKey1);
        CmpLockHashEntryExclusive(ConvKey2);
    } else {
        CmpLockHashEntryExclusive(ConvKey2);
        if( Index1 != Index2 ) {
            CmpLockHashEntryExclusive(ConvKey1);
        }
        
    }
}

VOID
CmpLockTwoHashEntriesShared(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    )
/*++

Routine Description:

    Lock two entries ordered by the index. deadlock avoidance.
    Takes care of not locking the same entry twice.

  NOTE: should be used in pair with UnlockTwoHashEntries

Arguments:

    None.

Return Value:

    None

--*/
{
    ULONG   Index1;
    ULONG   Index2;

    ASSERT_CM_LOCK_OWNED();

    Index1 = GET_HASH_INDEX(ConvKey1);
    Index2 = GET_HASH_INDEX(ConvKey2);

    if( Index1 < Index2 ) {
        CmpLockHashEntryShared(ConvKey1);
        CmpLockHashEntryShared(ConvKey2);
    } else {
        CmpLockHashEntryShared(ConvKey2);
        if( Index1 != Index2 ) {
            CmpLockHashEntryShared(ConvKey1);
        }
        
    }
}

VOID
CmpUnlockTwoHashEntries(
    ULONG   ConvKey1,
    ULONG   ConvKey2
    )
{
    ULONG   Index1;
    ULONG   Index2;

    ASSERT_CM_LOCK_OWNED();

    Index1 = GET_HASH_INDEX(ConvKey1);
    Index2 = GET_HASH_INDEX(ConvKey2);

    ASSERT_HASH_ENTRY_LOCKED(ConvKey2);
    if( Index1 < Index2 ) {
        ASSERT_HASH_ENTRY_LOCKED(ConvKey1);
        CmpUnlockHashEntry(ConvKey2);
        CmpUnlockHashEntry(ConvKey1);
    } else {
        if( Index1 != Index2 ) {
            ASSERT_HASH_ENTRY_LOCKED(ConvKey1);
            CmpUnlockHashEntry(ConvKey1);
        }
        CmpUnlockHashEntry(ConvKey2);
    }
}

//
// GOAL: get rid of the global registry lock.
//

//
// flusher lock: atomic flushes
//
#if DBG
VOID
CmpLockHiveFlusherShared(
    PCMHIVE CmHive
    )
/*++

Routine Description:

    Locks the writer lock for this hive; no cell allocs/free/markdirty are allowed unless this lock is held exclusive

Arguments:

    None.

Return Value:

    None

--*/
{
    ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING(CmHive);

    ASSERT_RESOURCE_NOT_OWNED(CmHive->FlusherLock);

    ExAcquireResourceSharedLite(CmHive->FlusherLock, TRUE);
}

VOID
CmpLockHiveFlusherExclusive(
    PCMHIVE CmHive
    )
/*++

Routine Description:

    Locks the Flusher lock for this hive; 
    
    do that in order to: cell allocs/free/markdirty 

Arguments:

    None.

Return Value:

    None

--*/
{
    ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING(CmHive);

    ASSERT_RESOURCE_NOT_OWNED(CmHive->FlusherLock);

    ExAcquireResourceExclusiveLite(CmHive->FlusherLock,TRUE);
}

VOID
CmpUnlockHiveFlusher(
    PCMHIVE CmHive
    )
/*++

Routine Description:

Arguments:

    None.

Return Value:

    None

--*/
{
    ASSERT_CM_LOCK_OWNED_OR_HIVE_LOADING(CmHive);

    ASSERT_HIVE_FLUSHER_LOCKED(CmHive);

    ExReleaseResourceLite(CmHive->FlusherLock);
}

BOOLEAN
CmpTestHiveFlusherLockShared(
    PCMHIVE CmHive
    )
{
    BOOLEAN rc;

    rc = TRUE;
    if (ExIsResourceAcquiredShared(CmHive->FlusherLock) == 0) {
        rc = FALSE;
    }
    return rc;
}

BOOLEAN
CmpTestHiveFlusherLockExclusive(
    PCMHIVE CmHive
    )
{
    if (ExIsResourceAcquiredExclusiveLite(CmHive->FlusherLock) == 0) {
        return(FALSE);
    }
    return(TRUE);
}
#endif // DBG

BOOLEAN
CmpKCBLockForceAcquireAllowed(ULONG Index1,
                              ULONG Index2,
                              ULONG NewIndex) 
/*++

Routine Description:

    Decides whether we are allowed to force acquire a hash entry bucket; Used for deadlock avoidance

Arguments:

Return Value:

--*/
{
    ASSERT( Index1 != NewIndex );
    ASSERT( Index2 != NewIndex );

    if( Index1 == CmpHashTableSize ) {
        if( Index2 == CmpHashTableSize ) {
            // none locked
            return TRUE;
        } else {
            return ((NewIndex > Index2)?TRUE:FALSE);
        }
    } else {
        if( Index2 == CmpHashTableSize ) {
            return ((NewIndex > Index1)?TRUE:FALSE);
        } else {
            //
            // both indexes valid
            //
            return (((NewIndex > Index1) && (NewIndex > Index2))?TRUE:FALSE);
        }
    }
}
