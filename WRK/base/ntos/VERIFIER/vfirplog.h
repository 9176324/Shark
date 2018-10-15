/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfirplog.h

Abstract:

    This header exposes functions for logging IRP events.

--*/

//
// Log-snapshots are retrievable by user mode for profiling and targetted
// probing of stacks. Content-wise they are heavier.
//
typedef struct _IRPLOG_SNAPSHOT {

    ULONG       Count;
    UCHAR       MajorFunction;
    UCHAR       MinorFunction;
    UCHAR       Flags;
    UCHAR       Control;
    ULONGLONG   ArgArray[4];

} IRPLOG_SNAPSHOT, *PIRPLOG_SNAPSHOT;

VOID
VfIrpLogInit(
    VOID
    );

VOID
VfIrpLogRecordEvent(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot,
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  PIRP                        Irp
    );

ULONG
VfIrpLogGetIrpDatabaseSiloCount(
    VOID
    );

NTSTATUS
VfIrpLogLockDatabase(
    IN  ULONG   SiloNumber
    );

NTSTATUS
VfIrpLogRetrieveWmiData(
    IN  ULONG   SiloNumber,
    OUT PUCHAR  OutputBuffer                OPTIONAL,
    OUT ULONG  *OffsetInstanceNameOffsets,
    OUT ULONG  *InstanceCount,
    OUT ULONG  *DataBlockOffset,
    OUT ULONG  *TotalRequiredSize
    );

VOID
VfIrpLogUnlockDatabase(
    IN  ULONG   SiloNumber
    );

VOID
VfIrpLogDeleteDeviceLogs(
    IN PDEVICE_OBJECT DeviceObject
    );

