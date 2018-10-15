/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.
 
Module Name:

    toastmon.c

Abstract: This sample demonstrates how to register PnP event notification
          for an interface class, how to open the associated device in the 
          callback to get top level deviceobject and it's PDO, and how to  
          register for device change notification and handle the callback
          during device remove.

Author:   Eliyas Yakub  Feb 15, 2000

Environment:

    Kernel mode

Revision History:

         Added module to demonstrate how to register and receive WMI
         notification in Kernel-mode (Eliyas Yakub - 3/3/2004)

--*/
#include "toastmon.h"
#include <initguid.h>
#include <wdmguid.h>
#include "public.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ToastMon_AddDevice)
#pragma alloc_text (PAGE, ToastMon_DispatchPnp)
#pragma alloc_text (PAGE, ToastMon_DispatchPower)
#pragma alloc_text (PAGE, ToastMon_Dispatch)
#pragma alloc_text (PAGE, ToastMon_DispatchSystemControl)
#pragma alloc_text (PAGE, ToastMon_Unload)
#pragma alloc_text (PAGE, ToastMon_PnpNotifyDeviceChange)
#pragma alloc_text (PAGE, ToastMon_PnpNotifyInterfaceChange)
#pragma alloc_text (PAGE, ToastMon_GetTargetDevicePdo)
#pragma alloc_text (PAGE, ToastMon_OpenTargetDevice)
#pragma alloc_text (PAGE, ToastMon_CloseTargetDevice)
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
    
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ToastMon_Dispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ToastMon_Dispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ToastMon_Dispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]            = ToastMon_DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = ToastMon_DispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = 
                                                ToastMon_DispatchSystemControl;
    DriverObject->DriverExtension->AddDevice           = ToastMon_AddDevice;
    DriverObject->DriverUnload                         = ToastMon_Unload;

    return STATUS_SUCCESS;
}


NTSTATUS
ToastMon_AddDevice(
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

    Remember: we can NOT actually send ANY non pnp IRPS to the given driver
    stack, UNTIL we have received an IRP_MN_START_DEVICE.

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
    PDEVICE_EXTENSION       deviceExtension; 

    PAGED_CODE();

    //
    // Create a device object.
    //

    status = IoCreateDevice (DriverObject,
                             sizeof (DEVICE_EXTENSION),
                             NULL,
                             FILE_DEVICE_UNKNOWN,
                             0,
                             FALSE,
                             &deviceObject);

    
    if (!NT_SUCCESS (status)) {
        //
        // Either not enough memory to create a deviceobject or another
        // deviceobject with the same name exits. This could happen
        // if you install another instance of this device.
        //
        return status;
    }

    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
    
    deviceExtension->TopOfStack = IoAttachDeviceToDeviceStack (
                                       deviceObject,
                                       PhysicalDeviceObject);
    if(NULL == deviceExtension->TopOfStack) {
        IoDeleteDevice(deviceObject);
        return STATUS_DEVICE_REMOVED;
    }

    IoInitializeRemoveLock (&deviceExtension->RemoveLock , 
                            DRIVER_TAG,
                            1, // MaxLockedMinutes 
                            5); // HighWatermark, this parameter is 
                                // used only on checked build.
    //
    // Set the flag if the device is not holding a pagefile
    // crashdump file or hibernate file. 
    // 
    
    deviceObject->Flags |=  DO_POWER_PAGABLE;
    deviceObject->Flags |= DO_BUFFERED_IO;

    deviceExtension->DeviceObject = deviceObject;
    INITIALIZE_PNP_STATE(deviceExtension);

    //
    // We will keep list of all the toaster devices we
    // interact with and protect the access to the list
    // with a mutex.
    //
    InitializeListHead(&deviceExtension->DeviceListHead);
    ExInitializeFastMutex (&deviceExtension->ListMutex);

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    DebugPrint(("AddDevice: %p to %p->%p \n", deviceObject, 
                       deviceExtension->TopOfStack,
                       PhysicalDeviceObject));

    //
    // Register for TOASTER device interface changes. 
    //
    // We will get an ARRIVAL callback for every TOASTER device that is 
    // started and a REMOVAL callback for every TOASTER that is removed
    //

    status = IoRegisterPlugPlayNotification (
                EventCategoryDeviceInterfaceChange,            
                PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                (PVOID)&GUID_DEVINTERFACE_TOASTER,
                DriverObject,
                (PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)
                                        ToastMon_PnpNotifyInterfaceChange,
                (PVOID)deviceExtension,
                &deviceExtension->NotificationHandle);
    
    if (!NT_SUCCESS(status)) {
        DebugPrint(("RegisterPnPNotifiction failed: %x\n", status));
    }

    RegisterForWMINotification(deviceExtension); 

    return STATUS_SUCCESS;

}

NTSTATUS 
ToastMon_CompletionRoutine(
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
    PKEVENT             event;

    event = (PKEVENT) Context;

    UNREFERENCED_PARAMETER(DeviceObject);

    //
    // If the lower driver didn't return STATUS_PENDING, we don't need to 
    // set the event because we won't be waiting on it. 
    // This optimization avoids grabbing the dispatcher lock and improves perf.
    //
    if (Irp->PendingReturned == TRUE) {
        KeSetEvent(event, 0, FALSE);
    }
    //
    // Allows the caller to reuse the IRP
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
ToastMon_DispatchPnp (
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
    PDEVICE_EXTENSION           deviceExtension;
    PLIST_ENTRY                 thisEntry;
    PDEVICE_INFO                list;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    DebugPrint(("%s IRP:0x%x \n", 
                PnPMinorFunctionString(irpStack->MinorFunction), Irp));

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

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
                           (PIO_COMPLETION_ROUTINE)ToastMon_CompletionRoutine, 
                           &event,
                           TRUE,
                           TRUE,
                           TRUE); // No need for Cancel

        status = IoCallDriver(deviceExtension->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
            KeWaitForSingleObject(
               &event,
               Executive, // Waiting for reason of a driver
               KernelMode, // Waiting in kernel mode
               FALSE, // No alert
               NULL); // No timeout
            status = Irp->IoStatus.Status;
        }

        if (NT_SUCCESS(status)) {

            SET_NEW_PNP_STATE(deviceExtension, Started);
        }

        //
        // We must now complete the IRP, since we stopped it in the
        // completion routine with STATUS_MORE_PROCESSING_REQUIRED.
        //
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);       
        return status;
        
    case IRP_MN_REMOVE_DEVICE:
        

        //
        // Wait for all outstanding requests to complete
        //
        IoReleaseRemoveLockAndWait(&deviceExtension->RemoveLock, Irp);
        SET_NEW_PNP_STATE(deviceExtension, Deleted);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(deviceExtension->TopOfStack, Irp);

        //
        // Unregister the interface notification
        //
        IoUnregisterPlugPlayNotification(deviceExtension->NotificationHandle);
        //
        // Close all the handles to the target device
        //
        ExAcquireFastMutex (&deviceExtension->ListMutex);
        while(!IsListEmpty(&deviceExtension->DeviceListHead))
        {
            thisEntry = RemoveHeadList(&deviceExtension->DeviceListHead);
            list = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
            ToastMon_CloseTargetDevice(list);
        }
        ExReleaseFastMutex (&deviceExtension->ListMutex);

        UnregisterForWMINotification(deviceExtension); 

        //
        // Detach and delete our deviceobject.
        //
        IoDetachDevice(deviceExtension->TopOfStack); 
        IoDeleteDevice(DeviceObject);
        return status;
        
        
    case IRP_MN_QUERY_STOP_DEVICE:
        SET_NEW_PNP_STATE(deviceExtension, StopPending);
        status = STATUS_SUCCESS;
        break;
        
    case IRP_MN_CANCEL_STOP_DEVICE:

        //
        // Check to see whether you have received cancel-stop
        // without first receiving a query-stop. This could happen if someone
        // above us fails a query-stop and passes down the subsequent
        // cancel-stop.
        //
        
        if(StopPending == deviceExtension->DevicePnPState)
        {
            //
            // We did receive a query-stop, so restore.
            //             
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
        }
        status = STATUS_SUCCESS; // We must not fail this IRP.
        break;
        
    case IRP_MN_STOP_DEVICE:
        SET_NEW_PNP_STATE(deviceExtension, Stopped);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        SET_NEW_PNP_STATE(deviceExtension, RemovePending);
        status = STATUS_SUCCESS;
        break;
       
    case IRP_MN_SURPRISE_REMOVAL:
               
        SET_NEW_PNP_STATE(deviceExtension, SurpriseRemovePending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        //
        // Check to see whether you have received cancel-remove
        // without first receiving a query-remove. This could happen if 
        // someone above us fails a query-remove and passes down the 
        // subsequent cancel-remove.
        //
        
        if(RemovePending == deviceExtension->DevicePnPState)
        {
            //
            // We did receive a query-remove, so restore.
            //             
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
        }
        status = STATUS_SUCCESS; // We must not fail this IRP.
        break;
         
        
    default:
        //
        // If you don't handle any IRP you must leave the
        // status as is.
        //        
        status = Irp->IoStatus.Status;

        break;
    }

    //
    // Pass the IRP down and forget it.
    //
    Irp->IoStatus.Status = status;
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (deviceExtension->TopOfStack, Irp);
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);       
    return status;
}

NTSTATUS
ToastMon_DispatchPower(
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
    PDEVICE_EXTENSION   deviceExtension;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // If the device has been removed, the driver should not pass 
    // the IRP down to the next lower driver.
    //
    
    if (Deleted == deviceExtension->DevicePnPState) {
        
        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status =  STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        return STATUS_NO_SUCH_DEVICE ;
    }
    
    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(deviceExtension->TopOfStack, Irp);
}

NTSTATUS 
ToastMon_DispatchSystemControl(
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
    PDEVICE_EXTENSION   deviceExtension;

    PAGED_CODE();

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(deviceExtension->TopOfStack, Irp);
}

    
VOID
ToastMon_Unload(
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
ToastMon_Dispatch(
    IN    PDEVICE_OBJECT DeviceObject,
    IN    PIRP Irp             
    )

/*++

Routine Description:
    This routine is the dispatch handler for the driver.  It is responsible
    for processing the IRPs.

Arguments:
    
    pDO - Pointer to device object.

    Irp - Pointer to the current IRP.

Return Value:

    STATUS_SUCCESS if the IRP was processed successfully, otherwise an error
    indicating the reason for failure.

--*/

{
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

    DebugPrint (("Entered ToastMon_Dispatch\n"));

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    if (NotStarted == deviceExtension->DevicePnPState) {
        //
        // We fail all the IRPs that arrive before the device is started.
        //
        Irp->IoStatus.Status = status = STATUS_DEVICE_NOT_READY;
        IoCompleteRequest(Irp, IO_NO_INCREMENT );
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);       
        return status;
    }
    
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    // Dispatch based on major fcn code.

    switch (irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:
        case IRP_MJ_CLOSE:
            // We don't need any special processing on open/close so we'll
            // just return success.
            status = STATUS_SUCCESS;
            break;

        case IRP_MJ_DEVICE_CONTROL:
        default: 
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    //
    // We're done with I/O request.  Record the status of the I/O action.
    //
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT );
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);       
    return status;
}


NTSTATUS
ToastMon_PnpNotifyInterfaceChange(
    IN  PDEVICE_INTERFACE_CHANGE_NOTIFICATION NotificationStruct,
    IN  PVOID                        Context
    )
/*++

Routine Description:

    This routine is the PnP "interface change notification" callback routine.

    This gets called on a Toaster triggered device interface arrival or 
    removal.
      - Interface arrival corresponds to a Toaster device being STARTED
      - Interface removal corresponds to a Toaster device being REMOVED

    On arrival:
      - Get the target deviceobject pointer by using the symboliclink.
      - Get the PDO of the target device, in case you need to set the
        device parameters of the target device.
      - Regiter for EventCategoryTargetDeviceChange notification on the fileobject
        so that you can cleanup whenever associated device is removed.

    On removal:
      - This callback is a NO-OP for interface removal because we
      for PnP EventCategoryTargetDeviceChange callbacks and
      use that callback to clean up when their associated toaster device goes
      away.

Arguments:

    NotificationStruct  - Structure defining the change.

    Context -    pointer to the device extension. 
                 (supplied as the "context" when we 
                  registered for this callback)
Return Value:

    STATUS_SUCCESS - always, even if something goes wrong

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_INFO            list = NULL;
    PDEVICE_EXTENSION       deviceExtension = Context;
    PUNICODE_STRING         symbolicLinkName;
    ULONG                   len;

    PAGED_CODE();

    symbolicLinkName = NotificationStruct->SymbolicLinkName;

    //
    // Verify that interface class is a toaster device interface.
    //
    // Any other InterfaceClassGuid is an error, but let it go since
    //   it is not fatal to the machine.
    //
    
    if( !IsEqualGUID( (LPGUID)&(NotificationStruct->InterfaceClassGuid),
                      (LPGUID)&GUID_DEVINTERFACE_TOASTER) ) {
        DebugPrint(("Bad interfaceClassGuid\n"));
        return STATUS_SUCCESS;
    }

    //
    // Check the callback event.
    //
    if(IsEqualGUID( (LPGUID)&(NotificationStruct->Event), 
                     (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL )) {

        DebugPrint(("Arrival Notification\n"));

        //
        // Allocate memory for the deviceinfo
        //
        
        list = ExAllocatePoolWithTag(PagedPool,sizeof(DEVICE_INFO),DRIVER_TAG);
        if(list == NULL)
        {
            goto Error;                            
        }
        RtlZeroMemory(list, sizeof(DEVICE_INFO));

        //
        // Copy the symbolic link
        //
        list->SymbolicLink.MaximumLength = symbolicLinkName->Length +
                                          sizeof(UNICODE_NULL);
        list->SymbolicLink.Length = symbolicLinkName->Length;
        list->SymbolicLink.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       list->SymbolicLink.MaximumLength,
                                       DRIVER_TAG);  
        if(list->SymbolicLink.Buffer == NULL)
        {
            goto Error;                            
        }
        RtlCopyUnicodeString(&list->SymbolicLink,symbolicLinkName);

        list->DeviceExtension = deviceExtension;
        InitializeListHead(&list->ListEntry);

        //
        // Warning: It's not recommended to open the targetdevice
        // from a pnp notification callback routine, because if
        // the target device initiates any kind of PnP action as
        // a result of this open, the PnP manager could deadlock.
        // You should queue a workitem to do that.
        // For example, SWENUM devices in conjunction with KS
        // initiate an enumeration of a device when you do the 
        // open on the device interface. 
        // For simplicity, I'm opening the device here, because
        // I know the toaster function driver doesn't trigger
        // any pnp action. 
        // For an example on how to queue a workitem, take a look
        // at ToasterQueuePassiveLevelCallback in func\featured2\
        // wake.c file.
        // 
        
        status = ToastMon_OpenTargetDevice(list);
        if (!NT_SUCCESS( status)) {
            goto Error;
        }

        //
        // Finally queue the Deviceinfo.
        //
        ExAcquireFastMutex (&deviceExtension->ListMutex);
        InsertTailList(&deviceExtension->DeviceListHead, &list->ListEntry);
        ExReleaseFastMutex (&deviceExtension->ListMutex);     
    } else {
        DebugPrint(("Removal Interface Notification\n"));
    }
    return STATUS_SUCCESS;

 Error:
    DebugPrint(("PnPNotifyInterfaceChange failed: %x\n", status) );
    ToastMon_CloseTargetDevice(list);
    return STATUS_SUCCESS;
}

NTSTATUS
ToastMon_PnpNotifyDeviceChange(
    IN  PPLUGPLAY_NOTIFICATION_HEADER         NotificationStruct,
    IN  PVOID                                  Context
    )
/*++

Routine Description:

    This routine is the PnP "Device Change Notification" callback routine.

    This gets called on a when the target is query or surprise removed.

      - Interface arrival corresponds to a toaster device being STARTed
      - Interface removal corresponds to a toaster device being REMOVEd

    On Query_Remove or Remove_Complete:
      - Find the targetdevice from the list by matching the fileobject pointers.
      - Dereference the FileObject (this generates a  close to the target device)
      - Free the resources.

Arguments:

    NotificationStruct  - Structure defining the change.

    Context -    pointer to the device extension. 
                 (supplied as the "context" when we 
                  registered for this callback)
Return Value:

    STATUS_SUCCESS - always, even if something goes wrong

--*/
{
    NTSTATUS                status;
    PDEVICE_INFO            list = Context;
    PDEVICE_EXTENSION       deviceExtension = list->DeviceExtension;
    
    PAGED_CODE();

    //
    // if the event is query_remove
    //
    if( (IsEqualGUID( (LPGUID)&(NotificationStruct->Event),
                      (LPGUID)&GUID_TARGET_DEVICE_QUERY_REMOVE))){
        PTARGET_DEVICE_REMOVAL_NOTIFICATION removalNotification;
        
        removalNotification = 
                    (PTARGET_DEVICE_REMOVAL_NOTIFICATION)NotificationStruct;

        DebugPrint(("Device Removal (query remove) Notification\n"));

        ASSERT(list->FileObject == removalNotification->FileObject);
        //
        // Deref the fileobject so that we don't prevent
        // the target device from being removed.
        //
        ObDereferenceObject(list->FileObject);
        list->FileObject = NULL;
        //
        // Deref the PDO to compensate for the reference taken
        // by the bus driver when it returned the PDO in response
        // to the query-device-relations (target-relations).
        //
        ObDereferenceObject(list->Pdo);
        list->Pdo = NULL;
        //
        // We will defer freeing other resources to remove-complete
        // notification because if query-remove is vetoed, we would reopen 
        // the device in remove-cancelled notification.
        //
        
    } else if(IsEqualGUID( (LPGUID)&(NotificationStruct->Event),
                      (LPGUID)&GUID_TARGET_DEVICE_REMOVE_COMPLETE) ) {
        //
        // Device is gone. Let us cleanup our resources.
        //
        DebugPrint(("Device Removal (remove complete) Notification\n"));

        ExAcquireFastMutex (&deviceExtension->ListMutex);
        RemoveEntryList(&list->ListEntry);
        ExReleaseFastMutex (&deviceExtension->ListMutex);

        ToastMon_CloseTargetDevice(list);
                      
    } else if( IsEqualGUID( (LPGUID)&(NotificationStruct->Event),
                      (LPGUID)&GUID_TARGET_DEVICE_REMOVE_CANCELLED) ) {

        DebugPrint(("Device Removal (remove cancelled) Notification\n"));

        // Should be null because we cleared it in query-remove

        ASSERT(!list->FileObject);         
        ASSERT(!list->Pdo);

        //
        // Unregister the previous notification because when we reopen
        // the device we will register again on the new fileobject.
        //
        IoUnregisterPlugPlayNotification(list->NotificationHandle);

        //
        // Reopen the device
        //
        status = ToastMon_OpenTargetDevice(list);
        if (!NT_SUCCESS (status)) {
            //
            // Couldn't reopen the device. Cleanup.
            //
            ExAcquireFastMutex (&deviceExtension->ListMutex);
            RemoveEntryList(&list->ListEntry);
            ExReleaseFastMutex (&deviceExtension->ListMutex);

            ToastMon_CloseTargetDevice(list);
        }
        
    } else {
        DebugPrint(("Unknown Device Notification\n"));
    }
    
    return STATUS_SUCCESS;
}

NTSTATUS
ToastMon_GetTargetDevicePdo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_OBJECT *PhysicalDeviceObject
    )
/*++

Routine Description:

    This builds and send a pnp irp to get the PDO a device.

Arguments:

    DeviceObject - This is the top of the device in the device stack 
                   the irp is to be sent to.

   PhysicalDeviceObject - Address where the PDO pointer is returned

Return Value:

   NT status code
--*/
{

    KEVENT                  event;
    NTSTATUS                status;
    PIRP                    irp;
    IO_STATUS_BLOCK         ioStatusBlock;
    PIO_STACK_LOCATION      irpStack;
    PDEVICE_RELATIONS       deviceRelations;

    PAGED_CODE();

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );

    if (irp == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }

    irpStack = IoGetNextIrpStackLocation( irp );
    irpStack->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpStack->Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;

    //
    // Initialize the status to error in case the bus driver decides not to
    // set it correctly.
    //

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;


    status = IoCallDriver( DeviceObject, irp );

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
        status = ioStatusBlock.Status;
    }

    if (NT_SUCCESS( status)) {
        deviceRelations = (PDEVICE_RELATIONS)ioStatusBlock.Information;
        ASSERT(deviceRelations);
        //
        // You must dereference the PDO when it's no longer
        // required.
        //
        *PhysicalDeviceObject = deviceRelations->Objects[0];
        ExFreePool(deviceRelations);        
    }

End:
    return status;

}

NTSTATUS
ToastMon_OpenTargetDevice(
    PDEVICE_INFO        List
    )
/*++

Routine Description:

    Open the target device, get the PDO and register
    for TargetDeviceChange notification on the fileobject.
    
Arguments:


Return Value:

   NT status code
--*/
{
    NTSTATUS             status;

    PAGED_CODE();

    //
    // Get a pointer to and open a handle to the toaster device
    //

    status = IoGetDeviceObjectPointer(&List->SymbolicLink, 
                                      STANDARD_RIGHTS_ALL,
                                      &List->FileObject, 
                                      &List->TargetDeviceObject);
    if( !NT_SUCCESS(status) ) {
        goto Error;                            
    }

    //
    // Register for TargerDeviceChange notification on the fileobject.
    //
    status = IoRegisterPlugPlayNotification (
            EventCategoryTargetDeviceChange,            
            0,
            (PVOID)List->FileObject,
            List->DeviceExtension->DeviceObject->DriverObject,
            (PDRIVER_NOTIFICATION_CALLBACK_ROUTINE)ToastMon_PnpNotifyDeviceChange,
            (PVOID)List,
            &List->NotificationHandle);
    if (!NT_SUCCESS (status)) {
        goto Error;                            
    }      

    //
    // Get the PDO. This is required in case you need to set
    // the target device's parameters using IoOpenDeviceRegistryKey
    //
    status = ToastMon_GetTargetDevicePdo(List->TargetDeviceObject,
                                            &List->Pdo);
    if (!NT_SUCCESS (status)) {
        goto Error;                            
    }      
    
    DebugPrint(("Target device Toplevel DO:0x%x and PDO: 0x%x\n", 
                    List->TargetDeviceObject, List->Pdo));    
Error:

    return status;
}


VOID
ToastMon_CloseTargetDevice(
    PDEVICE_INFO        List
    )
/*++

Routine Description:

    Close all the handles and free the list.
    
Arguments:


Return Value:
    VOID
--*/
{
    PAGED_CODE();

    DebugPrint(("Closing handle to 0x%x\n", List->TargetDeviceObject));
    if(List->FileObject){
        ObDereferenceObject(List->FileObject);
    }
    if(List->NotificationHandle) {
        IoUnregisterPlugPlayNotification(List->NotificationHandle);
    }
    if(List->Pdo) {
        ObDereferenceObject(List->Pdo);
    }        
    RtlFreeUnicodeString(&List->SymbolicLink);
    ExFreePool(List);
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
        case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
            return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";                      
        default:
            return "unknown_pnp_irp";
    }
}

#endif


