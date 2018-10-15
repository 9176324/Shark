/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    VHIDMINI.C
    
Abstract:


Author:

    Kumar Rajeev & Eliyas Yakub  (1/04/2002) 


Environment:

    kernel mode only

Notes:


Revision History:


--*/

#include <VHidMini.h>

#ifdef ALLOC_PRAGMA
    #pragma alloc_text( INIT, DriverEntry )
    #pragma alloc_text( PAGE, AddDevice)
    #pragma alloc_text( PAGE, Unload)
    #pragma alloc_text( PAGE, PnP)
    #pragma alloc_text( PAGE, ReadDescriptorFromRegistry)

#endif 


NTSTATUS 
DriverEntry (
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

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    NTSTATUS                      ntStatus;
    HID_MINIDRIVER_REGISTRATION   hidMinidriverRegistration;

    DebugPrint(("Enter DriverEntry()\n"));

    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                InternalIoctl;
    DriverObject->MajorFunction[IRP_MJ_PNP]   = PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = Power;
    DriverObject->DriverUnload                = Unload;
    DriverObject->DriverExtension->AddDevice  = AddDevice;

    RtlZeroMemory(&hidMinidriverRegistration, 
                  sizeof(hidMinidriverRegistration));

    //
    // Revision must be set to HID_REVISION by the minidriver
    //
    
    hidMinidriverRegistration.Revision            = HID_REVISION;
    hidMinidriverRegistration.DriverObject        = DriverObject;
    hidMinidriverRegistration.RegistryPath        = RegistryPath;
    hidMinidriverRegistration.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);

    //
    // if "DevicesArePolled" is False then the hidclass driver does not do
    // polling and instead reuses a few Irps (ping-pong) if the device has 
    // an Input item. Otherwise, it will do polling at regular interval. USB 
    // HID devices do not need polling by the HID classs driver. Some leagcy 
    // devices may need polling. 
    //
    
    hidMinidriverRegistration.DevicesArePolled    = FALSE; 

    //
    //Register with hidclass
    //

    ntStatus = HidRegisterMinidriver(&hidMinidriverRegistration);

    if(!NT_SUCCESS(ntStatus) ){
        DebugPrint(("HidRegisterMinidriver FAILED, returnCode=%x\n", 
                  ntStatus));

    }
    
    DebugPrint(("Exit DriverEntry() status=0x%x\n", ntStatus));

    return ntStatus;
    
}


NTSTATUS  
AddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    )
/*++

Routine Description:

    HidClass Driver calls our AddDevice routine after creating an FDO for us.
    We do not need to create a device object or attach it to the PDO. 
    Hidclass driver will do it for us.
   
Arguments:

    DriverObject - pointer to the driver object.

    FunctionalDeviceObject -  pointer to the FDO created by the
                            Hidclass driver for us.

Return Value:

    NT status code.

--*/
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceInfo;

    PAGED_CODE ();

    DebugPrint(("Enter AddDevice(DriverObject=0x%x,"
                "FunctionalDeviceObject=0x%x)\n",
                DriverObject, FunctionalDeviceObject));

    ASSERTMSG("AddDevice:", FunctionalDeviceObject != NULL);

    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION (FunctionalDeviceObject);

    //
    // Initialize all the members of device extension
    //

    RtlZeroMemory((PVOID)deviceInfo, sizeof(DEVICE_EXTENSION));

    //
    // Set the initial state of the FDO
    //

    INITIALIZE_PNP_STATE(deviceInfo);

    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    
    DebugPrint(("Exit AddDevice\n", ntStatus));

    return ntStatus;
    
}


NTSTATUS  
PnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
Routine Description:

    Handles PnP Irps sent to FDO .

Arguments:

    DeviceObject - Pointer to deviceobject
    Irp          - Pointer to a PnP Irp.
    
Return Value:

    NT Status is returned.
--*/
{
    NTSTATUS            ntStatus, queryStatus;
    PDEVICE_EXTENSION   deviceInfo;
    KEVENT              startEvent;
    PIO_STACK_LOCATION  IrpStack, previousSp;
    PWCHAR              buffer;

    PAGED_CODE();

    //
    //Get a pointer to the device extension
    //

    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    //
    //Get a pointer to the current location in the Irp
    //

    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    DebugPrint(("%s Irp=0x%x\n", 
               PnPMinorFunctionString(IrpStack->MinorFunction), 
               Irp ));

    switch(IrpStack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:

        KeInitializeEvent(&startEvent, NotificationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp, 
                                PnPComplete, 
                                &startEvent, 
                                TRUE, 
                                TRUE, 
                                TRUE);

        ntStatus = IoCallDriver (GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);

        if (STATUS_PENDING == ntStatus) {
            KeWaitForSingleObject(
                    		&startEvent,
                    		Executive, 
                    		KernelMode, 
                    		FALSE, // No allert
                    		NULL); // No timeout
            ntStatus = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(ntStatus)) {
            //
            // Use default "HID Descriptor" (hardcoded). We will set the 
            // wReportLength memeber of HID descriptor when we read the 
            // the report descriptor either from registry or the hard-coded 
            // one.
            //

            deviceInfo->HidDescriptor = DefaultHidDescriptor;

            //
            // Check to see if we need to read the Report Descriptor from
            // registry. If the "ReadFromRegistry" flag in the registry is set
            // then we will read the descriptor from registry using routine 
            // ReadDescriptorFromRegistry(). Otherwise, we will use the 
            // hard-coded default report descriptor.
            //

            queryStatus = CheckRegistryForDescriptor(DeviceObject);

            if(NT_SUCCESS(queryStatus)){
                //
                // We need to read read descriptor from registry
                //
                
                queryStatus = ReadDescriptorFromRegistry(DeviceObject);
            
                if(!NT_SUCCESS(queryStatus)){
                    DebugPrint(("Failed to read descriptor from registry\n"));
                    ntStatus = STATUS_UNSUCCESSFUL;
                }

            }
            else{
                //
                // We will use hard-coded report descriptor. 
                //
                
                deviceInfo->ReportDescriptor = DefaultReportDescriptor;

                DebugPrint(("Using Hard-coded Report descriptor\n"));
                
            }
            
            //
            //Set new PnP state
            //
            if(NT_SUCCESS(ntStatus)){
                
                SET_NEW_PNP_STATE(deviceInfo, Started);
                
            }
        }

        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return ntStatus;

    case IRP_MN_STOP_DEVICE:
        //
        // Mark the device as stopped.
        //
        SET_NEW_PNP_STATE(deviceInfo, Stopped);
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        RESTORE_PREVIOUS_PNP_STATE(deviceInfo);
        ntStatus = STATUS_SUCCESS;         
        break;

    case IRP_MN_QUERY_STOP_DEVICE:    

        SET_NEW_PNP_STATE(deviceInfo, StopPending);
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SET_NEW_PNP_STATE(deviceInfo, RemovePending);
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        RESTORE_PREVIOUS_PNP_STATE(deviceInfo);
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_SURPRISE_REMOVAL:

        SET_NEW_PNP_STATE(deviceInfo, SurpriseRemovePending);
        ntStatus = STATUS_SUCCESS;
        break;

    case IRP_MN_REMOVE_DEVICE:
        //
        // free memory if allocated for report descriptor
        //
        if(deviceInfo->ReadReportDescFromRegistry)
            ExFreePool(deviceInfo->ReportDescriptor);
        
        SET_NEW_PNP_STATE(deviceInfo, Deleted);
        ntStatus = STATUS_SUCCESS;           
        break;
        
    case IRP_MN_QUERY_ID:

        //
        // This check is required to filter out QUERY_IDs forwarded
        // by the HIDCLASS for the parent FDO. These IDs are sent
        // by PNP manager for the parent FDO if you root-enumerate this driver.
        //
        previousSp = ((PIO_STACK_LOCATION) ((UCHAR *) (IrpStack) + 
                            sizeof(IO_STACK_LOCATION)));
        
        if(previousSp->DeviceObject == DeviceObject) {
            //
            // Filtering out this basically prevents the Found New Hardware 
            // popup for the root-enumerated VHIDMINI on reboot.
            // 
            ntStatus = Irp->IoStatus.Status;            
            break;
        }
                
        switch (IrpStack->Parameters.QueryId.IdType) 
        {
        case BusQueryDeviceID:
        case BusQueryHardwareIDs:
            //
            // HIDClass is asking for child deviceid & hardwareids.
            // Let us just make up some  id for our child device.
            //
            buffer = (PWCHAR)ExAllocatePoolWithTag(
                          NonPagedPool, 
                          VHID_HARDWARE_IDS_LENGTH, 
                          VHID_POOL_TAG
                          );

            if (buffer) {
                //
                // Do the copy, store the buffer in the Irp
                //
                RtlCopyMemory(buffer, 
                              VHID_HARDWARE_IDS, 
                              VHID_HARDWARE_IDS_LENGTH
                              );
                
                Irp->IoStatus.Information = (ULONG_PTR)buffer;
                ntStatus = STATUS_SUCCESS;
            } 
            else {
                //
                //  No memory
                //
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }

            Irp->IoStatus.Status = ntStatus;
            //
            // We don't need to forward this to our bus. This query 
            // is for our child so we should complete it right here. 
            // fallthru.
            //
            IoCompleteRequest (Irp, IO_NO_INCREMENT);     
            
            return ntStatus;           
                 
        default:            
            ntStatus = Irp->IoStatus.Status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);          
            return ntStatus;
        }

    default:         
        ntStatus = Irp->IoStatus.Status;
        break;
    }
    
    Irp->IoStatus.Status = ntStatus;
    IoSkipCurrentIrpStackLocation (Irp);
    return IoCallDriver(GET_NEXT_DEVICE_OBJECT(DeviceObject), Irp);
    
}


NTSTATUS 
PnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    DebugPrint(("PnPComplete(DeviceObject=0x%x,Irp=0x%x,Context=0x%x)\n",
                DeviceObject, Irp, Context));

    UNREFERENCED_PARAMETER (DeviceObject);

    if (Irp->PendingReturned) 
    {
        KeSetEvent ((PKEVENT) Context, 0, FALSE);
    }
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
Power(
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
    PDEVICE_EXTENSION   deviceInfo;

    DebugPrint(("Enter Power()\n"));
    
    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION (DeviceObject);

    //
    // Make sure to store and restore the device context depending
    // on whether the system is going into lower power state
    // or resuming. The job of converting S-IRPs to D-IRPs is done
    // by HIDCLASS. All you need to do here is handle Set-Power request
    // for device power state according to the guidelines given in the 
    // power management documentation of the DDK. Before powering down  
    // your device make sure to cancel any pending IRPs, if any, sent by 
    // you to the lower device stack.
    //

    PoStartNextPowerIrp(Irp);     
    IoSkipCurrentIrpStackLocation(Irp);

    return PoCallDriver(GET_NEXT_DEVICE_OBJECT (DeviceObject), Irp);
}


VOID
Unload(
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
    DebugPrint(("Enter Unload()\n"));

    ASSERT(DriverObject->DeviceObject == NULL);

    return;
}


NTSTATUS 
InternalIoctl
    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Handles all the internal ioctls

Arguments:

    DeviceObject - Pointer to the device object.

    Irp - Pointer to the request packet.

Return Value:

    NT Status code

--*/
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpStack;

    DebugPrint(("Enter InternalIoctl (DO=0x%x,Irp=0x%x)\n",
                DeviceObject, Irp));

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    switch(IrpStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
        //
        // Retrieves the device's HID descriptor. 
        //
        DebugPrint(("IOCTL_HID_GET_DEVICE_DESCRIPTOR\n"));
        ntStatus = GetHidDescriptor(DeviceObject, Irp);
        break;
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        //
        //Obtains the report descriptor for the HID device.
        //
        DebugPrint(("IOCTL_HID_GET_REPORT_DESCRIPTOR\n"));
        ntStatus = GetReportDescriptor(DeviceObject, Irp);
        break;
    case IOCTL_HID_READ_REPORT:
        //
        //Return a report from the device into a class driver-supplied buffer.
        //
        DebugPrint(("IOCTL_HID_READ_REPORT\n"));
        ntStatus = ReadReport(DeviceObject, Irp);
        return ntStatus;
        
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        //
        //Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
        //
        DebugPrint(("IOCTL_HID_GET_DEVICE_ATTRIBUTES\n"));
        ntStatus = GetDeviceAttributes(DeviceObject, Irp);
        break;
    case IOCTL_HID_WRITE_REPORT:
        //
        //Transmits a class driver-supplied report to the device.
        //
        DebugPrint(("IOCTL_HID_WRITE_REPORT\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    case IOCTL_HID_SET_FEATURE:
        //
        // This sends a HID class feature report to a top-level collection of 
        // a HID class device.
        //
        DebugPrint(("IOCTL_HID_SET_FEATURE\n"));
        ntStatus = GetSetFeature(DeviceObject, Irp);                
        break;
    case IOCTL_HID_GET_FEATURE:
        //
        // returns a feature report associated with a top-level collection
        //
        DebugPrint(("IOCTL_HID_GET_FEATURE\n"));
        ntStatus = GetSetFeature(DeviceObject, Irp);
        break;
/* Following two ioctls are not defined in the Win2K HIDCLASS.H headerfile        
    case IOCTL_HID_GET_INPUT_REPORT:
        //
        // returns a HID class input report associated with a top-level 
        // collection of a HID class device.
        //
        DebugPrint(("IOCTL_HID_GET_INPUT_REPORT\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    case IOCTL_HID_SET_OUTPUT_REPORT:
        //
        // sends a HID class output report to a top-level collection of a HID
        // class device.
        //
        DebugPrint(("IOCTL_HID_SET_OUTPUT_REPORT\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
*/        
    case IOCTL_HID_GET_STRING:
        //
        //Requests that the HID minidriver retrieve a human-readable string 
        //for either the manufacturer ID, the product ID, or the serial number
        //from the string descriptor of the device. The minidriver must send 
        //a Get String Descriptor request to the device, in order to retrieve 
        //the string descriptor, then it must extract the string at the 
        //appropriate index from the string descriptor and return it in the 
        //output buffer indicated by the IRP. Before sending the Get String 
        //Descriptor request, the minidriver must retrieve the appropriate 
        //index for the manufacturer ID, the product ID or the serial number
        //from the device extension of a top level collection associated with
        //the device. 
        //
        DebugPrint(("IOCTL_HID_GET_STRING\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    case IOCTL_HID_ACTIVATE_DEVICE:
        //
        //Makes the device ready for I/O operations.
        //
        DebugPrint(("IOCTL_HID_ACTIVATE_DEVICE\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    case IOCTL_HID_DEACTIVATE_DEVICE:
        //
        //Causes the device to cease operations and terminate all outstanding
        //I/O requests.
        //
        DebugPrint(("IOCTL_HID_DEACTIVATE_DEVICE\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    //case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
    //    DebugPrint(("IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST\n"));
    //    ntStatus = STATUS_NOT_SUPPORTED;
    //    break;
    default:
        DebugPrint(("Unknown or unsupported IOCTL (%x)\n",
        IrpStack->Parameters.DeviceIoControl.IoControlCode));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    }
    //
    //Set real return status in Irp
    //
    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    ntStatus = STATUS_SUCCESS;

    return ntStatus;
} 


NTSTATUS
GetSetFeature(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    )
/*++

Routine Description:

    Handles Ioctls for Get and Set feature for all the collection. 
    For control collection (custom defined collection) it handles 
    the user-defined control codes for sideband communication 
    
Arguments:

    DeviceObject - pointer to a device object.

    Irp - Pointer to Interrupt Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION  IrpStack;
    PHID_XFER_PACKET    transferPacket = NULL;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    transferPacket = (PHID_XFER_PACKET)Irp->UserBuffer;

    if(!transferPacket->reportBufferLen){
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        return ntStatus;
    }

    switch(IrpStack->Parameters.DeviceIoControl.IoControlCode) 
    {
    case IOCTL_HID_GET_FEATURE:
        //
        // IOCTL_HID_GET_FEATURE is being used to send user-defined 
        // HIDMINI_CONTROL_CODE_Xxxx request to a custom collection
        // defined especially for this purpose
        // 

        if(CONTROL_COLLECTION_REPORT_ID == transferPacket->reportId){
            //
            //This is a special HIDMINI_CONTROL_CODE_Xxxx Type request
            //

            ntStatus = HandleControlRequests(transferPacket);
            break;
        }
        //
        // If collection ID is not for control collection then handle
        // this request just as you would for a regular collection.
        //
        // fall thru
        //
    case IOCTL_HID_SET_FEATURE:
        //
        // not handled, fall thru
        //
    default:
    
        ntStatus = STATUS_NOT_SUPPORTED;
        break;            
    }

    return ntStatus;
    
}


NTSTATUS
HandleControlRequests(
    PHID_XFER_PACKET TransferPacket
    )
/*++

Routine Description:

    Handles HIDMINI_CONTROLCODE_Xxxx type requests

Arguments:

    TransferPacket - pointer to HID_XFER_PACKET.

Return Value:

    NT status code.

--*/

{
    NTSTATUS                   ntStatus = STATUS_SUCCESS;
    PHIDMINI_CONTROL_INFO      controlInfo = NULL;
    PHID_DEVICE_ATTRIBUTES     deviceAttributes = NULL;

    if(TransferPacket->reportBufferLen < sizeof(HIDMINI_CONTROL_INFO))
    {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        return ntStatus;
    }

    controlInfo = (PHIDMINI_CONTROL_INFO)TransferPacket->reportBuffer;

    switch(controlInfo->ControlCode)
    {
    case HIDMINI_CONTROL_CODE_GET_ATTRIBUTES:

        DebugPrint(("Control Code HIDMINI_CONTROL_CODE_GET_ATTRIBUTES\n"));

        if(TransferPacket->reportBufferLen 
                         < (sizeof(HID_DEVICE_ATTRIBUTES) 
                         + sizeof(HIDMINI_CONTROL_INFO))) 
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            return ntStatus;
        }

        deviceAttributes = (PHID_DEVICE_ATTRIBUTES) controlInfo->ControlBuffer;

        deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
        deviceAttributes->VendorID = HIDMINI_VID;
        deviceAttributes->ProductID = HIDMINI_PID;
        deviceAttributes->VersionNumber = HIDMINI_VERSION;

        break;

    case HIDMINI_CONTROL_CODE_DUMMY1:

        DebugPrint(("Control Code HIDMINI_CONTROL_CODE_DUMMY1\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
     
    case HIDMINI_CONTROL_CODE_DUMMY2:
        
        DebugPrint(("Control Code HIDMINI_CONTROL_CODE_DUMMY2\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
        
    default:

        DebugPrint(("Unknown control Code\n"));
        ntStatus = STATUS_NOT_SUPPORTED;
        break;
    }

    return ntStatus;         
}


NTSTATUS 
GetHidDescriptor(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    )
/*++

Routine Description:

    Finds the HID descriptor and copies it into the buffer provided by the Irp.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - Pointer to Interrupt Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceInfo;
    PIO_STACK_LOCATION  IrpStack;
    ULONG               bytesToCopy;

    DebugPrint(("GetHIDDescriptor Entry\n"));

    //
    // Get a pointer to the current location in the Irp
    //

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get a pointer to the device extension
    //

    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    //
    // Copy device descriptor to HIDCLASS buffer
    //

    DebugPrint(("HIDCLASS Buffer = 0x%x, Buffer length = 0x%x\n", 
                Irp->UserBuffer, 
                IrpStack->Parameters.DeviceIoControl.OutputBufferLength));

    //
    // Copy MIN (OutputBufferLength, DeviceExtension->HidDescriptor->bLength)
    //

    bytesToCopy = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if(!bytesToCopy) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    //
    // Since HidDescriptor.bLength could be >= sizeof(HID_DESCRIPTOR) we 
    // just check for HidDescriptor.bLength and 
    // copy MIN (OutputBufferLength, DeviceExtension->HidDescriptor->bLength)
    //
    
    if (bytesToCopy > deviceInfo->HidDescriptor.bLength) {
        bytesToCopy = deviceInfo->HidDescriptor.bLength;
    }

    DebugPrint(("Copying %d bytes to HIDCLASS buffer\n", 
                bytesToCopy));

    RtlCopyMemory((PUCHAR) Irp->UserBuffer, 
                (PUCHAR) &deviceInfo->HidDescriptor, 
                bytesToCopy);                

    DebugPrint(("   bLength: 0x%x \n"
                "   bDescriptorType: 0x%x \n"
                "   bcdHID: 0x%x \n"
                "   bCountry: 0x%x \n"
                "   bNumDescriptors: 0x%x \n"
                "   bReportType: 0x%x \n"
                "   wReportLength: 0x%x \n",
                deviceInfo->HidDescriptor.bLength, 
                deviceInfo->HidDescriptor.bDescriptorType,
                deviceInfo->HidDescriptor.bcdHID,
                deviceInfo->HidDescriptor.bCountry,
                deviceInfo->HidDescriptor.bNumDescriptors,
                deviceInfo->HidDescriptor.DescriptorList[0].bReportType,
                deviceInfo->HidDescriptor.DescriptorList[0].wReportLength
                ));

    //
    // Report how many bytes were copied
    //
    Irp->IoStatus.Information = bytesToCopy;

    DebugPrint(("HidMiniGetHIDDescriptor Exit = 0x%x\n", ntStatus));

    return ntStatus;
}

NTSTATUS 
ReadReport(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    )
/*++

Routine Description:

    Creates reports and sends it back to the requester.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - Pointer to Interrupt Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS ntStatus = STATUS_PENDING;
    PDEVICE_EXTENSION   deviceInfo;
    PIO_STACK_LOCATION  IrpStack;
    LARGE_INTEGER timeout;
    PREAD_TIMER    readTimerStruct;

    DebugPrint(("ReadReport Entry\n"));
    //
    // Get a pointer to the device extension
    //
    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    //
    // Allocate the Timer structure
    //
    readTimerStruct = ExAllocatePoolWithTag(NonPagedPool, 
                                            sizeof(READ_TIMER), 
                                            VHID_POOL_TAG
                                            );

    if(!readTimerStruct){
        DebugPrint(("Mem allocation for readTimerStruct failed\n"));
        Irp->IoStatus.Status = ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    else{

        RtlZeroMemory(readTimerStruct, sizeof(READ_TIMER));
        //
        // Since the IRP will be completed later in the DPC,
        // mark the IRP pending and return STATUS_PENDING.
        //
        IoMarkIrpPending(Irp);
        
        //
        //remember the Irp
        //
        readTimerStruct->Irp = Irp;

        //
        // Initialize the DPC structure and Timer
        //
        
        KeInitializeDpc(&readTimerStruct->ReadTimerDpc,
                        ReadTimerDpcRoutine,
                        (PVOID)readTimerStruct
                        );

        KeInitializeTimer(&readTimerStruct->ReadTimer);

        //
        // Queue the timer DPC  
        //
        timeout.HighPart = -1;
        timeout.LowPart = -(LONG)(10*1000*5000);  //in 100 ns.total 5 sec
        KeSetTimer(&readTimerStruct->ReadTimer,
                   timeout,
                   &readTimerStruct->ReadTimerDpc
                   );
    }
    DebugPrint(("ReadReport Exit = 0x%x\n", ntStatus));

    return ntStatus;
}


VOID 
ReadTimerDpcRoutine(   
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    NTSTATUS              ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION     deviceInfo;
    PIRP                  Irp;
    PIO_STACK_LOCATION    IrpStack;
    PREAD_TIMER           readTimerStruct;
    ULONG                 bytesToCopy = INPUT_REPORT_BYTES + 1;
    UCHAR                 readReport[INPUT_REPORT_BYTES + 1];


    DebugPrint(("ReadTimerDpcRoutine Entry\n"));

    //
    // Get the Irp from context 
    //
    readTimerStruct = (PREAD_TIMER)DeferredContext;
    Irp = readTimerStruct->Irp;

    //
    // Get a pointer to the current location in the Irp
    //
    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    //First check the size of the output buffer (there is no input buffer)
    //

    if( IrpStack->Parameters.DeviceIoControl.OutputBufferLength < bytesToCopy)
    {
        DebugPrint(("ReadReport: Buffer too small, output=0x%x need=0x%x\n", 
                    IrpStack->Parameters.DeviceIoControl.OutputBufferLength, 
                    bytesToCopy
                    ));

        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        //
        //Create input report
        //
        readReport[0] = CONTROL_FEATURE_REPORT_ID;
        readReport[1] = 'K';

        //
        // Copy input report to the Irp buffer
        //

        RtlCopyMemory(Irp->UserBuffer, readReport, bytesToCopy);
        
        //
        // Report how many bytes were copied
        //

        Irp->IoStatus.Information = bytesToCopy;
    }

    //
    //Set real return status in Irp
    //

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    //
    //Free the DPC structure
    //

    ExFreePool(readTimerStruct);
    
    DebugPrint(("ReadTimerDpcRoutine Exit = 0x%x\n", ntStatus));

}


NTSTATUS 
GetReportDescriptor(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    )
/*++

Routine Description:

    Finds the Report descriptor and copies it into the buffer provided by the 
    Irp.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - Pointer to Interrupt Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION   deviceInfo;
    PIO_STACK_LOCATION  IrpStack;
    ULONG               bytesToCopy;

    DebugPrint(("HidMiniGetReportDescriptor Entry\n"));

    //
    // Get a pointer to the current location in the Irp
    //

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get a pointer to the device extension
    //

    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    //
    // Copy device descriptor to HIDCLASS buffer
    //

    DebugPrint(("HIDCLASS Buffer = 0x%x, Buffer length = 0x%x\n", 
                Irp->UserBuffer, 
                IrpStack->Parameters.DeviceIoControl.OutputBufferLength));


    bytesToCopy = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if(!bytesToCopy) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    if (bytesToCopy > 
             deviceInfo->HidDescriptor.DescriptorList[0].wReportLength) {
        bytesToCopy = 
             deviceInfo->HidDescriptor.DescriptorList[0].wReportLength;
    }

    DebugPrint(("Copying 0x%x bytes to HIDCLASS buffer\n", bytesToCopy));

    RtlCopyMemory((PUCHAR) Irp->UserBuffer,
                (PUCHAR) deviceInfo->ReportDescriptor, 
                bytesToCopy
                );

    //
    // Report how many bytes were copied
    //
    Irp->IoStatus.Information = bytesToCopy;

    DebugPrint(("HidMiniGetReportDescriptor Exit = 0x%x\n", ntStatus));

    return ntStatus;
    
}


NTSTATUS 
GetDeviceAttributes(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp)
/*++

Routine Description:

    Fill in the given struct _HID_DEVICE_ATTRIBUTES

Arguments:

    DeviceObject - pointer to a device object.

    Irp - Pointer to Interrupt Request Packet.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                 ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION       irpStack;
    PHID_DEVICE_ATTRIBUTES   deviceAttributes;

    DebugPrint(("HidMiniGetDeviceAttributes Entry\n"));

    //
    // Get a pointer to the current location in the Irp
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get a pointer to the device extension
    //

    deviceAttributes = (PHID_DEVICE_ATTRIBUTES) Irp->UserBuffer;

    if(sizeof (HID_DEVICE_ATTRIBUTES) >
        irpStack->Parameters.DeviceIoControl.OutputBufferLength) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    //
    // Report how many bytes were copied
    //
    Irp->IoStatus.Information = sizeof (HID_DEVICE_ATTRIBUTES);

    deviceAttributes->Size = sizeof (HID_DEVICE_ATTRIBUTES);
    deviceAttributes->VendorID = HIDMINI_VID;
    deviceAttributes->ProductID = HIDMINI_PID;
    deviceAttributes->VersionNumber = HIDMINI_VERSION;

    DebugPrint(("HidMiniGetAttributes Exit = 0x%x\n", ntStatus));

    return ntStatus;
}


NTSTATUS
CheckRegistryForDescriptor(
        IN PDEVICE_OBJECT  DeviceObject
        )
/*++

Routine Description:

    Read "ReadFromRegistry" key value from device parameters in the registry. 

Arguments:

    DeviceObject - pointer to a device object.

Return Value:

    NT status code.

--*/

{

    NTSTATUS           ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT     pdo;
    HANDLE             handle;
    ULONG              length = 0, resultLength = 0;
    PUCHAR             ReadFromRegistryKeyValue = NULL;
    UNICODE_STRING     ReadFromRegistryKeyValueName;
    ULONG              ReadFromRegistry;

    //
    // Get physical device object
    //

    pdo = GET_PHYSICAL_DEVICE_OBJECT(DeviceObject);

    //
    // Get a handle to the Device Parameters registry key
    //

    ntStatus = IoOpenDeviceRegistryKey(
                         pdo,   
                         PLUGPLAY_REGKEY_DEVICE,   
                         KEY_QUERY_VALUE,    
                         &handle          
                         );
    
    if(!NT_SUCCESS(ntStatus)){   
        
       DebugPrint(("IoOpenDeviceRegistryKey failed status:0x%x\n", ntStatus));
       return ntStatus;

    }

    //
    //Query the "ReadFromRegistry" key value in the parameters key
    //

    RtlInitUnicodeString(&ReadFromRegistryKeyValueName,
                         L"ReadFromRegistry");
    
    ntStatus= ZwQueryValueKey(
                handle,     
                &ReadFromRegistryKeyValueName, 
                KeyValuePartialInformation , 
                (PVOID)ReadFromRegistryKeyValue, 
                length,   
                &resultLength    
                );

    if(STATUS_BUFFER_TOO_SMALL == ntStatus)
    {
        //
        // Query the registry again using resultlength as buffer length
        //
        
        length = resultLength;            
        ReadFromRegistryKeyValue = ExAllocatePoolWithTag(
                                                    PagedPool,
                                                    length,
                                                    VHID_POOL_TAG
                                                    );

        if(!ReadFromRegistryKeyValue){
            
            DebugPrint(("Allocating to keyValueInformation failed\n"));
            ntStatus = STATUS_UNSUCCESSFUL;
            goto query_end;
            
        }
      
        ntStatus= ZwQueryValueKey(
                handle,    
                &ReadFromRegistryKeyValueName ,
                KeyValuePartialInformation , 
                (PVOID)ReadFromRegistryKeyValue, 
                length,  
                &resultLength   
                );

        if(!NT_SUCCESS(ntStatus)){
            
           DebugPrint(("ZwQueryValueKey for ReadFromRegistry"
                        "failed status:0x%x\n", ntStatus));
           
           goto query_end;
           
        }
        
    }
    else {
        
        if(!NT_SUCCESS(ntStatus)){
            
           DebugPrint(("ZwQueryValueKey for ReadFromRegistry failed status:0x%x\n", ntStatus));
           goto query_end;
           
        }
        
    }

    //
    // if ReadFromRegsitry is set to 0 then we do not need to read 
    // the descriptor from regsistry. Just return failure.
    //
    
    ReadFromRegistry = ((PKEY_VALUE_PARTIAL_INFORMATION) 
                       ReadFromRegistryKeyValue)->Data[0] ;

    if(!ReadFromRegistry) {
        
        ntStatus = STATUS_UNSUCCESSFUL;
        goto query_end;
        
    }

query_end:

    ZwClose(handle);
    
    if(ReadFromRegistryKeyValue)
        ExFreePool(ReadFromRegistryKeyValue);

    return ntStatus;

}


NTSTATUS
ReadDescriptorFromRegistry(
        IN PDEVICE_OBJECT  DeviceObject
        )
/*++

Routine Description:

    Read HID report descriptor from registry
    
Arguments:

    DeviceObject - pointer to a device object.

Return Value:

    NT status code.

--*/
{
    NTSTATUS             ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT       pdo;
    PDEVICE_EXTENSION    deviceInfo;
    HANDLE               handle;
    ULONG                length = 0;
    ULONG                resultLength = 0;
    ULONG                RegistryReportDescriptorLength = 0;
    PUCHAR               RegistryReportDescriptor= NULL;
    PUCHAR               keyValueInformation = NULL;
    UNICODE_STRING       valueName;

    //
    // Get physical device object
    //

    pdo = GET_PHYSICAL_DEVICE_OBJECT(DeviceObject);

    //
    // Get the hid mini driver device extension
    //

    deviceInfo = GET_MINIDRIVER_DEVICE_EXTENSION(DeviceObject);

    //
    // Get a handle to the Device registry key
    //

    ntStatus = IoOpenDeviceRegistryKey(pdo,   
                                       PLUGPLAY_REGKEY_DEVICE,   
                                       KEY_QUERY_VALUE,    
                                       &handle          
                                       );

    if(!NT_SUCCESS(ntStatus)){   
        
       DebugPrint(("IoOpenDeviceRegistryKey failed status:0x%x\n", ntStatus));
       return ntStatus;

    }
    
    //
    //Query the Descriptor value in the parameters key
    //
    RtlInitUnicodeString(&valueName, L"MyReportDescriptor");
    
    ntStatus= ZwQueryValueKey(
                handle,     
                &valueName,
                KeyValuePartialInformation, 
                (PVOID)keyValueInformation, 
                length,   
                &resultLength    
                );

    if(STATUS_BUFFER_TOO_SMALL == ntStatus){
        length = resultLength;            
        keyValueInformation = ExAllocatePoolWithTag(
                                    PagedPool,
                                    length,
                                    VHID_POOL_TAG
                                    );

        if(!keyValueInformation){
            
            DebugPrint(("Allocating to keyValueInformation failed\n"));
            ntStatus = STATUS_UNSUCCESSFUL;
            goto query_end;
            
        }
        ntStatus= ZwQueryValueKey(
                handle,     
                &valueName , 
                KeyValuePartialInformation , 
                (PVOID)keyValueInformation, 
                length,   
                &resultLength    
                );

        if(!NT_SUCCESS(ntStatus)){
            
            DebugPrint(("ZwQueryValueKey failed status:0x%x\n", ntStatus));                    
            goto query_end;
            
        }
        //
        // Allocate memory for Report Descriptor   
        //
        RegistryReportDescriptorLength = 
                  ((PKEY_VALUE_PARTIAL_INFORMATION)
                  keyValueInformation)->DataLength;

        RegistryReportDescriptor = 
                  ExAllocatePoolWithTag(NonPagedPool, 
                                        RegistryReportDescriptorLength,
                                        VHID_POOL_TAG
                                        );
                            
        if(!RegistryReportDescriptor){
            
            DebugPrint(("Memory allocation for Report descriptor failed\n"));            
            ntStatus = STATUS_UNSUCCESSFUL;
            goto query_end;

        }

        //
        // Copy report descriptor from registry
        //
        RtlCopyMemory(RegistryReportDescriptor, 
                      ((PKEY_VALUE_PARTIAL_INFORMATION)
                      keyValueInformation)->Data, 
                      RegistryReportDescriptorLength
                      );

        DebugPrint(("No. of report descriptor bytes copied: %d\n", 
                    RegistryReportDescriptorLength
                    ));

        //
        // Store the registry report descriptor in the device extension
        //
        deviceInfo->ReadReportDescFromRegistry = TRUE;
        deviceInfo->ReportDescriptor = RegistryReportDescriptor;
        deviceInfo->HidDescriptor.DescriptorList[0].wReportLength = 
                                   (USHORT)RegistryReportDescriptorLength;

    }   
    else {
        if(!NT_SUCCESS(ntStatus)) {            
            DebugPrint(("ZwQueryValueKey failed status:0x%x\n", ntStatus));                
        }
    }            

    query_end:

    ZwClose(handle);
   
    if(keyValueInformation)
        ExFreePool(keyValueInformation);
    
    return ntStatus;

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
    //case IRP_MN_QUERY_LEGACY_BUS_INFORMATION: // Not defined in WDM.H
        //return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";
    default:
        return "unknown_pnp_irp";
    }
}

#endif

