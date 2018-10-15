/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ppvutil.h

Abstract:

    This header exposes various utilities required to do driver verification.

--*/

#ifndef _PPVUTIL_H_
#define _PPVUTIL_H_

typedef enum {

    PPVERROR_DUPLICATE_PDO_ENUMERATED           = 0,
    PPVERROR_MISHANDLED_TARGET_DEVICE_RELATIONS,
    PPVERROR_DDI_REQUIRES_PDO

} PPVFAILURE_TYPE;

typedef enum {

    PPVREMOVAL_SHOULD_DELETE           = 0,
    PPVREMOVAL_SHOULDNT_DELETE,
    PPVREMOVAL_MAY_DEFER_DELETION

} PPVREMOVAL_OPTION;

VOID
FASTCALL
PpvUtilInit(
    VOID
    );

NTSTATUS
FASTCALL
PpvUtilCallAddDevice(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDRIVER_ADD_DEVICE  AddDeviceFunction,
    IN  VF_DEVOBJ_TYPE      DevObjType
    );

VOID
FASTCALL
PpvUtilTestStartedPdoStack(
    IN  PDEVICE_OBJECT  DeviceObject
    );

PPVREMOVAL_OPTION
FASTCALL
PpvUtilGetDevnodeRemovalOption(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

VOID
FASTCALL
PpvUtilFailDriver(
    IN  PPVFAILURE_TYPE FailureType,
    IN  PVOID           CulpritAddress,
    IN  PDEVICE_OBJECT  DeviceObject    OPTIONAL,
    IN  PVOID           ExtraneousInfo  OPTIONAL
    );

BOOLEAN
FASTCALL
PpvUtilIsHardwareBeingVerified(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject
    );

#endif // _PPVUTIL_H_

