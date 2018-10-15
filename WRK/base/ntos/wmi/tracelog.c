/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    tracelog.c

Abstract:

    This is the source file that implements the private routines for 
    the performance event tracing and logging facility.
    The routines here work on a single event tracing session, the logging
    thread, and buffer synchronization within a session.

--*/

// NOTE: In future, may need to align buffer size to larger of disk alignment or 1024.

#pragma warning(disable:4214)
#pragma warning(disable:4115)
#pragma warning(disable:4201)
#pragma warning(disable:4127)
#include "ntos.h"
#include "wmikmp.h"
#include <zwapi.h>
#pragma warning(default:4214)
#pragma warning(default:4115)
#pragma warning(default:4201)
#pragma warning(default:4127)

#ifndef _WMIKM_
#define _WMIKM_
#endif

#include "evntrace.h"

//
// Constants and Types used locally
//
#if DBG
ULONG WmipTraceDebugLevel=0;
// 5 All messages
// 4 Messages up to event operations
// 3 Messages up to buffer operations
// 2 Flush operations
// 1 Common operations and debugging statements
// 0 Always on - use for real error
#endif

#define ERROR_RETRY_COUNT       100

#include "tracep.h"

// Non-paged global variables
//
ULONG WmiTraceAlignment = DEFAULT_TRACE_ALIGNMENT;
ULONG WmiUsePerfClock = EVENT_TRACE_CLOCK_SYSTEMTIME;      // Global clock switch
LONG  WmipRefCount[MAXLOGGERS];
ULONG WmipGlobalSequence = 0;
PWMI_LOGGER_CONTEXT WmipLoggerContext[MAXLOGGERS];
PWMI_BUFFER_HEADER WmipContextSwapProcessorBuffers[MAXIMUM_PROCESSORS];

//
// Paged global variables
//
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG WmiWriteFailureLimit = ERROR_RETRY_COUNT;
ULONG WmipFileSystemReady  = FALSE;
WMI_TRACE_BUFFER_CALLBACK WmipGlobalBufferCallback = NULL;
PVOID WmipGlobalCallbackContext = NULL;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

//
// Function prototypes for routines used locally
//

NTSTATUS
WmipSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER *BufferPointer,
    IN PVOID BufferPointerLocation,
    IN ULONG ProcessorNumber
    );

NTSTATUS
WmipPrepareHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    );

VOID
FASTCALL
WmipPushDirtyBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
    );

VOID
FASTCALL
WmipPushFreeBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
    );

//
// Logger functions
//

NTSTATUS
WmipCreateLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG SwitchFile,
    IN ULONG Append
    );

NTSTATUS
WmipSwitchToNewFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipRequestLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFinalizeHeader(
    IN HANDLE FileHandle,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFlushBuffersWithMarker (
    IN PWMI_LOGGER_CONTEXT  LoggerContext,
    IN PSLIST_ENTRY         List,
    IN USHORT               BufferFlag
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,    WmipLogger)
#pragma alloc_text(PAGE,    WmipSendNotification)
#pragma alloc_text(PAGE,    WmipCreateLogFile)
#pragma alloc_text(PAGE,    WmipFlushActiveBuffers)
#pragma alloc_text(PAGE,    WmipGenerateFileName)
#pragma alloc_text(PAGE,    WmipPrepareHeader)
#pragma alloc_text(PAGE,    WmiBootPhase1)
#pragma alloc_text(PAGE,    WmipFinalizeHeader)
#pragma alloc_text(PAGE,    WmipSwitchToNewFile)
#pragma alloc_text(PAGE,    WmipRequestLogFile)
#pragma alloc_text(PAGE,    WmipAdjustFreeBuffers)
#pragma alloc_text(PAGEWMI, WmipFlushBuffer)
#pragma alloc_text(PAGEWMI, WmipReserveTraceBuffer)
#pragma alloc_text(PAGEWMI, WmipGetFreeBuffer)
#pragma alloc_text(PAGEWMI, WmiReserveWithPerfHeader)
#pragma alloc_text(PAGEWMI, WmiReserveWithSystemHeader)
#pragma alloc_text(PAGEWMI, WmipAllocateFreeBuffers)
#pragma alloc_text(PAGEWMI, WmipSwitchBuffer)
#pragma alloc_text(PAGEWMI, WmipReleaseTraceBuffer)
#pragma alloc_text(PAGEWMI, WmiReleaseKernelBuffer)
#pragma alloc_text(PAGEWMI, WmipResetBufferHeader)
#pragma alloc_text(PAGEWMI, WmipPushDirtyBuffer)
#pragma alloc_text(PAGEWMI, WmipPushFreeBuffer)
#pragma alloc_text(PAGEWMI, WmipPopFreeContextSwapBuffer)
#pragma alloc_text(PAGEWMI, WmipPushDirtyContextSwapBuffer)
#pragma alloc_text(PAGEWMI, WmipFlushBuffersWithMarker)
#endif

//
// Actual code starts here
//

PWMI_BUFFER_HEADER
WmipGetFreeBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
//
// This routine works at any IRQL
//
{
    PWMI_BUFFER_HEADER Buffer;
    PSLIST_ENTRY Entry;
    if (LoggerContext->SwitchingInProgress == 0) {
        //
        // Not in the middle of switching.
        //

        Entry = InterlockedPopEntrySList(&LoggerContext->FreeList);

        if (Entry != NULL) {
            Buffer = CONTAINING_RECORD (Entry,
                                        WMI_BUFFER_HEADER,
                                        SlistEntry);
    
            //
            // Reset the buffer. 
            // For circular persist mode, we want to write the buffers as
            // RunDown buffers so that post processing would work properly. 
            //

            if (LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_PERSIST) {
                WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_RUNDOWN);
            }
            else {
                WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_GENERIC);
            }

            //
            // Maintain some Wmi logger context buffer counts
            //
            InterlockedDecrement((PLONG) &LoggerContext->BuffersAvailable);
            InterlockedIncrement((PLONG) &LoggerContext->BuffersInUse);

            TraceDebug((2, "WmipGetFreeBuffer: %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n", 
                            LoggerContext->LoggerId,
                            Buffer,
                            LoggerContext->BuffersAvailable,
                            LoggerContext->BuffersInUse,
                            LoggerContext->BuffersDirty,
                            LoggerContext->NumberOfBuffers));

            return Buffer;
        } else {
            if (LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE) {
                //
                // If we are in BUFFERING Mode, put all buffers from
                // Flushlist into FreeList.
                //
            
                if (InterlockedIncrement((PLONG) &LoggerContext->SwitchingInProgress) == 1) {
                    while (Entry = InterlockedPopEntrySList(&LoggerContext->FlushList)) {
                        Buffer = CONTAINING_RECORD (Entry,
                                                    WMI_BUFFER_HEADER,
                                                    SlistEntry);
                
                        WmipPushFreeBuffer (LoggerContext, Buffer);
                    }
                }
                InterlockedDecrement((PLONG) &LoggerContext->SwitchingInProgress);
            }
            return NULL;
        }
    } else {
        return NULL;
    }
}


ULONG
WmipAllocateFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG NumberOfBuffers
    )                

/*++

Routine Description:

    This routine allocate addition buffers into the free buffer list.
    Logger can allocate more buffer to handle bursty logging behavior.
    This routine can be called by multiple places and counters must be
    manipulated using interlocked operations.

Arguments:

    LoggerContext - Logger Context
    NumberOfBuffers - Number of buffers to be allocated.

Return Value:

    The total number of buffers actually allocated.  When it is fewer than the requested number:
    If this is called when trace is turned on, we fail to turn on trace.
    If this is called by walker thread to get more buffer, it is OK.

Environment:

    Kernel mode.

--*/
{
    ULONG i;
    PWMI_BUFFER_HEADER Buffer;
    ULONG TotalBuffers;

    for (i=0; i<NumberOfBuffers; i++) {
        //
        // Multiple threads can ask for more buffers, make sure
        // we do not go over the maximum.
        //
        TotalBuffers = InterlockedIncrement(&LoggerContext->NumberOfBuffers);
        if (TotalBuffers <= LoggerContext->MaximumBuffers) {

            Buffer = (PWMI_BUFFER_HEADER)
                        ExAllocatePoolWithTag(LoggerContext->PoolType,
                                              LoggerContext->BufferSize, 
                                              TRACEPOOLTAG);
            if (Buffer != NULL) {
    
                TraceDebug((3,
                    "WmipAllocateFreeBuffers: Allocated buffer size %d type %d\n",
                    LoggerContext->BufferSize, LoggerContext->PoolType));
                InterlockedIncrement(&LoggerContext->BuffersAvailable);
                //
                // Initialize newly created buffer
                //
                RtlZeroMemory(Buffer, sizeof(WMI_BUFFER_HEADER));
                Buffer->CurrentOffset = sizeof(WMI_BUFFER_HEADER);
                KeQuerySystemTime(&Buffer->TimeStamp);
                Buffer->State.Free = 1;
    
                //
                // Insert it into the free List
                //
                InterlockedPushEntrySList(&LoggerContext->FreeList,
                                          (PSLIST_ENTRY) &Buffer->SlistEntry);
    
                InterlockedPushEntrySList(&LoggerContext->GlobalList,
                                          (PSLIST_ENTRY) &Buffer->GlobalEntry);
            } else {
                //
                // Allocation failed, decrement the NumberOfBuffers
                // we increment earlier.
                //
                InterlockedDecrement(&LoggerContext->NumberOfBuffers);
                break;
            } 
        } else {
            //
            // Maximum is reached, decrement the NumberOfBuffers
            // we increment earlier.
            //
            InterlockedDecrement(&LoggerContext->NumberOfBuffers);
            break;
        }
    }

    TraceDebug((2, "WmipAllocateFreeBuffers %3d (%3d): Free: %d, InUse: %d, Dirty: %d, Total: %d\n", 
                    NumberOfBuffers,
                    i,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers));

    return i;
}

NTSTATUS
WmipAdjustFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
/*++

Routine Description:

    This routine does buffer management.  It checks the number of free buffers and
    will allocate additonal or free some based on the situation.

Arguments:

    LoggerContext - Logger Context

Return Value:

    Status

Environment:

    Kernel mode.

--*/
{
    ULONG FreeBuffers;
    ULONG AdditionalBuffers;
    NTSTATUS Status = STATUS_SUCCESS;
    //
    //  Check if we need to allocate more buffers
    //

    FreeBuffers = ExQueryDepthSList(&LoggerContext->FreeList);
    if (FreeBuffers <  LoggerContext->MinimumBuffers) {
        AdditionalBuffers = LoggerContext->MinimumBuffers - FreeBuffers;
        if (AdditionalBuffers != WmipAllocateFreeBuffers(LoggerContext, AdditionalBuffers)) {
            Status = STATUS_NO_MEMORY;
        }
    }
    return Status;
}


//
// Event trace/record and buffer related routines
//

PSYSTEM_TRACE_HEADER
FASTCALL
WmiReserveWithSystemHeader(
    IN ULONG LoggerId,
    IN ULONG AuxSize,
    IN PETHREAD Thread,
    OUT PWMI_BUFFER_HEADER *BufferResource
    )
//
// It returns with LoggerContext locked, so caller must explicitly call
// WmipDereferenceLogger() after call WmipReleaseTraceBuffer()
//
{
    PSYSTEM_TRACE_HEADER Header;
    PWMI_LOGGER_CONTEXT LoggerContext;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount;
#endif

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiReserveWithSystemHeader: %d %d->%d\n",
                    LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext(LoggerId);

    AuxSize += sizeof(SYSTEM_TRACE_HEADER);    // add header size first
    Header = WmipReserveTraceBuffer( LoggerContext, 
                                     AuxSize, 
                                     BufferResource,
                                     &TimeStamp);
    if (Header != NULL) {

        //
        // Now copy the necessary information into the buffer
        //

        Header->SystemTime = TimeStamp;
        if (Thread == NULL) {
            Thread = PsGetCurrentThread();
        }

        Header->Marker       = SYSTEM_TRACE_MARKER;
        Header->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
        Header->ProcessId    = HandleToUlong(Thread->Cid.UniqueProcess);
        Header->KernelTime   = Thread->Tcb.KernelTime;
        Header->UserTime     = Thread->Tcb.UserTime;
        Header->Packet.Size  = (USHORT) AuxSize;
    }
    else {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);                             //Interlocked decrement
        TraceDebug((4, "WmiReserveWithSystemHeader: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
// NOTE: Caller must still put in a proper MARKER
    return Header;
}


PPERFINFO_TRACE_HEADER
FASTCALL
WmiReserveWithPerfHeader(
    IN ULONG AuxSize,
    OUT PWMI_BUFFER_HEADER *BufferResource
    )
//
// It returns with LoggerContext locked, so caller must explicitly call
// WmipDereferenceLogger() after call WmipReleaseTraceBuffer()
//
{
    PPERFINFO_TRACE_HEADER Header;
    ULONG LoggerId = WmipKernelLogger;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount;
#endif
//
// We must have this check here to see the logger is still running
// before calling ReserveTraceBuffer.
// The stopping thread may have cleaned up the logger context at this 
// point, which will cause AV.
// For all other kernel events, this check is made in callouts.c.
//
    if (WmipIsLoggerOn(LoggerId) == NULL) {
        return NULL;
    }

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiReserveWithPerfHeader: %d %d->%d\n",
                    LoggerId, RefCount-1, RefCount));

    AuxSize += FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);    // add header size first
    Header = WmipReserveTraceBuffer( WmipGetLoggerContext(LoggerId), 
                                     AuxSize, 
                                     BufferResource,
                                     &TimeStamp);
    if (Header != NULL) {
        //
        // Now copy the necessary information into the buffer
        //
        Header->SystemTime = TimeStamp;
        Header->Marker = PERFINFO_TRACE_MARKER;
        Header->Packet.Size = (USHORT) AuxSize;
    } else {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiWmiReserveWithPerfHeader: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
// NOTE: Caller must still put in a proper MARKER
    return Header;
}


PVOID
FASTCALL
WmipReserveTraceBuffer(
    IN  PWMI_LOGGER_CONTEXT LoggerContext,
    IN  ULONG RequiredSize,
    OUT PWMI_BUFFER_HEADER *BufferResource,
    OUT PLARGE_INTEGER TimeStamp
    )
/*++

Routine Description:
    This function is the main logging function that reserves spaces for any events.
    The algorithm is as follows:

    Every time a space is needed, we InterlockedExchangeAdd CurrentOffset.
    A local variable Offset is used to track the initial value when 
    InterlockedExchangeAdd is taken.  If there is enough space for this
    event (i.e., (Offset + RequiredSize) <= BufferSize), then we have successfully
    reserved the space.
    
    If there is not enough space left on this buffer, we will call WmipSwitchBuffer 
    for a new buffer.  In this case, CurrentOffset should be larger than the buffersize.
    Since other thread can still be trying to reserve space using their buffer, we
    saved the offset on SavedOffset the the logger thread knows the real offset to be
    written to disk.

    Note that, since the CurrentOffset if growing monotonically, only one thread is
    advancing the CurrentOffset from below BufferSize to beyond BufferSize.  
    It is this thread's responsibility to set the SavedOffset properly.

Arguments:

    LoggerContext - Logger context from current logging session.

    RequiredSize  - The space needed for logging the data.

    Buffer        - Pointer to a buffer header

    TimeStamp     - TimeStamp of the event

Return Value:

    The status of running the buffer manager

Environment:

    Kernel mode.  This routine should work at any IRQL.

--*/
{
    PVOID       ReservedSpace;
    PWMI_BUFFER_HEADER Buffer = NULL;
    ULONG       Offset;

    //
    // Mark it volatile to work around compiler bug.
    //
    volatile ULONG Processor;
    NTSTATUS    Status;

    if (!WmipIsValidLogger(LoggerContext)) {
        return NULL;
    }
    if (!LoggerContext->CollectionOn) {
        return NULL;
    }

    *BufferResource = NULL;

    RequiredSize = (ULONG) ALIGN_TO_POWER2(RequiredSize, WmiTraceAlignment);

    if (RequiredSize > LoggerContext->BufferSize - sizeof (WMI_BUFFER_HEADER)) {
        goto LostEvent;
    }

    //
    // Get processor number again here due to possible context switch
    //
    Processor = KeGetCurrentProcessorNumber();

    //
    // Get the processor specific buffer pool
    //
    Buffer = LoggerContext->ProcessorBuffers[Processor];

    if (Buffer == NULL) {
        //
        // Nothing in per process list, ask to get a new buffer
        //
        Status = WmipSwitchBuffer(LoggerContext, 
                                  &Buffer,
                                  &LoggerContext->ProcessorBuffers[Processor],
                                  Processor);

        if (!NT_SUCCESS(Status)) {
            //
            // Nothing available
            //
            goto LostEvent;
        }

        ASSERT(Buffer != NULL);
    }

TryFindSpace:

    //
    // Increment refcount to buffer first to prevent it from going away
    //
    InterlockedIncrement(&Buffer->ReferenceCount);
    
    //
    // Check if there is enough space in this buffer.
    //
    Offset = (ULONG) InterlockedExchangeAdd(
                     (PLONG) &Buffer->CurrentOffset, RequiredSize);

    //
    // If many threads concurrently hit this line, the Offset+RequiredSize
    // might arithmetically overflow. Therefore we need to add an extra 
    // check to make sure the Offset is in range.
    //

    if ( (Offset+RequiredSize <= LoggerContext->BufferSize) && 
         (Offset < LoggerContext->BufferSize) ) {
    
        //
        // Successfully reserved the space
        // Get the timestamp of the event
        //
        if (TimeStamp) {
            TimeStamp->QuadPart = (*LoggerContext->GetCpuClock)();
        }

        //
        // Set up the space pointer
        //
        ReservedSpace = (PVOID) (Offset +  (char*)Buffer);
    
        if (LoggerContext->SequencePtr) {
            *((PULONG) ReservedSpace) =
                (ULONG)InterlockedIncrement(LoggerContext->SequencePtr);
        }
        goto FoundSpace;
    } else {
        //
        // There is not enough space left to log this event,
        // Ask for buffer switch.  The WmipSwitchBuffer()
        // will push this current buffer into Dirty list.
        //
        // Before asking for buffer switch, 
        // check if I am the one that got overboard.  
        // If yes, put the correct offset back.
        //
        if (Offset <= LoggerContext->BufferSize) {
            Buffer->SavedOffset = Offset;
        }

        //
        // Also, dereference the buffer, so it can be freed.
        //
        InterlockedDecrement((PLONG) &Buffer->ReferenceCount);

        //
        // Nothing in per process list, ask to get a new buffer
        //
        Status = WmipSwitchBuffer(LoggerContext, 
                                  &Buffer,
                                  &LoggerContext->ProcessorBuffers[Processor],
                                  Processor);

        if (!NT_SUCCESS(Status)) {
            //
            // Nothing available
            //
            goto LostEvent;
        }

        ASSERT (Buffer != NULL);
        goto TryFindSpace;
    }

LostEvent:
    //
    // Will get here it we are throwing away the event
    //
    ASSERT(Buffer == NULL);
    LoggerContext->EventsLost++;    // best attempt to be accurate
    ReservedSpace = NULL;
    if (LoggerContext->SequencePtr) {
        InterlockedIncrement(LoggerContext->SequencePtr);
    }

FoundSpace:
    //
    // notify the logger after critical section
    //
    *BufferResource = Buffer;

    return ReservedSpace;
}

NTSTATUS
WmipSwitchToNewFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
/*++

Routine Description:

    This routine is called to get a LogFileHandle for GlobalLogger or
    when a fileswitch is needed for NEWFILE mode. It will create the file
    and add logfileheader to it. It closes the oldfile by properly
    finalizing its header.  

Arguments:

    LoggerContext - Logger Context

Return Value:

    Status

Environment:

    Kernel mode.

--*/
{
    IO_STATUS_BLOCK IoStatus;
    HANDLE OldHandle, NewHandle;
    UNICODE_STRING NewFileName, OldFileName;
    ULONG BufferSize = LoggerContext->BufferSize;
    PWMI_BUFFER_HEADER NewHeaderBuffer;
    NTSTATUS Status=STATUS_SUCCESS;

    PAGED_CODE();

    NewFileName.Buffer = NULL;

    if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {

        Status = WmipGenerateFileName(
                                      &LoggerContext->LogFilePattern,
                                      (PLONG) &LoggerContext->FileCounter,
                                      &NewFileName
                                     );

        if (!NT_SUCCESS(Status)) {
            TraceDebug((1, "WmipSwitchToNewFile: Error %x generating filename\n", Status));
            return Status;
        }

    }
    else {
        //
        // GlobalLogger path. It is executed only once to set up 
        // the logfile. 
        //
        if (LoggerContext->LogFileHandle != NULL) {
            LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;
            return STATUS_SUCCESS;
        }
        if (LoggerContext->LogFileName.Buffer == NULL) {
            TraceDebug((1, "WmipSwitchToNewFile: No LogFileName\n"));
            return STATUS_INVALID_PARAMETER;
        }
        if (! RtlCreateUnicodeString( &NewFileName, 
                                      LoggerContext->LogFileName.Buffer) ) {
            TraceDebug((1, "WmipSwitchToNewFile: No Memory for NewFileName\n"));
            return STATUS_NO_MEMORY;
        }
    }

    //
    // We have a NewFileName. Create the File 
    //
    Status = WmipDelayCreate(&NewHandle, &NewFileName, FALSE);

    if (NT_SUCCESS(Status)) {
        NewHeaderBuffer = (PWMI_BUFFER_HEADER)
                           ExAllocatePoolWithTag(LoggerContext->PoolType,
                                              LoggerContext->BufferSize,
                                              TRACEPOOLTAG);
        if (NewHeaderBuffer != NULL) {
        //
        // Now we have all the resources we need for the new file. 
        // Let's close out the old file, if necessary and switch
        //
            OldFileName = LoggerContext->LogFileName;
            OldHandle = LoggerContext->LogFileHandle;
            if (OldHandle) {
                WmipFinalizeHeader(OldHandle, LoggerContext);
                ZwClose(OldHandle);
            }

            // NOTE: Assumes LogFileName cannot be changed
            //  for NEWFILE mode!!!
            if (OldFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&OldFileName);
            }

            LoggerContext->BuffersWritten = 1;
            LoggerContext->LogFileHandle = NewHandle;
            LoggerContext->LogFileName = NewFileName;

            NewFileName.Buffer = NULL;

            RtlZeroMemory( NewHeaderBuffer, LoggerContext->BufferSize );
            WmipResetBufferHeader(LoggerContext, 
                                  NewHeaderBuffer, 
                                  WMI_BUFFER_TYPE_RUNDOWN);

            WmipAddLogHeader(LoggerContext, NewHeaderBuffer);

            LoggerContext->LastFlushedBuffer = 1;
            LoggerContext->ByteOffset.QuadPart = BufferSize;
            LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;
            LoggerContext->LoggerMode &= ~EVENT_TRACE_DELAY_OPEN_FILE_MODE;
            LoggerContext->LoggerMode &= ~EVENT_TRACE_ADD_HEADER_MODE;

            Status = WmipPrepareHeader(LoggerContext,  
                                       NewHeaderBuffer, 
                                       WMI_BUFFER_TYPE_RUNDOWN);
            if (NT_SUCCESS(Status)) {
                Status = ZwWriteFile(
                                     NewHandle,
                                     NULL, NULL, NULL,
                                     &IoStatus,
                                     NewHeaderBuffer,
                                     BufferSize,
                                     NULL, NULL);
            }
            if (!NT_SUCCESS(Status) ) {
                TraceDebug((1, "WmipSwitchToNewFile: Write Failed\n", Status));
            }

            WmipSendNotification(LoggerContext, 
                                 STATUS_MEDIA_CHANGED, 
                                 STATUS_SEVERITY_INFORMATIONAL);

            ExFreePool(NewHeaderBuffer);
        }
    }
    
    if (NewFileName.Buffer != NULL) {
        ExFreePool(NewFileName.Buffer);
    }

    return Status;
}


NTSTATUS
WmipRequestLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
/*++

Routine Description:

    This routine switches logfiles for an active logger. This routine must   
    be called only by the Logger Thread. It will close the previous logfile,
    if any, properly by terminating it with FLUSH_MARKER and Finalizing the
    LogHeader. 

    Two different cases to consider here are: 
        1. The newfile was created in user mode with headers and rundown data
        2. The newfile is created in kernel (need to add LogFileHeader)

    The callers of this function are
        1. UpdateLogger: Sets the REQUEST_FLAG_NEW_FILE after presenting
           a user mode created logfile in NewLogFile.
        2. NT Kernel Logger session: Switches from DELAY_OPEN mode with 
           a user mode created logfile 
        3. FILE_MODE_NEWFILE: When current logfile reaches the FileLimit
           FlushBuffer requests a newfile. 
        4. GlobalLogger: Started in DELAY_OPEN && ADD_HEADER mode needs
           to create the logfile when the FileSystem is ready. 

    In all cases, when the switch is made the old logfile needs to be 
    properly closed after Finalizing its header. 



Arguments:

    LoggerContext - Logger Context

Return Value:

    Status

Environment:

    Kernel mode.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    if (!WmipFileSystemReady) {
    //
    // FileSystem is not ready yet, so return for now.
    //
        return Status;
    }

    //
    // In order for us to act on this request we need something to create
    // a file with, such as FileName, Pattern etc., 
    //

    if ((LoggerContext->LogFileName.Buffer == NULL ) &&
        (LoggerContext->LogFilePattern.Buffer == NULL)  &&
        (LoggerContext->NewLogFileName.Buffer == NULL) ) {

        return Status;
    }
        
    //
    // With the REQUEST_FLAG_NEW_FILE set, flush all active buffers. 
    //

    if (LoggerContext->LogFileHandle != NULL ) {
        Status = WmipFlushActiveBuffers(LoggerContext, TRUE);
    }

    if (NT_SUCCESS(Status)) {
        if ( (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) ||
             ( (LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) &&
               (LoggerContext->LoggerMode & EVENT_TRACE_ADD_HEADER_MODE))) {  

            Status = WmipSwitchToNewFile(LoggerContext);
        }
        else {
            //
            // UpdateTrace case
            //
            TraceDebug((3, "WmipLogger: New File\n"));
            Status = WmipCreateLogFile(LoggerContext,
                                       TRUE,
                                       EVENT_TRACE_FILE_MODE_APPEND);
            if (NT_SUCCESS(Status)) {
                LoggerContext->LoggerMode &= ~EVENT_TRACE_DELAY_OPEN_FILE_MODE;
            }
            //
            // This is to release the Update thread from the wait
            //
            KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);
        }
    }

    if (! NT_SUCCESS(Status)) {
        LoggerContext->LoggerStatus = Status;
    }
    return Status;
}


//
// Actual Logger code starts here
//


VOID
WmipLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )

/*++

Routine Description:
    This function is the logger itself. It is started as a system thread.
    It will not return until someone has stopped data collection or it
    is not successful is flushing out a buffer (e.g. disk is full).

Arguments:

    None.

Return Value:

    The status of running the buffer manager

--*/

{
    NTSTATUS Status, InitialStatus;
    ULONG ErrorCount;
    ULONG FlushTimeOut;
    ULONG64 LastFlushTime=0;

    PAGED_CODE();

    //
    // Set the logger status to PENDING before touching logger context.
    // If the starting threads times out and sees STATUS_PENDING, it will wait
    // until we notify.
    //
    InitialStatus = InterlockedExchange(&LoggerContext->LoggerStatus,
                                        STATUS_PENDING);

    if (STATUS_CANCELLED == InitialStatus) {
        // 
        // The logger thread was too late to notify the starting thread.
        //
        // Clean up the LoggerContext, and go away here. 
        //
        WmipFreeLoggerContext(LoggerContext);
        PsTerminateSystemThread(InitialStatus);
        return;
    }

    LoggerContext->LoggerThread = PsGetCurrentThread();

    if ((LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)
        || (LoggerContext->LogFileName.Length == 0)) {

        // If EVENT_TRACE_DELAY_OPEN_FILE_MODE is specified, WMI does not
        // need to create logfile now.
        //
        // If there is no LogFileName specified, WMI does not need to create
        // logfile either. WmipStartLogger() already checks all possible
        // combination of LoggerMode and LogFileName, so we don't need to
        // perform the same check again.
        //
        Status = STATUS_SUCCESS;
    } else {
        Status = WmipCreateLogFile(LoggerContext, 
                                   FALSE,
                                   LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_APPEND);
    }


    //
    // Set the logger status to let the starting thread know what happened.
    //
    InitialStatus = InterlockedExchange(&LoggerContext->LoggerStatus,
                                        Status);
    ASSERT(InitialStatus != STATUS_CANCELLED);

    if (!NT_SUCCESS(Status)){
        //
        // Logger thread failed to open the file. We will notify the starting 
        // thread, but we will clean up the logger context here.
        //
        if (LoggerContext->LogFileHandle != NULL) {
            Status = ZwClose(LoggerContext->LogFileHandle);
            LoggerContext->LogFileHandle = NULL;
        }
        KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
        WmipFreeLoggerContext(LoggerContext);
        PsTerminateSystemThread(Status);
        return;
    }
    else { // All succeeded.
        //
        // It is safe to go.
        // This is the only place where CollectionOn will be turn on!!!
        //
        LoggerContext->CollectionOn = TRUE;
        KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
    }

    ErrorCount = 0;
    // by now, the caller has been notified that the logger is running

    //
    // Loop and wait for buffers to be filled until someone turns off CollectionOn
    //
    KeSetBasePriorityThread(KeGetCurrentThread(), LOW_REALTIME_PRIORITY-1);

    while (LoggerContext->CollectionOn) {

        if (LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE) {
            //
            // Wait forever until signaled by when logging is terminated.
            //
            Status = KeWaitForSingleObject(
                        &LoggerContext->LoggerSemaphore,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL);
            LoggerContext->LoggerStatus = STATUS_SUCCESS;
        } else {
            ULONG FlushAll = 0;
            ULONG FlushFlag;

            FlushTimeOut = LoggerContext->FlushTimer;
            //
            // Wake up every second to see if there are any buffers in
            // flush list.
            //
            Status = KeWaitForSingleObject(
                        &LoggerContext->LoggerSemaphore,
                        Executive,
                        KernelMode,
                        FALSE,
                        &WmiOneSecond);
    
            //
            //  Check if number of buffers need to be adjusted.
            //
            WmipAdjustFreeBuffers(LoggerContext);

            LoggerContext->LoggerStatus = STATUS_SUCCESS;

            if ((LoggerContext->RequestFlag & REQUEST_FLAG_NEW_FILE)  ||
                ((LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)
                && (LoggerContext->LoggerMode & EVENT_TRACE_ADD_HEADER_MODE)) ) {
                Status = WmipRequestLogFile( LoggerContext);
            }

            //
            // Check to see if we need to FlushAll 
            //
            if (FlushTimeOut) {
                ULONG64 Now;
                KeQuerySystemTime((PLARGE_INTEGER) &Now);
                if ( ((Now - LastFlushTime) / 10000000) >= FlushTimeOut) { 
                    FlushAll = 1;
                    LastFlushTime = Now;
                }
                else {
                    FlushAll = 0;
                }
            }

            FlushFlag = (LoggerContext->RequestFlag & REQUEST_FLAG_FLUSH_BUFFERS);
            if (  FlushFlag ) 
                FlushAll = TRUE;

                Status = WmipFlushActiveBuffers(LoggerContext, FlushAll);
                //
                // Should check the status, and if failed to write a log file
                // header, should clean up.  As the log file is bad anyway.
                //
                if (  FlushFlag )  {
                    LoggerContext->RequestFlag &= ~REQUEST_FLAG_FLUSH_BUFFERS;
                    //
                    // If this was a flush for persistent events, this request 
                    // flag must be reset here.
                    //
                    if (LoggerContext->RequestFlag & 
                                       REQUEST_FLAG_CIRCULAR_TRANSITION) {
                        if (LoggerContext->LogFileHandle != NULL) {
                            WmipFinalizeHeader(LoggerContext->LogFileHandle, 
                                               LoggerContext);
                        }

                        LoggerContext->RequestFlag &= ~REQUEST_FLAG_CIRCULAR_TRANSITION;
                    }

                    LoggerContext->LoggerStatus = Status;
                    KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);

                }
                if (!NT_SUCCESS(Status)) {
                    LoggerContext->LoggerStatus = Status;
                    WmipStopLoggerInstance(LoggerContext);
                }
        }
    } // while loop

    if (Status == STATUS_TIMEOUT) {
        Status = STATUS_SUCCESS;
    }
//
// if a normal collection end, flush out all the buffers before stopping
//

    TraceDebug((2, "WmipLogger: Flush all buffers before stopping...\n"));
//
// First, move the per processor buffer out to FlushList
//
    // This is to force buffers to be written
    // in FlushBuffer without snapping back to this routine to create a file. 
    LoggerContext->RequestFlag |= REQUEST_FLAG_NEW_FILE;

    while ((LoggerContext->NumberOfBuffers > 0) &&
           (LoggerContext->NumberOfBuffers > LoggerContext->BuffersAvailable)) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->LoggerSemaphore,
                    Executive,
                    KernelMode,
                    FALSE,
                    &WmiOneSecond);
        WmipFlushActiveBuffers(LoggerContext, 1);
        TraceDebug((2, "WmipLogger: Stop %d %d %d %d %d\n",
                        LoggerContext->LoggerId,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));
    }

    //
    // Note that LoggerContext->LogFileObject needs to remain set
    //    for QueryLogger to work after close
    //
    if (LoggerContext->LogFileHandle != NULL) {
        ZwClose(LoggerContext->LogFileHandle);
        TraceDebug((1, "WmipLogger: Close logfile with status=%X\n", Status));
    }
    LoggerContext->LogFileHandle = NULL;
    KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);
    KeSetEvent(&LoggerContext->LoggerEvent, 0, FALSE);
#if DBG
    if (!NT_SUCCESS(Status)) {
        TraceDebug((1, "WmipLogger: Aborting %d %X\n",
                        LoggerContext->LoggerId, LoggerContext->LoggerStatus));
    }
#endif

    WmipFreeLoggerContext(LoggerContext);
    PsTerminateSystemThread(Status);
}

NTSTATUS
WmipSendNotification(
    PWMI_LOGGER_CONTEXT LoggerContext,
    NTSTATUS            Status,
    ULONG               Flag
    )
{
    WMI_TRACE_EVENT WmiEvent;

    RtlZeroMemory(& WmiEvent, sizeof(WmiEvent));
    WmiEvent.Status = Status;
    KeQuerySystemTime(& WmiEvent.Wnode.TimeStamp);

    WmiEvent.Wnode.BufferSize = sizeof(WmiEvent);
    WmiEvent.Wnode.Guid       = TraceErrorGuid;
    WmiSetLoggerId(
          LoggerContext->LoggerId,
          (PTRACE_ENABLE_CONTEXT) & WmiEvent.Wnode.HistoricalContext);

    WmiEvent.Wnode.ClientContext = 0XFFFFFFFF;
    WmiEvent.TraceErrorFlag = Flag;

    WmipProcessEvent(&WmiEvent.Wnode,
                     FALSE,
                     FALSE);
    

    return STATUS_SUCCESS;
}

//
// convenience routine to flush the current buffer by the logger above
//

NTSTATUS
WmipFlushBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    )
/*++

Routine Description:
    This function is responsible for flushing a filled buffer out to
    disk, or to a real time consumer.

Arguments:

    LoggerContext -      Context of the logger

    Buffer - 

    BufferFlag -

Return Value:

    The status of flushing the buffer

--*/
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status;
    ULONG BufferSize;
    ULONG BufferPersistenceData = LoggerContext->RequestFlag
                                & (  REQUEST_FLAG_CIRCULAR_PERSIST
                                   | REQUEST_FLAG_CIRCULAR_TRANSITION);

    ASSERT(LoggerContext != NULL);
    ASSERT(Buffer != NULL);

    if (LoggerContext == NULL || Buffer == NULL) {
        return STATUS_SEVERITY_ERROR;
    }

    //
    // Grab the buffer to be flushed
    //
    BufferSize = LoggerContext->BufferSize;

    //
    // Put end of record marker in buffer if available space
    //

    TraceDebug((2, "WmipFlushBuffer: %p, Flushed %X %8x %8x %5d\n",
                Buffer,
                Buffer->ClientContext, Buffer->SavedOffset,
                Buffer->CurrentOffset, LoggerContext->BuffersWritten));

    Status = WmipPrepareHeader(LoggerContext, Buffer, BufferFlag);

    if (Status == STATUS_SUCCESS) {

        //
        // Buffering mode is mutually exclusive with REAL_TIME_MODE
        //
        if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
            if (LoggerContext->LoggerMode & EVENT_TRACE_REAL_TIME_MODE) {

                if (LoggerContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
                    Buffer->Wnode.Flags |= WNODE_FLAG_USE_TIMESTAMP;
                }

                // need to see if we can send anymore
                // check for queue length
                if (! NT_SUCCESS(WmipProcessEvent((PWNODE_HEADER)Buffer,
                                                  FALSE,
                                                  FALSE))) {
                    LoggerContext->RealTimeBuffersLost++;
                }
            }
        }

        if (LoggerContext->LogFileHandle != NULL) {

            if (LoggerContext->MaximumFileSize > 0) { // if quota given
                ULONG64 FileSize = (ULONG64)(LoggerContext->LastFlushedBuffer) * BufferSize;
                ULONG64 FileLimit = (ULONG64)(LoggerContext->MaximumFileSize) * BYTES_PER_MB;
                if (LoggerContext->LoggerMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
                    FileLimit = LoggerContext->MaximumFileSize * 1024;
                }

                if ( FileSize >= FileLimit ) {

                    ULONG LoggerMode = LoggerContext->LoggerMode & 0X000000FF;
                    //
                    // Files from user mode always have the APPEND flag. 
                    // We mask it out here to simplify the testing below.
                    //
                    LoggerMode &= ~EVENT_TRACE_FILE_MODE_APPEND;
                    //
                    // PREALLOCATE flag has to go, too. 
                    //
                    LoggerMode &= ~EVENT_TRACE_FILE_MODE_PREALLOCATE;

                    if (LoggerMode == EVENT_TRACE_FILE_MODE_SEQUENTIAL) {
                        // do not write to logfile anymore

                        Status = STATUS_LOG_FILE_FULL; // control needs to stop logging
                        // need to fire up a Wmi Event to control console
                        WmipSendNotification(LoggerContext,
                            Status, STATUS_SEVERITY_ERROR);
                    }
                    else if (LoggerMode == EVENT_TRACE_FILE_MODE_CIRCULAR ||
                             LoggerMode == EVENT_TRACE_FILE_MODE_CIRCULAR_PERSIST) {
                        if (BufferPersistenceData > 0) {
                            // treat circular logfile as sequential logfile if
                            // logger still processes Persistence events (events
                            // that cannot be overwritten in circular manner).
                            //
                            Status = STATUS_LOG_FILE_FULL;
                            WmipSendNotification(LoggerContext,
                                Status, STATUS_SEVERITY_ERROR);
                        }
                        else {
                            // reposition file

                            LoggerContext->ByteOffset
                                    = LoggerContext->FirstBufferOffset;
                            LoggerContext->LastFlushedBuffer = (ULONG)
                                      (LoggerContext->FirstBufferOffset.QuadPart
                                    / LoggerContext->BufferSize);
                        }
                    }
                    else if (LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {

                        //
                        // We will set the RequestFlag to initiate a file switch. 
                        // If that flag is already set, then we continue to flush
                        // past the FileLimit.  
                        //
                        // There should be no race condition with an UpdateTrace
                        // setting the RequestFlag since we do not allow change of
                        // filename for NEWFILE mode. 
                        //

                        if ( (LoggerContext->RequestFlag & REQUEST_FLAG_NEW_FILE) != 
                             REQUEST_FLAG_NEW_FILE) 
                        {
                            LoggerContext->RequestFlag |= REQUEST_FLAG_NEW_FILE;
                        }
                    }
                }
            }

            if (NT_SUCCESS(Status)) {
                Status = ZwWriteFile(
                            LoggerContext->LogFileHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            Buffer,
                            BufferSize,
                            &LoggerContext->ByteOffset,
                            NULL);

                if (NT_SUCCESS(Status)) {
                    LoggerContext->ByteOffset.QuadPart += BufferSize;
                    
                    //
                    // update FirstBufferOffset so that persistence event will
                    // not be overwritten in circular logfile. We need to check that
                    // buffer type is RUNDOWN because if persistent --> non persistent
                    // transition happens at high IRQL we will not be able to flush logger
                    // and might end up with non RUNDOWN buffers in the rundown stream
                    //
                    if ((BufferPersistenceData > 0)  && 
                        (Buffer->BufferType == WMI_BUFFER_TYPE_RUNDOWN)) {

                        LoggerContext->FirstBufferOffset.QuadPart += BufferSize;
                    }
                }
                else if (Status == STATUS_LOG_FILE_FULL ||
                         Status == STATUS_DISK_FULL) {
                    // need to fire up a Wmi Event to control console
                    WmipSendNotification(LoggerContext,
                        STATUS_LOG_FILE_FULL, STATUS_SEVERITY_ERROR);
                }
                else {
                    TraceDebug((2, "WmipFlushBuffer: Unknown WriteFile Failure with status=%X\n", Status));
                }
            }
        }

        // Now do callbacks. This happens whether a file exists or not.
        if (WmipGlobalBufferCallback) {
            (WmipGlobalBufferCallback) (Buffer, WmipGlobalCallbackContext);
        }
        if (LoggerContext->BufferCallback) {
            (LoggerContext->BufferCallback) (Buffer, LoggerContext->CallbackContext);
        }
    }

    if (NT_SUCCESS(Status)) {
        LoggerContext->BuffersWritten++;
        LoggerContext->LastFlushedBuffer++;
    }
    else {
#if DBG
        if (Status == STATUS_NO_DATA_DETECTED) {
            TraceDebug((2, "WmipFlushBuffer: Empty buffer detected\n"));
        }
        else if (Status == STATUS_SEVERITY_WARNING) {
            TraceDebug((2, "WmipFlushBuffer: Buffer could be corrupted\n"));
        }
        else {
            TraceDebug((2,
                "WmipFlushBuffer: Unable to write buffer: status=%X\n",
                Status));
        }
#endif
        if ((Status != STATUS_NO_DATA_DETECTED) &&
            (Status != STATUS_SEVERITY_WARNING))
            LoggerContext->LogBuffersLost++;
    }

    return Status;
}

NTSTATUS
WmipCreateLogFile(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG SwitchFile,
    IN ULONG Append
    )
{
    NTSTATUS Status;
    HANDLE newHandle = NULL;
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileSize = {0};
    LARGE_INTEGER ByteOffset;
    BOOLEAN FileSwitched = FALSE;
    UNICODE_STRING OldLogFileName;

    PWCHAR            strLogFileName = NULL;
    PUCHAR            pFirstBuffer = NULL;

    PAGED_CODE();

    RtlZeroMemory(&OldLogFileName, sizeof(UNICODE_STRING));
    LoggerContext->RequestFlag &= ~REQUEST_FLAG_NEW_FILE;

    pFirstBuffer = (PUCHAR) ExAllocatePoolWithTag(
            PagedPool, LoggerContext->BufferSize, TRACEPOOLTAG);
    if(pFirstBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(pFirstBuffer, LoggerContext->BufferSize);

    if (SwitchFile) {
        Status = WmipCreateNtFileName(
                        LoggerContext->NewLogFileName.Buffer,
                        & strLogFileName);
    }
    else {
        Status = WmipCreateNtFileName(
                        LoggerContext->LogFileName.Buffer,
                        & strLogFileName);
    }
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    if (LoggerContext->ClientSecurityContext.ClientToken != NULL) {
        Status = SeImpersonateClientEx(
                        &LoggerContext->ClientSecurityContext, NULL);
    }
    if (NT_SUCCESS(Status)) {
        // first open logfile using user security context
        //
        Status = WmipCreateDirectoryFile(strLogFileName, FALSE, & newHandle, Append);
        PsRevertToSelf();
    }
    if (!NT_SUCCESS(Status)) {
        // if using user security context fails to open logfile,
        // then try open logfile again using local system security context
        //
        Status = WmipCreateDirectoryFile(strLogFileName, FALSE, & newHandle, Append);
    }

    if (NT_SUCCESS(Status)) {
        HANDLE tempHandle = LoggerContext->LogFileHandle;
        PWMI_BUFFER_HEADER    BufferChecksum;
        PTRACE_LOGFILE_HEADER LogfileHeaderChecksum;
        ULONG BuffersWritten = 0;

        BufferChecksum = (PWMI_BUFFER_HEADER) LoggerContext->LoggerHeader;
        LogfileHeaderChecksum = (PTRACE_LOGFILE_HEADER)
                (((PUCHAR) BufferChecksum) + sizeof(WNODE_HEADER));
        if (LogfileHeaderChecksum) {
            BuffersWritten = LogfileHeaderChecksum->BuffersWritten;
        }

        ByteOffset.QuadPart = 0;
        Status = ZwReadFile(
                    newHandle,
                    NULL,
                    NULL,
                    NULL,
                    & IoStatus,
                    pFirstBuffer,
                    LoggerContext->BufferSize,
                    & ByteOffset,
                    NULL);
        if (NT_SUCCESS(Status)) {
            PWMI_BUFFER_HEADER    BufferFile;
            PTRACE_LOGFILE_HEADER LogfileHeaderFile;
            ULONG Size;

            BufferFile =
                    (PWMI_BUFFER_HEADER) pFirstBuffer;

            if (BufferFile->Wnode.BufferSize != LoggerContext->BufferSize) {
                TraceDebug((1,
                        "WmipCreateLogFile::BufferSize check fails (%d,%d)\n",
                        BufferFile->Wnode.BufferSize,
                        LoggerContext->BufferSize));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }

            if (RtlCompareMemory(BufferFile,
                                 BufferChecksum,
                                 sizeof(WNODE_HEADER))
                        != sizeof(WNODE_HEADER)) {
                TraceDebug((1,"WmipCreateLogFile::WNODE_HEAD check fails\n"));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }

            LogfileHeaderFile = (PTRACE_LOGFILE_HEADER)
                    (((PUCHAR) BufferFile) + sizeof(WMI_BUFFER_HEADER)
                                          + sizeof(SYSTEM_TRACE_HEADER));

            // We can only validate part of the header because a 32-bit
            // DLL will be passing in 32-bit pointers
            Size = FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName);
            if (RtlCompareMemory(LogfileHeaderFile,
                                  LogfileHeaderChecksum,
                                  Size)
                        != Size) {
                TraceDebug((1,
                    "WmipCreateLogFile::TRACE_LOGFILE_HEAD check fails\n"));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }
        }
        else {
            ZwClose(newHandle);
            goto Cleanup;
        }

        if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE) {
            ByteOffset.QuadPart = ((LONGLONG) LoggerContext->BufferSize) * BuffersWritten;
        }
        else {
            Status = ZwQueryInformationFile(
                            newHandle,
                            &IoStatus,
                            &FileSize,
                            sizeof (FILE_STANDARD_INFORMATION),
                            FileStandardInformation
                            );
            if (!NT_SUCCESS(Status)) {
                ZwClose(newHandle);
                goto Cleanup;
            }

            ByteOffset = FileSize.EndOfFile;
        }

        //
        // Force to 1K alignment. In future, if disk alignment exceeds this,
        // then use that
        //
        if ((ByteOffset.QuadPart % 1024) != 0) {
            ByteOffset.QuadPart = ((ByteOffset.QuadPart / 1024) + 1) * 1024;
        }

        if (!(LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE)) {
            // NOTE: Should also validate BuffersWritten from logfile header with
            // the end of file to make sure that no one else has written garbage
            // to it
            //
            if (ByteOffset.QuadPart !=
                        (  ((LONGLONG) LoggerContext->BufferSize)
                         * BuffersWritten)) {
                TraceDebug((1,
                        "WmipCreateLogFile::FileSize check fails (%I64d,%I64d)\n",
                        ByteOffset.QuadPart,
                        (  ((LONGLONG) LoggerContext->BufferSize)
                         * BuffersWritten)));
                Status = STATUS_FAIL_CHECK;
                ZwClose(newHandle);
                goto Cleanup;
            }
        }

        //
        // Before Switching to the newfile, let's finalize the old file
        //

        if ( SwitchFile && (tempHandle != NULL) ) {
            WmipFinalizeHeader(tempHandle, LoggerContext);
        }

        LoggerContext->FirstBufferOffset = ByteOffset;
        LoggerContext->ByteOffset        = ByteOffset;

        if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_PREALLOCATE) {
            LoggerContext->BuffersWritten = BuffersWritten;
        }
        else {
            LoggerContext->BuffersWritten = (ULONG) (FileSize.EndOfFile.QuadPart / LoggerContext->BufferSize);
        }

        LoggerContext->LastFlushedBuffer = LoggerContext->BuffersWritten;

        // Update the log file handle and log file name in the LoggerContext
        LoggerContext->LogFileHandle = newHandle;

        if (SwitchFile) {

            OldLogFileName = LoggerContext->LogFileName;
            LoggerContext->LogFileName = LoggerContext->NewLogFileName;
            FileSwitched = TRUE;

            if ( tempHandle != NULL) {
                //
                // just to be safe, close old file after the switch
                //
                TraceDebug((1, "WmipCreateLogFile: Closing handle %X\n",
                    tempHandle));
                ZwClose(tempHandle);
            }
        }
    }

#if DBG
    else {
        TraceDebug((1,
            "WmipCreateLogFile: ZwCreateFile(%ws) failed with status=%X\n",
            LoggerContext->LogFileName.Buffer, Status));
    }
#endif

Cleanup:
    if (LoggerContext->ClientSecurityContext.ClientToken != NULL) {
        SeDeleteClientSecurity(& LoggerContext->ClientSecurityContext);
        LoggerContext->ClientSecurityContext.ClientToken = NULL;
    }

    // Clean up unicode strings.
    if (SwitchFile) {
        if (!FileSwitched) {
            RtlFreeUnicodeString(&LoggerContext->NewLogFileName);
        }
        else if (OldLogFileName.Buffer != NULL) {
            // OldLogFileName.Buffer can still be NULL if it is the first update
            // for the Kernel Logger.
            RtlFreeUnicodeString(&OldLogFileName);
        }
        // Must do this for the next file switch.
        RtlZeroMemory(&LoggerContext->NewLogFileName, sizeof(UNICODE_STRING));
    }

    if (strLogFileName != NULL) {
        ExFreePool(strLogFileName);
    }
    if (pFirstBuffer != NULL) {
        ExFreePool(pFirstBuffer);
    }
    LoggerContext->LoggerStatus = Status;
    return Status;
}

ULONG
FASTCALL
WmipReleaseTraceBuffer(
    IN PWMI_BUFFER_HEADER BufferResource,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG BufRefCount;
    LONG  ReleaseQueue;

    ASSERT(LoggerContext);
    ASSERT(BufferResource);

    BufRefCount = InterlockedDecrement((PLONG) &BufferResource->ReferenceCount);

    //
    // Check if there are buffers to be flushed.
    //
    if (LoggerContext->ReleaseQueue) {
        if (KeGetCurrentIrql() <= DISPATCH_LEVEL) {
            WmipNotifyLogger(LoggerContext);
            LoggerContext->ReleaseQueue = 0;
        }
    }

    ReleaseQueue = LoggerContext->ReleaseQueue;
    WmipDereferenceLogger(LoggerContext->LoggerId);
    return (ReleaseQueue);
}

NTKERNELAPI
ULONG
FASTCALL
WmiReleaseKernelBuffer(
    IN PWMI_BUFFER_HEADER BufferResource
    )
{
    PWMI_LOGGER_CONTEXT LoggerContext = WmipLoggerContext[WmipKernelLogger];
    if (LoggerContext == (PWMI_LOGGER_CONTEXT) &WmipLoggerContext[0]) {
        LoggerContext = BufferResource->LoggerContext;
    }
    WmipAssert(LoggerContext != NULL);
    WmipAssert(LoggerContext != (PWMI_LOGGER_CONTEXT) &WmipLoggerContext[0]);
    return WmipReleaseTraceBuffer(BufferResource, LoggerContext);
}

NTSTATUS
WmipFlushBuffersWithMarker (
    IN PWMI_LOGGER_CONTEXT  LoggerContext,
    IN PSLIST_ENTRY         List,
    IN USHORT               BufferFlag
    ) 
{
    PSLIST_ENTRY  LocalList, Entry;
    PWMI_BUFFER_HEADER Buffer;
    PWMI_BUFFER_HEADER TmpBuffer=NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Retry;
    ULONG BufferCounts = 0;
    USHORT Flag = WMI_BUFFER_FLAG_NORMAL;
    ULONG ErrorCount = 0;

    LocalList = List;

    //
    // Reverse the list to preserve the FIFO order
    //
    Entry = NULL;
    while (LocalList!=NULL) {
        PSLIST_ENTRY  Next;
        Next = LocalList->Next;
        LocalList->Next = Entry;
        Entry = LocalList;
        LocalList = Next;
        BufferCounts++;
    }
    LocalList = Entry;

    //
    // Write all buffers into disk
    //
    while (LocalList != NULL){
        BufferCounts--;
        if (BufferCounts == 0) {
            //
            // Only set the flag at the last buffer.
            //
            Flag = BufferFlag;
        }

        Entry = LocalList;
        LocalList = LocalList->Next;

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
            //
            // When there is a bursty logging, we can be stuck in this loop.
            // Check if we need to allocate more buffers
            //
            // Only do buffer adjustment if we are not in buffering mode
            //
            WmipAdjustFreeBuffers(LoggerContext);
        }
    

        //
        //
        // Wait until no one else is using the buffer before
        // writing it out.
        //
             
        Retry = 0;
        TmpBuffer = Buffer;
        while (Buffer->ReferenceCount) {
            TraceDebug((1,"Waiting for reference count %3d, retry: %3d\n", 
                        Buffer->ReferenceCount, Retry));
            //
            //
            //
            KeDelayExecutionThread (KernelMode,
                                    FALSE, 
                                    (PLARGE_INTEGER)&WmiShortTime);
            Retry++;
            if (Retry > 10) {
                //
                // The buffer is still in use, we cannot overwite the header.
                // Otherwise it will cause buffer corruption.
                // Use a tempamory buffer instead.
                //
                ULONG BufferSize = LoggerContext->BufferSize;
                TmpBuffer = ExAllocatePoolWithTag(NonPagedPool, 
                                                  BufferSize,
                                                  TRACEPOOLTAG);
    
                if (TmpBuffer) {
                    TraceDebug((1,"Buffer %p has ref count %3d, Tmporary buffer %p Allocated\n", 
                                   Buffer,
                                   Buffer->ReferenceCount,
                                   TmpBuffer));

                    RtlCopyMemory(TmpBuffer, Buffer, BufferSize);
                } else {
                    Status = STATUS_NO_MEMORY;
                }
                break;
            }
        }

        if (TmpBuffer) {
            Status = WmipFlushBuffer(LoggerContext, TmpBuffer, Flag);
        } else {
            //
            // The buffer still in use but allocation of temporary
            // buffer failed.
            // Cannot write this buffer out, claim it as buffer lost
            //

            // If this were the last buffer on file, post processing can
            // fail due to marker buffer failing.  

            LoggerContext->LogBuffersLost++;
        }

        if (TmpBuffer != Buffer) {
            if (TmpBuffer != NULL) {
                ExFreePool(TmpBuffer);
            }
            InterlockedPushEntrySList(&LoggerContext->WaitList,
                                      (PSLIST_ENTRY) &Buffer->SlistEntry);
        } else {
            //
            // Reference count is overwritten during the flush,
            // Set it back.
            //
            Buffer->ReferenceCount = 0;
            WmipPushFreeBuffer (LoggerContext, Buffer);
        }

        if ((Status == STATUS_LOG_FILE_FULL) ||
            (Status == STATUS_DISK_FULL) ||
            (Status == STATUS_NO_DATA_DETECTED) ||
            (Status == STATUS_SEVERITY_WARNING)) {
 
            TraceDebug((1,
                "WmipFlushActiveBuffers: Buffer flushed with status=%X\n",
                Status));
            if ((Status == STATUS_LOG_FILE_FULL) ||
                (Status == STATUS_DISK_FULL)) {
                ErrorCount ++;
            } else {
                ErrorCount = 0; // reset to zero otherwise
            }

            if (ErrorCount <= WmiWriteFailureLimit) {
                Status = STATUS_SUCCESS;     // Let tracing continue
                continue;       // for now. Should raise WMI event
            }
        }
    }

    if (!NT_SUCCESS(Status)) {
        TraceDebug((1,
            "WmipLogger: Flush failed, status=%X LoggerContext=%X\n",
                 Status, LoggerContext));
        if (LoggerContext->LogFileHandle != NULL) {
#if DBG
            NTSTATUS CloseStatus = 
#endif
            ZwClose(LoggerContext->LogFileHandle);
            TraceDebug((1,
                "WmipLogger: Close logfile with status=%X\n", CloseStatus));
        }
        LoggerContext->LogFileHandle = NULL;

        WmipSendNotification(LoggerContext,
            Status, (Status & 0xC0000000) >> 30);

    }

    return Status;
}

NTSTATUS
WmipFlushActiveBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG FlushAll
    )
{
    PWMI_BUFFER_HEADER Buffer;
    ULONG i;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG LoggerMode;
    PSLIST_ENTRY  LocalList, Entry;


    PAGED_CODE();

    //
    // If we have no LogFileHandle or RealTime mode, and the collection
    // is still on, we simply return and let the buffers back up potentially
    // losing events. If the Collection is Off under these conditions
    // we will simply move the FlushList to FreeList as in Buffering mode. 
    //

    LoggerMode = LoggerContext->LoggerMode;

    if ( (LoggerContext->LogFileHandle == NULL)       &&
         (!(LoggerMode & EVENT_TRACE_REAL_TIME_MODE)) &&
         (LoggerContext->CollectionOn) ) 
    {
        return Status;
    }


    LocalList = NULL;
    if (FlushAll) {
        PWMI_BUFFER_HEADER Buffers[MAXIMUM_PROCESSORS];
        //
        // First switch all in-used buffers.
        // Put them in a tighter loop to minimize the chance of
        // events out of order.
        //
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            Buffers[i] = InterlockedExchangePointer(&LoggerContext->ProcessorBuffers[i], NULL);
        }

        //
        // Put all in-used buffers into flush list
        //
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            if (Buffers[i]) {
                WmipPushDirtyBuffer ( LoggerContext, Buffers[i] );
            }
        }

        if (LoggerContext->LoggerId == WmipKernelLogger) {
            if ( PERFINFO_IS_GROUP_ON(PERF_CONTEXT_SWITCH) ) {
                for (i=0; i<(ULONG)KeNumberProcessors; i++) {
                    //
                    // Flush all Buffers used for Context swaps
                    //
                    Buffer = InterlockedExchangePointer(&WmipContextSwapProcessorBuffers[i], NULL);
                    if (Buffer) {
                        WmipPushDirtyBuffer ( LoggerContext, Buffer);
                    }
                }
            }
        }

        //
        //  Now push all dirty buffers in the local list.
        //  This almost guarantees that the flush marker will work.
        //
        if (ExQueryDepthSList(&LoggerContext->FlushList) != 0) {
            LocalList = ExInterlockedFlushSList (&LoggerContext->FlushList);
            WmipFlushBuffersWithMarker (LoggerContext, LocalList, WMI_BUFFER_FLAG_FLUSH_MARKER);
        }

    } else if (ExQueryDepthSList(&LoggerContext->FlushList) != 0) {
        LocalList = ExInterlockedFlushSList (&LoggerContext->FlushList);
        WmipFlushBuffersWithMarker (LoggerContext, LocalList, WMI_BUFFER_FLAG_NORMAL);
    }

    //
    // Now check if any of the in-used buffer is freed.
    //
    if (ExQueryDepthSList(&LoggerContext->WaitList) != 0) {
        LocalList = ExInterlockedFlushSList (&LoggerContext->WaitList);
        while (LocalList != NULL){
            Entry = LocalList;
            LocalList = LocalList->Next;

            Buffer = CONTAINING_RECORD(Entry,
                                       WMI_BUFFER_HEADER,
                                       SlistEntry);

            TraceDebug((1,"Wait List Buffer %p RefCount: %3d\n", 
                           Buffer,
                           Buffer->ReferenceCount));

            if (Buffer->ReferenceCount) {

                //
                // Still in use, put it back to WaitList
                //
                InterlockedPushEntrySList(&LoggerContext->WaitList,
                                          (PSLIST_ENTRY) &Buffer->SlistEntry);

            } else {
                //
                // Push it to free list
                //
                WmipPushFreeBuffer (LoggerContext, Buffer);
            }
        }

    }
    return Status;
}

NTSTATUS
WmipGenerateFileName(
    IN PUNICODE_STRING FilePattern,
    IN OUT PLONG FileCounter,
    OUT PUNICODE_STRING FileName
    )
{
    LONG FileCount, Size;
    PWCHAR Buffer = NULL;
    HRESULT hr;
    PWCHAR wcptr;

    PAGED_CODE();

    if (FilePattern->Buffer == NULL)
        return STATUS_INVALID_PARAMETER_MIX;

    // Check for valid format string
    wcptr = wcschr(FilePattern->Buffer, L'%');
    if (NULL == wcptr || wcptr != wcsrchr(FilePattern->Buffer, L'%')) {
        return STATUS_OBJECT_NAME_INVALID;
    }
    else if (NULL == wcsstr(FilePattern->Buffer, L"%d")) {
        return STATUS_OBJECT_NAME_INVALID;
    }

    FileCount = InterlockedIncrement(FileCounter);
    Size = FilePattern->MaximumLength + 64; // 32 digits: plenty for ULONG

    Buffer = ExAllocatePoolWithTag(PagedPool, Size, TRACEPOOLTAG);
    if (Buffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    hr = StringCbPrintfW(Buffer, Size, FilePattern->Buffer, FileCount);

    if (FAILED(hr) || RtlEqualMemory(FilePattern->Buffer, Buffer, FilePattern->Length)) {
        ExFreePool(Buffer);
        return STATUS_INVALID_PARAMETER_MIX;
    }
    RtlInitUnicodeString(FileName, Buffer);
    return STATUS_SUCCESS;
}

NTSTATUS
WmipPrepareHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    )
/*++

Routine Description:
    This routine prepares the buffer header before writing the
    buffer out to Disk.

    If (SavedOffset > 0), it must be the case the we have overflown
    CurrentOffset during WmipReserveTraceBuffer.  SavedOffset should
    be the real offset of dirty buffer area.

    If SavedOffset is not set, it is either a ContextSwap buffer
    or a buffer flushed due to flush timer.  CurrentOffset should
    be used for writting it out.


Calling Functions:
    - WmipFlushBuffer

Arguments:

    LoggerContext - Logger context

    Buffer        - Pointer to a buffer header that we wish to write to disk

Return Value:

    NtStatus.

--*/
{
    ULONG BufferSize;
    PAGED_CODE();

    BufferSize = LoggerContext->BufferSize;

    if (Buffer->SavedOffset > 0) {
        Buffer->Offset = Buffer->SavedOffset;
    }
    else {
        if (Buffer->CurrentOffset > BufferSize) {
            //
            // Some thread has incremented CurrentOffset but was swapped out 
            // and did not come back until the buffer is about to be flushed.
            // We will correct the CurrentOffset here and hope that post
            // processing will handle this correctly.
            //
            TraceDebug((3, "WmipPrepareHeader: correcting Buffer Offset %d, RefCount: %d\n",
                        Buffer->CurrentOffset, Buffer->ReferenceCount));
            Buffer->Offset = BufferSize;
        }
        else {
            Buffer->Offset = Buffer->CurrentOffset;
        }
    }

    ASSERT (Buffer->Offset >= sizeof(WMI_BUFFER_HEADER));
    ASSERT (Buffer->Offset <= LoggerContext->BufferSize);


    //
    // We write empty buffers if they have FLUSH_MARKER to facilitate 
    // post processing
    //

    if ( (BufferFlag != WMI_BUFFER_FLAG_FLUSH_MARKER) && (Buffer->Offset == sizeof(WMI_BUFFER_HEADER)) ) { // empty buffer
        return STATUS_NO_DATA_DETECTED;
    }

    //
    // Fill the rest of buffer with 0XFF
    //
    if ( Buffer->Offset < BufferSize ) {
        RtlFillMemory(
            (char *) Buffer + Buffer->Offset,
            BufferSize - Buffer->Offset,
            0XFF);
    }

    Buffer->Wnode.BufferSize = BufferSize;
    Buffer->ClientContext.LoggerId = (USHORT) LoggerContext->LoggerId;
    if (Buffer->ClientContext.LoggerId == 0)
        Buffer->ClientContext.LoggerId = (USHORT) KERNEL_LOGGER_ID;

    Buffer->ClientContext.Alignment = (UCHAR) WmiTraceAlignment;
    Buffer->Wnode.Guid = LoggerContext->InstanceGuid;
    Buffer->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    Buffer->Wnode.ProviderId = LoggerContext->BuffersWritten+1;
    Buffer->BufferFlag = BufferFlag;

    KeQuerySystemTime(&Buffer->Wnode.TimeStamp);
    return STATUS_SUCCESS;
}

NTKERNELAPI
VOID
WmiBootPhase1(
    )                
/*++

Routine Description:

    NtInitializeRegistry to inform WMI that autochk is performed
    and it is OK now to write to disk.

Arguments:

    None

Return Value:

    None

--*/

{
    PAGED_CODE();

    WmipFileSystemReady = TRUE;
}


NTSTATUS
WmipFinalizeHeader(
    IN HANDLE FileHandle,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    LARGE_INTEGER ByteOffset;
    NTSTATUS Status;
    PTRACE_LOGFILE_HEADER FileHeader;
    IO_STATUS_BLOCK IoStatus;
    CHAR Buffer[PAGE_SIZE];     // Assumes Headers less than PAGE_SIZE

    PAGED_CODE();

    ByteOffset.QuadPart = 0;
    Status = ZwReadFile(
                FileHandle,
                NULL,
                NULL,
                NULL,
                & IoStatus,
                &Buffer[0],
                PAGE_SIZE,
                & ByteOffset,
                NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    FileHeader = (PTRACE_LOGFILE_HEADER)
                 &Buffer[sizeof(WMI_BUFFER_HEADER) + sizeof(SYSTEM_TRACE_HEADER)];
    FileHeader->BuffersWritten = LoggerContext->BuffersWritten;

    if (LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_TRANSITION) {
        FileHeader->StartBuffers = (ULONG) 
                                   (LoggerContext->FirstBufferOffset.QuadPart
                                   / LoggerContext->BufferSize);
    }

    KeQuerySystemTime(&FileHeader->EndTime);
    if (LoggerContext->Wow && !LoggerContext->KernelTraceOn) {
        // We need to adjust a log file header for a non-kernel WOW64 logger.
        *((PULONG)((PUCHAR)(&FileHeader->BuffersLost) - 8)) 
                                            = LoggerContext->LogBuffersLost;
    }
    else {
        FileHeader->BuffersLost = LoggerContext->LogBuffersLost;
    }
    FileHeader->EventsLost = LoggerContext->EventsLost;
    Status = ZwWriteFile(
                FileHandle,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                &Buffer[0],
                PAGE_SIZE,
                &ByteOffset,
                NULL);
    return Status;
}

#if DBG

#define DEBUG_BUFFER_LENGTH 1024
UCHAR TraceDebugBuffer[DEBUG_BUFFER_LENGTH];

VOID
TraceDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

/*++

Routine Description:

    Debug print for all DiskPerf

Arguments:

    Debug print level between 0 and 3, with 3 being the most verbose.

Return Value:

    None

--*/

{
    LARGE_INTEGER Clock;
    ULONG Tid;
    va_list ap;

    va_start(ap, DebugMessage);


    if  (WmipTraceDebugLevel >= DebugPrintLevel) {

        StringCbVPrintfA((PCHAR)TraceDebugBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

        Clock = KeQueryPerformanceCounter(NULL);
        Tid = HandleToUlong(PsGetCurrentThreadId());
        DbgPrintEx(DPFLTR_WMILIB_ID, DPFLTR_INFO_LEVEL,
                   "%u (%5u): %s", Clock.LowPart, Tid, TraceDebugBuffer);
    }

    va_end(ap);

}
#endif //DBG


VOID
FASTCALL
WmipResetBufferHeader (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer,
    USHORT                  BufferType
    )
/*++

Routine Description:
    This is a function which initializes a few buffer header values
    that are used by both WmipGetFreeBuffer and WmipPopFreeContextSwapBuffer

    Note that this function increments a few logger context reference counts

Calling Functions:
    - WmipGetFreeBuffer
    - WmipPopFreeContextSwapBuffer

Arguments:

    LoggerContext - Logger context from where we have acquired a free buffer

    Buffer        - Pointer to a buffer header that we wish to reset

    BufferType    - Buffer type (e.g., Generic, ContextSwap, etc.).
                    This is to make postprocessing easier.

Return Value:

    None

--*/
{
    ASSERT (BufferType < WMI_BUFFER_TYPE_MAXIMUM);
    Buffer->SavedOffset = 0;
    Buffer->CurrentOffset = sizeof(WMI_BUFFER_HEADER);
    Buffer->Wnode.ClientContext = 0;
    Buffer->LoggerContext = LoggerContext;
    Buffer->BufferType = BufferType;
       
    Buffer->State.Free = 0;
    Buffer->State.InUse = 1;

}


VOID
FASTCALL
WmipPushDirtyBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
)
/*++

Routine Description:
    This is a function which prepares a buffer's header and places it on a
    logger's flush list.  

    Note that this function manages a few logger context reference counts

Calling Functions:
    - WmipFlushActiveBuffers
    - WmipPushDirtyContextSwapBuffer

Arguments:

    LoggerContext - Logger context from which we originally acquired a buffer

    Buffer        - Pointer to a buffer that we wish to flush

Return Value:

    None

--*/
{
    ASSERT(Buffer->State.Flush == 0);
    ASSERT(Buffer->State.Free == 0);
    ASSERT(Buffer->State.InUse == 1);
    //
    // Set the buffer flags to "flush" state
    //
    Buffer->State.InUse = 0;
    Buffer->State.Flush = 1;

    //
    // Push the buffer onto the flushlist.  This could only
    // fail if the Wmi kernel logger shut down without notifying us.
    // If this happens, there is nothing we can do about it anyway.
    // If Wmi is well behaved, this will never fail.
    //
    InterlockedPushEntrySList(
        &LoggerContext->FlushList,
        (PSLIST_ENTRY) &Buffer->SlistEntry);

    //
    // Maintain some reference counts
    //
    InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    InterlockedIncrement((PLONG) &LoggerContext->BuffersDirty);


    TraceDebug((2, "Flush Dirty Buffer: %p, Free: %d, InUse: %d, %Dirty: %d, Total: %d, (Thread: %p)\n",
                    Buffer,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers,
                    PsGetCurrentThread()));
}


VOID
FASTCALL
WmipPushFreeBuffer (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer
)
/*++

Routine Description:
    This is a function which prepares a buffer's header and places it on a
    logger's Free list.  

    Note that this function manages a few logger context reference counts

Calling Functions:
    - WmipFlushActiveBuffers
    - WmipGetFreeBuffer

Arguments:

    LoggerContext - Logger context from which we originally acquired a buffer

    Buffer        - Pointer to a buffer that we wish to flush

Return Value:

    None

--*/
{
    //
    // Set the buffer flags to "free" state and save the offset
    //
    Buffer->State.Flush = 0;
    Buffer->State.InUse = 0;
    Buffer->State.Free = 1;

    //
    // Push the buffer onto the free list.  
    //
    InterlockedPushEntrySList(&LoggerContext->FreeList,
                              (PSLIST_ENTRY) &Buffer->SlistEntry);

    //
    // Maintain the reference counts
    //
    InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
    InterlockedDecrement((PLONG) &LoggerContext->BuffersDirty);

    TraceDebug((2, "Push Free Buffer: %p, Free: %d, InUse: %d, %Dirty: %d, Total: %d, (Thread: %p)\n",
                    Buffer,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers,
                    PsGetCurrentThread()));
}


PWMI_BUFFER_HEADER
FASTCALL
WmipPopFreeContextSwapBuffer
    ( UCHAR CurrentProcessor
    )
/*++

Routine Description:

    Attempts to remove a buffer from the kernel logger free buffer list.
    We confirm that logging is on, that buffer switching is
    not in progress and that the buffers available count is greater than
    zero.  If we are unable to acquire a buffer, we increment LostEvents
    and return.  Otherwise, we initialize the buffer and pass it back.

    Assumptions:
    - This routine will only be called from WmiTraceContextSwap
    - Inherits all assumptions listed in WmiTraceContextSwap

    Calling Functions:
    - WmiTraceContextSwap

Arguments:

    CurrentProcessor- The current processor number (0 to (NumProc - 1))

Return Value:

    Pointer to the newly acquired buffer.  NULL on failure.

--*/
{
    PWMI_LOGGER_CONTEXT LoggerContext;
    PWMI_BUFFER_HEADER  Buffer;
        
    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    
    //
    // Could only happen if for some reason the logger has not been initialized
    // before we see the global context swap flag set.  This should not happen.
    //
    if(! WmipIsValidLogger(LoggerContext) ) {
        return NULL;
    }

    //
    // "Switching" is a Wmi state available only while BUFFERING is
    // enabled that occurs when the free buffer list is empty. During switching
    // all the buffers in the flushlist are simply moved back to the free list.
    // Normally if we found that the free list was empty we would perform the
    // switch here, and if the switch was already occurring we would spin until
    // it completed.  Instead of introducing an indefinite spin, as well as a
    // ton of interlocked pops and pushes, we opt to simply drop the event.
    //
    if ( !(LoggerContext->SwitchingInProgress) 
        && LoggerContext->CollectionOn
        && LoggerContext->BuffersAvailable > 0) {

        //
        // Attempt to get a free buffer from the Kernel Logger FreeList
        //
        Buffer = (PWMI_BUFFER_HEADER)InterlockedPopEntrySList(
            &LoggerContext->FreeList);

        //
        // This second check is necessary because
        // LoggerContext->BuffersAvailable may have changed.
        //
        if(Buffer != NULL) {

            Buffer = CONTAINING_RECORD (Buffer, WMI_BUFFER_HEADER, SlistEntry);

            //
            // Reset the buffer header
            //
            WmipResetBufferHeader( LoggerContext, Buffer, WMI_BUFFER_TYPE_CTX_SWAP);
            //
            // Maintain some Wmi logger context buffer counts
            //
            InterlockedDecrement((PLONG) &LoggerContext->BuffersAvailable);
            InterlockedIncrement((PLONG) &LoggerContext->BuffersInUse);

            Buffer->ClientContext.ProcessorNumber = CurrentProcessor;
            Buffer->Offset = LoggerContext->BufferSize;

            ASSERT( Buffer->Offset % WMI_CTXSWAP_EVENTSIZE_ALIGNMENT == 0);

            // Return our buffer
            return Buffer;
        }
    }
    
    LoggerContext->EventsLost++;
    return NULL;
}

VOID
FASTCALL
WmipPushDirtyContextSwapBuffer (
    UCHAR               CurrentProcessor,
    PWMI_BUFFER_HEADER  Buffer
    )
/*++

Routine Description:

    Prepares the current buffer to be placed on the Wmi flushlist
    and then pushes it onto the flushlist.  Maintains some Wmi
    Logger reference counts.

    Assumptions:
    - The value of WmipContextSwapProcessorBuffers[CurrentProcessor]
      is not equal to NULL, and the LoggerContext reference count
      is greater than zero.

    - This routine will only be called when the KernelLogger struct
      has been fully initialized.

    - The Wmi kernel WMI_LOGGER_CONTEXT object, as well as all buffers
      it allocates are allocated from nonpaged pool.  All Wmi globals
      that we access are also in nonpaged memory

    - This code has been locked into paged memory when the logger started

    - The logger context reference count has been "Locked" via the 
      InterlockedIncrement() operation in WmipReferenceLogger(WmipLoggerContext)

    Calling Functions:
    - WmiTraceContextSwap

    - WmipStopContextSwapTrace

Arguments:

    CurrentProcessor    Processor we are currently running on

    Buffer              Buffer to be flushed
    
Return Value:

    None
    
--*/
{
    PWMI_LOGGER_CONTEXT     LoggerContext;

    UNREFERENCED_PARAMETER (CurrentProcessor);

    //
    // Grab the kernel logger context
    // This should never be NULL as long as we keep the KernelLogger
    // reference count above zero via "WmipReferenceLogger"
    //
    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    if( ! WmipIsValidLogger(LoggerContext) ) {
        return;
    }

    WmipPushDirtyBuffer( LoggerContext, Buffer );

    //
    // Increment the ReleaseQueue count here. We can't signal the 
    // logger semaphore here while holding the context swap lock.
    //
    InterlockedIncrement(&LoggerContext->ReleaseQueue);

    return;
}

NTSTATUS
WmipSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER *BufferPointer,
    IN PVOID BufferPointerLocation,
    IN ULONG ProcessorNumber
    ) 
/*++

Routine Description:

    This routine is used to switch buffers when a buffer is Full.
    
    The mechanism is as follows:

    1. The caller gives us a buffer (OldBuffer) that needs to be switched.
    2. Get a new buffer and use InterlockedCompareExchangePointer to switch buffers
       only if the OldBuffer is still not switched.
    3. If the OldBuffer has been switched, ask the caller to try using the newly
       switched buffer for logging.
    
Assumptions:

    - The LoggerContext is locked before this routine is called.

Arguments:

    LoggerContext - Logger Context.

    BufferPointer - The Old buffer that needs to be switched.

    BufferPointerLocation - The location of the buffer pointer for switching.

    ProcessorNumber - Processor Id. Processor Id is set before switching.

Return Value:

    Status
    
--*/
{
    PWMI_BUFFER_HEADER CurrentBuffer, NewBuffer, OldBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    //
    // Get a new buffer from Free List
    //

    if (!LoggerContext->CollectionOn) {
        Status = STATUS_WMI_ALREADY_DISABLED;
        NewBuffer = NULL;
    } else {
        //
        // Allocate a buffer.
        //

        NewBuffer = WmipGetFreeBuffer (LoggerContext);
        if (NewBuffer) {
            NewBuffer->ClientContext.ProcessorNumber = (UCHAR) ProcessorNumber;

            OldBuffer = *BufferPointer;
    
            CurrentBuffer = InterlockedCompareExchangePointer(
                                BufferPointerLocation,
                                NewBuffer,
                                OldBuffer);
            //
            // There are 3 cases that we need to consider depending on the outcome
            // of InterlockedCompareExchangePointer.
            //
            // 1. CurrentBuffer is NULL and OldBuffer is not. This means FlushAll 
            //    Code path has replaced ProcessorBuffers with NULL after this 
            //    thread got into WmipReserveTraceBuffer. If this is the case, we 
            //    need to do InterlockedCompareExchangePointer with NULL pointer 
            //    one more time to push the good free buffer into ProcessorBuffer.
            // 2. CurrentBuffer is not NULL, but it is not the same buffer we had, 
            //    which means somebody already switched the buffer with a new one.
            //    We push the new buffer we have into FreeList and use 
            //    the current ProcessorBuffer.
            // 3. CurrentBuffer is the same as OldBuffer and it is not NULL, which
            //    means Switching succeeded. Push the old buffer into FlushList and 
            //    wake up the logger thread.
            //
            // If both CurrentBuffer and OldBuffer are NULL, we just switch.
            //
            if (OldBuffer != NULL && CurrentBuffer == NULL) {
                CurrentBuffer = InterlockedCompareExchangePointer(
                                    BufferPointerLocation,
                                    NewBuffer,
                                    NULL);

                //
                // If CurrentBuffer is NULL, we successfully pushed the clean free 
                // buffer into ProcessorBuffer. NewBuffer already points to a new clean
                // Buffer, and WmipFlushActiveBuffers already handled the old buffer 
                // (Pusing to FlushList and all), so there is no need to do anything.
                //

                if (CurrentBuffer != NULL) {
                    //
                    // Somebody pushed a new buffer to ProcessorBuffer between two
                    // InterlockedCompareExchangePointer calls.
                    // We will use the ProcessorBuffer and push our new buffer into
                    // FreeList.
                    //
                    InterlockedPushEntrySList(&LoggerContext->FreeList,
                                        (PSLIST_ENTRY) &NewBuffer->SlistEntry);
                    InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
                    InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    
                    NewBuffer = CurrentBuffer;
                }
            } else if (OldBuffer != CurrentBuffer) {
                //
                // Someone has switched the buffer, use this one
                // and push the new allocated buffer back to free list.
                //
                InterlockedPushEntrySList(&LoggerContext->FreeList,
                                    (PSLIST_ENTRY) &NewBuffer->SlistEntry);
                InterlockedIncrement((PLONG) &LoggerContext->BuffersAvailable);
                InterlockedDecrement((PLONG) &LoggerContext->BuffersInUse);
    
                NewBuffer = CurrentBuffer;
            } else if (OldBuffer != NULL) {
                //
                // Successfully switched the buffer, push the current buffer into
                // flush list
                //
                WmipPushDirtyBuffer( LoggerContext, OldBuffer );

                if (!(LoggerContext->LoggerMode & EVENT_TRACE_BUFFERING_MODE)) {
                    if (KeGetCurrentIrql() <= DISPATCH_LEVEL) {
                        //
                        // Wake up the walker thread to write it out to disk.
                        //
                        WmipNotifyLogger(LoggerContext);
                    } else {
                        //
                        // Queue the item.
                        //
                        InterlockedIncrement(&LoggerContext->ReleaseQueue);
                    }
                }
            }
        } else {
            //
            // There is no free buffer to switch with. NewBuffer is NULL.
            // We just push the processor buffer into FlushList and exit.
            // If we don't do this, CurrentOffset in the processor buffer header 
            // may overflow. 
            //
            OldBuffer = *BufferPointer;
            CurrentBuffer = InterlockedCompareExchangePointer(
                                BufferPointerLocation,
                                NULL,
                                OldBuffer);
            //
            // CurrentBuffer is not NULL, so either we switched it to NULL 
            // ourselves, or someone else has done so right before us.
            //
            if (CurrentBuffer != NULL) {
                if (CurrentBuffer == OldBuffer) {
                    // We switched successfully.
                    // Push the processor buffer to FlushList.
                    WmipPushDirtyBuffer (LoggerContext, OldBuffer);
                    Status = STATUS_NO_MEMORY;
                }
                else { 
                    // Someone has pushed a new free buffer to a processor.
                    // We will try using this buffer.
                    NewBuffer = CurrentBuffer;
                }
            }
            else {
                Status = STATUS_NO_MEMORY;
            }
        }
    }

    TraceDebug((2, "Switching CPU Buffers, CurrentOne: %p\n", *BufferPointer));

    *BufferPointer = NewBuffer;

    TraceDebug((2, "Switching CPU Buffers, New One  : %p, %x\n", *BufferPointer, Status));

    return(Status); 
}

