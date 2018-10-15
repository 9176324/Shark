/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    dbgk.h

Abstract:

    This header file describes public data structures and functions
    that make up the kernel mode portion of the Dbg subsystem.

--*/

#ifndef _DBGK_
#define _DBGK_

//
// Define the debug object thats used to attatch to processes that are being debugged.
//
#define DEBUG_OBJECT_DELETE_PENDING (0x1) // Debug object is delete pending.
#define DEBUG_OBJECT_KILL_ON_CLOSE  (0x2) // Kill all debugged processes on close

typedef struct _DEBUG_OBJECT {
    //
    // Event thats set when the EventList is populated.
    //
    KEVENT EventsPresent;
    //
    // Mutex to protect the structure
    //
    FAST_MUTEX Mutex;
    //
    // Queue of events waiting for debugger intervention
    //
    LIST_ENTRY EventList;
    //
    // Flags for the object
    //
    ULONG Flags;
} DEBUG_OBJECT, *PDEBUG_OBJECT;

VOID
DbgkCreateThread(
    PETHREAD Thread,
    PVOID StartAddress
    );

VOID
DbgkExitThread(
    NTSTATUS ExitStatus
    );

VOID
DbgkExitProcess(
    NTSTATUS ExitStatus
    );

VOID
DbgkMapViewOfSection(
    IN HANDLE SectionHandle,
    IN PVOID BaseAddress,
    IN ULONG SectionOffset,
    IN ULONG_PTR ViewSize
    );

VOID
DbgkUnMapViewOfSection(
    IN PVOID BaseAddress
    );

BOOLEAN
DbgkForwardException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN BOOLEAN DebugException,
    IN BOOLEAN SecondChance
    );

NTSTATUS
DbgkInitialize (
    VOID
    );

VOID
DbgkCopyProcessDebugPort (
    IN PEPROCESS TargetProcess,
    IN PEPROCESS SourceProcess
    );

NTSTATUS
DbgkOpenProcessDebugPort (
    IN PEPROCESS TargetProcess,
    IN KPROCESSOR_MODE PreviousMode,
    OUT HANDLE *pHandle
    );

NTSTATUS
DbgkClearProcessDebugObject (
    IN PEPROCESS Process,
    IN PDEBUG_OBJECT SourceDebugObject
    );


extern POBJECT_TYPE DbgkDebugObjectType;


#endif // _DBGK_
