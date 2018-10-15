/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved
 
Module Name:

    toastmon.h

Abstract:  

Author:  Eliyas Yakub     Feb 15, 2000

Environment:

    Kernel mode

Revision History:

--*/
#include <ntddk.h>

#if     !defined(__TOASTMON_H__)
#define __TOASTMON_H__

#define DRIVER_TAG 'NOMT'

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\
//
// These are the states a PDO or FDO transition upon
// receiving a specific PnP Irp. Refer to the PnP Device States
// diagram in DDK documentation for better understanding.
//

typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted,                // Device has received the REMOVE_DEVICE IRP
    UnKnown                 // Unknown state

} DEVICE_PNP_STATE;


typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT      DeviceObject; // The ToastMon device object.
    PDEVICE_OBJECT      TopOfStack;   // The top of the stack
    DEVICE_PNP_STATE     DevicePnPState; // Track PnP state
    DEVICE_PNP_STATE     PreviousPnPState;
    IO_REMOVE_LOCK      RemoveLock;   // 
    PVOID                NotificationHandle; // Interface notification handle
    LIST_ENTRY            DeviceListHead; // List to queue the target device info
    FAST_MUTEX          ListMutex;    // Synchronize access to DeviceList
    PVOID		WMIDeviceArrivalNotificationObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _DEVICE_INFO {
    PDEVICE_EXTENSION    DeviceExtension; // Our FDO device extension
    PDEVICE_OBJECT      TargetDeviceObject; // Toplevel deviceobject of the target device
    PDEVICE_OBJECT      Pdo;          // It's PDO
    PFILE_OBJECT        FileObject;   
    PVOID                NotificationHandle; //Device change notification handle
    LIST_ENTRY            ListEntry; // Entry to chain to the listhead
    UNICODE_STRING        SymbolicLink; 
} DEVICE_INFO, *PDEVICE_INFO;


#if DBG
#define DebugPrint(_x_) \
               DbgPrint ("ToastMon:"); \
               DbgPrint _x_;

#define TRAP() DbgBreakPoint()

#else
#define DebugPrint(_x_)
#define TRAP()
#endif


/********************* function prototypes ***********************************/
//

NTSTATUS    
DriverEntry(       
    IN  PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPath 
    );


NTSTATUS    
ToastMon_Dispatch(       
    IN  PDEVICE_OBJECT pDO,
    IN  PIRP pIrp                    
    );

VOID        
ToastMon_Unload(         
    IN  PDRIVER_OBJECT DriverObject 
    );


NTSTATUS
ToastMon_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );


NTSTATUS
ToastMon_DispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToastMon_StartDevice (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
ToastMon_DispatchPower(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );
NTSTATUS 
ToastMon_DispatchSystemControl(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
ToastMon_PnpNotifyDeviceChange(
    IN  PPLUGPLAY_NOTIFICATION_HEADER         NotificationStruct,
    IN  PVOID                                  Context
    );


NTSTATUS
ToastMon_PnpNotifyInterfaceChange(
    IN  PDEVICE_INTERFACE_CHANGE_NOTIFICATION NotificationStruct,
    IN  PVOID                        Context
    );    

NTSTATUS 
ToastMon_GetTargetDevicePdo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_OBJECT* PhysicalDeviceObject
    );

NTSTATUS
ToastMon_OpenTargetDevice(
    PDEVICE_INFO        List
    );
    
VOID
ToastMon_CloseTargetDevice(
    PDEVICE_INFO        List
    );
    
PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
);

NTSTATUS
RegisterForWMINotification(
    PDEVICE_EXTENSION DeviceExt
    );

VOID
UnregisterForWMINotification(
    PDEVICE_EXTENSION DeviceExt
);

VOID
WmiNotificationCallback(
    IN PVOID Wnode,
    IN PVOID Context
    );

#endif
