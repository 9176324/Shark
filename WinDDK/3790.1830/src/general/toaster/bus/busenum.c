/*++

Copyright (c) 1990-2000  Microsoft Corporation All Rights Reserved

Module Name:

    BUSENUM.C

Abstract:

    This module contains the entry points for a toaster bus driver.

Author:

    Eliyas Yakub    Sep 10, 1998
    
Environment:

    kernel mode only

Revision History:

    Cleaned up sample 05/05/99
    Fixed the create_close and ioctl handler to fail the request sent on the child stack - 3/15/04


--*/

#include "busenum.h"


//
// Global Debug Level
//

ULONG BusEnumDebugLevel = BUS_DEFAULT_DEBUG_OUTPUT_LEVEL;


GLOBALS Globals;


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, Bus_DriverUnload)
#pragma alloc_text (PAGE, Bus_CreateClose)
#pragma alloc_text (PAGE, Bus_IoCtl)
#endif

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
/*++
Routine Description:

    Initialize the driver dispatch table. 

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

  NT Status Code

--*/
{

    Bus_KdPrint_Def (BUS_DBG_SS_TRACE, ("Driver Entry \n"));

    //
    // Save the RegistryPath for WMI.
    //

    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;
    Globals.RegistryPath.Buffer = ExAllocatePoolWithTag(
                                       PagedPool,
                                       Globals.RegistryPath.MaximumLength,
                                       BUSENUM_POOL_TAG
                                       );    

    if (!Globals.RegistryPath.Buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlCopyUnicodeString(&Globals.RegistryPath, RegistryPath);

    //
    // Set entry points into the driver
    //
    DriverObject->MajorFunction [IRP_MJ_CREATE] =
    DriverObject->MajorFunction [IRP_MJ_CLOSE] = Bus_CreateClose;
    DriverObject->MajorFunction [IRP_MJ_PNP] = Bus_PnP;
    DriverObject->MajorFunction [IRP_MJ_POWER] = Bus_Power;
    DriverObject->MajorFunction [IRP_MJ_DEVICE_CONTROL] = Bus_IoCtl;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = Bus_SystemControl;
    DriverObject->DriverUnload = Bus_DriverUnload;
    DriverObject->DriverExtension->AddDevice = Bus_AddDevice;

    return STATUS_SUCCESS;
}

NTSTATUS
Bus_CreateClose (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++
Routine Description:

    Some outside source is trying to create a file against us.

    If this is for the FDO (the bus itself) then the caller 
    is trying to open the proprietary connection to tell us 
    to enumerate or remove a device.

    If this is for the PDO (an object on the bus) then this 
    is a client that wishes to use the toaster device.
    
Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PFDO_DEVICE_DATA    fdoData;
    PCOMMON_DEVICE_DATA     commonData;

    PAGED_CODE ();

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    //
    // We only allow create/close requests for the FDO.
    // That is the bus itself.
    //

    if (!commonData->IsFDO) {
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    Bus_IncIoCount (fdoData);

    //
    // Check to see whether the bus is removed
    //
    
    if (fdoData->DevicePnPState == Deleted) {
        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }    
         

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    switch (irpStack->MajorFunction) {
    case IRP_MJ_CREATE:
        Bus_KdPrint_Def (BUS_DBG_SS_TRACE, ("Create \n"));
        status = STATUS_SUCCESS;
        break;

    case IRP_MJ_CLOSE:
        Bus_KdPrint_Def (BUS_DBG_SS_TRACE, ("Close \n"));
        status = STATUS_SUCCESS;
        break;
     default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    } 
        
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    Bus_DecIoCount (fdoData);
    return status;
}

NTSTATUS
Bus_IoCtl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++
Routine Description:

    Handle user mode PlugIn, UnPlug and device Eject requests.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    ULONG                   inlen;
    PFDO_DEVICE_DATA        fdoData;
    PVOID                   buffer;
    PCOMMON_DEVICE_DATA     commonData;

    PAGED_CODE ();

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;
    //
    // We only allow create/close requests for the FDO.
    // That is the bus itself.
    //
    if (!commonData->IsFDO) {
        Irp->IoStatus.Status = status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
    
    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    Bus_IncIoCount (fdoData);

    //
    // Check to see whether the bus is removed
    //
    
    if (fdoData->DevicePnPState == Deleted) {
        status = STATUS_NO_SUCH_DEVICE;
        goto END;
    }    
    
    irpStack = IoGetCurrentIrpStackLocation (Irp);

    buffer = Irp->AssociatedIrp.SystemBuffer;  
    inlen = irpStack->Parameters.DeviceIoControl.InputBufferLength;

    status = STATUS_INVALID_PARAMETER;
    
    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_BUSENUM_PLUGIN_HARDWARE:
        if (
            //
            // Make sure it has at least two nulls and the size 
            // field is set to the declared size of the struct
            //
            ((sizeof (BUSENUM_PLUGIN_HARDWARE) + sizeof(UNICODE_NULL) * 2) <=
             inlen) &&

            //
            // The size field should be set to the sizeof the struct as declared
            // and *not* the size of the struct plus the multi_sz
            //
            (sizeof (BUSENUM_PLUGIN_HARDWARE) ==
             ((PBUSENUM_PLUGIN_HARDWARE) buffer)->Size)) {

            Bus_KdPrint(fdoData, BUS_DBG_IOCTL_TRACE, ("PlugIn called\n"));

            status= Bus_PlugInDevice((PBUSENUM_PLUGIN_HARDWARE)buffer,
                                inlen, fdoData);
           

        }
        break;

    case IOCTL_BUSENUM_UNPLUG_HARDWARE:

        if ((sizeof (BUSENUM_UNPLUG_HARDWARE) == inlen) &&
              (((PBUSENUM_UNPLUG_HARDWARE)buffer)->Size == inlen)) {

            Bus_KdPrint(fdoData, BUS_DBG_IOCTL_TRACE, ("UnPlug called\n"));

            status= Bus_UnPlugDevice(
                    (PBUSENUM_UNPLUG_HARDWARE)buffer, fdoData);

        }
        break;

    case IOCTL_BUSENUM_EJECT_HARDWARE:
    
        if ((sizeof (BUSENUM_EJECT_HARDWARE) == inlen) &&
            (((PBUSENUM_EJECT_HARDWARE)buffer)->Size == inlen)) {

            Bus_KdPrint(fdoData, BUS_DBG_IOCTL_TRACE, ("Eject called\n"));

            status= Bus_EjectDevice((PBUSENUM_EJECT_HARDWARE)buffer, fdoData);

        }
        break;   

    default:
        break; // default status is STATUS_INVALID_PARAMETER
    }

    Irp->IoStatus.Information = 0;
END:
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    Bus_DecIoCount (fdoData);
    return status;
}


VOID
Bus_DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    )
/*++
Routine Description:
    Clean up everything we did in driver entry.

Arguments:

   DriverObject - pointer to this driverObject.


Return Value:

--*/
{
    PAGED_CODE ();

    Bus_KdPrint_Def (BUS_DBG_SS_TRACE, ("Unload\n"));
    
    //
    // All the device objects should be gone.
    //

    ASSERT (NULL == DriverObject->DeviceObject);

    //
    // Here we free all the resources allocated in the DriverEntry
    //

    if(Globals.RegistryPath.Buffer)
        ExFreePool(Globals.RegistryPath.Buffer);   
        
    return;
}

VOID
Bus_IncIoCount (
    IN  PFDO_DEVICE_DATA   FdoData
    )   

/*++

Routine Description:

    This routine increments the number of requests the device receives
    

Arguments:

    FdoData - pointer to the FDO device extension.
    
Return Value:

    VOID

--*/

{

    LONG            result;


    result = InterlockedIncrement(&FdoData->OutstandingIO);

    ASSERT(result > 0);
    //
    // Need to clear StopEvent (when OutstandingIO bumps from 1 to 2) 
    //
    if (result == 2) {
        //
        // We need to clear the event
        //
        KeClearEvent(&FdoData->StopEvent);
    }

    return;
}

VOID
Bus_DecIoCount(
    IN  PFDO_DEVICE_DATA  FdoData
    )   

/*++

Routine Description:

    This routine decrements as it complete the request it receives

Arguments:

    FdoData - pointer to the FDO device extension.
    
Return Value:

    VOID

--*/
{

    LONG            result;
    
    result = InterlockedDecrement(&FdoData->OutstandingIO);

    ASSERT(result >= 0);

    if (result == 1) {
        //
        // Set the stop event. Note that when this happens
        // (i.e. a transition from 2 to 1), the type of requests we 
        // want to be processed are already held instead of being 
        // passed away, so that we can't "miss" a request that
        // will appear between the decrement and the moment when
        // the value is actually used.
        //
 
        KeSetEvent (&FdoData->StopEvent, IO_NO_INCREMENT, FALSE);
        
    }
    
    if (result == 0) {

        //
        // The count is 1-biased, so it can be zero only if an 
        // extra decrement is done when a remove Irp is received 
        //

        ASSERT(FdoData->DevicePnPState == Deleted);

        //
        // Set the remove event, so the device object can be deleted
        //

        KeSetEvent (&FdoData->RemoveEvent, IO_NO_INCREMENT, FALSE);
        
    }

    return;
}

