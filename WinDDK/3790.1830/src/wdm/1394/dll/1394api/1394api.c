/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    1394api.c

Abstract


Author:

    Peter Binder (pbinder) 4/08/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/08/98  pbinder   birth
--*/

#define _1394API_C
#include "pch.h"
#undef _1394API_C

ULONG
WINAPI
GetLocalHostInformation(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PGET_LOCAL_HOST_INFORMATION     getLocalHostInfo,
    BOOL                            bAutoAlloc
    )
{
    PGET_LOCAL_HOST_INFORMATION     pGetLocalHostInfo;
    HANDLE                          hDevice;
    DWORD                           dwRet, dwBytesRet;
    ULONG                           ulBufferSize;

    TRACE(TL_TRACE, (hWnd, "Enter GetLocalHostInformation\r\n"));

    TRACE(TL_TRACE, (hWnd, "nLevel = 0x%x\r\n", getLocalHostInfo->nLevel));
    TRACE(TL_TRACE, (hWnd, "ulBufferSize = 0x%x\r\n", getLocalHostInfo->ulBufferSize));

    // set Status = 0, this means even if we fail, buffer won't get updated and
    // it will stay at zero and we will get the true error from the ioctl call
    getLocalHostInfo->Status = 0;

    if (bAutoAlloc) {   

        // we don't support auto alloc for nLevel = 5 or 6
        switch (getLocalHostInfo->nLevel)
        {
            case 1:
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFO1);
                break;

            case 2:
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFO2);
                break;

            case 3:
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFO3);
                break;

            case 4:
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFO4);
                break;

            case 7:
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFO7);
                break;
                
            case 5:
            case 6:
            default:
                dwRet = ERROR_INVALID_FUNCTION;
                goto Exit_GetLocalHostInfo;
                break;
        }
        
        ulBufferSize += sizeof(GET_LOCAL_HOST_INFORMATION);

        pGetLocalHostInfo = (PGET_LOCAL_HOST_INFORMATION)LocalAlloc(LPTR, ulBufferSize);
        if (!pGetLocalHostInfo)
        {
            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Could not allocate pGetLocalHostInfo!\r\n"));
            goto Exit_GetLocalHostInfo;
        }

        *pGetLocalHostInfo = *getLocalHostInfo;
    }
    else 
    {
        pGetLocalHostInfo = getLocalHostInfo;
        ulBufferSize = getLocalHostInfo->ulBufferSize;
    }

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_LOCAL_HOST_INFORMATION,
                                 pGetLocalHostInfo,
                                 ulBufferSize,
                                 pGetLocalHostInfo,
                                 ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if ((dwRet) && (!pGetLocalHostInfo->Status)) {

            dwRet = ERROR_SUCCESS;

            if (pGetLocalHostInfo->nLevel == 1) {

                PGET_LOCAL_HOST_INFO1   LocalHostInfo1;

                LocalHostInfo1 = (PGET_LOCAL_HOST_INFO1)&pGetLocalHostInfo->Information;

                TRACE(TL_TRACE, (hWnd, "UniqueId.LowPart = 0x%x\r\n", LocalHostInfo1->UniqueId.LowPart));
                TRACE(TL_TRACE, (hWnd, "UniqueId.HighPart = 0x%x\r\n", LocalHostInfo1->UniqueId.HighPart));
            }
            else if (pGetLocalHostInfo->nLevel == 2) {

                PGET_LOCAL_HOST_INFO2   LocalHostInfo2;

                LocalHostInfo2 = (PGET_LOCAL_HOST_INFO2)&pGetLocalHostInfo->Information;

                TRACE(TL_TRACE, (hWnd, "HostCapabilities = 0x%x\r\n", LocalHostInfo2->HostCapabilities));
                TRACE(TL_TRACE, (hWnd, "MaxAsyncReadRequest = 0x%x\r\n", LocalHostInfo2->MaxAsyncReadRequest));
                TRACE(TL_TRACE, (hWnd, "MaxAsyncWriteRequest = 0x%x\r\n", LocalHostInfo2->MaxAsyncWriteRequest));
            }
            else if (pGetLocalHostInfo->nLevel == 3) {

                PGET_LOCAL_HOST_INFO3   LocalHostInfo3;

                LocalHostInfo3 = (PGET_LOCAL_HOST_INFO3)&pGetLocalHostInfo->Information;

                TRACE(TL_TRACE, (hWnd, "deciWattsSupplied = 0x%x\r\n", LocalHostInfo3->deciWattsSupplied));
                TRACE(TL_TRACE, (hWnd, "Voltage = 0x%x\r\n", LocalHostInfo3->Voltage));
            }
            else if (pGetLocalHostInfo->nLevel == 4) {

                PGET_LOCAL_HOST_INFO4   LocalHostInfo4;

                LocalHostInfo4 = (PGET_LOCAL_HOST_INFO4)&pGetLocalHostInfo->Information;

                TRACE(TL_TRACE, (hWnd, "PhysAddrMappingRoutine = 0x%x\r\n", LocalHostInfo4->PhysAddrMappingRoutine));
                TRACE(TL_TRACE, (hWnd, "Context = 0x%x\r\n", LocalHostInfo4->Context));
            }
            else if (pGetLocalHostInfo->nLevel == 5) {

                GET_LOCAL_HOST_INFO5   LocalHostInfo5;

                CopyMemory (&LocalHostInfo5,  pGetLocalHostInfo->Information, sizeof (GET_LOCAL_HOST_INFO5));

                TRACE(TL_TRACE, (hWnd, "ConfigRom = 0x%x\r\n", LocalHostInfo5.ConfigRom));
                TRACE(TL_TRACE, (hWnd, "ConfigRomLength = 0x%p\r\n", LocalHostInfo5.ConfigRomLength));
            }
            else if (pGetLocalHostInfo->nLevel == 6) {

                PGET_LOCAL_HOST_INFO6   LocalHostInfo6;

                LocalHostInfo6 = (PGET_LOCAL_HOST_INFO6)&pGetLocalHostInfo->Information;

                TRACE(TL_TRACE, (hWnd, "CsrBaseAddress = 0x%x\r\n", LocalHostInfo6->CsrBaseAddress));
                TRACE(TL_TRACE, (hWnd, "CsrDataLength = 0x%x\r\n", LocalHostInfo6->CsrDataLength));
                TRACE(TL_TRACE, (hWnd, "CsrDataBuffer = 0x%x\r\n", &LocalHostInfo6->CsrDataBuffer));

                if (LocalHostInfo6->CsrBaseAddress.Off_Low == SPEED_MAP_LOCATION) {

                    PSPEED_MAP  SpeedMap;
                    ULONG       NumNodes, i;

                    SpeedMap = (PSPEED_MAP)&LocalHostInfo6->CsrDataBuffer;

                    TRACE(TL_TRACE, (hWnd, "SpeedMap.SPD_Length = 0x%x\r\n", SpeedMap->SPD_Length));
                    TRACE(TL_TRACE, (hWnd, "SpeedMap.SPD_CRC = 0x%x\r\n", SpeedMap->SPD_CRC));
                    TRACE(TL_TRACE, (hWnd, "SpeedMap.SPD_Generation = 0x%x\r\n", SpeedMap->SPD_Generation));
                }
                else if (LocalHostInfo6->CsrBaseAddress.Off_Low == TOPOLOGY_MAP_LOCATION) {

                    PTOPOLOGY_MAP   TopologyMap;
                    ULONG           i;
                    PSELF_ID        SelfId;
                    PSELF_ID_MORE   SelfIdMore;
                    BOOL            bMore;

                    TopologyMap = (PTOPOLOGY_MAP)&LocalHostInfo6->CsrDataBuffer;

                    TRACE(TL_TRACE, (hWnd, "TopologyMap.TOP_Length = 0x%x\r\n", TopologyMap->TOP_Length));
                    TRACE(TL_TRACE, (hWnd, "TopologyMap.TOP_CRC = 0x%x\r\n", TopologyMap->TOP_CRC));
                    TRACE(TL_TRACE, (hWnd, "TopologyMap.TOP_Generation = 0x%x\r\n", TopologyMap->TOP_Generation));
                    TRACE(TL_TRACE, (hWnd, "TopologyMap.TOP_Node_Count = 0x%x\r\n", TopologyMap->TOP_Node_Count));
                    TRACE(TL_TRACE, (hWnd, "TopologyMap.TOP_Self_ID_Count = 0x%x\r\n", TopologyMap->TOP_Self_ID_Count));

                    for (i=0; i < TopologyMap->TOP_Self_ID_Count; i++) {

                        SelfId = &TopologyMap->TOP_Self_ID_Array[i];

                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Phys_ID = 0x%x\r\n", i, SelfId->SID_Phys_ID));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Packet_ID = 0x%x\r\n", i, SelfId->SID_Packet_ID));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Gap_Count = 0x%x\r\n", i, SelfId->SID_Gap_Count));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Link_Active = 0x%x\r\n", i, SelfId->SID_Link_Active));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Zero = 0x%x\r\n", i, SelfId->SID_Zero));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Power_Class = 0x%x\r\n", i, SelfId->SID_Power_Class));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Contender = 0x%x\r\n", i, SelfId->SID_Contender));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Delay = 0x%x\r\n", i, SelfId->SID_Delay));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Speed = 0x%x\r\n", i, SelfId->SID_Speed));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_More_Packets = 0x%x\r\n", i, SelfId->SID_More_Packets));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Initiated_Rst = 0x%x\r\n", i, SelfId->SID_Initiated_Rst));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Port3 = 0x%x\r\n", i, SelfId->SID_Port3));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Port2 = 0x%x\r\n", i, SelfId->SID_Port2));
                        TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Port1 = 0x%x\r\n", i, SelfId->SID_Port1));

                        if (SelfId->SID_More_Packets)
                            bMore = TRUE;
                        else
                            bMore = FALSE;

                        while (bMore) {

                            i++;

                            SelfIdMore = (PSELF_ID_MORE)&TopologyMap->TOP_Self_ID_Array[i];

                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Phys_ID = 0x%x\r\n", i, SelfIdMore->SID_Phys_ID));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Packet_ID = 0x%x\r\n", i, SelfIdMore->SID_Packet_ID));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortA = 0x%x\r\n", i, SelfIdMore->SID_PortA));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Reserved2 = 0x%x\r\n", i, SelfIdMore->SID_Reserved2));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Sequence = 0x%x\r\n", i, SelfIdMore->SID_Sequence));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_One = 0x%x\r\n", i, SelfIdMore->SID_One));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortE = 0x%x\r\n", i, SelfIdMore->SID_PortE));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortD = 0x%x\r\n", i, SelfIdMore->SID_PortD));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortC = 0x%x\r\n", i, SelfIdMore->SID_PortC));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortB = 0x%x\r\n", i, SelfIdMore->SID_PortB));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_More_Packets = 0x%x\r\n", i, SelfIdMore->SID_More_Packets));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_Reserved3 = 0x%x\r\n", i, SelfIdMore->SID_Reserved3));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortH = 0x%x\r\n", i, SelfIdMore->SID_PortH));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortG = 0x%x\r\n", i, SelfIdMore->SID_PortG));
                            TRACE(TL_TRACE, (hWnd, "SelfId[%d].SID_PortF = 0x%x\r\n", i, SelfIdMore->SID_PortF));

                            if (SelfIdMore->SID_More_Packets)
                                bMore = TRUE;
                            else
                                bMore = FALSE;
                        }
                    }
                }
                else {

                    TRACE(TL_TRACE, (hWnd, "Unknown Data Buffer.\r\n"));
                }
            } else if (pGetLocalHostInfo->nLevel == 7)
            {
                GET_LOCAL_HOST_INFO7    LocalHostInfo7;
                
                CopyMemory (&LocalHostInfo7, pGetLocalHostInfo->Information, sizeof (GET_LOCAL_HOST_INFO7));

                TRACE(TL_TRACE, (hWnd, "Host DMA Capabilities = 0x%x\r\n", LocalHostInfo7.HostDmaCapabilities));
                TRACE(TL_TRACE, (hWnd, "Max DMA Buffer Size =0x%x\r\n", LocalHostInfo7.MaxDmaBufferSize));
            }
        }
        else if (pGetLocalHostInfo->Status) {

            // this means its an insufficient buffer, that's the only
            // time this will be set to anything but 0 (SUCCESS)
            dwRet = ERROR_INSUFFICIENT_BUFFER;
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

Exit_GetLocalHostInfo:

    if ((bAutoAlloc) && (pGetLocalHostInfo))
        LocalFree(pGetLocalHostInfo);

    TRACE(TL_TRACE, (hWnd, "Exit GetLocalHostInformation = %d\r\n", dwRet));
    return(dwRet);
} // GetLocalHostInformation

ULONG
WINAPI
Get1394AddressFromDeviceObject(
    HWND                hWnd,
    PSTR                szDeviceName,
    PGET_1394_ADDRESS   Get1394Address
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter Get1394AddressFromDeviceObject\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", Get1394Address->fulFlags));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT,
                                 Get1394Address,
                                 sizeof(GET_1394_ADDRESS),
                                 Get1394Address,
                                 sizeof(GET_1394_ADDRESS),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "NA_Bus_Number = 0x%x\r\n", Get1394Address->NodeAddress.NA_Bus_Number));
            TRACE(TL_TRACE, (hWnd, "NA_Node_Number = 0x%x\r\n", Get1394Address->NodeAddress.NA_Node_Number));
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

    TRACE(TL_TRACE, (hWnd, "Exit Get1394AddressFromDeviceObject = %d\r\n", dwRet));
    return(dwRet);
} // Get1394AddressFromDeviceObject

ULONG
WINAPI
Control(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter Control\r\n"));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_CONTROL,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

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
    
    TRACE(TL_TRACE, (hWnd, "Exit Control = %d\r\n", dwRet));
    return(dwRet);
} // Control

ULONG
WINAPI
GetMaxSpeedBetweenDevices(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PGET_MAX_SPEED_BETWEEN_DEVICES  GetMaxSpeedBetweenDevices
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    ULONG       i;

    TRACE(TL_TRACE, (hWnd, "Enter GetMaxSpeedBetweenDevices\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", GetMaxSpeedBetweenDevices->fulFlags));
    TRACE(TL_TRACE, (hWnd, "ulNumberOfDestinations = 0x%x\r\n", GetMaxSpeedBetweenDevices->ulNumberOfDestinations));

    for (i=0; i<GetMaxSpeedBetweenDevices->ulNumberOfDestinations; i++) {

        TRACE(TL_TRACE, (hWnd, "hDestinationDeviceObjects[%d] = 0x%x\r\n", i, 
            GetMaxSpeedBetweenDevices->hDestinationDeviceObjects[i]));
    }

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES,
                                 GetMaxSpeedBetweenDevices,
                                 sizeof(GET_MAX_SPEED_BETWEEN_DEVICES),
                                 GetMaxSpeedBetweenDevices,
                                 sizeof(GET_MAX_SPEED_BETWEEN_DEVICES),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "fulSpeed = 0x%x\r\n", GetMaxSpeedBetweenDevices->fulSpeed));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit GetMaxSpeedBetweenDevices\r\n"));
    return(dwRet);
} // GetMaxSpeedBetweenDevices

ULONG
WINAPI
SetDeviceXmitProperties(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PDEVICE_XMIT_PROPERTIES     DeviceXmitProperties
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter SetDeviceXmitProperties\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulSpeed = 0x%x\r\n", DeviceXmitProperties->fulSpeed));
    TRACE(TL_TRACE, (hWnd, "fulPriority = 0x%x\r\n", DeviceXmitProperties->fulPriority));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_SET_DEVICE_XMIT_PROPERTIES,
                                 DeviceXmitProperties,
                                 sizeof(DEVICE_XMIT_PROPERTIES),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit SetDeviceXmitProperties = %d\r\n", dwRet));
    return(dwRet);
} // SetDeviceXmitProperties

ULONG
WINAPI
GetConfigurationInformation(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter GetConfigurationInformation\r\n"));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_CONFIGURATION_INFORMATION,
                                 NULL,
                                 0,
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
    
    TRACE(TL_TRACE, (hWnd, "Exit GetConfigurationInformation = %d\r\n", dwRet));
    return(dwRet);
} // GetConfigurationInformation

ULONG
WINAPI
BusReset(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   fulFlags
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter BusReset\r\n"));
    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", fulFlags));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_BUS_RESET,
                                 &fulFlags,
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
    
    TRACE(TL_TRACE, (hWnd, "Exit BusReset = %d\r\n", dwRet));
    return(dwRet);
} // BusReset

ULONG
WINAPI
GetGenerationCount(
    HWND    hWnd,
    PSTR    szDeviceName,
    PULONG  GenerationCount
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter GetGenerationCount\r\n"));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_GENERATION_COUNT,
                                 GenerationCount,
                                 sizeof(ULONG),
                                 GenerationCount,
                                 sizeof(ULONG),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            TRACE(TL_TRACE, (hWnd, "GenerationCount = 0x%x\r\n", *GenerationCount));
        }
        else {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = %d\r\n", dwRet));
        }

        // free up resources
        CloseHandle(hDevice);
    }
    else {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));        
    }
    
    TRACE(TL_TRACE, (hWnd, "Exit GetGenerationCount = %d\r\n", dwRet));
    return(dwRet);
} // GetGenerationCount

ULONG
WINAPI
SendPhyConfigurationPacket(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PPHY_CONFIGURATION_PACKET   PhyConfigurationPacket
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter SendPhyConfigurationPacket\r\n"));

    TRACE(TL_TRACE, (hWnd, "PCP_Phys_ID = 0x%x\r\n", PhyConfigurationPacket->PCP_Phys_ID));
    TRACE(TL_TRACE, (hWnd, "PCP_Packet_ID = 0x%x\r\n", PhyConfigurationPacket->PCP_Packet_ID));
    TRACE(TL_TRACE, (hWnd, "PCP_Gap_Count = 0x%x\r\n", PhyConfigurationPacket->PCP_Gap_Count));
    TRACE(TL_TRACE, (hWnd, "PCP_Set_Gap_Count = 0x%x\r\n", PhyConfigurationPacket->PCP_Set_Gap_Count));
    TRACE(TL_TRACE, (hWnd, "PCP_Force_Root = 0x%x\r\n", PhyConfigurationPacket->PCP_Force_Root));
    TRACE(TL_TRACE, (hWnd, "PCP_Reserved1 = 0x%x\r\n", PhyConfigurationPacket->PCP_Reserved1));
    TRACE(TL_TRACE, (hWnd, "PCP_Reserved2 = 0x%x\r\n", PhyConfigurationPacket->PCP_Reserved2));
    TRACE(TL_TRACE, (hWnd, "PCP_Inverse = 0x%x\r\n", PhyConfigurationPacket->PCP_Inverse));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_SEND_PHY_CONFIGURATION_PACKET,
                                 PhyConfigurationPacket,
                                 sizeof(PHY_CONFIGURATION_PACKET),
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
    
    TRACE(TL_TRACE, (hWnd, "Exit SendPhyConfigurationPacket = %d\r\n", dwRet));
    return(dwRet);
} // SendPhyConfigurationPacket

ULONG
WINAPI
BusResetNotification(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   fulFlags
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter BusResetNotification\r\n"));

    TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", fulFlags));

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_BUS_RESET_NOTIFICATION,
                                 &fulFlags,
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
    
    TRACE(TL_TRACE, (hWnd, "Exit BusResetNotification = %d\r\n", dwRet));
    return(dwRet);
} // BusResetNotification


ULONG
WINAPI
SetLocalHostInformation(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PSET_LOCAL_HOST_INFORMATION     SetLocalHostInfo
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter SetLocalHostInformation\r\n"));

    TRACE(TL_TRACE, (hWnd, "nLevel = 0x%x\r\n", SetLocalHostInfo->nLevel));
    TRACE(TL_TRACE, (hWnd, "ulBufferSize = 0x%x\r\n", SetLocalHostInfo->ulBufferSize));

    if (SetLocalHostInfo->nLevel == SET_LOCAL_HOST_PROPERTIES_GAP_COUNT) {

        PSET_LOCAL_HOST_PROPS2  LocalHostProps2;

        LocalHostProps2 = (PSET_LOCAL_HOST_PROPS2)&SetLocalHostInfo->Information;

        TRACE(TL_TRACE, (hWnd, "GapCountLowerBound = 0x%x\r\n", LocalHostProps2->GapCountLowerBound));
    }
    else if (SetLocalHostInfo->nLevel == SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM) {

        PSET_LOCAL_HOST_PROPS3  LocalHostProps3;

        LocalHostProps3 = (PSET_LOCAL_HOST_PROPS3)&SetLocalHostInfo->Information;

        TRACE(TL_TRACE, (hWnd, "fulFlags = 0x%x\r\n", LocalHostProps3->fulFlags));

        if (LocalHostProps3->fulFlags == SLHP_FLAG_ADD_CROM_DATA) {

            PULONG  pUlong;
            ULONG   i;

            TRACE(TL_TRACE, (hWnd, "nLength = 0x%x\r\n", LocalHostProps3->nLength));

            pUlong = (PULONG)&LocalHostProps3->Buffer;

            for (i=0; i<(LocalHostProps3->nLength/sizeof(ULONG)); i++) {

                TRACE(TL_TRACE, (hWnd, "Buffer[0x%x] = 0x%x\r\n", i, *pUlong));
                pUlong++;
            }
        }
        else if (LocalHostProps3->fulFlags == SLHP_FLAG_REMOVE_CROM_DATA) {

            TRACE(TL_TRACE, (hWnd, "hCromData = 0x%x\r\n", LocalHostProps3->hCromData));
        }
    }

    TRACE(TL_TRACE, (hWnd, "SetLocalHostInfo = 0x%x\r\n", SetLocalHostInfo));

    dwRet = 0;

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_SET_LOCAL_HOST_INFORMATION,
                                 SetLocalHostInfo,
                                 sizeof(SET_LOCAL_HOST_INFORMATION)+SetLocalHostInfo->ulBufferSize,
                                 SetLocalHostInfo,
                                 sizeof(SET_LOCAL_HOST_INFORMATION)+SetLocalHostInfo->ulBufferSize,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (hWnd, "Error = 0x%x\r\n", dwRet));
        }
        else {

            PSET_LOCAL_HOST_PROPS3  LocalHostProps3;

            LocalHostProps3 = (PSET_LOCAL_HOST_PROPS3)&SetLocalHostInfo->Information;

            dwRet = ERROR_SUCCESS;

            if (SetLocalHostInfo->nLevel == SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM) {

                if (LocalHostProps3->fulFlags == SLHP_FLAG_ADD_CROM_DATA) {

                    TRACE(TL_TRACE, (hWnd, "hCromData = 0x%x\r\n", LocalHostProps3->hCromData));
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
    
    TRACE(TL_TRACE, (hWnd, "Exit SetLocalHostInformation = %d\r\n", dwRet));
    return(dwRet);
} // SetLocalHostInformation


