/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   iosup.c

Abstract:

    This module contains routines which provide support for the I/O system.

--*/

#include "mi.h"

#undef MmIsRecursiveIoFault

ULONG MiCacheOverride[4];

#if DBG
ULONG MmShowMapOverlaps;
#endif

extern LONG MmTotalSystemDriverPages;
extern PFN_NUMBER MiStartOfInitialPoolFrame;
extern PFN_NUMBER MiEndOfInitialPoolFrame;

BOOLEAN
MmIsRecursiveIoFault (
    VOID
    );

PVOID
MiAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MEMORY_CACHING_TYPE CacheType,
    PVOID CallingAddress
    );

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     );

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     );

KSPIN_LOCK MmIoTrackerLock;
LIST_ENTRY MmIoHeader;

#if DBG
PFN_NUMBER MmIoHeaderCount;
ULONG MmIoHeaderNumberOfEntries;
ULONG MmIoHeaderNumberOfEntriesPeak;
#endif

PCHAR MiCacheStrings[] = {
    "noncached",
    "cached",
    "writecombined",
    "None"
};

typedef struct _PTE_TRACKER {
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    PFN_NUMBER Count;
    PVOID SystemVa;
    PVOID StartVa;
    ULONG Offset;
    ULONG Length;
    PFN_NUMBER Page;
    struct {
        ULONG IoMapping: 1;
        ULONG Matched: 1;
        ULONG CacheAttribute : 2;
        ULONG Spare : 28;
    };
    PVOID CallingAddress;
    PVOID CallersCaller;
} PTE_TRACKER, *PPTE_TRACKER;

typedef struct _SYSPTES_HEADER {
    LIST_ENTRY ListHead;
    PFN_NUMBER Count;
    PFN_NUMBER NumberOfEntries;
    PFN_NUMBER NumberOfEntriesPeak;
} SYSPTES_HEADER, *PSYSPTES_HEADER;

ULONG MmTrackPtes = 0;
BOOLEAN MiTrackPtesAborted = FALSE;
SYSPTES_HEADER MiPteHeader;
SLIST_HEADER MiDeadPteTrackerSListHead;
KSPIN_LOCK MiPteTrackerLock;

KSPIN_LOCK MiTrackIoLock;

LONG MiActiveIoCounter;

#if (_MI_PAGING_LEVELS>=3)
KSPIN_LOCK MiLargePageLock;
RTL_BITMAP MiLargeVaBitMap;
#endif

ULONG MiNonCachedCollisions;

#if DBG
PFN_NUMBER MiCurrentAdvancedPages;
PFN_NUMBER MiAdvancesGiven;
PFN_NUMBER MiAdvancesFreed;
#endif

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    );

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    );

LOGICAL
MiReferenceIoSpace (
    IN PMDL MemoryDescriptorList,
    IN PPFN_NUMBER Page
    );

VOID
MiZeroAwePageWorker (
    IN PVOID Context
    );

#if DBG
ULONG MiPrintLockedPages;

VOID
MiVerifyLockedPageCharges (
    VOID
    );
#endif

VOID
MmLockPagableSectionByHandle (
    __in PVOID ImageSectionHandle
    );

VOID
MmUnlockPagableImageSection (
    __in PVOID ImageSectionHandle
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MmSetPageProtection)
#pragma alloc_text(INIT, MiInitializeIoTrackers)
#pragma alloc_text(INIT, MiInitializeLargePageSupport)

#pragma alloc_text(PAGE, MmAllocateIndependentPages)
#pragma alloc_text(PAGE, MmFreeIndependentPages)
#pragma alloc_text(PAGE, MmLockPageableDataSection)
#pragma alloc_text(PAGE, MiLookupDataTableEntry)
#pragma alloc_text(PAGE, MmSetBankedSection)
#pragma alloc_text(PAGE, MmProbeAndLockProcessPages)
#pragma alloc_text(PAGE, MmMapVideoDisplay)
#pragma alloc_text(PAGE, MmUnmapVideoDisplay)
#pragma alloc_text(PAGE, MmGetSectionRange)
#pragma alloc_text(PAGE, MmAllocateMappingAddress)
#pragma alloc_text(PAGE, MmFreeMappingAddress)
#pragma alloc_text(PAGE, MmAllocateNonCachedMemory)
#pragma alloc_text(PAGE, MmFreeNonCachedMemory)
#pragma alloc_text(PAGE, MmLockPagedPool)
#pragma alloc_text(PAGE, MmLockPagableSectionByHandle)
#pragma alloc_text(PAGE, MmLockPageableSectionByHandle)
#pragma alloc_text(PAGE, MiZeroAwePageWorker)
#pragma alloc_text(PAGE, MmAllocatePagesForMdl)
#pragma alloc_text(PAGE, MmAllocatePagesForMdlEx)

#pragma alloc_text(PAGELK, MmEnablePAT)
#pragma alloc_text(PAGELK, MiUnmapLockedPagesInUserSpace)
#pragma alloc_text(PAGELK, MiAllocatePagesForMdl)
#pragma alloc_text(PAGELK, MiZeroInParallel)
#pragma alloc_text(PAGELK, MmFreePagesFromMdl)
#pragma alloc_text(PAGELK, MmUnlockPagedPool)
#pragma alloc_text(PAGELK, MmGatherMemoryForHibernate)
#pragma alloc_text(PAGELK, MmReturnMemoryForHibernate)
#pragma alloc_text(PAGELK, MmReleaseDumpAddresses)
#pragma alloc_text(PAGELK, MmMapUserAddressesToPage)

#pragma alloc_text(PAGEVRFY, MmIsSystemAddressLocked)
#if !DBG
#pragma alloc_text(PAGEVRFY, MmAreMdlPagesLocked)
#endif
#endif

extern POOL_DESCRIPTOR NonPagedPoolDescriptor;

PFN_NUMBER MmMdlPagesAllocated;

KEVENT MmCollidedLockEvent;
LONG MmCollidedLockWait;

BOOLEAN MiWriteCombiningPtes = FALSE;

#if DBG
ULONG MiPrintAwe;
ULONG MmStopOnBadProbe = 1;
#endif

#define MI_PROBE_RAISE_SIZE 34

ULONG MiProbeRaises[MI_PROBE_RAISE_SIZE];

#define MI_INSTRUMENT_PROBE_RAISES(i)       \
        ASSERT (i < MI_PROBE_RAISE_SIZE);   \
        MiProbeRaises[i] += 1;

//
//  Note: this should be > 2041 to account for the cache manager's
//  aggressive zeroing logic.
//

ULONG MmReferenceCountCheck = MAXUSHORT / 2;

ULONG MiMdlsAdjusted = FALSE;

typedef enum _MI_LOCK_USED_FOR_PROBE {
    LOCK_TYPE_NONE = 0,
    LOCK_TYPE_AWE = 1,      // This must come first (code depends on it).
    LOCK_TYPE_WS = 2,
    LOCK_TYPE_PFN = 3
} MI_LOCK_USED_FOR_PROBE, *PMI_LOCK_USED_FOR_PROBE;


VOID
MmProbeAndLockPages (
    __inout PMDL MemoryDescriptorList,
    __in KPROCESSOR_MODE AccessMode,
    __in LOCK_OPERATION Operation
    )

/*++

Routine Description:

    This routine probes the specified pages, makes the pages resident and
    locks the physical pages mapped by the virtual pages in memory.  The
    Memory descriptor list is updated to describe the physical pages.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List
                            (MDL). The supplied MDL must supply a virtual
                            address, byte offset and length field.  The
                            physical page portion of the MDL is updated when
                            the pages are locked in memory.

    AccessMode - Supplies the access mode in which to probe the arguments.
                 One of KernelMode or UserMode.

    Operation - Supplies the operation type.  One of IoReadAccess, IoWriteAccess
                or IoModifyAccess.

Return Value:

    None - exceptions are raised.

Environment:

    Kernel mode.  APC_LEVEL and below for pageable addresses,
                  DISPATCH_LEVEL and below for non-pageable addresses.

--*/

{
    ULONG Processor;
    LOGICAL PfnHeldToo;
    PPFN_NUMBER Page;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PVOID Va;
    PVOID EndVa;
    PVOID AlignedVa;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;
    PEPROCESS CurrentProcess;
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPagesToLock;
    PFN_NUMBER NumberOfPagesSpanned;
    NTSTATUS status;
    NTSTATUS ProbeStatus;
    PETHREAD Thread;
    ULONG SavedState;
    PMI_PHYSICAL_VIEW PhysicalView;
    PCHAR StartVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PAWEINFO AweInfo;
    PEX_PUSH_LOCK PushLock;
    TABLE_SEARCH_RESULT SearchResult;
    LONG EntryCounter;
    MI_LOCK_USED_FOR_PROBE LockType;
#if defined (_X86PAE_)
    MMPTE BogusPte;
    
    BogusPte.u.Long = (ULONG64)-1;
#endif


Top:

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT (((ULONG)MemoryDescriptorList->ByteOffset & ~(PAGE_SIZE - 1)) == 0);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT (((ULONG_PTR)MemoryDescriptorList->StartVa & (PAGE_SIZE - 1)) == 0);
    AlignedVa = (PVOID)MemoryDescriptorList->StartVa;

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL |
                    MDL_IO_SPACE)) == 0);

    Va = (PCHAR)AlignedVa + MemoryDescriptorList->ByteOffset;
    StartVa = Va;

    //
    // Endva is one byte past the end of the buffer, if ACCESS_MODE is not
    // kernel, make sure the EndVa is in user space AND the byte count
    // does not cause it to wrap.
    //

    EndVa = (PVOID)((PCHAR)Va + MemoryDescriptorList->ByteCount);

    if ((AccessMode != KernelMode) &&
        ((EndVa > (PVOID)MM_USER_PROBE_ADDRESS) || (Va >= EndVa))) {
#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
        *Page = MM_EMPTY_LIST;
        MI_INSTRUMENT_PROBE_RAISES (0);
        ExRaiseStatus (STATUS_ACCESS_VIOLATION);
    }

    Thread = PsGetCurrentThread ();

    NumberOfPagesToLock = ADDRESS_AND_SIZE_TO_SPAN_PAGES (Va,
                                   MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPagesToLock != 0);

    if (Va <= MM_HIGHEST_USER_ADDRESS) {

        CurrentProcess = PsGetCurrentProcessByThread (Thread);

        if (CurrentProcess->AweInfo != NULL) {

            AweInfo = CurrentProcess->AweInfo;                
        
            //
            // Block APCs to prevent recursive pushlock scenarios as
            // this is not supported.
            //

            KeEnterGuardedRegionThread (&Thread->Tcb);

            PushLock = ExAcquireCacheAwarePushLockShared (AweInfo->PushLock);

            //
            // Provide a fast path for transfers that are within
            // a single AWE region.
            //

            Processor = KeGetCurrentProcessorNumber ();
            PhysicalView = AweInfo->PhysicalViewHint[Processor];

            if ((PhysicalView != NULL) &&
                ((PVOID)StartVa >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                ((PVOID)((PCHAR)EndVa - 1) <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                NOTHING;
            }
            else {

                //
                // Lookup the element and save the result.
                //

                SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                                   MI_VA_TO_VPN (StartVa),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    ((PVOID)StartVa >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                    ((PVOID)((PCHAR)EndVa - 1) <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                    AweInfo->PhysicalViewHint[Processor] = PhysicalView;
                }
                else {
                    ExReleaseCacheAwarePushLockShared (PushLock);
                    KeLeaveGuardedRegionThread (&Thread->Tcb);
                    goto DefaultProbeAndLock;
                }
            }
            
            MemoryDescriptorList->Process = CurrentProcess;

            MemoryDescriptorList->MdlFlags |= (MDL_PAGES_LOCKED | MDL_DESCRIBES_AWE);

            if (PhysicalView->VadType == VadAwe) {

                PointerPte = MiGetPteAddress (StartVa);
                LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);

                do {
                    PteContents = *PointerPte;

                    if (PteContents.u.Hard.Valid == 0) {

                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        MI_INSTRUMENT_PROBE_RAISES (9);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureUnlockAnyPages;
                    }

                    //
                    // This is an AWE frame - it is either
                    // noaccess, readonly or readwrite.
                    //

                    if ((PteContents.u.Hard.Owner == MI_PTE_OWNER_KERNEL) ||
                        ((Operation != IoReadAccess) && (PteContents.u.Hard.Write == 0))) {

                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        MI_INSTRUMENT_PROBE_RAISES (9);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureUnlockAnyPages;
                    }

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {

                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        MI_INSTRUMENT_PROBE_RAISES (17);
                        status = STATUS_WORKING_SET_QUOTA;
                        goto FailureUnlockAnyPages;
                    }

                    InterlockedIncrement (&Pfn1->AweReferenceCount);

                    *Page = PageFrameIndex;

                    Page += 1;
                    PointerPte += 1;
                } while (PointerPte <= LastPte);


                ExReleaseCacheAwarePushLockShared (PushLock);
                KeLeaveGuardedRegionThread (&Thread->Tcb);

                return;
            }

            if (PhysicalView->VadType == VadLargePages) {

                //
                // The PTE cannot be referenced (it doesn't exist), but it
                // serves the useful purpose of identifying when we cross
                // PDEs and therefore must recompute the base PFN.
                //

                PointerPte = MiGetPteAddress (StartVa);
                PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                do {

                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {
                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);

                        *Page = MM_EMPTY_LIST;
                        MI_INSTRUMENT_PROBE_RAISES (18);
                        status = STATUS_WORKING_SET_QUOTA;
                        goto FailureUnlockAnyPages;
                    }

                    InterlockedIncrement (&Pfn1->AweReferenceCount);

                    *Page = PageFrameIndex;
                    Page += 1;

                    NumberOfPagesToLock -= 1;

                    if (NumberOfPagesToLock == 0) {
                        break;
                    }

                    PointerPte += 1;

                    if (!MiIsPteOnPdeBoundary (PointerPte)) {
                        PageFrameIndex += 1;
                        Pfn1 += 1;
                    }
                    else {
                        StartVa = MiGetVirtualAddressMappedByPte (PointerPte);
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    }

                } while (TRUE);

                ExReleaseCacheAwarePushLockShared (PushLock);
                KeLeaveGuardedRegionThread (&Thread->Tcb);

                return;
            }
        }
    }
    else {
        CurrentProcess = NULL;
    }

DefaultProbeAndLock:

    NumberOfPagesSpanned = NumberOfPagesToLock;

    if (!MI_IS_PHYSICAL_ADDRESS (Va)) {

        ProbeStatus = STATUS_SUCCESS;

        MmSavePageFaultReadAhead (Thread, &SavedState);
        MmSetPageFaultReadAhead (Thread, (ULONG)(NumberOfPagesToLock - 1));

        try {

            do {

                *Page = MM_EMPTY_LIST;

                //
                // Make sure the page is resident.
                //

                *(volatile CHAR *)Va;

                if ((Operation != IoReadAccess) &&
                    (Va <= MM_HIGHEST_USER_ADDRESS)) {

                    //
                    // Probe for write access as well.
                    //

                    ProbeForWriteChar ((PCHAR)Va);
                }

                NumberOfPagesToLock -= 1;

                MmSetPageFaultReadAhead (Thread, (ULONG)(NumberOfPagesToLock - 1));
                Va = (PVOID) (((ULONG_PTR)Va + PAGE_SIZE) & ~(PAGE_SIZE - 1));
                Page += 1;
            } while (Va < EndVa);

            ASSERT (NumberOfPagesToLock == 0);
            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            ProbeStatus = GetExceptionCode();
        }

        //
        // We may still fault again below but it's generally rare.
        // Restore this thread's normal fault behavior now.
        //

        MmResetPageFaultReadAhead (Thread, SavedState);

        if (ProbeStatus != STATUS_SUCCESS) {
            MI_INSTRUMENT_PROBE_RAISES (1);
            MemoryDescriptorList->Process = NULL;
            ExRaiseStatus (ProbeStatus);
        }

        PointerPte = MiGetPteAddress (StartVa);
    }
    else {

        //
        // Set PointerPte to NULL to indicate this is a physical address range.
        //

        if (Va <= MM_HIGHEST_USER_ADDRESS) {
            PointerPte = MiGetPteAddress (StartVa);
        }
        else {
            PointerPte = NULL;
        }

        *Page = MM_EMPTY_LIST;
    }

    PointerPxe = MiGetPxeAddress (StartVa);
    PointerPpe = MiGetPpeAddress (StartVa);
    PointerPde = MiGetPdeAddress (StartVa);

    Va = AlignedVa;
    ASSERT (Page == (PPFN_NUMBER)(MemoryDescriptorList + 1));
    SATISFY_OVERZEALOUS_COMPILER (EntryCounter = 0);

    //
    // Indicate whether this is a read or write operation.
    //

    if (Operation != IoReadAccess) {
        MemoryDescriptorList->MdlFlags |= MDL_WRITE_OPERATION;
    }
    else {
        MemoryDescriptorList->MdlFlags &= ~(MDL_WRITE_OPERATION);
    }

    //
    // Initialize MdlFlags (assume the probe will succeed).
    //

    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;

    if (Va <= MM_HIGHEST_USER_ADDRESS) {

        //
        // These are user space addresses, check them carefully.
        //

        ASSERT (NumberOfPagesSpanned != 0);

        ASSERT (CurrentProcess == PsGetCurrentProcess ());

        //
        // Initialize the MDL process field (assume the probe will succeed).
        //

        MemoryDescriptorList->Process = CurrentProcess;

        LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);

        InterlockedExchangeAddSizeT (&CurrentProcess->NumberOfLockedPages,
                                     NumberOfPagesSpanned);
        OldIrql = MM_NOIRQL;
        LockType = LOCK_TYPE_WS;
        LOCK_WS_SHARED (Thread, CurrentProcess);
    }
    else {

        ASSERT (CurrentProcess == NULL);

        MemoryDescriptorList->Process = NULL;

        Va = (PCHAR)Va + MemoryDescriptorList->ByteOffset;

        NumberOfPagesToLock = ADDRESS_AND_SIZE_TO_SPAN_PAGES (Va,
                                    MemoryDescriptorList->ByteCount);

        if (PointerPte == NULL) {

            //
            // On certain architectures, virtual addresses
            // may be physical and hence have no corresponding PTE.
            //

            PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (Va);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            LastPageFrameIndex = PageFrameIndex + NumberOfPagesToLock;

            ASSERT ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0);

            //
            // Acquire the PFN database lock.
            //
    
            LOCK_PFN2 (OldIrql);

            do {
        
                if (MI_IS_PFN (PageFrameIndex)) {

                    //
                    // Check to make sure each page is not locked down an
                    // unusually high number of times.
                    //
        
                    ASSERT (PageFrameIndex <= MmHighestPhysicalPage);
    
                    if (Pfn1->u3.e2.ReferenceCount >= MmReferenceCountCheck) {
                        UNLOCK_PFN2 (OldIrql);
                        ASSERT (FALSE);
                        MI_INSTRUMENT_PROBE_RAISES (19);
                        status = STATUS_WORKING_SET_QUOTA;
                        goto FailureUnlockAnyPages;
                    }
        
                    if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                        MI_SNAP_DIRTY (Pfn1, 1, 0x99);
                    }
    
                    MI_ADD_LOCKED_PAGE_CHARGE (Pfn1);
                }
                else {
                    MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
                }
    
                *Page = PageFrameIndex;
    
                Page += 1;
                PageFrameIndex += 1;
                Pfn1 += 1;
    
            } while (PageFrameIndex < LastPageFrameIndex);

            UNLOCK_PFN2 (OldIrql);

            if (MmTrackLockedPages == TRUE) {

                ASSERT (NumberOfPagesSpanned != 0);

                RtlGetCallersAddress (&CallingAddress, &CallersCaller);

                MiAddMdlTracker (MemoryDescriptorList,
                                 CallingAddress,
                                 CallersCaller,
                                 NumberOfPagesSpanned,
                                 1);
            }

            return;
        }

        //
        // Since this operation is to a system address, no need to check for
        // PTE write access below so mark the access as a read so only the
        // operation type (and not where the Va is) needs to be checked in the
        // subsequent loop.
        //

        Operation = IoReadAccess;

        LastPte = MiGetPteAddress ((PCHAR)EndVa - 1);

        //
        // A driver may build an MDL for a user address and then get
        // a system VA for it.  The driver may then call MmProbeAndLockPages
        // again, this time with the system VA (instead of the original
        // user VA) for the same range.
        //
        // Thus the PFNs will really point at PFNs which generally
        // require the relevant process' working set pushlock, but in
        // this instance we would only be holding the system working set
        // pushlock.  Therefore the user can change PFN sharecounts and
        // active states at any time.  The only thing we'd know for sure
        // is that the refcount must be nonzero on entry and will stay
        // that way because it would be illegal for the driver to
        // have called MmUnlockPages prior to this probe call.
        //
        // However, this is not enough state to ensure we charge locking
        // properly (without leaking) so the code that used to acquire the
        // system working set mutex here has been changed to instead always
        // use the PFN lock to synchronize system space VA arguments.
        //
    
        LockType = LOCK_TYPE_PFN;
        LOCK_PFN2 (OldIrql);
    }

    do {

        //
        // Set the current MDL entry to empty so all error paths can
        // call MmUnlockPages.
        //

        *Page = MM_EMPTY_LIST;

        while (
#if (_MI_PAGING_LEVELS>=4)
               (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS>=3)
               (PointerPpe->u.Hard.Valid == 0) ||
#endif
               ((PointerPde->u.Hard.Valid == 0) ||
                (((MI_PDE_MAPS_LARGE_PAGE (PointerPde)) == 0) &&
                 (PointerPte->u.Hard.Valid == 0)))) {

            //
            // The VA is not resident, release the PFN lock and access the page
            // to make it appear.
            //

            if (LockType == LOCK_TYPE_WS) {
                UNLOCK_WS_SHARED (Thread, CurrentProcess);
            }
            else {
                ASSERT (LockType == LOCK_TYPE_PFN);
                UNLOCK_PFN2 (OldIrql);
            }

            MmSavePageFaultReadAhead (Thread, &SavedState);
            MmSetPageFaultReadAhead (Thread, 0);

            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            status = MmAccessFault (FALSE, Va, KernelMode, NULL);

            MmResetPageFaultReadAhead (Thread, SavedState);

            if (!NT_SUCCESS(status)) {
                MI_INSTRUMENT_PROBE_RAISES (20);
                goto FailureUnlockAnyPages;
            }

            if (LockType == LOCK_TYPE_WS) {
                LOCK_WS_SHARED (Thread, CurrentProcess);
            }
            else {
                ASSERT (LockType == LOCK_TYPE_PFN);
                LOCK_PFN2 (OldIrql);
            }
        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            if ((Operation != IoReadAccess) &&
                (PointerPde->u.Hard.Write == 0)) {

                if (LockType == LOCK_TYPE_WS) {
                    UNLOCK_WS_SHARED (Thread, CurrentProcess);
                }
                else {
                    ASSERT (LockType == LOCK_TYPE_PFN);
                    UNLOCK_PFN2 (OldIrql);
                }

                MI_INSTRUMENT_PROBE_RAISES (4);
                status = STATUS_ACCESS_VIOLATION;
                goto FailureUnlockAnyPages;
            }

            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (Va);
        }
        else {

#if defined (_X86PAE_)

            //
            // Pick up the PTE as an interlocked operation to prevent AWE
            // updates from tearing our read on these platforms.
            //

            PteContents.u.Long = InterlockedCompareExchangePte (PointerPte,
                                                                BogusPte.u.Long,
                                                                BogusPte.u.Long);
            ASSERT (PteContents.u.Long != BogusPte.u.Long);

#else
            PteContents = *PointerPte;
#endif

            //
            // There is a subtle race here where the PTE contents can get zeroed
            // by a thread running on another processor.  This can only happen
            // for an AWE address space because these ranges (deliberately for
            // performance reasons) do not acquire the PFN lock during remap
            // operations.  In this case, one of 2 scenarios is possible -
            // either the old PTE is read or the new.  The new may be a zero
            // PTE if the map request was to invalidate *or* non-zero (and
            // valid) if the map request was inserting a new entry.  For the
            // latter, we don't care if we lock the old or new frame here as
            // it's an application bug to provoke this behavior - and
            // regardless of which is used, no corruption can occur because
            // the PFN lock is acquired during an NtFreeUserPhysicalPages.
            // But the former must be checked for explicitly here.  As a
            // separate note, the PXE/PPE/PDE accesses above are always safe
            // even for the AWE deletion race because these tables
            // are never lazy-allocated for AWE ranges.
            //

            if (PteContents.u.Hard.Valid == 0) {
                ASSERT (PteContents.u.Long == 0);
                ASSERT (PsGetCurrentProcess ()->AweInfo != NULL);

                MI_INSTRUMENT_PROBE_RAISES (5);
                status = STATUS_ACCESS_VIOLATION;
                goto FailureReleaseLocks;
            }

            if (Operation != IoReadAccess) {

                if ((PteContents.u.Long & MM_PTE_WRITE_MASK) == 0) {

                    if (PteContents.u.Long & MM_PTE_COPY_ON_WRITE_MASK) {

                        //
                        // The protection has changed from writable to copy on
                        // write.  This can happen if a fork is in progress for
                        // example.  Restart the operation at the top.
                        //

                        Va = MiGetVirtualAddressMappedByPte (PointerPte);

                        if (Va <= MM_HIGHEST_USER_ADDRESS) {

                            if (LockType == LOCK_TYPE_WS) {
                                UNLOCK_WS_SHARED (Thread, CurrentProcess);
                            }
                            else {
                                ASSERT (LockType == LOCK_TYPE_PFN);
                                UNLOCK_PFN2 (OldIrql);
                            }

                            MmSavePageFaultReadAhead (Thread, &SavedState);
                            MmSetPageFaultReadAhead (Thread, 0);

                            status = MmAccessFault (TRUE, Va, KernelMode, NULL);

                            MmResetPageFaultReadAhead (Thread, SavedState);

                            if (!NT_SUCCESS(status)) {
                                MI_INSTRUMENT_PROBE_RAISES (23);
                                goto FailureUnlockAnyPages;
                            }

                            if (LockType == LOCK_TYPE_WS) {
                                LOCK_WS_SHARED (Thread, CurrentProcess);
                            }
                            else {
                                ASSERT (LockType == LOCK_TYPE_PFN);
                                LOCK_PFN2 (OldIrql);
                            }

                            continue;
                        }
                    }

                    //
                    // The caller has made the page protection more
                    // restrictive, this should never be done once the
                    // request has been issued !  Rather than wading
                    // through the PFN database entry to see if it
                    // could possibly work out, give the caller an
                    // access violation.
                    //

#if DBG
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MmProbeAndLockPages: PTE %p %p changed\n",
                        PointerPte,
                        PteContents.u.Long);

                    if (MmStopOnBadProbe) {
                        DbgBreakPoint ();
                    }
#endif

                    MI_INSTRUMENT_PROBE_RAISES (16);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }
            }

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }

        if (MI_IS_PFN (PageFrameIndex)) {

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            ASSERT ((CurrentProcess == NULL) || (LockType == LOCK_TYPE_WS));
    
            PfnHeldToo = FALSE;

            if ((CurrentProcess != NULL) && (CurrentProcess->PhysicalVadRoot != NULL)) {
    
                //
                // This process has a \Device\PhysicalMemory VAD so it must be
                // checked to see if the current address resides in it.
                //
    
                SearchResult = MiFindNodeOrParent (CurrentProcess->PhysicalVadRoot,
                                                   MI_VA_TO_VPN (Va),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    (PhysicalView->VadType == VadDevicePhysicalMemory)) {
    
                    ASSERT ((ULONG)PhysicalView->Vad->u.VadFlags.VadType == (ULONG)PhysicalView->VadType);
    
                    //
                    // The VA lies within a device physical memory VAD.
                    //
                    // The PTE was already checked for read/write permissions
                    // above, but the PFN must still be checked for ownership
                    // and this must be done with the PFN lock since this
                    // process is not necessarily the correct owner.
                    //
    
                    PfnHeldToo = TRUE;

                    LOCK_PFN (OldIrql);

                    if (Pfn1->u3.e1.PageLocation != ActiveAndValid) {
                        UNLOCK_PFN (OldIrql);
                        MI_INSTRUMENT_PROBE_RAISES (32);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureReleaseLocks;
                    }

                    //
                    // Continue to hold the PFN lock until the reference count
                    // on the desired PFN is incremented.
                    //
                }
            }

            if (MI_ADD_LOCKED_PAGE_CHARGE_FOR_PROBE (Pfn1, FALSE, PointerPte) == FALSE) {

                if (PfnHeldToo == TRUE) {
                    UNLOCK_PFN (OldIrql);
                }
    
                //
                // If this page is for privileged code/data,
                // then force it in regardless.
                //
    
                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                if ((Va < MM_HIGHEST_USER_ADDRESS) ||
                    (MI_IS_SYSTEM_CACHE_ADDRESS(Va)) ||
                    ((Va >= MmPagedPoolStart) && (Va <= MmPagedPoolEnd))) {

                    MI_INSTRUMENT_PROBE_RAISES (10);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto FailureReleaseLocks;
                }

                if (MI_ADD_LOCKED_PAGE_CHARGE_FOR_PROBE (Pfn1, TRUE, PointerPte) == FALSE) {

                    //
                    // The only reason we don't force for privileged code/data
                    // is if it would have caused a reference count wrap.
                    // Typically this indicates a driver is forgetting to call
                    // MmUnlockPages as the page is locked down an unusually
                    // high number of times.
                    //

                    MI_INSTRUMENT_PROBE_RAISES (3);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto FailureReleaseLocks;
                }
            }
            else if (PfnHeldToo == TRUE) {
                UNLOCK_PFN (OldIrql);
            }
    

            if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                MI_SNAP_DIRTY (Pfn1, 1, 0x98);
            }
        }
        else {

            //
            // This is an I/O space address - there is no PFN database entry
            // for it, so no reference counts may be modified for these pages.
            //
            // Don't charge page locking for this page, just add it to the MDL.
            //

            if (CurrentProcess != NULL) {

                //
                // The VA better be within a \Device\PhysicalMemory VAD.
                //

                if (CurrentProcess->PhysicalVadRoot == NULL) {
                    MI_INSTRUMENT_PROBE_RAISES (2);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                SearchResult = MiFindNodeOrParent (CurrentProcess->PhysicalVadRoot,
                                                   MI_VA_TO_VPN (Va),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    ((PhysicalView->VadType == VadRotatePhysical) ||
                     (PhysicalView->VadType == VadDevicePhysicalMemory))) {

                    ASSERT ((ULONG)PhysicalView->Vad->u.VadFlags.VadType == (ULONG)PhysicalView->VadType);

                    //
                    // The range lies within a rotate physical VAD or a
                    // device physical memory VAD.
                    //
                    // We already know the access is ok because the PTE was
                    // already checked for read/write permissions above.
                    //
    
                    NOTHING;
                }
                else {
                    MI_INSTRUMENT_PROBE_RAISES (11);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }
            }

            if (((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) &&
                (CurrentProcess != NULL)) {

                EntryCounter = *(volatile PLONG) &MiActiveIoCounter;
            }

            MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
        }

        *Page = PageFrameIndex;

        Page += 1;

        PointerPte += 1;

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde += 1;

            if (MiIsPteOnPpeBoundary (PointerPte)) {
                PointerPpe += 1;
                if (MiIsPteOnPxeBoundary (PointerPte)) {
                    PointerPxe += 1;
                }
            }
        }

    } while (PointerPte <= LastPte);

    if (LockType == LOCK_TYPE_WS) {
        UNLOCK_WS_SHARED (Thread, CurrentProcess);
    }
    else {
        ASSERT (LockType == LOCK_TYPE_PFN);
        UNLOCK_PFN2 (OldIrql);
    }

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_DESCRIBES_AWE) == 0);

    if (AlignedVa <= MM_HIGHEST_USER_ADDRESS) {

        //
        // User space buffers that reside in I/O space need to be reference
        // counted because SANs will want to reuse the physical space but cannot
        // do this unless it is guaranteed there are no more pending I/Os
        // going from/to it.
        //

        if (MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) {

            if (MiReferenceIoSpace (MemoryDescriptorList, Page) == FALSE) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                MI_INSTRUMENT_PROBE_RAISES (24);
                goto FailureUnlockAnyPages;
            }

            if (EntryCounter != *(volatile PLONG) &MiActiveIoCounter) {

                //
                // The counter has changed since we captured the first PTE.
                // Just repeat everything.
                //

                MiDereferenceIoSpace (MemoryDescriptorList);

                if (MmTrackLockedPages == TRUE) {

                    ASSERT (NumberOfPagesSpanned != 0);

                    RtlGetCallersAddress (&CallingAddress, &CallersCaller);

                    MiAddMdlTracker (MemoryDescriptorList,
                                     CallingAddress,
                                     CallersCaller,
                                     NumberOfPagesSpanned,
                                     2);
                }

                MmUnlockPages (MemoryDescriptorList);
                goto Top;
            }
        }
    }

    if (MmTrackLockedPages == TRUE) {

        ASSERT (NumberOfPagesSpanned != 0);

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         NumberOfPagesSpanned,
                         3);
    }

    return;

FailureReleaseLocks:

    //
    // A violation was detected.  Release the synchronization primitive
    // we used and unlock any pages locked so far.
    //

    ASSERT (!NT_SUCCESS (status));

    ASSERT (LockType > LOCK_TYPE_AWE);

    if (LockType == LOCK_TYPE_PFN) {
        UNLOCK_PFN2 (OldIrql);
    }
    else {
        ASSERT (LockType == LOCK_TYPE_WS);
        UNLOCK_WS_SHARED (Thread, CurrentProcess);
    }

FailureUnlockAnyPages:

    //
    // An exception occurred, unlock any pages locked so far.  Note that
    // a tracker entry must be inserted for MmUnlockPages to find.
    //

    ASSERT (MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED);

    if ((MmTrackLockedPages == TRUE) &&
        ((MemoryDescriptorList->MdlFlags & MDL_DESCRIBES_AWE) == 0)) {

        //
        // Compute NumberOfPagesToLock as MmUnlockPages will also when
        // freeing the MDL tracker.
        //

        NumberOfPagesToLock = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartVa,
                                        MemoryDescriptorList->ByteCount);

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         NumberOfPagesToLock,
                         5);
    }

    MmUnlockPages (MemoryDescriptorList);

    //
    // Raise an exception of access violation to the caller.
    //

    MI_INSTRUMENT_PROBE_RAISES (13);
    ExRaiseStatus (status);
}

NTKERNELAPI
VOID
MmProbeAndLockProcessPages (
    __inout PMDL MemoryDescriptorList,
    __in PEPROCESS Process,
    __in KPROCESSOR_MODE AccessMode,
    __in LOCK_OPERATION Operation
    )

/*++

Routine Description:

    This routine probes and locks the address range specified by
    the MemoryDescriptorList in the specified Process for the AccessMode
    and Operation.

Arguments:

    MemoryDescriptorList - Supplies a pre-initialized MDL that describes the
                           address range to be probed and locked.

    Process - Specifies the address of the process whose address range is
              to be locked.

    AccessMode - The mode for which the probe should check access to the range.

    Operation - Supplies the type of access which for which to check the range.

Return Value:

    None.

--*/

{
    KAPC_STATE ApcState;
    LOGICAL Attached;
    NTSTATUS Status;

    Attached = FALSE;
    Status = STATUS_SUCCESS;

    if (Process != PsGetCurrentProcess ()) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    try {

        MmProbeAndLockPages (MemoryDescriptorList,
                             AccessMode,
                             Operation);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (Status != STATUS_SUCCESS) {

        ExRaiseStatus (Status);
    }
    return;
}

VOID
MiAddMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller,
    IN PFN_NUMBER NumberOfPagesToLock,
    IN ULONG Who
    )

/*++

Routine Description:

    This routine adds an MDL to the specified process' chain.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List
                           (MDL). The MDL must supply the length. The
                           physical page portion of the MDL is updated when
                           the pages are locked in memory.

    CallingAddress - Supplies the address of the caller of our caller.

    CallersCaller - Supplies the address of the caller of CallingAddress.

    NumberOfPagesToLock - Specifies the number of pages to lock.

    Who - Specifies which routine is adding the entry.

Return Value:

    None - exceptions are raised.

Environment:

    Kernel mode.  APC_LEVEL and below.

--*/

{
    PEPROCESS Process;
    PLOCK_HEADER LockedPagesHeader;
    PLOCK_TRACKER Tracker;
    PLOCK_TRACKER P;
    PLIST_ENTRY NextEntry;
    KLOCK_QUEUE_HANDLE LockHandle;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        Process = PsInitialSystemProcess;
    }

    LockedPagesHeader = Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return;
    }

    //
    // It's ok to check unsynchronized for aborted tracking as the worst case
    // is just that one more entry gets added which will be freed later anyway.
    // The main purpose behind aborted tracking is that frees and exits don't
    // mistakenly bugcheck when an entry cannot be found.
    //

    if (LockedPagesHeader->Valid == FALSE) {
        return;
    }

    Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof (LOCK_TRACKER),
                                     'kLmM');

    if (Tracker == NULL) {

        //
        // It's ok to set this without synchronization as the worst case
        // is just that a few more entries gets added which will be freed
        // later anyway.  The main purpose behind aborted tracking is that
        // frees and exits don't mistakenly bugcheck when an entry cannot
        // be found.
        //
    
        LockedPagesHeader->Valid = FALSE;

        return;
    }

    Tracker->Mdl = MemoryDescriptorList;
    Tracker->Count = NumberOfPagesToLock;
    Tracker->StartVa = MemoryDescriptorList->StartVa;
    Tracker->Offset = MemoryDescriptorList->ByteOffset;
    Tracker->Length = MemoryDescriptorList->ByteCount;
    Tracker->Page = *(PPFN_NUMBER)(MemoryDescriptorList + 1);

    Tracker->CallingAddress = CallingAddress;
    Tracker->CallersCaller = CallersCaller;

    Tracker->Who = Who;
    Tracker->Process = Process;

    //
    // Update the list for this process.  First make sure it's not already
    // inserted.
    //

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    NextEntry = LockedPagesHeader->ListHead.Flink;

    while (NextEntry != &LockedPagesHeader->ListHead) {

        P = CONTAINING_RECORD (NextEntry,
                               LOCK_TRACKER,
                               ListEntry);

        if (P->Mdl == MemoryDescriptorList) {
            KeBugCheckEx (LOCKED_PAGES_TRACKER_CORRUPTION,
                          0x1,
                          (ULONG_PTR) P,
                          (ULONG_PTR) MemoryDescriptorList,
                          (ULONG_PTR) LockedPagesHeader->Count);
        }
        NextEntry = NextEntry->Flink;
    }

    InsertTailList (&LockedPagesHeader->ListHead, &Tracker->ListEntry);

    LockedPagesHeader->Count += NumberOfPagesToLock;

    KeReleaseInStackQueuedSpinLock (&LockHandle);
}

LOGICAL
MiFreeMdlTracker (
    IN OUT PMDL MemoryDescriptorList,
    IN PFN_NUMBER NumberOfPages
    )

/*++

Routine Description:

    This deletes an MDL from the specified process' chain.  Used specifically
    by MmProbeAndLockSelectedPages () because it builds an MDL in its local
    stack and then copies the requested pages into the real MDL.  This lets
    us track these pages.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List
                           (MDL). The MDL must supply the length.

    NumberOfPages - Supplies the number of pages to be freed.

Return Value:

    TRUE.

Environment:

    Kernel mode.  APC_LEVEL and below.

--*/
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PPFN_NUMBER Page;
    PVOID PoolToFree;
    PEPROCESS Process;

    Process = MemoryDescriptorList->Process;
    
    if (Process == NULL) {
        Process = PsInitialSystemProcess;
    }

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return TRUE;
    }

    PoolToFree = NULL;

    Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    NextEntry = LockedPagesHeader->ListHead.Flink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {

            if (PoolToFree != NULL) {
                KeBugCheckEx (LOCKED_PAGES_TRACKER_CORRUPTION,
                              0x3,
                              (ULONG_PTR) PoolToFree,
                              (ULONG_PTR) Tracker,
                              (ULONG_PTR) MemoryDescriptorList);
            }

            ASSERT (Tracker->Page == *Page);
            ASSERT (Tracker->Count == NumberOfPages);

            RemoveEntryList (NextEntry);
            LockedPagesHeader->Count -= NumberOfPages;

            PoolToFree = (PVOID) Tracker;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    if (PoolToFree == NULL) {

        //
        // A driver is trying to unlock pages that aren't locked.
        //

        if (LockedPagesHeader->Valid == FALSE) {
            return TRUE;
        }

        KeBugCheckEx (PROCESS_HAS_LOCKED_PAGES,
                      1,
                      (ULONG_PTR) MemoryDescriptorList,
                      Process->NumberOfLockedPages,
                      (ULONG_PTR) Process->LockedPagesList);
    }

    ExFreePool (PoolToFree);

    return TRUE;
}


LOGICAL
MmUpdateMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller
    )

/*++

Routine Description:

    This updates an MDL in the specified process' chain.  Used by the I/O
    system so that proper driver identification can be done even when I/O
    is actually locking the pages on their behalf.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List.

    CallingAddress - Supplies the address of the caller of our caller.

    CallersCaller - Supplies the address of the caller of CallingAddress.

Return Value:

    TRUE if the MDL was found, FALSE if not.

Environment:

    Kernel mode.  APC_LEVEL and below.

--*/
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PEPROCESS Process;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        Process = PsInitialSystemProcess;
    }

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return FALSE;
    }

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    //
    // Walk the list backwards as it's likely the MDL was
    // just recently inserted.
    //

    NextEntry = LockedPagesHeader->ListHead.Blink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {
            ASSERT (Tracker->Page == *(PPFN_NUMBER) (MemoryDescriptorList + 1));
            Tracker->CallingAddress = CallingAddress;
            Tracker->CallersCaller = CallersCaller;
            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return TRUE;
        }
        NextEntry = Tracker->ListEntry.Blink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    //
    // The caller is trying to update an MDL that is no longer locked.
    //

    return FALSE;
}


LOGICAL
MiUpdateMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG AdvancePages
    )

/*++

Routine Description:

    This updates an MDL in the specified process' chain.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List.

    AdvancePages - Supplies the number of pages being advanced.

Return Value:

    TRUE if the MDL was found, FALSE if not.

Environment:

    Kernel mode.  DISPATCH_LEVEL and below.

--*/
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PPFN_NUMBER Page;
    PLOCK_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PLOCK_HEADER LockedPagesHeader;
    PEPROCESS Process;

    ASSERT (MmTrackLockedPages == TRUE);

    Process = MemoryDescriptorList->Process;

    if (Process == NULL) {
        Process = PsInitialSystemProcess;
    }

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (LockedPagesHeader == NULL) {
        return FALSE;
    }

    KeAcquireInStackQueuedSpinLock (&LockedPagesHeader->Lock, &LockHandle);

    //
    // Walk the list backwards as it's likely the MDL was
    // just recently inserted.
    //

    NextEntry = LockedPagesHeader->ListHead.Blink;
    while (NextEntry != &LockedPagesHeader->ListHead) {

        Tracker = CONTAINING_RECORD (NextEntry,
                                     LOCK_TRACKER,
                                     ListEntry);

        if (MemoryDescriptorList == Tracker->Mdl) {

            Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);

            ASSERT (Tracker->Page == *Page);
            ASSERT (Tracker->Count > AdvancePages);

            Tracker->Page = *(Page + AdvancePages);
            Tracker->Count -= AdvancePages;

            KeReleaseInStackQueuedSpinLock (&LockHandle);
            return TRUE;
        }
        NextEntry = Tracker->ListEntry.Blink;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);

    //
    // The caller is trying to update an MDL that is no longer locked.
    //

    return FALSE;
}


#if defined(_AMD64_)
#define COPY_STACK_SIZE              350    // so chkstk doesn't complain
#else
#define COPY_STACK_SIZE              512
#endif


NTKERNELAPI
VOID
MmProbeAndLockSelectedPages (
    __inout PMDL MemoryDescriptorList,
    __in PFILE_SEGMENT_ELEMENT PagedSegmentArray,
    __in KPROCESSOR_MODE AccessMode,
    __in LOCK_OPERATION Operation
    )

/*++

Routine Description:

    This routine probes the specified pages, makes the pages resident and
    locks the physical pages mapped by the virtual pages in memory.  The
    Memory descriptor list is updated to describe the physical pages.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List
                           (MDL). The MDL must supply the length. The
                           physical page portion of the MDL is updated when
                           the pages are locked in memory.

    PagedSegmentArray - Supplies a pointer to a list of buffer segments to be
                        probed and locked.  Note that this array is in kernel
                        space and contains a list of user mode addresses.

    AccessMode - Supplies the access mode in which to probe the arguments.
                 One of KernelMode or UserMode.

    Operation - Supplies the operation type.  One of IoReadAccess, IoWriteAccess
                or IoModifyAccess.

Return Value:

    None - exceptions are raised.

Environment:

    Kernel mode.  APC_LEVEL and below.

--*/

{
    LOGICAL AweOk;
    LOGICAL LockedPagesCharged;
    LOGICAL PfnHeldToo;
    ULONG Processor;
    PPFN_NUMBER Page;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PVOID Va;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PEPROCESS CurrentProcess;
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPagesToLock;
    NTSTATUS status;
    PETHREAD Thread;
    ULONG SavedState;
    PMI_PHYSICAL_VIEW PhysicalView;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PAWEINFO AweInfo;
    PEX_PUSH_LOCK PushLock;
    TABLE_SEARCH_RESULT SearchResult;
    LONG EntryCounter;
    MI_LOCK_USED_FOR_PROBE LockType;
    PFILE_SEGMENT_ELEMENT SegmentArray;
    PFILE_SEGMENT_ELEMENT EntrySegment;
    PFILE_SEGMENT_ELEMENT LastSegment;
    FILE_SEGMENT_ELEMENT StackArray[COPY_STACK_SIZE];
#if defined (_X86PAE_)
    MMPTE BogusPte;
    
    BogusPte.u.Long = (ULONG64)-1;
#endif


    PAGED_CODE ();

    AweOk = TRUE;
    LockedPagesCharged = FALSE;
    Thread = PsGetCurrentThread ();

    SATISFY_OVERZEALOUS_COMPILER (OldIrql = MM_NOIRQL);
    SATISFY_OVERZEALOUS_COMPILER (EntryCounter = 0);
    SATISFY_OVERZEALOUS_COMPILER (PushLock = NULL);

    //
    // Calculate the end of the segment list.
    //

    NumberOfPagesToLock = BYTES_TO_PAGES (MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPagesToLock != 0);
    ASSERT (NumberOfPagesToLock <= (MAXULONG_PTR / sizeof (FILE_SEGMENT_ELEMENT)));

    EntrySegment = (PVOID)&StackArray[0];

    if (NumberOfPagesToLock > COPY_STACK_SIZE) {

        EntrySegment = ExAllocatePoolWithTag (NonPagedPool,
                                              NumberOfPagesToLock * sizeof (FILE_SEGMENT_ELEMENT),
                                              'rPmM');

        if (EntrySegment == NULL) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    SegmentArray = EntrySegment;

    LastSegment = SegmentArray + NumberOfPagesToLock;

    ASSERT (SegmentArray < LastSegment);

    //
    // Capture the specified segment elements so we can safely access them
    // while locks are held below.  This is because the caller's segment
    // array can be in paged pool.
    //

    RtlCopyMemory (SegmentArray,
                   PagedSegmentArray,
                   NumberOfPagesToLock * sizeof (FILE_SEGMENT_ELEMENT));

    //
    // Indicate whether this is a read or write operation.
    //

    if (Operation != IoReadAccess) {
        MemoryDescriptorList->MdlFlags |= MDL_WRITE_OPERATION;
    }
    else {
        MemoryDescriptorList->MdlFlags &= ~(MDL_WRITE_OPERATION);
    }

    MemoryDescriptorList->Process = NULL;

    Va = (PVOID) Ptr64ToPtr (SegmentArray->Buffer);

    if (Va <= MM_HIGHEST_USER_ADDRESS) {
        CurrentProcess = PsGetCurrentProcessByThread (Thread);
    }
    else {
        CurrentProcess = NULL;
    }
    
Top:

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT (((ULONG)MemoryDescriptorList->ByteOffset & ~(PAGE_SIZE - 1)) == 0);
    ASSERT (((ULONG_PTR)MemoryDescriptorList->StartVa & (PAGE_SIZE - 1)) == 0);

    LockType = LOCK_TYPE_NONE;
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL |
                    MDL_IO_SPACE)) == 0);

    //
    // Initialize MdlFlags (assume the probe will succeed).
    //

    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;
    
    for (SegmentArray = EntrySegment; SegmentArray < LastSegment; SegmentArray += 1) {

        //
        // Even systems without 64 bit pointers are required to zero the
        // upper 32 bits of the segment address so use alignment rather
        // than the buffer pointer.
        //

        Va = Ptr64ToPtr (SegmentArray->Buffer);

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
        *Page = MM_EMPTY_LIST;

        if (Va <= MM_HIGHEST_USER_ADDRESS) {
    
            ASSERT (LockType != LOCK_TYPE_PFN);
            ASSERT (CurrentProcess == PsGetCurrentProcess ());
    
            if ((AweOk == TRUE) && (CurrentProcess->AweInfo != NULL)) {
    
                AweInfo = CurrentProcess->AweInfo;                
            
                //
                // Block APCs to prevent recursive pushlock scenarios as
                // this is not supported.
                //
    
                if (LockType == LOCK_TYPE_NONE) {
                    LockType = LOCK_TYPE_AWE;
                    KeEnterGuardedRegionThread (&Thread->Tcb);
    
                    PushLock = ExAcquireCacheAwarePushLockShared (AweInfo->PushLock);
                }
    
                //
                // Provide a fast path for transfers that are within
                // a single AWE region.
                //
    
                Processor = KeGetCurrentProcessorNumber ();
                PhysicalView = AweInfo->PhysicalViewHint[Processor];
    
                if ((PhysicalView != NULL) &&
                    ((PVOID)Va >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                    ((PVOID)Va <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                    NOTHING;
                }
                else {
    
                    //
                    // Lookup the element and save the result.
                    //
    
                    SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                                       MI_VA_TO_VPN (Va),
                                                       (PMMADDRESS_NODE *) &PhysicalView);
                    if ((SearchResult == TableFoundNode) &&
                        ((PVOID)Va >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                        ((PVOID)Va <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {
                        AweInfo->PhysicalViewHint[Processor] = PhysicalView;
                    }
                    else {
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);
                        LockType = LOCK_TYPE_NONE;

                        AweOk = FALSE;

                        if (Page == (PPFN_NUMBER)(MemoryDescriptorList + 1)) {

                            //
                            // This is the first page of the transfer.  This
                            // process has AWE mappings, but this range is
                            // not in it.  That's ok, just use the working
                            // set pushlock to process it normally.
                            //

                            goto DefaultProbeAndLock;
                        }

                        //
                        // Release the existing MDL's pages as they will have
                        // to be relocked without using the AWE mechanism.
                        //

                        MmUnlockPages (MemoryDescriptorList);

                        goto Top;
                    }
                }
                
                MemoryDescriptorList->Process = CurrentProcess;
    
                MemoryDescriptorList->MdlFlags |= (MDL_PAGES_LOCKED | MDL_DESCRIBES_AWE);
    
                if (PhysicalView->VadType == VadAwe) {
    
                    PointerPte = MiGetPteAddress (Va);
    
                    PteContents = *PointerPte;
    
                    if (PteContents.u.Hard.Valid == 0) {
    
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);
                        LockType = LOCK_TYPE_NONE;
    
                        MI_INSTRUMENT_PROBE_RAISES (17);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureUnlockAnyPages;
                    }
    
                    //
                    // This is an AWE frame - it is either
                    // noaccess, readonly or readwrite.
                    //

                    if ((PteContents.u.Hard.Owner == MI_PTE_OWNER_KERNEL) ||
                        ((Operation != IoReadAccess) && (PteContents.u.Hard.Write == 0))) {

                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);
                        LockType = LOCK_TYPE_NONE;

                        MI_INSTRUMENT_PROBE_RAISES (18);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureUnlockAnyPages;
                    }

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {
    
                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);
                        LockType = LOCK_TYPE_NONE;
    
                        MI_INSTRUMENT_PROBE_RAISES (19);
                        status = STATUS_WORKING_SET_QUOTA;
                        goto FailureUnlockAnyPages;
                    }
    
                    InterlockedIncrement (&Pfn1->AweReferenceCount);
    
                    *Page = PageFrameIndex;
                    Page += 1;
    
                    //
                    // Captured this page, march on to the next one.
                    //

                    continue;
                }
    
                if (PhysicalView->VadType == VadLargePages) {
    
                    //
                    // The PTE cannot be referenced (it doesn't exist), but it
                    // serves the useful purpose of identifying when we cross
                    // PDEs and therefore must recompute the base PFN.
                    //
    
                    PointerPte = MiGetPteAddress (Va);
                    PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (Va);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
                    if (Pfn1->AweReferenceCount >= (LONG)MmReferenceCountCheck) {
                        ASSERT (FALSE);
                        ExReleaseCacheAwarePushLockShared (PushLock);
                        KeLeaveGuardedRegionThread (&Thread->Tcb);
                        LockType = LOCK_TYPE_NONE;
    
                        MI_INSTRUMENT_PROBE_RAISES (20);
                        status = STATUS_WORKING_SET_QUOTA;
                        goto FailureUnlockAnyPages;
                    }
    
                    InterlockedIncrement (&Pfn1->AweReferenceCount);
    
                    *Page = PageFrameIndex;
                    Page += 1;
    
                    //
                    // Captured this page, march on to the next one.
                    //

                    continue;
                }
            }

            AweOk = FALSE;
        }
        else {

            //
            // The access mode must be kernel to reference kernel addresses.
            //
    
            if (AccessMode != KernelMode) {

                //
                // Order the checks from hot to cool (lockwise).
                //

                ASSERT (LockType != LOCK_TYPE_PFN);

                if (LockType == LOCK_TYPE_WS) {
                    UNLOCK_WS_SHARED (Thread, CurrentProcess);
                }
                else if (LockType == LOCK_TYPE_AWE) {
                    ExReleaseCacheAwarePushLockShared (PushLock);
                    KeLeaveGuardedRegionThread (&Thread->Tcb);
                }

                MI_INSTRUMENT_PROBE_RAISES (21);
                status = STATUS_ACCESS_VIOLATION;
                goto FailureUnlockAnyPages;
            }

            ASSERT (CurrentProcess == NULL);
        }

DefaultProbeAndLock:

        if (!MI_IS_PHYSICAL_ADDRESS (Va)) {
            PointerPte = MiGetPteAddress (Va);
        }
        else {
    
            //
            // Set PointerPte to NULL to indicate this is a
            // physical address range.
            //
    
            if (Va <= MM_HIGHEST_USER_ADDRESS) {
                PointerPte = MiGetPteAddress (Va);
            }
            else {
                PointerPte = NULL;
            }
        }

        PointerPxe = MiGetPxeAddress (Va);
        PointerPpe = MiGetPpeAddress (Va);
        PointerPde = MiGetPdeAddress (Va);
    
        if (Va <= MM_HIGHEST_USER_ADDRESS) {
    
            //
            // These are user space addresses, check them carefully.
            //
    
            ASSERT (NumberOfPagesToLock != 0);
    
            ASSERT (CurrentProcess == PsGetCurrentProcess ());
    
            //
            // Initialize the MDL process field (assume the probe will succeed).
            //
    
            MemoryDescriptorList->Process = CurrentProcess;
    
            if (LockedPagesCharged == FALSE) {
                InterlockedExchangeAddSizeT (&CurrentProcess->NumberOfLockedPages,
                                             NumberOfPagesToLock);
                LockedPagesCharged = TRUE;
            }

            if (LockType != LOCK_TYPE_WS) {
                ASSERT (LockType == LOCK_TYPE_NONE);
                LockType = LOCK_TYPE_WS;
                LOCK_WS_SHARED (Thread, CurrentProcess);
            }
        }
        else {
    
            ASSERT (CurrentProcess == NULL);
    
            MemoryDescriptorList->Process = NULL;
    
            Va = (PCHAR)Va + MemoryDescriptorList->ByteOffset;
    
            if (PointerPte == NULL) {
    
                //
                // On certain architectures, virtual addresses
                // may be physical and hence have no corresponding PTE.
                //
    
                PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (Va);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
                ASSERT ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0);
    
                //
                // Acquire the PFN database lock.
                //
        
                if (LockType == LOCK_TYPE_NONE) {
                    LockType = LOCK_TYPE_PFN;
                    LOCK_PFN2 (OldIrql);
                }
                else {
                    ASSERT (LockType == LOCK_TYPE_PFN);
                }
    
                if (MI_IS_PFN (PageFrameIndex)) {
    
                    //
                    // Check to make sure each page is not locked down an
                    // unusually high number of times.
                    //
        
                    if (Pfn1->u3.e2.ReferenceCount >= MmReferenceCountCheck) {
                        UNLOCK_PFN2 (OldIrql);
                        ASSERT (FALSE);
                        status = STATUS_WORKING_SET_QUOTA;
                        MI_INSTRUMENT_PROBE_RAISES (22);
                        goto FailureUnlockAnyPages;
                    }
        
                    if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                        MI_SNAP_DIRTY (Pfn1, 1, 0x99);
                    }
    
                    MI_ADD_LOCKED_PAGE_CHARGE (Pfn1);
                }
                else {
                    MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
                }
    
                *Page = PageFrameIndex;
                Page += 1;

                //
                // Captured this page, march on to the next one.
                //

                continue;
            }
    
            //
            // Since this operation is to a system address, no need to check for
            // PTE write access below so mark the access as a read so only the
            // operation type (and not where the Va is) needs to be checked in
            // the subsequent loop.
            //
    
            Operation = IoReadAccess;
    
            //
            // A driver may build an MDL for a user address and then get
            // a system VA for it.  The driver may then call MmProbeAndLockPages
            // again, this time with the system VA (instead of the original
            // user VA) for the same range.
            //
            // Thus the PFNs will really point at PFNs which generally
            // require the relevant process' working set pushlock, but in
            // this instance we would only be holding the system working set
            // pushlock.  Therefore the user can change PFN sharecounts and
            // active states at any time.  The only thing we'd know for sure
            // is that the refcount must be nonzero on entry and will stay
            // that way because it would be illegal for the driver to
            // have called MmUnlockPages prior to this probe call.
            //
            // However, this is not enough state to ensure we charge locking
            // properly (without leaking) so the code that used to acquire the
            // system working set mutex here has been changed to instead always
            // use the PFN lock to synchronize system space VA arguments.
            //
        
            if (LockType == LOCK_TYPE_NONE) {
                LockType = LOCK_TYPE_PFN;
                LOCK_PFN2 (OldIrql);
            }
            else {
                ASSERT (LockType == LOCK_TYPE_PFN);
            }
        }
    
        while (
#if (_MI_PAGING_LEVELS>=4)
               (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS>=3)
               (PointerPpe->u.Hard.Valid == 0) ||
#endif
               ((PointerPde->u.Hard.Valid == 0) ||
                (((MI_PDE_MAPS_LARGE_PAGE (PointerPde)) == 0) &&
                 (PointerPte->u.Hard.Valid == 0)))) {
    
            //
            // The VA is not resident, release the PFN lock and access the page
            // to make it appear.
            //
    
            if (LockType == LOCK_TYPE_WS) {
                UNLOCK_WS_SHARED (Thread, CurrentProcess);
            }
            else {
                ASSERT (LockType == LOCK_TYPE_PFN);
                UNLOCK_PFN2 (OldIrql);
            }
    
            MmSavePageFaultReadAhead (Thread, &SavedState);
            MmSetPageFaultReadAhead (Thread, 0);
    
            status = MmAccessFault (FALSE, Va, KernelMode, NULL);
    
            MmResetPageFaultReadAhead (Thread, SavedState);
    
            if (!NT_SUCCESS(status)) {
                MI_INSTRUMENT_PROBE_RAISES (23);
                goto FailureUnlockAnyPages;
            }
    
            if (LockType == LOCK_TYPE_WS) {
                LOCK_WS_SHARED (Thread, CurrentProcess);
            }
            else {
                ASSERT (LockType == LOCK_TYPE_PFN);
                LOCK_PFN2 (OldIrql);
            }
        }
    
        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
    
            if ((Operation != IoReadAccess) &&
                (PointerPde->u.Hard.Write == 0)) {
    
                MI_INSTRUMENT_PROBE_RAISES (24);
                status = STATUS_ACCESS_VIOLATION;
                goto FailureReleaseLocks;
            }
    
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (Va);
        }
        else {
    
#if defined (_X86PAE_)

            //
            // Pick up the PTE as an interlocked operation to prevent AWE
            // updates from tearing our read on these platforms.
            //

            PteContents.u.Long = InterlockedCompareExchangePte (PointerPte,
                                                                BogusPte.u.Long,
                                                                BogusPte.u.Long);
            ASSERT (PteContents.u.Long != BogusPte.u.Long);

#else
            PteContents = *PointerPte;
#endif
    
            //
            // There is a subtle race here where the PTE contents can get zeroed
            // by a thread running on another processor.  This can only happen
            // for an AWE address space because these ranges (deliberately for
            // performance reasons) do not acquire the PFN lock during remap
            // operations.  In this case, one of 2 scenarios is possible -
            // either the old PTE is read or the new.  The new may be a zero
            // PTE if the map request was to invalidate *or* non-zero (and
            // valid) if the map request was inserting a new entry.  For the
            // latter, we don't care if we lock the old or new frame here as
            // it's an application bug to provoke this behavior - and
            // regardless of which is used, no corruption can occur because
            // the PFN lock is acquired during an NtFreeUserPhysicalPages.
            // But the former must be checked for explicitly here.  As a
            // separate note, the PXE/PPE/PDE accesses above are always safe
            // even for the AWE deletion race because these tables
            // are never lazy-allocated for AWE ranges.
            //
    
            if (PteContents.u.Hard.Valid == 0) {
                ASSERT (PteContents.u.Long == 0);
                ASSERT (PsGetCurrentProcess ()->AweInfo != NULL);
    
                MI_INSTRUMENT_PROBE_RAISES (25);
                status = STATUS_ACCESS_VIOLATION;
                goto FailureReleaseLocks;
            }
    
            if (Operation != IoReadAccess) {
    
                if ((PteContents.u.Long & MM_PTE_WRITE_MASK) == 0) {
    
                    if (PteContents.u.Long & MM_PTE_COPY_ON_WRITE_MASK) {
    
                        //
                        // The protection has changed from writable to copy on
                        // write.  This can happen if a fork is in progress for
                        // example.  Restart the operation at the top.
                        //
    
                        if (Va <= MM_HIGHEST_USER_ADDRESS) {
    
                            if (LockType == LOCK_TYPE_WS) {
                                UNLOCK_WS_SHARED (Thread, CurrentProcess);
                            }
                            else {
                                ASSERT (LockType == LOCK_TYPE_PFN);
                                UNLOCK_PFN2 (OldIrql);
                            }
    
                            MmSavePageFaultReadAhead (Thread, &SavedState);
                            MmSetPageFaultReadAhead (Thread, 0);
    
                            status = MmAccessFault (TRUE, Va, KernelMode, NULL);
    
                            MmResetPageFaultReadAhead (Thread, SavedState);
    
                            if (!NT_SUCCESS(status)) {
                                MI_INSTRUMENT_PROBE_RAISES (19);
                                goto FailureUnlockAnyPages;
                            }
    
                            //
                            // Retry this VA again.
                            //

                            SegmentArray -= 1;

                            if (LockType == LOCK_TYPE_WS) {
                                LOCK_WS_SHARED (Thread, CurrentProcess);
                            }
                            else {
                                ASSERT (LockType == LOCK_TYPE_PFN);
                                LOCK_PFN2 (OldIrql);
                            }
    
                            continue;
                        }
                    }
    
                    //
                    // The caller has made the page protection more
                    // restrictive, this should never be done once the
                    // request has been issued !  Rather than wading
                    // through the PFN database entry to see if it
                    // could possibly work out, give the caller an
                    // access violation.
                    //
    
#if DBG
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MmProbeAndLockPages: PTE %p %p changed\n",
                        PointerPte,
                        PteContents.u.Long);
    
                    if (MmStopOnBadProbe) {
                        DbgBreakPoint ();
                    }
#endif
    
                    MI_INSTRUMENT_PROBE_RAISES (26);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }
            }
    
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }

        //
        // We have a PFN that is mapped by the caller.  If this is a user
        // space mapping, check to ensure the frame is owned by the caller.
        // The only way this isn't TRUE is if the caller has a
        // \Device\PhysicalMemory mapping for frames he doesn't own.
        //
    
        if (MI_IS_PFN (PageFrameIndex)) {
    
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
            ASSERT ((CurrentProcess == NULL) || (LockType == LOCK_TYPE_WS));
    
            PfnHeldToo = FALSE;

            if ((CurrentProcess != NULL) && (CurrentProcess->PhysicalVadRoot != NULL)) {
    
                //
                // This process has a \Device\PhysicalMemory VAD so it must be
                // checked to see if the current address resides in it.
                //
    
                SearchResult = MiFindNodeOrParent (CurrentProcess->PhysicalVadRoot,
                                                   MI_VA_TO_VPN (Va),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    (PhysicalView->VadType == VadDevicePhysicalMemory)) {
    
                    ASSERT ((ULONG)PhysicalView->Vad->u.VadFlags.VadType == (ULONG)PhysicalView->VadType);
    
                    //
                    // The VA lies within a device physical memory VAD.
                    //
                    // The PTE was already checked for read/write permissions
                    // above, but the PFN must still be checked for ownership
                    // and this must be done with the PFN lock since this
                    // process is not necessarily the correct owner.
                    //
    
                    PfnHeldToo = TRUE;

                    LOCK_PFN (OldIrql);

                    if (Pfn1->u3.e1.PageLocation != ActiveAndValid) {
                        UNLOCK_PFN (OldIrql);
                        MI_INSTRUMENT_PROBE_RAISES (33);
                        status = STATUS_ACCESS_VIOLATION;
                        goto FailureReleaseLocks;
                    }

                    //
                    // Continue to hold the PFN lock until the reference count
                    // on the desired PFN is incremented.
                    //
                }
            }

            if (MI_ADD_LOCKED_PAGE_CHARGE_FOR_PROBE (Pfn1, FALSE, PointerPte) == FALSE) {
                if (PfnHeldToo == TRUE) {
                    UNLOCK_PFN (OldIrql);
                }
    
                //
                // If this page is for privileged code/data,
                // then force it in regardless.
                //
    
                Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
                if ((Va < MM_HIGHEST_USER_ADDRESS) ||
                    (MI_IS_SYSTEM_CACHE_ADDRESS(Va)) ||
                    ((Va >= MmPagedPoolStart) && (Va <= MmPagedPoolEnd))) {
    
                    MI_INSTRUMENT_PROBE_RAISES (27);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto FailureReleaseLocks;
                }
    
                if (MI_ADD_LOCKED_PAGE_CHARGE_FOR_PROBE (Pfn1, TRUE, PointerPte) == FALSE) {
    
                    //
                    // The only reason we don't force for privileged code/data
                    // is if it would have caused a reference count wrap.
                    // Typically this indicates a driver is forgetting to call
                    // MmUnlockPages as the page is locked down an unusually
                    // high number of times.
                    //
    
                    MI_INSTRUMENT_PROBE_RAISES (28);
                    status = STATUS_WORKING_SET_QUOTA;
                    goto FailureReleaseLocks;
                }
            }
            else if (PfnHeldToo == TRUE) {
                UNLOCK_PFN (OldIrql);
            }
    
            if (MemoryDescriptorList->MdlFlags & MDL_WRITE_OPERATION) {
                MI_SNAP_DIRTY (Pfn1, 1, 0x98);
            }
        }
        else {
    
            //
            // This is an I/O space address - there is no PFN database entry
            // for it, so no reference counts may be modified for these pages.
            //
            // Don't charge page locking for this page, just add it to the MDL.
            //
    
            if (CurrentProcess != NULL) {
    
                //
                // The VA better be within a \Device\PhysicalMemory VAD.
                //
    
                if (CurrentProcess->PhysicalVadRoot == NULL) {
                    MI_INSTRUMENT_PROBE_RAISES (29);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }
    
                Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
                SearchResult = MiFindNodeOrParent (CurrentProcess->PhysicalVadRoot,
                                                   MI_VA_TO_VPN (Va),
                                                   (PMMADDRESS_NODE *) &PhysicalView);
                if ((SearchResult == TableFoundNode) &&
                    ((PhysicalView->VadType == VadRotatePhysical) ||
                     (PhysicalView->VadType == VadDevicePhysicalMemory))) {
    
                    ASSERT ((ULONG)PhysicalView->Vad->u.VadFlags.VadType == (ULONG)PhysicalView->VadType);
    
                    //
                    // The range lies within a rotate physical VAD or a
                    // device physical memory VAD.
                    //
                    // We already know the access is ok because the PTE was
                    // already checked for read/write permissions above.
                    //
    
                    NOTHING;
                }
                else {
                    MI_INSTRUMENT_PROBE_RAISES (30);
                    status = STATUS_ACCESS_VIOLATION;
                    goto FailureReleaseLocks;
                }
            }
    
            if (((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) &&
                (CurrentProcess != NULL)) {
    
                EntryCounter = *(volatile PLONG) &MiActiveIoCounter;
            }
    
            MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
        }
    
        *Page = PageFrameIndex;
        Page += 1;
    }

    //
    // Order the checks from hot to cool (lockwise).
    //

    if (LockType == LOCK_TYPE_PFN) {
        UNLOCK_PFN2 (OldIrql);
    }
    else if (LockType == LOCK_TYPE_WS) {
        UNLOCK_WS_SHARED (Thread, CurrentProcess);
    }
    else if (LockType == LOCK_TYPE_AWE) {
        ExReleaseCacheAwarePushLockShared (PushLock);
        KeLeaveGuardedRegionThread (&Thread->Tcb);
    }

    if (Va <= MM_HIGHEST_USER_ADDRESS) {

        //
        // User space buffers that reside in I/O space need to be reference
        // counted because SANs will want to reuse the physical space but cannot
        // do this unless it is guaranteed there are no more pending I/Os
        // going from/to it.
        //

        if (MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) {

            ASSERT ((MemoryDescriptorList->MdlFlags & MDL_DESCRIBES_AWE) == 0);

            if (MiReferenceIoSpace (MemoryDescriptorList, Page) == FALSE) {
                MI_INSTRUMENT_PROBE_RAISES (31);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto FailureUnlockAnyPages;
            }

            if (EntryCounter != *(volatile PLONG) &MiActiveIoCounter) {

                //
                // The counter has changed since we captured the first PTE.
                // Just repeat everything.
                //

                ASSERT (LockedPagesCharged == TRUE);
                MiDereferenceIoSpace (MemoryDescriptorList);

                if (MmTrackLockedPages == TRUE) {

                    RtlGetCallersAddress (&CallingAddress, &CallersCaller);
            
                    MiAddMdlTracker (MemoryDescriptorList,
                                     CallingAddress,
                                     CallersCaller,
                                     NumberOfPagesToLock,
                                     6);
                }

                MmUnlockPages (MemoryDescriptorList);
                LockedPagesCharged = FALSE;
                goto Top;
            }
        }
    }

    if (EntrySegment != (PVOID)&StackArray[0]) {
        ExFreePool (EntrySegment);
    }

    if ((MmTrackLockedPages == TRUE) &&
        ((MemoryDescriptorList->MdlFlags & MDL_DESCRIBES_AWE) == 0)) {

        ASSERT (NumberOfPagesToLock != 0);

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         NumberOfPagesToLock,
                         7);
    }

    return;

FailureReleaseLocks:

    //
    // A violation was detected.  Release the synchronization primitive
    // we used and unlock any pages locked so far.
    //

    ASSERT (!NT_SUCCESS (status));

    ASSERT (LockType > LOCK_TYPE_AWE);

    if (LockType == LOCK_TYPE_PFN) {
        UNLOCK_PFN2 (OldIrql);
    }
    else {
        ASSERT (LockType == LOCK_TYPE_WS);
        UNLOCK_WS_SHARED (Thread, CurrentProcess);
    }

FailureUnlockAnyPages:

    //
    // An exception occurred, unlock any pages locked so far.  Note that
    // a tracker entry must be inserted for MmUnlockPages to find.
    //

    ASSERT (MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED);

    if ((MmTrackLockedPages == TRUE) &&
        ((MemoryDescriptorList->MdlFlags & MDL_DESCRIBES_AWE) == 0)) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MiAddMdlTracker (MemoryDescriptorList,
                         CallingAddress,
                         CallersCaller,
                         NumberOfPagesToLock,
                         8);
    }
    
    MmUnlockPages (MemoryDescriptorList);

    if (EntrySegment != (PVOID)&StackArray[0]) {
        ExFreePool (EntrySegment);
    }

    //
    // Raise an exception of access violation to the caller.
    //

    ExRaiseStatus (status);
}

VOID
MiDecrementReferenceCountForAwePage (
    IN PMMPFN Pfn1,
    IN LOGICAL PfnHeld
    )

/*++

Routine Description:

    This routine decrements the reference count for an AWE-allocated page.
    Descriptor List.  If this decrements the count to zero, the page is
    put on the freelist and various resident available and commitment
    counters are updated.

Arguments:

    Pfn - Supplies a pointer to the PFN database element for the physical
          page to decrement the reference count for.

    PfnHeld - Supplies TRUE if the caller holds the PFN lock.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;

    if (PfnHeld == FALSE) {
        LOCK_PFN2 (OldIrql);
    }
    else {
        OldIrql = PASSIVE_LEVEL;
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    ASSERT (Pfn1->AweReferenceCount == 0);
    ASSERT (Pfn1->u4.AweAllocation == 1);

    if (Pfn1->u3.e2.ReferenceCount >= 2) {
        InterlockedDecrement16 ((PSHORT)&Pfn1->u3.e2.ReferenceCount);
        if (PfnHeld == FALSE) {
            UNLOCK_PFN2 (OldIrql);
        }
    }
    else {

        //
        // This is the final dereference - the page was sitting in
        // limbo (not on any list) waiting for this last I/O to complete.
        //

        ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
        ASSERT (Pfn1->u2.ShareCount == 0);
        MiDecrementReferenceCount (Pfn1, MI_PFN_ELEMENT_TO_INDEX (Pfn1));

        if (PfnHeld == FALSE) {
            UNLOCK_PFN2 (OldIrql);
        }
    }

    MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_AWE);

    MiReturnCommitment (1);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_MDL_PAGES, 1);

    InterlockedDecrementSizeT (&MmMdlPagesAllocated);

    return;
}

SINGLE_LIST_ENTRY MmLockedIoPagesHead;

LOGICAL
MiReferenceIoSpace (
    IN OUT PMDL MemoryDescriptorList,
    IN PPFN_NUMBER Page
    )

/*++

Routine Description:

    This routine reference counts physical pages which reside in I/O space
    when they are probed on behalf of a user-initiated transfer.  These counts
    cannot be kept inside PFN database entries because there are no PFN entries
    for I/O space.
    
    These counts are kept because SANs will want to reuse the physical space
    but cannot do this unless it is guaranteed there are no more pending I/Os
    going from/to it.  If the process has not exited, but the SAN driver has
    unmapped the views to its I/O space, it still needs this as a way to know
    that the application does not have a transfer in progress to the previously
    mapped space.

Arguments:

    MemoryDescriptorList - Supplies a pointer to the memory descriptor list.

    Page - Supplies a pointer to the PFN just after the end of the MDL.

Return Value:

    TRUE if the reference counts were updated, FALSE if not.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PMDL Tracker;
    KIRQL OldIrql;
    SIZE_T MdlSize;

    MdlSize = (PCHAR)Page - (PCHAR)MemoryDescriptorList;

    Tracker = ExAllocatePoolWithTag (NonPagedPool, MdlSize, 'tImM');

    if (Tracker == NULL) {
        return FALSE;
    }

    RtlCopyMemory ((PVOID) Tracker,
                   (PVOID) MemoryDescriptorList,
                   MdlSize);

    Tracker->MappedSystemVa = (PVOID) MemoryDescriptorList;

    //
    // Add this transfer to the list.
    //

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    PushEntryList (&MmLockedIoPagesHead, (PSINGLE_LIST_ENTRY) Tracker);

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return TRUE;
}

NTKERNELAPI
LOGICAL
MiDereferenceIoSpace (
    IN OUT PMDL MemoryDescriptorList
    )

/*++

Routine Description:

    This routine decrements the reference counts on physical pages which
    reside in I/O space that were previously probed on behalf of a
    user-initiated transfer.  These counts cannot be kept inside PFN
    database entries because there are no PFN entries for I/O space.
    
    These counts are kept because SANs will want to reuse the physical space
    but cannot do this unless it is guaranteed there are no more pending I/Os
    going from/to it.  If the process has not exited, but the SAN driver has
    unmapped the views to its I/O space, it still needs this as a way to know
    that the application does not have a transfer in progress to the previously
    mapped space.

Arguments:

    MemoryDescriptorList - Supplies a pointer to the memory descriptor list.

Return Value:

    TRUE if the reference counts were updated, FALSE if not.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    PMDL PrevEntry;
    PMDL NextEntry;

    PrevEntry = NULL;

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    NextEntry = (PMDL) MmLockedIoPagesHead.Next;

    while (NextEntry != NULL) {

        if (NextEntry->MappedSystemVa == (PVOID) MemoryDescriptorList) {

            if (PrevEntry != NULL) {
                PrevEntry->Next = NextEntry->Next;
            }
            else {
                MmLockedIoPagesHead.Next = (PSINGLE_LIST_ENTRY) NextEntry->Next;
            }

            ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

            ExFreePool (NextEntry);

            return TRUE;
        }

        PrevEntry = NextEntry;
        NextEntry = NextEntry->Next;
    }

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return FALSE;
}

LOGICAL
MmIsIoSpaceActive (
    __in PHYSICAL_ADDRESS StartAddress,
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This routine returns TRUE if any portion of the requested range still has
    an outstanding pending I/O.  It is the calling driver's responsibility to
    unmap all usermode mappings to the specified range (so another transfer
    cannot be initiated) prior to calling this API.
    
    These counts are kept because SANs will want to reuse the physical space
    but cannot do this unless it is guaranteed there are no more pending I/Os
    going from/to it.  If the process has not exited, but the SAN driver has
    unmapped the views to its I/O space, it still needs this as a way to know
    that the application does not have a transfer in progress to the previously
    mapped space.

Arguments:

    StartAddress - Supplies the physical address of the start of the I/O range.
                   This MUST NOT be within system DRAM as those pages are not
                   tracked by this structure.

    NumberOfBytes - Supplies the number of bytes in the range.

Return Value:

    TRUE if any page in the range is currently locked for I/O, FALSE if not.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    PMDL MemoryDescriptorList;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    PHYSICAL_ADDRESS EndAddress;

    ASSERT (NumberOfBytes != 0);
    StartPage = (PFN_NUMBER) (StartAddress.QuadPart >> PAGE_SHIFT);
    EndAddress.QuadPart = StartAddress.QuadPart + NumberOfBytes - 1;
    EndPage = (PFN_NUMBER) (EndAddress.QuadPart >> PAGE_SHIFT);

#if DBG
    do {
        ASSERT (!MI_IS_PFN (StartPage));
        StartPage += 1;
    } while (StartPage <= EndPage);
    StartPage = (PFN_NUMBER) (StartAddress.QuadPart >> PAGE_SHIFT);
#endif

    InterlockedIncrement (&MiActiveIoCounter);

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    MemoryDescriptorList = (PMDL) MmLockedIoPagesHead.Next;

    while (MemoryDescriptorList != NULL) {

        StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

        ASSERT (NumberOfPages != 0);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        LastPage = Page + NumberOfPages;

        do {

            if (*Page == MM_EMPTY_LIST) {

                //
                // There are no more locked pages.
                //

                break;
            }

            if ((*Page >= StartPage) && (*Page <= EndPage)) {
                ExReleaseSpinLock (&MiTrackIoLock, OldIrql);
                return TRUE;
            }

            Page += 1;

        } while (Page < LastPage);

        MemoryDescriptorList = MemoryDescriptorList->Next;
    }

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return FALSE;
}

LOGICAL
MiIsProbeActive (
    IN PMMPTE InputPte,
    IN PFN_NUMBER InputPages
    )

/*++

Routine Description:

    This routine returns TRUE if any portion of the requested range still has
    an outstanding pending I/O.
    
    These counts are kept because drivers will want to remap the physical space
    but cannot do this unless it is guaranteed there are no more pending I/Os
    going from/to it.

Arguments:

    InputPte - Supplies the starting PTE of the range to check.

    InputPages - Supplies the number of pages to check.

Return Value:

    TRUE if any page in the range is currently locked for I/O, FALSE if not.

Environment:

    Kernel mode, working set pushlock held, APC_LEVEL.

--*/

{
    PMMPTE PointerPte;
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    PMDL MemoryDescriptorList;
    PFN_NUMBER PageFrameIndex;

    InterlockedIncrement (&MiActiveIoCounter);

    ExAcquireSpinLock (&MiTrackIoLock, &OldIrql);

    MemoryDescriptorList = (PMDL) MmLockedIoPagesHead.Next;

    while (MemoryDescriptorList != NULL) {

        StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

        ASSERT (NumberOfPages != 0);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        LastPage = Page + NumberOfPages;

        do {

            if (*Page == MM_EMPTY_LIST) {

                //
                // There are no more locked pages.
                //

                break;
            }

            for (PointerPte = InputPte; PointerPte < InputPte + InputPages; PointerPte += 1) {
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                if (*Page == PageFrameIndex) {
                    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);
                    return TRUE;
                }
            }

            Page += 1;

        } while (Page < LastPage);

        MemoryDescriptorList = MemoryDescriptorList->Next;
    }

    ExReleaseSpinLock (&MiTrackIoLock, OldIrql);

    return FALSE;
}


VOID
MmUnlockPages (
     __inout PMDL MemoryDescriptorList
     )

/*++

Routine Description:

    This routine unlocks physical pages which are described by a Memory
    Descriptor List.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a memory descriptor list
                           (MDL). The supplied MDL must have been supplied
                           to MmLockPages to lock the pages down.  As the
                           pages are unlocked, the MDL is updated.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    LONG EntryCount;
    LONG OriginalCount;
    PVOID OldValue;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    PEPROCESS MdlProcess;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PVOID StartingVa;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PSLIST_ENTRY SingleListEntry;
    PMI_PFN_DEREFERENCE_CHUNK DerefMdl;
    PSLIST_HEADER PfnDereferenceSListHead;
    PSLIST_ENTRY *PfnDeferredList;

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED) != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) == 0);
    ASSERT (MemoryDescriptorList->ByteCount != 0);

    MdlProcess = MemoryDescriptorList->Process;

    //
    // Carefully snap a copy of the MDL flags - realize that bits in it may
    // change due to some of the subroutines called below.  Only bits that
    // we know can't change are examined in this local copy.  This is done
    // to reduce the amount of processing while the PFN lock is held.
    //

    MdlFlags = MemoryDescriptorList->MdlFlags;

    if (MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

        //
        // This MDL has been mapped into system space, unmap now.
        //

        MmUnmapLockedPages (MemoryDescriptorList->MappedSystemVa,
                            MemoryDescriptorList);
    }

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa,
                                              MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    if (MdlFlags & MDL_DESCRIBES_AWE) {

        ASSERT (MdlProcess != NULL);
        ASSERT (MdlProcess->AweInfo != NULL);

        LastPage = Page + NumberOfPages;

        //
        // Note neither AWE nor PFN locks are needed for unlocking these MDLs
        // in all but the very rare cases (see below).
        //

        do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
            if (*Page == MM_EMPTY_LIST) {

                //
                // There are no more locked pages - if there were none at all
                // then we're done.
                //

                break;
            }

            ASSERT (MI_IS_PFN (*Page));
            ASSERT (*Page <= MmHighestPhysicalPage);
            Pfn1 = MI_PFN_ELEMENT (*Page);

            do {
                EntryCount = ReadForWriteAccess (&Pfn1->AweReferenceCount);

                ASSERT ((LONG)EntryCount > 0);
                ASSERT (Pfn1->u4.AweAllocation == 1);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

                OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                            EntryCount - 1,
                                                            EntryCount);

                if (OriginalCount == EntryCount) {

                    //
                    // This thread can be racing against other threads also
                    // calling MmUnlockPages and also a thread calling
                    // NtFreeUserPhysicalPages.  All threads can safely do
                    // interlocked decrements on the "AWE reference count".
                    // Whichever thread drives it to zero is responsible for
                    // decrementing the actual PFN reference count (which may
                    // be greater than 1 due to other non-AWE API calls being
                    // used on the same page).  The thread that drives this
                    // reference count to zero must put the page on the actual
                    // freelist at that time and decrement various resident
                    // available and commitment counters also.
                    //

                    if (OriginalCount == 1) {

                        //
                        // This thread has driven the AWE reference count to
                        // zero so it must initiate a decrement of the PFN
                        // reference count (while holding the PFN lock), etc.
                        //
                        // This path should be rare since typically I/Os
                        // complete before these types of pages are freed by
                        // the app.
                        //

                        MiDecrementReferenceCountForAwePage (Pfn1, FALSE);
                    }

                    break;
                }
            } while (TRUE);

            Page += 1;
        } while (Page < LastPage);

        MemoryDescriptorList->MdlFlags &= ~(MDL_PAGES_LOCKED | MDL_DESCRIBES_AWE);

        return;
    }

    if (MmTrackLockedPages == TRUE) {
        MiFreeMdlTracker (MemoryDescriptorList, NumberOfPages);
    }

    //
    // Try for the fast path if not I/O space.
    //

    if ((MdlFlags & MDL_IO_SPACE) == 0) {

        CurrentThread = NULL;
        CurrentProcess = NULL;

        if (MdlProcess != NULL) {
            ASSERT ((SPFN_NUMBER)MdlProcess->NumberOfLockedPages >= 0);
            InterlockedExchangeAddSizeT (&MdlProcess->NumberOfLockedPages,
                                         0 - NumberOfPages);

            //
            // See if this was a transfer from user space and we are in the
            // initiating process context.  If so, set Thread to indicate
            // the fastpath (using the working set pushlock instead of the PFN
            // lock) should be used.
            //

            if (((MdlFlags & MDL_WRITE_OPERATION) == 0) &&
                (KeGetCurrentIrql () == PASSIVE_LEVEL)) {

                CurrentThread = PsGetCurrentThread ();
                CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);
            }
        }

        LastPage = Page + NumberOfPages;

        //
        // Calculate PFN addresses and termination without the PFN lock
        // (it's not needed for this) to reduce PFN lock contention.
        //

        ASSERT (sizeof(PFN_NUMBER) == sizeof(PMMPFN));

        do {

            if (*Page == MM_EMPTY_LIST) {

                //
                // There are no more locked pages - if there were none at all
                // then we're done.
                //

                if (Page == (PPFN_NUMBER)(MemoryDescriptorList + 1)) {
                    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;
                    return;
                }

                LastPage = Page;
                break;
            }
            ASSERT (MI_IS_PFN (*Page));
            ASSERT (*Page <= MmHighestPhysicalPage);

            Pfn1 = MI_PFN_ELEMENT (*Page);
            *Page = (PFN_NUMBER) Pfn1;
            Page += 1;
        } while (Page < LastPage);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        //
        // If this was a transfer from user space and we are in the
        // initiating process context then take advantage of this fact
        // and unlock the pages using the process' working set pushlock
        // instead of the PFN lock.
        //

        if (CurrentThread != NULL) {

            if (CurrentProcess == MdlProcess) {
                LOCK_WS_SHARED (CurrentThread, MdlProcess);
            }
            else {
                LOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
            }

            //
            // If all the pages are still valid in the process' address space
            // then process the pages directly here without the PFN lock.
            // Note if any of the pages are no longer valid in this process'
            // address space then the entire MDL must be processed under the
            // PFN lock.
            //

            do {
                Pfn1 = (PMMPFN) (*Page);

                if (Pfn1->u2.ShareCount >= 1) {

                    if (Pfn1->u3.e1.PrototypePte == 0) {

                        //
                        // This page is still mapped and it's private so it
                        // must be mapped in this process.
                        //

                        NOTHING;
                    }
                    else if (CurrentProcess != MdlProcess) {

                        //
                        // This is a shared page so the working set pushlock
                        // is not sufficient - the PXE/PPE/PDE/PTE mappings
                        // must be checked.
                        //
                        // Since we are not in the address space of the MDL's
                        // initiating process, it's not worth the cost of
                        // attaching to interrogate.  Just go the expensive
                        // (PFN) way instead.
                        //

                        UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                        CurrentThread = NULL;
                        break;
                    }
                    else {

                        //
                        // This is a sharable page so the current process'
                        // working set pushlock is not enough to check
                        // the PFN contents.  The PXE/PPE/PDE/PTE hierarchy
                        // must be checked to ensure the user has not deleted
                        // (and replaced) the original virtual address space.
                        //

                        PVOID TempVa;
                        PFN_NUMBER PageFrameIndex;
#if (_MI_PAGING_LEVELS>=4)
                        PMMPTE PointerPxe;
#endif
#if (_MI_PAGING_LEVELS>=3)
                        PMMPTE PointerPpe;
#endif
                        PMMPTE PointerPde;
                        PMMPTE PointerPte;

                        TempVa = (PVOID) ((PCHAR)MemoryDescriptorList->StartVa + ((Page -
                                    (PPFN_NUMBER)(MemoryDescriptorList + 1)) << PAGE_SHIFT));

#if (_MI_PAGING_LEVELS>=4)
                        PointerPxe = MiGetPxeAddress (TempVa);
                        if (PointerPxe->u.Hard.Valid == 0) {
                            if (CurrentProcess == MdlProcess) {
                                UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                            }
                            else {
                                UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                            }
                            CurrentThread = NULL;
                            break;
                        }
#endif
#if (_MI_PAGING_LEVELS>=3)
                        PointerPpe = MiGetPpeAddress (TempVa);
                        if (PointerPpe->u.Hard.Valid == 0) {
                            if (CurrentProcess == MdlProcess) {
                                UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                            }
                            else {
                                UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                            }
                            CurrentThread = NULL;
                            break;
                        }
#endif
                        PointerPde = MiGetPdeAddress (TempVa);

                        if (PointerPde->u.Hard.Valid == 0) {
                            if (CurrentProcess == MdlProcess) {
                                UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                            }
                            else {
                                UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                            }
                            CurrentThread = NULL;
                            break;
                        }
                        
                        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
                            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (TempVa);
                        }
                        else {
                            PointerPte = MiGetPteAddress (TempVa);

                            if (PointerPte->u.Hard.Valid == 0) {
                                if (CurrentProcess == MdlProcess) {
                                    UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                                }
                                else {
                                    UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                                }
                                CurrentThread = NULL;
                                break;
                            }

                            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                        }
                        if (MI_PFN_ELEMENT (PageFrameIndex) != (PMMPFN) *Page) {
                            if (CurrentProcess == MdlProcess) {
                                UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                            }
                            else {
                                UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                            }
                            CurrentThread = NULL;
                            break;
                        }
                    }
                }
                else {
                    if (CurrentProcess == MdlProcess) {
                        UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                    }
                    else {
                        UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                    }
                    CurrentThread = NULL;
                    break;
                }

                ASSERT (Pfn1->u3.e2.ReferenceCount >= 2);

                Page += 1;
            } while (Page < LastPage);

            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

            if (CurrentThread != NULL) {
                do {
                    Pfn1 = (PMMPFN) (*Page);
                    ASSERT (Pfn1->u3.e2.ReferenceCount >= 2);
    
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
    
                    Page += 1;
                } while (Page < LastPage);
    
                if (CurrentProcess == MdlProcess) {
                    UNLOCK_WS_SHARED (CurrentThread, MdlProcess);
                }
                else {
                    UNLOCK_WS_SHARED_CROSS_PROCESS (CurrentThread, MdlProcess);
                }

                MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;
    
                return;
            }
        }

        //
        // If the MDL can be queued so the PFN acquisition/release can be
        // amortized then do so.
        //

        if (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK) {

#if defined(MI_MULTINODE)

            PKNODE Node = KeGetCurrentNode ();

            //
            // The node may change beneath us but that should be fairly
            // infrequent and not worth checking for.  Just make sure the
            // same node that gives us a free entry gets the deferred entry
            // back.
            //

            PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
#else
            PfnDereferenceSListHead = &MmPfnDereferenceSListHead;
#endif

            //
            // Pop an entry from the freelist.
            //

            SingleListEntry = InterlockedPopEntrySList (PfnDereferenceSListHead);

            if (SingleListEntry != NULL) {
                DerefMdl = CONTAINING_RECORD (SingleListEntry,
                                              MI_PFN_DEREFERENCE_CHUNK,
                                              ListEntry);

                DerefMdl->Flags = MdlFlags;
                DerefMdl->NumberOfPages = (USHORT) (LastPage - Page);

#if defined (_WIN64)

                //
                // Avoid the majority of the high cost of RtlCopyMemory on
                // 64 bit platforms.
                //

                if (DerefMdl->NumberOfPages == 1) {
                    DerefMdl->Pfns[0] = *Page;
                }
                else if (DerefMdl->NumberOfPages == 2) {
                    DerefMdl->Pfns[0] = *Page;
                    DerefMdl->Pfns[1] = *(Page + 1);
                }
                else
#endif
                RtlCopyMemory ((PVOID)(&DerefMdl->Pfns[0]),
                               (PVOID)Page,
                               (LastPage - Page) * sizeof (PFN_NUMBER));

                MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;

                //
                // Push this entry on the deferred list.
                //

#if defined(MI_MULTINODE)
                PfnDeferredList = &Node->PfnDeferredList;
#else
                PfnDeferredList = &MmPfnDeferredList;
#endif

                do {

                    OldValue = *PfnDeferredList;
                    SingleListEntry->Next = OldValue;

                } while (InterlockedCompareExchangePointer (
                                PfnDeferredList,
                                SingleListEntry,
                                OldValue) != OldValue);
                return;
            }
        }

        SingleListEntry = NULL;

        if (MdlFlags & MDL_WRITE_OPERATION) {

            LOCK_PFN2 (OldIrql);

            do {

                //
                // If this was a write operation set the modified bit in the
                // PFN database.
                //

                Pfn1 = (PMMPFN) (*Page);

                MI_SET_MODIFIED (Pfn1, 1, 0x3);

                if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {

                    ULONG FreeBit;
                    FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                    if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                        MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                        Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                    }
                }

                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                Page += 1;
            } while (Page < LastPage);
        }
        else {

            LOCK_PFN2 (OldIrql);

            do {

                Pfn1 = (PMMPFN) (*Page);

                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                Page += 1;
            } while (Page < LastPage);
        }

        if (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK) {

            //
            // The only reason this code path is being reached is because
            // a deferred entry was not available so clear the list now.
            //

            MiDeferredUnlockPages (MI_DEFER_PFN_HELD | MI_DEFER_DRAIN_LOCAL_ONLY);
        }

        UNLOCK_PFN2 (OldIrql);
    }
    else {

        LastPage = Page + NumberOfPages;

        if (MdlFlags & MDL_WRITE_OPERATION) {

            LOCK_PFN2 (OldIrql);

            do {

                if (*Page == MM_EMPTY_LIST) {
                    break;
                }

                if (MI_IS_PFN (*Page)) {

                    //
                    // If this was a write operation set the modified bit in the
                    // PFN database.
                    //
    
                    Pfn1 = MI_PFN_ELEMENT (*Page);
    
                    MI_SET_MODIFIED (Pfn1, 1, 0x3);
    
                    if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                        (Pfn1->u3.e1.WriteInProgress == 0)) {
    
                        ULONG FreeBit;
                        FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);
    
                        if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                            MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                        }
                    }
    
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
                }

                Page += 1;
            } while (Page < LastPage);
        }
        else {

            LOCK_PFN2 (OldIrql);

            do {

                if (*Page == MM_EMPTY_LIST) {
                    break;
                }

                if (MI_IS_PFN (*Page)) {
                    Pfn1 = MI_PFN_ELEMENT (*Page);
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
                }

                Page += 1;
            } while (Page < LastPage);
        }

        UNLOCK_PFN2 (OldIrql);

        if (MdlProcess != NULL) {
            ASSERT ((SPFN_NUMBER)MdlProcess->NumberOfLockedPages >= 0);
            InterlockedExchangeAddSizeT (&MdlProcess->NumberOfLockedPages,
                                         0 - NumberOfPages);
        }

        MiDereferenceIoSpace (MemoryDescriptorList);

        MemoryDescriptorList->MdlFlags &= ~MDL_IO_SPACE;
    }

    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;

    return;
}


VOID
MiDeferredUnlockPages (
     ULONG Flags
     )

/*++

Routine Description:

    This routine unlocks physical pages which were previously described by
    a Memory Descriptor List.

Arguments:

    Flags - Supplies a bitfield of the caller's needs :

        MI_DEFER_PFN_HELD - Indicates the caller holds the PFN lock on entry.

        MI_DEFER_DRAIN_LOCAL_ONLY - Indicates the caller only wishes to drain
                                    the current processor's queue.  This only
                                    has meaning in NUMA systems.

Return Value:

    None.

Environment:

    Kernel mode, PFN database lock *MAY* be held on entry (see Flags).

--*/

{
    KIRQL OldIrql = 0;
    ULONG FreeBit;
    ULONG i;
    ULONG ListCount;
    ULONG TotalNodes;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PSLIST_ENTRY SingleListEntry;
    PSLIST_ENTRY LastEntry;
    PSLIST_ENTRY FirstEntry;
    PSLIST_ENTRY NextEntry;
    PSLIST_ENTRY VeryLastEntry;
    PMI_PFN_DEREFERENCE_CHUNK DerefMdl;
    PSLIST_HEADER PfnDereferenceSListHead;
    PSLIST_ENTRY *PfnDeferredList;
#if defined(MI_MULTINODE)
    PKNODE Node;
#endif

    i = 0;
    ListCount = 0;
    TotalNodes = 1;

    if ((Flags & MI_DEFER_PFN_HELD) == 0) {
        LOCK_PFN2 (OldIrql);
    }

    MM_PFN_LOCK_ASSERT();

#if defined(MI_MULTINODE)
    if (Flags & MI_DEFER_DRAIN_LOCAL_ONLY) {
        Node = KeGetCurrentNode();
        PfnDeferredList = &Node->PfnDeferredList;
        PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
    }
    else {
        TotalNodes = KeNumberNodes;
        Node = KeNodeBlock[0];
        PfnDeferredList = &Node->PfnDeferredList;
        PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
    }
#else
    PfnDeferredList = &MmPfnDeferredList;
    PfnDereferenceSListHead = &MmPfnDereferenceSListHead;
#endif

    do {

        if (*PfnDeferredList == NULL) {

#if !defined(MI_MULTINODE)
            if ((Flags & MI_DEFER_PFN_HELD) == 0) {
                UNLOCK_PFN2 (OldIrql);
            }
            return;
#else
            i += 1;
            if (i < TotalNodes) {
                Node = KeNodeBlock[i];
                PfnDeferredList = &Node->PfnDeferredList;
                PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
                continue;
            }
            break;
#endif
        }

        //
        // Process each deferred unlock entry until they're all done.
        //

        LastEntry = NULL;
        VeryLastEntry = NULL;

        do {

            SingleListEntry = *PfnDeferredList;

            FirstEntry = SingleListEntry;

            do {

                NextEntry = SingleListEntry->Next;

                //
                // Process the deferred entry.
                //

                DerefMdl = CONTAINING_RECORD (SingleListEntry,
                                              MI_PFN_DEREFERENCE_CHUNK,
                                              ListEntry);

                MdlFlags = DerefMdl->Flags;
                NumberOfPages = (PFN_NUMBER) DerefMdl->NumberOfPages;
                ASSERT (NumberOfPages <= MI_MAX_DEREFERENCE_CHUNK);
                Page = &DerefMdl->Pfns[0];
                LastPage = Page + NumberOfPages;

#if DBG
                //
                // Mark the entry as processed so if it mistakenly gets
                // reprocessed, we will assert above.
                //

                DerefMdl->NumberOfPages |= 0x80;
#endif
                if (MdlFlags & MDL_WRITE_OPERATION) {

                    do {

                        //
                        // If this was a write operation set the modified bit
                        // in the PFN database.
                        //

                        Pfn1 = (PMMPFN) (*Page);

                        MI_SET_MODIFIED (Pfn1, 1, 0x4);

                        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                            (Pfn1->u3.e1.WriteInProgress == 0)) {

                            FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                                MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                            }
                        }

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                        Page += 1;
                    } while (Page < LastPage);
                }
                else {

                    do {

                        Pfn1 = (PMMPFN) (*Page);

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                        Page += 1;
                    } while (Page < LastPage);
                }

                ListCount += 1;

                //
                // March on to the next entry if there is one.
                //

                if (NextEntry == LastEntry) {
                    break;
                }

                SingleListEntry = NextEntry;

            } while (TRUE);

            if (VeryLastEntry == NULL) {
                VeryLastEntry = SingleListEntry;
            }

            if (InterlockedCompareExchangePointer (PfnDeferredList,
                                                   NULL,
                                                   FirstEntry) == FirstEntry) {
                ASSERT (*PfnDeferredList != FirstEntry);
                break;
            }
            ASSERT (*PfnDeferredList != FirstEntry);
            LastEntry = FirstEntry;

        } while (TRUE);

        //
        // Push the processed list chain on the freelist.
        //

        ASSERT (ListCount != 0);
        ASSERT (FirstEntry != NULL);
        ASSERT (VeryLastEntry != NULL);

#if defined(MI_MULTINODE)
        InterlockedPushListSList (PfnDereferenceSListHead,
                                  FirstEntry,
                                  VeryLastEntry,
                                  ListCount);

        i += 1;
        if (i < TotalNodes) {
            Node = KeNodeBlock[i];
            PfnDeferredList = &Node->PfnDeferredList;
            PfnDereferenceSListHead = &Node->PfnDereferenceSListHead;
            ListCount = 0;
        }
        else {
            break;
        }
    } while (TRUE);
#else
    } while (FALSE);
#endif

    if ((Flags & MI_DEFER_PFN_HELD) == 0) {
        UNLOCK_PFN2 (OldIrql);
    }

#if !defined(MI_MULTINODE)

    //
    // If possible, push the processed chain after releasing the PFN lock.
    //

    InterlockedPushListSList (PfnDereferenceSListHead,
                              FirstEntry,
                              VeryLastEntry,
                              ListCount);
#endif
}

VOID
MmBuildMdlForNonPagedPool (
    __inout PMDL MemoryDescriptorList
    )

/*++

Routine Description:

    This routine fills in the "pages" portion of the MDL using the PFN
    numbers corresponding to the buffers which reside in non-paged pool.

    Unlike MmProbeAndLockPages, there is no corresponding unlock as no
    reference counts are incremented as the buffers being in nonpaged
    pool are always resident.

Arguments:

    MemoryDescriptorList - Supplies a pointer to a Memory Descriptor List
                            (MDL). The supplied MDL must supply a virtual
                            address, byte offset and length field.  The
                            physical page portion of the MDL is updated when
                            the pages are locked in memory.  The virtual
                            address must be within the non-paged portion
                            of the system space.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PPFN_NUMBER Page;
    PPFN_NUMBER EndPage;
    PMMPTE PointerPte;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & (
                    MDL_PAGES_LOCKED |
                    MDL_MAPPED_TO_SYSTEM_VA |
                    MDL_SOURCE_IS_NONPAGED_POOL |
                    MDL_PARTIAL)) == 0);

    MemoryDescriptorList->Process = NULL;

    //
    // Endva is last byte of the buffer.
    //

    VirtualAddress = MemoryDescriptorList->StartVa;

    MemoryDescriptorList->MappedSystemVa =
            (PVOID)((PCHAR)VirtualAddress + MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (MemoryDescriptorList->MappedSystemVa,
                                           MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    EndPage = Page + NumberOfPages;

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) {

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);

        do {
#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
            *Page = PageFrameIndex;
            Page += 1;
            PageFrameIndex += 1;
        } while (Page < EndPage);

        PageFrameIndex -= 1;
    }
    else {

        PointerPte = MiGetPteAddress (VirtualAddress);

        do {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            *Page = PageFrameIndex;
            Page += 1;
            PointerPte += 1;
        } while (Page < EndPage);
    }

    ASSERT ((MmIsNonPagedSystemAddressValid (MemoryDescriptorList->StartVa)) ||
            (MmAreMdlPagesLocked (MemoryDescriptorList)));

    MemoryDescriptorList->MdlFlags |= MDL_SOURCE_IS_NONPAGED_POOL;

    //
    // Assume either all the frames are in the PFN database (ie: the MDL maps
    // pool) or none of them (the MDL maps dualport RAM) are.
    //

    if (!MI_IS_PFN (PageFrameIndex)) {
        MemoryDescriptorList->MdlFlags |= MDL_IO_SPACE;
    }

    return;
}

VOID
MiInitializeIoTrackers (
    VOID
    )
{
    InitializeSListHead (&MiDeadPteTrackerSListHead);
    KeInitializeSpinLock (&MiPteTrackerLock);
    InitializeListHead (&MiPteHeader.ListHead);

    KeInitializeSpinLock (&MmIoTrackerLock);
    InitializeListHead (&MmIoHeader);

    KeInitializeSpinLock (&MiTrackIoLock);
}

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    )

/*++

Routine Description:

    This function inserts a PTE tracking block as the caller has just
    consumed system PTEs.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List.

    Flags - Supplies the following values:

            0 - Indicates all the fields of the MDL are legitimate and can
                be snapped.

            1 - Indicates the caller is mapping physically contiguous memory.
                The only valid MDL fields are Page[0] & ByteCount.
                Page[0] contains the PFN start, ByteCount the byte count.

            2 - Indicates the caller is just reserving mapping PTEs.
                The only valid MDL fields are Page[0] & ByteCount.
                Page[0] contains the pool tag, ByteCount the byte count.

    MyCaller - Supplies the return address of the caller who consumed the
               system PTEs to map this MDL.

    MyCallersCaller - Supplies the return address of the caller of the caller
                      who consumed the system PTEs to map this MDL.

Return Value:

    None.

Environment:

    Kernel mode, DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    PVOID StartingVa;
    PPTE_TRACKER Tracker;
    PSLIST_ENTRY SingleListEntry;
    PSLIST_ENTRY NextSingleListEntry;
    PFN_NUMBER NumberOfPtes;

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    if (ExQueryDepthSList (&MiDeadPteTrackerSListHead) < 10) {
        Tracker = (PPTE_TRACKER) InterlockedPopEntrySList (&MiDeadPteTrackerSListHead);
    }
    else {
        SingleListEntry = ExInterlockedFlushSList (&MiDeadPteTrackerSListHead);

        Tracker = (PPTE_TRACKER) SingleListEntry;

        if (SingleListEntry != NULL) {

            SingleListEntry = SingleListEntry->Next;

            while (SingleListEntry != NULL) {

                NextSingleListEntry = SingleListEntry->Next;

                ExFreePool (SingleListEntry);

                SingleListEntry = NextSingleListEntry;
            }
        }
    }

    if (Tracker == NULL) {

        Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof (PTE_TRACKER),
                                         'ySmM');

        if (Tracker == NULL) {
            MiTrackPtesAborted = TRUE;
            return;
        }
    }

    switch (Flags) {

        case 0:

            //
            // Regular MDL mapping.
            //

            StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                            MemoryDescriptorList->ByteOffset);

            NumberOfPtes = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                                   MemoryDescriptorList->ByteCount);
            Tracker->Mdl = MemoryDescriptorList;

            Tracker->StartVa = MemoryDescriptorList->StartVa;
            Tracker->Offset = MemoryDescriptorList->ByteOffset;
            Tracker->Length = MemoryDescriptorList->ByteCount;

            break;

        case 1:

            //
            // MmMapIoSpace call (ie: physically contiguous mapping).
            //

            StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                            MemoryDescriptorList->ByteOffset);

            NumberOfPtes = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                                   MemoryDescriptorList->ByteCount);
            Tracker->Mdl = (PVOID)1;
            break;

        default:
            ASSERT (FALSE);
            // Fall through

        case 2:

            //
            // MmAllocateReservedMapping call (ie: currently maps nothing).
            //

            NumberOfPtes = (MemoryDescriptorList->ByteCount >> PAGE_SHIFT);
            Tracker->Mdl = NULL;
            break;
    }

    Tracker->Count = NumberOfPtes;

    Tracker->CallingAddress = MyCaller;
    Tracker->CallersCaller = MyCallersCaller;

    Tracker->SystemVa = MemoryDescriptorList->MappedSystemVa;
    Tracker->Page = *(PPFN_NUMBER)(MemoryDescriptorList + 1);

    Tracker->CacheAttribute = CacheAttribute;
    Tracker->IoMapping = IoMapping;
    Tracker->Matched = 0;

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    InsertHeadList (&MiPteHeader.ListHead, &Tracker->ListEntry);

    MiPteHeader.Count += NumberOfPtes;
    MiPteHeader.NumberOfEntries += 1;

    if (MiPteHeader.NumberOfEntries > MiPteHeader.NumberOfEntriesPeak) {
        MiPteHeader.NumberOfEntriesPeak = MiPteHeader.NumberOfEntries;
    }

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    )

/*++

Routine Description:

    This function removes a PTE tracking block from the lists as the PTEs
    are being freed.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List.

    PteAddress - Supplies the address the system PTEs were mapped to.

    NumberOfPtes - Supplies the number of system PTEs allocated.

Return Value:

    None.

Environment:

    Kernel mode, DISPATCH_LEVEL or below. Locks (including the PFN) may be held.

--*/

{
    KIRQL OldIrql;
    PPTE_TRACKER Tracker;
    PLIST_ENTRY LastFound;
    PLIST_ENTRY NextEntry;

    LastFound = NULL;

    VirtualAddress = PAGE_ALIGN (VirtualAddress);

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    NextEntry = MiPteHeader.ListHead.Flink;
    while (NextEntry != &MiPteHeader.ListHead) {

        Tracker = (PPTE_TRACKER) CONTAINING_RECORD (NextEntry,
                                                    PTE_TRACKER,
                                                    ListEntry.Flink);

        if (VirtualAddress == PAGE_ALIGN (Tracker->SystemVa)) {

            if (LastFound != NULL) {

                //
                // Duplicate map entry.
                //

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x1,
                              (ULONG_PTR)Tracker,
                              (ULONG_PTR)MemoryDescriptorList,
                              (ULONG_PTR)LastFound);
            }

            if (Tracker->Count != NumberOfPtes) {

                //
                // Not unmapping the same of number of PTEs that were mapped.
                //

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x2,
                              (ULONG_PTR)Tracker,
                              Tracker->Count,
                              NumberOfPtes);
            }

            if ((ARGUMENT_PRESENT (MemoryDescriptorList)) &&
                ((MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) == 0) &&
                (MiMdlsAdjusted == FALSE)) {

                if (Tracker->SystemVa != MemoryDescriptorList->MappedSystemVa) {

                    //
                    // Not unmapping the same address that was mapped.
                    //

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x3,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->SystemVa,
                                  (ULONG_PTR)MemoryDescriptorList->MappedSystemVa);
                }

                if (Tracker->Page != *(PPFN_NUMBER)(MemoryDescriptorList + 1)) {

                    //
                    // The first page in the MDL has changed since it was mapped.
                    //

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x4,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->Page,
                                  (ULONG_PTR) *(PPFN_NUMBER)(MemoryDescriptorList + 1));
                }

                if (Tracker->StartVa != MemoryDescriptorList->StartVa) {

                    //
                    // Map and unmap don't match up.
                    //

                    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                                  0x5,
                                  (ULONG_PTR)Tracker,
                                  (ULONG_PTR)Tracker->StartVa,
                                  (ULONG_PTR)MemoryDescriptorList->StartVa);
                }
            }

            RemoveEntryList (NextEntry);
            LastFound = NextEntry;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

    if ((LastFound == NULL) && (MiTrackPtesAborted == FALSE)) {

        //
        // Can't unmap something that was never (or isn't currently) mapped.
        //

        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x6,
                      (ULONG_PTR)MemoryDescriptorList,
                      (ULONG_PTR)VirtualAddress,
                      (ULONG_PTR)NumberOfPtes);
    }

    MiPteHeader.Count -= NumberOfPtes;
    MiPteHeader.NumberOfEntries -= 1;

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);

    //
    // Insert the tracking block into the dead PTE list for later
    // release.  Locks (including the PFN lock) may be held on entry, thus the
    // block cannot be directly freed to pool at this time.
    //

    if (LastFound != NULL) {
        InterlockedPushEntrySList (&MiDeadPteTrackerSListHead,
                                   (PSLIST_ENTRY)LastFound);
    }

    return;
}

PVOID
MiGetHighestPteConsumer (
    OUT PULONG_PTR NumberOfPtes
    )

/*++

Routine Description:

    This function examines the PTE tracking blocks and returns the biggest
    consumer.

Arguments:

    None.

Return Value:

    The loaded module entry of the biggest consumer.

Environment:

    Kernel mode, called during bugcheck only.  Many locks may be held.

--*/

{
    PPTE_TRACKER Tracker;
    PVOID BaseAddress;
    PFN_NUMBER NumberOfPages;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY NextEntry2;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG_PTR Highest;
    ULONG_PTR PagesByThisModule;
    PKLDR_DATA_TABLE_ENTRY HighDataTableEntry;

    *NumberOfPtes = 0;

    //
    // No locks are acquired as this is only called during a bugcheck.
    //

    if ((MmTrackPtes & 0x1) == 0) {
        return NULL;
    }

    if (MiTrackPtesAborted == TRUE) {
        return NULL;
    }

    if (IsListEmpty (&MiPteHeader.ListHead)) {
        return NULL;
    }

    if (PsLoadedModuleList.Flink == NULL) {
        return NULL;
    }

    Highest = 0;
    HighDataTableEntry = NULL;

    //
    // Walk the loaded module list backwards so we do the kernel last as the
    // bit bucket for everyone else.
    //

    NextEntry = PsLoadedModuleList.Blink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        PagesByThisModule = 0;

        //
        // Walk the PTE mapping list and update each driver's counts.
        //
    
        NextEntry2 = MiPteHeader.ListHead.Flink;
        while (NextEntry2 != &MiPteHeader.ListHead) {
    
            Tracker = (PPTE_TRACKER) CONTAINING_RECORD (NextEntry2,
                                                        PTE_TRACKER,
                                                        ListEntry.Flink);
    
            BaseAddress = Tracker->CallingAddress;
            NumberOfPages = Tracker->Count;
    
            if (Tracker->Matched == 1) {

                //
                // We already processed this one in an earlier loop.
                //

                NOTHING;
            }
            else if ((BaseAddress >= DataTableEntry->DllBase) &&
                (BaseAddress < (PVOID)((ULONG_PTR)(DataTableEntry->DllBase) + DataTableEntry->SizeOfImage))) {

                PagesByThisModule += NumberOfPages;
                Tracker->Matched = 1;
            }
            else {

                //
                // See if the caller's caller can be charged instead.  This way
                // drivers that call MmMapLockedPages (which in turn calls
                // MmMapLockedPagesSpecifyCache where the charging is done)
                // don't get their allocations incorrectly logged against the
                // kernel.
                //

                BaseAddress = Tracker->CallersCaller;

                if ((BaseAddress >= DataTableEntry->DllBase) &&
                    (BaseAddress < (PVOID)((ULONG_PTR)(DataTableEntry->DllBase) + DataTableEntry->SizeOfImage))) {
    
                    PagesByThisModule += NumberOfPages;
                    Tracker->Matched = 1;
                }
            }

            NextEntry2 = NextEntry2->Flink;
        }
    
        if (PagesByThisModule > Highest) {
            Highest = PagesByThisModule;
            HighDataTableEntry = DataTableEntry;
        }

        NextEntry = NextEntry->Blink;
    }

    *NumberOfPtes = Highest;

    return (PVOID)HighDataTableEntry;
}

MI_PFN_CACHE_ATTRIBUTE
MiInsertIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )

/*++

Routine Description:

    This function inserts an I/O space tracking block, returning the cache
    type the caller should use.  The cache type is different from the input
    cache type if an overlap collision is detected.

Arguments:

    BaseVa - Supplies the virtual address that will be used for the mapping.

    PageFrameIndex - Supplies the starting physical page number that will be
                     mapped.

    NumberOfPages - Supplies the number of pages to map.

    CacheAttribute - Supplies the caller's desired cache attribute.

Return Value:

    The cache attribute that is safe to use.

Environment:

    Kernel mode, DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    PMMIO_TRACKER Tracker;
    PMMIO_TRACKER Tracker2;
    PLIST_ENTRY NextEntry;

#if DBG

    ULONG Hash;

#endif

    ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

    Tracker = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof (MMIO_TRACKER),
                                     'ySmM');

    if (Tracker == NULL) {
        return MiNotMapped;
    }

    Tracker->BaseVa = BaseVa;
    Tracker->PageFrameIndex = PageFrameIndex;
    Tracker->NumberOfPages = NumberOfPages;
    Tracker->CacheAttribute = CacheAttribute;

    RtlZeroMemory (&Tracker->StackTrace[0], MI_IO_BACKTRACE_LENGTH * sizeof(PVOID)); 

#if DBG

    RtlCaptureStackBackTrace (2, MI_IO_BACKTRACE_LENGTH, Tracker->StackTrace, &Hash);

#endif

    ASSERT (!MI_IS_PFN (PageFrameIndex));

    ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

    //
    // Scan I/O space mappings for duplicate or overlapping entries.
    //

    NextEntry = MmIoHeader.Flink;
    while (NextEntry != &MmIoHeader) {

        Tracker2 = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                      MMIO_TRACKER,
                                                      ListEntry.Flink);

        if ((Tracker->PageFrameIndex < Tracker2->PageFrameIndex + Tracker2->NumberOfPages) &&
            (Tracker->PageFrameIndex + Tracker->NumberOfPages > Tracker2->PageFrameIndex)) {

#if DBG
            if ((MmShowMapOverlaps & 0x1) ||

                ((Tracker->CacheAttribute != Tracker2->CacheAttribute) &&
                (MmShowMapOverlaps & 0x2))) {

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                "MM: Iospace mapping overlap %p %p\n",
                                Tracker,
                                Tracker2);

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                "Physical range 0x%p->%p first mapped %s at VA %p\n",
                                Tracker2->PageFrameIndex << PAGE_SHIFT,
                                (Tracker2->PageFrameIndex + Tracker2->NumberOfPages) << PAGE_SHIFT,
                                MiCacheStrings[Tracker2->CacheAttribute],
                                Tracker2->BaseVa);

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                "\tCall stack: %p %p %p %p %p %p\n",
                                Tracker2->StackTrace[0],
                                Tracker2->StackTrace[1],
                                Tracker2->StackTrace[2],
                                Tracker2->StackTrace[3],
                                Tracker2->StackTrace[4],
                                Tracker2->StackTrace[5]);

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                "Physical range 0x%p->%p now being mapped %s at VA %p\n",
                                Tracker->PageFrameIndex << PAGE_SHIFT,
                                (Tracker->PageFrameIndex + Tracker->NumberOfPages) << PAGE_SHIFT,
                                MiCacheStrings[Tracker->CacheAttribute],
                                Tracker->BaseVa);

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                "\tCall stack: %p %p %p %p %p %p\n",
                                Tracker->StackTrace[0],
                                Tracker->StackTrace[1],
                                Tracker->StackTrace[2],
                                Tracker->StackTrace[3],
                                Tracker->StackTrace[4],
                                Tracker->StackTrace[5]);

                if (MmShowMapOverlaps & 0x80000000) {
                    DbgBreakPoint ();
                }
            }
#endif

            if (Tracker->CacheAttribute != Tracker2->CacheAttribute) {
                MiCacheOverride[3] += 1;

                Tracker->CacheAttribute = Tracker2->CacheAttribute;
            }

            //
            // Don't bother checking for overlapping multiple entries.
            // This would be a very strange driver bug and is already
            // caught by the verifier anyway.
            //
        }

        NextEntry = Tracker2->ListEntry.Flink;
    }

    InsertHeadList (&MmIoHeader, &Tracker->ListEntry);

#if DBG
    MmIoHeaderCount += NumberOfPages;
    MmIoHeaderNumberOfEntries += 1;

    if (MmIoHeaderNumberOfEntries > MmIoHeaderNumberOfEntriesPeak) {
        MmIoHeaderNumberOfEntriesPeak = MmIoHeaderNumberOfEntries;
    }
#endif

    ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);

    return Tracker->CacheAttribute;
}

VOID
MiRemoveIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER NumberOfPages
    )

/*++

Routine Description:

    This function removes an I/O space tracking block from the lists.

Arguments:

    BaseVa - Supplies the virtual address that will be used for the unmapping.

    NumberOfPages - Supplies the number of pages to unmap.

Return Value:

    None.

Environment:

    Kernel mode, DISPATCH_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    PMMIO_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    PVOID AlignedVa;

    AlignedVa = PAGE_ALIGN (BaseVa);

    ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

    NextEntry = MmIoHeader.Flink;
    while (NextEntry != &MmIoHeader) {

        Tracker = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                     MMIO_TRACKER,
                                                     ListEntry.Flink);

        if ((PAGE_ALIGN (Tracker->BaseVa) == AlignedVa) &&
            (Tracker->NumberOfPages == NumberOfPages)) {

            RemoveEntryList (NextEntry);

#if DBG
            MmIoHeaderCount -= NumberOfPages;
            MmIoHeaderNumberOfEntries -= 1;
#endif

            ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);

            ExFreePool (Tracker);

            return;
        }
        NextEntry = Tracker->ListEntry.Flink;
    }

    //
    // Can't unmap something that was never (or isn't currently) mapped.
    //

    KeBugCheckEx (SYSTEM_PTE_MISUSE,
                  0x400,
                  (ULONG_PTR)BaseVa,
                  (ULONG_PTR)NumberOfPages,
                  0);
}

PVOID
MiMapSinglePage (
     IN PVOID VirtualAddress OPTIONAL,
     IN PFN_NUMBER PageFrameIndex,
     IN MEMORY_CACHING_TYPE CacheType,
     IN MM_PAGE_PRIORITY Priority
     )

/*++

Routine Description:

    This function (re)maps a single system PTE to the specified physical page.

Arguments:

    VirtualAddress - Supplies the virtual address to map the page frame at.
                     NULL indicates a system PTE is needed.  Non-NULL supplies
                     the virtual address returned by an earlier
                     MiMapSinglePage call.

    PageFrameIndex - Supplies the page frame index to map.

    CacheType - Supplies the type of cache mapping to use for the MDL.
                MmCached indicates "normal" kernel or user mappings.

    Priority - Supplies an indication as to how important it is that this
               request succeed under low available PTE conditions.

Return Value:

    Returns the base address where the page is mapped, or NULL if the
    mapping failed.

Environment:

    Kernel mode.  APC_LEVEL or below.  Since the process working set pushlock
    may be held by callers, this routine is nonpaged.

--*/

{
    KIRQL OldIrql;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PMMPFN Pfn1;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (Priority);

    ASSERT (MI_IS_PFN (PageFrameIndex));

    if (VirtualAddress == NULL) {

        PointerPte = MiReserveSystemPtes (1, SystemPteSpace);

        if (PointerPte == NULL) {
    
            //
            // Not enough system PTES are available.
            //
    
            return NULL;
        }

        ASSERT (PointerPte->u.Hard.Valid == 0);
        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
    }
    else {
        ASSERT (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == 0);
        ASSERT (VirtualAddress >= MM_SYSTEM_RANGE_START);

        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);

        MI_WRITE_ZERO_PTE (PointerPte);

        MI_FLUSH_SINGLE_TB (VirtualAddress, TRUE);
    }

    TempPte = ValidKernelPte;

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
        CacheAttribute = MiCached;
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    switch (Pfn1->u3.e1.CacheAttribute) {

        case MiCached:
            if (CacheAttribute != MiCached) {

                //
                // The caller asked for a noncached or writecombined
                // mapping, but the page is already mapped cached by
                // someone else.  Override the caller's request in
                // order to keep the TB page attribute coherent.
                //

                MiCacheOverride[0] += 1;
                CacheAttribute = MiCached;
            }
            break;

        case MiNonCached:
            if (CacheAttribute != MiNonCached) {

                //
                // The caller asked for a cached or writecombined
                // mapping, but the page is already mapped noncached
                // by someone else.  Override the caller's request
                // in order to keep the TB page attribute coherent.
                //

                MiCacheOverride[1] += 1;
                CacheAttribute = MiNonCached;
            }
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiWriteCombined:
            if (CacheAttribute != MiWriteCombined) {

                //
                // The caller asked for a cached or noncached
                // mapping, but the page is already mapped
                // writecombined by someone else.  Override the
                // caller's request in order to keep the TB page
                // attribute coherent.
                //

                MiCacheOverride[2] += 1;
                CacheAttribute = MiWriteCombined;
            }
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        case MiNotMapped:

            //
            // This better be for a page allocated with
            // MmAllocatePagesForMdl.  Otherwise it might be a
            // page on the freelist which could subsequently be
            // given out with a different attribute !
            //

            ASSERT ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                    (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));

            LOCK_PFN2 (OldIrql);

            switch (CacheAttribute) {

                case MiCached:
                    Pfn1->u3.e1.CacheAttribute = MiCached;
                    break;

                case MiNonCached:
                    Pfn1->u3.e1.CacheAttribute = MiNonCached;
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    Pfn1->u3.e1.CacheAttribute = MiWriteCombined;
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }

            UNLOCK_PFN2 (OldIrql);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    switch (CacheAttribute) {

        case MiCached:
            break;

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    return VirtualAddress;
}

PVOID
MmMapLockedPages (
    __in PMDL MemoryDescriptorList,
    __in KPROCESSOR_MODE AccessMode
    )

/*++

Routine Description:

    This function maps physical pages described by a memory descriptor
    list into the system virtual address space or the user portion of
    the virtual address space.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                            been updated by MmProbeAndLockPages.


    AccessMode - Supplies an indicator of where to map the pages;
                 KernelMode indicates that the pages should be mapped in the
                 system part of the address space, UserMode indicates the
                 pages should be mapped in the user part of the address space.

Return Value:

    Returns the base address where the pages are mapped.  The base address
    has the same offset as the virtual address in the MDL.

    This routine will raise an exception if the processor mode is USER_MODE
    and quota limits or VM limits are exceeded.

Environment:

    Kernel mode.  DISPATCH_LEVEL or below if access mode is KernelMode,
                  APC_LEVEL or below if access mode is UserMode.

--*/

{
    return MmMapLockedPagesSpecifyCache (MemoryDescriptorList,
                                         AccessMode,
                                         MmCached,
                                         NULL,
                                         TRUE,
                                         HighPagePriority);
}

VOID
MiUnmapSinglePage (
     IN PVOID VirtualAddress
     )

/*++

Routine Description:

    This routine unmaps a single locked page which was previously mapped via
    an MiMapSinglePage call.

Arguments:

    VirtualAddress - Supplies the virtual address used to map the page.

Return Value:

    None.

Environment:

    Kernel mode.  APC_LEVEL or below, base address is within system space.
    Since the process working set pushlock may be held by callers, this
    routine is nonpaged.

--*/

{
    PMMPTE PointerPte;

    PAGED_CODE ();

    ASSERT (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == 0);
    ASSERT (VirtualAddress >= MM_SYSTEM_RANGE_START);

    PointerPte = MiGetPteAddress (VirtualAddress);

    MiReleaseSystemPtes (PointerPte, 1, SystemPteSpace);
    return;
}

__bcount(NumberOfBytes) PVOID
MmAllocateMappingAddress (
     __in SIZE_T NumberOfBytes,
     __in ULONG PoolTag
     )

/*++

Routine Description:

    This function allocates a system PTE mapping of the requested length
    that can be used later to map arbitrary addresses.

Arguments:

    NumberOfBytes - Supplies the maximum number of bytes the mapping can span.

    PoolTag - Supplies a pool tag to associate this mapping to the caller.

Return Value:

    Returns a virtual address where to use for later mappings.

Environment:

    Kernel mode.  PASSIVE_LEVEL.

--*/

{
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PVOID BaseVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PMDL MemoryDescriptorList;
    PFN_NUMBER NumberOfPages;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    //
    // Make sure there are enough PTEs of the requested size.
    // Try to ensure available PTEs inline when we're rich.
    // Otherwise go the long way.
    //

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (0, NumberOfBytes);

    if (NumberOfPages == 0) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x100,
                      NumberOfPages,
                      PoolTag,
                      (ULONG_PTR) CallingAddress);
    }

    //
    // Callers must identify themselves.
    //

    if (PoolTag == 0) {
        return NULL;
    }

    //
    // Leave space to stash the length and tag.
    //

    NumberOfPages += 2;

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {

        //
        // Not enough system PTES are available.
        //

        return NULL;
    }

    //
    // Make sure the valid bit is always zero in the stash PTEs.
    //

    *(PULONG_PTR)PointerPte = (NumberOfPages << 1);
    PointerPte += 1;

    *(PULONG_PTR)PointerPte = (PoolTag & ~0x1);
    PointerPte += 1;

    //
    // Zero out the PTEs as they may have pointers and timestamps in them,
    // and we want to be able to assert they are all NULL on release.
    //

    MiZeroMemoryPte (PointerPte, NumberOfPages - 2);

    BaseVa = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MmTrackPtes & 0x1) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        MemoryDescriptorList = (PMDL) MdlHack;

        MemoryDescriptorList->MappedSystemVa = BaseVa;
        MemoryDescriptorList->StartVa = (PVOID)(ULONG_PTR)PoolTag;
        MemoryDescriptorList->ByteOffset = 0;
        MemoryDescriptorList->ByteCount = (ULONG)((NumberOfPages - 2) * PAGE_SIZE);

        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
        *Page = 0;

        MiInsertPteTracker (MemoryDescriptorList,
                            2,
                            FALSE,
                            MiCached,
                            CallingAddress,
                            CallersCaller);
    }

    return BaseVa;
}

VOID
MmFreeMappingAddress (
     __in PVOID BaseAddress,
     __in ULONG PoolTag
     )

/*++

Routine Description:

    This routine unmaps a virtual address range previously reserved with
    MmAllocateMappingAddress.

Arguments:

    BaseAddress - Supplies the base address previously reserved.

    PoolTag - Supplies the caller's identifying tag.

Return Value:

    None.

Environment:

    Kernel mode.  PASSIVE_LEVEL.

--*/

{
    ULONG OriginalPoolTag;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerBase;
    PMMPTE PointerPte;
    PMMPTE LastPte;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));
    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x101,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    NumberOfPages = *(PULONG_PTR)PointerBase;
    ASSERT ((NumberOfPages & 0x1) == 0);
    NumberOfPages >>= 1;

    if (NumberOfPages <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x102,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      NumberOfPages);
    }

    NumberOfPages -= 2;
    LastPte = PointerPte + NumberOfPages;

    while (PointerPte < LastPte) {
        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x103,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }
        PointerPte += 1;
    }

    if (MmTrackPtes & 0x1) {
        MiRemovePteTracker (NULL, BaseAddress, NumberOfPages);
    }

    //
    // Note the tag and size are nulled out when the PTEs are released below
    // so any drivers that try to use their mapping after freeing it get
    // caught immediately.
    //

    MiReleaseSystemPtes (PointerBase, (ULONG)NumberOfPages + 2, SystemPteSpace);
    return;
}

PVOID
MmMapLockedPagesWithReservedMapping (
    __in PVOID MappingAddress,
    __in ULONG PoolTag,
    __in PMDL MemoryDescriptorList,
    __in MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function maps physical pages described by a memory descriptor
    list into the system virtual address space.

Arguments:

    MappingAddress - Supplies a valid mapping address obtained earlier via
                     MmAllocateMappingAddress.

    PoolTag - Supplies the caller's identifying tag.

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

    CacheType - Supplies the type of cache mapping to use for the MDL.
                MmCached indicates "normal" kernel or user mappings.

Return Value:

    Returns the base address where the pages are mapped.  The base address
    has the same offset as the virtual address in the MDL.

    This routine will return NULL if the cache type requested is incompatible
    with the pages being mapped or if the caller tries to map an MDL that is
    larger than the virtual address range originally reserved.

Environment:

    Kernel mode.  DISPATCH_LEVEL or below.  The caller must synchronize usage
    of the argument virtual address space.

--*/

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER VaPageSpan;
    PFN_NUMBER SavedPageCount;
    PPFN_NUMBER Page;
    PMMPTE PointerBase;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    PVOID StartingVa;
    PFN_NUMBER NumberOfPtes;
    PFN_NUMBER PageFrameIndex;
    ULONG OriginalPoolTag;
    PMMPFN Pfn2;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    PointerPte = MiGetPteAddress (MappingAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x104,
                      (ULONG_PTR)MappingAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    VaPageSpan = *(PULONG_PTR)PointerBase;
    ASSERT ((VaPageSpan & 0x1) == 0);
    VaPageSpan >>= 1;

    if (VaPageSpan <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x105,
                      (ULONG_PTR)MappingAddress,
                      PoolTag,
                      VaPageSpan);
    }

    if (NumberOfPages > VaPageSpan - 2) {

        //
        // The caller is trying to map an MDL that spans a range larger than
        // the reserving mapping !  This is a driver bug.
        //

        ASSERT (FALSE);
        return NULL;
    }

    //
    // All the mapping PTEs must be zero.
    //

    LastPte = PointerPte + VaPageSpan - 2;

    while (PointerPte < LastPte) {

        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x107,
                          (ULONG_PTR)MappingAddress,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)LastPte);
        }

        PointerPte += 1;
    }

    PointerPte = PointerBase + 2;
    SavedPageCount = NumberOfPages;

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_MAPPED_TO_SYSTEM_VA |
                        MDL_SOURCE_IS_NONPAGED_POOL |
                        MDL_PARTIAL_HAS_BEEN_MAPPED)) == 0);

    ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_PAGES_LOCKED |
                        MDL_PARTIAL)) != 0);

    //
    // If a noncachable mapping is requested, none of the pages in the
    // requested MDL can reside in a large page.  Otherwise we would be
    // creating an incoherent overlapping TB entry as the same physical
    // page would be mapped by 2 different TB entries with different
    // cache attributes.
    //

    IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (CacheAttribute != MiCached) {

        LOCK_PFN2 (OldIrql);

        do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
            if (*Page == MM_EMPTY_LIST) {
                break;
            }

            PageFrameIndex = *Page;

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN2 (OldIrql);
                MiNonCachedCollisions += 1;
                return NULL;
            }

            Page += 1;
            NumberOfPages -= 1;
        } while (NumberOfPages != 0);

        UNLOCK_PFN2 (OldIrql);

        NumberOfPages = SavedPageCount;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        MI_PREPARE_FOR_NONCACHED (CacheAttribute);
    }

    NumberOfPtes = NumberOfPages;

    TempPte = ValidKernelPte;

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    OldIrql = HIGH_LEVEL;

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        ASSERT (PointerPte->u.Hard.Valid == 0);

        if ((IoMapping == 0) || (MI_IS_PFN (*Page))) {

            Pfn2 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);
            TempPte = ValidKernelPte;

            MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

            switch (Pfn2->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {

                        //
                        // The caller asked for a noncached or writecombined
                        // mapping, but the page is already mapped cached by
                        // someone else.  Override the caller's request in
                        // order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                        //
                        // The caller asked for a cached or writecombined
                        // mapping, but the page is already mapped noncached
                        // by someone else.  Override the caller's request
                        // in order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                        //
                        // The caller asked for a cached or noncached
                        // mapping, but the page is already mapped
                        // writecombined by someone else.  Override the
                        // caller's request in order to keep the TB page
                        // attribute coherent.
                        //

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                    //
                    // This better be for a page allocated with
                    // MmAllocatePagesForMdl.  Otherwise it might be a
                    // page on the freelist which could subsequently be
                    // given out with a different attribute !
                    //

                    ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
                    if (OldIrql == HIGH_LEVEL) {
                        LOCK_PFN2 (OldIrql);
                    }

                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn2->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }

        TempPte.u.Hard.PageFrameNumber = *Page;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    if (OldIrql != HIGH_LEVEL) {
        UNLOCK_PFN2 (OldIrql);
    }

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
    MemoryDescriptorList->MappedSystemVa = MappingAddress;

    MemoryDescriptorList->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

    if ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) != 0) {
        MemoryDescriptorList->MdlFlags |= MDL_PARTIAL_HAS_BEEN_MAPPED;
    }

    MappingAddress = (PVOID)((PCHAR)MappingAddress + MemoryDescriptorList->ByteOffset);

    return MappingAddress;
}

VOID
MmUnmapReservedMapping (
     IN PVOID BaseAddress,
     IN ULONG PoolTag,
     IN PMDL MemoryDescriptorList
     )

/*++

Routine Description:

    This routine unmaps locked pages which were previously mapped via
    a MmMapLockedPagesWithReservedMapping call.

Arguments:

    BaseAddress - Supplies the base address where the pages were previously
                  mapped.

    PoolTag - Supplies the caller's identifying tag.

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

Return Value:

    None.

Environment:

    Kernel mode.  DISPATCH_LEVEL or below.  The caller must synchronize usage
    of the argument virtual address space.

--*/

{
    ULONG OriginalPoolTag;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER ExtraPages;
    PFN_NUMBER VaPageSpan;
    PMMPTE PointerBase;
    PMMPTE LastPte;
    PMMPTE LastMdlPte;
    PVOID StartingVa;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    PMMPTE PointerPte;
    PFN_NUMBER i;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastCurrentPage;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) != 0);

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARENT_MAPPED_SYSTEM_VA) == 0);
    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));
    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerBase = PointerPte - 2;

    OriginalPoolTag = *(PULONG) (PointerPte - 1);
    ASSERT ((OriginalPoolTag & 0x1) == 0);

    if (OriginalPoolTag != (PoolTag & ~0x1)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x108,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      OriginalPoolTag);
    }

    VaPageSpan = *(PULONG_PTR)PointerBase;
    ASSERT ((VaPageSpan & 0x1) == 0);
    VaPageSpan >>= 1;

    if (VaPageSpan <= 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x109,
                      (ULONG_PTR)BaseAddress,
                      PoolTag,
                      VaPageSpan);
    }

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    if (NumberOfPages > VaPageSpan - 2) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x10A,
                      (ULONG_PTR)BaseAddress,
                      VaPageSpan,
                      NumberOfPages);
    }

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    LastCurrentPage = Page + NumberOfPages;

    if (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) {

        ExtraPages = *(Page + NumberOfPages);
        ASSERT (ExtraPages <= MiCurrentAdvancedPages);
        ASSERT (NumberOfPages + ExtraPages <= VaPageSpan - 2);
        NumberOfPages += ExtraPages;
#if DBG
        InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, 0 - ExtraPages);
        MiAdvancesFreed += ExtraPages;
#endif
    }

    LastMdlPte = PointerPte + NumberOfPages;
    LastPte = PointerPte + VaPageSpan - 2;

    //
    // The range described by the argument MDL must be mapped.
    //

    while (PointerPte < LastMdlPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x10B,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }

#if DBG
        ASSERT ((*Page == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte)) ||
                (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES));

        if ((MI_IS_PFN (*Page)) && (Page < LastCurrentPage)) {
            PMMPFN Pfn3;
            Pfn3 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn3->u3.e2.ReferenceCount != 0);
        }

        Page += 1;
#endif

        PointerPte += 1;
    }

    //
    // The range past the argument MDL must be unmapped.
    //

    while (PointerPte < LastPte) {
        if (PointerPte->u.Long != 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x10C,
                          (ULONG_PTR)BaseAddress,
                          PoolTag,
                          NumberOfPages);
        }
        PointerPte += 1;
    }

    MiZeroMemoryPte (PointerBase + 2, NumberOfPages);

    if (NumberOfPages == 1) {
        MI_FLUSH_SINGLE_TB (BaseAddress, TRUE);
    }
    else if (NumberOfPages < MM_MAXIMUM_FLUSH_COUNT) {

        for (i = 0; i < NumberOfPages; i += 1) {
            VaFlushList[i] = BaseAddress;
            BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
        }

        MI_FLUSH_MULTIPLE_TB ((ULONG)NumberOfPages, &VaFlushList[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (2);
    }

    MemoryDescriptorList->MdlFlags &= ~(MDL_MAPPED_TO_SYSTEM_VA |
                                        MDL_PARTIAL_HAS_BEEN_MAPPED);

    return;
}

NTKERNELAPI
NTSTATUS
MmAdvanceMdl (
    __inout PMDL Mdl,
    __in ULONG NumberOfBytes
    )

/*++

Routine Description:

    This routine takes the specified MDL and "advances" it forward
    by the specified number of bytes.  If this causes the MDL to advance
    past the initial page, the pages that are advanced over are immediately
    unlocked and the system VA that maps the MDL is also adjusted (along
    with the user address).
    
    WARNING !  WARNING !  WARNING !

    This means the caller MUST BE AWARE that the "advanced" pages are
    immediately reused and therefore MUST NOT BE REFERENCED by the caller
    once this routine has been called.  Likewise the virtual address as
    that is also being adjusted here.

    Even if the caller has statically allocated this MDL on his local stack,
    he cannot use more than the space currently described by the MDL on return
    from this routine unless he first unmaps the MDL (if it was mapped).
    Otherwise the system PTE lists will be corrupted.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

    NumberOfBytes - The number of bytes to advance the MDL by.

Return Value:

    NTSTATUS.

--*/

{
    ULONG i;
    ULONG PageCount;
    LONG EntryCount;
    LONG OriginalCount;
    ULONG FreeBit;
    ULONG Slush;
    KIRQL OldIrql;
    PPFN_NUMBER Page;
    PPFN_NUMBER NewPage;
    ULONG OffsetPages;
    PEPROCESS Process;
    PMMPFN Pfn1;
    CSHORT MdlFlags;
    PVOID StartingVa;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER LastPage;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT (Mdl->MdlFlags & (MDL_PAGES_LOCKED | MDL_SOURCE_IS_NONPAGED_POOL));
    ASSERT (BYTE_OFFSET (Mdl->StartVa) == 0);

    //
    // Disallow advancement past the end of the MDL.
    //

    if (NumberOfBytes >= Mdl->ByteCount) {
        return STATUS_INVALID_PARAMETER_2;
    }

    PageCount = 0;

    MiMdlsAdjusted = TRUE;

    StartingVa = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingVa, Mdl->ByteCount);

    if (Mdl->ByteOffset != 0) {
        Slush = PAGE_SIZE - Mdl->ByteOffset;

        if (NumberOfBytes < Slush) {

            Mdl->ByteCount -= NumberOfBytes;
            Mdl->ByteOffset += NumberOfBytes;

            //
            // StartVa never includes the byte offset (it's always page-aligned)
            // so don't adjust it here.  MappedSystemVa does include byte
            // offsets so do adjust that.
            //

            if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa + NumberOfBytes);
            }

            return STATUS_SUCCESS;
        }

        NumberOfBytes -= Slush;

        Mdl->StartVa = (PVOID) ((PCHAR)Mdl->StartVa + PAGE_SIZE);
        Mdl->ByteOffset = 0;
        Mdl->ByteCount -= Slush;

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa + Slush);
        }

        //
        // Up the number of pages (and addresses) that need to slide.
        //

        PageCount += 1;
    }

    //
    // The MDL start is now nicely page aligned.  Make sure there's still
    // data left in it (we may have finished it off above), then operate on it.
    //

    if (NumberOfBytes != 0) {

        Mdl->ByteCount -= NumberOfBytes;

        Mdl->ByteOffset = BYTE_OFFSET (NumberOfBytes);

        OffsetPages = NumberOfBytes >> PAGE_SHIFT;

        Mdl->StartVa = (PVOID) ((PCHAR)Mdl->StartVa + (OffsetPages << PAGE_SHIFT));
        PageCount += OffsetPages;

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

            Mdl->MappedSystemVa = (PVOID) ((PCHAR)Mdl->MappedSystemVa +
                                           (OffsetPages << PAGE_SHIFT) +
                                           Mdl->ByteOffset);
        }
    }

    ASSERT (PageCount <= NumberOfPages);

    if (PageCount != 0) {

        //
        // Slide the page frame numbers forward decrementing reference counts
        // on the ones that are released.  Then adjust the mapped system VA
        // (if there is one) to reflect the current frame.  Note that the TB
        // does not need to be flushed due to the careful sliding and when
        // the MDL is finally completely unmapped, the extra information
        // added to the MDL here is used to free the entire original PTE
        // mapping range in one chunk so as not to fragment the PTE space.
        //

        Page = (PPFN_NUMBER)(Mdl + 1);
        NewPage = Page;

        Process = Mdl->Process;

        MdlFlags = Mdl->MdlFlags;

        if (MdlFlags & MDL_DESCRIBES_AWE) {
    
            ASSERT (Process != NULL);
            ASSERT (Process->AweInfo != NULL);
    
            LastPage = Page + PageCount;
    
            //
            // Note neither AWE nor PFN locks are needed for unlocking these
            // MDLs in all but the very rare cases (see below).
            //
    
            do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")    
                if (*Page == MM_EMPTY_LIST) {
    
                    //
                    // There are no more locked pages - if there were none
                    // at all then we're done.
                    //
    
                    break;
                }
    
                ASSERT (MI_IS_PFN (*Page));
                ASSERT (*Page <= MmHighestPhysicalPage);
                Pfn1 = MI_PFN_ELEMENT (*Page);
    
                do {
                    EntryCount = ReadForWriteAccess (&Pfn1->AweReferenceCount);
    
                    ASSERT ((LONG)EntryCount > 0);
                    ASSERT (Pfn1->u4.AweAllocation == 1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    
                    OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                                EntryCount - 1,
                                                                EntryCount);
    
                    if (OriginalCount == EntryCount) {
    
                        //
                        // This thread can be racing against other threads also
                        // calling MmUnlockPages and also a thread calling
                        // NtFreeUserPhysicalPages.  All threads can safely do
                        // interlocked decrements on the "AWE reference count".
                        // Whichever thread drives it to zero is responsible for
                        // decrementing the actual PFN reference count (which
                        // may be greater than 1 due to other non-AWE API calls
                        // being used on the same page).  The thread that
                        // drives this reference count to zero must put the
                        // page on the actual freelist at that time and
                        // decrement various resident available and commitment
                        // counters also.
                        //
    
                        if (OriginalCount == 1) {
    
                            //
                            // This thread has driven the AWE reference count to
                            // zero so it must initiate a decrement of the PFN
                            // reference count (while holding the PFN lock),
                            // etc.
                            //
                            // This path should be rare since typically I/Os
                            // complete before these types of pages are freed by
                            // the app.
                            //
    
                            MiDecrementReferenceCountForAwePage (Pfn1, FALSE);
                        }
    
                        break;
                    }
                } while (TRUE);
    
                Page += 1;
            } while (Page < LastPage);

            i = (ULONG) (Page - (PPFN_NUMBER)(Mdl + 1));
        }
        else {

            LOCK_PFN2 (OldIrql);
    
            for (i = 0; i < PageCount; i += 1) {
    
                //
                // Decrement the stale page frames now, this will unlock them
                // resulting in them being immediately reused if necessary.
                //
    
                if ((MdlFlags & MDL_PAGES_LOCKED) &&
                    ((MdlFlags & MDL_IO_SPACE) == 0)) {
    
                    ASSERT ((MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
    
                    Pfn1 = MI_PFN_ELEMENT (*Page);
    
                    if (MdlFlags & MDL_WRITE_OPERATION) {
    
                        //
                        // If this was a write operation set the modified bit
                        // in the PFN database.
                        //
    
                        MI_SET_MODIFIED (Pfn1, 1, 0x3);
    
                        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                                     (Pfn1->u3.e1.WriteInProgress == 0)) {
    
                            FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);
    
                            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                                MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                            }
                        }
                    }
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
                }
                Page += 1;
            }
    
            UNLOCK_PFN2 (OldIrql);

            if (Process != NULL) {
    
                if ((MdlFlags & MDL_PAGES_LOCKED) &&
                    ((MdlFlags & MDL_IO_SPACE) == 0)) {
    
                    ASSERT ((MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
                    ASSERT ((SPFN_NUMBER)Process->NumberOfLockedPages >= 0);
    
                    InterlockedExchangeAddSizeT (&Process->NumberOfLockedPages,
                                                 0 - (PFN_NUMBER) PageCount);
                }
    
                if (MmTrackLockedPages == TRUE) {
                    MiUpdateMdlTracker (Mdl, PageCount);
                }
            }
        }

        //
        // Now ripple the remaining pages to the front of the MDL, effectively
        // purging the old ones which have just been released.
        //

        ASSERT (i < NumberOfPages);

        for ( ; i < NumberOfPages; i += 1) {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }

            *NewPage = *Page;
            NewPage += 1;
            Page += 1;
        }

        //
        // If the MDL has been mapped, stash the number of pages advanced
        // at the end of the frame list inside the MDL and mark the MDL as
        // containing extra PTEs to free.  Thus when the MDL is finally
        // completely unmapped, this can be used so the entire original PTE
        // mapping range can be freed in one chunk so as not to fragment the
        // PTE space.
        //

        if (MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {

#if DBG
            InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, PageCount);
            MiAdvancesGiven += PageCount;
#endif

            if (MdlFlags & MDL_FREE_EXTRA_PTES) {

                //
                // This MDL has already been advanced at least once.  Any
                // PTEs from those advancements need to be preserved now.
                //

                ASSERT (*Page <= MiCurrentAdvancedPages - PageCount);
                PageCount += *(PULONG)Page;
            }
            else {
                Mdl->MdlFlags |= MDL_FREE_EXTRA_PTES;
            }

            *NewPage = PageCount;
        }
    }

    return STATUS_SUCCESS;
}

NTKERNELAPI
NTSTATUS
MmProtectMdlSystemAddress (
    __in PMDL MemoryDescriptorList,
    __in ULONG NewProtect
    )

/*++

Routine Description:

    This function protects the system address range specified
    by the argument Memory Descriptor List.

    Note the caller must make this MDL mapping readwrite before finally
    freeing (or reusing) it.

Arguments:

    MemoryDescriptorList - Supplies the MDL describing the virtual range.

    NewProtect - Supplies the protection to set the pages to (PAGE_XX).

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, IRQL DISPATCH_LEVEL or below.  The caller is responsible for
    synchronizing access to this routine.

--*/

{
    KIRQL OldIrql;
    PVOID BaseAddress;
    PVOID SystemVa;
    MMPTE PteContents;
    PMMPTE PointerPte;
    ULONG ProtectionMask;
#if DBG
    PMMPFN Pfn1;
    PPFN_NUMBER Page;
#endif
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    MMPTE OriginalPte;
    LOGICAL WasValid;
    PMM_PTE_MAPPING Map;
    PMM_PTE_MAPPING MapEntry;
    PMM_PTE_MAPPING FoundMap;
    PLIST_ENTRY NextEntry;

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PAGES_LOCKED) != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) == 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) == 0);
    ASSERT (MemoryDescriptorList->ByteCount != 0);

    if ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    BaseAddress = MemoryDescriptorList->MappedSystemVa;

    ASSERT (BaseAddress > MM_HIGHEST_USER_ADDRESS);

    //
    // On some architectures, we may use a large or KSEG-based MDL mapping
    // which must always be READWRITE, so disallow any changes to these.
    //

    if (MI_IS_PHYSICAL_ADDRESS (BaseAddress)) {
        return STATUS_NOT_SUPPORTED;
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);

    //
    // No bogus or copy-on-write protections allowed for these.
    //

    if ((ProtectionMask == MM_INVALID_PROTECTION) ||
        (MI_IS_GUARD (ProtectionMask)) ||
        (MI_IS_NOCACHE (ProtectionMask)) ||
        (MI_IS_WRITECOMBINE (ProtectionMask)) ||
        (ProtectionMask == MM_WRITECOPY) ||
        (ProtectionMask == MM_EXECUTE_WRITECOPY)) {

        return STATUS_INVALID_PAGE_PROTECTION;
    }

    PointerPte = MiGetPteAddress (BaseAddress);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress,
                                           MemoryDescriptorList->ByteCount);

    SystemVa = PAGE_ALIGN (BaseAddress);

    //
    // Initializing Map is not needed for correctness
    // but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    Map = NULL;

    if (ProtectionMask != MM_READWRITE) {

        Map = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(MM_PTE_MAPPING),
                                     'mPmM');

        if (Map == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Map->SystemVa = SystemVa;
        Map->SystemEndVa = (PVOID)((ULONG_PTR)SystemVa + (NumberOfPages << PAGE_SHIFT));
        Map->Protection = ProtectionMask;
    }

#if DBG
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
#endif

    FlushCount = 0;

    while (NumberOfPages != 0) {

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {
            WasValid = TRUE;
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            OriginalPte = PteContents;
        }
        else if ((PteContents.u.Soft.Transition == 1) &&
                 (PteContents.u.Soft.Protection == MM_NOACCESS)) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
            WasValid = FALSE;

            OriginalPte.u.Hard.WriteThrough = PteContents.u.Soft.PageFileLow;
            OriginalPte.u.Hard.CacheDisable = (PteContents.u.Soft.PageFileLow >> 1);

        }
        else {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x1235,
                          (ULONG_PTR)MemoryDescriptorList,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteContents.u.Long);
        }

#if DBG
        ASSERT (*Page == PageFrameIndex);

        if (MI_IS_PFN (PageFrameIndex)) {
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
        }

        Page += 1;
#endif

        if (ProtectionMask == MM_NOACCESS) {

            //
            // To generate a bugcheck on bogus access: Prototype must stay
            // clear, transition must stay set, protection must stay NO_ACCESS.
            //

            MI_MAKE_VALID_PTE_TRANSITION (PteContents, MM_NOACCESS);

            //
            // Stash the cache attributes into the software PTE so they can
            // be restored later.
            //

            PteContents.u.Soft.PageFileLow = OriginalPte.u.Hard.WriteThrough;
            PteContents.u.Soft.PageFileLow |= (OriginalPte.u.Hard.CacheDisable << 1);

            MI_WRITE_INVALID_PTE (PointerPte, PteContents);
        }
        else {
            MI_MAKE_VALID_KERNEL_PTE (PteContents,
                                      PageFrameIndex,
                                      ProtectionMask,
                                      PointerPte);

            if (ProtectionMask & MM_READWRITE) {
                MI_SET_PTE_DIRTY (PteContents);
            }

            //
            // Extract cache type from the original PTE so it can be preserved.
            // Note that since we only allow protection changes (not caching
            // attribute changes), there is no need to flush or sweep TBs on
            // insertion below.
            //

            PteContents.u.Hard.WriteThrough = OriginalPte.u.Hard.WriteThrough;
            PteContents.u.Hard.CacheDisable = OriginalPte.u.Hard.CacheDisable;

            if (WasValid == TRUE) {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);
            }
            else {
                MI_WRITE_VALID_PTE (PointerPte, PteContents);
            }
        }

        if ((WasValid == TRUE) && (FlushCount != MM_MAXIMUM_FLUSH_COUNT)) {
            VaFlushList[FlushCount] = BaseAddress;
            FlushCount += 1;
        }

        BaseAddress = (PVOID)((ULONG_PTR)BaseAddress + PAGE_SIZE);
        PointerPte += 1;
        NumberOfPages -= 1;
    }

    //
    // Flush the TB entries for any relevant pages.
    //

    if (FlushCount == 0) {
        NOTHING;
    }
    else if (FlushCount == 1) {
        MI_FLUSH_SINGLE_TB (VaFlushList[0], TRUE);
    }
    else if (FlushCount < MM_MAXIMUM_FLUSH_COUNT) {
        MI_FLUSH_MULTIPLE_TB (FlushCount, &VaFlushList[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (0x12);
    }

    if (ProtectionMask != MM_READWRITE) {

        //
        // Insert (or update) the list entry describing this range.
        // Don't bother sorting the list as there will never be many entries.
        //

        FoundMap = NULL;

        OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

        NextEntry = MmProtectedPteList.Flink;

        while (NextEntry != &MmProtectedPteList) {

            MapEntry = CONTAINING_RECORD (NextEntry,
                                          MM_PTE_MAPPING,
                                          ListEntry);

            if (MapEntry->SystemVa == SystemVa) {
                ASSERT (MapEntry->SystemEndVa == Map->SystemEndVa);
                MapEntry->Protection = Map->Protection;
                FoundMap = MapEntry;
                break;
            }
            NextEntry = NextEntry->Flink;
        }

        if (FoundMap == NULL) {
            InsertHeadList (&MmProtectedPteList, &Map->ListEntry);
        }

        KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

        if (FoundMap != NULL) {
            ExFreePool (Map);
        }
    }
    else {

        //
        // If there is an existing list entry describing this range, remove it.
        //

        if (!IsListEmpty (&MmProtectedPteList)) {

            FoundMap = NULL;

            OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

            NextEntry = MmProtectedPteList.Flink;

            while (NextEntry != &MmProtectedPteList) {

                MapEntry = CONTAINING_RECORD (NextEntry,
                                              MM_PTE_MAPPING,
                                              ListEntry);

                if (MapEntry->SystemVa == SystemVa) {
                    RemoveEntryList (NextEntry);
                    FoundMap = MapEntry;
                    break;
                }
                NextEntry = NextEntry->Flink;
            }

            KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

            if (FoundMap != NULL) {
                ExFreePool (FoundMap);
            }
        }
    }

    ASSERT (MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA);

    return STATUS_SUCCESS;
}

LOGICAL
MiCheckSystemPteProtection (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function determines whether the faulting virtual address lies
    within the non-writable alternate system PTE mappings.

Arguments:

    StoreInstruction - Supplies nonzero if the operation causes a write into
                       memory, zero if not.

    VirtualAddress - Supplies the virtual address which caused the fault.

Return Value:

    TRUE if the fault was handled by this code (and PTE updated), FALSE if not.

Environment:

    Kernel mode.  Called from the fault handler at any IRQL.

--*/

{
    KIRQL OldIrql;
    PMMPTE PointerPte;
    ULONG ProtectionCode;
    PLIST_ENTRY NextEntry;
    PMM_PTE_MAPPING MapEntry;

    //
    // If PTE mappings with various protections are active and the faulting
    // address lies within these mappings, resolve the fault with
    // the appropriate protections.
    //

    if (IsListEmpty (&MmProtectedPteList)) {
        return FALSE;
    }

    OldIrql = KeAcquireSpinLockRaiseToSynch (&MmProtectedPteLock);

    NextEntry = MmProtectedPteList.Flink;

    while (NextEntry != &MmProtectedPteList) {

        MapEntry = CONTAINING_RECORD (NextEntry,
                                      MM_PTE_MAPPING,
                                      ListEntry);

        if ((VirtualAddress >= MapEntry->SystemVa) &&
            (VirtualAddress < MapEntry->SystemEndVa)) {

            ProtectionCode = MapEntry->Protection;
            KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

            PointerPte = MiGetPteAddress (VirtualAddress);

            if (StoreInstruction != 0) {
                if ((ProtectionCode & MM_READWRITE) == 0) {

                    KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                  (ULONG_PTR)VirtualAddress,
                                  (ULONG_PTR)PointerPte->u.Long,
                                  0,
                                  16);
                }
            }

            MI_NO_FAULT_FOUND (StoreInstruction,
                               PointerPte,
                               VirtualAddress,
                               FALSE);

            //
            // Fault was handled directly here, no need for the caller to
            // do anything.
            //

            return TRUE;
        }
        NextEntry = NextEntry->Flink;
    }

    KeReleaseSpinLock (&MmProtectedPteLock, OldIrql);

    return FALSE;
}

PVOID
MiCreatePhysicalVadRoot (
    IN PEPROCESS Process,
    IN LOGICAL WsHeld
    )

/*++

Routine Description:

    This function creates a physical VAD AVL root table for the specified
    process.

Arguments:

    Process - Supplies the process to add the physical VAD root to.

    WsHeld - Supplies TRUE if the caller holds the WS mutex, FALSE if not.

Return Value:

    The PhysicalVadRoot or NULL.

Environment:

    Kernel mode.  APC_LEVEL or below.

--*/
{
    PETHREAD Thread;
    PMM_AVL_TABLE PhysicalVadRoot;

    //
    // The address space mutex synchronizes the allocation of the
    // EPROCESS PhysicalVadRoot.  This table root is not deleted until
    // the process exits.
    //

    if (Process->PhysicalVadRoot == NULL) {

        PhysicalVadRoot = (PMM_AVL_TABLE) ExAllocatePoolWithTag (
                                                    NonPagedPool,
                                                    sizeof (MM_AVL_TABLE),
                                                    MI_PHYSICAL_VIEW_ROOT_KEY);

        if (PhysicalVadRoot == NULL) {
            return NULL;
        }

        RtlZeroMemory (PhysicalVadRoot, sizeof (MM_AVL_TABLE));
        ASSERT (PhysicalVadRoot->NumberGenericTableElements == 0);
        PhysicalVadRoot->BalancedRoot.u1.Parent = &PhysicalVadRoot->BalancedRoot;

        Thread = PsGetCurrentThread ();

        //
        // Synchronize with concurrent MmProbeAndLockPages callers by
        // acquiring the working set mutex.
        //

        if (WsHeld == FALSE) {
            LOCK_WS (Thread, Process);
        }

        if (Process->PhysicalVadRoot == NULL) {

            Process->PhysicalVadRoot = PhysicalVadRoot;

            if (WsHeld == FALSE) {
                UNLOCK_WS (Thread, Process);
            }
        }
        else {
            if (WsHeld == FALSE) {
                UNLOCK_WS (Thread, Process);
            }
            ExFreePool (PhysicalVadRoot);
        }
    }

    return Process->PhysicalVadRoot;
}

VOID
MiPhysicalViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    )

/*++

Routine Description:

    This function removes a physical VAD from the process chain.

Arguments:

    Process - Supplies the process to remove the physical VAD from.

    Vad - Supplies the Vad to remove.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL, working set and address space mutexes held.

--*/

{
    PETHREAD Thread;
    PRTL_BITMAP BitMap;
    PMI_PHYSICAL_VIEW PhysicalView;
    ULONG BitMapSize;
    TABLE_SEARCH_RESULT SearchResult;
    PFN_NUMBER PagesRequired;

    //
    // Lookup the element and save the result.
    //

    ASSERT (Process->PhysicalVadRoot != NULL);

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       Vad->StartingVpn,
                                       (PMMADDRESS_NODE *) &PhysicalView);

    ASSERT (SearchResult == TableFoundNode);
    ASSERT (PhysicalView->Vad == Vad);

    MiRemoveNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);

    if (Vad->u.VadFlags.VadType == VadWriteWatch) {
        Thread = PsGetCurrentThread ();
        UNLOCK_WS_UNSAFE (Thread, Process);
        BitMap = PhysicalView->u.BitMap;
        BitMapSize = sizeof(RTL_BITMAP) + (ULONG)(((BitMap->SizeOfBitMap + 31) / 32) * 4);
        PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
        ExFreePool (BitMap);
        LOCK_WS_UNSAFE (Thread, Process);
    }

    if (Vad->u.VadFlags.VadType == VadRotatePhysical) {
        MiFreeInPageSupportBlock (PhysicalView->InPageSupport);
        MiReleaseSystemPtes (PhysicalView->u.MappingPte, 2, SystemPteSpace);

        PagesRequired = MiResidentPagesForSpan (MI_VPN_TO_VA (Vad->StartingVpn),
                                                MI_VPN_TO_VA_ENDING (Vad->EndingVpn));
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired, MM_RESAVAIL_FREE_ROTATE_VAD);
    }

    ExFreePool (PhysicalView);

    return;
}

VOID
MiPhysicalViewAdjuster (
    IN PEPROCESS Process,
    IN PMMVAD OldVad,
    IN PMMVAD NewVad
    )

/*++

Routine Description:

    This function re-points a physical VAD in the process chain.

Arguments:

    Process - Supplies the process in which to adjust the physical VAD.

    Vad - Supplies the old Vad to replace.

    NewVad - Supplies the newVad to substitute.

Return Value:

    None.

Environment:

    Kernel mode, called with APCs disabled, working set mutex held.

--*/
{
    PMI_PHYSICAL_VIEW PhysicalView;
    TABLE_SEARCH_RESULT SearchResult;

    //
    // Lookup the element and save the result.
    //

    ASSERT (Process->PhysicalVadRoot != NULL);

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       OldVad->StartingVpn,
                                       (PMMADDRESS_NODE *) &PhysicalView);

    ASSERT (SearchResult == TableFoundNode);
    ASSERT (PhysicalView->Vad == OldVad);

    PhysicalView->Vad = NewVad;

    return;
}

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     )

/*++

Routine Description:

    This function maps physical pages described by a memory descriptor
    list into the user portion of the virtual address space.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.


    StartingVa - Supplies the starting address.

    CacheType - Supplies the type of cache mapping to use for the MDL.
                MmCached indicates "normal" user mappings.

    BaseVa - Supplies the base address of the view. If the initial
             value of this argument is not null, then the view will
             be allocated starting at the specified virtual
             address rounded down to the next 64kb address
             boundary. If the initial value of this argument is
             null, then the operating system will determine
             where to allocate the view.

Return Value:

    Returns the base address where the pages are mapped.  The base address
    has the same offset as the virtual address in the MDL.

    This routine will raise an exception if quota limits or VM limits are
    exceeded.

Environment:

    Kernel mode.  APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER SavedPageCount;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PCHAR Va;
    MMPTE TempPte;
    PVOID EndingAddress;
    PMMVAD_LONG Vad;
    PETHREAD Thread;
    PEPROCESS Process;
    PMMPFN Pfn2;
    PVOID UsedPageTableHandle;
    PMI_PHYSICAL_VIEW PhysicalView;
    NTSTATUS Status;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE ();
    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    //
    // If a noncachable mapping is requested, none of the pages in the
    // requested MDL can reside in a large page.  Otherwise we would be
    // creating an incoherent overlapping TB entry as the same physical
    // page would be mapped by 2 different TB entries with different
    // cache attributes.
    //

    IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (CacheAttribute != MiCached) {

        SavedPageCount = NumberOfPages;

        LOCK_PFN (OldIrql);

        do {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }
            PageFrameIndex = *Page;
            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN (OldIrql);
                MiNonCachedCollisions += 1;

                ExRaiseStatus (STATUS_INVALID_ADDRESS);
            }

            Page += 1;
            NumberOfPages -= 1;
        } while (NumberOfPages != 0);
        UNLOCK_PFN (OldIrql);

        NumberOfPages = SavedPageCount;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    }

    //
    // Map the pages into the user part of the address as user
    // read/write no-delete.
    //

    Vad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_LONG), 'ldaV');

    if (Vad == NULL) {

        ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
    }

    PhysicalView = (PMI_PHYSICAL_VIEW)ExAllocatePoolWithTag (NonPagedPool,
                                                             sizeof(MI_PHYSICAL_VIEW),
                                                             MI_PHYSICAL_VIEW_KEY);
    if (PhysicalView == NULL) {
        ExFreePool (Vad);

        ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory (Vad, sizeof (MMVAD_LONG));

    ASSERT (Vad->ControlArea == NULL);
    ASSERT (Vad->FirstPrototypePte == NULL);
    ASSERT (Vad->u.LongFlags == 0);
    Vad->u.VadFlags.Protection = MM_READWRITE;
    Vad->u.VadFlags.VadType = VadDevicePhysicalMemory;
    Vad->u.VadFlags.PrivateMemory = 1;

    Vad->u2.VadFlags2.LongVad = 1;

    PhysicalView->Vad = (PMMVAD) Vad;
    PhysicalView->VadType = VadDevicePhysicalMemory;

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

    //
    // Make sure the specified starting and ending addresses are
    // within the user part of the virtual address space.
    //

    if (BaseVa != NULL) {

        if (BYTE_OFFSET (BaseVa) != 0) {

            //
            // Invalid base address.
            //

            Status = STATUS_INVALID_ADDRESS;
            goto ErrorReturn;
        }

        EndingAddress = (PVOID)((PCHAR)BaseVa + ((ULONG_PTR)NumberOfPages * PAGE_SIZE) - 1);

        if ((EndingAddress <= BaseVa) || (EndingAddress > MM_HIGHEST_VAD_ADDRESS)) {
            //
            // Invalid region size.
            //

            Status = STATUS_INVALID_ADDRESS;
            goto ErrorReturn;
        }

        LOCK_ADDRESS_SPACE (Process);

        //
        // Make sure the address space was not deleted, if so, return an error.
        //

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReturn;
        }

        //
        // Make sure the address space is not already in use.
        //

        if (MiCheckForConflictingVadExistence (Process, BaseVa, EndingAddress) == TRUE) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorReturn;
        }
    }
    else {

        //
        // Get the address creation mutex.
        //

        LOCK_ADDRESS_SPACE (Process);

        //
        // Make sure the address space was not deleted, if so, return an error.
        //

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            UNLOCK_ADDRESS_SPACE (Process);
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReturn;
        }

        Status = MiFindEmptyAddressRange ((ULONG_PTR)NumberOfPages * PAGE_SIZE,
                                          X64K,
                                          0,
                                          &BaseVa);

        if (!NT_SUCCESS (Status)) {
            UNLOCK_ADDRESS_SPACE (Process);
            goto ErrorReturn;
        }

        EndingAddress = (PVOID)((PCHAR)BaseVa + ((ULONG_PTR)NumberOfPages * PAGE_SIZE) - 1);
    }

    Vad->StartingVpn = MI_VA_TO_VPN (BaseVa);
    Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);

    PhysicalView->StartingVpn = Vad->StartingVpn;
    PhysicalView->EndingVpn = Vad->EndingVpn;

    //
    // The PhysicalVadRoot table root is not deleted until
    // the process exits.
    //

    if ((Process->PhysicalVadRoot == NULL) &&
        (MiCreatePhysicalVadRoot (Process, FALSE) == NULL)) {

        UNLOCK_ADDRESS_SPACE (Process);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

    Status = MiInsertVadCharges ((PMMVAD) Vad, Process);

    if (!NT_SUCCESS (Status)) {
        UNLOCK_ADDRESS_SPACE (Process);
        goto ErrorReturn;
    }

    LOCK_WS_UNSAFE (Thread, Process);

    MiInsertVad ((PMMVAD) Vad, Process);

    //
    // The VAD has been inserted, but the physical view descriptor cannot
    // be until the page table page hierarchy is in place.  This is to
    // prevent races with probes.
    //

    //
    // Create a page table and fill in the mappings for the Vad.
    //

    Va = BaseVa;
    PointerPte = MiGetPteAddress (BaseVa);

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        PointerPde = MiGetPteAddress (PointerPte);

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

        ASSERT (PointerPte->u.Hard.Valid == 0);

        //
        // Another zeroed PTE is being made non-zero.
        //

        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        TempPte = ValidUserPte;
        TempPte.u.Hard.PageFrameNumber = *Page;

        if ((IoMapping == 0) || (MI_IS_PFN (*Page))) {

            Pfn2 = MI_PFN_ELEMENT (*Page);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

            switch (Pfn2->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {
                        //
                        // The caller asked for a noncached or writecombined
                        // mapping, but the page is already mapped cached by
                        // someone else.  Override the caller's request in
                        // order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                        //
                        // The caller asked for a cached or writecombined
                        // mapping, but the page is already mapped noncached
                        // by someone else.  Override the caller's request
                        // in order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                        //
                        // The caller asked for a cached or noncached
                        // mapping, but the page is already mapped
                        // writecombined by someone else.  Override the
                        // caller's request in order to keep the TB page
                        // attribute coherent.
                        //

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                    //
                    // This better be for a page allocated with
                    // MmAllocatePagesForMdl.  Otherwise it might be a
                    // page on the freelist which could subsequently be
                    // given out with a different attribute !
                    //

                    ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn2->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }
        else {
            switch (CacheAttribute) {

                case MiCached:
                    break;

                case MiNonCached:
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        //
        // A PTE just went from not present, not transition to
        // present.  The share count and valid count must be
        // updated in the page table page which contains this PTE.
        //

        Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
        LOCK_PFN (OldIrql);
        Pfn2->u2.ShareCount += 1;
        UNLOCK_PFN (OldIrql);

        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
        Va += PAGE_SIZE;
    } while (NumberOfPages != 0);

    //
    // Insert the physical view descriptor now that the page table page
    // hierarchy is in place.  Note probes can find this descriptor immediately
    // once the working set mutex is released.
    //

    ASSERT (Process->PhysicalVadRoot != NULL);

    MiInsertNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);

    UNLOCK_WS_AND_ADDRESS_SPACE (Thread, Process);

    ASSERT (BaseVa != NULL);

    BaseVa = (PVOID)((PCHAR)BaseVa + MemoryDescriptorList->ByteOffset);

    return BaseVa;

ErrorReturn:

    ExFreePool (Vad);
    ExFreePool (PhysicalView);

    ExRaiseStatus (Status);
}

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     )

/*++

Routine Description:

    This routine unmaps locked pages which were previously mapped via
    a MmMapLockedPages function.

Arguments:

    BaseAddress - Supplies the base address where the pages were previously
                  mapped.

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

Return Value:

    None.

Environment:

    Kernel mode.  APC_LEVEL or below.

--*/

{
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif
    PVOID StartingVa;
    PVOID EndingVa;
    KIRQL OldIrql;
    PMMVAD Vad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PVOID TempVa;
    PETHREAD Thread;
    PEPROCESS Process;
    PMMPFN PageTablePfn;
    PFN_NUMBER PageTablePage;
    PVOID UsedPageTableHandle;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

    MmLockPageableSectionByHandle (ExPageLockHandle);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                           MemoryDescriptorList->ByteCount);

    ASSERT (NumberOfPages != 0);

    PointerPte = MiGetPteAddress (BaseAddress);
    PointerPde = MiGetPdeAddress (BaseAddress);

    //
    // This was mapped into the user portion of the address space and
    // the corresponding virtual address descriptor must be deleted.
    //
    // Get the working set pushlock and the address creation mutex.
    //

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

    LOCK_ADDRESS_SPACE (Process);

    Vad = MiLocateAddress (BaseAddress);

    if ((Vad == NULL) || (Vad->u.VadFlags.VadType != VadDevicePhysicalMemory)) {
        UNLOCK_ADDRESS_SPACE (Process);
        MmUnlockPageableImageSection (ExPageLockHandle);
        return;
    }

    PreviousVad = MiGetPreviousVad (Vad);
    NextVad = MiGetNextVad (Vad);

    StartingVa = MI_VPN_TO_VA (Vad->StartingVpn);
    EndingVa = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

    MiRemoveVadCharges (Vad, Process);

    //
    // Return commitment for page table pages if possible.
    //

    MiReturnPageTablePageCommitment (StartingVa,
                                     EndingVa,
                                     Process,
                                     PreviousVad,
                                     NextVad);

    LOCK_WS_UNSAFE (Thread, Process);

    MiPhysicalViewRemover (Process, Vad);

    MiRemoveVad (Vad, Process);

    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (BaseAddress);
    PageTablePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    PageTablePfn = MI_PFN_ELEMENT (PageTablePage);

    //
    // Get the PFN lock so we can safely decrement share and valid
    // counts on page table pages.
    //

    LOCK_PFN (OldIrql);

    do {

        if (*Page == MM_EMPTY_LIST) {
            break;
        }

        ASSERT64 (MiGetPdeAddress(PointerPte)->u.Hard.Valid == 1);
        ASSERT (MiGetPteAddress(PointerPte)->u.Hard.Valid == 1);
        ASSERT (PointerPte->u.Hard.Valid == 1);

        //
        // Another PTE is being zeroed.
        //

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        MI_WRITE_ZERO_PTE (PointerPte);

        if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
            PteFlushList.Count += 1;
        }

        MiDecrementShareCountInline (PageTablePfn, PageTablePage);

        PointerPte += 1;
        NumberOfPages -= 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
        Page += 1;

        if ((MiIsPteOnPdeBoundary(PointerPte)) || (NumberOfPages == 0)) {

            if (PteFlushList.Count != 0) {
                MiFlushPteList (&PteFlushList);
            }

            PointerPde = MiGetPteAddress(PointerPte - 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);

            //
            // If all the entries have been eliminated from the previous
            // page table page, delete the page table page itself.  Likewise
            // with the page directory and parent pages.
            //

            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                ASSERT (PointerPde->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 3)
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte (PointerPde);
                MiDeletePte (PointerPde,
                             TempVa,
                             FALSE,
                             Process,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
                if ((MiIsPteOnPpeBoundary(PointerPte)) || (NumberOfPages == 0)) {
    
                    PointerPpe = MiGetPteAddress (PointerPde);
                    ASSERT (PointerPpe->u.Hard.Valid == 1);
    
                    //
                    // If all the entries have been eliminated from the previous
                    // page directory page, delete the page directory page too.
                    //
    
                    if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                        ASSERT (PointerPpe->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 4)
                        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                        TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                        MiDeletePte (PointerPpe,
                                     TempVa,
                                     FALSE,
                                     Process,
                                     NULL,
                                     NULL,
                                     OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                        if ((MiIsPteOnPxeBoundary(PointerPte)) || (NumberOfPages == 0)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            ASSERT (PointerPxe->u.Long != 0);
                            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                                TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                                MiDeletePte (PointerPxe,
                                             TempVa,
                                             FALSE,
                                             Process,
                                             NULL,
                                             NULL,
                                             OldIrql);
                            }
                        }
#endif    
                    }
                }
#endif
            }

            if (NumberOfPages == 0) {
                break;
            }

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (BaseAddress);
            PointerPde += 1;
            PageTablePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
            PageTablePfn = MI_PFN_ELEMENT (PageTablePage);
        }

    } while (NumberOfPages != 0);

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS_AND_ADDRESS_SPACE (Thread, Process);

    ExFreePool (Vad);

    MmUnlockPageableImageSection (ExPageLockHandle);

    return;
}

#define MI_LARGE_PAGE_VA_SPACE ((ULONG64)8 * 1024 * 1024 * 1024)  // Relatively arbitrary

#if (_MI_PAGING_LEVELS>=3)

PVOID MiLargeVaStart;
ULONG MiLargeVaInUse [(MI_LARGE_PAGE_VA_SPACE / MM_MINIMUM_VA_FOR_LARGE_PAGE) / 32];

#endif

VOID
MiInitializeLargePageSupport (
    VOID
    )

/*++

Routine Description:

    This function is called once at system initialization.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, INIT time.  Resident available pages are not yet initialized,
    but everything else is.

--*/

{

#if (_MI_PAGING_LEVELS>=3)

    ULONG PageColor;
    KIRQL OldIrql;
    MMPTE TempPte;
    PMMPTE PointerPpe;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;

    MiLargeVaStart = (PVOID)-1;

    RtlInitializeBitMap (&MiLargeVaBitMap,
                         MiLargeVaInUse,
                         (ULONG) sizeof (MiLargeVaInUse) * 8);

    ASSERT (MmNonPagedPoolEnd != NULL);

    KeInitializeSpinLock (&MiLargePageLock);

#if (_MI_PAGING_LEVELS>=4)

    PointerPpe = MiGetPxeAddress (MmNonPagedPoolEnd) + 1;

    while (PointerPpe->u.Long != 0) {
        PointerPpe += 1;
    }

    //
    // Allocate the top level extended page directory parent.
    //

    if (MiChargeCommitment (1, NULL) == FALSE) {
        RtlSetAllBits (&MiLargeVaBitMap);
        return;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LARGE_VA_PAGES, 1);

    ASSERT (PointerPpe->u.Long == 0);
    PointerPpe->u.Long = MM_DEMAND_ZERO_WRITE_PTE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    PageColor = MI_GET_PAGE_COLOR_FROM_VA (VirtualAddress);

    PageFrameIndex = MiRemoveZeroPage (PageColor);

    MiInitializePfn (PageFrameIndex, PointerPpe, 1);

    UNLOCK_PFN (OldIrql);

    MI_MAKE_VALID_PTE (TempPte, PageFrameIndex, MM_READWRITE, PointerPpe);

    MI_SET_PTE_DIRTY (TempPte);

    MI_WRITE_VALID_PTE (PointerPpe, TempPte);

    PointerPpe = MiGetVirtualAddressMappedByPte (PointerPpe);

#else

    PointerPpe = MiGetPpeAddress (MmNonPagedPoolEnd) + 1;

    while (PointerPpe->u.Long != 0) {
        PointerPpe += 1;
    }

#endif

    MiLargeVaStart = MiGetVirtualAddressMappedByPpe (PointerPpe);

    NumberOfPages = (MI_LARGE_PAGE_VA_SPACE / MM_VA_MAPPED_BY_PPE);

    ASSERT (NumberOfPages != 0);

    if (MiChargeCommitment (NumberOfPages, NULL) == FALSE) {
        RtlSetAllBits (&MiLargeVaBitMap);
        return;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LARGE_VA_PAGES, NumberOfPages);

    do {

        ASSERT (PointerPpe->u.Long == 0);
        PointerPpe->u.Long = MM_DEMAND_ZERO_WRITE_PTE;

        LOCK_PFN (OldIrql);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, OldIrql);
        }

        PageColor = MI_GET_PAGE_COLOR_FROM_VA (VirtualAddress);

        PageFrameIndex = MiRemoveZeroPage (PageColor);

        MiInitializePfn (PageFrameIndex, PointerPpe, 1);

        UNLOCK_PFN (OldIrql);

        MI_MAKE_VALID_PTE (TempPte, PageFrameIndex, MM_READWRITE, PointerPpe);

        MI_SET_PTE_DIRTY (TempPte);

        MI_WRITE_VALID_PTE (PointerPpe, TempPte);

        PointerPpe += 1;

        NumberOfPages -= 1;

    } while (NumberOfPages != 0);

    RtlClearAllBits (&MiLargeVaBitMap);

#else

    //
    // Initialize process tracking so that large page system PTE mappings
    // can be rippled during creation/deletion.
    //

    MiLargePageHyperPte = MiReserveSystemPtes (1, SystemPteSpace);

    if (MiLargePageHyperPte == NULL) {
        MiIssueNoPtesBugcheck (1, SystemPteSpace);
    }

    MiLargePageHyperPte->u.Long = 0;

    InitializeListHead (&MmProcessList);

    InsertTailList (&MmProcessList, &PsGetCurrentProcess()->MmProcessLinks);

#endif

    return;
}

#if !defined (_WIN64)
PMMPTE MiInitialSystemPageDirectory;
#endif


PVOID
MiMapWithLargePages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Protection,
    IN MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function maps the specified physical address into the non-pageable
    portion of the system address space using large TB entries.  If the range
    cannot be mapped using large TB entries then NULL is returned and the
    caller will map it with small TB entries.

Arguments:

    PageFrameIndex - Supplies the starting page frame index to map.

    NumberOfPages - Supplies the number of pages to map.

    Protection - Supplies the number of pages to map.

    CacheType - Supplies MmNonCached if the physical address is to be mapped
                as non-cached, MmCached if the address should be cached, and
                MmWriteCombined if the address should be cached and
                write-combined as a frame buffer which is to be used only by
                the video port driver.  All other callers should use
                MmUSWCCached.  MmUSWCCached is available only if the PAT
                feature is present and available.

                For I/O device registers, this is usually specified
                as MmNonCached.

Return Value:

    Returns the virtual address which maps the specified physical addresses.
    The value NULL is returned if sufficient large virtual address space for
    the mapping could not be found.

Environment:

    Kernel mode, Should be IRQL of APC_LEVEL or below, but unfortunately
    callers are coming in at DISPATCH_LEVEL and it's too late to change the
    rules now.  This means you can never make this routine pageable.

--*/

{
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    PVOID BaseVa;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    KIRQL OldIrql;
    LOGICAL IoMapping;
#if defined(_WIN64)
    ULONG StartPosition;
    ULONG NumberOfBits;
#else
    PMMPTE TargetPde;
    PMMPTE TargetPdeBase;
    PMMPTE PointerPdeBase;
    PFN_NUMBER PageDirectoryIndex;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY NextEntry;
#endif
#if defined (_X86PAE_)
    ULONG i;
    PMMPTE PaeTop;
#endif

    ASSERT ((NumberOfPages % (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) == 0);
    ASSERT ((PageFrameIndex % (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) == 0);

#ifdef _X86_
    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        return NULL;
    }
#endif

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, TRUE);

    IoMapping = !MI_IS_PFN (PageFrameIndex);

#if defined(_WIN64)

    NumberOfBits = (ULONG)(NumberOfPages / (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

    ExAcquireSpinLock (&MiLargePageLock, &OldIrql);

    StartPosition = RtlFindClearBitsAndSet (&MiLargeVaBitMap,
                                            NumberOfBits,
                                            0);

    ExReleaseSpinLock (&MiLargePageLock, OldIrql);

    if (StartPosition == NO_BITS_FOUND) {
        return NULL;
    }

    BaseVa = (PVOID)((PCHAR)MiLargeVaStart + ((ULONG_PTR)StartPosition * MM_MINIMUM_VA_FOR_LARGE_PAGE));

    if (IoMapping) {

        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            ExAcquireSpinLock (&MiLargePageLock, &OldIrql);
            RtlClearBits (&MiLargeVaBitMap, StartPosition, NumberOfBits);
            ExReleaseSpinLock (&MiLargePageLock, OldIrql);
            return NULL;
        }
    }

    PointerPde = MiGetPdeAddress (BaseVa);

#else

    PointerPde = MiReserveAlignedSystemPtes ((ULONG)NumberOfPages,
                                             SystemPteSpace,
                                             MM_MINIMUM_VA_FOR_LARGE_PAGE);

    if (PointerPde == NULL) {
        return NULL;
    }

    ASSERT (BYTE_OFFSET (PointerPde) == 0);

    BaseVa = MiGetVirtualAddressMappedByPte (PointerPde);
    ASSERT (((ULONG_PTR)BaseVa & (MM_VA_MAPPED_BY_PDE - 1)) == 0);

    if (IoMapping) {

        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            MiReleaseSystemPtes (PointerPde,
                                 NumberOfPages,
                                 SystemPteSpace);
            return NULL;
        }
    }

    PointerPde = MiGetPteAddress (PointerPde);

    PointerPdeBase = PointerPde;

#endif

    MI_MAKE_VALID_PTE (TempPde,
                       PageFrameIndex,
                       Protection,
                       PointerPde);

    MI_SET_PTE_DIRTY (TempPde);
    MI_SET_ACCESSED_IN_PTE (&TempPde, 1);
    MI_SET_GLOBAL_STATE (TempPde, 1);
    MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_LARGE_PTE_CACHING (TempPde);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_LARGE_PTE_WRITE_COMBINE (TempPde);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    LastPde = PointerPde + (NumberOfPages / (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT));

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

#if defined(_WIN64)

    while (PointerPde < LastPde) {

        ASSERT (PointerPde->u.Long == 0);

        MI_WRITE_VALID_PTE (PointerPde, TempPde);

        TempPde.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

        PointerPde += 1;
    }

#else

    CurrentProcess = PsGetCurrentProcess ();

    LOCK_EXPANSION2 (OldIrql);

    NextEntry = MmProcessList.Flink;

    while (NextEntry != &MmProcessList) {

        Process = CONTAINING_RECORD (NextEntry, EPROCESS, MmProcessLinks);

        // Two process states must be carefully handled here -
        //
        // 1.  Processes that are just being created where they are still
        //     initializing their page directory, etc.
        //
        // 2.  Processes that are outswapped.
        //

        if (Process->Flags & PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED) {

            // 
            // The process is further along in process creation or is still
            // outswapped.  Either way, an update is already queued so our
            // current changes will be processed later anyway before the process
            // can run so no need to do anything here.
            // 

            NOTHING;
        }
        else if (Process->Pcb.DirectoryTableBase[0] == 0) {

            //
            // This process is being created and there is no way to tell where
            // during creation it is (ie: it may be filling PDEs right now!).
            // So just mark the process as needing a PDE update at the
            // beginning of MmInitializeProcessAddressSpace.
            //

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else if (Process->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) {

            // 
            // This process is outswapped.  Even though the page directory
            // may still be in transition, the process must be inswapped
            // before it can run again, so just mark the process as needing
            // a PDE update at that time.
            // 

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else {

            //
            // This process is resident so update the relevant PDEs in its
            // address space right now.
            //

            PointerPde = PointerPdeBase;
            TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

#if !defined (_X86PAE_)
            PageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#else
            //
            // The range cannot cross PAE PDPTE entries, but we do need to
            // locate which entry it does lie in.
            //

            PaeTop = Process->PaeTop;

            i = (((ULONG_PTR) PointerPde - PDE_BASE) >> PAGE_SHIFT);
            ASSERT ((PaeTop + i)->u.Hard.Valid == 1);
            PageDirectoryIndex = (PFN_NUMBER)((PaeTop + i)->u.Hard.PageFrameNumber);
#endif

            TargetPdeBase = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                    CurrentProcess,
                                                    PageDirectoryIndex);

            TargetPde = (PMMPTE)((PCHAR) TargetPdeBase + BYTE_OFFSET (PointerPde));

            while (PointerPde < LastPde) {

                ASSERT (TargetPde->u.Long != 0);
                ASSERT (TargetPde->u.Hard.Valid != 0);

                *TargetPde = TempPde;

                TempPde.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

                PointerPde += 1;
                TargetPde += 1;
            }

            MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, TargetPdeBase);
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION2 (OldIrql);

#endif

    //
    // Force all processors to use the latest mappings.
    //

    MI_FLUSH_ENTIRE_TB (3);

    return BaseVa;
}


VOID
MiUnmapLargePages (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function unmaps a range of physical addresses which were previously
    mapped via MiMapWithLargePages.

Arguments:

    BaseAddress - Supplies the base virtual address where the physical
                  address was previously mapped.

    NumberOfBytes - Supplies the number of bytes which were mapped.

Return Value:

    None.

Environment:

    Kernel mode, Should be IRQL of APC_LEVEL or below, but unfortunately
    callers are coming in at DISPATCH_LEVEL and it's too late to change the
    rules now.  This means you can never make this routine pageable.

--*/

{
    PMMPTE PointerPde;
    PMMPTE LastPde;
    KIRQL OldIrql;
#if defined(_WIN64)
    ULONG StartPosition;
    ULONG NumberOfBits;
#else
    PMMPTE RestorePde;
    PMMPTE TargetPde;
    PMMPTE TargetPdeBase;
    PMMPTE PointerPdeBase;
    PFN_NUMBER PageDirectoryIndex;
    PEPROCESS Process;
    PEPROCESS CurrentProcess;
    PLIST_ENTRY NextEntry;
#endif
#if defined (_X86PAE_)
    PMMPTE PaeTop;
    ULONG i;
#endif

    ASSERT (NumberOfBytes != 0);

    ASSERT (((ULONG_PTR)BaseAddress % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);
    ASSERT ((NumberOfBytes % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);

#if defined(_WIN64)
    NumberOfBits = (ULONG)(NumberOfBytes / MM_MINIMUM_VA_FOR_LARGE_PAGE);

    StartPosition = (ULONG)(((ULONG_PTR)BaseAddress - (ULONG_PTR)MiLargeVaStart) / MM_MINIMUM_VA_FOR_LARGE_PAGE);

    ASSERT (RtlAreBitsSet (&MiLargeVaBitMap, StartPosition, NumberOfBits) == TRUE);
#endif

    PointerPde = MiGetPdeAddress (BaseAddress);

    LastPde = PointerPde + (NumberOfBytes / MM_VA_MAPPED_BY_PDE);

#if defined(_WIN64)

    while (PointerPde < LastPde) {

        ASSERT (PointerPde->u.Hard.Valid != 0);
        ASSERT (PointerPde->u.Long != 0);
        ASSERT (MI_PDE_MAPS_LARGE_PAGE (PointerPde));

        MI_WRITE_ZERO_PTE (PointerPde);

        PointerPde += 1;
    }

#else

    PointerPdeBase = PointerPde;

    CurrentProcess = PsGetCurrentProcess ();

    LOCK_EXPANSION2 (OldIrql);

    NextEntry = MmProcessList.Flink;

    while (NextEntry != &MmProcessList) {

        Process = CONTAINING_RECORD (NextEntry, EPROCESS, MmProcessLinks);

        // Two process states must be carefully handled here -
        //
        // 1.  Processes that are just being created where they are still
        //     initializing their page directory, etc.
        //
        // 2.  Processes that are outswapped.
        //

        if (Process->Flags & PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED) {

            // 
            // The process is further along in process creation or is still
            // outswapped.  Either way, an update is already queued so our
            // current changes will be processed later anyway before the process
            // can run so no need to do anything here.
            // 

            NOTHING;
        }
        else if (Process->Pcb.DirectoryTableBase[0] == 0) {

            //
            // This process is being created and there is no way to tell where
            // during creation it is (ie: it may be filling PDEs right now!).
            // So just mark the process as needing a PDE update at the
            // beginning of MmInitializeProcessAddressSpace.
            //

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else if (Process->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) {

            // 
            // This process is outswapped.  Even though the page directory
            // may still be in transition, the process must be inswapped
            // before it can run again, so just mark the process as needing
            // a PDE update at that time.
            // 

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);
        }
        else {

            //
            // This process is resident so update the relevant PDEs in its
            // address space right now.
            //

            PointerPde = PointerPdeBase;

#if !defined (_X86PAE_)
            PageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#else
            //
            // The range cannot cross PAE PDPTE entries, but we do need to
            // locate which entry it does lie in.
            //

            PaeTop = Process->PaeTop;

            i = (((ULONG_PTR) PointerPde - PDE_BASE) >> PAGE_SHIFT);
            ASSERT ((PaeTop + i)->u.Hard.Valid == 1);
            PageDirectoryIndex = (PFN_NUMBER)((PaeTop + i)->u.Hard.PageFrameNumber);
#endif

            TargetPdeBase = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                    CurrentProcess,
                                                    PageDirectoryIndex);

            TargetPde = (PMMPTE)((PCHAR) TargetPdeBase + BYTE_OFFSET (PointerPde));

            RestorePde = MiInitialSystemPageDirectory + (PointerPde - (PMMPTE)PDE_BASE);

            while (PointerPde < LastPde) {

                ASSERT (TargetPde->u.Long != 0);
                ASSERT (TargetPde->u.Hard.Valid != 0);
                ASSERT (RestorePde->u.Long != 0);
                ASSERT (RestorePde->u.Hard.Valid != 0);

                *TargetPde = *RestorePde;

                PointerPde += 1;
                TargetPde += 1;
                RestorePde += 1;
            }

            MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, TargetPdeBase);
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_EXPANSION2 (OldIrql);

#endif

    //
    // Force all processors to use the latest mappings.
    //

    MI_FLUSH_ENTIRE_TB (4);

#if defined(_WIN64)

    ExAcquireSpinLock (&MiLargePageLock, &OldIrql);

    RtlClearBits (&MiLargeVaBitMap, StartPosition, NumberOfBits);

    ExReleaseSpinLock (&MiLargePageLock, OldIrql);

#else

    PointerPde = MiGetVirtualAddressMappedByPte (PointerPdeBase);

    MiReleaseSystemPtes (PointerPde,
                         (ULONG)(NumberOfBytes >> PAGE_SHIFT),
                         SystemPteSpace);

#endif

    return;
}


__out_bcount(NumberOfBytes) PVOID
MmMapIoSpace (
    __in PHYSICAL_ADDRESS PhysicalAddress,
    __in SIZE_T NumberOfBytes,
    __in MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function maps the specified physical address into the non-pageable
    portion of the system address space.

Arguments:

    PhysicalAddress - Supplies the starting physical address to map.

    NumberOfBytes - Supplies the number of bytes to map.

    CacheType - Supplies MmNonCached if the physical address is to be mapped
                as non-cached, MmCached if the address should be cached, and
                MmWriteCombined if the address should be cached and
                write-combined as a frame buffer which is to be used only by
                the video port driver.  All other callers should use
                MmUSWCCached.  MmUSWCCached is available only if the PAT
                feature is present and available.

                For I/O device registers, this is usually specified
                as MmNonCached.

Return Value:

    Returns the virtual address which maps the specified physical addresses.
    The value NULL is returned if sufficient virtual address space for
    the mapping could not be found.

Environment:

    Kernel mode, Should be IRQL of APC_LEVEL or below, but unfortunately
    callers are coming in at DISPATCH_LEVEL and it's too late to change the
    rules now.  This means you can never make this routine pageable.

--*/

{
    KIRQL OldIrql;
    CSHORT IoMapping;
    PMMPFN Pfn1;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;
    PMMPTE PointerPte;
    PVOID BaseVa;
    MMPTE TempPte;
    PMDL TempMdl;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PVOID CallingAddress;
    PVOID CallersCaller;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    //
    // For compatibility for when CacheType used to be passed as a BOOLEAN
    // mask off the upper bits (TRUE == MmCached, FALSE == MmNonCached).
    //

    CacheType &= 0xFF;

    if (CacheType >= MmMaximumCacheType) {
        return NULL;
    }

#if !defined (_MI_MORE_THAN_4GB_)
    ASSERT (PhysicalAddress.HighPart == 0);
#endif

    ASSERT (NumberOfBytes != 0);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (PhysicalAddress.LowPart,
                                                    NumberOfBytes);

    //
    // See if the first frame is in the PFN database and if so, they all must
    // be.  Note since the caller is mapping the frames, they must already be
    // locked so the PFN lock is not needed to protect against a hot-remove.
    //

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    IoMapping = (CSHORT) (!MI_IS_PFN (PageFrameIndex));

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    if (IoMapping) {

        //
        // If the size and start address are an exact multiple of the
        // minimum large page size, try to use large pages to map the request.
        //

        if (((PhysicalAddress.LowPart % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0) &&
            ((NumberOfBytes % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0)) {

            BaseVa = MiMapWithLargePages (PageFrameIndex,
                                          NumberOfPages,
                                          MM_EXECUTE_READWRITE,
                                          CacheType);

            if (BaseVa != NULL) {
                goto Done;
            }
        }

        Pfn1 = NULL;
    }
    else {
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    }

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {
        return NULL;
    }

    BaseVa = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    if (Pfn1 == NULL) {
        CacheAttribute = MiInsertIoSpaceMap (BaseVa,
                                             PageFrameIndex,
                                             NumberOfPages,
                                             CacheAttribute);

        if (CacheAttribute == MiNotMapped) { 
            MiReleaseSystemPtes (PointerPte, (ULONG) NumberOfPages, SystemPteSpace);
            return NULL;
        }
    }

    if (CacheAttribute != MiCached) {

        //
        // If a noncachable mapping is requested, none of the pages in the
        // requested MDL can reside in a large page.  Otherwise we would be
        // creating an incoherent overlapping TB entry as the same physical
        // page would be mapped by 2 different TB entries with different
        // cache attributes.
        //

        LastPageFrameIndex = PageFrameIndex + NumberOfPages;

        LOCK_PFN2 (OldIrql);

        do {

            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                UNLOCK_PFN2 (OldIrql);
                MiNonCachedCollisions += 1;
                if (Pfn1 == NULL) {
                    MiRemoveIoSpaceMap (BaseVa, NumberOfPages);
                }
                MiReleaseSystemPtes (PointerPte,
                                     (ULONG) NumberOfPages,
                                     SystemPteSpace);
                return NULL;
            }

            PageFrameIndex += 1;

        } while (PageFrameIndex < LastPageFrameIndex);

        UNLOCK_PFN2 (OldIrql);

        MI_PREPARE_FOR_NONCACHED (CacheAttribute);
    }

    BaseVa = (PVOID)((PCHAR)BaseVa + BYTE_OFFSET(PhysicalAddress.LowPart));

    TempPte = ValidKernelPte;

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

#if defined(_X86_)

    //
    // Set the physical range to the proper caching type.  If the PAT feature
    // is supported, then we just use the caching type in the PTE.  Otherwise
    // modify the MTRRs if applicable.
    //
    // Note if the cache request is for cached or noncached, don't waste
    // an MTRR on this range because the PTEs can be encoded to provide
    // equivalent functionality.
    //

    if ((MiWriteCombiningPtes == FALSE) && (CacheAttribute == MiWriteCombined)) {

        //
        // If the address is an I/O space address, use MTRRs if possible.
        //

        NTSTATUS Status;

        //
        // If the address is a memory address, don't risk using MTRRs because
        // other pages in the range are likely mapped with differing attributes
        // in the TB and we must not add a conflicting range.
        //

        if (Pfn1 != NULL) {
            MiReleaseSystemPtes(PointerPte, NumberOfPages, SystemPteSpace);
            return NULL;
        }

        //
        // Since the attribute may have been overridden (due to a collision
        // with a prior exiting mapping), make sure the CacheType is also
        // consistent before editing the MTRRs.
        //

        CacheType = MmWriteCombined;

        Status = KeSetPhysicalCacheTypeRange (PhysicalAddress,
                                              NumberOfBytes,
                                              CacheType);

        if (!NT_SUCCESS(Status)) {

            //
            // There's still a problem, fail the request.
            //

            ASSERT (Pfn1 == NULL);
            MiRemoveIoSpaceMap (BaseVa, NumberOfPages);
            MiReleaseSystemPtes(PointerPte, NumberOfPages, SystemPteSpace);
            return NULL;
        }

        //
        // Override the write combine (weak UC) bits in the PTE and
        // instead use a cached attribute.  This is because the processor
        // will use the least cachable (ie: functionally safer) attribute
        // of the PTE & MTRR to use - so specifying fully cached for the PTE
        // ensures that the MTRR value will win out.
        //

        TempPte = ValidKernelPte;
    }
#endif

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
    ASSERT ((Pfn1 == MI_PFN_ELEMENT (PageFrameIndex)) || (Pfn1 == NULL));

    OldIrql = HIGH_LEVEL;

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);
        if (Pfn1 != NULL) {

            ASSERT ((Pfn1->u3.e2.ReferenceCount != 0) ||
                    ((Pfn1->u3.e1.Rom == 1) && (CacheType == MmCached)));

            TempPte = ValidKernelPte;

            MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

            switch (Pfn1->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {

                        //
                        // The caller asked for a noncached or writecombined
                        // mapping, but the page is already mapped cached by
                        // someone else.  Override the caller's request in
                        // order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[0] += 1;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {

                        //
                        // The caller asked for a cached or writecombined
                        // mapping, but the page is already mapped noncached
                        // by someone else.  Override the caller's request
                        // in order to keep the TB page attribute coherent.
                        //

                        MiCacheOverride[1] += 1;
                    }
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {

                        //
                        // The caller asked for a cached or noncached
                        // mapping, but the page is already mapped
                        // writecombined by someone else.  Override the
                        // caller's request in order to keep the TB page
                        // attribute coherent.
                        //

                        MiCacheOverride[2] += 1;
                    }
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                case MiNotMapped:

                    //
                    // This better be for a page allocated with
                    // MmAllocatePagesForMdl.  Otherwise it might be a
                    // page on the freelist which could subsequently be
                    // given out with a different attribute !
                    //

#if defined (_MI_MORE_THAN_4GB_)
                    ASSERT ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn1->u4.PteFrame == MI_MAGIC_4GB_RECLAIM) ||
                            (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
#else
                    ASSERT ((Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                            (Pfn1->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
#endif

                    if (OldIrql == HIGH_LEVEL) {
                        LOCK_PFN2 (OldIrql);
                    }

                    switch (CacheAttribute) {

                        case MiCached:
                            Pfn1->u3.e1.CacheAttribute = MiCached;
                            break;

                        case MiNonCached:
                            Pfn1->u3.e1.CacheAttribute = MiNonCached;
                            MI_DISABLE_CACHING (TempPte);
                            break;

                        case MiWriteCombined:
                            Pfn1->u3.e1.CacheAttribute = MiWriteCombined;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;

                        default:
                            ASSERT (FALSE);
                            break;
                    }
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
            Pfn1 += 1;
        }
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
        PageFrameIndex += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    if (OldIrql != HIGH_LEVEL) {
        UNLOCK_PFN2 (OldIrql);
    }

Done:

    if (MmTrackPtes & 0x1) {

        RtlGetCallersAddress (&CallingAddress, &CallersCaller);

        TempMdl = (PMDL) MdlHack;
        TempMdl->MappedSystemVa = BaseVa;

        PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);
        *(PPFN_NUMBER)(TempMdl + 1) = PageFrameIndex;

        TempMdl->StartVa = (PVOID)(PAGE_ALIGN((ULONG_PTR)PhysicalAddress.QuadPart));
        TempMdl->ByteOffset = BYTE_OFFSET(PhysicalAddress.LowPart);
        TempMdl->ByteCount = (ULONG)NumberOfBytes;
    
        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

        MiInsertPteTracker (TempMdl,
                            1,
                            IoMapping,
                            CacheAttribute,
                            CallingAddress,
                            CallersCaller);
    }
    
    return BaseVa;
}

VOID
MmUnmapIoSpace (
    __in_bcount(NumberOfBytes) PVOID BaseAddress,
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function unmaps a range of physical addresses which were previously
    mapped via an MmMapIoSpace function call.

Arguments:

    BaseAddress - Supplies the base virtual address where the physical
                  address was previously mapped.

    NumberOfBytes - Supplies the number of bytes which were mapped.

Return Value:

    None.

Environment:

    Kernel mode, Should be IRQL of APC_LEVEL or below, but unfortunately
    callers are coming in at DISPATCH_LEVEL and it's too late to change the
    rules now.  This means you can never make this routine pageable.

--*/

{
    ULONG i;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];

    ASSERT (NumberOfBytes != 0);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress, NumberOfBytes);

    if (MmTrackPtes & 0x1) {
        MiRemovePteTracker (NULL, BaseAddress, NumberOfPages);
    }

    PointerPde = MiGetPdeAddress (BaseAddress);

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPde) == 0) {

        PointerPte = MiGetPteAddress (BaseAddress);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        if (!MI_IS_PFN (PageFrameIndex)) {

            //
            // The PTEs must be invalidated and the TB flushed *BEFORE*
            // removing the I/O space map.  This is because another
            // thread can immediately map the same I/O space with another
            // set of PTEs (and conflicting TB attributes) before we
            // call MiReleaseSystemPtes.
            //

            MiZeroMemoryPte (PointerPte, NumberOfPages);

            if (NumberOfPages == 1) {
                MI_FLUSH_SINGLE_TB (BaseAddress, TRUE);
            }
            else if (NumberOfPages < MM_MAXIMUM_FLUSH_COUNT) {

                for (i = 0; i < NumberOfPages; i += 1) {
                    VaFlushList[i] = BaseAddress;
                    BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
                }

                MI_FLUSH_MULTIPLE_TB ((ULONG)NumberOfPages, &VaFlushList[0], TRUE);
                BaseAddress = (PVOID)((PCHAR)BaseAddress - (i * PAGE_SIZE));
            }
            else {
                MI_FLUSH_ENTIRE_TB (5);
            }

            MiRemoveIoSpaceMap (BaseAddress, NumberOfPages);
        }

        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
    }
    else {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) +
                                   MiGetPteOffset (BaseAddress);

        if (!MI_IS_PFN (PageFrameIndex)) {
            MiRemoveIoSpaceMap (BaseAddress, NumberOfPages);
        }

        //
        // There is a race here because the I/O space mapping entry has been
        // removed but the TB has not yet been flushed (nor have the PDEs
        // been invalidated).  Another thread could request a map in between
        // the above removal and the below invalidation.  If this map occurs
        // where a driver provides the wrong page attribute it will not be
        // detected.  This is not worth closing as it is a driver bug anyway
        // and you really can't always stop them from hurting themselves if
        // they are determined to do so.  Note the alternative of invalidating
        // first isn't attractive because then the same PTEs could be
        // immediately reallocated and the new owner might want to add an
        // I/O space entry before this thread got to remove his.  So additional
        // lock serialization would need to be added.  Not worth it.
        //

        MiUnmapLargePages (BaseAddress, NumberOfBytes);
    }

    return;
}

PVOID
MiAllocateContiguousMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MEMORY_CACHING_TYPE CacheType,
    PVOID CallingAddress
    )

/*++

Routine Description:

    This function allocates a range of physically contiguous non-paged
    pool.  It relies on the fact that non-paged pool is built at
    system initialization time from a contiguous range of physical
    memory.  It allocates the specified size of non-paged pool and
    then checks to ensure it is contiguous as pool expansion does
    not maintain the contiguous nature of non-paged pool.

    This routine is designed to be used by a driver's initialization
    routine to allocate a contiguous block of physical memory for
    issuing DMA requests from.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

    LowestAcceptablePfn - Supplies the lowest page frame number
                          which is valid for the allocation.

    HighestAcceptablePfn - Supplies the highest page frame number
                           which is valid for the allocation.

    BoundaryPfn - Supplies the page frame number multiple the allocation must
                  not cross.  0 indicates it can cross any boundary.

    CacheType - Supplies the type of cache mapping that will be used for the
                memory.

    CallingAddress - Supplies the calling address of the allocator.

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PVOID BaseAddress;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER LowestPfn;
    PFN_NUMBER HighestPfn;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (NumberOfBytes != 0);

    LowestPfn = LowestAcceptablePfn;

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        if (HighestAcceptablePfn < MiNoLowMemory) {

            return MiAllocateLowMemory (NumberOfBytes,
                                        LowestAcceptablePfn,
                                        HighestAcceptablePfn,
                                        BoundaryPfn,
                                        CallingAddress,
                                        CacheType,
                                        'tnoC');
        }
        LowestPfn = MiNoLowMemory;
    }
#endif

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    //
    // N.B. This setting of SizeInPages to exactly the request size
    // means the non-NULL return value from MiCheckForContiguousMemory
    // is guaranteed to be the BaseAddress.  If this size is ever
    // changed, then the non-NULL return value must be checked and
    // split/returned accordingly.
    //

    SizeInPages = BYTES_TO_PAGES (NumberOfBytes);
    HighestPfn = HighestAcceptablePfn;

    if (CacheAttribute == MiCached) {

        BaseAddress = ExAllocatePoolWithTag (NonPagedPoolCacheAligned,
                                             NumberOfBytes,
                                             'mCmM');

        if (BaseAddress != NULL) {

            if (MiCheckForContiguousMemory (BaseAddress,
                                            SizeInPages,
                                            SizeInPages,
                                            LowestPfn,
                                            HighestPfn,
                                            BoundaryPfn,
                                            CacheAttribute)) {

                return BaseAddress;
            }

            //
            // The allocation from pool does not meet the contiguous
            // requirements.  Free the allocation and see if any of
            // the free pool pages do.
            //

            ExFreePool (BaseAddress);
        }
    }

    if (KeGetCurrentIrql() > APC_LEVEL) {
        return NULL;
    }

    BaseAddress = MiFindContiguousMemory (LowestPfn,
                                          HighestPfn,
                                          BoundaryPfn,
                                          SizeInPages,
                                          CacheType,
                                          CallingAddress);

    return BaseAddress;
}


__bcount(NumberOfBytes) PVOID
MmAllocateContiguousMemorySpecifyCache (
    __in SIZE_T NumberOfBytes,
    __in PHYSICAL_ADDRESS LowestAcceptableAddress,
    __in PHYSICAL_ADDRESS HighestAcceptableAddress,
    __in PHYSICAL_ADDRESS BoundaryAddressMultiple,
    __in MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function allocates a range of physically contiguous non-cached,
    non-paged memory.  This is accomplished by using MmAllocateContiguousMemory
    which uses nonpaged pool virtual addresses to map the found memory chunk.

    Then this function establishes another map to the same physical addresses,
    but this alternate map is initialized as non-cached.  All references by
    our caller will be done through this alternate map.

    This routine is designed to be used by a driver's initialization
    routine to allocate a contiguous block of noncached physical memory for
    things like the AGP GART.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

    LowestAcceptableAddress - Supplies the lowest physical address
                              which is valid for the allocation.  For
                              example, if the device can only reference
                              physical memory in the 8M to 16MB range, this
                              value would be set to 0x800000 (8Mb).

    HighestAcceptableAddress - Supplies the highest physical address
                               which is valid for the allocation.  For
                               example, if the device can only reference
                               physical memory below 16MB, this
                               value would be set to 0xFFFFFF (16Mb - 1).

    BoundaryAddressMultiple - Supplies the physical address multiple this
                              allocation must not cross.

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PVOID BaseAddress;
    PFN_NUMBER LowestPfn;
    PFN_NUMBER HighestPfn;
    PFN_NUMBER BoundaryPfn;
    PVOID CallingAddress;
    PVOID CallersCaller;

    RtlGetCallersAddress (&CallingAddress, &CallersCaller);

    ASSERT (NumberOfBytes != 0);

    LowestPfn = (PFN_NUMBER)(LowestAcceptableAddress.QuadPart >> PAGE_SHIFT);
    if (BYTE_OFFSET(LowestAcceptableAddress.LowPart)) {
        LowestPfn += 1;
    }

    if (BYTE_OFFSET(BoundaryAddressMultiple.LowPart)) {
        return NULL;
    }

    BoundaryPfn = (PFN_NUMBER)(BoundaryAddressMultiple.QuadPart >> PAGE_SHIFT);

    HighestPfn = (PFN_NUMBER)(HighestAcceptableAddress.QuadPart >> PAGE_SHIFT);

    if (HighestPfn > MmHighestPossiblePhysicalPage) {
        HighestPfn = MmHighestPossiblePhysicalPage;
    }

    if (LowestPfn > HighestPfn) {

        //
        // The caller's range is beyond what physically exists, it cannot
        // succeed.  Bail now to avoid an expensive fruitless search.
        //

        return NULL;
    }

    BaseAddress = MiAllocateContiguousMemory (NumberOfBytes,
                                              LowestPfn,
                                              HighestPfn,
                                              BoundaryPfn,
                                              CacheType,
                                              CallingAddress);

    return BaseAddress;
}

__bcount(NumberOfBytes) PVOID
MmAllocateContiguousMemory (
    __in SIZE_T NumberOfBytes,
    __in PHYSICAL_ADDRESS HighestAcceptableAddress
    )

/*++

Routine Description:

    This function allocates a range of physically contiguous non-paged pool.

    This routine is designed to be used by a driver's initialization
    routine to allocate a contiguous block of physical memory for
    issuing DMA requests from.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

    HighestAcceptableAddress - Supplies the highest physical address
                               which is valid for the allocation.  For
                               example, if the device can only reference
                               physical memory in the lower 16MB this
                               value would be set to 0xFFFFFF (16Mb - 1).

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PFN_NUMBER HighestPfn;
    PVOID CallingAddress;
    PVOID VirtualAddress;
    PVOID CallersCaller;

    RtlGetCallersAddress (&CallingAddress, &CallersCaller);

    HighestPfn = (PFN_NUMBER)(HighestAcceptableAddress.QuadPart >> PAGE_SHIFT);

    if (HighestPfn > MmHighestPossiblePhysicalPage) {
        HighestPfn = MmHighestPossiblePhysicalPage;
    }

    VirtualAddress = MiAllocateContiguousMemory (NumberOfBytes,
                                                 0,
                                                 HighestPfn,
                                                 0,
                                                 MmCached,
                                                 CallingAddress);
            
    return VirtualAddress;
}

#if defined (_WIN64)
#define SPECIAL_POOL_ADDRESS(p) \
        ((((p) >= MmSpecialPoolStart) && ((p) < MmSpecialPoolEnd)) || \
        (((p) >= MmSessionSpecialPoolStart) && ((p) < MmSessionSpecialPoolEnd)))
#else
#define SPECIAL_POOL_ADDRESS(p) \
        (((p) >= MmSpecialPoolStart) && ((p) < MmSpecialPoolEnd))
#endif


VOID
MmFreeContiguousMemory (
    __in PVOID BaseAddress
    )

/*++

Routine Description:

    This function deallocates a range of physically contiguous non-paged
    pool which was allocated with the MmAllocateContiguousMemory function.

Arguments:

    BaseAddress - Supplies the base virtual address where the physical
                  address was previously mapped.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    ULONG SizeInPages;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPage;
    PMMPFN Pfn1;
    PMMPFN StartPfn;

    PAGED_CODE();

#if defined (_MI_MORE_THAN_4GB_)
    if (MiNoLowMemory != 0) {
        if (MiFreeLowMemory (BaseAddress, 'tnoC') == TRUE) {
            return;
        }
    }
#endif

    if (((BaseAddress >= MmNonPagedPoolStart) &&
        (BaseAddress < (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) ||

        ((BaseAddress >= MmNonPagedPoolExpansionStart) &&
        (BaseAddress < MmNonPagedPoolEnd)) ||

        (SPECIAL_POOL_ADDRESS(BaseAddress))) {

        ExFreePool (BaseAddress);
    }
    else {

        //
        // The contiguous memory being freed may be the target of a delayed
        // unlock.  Since these pages may be immediately released, force
        // any pending delayed actions to occur now.
        //

        MiDeferredUnlockPages (0);

        PointerPte = MiGetPteAddress (BaseAddress);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x60,
                          (ULONG_PTR)BaseAddress,
                          0,
                          0);
        }

        StartPfn = Pfn1;
        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1 -= 1;

        do {
            Pfn1 += 1;
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            ASSERT (Pfn1->u2.ShareCount == 1);
            ASSERT (Pfn1->PteAddress == PointerPte);
            ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
            ASSERT (Pfn1->u4.PteFrame == MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte)));
            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
            ASSERT (Pfn1->u4.VerifierAllocation == 0);
            ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
            ASSERT (Pfn1->u3.e1.PrototypePte == 0);
            MI_SET_PFN_DELETED(Pfn1);
            PointerPte += 1;

        } while (Pfn1->u3.e1.EndOfAllocation == 0);

        Pfn1->u3.e1.EndOfAllocation = 0;

        SizeInPages = (ULONG)(Pfn1 - StartPfn + 1);

        //
        // Notify deadlock verifier that a region that can contain locks
        // will become invalid.
        //

        if (MmVerifierData.Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) {
            VerifierDeadlockFreePool (BaseAddress, SizeInPages << PAGE_SHIFT);
        }

        //
        // Release the mapping.
        //

        MmUnmapIoSpace (BaseAddress, SizeInPages << PAGE_SHIFT);

        //
        // Release the actual pages.
        //

        LastPage = PageFrameIndex + SizeInPages;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        do {
            MiDecrementShareCount (Pfn1, PageFrameIndex);
            PageFrameIndex += 1;
            Pfn1 += 1;
        } while (PageFrameIndex < LastPage);

        UNLOCK_PFN (OldIrql);

        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS2);

        MiReturnCommitment (SizeInPages);
    }
}


VOID
MmFreeContiguousMemorySpecifyCache (
    __in_bcount(NumberOfBytes) PVOID BaseAddress,
    __in SIZE_T NumberOfBytes,
    __in MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function deallocates a range of noncached memory in
    the non-paged portion of the system address space.

Arguments:

    BaseAddress - Supplies the base virtual address where the noncached

    NumberOfBytes - Supplies the number of bytes allocated to the request.
                    This must be the same number that was obtained with
                    the MmAllocateContiguousMemorySpecifyCache call.

    CacheType - Supplies the cachetype used when the caller made the
                MmAllocateContiguousMemorySpecifyCache call.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    UNREFERENCED_PARAMETER (NumberOfBytes);
    UNREFERENCED_PARAMETER (CacheType);

    MmFreeContiguousMemory (BaseAddress);
}



PVOID
MmAllocateIndependentPages (
    IN SIZE_T NumberOfBytes,
    IN ULONG Node
    )

/*++

Routine Description:

    This function allocates a range of virtually contiguous nonpaged pages
    without using superpages.  This allows the caller to apply independent
    page protections to each page.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

    Node - Supplies the preferred node number for the backing physical pages.
           If pages on the preferred node are not available, any page will
           be used.  -1 indicates no preferred node.

Return Value:

    The virtual address of the memory or NULL if none could be allocated.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    ULONG PageColor;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID BaseAddress;
    KIRQL OldIrql;

    ASSERT ((Node == (ULONG)-1) || (Node < KeNumberNodes));

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages, SystemPteSpace);

    if (PointerPte == NULL) {
        return NULL;
    }

    if (MiChargeCommitment (NumberOfPages, NULL) == FALSE) {
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
        return NULL;
    }

    BaseAddress = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    MI_MAKE_VALID_KERNEL_PTE (TempPte,
                              0,
                              MM_READWRITE,
                              PointerPte);

    MI_SET_PTE_DIRTY (TempPte);

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)NumberOfPages > MI_NONPAGEABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        MiReturnCommitment (NumberOfPages);
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
        return NULL;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_INDEPENDENT_PAGES, NumberOfPages);

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_ALLOCATE_INDEPENDENT);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, OldIrql);
        }

        if (Node == (ULONG)-1) {
            PageColor = MI_GET_PAGE_COLOR_FROM_PTE (PointerPte);
        }
        else {
            PageColor = (((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask) |
                           (Node << MmSecondaryColorNodeShift));
        }

        PageFrameIndex = MiRemoveAnyPage (PageColor);

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPte, TempPte);

        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    UNLOCK_PFN (OldIrql);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    return BaseAddress;
}

BOOLEAN
MmSetPageProtection (
    __in_bcount(NumberOfBytes) PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes,
    __in ULONG NewProtect
    )

/*++

Routine Description:

    This function sets the specified virtual address range to the desired
    protection.  This assumes that the virtual addresses are backed by PTEs
    which can be set (ie: not in kseg0 or large pages).

Arguments:

    VirtualAddress - Supplies the start address to protect.

    NumberOfBytes - Supplies the number of bytes to set.

    NewProtect - Supplies the protection to set the pages to (PAGE_XX).

Return Value:

    TRUE if the protection was applied, FALSE if not.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE NewPteContents;
    KIRQL OldIrql;
    ULONG ProtectionMask;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {
        return FALSE;
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return FALSE;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);
    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);
    ASSERT (NumberOfPages != 0);

    MI_MAKE_VALID_KERNEL_PTE (NewPteContents,
                              0,
                              ProtectionMask,
                              PointerPte);

    LOCK_PFN (OldIrql);

    for (i = 0; i < NumberOfPages; i += 1) {

        TempPte = *PointerPte;

        NewPteContents.u.Hard.PageFrameNumber = TempPte.u.Hard.PageFrameNumber;
        NewPteContents.u.Hard.Dirty = TempPte.u.Hard.Dirty;

        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, NewPteContents);

        PointerPte += 1;
    }

    if (NumberOfPages == 1) {
        MI_FLUSH_SINGLE_TB (VirtualAddress, TRUE);
    }
    else if (NumberOfPages < MM_MAXIMUM_FLUSH_COUNT) {

        for (i = 0; i < NumberOfPages; i += 1) {
            VaFlushList[i] = (PVOID)((PUCHAR)VirtualAddress + (i << PAGE_SHIFT));
        }
        MI_FLUSH_MULTIPLE_TB ((ULONG)NumberOfPages, &VaFlushList[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (0x13);
    }

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

VOID
MmFreeIndependentPages (
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    Returns pages previously allocated with MmAllocateIndependentPages.

Arguments:

    VirtualAddress - Supplies the virtual address to free.

    NumberOfBytes - Supplies the number of bytes to free.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE BasePte;
    PMMPTE EndPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    PointerPte = MiGetPteAddress (VirtualAddress);
    BasePte = PointerPte;
    EndPte = PointerPte + NumberOfPages;

    LOCK_PFN (OldIrql);

    do {

        PteContents = *PointerPte;

        ASSERT (PteContents.u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);

        PointerPte += 1;

    } while (PointerPte < EndPte);

    //
    // Update the count of resident available pages.
    //

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPages, MM_RESAVAIL_FREE_INDEPENDENT);

    //
    // Return PTEs and commitment.
    //

    MiReleaseSystemPtes (BasePte, (ULONG)NumberOfPages, SystemPteSpace);

    MiReturnCommitment (NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_INDEPENDENT_PAGES, NumberOfPages);
}


VOID
MiZeroAwePageWorker (
    IN PVOID Context
    )

/*++

Routine Description:

    This routine is the worker routine executed by all processors to
    fan out page zeroing for AWE allocations.

Arguments:

    Context - Supplies a pointer to the workitem.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
#if defined(MI_MULTINODE) 
    LOGICAL SetIdeal;
    ULONG Processor;
    PEPROCESS DefaultProcess;
#endif
    PKTHREAD Thread;
    KPRIORITY OldPriority;
    PMMPFN Pfn1;
    SCHAR OldBasePriority;
    PMMPFN PfnNextColored;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    MMPTE TempPte;
    MMPTE TempCachedPte;
    PMMPTE BasePte;
    PMMPTE PointerPte;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    PFN_COUNT i;
    PFN_COUNT RequestedPtes;

    ColoredPageInfo = (PCOLORED_PAGE_INFO) Context;

    //
    // Use the initiating thread's priority instead of the default system
    // thread priority.
    //

    Thread = KeGetCurrentThread ();
    OldBasePriority = Thread->BasePriority;
    Thread->BasePriority = ColoredPageInfo->BasePriority;
    OldPriority = KeSetPriorityThread (Thread, Thread->BasePriority);

    //
    // Dispatch each worker thread to a processor local to the memory being
    // zeroed.
    //

#if defined(MI_MULTINODE) 
    Processor = 0;

    if (PsInitialSystemProcess != NULL) {
        DefaultProcess = PsInitialSystemProcess;
    }
    else {
        DefaultProcess = PsIdleProcess;
    }

    if ((ColoredPageInfo->Affinity != 0) &&
        (ColoredPageInfo->Affinity != DefaultProcess->Pcb.Affinity)) {

        KeFindFirstSetLeftAffinity (ColoredPageInfo->Affinity, &Processor);

        //
        // Only affinitize if the node has a processor.
        //

        if (Processor != NO_BITS_FOUND) {
            Processor = (CCHAR) KeSetIdealProcessorThread (Thread,
                                                           (CCHAR) Processor);
            SetIdeal = TRUE;
        }
        else {
            SetIdeal = FALSE;
        }
    }
    else {
        SetIdeal = FALSE;
    }
#endif

    Pfn1 = ColoredPageInfo->PfnAllocation;

    ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);
    ASSERT (ColoredPageInfo->PagesQueued != 0);

    //
    // Zero all argument pages.
    //

    do {

        ASSERT (ColoredPageInfo->PagesQueued != 0);

        RequestedPtes = ColoredPageInfo->PagesQueued;

#if !defined (_WIN64)

        //
        // NT64 has an abundance of PTEs so try to map the whole request with
        // a single call.  For NT32, this resource needs to be carefully shared.
        //

        if (RequestedPtes > (1024 * 1024) / PAGE_SIZE) {
            RequestedPtes = (1024 * 1024) / PAGE_SIZE;
        }
#endif

        do {
            BasePte = MiReserveSystemPtes (RequestedPtes, SystemPteSpace);

            if (BasePte != NULL) {
                break;
            }

            RequestedPtes >>= 1;

        } while (RequestedPtes != 0);

        if (BasePte != NULL) {

            //
            // Able to get a reasonable chunk, go for a big zero.
            //

            PointerPte = BasePte;

            MI_MAKE_VALID_KERNEL_PTE (TempCachedPte,
                                      0,
                                      MM_READWRITE,
                                      PointerPte);

            MI_SET_PTE_DIRTY (TempCachedPte);

            for (i = 0; i < RequestedPtes; i += 1) {

                ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

                PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                ASSERT (PointerPte->u.Hard.Valid == 0);

                TempPte = TempCachedPte;

                switch (Pfn1->u3.e1.CacheAttribute) {
                    case MiCached:
                        break;

                    case MiNonCached:
                        MI_DISABLE_CACHING (TempPte);
                        break;

                    case MiWriteCombined:
                        MI_SET_PTE_WRITE_COMBINE (TempPte);
                        break;

                    default:
                        break;
                }

                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                PfnNextColored = (PMMPFN) (ULONG_PTR) Pfn1->OriginalPte.u.Long;
                Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                Pfn1 = PfnNextColored;

                PointerPte += 1;
            }

            ColoredPageInfo->PagesQueued -= RequestedPtes;

            VirtualAddress = MiGetVirtualAddressMappedByPte (BasePte);

            KeZeroPages (VirtualAddress, ((ULONG_PTR)RequestedPtes) << PAGE_SHIFT);

            MiReleaseSystemPtes (BasePte, RequestedPtes, SystemPteSpace);
        }
        else {

            //
            // No PTEs left, zero a single page at a time.
            //

            MiZeroPhysicalPage (MI_PFN_ELEMENT_TO_INDEX (Pfn1));

            PfnNextColored = (PMMPFN) (ULONG_PTR) Pfn1->OriginalPte.u.Long;
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
            Pfn1 = PfnNextColored;

            ColoredPageInfo->PagesQueued -= 1;
        }

    } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

    //
    // Let the initiator know we've zeroed our share.
    //

    KeSetEvent (&ColoredPageInfo->Event, 0, FALSE);

    //
    // Restore the entry thread priority and ideal processor - this is critical
    // if we were called directly from the initiator instead of in the
    // context of a system thread.
    //

#if defined(MI_MULTINODE) 
    if (SetIdeal == TRUE) {
        KeSetIdealProcessorThread (Thread, (CCHAR) Processor);
    }
#endif

    KeSetPriorityThread (Thread, OldPriority);
    Thread->BasePriority = OldBasePriority;

    return;
}

VOID
MiZeroInParallel (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase
    )

/*++

Routine Description:

    This routine zeroes all the free & standby pages, fanning out the work.
    This is done even on UP machines because the worker thread code maps
    large MDLs and is thus better performing than zeroing a single
    page at a time.

Arguments:

    ColoredPageInfoBase - Supplies the informational structure about which
                          pages to zero.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.  The caller must lock down the
    PAGELK section that this routine resides in.

--*/

{
#if defined(MI_MULTINODE) 
    ULONG i;
#endif
    ULONG WaitCount;
    PKEVENT WaitObjects[MAXIMUM_WAIT_OBJECTS];
    KWAIT_BLOCK WaitBlockArray[MAXIMUM_WAIT_OBJECTS];
    KPRIORITY OldPriority;
    SCHAR OldBasePriority;
    NTSTATUS WakeupStatus;
    PETHREAD EThread;
    PKTHREAD Thread;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ThreadHandle;
    SCHAR WorkerThreadPriority;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    ULONG Color;
    PMMPFN Pfn1;
    NTSTATUS Status;

    WaitCount = 0;

    EThread = PsGetCurrentThread ();
    Thread = &EThread->Tcb;

    //
    // Raise our priority to the highest non-realtime priority.  This
    // usually allows us to spawn all our worker threads without
    // interruption from them, but also doesn't starve the modified or
    // mapped page writers because we are going to access pageable code
    // and data below and during the spawning.
    //

    OldBasePriority = Thread->BasePriority;
    ASSERT ((OldBasePriority >= 1) || (InitializationPhase == 0));
    Thread->BasePriority = LOW_REALTIME_PRIORITY - 1;
    OldPriority = KeSetPriorityThread (Thread, LOW_REALTIME_PRIORITY - 1);
    WorkerThreadPriority = OldBasePriority;

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {

        ColoredPageInfo = &ColoredPageInfoBase[Color];

        Pfn1 = ColoredPageInfo->PfnAllocation;

        if (Pfn1 != (PMMPFN) MM_EMPTY_LIST) {

            //
            // Assume no memory penalty for non-local memory on this
            // machine so there's no need to run with restricted affinity.
            //

            ColoredPageInfo->BasePriority = WorkerThreadPriority;

#if defined(MI_MULTINODE) 

            if (PsInitialSystemProcess != NULL) {
                ColoredPageInfo->Affinity = PsInitialSystemProcess->Pcb.Affinity;
            }
            else {
                ColoredPageInfo->Affinity = PsIdleProcess->Pcb.Affinity;
            }

            for (i = 0; i < KeNumberNodes; i += 1) {

                if (KeNodeBlock[i]->Color == (Color >> MmSecondaryColorNodeShift)) {
                    ColoredPageInfo->Affinity = KeNodeBlock[i]->ProcessorMask;
                    break;
                }
            }

#endif

            KeInitializeEvent (&ColoredPageInfo->Event,
                               SynchronizationEvent,
                               FALSE);

            //
            // Don't spawn threads to zero the memory if we are a system
            // thread.  This is because this routine can be called by
            // drivers in the context of the segment dereference thread, so
            // referencing pageable memory here can cause a deadlock.
            //

            if ((IS_SYSTEM_THREAD (EThread)) || (InitializationPhase == 0)) {
                MiZeroAwePageWorker ((PVOID) ColoredPageInfo);
            }
            else {

                InitializeObjectAttributes (&ObjectAttributes,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL);

                //
                // We are creating a system thread for each memory
                // node instead of using the executive worker thread
                // pool.   This is because we want to run the threads
                // at a lower priority to keep the machine responsive
                // during all this zeroing.  And doing this to a worker
                // thread can cause a deadlock as various other
                // components (registry, etc) expect worker threads to be
                // available at a higher priority right away.
                //

                Status = PsCreateSystemThread (&ThreadHandle,
                                               THREAD_ALL_ACCESS,
                                               &ObjectAttributes,
                                               0L,
                                               NULL,
                                               MiZeroAwePageWorker,
                                               (PVOID) ColoredPageInfo);

                if (NT_SUCCESS(Status)) {
                    ZwClose (ThreadHandle);
                }
                else {
                    MiZeroAwePageWorker ((PVOID) ColoredPageInfo);
                }
            }

            WaitObjects[WaitCount] = &ColoredPageInfo->Event;

            WaitCount += 1;

            if (WaitCount == MAXIMUM_WAIT_OBJECTS) {

                //
                // Done issuing the first round of workitems,
                // lower priority & wait.
                //

                KeSetPriorityThread (Thread, OldPriority);
                Thread->BasePriority = OldBasePriority;

                WakeupStatus = KeWaitForMultipleObjects (WaitCount,
                                                         &WaitObjects[0],
                                                         WaitAll,
                                                         Executive,
                                                         KernelMode,
                                                         FALSE,
                                                         NULL,
                                                         &WaitBlockArray[0]);
                ASSERT (WakeupStatus == STATUS_SUCCESS);

                WaitCount = 0;

                Thread->BasePriority = LOW_REALTIME_PRIORITY - 1;
                KeSetPriorityThread (Thread, LOW_REALTIME_PRIORITY - 1);
            }
        }
    }

    //
    // Done issuing all the workitems, lower priority & wait.
    //

    KeSetPriorityThread (Thread, OldPriority);
    Thread->BasePriority = OldBasePriority;

    if (WaitCount != 0) {

        WakeupStatus = KeWaitForMultipleObjects (WaitCount,
                                                 &WaitObjects[0],
                                                 WaitAll,
                                                 Executive,
                                                 KernelMode,
                                                 FALSE,
                                                 NULL,
                                                 &WaitBlockArray[0]);

        ASSERT (WakeupStatus == STATUS_SUCCESS);
    }

    return;
}


PMDL
MiAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN ULONG Flags
    )

/*++

Routine Description:

    This routine searches the PFN database for free, zeroed or standby pages
    to satisfy the request.  This does not map the pages - it just allocates
    them and puts them into an MDL.  It is expected that our caller will
    map the MDL as needed.

    NOTE: this routine may return an MDL mapping a smaller number of bytes
    than the amount requested.  It is the caller's responsibility to check the
    MDL upon return for the size actually allocated.

    These pages comprise physical non-paged memory and are zero-filled.

    This routine is designed to be used by an AGP driver to obtain physical
    memory in a specified range since hardware may provide substantial
    performance wins depending on where the backing memory is allocated.

    Because the caller may use these pages for a noncached mapping, care is
    taken to never allocate any pages that reside in a large page (in order
    to prevent TB incoherency of the same page being mapped by multiple
    translations with different attributes).

Arguments:

    LowAddress - Supplies the low physical address of the first range that
                 the allocated pages can come from.

    HighAddress - Supplies the high physical address of the first range that
                  the allocated pages can come from.

    SkipBytes - Number of bytes to skip (from the Low Address) to get to the
                next physical address range that allocated pages can come from.

    TotalBytes - Supplies the number of bytes to allocate.

    Flags - Supplies flags passed by the caller.

Return Value:

    MDL - An MDL mapping a range of pages in the specified range.
          This may map less memory than the caller requested if the full amount
          is not currently available.

    NULL - No pages in the specified range OR not enough virtually contiguous
           nonpaged pool for the MDL is available at this time.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PKPRCB Prcb;
    PKTHREAD Thread;
    ULONG StartColor;
    PMDL MemoryDescriptorList;
    PMDL MemoryDescriptorList2;
    PMMPFN Pfn1;
    PMMPFN PfnNextColored;
    PMMPFN PfnNextFlink;
    PMMPFN PfnLastColored;
    KIRQL OldIrql;
    PFN_NUMBER start;
    PFN_NUMBER Page;
    PFN_NUMBER NextPage;
    PFN_NUMBER found;
    PFN_NUMBER BasePage;
    PFN_NUMBER LowPage;
    PFN_NUMBER HighPage;
    PFN_NUMBER SizeInPages;
    PFN_NUMBER MdlPageSpan;
    PFN_NUMBER SkipPages;
    PFN_NUMBER MaxPages;
    PFN_NUMBER PagesExamined;
    PPFN_NUMBER MdlPage;
    ULONG Color;
    PMMCOLOR_TABLES ColorHead;
    MMLISTS MemoryList;
    PFN_NUMBER LowPage1;
    PFN_NUMBER HighPage1;
    LOGICAL PagePlacementOk;
    PFN_NUMBER PageNextColored;
    PFN_NUMBER PageNextFlink;
    PFN_NUMBER PageLastColored;
    PMMPFNLIST ListHead;
    PCOLORED_PAGE_INFO ColoredPageInfoBase;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    ULONG ColorHeadsDrained;
    ULONG NodePassesLeft;
    ULONG ColorCount;
    ULONG BaseColor;
    ULONG CacheTypesFound;
    PFN_NUMBER ZeroCount;
#if DBG
    ULONG i;
    PPFN_NUMBER LastMdlPage;
    ULONG FinishedCount;
    PEPROCESS Process;
#endif

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

#if DBG
    if (Flags & MI_ALLOCATION_IS_AWE) {
        ASSERT ((Flags & ~MI_ALLOCATION_IS_AWE) == 0);
    }
#endif

    //
    // The skip increment must be a page-size multiple.
    //

    if (BYTE_OFFSET(SkipBytes.LowPart)) {
        return NULL;
    }

    LowPage = (PFN_NUMBER)(LowAddress.QuadPart >> PAGE_SHIFT);
    HighPage = (PFN_NUMBER)(HighAddress.QuadPart >> PAGE_SHIFT);

    if (HighPage > MmHighestPossiblePhysicalPage) {
        HighPage = MmHighestPossiblePhysicalPage;
    }

    //
    // Maximum allocation size is constrained by the MDL ByteCount field.
    //

    if (TotalBytes > (SIZE_T)((ULONG)(MAXULONG - PAGE_SIZE))) {
        TotalBytes = (SIZE_T)((ULONG)(MAXULONG - PAGE_SIZE));
    }

    SizeInPages = (PFN_NUMBER)ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes);

    SkipPages = (PFN_NUMBER)(SkipBytes.QuadPart >> PAGE_SHIFT);

    BasePage = LowPage;

    //
    // Check without the PFN lock as the actual number of pages to get will
    // be recalculated later while holding the lock.
    //

    MaxPages = MI_NONPAGEABLE_MEMORY_AVAILABLE() - 1024;

    if ((SPFN_NUMBER)MaxPages <= 0) {
        SizeInPages = 0;
    }
    else if (SizeInPages > MaxPages) {
        SizeInPages = MaxPages;
    }

    if (SizeInPages == 0) {
        return NULL;
    }

#if DBG
    if (SizeInPages < (PFN_NUMBER)ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes)) {
        if (MiPrintAwe != 0) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                "MiAllocatePagesForMdl1: unable to get %p pages, trying for %p instead\n",
                ADDRESS_AND_SIZE_TO_SPAN_PAGES(0, TotalBytes),
                SizeInPages);
        }
    }
#endif

    //
    // Allocate an MDL to return the pages in.
    //

    do {
        MemoryDescriptorList = MmCreateMdl (NULL,
                                            NULL,
                                            SizeInPages << PAGE_SHIFT);
    
        if (MemoryDescriptorList != NULL) {
            break;
        }
        SizeInPages -= (SizeInPages >> 4);
    } while (SizeInPages != 0);

    if (MemoryDescriptorList == NULL) {
        return NULL;
    }

    //
    // Ensure there is enough commit prior to allocating the pages.
    //

    if (MiChargeCommitment (SizeInPages, NULL) == FALSE) {
        ExFreePool (MemoryDescriptorList);
        return NULL;
    }

    //
    // Allocate a list of colored anchors.
    //

    ColoredPageInfoBase = (PCOLORED_PAGE_INFO) ExAllocatePoolWithTag (
                                NonPagedPool,
                                MmSecondaryColors * sizeof (COLORED_PAGE_INFO),
                                'ldmM');

    if (ColoredPageInfoBase == NULL) {
        ExFreePool (MemoryDescriptorList);
        MiReturnCommitment (SizeInPages);
        return NULL;
    }

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {
        ColoredPageInfoBase[Color].PfnAllocation = (PMMPFN) MM_EMPTY_LIST;
        ColoredPageInfoBase[Color].PagesQueued = 0;
    }

    MdlPageSpan = SizeInPages;

    //
    // Recalculate the total size while holding the PFN lock.
    //

    start = 0;
    found = 0;
    ZeroCount = 0;
    CacheTypesFound = 0;

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    MmLockPageableSectionByHandle (ExPageLockHandle);

    Thread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (Thread);
    MI_LOCK_DYNAMIC_MEMORY_SHARED ();

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    MaxPages = MI_NONPAGEABLE_MEMORY_AVAILABLE() - 1024;

    if ((SPFN_NUMBER)MaxPages <= 0) {
        SizeInPages = 0;
    }
    else if (SizeInPages > MaxPages) {
        SizeInPages = MaxPages;
    }

    //
    // Systems utilizing memory compression may have more pages on the zero,
    // free and standby lists than we want to give out.  Explicitly check
    // MmAvailablePages instead (and recheck whenever the PFN lock is released
    // and reacquired).
    //

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        if (MmAvailablePages > MM_HIGH_LIMIT) {
            SizeInPages = MmAvailablePages - MM_HIGH_LIMIT;
        }
        else {
            SizeInPages = 0;
        }
    }

    if (SizeInPages == 0) {
        UNLOCK_PFN (OldIrql);
        MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
        KeLeaveGuardedRegionThread (Thread);
        MmUnlockPageableImageSection (ExPageLockHandle);
        ExFreePool (MemoryDescriptorList);
        MiReturnCommitment (MdlPageSpan);
        ExFreePool (ColoredPageInfoBase);
        return NULL;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_MDL_PAGES, SizeInPages);

    //
    // Charge resident available pages now for all the pages so the PFN lock
    // can be released between the loops below.  Excess charging is returned
    // at the conclusion of the loops.
    //

    InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, SizeInPages);

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_FOR_MDL);

    //
    // Grab all zeroed (and then free) pages first directly from the
    // colored lists to avoid multiple walks down these singly linked lists.
    // Then snatch transition pages as needed.  In addition to optimizing
    // the speed of the removals this also avoids cannibalizing the page
    // cache unless it's absolutely needed.
    //

    NodePassesLeft = 1;
    ColorCount = MmSecondaryColors;
    BaseColor = 0;

    Prcb = KeGetCurrentPrcb ();
#if defined (NT_UP)
    StartColor = (MI_SYSTEM_PAGE_COLOR++);
#else
    StartColor = (Prcb->PageColor++);
#endif
    StartColor &= MmSecondaryColorMask;

#if defined(MI_MULTINODE) 

    if (KeNumberNodes > 1) {

        PKNODE Node;

        Node = KeGetCurrentNode ();

        if ((Node->FreeCount[ZeroedPageList]) ||
            (Node->FreeCount[FreePageList])) {

            //
            // There are available pages on this node.  Restrict search.
            //

            NodePassesLeft = 2;
            ColorCount = MmSecondaryColorMask + 1;
            BaseColor = Node->MmShiftedColor;
            ASSERT(ColorCount == MmSecondaryColors / KeNumberNodes);
        }
    }
    else {
        Flags &= ~MM_ALLOCATE_FROM_LOCAL_NODE_ONLY;
    }

    do {

        //
        // Loop: 1st pass restricted to node, 2nd pass unrestricted.
        //

#else
    Flags &= ~MM_ALLOCATE_FROM_LOCAL_NODE_ONLY;
#endif

        MemoryList = ZeroedPageList;

        do {

            //
            // Scan the zero list and then the free list.
            //

            ASSERT (MemoryList <= FreePageList);

            ListHead = MmPageLocationList[MemoryList];

            //
            // Initialize the loop iteration controls.  Clearly pages
            // can be added or removed from the colored lists when we
            // deliberately drop the PFN lock below (just to be a good
            // citizen), but even if we never released the lock, we wouldn't
            // have scanned more than the colorhead count anyway, so
            // this is a much better way to go.
            //

            ColorHeadsDrained = 0;
            PagesExamined = 0;

            ColorHead = &MmFreePagesByColor[MemoryList][BaseColor];
            ColoredPageInfo = &ColoredPageInfoBase[BaseColor];
            for (Color = 0; Color < ColorCount; Color += 1) {
                ASSERT (ColorHead->Count <= MmNumberOfPhysicalPages);
                ColoredPageInfo->PagesLeftToScan = ColorHead->Count;
                if (ColorHead->Count == 0) {
                    ColorHeadsDrained += 1;
                }
                ColorHead += 1;
                ColoredPageInfo += 1;
            }

            Color = BaseColor | StartColor;

            ASSERT (Color < MmSecondaryColors);
            do {

                //
                // Scan the current list by color.
                //

                ColorHead = &MmFreePagesByColor[MemoryList][Color];
                ColoredPageInfo = &ColoredPageInfoBase[Color];

                if (NodePassesLeft == 1) {

                    //
                    // Unrestricted search across all colors.
                    //

                    Color += 1;
                    if (Color >= MmSecondaryColors) {
                        Color = 0;
                    }
                }

#if defined(MI_MULTINODE) 

                else {

                    //
                    // Restrict first pass searches to current node.
                    //

                    ASSERT (NodePassesLeft == 2);
                    Color = BaseColor | ((Color + 1) & MmSecondaryColorMask);
                }

#endif

                if (ColoredPageInfo->PagesLeftToScan == 0) {

                    //
                    // This colored list has already been completely
                    // searched.
                    //

                    continue;
                }

                if (ColorHead->Flink == MM_EMPTY_LIST) {

                    //
                    // This colored list is empty.
                    //

                    ColoredPageInfo->PagesLeftToScan = 0;
                    ColorHeadsDrained += 1;
                    continue;
                }

                while (ColorHead->Flink != MM_EMPTY_LIST) {

                    ASSERT (ColoredPageInfo->PagesLeftToScan != 0);

                    ColoredPageInfo->PagesLeftToScan -= 1;

                    if (ColoredPageInfo->PagesLeftToScan == 0) {
                        ColorHeadsDrained += 1;
                    }

                    PagesExamined += 1;

                    Page = ColorHead->Flink;
    
                    Pfn1 = MI_PFN_ELEMENT(Page);

                    ASSERT ((MMLISTS)Pfn1->u3.e1.PageLocation == MemoryList);

                    //
                    // See if the page is within the caller's page constraints.
                    //

                    PagePlacementOk = FALSE;

                    //
                    // Since the caller may do anything with these frames
                    // including mapping them uncached or write combined,
                    // don't give out frames that are being mapped
                    // by (cached) superpages.
                    //

                    if ((CacheAttribute == MiCached) ||
                        (Pfn1->u4.MustBeCached == 0)) {

                        LowPage1 = LowPage;
                        HighPage1 = HighPage;

                        do {
                            if ((Page >= LowPage1) && (Page <= HighPage1)) {
                                PagePlacementOk = TRUE;
                                break;
                            }

                            if (SkipPages == 0) {
                                break;
                            }

                            LowPage1 += SkipPages;
                            HighPage1 += SkipPages;

                            if (HighPage1 > MmHighestPhysicalPage) {
                                HighPage1 = MmHighestPhysicalPage;
                            }

                        } while (LowPage1 <= MmHighestPhysicalPage);
                    }
            
                    // 
                    // The Flink and Blink must be nonzero here for the page
                    // to be on the listhead.  Only code that scans the
                    // MmPhysicalMemoryBlock has to check for the zero case.
                    //

                    ASSERT (Pfn1->u1.Flink != 0);
                    ASSERT (Pfn1->u2.Blink != 0);

                    if (PagePlacementOk == FALSE) {

                        if (ColoredPageInfo->PagesLeftToScan == 0) {

                            //
                            // No more pages to scan in this colored chain.
                            //

                            break;
                        }

                        //
                        // If the colored list has more than one entry then
                        // move this page to the end of this colored list.
                        //

                        PageNextColored = (PFN_NUMBER)Pfn1->OriginalPte.u.Long;

                        if (PageNextColored == MM_EMPTY_LIST) {

                            //
                            // No more pages in this colored chain.
                            //

                            ColoredPageInfo->PagesLeftToScan = 0;
                            ColorHeadsDrained += 1;
                            break;
                        }

                        ASSERT (Pfn1->u1.Flink != 0);
                        ASSERT (Pfn1->u1.Flink != MM_EMPTY_LIST);
                        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                        PfnNextColored = MI_PFN_ELEMENT(PageNextColored);
                        ASSERT ((MMLISTS)PfnNextColored->u3.e1.PageLocation == MemoryList);
                        ASSERT (PfnNextColored->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                        //
                        // Adjust the free page list so Page
                        // follows PageNextFlink.
                        //

                        PageNextFlink = Pfn1->u1.Flink;
                        PfnNextFlink = MI_PFN_ELEMENT(PageNextFlink);

                        ASSERT ((MMLISTS)PfnNextFlink->u3.e1.PageLocation == MemoryList);
                        ASSERT (PfnNextFlink->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

                        PfnLastColored = ColorHead->Blink;
                        ASSERT (PfnLastColored != (PMMPFN)MM_EMPTY_LIST);
                        ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                        ASSERT (PfnLastColored->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                        ASSERT (PfnLastColored->u2.Blink != MM_EMPTY_LIST);

                        ASSERT ((MMLISTS)PfnLastColored->u3.e1.PageLocation == MemoryList);
                        PageLastColored = MI_PFN_ELEMENT_TO_INDEX (PfnLastColored);

                        if (ListHead->Flink == Page) {

                            ASSERT (Pfn1->u2.Blink == MM_EMPTY_LIST);
                            ASSERT (ListHead->Blink != Page);

                            ListHead->Flink = PageNextFlink;

                            PfnNextFlink->u2.Blink = MM_EMPTY_LIST;
                        }
                        else {

                            ASSERT (Pfn1->u2.Blink != MM_EMPTY_LIST);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT((MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink)))->u3.e1.PageLocation == MemoryList);

                            MI_PFN_ELEMENT(Pfn1->u2.Blink)->u1.Flink = PageNextFlink;
                            PfnNextFlink->u2.Blink = Pfn1->u2.Blink;
                        }

#if DBG
                        if (PfnLastColored->u1.Flink == MM_EMPTY_LIST) {
                            ASSERT (ListHead->Blink == PageLastColored);
                        }
#endif

                        Pfn1->u1.Flink = PfnLastColored->u1.Flink;
                        Pfn1->u2.Blink = PageLastColored;

                        if (ListHead->Blink == PageLastColored) {
                            ListHead->Blink = Page;
                        }

                        //
                        // Adjust the colored chains.
                        //

                        if (PfnLastColored->u1.Flink != MM_EMPTY_LIST) {
                            ASSERT (MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                            ASSERT ((MMLISTS)(MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u3.e1.PageLocation) == MemoryList);
                            MI_PFN_ELEMENT(PfnLastColored->u1.Flink)->u2.Blink = Page;
                        }

                        PfnLastColored->u1.Flink = Page;

                        ColorHead->Flink = PageNextColored;
                        PfnNextColored->u4.PteFrame = MM_EMPTY_LIST;

                        Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;
                        Pfn1->u4.PteFrame = PageLastColored;

                        ASSERT (PfnLastColored->OriginalPte.u.Long == MM_EMPTY_LIST);
                        PfnLastColored->OriginalPte.u.Long = Page;
                        ColorHead->Blink = Pfn1;

                        continue;
                    }

                    found += 1;
                    ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
                    MiUnlinkFreeOrZeroedPage (Pfn1);

                    Pfn1->u3.e2.ReferenceCount = 1;
                    Pfn1->u2.ShareCount = 1;
                    MI_SET_PFN_DELETED(Pfn1);
                    Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
                    Pfn1->u3.e1.PageLocation = ActiveAndValid;

                    CacheTypesFound |= (1 << Pfn1->u3.e1.CacheAttribute);

                    Pfn1->u3.e1.CacheAttribute = CacheAttribute;

                    Pfn1->u3.e1.StartOfAllocation = 1;
                    Pfn1->u3.e1.EndOfAllocation = 1;
                    Pfn1->u4.VerifierAllocation = 0;
                    Pfn1->u3.e1.LargeSessionAllocation = 0;

                    //
                    // Add free pages to the list of pages to be
                    // zeroed before returning.
                    //

                    if ((MemoryList == FreePageList) &&
                        ((Flags & MM_DONT_ZERO_ALLOCATION) == 0)) {

                        Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
                        ColoredPageInfo->PfnAllocation = Pfn1;
                        ColoredPageInfo->PagesQueued += 1;
                        ZeroCount += 1;
                    }
                    else {
                        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                    }

                    *MdlPage = Page;
                    MdlPage += 1;

                    if (found == SizeInPages) {

                        //
                        // All the pages requested are available.
                        //

#if DBG
                        FinishedCount = 0;
                        for (i = 0; i < ColorCount; i += 1) {
                            if (ColoredPageInfoBase[i + BaseColor].PagesLeftToScan == 0) {
                                FinishedCount += 1;
                            }
                        }
                        ASSERT (FinishedCount == ColorHeadsDrained);
#endif

                        UNLOCK_PFN (OldIrql);
                        goto pass2_done;
                    }

                    //
                    // March on to the next colored chain so the overall
                    // allocation round-robins the page colors.
                    //

                    PagesExamined = PAGE_SIZE;
                    break;
                }

                //
                // If we've held the PFN lock for a while, release it to
                // give DPCs and other processors a chance to run.
                //

                if (PagesExamined >= PAGE_SIZE) {
                    UNLOCK_PFN (OldIrql);
                    PagesExamined = 0;
                    LOCK_PFN (OldIrql);
                }

                //
                // Systems utilizing memory compression may have more
                // pages on the zero, free and standby lists than we
                // want to give out.  The same is true when machines
                // are low on memory - we don't want this thread to gobble
                // up the pages from every modified write that completes
                // because that would starve waiting threads.
                //
                // Explicitly check MmAvailablePages instead (and recheck
                // whenever the PFN lock is released and reacquired).
                //

                if (MmAvailablePages < MM_HIGH_LIMIT) {
                    UNLOCK_PFN (OldIrql);
                    goto pass2_done;
                }

            } while (ColorHeadsDrained != ColorCount);

            //
            // Release the PFN lock to give DPCs and other processors
            // a chance to run.  Nothing magic about the instructions
            // between the unlock and the relock.
            //

            UNLOCK_PFN (OldIrql);

            //
            // Increment the start color so the next pass will keep walking
            // forwards through the color choices.
            //

            StartColor = Color + 1;
            StartColor &= MmSecondaryColorMask;

#if DBG
            FinishedCount = 0;
            for (i = 0; i < ColorCount; i += 1) {
                if (ColoredPageInfoBase[BaseColor + i].PagesLeftToScan == 0) {
                    FinishedCount += 1;
                }
            }
            ASSERT (FinishedCount == ColorHeadsDrained);
#endif

            MemoryList += 1;

            LOCK_PFN (OldIrql);

        } while (MemoryList <= FreePageList);

#if defined(MI_MULTINODE)

        if (Flags & MM_ALLOCATE_FROM_LOCAL_NODE_ONLY) {
            break;
        }

        //
        // Expand range to all colors for next pass.
        //

        ColorCount = MmSecondaryColors;
        BaseColor = 0;

        NodePassesLeft -= 1;

    } while (NodePassesLeft != 0);

#endif

    //
    // Briefly release the PFN lock to give DPCs and other processors
    // a time slice.
    //

    UNLOCK_PFN (OldIrql);

    if (Flags & MM_ALLOCATE_FROM_LOCAL_NODE_ONLY) {
        BaseColor = KeGetCurrentNode()->Color;
    }
    else {
        BaseColor = 0;
    }

    //
    // Walk the transition lists looking for pages satisfying the
    // constraints as walking the physical memory block can be draining.
    //

    for (ListHead = &MmStandbyPageListByPriority[0];
         ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
         ListHead += 1) {

        if (ListHead->Flink == MM_EMPTY_LIST) {
            continue;
        }

        LOCK_PFN (OldIrql);

        for (Page = ListHead->Flink; Page != MM_EMPTY_LIST; Page = NextPage) {
    
            //
            // Systems utilizing memory compression may have more
            // pages on the zero, free and standby lists than we
            // want to give out.  The same is true when machines
            // are low on memory - we don't want this thread to gobble
            // up the pages from every modified write that completes
            // because that would starve waiting threads.
            //
            // Explicitly check MmAvailablePages instead (and recheck whenever
            // the PFN lock is released and reacquired).
            //
    
            if (MmAvailablePages < MM_HIGH_LIMIT) {
                ListHead = &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
                break;
            }
    
            Pfn1 = MI_PFN_ELEMENT (Page);
            NextPage = Pfn1->u1.Flink;
    
            //
            // Since the caller may do anything with these frames including
            // mapping them uncached or write combined, don't give out frames
            // that are being mapped by (cached) superpages.
            //
    
            if ((CacheAttribute != MiCached) && (Pfn1->u4.MustBeCached == 1)) {
                continue;
            }
    
            LowPage1 = LowPage;
            HighPage1 = HighPage;
            PagePlacementOk = FALSE;
    
            do {
                if ((Page >= LowPage1) && (Page <= HighPage1)) {

                    if (Flags & MM_ALLOCATE_FROM_LOCAL_NODE_ONLY) {
                        if (BaseColor == Pfn1->u3.e1.PageColor) {
                            PagePlacementOk = TRUE;
                        }
                    }
                    else {
                        PagePlacementOk = TRUE;
                    }

                    break;
                }
    
                if (SkipPages == 0) {
                    break;
                }
    
                LowPage1 += SkipPages;
                HighPage1 += SkipPages;
    
                if (HighPage1 > MmHighestPhysicalPage) {
                    HighPage1 = MmHighestPhysicalPage;
                }
    
            } while (LowPage1 <= MmHighestPhysicalPage);
    
            if (PagePlacementOk == TRUE) {
    
                ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
    
                found += 1;
    
                //
                // This page is in the desired range - grab it.
                //
    
                MiUnlinkPageFromList (Pfn1);
                ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
                MiRestoreTransitionPte (Pfn1);
    
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u2.ShareCount = 1;
                MI_SET_PFN_DELETED(Pfn1);
                Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;

                CacheTypesFound |= (1 << Pfn1->u3.e1.CacheAttribute);

                Pfn1->u3.e1.CacheAttribute = CacheAttribute;
                Pfn1->u3.e1.StartOfAllocation = 1;
                Pfn1->u3.e1.EndOfAllocation = 1;
                Pfn1->u4.VerifierAllocation = 0;
                Pfn1->u3.e1.LargeSessionAllocation = 0;
    
                //
                // Add standby pages to the list of pages to be
                // zeroed before returning.
                //
    
                if ((Flags & MM_DONT_ZERO_ALLOCATION) == 0) {
    
                    ColoredPageInfo = &ColoredPageInfoBase[MI_GET_COLOR_FROM_LIST_ENTRY (Page, Pfn1)];
                    Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
                    ColoredPageInfo->PfnAllocation = Pfn1;
                    ColoredPageInfo->PagesQueued += 1;
                    ZeroCount += 1;
                }
                else {
                    Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                }
    
                *MdlPage = Page;
                MdlPage += 1;
    
                if (found == SizeInPages) {
    
                    //
                    // All the pages requested are available.
                    //
    
                    ListHead = &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
                    break;
                }
            }
        }

        UNLOCK_PFN (OldIrql);
    }

pass2_done:

    //
    // The full amount was charged up front - remove any excess now.
    //

    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages - found, MM_RESAVAIL_FREE_FOR_MDL_EXCESS);

    InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, 0 - (SizeInPages - found));

    MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
    KeLeaveGuardedRegionThread (Thread);

    MmUnlockPageableImageSection (ExPageLockHandle);

    if (found != MdlPageSpan) {
        ASSERT (found < MdlPageSpan);
        MiReturnCommitment (MdlPageSpan - found);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_AWE_EXCESS, MdlPageSpan - found);
    }

    if (found == 0) {
        ExFreePool (ColoredPageInfoBase);
        ExFreePool (MemoryDescriptorList);
        return NULL;
    }

    //
    // Update the color to the last one allocated (excluding any repurposed
    // transition pages) so subsequent allocations continue to round-robin
    // fairly.  Since the local color has already been incremented, carefully
    // decrement making sure not to cross nodes.
    //

    if ((Color & MmSecondaryColorMask) != 0) {
        Color -= 1;
    }
    else {
#if defined(MI_MULTINODE) 
        if (KeNumberNodes > 1) {
            if (NodePassesLeft == 1) {
                if (Color == 0) {
                    Color = MmSecondaryColors - 1;
                }
                else {
                    Color -= 1;
                }
            }
            else {
                Color |= MmSecondaryColorMask;
            }
        }
        else
#endif
        Color |= MmSecondaryColorMask;
    }

#if defined (NT_UP)
    MI_SYSTEM_PAGE_COLOR = Color;
#else
    Prcb->PageColor = Color;
#endif

    MemoryDescriptorList->ByteCount = (ULONG)(found << PAGE_SHIFT);

    if (found != SizeInPages) {
        *MdlPage = MM_EMPTY_LIST;
    }

    //
    // If we are providing cached pages then flush any stale TB entries now.
    // Any needed zeroing below will then use cached mappings as will our
    // caller.
    //

    if (CacheAttribute == MiCached) {

        //
        // If all the allocated pages were either not mapped or were last
        // mapped as cached, then skip the TB flush as there can be no
        // conflicting attributes with the new mapping.
        //

        if (CacheTypesFound & ~((1 << MiCached) | (1 << MiNotMapped))) {
            MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
        }
    }

    if (ZeroCount != 0) {

        //
        // Flush the entire TB since we overwrote the initial PFN attributes
        // above with MiNotMapped.  This ensures that the zeroing below can
        // use any attribute (it will use cached).
        //

        if (CacheAttribute != MiCached) {
            MI_FLUSH_ENTIRE_TB_FOR_ATTRIBUTE_CHANGE (MiNonCached);
        }

        //
        // Zero all the free & standby pages, fanning out the work.  This
        // is done even on UP machines because the worker thread code maps
        // large MDLs and is thus better performing than zeroing a single
        // page at a time.
        //

        MiZeroInParallel (ColoredPageInfoBase);

        //
        // Denote that no pages are left to be zeroed because in addition
        // to zeroing them, we have reset all their OriginalPte fields
        // to demand zero so they cannot be walked by the zeroing loop
        // below.
        //

        ZeroCount = 0;
    }

    //
    // Flush the entire TB since we overwrote the initial PFN attributes
    // above with MiNotMapped.  Even if we didn't have to zero any of the
    // pages above, we still need to get rid of any stale (ie: like system
    // PTE mappings to the pages in their previous lives) TB entries since
    // our caller may choose to use any attribute.
    //

    if (CacheAttribute != MiCached) {
        MI_FLUSH_ENTIRE_TB_FOR_ATTRIBUTE_CHANGE (MiNonCached);
    }

    ExFreePool (ColoredPageInfoBase);

    //
    // If the number of pages allocated was substantially less than the
    // initial request amount, attempt to allocate a smaller MDL to save
    // pool.
    //

    if ((MdlPageSpan - found) > ((4 * PAGE_SIZE) / sizeof (PFN_NUMBER))) {

        MemoryDescriptorList2 = MmCreateMdl (NULL,
                                             NULL,
                                             found << PAGE_SHIFT);
    
        if (MemoryDescriptorList2 != NULL) {

            RtlCopyMemory ((PVOID)(MemoryDescriptorList2 + 1),
                           (PVOID)(MemoryDescriptorList + 1),
                           found * sizeof (PFN_NUMBER));

            ExFreePool (MemoryDescriptorList);
            MemoryDescriptorList = MemoryDescriptorList2;
        }
    }

#if DBG
    //
    // Ensure all pages are within the caller's page constraints.
    //

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    LastMdlPage = MdlPage + found;

    LowPage = (PFN_NUMBER)(LowAddress.QuadPart >> PAGE_SHIFT);
    HighPage = (PFN_NUMBER)(HighAddress.QuadPart >> PAGE_SHIFT);
    Process = PsGetCurrentProcess ();

    MmLockPageableSectionByHandle (ExPageLockHandle);

    while (MdlPage < LastMdlPage) {
        Page = *MdlPage;
        PagePlacementOk = FALSE;
        LowPage1 = LowPage;
        HighPage1 = HighPage;

        do {
            if ((Page >= LowPage1) && (Page <= HighPage1)) {
                PagePlacementOk = TRUE;
                break;
            }

            if (SkipPages == 0) {
                break;
            }

            LowPage1 += SkipPages;
            HighPage1 += SkipPages;

            if (LowPage1 > MmHighestPhysicalPage) {
                break;
            }
            if (HighPage1 > MmHighestPhysicalPage) {
                HighPage1 = MmHighestPhysicalPage;
            }
        } while (TRUE);

        ASSERT (PagePlacementOk == TRUE);
        Pfn1 = MI_PFN_ELEMENT(*MdlPage);
        ASSERT (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME);
        MdlPage += 1;
    }

    MmUnlockPageableImageSection (ExPageLockHandle);

#endif

    MemoryDescriptorList->Process = NULL;

    //
    // Mark the MDL's pages as locked so the the kernelmode caller can
    // map the MDL using MmMapLockedPages* without asserting.
    //

    MemoryDescriptorList->MdlFlags |= MDL_PAGES_LOCKED;

    return MemoryDescriptorList;
}


PMDL
MmAllocatePagesForMdl (
    __in PHYSICAL_ADDRESS LowAddress,
    __in PHYSICAL_ADDRESS HighAddress,
    __in PHYSICAL_ADDRESS SkipBytes,
    __in SIZE_T TotalBytes
    )

/*++

Routine Description:

    This routine searches the PFN database for free, zeroed or standby pages
    to satisfy the request.  This does not map the pages - it just allocates
    them and puts them into an MDL.  It is expected that our caller will
    map the MDL as needed.

    NOTE: this routine may return an MDL mapping a smaller number of bytes
    than the amount requested.  It is the caller's responsibility to check the
    MDL upon return for the size actually allocated.

    These pages comprise physical non-paged memory and are zero-filled.

    This routine is designed to be used by an AGP driver to obtain physical
    memory in a specified range since hardware may provide substantial
    performance wins depending on where the backing memory is allocated.

    Because the caller may use these pages for a noncached mapping, care is
    taken to never allocate any pages that reside in a large page (in order
    to prevent TB incoherency of the same page being mapped by multiple
    translations with different attributes).

Arguments:

    LowAddress - Supplies the low physical address of the first range that
                 the allocated pages can come from.

    HighAddress - Supplies the high physical address of the first range that
                  the allocated pages can come from.

    SkipBytes - Number of bytes to skip (from the Low Address) to get to the
                next physical address range that allocated pages can come from.

    TotalBytes - Supplies the number of bytes to allocate.

Return Value:

    MDL - An MDL mapping a range of pages in the specified range.
          This may map less memory than the caller requested if the full amount
          is not currently available.

    NULL - No pages in the specified range OR not enough virtually contiguous
           nonpaged pool for the MDL is available at this time.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    return MiAllocatePagesForMdl (LowAddress,
                                  HighAddress,
                                  SkipBytes,
                                  TotalBytes,
                                  MiNotMapped,
                                  0);
}


PMDL
MmAllocatePagesForMdlEx (
    __in PHYSICAL_ADDRESS LowAddress,
    __in PHYSICAL_ADDRESS HighAddress,
    __in PHYSICAL_ADDRESS SkipBytes,
    __in SIZE_T TotalBytes,
    __in MEMORY_CACHING_TYPE CacheType,
    __in ULONG Flags
    )

/*++

Routine Description:

    This routine searches the PFN database for free, zeroed or standby pages
    to satisfy the request.  This does not map the pages - it just allocates
    them and puts them into an MDL.  It is expected that our caller will
    map the MDL as needed.

    NOTE: this routine may return an MDL mapping a smaller number of bytes
    than the amount requested.  It is the caller's responsibility to check the
    MDL upon return for the size actually allocated.

    These pages comprise physical non-paged memory and are zero-filled.

    This routine is designed to be used by an AGP driver to obtain physical
    memory in a specified range since hardware may provide substantial
    performance wins depending on where the backing memory is allocated.

    Because the caller may use these pages for a noncached mapping, care is
    taken to never allocate any pages that reside in a large page (in order
    to prevent TB incoherency of the same page being mapped by multiple
    translations with different attributes).

Arguments:

    LowAddress - Supplies the low physical address of the first range that
                 the allocated pages can come from.

    HighAddress - Supplies the high physical address of the first range that
                  the allocated pages can come from.

    SkipBytes - Number of bytes to skip (from the Low Address) to get to the
                next physical address range that allocated pages can come from.

    TotalBytes - Supplies the number of bytes to allocate.

    CacheType - Supplies the cache type the caller desires for these pages.

    Flags - Supplies flags passed by the caller, currently there are 2:
            MM_DONT_ZERO_ALLOCATION and MM_ALLOCATE_FROM_LOCAL_NODE_ONLY

Return Value:

    MDL - An MDL mapping a range of pages in the specified range.
          This may map less memory than the caller requested if the full amount
          is not currently available.

    NULL - No pages in the specified range OR not enough virtually contiguous
           nonpaged pool for the MDL is available at this time.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    if (CacheType > MmWriteCombined) {
        CacheAttribute = MiNotMapped;
    }
    else {
        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, FALSE);
    }

    if (Flags & ~(MM_DONT_ZERO_ALLOCATION | MM_ALLOCATE_FROM_LOCAL_NODE_ONLY)) {

        //
        // Don't allow external callers to specify internal flags (like
        // MI_ALLOCATION_IS_AWE).
        //

        return NULL;
    }

    return MiAllocatePagesForMdl (LowAddress,
                                  HighAddress,
                                  SkipBytes,
                                  TotalBytes,
                                  CacheAttribute,
                                  Flags);
}


VOID
MmFreePagesFromMdl (
    __in PMDL MemoryDescriptorList
    )

/*++

Routine Description:

    This routine walks the argument MDL freeing each physical page back to
    the PFN database.  This is designed to free pages acquired via
    MmAllocatePagesForMdl only.

Arguments:

    MemoryDescriptorList - Supplies an MDL which contains the pages to be freed.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PVOID StartingAddress;
    PVOID AlignedVa;
    PPFN_NUMBER Page;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER TotalPages;
    PFN_NUMBER DeltaPages;
    LONG EntryCount;
    LONG OriginalCount;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    DeltaPages = 0;

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0);

    ASSERT (((ULONG_PTR)MemoryDescriptorList->StartVa & (PAGE_SIZE - 1)) == 0);
    AlignedVa = (PVOID)MemoryDescriptorList->StartVa;

    StartingAddress = (PVOID)((PCHAR)AlignedVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartingAddress,
                                              MemoryDescriptorList->ByteCount);

    TotalPages = NumberOfPages;

    MmLockPageableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
        if (*Page == MM_EMPTY_LIST) {

            //
            // There are no more locked pages.
            //

            break;
        }

        ASSERT (MI_IS_PFN (*Page));
        ASSERT (*Page <= MmHighestPhysicalPage);

        Pfn1 = MI_PFN_ELEMENT (*Page);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (MI_IS_PFN_DELETED (Pfn1) == TRUE);
        ASSERT (MI_PFN_IS_AWE (Pfn1) == TRUE);

        if (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME) {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x1236,
                          (ULONG_PTR) MemoryDescriptorList,
                          (ULONG_PTR) Page,
                          *Page);
        }

        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u2.ShareCount = 0;

        //
        // If the frame was never mapped, then default to calling it cached
        // to reduce TB invalidates.
        //

        if (Pfn1->u3.e1.CacheAttribute == MiNotMapped) {
            Pfn1->u3.e1.CacheAttribute = MiCached;
        }

        Pfn1->u4.PteFrame -= 1;
#if DBG
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif

        if (Pfn1->u4.AweAllocation == 1) {

            do {

                EntryCount = ReadForWriteAccess (&Pfn1->AweReferenceCount);

                ASSERT ((LONG)EntryCount > 0);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

                OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                            EntryCount - 1,
                                                            EntryCount);

                if (OriginalCount == EntryCount) {

                    //
                    // This thread can be racing against other threads
                    // calling MmUnlockPages.  All threads can safely do
                    // interlocked decrements on the "AWE reference count".
                    // Whichever thread drives it to zero is responsible for
                    // decrementing the actual PFN reference count (which may
                    // be greater than 1 due to other non-AWE API calls being
                    // used on the same page).  The thread that drives this
                    // reference count to zero must put the page on the actual
                    // freelist at that time and decrement various resident
                    // available and commitment counters also.
                    //

                    if (OriginalCount == 1) {

                        //
                        // This thread has driven the AWE reference count to
                        // zero so it must initiate a decrement of the PFN
                        // reference count (while holding the PFN lock), etc.
                        //
                        // This path should be the frequent one since typically
                        // I/Os complete before these types of pages are
                        // freed by the app.
                        //

                        MiDecrementReferenceCountForAwePage (Pfn1, TRUE);
                    }

                    break;
                }
            } while (TRUE);
        }
        else {
            MiDecrementReferenceCountInline (Pfn1, *Page);
            DeltaPages += 1;
        }

        *Page++ = MM_EMPTY_LIST;

        //
        // Nothing magic about the divisor here - just releasing the PFN lock
        // periodically to allow other processors and DPCs a chance to execute.
        //

        if ((NumberOfPages & 0xF) == 0) {

            UNLOCK_PFN (OldIrql);

            LOCK_PFN (OldIrql);
        }

        NumberOfPages -= 1;

    } while (NumberOfPages != 0);

    UNLOCK_PFN (OldIrql);

    MmUnlockPageableImageSection (ExPageLockHandle);

    if (DeltaPages != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (DeltaPages, MM_RESAVAIL_FREE_FROM_MDL);
        InterlockedExchangeAddSizeT (&MmMdlPagesAllocated, 0 - DeltaPages);
        MiReturnCommitment (DeltaPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_MDL_PAGES, DeltaPages);
    }

    MemoryDescriptorList->MdlFlags &= ~MDL_PAGES_LOCKED;
}


NTSTATUS
MmMapUserAddressesToPage (
    __in_bcount(NumberOfBytes) PVOID BaseAddress,
    __in SIZE_T NumberOfBytes,
    __in PVOID PageAddress
    )

/*++

Routine Description:

    This function maps a range of addresses in a physical memory VAD to the
    specified page address.  This is typically used by a driver to nicely
    remove an application's access to things like video memory when the
    application is not responding to requests to relinquish it.

    Note the entire range must be currently mapped (ie, all the PTEs must
    be valid) by the caller.

Arguments:

    BaseAddress - Supplies the base virtual address where the physical
                  address is mapped.

    NumberOfBytes - Supplies the number of bytes to remap to the new address.

    PageAddress - Supplies the virtual address of the page this is remapped to.
                  This must be nonpaged memory.

Return Value:

    Various NTSTATUS codes.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PMMVAD Vad;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PMMPTE LastPte;
    PETHREAD Thread;
    PEPROCESS Process;
    NTSTATUS Status;
    PVOID EndingAddress;
    PFN_NUMBER PageFrameNumber;
    SIZE_T NumberOfPtes;
    PHYSICAL_ADDRESS PhysicalAddress;
    KIRQL OldIrql;

    PAGED_CODE();

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER_1;
    }

    if ((ULONG_PTR)BaseAddress + NumberOfBytes > (ULONG64)MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER_2;
    }

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

    EndingAddress = (PVOID)((PCHAR)BaseAddress + NumberOfBytes - 1);

    LOCK_ADDRESS_SPACE (Process);

    //
    // Make sure the address space was not deleted.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    Vad = (PMMVAD)MiLocateAddress (BaseAddress);

    if (Vad == NULL) {

        //
        // No virtual address descriptor located.
        //

        Status = STATUS_MEMORY_NOT_ALLOCATED;
        goto ErrorReturn;
    }

    if (NumberOfBytes == 0) {

        //
        // If the region size is specified as 0, the base address
        // must be the starting address for the region.  The entire VAD
        // will then be re-pointed.
        //

        if (MI_VA_TO_VPN (BaseAddress) != Vad->StartingVpn) {
            Status = STATUS_FREE_VM_NOT_AT_BASE;
            goto ErrorReturn;
        }

        BaseAddress = MI_VPN_TO_VA (Vad->StartingVpn);
        EndingAddress = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);
        NumberOfBytes = (PCHAR)EndingAddress - (PCHAR)BaseAddress + 1;
    }

    //
    // Found the associated virtual address descriptor.
    //

    if (Vad->EndingVpn < MI_VA_TO_VPN (EndingAddress)) {

        //
        // The entire range to remap is not contained within a single
        // virtual address descriptor.  Return an error.
        //

        Status = STATUS_INVALID_PARAMETER_2;
        goto ErrorReturn;
    }

    if (Vad->u.VadFlags.VadType != VadDevicePhysicalMemory) {

        //
        // The virtual address descriptor is not a physical mapping.
        //

        Status = STATUS_INVALID_ADDRESS;
        goto ErrorReturn;
    }

    PointerPte = MiGetPteAddress (BaseAddress);
    LastPte = MiGetPteAddress (EndingAddress);
    NumberOfPtes = LastPte - PointerPte + 1;

    //
    // Lock down because the PFN lock is going to be acquired shortly.
    //

    MmLockPageableSectionByHandle(ExPageLockHandle);

    LOCK_WS_UNSAFE (Thread, Process);

    PhysicalAddress = MmGetPhysicalAddress (PageAddress);
    PageFrameNumber = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    PteContents = *PointerPte;
    PteContents.u.Hard.PageFrameNumber = PageFrameNumber;

#if DBG

    //
    // All the PTEs must be valid or the filling will corrupt the
    // UsedPageTableCounts.
    //

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PointerPte += 1;
    } while (PointerPte < LastPte);
    PointerPte = MiGetPteAddress (BaseAddress);
#endif

    //
    // Fill the PTEs and flush at the end - no race here because it doesn't
    // matter whether the user app sees the old or the new data until we
    // return (writes going to either page is acceptable prior to return
    // from this function).  There is no race with I/O and ProbeAndLockPages
    // because the PFN lock is acquired here.
    //

    LOCK_PFN (OldIrql);

#if !defined (_X86PAE_)
    MiFillMemoryPte (PointerPte, NumberOfPtes, PteContents.u.Long);
#else

    //
    // Note that the PAE architecture must very carefully fill these PTEs.
    //

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PointerPte += 1;
        InterlockedExchangePte (PointerPte, PteContents.u.Long);
    } while (PointerPte < LastPte);
    PointerPte = MiGetPteAddress (BaseAddress);

#endif

    if (NumberOfPtes == 1) {
        MI_FLUSH_SINGLE_TB (BaseAddress, FALSE);
    }
    else {
        MI_FLUSH_PROCESS_TB (FALSE);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS_UNSAFE (Thread, Process);

    MmUnlockPageableImageSection (ExPageLockHandle);

    Status = STATUS_SUCCESS;

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);

    return Status;
}


PHYSICAL_ADDRESS
MmGetPhysicalAddress (
    __in PVOID BaseAddress
    )

/*++

Routine Description:

    This function returns the corresponding physical address for a
    valid virtual address.

Arguments:

    BaseAddress - Supplies the virtual address for which to return the
                  physical address.

Return Value:

    Returns the corresponding physical address.

Environment:

    Kernel mode.  Any IRQL level.

--*/

{
    PMMPTE PointerPte;
    PHYSICAL_ADDRESS PhysicalAddress;

    if (MI_IS_PHYSICAL_ADDRESS(BaseAddress)) {
        PhysicalAddress.QuadPart = MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress);
    }
    else {

#if (_MI_PAGING_LEVELS>=4)
        PointerPte = MiGetPxeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }
#endif

#if (_MI_PAGING_LEVELS>=3)
        PointerPte = MiGetPpeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }
#endif

        PointerPte = MiGetPdeAddress (BaseAddress);
        if (PointerPte->u.Hard.Valid == 0) {
            KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                      BaseAddress));
            ZERO_LARGE (PhysicalAddress);
            return PhysicalAddress;
        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {
            PhysicalAddress.QuadPart = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) +
                                           MiGetPteOffset (BaseAddress);
        }
        else {
            PointerPte = MiGetPteAddress (BaseAddress);

            if (PointerPte->u.Hard.Valid == 0) {
                KdPrint(("MM:MmGetPhysicalAddressFailed base address was %p",
                          BaseAddress));
                ZERO_LARGE (PhysicalAddress);
                return PhysicalAddress;
            }
            PhysicalAddress.QuadPart = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        }
    }

    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;
    PhysicalAddress.LowPart += BYTE_OFFSET(BaseAddress);

    return PhysicalAddress;
}

PVOID
MmGetVirtualForPhysical (
    __in PHYSICAL_ADDRESS PhysicalAddress
    )

/*++

Routine Description:

    This function returns the corresponding virtual address for a physical
    address whose primary virtual address is in system space.

Arguments:

    PhysicalAddress - Supplies the physical address for which to return the
                      virtual address.

Return Value:

    Returns the corresponding virtual address.

Environment:

    Kernel mode.  Any IRQL level.

--*/

{
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    Pfn = MI_PFN_ELEMENT (PageFrameIndex);

    return (PVOID)((PCHAR)MiGetVirtualAddressMappedByPte (Pfn->PteAddress) +
                    BYTE_OFFSET (PhysicalAddress.LowPart));
}


__out_bcount(NumberOfBytes) PVOID
MmAllocateNonCachedMemory (
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function allocates a range of noncached memory in
    the non-paged portion of the system address space.

    This routine is designed to be used by a driver's initialization
    routine to allocate a noncached block of virtual memory for
    various device specific buffers.

Arguments:

    NumberOfBytes - Supplies the number of bytes to allocate.

Return Value:

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

    NULL - The specified request could not be satisfied.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PPFN_NUMBER Page;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMDL Mdl;
    PVOID BaseAddress;
    PHYSICAL_ADDRESS LowAddress;
    PHYSICAL_ADDRESS HighAddress;
    PHYSICAL_ADDRESS SkipBytes;
    PFN_NUMBER NumberOfPagesAllocated;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    ASSERT (NumberOfBytes != 0);

#if defined (_WIN64)

    //
    // Maximum allocation size is constrained by the MDL ByteCount field.
    //

    if (NumberOfBytes >= _4gb) {
        return NULL;
    }

#endif

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    //
    // Even though an MDL is not needed per se, it is much more convenient
    // to use the routine below because it checks for things like appropriate
    // cachability of the pages, etc.  Note that the MDL returned may map
    // fewer pages than requested - check for this and if so, return NULL.
    //

    LowAddress.QuadPart = 0;
    HighAddress.QuadPart = (ULONGLONG)-1;
    SkipBytes.QuadPart = 0;

    CacheAttribute = MI_TRANSLATE_CACHETYPE (MmNonCached, FALSE);

    Mdl = MiAllocatePagesForMdl (LowAddress,
                                 HighAddress,
                                 SkipBytes,
                                 NumberOfBytes,
                                 CacheAttribute,
                                 0);
    if (Mdl == NULL) {
        return NULL;
    }

    BaseAddress = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    NumberOfPagesAllocated = ADDRESS_AND_SIZE_TO_SPAN_PAGES (BaseAddress, Mdl->ByteCount);

    if (NumberOfPages != NumberOfPagesAllocated) {
        ASSERT (NumberOfPages > NumberOfPagesAllocated);
        MmFreePagesFromMdl (Mdl);
        ExFreePool (Mdl);
        return NULL;
    }

    //
    // Obtain enough virtual space to map the pages.  Add an extra PTE so the
    // MDL can be stashed now and retrieved on release.
    //

    PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages + 1, SystemPteSpace);

    if (PointerPte == NULL) {
        MmFreePagesFromMdl (Mdl);
        ExFreePool (Mdl);
        return NULL;
    }

    *(PMDL *)PointerPte = Mdl;
    PointerPte += 1;

    BaseAddress = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);

    Page = (PPFN_NUMBER)(Mdl + 1);

    MI_MAKE_VALID_KERNEL_PTE (TempPte,
                              0,
                              MM_READWRITE,
                              PointerPte);

    MI_SET_PTE_DIRTY (TempPte);

    switch (CacheAttribute) {

        case MiNonCached:
            MI_DISABLE_CACHING (TempPte);
            break;

        case MiCached:
            break;

        case MiWriteCombined:
            MI_SET_PTE_WRITE_COMBINE (TempPte);
            break;

        default:
            ASSERT (FALSE);
            break;
    }

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 0);
        PageFrameIndex = *Page;

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        Page += 1;
        PointerPte += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    return BaseAddress;
}

VOID
MmFreeNonCachedMemory (
    __in_bcount(NumberOfBytes) PVOID BaseAddress,
    __in SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This function deallocates a range of noncached memory in
    the non-paged portion of the system address space.

Arguments:

    BaseAddress - Supplies the base virtual address where the noncached
                  memory resides.

    NumberOfBytes - Supplies the number of bytes allocated to the request.
                    This must be the same number that was obtained with
                    the MmAllocateNonCachedMemory call.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PMDL Mdl;
    PMMPTE PointerPte;
    PFN_NUMBER NumberOfPages;
#if DBG
    PFN_NUMBER i;
    PVOID StartingAddress;
#endif

    ASSERT (NumberOfBytes != 0);
    ASSERT (PAGE_ALIGN (BaseAddress) == BaseAddress);

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    PointerPte = MiGetPteAddress (BaseAddress);

    Mdl = *(PMDL *)(PointerPte - 1);

#if DBG
    StartingAddress = (PVOID)((PCHAR)Mdl->StartVa + Mdl->ByteOffset);

    i = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingAddress, Mdl->ByteCount);

    ASSERT (NumberOfPages == i);
#endif

    MmFreePagesFromMdl (Mdl);

    ExFreePool (Mdl);

    MiReleaseSystemPtes (PointerPte - 1,
                         (ULONG)NumberOfPages + 1,
                         SystemPteSpace);

    return;
}

SIZE_T
MmSizeOfMdl (
    __in_bcount_opt(Length) PVOID Base,
    __in SIZE_T Length
    )

/*++

Routine Description:

    This function returns the number of bytes required for an MDL for a
    given buffer and size.

Arguments:

    Base - Supplies the base virtual address for the buffer.

    Length - Supplies the size of the buffer in bytes.

Return Value:

    Returns the number of bytes required to contain the MDL.

Environment:

    Kernel mode.  Any IRQL level.

--*/

{
    return( sizeof( MDL ) +
                (ADDRESS_AND_SIZE_TO_SPAN_PAGES( Base, Length ) *
                 sizeof( PFN_NUMBER ))
          );
}


PMDL
MmCreateMdl (
    __in_opt PMDL MemoryDescriptorList,
    __in_bcount_opt(Length) PVOID Base,
    __in SIZE_T Length
    )

/*++

Routine Description:

    This function optionally allocates and initializes an MDL.

Arguments:

    MemoryDescriptorList - Optionally supplies the address of the MDL
                           to initialize.  If this address is supplied as NULL
                           an MDL is allocated from non-paged pool and
                           initialized.

    Base - Supplies the base virtual address for the buffer.

    Length - Supplies the size of the buffer in bytes.

Return Value:

    Returns the address of the initialized MDL.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    SIZE_T MdlSize;

#if defined (_WIN64)
    //
    // Since the Length has to fit into the MDL's ByteCount field, ensure it
    // doesn't wrap on 64-bit systems.
    //

    if (Length >= _4gb) {
        return NULL;
    }
#endif

    MdlSize = MmSizeOfMdl (Base, Length);

    if (!ARGUMENT_PRESENT(MemoryDescriptorList)) {

        MemoryDescriptorList = (PMDL)ExAllocatePoolWithTag (NonPagedPool,
                                                            MdlSize,
                                                            'ldmM');
        if (MemoryDescriptorList == (PMDL)0) {
            return NULL;
        }
    }

    MmInitializeMdl (MemoryDescriptorList, Base, Length);
    return MemoryDescriptorList;
}

BOOLEAN
MmSetAddressRangeModified (
    __in_bcount(Length) PVOID Address,
    __in SIZE_T Length
    )

/*++

Routine Description:

    This routine sets the modified bit in the PFN database for the
    pages that correspond to the specified address range.

    Note that the dirty bit in the PTE is cleared by this operation.

Arguments:

    Address - Supplies the address of the start of the range.  This
              range must reside within the system cache.

    Length - Supplies the length of the range.

Return Value:

    TRUE if at least one PTE was dirty in the range, FALSE otherwise.

Environment:

    Kernel mode.  APC_LEVEL and below for pageable addresses,
                  DISPATCH_LEVEL and below for non-pageable addresses.

--*/

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPFN Pfn1;
    MMPTE PteContents;
    KIRQL OldIrql;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    ULONG Count;
    BOOLEAN Result;

    Count = 0;
    Result = FALSE;

    //
    // Loop on the copy on write case until the page is only
    // writable.
    //

    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + Length - 1));

    LOCK_PFN2 (OldIrql);

    do {

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            MI_SET_MODIFIED (Pfn1, 1, 0x5);

            if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                         (Pfn1->u3.e1.WriteInProgress == 0)) {
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
            }

#ifdef NT_UP
            //
            // On uniprocessor systems no need to flush if this processor
            // doesn't think the PTE is dirty.
            //

            if (MI_IS_PTE_DIRTY (PteContents)) {
                Result = TRUE;
#else  //NT_UP
                Result |= (BOOLEAN)(MI_IS_PTE_DIRTY (PteContents));
#endif //NT_UP

                //
                // Clear the write bit in the PTE so new writes can be tracked.
                //

                MI_SET_PTE_CLEAN (PteContents);
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

                if (Count != MM_MAXIMUM_FLUSH_COUNT) {
                    VaFlushList[Count] = Address;
                    Count += 1;
                }
#ifdef NT_UP
            }
#endif //NT_UP
        }
        PointerPte += 1;
        Address = (PVOID)((PCHAR)Address + PAGE_SIZE);
    } while (PointerPte <= LastPte);

    if (Count != 0) {
        if (Count == 1) {
            MI_FLUSH_SINGLE_TB (VaFlushList[0], TRUE);
        }
        else if (Count != MM_MAXIMUM_FLUSH_COUNT) {
            MI_FLUSH_MULTIPLE_TB (Count, &VaFlushList[0], TRUE);
        }
        else {
            MI_FLUSH_ENTIRE_TB (6);
        }
    }
    UNLOCK_PFN2 (OldIrql);
    return Result;
}


PVOID
MiCheckForContiguousMemory (
    IN PVOID BaseAddress,
    IN PFN_NUMBER BaseAddressPages,
    IN PFN_NUMBER SizeInPages,
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )

/*++

Routine Description:

    This routine checks to see if the physical memory mapped
    by the specified BaseAddress for the specified size is
    contiguous and that the first page is greater than or equal to
    the specified LowestPfn and that the last page of the physical memory is
    less than or equal to the specified HighestPfn.

Arguments:

    BaseAddress - Supplies the base address to start checking at.

    BaseAddressPages - Supplies the number of pages to scan from the
                       BaseAddress.

    SizeInPages - Supplies the number of pages in the range.

    LowestPfn - Supplies lowest PFN acceptable as a physical page.

    HighestPfn - Supplies the highest PFN acceptable as a physical page.

    BoundaryPfn - Supplies the PFN multiple the allocation must
                  not cross.  0 indicates it can cross any boundary.

    CacheAttribute - Supplies the type of cache mapping that will be used
                     for the memory.

Return Value:

    Returns the usable virtual address within the argument range that the
    caller should return to his caller.  NULL if there is no usable address.

Environment:

    Kernel mode, memory management internal.

--*/

{
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER PreviousPage;
    PFN_NUMBER Page;
    PFN_NUMBER HighestStartPage;
    PFN_NUMBER LastPage;
    PFN_NUMBER OriginalPage;
    PFN_NUMBER OriginalLastPage;
    PVOID BoundaryAllocation;
    PFN_NUMBER BoundaryMask;
    PFN_NUMBER PageCount;
    MMPTE PteContents;

    BoundaryMask = ~(BoundaryPfn - 1);

    if (LowestPfn > HighestPfn) {
        return NULL;
    }

    if (LowestPfn + SizeInPages <= LowestPfn) {
        return NULL;
    }

    if (LowestPfn + SizeInPages - 1 > HighestPfn) {
        return NULL;
    }

    if (BaseAddressPages < SizeInPages) {
        return NULL;
    }

    if (MI_IS_PHYSICAL_ADDRESS (BaseAddress)) {

        //
        // All physical addresses are by definition cached and therefore do
        // not qualify for our caller.
        //

        if (CacheAttribute != MiCached) {
            return NULL;
        }

        OriginalPage = MI_CONVERT_PHYSICAL_TO_PFN(BaseAddress);
        OriginalLastPage = OriginalPage + BaseAddressPages;

        Page = OriginalPage;
        LastPage = OriginalLastPage;

        //
        // Close the gaps, then examine the range for a fit.
        //

        if (Page < LowestPfn) {
            Page = LowestPfn;
        }

        if (LastPage > HighestPfn + 1) {
            LastPage = HighestPfn + 1;
        }

        HighestStartPage = LastPage - SizeInPages;

        if (Page > HighestStartPage) {
            return NULL;
        }

        if (BoundaryPfn != 0) {
            do {
                if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) == 0) {

                    //
                    // This portion of the range meets the alignment
                    // requirements.
                    //

                    break;
                }
                Page |= (BoundaryPfn - 1);
                Page += 1;
            } while (Page <= HighestStartPage);

            if (Page > HighestStartPage) {
                return NULL;
            }
            BoundaryAllocation = (PVOID)((PCHAR)BaseAddress + ((Page - OriginalPage) << PAGE_SHIFT));

            //
            // The request can be satisfied.  Since specific alignment was
            // requested, return the fit now without getting fancy.
            //

            return BoundaryAllocation;
        }

        //
        // If possible return a chunk on the end to reduce fragmentation.
        //
    
        if (LastPage == OriginalLastPage) {
            return (PVOID)((PCHAR)BaseAddress + ((BaseAddressPages - SizeInPages) << PAGE_SHIFT));
        }
    
        //
        // The end chunk did not satisfy the requirements.  The next best option
        // is to return a chunk from the beginning.  Since that's where the search
        // began, just return the current chunk.
        //

        return (PVOID)((PCHAR)BaseAddress + ((Page - OriginalPage) << PAGE_SHIFT));
    }

    //
    // Check the virtual addresses for physical contiguity.
    //

    PointerPte = MiGetPteAddress (BaseAddress);
    LastPte = PointerPte + BaseAddressPages;

    HighestStartPage = HighestPfn + 1 - SizeInPages;
    PageCount = 0;

    //
    // Initializing PreviousPage is not needed for correctness
    // but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    PreviousPage = 0;

    while (PointerPte < LastPte) {

        PteContents = *PointerPte;
        ASSERT (PteContents.u.Hard.Valid == 1);
        Page = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

        //
        // Before starting a new run, ensure that it
        // can satisfy the location & boundary requirements (if any).
        //

        if (PageCount == 0) {

            if ((Page >= LowestPfn) &&
                (Page <= HighestStartPage) &&
                ((CacheAttribute == MiCached) || (MI_PFN_ELEMENT (Page)->u4.MustBeCached == 0))) {

                if (BoundaryPfn == 0) {
                    PageCount += 1;
                }
                else if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) == 0) {
                    //
                    // This run's physical address meets the alignment
                    // requirement.
                    //

                    PageCount += 1;
                }
            }

            if (PageCount == SizeInPages) {

                if (CacheAttribute != MiCached) {

                    //
                    // Recheck the cachability while holding the PFN lock.
                    //

                    LOCK_PFN2 (OldIrql);
                
                    if (MI_PFN_ELEMENT (Page)->u4.MustBeCached == 0) {
                        PageCount = 1;
                    }
                    else {
                        PageCount = 0;
                    }

                    UNLOCK_PFN2 (OldIrql);
                }

                if (PageCount != 0) {

                    //
                    // Success - found a single page satifying the requirements.
                    //

                    BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                    return BaseAddress;
                }
            }

            PreviousPage = Page;
            PointerPte += 1;
            continue;
        }

        if (Page != PreviousPage + 1) {

            //
            // This page is not physically contiguous.  Start over.
            //

            PageCount = 0;
            continue;
        }

        PageCount += 1;

        if (PageCount == SizeInPages) {

            if (CacheAttribute != MiCached) {

                LOCK_PFN2 (OldIrql);

                do {
                    if ((MI_PFN_ELEMENT (Page))->u4.MustBeCached == 1) {
                        break;
                    }

                    Page -= 1;
                    PageCount -= 1;

                } while (PageCount != 0);

                UNLOCK_PFN2 (OldIrql);

                if (PageCount != 0) {
                    PageCount = 0;
                    continue;
                }

                PageCount = SizeInPages;
            }

            //
            // Success - found a page range satifying the requirements.
            //

            BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte - PageCount + 1);
            return BaseAddress;
        }

        PreviousPage = Page;
        PointerPte += 1;
    }

    return NULL;
}


VOID
MmLockPagableSectionByHandle (
    __in PVOID ImageSectionHandle
    )
{
    MmLockPageableSectionByHandle ( ImageSectionHandle );
}

VOID
MmLockPageableSectionByHandle (
    __in PVOID ImageSectionHandle
    )


/*++

Routine Description:

    This routine checks to see if the specified pages are resident in
    the process's working set and if so the reference count for the
    page is incremented.  The allows the virtual address to be accessed
    without getting a hard page fault (have to go to the disk... except
    for extremely rare case when the page table page is removed from the
    working set and migrates to the disk.

    If the virtual address is that of the system wide global "cache" the
    virtual address of the "locked" pages is always guaranteed to
    be valid.

    NOTE: This routine is not to be used for general locking of user
    addresses - use MmProbeAndLockPages.  This routine is intended for
    well behaved system code like the file system caches which allocates
    virtual addresses for mapping files AND guarantees that the mapping
    will not be modified (deleted or changed) while the pages are locked.

Arguments:

    ImageSectionHandle - Supplies the value returned by a previous call
                         to MmLockPageableDataSection.  This is a pointer to
                         the section header for the image.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    ULONG EntryCount;
    ULONG OriginalCount;
    PKTHREAD CurrentThread;
    PIMAGE_SECTION_HEADER NtSection;
    PVOID BaseAddress;
    ULONG SizeToLock;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PLONG SectionLockCountPointer;

    if (MI_IS_PHYSICAL_ADDRESS(ImageSectionHandle)) {

        //
        // No need to lock physical addresses.
        //

        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)ImageSectionHandle;

    BaseAddress = SECTION_BASE_ADDRESS(NtSection);
    SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);

    ASSERT (!MI_IS_SYSTEM_CACHE_ADDRESS(BaseAddress));

    //
    // The address must be within the system space.
    //

    ASSERT (BaseAddress >= MmSystemRangeStart);

    SizeToLock = NtSection->SizeOfRawData;

    //
    // Generally, SizeOfRawData is larger than VirtualSize for each
    // section because it includes the padding to get to the subsection
    // alignment boundary.  However, if the image is linked with
    // subsection alignment == native page alignment, the linker will
    // have VirtualSize be much larger than SizeOfRawData because it
    // will account for all the bss.
    //

    if (SizeToLock < NtSection->Misc.VirtualSize) {
        SizeToLock = NtSection->Misc.VirtualSize;
    }

    PointerPte = MiGetPteAddress(BaseAddress);
    LastPte = MiGetPteAddress((PCHAR)BaseAddress + SizeToLock - 1);

    ASSERT (SizeToLock != 0);

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    //
    //  The lock count values have the following meanings :
    //
    //  Value of 0 means unlocked.
    //  Value of 1 means lock in progress by another thread.
    //  Value of 2 or more means locked.
    //
    //  If the value is 1, this thread must block until the other thread's
    //  lock operation is complete.
    //

    do {
        EntryCount = *SectionLockCountPointer;

        if (EntryCount != 1) {

            OriginalCount = InterlockedCompareExchange (SectionLockCountPointer,
                                                        EntryCount + 1,
                                                        EntryCount);

            if (OriginalCount == EntryCount) {

                //
                // Success - this is the first thread to update.
                //

                ASSERT (OriginalCount != 1);
                break;
            }

            //
            // Another thread updated the count before this thread's attempt
            // so it's time to start over.
            //
        }
        else {

            //
            // A lock is in progress, wait for it to finish.  This should be
            // generally rare, and even in this case, the pulse will usually
            // wake us.  A timeout is used so that the wait and the pulse do
            // not need to be interlocked.
            //

            InterlockedIncrement (&MmCollidedLockWait);

            KeWaitForSingleObject (&MmCollidedLockEvent,
                                   WrVirtualMemory,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)&MmShortTime);

            InterlockedDecrement (&MmCollidedLockWait);
        }

    } while (TRUE);

    if (OriginalCount >= 2) {

        //
        // Already locked, just return.
        //

        KeLeaveCriticalRegionThread (CurrentThread);
        return;
    }

    ASSERT (OriginalCount == 0);
    ASSERT (*SectionLockCountPointer == 1);

    //
    // Value was 0 when the lock was obtained.  It is now 1 indicating
    // a lock is in progress.
    //

    MiLockCode (PointerPte, LastPte, MM_LOCK_BY_REFCOUNT);

    //
    // Set lock count to 2 (it was 1 when this started) and check
    // to see if any other threads tried to lock while this was happening.
    //

    ASSERT (*SectionLockCountPointer == 1);
    OriginalCount = InterlockedIncrement (SectionLockCountPointer);
    ASSERT (OriginalCount >= 2);

    if (MmCollidedLockWait != 0) {
        KePulseEvent (&MmCollidedLockEvent, 0, FALSE);
    }

    //
    // Enable user APCs now that the pulse has occurred.  They had to be
    // blocked to prevent any suspensions of this thread as that would
    // stop all waiters indefinitely.
    //

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


VOID
MiLockCode (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG LockType
    )

/*++

Routine Description:

    This routine checks to see if the specified pages are resident in
    the process's working set and if so the reference count for the
    page is incremented.  This allows the virtual address to be accessed
    without getting a hard page fault (have to go to the disk...) except
    for the extremely rare case when the page table page is removed from the
    working set and migrates to the disk.

    If the virtual address is that of the system wide global "cache", the
    virtual address of the "locked" pages is always guaranteed to
    be valid.

    NOTE: This routine is not to be used for general locking of user
    addresses - use MmProbeAndLockPages.  This routine is intended for
    well behaved system code like the file system caches which allocates
    virtual addresses for mapping files AND guarantees that the mapping
    will not be modified (deleted or changed) while the pages are locked.

Arguments:

    FirstPte - Supplies the base address to begin locking.

    LastPte - The last PTE to lock.

    LockType - Supplies either MM_LOCK_BY_REFCOUNT or MM_LOCK_NONPAGE.
               LOCK_BY_REFCOUNT increments the reference count to keep
               the page in memory, LOCK_NONPAGE removes the page from
               the working set so it's locked just like nonpaged pool.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE PteContents;
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER SwapEntry;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
    LOGICAL SessionSpace;
    PMMWSL WorkingSetList;
    PMMSUPPORT Vm;
    PETHREAD CurrentThread;

    ASSERT (!MI_IS_PHYSICAL_ADDRESS(MiGetVirtualAddressMappedByPte(FirstPte)));
    PointerPte = FirstPte;

    CurrentThread = PsGetCurrentThread ();

    SessionSpace = MI_IS_SESSION_IMAGE_ADDRESS (MiGetVirtualAddressMappedByPte(FirstPte));

    if (SessionSpace == TRUE) {
        Vm = &MmSessionSpace->GlobalVirtualAddress->Vm;
        WorkingSetList = MmSessionSpace->Vm.VmWorkingSetList;

        //
        // Session space is never locked by refcount.
        //

        ASSERT (LockType != MM_LOCK_BY_REFCOUNT);
    }
    else {

        Vm = &MmSystemCacheWs;
        WorkingSetList = NULL;
    }

    LOCK_WORKING_SET (CurrentThread, Vm);

    LOCK_PFN (OldIrql);

    do {

        PteContents = *PointerPte;
        ASSERT (PteContents.u.Long != 0);
        if (PteContents.u.Hard.Valid == 1) {

            //
            // This address is already in the system (or session) working set.
            //

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            //
            // Up the reference count so the page cannot be released.
            //

            MI_ADD_LOCKED_PAGE_CHARGE (Pfn1);

            if (LockType != MM_LOCK_BY_REFCOUNT) {

                //
                // If the page is in the system working set, remove it.
                // The system working set lock MUST be owned to check to
                // see if this page is in the working set or not.  This
                // is because the pager may have just released the PFN lock,
                // acquired the system lock and is now trying to add the
                // page to the system working set.
                //
                // If the page is in the SESSION working set, it cannot be
                // removed as all these pages are carefully accounted for.
                // Instead move it to the locked portion of the working set
                // if it is not there already.
                //

                if (Pfn1->u1.WsIndex != 0) {

                    UNLOCK_PFN (OldIrql);

                    if (SessionSpace == TRUE) {

                        WorkingSetIndex = MiLocateWsle (
                                    MiGetVirtualAddressMappedByPte(PointerPte),
                                    WorkingSetList,
                                    Pfn1->u1.WsIndex,
                                    FALSE);

                        if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {
                
                            SwapEntry = WorkingSetList->FirstDynamic;
                
                            if (WorkingSetIndex != WorkingSetList->FirstDynamic) {
                
                                //
                                // Swap this entry with the one at first
                                // dynamic.  Note that the working set index
                                // in the PTE is updated here as well.
                                //
                
                                MiSwapWslEntries (WorkingSetIndex,
                                                  SwapEntry,
                                                  Vm,
                                                  FALSE);
                            }
                
                            WorkingSetList->FirstDynamic += 1;

                            //
                            // Indicate that the page is now locked.
                            //
            
                            MmSessionSpace->Wsle[SwapEntry].u1.e1.LockedInWs = 1;
                            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_LOCK_CODE2, 1);
                            InterlockedIncrementSizeT (&MmSessionSpace->NonPageablePages);
                            LOCK_PFN (OldIrql);
                            Pfn1->u1.WsIndex = SwapEntry;

                            //
                            // Adjust available pages as this page is now not
                            // in any working set, just like a non-paged pool
                            // page.
                            //
            
                            MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE1);

                            if (Pfn1->u3.e1.PrototypePte == 0) {
                                InterlockedDecrement (&MmTotalSystemDriverPages);
                            }
                        }
                        else {
                            ASSERT (MmSessionSpace->Wsle[WorkingSetIndex].u1.e1.LockedInWs == 1);
                            LOCK_PFN (OldIrql);
                        }
                    }
                    else {
                        MiRemoveWsle (Pfn1->u1.WsIndex, MmSystemCacheWorkingSetList);
                        MiReleaseWsle (Pfn1->u1.WsIndex, &MmSystemCacheWs);

                        MI_SET_PTE_IN_WORKING_SET (PointerPte, 0);
                        LOCK_PFN (OldIrql);
                        MI_ZERO_WSINDEX (Pfn1);

                        //
                        // Adjust available pages as this page is now not in any
                        // working set, just like a non-paged pool page.
                        //
        
                        MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE2);

                        if (Pfn1->u3.e1.PrototypePte == 0) {
                            InterlockedDecrement (&MmTotalSystemDriverPages);
                        }
                    }

                }
                ASSERT (Pfn1->u3.e2.ReferenceCount > 1);
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);
            }
        }
        else if (PteContents.u.Soft.Prototype == 1) {

            //
            // Page is not in memory and it is a prototype.
            //

            MiMakeSystemAddressValidPfnSystemWs (
                    MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

            continue;
        }
        else if (PteContents.u.Soft.Transition == 1) {

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            if ((Pfn1->u3.e1.ReadInProgress) ||
                (Pfn1->u4.InPageError)) {

                //
                // Page read is ongoing, force a collided fault.
                //

                MiMakeSystemAddressValidPfnSystemWs (
                        MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

                continue;
            }

            //
            // Paged pool is trimmed without regard to sharecounts.
            // This means a paged pool PTE can be in transition while
            // the page is still marked active.
            //

            if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {

                ASSERT (((Pfn1->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                        (Pfn1->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                        ((Pfn1->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                        (Pfn1->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

                //
                // Don't increment the valid PTE count for the
                // paged pool page.
                //

                ASSERT (Pfn1->u2.ShareCount != 0);
                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                Pfn1->u2.ShareCount += 1;
            }
            else {

                if (MmAvailablePages == 0) {

                    //
                    // This can only happen if the system is utilizing
                    // a hardware compression cache.  This ensures that
                    // only a safe amount of the compressed virtual cache
                    // is directly mapped so that if the hardware gets
                    // into trouble, we can bail it out.
                    //
                    // Just unlock everything here to give the compression
                    // reaper a chance to ravage pages and then retry.
                    //

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (CurrentThread, Vm);

                    LOCK_WORKING_SET (CurrentThread, Vm);

                    LOCK_PFN (OldIrql);

                    continue;
                }

                MiUnlinkPageFromList (Pfn1);

                //
                // Increment the reference count and set the share count to 1.
                // Note the reference count may be 1 already if a modified page
                // write is underway.  The systemwide locked page charges
                // are correct in either case and nothing needs to be done
                // just yet.
                //

                InterlockedIncrement16 ((PSHORT)&Pfn1->u3.e2.ReferenceCount);
                Pfn1->u2.ShareCount = 1;
            }

            Pfn1->u3.e1.PageLocation = ActiveAndValid;

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               Pfn1->OriginalPte.u.Soft.Protection,
                               PointerPte);

            MI_WRITE_VALID_PTE (PointerPte, TempPte);

            //
            // Increment the reference count one for putting it the
            // working set list and one for locking it for I/O.
            //

            if (LockType == MM_LOCK_BY_REFCOUNT) {

                //
                // Lock the page in the working set by upping the
                // reference count.
                //

                MI_ADD_LOCKED_PAGE_CHARGE (Pfn1);

                Pfn1->u1.Event = NULL;

                UNLOCK_PFN (OldIrql);

                WorkingSetIndex = MiAllocateWsle (Vm,
                                                  PointerPte,
                                                  Pfn1,
                                                  0);

                if (WorkingSetIndex == 0) {

                    //
                    // No working set entry was available.  Another (broken
                    // or malicious thread) may have already written to this
                    // page since the PTE was made valid.  So trim the
                    // page instead of discarding it.
                    //
                    // Note the page cannot be a prototype because the
                    // PTE was transition above.
                    //

                    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

                    LOCK_PFN (OldIrql);

                    //
                    // Undo the reference count & locked page charge (if any).
                    //

                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                    UNLOCK_PFN (OldIrql);

                    MiTrimPte (MiGetVirtualAddressMappedByPte (PointerPte),
                               PointerPte,
                               Pfn1,
                               NULL,
                               ZeroPte);

                    //
                    // Release all locks so that other threads (like the
                    // working set trimmer) can try to freely make memory.
                    //

                    UNLOCK_WORKING_SET (CurrentThread, Vm);

                    KeDelayExecutionThread (KernelMode,
                                            FALSE,
                                            (PLARGE_INTEGER)&Mm30Milliseconds);

                    LOCK_WORKING_SET (CurrentThread, Vm);

                    LOCK_PFN (OldIrql);

                    //
                    // Retry the same page now.
                    //

                    continue;
                }

                LOCK_PFN (OldIrql);
            }
            else {

                //
                // The wsindex field must be zero because the
                // page is not in the system (or session) working set.
                //

                ASSERT (Pfn1->u1.WsIndex == 0);

                //
                // Adjust available pages as this page is now not in any
                // working set, just like a non-paged pool page.  On entry
                // this page was in transition so it was part of the
                // available pages by definition.
                //

                MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_LOCK_CODE3);

                if (Pfn1->u3.e1.PrototypePte == 0) {
                    InterlockedDecrement (&MmTotalSystemDriverPages);
                }
                if (SessionSpace == TRUE) {
                    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_LOCK_CODE1, 1);
                    InterlockedIncrementSizeT (&MmSessionSpace->NonPageablePages);
                }
            }
        }
        else {

            //
            // Page is not in memory.
            //

            MiMakeSystemAddressValidPfnSystemWs (
                    MiGetVirtualAddressMappedByPte(PointerPte), OldIrql);

            continue;
        }

        PointerPte += 1;
    } while (PointerPte <= LastPte);

    UNLOCK_PFN (OldIrql);

    UNLOCK_WORKING_SET (CurrentThread, Vm);

    return;
}


NTSTATUS
MmGetSectionRange (
    IN PVOID AddressWithinSection,
    OUT PVOID *StartingSectionAddress,
    OUT PULONG SizeofSection
    )
{
    ULONG Span;
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    NTSTATUS Status;
    ULONG_PTR Rva;

    PAGED_CODE();

    //
    // Search the loaded module list for the data table entry that describes
    // the DLL that was just unloaded. It is possible that an entry is not in
    // the list if a failure occurred at a point in loading the DLL just before
    // the data table entry was generated.
    //

    Status = STATUS_NOT_FOUND;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, TRUE);
    if (DataTableEntry) {

        Rva = (ULONG_PTR)((PUCHAR)AddressWithinSection - (ULONG_PTR)DataTableEntry->DllBase);

        NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (DataTableEntry->DllBase);
        if (NtHeaders == NULL) {
            Status = STATUS_NOT_FOUND;
            goto Finished;
        }

        NtSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                            sizeof(ULONG) +
                            sizeof(IMAGE_FILE_HEADER) +
                            NtHeaders->FileHeader.SizeOfOptionalHeader
                            );

        for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {

            //
            // Generally, SizeOfRawData is larger than VirtualSize for each
            // section because it includes the padding to get to the subsection
            // alignment boundary.  However if the image is linked with
            // subsection alignment == native page alignment, the linker will
            // have VirtualSize be much larger than SizeOfRawData because it
            // will account for all the bss.
            //

            Span = NtSection->SizeOfRawData;

            if (Span < NtSection->Misc.VirtualSize) {
                Span = NtSection->Misc.VirtualSize;
            }

            if ((Rva >= NtSection->VirtualAddress) &&
                (Rva < NtSection->VirtualAddress + Span)) {

                //
                // Found it.
                //

                *StartingSectionAddress = (PVOID)
                    ((PCHAR) DataTableEntry->DllBase + NtSection->VirtualAddress);
                *SizeofSection = Span;
                Status = STATUS_SUCCESS;
                break;
            }

            NtSection += 1;
        }
    }

Finished:

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);
    return Status;
}


PVOID
MmLockPageableDataSection (
    __in PVOID AddressWithinSection
    )

/*++

Routine Description:

    This functions locks the entire section that contains the specified
    section in memory.  This allows pageable code to be brought into
    memory and to be used as if the code was not really pageable.  This
    should not be done with a high degree of frequency.

Arguments:

    AddressWithinSection - Supplies the address of a function
        contained within a section that should be brought in and locked
        in memory.

Return Value:

    This function returns a value to be used in a subsequent call to
    MmUnlockPageableImageSection.

--*/

{
    ULONG Span;
    PLONG SectionLockCountPointer;
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    PIMAGE_SECTION_HEADER FoundSection;
    ULONG_PTR Rva;

    PAGED_CODE();

    if (MI_IS_PHYSICAL_ADDRESS(AddressWithinSection)) {

        //
        // Physical address, just return that as the handle.
        //

        return AddressWithinSection;
    }

    //
    // Search the loaded module list for the data table entry that describes
    // the DLL that was just unloaded. It is possible that an entry is not in
    // the list if a failure occurred at a point in loading the DLL just before
    // the data table entry was generated.
    //

    FoundSection = NULL;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, TRUE);

    Rva = (ULONG_PTR)((PUCHAR)AddressWithinSection - (ULONG_PTR)DataTableEntry->DllBase);

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (DataTableEntry->DllBase);

    if (NtHeaders == NULL) {

        //
        // This is a firmware entry - no one should be trying to lock these.
        //

        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1234,
                      (ULONG_PTR)AddressWithinSection,
                      1,
                      0);
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {

        //
        // Generally, SizeOfRawData is larger than VirtualSize for each
        // section because it includes the padding to get to the subsection
        // alignment boundary.  However, if the image is linked with
        // subsection alignment == native page alignment, the linker will
        // have VirtualSize be much larger than SizeOfRawData because it
        // will account for all the bss.
        //

        Span = NtSection->SizeOfRawData;

        if (Span < NtSection->Misc.VirtualSize) {
            Span = NtSection->Misc.VirtualSize;
        }

        if ((Rva >= NtSection->VirtualAddress) &&
            (Rva < NtSection->VirtualAddress + Span)) {

            FoundSection = NtSection;

            if (SECTION_BASE_ADDRESS(NtSection) != ((PUCHAR)DataTableEntry->DllBase +
                            NtSection->VirtualAddress)) {

                //
                // Overwrite the PointerToRelocations field (and on Win64, the
                // PointerToLinenumbers field also) so that it contains
                // the Va of this section.
                //
                // NumberOfRelocations & NumberOfLinenumbers contains
                // the Lock Count for the section.
                //

                SECTION_BASE_ADDRESS(NtSection) = ((PUCHAR)DataTableEntry->DllBase +
                                        NtSection->VirtualAddress);

                SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);
                *SectionLockCountPointer = 0;
            }

            //
            // Now lock in the code.
            //

            MmLockPageableSectionByHandle ((PVOID)NtSection);

            break;
        }
        NtSection += 1;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);
    if (!FoundSection) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1234,
                      (ULONG_PTR)AddressWithinSection,
                      0,
                      0);
    }
    return (PVOID)FoundSection;
}


PKLDR_DATA_TABLE_ENTRY
MiLookupDataTableEntry (
    IN PVOID AddressWithinSection,
    IN ULONG ResourceHeld
    )

/*++

Routine Description:

    This functions locates the data table entry that maps the specified address.

Arguments:

    AddressWithinSection - Supplies the address of a function contained
                           within the desired module.

    ResourceHeld - Supplies TRUE if the loaded module resource is already held,
                   FALSE if not.

Return Value:

    The address of the loaded module list data table entry that maps the
    argument address.

--*/

{
    PKTHREAD CurrentThread;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY FoundEntry;
    PLIST_ENTRY NextEntry;

    PAGED_CODE();

    FoundEntry = NULL;

    //
    // Search the loaded module list for the data table entry that describes
    // the DLL that was just unloaded. It is possible that an entry is not in
    // the list if a failure occurred at a point in loading the DLL just before
    // the data table entry was generated.
    //

    if (!ResourceHeld) {
        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);
    }
    else {
        CurrentThread = NULL;
    }

    NextEntry = PsLoadedModuleList.Flink;
    ASSERT (NextEntry != NULL);

    do {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        //
        // Locate the loaded module that contains this address.
        //

        if ( AddressWithinSection >= DataTableEntry->DllBase &&
             AddressWithinSection < (PVOID)((PUCHAR)DataTableEntry->DllBase+DataTableEntry->SizeOfImage) ) {

            FoundEntry = DataTableEntry;
            break;
        }

        NextEntry = NextEntry->Flink;
    } while (NextEntry != &PsLoadedModuleList);

    if (CurrentThread != NULL) {
        ExReleaseResourceLite (&PsLoadedModuleResource);
        KeLeaveCriticalRegionThread (CurrentThread);
    }
    return FoundEntry;
}

VOID
MmUnlockPagableImageSection (
    __in PVOID ImageSectionHandle
    )
{
    MmUnlockPageableImageSection ( ImageSectionHandle );
}

VOID
MmUnlockPageableImageSection (
    __in PVOID ImageSectionHandle
    )

/*++

Routine Description:

    This function unlocks from memory, the pages locked by a preceding call to
    MmLockPageableDataSection.

Arguments:

    ImageSectionHandle - Supplies the value returned by a previous call
                         to MmLockPageableDataSection.

Return Value:

    None.

--*/

{
    PKTHREAD CurrentThread;
    PIMAGE_SECTION_HEADER NtSection;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PVOID BaseAddress;
    ULONG SizeToUnlock;
    ULONG Count;
    PLONG SectionLockCountPointer;

    if (MI_IS_PHYSICAL_ADDRESS(ImageSectionHandle)) {

        //
        // No need to unlock physical addresses.
        //

        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)ImageSectionHandle;

    //
    // Address must be in the system working set.
    //

    BaseAddress = SECTION_BASE_ADDRESS(NtSection);
    SectionLockCountPointer = SECTION_LOCK_COUNT_POINTER (NtSection);
    SizeToUnlock = NtSection->SizeOfRawData;

    //
    // Generally, SizeOfRawData is larger than VirtualSize for each
    // section because it includes the padding to get to the subsection
    // alignment boundary.  However, if the image is linked with
    // subsection alignment == native page alignment, the linker will
    // have VirtualSize be much larger than SizeOfRawData because it
    // will account for all the bss.
    //

    if (SizeToUnlock < NtSection->Misc.VirtualSize) {
        SizeToUnlock = NtSection->Misc.VirtualSize;
    }

    PointerPte = MiGetPteAddress(BaseAddress);
    LastPte = MiGetPteAddress((PCHAR)BaseAddress + SizeToUnlock - 1);

    CurrentThread = KeGetCurrentThread ();

    //
    // Block user APCs as the initial decrement below could push the count to 1.
    // This puts this thread into the critical path that must finish as all
    // other threads trying to lock the section will be waiting for this thread.
    // Entering a critical region here ensures that a suspend cannot stop us.
    //

    KeEnterCriticalRegionThread (CurrentThread);

    Count = InterlockedDecrement (SectionLockCountPointer);
    
    if (Count < 1) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x1010,
                      (ULONG_PTR)BaseAddress,
                      (ULONG_PTR)NtSection,
                      *SectionLockCountPointer);
    }

    if (Count != 1) {
        KeLeaveCriticalRegionThread (CurrentThread);
        return;
    }

    LOCK_PFN2 (OldIrql);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e2.ReferenceCount > 1);

        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

        PointerPte += 1;

    } while (PointerPte <= LastPte);

    UNLOCK_PFN2 (OldIrql);

    ASSERT (*SectionLockCountPointer == 1);
    Count = InterlockedDecrement (SectionLockCountPointer);
    ASSERT (Count == 0);

    if (MmCollidedLockWait != 0) {
        KePulseEvent (&MmCollidedLockEvent, 0, FALSE);
    }

    //
    // Enable user APCs now that the pulse has occurred.  They had to be
    // blocked to prevent any suspensions of this thread as that would
    // stop all waiters indefinitely.
    //

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}


BOOLEAN
MmIsRecursiveIoFault (
    VOID
    )

/*++

Routine Description:

    This function examines the thread's page fault clustering information
    and determines if the current page fault is occurring during an I/O
    operation.

Arguments:

    None.

Return Value:

    Returns TRUE if the fault is occurring during an I/O operation,
    FALSE otherwise.

--*/

{
    PETHREAD Thread;

    Thread = PsGetCurrentThread ();

    return (BOOLEAN)(Thread->DisablePageFaultClustering |
                     Thread->ForwardClusterOnly);
}


VOID
MmMapMemoryDumpMdl (
    __inout PMDL MemoryDumpMdl
    )

/*++

Routine Description:

    For use by crash dump routine ONLY.  Maps an MDL into a fixed
    portion of the address space.  Only 1 MDL can be mapped at a
    time.

Arguments:

    MemoryDumpMdl - Supplies the MDL to map.

Return Value:

    None, fields in MDL updated.

--*/

{
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    PCHAR BaseVa;
    MMPTE TempPte;
    PMMPFN Pfn1;
    PPFN_NUMBER Page;

    NumberOfPages = BYTES_TO_PAGES (MemoryDumpMdl->ByteCount + MemoryDumpMdl->ByteOffset);

    ASSERT (NumberOfPages <= 16);

    PointerPte = MmCrashDumpPte;
    BaseVa = (PCHAR)MiGetVirtualAddressMappedByPte(PointerPte);
    MemoryDumpMdl->MappedSystemVa = (PCHAR)BaseVa + MemoryDumpMdl->ByteOffset;
    TempPte = ValidKernelPte;
    Page = (PPFN_NUMBER)(MemoryDumpMdl + 1);

    //
    // If the pages don't span the entire dump virtual address range,
    // build a barrier.  Otherwise use the default barrier provided at the
    // end of the dump virtual address range.
    //

    if (NumberOfPages < 16) {
        MI_WRITE_ZERO_PTE (PointerPte + NumberOfPages);
        KiFlushSingleTb (BaseVa + (NumberOfPages << PAGE_SHIFT));
    }

    do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
        Pfn1 = MI_PFN_ELEMENT (*Page);
        TempPte = ValidKernelPte;

        switch (Pfn1->u3.e1.CacheAttribute) {
            case MiCached:
                break;

            case MiNonCached:
                MI_DISABLE_CACHING (TempPte);
                break;

            case MiWriteCombined:
                MI_SET_PTE_WRITE_COMBINE (TempPte);
                break;

            default:
                break;
        }

        TempPte.u.Hard.PageFrameNumber = *Page;

        //
        // Note this PTE may be valid or invalid prior to the overwriting here.
        //

        if (PointerPte->u.Hard.Valid == 1) {
            if (PointerPte->u.Long != TempPte.u.Long) {
                MI_WRITE_VALID_PTE_NEW_PAGE (PointerPte, TempPte);
                KiFlushSingleTb (BaseVa);
            }
        }
        else {
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
        }

        Page += 1;
        PointerPte += 1;
        BaseVa += PAGE_SIZE;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    return;
}


VOID
MmReleaseDumpAddresses (
    IN PFN_NUMBER NumberOfPages
    )

/*++

Routine Description:

    For use by hibernate routine ONLY.  Puts zeros back into the
    used dump PTEs.

Arguments:

    NumberOfPages - Supplies the number of PTEs to zero.

Return Value:

    None.

--*/

{
    PVOID BaseVa;
    PMMPTE PointerPte;

    PointerPte = MmCrashDumpPte;
    BaseVa = MiGetVirtualAddressMappedByPte (PointerPte);

    MiZeroMemoryPte (MmCrashDumpPte, NumberOfPages);

    while (NumberOfPages != 0) {

        KiFlushSingleTb (BaseVa);

        BaseVa = (PVOID) ((PCHAR) BaseVa + PAGE_SIZE);
        NumberOfPages -= 1;
    }
}


NTSTATUS
MmSetBankedSection (
    __in HANDLE ProcessHandle,
    __in_bcount(BankLength) PVOID VirtualAddress,
    __in ULONG BankLength,
    __in BOOLEAN ReadWriteBank,
    __in PBANKED_SECTION_ROUTINE BankRoutine,
    __in PVOID Context
    )

/*++

Routine Description:

    This function declares a mapped video buffer as a banked
    section.  This allows banked video devices (i.e., even
    though the video controller has a megabyte or so of memory,
    only a small bank (like 64k) can be mapped at any one time.

    In order to overcome this problem, the pager handles faults
    to this memory, unmaps the current bank, calls off to the
    video driver and then maps in the new bank.

    This function creates the necessary structures to allow the
    video driver to be called from the pager.

 ********************* NOTE NOTE NOTE *************************
    At this time only read/write banks are supported!

Arguments:

    ProcessHandle - Supplies a handle to the process in which to
                    support the banked video function.

    VirtualAddress - Supplies the virtual address where the video
                     buffer is mapped in the specified process.

    BankLength - Supplies the size of the bank.

    ReadWriteBank - Supplies TRUE if the bank is read and write.

    BankRoutine - Supplies a pointer to the routine that should be
                  called by the pager.

    Context - Supplies a context to be passed by the pager to the BankRoutine.

Return Value:

    Returns the status of the function.

Environment:

    Kernel mode, APC_LEVEL or below.

--*/

{
    KAPC_STATE ApcState;
    NTSTATUS Status;
    PETHREAD Thread;
    PEPROCESS Process;
    PMMVAD Vad;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    ULONG_PTR size;
    ULONG count;
    ULONG NumberOfPtes;
    PMMBANKED_SECTION Bank;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (ReadWriteBank);

    //
    // Reference the specified process handle for VM_OPERATION access.
    //

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        KernelMode,
                                        (PVOID *)&Process,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Thread = PsGetCurrentThread ();

    KeStackAttachProcess (&Process->Pcb, &ApcState);

    //
    // Get the address creation mutex to block multiple threads from
    // creating or deleting address space at the same time and
    // get the working set mutex so virtual address descriptors can
    // be inserted and walked.  Block APCs so an APC which takes a page
    // fault does not corrupt various structures.
    //

    LOCK_ADDRESS_SPACE (Process);

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    Vad = MiLocateAddress (VirtualAddress);

    if ((Vad == NULL) ||
        (Vad->StartingVpn != MI_VA_TO_VPN (VirtualAddress)) ||
        (Vad->u.VadFlags.VadType != VadDevicePhysicalMemory)) {
        Status = STATUS_NOT_MAPPED_DATA;
        goto ErrorReturn;
    }

    size = PAGE_SIZE + ((Vad->EndingVpn - Vad->StartingVpn) << PAGE_SHIFT);
    if ((size % BankLength) != 0) {
        Status = STATUS_INVALID_VIEW_SIZE;
        goto ErrorReturn;
    }

    count = (ULONG) -1;
    NumberOfPtes = BankLength;

    do {
        NumberOfPtes >>= 1;
        count += 1;
    } while (NumberOfPtes != 0);

    //
    // Turn VAD into a banked VAD.
    //

    NumberOfPtes = BankLength >> PAGE_SHIFT;

    Bank = ExAllocatePoolWithTag (NonPagedPool,
                                  sizeof (MMBANKED_SECTION) +
                                       (NumberOfPtes - 1) *  sizeof(MMPTE),
                                  'kBmM');

    if (Bank == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn;
    }

    Bank->BankShift = PTE_SHIFT + count - PAGE_SHIFT;

    PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
    ASSERT (PointerPte->u.Hard.Valid == 1);

    Bank->BasePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    Bank->BasedPte = PointerPte;
    Bank->BankSize = BankLength;
    Bank->BankedRoutine = BankRoutine;
    Bank->Context = Context;
    Bank->CurrentMappedPte = PointerPte;

    //
    // Build the template PTEs structure.
    //

    MI_MAKE_VALID_USER_PTE (TempPte,
                            Bank->BasePhysicalPage,
                            MM_READWRITE,
                            PointerPte);

    if (TempPte.u.Hard.Write) {
        MI_SET_PTE_DIRTY (TempPte);
    }

    count = 0;

    do {
        Bank->BankTemplate[count] = TempPte;
        TempPte.u.Hard.PageFrameNumber += 1;
        count += 1;
    } while (count < NumberOfPtes);

    LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));

    //
    // Set all PTEs within this range to zero.  Any faults within
    // this range will call the banked routine before making the
    // page valid.
    //

    LOCK_WS_UNSAFE (Thread, Process);

    ((PMMVAD_LONG) Vad)->u4.Banked = Bank;

    MiZeroMemoryPte (PointerPte, size >> PAGE_SHIFT);

    MI_FLUSH_PROCESS_TB (FALSE);

    UNLOCK_WS_UNSAFE (Thread, Process);

    Status = STATUS_SUCCESS;

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);
    KeUnstackDetachProcess (&ApcState);
    ObDereferenceObject (Process);
    return Status;
}

__out_bcount(NumberOfBytes) PVOID
MmMapVideoDisplay (
     __in PHYSICAL_ADDRESS PhysicalAddress,
     __in SIZE_T NumberOfBytes,
     __in MEMORY_CACHING_TYPE CacheType
     )

/*++

Routine Description:

    This function maps the specified physical address into the non-pageable
    portion of the system address space.

Arguments:

    PhysicalAddress - Supplies the starting physical address to map.

    NumberOfBytes - Supplies the number of bytes to map.

    CacheType - Supplies MmNonCached if the physical address is to be mapped
                as non-cached, MmCached if the address should be cached, and
                MmWriteCombined if the address should be cached and
                write-combined as a frame buffer. For I/O device registers,
                this is usually specified as MmNonCached.

Return Value:

    Returns the virtual address which maps the specified physical addresses.
    The value NULL is returned if sufficient virtual address space for
    the mapping could not be found.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PAGED_CODE();

    return MmMapIoSpace (PhysicalAddress, NumberOfBytes, CacheType);
}

VOID
MmUnmapVideoDisplay (
     __in_bcount(NumberOfBytes) PVOID BaseAddress,
     __in SIZE_T NumberOfBytes
     )

/*++

Routine Description:

    This function unmaps a range of physical address which were previously
    mapped via an MmMapVideoDisplay function call.

Arguments:

    BaseAddress - Supplies the base virtual address where the physical
                  address was previously mapped.

    NumberOfBytes - Supplies the number of bytes which were mapped.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    MmUnmapIoSpace (BaseAddress, NumberOfBytes);
    return;
}


VOID
MmLockPagedPool (
    IN PVOID Address,
    IN SIZE_T SizeInBytes
    )

/*++

Routine Description:

    Locks the specified address (which MUST reside in paged pool) into
    memory until MmUnlockPagedPool is called.

Arguments:

    Address - Supplies the address in paged pool to lock.

    SizeInBytes - Supplies the size in bytes to lock.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PMMPTE PointerPte;
    PMMPTE LastPte;

    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + (SizeInBytes - 1)));

    MiLockCode (PointerPte, LastPte, MM_LOCK_BY_REFCOUNT);

    return;
}

NTKERNELAPI
VOID
MmUnlockPagedPool (
    IN PVOID Address,
    IN SIZE_T SizeInBytes
    )

/*++

Routine Description:

    Unlocks paged pool that was locked with MmLockPagedPool.

Arguments:

    Address - Supplies the address in paged pool to unlock.

    Size - Supplies the size to unlock.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;

    PointerPte = MiGetPteAddress (Address);
    LastPte = MiGetPteAddress ((PVOID)((PCHAR)Address + (SizeInBytes - 1)));

    MmLockPageableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e2.ReferenceCount > 1);

        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

        PointerPte += 1;
    } while (PointerPte <= LastPte);

    UNLOCK_PFN (OldIrql);
    MmUnlockPageableImageSection (ExPageLockHandle);
    return;
}

NTKERNELAPI
ULONG
MmGatherMemoryForHibernate (
    IN PMDL Mdl,
    IN BOOLEAN Wait
    )

/*++

Routine Description:

    Finds enough memory to fill in the pages of the MDL for power management
    hibernate function.

Arguments:

    Mdl - Supplies an MDL, the start VA field should be NULL.  The length
          field indicates how many pages to obtain.

    Wait - FALSE to fail immediately if the pages aren't available.

Return Value:

    TRUE if the MDL could be filled in, FALSE otherwise.

Environment:

    Kernel mode, IRQL of DISPATCH_LEVEL or below.

--*/

{
    PMMPFNLIST ListHead;
    KIRQL OldIrql;
    PFN_NUMBER AvailablePages;
    PFN_NUMBER PagesNeeded;
    PPFN_NUMBER Pages;
    PFN_NUMBER i;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    ULONG status;
    PKTHREAD CurrentThread;

    status = FALSE;

    PagesNeeded = Mdl->ByteCount >> PAGE_SHIFT;
    Pages = (PPFN_NUMBER)(Mdl + 1);

    i = Wait ? 100 : 1;

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    InterlockedIncrement (&MiDelayPageFaults);

    do {

        LOCK_PFN2 (OldIrql);

        MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

        //
        // Don't use MmAvailablePages here because if compression hardware is
        // being used we would bail prematurely.  Check the lists explicitly
        // in order to provide our caller with the maximum number of pages.
        //

        AvailablePages = MmZeroedPageListHead.Total + MmFreePageListHead.Total;

        for (ListHead = &MmStandbyPageListByPriority[0];
             ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
             ListHead += 1) {

            AvailablePages += ListHead->Total;
        }

        if (AvailablePages > PagesNeeded) {

            //
            // Fill in the MDL.
            //

            do {
                PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (NULL));
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
#if DBG
                Pfn1->PteAddress = (PVOID) (ULONG_PTR)X64K;
#endif
                MI_SET_PFN_DELETED (Pfn1);
                Pfn1->u3.e2.ReferenceCount += 1;
                Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
                *Pages = PageFrameIndex;
                Pages += 1;
                PagesNeeded -= 1;
            } while (PagesNeeded);
            UNLOCK_PFN2 (OldIrql);
            Mdl->MdlFlags |= MDL_PAGES_LOCKED;
            status = TRUE;
            break;
        }

        UNLOCK_PFN2 (OldIrql);

        //
        // If we're being called at DISPATCH_LEVEL we cannot move pages to
        // the standby list because mutexes must be acquired to do so.
        //

        if (OldIrql > APC_LEVEL) {
            break;
        }

        if (!i) {
            break;
        }

        //
        // Attempt to move pages to the standby list.
        //

        MmEmptyAllWorkingSets ();
        MiFlushAllPages();

        KeDelayExecutionThread (KernelMode,
                                FALSE,
                                (PLARGE_INTEGER)&Mm30Milliseconds);
        i -= 1;

    } while (TRUE);

    InterlockedDecrement (&MiDelayPageFaults);

    KeLeaveCriticalRegionThread (CurrentThread);

    return status;
}

NTKERNELAPI
VOID
MmReturnMemoryForHibernate (
    IN PMDL Mdl
    )

/*++

Routine Description:

    Returns memory from MmGatherMemoryForHibername.

Arguments:

    Mdl - Supplies an MDL, the start VA field should be NULL.  The length
          field indicates how many pages to obtain.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PPFN_NUMBER Pages;
    PPFN_NUMBER LastPage;

    Pages = (PPFN_NUMBER)(Mdl + 1);
    LastPage = Pages + (Mdl->ByteCount >> PAGE_SHIFT);

    LOCK_PFN2 (OldIrql);

    do {
        Pfn1 = MI_PFN_ELEMENT (*Pages);
        MiDecrementReferenceCount (Pfn1, *Pages);
        Pages += 1;
    } while (Pages < LastPage);

    UNLOCK_PFN2 (OldIrql);
    return;
}


VOID
MmEnablePAT (
     VOID
     )

/*++

Routine Description:

    This routine enables the page attribute capability for individual PTE
    mappings.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.

--*/
{
    ULONG i;
    MMPTE PteContents;

    MiWriteCombiningPtes = TRUE;

    //
    // The attribute table is statically initialized with write combining being
    // equivalent to noncached.  Once all the processors have booted as we don't
    // know if they will all have a PAT (or equivalent functionality) until
    // then, we can override the PTE maps as below.
    //
    // Note if the machine cannot support all the attribute types to main
    // memory, then the MmProtectToPteMask table will already have been
    // modified in MiInitializeCacheOverrides.  If so, then nothing more
    // needs to be done here.
    //

    if (MiAllMainMemoryMustBeCached == TRUE) {

        //
        // The table has already been updated.
        //

        return;
    }

    for (i = MM_WRITECOMBINE + 1; i < 32; i += 1) {
        PteContents.u.Long = MmProtectToPteMask[i];
        MI_SET_PTE_WRITE_COMBINE (PteContents);
        MmProtectToPteMask[i] = PteContents.u.Long;
    }
}

LOGICAL
MmIsSystemAddressLocked (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This routine determines whether the specified system address is currently
    locked.

    This routine should only be called for debugging purposes, as it is not
    guaranteed upon return to the caller that the address is still locked.
    (The address could easily have been trimmed prior to return).

Arguments:

    VirtualAddress - Supplies the virtual address to check.

Return Value:

    TRUE if the address is locked.  FALSE if not.

Environment:

    DISPATCH LEVEL or below.  No memory management locks may be held.

--*/
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    if (IS_SYSTEM_ADDRESS (VirtualAddress) == FALSE) {
        return FALSE;
    }

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {
        return TRUE;
    }

    //
    // Hyperspace and page maps are not treated as locked down.
    //

    if (MI_IS_PROCESS_SPACE_ADDRESS (VirtualAddress) == TRUE) {
        return FALSE;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);

    LOCK_PFN2 (OldIrql);

    if (MiIsAddressValid (VirtualAddress, TRUE) == FALSE) {
        UNLOCK_PFN2 (OldIrql);
        return FALSE;
    }

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    //
    // Note that the mapped page may not be in the PFN database.  Treat
    // this as locked.
    //

    if (!MI_IS_PFN (PageFrameIndex)) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    //
    // Check for the page being locked by reference.
    //

    if (Pfn1->u3.e2.ReferenceCount > 1) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    if (Pfn1->u3.e2.ReferenceCount > Pfn1->u2.ShareCount) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    //
    // Check whether the page is locked into the working set.
    //

    if (Pfn1->u1.Event == NULL) {
        UNLOCK_PFN2 (OldIrql);
        return TRUE;
    }

    UNLOCK_PFN2 (OldIrql);

    return FALSE;
}

LOGICAL
MmAreMdlPagesLocked (
    IN PMDL MemoryDescriptorList
    )

/*++

Routine Description:

    This routine determines whether the pages described by the argument
    MDL are currently locked.

    This routine should only be called for debugging purposes, as it is not
    guaranteed upon return to the caller that the pages are still locked.

Arguments:

    MemoryDescriptorList - Supplies the memory descriptor list to check.

Return Value:

    TRUE if ALL the pages described by the argument MDL are locked.
    FALSE if not.

Environment:

    DISPATCH LEVEL or below.  No memory management locks may be held.

--*/
{
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PVOID VirtualAddress;
    PMMPFN Pfn1;
    KIRQL OldIrql;

    //
    // We'd like to assert that MDL_PAGES_LOCKED is set but can't because
    // some drivers have privately constructed MDLs and they never set the
    // bit properly.
    //

    if ((MemoryDescriptorList->MdlFlags & (MDL_IO_SPACE | MDL_SOURCE_IS_NONPAGED_POOL)) != 0) {
        return TRUE;
    }

    VirtualAddress = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (VirtualAddress,
                                              MemoryDescriptorList->ByteCount);

    Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    LOCK_PFN2 (OldIrql);

    do {

        if (*Page == MM_EMPTY_LIST) {

            //
            // There are no more locked pages.
            //

            break;
        }

        //
        // Note that the mapped page may not be in the PFN database.  Treat
        // this as locked.
        //

        if (MI_IS_PFN (*Page)) {

            Pfn1 = MI_PFN_ELEMENT (*Page);

            //
            // Check for the page being locked by reference
            //
            // - or -
            //
            // whether the page is locked into the working set.
            //
        
            if ((Pfn1->u3.e2.ReferenceCount <= Pfn1->u2.ShareCount) &&
                (Pfn1->u3.e2.ReferenceCount <= 1) &&
                (Pfn1->u1.Event != NULL)) {

                //
                // The page is not locked by reference or in a working set,
                // see if it is in nonpaged pool.
                //
    
                if ((Pfn1->u3.e1.PageLocation == ActiveAndValid) &&
                    (*Page >= MiStartOfInitialPoolFrame) &&
                    (*Page <= MiEndOfInitialPoolFrame)) {

                    //
                    // This is initial nonpaged pool.
                    //

                    NOTHING;
                }
                else {
                    VirtualAddress = MiGetVirtualAddressMappedByPte (Pfn1->PteAddress);
                    if ((VirtualAddress >= MmNonPagedPoolExpansionStart) &&
                        (VirtualAddress < MmNonPagedPoolEnd)) {

                        //
                        // This is expansion nonpaged pool.
                        //

                        NOTHING;
                    }
                    else if ((VirtualAddress >= MmSpecialPoolStart) &&
                             (VirtualAddress < MmSpecialPoolEnd) &&
                             (MmSpecialPoolStart != NULL) &&
                             ((MmQuerySpecialPoolBlockType (VirtualAddress) & 
                                BASE_POOL_TYPE_MASK) == NonPagedPool)) {
                        NOTHING;
                    }
                    else {
                        UNLOCK_PFN2 (OldIrql);
                        return FALSE;
                    }
                }
            }
        }

        Page += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    UNLOCK_PFN2 (OldIrql);

    return TRUE;
}

