/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    WMI.C

Abstract:

    This module handles all the WMI Irps.


Environment:

    Kernel mode

Revision History:

    Eliyas Yakub Dec 12, 1998


--*/
#include "busenum.h"
#include <wmistr.h>

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,Bus_WmiRegistration)
#pragma alloc_text(PAGE,Bus_WmiDeRegistration)
#pragma alloc_text(PAGE,Bus_SystemControl)
#pragma alloc_text(PAGE,Bus_SetWmiDataItem)
#pragma alloc_text(PAGE,Bus_SetWmiDataBlock)
#pragma alloc_text(PAGE,Bus_QueryWmiDataBlock)
#pragma alloc_text(PAGE,Bus_QueryWmiRegInfo)
#endif

#define MOFRESOURCENAME L"BusEnumWMI"

#define NUMBER_OF_WMI_GUIDS                 1
#define WMI_TOASTER_BUS_DRIVER_INFORMATION  0

WMIGUIDREGINFO ToasterBusWmiGuidList[] =
{
    {
        &TOASTER_BUS_WMI_STD_DATA_GUID, 1, 0 // driver information
    }
};

NTSTATUS
Bus_WmiRegistration(
    PFDO_DEVICE_DATA               FdoData
)
/*++
Routine Description

    Registers with WMI as a data provider for this
    instance of the device

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    FdoData->WmiLibInfo.GuidCount = sizeof (ToasterBusWmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);
    ASSERT (NUMBER_OF_WMI_GUIDS == FdoData->WmiLibInfo.GuidCount);
    FdoData->WmiLibInfo.GuidList = ToasterBusWmiGuidList;
    FdoData->WmiLibInfo.QueryWmiRegInfo = Bus_QueryWmiRegInfo;
    FdoData->WmiLibInfo.QueryWmiDataBlock = Bus_QueryWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataBlock = Bus_SetWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataItem = Bus_SetWmiDataItem;
    FdoData->WmiLibInfo.ExecuteWmiMethod = NULL;
    FdoData->WmiLibInfo.WmiFunctionControl = NULL;

    //
    // Register with WMI
    //
    
    status = IoWMIRegistrationControl(FdoData->Self,
                             WMIREG_ACTION_REGISTER
                             );

    //
    // Initialize the Std device data structure
    //
                    
    FdoData->StdToasterBusData.ErrorCount = 0; 
    FdoData->StdToasterBusData.DebugPrintLevel = BusEnumDebugLevel;

    return status;
    
}

NTSTATUS
Bus_WmiDeRegistration(
    PFDO_DEVICE_DATA               FdoData
)
/*++
Routine Description

     Inform WMI to remove this DeviceObject from its 
     list of providers. This function also 
     decrements the reference count of the deviceobject.

--*/
{

    PAGED_CODE();

    return IoWMIRegistrationControl(FdoData->Self,
                                 WMIREG_ACTION_DEREGISTER
                                 );

}

NTSTATUS
Bus_SystemControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++
Routine Description

    We have just received a System Control IRP.

    Assume that this is a WMI IRP and
    call into the WMI system library and let it handle this IRP for us.

--*/
{
    PFDO_DEVICE_DATA               fdoData;
    SYSCTL_IRP_DISPOSITION  disposition;
    NTSTATUS                status;
    PIO_STACK_LOCATION      stack;
    PCOMMON_DEVICE_DATA     commonData;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (!commonData->IsFDO) {
        //
        // The PDO, just complete the request with the current status
        //
        Bus_KdPrint (commonData, BUS_DBG_WMI_TRACE, 
            ("PDO %s\n", WMIMinorFunctionString(stack->MinorFunction)));
        status = Irp->IoStatus.Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;
    
    Bus_KdPrint (fdoData, BUS_DBG_WMI_TRACE, 
             ("FDO: %s\n", WMIMinorFunctionString(stack->MinorFunction)));

    Bus_IncIoCount (fdoData);
    
    if (fdoData->DevicePnPState == Deleted) {
        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE ;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        Bus_DecIoCount (fdoData);
        return status;
    }
    
    status = WmiSystemControl(&fdoData->WmiLibInfo, 
                                 DeviceObject, 
                                 Irp,
                                 &disposition);
    switch(disposition)
    {
        case IrpProcessed:
        {
            //
            // This irp has been processed and may be completed or pending.
            break;
        }
        
        case IrpNotCompleted:
        {
            //
            // This irp has not been completed, but has been fully processed.
            // we will complete it now
            IoCompleteRequest(Irp, IO_NO_INCREMENT);                
            break;
        }
        
        case IrpForward:
        case IrpNotWmi:
        {
            //
            // This irp is either not a WMI irp or is a WMI irp targetted
            // at a device lower in the stack.
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (fdoData->NextLowerDriver, Irp);
            break;
        }
                                    
        default:
        {
            //
            // We really should never get here, but if we do just forward....
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (fdoData->NextLowerDriver, Irp);
            break;
        }        
    }

    Bus_DecIoCount (fdoData);

    return(status);
}

//
// WMI System Call back functions
//

NTSTATUS
Bus_SetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to set for the contents of
    a data block. When the driver has finished filling the data block it
    must call WmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    InstanceIndex is the index that denotes which instance of the data block
        is being queried.
            
    DataItemId has the id of the data item being set

    BufferSize has the size of the data item passed

    Buffer has the new values for the data item


Return Value:

    status

--*/
{
    PFDO_DEVICE_DATA    fdoData;
    NTSTATUS status;
    ULONG requiredSize = 0;

    PAGED_CODE();

    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    switch(GuidIndex) {
    
    case WMI_TOASTER_BUS_DRIVER_INFORMATION:
 
       if(DataItemId == 2)
        {
           requiredSize = sizeof(ULONG);

           if (BufferSize < requiredSize) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
           }

           BusEnumDebugLevel = fdoData->StdToasterBusData.DebugPrintLevel = 
                                    *((PULONG)Buffer);
           status = STATUS_SUCCESS;
        }
        else {
            status = STATUS_WMI_READ_ONLY;
        }
        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  requiredSize,
                                  IO_NO_INCREMENT);

    return status;
}

NTSTATUS
Bus_SetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to set the contents of
    a data block. When the driver has finished filling the data block it
    must call WmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    InstanceIndex is the index that denotes which instance of the data block
        is being queried.
            
    BufferSize has the size of the data block passed

    Buffer has the new values for the data block


Return Value:

    status

--*/
{
    PFDO_DEVICE_DATA   fdoData;
    NTSTATUS status;
    ULONG requiredSize = 0;

    PAGED_CODE();

    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;


    switch(GuidIndex) {
    case WMI_TOASTER_BUS_DRIVER_INFORMATION:

        requiredSize = sizeof(TOASTER_BUS_WMI_STD_DATA);

        if(BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        //
        // We will update only writable elements.
        //
        BusEnumDebugLevel = fdoData->StdToasterBusData.DebugPrintLevel = 
                    ((PTOASTER_BUS_WMI_STD_DATA)Buffer)->DebugPrintLevel;
                    
        status = STATUS_SUCCESS;

        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  requiredSize,
                                  IO_NO_INCREMENT);

    return(status);
}

NTSTATUS
Bus_QueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to query for the contents of
    a data block. When the driver has finished filling the data block it
    must call WmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    InstanceIndex is the index that denotes which instance of the data block
        is being queried.
            
    InstanceCount is the number of instnaces expected to be returned for
        the data block.
            
    InstanceLengthArray is a pointer to an array of ULONG that returns the 
        lengths of each instance of the data block. If this is NULL then
        there was not enough space in the output buffer to fulfill the request
        so the irp should be completed with the buffer needed.        
            
    BufferAvail on has the maximum size available to write the data
        block.

    Buffer on return is filled with the returned data block


Return Value:

    status

--*/
{
    PFDO_DEVICE_DATA               fdoData;    
    NTSTATUS    status;
    ULONG       size = 0;

    PAGED_CODE();

    //
    // Only ever registers 1 instance per guid
    //
    
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));
    
    fdoData = (PFDO_DEVICE_DATA) DeviceObject->DeviceExtension;

    switch (GuidIndex) {
    case WMI_TOASTER_BUS_DRIVER_INFORMATION:

        size = sizeof (TOASTER_BUS_WMI_STD_DATA);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        * (PTOASTER_BUS_WMI_STD_DATA) Buffer = fdoData->StdToasterBusData;
        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;

        break;

    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT);

    return status;
}

NTSTATUS
Bus_QueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo	    
    )
/*++

Routine Description:

    This routine is a callback into the driver to retrieve the list of
    guids or data blocks that the driver wants to register with WMI. This
    routine may not pend or block. Driver should NOT call
    WmiCompleteRequest.

Arguments:

    DeviceObject is the device whose data block is being queried

    *RegFlags returns with a set of flags that describe the guids being
        registered for this device. If the device wants to enable and disable
        collection callbacks before receiving queries for the registered
        guids then it should return the WMIREG_FLAG_EXPENSIVE flag. Also the
        returned flags may specify WMIREG_FLAG_INSTANCE_PDO in which case
        the instance name is determined from the PDO associated with the
        device object. Note that the PDO must have an associated devnode. If
        WMIREG_FLAG_INSTANCE_PDO is not set then Name must return a unique
        name for the device.

    InstanceName returns with the instance name for the guids if
        WMIREG_FLAG_INSTANCE_PDO is not set in the returned *RegFlags. The
        caller will call ExFreePool with the buffer returned.

    *RegistryPath returns with the registry path of the driver

    *MofResourceName returns with the name of the MOF resource attached to
        the binary file. If the driver does not have a mof resource attached
        then this can be returned as NULL.

    *Pdo returns with the device object for the PDO associated with this
        device if the WMIREG_FLAG_INSTANCE_PDO flag is retured in 
        *RegFlags.

Return Value:

    status

--*/
{
    PFDO_DEVICE_DATA fdoData;
    
    PAGED_CODE();

    fdoData = (PFDO_DEVICE_DATA)DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = fdoData->UnderlyingPDO;
    RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);
    
    return STATUS_SUCCESS;
}

#if DBG

PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
)
{
    switch (MinorFunction)
    {
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
            return "IRP_MN_CHANGE_SINGLE_INSTANCE";
        case IRP_MN_CHANGE_SINGLE_ITEM:
            return "IRP_MN_CHANGE_SINGLE_ITEM";
        case IRP_MN_DISABLE_COLLECTION:
            return "IRP_MN_DISABLE_COLLECTION";
        case IRP_MN_DISABLE_EVENTS:
            return "IRP_MN_DISABLE_EVENTS";
        case IRP_MN_ENABLE_COLLECTION:
            return "IRP_MN_ENABLE_COLLECTION";
        case IRP_MN_ENABLE_EVENTS:
            return "IRP_MN_ENABLE_EVENTS";
        case IRP_MN_EXECUTE_METHOD:
            return "IRP_MN_EXECUTE_METHOD";
        case IRP_MN_QUERY_ALL_DATA:
            return "IRP_MN_QUERY_ALL_DATA";
        case IRP_MN_QUERY_SINGLE_INSTANCE:
            return "IRP_MN_QUERY_SINGLE_INSTANCE";
        case IRP_MN_REGINFO:
            return "IRP_MN_REGINFO";
        default:
            return "unknown_syscontrol_irp";
    }
}

#endif

