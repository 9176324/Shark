/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Event.c

Abstract:

    The purpose of this sample is to demonstrate how a kernel-mode driver can notify 
    an user-app about a device event.  There are several different techniques. This sample 
    will demonstrate two very commonly used techniques.

    1) Using an event: 
        The application creates an event object using CreateEvent().
        The app passes the event handle to the driver in a private IOCTL.
        The driver is running in the app's thread context during the IOCTL so 
        there is a valid user-mode handle at that time.
        The driver dereferences the user-mode handle into system space & saves 
        the event object pointer for later use.
        The driver signals the event via KeSetEvent() at IRQL <= DISPATCH_LEVEL.
        The driver deletes the references to the event object.

    2) Pending Irp: This technique is useful if you want to send a message
        to the app along with the notification. In this, an application sends
        a synchronous or asynchronous (overlapped) ioctl to the driver. The driver
        would then pend the IRP until the device event occurs. When the hardware
        event occurs, the driver will complete the IRP. This will cause the thread that
        sent the request to come out of DeviceIoControl call if it's  synchronous or signal 
        the event that the thread is waiting on in the usermode it's has done a 
        OVERLAPPED call. Another advantage of this technique over the event model
        is that the driver doesn't have to be in the context of the process that 
        sent the IOCTL request. You can't guarantee the process context in multi-level
        drivers. 

    3) Using WMI to fire events. Check the wmifilter sample in the DDK.

    4) Using PNP custom notification scheme. Walter Oney's book describes this.
        Can be used only in PNP drivers.

    4) Named events: In that an app creates a named event in the usermode
        and driver opens that in kernel and signal it. This technique is deprecated
        by the kb article (Q228785)

        This sample demonstrates the first two techniques. This sample is an 
        improvised version of the event sample available in the KB article
        Q176415
        
Author:

    Eliyas Yakub        11/20/2001

Enviroment:

    Kernel Mode Only

Revision History:

--*/

#include <ntddk.h>
#include "public.h" //common to app and driver
#include "event.h" // private to driver

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, EventCreateClose)
#pragma alloc_text (PAGE, EventUnload)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This routine gets called by the system to initialize the driver.

Arguments:

    DriverObject    - the system supplied driver object.
    RegistryPath    - the system supplied registry path for this driver.

Return Value:

    NTSTATUS

--*/

{

    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   deviceExtension;
    UNICODE_STRING      ntDeviceName;
    UNICODE_STRING      symbolicLinkName;
    NTSTATUS            status;


    DebugPrint(("==>DriverEntry\n"));

    //
    // Create the device object
    //
    RtlInitUnicodeString( &ntDeviceName, NTDEVICE_NAME_STRING );

    status = IoCreateDevice( 
                DriverObject,               // DriverObject 
                sizeof( DEVICE_EXTENSION ), // DeviceExtensionSize
                &ntDeviceName,              // DeviceName
                FILE_DEVICE_UNKNOWN,        // DeviceType
                FILE_DEVICE_SECURE_OPEN,  // DeviceCharacteristics
                FALSE,                       // Not Exclusive
                &deviceObject              // DeviceObject
                );

    if ( !NT_SUCCESS(status) ) {
        DebugPrint(("\tIoCreateDevice returned 0x%x\n", status));
        return( status );
    }

    //
    // Set up dispatch entry points for the driver.
    //
    DriverObject->MajorFunction[IRP_MJ_CREATE]          =
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = EventCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]           = EventCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = EventDispatchIoControl;
    DriverObject->DriverUnload = EventUnload;

    //
    // Create a symbolic link for userapp to interact with the driver.
    //
    RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
    status = IoCreateSymbolicLink( &symbolicLinkName, &ntDeviceName );

    if ( !NT_SUCCESS(status) ) {
        
        IoDeleteDevice( deviceObject );
        DebugPrint(("\tIoCreateSymbolicLink returned 0x%x\n", status));
        return( status );
    }

    //
    // Initialize the device extension.
    //
    deviceExtension = deviceObject->DeviceExtension;

    InitializeListHead(&deviceExtension->EventQueueHead);

    KeInitializeSpinLock(&deviceExtension->QueueLock);

    deviceExtension->Self = deviceObject;
    
    //
    // Establish user-buffer access method.
    //
    deviceObject->Flags |= DO_BUFFERED_IO;

    DebugPrint(("<==DriverEntry\n"));

    return( status );
}

NTSTATUS
EventUnload(
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This routine gets called to remove the driver from the system.

Arguments:

    DriverObject    - the system supplied driver object.

Return Value:

    NTSTATUS

--*/

{

    PDEVICE_OBJECT       deviceObject = DriverObject->DeviceObject;
    PDEVICE_EXTENSION    deviceExtension = deviceObject->DeviceExtension;
    
    UNICODE_STRING      symbolicLinkName;

    DebugPrint(("==>Unload\n"));

   if(!IsListEmpty(&deviceExtension->EventQueueHead)) {
        ASSERTMSG("Event Queue is not empty\n", FALSE);
    }

    //
    // Delete the user-mode symbolic link.
    //
    RtlInitUnicodeString( &symbolicLinkName, SYMBOLIC_NAME_STRING );
    IoDeleteSymbolicLink( &symbolicLinkName );

    // Delete the DeviceObject
    
    IoDeleteDevice( deviceObject );

    return( STATUS_SUCCESS );
}

NTSTATUS
EventCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This device control dispatcher handles create & close IRPs.

Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

Return Value:

    NTSTATUS

--*/
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS            status ;

    irpStack = IoGetCurrentIrpStackLocation( Irp );
    
    switch(irpStack->MajorFunction )
    {
        case IRP_MJ_CREATE:
            DebugPrint(("IRP_MJ_CREATE\n"));
            status = STATUS_SUCCESS;
            break;

        case IRP_MJ_CLOSE:
            DebugPrint(("IRP_MJ_CLOSE\n"));
            status = STATUS_SUCCESS;
            break;

        default:
            ASSERT(FALSE);  // should never hit this
            status = STATUS_NOT_IMPLEMENTED;
            break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
    
}


NTSTATUS
EventCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This device control dispatcher handles Cleanup IRP.

Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

Return Value:

    NTSTATUS

--*/
{
    PIO_STACK_LOCATION irpStack;
    NTSTATUS            status ;
    KIRQL                  oldIrql;
    PLIST_ENTRY            thisEntry, nextEntry, listHead;
    PNOTIFY_RECORD notifyRecord;
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    LIST_ENTRY             cleanupList;
    PIRP    pendingIrp;
    
     DebugPrint(("==>EventCleanup\n"));

    irpStack = IoGetCurrentIrpStackLocation( Irp );
    InitializeListHead(&cleanupList);

    //
    // Walk the list and remove all the pending notification records
    // that belong to this filehandle.
    //

    KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

    listHead = &deviceExtension->EventQueueHead;

    for(thisEntry = listHead->Flink,nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry,nextEntry = thisEntry->Flink)
    {

        notifyRecord = CONTAINING_RECORD(thisEntry, NOTIFY_RECORD, ListEntry);
        
        if (irpStack->FileObject == notifyRecord->FileObject)  {

            //
            // We will free the record only if the we are able to cancel the timer. If the
            // KeCancelTimer returns false, it means the DPC has been either
            // delivered or in the process of being delivered (timerDpc routine
            // could be waiting on the lock that we are holding), so don't
            // remove the record. It will be done by the DPC.
            //
            if(KeCancelTimer( &notifyRecord->Timer )) {
                
                DebugPrint(("\tCanceled timer\n"));
                RemoveEntryList(thisEntry);         
                if(IRP_BASED == notifyRecord->Type) {
                    //
                    // Reset the DriverContext to make sure that the cancel routine
                    // doesn't touch the IRP even if it is called prior to we calling 
                    // IoSetCancelRoutine.
                    //
                    InterlockedExchangePointer(&Irp->Tail.Overlay.DriverContext[3], NULL);
                    IoSetCancelRoutine (notifyRecord->PendingIrp, NULL);
                    //
                    // Queue this IRP in the cleauplist. This is done to make sure that we don't
                    // call IoCompleteRequest while holding our queue lock to avoid potential
                    // race conditions in case the completion routine of the driver above us re-enters
                    // our driver.
                    //
                    InsertTailList(&cleanupList, &notifyRecord->PendingIrp->Tail.Overlay.ListEntry); 
                    
                } else { //Event based 
                    ObDereferenceObject( notifyRecord->Event );
                }               
                ExFreePool(notifyRecord);
            }
        }
    }

    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

    //
    // Walk through the cleanup list and cancel all
    // the IRPs.
    //

    while(!IsListEmpty(&cleanupList))
    {
        //
        // Complete the IRP 
        //

        thisEntry = RemoveHeadList(&cleanupList);
        pendingIrp = CONTAINING_RECORD(thisEntry, IRP,
                                Tail.Overlay.ListEntry);
        DebugPrint (("\t canceled IRP %p\n", pendingIrp));              
        pendingIrp->IoStatus.Information = 0;
        pendingIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }

    //
    // Finally complete the cleanup Irp
    //

    Irp->IoStatus.Status = status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    DebugPrint(("<== EventCleanup\n"));
    return status;

}
NTSTATUS
EventDispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This device control dispatcher handles IOCTLs.

Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

Return Value:

    NTSTATUS

--*/

{

    PDEVICE_EXTENSION   deviceExtension;
    PIO_STACK_LOCATION irpStack;
    PREGISTER_EVENT           registerEvent;
    NTSTATUS            status ;
    PNOTIFY_RECORD notifyRecord;
    

    DebugPrint(("==> EventDispatchIoControl\n"));

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation( Irp );

    switch( irpStack->Parameters.DeviceIoControl.IoControlCode )
    {
        case IOCTL_REGISTER_EVENT:

            DebugPrint(("\tIOCTL_REGISTER_EVENT\n"));

            //
            // First validate the parameters.
            //
            if ( irpStack->Parameters.DeviceIoControl.InputBufferLength <  SIZEOF_REGISTER_EVENT ) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            registerEvent = (PREGISTER_EVENT)Irp->AssociatedIrp.SystemBuffer;
            
            if(IRP_BASED == registerEvent->Type) {
                
                status = RegisterIrpBasedNotification(DeviceObject, Irp);
                
            }else if (EVENT_BASED == registerEvent->Type){
            
                status = RegisterEventBasedNotification(DeviceObject, Irp);
                
            } else {
                ASSERTMSG("\tUnknow notification type from user-mode\n", FALSE);
                status = STATUS_INVALID_PARAMETER;
            }
    
           break;

        default:
            ASSERT(FALSE);  // should never hit this
            status = STATUS_NOT_IMPLEMENTED;
            break;

    } // switch IoControlCode

    if(status != STATUS_PENDING) {
        //
        // complete the Irp
        //
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    DebugPrint(("<== EventDispatchIoControl\n"));
    return status;
    
}

VOID
EventCancelRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

/*++

Routine Description:

    The cancel routine. It will remove the IRP from the queue
    and will complete it. The cancel spin lock is already acquired
    when this routine is called. This routine is not required if 
    you are just using the event based notification.

Arguments:

    DeviceObject - pointer to the device object.

    Irp - pointer to the IRP to be cancelled.


Return Value:

    VOID.

--*/
{
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;
    KIRQL               oldIrql ;
    BOOLEAN fCancelTheIrp = FALSE;
    PNOTIFY_RECORD notifyRecord;
    PLIST_ENTRY            thisEntry, nextEntry, listHead;
           
    DebugPrint (("==>EventCancelRoutine %p\n", Irp));

    //
    // Release the cancel spinlock
    //

    IoReleaseCancelSpinLock( Irp->CancelIrql);

    //
    // Acquire the queue spinlock 
    //
    KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

    notifyRecord = InterlockedExchangePointer(&Irp->Tail.Overlay.DriverContext[3], NULL);
    if(NULL == notifyRecord) {
        //
        // TimerDPC or Cleanup routine is in the process of completing this IRP.
        // So don't touch it.
        //

    } else {
            //
            // Remove the entry from the list and cancel the IRP only
            // if we are successful in cancelling the timer. Otherwise
            // the timerDpc will take care.
            //
            if(KeCancelTimer( &notifyRecord->Timer )) {
                
                ASSERT(IRP_BASED == notifyRecord->Type);
                
                DebugPrint(("\tCanceled timer\n"));
                fCancelTheIrp = TRUE;
                RemoveEntryList(&notifyRecord->ListEntry);                      

                ExFreePool(notifyRecord);
                
            } else {
                // Timer DPC is waiting to acquire the lock.
            }
    }
        
    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

    if(fCancelTheIrp) {    
        DebugPrint (("\t canceled IRP %p\n", Irp));
        Irp->IoStatus.Status = STATUS_CANCELLED;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    DebugPrint (("<==EventCancelRoutine %p\n", Irp));
    return;

}


VOID
CustomTimerDPC(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This is the DPC associated with this drivers Timer object setup in ioctl routine.

Arguments:

    Dpc             -   our DPC object associated with our Timer
    DeferredContext -   Context for the DPC that we setup in DriverEntry
    SystemArgument1 -
    SystemArgument2 -

Return Value:

    Nothing.

--*/

{

    PNOTIFY_RECORD      notifyRecord = DeferredContext;
    PDEVICE_EXTENSION deviceExtension;

    DebugPrint(("==> CustomTimerDPC \n"));

    ASSERT(notifyRecord); // can't be NULL

    deviceExtension = notifyRecord->DeviceExtension;
    
    KeAcquireSpinLockAtDpcLevel(&deviceExtension->QueueLock);

    RemoveEntryList(&notifyRecord->ListEntry);           

    if(IRP_BASED == notifyRecord->Type) {
        //
        // Clear the DriverContext so that CancelRoutine wouldn't cancel
        // this IRP.
        //
        InterlockedExchangePointer(
                                &notifyRecord->PendingIrp->Tail.Overlay.DriverContext[3],  
                                NULL);
    }

    KeReleaseSpinLockFromDpcLevel(&deviceExtension->QueueLock);

    if(IRP_BASED == notifyRecord->Type) {

        //
        // Even if the cancel routine is called at this point, it wouldn't
        // complete the IRP becuase we have cleared the DriverContext[3]
        //
        
        IoSetCancelRoutine (notifyRecord->PendingIrp, NULL);
        
        //
        // complete the Irp
        //

        notifyRecord->PendingIrp->IoStatus.Status = STATUS_SUCCESS;
        notifyRecord->PendingIrp->IoStatus.Information = 0;
        IoCompleteRequest( notifyRecord->PendingIrp, IO_NO_INCREMENT );
        
    }else {
        //
        // Signal the Event created in user-mode
        // Do not call KeSetEvent from your ISR;
        // you must call it at IRQL <= DISPATCH_LEVEL.
        // Your ISR should queue a DPC and the DPC can
        // then call KeSetEvent on the ISR's behalf.
        //
        KeSetEvent((PKEVENT)notifyRecord->Event,// Event
                    0,                                   // Increment
                    FALSE                                // Wait
                    );

        //
        // Dereference the object as we are done with it.
        //
        ObDereferenceObject( notifyRecord->Event );
    }

    ExFreePool(notifyRecord);

    DebugPrint(("<== CustomTimerDPC\n"));

    return;
}


NTSTATUS
RegisterIrpBasedNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

        This routine  queues a IRP based notification record to be 
        handled by a DPC.


Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

Return Value:

    NTSTATUS

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    PNOTIFY_RECORD notifyRecord;
    PIO_STACK_LOCATION irpStack;
    KIRQL   oldIrql;
    PREGISTER_EVENT registerEvent;

    DebugPrint(("\tRegisterIrpBasedNotification\n"));
    
    irpStack = IoGetCurrentIrpStackLocation( Irp );   
    deviceExtension = DeviceObject->DeviceExtension;
    registerEvent = (PREGISTER_EVENT)Irp->AssociatedIrp.SystemBuffer;

    //
    // Allocate a record and save all the event context.
    //

    notifyRecord = ExAllocatePoolWithTag(NonPagedPool, sizeof(NOTIFY_RECORD), TAG);

    if(NULL == notifyRecord) {
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    InitializeListHead(&notifyRecord->ListEntry); //not required for listentry but it's a good practice.

    notifyRecord->FileObject = irpStack->FileObject;
    notifyRecord->DeviceExtension = deviceExtension;
    notifyRecord->Type = IRP_BASED;
    notifyRecord-> PendingIrp= Irp;
    
    //
    // Start the timer to run the CustomTimerDPC in DueTime seconds to
    // simulate an interrupt (which would queue a DPC).
    // The user's event object is signaled or the IRP is completed in the DPC to
    // notify the hardware event.
    //

    // ensure relative time for this sample

    if ( registerEvent->DueTime.QuadPart > 0 ) {
        registerEvent->DueTime.QuadPart = -(registerEvent->DueTime.QuadPart);
    }

    KeInitializeDpc(&notifyRecord->Dpc, // Dpc
                                CustomTimerDPC,         // DeferredRoutine
                                notifyRecord           // DeferredContext
                                ); 

    KeInitializeTimer(&notifyRecord->Timer );
         
    IoMarkIrpPending(Irp);

    //
    // We will set the cancel routine and TimerDpc within the
    // lock so that they don't modify the list before we are
    // completely done.
    //
    
    KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

    //
    // Set the cancel routine. This is required if the app decides to
    // exit or cancel the event prematurely.
    //

    IoSetCancelRoutine (Irp, EventCancelRoutine);

    

    InsertTailList(&deviceExtension->EventQueueHead, 
                                            &notifyRecord->ListEntry);

    //
    // We will save the record pointer in the IRP so that we can get to it directly in the
    // CancelRoutine. (Nar, this eliminates the need to match IRP pointers)
    //
    InterlockedExchangePointer(&Irp->Tail.Overlay.DriverContext[3], notifyRecord);
        
    KeSetTimer( &notifyRecord->Timer,   // Timer
                        registerEvent->DueTime,         // DueTime
                        &notifyRecord->Dpc      // Dpc  
                        );
    
    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

    //
    // We will return pending as we have marked the IRP pending.
    // 
    return STATUS_PENDING;;

}

NTSTATUS
RegisterEventBasedNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine  queues a event based notification record 
    to be handled by a DPC.

Arguments:

    DeviceObject - Context for the activity.
    Irp          - The device control argument block.

Return Value:

    NTSTATUS

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    PNOTIFY_RECORD notifyRecord;
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PREGISTER_EVENT registerEvent;

    DebugPrint(("\tRegisterEventBasedNotification\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    
    irpStack = IoGetCurrentIrpStackLocation( Irp );   
    registerEvent = (PREGISTER_EVENT)Irp->AssociatedIrp.SystemBuffer;

    //
    // Allocate a record and save all the event context.
    //

    notifyRecord = ExAllocatePoolWithTag(NonPagedPool, sizeof(NOTIFY_RECORD), TAG);

    if(NULL == notifyRecord) {
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    InitializeListHead(&notifyRecord->ListEntry);

    notifyRecord->FileObject = irpStack->FileObject;
    notifyRecord->DeviceExtension = deviceExtension;
    notifyRecord->Type = EVENT_BASED;

    //
    // Get the object pointer from the handle. Note we must be in the context
    // of the process that created the handle.
    //
    status = ObReferenceObjectByHandle( registerEvent->hEvent,
                                        SYNCHRONIZE,
                                        *ExEventObjectType,
                                        Irp->RequestorMode,
                                        &notifyRecord->Event,
                                        NULL
                                        );
 
    if ( !NT_SUCCESS(status) ) {

        DebugPrint(("\tUnable to reference User-Mode Event object, Error = 0x%x\n", status));
        ExFreePool(notifyRecord);
        return status;
    } 
    
    //
    // Start the timer to run the CustomTimerDPC in DueTime seconds to
    // simulate an interrupt (which would queue a DPC).
    // The user's event object is signaled or the IRP is completed in the DPC to
    // notify the hardware event.
    //

    // ensure relative time for this sample

    if ( registerEvent->DueTime.QuadPart > 0 ) {
        registerEvent->DueTime.QuadPart = -(registerEvent->DueTime.QuadPart);
    }

    KeInitializeDpc(&notifyRecord->Dpc, // Dpc
                                CustomTimerDPC,         // DeferredRoutine
                                notifyRecord           // DeferredContext
                                ); 

    KeInitializeTimer(&notifyRecord->Timer );

    ExInterlockedInsertTailList(&deviceExtension->EventQueueHead, 
                                            &notifyRecord->ListEntry, 
                                            &deviceExtension->QueueLock);

    KeSetTimer( &notifyRecord->Timer,   // Timer
                        registerEvent->DueTime,         // DueTime
                        &notifyRecord->Dpc      // Dpc  
                        );
    return STATUS_SUCCESS;
    
}

