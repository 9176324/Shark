/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Idle.C (Idle detection)

Abstract:

    This module is used to detect whether the device is idle and if so
    put the device into sleep to save power. The device will be put
    into a D-state from where it can wakeup when the hardware wants to
    interact with the system. So the ability to Wait-wake the device is 
    pre-requiste to do idle detection in our case. This may not be true
    for other hardware if it's not capable of waking due to external event,
    and the device is powered up only when the system wants to talk to
    the device.
    

Author:    Eliyas Yakub  March 18, 2003


Environment:

    Kernel mode only

Notes:


Revision History:


--*/
#include "precomp.h"
#include <ntpoapi.h> // required for ZwQueryPowerInformation


#if defined(EVENT_TRACING)
#include "idle.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PciDrvRegisterForIdleDetection)
#pragma alloc_text (PAGE, PciDrvDeregisterIdleDetection)
#pragma alloc_text (PAGE, PciDrvPowerDownDeviceCallback)
#pragma alloc_text (PAGE, PciDrvPowerUpDeviceCallback)
#pragma alloc_text (PAGE, PciDrvReStartIdleDetectionTimer)
#pragma alloc_text (PAGE, PciDrvCancelIdleDetectionTimer)
#pragma alloc_text (PAGE, PciDrvSetPowerSaveEnableState)
#pragma alloc_text (PAGE, PciDrvGetPowerSaveEnableState)
#endif


VOID
PciDrvRegisterForIdleDetection( 
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

Routine Description: 

    Register for idle detection to save power. This routine is called
    1) From the StartDevice.
    2) From WMI callback routines, every time the user changes the settings
       in the Device Manager Power Management Tab.
    3) From cancel-remove pnp handler.

Arguments:

    FdoData - pointer to a FDO_DATA structure


Return Value:


--*/

{
    NTSTATUS status;
    
    PAGED_CODE();

    DebugPrint (TRACE, DBG_POWER, 
                            "--> PciDrvRegisterForIdleDetection\n");

    //
    // Grab disable/enable lock. The request for arm/disarm could be
    // called in the context of an user thread. So to prevent somebody
    // from suspending the thread while we are holding this PASSIVE_LEVEL
    // lock, we should call KeEnterCriticalRegion. This call disables normal
    // kernel APC from being delivered, which is used in suspending a thread.
    // This step is required to prevent possible deadlocks.
    //
    
    KeEnterCriticalRegion();

    DebugPrint (TRACE, DBG_LOCKS, 
                    "Acquiring  PowerSaveDisableEnableLock\n");
    
    status = KeWaitForSingleObject( &FdoData->PowerSaveDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    ASSERT(NT_SUCCESS(status)); 

    //
    // In order to use ConservationIdleTime or PerformanceIdleTime for
    // our custom idle detection, we need to know the current power
    // source and also we need to be notified whenever the power source
    // changes.
    //
    PciDrvRegisterPowerStateNotification(FdoData);   
    
    //
    // If this is coming in response to a PnP state change, allow current and
    // future attempts to register for idle detection. Until then, all attempts
    // would be failed.
    //
    
    if (DeviceStateChange) {

        FdoData->AllowIdleDetectionRegistration = TRUE; 
    }

    //
    // If it's okay to idle from PNP perspective and if the user made the choice
    // to idle, then do it.
    //
    if(FdoData->AllowIdleDetectionRegistration &&
                 PciDrvGetPowerSaveEnableState(FdoData)) {

        //
        // Did we initialize all the resources required to idle detection?
        //
        if(!FdoData->IdleDetectionEnabled){                
            //
            // This is our first valid attempt to register. 
            //
            KeInitializeTimer(&FdoData->IdleDetectionTimer);
            
            KeInitializeDpc(&FdoData->IdleDetectionTimerDpc, 
                                PciDrvIdleDetectionTimerDpc, 
                                FdoData);
            //
            // This event will be signalled whenever the D0 IRP
            // requested by us completes. This event along with 
            // IdlePowerUpRequested flag enables us to prevent 
            // requesting multiple requests to power up the device
            // and avoid any race conditions with PNP Irps.
            //
            KeInitializeEvent(&FdoData->IdlePowerUpCompleteEvent,
                                        NotificationEvent, FALSE);
            FdoData->IdlePowerUpRequested = FALSE;
            //
            // This event will be signalled whenever a D IRP
            // requested to power down completes. 
            //
            KeInitializeEvent(&FdoData->IdlePowerDownCompleteEvent,
                                        NotificationEvent, FALSE);            
            
            FdoData->IdleDetectionEnabled = TRUE;
        }
    
        //
        // Queue idle detection timer DPC.
        //         
        PciDrvSetIdleTimer(FdoData);
     } 

    //
    // Unlock enable/disable logic
    //
    KeSetEvent( &FdoData->PowerSaveDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    KeLeaveCriticalRegion();

    DebugPrint (TRACE, DBG_POWER, "<-- PciDrvRegisterForIdleDetection\n");
    
}

VOID 
PciDrvDeregisterIdleDetection(
    IN PFDO_DATA   FdoData,
    IN BOOLEAN      DeviceStateChange
    )
/*++

Routine Description:

Arguments:

    FdoData - pointer to a FDO_DATA structure


Return Value:


--*/

{
    NTSTATUS status;
    
    PAGED_CODE();

    DebugPrint (TRACE, DBG_POWER, "--> PciDrvDeregisterIdleDetection\n");

    KeEnterCriticalRegion();
    
    DebugPrint (TRACE, DBG_LOCKS, "Acquiring  PowerSaveDisableEnableLock\n");    
    status = KeWaitForSingleObject( &FdoData->PowerSaveDisableEnableLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );
    
    ASSERT(NT_SUCCESS(status));

    PciDrvUnregisterPowerStateNotification(FdoData);   
    
    
    //
    // If this is a state change from PnP, our device is going offline. 
    // As such, any attempts to register for idle detection should be  
    // failed until Pnp reactivates us.
    //
    if (DeviceStateChange) {

        FdoData->AllowIdleDetectionRegistration = FALSE;
    }
    
    if (FdoData->IdleDetectionEnabled) {

        FdoData->IdleDetectionEnabled = FALSE;
        if(KeCancelTimer(&FdoData->IdleDetectionTimer)){
            //
            // We are able to remove the timer from the timer queue.
            // So let us remove the reference we took when we set 
            // the timer ourself.
            //
            PciDrvIoDecrement(FdoData);
        } 
    }
    else {   // Bogus call to Deregister idle detection
    }

    //
    // Unlock enable/disable logic
    //
    KeSetEvent( &FdoData->PowerSaveDisableEnableLock,
                IO_NO_INCREMENT,
                FALSE );

    KeLeaveCriticalRegion();

    DebugPrint (TRACE, DBG_POWER, "<-- PciDrvDeregisterIdleDetection\n");
    
}

VOID
PciDrvSetIdleTimer(
    IN PFDO_DATA FdoData
    )
/*++
    
Routine Description:

    This function queues a timer DPC to see if the device
    has been idle for certain period of time. If the timer
    DPC is already in queue, KeSetTimer implictly resets the 
    the expiration time to new one.
    
Arguments:

    FdoData - pointer to a device object.

Return Value:

    VOID
    
--*/
{
    LARGE_INTEGER delay;

    DebugPrint (TRACE, DBG_POWER, "Set Idle timer\n");

    FdoData->IsDeviceIdle = TRUE;

    if(FdoData->RunningOnBattery) {
        //
        // While running on battery, our focus should be in saving power as
        // much as possible.
        //
        delay.QuadPart = FdoData->ConservationIdleTime;
    
    } else { //Running on AC
        //
        // While running on AC, our focus should be in running faster 
        //
        delay.QuadPart = FdoData->PerformanceIdleTime;

    }
    //
    // This reference is to ensure that DPC has completed execution
    // before allowing the driver to unload.
    //
    PciDrvIoIncrement(FdoData);
    
    KeSetTimer(&FdoData->IdleDetectionTimer, 
                    delay, 
                    &FdoData->IdleDetectionTimerDpc);
    
    return;
}


VOID
PciDrvReStartIdleDetectionTimer(
    IN PFDO_DATA FdoData
    )
/*++
    
Routine Description:

    Called to restart the timer when the device powers back
    up.
    
Arguments:

    FdoData - pointer to a device object.

Return Value:

    VOID
    
--*/
{   
    PAGED_CODE();
    
    if(FdoData->IdleDetectionEnabled){
        PciDrvSetIdleTimer(FdoData);        
    }
}


VOID
PciDrvCancelIdleDetectionTimer(
    IN PFDO_DATA FdoData
    )
/*++
    
Routine Description:

    Called to cancel the timer when the device goes in to
    low power state.
    
Arguments:

    FdoData - pointer to a device object.

Return Value:

    VOID
    
--*/
{
    PAGED_CODE();
    
    if(FdoData->IdleDetectionEnabled){
        
        if(KeCancelTimer(&FdoData->IdleDetectionTimer)){
            //
            // We are able to remove the timer from the timer queue.
            // So let us remove the reference we took when we set 
            // the timer ourself.
            //
            PciDrvIoDecrement(FdoData);
        } 
    }
    return;
}

VOID
PciDrvIdleDetectionTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
/*++

Routine Description:

    DPC for the idle timer.  We will queue a work item which will send down
    a power down request to the device if there is no work to do.

Arguments:
    Dpc - Timer DPC
    DeferredContext - pointer to a FDO_DATA structure    
    SystemContext1 - Ignored
    SystemContext2 - Ignored

Return Value:
    None

  --*/
{
    PFDO_DATA       fdoData = (PFDO_DATA)DeferredContext;

    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);
    
    DebugPrint(LOUD, DBG_POWER, 
                        "-->PciDrvIdleDetectionTimerDpc %p\n", fdoData);

    do{
        //
        // If the idle detection is cancelled or the device is removed,
        // exit from the DPC without restarting the timer.
        //
        if(fdoData->IdleDetectionEnabled == FALSE ||
            Deleted == fdoData->DevicePnPState)
        {
            break;
        }

        //
        // If the OutStandingIoCount is greater that Base+1, then
        // we know some I/Os are in progress.
        //
        if(PciDrvGetOutStandingIoCount(fdoData) > 3 || !fdoData->IsDeviceIdle){
            //
            // I/O in progress or an I/O completed just a while ago.
            // So just restart the timer and check later to see if the
            // device remains idle for the stipulated time period
            // (PCIDRV_IDLE_TIMEOUT).
            //       
            PciDrvSetIdleTimer(fdoData);
            break;
        }

        if(fdoData->DevicePowerState == PowerDeviceD0){
            //
            // Queue a workitem to power down the device at PASSIVE_LEVEL.
            // The idle detection timer will be re-activated when we power
            // back up.
            //            
            PciDrvQueuePassiveLevelCallback(fdoData, 
                                    PciDrvPowerDownDeviceCallback,
                                    NULL, NULL);
            //
            // If the D-IRP fails for any reason either in our driver or some-
            // where else, we will restart the IDLE timer in the IRP completion 
            // routine to try again.
            //
            
            break;
        }
    
    }while(FALSE);

    PciDrvIoDecrement(fdoData);

    DebugPrint(LOUD, DBG_POWER, 
                        "<--PciDrvIdleDetectionTimerDpc %p\n", fdoData);    
    return;
    
}

VOID
PciDrvCompletionOnIdlePowerUpIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID                       PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    )
/*++

Routine Description:

   Completion routine for D0-IRP.

Arguments:


Return Value:

   NT status code

--*/
{
    PFDO_DATA       fdoData = (PFDO_DATA)DeviceObject->DeviceExtension;
    
    DebugPrint(TRACE, DBG_POWER, 
                        "Entered PciDrvCompletionOnIdlePowerUpIrp\n");

    ASSERT((PKEVENT)PowerContext == &fdoData->IdlePowerUpCompleteEvent);

    fdoData->IdlePowerUpRequested = FALSE;

    //
    // Set the event to signal all the threads waiting for the device to 
    // power up in PciDrvPowerUpDevice routine.
    //
    KeSetEvent(&fdoData->IdlePowerUpCompleteEvent, IO_NO_INCREMENT, FALSE);

    return;
}

VOID
PciDrvPowerUpDeviceCallback(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
)
/*++

Routine Description:

   Worker routine to power up the device. This routine is called
   either as a workitem callback (queued by PciDrvPowerUpDevice)
   or directly by PciDrvPowerUpDevice depending on whether the
   thread is running at DISPATCH_LEVEL or not.

Arguments:

    DeviceObject - pointer to a device object.

    Context - pointer to workitem context.

Return Value:

   NT status code

--*/
{

    NTSTATUS        status;
    POWER_STATE     powerState;
    PFDO_DATA       fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_POWER, "--> PciDrvPowerUpDeviceCallback\n");

    PAGED_CODE();
    
    fdoData->IdlePowerUpRequested = TRUE;

    KeClearEvent(&fdoData->IdlePowerUpCompleteEvent);
            
    powerState.DeviceState = PowerDeviceD0;

    status = PoRequestPowerIrp(fdoData->Self, 
                            IRP_MN_SET_POWER, 
                            powerState, 
                            PciDrvCompletionOnIdlePowerUpIrp, 
                            &fdoData->IdlePowerUpCompleteEvent, 
                            NULL);
    if(!NT_SUCCESS(status)) {
    
        fdoData->IdlePowerUpRequested = FALSE;    
        //
        // Set the event to signal all the threads waiting for the device to 
        // power up in PciDrvPowerUpDevice routine.
        //
        KeSetEvent(&fdoData->IdlePowerUpCompleteEvent, IO_NO_INCREMENT, FALSE);
    } else {
        //
        // Wait for it to complete.
        //
        status = KeWaitForSingleObject(&fdoData->IdlePowerUpCompleteEvent, 
                                        Executive, KernelMode, FALSE, NULL);        
        ASSERT(NT_SUCCESS(status)); 
    }
    
    if(Context) {
        IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
        ExFreePool((PVOID)Context);
        
    }
                       
    DebugPrint(TRACE, DBG_POWER, "<-- PciDrvPowerUpDeviceCallback\n");
    
    return;
}


NTSTATUS
PciDrvPowerUpDevice(
    IN PFDO_DATA   FdoData,
    IN BOOLEAN     Wait
)
/*++
    
Routine Description:

    This routine is called to power up the device from 
    various dispatch rotuines. If it's called from any I/O
    dispatch routines, the Wait argument should be
    set to FALSE, because those routines can be called at
    DISPATCH_LEVEL. If the routine is called from a PNP
    dispatch handler, Wait is set to TRUE. 

    If the Wait argument is set to TRUE, this routine
    waits for the requested power up to complete before 
    returning. Otherwise it queues a workitem to request a 
    power IRP and returns immediately.
    
Arguments:

    FdoData - pointer to a device object.
    Wait   - TRUE if the caller want to wait for the power
             up request to completed. Must be called
             at PASSIVEL_LEVEL
             FALSE if the caller doesn't want to wait.


Return Value:

   NT status code
    
--*/
{
    PVOID       waitObjects[2]; // one for power up and one for power down
    NTSTATUS    status = STATUS_SUCCESS;
    
    //DebugPrint (LOUD, DBG_POWER, "--> PciDrvPowerUpDevice\n");

    //
    // Make sure the Wait is not set to True when the CurrentIrql is 
    // is DISPATCH_LEVEL. Since KeGetCurrentIrql is an expensive operation
    // and this function is called for every I/O, we will do this
    // check only on a checked build driver and hope we will never
    // hit this in a free build system.
    //
    ASSERT(!(Wait && KeGetCurrentIrql() == DISPATCH_LEVEL));
    
    if(FdoData->IdleDetectionEnabled && 
            FdoData->DevicePnPState == Started){
        //
        // Tell the idle detection logic that you are busy.
        //
        PciDrvSetDeviceBusy(FdoData);
        
        if(FdoData->IdlePowerUpRequested)
        {
            //
            // If Wait is true, we will wait for the IRPs to drain, else
            // we will return. This causes PNP/WMI IRPs to wait and I/O irps
            // to get queued or flushed depending on how far the power IRPs
            // have gone thru in changing the QueueState.
            //
            if(Wait){
                
                DebugPrint (TRACE, DBG_POWER, 
                   "Waiting for the previously generated power irp to complete\n");
                
                waitObjects[0] = &FdoData->IdlePowerUpCompleteEvent;
                waitObjects[1] = &FdoData->IdlePowerDownCompleteEvent;
                
                status = KeWaitForMultipleObjects(2,
                                           waitObjects,
                                           WaitAll,
                                           Executive,
                                           KernelMode,
                                           FALSE,
                                           (PLARGE_INTEGER) NULL,
                                           (PKWAIT_BLOCK) NULL
                                           );
                if(!NT_SUCCESS(status)){
                    ASSERT(NT_SUCCESS(status));                    
                    DebugPrint (ERROR, DBG_POWER, 
                                        "KeWaitForMultipleObjects %x", status);
                    goto End;
                }
            }else {
                //
                // This has to be an I/O request. It will be queued or processed
                // depending on the queue state, when we return from here. If 
                // it's queued, it will be processed at the end of power-up
                // sequence.
                //
                goto End;
            }
        }
        
        //
        // Make a request to power up the device only if we are sleeping and
        // we haven't received a system request to sleep.
        //        
        if(FdoData->SystemPowerState == PowerSystemWorking && 
                FdoData->DevicePowerState > PowerDeviceD0) {

            if(KeGetCurrentIrql() == DISPATCH_LEVEL) {
                //
                // We will queue a workitem to power up the device at passive-level.
                //                
                status = PciDrvQueuePassiveLevelCallback(FdoData, 
                                PciDrvPowerUpDeviceCallback, NULL, NULL );                        
            } else {
                //
                // If called at PASSIVE_LEVEL, we will wait for the D0-IRP to
                // complete before returning.
                //
                PciDrvPowerUpDeviceCallback(FdoData->Self, NULL);
            }
        }        
        
    }

End:
    
    //DebugPrint (LOUD, DBG_POWER, "<-- PciDrvPowerUpDevice\n");

    return status;
    
}


VOID
PciDrvCompletionOnIdlePowerDownIrp(
    IN  PDEVICE_OBJECT              DeviceObject,
    IN  UCHAR                       MinorFunction,
    IN  POWER_STATE                 PowerState,
    IN  PVOID                       PowerContext,
    IN  PIO_STATUS_BLOCK            IoStatus
    )
/*++

Routine Description:

   Completion routine for Power down D-IRP.

Arguments:


Return Value:

   NT status code

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA)DeviceObject->DeviceExtension;
    
    DebugPrint(TRACE, DBG_POWER, 
                        "Entered PciDrvCompletionOnIdlePowerDownIrp\n");
    //
    // If the request to power down failed for any reason, restart the idle
    // timer. 
    //
    if(!NT_SUCCESS(IoStatus->Status) && 
            Deleted != fdoData->DevicePnPState){        
            PciDrvSetIdleTimer(fdoData);            
    }
    
    ASSERT((PKEVENT)PowerContext == &fdoData->IdlePowerDownCompleteEvent);

    //
    // Set the event to signal all the threads waiting for the device to 
    // power down in PciDrvPowerUpDevice routine.
    //
    KeSetEvent(&fdoData->IdlePowerDownCompleteEvent, IO_NO_INCREMENT, FALSE);
    
}

VOID
PciDrvPowerDownDeviceCallback(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
)
/*++

Routine Description:

   WorkItem callback routine to power down the device. The
   workitem is queued by PciDrvIdleDetectionTimerDpc.
   

Arguments:

    DeviceObject - pointer to a device object.

    Context - pointer to workitem context.


Return Value:

   NT status code

--*/
{

    NTSTATUS    status;
    POWER_STATE powerState;
    PFDO_DATA   fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    
    DebugPrint(TRACE, DBG_POWER, "--> PciDrvPowerDownDevice\n");

    PAGED_CODE();
   
    do{

        //
        // This should be the deepest device state from where
        // the device can wake the system.
        //
        powerState.DeviceState = fdoData->DeviceCaps.DeviceWake;
        
        KeClearEvent(&fdoData->IdlePowerDownCompleteEvent);
        
        status = PoRequestPowerIrp(fdoData->Self, 
                                IRP_MN_SET_POWER, 
                                powerState, 
                                PciDrvCompletionOnIdlePowerDownIrp, 
                                &fdoData->IdlePowerDownCompleteEvent, 
                                NULL);
        if(!NT_SUCCESS(status)) {
            
            KeSetEvent(&fdoData->IdlePowerDownCompleteEvent, 
                        IO_NO_INCREMENT, 
                        FALSE);
            PciDrvSetIdleTimer(fdoData);            
            break;
        }
        
        //
        // Wait for it to complete.
        //
        status = KeWaitForSingleObject(&fdoData->IdlePowerDownCompleteEvent, 
                                        Executive, KernelMode, FALSE, NULL);        
        ASSERT(NT_SUCCESS(status)); 
        
    }while(FALSE);
    
    if(Context) {
        IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
        ExFreePool((PVOID)Context);
        
    }
    
    DebugPrint(TRACE, DBG_POWER, "<-- PciDrvPowerDownDevice\n");
    
    return;
}

NTSTATUS
PciDrvSetPowerSaveEnableState(
    IN PFDO_DATA FdoData,
    IN BOOLEAN State
    )
/*++

Routine Description:

   Sets wakestate in the device registry.
   Called in response to WMI set WMI_POWER_DEVICE_ENABLE requests.

Arguments:

     FdoData - pointer to a device object.
     State - TRUE or FALSE.

Return Value:

   NT status code

--*/
{
    PAGED_CODE();

    if(!PciDrvWriteRegistryValue(FdoData,
                                    PCIDRV_POWER_SAVE_ENABLE, 
                                    (ULONG)State)) 
    {
                                    
        DebugPrint(TRACE, DBG_POWER, "PciDrvWriteRegistryValue failed\n");
        return STATUS_UNSUCCESSFUL;
    }
    
    return STATUS_SUCCESS;

}


BOOLEAN
PciDrvGetPowerSaveEnableState(
    IN PFDO_DATA   FdoData
    )
/*++

Routine Description:

   Gets wakestate from the device registry.
   Called in response to WMI query WMI_POWER_DEVICE_ENABLE requests.

Arguments:

    FdoData - pointer to our device extension.

Return Value:

   TRUE if enabled
   FALSE if not present/disabled/error in reading registry

--*/
{
    ULONG savePowerEnabled;

    PAGED_CODE();

    PciDrvReadRegistryValue(FdoData,
                                PCIDRV_POWER_SAVE_ENABLE, 
                                &savePowerEnabled);
    
    DebugPrint(TRACE, DBG_POWER, "Idle detection is%s enabled\n",
                savePowerEnabled ? "":" not");

    return (BOOLEAN)savePowerEnabled;
}

VOID
PciDrvRegisterPowerStateNotification(
    IN PFDO_DATA   FdoData
    )
/*++

Routine Description:

    This routine opens the PowerState callback object and registers
    a callback so that we can be notified whenever power source 
    changes from AC<->DC.

Arguments:

    FdoData - pointer to our device extension.

Return Value:

   VOID

--*/
{
    OBJECT_ATTRIBUTES       objAttributes;
    UNICODE_STRING          callbackName;
    NTSTATUS                status;

    //
    // First get the current power source. Since we don't have a
    // documented way of knowing whether we are currently running
    // on DC or AC power source. We will just assume that we are
    // on DC. 
    //

    FdoData->RunningOnBattery = TRUE;

    //
    // Register for callbacks so that we can detect switch between
    // AC and battery power.
    //

    RtlInitUnicodeString(&callbackName, L"\\Callback\\PowerState");
    
    InitializeObjectAttributes(&objAttributes,
                               &callbackName,
                               OBJ_CASE_INSENSITIVE  | OBJ_PERMANENT,
                               NULL,
                               NULL);
    
    status = ExCreateCallback(&FdoData->PowerStateCallbackObject,
                                   &objAttributes,
                                   FALSE, // Should already exist
                                   TRUE); // AllowMultipleCallbacks

    
    if(!NT_SUCCESS(status)) {
        DebugPrint(ERROR, DBG_POWER, 
            "Failed to create a Callback object status %lx\n", status);    
        return;
    }
    
    FdoData->PowerStateCallbackRegistrationHandle = ExRegisterCallback(
                                            FdoData->PowerStateCallbackObject,
                                            PciDrvPowerStateCallback,
                                            FdoData // Context
                                            );
    if(FdoData->PowerStateCallbackRegistrationHandle == NULL){
        
        DebugPrint(ERROR, DBG_POWER, "Failed to register callback\n");
        //
        // Remove the reference taken by ExCreateCallback. 
        //
        ObDereferenceObject(FdoData->PowerStateCallbackObject);
        FdoData->PowerStateCallbackObject = NULL;
    }
    
    return;
    
}

VOID
PciDrvUnregisterPowerStateNotification(
    IN PFDO_DATA   FdoData
    )
/*++

Routine Description:

    Unregister PowerState notification callback and release the
    reference on the notification object.

Arguments:

    FdoData - pointer to a device extension.

Return Value:

   VOID

--*/
{
    if (FdoData->PowerStateCallbackRegistrationHandle)
    {
        ExUnregisterCallback(FdoData->PowerStateCallbackRegistrationHandle);
        FdoData->PowerStateCallbackRegistrationHandle = NULL;
    }
    //
    // Remove the reference taken by ExCreateCallback. 
    //
    if (FdoData->PowerStateCallbackObject)
    {
        ObDereferenceObject(FdoData->PowerStateCallbackObject);
        FdoData->PowerStateCallbackObject = NULL;
    }

    return;
}

VOID
PciDrvPowerStateCallback(
    IN  PVOID CallbackContext,
    IN  PVOID Argument1,
    IN  PVOID Argument2
    )
/*++
Routine Description:

    This is the notification callback routine called by the executive
    subsystem in the context of the thread that make the notification
    on the object we have registered.

Arguments:

    CallBackContext - This is the context value specified in the
                        ExRegisterCallback call. It's a pointer
                        to our device context.
                                                
    Argument1 - First parameter specified in the call to ExNotifyCallback.
                In this case it contains power state values.
                     
    Argument2 -  Second parameter specified in the call to ExNotifyCallback.
                 It contains TRUE or FALSE. 
                   
    
Return Value:
    
    VOID

--*/{
    BOOLEAN     powerStateIsAC;
    ULONG_PTR   action = (ULONG_PTR)Argument1;
    ULONG_PTR   state  = (ULONG_PTR)Argument2;
    PFDO_DATA   fdoData = (PFDO_DATA)CallbackContext;
    
    if( PO_CB_AC_STATUS == action ) {

        //
        // AC <-> DC Transition has occurred
        // state == TRUE if running on AC, 
        // state == FALSE if running on battery power.
        //
        powerStateIsAC = (BOOLEAN)state;
        
        DebugPrint(INFO, DBG_POWER, "Power is now %s\n", powerStateIsAC?"AC":"DC");
        
        fdoData->RunningOnBattery = (powerStateIsAC) ? FALSE : TRUE;
        
    }

    return;
}

