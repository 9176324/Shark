/*++

Copyright (c) 1997-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    wmi.c

Abstract:

    This module contains the code that handles the wmi IRPs for the
    AGP filter driver(s)

Author:

Environment:

    Kernel mode

Revision History :
--*/

#include "agplib.h"
#include <wmistr.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpWmiRegistration)
#pragma alloc_text(PAGE, AgpWmiDeRegistration)
#pragma alloc_text(PAGE, AgpSystemControl)
#pragma alloc_text(PAGE, AgpSetWmiDataItem)
#pragma alloc_text(PAGE, AgpSetWmiDataBlock)
#pragma alloc_text(PAGE, AgpQueryWmiDataBlock)
#pragma alloc_text(PAGE, AgpQueryWmiRegInfo)
#endif

#define WMI_AGP_INFORMATION 0
#define NUMBER_OF_WMI_GUIDS 1

GUID AgpWmiGuid = AGP_WMI_STD_DATA_GUID;

WMIGUIDREGINFO WmiGuidList[1] =
{
    { &AgpWmiGuid, 1, 0 }   // Pointer Port driver information
};

NTSTATUS
AgpWmiRegistration(
    PTARGET_EXTENSION Extension
)
/*++

Routine Description:

    Registers with WMI as a data provider for this
    instance of the device

Arguments:

    Extension - Pointer to our taget extension

Return Value:

    STATUS_SUCCESS or an appropriate error status

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    Extension->WmiLibInfo.GuidCount = sizeof(WmiGuidList) /
        sizeof(WMIGUIDREGINFO);
    ASSERT (NUMBER_OF_WMI_GUIDS == Extension->WmiLibInfo.GuidCount);
    Extension->WmiLibInfo.GuidList = WmiGuidList;
    Extension->WmiLibInfo.QueryWmiRegInfo = AgpQueryWmiRegInfo;
    Extension->WmiLibInfo.QueryWmiDataBlock = AgpQueryWmiDataBlock;
    Extension->WmiLibInfo.SetWmiDataBlock = AgpSetWmiDataBlock;
    Extension->WmiLibInfo.SetWmiDataItem = AgpSetWmiDataItem;
    Extension->WmiLibInfo.ExecuteWmiMethod = NULL;
    Extension->WmiLibInfo.WmiFunctionControl = NULL;

    //
    // Register with WMI
    //
    
    status = IoWMIRegistrationControl(Extension->Self,
                             WMIREG_ACTION_REGISTER
                             );

    //
    // Initialize the Std device data structure
    //
    Globals.AgpCommand = 0;
    Globals.AgpStatus = 0;

    return status;
    
}

NTSTATUS
AgpWmiDeRegistration(
    PTARGET_EXTENSION Extension
)
/*++

Routine Description:

     Inform WMI to remove this DeviceObject from its 
     list of providers. This function also 
     decrements the reference count of the deviceobject.

Arguments:

   Extension - Pointer to our target extension

Return Value:

   STATUS_SUCCESS or error

--*/
{
    PAGED_CODE();

    return IoWMIRegistrationControl(Extension->Self,
                                    WMIREG_ACTION_DEREGISTER
                                    );
}

NTSTATUS
AgpSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
/*++

Routine Description

    We have just received a System Control IRP.

    Assume that this is a WMI IRP and call into the WMI system library and let
    it handle this IRP for us.

Arguments:

    DeviceObject - Pointer to our device object

    Irp - Points to the corresponding I/O request packet

Return Value:

   STATUS_SUCCESS or an appropriate error status

--*/
{
    PTARGET_EXTENSION       deviceExtension;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    PAGED_CODE();
    
    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    status = WmiSystemControl(&deviceExtension->WmiLibInfo, 
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
            status =
                IoCallDriver(deviceExtension->CommonExtension.AttachedDevice,
                             Irp);
            break;
        }
                                    
        default:
        {
            //
            // We really should never get here, but if we do just forward....
            ASSERT(FALSE);
            IoSkipCurrentIrpStackLocation (Irp);
            status =
                IoCallDriver(deviceExtension->CommonExtension.AttachedDevice,
                             Irp);
            break;
        }        
    }
    
    return(status);
}

NTSTATUS
AgpSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to set for the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    DataItemId has the id of the data item being set

    BufferSize has the size of the data item passed

    Buffer has the new values for the data item


Return Value:

    status

--*/
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch(GuidIndex) {

    case WMI_AGP_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  0,
                                  IO_NO_INCREMENT
                                  );

    return status;
}

NTSTATUS
AgpSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to set the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    BufferSize has the size of the data block passed

    Buffer has the new values for the data block


Return Value:

    status

--*/
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;

    PAGED_CODE();

    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_AGP_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  0,
                                  IO_NO_INCREMENT
                                  );

    return status;
}

NTSTATUS
AgpQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    )
/*++

Routine Description:

    This routine is a callback into the driver to query for the contents of
    a data block. When the driver has finished filling the data block it
    must call ClassWmiCompleteRequest to complete the irp. The driver can
    return STATUS_PENDING if the irp cannot be completed immediately.

Arguments:

    DeviceObject is the device whose data block is being queried

    Irp is the Irp that makes this request

    GuidIndex is the index into the list of guids provided when the
        device registered

    BufferAvail on has the maximum size available to write the data
        block.

    Buffer on return is filled with the returned data block


Return Value:

    status

--*/
{
    PTARGET_EXTENSION   deviceExtension;
    NTSTATUS            status;
    ULONG               size = 0;

    PAGED_CODE();

    ASSERT((InstanceIndex == 0) && (InstanceCount == 1));
    
    deviceExtension = (PTARGET_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {

    case WMI_AGP_INFORMATION: {

        AGP_STD_DATA AgpData;

        size = sizeof(AGP_STD_DATA);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        AgpData.AgpStatus = Globals.AgpStatus;
        AgpData.AgpCommand = Globals.AgpCommand;
        AgpData.ApertureBase = deviceExtension->GartBase;
        AgpData.ApertureLength =
            deviceExtension->GartLengthInPages * PAGE_SIZE;

        *(PAGP_STD_DATA)Buffer = AgpData;

        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;
        break;
    }

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }

    status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT
                                  );

    return status;
}

NTSTATUS
AgpQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
/*++

Routine Description:

    This routine is a callback into the driver to retrieve information about
    the guids being registered. 
            
    Implementations of this routine may be in paged memory

Arguments:

    DeviceObject is the device whose registration information is needed

    *RegFlags returns with a set of flags that describe all of the guids being
        registered for this device. If the device wants enable and disable
        collection callbacks before receiving queries for the registered
        guids then it should return the WMIREG_FLAG_EXPENSIVE flag. Also the
        returned flags may specify WMIREG_FLAG_INSTANCE_PDO in which case
        the instance name is determined from the PDO associated with the
        device object. Note that the PDO must have an associated devnode. If
        WMIREG_FLAG_INSTANCE_PDO is not set then Name must return a unique
        name for the device. These flags are ORed into the flags specified
        by the GUIDREGINFO for each guid.               

    InstanceName returns with the instance name for the guids if
        WMIREG_FLAG_INSTANCE_PDO is not set in the returned *RegFlags. The
        caller will call ExFreePool with the buffer returned.

    *RegistryPath returns with the registry path of the driver. This is 
        required
                
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
    PTARGET_EXTENSION deviceExtension;
    PUNICODE_STRING regPath;    
        
    PAGED_CODE();
    
    deviceExtension = DeviceObject->DeviceExtension;
    
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = deviceExtension->PDO;
 
    return STATUS_SUCCESS;
}

