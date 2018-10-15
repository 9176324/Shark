/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name: testapp.c


Abstract:


Author:

     Eliyas Yakub   Dec 15, 2002

Environment:

    User mode only.

Revision History:

    
--*/

#define UNICODE 1
#define INITGUID

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <setupapi.h>
#include <dbt.h>
#include <winioctl.h>
#include <ntddndis.h> // for IOCTL_NDIS_QUERY_GLOBAL_STATS
#include <ndisguid.h> // for GUID_NDIS_LAN_CLASS
#include <strsafe.h>
#include "testapp.h"
#include "public.h" // for IOCTL_NETVMINI_HELLO
    
//
// Global variables
//
HINSTANCE   hInst;
HWND        hWndList;
TCHAR       szTitle[]=TEXT("NETVMINI's IOCTL Test Application");
LIST_ENTRY  ListHead;
HDEVNOTIFY  hInterfaceNotification;
TCHAR       OutText[500];
UINT        ListBoxIndex = 0;
GUID        InterfaceGuid;// = GUID_NDIS_LAN_CLASS;


_inline VOID Display(PWCHAR Format, PWCHAR Str) 
{
    HRESULT hr;
    
    if(Str) {
        hr = StringCchPrintf(OutText, sizeof(OutText)/sizeof(WCHAR), Format, Str);
    } else {
        hr = StringCchCopy(OutText, sizeof(OutText)/sizeof(WCHAR), Format);
    }
    if(FAILED(hr)){
        return;
    }
    SendMessage(hWndList, LB_INSERTSTRING, ListBoxIndex++, (LPARAM)OutText);
}


int PASCAL WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdParam,
                    int       nCmdShow)
{
    static    TCHAR szAppName[]=TEXT("NETVMINI TESTAPP");
    HWND      hWnd;
    MSG       msg;
    WNDCLASS  wndclass;

    InterfaceGuid = GUID_NDIS_LAN_CLASS;
    hInst=hInstance;

    if (!hPrevInstance)
       {
         wndclass.style        =  CS_HREDRAW | CS_VREDRAW;
         wndclass.lpfnWndProc  =  WndProc;
         wndclass.cbClsExtra   =  0;
         wndclass.cbWndExtra   =  0;
         wndclass.hInstance    =  hInstance;
         wndclass.hIcon        =  LoadIcon (NULL, IDI_APPLICATION);
         wndclass.hCursor      =  LoadCursor(NULL, IDC_ARROW);
         wndclass.hbrBackground=  GetStockObject(WHITE_BRUSH);
         wndclass.lpszMenuName =  TEXT("GenericMenu");
         wndclass.lpszClassName=  szAppName;

         RegisterClass(&wndclass);
       }

    hWnd = CreateWindow (szAppName,
                         szTitle,
                         WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         NULL,
                         NULL,
                         hInstance,
                         NULL);

    ShowWindow (hWnd,nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage (&msg, NULL, 0,0))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

    return (0);
}


LRESULT FAR PASCAL 
WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD nEventType = (DWORD)wParam; 
    PDEV_BROADCAST_HDR p = (PDEV_BROADCAST_HDR) lParam;
    DEV_BROADCAST_DEVICEINTERFACE filter;
    
    switch (message)
    {

        case WM_COMMAND:
            HandleCommands(hWnd, message, wParam, lParam);
            return 0;

        case WM_CREATE:

            hWndList = CreateWindow (TEXT("listbox"),
                         NULL,
                         WS_CHILD|WS_VISIBLE|LBS_NOTIFY |
                         WS_VSCROLL | WS_BORDER,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         CW_USEDEFAULT,
                         hWnd,
                         (HMENU)ID_EDIT,
                         hInst,
                         NULL);
                         
            filter.dbcc_size = sizeof(filter);
            filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
            filter.dbcc_classguid = InterfaceGuid;
            hInterfaceNotification = RegisterDeviceNotification(hWnd, &filter, 0);

            InitializeListHead(&ListHead);
            EnumExistingDevices(hWnd);

            return 0;

      case WM_SIZE:

            MoveWindow(hWndList, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            return 0;

      case WM_SETFOCUS:
            SetFocus(hWndList);
            return 0;
            
      case WM_DEVICECHANGE:      

            //
            // All the events we're interested in come with lParam pointing to
            // a structure headed by a DEV_BROADCAST_HDR.  This is denoted by
            // bit 15 of wParam being set, and bit 14 being clear.
            //
            if((wParam & 0xC000) == 0x8000) {
            
                if (!p)
                    return 0;
                                  
                if (p->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {

                    HandleDeviceInterfaceChange(hWnd, nEventType, (PDEV_BROADCAST_DEVICEINTERFACE) p);
                } else if (p->dbch_devicetype == DBT_DEVTYP_HANDLE) {

                    HandleDeviceChange(hWnd, nEventType, (PDEV_BROADCAST_HANDLE) p);
                }
            }
            return 0;

      case WM_CLOSE:
            Cleanup(hWnd);
            UnregisterDeviceNotification(hInterfaceNotification);
            return  DefWindowProc(hWnd,message, wParam, lParam);

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hWnd,message, wParam, lParam);
  }


LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM     lParam
    )

{
    switch (wParam) {

        case IDM_OPEN:
            Cleanup(hWnd); // close all open handles
            EnumExistingDevices(hWnd);
            break;

        case IDM_CLOSE:           
            Cleanup(hWnd);
            break;
            
        case IDM_CTL_IOCTL:   
            {
                PDEVICE_INFO deviceInfo = NULL;
                PLIST_ENTRY thisEntry;
                BOOLEAN    found = FALSE;
                //
                // Find the Virtual miniport driver
                // We need the deviceInfo to get the handle to the device.
                //
                for(thisEntry = ListHead.Flink; thisEntry != &ListHead;
                    thisEntry = thisEntry->Flink)
                {
                    deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
                    if(deviceInfo && 
                        deviceInfo->hDevice != INVALID_HANDLE_VALUE &&
                        wcsstr(deviceInfo->DeviceName, TEXT("Microsoft Virtual Ethernet Adapter"))) {
                            found = TRUE;
                            SendIoctlToControlDevice(deviceInfo);
                    }
                }
                    
                if(!found) {
                    Display(TEXT("Didn't find any NETVMINI device"), NULL);                     
                }
            }                
            break;
                                    
        case IDM_CLEAR:           
            SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
            ListBoxIndex = 0;
            break;
            
        case IDM_EXIT:           
            PostQuitMessage(0);
            break;
            
        default:
            break;
    }

    return TRUE;
}


BOOL 
HandleDeviceInterfaceChange(
    HWND hWnd, 
    DWORD evtype, 
    PDEV_BROADCAST_DEVICEINTERFACE dip
    )
{
    DEV_BROADCAST_HANDLE    filter;
    PDEVICE_INFO            deviceInfo = NULL;
    HRESULT                 hr;
    
    switch (evtype)
    {
        case DBT_DEVICEARRIVAL:
        //
        // New device arrived. Open handle to the device 
        // and register notification of type DBT_DEVTYP_HANDLE
        //

        deviceInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DEVICE_INFO));
        if(!deviceInfo)
            return FALSE;
            
        InitializeListHead(&deviceInfo->ListEntry);
        InsertTailList(&ListHead, &deviceInfo->ListEntry);
        

        if(!GetDeviceDescription(dip->dbcc_name, deviceInfo->DeviceName,
                                 NULL)) {
            MessageBox(hWnd, TEXT("GetDeviceDescription failed"), TEXT("Error!"), MB_OK);  
        }

        Display(TEXT("New device Arrived (Interface Change Notification): %ws"), 
                    deviceInfo->DeviceName);

        hr = StringCchCopy(deviceInfo->DevicePath, MAX_PATH, dip->dbcc_name);
        if(FAILED(hr)){
            break; // DeviceInfo will be freed later by the cleanup routine.
        }
        
        deviceInfo->hDevice = CreateFile(dip->dbcc_name, 
                                        GENERIC_READ |GENERIC_WRITE, 0, NULL, 
                                        OPEN_EXISTING, 0, NULL);
        if(deviceInfo->hDevice == INVALID_HANDLE_VALUE) {
            Display(TEXT("Failed to open the device: %ws"), 
                    deviceInfo->DeviceName);
            break;
        }
        Display(TEXT("Opened handled to the device: %ws"), 
                    deviceInfo->DeviceName);
        memset (&filter, 0, sizeof(filter)); //zero the structure
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;
 
        deviceInfo->hHandleNotification = 
                            RegisterDeviceNotification(hWnd, &filter, 0);
        break;
 
        case DBT_DEVICEREMOVECOMPLETE:
        Display(TEXT("Remove Complete (Interface Change Notification)"), NULL);
        break;    

        //
        // Device Removed.
        // 

        default:
        Display(TEXT("Unknown (Interface Change Notification)"), NULL);
        break;
    }
    return TRUE;
}
 
BOOL 
HandleDeviceChange(
    HWND hWnd, 
    DWORD evtype, 
    PDEV_BROADCAST_HANDLE dhp
    )
{
    UINT i;
    DEV_BROADCAST_HANDLE    filter;
    PDEVICE_INFO            deviceInfo = NULL;
    PLIST_ENTRY             thisEntry;
    HANDLE                  tempHandle;
    
    //
    // Walk the list to get the deviceInfo for this device
    // by matching the handle given in the notification.
    //
    for(thisEntry = ListHead.Flink; thisEntry != &ListHead;
                        thisEntry = thisEntry->Flink)
    {
        deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
        if(dhp->dbch_handle == deviceInfo->hDevice) {
            break;
        }
        deviceInfo = NULL;
    }

    if(!deviceInfo) {
        MessageBox(hWnd, TEXT("Unknown Device"), TEXT("Error"), MB_OK);  
        return FALSE;
    }

    switch (evtype)
    {
    
    case DBT_DEVICEQUERYREMOVE:

        Display(TEXT("Query Remove (Handle Notification)"),
                        deviceInfo->DeviceName);
        //
        // Close the handle so that target device can
        // get removed. Do not unregister the notification
        // at this point, because you want to know whether
        // the device is successfully removed or not.
        //

        tempHandle = deviceInfo->hDevice;
        
        CloseDeviceHandles(deviceInfo);
        //
        // Since we use the handle to locate the deviceinfo, we
        // will reset the handle to the original value and
        // clear it in the the remove_pending message callback.
        // ugly hack..
        //
        deviceInfo->hDevice = tempHandle;            
        break;
        
    case DBT_DEVICEREMOVECOMPLETE:
 
        Display(TEXT("Remove Complete (Handle Notification):%ws"),
                    deviceInfo->DeviceName);
        //
        // Device is removed so close the handle if it's there
        // and unregister the notification
        //
 
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
         }

         CloseDeviceHandles(deviceInfo);
         
        //
        // Unlink this deviceInfo from the list and free the memory
        //
         RemoveEntryList(&deviceInfo->ListEntry);
         HeapFree (GetProcessHeap(), 0, deviceInfo);
         
        break;
        
    case DBT_DEVICEREMOVEPENDING:
 
        Display(TEXT("Remove Pending (Handle Notification):%ws"),
                                        deviceInfo->DeviceName);
        //
        // Device is removed so close the handle if it's there
        // and unregister the notification
        //
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
            deviceInfo->hDevice = INVALID_HANDLE_VALUE;
        }
        //
        // Unlink this deviceInfo from the list and free the memory
        //
         RemoveEntryList(&deviceInfo->ListEntry);
         HeapFree (GetProcessHeap(), 0, deviceInfo);

        break;

    case DBT_DEVICEQUERYREMOVEFAILED :
        Display(TEXT("Remove failed (Handle Notification):%ws"),
                                    deviceInfo->DeviceName);
        //
        // Remove failed. So reopen the device and register for
        // notification on the new handle. But first we should unregister
        // the previous notification.
        //
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
         }
        deviceInfo->hDevice = CreateFile(deviceInfo->DevicePath, 
                                GENERIC_READ | GENERIC_WRITE, 
                                0, NULL, OPEN_EXISTING, 0, NULL);
        if(deviceInfo->hDevice == INVALID_HANDLE_VALUE) {
            Display(TEXT("Failed to reopen the device: %ws"), 
                    deviceInfo->DeviceName);
            //
            // Unlink this deviceInfo from the list and free the memory
            //
            RemoveEntryList(&deviceInfo->ListEntry);
            HeapFree (GetProcessHeap(), 0, deviceInfo);
            break;
        }

        //
        // Register handle based notification to receive pnp 
        // device change notification on the handle.
        //
        memset (&filter, 0, sizeof(filter)); //zero the structure
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;
 
        deviceInfo->hHandleNotification = 
                            RegisterDeviceNotification(hWnd, &filter, 0);
        Display(TEXT("Reopened device %ws"), deviceInfo->DeviceName);        
        break;
        
    default:
        Display(TEXT("Unknown (Handle Notification)"),
                                    deviceInfo->DeviceName);
        break;
 
    }
    return TRUE;
}


BOOLEAN
EnumExistingDevices(
    HWND   hWnd
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0, bytes=0;
    DWORD                               dwRegType, error;
    DEV_BROADCAST_HANDLE                filter;
    PDEVICE_INFO                        deviceInfo =NULL;
    UINT                                i=0;
    HRESULT                             hr;
    
    hardwareDeviceInfo = SetupDiGetClassDevs (
                       (LPGUID)&InterfaceGuid,
                       NULL, // Define no enumerator (global)
                       NULL, // Define no
                       (DIGCF_PRESENT | // Only Devices present
                       DIGCF_DEVICEINTERFACE)); // Function class devices.
    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }
  
    //
    // Enumerate devices of toaster class
    //
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    for(i=0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                 0, // No care about specific PDOs
                                 (LPGUID)&InterfaceGuid,
                                 i, //
                                 &deviceInterfaceData); i++ ) {
                                 
        //
        // Allocate a function class device data structure to 
        // receive the information about this particular device.
        //

        //
        // First find out required length of the buffer
        //
        if(deviceInterfaceDetailData)
                HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);
                
        if(!SetupDiGetDeviceInterfaceDetail (
                hardwareDeviceInfo,
                &deviceInterfaceData,
                NULL, // probing so no output buffer yet
                0, // probing so output buffer length of zero
                &requiredLength,
                NULL) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Error;
        }
        predictedLength = requiredLength;

        deviceInterfaceDetailData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                                        predictedLength);
        deviceInterfaceDetailData->cbSize = 
                        sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);


        if (! SetupDiGetDeviceInterfaceDetail (
                   hardwareDeviceInfo,
                   &deviceInterfaceData,
                   deviceInterfaceDetailData,
                   predictedLength,
                   &requiredLength,
                   NULL)) {              
            goto Error;
        }

        deviceInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                        sizeof(DEVICE_INFO));
        if(!deviceInfo)
            goto Error;
            
        InitializeListHead(&deviceInfo->ListEntry);
        InsertTailList(&ListHead, &deviceInfo->ListEntry);
        
        //
        // Get the device details such as friendly name and SerialNo
        //
        if(!GetDeviceDescription(deviceInterfaceDetailData->DevicePath, 
                                 deviceInfo->DeviceName,
                                 NULL)){
            goto Error;
        }

        Display(TEXT("Found device %ws"), deviceInfo->DeviceName );

        hr = StringCchCopy(deviceInfo->DevicePath, MAX_PATH, deviceInterfaceDetailData->DevicePath);
        if(FAILED(hr)){
            goto Error;
        }
        
        //
        // Open an handle to the device.
        //
        deviceInfo->hDevice = CreateFile ( 
                deviceInterfaceDetailData->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL, // no SECURITY_ATTRIBUTES structure
                OPEN_EXISTING, // No special create flags
                0, // No special attributes
                NULL);

        if (INVALID_HANDLE_VALUE == deviceInfo->hDevice) {
            Display(TEXT("Failed to open the device: %ws"), 
                    deviceInfo->DeviceName);            
            continue;
        }
        
        Display(TEXT("Opened handled to the device: %ws"), 
                    deviceInfo->DeviceName);
        //
        // Register handle based notification to receive pnp 
        // device change notification on the handle.
        //

        memset (&filter, 0, sizeof(filter)); //zero the structure
        filter.dbch_size = sizeof(filter);
        filter.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filter.dbch_handle = deviceInfo->hDevice;

        deviceInfo->hHandleNotification = RegisterDeviceNotification(hWnd, &filter, 0);        

    } 

    if(deviceInterfaceDetailData)
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return 0;

Error:

    error = GetLastError();
    MessageBox(hWnd, TEXT("EnumExisting Devices failed"), TEXT("Error!"), MB_OK);  
    if(deviceInterfaceDetailData)
        HeapFree (GetProcessHeap(), 0, deviceInterfaceDetailData);

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    Cleanup(hWnd);
    return 0;
}

BOOLEAN Cleanup(HWND hWnd)
{
    PDEVICE_INFO    deviceInfo =NULL;
    PLIST_ENTRY     thisEntry;

    while (!IsListEmpty(&ListHead)) {
        thisEntry = RemoveHeadList(&ListHead);
        deviceInfo = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
        if (deviceInfo->hHandleNotification) {
            UnregisterDeviceNotification(deviceInfo->hHandleNotification);
            deviceInfo->hHandleNotification = NULL;
        }
        CloseDeviceHandles(deviceInfo);
        HeapFree (GetProcessHeap(), 0, deviceInfo);
    }
    return TRUE;
}

VOID CloseDeviceHandles(
    IN PDEVICE_INFO deviceInfo)
{
    if(!deviceInfo) return;
    
    if (deviceInfo->hDevice != INVALID_HANDLE_VALUE && 
            deviceInfo->hDevice != NULL) {
            
        CloseHandle(deviceInfo->hDevice);
        deviceInfo->hDevice = INVALID_HANDLE_VALUE;
        
        //
        // If there is a valid control device handle, close
        // that also. We aren't going to get any notification
        // on the control-device because it's not known to PNP
        // subsystem.
        //       
        if (deviceInfo->hControlDevice != INVALID_HANDLE_VALUE && 
                        deviceInfo->hControlDevice != NULL) {
            CloseHandle(deviceInfo->hControlDevice);
            deviceInfo->hControlDevice = INVALID_HANDLE_VALUE;                            
        }
        
        Display(TEXT("Closed handle to device %ws"), deviceInfo->DeviceName );
    }
}
BOOL 
GetDeviceDescription(
    LPTSTR DevPath, 
    LPTSTR OutBuffer,
    PULONG Unused
)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    SP_DEVINFO_DATA                     deviceInfoData;
    DWORD                               dwRegType, error;

    hardwareDeviceInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        goto Error;
    }
    
    //
    // Enumerate devices of toaster class
    //
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

    SetupDiOpenDeviceInterface (hardwareDeviceInfo, DevPath,
                                 0, //
                                 &deviceInterfaceData);
                                 
    deviceInfoData.cbSize = sizeof(deviceInfoData);
    if(!SetupDiGetDeviceInterfaceDetail (
            hardwareDeviceInfo,
            &deviceInterfaceData,
            NULL, // probing so no output buffer yet
            0, // probing so output buffer length of zero
            NULL,
            &deviceInfoData) && (error = GetLastError()) != ERROR_INSUFFICIENT_BUFFER)
    {
        goto Error;
    }
    //
    // Get the friendly name for this instance, if that fails
    // try to get the device description.
    //

    if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     MAX_PATH,
                                     NULL))
    {
        if(!SetupDiGetDeviceRegistryProperty(hardwareDeviceInfo, &deviceInfoData,
                                     SPDRP_DEVICEDESC,
                                     &dwRegType,
                                     (BYTE*) OutBuffer,
                                     MAX_PATH,
                                     NULL)){
            goto Error;
                                     
        }
        

    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return TRUE;

Error:

    error = GetLastError();
    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
    return FALSE;
}

BOOLEAN
SendIoctlToControlDevice(
    IN PDEVICE_INFO deviceInfo)
{
    BOOLEAN result = FALSE;
    UINT bytes;       

    //
    // Open handle to the control device, if it's not already opened. Please
    // note that even a non-admin user can open handle to the device with 
    // FILE_READ_ATTRIBUTES | SYNCHRONIZE DesiredAccess and send IOCTLs if the 
    // IOCTL is defined with FILE_ANY_ACCESS. So for better security avoid 
    // specifying FILE_ANY_ACCESS in your IOCTL defintions. 
    // If you open the device with GENERIC_READ, you can send IOCTL with 
    // FILE_READ_DATA access rights. If you open the device with GENERIC_WRITE, 
    // you can sedn ioctl with FILE_WRITE_DATA access rights.
    // 
    //
    
    if(deviceInfo->hControlDevice != INVALID_HANDLE_VALUE) {
            
        deviceInfo->hControlDevice = CreateFile ( 
            TEXT("\\\\.\\NETVMINI"),
            GENERIC_READ | GENERIC_WRITE,//FILE_READ_ATTRIBUTES | SYNCHRONIZE,
            FILE_SHARE_READ,
            NULL, // no SECURITY_ATTRIBUTES structure
            OPEN_EXISTING, // No special create flags
            FILE_ATTRIBUTE_NORMAL, // No special attributes
            NULL);

        if (INVALID_HANDLE_VALUE == deviceInfo->hControlDevice) {
            Display(TEXT("Failed to open the control device: %ws"), 
                    deviceInfo->DeviceName);
            return result;
        } 
    }        
    
    //
    // send ioclt requests
    //
    if(!DeviceIoControl (deviceInfo->hControlDevice,
          IOCTL_NETVMINI_READ_DATA,
          NULL, 0,
          NULL, 0,
          &bytes, NULL)) {
       Display(TEXT("Read IOCTL to %ws failed"), deviceInfo->DeviceName);                    
    } else {
       Display(TEXT("Read IOCTL to %ws succeeded"), deviceInfo->DeviceName); 
       result = TRUE;
    }
   
    if(!DeviceIoControl (deviceInfo->hControlDevice,
          IOCTL_NETVMINI_WRITE_DATA,
          NULL, 0,
          NULL, 0,
          &bytes, NULL)) {
       Display(TEXT("Write IOCTL to %ws failed"), deviceInfo->DeviceName);                    
    } else {
       Display(TEXT("Write IOCTL to %ws succeeded"), deviceInfo->DeviceName); 
       result = TRUE;
    }
    
    return result;
}

