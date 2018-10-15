/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    shutdown.c

Abstract:

    This module contains the shutdown code for the memory management system.

--*/

#include "mi.h"

extern ULONG MmSystemShutdown;

typedef struct _MM_ZERO_PAGEFILE_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PMMPAGING_FILE PagingFile;
    PFN_NUMBER ZeroedPageFrame;
    PKEVENT AllDone;
} MM_ZERO_PAGEFILE_CONTEXT, *PMM_ZERO_PAGEFILE_CONTEXT;

VOID
MiReleaseAllMemory (
    VOID
    );

BOOLEAN
MiShutdownSystem (
    VOID
    );

VOID
MiZeroPageFile (
    IN PVOID Context
    );

LOGICAL
MiZeroAllPageFiles (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,MiZeroPageFile)
#pragma alloc_text(PAGELK,MiZeroAllPageFiles)
#pragma alloc_text(PAGELK,MiShutdownSystem)
#pragma alloc_text(PAGELK,MiReleaseAllMemory)
#pragma alloc_text(PAGELK,MmShutdownSystem)
#endif

ULONG MmZeroPageFile;

extern ULONG MmUnusedSegmentForceFree;
extern LIST_ENTRY MiVerifierDriverAddedThunkListHead;
extern LIST_ENTRY MmLoadedUserImageList;
extern LOGICAL MiZeroingDisabled;
extern ULONG MmNumberOfMappedMdls;
extern ULONG MmNumberOfMappedMdlsInUse;

VOID
MiZeroPageFile (
    IN PVOID Context
    )

/*++

Routine Description:

    This routine zeroes all inactive pagefile blocks in the specified paging
    file.

Arguments:

    Context - Supplies the information on which pagefile to zero and a zeroed
              page to use for the I/O.

Return Value:

    Returns TRUE on success, FALSE on failure.

Environment:

    Kernel mode, the caller must lock down PAGELK.

--*/

{
    PFN_NUMBER MaxPagesToWrite;
    PMMPFN Pfn1;
    PPFN_NUMBER Page;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_WRITE_CLUSTER];
    PMDL Mdl;
    NTSTATUS Status;
    KEVENT IoEvent;
    IO_STATUS_BLOCK IoStatus;
    KIRQL OldIrql;
    LARGE_INTEGER StartingOffset;
    ULONG count;
    ULONG i;
    PFN_NUMBER first;
    ULONG write;
    PKEVENT AllDone;
    SIZE_T NumberOfBytes;
    PMMPAGING_FILE PagingFile;
    PFN_NUMBER ZeroedPageFrame;
    PMM_ZERO_PAGEFILE_CONTEXT ZeroContext;

    ZeroContext = (PMM_ZERO_PAGEFILE_CONTEXT) Context;

    PagingFile = ZeroContext->PagingFile;
    ZeroedPageFrame = ZeroContext->ZeroedPageFrame;
    AllDone = ZeroContext->AllDone;

    ExFreePool (Context);

    NumberOfBytes = MmModifiedWriteClusterSize << PAGE_SHIFT;
    MaxPagesToWrite = NumberOfBytes >> PAGE_SHIFT;

    Mdl = (PMDL) MdlHack;
    Page = (PPFN_NUMBER)(Mdl + 1);

    KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

    MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    Mdl->StartVa = NULL;

    i = 0;
    Page = (PPFN_NUMBER)(Mdl + 1);

    for (i = 0; i < MaxPagesToWrite; i += 1) {
        *Page = ZeroedPageFrame;
        Page += 1;
    }

    count = 0;
    write = FALSE;

    SATISFY_OVERZEALOUS_COMPILER (first = 0);

    LOCK_PFN (OldIrql);

    for (i = 1; i < PagingFile->Size; i += 1) {

        if (RtlCheckBit (PagingFile->Bitmap, (ULONG) i) == 0) {

            //
            // Claim the pagefile location as the modified writer
            // may already be scanning.
            //

            RtlSetBit (PagingFile->Bitmap, (ULONG) i);

            if (count == 0) {
                first = i;
            }

            count += 1;

            if ((count == MaxPagesToWrite) || (i == PagingFile->Size - 1)) {
                write = TRUE;
            }
        }
        else {
            if (count != 0) {

                //
                // Issue a write.
                //

                write = TRUE;
            }
        }

        if (write) {

            UNLOCK_PFN (OldIrql);

            StartingOffset.QuadPart = (LONGLONG)first << PAGE_SHIFT;
            Mdl->ByteCount = count << PAGE_SHIFT;
            KeClearEvent (&IoEvent);

            Status = IoSynchronousPageWrite (PagingFile->File,
                                             Mdl,
                                             &StartingOffset,
                                             &IoEvent,
                                             &IoStatus);

            //
            // Ignore all I/O failures - there is nothing that can
            // be done at this point.
            //

            if (!NT_SUCCESS (Status)) {
                KeSetEvent (&IoEvent, 0, FALSE);
            }

            Status = KeWaitForSingleObject (&IoEvent,
                                            WrPageOut,
                                            KernelMode,
                                            FALSE,
                                            (PLARGE_INTEGER)&MmTwentySeconds);

            if (Status == STATUS_TIMEOUT) {

                //
                // The write did not complete in 20 seconds, assume
                // that the file systems are hung and return an error.
                //
                // Note the zero page (and any MDL system virtual address a
                // driver may have created) is leaked because we don't know
                // what the filesystem or storage stack might (still) be
                // doing to them.
                //

                Pfn1 = MI_PFN_ELEMENT (ZeroedPageFrame);

                LOCK_PFN (OldIrql);

                //
                // Increment the reference count on the zeroed page to ensure
                // it is never freed.
                //

                InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);

                RtlClearBits (PagingFile->Bitmap, (ULONG) first, count);

                break;
            }

            if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
            }

            write = FALSE;
            LOCK_PFN (OldIrql);
            RtlClearBits (PagingFile->Bitmap, (ULONG) first, count);
            count = 0;
        }
    }

    UNLOCK_PFN (OldIrql);

    KeSetEvent (AllDone, 0, FALSE);
    return;
}
LOGICAL
MiZeroAllPageFiles (
    VOID
    )

/*++

Routine Description:

    This routine zeroes all inactive pagefile blocks in all pagefiles.

Arguments:

    None.

Return Value:

    Returns TRUE on success, FALSE on failure.

Environment:

    Kernel mode, the caller must lock down PAGELK.

--*/

{
    PMMPFN Pfn1;
    PFN_NUMBER MaxPagesToWrite;
    KIRQL OldIrql;
    ULONG i;
    PFN_NUMBER j;
    PFN_NUMBER PageFrameIndex;
    PMM_ZERO_PAGEFILE_CONTEXT ZeroContext;
    ULONG NumberOfPagingFiles;
    KEVENT WaitEvents[MAX_PAGE_FILES];
    PKEVENT WaitObjects[MAX_PAGE_FILES];
    KWAIT_BLOCK WaitBlockArray[MAX_PAGE_FILES];

    MaxPagesToWrite = MmModifiedWriteClusterSize;

    //
    // Get a zeroed page to use as the source for the writes.
    //

    LOCK_PFN (OldIrql);

    MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_FOR_PAGEFILE_ZEROING);

    if (MmAvailablePages < MM_LOW_LIMIT) {
        UNLOCK_PFN (OldIrql);
        MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_FOR_PAGEFILE_ZEROING);
        return TRUE;
    }

    PageFrameIndex = MiRemoveZeroPage (0);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    ASSERT (Pfn1->u2.ShareCount == 0);
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

    Pfn1->u3.e2.ReferenceCount = (USHORT) MaxPagesToWrite;
    Pfn1->PteAddress = (PMMPTE) (ULONG_PTR)(X64K | 0x1);
    Pfn1->OriginalPte.u.Long = 0;
    MI_SET_PFN_DELETED (Pfn1);

    UNLOCK_PFN (OldIrql);

    //
    // Capture the number of paging files in case a new one gets added.
    //

    NumberOfPagingFiles = MmNumberOfPagingFiles;

    for (i = NumberOfPagingFiles; i != 0; i -= 1) {

        KeInitializeEvent (&WaitEvents[i - 1], NotificationEvent, FALSE);
        WaitObjects[i - 1] = &WaitEvents[i - 1];

        ZeroContext = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof (MM_ZERO_PAGEFILE_CONTEXT),
                                             'wZmM');

        if (ZeroContext == NULL) {
            KeSetEvent (WaitObjects[i - 1], 0, FALSE);
            continue;
        }

        ZeroContext->PagingFile = MmPagingFile[i - 1];
        ZeroContext->ZeroedPageFrame = PageFrameIndex;
        ZeroContext->AllDone = WaitObjects[i - 1];

        if (i != 1) {

            ExInitializeWorkItem (&ZeroContext->WorkItem,
                                  MiZeroPageFile,
                                  (PVOID) ZeroContext);

            ExQueueWorkItem (&ZeroContext->WorkItem, CriticalWorkQueue);
        }
        else {

            //
            // Zero the first pagefile ourself, then wait for
            // any others to finish.
            //

            KeSetEvent (WaitObjects[i - 1], 0, FALSE);
            MiZeroPageFile (ZeroContext);
        }
    }

    if (NumberOfPagingFiles > 1) {

        KeWaitForMultipleObjects (NumberOfPagingFiles,
                                  &WaitObjects[0],
                                  WaitAll,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL,
                                  &WaitBlockArray[0]);
    }

    LOCK_PFN (OldIrql);

    ASSERT (Pfn1->u3.e2.ReferenceCount >= MaxPagesToWrite);

    if (Pfn1->u3.e2.ReferenceCount == MaxPagesToWrite) {
        MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_FOR_PAGEFILE_ZEROING);
    }

    for (j = 0; j < MaxPagesToWrite; j += 1) {
        MiDecrementReferenceCountInline (Pfn1, PageFrameIndex);
    }

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

BOOLEAN
MiShutdownSystem (
    VOID
    )

/*++

Routine Description:

    This function performs the shutdown of memory management.  This
    is accomplished by writing out all modified pages which are
    destined for files other than the paging file.

    All processes have already been killed, the registry shutdown and
    shutdown IRPs already sent.  On return from this phase all mapped
    file data must be flushed and the unused segment list emptied.
    This releases all the Mm references to file objects, allowing many
    drivers (especially the network) to unload.

Arguments:

    None.

Return Value:

    TRUE if the pages were successfully written, FALSE otherwise.

--*/

{
    SIZE_T ImportListSize;
    PLOAD_IMPORTS ImportList;
    PLOAD_IMPORTS ImportListNonPaged;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PFN_NUMBER ModifiedPage;
    PMMPFN Pfn1;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PPFN_NUMBER Page;
    PFILE_OBJECT FilePointer;
    ULONG ConsecutiveFileLockFailures;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_WRITE_CLUSTER];
    PMDL Mdl;
    NTSTATUS Status;
    KEVENT IoEvent;
    IO_STATUS_BLOCK IoStatus;
    KIRQL OldIrql;
    LARGE_INTEGER StartingOffset;
    ULONG count;
    ULONG i;

    //
    // Don't do this more than once.
    //

    if (MmSystemShutdown == 0) {

        Mdl = (PMDL) MdlHack;
        Page = (PPFN_NUMBER)(Mdl + 1);

        KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

        MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

        Mdl->MdlFlags |= MDL_PAGES_LOCKED;

        MmLockPageableSectionByHandle (ExPageLockHandle);

        LOCK_PFN (OldIrql);

        ModifiedPage = MmModifiedPageListHead.Flink;

        while (ModifiedPage != MM_EMPTY_LIST) {

            //
            // There are modified pages.
            //

            Pfn1 = MI_PFN_ELEMENT (ModifiedPage);

            if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {

                //
                // This page is destined for a file.
                //

                Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
                ControlArea = Subsection->ControlArea;
                if ((!ControlArea->u.Flags.Image) &&
                   (!ControlArea->u.Flags.NoModifiedWriting)) {

                    MiUnlinkPageFromList (Pfn1);

                    //
                    // Issue the write.
                    //

                    MI_SET_MODIFIED (Pfn1, 0, 0x28);

                    //
                    // Up the reference count for the physical page as there
                    // is I/O in progress.
                    //

                    MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1);

                    *Page = ModifiedPage;
                    ControlArea->NumberOfMappedViews += 1;
                    ControlArea->NumberOfPfnReferences += 1;

                    UNLOCK_PFN (OldIrql);

                    StartingOffset.QuadPart = MiStartingOffset (Subsection,
                                                                Pfn1->PteAddress);
                    Mdl->StartVa = NULL;

                    ConsecutiveFileLockFailures = 0;
                    FilePointer = ControlArea->FilePointer;

retry:
                    KeClearEvent (&IoEvent);

                    Status = FsRtlAcquireFileForCcFlushEx (FilePointer);

                    if (NT_SUCCESS(Status)) {
                        Status = IoSynchronousPageWrite (FilePointer,
                                                         Mdl,
                                                         &StartingOffset,
                                                         &IoEvent,
                                                         &IoStatus);

                        //
                        // Release the file we acquired.
                        //

                        FsRtlReleaseFileForCcFlush (FilePointer);
                    }

                    if (!NT_SUCCESS(Status)) {

                        //
                        // Only try the request more than once if the
                        // filesystem said it had a deadlock.
                        //

                        if (Status == STATUS_FILE_LOCK_CONFLICT) {
                            ConsecutiveFileLockFailures += 1;
                            if (ConsecutiveFileLockFailures < 5) {
                                KeDelayExecutionThread (KernelMode,
                                                        FALSE,
                                                        (PLARGE_INTEGER)&MmShortTime);
                                goto retry;
                            }
                            goto wait_complete;
                        }

                        //
                        // Ignore all I/O failures - there is nothing that
                        // can be done at this point.
                        //

                        KeSetEvent (&IoEvent, 0, FALSE);
                    }

                    Status = KeWaitForSingleObject (&IoEvent,
                                                    WrPageOut,
                                                    KernelMode,
                                                    FALSE,
                                                    (PLARGE_INTEGER)&MmTwentySeconds);

wait_complete:

                    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                    }

                    if (Status == STATUS_TIMEOUT) {

                        //
                        // The write did not complete in 20 seconds, assume
                        // that the file systems are hung and return an
                        // error.
                        //

                        LOCK_PFN (OldIrql);

                        MI_SET_MODIFIED (Pfn1, 1, 0xF);

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
                        ControlArea->NumberOfMappedViews -= 1;
                        ControlArea->NumberOfPfnReferences -= 1;

                        //
                        // This routine returns with the PFN lock released!
                        //

                        MiCheckControlArea (ControlArea, OldIrql);

                        MmUnlockPageableImageSection (ExPageLockHandle);

                        return FALSE;
                    }

                    LOCK_PFN (OldIrql);
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
                    ControlArea->NumberOfMappedViews -= 1;
                    ControlArea->NumberOfPfnReferences -= 1;

                    //
                    // This routine returns with the PFN lock released!
                    //

                    MiCheckControlArea (ControlArea, OldIrql);
                    LOCK_PFN (OldIrql);

                    //
                    // Restart scan at the front of the list.
                    //

                    ModifiedPage = MmModifiedPageListHead.Flink;
                    continue;
                }
            }
            ModifiedPage = Pfn1->u1.Flink;
        }

        UNLOCK_PFN (OldIrql);

        //
        // Indicate to the modified page writer that the system has
        // shutdown.
        //

        MmSystemShutdown = 1;

        //
        // Check to see if the paging file should be overwritten.
        // Only free blocks are written.
        //

        if (MmZeroPageFile) {
            MiZeroAllPageFiles ();
        }

        MmUnlockPageableImageSection (ExPageLockHandle);
    }

    if (PoCleanShutdownEnabled ()) {

        //
        // Empty the unused segment list.
        //

        LOCK_PFN (OldIrql);
        MmUnusedSegmentForceFree = (ULONG)-1;
        KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);

        //
        // Give it 5 seconds to empty otherwise assume the filesystems are
        // hung and march on.
        //

        for (count = 0; count < 500; count += 1) {

            if (IsListEmpty(&MmUnusedSegmentList)) {
                break;
            }

            UNLOCK_PFN (OldIrql);

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmShortTime);
            LOCK_PFN (OldIrql);

#if DBG
            if (count == 400) {

                //
                // Everything should have been flushed by now.  Give the
                // filesystem team a chance to debug this on checked builds.
                //

                ASSERT (FALSE);
            }
#endif

            //
            // Resignal if needed in case more closed file objects triggered
            // additional entries.
            //

            if (MmUnusedSegmentForceFree == 0) {
                MmUnusedSegmentForceFree = (ULONG)-1;
                KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
            }
        }

        UNLOCK_PFN (OldIrql);

        //
        // Get rid of any paged pool references as they will be illegal
        // by the time MmShutdownSystem is called again since the filesystems
        // will have shutdown.
        //

        KeWaitForSingleObject (&MmSystemLoadLock,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        NextEntry = PsLoadedModuleList.Flink;
        while (NextEntry != &PsLoadedModuleList) {

            DataTableEntry = CONTAINING_RECORD (NextEntry,
                                                KLDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);

            ImportList = (PLOAD_IMPORTS)DataTableEntry->LoadedImports;

            if ((ImportList != (PVOID)LOADED_AT_BOOT) &&
                (ImportList != (PVOID)NO_IMPORTS_USED) &&
                (!SINGLE_ENTRY(ImportList))) {

                ImportListSize = ImportList->Count * sizeof(PVOID) + sizeof(SIZE_T);
                ImportListNonPaged = (PLOAD_IMPORTS) ExAllocatePoolWithTag (NonPagedPool,
                                                                    ImportListSize,
                                                                    'TDmM');

                if (ImportListNonPaged != NULL) {
                    RtlCopyMemory (ImportListNonPaged, ImportList, ImportListSize);
                    ExFreePool (ImportList);
                    DataTableEntry->LoadedImports = ImportListNonPaged;
                }
                else {

                    //
                    // Don't bother with the clean shutdown at this point.
                    //

                    PopShutdownCleanly = FALSE;
                    break;
                }
            }

            //
            // Free the full DLL name as it is pageable.
            //

            if (DataTableEntry->FullDllName.Buffer != NULL) {
                ExFreePool (DataTableEntry->FullDllName.Buffer);
                DataTableEntry->FullDllName.Buffer = NULL;
            }

            NextEntry = NextEntry->Flink;
        }

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

        //
        // Close all the pagefile handles, note we still have an object
        // reference to each keeping the underlying object resident.
        // At the end of Phase1 shutdown we'll release those references
        // to trigger the storage stack unload.  The handle close must be
        // done here however as it will reference pageable structures.
        //

        for (i = 0; i < MmNumberOfPagingFiles; i += 1) {

            //
            // Free each pagefile name now as it resides in paged pool and
            // may need to be inpaged to be freed.  Since the paging files
            // are going to be shutdown shortly, now is the time to access
            // pageable stuff and get rid of it.  Zeroing the buffer pointer
            // is sufficient as the only accesses to this are from the
            // try-except-wrapped GetSystemInformation APIs and all the
            // user processes are gone already.
            //
        
            ASSERT (MmPagingFile[i]->PageFileName.Buffer != NULL);
            ExFreePool (MmPagingFile[i]->PageFileName.Buffer);
            MmPagingFile[i]->PageFileName.Buffer = NULL;

            ZwClose (MmPagingFile[i]->FileHandle);
        }
    }

    return TRUE;
}

BOOLEAN
MmShutdownSystem (
    IN ULONG Phase
    )

/*++

Routine Description:

    This function performs the shutdown of memory management.  This
    is accomplished by writing out all modified pages which are
    destined for files other than the paging file.

Arguments:

    Phase - Supplies 0 on the initiation of shutdown.  All processes have
            already been killed, the registry shutdown and shutdown IRPs already
            sent.  On return from this phase all mapped file data must be
            flushed and the unused segment list emptied.  This releases all
            the Mm references to file objects, allowing many drivers (especially
            the network) to unload.

            Supplies 1 on the initiation of shutdown.  The filesystem stack
            has received its shutdown IRPs (the stack must free its paged pool
            allocations here and lock down any pageable code it intends to call)
            as no more references to pageable code or data are allowed on return.
            ie: Any IoPageRead at this point is illegal.
            Close the pagefile handles here so the filesystem stack will be
            dereferenced causing those drivers to unload as well.

            Supplies 2 on final shutdown of the system.  Any resources not
            freed by this point are treated as leaks and cause a bugcheck.

Return Value:

    TRUE if the pages were successfully written, FALSE otherwise.

--*/

{
    ULONG i;

    if (Phase == 0) {
        return MiShutdownSystem ();
    }

    if (Phase == 1) {

        //
        // The filesystem has shutdown.  References to pageable code or data
        // is no longer allowed at this point.
        //
        // Close the pagefile handles here so the filesystem stack will be
        // dereferenced causing those drivers to unload as well.
        //

        if (MmSystemShutdown < 2) {

            MmSystemShutdown = 2;

            if (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_PAGING) {

                //
                // Make any IoPageRead at this point illegal.  Detect this by
                // purging all system pageable memory.
                //

                MmTrimAllSystemPageableMemory (TRUE);

                //
                // There should be no dirty pages destined for the filesystem.
                // Give the filesystem team a shot to debug this on checked
                // builds.
                //

                ASSERT (MmModifiedPageListHead.Total == MmTotalPagesForPagingFile);
                //
                // Dereference all the pagefile objects to trigger a cascading
                // unload of the storage stack as this should be the last
                // reference to their driver objects.
                //

                for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
                    ObDereferenceObject (MmPagingFile[i]->File);
                }
            }
        }
        return TRUE;
    }

    ASSERT (Phase == 2);

    //
    // Check for resource leaks and bugcheck if any are found.
    //

    if (MmSystemShutdown < 3) {
        MmSystemShutdown = 3;
        if (PoCleanShutdownEnabled ()) {
            MiReleaseAllMemory ();
        }
    }

    return TRUE;
}


VOID
MiReleaseAllMemory (
    VOID
    )

/*++

Routine Description:

    This function performs the final release of memory management allocations.

Arguments:

    None.

Return Value:

    None.

Environment:

    No references to paged pool or pageable code/data are allowed.

--*/

{
    ULONG i;
    ULONG j;
    PEVENT_COUNTER EventSupport;
    PUNLOADED_DRIVERS Entry;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLOAD_IMPORTS ImportList;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    PMMINPAGE_SUPPORT Support;
    PSLIST_ENTRY SingleListEntry;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;

    ASSERT (MmUnusedSegmentList.Flink == &MmUnusedSegmentList);

    //
    // Don't clear free pages so problems can be debugged.
    //

    MiZeroingDisabled = TRUE;

    //
    // Free the unloaded driver list.
    //

    if (MmUnloadedDrivers != NULL) {
        Entry = &MmUnloadedDrivers[0];
        for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1) {
            if (Entry->Name.Buffer != NULL) {
                RtlFreeUnicodeString (&Entry->Name);
            }
            Entry += 1;
        }
        ExFreePool (MmUnloadedDrivers);
    }

    NextEntry = MmLoadedUserImageList.Flink;
    while (NextEntry != &MmLoadedUserImageList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        NextEntry = NextEntry->Flink;

        ExFreePool ((PVOID)DataTableEntry);
    }

    //
    // Release the loaded module list entries.
    //

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        ImportList = (PLOAD_IMPORTS)DataTableEntry->LoadedImports;

        if ((ImportList != (PVOID)LOADED_AT_BOOT) &&
            (ImportList != (PVOID)NO_IMPORTS_USED) &&
            (!SINGLE_ENTRY(ImportList))) {

                ExFreePool (ImportList);
        }

        if (DataTableEntry->FullDllName.Buffer != NULL) {
            ASSERT (DataTableEntry->FullDllName.Buffer == DataTableEntry->BaseDllName.Buffer);
        }

        NextEntry = NextEntry->Flink;

        ExFreePool ((PVOID)DataTableEntry);
    }

    //
    // Free the physical memory descriptor block.
    //

    ExFreePool (MmPhysicalMemoryBlock);

    ExFreePool (MiPfnBitMap.Buffer);

    //
    // Free the system views structure.
    //

    if (MmSession.SystemSpaceViewTable != NULL) {
        ExFreePool (MmSession.SystemSpaceViewTable);
    }

    if (MmSession.SystemSpaceBitMap != NULL) {
        ExFreePool (MmSession.SystemSpaceBitMap);
    }

    //
    // Free the pagefile structures - note the PageFileName buffer was freed
    // earlier as it resided in paged pool and may have needed an inpage
    // to be freed.
    //

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        ASSERT (MmPagingFile[i]->PageFileName.Buffer == NULL);
        for (j = 0; j < MM_PAGING_FILE_MDLS; j += 1) {
            ExFreePool (MmPagingFile[i]->Entry[j]);
        }
        ExFreePool (MmPagingFile[i]->Bitmap);
        ExFreePool (MmPagingFile[i]);
    }

    ASSERT (MmNumberOfMappedMdlsInUse == 0);

    i = 0;
    while (IsListEmpty (&MmMappedFileHeader.ListHead) != 0) {

        ModWriterEntry = (PMMMOD_WRITER_MDL_ENTRY)RemoveHeadList (
                                    &MmMappedFileHeader.ListHead);

        ExFreePool (ModWriterEntry);
        i += 1;
    }
    ASSERT (i == MmNumberOfMappedMdls);

    //
    // Free the paged pool bitmaps.
    //

    ExFreePool (MmPagedPoolInfo.PagedPoolAllocationMap);
    ExFreePool (MmPagedPoolInfo.EndOfPagedPoolBitmap);

    if (VerifierLargePagedPoolMap != NULL) {
        ExFreePool (VerifierLargePagedPoolMap);
    }

    //
    // Free the inpage structures.
    //

    while (ExQueryDepthSList (&MmInPageSupportSListHead) != 0) {

        SingleListEntry = InterlockedPopEntrySList (&MmInPageSupportSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         MMINPAGE_SUPPORT,
                                         ListEntry);

            ASSERT (Support->u1.e1.PrefetchMdlHighBits == 0);
            ExFreePool (Support);
        }
    }

    while (ExQueryDepthSList (&MmEventCountSListHead) != 0) {

        EventSupport = (PEVENT_COUNTER) InterlockedPopEntrySList (&MmEventCountSListHead);

        if (EventSupport != NULL) {
            ExFreePool (EventSupport);
        }
    }

    //
    // Free the verifier list last because it must be consulted to debug
    // any bugchecks.
    //

    NextEntry = MiVerifierDriverAddedThunkListHead.Flink;
    if (NextEntry != NULL) {
        while (NextEntry != &MiVerifierDriverAddedThunkListHead) {

            ThunkTableBase = CONTAINING_RECORD (NextEntry,
                                                DRIVER_SPECIFIED_VERIFIER_THUNKS,
                                                ListEntry );

            NextEntry = NextEntry->Flink;
            ExFreePool (ThunkTableBase);
        }
    }

    NextEntry = MiSuspectDriverList.Flink;
    while (NextEntry != &MiSuspectDriverList) {

        Verifier = CONTAINING_RECORD(NextEntry,
                                     MI_VERIFIER_DRIVER_ENTRY,
                                     Links);

        NextEntry = NextEntry->Flink;
        ExFreePool (Verifier);
    }
}

