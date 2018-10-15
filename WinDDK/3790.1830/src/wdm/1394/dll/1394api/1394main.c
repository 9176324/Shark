/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    1394main.c

Abstract


Author:

    Peter Binder (pbinder) 4/22/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/22/98  pbinder   moved from 1394api.c
--*/

#define _1394MAIN_C
#define INITGUID
#include "pch.h"
#undef _1394MAIN_C

//
// globals
//
ULONG TraceLevel = TL_FATAL;

//
//  LibMain32
//
BOOL
WINAPI
DllMain(
    HINSTANCE   hInstance,
    ULONG       ulReason,
    LPVOID      pvReserved
    )
{
    BOOL                    bReturn, bCreateProcess;
    DWORD                   dwError;

    TRACE(TL_TRACE, (NULL, "hInstance = 0x%x\r\n", hInstance));

    switch(ulReason) {

        case DLL_PROCESS_ATTACH:
            TRACE(TL_TRACE, (NULL, "DLLMain: DLL_PROCESS_ATTACH\r\n"));

            hData = CreateFileMapping( INVALID_HANDLE_VALUE,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       sizeof(SHARED_DATA),
                                       "1394APISharedDataMap"
                                       );
            dwError = GetLastError();

            if (!hData) {

                TRACE(TL_ERROR, (NULL, "CreateFileMapping failed = 0x%x\r\n", dwError));
            }
            else {

                bCreateProcess = (dwError != ERROR_ALREADY_EXISTS);
                SharedData = (PSHARED_DATA) MapViewOfFile( hData,
                                                           FILE_MAP_ALL_ACCESS,
                                                           0,
                                                           0,
                                                           0
                                                           );

                if (!SharedData) {

                    dwError = GetLastError();
                    TRACE(TL_ERROR, (NULL, "Error mapping SharedData = 0x%x\r\n", dwError));
                }
                else {

                    TRACE(TL_TRACE, (NULL, "SharedData = 0x%x\r\n", SharedData));

                    if (bCreateProcess) {

                        CHAR    szCommandLine[] = "\"RUNDLL32.exe\" 1394API.DLL,DLL_Thread";

                        TRACE(TL_TRACE, (NULL, "Creating process...\r\n"));

                        // zero out memory...
                        ZeroMemory(SharedData, sizeof(SHARED_DATA));

                        SharedData->g_hInstance = (HANDLE)0x400000; //hInstance;

                        // right now, this isn't being used. in the future, we might want
                        // to use this to keep track of internal dll trace.
                        SharedData->g_hWndEdit = NULL;

                        bReturn = CreateProcess( NULL,
                                                 szCommandLine,
                                                 NULL,
                                                 NULL,
                                                 FALSE,
                                                 DETACHED_PROCESS,
                                                 NULL,
                                                 NULL,
                                                 &SharedData->StartUpInfo,
                                                 &SharedData->ProcessInfo
                                                 );

                        if (!bReturn) {

                            dwError = GetLastError();
                            TRACE(TL_ERROR, (NULL, "CreateProcess failed = 0x%x\r\n", dwError));
                        }
                    }

                    SharedData->nAttachedProcess++;
                }
            }


            break;

        case DLL_PROCESS_DETACH:
            TRACE(TL_TRACE, (NULL, "DLLMain: DLL_PROCESS_DETACH\r\n"));

            TRACE(TL_TRACE, (NULL, "nAttachedProcess = 0x%x\n", SharedData->nAttachedProcess));

            SharedData->nAttachedProcess--;

            // check for no more attached processes. we use one here
            // because our attached process counts as one.
            if (SharedData->nAttachedProcess == 1) {

                // need to close the handles to the created process and thread
                CloseHandle(SharedData->ProcessInfo.hProcess);
                CloseHandle(SharedData->ProcessInfo.hThread);

                SendMessage(SharedData->g_hWnd, WM_DESTROY, 0, 0);
            }

            UnmapViewOfFile( SharedData );
            CloseHandle( hData );
            break;

        case DLL_THREAD_ATTACH:
            TRACE(TL_TRACE, (NULL, "DLLMain: DLL_THREAD_ATTACH\r\n"));
            break;

        case DLL_THREAD_DETACH:
            TRACE(TL_TRACE, (NULL, "DLLMain: DLL_THREAD_DETACH\r\n"));
            break;

        default:
            TRACE(TL_TRACE, (NULL, "DLLMain: UNKNOWN ulReason recieved = 0x%x\r\n", ulReason));
            break;
    }

    return(TRUE);
}

LRESULT
CALLBACK
DLL_WndProc(
    HWND        hWnd,
    UINT        iMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
/*++

Routine Description:

    This is the application main WndProc. Here we will handle
    all messages sent to the application.

--*/
{
    switch(iMsg) {

        case WM_CREATE:
            TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: WM_CREATE\r\n"));

            // create an edit control for the main app.
            SharedData->g_hWndEdit = CreateWindow( "edit",
                                                   NULL,
                                                   WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                                   WS_BORDER | ES_LEFT | ES_MULTILINE |
                                                   ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL |
                                                   ES_READONLY,
                                                   0,
                                                   0,
                                                   0,
                                                   0,
                                                   hWnd,
                                                   0,
                                                   SharedData->g_hInstance,
                                                   NULL
                                                   );

            // get the windows version
            InternalGetWinVersion(SharedData->g_hWndEdit);

            // get the device list
            InternalGetDeviceList(SharedData->g_hWndEdit);

            break; // WM_CREATE

        case WM_SIZE:
            MoveWindow(SharedData->g_hWndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            break; // WM_SIZE

        case WM_DESTROY:
            TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: WM_DESTROY\r\n"));
            //StopBusResetThread(hWnd);
            PostQuitMessage(0);
            break; // WM_DESTROY

        case WM_DEVICECHANGE:
            TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: WM_DEVICECHANGE\r\n"));

            switch ((UINT)wParam) {
/*
                case DBT_DEVICEARRIVAL: {

                    PDEV_BROADCAST_HDR  devHdr;

                    devHdr = (PDEV_BROADCAST_HDR)lParam;

                    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: DBT_DEVICEARRIVAL\r\n"));
                    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: devHdr.dbch_devicetype = %d\r\n", devHdr->dbch_devicetype));

                    if (devHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

                        InternalGetDeviceList(SharedData->g_hWndEdit);
                        NotifyClients(SharedData->g_hWndEdit, NOTIFY_DEVICE_CHANGE);
                    }
                }
                break; // DBT_DEVICEARRIVAL

                case DBT_DEVICEREMOVECOMPLETE:
                    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: DBT_DEVICEREMOVECOMPLETE\r\n"));

                    InternalGetDeviceList(SharedData->g_hWndEdit);
                    NotifyClients(SharedData->g_hWndEdit, NOTIFY_DEVICE_CHANGE);
                    break; // DBT_DEVICEREMOVECOMPLETE
*/
                case DBT_DEVNODES_CHANGED:
                    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: DBT_DEVNODES_CHANGED\r\n"));

                    InternalGetDeviceList(SharedData->g_hWndEdit);
                    NotifyClients(SharedData->g_hWndEdit, NOTIFY_DEVICE_CHANGE);
                    break; // DBT_DEVNODES_CHANGED

            case VER_PLATFORM_WIN32_WINDOWS:
           
            case VER_PLATFORM_WIN32_NT:

                default:
                    break; // default
            }

            break; // WM_DEVICECHANGE

        case WM_REGISTER_CLIENT:
            TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: WM_REGISTER_CLIENT\r\n"));

            InternalRegisterClient((HWND)lParam);

            // update the device list
//            InternalGetDeviceList(SharedData->g_hWndEdit);

            // send them a notify message to grab the updated device list...
            SendMessage((HWND)lParam, NOTIFY_DEVICE_CHANGE, 0, 0);
            break; // WM_REGISTER_CLIENT

        case WM_DEREGISTER_CLIENT:
            TRACE(TL_TRACE, (SharedData->g_hWndEdit, "DLL_WndProc: WM_DEREGISTER_CLIENT\r\n"));

            InternalDeRegisterClient((HWND)lParam);

            break; // WM_DEREGISTER_CLIENT

        default:
            break;
    } // switch iMsg

    return DefWindowProc(hWnd, iMsg, wParam, lParam);
} // DLL_WndProc

DWORD
WINAPI
DLL_Thread(
    LPVOID      lpParameter
    )
{
    CHAR                            szAppClassName[] = "DLL_1394Class";
    CHAR                            szTitleBar[] = "1394API DLL";
    MSG                             Msg;
    WNDCLASSEX                      WndClassEx;
    DEV_BROADCAST_DEVICEINTERFACE   devBroadcastInterface;
    HANDLE                          hNotify;
    GUID                            t1394DiagGUID = GUID_1394DIAG;
    ATOM                            atom;

    TRACE(TL_TRACE, (NULL, "Enter DLLThread\r\n"));
    TRACE(TL_TRACE, (NULL, "SharedData->g_hInstance = 0x%x\r\n", SharedData->g_hInstance));

	// initialize our global data
	BusResetThreadParams = NULL;
	
    // main window app...
    WndClassEx.cbSize = sizeof(WNDCLASSEX);
    WndClassEx.style = CS_DBLCLKS | CS_BYTEALIGNWINDOW | CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = DLL_WndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = 0;
    WndClassEx.hInstance = SharedData->g_hInstance;
    WndClassEx.hIcon = NULL;
    WndClassEx.hIconSm = NULL;
    WndClassEx.hCursor = NULL;
    WndClassEx.hbrBackground = NULL;
    WndClassEx.lpszMenuName = "AppMenu";
    WndClassEx.lpszClassName = szAppClassName;

    atom = RegisterClassEx( &WndClassEx );

    if (atom == 0) {

        TRACE(TL_ERROR, (NULL, "RegisterClassEx Failed = %d\r\n", GetLastError()));
    }
    else {

        SharedData->g_hWnd = CreateWindowEx( WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES,
                                             szAppClassName,
                                             szTitleBar,
                                             WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                             0, //CW_USEDEFAULT,
                                             0, //CW_USEDEFAULT,
                                             600, //CW_USEDEFAULT,
                                             200,
                                             NULL,
                                             NULL,
                                             SharedData->g_hInstance,
                                             NULL
                                             );

        if (SharedData->g_hWnd == NULL) {

            TRACE(TL_ERROR, (NULL, "Failed to create windows = 0x%x\r\n", GetLastError()));
        }
        else {

            devBroadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
            devBroadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            devBroadcastInterface.dbcc_name[0] = 0;
            devBroadcastInterface.dbcc_classguid = t1394DiagGUID;

            hNotify = RegisterDeviceNotification( SharedData->g_hWnd,
                                                  &devBroadcastInterface,
                                                  DEVICE_NOTIFY_WINDOW_HANDLE
                                                  );

            if (!hNotify){

                TRACE(TL_ERROR, (SharedData->g_hWndEdit, "RegisterDeviceNotification Failed = 0x%x\r\n", GetLastError()));
            }

            while (GetMessage(&Msg, NULL, 0, 0)) {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);
            }

            UnregisterDeviceNotification(hNotify);
        }
    }

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Exit DLLThread\r\n"));
    ExitProcess(0);
    return(0);
} // DLLThread

ULONG
InternalGetWinVersion(
    HWND    hWnd
    )
{
    OSVERSIONINFO   vi;
    ULONG           ulExitCode = 0;

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "CheckVersion\r\n"));

    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (GetVersionEx(&vi) == FALSE) {

        ulExitCode = GetLastError();
    }
    else {
    
        TRACE(TL_TRACE, (SharedData->g_hWndEdit, "dwPlatformId = %d\r\n", vi.dwPlatformId));

        SharedData->PlatformId = vi.dwPlatformId;
    }

    TRACE(TL_TRACE, (NULL, "CheckVersion = 0x%x\r\n", ulExitCode));
    return (ulExitCode);
}

DWORD
InternalFillDeviceList(
    HWND            hWnd,
    LPGUID          guid,
    PDEVICE_DATA    pDeviceData
    )
{
    HDEVINFO                            hDevInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    DeviceInterfaceDetailData;
    ULONG                               i = 0;
    ULONG                               requiredSize;
    ULONG                               dwError = 0;
    BOOL                                bReturn;

    //
    // get a handle to 1394 test class using guid.
    //
    hDevInfo = SetupDiGetClassDevs( guid,
                                    NULL,
                                    NULL,
                                    (DIGCF_PRESENT | DIGCF_INTERFACEDEVICE)
                                    );

    if (!hDevInfo) {

        dwError = GetLastError();
        TRACE(TL_ERROR, (hWnd, "SetupDiGetClassDevs failed = 0x%x\r\n", dwError));
    }
    else {
        // here we're going to loop and find all of the test
        // 1394 interfaces available.
        while (TRUE) {

            deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

            if (SetupDiEnumDeviceInterfaces(hDevInfo, 0, guid, i, &deviceInterfaceData)) {

                // figure out the size...
                bReturn = SetupDiGetDeviceInterfaceDetail( hDevInfo,
                                                           &deviceInterfaceData,
                                                           NULL,
                                                           0,
                                                           &requiredSize,
                                                           NULL
                                                           );

                if (!bReturn) {
                    dwError = GetLastError();
                    TRACE(TL_ERROR, (hWnd, "SetupDiGetDeviceInterfaceDetail failed (size) = 0x%x\r\n", dwError));
                }

                DeviceInterfaceDetailData = LocalAlloc(LPTR, requiredSize);
                DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                TRACE(TL_TRACE, (hWnd, "DeviceInterfaceDetailData = 0x%x\r\n", DeviceInterfaceDetailData));

                bReturn = SetupDiGetDeviceInterfaceDetail( hDevInfo,
                                                           &deviceInterfaceData,
                                                           DeviceInterfaceDetailData,
                                                           requiredSize,
                                                           NULL,
                                                           NULL
                                                           );

                if (!bReturn) {
                    dwError = GetLastError();

                    if (dwError != ERROR_NO_MORE_ITEMS) {
                        TRACE(TL_ERROR, (hWnd, "SetupDiGetDeviceInterfaceDetail failed (actual) = 0x%x\r\n", dwError));
                    }

                    LocalFree(DeviceInterfaceDetailData);
                    break;
                }
                else {

                    // we have a device, lets save it.
                    pDeviceData->numDevices++;
                    lstrcpy(pDeviceData->deviceList[i].DeviceName, DeviceInterfaceDetailData->DevicePath);
                    LocalFree(DeviceInterfaceDetailData);
                }
            }
            else {

                dwError = GetLastError();
                TRACE(TL_ERROR, (hWnd, "SetupDiEnumDeviceInterfaces failed = 0x%x\r\n", dwError));

                break;
            }

            i++;
        } // while

        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return dwError;
}
    

void
InternalGetDeviceList(
    HWND    hWnd
    )
{
    PDEVICE_DATA                        DiagDeviceData;
    PDEVICE_DATA                        VDevDeviceData;
    GUID                                t1394DiagGuid = GUID_1394DIAG;
    GUID                                t1394VDevGuid = GUID_1394VDEV;

    TRACE(TL_TRACE, (hWnd, "Enter InternalGetDeviceList\r\n"));

    //StopBusResetThread(hWnd);

    // zero out number of devices and discover all the ones attached
    DiagDeviceData = &SharedData->DiagDeviceData;
    DiagDeviceData->numDevices = 0;

    VDevDeviceData = &SharedData->VDevDeviceData;
    VDevDeviceData->numDevices = 0;

    // get 1394Diag and 1394Vdev lists
    InternalFillDeviceList (hWnd, &t1394DiagGuid, DiagDeviceData);
    InternalFillDeviceList (hWnd, &t1394VDevGuid, VDevDeviceData);

    TRACE(TL_TRACE, (hWnd, "DiagDeviceData->numDevices = 0x%x\r\n", DiagDeviceData->numDevices));
    TRACE(TL_TRACE, (hWnd, "VDevDeviceData->numDevices = 0x%x\r\n", VDevDeviceData->numDevices));

    // if we have at least one device, try to start our bus
    // reset thread. this will return if one's already been
    // created.
    if (DiagDeviceData->numDevices || VDevDeviceData->numDevices) {

        //StartBusResetThread(hWnd);
    }

    TRACE(TL_TRACE, (hWnd, "Exit InternalGetDeviceList\r\n"));
} // InternalGetDeviceList
    

void
InternalRegisterClient(
    HWND    hWnd
    )
{
    PCLIENT_NODE    ClientNode;

    TRACE(TL_TRACE, (SharedData->g_hWnd, "Enter InternalRegisterClient\r\n"));
    TRACE(TL_TRACE, (SharedData->g_hWnd, "hWnd = 0x%x\r\n", hWnd));

    if (hWnd) {

        ClientNode = LocalAlloc(LPTR, sizeof(CLIENT_NODE));
        if (!ClientNode) {

            TRACE(TL_ERROR, (hWnd, "Could not allocate ClientNode!\r\n"));
            goto Exit_InternalRegisterClient;
        }

        ClientNode->hWnd = hWnd;
        InsertTailList(&ClientList, (PLIST_NODE)ClientNode);
    }

Exit_InternalRegisterClient:

    TRACE(TL_TRACE, (SharedData->g_hWnd, "Exit InternalRegisterClient\r\n"));
} // InternalRegisterClient

void
InternalDeRegisterClient(
    HWND    hWnd
    )
{
    PCLIENT_NODE    ClientNode;

    TRACE(TL_TRACE, (SharedData->g_hWnd, "Enter InternalDeRegisterClient\r\n"));

    TRACE(TL_TRACE, (SharedData->g_hWnd, "Removing hWnd = 0x%x\r\n", hWnd));

    if (hWnd) {

        ClientNode = (PCLIENT_NODE)ClientList.Next;

        while (ClientNode) {

            TRACE(TL_TRACE, (SharedData->g_hWnd, "Current hWnd = 0x%x\r\n", hWnd));

            if (ClientNode->hWnd == hWnd) {

                RemoveEntryList(&ClientList, (PLIST_NODE)ClientNode);
                break;
            }

            ClientNode = ClientNode->Next;
        }
    }

    TRACE(TL_TRACE, (SharedData->g_hWnd, "Exit InternalDeRegisterClient\r\n"));
} // InternalDeRegisterClient

void
NotifyClients(
    HWND    hWnd,
    DWORD   dwNotify
    )
{
    PCLIENT_NODE    ClientNode;

    TRACE(TL_TRACE, (hWnd, "Enter NotifyClients\r\n"));

    switch (dwNotify) {

        case NOTIFY_DEVICE_CHANGE:
            TRACE(TL_TRACE, (hWnd, "NOTIFY_DEVICE_CHANGE\r\n"));

            ClientNode = (PCLIENT_NODE)ClientList.Next;

            while (ClientNode) {

                SendMessage(ClientNode->hWnd, NOTIFY_DEVICE_CHANGE, 0, 0);

                ClientNode = ClientNode->Next;
            }
            break; // NOTIFY_DEVICE_CHANGE

        case NOTIFY_BUS_RESET:
            TRACE(TL_TRACE, (hWnd, "NOTIFY_BUS_RESET\r\n"));

            ClientNode = (PCLIENT_NODE)ClientList.Next;

            while (ClientNode) {

                SendMessage(ClientNode->hWnd, NOTIFY_BUS_RESET, 0, 0);

                ClientNode = ClientNode->Next;
            }
            break; // NOTIFY_BUS_RESET

        default:
            break;

    }

    TRACE(TL_TRACE, (hWnd, "Exit NotifyClients\r\n"));
} // NotifyClients

void
StartBusResetThread(
    HWND    hWnd
    )
{
    //
    // StartBusResetThread is only called if we find a 1394 test device
    //

    TRACE(TL_TRACE, (hWnd, "Enter StartBusResetThread\r\n"));

    // make sure we don't already have a thread...
    if (BusResetThreadParams) {
        TRACE(TL_TRACE, (hWnd, "bus reset thread already exists\r\n"));
        return;
    }

    // allocate our bus reset thread params...
    BusResetThreadParams = LocalAlloc(LPTR, sizeof(BUS_RESET_THREAD_PARAMS));
    if (!BusResetThreadParams) {

        TRACE(TL_ERROR, (hWnd, "Could not allocate BusResetThreadParams!\r\n"));
        goto Exit_StartBusResetThread;
    }

    ZeroMemory(BusResetThreadParams, sizeof(BUS_RESET_THREAD_PARAMS));

    TRACE(TL_TRACE, (hWnd, "BusResetThreadParams = 0x%x\r\n", BusResetThreadParams));
    TRACE(TL_TRACE, (hWnd, "Creating BusResetThread...\r\n"));

    // create a bus reset thread...
    // If we have a diag device use that, otherwise use vdev device
    if (SharedData->DiagDeviceData.numDevices) {
        BusResetThreadParams->szDeviceName = SharedData->DiagDeviceData.deviceList[0].DeviceName;
    }
    else if (SharedData->VDevDeviceData.numDevices) {
        BusResetThreadParams->szDeviceName = SharedData->VDevDeviceData.deviceList[0].DeviceName;
    }
    else {
    	// no devices are present which can give us bus reset info, so free up memory and return
    	LocalFree(BusResetThreadParams);
        return;
    }
    
    BusResetThreadParams->bKill = FALSE;
    BusResetThreadParams->hThread = CreateThread( NULL,
                                                  0,
                                                  BusResetThread,
                                                  (LPVOID)BusResetThreadParams,
                                                  0,
                                                  &BusResetThreadParams->ThreadId
                                                  );
Exit_StartBusResetThread:

    TRACE(TL_TRACE, (hWnd, "Exit StartBusResetThread\r\n"));
} // StartBusResetThread

void
StopBusResetThread(
    HWND    hWnd
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter StopBusResetThread\r\n"));

    // see if the thread's running
    if (!BusResetThreadParams) {
        TRACE(TL_TRACE, (hWnd, "no thread to stop...\r\n"));
        return;
    }

	

    // kill the bus reset thread
    BusResetThreadParams->bKill = TRUE;
    if (BusResetThreadParams->hEvent)
    {
    	SetEvent(BusResetThreadParams->hEvent);
    }
    
    // wait for the thread to complete
    if (BusResetThreadParams->hThread)
    {    	
    	WaitForSingleObject(BusResetThreadParams->hThread, INFINITE);
		CloseHandle(BusResetThreadParams->hThread);	
    }
    
    LocalFree(BusResetThreadParams);
    BusResetThreadParams = NULL;

    TRACE(TL_TRACE, (hWnd, "Exit StopBusResetThread\r\n"));
} // StopBusResetThread

DWORD
WINAPI
BusResetThread(
    LPVOID  lpParameter
    )
{
    PBUS_RESET_THREAD_PARAMS    pThreadParams;
    HANDLE                      hDevice;
    DWORD                       dwRet, dwBytesRet;
    OVERLAPPED                  overLapped;

    pThreadParams = (PBUS_RESET_THREAD_PARAMS)lpParameter;

    TRACE(TL_TRACE, (NULL, "Enter BusResetThread\r\n"));

    // try to open the device
    hDevice = OpenDevice(NULL, pThreadParams->szDeviceName, TRUE);

    // device opened, so let's do loopback
    if (hDevice != INVALID_HANDLE_VALUE) {

        overLapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        pThreadParams->hEvent = overLapped.hEvent;

        while (TRUE) {

            DeviceIoControl( hDevice,
                             IOCTL_BUS_RESET_NOTIFY,
                             NULL,
                             0,
                             NULL,
                             0,
                             &dwBytesRet,
                             &overLapped
                             );

            dwRet = GetLastError();

            TRACE(TL_TRACE, (NULL, "BusThreadProc: DeviceIoControl: dwRet = %d\r\n", dwRet));

            // we should always return pending, if not, something's wrong...
            if (dwRet == ERROR_IO_PENDING) {

                WaitForSingleObject(overLapped.hEvent, INFINITE);
            	ResetEvent(overLapped.hEvent);

                if (!pThreadParams->bKill) {

                    if (dwRet) {

                        // bus reset!
                        TRACE(TL_TRACE, (NULL, "BusThreadProc: BUS RESET!!!\r\n"));
                        NotifyClients(NULL, NOTIFY_BUS_RESET);
                    }
                    else {

						// we got an error, just break here and we'll exit
                        dwRet = GetLastError();
                        TRACE(TL_ERROR, (NULL, "BusThreadProc: dwRet = 0x%x\r\n"));
                        break;
                    }
                }
                else {

                    TRACE(TL_TRACE, (NULL, "BusThreadProc: Cancelling thread...\r\n"));
                    CancelIo(hDevice);
                    break;
                }
            }
            else {
                TRACE(TL_WARNING, (NULL, "BusThreadProc: IOCTL didn't return PENDING\r\n"));
                break;
            }
        } // while

        // free up all resources
        CloseHandle(hDevice);
        CloseHandle(overLapped.hEvent);
    }

    TRACE(TL_TRACE, (NULL, "Exit BusResetThread\r\n"));

    // that's it, shut this thread down
    ExitThread(0);
} // BusResetThread

//
// misc exported functions
//
void
WINAPI
DiagnosticMode(
    HWND    hWnd,
    PSTR    szBusName,
    BOOL    bMode,
    BOOL    bAll
    )
{
    HANDLE          hDevice;
    DWORD           controlCode;
    DWORD           dwRet, dwBytesRet;
    
    UCHAR           BusName[32] = "\\\\.\\1394BUS ";
    ULONG           i = 0;

    TRACE(TL_TRACE, (hWnd, "Enter DiagnosticMode\r\n"));

    controlCode = (bMode) ? IOCTL_1394_TOGGLE_ENUM_TEST_ON : IOCTL_1394_TOGGLE_ENUM_TEST_OFF;

    if (bAll) {

        for (i = 0; i < 10; i++) {

            _itoa(i, &BusName[11], 10);

            hDevice = OpenDevice(NULL, BusName, FALSE);

            if (hDevice != INVALID_HANDLE_VALUE) {
                
                dwRet = DeviceIoControl( hDevice,
                                         controlCode,
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

                    if (bMode) {
                        TRACE(TL_TRACE, (hWnd, "Diagnostic Mode enabled on %s\r\n", BusName));
                    }
                    else {
                        TRACE(TL_TRACE, (hWnd, "Diagnostic Mode disabled on %s\r\n", BusName));
                    }
                }

                CloseHandle(hDevice);
            }
            else {
            
                goto Exit_DiagnosticMode;
            }
        }
    }
    else {

        hDevice = OpenDevice(hWnd, szBusName, FALSE);

        if (hDevice != INVALID_HANDLE_VALUE) {

            // toggle diagnostic mode
            dwRet = DeviceIoControl( hDevice,
                                     controlCode,
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

                if (bMode) {
                    TRACE(TL_TRACE, (hWnd, "Diagnostic Mode enabled on %s\r\n", szBusName));
                }
                else {
                    TRACE(TL_TRACE, (hWnd, "Diagnostic Mode disabled on %s\r\n", szBusName));
                }
            }

            CloseHandle(hDevice);
        }
    }
Exit_DiagnosticMode:
    
    TRACE(TL_TRACE, (hWnd, "Exit DiagnosticMode\r\n"));
} // DiagnosticMode

DWORD
WINAPI
RegisterClient(
    HWND    hWnd
    )
{
    BOOLEAN         bReturn = TRUE;

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Enter RegisterClient\r\n"));

    //
    // we may be here before our main thread starts. block until its there
    //
//    WaitForSingleObject(SharedData->Started, 100000);
    while (!SharedData->g_hWnd) {
        Sleep(1000);
    }

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "SharedData->g_hWndEdit = 0x%x\r\n", SharedData->g_hWndEdit));

    if (hWnd) {

        SendMessage(SharedData->g_hWnd, WM_REGISTER_CLIENT, 0, (LPARAM)hWnd);
    }
    else {

        bReturn = FALSE;
    }

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Exit RegisterClient\r\n"));
    return(bReturn);
} // RegisterClient


DWORD
WINAPI
DeRegisterClient(
    HWND    hWnd
    )
{
    BOOLEAN         bReturn = TRUE;

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Enter DeRegisterClient\r\n"));

    SendMessage(SharedData->g_hWnd, WM_DEREGISTER_CLIENT, 0, (LPARAM)hWnd);

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Exit DeRegisterClient\r\n"));
    return(bReturn);
} // DeRegisterClient


DWORD
WINAPI
GetVirtualDeviceList(
    PDEVICE_DATA    DeviceData
    )
{
    BOOLEAN         bReturn = TRUE;

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Enter GetVirtualDeviceList\r\n"));

    //
    // we may be here before our main thread starts. block until its there
    //
    while (!SharedData->g_hWndEdit) {
        Sleep(1000);
    }

    CopyMemory( DeviceData, &SharedData->VDevDeviceData, sizeof(DEVICE_DATA));

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Exit GetVirtualDeviceList\r\n"));
    return(bReturn);
} // GetVirtualDeviceList

DWORD
WINAPI
GetDeviceList(
    PDEVICE_DATA    DeviceData
    )
{
    BOOLEAN         bReturn = TRUE;

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Enter GetDeviceList\r\n"));

    //
    // we may be here before our main thread starts. block until its there
    //
    while (!SharedData->g_hWndEdit) {
        Sleep(1000);
    }

    CopyMemory( DeviceData, &SharedData->DiagDeviceData, sizeof(DEVICE_DATA));

    TRACE(TL_TRACE, (SharedData->g_hWndEdit, "Exit GetDeviceList\r\n"));
    return(bReturn);
} // GetDeviceList

ULONG
WINAPI
GetDiagVersion(
    HWND            hWnd,
    PSTR            szDeviceName,
    PVERSION_DATA   Version,
    BOOL            bMatch
    )
{
    HANDLE      hDevice;
    DWORD       dwRet, dwBytesRet;

    TRACE(TL_TRACE, (hWnd, "Enter GetDiagVersion\r\n"));

    bMatch = FALSE;

    hDevice = OpenDevice(hWnd, szDeviceName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_GET_DIAG_VERSION,
                                 Version,
                                 sizeof(VERSION_DATA),
                                 Version,
                                 sizeof(VERSION_DATA),
                                 &dwBytesRet,
                                 NULL
                                 );

        if (dwRet) {

            dwRet = ERROR_SUCCESS;

            if ((Version->ulVersion == DIAGNOSTIC_VERSION) &&
                (Version->ulSubVersion == DIAGNOSTIC_SUB_VERSION)) {

                bMatch = TRUE;
            }

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

    TRACE(TL_TRACE, (hWnd, "Exit GetDiagVersion\r\n"));
    return(dwRet);
} // GetDiagVersion


DWORD
WINAPI
RemoveVirtualDriver (
    HWND            hWnd,
    PVIRT_DEVICE    pVirtualDevice,
    ULONG           BusNumber
    )
{
    HANDLE                      hDevice;
    ULONG                       ulStrLen;
    UCHAR                       BusName[16] = "\\\\.\\1394BUS";
    PIEEE1394_API_REQUEST       p1394ApiReq;
    PIEEE1394_VDEV_PNP_REQUEST  pDevPnpReq;
    DWORD                       dwBytesRet;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (NULL, "Enter RemoveVirtualDriver\r\n"));
    
    if (!pVirtualDevice->DeviceID)
    {
        dwRet = ERROR_INVALID_PARAMETER;
        goto Exit_RemoveVirtualDriver;
    }
    
    _itoa(BusNumber, &BusName[11], 10);
    hDevice = OpenDevice(NULL, BusName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {
        
        ulStrLen = strlen(pVirtualDevice->DeviceID);
        p1394ApiReq = LocalAlloc(LPTR, sizeof(IEEE1394_API_REQUEST)+ulStrLen);

        p1394ApiReq->RequestNumber = IEEE1394_API_REMOVE_VIRTUAL_DEVICE;
        p1394ApiReq->Flags = 0;

        pDevPnpReq = &p1394ApiReq->u.RemoveVirtualDevice;

        pDevPnpReq->fulFlags = pVirtualDevice->fulFlags;
        pDevPnpReq->Reserved = 0;
        pDevPnpReq->InstanceId = pVirtualDevice->InstanceID;
        strncpy(&pDevPnpReq->DeviceId, pVirtualDevice->DeviceID, ulStrLen);

        TRACE(TL_TRACE, (NULL, "p1394ApiReq = 0x%x\r\n", p1394ApiReq));
        TRACE(TL_TRACE, (NULL, "pDevPnpReq = 0x%x\r\n", pDevPnpReq));

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_IEEE1394_API_REQUEST,
                                 p1394ApiReq,
                                 sizeof(IEEE1394_API_REQUEST)+ulStrLen,
                                 NULL,
                                 0,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (NULL, "DeviceIoControl Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free resources

        CloseHandle (hDevice);
        
        if (p1394ApiReq)
            LocalFree(p1394ApiReq);
    }
    else {
        dwRet = ERROR_INVALID_HANDLE;
    }
    
Exit_RemoveVirtualDriver:

    TRACE(TL_TRACE, (NULL, "Exit RemoveVirtualDriver\r\n"));
    return dwRet;
}

DWORD
WINAPI
AddVirtualDriver (
    HWND            hWnd,
    PVIRT_DEVICE    pVirtualDevice,
    ULONG           BusNumber
    )
{
    HANDLE                      hDevice;
    ULONG                       ulStrLen;
    UCHAR                       BusName[16] = "\\\\.\\1394BUS";
    PIEEE1394_API_REQUEST       p1394ApiReq;
    PIEEE1394_VDEV_PNP_REQUEST  pDevPnpReq;
    DWORD                       dwBytesRet;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (NULL, "Enter AddVirtualDriverr\n"));

    _itoa(BusNumber, &BusName[11], 10);
    hDevice = OpenDevice(NULL, BusName, FALSE);

    if (hDevice != INVALID_HANDLE_VALUE) {
    
        ulStrLen = strlen(pVirtualDevice->DeviceID);
        
        p1394ApiReq = LocalAlloc(LPTR, sizeof(IEEE1394_API_REQUEST)+ulStrLen);

        p1394ApiReq->RequestNumber = IEEE1394_API_ADD_VIRTUAL_DEVICE;
        p1394ApiReq->Flags = pVirtualDevice->fulFlags;

        pDevPnpReq = &p1394ApiReq->u.AddVirtualDevice;

        pDevPnpReq->fulFlags = pVirtualDevice->fulFlags;
        pDevPnpReq->Reserved = 0;
        pDevPnpReq->InstanceId = pVirtualDevice->InstanceID;
        strncpy(&pDevPnpReq->DeviceId, pVirtualDevice->DeviceID, ulStrLen);

        TRACE(TL_TRACE, (NULL, "pApiReq = 0x%x\r\n", p1394ApiReq));
        TRACE(TL_TRACE, (NULL, "pDevPnpReq = 0x%x\r\n", pDevPnpReq));

        dwRet = DeviceIoControl( hDevice,
                                 IOCTL_IEEE1394_API_REQUEST,
                                 p1394ApiReq,
                                 sizeof(IEEE1394_API_REQUEST)+ulStrLen,
                                 NULL,
                                 0,
                                 &dwBytesRet,
                                 NULL
                                 );

        if (!dwRet) {

            dwRet = GetLastError();
            TRACE(TL_ERROR, (NULL, "DeviceIoControl Error = 0x%x\r\n", dwRet));
        }
        else {

            dwRet = ERROR_SUCCESS;
        }

        // free resources

        CloseHandle (hDevice);
        if (p1394ApiReq)
            LocalFree(p1394ApiReq);
    }
    else {
        dwRet = ERROR_INVALID_HANDLE;
    }   
    
    TRACE(TL_TRACE, (NULL, "Exit Add VirtualDriver\r\n"));
    return dwRet;
}


DWORD
WINAPI
SetDebugSpew(
    HWND    hWnd,
    ULONG   SpewLevel
    )
{
    // set the TraceLevel
#if defined (DBG)
    TRACE(TL_TRACE, (hWnd, "Debug Spew Set to %i\r\n", SpewLevel));
    TraceLevel = SpewLevel;

    return TraceLevel;
#endif

    return -1;
}









