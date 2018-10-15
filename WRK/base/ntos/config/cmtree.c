/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmtree.c

Abstract:

    This module contains cm routines that understand the structure
    of the registry tree.

--*/

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpGetValueListFromCache)
#pragma alloc_text(PAGE,CmpGetValueKeyFromCache)
#pragma alloc_text(PAGE,CmpFindValueByNameFromCache)
#pragma alloc_text(PAGE,CmpGetValueDataFromCache)
#endif


VALUE_SEARCH_RETURN_TYPE
CmpGetValueListFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    OUT PCELL_DATA          *List,
    OUT BOOLEAN             *IndexCached,
    OUT PHCELL_INDEX        ValueListToRelease
)
/*++

Routine Description:

    Get the Valve Index Array.  Check if it is already cached, if not, cache it and return
    the cached entry.

Arguments:

    Hive - pointer to hive control structure for hive of interest

    ChildList - pointer/index to the Value Index array

    IndexCached - Indicating whether Value Index list is cached or not.

Return Value:

    Pointer to the Valve Index Array.

    NULL when we could not map view 

--*/
{
    HCELL_INDEX             CellToRelease;
    PCACHED_CHILD_LIST      ChildList;
    PHHIVE                  Hive;
#ifndef _WIN64
    ULONG                   AllocSize;
    PCM_CACHED_VALUE_INDEX  CachedValueIndex;
    ULONG                   i;
#endif
    ASSERT_KCB_LOCKED(KeyControlBlock);

    Hive = KeyControlBlock->KeyHive;
    ChildList = &(KeyControlBlock->ValueCache);
    *ValueListToRelease = HCELL_NIL;

#ifndef _WIN64
    *IndexCached = TRUE;
    if (CMP_IS_CELL_CACHED(ChildList->ValueList)) {
        //
        // The entry is already cached.
        //
        *List = CMP_GET_CACHED_CELLDATA(ChildList->ValueList);
    } else {
        //
        // The entry is not cached.  The element contains the hive index.
        // ensure exclusive lock.
        //
        if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
            (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
            //
            // need to upgrade lock to exclusive
            //
            return SearchNeedExclusiveLock;
        }

        CellToRelease = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
        *List = (PCELL_DATA) HvGetCell(Hive, CellToRelease);
        if( *List == NULL ) {
            //
            // we couldn't map a view for this cell
            //
            *IndexCached = FALSE; 
            return SearchFail;
        }

        //
        // Allocate a PagedPool to cache the value index cell.
        //

        AllocSize = ChildList->Count * sizeof(ULONG_PTR) + FIELD_OFFSET(CM_CACHED_VALUE_INDEX, Data);
        CachedValueIndex = (PCM_CACHED_VALUE_INDEX) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_INDEX_TAG);

        if (CachedValueIndex) {

            CachedValueIndex->CellIndex = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
#pragma prefast(suppress:12009, "no overflow")
            for (i=0; i<ChildList->Count; i++) {
                CachedValueIndex->Data.List[i] = (ULONG_PTR) (*List)->u.KeyList[i];
            }

            ChildList->ValueList = CMP_MARK_CELL_CACHED(CachedValueIndex);

            // Trying to catch the BAD guy who writes over our pool.
            CmpMakeSpecialPoolReadOnly( CachedValueIndex );

            //
            // Now we have the stuff cached, use the cache data.
            //
            *List = CMP_GET_CACHED_CELLDATA(ChildList->ValueList);
        } else {
            //
            // If the allocation fails, just do not cache it. continue.
            //
            *IndexCached = FALSE; 
        }
        *ValueListToRelease = CellToRelease;
    }
#else
    CellToRelease = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
    *List = (PCELL_DATA) HvGetCell(Hive, CellToRelease);
    *IndexCached = FALSE;
    if( *List == NULL ) {
        //
        // we couldn't map a view for this cell
        // OBS: we can drop this as we return List anyway; just for clarity
        //
        return SearchFail;
    }
    *ValueListToRelease = CellToRelease;
#endif

    return SearchSuccess;
}

VALUE_SEARCH_RETURN_TYPE
CmpGetValueKeyFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PCELL_DATA           List,
    IN ULONG                Index,
    OUT PPCM_CACHED_VALUE   *ContainingList,
    OUT PCM_KEY_VALUE       *Value,
    IN BOOLEAN              IndexCached,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
)
/*++

Routine Description:

    Get the Valve Node.  Check if it is already cached, if not but the index is cached, 
    cache and return the value node.

Arguments:

    Hive - pointer to hive control structure for hive of interest.

    List - pointer to the Value Index Array (of ULONG_PTR if cached and ULONG if non-cached)

    Index - Index in the Value index array

    ContainlingList - The address of the entry that will receive the found cached value.

    IndexCached - Indicate if the index list is cached.  If not, everything is from the
                  original registry data.

    ValueCached - Indicating whether Value is cached or not.

Return Value:

    Pointer to the Value Node.

    NULL when we couldn't map a view 
--*/
{
    PULONG_PTR          CachedList;
    ULONG               AllocSize;
    ULONG               CopySize;
    PCM_CACHED_VALUE    CachedValue;
    PHHIVE              Hive;

    *CellToRelease = HCELL_NIL;
    Hive = KeyControlBlock->KeyHive;
    *Value = NULL;
    if (IndexCached) {
        //
        // The index array is cached, so List is pointing to an array of ULONG_PTR.
        // Use CachedList.
        //
        CachedList = (PULONG_PTR) List;
        *ValueCached = TRUE;
        if (CMP_IS_CELL_CACHED(CachedList[Index])) {
            *Value = CMP_GET_CACHED_KEYVALUE(CachedList[Index]);
            *ContainingList = &((PCM_CACHED_VALUE) CachedList[Index]);
        } else {
            //
            // ensure exclusive lock.
            //
            if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
                (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
                //
                // need to upgrade lock to exclusive
                //
                return SearchNeedExclusiveLock;
            }
            *Value = (PCM_KEY_VALUE) HvGetCell(Hive, List->u.KeyList[Index]);
            if( *Value == NULL ) {
                //
                // we couldn't map a view for this cell
                // just return NULL; the caller must handle it gracefully
                //
                return SearchFail;
            }
            *CellToRelease = List->u.KeyList[Index];

            //
            // Allocate a PagedPool to cache the value node.
            //
            CopySize = (ULONG) HvGetCellSize(Hive, *Value);
            AllocSize = CopySize + FIELD_OFFSET(CM_CACHED_VALUE, KeyValue);
            
            CachedValue = (PCM_CACHED_VALUE) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_TAG);

            if (CachedValue) {
                //
                // Set the information for later use if we need to cache data as well.
                //
                if ((*Value)->Flags & VALUE_COMP_NAME) {
                    CachedValue->HashKey = CmpComputeHashKeyForCompressedName(0,(*Value)->Name,(*Value)->NameLength);
                } else {
                    UNICODE_STRING TmpStr;
                    TmpStr.Length = (*Value)->NameLength;
                    TmpStr.Buffer = (*Value)->Name;
                    CachedValue->HashKey = CmpComputeHashKey(0,&TmpStr
#if DBG
                                                             , TRUE
#endif
                        );
                }
                CachedValue->DataCacheType = CM_CACHE_DATA_NOT_CACHED;
                CachedValue->ValueKeySize = (USHORT) CopySize;

                RtlCopyMemory((PVOID)&(CachedValue->KeyValue), *Value, CopySize);


                // Trying to catch the BAD guy who writes over our pool.
                CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(CachedList) );

                CachedList[Index] = CMP_MARK_CELL_CACHED(CachedValue);

                // Trying to catch the BAD guy who writes over our pool.
                CmpMakeSpecialPoolReadOnly( CMP_GET_CACHED_ADDRESS(CachedList) );


                // Trying to catch the BAD guy who writes over our pool.
                CmpMakeSpecialPoolReadOnly(CachedValue);

                *ContainingList = &((PCM_CACHED_VALUE) CachedList[Index]);
                //
                // Now we have the stuff cached, use the cache data.
                //
                (*Value) = CMP_GET_CACHED_KEYVALUE(CachedValue);
            } else {
                //
                // If the allocation fails, just do not cache it. continue.
                //
                *ValueCached = FALSE;
            }
        }
    } else {
        //
        // The Valve Index Array is from the registry hive, just get the cell and move on.
        //
        (*Value) = (PCM_KEY_VALUE) HvGetCell(Hive, List->u.KeyList[Index]);
        *ValueCached = FALSE;
        if( *Value == NULL ) {
            //
            // we couldn't map a view for this cell
            // just return NULL; the caller must handle it gracefully
            // OBS: we may remove this as we return pchild anyway; just for clarity
            //
            return SearchFail;
        }
        *CellToRelease = List->u.KeyList[Index];
    }
    return SearchSuccess;
}

VALUE_SEARCH_RETURN_TYPE
CmpFindValueByNameFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PUNICODE_STRING      Name,
    OUT PPCM_CACHED_VALUE   *ContainingList,
    OUT ULONG               *Index,
    OUT PCM_KEY_VALUE       *Value,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
    )
/*++

Routine Description:

    Find a value node given a value list array and a value name.  It sequentially walk
    through each value node to look for a match.  If the array and 
    value nodes touched are not already cached, cache them.

Arguments:

    Hive - pointer to hive control structure for hive of interest

    ChildList - pointer/index to the Value Index array

    Name - name of value to find

    ContainlingList - The address of the entry that will receive the found cached value.

    Index - pointer to variable to receive index for child
    
    ValueCached - Indicate if the value node is cached.

Return Value:

    HCELL_INDEX for the found cell
    HCELL_NIL if not found


Notes:
    
    New hives (Minor >= 4) have ValueList sorted; this implies ValueCache is sorted too;
    So, we can do a binary search here!

--*/
{
    UNICODE_STRING      Candidate;
    LONG                Result;
    PCELL_DATA          List;
    BOOLEAN             IndexCached;
    ULONG               Current;
    HCELL_INDEX         ValueListToRelease = HCELL_NIL;
    ULONG               HashKey = 0;
    PHHIVE              Hive = KeyControlBlock->KeyHive;
    PCACHED_CHILD_LIST  ChildList = &(KeyControlBlock->ValueCache);
    VALUE_SEARCH_RETURN_TYPE    ret = SearchFail;

    ASSERT_KCB_LOCKED(KeyControlBlock);

    *CellToRelease = HCELL_NIL;
    *Value = NULL;

    if (ChildList->Count != 0) {
        ret = CmpGetValueListFromCache(KeyControlBlock,&List, &IndexCached,&ValueListToRelease);
        if( ret != SearchSuccess ) {
            //
            // retry with exclusive lock, since we need to update the cache
            // or fail altogether
            //
            ASSERT( (ret == SearchFail) || (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) );    
            ASSERT( ValueListToRelease == HCELL_NIL );    
            return ret;
        } 
        
        if( IndexCached ) {
            try {
                HashKey = CmpComputeHashKey(0,Name
#if DBG
                                            , TRUE
#endif
                    );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                ret = SearchFail;
                goto Exit;
            }
        }
        //
        // old plain hive; simulate a for
        //
        Current = 0;

        while( TRUE ) {
            if( *CellToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,*CellToRelease);
                *CellToRelease = HCELL_NIL;
            }
            ret =  CmpGetValueKeyFromCache(KeyControlBlock, List, Current, ContainingList, Value, IndexCached, ValueCached, CellToRelease);
            if( ret != SearchSuccess ) {
                //
                // retry with exclusive lock, since we need to update the cache
                // or fail altogether
                //
                ASSERT( (ret == SearchFail) || (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) );    
                ASSERT( *CellToRelease == HCELL_NIL );    
                return ret;
            } 

            //
            // only compare names when hash matches.
            //
            if( IndexCached && (*ValueCached) && (HashKey != ((PCM_CACHED_VALUE)((CMP_GET_CACHED_ADDRESS(**ContainingList))))->HashKey) ) {
                //
                // no hash match; skip it
                //
#if DBG
                try {
                    //
                    // Name has user-mode buffer.
                    //

                    if ((*Value)->Flags & VALUE_COMP_NAME) {
                        Result = CmpCompareCompressedName(  Name,
                                                            (*Value)->Name,
                                                            (*Value)->NameLength,
                                                            0);
                    } else {
                        Candidate.Length = (*Value)->NameLength;
                        Candidate.MaximumLength = Candidate.Length;
                        Candidate.Buffer = (*Value)->Name;
                        Result = RtlCompareUnicodeString(   Name,
                                                            &Candidate,
                                                            TRUE);
                    }


                } except (EXCEPTION_EXECUTE_HANDLER) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmpFindValueByNameFromCache: code:%08lx\n", GetExceptionCode()));
                    //
                    // the caller will bail out. Some ,alicious caller altered the Name buffer since we probed it.
                    //
                    *Value = NULL;
                    ret = SearchFail;
                    goto Exit;
                }
                ASSERT( Result != 0 );
#endif
                Result = 1;
            } else {
                try {
                    //
                    // Name has user-mode buffer.
                    //

                    if( (*Value)->Flags & VALUE_COMP_NAME) {
                        Result = CmpCompareCompressedName(  Name,
                                                            (*Value)->Name,
                                                            (*Value)->NameLength,
                                                            0);
                    } else {
                        Candidate.Length = (*Value)->NameLength;
                        Candidate.MaximumLength = Candidate.Length;
                        Candidate.Buffer = (*Value)->Name;
                        Result = RtlCompareUnicodeString(   Name,
                                                            &Candidate,
                                                            TRUE);
                    }


                } except (EXCEPTION_EXECUTE_HANDLER) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmpFindValueByNameFromCache: code:%08lx\n", GetExceptionCode()));
                    //
                    // the caller will bail out. Some ,alicious caller altered the Name buffer since we probed it.
                    //
                    *Value = NULL;
                    ret = SearchFail;
                    goto Exit;
                }
            }
            if (Result == 0) {
                //
                // Success, fill the index, return data to caller and exit
                //
                *Index = Current;
                ret = SearchSuccess;
                goto Exit;
            }

            //
            // compute the next index to try: old'n plain hive; go on
			//
            Current++;
            if( Current == ChildList->Count ) {
                //
                // we've reached the end of the list; nicely return
                //
                (*Value) = NULL;
                ret = SearchFail;
                goto Exit;
            }

        } // while(TRUE)
    }

    //
    // in the new design we shouldn't get here; we should exit the while loop with return
    //
    ASSERT( ChildList->Count == 0 );    

Exit:
    if( ValueListToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueListToRelease);
    }
    return ret;
}

VALUE_SEARCH_RETURN_TYPE
CmpGetValueDataFromCache(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PPCM_CACHED_VALUE    ContainingList,
    IN PCELL_DATA           ValueKey,
    IN BOOLEAN              ValueCached,
    OUT PUCHAR              *DataPointer,
    OUT PBOOLEAN            Allocated,
    OUT PHCELL_INDEX        CellToRelease
)
/*++

Routine Description:

    Get the cached Value Data given a value node.

Arguments:

    Hive - pointer to hive control structure for hive of interest

    ContainingList - Address that stores the allocation address of the value node.
                     We need to update this when we do a re-allocate to cache
                     both value key and value data.

    ValueKey - pointer to the Value Key

    ValueCached - Indicating whether Value key is cached or not.

    DataPointer - out param to receive a pointer to the data

    Allocated - out param telling if the caller should free the DataPointer

Return Value:

    TRUE - data was retrieved
    FALSE - some error (STATUS_INSUFFICIENT_RESOURCES) occured

Note:
    
    The caller is responsible for freeing the DataPointer when we signal it to him
    by setting Allocated on TRUE;

    Also we must be sure that MAXIMUM_CACHED_DATA is smaller than CM_KEY_VALUE_BIG
--*/
{
    //
    // Cache the data if needed.
    //
    PCM_CACHED_VALUE OldEntry;
    PCM_CACHED_VALUE NewEntry;
    PUCHAR      Cacheddatapointer;
    ULONG       AllocSize;
    ULONG       CopySize;
    ULONG       DataSize;
    PHHIVE      Hive;

    ASSERT( MAXIMUM_CACHED_DATA < CM_KEY_VALUE_BIG );

    //
    // this routine should not be called for small data
    //
    ASSERT( (ValueKey->u.KeyValue.DataLength & CM_KEY_VALUE_SPECIAL_SIZE) == 0 );

    ASSERT_KCB_LOCKED(KeyControlBlock);
    
    Hive = KeyControlBlock->KeyHive;
    //
    // init out params
    //
    *DataPointer = NULL;
    *Allocated = FALSE;
    *CellToRelease = HCELL_NIL;

    if (ValueCached) {
        OldEntry = (PCM_CACHED_VALUE) CMP_GET_CACHED_ADDRESS(*ContainingList);
        if (OldEntry->DataCacheType == CM_CACHE_DATA_CACHED) {
            //
            // Data is already cached, use it.
            //
            *DataPointer = (PUCHAR) ((ULONG_PTR) ValueKey + OldEntry->ValueKeySize);
        } else {
            //
            // ensure exclusive lock.
            //
            if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
                (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
                //
                // need to upgrade lock to exclusive
                //
                return SearchNeedExclusiveLock;
            }
            if ((OldEntry->DataCacheType == CM_CACHE_DATA_TOO_BIG) ||
                (ValueKey->u.KeyValue.DataLength > MAXIMUM_CACHED_DATA ) 
               ){
                //
                // Mark the type and do not cache it.
                //
                OldEntry->DataCacheType = CM_CACHE_DATA_TOO_BIG;

                //
                // Data is too big to warrant caching, get it from the registry; 
                // - regardless of the size; we might be forced to allocate a buffer
                //
                if( CmpGetValueData(Hive,&(ValueKey->u.KeyValue),&DataSize,DataPointer,Allocated,CellToRelease) == FALSE ) {
                    //
                    // insufficient resources; return NULL
                    //
                    ASSERT( *Allocated == FALSE );
                    ASSERT( *DataPointer == NULL );
                    return SearchFail;
                }

            } else {
                //
                // consistency check
                //
                ASSERT(OldEntry->DataCacheType == CM_CACHE_DATA_NOT_CACHED);

                //
                // Value data is not cached.
                // Check the size of value data, if it is smaller than MAXIMUM_CACHED_DATA, cache it.
                //
                // Anyway, the data is for sure not stored in a big data cell (see test above)
                //
                //
                *DataPointer = (PUCHAR)HvGetCell(Hive, ValueKey->u.KeyValue.Data);
                if( *DataPointer == NULL ) {
                    //
                    // we couldn't map this cell
                    // the caller must handle this gracefully !
                    //
                    return SearchFail;
                }
                //
                // inform the caller it has to release this cell
                //
                *CellToRelease = ValueKey->u.KeyValue.Data;
                
                //
                // copy only valid data; cell might be bigger
                //
                DataSize = (ULONG)ValueKey->u.KeyValue.DataLength;

                //
                // consistency check
                //
                ASSERT(DataSize <= MAXIMUM_CACHED_DATA);

                //
                // Data is not cached and now we are going to do it.
                // Reallocate a new cached entry for both value key and value data.
                //
                CopySize = DataSize + OldEntry->ValueKeySize;
                AllocSize = CopySize + FIELD_OFFSET(CM_CACHED_VALUE, KeyValue);

                NewEntry = (PCM_CACHED_VALUE) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_DATA_TAG);

                if (NewEntry) {
                    //
                    // Now fill the data to the new cached entry
                    //
                    NewEntry->HashKey = OldEntry->HashKey;
                    NewEntry->DataCacheType = CM_CACHE_DATA_CACHED;
                    NewEntry->ValueKeySize = OldEntry->ValueKeySize;

                    RtlCopyMemory((PVOID)&(NewEntry->KeyValue),
                                  (PVOID)&(OldEntry->KeyValue),
                                  NewEntry->ValueKeySize);

                    Cacheddatapointer = (PUCHAR) ((ULONG_PTR) &(NewEntry->KeyValue) + OldEntry->ValueKeySize);
                    RtlCopyMemory(Cacheddatapointer, *DataPointer, DataSize);

                    // Trying to catch the BAD guy who writes over our pool.
                    CmpMakeSpecialPoolReadWrite( OldEntry );

                    *ContainingList = (PCM_CACHED_VALUE) CMP_MARK_CELL_CACHED(NewEntry);

                    // Trying to catch the BAD guy who writes over our pool.
                    CmpMakeSpecialPoolReadOnly( NewEntry );

                    //
                    // Free the old entry
                    //
                    ExFreePool(OldEntry);

                } 
            }
        }
    } else {
        if( CmpGetValueData(Hive,&(ValueKey->u.KeyValue),&DataSize,DataPointer,Allocated,CellToRelease) == FALSE ) {
            //
            // insufficient resources; return NULL
            //
            ASSERT( *Allocated == FALSE );
            ASSERT( *DataPointer == NULL );
            return SearchFail;
        }
    }

    return SearchSuccess;
}

