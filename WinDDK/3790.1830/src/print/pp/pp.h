/*++ 

Copyright (c) 1990-2003 Microsoft Corporation
All Rights Reserved

Windows Server 2003 (Printing) Driver Development Kit Samples

--*/



#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <strsafe.h>

BOOL PP_OpenPrinter
(
    LPWSTR             pPrinterName,
    LPHANDLE           phPrinter,
    LPPRINTER_DEFAULTS pDefault
);

BOOL PP_SetJob
(
    HANDLE hPrinter,
    DWORD  JobId,
    DWORD  Level,
    LPBYTE pJob,
    DWORD  Command
);

BOOL PP_GetJob
(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_EnumJobs
(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

HANDLE PP_AddPrinter
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter
);

BOOL PP_DeletePrinter
(
    HANDLE  hPrinter
);

BOOL PP_SetPrinter
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
);

BOOL PP_GetPrinter
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_EnumPrinters
(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_AddPrinterDriver
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
);

BOOL PP_EnumPrinterDrivers
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_GetPrinterDriver
(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_GetPrinterDriverDirectory
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_DeletePrinterDriver
(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName
);

BOOL PP_AddPrintProcessor
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
);

BOOL PP_EnumPrintProcessors
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_GetPrintProcessorDirectory
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_DeletePrintProcessor
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
);

BOOL PP_EnumPrintProcessorDatatypes
(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDataypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

DWORD PP_StartDocPrinter
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL PP_StartPagePrinter
(
    HANDLE  hPrinter
);

BOOL PP_WritePrinter
(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

BOOL PP_EndPagePrinter
(
    HANDLE  hPrinter
);

BOOL PP_AbortPrinter
(
    HANDLE  hPrinter
);

BOOL PP_ReadPrinter
(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
);

BOOL PP_EndDocPrinter
(
    HANDLE  hPrinter
);

BOOL PP_AddJob
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_ScheduleJob
(
    HANDLE  hPrinter,
    DWORD   JobId
);

DWORD PP_GetPrinterData
(
    HANDLE   hPrinter,
    LPWSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
);

DWORD PP_SetPrinterData
(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);

DWORD PP_WaitForPrinterChange
(
    HANDLE hPrinter,
    DWORD  Flags
);

BOOL PP_ClosePrinter
(
    HANDLE hPrinter
);

BOOL PP_AddForm
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
);

BOOL PP_DeleteForm
(
    HANDLE  hPrinter,
    LPWSTR  pFormName
);

BOOL PP_GetForm
(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PP_SetForm
(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm
);

BOOL PP_EnumForms
(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_EnumMonitors
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_EnumPorts
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL PP_AddPort
(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pMonitorName
);

BOOL PP_ConfigurePort
(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
);

BOOL PP_DeletePort
(
    LPWSTR  pName,
    HWND    hWnd,
    LPWSTR  pPortName
);

HANDLE PP_CreatePrinterIC
(
    HANDLE     hPrinter,
    LPDEVMODEW pDevMode
);

BOOL PP_PlayGdiScriptOnPrinterIC
(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
);

BOOL PP_DeletePrinterIC
(
    HANDLE  hPrinterIC
);

BOOL PP_AddPrinterConnection
(
    LPWSTR  pName
);

BOOL PP_DeletePrinterConnection
(
    LPWSTR pName
);

DWORD PP_PrinterMessageBox
(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
);

BOOL PP_AddMonitor
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
);

BOOL PP_DeleteMonitor
(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
);

BOOL PP_ResetPrinter
(
    HANDLE             hPrinter,
    LPPRINTER_DEFAULTS pDefault
);

BOOL PP_GetPrinterDriverEx
(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
);

BOOL PP_FindFirstPrinterChangeNotification
(
    HANDLE hPrinter,
    DWORD  fdwFlags,
    DWORD  fdwOptions,
    HANDLE hNotify,
    PDWORD pfdwStatus,
    PVOID  pPrinterNotifyOptions,
    PVOID  pPrinterNotifyInit
);

BOOL PP_FindClosePrinterChangeNotification
(
    HANDLE hPrinter
);


BOOL PP_AddPortEx
(
    LPWSTR pName,
    DWORD  Level,
    LPBYTE lpBuffer,
    LPWSTR lpMonitorName
);

BOOL PP_ShutDown
(
    LPVOID pvReserved
);

BOOL PP_RefreshPrinterChangeNotification
(
    HANDLE hPrinter,
    DWORD  Reserved,
    PVOID  pvReserved,
    PVOID  pPrinterNotifyInfo
);

BOOL PP_OpenPrinterEx
(
    LPWSTR             pPrinterName,
    LPHANDLE           phPrinter,
    LPPRINTER_DEFAULTS pDefault,
    LPBYTE             pClientInfo,
    DWORD              Level
);

HANDLE PP_AddPrinterEx
(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pClientInfo,
    DWORD   ClientInfoLevel
);

BOOL PP_SetPort
(
    LPWSTR  pName,
    LPWSTR  pPortName,
    DWORD   Level,
    LPBYTE  pPortInfo
);

DWORD PP_EnumPrinterData
(
    HANDLE  hPrinter,
    DWORD   dwIndex,
    LPWSTR  pValueName,
    DWORD   cbValueName,
    LPDWORD pcbValueName,
    LPDWORD pType,
    LPBYTE  pData,
    DWORD   cbData,
    LPDWORD pcbData
);

DWORD PP_DeletePrinterData
(
    HANDLE   hPrinter,
    LPWSTR   pValueName
);







typedef struct _PORT
{
    DWORD   cb;
    struct  _PORT *pNext;
    LPWSTR  pName;
} PORT, *PPORT;

DWORD
InitializePortNames
(
    VOID
);

VOID pFreeAllContexts
(
    VOID
);

DWORD DeleteRegistryEntry
(
    LPWSTR pPortName
);

PPORT CreatePortEntry
(
    LPWSTR pPortName
);

BOOL DeletePortEntry
(
    LPWSTR pPortName
);

LPWSTR AllocSplStr
(
    LPWSTR pStr
);

BOOL IsLocalMachine
(
    LPWSTR pszName
);

BOOL PortKnown
(
    LPWSTR pPortName
);

BOOL ValidateUNCName
(
    LPWSTR pName
);

BOOL PortExists
(
    LPWSTR pPortName,
    LPDWORD pError
);

DWORD
StatusFromHResult
(
    IN HRESULT hr
);

BOOL
BoolFromStatus
(
    IN DWORD Status
);

extern HMODULE hmod;
extern HMODULE hSpoolssDll;

extern CRITICAL_SECTION SplSem;

extern FARPROC pfnSpoolssEnumPorts;

extern WCHAR *pszRegistryPath;
extern WCHAR *pszRegistryPortNames;
extern WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 3];

extern PPORT pFirstPort;



