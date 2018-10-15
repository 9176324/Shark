/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Toaster.c

Abstract:

    Featured2\Toaster.c is the fourth stage of the Toaster sample function driver.
    This stage builds on the support implemented in the third stage,
    Featured1\Toaster.c.

    New features implemented in this stage of the function driver:

    -Support for IRP_MN_WAIT_WAKE power IRPs. All the wait/wake management related
     code is located in Wake.c in this stage of the function driver.

    -Support for WPP software tracing to replace calls to ToasterDebugPrint and
     provide more information when debugging the function driver.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub - 06-Oct-1998
     -Fixed cancel bug - 11-Apr-2000
     -Added Wait-Wake Support - 02-Oct-2001
     -Added Tracing Support 01-Nov-2002

    Kevin Shirley  01-Jul-2003 - Commented for tutorial and learning purposes

--*/
#include "toaster.h"

#if defined(EVENT_TRACING)
//
// If the Featured2\source. file defined the EVENT_TRACING token when the BUILD
// utility compiles the sources to the function driver, the WPP preprocessor
// generates a trace message header (.tmh) file by the same name as the source file.
//
// The trace message header file must be included in the function driver's sources
// after defining a WPP_CONTROL_GUIDS macro (which is defined in Toaster.h) and
// before any calls to WPP macros, such as WPP_INIT_TRACING.
//
// During compilation, the WPP preprocessor examines the source files for
// DoTraceMessage() calls and builds the .tmh file, which stores a unique GUID for
// each message, the text resource string for each message, and the data types
// of the variables passed in for each message.
//
#include "toaster.tmh"
#endif

#if !defined(EVENT_TRACING)
//
// If the EVENT_TRACING token is not defined, then the function driver uses the
// ToasterDebugPrint routine instead of WPP software tracing. ToasterDebugPrint
// uses the DebugLevel variable to determine the level of debugging output to
// print.
//
ULONG DebugLevel = INFO;
#else 
//
// If the EVENT_TRACING token is defined, then declare the DebugLevel variable.
// However, it is not used by ToasterDebugPrint because ToasterDebugPrint is
// not compiled if the EVENT_TRACING token is defined.
//
ULONG DebugLevel;
#endif


GLOBALS Globals;
#define _DRIVER_NAME_ "Featured2"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ToasterAddDevice)
#pragma alloc_text (PAGE, ToasterCreate)
#pragma alloc_text (PAGE, ToasterClose)
#pragma alloc_text (PAGE, ToasterDispatchIoctl)
#pragma alloc_text (PAGE, ToasterDispatchIO)
#pragma alloc_text (PAGE, ToasterReadWrite)
#pragma alloc_text (PAGE, ToasterDispatchPnp)
#pragma alloc_text (PAGE, ToasterStartDevice)
#pragma alloc_text (PAGE, ToasterUnload)
#endif

 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Updated Routine Description:
    DriverEntry initializes WPP software tracing with the incoming DriverObject and
    RegistryPath parameters (when the function driver is compiled under Windows XP
    and later).

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;

#if !defined(WIN2K) && defined(EVENT_TRACING)
    ToasterDebugPrint(TRACE, "Entered DriverEntry of "_DRIVER_NAME_" version "
                        "with tracing enabled built on "__DATE__" at  "__TIME__ "\n");   

    //
    // On Windowx XP and later, initialize WPP software tracing by passing the
    // incoming DriverObject and RegistryPath parameters to the WPP_INIT_TRACING
    // macro. Windows 2000 requires the FDO, therefore the WPP_INIT_TRACING macro
    // is called from ToasterStartDevice.
    //
    WPP_INIT_TRACING(DriverObject, RegistryPath);
    
#else
    ToasterDebugPrint(TRACE, "Entered DriverEntry of "_DRIVER_NAME_" version built on "
                                                          __DATE__" at "__TIME__ "\n");
#endif

#if defined(WIN2K) 
    ToasterDebugPrint(TRACE, "Built in the Win2K build environment\n");
#endif 

    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;

    Globals.RegistryPath.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       Globals.RegistryPath.MaximumLength,
                                       TOASTER_POOL_TAG);

    if (NULL == Globals.RegistryPath.Buffer)
    {
        ToasterDebugPrint(ERROR,
                "Couldn't allocate pool for registry path.");

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
    ToasterAddDevice initializes the members of the device extension that are
    related to wait/wake processing in Wake.c.

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

    fdoData->DontDisplayInUI = FALSE;

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

    deviceObject->Flags |= DO_POWER_PAGABLE;
    deviceObject->Flags |= DO_BUFFERED_IO;

    fdoData->DevicePowerState = PowerDeviceD3;

    fdoData->SystemPowerState = PowerSystemWorking;

    powerState.DeviceState = PowerDeviceD3;
    PoSetPowerState ( deviceObject, DevicePowerState, powerState );

    //
    // Initialize the member of the device extension that describes the hardware
    // instance's present wake-up state to WAKESTATE_DISARMED.
    //
    fdoData->WakeState = WAKESTATE_DISARMED;

    //
    // Initialize the member of the device extension that indicates whether the
    // hardware instance can be armed to signal wake-up to FALSE.
    //
    fdoData->AllowWakeArming = FALSE;

    //
    // Initialize the function driver's current IRP_MN_WAIT_WAKE IRP pointer to
    // NULL.
    //
    fdoData->WakeIrp = NULL;

    //
    // Initialize WakeDisableEnableLock to a signaled state. Wake.c uses
    // WakeDisableEnableLock to prevent potential simultaneous arm/disarm requests
    // from causing a deadlock.
    //
    // The system clears WakeDisableEnableLock after a call to KeWaitForSingleObject
    // is satisfied, which prevents other calls to KeWaitForSingleObject from
    // returning until another thread calls KeSetEvent to signal
    // WakeDisableEnableLock again.
    //
    KeInitializeEvent( &fdoData->WakeDisableEnableLock,
                       SynchronizationEvent,
                       TRUE );

    //
    // Initialize WakeCompletedEvent to a signaled state. Wake.c uses
    // WakeCompletedEvent to flush any outstanding wait/wake power IRPs.
    //
    KeInitializeEvent( &fdoData->WakeCompletedEvent,
                       NotificationEvent,
                       TRUE );

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

 
NTSTATUS
ToasterDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchPnP arms the hardware instance to signal wake-up when
    processing IRP_MN_START_DEVICE (from ToasterStartDevice) or
    IRP_MN_CANCEL_REMOVE_DEVICE. ToasterDispatchPnP disarms the hardware instance
    from signaling wake-up when processing IRP_MN_STOP_DEVICE,
    IRP_MN_QUERY_REMOVE_DEVICE, and IRP_MN_SURPRISE_REMOVAL.

    ToasterDispatchPnP deactivates WPP software tracing when processing
    IRP_MN_REMOVE_DEVICE (if the function driver was compiled under Windows 2000).

    ToasterDispatchPnP calls ToasterAdjustCapabilities to adjust the device
    capabilities returned after the underlying bus driver completed
    IRP_MN_QUERY_CAPABILITIES.

--*/
{
    PFDO_DATA               fdoData;
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
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
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status) && StopPending == fdoData->DevicePnPState)
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            fdoData->QueueState = AllowRequests;

            ASSERT(fdoData->DevicePnPState == Started);

            ToasterProcessQueuedRequests(fdoData);
        }
        break;

    case IRP_MN_STOP_DEVICE:
        SET_NEW_PNP_STATE(fdoData, Stopped);

        status = ToasterReturnResources(DeviceObject);

        //
        // Disarm the hardware instance from signaling wake-up because the hardware
        // instance must stop using the hardware resources assigned to it.
        //
        ToasterDisarmWake(fdoData, TRUE);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        status = ToasterCanRemoveDevice(DeviceObject, Irp);

        if (NT_SUCCESS(status))
        {
            SET_NEW_PNP_STATE(fdoData, RemovePending);

            fdoData->QueueState = HoldRequests;

            ToasterDebugPrint(INFO, "Query - remove holding requests...\n");

            ToasterIoDecrement(fdoData);

            //
            // Disarm the hardware instance from signaling wake-up because the
            // hardware instance is potentially going to be removed.
            //
            // The hardware instance is re-armed to signal wake-up if the function
            // driver receives a subsequent IRP_MN_CANCEL_REMOVE_DEVICE.
            //
            ToasterDisarmWake(fdoData, TRUE);

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
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status) && RemovePending == fdoData->DevicePnPState)
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            fdoData->QueueState = AllowRequests;

            ToasterProcessQueuedRequests(fdoData);

            //
            // Arm the hardware instance to signal wake-up, if the Registry
            // indicates the hardware instance support wait/wake.
            //
            ToasterArmForWake(fdoData, TRUE);
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

        ToasterReturnResources(DeviceObject);

        //
        // Disarm the hardware instance from signaling wake-up because the hardware
        // instance has been surprise removed from the system.
        //
        ToasterDisarmWake(fdoData, TRUE);

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

            ToasterReturnResources(DeviceObject);
            
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

#if defined(WIN2K) && defined(EVENT_TRACING)
        //
        // On Windows 2000, deactivate WPP software tracing by passing the FDO to the
        // WPP_CLEANUP macro. ON Windows XP and later, deactivate WPP software
        // tracing by passing the DriverObject to the WPP_CLEANUP macro in
        // ToasterUnload.
        //
        WPP_CLEANUP(fdoData->Self);
        ToasterDebugPrint(TRACE,
            "Cleaned up software tracing (%p)\n", fdoData->Self);
#endif 

        IoDetachDevice (fdoData->NextLowerDriver);

        RtlFreeUnicodeString(&fdoData->InterfaceName);

        IoDeleteDevice (fdoData->Self);

        return status;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        if(TRUE == fdoData->DontDisplayInUI)
        {
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
ToasterCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Updated Routine Description:
    ToasterCreate does not change in this stage of the function driver.

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

    status = ToasterGetStandardInterface(fdoData->NextLowerDriver, &busInterface);

    if(NT_SUCCESS(status))
    {
        UCHAR powerlevel;

        (*busInterface.GetCrispinessLevel)(busInterface.Context, &powerlevel);
        (*busInterface.SetCrispinessLevel)(busInterface.Context, 8);
        (*busInterface.IsSafetyLockEnabled)(busInterface.Context);

        (*busInterface.InterfaceDereference)((PVOID)busInterface.Context);
    }

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
    ToasterDispatchIoctl does not change in this stage of the function driver.

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
ToasterStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP             Irp
    )
/*++

Updated Routine Description:
    ToasterStartDevice initializes WPP software tracing with the FDO and the
    function driver's Registry path (when the function driver is compiled under
    Windows 2000).

    ToasterStartDevice arms the hardware instance to signal wake-up.

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

    stack = IoGetCurrentIrpStackLocation (Irp);

    PAGED_CODE();

    if ((NULL != stack->Parameters.StartDevice.AllocatedResources) &&
        (NULL != stack->Parameters.StartDevice.AllocatedResourcesTranslated))
    {
        partialResourceList =
        &stack->Parameters.StartDevice.AllocatedResources->List[0].PartialResourceList;

        resource = &partialResourceList->PartialDescriptors[0];

        partialResourceListTranslated =
        &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;

        resourceTrans = &partialResourceListTranslated->PartialDescriptors[0];

        for (i = 0;
             i < partialResourceList->Count;
             i++, resource++, resourceTrans++)
        {

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

                break;

            case CmResourceTypeInterrupt:
                break;

            default:
                ToasterDebugPrint(ERROR, "Unhandled resource type (0x%x)\n", resource->Type);

                break;
            }
        }
    }

    FdoData->DevicePowerState = PowerDeviceD0;

    powerState.DeviceState = PowerDeviceD0;

    PoSetPowerState ( FdoData->Self, DevicePowerState, powerState );

    status = IoSetDeviceInterfaceState(&FdoData->InterfaceName, TRUE);

    if (!NT_SUCCESS (status))
    {
        ToasterDebugPrint(ERROR, "IoSetDeviceInterfaceState failed: 0x%x\n",
                                status);
        return status;
    }

    if(Stopped != FdoData->DevicePnPState)
    { 
        status = ToasterWmiRegistration(FdoData);

        if (!NT_SUCCESS (status))
        {
            ToasterDebugPrint(ERROR, "IoWMIRegistrationControl failed (%x)\n", status);
            return status;
        }
        
#if defined(WIN2K) && defined(EVENT_TRACING)
        //
        // On Windows 2000, initialize WPP software tracing by passing the FDO 
        // and the function driver's Registry path to the WPP_INIT_TRACING 
        // macro. Windows XP and later initializes WPP software tracing in 
        // DriverEntry.
        //
        WPP_INIT_TRACING(FdoData->Self, &Globals.RegistryPath);
        ToasterDebugPrint(TRACE,
                "StartDevice: Enabled software tracing (%p)\n", FdoData->Self);
#endif  

    }
    
    SET_NEW_PNP_STATE(FdoData, Started);

    FdoData->QueueState = AllowRequests;

    //
    // In order to arm for wake, the function driver must query for the hardware
    // instance's power capabilities.
    //
    status = ToasterGetDeviceCapabilities(FdoData->Self, &FdoData->DeviceCaps);
    if (!NT_SUCCESS (status) )
    {
        ToasterDebugPrint(ERROR, "ToasterGetDeviceCapabilities failed (%x)\n", status);

        return status;
    }
    
    //
    // Adjust the capabilities to handle device Wake.
    //
    ToasterAdjustCapabilities(&FdoData->DeviceCaps);
    
    ToasterProcessQueuedRequests(FdoData);

    ToasterFireArrivalEvent(FdoData->Self);

    //
    // Arm the hardware instance to signal wake-up, if the Registry indicates the
    // hardware instance support wait/wake.
    //
    ToasterArmForWake(FdoData, TRUE);

    return status;
}


 
NTSTATUS
ToasterCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Updated Routine Description:
    ToasterCleanup does not change in this stage of the function driver.

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

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    InitializeListHead(&cleanupList);

    KeAcquireSpinLock(&fdoData->QueueLock, &oldIrql);

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
            RemoveEntryList(thisEntry);

            if(NULL == IoSetCancelRoutine (pendingIrp, NULL))
            {
                InitializeListHead(thisEntry);
            }
            else
            {
                InsertTailList(&cleanupList, thisEntry);
            }
        }
    }

    KeReleaseSpinLock(&fdoData->QueueLock, oldIrql);

    while(!IsListEmpty(&cleanupList))
    {
        thisEntry = RemoveHeadList(&cleanupList);

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP,
                                Tail.Overlay.ListEntry);

        pendingIrp->IoStatus.Information = 0;

        pendingIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }

    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    ToasterIoDecrement (fdoData);

    return STATUS_SUCCESS;
}





 
VOID
ToasterUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Updated Routine Description:
    ToasterUnload deactivates WPP software tracing (if the function driver was
    compiled under Windows XP and later).

--*/
{
    PAGED_CODE();

    ASSERT(DriverObject->DeviceObject == NULL);

    ToasterDebugPrint(TRACE, "unload\n");

    if(NULL != Globals.RegistryPath.Buffer)
    {
        ExFreePool(Globals.RegistryPath.Buffer);
    }

#if !defined(WIN2K) && defined(EVENT_TRACING)
    //
    // On Windows XP an later, deactivate WPP software tracing by passing the
    // DriverObject to the WPP_CLEANUP macro. On Windows 2000, deactivate WPP
    // software tracing by passing the FDO to the WPP_CLEANUP macro when processing
    // IRP_MN_REMOVE_DEVICE.
    //
    WPP_CLEANUP(DriverObject);
#endif

    return;
}



 
NTSTATUS
ToasterQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    )

/*++

Updated Routine Description:

--*/
{

    KIRQL               oldIrql;
    PDRIVER_CANCEL      ret;
    
    ToasterDebugPrint(TRACE, "Queuing Requests\n");

    ASSERT(HoldRequests == FdoData->QueueState);

    IoMarkIrpPending(Irp);

    KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

    IoSetCancelRoutine (Irp, ToasterCancelQueued);

    if(TRUE == Irp->Cancel)
    {
        ret = IoSetCancelRoutine (Irp, NULL);

        //
        // Initialize the driver-managed IRP queue's list head to prevent
        // ToasterCancelQueued from potentially causing a system crash if the system
        // has already called it. The list head must be initialized because the
        // incoming IRP is not added to the driver-managed IRP queue, but when
        // ToasterQueueRequest releases the spin lock, QueueLock, a different thread
        // attempting to execute ToasterCancelQueued might resume and attempt to
        // remove the IRP that is not present in the queue, resulting in a system
        // crash. Therefore, initializing the list head allows ToasterCancelQueued
        // to proceed, and not attempt to remove the IRP that is not present in the
        // IRP queue, without resulting in a system crash.
        //
        
        InitializeListHead(&Irp->Tail.Overlay.ListEntry);

        KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

        if(NULL != ret)
        {
            Irp->IoStatus.Status = STATUS_CANCELLED;

            Irp->IoStatus.Information = 0;

            IoCompleteRequest (Irp, IO_NO_INCREMENT);

            //
            // Fall-through to the call to ToasterIoDecrement. ToasterQueueRequest
            // then returns STATUS_PENDING even though the IRP has been cancelled
            // because a routine must return STATUS_PENDING if it calls
            // IoMarkIrpPending.
            //
        }
        else
        {
            //
            // The system has already called ToasterCancelQueued. Fall-through to the
            // call to ToasterIoDecrement. ToasterQueueRequest then returns
            // STATUS_PENDING even though the IRP has been cancelled because a routine
            // must return STATUS_PENDING if it calls IoMarkIrpPending.
            //
        }
    }
    else
    {     
        //
        // The incoming IRP has not been canceled, therefore add it to the end of
        // the driver-managed IRP queue.
        //
        InsertTailList(&FdoData->NewRequestsQueue,
                                                &Irp->Tail.Overlay.ListEntry);

        KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
    }

    ToasterIoDecrement(FdoData);

    return STATUS_PENDING;
}


 
VOID
ToasterProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    )

/*++

Updated Routine Description:
    ToasterProcessQueuedRequests does not change in this stage of the function
    driver.

--*/
{

    KIRQL               oldIrql;
    PIRP                nextIrp, cancelledIrp;
    PLIST_ENTRY         listEntry;
    LIST_ENTRY          cancelledIrpList;
    PDRIVER_CANCEL      cancelRoutine= NULL;

    ToasterDebugPrint(TRACE, "Process or fail queued Requests\n");

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

        cancelRoutine = IoSetCancelRoutine (nextIrp, NULL);

        if (TRUE == nextIrp->Cancel)
        {
            if(NULL != cancelRoutine)
            {
                InsertTailList(&cancelledIrpList, listEntry);
            }
            else
            {
                InitializeListHead(listEntry);
            }

            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
        }
        else
        {
            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            if (FailRequests == FdoData->QueueState)
            {
                nextIrp->IoStatus.Information = 0;

                nextIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

                IoCompleteRequest (nextIrp, IO_NO_INCREMENT);
            }
            else
            {
                if(STATUS_PENDING == ToasterDispatchIO(FdoData->Self, 
                                            nextIrp) )
                {                    
                    break;
                }
            }
        }
    }

    while(!IsListEmpty(&cancelledIrpList))
    {
        PLIST_ENTRY cancelledListEntry = RemoveHeadList(&cancelledIrpList);

        cancelledIrp = CONTAINING_RECORD(cancelledListEntry, IRP,
                                Tail.Overlay.ListEntry);

        cancelledIrp->IoStatus.Information = 0;

        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(cancelledIrp, IO_NO_INCREMENT);
    }

    return;
}


 
VOID
ToasterCancelQueued (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

/*++

Updated Routine Description:
    This stage of the function driver optimizes the IRP cancellation processing
    By reducing the IRQL transition compared with the Featured1 stage.
 
--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Canceling Requests\n");

    //
    // Instead of releasing the cancel spin lock and passing Irp->CancelIrql, this
    // stage of the function driver passes DISPATCH_LEVEL because ToasterCancelQueued
    // acquires the cancel spin lock again later.
    //
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

    //
    // Acquire the driver-managed IRP queue spinlock and take advantage of the
    // fact that the system calls ToasterCancelQueued at DPC level.
    //
    KeAcquireSpinLockAtDpcLevel (&fdoData->QueueLock);

    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    //
    // Release the spinlock but stay at DISPATCH_LEVEL
    //
    KeReleaseSpinLockFromDpcLevel (&fdoData->QueueLock);

    //
    // Lower the system's IRQL to the level it was was before the system called
    // ToasterCancelQueued.
    //
    KeLowerIrql(Irp->CancelIrql);

    //
    // Note that the above two calls could also be collapsed into a single call
    // to KeReleaseSpinLock(&fdoData->QueueLock, Irp->CancelIrql);
    //
    Irp->IoStatus.Status = STATUS_CANCELLED;

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

Updated Routine Description:
    ToasterCanStopDevice does not change in this stage of the function driver.

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

Updated Routine Description:
    ToasterCanRemoveDevice does not change in this stage of the function driver.

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

Updated Routine Description:
    ToasterReturnResources does not change in this stage of the function driver.

--*/
{
    UNREFERENCED_PARAMETER(DeviceObject);

    return STATUS_SUCCESS;
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


 
NTSTATUS
ToasterGetStandardInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PTOASTER_INTERFACE_STANDARD BusInterface
    )
/*++

Updated Routine Description:
    ToasterGetStandardInterface does not change in this stage of the function
    driver.

--*/
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetObject;

    ToasterDebugPrint(TRACE, "ToasterGetBusStandardInterface entered.\n");

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    targetObject = IoGetAttachedDeviceReference( DeviceObject );

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

    irpStack = IoGetNextIrpStackLocation( irp );

    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    irpStack->Parameters.QueryInterface.InterfaceType =
                        (LPGUID) &GUID_TOASTER_INTERFACE_STANDARD;

    irpStack->Parameters.QueryInterface.Size = sizeof(TOASTER_INTERFACE_STANDARD);

    irpStack->Parameters.QueryInterface.Version = 1;

    irpStack->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;

    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    status = IoCallDriver( targetObject, irp );

    if (STATUS_PENDING == status)
    {
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );

        status = ioStatusBlock.Status;
    }

    if (!NT_SUCCESS( status))
    {
        ToasterDebugPrint(ERROR, "Failure status :0x%x\n", status);
    }

End:
    ObDereferenceObject( targetObject );

    return status;
}


NTSTATUS
ToasterGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
/*++

New Routine Description:
    ToasterGetDeviceCapabilities sends an IRP_MN_QUERY_CAPABILITIES to the device
    stack to obtain the device capabilities of the hardware instance.

Arguments Description:
    DeviceObject
    DeviceObject represents the hardware instance to query for device capabilities.
    DeviceObject is an FDO created earlier in ToasterAddDevice.

    DeviceCapabilites
    DeviceCapabilities represents the capabilities, including the power capabilities
    Of the hardware instance.

Return Value Description:
    ToasterGetDeviceCapabilities returns STATUS_SUCCESS. Otherwise, if
    ToasterGetDeviceCapabilities cannot build the query IRP, it returns
    STATUS_INSUFFICIENT_RESOURCES.

--*/
{
    IO_STATUS_BLOCK     ioStatus;
    KEVENT              pnpEvent;
    NTSTATUS            status;
    PDEVICE_OBJECT      targetObject;
    PIO_STACK_LOCATION  irpStack;
    PIRP                pnpIrp;

    PAGED_CODE();

    //
    // Initialize the capabilities that the function driver passes down the device
    // stack.
    //
    RtlZeroMemory( DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = -1;
    DeviceCapabilities->UINumber = -1;

    //
    // Initialize a kernel event to block the thread executing
    // ToasterGetDeviceCapabilities until the underlying bus driver completes
    // the query IRP.
    //
    KeInitializeEvent( &pnpEvent, NotificationEvent, FALSE );

    targetObject = IoGetAttachedDeviceReference( DeviceObject );

    //
    // Build the IRP_MN_QUERY_CAPABILITIES IRP.
    //
    pnpIrp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                            targetObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &pnpEvent,
                                            &ioStatus
                                            );
    if (NULL == pnpIrp)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto End;
    }

    //
    // All PnP IRPs must be initialized with a status of STATUS_NOT_SUPPORTED.
    //
    pnpIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    //
    // Get the driver at the top of hardware instance's device stack.
    //
    irpStack = IoGetNextIrpStackLocation( pnpIrp );

    //
    // Initialize the query IRP.
    //
    RtlZeroMemory( irpStack, sizeof(IO_STACK_LOCATION ) );
    irpStack->MajorFunction = IRP_MJ_PNP;
    irpStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    irpStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    //
    // Pass the query IRP to the driver at the top of the device stack. The query
    // IRP will be passed down the device stack until the underlying bus driver
    // completes it.
    //
    status = IoCallDriver( targetObject, pnpIrp );
    if (STATUS_PENDING == status)
    {
        //
        // Block the execution of the thread executing ToasterGetDeviceCapabilities
        // until the underlying bus driver completes the query IRP.
        // Pass KernelMode in the WaitMode parameter to KeWaitForSingleObject to
        // prevent the kernel from paging out the thread's local call stack which
        // contains pnpEvent.
        //
        status = KeWaitForSingleObject(&pnpEvent, Executive, KernelMode,
                                        FALSE, NULL);
        ASSERT(NT_SUCCESS(status));

        status = ioStatus.Status;
    }

End:
    //
    // Decrement the reference count on the driver at the top of the hardware
    // instance's device stack. The reference count was incremented earlier in
    // the call to IoGetAttachedDeviceReference.
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

 
//
// If the EVENT_TRACING token is not defined, then the function driver uses the
// ToasterDebugPrint routine instead of WPP software tracing. Otherwise, there is
// no need to compile ToasterDebugPrint.
//
#if !defined(EVENT_TRACING)

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

#endif 

 
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



