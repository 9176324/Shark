/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Power.h

Abstract:

    Power.h defines the data types used in all the stages of the function driver
    which implement power management support in Power.c.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub - 16-Set-1998

    Kevin Shirley - 01-Jul-2003 - Commented for tutorial and learning purposes

--*/

#if !defined(_POWER_H_)
#define _POWER_H_

//
// Define an enumeration to describe whether a power IRP has been processed by the
// underlying bus driver and does not need to be passed down the device stack, or
// whether the power IRP has not been passed down to the underlying bus driver to
// be processed.
//
// IRP_NEEDS_FORWARDING indicates that the function driver must pass the power IRP
// down the device stack to be processed by the underlying bus driver.
//
// IRP_MN_ALREADY_FORWARDED indicates that the underlying bus driver has already
// processed the power IRP and the function driver does not need to pass the power
// IRP down the device stack.
//
typedef enum {

    IRP_NEEDS_FORWARDING = 1,
    IRP_ALREADY_FORWARDED

} IRP_DIRECTION;

//
// Define a function signature for a work item for the system worker thread to
// call back at IRQL = PASSIVE_LEVEL.
//
typedef VOID (*PFN_QUEUE_SYNCHRONIZED_CALLBACK)(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction
    );

//
// Define the operating context for a work item for the system worker thread to
// process a power IRP at IRQL = PASSIVE_LEVEL.
//
// The context contains the hardware instance and power IRP to process at
// IRQL = PASSIVE_LEVEL. The context also contains whether the power IRP has
// already been passed down the device stack to be processed by the underlying
// bus driver, or if the power IRP must still be passed down the device stack.
// The context also contains a pointer to the callback function implemented by
// the function driver. The callback function must match the function signature
// defined by the PFN_QUEUE_SYNCHRONIZED_CALLBACK data type.
// The context also contains the work item for the system worker thread to process
// at IRQL = PASSIVE_LEVEL.
//
typedef struct _WORKER_THREAD_CONTEXT {

    PDEVICE_OBJECT                  DeviceObject;
    PIRP                            Irp;
    IRP_DIRECTION                   IrpDirection;
    PFN_QUEUE_SYNCHRONIZED_CALLBACK Callback;
    PIO_WORKITEM                    WorkItem;

} WORKER_THREAD_CONTEXT, *PWORKER_THREAD_CONTEXT;


//
// Declare the function prototypes for all of the function driver's power routines in
// the Featured1 and Featured2 stages of the function driver.
//

NTSTATUS
ToasterDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterDispatchPowerDefault(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    );

NTSTATUS
ToasterDispatchSetPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterDispatchQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ToasterDispatchSystemPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    );

NTSTATUS
ToasterCompletionSystemPowerUp (
    IN PDEVICE_OBJECT   Fdo ,
    IN PIRP             Irp ,
    IN PVOID            NotUsed 
    );

VOID
ToasterQueueCorrespondingDeviceIrp (
    IN PIRP SIrp,
    IN PDEVICE_OBJECT DeviceObject
     );

VOID
ToasterCompletionOnFinalizedDeviceIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID   PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    );

NTSTATUS
ToasterDispatchDeviceQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp 
    );

NTSTATUS
ToasterDispatchDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp 
    );

NTSTATUS
ToasterCompletionDevicePowerUp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp ,
    IN PVOID NotUsed 
    );

NTSTATUS
ToasterFinalizeDevicePowerIrp (
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction,
    IN  NTSTATUS            Result
    );

NTSTATUS
ToasterQueuePassiveLevelPowerCallback(
    IN  PDEVICE_OBJECT                      DeviceObject,
    IN  PIRP                                Irp,
    IN  IRP_DIRECTION                       Direction,
    IN  PFN_QUEUE_SYNCHRONIZED_CALLBACK     Callback
    );

VOID
ToasterCallbackHandleDeviceQueryPower (
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction
    );

VOID
ToasterCallbackHandleDeviceSetPower (
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction
    );

VOID
ToasterQueuePassiveLevelPowerCallbackWorker(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    );

NTSTATUS
ToasterPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    );

NTSTATUS
ToasterGetPowerPoliciesDeviceState(
    IN  PIRP                SIrp,
    IN  PDEVICE_OBJECT      DeviceObject,
     OUT DEVICE_POWER_STATE *DevicePowerState
    );

NTSTATUS
ToasterCanSuspendDevice(
    IN PDEVICE_OBJECT   DeviceObject
    );

PCHAR
DbgPowerMinorFunctionString (
    UCHAR MinorFunction
    );

PCHAR
DbgSystemPowerString(
    IN SYSTEM_POWER_STATE Type
    );

PCHAR
DbgDevicePowerString(
    IN DEVICE_POWER_STATE Type
    );

#endif

