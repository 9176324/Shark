/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    win1394.c

Abstract

    1394 Test Application.

Author:

    Peter Binder (pbinder) 5/4/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/04/98 pbinder   birth, taken from old win1394
08/18/98 pbinder   changed for new dialogs
--*/

#define _WIN1394_C
#include "pch.h"
#include "tchar.h"
#undef _WIN1394_C

INT_PTR CALLBACK
SelectDeviceDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static HWND         hWndListBox;
    CHAR                tmpBuff[256];
    static INT          nIndex;
    ULONG               i;

    switch (uMsg) {

        case WM_INITDIALOG:

            // get a handle to our listbox
            hWndListBox = GetDlgItem(hDlg, IDC_1394_DEVICES);

            // let's plug all of our devices into the list box
            for (i=0;i<DeviceData.numDevices;i++) {

                strncpy (tmpBuff, DeviceData.deviceList[i].DeviceName, 255);
                tmpBuff[255] = '\0';
                SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)tmpBuff);

                // see if this is our selected device
                if (lstrcmp(SelectedDevice, DeviceData.deviceList[i].DeviceName) == 0) {

                    // it is, lets select it...
                    SendMessage(hWndListBox, LB_SETCURSEL, 0, 0);
                }
            }

            // save the original device under test...
            nIndex = (INT)SendMessage(hWndListBox, LB_GETCURSEL, 0, 0);

            return(TRUE);

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDC_1394_DEVICES:

                    if (HIWORD(wParam) != LBN_DBLCLK)
                        return(TRUE);

                    // just fall through here, it's the same thing if they
                    // double clicked on device

                case IDOK:

                    // get the new device under test
                    nIndex = (INT)SendMessage(hWndListBox,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    // make sure it's a valid index
                    if (nIndex != -1)
                        SelectedDevice = DeviceData.deviceList[nIndex].DeviceName;

                    EndDialog(hDlg, TRUE);
                    return(TRUE);

                case IDCANCEL:

                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                default:
                    return(TRUE);
            }

            break;

        default:
            break;
    }

    return(FALSE);
} // SelectDeviceDlgProc

void
w1394_SelectTestDevice(
    HWND        hWnd
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_SelectTestDevice\r\n"));

    if (DialogBox( (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                   "SelectDevice",
                    hWnd,
                    SelectDeviceDlgProc
                    )) {

        TRACE(TL_TRACE, (hWnd, "Selected Device = %s\r\n", SelectedDevice));
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_SelectTestDevice\r\n"));
    return;
} // w1394_SelectTestDevice


INT_PTR CALLBACK
SelectVirtualDeviceDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static HWND         hWndListBox;
    CHAR                tmpBuff[256];
    static INT          nIndex;
    ULONG               i;

    switch (uMsg) {

        case WM_INITDIALOG:

            // get a handle to our listbox
            hWndListBox = GetDlgItem(hDlg, IDC_1394_DEVICES);

            // let's plug all of our devices into the list box
            for (i=0;i<VirtDeviceData.numDevices;i++) {

                strncpy (tmpBuff, VirtDeviceData.deviceList[i].DeviceName, 255);
                tmpBuff[255] = '\0';
                SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)tmpBuff);

                // see if this is our selected device
                if (lstrcmp(SelectedDevice, VirtDeviceData.deviceList[i].DeviceName) == 0) {

                    // it is, lets select it...
                    SendMessage(hWndListBox, LB_SETCURSEL, 0, 0);
                }
            }

            // save the original device under test...
            nIndex = (INT)SendMessage(hWndListBox, LB_GETCURSEL, 0, 0);

            return(TRUE);

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDC_1394_DEVICES:

                    if (HIWORD(wParam) != LBN_DBLCLK)
                        return(TRUE);

                    // just fall through here, it's the same thing if they
                    // double clicked on device

                case IDOK:

                    // get the new device under test
                    nIndex = (INT)SendMessage(hWndListBox,
                                              LB_GETCURSEL,
                                              0,
                                              0);

                    // make sure it's a valid index
                    if (nIndex != -1)
                        SelectedDevice = VirtDeviceData.deviceList[nIndex].DeviceName;

                    EndDialog(hDlg, TRUE);
                    return(TRUE);

                case IDCANCEL:

                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                default:
                    return(TRUE);
            }

            break;

        default:
            break;
    }

    return(FALSE);
} // SelectDeviceDlgProc

void
w1394_SelectVirtualTestDevice(
    HWND        hWnd
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_SelectVirtualTestDevice\r\n"));

    if (DialogBox( (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                    "SelectVirtualDevice",
                    hWnd,
                    SelectVirtualDeviceDlgProc
                    )) {

        TRACE(TL_TRACE, (hWnd, "Selected Device = %s\r\n", SelectedDevice));
    }
    
    TRACE(TL_TRACE, (hWnd, "Exit w1394_SelectVirtualTestDevice\r\n"));
    return;
} // w1394_SelectVirtualTestDevice

void
w1394_AddVirtualDriver (
    HWND        hWnd
    )
{
    VIRT_DEVICE     virtDevice;
    UCHAR           DeviceID[32] = "1394_VIRTUAL_DEVICE";
    ULONG           busNumber = 0;
    DWORD           dwRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter w1394_AddVirtualDriver\r\n"));

    virtDevice.fulFlags = 0;
    virtDevice.InstanceID.HighPart = 0;
    virtDevice.InstanceID.LowPart = 0;
    virtDevice.DeviceID = DeviceID;

    do {
        dwRet = AddVirtualDriver (hWnd, &virtDevice, busNumber);

        if (dwRet == ERROR_SUCCESS) {

            TRACE(TL_TRACE, (hWnd, "Virtual Driver successfully added on 1394Bus%i\r\n", busNumber));
        }

        virtDevice.InstanceID.LowPart++;
        busNumber++;
        
    } while (dwRet == ERROR_SUCCESS);

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AddVirtualDriver\r\n"));
    return;
}

void
w1394_RemoveVirtualDriver(
    HWND        hWnd
    )
{
    VIRT_DEVICE     virtDevice;
    UCHAR           DeviceID[32] = "1394_VIRTUAL_DEVICE";
    ULONG           busNumber = 0;
    DWORD           dwRet;
    
    TRACE(TL_TRACE, (hWnd, "Enter w1394_RemoveVirtualDriver\r\n"));

    virtDevice.fulFlags = 0;
    virtDevice.InstanceID.HighPart = 0;
    virtDevice.InstanceID.LowPart = 0;
    virtDevice.DeviceID = DeviceID;

    do {
        dwRet = RemoveVirtualDriver (hWnd, &virtDevice, busNumber);

        if (dwRet == ERROR_SUCCESS) {

            TRACE(TL_TRACE, (hWnd, "Virtual Driver successfully removed on 1394Bus%i\r\n", busNumber));
        }

        virtDevice.InstanceID.LowPart++;
        busNumber++;
        
    } while (dwRet == ERROR_SUCCESS);

    TRACE(TL_TRACE, (hWnd, "Exit w1394_RemoveVirtualDriver\r\n"));
    return;
}

INT_PTR CALLBACK
w1394_AboutDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
/*++

Routine Description:

    This is the DlgProc that we use for the About Box.

--*/
{
    HICON   hIcon;

    switch (uMsg) {

        case WM_INITDIALOG:

            // display initial icon
            hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

            SendDlgItemMessage(hDlg, IDI_APP_ICON, STM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hIcon);

            return TRUE;

        case WM_DESTROY:
            return TRUE;

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:
                    EndDialog(hDlg, TRUE);

                default:
                    return TRUE;
            }
            break;

        default:
            break;
    }

    return FALSE;
} // w1394_AboutDlgProc

#ifdef LOGGING
BOOL DoNT5Checking(void)
{
    TCHAR msg[]         = _T("You appear to be running an NT5 build with pool tagging turned OFF!.\n")
                          _T("Win1394 will not run on your machine unless you turn this on.");

    TCHAR msg2[]        = _T("You do not have Driver Verification enabled.\n")
                          _T("You need to enable this to run Win1394.");
                    
    TCHAR regKeyName[]  = _T("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");                  
    
    BOOL status = TRUE;
    OSVERSIONINFO os;
    //also need to check for the VerifyDrivers Key.
    HKEY hKey;

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&os);

    //check for NT5
    if (os.dwMajorVersion == 5)
    {
        if (GetSystemMetrics(SM_DEBUG)) //checks for debug version of user
        {
            HKEY hKey;
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKeyName, 0, KEY_READ, &hKey))
            {
                //key exists - halfway there
                DWORD size = sizeof(DWORD);
                DWORD lpData;
                if (ERROR_SUCCESS != RegQueryValueEx(hKey, _T("PoolTag"), NULL, NULL, (LPBYTE)&lpData, &size))
                {
                    status = FALSE;
                    MessageBox(NULL, msg, "Win1394", MB_OK);
                } 
                RegCloseKey(hKey);
            }
        }

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, regKeyName, 0, KEY_READ, &hKey))
        {
            //key exists - halfway there
            DWORD size = 256;
            TCHAR lpData[256];
            if (ERROR_SUCCESS != RegQueryValueEx(hKey, _T("VerifyDrivers"), NULL, NULL, (LPBYTE)&lpData, &size))
            {
                status = FALSE;
                MessageBox(NULL, msg2, "Win1394", MB_OK);
            } 
            RegCloseKey(hKey);
        }

    }
    return status;
} 
#endif

LRESULT CALLBACK
w1394_AppWndProc(
    HWND    hWnd,
    UINT    iMsg,
    WPARAM  wParam,
    LPARAM  lParam
    )
/*++

Routine Description:

    This is the application main WndProc. Here we will handle
    all messages sent to the application.

--*/
{
    ULONG   i;

    switch(iMsg) {

        case WM_CREATE:

#ifdef LOGGING
            if (!DoNT5Checking()) {

                PostMessage(hWnd, WM_DESTROY, 0, 0);
                return DefWindowProc(hWnd, iMsg, wParam, lParam);
            }
#endif
            // create an edit control for the main app.
            g_hWndEdit = CreateWindow( "edit",
                                       NULL,
                                       WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                       WS_BORDER | ES_LEFT | ES_MULTILINE | ES_WANTRETURN |
                                       ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY,
                                       0,
                                       0,
                                       0,
                                       0,
                                       hWnd,
                                       (HMENU) EDIT_ID,
                                       g_hInstance,
                                       NULL
                                       );

            // set a timer, 1 time a second, we will print loopback test results with this
            SetTimer(hWnd, APP_TIMER_ID, 1000, NULL);

            SelectedDevice = NULL;

            RegisterClient(hWnd);

        break; // WM_CREATE

        case WM_DESTROY:

            DeRegisterClient(hWnd);

            PostQuitMessage(0);
            break; // WM_DESTROY

        case WM_SIZE:

            MoveWindow(g_hWndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break; // WM_SIZE

        case WM_TIMER:

            // if async loopback test is running, print results
            if (asyncLoopbackStarted && !asyncLoopbackParams.bKill) {
                TRACE(TL_TRACE, (g_hWndEdit, "Async - Iterations = 0x%x\t\tPass = 0x%x\t\tFail = 0x%x\r\n", asyncLoopbackParams.ulIterations, asyncLoopbackParams.ulPass, asyncLoopbackParams.ulFail));
            }

            // if isoch loopback test is running, print results
            if (isochLoopbackStarted && !isochLoopbackParams.bKill) {
                TRACE(TL_TRACE, (g_hWndEdit, "Isoch - Iterations = 0x%x\t\tPass = 0x%x\t\tFail = 0x%x\r\n", isochLoopbackParams.ulIterations, isochLoopbackParams.ulPass, isochLoopbackParams.ulFail));
            }

            // if async stream loopback test is running, print results
            if (streamLoopbackStarted && !streamLoopbackParams.bKill) {
                TRACE(TL_TRACE, (g_hWndEdit, "Async Stream - Iterations = 0x%x\t\tPass = 0x%x\t\tFail = 0x%x\r\n", streamLoopbackParams.ulIterations, streamLoopbackParams.ulPass, streamLoopbackParams.ulFail));
            }

            break; // WM_TIMER

        case WM_COMMAND:

            switch(LOWORD(wParam)) {

                case EDIT_ID:

                    switch (HIWORD(wParam)) {

                        // check fot text overflow in edit control
                        case EN_ERRSPACE:
                        case EN_MAXTEXT:

                            // just clear out the buffer
                            SetWindowText(g_hWndEdit, "\0");
                            break;

                        default:
                            break;
                    }
                break;

                case IDM_CLEAR_BUFFER:

                    SetWindowText(g_hWndEdit, "\0");
                    break; // IDM_CLEAR_BUFFER

                case IDM_ABOUT:
                    DialogBox(g_hInstance, "IDD_ABOUTBOX", hWnd, w1394_AboutDlgProc);
                    break; // IDM_ABOUT

                case IDM_EXIT:

                    SendMessage(hWnd, WM_DESTROY, 0, 0L);
                    break; // IDM_EXIT

                case IDM_SELECT_TEST_DEVICE:

                    if (DeviceData.numDevices) {

                        w1394_SelectTestDevice(g_hWndEdit);
                    }
                    else {

                        TRACE(TL_TRACE, (g_hWndEdit, "No available test device.\r\n"));
                    }
                    break; // IDM_SELECT_TEST_DEVICE

                case IDM_ADD_VIRTUAL_DRIVER:

                    w1394_AddVirtualDriver (g_hWndEdit);

                    break; // IDM_ADD_VIRTUAL_DRIVER

                case IDM_REMOVE_VIRTUAL_DRIVER:

                    w1394_RemoveVirtualDriver (g_hWndEdit);

                    break; // IDM_ADD_VIRTUAL_DRIVER

                case IDM_SELECT_VIRTUAL_TEST_DEVICE:

                    if (VirtDeviceData.numDevices) {

                        w1394_SelectVirtualTestDevice(g_hWndEdit);
                    }
                    else {

                        TRACE(TL_TRACE, (g_hWndEdit, "No available virtual test devices.\r\n"));
                    }
                    break; // IDM_SELECT_VIRT_TEST_DEVICE

                case IDM_ENABLE_DIAGNOSTIC_MODE:

                    DiagnosticMode(g_hWndEdit, szBusName, TRUE, TRUE);
                    break; // IDM_ENABLE_DIAGNOSTIC_MODE

                case IDM_DISABLE_DIAGNOSTIC_MODE:

                    DiagnosticMode(g_hWndEdit, szBusName, FALSE, TRUE);
                    break; // IDM_DISABLE_DIAGNOSTIC_MODE


                case IDM_ALLOCATE_ADDRESS_RANGE:

                    w1394_AllocateAddressRange(g_hWndEdit, SelectedDevice);

                    break; // IDM_ALLOCATE_ADDRESS_RANGE

                case IDM_FREE_ADDRESS_RANGE:

                    w1394_FreeAddressRange(g_hWndEdit, SelectedDevice);

                    break; // IDM_FREE_ADDRESS_RANGE

                case IDM_ASYNC_READ:

                    w1394_AsyncRead(g_hWndEdit, SelectedDevice);

                    break; // IDM_ASYNC_READ

                case IDM_ASYNC_WRITE:

                    w1394_AsyncWrite(g_hWndEdit, SelectedDevice);

                    break; // IDM_ASYNC_WRITE

                case IDM_ASYNC_LOCK:

                    w1394_AsyncLock(g_hWndEdit, SelectedDevice);

                    break; // IDM_ASYNC_LOCK

                case IDM_ASYNC_STREAM:

                    w1394_AsyncStream(g_hWndEdit, SelectedDevice);

                    break; // IDM_ASYNC_STREAM

                case IDM_ASYNC_START_LOOPBACK:

                    w1394_AsyncStartLoopback(g_hWndEdit, SelectedDevice, &asyncLoopbackParams);

                    break; // IDM_ASYNC_LOOPBACK

                case IDM_ASYNC_STOP_LOOPBACK:

                    w1394_AsyncStopLoopback(g_hWndEdit, &asyncLoopbackParams);

                    break; // IDM_ASYNC_LOOPBACK


                case IDM_ASYNC_STREAM_START_LOOPBACK:

                    w1394_AsyncStreamStartLoopback(g_hWndEdit, SelectedDevice, &streamLoopbackParams);

                    break; // IDM_ASYNC_STREAM_START_LOOPBACK

                case IDM_ASYNC_STREAM_STOP_LOOPBACK:

                    w1394_AsyncStreamStopLoopback(g_hWndEdit, &streamLoopbackParams);

                    break; // IDM_ASYNC_STREAM_STOP_LOOPBACK
                    
                case IDM_ISOCH_ALLOCATE_BANDWIDTH:

                    w1394_IsochAllocateBandwidth(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_ALLOCATE_BANDWIDTH

                case IDM_ISOCH_ALLOCATE_CHANNEL:

                    w1394_IsochAllocateChannel(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_ALLOCATE_CHANNEL

                case IDM_ISOCH_ALLOCATE_RESOURCES:

                    w1394_IsochAllocateResources(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_ALLOCATE_RESOURCES

                case IDM_ISOCH_ATTACH_BUFFERS:

                    w1394_IsochAttachBuffers(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_ATTACH_BUFFERS

                case IDM_ISOCH_DETACH_BUFFERS:

                    w1394_IsochDetachBuffers(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_DETACH_BUFFERS

                case IDM_ISOCH_FREE_BANDWIDTH:

                    w1394_IsochFreeBandwidth(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_FREE_BANDWIDTH

                case IDM_ISOCH_FREE_CHANNEL:

                    w1394_IsochFreeChannel(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_FREE_CHANNEL

                case IDM_ISOCH_FREE_RESOURCES:

                    w1394_IsochFreeResources(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_FREE_RESOURCES

                case IDM_ISOCH_LISTEN:

                    w1394_IsochListen(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_LISTEN

                case IDM_ISOCH_QUERY_CURRENT_CYCLE_TIME:

                    w1394_IsochQueryCurrentCycleTime(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_QUERY_CURRENT_CYCLE_TIME

                case IDM_ISOCH_QUERY_RESOURCES:

                    w1394_IsochQueryResources(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_QUERY_RESOURCES

                case IDM_ISOCH_SET_CHANNEL_BANDWIDTH:

                    w1394_IsochSetChannelBandwidth(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_SET_CHANNEL_BANDWIDTH

                case IDM_ISOCH_STOP:

                    w1394_IsochStop(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_STOP

                case IDM_ISOCH_TALK:

                    w1394_IsochTalk(g_hWndEdit, SelectedDevice);

                    break; // IDM_ISOCH_TALK

                case IDM_ISOCH_START_LOOPBACK:

                    w1394_IsochStartLoopback(g_hWndEdit, SelectedDevice, &isochLoopbackParams);

                    break; // IDM_ISOCH_START_LOOPBACK

                case IDM_ISOCH_STOP_LOOPBACK:

                    w1394_IsochStopLoopback(g_hWndEdit, &isochLoopbackParams);

                    break; // IDM_ISOCH_STOP_LOOPBACK

                case IDM_BUS_RESET:

                    w1394_BusReset(g_hWndEdit, SelectedDevice);

                    break; // IDM_BUS_RESET

                case IDM_GET_GENERATION_COUNT:

                    w1394_GetGenerationCount(g_hWndEdit, SelectedDevice);

                    break; // IDM_GET_GENERATION_COUNT

                case IDM_GET_LOCAL_HOST_INFORMATION:

                    w1394_GetLocalHostInfo(g_hWndEdit, SelectedDevice);

                    break; // IDM_GET_LOCAL_HOST_INFORMATION


                case IDM_GET_ADDRESS_FROM_DEVICE_OBJECT:

                    w1394_Get1394AddressFromDeviceObject(g_hWndEdit, SelectedDevice);

                    break; // IDM_GET_ADDRESS_FROM_DEVICE_OBJECT

                case IDM_CONTROL:

                    w1394_Control(g_hWndEdit, SelectedDevice);

                    break; // IDM_CONTROL

                case IDM_GET_MAX_SPEED_BETWEEN_DEVICES:

                    w1394_GetMaxSpeedBetweenDevices(g_hWndEdit, SelectedDevice);

                    break; // IDM_GET_MAX_SPEED_BETWEEN_DEVICES

                case IDM_GET_CONFIGURATION_INFORMATION:

                    w1394_GetConfigurationInfo(g_hWndEdit, SelectedDevice);

                    break; // IDM_GET_CONFIGURATION_INFORMATION

                case IDM_SET_DEVICE_XMIT_PROPERTIES:

                    w1394_SetDeviceXmitProperties(g_hWndEdit, SelectedDevice);

                    break; // IDM_SET_DEVICE_XMIT_PROPERTIES

                case IDM_SEND_PHY_CONFIG_PACKET:

                    w1394_SendPhyConfigPacket(g_hWndEdit, SelectedDevice);

                    break; // IDM_SEND_PHY_CONFIG_PACKET

                case IDM_BUS_RESET_NOTIFICATION:

                    w1394_BusResetNotification(g_hWndEdit, SelectedDevice);

                    break; // IDM_BUS_RESET_NOTIFICATION

                case IDM_SET_LOCAL_HOST_PROPERTIES:

                    w1394_SetLocalHostInfo(g_hWndEdit, SelectedDevice);

                    break; // IDM_SET_LOCAL_HOST_PROPERTIES

                default:
                    break; // default
            }
            break; // WM_COMMAND

        case NOTIFY_DEVICE_CHANGE:
            TRACE(TL_TRACE, (g_hWndEdit, "NOTIFY_DEVICE_CHANGE\r\n"));

            GetDeviceList(&DeviceData);
            GetVirtualDeviceList(&VirtDeviceData);

            TRACE(TL_TRACE, (g_hWndEdit, "number Diagnostic Devices = 0x%x\r\n", DeviceData.numDevices));
            TRACE(TL_TRACE, (g_hWndEdit, "number Virtual Devices = 0x%x\r\n", VirtDeviceData.numDevices));

            for (i=0;i<DeviceData.numDevices;i++) {

                TRACE(TL_TRACE, (g_hWndEdit, "DeviceName[0x%x] = %s\r\n", i, DeviceData.deviceList[i].DeviceName));
            }

            for (i=0;i<VirtDeviceData.numDevices;i++) {

                TRACE(TL_TRACE, (g_hWndEdit, "DeviceName[0x%x] = %s\r\n", i, VirtDeviceData.deviceList[i].DeviceName));
            }

            if (!SelectedDevice && DeviceData.numDevices) {

                SelectedDevice = DeviceData.deviceList[0].DeviceName;
                TRACE(TL_TRACE, (g_hWndEdit, "Selected Device = %s\r\n", SelectedDevice));
            }
            if (!SelectedDevice && VirtDeviceData.numDevices) {

                SelectedDevice = VirtDeviceData.deviceList[0].DeviceName;
                TRACE(TL_TRACE, (g_hWndEdit, "Selected Device = %s\r\n", SelectedDevice));
            }
            
            break; // NOTIFY_DEVICE_CHANGE

        case NOTIFY_BUS_RESET:
            TRACE(TL_TRACE, (g_hWndEdit, "BUS RESET!!!\r\n"));
            break; // NOTIFY_BUS_RESET

        default:
            break;
    } // switch iMsg

    return DefWindowProc(hWnd, iMsg, wParam, lParam);
} // w1394_AppWndProc

int WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    PSTR        szCmdLine,
    int         iCmdShow
    )
/*++

Routine Description:

    Entry point for app. Registers class, creates window.

--*/
{
    CHAR        szAppClassName[] = "w1394_AppClass";
    CHAR        szTitleBar[] = "Win1394 Application";
    MSG         Msg;
    WNDCLASSEX  WndClassEx;
    HWND        hWnd;

    g_hInstance = hInstance;

    // main window app...
    WndClassEx.cbSize = sizeof(WNDCLASSEX);
    WndClassEx.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = w1394_AppWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = 0;
    WndClassEx.hInstance = g_hInstance;
    WndClassEx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    WndClassEx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    WndClassEx.hCursor = NULL;
    WndClassEx.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    WndClassEx.lpszMenuName = "AppMenu";
    WndClassEx.lpszClassName = szAppClassName;

    RegisterClassEx( &WndClassEx );

    hWnd = CreateWindowEx( WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
                           szAppClassName,
                           szTitleBar,
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           CW_USEDEFAULT,
                           NULL,
                           NULL,
                           g_hInstance,
                           NULL
                           );

    ShowWindow(hWnd, iCmdShow);

    while (GetMessage(&Msg, NULL, 0, 0)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return (int)(Msg.wParam);
} // WinMain



