/*++

Copyright (c) 1990-2003 Microsoft Corporation
All Rights Reserved

Windows Server 2003 (Printing) Driver Development Kit Samples

--*/



#include "pp.h"

HMODULE hmod = NULL;
WCHAR *pszRegistryPath = NULL;
WCHAR *pszRegistryPortNames=L"PortNames";
WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 3];
CRITICAL_SECTION SplSem;
HMODULE hSpoolssDll = NULL;
FARPROC pfnSpoolssEnumPorts = NULL;

static
PRINTPROVIDOR PrintProvidor =
{
    PP_OpenPrinter,
    PP_SetJob,
    PP_GetJob,
    PP_EnumJobs,
    PP_AddPrinter,
    PP_DeletePrinter,
    PP_SetPrinter,
    PP_GetPrinter,
    PP_EnumPrinters,
    PP_AddPrinterDriver,
    PP_EnumPrinterDrivers,
    PP_GetPrinterDriver,
    PP_GetPrinterDriverDirectory,
    PP_DeletePrinterDriver,
    PP_AddPrintProcessor,
    PP_EnumPrintProcessors,
    PP_GetPrintProcessorDirectory,
    PP_DeletePrintProcessor,
    PP_EnumPrintProcessorDatatypes,
    PP_StartDocPrinter,
    PP_StartPagePrinter,
    PP_WritePrinter,
    PP_EndPagePrinter,
    PP_AbortPrinter,
    PP_ReadPrinter,
    PP_EndDocPrinter,
    PP_AddJob,
    PP_ScheduleJob,
    PP_GetPrinterData,
    PP_SetPrinterData,
    PP_WaitForPrinterChange,
    PP_ClosePrinter,
    PP_AddForm,
    PP_DeleteForm,
    PP_GetForm,
    PP_SetForm,
    PP_EnumForms,
    PP_EnumMonitors,
    PP_EnumPorts,
    PP_AddPort,
    PP_ConfigurePort,
    PP_DeletePort,
    PP_CreatePrinterIC,
    PP_PlayGdiScriptOnPrinterIC,
    PP_DeletePrinterIC,
    PP_AddPrinterConnection,
    PP_DeletePrinterConnection,
    PP_PrinterMessageBox,
    PP_AddMonitor,
    PP_DeleteMonitor
};









BOOL DllMain
(
    HINSTANCE hdll,
    DWORD     dwReason,
    LPVOID    lpReserved
)
{
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {


        DisableThreadLibraryCalls(hdll);

        hmod = hdll;



    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {


    }
    return TRUE;
}



DWORD
InitializePortNames
(
    VOID
)

{
    DWORD err;
    HKEY  hkeyPath;
    HKEY  hkeyPortNames;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        pszRegistryPath,
                        0,
                        KEY_READ,
                        &hkeyPath );

    if (!err)
    {
        err = RegOpenKeyEx( hkeyPath,
                            pszRegistryPortNames,
                            0,
                            KEY_READ,
                            &hkeyPortNames );

        if (!err)
        {
            DWORD i = 0;
            WCHAR Buffer[MAX_PATH];
            DWORD BufferSize;

            while (!err)
            {
                BufferSize = sizeof Buffer;

                err = RegEnumValue( hkeyPortNames,
                                    i,
                                    Buffer,
                                    &BufferSize,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL );

                if (!err)
                {
                    CreatePortEntry(Buffer);
                }

                i++;
            }




            if (ERROR_NO_MORE_ITEMS == err)
            {
                err = NO_ERROR;
            }

            RegCloseKey(hkeyPortNames);
        }

        RegCloseKey(hkeyPath);
    }
    return err;
}








BOOL
InitializePrintProvidor
(
    LPPRINTPROVIDOR pPrintProvidor,
    DWORD           cbPrintProvidor,
    LPWSTR          pszFullRegistryPath
)

{
    DWORD dwLen;

    if (!pPrintProvidor || !pszFullRegistryPath || !*pszFullRegistryPath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    memcpy(pPrintProvidor, &PrintProvidor, min(sizeof(PRINTPROVIDOR), cbPrintProvidor));




    if (!(pszRegistryPath = AllocSplStr(pszFullRegistryPath)))
    {
        return FALSE;
    }


    //
    // Try and initialize the crit-sect
    //
    __try
    {
        InitializeCriticalSection(&SplSem);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }


    szMachineName[0] = szMachineName[1] = L'\\';
    dwLen = MAX_COMPUTERNAME_LENGTH;
    GetComputerName(szMachineName + 2, &dwLen);


    InitializePortNames();

    return TRUE;
}


BOOL
PP_OpenPrinter
(
    LPWSTR             pszPrinterName,
    LPHANDLE           phPrinter,
    LPPRINTER_DEFAULTS pDefault
)

{
    DWORD err = NO_ERROR;

    UNREFERENCED_PARAMETER(pDefault);

    if (!pszPrinterName)
    {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}



BOOL
PP_ClosePrinter
(
    HANDLE  hPrinter
)

{
    DWORD err = NO_ERROR;





    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}



BOOL
PP_GetPrinter
(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

{
    DWORD err = NO_ERROR;





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_SetPrinter
(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   dwCommand
)

{
    DWORD err = NO_ERROR;

    UNREFERENCED_PARAMETER(pbPrinter);

    switch (dwLevel)
    {
        case 0:
        case 1:
        case 2:
        case 3:
            break;

        default:
            SetLastError(ERROR_INVALID_LEVEL);
            return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_EnumPrinters
(
    DWORD   dwFlags,
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)

{
    DWORD  err = NO_ERROR;

    if ((dwLevel != 1) && (dwLevel != 2))
    {



        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }
    else if (!pcbNeeded || !pcReturned)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



DWORD
PP_StartDocPrinter
(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  lpbDocInfo
)

{
    DWORD err = NO_ERROR;
    DOC_INFO_1 *pDocInfo1 = (DOC_INFO_1 *)lpbDocInfo;
    LPWSTR pszUser = NULL;
    BOOL fGateway = FALSE;

    if (dwLevel != 1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_WritePrinter
(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcbWritten
)

{
    DWORD err = NO_ERROR;





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_AbortPrinter
(
    HANDLE  hPrinter
)

{
    DWORD err = NO_ERROR;





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);

}



BOOL
PP_EndDocPrinter
(
    HANDLE   hPrinter
)

{
    DWORD err = NO_ERROR;





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_GetJob
(
    HANDLE   hPrinter,
    DWORD    dwJobId,
    DWORD    dwLevel,
    LPBYTE   pbJob,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)

{
    DWORD err = NO_ERROR;

    if ((dwLevel != 1) && (dwLevel != 2))
    {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_EnumJobs
(
    HANDLE  hPrinter,
    DWORD   dwFirstJob,
    DWORD   dwNoJobs,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)

{
    DWORD err = NO_ERROR;

    if ((dwLevel != 1) && (dwLevel != 2))
    {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}



BOOL
PP_SetJob
(
    HANDLE  hPrinter,
    DWORD   dwJobId,
    DWORD   dwLevel,
    LPBYTE  pbJob,
    DWORD   dwCommand
)

{
    DWORD err = NO_ERROR;

    if ((dwLevel != 0) && (dwLevel != 1) && (dwLevel != 2))
    {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }
    else if ((dwLevel == 0) && (pbJob != NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }





    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}



BOOL
PP_AddJob
(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbAddJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

{
    DWORD err = NO_ERROR;

    ADDJOB_INFO_1W TempBuffer;
    PADDJOB_INFO_1W OutputBuffer;
    DWORD OutputBufferSize;

    if (dwLevel != 1)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }








    if (cbBuf < sizeof(ADDJOB_INFO_1W))
    {
        OutputBuffer = &TempBuffer;
        OutputBufferSize = sizeof(ADDJOB_INFO_1W);
    }
    else
    {
        OutputBuffer = (LPADDJOB_INFO_1W) pbAddJob;
        OutputBufferSize = cbBuf;
    }





    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_ScheduleJob
(
    HANDLE  hPrinter,
    DWORD   dwJobId
)

{
    DWORD err = NO_ERROR;




    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



DWORD
PP_WaitForPrinterChange
(
    HANDLE  hPrinter,
    DWORD   dwFlags
)

{
    DWORD err = NO_ERROR;






    if (err)
    {
        SetLastError(err);
        return 0;
    }

    return dwFlags;
}



BOOL
PP_EnumPorts
(
    LPWSTR   pszName,
    DWORD    dwLevel,
    LPBYTE   pbPort,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)

{
    DWORD         err = NO_ERROR;
    DWORD         cb = 0;
    PPORT         pPort;
    WCHAR        *pbEnd;
    PORT_INFO_1W *pInfoStruct;

    if (dwLevel != 1)
    {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }
    else if (!IsLocalMachine(pszName))
    {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    EnterCriticalSection(&SplSem);

    pPort = pFirstPort;
    while (pPort)
    {
        cb += sizeof(PORT_INFO_1W) + (wcslen(pPort->pName)+1)*sizeof(WCHAR);
        pPort = pPort->pNext;
    }

    *pcbNeeded = cb;
    *pcReturned = 0;

    pbEnd = (WCHAR *)(pbPort + cb);
    pInfoStruct = (PORT_INFO_1W *)pbPort;

    if (cb <= cbBuf)
    {
        pPort = pFirstPort;
        while (pPort)
        {
            int cwbName = wcslen(pPort->pName)+1;
            pbEnd -= cwbName;
            memmove((char *)pbEnd, (char *)pPort->pName, cwbName*sizeof(WCHAR));

            pInfoStruct->pName = pbEnd;
            pInfoStruct++;

            pPort = pPort->pNext;
            (*pcReturned)++;
        }
    }
    else
    {
        err = ERROR_INSUFFICIENT_BUFFER;
    }

    LeaveCriticalSection(&SplSem);

    if (err)
    {
        SetLastError(err);
    }

    return (err == NO_ERROR);
}



BOOL
PP_DeletePort
(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszPortName
)

{
    DWORD err = NO_ERROR;
    BOOL fPortDeleted;

    if (!IsLocalMachine(pszName))
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

    fPortDeleted = DeletePortEntry(pszPortName);

    if (fPortDeleted)
    {
        err = DeleteRegistryEntry(pszPortName);
    }
    else
    {
        err = ERROR_UNKNOWN_PORT;
    }

    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}



BOOL
PP_ConfigurePort
(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszPortName
)

{
    if (!IsLocalMachine(pszName))
    {
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }
    else if (!PortKnown(pszPortName))
    {
        SetLastError(ERROR_UNKNOWN_PORT);
        return FALSE;
    }

    return TRUE;
}

BOOL
PP_AddPrinterConnection
(
    LPWSTR  pszPrinterName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_EnumMonitors
(
    LPWSTR   pszName,
    DWORD    dwLevel,
    LPBYTE   pbMonitor,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}


BOOL
PP_AddPort
(
    LPWSTR  pszName,
    HWND    hWnd,
    LPWSTR  pszMonitorName
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}


HANDLE
PP_AddPrinter
(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter
)
{
    LPTSTR     pszPrinterName = NULL;
    LPTSTR     pszPServer = NULL;
    LPTSTR     pszQueue  =  NULL;
    HANDLE     hPrinter = NULL;
    size_t     uSize = 0;
    DWORD      err;
    PPRINTER_INFO_2 pPrinterInfo;

    pPrinterInfo = (PPRINTER_INFO_2)pbPrinter;

    if (dwLevel != 2)
    {
        err = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    uSize = wcslen(((PRINTER_INFO_2 *)pbPrinter)->pPrinterName) + 1;

    if (!(pszPrinterName = (LPTSTR)LocalAlloc(LPTR, uSize * sizeof(WCHAR))))
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    err = StatusFromHResult(StringCchCopy(pszPrinterName, uSize, pPrinterInfo->pPrinterName));

    if (err != ERROR_SUCCESS)
    {
        goto ErrorExit;
    }


    if (  ( !ValidateUNCName(pszPrinterName))
       || ( (pszQueue = wcschr(pszPrinterName + 2, L'\\')) == NULL)
       || ( pszQueue == (pszPrinterName + 2))
       || ( *(pszQueue + 1) == L'\0')
       )
    {
        err =  ERROR_INVALID_NAME;
        goto ErrorExit;
    }

    if (pszPrinterName == NULL)
    {
        SetLastError(ERROR_INVALID_NAME);
        goto ErrorExit;
    }

    if (PortExists(pszPrinterName, &err) && !err)
    {
        SetLastError(ERROR_ALREADY_ASSIGNED);
        goto ErrorExit;
    }

    return hPrinter;

ErrorExit:
    if (!pszPrinterName)
    {
        (void)LocalFree((HLOCAL)pszPrinterName);
    }

    SetLastError(err);
    return (HANDLE)0x0;
}


BOOL
PP_DeletePrinter
(
    HANDLE  hPrinter
)
{
    LPWSTR pszPrinterName = NULL ;
    DWORD err = NO_ERROR;
    DWORD DoesPortExist = FALSE;

    pszPrinterName = (LPWSTR)LocalAlloc(LPTR,sizeof(WCHAR)*MAX_PATH);

    if (pszPrinterName == NULL)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return FALSE;
    }





    if (!err && PortExists(pszPrinterName, &DoesPortExist) && DoesPortExist)
    {
        if (DeleteRegistryEntry(pszPrinterName))
        {
            (void) DeletePortEntry(pszPrinterName);
        }
    }

    if (err)
    {
        SetLastError(err);
    }
    return (err == NO_ERROR);
}


BOOL
PP_DeletePrinterConnection
(
    LPWSTR  pszName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}


BOOL
PP_AddPrinterDriver
(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbPrinter
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_EnumPrinterDrivers
(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_GetPrinterDriver
(
    HANDLE   hPrinter,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_GetPrinterDriverDirectory
(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbDriverDirectory,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_DeletePrinterDriver
(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszDriverName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_AddPrintProcessor
(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszPathName,
    LPWSTR  pszPrintProcessorName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_EnumPrintProcessors
(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbPrintProcessorInfo,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_EnumPrintProcessorDatatypes
(
    LPWSTR   pszName,
    LPWSTR   pszPrintProcessorName,
    DWORD    dwLevel,
    LPBYTE   pbDatatypes,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_GetPrintProcessorDirectory
(
    LPWSTR   pszName,
    LPWSTR   pszEnvironment,
    DWORD    dwLevel,
    LPBYTE   pbPrintProcessorDirectory,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_StartPagePrinter
(
    HANDLE  hPrinter
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_EndPagePrinter
(
    HANDLE  hPrinter
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_ReadPrinter
(
    HANDLE   hPrinter,
    LPVOID   pBuf,
    DWORD    cbBuf,
    LPDWORD  pcbRead
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

DWORD
PP_GetPrinterData
(
    HANDLE   hPrinter,
    LPWSTR   pszValueName,
    LPDWORD  pdwType,
    LPBYTE   pbData,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

DWORD
PP_SetPrinterData
(
    HANDLE  hPrinter,
    LPWSTR  pszValueName,
    DWORD   dwType,
    LPBYTE  pbData,
    DWORD   cbData
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_AddForm
(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  pbForm
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_DeleteForm
(
    HANDLE  hPrinter,
    LPWSTR  pszFormName
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_GetForm
(
    HANDLE   hPrinter,
    LPWSTR   pszFormName,
    DWORD    dwLevel,
    LPBYTE   pbForm,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_SetForm
(
    HANDLE  hPrinter,
    LPWSTR  pszFormName,
    DWORD   dwLevel,
    LPBYTE  pbForm
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_EnumForms
(
    HANDLE   hPrinter,
    DWORD    dwLevel,
    LPBYTE   pbForm,
    DWORD    cbBuf,
    LPDWORD  pcbNeeded,
    LPDWORD  pcReturned
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}


HANDLE
PP_CreatePrinterIC
(
    HANDLE     hPrinter,
    LPDEVMODE  pDevMode
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_PlayGdiScriptOnPrinterIC
(
    HANDLE  hPrinterIC,
    LPBYTE  pbIn,
    DWORD   cbIn,
    LPBYTE  pbOut,
    DWORD   cbOut,
    DWORD   ul
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_DeletePrinterIC
(
    HANDLE  hPrinterIC
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

DWORD
PP_PrinterMessageBox
(
    HANDLE  hPrinter,
    DWORD   dwError,
    HWND    hWnd,
    LPWSTR  pszText,
    LPWSTR  pszCaption,
    DWORD   dwType
)
{
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}

BOOL
PP_AddMonitor
(
    LPWSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pbMonitorInfo
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_DeleteMonitor
(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszMonitorName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

BOOL
PP_DeletePrintProcessor
(
    LPWSTR  pszName,
    LPWSTR  pszEnvironment,
    LPWSTR  pszPrintProcessorName
)
{
    SetLastError(ERROR_INVALID_NAME);
    return FALSE;
}

DWORD
StatusFromHResult(
    IN HRESULT hr
    )
{
    DWORD   Status = ERROR_SUCCESS;

    if (FAILED(hr))
    {
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
        {
            Status = HRESULT_CODE(hr);
        }
        else
        {
            Status = hr;
        }
    }

    return Status;
}

BOOL
BoolFromStatus(
    IN DWORD Status
    )
{
    BOOL bRet = TRUE;

    if (ERROR_SUCCESS != Status)
    {
        SetLastError(Status);

        bRet = FALSE;
    }

    return bRet;
}


