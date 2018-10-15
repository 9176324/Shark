/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Wake.c

Abstract:

    Wake.c implements the code required for the Toaster sample function driver to
    support wait/wake (IRP_MN_WAIT_WAKE) power IRPs.

    Features implemented in this stage of the function driver:

    -A dispatch routine to process IRP_MN_WAIT_WAKE power IRPs. This routine is
     called ToasterDispatchWaitWake.

    -Routines to arm a hardware instance to signal wake-up and disarm a hardware
     instance from signaling wake-up. These routines are called ToasterArmForWake
     and ToasterDisarmWake.

    -A Routine to determine from what device and system power state the hardware
     instance can signal wake-up. This routine is called
     ToasterAdjustCapabilities.

    -Routines to determine if the hardware instance can support IRP_MN_WAIT_WAKE
     power IRPs based on values in the Registry. These routines are called
     ToasterSetWaitWakeEnableState and ToasterGetWaitWakeEnableState.

    To arm a hardware instance to wake-up the system from a lower power state, the
    function driver calls ToasterArmForWake when it processes IRP_MN_START_DEVICE
    or IRP_MN_CANCEL_REMOVE_DEVICE. The function driver also calls
    ToasterArmForWake when the system sends WMI IRPs because the user checked the
    "Allow this device to bring the computer out of standby" checkbox in the
    hardware instance's Power Management tab in the device's Properties dialog in
    the Device Manager.

    If a key in the Registry indicates that the hardware instance supports
    wait/wake operations, then the ToasterArmForWake routine requests the system
    send a IRP_MN_WAIT_WAKE power IRP to the hardware instance's device stack.
    When the function driver requests the IRP_MN_WAIT_WAKE power IRP, it also
    specifies a power completion routine (different than an I/O completion
    routine) for the system to call after all drivers in the hardware instance's
    device stack has finished processing the IRP_MN_WAIT_WAKE_IRP.

    The function driver's power dispatch routine, ToasterDispatchPower receives
    the IRP_MN_WAIT_WAKE power IRP and passes it to the ToasterDispatchWaitWake
    routine, which marks the IRP pending, sets an I/O completion routine, and
    passes the IRP down the device stack to the underlying bus driver.

    Later, when the system is in a low-power state, and the circuitry on the
    hardware instance detects a wake-up signal, the underlying bus driver
    completes the IRP_MN_WAIT_WAIT that was marked as pending earlier. The system
    then calls the function driver's I/O completion routine and finally the power
    completion routine specified by the function driver when it requested the
    IRP_MN_WAIT_WAKE IRP. The power completion routine re-arms the hardware
    instance to signal wake-up again.

    To disarm a hardware instance from signaling wake-up, the function driver
    calls ToasterDisarmWake when it processes IRP_MN_STOP_DEVICE,
    IRP_MN_QUERY_REMOVE_DEVICE, or IRP_MN_SURPRISE_REMOVAL. The function driver
    also calls ToasterDisarmWake when the system sends WMI IRPs because the user
    unchecked the "Allow this device to bring the computer out of standby"
    checkbox in the hardware instance's Power Management tab in the device's
    Properties dialog in the Device Manager.

    ToasterDisarmWake cancels an IRP_MN_WAIT_WAKE power IRP requested earlier by
    ToasterArmForWake.

Environment:

    Kernel mode only

Revision History:

    Eliyas Yakub - 02-Oct-2001 -
     Based on Adrian J. Oney's wait-wake boilerplate code 
     
    Kevin Shirley - 27-Jun-2003 - Commented for tutorial and learning purposes
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
#include "wake.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, ToasterDispatchWaitWake)
#pragma alloc_text (PAGE, ToasterArmForWake)
#pragma alloc_text (PAGE, ToasterDisarmWake)
#pragma alloc_text (PAGE, ToasterAdjustCapabilities)
#pragma alloc_text (PAGE, ToasterSetWaitWakeEnableState)
#pragma alloc_text (PAGE, ToasterGetWaitWakeEnableState)
#pragma alloc_text (PAGE, ToasterPassiveLevelReArmCallbackWorker)
#pragma alloc_text (PAGE, ToasterPassiveLevelClearWaitWakeEnableState)
#endif


 
NTSTATUS
ToasterDispatchWaitWake(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

New Routine Description:
    ToasterDispatchWaitWake processes IRP_MN_WAIT_WAKE requested earlier by the
    ToasterArmForWake routine when the function driver processed
    IRP_MN_START_DEVICE or IRP_MN_CANCEL_REMOVE_DEVICE. ToasterDispatchWaitWake
    sets an I/O completion routine to be called, so the function driver can
    resume processing the IRP_MN_WAKE_WAKE power IRP on its way back up the
    device stack, after the underlying bus driver has completed it.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the wait/wake operation associated with DeviceObject.

Return Value Description:
    ToasterDispatchWaitWake returns STATUS_CANCELLED if the hardware instance's
    wait/wake state has been cancelled. Otherwise ToasterDispatchWaitWake returns
    STATUS_PENDING because it marked the incoming wait/wake power IRP pending.

--*/
{
    PFDO_DATA       fdoData;
    WAKESTATE       oldWakeState;

    PAGED_CODE ();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchWaitWake\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Save the incoming IRP_MN_WAIT_WAKE power IRP in the device extension. When the
    // WakeIrp member does not equal NULL, then that indicates the function driver is
    // currently processing a IRP_MN_WAIT_WAKE power IRP.
    //
    // The ToasterWaitWakePoCompletionRoutine routine clears WakeIrp to NULL after
    // the function driver completes the IRP_MN_WAIT_WAKE power IRP.
    //
    fdoData->WakeIrp = Irp;

    //
    // If fdoData->WakeState equals WAKESTATE_WAITING, then
    // InterlockedCompareExchange sets fdoData->WakeState to WAKESTATE_ARMED.
    // WAKESTATE_ARMED is the next step after WAKESTATE_WAITING when processing
    // IRP_MN_WAIT_WAKE.
    //
    // Otherwise, InterlockedCompareExchange does not change fdoData->WakeState. For
    // example, InterlockedCompareExchange does not change fdoData->WakeState to
    // WAKESTATE_ARMED if the present state equals WAKESTATE_WAITING_CANCELLED. The
    // present state might equal WAKESTATE_WAITING_CANCELLED if the function driver
    // has processed IRP_MN_STOP_DEVICE, IRP_MN_QUERY_REMOVE_DEVICE, or
    // IRP_MN_SURPRISE_REMOVE and thus called ToasterDisarmWake between the time it
    // called ToasterArmForWake and the function driver received the IRP_MN_WAIT_WAKE
    // power IRP.
    //
    oldWakeState = InterlockedCompareExchange( (PULONG)&fdoData->WakeState,
                                            WAKESTATE_ARMED,
                                            WAKESTATE_WAITING );

    if (WAKESTATE_WAITING_CANCELLED == oldWakeState)
    {
        //
        // Fail the IRP_MN_WAIT_WAKE power IRP if InterlockedCompareExchange did not
        // change FdoData->WakeState to WAKESTATE_ARMED because the function driver
        // has called ToasterDisarmWake to cancel the IRP_MN_WAIT_WAKE power IRP.
        //
        fdoData->WakeState = WAKESTATE_COMPLETING;

        Irp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        ToasterIoDecrement(fdoData);

        return STATUS_CANCELLED;
    }

    //
    // Mark the incoming IRP_MN_WAIT_WAKE power IRP as pending. The function driver
    // continues to process the IRP_MN_WAIT_WAKE power IRP when the system calls the
    // ToasterWaitWakeIoCompletionRoutine I/O completion routine. The system calls
    // ToasterWaitWakeIoCompletionRoutine after the underlying bus driver completes
    // the IRP_MN_WAIT_WAKE power IRP.
    //
    IoMarkIrpPending( Irp );

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the PoCallDriver call). The function driver calls
    // IoCopyCurrentIrpStackLocationToNext to copy the parameters from the I/O stack
    // location for the function driver to the I/O stack location for the next lower
    // driver, so that the next lower driver uses the same parameters as the function
    // driver.
    //
    IoCopyCurrentIrpStackLocationToNext( Irp );

    //
    // Set the system to call ToasterWaitWakeIoCompletionRoutine after the underlying
    // bus driver completes the wait/wake power IRP.
    //
    IoSetCompletionRoutine( Irp, ToasterWaitWakeIoCompletionRoutine, NULL,
                                                            TRUE, TRUE, TRUE );

    //
    // Pass the IRP_MN_WAIT_WAKE power IRP down the device stack. Drivers must use
    // PoCallDriver instead of IoCallDriver to pass power IRPs down the device stack.
    // PoCallDriver ensures that power IRPs are properly synchronized throughout the
    // system.
    //
    PoCallDriver( fdoData->NextLowerDriver, Irp);

    //
    // Decrement the count of how many IRPs remain uncompleted. This call to
    // ToasterIoDecrement balances the earlier call to ToasterIoIncrement in
    // ToasterDispatchPower. An equal number of calls to ToasterIoincrement and
    // ToasterIoDecrement is essential for the StopEvent and RemoveEvent kernel
    // events to be signaled correctly.
    //
    ToasterIoDecrement(fdoData);

    return STATUS_PENDING;
}


 
BOOLEAN
ToasterArmForWake(
    IN PFDO_DATA    FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

New Routine Description:
    ToasterArmForWake arms, or re-arms a hardware instance for wake-up by
    requesting a IRP_MN_WAIT_WAKE power IRP to the device stack.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance to
    arm for wake.

    DeviceStateChange
    DeviceStateChange represents what system component is arming for wake. If
    DeviceStateChange equals TRUE, then the PnP manager is arming for wake.
    Otherwise, if DeviceStateChange equals FALSE then the WMI library is arming
    for wake.

Return Value Description:
    ToasterArmForWake returns TRUE if the hardware instance is already armed for
    wake-up. Otherwise, ToasterArmForWake returns FALSE if the hardware instance
    is not armed for wake-up.

--*/
{
    NTSTATUS     status;
    WAKESTATE    oldWakeState;
    POWER_STATE  powerState;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ArmForWake\n");

    //
    // Lock the arming/disarming logic to prevent potential simultaneous arming and
    // disarming.
    //
    // The process of arming a hardware instance to signal wake-up can execute in
    // the context of a user-mode thread. The call to KeEnterCriticalRegion prevents
    // the user-mode thread from being suspended while processing the arming request
    // until KeLeaveCriticalRegion is called.
    //
    // KeEnterCriticalRegion disables the system from sending normal kernel-mode
    // asynchronous procedure calls (APCs). KeLeaveCriticalRegion reenables the
    // delivery of normal kernel-mode APCs.
    //
    KeEnterCriticalRegion();

    //
    // Call KeWaitForSingleObject to suspend the thread executing in
    // ToasterArmForWake until the WakeDisableEnableLock kernel event is signaled.
    // KeWaitForSingleObject does not return until the kernel event is signaled.
    // The event is initialized to signaled in ToasterAddDevice as a synchronization
    // event. When such an event is set, a single waiting thread becomes eligible for
    // execution. The system automatically resets (clears) WakeDisableEnableLock each
    // time a wait is satisfied.
    //
    KeWaitForSingleObject( &FdoData->WakeDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    if (TRUE == DeviceStateChange)
    {
        //
        // If the DeviceStateChange parameter equals TRUE, then that indicates the
        // PnP manager is the source of the call to ToasterArmForWake, and the
        // function driver is processing IRP_MN_START_DEVICE or
        // IRP_MN_CANCEL_REMOVE_DEVICE. Therefore set the device extension's
        // AllowWakeArming member to TRUE in order to allow present and future
        // attempts to arm.
        //
        // Before AllowWakeArming is set to TRUE, the function driver fails all
        // attempts to arm the hardware instance for wait/wake.
        //
        FdoData->AllowWakeArming = TRUE;
    }

    if (!(TRUE == FdoData->AllowWakeArming && TRUE == ToasterGetWaitWakeEnableState(FdoData)))
    {
        //
        // If the device extension's AllowWakeArming member does not equal TRUE,
        // because the hardware instance is no longer present or stopped, or the
        // function driver's wait/wake registry key does not indicate the hardware
        // instance supports wait/wake, then return failure.
        //
        status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        //
        // If FdoData->WakeState equals WAKESTATE_DISARMED, then
        // InterlockedCompareExchange sets FdoData->WakeState to WAKESTATE_WAITING.
        // Otherwise, InterlockedCompareExchange does not change FdoData->WakeState.
        //
        oldWakeState = InterlockedCompareExchange((PULONG) &FdoData->WakeState,
                                                   WAKESTATE_WAITING,
                                                   WAKESTATE_DISARMED );

        if (WAKESTATE_DISARMED != oldWakeState)
        {
            //
            // If the call to InterlockedCompareExchange did not change
            // FdoData->WakeState to WAKESTATE_DISARMED then the hardware instance is
            // already armed, or in the process of arming or re-arming for wake-up.
            //
            status = STATUS_SUCCESS;
        }
        else
        {
            //
            // The call to InterlockedCompareExchange changed FdoData->WakeState to
            // WAKESTATE_WAITING, and the ToasterDispatchWaitWake routine is ready to
            // receive an IRP_MN_WAIT_WAKE power IRP.
            //
            // Clear the WakeCompletedEvent kernel event. When the IRP_MN_WAIT_WAKE
            // power IRP requested by the function driver completes and the system
            // calls the ToasterWaitWakePoCompletionRoutine routine (which is
            // different than the ToasterWaitWakeIoCompletionRoutine),
            // ToasterWaitWakePoCompletionRoutine signals the WakeCompletedEvent
            // kernel event, which unblocks the thread executing in the
            // ToasterDisarmWake routine.
            //
            KeClearEvent(&FdoData->WakeCompletedEvent);

            //
            // Specify the least-powered system power state from which 
            // the device can wake the system.
            //
            powerState.SystemState = FdoData->DeviceCaps.SystemWake;

            //
            // Request the power manager to send a IRP_MN_WAIT_WAKE power IRP to the
            // hardware instance's device stack. The function driver will eventually
            // receive and process the IRP_MN_WAIT_WAKE power IRP in the
            // ToasterDispatchWaitWake routine
            //
            // If PoRequestPowerIrp returns STATUS_PENDING then that indicates
            // success.
            //
            status = PoRequestPowerIrp( FdoData->UnderlyingPDO,
                                        IRP_MN_WAIT_WAKE,
                                        powerState,
                                        ToasterWaitWakePoCompletionRoutine,
                                        (PVOID)FdoData,
                                        NULL );

            if (!NT_SUCCESS(status))
            {
                //
                // If the IRP_MN_WAIT_WAKE power IRP request fails then reset the
                // device extension's WakeState to indicate the hardware instance is
                // disarmed.
                //
                FdoData->WakeState = WAKESTATE_DISARMED;

                //
                // Reset the WakeCompletedEvent kernel event to signaled because it
                // was cleared earlier in the call to KeClearEvent.
                //
                KeSetEvent( &FdoData->WakeCompletedEvent,
                            IO_NO_INCREMENT,
                            FALSE );
            }
        }
    }

    //
    // Unlock the arm/disarm logic.
    //
    // Signal the WakeDisableEnableLock kernel event. After the event is set, a
    // single waiting thread becomes eligible for execution. The system automatically
    // resets the event to the not-signaled state each time a wait is satisfied.
    //
    KeSetEvent( &FdoData->WakeDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    //
    // Reenable the delivery of normal kernel-mode APCs. The system was prevented
    // from sending normal kernel-mode asynchronous procedure calls (APCs) earlier
    // when ToasterArmForWake called KeEnterCriticalRegion.
    //
    KeLeaveCriticalRegion();

    return (STATUS_PENDING == status);
 }



 
NTSTATUS
ToasterWaitWakeIoCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

New Routine Description:
    ToasterWaitWakeIoCompletionRoutine is the function driver's I/O completion
    routine for IRP_MN_WAIT_WAKE power IRPs. This routine continues to process
    the incoming IRP_MN_WAIT_WAKE power IRP that was pended earlier in
    ToasterDispatchWaitWake.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the wait/wake operation that has been completed by the
    underlying bus driver.

    Context
    Context represents an optional context parameter that the function driver
    specified to the system to pass to this routine. This parameter is not used.

Return Value Description:
    ToasterWaitWakeIoCompletionRoutine returns STATUS_CONTINUE_COMPLETION if the
    IRP_MN_WAIT_WAKE power IRP hasn't been cancelled. Otherwise
    ToasterWaitWakeIoCompletionRoutine returns STATUS_MORE_PROCESSING_REQUIRED if
    the IRP_MN_WAIT_WAKE power IRP has been cancelled. If the IRP_MN_WAIT_WAKE
    power IRP has been cancelled, then the ToasterDisarmWake routine completes it.

--*/
{
    PFDO_DATA               fdoData;
    WAKESTATE       oldWakeState;

    ToasterDebugPrint(TRACE, "Entered WaitWakeIoCompletionRoutine\n");

    fdoData  = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Set the device extension's WakeState to WAKESTATE_COMPLETING as an atomic
    // operation.
    //
    oldWakeState = InterlockedExchange( (PULONG)&fdoData->WakeState,
                                                    WAKESTATE_COMPLETING );

    if (WAKESTATE_ARMED == oldWakeState)
    {
        //
        // If the device extension's previous WakeState equaled WAKESTATE_ARMED, then
        // that indicates that the IRP_MN_WAIT_WAKE power IRP has not been cancelled.
        // That is, the function driver has not called IoCancelIrp to cancel the
        // IRP_MN_WAIT_WAKE power IRP, and the I/O manager can continue calling I/O
        // completion routines registered earlier by higher level drivers.
        //
        return STATUS_CONTINUE_COMPLETION;
    }
    else
    {
        //
        // Test the assumption that the device extension's WakeState should equal
        // WAKESTATE_ARMING_CANCELLED.
        //
        // If the device extension's previous WakeState did not equal
        // WAKESTATE_ARMED, then that indicates the IRP_MN_WAIT_WAKE power IRP is
        // being cancelled right now, and ToasterDisarmWake is attempting to change
        // WakeState to WAKESTATE_ARMED and the earlier call to InterlockedExchange
        // will cause ToasterDisarmWake to complete the IRP_MN_WAIT_WAKE power IRP.
        //
        ASSERT(WAKESTATE_ARMING_CANCELLED == oldWakeState);

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}


 
VOID
ToasterDisarmWake(
    IN PFDO_DATA    FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

New Routine Description:
    ToasterDisarmWake cancels an IRP_MN_WAIT_WAKE power IRP that was requested
    earlier by the function driver.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance to
    disarm for wake.

    DeviceStateChange
    DeviceStateChange represents what system component is disarming for wake. If
    DeviceStateChange equals TRUE, then the PnP manager is disarming for wake.
    Otherwise, if DeviceStateChange equals FALSE then the WMI library is disarming
    for wake.

Return Value Description:
    ToasterDisarmWake does not return a value.

--*/
{
    WAKESTATE       oldWakeState;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered DisarmWake\n");

    //
    // Lock the arming/disarming logic to prevent potential simultaneous arming and
    // disarming.
    //
    // The process of disarming a hardware instance to signal wake-up can execute in
    // the context of a user-mode thread. The call to KeEnterCriticalRegion prevents
    // the user-mode thread from being suspended while processing the arming request
    // until KeLeaveCriticalRegion is called.
    //
    // KeEnterCriticalRegion disables the system from sending normal kernel-mode
    // asynchronous procedure calls (APCs). KeLeaveCriticalRegion reenables the
    // delivery of normal kernel-mode APCs.
    //
    KeEnterCriticalRegion();

    //
    // Call KeWaitForSingleObject to suspend the thread executing in
    // ToasterArmForWake until the WakeDisableEnableLock kernel event is signaled.
    // KeWaitForSingleObject does not return until the kernel event is signaled.
    // The event is initialized to signaled in ToasterAddDevice as a synchronization
    // event. When such an event is set, a single waiting thread becomes eligible for
    // execution. The system automatically resets (clears) WakeDisableEnableLock each
    // time a wait is satisfied.
    //
    KeWaitForSingleObject( &FdoData->WakeDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    if (TRUE == DeviceStateChange)
    {
        //
        // If the DeviceStateChange parameter equals TRUE, then that indicates the
        // PnP manager is the source of the call to ToasterDisarmWake, and the 
        // function driver is processing IRP_MN_STOP_DEVICE,
        // IRP_MN_QUERY_REMOVE_DEVICE, or IRP_MN_SURPRISE_REMOVAL. Therefore set the
        // device extension's AllowWakeArming member to FALSE in order to prevent any
        // attempts to arm the hardware instance for wake-up until the function
        // driver subsequently processes IRP_MN_START_DEVICE or
        // IRP_MN_CANCEL_REMOVE_DEVICE.
        //
        FdoData->AllowWakeArming = FALSE;
    }

    //
    // Advance the device extension's WakeState member to its respective cancelled
    // state, if appropriate.
    //
    // If WakeState equals WAKESTATE_WAITING, then InterlockedOr sets WakeState to
    // WAKESTATE_WAITING_CANCELLED to indicate that the IRP_MN_WAIT_WAKE power IRP
    // has been canceled before the function driver has even received it.
    //
    // If WakeState equals WAKESTATE_ARMED, then InterlockedOr sets WakeState to
    // WAKESTATE_ARMING_CANCELLED to indicate that the function driver has received
    // and partially processed the IRP_MN_WAIT_WAKE power IRP, but the function
    // driver has not completed the power IRP.
    //
    // If WakeState equals WAKESTATE_DISARMED or WAKESTATE_COMPLETING, then
    // InterlockedOr does not change WakeState.
    //
    oldWakeState = InterlockedOr( (PULONG)&FdoData->WakeState, 1 );

    if (WAKESTATE_ARMED == oldWakeState)
    {
        ToasterDebugPrint(TRACE, "canceling wakeIrp\n");

        //
        // Set the IRP_MN_WAIT_WAKE power IRP's cancel bit to TRUE.
        //
        IoCancelIrp( FdoData->WakeIrp );

        //
        // The function driver has cancelled the wait/wake power IRP. Attempt to
        // return ownership of the wait/wake power IRP to the completion routine,
        // ToasterWaitWakeIoCompletionRoutine, by restoring FdoData->WakeState to
        // WAKE_STATE_ARMED.
        //
        // If WAKESTATE_ARMING_CANCELLED == FdoData->WakeState (changed earlier in
        // the call to InterlockedOr), then InterlockedCompareExchange sets
        // FdoData->WakeState to WAKESTATE_ARMED. Otherwise,
        // InterlockedCompareExchange does not change FdoData->WakeState.
        //
        oldWakeState = InterlockedCompareExchange((PULONG) &FdoData->WakeState,
                                                   WAKESTATE_ARMED,
                                                   WAKESTATE_ARMING_CANCELLED );

        if (WAKESTATE_COMPLETING == oldWakeState)
        {
            //
            // If InterlockedExchangeCompare returns WAKESTATE_COMPLETING, then the
            // function driver has completed processing the IRP_MN_WAIT_WAKE power
            // IRP and it can be completed.
            //
            IoCompleteRequest( FdoData->WakeIrp, IO_NO_INCREMENT );
        }
    }

    //
    // Wait for the IRP_MN_WAIT_WAKE power IRP to finish.
    // ToasterWaitWakePoCompletionRoutine signals WakeCompletedEvent when the
    // IRP_MN_WAIT_WAKE power IRP has been completed
    //
    KeWaitForSingleObject( &FdoData->WakeCompletedEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

    //
    // Unlock the arm/disarm logic.
    //
    // Signal the WakeDisableEnableLock kernel event. After the event is set, a
    // single waiting thread becomes eligible for execution. The system automatically
    // resets the event to the not-signaled state each time a wait is satisfied.
    //
    KeSetEvent( &FdoData->WakeDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    //
    // Reenable the delivery of normal kernel-mode APCs. The system was prevented
    // from sending normal kernel-mode asynchronous procedure calls (APCs) earlier
    // when ToasterArmForWake called KeEnterCriticalRegion.
    //
    KeLeaveCriticalRegion();
}


 
VOID
ToasterWaitWakePoCompletionRoutine(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         State,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
/*++

New Routine Description:
    ToasterWaitWakePoCompletionRoutine is the function driver’s power completion
    routine for IRP_MN_WAIT_WAKE power IRP's that the function driver requested
    earlier when ToasterArmForWake called PoRequestPowerIrp.

    The system calls this routine after the IRP_MN_WAIT_WAKE power IRP has been
    completed and all I/O completion routines, if any, have returned.

    The DDK PoRequestPowerIrp topic defines the function prototype that
    ToasterWaitWakePoCompletionRoutine must conform to.

Parameters Description:
    DeviceObject
    DeviceObject represents the target device object for the completed
    wait/wake power IRP. This parameter is not used.

    MinorFunction
    MinorFunction represents the minor function code for the completed
    wait/wake power IRP. This parameter is not used.

    State
    State represents the new device power state. This parameter is not used.

    Context
    Context represents the device extension for the hardware instance.
    ToasterArmForWake set the system to pass the device extension in the call to
    PoRequestPowerIrp.

    IoStatus
    IoStatus represents the I/O status block structure for the completed
    wait/wake power IRP.

Return Value Description:
    ToasterWaitWakePoCompletionRoutine does not return a value.

--*/
{
    PFDO_DATA               fdoData;

    ToasterDebugPrint(TRACE, "WaitWakePoCompletionRoutine  \n");

    //
    // Get the pointer to the FDO’s device extension. The device extension is used to
    // store any information that must not be paged out. This includes hardware state
    // information, storage for mechanisms implemented by the function driver, and
    // pointers to the underlying PDO and the next lower driver.
    //
    // The FDO’s device extension is defined as a PVOID. The PVOID must be recast to
    // a pointer to the data type of the device extension, PFDO_DATA.
    //
    fdoData  = (PFDO_DATA) Context;

    //
    // Clear the device extension's WakeIrp member to NULL. This step is not required,
    // but is helpful when debugging.
    //
    fdoData->WakeIrp = NULL;

    //
    // Re-initialize the device extension's WakeState member to WAKESTATE_DISARMED.
    // The previous state should be WAKESTATE_COMPLETING .
    //
    fdoData->WakeState = WAKESTATE_DISARMED;

    //
    // Signal the WakeCompletedEvent kernel event so that ToasterDisarmWake can
    // proceed and the function driver can request another IRP_MN_WAIT_WAKE power
    // IRP.
    //
    KeSetEvent( &fdoData->WakeCompletedEvent,
                IO_NO_INCREMENT,
                FALSE );

    if (NT_SUCCESS(IoStatus->Status))
    {
        //
        // The system has awakened and successfully completed the IRP_MN_WAIT_WAKE
        // power IRP.
        //
        // Because the routines for arming a hardware instance for wake-up can only
        // be called at IRQL = PASSIVE_LEVEL, call
        // ToasterQueuePassiveLevelCallback (which is different than
        // ToasterQueuePassiveLevelPowerCallback) to queue a work item for the system
        // worker thread to process at IRQL = PASSIVE_LEVEL.
         //
        // Specify the work item to call the
        // ToasterPassiveLevelReArmCallbackWorker routine to check the function
        // driver's WaitWakeEnabled Registry key again to determine the user wake-up
        // preference and re-arm the hardware instance if required.
        //
        ToasterQueuePassiveLevelCallback(
                                    fdoData,
                                    ToasterPassiveLevelReArmCallbackWorker
                                    );
    }
    else if (STATUS_UNSUCCESSFUL == IoStatus->Status || 
             STATUS_NOT_IMPLEMENTED == IoStatus->Status ||
             STATUS_NOT_SUPPORTED == IoStatus->Status)
    {
        //
        // The underlying bus driver is not capable of awakening the system from a
        // low-power state.
        //
        // Because the routines for disarming a hardware instance can only be called
        // at IRQL = PASSIVE_LEVEL, call ToasterQueuePassiveLevelCallback (which is
        // different than ToasterQueuePassiveLevelPowerCallback) to queue a work item
        // for the system worker thread to process at IRQL = PASSIVE_LEVEL. 
        //
        // Specify the work item to call the
        // ToasterPassiveLevelClearWaitWakeEnableState routine to clear the function
        // driver's WaitWakeEnabled Registry key. Clearing the Registry key provides
        // feedback to the user as to whether or not the device is armed for wake-up.
        //
        ToasterQueuePassiveLevelCallback(
                                    fdoData,
                                    ToasterPassiveLevelClearWaitWakeEnableState
                                    );
    }
 
    return;
}


 
VOID
ToasterQueuePassiveLevelCallback(
    IN PFDO_DATA    FdoData,
     IN PIO_WORKITEM_ROUTINE CallbackFunction
    )
/*++

New Routine Description:
    ToasterQueuePassiveLevelCallback queues a work item to be processed later
    by the system worker thread at IRQL = PASSIVE_LEVEL. The system thread calls
    the work item’s callback routine. However, if
    ToasterQueuePassiveLevelCallback is called at IRQL < DISPATCH_LEVEL, then
    ToasterQueuePassiveLevelCallback calls the callback routine immediately,
    without creating a work item for the system worker thread to process later
    at IRQL = PASSIVE_LEVEL.
 
 Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    processing the wait/wake power IRP.

    CallbackFunction
    CallbackFunction specifies the routine for the worker thread to callback at
    IRQL = PASSIVE_LEVEL.

Return Value Description:
    ToasterQueuePassiveLevelCallback does not return a value. 

--*/
{
    PIO_WORKITEM            item;

    //
    // Call the callback routine described by the CallbackFunction parameter directly
    // if the current thread is not executing at IRQL = DISPATCH_LEVEL. Otherwise,
    // queue a work item for the system worker thread to process at
    // IRQL = PASSIVE_LEVEL.
    //
    if(DISPATCH_LEVEL !=  KeGetCurrentIrql())
    {
        (*CallbackFunction)(FdoData->Self, NULL);
    }
    else
    {
        //
        // Windows 98 does not support IoAllocateWorkItem, it only supports
        // ExInitializeWorkItem and ExQueueWorkItem. The Toaster sample function
        // driver uses IoAllocateWorkItem because
        // ExInitializeWorkItem/ExQueueWorkItem can cause the system to crash when
        // the driver is unloaded. For example, the system can unload the driver in
        // the middle of an ExQueueWorkItem callback.
        //
        // After the callback routine described by the CallbackFunction parameter
        // (ToasterPassiveLevelReArmCallbackWorker or
        // ToasterPassiveLevelClearWaitWakeEnableState) processes the work item it
        // then frees the work item.
        //
        item = IoAllocateWorkItem(FdoData->Self);

        if (NULL != item)
        {
            //
            // Queue the initialized work item for the system worker thread. When the
            // system worker thread later processes the work item, it calls the
            // callback routine specified by the incoming CallbackFunction parameter
            // and passes it the information in the item variable.
            //
            IoQueueWorkItem( item,
                                        CallbackFunction,
                                        DelayedWorkQueue,
                                        item
                                        );
        }
     }
}


 
VOID
ToasterPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
/*++

New Routine Description:
    ToasterPassiveLevelReArmCallbackWorker re-arms the hardware instance after the
    function driver has completed a previous IRP_MN_WAIT_WAIT power IRP.
 
Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Context parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Context
    Context describes the work item initialized earlier when
    ToasterQueuePassiveLevelCallback was called to process a power IRP at
    IRQL = PASSIVE_LEVEL.
   
Return Value Description:
    ToasterPassiveLevelReArmCallbackWorker does not return a value.

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterPassiveLevelReArmCallbackWorker\n");

    //
    // Notify the hardware instance to re-arm itself for wake if wait/wake is enabled
    // in the Registry.
    //
    ToasterArmForWake(fdoData, FALSE);

    if(NULL != Context)
    {
        //
        // Free the work item allocated earlier in ToasterQueuePassiveLevelCallback.
        //
        IoFreeWorkItem((PIO_WORKITEM)Context);
    }
}

 
VOID
ToasterPassiveLevelClearWaitWakeEnableState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
/*++

New Routine Description:
    ToasterPassiveLevelClearWaitWakeEnableState clears the hardware instance's
    WaitWakeEnabled Registry key after the function driver has completed a
    previous IRP_MN_WAIT_WAIT power IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Context parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Context
    Context describes the work item initialized earlier when
    ToasterQueuePassiveLevelCallback was called to process a power IRP at
    IRQL = PASSIVE_LEVEL. Context contains the power IRP to be processed.
   
Return Value Description:
    ToasterPassiveLevelClearWaitWakeEnableState does not return a value.

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterPassiveLevelClearWaitWakeEnableState\n");

    //
    // Clear the wait/wake state in the Registry.
    //
    ToasterSetWaitWakeEnableState(fdoData, FALSE);

    if(NULL != Context)
    {
        //
        // Free the work item allocated earlier in ToasterQueuePassiveLevelCallback.
        //
        IoFreeWorkItem((PIO_WORKITEM)Context);
    }
}


 
VOID
ToasterAdjustCapabilities(
    IN OUT PDEVICE_CAPABILITIES DeviceCapabilities
    )
/*++

New Routine Description:
    ToasterAdjustCapabilities updates the hardware instance's device capabilities
    after the underlying bus driver has processed IRP_MN_QUERY_CAPABILITIES.
    However, because some bus drivers are written to the WDM 1.0 specification
    they do not set the DeviceD* and WakeFromD* members of the DEVICE_CAPABILITIES
    structure. ToasterAdjustCapabilities calculates the proper values to set these
    members to.

Parameters Description:
    DeviceCapabilities
    DeviceCapabilities represents the capabilities of the hardware instance, such
    as whether the device can be locked or ejected, and what device power states
    the hardware can wake the system from.

Return Value:
    ToasterAdjustCapabilities does not return a value.

--*/
{
    DEVICE_POWER_STATE  dState;
    SYSTEM_POWER_STATE  sState;
    DEVICE_POWER_STATE  deepestDeviceWakeState;
    int i;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "AdjustCapabilities  \n");

    //
    // Determine the hardware instance's power capabilities and set the DeviceD1
    // DeviceD2 members based on the system power state to device power state
    // mapping. The DeviceD1 bit indicates that the hardware instance supports the
    // D1 device power state. The DeviceD2 bit indicates that the hardware instance
    // supports the D2 device power state.
    //
    for(sState=PowerSystemSleeping1; sState<=PowerSystemHibernate; sState++)
    {

        if (PowerDeviceD1 == DeviceCapabilities->DeviceState[sState])
        {
            //
            // Adjust the device capabilities DeviceD1 member to indicate that the
            // hardware instance supports the DeviceD1 device power state.
            //
            DeviceCapabilities->DeviceD1 = TRUE;
        }
        else if (PowerDeviceD2 == DeviceCapabilities->DeviceState[sState])
        {
            //
            // Adjust the device capabilities DeviceD2 member to indicate that the
            // hardware instance supports the DeviceD2 device power state.
            //
            DeviceCapabilities->DeviceD2 = TRUE;
        }

    }

    dState = DeviceCapabilities->DeviceWake;

    for(i=0; i<2; i++)
    {
        //
        // Determine the least-powered device power state from which the hardware
        // instance can signal wake-up and set the WakeFromD* and DeviceD* members
        // as appropriate.
        //
        // In the first iteration of the for loop, ToasterAdjustCapabilities
        // determines the least-powered device power state from which the hardware
        // instance can signal wake-up based on the DeviceWake member.
        //
        // In the second iteration of the for loop, ToasterAdjustCapabilities
        // determines the least-powered device power state from which the hardware
        // instance can signal wake-up based on the most-powered device power state
        // that the hardware instance can maintain for the least-powered system power
        // state from which the device can signal wake-up.
        //
        switch(dState)
        {
        case PowerDeviceD0:
            DeviceCapabilities->WakeFromD0 = TRUE;
            break;
        case PowerDeviceD1:
            DeviceCapabilities->DeviceD1 = TRUE;
            DeviceCapabilities->WakeFromD1 = TRUE;
            break;
        case PowerDeviceD2:
            DeviceCapabilities->DeviceD2 = TRUE;
            DeviceCapabilities->WakeFromD2 = TRUE;
            break;
        case PowerDeviceD3:
            DeviceCapabilities->WakeFromD3 = TRUE;
            break;
        case PowerDeviceUnspecified:
            break;
        default:
            ASSERT(0);
        }

        //
        // Set the dState variable for the second iteration of the for loop.
        //
        if (PowerSystemUnspecified != DeviceCapabilities->SystemWake)
        {
            dState = DeviceCapabilities->DeviceState[DeviceCapabilities->SystemWake];
        }
        else
        {
            dState = PowerDeviceUnspecified;
        }
    }

    //
    // Determine the deepest device power state from which the hardware instance can
    // signal wake-up.
    //
    if (DeviceCapabilities->WakeFromD3)
    {
        deepestDeviceWakeState = PowerDeviceD3;
    }

    else if (DeviceCapabilities->WakeFromD2)
    {
        deepestDeviceWakeState = PowerDeviceD2;
    }

    else if (DeviceCapabilities->WakeFromD1)
    {
        deepestDeviceWakeState = PowerDeviceD1;
    }

    else if (DeviceCapabilities->WakeFromD0)
    {
        deepestDeviceWakeState = PowerDeviceD0;
    }

    else
    {
        deepestDeviceWakeState = PowerDeviceUnspecified;
    }

    sState = DeviceCapabilities->SystemWake;

    if (PowerSystemUnspecified != sState)
    {
        //
        // If the hardware instance cannot signal wake-up, then the underlying bus
        // driver sets the hardware instance's device capabilities' SystemWake member
        // to PowerSystemUnspecified.
        //
        // Test the assumption that the most-powered device power state that the
        // hardware instance can maintain for the least-powered system power state
        // from which the device can signal wake-up is enough power to cover the
        // deepest device wake state, as determined earlier.
        //
        ASSERT(DeviceCapabilities->DeviceState[sState] <= deepestDeviceWakeState);
    }
    else if (PowerDeviceUnspecified != deepestDeviceWakeState)
    {
        //
        // If the hardware instance can signal wake-up, then set the hardware
        // instance device capabilities' SystemWake member to the lowest-powered
        // system power state from which the hardware instance can signal wake-up.
        //
        // Start at the S3 system power state. If ToasterAdjustCapabilities does not
        // set the SystemWake member, but can signal wake-up from D3, then do not
        // assume that the function driver can wake the system from the S4 or S5
        // system power states.
        //
        for(sState=PowerSystemSleeping3; sState>=PowerSystemWorking; sState--)
        {

            if ((PowerDeviceUnspecified != DeviceCapabilities->DeviceState[sState]) &&
                (DeviceCapabilities->DeviceState[sState] <= deepestDeviceWakeState))
            {
                break;
            }
        }

        //
        // Set the hardware instance's device capabilities' SystemWake member to
        // PowerSystemUnspecified because the lowest system power state from which
        // the hardware instance can signal wake-up was not found.
        //
        DeviceCapabilities->SystemWake = sState;
    }
}


 
NTSTATUS
ToasterSetWaitWakeEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN WakeState
    )
/*++

New Routine Description:
    ToasterSetWaitWakeEnableState sets the WaitWakeEnable Registry key to the
    value specified in the WakeState parameter. The function driver calls this
    routine in response to WMI_POWER_DEVICE_WAKE_ENABLE requests.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance to
    set the wait/wake enable state for in the Registry.

    WakeState
    WakeState represents the TRUE or FALSE state to store in the function driver's
    "WaitWakeEnable" key in the Registry.

Return Value Description:
    ToasterSetWaitWakeEnableState returns TRUE if it is able to successfully open
    the function driver's "WaitWakeEnabled" registry key. Otherwise, if there was
    an error opening the registry key, then ToasterSetWaitWakeEnableState returns
    the value from IoOpenDeviceRegistryKey.

--*/
{
    HANDLE hKey = NULL;
    NTSTATUS status;
    ULONG tmp = WakeState;
    UNICODE_STRING strEnable;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "ToasterSetWaitWakeEnableState  \n");

    //
    // Open the function driver's registry key that corresponds to the specific
    // device interface instance.
    //
    status = IoOpenDeviceRegistryKey (FdoData->UnderlyingPDO,
                                             PLUGPLAY_REGKEY_DEVICE,
                                             STANDARD_RIGHTS_ALL,
                                             &hKey);
    if (NT_SUCCESS(status))
    {
        //
        // Copy the string name of the function driver's WaitWakeEnable registry key,
        // which is defined in Toaster.h as TOASTER_WAIT_WAKE_ENABLE, into the
        // temporary strEnable variable.
        //
        RtlInitUnicodeString (&strEnable, TOASTER_WAIT_WAKE_ENABLE);

        //
        // Write the incoming WakeState parameter to the "WaitWakeEnable" key.
        //
        ZwSetValueKey (hKey,
                       &strEnable,
                       0,
                       REG_DWORD,
                       &tmp,
                       sizeof(tmp));

        //
        // Close the device interface instance key's handle that was opened earlier
        // in the call to IoOpenDeviceRegistryKey.
        //
        ZwClose (hKey);
    }

    return status;
}


 
BOOLEAN
ToasterGetWaitWakeEnableState(
    IN PFDO_DATA   FdoData
    )
/*++

New Routine Description:
    ToasterGetWaitWakeEnableState returns the value of the WaitWakeEnabled
    Registry key. The function driver calls this routine in response to
    WMI_POWER_DEVICE_WAKE_ENABLE requests.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance to
    return the wait/wake enable state for in the Registry.

Return Value Description:
    ToasterGetWaitWakeEnableState returns TRUE if the hardware instance is enabled
    for wait/wake. Otherwise ToasterGetWaitWakeEnableState returns FALSE if the
    device interface instance's "WaitWakeEnabled" registry key is not present or
    there was an error opening the Registry, or if ToasterGetWaitWakeEnableState
    could not allocate memory to read the full information from the device
    interface instance's "WaitWakeEnabled" registry key.

--*/
{
    HANDLE hKey = NULL;
    NTSTATUS status;
    ULONG tmp;
    BOOLEAN waitWakeEnabled = FALSE;
    ULONG           length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;
    UNICODE_STRING  valueName;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "ToasterGetWaitWakeEnableState  \n");

    //
    // Open the function driver's registry key that corresponds to the specific
    // device interface instance.
    //
    status = IoOpenDeviceRegistryKey (FdoData->UnderlyingPDO,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &hKey);

    if (NT_SUCCESS (status))
    {
        //
        // Copy the string name of the function driver's WaitWakeEnable registry key,
        // which is defined in Toaster.h as TOASTER_WAIT_WAKE_ENABLE, into the
        // temporary valueName variable.
        //
        RtlInitUnicodeString (&valueName, TOASTER_WAIT_WAKE_ENABLE);

        length = sizeof (KEY_VALUE_FULL_INFORMATION)
                               + valueName.MaximumLength
                               + sizeof(ULONG);

        fullInfo = ExAllocatePoolWithTag (PagedPool, length, TOASTER_POOL_TAG);

        if (NULL != fullInfo)
        {
            //
            // Copy the specific device interface instance's "WaitWakeEnable" key
            // into the valueName variable.
            //
            status = ZwQueryValueKey (hKey,
                                      &valueName,
                                      KeyValueFullInformation,
                                      fullInfo,
                                      length,
                                      &length);

            if (NT_SUCCESS (status))
            {
                //
                // Test the assumption that the size of the data returned from the
                // Registry equals 4 bytes.
                //
                ASSERT (sizeof(ULONG) == fullInfo->DataLength);

                //
                // Copy the "WaitWakeEnable" value into the temporary tmp variable.
                //
                RtlCopyMemory (&tmp,
                               ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                               fullInfo->DataLength);

                //
                // Determine if the "waitWaitEnable" value is TRUE. If so, set the
                // return value to TRUE to indicate that the hardware instance
                // supports wait/wake. Otherwise set the value to FALSE to indicate
                // that the hardware instance does not support wait/wake.
                //
                waitWakeEnabled = tmp ? TRUE : FALSE;
            }

            ExFreePool (fullInfo);
        }
        else
        {
            //
            // Fail the routine if there is not enough memory to read the full
            // information from the device interface instance's "WaitWakeEnabled"
            // registry key.
            //
            waitWakeEnabled =  FALSE;
        }

        //
        // Close the device interface instance key's handle that was opened earlier
        // in the call to IoOpenDeviceRegistryKey.
        //
        ZwClose (hKey);
    }

    return waitWakeEnabled;
}
