/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmhook.c

Abstract:

    Provides routines for implementing callbacks into the registry code.
    Callbacks are to be used by the virus filter drivers and cluster 
    replication engine.

--*/
#include "cmp.h"

#define CM_MAX_CALLBACKS    100

typedef struct _CM_CALLBACK_CONTEXT_BLOCK {
    LARGE_INTEGER               Cookie;             // to identify a specific callback for deregistration purposes
    LIST_ENTRY                  ThreadListHead;     // Active threads inside this callback
    EX_PUSH_LOCK                ThreadListLock;     // synchronize access to the above
    PVOID                       CallerContext;
} CM_CALLBACK_CONTEXT_BLOCK, *PCM_CALLBACK_CONTEXT_BLOCK;

typedef struct _CM_ACTIVE_NOTIFY_THREAD {
    LIST_ENTRY  ThreadList;
    PETHREAD    Thread;
} CM_ACTIVE_NOTIFY_THREAD, *PCM_ACTIVE_NOTIFY_THREAD;

#define CmpLockContext(Context)    ExAcquirePushLockExclusive(&((Context)->ThreadListLock))
#define CmpUnlockContext(Context)  ExReleasePushLock(&((Context)->ThreadListLock))


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

ULONG       CmpCallBackCount = 0;
EX_CALLBACK CmpCallBackVector[CM_MAX_CALLBACKS] = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

VOID
CmpInitCallback(VOID);

BOOLEAN
CmpCheckRecursionAndRecordThreadInfo(
                                     PCM_CALLBACK_CONTEXT_BLOCK         CallbackBlock,
                                     PCM_ACTIVE_NOTIFY_THREAD   ActiveThreadInfo
                                     );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmRegisterCallback)
#pragma alloc_text(PAGE,CmUnRegisterCallback)
#pragma alloc_text(PAGE,CmpInitCallback)
#pragma alloc_text(PAGE,CmpCallCallBacks)
#pragma alloc_text(PAGE,CmpCheckRecursionAndRecordThreadInfo)
#endif


NTSTATUS
CmRegisterCallback(__in     PEX_CALLBACK_FUNCTION Function,
                   __in_opt PVOID                 Context,
                   __out    PLARGE_INTEGER    Cookie
                    )
/*++

Routine Description:

    Registers a new callback.

Arguments:



Return Value:


--*/
{
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;
    ULONG                       i;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;

    PAGED_CODE();
    
    CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExAllocatePoolWithTag (PagedPool,
                                                                    sizeof (CM_CALLBACK_CONTEXT_BLOCK),
                                                                    'bcMC');
    if( CmCallbackContext == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RoutineBlock = ExAllocateCallBack (Function,CmCallbackContext);
    if( RoutineBlock == NULL ) {
        ExFreePool(CmCallbackContext);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // init the context
    //
    KeQuerySystemTime(&(CmCallbackContext->Cookie));
    *Cookie = CmCallbackContext->Cookie;
    InitializeListHead(&(CmCallbackContext->ThreadListHead));   
	ExInitializePushLock(&(CmCallbackContext->ThreadListLock));
    CmCallbackContext->CallerContext = Context;

    //
    // find a spot where we could add this callback
    //
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        if( ExCompareExchangeCallBack (&CmpCallBackVector[i],RoutineBlock,NULL) ) {
            InterlockedExchangeAdd ((PLONG) &CmpCallBackCount, 1);
            return STATUS_SUCCESS;
        }
    }
    
    //
    // no more callbacks
    //
    ExFreePool(CmCallbackContext);
    ExFreeCallBack(RoutineBlock);
    return STATUS_INSUFFICIENT_RESOURCES;
}


NTSTATUS
CmUnRegisterCallback(__in LARGE_INTEGER  Cookie)
/*++

Routine Description:

    Unregisters a callback.

Arguments:

Return Value:


--*/
{
    ULONG                       i;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;

    PAGED_CODE();
    
    //
    // Search for this cookie
    //
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        RoutineBlock = ExReferenceCallBackBlock(&(CmpCallBackVector[i]) );
        if( RoutineBlock  ) {
            CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExGetCallBackBlockContext(RoutineBlock);
            if( CmCallbackContext && (CmCallbackContext->Cookie.QuadPart  == Cookie.QuadPart) ) {
                //
                // found it
                //
                if( ExCompareExchangeCallBack (&CmpCallBackVector[i],NULL,RoutineBlock) ) {
                    InterlockedExchangeAdd ((PLONG) &CmpCallBackCount, -1);
    
                    ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);
                    //
                    // wait for others to release their reference, then tear down the structure
                    //
                    ExWaitForCallBacks (RoutineBlock);

                    ExFreePool(CmCallbackContext);
                    ExFreeCallBack(RoutineBlock);
                    return STATUS_SUCCESS;
                }

            } else {
                ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);
            }
        }
            
    }

    return STATUS_INVALID_PARAMETER;
}

//
// Cm internals
//
NTSTATUS
CmpCallCallBacks (
    IN REG_NOTIFY_CLASS Type,
    IN PVOID            Argument,
    IN BOOLEAN          Wind,
    IN REG_NOTIFY_CLASS PostType,
    IN PVOID            Object
    )
/*++

Routine Description:

    This function calls the callback thats inside a callback structure

Arguments:

    Type - Nt call selector

    Argument - Caller provided argument to pass on (one of the REG_*_INFORMATION )

    Wind - tells if this is a pre or a post callback

    PostType - matching post notify class

    Object - to be used in case we fail part through.

Return Value:

    NTSTATUS - STATUS_SUCCESS or error status returned by the first callback

--*/
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    LONG                        i;
    LONG                        Direction;
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;
    BOOLEAN                     InternalUnWind = FALSE;

    PAGED_CODE();

    if( Wind == TRUE ) {
        Direction = 1;
        i = 0;
    } else {
        Direction = -1;
        i = CM_MAX_CALLBACKS - 1;
    }
    for(;(i >= 0) && (i < CM_MAX_CALLBACKS);i += Direction) {
        RoutineBlock = ExReferenceCallBackBlock(&(CmpCallBackVector[i]) );
        if( RoutineBlock != NULL ) {
            //
            // we have a safe reference on this block.
            //
            //
            // record thread on a stack struct, so we don't need to allocate pool for it. We unlink
            // it from our lists prior to this function exit, so we are on the safe side.
            //
            CM_ACTIVE_NOTIFY_THREAD ActiveThreadInfo;
            
            //
            // get context info
            //
            CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExGetCallBackBlockContext(RoutineBlock);
            ASSERT( CmCallbackContext != NULL );

            ActiveThreadInfo.Thread = PsGetCurrentThread();
#if DBG
            InitializeListHead(&(ActiveThreadInfo.ThreadList));   
#endif //DBG

            if( CmpCheckRecursionAndRecordThreadInfo(CmCallbackContext,&ActiveThreadInfo) ) {
                if( InternalUnWind == TRUE ) {
                    //
                    // this is an internal unwind due to fail part through. need to call posts manually
                    //
                    REG_POST_OPERATION_INFORMATION PostInfo;
                    PostInfo.Object = Object;
                    PostInfo.Status = Status;
                    ExGetCallBackBlockRoutine(RoutineBlock)(CmCallbackContext->CallerContext,(PVOID)(ULONG_PTR)PostType,&PostInfo);
                    // ignore return status.
                } else {
                    //
                    // regular callback call
                    //
                    Status = ExGetCallBackBlockRoutine(RoutineBlock)(CmCallbackContext->CallerContext,(PVOID)(ULONG_PTR)Type,Argument);
                    if( Wind == FALSE ) {
                        //
                        // always ignore post return calls so they all get a chance to run
                        //
                        Status = STATUS_SUCCESS;
                    }
                }
                //
                // now that we're down, remove ourselves from the thread list
                //
                CmpLockContext(CmCallbackContext);
                RemoveEntryList(&(ActiveThreadInfo.ThreadList));
                CmpUnlockContext(CmCallbackContext);
            } else {
                ASSERT( IsListEmpty(&(ActiveThreadInfo.ThreadList)) );
            }

            ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);

            if( !NT_SUCCESS(Status) ) {
                if( Wind == TRUE ) {
                    //
                    // switch direction and start calling the posts 
                    //
                    Wind = FALSE;
                    Direction *= -1;
                    InternalUnWind = TRUE;
                } else if( InternalUnWind == FALSE ) {
                    return Status;
                }
                //
                // else fall through so the unwind completes
                //
            }
        }
    }
    
    return Status;
}

VOID
CmpInitCallback(VOID)
/*++

Routine Description:

    Init the callback module

Arguments:



Return Value:


--*/
{
    ULONG   i;

    PAGED_CODE();
    
    CmpCallBackCount = 0;
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        ExInitializeCallBack (&(CmpCallBackVector[i]));
    }
}

BOOLEAN
CmpCheckRecursionAndRecordThreadInfo(
                                     PCM_CALLBACK_CONTEXT_BLOCK CallbackBlock,
                                     PCM_ACTIVE_NOTIFY_THREAD   ActiveThreadInfo
                                     )
/*++

Routine Description:

    Checks if current thread is already inside the callback (recursion avoidance)

Arguments:


Return Value:


--*/
{
    PLIST_ENTRY                 AnchorAddr;
    PCM_ACTIVE_NOTIFY_THREAD    CurrentThreadInfo;

    PAGED_CODE();

    CmpLockContext(CallbackBlock);
    //
	// walk the ActiveThreadList and see if we are already active
	//
	AnchorAddr = &(CallbackBlock->ThreadListHead);
	CurrentThreadInfo = (PCM_ACTIVE_NOTIFY_THREAD)(CallbackBlock->ThreadListHead.Flink);

	while ( CurrentThreadInfo != (PCM_ACTIVE_NOTIFY_THREAD)AnchorAddr ) {
		CurrentThreadInfo = CONTAINING_RECORD(
						                    CurrentThreadInfo,
						                    CM_ACTIVE_NOTIFY_THREAD,
						                    ThreadList
						                    );
		if( CurrentThreadInfo->Thread == ActiveThreadInfo->Thread ) {
			//
			// already there!
			//
            CmpUnlockContext(CallbackBlock);
            return FALSE;
		}
        //
        // skip to the next element
        //
        CurrentThreadInfo = (PCM_ACTIVE_NOTIFY_THREAD)(CurrentThreadInfo->ThreadList.Flink);
	}

    //
    // add this thread
    //
    InsertTailList(&(CallbackBlock->ThreadListHead), &(ActiveThreadInfo->ThreadList));
    CmpUnlockContext(CallbackBlock);
    return TRUE;
}

