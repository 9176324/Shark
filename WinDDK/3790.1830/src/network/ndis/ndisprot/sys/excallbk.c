/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    ExCallbk.c

Abstract: The routines in this module helps to solve driver load order
          dependency between this sample and NDISWDM sample. These 
          routines are not required in a typical protocol driver. By default
          this module is not included in the sample. You include these routines
          by adding EX_CALLBACK defines to the 'sources' file. Read the
          NDISWDM samples readme file for more information on how ExCallback
          kernel interfaces are used to solve driver load order issue.


Author: Eliyas Yakub (Jan 12, 2003)

Environment:

    Kernel mode


Revision History:

--*/

#include "precomp.h"

#ifdef EX_CALLBACK

#define __FILENUMBER 'LCxE'

#define NDISPROT_CALLBACK_NAME  L"\\Callback\\NdisProtCallbackObject"

#define CALLBACK_SOURCE_NDISPROT    0
#define CALLBACK_SOURCE_NDISWDM     1

PCALLBACK_OBJECT                CallbackObject = NULL;
PVOID                           CallbackRegisterationHandle = NULL;

typedef VOID (* NOTIFY_PRESENCE_CALLBACK)(OUT PVOID Source);

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, ndisprotRegisterExCallBack)
#pragma alloc_text(PAGE, ndisprotUnregisterExCallBack)

#endif // ALLOC_PRAGMA

BOOLEAN 
ndisprotRegisterExCallBack(VOID)
/*++
Routine Description:

    Create or open an existing callback object and send a 
    notification. Note that the system calls our notication
    callback routine in addition to notifying other 
    registered clients.

Arguments:
    
Return Value:
    
    TRUE or FALSE

--*/
{
    OBJECT_ATTRIBUTES   ObjectAttr;
    UNICODE_STRING      CallBackObjectName;
    NTSTATUS            Status;
    BOOLEAN             bResult = TRUE;

    DEBUGP(DL_LOUD, ("--> ndisprotRegisterExCallBack\n"));

    PAGED_CODE();
    
    do {
        
        RtlInitUnicodeString(&CallBackObjectName, NDISPROT_CALLBACK_NAME);

        InitializeObjectAttributes(&ObjectAttr,
                                   &CallBackObjectName,
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                   NULL,
                                   NULL);
                                   
        Status = ExCreateCallback(&CallbackObject,
                                  &ObjectAttr,
                                  TRUE,
                                  TRUE);

        
        if (!NT_SUCCESS(Status))
        {

            DEBUGP(DL_ERROR, ("RegisterExCallBack: failed to create callback %lx\n", Status));
            bResult = FALSE;
            break;
        }
       
        CallbackRegisterationHandle = ExRegisterCallback(CallbackObject,
                                                                 ndisprotCallback,
                                                                 (PVOID)NULL);
        if (CallbackRegisterationHandle == NULL)
        {
            DEBUGP(DL_ERROR,("RegisterExCallBack: failed to register a Callback routine%lx\n", Status));
            bResult = FALSE;
            break;
        }

        ExNotifyCallback(CallbackObject,
                        (PVOID)CALLBACK_SOURCE_NDISPROT,
                        (PVOID)NULL);
       
    
    }while(FALSE);

    if(!bResult) {
        if (CallbackRegisterationHandle)
        {
            ExUnregisterCallback(CallbackRegisterationHandle);
            CallbackRegisterationHandle = NULL;
        }

        if (CallbackObject)
        {
            ObDereferenceObject(CallbackObject);
            CallbackObject = NULL;
        }        
    }

    DEBUGP(DL_LOUD, ("<-- ndisprotRegisterExCallBack\n"));

    return bResult;
    
}

VOID 
ndisprotUnregisterExCallBack()
/*++
Routine Description:

    Unregister out callback routine and also delete the object
    by removing the reference on it.

Arguments:
    
Return Value:
    
    VOID

--*/
{
    DEBUGP(DL_LOUD, ("--> ndisprotUnregisterExCallBack\n"));

    PAGED_CODE();

    if (CallbackRegisterationHandle)
    {
        ExUnregisterCallback(CallbackRegisterationHandle);
        CallbackRegisterationHandle = NULL;
    }

    if (CallbackObject)
    {
        ObDereferenceObject(CallbackObject);
        CallbackObject = NULL;
    }   
    
    DEBUGP(DL_LOUD, ("<-- ndisprotUnregisterExCallBack\n"));
    
}

VOID
ndisprotCallback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   CallbackAddr
    )
/*++
Routine Description:

    This is the notification callback routine called by the executive
    subsystem in the context of the thread that make the notification
    on the object we have registered. This routine could be called
    by the NDISPROT driver or another instance of our own miniport.

Arguments:

    CallBackContext - This is the context value specified in the
                        ExRegisterCallback call. It's NULL.
                        
    Source - First parameter specified in the call to ExNotifyCallback.
                     This is used to indenty the caller. This could be
                     either ourself or NDISWDM.
                     
    CallbackAddr -  Second parameter specified in the call to ExNotifyCallback.
                     This is an additonal context the caller can specify. When it
                     comes from NDISWDM, it's a routine to call as part of
                     the notification.
Return Value:
    
    VOID

--*/
{
    NOTIFY_PRESENCE_CALLBACK func;
    
    DEBUGP(DL_LOUD, ("==>ndisprotoCallback: Source %lx, CallbackAddr %p\n", 
                            Source, CallbackAddr));
    
    //
    // if we are the one issuing this notification, just return
    //
    if (Source == CALLBACK_SOURCE_NDISPROT) {        
        return;
    }
    
    //
    // Notification is coming from NDISWDM
    // let it know that you are here
    //
    ASSERT(Source == (PVOID)CALLBACK_SOURCE_NDISWDM);
    
    if(Source == (PVOID)CALLBACK_SOURCE_NDISWDM) {

        ASSERT(CallbackAddr);
        
        if (CallbackAddr == NULL)
        {
            DEBUGP(DL_ERROR, ("Callback called with invalid address %p\n", CallbackAddr));
            return;     
        }

        func = CallbackAddr;
    
        func(CALLBACK_SOURCE_NDISPROT);
    }
    
    DEBUGP(DL_LOUD, ("<==ndisprotoCallback: Source,  %lx\n", Source));
    
}

#endif

