/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    stortrce.w

Abstract:

    These are the structures and definitions used for tracing
    in storage miniports.

Authors:

Revision History:

--*/

#include <stdarg.h>

#ifndef _NTSTORTRCE_
#define _NTSTORTRCE_

#define INLINE  __inline

//
// Determine the right PortNotification call for the miniport
//
#ifndef _PortTraceNotification

//
// Storport miniport
//
#ifdef _NTSTORPORT_
#define _PortTraceNotification StorPortNotification
#endif

//
// Scsi miniport
//
#ifdef _NTSRB_
#undef _PortTraceNotification
#define _PortTraceNotification   ScsiPortNotification
#endif

#ifndef _PortTraceNotification
#error "PortNotification not defined. Include scsi.h or storport.h"
#endif

#endif   //#ifndef _PortTraceNotification

#ifndef StorMoveMemory

#ifdef _NTSTORPORT_
#define StorMoveMemory  StorPortMoveMemory
#endif

#ifdef _NTSRB_
#undef StorMoveMemory
#define StorMoveMemory   ScsiPortMoveMemory
#endif

#ifndef StorMoveMemory
#error "StorMoveMemory not defined. Include scsi.h or storport.h"
#endif

#endif //#ifndef StorMoveMemory

typedef PVOID STORAGE_TRACE_CONTEXT;

//
// Prototype for the cleanup routine
//
typedef 
VOID
(*STOR_CLEANUP_TRACING) (
        PVOID Arg1
        );

//
// This structure is used to initializing the storage tracing library.
//
typedef struct _STORAGE_TRACE_INIT_INFO {
    //
    // The size, in bytes, of this structure.
    //
    ULONG Size;

    //
    // The number of diagnostic contexts the caller wants pre-allocated for
    // diagnostic events. By pre-allocating contexts, the caller will be
    // able to generate diagnostic events at any IRQL.
    //
    ULONG NumDiagEventRecords;

    //
    // The size, in bytes, of the user-defined data space to be allocated in
    // each pre-allocated diagnostic context.
    //
    ULONG DiagEventRecordUserDataSize;

    //
    // The number of error log records the caller wants pre-allocated.
    //
    ULONG NumErrorLogRecords;

    //
    // The trace GUID of the caller uniquely identifies the component as a
    // diagnostic event source.
    //
    GUID TraceGuid;

    //
    // Callback to cleanup tracing
    //
    STOR_CLEANUP_TRACING TraceCleanupRoutine;

    //
    // A pointer to the caller's driver object.
    //
    PVOID DriverObject;

    //
    // OUT : TraceContext to be used for error/diag support
    //
    PVOID TraceContext;

} STORAGE_TRACE_INIT_INFO, *PSTORAGE_TRACE_INIT_INFO;


//
// This structure is used to hold the user data that is attached to a
// diagnostic event.
//
typedef struct _STORAGE_DIAG_EVENT_RECORD {

    //
    // The size, in bytes of this structure. This value includes the
    // size of the information.
    //
    ULONG Size;

    //
    // Reserved.
    //
    ULONG Reserved;

    //
    // Additional information to be sent with the diagnostic event.
    //
    UCHAR Info[1];
} STORAGE_DIAG_EVENT_RECORD, *PSTORAGE_DIAG_EVENT_RECORD;


//
// This structure holds information about a diagnostic trace event.
//
typedef struct _STORAGE_TRACE_DPS_INFO {
    //
    // The event ID uniquely identifies a diagnostic event. Applications
    // can use the value to identify certain and take specific actions
    // accordingly.
    //
    GUID EventId;

    //
    // The flags field is used to control how the tracing library sends a
    // diagnostic trace event.
    //
    ULONG Flags;

    //
    // The status of the attempt to log the diagnostic event is recorded in
    // the status field.
    //
    ULONG Status;

    //
    // This field specifies the number of TRACE_CONTEXT records the caller is
    // supplying in the Contexts array.
    //
    ULONG NumContexts;

    //
    // An array of TRACE_CONTEXT structures. To be sent with the diagnostic
    // event.
    //
    PVOID Contexts;

    //
    // Specifies the size of the user data area.
    //
    ULONG UserDataSize;
    
} STORAGE_TRACE_DPS_INFO, *PSTORAGE_TRACE_DPS_INFO;

//
// This structure holds the error log from the miniport
//
typedef struct _STORAGE_ERRORLOG_PACKET {
    UCHAR         MajorFunctionCode;
    UCHAR         RetryCount;
    USHORT        DumpDataSize;
    USHORT        NumberOfStrings;
    USHORT        StringOffset;
    USHORT        EventCategory;
    ULONG         ErrorCode;
    ULONG         UniqueErrorValue;
    ULONG         FinalStatus;
    ULONG         SequenceNumber;
    ULONG         IoControlCode;
    LARGE_INTEGER DeviceOffset;
    ULONG         DumpData[1];
} STORAGE_ERRORLOG_PACKET, *PSTORAGE_ERRORLOG_PACKET;

//
// Tracing related notification types
//
typedef enum _STORAGE_TRACE_NOTIFY_TYPE {
    //
    // Initialization and cleanup
    //
    InitTracing = 1000,                 // 0x3E8 (1000)
    CleanupTracing,                     // 0x3E9 (1001)

    //
    // WPP support
    //
    TraceMessage = 2000,                // 0x7D0 (2000)
    InitGlobalLogger,                   // 0x7D1 (2001)
    WMIRegistrationControl,             // 0x7E2 (2002)
    WmiQueryTraceInfo,                  // 0x7E3 (2003)
    InitUnicodeString,                  // 0x7E4 (2004) 
    TraceDebugPrint,                    // 0x7E5 (2005)
        
    //
    // WDI support
    //
    AllocDiagEvent = 3000,              // 0xBB8 (3000)
    FreeDiagEvent,                      // 0xBB9 (3001)
    LogDiagEvent,                       // 0xBBA (3002)

    //
    // Error log support
    //
    WriteErrorLogRecord = 4000,         // 0xFA0 (4000)
    AllocErrorLog,
    FreeErrorLog

} STORAGE_TRACE_NOTIFY_TYPE, *PSTORAGE_TRACE_NOTIFY_TYPE;


//
// StorDebugPrint
//

typedef struct _STOR_DEBUGPRINT_ARGS {
    PCHAR                      Message;
    va_list                    ArgList;
} STOR_DEBUGPRINT_ARGS, *PSTOR_DEBUGPRINT_ARGS;

//
// StorInitTracing
//

typedef struct _STOR_INIT_TRACING_ARGS {
    PVOID   InitInfo;
    ULONG   Result;
} STOR_INIT_TRACING_ARGS, *PSTOR_INIT_TRACING_ARGS;


//
// StorCleanupTracing
//

typedef struct _STOR_CLEANUP_TRACING_ARGS {
    PVOID                      TraceContext;
} STOR_CLEANUP_TRACING_ARGS, *PSTOR_CLEANUP_TRACING_ARGS;

//
// WriteErrorLogEntry
//

typedef struct _STOR_WRITE_EL_RECORD_ARGS {
    PVOID                      TraceContext;
    PVOID                      ErrorLogPacket;
} STOR_WRITE_EL_RECORD_ARGS, *PSTOR_WRITE_EL_RECORD_ARGS;

//
// AllocateErrorLogEntry
//

typedef struct _STOR_ALLOC_EL_RECORD_ARGS {
    PVOID                      TraceContext;
    ULONG                      Size;
    PSTORAGE_ERRORLOG_PACKET   Result;
} STOR_ALLOC_EL_RECORD_ARGS, *PSTOR_ALLOC_EL_RECORD_ARGS;

//
// FreeErrorLogEntry
//

typedef struct _STOR_FREE_EL_RECORD_ARGS {
    PVOID                      TraceContext;
    PSTORAGE_ERRORLOG_PACKET   ErrorLogPacket;
} STOR_FREE_EL_RECORD_ARGS, *PSTOR_FREE_EL_RECORD_ARGS;


//
// TraceDriverLogEvent
//

typedef struct _STOR_LOG_DIAG_EVENT_ARGS {
    PVOID                      TraceContext;
    PVOID                      ContextEvent;
    ULONG                      result;
} STOR_LOG_DIAG_EVENT_ARGS, *PSTOR_LOG_DIAG_EVENT_ARGS;


//
// TraceDriverAllocEvent
//

typedef struct _STOR_ALLOC_DIAG_EVENT_ARGS {
    PVOID                      TraceContext;
    ULONG                      UserDataSize;
    BOOLEAN                    Allocate;
    PVOID                      result;    
} STOR_ALLOC_DIAG_EVENT_ARGS, *PSTOR_ALLOC_DIAG_EVENT_ARGS;


//
// TraceDriverFreeEvent
//

typedef struct _STOR_FREE_DIAG_EVENT_ARGS {
    PVOID                      TraceContext;
    PVOID                      EventRecord;
} STOR_FREE_DIAG_EVENT_ARGS, *PSTOR_FREE_DIAG_EVENT_ARGS;


//
// WmiTraceMessage
//

typedef struct _STOR_WMI_TRACE_MESSAGE_ARGS {
    ULONG64                    TraceHandle;
    ULONG                      MessageFlags;
    LPGUID                     MessageGuid;
    USHORT                     MessageNumber;
    va_list                    Args;
    ULONG                      result;
} STOR_WMI_TRACE_MESSAGE_ARGS, *PSTOR_WMI_TRACE_MESSAGE_ARGS;

//
// RtlInitUnicodeString
//

typedef struct _STOR_INIT_UNICODE_STRING_ARGS {
    PVOID                      DestinationString;
    PCWSTR                     SourceString;
} STOR_INIT_UNICODE_STRING_ARGS, *PSTOR_INIT_UNICODE_STRING_ARGS;


//
// IoWMIRegistrationControl
//

typedef struct _STOR_WMI_REGCONTROL_ARGS {
    PVOID                      DeviceObject;
    ULONG                      Action;
    ULONG                      result;
} STOR_WMI_REGCONTROL_ARGS, *PSTOR_WMI_REGCONTROL_ARGS;


//
// IoWMIRegistrationControl
//

typedef struct _STOR_WMI_QUERYTRACEINFO_ARGS {
    ULONG                      TraceInformationClass;
    PVOID                      TraceInformation;
    ULONG                      TraceInformationLength;
    PULONG                     RequiredLength;
    PVOID                      Buffer;
    ULONG                      result;
} STOR_WMI_QUERYTRACEINFO_ARGS, *PSTOR_WMI_QUERYTRACEINFO_ARGS;


//
// WppInitGlobalLogger
//

typedef struct _STOR_INITGLOBALLOGGER_ARGS {
    LPCGUID                     ControlGuid;
    PVOID                       Logger;
    PVOID                       Flags;
    PVOID                       Level;
} STOR_INITGLOBALLOGGER_ARGS, *PSTOR_INITGLOBALLOGGER_ARGS;

//
// memset
//
#define StorMemSet(dst, val, count) \
{ \
    ULONG _i = count; \
    while (_i) { \
        *((char *)dst+_i-1) = (char)val; \
        _i--; \
    } \
} 

//
// StorInitTracing
//

ULONG
__inline
StorInitTracing(
    IN PVOID  InitInfo
    )
{
    STOR_INIT_TRACING_ARGS args = {InitInfo, 0xC00000BB};
    _PortTraceNotification(TraceNotification, NULL, InitTracing, &args);

    return args.Result;
}

//
// StorCleanupTracing
//

VOID
__inline
StorCleanupTracing(
    IN PVOID TraceContext
    )
{
    STOR_CLEANUP_TRACING_ARGS args = {TraceContext};
    _PortTraceNotification(TraceNotification, NULL, CleanupTracing, &args);
}

//
// TraceDriverLogEvent
//

ULONG
__inline
StorTraceDiagLogEvent(
    IN PVOID                 DeviceExtension,
    IN STORAGE_TRACE_CONTEXT TraceContext,
    IN PVOID                 Event
    )
{
    STOR_LOG_DIAG_EVENT_ARGS args = {TraceContext, Event};
    _PortTraceNotification(TraceNotification, DeviceExtension, LogDiagEvent, &args);
    return args.result;
}

//
// TraceDriverAllocEvent
//

PVOID
__inline
StorTraceDiagAllocEvent(
    IN PVOID                 DeviceExtension,
    IN STORAGE_TRACE_CONTEXT TraceContext,
    IN ULONG                 DataSize,
    IN BOOLEAN               Allocate
    )
{
    STOR_ALLOC_DIAG_EVENT_ARGS args = {TraceContext, DataSize, Allocate};
    _PortTraceNotification(TraceNotification, DeviceExtension, AllocDiagEvent, &args);

    return args.result;
}

//
// TraceDriverFreeEvent
//

VOID
__inline
StorTraceDiagFreeEvent(
    IN PVOID                 DeviceExtension,
    IN STORAGE_TRACE_CONTEXT TraceContext,
    IN PVOID                 Event
    )
{
    STOR_FREE_DIAG_EVENT_ARGS args = {TraceContext, Event};
    _PortTraceNotification(TraceNotification, DeviceExtension, FreeDiagEvent, &args);
}

//
// WriteErrorLogEntry
//

VOID
__inline
StorTraceErrorWriteRecord(
    PVOID  DeviceExtension,
    PVOID  Arg1,
    PVOID  Arg2
    )
{
    STOR_WRITE_EL_RECORD_ARGS args = {Arg1, Arg2};
    _PortTraceNotification(TraceNotification, DeviceExtension, WriteErrorLogRecord, &args);
}

//
// AllocateErrorLogEntry
//

PSTORAGE_ERRORLOG_PACKET
__inline
StorTraceErrorAllocRecord(
    IN PVOID                        DeviceExtension,
    IN STORAGE_TRACE_CONTEXT        TraceContext,
    IN ULONG                        Size
    )
{
    STOR_ALLOC_EL_RECORD_ARGS args = {TraceContext, Size};
    _PortTraceNotification(TraceNotification, DeviceExtension, AllocErrorLog, &args);
    return args.Result;
}

//
// FreeErrorLogEntry
//

VOID
__inline
StorTraceErrorFreeRecord(
    IN PVOID                    DeviceExtension,
    IN STORAGE_TRACE_CONTEXT    TraceContext,
    IN PSTORAGE_ERRORLOG_PACKET ErrorLogPacket
    )
{
    STOR_FREE_EL_RECORD_ARGS args = {TraceContext, ErrorLogPacket};
    _PortTraceNotification(TraceNotification, DeviceExtension, FreeErrorLog, &args);
}

//
// WmiTraceMessage
//

ULONG
__inline
StorWmiTraceMessage(
    IN ULONG64 Arg1,
    IN ULONG   Arg2,
    IN LPGUID  Arg3,
    IN USHORT  Arg4,
    ... 
    )
{
    STOR_WMI_TRACE_MESSAGE_ARGS args = {Arg1, Arg2, Arg3, Arg4, };
    va_list ap;
    va_start(ap, Arg4);
    args.Args = ap;

    _PortTraceNotification(TraceNotification, NULL, TraceMessage, &args);
    return args.result;
}

//
// RtlInitUnicodeString
//

VOID
__inline
StorRtlInitUnicodeString(
    IN OUT PVOID Arg1,
    IN PCWSTR    Arg2
    )
{
    STOR_INIT_UNICODE_STRING_ARGS args = {Arg1, Arg2};
    _PortTraceNotification(TraceNotification, NULL, InitUnicodeString, &args);
}

//
// WppInitGlobalLogger
//

VOID
__inline
StorWppInitGlobalLogger(
    LPCGUID Arg1,
    PVOID Arg2,
    PVOID Arg3,
    PVOID Arg4
    )
{
    STOR_INITGLOBALLOGGER_ARGS args = {Arg1, Arg2, Arg3, Arg4};
    _PortTraceNotification(TraceNotification, NULL, InitGlobalLogger, &args);
}

//
// IoWMIRegistrationControl
//

ULONG
__inline
StorIoWMIRegistrationControl(
    IN PVOID Arg1,
    IN ULONG Arg2
    )
{
    STOR_WMI_REGCONTROL_ARGS args = {Arg1, Arg2};
    _PortTraceNotification(TraceNotification, NULL, WMIRegistrationControl, &args);
    return args.result;
}

//
// WmiQueryTraceInformation
//

ULONG
__inline
StorWmiQueryTraceInformation(
    IN  ULONG  Arg1,
    OUT PVOID  Arg2,
    IN  ULONG  Arg3,
    OUT PULONG Arg4,
    IN  PVOID  Arg5
    )
{
    STOR_WMI_QUERYTRACEINFO_ARGS args = {Arg1, Arg2, Arg3, Arg4, Arg5};
    _PortTraceNotification(TraceNotification, NULL, WmiQueryTraceInfo, &args);
    return args.result;
}


/*
//
// DebugPrint
//

VOID
__inline
StorDebugPrint(
    PCHAR   Arg1,
    va_list Arg2
    )
{
    STOR_DEBUGPRINT_ARGS args = {Arg1, Arg2};
    _PortTraceNotification(TraceNotification, NULL, TraceDebugPrint, &args);
}


#ifdef DO_DBGPRINT
#define WPP_DEBUG(A) StorDebugPrint A
#endif
*/

#endif

