/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    power.c

Abstract


Author:

    Kashif Hasan (khasan) 3/31/00

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   taken from 1394diag/ohcidiag
3/31/00  khasan    implement suspend/resume ability
--*/

#include "pch.h"

NTSTATUS
t1394Diag_Power(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION      IrpSp;
    PDEVICE_EXTENSION       deviceExtension;
    POWER_STATE             State;
    KIRQL                   Irql;

    ENTER("t1394Diag_Power");

    deviceExtension = DeviceObject->DeviceExtension;

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    State = IrpSp->Parameters.Power.State;

    TRACE(TL_TRACE, ("Power.Type = 0x%x\n", IrpSp->Parameters.Power.Type));
    TRACE(TL_TRACE, ("Power.State.SystemState = 0x%x\n", State.SystemState));
    TRACE(TL_TRACE, ("Power.State.DeviceState = 0x%x\n", State.DeviceState));

    switch (IrpSp->MinorFunction) {

        case IRP_MN_SET_POWER:
            
            TRACE(TL_TRACE, ("IRP_MN_SET_POWER\n"));                        

            switch (IrpSp->Parameters.Power.Type) {

                case SystemPowerState:
                    TRACE(TL_TRACE, ("SystemPowerState\n"));
                    
                    // Send the IRP down
                    IoMarkIrpPending (Irp);
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    IoSetCompletionRoutine(Irp,
                        (PIO_COMPLETION_ROUTINE) t1394Diag_SystemSetPowerIrpCompletion,
                        NULL, TRUE, TRUE, TRUE);

                    PoCallDriver(deviceExtension->StackDeviceObject, Irp);
                    ntStatus = STATUS_PENDING;
                    break;
                
                case DevicePowerState:
                    TRACE(TL_TRACE, ("DevicePowerState\n"));
                    TRACE(TL_TRACE, ("Current device state = 0x%x, new device state = 0x%x\n", 
                                     deviceExtension->CurrentDevicePowerState, State.DeviceState));

                    PoStartNextPowerIrp(Irp);
                    IoCopyCurrentIrpStackLocationToNext(Irp);
                    ntStatus = PoCallDriver(deviceExtension->StackDeviceObject, Irp);
                    break; // DevicePowerState

                default:
                    break;

            }
            break; // IRP_MN_SET_POWER

        case IRP_MN_QUERY_POWER:
            TRACE(TL_TRACE, ("IRP_MN_QUERY_POWER\n"));

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            ntStatus = PoCallDriver(deviceExtension->StackDeviceObject, Irp);

            break; // IRP_MN_QUERY_POWER

        default:
            TRACE(TL_TRACE, ("Default = 0x%x\n", IrpSp->MinorFunction));

            PoStartNextPowerIrp(Irp);
            IoSkipCurrentIrpStackLocation(Irp);
            ntStatus = PoCallDriver(deviceExtension->StackDeviceObject, Irp);

            break; // default
            
    } // switch

    EXIT("t1394Diag_Power", ntStatus);
    return(ntStatus);
} // t1394Diag_Power


NTSTATUS
t1394Diag_SystemSetPowerIrpCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    )
{

    PDEVICE_EXTENSION           deviceExtension     = DeviceObject->DeviceExtension;
    NTSTATUS                    ntStatus            = Irp->IoStatus.Status;
    PIO_STACK_LOCATION          stack               = IoGetCurrentIrpStackLocation(Irp);
    POWER_STATE                 state               = stack->Parameters.Power.State;
    POWER_COMPLETION_CONTEXT    *powerContext       = NULL;
    PIRP                        pDIrp               = NULL;

    ENTER("t1394Diag_SystemPowerIrpCompletion");

    if (!NT_SUCCESS(ntStatus)) {

        TRACE(TL_TRACE, ("Set System Power Irp failed, status = 0x%x\n", ntStatus));
        PoStartNextPowerIrp(Irp);
        return STATUS_SUCCESS;
    }

    // allocate powerContext
    powerContext = (POWER_COMPLETION_CONTEXT*)
                ExAllocatePool(NonPagedPool, sizeof(POWER_COMPLETION_CONTEXT));

    if (!powerContext) {

        TRACE(TL_TRACE, ("Failed to allocate powerContext, status = 0x%x\n", ntStatus));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_SystemSetPowerIrpCompletion;

    } else {

        if (state.SystemState == PowerSystemWorking) {

            state.DeviceState = PowerDeviceD0;
        }
        else {
                    
            state.DeviceState = PowerDeviceD3;
        }
        
        powerContext->DeviceObject = DeviceObject;
        powerContext->SIrp = Irp;
        
        ntStatus = PoRequestPowerIrp(deviceExtension->StackDeviceObject,
                                    IRP_MN_SET_POWER,
                                    state,
                                    t1394Diag_DeviceSetPowerIrpCompletion,
                                    powerContext,
                                    &pDIrp);
        
        TRACE(TL_TRACE, ("New Device Set Power Irp = 0x%x\n", pDIrp));
    }

Exit_SystemSetPowerIrpCompletion:

    if (!NT_SUCCESS(ntStatus)) {


        TRACE(TL_TRACE, ("System SetPowerIrp Completion routine failed = 0x%x\n", ntStatus));

        if (powerContext) {
            ExFreePool(powerContext);
        }

        PoStartNextPowerIrp(Irp);
        Irp->IoStatus.Status = ntStatus;
    }
    else
    {
        ntStatus = STATUS_MORE_PROCESSING_REQUIRED;
    }

    EXIT("t1394Diag_SystemPowerIrpCompletion", ntStatus);    
    return ntStatus;
}



VOID
t1394Diag_DeviceSetPowerIrpCompletion(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE state,
    POWER_COMPLETION_CONTEXT* PowerContext,
    PIO_STATUS_BLOCK IoStatus
    )
{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) PowerContext->DeviceObject->DeviceExtension;
    PIRP                sIrp = PowerContext->SIrp;
    NTSTATUS            ntStatus = IoStatus->Status;

    ENTER("t1394Diag_DevicePowerIrpCompletion");

    if(!NT_SUCCESS(ntStatus))
    {
        TRACE(TL_WARNING, ("Device Power Irp failed by driver below us = 0x%x\n", ntStatus));
        goto Exit_t1394Diag_DeviceSetPowerIrpCompletion;
    }

    // set the new power state
    PoSetPowerState(DeviceObject, DevicePowerState, state);

    // figure out how to respond to this power state change
    if (state.DeviceState < deviceExtension->CurrentDevicePowerState)
    {
        // adding power
        // check to see if we are working again
        if (PowerDeviceD0 == state.DeviceState)
        {
            TRACE (TL_TRACE, ("Restarting our device\n"));
            deviceExtension->bShutdown = FALSE;
            
            // update the generation count                    
            t1394_GetGenerationCount(DeviceObject, NULL, &deviceExtension->GenerationCount);
        }
    }
    else
    {
        // removing power
        deviceExtension->bShutdown = TRUE;
    }
        
    // save the current device power state
    deviceExtension->CurrentDevicePowerState = state.DeviceState;

Exit_t1394Diag_DeviceSetPowerIrpCompletion:

    // Here we copy the D-IRP status into the S-IRP
    sIrp->IoStatus.Status = IoStatus->Status;

    // Release the IRP
    PoStartNextPowerIrp(sIrp);    
    sIrp->IoStatus.Information = 0;
    IoCompleteRequest(sIrp, IO_NO_INCREMENT);

    // Cleanup
    ExFreePool(PowerContext);
    
    EXIT("t1394Diag_DevicePowerIrpCompletion", ntStatus);
}

