/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Incomplete2.c

Abstract:

    Incomplete2.c is the second stage of the Toaster sample function driver. This
    stage builds upon the support implemented in the first stage, Incomplete1.c.

    New features implemented in this stage of the function driver:

    -Dispatch routines to support IRP_MJ_READ, IRP_MJ_WRITE, and
     IRP_MN_DEVICE_CONTROL. These routines are named ToasterCreate, ToasterClose,
     ToasterDispatchIO, ToasterReadWrite, and ToasterDispatchIoctl.

    -A driver-managed IRP queue to hold IRPs dispatched to the function driver.
     These routines are named ToasterQueueRequest and ToasterProcessQueuedRequests.

    -A StartDevice routine to start a toaster instance’s device hardware. This
     routine is named ToasterStartDevice.

    -Routines to synchronize the processing of IRP_MN_QUERY_STOP_DEVICE,
     IRP_MN_QUERY_REMOVE_DEVICE and IRP_MN_REMOVE_DEVICE when stopping or removing
     hardware instances. These routines are named ToasterIoIncrement and
     ToasterIoDecrement.

    This code is for learning purposes only. For additional code that supports
    other features, for example, Power Management and Windows Management
    Instrumentation (WMI), see the Featured1 stage.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub - 10-Oct-2002

    Kevin Shirley - 05-May-2003 - Commented for tutorial and learning purposes

--*/
#include "toaster.h"

ULONG DebugLevel = INFO;
#define _DRIVER_NAME_ "Incomplete2"

#ifdef ALLOC_PRAGMA
//
// Add the routines implemented in this stage to the PAGE section.
//
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
#pragma alloc_text (PAGE, ToasterSendIrpSynchronously)
#pragma alloc_text (PAGE, ToasterDispatchPower)
#pragma alloc_text (PAGE, ToasterSystemControl)
#endif

 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Updated Routine Description:
    The dispatch routines implemented in this stage of the function driver are
    connected to the MajorFunction array member of the DriverObject parameter.

--*/
{
    ToasterDebugPrint(TRACE, "Entered DriverEntry of "_DRIVER_NAME_"  version "
                                                         "built on " __DATE__" at "__TIME__ "\n");

    DriverObject->DriverExtension->AddDevice           = ToasterAddDevice;

    DriverObject->DriverUnload                         = ToasterUnload;

    DriverObject->MajorFunction[IRP_MJ_PNP]            = ToasterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = ToasterDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ToasterSystemControl;

    //
    // Connect the new dispatch routines implemented in this stage of the function
    // driver that support communication from applications. ToasterDispatchIO
    // initially processes read, write, and device control operations. These
    // operations are the result of user-mode calls to ReadFile, WriteFile, or
    // DeviceIoControl.
    //
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = ToasterCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = ToasterClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ToasterDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_READ]           = ToasterDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_WRITE]          = ToasterDispatchIO;

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

Updated Routine Description:
    ToasterAddDevice initializes additional members of the device extension,
    including those for the driver-managed IRP queue. The driver-managed IRP queue
    mechanism uses the following three members of the device extension:

    -QueueState
     The QueueState member of the device extension is a custom enumeration data
     type defined by the function driver that specifies three states which control
     how the driver-managed IRP queue mechanism operates. The three states are:

     --QueueState = HoldRequests

       The HoldRequests state specifies that any new IRPs dispatched to the
       function driver be added to the tail of the driver-managed IRP queue for
       later processing instead of being processed immediately. The function
       driver sets QueueState to HoldRequests when:

       ---ToasterAddDevice initializes the device extension, which occurs before
          ToasterDispatchPnP processes IRP_MN_START_DEVICE.

       ---ToasterDispatchPnP processes IRP_MN_QUERY_STOP_DEVICE, possibly to
          rebalance hardware resources.

       ---ToasterDispatchPnP processes IRP_MN_QUERY_REMOVE_DEVICE, because the
          hardware instance is possibly going to be removed.

       ---ToasterDispatchPower processes IRP_MN_QUERY_POWER or IRP_MN_SET_POWER.
          This feature is implemented in the Featured1 stage of the function
          driver.

     --QueueState = AllowRequests

       The AllowRequests state specifies that any new IRPs dispatched to the
       function driver be processed immediately instead of being added to the 
       driver-managed IRP queue. In addition, AllowRequests also specifies that
       ToasterProcessQueuedRequests dispatch any IRPs in the driver-managed IRP
       queue back to ToasterDispatchIO to be processed by the function driver.
       The function driver sets QueueState to AllowRequests when:

       ---ToasterDispatchPnP processes IRP_MN_START_DEVICE

       ---ToasterDispatchPnP processes IRP_MN_CANCEL_STOP_DEVICE or
          IRP_MN_CANCEL_REMOVE_DEVICE because the hardware instance is not going
          to be stopped or removed.

     --QueueState = FailRequests

       The FailRequests state specifies that any new IRPs dispatched to the
       function driver be failed immediately. In addition, FailRequests also
       specifies that all IRPs in the driver-managed IRP queue be failed when
       ToasterProcessQueuedRequests processes them. The function driver sets
       QueueState to FailRequests when:

       ---ToasterDispatchPnP processes IRP_MN_SURPRISE_REMOVAL, because the
          hardware instance has been surprise removed from the computer

       ---ToasterDispatchPnP processes IRP_MN_REMOVE_DEVICE, because the user has
          notified the system that the hardware is going to be removed

    -NewRequestsQueue

     The NewRequestsQueue member of the device extension is the driver-managed IRP
     queue. Any new IRPs that the system dispatches to the function driver are
     added to the tail of NewRequestsQueue when QueueState equals HoldRequests.
     Queued IRPs are removed from the head of the list. That is, the list is
     processed as a first-in first-out (FIFO) list of requests.

    -QueueLock

     The QueueLock member of the device extension is a spin lock that protects
     access to NewRequestsQueue. NewRequestsQueue requires a spin lock to
     synchronize thread access to NewRequestsQueue because multiple threads could
     attempt to simultaneously manipulate the queue’s contents, but only one
     thread at a time is allowed have access to the queue. Otherwise the IRPs in
     the queue cannot be processed in order resulting in unpredictable behavior.
     The queue must be accessed whenever an IRP is added or removed from the queue.

    ToasterAddDevice also initializes the kernel event members of the device
    extension that are used to synchronize the processing of
    IRP_MN_QUERY_STOP_DEVICE, IRP_MN_QUERY_REMOVE_DEVICE and IRP_MN_REMOVE_DEVICE.
    The mechanism to synchronize the processing of these IRPs uses the
    RemoveEvent, StopEvent, and OutstandingIO members of the device extension.

    The OutstandingIO member keeps the count of uncompleted IRPs that the system
    has dispatched to the function driver. OutstandingIO controls when the
    RemoveEvent and StopEvent kernel events are signaled.

    ToasterAddDevice initializes OutstandingIO to 1. That is, when there are zero
    uncompleted IRPs to process, OutstandingIo equals 1, not zero. The function
    driver calls ToasterIoIncrement to increment OutstandingIO every time the
    system dispatches a new IRP to the function driver. The function driver calls
    ToasterIoDecrement to decrement OutstandingIO when the function driver
    completes a dispatched IRP. The function driver must make an equal number of
    calls to increment and decrement OutstandingIO, except when ToasterDispatchPnP
    processes IRP_MN_REMOVE_DEVICE. When the function driver processes
    IRP_MN_REMOVE_DEVICE, the function driver must call ToasterIoDecrement twice
    in order to signal RemoveEvent. The extra call to ToasterIoDecrement is the
    only time OutstandingIO is decremented to 0.

    ToasterDispatchPnP uses StopEvent to synchronize the processing of
    IRP_MN_QUERY_STOP_DEVICE and IRP_MN_QUERY_REMOVE_DEVICE with any other threads
    that might be processing other IRPs in the function driver. Any thread that
    processes IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE is blocked
    from completing its respective IRP until ToasterIoDecrement signals StopEvent.

    ToasterDispatchPnP uses RemoveEvent to synchronize the processing of
    IRP_MN_REMOVE_DEVICE with any other threads that might be processing other IRPs
    in the function driver. Any thread that processes IRP_MN_REMOVE_DEVICE is
    blocked from completing the IRP until ToasterIoDecrement signals RemoveEvent.

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PFDO_DATA               fdoData;

    //
    // Call the PAGED_CODE macro because this routine must only execute at
    // IRQL = PASSIVE_LEVEL. The macro halts the system in the checked build
    // of Windows if IRQL >= DISPATCH_LEVEL, because then the Dispatcher cannot
    // be invoked, causing a system crash.
    //
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

    //
    // Initialize QueueState to HoldRequests. The function driver queues any new IRPs
    // that the system dispatches to it until ToasterDispatchPnP changes QueueState
    // to AllowRequests.
    // 
    // In Windows 2000, the system does not dispatch any read, write, or device
    // control operations until the function driver has started the hardware. However,
    // if the function driver executes in Win9x, initializing QueueState to
    // HoldRequests is a preventative measure because Win9x does dispatch read, write
    // or device control operations before the function driver has started its
    // hardware.
    //
    fdoData->QueueState = HoldRequests;

    fdoData->Self = deviceObject;
    fdoData->NextLowerDriver = NULL;

    //
    // Initialize the driver-managed IRP queue. New IRPs are added to the tail of the
    // list and queued IRPs are processed from the head of the queue.
    //
    InitializeListHead(&fdoData->NewRequestsQueue);

    //
    // Initialize the spin lock that protects and synchronizes thread access to the
    // driver-managed IRP queue. QueueLock synchronizes access to NewRequestsQueue by
    // allowing only one thread at a time to acquire the spin lock.
    //
    KeInitializeSpinLock(&fdoData->QueueLock);

    //
    // Initialize RemoveEvent to an unsignaled state. ToasterIoDecrement later
    // signals RemoveEvent when OutstandingIO transitions from 1 to 0.
    //
    KeInitializeEvent(&fdoData->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);

    //
    // Initialize StopEvent to a signaled state. ToasterIoIncrement unsignals
    // StopEvent when OutstandingIO transitions from 1 to 2. When StopEvent is in an
    // unsignaled state, ToasterDispatchPnP is blocked from continuing to process
    // IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE until a call to
    // ToasterIoDecrement later signals StopEvent when OutstandingIO transitions
    // from 2 to 1.
    //
    KeInitializeEvent(&fdoData->StopEvent,
                      SynchronizationEvent,
                      TRUE);

    //
    // Initialize OutstandingIO to 1. The function driver must make an equal number
    // of calls to ToasterIoIncrement and ToasterIoDecrement in order for
    // ToasterIoDecrement to properly signal StopEvent or RemoveEvent. The only time
    // OutstandingIO equals 0 is when ToasterDispatchPnP processes
    // IRP_MN_REMOVE_DEVICE and calls ToasterIoDecrement twice. Otherwise, if there
    // are not an equal number of calls to ToasterIoIncrement and ToasterIoDecrement
    // then StopEvent and RemoveEvent may be prematurely signaled, resulting in
    // unpredictable behavior in the function driver.
    //
    fdoData->OutstandingIO = 1;

    deviceObject->Flags |= DO_POWER_PAGABLE;
    deviceObject->Flags |= DO_BUFFERED_IO;

    fdoData->NextLowerDriver = IoAttachDeviceToDeviceStack (deviceObject,
                                                       fdoData->UnderlyingPDO);
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
    ToasterUnload does not change in this stage of the function driver.

--*/
{
    PAGED_CODE();

    ASSERT(NULL == DriverObject->DeviceObject);

    ToasterDebugPrint(TRACE, "unload\n");

    return;
}

 
NTSTATUS
ToasterDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchPnP calls ToasterIoIncrement when the system dispatches a PnP
    IRP to it. ToasterDispatchPnP later calls ToasterIoDecrement when it completes
    the IRP. ToasterDispatchPnP also updates QueueState to control the operation
    of the driver-managed IRP queue.

    When the system dispatches a new IRP to ToasterDispatchPnP to be processed,
    ToasterDispatchPnP calls ToasterIoIncrement to increment the count of
    uncompleted IRPs. When ToasterDispatchPnP later completes the IRP, it calls
    ToasterIoDecrement to decrement the count of uncompleted IRPs.
    ToasterDispatchPnP only calls ToasterIoDecrement once during the lifetime of
    the IRP, except with processing IRP_MN_REMOVE_DEVICE. When ToasterDispatchPnP
    processes IRP_MN_REMOVE_DEVICE, it must call ToasterIoDecrement two times in
    order to signal RemoveEvent.

    ToasterDispatchPnP also processes the PnP IRPs that were only processed in a
    limited capacity in the previous stage of the function driver, including:
    -IRP_MN_QUERY_STOP_DEVICE
    -IRP_MN_CANCEL_STOP_DEVICE
    -IRP_MN_STOP_DEVICE
    -IRP_MN_QUERY_REMOVE_DEVICE
    -IRP_MN_CANCEL_REMOVE_DEVICE

    Some of the PnP IRPs are completed directly in the switch case statement, and
    then immediately return to the caller. Other PnP IRPs break out of the switch
    case to be completed at the end of ToasterDispatchPnP.

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
    //
    // Increment the count of uncompleted IRPs. When the incoming IRP is finally
    // completed, then call ToasterIoDecrement. ToasterIoDecrement must be called
    // only once for each IRP. It is important to ensure that an equal number of
    // calls are made to ToasterIoIncrement and ToasterIoDecrement, otherwise the
    // driver-managed IRP queue, and the StopEvent and RemoveEvent kernel events
    // do not operate correctly, and ToasterDispatchPnP cannot correctly process
    // IRP_MN_QUERY_STOP_DEVICE, IRP_MN_QUERY_REMOVE_DEVICE, or
    // IRP_MN_REMOVE_DEVICE.
    //
    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        ToasterIoDecrement(fdoData);

        return STATUS_NO_SUCH_DEVICE;
    }

    switch (stack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        if (NT_SUCCESS (status))
        {
            //
            // After ToasterSendIrpSynchronously returns, the underlying bus driver
            // has completed IRP_MN_START_DEVICE. The IRP now contains the data
            // required by the function driver to start the hardware instance, such
            // as the I/O ports, I/O memory ranges, and interrupts to use.
            //
            // Start the toaster instance’s hardware. The code that started the
            // hardware in the previous stage of the function driver has been moved
            // into the ToasterStartDevice routine.
            //
            status = ToasterStartDevice (fdoData, Irp);
        }

        //
        // Break out of the switch case. The function driver stopped processing the
        // IRP in ToasterDispatchPnpComplete where the IRP’s status was set to
        // STATUS_MORE_PROCESSING_REQUIRED. ToasterDispatchPnP completes the IRP
        // outside the switch case.
        //
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        SET_NEW_PNP_STATE(fdoData, StopPending);

        //
        // Change the driver-managed IRP queue state to HoldRequests. Any new incoming
        // IRPs dispatched to ToasterDispatchIO will be added to the tail of the
        // driver-managed IRP queue instead of being processed immediately.
        //
        fdoData->QueueState = HoldRequests;

        ToasterDebugPrint(INFO, "Holding requests...\n");

        //
        // The count of uncompleted IRPs must be decremented before the call to
        // KeWaitForSingleObject in order for the StopEvent and RemoveEvent kernel
        // events to be signaled correctly.
        //
        ToasterIoDecrement(fdoData);

        //
        // Block ToasterDispatchPnP from passing IRP_MN_QUERY_STOP_DEVICE down the
        // device stack until StopEvent is signaled. IRP_MN_QUERY_STOP_DEVICE must
        // not be passed down the device stack to be processed by the bus driver
        // until the hardware instance is idle and no longer processing any requests.
        // 
        // The call to KeWaitForSingleObject does not return until StopEvent is
        // signaled. For example, if no other threads are processing IRPs when the
        // system dispatches IRP_MN_QUERY_STOP_DEVICE to the function driver, then
        // the earlier call to ToasterIoDecrement signals StopEvent. However, if
        // another thread is processing an IRP, then StopEvent is signaled when that
        // thread calls ToasterIoDecrement.
        //
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

   case IRP_MN_CANCEL_STOP_DEVICE:
        //
        // The underlying bus driver must process IRP_MN_CANCEL_STOP_DEVICE before
        // the function driver can because the bus driver must reallocate the same
        // hardware resources that the toaster instance was using before the function
        // driver received and processed IRP_MN_QUERY_STOP_DEVICE.
        // 
        // The function driver calls ToasterSendIrpSynchronously to have the bus
        // driver process the IRP before the function driver processes it.
        // ToasterSendIrpSynchronously passes the IRP down the device stack to be
        // processed by the bus driver, and does not return to the caller until the
        // bus driver completes the IRP.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status))
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            //
            // Change the driver-managed IRP queue state to AllowRequests. Any new
            // incoming IRPs dispatched to ToasterDispatchIO will be processed
            // immediately instead of being added to the tail of the driver-managed
            // IRP queue. In addition, AllowRequests also specifies that
            // ToasterProcessQueuedRequests dispatch any IRPs in the driver-managed
            // IRP queue back to ToasterDispatchIO to be processed by the function
            // driver.
            //
            fdoData->QueueState = AllowRequests;

            //
            // Test the assumption that the previous hardware state equaled Started.
            // The previous hardware state should be Started because that should have
            // been the state when ToasterDispatchPnP processed
            // IRP_MN_QUERY_STOP_DEVICE.
            //
            ASSERT(Started == fdoData->DevicePnPState);

            //
            // Call ToasterProcessQueuedRequests to dispatch again all IRPs in the
            // driver-managed IRP queue. This includes all IRPs already present in
            // the queue before IRP_MN_QUERY_STOP_DEVICE was processed, as well as
            // any new IRPs dispatched to ToasterDispatchIO between
            // IRP_MN_QUERY_STOP_DEVICE and IRP_MN_CANCEL_STOP_DEVICE.
            //
            ToasterProcessQueuedRequests(fdoData);
        }
        else
        {
            //
            // If ToasterSendIrpSynchronously returns a failure that is because
            // a lower driver in the device stack failed IRP_MN_CANCEL_STOP_DEVICE.
            // That is a fatal error.
            //
            ASSERTMSG("Cancel stop failed. Fatal error!", FALSE);
            ToasterDebugPrint(WARNING, "Failure statute = 0x%x\n", status);
        }

        break;

    case IRP_MN_STOP_DEVICE:
        //
        // If the function driver is unable to stop the hardware instance without
        // losing any data when it processed an earlier IRP_MN_QUERY_STOP_DEVICE,
        // then the function driver should have failed IRP_MN_QUERY_STOP_DEVICE
        // with STATUS_UNSUCCESSFUL. 
        //

        SET_NEW_PNP_STATE(fdoData, Stopped);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        SET_NEW_PNP_STATE(fdoData, RemovePending);

        //
        // Change the driver-managed IRP queue state to HoldRequests. Any new incoming
        // IRPs dispatched to ToasterDispatchIO will be added to the tail of the
        // driver-managed IRP queue instead of being processed immediately.
        //
        fdoData->QueueState = HoldRequests;

        ToasterDebugPrint(INFO, "Query - remove holding requests...\n");

        //
        // The count of uncompleted IRPs must be decremented before the call to
        // KeWaitForSingleObject in order for the StopEvent and RemoveEvent kernel
        // events to be signaled correctly.
        //
        ToasterIoDecrement(fdoData);

        //
        // Block ToasterDispatchPnP from passing IRP_MN_QUERY_REMOVE_DEVICE down the
        // device stack until StopEvent is signaled. IRP_MN_QUERY_REMOVE_DEVICE must
        // not be passed down the device stack to be processed by the bus driver
        // until the hardware instance is idle and no longer processing any requests.
        //
        // When ToasterDispatchPnP processes IRP_MN_QUERY_REMOVE_DEVICE, the call to
        // KeWaitForSingleObject waits on StopEvent instead of RemoveEvent because
        // processing IRP_MN_QUERY_REMOVE_DEVICE is similar to processing
        // IRP_MN_QUERY_STOP_DEVICE. ToasterDispatchPnP only waits on RemoveEvent
        // when it processes IRP_MN_REMOVE_DEVICE.
        // 
        // The call to KeWaitForSingleObject does not return until StopEvent is
        // signaled. For example, if no other threads are processing IRPs when the
        // system dispatches IRP_MN_QUERY_REMOVE_DEVICE to the function driver, then
        // the earlier call to ToasterIoDecrement signals StopEvent. However, if
        // another thread is processing an IRP, then StopEvent is signaled when that
        // thread calls ToasterIoDecrement.
        //
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

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        //
        // The underlying bus driver must process IRP_MN_CANCEL_REMOVE_DEVICE before
        // the function driver can because the bus driver must reallocate the same
        // hardware resources that the toaster instance was using before the function
        // driver received and processed IRP_MN_QUERY_REMOVE_DEVICE.
        // 
        // The function driver calls ToasterSendIrpSynchronously to have the bus
        // driver process the IRP before the function driver processes it.
        // ToasterSendIrpSynchronously passes the IRP down the device stack to be
        // processed by the bus driver, and does not return to the caller until the
        // bus driver completes the IRP.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status))
        {
            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            //
            // Change the driver-managed IRP queue state to AllowRequests. Any new
            // incoming IRPs dispatched to ToasterDispatchIO will be processed
            // immediately instead of being added to the tail of the driver-managed
            // IRP queue. In addition, AllowRequests also specifies that
            // ToasterProcessQueuedRequests dispatch any IRPs in the driver-managed
            // IRP queue back to ToasterDispatchIO to be processed by the function
            // driver.
            //
            fdoData->QueueState = AllowRequests;

            //
            // Call ToasterProcessQueuedRequests to dispatch again all IRPs in the
            // driver-managed IRP queue. This includes all IRPs already present in
            // the queue before IRP_MN_QUERY_REMOVE_DEVICE was processed, as well as
            // any new IRPs dispatched to ToasterDispatchIO between
            // IRP_MN_QUERY_REMOVE_DEVICE and IRP_MN_CANCEL_REMOVE_DEVICE.
            //
            ToasterProcessQueuedRequests(fdoData);
        }
        else
        {
            //
            // If ToasterSendIrpSynchronously returns a failure that is because
            // a lower driver in the device stack failed IRP_MN_CANCEL_STOP_DEVICE.
            // That is a fatal error.
            //
            ASSERTMSG("Cancel remove failed. Fatal error!", FALSE);
            ToasterDebugPrint(WARNING, "Failure status = 0x%x\n", status);
        }

        break;

   case IRP_MN_SURPRISE_REMOVAL:
        SET_NEW_PNP_STATE(fdoData, SurpriseRemovePending);

        //
        // Change the driver-managed IRP queue state to FailRequests. Any new
        // incoming IRPs dispatch to ToasterDispatchIO will be failed immediately.
        // In addition, FailRequests also specifies that any IRPs in the
        // driver-managed IRP queue be failed when ToasterProcessQueuedRequests
        // processes them.
        //
        fdoData->QueueState = FailRequests;

        //
        // Call ToasterProcessQueuedRequests to fail all IRPs in the driver-managed
        // IRP queue. ToasterProcessQueuedRequests simply flushes the queue and
        // fails each IRP in the queue with STATUS_NO_SUCH_DEVICE.
        //
        ToasterProcessQueuedRequests(fdoData);

        status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

        if (!NT_SUCCESS (status))
        {
            ToasterDebugPrint(ERROR, 
                "IoSetDeviceInterfaceState failed: 0x%x\n", status);
        }

        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;

   case IRP_MN_REMOVE_DEVICE:
        //
        // If the function driver is unable to stop the hardware instance without
        // losing any data when it processed an earlier IRP_MN_QUERY_REMOVE_DEVICE,
        // then the function driver should have failed IRP_MN_QUERY_REMOVE_DEVICE
        // with STATUS_UNSUCCESSFUL
        //

        SET_NEW_PNP_STATE(fdoData, Deleted);

        if(SurpriseRemovePending != fdoData->PreviousPnPState)
        {
            //
            // Change the driver-managed IRP queue state to FailRequests. Any new
            // incoming IRPs dispatch to ToasterDispatchIO will be failed
            // immediately. In addition, FailRequests also specifies that any IRPs
            // in the driver-managed IRP queue be failed when
            // ToasterProcessQueuedRequests processes them.
            //
            fdoData->QueueState = FailRequests;

            //
            // Call ToasterProcessQueuedRequests to fail all IRPs in the
            // driver-managed IRP queue. ToasterProcessQueuedRequests simply flushes
            // the queue and fails each IRP in the queue with STATUS_NO_SUCH_DEVICE.
            //
            ToasterProcessQueuedRequests(fdoData);

            status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

            if (!NT_SUCCESS (status))
            {
                ToasterDebugPrint(ERROR,
                        "IoSetDeviceInterfaceState failed: 0x%x\n", status);
            }
        }

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        requestCount = ToasterIoDecrement (fdoData);

        //
        // Test the assumption that the count of uncompleted IRPs is greater than
        // zero after the ToasterIoDecrement call. The count should be greater than
        // zero unless there is a bug in the code somewhere and ToasterIoDecrement
        // was not called an equal number of times that ToasterIoIncrement was
        // called.
        //
        ASSERT(requestCount > 0);

        //
        // Call ToasterIoDecrement again to decrement OutstandingIO to zero. This is
        // the only time that OutstandingIO should equal 0. The extra call to
        // ToasterIoDecrement synchronizes the processing of IRP_MN_REMOVE_DEVICE by
        // signaling RemoveEvent.
        // 
        // RemoveEvent can only be signaled after StopEvent, when no more IRPs are
        // being processed, and the toaster instance’s hardware is stopped.
        //
        requestCount = ToasterIoDecrement (fdoData);

        //
        // Block ToasterDispatchPnP from passing IRP_MN_REMOVE_DEVICE down the device
        // stack until RemoveEvent is signaled. IRP_MN_REMOVE_DEVICE must not be passed
        // down the device stack to be processed by the bus driver until the hardware
        // instance has been stopped and RemoveEvent is signaled
        // 
        // The call to KeWaitForSingleObject does not return until RemoveEvent is
        // signaled. For example, if no other threads are processing IRPs when the
        // system dispatches IRP_MN_REMOVE_DEVICE to the function driver, then the
        // second call to ToasterIoDecrement signals RemoveEvent. However, if another
        // thread is processing an IRP, then RemoveEvent is signaled when that thread
        // calls ToasterIoDecrement.
        //
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

    default:
        //
        // Pass all unprocessed IRPs down the device stack. All PnP IRPs, as well as
        // all power and WMI IRPSs, must be passed down the device stack so that the
        // underlying bus driver can process them, regardless of whether the function
        // driver processes them.
        //

        IoSkipCurrentIrpStackLocation (Irp);

        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement(fdoData);

        return status;
    }

    //
    // The IRP_MN_START_DEVICE, IRP_MN_CANCEL_STOP_DEVICE, and
    // IRP_MN_CANCEL_REMOVE_DEVICE switch cases break out of the switch case to
    // have their respective IRPs completed here. Set the IRP’s status to the value
    // returned from ToasterSendIrpSynchronously.
    //
    Irp->IoStatus.Status = status;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    //
    // Decrement the count of how many IRPs remain uncompleted. This call to
    // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An equal
    // number of calls to ToasterIoincrement and ToasterIoDecrement is essential for
    // the StopEvent and RemoveEvent kernel events to be signaled correctly.
    //
    ToasterIoDecrement(fdoData);

    //
    // Return the status value returned from ToasterSendIrpSynchronously to the
    // caller.
    //
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

New Routine Description:
    ToasterDispatchIO dispatches IRP_MJ_READ and IRP_MJ_WRITE IRPs to
    ToasterReadWrite. Read and write operations can be processed by a single,
    combined routine, or by separate routines. The Toaster sample function driver
    demonstrates a combined dispatch read and dispatch write routine.
    ToasterReadWrite processes user-mode ReadFile and WriteFile calls.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the read or write operation to perform on hardware instance
    described by the DeviceObject parameter, such as read 4KB of data from the
    hardware, or write 10 bytes to it.

Return Value Description:
    ToasterReadWrite returns STATUS_SUCCESS to indicate that the read or write
    operation succeeded. Because the Toaster hardware is imaginary, the read/write
    operation always succeeds. However, in practice, ToasterReadWrite could return
    many errors to the caller.

--*/
{
    PFDO_DATA   fdoData;
    NTSTATUS    status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "ReadWrite called\n");

    //
    // Perform the read or write operation here. Because the toaster hardware is
    // imaginary (the device hardware as well as the bus hardware), there is no code
    // to execute here. Examine the other sample drivers shipped with the DDK to see
    // how read and write operations are performed.
    // 
    // This is where the hardware instance would be set to perform a programmed I/O
    // or DMA operation.
    //

    //
    // Set the status of the read or write IRP to success. If there was a hardware
    // error during the read or write operation, then the IRP would be failed with
    // the appropriate code.
    //
    status = STATUS_SUCCESS;

    //
    // Return the number of bytes transferred during the read of write operation to
    // the original user-mode ReadFile or WriteFile call in the IRP’s
    // IoStatus.Information member.
    // 
    // Because the toaster hardware is imaginary, 0 bytes are transferred.
    //
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

New Routine Description:
    The system dispatches IRP_MJ_CREATE IRPs to ToasterCreate. ToasterCreate
    processes user-mode CreateFile calls.

    The presence of a driver-implemented routine in the IRP_MN_CREATE MajorFunction
    array entry of the DriverObject parameter in DriverEntry allows the system to
    successfully return a handle to the hardware instance to the CreateFile call.
    Without a driver implemented routine in this array entry, the system fails
    any calls to CreateFile.

    A DispatchCreate routine for PnP hardware is not usually required to perform
    any special tasks to create and return a handle to the caller, except return
    STATUS_SUCCESS to the caller.

    In the case of the Toaster sample function driver, it is not necessary to
    access the toaster instance’s hardware to process a create request. The
    hardware state does not need to be changed, and the driver-managed IRP queue
    does not need to be manipulated.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the handle creation operation to perform on the hardware
    instance described by the DeviceObject parameter.

Return Value Description:
    ToasterCreate returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Otherwise ToasterCreate returns
    STATUS_SUCCESS to indicate that the create handle operation succeeded.

--*/
{
    PFDO_DATA    fdoData;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Create \n");

    //
    // Because the toaster hardware is imaginary, it is not necessary to access the
    // hardware to process create IRPs, nor is it necessary to add the incoming IRP
    // to the driver-managed IRP queue.
    //

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Set the IRP’s IoStatus.Information member with the number of bytes transferred
    // during the create operation. Because it is not necessary to access the toaster
    // instance’s hardware to process the create request, the number of bytes
    // transferred is 0.
    //
    Irp->IoStatus.Information = 0;

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    ToasterIoDecrement(fdoData);

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

New Routine Description:
    The system dispatches IRP_MJ_CLOSE IRPs to ToasterClose. ToasterClose
    processes user-mode CloseHandle calls.

    The presence of a driver implemented routine in the IRP_MN_CLOSE MajorFunction
    array entry of the DriverObject parameter in DriverEntry allows the system to
    successfully close a handle to the hardware instance to the CloseHandle call.
    Without a driver implemented routine in this array entry, the system fails
    any calls to CloseHandle.

    The DispatchClose routine for PnP hardware is not usually required to perform
    any special tasks to close a handle and return to the caller, except return
    STATUS_SUCCESS to the caller.

    In the case of the Toaster sample function driver, it is not necessary to
    access the toaster instance’s hardware to process a close request. The
    hardware state does not need to be changed, and the driver-managed IRP queue
    does not need to be manipulated.

    Note that ToasterClose does not check to determine if the hardware instance
    has been deleted. On Windows 2000 and later, the system does not send
    IRP_MJ_CLOSE after it sends IRP_MN_REMOVE_DEVICE (although it is possible that
    a 3rd party driver higher in the device stack could fake one). However,
    Windows 9x does send IRP_MJ_CLOSE after it sends IRP_MN_REMOVE_DEVICE.

    In addition, Windows 2000 sends IRP_MN_SURPRISE_REMOVAL and then waits for all
    handles to the hardware instance to close before it sends IRP_MN_REMOVE_DEVICE.
    However, Windows 9x immediately sends IRP_MN_REMOVE_DEVICE, even if there are
    open handles to the hardware instance. Therefore, to prevent leaking memory,
    ToasterClose does not check if the hardware instance has been deleted, and
    proceeds to close all handles even after the hardware instance has been
    deleted.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the close handle operation to perform on the hardware instance
    described by the DeviceObject parameter.

Return Value Description:
    ToasterClose returns STATUS_SUCCESS to indicate the close handle operation
    succeeded. Close IRPs can be pended but cannot be failed.

--*/
{
    PFDO_DATA    fdoData;
    NTSTATUS     status;
    TOASTER_INTERFACE_STANDARD busInterface;

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

New Routine Description:
    ToasterDispatchIO dispatches IRP_MJ_DEVICE_CONTROL IRPs to
    ToasterDispatchIoctl. ToasterDispatchIoctl processes user-mode DeviceIoControl
    calls.

    This stage of the function driver does not implement support for any IOCTLs.
    The Featured1 stage of the function driver implements support to hide the
    hardware instance from displaying in the Device Manager, unless the
    "Show hidden devices" menu item is checked.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp    
    Irp describes the device control operation to perform on the hardware instance
    described by the DeviceObject parameter, such as to change the operating mode
    of the hardware instance.

Return Value Description:
    ToasterDispatchIoctl returns STATUS_INVALID_DEVICE_REQUEST for all incoming
    device control operations in this stage of the function driver.

--*/
{
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status= STATUS_SUCCESS;
    PFDO_DATA               fdoData;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Ioctl called\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Get the parameters of the IRP from the function driver’s location in the IRP’s
    // I/O stack. The results of the function driver’s processing of the IRP, if any,
    // are then stored back in the same I/O stack location.
    // 
    // A device control’s parameters are stored in the IRP’s
    // Parameters.DeviceIoControl member. Drivers can change the members of the
    // device control operation as the IRP is processed and passed down and back up
    // the device stack, if necessary.
    //
    stack = IoGetCurrentIrpStackLocation (Irp);

    switch (stack->Parameters.DeviceIoControl.IoControlCode)
    {
    default:
        //
        // Fail all device control requests in this stage of the function driver.
        //
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

New Routine Description:
    The system dispatches IRP_MJ_READ, IRP_MJ_WRITE, and IRP_MJ_DEVICE_CONTROL
    IRPs to ToasterDispatchIO. ToasterDispatchIO processes user-mode ReadFile,
    WriteFile, and DeviceIoControl calls. ToasterDispatchIO acts as a layer of
    abstraction between I/O requests sent from applications, as well as queued
    IRPs that are being dispatched again from the driver-managed IRP queue.

    If QueueState equals AllowRequests, then ToasterDispatchIO dispatches the
    incoming IRP to the respective DispatchRead, DispatchWrite, or
    DispatchDeviceControl routine. If QueueState equals HoldRequests then
    ToasterDispatchIO queues the incoming IRP and returns to the caller.

    If QueueState equals AllowRequests, then the major function code of the
    code of the incoming IRP must be determined from its I/O stack location so
    that ToasterDispatchIO can dispatch the IRP to the respective routine.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp    
    Irp describes a read, write, or device control operation to perform on the
    hardware instance described by the DeviceObject parameter.

Return Value Description:
    If QueueState equals HoldRequests then ToasterDispatchIO returns the status
    returned by ToasterQueueRequest, which indicates if the incoming IRP was
    successfully added to the driver-managed IRP queue.

    ToasterDispatchIO returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Otherwise, ToasterDispatchIO
    returns the status of the read, write, or device control operation.

    ToasterDispatchIO returns STATUS_UNSUCCESSFUL if it is passed an IRP that is
    not a read, write, or device control operation.

--*/
{
    PIO_STACK_LOCATION      stack;
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
        //
        // If the driver-managed IRP queue is holding requests, then the incoming
        // IRP must be added to the queue. The IRP will be processed at a later
        // time when ToasterDispatchPnP changes QueueState to AllowRequests.
        //
        return ToasterQueueRequest(fdoData, Irp);
    }

    //
    // Get the parameters of the IRP from the function driver’s location in the IRP’s
    // I/O stack. The results of the function driver’s processing of the IRP, if any,
    // are then stored back in the same I/O stack location.
    // 
    // The major function code is stored in the IRP’s MajorFunction member.
    //
    stack = IoGetCurrentIrpStackLocation (Irp);
    
    switch (stack->MajorFunction)
    {
    case IRP_MJ_READ:
        //
        // Fall through to the next case statement.
        //
    case IRP_MJ_WRITE:
        //
        // Send the read or write IRP to the combined ToasterReadWrite dispatch
        // routine. The value returned from ToasterReadWrite is returned to the
        // original user-mode ReadFile or WriteFile caller.
        //
        status =  ToasterReadWrite(DeviceObject, Irp);

        break;

    case IRP_MJ_DEVICE_CONTROL:
        //
        // Send the device control IRP to the ToasterDispatchIoctl dispatch
        // routine. The value returned from ToasterDispatchIoctl is returned to the
        // user-mode DeviceIoControl caller.
        //
        status =  ToasterDispatchIoctl(DeviceObject, Irp);

        break;

    default:
        //
        // Fail the incoming IRP if it is not a read, write, or device control
        // operation. The I/O manager will not pass a non read, write, or device
        // control IRP to the function driver. However, it is possible another driver
        // might send a non-read, write, or device control IRP directly to the
        // function driver’s IRP_MJ_READ, IRP_MJ_WRITE, or IRP_MJ_DEVICE_CONTROL
        // dispatch routines (which is processed by ToasterDispatchIO for all three).
        //
        ASSERTMSG(FALSE, "ToasterDispatchIO invalid IRP");

        status = STATUS_UNSUCCESSFUL;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        break;
    }               

    ToasterIoDecrement(fdoData);

    //
    // After ToasterDispatchIO returns, the IRP is finished. Depending on the
    // operation described by the IRP, the results of the operation must then be
    // copied back to the caller by the I/O manager. The Toaster sample function
    // driver specified buffered I/O (DO_BUFFERED_IO) ToasterAddDevice, which causes
    // the I/O manager to copy any data buffer from kernel memory space back into
    // the caller’s memory space.
    //
    return status;
}

 
NTSTATUS
ToasterStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP             Irp
    )
/*++

New Routine Description:
    ToasterDispatchPnP calls ToasterStartDevice after the underlying bus driver
    has processed IRP_MN_START_DEVICE. ToasterStartDevice performs the operations
    that were processed by ToasterDispatchPnP in the IRP_MN_START_DEVICE switch
    case in the Incomplete1 stage of the function driver, such as enabling the
    device interface instance, and updating the hardware state.

    After ToasterStartDevice has performed whatever initialization is needed to
    start the hardware instance, it calls ToasterProcessQueuedRequests to process
    any IRPs that the system might have dispatched to the function driver before
    the hardware instance was started.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance to
    start.

    Irp
    Irp represents the IRP_MN_START_DEVICE that now contains the data required by
    the function driver to start the hardware instance, such as I/O port
    resources, I/O memory ranges, or interrupt.

Return Value Description:
    ToasterStartDevice returns STATUS_SUCCESS to indicate the device hardware was
    successfully started. Otherwise ToasterStartDevice returns the status returned
    by IoSetDeviceInterfaceState if an error occurred.

--*/
{
    NTSTATUS    status = STATUS_SUCCESS;
    PIO_STACK_LOCATION      stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    status = IoSetDeviceInterfaceState(&FdoData->InterfaceName, TRUE);

    if (!NT_SUCCESS (status))
    {
        ToasterDebugPrint(ERROR, "IoSetDeviceInterfaceState failed: 0x%x\n",
                                status);
        return status;
    }

    SET_NEW_PNP_STATE(FdoData, Started);

    //
    // Change the driver-managed IRP queue state to AllowRequests. Any new incoming
    // IRPs dispatched to ToasterDispatchIO will be processed immediately instead of
    // being added to the tail of the driver-managed IRP queue. In addition,
    // AllowRequests also specifies that ToasterProcessQueuedRequests dispatch any
    // IRPs in the driver-managed IRP queue back to ToasterDispatchIO to be processed
    // by the function driver.
    //
    FdoData->QueueState = AllowRequests;

    //
    // Process any IRPs that the system might have dispatched to the function driver
    // before the hardware instance was started.
    //
    ToasterProcessQueuedRequests(FdoData);

    return status;
}

 
NTSTATUS
ToasterQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    )

/*++

New Routine Description:
    The ToasterQueueRequest and ToasterProcessQueuedRequests implement the
    driver-managed IRP queue. These routines are closely related.

    The Toaster sample function driver implements its own driver-managed IRP queue
    instead of using the system-managed IRP queue. When a driver relies on the
    system-managed IRP queue, the driver can only process a single IRP at a time.
    For simple hardware devices, this method is acceptable. However, for more
    complex hardware, such as Toaster class hardware, implementing a
    driver-managed IRP queue is more efficient because it allows multiple threads
    to process IRPs in the function driver simultaneously.

    Because the function driver implements its own mechanism to manage IRPs
    dispatched to it by the system, the function driver does not implement a
    StartIo routine, nor is the driver required to call IoStartNextPacket. These
    system calls are only required when using the system-managed IRP queue.

    ToasterQueueRequest adds the incoming IRP to the tail of the driver-managed
    IRP queue whenever the hardware instance is in a paused or hold state as a
    result of the function driver processing IRP_MN_QUERY_STOP_DEVICE or
    IRP_MN_QUERY_REMOVE_DEVICE in ToasterDispatchPnP, thus allowing any operation
    in progress on the hardware instance to continue uninterrupted. When the
    operation completes, the hardware instance becomes available to process the
    next queued IRP in the driver-managed IRP queue.

    The queue is processed later, when QueueState is set to AllowRequests and the
    function driver calls ToasterProcessQueuedRequests. Access to the IRPs in the
    queue is synchronized using the spin lock QueueLock.

    ToasterDispatchIO calls ToasterQueueRequest to queue new IRPs when QueueState
    equals HoldRequests. QueueState is set to HoldRequests when ToasterDispatchPnP
    processes IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE.

    The number of uncompleted IRPs is already incremented before ToasterQueueRequest
    is called. After ToasterQueueRequest has queued the incoming IRP it must
    decrement the number of uncompleted IRPs even though the IRP is not completed.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that contains the driver-managed IRP queue to add the incoming IRP to.

    Irp
    Irp represents an IRP to be added to the driver-managed IRP queue.

Return Value Description:
    ToasterQueueRequest must return STATUS_PENDING because the incoming IRP is
    marked pending.

--*/
{
    KIRQL               oldIrql;

    ToasterDebugPrint(TRACE, "Queuing Requests\n");

    //
    // Test the assumption that ToasterQueueRequest is called when QueueState equals
    // HoldRequests. ToasterQueueRequest should not be called when QueueState equals
    // AllowRequests or FailRequests.
    //
    ASSERT(HoldRequests == FdoData->QueueState);

    //
    // Mark the incoming IRP as pending. Because the IRP is going to be added to the
    // driver-managed IRP queue and will not be processed immediately, it must be
    // marked pending. When an IRP is marked pending, the function driver must return
    // STATUS_PENDING to the system.
    // 
    // The function driver will later resume processing the IRP in
    // ToasterDispatchPnpComplete. The system calls ToasterDispatchPnpComplete after
    // the bus driver has completed the IRP.
    //
    IoMarkIrpPending(Irp);
    
    //
    // Acquire the spin lock that protects and synchronizes thread access to
    // NewRequestsQueue. Only one thread at a time is allowed to have access to the
    // driver-managed IRP queue. Otherwise the IRPs in the queue cannot be processed
    // in order resulting in unpredictable behavior.
    // 
    // When the spin lock is acquired, the system’s current IRQL must be saved in
    // oldIrql. When the spin lock is released, the system’s original IRQL must be
    // restored. Saving and restoring the IRQL is critical to the normal operation
    // of the system.
    //
    KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

    //
    // Add the incoming IRP to the tail of the driver-managed IRP queue. The queue is
    // a FIFO queue. The oldest IRPs in the queue are the first to be removed and
    // processed from the queue when the function driver calls
    // ToasterProcessQueuedRequests.
    //
    InsertTailList(&FdoData->NewRequestsQueue,
                   &Irp->Tail.Overlay.ListEntry);

    //
    // Release the spin lock that protects the driver-managed IRP queue. If another
    // thread was waiting in a call to KeAcquireSpinLock while attempting to acquire
    // QueueLock, it can now acquire it and manipulate the contents of the queue.
    // 
    // When the spin lock is released, the system’s original IRQL must be restored to
    // the value is was before the spin lock was acquired.
    //
    KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

    ToasterIoDecrement(FdoData);

    //
    // Return STATUS_PENDING. ToasterQueueRequest must return STATUS_PENDING to the
    // caller because ToasterQueueRequest called IoMarkIrpPending earlier. If the
    // read, write, or device control operation is synchronous (that is, the
    // lpOverlapped parameter of the user-mode call equals NULL), then the caller
    // cannot continue its execution until the function driver completes the pending
    // IRP. However, if the operation is asynchronous (that is, the lpOverlapped
    // parameter of the user-mode call does not equal NULL), then the user-mode call
    // returns ERROR_IO_PENDING and the caller can resume its execution. When the
    // function driver later completes the pending IRP, the I/O manager signals the
    // event specified in the overlapped structure and the caller can process the
    // results of the read, write, or device control operation.
    //
    return STATUS_PENDING;
}

 
VOID
ToasterProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    )

/*++

New Routine Description:
    The ToasterQueueRequest and ToasterProcessQueuedRequests implement the
    driver-managed IRP queue. These routines are closely related.

    The Toaster sample function driver implements its own driver-managed IRP queue
    instead of using the system-managed IRP queue. When a driver relies on the
    system-managed IRP queue, the driver can only process a single IRP at a time.
    For simple hardware devices, this method is acceptable. However, for more
    complex hardware, such as Toaster class hardware, implementing a
    driver-managed IRP queue is more efficient because it allows multiple threads
    to process IRPs in the function driver simultaneously.

    Because the function driver implements its own mechanism to manage IRPs
    dispatched to it by the system, the function driver does not implement a
    StartIo routine, nor is the driver required to call IoStartNextPacket. These
    system calls are only required when using the system-managed IRP queue.

    ToasterProcessQueuedRequests processes the entire driver-managed IRP queue
    until it is empty. For each queued IRP, ToasterProcessQueuedRequests
    determines if the IRP shold be failed because the hardware instance is no
    longer connected to the computer, or dispatched again to ToasterDispatchIO if
    the hardware instance is present.

    If ToasterDispatchPnP calls ToasterProcessQueuedRequests when processesing
    IRP_MN_REMOVE_DEVICE, or IRP_MN_SURPRISE_REMOVAL, then QueueState should
    equal FailRequests and each IRP in the driver-managed IRP queue is failed with
    STATUS_NO_SUCH_DEVICE.

    If ToasterDispatchPnP calls ToasterProcessQueuedRequests when processesing
    IRP_MN_CANCEL_STOP_DEVICE, IRP_MN_CANCEL_REMOVE_DEVICE, or IRP_MN_START_DEVICE,
    then QueueState should equals AllowRequests and each IRP in the driver-managed
    IRP queue is dispatched to ToasterDispatchIO.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that contains the driver-managed IRP queue.

Return Value Description:
    This routine does not return a value.

--*/
{
    KIRQL               oldIrql;
    PIRP                nextIrp;
    PLIST_ENTRY         listEntry;
    NTSTATUS            status;

    ToasterDebugPrint(TRACE, "Process or fail queued Requests\n");

    for(;;)
    {
        //
        // Enter a loop to process all the IRPs in the driver-managed IRP queue. The
        // loop exits when the queue is empty because either all the IRPs have been
        // processed, or the hardware instance has been disconnected and all the IRPs
        // have been failed.
        //

        KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

        //
        // Determine if the queue is empty. If the queue is empty then break out of
        // the for loop.
        //
        if(IsListEmpty(&FdoData->NewRequestsQueue))
        {
            //
            // It is very important to release the spin lock that protects the queue
            // before breaking out of the for loop. Otherwise a system deadlock
            // could result.
            //
            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            break;
        }

        //
        // Remove the list entry at the head of the queue. The driver-managed IRP
        // queue is processed in a first-in first-out (FIFO) order. The oldest IRP
        // in the queue is processed first.
        //
        listEntry = RemoveHeadList(&FdoData->NewRequestsQueue);

        //
        // Get the next IRP to process from the list entry. The CONTAINING_RECORD
        // macro returns the IRP from the list entry at the head of the queue.
        //
        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

        //
        // Determine if the IRP should be failed because the hardware instance has
        // been disconnected from the computer.
        //
        if (FailRequests == FdoData->QueueState)
        {
            //
            // Complete the IRP with STATUS_NO_SUCH_DEVICE. The IRP’s
            // IoStatus.Information member must also be set to the number of bytes
            // transferred before the IRP is completed. Because no data was
            // transferred for failed IRPs, the member is set to 0.
            //
            nextIrp->IoStatus.Information = 0;

            nextIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

            IoCompleteRequest (nextIrp, IO_NO_INCREMENT);
        }
        else
        {
            //
            // The hardware instance is still connected to the computer. Dispatch the
            // IRP to ToasterDispatchIO. If ToasterDispatchIO returns STATUS_PENDING
            // then that is because QueueState has been changed to HoldRequests. If
            // QueueState has been changed to HoldRequests then the hardware is not
            // ready to process any IRPs and ToasterProcessQueuedRequests should
            // break out of the for loop.
            // 
            // STATUS_PENDING can also be returned if ToasterReadWrite or
            // ToasterDispatchIoctl pass the IRP down the device stack and the
            // underlying bus driver returns STATUS_PENDING. In this case the
            // function driver’s dispatch routines must be modified to return the
            // reason that STATUS_PENDING was returned. Then the logic in this
            // routine must be changed to determine whether to continue to dispatch
            // queued IRPs, or break out of the for loop.
            //
            status = ToasterDispatchIO(FdoData->Self, nextIrp);

            if(STATUS_PENDING == status)
            {
                break;
            }
        }
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

 
NTSTATUS
ToasterDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchPower calls ToasterIoIncrement to increment OutstandingIO.
    ToasterDispatchPower calls ToasterIoDecrement to decrement OutstandingIO after
    it completes the incoming power IRP.

    ToasterDispatchPower fails the incoming power IRP if it determines that the
    hardware instance has been removed.

    Power IRPs are always system-managed and must not be queued in the
    driver-managed IRP queue. A driver must never queue power IRPs. Otherwise, the
    system could stop responding.

    The function driver must immediately process power IRPs and then pass them
    down the device stack to be processed by the underlying bus driver. If the
    function driver cannot immediately process a power IRP, then it should mark
    the power IRP as pending, change the driver-managed IRP queue to begin
    queuing any new non-power IRPs, and then return STATUS_PENDING to the system
    until the function driver completes the pending power IRP.

    This stage of the function driver does not implement support pending power
    IRPs. The Featured1 stage of the function driver implements support for
    pending power IRPs.

Updated Return Value Description:
    ToasterDispatchPower returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Otherwise, ToasterDispatchPower
    returns the value returned by the underlying bus driver.

--*/
{
    PFDO_DATA           fdoData;
    NTSTATUS            status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchPower\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        PoStartNextPowerIrp (Irp);

        status = STATUS_NO_SUCH_DEVICE;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement (fdoData);

        return status;
    }

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    status = PoCallDriver(fdoData->NextLowerDriver, Irp);
    
    ToasterIoDecrement (fdoData);

    return status;
}

 
NTSTATUS
ToasterSystemControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Updated Routine Description:
    ToasterSystemControl calls ToasterIoIncrement to increment OutstandingIO.
    ToasterSystemControl calls ToasterIoDecrement to decrement OutstandingIO after
    it completes the incoming power IRP.

    ToasterSystemControl fails the incoming WMI IRP if it determines that the
    hardware instance has been removed.

Updated Return Value Description:
    ToasterSystemControl returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Otherwise, ToasterSystemControl
    returns the value returned by the underlying bus driver.

--*/
{
    PFDO_DATA               fdoData;
    NTSTATUS                status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "FDO received WMI IRP\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        status = STATUS_NO_SUCH_DEVICE;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement (fdoData);

        return status;
    }

    IoSkipCurrentIrpStackLocation (Irp);

    status = IoCallDriver (fdoData->NextLowerDriver, Irp);

    ToasterIoDecrement (fdoData);

    return status;
}

 
LONG
ToasterIoIncrement    (
    IN  OUT PFDO_DATA   FdoData
    )

/*++

New Routine Description:
    ToasterIoIncrement increments the count of uncompleted IRPs dispatched to the
    function driver. The count of uncompleted IRPs affects the StopEvent and
    RemoveEvent kernel events.

    ToasterIoIncrement unsignals (clears) StopEvent when the count of uncompleted
    IRPs increments from 1 to 2. When StopEvent is unsignaled, ToasterDispatchPnP
    cannot proceed to process IRP_MN_QUERY_STOP_DEVICE or
    IRP_MN_QUERY_REMOVE_DEVICE and must wait until all IRPs are completed by the
    function driver.

    ToasterAddDevice initializes OutstandingIO to 1. That is, when there are
    zero uncompleted IRPs, OutstandingIO equals 1. When there is a single
    uncompleted IRP, then OutstandingIO equals 2.

    ToasterDispatchPnP uses StopEvent to block the execution of any thread
    processing IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE because
    those IRPs must not be passed down the device stack to be processed by the
    underlying bus driver until the hardware instance is idle and no longer
    processing any requests. Otherwise, the function driver would prematurely pass
    those IRPs down the device stack to be processed by the bus driver while the
    hardware instance is processing requests. The bus driver might then reassign
    the hardware resources being used by the hardware instance causing any
    requests being processed by the hardware instance to become invalid.

    When the function driver later calls ToasterIoDecrement when it completes an
    IRP, ToasterIoDecrement signals StopEvent if that IRP is the last uncompleted
    IRP.

    ToasterDispatchPnP uses RemoveEvent to block the execution of any thread
    processing IRP_MN_REMOVE_DEVICE because the IRP must not be passed down the
    device stack to be processed by the bus driver until after StopEvent has been
    signaled. 

    When the ToasterDispatchPnP processes IRP_MN_REMOVE_DEVICE, it calls
    ToasterIoDecrement twice. The function driver only calls ToasterIoDecrement
    twice when ToasterDispatchPnP processes IRP_MN_REMOVE_DEVICE. The second call
    to ToasterIoDecrement signals RemoveEvent, allowing the thread processing
    IRP_MN_REMOVE_DEVICE to resume and pass the IRP down the device stack to be
    processed by the bus driver.

    The function driver’s dispatch routines call ToasterIoIncrement when new IRPs
    are dispatched to them to be processed. When the dispatch routines later
    complete an IRP, they must call ToasterIoDecrement.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that contains the OutstandingIO member, which stores the count of uncompleted
    IRPs dispatched to the function driver. 

Return Value Description:
    ToasterIoIncrement returns the number of uncompleted IRPs after the value has
    been incremented.

--*/

{
    LONG            result;

    //
    // Increment the count of uncompleted IRPs. The InterlockedIncrement function
    // increments the count in a thread-safe manner. Thus, the function driver does
    // not need to define a separate spin lock in the device extension to protect
    // the count from being simultaneously manipulated by multiple threads.
    //
    result = InterlockedIncrement(&FdoData->OutstandingIO);

    //ToasterDebugPrint(TRACE, "ToasterIoIncrement %d\n", result);

    ASSERT(result > 1);

    //
    // Determine if StopEvent should be unsignaled (cleared). StopEvent must only be
    // unsignaled when the count of uncompleted IRPs increments from 1 to 2.
    // Unsignaling StopEvent prevents ToasterDispatchPnP from prematurely passing
    // IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE down the device stack
    // to be processed by the underlying bus driver.
    //
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

New Routine Description:
    ToasterIoDecrement decrements the count of uncompleted IRPs dispatched to the
    function driver. The count of uncompleted IRPs affects the StopEvent and
    RemoveEvent kernel events.

    If the resulting count equals 1, then that indicates the hardware instance is
    stopping. If the resulting count equals 0, then that indicates the device is
    being removed.

    ToasterIoDecrement signals StopEvent when the count of uncompleted IRPs
    decrements from 2 to 1. Signaling StopEvent allows ToasterDispatchPnP to
    proceed and pass IRP_MN_QUERY_STOP_DEVICE or IRP_MN_QUERY_REMOVE_DEVICE down
    the device stack to be processed by the underlying bus driver.

    ToasterAddDevice initializes OutstandingIO to 1. Thus, when the count of
    uncompleted IRPs decrements to 1, then that indicates the hardware instance is
    idle and no longer processing any requests, and can safely be removed without
    losing any data.

    ToasterIoDecrement signals RemoveEvent when the count of uncompleted IRPs
    decrements from 1 to 0. Signaling RemoveEvent allows ToasterDispatchPnP to
    proceed and pass IRP_MN_REMOVE_DEVICE down the device stack to be processed by
    the bus driver.

    ToasterAddDevice initializes OutstandingIO to 1. The only time OutstandingIO
    decrements to 0 is when ToasterDispatchPnP processes IRP_MN_REMOVE_DEVICE and
    calls ToasterIoDecrement one extra time. The transition from 1 to 0 indicates
    that the system is removing the hardware instance. ToasterIoDecrement only
    signals RemoveEvent is after it has signaled StopEvent.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that contains the OutstandingIO member, which stores the count of uncompleted
    IRPs dispatched to the function driver. 

Return Value Description:
    ToasterIoDecrement returns the number of uncompleted IRPs after the value has
    been decremented.

--*/
{
    LONG            result;

    //
    // Decrement the count of uncompleted IRPs. The InterlockedDecrement function
    // decrements the count in a thread-safe manner. Thus, the function driver does
    // not need to define a separate spin lock in its device extension to protect
    // the count from being simultaneously manipulated by multiple threads.
    //
    result = InterlockedDecrement(&FdoData->OutstandingIO);

    //ToasterDebugPrint(TRACE, "ToasterIoDecrement %d\n", result);

    ASSERT(result >= 0);

    //
    // Determine if StopEvent should be signaled. StopEvent must only be signaled
    // when the count of uncompleted IRPs decrements from 2 to 1. A count of 1
    // indicates that there are no more uncompleted IRPs.
    // 
    // Signaling StopEvent allows ToasterDispatchPnP to pass IRP_MN_QUERY_STOP_DEVICE
    // or IRP_MN_QUERY_REMOVE_DEVICE down the device stack to be processed by the
    // underlying bus driver.
    //
    if (1 == result)
    {
        KeSetEvent (&FdoData->StopEvent,
                    IO_NO_INCREMENT,
                    FALSE);
    }

    //
    // Determine if RemoveEvent should be signaled. RemoveEvent must only be signaled
    // after StopEvent has been signaled, when the count of uncompleted IRPs
    // decrements from 1 to 0. A count of 0 indicates that ToasterDispatchPnP is
    // waiting to send IRP_MN_REMOVE_DEVICE down the device stack to be processed by
    // the bus driver.
    //
    if (0 == result)
    {
        //
        // Test the assumption that the toaster instance’s hardware state equals
        // Deleted. RemoveEvent should not be signaled if the hardware instance is
        // still connected to the computer.
        //
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
            KdPrint ((_DRIVER_NAME_": RtlStringCbVPrintfA failed %x \n", status));
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

