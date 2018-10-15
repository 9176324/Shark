/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    wmi.c

Abstract: This module demonstrates how to receive WMI notification fired by 
          another driver. The code here basically registers for toaster 
          device arrival WMI notification fired by the toaster function driver.
          You can use similar technique to receive media sense notification 
          (GUID_NDIS_STATUS_MEDIA_CONNECT/GUID_NDIS_STATUS_MEDIA_DISCONNECT) 
          fired by NDIS whenever the network cable is plugged or unplugged.

Author:   Eliyas Yakub  Oct 27, 2003

Environment:

    Kernel mode

Revision History:

--*/
#include "toastmon.h"
#include "public.h"
#include <wmistr.h>

//
// These typedefs required to avoid compilation errors in Win2K build environment.
//
typedef
VOID 
(*WMI_NOTIFICATION_CALLBACK)( // Copied from WDM.H
    PVOID Wnode,
    PVOID Context
    );

typedef
NTSTATUS
(*PFN_IO_WMI_OPEN_BLOCK)(
    IN GUID  *DataBlockGuid,
    IN ULONG  DesiredAccess,
    OUT PVOID  *DataBlockObject
    );

typedef
NTSTATUS
(*PFN_IO_WMI_SET_NOTIFICATION_CALLBACK)(
    IN PVOID  Object,
    IN WMI_NOTIFICATION_CALLBACK  Callback,
    IN PVOID  Context
    );

NTSTATUS
RegisterForWMINotification(
    PDEVICE_EXTENSION DeviceExt
    )
{
    NTSTATUS           status = STATUS_SUCCESS;
    GUID               wmiGuid;
    UNICODE_STRING funcName;
    PFN_IO_WMI_OPEN_BLOCK WmiOpenBlock = NULL;
    PFN_IO_WMI_SET_NOTIFICATION_CALLBACK WmiSetNotificationCallback = NULL;

    PAGED_CODE();

    //
    // APIs  to open WMI interfaces are available on XP and beyond, so let us
    // first check to see whether there are exported in the kernel we are running
    // before using them.
    //    
    RtlInitUnicodeString(&funcName, L"IoWMIOpenBlock");
    WmiOpenBlock = MmGetSystemRoutineAddress(&funcName);
        
    RtlInitUnicodeString(&funcName, L"IoWMISetNotificationCallback");
    WmiSetNotificationCallback = MmGetSystemRoutineAddress(&funcName);

    if(WmiOpenBlock == NULL || WmiSetNotificationCallback==NULL) {
        //
        // Not available.
        //
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Check to make sure we are not called multiple times.
    //
    if (DeviceExt->WMIDeviceArrivalNotificationObject != NULL) {
        return STATUS_UNSUCCESSFUL;
    }
    
    //
    // Register WMI callbacks for toaster device arrival events
    //
    wmiGuid = TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT;
    
    status = WmiOpenBlock(
                 &wmiGuid,
                 WMIGUID_NOTIFICATION,
                 &DeviceExt->WMIDeviceArrivalNotificationObject
                 );
    if (!NT_SUCCESS(status)) {
        
        DebugPrint(("Unable to open wmi data block status 0x%x\n", status));
        DeviceExt->WMIDeviceArrivalNotificationObject = NULL;
        
    } else {

        status = WmiSetNotificationCallback(
                     DeviceExt->WMIDeviceArrivalNotificationObject,
                     WmiNotificationCallback,
                     DeviceExt
                     );
        if (!NT_SUCCESS(status)) {
            DebugPrint(("Unable to register for wmi notification 0x%x\n", status));
            ObDereferenceObject(DeviceExt->WMIDeviceArrivalNotificationObject);
            DeviceExt->WMIDeviceArrivalNotificationObject = NULL;
        }
    }

    return status;    
}


VOID
UnregisterForWMINotification(
    PDEVICE_EXTENSION DeviceExt
)
{
    if (DeviceExt->WMIDeviceArrivalNotificationObject != NULL) {
        ObDereferenceObject(DeviceExt->WMIDeviceArrivalNotificationObject);
        DeviceExt->WMIDeviceArrivalNotificationObject = NULL;
    }
}

NTSTATUS
GetDeviceFriendlyName(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PUNICODE_STRING DeviceName
    )
/*++

Routine Description:

    Return the friendly name associated with the given device object.

Arguments:

Return Value:

    NT status

--*/
{
    NTSTATUS                    status;
    ULONG                       resultLength = 0;
    DEVICE_REGISTRY_PROPERTY    property;
    PWCHAR                      valueInfo = NULL;
    ULONG                       i;

    valueInfo = NULL;

    //
    // First get the length of the string. If the FriendlyName
    // is not there then get the lenght of device description.
    //
    property = DevicePropertyFriendlyName;

    for (i = 0; i < 2; i++) {
        status = IoGetDeviceProperty(Pdo,
                                       property,
                                       0,
                                       NULL,
                                       &resultLength);

        if (status != STATUS_BUFFER_TOO_SMALL) {
            property = DevicePropertyDeviceDescription;
        }
    }

    valueInfo = ExAllocatePoolWithTag (NonPagedPool, resultLength,
                                                DRIVER_TAG);
    if (NULL == valueInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

    //
    // Now we have the right size buffer, we can get the name
    //
    status = IoGetDeviceProperty(Pdo,
                                   property,
                                   resultLength,
                                   valueInfo,
                                   &resultLength);
    if (!NT_SUCCESS(status)) {
        DebugPrint(("IoGetDeviceProperty returned %x\n", status));
        goto Error;
    }

    RtlInitUnicodeString(DeviceName, valueInfo);

    return STATUS_SUCCESS;

Error:
    if (valueInfo) {
        ExFreePool(valueInfo);
    }
    return (status);
}

VOID
WmiNotificationCallback(
    IN PVOID Wnode,
    IN PVOID Context
    )
/*++

Routine Description:

    WMI calls this function to notify the caller that the specified event has occurred.
    
Arguments:

    Wnode - points to the WNODE_EVENT_ITEM structure returned by the driver triggering the event. 

    Context - points to the value specified in the Context parameter of the
                    IoWMISetNotificationCallback routine. 

Return Value:

    NT status

--*/    
{
    PWNODE_SINGLE_INSTANCE wnode = (PWNODE_SINGLE_INSTANCE) Wnode;
    PDEVICE_EXTENSION deviceExtension = Context;
    UNICODE_STRING deviceName;
    PLIST_ENTRY            thisEntry, listHead;
    PDEVICE_INFO                list;
    PDEVICE_OBJECT pdo, devobj;
    NTSTATUS status;
    BOOLEAN         found = FALSE;

    ExAcquireFastMutex (&deviceExtension->ListMutex);

    listHead = &deviceExtension->DeviceListHead;

    for(thisEntry = listHead->Flink;
        thisEntry != listHead;
        thisEntry = thisEntry->Flink)
    {
        list = CONTAINING_RECORD(thisEntry, DEVICE_INFO, ListEntry);
        devobj = list->TargetDeviceObject;

        if(devobj && IoWMIDeviceObjectToProviderId(devobj) 
                            == wnode->WnodeHeader.ProviderId) {
        
            if( IsEqualGUID( (LPGUID)&(wnode->WnodeHeader.Guid),
                          (LPGUID)&TOASTER_NOTIFY_DEVICE_ARRIVAL_EVENT) ) {
                //
                // Currently IRQL level is APC_LEVEL since we have acquired the mutex.
                // So make sure to release the mutext before doing anything
                // that can be done only at PASSIVE_LEVEL such as sending PNP
                // IRPs.
                //
                found = TRUE;                          
                break;
                    
            } else {
                DebugPrint(("Unknown event.\n"));    
            }
        }
                
    }

    ExReleaseFastMutex (&deviceExtension->ListMutex);

    //
    // Get the friendlyname of the device.
    //
    if(found) {
          
        status = GetDeviceFriendlyName(list->Pdo, &deviceName);
        if (!NT_SUCCESS(status)) {
            DebugPrint(("GetDeviceFriendlyName returned %x\n", status));
            return;
        }
        
        DebugPrint(("%ws fired a device arrival event\n", deviceName.Buffer));
        //
        // Free the memory allocated by GetDeviceFriendlyName.
        //
        ExFreePool(deviceName.Buffer);
    }

    return;
}
