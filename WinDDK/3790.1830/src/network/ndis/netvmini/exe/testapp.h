/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    TESTAPP.h

Abstract:


Author:

     Eliyas Yakub   
     
Environment:


Revision History:


--*/

#ifndef __TESTAPP_H
#define __TESTAPP_H



//
// Copied Macros from ntddk.h
//

#define CONTAINING_RECORD(address, type, field) ((type *)( \
                          (PCHAR)(address) - \
                          (ULONG_PTR)(&((type *)0)->field)))


#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}
    
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

typedef struct _DEVICE_INFO
{
   HANDLE       hDevice; // file handle
   HANDLE       hControlDevice; // file handle
   HDEVNOTIFY   hHandleNotification; // notification handle
   TCHAR        DeviceName[MAX_PATH];// friendly name of device description
   TCHAR        DevicePath[MAX_PATH];// 
   LIST_ENTRY   ListEntry;
} DEVICE_INFO, *PDEVICE_INFO;


#define ID_EDIT 1
    
#define  IDM_OPEN       100
#define  IDM_CLOSE      101
#define  IDM_EXIT       102
#define  IDM_CTL_IOCTL  103
#define  IDM_CLEAR     105

LRESULT FAR PASCAL 
WndProc (
    HWND hwnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    ); 

BOOLEAN EnumExistingDevices(
    HWND   hWnd
    );

BOOL HandleDeviceInterfaceChange(
    HWND hwnd, 
    DWORD evtype, 
    PDEV_BROADCAST_DEVICEINTERFACE dip
    );
    
BOOL HandleDeviceChange(
    HWND hwnd, 
    DWORD evtype, 
    PDEV_BROADCAST_HANDLE dhp
    );

LRESULT
HandleCommands(
    HWND     hWnd,
    UINT     uMsg,
    WPARAM   wParam,
    LPARAM   lParam
    );

BOOLEAN Cleanup(
    HWND hWnd
    );

BOOL 
GetDeviceDescription(
    LPTSTR InputName, 
    LPTSTR OutBuffer,
    PULONG SerialNo
    );
    
VOID CloseDeviceHandles(
    IN PDEVICE_INFO deviceInfo);

BOOLEAN
SendIoctlToControlDevice(
    IN PDEVICE_INFO deviceInfo);


#endif

