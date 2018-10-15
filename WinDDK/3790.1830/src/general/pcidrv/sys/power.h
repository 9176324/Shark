/*++
Copyright (c) 1990-2000    Microsoft Corporation All Rights Reserved

Module Name:

    power.h

Abstract:

    This module contains the declarations used by power.c

Author:

    Eliyas Yakub Feb 13, 2003

Environment:

    user and kernel
Notes:


Revision History:


--*/

#ifndef __POWER_H
#define __POWER_H

#define MAGIC_NUMBER -1

typedef enum {

    IRP_NEEDS_FORWARDING = 1,
    IRP_ALREADY_FORWARDED

} IRP_DIRECTION;

typedef struct _POWER_COMPLETION_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    PIRP            SIrp;
} POWER_COMPLETION_CONTEXT, *PPOWER_COMPLETION_CONTEXT;


NTSTATUS
PciDrvDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvDispatchPowerDefault(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    );

NTSTATUS
PciDrvDispatchSetPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvDispatchQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvDispatchSystemPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvCompletionSystemPowerUp(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

VOID
PciDrvQueueCorrespondingDeviceIrp(
    IN PIRP SIrp,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
PciDrvCompletionOnFinalizedDeviceIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID   PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    );

NTSTATUS
PciDrvDispatchDeviceQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
PciDrvDispatchDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
PciDrvCompletionDevicePowerUp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID NotUsed
    );

NTSTATUS
PciDrvFinalizeDevicePowerIrp(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction,
    IN  NTSTATUS            Result
    );

VOID
PciDrvCallbackHandleDeviceQueryPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PWORK_ITEM_CONTEXT Context
    );

VOID
PciDrvCallbackHandleDeviceSetPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PWORK_ITEM_CONTEXT Context
    );

NTSTATUS
PciDrvPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    );

NTSTATUS
PciDrvGetPowerPoliciesDeviceState(
    IN  PIRP                SIrp,
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT DEVICE_POWER_STATE *DevicePowerState
    );

NTSTATUS
PciDrvCanSuspendDevice(
    IN PDEVICE_OBJECT   DeviceObject
    );

PCHAR
DbgPowerMinorFunctionString(
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

