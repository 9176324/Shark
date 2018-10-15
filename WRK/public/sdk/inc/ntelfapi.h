/*++

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntelfapi.h

Abstract:

    This file contains the prototypes for the user-level Elf APIs.

--*/

#ifndef _NTELFAPI_
#define _NTELFAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

// begin_winnt

//
// Defines for the READ flags for Eventlogging
//
#define EVENTLOG_SEQUENTIAL_READ        0x0001
#define EVENTLOG_SEEK_READ              0x0002
#define EVENTLOG_FORWARDS_READ          0x0004
#define EVENTLOG_BACKWARDS_READ         0x0008

//
// The types of events that can be logged.
//
#define EVENTLOG_SUCCESS                0x0000
#define EVENTLOG_ERROR_TYPE             0x0001
#define EVENTLOG_WARNING_TYPE           0x0002
#define EVENTLOG_INFORMATION_TYPE       0x0004
#define EVENTLOG_AUDIT_SUCCESS          0x0008
#define EVENTLOG_AUDIT_FAILURE          0x0010

//
// Defines for the WRITE flags used by Auditing for paired events
// These are not implemented in Product 1
//

#define EVENTLOG_START_PAIRED_EVENT    0x0001
#define EVENTLOG_END_PAIRED_EVENT      0x0002
#define EVENTLOG_END_ALL_PAIRED_EVENTS 0x0004
#define EVENTLOG_PAIRED_EVENT_ACTIVE   0x0008
#define EVENTLOG_PAIRED_EVENT_INACTIVE 0x0010

//
// Structure that defines the header of the Eventlog record. This is the
// fixed-sized portion before all the variable-length strings, binary
// data and pad bytes.
//
// TimeGenerated is the time it was generated at the client.
// TimeWritten is the time it was put into the log at the server end.
//

typedef struct _EVENTLOGRECORD {
    ULONG  Length;        // Length of full record
    ULONG  Reserved;      // Used by the service
    ULONG  RecordNumber;  // Absolute record number
    ULONG  TimeGenerated; // Seconds since 1-1-1970
    ULONG  TimeWritten;   // Seconds since 1-1-1970
    ULONG  EventID;
    USHORT EventType;
    USHORT NumStrings;
    USHORT EventCategory;
    USHORT ReservedFlags; // For use with paired events (auditing)
    ULONG  ClosingRecordNumber; // For use with paired events (auditing)
    ULONG  StringOffset;  // Offset from beginning of record
    ULONG  UserSidLength;
    ULONG  UserSidOffset;
    ULONG  DataLength;
    ULONG  DataOffset;    // Offset from beginning of record
    //
    // Then follow:
    //
    // WCHAR SourceName[]
    // WCHAR Computername[]
    // SID   UserSid
    // WCHAR Strings[]
    // BYTE  Data[]
    // CHAR  Pad[]
    // ULONG Length;
    //
} EVENTLOGRECORD, *PEVENTLOGRECORD;

#define MAXLOGICALLOGNAMESIZE   256

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable : 4200)
typedef struct _EVENTSFORLOGFILE{
	ULONG			ulSize;
    WCHAR   		szLogicalLogFile[MAXLOGICALLOGNAMESIZE];        //name of the logical file-security/application/system
    ULONG			ulNumRecords;
	EVENTLOGRECORD 	pEventLogRecords[];
}EVENTSFORLOGFILE, *PEVENTSFORLOGFILE;

typedef struct _PACKEDEVENTINFO{
    ULONG               ulSize;  //total size of the structure
    ULONG               ulNumEventsForLogFile; //number of EventsForLogFile structure that follow
    ULONG 				ulOffsets[];           //the offsets from the start of this structure to the EVENTSFORLOGFILE structure
}PACKEDEVENTINFO, *PPACKEDEVENTINFO;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default : 4200)
#endif
// end_winnt

#ifdef UNICODE
#define ElfClearEventLogFile   ElfClearEventLogFileW
#define ElfBackupEventLogFile  ElfBackupEventLogFileW
#define ElfOpenEventLog        ElfOpenEventLogW
#define ElfRegisterEventSource ElfRegisterEventSourceW
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogW
#define ElfReadEventLog        ElfReadEventLogW
#define ElfReportEvent         ElfReportEventW
#else
#define ElfClearEventLogFile   ElfClearEventLogFileA
#define ElfBackupEventLogFile  ElfBackupEventLogFileA
#define ElfOpenEventLog        ElfOpenEventLogA
#define ElfRegisterEventSource ElfRegisterEventSourceA
#define ElfOpenBackupEventLog  ElfOpenBackupEventLogA
#define ElfReadEventLog        ElfReadEventLogA
#define ElfReportEvent         ElfReportEventA
#endif // !UNICODE

//
// Handles are RPC context handles. Note that a Context Handle is
// always a pointer type unlike regular handles.
//

//
// Prototypes for the APIs
//

NTSTATUS
NTAPI
ElfClearEventLogFileW (
    __in HANDLE LogHandle,
    __in_opt PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfClearEventLogFileA (
    __in  HANDLE LogHandle,
    __in_opt  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileW (
    __in  HANDLE LogHandle,
    __in  PUNICODE_STRING BackupFileName
    );

NTSTATUS
NTAPI
ElfBackupEventLogFileA (
    __in  HANDLE LogHandle,
    __in  PSTRING BackupFileName
    );

NTSTATUS
NTAPI
ElfCloseEventLog (
    __in  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfDeregisterEventSource (
    __in  HANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfNumberOfRecords (
    __in  HANDLE LogHandle,
    __out PULONG NumberOfRecords
    );

NTSTATUS
NTAPI
ElfOldestRecord (
    __in  HANDLE LogHandle,
    __out PULONG OldestRecord
    );


NTSTATUS
NTAPI
ElfChangeNotify (
    __in  HANDLE LogHandle,
    __in  HANDLE Event
    );


NTSTATUS
ElfGetLogInformation (
    __in     HANDLE                LogHandle,
    __in     ULONG                 InfoLevel,
    __out_bcount(cbBufSize)    PVOID                 lpBuffer,
    __in     ULONG                 cbBufSize,
    __out    PULONG                pcbBytesNeeded
    );


NTSTATUS
NTAPI
ElfOpenEventLogW (
    __in_opt	PUNICODE_STRING UNCServerName,
    __in		PUNICODE_STRING SourceName,
    __out		PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceW (
    __in_opt	PUNICODE_STRING UNCServerName,
    __in		PUNICODE_STRING SourceName,
    __out		PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogW (
    __in_opt	PUNICODE_STRING UNCServerName,
    __in		PUNICODE_STRING FileName,
    __out		PHANDLE         LogHandle
    );

NTSTATUS
NTAPI
ElfOpenEventLogA (
    __in_opt	PSTRING UNCServerName,
    __in		PSTRING SourceName,
    __in		PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfRegisterEventSourceA (
    __in_opt  PSTRING UNCServerName,
    __in  PSTRING SourceName,
    __out PHANDLE LogHandle
    );

NTSTATUS
NTAPI
ElfOpenBackupEventLogA (
    __in_opt  PSTRING UNCServerName,
    __in  PSTRING FileName,
    __out PHANDLE LogHandle
    );


NTSTATUS
NTAPI
ElfReadEventLogW (
    __in  HANDLE LogHandle,
    __in  ULONG  ReadFlags,
    __in  ULONG  RecordNumber,
    __out_bcount(NumberOfBytesToRead) PVOID  Buffer,
    __in  ULONG  NumberOfBytesToRead,
    __out PULONG NumberOfBytesRead,
    __out PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReadEventLogA (
    __in  HANDLE LogHandle,
    __in  ULONG  ReadFlags,
    __in  ULONG  RecordNumber,
    __out_bcount(NumberOfBytesToRead) PVOID  Buffer,
    __in  ULONG  NumberOfBytesToRead,
    __out PULONG NumberOfBytesRead,
    __out PULONG MinNumberOfBytesNeeded
    );


NTSTATUS
NTAPI
ElfReportEventW (
    __in     HANDLE      LogHandle,
    __in     USHORT      EventType,
    __in_opt     USHORT      EventCategory,
    __in     ULONG       EventID,
    __in_opt     PSID        UserSid,
    __in     USHORT      NumStrings,
    __in     ULONG       DataSize,
    __in_ecount_opt(NumStrings)     PUNICODE_STRING *Strings,
    __in_bcount_opt(DataSize)     PVOID       Data,
    __in     USHORT      Flags,
    __inout_opt PULONG      RecordNumber,
    __inout_opt PULONG      TimeWritten
    );

NTSTATUS
NTAPI
ElfReportEventA (
    __in     HANDLE      LogHandle,
    __in     USHORT      EventType,
    __in_opt     USHORT      EventCategory,
    __in     ULONG       EventID,
    __in_opt     PSID        UserSid,
    __in     USHORT      NumStrings,
    __in     ULONG       DataSize,
    __in_ecount_opt(NumStrings)     PANSI_STRING *Strings,
    __in_bcount_opt(DataSize)     PVOID       Data,
    __in     USHORT      Flags,
    __inout_opt PULONG      RecordNumber,
    __inout_opt PULONG      TimeWritten
    );

NTSTATUS
NTAPI
ElfRegisterClusterSvc(
    __in_opt  PUNICODE_STRING UNCServerName,
    __out PULONG pulEventInfoSize,
    __out PVOID  *ppPackedEventInfo
);

NTSTATUS
NTAPI
ElfDeregisterClusterSvc(
    __in_opt  PUNICODE_STRING UNCServerName
    );

NTSTATUS
NTAPI
ElfWriteClusterEvents(
    __in_opt PUNICODE_STRING UNCServerName,
    __in ULONG ulEventInfoSize,
    __in_bcount(ulEventInfoSize) PVOID pPackedEventInfo
    );

NTSTATUS
NTAPI
ElfFlushEventLog (
    __in  HANDLE LogHandle
    );

NTSTATUS
ElfReportEventAndSourceW (
    __in      HANDLE          LogHandle,
    __in      ULONG EventTime,
    __in      PUNICODE_STRING pComputerNameU,
    __in      USHORT          EventType,
    __in_opt      USHORT          EventCategory,
    __in      ULONG           EventID,
    __in_opt      PSID            UserSid,
    __in      PUNICODE_STRING     UNCSourceName,
    __in      USHORT          NumStrings,
    __in      ULONG           DataSize,
    __in_ecount_opt(NumStrings)           PUNICODE_STRING *Strings,
    __in_bcount_opt(DataSize)      PVOID           Data,
    __in      USHORT          Flags,
    __inout_opt  PULONG          RecordNumber,
    __inout_opt  PULONG          TimeWritten
    );

#ifdef __cplusplus
}
#endif

#endif // _NTELFAPI_

