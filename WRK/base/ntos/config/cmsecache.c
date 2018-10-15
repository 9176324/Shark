/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmsecache.c

Abstract:

    This module implements the security cache.

--*/

#include "cmp.h"

#define SECURITY_CACHE_GROW_INCREMENTS  0x10

BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    );

PCM_KEY_SECURITY_CACHE
CmpFindReusableCellFromCache(IN PCMHIVE     CmHive,
                             IN HCELL_INDEX SecurityCell,
                             IN ULONG       PreviousCount
                             );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpSecConvKey)
#pragma alloc_text(PAGE,CmpInitSecurityCache)
#pragma alloc_text(PAGE,CmpDestroySecurityCache)
#pragma alloc_text(PAGE,CmpRebuildSecurityCache)
#pragma alloc_text(PAGE,CmpAddSecurityCellToCache)
#pragma alloc_text(PAGE,CmpFindSecurityCellCacheIndex)
#pragma alloc_text(PAGE,CmpAdjustSecurityCacheSize)
#pragma alloc_text(PAGE,CmpRemoveFromSecurityCache)
#pragma alloc_text(PAGE,CmpFindMatchingDescriptorCell)
#pragma alloc_text(PAGE,CmpAssignSecurityToKcb)
#pragma alloc_text(PAGE,CmpFindReusableCellFromCache)
#pragma alloc_text(PAGE,CmpBuildSecurityCellMappingArray)
#endif

ULONG
CmpSecConvKey(
              IN ULONG  DescriptorLength,
              IN PULONG Descriptor
              )
/*++

Routine Description:

    Computes the ConvKey for the given security descriptor.
    The algorithm is taken from the NTFS security hash. 
    (it was proven to be efficient there; why shouldn't do the same ?)


    For speed in the hash, we consider the security descriptor as an array
    of ULONGs.  The fragment at the end that is ignored should not affect
    the collision nature of this hash.

Arguments:

    DescriptorLength - length (in bytes) of the sd

    Descriptor - actual sd to cache

Return Value:

    ConvKey

Note:
    
      We may want to convert this to a macro
--*/

{
    ULONG   Count;     
    ULONG   Hash = 0;

    PAGED_CODE();

    Count = DescriptorLength / 4;

    while (Count--) {
        Hash = ((Hash << 3) | (Hash >> (32-3))) + *Descriptor++;
    }

    return Hash;
}

VOID
CmpInitSecurityCache(
    IN OUT PCMHIVE      CmHive
    )
{
    ULONG i;

    PAGED_CODE();

    CmHive->SecurityCache = NULL;        
    CmHive->SecurityCacheSize = 0;       
    CmHive->SecurityCount = 0;
    CmHive->SecurityHitHint = -1; // no hint

    for( i=0;i<CmpSecHashTableSize;i++) {
        InitializeListHead(&(CmHive->SecurityHash[i]));
    }
}

NTSTATUS
CmpAddSecurityCellToCache (
    IN OUT PCMHIVE              CmHive,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  BuildUp,
    IN PCM_KEY_SECURITY_CACHE   SecurityCached
    )

/*++

Routine Description:

    This routine adds the specified security cell to the cache of the
    specified hive. It takes care of cache allocation (grow) as well.
    At build up time, cache size grows with a PAGE_SIZE, to avoid memory 
    fragmentation. After the table is built, it's size is adjusted (most 
    of the hives never add new security cells). Then, at run-time, the size 
    grows with 16 entries at a time (same reason)
    The cache is ordered by the cell's index, so we can do a binary search on 
    cells retrieval.

Arguments:

    CmHive - the hive to which the security cell belongs

    SecurityCell - the security cell to be added to the cache

    BuildUp - specifies that this is build up time 

    SecurityCached - if not NULL it means we have it already allocated (this happens when rebuilding the cache).

Return Value:

    NTSTATUS - STATUS_SUCCESS if the operation is successful and an
        appropriate error status otherwise (i.e STATUS_INSUFFICIENT_RESOURCES).

Note: 

    If the security cell is already IN the cache; this function will return TRUE.
--*/
{
    ULONG                   Index;
    PCM_KEY_SECURITY        Security;

    PAGED_CODE();

    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == TRUE ) {
        // 
        // cell already exist in the cache; return;
        //
        return STATUS_SUCCESS;
    }

    //
    // if this fails, we're doomed !
    //
    ASSERT( (PAGE_SIZE % sizeof(CM_KEY_SECURITY_CACHE_ENTRY)) == 0 );

    //
    // check if the cache can accommodate a new cell
    //
    if( CmHive->SecurityCount == CmHive->SecurityCacheSize ) {
        //
        // We're at the limit with the cache; we need to extend it by a page
        //
        // OBS: this takes care of the first allocation too, as SecurityCount 
        // and SecurityCacheSize are both initialized with 0
        //
        PCM_KEY_SECURITY_CACHE_ENTRY  Temp;

        // store the actual buffer
        Temp = CmHive->SecurityCache;
        
        //
        // compute the new size and allocate a new buffer 
        //
        if( BuildUp == TRUE ) {
            //
            // We are building up the cache; grow the table in page increments
            //
            ASSERT( ((CmHive->SecurityCacheSize * sizeof(CM_KEY_SECURITY_CACHE_ENTRY)) % PAGE_SIZE) == 0 );
            CmHive->SecurityCacheSize += (PAGE_SIZE / sizeof(CM_KEY_SECURITY_CACHE_ENTRY));
        } else {
            //
            // normal case (running time); a new security cell is added; grow the
            // table with a fixed number of increments (to avoid fragmentation, in
            // case of an Office install :-) )
            //
            CmHive->SecurityCacheSize += SECURITY_CACHE_GROW_INCREMENTS;

        }
        CmRetryExAllocatePoolWithTag(PagedPool, CmHive->SecurityCacheSize * sizeof(CM_KEY_SECURITY_CACHE_ENTRY),
                                    CM_SECCACHE_TAG|PROTECTED_POOL,CmHive->SecurityCache);
        if( CmHive->SecurityCache == NULL ) {
            //
            // bad luck; bail out
            //
            CmHive->SecurityCache = Temp;
            CmHive->SecurityCacheSize = CmHive->SecurityCount;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        //
        // copy existing data in the new location and free the old buffer
        //
        RtlCopyMemory(CmHive->SecurityCache,Temp,CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY));
        if( Temp != NULL ) {
            ExFreePoolWithTag(Temp, CM_SECCACHE_TAG|PROTECTED_POOL );
        } else {
            ASSERT( CmHive->SecurityCount == 0 );
        }
    }

    //
    // try first to get the security cell from the hive; if this fails, there is no point to go on 
    //
    Security = (PCM_KEY_SECURITY)HvGetCell(&(CmHive->Hive),SecurityCell);
    if( Security == NULL ){
        //
        // we failed to map the view containing this cell; bail out
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( !SecurityCached ) {
        ULONG                   Size;
        //
        // compute the size for the cached security structure
        //
        Size = FIELD_OFFSET(CM_KEY_SECURITY_CACHE,Descriptor) + Security->DescriptorLength;

        //
        // think forward: allocate and initialize a copy for the security cell, in order to store it in the cache
        //
        CmRetryExAllocatePoolWithTag(PagedPool,Size,CM_SECCACHE_TAG|PROTECTED_POOL,SecurityCached);
        if(SecurityCached == NULL) {
            //
            // bad luck; bail out
            //
            HvReleaseCell(&(CmHive->Hive),SecurityCell);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    //
    // from now on, nothing can go wrong !
    //
    RtlCopyMemory(&(SecurityCached->Descriptor),&(Security->Descriptor),Security->DescriptorLength);
    SecurityCached->Cell = SecurityCell;
    SecurityCached->DescriptorLength = Security->DescriptorLength;

    //
    // now add this to the hash table
    //
    SecurityCached->ConvKey = CmpSecConvKey(Security->DescriptorLength,(PULONG)(&(Security->Descriptor)));
    // add it to the end of the list with this conv key
    InsertTailList( &(CmHive->SecurityHash[SecurityCached->ConvKey % CmpSecHashTableSize]),
                    &(SecurityCached->List)
                   );
    
    HvReleaseCell(&(CmHive->Hive),SecurityCell);

    //
    // At this point we are sure we have space for at least one more entry
    // Move data to make room for the new entry
    //
    if( Index < CmHive->SecurityCount ) {
        //
        // RtlMoveMemory will take care of the overlapping problem
        //
        RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (Index+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),     // destination
                       ((PUCHAR)CmHive->SecurityCache) + Index*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),         // source
                       (CmHive->SecurityCount - Index)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)                  // size
                        );
    }

    //
    // setup the new entry
    //
    CmHive->SecurityCache[Index].Cell = SecurityCell;
    CmHive->SecurityCache[Index].CachedSecurity = SecurityCached;

    // update the count
    CmHive->SecurityCount++;

    return STATUS_SUCCESS;
}

BOOLEAN
CmpFindSecurityCellCacheIndex (
    IN PCMHIVE      CmHive,
    IN HCELL_INDEX  SecurityCell,
    OUT PULONG      Index
    )

/*++

Routine Description:

    Search (binary) for the specified cellindex in the security cache.
    Returns the index of the cache entry where the cell is cached or 
    it should be added

Arguments:

    CmHive - the hive to which the security cell belongs

    SecurityCell - the security cell to search for

    Index - out param to pass the index at which the cell is (or it should be)

Return Value:

    TRUE - the cell was found (at *Index)
    FALSE - the cell is not in the cache (it should be added at *Index)

--*/
{
    ULONG           High;
    ULONG           Low;
    ULONG           Current;
    USHORT          State = 0;  // state of the operation:  0 - normal binary search
                                //                          1 - last low
                                //                          2 - last high
    LONG            Result;
    LONG            Tmp1,Tmp2;
    
    PAGED_CODE();

    if( CmHive->SecurityCount == 0 ) {
        //
        // there is no cell in the security cache
        //
        *Index = 0;
        return FALSE;
    }

    // sanity asserts
    ASSERT( CmHive->SecurityCount <= CmHive->SecurityCacheSize );
    ASSERT( CmHive->SecurityCache != NULL );


    High = CmHive->SecurityCount - 1;
    Low = 0;
    if( (CmHive->SecurityHitHint >= 0) && ( (ULONG)CmHive->SecurityHitHint <= High) ) {
        //
        // try the last search
        //
        Current = CmHive->SecurityHitHint;
    } else {
        Current = High/2;
    }

    // sign adjustment
    Tmp1 = SecurityCell & ~HCELL_TYPE_MASK;
    if( SecurityCell & HCELL_TYPE_MASK ) {
        Tmp1 = -Tmp1;
    }

    while( TRUE ) {

        Tmp2 = CmHive->SecurityCache[Current].Cell & ~HCELL_TYPE_MASK;
        // sign adjustment
        if( CmHive->SecurityCache[Current].Cell & HCELL_TYPE_MASK ) {
            Tmp2 = -Tmp2;
        }

        Result = Tmp1 -  Tmp2;    
        
        if (Result == 0) {
            //
            // Success, return data to caller and exit
            //

            *Index = Current;
            //
            // we have a hit! update the count and exit
            //
            CmHive->SecurityHitHint = Current;
            return TRUE;
        }
        //
        // compute the next index to try
        //
        switch(State) {
        case 0:
            //
            // normal binary search state
            //
            if( Result < 0 ) {
                High = Current;
            } else {
                Low = Current;
            }
            if ((High - Low) <= 1) {
                //
                // advance to the new state
                //
                Current = Low;
                State = 1;
            } else {
                Current = Low + ( (High-Low) / 2 );
            }
            break;
        case 1:
            //
            // last low state
            //

            // this should be true
            ASSERT( Current == Low );
            if (Result < 0) {
                //
                // does not exist, under
                //
            
                *Index = Current;
                return FALSE;
            } else if( Low == High ) {
                        //
                        // low and high are identical; but current is bigger than them; insert after
                        //

                        *Index = Current + 1;
                        return FALSE;
                    } else {
                        //
                        // advance to the new state; i.e. look at high
                        //
                        State = 2;
                        Current = High;
                    }

            break;
        case 2:
            //
            // last high state; if we got here, High = Low +1 and Current == High
            //
            ASSERT( Current == High);
            ASSERT( High == (Low + 1) );
            if( Result < 0 ) {
                //
                // under High, but above Low; we should insert it here
                //

                *Index = Current;
                return FALSE;
            } else {
                //
                // above High; 
                //

                *Index = Current + 1;
                return FALSE;
            }
            break;
        default:
            ASSERT( FALSE );
            break;
        }
    }

    //
    // we shouldn't get here !!!
    //
    ASSERT( FALSE );
    return FALSE;
}

BOOLEAN
CmpAdjustSecurityCacheSize (
    IN PCMHIVE      CmHive
    )

/*++

Routine Description:

    Adjust the scusrity cache size for the specified hive. This function
    should be called after all the security cells for the hive were cached, 
    in order to give back extra memory used in the process.


Arguments:

    CmHive - the hive to which the security cell belongs

Return Value:

    TRUE - success
    FALSE - failure - the size remains the same

--*/
{
    PCM_KEY_SECURITY_CACHE_ENTRY  Buffer;
    
    PAGED_CODE();

    if( CmHive->SecurityCount < CmHive->SecurityCacheSize ) {
        //
        // cache size is bigger than what we need; there is a good chance 
        // nobody will ever add new security cells to this hive, so go on
        // and free the extra space
        //

        //
        // allocate a new buffer with the exact size we need
        //
        CmRetryExAllocatePoolWithTag(PagedPool, CmHive->SecurityCount * sizeof(CM_KEY_SECURITY_CACHE_ENTRY),
                                        CM_SECCACHE_TAG|PROTECTED_POOL,Buffer);
        
        if( Buffer == NULL ) {
            //
            // the system is low on resources; leave the cache as it is
            //
            return FALSE;
        }

        //
        // copy significant data inot the new buffer
        //
        RtlCopyMemory(Buffer,CmHive->SecurityCache,CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY));

        //
        // free the old buffer and update cache members
        //
        ExFreePoolWithTag(CmHive->SecurityCache, CM_SECCACHE_TAG|PROTECTED_POOL );
        
        CmHive->SecurityCache = Buffer;
        CmHive->SecurityCacheSize = CmHive->SecurityCount;
    }

    return TRUE;
}

VOID
CmpRemoveFromSecurityCache (
    IN OUT PCMHIVE      CmHive,
    IN HCELL_INDEX      SecurityCell
    )

/*++

Routine Description:

    Removes the specified security cell from the security cache.
    (only if present !)
    For performance (and memory fragmentation) reasons, it does not 
    change (shrink) the cache size.

Arguments:

    CmHive - the hive to which the security cell belongs

    SecurityCell - the security cell to be removed from the cache

Return Value:

    <none>
--*/
{
    ULONG               Index;

    PAGED_CODE();

    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == FALSE ) {
        // 
        // cell is not in the cache
        //
        return;
    }

    ASSERT( CmHive->SecurityCache[Index].Cell == SecurityCell );
    ASSERT( CmHive->SecurityCache[Index].CachedSecurity->Cell == SecurityCell );
    
    //
    // remove the cached structure from the hash
    //
    CmpRemoveEntryList(&(CmHive->SecurityCache[Index].CachedSecurity->List));
    
    //
    // free up the cached security cell;
    //
    ExFreePoolWithTag(CmHive->SecurityCache[Index].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );

    //
    // move memory to reflect the new size, and update the cache count
    //
    RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + Index*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),         // destination
                   ((PUCHAR)CmHive->SecurityCache) + (Index+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),     // source
                   (CmHive->SecurityCount - Index - 1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)              // size   
                 );
    
    CmHive->SecurityCount--;
}

VOID
CmpDestroySecurityCache (
    IN OUT PCMHIVE      CmHive
    )
/*++

Routine Description:

    Frees up all the cached security cells and the cache itself

Arguments:

    CmHive - the hive to which the security cell belongs

Return Value:

    <none>
--*/
{
    ULONG   i;

    PAGED_CODE();

    for( i=0;i<CmHive->SecurityCount;i++) {
        CmpRemoveEntryList(&(CmHive->SecurityCache[i].CachedSecurity->List));
        ExFreePoolWithTag(CmHive->SecurityCache[i].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );
    }

    if( CmHive->SecurityCount != 0 ) {
        ASSERT( CmHive->SecurityCache != NULL );
        ExFreePoolWithTag(CmHive->SecurityCache, CM_SECCACHE_TAG|PROTECTED_POOL );
    }

    CmHive->SecurityCache = NULL;
    CmHive->SecurityCacheSize = CmHive->SecurityCount = 0;
}

PCM_KEY_SECURITY_CACHE
CmpFindReusableCellFromCache(IN PCMHIVE     CmHive,
                             IN HCELL_INDEX SecurityCell,
                             IN ULONG       PreviousCount)
/*++

Routine Description:

    Attempts to find the smallest cell which can accommodate the current security cell.
    Then moves it at the end and return a pointer to it. Shifts the array towards the end 
	as we are going to extend the cache

    Security doesn't change too often, so chances are that we will be able to reuse the cells 90% 
    of the time.

    If one cannot be found, the last cell in the array is freed. A new one will be allocated inside
    CmpAddSecurityCellToCache

Arguments:

    CmHive - the hive to which the security cell belongs

    SecurityCell - the security cell 

    PreviousCount - the end of the array

Return Value:

    the cached cell or NULL
--*/
{
    ULONG                   Size;
    PCM_KEY_SECURITY        Security;
    PCM_KEY_SECURITY_CACHE  SecurityCached;
    ULONG                   i;
    ULONG                   SmallestSize = 0;
    ULONG                   SmallestIndex = 0;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    //
    // try first to get the security cell from the hive; if this fails, there is no point to go on 
    //
    Security = (PCM_KEY_SECURITY)HvGetCell(&(CmHive->Hive),SecurityCell);
    if( Security == NULL ){
        //
        // we failed to map the view containing this cell; bail out
        //
        goto ErrorExit;
    }

    //
    // compute the size for the cached security structure
    //
    Size = Security->DescriptorLength;
    HvReleaseCell(&(CmHive->Hive),SecurityCell);

    //
    // Now that we know the desired size, start iterating the array to find a suitable entry.
    //
    for(i = CmHive->SecurityCount; i < PreviousCount; i++) {
       SecurityCached =  CmHive->SecurityCache[i].CachedSecurity;
       if( SecurityCached->DescriptorLength == Size ) {
Found:
            //
            // we have found one matching the exact size; move it at the end 
			// shift to the end one entry as we are going to extend the cache
			//
			// this factored down translates to:
            //
			RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (CmHive->SecurityCount+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),     // destination
						   ((PUCHAR)CmHive->SecurityCache) + CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),         // source
						   (i - CmHive->SecurityCount)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)							// size
							);

            return SecurityCached;
       }

       //
       // if not; record the smallest suitable entry
       //
       if( SecurityCached->DescriptorLength > Size ) {
           if( (SmallestSize == 0) ||
               (SmallestSize > SecurityCached->DescriptorLength)
               ) {
               //
               // first one or this one is smaller
               //
               SmallestSize = SecurityCached->DescriptorLength;
               SmallestIndex = i;
           } 
       }
    }

    if( SmallestSize != 0 ) {
        SecurityCached = CmHive->SecurityCache[SmallestIndex].CachedSecurity;
        ASSERT( SecurityCached->DescriptorLength == SmallestSize );
        ASSERT( Size < SmallestSize );
        i = SmallestIndex;
        goto Found;
    }

ErrorExit:
    ExFreePoolWithTag(CmHive->SecurityCache[PreviousCount - 1].CachedSecurity, CM_SECCACHE_TAG|PROTECTED_POOL );
	//
	// shift to the end one entry as we are going to extend the cache
	//
	RtlMoveMemory( ((PUCHAR)CmHive->SecurityCache) + (CmHive->SecurityCount+1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),     // destination
				   ((PUCHAR)CmHive->SecurityCache) + CmHive->SecurityCount*sizeof(CM_KEY_SECURITY_CACHE_ENTRY),         // source
				   (PreviousCount - CmHive->SecurityCount - 1)*sizeof(CM_KEY_SECURITY_CACHE_ENTRY)							// size
					);
    return NULL;
}

BOOLEAN
CmpRebuildSecurityCache(
                        IN OUT PCMHIVE      CmHive
                        )
/*++

Routine Description:

    Rebuilds the security cache by reiterating all security cells
    and adding them to the cache; this routine is intended for hive
    refresh operations

Arguments:

    CmHive - the hive to which the security cell belongs

Return Value:

    TRUE or FALSE
--*/
{
    PCM_KEY_NODE            RootNode;
    PCM_KEY_SECURITY        SecurityCell;
    HCELL_INDEX             ListAnchor;
    HCELL_INDEX             NextCell;
    HCELL_INDEX             LastCell = 0;
    PHHIVE                  Hive;
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;
    BOOLEAN                 Result = TRUE;
    ULONG                   PreviousCount;
    PCM_KEY_SECURITY_CACHE  SecCache;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    //
    // avoid extra work
    //
    Hive = &(CmHive->Hive);
    ReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

    //
    // be smart and reuse the cache; For each cell, we'll iterate through the cache
    // find an entry big enough to accommodate the current cell, move it at the end 
    // and then reuse it. 
    //
    // first, reinitialize the hash table.
    //
    for( PreviousCount=0;PreviousCount<CmpSecHashTableSize;PreviousCount++) {
        InitializeListHead(&(CmHive->SecurityHash[PreviousCount]));
    }

    //
    // this we use to keep track of how many valid cells were in the cache previously
    //
    PreviousCount = CmHive->SecurityCount;
    //
    // start building an pseudo-empty one.
    //
    CmHive->SecurityCount = 0;

    if (!HvIsCellAllocated(Hive,Hive->BaseBlock->RootCell)) {
        //
        // root cell HCELL_INDEX is bogus
        //
        Result = FALSE;
        goto JustReturn;
    }
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    if( RootNode == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        Result = FALSE;
        goto JustReturn;
    }
    ListAnchor = NextCell = RootNode->Security;

    do {
        if (!HvIsCellAllocated(Hive, NextCell)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CM: CmpRebuildSecurityCache\n"));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"    NextCell: %08lx is invalid HCELL_INDEX\n",NextCell));
            Result = FALSE;
            goto JustReturn;
        }
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        if( SecurityCell == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            Result = FALSE;
            goto JustReturn;
        }
        if (NextCell != ListAnchor) {
            //
            // Check to make sure that our Blink points to where we just
            // came from.
            //
            if (SecurityCell->Blink != LastCell) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  Invalid Blink (%ld) on security cell %ld\n",SecurityCell->Blink, NextCell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  should point to %ld\n", LastCell));
                Result = FALSE;
                goto JustReturn;
            }
        }
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidSD:  SD shared by %d nodes\n",SecurityCell->ReferenceCount));
        if (!SeValidSecurityDescriptor(SecurityCell->DescriptorLength, &SecurityCell->Descriptor)) {
#if DBG
            CmpDumpSecurityDescriptor(&SecurityCell->Descriptor,"INVALID DESCRIPTOR");
#endif
            Result = FALSE;
            goto JustReturn;
        }

        SecCache = CmpFindReusableCellFromCache(CmHive,NextCell,PreviousCount);

        if( !NT_SUCCESS(CmpAddSecurityCellToCache ( CmHive,NextCell,TRUE,SecCache) ) ) {
            Result = FALSE;
            goto JustReturn;
        }

        LastCell = NextCell;
        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );


JustReturn:
    Hive->ReleaseCellRoutine = ReleaseCellRoutine;
    return Result;
}

BOOLEAN
CmpFindMatchingDescriptorCell(
    IN PCMHIVE CmHive,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Type,
    OUT PHCELL_INDEX MatchingCell,
    OUT OPTIONAL PCM_KEY_SECURITY_CACHE *CachedSecurityPointer
    )

/*++

Routine Description:

    This routine attempts to find a security descriptor in the hive that
    is identical to the one passed in.  If it finds one, it returns its
    cell index.

    Obsolete:
    Currently, this routine checks the security descriptors of the parent
    and siblings of the node to find a match.

    New:
    It looks for the sd in the security cache for this hive. This will 
    eliminate duplicates and make the search process faster.

Arguments:

    CmHive - Supplies a pointer to the hive control structure for the node.
            Needed to get access to the cache

    SecurityDescriptor - Supplies the cooked security descriptor which
           should be searched for.

    Type - Indicates whether the Security Descriptor that matches must
            be in Stable or Volatile store

    MatchingCell - Returns the cell index of a security cell whose
           security descriptor is identical to SecurityDescriptor.
           Valid only if TRUE is returned.

    CachedSecurityPointer - pointer to the cached security (for update reasons)

Return Value:

    TRUE - Matching security descriptor found.  MatchingCell returns the
           cell index of the matching security descriptor.

    FALSE - No matching security descriptor found.  MatchingCell is invalid.

--*/

{
    ULONG                   DescriptorLength;
    ULONG                   ConvKey;
    PLIST_ENTRY             ListAnchor;
    PLIST_ENTRY             Current;
    PCM_KEY_SECURITY_CACHE  CachedSecurity;

    PAGED_CODE();
	
    DescriptorLength = RtlLengthSecurityDescriptor(SecurityDescriptor);

    //
    // calculate the conv key
    //
    ConvKey = CmpSecConvKey(DescriptorLength,(PULONG)SecurityDescriptor);

    ListAnchor = &(CmHive->SecurityHash[ConvKey % CmpSecHashTableSize]);
    if( IsListEmpty(ListAnchor) == TRUE ) {
        return FALSE;
    }

    //
    // iterate through the list of colisions for this convkey
    // start with teh first element in list
    //
    Current = (PLIST_ENTRY)(ListAnchor->Flink);
    while( Current != ListAnchor ){
        //
        // get the current cached security 
        //
        CachedSecurity = CONTAINING_RECORD(Current,
                                           CM_KEY_SECURITY_CACHE,
                                           List);

        //
        // see if it matches with the given descriptor; 
        //
        if( (CachedSecurity->ConvKey == ConvKey) &&                             // same convkey
            (Type == HvGetCellType(CachedSecurity->Cell)) &&                    // same cell type
            (DescriptorLength == CachedSecurity->DescriptorLength) &&  // same length
            (RtlEqualMemory(SecurityDescriptor,                                 // and, finally, bit-wise identical
                            &(CachedSecurity->Descriptor),
                            DescriptorLength))
            ) {
            //
            // we have found a match
            //
            *MatchingCell = CachedSecurity->Cell;
            if (ARGUMENT_PRESENT(CachedSecurityPointer)) {
                *CachedSecurityPointer = CachedSecurity;
            }
            return TRUE;
        }

        //
        // advance to the next element
        //
        Current = (PLIST_ENTRY)(Current->Flink);
    } 

    // sorry, no match
    return FALSE;
}

VOID
CmpAssignSecurityToKcb(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN HCELL_INDEX              SecurityCell,
    IN BOOLEAN                  SecurityLocked
    )
/*++

Routine Description:

    Establishes the connection between the KCB and the cached security
    descriptor.

    As most of the time this is called after the security cell has been 
    linked to the Key Node, and because the binary search starts with 
    the last cell looked up, we will not hit a performance impact here.

Arguments:

    Kcb - the KCb to which this security cell needs to be attached

    SecurityCell - Security cell for the kcb


Return Value:

    NONE; bugchecks on error

--*/
{
    ULONG   Index;
    PCMHIVE CmHive;

    PAGED_CODE();

    if( SecurityCell == HCELL_NIL ) {
        Kcb->CachedSecurity = NULL;
        return;
    }

    CmHive = (PCMHIVE)(Kcb->KeyHive);

    if( !SecurityLocked ) {
        CmLockHiveSecurityShared(CmHive);
    }
    //
    // get the security descriptor from cache
    //
    if( CmpFindSecurityCellCacheIndex (CmHive,SecurityCell,&Index) == FALSE ) {
        Kcb->CachedSecurity = NULL;
        //
        //  we are doomed !!!
        //
        CM_BUGCHECK( REGISTRY_ERROR,BAD_SECURITY_CACHE,1,Kcb,SecurityCell);

    } 

    //
    // success; link the cached security to this KCB
    //
    Kcb->CachedSecurity = CmHive->SecurityCache[Index].CachedSecurity;
    if( !SecurityLocked ) {
        CmUnlockHiveSecurity(CmHive);
    }
}

BOOLEAN
CmpBuildSecurityCellMappingArray(
    IN PCMHIVE CmHive
    )
/*++

Routine Description:

    Iterates through the security cache for the specified hive and 
	build the array of mappings.

Arguments:

    CmHive - the hive in question

Return Value:

    TRUE/FALSE
--*/
{
    ULONG                   i;
    PAGED_CODE();

	ASSERT( CmHive->CellRemapArray == NULL );
	CmHive->CellRemapArray = ExAllocatePool(PagedPool,sizeof(CM_CELL_REMAP_BLOCK)*CmHive->SecurityCount);

	if( CmHive->CellRemapArray == NULL ) {
		return FALSE;
	}

#pragma prefast(suppress:12009, "silence prefast")
    for( i=0;i<CmHive->SecurityCount;i++) {
		CmHive->CellRemapArray[i].OldCell = CmHive->SecurityCache[i].Cell;
		if( HvGetCellType(CmHive->SecurityCache[i].Cell) == (ULONG)Volatile ) {
			//
			// we preserve volatile cells
			//
			CmHive->CellRemapArray[i].NewCell = CmHive->SecurityCache[i].Cell;
		} else {
			CmHive->CellRemapArray[i].NewCell = HCELL_NIL;
		}
    }

	return TRUE;
}

