/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    CorrectedEngine.cpp

Abstract:

    This module encapsulates the routines that are needed only for
    corrected error retrieval.
    
Author:

    Abdullah Ustuner (AUstuner) 28-August-2002
        
--*/

#include "mca.h"

extern IWbemServices *gPIWbemServices;
extern IWbemLocator *gPIWbemLocator;

//
// Event which signals the corrected error retrieval. 
//
HANDLE gErrorProcessedEvent;

//
// TimeOut period in minutes for corrected error retrieval.
//
INT gTimeOut;


BOOL
MCACreateProcessedEvent(
	VOID
	)
/*++

Routine Description:

    This function creates the "processed event", which is used to keep track of
    whether a corrected error record has been retrieved from WMI or not. When a 
    corrected error is retrieved from WMI then, this event is signaled which 
    causes the application to finish.

Arguments:

    none

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
    //
    // Create the processsed event.
    //
    gErrorProcessedEvent = CreateEvent(NULL,
                                       TRUE,
                                       FALSE,
                                       L"ErrorProcessedEvent"
                                       );

    return (gErrorProcessedEvent != NULL);
}


VOID
MCAErrorReceived(
	IN IWbemClassObject *ErrorObject
	)
/*++

Routine Description:

    This function is called by an instance of the MCAObjectSink class when a corrected
    error is retrieved from WMI. The error record data is extracted from the object and
    the contents of this record is displayed on the screen.
          
Arguments:

    ErrorObject  - Error object retrieved from WMI.    

Return Value:

    none

 --*/
{	
	PUCHAR pErrorRecordBuffer = NULL;

	//
	// Extract the actual MCA error record from the retrieved object.
	//
	if (!MCAExtractErrorRecord(ErrorObject, &pErrorRecordBuffer)) {
    
        wprintf(L"ERROR: Failed to get corrected error record data!\n");

        goto CleanUp;        
        
    }
	
	//
	// Display the corrected error record on the screen.
	//
    MCAPrintErrorRecord(pErrorRecordBuffer);

    CleanUp:
    	
    if (pErrorRecordBuffer) {
    	
        free(pErrorRecordBuffer);
        
    }
}


BOOL
MCAGetCorrectedError(
	VOID
	)
/*++

Routine Description:

    This function registers to WMI for corrected error notification and waits until
    TimeOut limit is reached or an error is retrieved. If an error is successfully
    retrieved then, the contents of the error record are displayed on the screen. 
          
Arguments:

    none
    
Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
	BOOL isSuccess = TRUE;
	HRESULT hResult = WBEM_S_NO_ERROR;
	DWORD returnValue = 0;

	//
    // Create the Sink instances, which will be responsible for handling
    // the event callbacks from WMI.
    //	
	MCAObjectSink *pCMCSink = new MCAObjectSink();
	MCAObjectSink *pCPESink = new MCAObjectSink();

    //
    // Check if instance creation was successful.
    //
    if (pCMCSink == NULL || pCPESink == NULL) {

    	isSuccess = FALSE;

    	wprintf(L"ERROR: Memory allocation failed for object sinks!");

    	goto CleanUp;
    	
    }

   	//
	// Complete the required initialization tasks.
	//
	if (!MCAInitialize()) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Initialization failed!\n");

		goto CleanUp;	
		
	}  

    //
    // Create processed event, which will be used to signal event retrieval from WMI.
    //
    if(!MCACreateProcessedEvent()){

        isSuccess = FALSE;

        wprintf(L"ERROR: Processed event creation failed!\n");

        goto CleanUp;          
            
    }

	//
	// Register to WMI for CMC event notification.
	//
	if (!MCARegisterCMCConsumer(pCMCSink)) {

		isSuccess = FALSE;

		goto CleanUp;
		
	}
	
	//
	// Register to WMI for CPE event notification.
	//
	if (!MCARegisterCPEConsumer(pCPESink)) {

		isSuccess = FALSE;

		goto CleanUp;
		
	}

	wprintf(L"INFO: Waiting for notification from WMI...\n");

    //
    // Wait for error retrieval until TimeOut limit is reached.
    //
    returnValue = WaitForSingleObjectEx(gErrorProcessedEvent,
                   				        gTimeOut*60*1000,
                          			    FALSE
                          				);

    if (returnValue == WAIT_TIMEOUT) {
    	
    	wprintf(L"INFO: No error notification is received during the timeout period.\n");
    	
    }

	CleanUp:	

	if (gPIWbemServices) { 

	    //
    	// Cancel any currently pending asynchronous call based on the MCAObjectSink pointers.
    	//
	    hResult = gPIWbemServices->CancelAsyncCall(pCMCSink);
    
    	if (hResult != WBEM_S_NO_ERROR){

        	wprintf(L"IWbemServices::CancelAsyncCall failed on CMCSink: %d\n", hResult);
        	
	    }

    	hResult = gPIWbemServices->CancelAsyncCall(pCPESink);
	
	    if(hResult != WBEM_S_NO_ERROR){

    	    wprintf(L"IWbemServices::CancelAsyncCall failed on CPESink: %d\n", hResult);
    	    
	    }

	    gPIWbemServices->Release();
	    
	}  

	//
	// Release the sink object associated with CMC notification.
	//
	if (pCMCSink != NULL) {
			
	   	pCMCSink->Release();
	    	
	}

	//
	// Release the sink object associated with CPE notification.
	//
	if (pCPESink != NULL) {
			
	    pCPESink->Release();
		    
	}

	if (gPIWbemLocator) {

		gPIWbemLocator->Release();
	}
			
	return isSuccess;
}


BOOL
MCARegisterCMCConsumer(
	MCAObjectSink *pCMCSink
	)
/*++

Routine Description:

    This function registers the provided object sink as a temporary consumer
    to WMI for CMC event notification.
          
Arguments:

    pCMCSink - Object sink that will be registered to WMI for CMC error notification.

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/ 
{
	HRESULT hResult = 0;
	BOOL isSuccess = TRUE;
 	LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawCMCEvent";	

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}

    //
    // Register the object sink as a temporary consumer to WMI for CMC error notification.
    //
    hResult = gPIWbemServices->ExecNotificationQueryAsync(bQueryLanguage,
                                                          bQueryStatement,
                                                          WBEM_FLAG_SEND_STATUS,
                                                          NULL,
                                                          pCMCSink
                                                          );
    
    if (FAILED(hResult)) { 

    	isSuccess = FALSE;

        wprintf(L"ERROR: Temporary consumer registration for CMC failed!\n");

        wprintf(L"Result: 0x%x\n", hResult);

        goto CleanUp;
        
    }

    wprintf(L"INFO: Registered for CMC error notification successfully.\n");

    CleanUp:

    if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}

    return isSuccess;    
}


BOOL
MCARegisterCPEConsumer(
	MCAObjectSink *pCPESink
	)
/*++

Routine Description:

    This function registers the provided object sink as a temporary consumer
    to WMI for CPE event notification.
          
Arguments:

    pCPESink - Object sink that will be registered to WMI for CPE error notification.

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
    HRESULT hResult = 0;
    BOOL isSuccess = TRUE;
    LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawCorrectedPlatformEvent";	

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}

    //
    // Register the object sink as a temporary consumer to WMI for CPE error notification.
    //
    hResult = gPIWbemServices->ExecNotificationQueryAsync(bQueryLanguage,
                                                          bQueryStatement,
                                                          WBEM_FLAG_SEND_STATUS,
                                                          NULL,
                                                          pCPESink
                                                          );
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

        wprintf(L"ERROR: Temporary consumer registration for CPE failed!\n");

        wprintf(L"ERROR: Result: 0x%x\n", hResult);        

        goto CleanUp;
        
    }

    wprintf(L"INFO: Registered for CPE error notification successfully.\n");

    CleanUp:

    if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}
       
    return isSuccess;    
}

