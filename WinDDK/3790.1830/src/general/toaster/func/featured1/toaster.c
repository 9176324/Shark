/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Toaster.c

Abstract:

    Featured1\Toaster.c is the third stage of the Toaster sample function driver.
    This stage builds on the support implemented in the second stage,
    Incomplete2.c.

    New features implemented in this stage of the function driver:

    -A cancel routine to remove canceled IRPs from the driver-managed IRP queue.
     This routine is named ToasterCancelQueued. The logic to cancel an IRP while
     it is in the driver-managed IRP queue is complex and located in several other
     places in the function driver, including:
     --ToasterCleanup
     --ToasterQueueRequest
     --ToasterProcessQueuedRequests
     --ToasterCancelQueued

    -Code to prevent adding an IRP to the driver-managed IRP queue if the system
     has canceled it. This code is related to, but in addition to
     ToasterCancelQueued.

    -Code that demonstrates how to hide a hardware instance from displaying in the
     Device Manager, unless the "Show hidden devices" menu item is checked. This
     code is in the ToasterDispatchIoctl routine.

    -A dispatch routine to support IRP_MJ_CLEANUP. This routine is named
     ToasterCleanup.

    -Full support for power management IRPs. All the power management related
     code is located in Power.c in this stage of the function driver.

    -Full support for WMI IRPs. All the system control related code is located
     in Wmi.c in this stage of the function driver.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub - 10-Oct-2002

    Kevin Shirley – 08-May-2003 – Commented for tutorial and learning purposes

--*/
#include "toaster.h"

ULONG DebugLevel = INFO;
#define _DRIVER_NAME_ "Featured1"

//
// Declare a variable to hold all global variables used throughout the function
// driver. The GLOBALS data type is defined in Toaster.h. All the global variables
// used by the driver are defined in the GLOBALS data type. This technique reduces
// the number of variables in the function driver’s global namespace.
//
GLOBALS Globals;

#ifdef ALLOC_PRAGMA
//
// Remove ToasterSystemControl from Toaster.c in this stage of the function driver
// because the WMI support implemented in this stage is in the file, Wmi.c.
//
// Remove ToasterDispatchPower from Toaster.c in this stage of the function driver
// because the Power management support implemented in this stage is in the file,
// Power.c.
//
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ToasterAddDevice)
#pragma alloc_text (PAGE, ToasterDispatchPnp)
#pragma alloc_text (PAGE, ToasterReadWrite)
#pragma alloc_text (PAGE, ToasterCreate)
#pragma alloc_text (PAGE, ToasterClose)
#pragma alloc_text (PAGE, ToasterDispatchIoctl)
#pragma alloc_text (PAGE, ToasterDispatchIO)
#pragma alloc_text (PAGE, ToasterStartDevice)
#pragma alloc_text (PAGE, ToasterUnload)
#pragma alloc_text (PAGE, ToasterSendIrpSynchronously)
#endif

 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Updated Routine Description:
    DriverEntry saves a copy of the registry path string. A copy of the string
    must be saved instead of the pointer because the pointer becomes invalid after
    DriverEntry returns. ToasterUnload releases the memory when the system unloads
    the driver from memory.

    The INF file used to install the Toaster sample function driver places initial
    information in the Registry. In the case of the Toaster sample, a value called
    BeepCount was created. This value is not used in the function driver (in any
    stage). It simply demonstrates how initial values can be added to the Registry
    to be subsequently accessed by the function driver.

Updated Return Value Description:
    DriverEntry returns STATUS_INSUFFICIENT_RESOURCES if it cannot allocate memory
    to store the copy of the string. Otherwise, DriverEntry returns STATUS_SUCCESS.
--*/
{
    NTSTATUS            status = STATUS_SUCCESS;

    ToasterDebugPrint(TRACE, "Entered DriverEntry of "_DRIVER_NAME_" version built on "
                                                          __DATE__" at "__TIME__ "\n");

    //
    // Create a copy of the registry path string and store it in the Globals
    // variable. The Globals variable datatype is defined in Toaster.h.
    //
    // Determine the number of bytes to allocate for the string. RegistryPath is
    // a counted UNICODE_STRING.
    //
    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;

    //
    // Allocate memory to store a copy of the registry path string. The memory
    // allocated to hold the string is tagged with TOASTER_POOL_TAG. This
    // technique specifies a 4-byte value when memory is allocated. A debugger can
    // then search memory for the tag to identify specific allocations.
    // TOASTER_POOL_TAG is defined in Toaster.h as "soaT". "soaT" is "Toas"
    // backwards. Specify "Toas" when using a debugger to search for memory
    // allocated by the Toaster sample function driver.
    //
    Globals.RegistryPath.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       Globals.RegistryPath.MaximumLength,
                                       TOASTER_POOL_TAG);

    if (NULL == Globals.RegistryPath.Buffer)
    {
        ToasterDebugPrint(ERROR,
                "Couldn't allocate pool for registry path.");

        //
        // If DriverEntry returns a failure, then the system unloads the driver
        // image from memory and the action specified in the ErrorControl directive
        // in the INF file used to install the driver is taken.
        //
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&Globals.RegistryPath, RegistryPath);

    DriverObject->DriverExtension->AddDevice           = ToasterAddDevice;

    DriverObject->DriverUnload                         = ToasterUnload;

    DriverObject->MajorFunction[IRP_MJ_PNP]            = ToasterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = ToasterDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ToasterCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ToasterClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = ToasterCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ToasterDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_READ]           = ToasterDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = ToasterDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ToasterSystemControl;

    return status;
}


 
NTSTATUS
ToasterAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

Updated Routine Description:
    ToasterAddDevice assigns a tag to the functional device object’s (FDO’s)
    device extension. A debugger can then search memory for the tag to identify
    toaster instance device extensions.

    ToasterAddDevice also initialized the hardware instance’s power state in the
    device extension.

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PFDO_DATA               fdoData;
    POWER_STATE             powerState;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "AddDevice PDO (0x%p)\n", PhysicalDeviceObject);

    status = IoCreateDevice (DriverObject,
                             sizeof (FDO_DATA),
                             NULL,
                             FILE_DEVICE_UNKNOWN,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &deviceObject);

    if (!NT_SUCCESS (status))
    {
        return status;
    }

    ToasterDebugPrint(INFO, "AddDevice FDO (0x%p)\n", deviceObject);

    fdoData = (PFDO_DATA) deviceObject->DeviceExtension;

    fdoData->Signature = TOASTER_FDO_INSTANCE_SIGNATURE;
    
    INITIALIZE_PNP_STATE(fdoData);

    fdoData->UnderlyingPDO = PhysicalDeviceObject;

    fdoData->QueueState = HoldRequests;

    fdoData->Self = deviceObject;

    fdoData->NextLowerDriver = NULL;

    InitializeListHead(&fdoData->NewRequestsQueue);

    KeInitializeSpinLock(&fdoData->QueueLock);

    KeInitializeEvent(&fdoData->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);

    KeInitializeEvent(&fdoData->StopEvent,
                      SynchronizationEvent,
                      TRUE);

    fdoData->OutstandingIO = 1;

    //
    // Initialize DontDisplayInUI to FALSE. ToasterDispatchIoctl sets DontDisplayInUI
    // to TRUE when it processes IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE. This causes
    // ToasterDispatchPnP to hide the hardware instance when it processes
    // IRP_MN_QUERY_PNP_DEVICE_STATE.
    //
    fdoData->DontDisplayInUI = FALSE;

    deviceObject->Flags |= DO_POWER_PAGABLE;
    deviceObject->Flags |= DO_BUFFERED_IO;

    //
    // Initialize DevicePowerState to PowerDeviceD3. PowerDeviceD3 is the lowest
    // power state for hardware. ToasterAddDevice initializes DevicePowerState to
    // the lowest power state instead of the working power state because the
    // hardware instance cannot be working until ToasterDispatchPnP processes
    // IRP_MN_START_DEVICE.
    //
    fdoData->DevicePowerState = PowerDeviceD3;

    //
    // Initialize SystemPowerState to PowerSystemWorking. PowerSystemWorking is the
    // highest power state for the system. ToasterAddDevice initializes
    // SystemPowerState to the highest power state because the system only executes
    // code, such as drivers, when the system is working.
    //
    fdoData->SystemPowerState = PowerSystemWorking;

    //
    // Notify the power manager of the initial power state of the hardware instance.
    //
    powerState.DeviceState = PowerDeviceD3;
    PoSetPowerState ( deviceObject, DevicePowerState, powerState );

    fdoData->NextLowerDriver = IoAttachDeviceToDeviceStack (deviceObject,
                                                       PhysicalDeviceObject);
    if(NULL == fdoData->NextLowerDriver)
    {
        IoDeleteDevice(deviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

    status = IoRegisterDeviceInterface (
                PhysicalDeviceObject,
                (LPGUID) &GUID_DEVINTERFACE_TOASTER,
                NULL,
                &fdoData->InterfaceName);

    if (!NT_SUCCESS (status))
    {
        ToasterDebugPrint(ERROR,
            "AddDevice: IoRegisterDeviceInterface failed (%x)\n", status);
        IoDetachDevice(fdoData->NextLowerDriver);
        IoDeleteDevice (deviceObject);
        return status;
    }

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

 
VOID
ToasterUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Updated Routine Description:
    ToasterUnload releases the memory that was allocated earlier in DriverEntry to
    hold a copy of the registry path string.

--*/
{
    PAGED_CODE();

    ASSERT(DriverObject->DeviceObject == NULL);

    ToasterDebugPrint(TRACE, "unload\n");

    if(NULL != Globals.RegistryPath.Buffer)
    {
        //
        // Release the memory allocated earlier in DriverEntry that stored a copy of
        // the registry path string.
        //
        ExFreePool(Globals.RegistryPath.Buffer);
    }

    return;
}



 
NTSTATUS
ToasterDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchPnP calls ToasterCanStopDevice when it processes
    IRP_MN_QUERY_STOP_DEVICE to determine whether the hardware instance can be
    stopped without losing any data.

    ToasterDispatchPnP calls ToasterCanRemoveDevice when it processes
    IRP_MN_QUERY_REMOVE_DEVICE to determine whether the hardware instance can be
    removed without losing any data.

    ToasterDispatchPnP calls ToasterReturnResources when it processes
    IRP_MN_STOP_DEVICE, IRP_MN_QUERY_REMOVE_DEVICE, IRP_MN_SURPRISE_REMOVAL,
    and IRP_MN_REMOVE_DEVICE to return the hardware resources that
    ToasterStartDevice used earlier to start the hardware instance.

    ToasterDispatchPnP processes a new PnP IRP, IRP_MN_QUERY_CAPABILITIES to
    demonstrate how to process a PnP IRP as it is passed down the device stack
    and again as it is passed back up the device stack.

    ToasterDispatchPnP processes a new PnP IRP, IRP_MN_QUERY_PNP_DEVICE_STATE to
    hide the hardware instance from displaying in the Device Manager.

--*/
{
    PFDO_DATA               fdoData;
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_CAPABILITIES    deviceCapabilities;
    ULONG                   requestCount;
    PPNP_DEVICE_STATE       deviceState;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    stack = IoGetCurrentIrpStackLocation (Irp);

    ToasterDebugPrint(TRACE, "FDO %s \n",
                PnPMinorFunctionString(stack->MinorFunction));

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return STATUS_NO_SUCH_DEVICE;
    }

    switch (stack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        if (NT_SUCCESS (status))
        {
            status = ToasterStartDevice (fdoData, Irp);
        }

        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        //
        // Determine if the hardware instance can safely be stopped, without losing
        // any data.
        //
        status = ToasterCanStopDevice(DeviceObject, Irp);

        if (NT_SUCCESS(status))
        {
            SET_NEW_PNP_STATE(fdoData, StopPending);

            fdoData->QueueState = HoldRequests;

            ToasterDebugPrint(INFO, "Holding requests...\n");

            ToasterIoDecrement(fdoData);

            KeWaitForSingleObject(
               &fdoData->StopEvent,
               Executive,
               KernelMode,
               FALSE,
               NULL);

            Irp->IoStatus.Status = STATUS_SUCCESS;

            IoSkipCurrentIrpStackLocation (Irp);

            status = IoCallDriver (fdoData->NextLowerDriver, Irp);

            return status;
        }

        break;

   case IRP_MN_CANCEL_STOP_DEVICE:
        //
        // Determine if the function driver previously processed
        // IRP_MN_QUERY_STOP_DEVICE. If the function driver previously processed
        // IRP_MN_QUERY_STOP_DEVICE, then the hardware instance’s DevicePnPState
        // would have been set to StopPending. If the function driver did not
        // previously process IRP_MN_QUERY_STOP_DEVICE, then this is a spurious
        // IRP_MN_CANCEL_STOP_DEVICE.
        //
        // The function driver can receive a spurious IRP_MN_CANCEL_STOP_DEVICE
        // without first receiving a IRP_MN_QUERY_STOP_DEVICE if a driver above the
        // function driver failed to pass IRP_MN_QUERY_STOP_DEVICE down the device
        // stack, but then subsequently passed down a IRP_MN_CANCEL_STOP_DEVICE.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status) && StopPending == fdoData->DevicePnPState)
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            fdoData->QueueState = AllowRequests;

            ASSERT(Started == fdoData->DevicePnPState);

            ToasterProcessQueuedRequests(fdoData);
        }

        break;

    case IRP_MN_STOP_DEVICE:
        SET_NEW_PNP_STATE(fdoData, Stopped);

        //
        // Return the hardware resources that ToasterStartDevice used earlier to
        // start the hardware instance when ToasterDispatchPnP processed
        // IRP_MN_START_DEVICE.
        //
        status = ToasterReturnResources(DeviceObject);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        //
        // Determine if the hardware instance can safely be removed, without losing
        // any data.
        //
        status = ToasterCanRemoveDevice(DeviceObject, Irp);

        if (NT_SUCCESS(status))
        {
            SET_NEW_PNP_STATE(fdoData, RemovePending);

            fdoData->QueueState = HoldRequests;

            ToasterDebugPrint(INFO, "Query - remove holding requests...\n");

            ToasterIoDecrement(fdoData);

            KeWaitForSingleObject(
                &fdoData->StopEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);

            Irp->IoStatus.Status = STATUS_SUCCESS;

            IoSkipCurrentIrpStackLocation (Irp);

            status = IoCallDriver (fdoData->NextLowerDriver, Irp);

            return status;
        }

        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        //
        // Determine if the function driver previously processed
        // IRP_MN_QUERY_REMOVE_DEVICE. If the function driver previously processed
        // IRP_MN_QUERY_REMOVE_DEVICE, then the hardware instance’s DevicePnPState
        // would have been set to RemovePending. If the function driver did not
        // previously process IRP_MN_QUERY_REMOVE_DEVICE, then this is a spurious
        // IRP_MN_CANCEL_REMOVE_DEVICE.
        //
        // The function driver can receive a spurious IRP_MN_CANCEL_REMOVE_DEVICE
        // without first receiving a IRP_MN_QUERY_REMOVE_DEVICE if a driver above the
        // function driver failed to pass IRP_MN_QUERY_REMOVE_DEVICE down the device
        // stack, but then subsequently passed down a IRP_MN_CANCEL_REMOVE_DEVICE.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status) && RemovePending == fdoData->DevicePnPState)
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            fdoData->QueueState = AllowRequests;

            ToasterProcessQueuedRequests(fdoData);
        }

        break;

   case IRP_MN_SURPRISE_REMOVAL:
        SET_NEW_PNP_STATE(fdoData, SurpriseRemovePending);

        fdoData->QueueState = FailRequests;

        ToasterProcessQueuedRequests(fdoData);

        status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

        if (!NT_SUCCESS (status))
        {
            ToasterDebugPrint(ERROR, 
                "IoSetDeviceInterfaceState failed: 0x%x\n", status);
        }

        //
        // Return the hardware resources that ToasterStartDevice used earlier to
        // start the hardware instance when ToasterDispatchPnP processed
        // IRP_MN_START_DEVICE.
        //
        ToasterReturnResources(DeviceObject);

        //
        // Deregister with WMI. The function driver registered to be a WMI provider
        // earlier in ToasterStartDevice called ToasterWmiRegistration.
        //
        ToasterWmiDeRegistration(fdoData);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;

   case IRP_MN_REMOVE_DEVICE:
        SET_NEW_PNP_STATE(fdoData, Deleted);

        if(SurpriseRemovePending != fdoData->PreviousPnPState)
        {
            fdoData->QueueState = FailRequests;

            ToasterProcessQueuedRequests(fdoData);

            status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

            if (!NT_SUCCESS (status))
            {
                ToasterDebugPrint(ERROR,
                        "IoSetDeviceInterfaceState failed: 0x%x\n", status);
            }

            //
            // Return the hardware resources that ToasterStartDevice used earlier to
            // start the hardware instance when ToasterDispatchPnP processed
            // IRP_MN_START_DEVICE.
            //
            ToasterReturnResources(DeviceObject);
            
            //
            // Deregister with WMI. The function driver registered to be a WMI provider
            // earlier in ToasterStartDevice called ToasterWmiRegistration.
            //
            ToasterWmiDeRegistration(fdoData);
        }

        requestCount = ToasterIoDecrement (fdoData);

        ASSERT(requestCount > 0);

        requestCount = ToasterIoDecrement (fdoData);

        KeWaitForSingleObject (
                &fdoData->RemoveEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        IoDetachDevice (fdoData->NextLowerDriver);

        RtlFreeUnicodeString(&fdoData->InterfaceName);

        IoDeleteDevice (fdoData->Self);

        return status;

    case IRP_MN_QUERY_CAPABILITIES:
        //
        // ToasterDispatchPnP processes IRP_MN_QUERY_CAPABILITIES both as it is
        // passed down the device stack and later after the underlying bus driver
        // has completed IRP_MN_QUERY_CAPABILITIES and it is being passed back up
        // the device stack.
        //
        // It is optional for a driver to processing IRP_MN_QUERY_CAPABILITIES.
        // However, as with all PnP IRPs, IRP_MN_QUERY_CAPABILITES must be passed
        // down the device stack to be processed by the underlying bus driver.
        //
        // The function driver can modify the information in the IRP before it is
        // passed down the device stack, and again after the bus driver has completed
        // it and the IRP is being passed back up the device stack.
        //

        deviceCapabilities = stack->Parameters.DeviceCapabilities.Capabilities;

        if(1 != deviceCapabilities->Version ||
            deviceCapabilities->Size < sizeof(DEVICE_CAPABILITIES))
        {
            //
            // Presently, the function driver only supports version 1 of the
            // DEVICE_CAPABILITIES structure. Therefore, if the version does not
            // equal 1, then set the IRP's IoStatus.Status to STATUS_UNSUCCESSFUL
            // before passing the IRP down the device stack to be processed by the
            // bus driver.
            //
            status = STATUS_UNSUCCESSFUL;

            break;
        }

        //
        // The values of the DEVICE_CAPABILITES structure can be modified before
        // IRP_MN_QUERY_CAPABILITES is passed down the device stack. If the function
        // driver must change values before passing the IRP down the device stack,
        // then those changed must occur here.
        //

        //
        // Pass the IRP down the device stack to be processed by the bus driver.
        // ToasterSendIrpSynchronously does not return until the bus driver completes
        // IRP_MN_QUERY_CAPABILITIES.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        //
        // After ToasterSendIrpSynchronously returns, the bus driver has completed
        // IRP_MN_QUERY_CAPABILITES and possibly changed some of its information.
        //
        // The function driver can then further modify any information in the IRP,
        // or override the information set by the bus driver.
        //
        if (NT_SUCCESS (status) )
        {
            //
            // Get the device capabilities supported by the hardware instance.
            //
            fdoData->DeviceCaps = *deviceCapabilities;
        }

        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        //
        // The system sends IRP_MN_QUERY_PNP_DEVICE_STATE to determine the plug and
        // play state of the hardware instance. The IRP’s IoStatus.Information member
        // describes the hardware’s Pnp device state. The function driver hides the
        // hardware instance from displaying in the Device Manager by setting the
        // PNP_DEVICE_DONT_DISPLAY_IN_UI bit in the IRP’s IoStatus.Information
        // member.
        //
        // The underlying bus driver must process IRP_MN_QUERY_PNP_DEVICE_STATE
        // before the function driver can because the modification to the IRP is
        // performed on the IRP’s passage back up the device stack.
        //
        // It is optional for a driver to process IRP_MN_QUERY_PNP_DEVICE_STATE.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        if(TRUE == fdoData->DontDisplayInUI)
        {
            //
            // ToasterDispatchIoctl set DontDisplayinUI to TRUE when it processed
            // IOCTL_DONT_DISPLAY_UI_IN_DEVICE. ToasterDispatchIoctl then called
            // IoInvalidateDeviceState, which causes the system to send
            // IRP_MN_QUERY_PNP_DEVICE_STATE. When ToasterDispatchPnP completes
            // IRP_MN_QUERY_PNP_DEVICE_STATE, then the hardware instance is no longer
            // displayed in the Device Manager unless the "Show hidden devices" menu
            // item is checked.
            //
            deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);

            if(NULL != deviceState)
            {
                SETMASK((*deviceState), PNP_DEVICE_DONT_DISPLAY_IN_UI);
            }
            else
            {
                ASSERT(deviceState);
            }

            status = STATUS_SUCCESS;
        }

        break;

    default:
        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    ToasterIoDecrement(fdoData);

    return status;
}


 
NTSTATUS
ToasterDispatchPnpComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
/*++

Updated Routine Description:
    ToasterDispatchPnpComplete does not change in this stage of the function
    driver.

--*/
{
    PKEVENT             event = (PKEVENT)Context;

    UNREFERENCED_PARAMETER (DeviceObject);

    if (TRUE == Irp->PendingReturned)
    {
        KeSetEvent (event, IO_NO_INCREMENT, FALSE);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}



 
NTSTATUS
ToasterCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Updated Routine Description:
    ToasterCreate calls ToasterGetStandardInterface to demonstrate how to return a
    direct-call interface to the underlying bus driver.

--*/
{
    PFDO_DATA    fdoData;
    NTSTATUS     status;
    TOASTER_INTERFACE_STANDARD busInterface;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Create \n");

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Here is an example of how to get the direct-call interface exported 
    // by the underlying bus driver. Please note that the following block
    // of code is for demonstration purposes only - you may not need this in a real
    // driver. 
    // The TOASTER_INTERFACE_STANDARD data type is defined in Driver.h, and is a custom
    // structure defined by the author of the bus driver. The author of the bus
    // driver determines what functionality to export for other drivers to call based
    // on the details of the bus hardware. In the case of the Toaster sample bus
    // driver, the Toaster bus hardware supports a "crispiness level" and a "safety
    // lock" mechanism. 
    //
    status = ToasterGetStandardInterface(fdoData->NextLowerDriver, &busInterface);

    if(NT_SUCCESS(status))
    {
        UCHAR powerlevel;

        (*busInterface.GetCrispinessLevel)(busInterface.Context, &powerlevel);
        (*busInterface.SetCrispinessLevel)(busInterface.Context, 8);
        (*busInterface.IsSafetyLockEnabled)(busInterface.Context);

        //
        // When ToasterGetStandardInterface returns a pointer to the direct-call
        // interface exported by the Toaster bus driver, the bus driver increments a
        // reference count to itself. This prevents the system from stopping or
        // removing the bus driver while other drivers have pointers to the bus
        // driver’s callback functions.
        //
        // When the function driver no longer requires the direct-call interface, it
        // must call the direct-call interface’s "InterfaceDereference" method to
        // decrement the bus driver’s reference count. Otherwise, the system cannot
        // unload the bus driver.
        //
        // If the device interface is from an unrelated device we must register a
        // PnP notification for device removal to dereference and stop using the interface.
        //
        (*busInterface.InterfaceDereference)((PVOID)busInterface.Context);
    }

    //
    // We don't want to fail Create just because we weren't able to get the
    // direct-call interface. If this driver is loaded on top of a bus other
    // than toaster, ToasterGetStandardInterface will return an error.
    //

    status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    ToasterIoDecrement(fdoData);

    return status;
}


 
NTSTATUS
ToasterClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Updated Routine Description:
    ToasterClose does not change in this stage of the function driver.

--*/
{
    PFDO_DATA    fdoData;
    NTSTATUS     status;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Close \n");

    ToasterIoIncrement (fdoData);

    status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    ToasterIoDecrement(fdoData);

    return status;
}


 
NTSTATUS
ToasterDispatchIoctl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchIoctl implements support to hide a hardware instance from
    displaying in the Device Manager, unless the "Show hidden devices" menu item
    is checked.

--*/
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status= STATUS_SUCCESS;
    PFDO_DATA               fdoData;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Ioctl called\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE:
        //
        // An application or driver sends IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE to
        // hide the hardware instance from displaying in the Device Manager. When
        // the function driver receives this IOCTL, it sets DontDisplayInUI to TRUE
        // and then calls IoInvalidateDeviceState.
        //
        // Calling IoInvalidateDeviceState causes the system to send 
        // IRP_MN_QUERY_PNP_DEVICE_STATE to the hardware instance’s device stack. In
        // this stage of the function driver, ToasterDispatchPnP processes this IRP,
        // and sets the PNP_DEVICE_DONT_DISPLAY_IN_UI bit in the IRP’s
        // IoStatus.Information, which prevents the Device Manager from displaying
        // the hardware instance once the function driver has completed the IRP.
        //
        // IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE is defined in Public.h. The
        // Toaster sample function driver only defines the single IOCTL, 
        // IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE, however, a driver-writer can
        // define whatever custom IOCTLs are necessary. If the IOCTLs are intended
        // to be used by 3rd parties, then the driver-writer must release them to the
        // public.
        //
        fdoData->DontDisplayInUI = TRUE;

        IoInvalidateDeviceState(fdoData->UnderlyingPDO);

        Irp->IoStatus.Information = 0;

        break;

    default:
        ASSERTMSG(FALSE, "Invalid IOCTL request\n");

        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

 
NTSTATUS
ToasterDispatchIO(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchIO does not change in this stage of the function driver.

--*/
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status= STATUS_SUCCESS;
    PFDO_DATA               fdoData;

    PAGED_CODE();
    
    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterIoIncrement (fdoData);
    
    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return STATUS_NO_SUCH_DEVICE;
    }
    
    if (HoldRequests == fdoData->QueueState)
    {
        return ToasterQueueRequest(fdoData, Irp);
    }

    irpStack = IoGetCurrentIrpStackLocation (Irp);
    
    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_READ:

    case IRP_MJ_WRITE:
        status =  ToasterReadWrite(DeviceObject, Irp);

        break;

    case IRP_MJ_DEVICE_CONTROL:
        status =  ToasterDispatchIoctl(DeviceObject, Irp);

        break;

    default:
        ASSERTMSG(FALSE, "ToasterDispatchIO invalid IRP");

        status = STATUS_UNSUCCESSFUL;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        break;
    }               

    ToasterIoDecrement(fdoData);

    return status;
}


 
NTSTATUS
ToasterReadWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterReadWrite does not change in this stage of the function driver.

--*/

{
    PFDO_DATA   fdoData;
    NTSTATUS    status;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "ReadWrite called\n");

    status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

 
NTSTATUS
ToasterStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP             Irp
    )
/*++

Updated Routine Description:
    ToasterStartDevice demonstrates how to initialize the hardware instance using
    the hardware resources that the underlying bus driver assigns to the hardware
    instance. Examples of such resources are mapping I/O ports or I/O memory
    ranges, allocating a DMA adapter and connecting to an interrupt vector, if
    applicable.

    ToasterDispatchPnP calls ToasterStartDevice after the underlying bus driver
    has processed IRP_MN_START_DEVICE. The bus driver must process
    IRP_MN_START_DEVICE before the function driver can because until the bus
    driver has processed IRP_MN_START_DEVICE, the IRP does not contain the
    hardware resources required by the function driver to program the hardware
    instance.

    The hardware resources assigned by the bus driver to the device instance are
    stored in the IRP’s I/O stack location. The Parameters.StartDevice member of
    the IRP describes both raw and translated resources to use for the hardware
    instance. The translated resources are used to map I/O ports, I/O memory 
    ranges, and connect to the hardware’s interrupt vector, if applicable and
    initialize the hardware.

--*/
{
    NTSTATUS    status = STATUS_SUCCESS;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resource;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceTrans;
    PCM_PARTIAL_RESOURCE_LIST   partialResourceList;
    PCM_PARTIAL_RESOURCE_LIST   partialResourceListTranslated;
    PIO_STACK_LOCATION stack;
    POWER_STATE powerState;
    ULONG i;

    //
    // Get the parameters of the IRP from the function driver’s location in the IRP’s
    // I/O stack. The results of the function driver’s processing of the IRP, if any,
    // are then stored back in the same I/O stack location.
    //
    // The hardware resources that the bus driver assigned the hardware instance are
    // stored in the IRP’s Parameters.StartDevice member.
    //
    stack = IoGetCurrentIrpStackLocation (Irp);

    PAGED_CODE();

    //
    // Initialize the hardware instance. Perform any tasks required to start the
    // device hardware, such as updating the Registry and gathering information
    // from it.
    //

    if ((NULL != stack->Parameters.StartDevice.AllocatedResources) &&
        (NULL != stack->Parameters.StartDevice.AllocatedResourcesTranslated))
    {
        //
        // Get the pointer to the raw list of hardware resources. ToasterStartDevice
        // uses these resources to program the hardware instance.
        //
        partialResourceList =
        &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;

        //
        // Get the pointer to the first raw hardware resource in the list. As this
        // routine processes the list of raw hardware resources, a for loop iterates
        // this pointer until entire list has been processed.
        //
        resource = &partialResourceList->PartialDescriptors[0];

        //
        // Get the pointer to the translated list of hardware resources.
        // ToasterStartDevice uses these resources to map I/O ports and I/O memory
        // ranges, and connect to the hardware’s interrupt vector, if applicable.
        //
        partialResourceListTranslated =
        &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;

        //
        // Get the pointer to the first translated hardware resource in the list. As
        // this routine processes the list of translated hardware resources, a for
        // loop iterates this pointer until entire list has been processed.
        //
        resourceTrans = &partialResourceListTranslated->PartialDescriptors[0];

        for (i = 0;
             i < partialResourceList->Count;
             i++, resource++, resourceTrans++)
        {
            //
            // Enter a loop to process all the entries in the raw hardware resource
            // list. The loop exits after all the hardware resources in the list have
            // been processed.
            //

            switch (resource->Type)
            {
            case CmResourceTypePort:
                ToasterDebugPrint(INFO, "Resource RAW Port: (%x) Length: (%d)\n",
                    resource->u.Port.Start.LowPart, resource->u.Port.Length);

                switch (resourceTrans->Type)
                {
                case CmResourceTypePort:
                    ToasterDebugPrint(INFO,
                        "Resource Translated Port: (%x) Length: (%d)\n",
                        resourceTrans->u.Port.Start.LowPart,
                        resourceTrans->u.Port.Length);

                    break;

                case CmResourceTypeMemory:
                    ToasterDebugPrint(INFO,
                        "Resource Translated Memory: (%x) Length: (%d)\n",
                        resourceTrans->u.Memory.Start.LowPart,
                        resourceTrans->u.Memory.Length);

                    //
                    // Map any I/O memory here. Because all the toaster hardware is
                    // imaginary (the device hardware, as well as the bus hardware),
                    // there is no code to execute here. The PCIDRV sample that is
                    // shipped with the DDK demonstrates this concept.
                    // memory.
                    //

                    break;

                default:
                    ToasterDebugPrint(ERROR, "Unhandled resource_type (0x%x)\n",
                                            resourceTrans->Type);

                    DbgBreakPoint();
                }
                break;

            case CmResourceTypeMemory:
                ToasterDebugPrint(INFO, "Resource RAW Memory: (%x) Length: (%d)\n",
                    resource->u.Memory.Start.LowPart, resource->u.Memory.Length);

                ToasterDebugPrint(INFO, "Resource Translated Memory: (%x) Length: (%d)\n",
                    resourceTrans->u.Memory.Start.LowPart, resourceTrans->u.Memory.Length);

                //
                // Map any I/O memory here. Because all the toaster hardware is
                // imaginary (the device hardware, as well as the bus hardware),
                // there is no code to execute here. The PCIDRV sample that is
                // shipped with the DDK demonstrates this concept.
                //

                break;

            case CmResourceTypeInterrupt:
                break;

            default:
                ToasterDebugPrint(ERROR, "Unhandled resource type (0x%x)\n", resource->Type);

                break;
            }
        }
    }

    //
    // When the system dispatches IRP_MN_START_DEVICE to the function driver, that
    // represents an implicit power-up operation. After the hardware instance has
    // been programmed (in the switch-cases earlier) the hardware instance should
    // now be in the PowerDeviceDO power state.
    //
    FdoData->DevicePowerState = PowerDeviceD0;

    powerState.DeviceState = PowerDeviceD0;

    //
    // Notify the power manager of the new device power state.
    //
    PoSetPowerState ( FdoData->Self, DevicePowerState, powerState );

    status = IoSetDeviceInterfaceState(&FdoData->InterfaceName, TRUE);

    if (!NT_SUCCESS (status))
    {
        ToasterDebugPrint(ERROR, "IoSetDeviceInterfaceState failed: 0x%x\n",
                                status);
        return status;
    }

    //
    // Register with WMI. Register with WMI only once during the lifetime of the
    // function driver. Skip registering with WMI if the function driver has already
    // registered itself. If DevicePnPState equals Stopped, then the system sent
    // IRP_MN_STOP_DEVICE to the function driver, possibly to rebalance hardware
    // resources, and the function driver has already called ToasterWmiRegistration.
    //
    if(Stopped != FdoData->DevicePnPState)
    { 
        status = ToasterWmiRegistration(FdoData);

        if (!NT_SUCCESS (status))
        {
            ToasterDebugPrint(ERROR, "IoWMIRegistrationControl failed (%x)\n", status);
            return status;
        }
    }
    
    SET_NEW_PNP_STATE(FdoData, Started);

    FdoData->QueueState = AllowRequests;

    ToasterProcessQueuedRequests(FdoData);

    //
    // Notify WMI that the hardware instance is fully powered and ready to process
    // requests.
    //
    ToasterFireArrivalEvent(FdoData->Self);

    return status;
}


 
NTSTATUS
ToasterCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

New Routine Description:
    The system dispatches IRP_MJ_CLEANUP IRPs to ToasterCleanup. ToasterCleanup
    cancels all IRPs from the driver-managed IRP queue whose file object matches
    the file object of the incoming IRP.

    IRP_MJ_CLEANUP and IRP_MJ_CLOSE are related but not identical, and are often
    confused. The system sends IRP_MJ_CLOSE to the function driver to close a
    handle returned earlier by the function driver’s IRP_MJ_CREATE dispatch
    routine. ToasterCreate is the function drivers IRP_MJ_CREATE dispatch routine.
    However, ToasterClose only closes the file object handle. It does not cancel
    any IRPs in the driver-managed IRP queue that are associated with the handle.
    Thus, the system sends IRP_MJ_CLEANUP to the function driver, so that the
    driver can remove any IRPs in the driver-managed IRP queue that belonged to
    the handle that is now closed.

    ToasterCleanup processes each queued IRP in the driver-managed IRP queue and,
    if the queued IRP's file object matches that of the cleanup IRP, then the
    queued IRP is removed from the driver-managed IRP queue and added to a
    temporary IRP queue.

    After every IRP in the driver-managed IRP queue has been processed, the
    temporary IRP queue is processed. Each IRP in the temporary IRP queue is
    removed and completed with STATUS_CANCELLED.

    Note that ToasterCleanup does not call the PAGED_CODE macro because the routine
    uses a spin lock.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the cleanup operation associated with DeviceObject.

Return Value Description:
    ToasterCleanup returns STATUS_SUCCESS.

--*/
{
    PFDO_DATA              fdoData;
    KIRQL                  oldIrql;
    LIST_ENTRY             cleanupList;
    PLIST_ENTRY            thisEntry, nextEntry, listHead;
    PIRP                   pendingIrp;
    PIO_STACK_LOCATION     pendingIrpStack, irpStack;

    ToasterDebugPrint(TRACE, "Cleanup called\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterIoIncrement (fdoData);

    //
    // Get the parameters of the IRP from the function driver’s location in the IRP’s
    // I/O stack. The results of the function driver’s processing of the IRP, if any,
    // are then stored back in the same I/O stack location.
    //
    // The cleanup IRP’s file object is stored in the IRP’s FileObject member. Each
    // queued IRP’s file object must be compared with the file object of the cleanup
    // IRP.
    //
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Initialize a temporary IRP queue. Any IRP in the driver-managed IRP queue
    // whose file object matches that of the cleanup IRP is added to the temporary
    // IRP queue to be canceled at the end of ToasterCleanup.
    //
    InitializeListHead(&cleanupList);

    KeAcquireSpinLock(&fdoData->QueueLock, &oldIrql);

    //
    // Process every IRP in the driver-managed IRP queue and remove it from the
    // queue if its file object matches that of the incoming IRP.
    //
    listHead = &fdoData->NewRequestsQueue;

    for(thisEntry = listHead->Flink, nextEntry = thisEntry->Flink;
        thisEntry != listHead;
        thisEntry = nextEntry, nextEntry = thisEntry->Flink)
    {
        pendingIrp = CONTAINING_RECORD(thisEntry, IRP,
                                Tail.Overlay.ListEntry);

        pendingIrpStack = IoGetCurrentIrpStackLocation(pendingIrp);

        if (irpStack->FileObject == pendingIrpStack->FileObject)
        {
            //
            // If the file object of the IRP in the driver-managed IRP queue
            // matches the file object of the incoming IRP, then remove the
            // IRP from the driver-managed IRP queue.
            //
            RemoveEntryList(thisEntry);

            //
            // Clear the IRP’s cancel routine to NULL. Then determine if the system
            // has already called the IRP’s cancel routine. If IoSetCancelRoutine
            // returns NULL, then the system has already called ToasterCancelQueued.
            // If IoSetCancelRoutine does not return NULL, then the system will not
            // call ToasterCancelQueued, because ToasterCancelQueued clears the IRP’s
            // cancel routine to NULL.
            //
            if(NULL == IoSetCancelRoutine (pendingIrp, NULL))
            {
                //
                // The system has already called ToasterCancelQueued. However,
                // ToasterCancelQueued must be waiting to acquire QueueLock because
                // QueueLock was acquired before the for-loop began to process the
                // driver-managed IRP queue.
                //
                // The thread that executes ToasterCancelQueued will resume when
                // QueueLock is released outside of the for loop. ToasterCancelQueued
                // will then proceed to remove the IRP from the driver-managed IRP
                // queue, thus ToasterCleanup must not add the IRP to the temporary
                // IRP queue. Otherwise, when the temporary queue is processed at the
                // end of ToasterCleanup the same IRP would be completed twice,
                // causing a system crash because an IRP must only be completed once.
                //
                // Initialize NewRequestsQueue’s list head to prevent
                // ToasterCancelQueued from causing a system crash when it attempts
                // to remove the IRP which has already been removed from the
                // driver-managed IRP queue earlier.
                //
                InitializeListHead(thisEntry);
            }
            else
            {
                //
                // The system has not already called ToasterCancelQueued, nor
                // will it call ToasterCancelQueued because the IRP’s cancel
                // routine has been cleared to NULL.
                //
                // Add the de-queued IRP to the temporary IRP queue to be completed
                // at the end of ToasterCleanup.
                //
                InsertTailList(&cleanupList, thisEntry);
            }
        }
    }

    KeReleaseSpinLock(&fdoData->QueueLock, oldIrql);

    while(!IsListEmpty(&cleanupList))
    {
        //
        // Process every IRP in the temporary IRP queue. Remove one IRP at a 
        // time and complete it with STATUS_CANCELLED until the queue is empty.
        //
        thisEntry = RemoveHeadList(&cleanupList);

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP,
                                Tail.Overlay.ListEntry);

        pendingIrp->IoStatus.Information = 0;

        pendingIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }

    //
    // Set the cleanup IRP’s IoStatus.Information member with the number of bytes
    // transferred during the cleanup operation. Because it is not necessary to
    // access the toaster instance’s hardware to process the cleanup request, the
    // number of bytes transferred is 0.
    //
    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    ToasterIoDecrement (fdoData);

    return STATUS_SUCCESS;
}




 
 NTSTATUS
ToasterQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    )

/*++

Updated Routine Description:
    ToasterQueueRequest sets the IRP’s cancel routine to be called by the system
    if the incoming IRP is canceled.

    In addition, ToasterQueueRequests also tests if the incoming IRP not been
    canceled between the time it was created and the time it is processed by
    ToasterQueueRequest before adding it to the tail of the driver-managed IRP
    queue. If the application that originated the IRP has been closed, then the
    IRP can be failed with STATUS_CANCELLED instead of being added to the
    driver-managed IRP queue.

--*/
{
    KIRQL               oldIrql;
    PDRIVER_CANCEL      ret;
    
    ToasterDebugPrint(TRACE, "Queuing Requests\n");

    ASSERT(HoldRequests == FdoData->QueueState);

    //
    // Mark the incoming IRP as pending. The incoming IRP must be marked pending
    // before setting the cancel routine so that the IRP can be completed in a
    // different thread context.
    //
    IoMarkIrpPending(Irp);

    KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

    //
    // Set the system to call the function driver’s I/O cancel routine,
    // ToasterCancelQueued, if the IRP is canceled after it has been added to the
    // driver-managed IRP queue.
    //
    // The cancel routine can only be set after the IRP is marked pending.
    //
    IoSetCancelRoutine (Irp, ToasterCancelQueued);

    //
    // Before adding the incoming IRP to the driver-managed IRP queue, determine if
    // the incoming IRP is, or should be canceled. If the IRP is, or should be
    // canceled, then the Cancel member of the IRP equals TRUE, and the IRP should
    // not be added to the driver-managed IRP queue.
    //
    // The incoming IRP might have been canceled if, for example, the application
    // that originated the IRP was closed before the system sent the IRP to the
    // driver to be processed.
    //
    // Checking the IRP's Cancel member must occur after the IRP's cancel routine
    // Is set.
    //
    if(TRUE == Irp->Cancel)
    {
        //
        // Clear the IRP’s cancel routine to NULL. Then determine if the system
        // has already called the IRP’s cancel routine. If IoSetCancelRoutine
        // returns NULL, then the system has already called ToasterCancelQueued.
        // If IoSetCancelRoutine does not return NULL, then the system will not
        // call ToasterCancelQueued because IoSetCancelRoutine clears the IRP’s
        // cancel routine to NULL.
        //
        ret = IoSetCancelRoutine (Irp, NULL);
        if(NULL != ret)
        {
            //
            // The system has not called ToasterCancelQueued, nor will it call
            // ToasterCancelQueued because the IRP’s cancel routine has been cleared
            // to NULL.
            //
            // Complete the incoming IRP with STATUS_CANCELLED. Note that QueueLock
            // must be released before completing the IRP, otherwise a system
            // deadlock could result.
            //

            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            Irp->IoStatus.Status = STATUS_CANCELLED;

            Irp->IoStatus.Information = 0;

            IoCompleteRequest (Irp, IO_NO_INCREMENT);

            ToasterIoDecrement(FdoData);

            //
            // ToasterQueueRequest must return STATUS_PENDING to the caller even
            // though the IRP has been completed with STATUS_CANCELLED because
            // ToasterQueueRequest called IoMarkIrpPending earlier.
            //
            return STATUS_PENDING;
        }  
    }        
    
    //
    // Add the incoming IRP to the tail of the driver-managed IRP queue if either:
    //
    // -The incoming IRP has not been canceled.
    //
    // -The incoming IRP has been canceled but the system has already called
    //  ToasterCancelQueued. The incoming IRP must be added to the driver-managed IRP
    //  queue because ToasterCancelQueue must be waiting to acquire QueueLock in
    //  order to remove the IRP from the driver-managed IRP queue. However, because
    //  the IRP has not yet been added to the driver-managed IRP queue, attempting to
    //  remove it would cause a system crash. Thus the incoming IRP must be added to
    //  the queue so that ToasterCancelQueued can remove it when QueueLock is
    //  released below.
    //
    InsertTailList(&FdoData->NewRequestsQueue,
                   &Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

    // 
    // Decrement the count below because the IRP waiting in the NewRequestsQueue
    // are not tracked. The OutStandingIoCount is used to track IRPs that are
    // active in this driver or forwarded to another driver with a completion
    // routine set in it.
    //

    ToasterIoDecrement(FdoData);

    return STATUS_PENDING;
}





 
VOID
ToasterProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    )

/*++

Updated Routine Description:
    ToasterProcessQueuedRequests determines if a de-queued IRP should
    be processed or canceled.

    ToasterProcessQueuedRequests de-queues one IRP at a time from the driver-managed
    IRP queue. Before ToasterProcessQueuedRequests processes the IRP, its I/O cancel
    routine is cleared to prevent the system from calling ToasterCancelQueued while
    ToasterProcessQueuedRequests processes the IRP. However, it is possible that the
    system has already called ToasterCancelQueued for that IRP. If the system has
    already called ToasterCancelQueued, then ToasterCancelQueued must be allowed to
    execute and remove that IRP from the driver-managed IRP queue, in which case
    ToasterProcessQueuedRequests does not process the IRP and instead lets
    ToasterCancelQueued remove the IRP from the driver-managed IRP queue.

    If the de-queued IRP has not been canceled by the system then it is re-dispatched
    to be processed by the function driver. However, if the hardware instance has
    been removed, then that IRP is completed with STATUS_NO_SUCH_DEVICE.

--*/
{

    KIRQL               oldIrql;
    PIRP                nextIrp, cancelledIrp;
    PLIST_ENTRY         listEntry;
    LIST_ENTRY          cancelledIrpList;
    PDRIVER_CANCEL      cancelRoutine= NULL;
    NTSTATUS            status;

    ToasterDebugPrint(TRACE, "Process or fail queued Requests\n");

    //
    // Initialize a temporary IRP queue. Any IRP in the driver-managed IRP queue
    // that is, or should be canceled, but whose cancel routine has not yet been
    // called by the system is added to the temporary IRP queue to be canceled at the
    // end of ToasterProcessQueuedRequests.
    //
    InitializeListHead(&cancelledIrpList);

    for(;;)
    {
        KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

        if(IsListEmpty(&FdoData->NewRequestsQueue))
        {
            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            break;
        }

        listEntry = RemoveHeadList(&FdoData->NewRequestsQueue);

        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        //
        // Clear the IRP’s cancel routine to NULL. This prevents the system from
        // calling ToasterCancelQueued if the system has not already called it.
        //
        cancelRoutine = IoSetCancelRoutine (nextIrp, NULL);

        //
        // Before processing the IRP, determine if the IRP is, or should be canceled.
        // If the IRP is, or should be canceled, then the Cancel member of the IRP
        // equals TRUE, and the IRP should be added to the temporary IRP queue to be
        // canceled at the end of ToasterProcessQueuedRequests.
        //
        // The IRP might have been canceled if, for example, the application that
        // originated the IRP has been closed but the IRP remained in the
        // driver-managed IRP queue.
        //
        if (TRUE == nextIrp->Cancel)
        {
            //
            // Determine if the system has already called the IRP’s cancel routine.
            // If IoSetCancelRoutine returns NULL, then the system has already called
            // ToasterCancelQueued. If IoSetCancelRoutine does not return NULL, then
            // the system will not call ToasterCancelQueued because
            // IoSetCancelRoutine clears the IRP’s cancel routine to NULL.
            //
            if(NULL != cancelRoutine)
            {
                //
                // The system has not called ToasterCancelQueued, nor will it call
                // ToasterCancelQueued because the IRP’s cancel routine has been
                // cleared to NULL.
                //
                // Add the canceled IRP to the temporary IRP queue to be completed at
                // the end of ToasterProcessQueuedRequests.
                //
                InsertTailList(&cancelledIrpList, listEntry);
            }
            else
            {
                //
                // The system has already called ToasterCancelQueued. However,
                // ToasterCancelQueued must be waiting to acquire QueueLock because
                // QueueLock was acquired before the for-loop began to process the
                // driver-managed IRP queue.
                //
                // The thread that executes ToasterCancelQueued will resume when
                // QueueLock is released outside of the if test. ToasterCancelQueued
                // will then proceed to remove the IRP from the driver-managed IRP
                // queue, thus ToasterProcessQueuedRequests must not add the IRP to
                // the temporary IRP queue. Otherwise, when the temporary IRP queue
                // is processed at the end of ToasterProcessQueuedRequests the same
                // IRP would be completed twice, causing a system crash because an
                // IRP must only be completed once.
                //
                InitializeListHead(listEntry);
            }

            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
        }
        else
        {
            //
            // It is very important to release the spin lock that protects the queue
            // before breaking out of the for loop. Otherwise, a system dead lock
            // could result.
            //
            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            if (FailRequests == FdoData->QueueState)
            {
                nextIrp->IoStatus.Information = 0;

                nextIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

                IoCompleteRequest (nextIrp, IO_NO_INCREMENT);
            }
            else
            {
                status = ToasterDispatchIO(FdoData->Self, nextIrp);

                if(STATUS_PENDING == status)
                {
                    break;
                }
            }
        }
    }

    while(!IsListEmpty(&cancelledIrpList))
    {
        //
        // Process every IRP in the temporary IRP queue. Remove one IRP at a time and
        // complete it with STATUS_CANCELLED until the queue is empty.
        //
        PLIST_ENTRY cancelledListEntry = RemoveHeadList(&cancelledIrpList);

        cancelledIrp = CONTAINING_RECORD(cancelledListEntry, IRP,
                                Tail.Overlay.ListEntry);

        cancelledIrp->IoStatus.Information = 0;

        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(cancelledIrp, IO_NO_INCREMENT);
    }

    return;
}

 
NTSTATUS
ToasterSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterSendIrpSynchronously does not change in this stage of the function
    driver.

--*/
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           ToasterDispatchPnpComplete,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(DeviceObject, Irp);

    if (STATUS_PENDING == status)
    {
       KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL
                             );
       status = Irp->IoStatus.Status;
    }

    return status;
}

 
VOID
ToasterCancelQueued (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

/*++

New Routine Description:
    The system calls ToasterCancelQueued to remove the incoming IRP from the
    driver-managed IRP queue because the IRP has been canceled.

    When the system dispatches a new IRP to ToasterDispatchIO and QueueState
    equals HoldRequests, then ToasterDispatchIO calls ToasterQueueRequest.
    ToasterQueueRequest sets the system to call ToasterCancelQueued if the system
    cancels the IRP.

    The IRP might need to be canceled if the application that originated the IRP
    has been closed, but the IRP remains in the driver-managed IRP queue. The
    system then calls ToasterCancelQueued to remove the IRP from the
    driver-managed IRP queue.

    A driver that implements its own driver-managed IRP queue must supply a cancel
    routine to the system when the IRP is added to the driver-managed IRP queue
    because only the driver can correctly remove the IRP from its own
    driver-managed IRP queue.

    Note that the system-wide cancel spin lock is already acquired when the system
    calls ToasterCancelQueued, and this routine must release it.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the queued I/O request that must be canceled and removed from
    the driver-managed IRP queue.

Return Value Description:
    ToasterCancelQueued does not return a value.

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;
    KIRQL               oldIrql;

    ToasterDebugPrint(TRACE, "Canceling Requests\n");

    //
    // Release the system-wide cancel spin lock. Releasing this spin lock immediately
    // upon entering ToasterCancelQueued improves overall system performance because
    // the function driver does not need to use the system provided cancel spin lock.
    //
    IoReleaseCancelSpinLock( Irp->CancelIrql);

    KeAcquireSpinLock(&fdoData->QueueLock, &oldIrql);

    //
    // Remove the canceled IRP from the driver-managed IRP queue while holding
    // QueueLock.
    //
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&fdoData->QueueLock, oldIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;

    //
    // Set the canceled IRP’s IoStatus.Information member with the number of bytes
    // transferred to 0 to indicate that the I/O manager does not need to copy any
    // data from the IRP back into the caller’s memory space.
    //
    Irp->IoStatus.Information = 0;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return;
}


 
NTSTATUS
ToasterCanStopDevice    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

/*++

New Routine Description:
    ToasterCanStopDevice determines whether the hardware instance can be stopped
    safely, without losing any data. In the case of an imaginary toaster, the
    hardware can always be stopped without losing any data.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the IRP_MN_QUERY_STOP_DEVICE operation associated with the
    hardware instance described by the DeviceObject parameter.

Return Value Description:
    ToasterCanStopDevice returns STATUS_SUCCESS to indicate that the hardware
    instance can be stopped safely, without losing any data. Because the Toaster
    hardware is imaginary, IRP_MN_QUERY_STOP_DEVICE always succeeds. However, in
    practice, ToasterCanStopDevice could return many errors to the caller. A
    driver might return failure if stopping the hardware could result in lost
    data, or if its hardware does not have a queue for any new I/O requests that
    might be dispatched to it, or if the driver was notified that it is in the
    paging path. A driver can determine if it is in the paging path by 
    processing IRP_MN_DEVICE_USAGE_NOTIFICATION sent to its device stack.

--*/
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterCanRemoveDevice    (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )

/*++

New Routine Description:
    ToasterCanRemoveDevice determines whether the hardware instance can be safely
    removed, without losing any data. In the case of an imaginary toaster, the
    hardware can always be removed without losing any data.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the IRP_MN_QUERY_REMOVE_DEVICE operation associated with the
    hardware instance described by the DeviceObject parameter.

Return Value Description:
    ToasterCanRemoveDevice returns STATUS_SUCCESS to indicate that the hardware
    instance can be removed safely, without losing any data. Because the Toaster
    hardware is imaginary, IRP_MN_QUERY_REMOVE_DEVICE always succeeds. However, in
    practice, ToasterCanRemoveDevice could return many errors to the caller. A
    driver should return failure if there are any open handles to its hardware, or
    if removal of the device could result in lost data, or the hardware does not
    have a queue for the I/O requests that might be dispatched to it, or if the
    driver was notified that it is in the paging path. A driver can determine 
    if it is in the paging path by processing IRP_MN_DEVICE_USAGE_NOTIFICATION 
    sent to its device stack.

    The PnP manager on Windows 2000 and later fails any attempt to remove hardware
    if there any open handles to it. However on Win9x, the driver must keep count of
    open handles and fail IRP_MN_QUERY_REMOVE_DEVICE if there are any open handles.

--*/
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterReturnResources (
    IN PDEVICE_OBJECT DeviceObject
    )

/*++

New Routine Description:
    The ToasterReturnResources routine returns the hardware resources used earlier
    to program the hardware instance when ToasterDispatchPnP processed
    IRP_MN_START_DEVICE and called ToasterStartDevice.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that contains the hardware
    resources used to program the hardware earlier in ToasterStartDevice.

Return Value Description:
    ToasterReturnResources returns STATUS_SUCCESS to indicate that the hardware
    resources were successfully returned. Because the Toaster hardware is
    imaginary this operation always succeeds.

--*/
{
    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // Release any hardware resources used earlier in ToasterStartDevice. Unmap any
    // I/O ports or I/O memory ranges, and disconnect from the hardware instance’s
    // interrupt, if applicable. The PCIDRV sample that is shipped with the DDK
    // demonstrates this concept.
    //

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterGetStandardInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PTOASTER_INTERFACE_STANDARD BusInterface
    )
/*++

New Routine Description:
    ToasterGetStandardInterface returns a direct-call interface to the
    functionality exported by the underlying bus driver.

    ToasterGetStandardInterface demonstrates how to create a custom IRP to pass
    down the device stack to be processed by the bus driver.

Parameters Description:
    DeviceObject
    DeviceObject represents the target device object to query for the direct-call
    interface.

    BusInterface
    BusInterface represents the interface exported by the underlying bus driver.
    The members of this pointer are filled out correctly if the bus driver
    successfully receives and processes the custom IRP.

Return Value Description:
    ToasterGetStandardInterface returns STATUS_INSUFFICIENT_RESOURCES if it cannot
    allocate memory for the custom IRP. ToasterGetStandardInterface returns
    STATUS_NOT_SUPPORTED if the bus driver fails to receive and process the custom
    IRP. Otherwise, ToasterGetStandardInterface returns the status of how the bus
    driver processed the custom IRP.

--*/
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetObject;

    ToasterDebugPrint(TRACE, "ToasterGetBusStandardInterface entered.\n");

    //
    // Initialize a kernel event to an unsignaled state. The event blocks the thread
    // that processes ToasterGetStandardInterface from returning to the caller until
    // the system signals the event when the bus driver completes the custom IRP.
    //
    KeInitializeEvent( &event, NotificationEvent, FALSE );

    //
    // Get a pointer to the device object at top of the device stack which contains
    // DeviceObject. IoGetAttachedDeviceReference increments the reference count on
    // DeviceObject. 
    //
    targetObject = IoGetAttachedDeviceReference( DeviceObject );

    //
    // Allocate an IRP to be processed synchronously by the underlying bus driver.
    // Pass IRP_MN_PNP as the major function code for the custom IRP. The system 
    // dispatches the IRP to the bus driver’s DispatchPnP routine when
    // ToasterGetStandardInterface calls IoCallDriver to pass the IRP down the device
    // stack.
    //
    // Pass the kernel event initialized earlier. The system signals the event when
    // the bus driver completes the custom IRP.
    //
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        targetObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );

    if (NULL == irp)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }

    //
    // Get a pointer to the IRP’s parameters from their location on the device stack.
    // ToasterGetStandardInterface calls IoGetNextIrpStackLocation to get a pointer
    // to the parameters for the next lower driver because that is the driver the
    // custom IRP is going to be passed down to. Higher level drivers are responsible
    // for setting up the I/O stack location for the next lower driver.
    //
    irpStack = IoGetNextIrpStackLocation( irp );

    //
    // Specify the minor function code for the custom IRP for the bus driver to
    // process.
    //
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    //
    // Specify the GUID of the direct-call interface to obtain a pointer to from the
    // bus driver. The GUID must be made public by the author of the bus driver if it
    // is intended to be used by 3rd parties.
    //
    irpStack->Parameters.QueryInterface.InterfaceType =
                        (LPGUID) &GUID_TOASTER_INTERFACE_STANDARD;

    //
    // Initialize the remaining members of the custom IRP before passing the IRP down
    // the device stack.
    //
    irpStack->Parameters.QueryInterface.Size = sizeof(TOASTER_INTERFACE_STANDARD);

    irpStack->Parameters.QueryInterface.Version = 1;

    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;

    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    //
    // Initialize the status of the custom IRP to an error in case the bus driver
    // does not receive or successfully process the IRP. That is, assume the bus
    // driver is not going to process the IRP, so initialize the IRP with an error
    // value. If the bus driver processes the IRP then the bus driver will change
    // the status to indicate success or failure.
    //
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    //
    // Pass the custom IRP to the driver at the top of the device stack. The drivers
    // in the device stack pass down unhandled IRPs, including the custom IRP until
    // it reaches the underlying bus driver where the interface is obtained.
    //
    status = IoCallDriver( targetObject, irp );

    if (STATUS_PENDING == status)
    {
        //
        // If the bus driver marked the IRP as pending, then suspend the thread that
        // is processing ToasterGetStandardInterface until the system signals the
        // kernel event initialized earlier.
        //
        // Pass KernelMode as the wait mode in the call to KeWaitForSingleObject to
        // prevent the device stack from being paged out. If the memory for the
        // kernel event was paged out when the system attempts to signal the kernel
        // event, a system crash would result.
        //
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );

        //
        // Copy the status of the custom IRP to return to the caller.
        //
        status = ioStatusBlock.Status;
    }

    if (!NT_SUCCESS( status))
    {
        ToasterDebugPrint(ERROR, "Failure status :0x%x\n", status);
    }

End:
    //
    // Decrement the outstanding reference count of the device object at the top of
    // the device stack. The reference count to that device object was incremented
    // earlier when ToasterGetStandardInterface called IoGetAttachedDeviceReference.
    //
    // Otherwise, the system cannot later delete DeviceObject because of the
    // outstanding reference count.
    //
    ObDereferenceObject( targetObject );

    return status;
}



 
LONG
ToasterIoIncrement    (
    IN  OUT PFDO_DATA   FdoData
    )

/*++

Updated Routine Description:
    ToasterIoIncrement does not change in this stage of the function driver.

--*/

{
    LONG            result;

    result = InterlockedIncrement(&FdoData->OutstandingIO);

    //ToasterDebugPrint(TRACE, "ToasterIoIncrement %d\n", result);

    ASSERT(result > 0);

    if (2 == result)
    {
        KeClearEvent(&FdoData->StopEvent);
    }

    return result;
}

 
LONG
ToasterIoDecrement    (
    IN  OUT PFDO_DATA  FdoData
    )

/*++

Updated Routine Description:
    ToasterIoDecrement does not change in this stage of the function driver.

--*/
{
    LONG            result;

    result = InterlockedDecrement(&FdoData->OutstandingIO);

    //ToasterDebugPrint(TRACE, "ToasterIoDecrement %d\n", result);

    ASSERT(result >= 0);

    if (1 == result)
    {
        KeSetEvent (&FdoData->StopEvent,
                    IO_NO_INCREMENT,
                    FALSE);
    }

    if (0 == result)
    {
        ASSERT(Deleted == FdoData->DevicePnPState);

        KeSetEvent (&FdoData->RemoveEvent,
                    IO_NO_INCREMENT,
                    FALSE);
    }

    return result;
}


 
VOID
ToasterDebugPrint    (
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    ...
    )

/*++

Updated Routine Description:
    ToasterDebugPrint does not change in this stage of the function driver.

 --*/
{
#define     TEMP_BUFFER_SIZE        1024
    va_list    list;
    UCHAR      debugMessageBuffer[TEMP_BUFFER_SIZE];
    NTSTATUS status;
    
    va_start(list, DebugMessage);
    
    if (DebugMessage)
    {
        status = RtlStringCbVPrintfA(debugMessageBuffer, sizeof(debugMessageBuffer), 
                                    DebugMessage, list);
        if(!NT_SUCCESS(status))
        {
            KdPrint ((_DRIVER_NAME_": RtlStringCbVPrintfA failed %x\n", status));
            return;
        }
        if (DebugPrintLevel <= DebugLevel)
        {
            KdPrint ((_DRIVER_NAME_": %s", debugMessageBuffer));
        }        
    }
    va_end(list);

    return;
}

 
PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
)
/*++

Updated Routine Description:
    PnPMinorFunctionString does not change in this stage of the function driver.

--*/
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
    case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
        return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";
    default:
        return "unknown_pnp_irp";
    }
}

