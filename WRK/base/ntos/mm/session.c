/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   session.c

Abstract:

    This module contains the routines which implement the creation and
    deletion of session spaces along with associated support routines.

--*/

#include "mi.h"

LONG MmSessionDataPages;

KGUARDED_MUTEX  MiSessionIdMutex;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

PRTL_BITMAP MiSessionIdBitmap = {0};

#define MI_SESSION_ID_INCREMENT (POOL_SMALLEST_BLOCK * 8)

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

ULONG MiSessionDataPages;
ULONG MiSessionTagPages;
ULONG MiSessionTagSizePages;
ULONG MiSessionBigPoolPages;

ULONG MiSessionCreateCharge;

//
// Note that actually the SUM of the two maximums below is currently
// MM_ALLOCATION_GRANULARITY / PAGE_SIZE.
//

#define MI_SESSION_DATA_PAGES_MAXIMUM (MM_ALLOCATION_GRANULARITY / PAGE_SIZE)
#define MI_SESSION_TAG_PAGES_MAXIMUM  (MM_ALLOCATION_GRANULARITY / PAGE_SIZE)

VOID
MiSessionAddProcess (
    PEPROCESS NewProcess
    );

VOID
MiSessionRemoveProcess (
    VOID
    );

VOID
MiInitializeSessionIds (
    VOID
    );

NTSTATUS
MiSessionCreateInternal (
    OUT PULONG SessionId
    );

NTSTATUS
MiSessionCommitPageTables (
    IN PVOID StartVa,
    IN PVOID EndVa
    );

VOID
MiDereferenceSession (
    VOID
    );

VOID
MiSessionDeletePde (
    IN PMMPTE Pde,
    IN PMMPTE SelfMapPde
    );

VOID
MiDereferenceSessionFinal (
    VOID
    );

#if DBG
VOID
MiCheckSessionVirtualSpace (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeSessionIds)

#pragma alloc_text(PAGE, MmSessionSetUnloadAddress)
#pragma alloc_text(PAGE, MmSessionCreate)
#pragma alloc_text(PAGE, MmSessionDelete)
#pragma alloc_text(PAGE, MmGetSessionLocaleId)
#pragma alloc_text(PAGE, MmSetSessionLocaleId)
#pragma alloc_text(PAGE, MmQuitNextSession)
#pragma alloc_text(PAGE, MiDereferenceSession)
#pragma alloc_text(PAGE, MiSessionPoolLookaside)
#pragma alloc_text(PAGE, MiSessionPoolSmallLists)
#pragma alloc_text(PAGE, MiSessionPoolTrackTable)
#pragma alloc_text(PAGE, MiSessionPoolTrackTableSize)
#pragma alloc_text(PAGE, MiSessionPoolBigPageTable)
#pragma alloc_text(PAGE, MiSessionPoolBigPageTableSize)
#if DBG
#pragma alloc_text(PAGE, MiCheckSessionVirtualSpace)
#endif

#pragma alloc_text(PAGELK, MiSessionCreateInternal)
#pragma alloc_text(PAGELK, MiDereferenceSessionFinal)

#endif

VOID
MiSessionLeader (
    IN PEPROCESS Process
    )

/*++

Routine Description:

    Mark the argument process as having the ability to create or delete session
    spaces.  This is only granted to the session manager process.

Arguments:

    Process - Supplies a pointer to the privileged process.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    KIRQL OldIrql;

    LOCK_EXPANSION (OldIrql);

    Process->Vm.Flags.SessionLeader = 1;

    UNLOCK_EXPANSION (OldIrql);
}


VOID
MmSessionSetUnloadAddress (
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    Copy the win32k.sys driver object to the session structure for use during
    unload.

Arguments:

    DriverObject - Supplies a pointer to the win32k driver object.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ASSERT (PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION);
    ASSERT (MmIsAddressValid(MmSessionSpace) == TRUE);

    MmSessionSpace->Win32KDriverUnload = DriverObject->DriverUnload;
}

VOID
MiSessionAddProcess (
    PEPROCESS NewProcess
    )

/*++

Routine Description:

    Add the new process to the current session space.

Arguments:

    NewProcess - Supplies a pointer to the process being created.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled.

--*/

{
    KIRQL OldIrql;
    PMM_SESSION_SPACE SessionGlobal;

    //
    // If the calling process has no session, then the new process won't get
    // one either.
    //

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
        return;
    }

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    SessionGlobal = SESSION_GLOBAL(MmSessionSpace);

    InterlockedIncrement ((PLONG)&SessionGlobal->ReferenceCount);

    InterlockedIncrement (&SessionGlobal->ResidentProcessCount);

    InterlockedIncrement (&SessionGlobal->ProcessReferenceToSession);

    //
    // Once the Session pointer in the EPROCESS is set it can never
    // be cleared because it is accessed lock-free.
    //

    ASSERT (NewProcess->Session == NULL);
    NewProcess->Session = (PVOID) SessionGlobal;

    //
    // Link the process entry into the session space and WSL structures.
    //

    LOCK_EXPANSION (OldIrql);

    InsertTailList (&SessionGlobal->ProcessList, &NewProcess->SessionProcessLinks);
    UNLOCK_EXPANSION (OldIrql);

    PS_SET_BITS (&NewProcess->Flags, PS_PROCESS_FLAGS_IN_SESSION);
}


VOID
MiSessionRemoveProcess (
    VOID
    )

/*++

Routine Description:

    This routine removes the current process from the current session space.
    This may trigger a substantial round of dereferencing and resource freeing
    if it is also the last process in the session, (holding the last image
    in the group, etc).

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL and below, but queueing of APCs to this thread has
    been permanently disabled.  This is the last thread in the process
    being deleted.  The caller has ensured that this process is not
    on the expansion list and therefore there can be no races in regards to
    trimming.

--*/

{
    KIRQL OldIrql;
    PEPROCESS CurrentProcess;
#if DBG
    ULONG Found;
    PEPROCESS Process;
    PLIST_ENTRY NextEntry;
    PMM_SESSION_SPACE SessionGlobal;
#endif

    CurrentProcess = PsGetCurrentProcess();

    if (((CurrentProcess->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) ||
        (CurrentProcess->Vm.Flags.SessionLeader == 1)) {
        return;
    }

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    //
    // Remove this process from the list of processes in the current session.
    //

    LOCK_EXPANSION (OldIrql);

#if DBG

    SessionGlobal = SESSION_GLOBAL(MmSessionSpace);

    Found = 0;
    NextEntry = SessionGlobal->ProcessList.Flink;

    while (NextEntry != &SessionGlobal->ProcessList) {
        Process = CONTAINING_RECORD (NextEntry, EPROCESS, SessionProcessLinks);

        if (Process == CurrentProcess) {
            Found = 1;
        }

        NextEntry = NextEntry->Flink;
    }

    ASSERT (Found == 1);

#endif

    RemoveEntryList (&CurrentProcess->SessionProcessLinks);

    UNLOCK_EXPANSION (OldIrql);

    //
    // Decrement this process' reference count to the session.  If this
    // is the last reference, then the entire session will be destroyed
    // upon return.  This includes unloading drivers, unmapping pools,
    // freeing page tables, etc.
    //

    MiDereferenceSession ();
}

LCID
MmGetSessionLocaleId (
    VOID
    )

/*++

Routine Description:

    This routine gets the locale ID for the current session.

Arguments:

    None.

Return Value:

    The locale ID for the current session.

Environment:

    PASSIVE_LEVEL, the caller must supply any desired synchronization.

--*/

{
    PEPROCESS Process;
    PMM_SESSION_SPACE SessionGlobal;

    PAGED_CODE ();

    Process = PsGetCurrentProcess ();

    if (Process->Vm.Flags.SessionLeader == 1) {

        //
        // smss may transiently have a session space but that's of no interest
        // to our caller.
        //

        return PsDefaultThreadLocaleId;
    }

    //
    // The Session field of the EPROCESS is never cleared once set so these
    // checks can be done lock free.
    //

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    if (SessionGlobal == NULL) {

        //
        // The system process has no session space.
        //

        return PsDefaultThreadLocaleId;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    return SessionGlobal->LocaleId;
}

VOID
MmSetSessionLocaleId (
    IN LCID LocaleId
    )

/*++

Routine Description:

    This routine sets the locale ID for the current session.

Arguments:

    LocaleId - Supplies the desired locale ID.

Return Value:

    None.

Environment:

    PASSIVE_LEVEL, the caller must supply any desired synchronization.

--*/

{
    PEPROCESS Process;
    PMM_SESSION_SPACE SessionGlobal;

    PAGED_CODE ();

    Process = PsGetCurrentProcess ();

    if (Process->Vm.Flags.SessionLeader == 1) {

        //
        // smss may transiently have a session space but that's of no interest
        // to our caller.
        //

        PsDefaultThreadLocaleId = LocaleId;
        return;
    }

    //
    // The Session field of the EPROCESS is never cleared once set so these
    // checks can be done lock free.
    //

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    if (SessionGlobal == NULL) {

        //
        // The system process has no session space.
        //

        PsDefaultThreadLocaleId = LocaleId;
        return;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    SessionGlobal->LocaleId = LocaleId;
}

VOID
MiInitializeSessionIds (
    VOID
    )

/*++

Routine Description:

    This routine creates and initializes session ID allocation/deallocation.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG TotalPages;

    //
    // If this ever grows beyond the maximum size, the session virtual
    // address space layout will need to be enlarged.
    //

    TotalPages = MI_SESSION_DATA_PAGES_MAXIMUM;

    MiSessionDataPages = ROUND_TO_PAGES (sizeof (MM_SESSION_SPACE));
    MiSessionDataPages >>= PAGE_SHIFT;

    ASSERT (MiSessionDataPages <= MI_SESSION_DATA_PAGES_MAXIMUM - 3);

    TotalPages -= MiSessionDataPages;

    MiSessionTagSizePages = 2;
    MiSessionBigPoolPages = 1;

    MiSessionTagPages = MiSessionTagSizePages + MiSessionBigPoolPages;

    ASSERT (MiSessionTagPages <= TotalPages);
    ASSERT (MiSessionTagPages < MI_SESSION_TAG_PAGES_MAXIMUM);

#if defined(_AMD64_)
    MiSessionCreateCharge = 3 + MiSessionDataPages + MiSessionTagPages;
#elif defined(_X86_)
    MiSessionCreateCharge = 1 + MiSessionDataPages + MiSessionTagPages;
#else
#error "no target architecture"
#endif

    KeInitializeGuardedMutex (&MiSessionIdMutex);

    MiCreateBitMap (&MiSessionIdBitmap, MI_SESSION_ID_INCREMENT, PagedPool);

    if (MiSessionIdBitmap != NULL) {
        RtlClearAllBits (MiSessionIdBitmap);
    }
    else {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      0x200);
    }
}


ULONG
MiSessionPoolSmallLists (
    VOID
    )
{
    return SESSION_POOL_SMALL_LISTS;
}

PGENERAL_LOOKASIDE
MiSessionPoolLookaside (
    VOID
    )
{
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    return (PVOID) &MmSessionSpace->Lookaside;
}

PVOID
MiSessionPoolTrackTable (
    VOID
    )
{
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    return (PPOOL_TRACKER_TABLE) ((ULONG_PTR)MmSessionSpace + (MiSessionDataPages << PAGE_SHIFT));
}

SIZE_T
MiSessionPoolTrackTableSize (
    VOID
    )
{
    SIZE_T i;
    SIZE_T NumberOfBytes;
    SIZE_T NumberOfEntries;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    NumberOfBytes = (MiSessionTagSizePages << PAGE_SHIFT);
    NumberOfEntries = NumberOfBytes / sizeof (POOL_TRACKER_TABLE);

    //
    // Convert to the closest power of 2 <= NumberOfEntries.
    //

    for (i = 0; i < 32; i += 1) {
        if (((SIZE_T)1 << i) > NumberOfEntries) {
            return (((SIZE_T)1) << (i - 1));
        }
    }

    ASSERT (FALSE);

    return 0;
}

PVOID
MiSessionPoolBigPageTable (
    VOID
    )
{
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    return (PPOOL_TRACKER_BIG_PAGES) ((ULONG_PTR)MmSessionSpace + ((MiSessionDataPages + MiSessionTagSizePages) << PAGE_SHIFT));
}

SIZE_T
MiSessionPoolBigPageTableSize (
    VOID
    )
{
    SIZE_T i;
    SIZE_T NumberOfBytes;
    SIZE_T NumberOfEntries;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    NumberOfBytes = (MiSessionBigPoolPages << PAGE_SHIFT);
    NumberOfEntries = NumberOfBytes / sizeof (POOL_TRACKER_BIG_PAGES);

    //
    // Convert to the closest power of 2 <= NumberOfEntries.
    //

    for (i = 0; i < 32; i += 1) {
        if (((SIZE_T)1 << i) > NumberOfEntries) {
            return (((SIZE_T)1) << (i - 1));
        }
    }

    ASSERT (FALSE);

    return 0;
}

NTSTATUS
MiSessionCreateInternal (
    OUT PULONG SessionId
    )

/*++

Routine Description:

    This routine creates the data structure that describes and maintains
    the session space.  It resides at the beginning of the session space.
    Carefully construct the first page mapping to bootstrap the fault
    handler which relies on the session space data structure being
    present and valid.

    In NT32, this initial mapping for the portion of session space
    mapped by the first PDE will automatically be inherited by all child
    processes when the system copies the system portion of the page
    directory for new address spaces.  Additional entries are faulted
    in by the session space fault handler, which references this structure.

    For NT64, everything is automatically inherited.

    This routine commits virtual memory within the current session space with
    backing pages.  The virtual addresses within session space are
    allocated with a separate facility in the image management facility.
    This is because images must be at a unique system wide virtual address.

Arguments:

    SessionId - Supplies a pointer to place the new session ID into.

Return Value:

    STATUS_SUCCESS if all went well, various failure status codes
    if the session was not created.

Environment:

    Kernel mode, no mutexes held.

--*/

{
    KIRQL  OldIrql;
    PRTL_BITMAP NewBitmap;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE GlobalMappingPte;
    NTSTATUS Status;
    PMM_SESSION_SPACE SessionSpace;
    PMM_SESSION_SPACE SessionGlobal;
    PFN_NUMBER ResidentPages;
    LOGICAL GotCommit;
    LOGICAL GotPages;
    LOGICAL PoolInitialized;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    MMPTE TempPte;
    PFN_NUMBER DataPage[MI_SESSION_DATA_PAGES_MAXIMUM] = {0};
    PFN_NUMBER PageTablePage;
    PFN_NUMBER TagPage[MI_SESSION_TAG_PAGES_MAXIMUM];
    ULONG i;
    ULONG PageColor;
    ULONG ProcessFlags;
    ULONG NewProcessFlags;
    PEPROCESS Process;
#if (_MI_PAGING_LEVELS < 3)
    SIZE_T PageTableBytes;
    PMMPTE PageTables;
#else
    PMMPTE PointerPpe;
    PFN_NUMBER PageDirectoryPage;
    PFN_NUMBER PageDirectoryParentPage;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif

    GotCommit = FALSE;
    GotPages = FALSE;
    GlobalMappingPte = NULL;
    PoolInitialized = FALSE;

    //
    // Initializing these are not needed for correctness
    // but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    PageTablePage = 0;
    PointerPte = NULL;

#if (_MI_PAGING_LEVELS >= 3)
    PageDirectoryPage = 0;
    PageDirectoryParentPage = 0;
#endif

    Process = PsGetCurrentProcess ();

    ASSERT (MmIsAddressValid(MmSessionSpace) == FALSE);

    //
    // Check for concurrent session creation attempts.
    //

    ProcessFlags = Process->Flags;

    while (TRUE) {

        if (ProcessFlags & PS_PROCESS_FLAGS_CREATING_SESSION) {
            return STATUS_ALREADY_COMMITTED;
        }

        NewProcessFlags = (ProcessFlags | PS_PROCESS_FLAGS_CREATING_SESSION);

        NewProcessFlags = InterlockedCompareExchange ((PLONG)&Process->Flags,
                                                      (LONG)NewProcessFlags,
                                                      (LONG)ProcessFlags);
                                                             
        if (NewProcessFlags == ProcessFlags) {
            break;
        }

        //
        // The structure changed beneath us.  Use the return value from the
        // exchange and try it all again.
        //

        ProcessFlags = NewProcessFlags;
    }

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_CREATING_SESSION);

#if (_MI_PAGING_LEVELS < 3)

    PageTableBytes = MI_SESSION_SPACE_MAXIMUM_PAGE_TABLES * sizeof (MMPTE);

    PageTables = (PMMPTE) ExAllocatePoolWithTag (NonPagedPool,
                                                 PageTableBytes,
                                                 'tHmM');

    if (PageTables == NULL) {

        ASSERT (Process->Flags & PS_PROCESS_FLAGS_CREATING_SESSION);
        PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATING_SESSION);

        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (PageTables, PageTableBytes);

#endif

    //
    // Select a free session ID.
    //



    KeAcquireGuardedMutex (&MiSessionIdMutex);

    *SessionId = RtlFindClearBitsAndSet (MiSessionIdBitmap, 1, 0);

    if (*SessionId == NO_BITS_FOUND) {

        MiCreateBitMap (&NewBitmap,
                        MiSessionIdBitmap->SizeOfBitMap + MI_SESSION_ID_INCREMENT,
                        PagedPool);

        if (NewBitmap == NULL) {

            KeReleaseGuardedMutex (&MiSessionIdMutex);

            ASSERT (Process->Flags & PS_PROCESS_FLAGS_CREATING_SESSION);
            PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATING_SESSION);

#if (_MI_PAGING_LEVELS < 3)
            ExFreePool (PageTables);
#endif

            MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_IDS);
            return STATUS_NO_MEMORY;
        }

        RtlClearAllBits (NewBitmap);

        //
        // Copy the bits from the existing map.
        //

        RtlCopyMemory (NewBitmap->Buffer,
                       MiSessionIdBitmap->Buffer,
                       ((MiSessionIdBitmap->SizeOfBitMap + 31) / 32) * sizeof (ULONG));

        MiRemoveBitMap (&MiSessionIdBitmap);

        MiSessionIdBitmap = NewBitmap;

        *SessionId = RtlFindClearBitsAndSet (MiSessionIdBitmap, 1, 0);

        ASSERT (*SessionId != NO_BITS_FOUND);
    }

    KeReleaseGuardedMutex (&MiSessionIdMutex);

    //
    // Lock down this routine in preparation for the PFN lock acquisition.
    // Note this is done prior to the commitment charges just to simplify
    // error handling.
    //

    MmLockPageableSectionByHandle (ExPageLockHandle);

    //
    // Charge commitment.
    //


    ResidentPages = MiSessionCreateCharge;

    if (MiChargeCommitment (ResidentPages, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        goto Failure;
    }

    GotCommit = TRUE;

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_CREATE, ResidentPages);

    //
    // Reserve global system PTEs to map the data pages with.
    //

    GlobalMappingPte = MiReserveSystemPtes (MiSessionDataPages, SystemPteSpace);

    if (GlobalMappingPte == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_SYSPTES);
        goto Failure;
    }

    //
    // Ensure the resident physical pages are available.
    //

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)(ResidentPages + MI_SESSION_SPACE_WORKING_SET_MINIMUM) > MI_NONPAGEABLE_MEMORY_AVAILABLE()) {

        UNLOCK_PFN (OldIrql);

        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_RESIDENT);
        goto Failure;
    }

    GotPages = TRUE;

    MI_DECREMENT_RESIDENT_AVAILABLE (
        ResidentPages + MI_SESSION_SPACE_WORKING_SET_MINIMUM,
        MM_RESAVAIL_ALLOCATE_CREATE_SESSION);

    //
    // Allocate both session space data pages first as on some architectures
    // a region ID will be used immediately for the TB references as the
    // PTE mappings are initialized.
    //

    TempPte.u.Long = ValidKernelPte.u.Long;

    for (i = 0; i < MiSessionDataPages; i += 1) {

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, OldIrql);
        }

        PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

        DataPage[i] = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

        TempPte.u.Hard.PageFrameNumber = DataPage[i];

        //
        // Map the data pages immediately in global space.  Some architectures
        // use a region ID which is used immediately for the TB references after
        // the PTE mappings are initialized.
        //
        //
        // The global bit can be left on for the global mappings (unlike the
        // session space mapping which must have the global bit off since
        // we need to make sure the TB entry is flushed when we switch to
        // a process in a different session space).
        //

        MI_WRITE_VALID_PTE (GlobalMappingPte + i, TempPte);
    }

    SessionGlobal = (PMM_SESSION_SPACE) MiGetVirtualAddressMappedByPte (GlobalMappingPte);


#if (_MI_PAGING_LEVELS >= 3)

    //
    // Initialize the page directory parent page.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

    PageDirectoryParentPage = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

    TempPte.u.Long = ValidKernelPdeLocal.u.Long;
    TempPte.u.Hard.PageFrameNumber = PageDirectoryParentPage;

    //
    // The global bit is masked off since we need to make sure the TB entry
    // is flushed when we switch to a process in a different session space.
    //

    TempPte.u.Long = ValidKernelPdeLocal.u.Long;
    TempPte.u.Hard.PageFrameNumber = PageDirectoryParentPage;

    PointerPxe = MiGetPxeAddress ((PVOID)MmSessionSpace);

    ASSERT (PointerPxe->u.Long == 0);

    MI_WRITE_VALID_PTE (PointerPxe, TempPte);

    //
    // Do not reference the top level parent page as it belongs to the
    // current process (SMSS).
    //

    MiInitializePfnForOtherProcess (PageDirectoryParentPage, PointerPxe, 0);

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryParentPage);
    Pfn1->u4.PteFrame = 0;

    ASSERT (MI_PFN_ELEMENT(PageDirectoryParentPage)->u1.WsIndex == 0);

    //
    // Initialize the page directory page.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

    PageDirectoryPage = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

    //
    // The global bit is masked off since we need to make sure the TB entry
    // is flushed when we switch to a process in a different session space.
    //

    TempPte.u.Long = ValidKernelPdeLocal.u.Long;
    TempPte.u.Hard.PageFrameNumber = PageDirectoryPage;

    PointerPpe = MiGetPpeAddress ((PVOID)MmSessionSpace);

    ASSERT (PointerPpe->u.Long == 0);

    MI_WRITE_VALID_PTE (PointerPpe, TempPte);

#if defined (_WIN64)

    //
    // IA64 can reference the top level parent page here because a unique
    // one is allocated per process.  AMD64 is also ok because there is
    // another hierarchy level above.
    //

    MiInitializePfnForOtherProcess (PageDirectoryPage, PointerPpe, PageDirectoryParentPage);

#else

    //
    // Do not reference the top level parent page as it belongs to the
    // current process (SMSS).
    //

    MiInitializePfnForOtherProcess (PageDirectoryPage, PointerPpe, 0);
    Pfn1 = MI_PFN_ELEMENT (PageDirectoryPage);
    Pfn1->u4.PteFrame = 0;

#endif

    ASSERT (MI_PFN_ELEMENT(PageDirectoryPage)->u1.WsIndex == 0);

#endif

    //
    // Initialize the page table page.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

    PageTablePage = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

    //
    // The global bit is masked off since we need to make sure the TB entry
    // is flushed when we switch to a process in a different session space.
    //

    TempPte.u.Long = ValidKernelPdeLocal.u.Long;
    TempPte.u.Hard.PageFrameNumber = PageTablePage;

    PointerPde = MiGetPdeAddress ((PVOID)MmSessionSpace);

    ASSERT (PointerPde->u.Long == 0);

    MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS >= 3)
    MiInitializePfnForOtherProcess (PageTablePage, PointerPde, PageDirectoryPage);
#else
    //
    // This page frame references itself instead of the current (SMSS.EXE)
    // page directory as its PteFrame.  This allows the current process to
    // appear more normal (at least on 32-bit NT).  It just means we have
    // to treat this page specially during teardown.
    //

    MiInitializePfnForOtherProcess (PageTablePage, PointerPde, PageTablePage);
#endif

    //
    // This page is never paged, ensure that its WsIndex stays clear so the
    // release of the page is handled correctly.
    //

    ASSERT (MI_PFN_ELEMENT(PageTablePage)->u1.WsIndex == 0);





    //
    // The global bit is masked off since we need to make sure the TB entry
    // is flushed when we switch to a process in a different session space.
    //

    TempPte.u.Long = ValidKernelPteLocal.u.Long;

    PointerPte = MiGetPteAddress (MmSessionSpace);

    for (i = 0; i < MiSessionDataPages; i += 1) {

        TempPte.u.Hard.PageFrameNumber = DataPage[i];

        MiInitializePfnAndMakePteValid (DataPage[i], PointerPte + i, TempPte);

        ASSERT (MI_PFN_ELEMENT(DataPage[i])->u1.WsIndex == 0);
    }

    //
    // Allocate pages for session pool tag information and
    // large session pool allocation tracking.
    //

    for (i = 0; i < MiSessionTagPages; i += 1) {

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, OldIrql);
        }

        PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

        TagPage[i] = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

        TempPte.u.Hard.PageFrameNumber = TagPage[i];

        MiInitializePfnAndMakePteValid (TagPage[i], PointerPte + MiSessionDataPages + i, TempPte);
    }

    //
    // Now that the data page is mapped, it can be freed as full hierarchy
    // teardown in the event of any future failures encountered by this routine.
    //

    UNLOCK_PFN (OldIrql);

    //
    // Initialize the new session space data structure.
    //

    SessionSpace = MmSessionSpace;

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGETABLE_ALLOC, 1);
    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGE_ALLOC, 1);

    SessionSpace->GlobalVirtualAddress = SessionGlobal;
    SessionSpace->ReferenceCount = 1;
    SessionSpace->ResidentProcessCount = 1;
    SessionSpace->u.LongFlags = 0;
    SessionSpace->SessionId = *SessionId;
    SessionSpace->LocaleId = PsDefaultSystemLocaleId;
    SessionSpace->SessionPageDirectoryIndex = PageTablePage;

    SessionSpace->Color = PageColor;

    //
    // Track the page table page and the data page.
    //

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_SESSION_CREATE, (ULONG)ResidentPages);
    SessionSpace->NonPageablePages = ResidentPages;
    SessionSpace->CommittedPages = ResidentPages;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Initialize the session data page directory entry so trimmers can attach.
    //

#if defined(_AMD64_)
    PointerPpe = MiGetPxeAddress ((PVOID)MmSessionSpace);
#else
    PointerPpe = MiGetPpeAddress ((PVOID)MmSessionSpace);
#endif
    SessionSpace->PageDirectory = *PointerPpe;

#else

    SessionSpace->PageTables = PageTables;

    //
    // Load the session data page table entry so that other processes
    // can fault in the mapping.
    //

    SessionSpace->PageTables[PointerPde - MiGetPdeAddress (MmSessionBase)] = *PointerPde;

#endif

    //
    // This list entry is only referenced while within the
    // session space and has session space (not global) addresses.
    //

    InitializeListHead (&SessionSpace->ImageList);

    //
    // Initialize the session space pool.
    //

    Status = MiInitializeSessionPool ();

    if (!NT_SUCCESS(Status)) {
        goto Failure;
    }

    PoolInitialized = TRUE;

    //
    // Initialize the view mapping support - note this must happen after
    // initializing session pool.
    //

    if (MiInitializeSystemSpaceMap (&SessionGlobal->Session) == FALSE) {
        goto Failure;
    }

    MmUnlockPageableImageSection (ExPageLockHandle);

    //
    // Use the global virtual address rather than the session space virtual
    // address to set up fields that need to be globally accessible.
    //

    ASSERT (SessionGlobal->WsListEntry.Flink == NULL);
    ASSERT (SessionGlobal->WsListEntry.Blink == NULL);

    InitializeListHead (&SessionGlobal->ProcessList);

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_CREATING_SESSION);
    PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATING_SESSION);

    ASSERT (Process->Session == NULL);

    ASSERT (SessionGlobal->ProcessReferenceToSession == 0);
    SessionGlobal->ProcessReferenceToSession = 1;

    InterlockedIncrement (&MmSessionDataPages);

    return STATUS_SUCCESS;

Failure:

#if (_MI_PAGING_LEVELS < 3)
    ExFreePool (PageTables);
#endif

    if (GotCommit == TRUE) {
        MiReturnCommitment (ResidentPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SESSION_CREATE_FAILURE,
                         ResidentPages);
    }

    if (GotPages == TRUE) {

#if (_MI_PAGING_LEVELS >= 4)
        PointerPxe = MiGetPxeAddress ((PVOID)MmSessionSpace);
        ASSERT (PointerPxe->u.Hard.Valid != 0);
#endif

#if (_MI_PAGING_LEVELS >= 3)
        PointerPpe = MiGetPpeAddress ((PVOID)MmSessionSpace);
        ASSERT (PointerPpe->u.Hard.Valid != 0);
#endif

        PointerPde = MiGetPdeAddress (MmSessionSpace);
        ASSERT (PointerPde->u.Hard.Valid != 0);

        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGE_FREE_FAIL1, 1);

        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_FAIL1, 1);


        //
        // Do not call MiFreeSessionSpaceMap () as the maps cannot have been
        // initialized if we are in this path.
        //

        //
        // Free the initial page table page that was allocated for the
        // paged pool range (if it has been allocated at this point).
        //

        MiFreeSessionPoolBitMaps ();

        //
        // Capture all needed session information now as after sharing
        // is disabled below, no references to session space can be made.
        //

        ASSERT (MiSessionDataPages + MiSessionTagPages != 0);
        MiZeroMemoryPte (PointerPte, MiSessionDataPages + MiSessionTagPages);

        MI_WRITE_ZERO_PTE (PointerPde);
#if defined(_AMD64_)
        MI_WRITE_ZERO_PTE (PointerPpe);
        MI_WRITE_ZERO_PTE (PointerPxe);
#endif

        MI_FLUSH_SESSION_TB ();

        LOCK_PFN (OldIrql);

        //
        // Free the session tag structure pages.
        //

        for (i = 0; i < MiSessionTagPages; i += 1) {
            Pfn1 = MI_PFN_ELEMENT (TagPage[i]);
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

            MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementShareCount (Pfn1, TagPage[i]);
        }

        //
        // Free the session data structure pages.
        //

        for (i = 0; i < MiSessionDataPages; i += 1) {
            Pfn1 = MI_PFN_ELEMENT (DataPage[i]);
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

            MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementShareCount (Pfn1, DataPage[i]);
        }

        //
        // Free the page table page.
        //

        Pfn1 = MI_PFN_ELEMENT (PageTablePage);

#if (_MI_PAGING_LEVELS >= 3)

        if (PoolInitialized == TRUE) {
            ASSERT (Pfn1->u2.ShareCount == 2);
            Pfn1->u2.ShareCount -= 1;
        }
        ASSERT (Pfn1->u2.ShareCount == 1);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

#else

        ASSERT (PageTablePage == Pfn1->u4.PteFrame);

        if (PoolInitialized == TRUE) {
            ASSERT (Pfn1->u2.ShareCount == 3);
            Pfn1->u2.ShareCount -= 2;
        }
        else {
            ASSERT (Pfn1->u2.ShareCount == 2);
            Pfn1->u2.ShareCount -= 1;
        }

#endif

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageTablePage);

#if (_MI_PAGING_LEVELS >= 3)

        //
        // Free the page directory page.
        //

        Pfn1 = MI_PFN_ELEMENT (PageDirectoryPage);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
        ASSERT (Pfn1->u4.PteFrame == PageDirectoryParentPage);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageDirectoryPage);

        //
        // Free the page directory parent page.
        //

        Pfn1 = MI_PFN_ELEMENT (PageDirectoryParentPage);

        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageDirectoryParentPage);

#endif


        UNLOCK_PFN (OldIrql);

        MI_INCREMENT_RESIDENT_AVAILABLE (
            ResidentPages + MI_SESSION_SPACE_WORKING_SET_MINIMUM,
            MM_RESAVAIL_FREE_CREATE_SESSION);
    }

    if (GlobalMappingPte != NULL) {
        MiReleaseSystemPtes (GlobalMappingPte,
                             MiSessionDataPages,
                             SystemPteSpace);
    }

    MmUnlockPageableImageSection (ExPageLockHandle);

    KeAcquireGuardedMutex (&MiSessionIdMutex);

    ASSERT (RtlCheckBit (MiSessionIdBitmap, *SessionId));
    RtlClearBit (MiSessionIdBitmap, *SessionId);

    KeReleaseGuardedMutex (&MiSessionIdMutex);

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_CREATING_SESSION);
    PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_CREATING_SESSION);

    return STATUS_NO_MEMORY;
}

LONG MiSessionLeaderExists;


NTSTATUS
MmSessionCreate (
    OUT PULONG SessionId
    )

/*++

Routine Description:

    Called from NtSetSystemInformation() to create a session space
    in the calling process with the specified SessionId.  An error is returned
    if the calling process already has a session space.

Arguments:

    SessionId - Supplies a pointer to place the resulting session id in.

Return Value:

    Various NTSTATUS error codes.

Environment:

    Kernel mode, no mutexes held.

--*/

{
    ULONG i;
    ULONG SessionLeaderExists;
    PMM_SESSION_SPACE SessionGlobal;
    PETHREAD CurrentThread;
    NTSTATUS Status;
    PEPROCESS CurrentProcess;
#if DBG && (_MI_PAGING_LEVELS < 3)
    PMMPTE StartPde;
    PMMPTE EndPde;
#endif

    CurrentThread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    //
    // A simple check to see if the calling process already has a session space.
    // No need to go through all this if it does.  Creation races are caught
    // below and recovered from regardless.
    //

    if (CurrentProcess->Flags & PS_PROCESS_FLAGS_IN_SESSION) {
        return STATUS_ALREADY_COMMITTED;
    }

    if (CurrentProcess->Vm.Flags.SessionLeader == 0) {

        //
        // Only the session manager can create a session.  Make the current
        // process the session leader if this is the first session creation
        // ever.
        //
        // Make sure the add is only done once as this is called multiple times.
        //
    
        SessionLeaderExists = InterlockedCompareExchange (&MiSessionLeaderExists, 1, 0);
    
        if (SessionLeaderExists != 0) {
            return STATUS_INVALID_SYSTEM_SERVICE;
        }

        MiSessionLeader (CurrentProcess);
    }

    ASSERT (MmIsAddressValid(MmSessionSpace) == FALSE);

#if defined (_AMD64_)
    ASSERT ((MiGetPxeAddress(MmSessionBase))->u.Long == 0);
#endif

#if (_MI_PAGING_LEVELS < 3)

#if DBG
    StartPde = MiGetPdeAddress (MmSessionBase);
    EndPde = MiGetPdeAddress (MiSessionSpaceEnd);

    while (StartPde < EndPde) {
        ASSERT (StartPde->u.Long == 0);
        StartPde += 1;
    }
#endif

#endif

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    Status = MiSessionCreateInternal (SessionId);

    if (!NT_SUCCESS(Status)) {
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        return Status;
    }

    //
    // Add the session space to the working set list.
    //
    // NO SESSION POOL CAN BE ALLOCATED UNTIL THIS COMPLETES.
    //

    Status = MiSessionInitializeWorkingSetList ();

    if (!NT_SUCCESS(Status)) {
        MiDereferenceSession ();
        KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        return Status;
    }

    //
    // Initialize the session paged pool lookaside lists.  Note this cannot
    // be done until both the session pool and working set lists are
    // fully initialized because on IA64, the lookaside list initialization
    // does a pool allocation which will trigger a demand zero fault in 
    // the session pool.
    //
    // THIS IS THE FIRST POOL ALLOCATION IN THIS SESSION SPACE.
    //

    SessionGlobal = SESSION_GLOBAL (MmSessionSpace);

    for (i = 0; i < SESSION_POOL_SMALL_LISTS; i += 1) {

        ExInitializePagedLookasideList ((PPAGED_LOOKASIDE_LIST)&SessionGlobal->Lookaside[i],
                                        NULL,
                                        NULL,
                                        PagedPoolSession,
                                        (i + 1) * sizeof (POOL_BLOCK),
                                        'looP',
                                        256);
    }

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

#if defined (_WIN64)
    MiInitializeSpecialPool (PagedPoolSession);
#endif

    MmSessionSpace->u.Flags.Initialized = 1;

    PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_IN_SESSION);

    ASSERT (MiSessionLeaderExists == 1);

    return Status;
}


NTSTATUS
MmSessionDelete (
    ULONG SessionId
    )

/*++

Routine Description:

    Called from NtSetSystemInformation() to detach from an existing
    session space in the calling process.  An error is returned
    if the calling process has no session space.

Arguments:

    SessionId - Supplies the session id to delete.

Return Value:

    STATUS_SUCCESS on success, STATUS_UNABLE_TO_FREE_VM on failure.

    This process will not be able to access session space anymore upon
    a successful return.  If this is the last process in the session then
    the entire session is torn down.

Environment:

    Kernel mode, no mutexes held.

--*/

{
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    CurrentThread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    //
    // See if the calling process has a session space - this must be
    // checked since we can be called via a system service.
    //

    if ((CurrentProcess->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
            "MmSessionDelete: Process %p not in a session\n",
            CurrentProcess);
        DbgBreakPoint ();
#endif
        return STATUS_UNABLE_TO_FREE_VM;
    }

    if (CurrentProcess->Vm.Flags.SessionLeader == 0) {

        //
        // Only the session manager can delete a session.  This is because
        // it affects the virtual mappings for all threads within the process
        // when this address space is deleted.  This is different from normal
        // VAD clearing because win32k and other drivers rely on this space.
        //

        return STATUS_UNABLE_TO_FREE_VM;
    }

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    if (MmSessionSpace->SessionId != SessionId) {
#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
            "MmSessionDelete: Wrong SessionId! Own %d, Ask %d\n",
            MmSessionSpace->SessionId,
            SessionId);
        DbgBreakPoint();
#endif
        return STATUS_UNABLE_TO_FREE_VM;
    }

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);

    MiDereferenceSession ();

    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

    return STATUS_SUCCESS;
}


VOID
MiAttachSession (
    IN PMM_SESSION_SPACE SessionGlobal
    )

/*++

Routine Description:

    Attaches to the specified session space.

Arguments:

    SessionGlobal - Supplies a pointer to the session to attach to.

Return Value:

    None.

Environment:

    Kernel mode.  No locks held.  Current process must not have a session
    space - ie: the caller should be the system process or smss.exe.

--*/

{
    PMMPTE PointerPde;

    ASSERT ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0);

#if defined (_AMD64_)

    PointerPde = MiGetPxeAddress (MmSessionBase);
    ASSERT (PointerPde->u.Long == 0);
    MI_WRITE_VALID_PTE (PointerPde, SessionGlobal->PageDirectory);

#else

    PointerPde = MiGetPdeAddress (MmSessionBase);

    ASSERT (RtlCompareMemoryUlong (PointerPde, 
                                   MiSessionSpacePageTables * sizeof (MMPTE),
                                   0) == MiSessionSpacePageTables * sizeof (MMPTE));

    RtlCopyMemory (PointerPde,
                   &SessionGlobal->PageTables[0],
                   MiSessionSpacePageTables * sizeof (MMPTE));

#endif
}


VOID
MiDetachSession (
    VOID
    )

/*++

Routine Description:

    Detaches from the specified session space.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.  No locks held.  Current process must not have a session
    space to return to - ie: this should be the system process.

--*/

{
    PMMPTE PointerPde;

    ASSERT ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0);
    ASSERT (MmIsAddressValid(MmSessionSpace) == TRUE);

#if defined (_AMD64_)

    PointerPde = MiGetPxeAddress (MmSessionBase);

    MI_WRITE_ZERO_PTE (PointerPde);

#else

    PointerPde = MiGetPdeAddress (MmSessionBase);

    RtlZeroMemory (PointerPde, MiSessionSpacePageTables * sizeof (MMPTE));

#endif

    MI_FLUSH_SESSION_TB ();
}

#if DBG

VOID
MiCheckSessionVirtualSpace (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    Used to verify that no drivers fail to clean up their session allocations.

Arguments:

    VirtualAddress - Supplies the starting virtual address to check.

    NumberOfBytes - Supplies the number of bytes to check.

Return Value:

    TRUE if all the PTEs have been freed, FALSE if not.

Environment:

    Kernel mode.  APCs disabled.

--*/

{
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPTE StartPte;
    PMMPTE EndPte;
    ULONG Index;

    //
    // Check the specified region.  Everything should have been cleaned up
    // already.
    //

#if defined (_AMD64_)
    ASSERT64 (MiGetPxeAddress (VirtualAddress)->u.Hard.Valid == 1);
#endif

    ASSERT64 (MiGetPpeAddress (VirtualAddress)->u.Hard.Valid == 1);

    StartPde = MiGetPdeAddress (VirtualAddress);
    EndPde = MiGetPdeAddress ((PVOID)((PCHAR)VirtualAddress + NumberOfBytes - 1));

    StartPte = MiGetPteAddress (VirtualAddress);
    EndPte = MiGetPteAddress ((PVOID)((PCHAR)VirtualAddress + NumberOfBytes - 1));

    Index = (ULONG)(StartPde - MiGetPdeAddress ((PVOID)MmSessionBase));

#if (_MI_PAGING_LEVELS >= 3)
    while (StartPde <= EndPde && StartPde->u.Long == 0)
#else
    while (StartPde <= EndPde && MmSessionSpace->PageTables[Index].u.Long == 0)
#endif
    {
        StartPde += 1;
        Index += 1;
        StartPte = MiGetVirtualAddressMappedByPte (StartPde);
    }

    while (StartPte <= EndPte) {

        if (MiIsPteOnPdeBoundary(StartPte)) {

            StartPde = MiGetPteAddress (StartPte);
            Index = (ULONG)(StartPde - MiGetPdeAddress ((PVOID)MmSessionBase));

#if (_MI_PAGING_LEVELS >= 3)
            while (StartPde <= EndPde && StartPde->u.Long == 0)
#else
            while (StartPde <= EndPde && MmSessionSpace->PageTables[Index].u.Long == 0)
#endif
            {
                Index += 1;
                StartPde += 1;
                StartPte = MiGetVirtualAddressMappedByPte (StartPde);
            }
            if (StartPde > EndPde) {
                break;
            }
        }

        if (StartPte->u.Long != 0 && StartPte->u.Long != MM_KERNEL_NOACCESS_PTE) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                "MiCheckSessionVirtualSpace: StartPte 0x%p is still valid! 0x%p, VA 0x%p\n",
                StartPte,
                StartPte->u.Long,
                MiGetVirtualAddressMappedByPte(StartPte));

            DbgBreakPoint ();
        }
        StartPte += 1;
    }
}
#endif


VOID
MiReleaseProcessReferenceToSessionDataPage (
    PMM_SESSION_SPACE SessionGlobal
    )

/*++

Routine Description:

    Decrement this process' session reference.  The session itself may have
    already been deleted.  If this is the last reference to the session,
    then the session data page and its mapping PTE (if any) will be destroyed
    upon return.

Arguments:

    SessionGlobal - Supplies the global session space pointer being
                    dereferenced.  The caller has already verified that this
                    process is a member of the target session.

Return Value:

    None.

Environment:

    Kernel mode, no mutexes held, APCs disabled.

--*/

{
    ULONG i;
    ULONG SessionId;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex[MI_SESSION_DATA_PAGES_MAXIMUM];
    PMMPFN Pfn1;
    KIRQL OldIrql;

    if (InterlockedDecrement (&SessionGlobal->ProcessReferenceToSession) != 0) {
        return;
    }

    SessionId = SessionGlobal->SessionId;

    //
    // Free the datapages & self-map PTE now since this is the last
    // process reference and KeDetach has returned.
    //

#if (_MI_PAGING_LEVELS < 3)
    ExFreePool (SessionGlobal->PageTables);
#endif

    ASSERT (!MI_IS_PHYSICAL_ADDRESS(SessionGlobal));

    PointerPte = MiGetPteAddress (SessionGlobal);

    for (i = 0; i < MiSessionDataPages; i += 1) {
        PageFrameIndex[i] = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte + i);
    }

    MiReleaseSystemPtes (PointerPte, MiSessionDataPages, SystemPteSpace);

    for (i = 0; i < MiSessionDataPages; i += 1) {
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex[i]);
        MI_SET_PFN_DELETED (Pfn1);
    }

    LOCK_PFN (OldIrql);

    //
    // Get rid of the session data pages.
    //

    for (i = 0; i < MiSessionDataPages; i += 1) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex[i]);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 1);

        MiDecrementShareCount (Pfn1, PageFrameIndex[i]);
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (MiSessionDataPages,
                                     MM_RESAVAIL_FREE_DEREFERENCE_SESSION);

    InterlockedDecrement (&MmSessionDataPages);

    //
    // Return commitment for the datapages.
    //

    MiReturnCommitment (MiSessionDataPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SESSION_DATAPAGE,
                     MiSessionDataPages);

    //
    // Release the session ID so it can be recycled.
    //

    KeAcquireGuardedMutex (&MiSessionIdMutex);

    ASSERT (RtlCheckBit (MiSessionIdBitmap, SessionId));
    RtlClearBit (MiSessionIdBitmap, SessionId);

    KeReleaseGuardedMutex (&MiSessionIdMutex);
}


VOID
MiDereferenceSession (
    VOID
    )

/*++

Routine Description:

    Decrement this process' reference count to the session, unmapping access
    to the session for the current process.  If this is the last process
    reference to this session, then the entire session will be destroyed upon
    return.  This includes unloading drivers, unmapping pools, freeing
    page tables, etc.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, no mutexes held, APCs disabled.

--*/

{
    PMMPTE StartPde;

    ULONG SessionId;
    PEPROCESS Process;
    PMM_SESSION_SPACE SessionGlobal;

    ASSERT ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) ||
            ((MmSessionSpace->u.Flags.Initialized == 0) && (PsGetCurrentProcess()->Vm.Flags.SessionLeader == 1) && (MmSessionSpace->ReferenceCount == 1)));

    SessionId = MmSessionSpace->SessionId;

    ASSERT (RtlCheckBit (MiSessionIdBitmap, SessionId));

    InterlockedDecrement (&MmSessionSpace->ResidentProcessCount);

    if (InterlockedDecrement ((PLONG)&MmSessionSpace->ReferenceCount) != 0) {

        Process = PsGetCurrentProcess ();

        PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_IN_SESSION);

        //
        // Don't delete any non-smss session space mappings here.  Let them
        // live on through process death.  This handles the case where
        // MmDispatchWin32Callout picks csrss - csrss has exited as it's not
        // the last process (smss is).  smss is simultaneously detaching from
        // the session and since it is the last process, it's waiting on
        // the AttachCount below.  The dispatch callout ends up in csrss but
        // has no way to synchronize against csrss exiting through this path
        // as the object reference count doesn't stop it.  So leave the
        // session space mappings alive so the callout can execute through
        // the remains of csrss.
        //
        // Note that when smss detaches, the address space must get cleared
        // here so that subsequent session creations by smss will succeed.
        //

        if (Process->Vm.Flags.SessionLeader == 1) {

            SessionGlobal = SESSION_GLOBAL (MmSessionSpace);

#if defined (_AMD64_)

            StartPde = MiGetPxeAddress (MmSessionBase);
            StartPde->u.Long = 0;

#else

            StartPde = MiGetPdeAddress (MmSessionBase);
            RtlZeroMemory (StartPde, MiSessionSpacePageTables * sizeof(MMPTE));

#endif

            MI_FLUSH_SESSION_TB ();
    
            //
            // This process' reference to the session must be NULL as the
            // KeDetach has completed so no swap context referencing the
            // earlier session page can occur from here on.  This is also
            // needed because during clean shutdowns, the final dereference
            // of this process (smss) object will trigger an
            // MmDeleteProcessAddressSpace - this routine will dereference
            // the (no-longer existing) session space if this
            // bit is not cleared properly.
            //

            ASSERT (Process->Session == NULL);

            //
            // Another process may have won the race and exited the session
            // as this process is executing here.  Hence the reference count
            // is carefully checked here to ensure no leaks occur.
            //

            MiReleaseProcessReferenceToSessionDataPage (SessionGlobal);
        }
        return;
    }

    //
    // This is the final process in the session so the entire session must
    // be dereferenced now.
    //

    MiDereferenceSessionFinal ();
}


VOID
MiDereferenceSessionFinal (
    VOID
    )

/*++

Routine Description:

    Decrement this process' reference count to the session, unmapping access
    to the session for the current process.  If this is the last process
    reference to this session, then the entire session will be destroyed upon
    return.  This includes unloading drivers, unmapping pools, freeing
    page tables, etc.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, no mutexes held, APCs disabled.

--*/

{
    ULONG i;
    KIRQL OldIrql;
    ULONG Index;
    ULONG_PTR CountReleased;
    ULONG_PTR CountReleased2;
    ULONG SessionId;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrame;
    KEVENT Event;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PointerPte;
    PMMPTE EndPte;
    PMMPTE GlobalPteEntrySave;
    PMMPTE StartPde;
    PMM_SESSION_SPACE SessionGlobal;
    ULONG AttachCount;
    PEPROCESS Process;
    PKTHREAD CurrentThread;
    PMMPFN DataFramePfn[MI_SESSION_DATA_PAGES_MAXIMUM];
    PMMPTE PointerPde;
#if (_MI_PAGING_LEVELS >= 3)
    PFN_NUMBER PageDirectoryFrame;
    PFN_NUMBER PageParentFrame;
#endif

    Process = PsGetCurrentProcess();

    ASSERT ((Process->Flags & PS_PROCESS_FLAGS_IN_SESSION) ||
            ((MmSessionSpace->u.Flags.Initialized == 0) && (Process->Vm.Flags.SessionLeader == 1) && (MmSessionSpace->ReferenceCount == 0)));

    SessionId = MmSessionSpace->SessionId;

    ASSERT (RtlCheckBit (MiSessionIdBitmap, SessionId));

    //
    // This is the final dereference.  We could be any process
    // including SMSS when a session space load fails.  Note also that
    // processes can terminate in any order as well.
    //

    SessionGlobal = SESSION_GLOBAL (MmSessionSpace);

    MmLockPageableSectionByHandle (ExPageLockHandle);

    LOCK_EXPANSION (OldIrql);

    //
    // Wait for any cross-session process attaches to detach.  Refuse
    // subsequent attempts to cross-session attach so the address invalidation
    // code doesn't surprise an ongoing or subsequent attachee.
    //

    ASSERT (MmSessionSpace->u.Flags.DeletePending == 0);

    MmSessionSpace->u.Flags.DeletePending = 1;

    AttachCount = MmSessionSpace->AttachCount;

    if (AttachCount) {

        KeInitializeEvent (&SessionGlobal->AttachEvent,
                           NotificationEvent,
                           FALSE);

        UNLOCK_EXPANSION (OldIrql);

        KeWaitForSingleObject (&SessionGlobal->AttachEvent,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        LOCK_EXPANSION (OldIrql);

        ASSERT (MmSessionSpace->u.Flags.DeletePending == 1);
        ASSERT (MmSessionSpace->AttachCount == 0);
    }

    if (MmSessionSpace->Vm.WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING) {

        //
        // Initialize an event and put the event address
        // in the VmSupport.  When the trimming is complete,
        // this event will be set.
        //

        KeInitializeEvent (&Event, NotificationEvent, FALSE);

        MmSessionSpace->Vm.WorkingSetExpansionLinks.Blink = (PLIST_ENTRY)&Event;

        //
        // Release the mutex and wait for the event.
        //

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);

        UNLOCK_EXPANSION (OldIrql);

        KeWaitForSingleObject (&Event,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        KeLeaveCriticalRegionThread (CurrentThread);

        LOCK_EXPANSION (OldIrql);
        ASSERT (Process->Vm.WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED);
    }
    else if (MmSessionSpace->Vm.WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED) {
        //
        // This process' working set is in an initialization state and has
        // never been inserted into any lists.
        //

        NOTHING;
    }
    else {

        //
        // Remove this session from the working set list.
        //

        RemoveEntryList (&SessionGlobal->Vm.WorkingSetExpansionLinks);

        SessionGlobal->Vm.WorkingSetExpansionLinks.Flink = MM_WS_NOT_LISTED;
    }

    if (SessionGlobal->WsListEntry.Flink != NULL) {
        RemoveEntryList (&SessionGlobal->WsListEntry);
    }

    UNLOCK_EXPANSION (OldIrql);

#if DBG
    if (Process->Vm.Flags.SessionLeader == 0) {
        ASSERT (MmSessionSpace->ResidentProcessCount == 0);
        ASSERT (MmSessionSpace->ReferenceCount == 0);
    }
#endif

    MM_SNAP_SESS_MEMORY_COUNTERS(0);

    //
    // If an unload function has been registered for WIN32K.SYS,
    // call it now before we force an unload on any modules.  WIN32K.SYS
    // is responsible for calling any other loaded modules that have
    // unload routines to be run.  Another option is to have the other
    // session drivers register a DLL initialize/uninitialize pair on load.
    //

    if (MmSessionSpace->Win32KDriverUnload) {

        //
        // Note win32k does not reference the argument driver object so just
        // pass in NULL.
        //

        MmSessionSpace->Win32KDriverUnload (NULL);
    }

    //
    // Delete the session paged pool lookaside lists.
    //

    if (MmSessionSpace->u.Flags.Initialized) {

        for (i = 0; i < SESSION_POOL_SMALL_LISTS; i += 1) {
            ExDrainPoolLookasideList ((PPAGED_LOOKASIDE_LIST)&SessionGlobal->Lookaside[i]);
            ExDeletePagedLookasideList ((PPAGED_LOOKASIDE_LIST)&SessionGlobal->Lookaside[i]);
        }
    }

    //
    // Complete all deferred pool block deallocations.
    //

    ExDeferredFreePool (&MmSessionSpace->PagedPool);

    //
    // Now that all modules have had their unload routine(s)
    // called, check for pool leaks before unloading the images.
    //

    MiCheckSessionPoolAllocations ();

    ASSERT (MmSessionSpace->ReferenceCount == 0);

#if defined (_WIN64)
    if (MmSessionSpecialPoolStart != 0) {
        MiDeleteSessionSpecialPool ();
    }
#endif

    PointerPte = MiGetPteAddress (MmSessionSpace);

    LOCK_PFN (OldIrql);

    //
    // Free the physical frames for pool tracking & tagging now.
    //

    for (i = 0; i < MiSessionTagPages; i += 1) {
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte + MiSessionDataPages + i);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);
    }

    UNLOCK_PFN (OldIrql);

    //
    // Track the resident available and commit for return later in this routine.
    //

    CountReleased = MiSessionTagPages;

    MmSessionSpace->NonPageablePages -= MiSessionTagPages;
    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 0 - (SIZE_T)MiSessionTagPages);

    MM_SNAP_SESS_MEMORY_COUNTERS(1);

    //
    // Destroy the view mapping structures.
    //

    MiFreeSessionSpaceMap ();

    MM_SNAP_SESS_MEMORY_COUNTERS(2);

    //
    // Walk down the list of modules we have loaded dereferencing them.
    //
    // This allows us to force an unload of any kernel images loaded by
    // the session so we do not have any virtual space and paging
    // file leaks.
    //

    MiSessionUnloadAllImages ();

    MM_SNAP_SESS_MEMORY_COUNTERS(3);

    //
    // Destroy the session space bitmap structure
    //

    MiFreeSessionPoolBitMaps ();

    MM_SNAP_SESS_MEMORY_COUNTERS(4);

    //
    // Reference the session space structure using its global
    // kernel PTE based address.  This is to avoid deleting it out
    // from underneath ourselves.
    //

    GlobalPteEntrySave = MiGetPteAddress (SessionGlobal);

    //
    // Sweep the individual regions in their proper order.
    //

#if DBG

    //
    // Check the executable image region. All images
    // should have been unloaded by the image handler.
    //

    MiCheckSessionVirtualSpace ((PVOID) MiSessionImageStart, MmSessionImageSize);
#endif

    MM_SNAP_SESS_MEMORY_COUNTERS(5);

#if DBG

    //
    // Check the view region. All views should have been cleaned up already.
    //

    MiCheckSessionVirtualSpace ((PVOID) MiSessionViewStart, MmSessionViewSize);
#endif

    MM_SNAP_SESS_MEMORY_COUNTERS(6);

#if DBG
    //
    // Check everything possible before the remaining virtual address space
    // is torn down.  In this way if anything is amiss, the data can be
    // more easily examined.
    //

    Pfn1 = MI_PFN_ELEMENT (MmSessionSpace->SessionPageDirectoryIndex);

    //
    // This should be greater than 1 because working set page tables are
    // using this as their parent as well.
    //

    ASSERT (Pfn1->u2.ShareCount > 1);
#endif

    if (MmSessionSpace->u.Flags.Initialized == 1) {

        PointerPte = MiGetPteAddress ((PVOID)MiSessionSpaceWs);
#if (_MI_PAGING_LEVELS >= 3)
        PointerPde = MiGetPdeAddress ((PVOID)MiSessionSpaceWs);
#endif
        EndPte = MiGetPteAddress (MmSessionSpace->Vm.VmWorkingSetList->HighestPermittedHashAddress);

        CountReleased2 = 0;

        while (PointerPte < EndPte) {

            if (PointerPte->u.Long) {

                ASSERT (PointerPte->u.Hard.Valid == 1);

                //
                // Track the resident available and commit for return later
                // in this routine.
                //

                CountReleased2 += 1;
            }

            PointerPte += 1;

#if (_MI_PAGING_LEVELS >= 3)

            //
            // The PXE and PPE must be valid because all of session space is
            // contained within a single PPE.  However, each PDE must be
            // checked for validity.
            //

            if (MiIsPteOnPdeBoundary (PointerPte)) {
                ASSERT (PointerPde == MiGetPteAddress (PointerPte - 1));
                PointerPde += 1;
                while (PointerPde->u.Hard.Valid == 0) {
                    PointerPde += 1;
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    if (PointerPte >= EndPte) {
                        break;
                    }
                }
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            }
#endif

        }

        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_PAGE_FREE, (ULONG) CountReleased2);

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CountReleased2);

        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_WS_PAGE_FREE, (ULONG) CountReleased2);
        MmSessionSpace->NonPageablePages -= CountReleased2;

        CountReleased += CountReleased2;
    }

    //
    // Account for the session data structure data pages in our tracking
    // structures.  The actual data pages and their commitment can only be
    // returned after the last process has been reaped (not just exited).
    //

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGE_FREE, MiSessionDataPages);
    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 0 - (SIZE_T)MiSessionDataPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_SESSION_DESTROY, MiSessionDataPages);
    MmSessionSpace->NonPageablePages -= MiSessionDataPages;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // For NT64, the page directory page and its parent are explicitly
    // accounted for here because only page table pages are checked in
    // the loop after this.
    //

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_INITIAL_PAGE_FREE, 2);
    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, -2);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_SESSION_DESTROY, 2);
    MmSessionSpace->NonPageablePages -= 2;

    CountReleased += 2;

#endif

    //
    // Account for any needed session space page tables.  Note the common
    // structure (not the local PDEs) must be examined as any page tables
    // that were dynamically materialized in the context of a different
    // process may not be in the current process' page directory (ie: the
    // current process has never accessed the materialized VAs) !
    //

#if (_MI_PAGING_LEVELS >= 3)
    StartPde = MiGetPdeAddress ((PVOID)MmSessionBase);
#else
    StartPde = &MmSessionSpace->PageTables[0];
#endif

    CountReleased2 = 0;
    for (Index = 0; Index < MiSessionSpacePageTables; Index += 1) {

        if (StartPde->u.Long != 0) {
            CountReleased2 += 1;
        }

        StartPde += 1;
    }

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_PAGETABLE_FREE, (ULONG) CountReleased2);
    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                 0 - CountReleased2);
    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_SESSION_PTDESTROY, (ULONG) CountReleased2);
    MmSessionSpace->NonPageablePages -= CountReleased2;
    CountReleased += CountReleased2;

    ASSERT (MmSessionSpace->NonPageablePages == 0);

    //
    // Note that whenever win32k or drivers loaded by it leak pool, the
    // ASSERT below will be triggered.
    //

    ASSERT (MmSessionSpace->CommittedPages == 0);

    MiReturnCommitment (CountReleased);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_SESSION_DEREFERENCE, CountReleased);

    //
    // Sweep the working set entries.
    // No more accesses to the working set or its lock are allowed.
    //

    if (MmSessionSpace->u.Flags.Initialized == 1) {

        PointerPte = MiGetPteAddress ((PVOID)MiSessionSpaceWs);
        EndPte = MiGetPteAddress (MmSessionSpace->Vm.VmWorkingSetList->HighestPermittedHashAddress);

#if (_MI_PAGING_LEVELS >= 3)
        PointerPde = MiGetPdeAddress ((PVOID)MiSessionSpaceWs);
#endif

        while (PointerPte < EndPte) {

            if (PointerPte->u.Long) {

                ASSERT (PointerPte->u.Hard.Valid == 1);

                //
                // Delete the page.
                //

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                //
                // Each page should still be locked in the session working set.
                //

                LOCK_PFN (OldIrql);

                ASSERT (Pfn1->u3.e2.ReferenceCount == 1);

                MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

                MI_SET_PFN_DELETED (Pfn1);
                MiDecrementShareCount (Pfn1, PageFrameIndex);

                //
                // Don't return the resident available pages charge here
                // as it's going to be returned in one chunk below as part of
                // CountReleased.
                //

                UNLOCK_PFN (OldIrql);

                MI_WRITE_ZERO_PTE (PointerPte);
            }

            PointerPte += 1;

#if (_MI_PAGING_LEVELS >= 3)

            //
            // The PXE and PPE must be valid because all of session space is
            // contained within a single PPE.  However, each PDE must be
            // checked for validity.
            //

            if (MiIsPteOnPdeBoundary (PointerPte)) {
                ASSERT (PointerPde == MiGetPteAddress (PointerPte - 1));
                PointerPde += 1;
                while (PointerPde->u.Hard.Valid == 0) {
                    PointerPde += 1;
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    if (PointerPte >= EndPte) {
                        break;
                    }
                }
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            }
#endif

        }
    }

    ASSERT (!MI_IS_PHYSICAL_ADDRESS(SessionGlobal));

    for (i = 0; i < MiSessionDataPages; i += 1) {
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (GlobalPteEntrySave + i);
        DataFramePfn[i] = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (DataFramePfn[i]->u4.PteFrame == MmSessionSpace->SessionPageDirectoryIndex);

        //
        // Make sure the data pages are still locked.
        //

        ASSERT (DataFramePfn[i]->u1.WsIndex == 0);
        ASSERT (DataFramePfn[i]->u3.e2.ReferenceCount == 1);
    }

    //
    // Save the local page table index so it can be decremented safely below.
    //

    PageTableFrame = DataFramePfn[0]->u4.PteFrame;

#if (_MI_PAGING_LEVELS >= 3)

    PageDirectoryFrame = MI_PFN_ELEMENT(PageTableFrame)->u4.PteFrame;
    PageParentFrame = MI_PFN_ELEMENT(PageDirectoryFrame)->u4.PteFrame;

    for (i = 1; i < MiSessionDataPages; i += 1) {
        ASSERT (PageDirectoryFrame == MI_PFN_ELEMENT(DataFramePfn[i]->u4.PteFrame)->u4.PteFrame);
    }

#endif

    LOCK_PFN (OldIrql);

    //
    // Release the data page claims to their page table page.
    //

    for (i = 0; i < MiSessionDataPages; i += 1) {
        ASSERT (DataFramePfn[i]->u4.PteFrame == PageTableFrame);
        MiDecrementShareCount (MI_PFN_ELEMENT (PageTableFrame),
                               PageTableFrame);
    }

    //
    // Delete the VA space - no more accesses to MmSessionSpace at this point.
    //
    // Cut off the pagetable reference as the local page table is going to be
    // freed now.  Any needed references must go through the global PTE
    // space or superpages but never through the local session VA.  The
    // actual session data pages are not freed until the very last process
    // of the session receives its very last object dereference.
    //
    // Delete page table pages first.
    //

#if (_MI_PAGING_LEVELS >= 3)
    PointerPde = MiGetPdeAddress ((PVOID)MmSessionBase);
#else
    PointerPde = &SessionGlobal->PageTables[0];
#endif

    for (Index = 0; Index < MiSessionSpacePageTables; Index += 1, PointerPde += 1) {

        if (PointerPde->u.Long == 0) {
            continue;
        }
    
        ASSERT (PointerPde->u.Hard.Valid == 1);
    
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
        PageTableFrame = Pfn1->u4.PteFrame;

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);

        //
        // Do the page table page *AFTER* the data page because for 32-bit NT,
        // all the page session space page tables use the session data page's
        // page table as the containing frame (to avoid referencing a top level
        // user page directory page in smss).  However, the MiIdentifyPfn
        // logging code gets confused if we delete a page that has an
        // already-freed containing frame, hence the ordering.
        //

        Pfn2 = MI_PFN_ELEMENT (PageTableFrame);
        MiDecrementShareCount (Pfn2, PageTableFrame);

        PointerPde->u.Long = 0;
    }

#if defined (_AMD64_)
    StartPde = MiGetPxeAddress (MmSessionBase);
    StartPde->u.Long = 0;
#endif

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Delete the session page directory page.
    //

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryFrame);
    MI_SET_PFN_DELETED (Pfn1);
    MiDecrementShareCount (Pfn1, PageDirectoryFrame);

    //
    // Delete the session page directory parent page.
    //

    Pfn1 = MI_PFN_ELEMENT (PageParentFrame);
    MI_SET_PFN_DELETED (Pfn1);
    MiDecrementShareCount (Pfn1, PageParentFrame);

    //
    // The page directory still has a share count with the page directory
    // parent so decrement that now so the parent can be freed.
    //

    MiDecrementShareCount (Pfn1, PageParentFrame);

#else

    StartPde = MiGetPdeAddress ((PVOID)MmSessionBase);
    MiZeroMemoryPte (StartPde, MiSessionSpacePageTables);

#endif

    UNLOCK_PFN (OldIrql);

    //
    // Return resident available for the pool tagging & tracking tables as
    // well as page table pages and working set structure pages.
    //

    MI_INCREMENT_RESIDENT_AVAILABLE (CountReleased,
                                     MM_RESAVAIL_FREE_DEREFERENCE_SESSION_PAGES);

    MI_INCREMENT_RESIDENT_AVAILABLE (MI_SESSION_SPACE_WORKING_SET_MINIMUM,
                                     MM_RESAVAIL_FREE_DEREFERENCE_SESSION_WS);

    //
    // Flush the session space TB entries.
    //

    MI_FLUSH_SESSION_TB ();

    PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_IN_SESSION);

    //
    // The session space has been deleted and all TB flushing is complete.
    //

    MmUnlockPageableImageSection (ExPageLockHandle);

    //
    // The session leader must release the reference here since the
    // session leader never exits and thus doesn't trigger this via
    // process delete.
    //

    if (Process->Vm.Flags.SessionLeader == 1) {
        ASSERT (Process->Session == NULL);
        MiReleaseProcessReferenceToSessionDataPage (SessionGlobal);
    }

    return;
}

NTSTATUS
MiSessionCommitPageTables (
    IN PVOID StartVa,
    IN PVOID EndVa
    )

/*++

Routine Description:

    Fill in page tables covering the specified virtual address range.

Arguments:

    StartVa - Supplies a starting virtual address.

    EndVa - Supplies an ending virtual address.

Return Value:

    STATUS_SUCCESS on success, STATUS_NO_MEMORY on failure.

Environment:

    Kernel mode.  Session space working set mutex NOT held.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

--*/

{
    ULONG Waited;
    KIRQL OldIrql;
    ULONG Color;
    ULONG Index;
    PMMPTE StartPde;
    PMMPTE EndPde;
    MMPTE TempPte;
    PMMPFN Pfn1;
    WSLE_NUMBER SwapEntry;
    WSLE_NUMBER WorkingSetIndex;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER ActualPages;
    PFN_NUMBER PageTablePage;
    PVOID SessionPte;
    PMMWSL WorkingSetList;
    PETHREAD CurrentThread;
    NTSTATUS Status;
    PMMSUPPORT Ws;
    
    ASSERT (StartVa >= (PVOID)MmSessionBase);
    ASSERT (EndVa < (PVOID)MiSessionSpaceEnd);
    ASSERT (PAGE_ALIGN (EndVa) == EndVa);

    //
    // Allocate the page table pages, loading them
    // into the current process's page directory.
    //

    StartPde = MiGetPdeAddress (StartVa);
    EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)EndVa - 1));
    Index = MiGetPdeSessionIndex (StartVa);
    SizeInPages = 0;

    while (StartPde <= EndPde) {
#if (_MI_PAGING_LEVELS >= 3)
        if (StartPde->u.Long == 0)
#else
        if (MmSessionSpace->PageTables[Index].u.Long == 0)
#endif
        {
            SizeInPages += 1;
        }
        StartPde += 1;
        Index += 1;
    }

    if (SizeInPages == 0) {
        return STATUS_SUCCESS;
    }

    if (MiChargeCommitment (SizeInPages, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        return STATUS_NO_MEMORY;
    }

    LOCK_PFN (OldIrql);

    //
    // Check to make sure the physical pages are available.
    //

    if ((SPFN_NUMBER)SizeInPages > MI_NONPAGEABLE_MEMORY_AVAILABLE() - 20) {
        UNLOCK_PFN (OldIrql);
        MiReturnCommitment (SizeInPages);
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_RESIDENT);
        return STATUS_NO_MEMORY;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_SESSION_PAGE_TABLES);

    UNLOCK_PFN (OldIrql);



    CurrentThread = PsGetCurrentThread ();
    ActualPages = 0;
    TempPte = ValidKernelPdeLocal;
    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
    WorkingSetList = MmSessionSpace->Vm.VmWorkingSetList;
    StartPde = MiGetPdeAddress (StartVa);
    Index = MiGetPdeSessionIndex (StartVa);
    Status = STATUS_SUCCESS;



    LOCK_WORKING_SET (CurrentThread, Ws);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_PAGETABLE_PAGES, SizeInPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_PAGETABLE_ALLOC, (ULONG)SizeInPages);

    while (StartPde <= EndPde) {

#if (_MI_PAGING_LEVELS >= 3)
        if (StartPde->u.Long == 0)
#else
        if (MmSessionSpace->PageTables[Index].u.Long == 0)
#endif
        {

            ASSERT (StartPde->u.Hard.Valid == 0);

            LOCK_PFN (OldIrql);

            Waited = MiEnsureAvailablePageOrWait (HYDRA_PROCESS, OldIrql);

            if (Waited != 0) {

                //
                // The session space working set mutex & PFN lock were
                // released and reacquired so recheck the master page
                // directory as another thread may have filled this entry
                // in the interim.
                //

                UNLOCK_PFN (OldIrql);
                continue;
            }

            Color = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);

            PageTablePage = MiRemoveZeroPage (Color);

            TempPte.u.Hard.PageFrameNumber = PageTablePage;
            MI_WRITE_VALID_PTE (StartPde, TempPte);

#if (_MI_PAGING_LEVELS < 3)
            ASSERT (MmSessionSpace->PageTables[Index].u.Long == 0);
            MmSessionSpace->PageTables[Index] = TempPte;
#endif
            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_COMMIT_IMAGE_PT, 1);

            MiInitializePfnForOtherProcess (PageTablePage,
                                            StartPde,
                                            MmSessionSpace->SessionPageDirectoryIndex);
            UNLOCK_PFN (OldIrql);

            Pfn1 = MI_PFN_ELEMENT (PageTablePage);

            ASSERT (Pfn1->u1.Event == NULL);

            SessionPte = MiGetVirtualAddressMappedByPte (StartPde);

            WorkingSetIndex = MiAddValidPageToWorkingSet (SessionPte,
                                                          StartPde,
                                                          Pfn1,
                                                          0);

            if (WorkingSetIndex == 0) {

                //
                // A working set entry could not be allocated.  Just delete
                // the page table we just allocated as no one else could
                // be using it (as we have held the session's working set mutex
                // since initializing the PTE) and return a failure.
                //

                ASSERT (Pfn1->u3.e1.PrototypePte == 0);

                LOCK_PFN (OldIrql);
                MI_SET_PFN_DELETED (Pfn1);
                UNLOCK_PFN (OldIrql);

                MiTrimPte (SessionPte,
                           StartPde,
                           Pfn1,
                           HYDRA_PROCESS,
                           ZeroPte);

#if (_MI_PAGING_LEVELS < 3)

                ASSERT (MmSessionSpace->PageTables[Index].u.Long != 0);
                MmSessionSpace->PageTables[Index].u.Long = 0;
#endif

                Status = STATUS_NO_MEMORY;
                break;
            }

            ActualPages += 1;

            ASSERT (WorkingSetIndex == MiLocateWsle (SessionPte,
                                                     WorkingSetList,
                                                     Pfn1->u1.WsIndex,
                                                     FALSE));

            if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {

                SwapEntry = WorkingSetList->FirstDynamic;

                if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

                    //
                    // Swap this entry with the one at first dynamic.
                    //

                    MiSwapWslEntries (WorkingSetIndex,
                                      SwapEntry,
                                      &MmSessionSpace->Vm,
                                      FALSE);
                }

                WorkingSetList->FirstDynamic += 1;
            }
            else {
                SwapEntry = WorkingSetIndex;
            }

            //
            // Indicate that the page is locked.
            //

            MmSessionSpace->Wsle[SwapEntry].u1.e1.LockedInWs = 1;
        }

        StartPde += 1;
        Index += 1;
    }

    ASSERT (ActualPages <= SizeInPages);

    UNLOCK_WORKING_SET (CurrentThread, Ws);

    if (ActualPages != 0) {

        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPageablePages,
                                     ActualPages);

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     ActualPages);
    }

    //
    // Return any excess commitment and resident available charges.
    //

    if (ActualPages < SizeInPages) {
        MiReturnCommitment (SizeInPages - ActualPages);
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages - ActualPages,
                                   MM_RESAVAIL_FREE_SESSION_PAGE_TABLES_EXCESS);
    }

    return Status;
}

#if DBG
typedef struct _MISWAP {
    ULONG Flag;
    ULONG ResidentProcessCount;
    PEPROCESS Process;
    PMM_SESSION_SPACE Session;
} MISWAP, *PMISWAP;

#define MI_SESSION_SWAP_SIZE 0x100

ULONG MiSessionInfo[4];
MISWAP MiSessionSwap[MI_SESSION_SWAP_SIZE];
LONG  MiSwapIndex;
#endif


VOID
MiSessionOutSwapProcess (
    IN PEPROCESS Process
    )

/*++

Routine Description:

    This routine notifies the containing session that the specified process is
    being outswapped.  When all the processes within a session have been
    outswapped, the containing session undergoes a heavy trim.

Arguments:

    Process - Supplies a pointer to the process that is swapped out of memory.

Return Value:

    None.

Environment:

    Kernel mode.  This routine must not enter a wait state for memory resources
    or the system will deadlock.

--*/

{
    PMM_SESSION_SPACE SessionGlobal;
#if DBG
    LONG SwapIndex;
#endif

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_IN_SESSION);

    //
    // smss doesn't count when we swap it before it has detached from the
    // session it is currently creating.
    //

    if (Process->Vm.Flags.SessionLeader == 1) {
        return;
    }

    //
    // If the process has already exited then it is no longer really a
    // member of the session so don't count it in the outswap counts.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        return;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    ASSERT (SessionGlobal != NULL);
    ASSERT (SessionGlobal->ResidentProcessCount > 0);
    ASSERT (SessionGlobal->ResidentProcessCount <= SessionGlobal->ReferenceCount);

#if DBG
    SwapIndex = InterlockedIncrement (&MiSwapIndex);
    SwapIndex &= (MI_SESSION_SWAP_SIZE - 1);

    MiSessionSwap[SwapIndex].Flag = 1;
    MiSessionSwap[SwapIndex].Process = Process;
    MiSessionSwap[SwapIndex].Session = SessionGlobal;
    MiSessionSwap[SwapIndex].ResidentProcessCount = SessionGlobal->ResidentProcessCount;
#endif

    if (InterlockedDecrement (&SessionGlobal->ResidentProcessCount) == 0) {

#if DBG
        if (MmDebug & MM_DBG_SESSIONS) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
                "Mm: Last process (%d total) just swapped out for session %d, %d pages\n",
                SessionGlobal->ReferenceCount,
                SessionGlobal->SessionId,
                SessionGlobal->Vm.WorkingSetSize);
        }
        MiSessionInfo[0] += 1;
#endif
        KeQuerySystemTime (&SessionGlobal->LastProcessSwappedOutTime);
    }
#if DBG
    else {
        MiSessionInfo[1] += 1;
    }
#endif

    return;
}


VOID
MiSessionInSwapProcess (
    IN PEPROCESS Process,
    IN LOGICAL Forced
    )

/*++

Routine Description:

    This routine in swaps the specified process.

Arguments:

    Process - Supplies a pointer to the process that is to be swapped
              into memory.

    Forced - Supplies TRUE if the process was brought in via KeForceAttach,
             FALSE otherwise.

Return Value:

    None.

Environment:

    Kernel mode.  This routine must not enter a wait state for memory resources
    or the system will deadlock.

--*/

{
    PMM_SESSION_SPACE SessionGlobal;
#if DBG
    LONG SwapIndex;
#else
    UNREFERENCED_PARAMETER (Forced);
#endif

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_IN_SESSION);

    //
    // smss doesn't count when we swap it before it has detached from the
    // session it is currently creating.
    //

    if (Process->Vm.Flags.SessionLeader == 1) {
        return;
    }

    //
    // If the process has already exited then it is no longer really a
    // member of the session so don't count it in the inswap counts.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        return;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;
    ASSERT (SessionGlobal != NULL);

    ASSERT (SessionGlobal->ResidentProcessCount >= 0);
    ASSERT (SessionGlobal->ResidentProcessCount <= SessionGlobal->ReferenceCount);

#if DBG
    SwapIndex = InterlockedIncrement (&MiSwapIndex);
    SwapIndex &= (MI_SESSION_SWAP_SIZE - 1);

    if (Forced == FALSE) {
        MiSessionSwap[SwapIndex].Flag = 2;
    }
    else {
        MiSessionSwap[SwapIndex].Flag = 3;
    }
    MiSessionSwap[SwapIndex].Process = Process;
    MiSessionSwap[SwapIndex].Session = SessionGlobal;
    MiSessionSwap[SwapIndex].ResidentProcessCount = SessionGlobal->ResidentProcessCount;
#endif

    if (InterlockedIncrement (&SessionGlobal->ResidentProcessCount) == 1) {
#if DBG
        MiSessionInfo[2] += 1;
        if (MmDebug & MM_DBG_SESSIONS) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
                "Mm: First process (%d total) just swapped back in for session %d, %d pages\n",
                SessionGlobal->ReferenceCount,
                SessionGlobal->SessionId,
                SessionGlobal->Vm.WorkingSetSize);
        }
#endif
        SessionGlobal->Vm.Flags.TrimHard = 0;
    }
#if DBG
    else {
        MiSessionInfo[3] += 1;
    }
#endif

    return;
}


ULONG
MmGetSessionId (
    IN PEPROCESS Process
    )

/*++

Routine Description:

    This routine returns the session ID of the specified process.

Arguments:

    Process - Supplies a pointer to the process whose session ID is desired.

Return Value:

    The session ID.  Note these are recycled when sessions exit, hence the
    caller must use proper object referencing on the specified process.

Environment:

    Kernel mode.  PASSIVE_LEVEL.

--*/

{
    PMM_SESSION_SPACE SessionGlobal;

    if (Process->Vm.Flags.SessionLeader == 1) {

        //
        // smss may transiently have a session space but that's of no interest
        // to our caller.
        //

        return 0;
    }

    //
    // The Session field of the EPROCESS is never cleared once set so these
    // checks can be done lock free.
    //

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    if (SessionGlobal == NULL) {

        //
        // The system process has no session space.
        //

        return 0;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    return SessionGlobal->SessionId;
}

ULONG
MmGetSessionIdEx (
    IN PEPROCESS Process
    )

/*++

Routine Description:

    This routine returns the session ID of the specified process or -1 if 
    if the process does not belong to any session.

Arguments:

    Process - Supplies a pointer to the process whose session ID is desired.

Return Value:

    The session ID.  Note these are recycled when sessions exit, hence the
    caller must use proper object referencing on the specified process.

Environment:

    Kernel mode.  PASSIVE_LEVEL.

--*/

{
    PMM_SESSION_SPACE SessionGlobal;

    if (Process->Vm.Flags.SessionLeader == 1) {

        //
        // smss may transiently have a session space but that's of no interest
        // to our caller.
        //

        return (ULONG)-1;
    }

    //
    // The Session field of the EPROCESS is never cleared once set so these
    // checks can be done lock free.
    //

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    if (SessionGlobal == NULL) {

        //
        // The system process has no session space.
        //

        return (ULONG)-1;
    }

    SessionGlobal = (PMM_SESSION_SPACE) Process->Session;

    return SessionGlobal->SessionId;
}

PVOID
MmGetNextSession (
    IN PVOID OpaqueSession
    )

/*++

Routine Description:

    This function allows code to enumerate all the sessions in the system.
    The first session (if OpaqueSession is NULL) or subsequent session
    (if session is not NULL) is returned on each call.

    If OpaqueSession is not NULL then this session must have previously
    been obtained by a call to MmGetNextSession.

    Enumeration may be terminated early by calling MmQuitNextSession on
    the last non-NULL session returned by MmGetNextSession.

    Sessions may be referenced in this manner and used later safely.

    For example, to enumerate all sessions in a loop use this code fragment:

    for (OpaqueSession = MmGetNextSession (NULL);
         OpaqueSession != NULL;
         OpaqueSession = MmGetNextSession (OpaqueSession)) {

         ...
         ...

         //
         // Checking for a specific session (if needed) is handled like this:
         //

         if (MmGetSessionId (OpaqueSession) == DesiredId) {

             //
             // Attach to session now to perform operations...
             //

             KAPC_STATE ApcState;

             if (NT_SUCCESS (MmAttachSession (OpaqueSession, &ApcState))) {

                //
                // Session hasn't exited yet, so call interesting work
                // functions that need session context ...
                //

                ...

                //
                // Detach from session.
                //

                MmDetachSession (OpaqueSession, &ApcState);
             }

             //
             // If the interesting work functions failed and error recovery
             // (ie: walk back through all the sessions already operated on
             // and try to undo the actions), then do this.  Note you must add
             // similar checks to the above if the operations were only done
             // to specifically requested session IDs.
             //

             if (ErrorRecoveryNeeded) {

                 for (OpaqueSession = MmGetPreviousSession (OpaqueSession);
                      OpaqueSession != NULL;
                      OpaqueSession = MmGetPreviousSession (OpaqueSession)) {

                      //
                      // MmAttachSession/DetachSession as needed to obtain
                      // context, etc.
                      //
                 }

                 break;
             }

             //
             // Bail if only this session was of interest.
             //

             MmQuitNextSession (OpaqueSession);
             break;
         }

         //
         // Early terminating conditions are handled like this:
         //

         if (NeedToBreakOutEarly) {
             MmQuitNextSession (OpaqueSession);
             break;
         }
    }
    

Arguments:

    OpaqueSession - Supplies the session to get the next session from
                    or NULL for the first session.

Return Value:

    Next session or NULL if no more sessions exist.

--*/

{
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PMM_SESSION_SPACE Session;
    PMM_SESSION_SPACE EntrySession;
    PLIST_ENTRY NextProcessEntry;
    PEPROCESS Process;
    PVOID OpaqueNextSession;
    PEPROCESS EntryProcess;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    OpaqueNextSession = NULL;

    EntryProcess = (PEPROCESS) OpaqueSession;

    if (EntryProcess == NULL) {
        EntrySession = NULL;
    }
    else {
        ASSERT (EntryProcess->Vm.Flags.SessionLeader == 0);

        //
        // The Session field of the EPROCESS is never cleared once set so this
        // field can be used lock free.
        //

        EntrySession = (PMM_SESSION_SPACE) EntryProcess->Session;

        ASSERT (EntrySession != NULL);
    }

    LOCK_EXPANSION (OldIrql);

    if (EntrySession == NULL) {
        NextEntry = MiSessionWsList.Flink;
    }
    else {
        NextEntry = EntrySession->WsListEntry.Flink;
    }

    while (NextEntry != &MiSessionWsList) {

        Session = CONTAINING_RECORD (NextEntry, MM_SESSION_SPACE, WsListEntry);

        NextProcessEntry = Session->ProcessList.Flink;

        if ((Session->u.Flags.DeletePending == 0) &&
            (NextProcessEntry != &Session->ProcessList)) {

            Process = CONTAINING_RECORD (NextProcessEntry,
                                         EPROCESS,
                                         SessionProcessLinks);

            if (Process->Vm.Flags.SessionLeader == 1) {

                //
                // If session manager is still the first process (ie: smss
                // hasn't detached yet), then don't bother delivering to this
                // session this early in its lifetime.  And since smss is
                // serialized, it can't be creating another session yet so
                // just bail now as we must be at the end of the list.
                //

                break;
            }

            //
            // If the process has finished rudimentary initialization, then
            // select it as an attach can be performed safely.  If this first
            // process has not finished initializing there can be no others
            // in this session, so just march on to the next session.
            //
            // Note the VmWorkingSetList is used instead of the
            // AddressSpaceInitialized field because the VmWorkingSetList is
            // never cleared so we can never see an exiting process (whose
            // AddressSpaceInitialized field gets zeroed) and incorrectly
            // decide the list must be empty.
            //

            if (Process->Vm.VmWorkingSetList != NULL) {

                //
                // Reference any process in the session so that the session
                // cannot be completely deleted once the expansion lock is
                // released (note this does NOT prevent the session from being
                // cleaned).
                //

                ObReferenceObject (Process);
                OpaqueNextSession = (PVOID) Process;
                break;
            }
        }
        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION (OldIrql);

    //
    // Regardless of whether a next session is returned, if a starting one
    // was passed in, it must be dereferenced now.
    //

    if (EntryProcess != NULL) {
        ObDereferenceObject (EntryProcess);
    }

    return OpaqueNextSession;
}

PVOID
MmGetPreviousSession (
    IN PVOID OpaqueSession
    )

/*++

Routine Description:

    This function allows code to reverse-enumerate all the sessions in
    the system.  This is typically used for error recovery - ie: to walk
    backwards undoing work done by MmGetNextSession semantics.

    The first session (if OpaqueSession is NULL) or subsequent session
    (if session is not NULL) is returned on each call.

    If OpaqueSession is not NULL then this session must have previously
    been obtained by a call to MmGetNextSession.

Arguments:

    OpaqueSession - Supplies the session to get the next session from
                    or NULL for the first session.

Return Value:

    Next session or NULL if no more sessions exist.

--*/

{
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PMM_SESSION_SPACE Session;
    PMM_SESSION_SPACE EntrySession;
    PLIST_ENTRY NextProcessEntry;
    PEPROCESS Process;
    PVOID OpaqueNextSession;
    PEPROCESS EntryProcess;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    OpaqueNextSession = NULL;

    EntryProcess = (PEPROCESS) OpaqueSession;

    if (EntryProcess == NULL) {
        EntrySession = NULL;
    }
    else {
        ASSERT (EntryProcess->Vm.Flags.SessionLeader == 0);

        //
        // The Session field of the EPROCESS is never cleared once set so this
        // field can be used lock free.
        //

        EntrySession = (PMM_SESSION_SPACE) EntryProcess->Session;

        ASSERT (EntrySession != NULL);
    }

    LOCK_EXPANSION (OldIrql);

    if (EntrySession == NULL) {
        NextEntry = MiSessionWsList.Blink;
    }
    else {
        NextEntry = EntrySession->WsListEntry.Blink;
    }

    while (NextEntry != &MiSessionWsList) {

        Session = CONTAINING_RECORD (NextEntry, MM_SESSION_SPACE, WsListEntry);

        NextProcessEntry = Session->ProcessList.Flink;

        if ((Session->u.Flags.DeletePending == 0) &&
            (NextProcessEntry != &Session->ProcessList)) {

            Process = CONTAINING_RECORD (NextProcessEntry,
                                         EPROCESS,
                                         SessionProcessLinks);

            ASSERT (Process->Vm.Flags.SessionLeader == 0);

            //
            // Reference any process in the session so that the session
            // cannot be completely deleted once the expansion lock is
            // released (note this does NOT prevent the session from being
            // cleaned).
            //

            ObReferenceObject (Process);
            OpaqueNextSession = (PVOID) Process;
            break;
        }
        NextEntry = NextEntry->Blink;
    }

    UNLOCK_EXPANSION (OldIrql);

    //
    // Regardless of whether a next session is returned, if a starting one
    // was passed in, it must be dereferenced now.
    //

    if (EntryProcess != NULL) {
        ObDereferenceObject (EntryProcess);
    }

    return OpaqueNextSession;
}

NTSTATUS
MmQuitNextSession (
    IN PVOID OpaqueSession
    )

/*++

Routine Description:

    This function is used to prematurely terminate a session enumeration
    that began using MmGetNextSession.

Arguments:

    OpaqueSession - Supplies a non-NULL session previously obtained by
                    a call to MmGetNextSession.

Return Value:

    NTSTATUS.

--*/

{
    PEPROCESS EntryProcess;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    EntryProcess = (PEPROCESS) OpaqueSession;

    ASSERT (EntryProcess->Vm.Flags.SessionLeader == 0);

    //
    // The Session field of the EPROCESS is never cleared once set so this
    // field can be used lock free.
    //

    ASSERT (EntryProcess->Session != NULL);

    ObDereferenceObject (EntryProcess);

    return STATUS_SUCCESS;
}

NTSTATUS
MmAttachSession (
    IN PVOID OpaqueSession,
    OUT PRKAPC_STATE ApcState
    )

/*++

Routine Description:

    This function attaches the calling thread to a referenced session
    previously obtained via MmGetNextSession.

Arguments:

    OpaqueSession - Supplies a non-NULL session previously obtained by
                    a call to MmGetNextSession.

    ApcState - Supplies APC state storage for the subsequent detach.

Return Value:

    NTSTATUS.  If successful then we are attached on return.  The caller is
               responsible for calling MmDetachSession when done.

--*/

{
    KIRQL OldIrql;
    PEPROCESS EntryProcess;
    PMM_SESSION_SPACE EntrySession;
    PEPROCESS CurrentProcess;
    PMM_SESSION_SPACE CurrentSession;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    EntryProcess = (PEPROCESS) OpaqueSession;

    ASSERT (EntryProcess->Vm.Flags.SessionLeader == 0);

    //
    // The Session field of the EPROCESS is never cleared once set so this
    // field can be used lock free.
    //

    EntrySession = (PMM_SESSION_SPACE) EntryProcess->Session;

    ASSERT (EntrySession != NULL);

    CurrentProcess = PsGetCurrentProcess ();

    CurrentSession = (PMM_SESSION_SPACE) CurrentProcess->Session;

    LOCK_EXPANSION (OldIrql);

    if (EntrySession->u.Flags.DeletePending == 1) {
        UNLOCK_EXPANSION (OldIrql);
        return STATUS_PROCESS_IS_TERMINATING;
    }

    EntrySession->AttachCount += 1;

    UNLOCK_EXPANSION (OldIrql);

    if ((CurrentProcess->Vm.Flags.SessionLeader == 0) &&
        (CurrentSession != NULL)) {

        //
        // smss may transiently have a session space but that's of
        // no interest to our caller.
        //

        if (CurrentSession == EntrySession) {

            ASSERT (CurrentSession->SessionId == EntrySession->SessionId);

            //
            // The current and target sessions match so an attach is not needed.
            // Call KeStackAttach anyway (this has the overhead of an extra
            // dispatcher lock acquire and release) so that callers can always
            // use MmDetachSession to detach.  This is a very infrequent path so
            // the extra lock acquire and release is not significant.
            //
            // Note that by resetting EntryProcess below, an attach will not
            // actually occur.
            //

            EntryProcess = CurrentProcess;
        }
        else {
            ASSERT (CurrentSession->SessionId != EntrySession->SessionId);
        }
    }

    KeStackAttachProcess (&EntryProcess->Pcb, ApcState);

    return STATUS_SUCCESS;
}

NTSTATUS
MmDetachSession (
    IN PVOID OpaqueSession,
    IN PRKAPC_STATE ApcState
    )
/*++

Routine Description:

    This function detaches the calling thread from the referenced session
    previously attached to via MmAttachSession.

Arguments:

    OpaqueSession - Supplies a non-NULL session previously obtained by
                    a call to MmGetNextSession.

    ApcState - Supplies APC state storage information for the detach.

Return Value:

    NTSTATUS.  If successful then we are detached on return.  The caller is
               responsible for eventually calling MmQuitNextSession on return.

--*/

{
    KIRQL OldIrql;
    PEPROCESS EntryProcess;
    PMM_SESSION_SPACE EntrySession;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    EntryProcess = (PEPROCESS) OpaqueSession;

    ASSERT (EntryProcess->Vm.Flags.SessionLeader == 0);

    //
    // The Session field of the EPROCESS is never cleared once set so this
    // field can be used lock free.
    //

    EntrySession = (PMM_SESSION_SPACE) EntryProcess->Session;

    ASSERT (EntrySession != NULL);

    LOCK_EXPANSION (OldIrql);

    ASSERT (EntrySession->AttachCount >= 1);

    EntrySession->AttachCount -= 1;

    if ((EntrySession->u.Flags.DeletePending == 0) ||
        (EntrySession->AttachCount != 0)) {

        EntrySession = NULL;
    }

    UNLOCK_EXPANSION (OldIrql);

    KeUnstackDetachProcess (ApcState);

    if (EntrySession != NULL) {
        KeSetEvent (&EntrySession->AttachEvent, 0, FALSE);
    }

    return STATUS_SUCCESS;
}

PVOID
MmGetSessionById (
    IN ULONG SessionId
    )

/*++

Routine Description:

    This function allows callers to obtain a reference to a specific session.
    The caller can then MmAttachSession, MmDetachSession & MmQuitNextSession
    to complete the proper sequence so reference counting and address context
    operate properly.

Arguments:

    SessionId - Supplies the session ID of the desired session.

Return Value:

    An opaque session token or NULL if the session cannot be found.

Environment:

    Kernel mode, the caller must guarantee the session cannot exit or the ID
    becomes meaningless as it can be reused.

--*/

{
    KIRQL OldIrql;
    PLIST_ENTRY NextEntry;
    PMM_SESSION_SPACE Session;
    PLIST_ENTRY NextProcessEntry;
    PEPROCESS Process;
    PVOID OpaqueSession;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    OpaqueSession = NULL;

    LOCK_EXPANSION (OldIrql);

    NextEntry = MiSessionWsList.Flink;

    while (NextEntry != &MiSessionWsList) {

        Session = CONTAINING_RECORD (NextEntry, MM_SESSION_SPACE, WsListEntry);

        NextProcessEntry = Session->ProcessList.Flink;

        if (Session->SessionId == SessionId) {

            if ((Session->u.Flags.DeletePending != 0) ||
                (NextProcessEntry == &Session->ProcessList)) {

                //
                // Session is empty or exiting so return failure to the caller.
                //

                break;
            }

            Process = CONTAINING_RECORD (NextProcessEntry,
                                         EPROCESS,
                                         SessionProcessLinks);

            if (Process->Vm.Flags.SessionLeader == 1) {

                //
                // Session manager is still the first process (ie: smss
                // hasn't detached yet), don't bother delivering to this
                // session this early in its lifetime.  And since smss is
                // serialized, it can't be creating another session yet so
                // just bail now as we must be at the end of the list.
                //

                break;
            }

            //
            // Reference any process in the session so that the session
            // cannot be completely deleted once the expansion lock is
            // released (note this does NOT prevent the session from being
            // cleaned).
            //

            ObReferenceObject (Process);
            OpaqueSession = (PVOID) Process;
            break;
        }
        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION (OldIrql);

    return OpaqueSession;
}

