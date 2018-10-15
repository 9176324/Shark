/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Wmi.c

Abstract:

    This stage of Wmi.c builds upon the support implemented in the previous
    stage, Featured1\Wmi.c.

    New features implemented in this stage of Wmi.c:

    -The list of GUIDs supported by the function driver is changed to support WPP
     software tracing.

    -The wait/wake routines implemented in Wake.c are called to arm or disarm the
     hardware instance. The hardware instance is only armed if the function
     driver's WaitWakeEnabled Registry key indicates the hardware instance can
     signal wake-up.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub – 26-Oct-1998
     Updated (9/28/2000) to demonstrate:
     -How to bring power management tab in the device manager and
      enable the check boxes. The Featured1 stage of the Toaster sample driver
      does not demonstrate what a driver specifically needs to perform to process
      wait-wake and power device enable. The Featured2 stage demonstrates these.
     -How to fire a WMI event to notify device arrival.

    Kevin Shirley – 01-Jul-2003 – Commented for tutorial and learning purposes

--*/


#include "toaster.h"
#include <initguid.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>
#include <stdio.h>

#if defined(EVENT_TRACING)
//
// If the Featured2\source. file defined the EVENT_TRACING token when the BUILD
// utility compiles the sources to the function driver, the WPP preprocessor
// generates a trace message header (.tmh) file by the same name as the source file.
// 
// The trace message header file must be included in the function driver's sources
// after defining a WPP control GUID (which is defined in Toaster.h as
// WPP_CONTROL_GUIDS) and before any calls to WPP macros, such as WPP_INIT_TRACING.
// 
// During compilation, the WPP preprocessor examines the source files for
// DoTraceMessage() calls and builds the .tmh file, which stores a unique GUID for
// each message, the text resource string for each message, and the data types
// of the variables passed in for each message.
//
#include <wmi.tmh> 
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ToasterWmiRegistration)
#pragma alloc_text(PAGE,ToasterWmiDeRegistration)
#pragma alloc_text(PAGE,ToasterSystemControl)
#pragma alloc_text(PAGE,ToasterSetWmiDataItem)
#pragma alloc_text(PAGE,ToasterSetWmiDataBlock)
#pragma alloc_text(PAGE,ToasterQueryWmiDataBlock)
#pragma alloc_text(PAGE,ToasterQueryWmiRegInfo)
#endif

#define MOFRESOURCENAME L"ToasterWMI"

#if defined(WIN2K) && defined(EVENT_TRACING)  
#define NUMBER_OF_WMI_GUIDS             4
#else 
#define NUMBER_OF_WMI_GUIDS             3
#endif
#define WMI_TOASTER_DRIVER_INFORMATION  0
#define TOASTER_NOTIFY_DEVICE_ARRIVAL   1
#define WMI_POWER_DEVICE_WAKE_ENABLE    2

WMIGUIDREGINFO ToasterWmiGuidList[] =
{
    {
        &TOASTER_WMI_STD_DATA_GUID,
        1,
        0
    },
    {
        &TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &GUID_POWER_DEVICE_WAKE_ENABLE,
        1,
        0
    }
#if defined(WIN2K) && defined(EVENT_TRACING)  
    ,
    {
        &WPP_TRACE_CONTROL_NULL_GUID,        
        1,                                            
        0
    }
#endif 
};

extern ULONG DebugLevel;

 
NTSTATUS
ToasterWmiRegistration(
    PFDO_DATA               FdoData
)
/*++

Updated Routine Description:
    ToasterWmiRegistration does not change in this stage of the function driver.

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    FdoData->WmiLibInfo.GuidCount = sizeof (ToasterWmiGuidList) /
                                 sizeof (WMIGUIDREGINFO);
    ASSERT (NUMBER_OF_WMI_GUIDS == FdoData->WmiLibInfo.GuidCount);
    FdoData->WmiLibInfo.GuidList = ToasterWmiGuidList;
    FdoData->WmiLibInfo.QueryWmiRegInfo = ToasterQueryWmiRegInfo;
    FdoData->WmiLibInfo.QueryWmiDataBlock = ToasterQueryWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataBlock = ToasterSetWmiDataBlock;
    FdoData->WmiLibInfo.SetWmiDataItem = ToasterSetWmiDataItem;
    FdoData->WmiLibInfo.ExecuteWmiMethod = NULL;
    FdoData->WmiLibInfo.WmiFunctionControl = ToasterFunctionControl;

    status = IoWMIRegistrationControl(FdoData->Self,
                             WMIREG_ACTION_REGISTER
                             );

    FdoData->StdDeviceData.ConnectorType = TOASTER_WMI_STD_USB;
    FdoData->StdDeviceData.Capacity = 2000;
    FdoData->StdDeviceData.ErrorCount = 0;
    FdoData->StdDeviceData.Controls = 5;
    FdoData->StdDeviceData.DebugPrintLevel = DebugLevel;

    return status;
}

 
NTSTATUS
ToasterWmiDeRegistration(
    PFDO_DATA               FdoData
)
/*++

Updated Routine Description:
    ToasterWmiDeRegistration does not change in this stage of the function driver.

--*/
{
    PAGED_CODE();

    return IoWMIRegistrationControl(FdoData->Self,
                                 WMIREG_ACTION_DEREGISTER
                                 );

}

 
NTSTATUS
ToasterSystemControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Updated Routine Description:
    ToasterSystemControl calls the WPP_TRACE_CONTROL macro to log the execution of
    the function driver through WPP software tracing.

--*/
{
    PFDO_DATA               fdoData;
    SYSCTL_IRP_DISPOSITION  disposition;
    NTSTATUS                status;
    PIO_STACK_LOCATION      stack;

    PAGED_CODE();

    stack = IoGetCurrentIrpStackLocation (Irp);

    ToasterDebugPrint(TRACE, "FDO %s\n",
                WMIMinorFunctionString(stack->MinorFunction));

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterIoIncrement (fdoData);

    if (Deleted == fdoData->DevicePnPState)
    {
        Irp->IoStatus.Status = status = STATUS_NO_SUCH_DEVICE;

        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        ToasterIoDecrement (fdoData);

        return status;
    }

    status = WmiSystemControl(&fdoData->WmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 &disposition);
#if defined(WIN2K) && defined(EVENT_TRACING)

    if ((disposition != IrpProcessed) && 
            (DeviceObject == (PDEVICE_OBJECT)stack->Parameters.WMI.ProviderId))
    {
        ULONG returnSize=0;

        ToasterDebugPrint(INFO, "Calling WPP_TRACE_CONTROL\n");

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
            break;
        }

        case IrpNotCompleted:
        {
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            break;
        }

        case IrpForward:

        case IrpNotWmi:
        {
            IoSkipCurrentIrpStackLocation (Irp);

            status = IoCallDriver (fdoData->NextLowerDriver, Irp);

            break;
        }

        default:
        {
            ASSERT(FALSE);

            IoSkipCurrentIrpStackLocation (Irp);

            status = IoCallDriver (fdoData->NextLowerDriver, Irp);

            break;
        }
    }

    ToasterIoDecrement(fdoData);

    return(status);
}

 

//
// Begin WMI call back routines
// ----------------------------
//

NTSTATUS
ToasterSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
/*++

Updated Routine Description:
    ToasterSetWmiDataItem arms or disarms the hardware instance to signal wake-up,
    if the function driver's WaitWakeEnabled Registry key indicates the hardware
    instance can support wait/wake.

--*/
{
    PFDO_DATA   fdoData;
    NTSTATUS    status;
    BOOLEAN     waitWakeEnabled;
    ULONG       requiredSize = 0;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterSetWmiDataItem\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    switch(GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:
        if(5 == DataItemId)
        {
            requiredSize = sizeof(ULONG);
            if (BufferSize < requiredSize)
            {
                status = STATUS_BUFFER_TOO_SMALL;

                break;
            }

            DebugLevel = fdoData->StdDeviceData.DebugPrintLevel =
                                    *((PULONG)Buffer);

            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_WMI_READ_ONLY;
        }

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:

        requiredSize = sizeof(BOOLEAN);
        if (BufferSize < requiredSize)
        {
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // The wait/wake value passed from WMI is based on the result of the function
        // driver's call attempt to wait/wake.
        //
        waitWakeEnabled = *(PBOOLEAN) Buffer;

        ToasterSetWaitWakeEnableState(fdoData, waitWakeEnabled);

        if (TRUE == waitWakeEnabled)
        {
            //
            // If wait/wake is supported, then arm the hardware instance to signal
            // wake-up.
            //
            ToasterArmForWake(fdoData, FALSE);
        }
        else
        {
            //
            // If wait/wake is not supported, then disarm the hardware instance from
            // signaling wake-up.
            //
            ToasterDisarmWake(fdoData, FALSE);
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

    return status;
}

 
NTSTATUS
ToasterSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
/*++

Updated Routine Description:
    ToasterSetWmiDataItem arms or disarms the hardware instance to signal wake-up,
    if the function driver's WaitWakeEnabled Registry key indicates the hardware
    instance can support wait/wake.

--*/
{
    PFDO_DATA   fdoData;
    NTSTATUS    status;
    BOOLEAN     waitWakeEnabled;
    ULONG       requiredSize = 0;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Entered ToasterSetWmiDataBlock\n");

    switch(GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:

        requiredSize = sizeof(TOASTER_WMI_STD_DATA);

        if (BufferSize < requiredSize)
        {
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        DebugLevel = fdoData->StdDeviceData.DebugPrintLevel =
                    ((PTOASTER_WMI_STD_DATA)Buffer)->DebugPrintLevel;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:

        requiredSize = sizeof(BOOLEAN);

        if (BufferSize < requiredSize)
        {
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // The wait/wake value passed from WMI is based on the result of the function
        // driver's call attempt to wait/wake.
        //
        waitWakeEnabled = *(PBOOLEAN) Buffer;

        ToasterSetWaitWakeEnableState(fdoData, waitWakeEnabled);

        if (TRUE == waitWakeEnabled)
        {
            //
            // If wait/wake is supported, then arm the hardware instance to signal
            // wake-up.
            //
            ToasterArmForWake(fdoData, FALSE);
        }
        else
        {
            //
            // If wait/wake is not supported, then disarm the hardware instance from
            // signaling wake-up.
            //
            ToasterDisarmWake(fdoData, FALSE);
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
ToasterQueryWmiDataBlock(
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

Updated Routine Description:
    ToasterQueryWmiDataBlocks returns whether the hardware instance can arm to
    signal wake-up.

--*/
{
    PFDO_DATA               fdoData;
    NTSTATUS    status;
    ULONG       size = 0;

    WCHAR       modelName[]=L"Aishwarya\0\0";

    USHORT      modelNameLen = (wcslen(modelName) + 1) * sizeof(WCHAR);

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterQueryWmiDataBlock\n");

    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    switch (GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:
        size = sizeof (TOASTER_WMI_STD_DATA) + modelNameLen + sizeof(USHORT);

        if (OutBufferSize < size )
        {
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        *(PTOASTER_WMI_STD_DATA) Buffer = fdoData->StdDeviceData;

        Buffer += sizeof (TOASTER_WMI_STD_DATA);

        *((PUSHORT)Buffer) = modelNameLen;

        Buffer = (PUCHAR)Buffer + sizeof(USHORT);

        RtlCopyBytes((PVOID)Buffer, (PVOID)modelName, modelNameLen);

        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:
        size = sizeof(BOOLEAN);

        if (OutBufferSize < size)
        {
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        *(PBOOLEAN) Buffer = ToasterGetWaitWakeEnableState(fdoData);

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
ToasterFunctionControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
/*++

Updated Routine Description:
    ToasterFunctionControl does not change in this stage of the function driver.

--*/
{
    NTSTATUS status;

    status = WmiCompleteRequest(
                                     DeviceObject,
                                     Irp,
                                     STATUS_SUCCESS,
                                     0,
                                     IO_NO_INCREMENT);

    return(status);
}


//
// End WMI call back routines
// --------------------------
//

 
NTSTATUS ToasterFireArrivalEvent(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Updated Routine Description:
    ToasterFireArrivalEvent does not change in this stage of the function driver.

--*/
{

    PWNODE_SINGLE_INSTANCE  wnode;
    ULONG                   wnodeSize;
    ULONG                   wnodeDataBlockSize;
    ULONG                   wnodeInstanceNameSize;
    PUCHAR                  ptmp;
    ULONG                   size;
    UNICODE_STRING          deviceName;
    UNICODE_STRING          modelName;
    NTSTATUS                status;
    PFDO_DATA               fdoData = DeviceObject->DeviceExtension;

    RtlInitUnicodeString(&modelName, L"Sonali\0\0");

    status = GetDeviceFriendlyName (fdoData->UnderlyingPDO,
                                        &deviceName);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    wnodeSize = sizeof(WNODE_SINGLE_INSTANCE);

    wnodeInstanceNameSize = deviceName.Length + sizeof(USHORT);

    wnodeDataBlockSize = modelName.Length + sizeof(USHORT);

    size = wnodeSize + wnodeInstanceNameSize + wnodeDataBlockSize;

    wnode = ExAllocatePoolWithTag (NonPagedPool, size, TOASTER_POOL_TAG);

    if (NULL != wnode)
    {
        RtlZeroMemory(wnode, size);

        wnode->WnodeHeader.BufferSize = size;

        wnode->WnodeHeader.ProviderId =
                            IoWMIDeviceObjectToProviderId(DeviceObject);

        wnode->WnodeHeader.Version = 1;

        KeQuerySystemTime(&wnode->WnodeHeader.TimeStamp);

        RtlCopyMemory(&wnode->WnodeHeader.Guid,
                        &TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT,
                        sizeof(GUID));

        wnode->WnodeHeader.Flags = WNODE_FLAG_EVENT_ITEM |
                                    WNODE_FLAG_SINGLE_INSTANCE;

        wnode->OffsetInstanceName = wnodeSize;

        wnode->DataBlockOffset = wnodeSize + wnodeInstanceNameSize;

        wnode->SizeDataBlock = wnodeDataBlockSize;

        ptmp = (PUCHAR)wnode + wnode->OffsetInstanceName;

        *((PUSHORT)ptmp) = deviceName.Length;

        RtlCopyMemory(ptmp + sizeof(USHORT),
                            deviceName.Buffer,
                            deviceName.Length);

        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

        *((PUSHORT)ptmp) = modelName.Length;

        RtlCopyMemory(ptmp + sizeof(USHORT),
                        modelName.Buffer,
                        modelName.Length);

        status = IoWMIWriteEvent(wnode);

        if (!NT_SUCCESS(status))
        {
            ToasterDebugPrint(ERROR, "IoWMIWriteEvent failed %x\n", status);
            ExFreePool(wnode);
        }

    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ExFreePool(deviceName.Buffer);

    return status;
}

 
NTSTATUS
ToasterQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
/*++

Updated Routine Description:
    ToasterQueryWmiRegInfo does not change in this stage of the function driver.

--*/
{
    PFDO_DATA fdoData;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterQueryWmiRegInfo\n");

    fdoData = DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;

    *RegistryPath = &Globals.RegistryPath;

    *Pdo = fdoData->UnderlyingPDO;

    RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);

    return STATUS_SUCCESS;
}

 
NTSTATUS
GetDeviceFriendlyName(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PUNICODE_STRING DeviceName
    )
/*++

Updated Routine Description:
    GetDeviceFriendlyName does not change in this stage of the function driver.

--*/
{
    NTSTATUS                    status;
    ULONG                       resultLength = 0;
    DEVICE_REGISTRY_PROPERTY    property;
    PWCHAR                      valueInfo = NULL;
    ULONG                       i;

    valueInfo = NULL;

    property = DevicePropertyFriendlyName;

    for (i = 0; i < 2; i++)
    {
        status = IoGetDeviceProperty(Pdo,
                                       property,
                                       0,
                                       NULL,
                                       &resultLength);

        if (STATUS_BUFFER_TOO_SMALL != status)
        {
            property = DevicePropertyDeviceDescription;
        }
    }

    valueInfo = ExAllocatePoolWithTag (NonPagedPool, resultLength,
                                                TOASTER_POOL_TAG);
    if (NULL == valueInfo)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto Error;
    }

    status = IoGetDeviceProperty(Pdo,
                                   property,
                                   resultLength,
                                   valueInfo,
                                   &resultLength);
    if (!NT_SUCCESS(status))
    {
        ToasterDebugPrint(ERROR, "IoGetDeviceProperty returned %x\n", status);

        goto Error;
    }

    RtlInitUnicodeString(DeviceName, valueInfo);

    return STATUS_SUCCESS;

Error:
    if (NULL != valueInfo)
    {
        ExFreePool(valueInfo);
    }

    return (status);
}


 
PCHAR
WMIMinorFunctionString (
    UCHAR MinorFunction
)
/*++

Updated Routine Description:
    WMIMinorFunctionString does not change in this stage of the function driver.

--*/
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



