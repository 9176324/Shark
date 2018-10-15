/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2ocontrol.h

Abstract:

    This file contains declarations and definitions for the interface between the management
    drivers (i2omgmt.sys) and the miniport. An application submits requests to the management
    driver which then turns the requests into SRB_IO_CONTROLS.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/

#ifndef _I2OCONTROL_H
#define _I2OCONTROL_H

#include <ntddscsi.h>
#include "i2omsg.h"

//
// Structs, defines for handling management app. pass-through requests.
//
// Signature in the SRB_IO_CONTROL header.
//
#define IOCTL_SIGNATURE "MSI2OMP"

//
// Control codes.
//
#define I2O_IOCTL_REGISTER          0x00000001
#define I2O_IOCTL_GET_IOP_COUNT     0x00000002
#define I2O_IOCTL_PARAMS_GET        0x00000003
#define I2O_IOCTL_PARAMS_SET        0x00000004
#define I2O_IOCTL_CONFIG_DIALOG     0x00000005
#define I2O_IOCTL_SEND_MESSAGE      0x00000006
#define I2O_IOCTL_GET_CONFIG_INFO   0x00000007
#define I2O_IOCTL_RESET_ADAPTER     0x00000008
#define I2O_IOCTL_RESCAN_ADAPTER    0x00000009
#define I2O_IOCTL_SHUTDOWN_ADAPTER  0x0000000A

#define I2O_IOCTL_STATUS_SUCCESS 0x00000000
#define I2O_IOCTL_STATUS_ERROR   0x00000001

//
// For reference.
//
//typedef struct _SRB_IO_CONTROL {
//    ULONG HeaderLength;
//    UCHAR Signature[8];
//    ULONG Timeout;
//   ULONG ControlCode;
//    ULONG ReturnCode;
//    ULONG Length;
//} SRB_IO_CONTROL, *PSRB_IO_CONTROL;
//
//
// Structures used to pass the request.
//
typedef union _I2O_REQUEST_SPECIFIC {
    ULONG RequestFlags;
    ULONG RequestPage;
} I2O_REQUEST_SPECIFIC, *PI2O_REQUEST_SPECIFIC;

typedef struct _I2O_PARAMS_HEADER {

    //
    // TID to which the request refers.
    // 
    ULONG TargetId;

    //
    // Total length of the read/write buffer.
    // This is updated by the miniport to indicate
    // how much data was actually transferred.
    //
    ULONG DataTransferLength;

    UCHAR Reserved;
    UCHAR RequestStatus;
    USHORT DetailedStatusCode;

    //
    // Offset to the data buffer
    //
    ULONG DataOffset;

    //
    // Offset to the read buffer
    // and length;
    //
    ULONG ReadOffset;
    ULONG ReadLength;

    //
    // Offset and length of the write buffer.
    //
    ULONG WriteOffset;
    ULONG WriteLength;

    I2O_REQUEST_SPECIFIC RequestSpecificInfo;
    
} I2O_PARAMS_HEADER, *PI2O_PARAMS_HEADER;    
   

//
// Structures associated with the requests.
//
typedef struct _I2O_REGISTRATION_INFO {
    
    //
    // Sizeof the structure. Used for versioning.
    // 
    ULONG Size;
    
    //
    // Adapter-unique signature.
    // For Intel-based adapters "IntelI2O"
    // 
    UCHAR Signature[8];
} I2O_REGISTRATION_INFO, *PI2O_REGISTRATION_INFO;

typedef struct _I2O_IOP_COUNT {
    ULONG IOPCount;
} I2O_IOP_COUNT, *PI2O_IOP_COUNT;

#define I2O_PAUSE_DEVICE 0x00000001
#define I2O_RESUME_DEVICE 0x00000002
#define I2O_LINK_DOWN     0x00000003
#define I2O_LINK_UP       0x00000004

typedef struct _I2O_QUEUE_MGMT_REQUEST {
    ULONG Function;
    ULONG Timeout;
    UCHAR TargetId;
} I2O_QUEUE_MGMT_REQUEST, *PI2O_QUEUE_MGMT_REQUEST;    

#endif

