/*++

Copyright (c) 1998 Microsoft Corporation

Module Name: 

    asyncapi.c

Abstract


Author:

    Peter Binder (pbinder) 5-13-98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
5-13-98  pbinder   birth
--*/    

#define _ASYNCAPI_C
#include "pch.h"
#undef _ASYNCAPI_C
#include <stdlib.h>

ULONG
WINAPI
AllocateAddressRange(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PALLOCATE_ADDRESS_RANGE     allocateAddressRange,
    BOOL                        bAutoAlloc
    )
{
    HANDLE                      hDevice;
    DWORD                       dwRet, dwBytesRet;
    PALLOCATE_ADDRESS_RANGE     pAllocateAddressRange;
    ULONG                       ulBufferSize;

    TRACE(TL_TRACE, (hWnd, "Enter AllocateAddressRange\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulAllocateFlags = 0x%x\r\n", allocateAddressRange->fulAllocateFlags));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", allocateAddressRange->fulFlags));
    TRACE(TL_TRACE, (hWnd, "nLength = 0x%x\r\n", allocateAddressRange->nLength));
    TRACE(TL_TRACE, (hWnd, "MaxSegmentSize = 0x%x\r\n", allocateAddressRange->MaxSegmentSize));
    TRACE(TL_TRACE, (hWnd, "fulAccessType = 0x%x\r\n", allocateAddressRange->fulAccessType));
    TRACE(TL_TRACE, (hWnd, "fulNotificationOptions = 0x%x\r\n", allocateAddressRange->fulNotificationOptions));
    TRACE(TL_TRACE, (hWnd, "Required1394Offset.Off_High = 0x%x\r\n", allocateAddressRange->Required1394Offset.Off_High));
    TRACE(TL_TRACE, (hWnd, "Required1394Offset.Off_low = 0x%x\r\n", allocateAddressRange->Required1394Offset.Off_Low));

    ulBufferSize = sizeof(ALLOCATE_ADDRESS_RANGE) + allocateAddressRange->nLength;

    if (bAutoAlloc) {

        pAllocateAddressRange = (PALLOCATE_ADDRESS_RANGE)LocalAlloc(LPTR, ulBufferSize);
        if (!pAllocateAddressRange) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Could not allocate pAllocateAddressRange\r\n"));
            goto Exit_AsyncAllocateAddressRange;
        }

        *pAllocateAddressRange = *allocateAddressRange;
    }
    else
        pAllocateAddressRange = allocateAddressRange;

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ALLOCATE_ADDRESS_RANGE,
                                 pAllocateAddressRange,
                                 ulBufferSize, 
                                 pAllocateAddressRange,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            TRACE(TL_TRACE, (hWnd, "hAddressRange = %p\r\n", pAllocateAddressRange->hAddressRange));
            TRACE(TL_TRACE, (hWnd, "Required1394Offset.Off_High = 0x%x\r\n", pAllocateAddressRange->Required1394Offset.Off_High));
            TRACE(TL_TRACE, (hWnd, "Required1394Offset.Off_low = 0x%x\r\n", pAllocateAddressRange->Required1394Offset.Off_Low));

            dwRet = ERROR_SUCCESS;
        }
        else {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        
        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }

Exit_AsyncAllocateAddressRange:

    // need to free the alloc buffer here...
    if (bAutoAlloc && pAllocateAddressRange) {

        // copy the handle and address offset
        allocateAddressRange->hAddressRange         = pAllocateAddressRange->hAddressRange;
        allocateAddressRange->Required1394Offset    = pAllocateAddressRange->Required1394Offset;
        LocalFree (pAllocateAddressRange);
    }

    TRACE(TL_TRACE, (hWnd, "Exit AllocateAddressRange = %d\r\n", dwRet));
    return(dwRet);
} // AllocateAddressRange

ULONG
WINAPI
FreeAddressRange(
    HWND        hWnd,
    PSTR        szDeviceName,
    HANDLE      hAddressRange
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter FreeAddressRange\r\n"));

    TRACE(TL_TRACE, (hWnd, "hAddressRange = %p\r\n", hAddressRange));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_FREE_ADDRESS_RANGE,
                                 &hAddressRange,
                                 sizeof(HANDLE),
                                 NULL,
                                 0,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
    TRACE(TL_TRACE, (hWnd, "Exit FreeAddressRange = %d\r\n", dwRet));
    return(dwRet);
} // FreeAddressRange

ULONG
WINAPI
GetAddressData (
    HWND                hWnd,
    PSTR                szDeviceName,
    PGET_ADDRESS_DATA   getAddressData
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter GetAddressData\r\n"));

    TRACE(TL_TRACE, (hWnd, "hAddressRange = %p\r\n", getAddressData->hAddressRange));
    TRACE(TL_TRACE, (hWnd, "nLength = 0x%x\r\n", getAddressData->nLength));
    TRACE(TL_TRACE, (hWnd, "ulOffset = 0x%x\r\n", getAddressData->ulOffset));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_ADDRESS_DATA,
                                 getAddressData,
                                 sizeof(GET_ADDRESS_DATA)+getAddressData->nLength,
                                 getAddressData,
                                 sizeof(GET_ADDRESS_DATA)+getAddressData->nLength,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    

        
    TRACE(TL_TRACE, (hWnd, "Exit GetAddressData = %d\r\n", dwRet));
    return(dwRet);
} // GetAddressData

ULONG
WINAPI
SetAddressData(
    HWND                hWnd,
    PSTR                szDeviceName,
    PSET_ADDRESS_DATA   setAddressData
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter SetAddressData\r\n"));

    TRACE(TL_TRACE, (hWnd, "hAddressRange = %p\r\n", setAddressData->hAddressRange));
    TRACE(TL_TRACE, (hWnd, "nLength = 0x%x\r\n", setAddressData->nLength));
    TRACE(TL_TRACE, (hWnd, "ulOffset = 0x%x\r\n", setAddressData->ulOffset));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_SET_ADDRESS_DATA,
                                 setAddressData,
                                 sizeof(SET_ADDRESS_DATA)+setAddressData->nLength,
                                 NULL,
                                 0,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
    TRACE(TL_TRACE, (hWnd, "Exit SetAddressData = %d\r\n", dwRet));
    return(dwRet);
} // SetAddressData

ULONG
WINAPI
AsyncRead(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_READ     asyncRead,
    BOOL            bAutoAlloc
    )
{
    HANDLE          hDevice;
    DWORD           dwRet, dwBytesRet;
    PASYNC_READ     pAsyncRead;
    ULONG           ulBufferSize;
    ULONG           i;

    TRACE(TL_TRACE, (hWnd, "Enter AsyncRead\r\n"));

    TRACE(TL_TRACE, (hWnd, "bRawMode = %d\r\n", asyncRead->bRawMode));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x%x\r\n", asyncRead->DestinationAddress.IA_Destination_ID.NA_Bus_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Node_Number = 0x%x\r\n", asyncRead->DestinationAddress.IA_Destination_ID.NA_Node_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_High = 0x%x\r\n", asyncRead->DestinationAddress.IA_Destination_Offset.Off_High));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_Low = 0x%x\r\n", asyncRead->DestinationAddress.IA_Destination_Offset.Off_Low));
    TRACE(TL_TRACE, (hWnd, "nNumberOfBytesToRead = 0x%x\r\n", asyncRead->nNumberOfBytesToRead));
    TRACE(TL_TRACE, (hWnd, "nBlockSize = 0x%x\r\n", asyncRead->nBlockSize));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", asyncRead->fulFlags));
    TRACE(TL_TRACE, (hWnd, "ulGeneration = 0x%x\r\n", asyncRead->ulGeneration));

    ulBufferSize = sizeof(ASYNC_READ) + asyncRead->nNumberOfBytesToRead;

    if (bAutoAlloc) {

        pAsyncRead = (PASYNC_READ)LocalAlloc(LPTR, ulBufferSize);
        if (!pAsyncRead) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Could not allocate pAsyncRead\r\n"));
            goto Exit_AsyncRead;
        }

        FillMemory(pAsyncRead, ulBufferSize, 0);

        *pAsyncRead = *asyncRead;
    }
    else
        pAsyncRead = asyncRead;

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ASYNC_READ,
                                 pAsyncRead,
                                 ulBufferSize,
                                 pAsyncRead,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

           dwRet = GetLastError();
           TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;

            for (i=0; i<(asyncRead->nNumberOfBytesToRead/sizeof(ULONG)); i++) {

                PULONG ulTemp;

                ulTemp = (PULONG)&pAsyncRead->Data[i*sizeof(ULONG)];
                TRACE(TL_TRACE, (hWnd, "Quadlet[0x%x] = 0x%x\r\n", i, (ULONG)*ulTemp));
            }
        }
                  
        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }

Exit_AsyncRead:
    
    if (pAsyncRead && bAutoAlloc)
        LocalFree(pAsyncRead);

    TRACE(TL_TRACE, (hWnd, "Exit AsyncRead = %d\r\n", dwRet));
    return(dwRet);
} // AsyncRead

ULONG
WINAPI
AsyncWrite(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_WRITE    asyncWrite,
    BOOL            bAutoAlloc
    )
{
    HANDLE          hDevice;
    DWORD           dwRet, dwBytesRet;
    PASYNC_WRITE    pAsyncWrite;
    ULONG           ulBufferSize;
    ULONG           i;

    TRACE(TL_TRACE, (hWnd, "Enter AsyncWrite\r\n"));

    TRACE(TL_TRACE, (hWnd, "bRawMode = %d\r\n", asyncWrite->bRawMode));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x%x\r\n", asyncWrite->DestinationAddress.IA_Destination_ID.NA_Bus_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Node_Number = 0x%x\r\n", asyncWrite->DestinationAddress.IA_Destination_ID.NA_Node_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_High = 0x%x\r\n", asyncWrite->DestinationAddress.IA_Destination_Offset.Off_High));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_Low = 0x%x\r\n", asyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low));
    TRACE(TL_TRACE, (hWnd, "nNumberOfBytesToWrite = 0x%x\r\n", asyncWrite->nNumberOfBytesToWrite));
    TRACE(TL_TRACE, (hWnd, "nBlockSize = 0x%x\r\n", asyncWrite->nBlockSize));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", asyncWrite->fulFlags));
    TRACE(TL_TRACE, (hWnd, "ulGeneration = 0x%x\r\n", asyncWrite->ulGeneration));

    ulBufferSize = sizeof(ASYNC_WRITE) + asyncWrite->nNumberOfBytesToWrite;

    if (bAutoAlloc) {

        pAsyncWrite = (PASYNC_WRITE)LocalAlloc(LPTR, ulBufferSize);
        if (!pAsyncWrite) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Could not allocate pAsyncWrite\r\n"));
            goto Exit_AsyncWrite;
        }
        
        FillMemory(pAsyncWrite, ulBufferSize, 0);
        *pAsyncWrite = *asyncWrite;

        for (i=0; i<asyncWrite->nNumberOfBytesToWrite/sizeof(ULONG); i++) {

            CopyMemory((ULONG *)&pAsyncWrite->Data+i, (ULONG *)&i, sizeof(ULONG));
        }
    }
    else
        pAsyncWrite = asyncWrite;

    for (i=0; i<(asyncWrite->nNumberOfBytesToWrite/sizeof(ULONG)); i++) {

        PULONG ulTemp;

        ulTemp = (PULONG)&pAsyncWrite->Data[i*sizeof(ULONG)];
        TRACE(TL_TRACE, (hWnd, "Quadlet[0x%x] = 0x%x\r\n", i, *ulTemp));
    }

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ASYNC_WRITE,
                                 pAsyncWrite,
                                 ulBufferSize,
                                 pAsyncWrite,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
Exit_AsyncWrite:

    if (pAsyncWrite && bAutoAlloc)
        LocalFree(pAsyncWrite);

    TRACE(TL_TRACE, (hWnd, "Exit AsyncWrite = %d\r\n", dwRet));
    return(dwRet);
} // AsyncWrite

ULONG
WINAPI
AsyncLock(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_LOCK     asyncLock
    )
{
    HANDLE          hDevice;
    DWORD           dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter AsyncLock\r\n"));

    TRACE(TL_TRACE, (hWnd, "bRawMode = %d\r\n", asyncLock->bRawMode));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x%x\r\n", asyncLock->DestinationAddress.IA_Destination_ID.NA_Bus_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_ID.NA_Node_Number = 0x%x\r\n", asyncLock->DestinationAddress.IA_Destination_ID.NA_Node_Number));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_High = 0x%x\r\n", asyncLock->DestinationAddress.IA_Destination_Offset.Off_High));
    TRACE(TL_TRACE, (hWnd, "DestinationAddress.IA_Destination_Offset.Off_Low = 0x%x\r\n", asyncLock->DestinationAddress.IA_Destination_Offset.Off_Low));
    TRACE(TL_TRACE, (hWnd, "nNumberOfArgBytes = 0x%x\r\n", asyncLock->nNumberOfArgBytes));
    TRACE(TL_TRACE, (hWnd, "nNumberOfDataBytes = 0x%x\r\n", asyncLock->nNumberOfDataBytes));
    TRACE(TL_TRACE, (hWnd, "fulTransactionType = 0x%x\r\n", asyncLock->fulTransactionType));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", asyncLock->fulFlags));
    TRACE(TL_TRACE, (hWnd, "Arguments[0] = 0x%x\r\n", asyncLock->Arguments[0]));
    TRACE(TL_TRACE, (hWnd, "Arguments[1] = 0x%x\r\n", asyncLock->Arguments[1]));
    TRACE(TL_TRACE, (hWnd, "DataValues[0] = 0x%x\r\n", asyncLock->DataValues[0]));
    TRACE(TL_TRACE, (hWnd, "DataValues[1] = 0x%x\r\n", asyncLock->DataValues[1]));
    TRACE(TL_TRACE, (hWnd, "ulGeneration = 0x%x\r\n", asyncLock->ulGeneration));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ASYNC_LOCK,
                                 asyncLock,
                                 sizeof(ASYNC_LOCK),
                                 asyncLock,
                                 sizeof(ASYNC_LOCK),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            TRACE(TL_TRACE, (hWnd, "Buffer[0] = 0x%x\r\n", asyncLock->Buffer[0]));
            TRACE(TL_TRACE, (hWnd, "Buffer[1] = 0x%x\r\n", asyncLock->Buffer[1]));

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
    TRACE(TL_TRACE, (hWnd, "Exit AsyncLock = %d\r\n", dwRet));
    return(dwRet);
} // AsyncLock

ULONG
WINAPI
AsyncStream(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_STREAM   asyncStream,
    BOOL            bAutoAlloc
    )
{
    HANDLE          hDevice;
    DWORD           dwRet, dwBytesRet;
    PASYNC_STREAM   pAsyncStream;
    ULONG           ulBufferSize;
    ULONG           i;

    TRACE(TL_TRACE, (hWnd, "Enter AsyncStream\r\n"));

    TRACE(TL_TRACE, (hWnd, "nNumberOfBytesToStream = 0x%x\r\n", asyncStream->nNumberOfBytesToStream));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", asyncStream->fulFlags));
    TRACE(TL_TRACE, (hWnd, "ulTag = 0x%x\r\n", asyncStream->ulTag));
    TRACE(TL_TRACE, (hWnd, "nChannel = 0x%x\r\n", asyncStream->nChannel));
    TRACE(TL_TRACE, (hWnd, "ulSynch = 0x%x\r\n", asyncStream->ulSynch));
    TRACE(TL_TRACE, (hWnd, "nSpeed = 0x%x\r\n", asyncStream->nChannel));

    ulBufferSize = sizeof(ASYNC_STREAM) + asyncStream->nNumberOfBytesToStream;

    if (bAutoAlloc) {

        pAsyncStream = (PASYNC_STREAM)LocalAlloc(LPTR, ulBufferSize);

        if (!pAsyncStream) {

            dwRet = GetLastError();
            return (dwRet);
        }
        
        FillMemory(pAsyncStream, ulBufferSize, 0);
        *pAsyncStream = *asyncStream;

        for (i=0; i<asyncStream->nNumberOfBytesToStream/sizeof(ULONG); i++) {

            CopyMemory((ULONG *)&pAsyncStream->Data+i, (ULONG *)&i, sizeof(ULONG));
        }
    }
    else
        pAsyncStream = asyncStream;

    for (i=0; i<(asyncStream->nNumberOfBytesToStream/sizeof(ULONG)); i++) {

        PULONG ulTemp;

        ulTemp = (PULONG)&pAsyncStream->Data[i];
        TRACE(TL_TRACE, (hWnd, "Quadlet[0x%x] = 0x%x\r\n", i, *ulTemp));
    }

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ASYNC_STREAM,
                                 pAsyncStream,
                                 ulBufferSize,
                                 pAsyncStream,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
    if (pAsyncStream && bAutoAlloc)
        LocalFree(pAsyncStream);

    TRACE(TL_TRACE, (hWnd, "Exit AsyncStream = %d\r\n", dwRet));
    return(dwRet);
} // AsyncStream

void
WINAPI
AsyncStartLoopback(
    HWND                    hWnd,
    PSTR                    szDeviceName,
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    )
{
    asyncLoopbackParams->szDeviceName = szDeviceName;
    asyncLoopbackParams->hWnd = hWnd;
    asyncLoopbackParams->bKill = FALSE;
        
    asyncLoopbackParams->hThread = CreateThread( NULL,
                                                 0,
                                                 AsyncLoopbackThread,
                                                 (LPVOID) asyncLoopbackParams,
                                                 0,
                                                 &asyncLoopbackParams->ThreadId
                                                 );
} // AsyncStartLoopback

void
WINAPI
AsyncStopLoopback(
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    )
{
    // kill the thread
    asyncLoopbackParams->bKill = TRUE;
        
    // wait for the thread to exit before returning
    WaitForSingleObjectEx(asyncLoopbackParams->hThread,
                          INFINITE,
                          FALSE);
    
    // close the handle
    CloseHandle(asyncLoopbackParams->hThread);
    return;
} // AsyncStopLoopback

DWORD
WINAPI
AsyncLoopbackThread(
    LPVOID  lpParameter
    )
{
    PASYNC_LOOPBACK_PARAMS          pLoopbackParams = NULL;
    PASYNC_READ                     pAsyncRead      = NULL;
    PASYNC_WRITE                    pAsyncWrite     = NULL;
    PVOID                           pAsyncLock      = NULL;

    HANDLE                          hDevice;
    DWORD                           dwRet, dwBytesRet;

    ULONG                           ulReadSize;
    ULONG                           ulWriteSize;
    ULONG                           ulLockSize;
    ULONG                           ulBufferSize;

    BOOLEAN                         bFailed = FALSE;

    // get pointer to our thread parameters
    pLoopbackParams = (PASYNC_LOOPBACK_PARAMS)lpParameter;

    // reset pass/fail iteration counts
    pLoopbackParams->ulPass = 0;
    pLoopbackParams->ulFail = 0;
    pLoopbackParams->ulIterations = 0;

    // try to open the device
    hDevice = OpenDevice(pLoopbackParams->hWnd, pLoopbackParams->szDeviceName, FALSE);

    // device opened, so let's do loopback
    if (hDevice != INVALID_HANDLE_VALUE) {

        // lets get our buffers setup for whatever loopback we're doing.

        // see if we want random sized buffers...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_RANDOM_LENGTH) {

            // lets get a random buffer size that's under our max bytes to read
            srand((unsigned) time(NULL));
        }

        // write...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_WRITE) {

            ulWriteSize = sizeof(ASYNC_WRITE) + pLoopbackParams->nMaxBytes;

            pAsyncWrite = (PASYNC_WRITE)LocalAlloc(LPTR, ulWriteSize);
            if (!pAsyncWrite) {

                dwRet = GetLastError();
                TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Could not allocate pAsyncWrite\r\n"));
                goto Exit_AsyncLoopbackThread;
            }

            FillMemory(pAsyncWrite, ulWriteSize, 0);

            // setup our values in the write buffer
            pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_High =
                pLoopbackParams->AddressOffset.Off_High;
            pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low = 
                pLoopbackParams->AddressOffset.Off_Low;
            pAsyncWrite->nBlockSize = 0;
            pAsyncWrite->fulFlags = 0;

            pAsyncWrite->bRawMode = FALSE;
            pAsyncWrite->bGetGeneration = TRUE;
        }

        // read...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_READ) {

            ulReadSize = sizeof(ASYNC_READ) + pLoopbackParams->nMaxBytes;

            pAsyncRead = (PASYNC_READ)LocalAlloc(LPTR, ulReadSize);
            if (!pAsyncRead) {

                dwRet = GetLastError();
                TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Could not allocate pAsyncRead\r\n"));
                goto Exit_AsyncLoopbackThread;
            }

            FillMemory(pAsyncRead, ulReadSize, 0);

            // setup our values in the read buffer
            pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_High = 
                pLoopbackParams->AddressOffset.Off_High;
            pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_Low = 
                pLoopbackParams->AddressOffset.Off_Low;
            pAsyncRead->nBlockSize = 0;
            pAsyncRead->fulFlags = 0;

            pAsyncRead->bRawMode = FALSE;
            pAsyncRead->bGetGeneration = TRUE;
        }

        // lock...
        if (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_LOCK) {

           // TODO: Add lock setup here

        }

        // loop until the thread is killed
        while (TRUE) {

            bFailed = FALSE;

            // lets see what type of loopback this is. we'll go through and
            // process and reads, writes, or locks...

            // lets get our buffer size...
            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_RANDOM_LENGTH) {

                ulBufferSize = rand() % pLoopbackParams->nMaxBytes;

                ulBufferSize++;
            }
            else {

                ulBufferSize = pLoopbackParams->nMaxBytes;
            }

            // write...
            if ((!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_WRITE)) {

                ULONG   i;

                // set the size/data buffer for our write
                pAsyncWrite->nNumberOfBytesToWrite = ulBufferSize;

                for (i=0; i<pAsyncWrite->nNumberOfBytesToWrite/sizeof(ULONG); i++) {

                    CopyMemory((ULONG *)&pAsyncWrite->Data+i, (ULONG *)&i, sizeof(ULONG));
                }

                dwRet = DeviceIoControl( hDevice,
                                         IOCTL_ASYNC_WRITE,
                                         pAsyncWrite,
                                         sizeof(ASYNC_WRITE) + ulBufferSize,
                                         pAsyncWrite,
                                         sizeof(ASYNC_WRITE) + ulBufferSize,
                                         &dwBytesRet,
                                         NULL
                                         );

                if (!dwRet) {

                    dwRet = GetLastError();
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Error = 0x%x\r\n", dwRet));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Write Failed\r\n"));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncWrite = 0x%p\r\n", pAsyncWrite));

                    // see if this is a generation error, if so, we'll assume
                    // a bus reset and continue
                    if (dwRet != ERROR_MR_MID_NOT_FOUND) {

                        // we had a failure, up the fail count...
                        if (dwRet != ERROR_OPERATION_ABORTED)
                            bFailed = TRUE;
                    }
                    else {

                        TRACE(TL_WARNING, (pLoopbackParams->hWnd, "Write - Invalid Generation!\r\n"));
                    }
                }
            } // write

            // read
            if ((!bFailed) && (!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_READ)) {

                // set the size/data buffer for our read
                pAsyncRead->nNumberOfBytesToRead = ulBufferSize;

                FillMemory(&pAsyncRead->Data, ulBufferSize, 0);

                dwRet = DeviceIoControl( hDevice,
                                         IOCTL_ASYNC_READ,
                                         pAsyncRead,
                                         sizeof(ASYNC_READ) + ulBufferSize,
                                         pAsyncRead,
                                         sizeof(ASYNC_READ) + ulBufferSize,
                                         &dwBytesRet,
                                         NULL
                                         );

                if (!dwRet) {

                    dwRet = GetLastError();

                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Error = 0x%x\r\n", dwRet));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Read Failed\r\n"));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncRead = 0x%p\r\n", pAsyncRead));

                    // see if this is a generation error, if so, we'll assume
                    // a bus reset and continue
                    if (dwRet != ERROR_MR_MID_NOT_FOUND) {

                        // we had a failure, up the fail count...
                        if (dwRet != ERROR_OPERATION_ABORTED)
                            bFailed = TRUE;
                    }
                    else {

                        TRACE(TL_WARNING, (pLoopbackParams->hWnd, "Read - Invalid Generation!\r\n"));
                    }
                }
                else {

                    if (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_WRITE) {

                        ULONG   ulResult;

                        ulResult = memcmp(pAsyncWrite->Data, pAsyncRead->Data, ulBufferSize);

                        if (ulResult != 0)
                            bFailed = TRUE;
                        else
                            bFailed = FALSE;

                        if (bFailed) {
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "ulResult = 0x%x\r\n", ulResult));
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncWrite = 0x%p\r\n", pAsyncWrite));
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncRead = 0x%p\r\n", pAsyncRead));
                        }
                    }
                }
            } // read

            // lock
            if ((!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_LOCK)) {

                // TODO: Add lock testing here

            } // lock

            // increment our counters...
            if (!pLoopbackParams->bKill) {

                if (bFailed)
                    pLoopbackParams->ulFail++;
                else
                    pLoopbackParams->ulPass++;

                pLoopbackParams->ulIterations++;

                // if we reached our iteration count, then lets exit...
                if (pLoopbackParams->ulIterations == pLoopbackParams->nIterations)
                    pLoopbackParams->bKill = TRUE;
            }

            // see if we have been shut down
            if (pLoopbackParams->bKill) {

                // we've been shutdown
                break;
            }
        } // while

Exit_AsyncLoopbackThread:

        // free up all resources
        CloseHandle(hDevice);

        if (pAsyncWrite)
            LocalFree(pAsyncWrite);

        if (pAsyncRead)
            LocalFree(pAsyncRead);

        if (pAsyncLock)
            LocalFree(pAsyncLock);

    } // if

    // that's it, shut this thread down
    ExitThread(0);
    return(0);
} // AsyncLoopbackThread

void
WINAPI
AsyncStartLoopbackEx(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PASYNC_LOOPBACK_PARAMS_EX   asyncLoopbackParamsEx
    )
{
    asyncLoopbackParamsEx->szDeviceName = szDeviceName;
    asyncLoopbackParamsEx->hWnd = hWnd;
    asyncLoopbackParamsEx->bKill = FALSE;
        
    asyncLoopbackParamsEx->hThread = CreateThread( NULL,
                                                 0,
                                                 AsyncLoopbackThreadEx,
                                                 (LPVOID) asyncLoopbackParamsEx,
                                                 0,
                                                 &asyncLoopbackParamsEx->ThreadId
                                                 );
} // AsyncStartLoopbackEx

void
WINAPI
AsyncStopLoopbackEx(
    PASYNC_LOOPBACK_PARAMS_EX  asyncLoopbackParamsEx
    )
{
    // kill the thread
    asyncLoopbackParamsEx->bKill = TRUE;
        
    // wait for the thread to exit before returning
    WaitForSingleObjectEx(asyncLoopbackParamsEx->hThread,
                          INFINITE,
                          FALSE);
    
    // close the handle
    CloseHandle(asyncLoopbackParamsEx->hThread);
    return;
} // AsyncStopLoopbackEx

DWORD
WINAPI
AsyncLoopbackThreadEx(
    LPVOID  lpParameter
    )
{
    PASYNC_LOOPBACK_PARAMS_EX       pLoopbackParams = NULL;
    PASYNC_READ                     pAsyncRead      = NULL;
    PASYNC_WRITE                    pAsyncWrite     = NULL;
    PVOID                           pAsyncLock      = NULL;

    HANDLE                          hDevice;
    DWORD                           dwRet, dwBytesRet;

    ULONG                           ulReadSize;
    ULONG                           ulWriteSize;
    ULONG                           ulLockSize;

    ULONG                           ulBufferSize;

    BOOLEAN                         bFailed = FALSE;

    // get pointer to our thread parameters
    pLoopbackParams = (PASYNC_LOOPBACK_PARAMS_EX)lpParameter;

    // reset pass/fail iteration counts
    pLoopbackParams->ulPass = 0;
    pLoopbackParams->ulFail = 0;
    pLoopbackParams->ulIterations = 0;

    // try to open the device
    hDevice = OpenDevice(pLoopbackParams->hWnd, pLoopbackParams->szDeviceName, FALSE);

    // device opened, so let's do loopback
    if (hDevice != INVALID_HANDLE_VALUE) {

        // lets get our buffers setup for whatever loopback we're doing.

        // see if we want random sized buffers...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_RANDOM_LENGTH) {

            // lets get a random buffer size that's under our max bytes to read
            srand((unsigned) time(NULL));
        }

        // write...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_WRITE) {

            ulWriteSize = sizeof(ASYNC_WRITE) + pLoopbackParams->nMaxBytes;

            pAsyncWrite = (PASYNC_WRITE)LocalAlloc(LPTR, ulWriteSize);
            if (!pAsyncWrite) {

                dwRet = GetLastError();
                TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Could not allocate pAsyncWrite\r\n"));
                goto Exit_AsyncLoopbackThread;
            }

            FillMemory(pAsyncWrite, ulWriteSize, 0);

            // setup our values in the write buffer
            pAsyncWrite->DestinationAddress.IA_Destination_ID = 
                pLoopbackParams->Destination.IA_Destination_ID;
            pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_High = 
                pLoopbackParams->Destination.IA_Destination_Offset.Off_High;
            pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low =
                pLoopbackParams->Destination.IA_Destination_Offset.Off_Low;

            pAsyncWrite->nBlockSize = 0;
            pAsyncWrite->fulFlags = 0;

            pAsyncWrite->bRawMode = pLoopbackParams->bRawMode;
            pAsyncWrite->bGetGeneration = TRUE;
        }

        // read...
        if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_READ) {

            ulReadSize = sizeof(ASYNC_READ) + pLoopbackParams->nMaxBytes;

            pAsyncRead = (PASYNC_READ)LocalAlloc(LPTR, ulReadSize);
            if (!pAsyncRead) {

                dwRet = GetLastError();
                TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Could not allocate pAsyncRead\r\n"));
                goto Exit_AsyncLoopbackThread;
            }

            FillMemory(pAsyncRead, ulReadSize, 0);

            // setup our values in the read buffer
            pAsyncRead->DestinationAddress.IA_Destination_ID = 
                pLoopbackParams->Destination.IA_Destination_ID;
            pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_High = 
                pLoopbackParams->Destination.IA_Destination_Offset.Off_High;
            pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_Low =
                pLoopbackParams->Destination.IA_Destination_Offset.Off_Low;
                
            pAsyncRead->nBlockSize = 0;
            pAsyncRead->fulFlags = 0;

            pAsyncRead->bRawMode = pLoopbackParams->bRawMode;
            pAsyncRead->bGetGeneration = TRUE;
        }

        // lock...
        if (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_LOCK) {



        }

        // loop until the thread is killed
        while (TRUE) {

            bFailed = FALSE;

            // lets see what type of loopback this is. we'll go through and
            // process and reads, writes, or locks...

            // lets get our buffer size...
            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_RANDOM_LENGTH) {

                // generate a random value between 1 and nMaxBytes
                ulBufferSize = rand() % pLoopbackParams->nMaxBytes;
                ulBufferSize++;
            }
            else {

                ulBufferSize = pLoopbackParams->nMaxBytes;
            }

            // write...
            if ((!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_WRITE)) {

                ULONG   i;

                // set the size/data buffer for our write
                pAsyncWrite->nNumberOfBytesToWrite = ulBufferSize;

                for (i=0; i<pAsyncWrite->nNumberOfBytesToWrite/sizeof(ULONG); i++) {

                    CopyMemory((ULONG *)&pAsyncWrite->Data+i, (ULONG *)&i, sizeof(ULONG));
                }

                dwRet = DeviceIoControl( hDevice,
                                         IOCTL_ASYNC_WRITE,
                                         pAsyncWrite,
                                         sizeof(ASYNC_WRITE) + ulBufferSize,
                                         pAsyncWrite,
                                         sizeof(ASYNC_WRITE) + ulBufferSize,
                                         &dwBytesRet,
                                         NULL
                                         );

                if (!dwRet) {

                    dwRet = GetLastError();
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Error = 0x%x\r\n", dwRet));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Write Failed\r\n"));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncWrite = 0x%p\r\n", pAsyncWrite));

                    // test code, hard code debug spew here...
                    TRACE(TL_FATAL, (NULL, "Async Read Failed Error = 0x%x\r\n", dwRet));
                    TRACE(TL_FATAL, (NULL, "Bytes to write = 0x%x, offset high = 0x%x, offset low = 0x%x\r\n",
                                     pAsyncWrite->nNumberOfBytesToWrite, 
                                     pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_High,
                                     pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low));

                    // see if this is a generation error, if so, we'll assume
                    // a bus reset and continue
                    if (dwRet != ERROR_MR_MID_NOT_FOUND) {

                        // we had a failure, up the fail count...
                        if (dwRet != ERROR_OPERATION_ABORTED)
                            bFailed = TRUE;
                    }
                    else {

                        TRACE(TL_WARNING, (pLoopbackParams->hWnd, "Write - Invalid Generation!\r\n"));
                    }
                }
            } // write

            // read
            if ((!bFailed) && (!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_READ)) {

                // set the size/data buffer for our read
                pAsyncRead->nNumberOfBytesToRead = ulBufferSize;

                FillMemory(&pAsyncRead->Data, ulBufferSize, 0);

                dwRet = DeviceIoControl( hDevice,
                                         IOCTL_ASYNC_READ,
                                         pAsyncRead,
                                         sizeof(ASYNC_READ) + ulBufferSize,
                                         pAsyncRead,
                                         sizeof(ASYNC_READ) + ulBufferSize,
                                         &dwBytesRet,
                                         NULL
                                         );

                if (!dwRet) {

                    dwRet = GetLastError();

                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Error = 0x%x\r\n", dwRet));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "Read Failed\r\n"));
                    TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncRead = 0x%p\r\n", pAsyncRead));

                    // test code, hard code debug spew here...
                    TRACE(TL_FATAL, (NULL, "Async Write Failed Error = 0x%x\r\n", dwRet));
                    TRACE(TL_FATAL, (NULL, "Bytes to write = 0x%x, offset high = 0x%x, offset low = 0x%x\r\n",
                                     pAsyncRead->nNumberOfBytesToRead, 
                                     pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_High,
                                     pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_Low));

                    // see if this is a generation error, if so, we'll assume
                    // a bus reset and continue
                    if (dwRet != ERROR_MR_MID_NOT_FOUND) {

                        // we had a failure, up the fail count...
                        if (dwRet != ERROR_OPERATION_ABORTED)
                            bFailed = TRUE;
                    }
                    else {

                        TRACE(TL_WARNING, (pLoopbackParams->hWnd, "Read - Invalid Generation!\r\n"));
                    }
                }
                else {

                    if (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_WRITE) {

                        ULONG   ulResult;

                        ulResult = memcmp(pAsyncWrite->Data, pAsyncRead->Data, ulBufferSize);

                        if (ulResult != 0)
                            bFailed = TRUE;
                        else
                            bFailed = FALSE;

                        if (bFailed) {
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "ulResult = 0x%x\r\n", ulResult));
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "bFailed = 0x%x\r\n", bFailed));
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncWrite = 0x%p\r\n", pAsyncWrite));
                            TRACE(TL_ERROR, (pLoopbackParams->hWnd, "pAsyncRead = 0x%p\r\n", pAsyncRead));

                        }
                    }
                }
            } // read

            // lock
            if ((!pLoopbackParams->bKill) && (pLoopbackParams->ulLoopFlag & ACCESS_FLAGS_TYPE_LOCK)) {


            } // lock

            // increment our counters...
            if (!pLoopbackParams->bKill) {

                if (bFailed)
                    pLoopbackParams->ulFail++;
                else
                    pLoopbackParams->ulPass++;

                pLoopbackParams->ulIterations++;

                // if we reached our iteration count, then lets exit...
                if (pLoopbackParams->ulIterations == pLoopbackParams->nIterations)
                    pLoopbackParams->bKill = TRUE;
            }

            // see if we have been shut down
            if (pLoopbackParams->bKill) {

                // we've been shutdown
                break;
            }
        } // while

Exit_AsyncLoopbackThread:

        // free up all resources
        CloseHandle(hDevice);

        if (pAsyncWrite)
            LocalFree(pAsyncWrite);

        if (pAsyncRead)
            LocalFree(pAsyncRead);

//        if (pAsyncLock)
//            LocalFree(pAsyncLock);

    } // if

    // that's it, shut this thread down
    ExitThread(0);
    return(0);
} // AsyncLoopbackThread











void
WINAPI
AsyncStreamStartLoopback(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    )
{
    streamLoopbackParams->szDeviceName = szDeviceName;
    streamLoopbackParams->hWnd = hWnd;
    streamLoopbackParams->bKill = FALSE;

    streamLoopbackParams->hThread = CreateThread( NULL,
                                                  0,
                                                  AsyncStreamLoopbackThread,
                                                  (LPVOID)streamLoopbackParams,
                                                  0,
                                                  &streamLoopbackParams->ThreadId
                                                  );
} // AsyncStreamStartLoopback

void
WINAPI
AsyncStreamStopLoopback(
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    )
{
    // close the handle
    CloseHandle(streamLoopbackParams->hThread);

    if (streamLoopbackParams->bKill)
        return;

    // kill the thread
    streamLoopbackParams->bKill = TRUE;
} // AsyncStreamStopLoopback

DWORD
WINAPI
AsyncStreamLoopbackThread(
    LPVOID  lpParameter
    )
{
    PASYNC_STREAM_LOOPBACK_PARAMS   pStreamLoopbackParams;
    PASYNC_STREAM                   pAsyncStream;

    HANDLE                          hDevice;
    DWORD                           dwRet, dwBytesRet;
    ULONG                           ulBufferSize;
    OVERLAPPED                      overLapped;
    ULONG                           i;

    TRACE(TL_TRACE, (NULL, "Enter AsyncStreamLoopbackThread\r\n"));

    // get pointer to our thread parameters
    pStreamLoopbackParams = (PASYNC_STREAM_LOOPBACK_PARAMS)lpParameter;

    // try to open the device
    hDevice = OpenDevice(pStreamLoopbackParams->hWnd, pStreamLoopbackParams->szDeviceName, TRUE);

    // device opened, so let's do loopback
    if (hDevice != INVALID_HANDLE_VALUE) {

        //
        // if the bAutoAlloc flag is set, that means that we allocate according to
        // the first descriptor. we assume there's only one configured descriptor
        //
        if (pStreamLoopbackParams->bAutoAlloc) {

            // lets calculate the buffer size
            ulBufferSize = sizeof(ASYNC_STREAM) + 
                           pStreamLoopbackParams->asyncStream.nNumberOfBytesToStream;

            pAsyncStream = (PASYNC_STREAM)LocalAlloc(LPTR, ulBufferSize);

            if (!pAsyncStream) {

                pStreamLoopbackParams->bKill = TRUE;
                goto Exit_AsyncStreamLoopbackThread;
            }

            ZeroMemory(pAsyncStream, ulBufferSize);
            *pAsyncStream = pStreamLoopbackParams->asyncStream;
        }
        else {

            // we have already received an allocated buffer, let's just map it into
            // our attach struct and pass it down.
            pAsyncStream = &pStreamLoopbackParams->asyncStream;
            ulBufferSize = sizeof(ASYNC_STREAM) + pAsyncStream->nNumberOfBytesToStream;
        }

        if (pAsyncStream) {
        
            overLapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

            // loop until the thread is killed
            while (TRUE) {

                // reset our event, in case we'll be doing another attach
                ResetEvent(overLapped.hEvent);

                if (pStreamLoopbackParams->bAutoFill) {

                    for (i=0; i < (pAsyncStream->nNumberOfBytesToStream/sizeof(ULONG)); i++) {

                        CopyMemory((ULONG *)&pAsyncStream->Data+i, (ULONG *)&i, sizeof(ULONG));
                    }
                }

                DeviceIoControl( hDevice,
                                 IOCTL_ASYNC_STREAM,
                                 pAsyncStream,
                                 ulBufferSize,
                                 pAsyncStream,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 &overLapped
                                 );

                dwRet = GetLastError();

                TRACE(TL_TRACE, (NULL, "DeviceIoControl: dwRet = %d\r\n", dwRet));

                if (dwRet != ERROR_SUCCESS) {

                    pStreamLoopbackParams->bKill = TRUE;
                    pStreamLoopbackParams->ulFail++;
                }
                else {

                    // we're doing a loopback test, so we need to increment our
                    // test values
                    if (!pStreamLoopbackParams->bKill) {

                        // any check here??
                        pStreamLoopbackParams->ulPass++;
                    }

                    pStreamLoopbackParams->ulIterations++;
                }

                // if we reached our iteration count, then lets exit...
                if (pStreamLoopbackParams->ulIterations == pStreamLoopbackParams->nIterations)
                    pStreamLoopbackParams->bKill = TRUE;

                // see if we have been shut down
                if (pStreamLoopbackParams->bKill) {
                    break;
                }
            } // while
        } // if

        // free up all resources
        CloseHandle(hDevice);
        CloseHandle(overLapped.hEvent);
        
Exit_AsyncStreamLoopbackThread:

        if ((pStreamLoopbackParams->bAutoAlloc) && (pAsyncStream))
            LocalFree(pAsyncStream);

    } // if hDevice

    TRACE(TL_TRACE, (NULL, "Exit AsyncStreamLoopbackThread\r\n"));
    // that's it, shut this thread down
    ExitThread(0);
    return(0);
} // AsyncStreamLoopbackThread

