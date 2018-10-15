/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmdelay.c

Abstract:

    This module implements the new algorithm (LRU style) for the 
    Delayed Close KCB table.

    Functions in this module are thread safe protected by the kcb lock.
    When kcb lock is converted to a resource, we should assert (enforce)
    exclusivity of that resource here !!!

Note:
    
    We might want to convert these functions to macros after enough testing
    provides that they work well

--*/

#include    "cmp.h"

ULONG                   CmpDelayedCloseSize = 2048; // !!!! Cannot be bigger that 4094 !!!!!
ULONG                   CmpDelayedCloseElements = 0; 

#define MAX_DELAY_WORKER_ITERATIONS     ( CmpDelayedCloseSize / 4 )

VOID
CmpDelayDerefKCBWorker(
    IN PVOID Parameter
    );

VOID
CmpDelayCloseWorker(
    IN PVOID Parameter
    );

typedef struct _CM_DELAY_DEREF_KCB_ITEM {
    LIST_ENTRY              ListEntry;
    PCM_KEY_CONTROL_BLOCK   Kcb;
} CM_DELAY_DEREF_KCB_ITEM, *PCM_DELAY_DEREF_KCB_ITEM;

VOID
CmpDelayCloseDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CmpDelayDerefKCBDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CmpDoQueueLateUnloadWorker(IN PCMHIVE CmHive);

VOID
CmpArmDelayedCloseTimer(VOID);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpInitializeDelayedCloseTable)
#pragma alloc_text(PAGE,CmpRemoveFromDelayedClose)
#pragma alloc_text(PAGE,CmpAddToDelayedClose)

#pragma alloc_text(PAGE,CmpInitDelayDerefKCBEngine)
#pragma alloc_text(PAGE,CmpRunDownDelayDerefKCBEngine)
#pragma alloc_text(PAGE,CmpDelayDerefKeyControlBlock)
#pragma alloc_text(PAGE,CmpDelayDerefKCBWorker)
#pragma alloc_text(PAGE,CmpArmDelayedCloseTimer)
#endif

WORK_QUEUE_ITEM CmpDelayCloseWorkItem;
BOOLEAN         CmpDelayCloseWorkItemActive = FALSE;

KGUARDED_MUTEX  CmpDelayedCloseTableLock;                
LIST_ENTRY      CmpDelayedLRUListHead;          // head of the LRU list of Delayed Close Table entries

KTIMER          CmpDelayCloseTimer;
KDPC            CmpDelayCloseDpc;

ULONG           CmpDelayCloseIntervalInSeconds = 5;


#define LOCK_DELAY_CLOSE() KeAcquireGuardedMutex(&CmpDelayedCloseTableLock)
#define UNLOCK_DELAY_CLOSE() KeReleaseGuardedMutex(&CmpDelayedCloseTableLock)

VOID
CmpDelayCloseDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This is the DPC routine triggered by the delayclose timer.  
    is queue a work item to an executive worker thread.  

Arguments:

    Dpc - Supplies a pointer to the DPC object.

    DeferredContext - not used

    SystemArgument1 - not used

    SystemArgument2 - not used

Return Value:

    None.

--*/

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    ASSERT(CmpDelayCloseWorkItemActive);
    ExQueueWorkItem(&CmpDelayCloseWorkItem, DelayedWorkQueue);
}


#define CmpDelayCloseAllocNewEntry() (PCM_DELAYED_CLOSE_ENTRY)CmpAllocateDelayItem()
#define CmpDelayCloseFreeEntry(Item) CmpFreeDelayItem((PVOID)(Item))

VOID
CmpInitializeDelayedCloseTable()
/*++

Routine Description:

    Initialize delayed close table; allocation + LRU list initialization.

Arguments:


Return Value:

    NONE.

--*/
{
    ExInitializeWorkItem(&CmpDelayCloseWorkItem, CmpDelayCloseWorker, NULL);
    KeInitializeGuardedMutex(&CmpDelayedCloseTableLock);
    InitializeListHead(&(CmpDelayedLRUListHead));
    KeInitializeDpc(&CmpDelayCloseDpc,
                    CmpDelayCloseDpcRoutine,
                    NULL);

    KeInitializeTimer(&CmpDelayCloseTimer);

}

VOID
CmpDelayCloseWorker(
    IN PVOID Parameter
    )

/*++

Routine Description:
    
      The fun part. We need to walk the cache and look for kcbs with refcount == 0
      Get the oldest one and kick it out of cache.

Arguments:

    Parameter - not used.

Return Value:

    None.

--*/

{
    PCM_DELAYED_CLOSE_ENTRY     DelayedEntry;
    ULONG                       ConvKey;
    ULONG                       MaxIterations = MAX_DELAY_WORKER_ITERATIONS;

    CM_PAGED_CODE();

    UNREFERENCED_PARAMETER (Parameter);

    ASSERT(CmpDelayCloseWorkItemActive);

    BEGIN_LOCK_CHECKPOINT;
    CmpLockRegistry();
    
    //
    // process kick out every entry with RefCount == 0 && DelayCloseIndex == 0
    // ignore the others; we only do this while there is excess of delay - close kcbs
    //
    LOCK_DELAY_CLOSE();
    while( (CmpDelayedCloseElements > CmpDelayedCloseSize) && (MaxIterations--) ) {
        ASSERT( !CmpIsListEmpty(&CmpDelayedLRUListHead) );
        //
        // We first need to get the hash entry and attempt to lock it.
        //
        DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)(CmpDelayedLRUListHead.Blink);
        DelayedEntry = CONTAINING_RECORD(   DelayedEntry,
                                            CM_DELAYED_CLOSE_ENTRY,
                                            DelayedLRUList);
        ConvKey = DelayedEntry->KeyControlBlock->ConvKey;
        UNLOCK_DELAY_CLOSE();
        //
        // now lock the hash then recheck if the entry is still first on the list
        //
        CmpLockHashEntryExclusive(ConvKey);
        LOCK_DELAY_CLOSE();
        if( CmpDelayedCloseElements <= CmpDelayedCloseSize ) {
            //
            // just bail out; no need to kick them out
            //
            CmpUnlockHashEntry(ConvKey);
            break;
        }
        DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)(CmpDelayedLRUListHead.Blink);
        DelayedEntry = CONTAINING_RECORD(   DelayedEntry,
                                            CM_DELAYED_CLOSE_ENTRY,
                                            DelayedLRUList);
        if( ConvKey == DelayedEntry->KeyControlBlock->ConvKey ) {
            //
            // same hash entry; proceed
            // pull it out of the list
            //
            DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)RemoveTailList(&CmpDelayedLRUListHead);
    
            DelayedEntry = CONTAINING_RECORD(   DelayedEntry,
                                                CM_DELAYED_CLOSE_ENTRY,
                                                DelayedLRUList);
            CmpClearListEntry(&(DelayedEntry->DelayedLRUList));

            if( (DelayedEntry->KeyControlBlock->RefCount == 0) && (DelayedEntry->KeyControlBlock->DelayedCloseIndex == 0) ){
                //
                // free this kcb and the entry
                //
                UNLOCK_DELAY_CLOSE();
                DelayedEntry->KeyControlBlock->DelayCloseEntry = NULL;
                CmpCleanUpKcbCacheWithLock(DelayedEntry->KeyControlBlock,FALSE);
                CmpDelayCloseFreeEntry(DelayedEntry);
                InterlockedDecrement((PLONG)&CmpDelayedCloseElements);
            } else {
                //
                // put it back at the top
                //
                InsertHeadList( &CmpDelayedLRUListHead,
                                &(DelayedEntry->DelayedLRUList)
                            );
                UNLOCK_DELAY_CLOSE();
            }
        } else {
            UNLOCK_DELAY_CLOSE();
        }
        CmpUnlockHashEntry(ConvKey);

        LOCK_DELAY_CLOSE();
    }
    if( CmpDelayedCloseElements > CmpDelayedCloseSize ) {
        //
        // iteration run was too short, there are more elements to process, queue ourselves for later
        //
        CmpArmDelayedCloseTimer();
    } else {
        //
        // signal that we have finished our work.
        //
        CmpDelayCloseWorkItemActive = FALSE;
    }
    UNLOCK_DELAY_CLOSE();


    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;
}

VOID
CmpArmDelayedCloseTimer( )

/*++

Routine Description:

    Arms timer for DelayClose worker procession

    NB: this routine should be called with CmpDelayedCloseTableLock held. 
        If not --> change CmpDelayCloseWorkItemActive to a ULONG and do an InterlockedCompareExchange on it 
        before setting the timer

Arguments:


Note: 
    

Return Value:

    NONE.

--*/
{
    LARGE_INTEGER DueTime;

    CM_PAGED_CODE();
    
    CmpDelayCloseWorkItemActive = TRUE;

    //
    // queue a timer for 5 secs later to do the actual delay close
    //

    DueTime.QuadPart = Int32x32To64(CmpDelayCloseIntervalInSeconds,
                                    - SECOND_MULT);
    //
    // Indicate relative time
    //

    KeSetTimer(&CmpDelayCloseTimer,
               DueTime,
               &CmpDelayCloseDpc);

}

VOID
CmpRemoveFromDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    )
/*++

Routine Description:

    Removes a KCB from the delayed close table;

Arguments:

    kcb - the kcb in question

Note: 
    
    kcb lock/resource should be acquired exclusively when this function is called

Return Value:

    NONE.

--*/
{
    PCM_DELAYED_CLOSE_ENTRY     DelayedEntry;

    CM_PAGED_CODE();


    ASSERT( (CmpIsKCBLockedExclusive(kcb) == TRUE) || // this kcb is owned exclusive
            (CmpTestRegistryLockExclusive() == TRUE) ); // or the entire registry is locked exclusive

    // not on delay close; don't try to remove it.
    if (kcb->DelayedCloseIndex == CmpDelayedCloseSize) { // see if we really need this
        ASSERT( FALSE );
        return;
    }

    DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)kcb->DelayCloseEntry;
    if( DelayedEntry != NULL ) {
        LOCK_DELAY_CLOSE();
        CmpRemoveEntryList(&(DelayedEntry->DelayedLRUList));
        UNLOCK_DELAY_CLOSE();

        //
        // give back the entry
        //
        CmpDelayCloseFreeEntry(DelayedEntry);
        InterlockedDecrement((PLONG)&CmpDelayedCloseElements);
#if DBG
        if( kcb->InDelayClose == 0 ) {
            ASSERT( FALSE );
        }
        {
            LONG                        OldRefCount;
            LONG                        NewRefCount;

            OldRefCount = *(PLONG)&kcb->InDelayClose; //get entire dword
            ASSERT( OldRefCount == 1 );
            NewRefCount = 0;
            if( InterlockedCompareExchange((PLONG)&kcb->InDelayClose,NewRefCount,OldRefCount)
                    != OldRefCount ) {
    
                ASSERT( FALSE );
            }
        }
#endif //DBG
    }

    //
    // easy enough huh ?
    //
    kcb->DelayedCloseIndex = CmpDelayedCloseSize; // see if we really need this
    kcb->DelayCloseEntry = NULL;
}


VOID
CmpAddToDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN BOOLEAN                  RegLockHeldEx
    )
/*++

Routine Description:

    Adds a kcb to the delayed close table

Arguments:

    kcb - the kcb in question

Note: 
    
    kcb lock/resource should be acquired exclusively when this function is called

Return Value:

    NONE.

--*/
{
    PCM_DELAYED_CLOSE_ENTRY     DelayedEntry = NULL;

    CM_PAGED_CODE();

    ASSERT( (CmpIsKCBLockedExclusive(kcb) == TRUE) || // this kcb is owned exclusive
            (CmpTestRegistryLockExclusive() == TRUE) ); // or the entire registry is locked exclusive


    // Already on delayed close, don't try to put on again
    if (kcb->DelayedCloseIndex != CmpDelayedCloseSize) { // see if we really need this
        ASSERT( FALSE );
        return;
    }

    ASSERT(kcb->RefCount == 0);

    //
    // now materialize a new entry for this kcb
    //
    ASSERT_KEYBODY_LIST_EMPTY(kcb);
    DelayedEntry = CmpDelayCloseAllocNewEntry();
    if( !DelayedEntry ) {
        //
        // this is bad luck; we need to free the kcb in place
        //
        CmpCleanUpKcbCacheWithLock(kcb,RegLockHeldEx);
        return;
    }
#if DBG
    if( kcb->InDelayClose != 0 ) {
        ASSERT( FALSE );
    }
    {
        LONG                        OldRefCount;
        LONG                        NewRefCount;

        OldRefCount = *(PLONG)&kcb->InDelayClose; //get entire dword
        ASSERT( OldRefCount == 0 );
        NewRefCount = 1;
        if( InterlockedCompareExchange((PLONG)&kcb->InDelayClose,NewRefCount,OldRefCount)
                != OldRefCount ) {
    
            ASSERT( FALSE );
        }
    }
#endif //DBG
    //
    // populate the entry and insert it into the LRU list (at the top).
    //
    kcb->DelayedCloseIndex = 0; // see if we really need this
    kcb->DelayCloseEntry = (PVOID)DelayedEntry;    // need this for removing it back from here
    DelayedEntry->KeyControlBlock = kcb;
    InterlockedIncrement((PLONG)&CmpDelayedCloseElements);

    LOCK_DELAY_CLOSE();
    InsertHeadList(
        &CmpDelayedLRUListHead,
        &(DelayedEntry->DelayedLRUList)
        );
    //
    // check if limit hit and arm timer if not already armed
    //
    if( (CmpDelayedCloseElements > CmpDelayedCloseSize) && (!CmpDelayCloseWorkItemActive) ) {
        CmpArmDelayedCloseTimer();
    } 
    UNLOCK_DELAY_CLOSE();
    //
    // we're done here
    //

}

//-----------------------------------------------------------------------------------------------------------//
//                                                                                                           //
// Delayed KCB deref. Used when we hit a symlink or when we simply cannot safely acquire the kcb lock        //
//                                                                                                           //
//-----------------------------------------------------------------------------------------------------------//
LIST_ENTRY      CmpDelayDerefKCBListHead;
KGUARDED_MUTEX  CmpDelayDerefKCBLock;                
WORK_QUEUE_ITEM CmpDelayDerefKCBWorkItem;
BOOLEAN         CmpDelayDerefKCBWorkItemActive = FALSE;

KTIMER          CmpDelayDerefKCBTimer;
KDPC            CmpDelayDerefKCBDpc;

ULONG           CmpDelayDerefKCBIntervalInSeconds = 5;


#define LOCK_KCB_DELAY_DEREF_LIST() KeAcquireGuardedMutex(&CmpDelayDerefKCBLock)
#define UNLOCK_KCB_DELAY_DEREF_LIST() KeReleaseGuardedMutex(&CmpDelayDerefKCBLock)

VOID
CmpDelayDerefKCBDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This is the DPC routine triggered by the delayclose timer.  
    is queue a work item to an executive worker thread.  

Arguments:

    Dpc - Supplies a pointer to the DPC object.

    DeferredContext - not used

    SystemArgument1 - not used

    SystemArgument2 - not used

Return Value:

    None.

--*/

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    ASSERT(CmpDelayDerefKCBWorkItemActive);
    ExQueueWorkItem(&CmpDelayDerefKCBWorkItem, DelayedWorkQueue);
}

#define CmpDelayDerefKCBAllocNewEntry() (PCM_DELAY_DEREF_KCB_ITEM)CmpAllocateDelayItem()
#define CmpDelayDerefKCBFreeEntry(Item) CmpFreeDelayItem((PVOID)(Item))

//#define CmpDelayDerefKCBAllocNewEntry() (PCM_DELAY_DEREF_KCB_ITEM)ExAllocatePool(PagedPool,sizeof(CM_DELAY_DEREF_KCB_ITEM))
//#define CmpDelayDerefKCBFreeEntry(Item) ExFreePool(Item)


VOID
CmpInitDelayDerefKCBEngine()
{
    InitializeListHead(&CmpDelayDerefKCBListHead);
    KeInitializeGuardedMutex(&CmpDelayDerefKCBLock);
    ExInitializeWorkItem(&CmpDelayDerefKCBWorkItem, CmpDelayDerefKCBWorker, NULL);

    KeInitializeDpc(&CmpDelayDerefKCBDpc,
                    CmpDelayDerefKCBDpcRoutine,
                    NULL);

    KeInitializeTimer(&CmpDelayDerefKCBTimer);
}

VOID    
CmpRunDownDelayDerefKCBEngine(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                BOOLEAN               RegLockHeldEx)
{
    PCM_DELAY_DEREF_KCB_ITEM    DelayItem;
    ULONG                       NewIndex;
    ULONG                       Index1 = CmpHashTableSize;
    ULONG                       Index2 = CmpHashTableSize;
    PCMHIVE                     CmHive = NULL;
    BOOLEAN                     DoUnloadCheck = FALSE;

    CM_PAGED_CODE();

    ASSERT_CM_LOCK_OWNED();

    ASSERT( (KeyControlBlock && CmpIsKCBLockedExclusive(KeyControlBlock)) || (CmpTestRegistryLockExclusive() == TRUE) );

    if( KeyControlBlock ) {
        Index1 = GET_HASH_INDEX(KeyControlBlock->ConvKey);
        if( KeyControlBlock->ParentKcb ) {
            Index2 = GET_HASH_INDEX(KeyControlBlock->ParentKcb->ConvKey);
        }
    }
    LOCK_KCB_DELAY_DEREF_LIST();
    while( !CmpIsListEmpty(&CmpDelayDerefKCBListHead) ) {
        //
        // pull it out of the list
        //
        DelayItem = (PCM_DELAY_DEREF_KCB_ITEM)RemoveHeadList(&CmpDelayDerefKCBListHead);
        UNLOCK_KCB_DELAY_DEREF_LIST();

        DelayItem = CONTAINING_RECORD(  DelayItem,
                                        CM_DELAY_DEREF_KCB_ITEM,
                                        ListEntry);
        CmpClearListEntry(&(DelayItem->ListEntry));

        //
        // take additional precaution in the case the hive has been unloaded and this is the root
        //
        DoUnloadCheck = FALSE;
        if( !DelayItem->Kcb->Delete ) {
            CmHive = (PCMHIVE)CONTAINING_RECORD(DelayItem->Kcb->KeyHive, CMHIVE, Hive);
            if( IsHiveFrozen(CmHive) ) {
                //
                // unload is pending for this hive;
                //
                DoUnloadCheck = TRUE;

            }
        }

        NewIndex = GET_HASH_INDEX(DelayItem->Kcb->ConvKey);
        //
        // now deref and free 
        //
        if( (NewIndex == Index1) || (NewIndex == Index2) ) {
            //
            // we already hold the lock
            //
            ASSERT_KCB_LOCKED_EXCLUSIVE(DelayItem->Kcb);
            CmpDereferenceKeyControlBlockWithLock(DelayItem->Kcb,RegLockHeldEx);
        } else {
            //
            // we can't afford to force acquire; try to acquire and if it fails bail out
            //
            if( CmpKCBLockForceAcquireAllowed(Index1,Index2,NewIndex) ) {
                CmpLockHashEntryByIndexExclusive(NewIndex);
            } else if( CmpTryToLockHashEntryByIndexExclusive(NewIndex) == FALSE ) {
                //
                // add the item back to the list and bail out
                //
                ASSERT( CmpTestRegistryLockExclusive() == FALSE );
                LOCK_KCB_DELAY_DEREF_LIST();
                InsertHeadList(&CmpDelayDerefKCBListHead,&(DelayItem->ListEntry));
                UNLOCK_KCB_DELAY_DEREF_LIST();
                return;
            }
            ASSERT_KCB_LOCKED_EXCLUSIVE(DelayItem->Kcb);
            CmpDereferenceKeyControlBlockWithLock(DelayItem->Kcb,RegLockHeldEx);
            CmpUnlockHashEntryByIndex(NewIndex);
        }
        CmpDelayDerefKCBFreeEntry(DelayItem);

        //
        // if we dropped a reference inside a frozen hive, we may need to unload the hive
        //
        if( DoUnloadCheck == TRUE ) {
            CmpDoQueueLateUnloadWorker(CmHive);
        }

        LOCK_KCB_DELAY_DEREF_LIST();
    }
    UNLOCK_KCB_DELAY_DEREF_LIST();
}

VOID
CmpDelayDerefKCBWorker(
    IN PVOID Parameter
    )

/*++

Routine Description:


Arguments:

    Parameter - not used.

Return Value:

    None.

--*/

{
    PCM_DELAY_DEREF_KCB_ITEM DelayItem;
    PCMHIVE                 CmHive = NULL;
    BOOLEAN                 DoUnloadCheck = FALSE;

    CM_PAGED_CODE();

    UNREFERENCED_PARAMETER (Parameter);

    ASSERT(CmpDelayDerefKCBWorkItemActive);

    BEGIN_LOCK_CHECKPOINT;
    CmpLockRegistry();

    LOCK_KCB_DELAY_DEREF_LIST();
    while( !CmpIsListEmpty(&CmpDelayDerefKCBListHead) ) {
        //
        // pull it out of the list
        //
        DelayItem = (PCM_DELAY_DEREF_KCB_ITEM)RemoveHeadList(&CmpDelayDerefKCBListHead);
        UNLOCK_KCB_DELAY_DEREF_LIST();

        DelayItem = CONTAINING_RECORD(  DelayItem,
                                        CM_DELAY_DEREF_KCB_ITEM,
                                        ListEntry);
        CmpClearListEntry(&(DelayItem->ListEntry));
        //
        // take additional precaution in the case the hive has been unloaded and this is the root
        //
        DoUnloadCheck = FALSE;
        if( !DelayItem->Kcb->Delete ) {
            CmHive = (PCMHIVE)CONTAINING_RECORD(DelayItem->Kcb->KeyHive, CMHIVE, Hive);
            if( IsHiveFrozen(CmHive) ) {
                //
                // unload is pending for this hive;
                //
                DoUnloadCheck = TRUE;

            }
        }

        //
        // now deref and free 
        //
        CmpDereferenceKeyControlBlock(DelayItem->Kcb);
        CmpDelayDerefKCBFreeEntry(DelayItem);

        //
        // if we dropped a reference inside a frozen hive, we may need to unload the hive
        //
        if( DoUnloadCheck == TRUE ) {
            CmpDoQueueLateUnloadWorker(CmHive);
        }

        LOCK_KCB_DELAY_DEREF_LIST();
    }
    //
    // signal that we have finished our work.
    //
    CmpDelayDerefKCBWorkItemActive = FALSE;
    UNLOCK_KCB_DELAY_DEREF_LIST();

    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;
}

VOID
CmpDelayDerefKeyControlBlock( PCM_KEY_CONTROL_BLOCK   KeyControlBlock )
/*++

Routine Description:

    Adds kcb to a list to be deref in a workitem

Arguments:

    kcb - the kcb in question

Note: 
    
    kcb lock/resource should be acquired exclusively when this function is called

Return Value:

    NONE.

--*/
{
    PCM_DELAY_DEREF_KCB_ITEM    DelayItem;
    LONG                        OldRefCount;
    LONG                        NewRefCount;

    CM_PAGED_CODE();

    //
    // try the fast path first; we only need to take the work item approach when we drop to 0
    //
    OldRefCount = *(PLONG)&KeyControlBlock->RefCount; //get entire dword
    NewRefCount = OldRefCount - 1;
    if( (NewRefCount & 0xffff) > 0  &&
        InterlockedCompareExchange((PLONG)&KeyControlBlock->RefCount,NewRefCount,OldRefCount)
            == OldRefCount ) {
    
        LogKCBReference(KeyControlBlock,2);
        return;
    }

    DelayItem = CmpDelayDerefKCBAllocNewEntry();
    if( DelayItem == NULL ) {
        //
        // nothing to do here ; we'll leak a reference on this kcb
        //
        return;
    }

    DelayItem->Kcb = KeyControlBlock;
    LOCK_KCB_DELAY_DEREF_LIST();
    InsertTailList( &CmpDelayDerefKCBListHead,&(DelayItem->ListEntry));
    //
    // if worker is not already up; queue it
    //
    if( !CmpDelayDerefKCBWorkItemActive ) {
        LARGE_INTEGER DueTime;

        CmpDelayDerefKCBWorkItemActive = TRUE;
            
        //
        // queue a timer for 5 secs later to do the actual delay close
        //

        DueTime.QuadPart = Int32x32To64(CmpDelayDerefKCBIntervalInSeconds,
                                        - SECOND_MULT);
        //
        // Indicate relative time
        //

        KeSetTimer(&CmpDelayDerefKCBTimer,
                   DueTime,
                   &CmpDelayDerefKCBDpc);
    } 
    UNLOCK_KCB_DELAY_DEREF_LIST();
}

