/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Incomplete1.c

Abstract:

    Incomplete1.c is the first stage of the Toaster sample function driver.

    Features implemented in this stage of the function driver:

    -An AddDevice routine to support new instances of Toaster class hardware
     that connect to the computer. This routine is named ToasterAddDevice.

    -An Unload routine to support removal of the function driver from the system.
     This routine is named ToasterUnload.

    -Dispatch routines to support IRP_MJ_PNP, IRP_MN_POWER, and
     IRP_MJ_SYSTEM_CONTROL. These routines are named ToasterDispatchPnP,
     ToasterDispatchPower and ToasterSystemControl, respectively.

    -Routines to stop the function driver from processing a PnP IRP until
     after the underlying bus driver has completed it. These routines are named
     ToasterSendIrpSynchronously and ToasterDispatchPnpComplete.

    -A debugging routine to print information from the function driver. This
     routine is named ToasterDebugPrint.

    This code is for learning purposes only. For additional code that supports
    other features, for example, Power Management and Windows Management
    Instrumentation (WMI), see the Featured1 stage.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub - 10-Oct-2002

    Kevin Shirley - 22-Apr-2003 - Commented for tutorial and learning purposes

--*/
#include "toaster.h"

//
// Global debug error level. This variable controls the level of debug output,
// and ranges from 0 (least verbose) to 3 (most verbose). The debug level
// #define tokens, such as INFO, are defined in Toaster.h
//
ULONG DebugLevel = INFO;
#define _DRIVER_NAME_ "Incomplete1"

#ifdef ALLOC_PRAGMA
//
// #pragma alloc_text specifies the binary image sections to place compiled
// routines in. For example, DriverEntry is placed in the INIT section, and
// ToasterAddDevice is placed in the PAGE section.
//
// Routines placed in the INIT section are released from memory after DriverEntry
// returns. This technique reduces the memory footprint of a driver. Routines
// placed in the PAGE section can be paged out to disk by the system or the driver
// as necessary. Note that if a routine is placed in the PAGE section, it must
// only execute at IRQL < DISPATCH_LEVEL. If a routine that is placed in the PAGE
// section executes at IRQL >= DISPATCH_LEVEL, and contains or references pageable
// code, then the Dispatcher cannot be invoked by the system to page in the
// required code or data. This causes a system crash.
//
// The PAGED_CODE macro is called in the routines listed below. The macro halts
// the system in the checked build of Windows if IRQL >= DISPATCH_LEVEL, because
// then the Dispatcher cannot be invoked, causing a system crash.
//
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, ToasterAddDevice)
#pragma alloc_text (PAGE, ToasterDispatchPnp)
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

New Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as ToasterAddDevice and ToasterUnload.

Parameters Description:
    DriverObject
    DriverObject represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory. 

    RegistryPath
    RegistryPath represents the driver specific path in the Registry. The function
    driver can use the path to store driver related data between reboots. The path
    does not store hardware instance specific data.

Return Value Description:
    DriverEntry returns STATUS_SUCCESS. NTSTATUS values are defined in the DDK
    header file, Ntstatus.h.
    
    Drivers execute in the context of other threads in the system. They do not
    receive their own threads. This is different than with applications. After
    DriverEntry returns, the Toaster function driver executes in an asynchronous,
    event driven, manner. 

    The system schedules other threads to execute on behalf of the driver. Every
    thread has a user-mode stack and a kernel-mode stack. When the system schedules
    a thread to execute in a driver, that thread’s kernel stack is used.

    The I/O manager calls the function driver’s routines to process events and I/O
    operations as needed. Multiple threads can execute simultaneously throughout the
    driver, thus special attention to how the driver operates and processes events
    and I/O operations is required

--*/
{
    ToasterDebugPrint(TRACE, "Entered DriverEntry of "_DRIVER_NAME_" version "
                                       "built on " __DATE__" at "__TIME__ "\n");

    //
    // Connect the ToasterAddDevice routine that is implemented in this stage of
    // the function driver. The system calls ToasterAddDevice when a new instance
    // of Toaster class hardware is connected to the computer.
    //
    DriverObject->DriverExtension->AddDevice           = ToasterAddDevice;

    //
    // Connect the ToasterUnload routine that is implemented in this stage of the
    // function driver. The system calls ToasterUnload when no more instances of
    // Toaster class hardware are connected to the computer.
    //
    DriverObject->DriverUnload                         = ToasterUnload;

    //
    // Connect the dispatch routines which are implemented in this stage of the
    // function driver, including PnP, Power Management and WMI. The system calls
    // these dispatch routines to handle PnP, Power and WMI operations.
    //
    DriverObject->MajorFunction[IRP_MJ_PNP]            = ToasterDispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = ToasterDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = ToasterSystemControl;

    return STATUS_SUCCESS;
}

 
NTSTATUS
ToasterAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
/*++

New Routine Description:
    The system calls ToasterAddDevice to create and initialize a device object to
    represent a new instance of Toaster class hardware when the underlying bus
    driver detects that a new instance has been connected to the computer.
    ToasterAddDevice creates and initializes a device object to represent new
    instances of Toaster class hardware. The device object ToasterAddDevice
    creates is called the FDO, for functional device object because it is the
    device object that is created by the function driver.

    ToasterAddDevice must also initialize the device extension, which is an area
    of memory that is allocated in the FDO when it was created. The device
    extension is defined in Toaster.h as FDO_DATA. The device extension stores
    data in non-paged memory. This includes hardware state variables, variables
    for mechanisms implemented by the function driver, and pointers to the
    underlying physical device object (PDO) and the driver immediately below the
    function driver.

    After the FDO has been created and the device extension initialized, the FDO
    must be attached to the top of the device stack for the new hardware instance.

    ToasterAddDevice also registers an instance of a device interface. Applications
    and other drivers use this interface to interact with the hardware instance. 

Parameters Description:
    DriverObject
    DriverObject represents the instance of the function driver that is loaded
    into memory. The FDO that ToasterAddDevice creates is associated with the
    DriverObject parameter.

    PhysicalDeviceObject
    PhysicalDeviceObject represents a device object that was created by the
    underlying bus driver for the new hardware instance. PhysicalDeviceObject is
    called the PDO, for physical device object. The PDO forms the base of the
    device stack for the new hardware instance. When the function driver passes
    IRPs down the device stack, it is usually because they must also be processed
    by the underlying bus driver.

Return Value Description:
    ToasterAddDevice returns STATUS_SUCCESS if there are no errors. Otherwise it
    returns an error if:

    -It cannot create a device object

    -It cannot attach the device object to the device stack

    -It cannot register a new instance of the device interface.

    If ToasterAddDevice returns a failure, then the new hardware instance cannot
    be used, and a bubble caption informs the user there is a problem.

    After ToasterAddDevice returns to the caller, the new hardware instance is
    represented in the system. However, before applications can use it, the
    function driver must receive and process IRP_MN_START_DEVICE in
    ToasterDispatchPnP.

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

    //
    // Create the functional device object (FDO) for the new hardware instance. In
    // the past, applications have usually communicated with a hardware instance
    // using the FDO’s name. However, the FDO created in the call to IoCreateDevice is
    // not given a name because that creates a security risk. Instead applications
    // use the Toaster device interface. A new instance of the Toaster device
    // interface is registered after the FDO is created.
    //
    status = IoCreateDevice (DriverObject,
                             sizeof (FDO_DATA),
                             NULL,
                             FILE_DEVICE_UNKNOWN,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &deviceObject);

    //
    // Test the return value with the NT_SUCCESS macro instead of testing for a
    // specific value like STATUS_SUCCESS, because system calls can return multiple
    // values that indicate success other than STATUS_SUCCESS.
    //
    if(!NT_SUCCESS (status))
    {
        return status;
    }

    ToasterDebugPrint(INFO, "AddDevice FDO (0x%p)\n", deviceObject);

    //
    // Get the pointer to the FDO’s device extension. The device extension stores
    // data in non-paged memory. This includes hardware state variables, variables
    // for mechanisms implemented by the function driver, and pointers to the
    // underlying physical device object (PDO) and the driver immediately below the
    // function driver.
    //
    // The FDO’s device extension is defined as a PVOID. The PVOID must be recast to
    // a pointer to the data type of the device extension, PFDO_DATA.
    //
    fdoData = (PFDO_DATA) deviceObject->DeviceExtension;

    //
    // Tag the FDO’s device extension with TOASTER_FDO_INSTANCE_SIGNATURE.
    // TOASTER_FDO_INSTANCE_SIGNATURE is defined in Toaster.h as "odFT". "odFT" is
    // "TFdo" backwards (for "Toaster FDO"). Specify "TFdo" when using a debugger to
    // search memory for FDO device extensions allocated by the Toaster sample
    // function driver.
    //
    fdoData->Signature = TOASTER_FDO_INSTANCE_SIGNATURE;

    //
    // Initialize the variable that indicates the hardware state of the toaster
    // instance. The INITIALIZE_PNP_STATE macro is defined in Toaster.h. The macro
    // initializes the DevicePnPState member of the device extension to NotStarted.
    //
    // The function driver’s dispatch routines check the DevicePnPState member to
    // determine how to proceed to process IRPs dispatched to them.
    //
    INITIALIZE_PNP_STATE(fdoData);

    //
    // Save the pointer to the PDO in the device extension. In later stages of the
    // function driver the PDO, which represents the base of the device stack for the
    // hardware instance, is required when making a change that affects the entire
    // device stack. The PDO is also helpful when debugging.
    //
    fdoData->UnderlyingPDO = PhysicalDeviceObject;

    //
    // Save the pointer to the FDO in the device extension. Saving a pointer to the
    // FDO in the device extension allows just the device extension to be passed to
    // routines where the routines can get the FDO from the device extension. This
    // prevents requiring passing another parameter to routines.
    //
    fdoData->Self = deviceObject;

    //
    // Set the DO_POWER_PAGABLE bit to indicate to the power manager that it must
    // only send power IRPs to the function driver at IRQL = PASSIVE_LEVEL. In later
    // stages of the function driver the ToasterDispatchPower must only execute at
    // IRQL = PASSIVE_LEVEL. If this bit is not set, then the power manager can send
    // power IRPs at IRQL >= DISPATCH_LEVEL, which can cause a system crash because
    // the Dispatcher cannot be invoked when a driver executes at
    // IRQL >= DISPATCH_LEVEL.
    //
    deviceObject->Flags |= DO_POWER_PAGABLE;

    //
    // Set the DO_BUFFERED_IO bit to indicate to the I/O manager that it must copy
    // data that is passed between a calling application and the function driver. For
    // example, this bit specifies that the I/O manager must copy the data buffer
    // that is specified in user-mode ReadFile or WriteFile calls into memory that
    // can be directly accessed by the function driver. When the read or write
    // operation is completed, the I/O manager must then copy the modified data back
    // into the application’s original buffer.
    //
    deviceObject->Flags |= DO_BUFFERED_IO;

    //
    // Attach the FDO to the top of the device stack for the hardware instance. If
    // the call succeeds then the value returned is the device object that was
    // previously at the top of the device stack. This is the device object that
    // IRPs must be sent to when they are passed down the stack.
    //
    // NextLowerDriver and UnderlyingPDO are not necessarily the same. They can be
    // different when other device objects, such as those created by filter drivers,
    // are layered between the PDO and the FDO.
    //
    fdoData->NextLowerDriver = IoAttachDeviceToDeviceStack (deviceObject,
                                                       fdoData->UnderlyingPDO);
    if(NULL == fdoData->NextLowerDriver)
    {
        //
        // If the call failed, then the FDO is not attached to the device stack.
        // Because the call failed, the FDO must be deleted to prevent leaking
        // memory before returning to the caller.
        //
        IoDeleteDevice(deviceObject);

        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Register a new instance of the toaster device interface for the new hardware
    // instance. Applications use the device interface to interact with the new
    // hardware instance.
    //
    status = IoRegisterDeviceInterface (
                PhysicalDeviceObject,
                (LPGUID) &GUID_DEVINTERFACE_TOASTER,
                NULL,
                &fdoData->InterfaceName);

    //
    // Test the return value with the !NT_SUCCESS macro instead of testing for a
    // specific value like STATUS_UNSUCCESSFUL, because system calls can return
    // multiple values that indicate failure other than STATUS_UNSUCCESSFUL.
    //
    if (!NT_SUCCESS (status))
    {
        //
        // If the call failed, then the FDO must be detached from the device stack
        // and then deleted to prevent leaking memory before returning to the caller.
        //
        ToasterDebugPrint(ERROR,
            "AddDevice: IoRegisterDeviceInterface failed (%x)\n", status);
        IoDetachDevice(fdoData->NextLowerDriver);
        IoDeleteDevice (deviceObject);
        return status;
    }

    //
    // Clear the DO_DEVICE_INITIALIZING bit. The I/O manager verifies that this bit
    // is cleared after ToasterAddDevice returns. After this bit is cleared the I/O
    // manager can send IRPs to the function driver’s dispatch routines.
    //
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

 
VOID
ToasterUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

New Routine Description:
    ToasterUnload releases any memory or resources allocated in DriverEntry and is
    the last routine called by the system before the system unloads the function
    driver. The system calls ToasterUnload when no more instances of Toaster class
    hardware are connected to the computer.
    
    The Incomplete1 stage of the function driver does not allocate any memory or
    resources in DriverEntry, so there is nothing for ToasterUnload to release.
    
Parameters Description:
    DriverObject
    DriverObject represents the instance of the function driver that the system is
    ready to unload from memory.

Return Value Description:
    ToasterUnload does not return a value.

--*/
{
    PAGED_CODE();

    //
    // Test the assumption that DriverObject’s DeviceObject pointer is NULL. The
    // DeviceObject member of DriverObject is a single-linked list of all FDOs
    // created earlier by ToasterAddDevice. DriverObject->DeviceObject should equal
    // NULL when the system calls ToasterUnload. All FDOs should have been deleted
    // earlier when the function driver processed IRP_MN_REMOVE_DEVICE for each
    // hardware instance as the hardware was removed.
    //
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

New Routine Description:
    The system dispatches IRP_MJ_PNP IRPs to ToasterDispatchPnP. This stage of the
    function processes three PnP IRPs:
    -IRP_MN_START_DEVICE
    -IRP_MN_SURPRISE_REMOVAL
    -IRP_MN_REMOVE_DEVICE

    This stage of the function driver also process several other PnP IRPs in a
    limited capacity. These IRPs are completely processed in later stages of the
    function driver:
    -IRP_MN_QUERY_STOP_DEVICE
    -IRP_MN_CANCEL_STOP_DEVICE
    -IRP_MN_STOP_DEVICE
    -IRP_MN_QUERY_REMOVE_DEVICE
    -IRP_MN_CANCEL_REMOVE_DEVICE

    Each IRP updates the hardware instance’s PnP state in the device extension.
    For example, when ToasterDispatchPnP processes IRP_MN_REMOVE_DEVICE, the PnP
    state is updated to Deleted. The other dispatch routines in the function
    driver check if the PnP state equals Deleted before processing any IRPs
    dispatched to them. If the PnP state is set to Deleted then any IRPs
    dispatched to the other routines can immediately be failed.

    All PnP IRPs must be passed down the device stack so that lower drivers can
    also process them by calling IoCallDriver (or ToasterSendIrpSynchronously,
    which calls IoCallDriver). Some IRPs must be processed by the bus driver
    before they are processed by the function driver. IRPs that must be processed
    by the bus driver before the function driver processes call the function
    driver routine, ToasterSendIrpSynchronously, to block the thread that is
    processing them until the bus driver completes the IRP. For example,
    IRP_MN_START_DEVICE must be processed this way. Other IRPs must be processed
    by the function driver before they are passed down the device stack. For
    example, IRP_MN_SURPRISE_REMOVAL is processed this way.

    Before any IRP is passed down the stack, the function driver must set up the
    I/O stack location for the next lower driver. If the function driver does not
    modify any data for the IRP, it can skip the I/O stack location that contains
    the data. This allows the system to reuse I/O stack locations.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the PnP event associated with the hardware instance described
    by the DeviceObject parameter, such as to start or remove a hardware instance.

Return Value Description:
    ToasterDispatchPnP returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Other return values indicate if
    the IRP was successfully processed by the device stack, or if an error
    occurred.

--*/
{
    PFDO_DATA               fdoData;
    PIO_STACK_LOCATION      stack;
    NTSTATUS                status = STATUS_SUCCESS;
    PPNP_DEVICE_STATE       deviceState;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Get the parameters of the IRP from the function driver’s location in the IRP’s
    // I/O stack. The results of the function driver’s processing of the IRP, if any,
    // are then stored back in the same I/O stack location.
    //
    // The PnP IRP’s minor function code is stored in the IRP’s MinorFunction member.
    //
    stack = IoGetCurrentIrpStackLocation (Irp);

    ToasterDebugPrint(TRACE, "FDO %s \n",
                PnPMinorFunctionString(stack->MinorFunction));

    if (Deleted == fdoData->DevicePnPState)
    {
        //
        // Fail the incoming IRP if the hardware instance has been removed. A driver
        // can receive an IRP to process after it’s hardware is removed because
        // multiple threads might execute simultaneously. One thread might process
        // IRP_MN_REMOVE_DEVICE (and change the hardware state to Deleted) and then
        // be suspended before completing IRP_MN_REMOVE_DEVICE, while the system
        // dispatches another IRP to be processed by a different thread. Because the
        // first thread updated the hardware state in the device extension, the
        // second thread will fail it’s IRP.
        //
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

        //
        // Call IoCompleteRequest to complete the incoming IRP. When calling
        // IoCompleteRequest to complete an IRP, the caller can supply a priority
        // boost. The priority boost increment the runtime priority of the original
        // thread that requested the operation. The function driver passes
        // IO_NO_INCREMENT to leave the original threads priority alone. Changing
        // the original thread’s runtime priority by specifying a value other than
        // IO_NO_INCREMENT can adversely affect the system’s performance.
        //
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        //
        // Note that the return statement does not dereference the Irp pointer.
        // Instead, the return statement returns the same value that the
        // IoStatus.Status member is set to. Dereferencing the Irp pointer after
        // calling IoCompleteRequest can cause a system crash. For example, the
        // thread that completed the IRP might be suspended between the call to
        // IoCompleteRequest and the return statement. While the thread is suspended,
        // the I/O manager may release the memory associated with the IRP. When the
        // thread later resumes and executes the return statement and attempts to
        // dereference the Irp pointer, a system crash results because the memory is
        // no longer valid.
        //
        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Determine the PnP IRP’s minor function code which describes the PnP operation.
    //
    switch (stack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:
        //
        // The system sends IRP_MN_START_DEVICE to start the hardware instance.
        // The system sends IRP_MN_START_DEVICE after the system has called
        // ToasterAddDevice. The system also sends IRP_MN_START_DEVICE if the
        // PnP hardware was connected when the computer booted. The system also
        // sends IRP_MN_START_DEVICE after it sends a previous IRP_MN_STOP_DEVICE.
        //
        // The underlying bus driver must process IRP_MN_START_DEVICE before the
        // function driver can because the bus driver must assign the hardware
        // resources, such as the I/O ports, I/O memory ranges, and interrupts,
        // that the toaster instance can use before the function driver can use
        // them.
        //
        // The function driver calls ToasterSendIrpSynchronously to have the bus
        // driver process the IRP before the function driver processes the IRP.
        // ToasterSendIrpSynchronously passes the IRP down the device stack to be
        // processed by the bus driver, and does not return to the caller until the
        // bus driver completes the IRP.
        //
        // This process can be initiated using the Enum program with the "-p #"
        // syntax to simulate plugging a toaster instance into the computer.
        //
        status = ToasterSendIrpSynchronously(fdoData->NextLowerDriver, Irp);

        if (NT_SUCCESS (status))
        {
            //
            // Enable the device interface that was registered earlier in
            // ToasterAddDevice to allow applications to interact with the toaster
            // instance.
            //
            // If IoSetDeviceInterfaceState returns STATUS_OBJECT_NAME_EXISTS then
            // the interface is already enabled. The interface might already be
            // enabled if the system sent and earlier IRP_MN_STOP_DEVICE to
            // rebalance hardware resources.
            //
            status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, TRUE);

            if (NT_SUCCESS (status))
            {
                //
                // Update the variable that indicates the hardware state of the
                // toaster instance to Started. When the hardware state is set to
                // Started the function driver is ready to process I/O operations,
                // such as read, write, and device control in its DispatchRead,
                // DispatchWrite, and DispatchDeviceControl routines. These routines
                // are implemented in later stages of the function driver.
                //
                // The SET_NEW_PNP_STATE macro first saves the current hardware
                // state before setting it to a new state. This macro is closely
                // related to the RESTORE_PREVIOUS_PNP_STATE macro. Both macros
                // are defined in Toaster.h. These macros provide a mechanism to
                // set and restore the hardware state of a toaster instance. For
                // example, some PnP IRPs, such as IRP_MN_QUERY_STOP_DEVICE, set
                // the hardware state to a new value. Other PnP IRPs, such as
                // IRP_MN_CANCEL_STOP_DEVICE, restore the hardware state to its
                // previous value.
                //
                SET_NEW_PNP_STATE(fdoData, Started);
            }
            else
            {
                ToasterDebugPrint(ERROR, "IoSetDeviceInterfaceState failed: 0x%x\n",
                                        status);
            }
        }

        //
        // Set the status of the IRP to the value returned by the bus driver. The
        // value returned by the bus driver is also the value returned to the caller.
        //
        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        return status;

    case IRP_MN_QUERY_STOP_DEVICE:
        //
        // The system sends IRP_MN_QUERY_STOP_DEVICE to query the function driver if
        // the hardware instance can be stopped safely, without losing any data. When
        // the function driver receives this IRP it must also prepare to stop the
        // hardware instance. If the function driver is unable to stop the hardware
        // instance, then it should fail this IRP with STATUS_UNSUCCESSFUL and then
        // complete the IRP in the IRP_MN_QUERY_STOP_DEVICE case without passing the
        // IRP down the device stack.
        //
        // The function driver must process this IRP before it passes the IRP down
        // the device stack to be processed by the underlying bus driver.
        //
        // If the system does not later send IRP_MN_STOP_DEVICE to stop the hardware
        // instance because another driver in the device stack failed
        // IRP_MN_QUERY_STOP_DEVICE, then the system sends IRP_MN_CANCEL_STOP_DEVICE
        // to inform the function driver that it can resume processing IRPs on the
        // hardware instance.        
        //

        //
        // Update the variable that indicates the hardware state of the toaster
        // instance to StopPending. When the hardware state is set to StopPending,
        // any new IRPs that the system dispatches to the function driver are added
        // to the driver-managed IRP queue for later processing. The driver-managed
        // IRP queue is implemented in later stages of the function driver.
        //
        // Otherwise, if the system subsequently sends IRP_MN_CANCEL_STOP_DEVICE then
        // any IRPs the system dispatched between IRP_MN_QUERY_STOP_DEVICE and
        // IRP_MN_CANCEL_STOP_DEVICE would be lost.
        //
        SET_NEW_PNP_STATE(fdoData, StopPending);

        //
        // Set the status of the IRP to STATUS_SUCCESS before passing it down the
        // device stack to indicate that the function driver successfully processed
        // the IRP.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;

        //
        // Break out of the switch case statement. The IRP is passed down the device
        // stack before it is completed at the end of ToasterDispatchPnP.
        //
        break;

   case IRP_MN_CANCEL_STOP_DEVICE:
        //
        // The system sends IRP_MN_CANCEL_STOP_DEVICE to inform the function driver
        // that it can resume processing IRPs on the hardware instance. The system
        // should have sent a previous IRP_MN_QUERY_STOP_DEVICE, which the function
        // driver completed, before the system sends IRP_MN_CANCEL_STOP_DEVICE.
        //

        //
        // Restore the variable that indicates the hardware state of the toaster
        // instance to its previous saved state. The previous state was saved when
        // the function driver processed IRP_MN_QUERY_STOP_DEVICE. The
        // SET_NEW_PNP_STATE and RESTORE_PREVIOUS_PNP_STATE macros are closely
        // related. Both macros are defined in Toaster.h. These macros provide a
        // mechanism to set and restore the hardware state of the toaster instance.
        //
        // For example, some PnP IRPs, such as IRP_MN_QUERY_STOP_DEVICE, set the
        // hardware state to a new value. Other PnP IRPs, such as
        // IRP_MN_CANCEL_STOP_DEVICE, restore the hardware state to its previous
        // value.
        //
        RESTORE_PREVIOUS_PNP_STATE(fdoData);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_STOP_DEVICE:
        //
        // The system sends IRP_MN_STOP_DEVICE to stop a toaster instance’s hardware.
        // The system should have sent a previous IRP_MN_QUERY_STOP_DEVICE, which the
        // function driver completed, before the system sends IRP_MN_STOP_DEVICE.
        //
        // The function driver must process this IRP before it passes the IRP down
        // the device stack to be processed by the underlying bus driver.
        //
        // After IRP_MN_STOP_DEVICE is passed down the device stack, the function
        // driver must not pass any more IRPs down the device stack until the
        // function driver receives and completes another IRP_MN_START_DEVICE.
        //

        //
        // Update the variable that indicates the hardware state of the toaster
        // instance to Stopped. When the hardware state is set to Stopped, any new
        // IRPs that the system dispatches to the function driver are added to the
        // driver-managed IRP queue for later processing. The driver-managed IRP
        // queue is implemented in later stages of the function driver.
        //
        // Otherwise, if the system subsequently sends IRP_MN_START_DEVICE then
        // any IRPs the system dispatched between IRP_MN_STOP_DEVICE and
        // IRP_MN_START_DEVICE would be lost.
        //
        SET_NEW_PNP_STATE(fdoData, Stopped);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        //
        // The system sends IRP_MN_QUERY_REMOVE_DEVICE to query the function driver if
        // the hardware instance can be removed safely, without losing any data. When
        // the function driver receives this IRP, it must also prepare to remove the
        // hardware instance. If the function driver is unable to remove the hardware
        // instance, then it should fail this IRP with STATUS_UNSUCCESSFUL.
        //
        // The function driver must process this IRP before it passes the IRP down
        // the device stack to be processed by the underlying bus driver.
        //
        // Failing this IRP does not prevent the system from sending a subsequent
        // IRP_MN_REMOVE_DEVICE, it just informs the system that the function driver
        // is unable to remove without losing data.
        //
        // If the system does not later send IRP_MN_REMOVE_DEVICE to remove the
        // hardware instance, then the system sends IRP_MN_CANCEL_REMOVE_DEVICE to
        // inform the function driver that it can resume processing IRPs on the
        // hardware instance.
        //

        //
        // Update the variable that indicates the hardware state of the toaster
        // instance to RemovePending. When the hardware state is set to RemovePending,
        // any new IRPs that the system dispatches to the function driver are added
        // to the driver-managed IRP queue for later processing. The driver-managed
        // IRP queue is implemented in later stages of the function driver.
        //
        // Otherwise, if the system later sends IRP_MN_CANCEL_REMOVE_DEVICE then any
        // IRPs the system dispatched between IRP_MN_QUERY_REMOVE_DEVICE and
        // IRP_MN_CANCEL_REMOVE_DEVICE would be lost.
        //
        SET_NEW_PNP_STATE(fdoData, RemovePending);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        //
        // The system sends IRP_MN_CANCEL_REMOVE_DEVICE to inform the function driver
        // that is can resume processing IRPs on the hardware instance. The system
        // should have sent a previous IRP_MN_QUERY_STOP_DEVICE, which the function
        // driver completed, before the system sends IRP_MN_CANCEL_REMOVE_DEVICE.
        //

        //
        // Restore the variable that indicates the hardware state of the toaster
        // instance to its previous saved state. The previous state was saved when
        // the function driver processed IRP_MN_QUERY_REMOVE_DEVICE. The
        // SET_NEW_PNP_STATE and RESTORE_PREVIOUS_PNP_STATE macros are closely
        // related. Both macros are defined in Toaster.h. These macros provide a
        // mechanism to set and restore the hardware state of the toaster instance.
        //
        // For example, some PnP IRPs, such as IRP_MN_QUERY_STOP_DEVICE, set the
        // hardware state to a new value. Other PnP IRPs, such as
        // IRP_MN_CANCEL_STOP_DEVICE, restore the hardware state to its previous
        // value.
        //
        RESTORE_PREVIOUS_PNP_STATE(fdoData);

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

   case IRP_MN_SURPRISE_REMOVAL:
        //
        // The system sends IRP_MN_SURPRISE_REMOVAL when the hardware instance is no
        // connected to the computer. A surprise removal usually occurs when the user
        // disconnects the hardware instance from the computer without first
        // notifying the system.
        //
        // The function driver must process this IRP before it passes the IRP down
        // the device stack to be processed by the underlying bus driver.
        //
        // This process can be initiated using the Enum program with the "-u #"
        // syntax to simulate unplugging a toaster instance from the computer.
        //

        //
        // Update the variable that indicates the hardware state of the toaster
        // instance to SurpriseRemovePending. When the hardware state is set to
        // SurpriseRemovePending, any new IRPs that the system dispatches to the
        // function driver are failed because the hardware instance is no longer
        // present.
        //
        SET_NEW_PNP_STATE(fdoData, SurpriseRemovePending);

        //
        // Disable the device interface that was enabled earlier when
        // ToasterDispatchPnP processed IRP_MN_START_DEVICE. This operation stops
        // applications from interacting with the (now removed) hardware instance.
        //
        status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

        if (!NT_SUCCESS (status))
        {
            ToasterDebugPrint(ERROR, 
                "IoSetDeviceInterfaceState failed: 0x%x\n", status);
        }

        //
        // Set the status of the IRP to STATUS_SUCCESS before passing it down the
        // device stack to indicate that the function driver successfully processed
        // IRP_MN_SURPRISE_REMOVAL.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;

        //
        // Set up the I/O stack location for the next lower driver (the target device
        // object for the IoCallDriver call). Call IoSkipCurrentIrpStackLocation
        // because the function driver does not change any of the IRP’s values in the
        // current I/O stack location. This allows the system to reuse I/O stack
        // locations.
        //
        IoSkipCurrentIrpStackLocation (Irp);

        //
        // Pass IRP_MN_SURPRISE_REMOVAL down the device stack to be processed by the
        // bus driver.
        //
        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        //
        // Return the status returned by IoCallDriver to the caller.
        //
        return status;

   case IRP_MN_REMOVE_DEVICE:
        //
        // The system sends IRP_MN_REMOVE_DEVICE to delete the FDO that represents
        // the hardware instance. The system sends IRP_MN_REMOVE_DEVICE after it
        // sends IRP_MN_QUERY_REMOVE_DEVICE, or after the it sends
        // IRP_MN_SURPRISE_REMOVAL if there are no open handles to this device.
        //
        // The function driver must process this IRP before it passes the IRP down
        // the device stack to be processed by the underlying bus driver.
        //
        // This process can be initiated using the Enum program with the "-u #"
        // syntax to simulate unplugging a toaster instance from the computer.
        //
        // This process can also be initiated using the Safely Remove Hardware icon
        // in the tray, near the time.
        //

        //
        // Update the variable that indicates the hardware state of the toaster
        // instance to Deleted. When the hardware state is set to Deleted, any new
        // IRPs that the system dispatches to the function driver are failed because
        // the hardware instance is no longer present.
        //
        SET_NEW_PNP_STATE(fdoData, Deleted);

        //
        // Check the previous PnP state, instead of the present device state because
        // the earlier call to the SET_NEW_PNP_STATE macro changed the current device
        // state.
        //
        if(SurpriseRemovePending != fdoData->PreviousPnPState)
        {
            //
            // If the function driver did not process IRP_MN_SURPRISE_REMOVAL earlier
            // then it must disable the device interface that was enabled earlier
            // when it processed IRP_MN_START_DEVICE. This operation stops
            // applications from interacting with the (now removed) hardware instance.
            //
            status = IoSetDeviceInterfaceState(&fdoData->InterfaceName, FALSE);

            if (!NT_SUCCESS (status))
            {
                ToasterDebugPrint(ERROR,
                        "IoSetDeviceInterfaceState failed: 0x%x\n", status);
            }
        }

        //
        // Set the status of the IRP to STATUS_SUCCESS before passing it down the
        // device stack to indicate that the function driver successfully processed
        // IRP_MN_REMOVE_DEVICE.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;

        //
        // Set up the I/O stack location for the next lower driver (the target device
        // object for the IoCallDriver call). Call IoSkipCurrentIrpStackLocation
        // because the function driver does not change any of the IRP’s values in the
        // current I/O stack location. This eliminates an unnecessary copy operation
        // and allows the lower driver to reuse the same I/O stack location.
        //
        IoSkipCurrentIrpStackLocation (Irp);

        //
        // Pass IRP_MN_REMOVE_DEVICE down the device stack to be processed by the
        // bus driver.
        //
        status = IoCallDriver (fdoData->NextLowerDriver, Irp);

        //
        // Detach the hardware instance’s FDO from the device stack.
        //
        IoDetachDevice (fdoData->NextLowerDriver);

        //
        // Release the memory that was allocated for the device interface’s name
        // earlier when ToasterAddDevice called IoRegisterDeviceInterface.
        //
        RtlFreeUnicodeString(&fdoData->InterfaceName);

        //
        // Delete the hardware instance’s FDO. After IoDeleteDevice returns, the FDO
        // becomes invalid and must not be used.
        //
        IoDeleteDevice (fdoData->Self);

        //
        // Return the status returned by IoCallDriver to the caller.
        //
        return status;

    default:
        //
        // Break out of the switch case statement. All PnP IRPs that the function
        // driver does not process must be passed down the device stack so that the
        // underlying bus driver can process them.
        //
        break;
    }

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the IoCallDriver call). Call IoSkipCurrentIrpStackLocation
    // because the function driver does not change any of the IRP’s values in the
    // current I/O stack location. This allows the system to reuse I/O stack
    // locations.
    //
    IoSkipCurrentIrpStackLocation (Irp);

    //
    // Pass the IRP down the device stack to be processed by the bus driver.
    //
    status = IoCallDriver (fdoData->NextLowerDriver, Irp);

    return status;
}

 
NTSTATUS
ToasterSendIrpSynchronously (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    ToasterSendIrpSynchronously passes a PnP IRP down the device stack to be
    processed by the underlying bus driver and does not return to the caller until
    the bus driver completes the IRP. ToasterSendIrpSynchronously uses a kernel
    event to synchronize the processing of the IRP between the function driver and
    the bus driver. The function driver can resume processing the PnP IRP after
    ToasterSendIrpSynchronously returns to the caller.

    ToasterSendIrpSynchronously passes the incoming IRP down the device stack to
    be processed by the bus driver. If the bus driver does not immediately
    complete the IRP then the bus driver marks the IRP as pending and the thread
    executing in ToasterSendIrpSynchronously must wait until the bus driver
    completes the IRP and the system calls the function driver’s I/O completion
    routine, ToasterDispatchPnpComplete.

    If the bus driver marks the PnP IRP as pending, then
    ToasterSendIrpSynchronously calls KeWaitForSingleObject to suspend the
    execution of the thread in ToasterSendIrpSynchronously until the kernel event
    that the KeWaitForSingleObject call waits on is signaled. The kernel event is
    signaled after the bus driver complete the IRP and the system calls the
    function driver’s I/O completion routine, ToasterDispatchPnpComplete.

    This mechanism prevents the function driver from processing the PnP IRP until
    the bus driver has completed it. 

    For example, the bus driver must process IRP_MN_START_DEVICE before the
    function driver. If the bus driver marks the IRP as pending, then the function
    driver cannot proceed until the bus driver completes it. In this case, the
    thread that processes IRP_MN_START_DEVICE in ToasterDispatchPnP (which calls
    ToasterSendIrpSynchronously) must be suspended until the bus driver completes
    the IRP.

    When the bus driver completes IRP_MN_START_DEVICE, the I/O manager passes the
    IRP back up the device stack, calling any I/O completion routines previously
    set to be called. ToasterSendIrpSynchronously calls IoSetCompletionRoutine to
    set the function driver’s I/O completion routine, ToasterDispatchPnpComplete,
    to be called before IRP_MN_START_DEVICE is passed down the device stack.

Parameters Description:
    DeviceObject
    DeviceObject represents the target device object to pass the PnP IRP to.

    Irp
    Irp represents the PnP operation that must be processed by the bus driver
    before the function driver can process it.

Return Value Description:
    ToasterSendIrpSynchronously returns the status value returned by the next
    Lower driver.

--*/
{
    KEVENT   event;
    NTSTATUS status;

    PAGED_CODE();

    //
    // Initialize a kernel event to an unsignaled state. The event is signaled in
    // ToasterDispatchPnpComplete when the bus driver completes the PnP IRP.
    //
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the IoCallDriver call). Call IoCopyCurrentIrpStackLocationToNext
    // to copy the parameters from the I/O stack location for the function driver
    // to the I/O stack location for the next lower driver, so that the next lower
    // driver uses the same parameters as the function driver.
    //
    IoCopyCurrentIrpStackLocationToNext(Irp);

    //
    // Set the system to call the function driver’s I/O completion routine,
    // ToasterDispatchPnpComplete, after the bus driver has completed the PnP IRP.
    //
    // Pass the kernel event that was initialized earlier as the context parameter
    // for when the system calls ToasterDispatchPnpComplete.
    //
    IoSetCompletionRoutine(Irp,
                           ToasterDispatchPnpComplete,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

    //
    // Pass the incoming IRP down the device stack to be processed by the bus driver.
    // The value returned from IoCallDriver indicates whether the bus driver marked
    // the IRP as pending, or completed the IRP.
    //
    status = IoCallDriver(DeviceObject, Irp);

    if (STATUS_PENDING == status)
    {
        //
        // If the bus driver marked the IRP as pending, then call
        // KeWaitForSingleObject to suspend the thread executing in
        // ToasterSendIrpSynchronously until the kernel event is signaled.
        // KeWaitForSingleObject does not return until the kernel event is signaled.
        // The event is signaled after the bus driver completes the PnP IRP and the
        // system calls the function driver’s I/O completion routine,
        // ToasterDispatchPnpComplete.
        //
        // Pass KernelMode as the wait mode in the call to KeWaitForSingleObject to
        // prevent the memory manager from paging out the device stack.
        //
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
ToasterDispatchPnpComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
/*++

New Routine Description:
    ToasterDispatchPnpComplete signals the kernel event which unblocks the call to
    KeWaitForSingleObject in ToasterSendIrpSynchronously, allowing the thread to
    resume and continue to process the PnP IRP. ToasterSendIrpSynchronously called
    IoSetCompletionRoutine to set the system to call ToasterDispatchPnpComplete
    after the bus driver completes the PnP IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the PnP operation that has been completed by the underlying bus
    driver. The IRP now contains the data required by the function driver to
    proceed. For example, if the bus driver completed IRP_MN_START_DEVICE, then
    the IRP now contains the hardware resources the function driver can use, such
    as DMA channels, I/O ports, I/O memory ranges, and/or interrupt(s).

    Context
    Context represents the kernel event that the call to KeWaitForSingleObject in
    ToasterSendIrpSynchronously is waiting on. ToasterSendIrpSynchronously set the
    system to pass the kernel event as the context parameter in the call to
    IoSetCompletionRoutine.

Return Value Description:
    ToasterDispatchPnpComplete returns STATUS_MORE_PROCESSING_REQUIRED.
    STATUS_MORE_PROCESSING_REQUIRED indicates to the I/O manager that the function
    driver must process the IRP further before the I/O manager resumes calling any
    other I/O completion routines.

--*/
{
    //
    // The Context parameter is a PVOID. The PVOID must be recast to the data type of
    // the kernel event, PKEVENT.
    //
    PKEVENT             event = (PKEVENT)Context;

    //
    // The UNREFERENCED_PARAMETER macro suppresses the compiler warning about
    // unreferenced parameters.
    //
    UNREFERENCED_PARAMETER (DeviceObject);

    if (TRUE == Irp->PendingReturned)
    {
        //
        // If the bus driver marked the IRP as pending, then signal the kernel event.
        // When the kernel event is signaled, the thread that was suspended earlier
        // when ToasterSendIrpSynchronously called KeWaitForSingleObject can resume
        // and the PnP IRP that ToasterSendIrpSynchronously was processing can be
        // completed in ToasterDispatchPnP.
        //
        KeSetEvent (event, IO_NO_INCREMENT, FALSE);
    }

    //
    // Do not complete the IRP here in the I/O completion routine. Instead, the IRP
    // is completed in ToasterDispatchPnP.
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 
NTSTATUS
ToasterDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    The system dispatches IRP_MJ_POWER IRPs to ToasterDispatchPower. This stage of
    the function driver does not process any specific power IRPs. All power IRPs
    that the system dispatches to ToasterDispatchPower are passed down the device
    stack to be processed by the underlying bus driver. The Featured1 stage of the
    function driver demonstrates how to process specific power IRPs.

    The system calls ToasterDispatchPower when the computer shuts down,
    hibernates, suspends, or resumes power while instances of toaster hardware
    remain connected to the computer.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the power operation associated with the hardware instance
    described by the DeviceObject parameter, such as powering up or powering down
    the hardware instance.

Return Value Description:
    ToasterDispatchPower returns the status of how the bus driver processed the
    power IRP.

--*/
{
    PFDO_DATA           fdoData;
    NTSTATUS            status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchPower\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    if (Deleted == fdoData->DevicePnPState)
    {
        //
        // Start the next power IRP. Power IRPs are processes differently than
        // non-power IRPs. A driver’s DispatchPower routine must call
        // PoStartNextPowerIrp to signal the power manager to send the next power
        // IRP. PoStartNextPowerIrp must still be called even if the driver’s
        // DispatchPower routine fails the incoming Power IRP.
        //
        // A driver must call PoStartNextPowerIrp while the IRP’s current I/O
        // stack location points to the current driver. That is,
        // PoStartNextPowerIrp must be called before IoSkipCurrentIrpStackLocation
        // or IoCopyIrpStackLocationToNext. In addition, a driver must call
        // PoStartNextPowerIrp before IoCompleteRequest or PoCallDriver, otherwise
        // a system deadlock could result.
        //
        PoStartNextPowerIrp (Irp);

        status = STATUS_NO_SUCH_DEVICE;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        return status;
    }

    //
    // Start the next power IRP. It would not be safe to call PoStartNextPowerIrp
    // before checking to see if the DevicePnPState equals Deleted (above) because
    // a driver should only call PoStartNextPowerIrp immediately before it completes
    // the current power IRP. Because additional logic appears in the Featured1 stage
    // for ToasterDispatchPower the safe method requires these separate calls to
    // PoStartNextPowerIrp.
    //

    PoStartNextPowerIrp(Irp);

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the IoCallDriver call). Call IoSkipCurrentIrpStackLocation
    // because the function driver does not change any of the IRP’s values in the
    // current I/O stack location. This allows the system to reuse I/O stack
    // locations.
    //
    IoSkipCurrentIrpStackLocation(Irp);

    //
    // Pass the power IRP down the device stack. Drivers must call PoCallDriver
    // instead of IoCallDriver to pass power IRPs down the device stack. PoCallDriver
    // ensures that power IRPs are properly synchronized throughout the system.
    //
    status = PoCallDriver(fdoData->NextLowerDriver, Irp);
    
    return status;
}

 
NTSTATUS
ToasterSystemControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

New Routine Description:
    The system dispatches WMI IRPs to ToasterSystemControl. This stage of the
    function driver does not process any specific WMI IRPs. All WMI IRPs that the
    system dispatches to ToasterSystemControl as passed down the device stack to
    be processed by the underlying bus driver. In later stages of the function
    driver, ToasterSystemControl demonstrates how to process specific WMI IRPs.

    The system calls ToasterSystemControl to return metrics about the operation
    and status of a toaster hardware instance.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is a FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the WMI operation associated with the hardware instance
    described by the DeviceObject parameter, such as querying the number of
    failures that the hardware instance has recorded.

Return Value Description:
    ToasterSystemControl returns a value that indicates if the IRP was
    successfully processed by the device stack, or an error, if one occurred.

--*/
{
    PFDO_DATA               fdoData;
    NTSTATUS                status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "FDO received WMI IRP\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    if (Deleted == fdoData->DevicePnPState)
    {
        status = STATUS_NO_SUCH_DEVICE;

        Irp->IoStatus.Status = status;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        return status;
    }

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the IoCallDriver call). Call IoSkipCurrentIrpStackLocation
    // because the function driver does not change any of the IRP’s values in the
    // current I/O stack location. This allows the system to reuse I/O stack
    // locations.
    //
    IoSkipCurrentIrpStackLocation (Irp);

    //
    // Pass the WMI (system control) IRP down the device stack to be processed by the
    // bus driver.
    //
    status = IoCallDriver (fdoData->NextLowerDriver, Irp);

    return status;
}

 
VOID
ToasterDebugPrint    (
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    ...
    )
/*++

New Routine Description:
    ToasterDebugPrint outputs information from the function driver to a debugger.

Parameters Description:
    DebugPrintLevel
    DebugPrintLevel represents the level of debug output, which ranges from 0
    (least verbose) to 3 (most verbose). If this parameter is <= to the global
    variable, DebugLevel, then the message is output to a debugger.

    DebugMessage
    DebugMessage is a formatted C-style string that can contain delimiters.
    Optional parameters for the string can be passed in the call to
    ToasterDebugPrint after DebugMessage. For example:
    ToasterDebugPrint( 0,
                       "PDO=0x%p, FDO=0x%p\n",
                       NextLowerDriver,
                       PhysicalDeviceObject);

Return Value Description:
    ToasterDebugPrint does not return a value.

--*/
{
#define     TEMP_BUFFER_SIZE        1024
    va_list    list;
    UCHAR      debugMessageBuffer[TEMP_BUFFER_SIZE];
    NTSTATUS status;
    
    va_start(list, DebugMessage);
    
    if (DebugMessage)
    {
        //
        // Use the safe string function, RtlStringCbVPrintfA, instead of _vsnprintf.
        // RtlStringCbVPrintfA NULL terminates the output buffer even if the message
        // is longer than the buffer. This prevents malicious code from compromising
        // the security of the system.
        //
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

New Routine Description:
    PnPMinorFunctionString converts the minor function code of a PnP IRP to a
    text string that is more helpful when tracing the execution of PnP IRPs.

Parameters Description:
    MinorFunction
    MinorFunction specifies the minor function code of a PnP IRP.

Return Value Description:
    PnPMinorFunctionString returns a pointer to a string that represents the
    text description of the incoming minor function code.

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

