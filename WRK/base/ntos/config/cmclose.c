/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmclose.c

Abstract:

    This module contains the close object method.

--*/

#include    "cmp.h"

VOID
CmpDelayedDerefKeys(
                    PLIST_ENTRY DelayedDeref
                    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCloseKeyObject)
#endif

VOID
CmpCloseKeyObject(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )
/*++

Routine Description:

    This routine interfaces to the NT Object Manager.  It is invoked when
    a Key object (or Key Root object) is closed.

    It's function is to do cleanup processing by waking up any notifies
    pending on the handle.  This keeps the key object from hanging around
    forever because a synchronous notify is stuck on it somewhere.

    All other cleanup, in particular, the freeing of storage, will be
    done in CmpDeleteKeyObject.

Arguments:

    Process - ignored

    Object - supplies a pointer to a KeyRoot or Key, thus -> KEY_BODY.

    GrantedAccess, ProcessHandleCount, SystemHandleCount - ignored

Return Value:

    NONE.

--*/
{
    PCM_KEY_BODY        KeyBody;
    PCM_NOTIFY_BLOCK    NotifyBlock;

    CM_PAGED_CODE();

    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"CmpCloseKeyObject: Object = %p\n", Object));

    if( SystemHandleCount > 1 ) {
        //
        // There are still has open handles on this key. Do nothing
        //
        return;
    }

    KeyBody = (PCM_KEY_BODY)Object;

    //
    // Check the type, it will be something else if we are closing a predefined
    // handle key
    //
    if (KeyBody->Type == KEY_BODY_TYPE) {

        if (KeyBody->NotifyBlock == NULL) {
            return;
        }

        CmpLockRegistry();

        //
        // Clean up any outstanding notifies attached to the KeyBody
        //
        CmLockHive((PCMHIVE)(KeyBody->KeyControlBlock->KeyHive));
        if (KeyBody->NotifyBlock != NULL) {
            //
            // Post all PostBlocks waiting on the NotifyBlock
            //
            NotifyBlock = KeyBody->NotifyBlock;
            if (IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {
                LIST_ENTRY          DelayedDeref;
                //
                // we need to follow the rule here the hive lock
                // otherwise we could deadlock down in CmDeleteKeyObject. We don't acquire the kcb lock, 
                // but we make sure that in subsequent places where we get the hive lock we get it before 
                // the kcb lock, ie. we follow the precedence rule below. 
                //
                // NB: the order of these locks is First the hive lock, then the kcb lock
                //
                InitializeListHead(&DelayedDeref);
                CmpPostNotify(NotifyBlock,
                              NULL,
                              0,
                              STATUS_NOTIFY_CLEANUP,
                              FALSE,
                              &DelayedDeref
#if DBG
                              ,(PCMHIVE)(KeyBody->KeyControlBlock->KeyHive)
#endif
                              );
                //
                // finish the job started in CmpPostNotify (i.e. dereference the keybodies
                // we prevented. this may cause some notifyblocks to be freed
                //
                CmpDelayedDerefKeys(&DelayedDeref);
            }
        }
        CmUnlockHive((PCMHIVE)(KeyBody->KeyControlBlock->KeyHive));

        CmpUnlockRegistry();
    }

    return;
}
