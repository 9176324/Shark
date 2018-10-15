/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntcsrmsg.h

Abstract:

    This module defines the public message format shared by the client and
    server sides of the Client-Server Runtime (Csr) Subsystem.

--*/

#ifndef _NTCSRMSG_
#define _NTCSRMSG_

#ifdef __cplusplus
extern "C" {
#endif

#define CSR_API_PORT_NAME L"ApiPort"

//
// This structure is filled in by the client prior to connecting to the CSR
// server.  The CSR server will fill in the OUT fields if prior to accepting
// the connection.
//

typedef struct _CSR_API_CONNECTINFO {
    OUT HANDLE ObjectDirectory;
    OUT PVOID SharedSectionBase;
    OUT PVOID SharedStaticServerData;
    OUT PVOID SharedSectionHeap;
    OUT ULONG DebugFlags;
    OUT ULONG SizeOfPebData;
    OUT ULONG SizeOfTebData;
    OUT ULONG NumberOfServerDllNames;
    OUT HANDLE ServerProcessId;
} CSR_API_CONNECTINFO, *PCSR_API_CONNECTINFO;

//
// Message format for messages sent from the client to the server
//

typedef struct _CSR_CLIENTCONNECT_MSG {
    IN ULONG ServerDllIndex;
    IN OUT PVOID ConnectionInformation;
    IN OUT ULONG ConnectionInformationLength;
} CSR_CLIENTCONNECT_MSG, *PCSR_CLIENTCONNECT_MSG;

#define CSR_NORMAL_PRIORITY_CLASS   0x00000010
#define CSR_IDLE_PRIORITY_CLASS     0x00000020
#define CSR_HIGH_PRIORITY_CLASS     0x00000040
#define CSR_REALTIME_PRIORITY_CLASS 0x00000080

//
// This helps out the Wow64 thunk generater, so we can change
// RelatedCaptureBuffer from struct _CSR_CAPTURE_HEADER* to PCSR_CAPTURE_HEADER.
// Redundant typedefs are legal, so we leave the usual form in as well.
//
struct _CSR_CAPTURE_HEADER;
typedef struct _CSR_CAPTURE_HEADER CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef struct _CSR_CAPTURE_HEADER {
    ULONG Length;
    PCSR_CAPTURE_HEADER RelatedCaptureBuffer;
    ULONG CountMessagePointers;
    PCHAR FreeSpace;
    ULONG_PTR MessagePointerOffsets[1]; // Offsets within CSR_API_MSG of pointers
} CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef ULONG CSR_API_NUMBER;

typedef struct _CSR_API_MSG {
    PORT_MESSAGE h;
    union {
        CSR_API_CONNECTINFO ConnectionRequest;
        struct {
            PCSR_CAPTURE_HEADER CaptureBuffer;
            CSR_API_NUMBER ApiNumber;
            ULONG ReturnValue;
            ULONG Reserved;
            union {
                CSR_CLIENTCONNECT_MSG ClientConnect;
                ULONG_PTR ApiMessageData[39];
            } u;
        };
    };
} CSR_API_MSG, *PCSR_API_MSG;

#define WINSS_OBJECT_DIRECTORY_NAME     L"\\Windows"

#define CSRSRV_SERVERDLL_INDEX          0
#define CSRSRV_FIRST_API_NUMBER         0

#define BASESRV_SERVERDLL_INDEX         1
#define BASESRV_FIRST_API_NUMBER        0

#define CONSRV_SERVERDLL_INDEX          2
#define CONSRV_FIRST_API_NUMBER         512

#define USERSRV_SERVERDLL_INDEX         3
#define USERSRV_FIRST_API_NUMBER        1024

#define CSR_MAKE_API_NUMBER( DllIndex, ApiIndex ) \
    (CSR_API_NUMBER)(((DllIndex) << 16) | (ApiIndex))

#define CSR_APINUMBER_TO_SERVERDLLINDEX( ApiNumber ) \
    ((ULONG)((ULONG)(ApiNumber) >> 16))

#define CSR_APINUMBER_TO_APITABLEINDEX( ApiNumber ) \
    ((ULONG)((USHORT)(ApiNumber)))

#ifdef __cplusplus
}
#endif

#endif

