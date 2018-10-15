/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved
 
Module Name:

    gpioctl.h  

Abstract:   Include file for Generic Port I/O Example Driver


Author:    Robert R. Howell  January 8, 1993


Revision History:

 Robert B. Nelson (Microsoft)     March 1, 1993

--*/

#if     !defined(__GPIOCTL_H__)
#define __GPIOCTL_H__

//
// Define the IOCTL codes we will use.  The IOCTL code contains a command
// identifier, plus other information about the device, the type of access
// with which the file must have been opened, and the type of buffering.
//

//
// Device type           -- in the "User Defined" range."
//

#define GPD_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.

#define IOCTL_GPD_READ_PORT_UCHAR \
    CTL_CODE( GPD_TYPE, 0x900, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_READ_PORT_USHORT \
    CTL_CODE( GPD_TYPE, 0x901, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_READ_PORT_ULONG \
    CTL_CODE( GPD_TYPE, 0x902, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOCTL_GPD_WRITE_PORT_UCHAR \
    CTL_CODE(GPD_TYPE,  0x910, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_GPD_WRITE_PORT_USHORT \
    CTL_CODE(GPD_TYPE,  0x911, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_GPD_WRITE_PORT_ULONG \
    CTL_CODE(GPD_TYPE,  0x912, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct  _GENPORT_WRITE_INPUT {
    ULONG   PortNumber;     // Port # to write to
    union   {               // Data to be output to port
        ULONG   LongData;
        USHORT  ShortData;
        UCHAR   CharData;
    };
}   GENPORT_WRITE_INPUT;

#endif

