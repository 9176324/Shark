/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Power.c

Abstract:

    This stage of Power.c builds upon the support implemented in the previous
    stage., Featured1\Power.c

    New features implemented in this stage of Power.c:

    -WPP software tracing is used.

    -ToasterDispatchPower passes IRP_MN_WAIT_WAKE power IRPs to
     ToasterDispatchWaitWake, which is implemented in Wake.c.   

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub – 02-Oct-2001 -
     Based on Adrian J. Oney's power management boilerplate code 
     
    Kevin Shirley – 01-Jul-2003 – Commented for tutorial and learning purposes

--*/

#include "toaster.h"
#include "power.h"

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
#include "power.tmh"
#endif

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
    ToasterDispatchPower calls ToasterDispatchWaitWake to process IRP_MN_WAIT_WAKE
    power IRPs.

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
        PoStartNextPowerIrp(Irp);

        IoSkipCurrentIrpStackLocation(Irp);

        status = PoCallDriver(fdoData->NextLowerDriver, Irp);

        ToasterIoDecrement (fdoData);

        return status;
    }

    switch(stack->MinorFunction)
    {
    case IRP_MN_SET_POWER:
        status = ToasterDispatchSetPowerState(DeviceObject, Irp);

        break;

    case IRP_MN_QUERY_POWER:
        status = ToasterDispatchQueryPowerState(DeviceObject, Irp);

        break;

    case IRP_MN_WAIT_WAKE:
        //
        // ToasterDispatchWaitWake processes IRP_MN_WAIT_WAKE power IRPs requested
        // earlier by the function driver when the ToasterArmForWake routine called
        // PoRequestPowerIrp.
        //
        status = ToasterDispatchWaitWake(DeviceObject, Irp);

        break;

    case IRP_MN_POWER_SEQUENCE:

    default:
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

Updated Routine Description:
    ToasterDispatchPowerDefault does not change in this stage of the function
    driver.

--*/
{
    NTSTATUS         status;
    PFDO_DATA        fdoData;

    PAGED_CODE();

    PoStartNextPowerIrp(Irp);

    IoSkipCurrentIrpStackLocation(Irp);

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    status = PoCallDriver(fdoData->NextLowerDriver, Irp);

    return status;
}


 
NTSTATUS
ToasterDispatchSetPowerState(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchSetPowerState does not change in this stage of the function
    driver.

--*/
{
    PIO_STACK_LOCATION stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchSetPowerState\n");

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

Updated Routine Description:
    ToasterDispatchQueryPowerState does not change in this stage of the function
    driver.

--*/
{
    PIO_STACK_LOCATION stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchQueryPowerState\n");

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

Updated Routine Description:
    ToasterDispatchSystemPowerIrp does not change in this stage of the function
    driver.

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
        fdoData->SystemPowerState = newSystemState;

        ToasterDebugPrint(INFO, "\tSetting the system state to %s\n",
                    DbgSystemPowerString(fdoData->SystemPowerState));
    }

    IoMarkIrpPending(Irp);

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
        Irp,
        (PIO_COMPLETION_ROUTINE) ToasterCompletionSystemPowerUp,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

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

Updated Routine Description:
    ToasterCompletionSystemPowerUp does not change in this stage of the function
    driver.

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA) Fdo->DeviceExtension;
    NTSTATUS    status = Irp->IoStatus.Status;

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionSystemPowerUp\n");

    if (!NT_SUCCESS(status))
    {
        PoStartNextPowerIrp(Irp);

        ToasterIoDecrement(fdoData);

        return STATUS_CONTINUE_COMPLETION;
    }

    ToasterQueueCorrespondingDeviceIrp(Irp, Fdo);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



 
VOID
ToasterQueueCorrespondingDeviceIrp(
    IN PIRP SIrp,
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Updated Routine Description:
    ToasterQueueCorrespondingDeviceIrp does not change in this stage of the
    function driver.

--*/

{
    NTSTATUS            status;
    POWER_STATE         state;
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);

    ToasterDebugPrint(TRACE, "Entered ToasterQueueCorrespondingDeviceIrp\n");

    status = ToasterGetPowerPoliciesDeviceState(
        SIrp,
        DeviceObject,
        &state.DeviceState
        );

    if (NT_SUCCESS(status))
    {
        ASSERT(NULL == fdoData->PendingSIrp);

        fdoData->PendingSIrp = SIrp;
        
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
        PoStartNextPowerIrp(SIrp);

        SIrp->IoStatus.Status = status;

        IoCompleteRequest(SIrp, IO_NO_INCREMENT);

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

Updated Routine Description:
    ToasterCompletionOnFinalizedDeviceIrp does not change in this stage of the
    function driver.

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA) PowerContext;
    PIRP        sIrp = fdoData->PendingSIrp;

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionOnFinalizedDeviceIrp\n");

    if(NULL != sIrp)
    {
        sIrp->IoStatus.Status = IoStatus->Status;

        PoStartNextPowerIrp(sIrp);

        IoCompleteRequest(sIrp, IO_NO_INCREMENT);

        fdoData->PendingSIrp = NULL;

        ToasterIoDecrement(fdoData);
    }
}


 
NTSTATUS
ToasterDispatchDeviceQueryPower(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Updated Routine Description:
    ToasterDispatchDeviceQueryPower does not change in this stage of the function
    driver.

--*/
{
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);
    DEVICE_POWER_STATE  deviceState = stack->Parameters.Power.State.DeviceState;
    NTSTATUS            status;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterDispatchDeviceQueryPower\n");

    if (PowerDeviceD0 == deviceState)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = ToasterQueuePassiveLevelPowerCallback(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            ToasterCallbackHandleDeviceQueryPower
            );

        if (STATUS_PENDING == status)
        {
            return status;
        }
    }

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

Updated Routine Description:
    ToasterDispatchDeviceSetPower does not change in this stage of the function
    driver.

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
        IoMarkIrpPending(Irp);

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
            Irp,
            (PIO_COMPLETION_ROUTINE) ToasterCompletionDevicePowerUp,
            NULL,
            TRUE,
            TRUE,
            TRUE
            );

        PoCallDriver(fdoData->NextLowerDriver, Irp);

        return STATUS_PENDING;
    }
    else
    {
        status = ToasterQueuePassiveLevelPowerCallback(
            DeviceObject,
            Irp,
            IRP_NEEDS_FORWARDING,
            ToasterCallbackHandleDeviceSetPower
            );

        if (STATUS_PENDING == status)
        {
            return status;
        }

        ASSERT(!NT_SUCCESS(status));

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

Updated Routine Description:
    ToasterCompletionDevicePowerUp does not change in this stage of the function
    driver.

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    NTSTATUS            status = Irp->IoStatus.Status;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(Irp);

    ToasterDebugPrint(TRACE, "Entered ToasterCompletionDevicePowerUp\n");

    if (!NT_SUCCESS(status))
    {
        PoStartNextPowerIrp(Irp);

        ToasterIoDecrement(fdoData);

        return STATUS_CONTINUE_COMPLETION;
    }

    ASSERT(IRP_MJ_POWER == stack->MajorFunction);
    ASSERT(IRP_MN_SET_POWER == stack->MinorFunction);

    status = ToasterQueuePassiveLevelPowerCallback(
        DeviceObject,
        Irp,
        IRP_ALREADY_FORWARDED,
        ToasterCallbackHandleDeviceSetPower
        );

    if (STATUS_PENDING != status)
    {
        ASSERT(!NT_SUCCESS(status));

        ToasterFinalizeDevicePowerIrp(
            DeviceObject,
            Irp,
            IRP_ALREADY_FORWARDED,
            status
            );
    }

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

Updated Routine Description:
    ToasterFinalizeDevicePowerIrp does not change in this stage of the function
    driver.

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Entered ToasterFinalizeDevicePowerIrp\n");

    if (IRP_ALREADY_FORWARDED == Direction || (!NT_SUCCESS(Result)))
    {
        PoStartNextPowerIrp(Irp);

        Irp->IoStatus.Status = Result;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        ToasterIoDecrement(fdoData);

        return Result;
    }

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

Updated Routine Description:
    ToasterQueuePassiveLevelPowerCallback does not change in this stage of the
    function driver.

--*/
{
    PIO_WORKITEM            item;
    PWORKER_THREAD_CONTEXT  context;
    PFDO_DATA               fdoData;

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    context = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(WORKER_THREAD_CONTEXT),
        TOASTER_POOL_TAG
        );

    if (NULL == context)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    item = IoAllocateWorkItem(DeviceObject);

    if (NULL == item)
    {
        ExFreePool(context);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    context->Irp = Irp;
    context->DeviceObject= DeviceObject;
    context->IrpDirection = Direction;
    context->Callback = Callback;
    context->WorkItem = item;

    IoMarkIrpPending(Irp);

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

Updated Routine Description:
    ToasterCallbackHandleDeviceQueryPower does not change in this stage of the
    function driver.

--*/
{
    DEVICE_POWER_STATE deviceState;
    PIO_STACK_LOCATION stack;
    NTSTATUS status;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation(Irp);

    deviceState = stack->Parameters.Power.State.DeviceState;

    ASSERT(PowerDeviceD0 != deviceState);

    status = ToasterPowerBeginQueuingIrps(
        DeviceObject,
        2,
        TRUE
        );

    ToasterFinalizeDevicePowerIrp(DeviceObject, Irp, Direction, status);
}


 
VOID
ToasterQueuePassiveLevelPowerCallbackWorker(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PVOID           Context
    )
/*++

Updated Routine Description:
    ToasterQueuePassiveLevelPowerCallbackWorker does not change in this stage of
    the function driver.

--*/
{
    PWORKER_THREAD_CONTEXT  context;

    PAGED_CODE();

    context = (PWORKER_THREAD_CONTEXT) Context;

    context->Callback(
        context->DeviceObject,
        context->Irp,
        context->IrpDirection
        );

    IoFreeWorkItem(context->WorkItem);

    ExFreePool((PVOID)context);
}


 
NTSTATUS
ToasterPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    )
/*++

Updated Routine Description:
    ToasterPowerBeginQueuingIrps does not change in this stage of the function
    driver.

--*/
{
    NTSTATUS status;
    PFDO_DATA fdoData;
    ULONG chargesRemaining;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    fdoData->QueueState = HoldRequests;

    chargesRemaining = IrpIoCharges;
    while(chargesRemaining--)
    {
        ToasterIoDecrement(fdoData);
    }

    KeWaitForSingleObject(
        &fdoData->StopEvent,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

    if (Query)
    {
        status = ToasterCanSuspendDevice(DeviceObject);

        if (!NT_SUCCESS(status))
        {
            fdoData->QueueState = AllowRequests;
            ToasterProcessQueuedRequests(fdoData);
        }
    }
    else
    {
        status = STATUS_SUCCESS;
    }

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

Updated Routine Description:
    ToasterCallbackHandleDeviceSetPower does not change in this stage of the
    function driver.

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

    newState =  stack->Parameters.Power.State;
    newDeviceState = newState.DeviceState;

    oldDeviceState = fdoData->DevicePowerState;

    fdoData->DevicePowerState = newDeviceState;

    ToasterDebugPrint(INFO, "\tSetting the device state to %s\n",
                            DbgDevicePowerString(fdoData->DevicePowerState));

    if (newDeviceState == oldDeviceState)
    {
        if (newDeviceState == PowerDeviceD0)
        {
            fdoData->QueueState = AllowRequests;

            ToasterProcessQueuedRequests(fdoData);
        }

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
        status = ToasterPowerBeginQueuingIrps(
            DeviceObject,
            2,
            FALSE
            );

        ASSERT(NT_SUCCESS(status));
    }

    newDeviceAction = stack->Parameters.Power.ShutdownType;

    if (newDeviceState > oldDeviceState)
    {
        PoSetPowerState(DeviceObject, DevicePowerState, newState);

        status = STATUS_SUCCESS;
    }
    else if (newDeviceState < oldDeviceState)
    {
        PoSetPowerState(DeviceObject, DevicePowerState, newState);

        status = STATUS_SUCCESS;
    }

    if (PowerDeviceD0 == newDeviceState)
    {
        fdoData->QueueState = AllowRequests;

        ToasterProcessQueuedRequests(fdoData);
    }

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

Updated Routine Description:
    ToasterGetPowerPoliciesDeviceState determines whether the device extension's
    WakeState member does not equal WAKESTATE_ARMED. If Wake does not equal
    WAKESTATE_ARMED, then return D3 device power state as the deepest device power
    state that can signal wake-up.

--*/
{
    PFDO_DATA           fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  stack = IoGetCurrentIrpStackLocation(SIrp);
    SYSTEM_POWER_STATE  systemState = stack->Parameters.Power.State.SystemState;
    DEVICE_POWER_STATE  deviceState;
    ULONG               wakeSupported;

    if (PowerSystemWorking == systemState)
    {
        *DevicePowerState = PowerDeviceD0;
        return STATUS_SUCCESS;
    } 
    else if (WAKESTATE_ARMED != fdoData->WakeState)
    {
        *DevicePowerState = PowerDeviceD3;

        return STATUS_SUCCESS;
    }

    if ((systemState <= PowerSystemSleeping3) &&
        (systemState > fdoData->DeviceCaps.SystemWake))
    {
        return STATUS_UNSUCCESSFUL;
    }

    for(deviceState = PowerDeviceD3;
        deviceState >= fdoData->DeviceCaps.DeviceState[systemState];
        deviceState--)
    {
        switch(deviceState)
        {
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

        if (wakeSupported)
        {
            *DevicePowerState = deviceState;
            return STATUS_SUCCESS;
        }
    }

    if (systemState <= PowerSystemSleeping3)
    {
        return STATUS_UNSUCCESSFUL;
    }

    *DevicePowerState = PowerDeviceD3;

    return STATUS_SUCCESS;
}


 
NTSTATUS
ToasterCanSuspendDevice(
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

Updated Routine Description:
    ToasterCanSuspendDevice does not change in this stage of the function driver.

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

Updated Routine Description:
    DbgPowerMinorFunctionString does not change in this stage of the function
    driver.

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

Updated Routine Description:
    DbgSystemPowerString does not change in this stage of the function driver.

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

Updated Routine Description:
    DbgDevicePowerString does not change in this stage of the function driver.

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

