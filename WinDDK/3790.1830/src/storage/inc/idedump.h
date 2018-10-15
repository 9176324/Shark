/*++

Copyright (c) 2002  Microsoft Corporation

Module Name:

    idedump.h

Abstract:

    Dump data structures used by IDE/ATA when generating IDE-specific dump
    information. These data structures are shared by ATAPI and the ATAPI
    debugger extension used to debug dump failures.

    NOTE: The structure is an on-disk structure, so it must be the same
    for x86 and all other architectures.

Author:

    Matthew D Hendel (math) 15-April-2002

Revision History:

--*/

// {CA01AC1C-9D65-42c9-8DAF-CF24EF8885C8}
DEFINE_GUID(ATAPI_DUMP_ID, 0xca01ac1c, 0x9d65, 0x42c9, 0x8d, 0xaf, 0xcf, 0x24, 0xef, 0x88, 0x85, 0xc8);

#ifndef _IDEDUMP_H_
#define _IDEDUMP_H_

#include <ntdddisk.h>

typedef ULONG ATAPI_DUMP_BMSTATUS;
#define ATAPI_DUMP_BMSTATUS_NO_ERROR                    (0)
#define ATAPI_DUMP_BMSTATUS_NOT_REACH_END_OF_TRANSFER   (1 << 0)
#define ATAPI_DUMP_BMSTATUS_ERROR_TRANSFER              (1 << 1)
#define ATAPI_DUMP_BMSTATUS_INTERRUPT                   (1 << 2)
#define ATAPI_DUMP_BMSTATUS_SUCCESS(x)\
            ((x & ~ATAPI_DUMP_BMSTATUS_INTERRUPT) == 0)

//
// Dead Meat Reasons
//

enum {
    DeadMeatEnumFailed = 1,
    DeadMeatReportedMissing,
    DeadMeatTooManyTimeout,
    DeadMeatByKilledPdo,
    DeadMeatReplacedByUser
};
    

#include <pshpack8.h>
typedef struct _ATAPI_DUMP_COMMAND_LOG {
    UCHAR Cdb[16];
    IDEREGS InitialTaskFile;
    IDEREGS FinalTaskFile;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    ATAPI_DUMP_BMSTATUS BmStatus;
    UCHAR SenseData[3];
    UCHAR SrbStatus;
} ATAPI_DUMP_COMMAND_LOG, *PATAPI_DUMP_COMMAND_LOG;
#include <poppack.h>

#define ATAPI_DUMP_COMMAND_LOG_COUNT    (40)

#define ATAPI_DUMP_RECORD_VERSION   (0x01)

#include <pshpack1.h>
typedef struct _ATAPI_DUMP_PDO_INFO {
    UCHAR Version : 7;
    UCHAR WriteCacheEnable : 1;
    UCHAR TargetId;
    UCHAR DriveRegisterStatus;
    UCHAR BusyStatus;
    UCHAR FullVendorProductId[41];
    UCHAR FullProductRevisionId[8 + 1];
    UCHAR FullSerialNumber[20 * 2 + 1];
    ULONG Reason;
    ULONG TransferModeSelected;
    ULONG ConsecutiveTimeoutCount;
    ULONG DmaTransferTimeoutCount;
    ULONG FlushCacheTimeoutCount;
    ULONG IdeCommandLogIndex; 
    ATAPI_DUMP_COMMAND_LOG CommandLog[ATAPI_DUMP_COMMAND_LOG_COUNT]; 
} ATAPI_DUMP_PDO_INFO, *PATAPI_DUMP_PDO_INFO;
#include <poppack.h>

#endif // _IDEDUMP_H_

