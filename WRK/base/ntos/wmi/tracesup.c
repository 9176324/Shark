/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    tracesup.c

Abstract:

    This is the source file that implements the private routines of
    the performance event tracing and logging facility. These routines
    work on manipulating the LoggerContext table and synchronization
    across event tracing sessions.

--*/

#include "wmikmp.h"
#include <ntos.h>
#include <evntrace.h>

#include <wmi.h>
#include "tracep.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define KERNEL_LOGGER_CAPTION   L"NT Kernel Logger"
#define DEFAULT_BUFFERS         2
#define DEFAULT_AGE_LIMIT       15          // 15 minutes
#define SEMAPHORE_LIMIT         1024
#define CONTEXT_SIZE            PAGE_SIZE
#define DEFAULT_MAX_IRQL        DISPATCH_LEVEL
#define DEFAULT_MAX_BUFFERS     20

ULONG WmipKernelLogger = KERNEL_LOGGER;
ULONG WmipEventLogger = 0XFFFFFFFF;
KGUARDED_MUTEX WmipTraceGuardedMutex;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG   WmipLoggerCount = 0;
HANDLE  EtwpPageLockHandle = NULL;
PSECURITY_DESCRIPTOR EtwpDefaultTraceSecurityDescriptor;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

extern SIZE_T MmMaximumNonPagedPoolInBytes;
extern SIZE_T MmSizeOfPagedPoolInBytes;

WMI_GET_CPUCLOCK_ROUTINE WmiGetCpuClock = &WmipGetSystemTime;

//
// Function prototypes for routines used locally
//

NTSTATUS
WmipLookupLoggerIdByName(
    IN PUNICODE_STRING Name,
    OUT PULONG LoggerId
    );

PWMI_LOGGER_CONTEXT
WmipInitContext(
    );

NTSTATUS
WmipAllocateTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFreeTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, WmipStartLogger)
#pragma alloc_text(PAGE, WmipQueryLogger)
#pragma alloc_text(PAGE, WmipStopLoggerInstance)
#pragma alloc_text(PAGE, WmipVerifyLoggerInfo)
#pragma alloc_text(PAGE, WmipExtendBase)
#pragma alloc_text(PAGE, WmipFreeLoggerContext)
#pragma alloc_text(PAGE, WmipInitContext)
#pragma alloc_text(PAGE, WmipAllocateTraceBufferPool)
#pragma alloc_text(PAGE, WmipFreeTraceBufferPool)
#pragma alloc_text(PAGE, WmipLookupLoggerIdByName)
#pragma alloc_text(PAGE, WmipShutdown)
#pragma alloc_text(PAGE, WmipFlushLogger)
#pragma alloc_text(PAGE, WmipNtDllLoggerInfo)
#pragma alloc_text(PAGE, WmipValidateClockType)
/* Look at the comments in the function body
#pragma alloc_text(PAGE, WmipDumpGuidMaps)
#pragma alloc_text(PAGE, WmipGetTraceBuffer)
*/
#pragma alloc_text(PAGEWMI, WmipNotifyLogger)
#endif


NTSTATUS
WmipStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
/*++

Routine Description:

    It is called by WmipIoControl in wmi.c, with IOCTL_WMI_START_LOGGER
    to start up an instance of the logger. It basically creates and
    initializes the logger instance context, and starts up a system
    thread for the logger (WmipLogger()). If the user has requested to
    turn on kernel tracing, it will also lock in the necessary routines
    after the logger has started.
    NOTE: A special instance (KERNEL_LOGGER) is reserved exclusively for
    logging kernel tracing.

    To turn on KERNEL_LOGGER, LoggerInfo->Wnode.Guid should be set to
    SystemTraceControlGuid, and sufficient space must be provided in
    LoggerInfo->LoggerName.

    To turn on other loggers, simply provide a name in LoggerName. The
    logger id will be returned.

Arguments:

    LoggerInfo     a pointer to the structure for the logger's control
                    and status information

Return Value:

    The status of performing the action requested.

--*/

{
    NTSTATUS Status;
    ULONG               LoggerId, EnableKernel, EnableFlags;
    HANDLE              ThreadHandle;
    PWMI_LOGGER_CONTEXT LoggerContext;
    LARGE_INTEGER       TimeOut = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
    ACCESS_MASK         DesiredAccess = TRACELOG_GUID_ENABLE;
    PWMI_LOGGER_CONTEXT *ContextTable;
    PFILE_OBJECT        FileObject;
    GUID                InstanceGuid;
    KPROCESSOR_MODE     RequestorMode;
    SECURITY_QUALITY_OF_SERVICE ServiceQos;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;
    PERFINFO_GROUPMASK *PerfGroupMasks=NULL;
    BOOLEAN             IsGlobalForKernel = FALSE;
    BOOLEAN             IsKernelRealTimeNoFile = FALSE;
    ULONG               GroupMaskSize;
    UNICODE_STRING      FileName, LoggerName;
    ULONG               LogFileMode;
#if DBG
    LONG                RefCount;
#endif

    PAGED_CODE();
    if (LoggerInfo == NULL)
        return STATUS_SEVERITY_ERROR;

    //
    // try and check for bogus parameter
    // if the size is at least what we want, we have to assume it's valid
    //
    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return STATUS_INVALID_BUFFER_SIZE;

    if (! (LoggerInfo->Wnode.Flags & WNODE_FLAG_TRACED_GUID) )
        return STATUS_INVALID_PARAMETER;

    LogFileMode = LoggerInfo->LogFileMode;
    if ( (LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) &&
         (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( (LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) &&
         (LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) ) {
        return STATUS_INVALID_PARAMETER;
    }

/*    if (LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) {
        if ((LoggerInfo->LogFileName.Length == 0) ||
            (LoggerInfo->LogFileName.Buffer == NULL) )
            return STATUS_INVALID_PARAMETER;
    }*/
    if ( !(LogFileMode & EVENT_TRACE_REAL_TIME_MODE) ) {
        if ( !(LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) )
            if (LoggerInfo->LogFileHandle == NULL)
                return STATUS_INVALID_PARAMETER;
    }

    // Cannot support append to circular
    if ( (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
         (LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) ) {
        return STATUS_INVALID_PARAMETER;
    }


    if (LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        DesiredAccess |= TRACELOG_CREATE_REALTIME;
    }

    if ((LoggerInfo->LogFileHandle != NULL) ||
        (LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)) {
        DesiredAccess |= TRACELOG_CREATE_ONDISK;
    }

    EnableFlags = LoggerInfo->EnableFlags;
    if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
        FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &EnableFlags;

        if ((FlagExt->Length == 0) || 
            (FlagExt->Offset == 0) ||
            (LoggerInfo->Wnode.BufferSize < FlagExt->Offset)) {
            return STATUS_INVALID_PARAMETER;
        }
        if ((FlagExt->Length * sizeof(ULONG)) >
            (LoggerInfo->Wnode.BufferSize - FlagExt->Offset))
            return STATUS_INVALID_PARAMETER;
    }

    if (LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
        if ((LoggerInfo->LogFileName.Buffer == NULL) ||
            (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ||
            (LoggerInfo->MaximumFileSize == 0) ||
            IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid))
            return STATUS_INVALID_PARAMETER;
    }

    if (LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
        // Default Minimum Buffers and default Buffer Size are computed
        // later in the Context initialization after Context allocation.
        // To avoid having to allocate memory for this error checking,
        // we compute local parameters. 
        ULONG LocalMinBuffers = (ULONG)KeNumberProcessors + DEFAULT_BUFFERS;
        ULONG LocalBufferSize = PAGE_SIZE / 1024;
        ULONG LocalMaxBuffers; 
        SIZE_T WmiMaximumPoolInBytes;

        if (LoggerInfo->BufferSize > 0) {
            if (LoggerInfo->BufferSize > MAX_WMI_BUFFER_SIZE) {
                LocalBufferSize = MAX_WMI_BUFFER_SIZE;
            }
            else {
                LocalBufferSize = LoggerInfo->BufferSize;
            }
        }
        if (LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
            WmiMaximumPoolInBytes = MmSizeOfPagedPoolInBytes;
        }
        else {
            WmiMaximumPoolInBytes = MmMaximumNonPagedPoolInBytes;
        }
        LocalMaxBuffers = (ULONG) (WmiMaximumPoolInBytes
                            / TRACE_MAXIMUM_NP_POOL_USAGE
                            / LocalBufferSize);
        if (LoggerInfo->MaximumBuffers != 0 && 
            LoggerInfo->MaximumBuffers < LocalMaxBuffers) {
            LocalMaxBuffers = LoggerInfo->MaximumBuffers;
        }
        if (LocalMinBuffers < LoggerInfo->MinimumBuffers && 
            LoggerInfo->MinimumBuffers < LocalMaxBuffers) {
            LocalMinBuffers = LoggerInfo->MinimumBuffers;
        }
        // MaximumFileSize must be multiples of buffer size
        if ((LoggerInfo->LogFileName.Buffer == NULL) ||
            (LoggerInfo->MaximumFileSize == 0) || 
            ((LoggerInfo->MaximumFileSize % LocalBufferSize) != 0) ||
            (LoggerInfo->MaximumFileSize < (LocalMinBuffers * LocalBufferSize))) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    RequestorMode = KeGetPreviousMode();

    LoggerName.Buffer = NULL;

    if (LoggerInfo->LoggerName.Length > 0) {
        try {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length,
                    sizeof (UCHAR) );
            }
            if (! RtlCreateUnicodeString(
                    &LoggerName,
                    LoggerInfo->LoggerName.Buffer) ) {
                return STATUS_NO_MEMORY;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            if (LoggerName.Buffer) {
                RtlFreeUnicodeString(&LoggerName);
            }
            return GetExceptionCode();
        }
        Status = WmipLookupLoggerIdByName(&LoggerName, &LoggerId);
        if (NT_SUCCESS(Status)) {
            RtlFreeUnicodeString(&LoggerName);
            return STATUS_OBJECT_NAME_COLLISION;
        }
    }

    ContextTable = (PWMI_LOGGER_CONTEXT *) &WmipLoggerContext[0];

    // If NULL GUID is given, generate a random GUID.
    RtlZeroMemory(&InstanceGuid, sizeof(GUID));
    if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &InstanceGuid)) {
        Status = ExUuidCreate(&LoggerInfo->Wnode.Guid);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }
    else {
        InstanceGuid = LoggerInfo->Wnode.Guid;
    }

    EnableKernel = IsEqualGUID(&InstanceGuid, &SystemTraceControlGuid);

    if (EnableKernel) {
        //
        // Check if this is the Real-Time No LogFile case.
        //
        if ((LogFileMode & EVENT_TRACE_REAL_TIME_MODE) &&
            !(LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)){

            IsKernelRealTimeNoFile = TRUE;

        }
        //
        // This prevents multiple threads from continuing beyond this
        // point in the code.  Only the first thread will progress.
        //
        if (InterlockedCompareExchangePointer(  // if already running
                &ContextTable[WmipKernelLogger], ContextTable, NULL) != NULL)
            return STATUS_OBJECT_NAME_COLLISION;

        LoggerId = WmipKernelLogger;
        DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;
    }
    else if (IsEqualGUID(&InstanceGuid, &GlobalLoggerGuid)) {
        LoggerId = WMI_GLOBAL_LOGGER_ID;
        if (InterlockedCompareExchangePointer(  // if already running
                &ContextTable[LoggerId], ContextTable, NULL) != NULL)
            return STATUS_OBJECT_NAME_COLLISION;
        if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            ULONG lFlag;
            //
            // To avoid alignment woes, copy the flags 
            //
            RtlCopyMemory(&lFlag, ((PCHAR)LoggerInfo + FlagExt->Offset), sizeof(ULONG));
            if (lFlag != 0) {
                EnableKernel = TRUE;
                IsGlobalForKernel = TRUE;
                WmipKernelLogger = LoggerId;
            }
        }
        // everyone has access to send to this
    }
    else {   // other loggers requested
        for (LoggerId = 2; LoggerId < MAXLOGGERS; LoggerId++) {
            if ( InterlockedCompareExchangePointer(
                    &ContextTable[LoggerId],
                    ContextTable,
                    NULL ) == NULL )
                break;      // mark the slot as busy by putting in ServiceInfo
        }

        if (LoggerId >=  MAXLOGGERS) {    // could not find any more slots
            return STATUS_UNSUCCESSFUL;
        }
    }
#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((1, "WmipStartLogger: %d %d->%d\n", LoggerId,
                    RefCount-1, RefCount));
    //
    // first, check to see if caller has access to proper Guids.
    //
    Status = WmipCheckGuidAccess(
                &InstanceGuid,
                DesiredAccess,
                EtwpDefaultTraceSecurityDescriptor
                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status1=%X %d %d->%d\n",
                    Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }

    // Next, try and see if we need to get the logfile object first
    //
    FileObject = NULL;
    if (LoggerInfo->LogFileHandle != NULL) {
        OBJECT_HANDLE_INFORMATION handleInformation;
        ACCESS_MASK grantedAccess;

        Status = ObReferenceObjectByHandle(
                    LoggerInfo->LogFileHandle,
                    0L,
                    IoFileObjectType,
                    RequestorMode,
                    (PVOID *) &FileObject,
                    &handleInformation);

        if (NT_SUCCESS(Status)) {
            TraceDebug((1, "WmipStartLogger: Referenced FDO %X %X %d\n",
                        FileObject, LoggerInfo->LogFileHandle,
                        ((POBJECT_HEADER)FileObject)->PointerCount));
            if (RequestorMode != KernelMode) {
                grantedAccess = handleInformation.GrantedAccess;
                if (!SeComputeGrantedAccesses(grantedAccess, FILE_WRITE_DATA)) {
                    TraceDebug((1, "WmipStartLogger: Deref FDO %x %d\n",
                                FileObject,
                                ((POBJECT_HEADER)FileObject)->PointerCount));
                    Status = STATUS_ACCESS_DENIED;
                }
            }
            ObDereferenceObject(FileObject);
        }

        if (!NT_SUCCESS(Status)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status2=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            return Status;
        }
    }

    LoggerContext = WmipInitContext();
    if (LoggerContext == NULL) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        Status = STATUS_NO_MEMORY;
        TraceDebug((1, "WmipStartLogger: Status5=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }
    WmipInitializeMutex(&LoggerContext->LoggerMutex);

    if (LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        LoggerContext->PoolType = PagedPool;
        LoggerContext->LoggerMode |= EVENT_TRACE_USE_PAGED_MEMORY;
    }
    else {
        LoggerContext->PoolType = NonPagedPool;
    }

    if (LogFileMode & EVENT_TRACE_KD_FILTER_MODE) {
        LoggerContext->LoggerMode |= EVENT_TRACE_KD_FILTER_MODE;
        LoggerContext->BufferCallback = &KdReportTraceData;
    }
    LoggerContext->InstanceGuid = InstanceGuid;
    // By now, the slot will be allocated properly

    LoggerContext->MaximumFileSize = LoggerInfo->MaximumFileSize;
    LoggerContext->BuffersWritten  = LoggerInfo->BuffersWritten;

    LoggerContext->LoggerMode |= LoggerInfo->LogFileMode & 0x0000FFFF;

    // For circular logging with persistent events.
    if (!EnableKernel && LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_CIRCULAR_PERSIST) {
        LoggerContext->RequestFlag |= REQUEST_FLAG_CIRCULAR_PERSIST;
    }

    // LoggerInfo->Wow is set by the kernel in IOCTL
    LoggerContext->Wow = LoggerInfo->Wow;

    WmipValidateClockType(LoggerInfo);

    LoggerContext->UsePerfClock = LoggerInfo->Wnode.ClientContext;

    if (LoggerInfo->FlushTimer > 0)
        LoggerContext->FlushTimer = LoggerInfo->FlushTimer;

    if (LoggerInfo->AgeLimit >= 0) { // minimum is 15 minutes
        LoggerContext->BufferAgeLimit.QuadPart
            = max (DEFAULT_AGE_LIMIT, LoggerInfo->AgeLimit)
                     * WmiOneSecond.QuadPart * 60;
    }
    else if (LoggerInfo->AgeLimit < 0) {
        LoggerContext->BufferAgeLimit.QuadPart = 0;
    }

    LoggerContext->LoggerId = LoggerId;
    LoggerContext->EnableFlags = EnableFlags;
    LoggerContext->KernelTraceOn = EnableKernel;
    LoggerContext->MaximumIrql = DEFAULT_MAX_IRQL;

    if (EnableKernel) {
        //
        // Always reserve space for FileTable to allow file trace
        // to be turn on/off dynamically
        //
        WmipFileTable
            = (PFILE_OBJECT*) WmipExtendBase(
                 LoggerContext, MAX_FILE_TABLE_SIZE * sizeof(PVOID));

        Status = (WmipFileTable == NULL) ? STATUS_NO_MEMORY : STATUS_SUCCESS;
        if (NT_SUCCESS(Status)) {
            if (! RtlCreateUnicodeString(
                    &LoggerContext->LoggerName, KERNEL_LOGGER_CAPTION)) {
                Status = STATUS_NO_MEMORY;
            }
        }

        if (!NT_SUCCESS(Status)) {
            ExFreePool(LoggerContext);      // free the partial context
#if DBG
        RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status6=%X %d %d->%d\n",
                           Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            return Status;
        }
    }

//
// Next, if user provided acceptable default buffer parameters, use them.
// Otherwise,  set them to predetermined default values.
//
    if (LoggerInfo->BufferSize > 0) {
        if (LoggerInfo->BufferSize > MAX_WMI_BUFFER_SIZE) {
            LoggerInfo->BufferSize = MAX_WMI_BUFFER_SIZE;
        }
        LoggerContext->BufferSize = LoggerInfo->BufferSize * 1024;
    }

    LoggerInfo->BufferSize = LoggerContext->BufferSize / 1024;
    if (LoggerInfo->MaximumBuffers >= 2) {
        LoggerContext->MaximumBuffers = LoggerInfo->MaximumBuffers;
    }

    if (LoggerInfo->MinimumBuffers >= 2 &&
        LoggerInfo->MinimumBuffers <= LoggerContext->MaximumBuffers) {
        LoggerContext->MinimumBuffers = LoggerInfo->MinimumBuffers;
    }

    RtlInitUnicodeString(&FileName, NULL);
    if (LoggerName.Buffer != NULL) {
        if (LoggerContext->KernelTraceOn) {
            RtlFreeUnicodeString(&LoggerName);
            LoggerName.Buffer = NULL;
        }
        else {
            RtlInitUnicodeString(&LoggerContext->LoggerName, LoggerName.Buffer);
        }
    }

    try {
        if (LoggerInfo->Checksum != NULL) {
            ULONG SizeNeeded = sizeof(WNODE_HEADER)
                             + sizeof(TRACE_LOGFILE_HEADER);
            if (RequestorMode != KernelMode) {
                ProbeForRead(LoggerInfo->Checksum, SizeNeeded, sizeof(UCHAR));
            }
            LoggerContext->LoggerHeader =
                    ExAllocatePoolWithTag(PagedPool, SizeNeeded, TRACEPOOLTAG);
            if (LoggerContext->LoggerHeader != NULL) {
                RtlCopyMemory(LoggerContext->LoggerHeader,
                              LoggerInfo->Checksum,
                              SizeNeeded);
            }
        }
        if (LoggerContext->KernelTraceOn) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerContext->LoggerName.Length + sizeof(WCHAR),
                    sizeof (UCHAR) );
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LoggerName, &LoggerContext->LoggerName);
        }
        if (LoggerInfo->LogFileName.Length > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LogFileName.Buffer,
                    LoggerInfo->LogFileName.Length,
                    sizeof (UCHAR) );
            }
            if (! RtlCreateUnicodeString(
                    &FileName,
                    LoggerInfo->LogFileName.Buffer) ) {
                Status = STATUS_NO_MEMORY;
            }
        }

        //
        // Set up the Global mask for Perf traces
        //
        if (IsGlobalForKernel || IsKernelRealTimeNoFile) {
            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                GroupMaskSize = FlagExt->Length * sizeof(ULONG);
                if (GroupMaskSize < sizeof(PERFINFO_GROUPMASK)) {
                    GroupMaskSize = sizeof(PERFINFO_GROUPMASK);
                }
            } else {
                GroupMaskSize = sizeof(PERFINFO_GROUPMASK);
            }
    
            LoggerContext->EnableFlagArray = (PULONG) WmipExtendBase(LoggerContext, GroupMaskSize);
    
            if (LoggerContext->EnableFlagArray) {
                PCHAR FlagArray;

                RtlZeroMemory(LoggerContext->EnableFlagArray, GroupMaskSize);
                if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                    FlagArray = (PCHAR) (FlagExt->Offset + (PCHAR) LoggerInfo);
    
                    //
                    // Copy only the bytes actually supplied
                    //
                    RtlCopyMemory(LoggerContext->EnableFlagArray, FlagArray, FlagExt->Length * sizeof(ULONG));

                    LoggerContext->EnableFlags = LoggerContext->EnableFlagArray[0];
    
                } else {
                    LoggerContext->EnableFlagArray[0] = EnableFlags;
                }
    
                PerfGroupMasks = (PERFINFO_GROUPMASK *) &LoggerContext->EnableFlagArray[0];
            } else {
                Status = STATUS_NO_MEMORY;
            }
        } else {
            ASSERT((EnableFlags & EVENT_TRACE_FLAG_EXTENSION) ==0);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
    //
    // The context is partially set up by now, so have to clean up
    //
        if (LoggerContext->LoggerName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerContext->LoggerName);
        }
        if (FileName.Buffer != NULL) {
            RtlFreeUnicodeString(&FileName);
        }

        if (LoggerContext->LoggerHeader != NULL) {
            ExFreePool(LoggerContext->LoggerHeader);
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status7=EXCEPTION %d %d->%d\n",
                       LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        ExFreePool(LoggerContext);      // free the partial context
        return GetExceptionCode();
    }

    if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
        RtlInitUnicodeString(&LoggerContext->LogFilePattern, FileName.Buffer);
        Status = WmipGenerateFileName(
                    &LoggerContext->LogFilePattern,
                    &LoggerContext->FileCounter,
                    &LoggerContext->LogFileName);
        if (!NT_SUCCESS(Status)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status8=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
            return(Status);
        }
    }
    else {
        RtlInitUnicodeString(&LoggerContext->LogFileName, FileName.Buffer);
    }

    if (NT_SUCCESS(Status)) {
        // Obtain the security context here so we can use it
        // later to impersonate the user, which we will do
        // if we cannot access the file as SYSTEM.  This
        // usually occurs if the file is on a remote machine.
        //
        ServiceQos.Length  = sizeof(SECURITY_QUALITY_OF_SERVICE);
        ServiceQos.ImpersonationLevel = SecurityImpersonation;
        ServiceQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        ServiceQos.EffectiveOnly = TRUE;
        Status = SeCreateClientSecurity(
                    CONTAINING_RECORD(KeGetCurrentThread(), ETHREAD, Tcb),
                    &ServiceQos,
                    FALSE,
                    &LoggerContext->ClientSecurityContext);
    }
    if (!NT_SUCCESS(Status)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status8=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        if (LoggerContext != NULL) {
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
        }
        return(Status);
    }

    //
    // Now, allocate the buffer pool and associated buffers.
    // Note that buffer allocation routine will also set NumberOfBuffers and
    // MaximumBuffers.
    //

    InitializeSListHead (&LoggerContext->FreeList);
    InitializeSListHead (&LoggerContext->FlushList);
    InitializeSListHead (&LoggerContext->WaitList);
    InitializeSListHead (&LoggerContext->GlobalList);

    Status = WmipAllocateTraceBufferPool(LoggerContext);
    if (!NT_SUCCESS(Status)) {
        if (LoggerContext != NULL) {
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status9=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }

    //
    // From this point on, LoggerContext is a valid structure
    //
    LoggerInfo->NumberOfBuffers = (ULONG) LoggerContext->NumberOfBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->FreeBuffers     = (ULONG) LoggerContext->BuffersAvailable;
    LoggerInfo->EnableFlags     = LoggerContext->EnableFlags;
    LoggerInfo->AgeLimit        = (ULONG) (LoggerContext->BufferAgeLimit.QuadPart
                                    / WmiOneSecond.QuadPart / 60);
    LoggerInfo->BufferSize = LoggerContext->BufferSize / 1024;

    WmiSetLoggerId(LoggerId,
                (PTRACE_ENABLE_CONTEXT)&LoggerInfo->Wnode.HistoricalContext);

    if (LoggerContext->LoggerMode & EVENT_TRACE_USE_LOCAL_SEQUENCE)
        LoggerContext->SequencePtr = (PLONG) &LoggerContext->LocalSequence;
    else if (LoggerContext->LoggerMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE)
        LoggerContext->SequencePtr = (PLONG) &WmipGlobalSequence;

// Initialize synchronization event with logger
    KeInitializeEvent(
        &LoggerContext->LoggerEvent,
        NotificationEvent,
        FALSE
        );
    KeInitializeEvent(
        &LoggerContext->FlushEvent,
        NotificationEvent,
        FALSE
        );

//
// Close file handle here so that it can be opened by system thread
//
    if (LoggerInfo->LogFileHandle != NULL) {
        ZwClose(LoggerInfo->LogFileHandle);
        LoggerInfo->LogFileHandle = NULL;
    }

    //
    // User Mode call always gets APPEND mode
    // 
    LogFileMode = LoggerContext->LoggerMode;

    if (RequestorMode != KernelMode) {
        LoggerContext->LoggerMode |= EVENT_TRACE_FILE_MODE_APPEND;
    }

    //
    // Lock down the routines that need to be non-pageable
    //
    KeAcquireGuardedMutex(&WmipTraceGuardedMutex);
    if (++WmipLoggerCount == 1) {

        ASSERT(EtwpPageLockHandle);
        MmLockPageableSectionByHandle(EtwpPageLockHandle);
        WmipGlobalSequence = 0;
    }
    KeReleaseGuardedMutex(&WmipTraceGuardedMutex);

    //
    // Set the clock function and 
    // start up the logger as a system thread
    //
    if (NT_SUCCESS(Status)) {
        LoggerContext->WriteFailureLimit = 100;
        switch (LoggerContext->UsePerfClock) {
            case EVENT_TRACE_CLOCK_CPUCYCLE:
                    if (EnableKernel) {
                        WmiGetCpuClock = &PerfGetCycleCount;
                    }
                    LoggerContext->GetCpuClock = &PerfGetCycleCount;
                    break;
            case EVENT_TRACE_CLOCK_PERFCOUNTER: 
                    if (EnableKernel) {
                        WmiGetCpuClock = &WmipGetPerfCounter; 
                    }
                    LoggerContext->GetCpuClock = &WmipGetPerfCounter;
                    break;
            case EVENT_TRACE_CLOCK_SYSTEMTIME: 
            default : 
                    if (EnableKernel) {
                        WmiGetCpuClock = &WmipGetSystemTime;
                    }
                    LoggerContext->GetCpuClock = &WmipGetSystemTime;
                    break;
        }

        //
        // At this point, the clock type should be set and we take a
        // reference timesamp, which should be the earliest timestamp 
        // for the logger.  The order is this way since SystemTime
        // is typically cheaper to obtain. 
        // 

        LoggerContext->ReferenceTimeStamp.QuadPart = (*LoggerContext->GetCpuClock)();
        KeQuerySystemTime(&LoggerContext->ReferenceSystemTime);

        //
        // Start up the logger as a system thread
        //
        LoggerContext->LoggerStatus = STATUS_UNSUCCESSFUL;

        WmipReferenceLogger(LoggerId);

        Status = PsCreateSystemThread(
                    &ThreadHandle,
                    THREAD_ALL_ACCESS,
                    NULL,
                    NULL,
                    NULL,
                    WmipLogger,
                    LoggerContext );

        if (NT_SUCCESS(Status)) {  // if SystemThread is started
            ZwClose (ThreadHandle);

            Status = STATUS_UNSUCCESSFUL;
            do {
                //
                // Wait for Logger to start up properly before proceeding
                //
                KeWaitForSingleObject(
                            &LoggerContext->LoggerEvent,
                            Executive,
                            KernelMode,
                            FALSE,
                            &TimeOut
                            );

                KeResetEvent(&LoggerContext->LoggerEvent);

                if (Status == STATUS_UNSUCCESSFUL) {
                    //
                    // If the logger thread hasn't responded yet, replace the LoggerStatus
                    // to STATUS_CANCELLED so that if it ever comes up, it will terminate.
                    // We clean up the logger cointext if that is the case.
                    //
                    Status = InterlockedCompareExchange(&LoggerContext->LoggerStatus,
                                            STATUS_CANCELLED,
                                            STATUS_UNSUCCESSFUL);
                }
                else {
                    Status = LoggerContext->LoggerStatus;
                }
            }
            while (Status == STATUS_PENDING); 

            //
            // If the logger is up and running properly, we can now turn on
            // event tracing if kernel tracing is requested
            //
            if (NT_SUCCESS(Status)) {
                LoggerContext->LoggerMode = LogFileMode;

                //
                // After we release this mutex, any other thread can acquire
                // the valid logger context and call the shutdown path for 
                // this logger.  Until this, no other thread can call the enable
                // or disable code for this logger.
                //
                WmipAcquireMutex( &LoggerContext->LoggerMutex );
                InterlockedIncrement(&LoggerContext->MutexCount);

                LoggerInfo->BuffersWritten = LoggerContext->BuffersWritten;

                WmipLoggerContext[LoggerId] = LoggerContext;
                TraceDebug((1, "WmipStartLogger: Started %X %d\n",
                            LoggerContext, LoggerContext->LoggerId));
                if (LoggerContext->KernelTraceOn) {
                    EnableFlags = LoggerContext->EnableFlags;
                    if (EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO)
                        EnableFlags |= EVENT_TRACE_FLAG_DISK_IO;
                    WmipEnableKernelTrace(EnableFlags);
                }

                if (IsEqualGUID(&InstanceGuid, &WmiEventLoggerGuid)) {
                    WmipEventLogger = LoggerId;
                    EnableFlags = EVENT_TRACE_FLAG_PROCESS |
                                  EVENT_TRACE_FLAG_THREAD |
                                  EVENT_TRACE_FLAG_IMAGE_LOAD;
                    WmipEnableKernelTrace(EnableFlags);
                    LoggerContext->EnableFlags = EnableFlags;
                }

                if (LoggerContext->LoggerThread) {
                    LoggerInfo->LoggerThreadId
                        = LoggerContext->LoggerThread->Cid.UniqueThread;
                }

                //
                // Logger is started properly, now turn on perf trace
                //
                if (IsGlobalForKernel) {
                    ASSERT(LoggerContext->KernelTraceOn);
                    ASSERT(EnableKernel);
                    Status = PerfInfoStartLog(PerfGroupMasks, 
                                              PERFINFO_START_LOG_FROM_GLOBAL_LOGGER);
                    if (!NT_SUCCESS(Status)) {
                        //
                        // Turning on tracing failed, needs to clean up.
                        // Logger Thread has been created at this point.
                        // Just do WmipStopLoggerInstance and let Logger Thread
                        // handle all the cleanup work.
                        //
                        LoggerContext->LoggerStatus = Status;
                        WmipStopLoggerInstance(LoggerContext);
                    }
                } else if (IsKernelRealTimeNoFile) {
                    //
                    // We need to protect PerfInfoStartLog from stopping thread.
                    //
                    LONG PerfLogInTransition;

                    ASSERT(LoggerContext->KernelTraceOn);
                    ASSERT(EnableKernel);
                    PerfLogInTransition = InterlockedCompareExchange(
                                                &LoggerContext->PerfLogInTransition,
                                                PERF_LOG_START_TRANSITION,
                                                PERF_LOG_NO_TRANSITION);

                    if (PerfLogInTransition != PERF_LOG_NO_TRANSITION) {
                        Status = STATUS_ALREADY_DISCONNECTED;
                        LoggerContext->LoggerStatus = Status;
                    } else {
                        Status = PerfInfoStartLog(PerfGroupMasks, 
                                                  PERFINFO_START_LOG_POST_BOOT);
                        PerfLogInTransition =
                                InterlockedExchange(&LoggerContext->PerfLogInTransition,
                                                    PERF_LOG_NO_TRANSITION);
                        ASSERT(PerfLogInTransition == PERF_LOG_START_TRANSITION);
                        if (!NT_SUCCESS(Status)) {
                            //
                            // Turning on tracing failed, needs to clean up.
                            // Logger Thread has been created at this point.
                            // Just do WmipStopLoggerInstance and let Logger Thread
                            // handle all the cleanup work.
                            //
                            LoggerContext->LoggerStatus = Status;
                            WmipStopLoggerInstance(LoggerContext);
                        }
                    }
                }

                InterlockedDecrement(&LoggerContext->MutexCount);
                WmipReleaseMutex(&LoggerContext->LoggerMutex);

                WmipDereferenceLogger(LoggerId);
                // LoggerContext refcount is now >= 1 until it is stopped
                return Status;
            }
            else {
                //
                // The logger thread did not notify the starting thread, or
                // the logger thread started OK, but something failed during
                // file creation. The logger thread will clean up the logger
                // context. Just return.
                // 

                WmipDereferenceLogger(LoggerId);
                return Status;
            }
        }
        WmipDereferenceLogger(LoggerId);

    }
    TraceDebug((2, "WmipStartLogger: %d %X failed with status=%X ref %d\n",
                    LoggerId, LoggerContext, Status, WmipRefCount[LoggerId]));
    //
    // Will get here if Status has failed earlier.
    //
    if (LoggerContext != NULL) { // should not be NULL
        WmipFreeLoggerContext(LoggerContext);
    }
    else {
        WmipDereferenceLogger(LoggerId);
        ContextTable[LoggerId] = NULL;
    }
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
WmipQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
/*++

Routine Description:

    This routine is called to control the data collection and logger.
    It is called by WmipIoControl in wmi.c, with IOCTL_WMI_QUERY_LOGGER.
    Caller must pass in either the Logger Name or a valid Logger Id/Handle.

Arguments:

    LoggerInfo     a pointer to the structure for the logger's control
                    and status information

    LoggerContext  if this is provided, it assumes it is a valid one

Return Value:

    The status of performing the action requested.

--*/

{
    NTSTATUS            Status;
    ULONG               LoggerId, NoContext;
    ACCESS_MASK         DesiredAccess = WMIGUID_QUERY;
    KPROCESSOR_MODE     RequestorMode;
#if DBG
    LONG                RefCount;
#endif

    PAGED_CODE();

    NoContext = (LoggerContext == NULL);
    if (NoContext) {
    
if ((LoggerInfo->Wnode.HistoricalContext == 0XFFFF) || (LoggerInfo->Wnode.HistoricalContext < 1))
        TraceDebug((2, "WmipQueryLogger: %d\n",
                        LoggerInfo->Wnode.HistoricalContext));
#if DBG
        Status = WmipVerifyLoggerInfo(
                    LoggerInfo, &LoggerContext, "WmipQueryLogger");
#else
        Status = WmipVerifyLoggerInfo( LoggerInfo, &LoggerContext );
#endif

        if (!NT_SUCCESS(Status) || (LoggerContext == NULL))
            return Status;        // cannot find by name nor logger id

        LoggerInfo->Wnode.Flags = 0;
        LoggerInfo->EnableFlags = 0;
        LoggerId = (ULONG) LoggerContext->LoggerId;

        if (LoggerContext->KernelTraceOn) {
            DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;
        }

        Status = WmipCheckGuidAccess(
                    &LoggerContext->InstanceGuid,
                    DesiredAccess,
                    EtwpDefaultTraceSecurityDescriptor
                    );
        if (!NT_SUCCESS(Status)) {
            InterlockedDecrement(&LoggerContext->MutexCount);
            TraceDebug((1, "WmipQueryLogger: Release mutex1 %d %d\n",
                LoggerId, LoggerContext->MutexCount));
            WmipReleaseMutex(&LoggerContext->LoggerMutex);
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipQueryLogger: Status1=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            return Status;
        }
    }
    else {
        LoggerId = LoggerContext->LoggerId;
    }

    if (LoggerContext->KernelTraceOn) {
        LoggerInfo->Wnode.Guid = SystemTraceControlGuid;
        LoggerInfo->EnableFlags = LoggerContext->EnableFlags;
    }
    else
        LoggerInfo->Wnode.Guid = LoggerContext->InstanceGuid;

    LoggerInfo->LogFileMode     = LoggerContext->LoggerMode;
    LoggerInfo->MaximumFileSize = LoggerContext->MaximumFileSize;
    LoggerInfo->FlushTimer      = LoggerContext->FlushTimer;

    LoggerInfo->BufferSize      = LoggerContext->BufferSize / 1024;
    LoggerInfo->NumberOfBuffers = (ULONG) LoggerContext->NumberOfBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->EventsLost      = LoggerContext->EventsLost;
    LoggerInfo->FreeBuffers     = (ULONG) LoggerContext->BuffersAvailable;
    LoggerInfo->BuffersWritten  = LoggerContext->BuffersWritten;
    LoggerInfo->Wow             = LoggerContext->Wow;
    LoggerInfo->LogBuffersLost  = LoggerContext->LogBuffersLost;
    LoggerInfo->RealTimeBuffersLost = LoggerContext->RealTimeBuffersLost;
    LoggerInfo->AgeLimit        = (ULONG)
                                  (LoggerContext->BufferAgeLimit.QuadPart
                                    / WmiOneSecond.QuadPart / 60);
    WmiSetLoggerId(LoggerId,
                (PTRACE_ENABLE_CONTEXT)&LoggerInfo->Wnode.HistoricalContext);

    if (LoggerContext->LoggerThread) {
        LoggerInfo->LoggerThreadId
            = LoggerContext->LoggerThread->Cid.UniqueThread;
    }

    LoggerInfo->Wnode.ClientContext = LoggerContext->UsePerfClock;

//
// Return LogFileName and Logger Caption here
//
    RequestorMode = KeGetPreviousMode();
    try {
        if (LoggerContext->LogFileName.Length > 0 &&
            LoggerInfo->LogFileName.MaximumLength > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LogFileName.Buffer,
                    LoggerContext->LogFileName.Length + sizeof(WCHAR),
                    sizeof (UCHAR) );
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LogFileName,
                &LoggerContext->LogFileName);
        }
        if (LoggerContext->LoggerName.Length > 0 &&
            LoggerInfo->LoggerName.MaximumLength > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerContext->LoggerName.Length + sizeof(WCHAR),
                    sizeof(UCHAR));
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LoggerName,
                &LoggerContext->LoggerName);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (NoContext) {
            InterlockedDecrement(&LoggerContext->MutexCount);
            TraceDebug((1, "WmipQueryLogger: Release mutex3 %d %d\n",
                LoggerId, LoggerContext->MutexCount));
            WmipReleaseMutex(&LoggerContext->LoggerMutex);
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipQueryLogger: Status3=EXCEPTION %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
        }
        return GetExceptionCode();
    }

    if (NoContext) {
        InterlockedDecrement(&LoggerContext->MutexCount);
        TraceDebug((1, "WmipQueryLogger: Release mutex %d %d\n",
            LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipQueryLogger: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
    return STATUS_SUCCESS;
}


NTSTATUS
WmipStopLoggerInstance(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    LONG               LoggerOn;

    PAGED_CODE();
    if (LoggerContext == NULL) {    // just in case
        return STATUS_INVALID_HANDLE;
    }

    if (LoggerContext->KernelTraceOn) {
        // PerfInfoStopLog should not be executed when perf logging is starting
        // or stopping by other thread. PerfLogInTransition flag in the logger
        // context should only be used here and UpdateTrace and NO WHERE ELSE.
        LONG PerfLogInTransition = 
            InterlockedCompareExchange(&LoggerContext->PerfLogInTransition,
                                    PERF_LOG_STOP_TRANSITION,
                                    PERF_LOG_NO_TRANSITION);
        if (PerfLogInTransition == PERF_LOG_START_TRANSITION) {
            // This is the logger thread, and it is terminating. 
            // UpdateTrace call is enabling perf logging at the moment. 
            // Come back later.
            return STATUS_UNSUCCESSFUL;
        }
        else if (PerfLogInTransition == PERF_LOG_STOP_TRANSITION) {
            return STATUS_ALREADY_DISCONNECTED;
        }
        //
        // Time to turn off trace in perf tools
        //
        PerfInfoStopLog();
    }

    //
    // turn off data tracing first
    //
    LoggerOn = InterlockedExchange(&LoggerContext->CollectionOn, FALSE);
    if (LoggerOn == FALSE) {
        // This happens if another stoplogger already in progress
        return STATUS_ALREADY_DISCONNECTED;
    }
    if (LoggerContext->KernelTraceOn) {
        //
        // Turn off everything, just to be on the safe side
        // NOTE: If we start sharing callouts, the argument should be
        // LoggerContext->EnableFlags
        //
        WmipDisableKernelTrace(LoggerContext->EnableFlags);
    }
    if (LoggerContext->LoggerId == WmipEventLogger) {
        WmipDisableKernelTrace(EVENT_TRACE_FLAG_PROCESS |
                               EVENT_TRACE_FLAG_THREAD |
                               EVENT_TRACE_FLAG_IMAGE_LOAD);
        WmipEventLogger = 0xFFFFFFFF;
    }

    //
    // Mark the table entry as in-transition
    // From here on, the stop operation will not fail
    //
    WmipLoggerContext[LoggerContext->LoggerId] = (PWMI_LOGGER_CONTEXT)
                                                 &WmipLoggerContext[0];

    WmipNotifyLogger(LoggerContext);

    WmipSendNotification(LoggerContext, STATUS_THREAD_IS_TERMINATING, 0);
    return STATUS_SUCCESS;
}


NTSTATUS
WmipVerifyLoggerInfo(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
#if DBG
    OUT PWMI_LOGGER_CONTEXT *pLoggerContext,
    IN  LPSTR Caller
#else
    OUT PWMI_LOGGER_CONTEXT *pLoggerContext
#endif
    )
{
    NTSTATUS Status = STATUS_SEVERITY_ERROR;
    ULONG LoggerId;
    UNICODE_STRING LoggerName;
    KPROCESSOR_MODE     RequestorMode;
    PWMI_LOGGER_CONTEXT LoggerContext, CurrentContext;
    LONG            MutexCount = 0;
#if DBG
    LONG            RefCount;
#endif

    PAGED_CODE();

    *pLoggerContext = NULL;

    if (LoggerInfo == NULL)
        return STATUS_SEVERITY_ERROR;

    //
    // try and check for bogus parameter
    // if the size is at least what we want, we have to assume it's valid
    //

    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return STATUS_INVALID_BUFFER_SIZE;

    if (! (LoggerInfo->Wnode.Flags & WNODE_FLAG_TRACED_GUID) )
        return STATUS_INVALID_PARAMETER;

    RtlInitUnicodeString(&LoggerName, NULL);

    RequestorMode = KeGetPreviousMode();
    try {
        if (LoggerInfo->LoggerName.Length > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length,
                    sizeof (UCHAR) );
            }
            RtlCreateUnicodeString(
                &LoggerName,
                LoggerInfo->LoggerName.Buffer);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (LoggerName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerName);
        }
        return GetExceptionCode();
    }
    Status = STATUS_SUCCESS;
    if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid)) {
        LoggerId = WmipKernelLogger;
    }
    else if (LoggerName.Length > 0) { // Logger Name is passed
        Status =  WmipLookupLoggerIdByName(&LoggerName, &LoggerId);
    }
    else {
        LoggerId = WmiGetLoggerId(LoggerInfo->Wnode.HistoricalContext);
        if (LoggerId == KERNEL_LOGGER_ID) {
            LoggerId = WmipKernelLogger;
        }
        else if (LoggerId < 1 || LoggerId >= MAXLOGGERS) {
            Status  = STATUS_INVALID_HANDLE;
        }
    }
    if (LoggerName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerName);
    }
    if (!NT_SUCCESS(Status)) { // cannot find by name nor logger id
        return Status;
    }

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((2, "WmipVerifyLoggerInfo(%s): %d %d->%d\n",
                    Caller, LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((2, "WmipVerifyLoggerInfo(%s): Status=%X %d %d->%d\n",
                        Caller, STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }

    InterlockedIncrement(&LoggerContext->MutexCount);
    TraceDebug((1, "WmipVerifyLoggerInfo: Acquiring mutex... %d %d\n",
                    LoggerId, LoggerContext->MutexCount));
    WmipAcquireMutex (&LoggerContext->LoggerMutex);
    TraceDebug((1, "WmipVerifyLoggerInfo: Acquired mutex %d %d %X\n",
                    LoggerId, LoggerContext->MutexCount, LoggerContext));

    // Need to check for validity of LoggerContext in mutex
    CurrentContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(CurrentContext) ||
        !LoggerContext->CollectionOn ) {
        TraceDebug((1, "WmipVerifyLoggerInfo: Released mutex %d %d\n",
            LoggerId, LoggerContext->MutexCount-1));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
        MutexCount = InterlockedDecrement(&LoggerContext->MutexCount);
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((2, "WmipVerifyLoggerInfo(%s): Status2=%X %d %d->%d\n",
                        Caller, STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));

        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }
    *pLoggerContext = LoggerContext;
    return STATUS_SUCCESS;
}

PVOID
WmipExtendBase(
    IN PWMI_LOGGER_CONTEXT Base,
    IN ULONG Size
    )
{
//
// This private routine only return space from the Base by extending its
// offset. It does not actually try and allocate memory from the system
//
// It rounds the size to a ULONGLONG alignment and expects EndPageMarker
// to already be aligned.
//
    PVOID Space = NULL;
    ULONG SpaceLeft;

    PAGED_CODE();

    ASSERT(((ULONGLONG) Base->EndPageMarker % sizeof(ULONGLONG)) == 0);

    //
    // Round up to pointer boundary
    //
#ifdef _WIN64
    Size = ALIGN_TO_POWER2(Size, 16);
#else
    Size = ALIGN_TO_POWER2(Size, DEFAULT_TRACE_ALIGNMENT);
#endif

    SpaceLeft = CONTEXT_SIZE - (ULONG) (Base->EndPageMarker - (PUCHAR)Base);

    if ( SpaceLeft > Size ) {
        Space = Base->EndPageMarker;
        Base->EndPageMarker += Size;
    }

    return Space;
}

VOID
WmipFreeLoggerContext(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG LoggerId;
    LONG  RefCount;
    LARGE_INTEGER Timeout = {(ULONG)(-50 * 1000 * 10), -1}; // 50 ms
    NTSTATUS Status = STATUS_TIMEOUT;

    PAGED_CODE();

    if (LoggerContext == NULL)
        return;             // should not happen

    if (LoggerContext->LoggerHeader != NULL) {
        ExFreePool(LoggerContext->LoggerHeader);
    }

    LoggerId = LoggerContext->LoggerId;

    //
    // The RefCount must be at least 2 at this point.
    // One was set by WmipStartLogger() in the beginning, and the
    // second must be done normally by WmiStopTrace() or anybody who
    // needs to call this routine to free the logger context
    //
    //  RefCount = WmipDereferenceLogger(LoggerId);

    KeResetEvent(&LoggerContext->LoggerEvent);
    RefCount = WmipRefCount[LoggerId];
    WmipAssert(RefCount >= 1);
    TraceDebug((1, "WmipFreeLoggerContext: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    while (RefCount > 1) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->LoggerEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    &Timeout);
        KeResetEvent(&LoggerContext->LoggerEvent);
        KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);     // Just to be sure

        if (LoggerContext->MutexCount >= 1) {
            KeResetEvent(&LoggerContext->LoggerEvent);
            Status = STATUS_TIMEOUT;
            continue;
        }

        if (WmipRefCount[LoggerId] <= 1)
            break;
        RefCount = WmipRefCount[LoggerId];
    }

    KeAcquireGuardedMutex(&WmipTraceGuardedMutex);
    if (--WmipLoggerCount == 0) {
        if (EtwpPageLockHandle) {
            MmUnlockPageableImageSection(EtwpPageLockHandle);
        }
#if DBG
        else {
            ASSERT(EtwpPageLockHandle);
        }
#endif
    }
    KeReleaseGuardedMutex(&WmipTraceGuardedMutex);

    WmipFreeTraceBufferPool(LoggerContext);

    if (LoggerContext->LoggerName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LoggerName);
    }
    if (LoggerContext->LogFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LogFileName);
    }
    if (LoggerContext->LogFilePattern.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
    }
    if (LoggerContext->NewLogFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->NewLogFileName);
    }
#if DBG
        RefCount =
#endif
    //
    // Finally, decrement the refcount incremented by WmipStartLogger()
    //
    WmipDereferenceLogger(LoggerId);

#if DBG
    TraceDebug((2, "WmipFreeLoggerContext: Freeing pool %X %d %d->%d\n",
                    LoggerContext, LoggerId, RefCount+1, RefCount));
    if (LoggerContext->CollectionOn) {
        TraceDebug((1,
            "WmipFreeLoggerContext: %X %d still active\n", LoggerContext,
            LoggerId));
    }

    if (LoggerContext->MutexCount >= 1) {
        TraceDebug((0, "****ERROR**** Mutex count is %d for %d\n", LoggerId,
            LoggerContext->MutexCount));
    }
#endif // DBG
    ExFreePool(LoggerContext);
    WmipLoggerContext[LoggerId] = NULL;
}


PWMI_LOGGER_CONTEXT
WmipInitContext(
    )

/*++

Routine Description:

    This routine is called to initialize the context of LoggerContext

Arguments:

    None

Returned Value:

    Status of STATUS_SUCCESS if the allocation was successful

--*/

{
    PWMI_LOGGER_CONTEXT LoggerContext;

    PAGED_CODE();

    LoggerContext = (PWMI_LOGGER_CONTEXT)
                    ExAllocatePoolWithTag(NonPagedPool,
                         CONTEXT_SIZE, TRACEPOOLTAG);

// One page is reserved to store the buffer pool pointers plus anything
// else that we need. Should experiment a little more to reduce it further

    if (LoggerContext == NULL) {
        return NULL;
    }

    RtlZeroMemory(LoggerContext, CONTEXT_SIZE);

    LoggerContext->EndPageMarker =
        (PUCHAR) LoggerContext + 
                 ALIGN_TO_POWER2(sizeof(WMI_LOGGER_CONTEXT), DEFAULT_TRACE_ALIGNMENT);

    LoggerContext->BufferSize     = PAGE_SIZE;
    LoggerContext->MinimumBuffers = (ULONG)KeNumberProcessors + DEFAULT_BUFFERS;
    // 20 additional buffers for MaximumBuffers
    LoggerContext->MaximumBuffers
       = LoggerContext->MinimumBuffers + DEFAULT_BUFFERS + 20;

    KeQuerySystemTime(&LoggerContext->StartTime);

    KeInitializeSemaphore( &LoggerContext->LoggerSemaphore,
                           0,
                           SEMAPHORE_LIMIT  );

    KeInitializeSpinLock(&LoggerContext->BufferSpinLock);

    return LoggerContext;
}


NTSTATUS
WmipAllocateTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )

/*++

Routine Description:
    This routine is used to set up the circular trace buffers

Arguments:

    LoggerContext       Context of the logger to own the buffers.

Returned Value:

    STATUS_SUCCESS if the initialization is successful

--*/

{
    ULONG NumberProcessors, SysMax_Buffers, SysMin_Buffers;
    LONG i;
    PWMI_BUFFER_HEADER Buffer;
    ULONG AllocatedBuffers, NumberOfBuffers;
    SIZE_T WmiMaximumPoolInBytes;

    PAGED_CODE();
//
// Allocate the pointers the each buffer here by sharing the same page
//    with LoggerContext context pointer
//
    NumberProcessors = (ULONG) KeNumberProcessors;

    // This does not keep track of the amount already used by other sessions
    if (LoggerContext->LoggerMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        WmiMaximumPoolInBytes = MmSizeOfPagedPoolInBytes;
    }
    else {
        WmiMaximumPoolInBytes = MmMaximumNonPagedPoolInBytes;
    }

    // Compute System limits for min and max

    // This is the absolute maximum that ANYONE can use
    SysMax_Buffers = (ULONG) (WmiMaximumPoolInBytes
                            / TRACE_MAXIMUM_NP_POOL_USAGE
                            / LoggerContext->BufferSize);

    // This is the absolute minimum that ANYONE MUST have
    SysMin_Buffers = NumberProcessors + DEFAULT_BUFFERS;

    // Sanity check to ensure that we have at least the minimum available
    if (SysMin_Buffers > SysMax_Buffers) {
        return STATUS_NO_MEMORY;
    }


    // Cover the case if the caller did not specify any values
    LoggerContext->MaximumBuffers = max(LoggerContext->MaximumBuffers,
                                    NumberProcessors + DEFAULT_BUFFERS +
                                    DEFAULT_MAX_BUFFERS);

    LoggerContext->MinimumBuffers = max(LoggerContext->MinimumBuffers,
                                        SysMin_Buffers);


    // Ensure each parameter is in range of SysMin and SysMax

    LoggerContext->MaximumBuffers = max (LoggerContext->MaximumBuffers, 
                                         SysMin_Buffers);
    LoggerContext->MaximumBuffers = min (LoggerContext->MaximumBuffers, 
                                         SysMax_Buffers);

    LoggerContext->MinimumBuffers = max (LoggerContext->MinimumBuffers, 
                                         SysMin_Buffers);
    LoggerContext->MinimumBuffers = min (LoggerContext->MinimumBuffers, 
                                         SysMax_Buffers);

    // In case the MaximumBuffers and MinimumBuffers got reversed pick the 
    // larger value

    if (LoggerContext->MinimumBuffers > LoggerContext->MaximumBuffers) {
        LoggerContext->MaximumBuffers = LoggerContext->MinimumBuffers;
    }

    // NOTE: We do not return anything if we reset MaximumBuffers or MinimumBuffers
    // provided by the caller.

    LoggerContext->NumberOfBuffers = (LONG) LoggerContext->MinimumBuffers;
    LoggerContext->BuffersAvailable = LoggerContext->NumberOfBuffers;

    //
    // Allocate the buffers now
    //
    //
    // Now determine the initial number of buffers
    //
    NumberOfBuffers = LoggerContext->NumberOfBuffers;
    LoggerContext->NumberOfBuffers = 0;
    LoggerContext->BuffersAvailable = 0;

    AllocatedBuffers = WmipAllocateFreeBuffers(LoggerContext,
                                              NumberOfBuffers);

    if (AllocatedBuffers < NumberOfBuffers) {
        //
        // No enough buffer is allocated.
        //
        WmipFreeTraceBufferPool(LoggerContext);
        return STATUS_NO_MEMORY;
    }

//
// Allocate Per Processor Buffer pointers
//

    LoggerContext->ProcessorBuffers
        = (PWMI_BUFFER_HEADER *)
          WmipExtendBase(LoggerContext,
                         sizeof(PWMI_BUFFER_HEADER)*NumberProcessors);


    if (LoggerContext->ProcessorBuffers == NULL) {
        WmipFreeTraceBufferPool(LoggerContext);
        return STATUS_NO_MEMORY;
    }

    //
    // NOTE: We already know that we have allocated > number of processors
    // buffers
    //
    for (i=0; i<(LONG)NumberProcessors; i++) {
        Buffer = (PWMI_BUFFER_HEADER) WmipGetFreeBuffer(LoggerContext);
        LoggerContext->ProcessorBuffers[i] = Buffer;
        Buffer->ClientContext.ProcessorNumber = (UCHAR)i;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
WmipFreeTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG i;
    PSLIST_ENTRY Entry;
    PWMI_BUFFER_HEADER* ProcessorBuffers;
    PWMI_BUFFER_HEADER Buffer;

    PAGED_CODE();

    TraceDebug((2, "Free Buffer Pool: %2d, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                    LoggerContext->LoggerId,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers));

    while (Entry = InterlockedPopEntrySList(&LoggerContext->FreeList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersAvailable);

        TraceDebug((2, "WmipFreeTraceBufferPool (Free): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

        WMI_FREE_TRACE_BUFFER(Buffer);
    }

    while (Entry = InterlockedPopEntrySList(&LoggerContext->FlushList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersDirty);

        TraceDebug((2, "WmipFreeTraceBufferPool (Flush): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

         WMI_FREE_TRACE_BUFFER(Buffer);
    }

    while (Entry = InterlockedPopEntrySList(&LoggerContext->WaitList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersDirty);

        TraceDebug((2, "WmipFreeTraceBufferPool (Wait): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

         WMI_FREE_TRACE_BUFFER(Buffer);
    }

    ProcessorBuffers = LoggerContext->ProcessorBuffers;
    if (ProcessorBuffers != NULL) {
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            Buffer = InterlockedExchangePointer(&ProcessorBuffers[i], NULL);
            if (Buffer) {
                InterlockedDecrement(&LoggerContext->NumberOfBuffers);
                InterlockedDecrement(&LoggerContext->BuffersInUse);

                TraceDebug((2, "WmipFreeTraceBufferPool (CPU %2d): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                                i,
                                LoggerContext->LoggerId,
                                Buffer,
                                LoggerContext->BuffersAvailable,
                                LoggerContext->BuffersInUse,
                                LoggerContext->BuffersDirty,
                                LoggerContext->NumberOfBuffers));

                WMI_FREE_TRACE_BUFFER(Buffer);
            }
        }
    }

    ASSERT(LoggerContext->BuffersAvailable == 0);
    ASSERT(LoggerContext->BuffersInUse == 0);
    ASSERT(LoggerContext->BuffersDirty == 0);
    ASSERT(LoggerContext->NumberOfBuffers == 0);

    return STATUS_SUCCESS;
}


NTSTATUS
WmipLookupLoggerIdByName(
    IN PUNICODE_STRING Name,
    OUT PULONG LoggerId
    )
{
    ULONG i;
    PWMI_LOGGER_CONTEXT *ContextTable;

    PAGED_CODE();
    if (Name == NULL) {
        *LoggerId = (ULONG) -1;
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }
    ContextTable = (PWMI_LOGGER_CONTEXT *) &WmipLoggerContext[0];
    for (i=0; i<MAXLOGGERS; i++) {
        if (ContextTable[i] == NULL ||
            ContextTable[i] == (PWMI_LOGGER_CONTEXT) ContextTable)
            continue;
        if (RtlEqualUnicodeString(&ContextTable[i]->LoggerName, Name, TRUE) ) {
            *LoggerId = i;
            return STATUS_SUCCESS;
        }
    }
    *LoggerId = (ULONG) -1;
    return STATUS_WMI_INSTANCE_NOT_FOUND;
}

NTSTATUS
WmipShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
//
// Shutdown all loggers cleanly. If a logger is in transition, it may
// not be stopped properly.
//
{
    ULONG LoggerCount;
    USHORT i;
    PWMI_LOGGER_CONTEXT LoggerContext;
    WMI_LOGGER_INFORMATION LoggerInfo;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    PAGED_CODE();

    TraceDebug((2, "WmipShutdown called\n"));
    if (WmipLoggerCount > 0) {
        RtlZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
        LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
        LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

        LoggerCount = 0;
        for (i=0; i<MAXLOGGERS; i++) {
            LoggerContext = WmipLoggerContext[i];
            if ((LoggerContext != NULL) &&
                (LoggerContext != (PWMI_LOGGER_CONTEXT)&WmipLoggerContext[0])) {
                WmiSetLoggerId(i, &LoggerInfo.Wnode.HistoricalContext);
                LoggerInfo.Wnode.Guid = LoggerContext->InstanceGuid;
                WmiStopTrace(&LoggerInfo);
                if (++LoggerCount == WmipLoggerCount)
                    break;
            }
#if DBG
            else if (LoggerContext
                        == (PWMI_LOGGER_CONTEXT)&WmipLoggerContext[0]) {
                TraceDebug((4, "WmipShutdown: Logger %d in transition\n", i));
            }
#endif
        }
    }
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
WmipFlushLogger(
    IN OUT PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG Wait
    )
{
    LARGE_INTEGER TimeOut = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
    NTSTATUS Status;

    PAGED_CODE();

    KeResetEvent(&LoggerContext->FlushEvent);

    LoggerContext->RequestFlag |= REQUEST_FLAG_FLUSH_BUFFERS;
    Status = WmipNotifyLogger(LoggerContext);
    if (!NT_SUCCESS(Status))
        return Status;
    if (Wait) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->FlushEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    & TimeOut
                    );
#if DBG
        if (Status == STATUS_TIMEOUT) {
            TraceDebug((1, "WmiFlushLogger: Wait status=%X\n",Status));
        }
#endif 
        KeResetEvent(&LoggerContext->FlushEvent);
        Status = LoggerContext->LoggerStatus;
    }
    return Status;
}

NTSTATUS
FASTCALL
WmipNotifyLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
// Routine can be called at DISPATCH_LEVEL
{
    LONG SemCount = KeReadStateSemaphore(&LoggerContext->LoggerSemaphore);
    if (SemCount >= SEMAPHORE_LIMIT/2) {
        return STATUS_SEMAPHORE_LIMIT_EXCEEDED;
    }
    {
        KeReleaseSemaphore(&LoggerContext->LoggerSemaphore, 0, 1, FALSE);
        return STATUS_SUCCESS;
    }
}


NTSTATUS
WmipNtDllLoggerInfo(
    IN OUT PWMINTDLLLOGGERINFO Buffer
    )
{

    NTSTATUS            Status = STATUS_SUCCESS;

    KPROCESSOR_MODE     RequestorMode;
    PBGUIDENTRY         GuidEntry;    
    ULONG               SizeNeeded;
    GUID                Guid;
    ACCESS_MASK         DesiredAccess = TRACELOG_GUID_ENABLE;

    PAGED_CODE();

    RequestorMode = KeGetPreviousMode();

    SizeNeeded = sizeof(WMI_LOGGER_INFORMATION);

    __try {

        if (RequestorMode != KernelMode){
            ProbeForRead(Buffer->LoggerInfo, SizeNeeded, sizeof(ULONGLONG));
        }

        RtlCopyMemory(&Guid, &Buffer->LoggerInfo->Wnode.Guid, sizeof(GUID));

        if(!IsEqualGUID(&Guid, &NtdllTraceGuid)){

            return STATUS_UNSUCCESSFUL;

        }

        SizeNeeded = Buffer->LoggerInfo->Wnode.BufferSize;

    }  __except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    WmipEnterTLCritSection();
    WmipEnterSMCritSection();

    GuidEntry = WmipFindGEByGuid(&Guid, FALSE);

    if(Buffer->IsGet){

        if( GuidEntry ){

            if(GuidEntry->LoggerInfo){

                SizeNeeded = GuidEntry->LoggerInfo->Wnode.BufferSize;

                __try {

                    if (RequestorMode != KernelMode){
                        ProbeForWrite(Buffer->LoggerInfo, SizeNeeded, sizeof(ULONGLONG));
                    }

                    RtlCopyMemory(Buffer->LoggerInfo,GuidEntry->LoggerInfo,SizeNeeded);

                } __except(EXCEPTION_EXECUTE_HANDLER) {

                    WmipUnreferenceGE(GuidEntry);
                    WmipLeaveSMCritSection();
                    WmipLeaveTLCritSection();
                    return GetExceptionCode();
                } 
            }

            WmipUnreferenceGE(GuidEntry);

        }  else {

            Status = STATUS_UNSUCCESSFUL;
        }

    } else {

        //
        // This must be a control operation.
        // Check to see if heap/critsec controller has access 
        // to proper Guids.
        //
        Status = WmipCheckGuidAccess(
                    &Guid,
                    DesiredAccess,
                    EtwpDefaultTraceSecurityDescriptor
                    );
        if (!NT_SUCCESS(Status)) {
            if( GuidEntry ){
                WmipUnreferenceGE(GuidEntry);
            }
            WmipLeaveSMCritSection();
            WmipLeaveTLCritSection();
            return Status;
        }

        if(SizeNeeded){

            if(GuidEntry == NULL){

                GuidEntry = WmipAllocGuidEntry();

                if (GuidEntry){

                    //
                    // Initialize the guid entry and keep the ref count
                    // from creation. When tracelog enables we take a ref
                    // count and when it disables we release it
                    //
                    GuidEntry->Guid = Guid;
                    GuidEntry->EventRefCount = 1;
                    GuidEntry->Flags |= GE_NOTIFICATION_TRACE_FLAG;
                    InsertHeadList(WmipGEHeadPtr, &GuidEntry->MainGEList);

                    //
                    // Take Extra Refcount so that we release it at stoplogger call
                    //

                    WmipReferenceGE(GuidEntry); 

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            if(NT_SUCCESS(Status)){

                if(GuidEntry->LoggerInfo) {
                    Status = STATUS_UNSUCCESSFUL;
                } else {

                    GuidEntry->LoggerInfo = WmipAlloc(SizeNeeded);

                    if(GuidEntry->LoggerInfo){

                        WMITRACEENABLEDISABLEINFO TraceEnableInfo;
                        PTRACE_ENABLE_CONTEXT pContext;

                        __try {

                            pContext = (PTRACE_ENABLE_CONTEXT)&Buffer->LoggerInfo->Wnode.HistoricalContext;

                            pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;
                            pContext->LoggerId = 1;

                            RtlCopyMemory(GuidEntry->LoggerInfo,Buffer->LoggerInfo,SizeNeeded);

                        } __except(EXCEPTION_EXECUTE_HANDLER) {

                            WmipUnreferenceGE(GuidEntry);
                            WmipLeaveSMCritSection();
                            WmipLeaveTLCritSection();
                            return GetExceptionCode();
                        }

                        TraceEnableInfo.Guid = GuidEntry->Guid;
                        TraceEnableInfo.Enable = TRUE;
                        TraceEnableInfo.LoggerContext = 0;
                        Status = WmipEnableDisableTrace(IOCTL_WMI_ENABLE_DISABLE_TRACELOG, &TraceEnableInfo);

                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                WmipUnreferenceGE(GuidEntry);
            }
        } else {

            //
            // This is stop logger call.
            //

            if(GuidEntry){

                WMITRACEENABLEDISABLEINFO TraceEnableInfo;

                if(GuidEntry->LoggerInfo) {

                    __try{

                        if (RequestorMode != KernelMode){
                            ProbeForWrite(Buffer->LoggerInfo, sizeof(WMI_LOGGER_INFORMATION), sizeof(ULONGLONG));
                        }

                        Buffer->LoggerInfo->BufferSize     = GuidEntry->LoggerInfo->BufferSize;
                        Buffer->LoggerInfo->MinimumBuffers = GuidEntry->LoggerInfo->MinimumBuffers;
                        Buffer->LoggerInfo->MaximumBuffers = GuidEntry->LoggerInfo->MaximumBuffers;

                        WmipFree(GuidEntry->LoggerInfo);
                        GuidEntry->LoggerInfo = NULL;

                    } __except(EXCEPTION_EXECUTE_HANDLER) {

                            WmipUnreferenceGE(GuidEntry);
                            WmipLeaveSMCritSection();
                            WmipLeaveTLCritSection();
                            return GetExceptionCode();
                    }
                }

                TraceEnableInfo.Guid = GuidEntry->Guid;
                TraceEnableInfo.Enable = FALSE;
                TraceEnableInfo.LoggerContext = 0;

                //
                //  The Extra Refcount taken at logger start is released by calling
                //  Disable trace. 
                //
    
                Status = WmipEnableDisableTrace(IOCTL_WMI_ENABLE_DISABLE_TRACELOG, &TraceEnableInfo);
                WmipUnreferenceGE(GuidEntry); 
            } 
        }
    }
    
    WmipLeaveSMCritSection();
    WmipLeaveTLCritSection();

    return Status;
}

VOID
WmipValidateClockType(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
/*++

Routine Description:

    This routine is called to validate the requested clock type in the
    LoggerInfo. If the requested type can not be handled, we will override
    to a type that this system will support. 

    This routine assumes that LoggerInfo pointer is a valid one. 

Arguments:

    LoggerInfo - a pointer to the structure for the logger's control
                 and status information

Returned Value:

    Status of STATUS_SUCCESS 

--*/
{
    UNREFERENCED_PARAMETER (LoggerInfo);
}

