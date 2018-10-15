/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

    isochapi.c

Abstract


Author:

    Peter Binder (pbinder) 9-15-97

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
9-15-97  pbinder   birth
4-08-98  pbinder   taken from win1394
--*/

#define _ISOCHAPI_C
#include "pch.h"
#undef _ISOCHAPI_C

ULONG
WINAPI
IsochAllocateBandwidth(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_BANDWIDTH   isochAllocateBandwidth
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochAllocateBandwidth\r\n"));

    TRACE(TL_TRACE, (hWnd, "nMaxBytesPerFrameRequested = 0x%x\r\n", isochAllocateBandwidth->nMaxBytesPerFrameRequested));
    TRACE(TL_TRACE, (hWnd, "fulSpeed = 0x%x\r\n", isochAllocateBandwidth->fulSpeed));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_ALLOCATE_BANDWIDTH,
                                 isochAllocateBandwidth,
                                 sizeof(ISOCH_ALLOCATE_BANDWIDTH),
                                 isochAllocateBandwidth,
                                 sizeof(ISOCH_ALLOCATE_BANDWIDTH),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "hBandwidth = 0x%x\r\n", isochAllocateBandwidth->hBandwidth));
            TRACE(TL_TRACE, (hWnd, "BytesPerFrameAvailable = 0x%x\r\n", isochAllocateBandwidth->BytesPerFrameAvailable));
            TRACE(TL_TRACE, (hWnd, "Speed Selected = 0x%x\r\n", isochAllocateBandwidth->SpeedSelected));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochAllocateBandwidth = %d\r\n", dwRet));
    return(dwRet);
} // IsochAllocateBandwidth

ULONG
WINAPI
IsochAllocateChannel(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_CHANNEL     isochAllocateChannel
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochAllocateChannel\r\n"));

    TRACE(TL_TRACE, (hWnd, "nRequestedChannel = 0x%x\r\n", isochAllocateChannel->nRequestedChannel));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_ALLOCATE_CHANNEL,
                                 isochAllocateChannel,
                                 sizeof(ISOCH_ALLOCATE_CHANNEL),
                                 isochAllocateChannel,
                                 sizeof(ISOCH_ALLOCATE_CHANNEL),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "Channel = 0x%x\r\n", isochAllocateChannel->Channel));
            TRACE(TL_TRACE, (hWnd, "ChannelsAvailable.LowPart = 0x%x\r\n", isochAllocateChannel->ChannelsAvailable.LowPart));
            TRACE(TL_TRACE, (hWnd, "ChannelsAvailable.HighPart = 0x%x\r\n", isochAllocateChannel->ChannelsAvailable.HighPart));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochAllocateChannel = %d\r\n", dwRet));
    return(dwRet);
} // IsochAllocateChannel

ULONG
WINAPI
IsochAllocateResources(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_RESOURCES   isochAllocateResources
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter IsochAllocateResources\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulSpeed = 0x%x\r\n", isochAllocateResources->fulSpeed));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", isochAllocateResources->fulFlags));
    TRACE(TL_TRACE, (hWnd, "nChannel = 0x%x\r\n", isochAllocateResources->nChannel));
    TRACE(TL_TRACE, (hWnd, "nMaxBytesPerFrame = 0x%x\r\n", isochAllocateResources->nMaxBytesPerFrame));
    TRACE(TL_TRACE, (hWnd, "nNumberOfBuffers = 0x%x\r\n", isochAllocateResources->nNumberOfBuffers));
    TRACE(TL_TRACE, (hWnd, "nMaxBufferSize = 0x%x\r\n", isochAllocateResources->nMaxBufferSize));
    TRACE(TL_TRACE, (hWnd, "nQuadletsToStrip = 0x%x\r\n", isochAllocateResources->nQuadletsToStrip));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_ALLOCATE_RESOURCES,
                                 isochAllocateResources,
                                 sizeof(ISOCH_ALLOCATE_RESOURCES),
                                 isochAllocateResources,
                                 sizeof(ISOCH_ALLOCATE_RESOURCES),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochAllocateResources->hResource));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochAllocateResources = %d\r\n", dwRet));
    return(dwRet);
} // IsochAllocateResources

ULONG
WINAPI
IsochAttachBuffers(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ATTACH_BUFFERS       isochAttachBuffers,
    BOOL                        bAutoAlloc,
    BOOL                        bAutoFill
    )
{
    HANDLE      hDevice;

    PISOCH_ATTACH_BUFFERS           pIsochAttachBuffers;

    PRING3_ISOCH_DESCRIPTOR         pR3TempDescriptor;
    RING3_ISOCH_DESCRIPTOR          R3_IsochDescriptor;
    
    OVERLAPPED                      overLapped;
    
    ULONG                           ulBufferSize;
    DWORD                           dwBytesRet;
    DWORD                           dwRet;
    
    ULONG                           i, n;

    TRACE(TL_TRACE, (hWnd, "Enter IsochAttachBuffers\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochAttachBuffers->hResource));
    TRACE(TL_TRACE, (hWnd, "nNumberOfDescriptors = 0x%x\r\n", isochAttachBuffers->nNumberOfDescriptors));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].fulFlags));
    TRACE(TL_TRACE, (hWnd, "ulLength = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].ulLength));
    TRACE(TL_TRACE, (hWnd, "nMaxBytesPerFrame = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].nMaxBytesPerFrame));
    TRACE(TL_TRACE, (hWnd, "ulSynch = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].ulSynch));
    TRACE(TL_TRACE, (hWnd, "ulTag = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].ulTag));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleOffset = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleOffset));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleCount = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleCount));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_SecondCount = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_SecondCount));
    TRACE(TL_TRACE, (hWnd, "bUseCallback = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].bUseCallback));
    TRACE(TL_TRACE, (hWnd, "bAutoDetach = 0x%x\r\n", isochAttachBuffers->R3_IsochDescriptor[0].bAutoDetach));
    TRACE(TL_TRACE, (hWnd, "bAutoAlloc = %d\r\n", bAutoAlloc));
    TRACE(TL_TRACE, (hWnd, "bAutoFill = %d\r\n", bAutoFill));

    hDevice = OpenDevice(hWnd, szDeviceName, TRUE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        R3_IsochDescriptor = isochAttachBuffers->R3_IsochDescriptor[0];
    
        // they are always required to give me at least one filled in ring3 isoch descriptor.
        // this way if they set the bAutoAlloc flag, i have a template on what to set up.
        // basically, i'll auto allocate however many descriptors they want, base on the first one.
        // if they want something more, then they are required to allocate all of the descriptors themselves

        //
        // if the bAutoAlloc flag is set, that means that we allocate according to
        // the first descriptor. we assume there's only one configured descriptor
        //
        if (bAutoAlloc) {

            // lets calculate the buffer size
            ulBufferSize = sizeof(ISOCH_ATTACH_BUFFERS) +
                (isochAttachBuffers->R3_IsochDescriptor[0].ulLength*isochAttachBuffers->nNumberOfDescriptors) +
                (sizeof(RING3_ISOCH_DESCRIPTOR)*(isochAttachBuffers->nNumberOfDescriptors-1));

            pIsochAttachBuffers = (PISOCH_ATTACH_BUFFERS)LocalAlloc(LPTR, ulBufferSize);
            if (!pIsochAttachBuffers) {
                
                dwRet = GetLastError();
                TRACE(TL_ERROR, (hWnd, "Could not allocate pIsochAttachBuffers\r\n"));
                goto Exit_IsochAttachBuffers;
            }
    
            ZeroMemory(pIsochAttachBuffers, ulBufferSize);
            *pIsochAttachBuffers = *isochAttachBuffers;

            //
            // now we need to setup each descriptor
            //
            pR3TempDescriptor = &pIsochAttachBuffers->R3_IsochDescriptor[0];

            for (i=0; i < isochAttachBuffers->nNumberOfDescriptors; i++) {

            *pR3TempDescriptor = R3_IsochDescriptor;

            pR3TempDescriptor =
                (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pR3TempDescriptor +
                                              pR3TempDescriptor->ulLength +
                                              sizeof(RING3_ISOCH_DESCRIPTOR));
            }
        }
        else {

            // we have already received an allocated buffer, let's just map it into
            // our attach struct and pass it down.
            pIsochAttachBuffers = isochAttachBuffers;
            ulBufferSize = pIsochAttachBuffers->ulBufferSize;
        }

        if (pIsochAttachBuffers) {

            overLapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

            // reset our event, in case we'll be doing another attach
            ResetEvent(overLapped.hEvent);

            if (bAutoFill) {

                // get the point to the first descriptor
                pR3TempDescriptor = &pIsochAttachBuffers->R3_IsochDescriptor[0];

                for (i=0; i < pIsochAttachBuffers->nNumberOfDescriptors;  i++) {

                    // if we're listening, fill it with 0's
                    // if we're talking, fill it incrementally
                    if (isochAttachBuffers->R3_IsochDescriptor[i].fulFlags & RESOURCE_USED_IN_LISTENING) {

                        FillMemory(&pR3TempDescriptor->Data, pR3TempDescriptor->ulLength, 0);
                    }
                    else {

                        if (pR3TempDescriptor->fulFlags == DESCRIPTOR_HEADER_SCATTER_GATHER) {


                        }
                
                        else {

                            for (n=0; n < (pR3TempDescriptor->ulLength/sizeof(ULONG)); n++) {

                                CopyMemory((ULONG *)&pR3TempDescriptor->Data+n, (ULONG *)&n, sizeof(ULONG));
                            }
                        }
                    }

                    pR3TempDescriptor = (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pR3TempDescriptor +
                                    pR3TempDescriptor->ulLength + sizeof(RING3_ISOCH_DESCRIPTOR));

                }
            }
        }

        DeviceIoControl( hDevice,
                         IOCTL_ISOCH_ATTACH_BUFFERS,
                         pIsochAttachBuffers,
                         ulBufferSize,
                         pIsochAttachBuffers,
                         ulBufferSize,
                         &dwBytesRet,
                         &overLapped);

        dwRet = GetLastError();

        if ((dwRet != ERROR_SUCCESS) && (dwRet != ERROR_IO_PENDING)) {

            TRACE(TL_ERROR, (hWnd, "IsochAttachBuffers Failed = %d\r\n", dwRet));
        }
        else {

            // we got a pending, so we need to wait...
            if (dwRet == ERROR_IO_PENDING) {

                if (!GetOverlappedResult(hDevice, &overLapped, &dwBytesRet, TRUE)) {

                    // getoverlappedresult failed, lets find out why...
                    dwRet = GetLastError();
                    TRACE(TL_ERROR, (hWnd, "IsochAttachBuffers: GetOverlappedResult Failed! dwRet = %d\r\n", dwRet));

                }
            }
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }

Exit_IsochAttachBuffers:
    
    TRACE(TL_TRACE, (hWnd, "pIsochDescriptor = 0x%x\r\n", pIsochAttachBuffers->pIsochDescriptor));
    TRACE(TL_TRACE, (hWnd, "Exit IsochAttachBuffers\r\n"));
    return(dwRet);
} // IsochAttachBuffers

ULONG
WINAPI
IsochDetachBuffers(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_DETACH_BUFFERS       isochDetachBuffers
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochDetachBuffers\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochDetachBuffers->hResource));
    TRACE(TL_TRACE, (hWnd, "nNumberOfDescriptors = 0x%x\r\n", isochDetachBuffers->nNumberOfDescriptors));
    TRACE(TL_TRACE, (hWnd, "pIsochDescriptor = 0x%x\r\n", isochDetachBuffers->pIsochDescriptor));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_DETACH_BUFFERS,
                                 isochDetachBuffers,
                                 sizeof(ISOCH_DETACH_BUFFERS),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochDetachBuffers = %d\r\n", dwRet));
    return(dwRet);
} // IsochDetachBuffers

ULONG
WINAPI
IsochFreeBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName,
    HANDLE  hBandwidth
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochFreeBandwidth\r\n"));

    TRACE(TL_TRACE, (hWnd, "hBandwidth = 0x%x\r\n", hBandwidth));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_FREE_BANDWIDTH,
                                 &hBandwidth,
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochFreeBandwidth = %d\r\n", dwRet));
    return(dwRet);
} // IsochFreeBandwidth

ULONG
WINAPI
IsochFreeChannel(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   nChannel
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochFreeChannel\r\n"));

    TRACE(TL_TRACE, (hWnd, "nChannel = 0x%x\r\n", nChannel));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_FREE_CHANNEL,
                                 &nChannel,
                                 sizeof(ULONG),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochFreeChannel = %d\r\n", dwRet));
    return(dwRet);
} // IsochFreeChannel

ULONG
WINAPI
IsochFreeResources(
    HWND    hWnd,
    PSTR    szDeviceName,
    HANDLE  hResource
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochFreeResources\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", hResource));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_FREE_RESOURCES,
                                 &hResource,
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochFreeResources = %d\r\n", dwRet));
    return(dwRet);
} // IsochFreeResources

ULONG
WINAPI
IsochListen(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_LISTEN   isochListen
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochListen\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochListen->hResource));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", isochListen->fulFlags));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleOffset = 0x%x\r\n", isochListen->StartTime.CL_CycleOffset));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleCount = 0x%x\r\n", isochListen->StartTime.CL_CycleCount));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_SecondCount = 0x%x\r\n", isochListen->StartTime.CL_SecondCount));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_LISTEN,
                                 isochListen,
                                 sizeof(ISOCH_LISTEN),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochListen = %d\r\n", dwRet));
    return(dwRet);
} // IsochListen

ULONG
WINAPI
IsochQueryCurrentCycleTime(
    HWND            hWnd,
    PSTR            szDeviceName,
    PCYCLE_TIME     CycleTime
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochQueryCurrentCycleTime\r\n"));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_QUERY_CURRENT_CYCLE_TIME,
                                 NULL,
                                 0,
                                 CycleTime,
                                 sizeof(CYCLE_TIME),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "CycleTime.CL_CycleOffset = 0x%x\r\n", CycleTime->CL_CycleOffset));
            TRACE(TL_TRACE, (hWnd, "CycleTime.CL_CycleCount = 0x%x\r\n", CycleTime->CL_CycleCount));
            TRACE(TL_TRACE, (hWnd, "CycleTime.CL_SecondCount = 0x%x\r\n", CycleTime->CL_SecondCount));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochQueryCurrentCycleTime = %d\r\n", dwRet));
    return(dwRet);
} // IsochQueryCurrentCycleTime

ULONG
WINAPI
IsochQueryResources(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_QUERY_RESOURCES      isochQueryResources
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochQueryResources\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulSpeed = 0x%x\r\n", isochQueryResources->fulSpeed));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_QUERY_RESOURCES,
                                 isochQueryResources,
                                 sizeof(ISOCH_QUERY_RESOURCES),
                                 isochQueryResources,
                                 sizeof(ISOCH_QUERY_RESOURCES),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "BytesPerFrameAvailable = 0x%x\r\n", isochQueryResources->BytesPerFrameAvailable));
            TRACE(TL_TRACE, (hWnd, "ChannelsAvailable.LowPart = 0x%x\r\n", isochQueryResources->ChannelsAvailable.LowPart));
            TRACE(TL_TRACE, (hWnd, "ChannelsAvailable.HighPart = 0x%x\r\n", isochQueryResources->ChannelsAvailable.HighPart));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochQueryResources = %d\r\n", dwRet));
    return(dwRet);
} // IsochQueryResources

ULONG
WINAPI
IsochSetChannelBandwidth(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PISOCH_SET_CHANNEL_BANDWIDTH    isochSetChannelBandwidth
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochSetChannelBandwidth\r\n"));

    TRACE(TL_TRACE, (hWnd, "hBandwidth = 0x%x\r\n", isochSetChannelBandwidth->hBandwidth));
    TRACE(TL_TRACE, (hWnd, "nMaxBytesPerFrame = 0x%x\r\n", isochSetChannelBandwidth->nMaxBytesPerFrame));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH,
                                 isochSetChannelBandwidth,
                                 sizeof(ISOCH_SET_CHANNEL_BANDWIDTH),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochSetChannelBandwidth = %d\r\n", dwRet));
    return(dwRet);
} // IsochSetChannelBandwidth

ULONG
WINAPI
IsochStop(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_STOP     isochStop
    )
{

    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochStop\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochStop->hResource));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", isochStop->fulFlags));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_STOP,
                                 isochStop,
                                 sizeof(ISOCH_STOP),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochStop = %d\r\n", dwRet));
    return(dwRet);
} // IsochStop

ULONG
WINAPI
IsochTalk(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_TALK     isochTalk
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter IsochTalk\r\n"));

    TRACE(TL_TRACE, (hWnd, "hResource = %p\r\n", isochTalk->hResource));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", isochTalk->fulFlags));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleOffset = 0x%x\r\n", isochTalk->StartTime.CL_CycleOffset));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_CycleCount = 0x%x\r\n", isochTalk->StartTime.CL_CycleCount));
    TRACE(TL_TRACE, (hWnd, "StartTime.CL_SecondCount = 0x%x\r\n", isochTalk->StartTime.CL_SecondCount));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_TALK,
                                 isochTalk,
                                 sizeof(ISOCH_TALK),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit IsochTalk = %d\r\n", dwRet));
    return(dwRet);
} // IsochTalk

void
WINAPI
IsochStartLoopback(
    HWND                    hWnd,
    PSTR                    szDeviceName,
    PISOCH_LOOPBACK_PARAMS  isochLoopbackParams
    )
{
    isochLoopbackParams->szDeviceName = szDeviceName;
    isochLoopbackParams->hWnd = hWnd;
    isochLoopbackParams->bKill = FALSE;
    isochLoopbackParams->bLoopback = TRUE;

    isochLoopbackParams->hThread = CreateThread( NULL,
                                                 0,
                                                 IsochAttachThread,
                                                 (LPVOID) isochLoopbackParams,
                                                 0,
                                                 &isochLoopbackParams->ThreadId
                                                 );
} // IsochStartLoopback

void
WINAPI
IsochStopLoopback(
    PISOCH_LOOPBACK_PARAMS  isochLoopbackParams
    )
{

    // kill the thread
    isochLoopbackParams->bKill = TRUE;

    // wait for the running thread to exit before returning    
    WaitForSingleObjectEx(isochLoopbackParams->hThread,
                          INFINITE,
                          FALSE);

    // close the handle
    CloseHandle(isochLoopbackParams->hThread);

    return;
} // IsochStopLoopback

DWORD
WINAPI
IsochAttachThread(
    LPVOID  lpParameter
    )
{
    PISOCH_LOOPBACK_PARAMS          pIsochLoopbackParams;

    PISOCH_ATTACH_BUFFERS           pIsochAttachBuffers;
    ISOCH_ATTACH_BUFFERS            isochAttachBuffers;
    RING3_ISOCH_DESCRIPTOR          R3_IsochDescriptor;

    PRING3_ISOCH_DESCRIPTOR         pR3TempDescriptor;

    HANDLE                          hDevice;
    DWORD                           dwRet, dwBytesRet;
    ULONG                           ulBufferSize;
    OVERLAPPED                      overLapped;

    BOOLEAN                         bIsochTalk = FALSE;
    BOOLEAN                         bIsochListen = FALSE;

    ULONG                           i, n;

    TRACE(TL_TRACE, (NULL, "Enter IsochAttachThread\r\n"));

    // get pointer to our thread parameters
    pIsochLoopbackParams = (PISOCH_LOOPBACK_PARAMS)lpParameter;

    // reset our pass / fail parameteres
    pIsochLoopbackParams->ulFail = 0;
    pIsochLoopbackParams->ulPass = 0;
    pIsochLoopbackParams->ulIterations = 0;
    
    isochAttachBuffers = pIsochLoopbackParams->isochAttachBuffers;
    R3_IsochDescriptor = isochAttachBuffers.R3_IsochDescriptor[0];

    // try to open the device
    hDevice = OpenDevice(pIsochLoopbackParams->hWnd, pIsochLoopbackParams->szDeviceName, TRUE);

    // device opened, so let's do loopback
    if (hDevice != INVALID_HANDLE_VALUE) {

        //
        // if the bAutoAlloc flag is set, that means that we allocate according to
        // the first descriptor. we assume there's only one configured descriptor
        //
        if (pIsochLoopbackParams->bAutoAlloc) {

            // lets calculate the buffer size
            ulBufferSize = sizeof(ISOCH_ATTACH_BUFFERS) +
                (R3_IsochDescriptor.ulLength*isochAttachBuffers.nNumberOfDescriptors) +
                (sizeof(RING3_ISOCH_DESCRIPTOR)*(isochAttachBuffers.nNumberOfDescriptors-1));

            pIsochAttachBuffers = (PISOCH_ATTACH_BUFFERS)LocalAlloc(LPTR, ulBufferSize);
            if (!pIsochAttachBuffers) {

                pIsochLoopbackParams->bKill = TRUE;
                goto Exit_IsochAttachThread;
            }

            ZeroMemory(pIsochAttachBuffers, ulBufferSize);
            *pIsochAttachBuffers = isochAttachBuffers;

            //
            // now we need to setup each descriptor
            //
            pR3TempDescriptor = &pIsochAttachBuffers->R3_IsochDescriptor[0];

            for (i=0; i < isochAttachBuffers.nNumberOfDescriptors; i++) {

                *pR3TempDescriptor = R3_IsochDescriptor;

                pR3TempDescriptor =
                    (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pR3TempDescriptor +
                                              pR3TempDescriptor->ulLength +
                                              sizeof(RING3_ISOCH_DESCRIPTOR));
            }
        }
        else {

            // we have already received an allocated buffer, let's just map it into
            // our attach struct and pass it down.
            pIsochAttachBuffers = &pIsochLoopbackParams->isochAttachBuffers;
            ulBufferSize = pIsochAttachBuffers->ulBufferSize;
        }

        if (pIsochAttachBuffers) {
        
            overLapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

            // loop until the thread is killed
            while (TRUE) {

                // reset our event, in case we'll be doing another attach
                ResetEvent(overLapped.hEvent);

                if (pIsochLoopbackParams->bAutoFill) {

                    // get the point to the first descriptor
                    pR3TempDescriptor = &pIsochAttachBuffers->R3_IsochDescriptor[0];

                    for (i=0; i < pIsochAttachBuffers->nNumberOfDescriptors;  i++) {

                        // if we're listening, fill it with 0's
                        // if we're talking, fill it incrementally
                        if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_LISTENING) {

                            FillMemory(&pR3TempDescriptor->Data, pR3TempDescriptor->ulLength, 0);
                        }
                        else {

                            for (n=0; n < (pR3TempDescriptor->ulLength/sizeof(ULONG)); n++) {

                                CopyMemory((ULONG *)&pR3TempDescriptor->Data+n, (ULONG *)&n, sizeof(ULONG));
                            }
                        }

                        pR3TempDescriptor =
                            (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pR3TempDescriptor +
                                                      pR3TempDescriptor->ulLength +
                                                      sizeof(RING3_ISOCH_DESCRIPTOR));
                    }
                }

                DeviceIoControl( hDevice,
                                 IOCTL_ISOCH_ATTACH_BUFFERS,
                                 pIsochAttachBuffers,
                                 ulBufferSize,
                                 pIsochAttachBuffers,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 &overLapped
                                 );

                dwRet = GetLastError();

                if ((dwRet != ERROR_IO_PENDING) && (dwRet != ERROR_SUCCESS)) {

                    pIsochLoopbackParams->bKill = TRUE;

                    // if we fail, up the isoch fail counter and stop this thread gracefully...
                    if ((dwRet != ERROR_OPERATION_ABORTED) && (pIsochLoopbackParams->bLoopback)) {

                        pIsochLoopbackParams->ulFail++;
                    }
                    else {

                        TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "IsochAttachBuffers Failed = %d\r\n", dwRet));
                    }
                }
                else {

                    // if we're doing a loopback, then we'll auto start talk or listen
                    if (pIsochLoopbackParams->bLoopback) {

                        if (!bIsochListen && pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_LISTENING) {

                            ISOCH_LISTEN    isochListen;
                            OVERLAPPED      ListenOverlapped;

                            ListenOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                            isochListen.hResource = isochAttachBuffers.hResource;
                            isochListen.fulFlags = 0;
                            isochListen.StartTime.CL_SecondCount = 0;
                            isochListen.StartTime.CL_CycleCount = 0;
                            isochListen.StartTime.CL_CycleOffset = 0;

                            DeviceIoControl( hDevice,
                                             IOCTL_ISOCH_LISTEN,
                                             &isochListen,
                                             sizeof(ISOCH_LISTEN),
                                             NULL,
                                             0,
                                             &dwBytesRet,
                                             &ListenOverlapped
                                             );

                            CloseHandle(ListenOverlapped.hEvent);

                            bIsochListen = TRUE;
                        }
                        else if (!bIsochTalk && pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_TALKING) {

                            ISOCH_TALK      isochTalk;
                            OVERLAPPED      TalkOverlapped;

                            TalkOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                            isochTalk.hResource = isochAttachBuffers.hResource;
                            isochTalk.fulFlags = 0;
                            isochTalk.StartTime.CL_SecondCount = 0;
                            isochTalk.StartTime.CL_CycleCount = 0;
                            isochTalk.StartTime.CL_CycleOffset = 0;

                            DeviceIoControl( hDevice,
                                             IOCTL_ISOCH_TALK,
                                             &isochTalk,
                                             sizeof(ISOCH_TALK),
                                             NULL,
                                             0,
                                             &dwBytesRet,
                                             &TalkOverlapped
                                             );

                            CloseHandle(TalkOverlapped.hEvent);

                            bIsochTalk = TRUE;
                        }
                    }

                    // we got a pending, so we need to wait...
                    if (dwRet == ERROR_IO_PENDING) {

                        if (!pIsochLoopbackParams->bLoopback) {

                            TRACE(TL_TRACE, (pIsochLoopbackParams->hWnd, "IsochAttachBuffers Pending...\r\n"));
                        }
                        
                        Sleep(0);
                        if (!GetOverlappedResult(hDevice, &overLapped, &dwBytesRet, FALSE)) {

                            // getoverlappedresult failed, lets find out why...
                            dwRet = GetLastError();
                            TRACE(TL_ERROR, (NULL, "IsochAttachBuffers: GetOverlappedResult Failed! dwRet = %d\r\n", dwRet));

                            pIsochLoopbackParams->bKill = TRUE;
                        }
                    }

                    // we're doing a loopback test, so we need to increment our
                    // test values
                    if ((!pIsochLoopbackParams->bKill) && (pIsochLoopbackParams->bLoopback)) {

                        // if listen, then verify the buffer
                        if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_LISTENING) {

                            PULONG      pDescriptorData;
                            ULONG       ulCurrData;
                            ULONG       ulFlipValue;
                            BOOLEAN     bValid = TRUE;

                            // get the point to the first descriptor
                            pR3TempDescriptor = &pIsochAttachBuffers->R3_IsochDescriptor[0];

                            for (i=0; i < pIsochAttachBuffers->nNumberOfDescriptors; i++) {

                                // set pDescriptorData to the first quadlet in the data packet.
                                // if we are not stripping quadlets on the listen, then this will
                                // start 2 quadlets (is this right??) into the data buffer
                                if ((pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_LISTENING) &&
                                    (pIsochLoopbackParams->ulLoopFlag & RESOURCE_STRIP_ADDITIONAL_QUADLETS)) {

                                    pDescriptorData = (PULONG)&pR3TempDescriptor->Data;
                                }
                                else {

                                    pDescriptorData = (PULONG)&pR3TempDescriptor->Data+2;
                                }

                                // set the first data byte we are looking for
                                ulCurrData = *(PULONG)pDescriptorData;

                                // we use this to determine if ulCurrData needs to be flipped to 0
                                ulFlipValue = pR3TempDescriptor->ulLength/sizeof(ULONG);

                                for (n=0; n < (pR3TempDescriptor->ulLength/sizeof(ULONG)); n++) {

                                    if (*((PULONG)pDescriptorData+n) != ulCurrData) {

                                        // here we need to check if we are in packet based. it is possible our
                                        // actual data is smaller than the payload. i'm going to do a simple check
                                        // if this quadlet equals 0. if so, then we will assume that it is correct
                                        // and move onto the next quadlet.
                                        if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USE_PACKET_BASED) {

                                            // we're packet based, if quadlets doesn't equal 0, then fail...
                                            if (*((PULONG)pDescriptorData+n)) {

                                                bValid = FALSE;
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "Failed! = 0x%x\r\n", &pDescriptorData));
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "n = 0x%x\r\n", n));
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "ulCurrData = 0x%x\r\n", ulCurrData));
                                            }
                                        }
                                        else {

                                            // need to see if we are getting the header/trailer. if so, this is a real
                                            // error, if not, then it might not be, since we're getting the header/trailer
                                            // embedded in the packet.
                                            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_STRIP_ADDITIONAL_QUADLETS) {

                                                bValid = FALSE;
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "Failed! = 0x%x\r\n", &pDescriptorData));
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "n = 0x%x\r\n", n));
                                                TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "ulCurrData = 0x%x\r\n", ulCurrData));
                                            }
                                            else {

                                                // not stripping, up the pointer by two and see if the quadlets match to
                                                // what we expect next, if it doesn't, then we have an error
                                                n = n+2;
                                                if (*((PULONG)pDescriptorData+n) != ulCurrData) {

                                                    bValid = FALSE;
                                                    TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "Failed! = 0x%x\r\n", &pDescriptorData));
                                                    TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "n = 0x%x\r\n", n));
                                                    TRACE(TL_ERROR, (pIsochLoopbackParams->hWnd, "ulCurrData = 0x%x\r\n", ulCurrData));
                                                }
                                            }
                                        }
                                    }

                                    if (!bValid)
                                        break;

                                    ulCurrData++;

                                    if (ulCurrData == ulFlipValue)
                                        ulCurrData = 0;
                                }

                                if (!bValid)
                                    break;

                                pR3TempDescriptor =
                                    (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pR3TempDescriptor +
                                                              pR3TempDescriptor->ulLength +
                                                              sizeof(RING3_ISOCH_DESCRIPTOR));
                            } // for

                            (bValid) ? pIsochLoopbackParams->ulPass++ : pIsochLoopbackParams->ulFail++;

                        } // if
                        else if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_TALKING) {

                            // any check here??
                            pIsochLoopbackParams->ulPass++;
                        }

                        pIsochLoopbackParams->ulIterations++;
                    }
                }

                // if we reached our iteration count, then lets exit...
                if (pIsochLoopbackParams->ulIterations == pIsochLoopbackParams->nIterations)
                    pIsochLoopbackParams->bKill = TRUE;

                // see if we have been shut down
                if (pIsochLoopbackParams->bKill) {

                    // we've been shutdown, lets see if we were in loopback mode
                    // if so, lets stop on this resource
                    if (pIsochLoopbackParams->bLoopback) {

                        ISOCH_STOP      isochStop;
                        OVERLAPPED      StopOverlapped;

                        StopOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                        isochStop.hResource = isochAttachBuffers.hResource;
                        isochStop.fulFlags = 0;

                        DeviceIoControl( hDevice,
                                         IOCTL_ISOCH_STOP,
                                         &isochStop,
                                         sizeof(ISOCH_STOP),
                                         NULL,
                                         0,
                                         &dwBytesRet,
                                         &StopOverlapped
                                         );

                        CloseHandle(StopOverlapped.hEvent);
                    }

                    break;
                }
            } // while
        } // if

Exit_IsochAttachThread:

        // free up all resources
        CloseHandle(hDevice);
        CloseHandle(overLapped.hEvent);

        if (pIsochAttachBuffers) {

            if (!pIsochLoopbackParams->bLoopback) {

                TRACE(TL_TRACE, (pIsochLoopbackParams->hWnd, "pIsochDescriptor = 0x%x\r\n", pIsochAttachBuffers->pIsochDescriptor));
            }

            if (pIsochLoopbackParams->bAutoAlloc)
                LocalFree(pIsochAttachBuffers);
        }
    } // if hDevice

    // if it's not loopback, then we need to free
    // this buffer, since our attach allocates it
    if (!pIsochLoopbackParams->bLoopback)
        LocalFree(pIsochLoopbackParams);

    TRACE(TL_TRACE, (NULL, "Exit IsochAttachThread\r\n"));
    // that's it, shut this thread down
    ExitThread(0);
    return(0);
} // IsochAttachThread




