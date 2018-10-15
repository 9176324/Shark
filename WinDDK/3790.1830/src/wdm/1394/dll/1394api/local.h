/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    local.h

Abstract


Author:

    Peter Binder (pbinder) 4/10/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/10/98  pbinder   birth
--*/

#define STRING_SIZE             80

#define MAX_CLIENTS             10

//
// internal windows messages
//
#define WM_REGISTER_CLIENT      (WM_USER+500)
#define WM_DEREGISTER_CLIENT    (WM_USER+501)
//#define WM_NOTIFY_CLIENTS       (WM_USER+502)

//
// List of allocated bandwidth handles
//
typedef struct _ISOCH_BANDWIDTH_DATA {
    struct _ISOCH_BANDWIDTH_DATA    *Next;
    HANDLE                          hBandwidth;
} ISOCH_BANDWIDTH_DATA, *PISOCH_BANDWIDTH_DATA;

//
// List of allocated channels
//
typedef struct _ISOCH_CHANNEL_DATA {
    struct _ISOCH_CHANNEL_DATA      *Next;
    ULONG                           Channel;
} ISOCH_CHANNEL_DATA, *PISOCH_CHANNEL_DATA;

// list of allocated resource handles
//
typedef struct _ISOCH_RESOURCES_DATA {
    struct _ISOCH_RESOURCES_DATA    *Next;
    HANDLE                          hResource;
} ISOCH_RESOURCES_DATA, *PISOCH_RESOURCES_DATA;

//
// list of registered clients
//
typedef struct _CLIENT_DATA {
    ULONG           numClients;
    HWND            hWnd[MAX_CLIENTS];
} CLIENT_DATA, *PCLIENT_DATA;

typedef struct _CLIENT_NODE {
    struct _CLIENT_NODE     *Next;
    HWND                    hWnd;
} CLIENT_NODE, *PCLIENT_NODE;

typedef struct _BUS_RESET_THREAD_PARAMS {
    PSTR                szDeviceName;
    BOOL                bKill;
    HANDLE              hEvent;
    HANDLE              hThread;
    ULONG               ThreadId;
} BUS_RESET_THREAD_PARAMS, *PBUS_RESET_THREAD_PARAMS;

typedef struct _SHARED_DATA {
    HINSTANCE               g_hInstance;
    HWND                    g_hWnd;
    HWND                    g_hWndEdit;
    ULONG                   nAttachedProcess;
    ULONG                   PlatformId;
    STARTUPINFO             StartUpInfo;
    PROCESS_INFORMATION     ProcessInfo;
    DEVICE_DATA             DiagDeviceData;
    DEVICE_DATA             VDevDeviceData;
} SHARED_DATA, *PSHARED_DATA;

#ifdef _1394MAIN_C

HANDLE                      hData;
PSHARED_DATA                SharedData;
LIST_NODE                   ClientList;
PBUS_RESET_THREAD_PARAMS    BusResetThreadParams;

#else


#endif // _1394MAIN_C

//
// 1394main.c
//
BOOL
WINAPI
DllMain(
    HINSTANCE   hInstance, 
    ULONG       ulReason, 
    LPVOID      pvReserved
    );

LRESULT
CALLBACK 
DLL_WndProc(
    HWND        hWnd, 
    UINT        iMsg, 
    WPARAM      wParam, 
    LPARAM      lParam
    );

DWORD
WINAPI
DLL_Thread(
    LPVOID      lpParameter
    );

ULONG
InternalGetWinVersion(
    HWND    hWnd
    );

void
InternalRegisterClient(
    HWND    hWnd
    );

void
InternalDeRegisterClient(
    HWND    hWnd
    );

void
InternalGetDeviceList(
    HWND    hWnd
    );

void
NotifyClients(
    HWND    hWnd,
    DWORD   dwNotify
    );

void
StartBusResetThread(
    HWND    hWnd
    );

void
StopBusResetThread(
    HWND    hWnd
    );

DWORD
WINAPI
BusResetThread(
    LPVOID  lpParameter
    );

PSECURITY_DESCRIPTOR
CreateSd(
    VOID
    );

DWORD
WINAPI
IsochAttachThread(
    LPVOID  lpParameter
    );

DWORD
WINAPI
AsyncLoopbackThread(
    LPVOID  lpParameter
    );

DWORD
WINAPI
AsyncLoopbackThreadEx(
    LPVOID  lpParameter
    );

DWORD
WINAPI
AsyncStreamLoopbackThread(
    LPVOID  lpParameter
    );


