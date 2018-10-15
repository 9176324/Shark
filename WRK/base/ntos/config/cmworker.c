/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmworker.c

Abstract:

    This module contains support for the worker thread of the registry.
    The worker thread (actually an executive worker thread is used) is
    required for operations that must take place in the context of the
    system process.  (particularly file I/O)

--*/

#include    "cmp.h"

extern  LIST_ENTRY  CmpHiveListHead;

VOID
CmpInitializeHiveList(
    VOID
    );

//
// ----- LAZY FLUSH CONTROL -----
//
// LAZY_FLUSH_INTERVAL_IN_SECONDS controls how many seconds will elapse
// between when the hive is marked dirty and when the lazy flush worker
// thread is queued to write the data to disk.
//
ULONG CmpLazyFlushIntervalInSeconds = 5;
//
// number of hive flushed at one time (default to all system hive + 2 = current logged on user hives)
//
ULONG CmpLazyFlushHiveCount = 7;

//
// LAZY_FLUSH_TIMEOUT_IN_SECONDS controls how long the lazy flush worker
// thread will wait for the registry lock before giving up and queueing
// the lazy flush timer again.
//
#define LAZY_FLUSH_TIMEOUT_IN_SECONDS 1

//
// force enable lazy flusher 10 mins after boot, regardless
//
#define ENABLE_LAZY_FLUSH_INTERVAL_IN_SECONDS   600

PKPROCESS   CmpSystemProcess;
KTIMER      CmpLazyFlushTimer;
KDPC        CmpLazyFlushDpc;
WORK_QUEUE_ITEM CmpLazyWorkItem;

KTIMER      CmpEnableLazyFlushTimer;
KDPC        CmpEnableLazyFlushDpc;

BOOLEAN CmpLazyFlushPending = FALSE;
BOOLEAN CmpForceForceFlush = FALSE;
BOOLEAN CmpHoldLazyFlush = TRUE;
BOOLEAN CmpDontGrowLogFile = FALSE;

extern BOOLEAN CmpNoWrite;
extern BOOLEAN CmpWasSetupBoot;
extern BOOLEAN HvShutdownComplete;
extern BOOLEAN CmpProfileLoaded;

//
// Indicate whether the "disk full" popup has been triggered yet or not.
//
extern BOOLEAN CmpDiskFullWorkerPopupDisplayed;

//
// set to true if disk full when trying to save the changes made between system hive loading and registry initialization
//
extern BOOLEAN CmpCannotWriteConfiguration;
extern UNICODE_STRING SystemHiveFullPathName;
extern HIVE_LIST_ENTRY CmpMachineHiveList[];
extern BOOLEAN  CmpTrackHiveClose;

#if DBG
PKTHREAD    CmpCallerThread = NULL;
#endif


VOID
CmpLazyFlushWorker(
    IN PVOID Parameter
    );

VOID
CmpLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CmpEnableLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
CmpDiskFullWarningWorker(
    IN PVOID WorkItem
    );

VOID
CmpDiskFullWarning(
    VOID
    );

BOOLEAN
CmpDoFlushNextHive(
    BOOLEAN     ForceFlush,
    PBOOLEAN    PostWarning,
    PULONG      DirtyCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpLazyFlush)
#pragma alloc_text(PAGE,CmpLazyFlushWorker)
#pragma alloc_text(PAGE,CmpDiskFullWarningWorker)
#pragma alloc_text(PAGE,CmpDiskFullWarning)
#pragma alloc_text(PAGE,CmpCmdHiveClose)
#pragma alloc_text(PAGE,CmpCmdInit)
#pragma alloc_text(PAGE,CmpCmdRenameHive)
#pragma alloc_text(PAGE,CmpCmdHiveOpen)
#pragma alloc_text(PAGE,CmSetLazyFlushState)
#endif

VOID 
CmpCmdHiveClose(
                     PCMHIVE    CmHive
                     )
/*++

Routine Description:

    Closes all the file handles for the specified hive
Arguments:

    CmHive - the hive to close

Return Value:
    none
--*/
{
    ULONG                   i;
    IO_STATUS_BLOCK         IoStatusBlock;
    FILE_BASIC_INFORMATION  BasicInfo;
    LARGE_INTEGER           systemtime;
    BOOLEAN                 oldFlag;

    CM_PAGED_CODE();

    //
    // disable hard error popups, to workaround ObAttachProcessStack 
    //
    oldFlag = IoSetThreadHardErrorMode(FALSE);

    //
    // Close the files associated with this hive.
    //
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE_OR_HIVE_LOADING(CmHive);

    for (i=0; i<HFILE_TYPE_MAX; i++) {
        if (CmHive->FileHandles[i] != NULL) {
            //
            // attempt to set change the last write time (profile guys are relying on it!)
            //
            if( i == HFILE_TYPE_PRIMARY ) {
                if( NT_SUCCESS(ZwQueryInformationFile(
                                        CmHive->FileHandles[i],
                                        &IoStatusBlock,
                                        &BasicInfo,
                                        sizeof(BasicInfo),
                                        FileBasicInformation) ) ) {

                    KeQuerySystemTime(&systemtime);

                    if( CmHive->Hive.DirtyFlag ) {
                        BasicInfo.LastWriteTime  = systemtime;
                    }
                    BasicInfo.LastAccessTime = systemtime;

                    ZwSetInformationFile(
                        CmHive->FileHandles[i],
                        &IoStatusBlock,
                        &BasicInfo,
                        sizeof(BasicInfo),
                        FileBasicInformation
                        );
                }

                CmpTrackHiveClose = TRUE;
                CmCloseHandle(CmHive->FileHandles[i]);
                CmpTrackHiveClose = FALSE;
                
            } else {
                CmCloseHandle(CmHive->FileHandles[i]);
            }
            
            CmHive->FileHandles[i] = NULL;
        }
    }
    //
    // restore hard error popups mode
    //
    IoSetThreadHardErrorMode(oldFlag);
}

VOID 
CmpCmdInit(
           BOOLEAN SetupBoot
            )
/*++

Routine Description:

    Initializes cm globals and flushes all hives to the disk.

Arguments:

    SetupBoot - whether the boot is from setup or a regular boot

Return Value:
    none
--*/
{
    LARGE_INTEGER   DueTime;

    CM_PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    //
    // Initialize lazy flush timer and DPC
    //
    KeInitializeDpc(&CmpLazyFlushDpc,
                    CmpLazyFlushDpcRoutine,
                    NULL);

    KeInitializeTimer(&CmpLazyFlushTimer);

    ExInitializeWorkItem(&CmpLazyWorkItem, CmpLazyFlushWorker, NULL);

    //
    // the one to force activate lazy flush 10 mins after boot.
    // arm the timer as well
    //
    KeInitializeDpc(&CmpEnableLazyFlushDpc,
                    CmpEnableLazyFlushDpcRoutine,
                    NULL);
    KeInitializeTimer(&CmpEnableLazyFlushTimer);
    DueTime.QuadPart = Int32x32To64(ENABLE_LAZY_FLUSH_INTERVAL_IN_SECONDS,
                                    - SECOND_MULT);

    KeSetTimer(&CmpEnableLazyFlushTimer,
               DueTime,
               &CmpEnableLazyFlushDpc);

    CmpNoWrite = CmpMiniNTBoot;

    CmpWasSetupBoot = SetupBoot;
    
    if (SetupBoot == FALSE) {
        CmpInitializeHiveList();
    } 
   
    //
    // Since we are done with initialization, 
    // disable the hive sharing
    // 
    if (CmpMiniNTBoot && CmpShareSystemHives) {
        CmpShareSystemHives = FALSE;
    }    
}


NTSTATUS 
CmpCmdRenameHive(
            PCMHIVE                     CmHive,
            POBJECT_NAME_INFORMATION    OldName,
            PUNICODE_STRING             NewName,
            ULONG                       NameInfoLength
            )
/*++

Routine Description:

    rename a cmhive's primary handle

    replaces old REG_CMD_RENAME_HIVE worker case

Arguments:

    CmHive - hive to rename
    
    OldName - old name information

    NewName - the new name for the file

    NameInfoLength - sizeof name information structure

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                    Status;
    HANDLE                      Handle;
    PFILE_RENAME_INFORMATION    RenameInfo;
    IO_STATUS_BLOCK IoStatusBlock;

    CM_PAGED_CODE();

    //
    // Rename a CmHive's primary handle
    //
    Handle = CmHive->FileHandles[HFILE_TYPE_PRIMARY];
    if (OldName != NULL) {
        ASSERT_PASSIVE_LEVEL();
        Status = ZwQueryObject(Handle,
                               ObjectNameInformation,
                               OldName,
                               NameInfoLength,
                               &NameInfoLength);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    RenameInfo = ExAllocatePool(PagedPool,
                                sizeof(FILE_RENAME_INFORMATION) + NewName->Length);
    if (RenameInfo == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RenameInfo->ReplaceIfExists = FALSE;
    RenameInfo->RootDirectory = NULL;
    RenameInfo->FileNameLength = NewName->Length;
    RtlCopyMemory(RenameInfo->FileName,
                  NewName->Buffer,
                  NewName->Length);

    Status = ZwSetInformationFile(Handle,
                                  &IoStatusBlock,
                                  (PVOID)RenameInfo,
                                  sizeof(FILE_RENAME_INFORMATION) +
                                  NewName->Length,
                                  FileRenameInformation);
    ExFreePool(RenameInfo);

    return Status;
}

NTSTATUS 
CmpCmdHiveOpen(
            POBJECT_ATTRIBUTES          FileAttributes,
            PSECURITY_CLIENT_CONTEXT    ImpersonationContext,
            PBOOLEAN                    Allocate,
            PCMHIVE                     *NewHive,
		    ULONG						CheckFlags
            )
/*++

Routine Description:


    replaces old REG_CMD_HIVE_OPEN worker case

Arguments:


Return Value:

    NTSTATUS

--*/
{
    PUNICODE_STRING FileName;
    NTSTATUS        Status;
    HANDLE          NullHandle;

    CM_PAGED_CODE();

    //
    // Open the file.
    //
    FileName = FileAttributes->ObjectName;

    Status = CmpInitHiveFromFile(FileName,
                                 0,
                                 NewHive,
                                 Allocate,
								 CheckFlags);
    //
    // NT Servers will return STATUS_ACCESS_DENIED. Netware 3.1x
    // servers could return any of the other error codes if the GUEST
    // account is disabled.
    //
    if (((Status == STATUS_ACCESS_DENIED) ||
         (Status == STATUS_NO_SUCH_USER) ||
         (Status == STATUS_WRONG_PASSWORD) ||
         (Status == STATUS_ACCOUNT_EXPIRED) ||
         (Status == STATUS_ACCOUNT_DISABLED) ||
         (Status == STATUS_ACCOUNT_RESTRICTION)) &&
        (ImpersonationContext != NULL)) {
        //
        // Impersonate the caller and try it again.  This
        // lets us open hives on a remote machine.
        //
        Status = SeImpersonateClientEx(
                        ImpersonationContext,
                        NULL);

        if ( NT_SUCCESS( Status ) ) {

            Status = CmpInitHiveFromFile(FileName,
                                         0,
                                         NewHive,
                                         Allocate,
										 CheckFlags);
            NullHandle = NULL;

            PsRevertToSelf();
        }
    }
    
    return Status;
}


VOID
CmpLazyFlush(
    VOID
    )

/*++

Routine Description:

    This routine resets the registry timer to go off at a specified interval
    in the future (LAZY_FLUSH_INTERVAL_IN_SECONDS).

Arguments:

    None

Return Value:

    None.

--*/

{
    LARGE_INTEGER DueTime;

    CM_PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlush: setting lazy flush timer\n"));
    if ((!CmpNoWrite) && (!CmpHoldLazyFlush)) {

        DueTime.QuadPart = Int32x32To64(CmpLazyFlushIntervalInSeconds,
                                        - SECOND_MULT);

        //
        // Indicate relative time
        //

        KeSetTimer(&CmpLazyFlushTimer,
                   DueTime,
                   &CmpLazyFlushDpc);

    }


}

VOID
CmpEnableLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    CmpHoldLazyFlush = FALSE;
}

VOID
CmpLazyFlushDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This is the DPC routine triggered by the lazy flush timer.  All it does
    is queue a work item to an executive worker thread.  The work item will
    do the actual lazy flush to disk.

Arguments:

    Dpc - Supplies a pointer to the DPC object.

    DeferredContext - not used

    SystemArgument1 - not used

    SystemArgument2 - not used

Return Value:

    None.

--*/

{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlushDpc: queuing lazy flush work item\n"));

    if ((!CmpLazyFlushPending) && (!CmpHoldLazyFlush)) {
        CmpLazyFlushPending = TRUE;
        ExQueueWorkItem(&CmpLazyWorkItem, DelayedWorkQueue);
    }

}

ULONG   CmpLazyFlushCount = 1;

VOID
CmpLazyFlushWorker(
    IN PVOID Parameter
    )

/*++

Routine Description:

    Worker routine called to do a lazy flush.  Called by an executive worker
    thread in the system process.  

Arguments:

    Parameter - not used.

Return Value:

    None.

--*/

{
    BOOLEAN Result = TRUE;
    BOOLEAN ForceFlush;
    BOOLEAN PostNewWorker = FALSE;
    ULONG   DirtyCount = 0;

    CM_PAGED_CODE();

    UNREFERENCED_PARAMETER (Parameter);

    if( CmpHoldLazyFlush ) {
        //
        // lazy flush mode is disabled
        //
        return;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_IO,"CmpLazyFlushWorker: flushing hives\n"));

    ForceFlush = CmpForceForceFlush;
    if(ForceFlush == TRUE) {
        //
        // something bad happened and we may need to fix hive's use count
        //
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpLazyFlushWorker: Force Flush - getting the reglock exclusive\n"));
        CmpLockRegistryExclusive();
    } else {
        CmpLockRegistry();
        ENTER_FLUSH_MODE();
    }
    if (!HvShutdownComplete) {
        //
        // no loads/unloads while flush in progress
        //
        LOCK_HIVE_LOAD();
        PostNewWorker = CmpDoFlushNextHive(ForceFlush,&Result,&DirtyCount);
        UNLOCK_HIVE_LOAD();

        if( !PostNewWorker ) {
            //
            // we have completed a sweep through the entire list of hives
            //
            InterlockedIncrement( (PLONG)&CmpLazyFlushCount );
        }
    } else {
        CmpForceForceFlush = FALSE;
    }

    if( ForceFlush == FALSE ) {
        EXIT_FLUSH_MODE();
    }

    CmpLazyFlushPending = FALSE;
    CmpUnlockRegistry();

    if( CmpCannotWriteConfiguration ) {
        //
        // Disk full; system hive has not been saved at initialization
        //
        if(!Result) {
            //
            // All hives were saved; No need for disk full warning anymore
            //
            CmpCannotWriteConfiguration = FALSE;
        } else {
            //
            // Issue another hard error (if not already displayed) and postpone a lazy flush operation
            //
            CmpDiskFullWarning();
            CmpLazyFlush();
        }
    }
    //
    // if we have not yet flushed the whole list; or there are still hives dirty from the curently ended iteration.
    //
    if( PostNewWorker || (DirtyCount != 0) ) {
        //
        // post a new worker to flush the next hive
        //
        CmpLazyFlush();
    }

}

VOID
CmpDiskFullWarningWorker(
    IN PVOID WorkItem
    )

/*++

Routine Description:

    Displays hard error popup that indicates the disk is full.

Arguments:

    WorkItem - Supplies pointer to the work item. This routine will
               free the work item.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    ULONG Response;

    ExFreePool(WorkItem);

    Status = ExRaiseHardError(STATUS_DISK_FULL,
                              0,
                              0,
                              NULL,
                              OptionOk,
                              &Response);
}



VOID
CmpDiskFullWarning(
    VOID
    )
/*++

Routine Description:

    Raises a hard error of type STATUS_DISK_FULL if wasn't already raised

Arguments:

    None

Return Value:

    None

--*/
{
    PWORK_QUEUE_ITEM WorkItem;

    if( (!CmpDiskFullWorkerPopupDisplayed) && (CmpCannotWriteConfiguration) && (ExReadyForErrors) && (CmpProfileLoaded) ) {

        //
        // Queue work item to display popup
        //
        WorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
        if (WorkItem != NULL) {

            CmpDiskFullWorkerPopupDisplayed = TRUE;
            ExInitializeWorkItem(WorkItem,
                                 CmpDiskFullWarningWorker,
                                 WorkItem);
            ExQueueWorkItem(WorkItem, DelayedWorkQueue);
        }
    }
}

VOID
CmpShutdownWorkers(
    VOID
    )
/*++

Routine Description:

    Shuts down the lazy flush worker (by killing the timer)

Arguments:

    None

Return Value:

    None

--*/
{
    CM_PAGED_CODE();

    KeCancelTimer(&CmpLazyFlushTimer);
}

VOID
CmSetLazyFlushState(__in BOOLEAN Enable)
/*++

Routine Description:
    
    Enables/Disables the lazy flusher; Designed for the standby/resume case, where 
    we we don't want the lazy flush to fire off, blocking registry writers until the 
    disk wakes up.

Arguments:

    Enable - TRUE = enable; FALSE = disable

Return Value:

    None.

--*/
{
    CM_PAGED_CODE();

    CmpDontGrowLogFile = CmpHoldLazyFlush = !Enable;
}

