//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1998 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:   dllentry.cpp
//    
//
//  PURPOSE:  Source module for DLL entry function(s).
//
//
//  Functions:
//          DllMain
//
//
//
//  PLATFORMS:  Windows XP, Windows Server 2003
//
//
//  History: 
//          06/24/03    xxx created.
//
//

#include "precomp.h"
#include "bitmap.h"
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

// DllMain isn't called/used for kernel mode version.
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
    OEMDBG(DBG_VERBOSE, L"DllMain entry.");
    
    switch(wReason)
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}





}  // extern "C" closing bracket



