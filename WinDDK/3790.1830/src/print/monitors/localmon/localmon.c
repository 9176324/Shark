/*++

Copyright (c) 1990-2003  Microsoft Corporation
All rights reserved

Module Name:

    localmon.c

--*/

#include "precomp.h"
#pragma hdrstop

#include "lmon.h"
#include "irda.h"


HANDLE LcmhMonitor;
HANDLE LcmhInst;
CRITICAL_SECTION LcmSpoolerSection;
DWORD LocalmonDebug;

DWORD LcmPortInfo1Strings[]={FIELD_OFFSET(PORT_INFO_1, pName),
                          (DWORD)-1};

DWORD LcmPortInfo2Strings[]={FIELD_OFFSET(PORT_INFO_2, pPortName),
                          FIELD_OFFSET(PORT_INFO_2, pMonitorName),
                          FIELD_OFFSET(PORT_INFO_2, pDescription),
                          (DWORD)-1};

WCHAR szPorts[]   = L"ports";
WCHAR gszPorts[]  = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Ports";
WCHAR gszWindows[]= L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows";
WCHAR szPortsEx[] = L"portsex"; /* Extra ports values */
WCHAR szFILE[]    = L"FILE:";
WCHAR szLcmCOM[]     = L"COM";
WCHAR szLcmLPT[]     = L"LPT";
WCHAR szIRDA[]    = L"IR";

extern WCHAR szWindows[];
extern WCHAR szINIKey_TransmissionRetryTimeout[];
extern DWORD g_COMWriteTimeoutConstant_ms;

BOOL
LocalMonInit(HANDLE hModule)
{
    LcmhInst = hModule;

    return InitializeCriticalSectionAndSpinCount(&LcmSpoolerSection,0x80000000);
}

VOID
LocalMonCleanUp(
    VOID
    )
{
    DeleteCriticalSection(&LcmSpoolerSection);
}


BOOL
LcmEnumPorts(
    HANDLE hMonitor,
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;
    PINIPORT pIniPort;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   LastError=0;

    LcmEnterSplSem();

    cb=0;

    pIniPort = pIniLocalMon->pIniPort;

    CheckAndAddIrdaPort(pIniLocalMon);

    while (pIniPort) {

        if ( !(pIniPort->Status & PP_FILEPORT) ) {

            cb+=GetPortSize(pIniPort, Level);
        }
        pIniPort=pIniPort->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort = pIniLocalMon->pIniPort;
        while (pIniPort) {

            if (!(pIniPort->Status & PP_FILEPORT)) {

                pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);

                if( !pEnd ){
                    LastError = GetLastError();
                    break;
                }

                switch (Level) {
                case 1:
                    pPorts+=sizeof(PORT_INFO_1);
                    break;
                case 2:
                    pPorts+=sizeof(PORT_INFO_2);
                    break;
                default:
                    DBGMSG(DBG_ERROR,
                           ("EnumPorts: invalid level %d", Level));
                    LastError = ERROR_INVALID_LEVEL;
                    goto Cleanup;
                }
                (*pcReturned)++;
            }
            pIniPort=pIniPort->pNext;
        }

    } else

        LastError = ERROR_INSUFFICIENT_BUFFER;

Cleanup:
   LcmLeaveSplSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;

    } else

        return TRUE;
}


BOOL
LcmxEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    )
{
    return LcmEnumPorts(LcmhMonitor, pName, Level, pPorts, cbBuf, pcbNeeded, pcReturned);
}

BOOL
LcmOpenPort(
    HANDLE  hMonitor,
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    PINILOCALMON    pIniLocalMon = (PINILOCALMON)hMonitor;
    PINIPORT        pIniPort;
    BOOL            bRet = FALSE;

    LcmEnterSplSem();

    if ( IS_FILE_PORT(pName) ) {

        //
        // We will always create multiple file port
        // entries, so that the spooler can print
        // to multiple files.
        //
        DBGMSG(DBG_TRACE, ("Creating a new pIniPort for %ws\n", pName));
        pIniPort = LcmCreatePortEntry( pIniLocalMon, pName );
        if ( !pIniPort )
            goto Done;

        pIniPort->Status |= PP_FILEPORT;
        *pHandle = pIniPort;
        bRet = TRUE;
        goto Done;
    }

    pIniPort = FindPort(pIniLocalMon, pName);

    if ( !pIniPort )
        goto Done;

    //
    // For LPT ports language monitors could do reads outside Start/End doc
    // port to do bidi even when there are no jobs printing. So we do a
    // CreateFile and keep the handle open all the time.
    //
    // But for COM ports you could have multiple devices attached to a COM
    // port (ex. a printer and some other device with a switch)
    // To be able to use the other device they write a utility which will
    // do a net stop serial and then use the other device. To be able to
    // stop the serial service spooler should not have a handle to the port.
    // So we need to keep handle to COM port open only when there is a job
    // printing
    //
    //
    if ( IS_COM_PORT(pName) ) {

        bRet = TRUE;
        goto Done;
    }

    //
    // If it is not a port redirected we are done (succeed the call)
    //
    if ( ValidateDosDevicePort(pIniPort) ) {

        bRet = TRUE;

        //
        // If it isn't a true dosdevice port (ex. net use lpt1 \\<server>\printer)
        // then we need to do CreateFile and CloseHandle per job so that
        // StartDoc/EndDoc is issued properly for the remote printer
        //
        if ( (pIniPort->Status & PP_DOSDEVPORT) &&
            !(pIniPort->Status & PP_COMM_PORT) ) {

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;

            (VOID)RemoveDosDeviceDefinition(pIniPort);
        }
    }

Done:
    if ( !bRet && pIniPort && (pIniPort->Status & PP_FILEPORT) )
        DeletePortNode(pIniLocalMon, pIniPort);

    if ( bRet )
        *pHandle = pIniPort;

    LcmLeaveSplSem();
    return bRet;
}

BOOL
LcmxOpenPort(
    LPWSTR  pName,
    PHANDLE pHandle
    )
{
    return LcmOpenPort(LcmhMonitor, pName, pHandle);
}

BOOL
LcmStartDocPort(
    HANDLE  hPort,
    LPWSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    DWORD Error = 0;

    DBGMSG(DBG_TRACE, ("StartDocPort(%08x, %ws, %d, %d, %08x)\n",
                       hPort, pPrinterName, JobId, Level, pDocInfo));

    if (pIniPort->Status & PP_STARTDOC) {
        return TRUE;
    }

    LcmEnterSplSem();
    pIniPort->Status |= PP_STARTDOC;
    LcmLeaveSplSem();

    pIniPort->hPrinter = NULL;
    pIniPort->pPrinterName = AllocSplStr(pPrinterName);

    if (pIniPort->pPrinterName) {

        if (OpenPrinter(pPrinterName, &pIniPort->hPrinter, NULL)) {

            pIniPort->JobId = JobId;

            //
            // For COMx port we need to validates dos device now since
            // we do not do it during OpenPort
            //
            if ( IS_COM_PORT(pIniPort->pName) &&
                 !ValidateDosDevicePort(pIniPort) ) {

                goto Fail;
            }

            if ( IS_FILE_PORT(pIniPort->pName) ) {

                HANDLE hFile = INVALID_HANDLE_VALUE;

                if (pDocInfo1                 &&
                    pDocInfo1->pOutputFile    &&
                    pDocInfo1->pOutputFile[0]){

                    hFile = CreateFile( pDocInfo1->pOutputFile,
                                         GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                         NULL );

                    DBGMSG(DBG_TRACE,
                           ("Print to file and the handle is %x\n", hFile));

                }
                else
                {
                    Error = ERROR_INVALID_PARAMETER;
                }

                if (hFile != INVALID_HANDLE_VALUE)
                    SetEndOfFile(hFile);

                pIniPort->hFile = hFile;
            } else if ( IS_IRDA_PORT(pIniPort->pName) ) {

                if ( !IrdaStartDocPort(pIniPort) )
                    goto Fail;
            } else if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

                //
                // For non dosdevices CreateFile on the name of the port
                //
                pIniPort->hFile = CreateFile(pIniPort->pName,
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL  |
                                             FILE_FLAG_SEQUENTIAL_SCAN,
                                             NULL);

                if ( pIniPort->hFile != INVALID_HANDLE_VALUE )
                    SetEndOfFile(pIniPort->hFile);

            } else if ( !IS_COM_PORT(pIniPort->pName) ) {

                if ( !FixupDosDeviceDefinition(pIniPort) )
                    goto Fail;
            }
        }
    } // end of if (pIniPort->pPrinterName)

    if (pIniPort->hFile == INVALID_HANDLE_VALUE)
        goto Fail;

    return TRUE;


Fail:
    SPLASSERT(pIniPort->hFile == INVALID_HANDLE_VALUE);

    LcmEnterSplSem();
    pIniPort->Status &= ~PP_STARTDOC;
    LcmLeaveSplSem();

    if (pIniPort->hPrinter) {
        ClosePrinter(pIniPort->hPrinter);
    }

    if (pIniPort->pPrinterName) {
        FreeSplStr(pIniPort->pPrinterName);
    }

    if (Error)
        SetLastError(Error);

    return FALSE;
}


BOOL
LcmWritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("WritePort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( IS_IRDA_PORT(pIniPort->pName) )
        rc = IrdaWritePort(pIniPort, pBuffer, cbBuf, pcbWritten);
    else if ( !pIniPort->hFile || pIniPort->hFile == INVALID_HANDLE_VALUE ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    } else {

        rc = WriteFile(pIniPort->hFile, pBuffer, cbBuf, pcbWritten, NULL);
        if ( rc && *pcbWritten == 0 ) {

            SetLastError(ERROR_TIMEOUT);
            rc = FALSE;
        }
    }

    DBGMSG(DBG_TRACE, ("WritePort returns %d; %d bytes written\n", rc, *pcbWritten));

    return rc;
}


BOOL
LcmReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuf,
    LPDWORD pcbRead)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("ReadPort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_COMM_PORT) ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    rc = ReadFile(pIniPort->hFile, pBuffer, cbBuf, pcbRead, NULL);

    DBGMSG(DBG_TRACE, ("ReadPort returns %d; %d bytes read\n", rc, *pcbRead));

    return rc;
}

BOOL
LcmEndDocPort(
    HANDLE   hPort
    )
{
    PINIPORT    pIniPort = (PINIPORT)hPort;

    DBGMSG(DBG_TRACE, ("EndDocPort(%08x)\n", hPort));

    if (!(pIniPort->Status & PP_STARTDOC)) {
        return TRUE;
    }

    // The flush here is done to make sure any cached IO's get written
    // before the handle is closed.   This is particularly a problem
    // for Intelligent buffered serial devices

    FlushFileBuffers(pIniPort->hFile);

    //
    // For any ports other than real LPT ports we open during StartDocPort
    // and close it during EndDocPort
    //
    if ( !(pIniPort->Status & PP_COMM_PORT) || IS_COM_PORT(pIniPort->pName) ) {

        if ( IS_IRDA_PORT(pIniPort->pName) ) {

            IrdaEndDocPort(pIniPort);
        } else {

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;

            if ( pIniPort->Status & PP_DOSDEVPORT ) {

                (VOID)RemoveDosDeviceDefinition(pIniPort);
            }

            if ( IS_COM_PORT(pIniPort->pName) ) {

                pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
                FreeSplStr(pIniPort->pDeviceName);
                pIniPort->pDeviceName = NULL;
            }
        }
    }

    SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);

    ClosePrinter(pIniPort->hPrinter);

    FreeSplStr(pIniPort->pPrinterName);

    //
    // Startdoc no longer active.
    //
    pIniPort->Status &= ~PP_STARTDOC;

    return TRUE;
}

BOOL
LcmClosePort(
    HANDLE  hPort
    )
{
    PINIPORT pIniPort = (PINIPORT)hPort;

    FreeSplStr(pIniPort->pDeviceName);
    pIniPort->pDeviceName = NULL;

    if (pIniPort->Status & PP_FILEPORT) {

        LcmEnterSplSem();
        DeletePortNode(pIniPort->pIniLocalMon, pIniPort);
        LcmLeaveSplSem();
    } else if ( pIniPort->Status & PP_COMM_PORT ) {

        (VOID) RemoveDosDeviceDefinition(pIniPort);
        if ( pIniPort->hFile != INVALID_HANDLE_VALUE ) {


            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;
        }
        pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
    }

    return TRUE;
}


BOOL
LcmAddPortEx(
    HANDLE   hMonitor,
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;
    LPWSTR pPortName;
    DWORD  Error;
    LPPORT_INFO_1 pPortInfo1;
    LPPORT_INFO_FF pPortInfoFF;

    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (LPPORT_INFO_FF)pBuffer;
        pPortName = pPortInfoFF->pName;
        break;

    case 1:
        pPortInfo1 =  (LPPORT_INFO_1)pBuffer;
        pPortName = pPortInfo1->pName;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return(FALSE);
    }
    if (!pPortName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (PortExists(pName, pPortName, &Error)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (Error != NO_ERROR) {
        SetLastError(Error);
        return(FALSE);
    }
    if (!LcmCreatePortEntry(pIniLocalMon, pPortName)) {
        return(FALSE);
    }
    if (!AddPortInRegistry (pPortName)) {
        LcmDeletePortEntry( pIniLocalMon, pPortName );
        return(FALSE);
    }
    return TRUE;
}

BOOL
LcmxAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    return LcmAddPortEx(LcmhMonitor, pName, Level, pBuffer, pMonitorName);
}

BOOL
LcmGetPrinterDataFromPort(
    HANDLE  hPort,
    DWORD   ControlID,
    LPWSTR  pValueName,
    LPWSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPWSTR  lpOutBuffer,
    DWORD   cbOutBuffer,
    LPDWORD lpcbReturned)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE,
           ("GetPrinterDataFromPort(%08x, %d, %ws, %ws, %d, ",
           hPort, ControlID, pValueName, lpInBuffer, cbInBuffer));

    if ( !ControlID                                 ||
         !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    rc = DeviceIoControl(pIniPort->hFile,
                         ControlID,
                         lpInBuffer,
                         cbInBuffer,
                         lpOutBuffer,
                         cbOutBuffer,
                         lpcbReturned,
                         NULL);

    DBGMSG(DBG_TRACE,
           ("%ws, %d, %d)\n", lpOutBuffer, cbOutBuffer, lpcbReturned));

    return rc;
}

BOOL
LcmSetPortTimeOuts(
    HANDLE  hPort,
    LPCOMMTIMEOUTS lpCTO,
    DWORD   reserved)    // must be set to 0
{
    PINIPORT        pIniPort = (PINIPORT)hPort;
    COMMTIMEOUTS    cto;

    if (reserved != 0)
        return FALSE;

    if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( GetCommTimeouts(pIniPort->hFile, &cto) )
    {
        cto.ReadTotalTimeoutConstant = lpCTO->ReadTotalTimeoutConstant;
        cto.ReadIntervalTimeout = lpCTO->ReadIntervalTimeout;
        return SetCommTimeouts(pIniPort->hFile, &cto);
    }

    return FALSE;
}

VOID
LcmShutdown(
    HANDLE hMonitor
    )
{
    PINIPORT pIniPort;
    PINIPORT pIniPortNext;
    PINILOCALMON pIniLocalMon = (PINILOCALMON)hMonitor;

    //
    // Delete the ports, then delete the LOCALMONITOR.
    //
    for( pIniPort = pIniLocalMon->pIniPort; pIniPort; pIniPort = pIniPortNext ){
        pIniPortNext = pIniPort->pNext;
        FreeSplMem( pIniPort );
    }

    FreeSplMem( pIniLocalMon );
}


BOOL
LcmxXcvOpenPort(
    LPCWSTR pszObject,
    ACCESS_MASK GrantedAccess,
    PHANDLE phXcv
    )
{
    return LcmXcvOpenPort(LcmhMonitor, pszObject, GrantedAccess, phXcv);
}


MONITOR2 Monitor2 = {
    sizeof(MONITOR2),
    LcmEnumPorts,
    LcmOpenPort,
    NULL,           // OpenPortEx is not supported
    LcmStartDocPort,
    LcmWritePort,
    LcmReadPort,
    LcmEndDocPort,
    LcmClosePort,
    NULL,           // AddPort is not supported
    LcmAddPortEx,
    NULL,           // ConfigurePort is not supported
    NULL,           // DeletePort is not supported
    LcmGetPrinterDataFromPort,
    LcmSetPortTimeOuts,
    LcmXcvOpenPort,
    LcmXcvDataPort,
    LcmXcvClosePort,
    LcmShutdown
};


LPMONITOR2
LocalMonInitializePrintMonitor2(
    PMONITORINIT pMonitorInit,
    PHANDLE phMonitor
    )
{
    LPWSTR   pPortTmp;
    DWORD    dwCharCount=1024, rc, i, j;
    PINILOCALMON pIniLocalMon = NULL;
    LPWSTR   pPorts = NULL;

    if( !pMonitorInit->bLocal ){
        return NULL;
    }

    do {
        FreeSplMem((LPVOID)pPorts);

        dwCharCount *= 2;
        pPorts = (LPWSTR) AllocSplMem(dwCharCount*sizeof(WCHAR));
        if ( !pPorts ) {

            DBGMSG(DBG_ERROR,
                   ("Failed to alloc %d characters for ports\n", dwCharCount));
            goto Fail;
        }

        rc = GetProfileString(szPorts, NULL, szNULL, pPorts, dwCharCount);
        //
        // GetProfileString will does not return the proper character count for long port names.
        // fail the call if the port list length exceeds 1mb
        //
        if ( !rc || dwCharCount >= 1024*1024 ) {

            DBGMSG(DBG_ERROR,
                   ("GetProfilesString failed with %d\n", GetLastError()));
            goto Fail;
        }

    } while ( rc >= dwCharCount - 2 );

    pIniLocalMon = (PINILOCALMON)AllocSplMem( sizeof( INILOCALMON ));

    if( !pIniLocalMon ){
        goto Fail;
    }

    pIniLocalMon->signature = ILM_SIGNATURE;
    pIniLocalMon->pMonitorInit = pMonitorInit;

    //
    // dwCharCount is now the count of return buffer, not including
    // the NULL terminator.  When we are past pPorts[rc], then
    // we have parsed the entire string.
    //
    dwCharCount = rc;

   LcmEnterSplSem();

    //
    // We now have all the ports
    //
    for( j = 0; j <= dwCharCount; j += rc + 1 ){

        pPortTmp = pPorts + j;

        rc = wcslen(pPortTmp);

        if( !rc ){
            continue;
        }

        if (!_wcsnicmp(pPortTmp, L"Ne", 2)) {

            i = 2;
            //
            // For Ne-ports
            //
            if ( rc > 2 && pPortTmp[2] == L'-' )
                ++i;
            for ( ; i < rc - 1 && iswdigit(pPortTmp[i]) ; ++i )
            ;

            if ( i == rc - 1 && pPortTmp[rc-1] == L':' ) {
                continue;
            }
        }

        LcmCreatePortEntry(pIniLocalMon, pPortTmp);
    }

    FreeSplMem(pPorts);

    LcmLeaveSplSem();

    CheckAndAddIrdaPort(pIniLocalMon);

    *phMonitor = (HANDLE)pIniLocalMon;

    return &Monitor2;

Fail:

    FreeSplMem( pPorts );
    FreeSplMem( pIniLocalMon );

    return NULL;
}


BOOL
DllMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        LocalMonInit(hModule);
        DisableThreadLibraryCalls(hModule);
        return TRUE;

    case DLL_PROCESS_DETACH:

        LocalMonCleanUp();
        return TRUE;
    }

    UNREFERENCED_PARAMETER(lpRes);

    return TRUE;
}

