/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Wmi.c

Abstract:

    Wmi.c implements the code required for the Toaster sample function driver to
    support the WMI library.

    Features implemented in this stage of the function driver:

    -A dispatch routine to process IRP_MJ_SYSTEM_CONTROL IRPs. The WMI library
     sends IRP_MJ_SYSTEM_CONTROL IRPs to support power management and to return
     measurement and instrumentation metrics from the hardware instance. This
     routine is called ToasterSystemControl.

    -How to show the "Power Management" tab in the hardware instance's Properties
     dialog.
     If the function driver registers either a GUID_POWER_DEVICE_ENABLE or
     GUID_POWER_DEVICE_WAKE_ENABLE data block with WMI, then the Device Manager
     shows the Power Management tab in the hardware instance's Properties dialog.

    -How to return measurement and instrumentation metrics to the system or an
     application.
     The function driver registers TOASTER_WMI_STD_DATA_GUID which allows the
     function driver to return various metrics about the hardware instance, such
     as the kilowatt capacity of the hardware instance, the number of errors that
     have occurred on the hardware instance, and the model name of the hardware.

    -How to change the function driver's level of debug output.

    -How to notify the system and registered applications of new device arrivals.

    The data blocks that the function driver registers are described by GUIDs in
    the Toaster.mof (Managed Object Format) file. The function driver defines two
    custom data blocks that correspond to the TOASTER_WMI_STD_DATA_GUID and
    TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT GUIDs defined in the Public.h file.

    The build process compiles the Toaster.mof file into a binary mof file called
    Toaster.bmf, and then attaches Toaster.bmf to the function driver's resulting
    Toaster.sys binary image. The function driver then registers with WMI and
    indicates that the binary WMI data is attached as a resource to the function
    driver's image.

Environment:

    Kernel mode

Revision History:

    Eliyas Yakub  26-Oct-1998
     Updated (9/28/2000) to demonstrate:
     -How to bring power management tab in the device manager and
      enable the check boxes. The Featured1 stage of the Toaster sample driver
      does not demonstrate what a driver specifically needs to perform to process
      wait-wake and power device enable. The Featured2 stage demonstrates these.
     -How to fire a WMI event to notify device arrival.

    Kevin Shirley  23-Jun-2003 - Commented for tutorial and learning purposes

--*/


#include "toaster.h"
#include <initguid.h>
#include <wdmguid.h>
#include <wmistr.h>
#include <wmilib.h>
#include <stdio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ToasterWmiRegistration)
#pragma alloc_text(PAGE,ToasterWmiDeRegistration)
#pragma alloc_text(PAGE,ToasterSystemControl)
#pragma alloc_text(PAGE,ToasterSetWmiDataItem)
#pragma alloc_text(PAGE,ToasterSetWmiDataBlock)
#pragma alloc_text(PAGE,ToasterQueryWmiDataBlock)
#pragma alloc_text(PAGE,ToasterQueryWmiRegInfo)
#endif

//
// Define the resource from the toaster.rc resource file that represents the binary
// MOF (managed object format) data implemented by the function driver. The original
// resource file, "toaster.mof" is compiled into a binary mof (.bmf file),
// "toaster.bmf" during the build process. the resulting BMF is then included into
// the driver image, where it is referenced by "ToasterWMI".
//
#define MOFRESOURCENAME L"ToasterWMI"

//
// Define the number of GUIDs, and their respective index (order) in the
// ToasterWmiGuidList array. When the function driver registers the array and
// calculates the number of data blocks in the array, the calculation is compared
// with the NUMBER_OF_GUIDS contstnace. They must be equal.
//
#define NUMBER_OF_WMI_GUIDS             4
#define WMI_TOASTER_DRIVER_INFORMATION  0
#define TOASTER_NOTIFY_DEVICE_ARRIVAL   1
#define WMI_POWER_DEVICE_WAKE_ENABLE    2
#define WMI_POWER_DEVICE_ENABLE         3

//
// Declare the WMI data blocks to export from the function driver. The first two
// entries in the array correspond to the two WMI data blocks in the Toaster.mof
// file. The last two entries in the array are system-defined GUIDs. They inform
// the Device Manager to show the Power Management tab in the driver's properties
// because the function driver implements power management support.
//
// Defined only a single instance of each data block. No data block requires any
// special flags except TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT because it represents
// an event.
//
WMIGUIDREGINFO ToasterWmiGuidList[] =
 {
    //
    // The first data block corresponds to the "class ToasterDeviceInformation" data
    // block in the Toaster.mof file. The ToasterDeviceInformation block contains six
    // individual data items, such as ConnectorType and DebugPrintLevel.
    //
    {
        &TOASTER_WMI_STD_DATA_GUID,
         1,
        0
    },
    //
    // The second data block corresponds to the "class ToasterNotifyDeviceArrival"
    // WMI event data block in the Toaster.mof file. The ToasterNotifyDeviceArrival
    // block contains a single event item which is related to the
    // ToasterFireArrivalEvent routine.
    //
    // The WMIREG_FLAG_EVENT_ONLY_GUID flag informs WMI that the data block described
    // by the TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT GUID represents an event. The data
    // block can be enabled or disable only. It cannot be queried or set.
    //
    {
        &TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    //
    // If the function driver for a device handles the following GUIDs, then the
    // Device Manager shows the Power Manager tab in the Device Properties dialog.
    //
    // The following GUIDs are system defined. They do not need to appear in the
    // Toaster.mof file.
    //
    // GUID_POWER_DEVICE_WAKE_ENABLE indicates that the function driver implements
    // wait/wake support.
    //
    // The meaning of GUID_POWER_DEVICE_ENABLE is up to the hardware instance.
    //
     {
        &GUID_POWER_DEVICE_WAKE_ENABLE,
        1,
        0
    },
    {
        &GUID_POWER_DEVICE_ENABLE,
        1,
        0
    }
};

//
// Global debug error level that is externally defined in Toaster.c
//
extern ULONG DebugLevel;

 
NTSTATUS
ToasterWmiRegistration(
    PFDO_DATA               FdoData
)
/*++

New Routine Description:
    ToasterWmiRegistration registers the function driver as a WMI data provider
     for the hardware instance described by the FdoData parameter.

    The function driver calls ToasterWmiRegistration when ToasterDispatchPnP
    processes IRP_MN_START_DEVICE and calls ToasterStartDevice.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that is registering to be a WMI data provider.

Return Value Description:
    ToasterWmiRegistration returns the value returned by IoWMIRegistrationControl.
    STATUS_SUCCESS indicates the function driver successfully registered to be a
    WMI data provider. Any other return value indicates that the function driver
    did not successfully register to be a WMI data provider.

--*/
{
    NTSTATUS status;

    PAGED_CODE();

    //
    // Initialize the members of the WmiLibInfo member. The function driver
    // specifies:
    //
    // -The number of GUIDs the function driver exposes
    //
    // -The list of GUIDs exposed by the function driver (ToasterWmiGuidLst).
    //
    // -The function driver's callback routine that provides information about the
    //  data blocks and event blocks to be registered by the function driver
    //  (ToasterQueryWmiRegInfo).
    //
    // -The function driver's callback routine to return either a single instance or
    //  all instances of a data block (ToasterQueryWmiDataBlock).
    //
    // -The function driver's callback routine to change all data items in a single
    //  instance of a data block (ToasterSetWmiDataBlock).
    //
    // -The function driver's callback routine to change a single data item in an
    //  instance of a data block (ToasterSetWmiDataItem).
    //
    // -The function driver's callback routine to execute a method associated with a
    //  data block. This feature is not demonstrated by the function driver. This
    //  member is set to NULL.
    //
    // -The function driver's callback routine to enable or disable notification of
    //  events, and enable or disable data collection for data blocks that the
    //  function driver designated as expensive to collect (ToasterFunctionControl).
    //
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

    //
    // Call IoWMIRegistrationControl to register the function driver as a WMI data
    // provider. The function driver later calls IoWMIRegistrationControl to
    // deregister from providing WMI data when ToasterDispatchPnP processes
    // IRP_MN_REMOVE_DEVICE and calls ToasterWMIDeRegistration.
    //
    // WMIREG_ACTION_REGISTER informs WMI to add the hardware instance's DeviceObject
    // to WMI's list of data providers. IoWMIRegistrationControl also increments the
    // reference count of the device object (FdoData->Self).
    //
    status = IoWMIRegistrationControl(FdoData->Self,
                             WMIREG_ACTION_REGISTER
                             );

    //
    // Initialize the members of the StdDeviceData member to describe the hardware
    // instance's standard device data, including:
    //
    // -How the toaster is connected to the computer.
    //
    // -The capacity in kilowatts of the toaster device.
    //
    // -The number of errors the toaster device has detected.
    //
    // -The number of device controls on the toaster device.
    //
    // -The debug output level of the toaster device.
    //
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

New Routine Description:
    ToasterWmiDeRegistration deregisters the function driver from providing WMI
    data for the hardware instance described by the FdoData parameter.

    The function driver calls ToasterWmiDeRegistration when ToasterDispatchPnP
    processes IRP_MN_REMOVE_DEVICE.

Parameters Description:
    FdoData
    FdoData represents the device extension of the FDO of the hardware instance
    that is deregistering to be a WMI data provider.

Return Value Description:
    ToasterWmiDeRegistration returns the value returned by
    IoWMIRegistrationControl. STATUS_SUCCESS indicates the function driver
    successfully deregistered from providing WMI data.

--*/
{
    PAGED_CODE();

    //
    // Call IoWMIRegistrationControl to deregister the function driver from providing
    // WMI data for the hardware instance. The function driver called
    // IoWMIRegistrationControl earlier to register to provide WMI data when
    // ToasterDispatchPnP processes IRP_MN_START_DEVICE and calls ToasterStartDevice.
    //
    // WMIREG_ACTION_DEREGISTER informs WMI to remove the hardware instance's
    // DeviceObject from WMI's list of data providers. IoWMIRegistrationControl also
    // decrements the reference count of the device object (FdoData->Self).
    //
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
    ToasterSystemControl passes the incoming WMI IRP to the WMI library to be
    processed. The WMI library will call the appropriate function driver provided
    callback routine.

Updated Return Value Description:
    ToasterSystemControl returns STATUS_NO_SUCH_DEVICE if the hardware instance
    represented by DeviceObject has been removed. Otherwise ToasterSystemControl
    returns the status of the WMI operation returned by WmiSystemControl, or the
    status of the incoming WMI IRP from the underlying bus driver.

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

    //
    // Pass the incoming WMI IRP to the WMI library for processing. The WMI library
    // will call the function driver's callback routines to process the IRP, if
    // necessary.
    //
    status = WmiSystemControl(&fdoData->WmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 &disposition);
    //
    // After WmiSystemControl returns, ToasterSystemControl must continue to process
    // the WMI IRP based on the IRP's disposition. That is, while the WMI library,
    // and the function driver's callback routines (called by WMI if necessary)
    // processed the IRP, the IRP may or may not have been passed down the device
    // stack to be processed by the underlying driver.
    //
    switch(disposition)
    {
        case IrpProcessed:
        {
            //
            // If the WMI IRP's disposition indicates that the IRP has already been
            // processed then the function driver must not need complete it again.
            // Otherwise a system crash results because an IRP must only be completed
            // once.
            //
            break;
        }

        case IrpNotCompleted:
        {
            //
            // If the WMI IRP's disposition indicates that the IRP has not been
            // completed, then the function driver must complete it.
            //
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            break;
        }

        case IrpForward:
            //
            // If the WMI IRP's disposition indicates that the IRP should be
            // forwarded down the device stack to be processed by the underlying bus
            // driver then fall-through to the next case statement, which passes the
            // IRP down the device stack.
            //

        case IrpNotWmi:
        {
            //
            // If the incoming IRP's disposition indicates that the IRP is not a WMI
            // IRP, or if the incoming IRP must be forwarded down the device stack,
            // then call IoCallDriver to pass the IRP down the device stack to be
            // processed by the underlying bus driver.
            //
            IoSkipCurrentIrpStackLocation (Irp);

            status = IoCallDriver (fdoData->NextLowerDriver, Irp);

            break;
        }

        default:
        {
            //
            // If the disposition is undefined, then pass the incoming IRP down the
            // device stack. The disposition should always be defined. However,
            // providing the default case statement which passes the IRP down guards
            // against system crashes on future versions of the operating system that
            // might (for some reason) add a new disposition for WMI IRPs. Drivers
            // that do not account for this possibility could cause a system crash
            // because they didn't properly handle such circumstances.
            //

            //
            // A checked build of the function driver will assert for any such IRPs.
            // However in the free build, this assert does nothing.
            //
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

New Routine Description:
    The WMI library calls back ToasterQueryWmiDataBlock to set the contents of a
    data block.

    If the function driver can complete the query WMI IRP within the callback,
    then it should call WmiCompleteRequest to complete the incoming WMI IRP before
    returning to the caller.

    The function driver can return STATUS_PENDING if it cannot immediately
    complete the incoming WMI IRP. The driver must then call WmiCompleteRequest
    after the data has changed.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the set WMI data item operation associated with the DeviceObject
    parameter.

    GuidIndex
    GuidIndex represents the index of the WMI data item in the ToasterWmiGuidList
    array that was passed to the WMI library when the function driver called
    ToasterWmiRegistration.

    InstanceIndex
    InstanceIndex represents the instance of the WMI data block being set.

    DataItemId
    DataItemId represents the Id of the data item being set as the data item is
    specified in the Toaster.mof file.

    BufferSize
    BufferSize represents the size of the caller-allocated buffer described by the
    Buffer parameter.

    Buffer
    Buffer represents the data buffer for the WMI data item.

Return Value Description:
    ToasterSetWmiDataItem returns STATUS_BUFFER_TOO_SMALL if the BufferSize
    parameter is less than the size that is required to fulfill the IRP.
    ToasterSetWmiDataItem returns STATUS_WMI_READ_ONLY if the DataItemId parameter
    specifies a data item that cannot be written to.
    ToasterSetWmiDataItem returns STATUS_WMI_GUID_NOT_FOUND if the incoming WMI
    IRP does not correspond to a GUID supported by the function driver.
    Otherwise ToasterSetWmiDataItem returns the value returned by
    WmiCompleteRequest.

--*/
{
    PFDO_DATA   fdoData;
    NTSTATUS    status;
    ULONG       requiredSize = 0;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterSetWmiDataItem\n");

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    switch(GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:
        //
        // WMI_TOASTER_DRIVER_INFORMATION corresponds to the first entry in the
        // ToasterWmiGuidList array, which must match the first data block in the
        // Toaster.mof file. Set the 5th data item in the data block. The 5th data
        // item is the debugging output level.
        //
        if(5 == DataItemId)
        {
            requiredSize = sizeof(ULONG);

            if (BufferSize < requiredSize)
            {
                //
                // Fail the WMI IRP if the BufferSize parameter is less than the size
                // that is required to fulfill the IRP. The 5th data item in the
                // Toaster.mof file is a UINT32 (the same as a ULONG).
                //
                status = STATUS_BUFFER_TOO_SMALL;

                break;
            }

            //
            // Set the function driver's global debug level to the value passed from
            // WMI. WMI passes the value in the Buffer parameter.
            //
            DebugLevel = fdoData->StdDeviceData.DebugPrintLevel =
                                    *((PULONG)Buffer);

            status = STATUS_SUCCESS;
        }
        else
        {
            //
            // Fail the incoming WMI IRP if the DataItemId parameter is for a data
            // item that is read-only.
            //
            status = STATUS_WMI_READ_ONLY;
        }

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:
        //
        // WMI_POWER_DEVICE_WAKE_ENABLE corresponds to the third entry in the
        // ToasterWmiGuidList array, which must match the third data block in the
        // Toaster.mof file. Set the device extension's wait/wake arming member to
        // fulfill the set WMI data item IRP.
        //
        requiredSize = sizeof(BOOLEAN);

        if (BufferSize < requiredSize)
        {
            //
            // Fail the WMI IRP if the BufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // Set the device extension's AllowWakeArming member to the value passed from
        // WMI. WMI passes the value in the Buffer parameter.
        //
        // The Featured2 stage of the function driver demonstrates how to process
        // wait/wake power operations.
        //
        fdoData->AllowWakeArming = *(PBOOLEAN) Buffer;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_ENABLE:
        //
        // Meaning of this request ("Allow the computer to turn off this
        // device to save power") is device dependent. For example NDIS
        // driver interpretation of this checkbox is different from Serial
        // class drivers.
        //
        requiredSize = sizeof(BOOLEAN);

        if (BufferSize < requiredSize)
        {
            //
            // Fail the WMI IRP if the BufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }
        
        //
        // Set the device extension's AllowIdleDetectionRegistration member to the
        // value passed from WMI. WMI passes the value in the Buffer parameter.
        //
        fdoData->AllowIdleDetectionRegistration = *(PBOOLEAN) Buffer;

        status = STATUS_SUCCESS;

        break;

     default:
        //
        // Fail any incoming WMI IRP that does not correspond to a GUID in the
        // ToasterWmiGuidList array that the function driver registered earlier in
        // ToasterWmiRegistration.
        //
        status = STATUS_WMI_GUID_NOT_FOUND;

    }

    //
    // Call WmiCompleteRequest to indicate that the function driver has finished
    // processing the WMI IRP.
    //
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

New Routine Description:
    The WMI library calls back ToasterQueryWmiDataBlock to set the contents of a
    data block.

    If the function driver can complete the query WMI IRP within the callback,
    then it should call WmiCompleteRequest to complete the incoming WMI IRP before
    returning to the caller.

    The function driver can return STATUS_PENDING if it cannot immediately
    complete the incoming WMI IRP. The driver must then call WmiCompleteRequest
    after the data has changed.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the set WMI data block operation associated with DeviceObject.

    GuidIndex represents the index of the WMI data item in the ToasterWmiGuidList
    array that was passed to the WMI library when the function driver called
    ToasterWmiRegistration.

    InstanceIndex
    InstanceIndex represents the instance of the WMI data block being set.

    BufferSize
    BufferSize represents the size of the caller-allocated buffer described by the
    Buffer parameter.

    Buffer
    Buffer represents the data buffer for the WMI data item.

Return Value Description:
    ToasterSetWmiDataBlock returns STATUS_BUFFER_TOO_SMALL if the OutBufferSize
    parameter is less than the size that is required to fulfill the IRP.
    ToasterSetWmiDataBlock returns STATUS_WMI_GUID_NOT_FOUND if the incoming WMI
    IRP does not correspond to a GUID supported by the function driver.
    Otherwise ToasterSetWmiDataBlock returns the value returned by
    WmiCompleteRequest.

--*/
{
    PFDO_DATA   fdoData;
    NTSTATUS    status;
    ULONG       requiredSize = 0;

    PAGED_CODE();

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    ToasterDebugPrint(TRACE, "Entered ToasterSetWmiDataBlock\n");

    switch(GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:
        //
        // WMI_TOASTER_DRIVER_INFORMATION corresponds to the first entry in the
        // ToasterWmiGuidList array, which must match the first data block in the
        // Toaster.mof file. Set only the writable data items of the
        // WMI_TOASTER_DRIVER_INFORMATION data block to fulfill the set WMI data
        // block IRP.
        //
        requiredSize = sizeof(TOASTER_WMI_STD_DATA);

        if (BufferSize < requiredSize)
        {
            //
            // Fail the WMI IRP if the BufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        DebugLevel = fdoData->StdDeviceData.DebugPrintLevel =
                    ((PTOASTER_WMI_STD_DATA)Buffer)->DebugPrintLevel;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:
        //
        // WMI_POWER_DEVICE_WAKE_ENABLE corresponds to the third entry in the
        // ToasterWmiGuidList array, which must match the third data block in the
        // Toaster.mof file. Set the device extension's wait/wake arming member to
        // fulfill the set WMI data item IRP.
        //

        requiredSize = sizeof(BOOLEAN);

        if (BufferSize < requiredSize)
        {
            //
            // Fail the WMI IRP if the BufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // Set the device extension's AllowWakeArming member to the value passed from
        // WMI. WMI passes the value in the Buffer parameter.
        //
        // The Featured2 stage of the function driver demonstrates how to process
        // wait/wake power operations.
        //
        fdoData->AllowWakeArming = *(PBOOLEAN) Buffer;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_ENABLE:
        //
        // Meaning of this request ("Allow the computer to turn off this
        // device to save power") is device dependent. For example NDIS
        // driver interpretation of this checkbox is different from Serial
        // class drivers.
        //
        requiredSize = sizeof(BOOLEAN);

        if (BufferSize < requiredSize)
        {
            //
            // Fail the WMI IRP if the BufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // Set the device extension's AllowIdleDetectionRegistration member to the
        // value passed from WMI. WMI passes the value in the Buffer parameter.
        //
        fdoData->AllowIdleDetectionRegistration = *(PBOOLEAN) Buffer;

        status = STATUS_SUCCESS;

        break;
 
    default:
        //
        // Fail any incoming WMI IRP that does not correspond to a GUID in the
        // ToasterWmiGuidList array that the function driver registered earlier in
        // ToasterWmiRegistration.
        //
        status = STATUS_WMI_GUID_NOT_FOUND;

    }

    //
    // Call WmiCompleteRequest to indicate that the function driver has finished
    // processing the WMI IRP.
    //
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

New Routine Description:
    The WMI library calls back ToasterQueryWmiDataBlock to query for the contents
    of a data block.

    If the function driver can complete the query WMI IRP within the callback,
    then it should call WmiCompleteRequest to complete the incoming WMI IRP before
    returning to the caller.

    The function driver can return STATUS_PENDING if it cannot immediately
    complete the incoming WMI IRP. The driver must then call WmiCompleteRequest
    after the data has changed.

Parameters Description:
    DeviceObject is the device whose data block is being queried
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the query WMI data block operation associated with DeviceObject.

    GuidIndex
    GuidIndex represents the index of the WMI data item in the ToasterWmiGuidList
    array that was passed to the WMI library when the function driver called
    ToasterWmiRegistration.

    InstanceIndex
    InstanceIndex represents the instance of the WMI data block being queried.

    InstanceCount
    InstanceCount represents the number of instances the WMI library expects the
    function driver to return for the data block.

    InstanceLengthArray
    InstanceLengthArray represents a pointer to an array of ULONG values that
    describe the lengths of each instance of the data block. If this is NULL then
    there was not enough space in the output buffer to fulfill the request so the
    function driver should complete the WMI IRP with the buffer size the function
    driver requires to fulfill the IRP.

    OutBufferSize
    OutBufferSize represents the size of the caller-allocated buffer described by
    the Buffer parameter.

    Buffer on return is filled with the returned data block

Return Value Description:
    ToasterQueryWmiDataBlock returns STATUS_BUFFER_TOO_SMALL if the OutBufferSize
    parameter is less than the size that is required to fulfill the IRP.
    ToasterQueryWmiDataBlock returns STATUS_WMI_GUID_NOT_FOUND if the incoming WMI
    IRP does not correspond to a GUID supported by the function driver.
    Otherwise ToasterQueryWmiDataBlock returns the value returned by
    WmiCompleteRequest.

--*/
{
    PFDO_DATA               fdoData;
    NTSTATUS    status;
    ULONG       size = 0;

    //
    // Terminate the model name with 2 NULLs because the data type is a WCHAR
    //
    WCHAR       modelName[]=L"Aishwarya\0\0";

    //
    // Calculate the size of the model name including the WCHAR NULL
    // terminator.
    //
    USHORT      modelNameLen = (USHORT)(wcslen(modelName) + 1) * sizeof(WCHAR);

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterQueryWmiDataBlock\n");

    //
    // Test the assumption that the function driver only registered a single instance
    // per GUID in the ToasterWmiGuidList array. That is, the InstanceCount member of
    // the array's WMIGUIDREGINFO data type equals 1.
    //
    ASSERT((InstanceIndex == 0) &&
           (InstanceCount == 1));

    fdoData = (PFDO_DATA) DeviceObject->DeviceExtension;

    switch (GuidIndex)
    {
    case WMI_TOASTER_DRIVER_INFORMATION:
        //
        // WMI_TOASTER_DRIVER_INFORMATION corresponds to the first entry in the
        // ToasterWmiGuidList array, which must match the first data block in the
        // Toaster.mof file. Get the data items of the WMI_TOASTER_DRIVER_INFORMATION
        // data block to fulfill the set WMI data block IRP.
        //

        //
        // Calculate the required buffer size to fulfill the query WMI IRP.
        //
        size = sizeof (TOASTER_WMI_STD_DATA) + modelNameLen + sizeof(USHORT);

        if (OutBufferSize < size )
        {
            //
            // Fail the WMI IRP if the OutBufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // Copy the hardware instance's "standard device data" into the query WMI
        // IRP's data buffer.
        //

        //
        // Point the incoming Buffer parameter to the device extension's standard
        // device data structure.
        //
        *(PTOASTER_WMI_STD_DATA) Buffer = fdoData->StdDeviceData;

        //
        // Advance the Buffer pointer offset by the size of the standard device data
        // structure.
        //
        Buffer += sizeof (TOASTER_WMI_STD_DATA);

        //
        // Copy the length of the model name into the current buffer offset.
        //
        *((PUSHORT)Buffer) = modelNameLen;

        //
        // Advance the Buffer pointer offset by the size of the length's data type.
        //
        Buffer = (PUCHAR)Buffer + sizeof(USHORT);

        //
        // Copy the contents of the model name variable into the Buffer at the
        // Buffer's current offset.
        //
        RtlCopyBytes((PVOID)Buffer, (PVOID)modelName, modelNameLen);

        //
        // Return the size of the data copied into the query WMI IRP's Buffer
        // parameter.
        //
        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_WAKE_ENABLE:
        //
        // WMI_POWER_DEVICE_WAKE_ENABLE corresponds to the third entry in the
        // ToasterWmiGuidList array, which must match the third data block in the
        // Toaster.mof file. Set the device extension's wait/wake arming member to
        // fulfill the set WMI data item IRP.
        //
        size = sizeof(BOOLEAN);

        if (OutBufferSize < size)
        {
            //
            // Fail the WMI IRP if the OutBufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        //
        // Return the user's preference for wait/waking the system to the WMI
        // library. The function driver should read the default value written to the
        // Registry by the INF file that was used to install the driver. To read the
        // default value from the Registry, the function driver should use the
        // IoOpenDeviceRegistryKey and ZwQueryValueKey system calls.
        //
        // If the user changes his preference, then the function driver must record
        // the change to the Registry so that the change persists between reboots.
        // To save the value to the Registry, the function driver should use the
        // ZwSetValueKey system call.
        //
        // The Featured2 stage of the function driver demonstrates how to process
        // wait/wake power operations.
        //
        *(PBOOLEAN) Buffer = fdoData->AllowWakeArming;

        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;

        break;

    case WMI_POWER_DEVICE_ENABLE:
        //
        // Here we return the current preference of the user for wait-waking
        // the system. We read(IoOpenDeviceRegistryKey/ZwQueryValueKey)
        // the default value written by the INF file in the HW registery.
        // If the user changes his preference, then we must record
        // the changes in the registry to have that in affect across
        // boots.
        // Note: Featured2 WMI.C implements this.
        size = sizeof(BOOLEAN);

        if (OutBufferSize < size)
        {
            //
            // Fail the WMI IRP if the OutBufferSize parameter is less than the size
            // that is required to fulfill the IRP.
            //
            status = STATUS_BUFFER_TOO_SMALL;

            break;
        }

        *(PBOOLEAN) Buffer = fdoData->AllowIdleDetectionRegistration;

        *InstanceLengthArray = size;

        status = STATUS_SUCCESS;

        break;
 
    default:
        //
        // Fail any incoming WMI IRP that does not correspond to a GUID in the
        // ToasterWmiGuidList array that the function driver registered earlier in
        // ToasterWmiRegistration.
        //
        status = STATUS_WMI_GUID_NOT_FOUND;

    }

    //
    // Call WmiCompleteRequest to indicate that the function driver has finished
    // processing the WMI IRP.
    //
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

New Routine Description:
    The WMI library calls back ToasterFunctionControl to enable or disable
    notification of events, and to enable or disable collection for data blocks
    that the function driver registered as expensive to collect. A function driver
    should only expect a single enable when the first event or data consumer
    enables events or data collection and a single disable when the last event or
    data consumer disables events or data collection. Data blocks only receive
    enable and disable notifications if the function driver registered them as
    requiring notification.

    If the function driver can complete enabling/disabling within the callback,
    then it should call WmiCompleteRequest to complete the incoming WMI IRP before
    returning to the caller.

    The function driver can return STATUS_PENDING if it cannot immediately
    complete the incoming WMI IRP. The driver must then call WmiCompleteRequest
    after the data has changed.

    Normally a function driver should save the value of the incoming Enable
    parameter in the device extension and then check the value before firing an
    event. However, since the function driver fires an event to indicate a new
    device arrival, the it cannot wait for WMI to send an IRP_MN_ENABLE_EVENTS WMI
    IRP. Thus the function driver does not check the Enable value.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is associated with the
    incoming Irp parameter. DeviceObject is an FDO created earlier in
    ToasterAddDevice.

    Irp
    Irp represents the set WMI data item operation associated with DeviceObject.

    GuidIndex
    GuidIndex represents the index of the WMI data item in the ToasterWmiGuidList
    array passed to the WMI library when the function driver called
    ToasterWmiRegistration.

    Function
    Function represents the functionality to enable or disable.

    Enable
    Enable represents whether to enable the function specified in the Function
    parameter. TRUE specifies that the function is to be enabled. FALSE specifies
    that the function is to be disabled.

Return Value Description:
    ToasterFunctionControl returns the value returned by WmiCompleteRequest.

--*/
{
    NTSTATUS status;

    //
    // Call WmiCompleteRequest to indicate that the function driver has finished
    // processing the WMI IRP with STATUS_SUCCESS.
    //
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

New Routine Description:
    ToasterFireArrivalEvent notifies WMI that the new hardware instance
    represented by the DeviceObject parameter has been started. WMI then notifies
    any applications that have registered for device arrival notification that the
    new hardware instance has started.

    ToasterStartDevice calls ToasterFireArrivalEvent after the hardware instance
    is fully powered and ready to process requests.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance to fire an arrival event for.
    DeviceObject is an FDO created earlier in ToasterAddDevice.

Return Value Description:
    ToasterFireArrivalEvent returns STATUS_INSUFFICIENT_RESOURCES if it cannot
    allocate memory to fire the arrival event. Otherwise, ToasterFireArrivalEvent
    returns the value from the IoWMIWriteEvent call.

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

    //
    // Get the friendly name of the hardware instance.
    //
    status = GetDeviceFriendlyName (fdoData->UnderlyingPDO,
                                        &deviceName);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    //
    // Calculate the amount of memory to allocate for the WNODE.
    //
    wnodeSize = sizeof(WNODE_SINGLE_INSTANCE);

    wnodeInstanceNameSize = deviceName.Length + sizeof(USHORT);

    wnodeDataBlockSize = modelName.Length + sizeof(USHORT);

    size = wnodeSize + wnodeInstanceNameSize + wnodeDataBlockSize;

    //
    // Allocate memory for the WNODE from the non-paged pool.
    //
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

        //
        // Specify the WNODE_FLAG_EVENT_ITEM and WNODE_FLAG_SINGLE_INSTANCE flags to
        // indicate that the function driver creates a dynamic instance name. The
        // function driver creates dynamic instance names because device arrival
        // notifications can fire at any time. If the function driver used a static
        // instance name, then it can only fire WMI events after the WMI library
        // sends IRP_MN_REGINFO WMI IRP. The WMI library only sends IRP_MN_REGINFO
        // WMI IRPs after the PnP manager sends IRP_MN_START_DEVICE, which would
        // prevent the firing of new device arrival notification.
        //
        // If the function driver fired other events after firing a device arrival
        // notification, then it should determine if the event is enabled before it
        // fires the event. Otherwise nothing has registered to be notified of the
        // event.
        //
        wnode->WnodeHeader.Flags = WNODE_FLAG_EVENT_ITEM |
                                    WNODE_FLAG_SINGLE_INSTANCE;

        wnode->OffsetInstanceName = wnodeSize;

        wnode->DataBlockOffset = wnodeSize + wnodeInstanceNameSize;

        wnode->SizeDataBlock = wnodeDataBlockSize;

        //
        // Get the pointer to the start of the instance name block in the memory
        // allocated for the WNODE.
        //
        ptmp = (PUCHAR)wnode + wnode->OffsetInstanceName;

        //
        // Save the number of elements to the start of the instance name block in the
        // memory allocated for the WNODE.
        //
        *((PULONG)ptmp) = deviceName.Length;

        //
        // Copy the device's name after the number of elements in the instance name
        // block for the WNODE.
        //
        RtlCopyMemory(ptmp + sizeof(USHORT),
                            deviceName.Buffer,
                            deviceName.Length);

        //
        // Get the pointer to the start of the data block in the memory allocated for
        // the WNODE.
        //
        ptmp = (PUCHAR)wnode + wnode->DataBlockOffset;

        //
        // Save the number of elements to the start of the data block in the memory
        // allocated for the WNODE.
        //
        *((PUSHORT)ptmp) = modelName.Length;

        //
        // Copy the model's name after the number of elements in the data block for
        // the WNODE.
        //
        RtlCopyMemory(ptmp + sizeof(USHORT),
                        modelName.Buffer,
                        modelName.Length);

        //
        // Notify WMI of the arrival of the new hardware instance. The WMI library
        // releases the memory for the WNODE.
        //
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

    //
    // Release the memory that was allocated earlier in the GetDeviceFriendName
    // routine to hold the hardware instance's friendly name.
    //
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

New Routine Description:
    The WMI library calls ToasterQueryWmiRegInfo to provide information about the
    data blocks and event blocks to register for the function driver.

    A driver's WmiQueryRegInfo callback routine (this routine) must not block the
    thread executing it.

Parameters Description:
    DeviceObject
    DeviceObject represents the hardware instance that is being queried
    by the WMI library to provide data. DeviceObject is an FDO created
    earlier in ToasterAddDevice.

    RegFlags
    RegFlags represents the flags to return to the caller (the WMI library). The
    flags describe the GUIDs being registered for the hardware instance described
    by the DeviceObject parameter. ToasterQueryWmiRegInfo specifies the
    WMIREG_FLAG_INSTANCE_PDO to request WMI to generate a static instance name
    from the device instance ID for the PDO. If ToasterQueryWmiRegInfo does not
    return the WMIREG_FLAG_INSTANCE_PDO flag, then it must return a unique name
    in the InstanceName parameter.

    InstanceName
    InstanceName represents the unique base name for all instances of all blocks
    registered by the function driver. Because ToasterQueryRegInfo does not
    return the WMIREG_FLAG_INSTANCE_BASENAME flag, the InstanceName parameter is
    not used. In addition, if ToasterQueryWmiRegInfo does not return the
    WMIREG_FLAG_INSTANCE_PDO flag in the RegFlags parameter and returns a unique
    name in this parameter, the WMI library calls ExFreePool to release the
    memory for the unique instance name.

    RegistryPath
    RegistryPath represents driver specific path in the Registry to return to the
    caller (the WMI library). The system passed the driver specific path to the
    DriverEntry routine.

    MofResourceName
    MofResourceName represents the MOF (managed object format) resource name that
    is attached to the function driver's binary image. If the function driver does
    not have a MOF resource attached to the its binary image, then MofResourceName
    can be set to NULL.

    Pdo
    Pdo represents the hardware instance's physical device object to return to the
    caller (the WMI library). Because ToasterQueryWmiRegInfo returns the
    WMIREG_FLAG_INSTANCE_PDO, WMI uses the device instance path of the PDO
    parameter as a base from which to generate static instance names.

Return Value Description:
    ToasterQueryWmiRegInfo always returns STATUS_SUCCESS.

--*/
{
    PFDO_DATA fdoData;

    PAGED_CODE();

    ToasterDebugPrint(TRACE, "Entered ToasterQueryWmiRegInfo\n");

    fdoData = DeviceObject->DeviceExtension;

    //
    // Return the WMIREG_FLAG_INSTANCE_PDO to request WMI to generate a static
    // instance name from the underlying PDO.
    //
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;

    //
    // Return the driver specific path in the Registry that was saved in DriverEntry.
    //
    *RegistryPath = &Globals.RegistryPath;

    //
    // Return the underlying PDO so WMI can generate a static instance name.
    //
    *Pdo = fdoData->UnderlyingPDO;
 
    //
    // Return the MOF resource name that is attached to the binary image of the
    // function driver.
    //
    RtlInitUnicodeString(MofResourceName, MOFRESOURCENAME);

    return STATUS_SUCCESS;
}

 
NTSTATUS
GetDeviceFriendlyName(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PUNICODE_STRING DeviceName
    )
/*++

New Routine Description:
    GetDeviceFriendlyName returns the friendly name of the hardware instance, or
    its device description if the hardware instance does not have a friendly name.

    The caller must later call ExFreePool to release the memory allocated to hold
    the friendly name.

    The friendly name of a device can be set by writing a co-installer DLL. Refer
    to the toast.co.inf file, the Toaster sample coinstaller, and the DDK
    documentation for a more thorough explanation of friendly names.

Parameters Description:
    Pdo
    Pdo represents the physical device object of the hardware instance to return
    the friendly name of. Pdo is a PDO created by the underlying bus driver.

    DeviceName
    DeviceName represents caller allocated space that the hardware instance's
    friendly device name is copied to if GetDeviceFriendlyName succeeds.

Return Value Description:
    GetDeviceFriendlyName returns STATUS_INSUFFICIENT_RESOURCES if it cannot
    allocate memory to store the friendly name. Otherwise GetFriendlyName returns
    STATUS_SUCCESS.

--*/
{
    NTSTATUS                    status;
    ULONG                       resultLength = 0;
    DEVICE_REGISTRY_PROPERTY    property;
    PWCHAR                      valueInfo = NULL;
    ULONG                       i;

    property = DevicePropertyFriendlyName;

    //
    // Get the length of the hardware instance's friendly name. If the hardware
    // instance does not have a friendly name, then get the length of the device
    // description.
    //
    for (i = 0; i < 2; i++)
    {
        status = IoGetDeviceProperty(Pdo,
                                       property,
                                       0,
                                       NULL,
                                       &resultLength);

        if (STATUS_BUFFER_TOO_SMALL != status)
        {
            //
            // IoGetDeviceProperty returns some value other than
            // STATUS_BUFFER_TOO_SMALL if the hardware instance does not have a
            // friendly name because it cannot return a correct buffer size for
            // a non-existent string. Therefore set the next iteration of the
            // for loop to return the device description.
            //
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

    //
    // Get the name of the hardware instance's friendly name, or its device
    // description if getting the friendly name failed.
    //
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

    //
    // Initialize the incoming DeviceName parameter with the local copy of the
    // hardware instance's friendly name, or its device description if getting the
    // friendly name failed.
    //
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

New Routine Description:
    WMIMinorFunctionString converts the minor function code of a WMI IRP to a
    text string that is more helpful when tracing the execution of WMI IRPs.

Parameters Description:
    MinorFunction
    MinorFunction specifies the minor function code of a WMI IRP.

Return Value Description:
    WMIMinorFunctionString returns a pointer to a string that represents the
    text description of the incoming minor function code.

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



