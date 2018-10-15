/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Wake.C

Abstract:

    This module handles waitwake IRPs.

Author: Eliyas Yakub    Oct 2, 2001


Environment:

    Kernel mode only

Notes:


Revision History:

     Based on Adrian J. Oney's wait-wake boilerplate code 
     
--*/

#include "precomp.h"
#if defined(EVENT_TRACING)
#include "wake.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PciDrvDispatchWaitWake)
#pragma alloc_text (PAGE, PciDrvArmForWake)
#pragma alloc_text (PAGE, PciDrvDisarmWake)
#pragma alloc_text (PAGE, PciDrvAdjustCapabilities)
#pragma alloc_text (PAGE, PciDrvSetWaitWakeEnableState)
#pragma alloc_text (PAGE, PciDrvGetWaitWakeEnableState)
#pragma alloc_text (PAGE, PciDrvPassiveLevelReArmCallbackWorker)
#pragma alloc_text (PAGE, PciDrvPassiveLevelClearWaitWakeEnableState)
#endif


NTSTATUS
PciDrvDispatchWaitWake(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Routine Description:

   Processes IRP_MN_WAIT_WAKE.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    PFDO_DATA           fdoData;
    WAKESTATE           oldWakeState;
    PIO_STACK_LOCATION  stack;
    
    PAGED_CODE ();
    
    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvDispatchWaitWake\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    //
    // Let us make sure that somebody else in the stack is not
    // sending a bogus Wait-Wake or more than one wait-wake irp.
    //
    if(fdoData->WakeIrp != NULL || 
        !PciDrvCanWakeUpDevice(fdoData, stack->Parameters.WaitWake.PowerState)
        )
    {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        PciDrvIoDecrement(fdoData);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    fdoData->WakeIrp = Irp;

    //
    // Advance the state if the armed if we are to proceed
    //

    oldWakeState = InterlockedCompareExchange( (PULONG)&fdoData->WakeState,
                                            WAKESTATE_ARMED,
                                            WAKESTATE_WAITING );

    if (oldWakeState == WAKESTATE_WAITING_CANCELLED) {
        //
        // We got disarmed, finish up and complete the IRP
        //
        fdoData->WakeState = WAKESTATE_COMPLETING;
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        PciDrvIoDecrement(fdoData);
        return STATUS_CANCELLED;
    }

    NICConfigureForWakeUp(fdoData, TRUE); //Add pattern before sending wait-wake

    //
    // We went from WAITING to ARMED. Set a completion routine and forward
    // the IRP. Note that our completion routine might complete the IRP
    // asynchronously, so we mark the IRP pending
    //

    IoMarkIrpPending( Irp );
    IoCopyCurrentIrpStackLocationToNext( Irp );
    IoSetCompletionRoutine( Irp, PciDrvWaitWakeIoCompletionRoutine, NULL,
                                                        TRUE, TRUE, TRUE );
    PoCallDriver( fdoData->NextLowerDriver, Irp);

    PciDrvIoDecrement(fdoData);
    return STATUS_PENDING;
}


BOOLEAN
PciDrvArmForWake(
    IN PFDO_DATA    FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

Routine Description:

    Sends a wait-wake to the bus driver.  If the device is already armed
    for wakeup, this routine returns TRUE.

Arguments:

    FdoData - Points to device extension

    DeviceStateChange - TRUE if this function is being called by PnP. FALSE if
                        it is coming in via WMI or due to a rearm attempt.

Return Value:

    TRUE of FALSE

--*/
{
    NTSTATUS     status;
    WAKESTATE    oldWakeState;
    POWER_STATE  powerState;

    PAGED_CODE();

    if(!IsPoMgmtSupported(FdoData)){
        return FALSE;
    }

    DebugPrint(TRACE, DBG_POWER, "--> ArmForWake\n");
    
    //
    // Grab disable/enable lock. The request for arm/disarm could be
    // called in the context of an user thread. So to prevent somebody
    // from suspending the thread while we are holding this PASSIVE_LEVEL
    // lock, we should call KeEnterCriticalRegion. This call disables normal
    // kernel APC from being delivered, which is used in suspending a thread.
    // This step is required to prevent possible deadlocks.
    //
    KeEnterCriticalRegion();
    status = KeWaitForSingleObject( &FdoData->WakeDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    ASSERT(NT_SUCCESS(status)); 
    
    //
    // If this is coming in response to a PnP state change, allow current and
    // future attempts to arm. Until now, all wait-wake arm attempts would be
    // failed.
    //
    if (DeviceStateChange) {

        FdoData->AllowWakeArming = TRUE;
    }

    //
    // Only arm if we aren't removed/stopped/etc, and the registry allows it.
    //
    if (!(FdoData->AllowWakeArming && PciDrvGetWaitWakeEnableState(FdoData))) {

        //
        // Wake not enabled
        //
        status = STATUS_UNSUCCESSFUL;

    } else {

        oldWakeState = InterlockedCompareExchange((PULONG) &FdoData->WakeState,
                                                   WAKESTATE_WAITING,
                                                   WAKESTATE_DISARMED );

        if (oldWakeState != WAKESTATE_DISARMED) {

            //
            // Already armed or in the process of arming/rearming.
            //
            status = STATUS_SUCCESS;

        } else {

            //
            // Our dispatch handler is now waiting for IRP_MN_WAIT_WAKE.
            // The state just got moved to WAKESTATE_WAITING.
            //
            KeClearEvent(&FdoData->WakeCompletedEvent);

            //
            // Specify the least-powered system power state from which 
            // the device can wake the system.
            //
            powerState.SystemState = FdoData->DeviceCaps.SystemWake;

            //
            // Request the power IRP, STATUS_PENDING is success
            //
            status = PoRequestPowerIrp( FdoData->UnderlyingPDO,
                                        IRP_MN_WAIT_WAKE,
                                        powerState,
                                        PciDrvWaitWakePoCompletionRoutine,
                                        (PVOID)FdoData,
                                        NULL );

            if (!NT_SUCCESS(status)) {

                FdoData->WakeState = WAKESTATE_DISARMED;

                KeSetEvent( &FdoData->WakeCompletedEvent,
                            IO_NO_INCREMENT,
                            FALSE );
            }
        }
    }

    //
    // Unlock enable/disable logic
    //
    KeSetEvent( &FdoData->WakeDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    KeLeaveCriticalRegion();

    DebugPrint(TRACE, DBG_POWER, "<-- ArmForWake\n");

    return (status == STATUS_PENDING ? TRUE:FALSE);
}



NTSTATUS
PciDrvWaitWakeIoCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

Routine Description:

    Completion routine for the WaitWake irp set by the PciDrvDispatchWaitWake handler

Arguments:


Return Value:

    NT status code

--*/
{
    PFDO_DATA               fdoData;
    WAKESTATE       oldWakeState;

    DebugPrint(TRACE, DBG_POWER, "Entered WaitWakeIoCompletionRoutine\n");

    fdoData  = (PFDO_DATA) DeviceObject->DeviceExtension;

    //
    // Advance the state to completing
    //
    oldWakeState = InterlockedExchange( (PULONG)&fdoData->WakeState,
                                                    WAKESTATE_COMPLETING );
    if (oldWakeState == WAKESTATE_ARMED) {
        //
        // Normal case, IoCancelIrp isn't being called. Note that we already
        // marked the IRP pending in our dispatch routine
        //
        return STATUS_CONTINUE_COMPLETION;
    } else {

        ASSERT(oldWakeState == WAKESTATE_ARMING_CANCELLED);
        //
        // IoCancelIrp is being called RIGHT NOW. The disarm code will try
        // to put back the WAKESTATE_ARMED state. It will then see our
        // WAKESTATE_COMPLETED value, and complete the IRP itself!
        //
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
}


VOID
PciDrvDisarmWake(
    IN PFDO_DATA    FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

Routine Description:

    Cancels an already issued wait-wake IRP.

Arguments:

    FdoData - Points to device extension

    DeviceStateChange - TRUE if this function is being called by PnP. FALSE if
                        it is coming in via WMI or due to a rearm attempt.

Return Value:

   VOID

--*/
{
    WAKESTATE       oldWakeState;
    NTSTATUS        status;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_POWER, "--> DisarmWake\n");

    //
    // Grab disable/enable lock.
    //
    KeEnterCriticalRegion(); 
    status = KeWaitForSingleObject( &FdoData->WakeDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    ASSERT(NT_SUCCESS(status)); 

    //
    // If this is a state change from PnP, our device is going offline. As such,
    // any arming attempts should be failed until Pnp reactivates us.
    //
    if (DeviceStateChange) {

        FdoData->AllowWakeArming = FALSE;
    }

    //
    // Go from WAKESTATE_WAITING to WAKESTATE_WAITING_CANCELLED, or
    //         WAKESTATE_ARMED   to WAKESTATE_ARMING_CANCELLED, or
    // stay in WAKESTATE_DISARMED or WAKESTATE_COMPLETING
    //
    oldWakeState = InterlockedOr( (PULONG)&FdoData->WakeState, 1 );

    if (oldWakeState == WAKESTATE_ARMED) {

        DebugPrint(TRACE, DBG_POWER, "canceling wakeIrp\n");

        IoCancelIrp( FdoData->WakeIrp );

        //
        // Now that we've cancelled the IRP, try to give back ownership
        // to the completion routine by restoring the WAKESTATE_ARMED state
        //

        oldWakeState = InterlockedCompareExchange((PULONG) &FdoData->WakeState,
                                                   WAKESTATE_ARMED,
                                                   WAKESTATE_ARMING_CANCELLED );
        if (oldWakeState == WAKESTATE_COMPLETING) {

            //
            // We didn't give back control of IRP in time, so we own it now.
            //
            IoCompleteRequest( FdoData->WakeIrp, IO_NO_INCREMENT );

        }
    }

    //
    // Wait for IRP to complete.
    //
    status = KeWaitForSingleObject( &FdoData->WakeCompletedEvent,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );    
    ASSERT(NT_SUCCESS(status)); 

    //
    // Unlock enable/disable logic
    //
    KeSetEvent( &FdoData->WakeDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    KeLeaveCriticalRegion();

    DebugPrint(TRACE, DBG_POWER, "<-- DisarmWake\n");
    
}


VOID
PciDrvWaitWakePoCompletionRoutine(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  UCHAR               MinorFunction,
    IN  POWER_STATE         State,
    IN  PVOID               Context,
    IN  PIO_STATUS_BLOCK    IoStatus
    )
/*++

Routine Description:

    Completion routine for the wait-wake IRP set by PoRequestPowerIrp

Arguments:


Return Value:

   VOID

--*/
{
    PFDO_DATA               fdoData;

    DebugPrint(TRACE, DBG_POWER, "WaitWakePoCompletionRoutine  \n");

    fdoData  = (PFDO_DATA) Context;

    //
    // Restore state (old state will have been completing)
    //
    fdoData->WakeState = WAKESTATE_DISARMED;
    fdoData->WakeIrp = NULL;

    NICConfigureForWakeUp(fdoData, FALSE); //remove pattern

    //
    // Adjust synchronization event so another Wait-Wake can get queued
    //
    KeSetEvent( &fdoData->WakeCompletedEvent,
                IO_NO_INCREMENT,
                FALSE );

    if (NT_SUCCESS(IoStatus->Status)) {

        //
        // The system has woken up and completed the wait-wake IRP successfully
        // so let us check the registry again for user-preference and rearm for
        // wait-wake if required.  Since the routines for arming the wait-wake
        // can only be called at PASSIVE_LEVEL, we will queue
        // a workitem to do that because this completion routine could be
        // running at DISPATCH_LEVEL.
        //
        PciDrvQueuePassiveLevelCallback(
                                    fdoData,
                                    PciDrvPassiveLevelReArmCallbackWorker,
                                    NULL, NULL);
        
    } else if (IoStatus->Status == STATUS_UNSUCCESSFUL || 
                        IoStatus->Status == STATUS_NOT_IMPLEMENTED ||
                        IoStatus->Status == STATUS_NOT_SUPPORTED) {
        //
        // The bus driver is not capable of wait-waking the system. So let us
        // queue a workitem to clear the registry. This would serve as a feedback to
        // the user as to whether or not their device is armed for wake..
        //
        PciDrvQueuePassiveLevelCallback(
                                    fdoData,
                                    PciDrvPassiveLevelClearWaitWakeEnableState,
                                     NULL, NULL);
    }            

    return;
}



VOID
PciDrvPassiveLevelReArmCallbackWorker(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
    )
/*++

Routine Description:

    This is callback worker routine called to put the device into D0 state
    and rearm the device if the wake is enabled in the registry. How to
    safely request D0 Irp will be shown in the next version. Take a look
    at the KeyboardClassWaitWakeComplete in the kbdclass sample to get
    an idea on how this should be done.

Arguments:

   DeviceObject - pointer to a device extenion.

   Context - Pointer to workitem
   
Return Value: VOID

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_POWER, "Entered PciDrvPassiveLevelReArmCallbackWorker\n");

    PciDrvPowerUpDevice(fdoData, TRUE);
    
    //
    // Let us rearm for wake if enabled in the registry.
    //
    PciDrvArmForWake(fdoData, FALSE);

    if(Context) {        
        IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
        ExFreePool((PVOID)Context);
    }
}

VOID
PciDrvPassiveLevelClearWaitWakeEnableState(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
    )
/*++

Routine Description:

    This is callback worker routine called to clear the wait wake set 
    in the registry.

Arguments:

   DeviceObject - pointer to a device extenion.

   Context - Pointer to workitem context
   
Return Value: VOID

--*/
{
    PFDO_DATA fdoData = DeviceObject->DeviceExtension;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_POWER, 
        "Entered PciDrvPassiveLevelClearWaitWakeEnableState\n");

    PciDrvSetWaitWakeEnableState(fdoData, FALSE);

    if(Context) {
        IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
        ExFreePool((PVOID)Context);
    }
}


BOOLEAN
PciDrvCanWakeUpDevice(
    IN PFDO_DATA            FdoData,
    IN SYSTEM_POWER_STATE   PowerState
    )
/*++

Routine Description:

    This function is used to find out whether we are capable of waking
    the system for the PowerState reported in the WakeIrp. Since we
    generated the Wake Irp, we know it will be consistent. We just 
    want to make sure out power states in the DeviceCaps are consistent
    with each other.
    
Arguments:


Return Value:

   TRUE or FALSE

--*/
{
    BOOLEAN             canWakeUp = FALSE;
    DEVICE_POWER_STATE  dState;
    
    //
    // Are we indeed capable of waking the system from the PowerState 
    // reported by the Wait-Wake?
    //

    dState = FdoData->DeviceCaps.DeviceState[PowerState];
    
    #pragma warning(disable:4242 4244) 
    switch(dState) {
        
    case PowerDeviceD0:
        canWakeUp = FdoData->DeviceCaps.WakeFromD0;
        break;
    case PowerDeviceD1:
        canWakeUp = FdoData->DeviceCaps.WakeFromD1;
        break;
    case PowerDeviceD2:
        canWakeUp = FdoData->DeviceCaps.WakeFromD2;
        break;
    case PowerDeviceD3:
        canWakeUp = FdoData->DeviceCaps.WakeFromD3;
        break;
    default: 
        canWakeUp = FALSE;
        break;
    }
    
    #pragma warning(default:4242 4244) 
    
    return canWakeUp;    
}



VOID
PciDrvAdjustCapabilities(
    IN OUT PDEVICE_CAPABILITIES DeviceCapabilities
    )
/*++

Routine Description:

   This routine updates the DeviceD1, DeviceD2, DeviceWake, SystemWake, and
   WakeFromDx fields. None of this should be needed if our bus driver was
   coded to the WDM 1.10 spec. However, some bus drivers are coded to the
   WDM 1.00 spec, and don't fill in the DeviceDx and WakeFromDx fields.

   This routine should be invoked from the IRP_MN_QUERY_CAPS handler on the way
   up to adjust capabilities of the device.

Arguments:


Return Value:

   NT status code

--*/
{
    DEVICE_POWER_STATE  dState;
    SYSTEM_POWER_STATE  sState;
    DEVICE_POWER_STATE  deepestDeviceWakeState;
    int i;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_POWER, "AdjustCapabilities  \n");

    //
    // Infer the DeviceD1 and DeviceD2 fields from the S->D state mapping.
    //
    for(sState=PowerSystemSleeping1; sState<=PowerSystemHibernate; sState++) {

        if (DeviceCapabilities->DeviceState[sState] == PowerDeviceD1) {

            DeviceCapabilities->DeviceD1 = TRUE;

        } else if (DeviceCapabilities->DeviceState[sState] == PowerDeviceD2) {

            DeviceCapabilities->DeviceD2 = TRUE;
        }
    }

    //
    // Examine DeviceWake, then SystemWake, and update the Wake/D-state bits
    // appropriately. This is for those older WDM 1.0 bus drivers that don't
    // bother to set the DeviceDx and WakeFromDx fields.
    //
    dState = DeviceCapabilities->DeviceWake;

    for(i=0; i<2; i++) {
        switch(dState) {
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

        if (DeviceCapabilities->SystemWake != PowerSystemUnspecified) {

            dState = 
               DeviceCapabilities->DeviceState[DeviceCapabilities->SystemWake];
        } else {

            dState = PowerDeviceUnspecified;
        }
    }

    //
    // Find the deepest D state for wake
    //
    if (DeviceCapabilities->WakeFromD3) {

        deepestDeviceWakeState = PowerDeviceD3;

    } else if (DeviceCapabilities->WakeFromD2) {

        deepestDeviceWakeState = PowerDeviceD2;

    } else if (DeviceCapabilities->WakeFromD1) {

        deepestDeviceWakeState = PowerDeviceD1;

    } else if (DeviceCapabilities->WakeFromD0) {

        deepestDeviceWakeState = PowerDeviceD0;

    } else {

        deepestDeviceWakeState = PowerDeviceUnspecified;
    }

    //
    // Now fill in the SystemWake field. If this field is unspecified, then we
    // should infer it from the D-state information.
    //
    sState = DeviceCapabilities->SystemWake;

    if (sState != PowerSystemUnspecified) {

        //
        // The D-state for SystemWake should provide enough power to cover the deepest
        // device wake state we've found.
        //
        ASSERT(DeviceCapabilities->DeviceState[sState] <= deepestDeviceWakeState);

    } else if (deepestDeviceWakeState != PowerDeviceUnspecified) {

        //
        // A system wake state wasn't specified, examine each S state and pick
        // the first one that supplies enough power to wake the system. Note
        // that we start with S3. If a driver doesn't set the SystemWake field
        // but can wake the system from D3, we do *not* assume the driver can
        // wake the system from S4 or S5.
        //
        for(sState=PowerSystemSleeping3; sState>=PowerSystemWorking; sState--) {

            if ((DeviceCapabilities->DeviceState[sState] != PowerDeviceUnspecified) &&
                (DeviceCapabilities->DeviceState[sState] <= deepestDeviceWakeState)) {
                break;
            }
        }

        //
        // If we didn't find a state, sState is PowerSystemUnspecified.
        //
        DeviceCapabilities->SystemWake = sState;
    }
}


NTSTATUS
PciDrvSetWaitWakeEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN WakeState
    )
/*++

Routine Description:

   Sets wakestate in the device registry.
   Called in response to WMI set WMI_POWER_DEVICE_WAKE_ENABLE requests.

Arguments:


Return Value:

   NT status code

--*/
{
    PAGED_CODE();

    if(!PciDrvWriteRegistryValue(FdoData,
                                    PCIDRV_WAIT_WAKE_ENABLE, 
                                    (ULONG)WakeState)) 
    {
                                    
        DebugPrint(TRACE, DBG_POWER, "PciDrvWriteRegistryValue failed\n");
        return STATUS_UNSUCCESSFUL;
    }
    
    return STATUS_SUCCESS;
}


BOOLEAN
PciDrvGetWaitWakeEnableState(
    IN PFDO_DATA   FdoData
    )
/*++

Routine Description:

   Gets wakestate from the device registry.
   Called in response to WMI query WMI_POWER_DEVICE_WAKE_ENABLE requests.

Arguments:


Return Value:

   TRUE if enabled
   FALSE if not present/disabled/error in reading registry

--*/
{
    ULONG waitWakeEnabled;

    PAGED_CODE();

    PciDrvReadRegistryValue(FdoData,
                                PCIDRV_WAIT_WAKE_ENABLE, 
                                &waitWakeEnabled);
    
    DebugPrint(TRACE, DBG_POWER, "Wait-Wake is%s enabled\n",
                waitWakeEnabled ? "":" not");

    return (BOOLEAN)waitWakeEnabled;

}

