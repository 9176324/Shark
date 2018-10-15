/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmsubs.c

Abstract:

    This module various support routines for the configuration manager.

    The routines in this module are not independent enough to be linked
    into any other program.  The routines in cmsubs2.c are.

--*/

#include    "cmp.h"

EX_PUSH_LOCK CmpPostLock;

PCM_KEY_HASH_TABLE_ENTRY CmpCacheTable;
PCM_NAME_HASH_TABLE_ENTRY CmpNameCacheTable;
ULONG CmpHashTableSize = 2048;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

extern ULONG CmpDelayedCloseSize; 
extern BOOLEAN CmpHoldLazyFlush;

VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    );

PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    );

//
// private prototype for recursive worker
//


VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    );

VOID
CmpDumpOneKeyBody(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PCM_KEY_BODY             KeyBody,
    IN PUNICODE_STRING          Name,
    IN PVOID                    Context
    );

VOID
CmpDumpKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK   kcb,
    IN PULONG                  Count,
    IN PVOID                   Context 
    );

BOOLEAN
CmpRehashKcbSubtree(
                    PCM_KEY_CONTROL_BLOCK   Start,
                    PCM_KEY_CONTROL_BLOCK   End
                    );

VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCleanUpKCBCacheTable)
#pragma alloc_text(PAGE,CmpSearchForOpenSubKeys)
#pragma alloc_text(PAGE,CmpReferenceKeyControlBlock)
#pragma alloc_text(PAGE,CmpGetNameControlBlock)
#pragma alloc_text(PAGE,CmpDereferenceNameControlBlockWithLock)
#pragma alloc_text(PAGE,CmpCleanUpSubKeyInfo)
#pragma alloc_text(PAGE,CmpCleanUpKcbValueCache)
#pragma alloc_text(PAGE,CmpCleanUpKcbCacheWithLock)
#pragma alloc_text(PAGE,CmpConstructName)
#pragma alloc_text(PAGE,CmpCreateKeyControlBlock)
#pragma alloc_text(PAGE,CmpSearchKeyControlBlockTree)
#pragma alloc_text(PAGE,CmpDereferenceKeyControlBlock)
#pragma alloc_text(PAGE,CmpDereferenceKeyControlBlockWithLock)
#pragma alloc_text(PAGE,CmpRemoveKeyControlBlock)
#pragma alloc_text(PAGE,CmpFreeKeyBody)
#pragma alloc_text(PAGE,CmpInsertKeyHash)
#pragma alloc_text(PAGE,CmpRemoveKeyHash)
#pragma alloc_text(PAGE,CmpInitializeCache)
#pragma alloc_text(PAGE,CmpDumpKeyBodyList)
#pragma alloc_text(PAGE,CmpFlushNotifiesOnKeyBodyList)
#pragma alloc_text(PAGE,CmpRebuildKcbCache)
#pragma alloc_text(PAGE,CmpComputeKcbConvKey)
#pragma alloc_text(PAGE,CmpRehashKcbSubtree)
#pragma alloc_text(PAGE,InitializeKCBKeyBodyList)
#pragma alloc_text(PAGE,EnlistKeyBodyWithKCB)
#pragma alloc_text(PAGE,DelistKeyBodyFromKCB)

#pragma alloc_text(PAGE,CmpDumpOneKeyBody)
#endif

#define CMP_FAST_KEY_BODY_ARRAY_DUMP    1
#define CMP_FAST_KEY_BODY_ARRAY_FLUSH   2

VOID
CmpDumpOneKeyBody(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PCM_KEY_BODY             KeyBody,
    IN PUNICODE_STRING          Name,
    IN PVOID                    Context
    )
{
    //
    // sanity check: this should be a KEY_BODY
    //
    ASSERT_KEY_OBJECT(KeyBody);
    
    if( !Context ) {
        //
        // NtQueryOpenSubKeys : dump it's name and owning process
        //
        {
            PEPROCESS   Process;
            PUCHAR      ImageName = NULL;


            if( NT_SUCCESS(PsLookupProcessByProcessId(KeyBody->ProcessID,&Process))) {
                ImageName = PsGetProcessImageFileName(Process);
            } else {
                Process = NULL;
            }

            if( !ImageName ) {
                ImageName = (PUCHAR)"Unknown";
            }
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Process %p (PID = %lx ImageFileName = %s) (KCB = %p) :: Key %wZ \n",
                                    Process,KeyBody->ProcessID,ImageName,kcb,Name);
            if( Process ) {
                ObDereferenceObject (Process);
            }
        }
    } else {
        //
        // NtQueryOpenSubKeysEx: build up the return buffer; make sure we only touch it
        // inside a try except as it's user-mode buffer
        //
        PQUERY_OPEN_SUBKEYS_CONTEXT     QueryContext = (PQUERY_OPEN_SUBKEYS_CONTEXT)Context;
        PKEY_OPEN_SUBKEYS_INFORMATION   SubkeysInfo = (PKEY_OPEN_SUBKEYS_INFORMATION)(QueryContext->Buffer);
        ULONG                           SizeNeeded;
        
		//
		// we need to ignore the one created by us inside NtQueryOpenSubKeysEx
		//
		if( QueryContext->KeyBodyToIgnore != KeyBody ) {
			//
			// update RequiredSize; we do this regardless if we have room or not in the buffer
			// reserve for one entry in the array and the unicode name buffer
			//
			SizeNeeded = (sizeof(KEY_PID_ARRAY) + (ULONG)(Name->Length));
			QueryContext->RequiredSize += SizeNeeded;
        
			//
			// if we have encountered an error (overflow, or else) at some previous iteration, no point going on
			//
			if( NT_SUCCESS(QueryContext->StatusCode) ) {
				//
				// see if we have enough room for current entry.
				//
				if( (QueryContext->UsedLength + SizeNeeded) > QueryContext->BufferLength ) {
					//
					// buffer not big enough; 
					//
					QueryContext->StatusCode = STATUS_BUFFER_OVERFLOW;
				} else {
					//
					// we have established we have enough room; create/add a new entry to the key array
					// and build up unicode name buffer. copy key name to it.
					// array elements are at the beginning of the user buffer, while name buffers start at 
					// the end and continue backwards, as long as there is enough room.
					//
					try {
						//
						// protect user mode memory
						//
						SubkeysInfo->KeyArray[SubkeysInfo->Count].PID = KeyBody->ProcessID;
						SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Length = Name->Length;
						SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.MaximumLength = Name->Length; 
						SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Buffer = (PWSTR)((PUCHAR)QueryContext->CurrentNameBuffer - Name->Length);
						RtlCopyMemory(  SubkeysInfo->KeyArray[SubkeysInfo->Count].KeyName.Buffer,
										Name->Buffer,
										Name->Length);
						//
						// update array count and work vars inside the querycontext
						//
						SubkeysInfo->Count++;
						QueryContext->CurrentNameBuffer = (PUCHAR)QueryContext->CurrentNameBuffer - Name->Length;
						QueryContext->UsedLength += SizeNeeded;
					} except (EXCEPTION_EXECUTE_HANDLER) {
						QueryContext->StatusCode = GetExceptionCode();
					}
				}
			}
		}

    }
}


VOID
CmpDumpKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN PULONG                   Count,
    IN PVOID                    Context
    )
{
        
    PCM_KEY_BODY    KeyBody;
    PUNICODE_STRING Name;
    ULONG           i;
    BOOLEAN         KeyBodyArrayEmpty = TRUE;

    ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);

    if( kcb->RefCount == 0 ) {
        //
        // this kcb is in the delay close, ignore it.
        //
        ASSERT( kcb->DelayedCloseIndex != CmpDelayedCloseSize );
        return;
    }

    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        if( kcb->KeyBodyArray[i] != NULL ) {
            KeyBodyArrayEmpty = FALSE;
            break;
        }
    }

    if( (IsListEmpty(&(kcb->KeyBodyListHead)) == TRUE) && (KeyBodyArrayEmpty == TRUE) ) {
        //
        // Nobody has this subkey open, but for sure some subkey must be 
        // open. nicely return.
        //
        return;
    }


    Name = CmpConstructName(kcb);
    if( !Name ){
        // we're low on resources
        if( Context != NULL ) {
            ((PQUERY_OPEN_SUBKEYS_CONTEXT)Context)->StatusCode = STATUS_INSUFFICIENT_RESOURCES;
        }
        return;
    }
    
    //
    // now iterate through the list of KEY_BODYs referencing this kcb
    //
    KeyBody = (PCM_KEY_BODY)kcb->KeyBodyListHead.Flink;
    while( KeyBody != (PCM_KEY_BODY)(&(kcb->KeyBodyListHead)) ) {
        KeyBody = CONTAINING_RECORD(KeyBody,
                                    CM_KEY_BODY,
                                    KeyBodyList);
        // dump it.
        CmpDumpOneKeyBody(kcb,KeyBody,Name,Context);

        // count it
        (*Count)++;
        
        KeyBody = (PCM_KEY_BODY)KeyBody->KeyBodyList.Flink;
    }

    //
    // now dump the ones that are on the fast array
    //
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        KeyBody = kcb->KeyBodyArray[i];
        if( (KeyBody != NULL) && 
            ((ULONG_PTR)KeyBody != CMP_FAST_KEY_BODY_ARRAY_DUMP)  &&
            ((ULONG_PTR)KeyBody != CMP_FAST_KEY_BODY_ARRAY_FLUSH) ) {
            //
            // avoid races
            //
            if( InterlockedCompareExchangePointer(&(kcb->KeyBodyArray[i]),
                                                (PVOID)CMP_FAST_KEY_BODY_ARRAY_DUMP,
                                                KeyBody) == KeyBody ) {
                // dump it.
                CmpDumpOneKeyBody(kcb,KeyBody,Name,Context);

                // count it
                (*Count)++;
                //
                // set it back
                //
                InterlockedCompareExchangePointer(&(kcb->KeyBodyArray[i]),
                                                KeyBody,
                                                (PVOID)CMP_FAST_KEY_BODY_ARRAY_DUMP);
            }
        }
    }

    ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);

}

VOID
CmpFlushNotifiesOnKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK    kcb,
    IN BOOLEAN                  LockHeld
    )
/*++
Routine Description:
    
      Flushes notifications on all key_bodies linked to this kcb
      and sets the key object flag.

Arguments:


Return Value:

--*/
{
    PCM_KEY_BODY    KeyBody;
    ULONG           i;
    
#if DBG
    if( LockHeld ) {
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    } else {
        ASSERT_KCB_LOCKED_EXCLUSIVE(kcb);
    }
#endif

Again:
    if( IsListEmpty(&(kcb->KeyBodyListHead)) == FALSE ) {
        //
        // now iterate through the list of KEY_BODYs referencing this kcb
        //
        KeyBody = (PCM_KEY_BODY)kcb->KeyBodyListHead.Flink;
        while( KeyBody != (PCM_KEY_BODY)(&(kcb->KeyBodyListHead)) ) {
            KeyBody = CONTAINING_RECORD(KeyBody,
                                        CM_KEY_BODY,
                                        KeyBodyList);
            //
            // sanity check: this should be a KEY_BODY
            //
            ASSERT_KEY_OBJECT(KeyBody);

            //
            // flush any notifies that might be set on it
            //
            if( KeyBody->NotifyBlock ) {
                //
                // if we hold exclusive registry lock, we are safe; defer delete can't race with us
                // otherwise add an extra reference on the key body so it won't go away.
                //
                if( LockHeld ) {
                    CmpFlushNotify(KeyBody,LockHeld);
                    ASSERT( KeyBody->NotifyBlock == NULL );
                    goto Again;
                } else if(ObReferenceObjectSafe(KeyBody)) {
                    CmpFlushNotify(KeyBody,LockHeld);
                    ASSERT( KeyBody->NotifyBlock == NULL );
				    ObDereferenceObjectDeferDelete(KeyBody);
                    goto Again;
                }
            }
            KeyBody = (PCM_KEY_BODY)KeyBody->KeyBodyList.Flink;
        }
    }
    //
    // same thing for fast array
    //
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        KeyBody = kcb->KeyBodyArray[i];
        if( (KeyBody != NULL) && 
            ((ULONG_PTR)KeyBody != CMP_FAST_KEY_BODY_ARRAY_DUMP)  &&
            ((ULONG_PTR)KeyBody != CMP_FAST_KEY_BODY_ARRAY_FLUSH) ) {
            //
            // avoid races
            //
            if( InterlockedCompareExchangePointer(&(kcb->KeyBodyArray[i]),
                                                (PVOID)CMP_FAST_KEY_BODY_ARRAY_FLUSH,
                                                KeyBody) == KeyBody ) {
                //
                // sanity check: this should be a KEY_BODY
                //
                ASSERT_KEY_OBJECT(KeyBody);

                //
                // flush any notifies that might be set on it
                //
                if( KeyBody->NotifyBlock ) {
                    //
                    // if we hold exclusive registry lock, we are safe; defer delete can't race with us
                    // otherwise add an extra reference on the key body so it won't go away.
                    //
                    if( LockHeld ) {
                        CmpFlushNotify(KeyBody,LockHeld);
                        ASSERT( KeyBody->NotifyBlock == NULL );
                    } else if(ObReferenceObjectSafe(KeyBody)) {
                        CmpFlushNotify(KeyBody,LockHeld);
                        ASSERT( KeyBody->NotifyBlock == NULL );
				        ObDereferenceObjectDeferDelete(KeyBody);
                    }
                }
                //
                // now set it back only if nobody else messed with it in between
                //
                InterlockedCompareExchangePointer(&(kcb->KeyBodyArray[i]),
                                                    KeyBody,
                                                    (PVOID)CMP_FAST_KEY_BODY_ARRAY_FLUSH);    
            }
        }
    }

}

VOID CmpCleanUpKCBCacheTable(PCM_KEY_CONTROL_BLOCK      KeyControlBlock,
                             BOOLEAN                    RegLockHeldEx)
/*++
Routine Description:

	Kicks out of cache all kcbs with RefCount == 0

Arguments:
    
    KeyControlBlock - when present, it's already locked EX.

Return Value:

--*/
{
    ULONG					i;
    PCM_KEY_HASH			*Current;
    PCM_KEY_CONTROL_BLOCK	kcb;
    ULONG                   ThisKcbHashIndex = CmpHashTableSize;
    ULONG                   ParentKcbHashIndex = CmpHashTableSize;
    BOOLEAN                 CacheChanged;

	CM_PAGED_CODE();

    ASSERT( (KeyControlBlock && CmpIsKCBLockedExclusive(KeyControlBlock) &&
            ((KeyControlBlock->ParentKcb == NULL) || (CmpIsKCBLockedExclusive(KeyControlBlock->ParentKcb))) 
            )
            || (CmpTestRegistryLockExclusive() == TRUE) );

    //
    // make sure all delayed dereferences are serviced first
    //
    CmpRunDownDelayDerefKCBEngine(KeyControlBlock,RegLockHeldEx);

    if(KeyControlBlock!= NULL) {
        ThisKcbHashIndex = GET_HASH_INDEX(KeyControlBlock->ConvKey);
        if( KeyControlBlock->ParentKcb != NULL ) {
            ParentKcbHashIndex = GET_HASH_INDEX(KeyControlBlock->ParentKcb->ConvKey);
        }
    }
TryAgain:
    CacheChanged = FALSE;
    for (i=0; i<CmpHashTableSize; i++) {
        if( (KeyControlBlock == NULL) || ((ThisKcbHashIndex != i) && (ParentKcbHashIndex != i)) ) {
            //
            // deadlock avoidance
            //
            if( CmpKCBLockForceAcquireAllowed(ParentKcbHashIndex,ThisKcbHashIndex,i) ) {
                CmpLockHashEntryByIndexExclusive(i);
            } else if( CmpTryToLockHashEntryByIndexExclusive(i) == FALSE ) {
                //
                // couldn't get EX access to this entry; we need to skip it
                //
                continue;
            }
        }
        Current = &(CmpCacheTable[i].Entry);
        while (*Current) {
            kcb = CONTAINING_RECORD(*Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            if( (kcb->RefCount == 0) &&
                ((KeyControlBlock == NULL) || (KeyControlBlock->KeyHive == kcb->KeyHive)) ) { // only interested in kcbs below this hive's root

                //
                // This kcb is in DelayClose case, remove it.
                //
                CmpRemoveFromDelayedClose(kcb);
                CmpCleanUpKcbCacheWithLock(kcb,RegLockHeldEx);
                CacheChanged = TRUE;
                //
                // The HashTable is changed, start over in this index again.
                //
                Current = &(CmpCacheTable[i].Entry);
                continue;
            }
            Current = &(kcb->NextHash);
        }
        if( (KeyControlBlock == NULL) || ((ThisKcbHashIndex != i) && (ParentKcbHashIndex != i)) ) {
            CmpUnlockHashEntryByIndex(i);
        }
        if( CacheChanged ) {
            goto TryAgain;
        }
    }

}

PERFINFO_REG_DUMP_CACHE()

ULONG
CmpSearchForOpenSubKeys(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    SUBKEY_SEARCH_TYPE      SearchType,
    BOOLEAN                 RegLockHeldEx,
    PVOID                   SearchContext
    )
/*++
Routine Description:

    This routine searches the KCB tree for any open handles to keys that
    are subkeys of the given key.

    It is used by CmRestoreKey to verify that the tree being restored to
    has no open handles.

Arguments:

    KeyControlBlock - Supplies the key control block for the key for which
        open subkeys are to be found.

    SearchType - the type of the search
        SearchIfExist - exits at the first open subkey found ==> returns 1 if any subkey is open
        
        SearchAndDeref - Forces the keys underneath the Key referenced KeyControlBlock to 
                be marked as not referenced (see the REG_FORCE_RESTORE flag in CmRestoreKey);
                return value indicates number of keys that could NOT be dereferenced, and thus are
                still open.
        
        SearchAndCount - Counts all open subkeys - returns the number of them

        SearchAndTagNoDelayClose - Flag the subkey so as not to place it in the delayed closed
                table.

        SearchAndRehash (NT_RENAME_KEY) - Rehash subkeys in KCB hash table, if needed.

Return Value:

    Returns the number of open sub keys.
    
--*/
{
    ULONG                   i;
    PCM_KEY_HASH            Current;
    PCM_KEY_CONTROL_BLOCK   kcb;
    PCM_KEY_CONTROL_BLOCK   Parent;
    ULONG                   LevelDiff, l;
    ULONG                   Count = 0;
    ULONG                   ThisKcbHashIndex = CmpHashTableSize;
    ULONG                   ParentKcbHashIndex = CmpHashTableSize;
    
    //
    // Registry lock should be held exclusively, so no need to KCB lock
    //
    ASSERT( ( CmpIsKCBLockedExclusive(KeyControlBlock) && 
             ((KeyControlBlock->ParentKcb == NULL) || CmpIsKCBLockedExclusive(KeyControlBlock->ParentKcb))
             ) ||
             (CmpTestRegistryLockExclusive() == TRUE) );

    //
    // First, clean up all subkeys in the cache
    //
    CmpCleanUpKCBCacheTable(((SearchType == SearchIfExist) || (SearchType == SearchAndCount))?KeyControlBlock:NULL,RegLockHeldEx);

    if( (KeyControlBlock->RefCount == 1) && (SearchType != SearchAndRehash) ) {
        //
        // There is only one open handle, so there must be no open subkeys.
        //
        Count = 0;
    } else {
        //
        // Now search for an open subkey handle.
        //
        Count = 0;

        //
        // dump the root first if we were asked to do so.
        //
        if(SearchType == SearchAndCount) {
            CmpDumpKeyBodyList(KeyControlBlock,&Count,SearchContext);
        }
        ThisKcbHashIndex = GET_HASH_INDEX(KeyControlBlock->ConvKey);
        if( KeyControlBlock->ParentKcb != NULL ) {
            ParentKcbHashIndex = GET_HASH_INDEX(KeyControlBlock->ParentKcb->ConvKey);
        }

        for (i=0; i<CmpHashTableSize; i++) {

            if( (ThisKcbHashIndex != i) && (ParentKcbHashIndex != i) ) {
                //
                // we can wait for the bucket lock in case we are going upwards of what we already own
                //
                if( CmpKCBLockForceAcquireAllowed(ParentKcbHashIndex,ThisKcbHashIndex,i) ) {
                    CmpLockHashEntryByIndexExclusive(i);
                } else {
                    //
                    // deadlock avoidance; bail out with error if we can't get EX access here
                    //
                    if( CmpTryToLockHashEntryByIndexExclusive(i) == FALSE ) {
                        ASSERT(CmpTestRegistryLockExclusive() == FALSE);
                        ASSERT( (SearchType == SearchIfExist) || (SearchType == SearchAndCount) );
                        if( SearchType == SearchIfExist ) {
                            //
                            // assume one already exist
                            //
                            return 1;
                        } 
                        //
                        // else skip this entry;
                        //
                        continue;
                    }
                }
            }

StartDeref:

            Current = CmpCacheTable[i].Entry;
            while (Current) {
                kcb = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
                if (kcb->TotalLevels > KeyControlBlock->TotalLevels) {
                    LevelDiff = kcb->TotalLevels - KeyControlBlock->TotalLevels;
                
                    Parent = kcb;
                    for (l=0; l<LevelDiff; l++) {
                        Parent = Parent->ParentKcb;
                    }
    
                    if (Parent == KeyControlBlock) {
                        //
                        // Found a match;
                        //
                        if( SearchType == SearchIfExist ) {
                            Count = 1;
                            if( (ThisKcbHashIndex != i) && (ParentKcbHashIndex != i) ) {
                                CmpUnlockHashEntryByIndex(i);
                            }
                            return Count;
                        } else if(SearchType == SearchAndTagNoDelayClose) {
				            kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
                        } else if(SearchType == SearchAndDeref) {
                            //
                            // Mark the key as deleted, remove it from cache, but don't add it
                            // to the Delay Close table (we want the key to be visible only to
                            // the one(s) that have open handles on it.
                            //

                            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

                            //
                            // don't mess with read only kcbs; this prevents a potential hack when 
                            // trying to FORCE_RESTORE over WPA keys.
                            //
                            if( !CmIsKcbReadOnly(kcb) ) {
                                //
                                // flush any pending notifies as the kcb won't be around any longer
                                //
                                CmpFlushNotifiesOnKeyBodyList(kcb,TRUE);
                            
                                CmpCleanUpSubKeyInfo(kcb->ParentKcb);
                                kcb->Delete = TRUE;
                                // Cache the pointer to the next hash block before unlinking the
                                // current one.
                                Current = kcb->NextHash;
                                CmpRemoveKeyControlBlock(kcb);
                                kcb->KeyCell = HCELL_NIL;
                                //
                                // Continue the search.
                                // 
                                continue;
                            } else {
                                // We cannot close the key. Record it as still open.
                                ++Count;
                            }
                        } else if(SearchType == SearchAndCount) {
                            //
                            // here do the dumping and count incrementing stuff
                            //
                            CmpDumpKeyBodyList(kcb,&Count,SearchContext);

                        } else if( SearchType == SearchAndRehash ) {
                            //
                            // every kcb which has the one passed as a parameter
                            // as an ancestor needs to be moved to the right location 
                            // in the kcb hash table.
                            //
                            ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

                            if( CmpRehashKcbSubtree(KeyControlBlock,kcb) == TRUE ) {
                                //
                                // at least one kcb has been moved, we need to reiterate this bucket
                                //
                                Count++;
                                goto StartDeref;
                            }
                        }
                    }   

                }
                Current = kcb->NextHash;
            }
            if( (ThisKcbHashIndex != i) && (ParentKcbHashIndex != i) ) {
                CmpUnlockHashEntryByIndex(i);
            }
        }
    }
    
                           
    return Count;
}

ULONG
CmpComputeKcbConvKey(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++
Routine Description:

    Computes the convkey for this kcb based on the NCB and its parent ConvKey

Arguments:

    KeyControlBlock - Supplies the key control block for the key for which
        the ConvKey is to be calculated

Return Value:

    The new ConvKey

Notes:

    This is to be used by the rename key api, which needs to rehash kcbs

--*/
{
    ULONG   ConvKey = 0;
    ULONG   Cnt;
    WCHAR   Cp;
    PUCHAR  u;
    PWCHAR  w;

    CM_PAGED_CODE();

    if( KeyControlBlock->ParentKcb != NULL ) {
        ConvKey = KeyControlBlock->ParentKcb->ConvKey;
    }

    //
    // Manually compute the hash to use.
    //
    ASSERT(KeyControlBlock->NameBlock->NameLength > 0);

    u = (PUCHAR)(&(KeyControlBlock->NameBlock->Name[0]));
    w = (PWCHAR)u;
    for( Cnt = 0; Cnt < KeyControlBlock->NameBlock->NameLength;) {
        if( KeyControlBlock->NameBlock->Compressed ) {
            Cp = (WCHAR)(*u);
            u++;
            Cnt += sizeof(UCHAR);
        } else {
            Cp = *w;
            w++;
            Cnt += sizeof(WCHAR);
        }
        ASSERT( Cp != OBJ_NAME_PATH_SEPARATOR );
        
        ConvKey = 37 * ConvKey + (ULONG)CmUpcaseUnicodeChar(Cp);
    }

    return ConvKey;
}

BOOLEAN
CmpRehashKcbSubtree(
                    PCM_KEY_CONTROL_BLOCK   Start,
                    PCM_KEY_CONTROL_BLOCK   End
                    )
/*++
Routine Description:

    Walks the path between End and Start and rehashed all kcbs that need
    rehashing;

    Assumptions: It is a priori taken that Start is an ancestor of End;

    Works in two steps:
    1. walks the path backwards from End to Start, reverting the back-link
    (we use the ParentKcb member in the kcb structure for that). I.e. we build a 
    forward path from Start to End
    2.Walks the forward path built at 1, rehashes kcbs whos need rehashing and restores
    the parent relationship.
    
Arguments:

    KeyControlBlock - where we start

    kcb - where we stop

Return Value:

    TRUE if at least one kcb has been rehashed

--*/
{
    PCM_KEY_CONTROL_BLOCK   Parent;
    PCM_KEY_CONTROL_BLOCK   Current;
    PCM_KEY_CONTROL_BLOCK   TmpKcb;
    ULONG                   ConvKey;
    BOOLEAN                 Result;

    CM_PAGED_CODE();

#if DBG
    //
    // make sure Start is an ancestor of End;
    //
    {
        ULONG LevelDiff = End->TotalLevels - Start->TotalLevels;

        ASSERT( (LONG)LevelDiff >= 0 );

        TmpKcb = End;
        for(;LevelDiff; LevelDiff--) {
            TmpKcb = TmpKcb->ParentKcb;
        }

        ASSERT( TmpKcb == Start );
    }
    
#endif
    //
    // Step 1: walk the path backwards (using the parentkcb link) and
    // revert it, until we reach Start. It is assumed that Start is an 
    // ancestor of End (the caller must not call this function otherwise !!!)
    //
    Current = NULL;
    Parent = End;
    while( Current != Start ) {
        //
        // revert the link
        //
        TmpKcb = Parent->ParentKcb;
        Parent->ParentKcb = Current;
        Current = Parent;
        Parent = TmpKcb;
        
        ASSERT( Current->TotalLevels >= Start->TotalLevels );
    }

    ASSERT( Current == Start );

    //
    // Step 2: Walk the forward path built at 1 and rehash the kcbs that need 
    // caching; At the same time, restore the links (parent relationships)
    //
    Result = FALSE;
    while( Current != NULL ) {
        //
        // see if we need to rehash this kcb;
        //
        //
        // restore the parent relationship; need to do this first so
        // CmpComputeKcbConvKey works OK
        //
        TmpKcb = Current->ParentKcb;
        Current->ParentKcb = Parent;

        ConvKey = CmpComputeKcbConvKey(Current);
        if( ConvKey != Current->ConvKey ) {
            //
            // rehash the kcb by removing it from hash, and then inserting it
            // again with th new ConvKey
            //
            CmpRemoveKeyHash(&(Current->KeyHash));
            Current->ConvKey = ConvKey;
            CmpInsertKeyHash(&(Current->KeyHash),FALSE);
            Result = TRUE;
        }

        //
        // advance forward
        //
        Parent = Current;
        Current = TmpKcb;
    }

    ASSERT( Parent == End );

    return Result;
}

BOOLEAN
CmpReferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
{
    // Note: this is called only under KCB lock
    LONG RefCount;
    
    ASSERT_KCB_LOCKED( KeyControlBlock ); 

    if( (KeyControlBlock->RefCount == 0) && 
        (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
        (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
        //
        // need to get the lock exclusive as we are changing the cache table
        // if we're here, the only way this kcb can go away from under us is if the delay close worker fires 
        // while we are dropping the lock; we set DelayCloseIndex to 1 (different than 0 that the worker uses)
        // to signal the worker to ignore this kcb; then after we get the lock exclusive, we put it back to 0 
        // (if it's still 1); We're the only ones playing with that value.
        //

        KeyControlBlock->DelayedCloseIndex = 1;

        // add an artificial refcount so it doesn't go away from under us while we drop the lock
        InterlockedIncrement( (PLONG)&KeyControlBlock->RefCount );
        CmpUpgradeKCBLockToExclusive(KeyControlBlock);

        // remove the artificial refcount we added above
        InterlockedDecrement( (PLONG)&KeyControlBlock->RefCount);
        if( KeyControlBlock->DelayedCloseIndex == 1 ) {
            KeyControlBlock->DelayedCloseIndex = 0;
        } else {
            // else someone beat us here
            ASSERT( (KeyControlBlock->DelayedCloseIndex == CmpDelayedCloseSize) ||
                    (KeyControlBlock->DelayedCloseIndex == 0)  );
        }
    }
    RefCount = (InterlockedIncrement( (PLONG)&KeyControlBlock->RefCount )) & 0xffff;
    if (RefCount == 0) {
        //
        // We have maxed out the ref count on this key. Probably
        // some bogus app has opened the same key 64K times without
        // ever closing it. Just fail the call
        //
        InterlockedDecrement( (PLONG)&KeyControlBlock->RefCount);
        return FALSE;
    } else if( KeyControlBlock->DelayedCloseIndex == 0 ) {
        if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
            (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
            //
            // we need to promote this kcb to EX; safe since we're already added a reference on it.
            //
            CmpUpgradeKCBLockToExclusive(KeyControlBlock);
        }
        if( KeyControlBlock->DelayedCloseIndex == 0 ) {
            CmpRemoveFromDelayedClose(KeyControlBlock);
        }
    }

    LogKCBReference(KeyControlBlock,1);
    return TRUE;
}

PCM_NAME_CONTROL_BLOCK
CmpGetNameControlBlock(
    PUNICODE_STRING NodeName,
    PBOOLEAN        NameUpCase
    )
{
    PCM_NAME_CONTROL_BLOCK   Ncb = NULL;
    ULONG  Cnt;
    WCHAR *Cp;
    WCHAR *Cp2;
    ULONG Index;
    ULONG i;
    ULONG   Size;
    PCM_NAME_HASH CurrentName;
    BOOLEAN NameFound = FALSE;
    USHORT NameSize;
    BOOLEAN NameCompressed;
    ULONG NameConvKey=0;

    //
    // Calculate the ConvKey for this NodeName;
    //
    *NameUpCase = TRUE;
    Cp = NodeName->Buffer;
    for (Cnt=0; Cnt<NodeName->Length; Cnt += sizeof(WCHAR)) {
        if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
            i = (ULONG)CmUpcaseUnicodeChar(*Cp);
            if( i != (*Cp) ) {
                *NameUpCase = FALSE;
            }
            NameConvKey = 37 * NameConvKey + i;
        }
        ++Cp;
    }

    //
    // Find the Name Size;
    // 
    NameCompressed = TRUE;
    NameSize = NodeName->Length / sizeof(WCHAR);
    for (i=0;i<NodeName->Length/sizeof(WCHAR);i++) {
        if ((USHORT)NodeName->Buffer[i] > (UCHAR)-1) {
            NameSize = NodeName->Length;
            NameCompressed = FALSE;
        }
    }

    CmpLockNameHashEntryExclusive(NameConvKey);
    Index = GET_HASH_INDEX(NameConvKey);
    CurrentName = CmpNameCacheTable[Index].Entry;

    while (CurrentName) {
        Ncb =  CONTAINING_RECORD(CurrentName, CM_NAME_CONTROL_BLOCK, NameHash);

        if ((NameConvKey == CurrentName->ConvKey) &&
            (NameSize == Ncb->NameLength)) {
            //
            // Hash value matches, compare the names.
            //
            NameFound = TRUE;
            if (Ncb->Compressed) {
                // we already know the name is uppercase
                if (CmpCompareCompressedName(NodeName, Ncb->Name, NameSize, CMP_DEST_UP)) {
                    NameFound = FALSE;
                }
            } else {
                Cp = (WCHAR *) NodeName->Buffer;
                Cp2 = (WCHAR *) Ncb->Name;
                for (i=0 ;i<Ncb->NameLength; i+= sizeof(WCHAR)) {
                    //
                    // Cp2 is always uppercase; see below
                    //
                    if (CmUpcaseUnicodeChar(*Cp) != (*Cp2) ) {
                        NameFound = FALSE;
                        break;
                    }
                    ++Cp;
                    ++Cp2;
                }
            }
            if (NameFound) {
                //
                // Found it, increase the refcount.
                //
                if ((USHORT) (Ncb->RefCount + 1) == 0) {
                    //
                    // We have maxed out the ref count.
                    // fail the call.
                    //
                    Ncb = NULL;
                } else {
                    ++Ncb->RefCount;
                }
                break;
            }
        }
        CurrentName = CurrentName->NextHash;
    }
    
    if (NameFound == FALSE) {
        //
        // Now need to create one Name block for this string.
        //
        Size = FIELD_OFFSET(CM_NAME_CONTROL_BLOCK, Name) + NameSize;
 
        Ncb = ExAllocatePoolWithTag(PagedPool,
                                    Size,
                                    CM_NAME_TAG | PROTECTED_POOL);
 
        if (Ncb == NULL) {
            CmpUnlockNameHashEntry(NameConvKey);
            return(NULL);
        }
        RtlZeroMemory(Ncb, Size);
 
        //
        // Update all the info for this newly created Name block.
        // Starting with Windows XP, the name is always uppercase in kcb name block
        //
        if (NameCompressed) {
            Ncb->Compressed = TRUE;
            for (i=0;i<NameSize;i++) {
                ((PUCHAR)Ncb->Name)[i] = (UCHAR)CmUpcaseUnicodeChar(NodeName->Buffer[i]);
            }
        } else {
            Ncb->Compressed = FALSE;
            for (i=0;i<NameSize/sizeof(WCHAR);i++) {
                Ncb->Name[i] = CmUpcaseUnicodeChar(NodeName->Buffer[i]);
            }
        }

        Ncb->ConvKey = NameConvKey;
        Ncb->RefCount = 1;
        Ncb->NameLength = NameSize;
        
        CurrentName = &(Ncb->NameHash);
        //
        // Insert into Name Hash table.
        //
        CurrentName->NextHash = CmpNameCacheTable[Index].Entry;
        CmpNameCacheTable[Index].Entry = CurrentName;
    }
    CmpUnlockNameHashEntry(NameConvKey);

    return(Ncb);
}


VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    )
{
    PCM_NAME_HASH *Prev;
    PCM_NAME_HASH Current;
    ULONG         ConvKey = Ncb->ConvKey;

    CmpLockNameHashEntryExclusive(ConvKey);
    if (--Ncb->RefCount == 0) {

        //
        // Remove it from the the Hash Table
        //
        Prev = &((GET_HASH_ENTRY(CmpNameCacheTable, Ncb->ConvKey)).Entry);
        
        while (TRUE) {
            Current = *Prev;
            ASSERT(Current != NULL);
            if (Current == &(Ncb->NameHash)) {
                *Prev = Current->NextHash;
                break;
            }
            Prev = &Current->NextHash;
        }

        //
        // Free storage
        //
        ExFreePoolWithTag(Ncb, CM_NAME_TAG | PROTECTED_POOL);
    }
    CmpUnlockNameHashEntry(ConvKey);
    return;
}

VOID
CmpRebuildKcbCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++
Routine Description:

    rebuilds all the kcb cache values from knode; this routine is intended to be called
    after a tree sync/copy

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    PCM_KEY_NODE    Node;

    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    ASSERT( !(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) ); 

    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    if( Node == NULL ) {
        // no can do
        return;
    }

    // subkey info;
    CmpCleanUpSubKeyInfo(KeyControlBlock);

    // value cache
    CmpCleanUpKcbValueCache(KeyControlBlock);
    CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

    // the rest of the cache
    KeyControlBlock->KcbLastWriteTime = Node->LastWriteTime;
    KeyControlBlock->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
    KeyControlBlock->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
    KeyControlBlock->KcbMaxValueDataLen = Node->MaxValueDataLen;
    HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
}

VOID
CmpCleanUpSubKeyInfo(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++
Routine Description:

    Clean up the subkey information cache due to create or delete keys.
    Registry is locked exclusively and no need to lock the KCB.

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    PCM_KEY_NODE    Node;

    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    if (KeyControlBlock->ExtFlags & (CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT)) {
        if (KeyControlBlock->ExtFlags & (CM_KCB_SUBKEY_HINT)) {
            ExFreePoolWithTag(KeyControlBlock->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
        }
        KeyControlBlock->ExtFlags &= ~((CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT));
    }
   
    //
    // Update the cached SubKeyCount in stored the kcb
    //
	if( KeyControlBlock->KeyCell == HCELL_NIL ) {
		//
		// prior call of ZwRestoreKey(REG_FORCE_RESTORE) invalidated this kcb
		//
		ASSERT( KeyControlBlock->Delete );
		Node = NULL;
	} else {
	    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
	}
    if( Node == NULL ) {
        //
        // insufficient resources; mark subkeycount as invalid
        //
        KeyControlBlock->ExtFlags |= CM_KCB_INVALID_CACHED_INFO;
    } else {
        KeyControlBlock->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;
        KeyControlBlock->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
        HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    }
    
}


VOID
CmpCleanUpKcbValueCache(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++

Routine Description:

    Clean up cached value/data that are associated to this key.

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    ULONG i;
    PULONG_PTR CachedList;

    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    if (CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) {
        CachedList = (PULONG_PTR) CMP_GET_CACHED_CELLDATA(KeyControlBlock->ValueCache.ValueList);
        for (i = 0; i < KeyControlBlock->ValueCache.Count; i++) {
            if (CMP_IS_CELL_CACHED(CachedList[i])) {

                // Trying to catch the BAD guy who writes over our pool.
                CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(CachedList[i]) );

                ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(CachedList[i]));
               
            }
        }

        // Trying to catch the BAD guy who writes over our pool.
        CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList) );

        ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(KeyControlBlock->ValueCache.ValueList));

        // Mark the ValueList as NULL 
        KeyControlBlock->ValueCache.ValueList = HCELL_NIL;

    } else if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
        //
        // This is a symbolic link key with symbolic name resolved.
        // Dereference to its real kcb and clear the bit.
        //
        if ((KeyControlBlock->ValueCache.RealKcb->RefCount == 1) && !(KeyControlBlock->ValueCache.RealKcb->Delete)) {
            KeyControlBlock->ValueCache.RealKcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;
        }
        CmpDelayDerefKeyControlBlock(KeyControlBlock->ValueCache.RealKcb);
        KeyControlBlock->ExtFlags &= ~CM_KCB_SYM_LINK_FOUND;
    }
}


VOID
CmpCleanUpKcbCacheWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    BOOLEAN                 RegLockHeldEx
    )
/*++

Routine Description:

    Clean up all cached allocations that are associated to this key.
    If the parent is still open just because of this one, Remove the parent as well.

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    PCM_KEY_CONTROL_BLOCK   Kcb;
    PCM_KEY_CONTROL_BLOCK   ParentKcb;

    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    Kcb = KeyControlBlock;

    ASSERT(KeyControlBlock->RefCount == 0);

    //
    // change in design: We are only freeing the current kcb and delay deref the parent
    //
    
    if(Kcb->RefCount == 0) {
        //
        // First, free allocations for Value/data.
        //
    
        CmpCleanUpKcbValueCache(Kcb);
    
        //
        // Free the kcb and dereference parentkcb and nameblock.
        //
    
        CmpDereferenceNameControlBlockWithLock(Kcb->NameBlock);
    
        if (Kcb->ExtFlags & CM_KCB_SUBKEY_HINT) {
            //
            // Now free the HintIndex allocation
            //
            ExFreePoolWithTag(Kcb->IndexHint, CM_CACHE_INDEX_TAG | PROTECTED_POOL);
        }

        //
        // Save the ParentKcb before we free the Kcb
        //
        ParentKcb = Kcb->ParentKcb;
        
        //
        // We cannot call CmpDereferenceKeyControlBlockWithLock so we can avoid recursion.
        //
        
        if (!Kcb->Delete) {
            CmpRemoveKeyControlBlock(Kcb);
        }
        SET_KCB_SIGNATURE(Kcb, '4FmC');

        CmpFreeKeyControlBlock( Kcb );
        
        if (ParentKcb) {
            if( RegLockHeldEx ) {
                CmpDereferenceKeyControlBlockWithLock(ParentKcb,RegLockHeldEx);
            } else {
                CmpDelayDerefKeyControlBlock(ParentKcb);
            }
        }
    }
}


PUNICODE_STRING
CmpConstructName(
    PCM_KEY_CONTROL_BLOCK kcb
)
/*++

Routine Description:

    Construct the name given a kcb.

Arguments:

    kcb - Kcb for the key

Return Value:

    Pointer to the unicode string constructed.  
    Caller is responsible to free this storage space.

--*/
{
    PUNICODE_STRING         FullName;
    PCM_KEY_CONTROL_BLOCK   TmpKcb;
    PCM_KEY_NODE            KeyNode;
    SIZE_T                  Length;
    SIZE_T                  size;
    USHORT                  i;
    SIZE_T                  BeginPosition;
    WCHAR                   *w1, *w2;
    UCHAR                   *u2;
    BOOLEAN                 DeletedPath = FALSE;

    //
    // Calculate the total string length.
    //
    Length = 0;
    TmpKcb = kcb;
    while (TmpKcb) {
        if (TmpKcb->NameBlock->Compressed) {
            Length += TmpKcb->NameBlock->NameLength * sizeof(WCHAR);
        } else {
            Length += TmpKcb->NameBlock->NameLength; 
        }
        //
        // Add the space for OBJ_NAME_PATH_SEPARATOR;
        //
        Length += sizeof(WCHAR);

        TmpKcb = TmpKcb->ParentKcb;
    }

    if (Length > MAXUSHORT) {
        return NULL;
    }

    //
    // Allocate the pool for the unicode string
    //
    size = sizeof(UNICODE_STRING) + Length;

    FullName = (PUNICODE_STRING) ExAllocatePoolWithTag(PagedPool,
                                                       size,
                                                       CM_NAME_TAG | PROTECTED_POOL);

    if (FullName) {
        FullName->Buffer = (USHORT *) ((ULONG_PTR) FullName + sizeof(UNICODE_STRING));
        FullName->Length = (USHORT) Length;
        FullName->MaximumLength = (USHORT) Length;

        //
        // Now fill the name into the buffer.
        //
        TmpKcb = kcb;
        BeginPosition = Length;

        while (TmpKcb) {
            if( (TmpKcb->KeyHive == NULL) || 
                (TmpKcb->ExtFlags & CM_KCB_KEY_NON_EXIST) ||
                ((!TmpKcb->Delete) && (TmpKcb->KeyCell == HCELL_NIL))  ) {
                ExFreePoolWithTag(FullName, CM_NAME_TAG | PROTECTED_POOL);
                FullName = NULL;
                break;
            }
            
#if defined(_WIN64)
            //
            // only go to the hive in case name hasn't been cached yet
            //
            if( TmpKcb->RealKeyName == NULL ) {
#endif
                if( !(TmpKcb->Delete || DeletedPath ) ) {
                    KeyNode = (PCM_KEY_NODE)HvGetCell(TmpKcb->KeyHive,TmpKcb->KeyCell);
                    if( KeyNode == NULL ) {
                        //
                        // could not allocate view
                        //
                        ExFreePoolWithTag(FullName, CM_NAME_TAG | PROTECTED_POOL);
                        FullName = NULL;
                        break;
                    }
                } else {
                    DeletedPath = TRUE;
                    KeyNode = NULL;
                }
#if defined(_WIN64)
            } else {
                KeyNode = NULL;
            }
#endif

            //
            // sanity
            //
#if DBG
            if( KeyNode && (!(TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT))) ) {
                ASSERT( KeyNode->NameLength == TmpKcb->NameBlock->NameLength );
                ASSERT( ((KeyNode->Flags&KEY_COMP_NAME) && (TmpKcb->NameBlock->Compressed)) ||
                        ((!(KeyNode->Flags&KEY_COMP_NAME)) && (!(TmpKcb->NameBlock->Compressed))) );
            }
#endif //DBG
            //
            // Calculate the begin position of each subkey. Then fill in the char.
            //
            //
            if (TmpKcb->NameBlock->Compressed) {
                BeginPosition -= (TmpKcb->NameBlock->NameLength + 1) * sizeof(WCHAR);
                w1 = &(FullName->Buffer[BeginPosition/sizeof(WCHAR)]);
                *w1 = OBJ_NAME_PATH_SEPARATOR;
                w1++;

                if( ! (TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT)) ) {
                    //
                    // Get the name from the knode; to preserve case
                    //
#if defined(_WIN64)
                    if( TmpKcb->RealKeyName == NULL ) {
                        //
                        // name not cached yet; we'll do it before we let go of KeyNode
                        //
                        if( KeyNode ) {
                            u2 = (UCHAR *) &(KeyNode->Name[0]);
                        } else {
                            //
                            // deleted key; need to get the name from the kcb
                            //
                            ASSERT( TmpKcb->Delete || DeletedPath );
                            u2 = (UCHAR *) &(TmpKcb->NameBlock->Name[0]);
                        }
                    } else if(TmpKcb->RealKeyName == CMP_KCB_REAL_NAME_UPCASE) {
                        //
                        // name is uppercase, use the one in the kcb name block
                        //
                        u2 = (UCHAR *) &(TmpKcb->NameBlock->Name[0]);
                    } else {
                        //
                        // we have previously cached this name; use that; it is guaranteed it doesn't go away from under us
                        // since the only 2 places where that can happen are:
                        //  1. NtRenameKey --> reglock held EX there
                        //  2. CmpFreeKeyControlBlock --> can't happen since we have a reference to this kcb
                        //
                        u2 = (UCHAR *) (TmpKcb->RealKeyName);
                    }
#else
                    if( KeyNode ) {
                        u2 = (UCHAR *) &(KeyNode->Name[0]);
                    } else {
                        //
                        // deleted key; need to get the name from the kcb
                        //
                        ASSERT( TmpKcb->Delete || DeletedPath );
                        u2 = (UCHAR *) &(TmpKcb->NameBlock->Name[0]);
                    }
#endif
                } else { 
                    //
                    // get it from the kcb, as in the keynode we don't hold the right name (see PROTO.HIV nodes)
                    //
                    u2 = (UCHAR *) &(TmpKcb->NameBlock->Name[0]);
                }

                for (i=0; i<TmpKcb->NameBlock->NameLength; i++) {
                    *w1 = (WCHAR)(*u2);
                    w1++;
                    u2++;
                }
            } else {
                BeginPosition -= (TmpKcb->NameBlock->NameLength + sizeof(WCHAR));
                w1 = &(FullName->Buffer[BeginPosition/sizeof(WCHAR)]);
                *w1 = OBJ_NAME_PATH_SEPARATOR;
                w1++;

                if( ! (TmpKcb->Flags & (KEY_HIVE_ENTRY | KEY_HIVE_EXIT)) ) {
                    //
                    // Get the name from the knode; to preserve case
                    //
#if defined(_WIN64)
                    if( TmpKcb->RealKeyName == NULL ) {
                        //
                        // name not cached yet; we'll do it before we let go of KeyNode
                        //
                        if( KeyNode ) {
                            w2 = KeyNode->Name;
                        } else {
                            //
                            // deleted key; need to get the name from the kcb
                            //
                            ASSERT( TmpKcb->Delete || DeletedPath );
                            w2 = TmpKcb->NameBlock->Name;
                        }
                    } else if(TmpKcb->RealKeyName == CMP_KCB_REAL_NAME_UPCASE) {
                        //
                        // name is uppercase, use the one in the kcb name block
                        //
                        w2 = TmpKcb->NameBlock->Name;
                    } else {
                        //
                        // we have previously cached this name; use that; it is guaranteed it doesn't go away from under us
                        // since the only 2 places where that can happen are:
                        //  1. NtRenameKey --> reglock held EX there
                        //  2. CmpFreeKeyControlBlock --> can't happen since we have a reference to this kcb
                        //
                        w2 = (WCHAR *) (TmpKcb->RealKeyName);
                    }
#else
                    if( KeyNode ) {
                        w2 = KeyNode->Name;
                    } else {
                        //
                        // deleted key; need to get the name from the kcb
                        //
                        ASSERT( TmpKcb->Delete || DeletedPath );
                        w2 = TmpKcb->NameBlock->Name;
                    }
#endif
                } else {
                    //
                    // get it from the kcb, as in the keynode we don't hold the right name (see PROTO.HIV nodes)
                    //
                    w2 = TmpKcb->NameBlock->Name;
                }
                for (i=0; i<TmpKcb->NameBlock->NameLength; i=i+sizeof(WCHAR)) {
                    *w1 = *w2;
                    w1++;
                    w2++;
                }
            }

#if defined(_WIN64)
            //
            // here's the challenging part; if the name is not yet cached; do it here; 
            // allocate pool for cached name; populate it from the KeyNode, then InterlockExchange it to the RealKeyName
            //
            if( (TmpKcb->RealKeyName == NULL) && (KeyNode != NULL) ) {
                PCHAR   CachedName = ExAllocatePoolWithTag(PagedPool,TmpKcb->NameBlock->NameLength,CM_NAME_TAG);
                
                if( CachedName != NULL ) {
                    
                    RtlCopyMemory(CachedName,KeyNode->Name,TmpKcb->NameBlock->NameLength);
                    if( InterlockedCompareExchangePointer(&(TmpKcb->RealKeyName),CachedName,NULL) != NULL ) {
                        //
                        // somebody else beat us here
                        //
                        ExFreePoolWithTag(CachedName, CM_NAME_TAG);
                    }
                }

            }
            //
            // we might end up with 2 threads racing in this routine, we can't rely on RealKeyName being NULL only
            //
#endif
            if( KeyNode != NULL ) {
                HvReleaseCell(TmpKcb->KeyHive,TmpKcb->KeyCell);
            }

            TmpKcb = TmpKcb->ParentKcb;
        }
    }
    return (FullName);
}

PCM_KEY_CONTROL_BLOCK
CmpCreateKeyControlBlock(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    PCM_KEY_NODE    Node,
    PCM_KEY_CONTROL_BLOCK ParentKcb,
    ULONG                   ControlFlags,
    PUNICODE_STRING KeyName
    )
/*++

Routine Description:

    Allocate and initialize a key control block, insert it into
    the kcb tree.

    Full path will be BaseName + '\' + KeyName, unless BaseName
    NULL, in which case the full path is simply KeyName.

    RefCount of returned KCB WILL have been incremented to reflect
    callers ref.

Arguments:

    Hive - Supplies Hive that holds the key we are creating a KCB for.

    Cell - Supplies Cell that contains the key we are creating a KCB for.

    Node - Supplies pointer to key node.

    ParentKcb - Parent kcb of the kcb to be created

    FakeKey - Whether the kcb to be create is a fake one or not

    KeyName - the subkey name to of the KCB to be created.
 
    NOTE:  We need the parameter instead of just using the name in the KEY_NODE 
           because there is no name in the root cell of a hive.

Return Value:

    NULL - failure (insufficient memory)
    else a pointer to the new kcb.

--*/
{
    PCM_KEY_CONTROL_BLOCK   kcb;
    PCM_KEY_CONTROL_BLOCK   kcbmatch=NULL;
    UNICODE_STRING          NodeName;
    ULONG                   ConvKey = 0;
    ULONG                   Cnt;
    WCHAR                   *Cp;
    BOOLEAN                 FakeKey;
    BOOLEAN                 NameUpCase;

    //
    // support for hive unloading in shared mode; do not allow other keys to be opened while unloading
    //
    if( Hive->HiveFlags & HIVE_IS_UNLOADING && (((PCMHIVE)Hive)->CreatorOwner != KeGetCurrentThread()) ) {
        return NULL;
    }

    FakeKey = (ControlFlags&CMP_CREATE_KCB_FAKE)?TRUE:FALSE;
    //
    // ParentKCb has the base hash value.
    //
    if (ParentKcb) {
        ConvKey = ParentKcb->ConvKey;
    }

    NodeName = *KeyName;

    while ((NodeName.Length > 0) && (NodeName.Buffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
        //
        // This must be the \REGISTRY.
        // Strip off the leading OBJ_NAME_PATH_SEPARATOR
        //
        NodeName.Buffer++;
        NodeName.Length -= sizeof(WCHAR);
    }

    //
    // Manually compute the hash to use.
    //
    ASSERT(NodeName.Length > 0);

    if (NodeName.Length) {
        Cp = NodeName.Buffer;
        for (Cnt=0; Cnt<NodeName.Length; Cnt += sizeof(WCHAR)) {
            //
            // UNICODE_NULL is a valid char !!!
            //
            if (*Cp != OBJ_NAME_PATH_SEPARATOR) {
                //(*Cp != UNICODE_NULL)) {
                ConvKey = 37 * ConvKey + (ULONG)CmUpcaseUnicodeChar(*Cp);
            }
            ++Cp;
        }
    }

    //
    // Create a new kcb, which we will free if one already exists
    // for this key.
    // Now it is a fixed size structure.
    //
    kcb = CmpAllocateKeyControlBlock( );

    if (kcb == NULL) {
        return(NULL);
    } else {
        SET_KCB_SIGNATURE(kcb, KCB_SIGNATURE);
        InitializeKCBKeyBodyList(kcb);
        kcb->Delete = FALSE;
        kcb->RefCount = 1;
        kcb->KeyHive = Hive;
        kcb->KeyCell = Cell;
        kcb->ConvKey = ConvKey;

        // Initialize as not on delayed close (0=1st delayed close slot)
        kcb->DelayedCloseIndex = CmpDelayedCloseSize;
#if defined(_WIN64)
        kcb->RealKeyName = NULL;
#endif

#if DBG
        kcb->InDelayClose = 0;
#endif //DBG
    }

    ASSERT_KCB(kcb);
    //
    // Find location to insert kcb in kcb tree.
    //
    if( !(ControlFlags & CMP_CREATE_KCB_KCB_LOCKED) ) {
        if( ParentKcb ) {
            //
            // need to lock them both atomically
            // 
            CmpLockTwoHashEntriesExclusive(ConvKey,ParentKcb->ConvKey);
        } else {
            CmpLockKCBExclusive(kcb);
        }
    }
    //
    // Add the KCB to the hash table
    //
    kcbmatch = CmpInsertKeyHash(&kcb->KeyHash, FakeKey);
    if (kcbmatch != NULL) {
        //
        // A match was found.
        //
        ASSERT(!kcbmatch->Delete);
        SET_KCB_SIGNATURE(kcb, '1FmC');

        CmpFreeKeyControlBlock(kcb);
        ASSERT_KCB(kcbmatch);
        kcb = kcbmatch;
        if( !CmpReferenceKeyControlBlock(kcb) ) {
            //
            // We have maxed out the ref count on this key. Probably
            // some bogus app has opened the same key 64K times without
            // ever closing it. Just fail the open, they've got enough
            // handles already.
            //
            ASSERT(kcb->RefCount + 1 != 0);
            kcb = NULL;
        } else {
            //
            // update the keycell and hive, in case this is a fake kcb
            //
            if( (kcb->ExtFlags & CM_KCB_KEY_NON_EXIST) && (!FakeKey) ) {
                kcb->ExtFlags = CM_KCB_INVALID_CACHED_INFO;
                kcb->KeyHive = Hive;
                kcb->KeyCell = Cell;
            }

            //
            // Update the cached information stored in the kcb, since we have the key_node handy
            //
            if (!(kcb->ExtFlags & (CM_KCB_NO_SUBKEY | CM_KCB_SUBKEY_ONE | CM_KCB_SUBKEY_HINT)) ) {
                // SubKeyCount
                kcb->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                // clean up the invalid flag (if any)
                kcb->ExtFlags &= ~CM_KCB_INVALID_CACHED_INFO;

            }

            kcb->KcbLastWriteTime = Node->LastWriteTime;
            kcb->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
            kcb->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
            kcb->KcbMaxValueDataLen = Node->MaxValueDataLen;
        }

    } else {
        //
        // No kcb created previously, fill in all the data.
        //

        //
        // Now try to reference the parentkcb
        //
        
        if (ParentKcb) {
            if ( ((ParentKcb->TotalLevels + 1) < CMP_MAX_REGISTRY_DEPTH) && (CmpReferenceKeyControlBlock(ParentKcb)) ) {
                kcb->ParentKcb = ParentKcb;
                kcb->TotalLevels = ParentKcb->TotalLevels + 1;
            } else {
                //
                // We have maxed out the ref count on the parent.
                // Since it has been cached in the cachetable,
                // remove it first before we free the allocation.
                //
                CmpRemoveKeyControlBlock(kcb);
                SET_KCB_SIGNATURE(kcb, '2FmC');

                CmpFreeKeyControlBlock(kcb);
                kcb = NULL;
            }
        } else {
            //
            // It is the \REGISTRY node.
            //
            kcb->ParentKcb = NULL;
            kcb->TotalLevels = 1;
        }

        if (kcb) {
            //
            // Cache the security cells in the kcb
            //
            CmpAssignSecurityToKcb(kcb,Node->Security,FALSE);

            //
            // Now try to find the Name Control block that has the name for this node.
            //
            kcb->NameBlock = CmpGetNameControlBlock (&NodeName,&NameUpCase);

            if (kcb->NameBlock) {
                //
                // Now fill in all the data needed for the cache.
                //
                kcb->ValueCache.Count = Node->ValueList.Count;                    
                kcb->ValueCache.ValueList = (ULONG_PTR)(Node->ValueList.List);
        
                kcb->Flags = Node->Flags;
                kcb->ExtFlags = 0;
                kcb->DelayedCloseIndex = CmpDelayedCloseSize;
        
                if (FakeKey) {
                    //
                    // The KCb to be created is a fake one; 
                    //
                    kcb->ExtFlags |= CM_KCB_KEY_NON_EXIST;
                }

                CmpTraceKcbCreate(kcb);
                PERFINFO_REG_KCB_CREATE(kcb);

                //
                // Update the cached information stored in the kcb, since we have the key_node handy
                //
                
                // SubKeyCount
                kcb->SubKeyCount = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                
                kcb->KcbLastWriteTime = Node->LastWriteTime;
                kcb->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
                kcb->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
                kcb->KcbMaxValueDataLen = Node->MaxValueDataLen;

#if defined(_WIN64)
                if(NameUpCase == TRUE) {
                    kcb->RealKeyName = CMP_KCB_REAL_NAME_UPCASE;
                }
#endif
            } else {
                //
                // We have maxed out the ref count on the Name.
                //
                
                //
                // First dereference the parent KCB.
                //
                CmpDereferenceKeyControlBlockWithLock(ParentKcb,FALSE);

                CmpRemoveKeyControlBlock(kcb);
                SET_KCB_SIGNATURE(kcb, '3FmC');

                CmpFreeKeyControlBlock(kcb);
                kcb = NULL;
            }
        }
    }
	if( kcb && IsHiveFrozen(Hive) && (!(kcb->Flags & KEY_SYM_LINK)) ) {
		//
		// kcbs created inside a frozen hive should not be added to delayclose table.
		//
		kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;

	}

    ASSERT( (!kcb) || (kcb->Delete == FALSE) );
    if( !(ControlFlags & CMP_CREATE_KCB_KCB_LOCKED) ) {
        if( ParentKcb ) {
            //
            // need to lock them both atomically
            // 
            CmpUnlockTwoHashEntries(ConvKey,ParentKcb->ConvKey);
        } else {
            // need to use ConvKey here since the kcb might not always be available
            CmpUnlockHashEntry(ConvKey);
        }
    }
    return kcb;
}

BOOLEAN
CmpSearchKeyControlBlockTree(
    PKCB_WORKER_ROUTINE WorkerRoutine,
    PVOID               Context1,
    PVOID               Context2
    )
/*++

Routine Description:

    Traverse the kcb tree.  We will visit all nodes unless WorkerRoutine
    tells us to stop part way through.

    For each node, call WorkerRoutine(..., Context1, Contex2).  If it returns
    KCB_WORKER_DONE, we are done, simply return.  If it returns
    KCB_WORKER_CONTINUE, just continue the search. If it returns KCB_WORKER_DELETE,
    the specified KCB is marked as deleted.
	If it returns KCB_WORKER_ERROR we bail out and signal the error to the caller.

    This routine has the side-effect of removing all delayed-close KCBs.

Arguments:

    WorkerRoutine - applied to nodes witch Match.

    Context1 - data we pass through

    Context2 - data we pass through


Return Value:

    NONE.

--*/
{
    PCM_KEY_CONTROL_BLOCK   Current;
    PCM_KEY_HASH *Prev;
    ULONG                   WorkerResult;
    ULONG                   i;

    //
    // Walk the hash table
    //
    for (i=0; i<CmpHashTableSize; i++) {
        CmpLockHashEntryByIndexExclusive(i);
        Prev = &(CmpCacheTable[i].Entry);
        while (*Prev) {
            Current = CONTAINING_RECORD(*Prev,
                                        CM_KEY_CONTROL_BLOCK,
                                        KeyHash);
            ASSERT_KCB(Current);
            ASSERT(!Current->Delete);
            if (Current->RefCount == 0) {
                //
                // This kcb is in DelayClose case, remove it.
                //
                CmpRemoveFromDelayedClose(Current);
                CmpCleanUpKcbCacheWithLock(Current,FALSE);

                //
                // The HashTable is changed, start over in this index again.
                //
                Prev = &(CmpCacheTable[i].Entry);
                continue;
            }

            WorkerResult = (WorkerRoutine)(Current, Context1, Context2);
            if (WorkerResult == KCB_WORKER_DONE) {
                CmpUnlockHashEntryByIndex(i);
                return TRUE;
            } else if (WorkerResult == KCB_WORKER_ERROR) {
                CmpUnlockHashEntryByIndex(i);
				return FALSE;
            } else if (WorkerResult == KCB_WORKER_DELETE) {
                ASSERT(Current->Delete);
                *Prev = Current->NextHash;
                continue;
            } else {
                ASSERT(WorkerResult == KCB_WORKER_CONTINUE);
                Prev = &Current->NextHash;
            }
        }
        CmpUnlockHashEntryByIndex(i);
    }

	return TRUE;
}

VOID
CmpDereferenceKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++

Routine Description:

    Decrements the reference count on a key control block, and frees it if it
    becomes zero.

    It is expected that no notify control blocks remain if the reference count
    becomes zero.

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    LONG OldRefCount;
    LONG NewRefCount;
    ULONG   ConvKey;

    OldRefCount = *(PLONG)&KeyControlBlock->RefCount; //get entire dword
    NewRefCount = OldRefCount - 1;
    if( (NewRefCount & 0xffff) > 0  &&
        InterlockedCompareExchange((PLONG)&KeyControlBlock->RefCount,NewRefCount,OldRefCount)
            == OldRefCount ) {
        LogKCBReference(KeyControlBlock,2);
        return;
    }

    // kcb may be freed from under us !
    ConvKey = KeyControlBlock->ConvKey;
    
    CmpLockKCBExclusive(KeyControlBlock);
    CmpDereferenceKeyControlBlockWithLock(KeyControlBlock,FALSE);
    CmpUnlockHashEntry(ConvKey);
    return;
}


VOID
CmpDereferenceKeyControlBlockWithLock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    BOOLEAN                 RegLockHeldEx
    )
{
    ASSERT_KCB(KeyControlBlock);
    ASSERT_KCB_LOCKED(KeyControlBlock);

    if( (InterlockedDecrement( (PLONG)&KeyControlBlock->RefCount ) & 0xffff) == 0) {
        LogKCBReference(KeyControlBlock,2);
        ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);
        //
        // Remove kcb from the tree
        //
        // delay close disabled during boot; up to the point CCS is saved.
        // for symbolic links, we still need to keep the symbolic link kcb around.
        //
        if((CmpHoldLazyFlush && (!(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND)) && (!(KeyControlBlock->Flags & KEY_SYM_LINK))) || 
            (KeyControlBlock->ExtFlags & CM_KCB_NO_DELAY_CLOSE) ) {
            //
            // Free storage directly so we can clean up junk quickly.
            //
            //
            // Need to free all cached Index List, Index Leaf, Value, etc.
            //
            CmpCleanUpKcbCacheWithLock(KeyControlBlock,RegLockHeldEx);
        } else if (!KeyControlBlock->Delete) {

            //
            // Put this kcb on our delayed close list.
            //
            CmpAddToDelayedClose(KeyControlBlock,RegLockHeldEx);

        } else {
            //
            // Free storage directly as there is no point in putting this on
            // our delayed close list.
            //
            //
            // Need to free all cached Index List, Index Leaf, Value, etc.
            //
            CmpCleanUpKcbCacheWithLock(KeyControlBlock,RegLockHeldEx);
        }
    }
    return;
}

VOID
CmpRemoveKeyControlBlock(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock
    )
/*++

Routine Description:

    Remove a key control block from the KCB tree.

    It is expected that no notify control blocks remain.

    The kcb will NOT be freed, call DereferenceKeyControlBlock for that.

    This call assumes the KCB tree is already locked or registry is locked exclusively.

Arguments:

    KeyControlBlock - pointer to a key control block.

Return Value:

    NONE.

--*/
{
    ASSERT_KCB(KeyControlBlock);
    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    //
    // Remove the KCB from the hash table
    //
    CmpRemoveKeyHash(&KeyControlBlock->KeyHash);

    return;
}


BOOLEAN
CmpFreeKeyBody(
    PHHIVE Hive,
    HCELL_INDEX Cell
    )
/*++

Routine Description:

    Free storage for the key entry Hive.Cell refers to, including
    its class and security data.  Will NOT free child list or value list.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of key to free

Return Value:

    TRUE - success

    FALSE - error; couldn't map cell
--*/
{
    PCELL_DATA key;

    //
    // map in the cell
    //
    key = HvGetCell(Hive, Cell);
    if( key == NULL ) {
        //
        // we couldn't map the bin containing this cell
        // Sorry, we cannot free the keybody
        // this shouldn't happen as the cell must've been
        // marked dirty (i.e. pinned in memory) by now
        //
        ASSERT( FALSE );
        return FALSE;
    }

    if (!(key->u.KeyNode.Flags & KEY_HIVE_EXIT)) {
        if (key->u.KeyNode.Security != HCELL_NIL) {
            HvFreeCell(Hive, key->u.KeyNode.Security);
        }

        if (key->u.KeyNode.ClassLength > 0) {
            HvFreeCell(Hive, key->u.KeyNode.Class);
        }
    }

    HvReleaseCell(Hive,Cell);

    //
    // unmap the cell itself and free it
    //
    HvFreeCell(Hive, Cell);

    return TRUE;
}

PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    )
/*++

Routine Description:

    Adds a key hash structure to the hash table. The hash table
    will be checked to see if a duplicate entry already exists. If
    a duplicate is found, its kcb will be returned. If a duplicate is not
    found, NULL will be returned.

Arguments:

    KeyHash - Supplies the key hash structure to be added.

Return Value:

    NULL - if the supplied key has was added
    PCM_KEY_HASH - The duplicate hash entry, if one was found

--*/

{
    ULONG Index;
    PCM_KEY_HASH Current;

    ASSERT_KEY_HASH(KeyHash);
    Index = GET_HASH_INDEX(KeyHash->ConvKey);

    //
    // If this is a fake key, we will use the cell and hive from its 
    // parent for uniqeness.  To deal with the case when the fake
    // has the same ConvKey as its parent (in which case we cannot distingish 
    // between the two), we set the lowest bit of the fake key's cell.
    //
    // It's possible (unlikely) that we cannot distingish two fake keys 
    // (when their Convkey's are the same) under the same key.  It is not breaking
    // anything, we just cannot find the other one in cache lookup.
    //
    //
    if (FakeKey) {
        KeyHash->KeyCell++;
    }

    //
    // First look for duplicates.
    //
    Current = CmpCacheTable[Index].Entry;
    while (Current) {
        ASSERT_KEY_HASH(Current);
        //
        // We must check ConvKey since we can create a fake kcb
        // for keys that does not exist.
        // We will use the Hive and Cell from the parent.
        //

        if ((KeyHash->ConvKey == Current->ConvKey) &&
            (KeyHash->KeyCell == Current->KeyCell) &&
            (KeyHash->KeyHive == Current->KeyHive)) {
            //
            // Found a match
            //
            return(CONTAINING_RECORD(Current,
                                     CM_KEY_CONTROL_BLOCK,
                                     KeyHash));
        }
        Current = Current->NextHash;
    }

    //
    // No duplicate was found, add this entry at the head of the list
    //
    KeyHash->NextHash = CmpCacheTable[Index].Entry;
    CmpCacheTable[Index].Entry = KeyHash;
    return(NULL);
}

VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    )
/*++

Routine Description:

    Removes a key hash structure from the hash table.

Arguments:

    KeyHash - Supplies the key hash structure to be deleted.

Return Value:

    None

--*/

{
    ULONG Index;
    PCM_KEY_HASH *Prev;
    PCM_KEY_HASH Current;

    ASSERT_KEY_HASH(KeyHash);

    Index = GET_HASH_INDEX(KeyHash->ConvKey);

    //
    // Find this entry.
    //
    Prev = &(CmpCacheTable[Index].Entry);
    while (TRUE) {
        Current = *Prev;
        ASSERT(Current != NULL);
        ASSERT_KEY_HASH(Current);
        if (Current == KeyHash) {
            *Prev = Current->NextHash;
#if DBG
            if (*Prev) {
                ASSERT_KEY_HASH(*Prev);
            }
#endif
            break;
        }
        Prev = &Current->NextHash;
    }
}


VOID
CmpInitializeCache()
{
    ULONG   i;
    ULONG   TotalCmCacheSize;
    TotalCmCacheSize = CmpHashTableSize * sizeof(CM_KEY_HASH_TABLE_ENTRY);

    CmpCacheTable = ExAllocatePoolWithTag(PagedPool,
                                          TotalCmCacheSize,
                                          'aCMC');
    if (CmpCacheTable == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_CACHE_TABLE,1,0,0);
        return;
    }
    RtlZeroMemory(CmpCacheTable, TotalCmCacheSize);
    
    //
    // initialize each entry lock
    //
#pragma prefast(suppress:12009, "silence prefast")
    for(i=0;i<CmpHashTableSize;i++) {
        ExInitializePushLock(&(CmpCacheTable[i].Lock));
    }

    TotalCmCacheSize = CmpHashTableSize * sizeof(CM_NAME_HASH_TABLE_ENTRY);
    CmpNameCacheTable = ExAllocatePoolWithTag(PagedPool,
                                              TotalCmCacheSize,
                                              'aCMC');
    if (CmpNameCacheTable == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_CACHE_TABLE,3,0,0);
        return;
    }
    RtlZeroMemory(CmpNameCacheTable, TotalCmCacheSize);

    for(i=0;i<CmpHashTableSize;i++) {
        ExInitializePushLock(&(CmpNameCacheTable[i].Lock));
    }

    CmpInitializeDelayedCloseTable();
}

//
// lock free keybody array ==> convert these to macros
//
VOID InitializeKCBKeyBodyList(IN PCM_KEY_CONTROL_BLOCK kcb)
{
    ULONG   i;

    InitializeListHead(&(kcb->KeyBodyListHead));
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        kcb->KeyBodyArray[i] = NULL;
    }
}

VOID EnlistKeyBodyWithKCB(IN PCM_KEY_BODY   KeyBody,
                          IN ULONG          ControlFlags )
{
    ULONG   i;
    
    ASSERT( KeyBody->KeyControlBlock != NULL );
    ASSERT_KCB_LOCKED(KeyBody->KeyControlBlock);

    CmpSetNoCallers(KeyBody);
    InitializeListHead(&(KeyBody->KeyBodyList));   

    // 
    // try the fast path first; no locks required here.
    //
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        if(InterlockedCompareExchangePointer(&(KeyBody->KeyControlBlock->KeyBodyArray[i]),
                                               KeyBody,
                                               NULL) == NULL) {
            //
            // we're lucky, it worked.
            //
            return;
        }
    }
    
    //
    // slow path
    //
    if( ControlFlags & CMP_ENLIST_KCB_LOCKED_SHARED ) {
        CmpUnlockKCB(KeyBody->KeyControlBlock);     
        ASSERT( !(ControlFlags & CMP_ENLIST_KCB_LOCKED_EXCLUSIVE) );
    } 
    if( !(ControlFlags & CMP_ENLIST_KCB_LOCKED_EXCLUSIVE) ) {
        CmpLockKCBExclusive(KeyBody->KeyControlBlock);                                      
    }                                                                                       
    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyBody->KeyControlBlock);
    InsertTailList(&(KeyBody->KeyControlBlock->KeyBodyListHead),&(KeyBody->KeyBodyList));   
    if( !(ControlFlags & (CMP_ENLIST_KCB_LOCKED_SHARED|CMP_ENLIST_KCB_LOCKED_EXCLUSIVE)) ) {  
        CmpUnlockKCB(KeyBody->KeyControlBlock);                                             
    }                                                                                       
}

VOID DelistKeyBodyFromKCB(IN PCM_KEY_BODY   KeyBody,
                          IN BOOLEAN        LockHeld )
{
    ULONG       i;
    ULONG_PTR   Value;

    ASSERT( KeyBody->KeyControlBlock != NULL );
Retry:
    // 
    // try the fast path first; no locks required here.
    //
    for(i=0;i<CMP_LOCK_FREE_KEY_BODY_ARRAY_SIZE;i++) {
        Value = (ULONG_PTR)InterlockedCompareExchangePointer(&(KeyBody->KeyControlBlock->KeyBodyArray[i]),
                                               NULL,
                                               KeyBody);
        if( Value == (ULONG_PTR)KeyBody) {
            //
            // we're lucky, it worked.
            //
            return;
        }
        if( (Value == CMP_FAST_KEY_BODY_ARRAY_DUMP) ||
            (Value == CMP_FAST_KEY_BODY_ARRAY_FLUSH) ) {
            //
            // other routine is performing work on this array; retry until they are done
            //
            goto Retry;
        }
    }

    //
    // slow path
    //
    ASSERT(IsListEmpty(&(KeyBody->KeyControlBlock->KeyBodyListHead)) == FALSE);
    ASSERT(IsListEmpty(&(KeyBody->KeyBodyList)) == FALSE);
    if( !(LockHeld) ) {                                                                     
        CmpLockKCBExclusive(KeyBody->KeyControlBlock);                                      
    }                                                                                       
    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyBody->KeyControlBlock);
    RemoveEntryList(&(KeyBody->KeyBodyList));                                               
    if( !(LockHeld) ) {                                                                     
        CmpUnlockKCB(KeyBody->KeyControlBlock);                                             
    }                                                                                       
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

