/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    POWER.C

Abstract:

    This module handles Power management calls for both FDO and
    child PDOs.

Author:

    Eliyas Yakub    Sep 18, 1998
    
Environment:

    Kernel mode only

Notes:


Revision History:


--*/

#include "busenum.h"



NTSTATUS
Bus_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++
    Handles power Irps sent to both FDO and child PDOs.
    Note: Currently we do not implement full power handling
          for the FDO.
    
Arguments:

    DeviceObject - Pointer to the device object.
    Irp          - Pointer to the irp.

Return Value:

    NT status is returned.

--*/
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_POWER == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    //
    // If the device has been removed, the driver should 
    // not pass the IRP down to the next lower driver.
    //
    
    if (commonData->DevicePnPState == Deleted) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    if (commonData->IsFDO) {

        Bus_KdPrint (commonData, BUS_DBG_POWER_TRACE,
            ("FDO %s IRP:0x%x %s %s\n",
            PowerMinorFunctionString(irpStack->MinorFunction), Irp,
            DbgSystemPowerString(commonData->SystemPowerState), 
            DbgDevicePowerString(commonData->DevicePowerState)));


        status = Bus_FDO_Power ((PFDO_DEVICE_DATA)DeviceObject->DeviceExtension,
                                Irp);
    } else {
    
        Bus_KdPrint (commonData, BUS_DBG_POWER_TRACE,
            ("PDO %s IRP:0x%x %s %s\n",
            PowerMinorFunctionString(irpStack->MinorFunction), Irp,
            DbgSystemPowerString(commonData->SystemPowerState), 
            DbgDevicePowerString(commonData->DevicePowerState)));

        status = Bus_PDO_Power ((PPDO_DEVICE_DATA)DeviceObject->DeviceExtension,
                                Irp);
    }

    return status;
}


NTSTATUS
Bus_FDO_Power (
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    )
/*++
    Handles power Irps sent to the FDO.
    This driver is power policy owner for the bus itself 
    (not the devices on the bus).Power handling for the bus FDO 
    should be implemented similar to the function driver (toaster.sys)
    power code. We will just print some debug outputs and 
    forward this Irp to the next level. 
    
Arguments:

    Data -  Pointer to the FDO device extension.
    Irp  -  Pointer to the irp.

Return Value:

    NT status is returned.

--*/

{
    NTSTATUS            status;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    Bus_IncIoCount (Data);

    //
    // If the device is not stated yet, just pass it down.
    //
    
    if (Data->DevicePnPState == NotStarted) {
        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        status = PoCallDriver(Data->NextLowerDriver, Irp);
        Bus_DecIoCount (Data);
        return status;

    }

    if(stack->MinorFunction == IRP_MN_SET_POWER) {
        Bus_KdPrint_Cont(Data, BUS_DBG_POWER_TRACE,
                     ("\tRequest to set %s state to %s\n",
                     ((powerType == SystemPowerState) ?  "System" : "Device"),
                     ((powerType == SystemPowerState) ? \
                        DbgSystemPowerString(powerState.SystemState) :\
                        DbgDevicePowerString(powerState.DeviceState))));
    }

    PoStartNextPowerIrp (Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    status =  PoCallDriver (Data->NextLowerDriver, Irp);    
    Bus_DecIoCount (Data);
    return status;
}


NTSTATUS
Bus_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
/*++
    Handles power Irps sent to the PDOs.
    Typically a bus driver, that is not a power
    policy owner for the device, does nothing
    more than starting the next power IRP and
    completing this one.
    
Arguments:

    PdoData - Pointer to the PDO device extension.
    Irp     - Pointer to the irp.

Return Value:

    NT status is returned.

--*/

{
    NTSTATUS            status;
    PIO_STACK_LOCATION  stack;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;

    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
    
        Bus_KdPrint_Cont(PdoData, BUS_DBG_POWER_TRACE, 
                     ("\tSetting %s power state to %s\n",
                     ((powerType == SystemPowerState) ?  "System" : "Device"),
                     ((powerType == SystemPowerState) ? \
                        DbgSystemPowerString(powerState.SystemState) : \
                        DbgDevicePowerString(powerState.DeviceState))));
                     
        switch (powerType) {
            case DevicePowerState:
                PoSetPowerState (PdoData->Self, powerType, powerState);
                PdoData->DevicePowerState = powerState.DeviceState;
                status = STATUS_SUCCESS;
                break;

            case SystemPowerState:
                PdoData->SystemPowerState = powerState.SystemState;
                status = STATUS_SUCCESS;
                break;

            default:
                status = STATUS_NOT_SUPPORTED;
                break;
        }
        break;

    case IRP_MN_QUERY_POWER:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_WAIT_WAKE:
        //
        // We cannot support wait-wake because we are root-enumerated 
        // driver, and our parent, the PnP manager, doesn't support wait-wake. 
        // If you are a bus enumerated device, and if  your parent bus supports
        // wait-wake,  you should send a wait/wake IRP (PoRequestPowerIrp)
        // in response to this request. 
        // If you want to test the wait/wake logic implemented in the function
        // driver (toaster.sys), you could do the following simulation: 
        // a) Mark this IRP pending.
        // b) Set a cancel routine.
        // c) Save this IRP in the device extension
        // d) Return STATUS_PENDING.
        // Later on if you suspend and resume your system, your BUS_FDO_POWER
        // will be called to power the bus. In response to IRP_MN_SET_POWER, if the
        // powerstate is PowerSystemWorking, complete this Wake IRP. 
        // If the function driver, decides to cancel the wake IRP, your cancel routine
        // will be called. There you just complete the IRP with STATUS_CANCELLED.
        //
    case IRP_MN_POWER_SEQUENCE:
    default:
        status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;
    }

    PoStartNextPowerIrp(Irp);
    status = Irp->IoStatus.Status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}

#if DBG

PCHAR
PowerMinorFunctionString (
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

#endif

