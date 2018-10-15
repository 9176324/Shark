/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    plotinit.c


Abstract:

    This module contains plotter UI dll entry point



Development History:

    18-Nov-1993 Thu 07:12:52 created  

    01-Nov-1995 Wed 10:29:33 updated  
        Re-write for the SUR common UI

[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPlotUI


#define DBG_PROCESS_ATTACH  0x00000001
#define DBG_PROCESS_DETACH  0x00000002

DEFINE_DBGVAR(0);




#if DBG
TCHAR   DebugDLLName[] = TEXT("PLOTUI");
#endif


HMODULE     hPlotUIModule = NULL;



BOOL
DllMain(
    HINSTANCE   hModule,
    DWORD       Reason,
    LPVOID      pReserved
    )

/*++

Routine Description:

    This is the DLL entry point


Arguments:

    hMoudle     - handle to the module for this function

    Reason      - The reason called

    pReserved   - Not used, do not touch


Return Value:

    BOOL, we will always return ture and never failed this function


Development History:

    15-Dec-1993 Wed 15:05:56 updated  
        Add the DestroyCachedData()

    18-Nov-1993 Thu 07:13:56 created  


Revision History:


--*/

{

    UNREFERENCED_PARAMETER(pReserved);

    switch (Reason) {

    case DLL_PROCESS_ATTACH:

        PLOTDBG(DBG_PROCESS_ATTACH,
                ("PlotUIDLLEntryFunc: DLL_PROCESS_ATTACH: hModule = %08lx",
                                                                    hModule));
        hPlotUIModule = hModule;

        //
        // Initialize GPC data cache
        //

        if (!InitCachedData())
            return FALSE;

        break;

    case DLL_PROCESS_DETACH:

        //
        // Free up all the memory used by this module
        //

        PLOTDBG(DBG_PROCESS_DETACH,
                ("PlotUIDLLEntryFunc: DLL_PROCESS_DETACH Destroy CACHED Data"));

        DestroyCachedData();
        break;
    }

    return(TRUE);
}

