/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    WMI.C

Abstract:

    This module handle all the WMI Irps.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub Oct 26, 1998

--*/


#include "precomp.h"

#if defined(EVENT_TRACING)
#include <wmi.tmh> 
#endif

static PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
);

#if defined(WIN2K) && defined(EVENT_TRACING)  
#define NUMBER_OF_WMI_GUIDS             6
#else 
#define NUMBER_OF_WMI_GUIDS             5
#endif
        

#define MOFRESOURCENAME L"PciDrvWMI"

WMIGUIDREGINFO PciDrvWmiGuidList[NUMBER_OF_WMI_GUIDS];

//
// These are indices of GUID list defined above.
//
#define WMI_PCIDRV_DRIVER_INFORMATION       0
#define WMI_POWER_DEVICE_WAKE_ENABLE        1 
#define WMI_POWER_DEVICE_ENABLE              2
#define WMI_POWER_CONSERVATION_IDLE_TIME    3
#define WMI_POWER_PERFORMANCE_IDLE_TIME     4

//
// Global debug error level
//
extern ULONG DebugLevel;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PciDrvWmiRegistration)
#pragma alloc_text(PAGE,PciDrvWmiDeRegistration)
#pragma alloc_text(PAGE,PciDrvSystemControl)
#pragma alloc_text(PAGE,PciDrvSetWmiDataBlock)
#pragma alloc_text(PAGE,PciDrvQueryWmiDataBlock)
#pragma alloc_text(PAGE,PciDrvQueryWmiRegInfo)
#endif

NTSTATUS
PciDrvWmiRegistration(
    PFDO_DATA               FdoData
)
/*++
Routine Description

    Registers with WMI as a data provider for this
    instance of the device

--*/
{
    NTSTATUS status;
    ULONG    guidCount = 0;
                                
   
    PAGED_CODE();

    //
    // Register a guid to get standard device information.
    //
    PciDrvWmiGuidList[guidCount].Flags = 0;
    PciDrvWmiGuidList[guidCount].InstanceCount = 1;
    PciDrvWmiGuidList[guidCount].Guid = &PCIDRV_WMI_STD_DATA_GUID;
    guidCount++;
    
    //
    // If upper edge is not NDIS, we will be the power policy
    // owner for this device.
    //
    if(!FdoData->IsUpperEdgeNdis) { 

        //
        // If you handle the following two GUIDs, the device manager will
        // show the power-management tab in the device properites dialog.
        //        
        PciDrvWmiGuidList[guidCount].Flags = 0;
        PciDrvWmiGuidList[guidCount].InstanceCount = 1;
        PciDrvWmiGuidList[guidCount].Guid = &GUID_POWER_DEVICE_WAKE_ENABLE;
        guidCount++;

        PciDrvWmiGuidList[guidCount].Flags = 0;
        PciDrvWmiGuidList[guidCount].InstanceCount = 1;
        PciDrvWmiGuidList[guidCount].Guid = &GUID_POWER_DEVICE_ENABLE;
        guidCount++;
        //
        // Following two guids are used to get/set the conservation and 
        // performance time during idle detection.
        //
        PciDrvWmiGuidList[guidCount].Flags = 0;
        PciDrvWmiGuidList[guidCount].InstanceCount = 1;
        PciDrvWmiGuidList[guidCount].Guid = &GUID_POWER_CONSERVATION_IDLE_TIME;
        guidCount++;

        PciDrvWmiGuidList[guidCount].Flags = 0;
        PciDrvWmiGuidList[guidCount].InstanceCount = 1;
        PciDrvWmiGuidList[guidCount].Guid = &GUID_POWER_PERFORMANCE_IDLE_TIME;
        guidCount++;
        
    }        

#if defined(WIN2K) && defined(EVENT_TRACING)  
    //
    // The GUID acts as the control GUID for enabling and disabling 
    // the tracing. 
    //
    PciDrvWmiGuidList[guidCount].Flags = 0;
    PciDrvWmiGuidList[guidCount].InstanceCount = 1;
    PciDrvWmiGuidList[guidCount].Guid = &WPP_TRACE_CONTROL_NULL_GUID;
    guidCount++;
    
#endif

    ASSERT(guidCount <= NUMBER_OF_WMI_GUIDS);

    FdoData->WmiLibInfo.GuidCount = guidCount;
    FdoData->WmiLibInfo.GuidList = PciDrvWmiGuidList;
    FdoData->WmiLibInfo.QueryWmiRegInfo = PciDrvQueryWmiRegInfo;
    FdoData->WmiLibInfo.QueryWmiDataBlock = PciDrvQueryWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataBlock = PciDrvSetWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataItem = NULL;
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

    FdoData->StdDeviceData.DebugPrintLevel = DebugLevel;

    return status;

}

NTSTATUS
PciDrvWmiDeRegistration(
    PFDO_DATA               FdoData
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
PciDrvSystemControl (
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
    PFDO_DATA               fdoData;
    SYSCTL_IRP_DISPOSITION  disposition;
    NTSTATUS                status;
    PIO_STACK_LOCATION      stack;
    
    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    DebugPrint(TRACE, DBG_WMI, "FDO %s\n",
                WMIMinorFunctionString(stack->MinorFunction));

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    PciDrvPowerUpDevice(fdoData, TRUE);
    
    PciDrvIoIncrement (fdoData);

    if (fdoData->DevicePnPState == Deleted) {
        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        PciDrvIoDecrement (fdoData);
        return status;
    }

    status = WmiSystemControl(&fdoData->WmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 &disposition);

    //
    // Following code is required if we want to do tracing in WIN2K.
    // Tracing is enabled or disabled explicitly by sending WMI
    // control events on Win2K.
    //
#if defined(WIN2K) && defined(EVENT_TRACING)

    if (disposition != IrpProcessed && 
            DeviceObject == (PDEVICE_OBJECT)stack->Parameters.WMI.ProviderId) {
        ULONG returnSize = 0;

        DebugPrint(INFO, DBG_WMI, "Calling WPP_TRACE_CONTROL\n");

        WPP_TRACE_CONTROL(stack->MinorFunction,
                          stack->Parameters.WMI.Buffer,
                          stack->Parameters.WMI.BufferSize,
                          returnSize);
    }
#endif

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
            // This irp is either not a WMI irp or is a WMI irp targeted
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

    PciDrvIoDecrement(fdoData);

    return(status);
}

//
// WMI System Call back functions
//

NTSTATUS
PciDrvSetWmiDataBlock(
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
    PFDO_DATA   fdoData;
    NTSTATUS    status = STATUS_WMI_GUID_NOT_FOUND;
    BOOLEAN     waitWakeEnabled;
    BOOLEAN     powerSaveEnabled;
    LONGLONG    newIdleTime, prevIdleTime;
    ULONG       requiredSize = 0;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    DebugPrint(TRACE, DBG_WMI, "Entered PciDrvSetWmiDataBlock\n");

    switch(GuidIndex) {
    case WMI_PCIDRV_DRIVER_INFORMATION:

        //
        // We will update only writable elements.
        //
        requiredSize = sizeof(PCIDRV_WMI_STD_DATA);
        if (BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }        
        DebugLevel = fdoData->StdDeviceData.DebugPrintLevel =
                    ((PPCIDRV_WMI_STD_DATA)Buffer)->DebugPrintLevel;

        status = STATUS_SUCCESS;

        break;


    case WMI_POWER_DEVICE_WAKE_ENABLE:
        
        //
        // If the wait-wake state is true (box is checked), we send a IRP_MN_WAIT_WAKE
        //  irp to the bus driver. If it's FALSE, we cancel it.
        // Every time the user changes his preference,
        // we write that to the registry to carry over his preference across boot.
        // If the user tries to enable wait-wake, and if the driver stack is not
        // able to do, it will reset the value in the registry to indicate that
        // the device is incapable of wait-waking the system.
        //
        requiredSize = sizeof(BOOLEAN);
        if (BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }        

        if(IsPoMgmtSupported(fdoData)){
        
            waitWakeEnabled = *(PBOOLEAN) Buffer;

            PciDrvSetWaitWakeEnableState(fdoData, waitWakeEnabled);

            if (waitWakeEnabled) {

                PciDrvArmForWake(fdoData, FALSE);

            } else {

                PciDrvDisarmWake(fdoData, FALSE);
            }

            status = STATUS_SUCCESS;
        }
        break;

    case WMI_POWER_DEVICE_ENABLE:

        requiredSize = sizeof(BOOLEAN);
        if (BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        
        if(IsPoMgmtSupported(fdoData)){

            powerSaveEnabled = *(PBOOLEAN) Buffer;

            PciDrvSetPowerSaveEnableState(fdoData,  powerSaveEnabled);
            
            if(powerSaveEnabled) {
                
                PciDrvRegisterForIdleDetection(fdoData, FALSE);
                
            } else {
            
                PciDrvDeregisterIdleDetection(fdoData, FALSE);
            }
            
            status = STATUS_SUCCESS;
        }
        break;

    case WMI_POWER_CONSERVATION_IDLE_TIME:

        requiredSize =  sizeof(ULONG);       
        if (BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        newIdleTime = *(PULONG) Buffer;
        newIdleTime = max(newIdleTime, PCIDRV_MIN_IDLE_TIME);
        //
        // Convert that to ms time units
        //
        newIdleTime = (LONGLONG) -SECOND_TO_100NS * newIdleTime;
        prevIdleTime = fdoData->ConservationIdleTime;
        fdoData->ConservationIdleTime = newIdleTime;
        
        if(IsPoMgmtSupported(fdoData) && 
            newIdleTime != prevIdleTime){ // if not setting the same value
            
            //
            // Reset the timer by deregistering and registering the Idle
            // detection.
            //
            PciDrvDeregisterIdleDetection(fdoData, FALSE);
            PciDrvRegisterForIdleDetection(fdoData, FALSE);
            
        }
        status = STATUS_SUCCESS;
        break;
        
    case WMI_POWER_PERFORMANCE_IDLE_TIME:

        requiredSize = sizeof(ULONG);              
        if (BufferSize < requiredSize) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        newIdleTime = *(PULONG) Buffer;
        newIdleTime = max(newIdleTime, PCIDRV_MIN_IDLE_TIME);
        //
        // Convert that to ms time units
        //
        newIdleTime = (LONGLONG) -SECOND_TO_100NS * newIdleTime;
        prevIdleTime = fdoData->PerformanceIdleTime;
        fdoData->PerformanceIdleTime = newIdleTime;
        
        if(IsPoMgmtSupported(fdoData) && 
            newIdleTime != prevIdleTime){ // if not setting the same value
                        //
            // Reset the timer by deregistering and registering the Idle
            // detection.
            //
            PciDrvDeregisterIdleDetection(fdoData, FALSE);
            PciDrvRegisterForIdleDetection(fdoData, FALSE);                
        }
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
PciDrvQueryWmiDataBlock(
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

    InstanceCount is the number of instances expected to be returned for
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
    PFDO_DATA               fdoData;
    NTSTATUS                status = STATUS_WMI_GUID_NOT_FOUND;
    ULONG                   size = 0;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_WMI, "Entered PciDrvQueryWmiDataBlock\n");

    //
    // Only ever registers 1 instance per guid
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    switch (GuidIndex) {
    case WMI_PCIDRV_DRIVER_INFORMATION:

        size = sizeof (PCIDRV_WMI_STD_DATA);
        if (OutBufferSize < size ) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        //
        // Copy the structure information
        //
        * (PPCIDRV_WMI_STD_DATA) Buffer = fdoData->StdDeviceData;

        *InstanceLengthArray = size ;
        status = STATUS_SUCCESS;
        break;


    case WMI_POWER_DEVICE_WAKE_ENABLE:

        //
        // Here we return the current preference of the user for wait-waking
        // the system. We read(IoOpenDeviceRegistryKey/ZwQueryValueKey)
        // the default value written by the INF file in the HW registery.
        // If the user changes his preference, then we must record
        // the changes in the registry to have that in affect across
        // boots.
        //

        size = sizeof(BOOLEAN);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        if(IsPoMgmtSupported(fdoData)){
            *(PBOOLEAN) Buffer = PciDrvGetWaitWakeEnableState(fdoData) ;
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;
        }
        break;

    case WMI_POWER_DEVICE_ENABLE:

        //
        // The same rule discussed above applies here
        // for responding to the query.
        //
        size = sizeof(BOOLEAN);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if(IsPoMgmtSupported(fdoData)){

            *(PBOOLEAN) Buffer = PciDrvGetPowerSaveEnableState(fdoData);
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;
        }
        
        break;

    case WMI_POWER_CONSERVATION_IDLE_TIME:

        size = sizeof(ULONG);
        
        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        //
        // Convert the value back to secs.
        //
        *(PULONG) Buffer = (ULONG)(fdoData->ConservationIdleTime/-SECOND_TO_100NS);
        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;
        break;

    case WMI_POWER_PERFORMANCE_IDLE_TIME:

        size = sizeof(ULONG);
        
        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        //
        // Convert the value back to secs.
        //
        *(PULONG) Buffer = (ULONG)(fdoData->PerformanceIdleTime/-SECOND_TO_100NS);
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
PciDrvQueryWmiRegInfo(
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
        registered for this device. If the device wants enable and disable
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
        device if the WMIREG_FLAG_INSTANCE_PDO flag is returned in
        *RegFlags.

Return Value:

    status

--*/
{
    PFDO_DATA fdoData;

    PAGED_CODE();

    DebugPrint(TRACE, DBG_WMI, "Entered PciDrvQueryWmiRegInfo\n");

    fdoData = DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = fdoData->UnderlyingPDO;
    RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);

    return STATUS_SUCCESS;
}


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



