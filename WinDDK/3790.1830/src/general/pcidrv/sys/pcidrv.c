
/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    PciDrv.c

Abstract:

    This is a generic WDM sample driver for Intel 82557/82558 
    based PCI Ethernet Adapter (10/100) and Intel compatibles. 
    The WDM interface in this sample is based on the Toaster function 
    driver, and all the code to access the hardware is taken from 
    the E100BEX NDIS miniport sample from the DDK and converted to
    use WDM interfaces instead of NDIS functions.

    This driver can be installed as a standalone driver (genpci.inf)
    for the Intel PCI device or can be used as a network driver by using
    NDIS-WDM driver sample (netdrv.inf) from the DDK as an upper
    device filter. Please read the PCIDRV.HTM file for more information.

Environment:

    Kernel mode


Revision History:

    Created - Eliyas Yakub Mar 3, 2003
    Updated to do asynchrnous start - 5/24/2004
    

--*/
#include "precomp.h"

#if defined(EVENT_TRACING)
//
// Required to start tracing at boot time. For this to work
// our driver tracing control GUID has to be enabled in the global 
// logger. The pcidrv.htm has more information on how to activate 
// the global logger.
//
#define WPP_STRSAFE
#define WPP_GLOBALLOGGER 1 
// 
// The trace message header file must be included in a source file 
// before any WPP macro calls and after defining a WPP_CONTROL_GUIDS 
// macro (defined in pcidrv.h). During the compilation, WPP scans the source  
// files for DoTraceMessage() calls and builds a .tmh file which stores a unique 
// data GUID for each message, the text resource string for each message, 
// and the data types of the variables passed in for each message.  This file  
// is automatically generated and used during post-processing.  
//
#include "pcidrv.tmh"
#endif

//
// Global debug error level
//
#if !defined(EVENT_TRACING)
ULONG DebugLevel = LOUD;
ULONG DebugFlag = 0x0000000F;//0xFFFFFFFE;
#else 
ULONG DebugLevel; // wouldn't be used to control the trace
ULONG DebugFlag;
#endif


GLOBALS Globals;
#define _DRIVER_NAME_ "PCIDRV"

//
// #pragma alloc_text specifies the binary image sections to place 
// compiled routines in. Routines placed in the INIT section are 
// released from memory after DriverEntry returns. Routines placed 
// in the PAGE section can be paged out to disk by the system or 
// the driver as necessary. Routines that are not specified in the
// in the alloc_text are placed in nonpaged memory. This technique 
// reduces the memory footprint of a driver. If a routine is placed 
// in the PAGE section, it must only execute at IRQL < DISPATCH_LEVEL. 
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, PciDrvAddDevice)
#pragma alloc_text (PAGE, PciDrvCreate)
#pragma alloc_text (PAGE, PciDrvClose)
#pragma alloc_text (PAGE, PciDrvDispatchPnp)
#pragma alloc_text (PAGE, PciDrvStartDevice)
#pragma alloc_text (PAGE, PciDrvStartDeviceWorker)
#pragma alloc_text (PAGE, PciDrvUnload)
#pragma alloc_text (PAGE, PciDrvReadRegistryValue)
#pragma alloc_text (PAGE, PciDrvWriteRegistryValue)
#pragma alloc_text (PAGE, PciDrvSendIrpSynchronously)
#pragma alloc_text (PAGE, PciDrvGetDeviceCapabilities)
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver.

Arguments:

    DriverObject - Represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory. 

    RegistryPath - Represents the driver specific path in the Registry. 
    The function driver can use the path to store driver related data 
    between reboots . The path does not store hardware instance specific data.

Return Value:

    DriverEntry returns STATUS_SUCCESS. NTSTATUS values are defined in the DDK
    header file, Ntstatus.h.

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;

#if !defined(WIN2K) && defined(EVENT_TRACING)
    DebugPrint(INFO, DBG_INIT, "Entered DriverEntry of "_DRIVER_NAME_" version "
                "with tracing enabled built on "__DATE__" at  "__TIME__ "\n");   

    //
    // On XP and beyond specify DriverObject and RegisteryPath
    // to initialize software tracing. On Win2K, we have to use 
    // DeviceObject to do the tracing. So we will do that in our 
    // AddDevice.
    // 
    WPP_INIT_TRACING(DriverObject, RegistryPath);
    
#else
    DebugPrint(INFO, DBG_INIT, "Entered DriverEntry of "_DRIVER_NAME_" version built on "
                                              __DATE__" at "__TIME__ "\n");

#endif

#if defined(WIN2K) 
    DebugPrint(INFO, DBG_INIT, "Built in the Win2K build environment\n");
#endif 

    //
    // Save the RegistryPath. We will need it to initialize WMI.
    //

    Globals.RegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    Globals.RegistryPath.Length = RegistryPath->Length;
    Globals.RegistryPath.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       Globals.RegistryPath.MaximumLength,
                                       PCIDRV_POOL_TAG);

    if (!Globals.RegistryPath.Buffer) {

        DebugPrint(ERROR, DBG_INIT,
                "Couldn't allocate pool for registry path.");

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&Globals.RegistryPath, RegistryPath);

    DriverObject->MajorFunction[IRP_MJ_PNP]            = PciDrvDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = PciDrvDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = PciDrvCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]          = PciDrvClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = PciDrvCleanup;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PciDrvDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_READ]           = PciDrvDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_WRITE]           = PciDrvDispatchIO;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PciDrvSystemControl;
    DriverObject->DriverExtension->AddDevice           = PciDrvAddDevice;
    DriverObject->DriverUnload                         = PciDrvUnload;

    return status;
    
}


NTSTATUS
PciDrvAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

Routine Description:

    The system calls PciDrvAddDevice to create and initialize a 
    device object (FDO) to represent a new instance of our device 
    when the PCI bus driver detects that a new instance has been 
    connected to the computer.

    After the FDO has been created and the device extension initialized, 
    the FDO must be attached to the top of the device stack for the 
    new hardware instance.

    We also register an instance of a device interface so that applications
    and other drivers can use this interface to interact with us.

    Remember: We can not touch the hardware or send any non-pnp IRPS to 
    the stack, UNTIL we have received an IRP_MN_START_DEVICE.

Arguments:

    DriverObject - pointer to a DriverObject.

    PhysicalDeviceObject -  Represents a device object that was created by 
                    the underlying bus driver for the new hardware instance. 
                    PhysicalDeviceObject is called the PDO. The PDO forms the 
                    base of the device stack for the new hardware instance. 
                    When the function driver passes IRPs down the device stack,
                    it is usually because they must also be processed by the 
                    underlying bus driver.

Return Value:

    NT status code.

--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_OBJECT          deviceObject = NULL;
    PFDO_DATA               fdoData;
    POWER_STATE             powerState;

    //
    // Call the PAGED_CODE macro because this routine must only execute at
    // IRQL <= DISPATCH_LEVEL. The macro halts the system in the checked build
    // of Windows if IRQL >= DISPATCH_LEVEL.
    //
    PAGED_CODE();

    DebugPrint(TRACE, DBG_PNP, 
        "AddDevice PDO (0x%p)\n", PhysicalDeviceObject);

    //
    // Create a function device object.
    //

    status = IoCreateDevice (DriverObject,
                             sizeof (FDO_DATA),
                             NULL,  // No Name
                             FILE_DEVICE_UNKNOWN,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &deviceObject);


    if (!NT_SUCCESS (status)) {
        //
        // returning failure here prevents the entire stack from functioning,
        // but most likely the rest of the stack will not be able to create
        // device objects either, so it is still OK.
        //
        return status;
    }

    //
    // Initialize the device extension.
    //

    fdoData = (PFDO_DATA) deviceObject->DeviceExtension;

    DebugPrint(INFO, DBG_PNP, "AddDevice FDO (0x%p), DevExt (0x%p)\n", 
                            deviceObject, fdoData);


    fdoData->Signature = PCIDRV_FDO_INSTANCE_SIGNATURE;
    
    //
    // Set the initial state of the FDO
    //

    INITIALIZE_PNP_STATE(fdoData);


    fdoData->UnderlyingPDO = PhysicalDeviceObject;

    //
    // We will hold all requests until we are started.
    // On W2K we will not get any I/O until the entire device
    // is started. On Win9x this may be required.
    //

    fdoData->QueueState = HoldRequests;

    fdoData->Self = deviceObject;
    fdoData->NextLowerDriver = NULL;

    InitializeListHead(&fdoData->NewRequestsQueue);
    KeInitializeSpinLock(&fdoData->QueueLock);

    // 
    // OutstandingIO count is biased to 2. It transitions to 1 if the device
    // needs to be temporarily stopeed. It transitions to zero when the 
    // device needs to be removed.
    //    
    fdoData->OutstandingIO = 2; 

    //
    // Initialize the remove event to Not-Signaled. This event will be
    // set when the OutstandingIO will become 0. 
    // Note: SynchronizationEvent is an autoreset or autoclearing event. 
    // When such an event is set, a single waiting thread becomes eligible 
    // for execution. The Kernel automatically resets the event to the 
    // not-signaled state each time a wait is satisfied.
    //

    KeInitializeEvent(&fdoData->RemoveEvent,
                      SynchronizationEvent,
                      FALSE);
    //
    // Initialize the stop event to Not-Signaled. This event will be set 
    // when the OutstandingIO will become 1.
    //
    KeInitializeEvent(&fdoData->StopEvent,
                      SynchronizationEvent,
                      FALSE);


    SET_FLAG(deviceObject->Flags, DO_DIRECT_IO);


    //
    // Typically, the function driver for a device is its
    // power policy owner, although for some devices another
    // driver or system component may assume this role.
    // Set the initial power state of the device, if known, by calling
    // PoSetPowerState.
    //

    fdoData->DevicePowerState = PowerDeviceD3;
    fdoData->SystemPowerState = PowerSystemWorking;

    powerState.DeviceState = PowerDeviceD3;
    PoSetPowerState ( deviceObject, DevicePowerState, powerState );

    //
    // Initialize Wake structures
    //
    fdoData->WakeState = WAKESTATE_DISARMED; // Initial state
    fdoData->AllowWakeArming = FALSE;

    //
    // Current Wait-Wake IRP
    //
    fdoData->WakeIrp = NULL;

    //
    // Sync event (protects against simultaneous arming/disarming requests)
    //
    KeInitializeEvent( &fdoData->WakeDisableEnableLock,
                       SynchronizationEvent,
                       TRUE );

    //
    // Notification event, used to flush outstanding Wait-Wake IRPs
    //
    KeInitializeEvent( &fdoData->WakeCompletedEvent,
                       NotificationEvent,
                       TRUE );
    //
    // Sync event (protects against simulatenous arming/disarming requests)
    //
    KeInitializeEvent( &fdoData->PowerSaveDisableEnableLock,
                       SynchronizationEvent,
                       TRUE );    
    //
    // Attach our driver to the device stack.
    // The return value of IoAttachDeviceToDeviceStack is the top of the
    // attachment chain.  This is where all the IRPs should be routed.
    //

    fdoData->NextLowerDriver = IoAttachDeviceToDeviceStack (deviceObject,
                                                       PhysicalDeviceObject);
    if(NULL == fdoData->NextLowerDriver) {

        IoDeleteDevice(deviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // The DO_POWER_PAGABLE bit of a device object indicates to the 
    // kernel that the power-handling code of the corresponding
    // driver is pageable, and so must be called at PASSIVE_LEVEL.
    // Before we set it, we must check the deviceobject beneath
    // us to see if the DO_POWER_INRUSH is set. 
    // 
    if (TEST_FLAG(fdoData->NextLowerDriver->Flags, DO_POWER_INRUSH)) {
        //
        // If we set DO_POWER_INRUSH, we shouldn't set DO_POWER_PAGABLE and
        // make sure all our power-handling code is either locked or marked
        // nonpageable because we can get power IRPs at DISPATCH_LEVEL.
        //
        SET_FLAG(deviceObject->Flags, DO_POWER_INRUSH);
    
    } else {
        //
        // Tell the system that our power IRP handling routines are pageable 
        // because our device is a network device and we wouldn't be in paging
        // path. Drivers for storage devices should be careful about setting 
        // this bit because system might decide to create pagefile, crash dump,
        // or hibernation file. They can initially set  DO_POWER_PAGABLE bit
        // and then dynamically change it and lock the power management 
        // code when the system notifies that their device is in paging path
        // by sending IRP_MN_DEVICE_USAGE_NOTIFICATION.
        //        
        SET_FLAG(deviceObject->Flags, DO_POWER_PAGABLE);

    }

    //
    // If the power pageable bit is not set, we will lock the code section
    // that contains all the power handling code so that it can be safely
    // called at IRQL>PASSIVE_LEVEL.
    //
    #pragma warning(disable:4054) 
    //
    // Disable the invalid typecast warning temporarily so that we can compile
    // rest of the driver with W4 option.
    //
    if(!TEST_FLAG(deviceObject->Flags, DO_POWER_PAGABLE))
    {
        fdoData->PowerCodeLockHandle = MmLockPagableCodeSection(
                                                (PVOID)PciDrvDispatchPower);
    }
    #pragma warning(default:4054)    
    
    status = NICInitializeDeviceExtension(fdoData);
    if (!NT_SUCCESS (status)) {
        DebugPrint(ERROR, DBG_PNP,
            "NICInitializeDeviceExtension failed (%x)\n", status);
        IoDetachDevice(fdoData->NextLowerDriver);
        IoDeleteDevice (deviceObject);
        return status;
    }

    //
    // Tell the Plug & Play system that this device will need an interface.
    // Applications use the device interface to interact with the new us.
    //

    status = IoRegisterDeviceInterface (
                PhysicalDeviceObject,
                (LPGUID) &GUID_DEVINTERFACE_PCIDRV,
                NULL,
                &fdoData->InterfaceName);

    if (!NT_SUCCESS (status)) {
        DebugPrint(ERROR, DBG_PNP,
            "AddDevice: IoRegisterDeviceInterface failed (%x)\n", status);
        IoDetachDevice(fdoData->NextLowerDriver);
        IoDeleteDevice (deviceObject);
        return status;
    }
    
    //
    // Clear the DO_DEVICE_INITIALIZING flag.
    // Note: Do not clear this flag until the driver has set the
    // device power state and the power DO flags.
    //

    CLEAR_FLAG(deviceObject->Flags, DO_DEVICE_INITIALIZING);

    return status;

}

NTSTATUS
PciDrvDispatchPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    The plug and play dispatch routine handles all the PNP Irps.    
    Each IRP updates the hardware instance’s PnP state in the device extension.

    All PnP IRPs must be passed down the device stack so that lower drivers can
    also process them by calling IoCallDriver. Some IRPs must be processed 
    by the bus driver before they are processed by the function driver. IRPs 
    that must be processed by the bus driver before the function driver 
    processes call the function driver routine, PciDrvSendIrpSynchronously, 
    to block the thread that is processing them until the bus driver completes
    the IRP. For example, IRP_MN_START_DEVICE must be processed this way. 
    Other IRPs must be processed  by the function driver before they are 
    passed down the device stack. For example, IRP_MN_SURPRISE_REMOVAL is 
    processed this way.

    Before any IRP is passed down the stack, the function driver must set 
    up the I/O stack location for the next lower driver. If the function 
    driver does not modify any data for the IRP, it can skip the I/O stack
    location that contains the data. This eliminates unnecessary memory copy.
    
Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PFDO_DATA               fdoData;
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
    PPNP_DEVICE_STATE       deviceState;

    PAGED_CODE ();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    stack = IoGetCurrentIrpStackLocation (Irp);

    DebugPrint(TRACE, DBG_PNP, "FDO %s \n",
                PnPMinorFunctionString(stack->MinorFunction));

    //
    // Power up the device if it's not surprise-removed.
    //
    if(stack->MinorFunction != IRP_MN_SURPRISE_REMOVAL){
        PciDrvPowerUpDevice(fdoData, TRUE);
    }    
    
    PciDrvIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState) {

        //
        // Since the device is removed, we will fail the request.
        //
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement(fdoData);
        return STATUS_NO_SUCH_DEVICE ;
    }

    
    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:
        //
        // If it's going to take substantial amount of time to start the 
        // device -due to hw self test, link negotiation, etc. - it's better
        // to pend the IRP and do the start operation asynchronously
        // in a workitem. This really improves the booting and 
        // docking time on systems that support Asynchronous start 
        // (such as Longhorn) because the PNP manager,  instead of 
        // waiting for the start to complete,  can start other devices 
        // in parallel. 
        // 
        IoMarkIrpPending(Irp);
        
        IoCopyCurrentIrpStackLocationToNext(Irp);
        //
        // Since start-device should be handle only after the lower
        // device completes it, set a completion routine and in the
        // completion routine, queue a workitem to do the 
        // asynchronous start.
        //
        IoSetCompletionRoutine(Irp,
                               PciDrvDispatchPnpStartComplete,
                               fdoData,
                               TRUE,
                               TRUE,
                               TRUE
                               );

        //
        // We marked the IRP pending above, so we shouldn't really care
        // about the return status of the IoCallDriver and return
        // STATUS_PENDING.
        //
        IoCallDriver(fdoData->NextLowerDriver, Irp);
        return STATUS_PENDING;

    case IRP_MN_QUERY_STOP_DEVICE:


        //
        // OK, we can stop our device.
        // First, don't allow any requests to be passed down.
        //

        SET_NEW_PNP_STATE(fdoData, StopPending);
        
        if(fdoData->IsUpperEdgeNdis){
            //
            // Since NDIS waits for all the pending requests to complete
            // before sending stop, we must fail all the 
            // the incoming new requests. 
            //
            fdoData->QueueState = FailRequests;
        } else {
            fdoData->QueueState = HoldRequests;
        }

        PciDrvWithdrawIrps(fdoData);

        //
        // Wait for the all other I/Os to finish. 1 represents
        // the current query-stop being processed.
        //
        PciDrvReleaseAndWait(fdoData, 1, STOP);

        //
        // We must set Irp->IoStatus.Status to STATUS_SUCCESS before
        // passing it down.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        
        return PciDrvForwardAndForget(fdoData, Irp);

   case IRP_MN_CANCEL_STOP_DEVICE:

        //
        // Send this IRP down and wait for it to come back.
        // Set the QueueState flag to AllowRequests,
        // and process all the previously queued up IRPs.
        // Also check to see whether you have received cancel-stop
        // without first receiving a query-stop. This could happen if someone
        // above us fails a query-stop and passes down the subsequent
        // cancel-stop.
        //

        status = PciDrvSendIrpSynchronously(fdoData->NextLowerDriver,Irp);
    
        if(NT_SUCCESS(status) && StopPending == fdoData->DevicePnPState)
        {

            fdoData->QueueState = AllowRequests;

            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            ASSERT(fdoData->DevicePnPState == Started);
            //
            // Process the queued requests
            //

            PciDrvProcessQueuedRequests(fdoData);
        } 
        break;


    case IRP_MN_STOP_DEVICE:

        //
        // After the stop IRP has been sent to the lower driver object, the
        // driver must not send any more IRPs down that touch the device until
        // another START has occurred.  For this reason we are holding IRPs
        // and waiting for the outstanding requests to be finished when
        // QUERY_STOP is received.
        // IRP_MN_STOP_DEVICE doesn't change anything in this behavior
        // (we continue to hold IRPs until a IRP_MN_START_DEVICE is issued).
        //
        //
        // Mark the device as stopped.
        //

        SET_NEW_PNP_STATE(fdoData, Stopped);

        //
        // This is the right place to actually give up all the resources used
        // This might include calls to IoDisconnectInterrupt, MmUnmapIoSpace,
        // etc.
        //
        status = PciDrvReturnResources(DeviceObject);

        //
        // Disarm WaitWake
        //
        PciDrvDisarmWake(fdoData, TRUE);
        PciDrvDeregisterIdleDetection(fdoData, TRUE);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        return PciDrvForwardAndForget(fdoData, Irp);

    case IRP_MN_QUERY_REMOVE_DEVICE:

        if(fdoData->IsUpperEdgeNdis){
            //
            // Since NDIS waits for all the pending requests to complete
            // before sending Remove, we must fail all the 
            // the incoming new requests. 
            //
            fdoData->QueueState = FailRequests;
        } else {
            fdoData->QueueState = HoldRequests;
        }
        
        PciDrvWithdrawIrps(fdoData);

        SET_NEW_PNP_STATE(fdoData, RemovePending);

        //
        // Disarm if you have armed for Wake. We will rearm in Cancel-remove
        // if the query-remove gets aborted for any reason.
        //
        PciDrvDisarmWake(fdoData, TRUE);
        PciDrvDeregisterIdleDetection(fdoData, TRUE);
        //
        // Wait for the all other I/Os to finish. 1 represents
        // the current query-remove IRP being processed.
        //
        PciDrvReleaseAndWait(fdoData, 1, STOP);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        return PciDrvForwardAndForget(fdoData, Irp);

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        //
        // We need to reset the QueueState flag to ProcessRequest,
        // since the device resume its normal activities.
        // Also check to see whether you have received cancel-remove
        // without first receiving a query-remove. This could happen if
        // someone above us fails a query-remove and passes down the
        // subsequent cancel-remove.
        //
        status = PciDrvSendIrpSynchronously(fdoData->NextLowerDriver,Irp);

        if(NT_SUCCESS(status) && RemovePending == fdoData->DevicePnPState)
        {
            fdoData->QueueState = AllowRequests;

            RESTORE_PREVIOUS_PNP_STATE(fdoData);

            //
            // Process the queued requests that arrived between
            // QUERY_REMOVE and CANCEL_REMOVE.
            //

            PciDrvProcessQueuedRequests(fdoData);

            //
            // Tell the wait-wake logic we can rearm if enabled in registry.
            //
            PciDrvArmForWake(fdoData, TRUE);
            PciDrvRegisterForIdleDetection(fdoData, TRUE);

        }
        break;

   case IRP_MN_SURPRISE_REMOVAL:

        //
        // The device has been unexpectedly removed from the machine
        // and is no longer available for I/O ("surprise" removal).
        // We must return device and memory resources,
        // disable interfaces. We will defer failing any outstanding
        // request to IRP_MN_REMOVE.
        //

        fdoData->QueueState = FailRequests;
        SET_NEW_PNP_STATE(fdoData, SurpriseRemovePending);

        //
        // Fail all the pending request. Since the QueueState is FailRequests
        // PciDrvProcessQueuedRequests will simply flush the queue,
        // completing each IRP with STATUS_NO_SUCH_DEVICE 
        //

        PciDrvProcessQueuedRequests(fdoData);
        //
        // Cancel all the requests from the secondary queues
        //
        PciDrvCancelQueuedReadIrps(fdoData);
        PciDrvCancelQueuedIoctlIrps(fdoData);

        //
        // Disable the device interface.
        //

        status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

        if (!NT_SUCCESS (status)) {
            DebugPrint(ERROR, DBG_PNP,
                "IoSetDeviceInterfaceState failed: 0x%x\n", status);
        }

        //
        // Return any resources acquired during device startup.
        //

        PciDrvReturnResources(DeviceObject);

        //
        // Disarm WaitWake
        //
        PciDrvDisarmWake(fdoData, TRUE);
        PciDrvDeregisterIdleDetection(fdoData, TRUE);

        //
        // Inform WMI to remove this DeviceObject from its
        // list of providers.
        //
        PciDrvWmiDeRegistration(fdoData);

        //
        // We must set Irp->IoStatus.Status to STATUS_SUCCESS before
        // passing it down.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        return PciDrvForwardAndForget(fdoData, Irp);

   case IRP_MN_REMOVE_DEVICE:

        //
        // The Plug & Play system has dictated the removal of this device.  We
        // have no choice but to detach and delete the device object.
        // (If we wanted to express an interest in preventing this removal,
        // we should have failed the query remove IRP).
        //

        SET_NEW_PNP_STATE(fdoData, Deleted);

        if(SurpriseRemovePending != fdoData->PreviousPnPState)
        {
            //
            // This means we are here after query-remove.
            // So first stop the device, disable the interface,
            // return resources, and fail all the pending request,.
            //

            fdoData->QueueState = FailRequests;

            //
            // Fail all the pending request. Since the QueueState is FailRequests
            // PciDrvProcessQueuedRequests will simply flush the queue,
            // completing each IRP with STATUS_NO_SUCH_DEVICE 
            //

            PciDrvProcessQueuedRequests(fdoData);
            //
            // Cancel all the requests from the secondary queues
            //
            PciDrvCancelQueuedReadIrps(fdoData);
            PciDrvCancelQueuedIoctlIrps(fdoData);

            //
            // Disable the Interface
            //

            status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

            if (!NT_SUCCESS (status)) {
                DebugPrint(ERROR, DBG_PNP,
                        "IoSetDeviceInterfaceState failed: 0x%x\n", status);
            }

            //
            // Return hardware resources.
            //

            PciDrvReturnResources(DeviceObject);

            //
            // Inform WMI to remove this DeviceObject from its
            // list of providers.
            //

            PciDrvWmiDeRegistration(fdoData);
        }

        if(fdoData->PowerCodeLockHandle){
            
            MmUnlockPagableImageSection(fdoData->PowerCodeLockHandle);
        }

        //
        // Wait for the all other I/Os to finish. 1 represents
        // the current Remove IRP being processed.
        //
        PciDrvReleaseAndWait(fdoData, 1, REMOVE);

        //
        // Send on the remove IRP.
        // We need to send the remove down the stack before we detach,
        // but we don't need to wait for the completion of this operation
        // (and to register a completion routine).
        //

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

#if defined(WIN2K) && defined(EVENT_TRACING)
        //
        // Since we used deviceobject to initialize the tracing on Win2K
        // we should use the same deviceobject to cleanup before deleting.
        //
        WPP_CLEANUP(fdoData->Self);
        DebugPrint(TRACE, DBG_PNP, 
                "Cleaned up software tracing (%p)\n", fdoData->Self);
#endif 

        //
        // Detach the FDO from the device stack
        //
        IoDetachDevice (fdoData->NextLowerDriver);

        //
        // Free up interface memory
        //

        RtlFreeUnicodeString(&fdoData->InterfaceName);
        IoDeleteDevice (fdoData->Self);

        return status;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:

        //
        // Pass the IRP down because the modification to the Irp is done
        // on the way up.
        //
        PciDrvSendIrpSynchronously(fdoData->NextLowerDriver, Irp);
        if(MP_TEST_FLAG(fdoData, fMP_ADAPTER_REMOVE_IN_PROGRESS))
        {
            deviceState = (PPNP_DEVICE_STATE) &(Irp->IoStatus.Information);
            if(deviceState) {
                SETMASK((*deviceState), PNP_DEVICE_FAILED);
            } else {
                ASSERT(deviceState);
            }
            status = STATUS_SUCCESS;
        }
        break;
        
    default:
        //
        // Pass down all the unhandled Irps.
        //
        return PciDrvForwardAndForget(fdoData, Irp);
        
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    PciDrvIoDecrement(fdoData);

    return status;
}

VOID
PciDrvStartDeviceWorker (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
    )
/*++

Routine Description:

    Workitem callback routine. This routine will start
    the device and complete the pending Start IRP.

Arguments:

   DeviceObject - pointer to a device object.

   Context - pointer to the workitem.
   
Return Value:

    NT status code


--*/    
{
    NTSTATUS    status;
    PFDO_DATA   fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIRP        irp;

    PAGED_CODE();
    //
    // Get the start IRP from the workitem context
    //
    irp = (PIRP)Context->Argument1;

    //
    // Start the device..
    //
    status = PciDrvStartDevice(fdoData, irp);

    irp->IoStatus.Status = status;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    
    PciDrvIoDecrement(fdoData);
    
    IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
    ExFreePool((PVOID)Context);

    return;
}

PciDrvDispatchPnpStartComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
/*++

Routine Description:
    The lower-level drivers have completed the start IRP.
    Queue a workitem so that we can start the device
    asynchronously at PASSIVE_LEVEL.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - start-irp.

   Context - pointer to our device extension
   
Return Value:

    NT status code


--*/
{
    PFDO_DATA               fdoData;
    NTSTATUS                status;

    fdoData = (PFDO_DATA) Context;

    status = PciDrvQueuePassiveLevelCallback(
                     fdoData,
                     PciDrvStartDeviceWorker,
                     (PVOID)Irp, // Pointer to Start-IRP
                     NULL
                    );
    if(!NT_SUCCESS(status)) {
        //
        // Since we are failing Start due to low resources, our device
        // will be removed immediately after this. We will get a 
        // REMOVE_DEVICE requests.
        //
        Irp->IoStatus.Status = status;
        PciDrvIoDecrement(fdoData);        
        return STATUS_CONTINUE_COMPLETION;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
PciDrvDispatchPnpComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
/*++

Routine Description:
    The lower-level drivers have completed the pnp IRP.
    Signal this to whoever registered us.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

   Context - pointer to the event to be signaled.
Return Value:

    NT status code


--*/
{
    PKEVENT             event = (PKEVENT)Context;

    UNREFERENCED_PARAMETER (DeviceObject);

    //
    // If the lower driver didn't return STATUS_PENDING, we don't need to 
    // set the event because we won't be waiting on it. 
    // This optimization avoids grabbing the dispatcher lock and improves perf.
    //
    if (Irp->PendingReturned == TRUE) {
        KeSetEvent (event, IO_NO_INCREMENT, FALSE);
    }

    //
    // The dispatch routine must call IoCompleteRequest
    //

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS 
PciDrvWrite(
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
)
/*++

Routine Description:

    Performs write to the NIC.

Arguments:

   FdoData - pointer to a FDO_DATA structure

   Irp - pointer to an I/O Request Packet.

Return Value:

    NT status code


--*/

{

    return NICWrite(FdoData, Irp);
    
}

NTSTATUS
PciDrvRead (
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
    )
/*++

Routine Description:

    Performs read from the NIC.

Arguments:

   FdoData - pointer to a FDO_DATA structure

   Irp - pointer to an I/O Request Packet.

Return Value:

    NT status code


--*/

{
    NTSTATUS    status;
    KIRQL oldIrql;

    DebugPrint(TRACE, DBG_READ, "--->PciDrvRead %p\n", Irp);

   
    KeAcquireSpinLock(&FdoData->RecvQueueLock, &oldIrql);

    //
    // Since we are queueing the IRP, we should set the cancel routine.
    //
    IoSetCancelRoutine(Irp, PciDrvCancelRoutineForReadIrp);

    //
    // Let us check to see if the IRP is cancelled at this point.
    //
    if(Irp->Cancel && IoSetCancelRoutine(Irp, NULL)){
        //
        // The IRP has been already cancelled but the I/O manager
        // hasn't called the cancel routine yet. So complete the
        // IRP after releasing the lock.
        //
        status = STATUS_CANCELLED;
        
    }else {
        //
        // Queue the IRP and return status pending.
        // 
        IoMarkIrpPending(Irp);
        InsertTailList(&FdoData->RecvQueueHead, 
                &Irp->Tail.Overlay.ListEntry);
        status = STATUS_PENDING;
        
        //
        // The IRP shouldn't be accessed after the lock is released
        // It could be grabbed by another thread or the cancel routine
        // is running.
        //
    }
        
    KeReleaseSpinLock(&FdoData->RecvQueueLock, oldIrql);

    if(status != STATUS_PENDING){
        Irp->IoStatus.Information = 0; 
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement (FdoData);

    }

    DebugPrint(TRACE, DBG_READ, "<-- Read called %x\n", status);
    
    return status;

}


NTSTATUS
PciDrvCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

   Dispatch routine to handle Create commands for the device.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PFDO_DATA    fdoData;
    NTSTATUS     status = STATUS_SUCCESS;

    PAGED_CODE ();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_CREATE_CLOSE, "Create \n");

    //
    // Since we don't access the hardware to process create, we don't have to
    // worry about about the current device state and whether or not to queue
    // this request.
    //

    PciDrvIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState) {

        //
        // Since the device is removed, we will not hold any IRPs.
        // We just fail it.
        //
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement(fdoData);
        return STATUS_NO_SUCH_DEVICE ;
    }

    status = PciDrvPowerUpDevice(fdoData, TRUE);
    
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    PciDrvIoDecrement(fdoData);


    return status;
}


NTSTATUS
PciDrvClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

   Dispatch routine to handle Close for the device.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PFDO_DATA    fdoData;
    NTSTATUS     status;

    PAGED_CODE ();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_CREATE_CLOSE, "Close \n");

    //
    // Since we don't access the hardware to process close requests, we don't
    // have to worry about the current device state and whether or not to queue
    // this request.
    //
    // Note that we don't check to see if the device is deleted. On Win2K and
    // above, we will never see a close IRP after we've been removed (unless
    // some 3rd party driver in our stack is faking them)
    //
    // On Win9x however, we can see these after the remove. For surprise
    // removal, Win2K will send IRP_MN_SURPRISE_REMOVAL, wait for the handles
    // to close, then send IRP_MN_REMOVE_DEVICE. Win9x never sends
    // IRP_MN_SURPRISE_REMOVAL - it immediately sends a IRP_MN_REMOVE_DEVICE,
    // even if we still have open handles! To keep from leaking memory, we
    // will handle the closes even after we are deleted.
    //

    PciDrvIoIncrement (fdoData);

    status = STATUS_SUCCESS;

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    PciDrvIoDecrement(fdoData);


    return status;
}


NTSTATUS
PciDrvDispatchIoctl(
    IN  PFDO_DATA       FdoData,
    IN  PIRP            Irp
    )
/*++
Routine Description:

    Handle IOCTL requests coming from the upper NDIS edge.

Arguments:

   FdoData - pointer to a FDO_DATA structure

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION      pIrpSp;
    NTSTATUS                status= STATUS_SUCCESS;
    ULONG                   FunctionCode;
    ULONG                   bytesReturned;
    KIRQL                   oldIrql;

    DebugPrint(LOUD, DBG_IOCTLS, "Ioctl called %p\n", Irp);

    pIrpSp = IoGetCurrentIrpStackLocation(Irp);

    FunctionCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    bytesReturned = 0;

    switch (FunctionCode)
    {
        case IOCTL_NDISPROT_QUERY_OID_VALUE:

            ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);

            status = NICHandleQueryOidRequest(
                        FdoData,
                        Irp,
                        &bytesReturned
                        );
            bytesReturned += FIELD_OFFSET(NDISPROT_QUERY_OID, Data);
            
            break;

        case IOCTL_NDISPROT_SET_OID_VALUE:

            ASSERT((FunctionCode & 0x3) == METHOD_BUFFERED);

            status = NICHandleSetOidRequest(
                        FdoData,
                        Irp
                        );

            bytesReturned = 0;

            break;

        case IOCTL_NDISPROT_INDICATE_STATUS:

            KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
            
            status = PciDrvQueueIoctlIrp(
                        FdoData,
                        Irp                        
                        );                 
            KeReleaseSpinLock(&FdoData->Lock, oldIrql);
            bytesReturned = 0;            
            break;
            
         default:
            ASSERTMSG(FALSE, "Invalid IOCTL request\n");
            status = STATUS_NOT_SUPPORTED;
            break;
    }

    if (status != STATUS_PENDING)
    {
        Irp->IoStatus.Information = bytesReturned;
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement (FdoData);
    }

    return status;
}

    
NTSTATUS
PciDrvDispatchIO(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++
Routine Description:

    This routine is called by: 
    1) The I/O manager to dispath read, write & IOCTL requests. 
    2) This is called by PciDrvProcessQueuedRequest routine to
        redispatch all queued IRPs. This is done whenever the QueueState
        changes for HoldRequests to AllowRequests.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status= STATUS_SUCCESS;
    PFDO_DATA               fdoData;

    DebugPrint(LOUD, DBG_IOCTLS, "PciDrvDispatchIO called %p\n", Irp);
        
    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation (Irp);

    status = PciDrvPowerUpDevice(fdoData, FALSE);
    if(!NT_SUCCESS(status)){
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
        
    }

    PciDrvIoIncrement (fdoData);
    if (Deleted == fdoData->DevicePnPState) {

        //
        // Since the device is removed, we will fail the request.
        //
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement(fdoData);
        return STATUS_NO_SUCH_DEVICE ;
    }
    
    if (HoldRequests == fdoData->QueueState) {

        //
        // If the UpperEdge is NDIS, we should allow
        // all the power related Set OID requests to be processed 
        // even if the queuestate is in hold. That's
        // the way we can get power management to work.
        //
        if(!(fdoData->IsUpperEdgeNdis 
                && irpStack->MajorFunction == IRP_MJ_DEVICE_CONTROL
                && irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_NDISPROT_SET_OID_VALUE)
                ){
            return PciDrvQueueRequest(fdoData, Irp);
        }

    } 
    
    switch (irpStack->MajorFunction) {
        case IRP_MJ_WRITE:
            status = PciDrvWrite(fdoData, Irp);
            break;
        case IRP_MJ_READ:
            status =  PciDrvRead(fdoData, Irp);    
            break;
        case IRP_MJ_DEVICE_CONTROL:
            status =  PciDrvDispatchIoctl(fdoData, Irp);
            break;
        default:
            ASSERTMSG(FALSE, "PciDrvDispatchIO invalid IRP");
            status = STATUS_UNSUCCESSFUL;
            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);  
            PciDrvIoDecrement (fdoData);
            break;
    }               

    return status;
}


NTSTATUS
PciDrvStartDevice (
    IN PFDO_DATA     FdoData,
    IN PIRP          Irp
    )
/*++

Routine Description:

    Performs whatever initialization is needed to setup the
    device, namely connecting to an interrupt,
    setting up a DMA channel or mapping any I/O port resources.

Arguments:

   Irp - pointer to an I/O Request Packet.

   FdoData - pointer to a FDO_DATA structure

Return Value:

    NT status code


--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    status = NICAllocateDeviceResources(FdoData, Irp);
    if (!NT_SUCCESS (status)){
        DebugPrint(ERROR, DBG_INIT, "NICAllocateDeviceResources failed: 0x%x\n",
                                status);
        return status;
    }
    
    //
    // Start is an implicit power-up. Since we are done programming the
    // hardware, we should be in D0.
    //
    FdoData->DevicePowerState = PowerDeviceD0;

    powerState.DeviceState = PowerDeviceD0;
    PoSetPowerState ( FdoData->Self, DevicePowerState, powerState );

    //
    // Enable the device interface. Return status
    // STATUS_OBJECT_NAME_EXISTS means we are enabling the interface
    // that was already enabled, which could happen if the device
    // is stopped and restarted for resource rebalancing.
    //
    status = IoSetDeviceInterfaceState(&FdoData->InterfaceName, TRUE);

    if (!NT_SUCCESS (status)){
        DebugPrint(ERROR, DBG_INIT, "IoSetDeviceInterfaceState failed: 0x%x\n",
                                status);
        return status;
    }

    //
    // Register with WMI before setting DevicePnPState to Started.
    // Skip registration if we are being restarted for resource
    // rebalance. It's better to register with WMI in start-device
    // rather than AddDevice because that's most logical place to register
    // for FDOs. 
    //
    
    if(FdoData->DevicePnPState != Stopped) { 
        //
        // Okay, this is the first start request for our device.
        //
        status = PciDrvWmiRegistration(FdoData);
        if (!NT_SUCCESS (status)) {
            DebugPrint(ERROR, DBG_INIT, "IoWMIRegistrationControl failed (%x)\n", status);
            return status;
        }
#if defined(WIN2K) && defined(EVENT_TRACING)
        //
        // Win2K requires deviceobject to initialize the tracing. 
        // For XP and beyond we do that in DriverEntry.
        //

        WPP_INIT_TRACING(FdoData->Self, &Globals.RegistryPath);
        DebugPrint(TRACE, DBG_INIT,
                "Enabled software tracing (%p)\n", FdoData->Self);
#endif  
        
    }
    

    SET_NEW_PNP_STATE(FdoData, Started);

    //
    // Mark the device as active and not holding IRPs
    //

    FdoData->QueueState = AllowRequests;

    status = PciDrvGetDeviceCapabilities(FdoData->Self, &FdoData->DeviceCaps);
    if (!NT_SUCCESS (status) ) {
        DebugPrint(ERROR, DBG_INIT, "PciDrvGetDeviceCapabilities failed (%x)\n", status);
        return status;
    }

    DebugPrint(INFO, DBG_INIT, "Device does%s support power management\n",
            IsPoMgmtSupported(FdoData)?"":" not");
    
    //
    // Adjust capabilities to handle device Wake.
    //
    if(!FdoData->IsUpperEdgeNdis){
        PciDrvAdjustCapabilities(&FdoData->DeviceCaps);
    }
    
    //
    // Tell the wait-wake logic we can rearm if enabled in the registry.
    //
    PciDrvArmForWake(FdoData, TRUE);

    //
    // Register for idle detection if enabled in the registry
    //
    PciDrvRegisterForIdleDetection(FdoData, TRUE);        
    
    //
    // The last thing to do is to process any pending queued IRPs.
    //

    PciDrvProcessQueuedRequests(FdoData);

    return status;

}


NTSTATUS
PciDrvCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    The dispatch routine for cleanup. This routine walk the Irp queue
    and cancels all the pending requests for which the file object is 
    the same with the one in the Irp.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PFDO_DATA              fdoData;
    KIRQL                  oldIrql;
    LIST_ENTRY             cleanupList;
    PLIST_ENTRY            thisEntry, nextEntry, listHead;
    PIRP                   pendingIrp;
    PIO_STACK_LOCATION     pendingIrpStack, irpStack;


    DebugPrint(TRACE, DBG_CREATE_CLOSE, "Cleanup called\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PciDrvIoIncrement (fdoData);

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    InitializeListHead(&cleanupList);

    //
    // We must acquire queue lock first.
    //

    KeAcquireSpinLock(&fdoData->QueueLock, &oldIrql);

    //
    // Walk through the list and remove all the IRPs
    // that belong to the input irp's fileobject.
    //

    listHead = &fdoData->NewRequestsQueue;
    for(thisEntry = listHead->Flink,nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry,nextEntry = thisEntry->Flink)
    {

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP,
                                Tail.Overlay.ListEntry);
        pendingIrpStack = IoGetCurrentIrpStackLocation(pendingIrp);

        if (irpStack->FileObject == pendingIrpStack->FileObject)
        {
            RemoveEntryList(thisEntry);

            //
            // Set the cancel routine to NULL
            //
            if(NULL == IoSetCancelRoutine (pendingIrp, NULL))
            {
                //
                // The cancel routine has run but it must be waiting to hold
                // the queue lock. It will cancel the IRP as soon as we
                // drop the lock outside this loop. We will initialize
                // the IRP's listEntry so that the cancel routine wouldn't barf
                // when it tries to remove the IRP from the queue, and
                // leave the this IRP alone.
                //
                InitializeListHead(thisEntry);
            } else {
                //
                // Cancel routine is not called and will never be
                // called. So we queue the IRP in the cleanupList
                // and cancel it after dropping the lock
                //
                InsertTailList(&cleanupList, thisEntry);
            }
        }
    }

    //
    // Release the spin lock.
    //

    KeReleaseSpinLock(&fdoData->QueueLock, oldIrql);

    //
    // Walk through the cleanup list and cancel all
    // the Irps.
    //

    while(!IsListEmpty(&cleanupList))
    {
        //
        // Complete the IRP
        //

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
    PciDrvIoDecrement (fdoData);
    return STATUS_SUCCESS;
}

VOID
PciDrvUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:

    Free all the resources allocated in DriverEntry.

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
    // driver must have already been deleted.
    //
    ASSERT(DriverObject->DeviceObject == NULL);

    //
    // We should not be unloaded until all the devices we control
    // have been removed from our queue.
    //
    DebugPrint(INFO, DBG_INIT, "Unload\n");

    if(Globals.RegistryPath.Buffer)
        ExFreePool(Globals.RegistryPath.Buffer);

#if !defined(WIN2K) && defined(EVENT_TRACING)
    // 
    // Cleanup using DriverObject on XP and beyond.
    //
    WPP_CLEANUP(DriverObject);
#endif

    return;
}



NTSTATUS
PciDrvQueueRequest    (
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    )

/*++

Routine Description:

    This function queues IRPs in the NewRequestsQueue. This queue is 
    used to temporarily hold the request when the device is changing
    PNP state. For example:
    1) All the incoming requests are queued here when the device 
    is stopped (IRP_MN_QUERY_STOP_DEVICE ) for resource rebalancing. 
    2) Requests are queued when the device is in the process of 
    being query-removed (IRP_MN_QUERY_REMOVE_DEVICE).
    3) If there are requests pending in other queues during stop, remove
    or suspend, they are temporarily withdrawn from there and moved 
    into this queue until the device returns to Started state.
    4) All the incoming requests are queued when the device is in a
    low power state.
    
Arguments:

    FdoData - pointer to the device's extension.

    Irp - the request to be queued.

Return Value:

    NT status code.

--*/
{

    KIRQL               oldIrql;
    PDRIVER_CANCEL      ret;
    
    DebugPrint(TRACE, DBG_QUEUEING, "Queuing Requests\n");

    IoMarkIrpPending(Irp);

    KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

    //
    // Check whether we are allowed to queue requests.
    //
    if(!FdoData->IsUpperEdgeNdis){
        ASSERT(HoldRequests == FdoData->QueueState);
    }
    //
    // Set the CancelRoutine so that it can cancelled
    // when it's waiting in the queue. Please note that
    // you must mark the irp pending before you set the 
    // CancelRoutine so that the IRP can be completed in 
    // a different thread context.
    //
    IoSetCancelRoutine (Irp, PciDrvCancelRoutine);

    //
    // Before we queue the request, let us check to see
    // if the IRP has been cancelled.
    //
    if(Irp->Cancel)
    {
        //
        // Yes, it is. Check whether the cancel routine
        // is already called.
        //
        ret = IoSetCancelRoutine (Irp, NULL);
        //
        // Initialize the listhead so that in case the CancelRoutine
        // is already invoked, it will not barf while trying to 
        // remove the IRP from the queue after QueueLock is released 
        // (below).
        //
        InitializeListHead(&Irp->Tail.Overlay.ListEntry);
        KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);        
        if(ret != NULL)
        {
            //
            // CancelRoutine hasn't be invoked and it will never be
            // as we have reset it to NULL.
            //
            Irp->IoStatus.Status = STATUS_CANCELLED;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            //
            // Since we marked the IRP pending, we must return
            // STATUS_PENDING even though the IRP has been 
            // completed with STATUS_CANCELLED.
            //
        } else {
            //
            // The CancelRoutine is called already. So let us just
            // return STATUS_PENDING.
            // 
        }
    }else {     
        //
        // IRP is not cancelled, so insert it in the queue.
        //
        InsertTailList(&FdoData->NewRequestsQueue,
                                                &Irp->Tail.Overlay.ListEntry);
        KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
    }

    PciDrvIoDecrement(FdoData);
    return STATUS_PENDING;
}


VOID
PciDrvProcessQueuedRequests    (
    IN OUT PFDO_DATA FdoData
    )

/*++

Routine Description:

    Removes and processes the entries in the queue. Requests are 
    either redispatched for processing or failed if the device
    is in a Deleted State.


Arguments:

    FdoData - pointer to the device's extension (where is the held IRPs queue).


Return Value:

    VOID.

--*/
{

    KIRQL               oldIrql;
    PIRP                nextIrp;
    PLIST_ENTRY         listEntry;
    ULONG               nIrpsReDispatched = 0; // For debugging purposes.
    PDRIVER_CANCEL      cancelRoutine;
    
    DebugPrint(TRACE, DBG_QUEUEING, "-->PciDrvProcessQueuedRequests\n");

    //
    // We need to dequeue all the entries in the queue, reset the cancel
    // routine for each of them and then process then:
    // - if the device is active, we will send them down
    // - else we will complete them with STATUS_NO_SUCH_DEVICE 
    //

    for(;;)
    {
        //
        // Acquire the queue lock before manipulating the list entries.
        //
        KeAcquireSpinLock(&FdoData->QueueLock, &oldIrql);

        if(IsListEmpty(&FdoData->NewRequestsQueue))
        {
            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
            break;
        }

        //
        // Remove a request from the queue.
        //
        listEntry = RemoveHeadList(&FdoData->NewRequestsQueue);

        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        cancelRoutine = IoSetCancelRoutine (nextIrp, NULL);
        
        //
        // Check to see if it's cancelled.
        //
        if (nextIrp->Cancel)
        {
            //
            // The I/O manager sets the CancelRoutine pointer in the IRP
            // to NULL before calling the routine. So check the previous
            // value to decide whether cancel routine has been
            // called or not.
            //
            if(cancelRoutine)
            {
                //
                // This IRP was just cancelled but the cancel routine
                // hasn't been called yet. So it's safe to cancel the IRP,
                // Let's complete the IRP after releasing the lock. 
                // This is to ensure that we don't call out of the driver 
                // while holding a lock.
                //
                KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
                nextIrp->IoStatus.Information = 0;
                nextIrp->IoStatus.Status = STATUS_CANCELLED;
                IoCompleteRequest(nextIrp, IO_NO_INCREMENT);

            } else {
                //
                // The cancel routine has run but it must be waiting to hold
                // the queue lock. It will cancel the IRP as soon as we
                // drop the lock. So initialize the IRPs
                // listEntry so that the cancel routine wouldn't barf
                // when it tries to remove the IRP from the queue.
                //
                InitializeListHead(listEntry);
                KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);
            }

        }
        else
        {
            //
            // Release the lock before we call out of the driver
            //

            KeReleaseSpinLock(&FdoData->QueueLock, oldIrql);

            if (FailRequests == FdoData->QueueState) {
                //
                // The device was removed, we need to fail the request
                //
                nextIrp->IoStatus.Information = 0;
                nextIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE ;
                IoCompleteRequest (nextIrp, IO_NO_INCREMENT);

            } else if (HoldRequests == FdoData->QueueState) {
                //
                // QueueStatus has changed again to HoldRequest, so queue the
                // the currentIrp back and break out of the loop.
                // Increment the I/O count to compensate for the decrement
                // done in the PciDrvQueueRequest routine.
                //
                PciDrvIoIncrement (FdoData);                
                PciDrvQueueRequest(FdoData, nextIrp);
                break;
                
            } else {
                //
                // Re-dispatch the IRP. 
                //
                PciDrvDispatchIO(FdoData->Self, nextIrp);
                nIrpsReDispatched++;            
            }

        }
    }// end of loop

    DebugPrint(TRACE, DBG_QUEUEING, "<--PciDrvProcessQueuedRequests %d\n", nIrpsReDispatched);

    return;

}

VOID
PciDrvCancelRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

/*++

Routine Description:

    The cancel routine for IRPs waiting in the NewRequestsQueue
    It will remove the IRP from the queue and will complete it. 
    The cancel spin lock is already acquired when this routine is called. 
  
Arguments:

    DeviceObject - pointer to the device object.

    Irp - pointer to the IRP to be cancelled.


Return Value:

    VOID.

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_QUEUEING, "Canceling Requests\n");

    //
    // Release the cancel spinlock but stay at DISPATCH_LEVEL
    // because we are going to acquire spinlock again.
    //
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

    //
    // Acquire the local spinlock and take advantage of the fact that you
    // are running at DPC level.
    //
    KeAcquireSpinLockAtDpcLevel(&fdoData->QueueLock);


    //
    // Remove the cancelled IRP from queue and
    // release the queue lock.
    //
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    //
    // Release the spinlock but stay at DISPATCH_LEVEL
    //
    KeReleaseSpinLockFromDpcLevel(&fdoData->QueueLock);

    //
    // Finally lower the IRQL to the value (Irp->CancelIrql) it was 
    // when the I/O manager called our CancelRoutine.
    //
    KeLowerIrql(Irp->CancelIrql);
    
    //
    // Complete the request with STATUS_CANCELLED.
    //
    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return;

}

VOID
PciDrvCancelRoutineForReadIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

/*++

Routine Description:

    The cancel routine for IRPs waiting in the RecvQueue. 
    The cancel spin lock is already acquired when this routine is called. 
 
Arguments:

    DeviceObject - pointer to the device object.

    Irp - pointer to the IRP to be cancelled.


Return Value:

    VOID.

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_QUEUEING, "Canceling Read Request\n");

    //
    // Release the cancel spinlock but stay at DISPATCH_LEVEL
    // because we are going to acquire spinlock again.
    //

    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

    //
    // Acquire the local spinlock and take advantage of the fact
    // that you are running at DISPATCH_LEVEL.
    //

    KeAcquireSpinLockAtDpcLevel(&fdoData->RecvQueueLock);


    //
    // Remove the cancelled IRP from queue and
    // release the queue lock.
    //
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    //
    // Release the spinlock but stay at DISPATCH_LEVEL
    //
    KeReleaseSpinLockFromDpcLevel(&fdoData->RecvQueueLock);

    //
    // Finally lower the IRQL to the value (Irp->CancelIrql) it was 
    // when the I/O manager called our CancelRoutine.
    //
    KeLowerIrql(Irp->CancelIrql);

    //
    // Complete the request with STATUS_CANCELLED.
    //

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    PciDrvIoDecrement (fdoData);
    
    return;

}


NTSTATUS
PciDrvQueueIoctlIrp(
    IN  PFDO_DATA               FdoData,
    IN  PIRP                    Irp
    )
/*++

Routine Description:

    This queues the ioctl requests. Since the number of outstanding
    ioctls at any time are only 3, we save the IRP pointers in the
    device extension instead of adding them to a list-entry. The
    upper edge miniport driver sends IOCTLs requests in response to
    query & set OID request from NDIS, which are serialized.
    
Arguments:


    Assumption:  FdoData->Lock is acquired.

Return Value:

    NTSTATUS code
    
--*/
{
    NTSTATUS            status = STATUS_PENDING;
    PIO_STACK_LOCATION  pIrpSp = NULL;   
    
    DebugPrint(TRACE, DBG_IOCTLS, "-->NICQueueIoctlIrp\n");

    
    pIrpSp = IoGetCurrentIrpStackLocation(Irp);
    
    //
    // Let us check to see if the IRP is cancelled at this point.
    //
    if(Irp->Cancel){
        return STATUS_CANCELLED;        
    }
    
    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_NDISPROT_QUERY_OID_VALUE:
            if(FdoData->QueryRequest){
               status = STATUS_INVALID_DEVICE_REQUEST;
               ASSERTMSG("More than one Request", FALSE);
               break;                    
            }
            FdoData->QueryRequest = Irp;

           break;

        case IOCTL_NDISPROT_SET_OID_VALUE:
            if(FdoData->SetRequest){
               status = STATUS_INVALID_DEVICE_REQUEST;
               ASSERTMSG("More than one Request", FALSE);
               break;                    
            }
            FdoData->SetRequest = Irp;

            break;
            
        case IOCTL_NDISPROT_INDICATE_STATUS:
            if(FdoData->StatusIndicationIrp){
               status = STATUS_INVALID_DEVICE_REQUEST;
               ASSERTMSG("More than one Request", FALSE);
               break;                    
            }
            FdoData->StatusIndicationIrp = Irp;
            break;
            
        default:
            ASSERTMSG("Unkwon ioctl\n", FALSE);
            break;
    }        


    if(status == STATUS_PENDING){

        IoMarkIrpPending(Irp);        
        //
        // Since we are queueing the IRP, we should set the cancel routine.
        //
        IoSetCancelRoutine(Irp, PciDrvCancelRoutineForIoctlIrp);
        
    }
            
    DebugPrint(TRACE, DBG_IOCTLS, "<--NICQueueIoctlIrp\n");

    return status;
    
}

VOID
PciDrvCancelRoutineForIoctlIrp(
    IN PDEVICE_OBJECT               DeviceObject,
    IN PIRP                         Irp
    )
/*++

Routine Description:

    Cancel the pending ioctl IRPs registered by the app or 
    another driver. Assumption: CancelSpinLock is already
    acquired.
    
Arguments:

    DeviceObject - pointer to our device object
    Irp - IRP to be cancelled

Return Value:

    None

--*/
{
    PFDO_DATA               fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION      pIrpSp = NULL;   

    DebugPrint(TRACE, DBG_IOCTLS, "-->NICCancelIoctlIrp\n");
    
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);

    pIrpSp = IoGetCurrentIrpStackLocation(Irp);

    KeAcquireSpinLockAtDpcLevel(&fdoData->Lock);

    switch (pIrpSp->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_NDISPROT_QUERY_OID_VALUE:
            fdoData->QueryRequest = NULL;

           break;

        case IOCTL_NDISPROT_SET_OID_VALUE:
            fdoData->SetRequest = NULL;

            break;
            
        case IOCTL_NDISPROT_INDICATE_STATUS:

            fdoData->StatusIndicationIrp = NULL;
            break;
        default:
            ASSERTMSG("Unkwon ioctl\n", FALSE);
            break;
    }        


    KeReleaseSpinLockFromDpcLevel(&fdoData->Lock);
    //
    // Finally lower the IRQL to the value (Irp->CancelIrql) it was 
    // when the I/O manager called our CancelRoutine.
    //
    KeLowerIrql(Irp->CancelIrql);
    
    
    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    PciDrvIoDecrement (fdoData);

    DebugPrint(LOUD, DBG_IOCTLS, "<--NICCancelIoctlIrp\n");

    return;

}

VOID 
PciDrvCancelQueuedReadIrps(
    PFDO_DATA FdoData
    )
/*++

Routine Description:

    Cancel all the read IRPs waiting in the RecvQueue.
    
Arguments:

    FdoData - Pointer to the device extension.
    
Return Value:

    None

--*/
{
    KIRQL               oldIrql;
    PIRP                irp;
    PLIST_ENTRY         listEntry;

    //
    // Acquire the queue lock before manipulating the list entries.
    //
    KeAcquireSpinLock(&FdoData->RecvQueueLock, &oldIrql);

    while(TRUE){
        
        if(IsListEmpty(&FdoData->RecvQueueHead))
        {
            KeReleaseSpinLock(&FdoData->RecvQueueLock, oldIrql);
            break;
        }

        //
        // Remove a request from the queue.
        //
        listEntry = RemoveHeadList(&FdoData->RecvQueueHead);

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        //
        // Set the cancel routine to NULL. This is an atomic operation.
        //
        if (IoSetCancelRoutine(irp, NULL))
        {
            //
            // Cancel routine cannot run now and cannot have already 
            // started to run.
            //
            KeReleaseSpinLock(&FdoData->RecvQueueLock, oldIrql);
            irp->IoStatus.Status = STATUS_CANCELLED;
            irp->IoStatus.Information = 0;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            PciDrvIoDecrement (FdoData);            
            KeAcquireSpinLock(&FdoData->RecvQueueLock, &oldIrql);            
        }
        else
        {
            //
            // Cancel rotuine is running. Leave the irp alone.
            //
            irp = NULL;
        }
    }
        
}

VOID
PciDrvCancelQueuedIoctlIrps(
    IN PFDO_DATA FdoData
    )
/*++

Routine Description:

    Cancel all the IOCLTs IRPs waiting in the RecvQueue.
    
Arguments:

    FdoData - Pointer to the device extension.
    
Return Value:

    None

--*/
{
    PIRP        irp = NULL;
    KIRQL       oldIrql;
    int         i;
    
    KeAcquireSpinLock(&FdoData->Lock, &oldIrql);

    for(i=0; i<3; i++) // 3 is number of ioctls we handle
    {        
        switch(i){
            case 0:
                irp = FdoData->QueryRequest;
                FdoData->QueryRequest = NULL;
                break;
            case 1:
                irp = FdoData->SetRequest;
                FdoData->SetRequest = NULL;
                break;
            case 2:
                irp = FdoData->StatusIndicationIrp;
                FdoData->StatusIndicationIrp = NULL;
                break;
                
            default: 
                ASSERT(FALSE);
                break;
         }
                
        if(irp){
        
            if(IoSetCancelRoutine(irp, NULL)){
                //
                // Cancel routine cannot run now and cannot have already 
                // started to run. 
                // Release the lock before completing the request.
                //
                KeReleaseSpinLock(&FdoData->Lock, oldIrql);
                
                irp->IoStatus.Information = 0;
                irp->IoStatus.Status = STATUS_CANCELLED;
                IoCompleteRequest(irp, IO_NO_INCREMENT);
                irp = NULL;
                KeAcquireSpinLock(&FdoData->Lock, &oldIrql);                
                PciDrvIoDecrement (FdoData);
                
             }else {
                //
                // Cancel rotuine is running. Leave the irp alone.
                //
                irp = NULL;
             }
        }
    }
    
    KeReleaseSpinLock(&FdoData->Lock, oldIrql);

    return;
}



VOID 
PciDrvWithdrawReadIrps(
    PFDO_DATA FdoData
    )
/*++

Routine Description:

    Retrieve IRPs from the RecvQueue and move it to the 
    NewRequestsQueue. This is done to adjust the 
    outstanding I/O counts and stop the processing of IRPs 
    in the secondary queue while the device is changing state.
    Note that requests waiting in the NewRequestsQueues are not
    counted as active IRPs.
    
Arguments:

    FdoData - Pointer to the device extension.
    
Return Value:

    None

--*/
{
    KIRQL               oldIrql;
    PIRP                irp;
    PLIST_ENTRY         listEntry;

    //
    // Acquire the queue lock before manipulating the list entries.
    //
    KeAcquireSpinLock(&FdoData->RecvQueueLock, &oldIrql);

    while(TRUE){
        
        if(IsListEmpty(&FdoData->RecvQueueHead))
        {
            KeReleaseSpinLock(&FdoData->RecvQueueLock, oldIrql);
            break;
        }

        //
        // Remove a request from the queue.
        //
        listEntry = RemoveHeadList(&FdoData->RecvQueueHead);

        irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        //
        // Set the cancel routine to NULL. This is an atomic operation.
        //
        if (IoSetCancelRoutine(irp, NULL))
        {
            //
            // Cancel routine cannot run now and cannot have already 
            // started to run.
            //
            KeReleaseSpinLock(&FdoData->RecvQueueLock, oldIrql);            
            PciDrvQueueRequest(FdoData, irp);            
            KeAcquireSpinLock(&FdoData->RecvQueueLock, &oldIrql);            
        }
        else
        {
            //
            // Cancel rotuine is running. Leave the irp alone.
            //
            irp = NULL;
        }
    }
        
}

VOID 
PciDrvWithdrawIoctlIrps(
    PFDO_DATA FdoData
    )
/*++

Routine Description:

    Retrieve all the pending ioctl IRPs and move it to the 
    NewRequestsQueue. This is done to adjust the 
    outstanding I/O counts and stop the processing of IRPs 
    in the secondary queue while the device is changing state.
    Note that requests waiting in the NewRequestsQueues are not
    counted as active IRPs.
    
Arguments:

    FdoData - Pointer to the device extension.
    
Return Value:

    None

--*/
{
    KIRQL               oldIrql;

    KeAcquireSpinLock(&FdoData->Lock, &oldIrql);

    if(FdoData->QueryRequest){        
        PciDrvQueueRequest(FdoData, FdoData->QueryRequest);           
        FdoData->QueryRequest = NULL;        
    }
        
    if(FdoData->SetRequest){
        PciDrvQueueRequest(FdoData, FdoData->SetRequest);   
        FdoData->SetRequest = NULL;
    }
    
    if(FdoData->StatusIndicationIrp){
        PciDrvQueueRequest(FdoData, FdoData->StatusIndicationIrp);   
        FdoData->StatusIndicationIrp = NULL;        
    }
    
    KeReleaseSpinLock(&FdoData->Lock, oldIrql);            
}

VOID 
PciDrvWithdrawIrps(
    PFDO_DATA FdoData
    )
/*++

Routine Description:

    This routine moves IRPs from secondary queues to main incoming 
    requests queue while the device is changing states.

Arguments:

    FdoData - pointer to a FDO_DATA structure


Return Value:

    VOID
    
--*/
{
    PciDrvWithdrawIoctlIrps(FdoData);
    
    PciDrvWithdrawReadIrps(FdoData);
    
    return;
}

NTSTATUS
PciDrvReturnResources (
    IN PDEVICE_OBJECT DeviceObject
    )

/*++

Routine Description:

    This routine returns all the resources acquired during
    device startup.

Arguments:

    DeviceObject - pointer to the device object.


Return Value:

    STATUS_SUCCESS if the device can be safely removed, an appropriate
    NT Status if not.

--*/
{

    PFDO_DATA       fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    NTSTATUS        status;
    POWER_STATE     powerState;

    DebugPrint(INFO, DBG_PNP, "-->PciDrvReturnResources\n");

    //
    // Make sure we are called to free resources when the device is stopped
    // query-removed or suprise-removed. Rules are different when NDIS is
    // installed as an upper edge.
    //
    if(!fdoData->IsUpperEdgeNdis){
        ASSERT(fdoData->DevicePnPState == Stopped ||
                fdoData->DevicePnPState == SurpriseRemovePending ||
                fdoData->DevicePnPState == Deleted);
    }

    //
    // Set the flag so that our WatchDogTimerDpc can exit gracefully.
    //
    MP_SET_FLAG(fdoData, fMP_ADAPTER_HALT_IN_PROGRESS);

    //
    // Touch the hardware to shutdown only f the device is 
    // not surprise-removed.
    //
    if(fdoData->DevicePnPState != SurpriseRemovePending){
        //
        // Remove is an implicit power-down. Since the PCI bus driver
        // writes to the power register to put the device in D3, we
        // will not worry about powering down. We just inform the system
        // about the new device power state.
        //
        if(fdoData->DevicePnPState == Deleted){
            powerState.DeviceState = PowerDeviceD3;
            PoSetPowerState(fdoData->Self, DevicePowerState, powerState);            
        }
        //
        // Reset and put the device into a known initial state.
        //
        NICShutdown(fdoData);
    }
    
    status = NICFreeDeviceResources(fdoData); 

    MP_CLEAR_FLAG(fdoData, fMP_ADAPTER_HALT_IN_PROGRESS);

    DebugPrint(INFO, DBG_PNP, "<--PciDrvReturnResources\n");
    
    return status;
}

NTSTATUS
PciDrvForwardAndForget(
    IN PFDO_DATA     FdoData,
    IN PIRP          Irp
    )
/*++

Routine Description:

    Sends the Irp down the stack and doesn't care what happens
    to the IRP after that.

Arguments:

    FdoData - pointer to the device extension.

    Irp - pointer to the current IRP.

Return Value:

    NT status code

--*/
{
    NTSTATUS status;
    
    IoSkipCurrentIrpStackLocation (Irp);
    status = IoCallDriver (FdoData->NextLowerDriver, Irp);
    PciDrvIoDecrement(FdoData);

    return status;    
}

NTSTATUS
PciDrvSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Sends the Irp down the stack and waits for it to complete.

Arguments:
    DeviceObject - pointer to the device object.

    Irp - pointer to the current IRP.

    NotImplementedIsValid -

Return Value:

    NT status code

--*/
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(Irp,
                           PciDrvDispatchPnpComplete,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    status = IoCallDriver(DeviceObject, Irp);

    //
    // Wait for lower drivers to be done with the Irp.
    // Important thing to note here is when you allocate
    // memory for an event in the stack you must do a
    // KernelMode wait instead of UserMode to prevent
    // the stack from getting paged out.
    //
    //

    if (status == STATUS_PENDING) {
       status = KeWaitForSingleObject(&event,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL
                             );       
       ASSERT(NT_SUCCESS(status)); 
       status = Irp->IoStatus.Status;
    }

    return status;
}


NTSTATUS
PciDrvQueuePassiveLevelCallback(
    IN PFDO_DATA            FdoData,
    IN PIO_WORKITEM_ROUTINE CallbackFunction,
    IN PVOID                Context1,
    IN PVOID                Context2
    )
/*++

Routine Description:

    This routine is used to queue workitems so that the callback
    functions can be executed at PASSIVE_LEVEL in the conext of 
    a system thread.

Arguments:

   DeviceObject - pointer to a device extenion.

   CallbackFunction - Function to invoke when at PASSIVE_LEVEL.

   Context1 & 2 - Meaning of the context values depends on the callback function.
    
Return Value:

--*/
{
    PIO_WORKITEM            item = NULL;
    NTSTATUS                status = STATUS_SUCCESS;
    PWORK_ITEM_CONTEXT      context;

    context = ExAllocatePoolWithTag(
                            NonPagedPool,
                            sizeof(WORKER_ITEM_CONTEXT),
                            PCIDRV_POOL_TAG
                            );

    if (NULL == context) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    //
    // Allocate a workitem and queue it for passive level processing.
    // Note - Windows 98 doesn't support IoWorkItem's, it only supports the
    //        ExWorkItem version. We use the Io versions here because the
    //        Ex versions can cause system crashes during unload (ie the driver
    //        can unload in the middle of an Ex work item callback)
    //
    item = IoAllocateWorkItem(FdoData->Self);

    if (NULL != item) {

        context->WorkItem = item;
        context->Argument1 = Context1;
        context->Argument2 = Context2;
        
        IoQueueWorkItem( item,
                        CallbackFunction,
                        DelayedWorkQueue,
                        context
                        );
    }else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    
    return status;
}

NTSTATUS
PciDrvGetDeviceCapabilities(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities
    )
/*++

Routine Description:

    This routine sends the get capabilities irp to the given stack

Arguments:

    DeviceObject        A device object in the stack whose capabilities we want
    DeviceCapabilites   Where to store the answer

Return Value:

    NTSTATUS

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
    // Initialize the capabilities that we will send down
    //
    RtlZeroMemory( DeviceCapabilities, sizeof(DEVICE_CAPABILITIES) );
    DeviceCapabilities->Size = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Version = 1;
    DeviceCapabilities->Address = 0xFFFFFFFF;//junk;
    DeviceCapabilities->UINumber = 0xFFFFFFFF;//junk

    //
    // Initialize the event
    //
    KeInitializeEvent( &pnpEvent, NotificationEvent, FALSE );

    targetObject = IoGetAttachedDeviceReference( DeviceObject );

    //
    // Build an Irp
    //
    pnpIrp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                            targetObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &pnpEvent,
                                            &ioStatus
                                            );
    if (pnpIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto GetDeviceCapabilitiesExit;

    }

    //
    // Pnp Irps all begin life as STATUS_NOT_SUPPORTED;
    //
    pnpIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    //
    // Get the top of stack
    //
    irpStack = IoGetNextIrpStackLocation( pnpIrp );

    //
    // Set the top of stack
    //
    RtlZeroMemory( irpStack, sizeof(IO_STACK_LOCATION ) );
    irpStack->MajorFunction = IRP_MJ_PNP;
    irpStack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
    irpStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    //
    // Call the driver
    //
    status = IoCallDriver( targetObject, pnpIrp );
    if (status == STATUS_PENDING) {

        //
        // Block until the irp comes back.
        // Important thing to note here is when you allocate 
        // the memory for an event in the stack you must do a  
        // KernelMode wait instead of UserMode to prevent 
        // the stack from getting paged out.
        //
        status = KeWaitForSingleObject(&pnpEvent, Executive, KernelMode,
                                        FALSE, NULL);
        ASSERT(NT_SUCCESS(status));         
        status = ioStatus.Status;

    }

GetDeviceCapabilitiesExit:
    //
    // Done with reference
    //
    ObDereferenceObject( targetObject );

    //
    // Done
    //
    return status;

}


BOOLEAN
PciDrvReadRegistryValue(
    IN PFDO_DATA   FdoData,
    IN PWCHAR     Name,
    OUT PLONG    Value
    )
/*++

Routine Description:

    Can be used to read any REG_DWORD registry value stored
    under Device Parameter.
    
Arguments:

    FdoData - pointer to the device extension
    Name - Name of the registry value
    Value - 


Return Value:

   TRUE if successful
   FALSE if not present/error in reading registry

--*/
{
    HANDLE      hKey = NULL;
    NTSTATUS    status;
    BOOLEAN     retValue = FALSE;
    ULONG       length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;
    UNICODE_STRING  valueName;


    PAGED_CODE();

    DebugPrint(TRACE, DBG_INIT, "-->PciDrvReadRegistryValue \n");

    *Value = 0;

    status = IoOpenDeviceRegistryKey (FdoData->UnderlyingPDO,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &hKey);

    if (NT_SUCCESS (status)) {

        RtlInitUnicodeString (&valueName, Name);

        length = sizeof (KEY_VALUE_FULL_INFORMATION)
                               + valueName.MaximumLength
                               + sizeof(ULONG);

        fullInfo = ExAllocatePoolWithTag (PagedPool, length, PCIDRV_POOL_TAG);

        if (fullInfo) {
            status = ZwQueryValueKey (hKey,
                                      &valueName,
                                      KeyValueFullInformation,
                                      fullInfo,
                                      length,
                                      &length);

            if (NT_SUCCESS (status)) {
                ASSERT (sizeof(ULONG) == fullInfo->DataLength);
                RtlCopyMemory (Value,
                               ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                               fullInfo->DataLength);
                retValue = TRUE;
            }
            
            ExFreePool (fullInfo);
        }
        
        ZwClose (hKey);
    }

    DebugPrint(TRACE, DBG_INIT, "<--PciDrvReadRegistryValue %ws %d \n", 
                                    Name, *Value);

    return retValue;
}

BOOLEAN
PciDrvWriteRegistryValue(
    IN PFDO_DATA  FdoData,
    IN PWCHAR     Name,
    IN ULONG      Value
    )
/*++

Routine Description:

    Can be used to write any REG_DWORD registry value stored
    under Device Parameter.

Arguments:


Return Value:

   TRUE - if write is successful
   FALSE - otherwise

--*/
{
    HANDLE          hKey = NULL;
    NTSTATUS        status;
    UNICODE_STRING  valName;
    BOOLEAN         retValue = FALSE;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_INIT, "Entered PciDrvWriteRegistryValue\n");

    //
    // write the value out to the registry
    //
    status = IoOpenDeviceRegistryKey (FdoData->UnderlyingPDO,
                                             PLUGPLAY_REGKEY_DEVICE,
                                             STANDARD_RIGHTS_ALL,
                                             &hKey);
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString (&valName, Name);

        status = ZwSetValueKey (hKey,
                       &valName,
                       0,
                       REG_DWORD,
                       &Value,
                       sizeof(Value));
        if(NT_SUCCESS(status)) {
            retValue = TRUE;
        }
        
        ZwClose (hKey);        
    } 
    
    return retValue;

}

LONG
PciDrvIoIncrement    (
    IN  OUT PFDO_DATA   FdoData
    )

/*++

Routine Description:

    This routine increments the number of requests the device receives

Arguments:

    FdoData - pointer to the device extension.

Return Value:

    The value of OutstandingIO field in the device extension.
--*/

{
    LONG            result;
    
    ASSERT(FdoData->OutstandingIO >= 1);

    result = InterlockedIncrement(&FdoData->OutstandingIO);

    DebugPrint(LOUD, DBG_LOCKS, "PciDrvIoIncrement %d\n", result);

    return result;
}

LONG
PciDrvIoDecrement    (
    IN  OUT PFDO_DATA  FdoData
    )
/*++

Routine Description:

    This routine decrements outstanding I/O count as it completes a 
    request. It also performs two special actions depending on the 
    resultant outstanding count. If the count equals 1 that indicates 
    the device is stopping. If the count equals 0 that indicates the 
    device is being removed.

Arguments:

    DeviceObject - pointer to the device object.

Return Value:

    The value of OutstandingIO field in the device extension.
--*/
{
    LONG            result;

    ASSERT(FdoData->OutstandingIO >= 1);
    
    result = InterlockedDecrement(&FdoData->OutstandingIO);

    DebugPrint(LOUD, DBG_LOCKS, "PciDrvIoDecrement %d\n", result);

    if (result == 1) {
        //
        // The count is 2-biased, so it can be 1 only if an 
        // extra decrement is done when the device needs to be stopped
        // for resource rebalancing or powered down.
        // Note that when this happens, any new requests we want to be 
        // processed are already held in the queue instead of being
        // passed away, so that we can't "miss" a request that
        // will appear between the decrement and the moment when
        // the value is actually used.
        //
        KeSetEvent (&FdoData->StopEvent, IO_NO_INCREMENT, FALSE);

    }

    if (result == 0) {
        //
        // The count is 2-biased, so it can be zero only if two 
        // extra decrement is done when a remove Irp is received
        //
        
        ASSERT(Deleted == FdoData->DevicePnPState);
        //
        // Set the remove event, so the device object can be deleted. 
        //
        KeSetEvent (&FdoData->RemoveEvent, IO_NO_INCREMENT, FALSE);
    }
    return result;
}

VOID
PciDrvReleaseAndWait(
    IN  OUT PFDO_DATA   FdoData,
    IN  ULONG           OnHoldCount, 
    IN  WAIT_REASON     Reason
    )
/*++

Routine Description:

    This routine waits for all outstanding requests, except the count specified
    in the OnHoldCount, to finish.

Arguments:

    FdoData - pointer to the device extension.
    OnHoldCount - This indicates the number of requests currently held
                    by the caller.
    Reason - Reason for all outstanding I/O to finish.                   

Return Value:

    VOID

--*/
{
    NTSTATUS status;
    ULONG    chargeRemining;
    
    ASSERT(OnHoldCount > 0);

    if(Reason == STOP){
        //
        // If the wait reason is STOP, we will do one extra decrement so
        // that the OustandingIo count can come down to 1 when all the
        // other requests in progress are finished. At that point, the Stop 
        // event will be signalled to indicate there are no I/Os currently active.
        //
        chargeRemining =  OnHoldCount + 1;
        while(chargeRemining--) {
            PciDrvIoDecrement(FdoData);
        }        
        
        DebugPrint(INFO, DBG_PNP, 
            "Waiting for pending requests to complete (Stop)...\n");
        
        status = KeWaitForSingleObject(
                               &FdoData->StopEvent,
                               Executive, // Waiting for reason of a driver
                               KernelMode, // Waiting in kernel mode
                               FALSE, // No alert
                               NULL); // No timeout
        ASSERT(NT_SUCCESS(status)); 

        ASSERT(FdoData->OutstandingIO == 1);
        //
        // Since STOP is a transient state, we should increment the OutstandingIo
        // count to account for requests currently held by the caller and 
        // also rebase the count to 2 by doing an extra increment.
        //
        chargeRemining =  OnHoldCount + 1;
        while(chargeRemining--) {
            PciDrvIoIncrement(FdoData);
        }
                
    } else if(Reason == REMOVE) {
        //
        // If the wait reason is REMOVE, we will do two extra decrements so
        // that the OustandingIo count can come down to 0 when all the
        // other requests in progress are finished. At that point, the Remove 
        // event will be signalled to indicate there are no I/Os currently active.
        // Remove is a destructive state. After this, no I/O requests will be
        // processed on this device and the deviceobject will be deleted.
        //
        chargeRemining =  OnHoldCount + 2;
        while(chargeRemining--) {
            PciDrvIoDecrement(FdoData);
        }

        DebugPrint(INFO, DBG_PNP, 
            "Waiting for pending requests to complete (Remove)...\n");

        status = KeWaitForSingleObject (
                &FdoData->RemoveEvent,
                Executive,
                KernelMode,
                FALSE,
                NULL);
        ASSERT(NT_SUCCESS(status));   
        ASSERT(FdoData->OutstandingIO == 0);

    } else {
        ASSERTMSG("Unknown reason", FALSE);
    }
    return;
}
    
ULONG
PciDrvGetOutStandingIoCount(
    IN PFDO_DATA FdoData
    )
{
    return FdoData->OutstandingIO;
    
}

#if !defined(EVENT_TRACING)

VOID
DebugPrint    (
    IN ULONG   DebugPrintLevel,
    IN ULONG   DebugPrintFlag,
    IN PCCHAR  DebugMessage,
    ...
    )

/*++

Routine Description:

    Debug print for the sample driver.

Arguments:

    DebugPrintLevel - print level between 0 and 3, with 3 the most verbose

Return Value:

    None.

 --*/
 {
#if DBG
#define     TEMP_BUFFER_SIZE        1024
    va_list    list;
    UCHAR      debugMessageBuffer[TEMP_BUFFER_SIZE];
    NTSTATUS status;
    
    va_start(list, DebugMessage);
    
    if (DebugMessage) {

        //
        // Using new safe string functions instead of _vsnprintf. This function takes
        // care of NULL terminating if the message is longer than the buffer.
        //
        status = RtlStringCbVPrintfA(debugMessageBuffer, sizeof(debugMessageBuffer), 
                                    DebugMessage, list);
        if(!NT_SUCCESS(status)) {
            
            KdPrint ((_DRIVER_NAME_": RtlStringCbVPrintfA failed %x\n", status));
            return;
        }
        if (DebugPrintLevel <= INFO || (DebugPrintLevel <= DebugLevel && 
                ((DebugPrintFlag & DebugFlag) == DebugPrintFlag))) {

            KdPrint ((_DRIVER_NAME_":%s", debugMessageBuffer));
        }        
    }
    va_end(list);

    return;
#endif
}

#endif 

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


