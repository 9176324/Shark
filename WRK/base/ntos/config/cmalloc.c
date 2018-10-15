/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmalloc.c

Abstract:

    Provides routines for implementing the registry's own pool allocator.

--*/

#include "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpInitCmPrivateAlloc)
#pragma alloc_text(PAGE,CmpDestroyCmPrivateAlloc)
#pragma alloc_text(PAGE,CmpAllocateKeyControlBlock)
#pragma alloc_text(PAGE,CmpFreeKeyControlBlock)
#pragma alloc_text(INIT,CmpInitCmPrivateDelayAlloc)
#pragma alloc_text(PAGE,CmpDestroyCmPrivateDelayAlloc)
#pragma alloc_text(PAGE,CmpAllocateDelayItem)
#pragma alloc_text(PAGE,CmpFreeDelayItem)
#endif

typedef struct _CM_ALLOC_PAGE {
    ULONG       FreeCount;		// number of free kcbs
    ULONG       Reserved;		// alignment
    PVOID       AllocPage;      // crud allocations - this member is NOT USED
} CM_ALLOC_PAGE, *PCM_ALLOC_PAGE;

#define CM_KCB_ENTRY_SIZE   sizeof( CM_KEY_CONTROL_BLOCK )
#define CM_KCBS_PER_PAGE    ((PAGE_SIZE - FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage)) / CM_KCB_ENTRY_SIZE)

#define KCB_TO_PAGE_ADDRESS( kcb ) (PVOID)(((ULONG_PTR)(kcb)) & ~(PAGE_SIZE - 1))
#define KCB_TO_ALLOC_PAGE( kcb ) ((PCM_ALLOC_PAGE)KCB_TO_PAGE_ADDRESS(kcb))

LIST_ENTRY          CmpFreeKCBListHead;   // list of free kcbs
BOOLEAN				CmpAllocInited = FALSE;

KGUARDED_MUTEX			CmpAllocBucketLock;                // used to protect the bucket

#define LOCK_ALLOC_BUCKET() KeAcquireGuardedMutex(&CmpAllocBucketLock)
#define UNLOCK_ALLOC_BUCKET() KeReleaseGuardedMutex(&CmpAllocBucketLock)

VOID
CmpInitCmPrivateAlloc( )

/*++

Routine Description:

    Initialize the CmPrivate pool allocation module

Arguments:


Return Value:


--*/

{
    if( CmpAllocInited ) {
        //
        // already initialized
        //
        return;
    }
    
    InitializeListHead(&(CmpFreeKCBListHead));   

    //
	// init the bucket lock
	//
	KeInitializeGuardedMutex(&CmpAllocBucketLock);
	
	CmpAllocInited = TRUE;
}

VOID
CmpDestroyCmPrivateAlloc( )

/*++

Routine Description:

    Frees memory used byt the CmPrivate pool allocation module

Arguments:


Return Value:


--*/

{
    PAGED_CODE();
    
    if( !CmpAllocInited ) {
        return;
    }
}


PCM_KEY_CONTROL_BLOCK
CmpAllocateKeyControlBlock( )

/*++

Routine Description:

    Allocates a kcb; first try from our own allocator.
    If it doesn't work (we have maxed out our number of allocs
    or private allocator is not initialized)
    try from paged pool

Arguments:


Return Value:

    The  new kcb

--*/

{
    USHORT                  j;
    PCM_KEY_CONTROL_BLOCK   kcb = NULL;
	PCM_ALLOC_PAGE			AllocPage;

    PAGED_CODE();
    
    if( !CmpAllocInited ) {
        //
        // not initialized
        //
        goto AllocFromPool;
    }
    
	LOCK_ALLOC_BUCKET();

SearchFreeKcb:
    //
    // try to find a free one
    //
    if( IsListEmpty(&CmpFreeKCBListHead) == FALSE ) {
        //
        // found one
        //
        kcb = (PCM_KEY_CONTROL_BLOCK)RemoveHeadList(&CmpFreeKCBListHead);
        kcb = CONTAINING_RECORD(kcb,
                                CM_KEY_CONTROL_BLOCK,
                                FreeListEntry);

		AllocPage = (PCM_ALLOC_PAGE)KCB_TO_ALLOC_PAGE( kcb );

        ASSERT( AllocPage->FreeCount != 0 );

        AllocPage->FreeCount--;
        
		//
		// set when page was allocated
		//
		ASSERT( kcb->PrivateAlloc == 1);

		UNLOCK_ALLOC_BUCKET();
        return kcb;
    }

    //
    // we need to allocate a new page as we ran out of free kcbs
    //
            
    //
    // allocate a new page and insert all kcbs in the freelist
    //
    AllocPage = (PCM_ALLOC_PAGE)ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, CM_ALLOCATE_TAG|PROTECTED_POOL);
    if( AllocPage == NULL ) {
        //
        // we might be low on pool; maybe small pool chunks will work
        //
		UNLOCK_ALLOC_BUCKET();
        goto AllocFromPool;
    }

	//
	// set up the page
	//
    AllocPage->FreeCount = CM_KCBS_PER_PAGE;

    //
    // now the dirty job; insert all kcbs inside the page in the free list
    //
    for(j=0;j<CM_KCBS_PER_PAGE;j++) {
        kcb = (PCM_KEY_CONTROL_BLOCK)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_KCB_ENTRY_SIZE);

		//
		// set it here; only once
		//
		kcb->PrivateAlloc = 1;
        kcb->DelayCloseEntry = NULL;
        
        InsertTailList(
            &CmpFreeKCBListHead,
            &(kcb->FreeListEntry)
            );
    }
            
    //
    // this time will find one for sure
    //
    goto SearchFreeKcb;

AllocFromPool:
    kcb = ExAllocatePoolWithTag(PagedPool,
                                sizeof(CM_KEY_CONTROL_BLOCK),
                                CM_KCB_TAG | PROTECTED_POOL);

    if( kcb != NULL ) {
        //
        // clear the private alloc flag
        //
        kcb->PrivateAlloc = 0;
        kcb->DelayCloseEntry = NULL;
    }

    return kcb;
}

#define LogKCBFree(kcb) //nothing

VOID
CmpFreeKeyControlBlock( PCM_KEY_CONTROL_BLOCK kcb )

/*++

Routine Description:

    Frees a kcb; if it's allocated from our own pool put it back in the free list.
    If it's allocated from general pool, just free it.

Arguments:

    kcb to free

Return Value:


--*/
{
    USHORT			j;
	PCM_ALLOC_PAGE	AllocPage;

    PAGED_CODE();

    ASSERT_KEYBODY_LIST_EMPTY(kcb);

#if defined(_WIN64)
    //
    // free cached name if any
    //
    if( (kcb->RealKeyName != NULL) && (kcb->RealKeyName != CMP_KCB_REAL_NAME_UPCASE) ) {
        ExFreePoolWithTag(kcb->RealKeyName, CM_NAME_TAG);
    }
#endif

    if( !kcb->PrivateAlloc ) {
        //
        // just free it and be done with it
        //
        ExFreePoolWithTag(kcb, CM_KCB_TAG | PROTECTED_POOL);
        return;
    }

	LOCK_ALLOC_BUCKET();

    ASSERT_HASH_ENTRY_LOCKED_EXCLUSIVE(kcb->ConvKey);
    LogKCBFree(kcb);

    //
    // add kcb to freelist
    //
    InsertTailList(
        &CmpFreeKCBListHead,
        &(kcb->FreeListEntry)
        );

	//
	// get the page
	//
	AllocPage = (PCM_ALLOC_PAGE)KCB_TO_ALLOC_PAGE( kcb );

    //
	// not all are free
	//
	ASSERT( AllocPage->FreeCount != CM_KCBS_PER_PAGE);

	AllocPage->FreeCount++;

    if( AllocPage->FreeCount == CM_KCBS_PER_PAGE ) {
        //
        // entire page is free; let it go
        //
        //
        // first; iterate through the free kcb list and remove all kcbs inside this page
        //
        for(j=0;j<CM_KCBS_PER_PAGE;j++) {
            kcb = (PCM_KEY_CONTROL_BLOCK)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_KCB_ENTRY_SIZE);
        
            RemoveEntryList(&(kcb->FreeListEntry));
        }
        ExFreePoolWithTag(AllocPage, CM_ALLOCATE_TAG|PROTECTED_POOL);
    }

	UNLOCK_ALLOC_BUCKET();

}

//
// delay deref and delay close private allocator
//

LIST_ENTRY          CmpFreeDelayItemsListHead;   // list of free delay items

KGUARDED_MUTEX			CmpDelayAllocBucketLock;                // used to protect the bucket

#define LOCK_DELAY_ALLOC_BUCKET() KeAcquireGuardedMutex(&CmpDelayAllocBucketLock)
#define UNLOCK_DELAY_ALLOC_BUCKET() KeReleaseGuardedMutex(&CmpDelayAllocBucketLock)



#define CM_DELAY_ALLOC_ENTRY_SIZE   sizeof( CM_DELAY_ALLOC )
#define CM_DELAYS_PER_PAGE          ((PAGE_SIZE - FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage)) / CM_DELAY_ALLOC_ENTRY_SIZE)

#define DELAY_ALLOC_TO_PAGE_ADDRESS( delay ) (PVOID)(((ULONG_PTR)(delay)) & ~(PAGE_SIZE - 1))
#define DELAY_ALLOC_TO_ALLOC_PAGE( delay )   ((PCM_ALLOC_PAGE)DELAY_ALLOC_TO_PAGE_ADDRESS(delay))


VOID
CmpInitCmPrivateDelayAlloc( )

/*++

Routine Description:

    Initialize the CmPrivate pool allocation module for delay related allocs

Arguments:


Return Value:


--*/

{
    InitializeListHead(&(CmpFreeDelayItemsListHead));   

    //
	// init the bucket lock
	//
	KeInitializeGuardedMutex(&CmpDelayAllocBucketLock);
}

VOID
CmpDestroyCmPrivateDelayAlloc( )

/*++

Routine Description:

    Frees memory used by the CmPrivate pool allocation module

Arguments:


Return Value:


--*/

{
    CM_PAGED_CODE();
}


PVOID
CmpAllocateDelayItem( )

/*++

Routine Description:

    Allocates a delay item from our own pool; 

Arguments:


Return Value:

    The  new item

--*/

{
    USHORT                  j;
    PCM_DELAY_ALLOC         DelayItem = NULL;
	PCM_ALLOC_PAGE			AllocPage;

    CM_PAGED_CODE();
    
    
	LOCK_DELAY_ALLOC_BUCKET();

SearchFreeItem:
    //
    // try to find a free one
    //
    if( CmpIsListEmpty(&CmpFreeDelayItemsListHead) == FALSE ) {
        //
        // found one
        //
        DelayItem = (PCM_DELAY_ALLOC)RemoveHeadList(&CmpFreeDelayItemsListHead);
        DelayItem = CONTAINING_RECORD(  DelayItem,
                                        CM_DELAY_ALLOC,
                                        ListEntry);

        CmpClearListEntry(&(DelayItem->ListEntry));

        AllocPage = (PCM_ALLOC_PAGE)DELAY_ALLOC_TO_ALLOC_PAGE( DelayItem );

        ASSERT( AllocPage->FreeCount != 0 );

        AllocPage->FreeCount--;
        
		UNLOCK_DELAY_ALLOC_BUCKET();
        return DelayItem;
    }

    //
    // we need to allocate a new page as we ran out of free items
    //
            
    //
    // allocate a new page and insert all kcbs in the freelist
    //
    AllocPage = (PCM_ALLOC_PAGE)ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, CM_ALLOCATE_TAG|PROTECTED_POOL);
    if( AllocPage == NULL ) {
        //
        // bad luck
        //
		UNLOCK_DELAY_ALLOC_BUCKET();
        return NULL;
    }

	//
	// set up the page
	//
    AllocPage->FreeCount = CM_DELAYS_PER_PAGE;

    //
    // now the dirty job; insert all items inside the page in the free list
    //
    for(j=0;j<CM_DELAYS_PER_PAGE;j++) {
        DelayItem = (PCM_DELAY_ALLOC)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_DELAY_ALLOC_ENTRY_SIZE);

        InsertTailList(
            &CmpFreeDelayItemsListHead,
            &(DelayItem->ListEntry)
            );
        DelayItem->Kcb = NULL;
    }
            
    //
    // this time will find one for sure
    //
    goto SearchFreeItem;
}

extern LIST_ENTRY      CmpDelayDerefKCBListHead;
extern LIST_ENTRY      CmpDelayedLRUListHead;

VOID
CmpFreeDelayItem( PVOID Item )

/*++

Routine Description:

    Frees a kcb; if it's allocated from our own pool put it back in the free list.
    If it's allocated from general pool, just free it.

Arguments:

    kcb to free

Return Value:


--*/
{
    USHORT			j;
	PCM_ALLOC_PAGE	AllocPage;
    PCM_DELAY_ALLOC DelayItem = (PCM_DELAY_ALLOC)Item;

    CM_PAGED_CODE();

	LOCK_DELAY_ALLOC_BUCKET();

    //
    // add kcb to freelist
    //
    InsertTailList(
        &CmpFreeDelayItemsListHead,
        &(DelayItem->ListEntry)
        );

	//
	// get the page
	//
    AllocPage = (PCM_ALLOC_PAGE)DELAY_ALLOC_TO_ALLOC_PAGE( DelayItem );

    //
	// not all are free
	//
	ASSERT( AllocPage->FreeCount != CM_DELAYS_PER_PAGE);

	AllocPage->FreeCount++;

    if( AllocPage->FreeCount == CM_DELAYS_PER_PAGE ) {
        //
        // entire page is free; let it go
        //
        //
        // first; iterate through the free item list and remove all items inside this page
        //
        for(j=0;j<CM_DELAYS_PER_PAGE;j++) {
            DelayItem = (PCM_DELAY_ALLOC)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_DELAY_ALLOC_ENTRY_SIZE);
            CmpRemoveEntryList(&(DelayItem->ListEntry));
        }
        ExFreePoolWithTag(AllocPage, CM_ALLOCATE_TAG|PROTECTED_POOL);
    }

	UNLOCK_DELAY_ALLOC_BUCKET();
}

