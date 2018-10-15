/*++

Copyright (c) 1998-2003  Microsoft Corporation
All rights reserved

Module Name:

    local.h

--*/

#ifndef _LOCAL_H_
#define _LOCAL_H_

typedef long NTSTATUS;

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"


#include <winddiui.h>

typedef struct _pfnWinSpoolDrv {
    BOOL    (*pfnOpenPrinter)(LPTSTR, LPHANDLE, LPPRINTER_DEFAULTS);
    BOOL    (*pfnClosePrinter)(HANDLE);
    BOOL    (*pfnDevQueryPrint)(HANDLE, LPDEVMODE, DWORD *, LPWSTR, DWORD);
    BOOL    (*pfnPrinterEvent)(LPWSTR, INT, DWORD, LPARAM, DWORD *);
    LONG    (*pfnDocumentProperties)(HWND, HANDLE, LPWSTR, PDEVMODE, PDEVMODE, DWORD);
    HANDLE  (*pfnLoadPrinterDriver)(HANDLE);
    BOOL    (*pfnSetDefaultPrinter)(LPCWSTR);
    BOOL    (*pfnGetDefaultPrinter)(LPWSTR, LPDWORD);
    HANDLE  (*pfnRefCntLoadDriver)(LPWSTR, DWORD, DWORD, BOOL);
    BOOL    (*pfnRefCntUnloadDriver)(HANDLE, BOOL);
    BOOL    (*pfnForceUnloadDriver)(LPWSTR);
}   fnWinSpoolDrv, *pfnWinSpoolDrv;


BOOL
SplInitializeWinSpoolDrv(
    pfnWinSpoolDrv   pfnList
    );

BOOL
GetJobAttributes(
    LPWSTR            pPrinterName,
    LPDEVMODEW        pDevmode,
    PATTRIBUTE_INFO_3 pAttributeInfo
    );


#define LOG_ERROR   EVENTLOG_ERROR_TYPE

LPWSTR AllocSplStr(LPWSTR pStr);
LPVOID AllocSplMem(DWORD cbAlloc);
LPVOID ReallocSplMem(   LPVOID pOldMem, 
                        DWORD cbOld, 
                        DWORD cbNew);


#define FreeSplMem( pMem )        (GlobalFree( pMem ) ? FALSE:TRUE)
#define FreeSplStr( lpStr )       ((lpStr) ? (GlobalFree(lpStr) ? FALSE:TRUE):TRUE)


//
//  DEBUGGING:
//

#if DBG


BOOL
DebugPrint(
    PCH pszFmt,
    ...
    );
  
//
// ODS - OutputDebugString 
//
#define ODS( MsgAndArgs )       \
    do {                        \
        DebugPrint  MsgAndArgs;   \
    } while(0)  

#else
//
// No debugging
//
#define ODS(x)
#endif             // DBG


#endif

