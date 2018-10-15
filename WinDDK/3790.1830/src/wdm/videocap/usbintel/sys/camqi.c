/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   camqi.c

Abstract:

    This module contains the general helper functions for dealing with
    QueryInetrface Pnp IRPs.

Author:

  Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.


Environment:

   Kernel mode only


Revision History:

--*/
#define INITGUID
#include "intelcam.h"



NTSTATUS
INTELCAM_QueryUSBCAMDInterface(
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PUSBCAMD_INTERFACE pUsbcamdInterface
    )
/*++

Routine Description:

    sends a QI to USBCAMD driver.

Arguments:

    PnpDeviceObject -
        Contains the next device object on the Pnp stack.

    pUsbcamdInterface -
        The place in which to return the Reference interface.

Return Value:

    Returns STATUS_SUCCESS if the interface was retrieved, else an error.

--*/
{
    NTSTATUS            Status,ntStatus;
    KEVENT              Event;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;

    ntStatus = STATUS_SUCCESS;

    //
    // There is no file object associated with this Irp, so the event may be located
    // on the stack as a non-object manager object.
    //
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        pDeviceObject,
        NULL,
        0,
        NULL,
        &Event,
        &IoStatusBlock);
    
    if (Irp) {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);
        //
        // Create an interface query out of the Irp.
        //
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)&GUID_USBCAMD_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.Size = sizeof(USBCAMD_INTERFACE);
        IrpStackNext->Parameters.QueryInterface.Version = USBCAMD_VERSION_200;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)pUsbcamdInterface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        Status = IoCallDriver(pDeviceObject, Irp);
        if (Status == STATUS_PENDING) {
            //
            // This waits using KernelMode, so that the stack, and therefore the
            // event on that stack, is not paged out.
            //
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }

    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}



