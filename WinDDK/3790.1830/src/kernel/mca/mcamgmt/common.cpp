/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    Common.cpp

Abstract:

    This module encapsulates the common routines that are used 
    during both fatal and corrected error retrieval.
    
Author:

    Abdullah Ustuner (AUstuner) 26-August-2002
        
--*/

#include "mca.h"

IWbemServices *gPIWbemServices = NULL;
IWbemLocator *gPIWbemLocator = NULL;


BOOL
MCAExtractErrorRecord(
    IN IWbemClassObject *PObject,
    OUT PUCHAR *PRecordBuffer    
)
/*++

Routine Description:

    This function retrieves embedded objects from the error record 
    (obtained from WMI) that contain both the record data and other
    information about the record (such as length). The data is saved
    into the output buffer provided.
          
Arguments:

    PObject  - Event object retrieved from WMI.
    PRecordBuffer - Pointer to a buffer to save the MCA error record.

Return Value:

    TRUE - Successful.
    FALSE - Unsuccessful.

 --*/
{    
    IWbemClassObject *pRecordsObject = NULL;
    VARIANT recordsPropertyVariant;
    VARIANT countPropertyVariant;
   	VARIANT recordLengthVariant;
   	VARIANT recordDataVariant;
    HRESULT hResult = WBEM_S_NO_ERROR;
    IUnknown *punk = NULL;
    LONG mcaRecordByte = 0, index = 0;
    UCHAR recordDataByte;
	BOOL isSuccess = TRUE;

    //
    // Retrieve the "Records" property value of the event object.
    //
    hResult = PObject->Get(L"Records",
                           0,
                           &recordsPropertyVariant,
                           NULL,
                           NULL
                           );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

    	wprintf(L"ERROR: \"Records\" property value couldn't be retrieved!\n");

        goto CleanUp;
        
    }

  	//
    // Retrieve the "Count" property value of the event object.
    //        
    hResult = PObject->Get(L"Count",
                           0,
                           &countPropertyVariant,
                           NULL,
                           NULL
                           );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;

        wprintf(L"ERROR: \"Count\" property value couldn't be retrieved!\n");

        goto CleanUp;
        
    }

    //
    // Check the "Count" property to ensure that it is not zero.
    //            
    if (countPropertyVariant.lVal < 1) {

    	isSuccess = FALSE;

		wprintf(L"ERROR: \"Count\" is less than 1!\n");

		goto CleanUp;
		
    }

    //
    // The Records Property Variant.parray should contain a
    // pointer to an array of pointers. However, MCA should only
    // place one pointer in this array. Use the Safearray APIs to
    // get that pointer.
    //
    hResult = SafeArrayGetElement(recordsPropertyVariant.parray,
                                  &index,
                                  &punk
                                  );

    if(FAILED(hResult)){

    	isSuccess = FALSE;
       
		wprintf(L"ERROR: Couldn't retrieve array pointer!\n");

        goto CleanUp;
        
    }   

    //
    // Punk should contain an object of type IWbemClassObject. This should
    // be the MCA record object that will contain "Length" and "Data" elements.
    //
    hResult = (punk->QueryInterface(IID_IWbemClassObject,
                                    (PVOID*)&pRecordsObject)
                                    );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;
        
        wprintf(L"ERROR: Interface pointer couldn't be retrieved!\n");

        goto CleanUp;
        
    }   
    
    //
    // Obtain the length of the error record.
    //
    hResult = pRecordsObject->Get(L"Length",
                                  0,
                                  &recordLengthVariant,
                                  NULL,
                                  NULL
                                  );

    if (FAILED(hResult)) {

    	isSuccess = FALSE;

    	wprintf(L"\"Length\" property value couldn't be retrieved!\n");

    	goto CleanUp;

    }
        
    //
    // Obtain the actual data from the records object. This should contain a parray
    // that points to the actual MCA data we are looking for.
    //
    hResult = pRecordsObject->Get(L"Data",
                                  0,
                                  &recordDataVariant,
                                  NULL,
                                  NULL
                                  );
    
    if (FAILED(hResult)) {

    	isSuccess = FALSE;
        
        wprintf(L"\"Data\" property value couldn't be retrieved!\n");

        goto CleanUp;

    } 

    //
    // Check if the "Data" field in the record contains any data.
    //
    if (recordDataVariant.parray == NULL) {       

    	isSuccess = FALSE;

        wprintf(L"ERROR: Error record contains to data!\n");

        goto CleanUp;
        
    }

    PUCHAR PTempBuffer = NULL;

    //
    // Allocate memory for the error record buffer. The size of the memory should be
    // equal to the size of the MCA error record data field.
    //    
    if ((*PRecordBuffer) == NULL) {
            
        *PRecordBuffer = (PUCHAR)(calloc(recordLengthVariant.lVal, sizeof(UINT8)));

        if((*PRecordBuffer) == NULL) {

        	isSuccess = FALSE;

	        wprintf(L"ERROR: Memory allocation for record buffer failed!\n");

    	    goto CleanUp;
    	    
        }
        
    } else{

        PTempBuffer = (PUCHAR)(realloc(*PRecordBuffer, (recordLengthVariant.lVal * sizeof(UINT8))));

        //
	    // If reallocation of memory for the buffer failed, then display error message and return.
    	//
        if (PTempBuffer == NULL) {

        	isSuccess = FALSE;

	        wprintf(L"ERROR: Memory reallocation for record buffer failed!\n");

    	    goto CleanUp;
        	
        } else {

        	*PRecordBuffer = PTempBuffer;

        	ZeroMemory(*PRecordBuffer, recordLengthVariant.lVal * sizeof(**PRecordBuffer));       
        
        }
        
    }       

    //
    // Get the MCA error record data byte by byte and save it into the allocated buffer.
    //        
    for (mcaRecordByte = 0; mcaRecordByte < recordLengthVariant.lVal; mcaRecordByte++){

        recordDataByte = 0;
            
        hResult = SafeArrayGetElement(recordDataVariant.parray,
                                      &mcaRecordByte,
                                      &recordDataByte
                                      );

        if (FAILED(hResult)) {

        	isSuccess = FALSE;

            wprintf(L"ERROR: Error record data couldn't be read!\n");

            goto CleanUp;
        }
 
        // Copy error record data byte into buffer.
        *((*PRecordBuffer) + (mcaRecordByte * sizeof(UINT8))) = recordDataByte;
        
    }           

    CleanUp:

    VariantClear(&recordDataVariant);
    
    VariantClear(&recordLengthVariant);

    VariantClear(&recordsPropertyVariant);

    VariantClear(&countPropertyVariant);
        
    return isSuccess;
}


BOOL
MCAInitialize(
	VOID
	)
/*++

Routine Description:

    This function accomplishes the required initialization tasks required by
    both fatal and corrected error retrieval.
          
Arguments:

    none

Return Value:

    TRUE - Successful.
    FALSE - Unsuccessful.

 --*/
{
	BOOL isSuccess = TRUE;
	
	//
	// Initialize COM Library
	//
	if (!MCAInitializeCOMLibrary()) {
		
		return FALSE;
		
	}

	//
	// Set Security
	//
	if(!MCAInitializeWMISecurity()){
            
        return FALSE;
            
    }

	return isSuccess;	
}


BOOL
MCAInitializeCOMLibrary(
	VOID
	)
/*++

Routine Description:

    This function initializes the COM library.
          
Arguments:

    none

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
    HRESULT hResult = 0;    
    
    hResult = CoInitializeEx(0, COINIT_MULTITHREADED); 
    
    if (FAILED(hResult)) {

        wprintf(L"ERROR: COM library initialization failed!\n");

        wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
    }

    wprintf(L"INFO: COM library initialization is successfully completed.\n");
        
    return TRUE;
}


BOOL
MCAInitializeWMISecurity(
	VOID
	)
/*++

Routine Description:

    This function initializes the required security settings and establishes 
    the connection to the WMI server on the local system.
    
Arguments:

    none

Return Value:

    TRUE  - Successful.
    FALSE - Unsuccessful.

 --*/
{
	HRESULT hResult = 0;    
	LPWSTR pNamespace = L"ROOT\\WMI";

	//
	// Register security and set the security values for the current process.
	//
	hResult = CoInitializeSecurity(NULL,
                                   -1,
                                   NULL,
                                   NULL,
                                   RPC_C_AUTHN_LEVEL_CONNECT,
                                   RPC_C_IMP_LEVEL_IDENTIFY,
                                   NULL,
                                   EOAC_NONE,
                                   NULL
                                   );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Security initialization failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
        
	}

	//
	// Create a single uninitialized object of class IWbemLocator on the local system.
	//
	hResult = CoCreateInstance(CLSID_WbemLocator,
                               0,
                               CLSCTX_INPROC_SERVER,
                               IID_IWbemLocator,
                               (LPVOID *) &gPIWbemLocator
                               );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: IWbemLocator instance creation failed!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);

        return FALSE;
        
	}

	BSTR bNamespace = SysAllocString(pNamespace);	

	if (bNamespace == NULL) {	

		wprintf(L"ERROR: Memory allocation for string failed!\n");

		return FALSE;
		
	}

    //
	// Connect to the root\wmi namespace with the current user.
	//
    hResult = (gPIWbemLocator)->ConnectServer(bNamespace,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  NULL,
                                     		  &gPIWbemServices
                                     		  );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Could not connect to the WMI Server!\n");
	    
	    wprintf(L"ERROR: Result: 0x%x\n", hResult);
        
        return FALSE;
        
	}

    //
    // Set the authentication information on the specified proxy such that
    // impersonation of the client occurs.
    //
	hResult = CoSetProxyBlanket(gPIWbemServices,
                                RPC_C_AUTHN_WINNT,
                                RPC_C_AUTHZ_NONE,
                                NULL,
                                RPC_C_AUTHN_LEVEL_CALL,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL,
                                EOAC_NONE
                                );

	if (FAILED(hResult)) {

	    wprintf(L"ERROR: Could not set proxy blanket!\n");

	    wprintf(L"ERROR: Result: 0x%x\n", hResult);
        
        return FALSE;
	
    }

	wprintf(L"INFO: WMI security is initialized successfully.\n");

	//
	// Free the string allocated for storing the namespace.
	//
	if (bNamespace != NULL) {
		
		SysFreeString(bNamespace);
			
	}

    return TRUE;    
}

#if defined(_X86_)

VOID
MCAPrintErrorRecordX86(
	PUCHAR PErrorData
	)
/*++

Routine Description:

    This function displays the machine check exception information on X86 
    systems to the standard output (console screen).

Arguments:

    PErrorData - Buffer containing the machine check exception information.

Return Value:

    none

 --*/
{

	PMCA_EXCEPTION pMCAException = NULL;

	pMCAException = (PMCA_EXCEPTION)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"**************************************************\n");
    wprintf(L"*              X86 MCA EXCEPTION                 *\n");
    wprintf(L"**************************************************\n");
    wprintf(L"* VersionNumber   : 0x%08x\n", (ULONG) pMCAException->VersionNumber);
    wprintf(L"* ExceptionType   : 0x%08x\n", (INT)   pMCAException->ExceptionType);
    wprintf(L"* TimeStamp \n");
    wprintf(L"*     LowPart : 0x%08x\n",(ULONG) pMCAException->TimeStamp.LowPart);
    wprintf(L"*     HighPart: 0x%08x\n", (LONG)  pMCAException->TimeStamp.HighPart);
    wprintf(L"* ProcessorNumber : 0x%08x\n", (ULONG) pMCAException->ProcessorNumber);
    wprintf(L"* Reserved1       : 0x%08x\n", (ULONG) pMCAException->Reserved1);

    if (pMCAException->ExceptionType == HAL_MCE_RECORD) {

    	wprintf(L"* Mce \n");    	
    	wprintf(L"*     Address     : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Address);
	    wprintf(L"*     Type        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Type);
	    
    } else {

        wprintf(L"* Mca \n");
    	wprintf(L"*     BankNumber  : 0x%02x\n", (UCHAR) pMCAException->u.Mca.BankNumber);

	    for (int index = 0 ; index < 7 ; index++) {
   		
   			wprintf(L"*     Reserved2[%d]: 0x%02x\n", index, (UCHAR) pMCAException->u.Mca.Reserved2[index]);

   		}
	    
	    wprintf(L"*     MciStats \n");	    
    	wprintf(L"*         McaCod      : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStats.McaCod);
    	wprintf(L"*         MsCod       : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStats.MsCod);
    	wprintf(L"*         OtherInfo   : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.OtherInfo);
    	wprintf(L"*         Damage      : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Damage);
    	wprintf(L"*         AddressValid: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.AddressValid);
    	wprintf(L"*         MiscValid   : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.MiscValid);
    	wprintf(L"*         Enabled     : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Enabled);
    	wprintf(L"*         UnCorrected : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.UnCorrected);
    	wprintf(L"*         OverFlow    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.OverFlow);
    	wprintf(L"*         Valid       : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStats.Valid);    	
	    wprintf(L"*     Address \n");
	    wprintf(L"*         Address : 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Address);
	    wprintf(L"*         Reserved: 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Reserved);	    
    	wprintf(L"*     Misc        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mca.Misc);
    	
    }

    wprintf(L"* ExtCnt          : 0x%08x\n", (ULONG) pMCAException->ExtCnt);
   	wprintf(L"* Reserved3       : 0x%08x\n", (ULONG) pMCAException->Reserved3);
  	
   	for (int index = 0 ; index < MCA_EXTREG_V2MAX ; index++) {
   		
   		wprintf(L"* ExtReg[%2d]     : 0x%016I64x\n" , index, (ULONGLONG) pMCAException->ExtReg[index]);

   	}
   	
   	wprintf(L"*********************************************\n\n");
}

#endif // _X86_

#if defined(_AMD64_)

VOID
MCAPrintErrorRecordAMD64(
	PUCHAR PErrorData
	)
/*++

Routine Description:

    This function displays the machine check exception information on AMD64
    systems to the standard output (console screen).

Arguments:

    PErrorData - Buffer containing the machine check exception information.

Return Value:

    none

 --*/
{
	PMCA_EXCEPTION pMCAException = NULL;

	pMCAException = (PMCA_EXCEPTION)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"*********************************************\n");
    wprintf(L"*           x64 MCA EXCEPTION               *\n");
    wprintf(L"*********************************************\n");
    wprintf(L"* VersionNumber   : 0x%08x\n", (ULONG) pMCAException->VersionNumber);
    wprintf(L"* ExceptionType   : 0x%08x\n", (INT)   pMCAException->ExceptionType);
    wprintf(L"* TimeStamp \n");
    wprintf(L"*     LowPart : 0x%08x\n", (ULONG) pMCAException->TimeStamp.LowPart);
    wprintf(L"*     HighPart: 0x%08x\n", (LONG)  pMCAException->TimeStamp.HighPart);
    wprintf(L"* ProcessorNumber : 0x%08x\n", (ULONG) pMCAException->ProcessorNumber);
    wprintf(L"* Reserved1       : 0x%08x\n", (ULONG) pMCAException->Reserved1);

    if (pMCAException->ExceptionType == HAL_MCE_RECORD) {

   	   	wprintf(L"* Mce \n");
   	   	
    	wprintf(L"*     Address     : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Address);
	    wprintf(L"*     Type        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mce.Type);
	    
    } else {

	 	wprintf(L"* Mca \n");   
	   
    	wprintf(L"*     BankNumber  : 0x%02x\n", (UCHAR) pMCAException->u.Mca.BankNumber);

	    for (int index = 0 ; index < 7 ; index++) {
   		
   			wprintf(L"*     Reserved2[%d]: 0x%02x\n", index, (UCHAR) pMCAException->u.Mca.Reserved2[index]);

   		}
	    
    	wprintf(L"*     MciStatus\n");
   		wprintf(L"*         McaErrorCode    : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStatus.McaErrorCode);
    	wprintf(L"*         ModelErrorCode  : 0x%04x\n", (USHORT) pMCAException->u.Mca.Status.MciStatus.ModelErrorCode);
    	wprintf(L"*         OtherInformation: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.OtherInformation);
    	wprintf(L"*         ContextCorrupt  : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.ContextCorrupt);
    	wprintf(L"*         AddressValid    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.AddressValid);
    	wprintf(L"*         MiscValid       : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.MiscValid);
    	wprintf(L"*         ErrorEnabled    : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.ErrorEnabled);
    	wprintf(L"*         UncorrectedError: 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.UncorrectedError);
    	wprintf(L"*         StatusOverFlow  : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.StatusOverFlow);
    	wprintf(L"*         Valid           : 0x%01x\n", (ULONG) pMCAException->u.Mca.Status.MciStatus.Valid);    	
   	    wprintf(L"*     Address \n");
	    wprintf(L"*         Address : 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Address);
	    wprintf(L"*         Reserved: 0x%08x\n", (ULONG) pMCAException->u.Mca.Address.Reserved);
    	wprintf(L"*     Misc        : 0x%016I64x\n", (ULONGLONG) pMCAException->u.Mca.Misc);
    	
    }  	
   	
   	wprintf(L"*********************************************\n\n");
}

#endif // _AMD64_

#if defined(_IA64_)

VOID
MCAPrintErrorRecordIA64(
	PUCHAR PErrorData
	)
/*++

Routine Description:

    This function displays the headers of the provided MCA error
    record on IA64 systems to the standard output (console screen).
    The Error Record Header and Section Headers are displayed in 
    a formatted manner.

Arguments:

    PErrorData - Buffer containing the MCA error record.

Return Value:

    none

 --*/
{
	PERROR_RECORD_HEADER pErrorRecordHeader = NULL;
    PERROR_SECTION_HEADER pErrorSectionHeader = NULL;
    ULONG sectionOffset = 0;
    INT sectionNumber = 0;

    //
    // The record header must be at the top of the record buffer.
    //
    pErrorRecordHeader = (PERROR_RECORD_HEADER)PErrorData;

    wprintf(L"\n");
    
    wprintf(L"***************************************************************************\n");
    wprintf(L"*                             IA64 MCA ERROR                              *\n");
    wprintf(L"***************************************************************************\n");
	wprintf(L"*                           Error Record Header                           *\n");
    wprintf(L"*-------------------------------------------------------------------------*\n");
    wprintf(L"* ID           : 0x%I64x\n", (ULONGLONG)pErrorRecordHeader->Id);
    wprintf(L"* Revision     : 0x%x\n"   , (ULONG) pErrorRecordHeader->Revision.Revision);
    wprintf(L"*     Major    : %x\n"	 , (ULONG) pErrorRecordHeader->Revision.Major);
    wprintf(L"*     Minor    : %x\n"	 , (ULONG) pErrorRecordHeader->Revision.Minor);
    wprintf(L"* Severity     : 0x%x\n"   , (ULONG) pErrorRecordHeader->ErrorSeverity);
    wprintf(L"* Validity     : 0x%x\n"   , (ULONG) pErrorRecordHeader->Valid.Valid);
    wprintf(L"*     OEMPlatformID : %x\n", (ULONG) pErrorRecordHeader->Valid.OemPlatformID);
    wprintf(L"* Length       : 0x%x\n"   , (ULONG) pErrorRecordHeader->Length);
    wprintf(L"* TimeStamp    : 0x%I64x\n", (ULONGLONG) pErrorRecordHeader->TimeStamp.TimeStamp);
    wprintf(L"*     Seconds : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Seconds);
    wprintf(L"*     Minutes : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Minutes);
    wprintf(L"*     Hours   : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Hours);
    wprintf(L"*     Day     : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Day);
    wprintf(L"*     Month   : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Month);
    wprintf(L"*     Year    : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Year);
    wprintf(L"*     Century : %x\n"      , (ULONG) pErrorRecordHeader->TimeStamp.Century);
    wprintf(L"* OEMPlatformID: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
    					  			 (ULONG) pErrorRecordHeader->OemPlatformId[0],
						             (ULONG) pErrorRecordHeader->OemPlatformId[1],
						             (ULONG) pErrorRecordHeader->OemPlatformId[2],
						             (ULONG) pErrorRecordHeader->OemPlatformId[3],
						             (ULONG) pErrorRecordHeader->OemPlatformId[4],
						             (ULONG) pErrorRecordHeader->OemPlatformId[5],
						             (ULONG) pErrorRecordHeader->OemPlatformId[6],
						             (ULONG) pErrorRecordHeader->OemPlatformId[7],
						             (ULONG) pErrorRecordHeader->OemPlatformId[8],
						             (ULONG) pErrorRecordHeader->OemPlatformId[9],
						             (ULONG) pErrorRecordHeader->OemPlatformId[10],
						             (ULONG) pErrorRecordHeader->OemPlatformId[11],
						             (ULONG) pErrorRecordHeader->OemPlatformId[12],
						             (ULONG) pErrorRecordHeader->OemPlatformId[13],
						             (ULONG) pErrorRecordHeader->OemPlatformId[14],
						             (ULONG) pErrorRecordHeader->OemPlatformId[15]);

	//
	// Now display each of the section headers in the error record.
	//	
    sectionOffset = sizeof(ERROR_RECORD_HEADER);

    while (sectionOffset < pErrorRecordHeader->Length) {

        pErrorSectionHeader = (PERROR_SECTION_HEADER)(PErrorData + sectionOffset);        

        wprintf(L"***************************************************************************\n");
	    wprintf(L"*                             Section Header                              *\n");
    	wprintf(L"***************************************************************************\n");
    	wprintf(L"* GUID: 0x%x, 0x%x, 0x%x, \n",
    								pErrorSectionHeader->Guid.Data1,
    								pErrorSectionHeader->Guid.Data2,
    								pErrorSectionHeader->Guid.Data3);
    	
    	wprintf(L"*       { 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x }\n",
    								pErrorSectionHeader->Guid.Data4[0],
    								pErrorSectionHeader->Guid.Data4[1], 
    								pErrorSectionHeader->Guid.Data4[2],
    								pErrorSectionHeader->Guid.Data4[3],
    								pErrorSectionHeader->Guid.Data4[4], 
    								pErrorSectionHeader->Guid.Data4[5],
    								pErrorSectionHeader->Guid.Data4[6], 
    								pErrorSectionHeader->Guid.Data4[7]);
    	
    	wprintf(L"* Revision: 0x%x\n", pErrorSectionHeader->Revision);
    	wprintf(L"*     Major    : %x\n"	 , (ULONG) pErrorSectionHeader->Revision.Major);
	    wprintf(L"*     Minor    : %x\n"	 , (ULONG) pErrorSectionHeader->Revision.Minor);
	    wprintf(L"* Recovery: 0x%x\n", (UCHAR)pErrorSectionHeader->RecoveryInfo.RecoveryInfo);
	    wprintf(L"* Reserved: 0x%x\n", (UCHAR)pErrorSectionHeader->Reserved);
	    wprintf(L"* Length  : 0x%x\n", (ULONG)pErrorSectionHeader->Length);

        sectionOffset += pErrorSectionHeader->Length;
        
        sectionNumber++;
        
    }    	

    wprintf(L"***************************************************************************\n\n");         
}

#endif // _IA64_

