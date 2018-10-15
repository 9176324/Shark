/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Power.c

Abstract:

    Power.c implements the power policy for the Toaster sample function driver.
    The function driver for a hardware device is usually the hardware’s power
    policy owner. However, for some devices, another driver or system component
    can assume this role. The Toaster sample function driver is the power policy
    owner for Toaster class hardware

    The Toaster sample function driver implements a simple power policy: A
    hardware instance enters the device power state D0 (working; fully powered and
    ready to process I/O operations) when the system enters the system power state
    S0 (working; fully powered and operational). A hardware instance enters the
    lowest-powered device state, D3 (sleeping), for all the other system power
    states (S1-S5). Refer to the DDK documentation for a thorough explanation of
    the device D* and system S* power states.

    Power IRPs are divided into two categories: system power IRPs (S-IRPs) and
    device power IRPs (D-IRPs). S-IRPs represent the system’s overall power state,
    whereas D-IRPs represent the hardware instance’s device power state.

    The system initially sends S-IRPs to the device stack for a hardware instance.
    When the power policy owner for the hardware instance receives an S-IRP it
    creates a corresponding D-IRP for the original S-IRP. The power policy owner
    marks the original S-IRP as pending and waits to receive and process the
    corresponding D-IRP that it requested earlier. The power policy owner
    completes the corresponding D-IRP first, copies the status of the D-IRP into
    the original S-IRP, and then finally completes the original S-IRP.

    Power management features implemented in this stage of the function driver:

    -Routines to process IRP_MN_QUERY_POWER S-IRPs. These routines are named
     ToasterDispatchSystemPowerIrp and ToasterCompletionSystemPowerUp.

    -Routines to process IRP_MN_QUERY_POWER D-IRPs. These routines are named
     ToasterDispatchQueryPowerState, ToasterDispatchDeviceQueryPower, and
     ToasterCallbackHandleDeviceQueryPower.

    -Routines to process IRP_MN_SET_POWER power-up D-IRPs. These routines are
     named ToasterDispatchSetPowerState, ToasterDispatchDeviceSetPower, and
     ToasterCompletionDevicePowerUp.

    -Routines to process IRP_MN_SET_POWER power-down D-IRPs. These routines are
     named ToasterDispatchSetPowerState, ToasterDispatchDeviceSetPower, and
     ToasterCallbackHandleDeviceSetPower.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub  02-Oct-2001 -
     Based on Adrian J. Oney's power management boilerplate code 
     
    Kevin Shirley  29-May-2003 - Commented for tutorial and learning purposes

--*/

#include "toaster.h"
#include "power.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ToasterDispatchPower)
#pragma alloc_text(PAGE, ToasterDispatchPowerDefault)
#pragma alloc_text(PAGE, ToasterDispatchSetPowerState)
#pragma alloc_text(PAGE, ToasterDispatchQueryPowerState)
#pragma alloc_text(PAGE, ToasterDispatchSystemPowerIrp)
#pragma alloc_text(PAGE, ToasterDispatchDeviceQueryPower)
#pragma alloc_text(PAGE, ToasterDispatchDeviceSetPower)
#pragma alloc_text(PAGE, ToasterQueuePassiveLevelPowerCallbackWorker)
#pragma alloc_text(PAGE, ToasterPowerBeginQueuingIrps)
#pragma alloc_text(PAGE, ToasterCallbackHandleDeviceQueryPower)
#pragma alloc_text(PAGE, ToasterCallbackHandleDeviceSetPower)
#pragma alloc_text(PAGE, ToasterCanSuspendDevice)
#endif

//
// The code for Power.c is divided into two sections. First comes the boilerplate
// code, followed by the device specific programming code.
//

 
//
// Begin Boilerplate Code
// ----------------------
//

NTSTATUS
ToasterDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    The system dispatches different IRP_MJ_POWER IRPs to ToasterDispatchPower for
    different purposes:

    -To query a driver to determine if the system can change its overall power
     state to a different power level.

    -To notify a driver of a change in the overall system power state so that the
     driver can adjust the device power state of its hardware instance to an
     appropriate level.

    -To respond to system wake-up events.

    Most function and filter drivers perform some processing for each power IRP
    and then pass the IRP down the device stack without completing it.
    Eventually the power IRP reaches the bus driver, which physically changes the
    power to the hardware instance and completes the power IRP.

    In general, a driver should not cause noticeable delays while processing power
    IRPs. If a driver cannot process a power in a short amount of time, it should
    mark the power IRP as pending, begin to queue any new non-power IRPs (until
    the driver completes the pending power IRP), and return STATUS_PENDING to the
    caller.

    Processing power IRPs is a complex procedure which can involve I/O completion
    routines, power completion routines, and system worker thread work items. The
    following sequence demonstrates how a power policy owner (the function driver
    in this case) processes different power IRPs:

----------------------------------------------------------------------------------

    The power manager sends a IRP_MN_QUERY_POWER S-IRP (query power system-IRP)
    with a specific system power state to query the power policy owner if it can
    change its hardware instance to an appropriate device power state without
    disrupting work.

1a) -The function driver receives an IRP_MN_QUERY_POWER (query power system-IRP)
     S-IRP.
    -The function driver sets the system to call an I/O completion routine
     because the function driver must process the IRP_MN_QUERY_POWER S-IRP on its
     passage back up the device stack, after the underlying bus driver completes
     it.
    -The function driver passes the IRP_MN_QUERY_POWER S-IRP down the device
     stack.
    -The function driver returns STATUS_PENDING to the caller.

    After the bus driver completes the IRP_MN_QUERY_POWER S-IRP, the system passes
    the IRP back up the device stack and calls the I/O completion routine set
    earlier:

1b) -The I/O completion routine requests that the power manager send a
     IRP_MN_QUERY_POWER D-IRP (query power device-IRP) that corresponds to the
     original pending IRP_MN_QUERY_POWER S-IRP. When the I/O completion routine
     requests the corresponding D-IRP it also sets the system to call a power
     completion routine after the function driver completes the corresponding
     D-IRP.
    -The I/O completion routine returns STATUS_MORE_PROCESSING_REQUIRED to the
     caller.

1c) -The function driver receives the corresponding IRP_MN_QUERY_POWER D-IRP and
     either completes it successfully to indicate that the hardware instance can
     change to an appropriate device power state without data loss, or fails the
     D-IRP to indicate that the hardware instance cannot change its device power
     state without disrupting work.

    The system then calls the power completion routine set to be called earlier
    when the I/O completion requested the corresponding D-IRP:

1d) -The power completion routine copies the status of the corresponding completed
     IRP_MN_QUERY_POWER D-IRP into the original pending IRP_MN_QUERY_POWER S-IRP.
    -The power completion routine then completes the original pending
     IRP_MN_QUERY_POWER S-IRP.
    -The power completion routine returns to the caller.

    If the function driver fails the original IRP_MN_QUERY_POWER S-IRP, then the
    power manager might send another IRP_MN_QUERY_POWER S-IRP to specify a
    different system power state, and the previous sequence repeats.

    However, if the function driver succeeds the original IRP_MN_QUERY_POWER
    S-IRP, then the power manager sends a IRP_MN_SET_POWER S-IRP (set power
    system IRP) with the same system power state as the original
    IRP_MN_QUERY_POWER S-IRP to notify the function driver to change its hardware
    instance to the device power state appropriate for the system power state.

    Alternately, even if the function driver fails the original IRP_MN_QUERY_POWER
    S-IRP, the power manager can still send a IRP_MN_SET_POWER S-IRP to notify the
    function driver that the system is preparing to change its power state. The
    power manager might force a driver to change its hardware instance to a
    specific device power state because a battery or UPS is going offline

----------------------------------------------------------------------------------

2a) -The function driver receives an IRP_MN_SET_POWER (set power system-IRP)
     S-IRP.
    -The function driver sets the system to call an I/O completion routine
     because the function driver must process the IRP_MN_SET_POWER S-IRP on its
     passage back up the device stack, after the underlying bus driver completes
     it.
    -The function driver passes the IRP_MN_SET_POWER S-IRP down the device
     stack.
    -The function driver returns STATUS_PENDING to the caller.

    After the bus driver completes the IRP_MN_SET_POWER S-IRP, the system calls
    the I/O completion routine set earlier:

2b) -The I/O completion routine requests that the power manager send a
     IRP_MN_SET_POWER D-IRP (set power device-IRP) that corresponds to the
     original pending IRP_MN_SET_POWER S-IRP. When the I/O completion routine
     requests the corresponding D-IRP it also sets the system to call a power
     completion routine after the function driver completes the corresponding
     D-IRP.
    -The I/O completion routine returns STATUS_MORE_PROCESSING_REQUIRED to the
     caller.

----------------------------------------------------------------------------------

3)  -The function driver receives the corresponding IRP_MN_SET_POWER D-IRP and
     must determine if it is a power-down D-IRP or a power-up D-IRP.

4)  If the original pending IRP_MN_SET_POWER S-IRP specifies a higher system power
    state than the current system power state then the corresponding
    IRP_MN_SET_POWER D-IRP is a power-up D-IRP. The underlying bus driver must
    process power-up D-IRPs before the function driver because the bus driver must
    supply power to the hardware instance before the function driver can use it.
    However, the function driver cannot process the power-up D-IRP until every
    pending IRP (if any) such as read, write, or device control operations
    complete (in other threads of execution). Note that, however, the function
    driver cannot wait in the thread processing the power-up D-IRP because that
    might cause the system to stop responding.

    Therefore, the function driver marks the power-up D-IRP as pending and sets
    the system to call an I/O completion routine after the bus driver completes
    the power-up D-IRP. Then, the function driver passes the power-up D-IRP down
    the device stack. Next, the function driver returns STATUS_PENDING to the
    caller. At this time, both the original IRP_MN_SET_POWER S-IRP and the
    corresponding power-up D-IRP are pending.

    After the bus driver completes the power-up D-IRP, the system calls the I/O
    completion routine set earlier. Recall, that the function driver must still
    wait until every pending IRP completes. However, because the function driver
    cannot wait in the I/O completion routine for every IRP to complete, the I/O
    completion routine queues a callback for the system worker thread to process
    at IRQL = PASSIVE_LEVEL. Then the I/O completion routine returns
    STATUS_MORE_PROCESSING_REQUIRED to the caller.

    Finally, the system worker thread calls the callback routine at
    IRQL = PASSIVE_LEVEL to finish processing the power-up D-IRP. The callback
    routine can then wait until every pending IRP completes by suspending the
    execution of the system worker thread (which will not cause a system deadlock
    because the system worker thread calls the callback routine at
    IRQL = PASSIVE_LEVEL), and then the callback routine finishes processing the
    power-up D-IRP before it returns to the caller.

----------------------------------------------------------------------------------

5)  Otherwise, if the original pending IRP_MN_SET_POWER S-IRP specifies the same
    system power state as the current system power state, or a lower system power
    state than the current system power state then the corresponding
    IRP_MN_SET_POWER D-IRP is a power-down D-IRP. The function driver must process
    power-down D-IRPs before passing them down the device stack to be processed by
    the underlying bus driver because the function driver must save the hardware
    instance’s operating context before the hardware instance is powered-down (or
    powered-off). The context can then be restored later when the hardware
    instance is powered-up.

    Therefore, the function driver queues a callback for the system worker thread
    to process at IRQL = PASSIVE_LEVEL. Then the function routine returns
    STATUS_PENDING to the caller.

    Finally, the system worker thread calls the callback routine at
    IRQL = PASSIVE_LEVEL to finish processing the power-down D-IRP. The callback
    routine can then wait until every pending IRP completes by suspending the
    execution of the system worker thread (which will not cause a system deadlock
    because the system worker thread calls the callback routine at
    IRQL = PASSIVE_LEVEL), and then the callback routine finishes processing the
    power-down D-IRP before it returns to the caller.

Updated Return Value Description:
    ToasterDispatchPower returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. If the function driver
    immediately processes the power IRP, then ToasterDispatchPower returns
    STATUS_SUCCESS. If the function driver marked the power IRP as pending, then
    ToasterDispatchPower returns STATUS_PENDING.

--*/
{
    PIO_STACK_LOCATION  stack;
    PFDO_DATA           fdoData;
    NTSTATUS            status;

    PAGED_CODE();

    stack   = IoGetCurrentIrpStackLocation(Irp);

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "FDO %s IRP:0x%p %s %s\n",
                  DbgPowerMinorFunctionString(stack->MinorFunction),
                  Irp,
                  DbgSystemPowerString(fdoData->SystemPowerState),
                  DbgDevicePowerString(fdoData->DevicePowerState));

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        PoStartNextPowerIrp (Irp);

        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement (fdoData);

        return status;
    }

    if (NotStarted == fdoData->DevicePnPState )
    {
        //
        // If ToasterDispatchPnP has not yet processed IRP_MN_START_DEVICE to start
        // the hardware instance, then pass the power IRP down the device stack.
        //
        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        status = PoCallDriver(fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement (fdoData);

        return status;
    }

    //
    // Determine the power IRP’s minor function code which describes the power
    // operation.
    //
    switch(stack->MinorFunction)
    {
    case IRP_MN_SET_POWER:
        //
        // ToasterDispatchSetPowerState processes IRP_MN_SET_POWER S-IRPs sent by
        // the system as well as IRP_MN_SET_POWER D-IRPs requested by the function
        // driver in response to a IRP_MN_SET_POWER S-IRPs.
        //
        status = ToasterDispatchSetPowerState(DeviceObject, Irp);

        break;

    case IRP_MN_QUERY_POWER:
        //
        // ToasterDispatchQueryPowerState process IRP_MN_QUERY_POWER S-IRPs sent by
        // the system as well as IRP_MN_SET_POWER D-IRPs requested by the function
        // driver in response to IRP_MN_QUERY_POWER S-IRPs.
        //
        status = ToasterDispatchQueryPowerState(DeviceObject, Irp);

        break;

    case IRP_MN_WAIT_WAKE:
        //
        // The power policy owner for a device (in this case the function driver)
        // sends IRP_MN_WAIT_WAKE to the device stack’s PDO to wait its device
        // hardware or to wake the system.
        //
        // The power manager only sends IRP_MN_WAIT_WAKE to devices that always wake
        // the system, such as the power-on switch.
        //
        // The Featured2 stage of the function driver demonstrates how to process
        // wait-wake support.
        //
        // Fall through down to the next switch-case statement.
        //

    case IRP_MN_POWER_SEQUENCE:
        //
        // Drivers send IRP_MN_POWER_SEQUENCE to themselves as an optimization to
        // determine whether their hardware actually entered a specific power state.
        // The power manager does not send this IRP. This IRP is optional to process.
        // The Toaster sample function driver does not demonstrate how to process
        // this IRP.
        //
        // Fall through down to the next switch-case statement.
        //

    default:
        //
        // Call ToasterDispatchPowerDefault to pass all unprocessed power IRPs (in
        // this case IRP_MN_WAIT_WAKE and IRP_MN_POWER_SEQUENCE) down the device
        // stack to be processed by the bus driver.
        //
        status = ToasterDispatchPowerDefault(DeviceObject, Irp);

        ToasterIoDecrement(fdoData);

        break;
    }

    return status;
}


 
NTSTATUS
ToasterDispatchPowerDefault(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    )

/*++

New Routine Description:
    ToasterDispatchPowerDefault sends the incoming power IRP down the device stack
    to be processed by the underlying bus driver.

    ToasterDispatchPower calls this routine to send down unprocessed
    IRP_MN_WAIT_WAKE and IRP_MN_POWER_SEQUENCE IRPs. ToasterFinalizeOnDeviceIrp
    calls this routine to send down IRP_MN_QUERY_POWER S-IRPs, IRP_MN_QUERY_POWER
    D-IRPs, IRP_MN_SET_POWER S-IRPs, and IRP_MN_SET_POWER D-IRPs.

    If a driver does not support a particular power IRP, such as
    IRP_MN_POWER_SEQUENCE, the driver must nevertheless pass the IRP down the
    device stack so that the bus driver can process it.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the power operation associated with the hardware instance
    described by the DeviceObject parameter. For example, the operation might
    specify a query to determine if the system can power down to conserve battery
    life, or to change the device power state to a higher level

Return Value Description:
    ToasterDispatchPowerDefault returns the NTSTATUS value returned by the next
    lower driver in the device stack. This value might equal STATUS_SUCCESS to
    indicate the power IRP was successfully processed, or this value might equal
    STATUS_UNSUCCESSFUL to indicate the power IRP was failed.

--*/
{
    NTSTATUS         status;
    PFDO_DATA        fdoData;

    PAGED_CODE();

    //
    // Notify the power manager to start the next power IRP. Power IRPs are processed
    // differently than non-power IRPs. A driver must call PoStartNextPowerIrp to
    // notify the power manager to send the next power IRP. PoStartNextPowerIrp must
    // be called even if the driver fails power IRPs.
    //
    // A driver must call PoStartNextPowerIrp while the power IRP’s current I/O stack
    // location points to the current driver. That is, PoStartNextPowerIrp must be
    // called before IoSkipCurrentIrpStackLocation, IoCopyIrpStackLocationToNext,
    // IoCompleteRequest, or PoCallDriver, otherwise a system deadlock may result.
    //
    PoStartNextPowerIrp(Irp);

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the PoCallDriver call). Call IoSkipCurrentIrpStackLocation
    // because the function driver does not change any of the IRP’s values in the
    // current I/O stack location. This allows the system to reuse I/O stack
    // locations.
    //
    IoSkipCurrentIrpStackLocation(Irp);

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Pass the power IRP down the device stack. Drivers must use PoCallDriver
    // instead of IoCallDriver to pass power IRPs down the device stack. PoCallDriver
    // ensures that power IRPs are properly synchronized throughout the
    // system.
    //
    status = PoCallDriver(fdoData->NextLowerDriver, Irp);

    return status;
}


 
NTSTATUS
ToasterDispatchSetPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    ToasterDispatchSetPowerState processes IRP_MN_SET_POWER S-IRPs sent originally
    from the system as well as IRP_MN_SET_POWER D-IRPs requested by the function
    driver in response to IRP_MN_SET_POWER S-IRPs. The incoming IRP’s
    Parameters.Power.Type I/O stack location member determines whether the
    incoming power IRP is a S-IRP or a D-IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the IRP_MN_SET_POWER operation to perform on the hardware
    instance described by the DeviceObject parameter. For example, if Irp is a
    IRP_MN_SET_POWER S-IRP, then the operation notifies the driver to create a
    corresponding IRP_MN_SET_POWER D-IRP to change the device power state of the
    hardware instance described by the DeviceObject parameter. If Irp is a
    IRP_MN_SET_POWER D-IRP, then the operation notifies the driver to change the
    power state of the hardware instance described by the DeviceObject parameter
    to a specific device power state.

Return Value Description:
    ToasterDispatchSetPowerState returns the value returned by either
    ToasterDispatchSystemPowerIrp, or the value returned by
    ToasterDispatchDeviceSetPower. The value should be STATUS_PENDING if the
    function driver does not fail the incoming Irp parameter.

--*/
{
    PIO_STACK_LOCATION stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchSetPowerState\n");

    //
    // Determine whether the incoming IRP_MN_SET_POWER Irp is a S-IRP or a D-IRP. If
    // the Irp’s Parameters.Power.Type I/O stack location member equals
    // SystemPowerState, then the incoming Irp is a S-IRP and the function driver
    // calls ToasterDispatchSystemPowerIrp to create a corresponding IRP_MN_SET_POWER
    // D-IRP. Otherwise, the incoming IRP_MN_SET_POWER Irp is a D-IRP (requested
    // earlier in response to an IRP_MN_SET_POWER S-IRP) and the function driver
    // calls ToasterDispatchDeviceSetPower to process the device power state
    // operation.
    //
    return (stack->Parameters.Power.Type == SystemPowerState) ?
        ToasterDispatchSystemPowerIrp(DeviceObject, Irp) :
        ToasterDispatchDeviceSetPower(DeviceObject, Irp);

}


 
NTSTATUS
ToasterDispatchQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    ToasterDispatchQueryPowerState processes IRP_MN_QUERY_POWER S-IRPs sent
    originally from the system as well as IRP_MN_QUERY_POWER D-IRPs requested by
    the function driver in response to IRP_MN_QUERY_POWER S-IRPs. The incoming
    IRP’s Parameters.Power.Type I/O stack location member determines whether the
    incoming power IRP is a S-IRP or a D-IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the IRP_MN_QUERY_POWER operation to perform on the hardware
    instance described by the DeviceObject parameter. For example, if Irp is a
    IRP_MN_QUERY_POWER S-IRP, then the operation notifies the driver to create a
    corresponding IRP_MN_QUERY_POWER D-IRP to determine if the hardware instance
    can change to an appropriate device power state without disrupting work. If
    Irp is a IRP_MN_QUERY_POWER D-IRP, then the operation determines if the
    hardware instance can change to an appropriate device power state without
    disrupting work.

Return Value Description:
    ToasterDispatchQueryPowerState returns the value returned by either
    ToasterDispatchSystemPowerIrp, or the value returned by either
    ToasterDispatchDeviceSetPower. The value should be STATUS_PENDING if the
    function driver does not fail the incoming Irp parameter.

--*/
{
    PIO_STACK_LOCATION stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchQueryPowerState\n");

    //
    // Determine whether the incoming IRP_MN_QUERY_POWER Irp is a S-IRP or a D-IRP.
    // If the Irp’s Parameters.Power.Type I/O stack location member equals
    // SystemPowerState, then the incoming Irp is a S-IRP and the function driver
    // calls ToasterDispatchSystemPowerIrp to create a corresponding
    // IRP_MN_QUERY_POWER D-IRP. Otherwise, the incoming IRP_MN_QUERY_POWER Irp is a
    // D-IRP (requested earlier in response to an IRP_MN_QUERY_POWER S-IRP) and the
    // function driver calls ToasterDispatchDeviceQueryPower to process the query
    // power state operation.
    //
    return (stack->Parameters.Power.Type == SystemPowerState) ?
        ToasterDispatchSystemPowerIrp(DeviceObject, Irp) :
        ToasterDispatchDeviceQueryPower(DeviceObject, Irp);

}


 
NTSTATUS
ToasterDispatchSystemPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    ToasterDispatchSystemPowerIrp processes IRP_MN_QUERY_POWER S-IRPs and
    IRP_MN_SET_POWER S-IRPs.

    IRP_MN_QUERY_POWER S-IRPs and IRP_MN_SET_POWER S-IRPs must be processed on
    their passage back up the device stack, after the underlying bus driver
    completes them.

    Therefore, ToasterDispatchSystemPowerIrp must set the system to call an I/O
    completion routine in order for the function driver to process the incoming
    Irp after the bus driver completes it.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the system power operation to perform. For example, if Irp is a
    IRP_MN_QUERY_POWER S-IRP, then the power manager is querying the function
    driver if its hardware instance can change to an appropriate device power
    state without disrupting work. If Irp is a IRP_MN_SET_POWER S-IRP then the
    power manager is notifying the function driver of a system power change and
    the function driver must change the hardware instance described by the
    DeviceObject parameter to an appropriate device power state.

    In both cases, the function driver must create a respective corresponding
    IRP_MN_QUERY_POWER D-IRP or IRP_MN_SET_POWER D-IRP to process the system power
    operation.

Return Value Description:
    ToasterDispatchSystemPowerIrp returns STATUS_PENDING because the incoming
    S-IRP is marked pending.

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    SYSTEM_POWER_STATE  newSystemState;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchSystemPowerIrp\n");

    newSystemState = stack->Parameters.Power.State.SystemState;

    if (IRP_MN_SET_POWER == stack->MinorFunction)
    {
        //
        // If the incoming Irp is a IRP_MN_SET_POWER S-IRP, then update the device
        // extension’s SystemPowerState member which represents the power state that
        // the function driver creates a corresponding IRP_MN_SET_POWER D-IRP for.
        //
        fdoData->SystemPowerState = newSystemState;

        ToasterDebugPrint(INFO, "\tSetting the system state to %s\n",
                    DbgSystemPowerString(fdoData->SystemPowerState));
    }

    //
    // Mark the incoming IRP_MN_SET_POWER or IRP_MN_QUERY_POWER S-IRP as pending. The
    // function driver continues to process the S-IRP when the system calls the
    // ToasterCompletionSystemPowerUp I/O completion routine. The system calls
    // ToasterCompletionSystemPowerUp after the underlying bus driver completes the
    // S-IRP.
    //
    IoMarkIrpPending(Irp);

    //
    // Set up the I/O stack location for the next lower driver (the target device
    // object for the PoCallDriver call). The function driver calls
    // IoCopyCurrentIrpStackLocationToNext to copy the parameters from the I/O stack
    // location for the function driver to the I/O stack location for the next lower
    // driver, so that the next lower driver uses the same parameters as the function
    // driver.
    //
    IoCopyCurrentIrpStackLocationToNext(Irp);

    //
    // Set the system to call ToasterCompletionSystemPowerUp after the underlying bus
    // driver completes the incoming S-IRP.
    //
    IoSetCompletionRoutine(
        Irp,
        ToasterCompletionSystemPowerUp,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

    //
    // Pass the incoming S-IRP down the device stack. Drivers must use PoCallDriver
    // instead of IoCallDriver to pass power IRPs down the device stack. PoCallDriver
    // ensures that power IRPs are properly synchronized throughout the
    // system.
    //
    PoCallDriver(fdoData->NextLowerDriver, Irp);

    return STATUS_PENDING;
}


 
NTSTATUS
ToasterCompletionSystemPowerUp(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
/*++

New Routine Description:
    ToasterCompletionSystemPowerUp is the function driver’s I/O completion routine
    for IRP_MN_SET_POWER S-IRPs and IRP_MN_QUERY_POWER S-IRPs. This routine
    continues to process the incoming S-IRP that was pended earlier in
    ToasterDispatchSystemPowerIrp.

    After the underlying bus driver completes the incoming S-IRP then the
    function driver requests a D-IRP that corresponds to the S-IRP, based on the
    power policy implemented by the driver.

    The Toaster sample function driver implements a simple power policy: A
    hardware instance enters the device power state D0 (working; fully powered and
    ready to process I/O operations) when the system enters the system power state
    S0 (working; fully powered and operational). The hardware instance enters the
    lowest-powered device state, D3 (sleeping), for all the other system power
    states (S1-S5). Refer to the DDK documentation for a more thorough explanation
    of the device D* and S* power states.

Parameters Description:
    Fdo
    Fdo represents the hardware instance that is associated with the incoming Irp
    parameter. Fdo is a FDO created earlier in ToasterAddDevice.

    Irp
    Irp describes the IRP_MN_SET_POWER or IRP_MN_QUERY_POWER S-IRP that has been
    completed by the underlying bus driver. The S-IRP now indicates if it was
    successfully processed by the bus driver in the device stack, or if it was
    failed.
    The function driver must now create a respective corresponding IRP_MN_SET_POWER
    or IRP_MN_QUERY_POWER D-IRP.

    NotUsed
    NotUsed represents an optional context parameter that the function driver
    specified to the system to pass to this routine. This parameter is not used.

Return Value Description:
    ToasterCompletionSystemPowerUp returns STATUS_CONTINUE_COMPLETION if the
    incoming S-IRP was failed by the bus driver. Otherwise,
    ToasterCompletionSystemPowerUp returns STATUS_MORE_PROCESSING_REQUIRED.
    STATUS_MORE_PROCESSING_REQUIRED indicates to the I/O manager that the function
    driver must process the IRP further before the I/O manager resumes calling any
    other I/O completion routines.

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA) Fdo->DeviceExtension;
    NTSTATUS    status = Irp->IoStatus.Status;

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionSystemPowerUp\n");

    if (!NT_SUCCESS(status))
    {
        //
        // If the underlying bus driver failed the S-IRP, then the S-IRP’s
        // IoStatus.Status member will indicate the failure and the NT_SUCCESS macro
        // fails. In this case, notify the power manager to start the next power IRP.
        //
        PoStartNextPowerIrp(Irp);

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        ToasterIoDecrement(fdoData);

        //
        // Return STATUS_CONTINUE_COMPLETION to the I/O manager to indicate that it
        // should continue to call other completion routines registered by other
        // drivers located above the function driver in the device stack.
        //
        return STATUS_CONTINUE_COMPLETION;
    }

    //
    // Queue a IRP_MN_QUERY_POWER D-IRP or IRP_MN_SET_POWER D-IRP that corresponds the
    // original IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP. The system sends
    // the corresponding D-IRP to the top of the hardware instance’s device stack.
    // The function driver eventually receives the corresponding D-IRP
    //
    ToasterQueueCorrespondingDeviceIrp(Irp, Fdo);

    //
    // Do not complete the original S-IRP here in the power completion routine. The
    // S-IRP is completed in the completion routine of the corresponding D-IRP.
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}



 
VOID
ToasterQueueCorrespondingDeviceIrp(
    IN PIRP SIrp,
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

New Routine Description:
    ToasterQueueCorrespondingDeviceIrp requests a IRP_MN_SET_POWER or
    IRP_MN_QUERY_POWER D-IRP that corresponds to the original incoming SIrp
    parameter. The system sends the corresponding D-IRP to the top of the hardware
    instance’s device stack. The D-IRP corresponds to the system power state that
    is specified in the original IRP_MN_SET_POWER or IRP_MN_QUERY_POWER S-IRP
    based on the power policy implemented by thefunction driver. 
    ToasterQueueCorrespondingDeviceIrp calls ToasterGetPowerPoliciesDeviceState
    to supply the power policy for the Toaster sample function driver.

Parameters Description:
    SIrp
    SIrp represents the original IRP_MN_QUERY_POWER or IRP_MN_SET_POWER S-IRP that
    indicates a specific system power state.
    The ToasterGetPowerPoliciesDeviceState determine the corresponding device
    power state. ToasterQueueCorrespondingDeviceIrp calls PoRequestPowerIrp to
    create a corresponding IRP_MN_SET_POWER or IRP_MN_QUERY_POWER D-IRP for the
    specific system power state. PoRequestPowerIrp sends the corresponding
    D-IRP to the top of the hardware instance’s device stack, where
    ToasterDispatchPower subsequently processes it.

    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming SIrp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

Return Value Description:
    This routine does not return a value.

--*/

{
    NTSTATUS            status;
    POWER_STATE         state;
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);

    ToasterDebugPrint(TRACE, "Entered ToasterQueueCorrespondingDeviceIrp\n");

    //
    // Get the device power state that corresponds to the system power state. The
    // ToasterGetPowerPoliciesDeviceState routine maps a system power state to a
    // device power state.
    //
    status = ToasterGetPowerPoliciesDeviceState(
        SIrp,
        DeviceObject,
        &state.DeviceState
        );

    if (NT_SUCCESS(status))
    {
        //
        // Test the assumption that the function driver is not already processing a
        // S-IRP. The function driver cannot process more than one S-IRP at a time.
        //
        ASSERT(NULL == fdoData->PendingSIrp);

        //
        // Save the incoming S-IRP in the device extension. When the PendingSIrp
        // member does not equal NULL, then that indicates the function driver is
        // currently processing a IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP.
        //
        // The ToasterCompletionOnFinalizedDeviceIrp routine clears this member to
        // NULL after the function driver completes the corresponding
        // IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP.
        //
        // In addition, the ToasterDispatchDeviceSetPower routine clears this member
        // to NULL when the function driver processes a S0 power-up S-IRP.
        //
        fdoData->PendingSIrp = SIrp;
        
        //
        // Allocate and initialize a D-IRP that corresponds to the incoming
        // IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP.
        //
        // PoRequestPowerIrp allocates a power IRP and sends it to the top of the
        // device stack that contains the device object specified in the first
        // parameter. On Windows 2000, PoRequestPowerIrp can accept a FDO, but
        // Windows 9x/Me requires a PDO.
        //
        // Pass the incoming S-IRP’s minor function code (IRP_MN_SET_POWER, or
        // IRP_MN_QUERY_POWER) as the corresponding D-IRP’s minor function code.
        //
        // Set the system to call back the power completion routine,
        // ToasterCompletionOnFinalizedDeviceIrp when the underlying bus driver
        // completes the corresponding D-IRP.
        //
        status = PoRequestPowerIrp(
            fdoData->UnderlyingPDO,
            stack->MinorFunction,
            state,
            ToasterCompletionOnFinalizedDeviceIrp,
            fdoData,
            NULL
            );
    }

    if (!NT_SUCCESS(status))
    {
        //
        // If either ToasterGetPowerPoliciesDeviceState or PoRequestPowerIrp fail,
        // then the function driver must still notify the power manager to start the
        // next power IRP before failing the incoming S-IRP.
        //
        // A driver must start the next power IRP before it completes the current one,
        // otherwise a system deadlock can result.
        //
        PoStartNextPowerIrp(SIrp);

        //
        // Copy the failure code returned by ToasterGetPowerPoliciesDeviceState or
        // PoRequestPowerIrp into the original S-IRPs IoStatus.Status member. This
        // member indicates to the system the reason for the failure.
        //
        SIrp->IoStatus.Status = status;

        IoCompleteRequest(SIrp, IO_NO_INCREMENT);

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        ToasterIoDecrement(fdoData);
    }
}


 
VOID
ToasterCompletionOnFinalizedDeviceIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID    PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    )
/*++

New Routine Description:
    ToasterCompletionOnFinalizedDeviceIrp is the function driver’s power
    completion routine for corresponding IRP_MN_SET_POWER and IRP_MN_QUERY_POWER
    D-IRPs that the function driver requested earlier when
    ToasterQueueCorrespondingDeviceIrp called PoRequestPowerIrp.

    The system calls this routine when the corresponding D-IRP has been completed
    and all I/O completion routines, if any, have returned.

    The DDK PoRequestPowerIrp topic defines the function prototype that
    ToasterCompletionOnFinalizedDeviceIrp must conform to.

Parameters Description:
    DeviceObject
    DeviceObject represents the target device object for the completed power
    D-IRP. This parameter is not used.

    MinorFunction
    MinorFunction represents the minor function code for the D-IRP. This parameter
    is not used.

    PowerState
    PowerState represents the new device power state.
    ToasterQueueCorrespondingDeviceIrp set the system to pass the new device power
    state in the call to PoRequestPowerIrp. This parameter is not used.

    PowerContext
    PowerContext represents the device extension for the hardware instance.
    ToasterQueueCorrespondingDeviceIrp set the system to pass the device extension
    in the call to PoRequestPowerIrp.

    IoStatus
    IoStatus represents the I/O status block structure for the completed D-IRP.

Return Value Description:
    This routine does not return a value.

--*/
{
    //
    // Get the pointer to the FDO’s device extension. The device extension is used to
    // store any information that must not be paged out. This includes hardware state
    // information, storage for mechanisms implemented by the function driver, and
    // pointers to the underlying PDO and the next lower driver.
    //
    // The FDO’s device extension is defined as a PVOID. The PVOID must be recast to
    // a pointer to the data type of the device extension, PFDO_DATA.
    //
    PFDO_DATA   fdoData = (PFDO_DATA) PowerContext;

    //
    // Get the pointer to the original pending S-IRP. The S-IRP can now be completed
    // with the corresponding D-IRP’s status.
    //
    PIRP        sIrp = fdoData->PendingSIrp;

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionOnFinalizedDeviceIrp\n");

    if(NULL != sIrp)
    {
        //
        // Copy the status of the underlying bus driver’s processing of the
        // corresponding IRP_MN_QUERY_POWER D-IRP or IRP_MN_SET_POWER D-IRP into the
        // original pending IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP.
        //
        sIrp->IoStatus.Status = IoStatus->Status;

        //
        // Notify the power manager to start the next power IRP before completing the
        // S-IRP.
        //
        // A driver must start the next power IRP before it completes the current
        // power IRP, otherwise a system deadlock can result.
        //
        PoStartNextPowerIrp(sIrp);

        IoCompleteRequest(sIrp, IO_NO_INCREMENT);

        //
        // Set the device extension’s PendingSIrp to NULL because the pending S-IRP
        // has been completed.
        //
        fdoData->PendingSIrp = NULL;

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        ToasterIoDecrement(fdoData);
    }
}


 
NTSTATUS
ToasterDispatchDeviceQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

New Routine Description:
    ToasterDispatchDeviceQueryPower processes corresponding IRP_MN_QUERY_POWER
    D-IRPs. The function driver previously requested a corresponding
    IRP_MN_QUERY_POWER D-IRP in ToasterQueueCorrespondingDeviceIrp, in response
    to an original IRP_MN_QUERY_POWER S-IRP.

    ToasterDispatchDeviceQueryPower determines if the hardware instance can change
    its device power state to the specified power level without disrupting work.
    If the hardware instance is currently processing a request then its power
    level should not be reduced and the corresponding IRP_MN_QUERY_POWER D-IRP
    should be failed, thus failing the original IRP_MN_QUERY_POWER S-IRP.

    If the function driver succeeds the corresponding IRP_MN_QUERY_POWER D-IRP,
    then it must prepare to begin queuing any data I/O requests that are
    dispatched to it after it has succeeded the D-IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the specific device power state that is appropriate for the
    original pending IRP_MN_QUERY_POWER S-IRP based on the power policy
    implemented by the function driver.

Return Value Description:
    ToasterDispatchDeviceQueryPower returns STATUS_SUCCESS if the device power
    state is for D0. ToasterDispatchDeviceQueryPower returns STATUS_PENDING if
    it successfully queues a work item for the system worker thread to callback
    at IRQL = PASSIVE_LEVEL. Otherwise ToasterDispatchDeviceQueryPower returns
    an error status that indicates the reason a work item could not be queued.

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get the specific device power state requested earlier by the function driver
    // when it mapped the original IRP_MN_QUERY_POWER S-IRP’s system power state to
    // an appropriate device power state.
    //
    DEVICE_POWER_STATE  deviceState = stack->Parameters.Power.State.DeviceState;
    NTSTATUS            status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchDeviceQueryPower\n");

    if (PowerDeviceD0 == deviceState)
    {
        //
        // The function driver does not change the driver-managed IRP queue state
        // to HoldRequests if the system power state to device power state mapping
        // indicates that the hardware instance remains fully powered (D0) during
        // system standby.
        //
        // This could be a problem for certain classes of devices, such as audio
        // adapters. For example, if the system power state to device power state
        // mapping implemented by the power policy
        // (ToasterGetPowerPoliciesDeviceState) indicates that S1 maps to D0 then
        // the audio adapter might remain on while the computer is in stand-by. That
        // could be noisy to the user.
        //
        // Unfortunately, a driver cannot use the ShutdownType member for
        // corresponding D0 IRP_MN_QUERY_POWER D-IRPs to identify such circumstances
        // because the member cannot be trusted for D0 D-IRPs. Instead, the Toaster
        // sample function driver identifies this circumstance by maintaining a
        // pointer to the current pending IRP_MN_QUERY_POWER S-IRP in the device
        // extension. However, when a driver uses this method, it must be very
        // careful if it also performs idle detection. The Toaster sample function
        // driver does not demonstrate how to implement idle detection.
        //
        status = STATUS_SUCCESS;
    }
    else
    {
        //
        // If the corresponding IRP_MN_QUERY_POWER D-IRP’s requested device power
        // state is deeper than D0, then the function driver must wait until every
        // pending IRP (if any) such as read, write, or device control operations
        // completes (in other threads of execution). However, the driver cannot
        // wait in the thread that is processing the IRP_MN_QUERY_POWER D-IRP
        // because that might cause a system deadlock.
        //
        // Instead, the function driver queues a separate work item to be processed
        // later by the system worker thread which executes a callback routine at
        // IRQL = PASSIVE_LEVEL. The queued work item is not related to the
        // driver-managed IRP queue. The queued work item uses a different mechanism
        // which is implemented by the system to process driver callback routines at
        // IRQL = PASSIVE_LEVEL.
        //
        // If the worker thread successfully processes the queued IRP_MN_QUERY_POWER
        // D-IRP, then the driver indicates that the hardware instance can change to
        // the specified device power state without disrupting work. The callback
        // routine fails the D-IRP if the hardware instance is busy processing
        // requests and thus cannot change to the specified device power state.
        //
        // Specify that the worker thread call back
        // ToasterCallbackHandleDeviceQueryPower to process the corresponding
        // IRP_MN_QUERY_POWER D-IRP when the system schedules the worker thread to
        // process the queued work item.
        //
        status = ToasterQueuePassiveLevelPowerCallback(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            ToasterCallbackHandleDeviceQueryPower
            );

        //
        // If ToasterQueuePassiveLevelPowerCallback successfully queued the work
        // item, then it returns STATUS_PENDING, and the system will finish
        // processing the IRP_MN_QUERY_POWER D-IRP at a later time when the system
        // worker thread calls the work item’s callback routine. Otherwise, if
        // ToasterQueuePassiveLevelPowerCallback is unable to allocate the resources
        // to queue the work item, then it returns STATUS_INSUFFICIENT_RESOURCES, and
        // the IRP_MN_QUERY_POWER D-IRP will be finalized below.
        //
        if (STATUS_PENDING == status)
        {
            return status;
        }
    }

    //
    // Finish the IRP_MN_QUERY_POWER D-IRP. The IRP_NEEDS_FORWARDING parameter
    // indicates that the D-IRP has not yet been passed down the device stack, so
    // ToasterFinalizeDevicePowerIrp will pass it down. Pass the status of the D-IRP
    // to ToasterFinalizeDevicePowerIrp. The status passed is either STATUS_SUCCESS,
    // if the hardware instance is in device power state D0, or
    // STATUS_INSUFFICIENT_RESOURCES if ToasterQueuePassiveLevelPowerCallback was
    // unable to allocate and queue a work item to be processed later by the system
    // worker thread.
    //
    return ToasterFinalizeDevicePowerIrp(
        DeviceObject,
        Irp,
        IRP_NEEDS_FORWARDING,
        status
        );
}


 
NTSTATUS
ToasterDispatchDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

New Routine Description:
    ToasterDispatchDeviceSetPower processes corresponding IRP_MN_SET_POWER D-IRPs.
    The function driver previously requested a corresponding IRP_MN_SET_POWER
    D-IRP in ToasterQueueCorrespondingDeviceIrp, in response to an original
    IRP_MN_SET_POWER S-IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the specific device power state that is appropriate for the
    original pending IRP_MN_SET_POWER S-IRP based on the power policy
    implemented by the function driver.

Return Value Description:
    ToasterDispatchDeviceSetPower returns STATUS_PENDING if it successfully
    queues a work item for the system worker thread to callback at
    IRQL = PASSIVE_LEVEL. Otherwise ToasterDispatchDeviceSetPower returns
    an error status that indicates the reason a work item could not be queued.

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    POWER_STATE         state = stack->Parameters.Power.State;
    NTSTATUS            status;
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  sIrpstack;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchDeviceSetPower\n");

    if(fdoData->PendingSIrp)
    {
        sIrpstack = IoGetCurrentIrpStackLocation(fdoData->PendingSIrp);
        
        if(IRP_MN_SET_POWER == sIrpstack->MinorFunction &&
           PowerSystemWorking == stack->Parameters.Power.State.SystemState)
        {
            //
            // On Windows 2000 and later, the Toaster sample function driver
            // completes S0 IRP_MN_SET_POWER S-IRPs before it begins to process D0
            // IRP_MN_SET_POWER D-IRPs. This enables the user to resume using the
            // computer sooner from a suspended power state. Otherwise, the user
            // may experience a slower resume from suspend than is necessary because
            // the system waits until all S0 IRP_MN_SET_POWER S-IRPs have been
            // completed before resuming any user-mode activity. Because powering-up
            // a device and restoring its context takes time, it is not necessary to
            // hold S0 IRP_MN_SET_POWER S-IRPs for non-critical devices.
            //
            ToasterDebugPrint(TRACE, "\t ****Completing S0 IRP\n");

            fdoData->PendingSIrp->IoStatus.Status = STATUS_SUCCESS;

            PoStartNextPowerIrp(fdoData->PendingSIrp);

            IoCompleteRequest(fdoData->PendingSIrp, IO_NO_INCREMENT);

            fdoData->PendingSIrp = NULL;

            ToasterIoDecrement(fdoData);            
        } 
    } 
    
    if (state.DeviceState < fdoData->DevicePowerState)
    {
        //
        // The system is increasing the power level to the device. The bus driver
        // must process the incoming power-up D-IRP before the function driver can,
        // otherwise the hardware instance may not even be powered on.
        //

        //
        // Mark the incoming IRP_MN_SET_POWER D-IRP as pending. The function driver
        // continues to process the D-IRP when the system calls the
        // ToasterCompletionDevicePowerUp I/O completion routine. The system calls
        // ToasterCompletionDevicePowerUp after the underlying bus driver completes
        // the S-IRP.
        //
        IoMarkIrpPending(Irp);

        //
        // Set up the I/O stack location for the next lower driver (the target
        // device object for the PoCallDriver call). The function driver calls
        // IoCopyCurrentIrpStackLocationToNext to copy the parameters from the I/O
        // stack location for the function driver to the I/O stack location for the
        // next lower driver, so that the next lower driver uses the same parameters
        // as the function driver.
        //
        IoCopyCurrentIrpStackLocationToNext(Irp);

        //
        // Set the system to call ToasterCompletionDevicePowerUp after the underlying
        // bus driver completes the incoming IRP_MN_SET_POWER D-IRP.
        //
        IoSetCompletionRoutine(
            Irp,
            ToasterCompletionDevicePowerUp,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );

        //
        // Pass the incoming S-IRP down the device stack. Drivers must use
        // PoCallDriver instead of IoCallDriver to pass power IRPs down the device
        // stack. PoCallDriver ensures that power IRPs are properly synchronized
        // throughout the system.
        //
        PoCallDriver(fdoData->NextLowerDriver, Irp);

        return STATUS_PENDING;
    }
    else
    {
        //
        // The system is decreasing the power level to the hardware instance, or is
        // entering a power state the hardware instance is already in.
        //
        // While D0 IRP_MN_SET_POWER D-IRPs are alike, non-D0 IRP_MN_SET_POWER D-IRPs
        // are not alike because some might be for suspend, stand-by, hibernate, or
        // shutdown. Although it is not necessary to manipulate the hardware instance
        // to process a D0 D-IRP (when the hardware instance is already in D0) each
        // non-D0 D-IRP must be processed by the function driver in case the function
        // driver succeeded an earlier IRP_MN_QUERY_POWER D-IRP (and thus began to
        // queue new incoming data I/O IRPs instead of processing them immediately)
        // and the system subsequently sent a S0 IRP_MN_SET_POWER S-IRP, which
        // effectively cancels the earlier IRP_MN_QUERY_POWER D-IRP.
        //

        //
        // The function driver must wait until every pending IRP (if any) such as
        // read, write, or device control operations completes (in other threads of
        // execution). However, the function driver cannot wait in the thread that is
        // processing the power-down D-IRP because that might cause a system
        // deadlock.
        //
        // Instead, the function driver queues a separate work item to be processed
        // later by the system worker thread which executes a callback routine at
        // IRQL = PASSIVE_LEVEL. The queued work item is not related to the
        // driver-managed IRP queue. The queued work item uses a different mechanism
        // which is implemented by the system to process driver callback routines at
        // IRQL = PASSIVE_LEVEL.
        //
        // The callback routine in the work item waits until the function driver
        // completes every pending IRP (in other threads of execution) and then
        // processes the power-down D-IRP.
        //
        // Specify that the worker thread call back
        // ToasterCallbackHandleDeviceSetPower to process the corresponding
        // power-down D-IRP when the system schedules the worker thread to
        // process the queued work item.
        //
        status = ToasterQueuePassiveLevelPowerCallback(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            ToasterCallbackHandleDeviceSetPower
            );

        //
        // If ToasterQueuePassiveLevelPowerCallback successfully queued the work
        // item, then it returns STATUS_PENDING, and the system will finish
        // processing the IRP_MN_QUERY_POWER D-IRP at a later time when the system
        // worker thread calls the work item’s callback routine. Otherwise, if
        // ToasterQueuePassiveLevelPowerCallback is unable to allocate the resources
        // to queue the work item, then it returns STATUS_INSUFFICIENT_RESOURCES, and
        // the IRP_MN_QUERY_POWER D-IRP will be finalized below.
        //
        if (STATUS_PENDING == status)
        {
            return status;
        }

        //
        // Test the assumption that IRP_MN_SET_POWER has not been failed. Drivers must
        // not fail IRP_MN_SET_POWER.
        //
        ASSERT(!NT_SUCCESS(status));

        //
        // Finish the power-down D-IRP. The IRP_NEEDS_FORWARDING parameter indicates
        // that the D-IRP has not yet been passed down the device stack, so
        // ToasterFinalizeDevicePowerIrp will pass it down. Pass the status of the
        // D-IRP to ToasterFinalizeDevicePowerIrp. The status passed is
        // STATUS_INSUFFICIENT_RESOURCES because
        // ToasterQueuePassiveLevelPowerCallback was unable to allocate and queue a
        // work item to be processed later by the system worker thread.
        //
        return ToasterFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            status
            );
    }
}


 
NTSTATUS
ToasterCompletionDevicePowerUp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID NotUsed
    )
/*++

New Routine Description:
    ToasterCompletionDevicePowerUp is the function driver’s I/O completion routine
    for power-up D-IRP. This routine continues to process the incoming power-up
    D-IRP that was pended earlier in ToasterDispatchDeviceSetPower.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the power-up operation that has been completed by the
    underlying bus driver.

    NotUsed
    NotUsed represents an optional context parameter that the function driver
    specified to the system to pass to this routine. This parameter is not used.

Return Value Description:
    ToasterCompletionDevicePowerUp returns STATUS_CONTINUE_COMPLETION if the
    incoming D-IRP was failed by the bus driver. Otherwise,
    ToasterCompletionDevicePowerUp returns STATUS_MORE_PROCESSING_REQUIRED.
    STATUS_MORE_PROCESSING_REQUIRED indicates to the I/O manager that the function
    driver must process the IRP further before the I/O manager resumes calling any
    other I/O completion routines.

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    NTSTATUS            status = Irp->IoStatus.Status;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionDevicePowerUp\n");

    if (!NT_SUCCESS(status))
    {
        //
        // If the underlying bus driver failed the D-IRP, then the D-IRP’s
        // IoStatus.Status member will indicate the failure and the NT_SUCCESS macro
        // fails. In this case, notify the power manager to start the next power IRP.
        //
        // Because the IRP has been failed, notify the power manager to start the
        // next power IRP.
        //
        PoStartNextPowerIrp(Irp);

        //
        // Decrement the count of how many IRPs remain uncompleted. This call to
        // ToasterIoDecrement balances the earlier call to ToasterIoIncrement. An
        // equal number of calls to ToasterIoincrement and ToasterIoDecrement is
        // essential for the StopEvent and RemoveEvent kernel events to be signaled
        // correctly.
        //
        ToasterIoDecrement(fdoData);

        //
        // Return STATUS_CONTINUE_COMPLETION to the I/O manager to indicate that it
        // should continue to call other completion routines registered by other
        // drivers located above the function driver in the device stack.
        //
        return STATUS_CONTINUE_COMPLETION;
    }

    //
    // Test the assumption that incoming IRP_MN_SET_POWER D-IRP is really a set power
    // IRP.
    //
    ASSERT(IRP_MJ_POWER == stack->MajorFunction);
    ASSERT(IRP_MN_SET_POWER == stack->MinorFunction);

    //
    // The function driver must wait until every pending IRP (if any) such as read,
    // write, or device control operations completes (in other threads of execution).
    // However, the function driver cannot wait in the thread that is processing the
    // power-up D-IRP because that might cause a system deadlock.
    //
    // Instead, the function driver queues a separate work item to be processed
    // later by the system worker thread which executes a callback routine at
    // IRQL = PASSIVE_LEVEL. The queued work item uses a different mechanism which is
    // implemented by the system to process driver callback routines at
    // IRQL = PASSIVE_LEVEL.
    //
    // The callback routine in the work item waits until the function driver
    // completes every pending IRP (in other threads of execution) and then processes
    // the power-up D-IRP.
    //
    // Specify that the worker thread call back ToasterCallbackHandleDeviceSetPower
    // to process the corresponding power-up D-IRP when the system schedules the
    // worker thread to process the queued work item.
    //
    status = ToasterQueuePassiveLevelPowerCallback(
        DeviceObject,
        Irp,
        IRP_ALREADY_FORWARDED,
        ToasterCallbackHandleDeviceSetPower
        );

    if (STATUS_PENDING != status)
    {
        //
        // If ToasterQueuePassiveLevelPowerCallback did not successfully queue a
        // separate work item, then it does not return STATUS_PENDING, and the
        // power-up D-IRP will be finalized below.
        //

        //
        // Test the assumption that ToasterQueuePassiveLevelPowerCallback returned
        // a failure.
        //
        ASSERT(!NT_SUCCESS(status));

        //
        // Finish the power-up D-IRP. The IRP_ALREADY_FORWARDED parameter indicates
        // that the D-IRP has been passed down the device stack, so
        // ToasterFinalizeDevicePowerIrp does not need to send it down. Pass the
        // status of the D-IRP to ToasterFinalizeDevicePowerIrp. The status passed is
        // STATUS_INSUFFICIENT_RESOURCES because
        // ToasterQueuePassiveLevelPowerCallback was unable to allocate and queue a
        // work item to be processed later by the system worker thread.
        //
        ToasterFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            IRP_ALREADY_FORWARDED,
            status
            );
    }

    //
    // Do not complete the power-up D-IRP here in the I/O completion routine.
    //
    return STATUS_MORE_PROCESSING_REQUIRED;
}


 
NTSTATUS
ToasterFinalizeDevicePowerIrp(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction,
    IN  NTSTATUS            Result
    )
/*++

New Routine Description:
    ToasterFinalizeDevicePowerIrp is the final step when processing all D-IRPs.

    If the incoming IRP has not already been passed down the device stack to be
    processed by the underlying bus driver then ToasterFinalizeDevicePowerIrp
    passes the incoming IRP down the device stack.

    Otherwise, if the underlying bus driver has already completed the incoming
    IRP then ToasterFinalizeDevicePowerIrp notifies the power manager to start
    the next power IRP.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the power operation performed on the hardware instance
    represented by the DeviceObject parameter. 

    Direction
    Direction indicates whether or not to pass the D-IRP down the device stack. If
    the hardware instance is powering up then Direction = IRP_ALREADY_FORWARDED
    because the underlying bus driver has already processed the power-up D-IRP and
    it is being passed back up the device stack. Otherwise,
    Direction = IRP_NEEDS_FORWARDING because the D-IRP has not yet been processed
    by the underlying bus driver and must still passed down the device stack to be
    processed by the underlying bus driver.

    Result
    Result represents how ToasterFinalizeDevicePowerIrp should indicate the function
    driver processed the D-IRP. For example, STATUS_SUCCESS or some error code which
    specifies the reason the D-IRP was failed.

Return Value Description:
    ToasterFinalizeDevicePowerIrp returns the incoming Result parameter if the
    incoming IRP has already been passed down the device stack. Otherwise,
    ToasterFinalizeDevicePowerIrp returns the result of the
    ToasterDispatchPowerDefault call.

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Entered ToasterFinalizeDevicePowerIrp\n");

    if (IRP_ALREADY_FORWARDED == Direction || (!NT_SUCCESS(Result)))
    {
        //
        // If the incoming IRP_MN_QUERY_POWER or IRP_MN_SET_POWER D-IRP has already
        // been passed down the device stack, or the function driver has failed the
        // incoming IRP_MN_QUERY_POWER or IRP_MN_SET_POWER D-IRP, then notify the
        // power manager to start the next power IRP and copy the incoming result
        // (failure) code into the D-IRP’s IoStatus.Status block before completing
        // the incoming IRP.
        //
        PoStartNextPowerIrp(Irp);

        Irp->IoStatus.Status = Result;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return Result;
    }

    //
    // Copy the incoming result code into the IRP’s IoStatus.Status block before
    // calling ToasterDispatchPowerDefault to notify the power manager to start
    // the next power IRP.
    //
    Irp->IoStatus.Status = Result;

    status = ToasterDispatchPowerDefault(DeviceObject, Irp);

    ToasterIoDecrement(fdoData);

    return status;
}


 
NTSTATUS
ToasterQueuePassiveLevelPowerCallback(
    IN  PDEVICE_OBJECT                      DeviceObject,
    IN  PIRP                                Irp,
    IN  IRP_DIRECTION                       Direction,
    IN  PFN_QUEUE_SYNCHRONIZED_CALLBACK     Callback
    )
/*++

New Routine Description:
    ToasterQueuePassiveLevelPowerCallback queues a work item to be processed later
    by the system worker thread at IRQL = PASSIVE_LEVEL. The system thread calls
    the work item’s callback routine.

    The Toaster sample function driver must wait for every pending IRP (if any)
    such as read, write, or device control operations to complete (in other
    threads of execution) before changing the hardware instance’s device power
    state. However, the function driver cannot wait in the thread processing the
    the incoming D-IRP because that might cause a system deadlock. Instead, the
    incoming D-IRP is marked pending and a work item is queued for the system
    worker thread to process the D-IRP at IRQL = PASSIVE_LEVEL. The work item’s
    callback routine can then wait until every pending IRP completes by suspending
    execution of the system worker thread (which will not cause a system deadlock
    because the worker thread calls the callback routine at IRQL = PASSIVE_LEVEL).

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the D-IRP that cannot be completed until every pending IRP
    completes

    Direction
    Direction indicates whether or not to pass the D-IRP down the device stack. If
    the hardware instance is powering up then Direction = IRP_ALREADY_FORWARDED
    because the underlying bus driver has already processed the power-up D-IRP and
    it is being passed back up the device stack. Otherwise,
    Direction = IRP_NEEDS_FORWARDING because the D-IRP has not yet been processed
    by the underlying bus driver and must still passed down the device stack to be
    processed by the underlying bus driver.

    Callback
    Callback specifies the routine for the worker thread to callback at
    IRQL = PASSIVE_LEVEL.

Return Value Description:
    ToasterQueuePassiveLevelPowerCallback returns STATUS_INSUFFICIENT_RESOURCES if
    the work item cannot be allocated. Otherwise,
    ToasterQueuePassiveLevelPowerCallback returns STATUS_PENDING because it marks
    the incoming D-IRP as pending.

--*/
{
    PIO_WORKITEM            item;
    PWORKER_THREAD_CONTEXT  context;
    PFDO_DATA               fdoData;

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Allocate memory for worker thread context. The PWORKER_THREAD_CONTEXT data
    // type is defined in Power.h. ToasterQueuePassiveLevelPowerCallbackWorker later
    // releases this memory after the system worker thread processes the work item.
    //
    context = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(WORKER_THREAD_CONTEXT),
        TOASTER_POOL_TAG
        );

    if (NULL == context)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Windows 98 does not support IoAllocateWorkItem, it only supports
    // ExInitializeWorkItem and ExQueueWorkItem. The Toaster sample function driver
    // uses IoAllocateWorkItem because ExInitializeWorkItem/ExQueueWorkItem can cause
    // the system to crash when the driver is unloaded. For example, the system can
    // unload the driver in the middle of an ExQueueWorkItem callback.
    //
    // After ToasterQueuePassiveLevelPowerCallbackWorker processes the work item it
    // then frees the work item.
    //
    item = IoAllocateWorkItem(DeviceObject);

    if (NULL == item)
    {
        ExFreePool(context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Specify the data to for the work item, such as the D-IRP, whether to pass the
    // D-IRP down the device stack and the callback routine.
    //
    context->Irp = Irp;
    context->DeviceObject= DeviceObject;
    context->IrpDirection = Direction;
    context->Callback = Callback;
    context->WorkItem = item;

    //
    // Mark the incoming D-IRP as pending. The function driver continues to process
    // the D-IRP in the routine specified in the Callback parameter.
    //
    IoMarkIrpPending(Irp);

    //
    // Queue the initialized work item for the system worker thread. When the system
    // worker thread later processes the work item, it calls
    // ToasterQueuePassiveLevelPowerCallbackWorker and passes it the information in
    // the context variable.
    //
    IoQueueWorkItem(
        item,
        ToasterQueuePassiveLevelPowerCallbackWorker,
        DelayedWorkQueue,
        context
        );

    return STATUS_PENDING;
}


 
VOID
ToasterCallbackHandleDeviceQueryPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction
    )
/*++

New Routine Description:
    ToasterCallbackHandleDeviceQueryPower processes corresponding
    IRP_MN_QUERY_POWER D-IRPs that the function driver created in response to
    IRP_MN_QUERY_POWER S-IRPs when ToasterQueueCorrespondingDeviceIrp calls
    PoRequestPowerIrp.
    ToasterQueuePassiveLevelPowerCallbackWorker calls this routine to process the
    IRP_MN_QUERY_POWER D-IRP at IRQL = PASSIVE_LEVEL because the system worker
    thread can be suspended without causing a system deadlock until every pending
    IRP completes.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp describes the corresponding IRP_MN_QUERY_POWER D-IRP operation to perform
    on the hardware instance represented by the DeviceObject parameter.

    Direction
    Direction indicates whether or not to pass the D-IRP down the device stack. If
    the hardware instance is powering up then Direction = IRP_ALREADY_FORWARDED
    because the underlying bus driver has already processed the power-up D-IRP and
    it is being passed back up the device stack. Otherwise,
    Direction = IRP_NEEDS_FORWARDING because the D-IRP has not yet been processed
    by the underlying bus driver and must still passed down the device stack to be
    processed by the underlying bus driver.

Return Value Description:
    This routine does not return a value.

--*/
{
    DEVICE_POWER_STATE deviceState;
    PIO_STACK_LOCATION stack;
    NTSTATUS status;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get the device power state of the corresponding IRP_MN_QUERY_POWER D-IRP.
    //
    deviceState = stack->Parameters.Power.State.DeviceState;

    //
    // Test the assumption that the power manager does not send a query to determine
    // if the hardware instance can change to the D0 device power state.
    //
    ASSERT(PowerDeviceD0 != deviceState);

    //
    // Change the driver managed IRP queue mechanism to begin queuing any new
    // incoming data I/O IRPs in preparation to change the hardware instance’s device
    // power state.
    //
    // Pass 2 as the IoIrpCharges parameter. One is for the original pending
    // IRP_MN_QUERY_POWER S-IRP, and the other is for the corresponding
    // IRP_MN_QUERY_POWER D-IRP.
    //
    // Pass TRUE as the Query parameter to indicate that
    // ToasterPowerBeginQueueingIrps should check to determine if the hardware
    // instance can enter a suspended power state.
    //
    status = ToasterPowerBeginQueuingIrps(
        DeviceObject,
        2,
        TRUE
        );

    //
    // Finish the IRP_MN_QUERY_POWER D-IRP. When the ToasterDispatchDeviceQueryPower
    // routine queued a work item to call this routine, it passed
    // IRP_NEEDS_FORWARDING for the incoming Direction parameter to indicate that the
    // incoming D-IRP has not yet been passed down the device stack. Thus
    // ToasterFinalizeDevicePowerIrp must pass it down the device stack to be
    // processed by the underlying bus driver. Pass the status value returned by
    // ToasterPowerBeginQueueingIrps which indicates if the device power state of the
    // hardware instance can be reduced.
    //
    ToasterFinalizeDevicePowerIrp(DeviceObject, Irp, Direction, status);
}


 
VOID
ToasterQueuePassiveLevelPowerCallbackWorker(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    )
/*++

New Routine Description:
    ToasterQueuePassiveLevelPowerCallbackWorker is the routine that the system
    worker thread calls to process the work item queued earlier in
    ToasterQueuePassiveLevelPowerCallback.
    ToasterQueuePassiveLevelPowerCallbackWorker processes the work item described
    in the Context parameter. ToasterQueuePassiveLevelPowerCallback initialized
    the members of the work item with the parameters it received when it was
    called by ToasterDispatchDeviceQueryPower, ToasterDispatchDeviceSetPower, or
    ToasterCompletionDevicePowerUp.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Context parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Context
    Context describes the work item initialized earlier when
    ToasterQueuePassiveLevelPowerCallback was called to process a power IRP at
    IRQL = PASSIVE_LEVEL. Context contains the power IRP to be processed, and also
    indicates whether the power IRP has already been passed down the device stack
    (IRP_ALREADY_FORWARDED) or it still needs to be sent down the device stack
    (IRP_NEEDS_FORWARDING).

Return Value Description:
    This routine does not return a value.

--*/
{
    PWORKER_THREAD_CONTEXT  context;

    PAGED_CODE();

    //
    // Get the pointer to the work item’s worker thread context. The context is used
    // to store the information required to perform the work item, such as the
    // hardware instance (DeviceObject), IRP, and whether or not the IRP has already
    // been sent down the device stack.
    //
    // The context parameter is defined as a PVOID. The PVOID must be recast to
    // a pointer to the data type of the context, PWORKER_THREAD_CONTEXT.
    //
    context = (PWORKER_THREAD_CONTEXT) Context;

    //
    // Call the callback routine (ToasterQueuePassiveLevelPowerCallbackWorker) that
    // was specified earlier when ToasterQueuePassiveLevelPowerCallback queued the
    // work item.
    //
    context->Callback(
        context->DeviceObject,
        context->Irp,
        context->IrpDirection
        );

    //
    // Free the work item allocated earlier in ToasterQueuePassiveLevelPowerCallback.
    //
    IoFreeWorkItem(context->WorkItem);

    //
    // Release the memory allocated earlier in ToasterQueuePassiveLevelPowerCallback
    // to store the work item.
    //
    ExFreePool((PVOID)context);
}


 
NTSTATUS
ToasterPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    )
/*++

New Routine Description:
    ToasterPowerBeginQueuingIrps changes the driver-managed IRP queuing mechanism
    to begin queuing any new incoming data I/O IRPs and any existing queued
    requests to be completed.


Parameters Description:
    DeviceObject
    DeviceObject represents an instance of a toaster whose device extension is
    required to manipulate the driver-managed IRP queue mechanism.

    IrpIoCharges
    IrpIoCharges represents the number ToasterIoIncrement calls made as part of
    processing a power IRP that must be decremented for the StopEvent kernel
    event to be signaled.

    Query
    Query represents a boolean that indicates if the function driver should query
    the hardware instance to determine whether or not to begin queuing any new
    incoming data I/O IRPs.

Return Value Description:
    ToasterPowerBeginQueuingIrps returns STATUS_SUCCESS if it successfully changed
    the driver-managed IRP queue state to begin holding any new incoming data I/O
    IRPs.

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData;
    ULONG chargesRemaining;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Change the driver-managed IRP queue mechanism to hold any new incoming data
    // I/O requests because the power manager might send an IRP_MN_SET_POWER S-IRP to
    // change the hardware instance’s device power state.
    //
    fdoData->QueueState = HoldRequests;

    //
    // Decrement the count of outstanding I/O operations to account for the earlier
    // calls to ToasterIoIncrement as part of processing the current power IRP, in
    // order that StopEvent can be signaled.
    //
    chargesRemaining = IrpIoCharges;
    while(chargesRemaining--)
    {
        ToasterIoDecrement(fdoData);
    }

    //
    // Wait for any pending IRPs to finish. ToasterIoDecrement signals StopEvent when
    // the count of outstanding I/O operations becomes 1. Either the last call to
    // ToasterIoDecrement (above) signaled stop event, or another call to
    // ToasterIoDecrement (in another thread of execution) signaled StopEvent because
    // that thread completed the last pending IRP.
    //
    KeWaitForSingleObject(
        &fdoData->StopEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    //
    // The function driver might be forced to queue if the machine is running out
    // of power. The function driver only asks if asking is optional.
    //
    if (Query)
    {
        //
        // If ToasterCallbackHandleDeviceQueryPower called this routine, then
        // determine if the hardware instance can enter a suspended power state.
        // Because the toaster hardware is imaginary, ToasterCanSuspendDevice always
        // returns STATUS_SUCCESS.
        //
        status = ToasterCanSuspendDevice(DeviceObject);

        if (!NT_SUCCESS(status))
        {
            //
            // If the hardware instance cannot be suspended then change the
            // driver-managed IRP queue state to AllowRequests and begin to process
            // any requests queued since the start of ToasterPowerBeginQueuingIrps.
            //
            fdoData->QueueState = AllowRequests;
            ToasterProcessQueuedRequests(fdoData);
        }
    }
    else
    {
        //
        // If ToasterCallbackHandleDeviceSetPower called this routine, then the function
        // driver is processing a IRP_MN_SET_POWER D-IRP, so indicate the operation is
        // successful.
        //
        status = STATUS_SUCCESS;
    }

    //
    // Increment the count of outstanding I/O back to its original value before it
    // was decremented at the start of this routine in order to signal StopEvent.
    //
    chargesRemaining = IrpIoCharges;
    while(chargesRemaining--)
    {
        ToasterIoIncrement(fdoData);
    }

    return status;
}

//
// End Boilerplate Code
// --------------------
//

 
//
// Begin device-specific code
// --------------------------
//

VOID
ToasterCallbackHandleDeviceSetPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction
    )
/*++

New Routine Description:
    ToasterCallbackHandleDeviceSetPower processes corresponding power-up or
    power-down D-IRPs that the function driver created in response to
    IRP_MN_SET_POWER S-IRPs when ToasterQueueCorrespondingDeviceIrp calls
    PoRequestPowerIrp.
    ToasterQueuePassiveLevelPowerCallbackWorker calls this routine to process the
    IRP_MN_SET_POWER D-IRP at IRQL = PASSIVE_LEVEL because the system worker
    thread can be suspended without causing a system deadlock until every pending
    IRP completes.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the IRP_MN_SET_POWER D-IRP operation to perform on the hardware
    instance represented by the DeviceObject parameter. 

    Direction
    Direction indicates whether or not to pass the D-IRP down the device stack. If
    the hardware instance is powering up then Direction = IRP_ALREADY_FORWARDED
    because the underlying bus driver has already processed the power-up D-IRP and
    it is being passed back up the device stack. Otherwise,
    Direction = IRP_NEEDS_FORWARDING because the D-IRP has not yet been processed
    by the underlying bus driver and must still passed down the device stack to be
    processed by the underlying bus driver.

Return Value Description:
    This routine does not return a value.

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    POWER_ACTION        newDeviceAction;
    DEVICE_POWER_STATE  newDeviceState, oldDeviceState;
    POWER_STATE         newState;
    NTSTATUS            status = STATUS_SUCCESS;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterCallbackHandleDeviceSetPower\n");

    //
    // Get the device power state from the corresponding power-up or power-down
    // D-IRP.
    //
    newState =  stack->Parameters.Power.State;
    newDeviceState = newState.DeviceState;

    //
    // Save the hardware instance’s present power state into a temporary variable.
    //
    oldDeviceState = fdoData->DevicePowerState;

    //
    // Update the device extension’s device power state to the new device power
    // state.
    //
    fdoData->DevicePowerState = newDeviceState;

    ToasterDebugPrint(INFO, "\tSetting the device state to %s\n",
                            DbgDevicePowerString(fdoData->DevicePowerState));

    if (newDeviceState == oldDeviceState)
    {
        //
        // If the new device power state is the same as the old device power state,
        // then the incoming D-IRP can be completed without manipulating the hardware
        // instance.
        //
        // The power manager sends IRP_MN_SET_POWER S-IRPs with the same power state
        // as the current power state to cancel earlier IRP_MN_QUERY_POWER S-IRPs
        // because drivers may have begun to queue new incoming data I/O IRPs in
        // response to the earlier S-IRP. If the new device power state is for D0,
        // then change the driver-managed IRP queue state to AllowRequests and begin
        // to process any requests queued since the function driver processed an
        // earlier IRP_MN_QUERY_POWER S-IRP.
        //
        if (newDeviceState == PowerDeviceD0)
        {
            fdoData->QueueState = AllowRequests;

            ToasterProcessQueuedRequests(fdoData);
        }

        //
        // Finish the power-up or power-down D-IRP. Pass the incoming Direction
        // parameter that indicates whether the underlying bus driver has already
        // processed the power-up or power-down D-IRP. Pass STATUS_SUCCESS to
        // indicate that the function driver has completed the power-up or power-down
        // D-IRP.
        //
        ToasterFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            Direction,
            STATUS_SUCCESS
            );

        return;
    }

    if (PowerDeviceD0 == oldDeviceState)
    {
        //
        // If the hardware instance’s previous device power state was D0, then change
        // the driver-managed IRP queue mechanism to begin queuing any new incoming
        // data I/O IRPs in preparation to change the device power state of the
        // hardware instance.
        //
        // Pass 2 as the IoIrpCharges parameter. One is for the original pending
        // IRP_MN_SET_POWER S-IRP, and the other is for the corresponding power-up or
        // power-down D-IRP.
        //
        // Pass FALSE as the Query parameter to indicate that
        // ToasterPowerBeginQueueingIrps should not query the hardware instance to
        // determine if it can be safely suspended because the function driver is
        // processing a power-up or power-down D-IRP.
        //
        status = ToasterPowerBeginQueuingIrps(
            DeviceObject,
            2,
            FALSE
            );

        ASSERT(NT_SUCCESS(status));
    }

    //
    // Get the reason for the device power state change. Note that ShutdownType is
    // not dependable if the device power state is D0.
    //
    newDeviceAction = stack->Parameters.Power.ShutdownType;

    if (newDeviceState > oldDeviceState)
    {
        //
        // If newDeviceState is greater than oldDeviceState then the hardware
        // instance is entering a lower (deeper) sleep state.
        //
        // Store the appropriate hardware context and perform the power-down
        // operation here. Because the toaster hardware is imaginary (the device
        // hardware as well as the bus hardware), there is no code to execute here.
        // Examine the other sample drivers shipped with the DDK to see how power
        // change operations are performed.
        //
        // The Toaster sample function driver does not distinguish the different
        // reasons for a power-down. The newDeviceAction variable would have to be
        // checked to determine the reason, such as standby, suspend, or hibernate.
        //

        //
        // Notify the power manager of the new device power state.
        //
        PoSetPowerState(DeviceObject, DevicePowerState, newState);

        //
        // Indicate that the imaginary hardware successfully powered down.
        //
        status = STATUS_SUCCESS;
    }
    else if (newDeviceState < oldDeviceState)
    {
        //
        // If newDeviceState is less than oldDeviceState then the hardware instance
        // is entering a higher (more awake) sleep state.
        //
        // Restore the saved hardware context and perform the power-up operation
        // here. Because the toaster hardware is imaginary (the device hardware as
        // well as the bus hardware), there is no code to execute here. Examine the
        // other sample drivers shipped with the DDK to see how power change
        // operations are performed.
        //
        // The Toaster sample function driver does not distinguish the different
        // reason for the power level change. The newDeviceAction variable must be
        // checked to determine the reason, such as standby, suspend, or hibernate.
        //

        //
        // Notify the power manager of the new device power state.
        //
        PoSetPowerState(DeviceObject, DevicePowerState, newState);

        //
        // Indicate the imaginary hardware successfully powered up. However, in
        // practice, the power-up D-IRP might be failed here because the hardware
        // has been removed between the power-down D-IRP and the power-up D-IRP.
        //
        status = STATUS_SUCCESS;
    }

    if (PowerDeviceD0 == newDeviceState)
    {
        //
        // If the new device power state is D0, then change the driver-managed IRP
        // queue state to AllowRequests and begin to process any requests queued
        // since the function driver processed an earlier IRP_MN_QUERY_POWER S-IRP.
        //
        // If the incoming power-up or power-down D-IRP does not change the device
        // power state, then this step unblocks the driver-managed IRP queue which
        // was blocked earlier when the function driver processed a
        // IRP_MN_QUERY_POWER D-IRP.
        //
        fdoData->QueueState = AllowRequests;
        ToasterProcessQueuedRequests(fdoData);
    }

    //
    // Finish the power-up or power-down D-IRP. Pass the incoming Direction parameter
    // that indicates whether the underlying bus driver has already processed the
    // power-up or power-down D-IRP. Pass the results of the power-up or power-down
    // operation (in this case, STATUS_SUCCESS) to indicate that the function driver
    // has completed the power-up or power-down D-IRP.
    //
    ToasterFinalizeDevicePowerIrp(
        DeviceObject,
        Irp,
        Direction,
        status
        );
}


 
NTSTATUS
ToasterGetPowerPoliciesDeviceState(
    IN  PIRP                SIrp,
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT DEVICE_POWER_STATE *DevicePowerState
    )
/*++

New Routine Description:
    ToasterGetPowerPoliciesDeviceState defines the appropriate device power state
    for the incoming system power state. This routine maps a given system power
    state to a device power state when changing a hardware instance’s power state.

    ToasterQueueCorrespondingDeviceIrp calls ToasterGetPowerPoliciesDeviceState
    when ToasterQueueCorrespondingDeviceIrp requests a corresponding D-IRP to be
    sent to the device stack for the hardware instance in response to an original
    pending IRP_MN_QUERY_POWER S-IRP or IRP_MN_SET_POWER S-IRP.

    ToasterGetPowerPoliciesDeviceState gets the corresponding D-IRP out of the
    system power state to device power state mapping array that ToasterDispatchPnP
    saved when it processed IRP_MN_QUERY_CAPABILITIES.

    The function driver can choose a deeper sleep state than the mapping (with the
    appropriate caveats if the function driver has children), but the function
    driver must not choose a more awake sleep state, because what hardware remains
    on in a given system power state depending on the mainboard wiring.

    Also note that if the function driver rounds down the device power state, it
    must ensure that such a state is supported. A function driver can determine
    this by examining the DeviceD1 and DeviceD2 bits in the DEVICE_CAPABILITIES
    structure saved when it processes IRP_MN_QUERY_DEVICE_CAPABILITIES (on Windows
    2000, Windows XP and later), or by examining the entire system power state to
    device power state mapping (on Windows 98).

Parameters Description:
    SIrp
    SIrp specifies the system power state to be mapped to a device power state.

    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming SIrp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    DevicePowerState
    DevicePowerState points to caller allocated space that receives the device
    power state appropriate for the incoming system power state.

Return Value Description:
    ToasterGetPowerPoliciesDeviceState returns STATUS_SUCCESS if there is an
    appropriate system power state to device power state mapping. Otherwise,
    ToasterGetPowerPoliciesDeviceState returns STATUS_UNSUCCESSFUL.

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);
    SYSTEM_POWER_STATE  systemState = stack->Parameters.Power.State.SystemState;

    if (PowerSystemWorking == systemState)
    {
        //
        // If the system is entering a fully-powered (S0) power state then the
        // system is waking-up to a working power state. Set the appropriate
        // device power state to D0 and then return.
        //
        *DevicePowerState = PowerDeviceD0;
    }
    else
    {
        //
        // This stage of the function driver does not demonstrate how to support
        // wait-wake. Therefore, return D3 device power state. The Featured2 stage
        // of the function driver implements wait-wake support.
        //
        *DevicePowerState = PowerDeviceD3;
    }

    return STATUS_SUCCESS;
}


 
NTSTATUS
ToasterCanSuspendDevice(
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

New Routine Description:
    ToasterCanSuspendDevice determines whether the hardware instance can be safely
    suspended. In the case of an imaginary toaster, the hardware can always be
    suspended.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is being queried if it can
    be suspended. DeviceObject is a FDO created earlier in ToasterAddDevice.

Return Value Description:
    ToasterCanSuspendDevice returns STATUS_SUCCESS to indicate that the hardware
    instance can safely be suspended. Because the Toaster hardware is imaginary,
    IRP_MN_QUERY_POWER always succeeds for device power-down. However, in
    practice, ToasterCanSuspendDevice could fail return failure if its hardware
    instance does not have a queue for the I/O operations that might come in, or
    if the driver was notified that it is in the paging path.

--*/
{
    PAGED_CODE();

    return STATUS_SUCCESS;
}

//
// End device-specific code
// ------------------------
//


 
PCHAR
DbgPowerMinorFunctionString (
    UCHAR MinorFunction
    )
/*++

New Routine Description:
    DbgPowerMinorFunctionString converts the minor function code of a power IRP to
    a text string that is more helpful when tracing the execution of power IRPs.

Parameters Description:
    MinorFunction
    MinorFunction specifies the minor function code of a power IRP.

Return Value Description:
    DbgPowerMinorFunctionString returns a pointer to a string that represents the
    text description of the incoming minor function code.

--*/
{
    switch (MinorFunction)
    {
    case IRP_MN_SET_POWER:
        return "IRP_MN_SET_POWER";
    case IRP_MN_QUERY_POWER:
        return "IRP_MN_QUERY_POWER";
    case IRP_MN_POWER_SEQUENCE:
        return "IRP_MN_POWER_SEQUENCE";
    case IRP_MN_WAIT_WAKE:
        return "IRP_MN_WAIT_WAKE";
    default:
        return "unknown_power_irp";
    }
}

 
PCHAR
DbgSystemPowerString(
    IN SYSTEM_POWER_STATE Type
    )
/*++

New Routine Description:
    DbgSystemPowerString converts the system power state code of a power IRP to a
    text string that is more helpful when tracing the execution of power IRPs.

Parameters Description:
    Type
    Type specifies the system power state code of a power IRP.

Return Value Description:
    DbgDevicePowerString returns a pointer to a string that represents the
    text description of the incoming system power state code.

--*/
{
    switch (Type)
    {
    case PowerSystemUnspecified:
        return "PowerSystemUnspecified";
    case PowerSystemWorking:
        return "PowerSystemWorking";
    case PowerSystemSleeping1:
        return "PowerSystemSleeping1";
    case PowerSystemSleeping2:
        return "PowerSystemSleeping2";
    case PowerSystemSleeping3:
        return "PowerSystemSleeping3";
    case PowerSystemHibernate:
        return "PowerSystemHibernate";
    case PowerSystemShutdown:
        return "PowerSystemShutdown";
    case PowerSystemMaximum:
        return "PowerSystemMaximum";
    default:
        return "UnKnown System Power State";
    }
 }

 
PCHAR
DbgDevicePowerString(
    IN DEVICE_POWER_STATE Type
    )
/*++

New Routine Description:
    DbgDevicePowerString converts the device power state code of a power IRP to a
    text string that is more helpful when tracing the execution of power IRPs.

Parameters Description:
    Type
    Type specifies the device power state code of a power IRP.

Return Value Description:
    DbgDevicePowerString returns a pointer to a string that represents the
    text description of the incoming device power state code.

--*/
{
    switch (Type)
    {
    case PowerDeviceUnspecified:
        return "PowerDeviceUnspecified";
    case PowerDeviceD0:
        return "PowerDeviceD0";
    case PowerDeviceD1:
        return "PowerDeviceD1";
    case PowerDeviceD2:
        return "PowerDeviceD2";
    case PowerDeviceD3:
        return "PowerDeviceD3";
    case PowerDeviceMaximum:
        return "PowerDeviceMaximum";
    default:
        return "UnKnown Device Power State";
    }
}

