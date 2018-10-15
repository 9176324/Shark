/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    1394.h

Abstract

    1394 api wrappers

Author:

    Peter Binder (pbinder) 5/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/13/98 pbinder   birth
--*/

INT_PTR CALLBACK
BusResetDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_BusReset(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_GetGenerationCount(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
GetLocalHostInfoDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_GetLocalHostInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
Get1394AddressFromDeviceObjectDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_Get1394AddressFromDeviceObject(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_Control(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
GetMaxSpeedBetweenDevicesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_GetMaxSpeedBetweenDevices(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_GetConfigurationInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
SetDeviceXmitPropertiesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_SetDeviceXmitProperties(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
SendPhyConfigDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_SendPhyConfigPacket(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_GetSpeedTopologyMaps(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
BusResetNotificationDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_BusResetNotification(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
SetLocalHostInfoDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_SetLocalHostInfo(
    HWND    hWnd,
    PSTR    szDeviceName
    );


