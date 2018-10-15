/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmdelete.c

Abstract:

    This module contains the delete object method (used to delete key
    control blocks  when last handle to a key is closed, and to delete
    keys marked for deletetion when last reference to them goes away.)

--*/

#include    "cmp.h"

extern  BOOLEAN HvShutdownComplete;

VOID
CmpLateUnloadHiveWorker(
    IN PVOID Hive
    );
VOID
CmpDoQueueLateUnloadWorker(IN PCMHIVE CmHive);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpDeleteKeyObject)
#pragma alloc_text(PAGE,CmpLateUnloadHiveWorker)
#pragma alloc_text(PAGE,CmpDoQueueLateUnloadWorker)
#endif


VOID
CmpDeleteKeyObject(
    IN  PVOID   Object
    )
/*++

Routine Description:

    This routine interfaces to the NT Object Manager.  It is invoked when
    the last reference to a particular Key object (or Key Root object)
    is destroyed.

    If the Key object going away holds the last reference to
    the extension it is associated with, that extension is destroyed.

Arguments:

    Object - supplies a pointer to a KeyRoot or Key, thus -> KEY_BODY.

Return Value:

    NONE.

--*/
{
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    PCM_KEY_BODY            KeyBody;
    PCMHIVE                 CmHive = NULL;
    BOOLEAN                 DoUnloadCheck = FALSE;

    CM_PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"CmpDeleteKeyObject: Object = %p\n", Object));

    //
    // HandleClose callback
    //
    if ( CmAreCallbacksRegistered() ) {
        REG_KEY_HANDLE_CLOSE_INFORMATION  KeyHandleCloseInfo;
       
        KeyHandleCloseInfo.Object = Object;

        CmpCallCallBacks(RegNtPreKeyHandleClose,&KeyHandleCloseInfo,TRUE,RegNtPostKeyHandleClose,Object);
    }

    KeyBody = (PCM_KEY_BODY)Object;

    BEGIN_LOCK_CHECKPOINT;

    CmpLockRegistry();

    if (KeyBody->Type==KEY_BODY_TYPE) {
        KeyControlBlock = KeyBody->KeyControlBlock;

        //
        // the keybody should be initialized; when kcb is null, something went wrong
        // between the creation and the dereferenciation of the object
        //
        if( KeyControlBlock != NULL ) {
            //
            // Clean up any outstanding notifies attached to the KeyBody
            //
            CmpFlushNotify(KeyBody,FALSE);

            //
            // Remove our reference to the KeyControlBlock, clean it up, perform any
            // pend-till-final-close operations.
            //
            // NOTE: Delete notification is seen at the parent of the deleted key,
            //       not the deleted key itself.  If any notify was outstanding on
            //       this key, it was cleared away above us.  Only parent/ancestor
            //       keys will see the report.
            //
            //
            // The dereference will free the KeyControlBlock.  If the key was deleted, it
            // has already been removed from the hash table, and relevant notifications
            // posted then as well.  All we are doing is freeing the tombstone.
            //
            // If it was not deleted, we're both cutting the kcb out of
            // the kcb list/tree, AND freeing its storage.
            //

           
            //
            // Replace this with the definition so we avoid dropping and reacquiring the lock
            DelistKeyBodyFromKCB(KeyBody,FALSE);

            //
            // take additional precaution in the case the hive has been unloaded and this is the root
            //
            if( !KeyControlBlock->Delete ) {
                CmHive = (PCMHIVE)CONTAINING_RECORD(KeyControlBlock->KeyHive, CMHIVE, Hive);
                if( IsHiveFrozen(CmHive) ) {
                    //
                    // unload is pending for this hive;
                    //
                    DoUnloadCheck = TRUE;

                }
            }

            CmpDelayDerefKeyControlBlock(KeyControlBlock);

        }
    } else {
        //
        // This must be a predefined handle
        //  some sanity asserts
        //
        KeyControlBlock = KeyBody->KeyControlBlock;

        ASSERT( KeyBody->Type&REG_PREDEF_HANDLE_MASK);
        ASSERT( KeyControlBlock->Flags&KEY_PREDEF_HANDLE );

        if( KeyControlBlock != NULL ) {
            CmHive = (PCMHIVE)CONTAINING_RECORD(KeyControlBlock->KeyHive, CMHIVE, Hive);
            if( IsHiveFrozen(CmHive) ) {
                //
                // unload is pending for this hive; we shouldn't put the kcb in the delay
                // close table
                //
                DoUnloadCheck = TRUE;

            }
            CmpDereferenceKeyControlBlock(KeyControlBlock);
        }

    }

    //
    // if a handle inside a frozen hive has been closed, we may need to unload the hive
    //
    if( DoUnloadCheck == TRUE ) {
        CmpDoQueueLateUnloadWorker(CmHive);
    }

    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

    // 
    // just a notification; disregard the return status
    //
    CmPostCallbackNotification(RegNtPostKeyHandleClose,NULL,STATUS_SUCCESS);
    return;
}

VOID
CmpDoQueueLateUnloadWorker(IN PCMHIVE CmHive)
{
    PWORK_QUEUE_ITEM    WorkItem;

    CM_PAGED_CODE();

    ASSERT( CmHive->RootKcb != NULL );

    //
    // NB: Hive lock has higher precedence; We don't need the kcb lock as we are only checking the refcount
    //
    CmLockHive(CmHive);

    if( (CmHive->RootKcb->RefCount == 1) && (CmHive->UnloadWorkItem == NULL) ) {
        //
        // the only reference on the rookcb is the one that we artificially created
        // queue a work item to late unload the hive
        //
        WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if( InterlockedCompareExchangePointer(&(CmHive->UnloadWorkItem),WorkItem,NULL) == NULL ) {
            ExInitializeWorkItem(CmHive->UnloadWorkItem,
                                 CmpLateUnloadHiveWorker,
                                 CmHive);
            ExQueueWorkItem(CmHive->UnloadWorkItem, DelayedWorkQueue);
        } else {
            ExFreePool(WorkItem);
        }

    }

    CmUnlockHive(CmHive);
}

VOID
CmpLateUnloadHiveWorker(
    IN PVOID Hive
    )
/*++

Routine Description:

    "Late" unloads the hive; If nothing goes badly wrong (i.e. insufficient resources),
    this function should succeed

Arguments:

    CmHive - the frozen hive to be unloaded

Return Value:

    NONE.

--*/
{
    NTSTATUS                Status;
    HCELL_INDEX             Cell;
    PCM_KEY_CONTROL_BLOCK   RootKcb;
    PCMHIVE                 CmHive;

    CM_PAGED_CODE();

    //
    // first, load the registry exclusive
    //
    CmpLockRegistryExclusive();

    //
    // hive is the parameter to this worker; make sure we free the work item
    // allocated by CmpDeleteKeyObject
    //
    CmHive = (PCMHIVE)Hive;

    ASSERT( CmHive->UnloadWorkItem != NULL );
    ExFreePool( CmHive->UnloadWorkItem );

    //
    // if this attempt doesn't succeed, mark that we can try another
    //
    CmHive->UnloadWorkItem = NULL;

    ASSERT( !(CmHive->Hive.HiveFlags & HIVE_IS_UNLOADING) );
    if( CmHive->Frozen == FALSE )  {
        //
        // another thread mounted the exact same hive in the exact same place, hence unfreezing the hive
        // we've done the cleanup part (free the workitem) nothing more to do.
        // or hive is already in process of being unloaded
        //
        ASSERT( CmHive->RootKcb == NULL );
        CmpUnlockRegistry();
        return;
    }
    //
    // this is just about the only possible way the hive can get corrupted in between
    //
    if( HvShutdownComplete == TRUE ) {
        // too late to do anything
        CmpUnlockRegistry();
        return;
    }

    //
    // hive should be frozen, otherwise we wouldn't get here
    //
    ASSERT( CmHive->Frozen == TRUE );

    RootKcb = CmHive->RootKcb;
    //
    // root kcb must be valid and has only our "artificial" refcount on it
    //
    ASSERT( RootKcb != NULL );

    if( RootKcb->RefCount > 1 ) {
        //
        // somebody else must've gotten in between dropping/reacquiring the reglock
        // and opened a handle inside this hive; bad luck, we can't unload
        //
        CmpUnlockRegistry();
        return;
    }

    ASSERT_KCB(RootKcb);

    Cell = RootKcb->KeyCell;
    Status = CmUnloadKey(RootKcb,0,CM_UNLOAD_REG_LOCKED_EX);
    ASSERT( (Status != STATUS_CANNOT_DELETE) && (Status != STATUS_INVALID_PARAMETER) );

    if(NT_SUCCESS(Status)) {
        // CmUnloadKey already released the lock
        CmpLockRegistry();
        CmpDereferenceKeyControlBlock(RootKcb);
    } 
    CmpUnlockRegistry();
}


