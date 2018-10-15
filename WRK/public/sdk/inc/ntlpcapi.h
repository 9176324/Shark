/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntlpcapi.h

Abstract:

    This is the include file for the Local Procedure Call (LPC) sub-component
    of NTOS.

--*/

#ifndef _NTLPCAPI_
#define _NTLPCAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Connection Port Type Specific Access Rights.
//

#define PORT_CONNECT (0x0001)

#define PORT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | \
                         SYNCHRONIZE | 0x1)

// begin_ntifs begin_nthal

#if defined(USE_LPC6432)
#define LPC_CLIENT_ID CLIENT_ID64
#define LPC_SIZE_T ULONGLONG
#define LPC_PVOID ULONGLONG
#define LPC_HANDLE ULONGLONG
#else
#define LPC_CLIENT_ID CLIENT_ID
#define LPC_SIZE_T SIZE_T
#define LPC_PVOID PVOID
#define LPC_HANDLE HANDLE
#endif


typedef struct _PORT_MESSAGE {
    union {
        struct {
            CSHORT DataLength;
            CSHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union {
        struct {
            CSHORT Type;
            CSHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union {
        LPC_CLIENT_ID ClientId;
        double DoNotUseThisField;       // Force quadword alignment
    };
    ULONG MessageId;
    union {
        LPC_SIZE_T ClientViewSize;          // Only valid on LPC_CONNECTION_REQUEST message
        ULONG CallbackId;                   // Only valid on LPC_REQUEST message
    };
//  UCHAR Data[];
} PORT_MESSAGE, *PPORT_MESSAGE;

// end_ntifs end_nthal

typedef struct _PORT_DATA_ENTRY {
    LPC_PVOID Base;
    ULONG Size;
} PORT_DATA_ENTRY, *PPORT_DATA_ENTRY;

typedef struct _PORT_DATA_INFORMATION {
    ULONG CountDataEntries;
    PORT_DATA_ENTRY DataEntries[1];
} PORT_DATA_INFORMATION, *PPORT_DATA_INFORMATION;


//
// Valid return values for the PORT_MESSAGE Type file
//

#define LPC_REQUEST             1
#define LPC_REPLY               2
#define LPC_DATAGRAM            3
#define LPC_LOST_REPLY          4
#define LPC_PORT_CLOSED         5
#define LPC_CLIENT_DIED         6
#define LPC_EXCEPTION           7
#define LPC_DEBUG_EVENT         8
#define LPC_ERROR_EVENT         9
#define LPC_CONNECTION_REQUEST 10

// begin_ntifs

//
// The following bit may be placed in the Type field of a message
// prior calling NtRequestPort or NtRequestWaitReplyPort.  If the
// previous mode is KernelMode, the bit it left as is and passed
// to the receiver of the message.  Otherwise the bit is clear.
//

#define LPC_KERNELMODE_MESSAGE  (CSHORT)0x8000

// end_ntifs

#define LPC_NO_IMPERSONATE  (CSHORT)0x4000

#define PORT_VALID_OBJECT_ATTRIBUTES (OBJ_CASE_INSENSITIVE)

// begin_ntddk begin_wdm
#ifdef _WIN64
#define PORT_MAXIMUM_MESSAGE_LENGTH 512
#else
#define PORT_MAXIMUM_MESSAGE_LENGTH 256
#endif

#define LPC_MAX_CONNECTION_INFO_SIZE (16 * sizeof(ULONG_PTR)) 

#define PORT_TOTAL_MAXIMUM_MESSAGE_LENGTH ((PORT_MAXIMUM_MESSAGE_LENGTH + sizeof (PORT_MESSAGE) + LPC_MAX_CONNECTION_INFO_SIZE + 15) & ~15)

// end_ntddk end_wdm

#if defined(USE_LPC6432)
#undef PORT_MAXIMUM_MESSAGE_LENGTH
#define PORT_MAXIMUM_MESSAGE_LENGTH 512
#endif

typedef struct _LPC_CLIENT_DIED_MSG {
    PORT_MESSAGE PortMsg;
    LARGE_INTEGER CreateTime;
} LPC_CLIENT_DIED_MSG, *PLPC_CLIENT_DIED_MSG;

// begin_ntifs

typedef struct _PORT_VIEW {
    ULONG Length;
    LPC_HANDLE SectionHandle;
    ULONG SectionOffset;
    LPC_SIZE_T ViewSize;
    LPC_PVOID ViewBase;
    LPC_PVOID ViewRemoteBase;
} PORT_VIEW, *PPORT_VIEW;

typedef struct _REMOTE_PORT_VIEW {
    ULONG Length;
    LPC_SIZE_T ViewSize;
    LPC_PVOID ViewBase;
} REMOTE_PORT_VIEW, *PREMOTE_PORT_VIEW;

// end_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreatePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateWaitablePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );


// begin_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSecureConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __in_opt PSID RequiredServerSid,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength
    );

// end_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtListenPort(
    __in HANDLE PortHandle,
    __out PPORT_MESSAGE ConnectionRequest
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAcceptConnectPort(
    __out PHANDLE PortHandle,
    __in_opt PVOID PortContext,
    __in PPORT_MESSAGE ConnectionRequest,
    __in BOOLEAN AcceptConnection,
    __inout_opt PPORT_VIEW ServerView,
    __out_opt PREMOTE_PORT_VIEW ClientView
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompleteConnectPort(
    __in HANDLE PortHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE RequestMessage
    );

// begin_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRequestWaitReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE RequestMessage,
    __out PPORT_MESSAGE ReplyMessage
    );

// end_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE ReplyMessage
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplyWaitReplyPort(
    __in HANDLE PortHandle,
    __inout PPORT_MESSAGE ReplyMessage
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplyWaitReceivePort(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext ,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplyWaitReceivePortEx(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage,
    __in_opt PLARGE_INTEGER Timeout
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtImpersonateClientOfPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __in_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );


typedef enum _PORT_INFORMATION_CLASS {
    PortBasicInformation
#if DEVL
,   PortDumpInformation
#endif
} PORT_INFORMATION_CLASS;


NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationPort(
    __in HANDLE PortHandle,
    __in PORT_INFORMATION_CLASS PortInformationClass,
    __out_bcount(Length) PVOID PortInformation,
    __in ULONG Length,
    __out_opt PULONG ReturnLength
    );

#ifdef __cplusplus
}
#endif

#endif  // _NTLPCAPI_

