/*++

Copyright (c) 1993-2003  Microsoft Corporation

Module Name:

    sffdisk.h

Abstract:

    This header file defines constants and types for accessing functionality
    specific to SFF (Small Form Factor) storage devices.

Author:

    Neil Sandlin (NeilSa) 14-Jan-2003

--*/

#ifndef _SFFDISK_H_
#define _SFFDISK_H_

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200) // array[0] is not a warning for this file


//
// IOCTL codes.
//

#define IOCTL_SFFDISK_QUERY_DEVICE_PROTOCOL \
            CTL_CODE( FILE_DEVICE_DISK, 0x7a0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SFFDISK_DEVICE_COMMAND \
            CTL_CODE( FILE_DEVICE_DISK, 0x7a1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SFFDISK_DEVICE_PASSWORD \
            CTL_CODE( FILE_DEVICE_DISK, 0x7a2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


//
// Structures
//


//
// Structure used in IOCTL_SFFDISK_QUERY_DEVICE_PROTOCOL
//

typedef struct _SFFDISK_QUERY_DEVICE_PROTOCOL_DATA {
    //
    // size of this structure in bytes to be filled in by the caller
    //
    USHORT Size;
    USHORT Reserved;

    //
    // This GUID is returned by the protocol which uniquely identifies it.
    //

    GUID ProtocolGUID;

} SFFDISK_QUERY_DEVICE_PROTOCOL_DATA, *PSFFDISK_QUERY_DEVICE_PROTOCOL_DATA;


//
// Structure used in IOCTL_SFFDISK_DEVICE_COMMAND
// The layout of the buffer passed to this IOCTL is as follows:
//
//        +-----------------------------+
//        | header (this structure)     |
//        +-----------------------------+
//        | protocol arguments          |
//        +-----------------------------+
//        | device data buffer          |
//        +-----------------------------+
//
// The actual layout of the protocol arguments depends on the protocol of
// the target device. So as an example, if the target device an SD (Secure Digital)
// storage device, then the protocol arguments would consist of an SDCMD_DESCRIPTOR,
// which is defined in SDDEF.H. The SD argument for the command should be stored
// in the "Information" field of this structure. In that case, ProtocolArgumentSize
// would be sizeof(SDCMD_DESCRIPTOR).
//
// The three size fields in the structure (HeaderSize, ProtocolArgumentSize,
// DeviceDataBufferSize) each hold the length in bytes of each respective area as
// described by the diagram above. Thus, the entire length of the buffer must be
// at least as large as the sum of these three fields.
//


typedef enum {
    SFFDISK_DC_GET_VERSION = 0,
    SFFDISK_DC_LOCK_CHANNEL,
    SFFDISK_DC_UNLOCK_CHANNEL,
    SFFDISK_DC_DEVICE_COMMAND,
} SFFDISK_DCMD;


typedef struct _SFFDISK_DEVICE_COMMAND_DATA {

    //
    // size of this structure in bytes to be filled in by the caller.
    // This size does not include any data concatenated at the end.
    //
    USHORT HeaderSize;

    USHORT Reserved;

    //
    // command defines the type of operation
    //

    SFFDISK_DCMD Command;

    //
    // ProtocolArgumentSize is the length in bytes of the device command
    // arguments specific to the protocol of the device. This data is appended
    // to the structure after the field member "Data".
    //
    USHORT ProtocolArgumentSize;

    //
    // DeviceDataBufferSize defines the length of data being sent to, or received
    // from the device.
    //

    ULONG DeviceDataBufferSize;

    //
    // Information is a parameter or return value for the operation
    //

    ULONG_PTR Information;

    //
    // Beginning of data.
    //

    UCHAR Data[0];

} SFFDISK_DEVICE_COMMAND_DATA, *PSFFDISK_DEVICE_COMMAND_DATA;




//
// Structure used in IOCTL_SFFDISK_DEVICE_PASSWORD
//

typedef enum {
    SFFDISK_DP_IS_SUPPORTED = 0,
    SFFDISK_DP_SET_PASSWORD,
    SFFDISK_DP_LOCK_DEVICE,
    SFFDISK_DP_UNLOCK_DEVICE,
    SFFDISK_DP_RESET_DEVICE_ALL_DATA
} SFFDISK_DPCMD;

typedef struct _SFFDISK_DEVICE_PASSWORD_DATA {
    //
    // size of this structure in bytes to be filled in by the caller
    //
    USHORT Size;
    USHORT Reserved;

    //
    // command defines the type of operation
    //

    SFFDISK_DPCMD Command;

    //
    // Information is a parameter or return value for the operation
    //

    ULONG_PTR Information;

    //
    // Password length and data supplied depend on the operation
    //

    UCHAR PasswordLength;
    UCHAR NewPasswordLength;
    UCHAR Data[0];

} SFFDISK_DEVICE_PASSWORD_DATA, *PSFFDISK_DEVICE_PASSWORD_DATA;



#if _MSC_VER >= 1200
#pragma warning(pop)          // un-sets any local warning changes
#endif
#pragma warning(default:4200) // array[0] is not a warning for this file


#endif // _SFFDISK_H_

