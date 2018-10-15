/*++
Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    cancel.c

Abstract:    Demonstrates the use of new Cancel-Safe queue
            APIs to perform queuing of IRPs without worrying about
            any synchronization issues between cancel lock in the I/O
            manager and the driver's queue lock.

            This driver is written for an hypothetical data acquisition
            device that requires polling at a regular interval.
            The device has some settling period between two reads. 
            Upon user request the driver reads data and records the time. 
            When the next read request comes in, it checks the interval 
            to see if it's reading the device too soon. If so, it pends
            the IRP and sleeps for while and tries again.

Author:

    Eliyas Yakub (Feb 3, 2001)

Environment:

    Kernel mode

Revision History:

    Updated to use IoCreateDeviceSecure function - Oct 10, 2002


--*/

#include "cancel.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, CsampCreateClose)
#pragma alloc_text( PAGE, CsampUnload)
#pragma alloc_text( PAGE, CsampRead)
#endif // ALLOC_PRAGMA

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

    registryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    UNICODE_STRING      unicodeDeviceName;   
    UNICODE_STRING      unicodeDosDeviceName;  
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   devExtension;
    UNICODE_STRING sddlString;

    UNREFERENCED_PARAMETER (RegistryPath);

    CSAMP_KDPRINT(("DriverEntry Enter \n"));


    (void) RtlInitUnicodeString(&unicodeDeviceName, CSAMP_DEVICE_NAME_U);

    //
    // We will create a secure deviceobject so that only processes running
    // in admin and local system account can access the device. Refer
    // "Security Descriptor String Format" section in the platform
    // SDK documentation to understand the format of the sddl string.
    // We need to do because this is a legacy driver and there is no INF
    // involved in installing the driver. For PNP drivers, security descriptor
    // is typically specified for the FDO in the INF file.
    //

    (void) RtlInitUnicodeString( &sddlString, L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

    status = IoCreateDeviceSecure(
                DriverObject,
                sizeof(DEVICE_EXTENSION),
                &unicodeDeviceName,
                FILE_DEVICE_UNKNOWN,
                0,
                (BOOLEAN) FALSE,
                &sddlString,
                (LPCGUID)&GUID_DEVCLASS_CANCEL_SAMPLE, 
                &deviceObject
                );


    if (!NT_SUCCESS(status))
    {
        return status;
    }

    //
    // Allocate and initialize a Unicode String containing the Win32 name
    // for our device.
    //

    (void)RtlInitUnicodeString( &unicodeDosDeviceName, CSAMP_DOS_DEVICE_NAME_U );


    status = IoCreateSymbolicLink(
                (PUNICODE_STRING) &unicodeDosDeviceName,
                (PUNICODE_STRING) &unicodeDeviceName
                );

    if (!NT_SUCCESS(status))
    {
        IoDeleteDevice(deviceObject);
        return status;
    }

    devExtension = deviceObject->DeviceExtension;

    DriverObject->MajorFunction[IRP_MJ_CREATE]=
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CsampCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = CsampRead;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CsampCleanup;

    DriverObject->DriverUnload = CsampUnload;



    //
    // Set the flag signifying that we will do buffered I/O. This causes NT
    // to allocate a buffer on a ReadFile operation which will then be copied
    // back to the calling application by the I/O subsystem
    //

    deviceObject->Flags |= DO_BUFFERED_IO;

    //
    // Initialize the spinlock. This is used to serailize
    // access to the device.
    //

    KeInitializeSpinLock(&devExtension->DeviceLock);

    //
    // This is used to serailize access to the queue.
    //

    KeInitializeSpinLock(&devExtension->QueueLock);


    //
    //Initialize the Dpc object
    //

    KeInitializeDpc( &devExtension->PollingDpc,
                        CsampPollingTimerDpc,
                        (PVOID)deviceObject );

    //
    // Initialize the timer object
    //

    KeInitializeTimer( &devExtension->PollingTimer );

    //
    // Initialize the pending Irp devicequeue
    //

    InitializeListHead( &devExtension->PendingIrpQueue );

    //
    // 10 is multiplied because system time is specified in 100ns units
    //

    devExtension->PollingInterval.QuadPart = Int32x32To64(
                                CSAMP_RETRY_INTERVAL, -10);
    //
    // Note down system time
    //

    KeQuerySystemTime (&devExtension->LastPollTime);

    IoCsqInitializeEx( &devExtension->CancelSafeQueue,
                     CsampInsertIrp,
                     CsampRemoveIrp,
                     CsampPeekNextIrp,
                     CsampAcquireLock,
                     CsampReleaseLock,
                     CsampCompleteCanceledIrp );

    CSAMP_KDPRINT(("DriverEntry Exit = %x\n", status));

    return status;
}



NTSTATUS
CsampCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

   Process the Create and close IRPs sent to this device.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT Status code

--*/
{
    PIO_STACK_LOCATION   irpStack;
    NTSTATUS             status = STATUS_SUCCESS;

    PAGED_CODE ();

    CSAMP_KDPRINT(("CsampCreateClose Enter\n"));

    //
    // Get a pointer to the current location in the Irp.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    switch(irpStack->MajorFunction)
    {
        case IRP_MJ_CREATE:

            // 
            // The dispatch routine for IRP_MJ_CREATE is called when a 
            // file object associated with the device is created. 
            // This is typically because of a call to CreateFile() in 
            // a user-mode program or because a higher-level driver is 
            // layering itself over a lower-level driver. A driver is 
            // required to supply a dispatch routine for IRP_MJ_CREATE.
            //

            CSAMP_KDPRINT(("IRP_MJ_CREATE\n"));
            Irp->IoStatus.Information = 0;
            break;

        case IRP_MJ_CLOSE:

            //
            // The IRP_MJ_CLOSE dispatch routine is called when a file object
            // opened on the driver is being removed from the system; that is,
            // all file object handles have been closed and the reference count
            // of the file object is down to 0. Certain types of drivers do not
            // need to handle IRP_MJ_CLOSE, mainly drivers of devices that must
            // be available for the system to continue running. In general, this
            // is the place that a driver should "undo" whatever has been done 
            // in the routine for IRP_MJ_CREATE.
            //

            CSAMP_KDPRINT(("IRP_MJ_CLOSE\n"));
            Irp->IoStatus.Information = 0;
            break;

        default:
            CSAMP_KDPRINT((" Invalid CreateClose Parameter\n"));
            status = STATUS_INVALID_PARAMETER;
            break;
    }

    //
    // Save Status for return and complete Irp
    //
    
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    CSAMP_KDPRINT((" CsampCreateClose Exit = %x\n", status));

    return status;
}



NTSTATUS
CsampRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
 )
 /*++
     Routine Description:
  
           Read disptach routine
           
     Arguments:
  
         DeviceObject - pointer to a device object.
                 Irp             - pointer to current Irp
  
     Return Value:
  
         NT status code.
  
--*/
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   devExtension;
    PIO_STACK_LOCATION  irpStack;
    LARGE_INTEGER       currentTime;

    PAGED_CODE();

    CSAMP_KDPRINT(("--->CsampReadReport :%x\n", Irp));
   
    //
    // Get a pointer to the device extension.
    //

    devExtension = DeviceObject->DeviceExtension;
   
    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ASSERT(irpStack->FileObject);
    
    //
    // First make sure there is enough room.
    //
    
    if (irpStack->Parameters.Read.Length < sizeof(INPUT_DATA))
    {
        Irp->IoStatus.Status = status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Information  = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    //
    // Simple little random polling time generator.
    // FOR TESTING:
    // Initialize the data to mod 2 of some random number.
    // With this value you can control the number of times the
    // Irp will be queued before completion. Check 
    // CsampPollDevice routine to know how this works.
    //

    KeQuerySystemTime(&currentTime);

    *(ULONG *)Irp->AssociatedIrp.SystemBuffer =
                    ((currentTime.LowPart/13)%2);

    //
    // Try inserting the IRP in the queue. If the device is busy,
    // the IRP will get queued and the following function will
    // return SUCCESS. If the device is not busy, it will set the
    // IRP to DeviceExtension->CurrentIrp and return UNSUCCESSFUL.
    //
    
    if (!NT_SUCCESS(IoCsqInsertIrpEx(&devExtension->CancelSafeQueue, 
                                        Irp, NULL, NULL))) {
        IoMarkIrpPending(Irp);
                                        
        CsampInitiateIo(DeviceObject);
    }
    
    CSAMP_KDPRINT(("<---CsampReadReport\n"));
    
    return STATUS_PENDING;
    
}


VOID 
CsampInitiateIo(
    IN PDEVICE_OBJECT DeviceObject
)
 /*++
     Routine Description:
  
           Performs the actual I/O operations.
           
     Arguments:
  
         DeviceObject - pointer to a device object.
  
     Return Value:
  
         NT status code.

  
--*/

{
    NTSTATUS       status;
    PDEVICE_EXTENSION   devExtension = DeviceObject->DeviceExtension;
    PIRP                irp = NULL;

    CSAMP_KDPRINT(("--> CsampInitiateIo\n"));    

    irp = devExtension->CurrentIrp;

    while(TRUE) {        
        
        ASSERT(irp);
        ASSERT(irp == devExtension->CurrentIrp);        

        status = CsampPollDevice(DeviceObject, irp);
        if(status == STATUS_PENDING)
        {
            //
            // Oops, polling too soon. Start the timer to retry the operation.
            // 
            
            KeSetTimer(&devExtension->PollingTimer, devExtension->PollingInterval,
                      &devExtension->PollingDpc);
            break;

        }
        else 
        {   
            //
            // Read device is successful. Now complete the IRP and service
            // the next one from the queue.
            //
            irp->IoStatus.Status = status;
            CSAMP_KDPRINT(("completing irp :%x\n", irp));        
            IoCompleteRequest (irp, IO_NO_INCREMENT);
            
            irp = IoCsqRemoveNextIrp(&devExtension->CancelSafeQueue, NULL);

            if(!irp) {
                break;
            }            
        }

    }

    CSAMP_KDPRINT(("<---CsampInitiateIo\n"));

    return;

}


VOID
CsampPollingTimerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
)
 /*++
     Routine Description:
  
         CustomTimerDpc routine to process Irp that are
         waiting in the PendingIrpQueue
  
     Arguments:
  
         DeviceObject    - pointer to DPC object
         Context         - pointer to device object
         SystemArgument1 -  undefined
         SystemArgument2 -  undefined
  
     Return Value:
--*/
{
    PDEVICE_OBJECT          deviceObject;
    PDEVICE_EXTENSION       deviceExtension;

    CSAMP_KDPRINT(("---> CsampPollingTimerDpc\n"));

    deviceObject = (PDEVICE_OBJECT)Context;
    deviceExtension = deviceObject->DeviceExtension;

    CsampInitiateIo(deviceObject);
    
    CSAMP_KDPRINT(("<--- CsampPollingTimerDpc\n"));

}

NTSTATUS
CsampCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
/*++

Routine Description:
    This dispatch routine is called when the last handle (in
    the whole system) to a file object is closed. In other words, the open
    handle count for the file object goes to 0. A driver that holds pending
    IRPs internally must implement a routine for IRP_MJ_CLEANUP. When the
    routine is called, the driver should cancel all the pending IRPs that
    belong to the file object identified by the IRP_MJ_CLEANUP call. In other
    words, it should cancel all the IRPs that have the same file-object pointer
    as the one supplied in the current I/O stack location of the IRP for the
    IRP_MJ_CLEANUP call. Of course, IRPs belonging to other file objects should
    not be canceled. Also, if an outstanding IRP is completed immediately, the
    driver does not have to cancel it.

Arguments:

    DeviceObject     -- pointer to the device object
    Irp             -- pointer to the requesing Irp

Return Value:

    STATUS_SUCCESS   -- if the poll succeeded,
--*/
{

    PDEVICE_EXTENSION     devExtension;
    LIST_ENTRY             tempQueue;   
    PLIST_ENTRY            thisEntry;
    PIRP                   pendingIrp;
    PIO_STACK_LOCATION    pendingIrpStack, irpStack;

    CSAMP_KDPRINT(("--->CsampCleanupIrp\n"));

    devExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    while(pendingIrp = IoCsqRemoveNextIrp(&devExtension->CancelSafeQueue,
                                irpStack->FileObject))
    {

        //
        // Cancel the IRP
        //
        
        pendingIrp->IoStatus.Information = 0;
        pendingIrp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
        
    }

    //
    // Finally complete the cleanup IRP
    //
    
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    CSAMP_KDPRINT(("<---CsampCleanupIrp\n"));

    return STATUS_SUCCESS;

}

NTSTATUS
CsampPollDevice(
    PDEVICE_OBJECT DeviceObject,
    PIRP    Irp
    )

/*++

Routine Description:

   Pools for data

Arguments:

    DeviceObject     -- pointer to the device object
    Irp             -- pointer to the requesing Irp


Return Value:

    STATUS_SUCCESS   -- if the poll succeeded,
    STATUS_TIMEOUT   -- if the poll failed (timeout),
                        or the checksum was incorrect
    STATUS_PENDING   -- if polled too soon

--*/
{
    PINPUT_DATA         pInput;
    PDEVICE_EXTENSION   devExtension;
    PIO_STACK_LOCATION  irpStack;
    LARGE_INTEGER       currentTime;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    devExtension = DeviceObject->DeviceExtension;

    pInput  = (PINPUT_DATA)Irp->AssociatedIrp.SystemBuffer;

#ifdef REAL

    RtlZeroMemory( pInput, sizeof(INPUT_DATA) );

    //
    // If currenttime is less than the lasttime polled plus
    // minimum time required for the device to settle
    // then don't poll  and return STATUS_PENDING
    //
    
    KeQuerySystemTime(&currentTime);
    if (currentTime->QuadPart < (TimeBetweenPolls +
                devExtension->LastPollTime.QuadPart))
    {
        return  STATUS_PENDING;
    }

    //
    // Read/Write to the port here.
    // Fill the INPUT structure
    // 
       
    // 
    // Note down the current time as the last polled time
    // 
    
    KeQuerySystemTime(&devExtension->LastPollTime);
    
    
    return STATUS_SUCCESS;
#else 

    //
    // With this conditional statement
    // you can control the number of times the
    // irp should be queued before completing.
    //
    
    if(pInput->Data-- <= 0) 
    {                    
        Irp->IoStatus.Information = sizeof(INPUT_DATA);
        return STATUS_SUCCESS;
    }
    return STATUS_PENDING;
    
 #endif

}


VOID
CsampUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Free all the allocated resources, etc.

Arguments:

    DriverObject - pointer to a driver object.

Return Value:

    VOID
--*/
{
    PDEVICE_OBJECT       deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING      uniWin32NameString;
    PDEVICE_EXTENSION    devExtension = deviceObject->DeviceExtension;

    PAGED_CODE();

    CSAMP_KDPRINT(("--->CsampUnload\n"));

    //
    // The OS (XP and beyond) forces any DPCs that are already 
    // running to run to completion, even after the driver unload , 
    // routine returns, but before unmapping the driver image from 
    // memory. 
    // This driver makes an assumption that I/O request are going to
    // come only from usermode app and as long as there are active
    // IRPs in the driver, the driver will not get unloaded.
    // NOTE: If a driver can get I/O request directly from another
    // driver without having an explicit handle, you should wait on an 
    // event signalled by the DPC to make sure that DPC doesn't access
    // the resources that you are going to free here.
    //
    KeCancelTimer(&devExtension->PollingTimer);


    //
    // Create counted string version of our Win32 device name.
    //

    RtlInitUnicodeString( &uniWin32NameString, CSAMP_DOS_DEVICE_NAME_U );

    //
    // Delete the link from our device name to a name in the Win32 namespace.
    //

    IoDeleteSymbolicLink( &uniWin32NameString );

    if ( deviceObject != NULL )
    {
        IoDeleteDevice( deviceObject );
    }
 
    CSAMP_KDPRINT(("<---CsampUnload\n"));
    return;
}

NTSTATUS CsampInsertIrp (
    IN PIO_CSQ   Csq,
    IN PIRP              Irp,
    IN PVOID    InsertContext
    )
{
    PDEVICE_EXTENSION   devExtension;

    devExtension = CONTAINING_RECORD(Csq, 
                                 DEVICE_EXTENSION, CancelSafeQueue);

    if (!devExtension->CurrentIrp) {
        devExtension->CurrentIrp = Irp;
        return STATUS_UNSUCCESSFUL;
    }


    InsertTailList(&devExtension->PendingIrpQueue, 
                         &Irp->Tail.Overlay.ListEntry);
    return STATUS_SUCCESS;
}

VOID CsampRemoveIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp
    )
{
    PDEVICE_EXTENSION   devExtension;

    devExtension = CONTAINING_RECORD(Csq, 
                                 DEVICE_EXTENSION, CancelSafeQueue);
    
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

}


PIRP CsampPeekNextIrp(
    IN  PIO_CSQ Csq,
    IN  PIRP    Irp,
    IN  PVOID   PeekContext
    )
{
    PDEVICE_EXTENSION      devExtension;
    PIRP                    nextIrp = NULL;
    PLIST_ENTRY             nextEntry;
    PLIST_ENTRY             listHead;
    PIO_STACK_LOCATION     irpStack;

    devExtension = CONTAINING_RECORD(Csq, 
                             DEVICE_EXTENSION, CancelSafeQueue);
    
    listHead = &devExtension->PendingIrpQueue;

    // 
    // If the IRP is NULL, we will start peeking from the listhead, else
    // we will start from that IRP onwards. This is done under the
    // assumption that new IRPs are always inserted at the tail.
    //
        
    if(Irp == NULL) {       
        nextEntry = listHead->Flink;
    } else {
        nextEntry = Irp->Tail.Overlay.ListEntry.Flink;
    }

    
    while(nextEntry != listHead) {
        
        nextIrp = CONTAINING_RECORD(nextEntry, IRP, Tail.Overlay.ListEntry);

        irpStack = IoGetCurrentIrpStackLocation(nextIrp);
        
        //
        // If context is present, continue until you find a matching one.
        // Else you break out as you got next one.
        //
        
        if(PeekContext) {
            if(irpStack->FileObject == (PFILE_OBJECT) PeekContext) {       
                break;
            }
        } else {
            break;
        }
        nextIrp = NULL;
        nextEntry = nextEntry->Flink;
    }

    //
    // Check if this is from start packet.
    //

    if (PeekContext == NULL) {
        devExtension->CurrentIrp = nextIrp;
    }

    return nextIrp;
    
}


VOID CsampAcquireLock(
    IN  PIO_CSQ Csq,
    OUT PKIRQL  Irql
    )
{
    PDEVICE_EXTENSION   devExtension;

    devExtension = CONTAINING_RECORD(Csq, 
                                 DEVICE_EXTENSION, CancelSafeQueue);

    KeAcquireSpinLock(&devExtension->QueueLock, Irql);
}

VOID CsampReleaseLock(
    IN PIO_CSQ Csq,
    IN KIRQL   Irql
    )
{
    PDEVICE_EXTENSION   devExtension;

    devExtension = CONTAINING_RECORD(Csq, 
                                 DEVICE_EXTENSION, CancelSafeQueue);
    
    KeReleaseSpinLock(&devExtension->QueueLock, Irql);
}

VOID CsampCompleteCanceledIrp(
    IN  PIO_CSQ             pCsq,
    IN  PIRP                Irp
    )
{
    CSAMP_KDPRINT(("Cancelled IRP: %p\n", Irp));

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}



