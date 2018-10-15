/*++

Copyright (c) 2003  Microsoft Corporation

Module Name:

    sddef.h

Abstract:

    This is the include file that defines the basic types used
    in the SD (Secure Digital) driver stack interface. These types
    are used in conjuction with header files NTDDSD.H or SFFDISK.H.

Author:

    Neil Sandlin (neilsa) 03/01/2003

--*/

#ifndef _SDDEFH_
#define _SDDEFH_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


//
// SD device commands codes either refer to the standard command
// set (0-63), or to the "App Cmd" set, which have the same value range,
// but are preceded by the app_cmd escape (55).
//

typedef UCHAR SD_COMMAND_CODE;

typedef enum {
    SDCC_STANDARD = 0,
    SDCC_APP_CMD
} SD_COMMAND_CLASS;
//
// SDTD_READ indicates data transfer from the SD device to the host
// SDTD_WRITE indicates data transfer from the SD host to the device
//

typedef enum {
    SDTD_UNSPECIFIED = 0,
    SDTD_READ,
    SDTD_WRITE
} SD_TRANSFER_DIRECTION;


//
// Transfer type refers to the style of data transfer of the SD data
// lines. Note that a command may have a response, but still not use
// the data lines (no_data).
//

typedef enum {
    SDTT_UNSPECIFIED = 0,
    SDTT_CMD_ONLY,
    SDTT_SINGLE_BLOCK,
    SDTT_MULTI_BLOCK,
    SDTT_MULTI_BLOCK_NO_CMD12
} SD_TRANSFER_TYPE;


//
// SD transfer responses are defined in the SD spec. A given command
// will only exhibit one of these responses. But because the interface
// allows function drivers to issue undefined commands, the transfer
// response may also need to be specified.
//

typedef enum {
    SDRT_UNSPECIFIED = 0,
    SDRT_NONE,
    SDRT_1,
    SDRT_1B,
    SDRT_2,
    SDRT_3,
    SDRT_4,
    SDRT_5,
    SDRT_5B,
    SDRT_6
} SD_RESPONSE_TYPE;

//
// Response structures are mapped into the response data field in the
// request packet
//

typedef struct _SDRESP_TYPE3 {
    ULONG Reserved1:4;
    ULONG VoltageProfile:20;
    ULONG Reserved2:7;
    ULONG PowerState:1;
} SDRESP_TYPE3, *PSDRESP_TYPE3;


//
// This structure defines a specific SD device command. Function drivers
// must build this structure to pass as a parameter to the DeviceCommand request.
//
//    Cmd               - SD device code
//    CmdClass          - specifies whether the command is a standard or APP command
//    TransferDirection - direction of data on SD data lines
//    TransferType      - 3 types of commands: CmdOnly, SingleBlock or MultiBlock
//    ResponseType      - SD response type
//
// For example, a driver can issue single byte (direct) I/O reads
// to an SDIO function by first defining the following structure:
//
//      const SDCMD_DESCRIPTOR ReadIoDirectDesc =
//          {SDCMD_IO_RW_DIRECT, SDTD_STANDARD, SDTD_READ, SDTT_CMD_ONLY, SDRT_5};
//
// Then, before the call to SdbusSubmitRequest(), copy this structure
// and the argument of the command to the request packet:
//
//      sdrp->RequestFunction = SDRF_DEVICE_COMMAND;
//      sdrp->Parameters.DeviceCommand.CmdDesc = ReadIoDirectDesc;
//      sdrp->Parameters.DeviceCommand.Argument = argument;
//      status = SdBusSubmitRequest(interfaceContext, sdrp);
//
//

typedef struct _SDCMD_DESCRIPTOR {

    SD_COMMAND_CODE  Cmd;
    SD_COMMAND_CLASS CmdClass;

    SD_TRANSFER_DIRECTION TransferDirection;
    SD_TRANSFER_TYPE      TransferType;
    SD_RESPONSE_TYPE      ResponseType;

} SDCMD_DESCRIPTOR, *PSDCMD_DESCRIPTOR;


//
// Class-neutral SD device definitions
//
// Note that the SDIO arguments may be validated by the bus driver. For
// example, the bus driver will reject attempts to write into the function
// space of a different function on an SD combo card.
//

typedef struct _SD_RW_DIRECT_ARGUMENT {

    union {
        struct {
            ULONG Data:8;
            ULONG Reserved1:1;
            ULONG Address:17;
            ULONG Reserved2:1;
            ULONG ReadAfterWrite:1;
            ULONG Function:3;
            ULONG WriteToDevice:1;
        } bits;

        ULONG AsULONG;
    } u;

} SD_RW_DIRECT_ARGUMENT, *PSD_RW_DIRECT_ARGUMENT;

typedef struct _SD_RW_EXTENDED_ARGUMENT {

    union {
        struct {
            ULONG Count:9;
            ULONG Address:17;
            ULONG OpCode:1;
            ULONG BlockMode:1;
            ULONG Function:3;
            ULONG WriteToDevice:1;
        } bits;

        ULONG AsULONG;
    } u;

} SD_RW_EXTENDED_ARGUMENT, *PSD_RW_EXTENDED_ARGUMENT;


//
// Class-neutral SD codes
//
// provided here are SD command codes that are not class specific.
// Typically other codes are defined in the class driver for the
// respective device class.
//

#define SDCMD_IO_RW_DIRECT              52
#define SDCMD_IO_RW_EXTENDED            53


#ifdef __cplusplus
}
#endif
#endif

