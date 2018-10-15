/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    firefly.h

Abstract:

    This header contains Device driver that controls the Firefly device.

Environment:

    Kernel mode

Revision History:

    Adrian J. Oney - Written March 17th, 2001
                     (based on Toaster DDK filter sample)

--*/

#if !defined(_FIREFLY_H_)
#define _FIREFLY_H_

#include <ntddk.h>
#include <initguid.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>
#include "firefly.h"
#include "magic.h"

//
// This is the WMI guid for our functionality.
//
//{ab27db29-db25-42e6-a3e7-28bd46bdb666}
DEFINE_GUID(                                                                  \
    FIREFLY_WMI_STD_DATA_GUID,                                                \
    0xAB27DB29, 0xDB25, 0x42E6, 0xA3, 0xE7, 0x28, 0xBD, 0x46, 0xBD, 0xB6, 0x66\
    );


#define DRIVERNAME "firefly.sys: "

#define POOL_TAG   'fFtS'

#if DBG
#define DebugPrint(_x_) \
               DbgPrint(DRIVERNAME); \
               DbgPrint _x_; 
#else
#define DebugPrint(_x_) 
#endif

#define ARRAYLEN(array) (sizeof(array)/sizeof(array[0]))

//
// These are the states Filter transition to upon receiving a specific PnP Irp.
// Refer to the PnP Device States diagram in DDK documentation for better
// understanding.
//
typedef enum {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted                 // Device has received the REMOVE_DEVICE IRP

} DEVICE_PNP_STATE;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\


typedef struct {

    //
    // TRUE if the tail light is on, FALSE otherwise.
    //
    BOOLEAN TailLit;

} FIREFLY_WMI_STD_DATA, *PFIREFLY_WMI_STD_DATA;

typedef struct {

    //
    // A back pointer to the device object.
    //
    PDEVICE_OBJECT Self;

    //
    // The top of the stack before this filter was added.
    //
    PDEVICE_OBJECT NextLowerDriver;

    //
    // Bottom of the stack.
    //
    PDEVICE_OBJECT PhysicalDeviceObject;

    //
    // Current PnP state of the device
    //
    DEVICE_PNP_STATE DevicePnPState;

    //
    // Previous PnP state of the device
    //
    DEVICE_PNP_STATE PreviousPnPState;

    //
    // WMI Information.
    //
    WMILIB_CONTEXT WmiLibInfo;

    //
    // Removelock to track IRPs so that device can be removed and 
    // the driver can be unloaded safely.
    //
    IO_REMOVE_LOCK RemoveLock; 

    //
    // Our firefly filter data.
    //
    FIREFLY_WMI_STD_DATA StdDeviceData;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

NTSTATUS
FireflyAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

VOID
FireflyUnload(
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
FireflyDispatchPnp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
FireflySynchronousCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
FireflyDispatchPower(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
FireflyDispatchWmi(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    );

NTSTATUS
FireflyForwardIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
FireflyRegisterWmi(
    IN  PDEVICE_EXTENSION   DeviceExtension
    );

NTSTATUS
FireflySetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
FireflySetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
FireflyQueryWmiDataBlock(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
    IN      ULONG           GuidIndex,
    IN      ULONG           InstanceIndex,
    IN      ULONG           InstanceCount,
    IN OUT  PULONG          InstanceLengthArray,
    IN      ULONG           OutBufferSize,
    OUT     PUCHAR          Buffer
    );

NTSTATUS
FireflyQueryWmiRegInfo(
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT ULONG              *RegFlags,
    OUT PUNICODE_STRING     InstanceName,
    OUT PUNICODE_STRING    *RegistryPath,
    OUT PUNICODE_STRING     MofResourceName,
    OUT PDEVICE_OBJECT     *Pdo
    );

PCHAR
PnPMinorFunctionString(
    IN UCHAR            MinorFunction
    );

NTSTATUS
FireflySendIoctl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  ULONG           ControlCode,
    IN  PVOID           InputBuffer,
    IN  ULONG           InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN  ULONG           OutputBufferLength,
    IN  PFILE_OBJECT    FileObject
    );

NTSTATUS
FireflySetFeature(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  UCHAR           PageId,
    IN  USHORT          FeatureId,
    IN  BOOLEAN         EnableFeature
    );

NTSTATUS
FireflyOpenStack(
    IN  PDEVICE_OBJECT   DeviceObject,
    OUT PFILE_OBJECT    *FileObject
    );

#endif // _FIREFLY_H_


