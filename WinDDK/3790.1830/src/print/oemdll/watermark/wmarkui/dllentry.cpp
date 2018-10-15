//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1998 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:	dllentry.cpp
//    
//
//  PURPOSE:  Source module for DLL entry function(s).
//
//
//	Functions:
//
//		DllMain
//
//
//  PLATFORMS:	Windows 2000, Windows XP, Windows Server 2003
//
//

#include "precomp.h"
#include "wmarkui.h"
#include "debug.h"

// StrSafe.h needs to be included last
// to disallow bad string functions.
#include <STRSAFE.H>



// Need to export these functions as c declarations.
extern "C" {


///////////////////////////////////////////////////////////
//
// DLL entry point
//
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
	switch(wReason)
	{
		case DLL_PROCESS_ATTACH:
            VERBOSE(DLLTEXT("Process attach.\r\n"));
            ghInstance = hInst;
            break;

		case DLL_THREAD_ATTACH:
            VERBOSE(DLLTEXT("Thread attach.\r\n"));
			break;

		case DLL_PROCESS_DETACH:
            VERBOSE(DLLTEXT("Process detach.\r\n"));
			break;

		case DLL_THREAD_DETACH:
            VERBOSE(DLLTEXT("Thread detach.\r\n"));
			break;
	}

	return TRUE;
}


}  // extern "C" closing bracket



