/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    1394.c

Abstract

    1394 api wrappers

Author:

    Peter Binder (pbinder) 5/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/13/98 pbinder   birth
08/18/98 pbinder   changed for new dialogs
--*/

#define _1394_C
#include "pch.h"
#undef _1394_C

INT_PTR CALLBACK
BusResetDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PULONG   pFlags;
    static CHAR     tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pFlags = (PULONG)lParam;

            if (*pFlags == BUS_RESET_FLAGS_FORCE_ROOT)
                CheckDlgButton( hDlg, IDC_BUS_RESET_FORCE_ROOT, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    *pFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_BUS_RESET_FORCE_ROOT))
                        *pFlags = BUS_RESET_FLAGS_FORCE_ROOT;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // BusResetDlgProc

void
w1394_BusReset(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ULONG   fulFlags;
    DWORD   dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_BusReset\r\n"));

    fulFlags = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "BusReset",
                        hWnd,
                        BusResetDlgProc,
                        (LPARAM)&fulFlags
                        )) {

        dwRet = BusReset( hWnd,
                          szDeviceName,
                          fulFlags
                          );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_BusReset\r\n"));
    return;
} // w1394_BusReset

void
w1394_GetGenerationCount(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ULONG       generationCount;
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_GetGenerationCount\r\n"));

    dwRet = GetGenerationCount( hWnd,
                                szDeviceName,
                                &generationCount
                                );

    TRACE(TL_TRACE, (hWnd, "Exit w1394_GetGenerationCount\r\n"));
    return;
} // w1394_GetGenerationCount

INT_PTR CALLBACK
GetLocalHostInfoDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PGET_LOCAL_HOST_INFORMATION  pGetLocalHostInfo;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pGetLocalHostInfo = (PGET_LOCAL_HOST_INFORMATION)lParam;

            CheckRadioButton( hDlg,
                              IDC_GET_HOST_UNIQUE_ID,
                              IDC_GET_TOPOLOGY_MAP,
                              pGetLocalHostInfo->nLevel + (IDC_GET_HOST_UNIQUE_ID-1)
                              );

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    if (IsDlgButtonChecked(hDlg, IDC_GET_HOST_UNIQUE_ID))
                        pGetLocalHostInfo->nLevel = GET_HOST_UNIQUE_ID;

                    if (IsDlgButtonChecked(hDlg, IDC_GET_HOST_HOST_CAPABILITIES))
                        pGetLocalHostInfo->nLevel = GET_HOST_CAPABILITIES;

                    if (IsDlgButtonChecked(hDlg, IDC_GET_HOST_POWER_SUPPLIED))
                        pGetLocalHostInfo->nLevel = GET_POWER_SUPPLIED;

                    if (IsDlgButtonChecked(hDlg, IDC_GET_HOST_PHYS_ADDR_ROUTINE))
                        pGetLocalHostInfo->nLevel = GET_PHYS_ADDR_ROUTINE;

                    if (IsDlgButtonChecked(hDlg, IDC_GET_HOST_CONFIG_ROM))
                        pGetLocalHostInfo->nLevel = GET_HOST_CONFIG_ROM;

                    // i'm going to piggyback on Status...
                    if (IsDlgButtonChecked(hDlg, IDC_GET_SPEED_MAP)) {

                        pGetLocalHostInfo->nLevel = GET_HOST_CSR_CONTENTS;
                        pGetLocalHostInfo->Status = SPEED_MAP_LOCATION;
                    }

                    if (IsDlgButtonChecked(hDlg, IDC_GET_TOPOLOGY_MAP)) {

                        pGetLocalHostInfo->nLevel = GET_HOST_CSR_CONTENTS;
                        pGetLocalHostInfo->Status = TOPOLOGY_MAP_LOCATION;
                    }

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // GetLocalHostInfoDlgProc

void
w1394_GetLocalHostInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    GET_LOCAL_HOST_INFORMATION  getLocalHostInfo;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_GetLocalHostInformation\r\n"));

    getLocalHostInfo.nLevel = GET_HOST_UNIQUE_ID;
    getLocalHostInfo.ulBufferSize = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "GetLocalHostInfo",
                        hWnd,
                        GetLocalHostInfoDlgProc,
                        (LPARAM)&getLocalHostInfo
                        )) {

        if (getLocalHostInfo.nLevel == 6) {

            PGET_LOCAL_HOST_INFORMATION     GetLocalHostInfo;
            PGET_LOCAL_HOST_INFO6           LocalHostInfo6;
            ULONG                           ulBufferSize;
            ULONG                           CsrDataLength;
            ULONG                           CsrDataAddress;

            // let's get the CsrDataAddress from Status. (from the dialog)
            CsrDataAddress = getLocalHostInfo.Status;

            // first thing is to get the buffer size we need...
            ulBufferSize = sizeof(GET_LOCAL_HOST_INFORMATION) + sizeof(GET_LOCAL_HOST_INFO6);

            GetLocalHostInfo = (PGET_LOCAL_HOST_INFORMATION)LocalAlloc(LPTR, ulBufferSize);
            LocalHostInfo6 = (PGET_LOCAL_HOST_INFO6)&GetLocalHostInfo->Information;

            GetLocalHostInfo->nLevel = 6;
            GetLocalHostInfo->ulBufferSize = ulBufferSize;

            LocalHostInfo6->CsrBaseAddress.Off_High = INITIAL_REGISTER_SPACE_HI;
            LocalHostInfo6->CsrBaseAddress.Off_Low = CsrDataAddress;

            dwRet = GetLocalHostInformation( hWnd,
                                             szDeviceName,
                                             GetLocalHostInfo,
                                             FALSE
                                             );

            if (dwRet == ERROR_INSUFFICIENT_BUFFER) {

                // we should have our buffer info.
                TRACE(TL_TRACE, (hWnd, "Insufficient Buffer\r\n"));
                TRACE(TL_TRACE, (hWnd, "CsrDataLength = 0x%x\r\n", LocalHostInfo6->CsrDataLength));

                CsrDataLength = LocalHostInfo6->CsrDataLength;

                LocalFree(GetLocalHostInfo);

                // start over with a big enough buffer
                ulBufferSize = sizeof(GET_LOCAL_HOST_INFORMATION) + sizeof(GET_LOCAL_HOST_INFO6) + CsrDataLength;

                GetLocalHostInfo = (PGET_LOCAL_HOST_INFORMATION)LocalAlloc(LPTR, ulBufferSize);
                LocalHostInfo6 = (PGET_LOCAL_HOST_INFO6)&GetLocalHostInfo->Information;

                GetLocalHostInfo->nLevel = 6;
                GetLocalHostInfo->ulBufferSize = ulBufferSize;

                LocalHostInfo6->CsrBaseAddress.Off_High = INITIAL_REGISTER_SPACE_HI;
                LocalHostInfo6->CsrBaseAddress.Off_Low = CsrDataAddress;
                LocalHostInfo6->CsrDataLength = CsrDataLength;

                dwRet = GetLocalHostInformation( hWnd,
                                                 szDeviceName,
                                                 GetLocalHostInfo,
                                                 FALSE
                                                 );

                if (LocalHostInfo6->CsrBaseAddress.Off_Low == SPEED_MAP_LOCATION) {

                    PSPEED_MAP                      SpeedMap;
                    PGET_LOCAL_HOST_INFORMATION     pGetTopologyMap;
                    PGET_LOCAL_HOST_INFO6           pTopologyInfo;
                    PTOPOLOGY_MAP                   TopologyMap;
                    ULONG                           NodeCount, i, j;

                    SpeedMap = (PSPEED_MAP)&LocalHostInfo6->CsrDataBuffer;

                    // get the node count from the topology map
                    pGetTopologyMap = LocalAlloc(LPTR, sizeof(GET_LOCAL_HOST_INFO6) + 4096);

                    pGetTopologyMap->ulBufferSize = sizeof(GET_LOCAL_HOST_INFO6) + 4096;
                    pGetTopologyMap->nLevel = 6;
                    pTopologyInfo = (PGET_LOCAL_HOST_INFO6)&pGetTopologyMap->Information;

                    pTopologyInfo->CsrDataLength = 4096; //0x400;
                    pTopologyInfo->CsrBaseAddress.Off_High = INITIAL_REGISTER_SPACE_HI;
                    pTopologyInfo->CsrBaseAddress.Off_Low = TOPOLOGY_MAP_LOCATION;

                    dwRet = GetLocalHostInformation( NULL,
                                                     szDeviceName,
                                                     pGetTopologyMap,
                                                     FALSE
                                                     );

                    TopologyMap = (PTOPOLOGY_MAP)&pTopologyInfo->CsrDataBuffer;
                    NodeCount = TopologyMap->TOP_Node_Count;

                    LocalFree(TopologyMap);

                    if (!dwRet) {

                        TRACE(TL_TRACE, (hWnd, "SpeedCodes = \t"));

                        for (i=0; i < NodeCount; i++) {

                            for (j=0; j < NodeCount; j++) {

                                TRACE(TL_TRACE, (hWnd, "%d\t", SpeedMap->SPD_Speed_Code[i*64+j]));
                            }

                            TRACE(TL_TRACE, (hWnd, "\r\n\t\t"));
                        }
                    }
                    else {

                        TRACE(TL_ERROR, (hWnd, "Error getting NodeCount\r\n"));
                    }
                    TRACE(TL_TRACE, (hWnd, "\r\n"));
                }

                LocalFree(GetLocalHostInfo);

            }
            else
                LocalFree(GetLocalHostInfo);
        }
        else {

            dwRet = GetLocalHostInformation( hWnd,
                                             szDeviceName,
                                             &getLocalHostInfo,
                                             TRUE
                                             );
        }
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_GetLocalHostInformation\r\n"));
    return;
} // w1394_GetLocalHostInfo

INT_PTR CALLBACK
Get1394AddressFromDeviceObjectDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PGET_1394_ADDRESS    pGet1394Address;
    static CHAR                 tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pGet1394Address = (PGET_1394_ADDRESS)lParam;

            if (pGet1394Address->fulFlags & USE_LOCAL_NODE)
                CheckDlgButton( hDlg, IDC_GET_ADDR_USE_LOCAL_NODE, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    pGet1394Address->fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_GET_ADDR_USE_LOCAL_NODE))
                        pGet1394Address->fulFlags |= USE_LOCAL_NODE;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // Get1394AddressFromDeviceObjectDlgProc

void
w1394_Get1394AddressFromDeviceObject(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    GET_1394_ADDRESS    get1394Address;
    DWORD               dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_Get1394AddressFromDeviceObject\r\n"));

    get1394Address.fulFlags = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "Get1394AddrFromDeviceObject",
                        hWnd,
                        Get1394AddressFromDeviceObjectDlgProc,
                        (LPARAM)&get1394Address
                        )) {

        dwRet = Get1394AddressFromDeviceObject( hWnd,
                                                szDeviceName,
                                                &get1394Address
                                                );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_Get1394AddressFromDeviceObject\r\n"));
    return;
} // w1394_Get1394AddressFromDeviceObject

void
w1394_Control(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_Control\r\n"));

    dwRet = Control( hWnd,
                     szDeviceName
                     );

    TRACE(TL_TRACE, (hWnd, "Exit w1394_Control\r\n"));
    return;
} // w1394_Control

INT_PTR CALLBACK
GetMaxSpeedBetweenDevicesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PGET_MAX_SPEED_BETWEEN_DEVICES   pGetMaxSpeed;
    static CHAR                             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pGetMaxSpeed = (PGET_MAX_SPEED_BETWEEN_DEVICES)lParam;

            if (pGetMaxSpeed->fulFlags & USE_LOCAL_NODE)
                CheckDlgButton( hDlg, IDC_GET_MAX_USE_LOCAL_NODE, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    pGetMaxSpeed->fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_GET_MAX_USE_LOCAL_NODE))
                        pGetMaxSpeed->fulFlags |= USE_LOCAL_NODE;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // GetMaxSpeedBetweenDevicesDlgProc

void
w1394_GetMaxSpeedBetweenDevices(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    GET_MAX_SPEED_BETWEEN_DEVICES   getMaxSpeed;
    DWORD                           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_GetMaxSpeedBetweenDevices\r\n"));

    getMaxSpeed.fulFlags = 0;
    getMaxSpeed.ulNumberOfDestinations = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "GetMaxSpeed",
                        hWnd,
                        GetMaxSpeedBetweenDevicesDlgProc,
                        (LPARAM)&getMaxSpeed
                        )) {

        dwRet = GetMaxSpeedBetweenDevices( hWnd,
                                           szDeviceName,
                                           &getMaxSpeed
                                           );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_GetMaxSpeedBetweenDevices\r\n"));
    return;
} // w1394_GetMaxSpeedBetweenDevices

void
w1394_GetConfigurationInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_GetConfigurationInfo\r\n"));

    dwRet = GetConfigurationInformation( hWnd,
                                         szDeviceName
                                         );

    TRACE(TL_TRACE, (hWnd, "Exit w1394_GetConfigurationInfo\r\n"));
    return;
} // w1394_GetConfigurationInfo

INT_PTR CALLBACK
SetDeviceXmitPropertiesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PDEVICE_XMIT_PROPERTIES      pDeviceXmitProperties;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pDeviceXmitProperties = (PDEVICE_XMIT_PROPERTIES)lParam;

            if (pDeviceXmitProperties->fulSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_SET_DEVICE_XMIT_100MBPS,
                                  IDC_SET_DEVICE_XMIT_FASTEST,
                                  IDC_SET_DEVICE_XMIT_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_SET_DEVICE_XMIT_100MBPS,
                                  IDC_SET_DEVICE_XMIT_FASTEST,
                                  pDeviceXmitProperties->fulSpeed + (IDC_SET_DEVICE_XMIT_100MBPS-1)
                                  );
            }

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    pDeviceXmitProperties->fulSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_SET_DEVICE_XMIT_100MBPS))
                        pDeviceXmitProperties->fulSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_SET_DEVICE_XMIT_200MBPS))
                        pDeviceXmitProperties->fulSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_SET_DEVICE_XMIT_400MBPS))
                        pDeviceXmitProperties->fulSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_SET_DEVICE_XMIT_1600MBPS))
                        pDeviceXmitProperties->fulSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_SET_DEVICE_XMIT_FASTEST))
                        pDeviceXmitProperties->fulSpeed = SPEED_FLAGS_FASTEST;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // SetDeviceXmitPropertiesDlgProc

void
w1394_SetDeviceXmitProperties(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    DEVICE_XMIT_PROPERTIES      deviceXmitProperties;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_SetDeviceXmitProperties\r\n"));

    deviceXmitProperties.fulSpeed = SPEED_FLAGS_200;
    deviceXmitProperties.fulPriority = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "SetDeviceXmitProperties",
                        hWnd,
                        SetDeviceXmitPropertiesDlgProc,
                        (LPARAM)&deviceXmitProperties
                        )) {

        dwRet = SetDeviceXmitProperties( hWnd,
                                         szDeviceName,
                                         &deviceXmitProperties
                                         );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_SetDeviceXmitProperties\r\n"));
    return;
} // w1394_SetDeviceXmitProperties

INT_PTR CALLBACK
SendPhyConfigDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PPHY_CONFIGURATION_PACKET    pPhyConfigPacket;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pPhyConfigPacket = (PPHY_CONFIGURATION_PACKET)lParam;

            _ultoa(pPhyConfigPacket->PCP_Phys_ID, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_PHYS_ID, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Packet_ID, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_PACKET_ID, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Gap_Count, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_GAP_COUNT, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Set_Gap_Count, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_SET_GAP_COUNT, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Force_Root, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_FORCE_ROOT, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Reserved1, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_RESERVED1, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Reserved2, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_RESERVED2, tmpBuff);

            _ultoa(pPhyConfigPacket->PCP_Inverse, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_SEND_PHY_INVERSE, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

               case IDOK:

                    GetDlgItemText(hDlg, IDC_SEND_PHY_PHYS_ID, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Phys_ID = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_PACKET_ID, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Packet_ID = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_GAP_COUNT, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Gap_Count = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_SET_GAP_COUNT, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Set_Gap_Count = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_FORCE_ROOT, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Force_Root = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_RESERVED1, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Reserved1 = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_RESERVED2, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Reserved2 = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_SEND_PHY_INVERSE, tmpBuff, STRING_SIZE);
                    pPhyConfigPacket->PCP_Inverse = strtoul(tmpBuff, NULL, 16);

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // SendPhyConfigDlgProc

void
w1394_SendPhyConfigPacket(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    PHY_CONFIGURATION_PACKET    phyConfigPacket;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_SendPhyConfigPacket\r\n"));

    phyConfigPacket.PCP_Phys_ID = 0;
    phyConfigPacket.PCP_Packet_ID = 0;
    phyConfigPacket.PCP_Gap_Count = 0;
    phyConfigPacket.PCP_Set_Gap_Count = 0;
    phyConfigPacket.PCP_Force_Root = 0;
    phyConfigPacket.PCP_Reserved1 = 0;
    phyConfigPacket.PCP_Reserved2 = 0;
    phyConfigPacket.PCP_Inverse = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "SendPhyConfigPacket",
                        hWnd,
                        SendPhyConfigDlgProc,
                        (LPARAM)&phyConfigPacket
                        )) {

        dwRet = SendPhyConfigurationPacket( hWnd,
                                            szDeviceName,
                                            &phyConfigPacket
                                            );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_SendPhyConfigPacket\r\n"));
    return;
} // w1394_SendPhyConfigPacket

INT_PTR CALLBACK
BusResetNotificationDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PULONG   pFlags;
    static CHAR     tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pFlags = (PULONG)lParam;

            CheckRadioButton( hDlg,
                              IDC_BUS_RESET_NOTIFY_REGISTER,
                              IDC_BUS_RESET_NOTIFY_DEREGISTER,
                              *pFlags + (IDC_BUS_RESET_NOTIFY_REGISTER-1)
                              );

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    if (IsDlgButtonChecked(hDlg, IDC_BUS_RESET_NOTIFY_REGISTER))
                        *pFlags = REGISTER_NOTIFICATION_ROUTINE;

                    if (IsDlgButtonChecked(hDlg, IDC_BUS_RESET_NOTIFY_DEREGISTER))
                        *pFlags = DEREGISTER_NOTIFICATION_ROUTINE;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // BusResetNotificationDlgProc

void
w1394_BusResetNotification(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ULONG   fulFlags;
    DWORD   dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_BusResetNotification\r\n"));

    fulFlags = REGISTER_NOTIFICATION_ROUTINE;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "BusResetNotification",
                        hWnd,
                        BusResetNotificationDlgProc,
                        (LPARAM)&fulFlags
                        )) {

        dwRet = BusResetNotification( hWnd,
                                      szDeviceName,
                                      fulFlags
                                      );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_BusResetNotification\r\n"));
    return;
} // w1394_BusResetNotification

INT_PTR CALLBACK
SetLocalHostInfoDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PSET_LOCAL_HOST_INFORMATION      pSetLocalHostInfo;
    static PSET_LOCAL_HOST_PROPS2           pSetLocalHostProps2;
    static PSET_LOCAL_HOST_PROPS3           pSetLocalHostProps3;
    static CHAR                             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pSetLocalHostInfo = (PSET_LOCAL_HOST_INFORMATION)lParam;

            CheckRadioButton( hDlg,
                              IDC_SET_LOCAL_HOST_LEVEL_GAP_COUNT,
                              IDC_SET_LOCAL_HOST_LEVEL_CROM,
                              pSetLocalHostInfo->nLevel + (IDC_SET_LOCAL_HOST_LEVEL_GAP_COUNT-2)
                              );

            CheckRadioButton( hDlg,
                              IDC_SET_LOCAL_HOST_CROM_ADD,
                              IDC_SET_LOCAL_HOST_CROM_REMOVE,
                              IDC_SET_LOCAL_HOST_CROM_ADD
                              );

            if (pSetLocalHostInfo->nLevel == SET_LOCAL_HOST_PROPERTIES_GAP_COUNT) {

                pSetLocalHostProps2 = (PSET_LOCAL_HOST_PROPS2)pSetLocalHostInfo->Information;

                _ultoa(pSetLocalHostProps2->GapCountLowerBound, tmpBuff, 16);
                SetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_GAP_COUNT, tmpBuff);
            }

            if (pSetLocalHostInfo->nLevel == SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM) {

                pSetLocalHostProps3 = (PSET_LOCAL_HOST_PROPS3)pSetLocalHostInfo->Information;

                CheckRadioButton( hDlg,
                                  IDC_SET_LOCAL_HOST_CROM_ADD,
                                  IDC_SET_LOCAL_HOST_CROM_REMOVE,
                                  pSetLocalHostProps3->fulFlags + (IDC_SET_LOCAL_HOST_CROM_ADD-1)
                                  );

                //_ultoa((ULONG)((ULONG_PTR)pSetLocalHostProps3->hCromData), tmpBuff, 16);
                sprintf (tmpBuff, "%p", pSetLocalHostProps3->hCromData);
                SetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_CROM_HCROMDATA, tmpBuff);

                _ultoa(pSetLocalHostProps3->nLength, tmpBuff, 16);
                SetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_CROM_NLENGTH, tmpBuff);
            }

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    if (IsDlgButtonChecked(hDlg, IDC_SET_LOCAL_HOST_LEVEL_GAP_COUNT)) {

                        pSetLocalHostInfo->nLevel = SET_LOCAL_HOST_PROPERTIES_GAP_COUNT;

                        GetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_GAP_COUNT, tmpBuff, STRING_SIZE);
                        pSetLocalHostProps2->GapCountLowerBound = strtoul(tmpBuff, NULL, 16);

                    }

                    if (IsDlgButtonChecked(hDlg, IDC_SET_LOCAL_HOST_LEVEL_CROM)) {

                        pSetLocalHostInfo->nLevel = SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM;

                        if (IsDlgButtonChecked(hDlg, IDC_SET_LOCAL_HOST_CROM_ADD))
                            pSetLocalHostProps3->fulFlags = SLHP_FLAG_ADD_CROM_DATA;

                        if (IsDlgButtonChecked(hDlg, IDC_SET_LOCAL_HOST_CROM_REMOVE))
                            pSetLocalHostProps3->fulFlags = SLHP_FLAG_REMOVE_CROM_DATA;

                        GetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_CROM_HCROMDATA, tmpBuff, STRING_SIZE);
                        if (!sscanf (tmpBuff, "%p", &(pSetLocalHostProps3->hCromData)))
                        {
                            // failed to get the handle, just return here
                            EndDialog(hDlg, TRUE);
                            return FALSE;
                        }

                        GetDlgItemText(hDlg, IDC_SET_LOCAL_HOST_CROM_NLENGTH, tmpBuff, STRING_SIZE);
                        pSetLocalHostProps3->nLength = strtoul(tmpBuff, NULL, 16);

                        // let's set the correct buffer size...
                        pSetLocalHostInfo->ulBufferSize = sizeof(SET_LOCAL_HOST_INFORMATION)+sizeof(SET_LOCAL_HOST_PROPS3)+pSetLocalHostProps3->nLength;
                    }

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // SetLocalHostInfoDlgProc

void
w1394_SetLocalHostInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    PSET_LOCAL_HOST_INFORMATION     SetLocalHostInfo;
    PSET_LOCAL_HOST_PROPS2          SetLocalHostProps2;
    PSET_LOCAL_HOST_PROPS3          SetLocalHostProps3;
    DWORD                           dwRet;
    ULONG                           ulBufferSize;
    PULONG                          UnitDir;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_SetLocalHostInfo\r\n"));
/*
    ulBufferSize = sizeof(SET_LOCAL_HOST_INFORMATION)+sizeof(SET_LOCAL_HOST_PROPS2);

    SetLocalHostInfo = (PSET_LOCAL_HOST_INFORMATION)LocalAlloc(LPTR, ulBufferSize);

    SetLocalHostInfo->nLevel = SET_LOCAL_HOST_PROPERTIES_GAP_COUNT;
    SetLocalHostInfo->ulBufferSize = ulBufferSize;

    SetLocalHostProps2 = (PSET_LOCAL_HOST_PROPS2)SetLocalHostInfo->Information;

    SetLocalHostProps2->GapCountLowerBound = 0;
*/
    ulBufferSize = sizeof(SET_LOCAL_HOST_INFORMATION)+sizeof(SET_LOCAL_HOST_PROPS3)+8;//4096;

    SetLocalHostInfo = (PSET_LOCAL_HOST_INFORMATION)LocalAlloc(LPTR, ulBufferSize);
    if (!SetLocalHostInfo) {

        dwRet = GetLastError();
        TRACE(TL_ERROR, (hWnd, "Unable to allocate SetLocalHostInfo!\r\n"));
        goto Exit_SetLocalHostInfo;
    }

    SetLocalHostInfo->nLevel = SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM;
    SetLocalHostInfo->ulBufferSize = ulBufferSize;

    SetLocalHostProps3 = (PSET_LOCAL_HOST_PROPS3)SetLocalHostInfo->Information;

    SetLocalHostProps3->fulFlags = SLHP_FLAG_ADD_CROM_DATA;
    SetLocalHostProps3->hCromData = 0;
    SetLocalHostProps3->nLength = 12;

    UnitDir = (PULONG)&SetLocalHostProps3->Buffer;

    *UnitDir = 0xEFBE0200;
    UnitDir++;
    *UnitDir = 0x0A000012;
    UnitDir++;
    *UnitDir = 0x0B000013;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "SetLocalHostInfo",
                        hWnd,
                        SetLocalHostInfoDlgProc,
                        (LPARAM)SetLocalHostInfo
                        )) {

        dwRet = SetLocalHostInformation( hWnd,
                                         szDeviceName,
                                         SetLocalHostInfo
                                         );
    }

Exit_SetLocalHostInfo:

    if (SetLocalHostInfo)
        LocalFree(SetLocalHostInfo);

    TRACE(TL_TRACE, (hWnd, "Exit w1394_SetLocalHostInfo\r\n"));
    return;
} // w1394_SetLocalHostInfo


