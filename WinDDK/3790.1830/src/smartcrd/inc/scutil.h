/***************************************************************************

Copyright (c) 2002 Microsoft Corporation

Module Name:

        SCUTIL.H

Abstract:

        Public interface for Smartcard Driver Utility Library

Environment:

        Kernel Mode Only

Notes:

        THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
        KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
        PURPOSE.

        Copyright (c) 2001 Microsoft Corporation.  All Rights Reserved.


Revision History:

        05/14/2002 : created

Authors:

        Randy Aull


****************************************************************************/

#ifndef __SCUTIL_H__
#define __SCUTIL_H__



typedef NTSTATUS (*PNP_CALLBACK)(PDEVICE_OBJECT DeviceObject,
                                 PIRP Irp);

typedef NTSTATUS (*POWER_CALLBACK)(PDEVICE_OBJECT DeviceObject,
                                   DEVICE_POWER_STATE DeviceState,
                                   PBOOLEAN PostWaitWake);

typedef PVOID   SCUTIL_HANDLE;

NTSTATUS
ScUtil_Initialize(
    SCUTIL_HANDLE           UtilHandle,
    PDEVICE_OBJECT          PhysicalDeviceObject,
    PDEVICE_OBJECT          LowerDeviceObject,
    PSMARTCARD_EXTENSION    SmartcardExtension,
    PIO_REMOVE_LOCK         RemoveLock,
    PNP_CALLBACK            StartDevice,
    PNP_CALLBACK            StopDevice,
    PNP_CALLBACK            RemoveDevice,
    PNP_CALLBACK            FreeResources,
    POWER_CALLBACK          SetPowerState
    );

NTSTATUS
ScUtil_DeviceIOControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_PnP(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_Power(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_Cleanup(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_UnloadDriver(
    PDRIVER_OBJECT DriverObject
    );

NTSTATUS
ScUtil_CreateClose(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
ScUtil_SystemControl(
    PDEVICE_OBJECT   DeviceObject,
    PIRP             Irp
    );

NTSTATUS
ScUtil_ForwardAndWait(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS
ScUtil_Cancel(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );



#endif // __SCUTIL_H__


