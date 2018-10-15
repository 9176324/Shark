/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmapi.c

Abstract:

    This module contains CM level entry points for the registry.

--*/

#include "cmp.h"

extern  BOOLEAN     CmpNoWrite;

extern  BOOLEAN CmpProfileLoaded;
extern  BOOLEAN CmpWasSetupBoot;

extern  UNICODE_STRING CmSymbolicLinkValueName;

extern HIVE_LIST_ENTRY CmpMachineHiveList[];

VOID
CmpDereferenceNameControlBlockWithLock(
    PCM_NAME_CONTROL_BLOCK   Ncb
    );

//
// procedures private to this file
//
NTSTATUS
CmpSetValueKeyExisting(
    IN PHHIVE  Hive,
    IN HCELL_INDEX OldChild,
    IN PCM_KEY_VALUE Value,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    );


NTSTATUS
CmpSetValueKeyNew(
    IN PHHIVE  Hive,
    IN PCM_KEY_NODE Parent,
    IN PUNICODE_STRING ValueName,
    IN ULONG Index,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    );

VOID
CmpRemoveKeyHash(
    IN PCM_KEY_HASH KeyHash
    );

PCM_KEY_CONTROL_BLOCK
CmpInsertKeyHash(
    IN PCM_KEY_HASH KeyHash,
    IN BOOLEAN      FakeKey
    );

#if DBG
ULONG
CmpUnloadKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );
#endif

ULONG
CmpCompressKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    );

NTSTATUS
CmpDuplicateKey(
    PHHIVE          Hive,
    HCELL_INDEX     OldKeyCell,
    PHCELL_INDEX    NewKeyCell
    );


VOID
CmpDestroyTemporaryHive(
    PCMHIVE CmHive
    );

VALUE_SEARCH_RETURN_TYPE
CmpCompareNewValueDataAgainstKCBCache(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                        PUNICODE_STRING ValueName,
                                        ULONG Type,
                                        PVOID Data,
                                        ULONG DataSize
                                        );

BOOLEAN
CmpIsHiveAlreadyLoaded( IN HANDLE KeyHandle,
                        IN POBJECT_ATTRIBUTES SourceFile,
                        OUT PCMHIVE *CmHive
                        );

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    );

BOOLEAN
CmpGetHiveName(
    PCMHIVE         CmHive,
    PUNICODE_STRING HiveName
    );

VOID
CmpDoQueueLateUnloadWorker(IN PCMHIVE CmHive);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmDeleteValueKey)
#pragma alloc_text(PAGE,CmEnumerateKey)
#pragma alloc_text(PAGE,CmEnumerateValueKey)
#pragma alloc_text(PAGE,CmFlushKey)
#pragma alloc_text(PAGE,CmQueryKey)
#pragma alloc_text(PAGE,CmQueryValueKey)
#pragma alloc_text(PAGE,CmQueryMultipleValueKey)
#pragma alloc_text(PAGE,CmSetValueKey)
#pragma alloc_text(PAGE,CmpSetValueKeyExisting)
#pragma alloc_text(PAGE,CmpSetValueKeyNew)
#pragma alloc_text(PAGE,CmSetLastWriteTimeKey)
#pragma alloc_text(PAGE,CmSetKeyUserFlags)
#pragma alloc_text(PAGE,CmLoadKey)
#pragma alloc_text(PAGE,CmUnloadKey)
#pragma alloc_text(PAGE,CmUnloadKeyEx)
#pragma alloc_text(PAGE,CmpDoFlushAll)
#pragma alloc_text(PAGE,CmpDoFlushNextHive)
#pragma alloc_text(PAGE,CmReplaceKey)
#pragma alloc_text(PAGE,CmRenameKey)
#pragma alloc_text(PAGE,CmLockKcbForWrite)

#if DBG
#pragma alloc_text(PAGE,CmpUnloadKeyWorker)
#endif

#pragma alloc_text(PAGE,CmMoveKey)
#pragma alloc_text(PAGE,CmpDuplicateKey)
#pragma alloc_text(PAGE,CmCompressKey)
#pragma alloc_text(PAGE,CmpCompressKeyWorker)
#pragma alloc_text(PAGE,CmpCompareNewValueDataAgainstKCBCache)
#pragma alloc_text(PAGE,CmpIsHiveAlreadyLoaded)
#endif

NTSTATUS
CmDeleteValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           ValueName         // RAW
    )
/*++

Routine Description:

    One of the value entries of a registry key may be removed with this call.

    The value entry with ValueName matching ValueName is removed from the key.
    If no such entry exists, an error is returned.

Arguments:

    KeyControlBlock - pointer to kcb for key to operate on

    ValueName - The name of the value to be deleted.  NULL is a legal name.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS        status;
    PCM_KEY_NODE    pcell = NULL;
    PCHILD_LIST     plist;
    PCM_KEY_VALUE   Value = NULL;
    ULONG           targetindex;
    HCELL_INDEX     ChildCell;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    LARGE_INTEGER   systemtime;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmDeleteValueKey\n"));

    status = STATUS_OBJECT_NAME_NOT_FOUND;

    ChildCell = HCELL_NIL;

    CmpLockRegistry();
    //
    // serialize access to this key.
    //
    CmpLockKCBExclusive(KeyControlBlock);

    PERFINFO_REG_DELETE_VALUE(KeyControlBlock, &ValueName);

    //
    // no edits, not even this one, on keys marked for deletion
    //
    if (KeyControlBlock->Delete) {
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();

        // Mark the hive as read only
        CmpMarkAllBinsReadOnly(Hive);

        return STATUS_KEY_DELETED;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    // 
    // no flush from this point on
    //
    CmpLockHiveFlusherShared((PCMHIVE)Hive);
    try {

        pcell = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
        if( pcell == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        // Mark the hive as read only
        CmpMarkAllBinsReadOnly(Hive);

        plist = &(pcell->ValueList);

        if (plist->Count != 0) {

            //
            // The parent has at least one value, map in the list of
            // values and call CmpFindChildInList
            //

            //
            // plist -> the CHILD_LIST structure
            // pchild -> the child node structure being examined
            //

            if( CmpFindNameInList(Hive,
                                  plist,
                                  &ValueName,
                                  &targetindex,
                                  &ChildCell) == FALSE ) {
            
                    // Mark the hive as read only
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
            }

            if (ChildCell != HCELL_NIL) {

                //
                // 1. the desired target was found
                // 2. ChildCell is it's HCELL_INDEX
                // 3. targetaddress points to it
                // 4. targetindex is it's index
                //

                //
                // attempt to mark all relevant cells dirty
                //
                if (!(HvMarkCellDirty(Hive, Cell, FALSE) &&
                      HvMarkCellDirty(Hive, pcell->ValueList.List, FALSE) &&
                      HvMarkCellDirty(Hive, ChildCell, FALSE)))

                {
                    // Mark the hive as read only
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_NO_LOG_SPACE;
                    leave;
                }

                Value = (PCM_KEY_VALUE)HvGetCell(Hive,ChildCell);
                if( Value == NULL ) {
                    //
                    // could not map view inside
                    // this is impossible as we just dirtied the view
                    //
                    ASSERT( FALSE );
                    // Mark the hive as read only
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
                if( !CmpMarkValueDataDirty(Hive,Value) ) {
                    // Mark the hive as read only
                    CmpMarkAllBinsReadOnly(Hive);

                    status = STATUS_NO_LOG_SPACE;
                    leave;
                }

                // sanity
                ASSERT_CELL_DIRTY(Hive,pcell->ValueList.List);
                ASSERT_CELL_DIRTY(Hive,ChildCell);

                if( !NT_SUCCESS(CmpRemoveValueFromList(Hive,targetindex,plist)) ) {
                    //
                    // bail out !
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
                if( CmpFreeValue(Hive, ChildCell) == FALSE ) {
                    //
                    // we couldn't map a view inside above call
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }

                KeQuerySystemTime(&systemtime);
                pcell->LastWriteTime = systemtime;
                // cache it in the kcb too.
                KeyControlBlock->KcbLastWriteTime = systemtime;
                
                // some sanity asserts
                ASSERT( pcell->MaxValueNameLen == KeyControlBlock->KcbMaxValueNameLen );
                ASSERT( pcell->MaxValueDataLen == KeyControlBlock->KcbMaxValueDataLen );
                ASSERT_CELL_DIRTY(Hive,Cell);

                if (pcell->ValueList.Count == 0) {
                    pcell->MaxValueNameLen = 0;
                    pcell->MaxValueDataLen = 0;
                    // update the kcb cache too
                    KeyControlBlock->KcbMaxValueNameLen = 0;
                    KeyControlBlock->KcbMaxValueDataLen = 0;
                }

                //
                // Invalidate and rebuild the cache
                //
                CmpCleanUpKcbValueCache(KeyControlBlock);
                CmpSetUpKcbValueCache(KeyControlBlock,plist->Count,plist->List);
    
                CmpReportNotify(
                        KeyControlBlock,
                        KeyControlBlock->KeyHive,
                        KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_LAST_SET
                        );
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_OBJECT_NAME_NOT_FOUND;
            }
        }
    } finally {
        if(pcell != NULL){
            HvReleaseCell(Hive, Cell);
        }
        if(Value != NULL){
            ASSERT( ChildCell != HCELL_NIL );
            HvReleaseCell(Hive, ChildCell);
        }

        CmpUnlockHiveFlusher((PCMHIVE)Hive);
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    return status;
}


NTSTATUS
CmEnumerateKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
/*++

Routine Description:

    Enumerate sub keys, return data on Index'th entry.

    CmEnumerateKey returns the name of the Index'th sub key of the open
    key specified.  The value STATUS_NO_MORE_ENTRIES will be
    returned if value of Index is larger than the number of sub keys.

    Note that Index is simply a way to select among child keys.  Two calls
    to CmEnumerateKey with the same Index are NOT guaranteed to return
    the same results.

    If KeyInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

Arguments:

    KeyControlBlock - pointer to the KCB that describes the key

    Index - Specifies the (0-based) number of the sub key to be returned.

    KeyInformationClass - Specifies the type of information returned in
        Buffer.  One of the following types:

        KeyBasicInformation - return last write time, title index, and name.
            (see KEY_BASIC_INFORMATION structure)

        KeyNodeInformation - return last write time, title index, name, class.
            (see KEY_NODE_INFORMATION structure)

    KeyInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyInformation in bytes.

    ResultLength - Number of bytes actually written into KeyInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS        status;
    HCELL_INDEX     childcell;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    PCM_KEY_NODE    Node;
    HV_TRACK_CELL_REF       CellRef = {0};

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmEnumerateKey\n"));


    CmpLockRegistry();
    // 
    // this should not be changed from under us
    //
    CmpLockKCBShared(KeyControlBlock);

    PERFINFO_REG_ENUM_KEY(KeyControlBlock, Index);

    if (KeyControlBlock->Delete) {
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    //
    // fetch the child of interest
    //

    Node = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        CmpMarkAllBinsReadOnly(Hive);
        return STATUS_INSUFFICIENT_RESOURCES;

    }
    childcell = CmpFindSubKeyByNumber(Hive, Node, Index);
    
    // release this cell here as we don't need this Node anymore
    HvReleaseCell(Hive, Cell);

    if (childcell == HCELL_NIL) {
        //
        // no such child, clean up and return error
        //
        // we cannot return STATUS_INSUFFICIENT_RESOURCES because of Iop 
        // subsystem which treats INSUFFICIENT RESOURCES as no fatal error
        //
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();

        // Mark the hive as read only
        CmpMarkAllBinsReadOnly(Hive);

        return STATUS_NO_MORE_ENTRIES;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive,childcell);
    if( (Node == NULL) || !HvTrackCellRef(&CellRef,Hive,childcell) ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmpMarkAllBinsReadOnly(Hive);
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

        //
        // call a worker to perform data transfer
        //

        status = CmpQueryKeyData(Hive,
                                 Node,
                                 KeyInformationClass,
                                 KeyInformation,
                                 Length,
                                 ResultLength
                                 );

     } except (EXCEPTION_EXECUTE_HANDLER) {

        HvReleaseFreeCellRefArray(&CellRef);

        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        status = GetExceptionCode();

        // Mark the hive as read only
        CmpMarkAllBinsReadOnly(Hive);

        return status;
    }

    HvReleaseFreeCellRefArray(&CellRef);

    CmpUnlockKCB(KeyControlBlock);

    CmpUnlockRegistry();

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    return status;
}



NTSTATUS
CmEnumerateValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
/*++

Routine Description:

    The value entries of an open key may be enumerated.

    CmEnumerateValueKey returns the name of the Index'th value
    entry of the open key specified by KeyHandle.  The value
    STATUS_NO_MORE_ENTRIES will be returned if value of Index is
    larger than the number of sub keys.

    Note that Index is simply a way to select among value
    entries.  Two calls to NtEnumerateValueKey with the same Index
    are NOT guaranteed to return the same results.

    If KeyValueInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

Arguments:

    KeyControlBlock - pointer to the KCB that describes the key

    Index - Specifies the (0-based) number of the sub key to be returned.

    KeyValueInformationClass - Specifies the type of information returned
    in Buffer. One of the following types:

        KeyValueBasicInformation - return time of last write,
            title index, and name.  (See KEY_VALUE_BASIC_INFORMATION)

        KeyValueFullInformation - return time of last write,
            title index, name, class.  (See KEY_VALUE_FULL_INFORMATION)

    KeyValueInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyValueInformation in bytes.

    ResultLength - Number of bytes actually written into KeyValueInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS            status;
    PHHIVE              Hive;
    PCM_KEY_NODE        Node;
    PCELL_DATA          ChildList;
    PCM_KEY_VALUE       ValueData = NULL;
    BOOLEAN             IndexCached;
    BOOLEAN             ValueCached = FALSE;
    PPCM_CACHED_VALUE   ContainingList = NULL;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;
    HCELL_INDEX         ValueListToRelease = HCELL_NIL;
    VALUE_SEARCH_RETURN_TYPE    SearchValue;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmEnumerateValueKey\n"));


    //
    // lock the parent cell
    //

    CmpLockRegistry();
    // 
    // try shared first; we only upgrade to exclusive when there is a symlink or we need to populate the cache
    //
    CmpLockKCBShared(KeyControlBlock); 

    PERFINFO_REG_ENUM_VALUE(KeyControlBlock, Index);

RetryExclusive:
    if (KeyControlBlock->Delete) {
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Hive = KeyControlBlock->KeyHive;
    Node = (PCM_KEY_NODE)HvGetCell(Hive, KeyControlBlock->KeyCell);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // fetch the child of interest
    //
    //
    // Do it using the cache
    //
    if (Index >= KeyControlBlock->ValueCache.Count) {
        //
        // No such child, clean up and return error.
        //
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return(STATUS_NO_MORE_ENTRIES);
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    if (KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
        //
        // The value list is now set to the KCB for symbolic link,
        // Clean it up and set the value right before we do the query.
        //
        if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
            (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
            //
            // need to upgrade lock to exclusive
            //
            HvReleaseCell(Hive, KeyControlBlock->KeyCell);
            CmpUpgradeKCBLockToExclusive(KeyControlBlock);
            goto RetryExclusive;
        }
        CmpCleanUpKcbValueCache(KeyControlBlock);
        CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);
    }

    SearchValue = CmpGetValueListFromCache(KeyControlBlock,&ChildList, &IndexCached, &ValueListToRelease);
    if( SearchValue == SearchNeedExclusiveLock ) {
        //
        // retry with exclusive lock, since we need to update the cache
        //
        ASSERT( ValueListToRelease == HCELL_NIL );    
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        CmpUpgradeKCBLockToExclusive(KeyControlBlock);
        goto RetryExclusive;
    }

    if (SearchValue != SearchSuccess) {
        ASSERT( ChildList == NULL );

        if( ValueListToRelease != HCELL_NIL ) {
            HvReleaseCell(Hive,ValueListToRelease);
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    SearchValue = CmpGetValueKeyFromCache(KeyControlBlock, ChildList, Index, &ContainingList, &ValueData, IndexCached, &ValueCached,&ValueDataCellToRelease);    
    if( SearchValue == SearchNeedExclusiveLock ) {
        //
        // retry with exclusive lock, since we need to update the cache
        //
        ASSERT( ValueDataCellToRelease == HCELL_NIL );    
    
        if( ValueListToRelease != HCELL_NIL ) {
            HvReleaseCell(Hive,ValueListToRelease);
            ValueListToRelease = HCELL_NIL;
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);

        CmpUpgradeKCBLockToExclusive(KeyControlBlock);
        goto RetryExclusive;
    }
    if (SearchValue != SearchSuccess) {
        ASSERT( ValueData == NULL );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    try {
        //
        // call a worker to perform data transfer; we are touching user-mode address; do it in a try/except
        //
        SearchValue = CmpQueryKeyValueData(KeyControlBlock,
                                  ContainingList,
                                  ValueData,
                                  ValueCached,
                                  KeyValueInformationClass,
                                  KeyValueInformation,
                                  Length,
                                  ResultLength,
                                  &status);
        if( SearchValue == SearchNeedExclusiveLock ) {
            //
            // retry with exclusive lock, since we need to update the cache
            //
            if( ValueListToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,ValueListToRelease);
            }

            HvReleaseCell(Hive, KeyControlBlock->KeyCell);

            if( ValueDataCellToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,ValueDataCellToRelease);
            }
            CmpUpgradeKCBLockToExclusive(KeyControlBlock);
            goto RetryExclusive;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmEnumerateValueKey: code:%08lx\n", GetExceptionCode()));
        status = GetExceptionCode();
    }

Exit:
    if( ValueListToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueListToRelease);
    }
    HvReleaseCell(Hive, KeyControlBlock->KeyCell);
    if( ValueDataCellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueDataCellToRelease);
    }

    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockRegistry();
    return status;
}

NTSTATUS
CmFlushKey(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN BOOLEAN                  RegistryLockOwnedExclusive
    )
/*++

Routine Description:

    Forces changes made to a key to disk.

    CmFlushKey will not return to its caller until any changed data
    associated with the key has been written out.

    WARNING: CmFlushKey will flush the entire registry tree, and thus will
    burn cycles and I/O.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of node to whose sub keys are to be found

Return Value:

    NTSTATUS

--*/
{
    PCMHIVE     CmHive;
    NTSTATUS    status = STATUS_SUCCESS;
    PHHIVE      Hive;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmFlushKey\n"));


    //
    // If writes are not working, lie and say we succeeded, will
    // clean up in a short time.  Only early system init code
    // will ever know the difference.
    //
    if (CmpNoWrite) {
        return STATUS_SUCCESS;
    }

    Hive = Kcb->KeyHive;

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);

    //
    // Don't flush the master hive.  If somebody asks for a flushkey on
    // the master hive, do a CmpDoFlushAll instead.  CmpDoFlushAll flushes
    // every hive except the master hive, which is what they REALLY want.
    //
    if (CmHive == CmpMasterHive) {
        CmpDoFlushAll(FALSE);
    } else {
        DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));

        //
        // no more writes to this hive while we are flushing it
        //
        CmpLockHiveFlusherExclusive(CmHive);
        CmLockHiveViews (CmHive);

        if( HvHiveWillShrink( &(CmHive->Hive) ) ) {
            //
            // we may end up here is when the hive shrinks and we need
            // exclusive access over the registry, as we are going to CcPurge !
            //
            CmUnlockHiveViews (CmHive);
            if( !RegistryLockOwnedExclusive ) {
                CmpUnlockHiveFlusher(CmHive);
                ASSERT_KCB_LOCKED(Kcb);
                CmpUnlockKCB(Kcb);
                CmpUnlockRegistry();
                CmpLockRegistryExclusive();
                CmpLockHiveFlusherExclusive(CmHive);
                CmpLockKCBShared(Kcb);
                if(Kcb->Delete) {
                    CmpUnlockHiveFlusher(CmHive);
                    return STATUS_KEY_DELETED;
                }

                if( CmHive->UseCount != 0) {
                    CmpFixHiveUsageCount(CmHive);
                    ASSERT( CmHive->UseCount == 0 );
                }
            } 
#if DBG
            else {
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE_OR_HIVE_LOADING(CmHive);
            }
#endif
        } else {
            //
            // release the views
            //
            CmUnlockHiveViews(CmHive);
        }

        if (! HvSyncHive(Hive)) {

            status = STATUS_REGISTRY_IO_FAILED;

            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmFlushKey: HvSyncHive failed\n"));
        }
        CmpUnlockHiveFlusher(CmHive);
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    return  status;
}


NTSTATUS
CmQueryKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN KEY_INFORMATION_CLASS    KeyInformationClass,
    IN PVOID                    KeyInformation,
    IN ULONG                    Length,
    IN PULONG                   ResultLength
    )
/*++

Routine Description:

    Data about the class of a key, and the numbers and sizes of its
    children and value entries may be queried with CmQueryKey.

    NOTE: The returned lengths are guaranteed to be at least as
          long as the described values, but may be longer in
          some circumstances.

Arguments:

    KeyControlBlock - pointer to the KCB that describes the key

    KeyInformationClass - Specifies the type of information
        returned in Buffer.  One of the following types:

        KeyBasicInformation - return last write time, title index, and name.
            (See KEY_BASIC_INFORMATION)

        KeyNodeInformation - return last write time, title index, name, class.
            (See KEY_NODE_INFORMATION)

        KeyFullInformation - return all data except for name and security.
            (See KEY_FULL_INFORMATION)

    KeyInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyInformation in bytes.

    ResultLength - Number of bytes actually written into KeyInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    PCM_KEY_NODE    Node = NULL;
    PUNICODE_STRING Name = NULL;
    HV_TRACK_CELL_REF       CellRef = {0};

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryKey\n"));

    CmpLockRegistry();
    // 
    // should not be changing from under us
    //
    CmpLockKCBShared(KeyControlBlock); 

    PERFINFO_REG_QUERY_KEY(KeyControlBlock);

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    try {

        //
        // request for the FULL path of the key
        //
        if( KeyInformationClass == KeyNameInformation ) {
            if (KeyControlBlock->Delete ) {
                //
                // special case: return key deleted status, but still fill the full name of the key.
                //
                status = STATUS_KEY_DELETED;
            } else {
                status = STATUS_SUCCESS;
            }
            
            if( KeyControlBlock->NameBlock ) {
                
                Name = CmpConstructName(KeyControlBlock);
                if (Name == NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    ULONG       requiredlength;
                    ULONG       minimumlength;
                    USHORT      NameLength;
                    LONG        leftlength;
                    PKEY_INFORMATION pbuffer = (PKEY_INFORMATION)KeyInformation;

                    NameLength = Name->Length;

                    requiredlength = FIELD_OFFSET(KEY_NAME_INFORMATION, Name) + NameLength;
                    
                    minimumlength = FIELD_OFFSET(KEY_NAME_INFORMATION, Name);

                    *ResultLength = requiredlength;
                    if (Length < minimumlength) {

                        status = STATUS_BUFFER_TOO_SMALL;

                    } else {
                        //
                        // Fill in the length of the name
                        //
                        pbuffer->KeyNameInformation.NameLength = NameLength;
                        
                        //
                        // Now copy the full name into the user buffer, if enough space
                        //
                        leftlength = Length - minimumlength;
                        requiredlength = NameLength;
                        if (leftlength < (LONG)requiredlength) {
                            requiredlength = leftlength;
                            status = STATUS_BUFFER_OVERFLOW;
                        }

                        //
                        // If not enough space, copy how much we can and return overflow
                        //
                        RtlCopyMemory(
                            &(pbuffer->KeyNameInformation.Name[0]),
                            Name->Buffer,
                            requiredlength
                            );
                    }
                }
            }
        } else if(KeyControlBlock->Delete ) {
            // 
            // key already deleted
            //
            status = STATUS_KEY_DELETED;
        } else if( KeyInformationClass == KeyFlagsInformation ) {
            //
            // we only want to get the user defined flags;
            //
            PKEY_INFORMATION    pbuffer = (PKEY_INFORMATION)KeyInformation;
            ULONG               requiredlength;

            requiredlength = sizeof(KEY_FLAGS_INFORMATION);

            *ResultLength = requiredlength;

            if (Length < requiredlength) {
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                pbuffer->KeyFlagsInformation.UserFlags = (ULONG)((USHORT)KeyControlBlock->Flags >> KEY_USER_FLAGS_SHIFT);
                status = STATUS_SUCCESS;
            }
        } else {
            //
            // call a worker to perform data transfer
            //

            if( KeyInformationClass == KeyCachedInformation ) {
                //
                // call the fast version
                //
                status = CmpQueryKeyDataFromCache(  KeyControlBlock,
                                                    KeyInformationClass,
                                                    KeyInformation,
                                                    Length,
                                                    ResultLength);
            } else {
                //
                // old'n plain slow version
                //
                Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
                if( (Node == NULL) || !HvTrackCellRef(&CellRef,KeyControlBlock->KeyHive, KeyControlBlock->KeyCell) ) {
                    //
                    // we couldn't map a view for the bin containing this cell
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                } else {
                    status = CmpQueryKeyData(KeyControlBlock->KeyHive,
                                             Node,
                                             KeyInformationClass,
                                             KeyInformation,
                                             Length,
                                             ResultLength
                                             );
                }
            }
        }

    } finally {
        HvReleaseFreeCellRefArray(&CellRef);

        if( Name != NULL ) {
            ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        }
        CmpUnlockKCB(KeyControlBlock); 
        CmpUnlockRegistry();
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmQueryValueKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
/*++

Routine Description:

    The ValueName, TitleIndex, Type, and Data for any one of a key's
    value entries may be queried with CmQueryValueKey.

    If KeyValueInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

Arguments:

    KeyControlBlock - pointer to the KCB that describes the key

    ValueName  - The name of the value entry to return data for.

    KeyValueInformationClass - Specifies the type of information
        returned in KeyValueInformation.  One of the following types:

        KeyValueBasicInformation - return time of last write, title
            index, and name.  (See KEY_VALUE_BASIC_INFORMATION)

        KeyValueFullInformation - return time of last write, title
            index, name, class.  (See KEY_VALUE_FULL_INFORMATION)

    KeyValueInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyValueInformation in bytes.

    ResultLength - Number of bytes actually written into KeyValueInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS            status;
    PCM_KEY_VALUE       ValueData = NULL;
    ULONG               Index;
    BOOLEAN             ValueCached = FALSE;
    PPCM_CACHED_VALUE   ContainingList = NULL;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;
    VALUE_SEARCH_RETURN_TYPE SearchValue;
    
    CM_PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryValueKey\n"));

    CmpLockRegistry();
    // 
    // try shared first, we'll only lock exclusive is symlink or cache not already set up
    //
    CmpLockKCBShared(KeyControlBlock); 
    PERFINFO_REG_QUERY_VALUE(KeyControlBlock, &ValueName);

RetryExclusive:

    if(KeyControlBlock->Delete) {
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    if(KeyControlBlock->ExtFlags & CM_KCB_SYM_LINK_FOUND) {
        if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
            (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
            //
            // need to upgrade lock to exclusive
            //
            CmpUpgradeKCBLockToExclusive(KeyControlBlock);
            goto RetryExclusive;
        }
        //
        // The value list is now set to the KCB for symbolic link,
        // Clean it up and set the value right before we do the query.
        //
        CmpCleanUpKcbValueCache(KeyControlBlock);

        {
            PCM_KEY_NODE Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
            if( Node == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ExitNoRelease;
            }

            CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

            HvReleaseCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
        }
    }
    //
    // Find the data
    //

    SearchValue = CmpFindValueByNameFromCache(KeyControlBlock,
                                            &ValueName,
                                            &ContainingList,
                                            &Index,
                                            &ValueData,
                                            &ValueCached,
                                            &ValueDataCellToRelease
                                            );

    if( SearchValue == SearchNeedExclusiveLock ) {
        //
        // retry with exclusive lock, since we need to update the cache
        //
        ASSERT( ValueDataCellToRelease == HCELL_NIL );    
        ASSERT( ValueData == NULL );
        CmpUpgradeKCBLockToExclusive(KeyControlBlock);
        goto RetryExclusive;
    }

    if (SearchValue == SearchSuccess) {
        ASSERT( ValueData != NULL );

        try {
            //
            // call a worker to perform data transfer; we are touching user-mode address; do it in a try/except
            //
            SearchValue = CmpQueryKeyValueData(KeyControlBlock,
                                          ContainingList,
                                          ValueData,
                                          ValueCached,
                                          KeyValueInformationClass,
                                          KeyValueInformation,
                                          Length,
                                          ResultLength,
                                          &status);
            if( SearchValue == SearchNeedExclusiveLock ) {
                //
                // retry with exclusive lock, since we need to update the cache
                //
                if(ValueDataCellToRelease != HCELL_NIL) {
                    HvReleaseCell(KeyControlBlock->KeyHive,ValueDataCellToRelease);
                    ValueDataCellToRelease = HCELL_NIL;
                }
                CmpUpgradeKCBLockToExclusive(KeyControlBlock);
                goto RetryExclusive;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmQueryValueKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }
    } else {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }


    if(ValueDataCellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,ValueDataCellToRelease);
    }

ExitNoRelease:
    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockRegistry();

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmQueryMultipleValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    IN PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    IN OPTIONAL PULONG ResultLength
    )
/*++

Routine Description:

    Multiple values of any key may be queried atomically with
    this api.

Arguments:

    KeyControlBlock - Supplies the key to be queried.

    ValueEntries - Returns an array of KEY_VALUE_ENTRY structures, one for each value.

    EntryCount - Supplies the number of entries in the ValueNames and ValueEntries arrays

    ValueBuffer - Returns the value data for each value.

    BufferLength - Supplies the length of the ValueBuffer array in bytes.
                   Returns the length of the ValueBuffer array that was filled in.

    ResultLength - if present, Returns the length in bytes of the ValueBuffer
                    array required to return the requested values of this key.

Return Value:

    NTSTATUS

--*/

{
    PHHIVE          Hive;
    NTSTATUS        Status;
    ULONG           i;
    UNICODE_STRING  CurrentName;
    HCELL_INDEX     ValueCell = HCELL_NIL;
    PCM_KEY_VALUE   ValueNode;
    ULONG           RequiredLength = 0;
    ULONG           UsedLength = 0;
    ULONG           DataLength;
    BOOLEAN         BufferFull = FALSE;
    BOOLEAN         Small;
    KPROCESSOR_MODE PreviousMode;
    PCM_KEY_NODE    Node;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmQueryMultipleValueKey\n"));


    CmpLockRegistry();
    CmpLockKCBShared(KeyControlBlock); 
    if (KeyControlBlock->Delete) {
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Hive = KeyControlBlock->KeyHive;
    Status = STATUS_SUCCESS;

    Node = (PCM_KEY_NODE)HvGetCell(Hive, KeyControlBlock->KeyCell);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    PreviousMode = KeGetPreviousMode();
    try {
        for (i=0; i < EntryCount; i++) {
            //
            // find the data
            //
            if (PreviousMode == UserMode) {
                ProbeAndReadUnicodeStringEx(&CurrentName,ValueEntries[i].ValueName);
                ProbeForRead(CurrentName.Buffer,CurrentName.Length,sizeof(WCHAR));
            } else {
                CurrentName = *(ValueEntries[i].ValueName);
            }

            PERFINFO_REG_QUERY_MULTIVALUE(KeyControlBlock, &CurrentName); 

            ValueCell = CmpFindValueByName(Hive,
                                           Node,
                                           &CurrentName);
            if (ValueCell != HCELL_NIL) {

                ValueNode = (PCM_KEY_VALUE)HvGetCell(Hive, ValueCell);
                if( ValueNode == NULL ) {
                    //
                    // we couldn't map a view for the bin containing this cell
                    //
                    ValueCell = HCELL_NIL;
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                Small = CmpIsHKeyValueSmall(DataLength, ValueNode->DataLength);

                //
                // Round up UsedLength and RequiredLength to a ULONG boundary
                //
                UsedLength = (UsedLength + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);
                RequiredLength = (RequiredLength + sizeof(ULONG)-1) & ~(sizeof(ULONG)-1);

                //
                // If there is enough room for this data value in the buffer,
                // fill it in now. Otherwise, mark the buffer as full. We must
                // keep iterating through the values in order to determine the
                // RequiredLength.
                //
                if ((UsedLength + DataLength <= *BufferLength) &&
                    (UsedLength + DataLength >= UsedLength) &&
                    (!BufferFull)) {
                    PCELL_DATA  Buffer;
                    BOOLEAN     BufferAllocated;
                    HCELL_INDEX CellToRelease;
                    //
                    // get the data from source, regardless of the size
                    //
                    if( CmpGetValueData(Hive,ValueNode,&DataLength,&Buffer,&BufferAllocated,&CellToRelease) == FALSE ) {
                        //
                        // insufficient resources; return NULL
                        //
                        ASSERT( BufferAllocated == FALSE );
                        ASSERT( Buffer == NULL );
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        break;
                    }

                    RtlCopyMemory((PUCHAR)ValueBuffer + UsedLength,
                                  Buffer,
                                  DataLength);
                    //
                    // cleanup the temporary buffer
                    //
                    if( BufferAllocated == TRUE ) {
                        ExFreePool( Buffer );
                    }
                    //
                    // release the buffer in case we are using hive storage
                    //
                    if( CellToRelease != HCELL_NIL ) {
                        HvReleaseCell(Hive,CellToRelease);
                    }

                    ValueEntries[i].Type = ValueNode->Type;
                    ValueEntries[i].DataLength = DataLength;
                    ValueEntries[i].DataOffset = UsedLength;
                    UsedLength += DataLength;
                } else {
                    BufferFull = TRUE;
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                RequiredLength += DataLength;
                HvReleaseCell(Hive, ValueCell);
                ValueCell = HCELL_NIL;
            } else {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                break;
            }
        }

        if (NT_SUCCESS(Status) ||
            (Status == STATUS_BUFFER_OVERFLOW)) {
            *BufferLength = UsedLength;
            if (ARGUMENT_PRESENT(ResultLength)) {
                *ResultLength = RequiredLength;
            }
        }

    } finally {
        if( ValueCell != HCELL_NIL) {
            HvReleaseCell(Hive, ValueCell);
        }
        HvReleaseCell(Hive, KeyControlBlock->KeyCell);
        
        CmpUnlockKCB(KeyControlBlock);
        CmpUnlockRegistry();
    }

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(Hive);

    return Status;
}

NTSTATUS
CmSetValueKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PUNICODE_STRING ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
/*++

Routine Description:

    A value entry may be created or replaced with CmSetValueKey.

    If a value entry with a Value ID (i.e. name) matching the
    one specified by ValueName exists, it is deleted and replaced
    with the one specified.  If no such value entry exists, a new
    one is created.  NULL is a legal Value ID.  While Value IDs must
    be unique within any given key, the same Value ID may appear
    in many different keys.

Arguments:

    KeyControlBlock - pointer to kcb for the key to operate on

    ValueName - The unique (relative to the containing key) name
        of the value entry.  May be NULL.

    Type - The integer type number of the value entry.

    Data - Pointer to buffer with actual data for the value entry.

    DataSize - Size of Data buffer.


Return Value:

    NTSTATUS

--*/
{
    NTSTATUS        status;
    PCM_KEY_NODE    parent = NULL;
    HCELL_INDEX     oldchild = 0;
    ULONG           count;
    PHHIVE          Hive = NULL;
    HCELL_INDEX     Cell;
    ULONG           StorageType;
    ULONG           TempData;
    BOOLEAN         found;
    PCM_KEY_VALUE   Value = NULL;
    LARGE_INTEGER   systemtime;
    ULONG           mustChange=FALSE;
    ULONG           ChildIndex;
    HCELL_INDEX     ParentToRelease = HCELL_NIL;
    HCELL_INDEX     ChildToRelease = HCELL_NIL;
    BOOLEAN                 FlusherLocked = FALSE;
    VALUE_SEARCH_RETURN_TYPE    SearchType;

    PERFINFO_REG_SET_VALUE_DECL();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetValueKey\n"));

    CmpLockRegistry();
    //
    // try shared first; we'll only lock exclusive if we're going to change the cache
    //
    CmpLockKCBShared(KeyControlBlock);

    ASSERT(sizeof(ULONG) == CM_KEY_VALUE_SMALL);

    PERFINFO_REG_SET_VALUE(KeyControlBlock);

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    while (TRUE) {
TryAgain:
        //
        // Check that we are not being asked to add a value to a key
        // that has been deleted
        //
        if (KeyControlBlock->Delete == TRUE) {
            status = STATUS_KEY_DELETED;
            goto Exit;
        }

        ASSERT_KCB_LOCKED(KeyControlBlock);
        //
        // Check to see if this is a symbolic link node.  If so caller
        // is only allowed to create/change the SymbolicLinkValue
        // value name
        //

        if (KeyControlBlock->Flags & KEY_SYM_LINK &&
            (( (Type != REG_LINK) ) ||
             ValueName == NULL ||
             !RtlEqualUnicodeString(&CmSymbolicLinkValueName, ValueName, TRUE)))
        {
            //
            // Disallow attempts to manipulate any value names under a symbolic link
            // except for the "SymbolicLinkValue" value name or type other than REG_LINK
            //

            // Mark the hive as read only
            CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

            status = STATUS_ACCESS_DENIED;
            goto Exit;
        }

        if( mustChange == FALSE ) {
            //
            // first iteration; look inside the kcb cache
            //
            
            SearchType = CmpCompareNewValueDataAgainstKCBCache(KeyControlBlock,ValueName,Type,Data,DataSize);
            if( SearchType == SearchNeedExclusiveLock ) {
                CmpUpgradeKCBLockToExclusive(KeyControlBlock);
                goto TryAgain;
            } else if( SearchType == SearchSuccess) {
                //
                // the value is in the cache and is the same; make this call a noop
                //
                status = STATUS_SUCCESS;
                goto Exit;
            }

            //
            // To Get here, we must either be changing a value, or setting a new one
            // we need to upgrade kcb lock if not yet exclusive
            //
            if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
                (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
                //
                // need to upgrade lock to exclusive
                //
                CmpUpgradeKCBLockToExclusive(KeyControlBlock);
            }
            mustChange=TRUE;
        } else {
            //
            // second iteration; look inside the hive
            //

            
            //
            // get reference to parent key,
            //
            Hive = KeyControlBlock->KeyHive;
            Cell = KeyControlBlock->KeyCell;
            if( ParentToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,ParentToRelease);
                ParentToRelease = HCELL_NIL;
            }
            parent = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
            if( parent == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
        
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
            ParentToRelease = Cell;
            //
            // try to find an existing value entry by the same name
            //
            count = parent->ValueList.Count;
            found = FALSE;

            if (count > 0) {
                if( CmpFindNameInList(Hive,
                                     &parent->ValueList,
                                     ValueName,
                                     &ChildIndex,
                                     &oldchild) == FALSE ) {
                    //
                    // we couldn't map a view for the bin containing this cell
                    //
        
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Exit;
                }

                if (oldchild != HCELL_NIL) {
                    if( ChildToRelease != HCELL_NIL ) {
                        HvReleaseCell(Hive,ChildToRelease);
                        ChildToRelease = HCELL_NIL;
                    }
                    Value = (PCM_KEY_VALUE)HvGetCell(Hive,oldchild);
                    if( Value == NULL ) {
                        //
                        // could no map view
                        //
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Exit;
                    }
                    ChildToRelease = oldchild;
                    found = TRUE;
                }
            } else {
                //
                // empty list; add it first
                //
                ChildIndex = 0;
            }

            //
            // Performance Hack:
            // If a Set is asking us to set a key to the current value (IE does this a lot)
            // drop it (and, therefore, the last modified time) on the floor, but return success
            // this stops the page from being dirtied, and us having to flush the registry.
            //
            //
            break;
        }

        //
        // We're going through these gyrations so that if someone does come in and try and delete the
        // key we're setting we're safe. Once we know we have to change the key, take the
        // flusher lock and restart
        //
        //
        ASSERT( !FlusherLocked );
        CmpLockHiveFlusherShared((PCMHIVE)KeyControlBlock->KeyHive);
        FlusherLocked = TRUE;

    }// while

    ASSERT( mustChange == TRUE );

    ASSERT_KCB_LOCKED_EXCLUSIVE(KeyControlBlock);

    // It's a different or new value, mark it dirty, since we'll
    // at least set its time stamp

    if (! HvMarkCellDirty(Hive, Cell, FALSE)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit;
    }

    StorageType = HvGetCellType(Cell);

    //
    // stash small data if relevant
    //
    TempData = 0;
    if ((DataSize <= CM_KEY_VALUE_SMALL) &&
        (DataSize > 0))
    {
        try {
            RtlCopyMemory(          // yes, move memory, could be 1 byte
                &TempData,          // at the end of a page.
                Data,
                DataSize
                );
         } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmSetValueKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
            goto Exit;
        }
    }

    if (found) {

        //
        // ----- Existing Value Entry Path -----
        //

        //
        // An existing value entry of the specified name exists,
        // set our data into it.
        //
        status = CmpSetValueKeyExisting(Hive,
                                        oldchild,
                                        Value,
                                        Type,
                                        Data,
                                        DataSize,
                                        StorageType,
                                        TempData);

        PERFINFO_REG_SET_VALUE_EXIST();
    } else {

        //
        // ----- New Value Entry Path -----
        //

        //
        // Either there are no existing value entries, or the one
        // specified is not in the list.  In either case, create and
        // fill a new one, and add it to the list
        //
        status = CmpSetValueKeyNew(Hive,
                                   parent,
                                   ValueName,
                                   ChildIndex,
                                   Type,
                                   Data,
                                   DataSize,
                                   StorageType,
                                   TempData);
        PERFINFO_REG_SET_VALUE_NEW();
    }

    if (NT_SUCCESS(status)) {

        // sanity assert
        ASSERT( parent->MaxValueNameLen == KeyControlBlock->KcbMaxValueNameLen );
        if (parent->MaxValueNameLen < ValueName->Length) {
            parent->MaxValueNameLen = ValueName->Length;
            // update the kcb cache too
            KeyControlBlock->KcbMaxValueNameLen = ValueName->Length;
        }

        //sanity assert
        ASSERT( parent->MaxValueDataLen == KeyControlBlock->KcbMaxValueDataLen );
        if (parent->MaxValueDataLen < DataSize) {
            parent->MaxValueDataLen = DataSize;
            // update the kcb cache too
            KeyControlBlock->KcbMaxValueDataLen = parent->MaxValueDataLen;
        }

        KeQuerySystemTime(&systemtime);
        parent->LastWriteTime = systemtime;
        // update the kcb cache too.
        KeyControlBlock->KcbLastWriteTime = systemtime;
    
        if( found && (CMP_IS_CELL_CACHED(KeyControlBlock->ValueCache.ValueList)) ) {
            //
            // invalidate only the entry we changed.
            //
            PULONG_PTR CachedList = (PULONG_PTR) CMP_GET_CACHED_CELLDATA(KeyControlBlock->ValueCache.ValueList);
            if (CMP_IS_CELL_CACHED(CachedList[ChildIndex])) {

                ExFreePool((PVOID) CMP_GET_CACHED_ADDRESS(CachedList[ChildIndex]));
            }
            CachedList[ChildIndex] = oldchild;

        } else {
            //
            // rebuild ALL KCB cache
            // 
            CmpCleanUpKcbValueCache(KeyControlBlock);
            CmpSetUpKcbValueCache(KeyControlBlock,parent->ValueList.Count,parent->ValueList.List);
        }
        CmpReportNotify(KeyControlBlock,
                        KeyControlBlock->KeyHive,
                        KeyControlBlock->KeyCell,
                        REG_NOTIFY_CHANGE_LAST_SET);
    }

Exit:
    PERFINFO_REG_SET_VALUE_DONE(ValueName);

    if( ParentToRelease != HCELL_NIL && Hive != NULL) {
        HvReleaseCell(Hive,ParentToRelease);
    }
    if( ChildToRelease != HCELL_NIL && Hive != NULL) {
        HvReleaseCell(Hive,ChildToRelease);
    }

    if( FlusherLocked ) {
        CmpUnlockHiveFlusher((PCMHIVE)KeyControlBlock->KeyHive);    
    }

    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockRegistry();
  
    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}


NTSTATUS
CmpSetValueKeyExisting(
    IN PHHIVE  Hive,
    IN HCELL_INDEX OldChild,
    IN PCM_KEY_VALUE Value,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    )
/*++

Routine Description:

    Helper for CmSetValueKey, implements the case where the value entry
    being set already exists.

Arguments:

    Hive - hive of interest

    OldChild - hcell_index of the value entry body to which we are to
                    set new data

    Type - The integer type number of the value entry.

    Data - Pointer to buffer with actual data for the value entry.

    DataSize - Size of Data buffer.

    StorageType - stable or volatile

    TempData - small values are passed here

Return Value:

    STATUS_SUCCESS if it worked, appropriate status code if it did not

Note: 
    
    For new hives format, we have the following cases:

    New Data                Old Data
    --------                --------

1.  small                   small
2.  small                   normal
3.  small                   bigdata
4.  normal                  small
5.  normal                  normal
6.  normal                  bigdata
7.  bigdata                 small
8.  bigdata                 normal
9.  bigdata                 bigdata  



--*/
{
    HCELL_INDEX     DataCell;
    HCELL_INDEX     OldDataCell;
    PCELL_DATA      pdata;
    HCELL_INDEX     NewCell;
    ULONG           OldRealSize;
    USHORT          OldSizeType;    // 0 - small
    USHORT          NewSizeType;    // 1 - normal
                                    // 2 - bigdata
    HANDLE          hSecure = 0;
    NTSTATUS        status = STATUS_SUCCESS;

    ASSERT_HIVE_FLUSHER_LOCKED((PCMHIVE)Hive);

    //
    // value entry by the specified name already exists
    // oldchild is hcell_index of its value entry body
    //  which we will always edit, so mark it dirty
    //
    if (! HvMarkCellDirty(Hive, OldChild, FALSE)) {
        return STATUS_NO_LOG_SPACE;
    }

    if(CmpIsHKeyValueSmall(OldRealSize, Value->DataLength) == TRUE ) {
        //
        // old data was small
        //
        OldSizeType = 0;
    } else if( CmpIsHKeyValueBig(Hive,OldRealSize) == TRUE ) {
        //
        // old data was big
        //
        OldSizeType = 2;
    } else {
        //
        // old data was normal
        //
        OldSizeType = 1;
    }

    if( DataSize <= CM_KEY_VALUE_SMALL ) {
        //
        // new data is small
        //
        NewSizeType = 0;
    } else if( CmpIsHKeyValueBig(Hive,DataSize) == TRUE ) {
        //
        // new data is big
        //
        NewSizeType = 2;
    } else {
        //
        // new data is normal
        //
        NewSizeType = 1;
    }


    //
    // this will handle all cases and will make sure data is marked dirty 
    //
    if( !CmpMarkValueDataDirty(Hive,Value) ) {
        return STATUS_NO_LOG_SPACE;
    }

    //
    // cases 1,2,3
    //
    if( NewSizeType == 0 ) {
        if( ((OldSizeType == 1) && (OldRealSize > 0) ) ||
            (OldSizeType == 2) 
            ) {
            CmpFreeValueData(Hive,Value->Data,OldRealSize);
        }
        
        //
        // write our new small data into value entry body
        //
        Value->DataLength = DataSize + CM_KEY_VALUE_SPECIAL_SIZE;
        Value->Data = TempData;
        Value->Type = Type;

        return STATUS_SUCCESS;
    }
    
    //
    // secure the user buffer so we don't get inconsistencies.
    // ONLY if we are called with a user mode buffer !!!
    //

    if ( (ULONG_PTR)Data <= (ULONG_PTR)MM_HIGHEST_USER_ADDRESS ) {
        hSecure = MmSecureVirtualMemory(Data,DataSize, PAGE_READONLY);
        if (hSecure == 0) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    
    //
    // store it to be freed if the allocation succeeds
    //
    OldDataCell = Value->Data;

    //
    // cases 4,5,6
    //
    if( NewSizeType == 1 ){

        if( (OldSizeType == 1) && (OldRealSize > 0)) { 
            //
            // we already have a cell; see if we can reuse it !
            //
            DataCell = Value->Data;
            ASSERT(DataCell != HCELL_NIL);
            pdata = HvGetCell(Hive, DataCell);
            if( pdata == NULL ) {
                //
                // we couldn't map a view for the bin containing this cell
                //
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
            // release it right here, as the registry is locked exclusively, so we don't care
            HvReleaseCell(Hive, DataCell);

            ASSERT(HvGetCellSize(Hive, pdata) > 0);

            if (DataSize <= (ULONG)(HvGetCellSize(Hive, pdata))) {

                //
                // The existing data cell is big enough to hold the new data.  
                //

                //
                // we'll keep this cell
                //
                NewCell = DataCell;

            } else {
                //
                // grow the existing cell
                //
                NewCell = HvReallocateCell(Hive,DataCell,DataSize);
                if (NewCell == HCELL_NIL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Exit;
                }
            }

        } else {
            //
            // allocate a new cell 
            //
            NewCell = HvAllocateCell(Hive, DataSize, StorageType,(HvGetCellType(OldChild)==StorageType)?OldChild:HCELL_NIL);

            if (NewCell == HCELL_NIL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Exit;
            }
        }
     
        //
        // now we have a cell that can accommodate the data
        //
        pdata = HvGetCell(Hive, NewCell);
        if( pdata == NULL ) {
            //
            // we couldn't map a view for the bin containing this cell
            //
            // this shouldn't happen as we just allocated/ reallocated/ marked dirty this cell
            //
            ASSERT( FALSE );
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }
        // release it right here, as the registry is locked exclusively, so we don't care
        HvReleaseCell(Hive, NewCell);

        //
        // copy the actual data
        //
        RtlCopyMemory(pdata,Data,DataSize);
        Value->Data = NewCell;
        Value->DataLength = DataSize;
        Value->Type = Type;
        
        // sanity
        ASSERT_CELL_DIRTY(Hive,NewCell);

        if( OldSizeType == 2 ) {
            //
            // old data was big; free it
            //
            ASSERT( OldDataCell != NewCell );
            CmpFreeValueData(Hive,OldDataCell,OldRealSize);
        }

        status = STATUS_SUCCESS;
        goto Exit;
    }
    
    //
    // cases 7,8,9
    //
    if( NewSizeType == 2 ) {

        if( OldSizeType == 2 ) { 
            //
            // data was previously big; grow it!
            //
            
            status =CmpSetValueDataExisting(Hive,Data,DataSize,StorageType,OldDataCell);
            if( !NT_SUCCESS(status) ) {
                goto Exit;
            }
            NewCell = OldDataCell;
            
        } else {
            //
            // data was small or normal. 
            // allocate and copy to a new big data cell; 
            // then free the old cell
            //
            status = CmpSetValueDataNew(Hive,Data,DataSize,StorageType,OldChild,&NewCell);
            if( !NT_SUCCESS(status) ) {
                //
                // We have bombed out loading user data, clean up and exit.
                //
                goto Exit;
            }
            
            if( (OldSizeType != 0) && (OldRealSize != 0) ) {
                //
                // there is something to free
                //
                HvFreeCell(Hive, Value->Data);
            }
        }

        Value->DataLength = DataSize;
        Value->Data = NewCell;
        Value->Type = Type;

        // sanity
        ASSERT_CELL_DIRTY(Hive,NewCell);

        status = STATUS_SUCCESS;
        goto Exit;

    }

    //
    // we shouldn't get here
    //
    ASSERT( FALSE );

Exit:
    if( hSecure) {
        MmUnsecureVirtualMemory(hSecure);
    }
    return status;
}

NTSTATUS
CmpSetValueKeyNew(
    IN PHHIVE  Hive,
    IN PCM_KEY_NODE Parent,
    IN PUNICODE_STRING ValueName,
    IN ULONG Index,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize,
    IN ULONG StorageType,
    IN ULONG TempData
    )
/*++

Routine Description:

    Helper for CmSetValueKey, implements the case where the value entry
    being set does not exist.  Will create new value entry and data,
    place in list (which may be created)

Arguments:

    Hive - hive of interest

    Parent - pointer to key node value entry is for

    ValueName - The unique (relative to the containing key) name
        of the value entry.  May be NULL.

    Index - where in the list should this value be inserted

    Type - The integer type number of the value entry.

    Data - Pointer to buffer with actual data for the value entry.

    DataSize - Size of Data buffer.

    StorageType - stable or volatile

    TempData - small data values passed here


Return Value:

    STATUS_SUCCESS if it worked, appropriate status code if it did not

--*/
{
    PCELL_DATA  pvalue;
    HCELL_INDEX ValueCell;
    NTSTATUS    Status;

    ASSERT_HIVE_FLUSHER_LOCKED((PCMHIVE)Hive);

    //
    // Either Count == 0 (no list) or our entry is simply not in
    // the list.  Create a new value entry body, and data.  Add to list.
    // (May create the list.)
    //
    if (Parent->ValueList.Count != 0) {
        ASSERT(Parent->ValueList.List != HCELL_NIL);
        if (! HvMarkCellDirty(Hive, Parent->ValueList.List, FALSE)) {
            return STATUS_NO_LOG_SPACE;
        }
    }

    //
    // allocate the body of the value entry, and the data
    //
    ValueCell = HvAllocateCell(
                    Hive,
                    CmpHKeyValueSize(Hive, ValueName),
                    StorageType,
                    HCELL_NIL
                    );

    if (ValueCell == HCELL_NIL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // map in the body, and fill in its fixed portion
    //
    pvalue = HvGetCell(Hive, ValueCell);
    if( pvalue == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        //
        // normally this shouldn't happen as we just allocated ValueCell
        // i.e. the bin containing ValueCell should be mapped in memory at this point.
        //
        ASSERT( FALSE );
        HvFreeCell(Hive, ValueCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    // release it right here, as the view is PINNED
    HvReleaseCell(Hive, ValueCell);

    // sanity
    ASSERT_CELL_DIRTY(Hive,ValueCell);

    pvalue->u.KeyValue.Signature = CM_KEY_VALUE_SIGNATURE;

    //
    // fill in the variable portions of the new value entry,  name and
    // and data are copied from caller space, could fault.
    //
    try {

        //
        // fill in the name
        //
        pvalue->u.KeyValue.NameLength = CmpCopyName(Hive,
                                                    pvalue->u.KeyValue.Name,
                                                    ValueName);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmSetValueKey: code:%08lx\n", GetExceptionCode()));

        //
        // We have bombed out loading user data, clean up and exit.
        //
        HvFreeCell(Hive, ValueCell);
        return GetExceptionCode();
    }

    if (pvalue->u.KeyValue.NameLength < ValueName->Length) {
        pvalue->u.KeyValue.Flags = VALUE_COMP_NAME;
    } else {
        pvalue->u.KeyValue.Flags = 0;
    }

    //
    // fill in the data
    //
    if (DataSize > CM_KEY_VALUE_SMALL) {
        Status = CmpSetValueDataNew(Hive,Data,DataSize,StorageType,ValueCell,&(pvalue->u.KeyValue.Data));
        if( !NT_SUCCESS(Status) ) {
            //
            // We have bombed out loading user data, clean up and exit.
            //
            HvFreeCell(Hive, ValueCell);
            return Status;
        }

        pvalue->u.KeyValue.DataLength = DataSize;
        // sanity
        ASSERT_CELL_DIRTY(Hive,pvalue->u.KeyValue.Data);

    } else {
        pvalue->u.KeyValue.DataLength = DataSize + CM_KEY_VALUE_SPECIAL_SIZE;
        pvalue->u.KeyValue.Data = TempData;
    }
    pvalue->u.KeyValue.Type = Type;

    if( !NT_SUCCESS(CmpAddValueToList(Hive,ValueCell,Index,StorageType,&(Parent->ValueList)) ) ) {
        // out of space, free all allocated stuff
        // this will free embedded cigdata cell info too (if any)
        CmpFreeValue(Hive,ValueCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CmSetLastWriteTimeKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PLARGE_INTEGER LastWriteTime
    )
/*++

Routine Description:

    The LastWriteTime associated with a key node can be set with
    CmSetLastWriteTimeKey

Arguments:

    KeyControlBlock - pointer to kcb for the key to operate on

    LastWriteTime - new time for key

Return Value:

    NTSTATUS

--*/
{
    PCM_KEY_NODE parent;
    PHHIVE      Hive;
    HCELL_INDEX Cell;
    NTSTATUS    status = STATUS_SUCCESS;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetLastWriteTimeKey\n"));

    CmpLockRegistry();
    //
    // serialize access to this key.
    //
    CmpLockKCBExclusive(KeyControlBlock);

    //
    // Check that we are not being asked to modify a key
    // that has been deleted
    //
    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_KEY_DELETED;
        goto Exit;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    // 
    // no flush from this point on
    //
    CmpLockHiveFlusherShared((PCMHIVE)Hive);

    if (! HvMarkCellDirty(Hive, Cell,FALSE)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit2;
    }

    parent = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( parent == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit2;
    }

    parent->LastWriteTime = *LastWriteTime;
    // update the kcb cache too.
    KeyControlBlock->KcbLastWriteTime = *LastWriteTime;
    
    HvReleaseCell(Hive, Cell);
Exit2:
    CmpUnlockHiveFlusher((PCMHIVE)Hive);
Exit:
    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockRegistry();
    return status;
}

NTSTATUS
CmSetKeyUserFlags(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN ULONG                    UserFlags
    )
/*++

Routine Description:

    Sets the user defined flags for the key; At this point there are only 
    4 bits reserved for user defined flags. kcb and knode must be kept in 
    sync.

Arguments:

    KeyControlBlock - pointer to kcb for the key to operate on

    UserFlags - user defined flags to be set on this key.

Return Value:

    NTSTATUS

--*/
{
    PCM_KEY_NODE    Node;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    LARGE_INTEGER   LastWriteTime;
    NTSTATUS        status = STATUS_SUCCESS;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmSetKeyUserFlags\n"));

    CmpLockRegistry();
    //
    // serialize access to this key.
    //
    CmpLockKCBExclusive(KeyControlBlock);

    //
    // Check that we are not being asked to modify a key
    // that has been deleted
    //
    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_KEY_DELETED;
        goto Exit;
    }

    if( UserFlags & (~((ULONG)KEY_USER_FLAGS_VALID_MASK)) ) {
        //
        // number of user defined flags exceeded; punt
        //
        status = STATUS_INVALID_PARAMETER;
        goto Exit;

    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    // 
    // no flush from this point on
    //
    CmpLockHiveFlusherShared((PCMHIVE)Hive);

    if (! HvMarkCellDirty(Hive, Cell,FALSE)) {
        status = STATUS_NO_LOG_SPACE;
        goto Exit2;
    }

    Node = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( Node == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit2;
    }
    //
    // shift/(pack) the user defined flags and
    // update knode and kcb cache
    //
    // first, erase the old flags
    Node->Flags &= KEY_USER_FLAGS_CLEAR_MASK;
    Node->Flags |= (USHORT)(UserFlags<<KEY_USER_FLAGS_SHIFT);
    // update the kcb cache
    KeyControlBlock->Flags = Node->Flags;

    //
    // we need to update the LstWriteTime as well
    //
    KeQuerySystemTime(&LastWriteTime);
    Node->LastWriteTime = LastWriteTime;
    // update the kcb cache too.
    KeyControlBlock->KcbLastWriteTime = LastWriteTime;

    HvReleaseCell(Hive, Cell);
Exit2:
    CmpUnlockHiveFlusher((PCMHIVE)Hive);
Exit:
    CmpUnlockKCB(KeyControlBlock);
    CmpUnlockRegistry();
    return status;
}

BOOLEAN
CmpIsHiveAlreadyLoaded( IN HANDLE KeyHandle,
                        IN POBJECT_ATTRIBUTES SourceFile,
                        OUT PCMHIVE *CmHive
                        )
/*++

Routine Description:

    Checks if the SourceFile is already loaded in the same spot as KeyHandle.

Arguments:

    KeyHandle - should be the root of a hive. We'll query the name of the primary file
                and compare it against the name of SourceFile

    SourceFile - specifies a file.  while file could be remote,
                that is strongly discouraged.

Return Value:

    TRUE/FALSE
--*/
{
    NTSTATUS                    status;
    PCM_KEY_BODY                KeyBody;
    BOOLEAN                     Result = FALSE; // pessimistic
    
    CM_PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    status = ObReferenceObjectByHandle(KeyHandle,
                                       0,
                                       CmpKeyObjectType,
                                       KernelMode,
                                       (PVOID *)(&KeyBody),
                                       NULL);
    if(!NT_SUCCESS(status)) {
        return FALSE;
    }

	if( KeyBody->KeyControlBlock->Delete ) {
		return FALSE;	
	}
    
    *CmHive = (PCMHIVE)CONTAINING_RECORD(KeyBody->KeyControlBlock->KeyHive, CMHIVE, Hive);

    //
    // should be the root of a hive
    // 
    if( !(KeyBody->KeyControlBlock->Flags & KEY_HIVE_ENTRY) || // not root of a hive
        ((*CmHive)->FileUserName.Buffer == NULL)// no name captured
        ) {
        goto ExitCleanup;
    }
    
    if( RtlCompareUnicodeString(&((*CmHive)->FileUserName),
                                SourceFile->ObjectName,
                                TRUE) == 0 ) {
        //
        // same file; same spot
        //
        Result = TRUE;
        //
        // unfreeze the hive;hive will become just a regular hive from now on
        // it is safe to do this because we hold an extra refcount on the root of the hive
        // as we have specifically opened the root to check if it's already loaded
        //
        if( IsHiveFrozen(*CmHive) ) {
            (*CmHive)->Frozen = FALSE;
            if( (*CmHive)->RootKcb ) {
                CmpDereferenceKeyControlBlockWithLock((*CmHive)->RootKcb,TRUE);
                (*CmHive)->RootKcb = NULL;
            }

        }

    }
    
ExitCleanup:
    ObDereferenceObject((PVOID)KeyBody);
    return Result;
}

#define LogHiveLoad(a,b) //nothing

NTSTATUS
CmLoadKey(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN PCM_KEY_BODY         KeyBody
    )

/*++

Routine Description:

    A hive (file in the format created by NtSaveKey) may be linked
    into the active registry with this call.  UNLIKE NtRestoreKey,
    the file specified to NtLoadKey will become the actual backing
    store of part of the registry (that is, it will NOT be copied.)

    The file may have an associated .log file.

    If the hive file is marked as needing a .log file, and one is
    not present, the call will fail.

    The name specified by SourceFile must be such that ".log" can
    be appended to it to generate the name of the log file.  Thus,
    on FAT file systems, the hive file may not have an extension.

    This call is used by logon to make the user's profile available
    in the registry.  It is not intended for use doing backup,
    restore, etc.  Use NtRestoreKey for that.

    N.B.  This routine assumes that the object attributes for the file
          to be opened have been captured into kernel space so that
          they can safely be passed to the worker thread to open the file
          and do the actual I/O.

Arguments:

    TargetKey - specifies the path to a key to link the hive to.
                path must be of the form "\registry\user\<username>"

    SourceFile - specifies a file.  while file could be remote,
                that is strongly discouraged.

    Flags - specifies any flags that should be used for the load operation.
            The only valid flag is REG_NO_LAZY_FLUSH.

Return Value:

    NTSTATUS

--*/
{
    PCMHIVE                     NewHive = NULL;
    NTSTATUS                    Status;
    BOOLEAN                     Allocate;
    SECURITY_QUALITY_OF_SERVICE ServiceQos;
    SECURITY_CLIENT_CONTEXT     ClientSecurityContext;
    HANDLE                      KeyHandle;
    PCMHIVE                     OtherHive = NULL;
    CM_PARSE_CONTEXT            ParseContext;


    if( KeyBody != NULL ) {
        OtherHive = (PCMHIVE)CONTAINING_RECORD(KeyBody->KeyControlBlock->KeyHive, CMHIVE, Hive);
        if( ! (OtherHive->Flags & CM_CMHIVE_FLAG_UNTRUSTED) ) {
            //
            // deny attempts to join the TRUSTED class of trust
            //
            return STATUS_INVALID_PARAMETER;
        }
    }


    //
    // Obtain the security context here so we can use it
    // later to impersonate the user, which we will do
    // if we cannot access the file as SYSTEM.  This
    // usually occurs if the file is on a remote machine.
    //
    ServiceQos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    ServiceQos.ImpersonationLevel = SecurityImpersonation;
    ServiceQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ServiceQos.EffectiveOnly = TRUE;
    Status = SeCreateClientSecurity(CONTAINING_RECORD(KeGetCurrentThread(),ETHREAD,Tcb),
                                    &ServiceQos,
                                    FALSE,
                                    &ClientSecurityContext);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    RtlZeroMemory(&ParseContext,sizeof(CM_PARSE_CONTEXT));
    ParseContext.CreateOperation = FALSE;
    //
    // we open the root of the hive here. if it already exists,this will prevent it from going
    // away from under us while we are doing the "already loaded" check (due to delay unload logic)
    //
    Status = ObOpenObjectByName(TargetKey,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_READ,
                                (PVOID)&ParseContext,
                                &KeyHandle);
    if(!NT_SUCCESS(Status)) {
        KeyHandle = NULL;
    }

    Allocate = TRUE;
    Status = CmpCmdHiveOpen(    SourceFile,             // FileAttributes
                                &ClientSecurityContext, // ImpersonationContext
                                &Allocate,              // Allocate
                                &NewHive,               // NewHive
								CM_CHECK_REGISTRY_CHECK_CLEAN // CheckFlags
                            );

    SeDeleteClientSecurity( &ClientSecurityContext );


    if (!NT_SUCCESS(Status)) {
        if( KeyHandle != NULL ) {
            PCMHIVE LoadedHive = NULL;
            
            CmpLockRegistryExclusive();

            //
            // check if the same file is loaded in the same spot
            //
            if( CmpIsHiveAlreadyLoaded(KeyHandle,SourceFile,&LoadedHive) ) {
                ASSERT( LoadedHive );
                if( OtherHive != NULL ) {
                    //
                    // unjoin the existing class (if any) and join the new one
                    //
                    CmpUnJoinClassOfTrust(LoadedHive);
                    CmpJoinClassOfTrust(LoadedHive,OtherHive);
                    LoadedHive->Flags |= CM_CMHIVE_FLAG_UNTRUSTED;
                }
                Status = STATUS_SUCCESS;
            }
            CmpUnlockRegistry();
        }
        
        if( KeyHandle != NULL ) {
            ZwClose(KeyHandle);
        }
        return(Status);
    } else {
        //
        // we only need shared
        //
        CmpLockRegistry();
    }

    //
    // only one hive loading at a time (I can imagine getting rid of that, if needed)
    //
    LOCK_HIVE_LOAD();
    //
    // only this thread is allowed to use this hive for now.
    //
    NewHive->Hive.HiveFlags |= HIVE_IS_UNLOADING;
    NewHive->CreatorOwner = KeGetCurrentThread();
    //
    // if this is a NO_LAZY_FLUSH hive, set the appropriate bit.
    //
    if (Flags & REG_NO_LAZY_FLUSH) {
        NewHive->Hive.HiveFlags |= HIVE_NOLAZYFLUSH;
    }
    //
    // mark the hive as untrusted
    //
    NewHive->Flags |= CM_CMHIVE_FLAG_UNTRUSTED;
    if( OtherHive != NULL ) {
        //
        // join the same class of trust with the otherhive
        //
        CmpJoinClassOfTrust(NewHive,OtherHive);
    }
    //
    // We now have a succesfully loaded and initialized CmHive, so we
    // just need to link that into the appropriate spot in the master hive.
    //
    Status = CmpLinkHiveToMaster(TargetKey->ObjectName,
                                 TargetKey->RootDirectory,
                                 NewHive,
                                 Allocate,
                                 TargetKey->SecurityDescriptor);

    if (NT_SUCCESS(Status)) {
        //
        // add new hive to hivelist
        //
        CmpAddToHiveFileList(NewHive);
        //
        // flush the hive right here if just created; this is to avoid situations where 
        // the lazy flusher doesn't get a chance to flush the hive, or it can't (because
        // the hive is a no_lazy_flush hive and it is never explicitly flushed)
        // 
        if( Allocate == TRUE ) {
            CmpLockHiveFlusherExclusive(NewHive);
            HvSyncHive(&(NewHive->Hive));
            CmpUnlockHiveFlusher(NewHive);
        }
        //
        // allow others to use this hive
        //
        NewHive->Hive.HiveFlags &= (~HIVE_IS_UNLOADING);
        NewHive->CreatorOwner = NULL;
        UNLOCK_HIVE_LOAD();

    } else {
        LogHiveLoad((PHHIVE)NewHive,Status);

        CmpLockHiveListExclusive();
        CmpRemoveEntryList(&(NewHive->HiveList));
        CmpUnlockHiveList();

        UNLOCK_HIVE_LOAD();
#if DBG
        NewHive->HiveIsLoading = TRUE;
#endif
        CmpDestroyHiveViewList(NewHive);
        CmpDestroySecurityCache (NewHive);
        CmpDropFileObjectForHive(NewHive);
        CmpUnJoinClassOfTrust(NewHive);

        HvFreeHive((PHHIVE)NewHive);

        //
        // Close the hive files
        //
        CmpCmdHiveClose(NewHive);

        //
        // free the cm level structure
        //
        CmpFreeMutex(NewHive->ViewLock);
#if DBG
        CmpFreeResource(NewHive->FlusherLock);
#endif
        CmpFree(NewHive, sizeof(CMHIVE));
    }

    //
    // We've given user chance to log on, so turn on quota
    //
    if ((CmpProfileLoaded == FALSE) &&
        (CmpWasSetupBoot == FALSE)) {
        CmpProfileLoaded = TRUE;
        CmpSetGlobalQuotaAllowed();
    }

    CmpUnlockRegistry();

    if( KeyHandle != NULL ) {
        ZwClose(KeyHandle);
    }
    return(Status);
}

#if DBG
ULONG
CmpUnloadKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    )
{
    PUNICODE_STRING ConstructedName;

    UNREFERENCED_PARAMETER (Context2);

    if (Current->KeyHive == Context1) {
        ConstructedName = CmpConstructName(Current);

        if (ConstructedName) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"%wZ\n", ConstructedName));
            ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
        }
    }
    return KCB_WORKER_CONTINUE;   // always keep searching
}
#endif

NTSTATUS
CmUnloadKey(
    IN PCM_KEY_CONTROL_BLOCK    Kcb,
    IN ULONG                    Flags,
    IN ULONG                    ControlFlags
    )

/*++

Routine Description:

    Unlinks a hive from its location in the registry, closes its file
    handles, and deallocates all its memory.

    There must be no key control blocks currently referencing the hive
    to be unloaded.

Arguments:

    Hive - Supplies a pointer to the hive control structure for the
           hive to be unloaded

    Kcb - Supplies the key control block

    Flags - REG_FORCE_UNLOAD will first mark open handles as invalid 
            and then unload the hive.

    KcbLocked  - tells if the kcb is locked ex on entry

Return Value:

    NTSTATUS

--*/

{
    PCMHIVE         CmHive;
    LOGICAL         Success;
    PHHIVE          Hive;
    HCELL_INDEX     Cell;
    UNICODE_STRING  EntryName = {0};
    BOOLEAN         Remove = FALSE;
    BOOLEAN         KcbLocked = (ControlFlags & CM_UNLOAD_KCB_LOCKED)?TRUE:FALSE;
    BOOLEAN         RegLockedEx = (ControlFlags & CM_UNLOAD_REG_LOCKED_EX)?TRUE:FALSE;
    

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmUnloadKey\n"));

    // sanity
    ASSERT( (KcbLocked && CmpIsKCBLockedExclusive(Kcb) && CmpIsKCBLockedExclusive(Kcb->ParentKcb)) || (CmpTestRegistryLockExclusive() == TRUE) );

    Hive = Kcb->KeyHive;
    Cell = Kcb->KeyCell;
    //
    // Make sure the cell passed in is the root cell of the hive.
    //
    if((Cell != Hive->BaseBlock->RootCell) || ((PCMHIVE)Hive == CmpMasterHive)) {
        return(STATUS_INVALID_PARAMETER);
    }

    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);

    if( InterlockedCompareExchangePointer(&(CmHive->UnloadWorkItem),(PVOID)1,NULL) != NULL ) {
        //
        // work item has already been queued
        //
        return STATUS_TOO_LATE;
    }
    //
    // mark the hive as HIVE_IS_UNLOADING so no new kcbs are created within this hive.
    //
    Hive->HiveFlags |= HIVE_IS_UNLOADING;

    //
    // Make sure there are no open references to key control blocks
    // for this hive.  If there are none, then we can unload the hive.
    //
    if(Kcb->RefCount != 1) {
        if( Flags == REG_FORCE_UNLOAD ) {
            //
            // This will mark open handles as invalid. However, it may fail, so we
            // account for that possibility here.
            //
            if (CmpSearchForOpenSubKeys(Kcb, SearchAndDeref, RegLockedEx, NULL)) {
                Hive->HiveFlags &= (~HIVE_IS_UNLOADING);
                InterlockedCompareExchangePointer(&(CmHive->UnloadWorkItem),NULL,(PVOID)1);
                return STATUS_CANNOT_DELETE;
            }
        } else {
            Success = (CmpSearchForOpenSubKeys(Kcb, SearchIfExist, RegLockedEx, NULL) == 0);
            Success = Success && (Kcb->RefCount == 1);
        
            if( Success == FALSE) {
#if DBG
                if( KcbLocked ) {
                    ASSERT( Kcb->ParentKcb != NULL );
                    CmpUnlockTwoHashEntries(Kcb->ConvKey,Kcb->ParentKcb->ConvKey);
                }
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"List of keys open against hive unload was attempted on:\n"));
                CmpSearchKeyControlBlockTree(
                    CmpUnloadKeyWorker,
                    Hive,
                    NULL
                    );
                if( KcbLocked ) {
                    ASSERT( Kcb->ParentKcb != NULL );
                    CmpLockTwoHashEntriesExclusive(Kcb->ConvKey,Kcb->ParentKcb->ConvKey);
                }
#endif
                Hive->HiveFlags &= (~HIVE_IS_UNLOADING);
                InterlockedCompareExchangePointer(&(CmHive->UnloadWorkItem),NULL,(PVOID)1);
                return STATUS_CANNOT_DELETE;
            }
            ASSERT( Kcb->RefCount == 1 );
        }
    }

#if DBG
    CmHive->HiveIsLoading = TRUE;
#endif
    //
    // Flush any dirty data to disk. If this fails, too bad.
    //
    CmFlushKey(Kcb,TRUE);

    //
    // get the hive name for later removal
    //
    if( CmpGetHiveName(CmHive, &EntryName) ) {
        Remove = TRUE;
    }
    //
    // Unlink from master hive, remove from list
    //
    Success = CmpDestroyHive(Hive, Cell);

    if (Success) {
        //
        // signal the user event (if any), then do the cleanup (i.e. deref the event
        // and the artificial refcount we set on the root kcb)
        //

        //
        // mark all key_bodies invalid
        //
        if( Flags == REG_FORCE_UNLOAD ) {
            CmpFlushNotifiesOnKeyBodyList(Kcb,TRUE);
        }
        Kcb->Delete = TRUE;
        //
        // If the parent has the subkey info or hint cached, free it.
        //
        CmpCleanUpSubKeyInfo(Kcb->ParentKcb);
        CmpRemoveKeyControlBlock(Kcb);

        //
        // no need to hold the lock anymore
        //

        if( KcbLocked ) {
            ASSERT( Kcb->ParentKcb != NULL );
            CmpUnlockTwoHashEntries(Kcb->ConvKey,Kcb->ParentKcb->ConvKey);
            UNLOCK_HIVE_LOAD();
        }
        CmpUnlockRegistry();

        if( Remove ) {
            //
            // Remove the hive from the HiveFileList
            //
            CmpRemoveFromHiveFileList(&EntryName);
        }
        if( CmHive->UnloadEvent != NULL ) {
            KeSetEvent(CmHive->UnloadEvent,0,FALSE);
            ObDereferenceObject(CmHive->UnloadEvent);
        }

        CmpDestroyHiveViewList(CmHive);
        CmpDestroySecurityCache (CmHive);
        CmpDropFileObjectForHive(CmHive);
        KeEnterCriticalRegion();
        CmpUnJoinClassOfTrust(CmHive);
        KeLeaveCriticalRegion();

        HvFreeHive(Hive);

        //
        // Close the hive files
        //
        CmpCmdHiveClose(CmHive);

        //
        // free the cm level structure
        //
        CmpFreeMutex(CmHive->ViewLock);
#if DBG
        CmpFreeResource(CmHive->FlusherLock);
#endif
        CmpFree(CmHive, sizeof(CMHIVE));

        RtlFreeUnicodeString(&EntryName);
        return(STATUS_SUCCESS);
    } else {
        Hive->HiveFlags &= (~HIVE_IS_UNLOADING);
        InterlockedCompareExchangePointer(&(CmHive->UnloadWorkItem),NULL,(PVOID)1);
#if DBG
        CmHive->HiveIsLoading = FALSE;
#endif
        RtlFreeUnicodeString(&EntryName);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

}

NTSTATUS
CmUnloadKeyEx(
    IN PCM_KEY_CONTROL_BLOCK kcb,
    IN PKEVENT UserEvent
    )

/*++

Routine Description:

    First tries to unlink the hive, by calling the sync version
    
    If the hive cannot be unloaded (there are open handles inside it),
    reference the root of the hive (i.e. kcb) and freeze the hive.

Arguments:

    Kcb - Supplies the key control block

    UserEvent - the event to be signaled after the hive was unloaded
                (only if late - unload is needed)

Return Value:

    STATUS_PENDING - the hive was frozen and it'll be unloaded later

    STATUS_SUCCESS - the hive was successfully sync-unloaded (no need 
                to signal for UserEvent)

    <other> - an error occured, operation failed

--*/
{
    PCMHIVE         CmHive;
    HCELL_INDEX     Cell;    
    NTSTATUS        Status;

    CM_PAGED_CODE();

    Cell = kcb->KeyCell;
    CmHive = (PCMHIVE)CONTAINING_RECORD(kcb->KeyHive, CMHIVE, Hive);

    if( IsHiveFrozen(CmHive) ) {
        //
        // don't let them hurt themselves by calling it twice
        //
        return STATUS_TOO_LATE;
    }
    //
    // first, try out he sync routine; this may or may not unload the hive,
    // but at least will kick kcbs with refcount = 0 out of cache
    //
    Status = CmUnloadKey(kcb,0,CM_UNLOAD_REG_LOCKED_EX);
    if( Status != STATUS_CANNOT_DELETE ) {
        //
        // the hive was either unloaded, or some bad thing happened
        //
        return Status;
    }

    ASSERT( kcb->RefCount > 1 );
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    //
    // Prepare for late-unloading:
    // 1. reference the kcb, to make sure it won't go away without us noticing
    //  (we have the registry locked in exclusive mode, so we don't need to lock the kcbtree
    //
    if (!CmpReferenceKeyControlBlock(kcb)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

	//
	// parse the kcb tree and mark all open kcbs inside this hive and "no delay close"
	//
    CmpSearchForOpenSubKeys(kcb,SearchAndTagNoDelayClose,TRUE,NULL);
	kcb->ExtFlags |= CM_KCB_NO_DELAY_CLOSE;

    //
    // 2. Freeze the hive
    //
    CmHive->RootKcb = kcb;
    CmHive->Frozen = TRUE;
    CmHive->UnloadEvent = UserEvent;

    return STATUS_PENDING;
}

// define in cmworker.c
extern BOOLEAN CmpForceForceFlush;

BOOLEAN
CmpDoFlushAll(
    BOOLEAN ForceFlush
    )
/*++

Routine Description:

    Flush all hives.

    Runs down list of Hives and applies HvSyncHive to them.

    NOTE: Hives which are marked as HV_NOLAZYFLUSH are *NOT* flushed
          by this call.  You must call HvSyncHive explicitly to flush
          a hive marked as HV_NOLAZYFLUSH.

Arguments:

    ForceFlush - used as a contingency plan when a prior exception left 
                some hive in a used state. When set to TRUE, assumes the 
                registry is locked exclusive. It also repairs the broken 
                hives.

               - When FALSE saves only the hives with UseCount == 0.

Return Value:

    NONE

Notes:

    If any of the hives is about to shrink CmpForceForceFlush is set to TRUE, 
    otherwise, it is set to FALSE

--*/
{
    NTSTATUS    Status;
    PLIST_ENTRY p;
    PCMHIVE     h;
    BOOLEAN     Result = TRUE;    
    //
    // If writes are not working, lie and say we succeeded, will
    // clean up in a short time.  Only early system init code
    // will ever know the difference.
    //
    if (CmpNoWrite) {
        return TRUE;
    }
    
    CmpForceForceFlush = FALSE;

    //
    // traverse list of hives, sync each one
    //
    CmpLockHiveListShared();
    p = CmpHiveListHead.Flink;
    while (p != &CmpHiveListHead) {

        h = CONTAINING_RECORD(p, CMHIVE, HiveList);

        if (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH)) {

            //
            // Lock the hive before we flush it.
            // -- since we now allow multiple readers
            // during a flush (a flush is considered a read)
            // we have to force a serialization on the vector table
            //
            CmpLockHiveFlusherExclusive(h);
            
            if( (ForceFlush == TRUE) &&  (h->UseCount != 0) ) {
                //
                // hive was left in an unstable state by a prior exception raised 
                // somewhere inside a CM function.
                //
                ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                CmpFixHiveUsageCount(h);
                ASSERT( h->UseCount == 0 );
            }

            
            if( (ForceFlush == TRUE) || (!HvHiveWillShrink((PHHIVE)h)) ) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpDoFlushAll hive = %p ForceFlush = %lu IsHiveShrinking = %lu BaseLength = %lx StableLength = %lx\n",
                    h,(ULONG)ForceFlush,(ULONG)HvHiveWillShrink((PHHIVE)h),((PHHIVE)h)->BaseBlock->Length,((PHHIVE)h)->Storage[Stable].Length));
                // 
                // no writes while we are flushing the hive
                //
                Status = HvSyncHive((PHHIVE)h);

                if( !NT_SUCCESS( Status ) ) {
                    Result = FALSE;
                }
            } else {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpDoFlushAll: Fail to flush hive %p because is shrinking\n",h));
                Result = FALSE;
                //
                // another unsuccessful attempt to save this hive, because we needed the reglock exclusive
                //
                CmpForceForceFlush = TRUE;
            }

            CmpUnlockHiveFlusher(h);
            //
            // WARNNOTE - the above means that a lazy flush or
            //            or shutdown flush did not work.  we don't
            //            know why.  there is no one to report an error
            //            to, so continue on and hope for the best.
            //            (in theory, worst that can happen is user changes
            //             are lost.)
            //
        }

        p = p->Flink;
    }
    CmpUnlockHiveList();
    
    return Result;
}

extern ULONG    CmpLazyFlushCount;
extern ULONG    CmpLazyFlushHiveCount;

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    )
/*++

Routine Description:

    Flush next hive in list with FlushCount != CmpLazyFlushCount

    Runs in the context of the CmpWorkerThread.

    Runs down list of Hives until it finds the first one with that was not yet flushed
    by the lazy flusher (ie. has its flush count different than the lazy flusher count)

    NOTE: Hives which are marked as HV_NOLAZYFLUSH are *NOT* flushed
          by this call.  You must call HvSyncHive explicitly to flush
          a hive marked as HV_NOLAZYFLUSH.

Arguments:

    ForceFlush - used as a contingency plan when a prior exception left 
                some hive in a used state. When set to TRUE, assumes the 
                registry is locked exclusive. It also repairs the broken 
                hives.

               - When FALSE saves only the hives with UseCount == 0.

Return Value:

    TRUE - if there are more hives to flush
    FALSE - otherwise

Notes:

    If any of the hives is about to shrink CmpForceForceFlush is set to TRUE, 
    otherwise, it is set to FALSE

--*/
{
    NTSTATUS    Status;
    PLIST_ENTRY p;
    PCMHIVE     h;
    BOOLEAN     Result;    
    ULONG       HiveCount = CmpLazyFlushHiveCount;

    *PostWarning = FALSE;
    *DirtyCount = 0;

    //
    // If writes are not working, lie and say we succeeded, will
    // clean up in a short time.  Only early system init code
    // will ever know the difference.
    //
    if (CmpNoWrite) {
        return TRUE;
    }

    //
    // flush at least one hive
    //
    if( !HiveCount ) {
        HiveCount = 1;
    }

    CmpForceForceFlush = FALSE;

    //
    // traverse list of hives, sync each one
    //
    CmpLockHiveListShared();
    p = CmpHiveListHead.Flink;
    while (p != &CmpHiveListHead) {

        h = CONTAINING_RECORD(p, CMHIVE, HiveList);

        if (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH) &&  // lazy flush is notspecifically disabled on this hive
            (h->FlushCount != CmpLazyFlushCount)        // and it was not already flushed during this iteration
            ) {

            Result = TRUE;    
            //
            // Lock the hive before we flush it.
            // -- since we now allow multiple readers
            // during a flush (a flush is considered a read)
            // we have to force a serialization on the vector table
            //
            CmpLockHiveFlusherExclusive(h);
            if( (h->Hive.DirtyCount == 0) || (h->Hive.HiveFlags & HIVE_VOLATILE) ) {
                //
                // if the hive is volatile or has no dirty data, just skip it.
                // silently update the flush count
                //
                h->FlushCount = CmpLazyFlushCount;
            } else {
                if( (ForceFlush == TRUE) &&  (h->UseCount != 0) ) {
                    //
                    // hive was left in an unstable state by a prior exception raised 
                    // somewhere inside a CM function.
                    //
                    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                    CmpFixHiveUsageCount(h);
                    ASSERT( h->UseCount == 0 );
                }

            
                if( (ForceFlush == TRUE) || (!HvHiveWillShrink((PHHIVE)h)) ) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpDoFlushAll hive = %p ForceFlush = %lu IsHiveShrinking = %lu BaseLength = %lx StableLength = %lx\n",
                        h,(ULONG)ForceFlush,(ULONG)HvHiveWillShrink((PHHIVE)h),((PHHIVE)h)->BaseBlock->Length,((PHHIVE)h)->Storage[Stable].Length));
                    // 
                    // no writes while we are flushing the hive
                    //
                    Status = HvSyncHive((PHHIVE)h);

                    if( !NT_SUCCESS( Status ) ) {
                        *PostWarning = TRUE;
                        Result = FALSE;
                    }
                } else {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpDoFlushAll: Fail to flush hive %p because is shrinking\n",h));
                    Result = FALSE;
                    //
                    // another unsuccessful attempt to save this hive, because we needed the reglock exclusive
                    //
                    CmpForceForceFlush = TRUE;
                }
                if( Result == TRUE ) {
                    //
                    // we have successfully flushed current hive hive
                    //
                    h->FlushCount = CmpLazyFlushCount;
                    HiveCount--;
                    if( (!HiveCount) && (!ForceFlush)) { // since we have it all locked exclusive; flush the remainder of the list
                        //
                        // skip to the next one and break out of the loop, so we can detect whether the last one was flushed out
                        //
                        CmpUnlockHiveFlusher(h);
                        p = p->Flink;
                        break;
                    }
                } else {
                    //
                    // do not update flush count for this one as we want to attempt to flush it at next iteration
                    //
                }
            }
            CmpUnlockHiveFlusher(h);

        } else if(  (h->Hive.DirtyCount != 0) &&                // hive has dirty data
                    (!(h->Hive.HiveFlags & HIVE_VOLATILE)) &&   // is not volatile
                    (!(h->Hive.HiveFlags & HIVE_NOLAZYFLUSH))){  // and lazy flush is enabled
            //
            // count dirty count for this hive; we'll need to fire another lazy flusher
            // to take this into account, even if we made it to the end of the list
            //
            // sanity; this has already been flushed
            ASSERT( h->FlushCount == CmpLazyFlushCount );
            *DirtyCount += h->Hive.DirtyCount;
        }
        //
        // for the ones that we cannot do the unload check at last deref
        //
        if( IsHiveFrozen(h) ) {
            CmpDoQueueLateUnloadWorker(h);
        }
        p = p->Flink;
    }
    if( p == &CmpHiveListHead ) {
        //
        // we have flushed out everything; caller must update globalflush count
        //
        Result = FALSE;
    } else {
        Result = TRUE;
    }
    CmpUnlockHiveList();

    return Result;
}


NTSTATUS
CmRenameKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN UNICODE_STRING           NewKeyName,         // RAW
    IN KPROCESSOR_MODE          PreviousMode
    )
/*++

Routine Description:

    Changes the name of the key to the given one.

    What needs to be done:
    
    1. Allocate a cell big enough to accommodate new knode 
    2. make a duplicate of the index in subkeylist of kcb's parent
    3. replace parent's subkeylist with the duplicate
    4. add new subkey to parent
    5. remove old subkey
    6. free storage.

Arguments:

    KeyControlBlock - pointer to kcb for key to operate on

    NewKeyName - The new name to be given to this key

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                Status;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    PCM_KEY_NODE            Node;
    PCM_KEY_NODE            ParentNode;
    ULONG                   NodeSize;
    HCELL_INDEX             NewKeyCell = HCELL_NIL;
    HSTORAGE_TYPE           StorageType;
    HCELL_INDEX             OldSubKeyList = HCELL_NIL;
    PCM_KEY_NODE            NewKeyNode;
    PCM_KEY_INDEX           Index;
    ULONG                   i;
    LARGE_INTEGER           TimeStamp;
    ULONG                   NameLength;
    PCM_NAME_CONTROL_BLOCK  OldNcb = NULL;
    ULONG                   ConvKey;
    BOOLEAN                 NameUpCase;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmRenameKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    //
    // no edits, on keys marked for deletion
    //
    if (KeyControlBlock->Delete) {
        return STATUS_KEY_DELETED;
    }

    //
    // see if the newName is not already a subkey of parentKcb
    //
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;
    StorageType = HvGetCellType(Cell);

    //
    // OBS. we could have worked with the kcb tree instead, but if this is not 
    // going to work, we are in trouble anyway, so it's better to find out soon
    //
    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
        //
        // cannot map view
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // release the cell right here, as the registry is locked exclusively, so we don't care
    HvReleaseCell(Hive, Cell);

    //
    // cannot rename the root of a hive; or anything in the master hive !!!
    //
    if((Hive == &CmpMasterHive->Hive) || (KeyControlBlock->ParentKcb == NULL) || (KeyControlBlock->ParentKcb->KeyHive == &CmpMasterHive->Hive) ) {
        return STATUS_ACCESS_DENIED;
    }

    //
    // Exhaustive access check is done here. We already checked we have DELETE on this KCB
    // We need to check we have KEY_CREATE_SUB_KEY on the parent
    // and DELETE on the entire subtree underneath us.
    //
    Status = CmpCheckKeyAccess( KeyControlBlock->ParentKcb->KeyHive,
                                KeyControlBlock->ParentKcb->KeyCell,
                                PreviousMode,
                                KEY_CREATE_SUB_KEY);
    if( NT_SUCCESS(Status) ) {
        Status = CmpDoAccessCheckOnSubtree( Hive,
                                            Cell,
                                            PreviousMode,
                                            DELETE,
                                            TRUE);
    }

    if( !NT_SUCCESS(Status) ) {
        return Status;
    }

    ParentNode = (PCM_KEY_NODE)HvGetCell(Hive,Node->Parent);
    if( ParentNode == NULL ) {
        //
        // cannot map view
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    // release the cell right here, as the registry is locked exclusively, so we don't care
    HvReleaseCell(Hive, Node->Parent);

    try {
        if( CmpFindSubKeyByName(Hive,ParentNode,&NewKeyName) != HCELL_NIL ) {
            //
            // a subkey with this name already exists
            //
            return STATUS_CANNOT_DELETE;
        }

        //
        // since we are in try-except, compute the new node size
        //
        NodeSize = CmpHKeyNodeSize(Hive, &NewKeyName);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        return GetExceptionCode();
    }    
    
    //
    // 1. Allocate the new knode cell and copy the data from the old one, updating 
    // the name. 
    
    //
    // mark the parent dirty, as we will modify its SubkeyLists
    //
    if(!HvMarkCellDirty(Hive, Node->Parent,FALSE)) {
        return STATUS_NO_LOG_SPACE;
    }

    //
    // mark the index dirty as we are going to free it on success
    //
    if ( !CmpMarkIndexDirty(Hive, Node->Parent, Cell) ) {
        return STATUS_NO_LOG_SPACE;
    }
    //
    // mark key_node as dirty as we are going to free it if we succeed
    //
    if(!HvMarkCellDirty(Hive, Cell,FALSE)) {
        return STATUS_NO_LOG_SPACE;
    }
   
    OldSubKeyList = ParentNode->SubKeyLists[StorageType];       
    if( (OldSubKeyList == HCELL_NIL) || (!HvMarkCellDirty(Hive, OldSubKeyList,FALSE)) ) {
        return STATUS_NO_LOG_SPACE;
    }
    Index = (PCM_KEY_INDEX)HvGetCell(Hive,OldSubKeyList);
    if( Index == NULL ) {
        //
        // we just marked this dirty
        //
        ASSERT( FALSE );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    // release the cell right here, as the registry is locked exclusively, so we don't care
    HvReleaseCell(Hive, OldSubKeyList);

    //
    // mark all the index cells dirty
    //
    if( Index->Signature == CM_KEY_INDEX_ROOT ) {
        //
        // it's a root
        //
        for(i=0;i<Index->Count;i++) {
            // common sense
            ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
            if(!HvMarkCellDirty(Hive, Index->List[i],FALSE)) {
                return STATUS_NO_LOG_SPACE;
            }
        }

    } 


    NewKeyCell = HvAllocateCell(
                    Hive,
                    NodeSize,
                    StorageType,
                    Cell // in the same vicinity
                    );
    if( NewKeyCell == HCELL_NIL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,NewKeyCell);
    if( NewKeyNode == NULL ) {
        //
        // cannot map view; this shouldn't happen as we just allocated 
        // this cell (i.e. it should be dirty/pinned into memory)
        //
        ASSERT( FALSE );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    // release the cell right here, as the registry is locked exclusively, so we don't care
    HvReleaseCell(Hive, NewKeyCell);

    //
    // copy old keynode info onto the new cell and update the name
    //
    // first everything BUT the name
    RtlCopyMemory(NewKeyNode,Node,FIELD_OFFSET(CM_KEY_NODE, Name));
    // second, the new name
    try {
        NewKeyNode->NameLength = CmpCopyName(   Hive,
                                                NewKeyNode->Name,
                                                &NewKeyName);
        NameLength = NewKeyName.Length;

        if (NewKeyNode->NameLength < NameLength ) {
            NewKeyNode->Flags |= KEY_COMP_NAME;
        } else {
            NewKeyNode->Flags &= ~KEY_COMP_NAME;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
        goto ErrorExit;
    }    
    // third, the timestamp
    KeQuerySystemTime(&TimeStamp);
    NewKeyNode->LastWriteTime = TimeStamp;
    
    //
    // at this point we have the new key_node all built up.
    //

    //
    // 2.3. Make a duplicate of the parent's subkeylist and replace the original
    //
    ParentNode->SubKeyLists[StorageType] = CmpDuplicateIndex(Hive,OldSubKeyList,StorageType);
    if( ParentNode->SubKeyLists[StorageType] == HCELL_NIL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // 4. Add new subkey to the parent. This will take care of index 
    // grow and rebalance problems. 
    // Note: the index is at this point a duplicate, so if we fail, we still have the 
    // original one handy to recover
    //
    if( !CmpAddSubKey(Hive,Node->Parent,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // 5. remove old subkey;
    //
    if( !CmpRemoveSubKey(Hive,Node->Parent,Cell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // update the NCB in the kcb; at the end of this function, the kcbs underneath this 
    // will eventually get rehashed
    //
    OldNcb = KeyControlBlock->NameBlock;
    try {
        KeyControlBlock->NameBlock = CmpGetNameControlBlock (&NewKeyName,&NameUpCase);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmRenameKey: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();
        goto ErrorExit;
    }    

    if( KeyControlBlock->NameBlock == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // 5'. update the parent on each and every son.
    //
    if( !CmpUpdateParentForEachSon(Hive,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // 6. At this point we have it all done. We just need to free the old index and key_cell
    //
    
    //
    // free old index
    //
    Index = (PCM_KEY_INDEX)HvGetCell(Hive,OldSubKeyList);
    if( Index == NULL ) {
        //
        // we just marked this dirty
        //
        ASSERT( FALSE );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }
    // release the cell right here, as the registry is locked exclusively, so we don't care
    HvReleaseCell(Hive, OldSubKeyList);

    if( Index->Signature == CM_KEY_INDEX_ROOT ) {
        //
        // it's a root
        //
        for(i=0;i<Index->Count;i++) {
            // common sense
            ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
            HvFreeCell(Hive, Index->List[i]);
        }

    } else {
        //
        // should be a leaf 
        //
        ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
               (Index->Signature == CM_KEY_FAST_LEAF)   ||
               (Index->Signature == CM_KEY_HASH_LEAF)
               );
        ASSERT(Index->Count != 0);
    }
    HvFreeCell(Hive, OldSubKeyList);
    
    //
    // free old cell
    //
    HvFreeCell(Hive,Cell);

    //
    // update the node KeyCell for this kcb and the timestamp on the kcb;
    //
    KeyControlBlock->KeyCell = NewKeyCell;
    KeyControlBlock->KcbLastWriteTime = TimeStamp;

    //
    // and one last "little" thing: update parent's maxnamelen and reset parents cache
    //
    CmpCleanUpSubKeyInfo (KeyControlBlock->ParentKcb);

    if (ParentNode->MaxNameLen < NameLength) {
        ParentNode->MaxNameLen = NameLength;
        KeyControlBlock->ParentKcb->KcbMaxNameLen = (USHORT)NameLength;
    }
    
    //
    // rehash this kcb
    //
    ConvKey = CmpComputeKcbConvKey(KeyControlBlock);
    if( ConvKey != KeyControlBlock->ConvKey ) {
        //
        // rehash the kcb by removing it from hash, and then inserting it
        // again with th new ConvKey
        //
        CmpRemoveKeyHash(&(KeyControlBlock->KeyHash));
        KeyControlBlock->ConvKey = ConvKey;
        CmpInsertKeyHash(&(KeyControlBlock->KeyHash),FALSE);
    }

    //
    // Additional work: take care of the kcb subtree; this cannot fail, punt
    //
    CmpSearchForOpenSubKeys(KeyControlBlock,SearchAndRehash,TRUE,NULL);

    //
    // last, dereference the OldNcb for this kcb
    //
    ASSERT( OldNcb != NULL );
    CmpDereferenceNameControlBlockWithLock(OldNcb);
#if defined(_WIN64)
    //
    // set up the UpCase name flag in the kcb; free old name if present
    //
    if( (KeyControlBlock->RealKeyName != NULL) && (KeyControlBlock->RealKeyName != CMP_KCB_REAL_NAME_UPCASE) ) {
        ExFreePoolWithTag(KeyControlBlock->RealKeyName, CM_NAME_TAG);
    }
    if( NameUpCase == TRUE ) {
        KeyControlBlock->RealKeyName = CMP_KCB_REAL_NAME_UPCASE;
    } else {
        KeyControlBlock->RealKeyName = NULL;
    }
#endif

    return STATUS_SUCCESS;

ErrorExit:
    if( OldSubKeyList != HCELL_NIL ) {
        //
        // we have attempted (maybe even succeeded) to duplicate parent's index)
        //
        if( ParentNode->SubKeyLists[StorageType] != HCELL_NIL ) {
            //
            // we need to free this as it is a duplicate
            //
            Index = (PCM_KEY_INDEX)HvGetCell(Hive,ParentNode->SubKeyLists[StorageType]);
            if( Index == NULL ) {
                //
                // could not map view;this shouldn't happen as we just allocated this cell
                //
                ASSERT( FALSE );
            } else {
                // release the cell right here, as the registry is locked exclusively, so we don't care
                HvReleaseCell(Hive, ParentNode->SubKeyLists[StorageType]);

                if( Index->Signature == CM_KEY_INDEX_ROOT ) {
                    //
                    // it's a root
                    //
                    for(i=0;i<Index->Count;i++) {
                        // common sense
                        ASSERT( (Index->List[i] != 0) && (Index->List[i] != HCELL_NIL) );
                        HvFreeCell(Hive, Index->List[i]);
                    }

                } else {
                    //
                    // should be a leaf 
                    //
                    ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
                           (Index->Signature == CM_KEY_FAST_LEAF)   ||
                           (Index->Signature == CM_KEY_HASH_LEAF)
                           );
                    ASSERT(Index->Count != 0);
                }
                HvFreeCell(Hive, ParentNode->SubKeyLists[StorageType]);
            }

        }
        //
        // restore the parent's index
        //
        ParentNode->SubKeyLists[StorageType] = OldSubKeyList;
    }
    ASSERT( NewKeyCell != HCELL_NIL );
    HvFreeCell(Hive,NewKeyCell);
    
    if( OldNcb != NULL ) {
        KeyControlBlock->NameBlock = OldNcb;
    }
    
    return Status;
}

NTSTATUS
CmMoveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock
    )
/*++

Routine Description:

    Moves all the cells related to this kcb above the specified fileoffset.

    What needs to be done:
    
    1. mark all data that we are going to touch dirty
    2. Duplicate the key_node (and values and all cells involved)
    3. Update the parent for all children
    4. replace the new Key_cell in the parent's subkeylist
    5. Update the kcb and the kcb cache
    6. remove old subkey

WARNING:
    after 3 we cannot fail anymore. if we do, we'll leak cells.

Arguments:

    KeyControlBlock - pointer to kcb for key to operate on

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                Status;
    PHHIVE                  Hive;
    HCELL_INDEX             OldKeyCell;
    HCELL_INDEX             NewKeyCell = HCELL_NIL;
    HCELL_INDEX             ParentKeyCell;
    HSTORAGE_TYPE           StorageType;
    PCM_KEY_NODE            OldKeyNode;
    PCM_KEY_NODE            ParentKeyNode;
    PCM_KEY_NODE            NewKeyNode;
    PCM_KEY_INDEX           ParentIndex;
    PCM_KEY_INDEX           OldIndex;
    ULONG                   i,j;
    HCELL_INDEX             LeafCell;
    PCM_KEY_INDEX           Leaf;
    PCM_KEY_FAST_INDEX      FastIndex;
    PHCELL_INDEX            ParentIndexLocation = NULL;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmMoveKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    //
    // no edits, on keys marked for deletion
    //
    if (KeyControlBlock->Delete) {
        return STATUS_KEY_DELETED;
    }

    //
    // see if the newName is not already a subkey of parentKcb
    //
    Hive = KeyControlBlock->KeyHive;
    OldKeyCell = KeyControlBlock->KeyCell;
    StorageType = HvGetCellType(OldKeyCell);

    if( StorageType != Stable ) {
        //
        // nop the volatiles
        //
        return STATUS_SUCCESS;
    }

    if( OldKeyCell ==  Hive->BaseBlock->RootCell ) {
        //
        // this works only for stable keys.
        //
        return STATUS_INVALID_PARAMETER;
    }

    //
    // 1. mark all data that we are going to touch dirty
    //
    // parent's index, as we will replace the key node cell in it
    // we only search in the Stable storage. It is supposed to be there
    //
    OldKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,OldKeyCell);
    if( OldKeyNode == NULL ) {
        //
        // cannot map view
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (! CmpMarkKeyDirty(Hive, OldKeyCell
#if DBG
		,FALSE
#endif //DBG
		)) {
        HvReleaseCell(Hive, OldKeyCell);
        return STATUS_NO_LOG_SPACE;
    }
    // release the cell right here, as the registry is locked exclusively, and the key_cell is marked as dirty
    HvReleaseCell(Hive, OldKeyCell);

	if( OldKeyNode->Flags & KEY_SYM_LINK ) {
		//
		// we do not compact links
		//
		return STATUS_INVALID_PARAMETER;
	}
	if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		//
		// mark the index dirty
		//
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if( OldIndex == NULL ) {
			//
			// we couldn't map the bin containing this cell
			//
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		HvReleaseCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if( !HvMarkCellDirty(Hive, OldKeyNode->SubKeyLists[Stable],FALSE) ) {
			return STATUS_NO_LOG_SPACE;
		}

		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				if( !HvMarkCellDirty(Hive, OldIndex->List[i],FALSE) ) {
					return STATUS_NO_LOG_SPACE;
				}
			}
		} 
	}

    ParentKeyCell = OldKeyNode->Parent;
    //
    // now in the parent's spot
    //
    ParentKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,ParentKeyCell);
    if( ParentKeyNode == NULL ) {
        //
        // cannot map view
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if( !HvMarkCellDirty(Hive, ParentKeyCell,FALSE) ) {
        HvReleaseCell(Hive, ParentKeyCell);
        return STATUS_NO_LOG_SPACE;
    }
    // release the cell right here, as the registry is locked exclusively, so we don't care
    // Key_cell is marked dirty to keep the parent knode mapped
    HvReleaseCell(Hive, ParentKeyCell);

    ParentIndex = (PCM_KEY_INDEX)HvGetCell(Hive, ParentKeyNode->SubKeyLists[Stable]);
    if( ParentIndex == NULL ) {
        //
        // we couldn't map the bin containing this cell
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    HvReleaseCell(Hive, ParentKeyNode->SubKeyLists[Stable]);

    if(ParentIndex->Signature == CM_KEY_INDEX_ROOT) {

        //
        // step through root, till we find the right leaf
        //
        for (i = 0; i < ParentIndex->Count; i++) {
            LeafCell = ParentIndex->List[i];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                //
                // we couldn't map the bin containing this cell
                //
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            HvReleaseCell(Hive, LeafCell);

            if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                 (Leaf->Signature == CM_KEY_HASH_LEAF)
                ) {
                FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
                for(j=0;j<FastIndex->Count;j++) {
                    if( FastIndex->List[j].Cell == OldKeyCell ) {
                        //
                        // found it! remember the locations we want to update later and break the loop
                        //
                        if( !HvMarkCellDirty(Hive, LeafCell,FALSE) ) {
					        return STATUS_NO_LOG_SPACE;
                        }
                        ParentIndexLocation = &(FastIndex->List[j].Cell);
                        break;
                    }
                }
                if( ParentIndexLocation != NULL ) {
                    break;
                }
            } else {
                for(j=0;j<Leaf->Count;j++) {
                    if( Leaf->List[j] == OldKeyCell ) {
                        //
                        // found it! remember the locations we want to update later and break the loop
                        //
                        if( !HvMarkCellDirty(Hive, LeafCell,FALSE) ) {
					        return STATUS_NO_LOG_SPACE;
                        }
                        ParentIndexLocation = &(Leaf->List[j]);
                        break;
                    }
                }
                if( ParentIndexLocation != NULL ) {
                    break;
                }
            }
        }
    } else if ( (ParentIndex->Signature == CM_KEY_FAST_LEAF) ||
                (ParentIndex->Signature == CM_KEY_HASH_LEAF)
        ) {
        FastIndex = (PCM_KEY_FAST_INDEX)ParentIndex;
        for(j=0;j<FastIndex->Count;j++) {
            if( FastIndex->List[j].Cell == OldKeyCell ) {
                //
                // found it! remember the locations we want to update later and break the loop
                //
                if( !HvMarkCellDirty(Hive, ParentKeyNode->SubKeyLists[Stable],FALSE) ) {
			        return STATUS_NO_LOG_SPACE;
                }
                ParentIndexLocation = &(FastIndex->List[j].Cell);
                break;
            }
        }
    } else {
        for(j=0;j<ParentIndex->Count;j++) {
            if( ParentIndex->List[j] == OldKeyCell ) {
                //
                // found it! remember the locations we want to update later and break the loop
                //
                if( !HvMarkCellDirty(Hive, ParentKeyNode->SubKeyLists[Stable],FALSE) ) {
			        return STATUS_NO_LOG_SPACE;
                }
                ParentIndexLocation = &(ParentIndex->List[j]);
                break;
            }
        }
    }

    // we should've find it !!!
    ASSERT( ParentIndexLocation != NULL );

    // 
    // 2. Duplicate the key_node (and values and all cells involved)
    //
    Status = CmpDuplicateKey(Hive,OldKeyCell,&NewKeyCell);
    if( !NT_SUCCESS(Status) ) {
        return Status;
    }

    // sanity
    ASSERT( (NewKeyCell != HCELL_NIL) && (StorageType == (HSTORAGE_TYPE)HvGetCellType(NewKeyCell)));

    //
    // 3. update the parent on each and every son.
    //
    if( !CmpUpdateParentForEachSon(Hive,NewKeyCell) ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

    //
    // 4. replace the new Key_cell in the parent's subkeylist
    // From now on, WE CANNOT fails. we have everything marked dirty
    // we just update some fields. no resources required !
    // If we fail to free some cells, too bad, we'll leak some cells.
    //
    *ParentIndexLocation = NewKeyCell;

    //
    // 5. Update the kcb and the kcb cache
    //
    CmpCleanUpSubKeyInfo(KeyControlBlock->ParentKcb);
    KeyControlBlock->KeyCell = NewKeyCell;
    CmpRebuildKcbCache(KeyControlBlock);

    //
    // 6. remove old subkey
    //
    // First the Index; it's already marked dirty (i.e. PINNED)
    //
	if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		ASSERT( OldIndex != NULL );
		HvReleaseCell(Hive, OldKeyNode->SubKeyLists[Stable]);
		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				HvFreeCell(Hive, OldIndex->List[i]);
			}
		} 
		HvFreeCell(Hive,OldKeyNode->SubKeyLists[Stable]);
	}

	OldKeyNode->SubKeyCounts[Stable] = 0;
    OldKeyNode->SubKeyCounts[Volatile] = 0;

    CmpFreeKeyByCell(Hive,OldKeyCell,FALSE);

    return STATUS_SUCCESS;

ErrorExit:
    //
    // we need to free the new knode allocated
    //
    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,NewKeyCell);
    // must be dirty
    ASSERT( NewKeyNode != NULL );
	HvReleaseCell(Hive, NewKeyCell);
	if( NewKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		OldIndex = (PCM_KEY_INDEX)HvGetCell(Hive, NewKeyNode->SubKeyLists[Stable]);
		ASSERT( OldIndex != NULL );
		HvReleaseCell(Hive, NewKeyNode->SubKeyLists[Stable]);
		if(OldIndex->Signature == CM_KEY_INDEX_ROOT) {
			for (i = 0; i < OldIndex->Count; i++) {
				HvFreeCell(Hive, OldIndex->List[i]);
			}
		} 
		HvFreeCell(Hive,NewKeyNode->SubKeyLists[Stable]);
	}
    NewKeyNode->SubKeyCounts[Stable] = 0;
    NewKeyNode->SubKeyCounts[Volatile] = 0;

    CmpFreeKeyByCell(Hive,NewKeyCell,FALSE);
    return Status;

}

NTSTATUS
CmpDuplicateKey(
    PHHIVE          Hive,
    HCELL_INDEX     OldKeyCell,
    PHCELL_INDEX    NewKeyCell
    )
/*++

Routine Description:

    Makes an exact clone of OldKeyCell key_node in the 
    space above AboveFileOffset.
    Operates on Stable storage ONLY!!!

Arguments:


Return Value:

    NTSTATUS

--*/
{
    PCM_KEY_NODE			OldKeyNode;
    PCM_KEY_NODE			NewKeyNode;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( HvGetCellType(OldKeyCell) == Stable );
    
    OldKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,OldKeyCell);
    if( OldKeyNode == NULL ) {
        //
        // cannot map view
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // since the registry is locked exclusively here, we don't need to lock/release cells 
    // while copying the trees; So, we just set the release routines to NULL and restore after
    // the copy is complete; this saves some pain
    //
    TargetReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

    *NewKeyCell = CmpCopyKeyPartial(Hive,OldKeyCell,Hive,OldKeyNode->Parent,TRUE);
    Hive->ReleaseCellRoutine  = TargetReleaseCellRoutine;

    if( *NewKeyCell == HCELL_NIL ) {
	    HvReleaseCell(Hive, OldKeyCell);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NewKeyNode = (PCM_KEY_NODE)HvGetCell(Hive,*NewKeyCell);
    if( NewKeyNode == NULL ) {
        //
        // cannot map view
        //
	    HvReleaseCell(Hive, OldKeyCell);
        CmpFreeKeyByCell(Hive,*NewKeyCell,FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // now we have the key_cell duplicated. Values and security has also been taken care of
    // Go ahead and duplicate the Index.
    //
    if( OldKeyNode->SubKeyLists[Stable] != HCELL_NIL ) {
		NewKeyNode->SubKeyLists[Stable] = CmpDuplicateIndex(Hive,OldKeyNode->SubKeyLists[Stable],Stable);
		if( NewKeyNode->SubKeyLists[Stable] == HCELL_NIL ) {
			HvReleaseCell(Hive, OldKeyCell);
			CmpFreeKeyByCell(Hive,*NewKeyCell,FALSE);
			HvReleaseCell(Hive, *NewKeyCell);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
	} else {
		ASSERT( OldKeyNode->SubKeyCounts[Stable] == 0 );
		NewKeyNode->SubKeyLists[Stable] = HCELL_NIL;
	}
    NewKeyNode->SubKeyCounts[Stable] = OldKeyNode->SubKeyCounts[Stable];
    NewKeyNode->SubKeyLists[Volatile] = OldKeyNode->SubKeyLists[Volatile];
    NewKeyNode->SubKeyCounts[Volatile] = OldKeyNode->SubKeyCounts[Volatile];

	HvReleaseCell(Hive, *NewKeyCell);
    HvReleaseCell(Hive, OldKeyCell);
    return STATUS_SUCCESS;

}

ULONG
CmpCompressKeyWorker(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    )
{
	PLIST_ENTRY				pListHead;
	PCM_KCB_REMAP_BLOCK		kcbRemapBlock;
	//PLIST_ENTRY             AnchorAddr;

    if (Current->KeyHive == Context1) {
		
		pListHead = (PLIST_ENTRY)Context2;
		ASSERT( pListHead );

		kcbRemapBlock = (PCM_KCB_REMAP_BLOCK)ExAllocatePool(PagedPool, sizeof(CM_KCB_REMAP_BLOCK));
		if( kcbRemapBlock == NULL ) {
			return KCB_WORKER_ERROR;
		}
		kcbRemapBlock->KeyControlBlock = Current;
		kcbRemapBlock->NewCellIndex = HCELL_NIL;
		kcbRemapBlock->OldCellIndex = Current->KeyCell;
		kcbRemapBlock->ValueCount = 0;
		kcbRemapBlock->ValueList = HCELL_NIL;
        InsertTailList(pListHead,&(kcbRemapBlock->RemapList));

    }
    return KCB_WORKER_CONTINUE;   // always keep searching
}

NTSTATUS
CmCompressKey(
    IN PHHIVE Hive
    )
/*++

Routine Description:

	Compresses the kcb, by means of simulating an "in-place" SaveKey

    What needs to be done:

	1. iterate through the kcb tree and make a list of all the kcbs 
	that need to be changed (their keycell will change during the process)
	2. iterate through the cache and compute an array of security cells.
	We'll need it to map security cells into the new hive.
	3. Save the hive into a temporary hive, preserving
	the volatile info in keynodes and updating the cell mappings.
	4. Update the cache by adding volatile security cells from the old hive.
	5. Dump temporary (compressed) hive over to the old file.
	6. Switch hive data from the compressed one to the existing one and update
	the kcb KeyCell and security mapping
	7. Invalidate the map and drop paged bins.
	8. Free storage for the new hive (OK if we fail)

Arguments:

    Hive - Hive to operate on

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                Status = STATUS_SUCCESS;
    HCELL_INDEX             KeyCell;
    PCMHIVE                 CmHive;
    PCM_KCB_REMAP_BLOCK     RemapBlock;
    PCMHIVE                 NewHive = NULL;
    HCELL_INDEX             LinkCell;
    PCM_KEY_NODE            LinkNode;
    PCM_KNODE_REMAP_BLOCK   KnodeRemapBlock;
    ULONG                   OldLength;

    
	PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmCompressKey\n"));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    if( HvAutoCompressCheck(Hive) == FALSE ) {
        return STATUS_SUCCESS;
    }

    KeyCell = Hive->BaseBlock->RootCell;
    CmHive = CONTAINING_RECORD(Hive, CMHIVE, Hive);
    //
    // Make sure the cell passed in is the root cell of the hive.
    //
    if ( CmHive == CmpMasterHive ) {
        return STATUS_INVALID_PARAMETER;
    }

	//
	// 0. Get the cells we need to relink the compressed hive
	//
	LinkNode = (PCM_KEY_NODE)HvGetCell(Hive,KeyCell);
	if( LinkNode == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
	}
	LinkCell = LinkNode->Parent;
	HvReleaseCell(Hive,KeyCell);
	LinkNode = (PCM_KEY_NODE)HvGetCell((PHHIVE)CmpMasterHive,LinkCell);

	// master storage is paged pool
	ASSERT(LinkNode != NULL);
	HvReleaseCell((PHHIVE)CmpMasterHive,LinkCell);

    OldLength = Hive->BaseBlock->Length;

	//
	//	1. iterate through the kcb tree and make a list of all the kcbs 
	//	that need to be changed (their keycell will change during the process)
	//
	ASSERT( IsListEmpty(&(CmHive->KcbConvertListHead)) );

	//
	// this will kick all kcb with refcount == 0 out of cache, so we can use 
	// CmpSearchKeyControlBlockTree for recording referenced kcbs
	//
	CmpCleanUpKCBCacheTable(NULL,TRUE);
    if( !CmpSearchKeyControlBlockTree(CmpCompressKeyWorker,(PVOID)Hive,(PVOID)(&(CmHive->KcbConvertListHead))) ) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	//
	// 2. iterate through the cache and compute an array of security cells.
	// We'll need it to map security cells into the new hive.
	//
	if( !CmpBuildSecurityCellMappingArray(CmHive) ) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	//
	// 3. Save the hive into a temporary hive , preserving
	// the volatile info in keynodes and updating the cell mappings.
	//
	Status = CmpShiftHiveFreeBins(CmHive,&NewHive);
	if( !NT_SUCCESS(Status) ) {
		goto Exit;
	}

	//
	// 5. Dump temporary (compressed) hive over to the old file.
	//
	Status = CmpOverwriteHive(CmHive,NewHive,LinkCell);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }


	//
	// From this point on, we WILL NOT FAIL!
	//

	//
	// get the root node and link it into the master storage
	//
	LinkNode->ChildHiveReference.KeyCell = NewHive->Hive.BaseBlock->RootCell;

	//
	// 6. Switch hive data from the compressed one to the existing one and update
	// the kcb KeyCell and security mapping
	// This should better NOT fail!!! If it does, we are doomed, as we have partial
	// data => bugcheck
	//
	CmpSwitchStorageAndRebuildMappings(CmHive,NewHive);

	
	//
	// 7. Invalidate the map and drop paged bins. If system hive, check for the hysteresis callback.
	//
    HvpDropAllPagedBins(&(CmHive->Hive));
    if( OldLength < CmHive->Hive.BaseBlock->Length ) {
        CmpUpdateSystemHiveHysteresis(&(CmHive->Hive),CmHive->Hive.BaseBlock->Length,OldLength);
    }


Exit:

	//
	// 8. Free storage for the new hive (OK if we fail)
	//
	if( NewHive != NULL ) { 
		CmpDestroyTemporaryHive(NewHive);	
	}

	if( CmHive->CellRemapArray != NULL ) {
		ExFreePool(CmHive->CellRemapArray);
		CmHive->CellRemapArray = NULL;
	}
	//
	// remove all remap blocks and free them
	//
	while (IsListEmpty(&(CmHive->KcbConvertListHead)) == FALSE) {
        RemapBlock = (PCM_KCB_REMAP_BLOCK)RemoveHeadList(&(CmHive->KcbConvertListHead));
        RemapBlock = CONTAINING_RECORD(
                        RemapBlock,
                        CM_KCB_REMAP_BLOCK,
                        RemapList
                        );
		ExFreePool(RemapBlock);
	}
	while (IsListEmpty(&(CmHive->KnodeConvertListHead)) == FALSE) {
        KnodeRemapBlock = (PCM_KNODE_REMAP_BLOCK)RemoveHeadList(&(CmHive->KnodeConvertListHead));
        KnodeRemapBlock = CONTAINING_RECORD(
                            KnodeRemapBlock,
                            CM_KNODE_REMAP_BLOCK,
                            RemapList
                        );
		ExFreePool(KnodeRemapBlock);
	}

	return Status;
}

NTSTATUS
CmLockKcbForWrite(PCM_KEY_CONTROL_BLOCK KeyControlBlock)
/*++

Routine Description:

    Tags the kcb as being read-only and no-delay-close

Arguments:

    KeyControlBlock

--*/
{
    CM_PAGED_CODE();

    CmpLockKCBExclusive(KeyControlBlock);

    ASSERT_KCB(KeyControlBlock);
    if( KeyControlBlock->Delete ) {
        CmpUnlockKCB(KeyControlBlock);
        return STATUS_KEY_DELETED;
    }
    //
    // sanity check in case we are called twice
    //
    ASSERT( ((KeyControlBlock->ExtFlags&CM_KCB_READ_ONLY_KEY) && (KeyControlBlock->ExtFlags&CM_KCB_NO_DELAY_CLOSE)) ||
            (!(KeyControlBlock->ExtFlags&CM_KCB_READ_ONLY_KEY))
        );

    //
    // tag the kcb as read-only; also make it no-delay close so it can revert to the normal state after all handles are closed.
    //
    KeyControlBlock->ExtFlags |= (CM_KCB_READ_ONLY_KEY|CM_KCB_NO_DELAY_CLOSE);

    //
    // add an artificial refcount on this kcb. This will keep the kcb (and the read only flag set in memory for as long as the system is up)
    //
    InterlockedIncrement( (PLONG)&KeyControlBlock->RefCount );

    CmpUnlockKCB(KeyControlBlock);

    return STATUS_SUCCESS;
}

VALUE_SEARCH_RETURN_TYPE
CmpCompareNewValueDataAgainstKCBCache(  PCM_KEY_CONTROL_BLOCK KeyControlBlock,
                                        PUNICODE_STRING ValueName,
                                        ULONG Type,
                                        PVOID Data,
                                        ULONG DataSize
                                        )

/*++

Routine Description:

    Most of the SetValue calls are noops (i.e. they are setting the same 
    value name to the same value data). By comparing against the data already 
    in the kcb cache (i.e. faulted in) we can save page faults.


Arguments:

    KeyControlBlock - pointer to kcb for the key to operate on

    ValueName - The unique (relative to the containing key) name
        of the value entry.  May be NULL.

    Type - The integer type number of the value entry.

    Data - Pointer to buffer with actual data for the value entry.

    DataSize - Size of Data buffer.


Return Value:

    TRUE - same value with the same data exist in the cache.

--*/
{
    PCM_KEY_VALUE       Value;
    ULONG               Index;
    BOOLEAN             ValueCached;
    PPCM_CACHED_VALUE   ContainingList;
    HCELL_INDEX         ValueDataCellToRelease = HCELL_NIL;
    PUCHAR              datapointer = NULL;
    BOOLEAN             BufferAllocated = FALSE;
    HCELL_INDEX         CellToRelease = HCELL_NIL;
    ULONG               compareSize;
    ULONG               realsize;
    BOOLEAN             small;
    VALUE_SEARCH_RETURN_TYPE SearchValue = SearchFail;

    CM_PAGED_CODE();

    ASSERT_KCB_LOCKED(KeyControlBlock);

    if( KeyControlBlock->Flags & KEY_SYM_LINK ) {
        //
        // need to rebuild the value cache, so we could runt the same code
        //
        PCM_KEY_NODE    Node;
        if( (CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE) &&
            (CmpTryConvertKCBLockSharedToExclusive(KeyControlBlock) == FALSE) ) {
            //
            // need to upgrade lock to exclusive
            //
            return SearchNeedExclusiveLock;
        }

        Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);

        if( Node == NULL ) {
            //
            // we couldn't map the bin containing this cell
            //
            return SearchFail;
        }

        CmpCleanUpKcbValueCache(KeyControlBlock);
        CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);

        HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);
    }

    SearchValue = CmpFindValueByNameFromCache(  KeyControlBlock,
                                                ValueName,
                                                &ContainingList,
                                                &Index,
                                                &Value,
                                                &ValueCached,
                                                &ValueDataCellToRelease
                                                );

    if( SearchValue == SearchNeedExclusiveLock ) {
        //
        // retry with exclusive lock, since we need to update the cache
        //
        ASSERT( CmpIsKCBLockedExclusive(KeyControlBlock) == FALSE );    
        ASSERT( ValueDataCellToRelease == HCELL_NIL );    
        ASSERT( Value == NULL );
        goto Exit;
    }

    if (SearchValue == SearchSuccess) {
        ASSERT( Value != NULL );
        if( (Type == Value->Type) && (DataSize == (Value->DataLength & ~CM_KEY_VALUE_SPECIAL_SIZE)) ) {
        
            small = CmpIsHKeyValueSmall(realsize, Value->DataLength);
            if (small == TRUE) {
                datapointer = (PUCHAR)(&(Value->Data));
            } else {
                SearchValue = CmpGetValueDataFromCache(KeyControlBlock, ContainingList,(PCELL_DATA)Value, 
                                                ValueCached,&datapointer,&BufferAllocated,&CellToRelease);
                if(SearchValue != SearchSuccess  ) {
                    ASSERT( datapointer == NULL );
                    ASSERT( BufferAllocated == FALSE );
                    goto Exit;
                }
            }
            //
            // compare data
            //
            if (DataSize > 0) {

                try {
                    compareSize = (ULONG)RtlCompareMemory ((PVOID)datapointer,Data,(DataSize & ~CM_KEY_VALUE_SPECIAL_SIZE));
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    SearchValue = SearchFail;
                    goto Exit;
                }

            } else {
                compareSize = 0;
            }

            if (compareSize != DataSize) {
                SearchValue = SearchFail;
            }

        } else {
            SearchValue = SearchFail;
        }
    }

Exit:

    if(ValueDataCellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,ValueDataCellToRelease);
    }
    if( BufferAllocated == TRUE ) {
        ExFreePool(datapointer);
    }
    if(CellToRelease != HCELL_NIL) {
        HvReleaseCell(KeyControlBlock->KeyHive,CellToRelease);
    }
    
    return SearchValue;
}

