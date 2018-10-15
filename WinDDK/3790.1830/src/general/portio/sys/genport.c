/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Module Name:

    genport.c

Abstract:  Generic Port I/O driver for Windows 2000


Author:    Author:    Robert R. Howell  January 8, 1993


Environment:

    Kernel mode

Revision History:

 Robert B. Nelson (Microsoft)     January 12, 1993
      Cleaned up comments
      Enabled and tested resource reporting
      Added code to retrieve I/O address and port count from the Registry.

 Robert B. Nelson (Microsoft)     March 1, 1993
      Added support for byte, word, and long I/O.
      Added support for MIPS.
      Fixed resource reporting.

 Robert B. Nelson (Microsoft)     May 1, 1993
      Fixed port number validation.

 Robert B. Nelson (Microsoft)     Oct 25, 1993
      Fixed MIPS support.
      
 Eliyas Yakub 
    Fixed AddressSpace Bug          Nov 30, 1997
    Converted to be PNP             Dec 29, 1998
    Fixed remlock bugs                              Feb 17, 2000

--*/

#include "genport.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, GpdAddDevice)
#pragma alloc_text (PAGE, GpdDispatchPnp)
#pragma alloc_text (PAGE, GpdDispatchPower)
#pragma alloc_text (PAGE, GpdDispatchSystemControl)
#pragma alloc_text (PAGE, GpdUnload)
#pragma alloc_text (PAGE, GpdDispatch)
#pragma alloc_text (PAGE, GpdIoctlReadPort)
#pragma alloc_text (PAGE, GpdIoctlWritePort)
#pragma alloc_text (PAGE, GpdStartDevice)
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS

--*/
{

    UNREFERENCED_PARAMETER (RegistryPath);

    DebugPrint (("Entered Driver Entry\n"));
    
    //
    // Create dispatch points for the IRPs.
    //
    
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = GpdDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = GpdDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = GpdDispatch;
    DriverObject->DriverUnload                          = GpdUnload;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = GpdDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = GpdDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = GpdDispatchSystemControl;
    DriverObject->DriverExtension->AddDevice           = GpdAddDevice;

    return STATUS_SUCCESS;
}


NTSTATUS
GpdAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

Routine Description:

    The Plug & Play subsystem is handing us a brand new PDO, for which we
    (by means of INF registration) have been asked to provide a driver.

    We need to determine if we need to be in the driver stack for the device.
    Create a functional device object to attach to the stack
    Initialize that device object
    Return status success.

    Remember: we can NOT actually talk to the device UNTIL we have 
    received an IRP_MN_START_DEVICE.

Arguments:

    DeviceObject - pointer to a device object.

    PhysicalDeviceObject -  pointer to a device object created by the
                            underlying bus driver.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PLOCAL_DEVICE_INFO      deviceInfo; 
    UNICODE_STRING          ntDeviceName;
    UNICODE_STRING          win32DeviceName;

    PAGED_CODE();

    DebugPrint(("Entered AddDevice: %p\n", PhysicalDeviceObject));

    RtlInitUnicodeString(&ntDeviceName, GPD_DEVICE_NAME);


    //
    // Create a named deviceobject so that legacy applications can talk to us.
    // Since we are naming the object, we wouldn't able to install multiple
    // instance of this driver. Please note that as per PNP guidelines, we 
    // should not name the FDO or create symbolic links. We are doing it because
    // we have a legacy APP that doesn't know how to open an interface.
    //
        
    status = IoCreateDevice(DriverObject,
                             sizeof (LOCAL_DEVICE_INFO),
                             &ntDeviceName,
                             GPD_TYPE,
                             FILE_DEVICE_SECURE_OPEN, // do security checks on relative open
                             FALSE,
                             &deviceObject);

    
    if (!NT_SUCCESS (status)) {
        //
        // Either not enough memory to create a deviceobject or another
        // deviceobject with the same name exits. This could happen
        // if you install another instance of this device.
        //
        DebugPrint(("IoCreateDevice failed: %x\n", status));
        return status;
    }

    RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);

    status = IoCreateSymbolicLink( &win32DeviceName, &ntDeviceName );

    if (!NT_SUCCESS(status))    // If we we couldn't create the link then
    {                           //  abort installation.
        IoDeleteDevice(deviceObject);
        return status;
    }

    deviceInfo = (PLOCAL_DEVICE_INFO) deviceObject->DeviceExtension;
    
    deviceInfo->NextLowerDriver = IoAttachDeviceToDeviceStack (
                                       deviceObject,
                                       PhysicalDeviceObject);
    if(NULL == deviceInfo->NextLowerDriver) {
        IoDeleteSymbolicLink(&win32DeviceName);
        IoDeleteDevice(deviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // We will use remove lock to keep count of IRPs so that 
    // we don't deteach or delete our deviceobject until all pending I/Os
    // are completed. 
    //
    
    IoInitializeRemoveLock (&deviceInfo->RemoveLock , 
                            PORTIO_TAG,
                            1, // MaxLockedMinutes 
                            5); // HighWatermark, this parameter is 
                                // used only on checked build.
    //
    // Set the flag if the device is not holding a pagefile
    // crashdump file or hibernate file. 
    // 
    
    deviceObject->Flags |=  DO_POWER_PAGABLE;

    deviceInfo->DeviceObject = deviceObject;

    //
    // Set the initial state of the FDO
    //

    INITIALIZE_PNP_STATE(deviceInfo);

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    //
    // If you provide read/write dispatch handlers, make sure
    // to specify the preferred buffering method for the I/O.
    // deviceObject->Flags |= DO_BUFFERED_IO;

    //
    // This values is based on the hardware design.
    // I'm assuming the address is in I/O space for our hardware. 
    // Refer http://support.microsoft.com/default.aspx?scid=kb;en-us;Q323595
    // for more info.
    //
    deviceInfo->PortMemoryType = 1; 

    DebugPrint(("AddDevice: %p to %p->%p \n", deviceObject, 
                       deviceInfo->NextLowerDriver,
                       PhysicalDeviceObject));


    return STATUS_SUCCESS;

}

NTSTATUS 
GpdCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

Routine Description:

    The completion routine for plug & play irps that needs to be
    processed first by the lower drivers. 

Arguments:

   DeviceObject - pointer to a device object.   

   Irp - pointer to an I/O Request Packet.

   Context - pointer to an event object.

Return Value:

      NT status code

--*/
{
    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // If the lower driver didn't return STATUS_PENDING, we don't need to 
    // set the event because we won't be waiting on it. 
    // This optimization avoids grabbing the dispatcher lock and improves perf.
    //
    //

    if (Irp->PendingReturned == TRUE) {
        KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
    }

    //
    // The dispatch routine will have to call IoCompleteRequest
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
GpdDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    The plug and play dispatch routines.

    Most of these the driver will completely ignore.
    In all cases it must pass the IRP to the next lower driver.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PIO_STACK_LOCATION          irpStack;
    NTSTATUS                    status = STATUS_SUCCESS;
    KEVENT                      event;        
    UNICODE_STRING              win32DeviceName;
    PLOCAL_DEVICE_INFO          deviceInfo;
 
    PAGED_CODE();

    deviceInfo = (PLOCAL_DEVICE_INFO) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    status = IoAcquireRemoveLock (&deviceInfo->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    DebugPrint(("%s\n",PnPMinorFunctionString(irpStack->MinorFunction)));

    switch (irpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:

        //
        // The device is starting.
        //
        // We cannot touch the device (send it any non pnp irps) until a
        // start device has been passed down to the lower drivers.
        //
        IoCopyCurrentIrpStackLocationToNext(Irp);
        KeInitializeEvent(&event,
                                  NotificationEvent,
                                  FALSE
                                  );

        IoSetCompletionRoutine(Irp,
                               (PIO_COMPLETION_ROUTINE) GpdCompletionRoutine, 
                               &event,
                               TRUE,
                               TRUE,
                               TRUE);
                               
        status = IoCallDriver(deviceInfo->NextLowerDriver, Irp);

        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &event,
               Executive, // Waiting for reason of a driver
               KernelMode, // Must be kernelmode if event memory is in stack
               FALSE, // No allert
               NULL); // No timeout
        }

        if (NT_SUCCESS(status) && NT_SUCCESS(Irp->IoStatus.Status)) {

            status = GpdStartDevice(DeviceObject, Irp);
            if(NT_SUCCESS(status))
            {
                //
                // As we are successfully now back from our start device
                // we can do some device specific work.
                //
                SET_NEW_PNP_STATE(deviceInfo, Started);                
            }
        }

        //
        // We must now complete the IRP, since we stopped it in the
        // completion routine with STATUS_MORE_PROCESSING_REQUIRED.
        //
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        //
        // Fail the query stop to prevent the system from taking away hardware 
        // resources. We do this because our hardware is a legacy hardware and 
        // it can't work with any other resources.
        // If you do support this, you must have a queue to hold
        // incoming requests between stop and subsequent start with new set of
        // resources. Check the toaster sample to learn how to do that.
        //

        SET_NEW_PNP_STATE(deviceInfo, StopPending);

        Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;
        
    case IRP_MN_QUERY_REMOVE_DEVICE:
        //
        // The device can be removed without disrupting the machine. 
        //

       SET_NEW_PNP_STATE(deviceInfo, RemovePending);
        
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceInfo->NextLowerDriver, Irp);
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        //
        // The device has been unexpectedly removed from the machine 
        // and is no longer available for I/O. Stop all access to the device.
        // Release any resources associated with the device, but leave the 
        // device object attached to the device stack until the PnP Manager 
        // sends a subsequent IRP_MN_REMOVE_DEVICE request. 
        // You should fail any outstanding I/O to the device. You will
        // not get a remove until all the handles open to the device
        // have been closed.
        //

        SET_NEW_PNP_STATE(deviceInfo, SurpriseRemovePending);
       
        if (deviceInfo->PortWasMapped)
        {
            MmUnmapIoSpace(deviceInfo->PortBase, deviceInfo->PortCount);
            deviceInfo->PortWasMapped = FALSE;
        }
        RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);
        IoDeleteSymbolicLink(&win32DeviceName);           
        
        IoSkipCurrentIrpStackLocation(Irp);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = IoCallDriver(deviceInfo->NextLowerDriver, Irp);
        break;       
        
    case IRP_MN_REMOVE_DEVICE:

        //
        // Relinquish all resources here.
        // Detach and delete the device object so that
        // your driver can be unloaded. You get remove
        // either after query_remove or surprise_remove.
        //

        if(SurpriseRemovePending != deviceInfo->DevicePnPState)
        {
            //
            // We received query-remove earlier so free up resources.
            //
            if (deviceInfo->PortWasMapped)
            {
                MmUnmapIoSpace(deviceInfo->PortBase, deviceInfo->PortCount);
                deviceInfo->PortWasMapped = FALSE;
            }
            RtlInitUnicodeString(&win32DeviceName, DOS_DEVICE_NAME);
            IoDeleteSymbolicLink(&win32DeviceName);           
        }        

        SET_NEW_PNP_STATE(deviceInfo, Deleted);

        //
        // Wait for all outstanding requests to complete
        //
        DebugPrint(("Waiting for outstanding requests\n"));
        IoReleaseRemoveLockAndWait(&deviceInfo->RemoveLock, Irp);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceInfo->NextLowerDriver, Irp);

        IoDetachDevice(deviceInfo->NextLowerDriver); 
        IoDeleteDevice(DeviceObject);
        
        return status;
                
    case IRP_MN_STOP_DEVICE:

        ASSERTMSG("We failed query stop, so we should never get this request\n", FALSE);

        SET_NEW_PNP_STATE(deviceInfo, Stopped);
        
        // No action required in this case. Just pass it down.
        Irp->IoStatus.Status = STATUS_SUCCESS;        
        goto passdown;
        break;
        
    case IRP_MN_CANCEL_REMOVE_DEVICE: 

        RESTORE_PREVIOUS_PNP_STATE(deviceInfo);
        //
        // No action required in this case. Just pass it down.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        goto passdown;
        break;
        
    case IRP_MN_CANCEL_STOP_DEVICE: 

        RESTORE_PREVIOUS_PNP_STATE(deviceInfo);
        //
        // No action required in this case. Just pass it down.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        goto passdown;
        break;
    default:
        //
        // Please see PnP documentation for use of these IRPs.
        //
        goto passdown;
        break;
    }
    
    IoReleaseRemoveLock(&deviceInfo->RemoveLock, Irp);       
    return status;

passdown:
   
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver(deviceInfo->NextLowerDriver, Irp);
    IoReleaseRemoveLock(&deviceInfo->RemoveLock, Irp);       
    return status;
        
}

NTSTATUS
GpdStartDevice (
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP             Irp
    )
/*++

Routine Description:
    
    Get the resources, map the resources if required
    and initialize the device.    

Arguments:
    
   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.
   
Return Value:

    NT status code
    

--*/
{
    NTSTATUS    status = STATUS_SUCCESS;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceTrans;
    PCM_PARTIAL_RESOURCE_LIST   partialResourceList;
    PCM_PARTIAL_RESOURCE_LIST   partialResourceListTranslated;
    PIO_STACK_LOCATION  stack;
    ULONG i;
    PLOCAL_DEVICE_INFO deviceInfo;

    deviceInfo = (PLOCAL_DEVICE_INFO) DeviceObject->DeviceExtension;

    stack = IoGetCurrentIrpStackLocation (Irp);

    PAGED_CODE();

    //
    // Do whatever initialization needed when starting the device: 
    // gather information about it,  update the registry, etc.
    //

    if ((NULL == stack->Parameters.StartDevice.AllocatedResources) &&
        (NULL == stack->Parameters.StartDevice.AllocatedResourcesTranslated)) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //
    // Parameters.StartDevice.AllocatedResources points to a 
    // CM_RESOURCE_LIST describing the hardware resources that 
    // the PnP Manager assigned to the device. This list contains 
    // the resources in raw form. Use the raw resources to program 
    // the device.
    //

    partialResourceList = 
    &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;

    resource = &partialResourceList->PartialDescriptors[0];
    
    //
    // Parameters.StartDevice.AllocatedResourcesTranslated points 
    // to a CM_RESOURCE_LIST describing the hardware resources that 
    // the PnP Manager assigned to the device. This list contains 
    // the resources in translated form. Use the translated resources 
    // to connect the interrupt vector, map I/O space, and map memory.
    //

    partialResourceListTranslated = 
    &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;

    resourceTrans = &partialResourceListTranslated->PartialDescriptors[0];

    for (i = 0;
            i < partialResourceList->Count; i++, resource++, resourceTrans++) {

        switch (resource->Type) {
        case CmResourceTypePort:

            switch (resourceTrans->Type) {

            case CmResourceTypePort:

                deviceInfo->PortWasMapped = FALSE;
                deviceInfo->PortBase = ULongToPtr(resourceTrans->u.Port.Start.LowPart);
                deviceInfo->PortCount       = resourceTrans->u.Port.Length;

                DebugPrint(("Resource Translated Port: (%x) Length: (%d)\n", 
                    resourceTrans->u.Port.Start.LowPart, 
                    resourceTrans->u.Port.Length));

                break;

            case CmResourceTypeMemory:

                //
                // We need to map the memory
                //

                deviceInfo->PortBase = (PVOID)
                    MmMapIoSpace (resourceTrans->u.Memory.Start,
                                  resourceTrans->u.Memory.Length,
                                  MmNonCached);

                deviceInfo->PortCount = resourceTrans->u.Memory.Length;
                deviceInfo->PortWasMapped = TRUE;

                DebugPrint(("Resource Translated Memory: (%x) Length: (%d)\n", 
                    resourceTrans->u.Memory.Start.LowPart, 
                    resourceTrans->u.Memory.Length));

                break;

            default:
                DebugPrint(("Unhandled resource_type (0x%x)\n", resourceTrans->Type));
                status = STATUS_UNSUCCESSFUL;
                ASSERTMSG("Unhandled resource_type in start request\n", FALSE);
            }             
            break;

        case CmResourceTypeMemory:

            deviceInfo->PortBase = (PVOID)
                MmMapIoSpace (resourceTrans->u.Memory.Start,
                              resourceTrans->u.Memory.Length,
                              MmNonCached);

            deviceInfo->PortCount = resourceTrans->u.Memory.Length;
            deviceInfo->PortWasMapped = TRUE;

            DebugPrint(("Resource Translated Memory: (%x) Length: (%d)\n", 
                resourceTrans->u.Memory.Start.LowPart, 
                resourceTrans->u.Memory.Length));

            break;

        case CmResourceTypeInterrupt:

        default:

            DebugPrint(("Unhandled resource type (0x%x)\n", resource->Type));
            status = STATUS_UNSUCCESSFUL;
            break;
    
        } // end of switch
    } // end of for

    return status;

}


NTSTATUS
GpdDispatchPower(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for power irps.
    Does nothing except forwarding the IRP to the next device
    in the stack.

Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    NT Status code
--*/
{
    PLOCAL_DEVICE_INFO   deviceInfo;
    
    deviceInfo = (PLOCAL_DEVICE_INFO) DeviceObject->DeviceExtension;

    PAGED_CODE();

    //
    // If the device has been removed, the driver should not pass 
    // the IRP down to the next lower driver.
    //
    
    if (deviceInfo->DevicePnPState == Deleted) {
        
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status =  STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return STATUS_DELETE_PENDING;
    }
    
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(deviceInfo->NextLowerDriver, Irp);
}

NTSTATUS 
GpdDispatchSystemControl(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp
    )
/*++

Routine Description:

    This routine is the dispatch routine for WMI irps.
    Does nothing except forwarding the IRP to the next device
    in the stack.
    
Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    NT Status code
--*/
{
    PLOCAL_DEVICE_INFO   deviceInfo;

    PAGED_CODE();

    deviceInfo = (PLOCAL_DEVICE_INFO) DeviceObject->DeviceExtension;

    //
    // If the device has been removed, the driver should not pass 
    // the IRP down to the next lower driver.
    //
    
    if (deviceInfo->DevicePnPState == Deleted) {
        
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status =  STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return STATUS_DELETE_PENDING;
    }

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(deviceInfo->NextLowerDriver, Irp);
}

    
VOID
GpdUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Free all the allocated resources, etc.

Arguments:

    DriverObject - pointer to a driver object.

Return Value:

    VOID.

--*/
{
    PAGED_CODE ();

    //
    // The device object(s) should be NULL now
    // (since we unload, all the devices objects associated with this
    // driver must have been deleted.
    //
    ASSERT(DriverObject->DeviceObject == NULL);
    
    DebugPrint (("unload\n"));

    return;
}

NTSTATUS
GpdDispatch(
    IN    PDEVICE_OBJECT pDO,
    IN    PIRP pIrp             
    )

/*++

Routine Description:
    This routine is the dispatch handler for the driver.  It is responsible
    for processing the IRPs.

Arguments:
    
    pDO - Pointer to device object.

    pIrp - Pointer to the current IRP.

Return Value:

    STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
    indicating the reason for failure.

--*/

{
    PLOCAL_DEVICE_INFO pLDI;
    PIO_STACK_LOCATION pIrpStack;
    NTSTATUS Status;

    PAGED_CODE();

    pIrp->IoStatus.Information = 0;
    pLDI = (PLOCAL_DEVICE_INFO)pDO->DeviceExtension;    // Get local info struct

    DebugPrint (("Entered GpdDispatch\n"));

    Status = IoAcquireRemoveLock (&pLDI->RemoveLock, pIrp);
    if (!NT_SUCCESS (Status)) { // may be device is being removed.
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = Status;
        IoCompleteRequest (pIrp, IO_NO_INCREMENT);
        return Status;
    }
   
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    // Dispatch based on major fcn code.

    switch (pIrpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        case IRP_MJ_CLOSE:
            // We don't need any special processing on open/close so we'll
            // just return success.
            Status = STATUS_SUCCESS;
            break;

        case IRP_MJ_DEVICE_CONTROL:
            //  Dispatch on IOCTL
            switch (pIrpStack->Parameters.DeviceIoControl.IoControlCode)
            {
            case IOCTL_GPD_READ_PORT_UCHAR:
            case IOCTL_GPD_READ_PORT_USHORT:
            case IOCTL_GPD_READ_PORT_ULONG:
                Status = GpdIoctlReadPort(
                            pLDI,
                            pIrp,
                            pIrpStack,
                            pIrpStack->Parameters.DeviceIoControl.IoControlCode
                            );
                break;

            case IOCTL_GPD_WRITE_PORT_UCHAR:
            case IOCTL_GPD_WRITE_PORT_USHORT:
            case IOCTL_GPD_WRITE_PORT_ULONG:
                Status = GpdIoctlWritePort(
                            pLDI, 
                            pIrp,
                            pIrpStack,
                            pIrpStack->Parameters.DeviceIoControl.IoControlCode
                            );
                break;
            default:      
                Status = STATUS_INVALID_PARAMETER;

            }
            break;
        default: 
            Status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    // We're done with I/O request.  Record the status of the I/O action.
    pIrp->IoStatus.Status = Status;

    // Don't boost priority when returning since this took little time.
    IoCompleteRequest(pIrp, IO_NO_INCREMENT );
    IoReleaseRemoveLock(&pLDI->RemoveLock, pIrp);       
    return Status;
}


NTSTATUS
GpdIoctlReadPort(
    IN PLOCAL_DEVICE_INFO pLDI,
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION IrpStack,
    IN ULONG IoctlCode  )


/*++

Routine Description:
    This routine processes the IOCTLs which read from the ports.

Arguments:
    
    pLDI        - our local device data
    pIrp        - IO request packet
    IrpStack    - The current stack location
    IoctlCode   - The ioctl code from the IRP

Return Value:
    STATUS_SUCCESS           -- OK

    STATUS_INVALID_PARAMETER -- The buffer sent to the driver
                                was too small to contain the
                                port, or the buffer which
                                would be sent back to the driver
                                was not a multiple of the data size.

    STATUS_ACCESS_VIOLATION  -- An illegal port number was given.

--*/

{
                                // NOTE:  Use METHOD_BUFFERED ioctls.
    PULONG pIOBuffer;           // Pointer to transfer buffer
                                //      (treated as an array of longs).
    ULONG InBufferSize;         // Amount of data avail. from caller.
    ULONG OutBufferSize;        // Max data that caller can accept.
    ULONG nPort;                // Port number to read
    ULONG DataBufferSize;

    PAGED_CODE();

    // Size of buffer containing data from application
    InBufferSize  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    // Size of buffer for data to be sent to application
    OutBufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    // NT copies inbuf here before entry and copies this to outbuf after
    // return, for METHOD_BUFFERED IOCTL's.
    pIOBuffer     = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

    // Check to ensure input buffer is big enough to hold a port number and
    // the output buffer is at least as big as the port data width.
    //
    switch (IoctlCode)
    {
    case IOCTL_GPD_READ_PORT_UCHAR:
        DataBufferSize = sizeof(UCHAR);
        break;
    case IOCTL_GPD_READ_PORT_USHORT:
        DataBufferSize = sizeof(USHORT);
        break;
    case IOCTL_GPD_READ_PORT_ULONG:
        DataBufferSize = sizeof(ULONG);
        break;
    default:      
        return STATUS_INVALID_PARAMETER;

    }

    if ( InBufferSize != sizeof(ULONG) || OutBufferSize < DataBufferSize )
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Buffers are big enough.

    nPort = *pIOBuffer;             // Get the I/O port number from the buffer.

    if (nPort >= pLDI->PortCount ||
        (nPort + DataBufferSize) > pLDI->PortCount ||
        (((ULONG_PTR)pLDI->PortBase + nPort) & (DataBufferSize - 1)) != 0)
    {
        return STATUS_ACCESS_VIOLATION;   // It was not legal.
    }
    
    if (pLDI->PortMemoryType == 1)
    {
        // Address is in I/O space
        
        switch (IoctlCode)
        {
        case IOCTL_GPD_READ_PORT_UCHAR:
            *(PUCHAR)pIOBuffer = READ_PORT_UCHAR(
                            (PUCHAR)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_USHORT:
            *(PUSHORT)pIOBuffer = READ_PORT_USHORT(
                            (PUSHORT)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_ULONG:
            *(PULONG)pIOBuffer = READ_PORT_ULONG(
                            (PULONG)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        default:      
            return STATUS_INVALID_PARAMETER;


        }
    } 
    else if (pLDI->PortMemoryType == 0)
    {
        // Address is in Memory space
        
        switch (IoctlCode)
        {
        case IOCTL_GPD_READ_PORT_UCHAR:
            *(PUCHAR)pIOBuffer = READ_REGISTER_UCHAR(
                            (PUCHAR)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_USHORT:
            *(PUSHORT)pIOBuffer = READ_REGISTER_USHORT(
                            (PUSHORT)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        case IOCTL_GPD_READ_PORT_ULONG:
            *(PULONG)pIOBuffer = READ_REGISTER_ULONG(
                            (PULONG)((ULONG_PTR)pLDI->PortBase + nPort) );
            break;
        default:      
            return STATUS_INVALID_PARAMETER;

        }
    } 
    else
    {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Indicate # of bytes read
    //
    
    pIrp->IoStatus.Information = DataBufferSize;

    return STATUS_SUCCESS;
}


NTSTATUS
GpdIoctlWritePort(
    IN PLOCAL_DEVICE_INFO pLDI, 
    IN PIRP pIrp, 
    IN PIO_STACK_LOCATION IrpStack,
    IN ULONG IoctlCode
    )

/*++

Routine Description:
    This routine processes the IOCTLs which write to the ports.

Arguments:
    
    pLDI        - our local device data
    pIrp        - IO request packet
    IrpStack    - The current stack location
    IoctlCode   - The ioctl code from the IRP

Return Value:
    STATUS_SUCCESS           -- OK

    STATUS_INVALID_PARAMETER -- The buffer sent to the driver
                                was too small to contain the
                                port, or the buffer which
                                would be sent back to the driver
                                was not a multiple of the data size.

    STATUS_ACCESS_VIOLATION  -- An illegal port number was given.

--*/

{
                                // NOTE:  Use METHOD_BUFFERED ioctls.
    PULONG pIOBuffer;           // Pointer to transfer buffer
                                //      (treated as array of longs).
    ULONG InBufferSize ;        // Amount of data avail. from caller.
    ULONG nPort;                // Port number to read or write.
    ULONG DataBufferSize;

    PAGED_CODE();

    // Size of buffer containing data from application
    InBufferSize  = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

    // NT copies inbuf here before entry and copies this to outbuf after return,
    // for METHOD_BUFFERED IOCTL's.
    pIOBuffer     = (PULONG) pIrp->AssociatedIrp.SystemBuffer;

    pIrp->IoStatus.Information = 0;
    
    // Check to ensure input buffer is big enough to hold a port number as well
    // as the data to write.
    //
    // The relative port # is a ULONG, and the data is the type appropriate to
    // the IOCTL.
    //

    switch (IoctlCode)
    {
    case IOCTL_GPD_WRITE_PORT_UCHAR:
        DataBufferSize = sizeof(UCHAR);
        break;
    case IOCTL_GPD_WRITE_PORT_USHORT:
        DataBufferSize = sizeof(USHORT);
        break;
    case IOCTL_GPD_WRITE_PORT_ULONG:
        DataBufferSize = sizeof(ULONG);
        break;
    default:      
        return STATUS_INVALID_PARAMETER;
    }

    if ( InBufferSize < (sizeof(ULONG) + DataBufferSize) )
    {
        return STATUS_INVALID_PARAMETER;
    }

    nPort = *pIOBuffer++;

    if (nPort >= pLDI->PortCount ||
        (nPort + DataBufferSize) > pLDI->PortCount ||
        (((ULONG_PTR)pLDI->PortBase + nPort) & (DataBufferSize - 1)) != 0)
    {
        return STATUS_ACCESS_VIOLATION;   // Illegal port number
    }

    if (pLDI->PortMemoryType == 1)
    {
        // Address is in I/O space
        
        switch (IoctlCode)
        {
        case IOCTL_GPD_WRITE_PORT_UCHAR:
            WRITE_PORT_UCHAR(
                (PUCHAR)((ULONG_PTR)pLDI->PortBase + nPort),
                *(PUCHAR)pIOBuffer );
            break;
        case IOCTL_GPD_WRITE_PORT_USHORT:
            WRITE_PORT_USHORT(
                (PUSHORT)((ULONG_PTR)pLDI->PortBase + nPort),
                *(PUSHORT)pIOBuffer );
            break;
        case IOCTL_GPD_WRITE_PORT_ULONG:
            WRITE_PORT_ULONG(
                (PULONG)((ULONG_PTR)pLDI->PortBase + nPort),
                *(PULONG)pIOBuffer );
            break;
        default:      
            return STATUS_INVALID_PARAMETER;
        }
    } 
    else if (pLDI->PortMemoryType == 0) 
    {
        // Address is in Memory space
        
        switch (IoctlCode)
        {
        case IOCTL_GPD_WRITE_PORT_UCHAR:
            WRITE_REGISTER_UCHAR( 
                    (PUCHAR)((ULONG_PTR)pLDI->PortBase + nPort),
                    *(PUCHAR)pIOBuffer );
            break;
        case IOCTL_GPD_WRITE_PORT_USHORT:
            WRITE_REGISTER_USHORT(
                    (PUSHORT)((ULONG_PTR)pLDI->PortBase + nPort),
                    *(PUSHORT)pIOBuffer );
            break;
        case IOCTL_GPD_WRITE_PORT_ULONG:
            WRITE_REGISTER_ULONG(
                    (PULONG)((ULONG_PTR)pLDI->PortBase + nPort),
                    *(PULONG)pIOBuffer );
            break;
        default:      
            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Indicate # of bytes written
    //
    
    pIrp->IoStatus.Information = DataBufferSize;

    return STATUS_SUCCESS;
}


#if DBG
PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE";
        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE";
        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE";
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE";
        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE";
        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE";
        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE";
        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS";
        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE";
        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES";
        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES";
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";
        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT";
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";
        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG";
        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG";
        case IRP_MN_EJECT:
            return "IRP_MN_EJECT";
        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK";
        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID";
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE";
        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION";
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION";
        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL";
            
        default:
            return "IRP_MN_?????";
    }
}

#endif

