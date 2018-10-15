/*++

Copyright (C) Microsoft Corporation, 1999

Module Name:

    mchgr.h

Abstract:

    SCSI Medium Changer class driver

Environment:

    kernel mode only

Notes:

Revision History:

--*/
#ifndef _MCHGR_H_
#define _MCHGR_H_

#include "stdarg.h"
#include "ntddk.h"
#include "mcd.h"

#include "initguid.h"
#include "ntddstor.h"

#include <wmidata.h>
#include <wmistr.h>
#include <stdarg.h>

//
// WMI guid list for changer.
//
extern GUIDREGINFO ChangerWmiFdoGuidList[];

//
// Changer class device extension
//
typedef struct _MCD_CLASS_DATA {
    LONG          DeviceOpen;

#if defined(_WIN64)
    //
    // Force PVOID alignment
    //
    ULONG_PTR Reserved;
#endif

    UNICODE_STRING  MediumChangerInterfaceString;
    ULONG           PnpNameDeviceNumber;
    ULONG           SymbolicNameDeviceNumber;
    BOOLEAN         PnpNameLinkCreated;
    BOOLEAN         DosNameCreated;
    BOOLEAN         PersistentSymbolicName;
    BOOLEAN         PersistencePreferred;

} MCD_CLASS_DATA, *PMCD_CLASS_DATA;


typedef enum _MCD_UID_TYPE {
    MCD_UID_CUSTOM = 1
} MCD_UID_TYPE, *PMCD_UID_TYPE;

typedef enum _MCD_PERSISTENCE_OPERATION {
    MCD_PERSISTENCE_QUERY = 1,
    MCD_PERSISTENCE_SET
} MCD_PERSISTENCE_OPERATION, *PMCD_PERSISTENCE_OPERATION;

NTSTATUS
ChangerClassCreateClose (
  IN PDEVICE_OBJECT DeviceObject,
  IN PIRP Irp
  );

NTSTATUS
ChangerClassDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
ChangerClassError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    );

NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ChangerStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
ChangerInitDevice(
    IN PDEVICE_OBJECT Fdo
    );

NTSTATUS
ChangerRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    );

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

VOID
ChangerUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NTSTATUS
CreateChangerDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  BOOLEAN         LegacyNameFormat
    );

NTSTATUS
ChangerReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
ChangerSymbolicNamePersistencePreference(
    IN PDRIVER_OBJECT DriverObject,
    IN PWCHAR KeyName,
    IN MCD_PERSISTENCE_OPERATION Operation,
    IN PWCHAR ValueName,
    IN OUT PBOOLEAN PersistencePreference
    );

NTSTATUS
ChangerCreateSymbolicName(
    IN PDEVICE_OBJECT Fdo,
    IN BOOLEAN PersistencePreference
    );

NTSTATUS
ChangerCreateUniqueId(
    IN PDEVICE_OBJECT Fdo,
    IN MCD_UID_TYPE UIDType,
    OUT PUCHAR *UniqueId,
    OUT PULONG UniqueIdLen
    );

BOOLEAN
ChangerCompareUniqueId(
    IN PUCHAR UniqueId1,
    IN ULONG UniqueId1Len,
    IN PUCHAR UniqueId2,
    IN ULONG UniqueId2Len,
    IN MCD_UID_TYPE Type
    );

NTSTATUS
ChangerGetSubKeyByIndex(
    IN HANDLE RootKey,
    IN ULONG SubKeyIndex,
    OUT PHANDLE SubKey,
    OUT PWCHAR ReturnString
    );

NTSTATUS
ChangerGetValuePartialInfo(
    IN HANDLE Key,
    IN PUNICODE_STRING ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *KeyValueInfo,
    OUT PULONG KeyValueInfoSize
    );

NTSTATUS
ChangerCreateNewDeviceSubKey(
    IN HANDLE RootKey,
    IN OUT PULONG FirstNumeralToUse,
    OUT PHANDLE NewSubKey,
    OUT PWCHAR NewSubKeyName
    );

NTSTATUS
ChangerCreateNonPersistentSymbolicName(
    IN HANDLE RootKey,
    IN OUT PULONG FirstNumeralToUse,
    OUT PWCHAR NonPersistentSymbolicName
    );

NTSTATUS
ChangerAssignSymbolicLink(
    IN ULONG DeviceNumber,
    IN PUNICODE_STRING RegDeviceName,
    IN BOOLEAN LegacyNameFormat,
    IN PWCHAR SymbolicNamePrefix,
    OUT PULONG SymbolicNameNumeral
    );

NTSTATUS
ChangerDeassignSymbolicLink(
    IN PWCHAR SymbolicNamePrefix,
    IN ULONG SymbolicNameNumeral,
    IN BOOLEAN LegacyNameFormat
    );


//
// WMI routines
//
NTSTATUS
ChangerFdoQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName
    );

NTSTATUS
ChangerFdoQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

NTSTATUS
ChangerFdoSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerFdoSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerFdoExecuteWmiMethod(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN PUCHAR Buffer
    );

NTSTATUS
ChangerWmiFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN CLASSENABLEDISABLEFUNCTION Function,
    IN BOOLEAN Enable
    );

#endif // _MCHGR_H_

