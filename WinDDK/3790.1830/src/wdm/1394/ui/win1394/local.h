/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    local.h

Abstract


Author:

    Peter Binder (pbinder) 4/22/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/22/98  pbinder   birth
4/22/98  pbinder   taken from template for 1394test
5/04/98  pbinder   taken for win1394
--*/

#define STRING_SIZE             1024

#define APP_TIMER_ID            0x0001
#define EDIT_ID                 0x0001

#ifdef _WIN1394_C

HINSTANCE               g_hInstance;
HWND                    g_hWndEdit;

PSTR                    szBusName = "\\\\.\\1394BUS0";

DEVICE_DATA             DeviceData;
DEVICE_DATA             VirtDeviceData;

PSTR                    SelectedDevice;

ASYNC_LOOPBACK_PARAMS           asyncLoopbackParams;
ISOCH_LOOPBACK_PARAMS           isochLoopbackParams;
ASYNC_STREAM_LOOPBACK_PARAMS    streamLoopbackParams;

BOOL                    asyncLoopbackStarted = FALSE;
BOOL                    isochLoopbackStarted = FALSE;
BOOL                    streamLoopbackStarted = FALSE;

SYSTEMTIME              AsyncStartTime;
SYSTEMTIME              IsochStartTime;
SYSTEMTIME              StreamStartTime;

#else

extern HINSTANCE        g_hInstance;
extern HWND             g_hWndEdit;

extern PDEVICE_DATA     DeviceData;
extern SYSTEMTIME       AsyncStartTime;
extern SYSTEMTIME       IsochStartTime;
extern SYSTEMTIME       StreamStartTime;

extern BOOL             asyncLoopbackStarted;
extern BOOL             isochLoopbackStarted;
extern BOOL             streamLoopbackStarted;

#endif // _WIN1394_C

INT_PTR CALLBACK
SelectDeviceDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_SelectTestDevice(
    HWND        hWnd
    );

INT_PTR CALLBACK
w1394_AboutDlgProc(
    HWND    hDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

LRESULT CALLBACK
w1394_AppWndProc(
    HWND    hWnd,
    UINT    iMsg,
    WPARAM  wParam,
    LPARAM  lParam
    );

int WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    PSTR        szCmdLine,
    int         iCmdShow
    );


