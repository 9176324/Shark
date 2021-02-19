/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>
#include <devicedefs.h>

#include "Shark.h"

#include "Guard.h"
#include "Reload.h"
#include "PatchGuard.h"
#include "Space.h"

#pragma section( ".block", read, write )

__declspec(allocate(".block")) GPBLOCK GpBlock = { 0 };
__declspec(allocate(".block")) PGBLOCK PgBlock = { 0 };

// #ifdef ALLOC_PRAGMA
// #pragma alloc_text(PAGE, DriverEntry)
// #endif

VOID
NTAPI
DriverUnload(
    __in PDRIVER_OBJECT DriverObject
);

NTSTATUS
NTAPI
DeviceCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
NTAPI
DeviceClose(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
NTAPI
DeviceWrite(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
NTAPI
DeviceRead(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
NTAPI
DeviceControl(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
);

NTSTATUS
NTAPI
DriverEntry(
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING DeviceName = { 0 };
    UNICODE_STRING SymbolicLinkName = { 0 };
    PMMPTE PointerPte = NULL;
    PFN_NUMBER NumberOfPages = 0;

    RtlInitUnicodeString(&DeviceName, DEVICE_STRING);

    Status = IoCreateDevice(
        DriverObject,
        0,
        &DeviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &DeviceObject);

    if (NT_SUCCESS(Status)) {
        DriverObject->MajorFunction[IRP_MJ_CREATE] = (PDRIVER_DISPATCH)DeviceCreate;
        DriverObject->MajorFunction[IRP_MJ_CLOSE] = (PDRIVER_DISPATCH)DeviceClose;
        DriverObject->MajorFunction[IRP_MJ_WRITE] = (PDRIVER_DISPATCH)DeviceWrite;
        DriverObject->MajorFunction[IRP_MJ_READ] = (PDRIVER_DISPATCH)DeviceRead;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)DeviceControl;

        RtlInitUnicodeString(&SymbolicLinkName, SYMBOLIC_STRING);

        Status = IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName);

        if (NT_SUCCESS(Status)) {
            DriverObject->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;

            GpBlock.PgBlock = &PgBlock;
            PgBlock.GpBlock = &GpBlock;

            InitializeGpBlock(&GpBlock);
            InitializeSpace(&GpBlock);

            PointerPte = GetPteAddress(&GpBlock);
            NumberOfPages = BYTES_TO_PAGES(sizeof(GPBLOCK));

            while (NumberOfPages--) {
                PointerPte[NumberOfPages].u.Hard.NoExecute = 0;
            }

            FlushMultipleTb(&GpBlock, sizeof(GPBLOCK), TRUE);

            PointerPte = GetPteAddress(&PgBlock);
            NumberOfPages = BYTES_TO_PAGES(sizeof(PGBLOCK));

            while (NumberOfPages--) {
                PointerPte[NumberOfPages].u.Hard.NoExecute = 0;
            }

            FlushMultipleTb(&PgBlock, sizeof(PGBLOCK), TRUE);

            InitializeGuardTrampoline();

#ifndef PUBLIC
            DbgPrint("[Shark] load\n");
#endif // !PUBLIC
        }
        else {
            IoDeleteDevice(DeviceObject);
        }
    }

    return Status;
}

VOID
NTAPI
DriverUnload(
    __in PDRIVER_OBJECT DriverObject
)
{
    UNICODE_STRING SymbolicLinkName = { 0 };

    RtlInitUnicodeString(&SymbolicLinkName, SYMBOLIC_STRING);
    IoDeleteSymbolicLink(&SymbolicLinkName);
    IoDeleteDevice(DriverObject->DeviceObject);

#ifndef PUBLIC
    DbgPrint("[Shark] - unload\n");
#endif // !PUBLIC
}

NTSTATUS
NTAPI
DeviceCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
NTAPI
DeviceClose(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
NTAPI
DeviceWrite(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
NTAPI
DeviceRead(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = Status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
NTAPI
DeviceControl(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = NULL;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case 0: {
        PgClear(&PgBlock);

        Irp->IoStatus.Information = 0;

        break;
    }

    default: {
        Irp->IoStatus.Information = 0;
        Status = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }
    }

    Irp->IoStatus.Status = Status;

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT);

    return Status;
}
