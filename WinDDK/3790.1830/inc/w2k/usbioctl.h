/*++

Copyright (c) 1998-99      Microsoft Corporation

Module Name:

        USBIOCTL.H

Abstract:

   structures common to the USBD and USB device drivers.

Environment:

    Kernel & user mode

Revision History:

    09-29-95 : created
    01-06-97 : added user mode hub ioctls

--*/

#ifndef   __USBIOCTL_H__
#define   __USBIOCTL_H__

#include "usb100.h"

#ifndef FAR
#define FAR
#endif

/* f18a0e88-c30c-11d0-8815-00a0c906bed8 */
DEFINE_GUID( GUID_CLASS_USBHUB,    0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, \
             0xa0, 0xc9, 0x06, 0xbe, 0xd8);

/* A5DCBF10-6530-11D2-901F-00C04FB951ED */
DEFINE_GUID(GUID_CLASS_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, \
             0xC0, 0x4F, 0xB9, 0x51, 0xED);

/* 4E623B20-CB14-11D1-B331-00A0C959BBD2 */
DEFINE_GUID(GUID_USB_WMI_STD_DATA, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00,\
             0xA0, 0xC9, 0x59, 0xBB, 0xD2);

/* 4E623B20-CB14-11D1-B331-00A0C959BBD2 */
DEFINE_GUID(GUID_USB_WMI_STD_NOTIFICATION, 0x4E623B20L, 0xCB14, 0x11D1, 0xB3, 0x31, 0x00,\
             0xA0, 0xC9, 0x59, 0xBB, 0xD2);             


#define FILE_DEVICE_USB         FILE_DEVICE_UNKNOWN

/*
   USB IOCTLS
*/

#define USB_IOCTL_INTERNAL_INDEX       0x0000
#define USB_IOCTL_INDEX                0x00ff

/*
   USB Internal IOCtls
*/

/* IOCTL_INTERNAL_USB_SUBMIT_URB

   This IOCTL is used by client drivers to submit URB (USB Request Blocks)

   Parameters.Others.Argument1 = pointer to URB

*/

#define IOCTL_INTERNAL_USB_SUBMIT_URB  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX,  \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)


/* IOCTL_INTERNAL_USB_RESET_PORT
*/

#define IOCTL_INTERNAL_USB_RESET_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+1, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)


/*  IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO

    This IOCTL is used internally by the hub driver

    Parameters.Others.Argument1 = pointer to be filled in with RootHubPdo;
    Parameters.Others.Argument2 = pointer to be filled in with HcdDeviceObject;

*/

#define IOCTL_INTERNAL_USB_GET_ROOTHUB_PDO  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+3, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)



/* IOCTL_INTERNAL_USB_GET_PORT_STATUS

    This IOCTL returns the current port status

    Parameters.Others.Argument1 = pointer to port status register (ULONG)

    status bits are:

    USBD_PORT_ENABLED

*/

#define  USBD_PORT_ENABLED      0x00000001
#define  USBD_PORT_CONNECTED    0x00000002


#define IOCTL_INTERNAL_USB_GET_PORT_STATUS  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+4, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

/* IOCTL_INTERNAL_USB_ENABLE_PORT

    This IOCTL will request the hub to re-enable a disabled port
*/

#define IOCTL_INTERNAL_USB_ENABLE_PORT      CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+5, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

/* IOCTL_INTERNAL_USB_GET_HUB_COUNT

    This IOCTL is used internally by the hub driver

    Parameters.Others.Argument1 = pointer to be count of hubs in chain;

*/
#define IOCTL_INTERNAL_USB_GET_HUB_COUNT      CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+6, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

/* IOCTL_INTERNAL_USB_CYCLE_PORT

    This will simulate a plug/unplug on the port
*/

#define IOCTL_INTERNAL_USB_CYCLE_PORT  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+7, \
                                                METHOD_NEITHER,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_USB_GET_HUB_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INTERNAL_INDEX+8,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_INTERNAL_USB_GET_BUS_INFO  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+9,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)   

#define IOCTL_INTERNAL_USB_GET_CONTROLLER_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+10,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS) 

#define IOCTL_INTERNAL_USB_GET_BUSGUID_INFO   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+11,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)                                                  

#define IOCTL_INTERNAL_USB_GET_PARENT_HUB_INFO   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+12,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)                                                    
/*
   USB Public IOCtls
*/

/************************************************************
The following IOCTLS are always sent to the HCD symbolic 
name
*************************************************************/


/*
   this ioctl is for adding debug hooks to HCDs
*/

#define IOCTL_USB_HCD_GET_STATS_1          CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_HCD_GET_STATS_2          CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+11,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)                                                

/*
   These ioctls are used for USB diagnostic and test applications
*/

#define IOCTL_USB_DIAGNOSTIC_MODE_ON   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+1,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAGNOSTIC_MODE_OFF  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+2,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_ROOT_HUB_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+3,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)   

#define IOCTL_GET_HCD_DRIVERKEY_NAME CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+10,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)                                                
						
#define IOCTL_USB_HCD_DISABLE_PORT CTL_CODE(FILE_DEVICE_USB, \
                                            USB_IOCTL_INDEX+13, \
					    METHOD_BUFFERED, \
					    FILE_ANY_ACCESS)
					    
#define IOCTL_USB_HCD_ENABLE_PORT CTL_CODE(FILE_DEVICE_USB, \
                                           USB_IOCTL_INDEX+14, \
					   METHOD_BUFFERED, \
					   FILE_ANY_ACCESS)


/*********************************************************
The following IOCTLS are always sent to symbolic names 
created by usbhub
**********************************************************/                                                

/*
    Utility IOCTLS supported by the hub device
*/

/*
   These ioctls are supported by the hub driver for
   use by user mode USB utilities.
*/


#define IOCTL_USB_GET_NODE_INFORMATION   CTL_CODE(FILE_DEVICE_USB,  \
                                               USB_IOCTL_INDEX+3,  \
                                               METHOD_BUFFERED,  \
                                               FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION       CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+4,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+5,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_NAME     CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+6,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAG_IGNORE_HUBS_ON   CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+7,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_DIAG_IGNORE_HUBS_OFF  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+8,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

#define IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME  CTL_CODE(FILE_DEVICE_USB,  \
                                                USB_IOCTL_INDEX+9,  \
                                                METHOD_BUFFERED,  \
                                                FILE_ANY_ACCESS)

/*
   structures for user mode ioctls
*/

#include <pshpack1.h>

typedef enum _USB_HUB_NODE {
    UsbHub,
    UsbMIParent
} USB_HUB_NODE;

typedef struct _USB_HUB_INFORMATION {
    /*
       copy of data from hub descriptor
    */
    USB_HUB_DESCRIPTOR HubDescriptor;

    BOOLEAN HubIsBusPowered;

} USB_HUB_INFORMATION, *PUSB_HUB_INFORMATION;

typedef struct _USB_MI_PARENT_INFORMATION {
    ULONG NumberOfInterfaces;
} USB_MI_PARENT_INFORMATION, *PUSB_MI_PARENT_INFORMATION;

typedef struct _USB_NODE_INFORMATION {
    USB_HUB_NODE NodeType;        /* hub, mi parent */
    union {
        USB_HUB_INFORMATION HubInformation;
        USB_MI_PARENT_INFORMATION MiParentInformation;
    } u;
} USB_NODE_INFORMATION, *PUSB_NODE_INFORMATION;	

typedef struct _USB_PIPE_INFO {
    USB_ENDPOINT_DESCRIPTOR EndpointDescriptor;
    ULONG ScheduleOffset;
} USB_PIPE_INFO, *PUSB_PIPE_INFO;	


typedef enum _USB_CONNECTION_STATUS {
    NoDeviceConnected,
    DeviceConnected,

    /* failure codes, these map to fail reasons */
    DeviceFailedEnumeration,
    DeviceGeneralFailure,
    DeviceCausedOvercurrent,
    DeviceNotEnoughPower,
    
    DeviceNotEnoughBandwidth        
} USB_CONNECTION_STATUS, *PUSB_CONNECTION_STATUS;

typedef struct _USB_NODE_CONNECTION_INFORMATION {
    ULONG ConnectionIndex;
    /* usb device descriptor returned by this device
       during enumeration */
    USB_DEVICE_DESCRIPTOR DeviceDescriptor;
    UCHAR CurrentConfigurationValue;
    BOOLEAN LowSpeed;

    BOOLEAN DeviceIsHub;

    USHORT DeviceAddress;

    ULONG NumberOfOpenPipes;

    USB_CONNECTION_STATUS ConnectionStatus;
    USB_PIPE_INFO PipeList[0];
} USB_NODE_CONNECTION_INFORMATION, *PUSB_NODE_CONNECTION_INFORMATION;	

typedef struct _USB_NODE_CONNECTION_DRIVERKEY_NAME {
    ULONG ConnectionIndex;  /* INPUT */
    ULONG ActualLength;     /* OUTPUT */
    /* unicode name for the devnode. */
    WCHAR DriverKeyName[1];      /* OUTPUT */
} USB_NODE_CONNECTION_DRIVERKEY_NAME, *PUSB_NODE_CONNECTION_DRIVERKEY_NAME;	

typedef struct _USB_NODE_CONNECTION_NAME {
    ULONG ConnectionIndex;  /* INPUT */
    ULONG ActualLength;     /* OUTPUT */
    /* unicode symbolic name for this node if it is a hub or parent driver
       null if this node is a device. */
    WCHAR NodeName[1];      /* OUTPUT */
} USB_NODE_CONNECTION_NAME, *PUSB_NODE_CONNECTION_NAME;	


typedef struct _USB_HUB_NAME {
    ULONG ActualLength;     /* OUTPUT */
    /* NULL terminated unicode symbolic name for the root hub */
    WCHAR HubName[1];       /* OUTPUT */
} USB_HUB_NAME, *PUSB_HUB_NAME;

typedef struct _USB_ROOT_HUB_NAME {
    ULONG ActualLength;     /* OUTPUT */
    /* NULL terminated unicode symbolic name for the root hub */
    WCHAR RootHubName[1];   /* OUTPUT */
} USB_ROOT_HUB_NAME, *PUSB_ROOT_HUB_NAME;

typedef struct _USB_HCD_DRIVERKEY_NAME {
    ULONG ActualLength;     /* OUTPUT */
    /* NULL terminated unicode driverkeyname for hcd */
    WCHAR DriverKeyName[1];   /* OUTPUT */
} USB_HCD_DRIVERKEY_NAME, *PUSB_HCD_DRIVERKEY_NAME;	

typedef struct _USB_DESCRIPTOR_REQUEST {
    ULONG ConnectionIndex;
    struct {
        UCHAR bmRequest;
        UCHAR bRequest;
        USHORT wValue;
        USHORT wIndex;
        USHORT wLength;
    } SetupPacket;
    UCHAR Data[0];
} USB_DESCRIPTOR_REQUEST, *PUSB_DESCRIPTOR_REQUEST;	


/*
   Structure for returning HCD debug and statistic information to
   a user mode application.
*/

typedef struct _HCD_STAT_COUNTERS {
    ULONG BytesTransferred;

    USHORT IsoMissedCount;
    USHORT DataOverrunErrorCount;

    USHORT CrcErrorCount;
    USHORT ScheduleOverrunCount;

    USHORT TimeoutErrorCount;
    USHORT InternalHcErrorCount;

    USHORT BufferOverrunErrorCount;
    USHORT SWErrorCount;

    USHORT StallPidCount;
    USHORT PortDisableCount;

} HCD_STAT_COUNTERS, *PHCD_STAT_COUNTERS;


typedef struct _HCD_ISO_STAT_COUNTERS {

    USHORT  LateUrbs;
    USHORT  DoubleBufferedPackets;

    USHORT  TransfersCF_5ms;
    USHORT  TransfersCF_2ms;

    USHORT  TransfersCF_1ms;
    USHORT  MaxInterruptLatency;
    
    USHORT  BadStartFrame;
    USHORT  StaleUrbs;

    USHORT  IsoPacketNotAccesed;
    USHORT  IsoPacketHWError;

    USHORT  SmallestUrbPacketCount;
    USHORT  LargestUrbPacketCount;

    USHORT IsoCRC_Error;
    USHORT IsoOVERRUN_Error;
    USHORT IsoINTERNAL_Error;
    USHORT IsoUNKNOWN_Error; 

    ULONG  IsoBytesTransferred;

    USHORT LateMissedCount;
    USHORT HWIsoMissedCount;
    
    ULONG  Reserved7[8];
    
} HCD_ISO_STAT_COUNTERS, *PHCD_ISO_STAT_COUNTERS;



typedef struct _HCD_STAT_INFORMATION_1 {
    ULONG Reserved1;
    ULONG Reserved2;
    ULONG ResetCounters;
    LARGE_INTEGER TimeRead;
    /*
       stat registers
    */
    HCD_STAT_COUNTERS Counters;

} HCD_STAT_INFORMATION_1, *PHCD_STAT_INFORMATION_1;

typedef struct _HCD_STAT_INFORMATION_2 {
    ULONG Reserved1;
    ULONG Reserved2;
    ULONG ResetCounters;
    LARGE_INTEGER TimeRead;

    LONG LockedMemoryUsed;
    /*
       stat registers
    */
    HCD_STAT_COUNTERS Counters;
    HCD_ISO_STAT_COUNTERS IsoCounters;

} HCD_STAT_INFORMATION_2, *PHCD_STAT_INFORMATION_2;


/*
   WMI related structures
*/

/* these index in to our array of guids */
#define WMI_USB_DRIVER_INFORMATION      0
#define WMI_USB_DRIVER_NOTIFICATION     1

typedef enum _USB_NOTIFICATION_TYPE {

    /*  the following return a 
        USB_CONNECTION_NOTIFICATION structure: */
    EnumerationFailure = 0,
    InsufficentBandwidth,
    InsufficentPower,
    OverCurrent,
    ResetOvercurrent,

    /* the following return a 
       USB_BUS_NOTIFICATION structure:*/
    AcquireBusInfo,

    /* the following return a 
      USB_ACQUIRE_INFO structure: */
    AcquireHubName,
    AcquireControllerName,

    /* the following return a 
       USB_HUB_NOTIFICATION structure: */
    HubOvercurrent,
    HubPowerChange,
    
} USB_NOTIFICATION_TYPE;

typedef struct _USB_NOTIFICATION {
    /* indicates type of notification */
    USB_NOTIFICATION_TYPE NotificationType;     
    
} USB_NOTIFICATION, *PUSB_NOTIFICATION;

/* this structure is used for connection notification
   codes */
 
typedef struct _USB_CONNECTION_NOTIFICATION {
    /* indicates type of notification */
    USB_NOTIFICATION_TYPE NotificationType;     

    /* valid for all connection notifictaion codes,
       0 indicates global condition for hub or parent 
       this value will be a port number for devices
       attached to a hub, otherwise a one based 
       index if the device is a child of a composite 
       parent */
    ULONG ConnectionNumber;

    /* valid for InsufficentBandwidth,
       the amount of bandwidth the device 
       tried to allocate and was denied. */
    ULONG RequestedBandwidth;     

    /* valid for EnumerationFailure, 
       gives some indication why the device failed 
       to enumerate */
    ULONG EnumerationFailReason;

    /* valid for InsufficentPower,
       the amount of power requested to configure 
       this device. */
    ULONG PowerRequested;

    /* length of the UNICODE symbolic name (in bytes) for the HUB 
       that this device is attached to.
       not including NULL */
    ULONG HubNameLength;
    
} USB_CONNECTION_NOTIFICATION, *PUSB_CONNECTION_NOTIFICATION;

/*
   This structure is used for the bus notification code 'AcquireBusInfo'
*/

typedef struct _USB_BUS_NOTIFICATION {
    /* indicates type of notification */
    USB_NOTIFICATION_TYPE NotificationType;     /* indicates type of 
                                                   notification */
    ULONG TotalBandwidth;
    ULONG ConsumedBandwidth;

    /* length of the UNICODE symbolic name (in bytes) for the controller 
       that this device is attached to.
       not including NULL */
    ULONG ControllerNameLength; 

} USB_BUS_NOTIFICATION, *PUSB_BUS_NOTIFICATION;

/*
   used to acquire user mode filenames to open respective objects
*/

typedef struct _USB_ACQUIRE_INFO {
    /* indicates type of notification */
    USB_NOTIFICATION_TYPE NotificationType;     
    /* TotalSize of this struct */
    ULONG TotalSize;                            

    WCHAR Buffer[1];
} USB_ACQUIRE_INFO, *PUSB_ACQUIRE_INFO;


#include <poppack.h>


#endif /* __USBIOCTL_H__ */

