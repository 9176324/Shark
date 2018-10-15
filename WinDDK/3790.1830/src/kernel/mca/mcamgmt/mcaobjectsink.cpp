/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    MCAObjectSink.cpp

Abstract:

    This class is used to register as a temporary consumer to WMI for CMC and CPE 
    event notifications. It functions as a IWbemObjectSink to retrieve the
    instances in the result of the query (WMI event notification). When a registered
    event occurs, WMI calls the Indicate function to notify about the event. 

    For more information look at MSDN for: IWbemObjectSink.
    
Author:

    Abdullah Ustuner (AUstuner) 28-August-2002
        
--*/

#include "MCAObjectSink.h"

extern HANDLE gErrorProcessedEvent;


MCAObjectSink::MCAObjectSink()
/*++

Routine Description:

    This function is the constructor of the class. It is responsible for initializing
    the member variables. 
    
Arguments:

    none

Return Value:

    none

 --*/
{    
    referenceCount = 0;
}


ULONG
MCAObjectSink::AddRef()
/*++

Routine Description:

    This function increases the object's reference count by one. The function prevents
    more than one thread to increase the value simultaneously.
    
Arguments:

    none

Return Value:

    Reference count to this object (after increment operation).

 --*/
{
    return InterlockedIncrement(&referenceCount);
}


ULONG
MCAObjectSink::Release()
/*++

Routine Description:

    This function decreases the object's reference count by one. The function prevents
    more than one thread to decrease the value simultaneously. If no other reference 
    is left, the object is deallocated.
    
Arguments:

    none

Return Value:

    lRef - Reference count to this object (after decrement operation).

 --*/
{
    LONG lRef = InterlockedDecrement(&referenceCount);
    
    if (lRef == 0) {
        
        delete this;
        
    }
    
    return lRef;
}


HRESULT
MCAObjectSink::QueryInterface(IN REFIID riid,
                              OUT VOID** ppv
                              )
/*++

Routine Description:

    This function determines if the object supports a particular COM interface.
    If it does, the system increases the object's reference count, and the
    application can use that interface immediately.

Arguments:

    riid - The COM interface identifier of the requested interface.

    ppv - Address of a pointer that will be filled with the interface pointer if
    	  the query succeeds.

Return Value:

    S_OK - Successful
    E_NOINTERFACE - Unsuccessful. Requested interface is not supported.

 --*/
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
        
        *ppv = (IWbemObjectSink *) this;
        
        AddRef();
        
        return S_OK;
        
    }
    
    else return E_NOINTERFACE;
}


HRESULT
MCAObjectSink::Indicate(IN LONG lObjCount,
                        IN IWbemClassObject **pArray
                      )
/*++

Routine Description:

    This function receives the notifications from the WMI provider.

Arguments:

    lObjCount - The number of objects in the following array of pointers.

    pArray - An array of pointers to IWbemClassObject interfaces, that is the event objects.
             The array memory itself is read-only, and is owned by the caller of the method.
             Since this is an in-parameter, the interface called has the option of calling
             IWbemServices::AddRef on any object pointer in the array and holding it before
             returning. The interface called is only required to copy and then call 
             IWbemServices::AddRef on the pointers if the pointers will be used after the 
             call returns, according to COM rules. The interface called should not call 
             Release on the objects without corresponding calls to AddRef. For more 
             information on the IUnknown Interface methods, see the Microsoft Platform SDK.

Return Value:

    WBEM_S_NO_ERROR - Indicating successful processing of the notification.

 --*/
{
    LONG objectIndex = 0;

    //
    // Extract all of the event objects from the array and inform the test engine.
    //
    for (objectIndex = 0; objectIndex < lObjCount; objectIndex++) {      

        //
        // Inform the corrected engine about the event retrieval.
        //
        MCAErrorReceived(pArray[objectIndex]);      
     
        SetEvent(gErrorProcessedEvent);

    }

    return WBEM_S_NO_ERROR;
}


HRESULT
MCAObjectSink::SetStatus(IN LONG lFlags,
                         IN HRESULT hResult,
                         IN BSTR strParam,
                         IN IWbemClassObject __RPC_FAR *pObjParam
                         )
/*++

Routine Description:

    This function is called by sources to indicate the end of a notification sequence
    or the end of a result code of an asynchronous method of IWbemServices.

Arguments:

    lFlags - Reserved. It must be zero.

    hResult - Set to the HRESULT of the asynchronous operation or notification.

    strParam - Receives a pointer to a read-only BSTR, if the original asynchronous operation
               returns a string.

    pObjParam - In cases where a complex error or status object is returned, this contains
                a pointer to the object. If the object is required after the call returns,
                the called object must AddRef the pointer before returning.

Return Value:

    WBEM_S_NO_ERROR

 --*/
{
  return WBEM_S_NO_ERROR;
}

