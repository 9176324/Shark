/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    FatalEngine.cpp

Abstract:

    This module encapsulates the routines that are needed only for
    fatal error retrieval.
    
Author:

    Abdullah Ustuner (AUstuner) 28-August-2002
        
--*/

#include "mca.h"

extern IWbemServices *gPIWbemServices;
extern IWbemLocator *gPIWbemLocator;


BOOL
MCAGetFatalError(
	VOID
	)
/*++

Routine Description:

    This function queries WMI for a fatal error upon successful completion
    of required initialization tasks. By using the enumerator provided by
    WMI as a response to the query, the object list is parsed and for each
    object the MCA Error Record is extracted. But in reality, there should be
    just one MCA Error Record present.

Arguments:

    none

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
	BOOL isSuccess = TRUE;
	HRESULT hResult = 0;
	PUCHAR pErrorRecordBuffer = NULL;	
	INT errorRecordCount = 0;	
	IEnumWbemClassObject *pObjectEnumerator = NULL;	
	IWbemClassObject *apObjects[10];     
    ULONG uReturned = 0;
    ULONG objectIndex = 0;
   	LPWSTR pQueryLanguage = L"WQL";
	LPWSTR pQueryStatement = L"select * from MSMCAInfo_RawMCAData";	
    
	//
	// Complete the required initialization tasks.
	//
	if (!MCAInitialize()) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Initialization failed!\n");

		goto CleanUp;
		
	}

	BSTR bQueryLanguage = SysAllocString(pQueryLanguage);	
	BSTR bQueryStatement = SysAllocString(pQueryStatement);

	if (bQueryLanguage == NULL || bQueryStatement == NULL) {

		isSuccess = FALSE;

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		goto CleanUp;
		
	}
	
	//
    // Query WMI for the fatal error record.
    //
    hResult = gPIWbemServices->ExecQuery(bQueryLanguage,
                              			 bQueryStatement,
                              			 0,
                              			 0,
                              			 &pObjectEnumerator
                              			 ); 

	if (FAILED(hResult)) {

		isSuccess = FALSE;

	    wprintf(L"ERROR: Fatal error querying failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

	    goto CleanUp;
    
    }

    //
    // Now we will retrieve objects of type IWbemClassOject from this enumeration. 
    //
    do {    	
    	
        //
        // Retrieve objects until none is left in which case the final Next()
        // will return WBEM_S_FALSE.
        //
        hResult = pObjectEnumerator->Next(WBEM_INFINITE,
                            	 		  10,
                            	 		  apObjects,
                            	 		  &uReturned
                            	 		  );

        if (SUCCEEDED(hResult)) {

        	//
        	// Now extract the actual MCA error record from the objects. 
        	//
            for (objectIndex = 0; objectIndex < uReturned; objectIndex++) {

	            if (!MCAExtractErrorRecord(apObjects[objectIndex], &pErrorRecordBuffer)) {
        	    
        	    	isSuccess = FALSE;

                    wprintf(L"ERROR: Failed to get fatal error record data!\n");
                    
                    goto CleanUp;
                    
                }

                if (pErrorRecordBuffer) {

                	errorRecordCount++;

	                wprintf(L"INFO: Succesfully retrieved fatal cache error data.\n");
	    
    			    MCAPrintErrorRecord(pErrorRecordBuffer);
    			    
                }
                
                apObjects[objectIndex]->Release();

            }

        } else {

            wprintf(L"ERROR: Couldn't retrieve WMI objects!\n");

            goto CleanUp;
            
        }
        
	} while (hResult == WBEM_S_NO_ERROR);

    if (errorRecordCount == 0) {

    	wprintf(L"INFO: No fatal error record was found!\n");
    	
    }

	CleanUp:

	//
	// Release all WMI related objects.
	//
	if (pObjectEnumerator) {

        pObjectEnumerator->Release();
        
    }

	if (gPIWbemLocator) {

		gPIWbemLocator->Release();
		
	}

	if (gPIWbemServices) {

		gPIWbemServices->Release();
		
	}

	if (bQueryLanguage != NULL) {
		
		SysFreeString(bQueryLanguage);
			
	}

	if (bQueryStatement != NULL) {
		
		SysFreeString(bQueryStatement);
			
	}
	
	return isSuccess;
}

