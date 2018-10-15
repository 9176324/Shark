/*++

Copyright (c) 1998 Microsoft Corporation

Module Name:

    1394diag.c

Abstract:

Author:

    Peter Binder (pbinder) 4/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   taken from original 1394diag...
--*/

#define _1394DIAG_C
#include "pch.h"
#undef _1394DIAG_C

#if DBG

unsigned char t1394DiagDebugLevel = TL_WARNING;

#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    ENTER("DriverEntry");

    DriverObject->MajorFunction[IRP_MJ_CREATE]          = t1394Diag_Create;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = t1394Diag_Close;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = t1394Diag_Pnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = t1394Diag_Power;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = t1394Diag_IoControl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = t1394Diag_IoControl;
    DriverObject->DriverExtension->AddDevice            = t1394Diag_PnpAddDevice;
//    DriverObject->DriverUnload                          = OhciDiag_Unload;

    EXIT("DriverEntry", ntStatus);
    return(ntStatus);
} // DriverEntry

NTSTATUS
t1394Diag_Create(
    IN PDEVICE_OBJECT   DriverObject,
    IN PIRP             Irp
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    ENTER("t1394Diag_Create");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    EXIT("t1394Diag_Create", ntStatus);
    return(ntStatus);
} // t1394Diag_Create

NTSTATUS
t1394Diag_Close(
    IN PDEVICE_OBJECT   DriverObject,
    IN PIRP             Irp
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    ENTER("t1394Diag_Close");

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    EXIT("t1394Diag_Close", ntStatus);
    return(ntStatus);
} // t1394Diag_Close

void
t1394Diag_CancelIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    KIRQL               Irql;
    PBUS_RESET_IRP      BusResetIrp;
    PDEVICE_EXTENSION   deviceExtension;

    ENTER("t1394Diag_CancelIrp");

    deviceExtension = DeviceObject->DeviceExtension;

    KeAcquireSpinLock(&deviceExtension->ResetSpinLock, &Irql);

    BusResetIrp = (PBUS_RESET_IRP) deviceExtension->BusResetIrps.Flink;

    TRACE(TL_TRACE, ("Irp = 0x%x\n", Irp));

    while (BusResetIrp) {

        TRACE(TL_TRACE, ("Cancelling BusResetIrp->Irp = 0x%x\n", BusResetIrp->Irp));

        if (BusResetIrp->Irp == Irp) {

            RemoveEntryList(&BusResetIrp->BusResetIrpList);
            ExFreePool(BusResetIrp);
            break;
        }
        else if (BusResetIrp->BusResetIrpList.Flink == &deviceExtension->BusResetIrps) {
            break;
        }
        else
            BusResetIrp = (PBUS_RESET_IRP)BusResetIrp->BusResetIrpList.Flink;
    }

    KeReleaseSpinLock(&deviceExtension->ResetSpinLock, Irql);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    EXIT("t1394Diag_CancelIrp", STATUS_SUCCESS);
} // t1394Diag_CancelIrp


