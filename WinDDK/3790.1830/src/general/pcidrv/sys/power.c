/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    power.c

Abstract:

    The power management related processing.

    The Power Manager uses IRPs to direct drivers to change system
    and device power levels, to respond to system wake-up events,
    and to query drivers about their devices. All power IRPs have
    the major function code IRP_MJ_POWER.

    Most function and filter drivers perform some processing for
    each power IRP, then pass the IRP down to the next lower driver
    without completing it. Eventually the IRP reaches the bus driver,
    which physically changes the power state of the device and completes
    the IRP.

    When the IRP has been completed, the I/O Manager calls any
    IoCompletion routines set by drivers as the IRP traveled
    down the device stack. Whether a driver needs to set a completion
    routine depends upon the type of IRP and the driver's individual
    requirements.

    The power policy of this driver is simple. The device enters the
    device working state D0 when the system enters the system
    working state S0. The device enters the lowest-powered sleeping state
    D3 for all the other system power states (S1-S5).

Environment:

    Kernel mode

Revision History:

     Based on Adrian J. Oney's power management boilerplate code 
     
--*/

#include "precomp.h"
#include "power.h"

#if defined(EVENT_TRACING)
#include "power.tmh"
#endif

//
// We will keep all the power code which could be called at
// DISPATCH_LEVEL in a separate section so that we can lock
// and unlock code section depending on whether DO_POWER_PAGABLE
// bit is set or not.
//
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_PWR, PciDrvDispatchPower)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchPowerDefault)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchSetPowerState)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchQueryPowerState)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchSystemPowerIrp)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchDeviceQueryPower)
#pragma alloc_text(PAGE_PWR, PciDrvDispatchDeviceSetPower)
#pragma alloc_text(PAGE_PWR, PciDrvPowerBeginQueuingIrps)
#pragma alloc_text(PAGE_PWR, PciDrvCallbackHandleDeviceQueryPower)
#pragma alloc_text(PAGE_PWR, PciDrvCallbackHandleDeviceSetPower)
#pragma alloc_text(PAGE_PWR, PciDrvCanSuspendDevice)
#endif // ALLOC_PRAGMA

//
// The code for this file is split into two sections, biolerplate code and
// device programming code.
//
// The Biolerplate code comes first. The device programming code comes last in
// the file.
//

//
// Begin Biolerplate Code
// ----------------------
//

NTSTATUS
PciDrvDispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    The power dispatch routine.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PIO_STACK_LOCATION  stack;
    PFDO_DATA           fdoData;
    NTSTATUS            status;

    stack   = IoGetCurrentIrpStackLocation(Irp);
    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Make sure the correct dispatch function was called.
    //
    ASSERT( stack->MajorFunction == IRP_MJ_POWER );

    DebugPrint(INFO, DBG_POWER, "FDO %s %s IRP:0x%p %s %s\n",
                  DbgPowerMinorFunctionString(stack->MinorFunction),
                  stack->Parameters.Power.Type == SystemPowerState ?"SIRP":"DIRP",
                  Irp,
                  DbgSystemPowerString(fdoData->SystemPowerState),
                  DbgDevicePowerString(fdoData->DevicePowerState));


    //
    // Increment our records to show there's currently a pending
    // request against this device.  This allows us to handle things
    // like undocking more cleanly.
    //
    PciDrvIoIncrement (fdoData);
    
    //
    // See if the device is even present.
    //
    if (Deleted == fdoData->DevicePnPState) {
        
        //
        // We're going to fail this power IRP because the device
        // is no longer present.
        //        
        //
        // Inform the power manager that we are able to accept another power IRP.
        //
        PoStartNextPowerIrp (Irp);
                
        //
        // Indicate why we're failing the IRP.
        //
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
                
        //
        // Tell the I/O manager that we've completed all processing on this
        // IRP.
        //
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
                
        //
        // Decrement our device's "Pending request" count and return.
        //
        PciDrvIoDecrement (fdoData);

        //
        // We can't just return Irp->IoStatus.Status here because
        // we've already called IoCompleteRequest with the Irp, so
        // it may have been freed by now.
        //
        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // If the device is not stated yet, just pass it down.
    //
    if (NotStarted == fdoData->DevicePnPState ) {
        
        //
        // Inform the power manager that we are able to accept another power IRP.
        //        
        PoStartNextPowerIrp(Irp);
        
        //
        // Fix up the IRP's stack location and call the next driver.
        //
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(fdoData->NextLowerDriver, Irp);
        
        //
        // Decrement our device's "Pending request" count and return.
        //
        PciDrvIoDecrement (fdoData);
        return status;
    }

    if(fdoData->IsUpperEdgeNdis){        
        // 
        // We will let the NDIS be the power policy owner and do the
        // S-D mappings and wait-wake handling. So let us just pass
        // whatever we get to the bus driver. 
        //
            
        //
        // Inform the power manager that we are able to accept another power IRP.
        //        
        PoStartNextPowerIrp(Irp);

        //
        // Fix up the IRP's stack location and call the next driver.
        //
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(fdoData->NextLowerDriver, Irp);

        //
        // Decrement our device's "Pending request" count and return.
        //
        PciDrvIoDecrement(fdoData);
        return status;
    }
            
    //
    // Check the request type.
    //

    switch(stack->MinorFunction) {

        case IRP_MN_SET_POWER:

            //
            // The Power Manager sends this IRP for one of the
            // following reasons:
            // 1) To notify drivers of a change to the system power state.
            // 2) To change the power state of a device for which
            //    the Power Manager is performing idle detection.
            // A driver sends IRP_MN_SET_POWER to change the power
            // state of its device if it's a power policy owner for the
            // device.
            //
            
            status = PciDrvDispatchSetPowerState(DeviceObject, Irp);

            break;


        case IRP_MN_QUERY_POWER:

            //
            // The Power Manager sends a power IRP with the minor
            // IRP code IRP_MN_QUERY_POWER to determine whether it
            // can safely change to the specified system power state
            // (S1-S5) and to allow drivers to prepare for such a change.
            // If a driver can put its device in the requested state,
            // it sets status to STATUS_SUCCESS and passes the IRP down.
            //            
            status = PciDrvDispatchQueryPowerState(DeviceObject, Irp);
            
            break;

        case IRP_MN_WAIT_WAKE:

            //
            // The minor power IRP code IRP_MN_WAIT_WAKE provides
            // for waking a device or waking the system. Drivers
            // of devices that can wake themselves or the system
            // send IRP_MN_WAIT_WAKE. The system sends IRP_MN_WAIT_WAKE
            // only to devices that always wake the system, such as
            // the power-on switch.
            //            
            status = PciDrvDispatchWaitWake(DeviceObject, Irp);
            break;

        case IRP_MN_POWER_SEQUENCE:

            //
            // A driver sends this IRP as an optimization to determine
            // whether its device actually entered a specific power state.
            // This IRP is optional. Power Manager cannot send this IRP.
            //

        default:
            //
            // Pass it down
            //
            status = PciDrvDispatchPowerDefault(DeviceObject, Irp);
            PciDrvIoDecrement(fdoData);
            break;
    }

    return status;
}


NTSTATUS
PciDrvDispatchPowerDefault(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    )

/*++

Routine Description:

    If a driver does not support a particular power IRP,
    it must nevertheless pass the IRP down the device stack
    to the next-lower driver. A driver further down the stack
    might be prepared to handle the IRP and must have the
    opportunity to do so.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    NTSTATUS         status;
    PFDO_DATA        fdoData;

    //
    // Drivers must call PoStartNextPowerIrp while the current
    // IRP stack location points to the current driver.
    // This routine can be called from the DispatchPower routine
    // or from the IoCompletion routine. However, PoStartNextPowerIrp
    // must be called before IoCompleteRequest, IoSkipCurrentIrpStackLocation,
    // and PoCallDriver. Calling the routines in the other order might
    // cause a system deadlock.
    //

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Drivers must use PoCallDriver, rather than IoCallDriver,
    // to pass power IRPs. PoCallDriver allows the Power Manager
    // to ensure that power IRPs are properly synchronized throughout
    // the system.
    //

    status = PoCallDriver(fdoData->NextLowerDriver, Irp);

    return status;
}


NTSTATUS
PciDrvDispatchSetPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

   Processes IRP_MN_SET_POWER.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchSetPowerState\n");


    //
    // If the set-power is for an S-state, then we will need to
    // convert the S-irp into a D-irp before handling it.  We
    // already have logic for that conversion (as well as handling
    // logic) in the SystemPowerIrp dispatch function.  We can leverage
    // that existing code if this is an S-irp by simply calling through
    // to DispatchSystemPowerIrp().  Otherwise, call through to our
    // DeviceSetPower() function.
    //
    return (stack->Parameters.Power.Type == SystemPowerState) ?
        PciDrvDispatchSystemPowerIrp(DeviceObject, Irp) :
        PciDrvDispatchDeviceSetPower(DeviceObject, Irp);
}


NTSTATUS
PciDrvDispatchQueryPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

   Processes IRP_MN_QUERY_POWER.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchQueryPowerState\n");

    
    //
    // If the query is for an S-state, then we will need to
    // convert the S-irp into a D-irp before handling it.  We
    // already have logic for that conversion (as well as handling
    // logic) in the SystemPowerIrp dispatch function.  We can leverage
    // that existing code if this is an S-irp by simply calling through
    // to DispatchSystemPowerIrp().  Otherwise, call through to our
    // DeviceQueryPower() function.
    //
    return (stack->Parameters.Power.Type == SystemPowerState) ?
        PciDrvDispatchSystemPowerIrp(DeviceObject, Irp) :
        PciDrvDispatchDeviceQueryPower(DeviceObject, Irp);
}


NTSTATUS
PciDrvDispatchSystemPowerIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

   Processes IRP_MN_SET_POWER and IRP_MN_QUERY_POWER
   for the system power Irp (S-IRP).

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchSystemPowerIrp\n");

    //
    // Here we update our cached away system power state.  We're careful
    // here to make sure to only do this if the MinorFunction is a SET_POWER.
    // That's because we're reusing this function for S-irp QUERY_POWERs also.
    // We don't want to record the new power state if they're only querying.
    //
    if (stack->MinorFunction == IRP_MN_SET_POWER) {
        fdoData->SystemPowerState = stack->Parameters.Power.State.SystemState;
        DebugPrint(INFO, DBG_POWER, "\tSetting the system state to %s\n",
                    DbgSystemPowerString(fdoData->SystemPowerState));
    }

    //
    // Send the IRP down asynchronously.  Do that by telling the
    // I/O manager that the IRP's progress is pending, then set
    // a callback routine that will get called when the next guy
    // down the driver stack 'completes' the IRP, then call the
    // next guy down the driver stack.
    //
    IoMarkIrpPending(Irp);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
        Irp,
        (PIO_COMPLETION_ROUTINE) PciDrvCompletionSystemPowerUp,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

    PoCallDriver(fdoData->NextLowerDriver, Irp);

    return STATUS_PENDING;
}


NTSTATUS
PciDrvCompletionSystemPowerUp(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
/*++

Routine Description:

   The completion routine for Power Up S-IRP.  This function gets called
   when the driver below us actually completes the IRP.  It gives us another
   chance to operate on the irp.  Here, we'll take the opportunity to queue
   a D-IRP which corresponds to the S-IRP that is being completed.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

   Not used  - context pointer

Return Value:

   NT status code

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA) Fdo->DeviceExtension;
    NTSTATUS    status = Irp->IoStatus.Status;

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvCompletionSystemPowerUp\n");

    if (!NT_SUCCESS(status)) {

        //
        // Someone below us has failed this S-IRP, so we won't be
        // queueing the corresponding D-IRP.  Just inform the 
        // power manager that we're now able to accept another
        // power IRP, decrement our device's "Pending requests" counter
        // and return.
        //
        PoStartNextPowerIrp(Irp);
        PciDrvIoDecrement(fdoData);
        
        //
        // Note that we do not return Irp->IoStatus.Status.
        // That will be percolated back up the stack via the IRP.
        // The DDK tells us that in our completion routine, we have
        // two choices.  We can either return STATUS_MORE_PROCESSING_REQUIRED,
        // which is what we'd do if we queued the corresponding D-IRP
        // *OR*
        // we can return STATUS_CONTINUE_COMPLETION.  This tells the I/O
        // manager that we're done with this IRP and he can continue
        // completing the IRP.
        //
        return STATUS_CONTINUE_COMPLETION;
    }

    //
    // Queue a D-IRP and tell the I/O manager that there's more
    // work to do..
    //
    PciDrvQueueCorrespondingDeviceIrp(Irp, Fdo);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID
PciDrvQueueCorrespondingDeviceIrp(
    IN PIRP SIrp,
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

   This routine gets the D-State for a particular S-State
   from DeviceCaps and generates a D-IRP.

Arguments:

   Irp - pointer to an S-IRP.

   DeviceObject - pointer to a device object.

Return Value:

   NT status code

--*/

{
    NTSTATUS            status;
    POWER_STATE         state;
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);
    UCHAR               minor = stack->MinorFunction;
    BOOLEAN             sIrpCompleted = FALSE;
        
    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvQueueCorrespondingDeviceIrp\n");

    //
    // Get the device state to request.
    //
    status = PciDrvGetPowerPoliciesDeviceState(
        SIrp,
        DeviceObject,
        &state.DeviceState
        );

    if (NT_SUCCESS(status)) {
        //
        // Remember the pending SIrp for use in our completion routine.
        //
        ASSERT(!fdoData->PendingSIrp);
        fdoData->PendingSIrp = SIrp;        

        //
        // On Win2K and beyond, to facilitate fast-resume from suspended
        // state, if the set-power IRP is to get back to PowerSystemWorking 
        // (S0) state, we will complete the S0 before acting on the DeviceIrp. 
        // This is because the system waits for all the S0 IRPs sent system 
        // wide to complete before resuming any usermode process activity. 
        // Since powering up the device and restoring device context takes 
        // longer, it's not necessary to hold the S-IRP and as a result 
        // slow down resume for non-critical devices.
        //
        
        if(stack->MinorFunction == IRP_MN_SET_POWER &&
            stack->Parameters.Power.State.SystemState == PowerSystemWorking) {
        
            DebugPrint(TRACE, DBG_POWER, "\t ****Completing S0 IRP\n");
            SIrp->IoStatus.Status = STATUS_SUCCESS;
            
            //
            // Tell the power manager we are capable of recieving power irps,
            // complete this S-IRP, erase any knowledge of a pending S-IRP,
            // then decrement our outstanding I/O request counter.
            // Please note that S-IRPs and state changing PNP IRPs such as
            // query-stop, cancel-stop, stop, query-remove. remove, surprise-
            // remove and start are serialized with S-Irps. So, if we complete 
            // the S0-IRP and then request a D0-IRP, there is a possibility 
            // for us to receive a stop or remove IRP before receiving the  
            // D0-IRP. To prevent those IRPs from freeing hardware resources, we
            // will take an extra reference on OutstandingIo before we generate
            // a DO-IRP and compensate for that when we complete the D0-IRP.
            //
            ASSERT(state.DeviceState == PowerDeviceD0);
            
            PciDrvIoIncrement(fdoData);  
            //
            // Later we need to know whether the D0-Irp we received is result
            // of S0-IRP or the one generated by idle detection logic so that
            // we can compensate for the extra count taken here. To do that, we
            // will set the PendingSIrp field to some MAGIC_NUMBER
            // and check for that. Since the idle-detection is enabled only
            // after the device is powered up, we don't have to worry about 
            // two D0-IRPs racing thru the stack and beating each other.
            //
            fdoData->PendingSIrp = (PVOID)MAGIC_NUMBER;
            
            PoStartNextPowerIrp(SIrp);
            IoCompleteRequest(SIrp, IO_NO_INCREMENT);
            PciDrvIoDecrement(fdoData); 
            sIrpCompleted = TRUE;
                
        }
        
        //
        // Initiate another IRP (this time it's the D-IRP) for this
        // power action.  Set a callback to get notified when the driver
        // below us has completed this D-IRP.
        //
        status = PoRequestPowerIrp(
            fdoData->UnderlyingPDO,
            minor,
            state,
            PciDrvCompletionOnFinalizedDeviceIrp,
            fdoData,
            NULL
            );
        
    }

    if (!NT_SUCCESS(status)) {

        //
        // Something failed.  Inform the power manager we are able to
        // accept another power IRP, indicate the failure code, tell
        // the I/O manager we've completed all processing on this IRP,
        // and decrement our device's "Pending request" counter.
        // Note STATUS_PENDING is not a failure code.
        //
        fdoData->PendingSIrp = NULL;        
        if(sIrpCompleted) {
            PciDrvIoDecrement(fdoData);            
        } else {
        
            PoStartNextPowerIrp(SIrp);
            SIrp->IoStatus.Status = status;
            IoCompleteRequest(SIrp, IO_NO_INCREMENT);
            PciDrvIoDecrement(fdoData);
        }
    }
}


VOID
PciDrvCompletionOnFinalizedDeviceIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID    PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    )
/*++

Routine Description:

   Completion routine for D-IRP.

Arguments:


Return Value:

   NT status code

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA) PowerContext;
    PIRP        sIrp = fdoData->PendingSIrp;

    DebugPrint(TRACE, DBG_POWER, 
                    "Entered PciDrvCompletionOnFinalizedDeviceIrp\n");

    //
    // If this D-IRP was the result of a translated S-IRP, then we need
    // to fix up the S-IRP and complete it.
    //

    if(sIrp == (PVOID)MAGIC_NUMBER){
        //
        // This D0-IRP was result of an already completed S0-IRP.
        // So, just do a decrement of the OutstandingIo to compensate
        // for the one taken before generating the D0-IRP, and
        // return.
        //
        ASSERT(PowerState.DeviceState == PowerDeviceD0);
        sIrp = fdoData->PendingSIrp = NULL;
        PciDrvIoDecrement(fdoData);        
        return;
    }

    if(sIrp) {
        //
        // Here we copy the D-IRP status into the S-IRP
        //
        sIrp->IoStatus.Status = IoStatus->Status;

        //
        // Inform the power manager we are able to accept another
        // power IRP, tell the I/O manager we've completed all 
        // processing on this IRP, and decrement our device's "Pending 
        // request" counter.
        //
        PoStartNextPowerIrp(sIrp);
        IoCompleteRequest(sIrp, IO_NO_INCREMENT);

        //
        // Erase the notion of a pending S-IRP.
        //
        fdoData->PendingSIrp = NULL;
        PciDrvIoDecrement(fdoData);
    }
}


NTSTATUS
PciDrvDispatchDeviceQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Routine Description:

    Handles IRP_MN_QUERY_POWER for D-IRP

Arguments:

    DeviceObject - pointer to a device object.

    Irp - pointer to an I/O Request Packet.

Return Value:

    NT status code

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE  deviceState = stack->Parameters.Power.State.DeviceState;
    NTSTATUS            status;

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchDeviceQueryPower\n");

    //
    // Here we check to see if it's OK to move to the specified power state. Note
    // that our driver may have requests that would cause us to fail this
    // check. If so, we need to begin queuing requests after succeeding this
    // call (otherwise we may succeed such an IRP *after* we've said we can
    // power down).
    //
    if (deviceState == PowerDeviceD0) {

        //
        // Note - this driver does not queue IRPs if the S-to-D state mapping
        //        specifies that the device will remain in D0 during standby.
        //        For some devices, this could be a problem. For instance, if
        //        an audio card were in a machine where S1->D0, it might not
        //        want to stay "active" during standby (that could be noisy).
        //
        //        Ideally, a driver would be able to use the ShutdownType field
        //        in the D-IRP to distinguish these cases. Unfortunately, this
        //        field cannot be trusted for D0 IRPs. A driver can get the same
        //        information however by maintaining a pointer to the current
        //        S-IRP in its device extension. Of course, such a driver must
        //        be very very careful if it also does idle detection (which is
        //        not shown here).
        //
        status = STATUS_SUCCESS;

    } else {

        //
        // They're asking if we can move to a low-power state.  We can't
        // know until we ask all the drivers below us in the stack.  We
        // also can't just send the IRP down the stack.  Rather, we need
        // to queue a worker to pend the IRP, then send it down the stack 
        // at PASSIVE_LEVEL.
        //
        status = PciDrvQueuePassiveLevelCallback(
                    (PFDO_DATA)DeviceObject->DeviceExtension,
                     PciDrvCallbackHandleDeviceQueryPower,
                    (PVOID)Irp, // Pointer to D-IRP
                    (PVOID)IRP_NEEDS_FORWARDING // Direction
                    );

        //
        // If successful, he'll return STATUS_SUCCESS and eventually
        // PciDrvFinalizeDevicePowerIrp() will get called from
        // PciDrvCallbackHandleDeviceQueryPower, which will finalize
        // the D-IRP handling.
        //
        if (NT_SUCCESS(status)) {

            return STATUS_PENDING;
        }
    }

    //
    // Either they're asking if we can go to D0 (which we'll always succeed),
    // or our attempt to queue a PASSIVE_LEVEL request to the drivers below
    // us failed.  Either way, it's time to finalize the D-IRP handling.
    //
    return PciDrvFinalizeDevicePowerIrp(
        DeviceObject,
        Irp,
        IRP_NEEDS_FORWARDING,
        status
        );
}


NTSTATUS
PciDrvDispatchDeviceSetPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Handles IRP_MN_SET_POWER for D-IRP

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

      NT status code

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    POWER_STATE         state = stack->Parameters.Power.State;
    NTSTATUS            status;
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchDeviceSetPower\n");

    if (state.DeviceState < fdoData->DevicePowerState) { // adding power


        //
        // Send the IRP down asynchronously.  Do that by telling the
        // I/O manager that the IRP's progress is pending, then set
        // a callback routine that will get called when the next guy
        // down the driver stack 'completes' the IRP, then call the
        // next guy down the driver stack.
        //
        IoMarkIrpPending(Irp);

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
            Irp,
            (PIO_COMPLETION_ROUTINE) PciDrvCompletionDevicePowerUp,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );

        PoCallDriver(fdoData->NextLowerDriver, Irp);
        return STATUS_PENDING;

    } else {

        //
        // We are here if we are entering a deeper sleep or entering a state
        // we are already in.
        //
        // As non-D0 IRPs are not alike (some may be for hibernate, shutdown,
        // or sleeping actions), we present these to our state machine.
        //
        // All D0 IRPs are alike though, and we don't want to touch our hardware
        // on a D0->D0 transition. However, we must still present them to our
        // state machine in case we succeeded a Query-D call (which may begin
        // queueing future requests) and the system has sent an S0 IRP to cancel
        // that preceeding query.
        //
        
        
        //
        // Send the IRP down our stack, but we need to send it down at PASSIVE_LEVEL,
        // Use a worker to queue the IRP.
        //
        status = PciDrvQueuePassiveLevelCallback(
                        fdoData,
                        PciDrvCallbackHandleDeviceSetPower,
                        (PVOID)Irp,
                        (PVOID)IRP_NEEDS_FORWARDING
                        );


        //
        // If successful, he'll return STATUS_SUCCESS and eventually
        // PciDrvFinalizeDevicePowerIrp() will get called from
        // PciDrvCallbackHandleDeviceSetPower, which will finalize
        // the D-IRP handling.
        //
        if (NT_SUCCESS(status)) {

            return STATUS_PENDING;
        }

        
        //
        // If he didn't return STATUS_PENDING, he had better have
        // returned a success code.
        //
        ASSERT(!NT_SUCCESS(status));

        //
        // If we didn't get a pending return code, then it's time
        // to finalize the D-IRP handling now.
        //
        return PciDrvFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            status
            );
    }
}


NTSTATUS
PciDrvCompletionDevicePowerUp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID NotUsed
    )
/*++

Routine Description:

   The completion routine for Power Up D-IRP.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

   Not used  - context pointer

Return Value:

   NT status code

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    NTSTATUS            status = Irp->IoStatus.Status;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvCompletionDevicePowerUp\n");

    if (!NT_SUCCESS(status)) {

        //
        // Someone below us has failed the D-IRP to move to a higher
        // power state.  Tell the power manager we can accept another
        // POWER irp, decrement our outstanding I/O requests counter, and
        // tell the I/O manager he can continue with the completion
        // process...  Note that the failure status will get percolated
        // up the stack in the IRP structure.  We don't return that
        // failure status here.
        //
        PoStartNextPowerIrp(Irp);
        PciDrvIoDecrement(fdoData);
        return STATUS_CONTINUE_COMPLETION;
    }

    ASSERT(stack->MajorFunction == IRP_MJ_POWER);
    ASSERT(stack->MinorFunction == IRP_MN_SET_POWER);

    //
    // Send the IRP down our stack, but we need to send it down at PASSIVE_LEVEL,
    // Use a worker to queue the IRP.
    //
    status = PciDrvQueuePassiveLevelCallback(
        fdoData,
        PciDrvCallbackHandleDeviceSetPower,
        (PVOID)Irp,
        (PVOID)IRP_ALREADY_FORWARDED
        );


    if (!NT_SUCCESS(status)) {
                
        //
        // If we didn't get a pending return code, then it's time
        // to finalize the D-IRP handling now.
        //
        // Note that we send the IRP_ALREADY_FORWARDED so that
        // PciDrvFinalizeDevicePowerIrp does little more than
        // complete the irp and do some cleanup for us.
        //
        PciDrvFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            IRP_ALREADY_FORWARDED,
            status
            );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}


NTSTATUS
PciDrvFinalizeDevicePowerIrp(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PIRP                Irp,
    IN  IRP_DIRECTION       Direction,
    IN  NTSTATUS            Result
    )
/*++

Routine Description:

   This is the final step in D-IRP handling.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an D-IRP.

   Direction -  Whether to forward the D-IRP down or not.
                This depends on whether the system is powering
                up or down.

   Result  -
Return Value:

   NT status code

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvFinalizeDevicePowerIrp\n");

    if (Direction == IRP_ALREADY_FORWARDED || (!NT_SUCCESS(Result))) {

        //
        // In either of these cases it is now time to complete the IRP.
        //


        //
        // Inform the power manager that we are able to accept another power IRP.
        //
        PoStartNextPowerIrp(Irp);
        
        //
        // Record the status in the IRP structure.
        //
        Irp->IoStatus.Status = Result;


        //
        // Tell the I/O manager that we've completed all processing on this
        // IRP.
        //
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        
        
        //
        // Decrement our device's "Pending request" count and return.
        //
        PciDrvIoDecrement (fdoData);

        //
        // We can't just return Irp->IoStatus.Status here because
        // we've already called IoCompleteRequest with the Irp, so
        // it may have been freed by now.
        //
        return Result;
    }

    //
    // Here we update our result. Note that PciDrvDefaultPowerHandler calls
    // PoStartNextPowerIrp for us.
    //
    Irp->IoStatus.Status = Result;
    status = PciDrvDispatchPowerDefault(DeviceObject, Irp);
    PciDrvIoDecrement(fdoData);
    return status;
}


VOID
PciDrvCallbackHandleDeviceQueryPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PWORK_ITEM_CONTEXT Context
    )
/*++

Routine Description:

   This routine handles the actual query-power state changes for the device.

Arguments:

   DeviceObject - pointer to a device object.

   Context - Pointer to WORKER_ITEM_CONTEXT
   
Return Value:

   NT status code

--*/
{
    DEVICE_POWER_STATE deviceState;
    PIO_STACK_LOCATION stack;
    NTSTATUS status;
    PIRP      irp;
    IRP_DIRECTION direction;
    PFDO_DATA fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    PAGED_CODE();
    //
    // Pointer to the D-IRP.
    //
    irp = Context->Argument1;
    
    // 
    // direction -  Whether to forward the D-IRP down or not.
    // This depends on whether the system is powering up or down.
    //
    direction = (IRP_DIRECTION)Context->Argument2;
    
    stack = IoGetCurrentIrpStackLocation(irp);

    deviceState = stack->Parameters.Power.State.DeviceState;
    
    //
    // We should never get here if they're querying to see if we can go
    // to D0 because that would have been succeeded back in 
    // PciDrvDispatchDeviceQueryPower()
    //
    ASSERT(PowerDeviceD0 != deviceState);


    //
    // The user has queried to see if we can go to a low power state.
    // We will take this as a signal that a low power state is coming
    // (and he'll send us a cancel in the form of an IRP_MN_SET_POWER
    // if that low power state isn't coming), so start queuing IRPs 
    // now so we can handle them once we move back to D0 at some point 
    // in the future.
    //
    status = PciDrvPowerBeginQueuingIrps(
        DeviceObject,
        fdoData->PendingSIrp? 2 : 1, // one for the S-IRP and one for the D-IRP;
        TRUE            // Query for state change.
        );

    //
    // Finish the power IRP operation.
    //
    PciDrvFinalizeDevicePowerIrp(DeviceObject, irp, direction, status);

    //
    // Cleanup before exiting from the worker thread.
    //
    IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
    ExFreePool((PVOID)Context);
    
}

NTSTATUS
PciDrvPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    )
/*++

Routine Description:

    This routine causes new Irp requests to be queued and existing IRP requests
    to be finished.

Arguments:

   DeviceObject - pointer to a device object.

   IrpIoCharges - Number of PciDrvIoIncrement calls charged against the
                  caller of this function.

   Query - TRUE if the driver should be queried as to whether the queuing
           should occur.

Return Value:

   NT status code

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;


    //
    // Tell everybody that incoming requests are to be queued instead
    // of processed.
    //
    fdoData->QueueState = HoldRequests;

    //
    // Move IRPs from active hw queue to main incoming requests queue.
    //
    PciDrvWithdrawIrps(fdoData);
    
    //
    // Cancel the idle detection timer. We will restart the timer
    // when we power back up.
    //
    PciDrvCancelIdleDetectionTimer(fdoData);


    //
    // Wait for the I/O in progress to finish. Our caller might have 
    // had Io counts charged against his IRPs too, so we also adjust for those.
    //   
    PciDrvReleaseAndWait(fdoData, IrpIoCharges, STOP);

    //
    // We might be forced to queue if the machine is running out of power.
    // We only ask if asking is optional.
    //
    if (Query) {

        //
        // Here's where we'd see if we could stop using our device. We'll
        // pretend to be successful instead.
        //
        status = PciDrvCanSuspendDevice(DeviceObject);

        if (!NT_SUCCESS(status)) {

            //
            // Open the locks and process anything that actually got
            // queued.
            //
            fdoData->QueueState = AllowRequests;
            PciDrvProcessQueuedRequests(fdoData);
        }

    } else {

        status = STATUS_SUCCESS;
    }

    return status;
}


//
// Begin device-specific code
// --------------------------
//

VOID
PciDrvCallbackHandleDeviceSetPower(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PWORK_ITEM_CONTEXT Context
    )
/*++

Routine Description:

   This routine performs the actual power changes to the device.

Arguments:

   DeviceObject - pointer to a device object.

   Context - Pointer to WORKER_ITEM_CONTEXT

Return Value:

   NT status code

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    POWER_ACTION        newDeviceAction;
    DEVICE_POWER_STATE  newDeviceState, oldDeviceState;
    POWER_STATE         newState;
    NTSTATUS            status = STATUS_SUCCESS;
    PIRP                Irp = Context->Argument1;
    IRP_DIRECTION       Direction = (IRP_DIRECTION)Context->Argument2;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    
    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvCallbackHandleDeviceSetPower\n");

    PAGED_CODE();
    
    newState =  stack->Parameters.Power.State;
    newDeviceState = newState.DeviceState;
    oldDeviceState = fdoData->DevicePowerState;

    if(newDeviceState == PowerDeviceD0 && 
       fdoData->SystemPowerState != PowerSystemWorking){
        
        //
        // We're being asked to power up to D0 while the system is
        // in standby, which is nonsensical.
        //
        // This odd scenario can happen under the following circumstances:
        // 1. The device is using idle detection and was powered off.
        // 2. The idle detector fired off a D0 IRP to us to restart power.
        // 3. The system detects an overall idle state and decides to go to
        //    S3, thus resulting in a D3 IRP coming to us.
        // 4. The D3 irp passes the original D0 irp, so we power down, then
        //    immediately get a stale D0 IRP.
        //
        // We're going to finalize the IRP as unsuccessful via a call
        // to PciDrvFinalizeDevicePowerIrp(STATUS_UNSUCCESSFUL).
        //
        //DebugPrint(INFO, DBG_POWER, "This is a request to power up from", 
        //     " the idle logic after the device has gone to sleep. So ignore\n");

        PciDrvFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            Direction,
            STATUS_UNSUCCESSFUL
            );
        goto Exit;
        
    }

    DebugPrint(INFO, DBG_POWER, "\tSetting the device state from %s to %s\n",
                            DbgDevicePowerString(oldDeviceState),
                            DbgDevicePowerString(newDeviceState));
    //
    // Update our state.
    //
    fdoData->DevicePowerState = newDeviceState;

    if (newDeviceState == oldDeviceState) {

        DebugPrint(INFO, DBG_POWER, "\tNo Change in Power state\n");

        //
        // No change, there is just one thing we need to do before we leave.
        // The device may be queuing due to a prior QueryPower, and the Power
        // Manager sends a SetPower(S0->D0) command.
        //
        if (newDeviceState == PowerDeviceD0) {

            fdoData->QueueState = AllowRequests;
            PciDrvProcessQueuedRequests(fdoData);
        }

        PciDrvFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            Direction,
            STATUS_SUCCESS
            );

        goto Exit;
    }

    if (oldDeviceState == PowerDeviceD0) {

        //
        // We're about to go down, so queuing IRPs.
        //
        status = PciDrvPowerBeginQueuingIrps(
            DeviceObject,
            fdoData->PendingSIrp? 2 : 1, // one for the S-IRP and one for the D-IRP;
            FALSE           // Do not query for state change.
            );

        ASSERT(NT_SUCCESS(status));
    }

    //
    // Save or restore the state of the hardware depending on
    // whether your are going into a deeper sleep state or lighter.
    //     
    status = NICSetPower(fdoData, newDeviceState);
    ASSERTMSG("NICSetPower failed\n", status == STATUS_SUCCESS);
    
    
    //
    // Inform the kernel's power manager about the new state that this
    // device is in.
    //
    PoSetPowerState(DeviceObject, DevicePowerState, newState);


    //
    // This is why we are going to sleep (note - this field is not dependable
    // if the D state is PowerDeviceD0).
    //
    newDeviceAction = stack->Parameters.Power.ShutdownType;

    if (newDeviceState > oldDeviceState) {

        //
        // We are entering a deeper sleep state. Save away the appropriate
        // state and update our hardware. Check to see if the newDeviceAction
        // is for hibernation, shutdown or standby and take the appropriate
        // action.
        //
        if(newDeviceAction >= PowerActionShutdown){
            NICShutdown(fdoData);
        }        

    } 
    
    if (newDeviceState == PowerDeviceD0) {

        //
        // Our hardware is now on again. Here we empty our existing queue of
        // requests and let in new ones. Note that if this is a D0->D0 (ie
        // no change) we will unblock our queue, which may have been blocked
        // processing our Query-D IRP.
        //
        fdoData->QueueState = AllowRequests;
        PciDrvProcessQueuedRequests(fdoData);
        //
        // Restart the idle detection timer.
        //
        PciDrvReStartIdleDetectionTimer(fdoData);        
    }

    PciDrvFinalizeDevicePowerIrp(
        DeviceObject,
        Irp,
        Direction,
        status
        );

Exit:
    //
    // Cleanup before exiting from the worker thread.
    //
    IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
    ExFreePool((PVOID)Context);
    
}


NTSTATUS
PciDrvGetPowerPoliciesDeviceState(
    IN  PIRP                SIrp,
    IN  PDEVICE_OBJECT      DeviceObject,
    OUT DEVICE_POWER_STATE *DevicePowerState
    )
/*++

Routine Description:

    This routine retrieves the appropriate D state for a given S-IRP.

Arguments:

   SIRP - S-IRP to retrieve D-IRP for

   DeviceObject - pointer to a device object

   DevicePowerState - D-IRP to request for passed in S-IRP

Return Value:

   STATUS_SUCCESS if D-IRP is to be requested, failure otherwise.

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);
    SYSTEM_POWER_STATE  systemState = stack->Parameters.Power.State.SystemState;
    DEVICE_POWER_STATE  deviceState;
    ULONG               wakeSupported;

    //
    // Read the D-IRP out of the S->D mapping array we captured in QueryCap's.
    // We can choose deeper sleep states than our mapping (with the appropriate
    // caveats if we have children), but we can never choose lighter ones, as
    // what hardware stays on in a given S-state is a function of the
    // motherboard wiring.
    //
    // Also note that if a driver rounds down it's D-state, it must ensure that
    // such a state is supported. A driver can do this by examining the
    // DeviceD1 and DeviceD2 flags (Win2k, XP, ...), or by examining the entire
    // S->D state mapping (Win98). Check the PciDrvAdjustCapabilities routine.
    //

    if (systemState == PowerSystemWorking) {

        *DevicePowerState = PowerDeviceD0;
        return STATUS_SUCCESS;

    } else if (fdoData->WakeState != WAKESTATE_ARMED) {

        *DevicePowerState = PowerDeviceD3;
        return STATUS_SUCCESS;
    }

    //
    // We are armed for wake. Pick the best state that supports wakeup. First
    // check the S-state. If system-wake doesn't support wake, we'll fail it.
    // We don't do this check for shutdown or hibernate to make the users life
    // more pleasant.
    //
    if ((systemState <= PowerSystemSleeping3) &&
        (systemState > fdoData->DeviceCaps.SystemWake)) {

        //
        // Force an attempt to go to the next S-state.
        //
        return STATUS_UNSUCCESSFUL;
    }

    //
    // Pick the deepest D state that supports wake, but don't choose a state
    // lighter than that specified in the S->D mapping.
    //
    for(deviceState = PowerDeviceD3;
        deviceState >= fdoData->DeviceCaps.DeviceState[systemState];
        deviceState--) {

        switch(deviceState) {

            case PowerDeviceD0:
                wakeSupported = fdoData->DeviceCaps.WakeFromD0;
                break;

            case PowerDeviceD1:
                wakeSupported = fdoData->DeviceCaps.WakeFromD1;
                break;

            case PowerDeviceD2:
                wakeSupported = fdoData->DeviceCaps.WakeFromD2;
                break;

            case PowerDeviceD3:
                wakeSupported = fdoData->DeviceCaps.WakeFromD3;
                break;

            default:
                ASSERT(0);
                wakeSupported = FALSE;
                break;
        }

        if (wakeSupported) {

            *DevicePowerState = deviceState;
            return STATUS_SUCCESS;
        }
    }

    //
    // We exhausted our wakeable D-states. Fail the request unless we're
    // hibernating or shutting down.
    //
    if (systemState <= PowerSystemSleeping3) {

        return STATUS_UNSUCCESSFUL;
    }

    //
    // We won't bother waking the machine, we'll just do D3.
    //
    *DevicePowerState = PowerDeviceD3;
    return STATUS_SUCCESS;
}


NTSTATUS
PciDrvCanSuspendDevice(
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

Routine Description:

    This routine determines whether a device powerdown request should be
    succeeded or failed.

Arguments:

    DeviceObject - pointer to a device object.

Return Value:

    NTSTATUS code

--*/
{
    return STATUS_SUCCESS;
}


PCHAR
DbgPowerMinorFunctionString (
    UCHAR MinorFunction
    )
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



