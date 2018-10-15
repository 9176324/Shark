/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:


Abstract:


Author:

    Kumar Rajeev  

Environment:

    kernel mode only

Notes:


Revision History:


--*/
#ifndef _VHIDMINI_

#define _VHIDMINI_

#include <wdm.h>
#include <hidport.h>
#include "common.h"   //for control code definitions

#if DBG
#define DebugPrint(_x_)     { DbgPrint ("VHIDMINI:"); DbgPrint _x_; }
#else 
#define DebugPrint(_x_)     
#endif

#define VHID_POOL_TAG (ULONG) 'diHV'
#define VHID_HARDWARE_IDS    L"HID\\MyVirtualHidDevice\0\0"
#define VHID_HARDWARE_IDS_LENGTH sizeof (VHID_HARDWARE_IDS)

#define INPUT_REPORT_BYTES    0x01
#define CONTROL_FEATURE_REPORT_ID   0x01

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

//
// This is the default report descriptor for the virtual Hid device returned
// by the mini driver in response to IOCTL_HID_GET_REPORT_DESCRIPTOR if the
// driver fails to read the report descriptor from registry. 
//
HID_REPORT_DESCRIPTOR           DefaultReportDescriptor[] = {
    0x06,0x00, 0xFF,                // USAGE_PAGE (Vender Defined Usage Page)     
    0x09,0x01,                          // USAGE (Vendor Usage 0x01)      
    0xA1,0x01,                           // COLLECTION (Application)        
    0x85,CONTROL_FEATURE_REPORT_ID,           // REPORT_ID (1)                      
    0x09,0x01,                          // USAGE (Vendor Usage 0x01)              
    0x15,0x00,                          // LOGICAL_MINIMUM(0)                   
    0x26,0xff, 0x00,                // LOGICAL_MAXIMUM(255)               
    0x75,0x08,                          // REPORT_SIZE (0x08)                     
    0x95,0x01,                          // REPORT_COUNT (0x01)                    
    0xB1,0x00,                          // FEATURE (Data,Ary,Abs)
    0x09,0x01,                          // USAGE (Vendor Usage 0x01)              
    0x75,0x08,                          // REPORT_SIZE (0x08)                     
    0x95,INPUT_REPORT_BYTES,           // REPORT_COUNT (0x01)                    
    0x81,0x00,                      // INPUT (Data,Ary,Abs)
    0xC0                                // END_COLLECTION                       

};

//
// This is the default HID descriptor returned by the mini driver
// in response to IOCTL_HID_GET_DEVICE_DESCRIPTOR. The size
// of report descriptor is currently the size of DefaultReportDescriptor.
// If the report descriptor is successfully read from the registry, it is 
// modified to corresponding size of report descriptor.
//

HID_DESCRIPTOR              DefaultHidDescriptor = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
    { 0x22,   // report descriptor type 0x22
    sizeof(DefaultReportDescriptor) }  // total length of report descriptor
};

//
//These are the device attributes returned by the mini driver in response 
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//

#define HIDMINI_PID                 0xFEED
#define HIDMINI_VID                 0xBEEF
#define HIDMINI_VERSION          0x0101

//
// These are the states FDO transition to upon
// receiving a specific PnP Irp. Refer to the PnP Device States
// diagram in DDK documentation for better understanding.
//

typedef enum _DEVICE_PNP_STATE {

    NotStarted = 0,         // Not started yet
    Started,                // Device has received the START_DEVICE IRP
    StopPending,            // Device has received the QUERY_STOP IRP
    Stopped,                // Device has received the STOP_DEVICE IRP
    RemovePending,          // Device has received the QUERY_REMOVE IRP
    SurpriseRemovePending,  // Device has received the SURPRISE_REMOVE IRP
    Deleted                 // Device has received the REMOVE_DEVICE IRP

} DEVICE_PNP_STATE;

BOOLEAN x = TRUE;

#define INITIALIZE_PNP_STATE(_Data_)    \
        (_Data_)->DevicePnPState =  NotStarted;\
        (_Data_)->PreviousPnPState = NotStarted;

#define SET_NEW_PNP_STATE(_Data_, _state_) \
        (_Data_)->PreviousPnPState =  (_Data_)->DevicePnPState;\
        (_Data_)->DevicePnPState = (_state_);

#define RESTORE_PREVIOUS_PNP_STATE(_Data_)   \
        (_Data_)->DevicePnPState =   (_Data_)->PreviousPnPState;\

typedef struct _DEVICE_EXTENSION{

    HID_DESCRIPTOR              HidDescriptor;

    PHID_REPORT_DESCRIPTOR          ReportDescriptor;

    BOOLEAN                           ReadReportDescFromRegistry;

    DEVICE_PNP_STATE    DevicePnPState;   // Track the state of the device

    DEVICE_PNP_STATE    PreviousPnPState; // Remembers the previous pnp state

} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

typedef struct _READ_TIMER{
    
    KDPC             ReadTimerDpc;
    KTIMER          ReadTimer;
    PIRP              Irp;
  
} READ_TIMER, * PREAD_TIMER;


#define GET_MINIDRIVER_DEVICE_EXTENSION(DO) \
    ((PDEVICE_EXTENSION) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->MiniDeviceExtension))

#define GET_NEXT_DEVICE_OBJECT(DO) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->NextDeviceObject)

#define GET_PHYSICAL_DEVICE_OBJECT(DO) \
    (((PHID_DEVICE_EXTENSION)(DO)->DeviceExtension)->PhysicalDeviceObject)

//
// driver routines
//

NTSTATUS 
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING registryPath
    );

NTSTATUS 
PnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
PnPComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS 
Power(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
SystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS  
AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT FunctionalDeviceObject
    );

VOID 
Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS 
InternalIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
GetHidDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
GetReportDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS 
GetAttributes(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    );

NTSTATUS 
GetDeviceAttributes(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp);

NTSTATUS
GetSetFeature(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

PCHAR
PnPMinorFunctionString(
    UCHAR MinorFunction
    );

NTSTATUS
HandleControlRequests(
    IN PHID_XFER_PACKET TransferPacket
    );

NTSTATUS
ReadDescriptorFromRegistry(
        IN PDEVICE_OBJECT       DeviceObject
        );

NTSTATUS 
ReadReport(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    );

VOID 
ReadTimerDpcRoutine(   
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS
CheckRegistryForDescriptor(
        IN PDEVICE_OBJECT  DeviceObject
        );

/*
//
// Here is sample descriptor that has two top level collection - mouse 
// collection and  vendor defined collection with a custom feature item. If 
// you want to provide sideband communication with your hidmini 
// driver, you can add a custom collection with the collection provided 
// by the hardware and open the custom collection from an app to 
// communicate with the driver.
//
HID_REPORT_DESCRIPTOR           DefaultReportDescriptor[] = {
        0x05, 0x01,     //Usage Page (Generic Desktop),
        0x09, 0x02,     //Usage (Mouse),        
        0xA1, 0x01,     //Collection (Application),
        0x85, 0x01,     //REPORT_ID (1)             
        0x09, 0x01,     //Usage (Pointer),
        0xA1, 0x00,     //Collection (Physical),
        0x05, 0x09,     //Usage Page (Buttons),
        0x19, 0x01,     //Usage Minimum (01),
        0x29, 0x03,     //Usage Maximun (03),
        0x15, 0x00,     //Logical Minimum (0),
        0x25, 0x01,     //Logical Maximum (1),
        0x95, 0x03,     //Report Count (3),
        0x75, 0x01,     //Report Size (1),
        0x81, 0x02,     //Input (Data, Variable, Absolute), ;3 button bits
        0x95, 0x01,     //Report Count (1),
        0x75, 0x05,     //Report Size (5),
        0x81, 0x01,     //Input (Constant), ;5 bit padding
        0x05, 0x01,     //Usage Page (Generic Desktop),
        0x09, 0x30,     //Usage (X),
        0x09, 0x31,     //Usage (Y),
        0x15, 0x81,     //Logical Minimum (-127),
        0x25, 0x7F,     //Logical Maximum (127),
        0x75, 0x08,     //Report Size (8),
        0x95, 0x02,     //Report Count (2),
        0x81, 0x06,     //input (Data, Variable, Relative), ;2 position bytes (X & Y)
        0xC0,             //End Collection,
        0xC0,             //End Collection,

        0x06,0x00, 0xFF,   // USAGE_PAGE (Vender Defined Usage Page)     
        0x09,0x01,           // USAGE (Vendor Usage 0x01)      
        0xA1,0x01,           // COLLECTION (Application)        
        0x85,0x02,           // REPORT_ID (2)                      
        0x09,0x01,           // USAGE (Vendor Usage 0x01)              
        0x15,0x00,           // LOGICAL_MINIMUM(0)                   
        0x26,0xff, 0x00,   // LOGICAL_MAXIMUM(255)               
        0x75,0x08,           // REPORT_SIZE (0x08)                     
        0x95,0x01,           // REPORT_COUNT (0x01)                    
        0xB1,0x00,           // FEATURE (Data,Ary,Abs)             
        0xC0                    // END_COLLECTION                       
};

*/

#endif

