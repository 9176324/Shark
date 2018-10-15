/*++

Copyright (c) 1990-2003 Microsoft Corporation
All Rights Reserved

Module Name:

    util.c

--*/

#include "precomp.h"
#pragma hdrstop


//
// These globals are needed so that AddPort can call
// SPOOLSS!EnumPorts to see whether the port to be added
// already exists.

HMODULE hSpoolssDll = NULL;
FARPROC pfnSpoolssEnumPorts = NULL;

VOID
LcmRemoveColon(
    LPWSTR  pName)
{
    DWORD   Length;

    Length = wcslen(pName);

    if (pName[Length-1] == L':')
        pName[Length-1] = 0;
}


BOOL
IsCOMPort(
    LPWSTR pPort
)
{
    //
    // Must begin with szLcmCOM
    //
    if ( _wcsnicmp( pPort, szLcmCOM, 3 ) )
    {
        return FALSE;
    }

    //
    // wcslen guarenteed >= 3
    //
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}

BOOL
IsLPTPort(
    LPWSTR pPort
)
{
    //
    // Must begin with szLcmLPT
    //
    if ( _wcsnicmp( pPort, szLcmLPT, 3 ) )
    {
        return FALSE;
    }

    //
    // wcslen guarenteed >= 3
    //
    return pPort[ wcslen( pPort ) - 1 ] == L':';
}




#define NEXTVAL(pch)                    \
    while( *pch && ( *pch != L',' ) )    \
        pch++;                          \
    if( *pch )                          \
        pch++


BOOL
GetIniCommValues(
    LPWSTR          pName,
    LPDCB          pdcb,
    LPCOMMTIMEOUTS pcto
)
{
    BOOL    bRet = TRUE;
    LPWSTR  pszEntry = NULL;

    if (bRet)
    {
        bRet = GetIniCommValuesFromRegistry (pName, &pszEntry);
    }
    if (bRet)
    {
        bRet =  BuildCommDCB (pszEntry, pdcb);
    }
    if (bRet)
    {
        GetTransmissionRetryTimeoutFromRegistry (&pcto-> WriteTotalTimeoutConstant);
        pcto->WriteTotalTimeoutConstant*=1000;
    }
    FreeSplMem(pszEntry);
    return bRet;
}


/* PortExists
 *
 * Calls EnumPorts to check whether the port name already exists.
 * This asks every monitor, rather than just this one.
 * The function will return TRUE if the specified port is in the list.
 * If an error occurs, the return is FALSE and the variable pointed
 * to by pError contains the return from GetLastError().
 * The caller must therefore always check that *pError == NO_ERROR.
 */
BOOL
PortExists(
    LPWSTR pName,
    LPWSTR pPortName,
    PDWORD pError
)
{
    DWORD cbNeeded;
    DWORD cReturned;
    DWORD cbPorts;
    LPPORT_INFO_1 pPorts;
    DWORD i;
    BOOL  Found = TRUE;

    *pError = NO_ERROR;

    if (!hSpoolssDll) {

        hSpoolssDll = LoadLibrary(L"SPOOLSS.DLL");

        if (hSpoolssDll) {
            pfnSpoolssEnumPorts = GetProcAddress(hSpoolssDll,
                                                 "EnumPortsW");
            if (!pfnSpoolssEnumPorts) {

                *pError = GetLastError();
                FreeLibrary(hSpoolssDll);
                hSpoolssDll = NULL;
            }

        } else {

            *pError = GetLastError();
        }
    }

    if (!pfnSpoolssEnumPorts)
        return FALSE;


    if (!(*pfnSpoolssEnumPorts)(pName, 1, NULL, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            cbPorts = cbNeeded;

            pPorts = AllocSplMem(cbPorts);

            if (pPorts)
            {
                if ((*pfnSpoolssEnumPorts)(pName, 1, (LPBYTE)pPorts, cbPorts,
                                           &cbNeeded, &cReturned))
                {
                    Found = FALSE;

                    for (i = 0; i < cReturned; i++)
                    {
                        if (!lstrcmpi(pPorts[i].pName, pPortName))
                            Found = TRUE;
                    }
                }
            }

            FreeSplMem(pPorts);
        }
    }

    else
        Found = FALSE;


    return Found;
}


VOID
LcmSplInSem(
   VOID
)
{
    if (LcmSpoolerSection.OwningThread != (HANDLE) UIntToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Not in spooler semaphore\n"));
    }
}

VOID
LcmSplOutSem(
   VOID
)
{
    if (LcmSpoolerSection.OwningThread == (HANDLE) UIntToPtr(GetCurrentThreadId())) {
        DBGMSG(DBG_ERROR, ("Inside spooler semaphore !!\n"));
    }
}

VOID
LcmEnterSplSem(
   VOID
)
{
    EnterCriticalSection(&LcmSpoolerSection);
}

VOID
LcmLeaveSplSem(
   VOID
)
{
#if DBG
    LcmSplInSem();
#endif
    LeaveCriticalSection(&LcmSpoolerSection);
}

PINIENTRY
LcmFindName(
   PINIENTRY pIniKey,
   LPWSTR pName
)
{
    if (pName) {
        while (pIniKey) {

            if (!lstrcmpi(pIniKey->pName, pName)) {
                return pIniKey;
            }

            pIniKey=pIniKey->pNext;
        }
    }

    return FALSE;
}

PINIENTRY
LcmFindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR pName
)
{
   if (!pName)
      return NULL;

   LcmSplInSem();

   while (pIniEntry && lstrcmpi(pName, pIniEntry->pName))
      pIniEntry = pIniEntry->pNext;

   return pIniEntry;
}

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
)
{
   while (*DestOffsets != -1) {
      if (*pSource) {
          size_t cbString = wcslen(*pSource)*sizeof(WCHAR) + sizeof(WCHAR);
         pEnd-= cbString;
         StringCbCopy ((LPWSTR) pEnd, cbString, *pSource);;
         *(LPWSTR UNALIGNED *)(pDest+*DestOffsets)= (LPWSTR) pEnd;
      } else
         *(LPWSTR UNALIGNED *)(pDest+*DestOffsets)=0;
      pSource++;
      DestOffsets++;
   }

   return pEnd;
}


/* LcmMessage
 *
 * Displays a LcmMessage by loading the strings whose IDs are passed into
 * the function, and substituting the supplied variable argument list
 * using the varargs macros.
 *
 */
int LcmMessage(HWND hwnd, DWORD Type, int CaptionID, int TextID, ...)
{
    WCHAR   MsgText[256];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( LcmhInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( LcmhInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        StringCchVPrintf (MsgText, COUNTOF (MsgText), MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}


/*
 *
 */
LPTSTR
LcmGetErrorString(
    DWORD   Error
)
{
    TCHAR   Buffer[1024];
    LPTSTR  pErrorString = NULL;

    if( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, Error, 0, Buffer,
                       COUNTOF(Buffer), NULL )
      == 0 )

        LoadString( LcmhInst, IDS_UNKNOWN_ERROR,
                    Buffer, COUNTOF(Buffer) );

    pErrorString = AllocSplStr(Buffer);

    return pErrorString;
}




DWORD ReportError( HWND  hwndParent,
                   DWORD idTitle,
                   DWORD idDefaultError )
{
    DWORD  ErrorID;
    DWORD  MsgType;
    LPTSTR pErrorString;

    ErrorID = GetLastError( );

    if( ErrorID == ERROR_ACCESS_DENIED )
        MsgType = MSG_INFORMATION;
    else
        MsgType = MSG_ERROR;


    pErrorString = LcmGetErrorString( ErrorID );

    LcmMessage( hwndParent, MsgType, idTitle,
             idDefaultError, pErrorString );

    FreeSplStr( pErrorString );


    return ErrorID;
}



LPWSTR
AllocSplStr(
    LPWSTR pStr
    )

/*++

Routine Description:

    This function will allocate enough local memory to store the specified
    string, and copy that string to the allocated memory

Arguments:

    pStr - Pointer to the string that needs to be allocated and stored

Return Value:

    NON-NULL - A pointer to the allocated memory containing the string

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}


LPVOID
AllocSplMem(
    DWORD cbAlloc
    )

{
    PVOID pvMemory;

    pvMemory = GlobalAlloc(GMEM_FIXED, cbAlloc);

    if( pvMemory ){
        ZeroMemory( pvMemory, cbAlloc );
    }

    return pvMemory;
}

DWORD
WINAPIV
StrNCatBuffW(
    IN      PWSTR       pszBuffer,
    IN      UINT        cchBuffer,
    ...
    )
/*++

Description:

    This routine concatenates a set of null terminated strings
    into the provided buffer.  The last argument must be a NULL
    to signify the end of the argument list.  This only called
        from LocalMon by functions that use WCHARS.

Arguments:

    pszBuffer  - pointer buffer where to place the concatenated
                 string.
    cchBuffer  - character count of the provided buffer including
                 the null terminator.
    ...        - variable number of string to concatenate.

Returns:

    ERROR_SUCCESS if new concatenated string is returned,
    or ERROR_XXX if an error occurred.

Notes:

    The caller must pass valid strings as arguments to this routine,
    if an integer or other parameter is passed the routine will either
    crash or fail abnormally.  Since this is an internal routine
    we are not in try except block for performance reasons.

--*/
{
    DWORD   dwRetval    = ERROR_INVALID_PARAMETER;
    PCWSTR  pszTemp     = NULL;
    PWSTR   pszDest     = NULL;
    va_list pArgs;

    //
    // Validate the pointer where to return the buffer.
    //
    if (pszBuffer && cchBuffer)
    {
        //
        // Assume success.
        //
        dwRetval = ERROR_SUCCESS;

        //
        // Get pointer to argument frame.
        //
        va_start(pArgs, cchBuffer);

        //
        // Get temp destination pointer.
        //
        pszDest = pszBuffer;

        //
        // Insure we have space for the null terminator.
        //
        cchBuffer--;

        //
        // Collect all the arguments.
        //
        for ( ; ; )
        {
            //
            // Get pointer to the next argument.
            //
            pszTemp = va_arg(pArgs, PCWSTR);

            if (!pszTemp)
            {
                break;
            }

            //
            // Copy the data into the destination buffer.
            //
            for ( ; cchBuffer; cchBuffer-- )
            {
                if (!(*pszDest = *pszTemp))
                {
                    break;
                }

                pszDest++, pszTemp++;
            }

            //
            // If were unable to write all the strings to the buffer,
            // set the error code and nuke the incomplete copied strings.
            //
            if (!cchBuffer && pszTemp && *pszTemp)
            {
                dwRetval = ERROR_BUFFER_OVERFLOW;
                *pszBuffer = L'\0';
                break;
            }
        }

        //
        // Terminate the buffer always.
        //
        *pszDest = L'\0';

        va_end(pArgs);
    }

    //
    // Set the last error in case the caller forgets to.
    //
    if (dwRetval != ERROR_SUCCESS)
    {
        SetLastError(dwRetval);
    }

    return dwRetval;

}

/* PortIsValid
 *
 * Validate the port by attempting to create/open it.
 */
BOOL
PortIsValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

    //
    // For COM and LPT ports, no verification
    //
    if ( IS_COM_PORT( pPortName ) ||
        IS_LPT_PORT( pPortName ) ||
        IS_FILE_PORT( pPortName ) )
    {
        return TRUE;
    }

    hFile = CreateFile(pPortName,
                       GENERIC_WRITE,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = CreateFile(pPortName,
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           OPEN_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                           NULL);
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        Valid = TRUE;
    } else {
        Valid = FALSE;
    }

    return Valid;
}

BOOL
GetIniCommValuesFromRegistry (
    IN  LPCWSTR     pszPortName,
    OUT LPWSTR*     ppszCommValues
    )
/*++

Description:
    This function returns the communication parameters from the Registry at
    HKLM\Software\Microsoft\Windows NT\CurrentVersion\Ports:<pszPortName>
    of the specified port in the format: "921600,8,n,1.5,x"

Arguments:
    pszPortName     - [in]  the name of the specific port
    ppszCommValues  - [out] address of the pointer that will point to the string
                            Note, if the function succeeds the caller is responsible for memory deletion by calling FreeSplMem.

Returns:
    Win32 error code
    Note, if not ERROR_SUCCESS the output parameter stays unchanged.

--*/
{
    extern WCHAR gszPorts   [];
    LPWSTR pszCommValues = NULL;
    DWORD dwStatus       = ERROR_SUCCESS;
    DWORD cbData         = 64,
          dwBlockSize;

    //
    // open the registry key
    //
    HKEY hPorts = NULL;
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszPorts, 0, KEY_QUERY_VALUE, &hPorts);
    }
    //
    // allocate the initial memory based on COM ports: "921600,8,n,1.5,x" with some extra space
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = (pszCommValues = AllocSplMem (cbData + sizeof(WCHAR))) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
    }
    //
    // get the string size
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        DWORD dwType = 0;
        dwBlockSize = cbData;
        pszCommValues[cbData / sizeof(WCHAR)] = UNICODE_NULL;
        dwStatus = RegQueryValueEx (hPorts, pszPortName, 0, &dwType, (BYTE*)pszCommValues, &cbData);
        //
        // check the result
        //
        if (dwStatus == ERROR_MORE_DATA)
        {
            dwStatus = ERROR_SUCCESS;
            //
            // release the buffer
            //
            FreeSplMem (pszCommValues);
            //
            // allocate precise amount of memory
            //
            if (dwStatus == ERROR_SUCCESS)
            {
                dwStatus = (pszCommValues = AllocSplMem (cbData + sizeof(WCHAR))) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
            }
            //
            // get data again
            //
            if (dwStatus == ERROR_SUCCESS)
            {
                dwBlockSize = cbData;
                pszCommValues[cbData / sizeof(WCHAR)] = UNICODE_NULL;
                dwStatus = RegQueryValueEx (hPorts, pszPortName, 0, &dwType, (BYTE*)pszCommValues, &cbData);
                if (dwStatus == ERROR_SUCCESS && dwType != REG_SZ)
                {
                    //
                    // unexpected data type
                    //
                    dwStatus = ERROR_INVALID_DATA;
                }
            }
        }
        else if (dwStatus == ERROR_SUCCESS && dwType != REG_SZ)
        {
            //
            // unexpected data type
            //
            dwStatus = ERROR_INVALID_DATA;
        }
    }
    //
    // update output parameter
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        // Add a NULL terminator because RegQueryValueEx does not guarantee it
        DWORD dwNullIndex = (dwBlockSize/2)-1;
        pszCommValues[dwNullIndex] = 0x00;
        *ppszCommValues = pszCommValues;
        pszCommValues   = NULL;
    }
    //
    // cleanup
    //
    if (pszCommValues)
    {
        FreeSplMem (pszCommValues);
    }
    if (hPorts)
    {
        RegCloseKey (hPorts);
    }
    return dwStatus == ERROR_SUCCESS;
}

VOID
GetTransmissionRetryTimeoutFromRegistry (
    OUT DWORD*      pdwTimeout
    )
/*++

Description:
    This function returns the timeout value stored in Registry at
    HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows:TransmissionRetryTimeout

Arguments:
    pdwTimeout  - [out] pointer to DWORD that will receive the timeout value

Returns:
    Win32 error code;
    Note, if not ERROR_SUCCESS the output parameter stays unchanged.

--*/
{
    extern WCHAR gszWindows [];
    DWORD  dwStatus     = ERROR_SUCCESS;
    LPWSTR pszTimeout   = NULL;
    DWORD  dwTimeout    = 0;
    DWORD  cbData       = 10,
           dwBlockSize;
    HKEY   hWindows     = NULL;
    //
    // open the registry key
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszWindows, 0, KEY_QUERY_VALUE, &hWindows);
    }
    //
    // allocate the initial memory for timeout string
    // this is a timeout period in seconds with initial value of 90
    // it shouldn't be larger then four digits (four WCHARs)
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = (pszTimeout = AllocSplMem (cbData + sizeof(WCHAR))) ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
    }
    //
    // get data
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        DWORD dwType = 0;
        dwBlockSize = cbData;
        pszTimeout[cbData / sizeof(WCHAR)] = UNICODE_NULL;
        dwStatus = RegQueryValueEx (hWindows, szINIKey_TransmissionRetryTimeout, 0, &dwType, (BYTE*)pszTimeout, &cbData);
        //
        // check the result
        //
        if (dwStatus == ERROR_MORE_DATA)
        {
            dwStatus = ERROR_SUCCESS;
            //
            // release the buffer
            //
            FreeSplMem (pszTimeout);
            //
            // allocate precise amount of memory
            //
            if (dwStatus == ERROR_SUCCESS)
            {
                dwStatus = (pszTimeout = AllocSplMem (cbData + sizeof(WCHAR)))? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
            }
            //
            // get data again
            //
            if (dwStatus == ERROR_SUCCESS)
            {
                dwBlockSize = cbData;
                pszTimeout[cbData / sizeof(WCHAR)] = UNICODE_NULL;
                dwStatus = RegQueryValueEx (hWindows, szINIKey_TransmissionRetryTimeout, 0, &dwType, (BYTE*)pszTimeout, &cbData);
                if (dwStatus == ERROR_SUCCESS && dwType != REG_SZ)
                {
                    //
                    // unexpected data type
                    //
                    dwStatus = ERROR_INVALID_DATA;
                }
            }
        }
        else if (dwStatus == ERROR_SUCCESS && dwType != REG_SZ)
        {
            //
            // unexpected data type
            //
            dwStatus = ERROR_INVALID_DATA;
        }
    }
    //
    // retrieve timeout value from string
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        // Add a NULL terminator because RegQueryValueEx does not guarantee it
        DWORD dwNullIndex = (dwBlockSize/2)-1;
        pszTimeout[dwNullIndex] = 0x00;
        dwStatus = (swscanf (pszTimeout, L"%lu", &dwTimeout) == 1)? ERROR_SUCCESS : ERROR_INVALID_DATA;
    }
    //
    // update output parameter
    //
    *pdwTimeout = (dwStatus == ERROR_SUCCESS)? dwTimeout : 45;
    //
    // cleanup
    //
    if (pszTimeout)
    {
        FreeSplMem (pszTimeout);
    }
    if (hWindows)
    {
        RegCloseKey (hWindows);
    }
}
DWORD
SetTransmissionRetryTimeoutInRegistry (
    IN  LPCWSTR       pszTimeout
    )
/*++

Description:
    This function returns sets the timeout value in the Registry at
    HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows:TransmissionRetryTimeout

Arguments:
    pdwTimeout  - [in] new timeout value

Returns:
    Win32 error code

--*/
{
    extern WCHAR gszWindows [];
    DWORD  dwStatus  = ERROR_SUCCESS;
    //
    // open the registry key
    //
    HKEY hWindows = NULL;
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszWindows, 0, KEY_SET_VALUE, &hWindows);
    }
    //
    // get data
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        DWORD cbData = (wcslen (pszTimeout) + 1) * sizeof pszTimeout [0];
        dwStatus = RegSetValueEx (hWindows, szINIKey_TransmissionRetryTimeout, 0, REG_SZ, (BYTE*)pszTimeout, cbData);
    }
    //
    // cleanup
    //
    if (hWindows)
    {
        RegCloseKey (hWindows);
    }
    return dwStatus;
}
BOOL
AddPortInRegistry (
    IN  LPCWSTR pszPortName
    )
/*++

Description:
    This function adds the port name in the Registry as
    HKLM\Software\Microsoft\Windows NT\CurrentVersion\Ports:<pszPortName>.
    Note, no communcation parameters will be added.

Arguments:
    pszPortName     - [in] port name

Returns:
    Win32 error code

--*/
{
    extern WCHAR gszPorts [];
    DWORD dwStatus = ERROR_SUCCESS;
    //
    // open the key
    //
    HKEY hPorts = NULL;
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszPorts, 0, KEY_SET_VALUE, &hPorts);
    }
    //
    // add the new value
    //
    if (dwStatus == ERROR_SUCCESS)
    {
        WCHAR szEmpty [] = L"";
        dwStatus = RegSetValueEx (hPorts, pszPortName, 0, REG_SZ, (BYTE*) szEmpty, sizeof szEmpty);
    }
    //
    // cleanup
    //
    if (hPorts)
    {
        RegCloseKey (hPorts);
    }
    return dwStatus == ERROR_SUCCESS;
}
VOID
DeletePortFromRegistry (
    IN  LPCWSTR         pszPortName
    )
/*++

Description:
    This function deletes the port from the Registry. Basicaly, it deletes the Registry value
    HKLM\Software\Microsoft\Windows NT\CurrentVersion\Ports:<pszPortName>

Arguments:
    pszPortName     - [in] port name

Returns:
    Win32 error code

--*/
{
    extern WCHAR gszPorts [];
    DWORD dwStatus = ERROR_SUCCESS;
    //
    // open the key
    //
    HKEY hPorts = NULL;
    if (dwStatus == ERROR_SUCCESS)
    {
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE, gszPorts, 0, KEY_SET_VALUE, &hPorts);
    }
    //
    // remove the new value
    //
    if (dwStatus == ERROR_SUCCESS && pszPortName)
    {
        dwStatus = RegDeleteValue (hPorts, pszPortName);
    }
    //
    // cleanup
    //
    if (hPorts)
    {
        RegCloseKey (hPorts);
    }
}


