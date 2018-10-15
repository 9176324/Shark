/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmapi2.c

Abstract:

    This module contains CM level entry points for the registry,
    particularly those which we don't want to link into tools,
    setup, the boot loader, etc.

--*/

#include "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmDeleteKey)
#endif


NTSTATUS
CmDeleteKey(
    IN PCM_KEY_BODY KeyBody
    )
/*++

Routine Description:

    Delete a registry key, clean up Notify block.

Arguments:

    KeyBody - pointer to key handle object

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    PCM_KEY_NODE            ptarget;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    HCELL_INDEX             Parent;
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    LARGE_INTEGER           TimeStamp;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_CM,"CmDeleteKey\n"));

    CmpLockRegistry();

    //
    // If already marked for deletion, storage is gone, so
    // do nothing and return success.
    //
    KeyControlBlock = KeyBody->KeyControlBlock;

    if( KeyControlBlock->ParentKcb == NULL ) {
        //
        // cannot delete \Registry
        //
        CmpUnlockRegistry();
        return STATUS_CANNOT_DELETE;
    }

    PERFINFO_REG_DELETE_KEY(KeyControlBlock);
    //
    // serialize access to this key. and its parent
    // need to lock them both atomically
    // 
    CmpLockTwoHashEntriesExclusive(KeyControlBlock->ConvKey,KeyControlBlock->ParentKcb->ConvKey);

    if (KeyControlBlock->Delete == TRUE) {
        status = STATUS_SUCCESS;
        goto Exit;
    }
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;
    // 
    // no flush from this point on
    //
    CmpLockHiveFlusherShared((PCMHIVE)KeyControlBlock->KeyHive);

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    ptarget = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive, KeyControlBlock->KeyCell);
    if( ptarget == NULL ) {
        //
        // we couldn't map a view for the bin containing this cell
        //

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit2;
    }

    ASSERT( ptarget->Flags == KeyControlBlock->Flags );

    if ( ((ptarget->SubKeyCounts[Stable] + ptarget->SubKeyCounts[Volatile]) == 0) &&
         ((ptarget->Flags & KEY_NO_DELETE) == 0))
    {
        //
        // Cell is NOT marked NO_DELETE and does NOT have children
        // Send Notification while key still present, if delete fails,
        //   we'll have sent a spurious notify, that doesn't matter
        // Delete the actual storage
        //
        Parent = ptarget->Parent;

        CmpReportNotify(
            KeyControlBlock,
            Hive,
            Cell,
            REG_NOTIFY_CHANGE_NAME
            );

        status = CmpFreeKeyByCell(Hive, Cell, TRUE);

        if (NT_SUCCESS(status)) {
            //
            // post any waiting notifies
            //
            CmpFlushNotifiesOnKeyBodyList(KeyControlBlock,FALSE);

            //
            // Remove kcb out of cache, but do NOT
            // free its storage, CmDelete will do that when
            // the RefCount becomes zero.
            //
            // There are two things that can hold the RefCount non-zero.
            //
            // 1. open handles for this key
            // 2. Fake subKeys that are still in DelayClose.
            //
            // At this point, we have no way of deleting the fake subkeys from cache
            // unless we do a search for the whole cache, which is too expensive.
            // Thus, we decide to either let the fake keys age out of cache or when 
            // someone is doing the lookup for the fake key, then we delete it at that point.
            // See routine CmpCacheLookup in cmparse.c for more details.
            //
            // If the parent has the subkey info or hint cached, free it.
            // Again, registry is locked exclusively, no need to lock KCB.
            //
            CmpCleanUpSubKeyInfo(KeyControlBlock->ParentKcb);
            ptarget = (PCM_KEY_NODE)HvGetCell(Hive, Parent);
            if( ptarget != NULL ) {

                //
                // this should always be true as CmpFreeKeyByCell always marks the parent dirty on success
                //
                KeyControlBlock->ParentKcb->KcbMaxNameLen = (USHORT)ptarget->MaxNameLen;
                // sanity
                ASSERT_CELL_DIRTY(Hive,Parent);
                //
                // update the LastWriteTime on parent and kcb too
                //
                KeQuerySystemTime(&TimeStamp);
                ptarget->LastWriteTime = TimeStamp;
                KeyBody->KeyControlBlock->ParentKcb->KcbLastWriteTime = TimeStamp;
                HvReleaseCell(Hive, Parent);
            }

            KeyControlBlock->Delete = TRUE;
            CmpRemoveKeyControlBlock(KeyControlBlock);
            KeyControlBlock->KeyCell = HCELL_NIL;
        }

    } else {

        status = STATUS_CANNOT_DELETE;

    }

Exit2:
    HvReleaseCell(Hive, Cell);
    CmpUnlockHiveFlusher((PCMHIVE)KeyControlBlock->KeyHive);

Exit:
    CmpUnlockTwoHashEntries(KeyControlBlock->ConvKey,KeyControlBlock->ParentKcb->ConvKey);

    CmpUnlockRegistry();

    // Mark the hive as read only
    CmpMarkAllBinsReadOnly(KeyControlBlock->KeyHive);

    return status;
}
