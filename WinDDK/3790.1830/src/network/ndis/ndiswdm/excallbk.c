/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   ExCallbk.C

Abstract: The routines in this module helps to solve driver load order
          dependency between this sample and NDISPROT sample. Since
          the NDISPROT driver can get loaded before or after our driver,
          this callback scheme helps us to know the readiness of the 
          protocol driver so that we can open the device and start
          interacting with it.

Author:  Eliyas Yakub (Jan 11, 2003)


Revision History:

Notes:

--*/
#include "ndiswdm.h"

#ifdef INTERFACE_WITH_NDISPROT

#define NDISPROT_CALLBACK_NAME  L"\\Callback\\NdisProtCallbackObject"

#define CALLBACK_SOURCE_NDISPROT    0
#define CALLBACK_SOURCE_NDISWDM     1

#pragma NDIS_PAGEABLE_FUNCTION(NICRegisterExCallback)
#pragma NDIS_PAGEABLE_FUNCTION(NICUnregisterExCallback)
#pragma NDIS_PAGEABLE_FUNCTION(NICExCallback)

BOOLEAN 
NICRegisterExCallback(
    PMP_ADAPTER Adapter)
/*++
Routine Description:

    Create or open an existing callback object and send a 
    notification. Note that the system calls our notication
    callback routine in addition to notifying other 
    registered clients.

Arguments:

    Adapter - Pointer to our adapter
    
Return Value:
    
    TRUE or FALSE

--*/
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    UNICODE_STRING      callBackObjectName;
    NTSTATUS            Status;
    BOOLEAN             bResult = TRUE;
    PCALLBACK_OBJECT    callbackObject = NULL;
    PVOID               callbackRegisterationHandle = NULL;    

    DEBUGP(MP_TRACE, ("-->NICRegisterExCallBack\n"));
    
    PAGED_CODE();
    
    do {
        
        RtlInitUnicodeString(&callBackObjectName, NDISPROT_CALLBACK_NAME);

        InitializeObjectAttributes(&ObjectAttr,
                                   &callBackObjectName,
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                   NULL,
                                   NULL);
                                   
        Status = ExCreateCallback(&callbackObject,
                                  &ObjectAttr,
                                  TRUE,
                                  TRUE);

        
        if (!NT_SUCCESS(Status))
        {

            DEBUGP(MP_ERROR, ("RegisterExCallBack: failed to create callback %lx\n", Status));
            bResult = FALSE;
            break;
        }

        callbackRegisterationHandle = ExRegisterCallback(callbackObject,
                                                                 NICExCallback,
                                                                 (PVOID)Adapter);
        if (callbackRegisterationHandle == NULL)
        {
            DEBUGP(MP_ERROR,("RegisterExCallBack: failed to register a Callback routine%lx\n", Status));
            bResult = FALSE;
            break;
        }

        ExNotifyCallback(callbackObject,
                        (PVOID)CALLBACK_SOURCE_NDISWDM,
                        (PVOID)NICInitAdapterWorker);
       
    
        MP_INC_REF(Adapter);            
        
    }while(FALSE);

    if(!bResult) {
        if (callbackRegisterationHandle)
        {
            ExUnregisterCallback(callbackRegisterationHandle);
        }

        if (callbackObject)
        {
            ObDereferenceObject(callbackObject);
        }        
    } else {
    
        Adapter->CallbackObject = callbackObject;
        Adapter->CallbackRegisterationHandle = callbackRegisterationHandle;    
    }

    DEBUGP(MP_TRACE, ("<--NICRegisterExCallBack\n"));

    return bResult;
    
    
}

VOID 
NICUnregisterExCallback(
    PMP_ADAPTER Adapter
)
/*++
Routine Description:

    Unregister out callback routine and also delete the object
    by removing the reference on it.

Arguments:

    Adapter - Pointer to our adapter
    
Return Value:
    
    VOID

--*/
{
    PAGED_CODE();

    DEBUGP(MP_TRACE, ("--> NICUnregisterExCallBack\n"));
    
    if (Adapter->CallbackRegisterationHandle)
    {
        ExUnregisterCallback(Adapter->CallbackRegisterationHandle);
        Adapter->CallbackRegisterationHandle = NULL;
    }

    if (Adapter->CallbackObject)
    {
        ObDereferenceObject(Adapter->CallbackObject);
        Adapter->CallbackObject = NULL;
        MP_DEC_REF(Adapter);             
    }        


    DEBUGP(MP_TRACE, ("<-- NICUnregisterExCallBack\n"));
    
}

VOID
NICExCallback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   UnUsed
    )
/*++
Routine Description:

    This is the notification callback routine called by the executive
    subsystem in the context of the thread that make the notification
    on the object we have registered. This routine could be called
    by the NDISPROT driver or another instance of our own miniport.

Arguments:

    CallBackContext - This is the context value specified in the
                        ExRegisterCallback call. It's a pointer
                        to Adapter context.
                                                
    Source - First parameter specified in the call to ExNotifyCallback.
                     This is used to indenty the caller. This could be
                     either ourself or NDISWDM.
                     
    UnUsed -  Second parameter specified in the call to ExNotifyCallback.
              This is an additonal context the caller can specify. 
              It's not used.
                   
    
Return Value:
    
    VOID

--*/
{
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    PNDIS_WORK_ITEM  workItem;
    PMP_ADAPTER Adapter = (PMP_ADAPTER)CallBackContext;
    
    PAGED_CODE();
    
    DEBUGP(MP_TRACE, ("-->NICExCallback: Context %p, Source %lx, UnUsed %p\n", 
                            CallBackContext, Source, UnUsed));
    
    //
    // if we are the one issuing this notification, just return
    //
    if (Source == (PVOID)CALLBACK_SOURCE_NDISWDM) {
        return;
    }
    
    //
    // Notification is coming from NDIS protocol driver. To avoid calling back 
    // into the protocol directly, let us queue a workitem to open the protocol 
    // interface.
    //
    if(Source == (PVOID)CALLBACK_SOURCE_NDISPROT) {        
        //
        // Allocate memory for workitem
        //
        Status = NdisAllocateMemoryWithTag(
            &workItem, 
            sizeof(NDIS_WORK_ITEM),
            NIC_TAG);
        
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("Failed to allocate memory for workitem\n"));
            
        } else {
            MP_INC_REF(Adapter);            
            NdisInitializeWorkItem(workItem, NICInitWorkItemCallback, Adapter);
            NdisScheduleWorkItem(workItem);     
        }
    } else {
        DEBUGP(MP_ERROR, ("Unknown callback notification source\n"));
    }
    
    DEBUGP(MP_TRACE, ("<--NICExCallback: Source,  %lx\n", Source));
    
}

#endif
