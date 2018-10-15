/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    Mca.cpp

Abstract:

    This module is the entry point of the management application. It basically
    handles all user interactions and starts the appropriate operation.
    
Author:

    Abdullah Ustuner (AUstuner) 30-August-2002
        
--*/

#include "mca.h"


typedef enum {
    DISPLAY_USAGE,
    QUERY_FATAL,
    QUERY_CORRECTED
} OPERATION, *POPERATION;

//
// Enumeration which indicates the current operation of the application.
//
OPERATION gOperation = DISPLAY_USAGE;

//
// TimeOut period for corrected error retrieval.
//
extern INT gTimeOut;

INT __cdecl
wmain(
    INT ArgumentCount,
    PWCHAR ArgumentList[]
    )
/*++

Routine Description:

    This function is the main entry point of the application. According to the
    results of the argument-parsing, the appropriate action is taken by this
    function.

Arguments:

    ArgumentCount - Number of command-line arguments passed to the executable
    				(including the executable name).
    
    ArgumentList - Pointer to the actual command-line parameters.

Return Value:

    0  - Successful.
    Otherwise - Unsuccessful.

 --*/
{  
	//
	// Validate the command-line usage of the application.
	//
	if (!MCAParseArguments(ArgumentCount, ArgumentList)) {

		wprintf(L"Incorrect command-line usage!\n");
		wprintf(L"Run as: \"mcamgmt /?\" to view the correct usage.\n");

		return -1;
		
	}	

	MCAPrintTitle();

	//
	// Start the appropriate operation. 
	//
	switch(gOperation){

		case DISPLAY_USAGE: {
			
			MCAPrintUsage();

			return 0;							

		}
			
		case QUERY_FATAL: {

			if (!MCAGetFatalError()) {

				wprintf(L"ERROR: Fatal error retrieval failed!\n");

				return -1;
				
			}

			wprintf(L"INFO: Fatal error retrieval completed successfully.\n");

			return 0;
			
		}
		
		case QUERY_CORRECTED: {

			if (!MCAGetCorrectedError()) {

				wprintf(L"ERROR: Corrected error retrieval failed!\n");

				return -1;
			}

			wprintf(L"INFO: Corrected error retrieval completed successfully.\n");

			return 0;
			
		}
		
	}
   
}


BOOL
MCAParseArguments(
	IN INT ArgumentCount,
	IN PWCHAR ArgumentList[]
	)
/*++

Routine Description:

    This function examines the command line arguments. If the arguments are inaccurate
    then, it returns FALSE. If the arguments are correct then, the operation of the
    tool is set appropriately.
    
Arguments:

    ArgumentCount - Number of command-line arguments passed to the executable
    (including the executable name).
    
    ArgumentList - Pointer to the actual command-line parameters.

Return Value:

    TRUE  - Command-line arguments are correct and tool operation is set successfully.
    FALSE - Incorrect command-line usage. 

 --*/
{
	//
	// Check if number of command-line arguments are in the expected range.
	//
	if (ArgumentCount < 2 || ArgumentCount > 3) {

		return FALSE;
		
	}

	//
	// If switch is "/?" or "/usage".
	//
	if (_wcsicmp(ArgumentList[1], L"/?") == 0 || 
		_wcsicmp(ArgumentList[1], L"/usage") == 0) {

		gOperation = DISPLAY_USAGE;

		return TRUE;
		
	}

	//
	// If switch is "/fatal".
	//
	if (_wcsicmp(ArgumentList[1], L"/fatal") == 0) {

		gOperation = QUERY_FATAL;

		return TRUE;
		
	}

	//
	// If switch is "/corrected".
	//
	if (_wcsicmp(ArgumentList[1], L"/corrected") == 0) {		

		gTimeOut = _wtoi(ArgumentList[2]);

		//
		// The <TimeOut> period should be a positive integer not larger than the
		// maximum time out value, which is predefined.
		//
		if (gTimeOut <= 0 || gTimeOut > TIME_OUT_MAX) {

			wprintf(L"<TimeOut> must be a positive integer with a maximum value of 60(minutes)!\n");

			return FALSE;
			
		}

		gOperation = QUERY_CORRECTED;

		return TRUE;
		
	}

	return FALSE;	
}


VOID
MCAPrintUsage(
	VOID
	)
/*++

Routine Description:

    This function displays the command-line usage of the application on the
    standard output.
    
Arguments:

    none

Return Value:

    none

 --*/
{
	wprintf(L"\n--------------------------- COMMAND-LINE USAGE ----------------------------\n");
    wprintf(L"Usage:\n\n");
    
    wprintf(L"MCAMGMT (/fatal | (/corrected <TimeOut>) | /usage | /?)\n\n");   	
   	
    wprintf(L"    /fatal                Queries the operating system for fatal machine\n");
    wprintf(L"                          check error and retrieves if one exists.\n");
    wprintf(L"    /corrected <TimeOut>  Registers to the operating system for corrected\n");
    wprintf(L"                          machine check error notification and waits for\n");
    wprintf(L"                          <TimeOut> minutes to retrieve an error. <TimeOut>\n");
    wprintf(L"                          can be maximum 60 (minutes).\n");
    wprintf(L"    /usage or /?          Displays the command-line usage.\n\n");
    
    wprintf(L"SAMPLES:\n\n");

    wprintf(L"MCAMGMT /fatal\n\n");
    
    wprintf(L"MCAMGMT /corrected 10\n\n");

    wprintf(L"MCAMGMT /usage\n\n");

    wprintf(L"---------------------------------------------------------------------------\n");    
}


VOID
MCAPrintTitle(
    VOID
    )
/*++

Routine Description:

    This function displays the title of the application on the standard output.
    The title provides an abstract definition of the functionality of the application.
    
Arguments:

    none

Return Value:

    none

 --*/
{
    wprintf(L"*************************************************************************\n");
    wprintf(L"*                       MCA Management Application                      *\n");
    wprintf(L"*-----------------------------------------------------------------------*\n");
    wprintf(L"* Queries the operating system for machine check events and in case     *\n");
   	wprintf(L"* of existence then, retrieves the corresponding MCA event information, *\n");
    wprintf(L"* parses and displays it to the console screen. On IA64 platforms only  *\n");
    wprintf(L"* the error record header and section header contents are displayed     *\n");
    wprintf(L"* whereas on X86 and AMD64 platforms the complete MCA exception         *\n");
    wprintf(L"* information is displayed.                                             *\n");
    wprintf(L"*************************************************************************\n\n");    
}

