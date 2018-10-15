/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   forksup.c

Abstract:

    This module contains the routines which support the POSIX fork operation.

--*/

#include "mi.h"

VOID
MiUpControlAreaRefs (
    IN PMMVAD Vad
    );

ULONG
MiDoneWithThisPageGetAnother (
    IN PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    );

ULONG
MiLeaveThisPageGetAnother (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    );

VOID
MiUpForkPageShareCount (
    IN PMMPFN PfnForkPtePage
    );

ULONG
MiHandleForkTransitionPte (
    IN PMMPTE PointerPte,
    IN PMMPTE PointerNewPte,
    IN PMMCLONE_BLOCK ForkProtoPte
    );

VOID
MiDownShareCountFlushEntireTb (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiBuildForkPageTable (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PMMPTE PointerNewPde,
    IN PFN_NUMBER PdePhysicalPage,
    IN PMMPFN PfnPdPage,
    IN LOGICAL MakeValid
    );

#define MM_FORK_SUCCEEDED 0
#define MM_FORK_FAILED 1


NTSTATUS
MiCloneProcessAddressSpace (
    IN PEPROCESS ProcessToClone,
    IN PEPROCESS ProcessToInitialize
    )

/*++

Routine Description:

    This routine stands on its head to produce a copy of the specified
    process's address space in the process to initialize.  This
    is done by examining each virtual address descriptor's inherit
    attributes.  If the pages described by the VAD should be inherited,
    each PTE is examined and copied into the new address space.

    For private pages, fork prototype PTEs are constructed and the pages
    become shared, copy-on-write, between the two processes.


Arguments:

    ProcessToClone - Supplies the process whose address space should be
                     cloned.

    ProcessToInitialize - Supplies the process whose address space is to
                          be created.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled.  No VADs exist for the new process on entry
    and the new process is not on the working set expansion links list so
    it cannot be trimmed (or outswapped).  Hence none of the pages in the
    new process need to be locked down.

--*/

{
    PMMCLONE_BLOCK TempCloneMapping;
    PETHREAD CurrentThread;
    PAWEINFO AweInfo;
    PVOID RestartKey;
    PFN_NUMBER PdePhysicalPage;
    PEPROCESS CurrentProcess;
    PMMPTE PdeBase;
    PMMCLONE_HEADER CloneHeader;
    PMMCLONE_BLOCK CloneProtos;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    PMM_AVL_TABLE CloneRoot;
    PMM_AVL_TABLE TargetCloneRoot;
    PFN_NUMBER RootPhysicalPage;
    PMMVAD NewVad;
    PMMVAD Vad;
    PMMVAD NextVad;
    PMMVAD *VadList;
    PMMVAD FirstNewVad;
    PMMCLONE_DESCRIPTOR *CloneList;
    PMMCLONE_DESCRIPTOR FirstNewClone;
    PMMCLONE_DESCRIPTOR Clone;
    PMMCLONE_DESCRIPTOR NextClone;
    PMMCLONE_DESCRIPTOR NewClone;
    ULONG Attached;
    WSLE_NUMBER WorkingSetIndex;
    PVOID VirtualAddress;
    NTSTATUS status;
    NTSTATUS status2;
    PMMPFN Pfn2;
    PMMPFN PfnPdPage;
    MMPTE TempPte;
    MMPTE PteContents;
    KAPC_STATE ApcState;
    ULONG i;
#if defined (_X86PAE_)
    PMDL MdlPageDirectory;
    PFN_NUMBER PageDirectoryFrames[PD_PER_SYSTEM];
    PFN_NUMBER MdlHackPageDirectory[(sizeof(MDL)/sizeof(PFN_NUMBER)) + PD_PER_SYSTEM];
#else
    PFN_NUMBER MdlDirPage;
#endif
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerNewPte;
    PMMPTE NewPteMappedAddress;
    PMMPTE PointerNewPde;
    PMI_PHYSICAL_VIEW PhysicalView;
    PMI_PHYSICAL_VIEW NextPhysicalView;
    PMI_PHYSICAL_VIEW PhysicalViewList;
    ULONG PhysicalViewCount;
    PFN_NUMBER PageFrameIndex;
    PMMCLONE_BLOCK ForkProtoPte;
    PMMCLONE_BLOCK CloneProto;
    PMMPTE ContainingPte;
    ULONG NumberOfForkPtes;
    PFN_NUMBER NumberOfPrivatePages;
    SIZE_T TotalPagedPoolCharge;
    SIZE_T TotalNonPagedPoolCharge;
    PMMPFN PfnForkPtePage;
    PVOID UsedPageTableEntries;
    ULONG ReleasedWorkingSetMutex;
    ULONG FirstTime;
    ULONG Waited;
    ULONG PpePdeOffset;
    PFN_NUMBER HyperPhysicalPage;
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpeLast;
    PFN_NUMBER PageDirFrameIndex;
    PVOID UsedPageDirectoryEntries;
    PMMPTE PointerNewPpe;
    PMMPTE PpeBase;
    PMMPFN PfnPpPage;
#if (_MI_PAGING_LEVELS >= 4)
    PVOID UsedPageDirectoryParentEntries;
    PFN_NUMBER PpePhysicalPage;
    PFN_NUMBER PageParentFrameIndex;
    PMMPTE PointerNewPxe;
    PMMPTE PxeBase;
    PMMPFN PfnPxPage;
    PFN_NUMBER MdlDirParentPage;
#endif

#else
    PMMWSL HyperBase = NULL;
    PMMWSL HyperWsl;
#endif

    PAGED_CODE();

    HyperPhysicalPage = ProcessToInitialize->WorkingSetPage;
    NumberOfForkPtes = 0;
    Attached = FALSE;
    PageFrameIndex = (PFN_NUMBER)-1;
    PhysicalViewList = NULL;
    PhysicalViewCount = 0;
    FirstNewVad = NULL;

    CloneHeader = NULL;
    NumberOfPrivatePages = 0;
    CloneDescriptor = NULL;
    CloneRoot = NULL;
    TargetCloneRoot = NULL;

    CurrentThread = PsGetCurrentThread ();

    if (ProcessToClone != PsGetCurrentProcessByThread (CurrentThread)) {
        Attached = TRUE;
        KeStackAttachProcess (&ProcessToClone->Pcb, &ApcState);
    }

#if defined (_X86PAE_)
    PointerPte = (PMMPTE) ProcessToInitialize->PaeTop;

    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        PageDirectoryFrames[i] = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        PointerPte += 1;
    }

    RootPhysicalPage = PageDirectoryFrames[PD_PER_SYSTEM - 1];
#else
    RootPhysicalPage = ProcessToInitialize->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#endif

    CurrentProcess = ProcessToClone;
    CloneProtos = NULL;

    //
    // Get the working set mutex and the address creation mutex
    // of the process to clone.  This prevents page faults while we
    // are examining the address map and prevents virtual address space
    // from being created or deleted.
    //

    LOCK_ADDRESS_SPACE (CurrentProcess);

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (CurrentProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn1;
    }

    //
    // Attempt to acquire the needed pool before starting the
    // clone operation, this allows an easier failure path in
    // the case of insufficient system resources.  The working set mutex
    // must be acquired (and held throughout) to protect against modifications
    // to the NumberOfPrivatePages field in the EPROCESS.
    //
    // In addition since paged pool cannot be allocated or accessed while
    // holding the working set pushlock (unless the pool is locked down),
    // allocate it and lock it down now.
    //

RetryPrivatePageAllocations:

    NumberOfPrivatePages = CurrentProcess->NumberOfPrivatePages;

    if (NumberOfPrivatePages == 0) {
        NumberOfPrivatePages = 1;
    }

    //
    // Charge the current process the quota for the paged and nonpaged
    // global structures.  This consists of the array of clone blocks
    // in paged pool and the clone header in non-paged pool.
    //

    status = PsChargeProcessPagedPoolQuota (CurrentProcess,
                                            sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);

    if (!NT_SUCCESS (status)) {

        //
        // Unable to charge quota for the clone blocks.
        //

        goto ErrorReturn1;
    }

    status = PsChargeProcessNonPagedPoolQuota (CurrentProcess,
                                               sizeof(MMCLONE_HEADER));

    if (!NT_SUCCESS (status)) {

        //
        // Unable to charge quota for the clone header.
        //

        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        goto ErrorReturn1;
    }

    //
    // Got the quotas, now get the pool.
    //

    CloneProtos = ExAllocatePoolWithTag (PagedPool,
                                         sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages,
                                         'lCmM');
    if (CloneProtos == NULL) {
        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        status = STATUS_INSUFFICIENT_RESOURCES;
        CloneProtos = NULL;
        goto ErrorReturn1;
    }

    //
    // Lock down the paged pool since we will be holding the working set
    // pushlock and cannot fault on it when this is held without potentially
    // deadlocking.
    //

    for (i = 0; i < sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages; i += PAGE_SIZE) {
        MiLockPagedAddress ((PVOID)((PCHAR) CloneProtos + i));
    }

    if (MI_NONPAGEABLE_MEMORY_AVAILABLE() < 100) {
        ExFreePool (CloneProtos);
        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        status = STATUS_INSUFFICIENT_RESOURCES;
        CloneProtos = NULL;
        goto ErrorReturn1;
    }

    LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

    if (CurrentProcess->NumberOfPrivatePages > NumberOfPrivatePages) {

        UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

        //
        // Unlock and free the paged pool.
        //

        for (i = 0; i < sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages; i += PAGE_SIZE) {
            MiUnlockPagedAddress ((PVOID)((PCHAR) CloneProtos + i));
        }

        ExFreePool (CloneProtos);

        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        CloneProtos = NULL;
        goto RetryPrivatePageAllocations;
    }

    //
    // Check for AWE, rotate physical, write watch and large page regions
    // as they are not duplicated so fork is not allowed.
    //

    if (CurrentProcess->PhysicalVadRoot != NULL) {

        RestartKey = NULL;

        //
        // Don't allow cloning of any processes with physical VADs of
        // any sort.  This is to prevent drivers that map nonpaged pool
        // into user space from creating security/corruption holes if
        // the application clones because drivers wouldn't know how
        // to clean up (destroy) all the views prior to freeing the pool.
        //

        if (MiEnumerateGenericTableWithoutSplayingAvl (CurrentProcess->PhysicalVadRoot, &RestartKey) != NULL) {
            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
            status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn1;
        }
    }

    AweInfo = (PAWEINFO) CurrentProcess->AweInfo;

    if (AweInfo != NULL) {
        RestartKey = NULL;

        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (&AweInfo->AweVadRoot, &RestartKey);

            if (PhysicalView == NULL) {
                break;
            }

            if (PhysicalView->VadType != VadDevicePhysicalMemory) {
                UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
                status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn1;
            }

        } while (TRUE);
    }

    ASSERT (CurrentProcess->ForkInProgress == NULL);

    //
    // Indicate to the pager that the current process is being
    // forked.  This blocks other threads in that process from
    // modifying clone block counts and contents as well as alternate PTEs.
    //

    CurrentProcess->ForkInProgress = PsGetCurrentThread ();

    TargetCloneRoot = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MM_AVL_TABLE),
                                             'rCmM');

    if (TargetCloneRoot == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    RtlZeroMemory (TargetCloneRoot, sizeof(MM_AVL_TABLE));
    TargetCloneRoot->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&TargetCloneRoot->BalancedRoot, 0);

    //
    // Only allocate a clone root for the calling process if he doesn't
    // already have one (ie: this is his first fork call).  Note that if
    // the allocation succeeds, the root table remains until the process
    // exits regardless of whether the fork succeeds.
    //

    if (CurrentProcess->CloneRoot == NULL) {
        CloneRoot = ExAllocatePoolWithTag (NonPagedPool,
                                           sizeof(MM_AVL_TABLE),
                                           'rCmM');

        if (CloneRoot == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn2;
        }

        RtlZeroMemory (CloneRoot, sizeof(MM_AVL_TABLE));
        CloneRoot->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&CloneRoot->BalancedRoot, 0);
        CurrentProcess->CloneRoot = CloneRoot;
    }

    CloneHeader = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof(MMCLONE_HEADER),
                                         'hCmM');
    if (CloneHeader == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    CloneDescriptor = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MMCLONE_DESCRIPTOR),
                                             'dCmM');
    if (CloneDescriptor == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    Vad = MiGetFirstVad (CurrentProcess);
    VadList = &FirstNewVad;

    if (PhysicalViewCount != 0) {

        //
        // The working set mutex synchronizes the allocation of the
        // EPROCESS PhysicalVadRoot but is not needed here because no one
        // can be manipulating the target process except this thread.
        // This table root is not deleted until the process exits.
        //

        if ((ProcessToInitialize->PhysicalVadRoot == NULL) &&
            (MiCreatePhysicalVadRoot (ProcessToInitialize, TRUE) == NULL)) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn2;
        }

        i = PhysicalViewCount;
        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_PHYSICAL_VIEW_KEY);

            if (PhysicalView == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn2;
            }

            PhysicalView->u1.Parent = (PMMADDRESS_NODE) PhysicalViewList;
            PhysicalViewList = PhysicalView;
            i -= 1;
        } while (i != 0);
    }

    while (Vad != NULL) {

        if (Vad->u.VadFlags.VadType == VadImageMap) {

            //
            // Refuse the fork if it is an image view using large pages.
            //

            VirtualAddress = MI_VPN_TO_VA (Vad->StartingVpn);

            PointerPxe = MiGetPxeAddress (VirtualAddress);
            PointerPpe = MiGetPpeAddress (VirtualAddress);
            PointerPde = MiGetPdeAddress (VirtualAddress);

            if (
#if (_MI_PAGING_LEVELS>=4)
               (PointerPxe->u.Hard.Valid == 1) &&
#endif
#if (_MI_PAGING_LEVELS>=3)
               (PointerPpe->u.Hard.Valid == 1) &&
#endif
               (PointerPde->u.Hard.Valid == 1) &&
               (MI_PDE_MAPS_LARGE_PAGE (PointerPde))) {

                //
                // Deallocate all VADs and other pool obtained so far.
                //

                *VadList = NULL;
                status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn2;
           }
        }

        //
        // If the VAD does not go to the child, ignore it.
        //

        if ((Vad->u.VadFlags.VadType != VadAwe) &&
            (Vad->u.VadFlags.VadType != VadLargePages) &&
            (Vad->u.VadFlags.VadType != VadRotatePhysical) &&
            (Vad->u.VadFlags.VadType != VadLargePageSection) &&

            ((Vad->u.VadFlags.PrivateMemory == 1) ||
             (Vad->u2.VadFlags2.Inherit == MM_VIEW_SHARE))) {

            NewVad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_LONG), 'ldaV');

            if (NewVad == NULL) {

                //
                // Unable to allocate pool for all the VADs.  Deallocate
                // all VADs and other pool obtained so far.
                //

                *VadList = NULL;
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn2;
            }

            RtlZeroMemory (NewVad, sizeof(MMVAD_LONG));

            *VadList = NewVad;
            VadList = &NewVad->u1.Parent;
        }
        Vad = MiGetNextVad (Vad);
    }

    //
    // Terminate list of VADs for new process.
    //

    *VadList = NULL;

    //
    // Initializing UsedPageTableEntries is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check
    // for use of uninitialized variables.
    //

    UsedPageTableEntries = NULL;
    PdeBase = NULL;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Initializing these is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check
    // for use of uninitialized variables.
    //

    PageDirFrameIndex = 0;
    UsedPageDirectoryEntries = NULL;
    PpeBase = NULL;

#if (_MI_PAGING_LEVELS >= 4)
    PxeBase = NULL;
    PageParentFrameIndex = 0;
    UsedPageDirectoryParentEntries = NULL;
#endif

    //
    // Map the (extended) page directory parent page into the system address
    // space.
    //

    PpeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        RootPhysicalPage,
                                        MmCached,
                                        HighPagePriority);

    if (PpeBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    PfnPpPage = MI_PFN_ELEMENT (RootPhysicalPage);

#if (_MI_PAGING_LEVELS >= 4)

    //
    // The PxeBase is going to map the real top-level.  For 4 level
    // architectures, the PpeBase above is mapping the wrong page, but
    // it doesn't matter because the initial value is never used - it
    // just serves as a way to obtain a mapping PTE and will be re-pointed
    // correctly before it is ever used.
    //

    PxeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        RootPhysicalPage,
                                        MmCached,
                                        HighPagePriority);

    if (PxeBase == NULL) {
        MiUnmapSinglePage (PpeBase);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    PfnPxPage = MI_PFN_ELEMENT (RootPhysicalPage);

    MdlDirParentPage = RootPhysicalPage;

#endif

#endif

    //
    // Initialize a page directory map so it can be
    // unlocked in the loop and the end of the loop without
    // any testing to see if has a valid value the first time through.
    // Note this is a dummy map for 64-bit systems and a real one for 32-bit.
    //

#if !defined (_X86PAE_)

    MdlDirPage = RootPhysicalPage;

    PdePhysicalPage = RootPhysicalPage;

    PdeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        MdlDirPage,
                                        MmCached,
                                        HighPagePriority);

#else

    //
    // All 4 page directory pages need to be mapped for PAE so the heavyweight
    // mapping must be used.
    //

    MdlPageDirectory = (PMDL)&MdlHackPageDirectory[0];

    MmInitializeMdl (MdlPageDirectory,
                     (PVOID)PDE_BASE,
                     PD_PER_SYSTEM * PAGE_SIZE);

    MdlPageDirectory->MdlFlags |= MDL_PAGES_LOCKED;

    RtlCopyMemory (MdlPageDirectory + 1,
                   PageDirectoryFrames,
                   PD_PER_SYSTEM * sizeof (PFN_NUMBER));

    PdePhysicalPage = RootPhysicalPage;

    PdeBase = (PMMPTE) MmMapLockedPagesSpecifyCache (MdlPageDirectory,
                                                     KernelMode,
                                                     MmCached,
                                                     NULL,
                                                     FALSE,
                                                     HighPagePriority);

#endif

    if (PdeBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    PfnPdPage = MI_PFN_ELEMENT (RootPhysicalPage);

#if (_MI_PAGING_LEVELS < 3)

    //
    // Map hyperspace so target UsedPageTable entries can be incremented.
    //

    HyperBase = (PMMWSL)MiMapSinglePage (NULL,
                                         HyperPhysicalPage,
                                         MmCached,
                                         HighPagePriority);

    if (HyperBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    //
    // MmWorkingSetList is not page aligned when booted /3GB so account
    // for that here when established the used page table entry pointer.
    //

    HyperWsl = (PMMWSL) ((PCHAR)HyperBase + BYTE_OFFSET(MmWorkingSetList));

#endif

    //
    // Map the hyperspace page (any cached page would serve equally well)
    // to acquire a mapping PTE now (prior to starting through the loop) -
    // this way we guarantee forward progress regardless of resources once
    // the loop is entered.
    //

    NewPteMappedAddress = (PMMPTE) MiMapSinglePage (NULL,
                                                    HyperPhysicalPage,
                                                    MmCached,
                                                    HighPagePriority);

    if (NewPteMappedAddress == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    PointerNewPte = NewPteMappedAddress;

    //
    // Build a new clone prototype PTE block and descriptor, note that
    // each prototype PTE has a reference count following it.
    //

    ForkProtoPte = CloneProtos;

    CloneHeader->NumberOfPtes = (ULONG)NumberOfPrivatePages;
    CloneHeader->NumberOfProcessReferences = 1;
    CloneHeader->ClonePtes = CloneProtos;

    CloneDescriptor->StartingVpn = (ULONG_PTR)CloneProtos;
    CloneDescriptor->EndingVpn = (ULONG_PTR)((ULONG_PTR)CloneProtos +
                            NumberOfPrivatePages *
                              sizeof(MMCLONE_BLOCK));
    CloneDescriptor->EndingVpn -= 1;
    CloneDescriptor->NumberOfReferences = 0;
    CloneDescriptor->FinalNumberOfReferences = 0;
    CloneDescriptor->NumberOfPtes = (ULONG)NumberOfPrivatePages;
    CloneDescriptor->CloneHeader = CloneHeader;
    CloneDescriptor->PagedPoolQuotaCharge = sizeof(MMCLONE_BLOCK) *
                                NumberOfPrivatePages;

    //
    // Insert the clone descriptor for this fork operation into the
    // process which was cloned.
    //

    MiInsertClone (CurrentProcess, CloneDescriptor);

    TempCloneMapping = NULL;

    //
    // Examine each virtual address descriptor and create the
    // proper structures for the new process.
    //

    Vad = MiGetFirstVad (CurrentProcess);
    NewVad = FirstNewVad;

    while (Vad != NULL) {

        //
        // Examine the VAD to determine its type and inheritance
        // attribute.
        //

        if ((Vad->u.VadFlags.VadType != VadAwe) &&
            (Vad->u.VadFlags.VadType != VadLargePages) &&
            (Vad->u.VadFlags.VadType != VadRotatePhysical) &&
            (Vad->u.VadFlags.VadType != VadLargePageSection) &&

            ((Vad->u.VadFlags.PrivateMemory == 1) ||
             (Vad->u2.VadFlags2.Inherit == MM_VIEW_SHARE))) {

            //
            // The virtual address descriptor should be shared in the
            // forked process.
            //
            // Make a copy of the VAD for the new process, the new VADs
            // are preallocated and linked together through the parent
            // field.
            //

            NextVad = NewVad->u1.Parent;

            if (Vad->u.VadFlags.PrivateMemory == 1) {
                *(PMMVAD_SHORT)NewVad = *(PMMVAD_SHORT)Vad;
                NewVad->u.VadFlags.NoChange = 0;
            }
            else {
                if (Vad->u2.VadFlags2.LongVad == 0) {
                    *NewVad = *Vad;
                }
                else {
                    *(PMMVAD_LONG)NewVad = *(PMMVAD_LONG)Vad;

                    if (Vad->u2.VadFlags2.ExtendableFile == 1) {
                        KeAcquireGuardedMutexUnsafe (&MmSectionBasedMutex);
                        ASSERT (Vad->ControlArea->Segment->ExtendInfo != NULL);
                        Vad->ControlArea->Segment->ExtendInfo->ReferenceCount += 1;
                        KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);
                    }
                }
            }

            NewVad->u2.VadFlags2.LongVad = 1;

            if (NewVad->u.VadFlags.NoChange) {
                if ((NewVad->u2.VadFlags2.OneSecured) ||
                    (NewVad->u2.VadFlags2.MultipleSecured)) {

                    //
                    // Eliminate these as the memory was secured
                    // only in this process, not in the new one.
                    //

                    NewVad->u2.VadFlags2.OneSecured = 0;
                    NewVad->u2.VadFlags2.MultipleSecured = 0;
                    ((PMMVAD_LONG) NewVad)->u3.List.Flink = NULL;
                    ((PMMVAD_LONG) NewVad)->u3.List.Blink = NULL;
                }
                if (NewVad->u2.VadFlags2.SecNoChange == 0) {
                    NewVad->u.VadFlags.NoChange = 0;
                }
            }
            NewVad->u1.Parent = NextVad;

            //
            // If the VAD refers to a section, up the view count for that
            // section.  This requires the PFN lock to be held.
            //

            if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                (Vad->ControlArea != NULL)) {

                if (((Vad->u.VadFlags.Protection == MM_READWRITE) ||
                    (Vad->u.VadFlags.Protection == MM_EXECUTE_READWRITE))
                                &&
                    (Vad->ControlArea->FilePointer != NULL) &&
                    (Vad->ControlArea->u.Flags.Image == 0)) {

                    InterlockedIncrement ((PLONG)&Vad->ControlArea->WritableUserReferences);
                }

                //
                // Increment the count of the number of views for the
                // section object.  This requires the PFN lock to be held.
                //

                MiUpControlAreaRefs (Vad);
            }

            if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {
                PhysicalView = PhysicalViewList;
                ASSERT (PhysicalViewCount != 0);
                ASSERT (PhysicalView != NULL);
                PhysicalViewCount -= 1;
                PhysicalViewList = (PMI_PHYSICAL_VIEW) PhysicalView->u1.Parent;

                PhysicalView->Vad = NewVad;
                PhysicalView->VadType = VadDevicePhysicalMemory;

                PhysicalView->StartingVpn = Vad->StartingVpn;
                PhysicalView->EndingVpn = Vad->EndingVpn;

                //
                // Insert the physical view descriptor now that the page table
                // page hierarchy is in place.  Note probes can find this
                // descriptor immediately once the working set
                // mutex is released.
                //

                ASSERT (ProcessToInitialize->PhysicalVadRoot != NULL);

                MiInsertNode ((PMMADDRESS_NODE)PhysicalView,
                              ProcessToInitialize->PhysicalVadRoot);
            }

            //
            // Examine each PTE and create the appropriate PTE for the
            // new process.
            //

            PointerPde = NULL;      // Needless, but keeps W4 happy
            PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
            LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));
            FirstTime = TRUE;

            do {

                //
                // For each PTE contained in the VAD check the page table
                // page, and if non-zero, make the appropriate modifications
                // to copy the PTE to the new process.
                //

                if ((FirstTime) || MiIsPteOnPdeBoundary (PointerPte)) {

                    PointerPxe = MiGetPpeAddress (PointerPte);
                    PointerPpe = MiGetPdeAddress (PointerPte);
                    PointerPde = MiGetPteAddress (PointerPte);

                    do {

#if (_MI_PAGING_LEVELS >= 4)
                        while (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                            //
                            // Extended page directory parent is empty,
                            // go to the next one.
                            //
    
                            PointerPxe += 1;
                            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                //
                                // All done with this VAD, exit loop.
                                //
    
                                goto AllDone;
                            }
                        }
    
#endif
                        Waited = 0;

                        while (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                            //
                            // Page directory parent is empty, go to the next one.
                            //
    
                            PointerPpe += 1;
                            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                                PointerPxe = MiGetPteAddress (PointerPpe);
                                Waited = 1;
                                break;
                            }
                            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                //
                                // All done with this VAD, exit loop.
                                //
    
                                goto AllDone;
                            }
                        }
    
                        if (Waited != 0) {
                            continue;
                        }
    
                        while (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                            //
                            // This page directory is empty, go to the next one.
                            //
    
                            PointerPde += 1;
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                //
                                // All done with this VAD, exit loop.
                                //
    
                                goto AllDone;
                            }
#if (_MI_PAGING_LEVELS >= 3)
                            if (MiIsPteOnPdeBoundary (PointerPde)) {
                                PointerPpe = MiGetPteAddress (PointerPde);
                                PointerPxe = MiGetPdeAddress (PointerPde);
                                Waited = 1;
                                break;
                            }
#endif
                        }
    
                    } while (Waited != 0);

                    FirstTime = FALSE;

#if (_MI_PAGING_LEVELS >= 4)
                    //
                    // Calculate the address of the PXE in the new process's
                    // extended page directory parent page.
                    //

                    PointerNewPxe = &PxeBase[MiGetPpeOffset(PointerPte)];

                    if (PointerNewPxe->u.Long == 0) {

                        //
                        // No physical page has been allocated yet, get a page
                        // and map it in as a valid page.  This will become
                        // a page directory parent page for the new process.
                        //
                        // Note that unlike page table pages which are left
                        // in transition, page directory parent pages (and page
                        // directory pages) are left valid and hence
                        // no share count decrement is done.
                        //

                        ReleasedWorkingSetMutex =
                                MiLeaveThisPageGetAnother (&PageParentFrameIndex,
                                                           PointerPxe,
                                                           CurrentProcess);

                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageParentFrameIndex));

                        if (ReleasedWorkingSetMutex) {

                            //
                            // Ensure the PDE (and any table above it) are still
                            // resident.
                            //

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                        //
                        // Hand initialize this PFN as normal initialization
                        // would do it for the process whose context we are
                        // attached to.
                        //
                        // The PFN lock must be held while initializing the
                        // frame to prevent those scanning the database for
                        // free frames from taking it after we fill in the
                        // u2 field.
                        //

                        MiBuildForkPageTable (PageParentFrameIndex,
                                              PointerPxe,
                                              PointerNewPxe,
                                              RootPhysicalPage,
                                              PfnPxPage,
                                              TRUE);

                        //
                        // Map the new page directory page into the system
                        // portion of the address space.  Note that hyperspace
                        // cannot be used as other operations (allocating
                        // nonpaged pool at DPC level) could cause the
                        // hyperspace page being used to be reused.
                        //

                        MdlDirParentPage = PageParentFrameIndex;

                        ASSERT (PpeBase != NULL);

                        PpeBase = (PMMPTE) MiMapSinglePage (PpeBase,
                                                            MdlDirParentPage,
                                                            MmCached,
                                                            HighPagePriority);

                        PpePhysicalPage = PageParentFrameIndex;

                        PfnPpPage = MI_PFN_ELEMENT (PpePhysicalPage);
    
                        UsedPageDirectoryParentEntries = (PVOID) PfnPpPage;
                    }
                    else {

                        ASSERT (PointerNewPxe->u.Hard.Valid == 1);

                        PpePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerNewPxe);

                        //
                        // If we are switching from one page directory parent
                        // frame to another and the last one is one that we
                        // freshly allocated, the last one's reference count
                        // must be decremented now that we're done with it.
                        //
                        // Note that at least one target PXE has already been
                        // initialized for this codepath to execute.
                        //

                        ASSERT (PageParentFrameIndex == MdlDirParentPage);

                        if (MdlDirParentPage != PpePhysicalPage) {
                            ASSERT (MdlDirParentPage != (PFN_NUMBER)-1);
                            PageParentFrameIndex = PpePhysicalPage;
                            MdlDirParentPage = PageParentFrameIndex;

                            ASSERT (PpeBase != NULL);
    
                            PpeBase = (PMMPTE) MiMapSinglePage (PpeBase,
                                                                MdlDirParentPage,
                                                                MmCached,
                                                                HighPagePriority);
    
                            PointerNewPpe = PpeBase;

                            PfnPpPage = MI_PFN_ELEMENT (PpePhysicalPage);
        
                            UsedPageDirectoryParentEntries = (PVOID)PfnPpPage;
                        }
                    }
#endif

#if (_MI_PAGING_LEVELS >= 3)

                    //
                    // Calculate the address of the PPE in the new process's
                    // page directory parent page.
                    //

                    PointerNewPpe = &PpeBase[MiGetPdeOffset(PointerPte)];

                    if (PointerNewPpe->u.Long == 0) {

                        //
                        // No physical page has been allocated yet, get a page
                        // and map it in as a valid page.  This will
                        // become a page directory page for the new process.
                        //
                        // Note that unlike page table pages which are left
                        // in transition, page directory pages are left valid
                        // and hence no share count decrement is done.
                        //

                        ReleasedWorkingSetMutex =
                                MiLeaveThisPageGetAnother (&PageDirFrameIndex,
                                                           PointerPpe,
                                                           CurrentProcess);

                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageDirFrameIndex));

                        if (ReleasedWorkingSetMutex) {

                            //
                            // Ensure the PDE (and any table above it) are still
                            // resident.
                            //

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                        //
                        // Hand initialize this PFN as normal initialization
                        // would do it for the process whose context we are
                        // attached to.
                        //
                        // The PFN lock must be held while initializing the
                        // frame to prevent those scanning the database for
                        // free frames from taking it after we fill in the
                        // u2 field.
                        //

                        MiBuildForkPageTable (PageDirFrameIndex,
                                              PointerPpe,
                                              PointerNewPpe,
#if (_MI_PAGING_LEVELS >= 4)
                                              PpePhysicalPage,
#else
                                              RootPhysicalPage,
#endif
                                              PfnPpPage,
                                              TRUE);

#if (_MI_PAGING_LEVELS >= 4)
                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryParentEntries);
#endif
                        //
                        // Map the new page directory page into the system
                        // portion of the address space.  Note that hyperspace
                        // cannot be used as other operations (allocating
                        // nonpaged pool at DPC level) could cause the
                        // hyperspace page being used to be reused.
                        //

                        MdlDirPage = PageDirFrameIndex;

                        ASSERT (PdeBase != NULL);

                        PdeBase = (PMMPTE) MiMapSinglePage (PdeBase,
                                                            MdlDirPage,
                                                            MmCached,
                                                            HighPagePriority);

                        PointerNewPde = PdeBase;
                        PdePhysicalPage = PageDirFrameIndex;

                        PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
    
                        UsedPageDirectoryEntries = (PVOID)PfnPdPage;
                    }
                    else {
                        ASSERT (PointerNewPpe->u.Hard.Valid == 1 ||
                                PointerNewPpe->u.Soft.Transition == 1);

                        if (PointerNewPpe->u.Hard.Valid == 1) {
                            PdePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerNewPpe);
                        }
                        else {
                            PdePhysicalPage = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerNewPpe);
                        }

                        //
                        // If we are switching from one page directory frame to
                        // another and the last one is one that we freshly
                        // allocated, the last one's reference count must be
                        // decremented now that we're done with it.
                        //
                        // Note that at least one target PPE has already been
                        // initialized for this codepath to execute.
                        //

                        ASSERT (PageDirFrameIndex == MdlDirPage);

                        if (MdlDirPage != PdePhysicalPage) {
                            ASSERT (MdlDirPage != (PFN_NUMBER)-1);
                            PageDirFrameIndex = PdePhysicalPage;
                            MdlDirPage = PageDirFrameIndex;

                            ASSERT (PdeBase != NULL);
    
                            PdeBase = (PMMPTE) MiMapSinglePage (PdeBase,
                                                                MdlDirPage,
                                                                MmCached,
                                                                HighPagePriority);
    
                            PointerNewPde = PdeBase;

                            PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
        
                            UsedPageDirectoryEntries = (PVOID)PfnPdPage;
                        }
                    }
#endif

                    //
                    // Calculate the address of the PDE in the new process's
                    // page directory page.
                    //

#if defined (_X86PAE_)
                    //
                    // All four PAE page directory frames are mapped virtually
                    // contiguous and so the PpePdeOffset can (and must) be
                    // safely used here.
                    //
                    PpePdeOffset = MiGetPdeIndex(MiGetVirtualAddressMappedByPte(PointerPte));
#else
                    PpePdeOffset = MiGetPdeOffset(MiGetVirtualAddressMappedByPte(PointerPte));
#endif

                    PointerNewPde = &PdeBase[PpePdeOffset];

                    if (PointerNewPde->u.Long == 0) {

                        //
                        // No physical page has been allocated yet, get a page
                        // and map it in as a transition page.  This will
                        // become a page table page for the new process.
                        //

                        ReleasedWorkingSetMutex =
                                MiDoneWithThisPageGetAnother (&PageFrameIndex,
                                                              PointerPde,
                                                              CurrentProcess);

#if (_MI_PAGING_LEVELS >= 3)
                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageFrameIndex));
#endif
                        if (ReleasedWorkingSetMutex) {

                            //
                            // Ensure the PDE (and any table above it) are still
                            // resident.
                            //

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                        //
                        // Hand initialize this PFN as normal initialization
                        // would do it for the process whose context we are
                        // attached to.
                        //
                        // The PFN lock must be held while initializing the
                        // frame to prevent those scanning the database for
                        // free frames from taking it after we fill in the
                        // u2 field.
                        //

#if defined (_X86PAE_)
                        PdePhysicalPage = PageDirectoryFrames[MiGetPdPteOffset(MiGetVirtualAddressMappedByPte(PointerPte))];
                        PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
#endif

                        MiBuildForkPageTable (PageFrameIndex,
                                              PointerPde,
                                              PointerNewPde,
                                              PdePhysicalPage,
                                              PfnPdPage,
                                              FALSE);

#if (_MI_PAGING_LEVELS >= 3)
                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryEntries);
#endif

                        //
                        // Map the new page table page into the system portion
                        // of the address space.  Note that hyperspace
                        // cannot be used as other operations (allocating
                        // nonpaged pool at DPC level) could cause the
                        // hyperspace page being used to be reused.
                        //

                        ASSERT (NewPteMappedAddress != NULL);

                        PointerNewPte = (PMMPTE) MiMapSinglePage (NewPteMappedAddress,
                                                                  PageFrameIndex,
                                                                  MmCached,
                                                                  HighPagePriority);
                    
                        ASSERT (PointerNewPte != NULL);
                    }

                    //
                    // Calculate the address of the new PTE to build.
                    // Note that FirstTime could be true, yet the page
                    // table page already built.
                    //

                    PointerNewPte = (PMMPTE)((ULONG_PTR)PAGE_ALIGN(PointerNewPte) |
                                            BYTE_OFFSET (PointerPte));

#if (_MI_PAGING_LEVELS >= 3)
                    UsedPageTableEntries = (PVOID)MI_PFN_ELEMENT((PFN_NUMBER)PointerNewPde->u.Hard.PageFrameNumber);
#else
#if !defined (_X86PAE_)
                    UsedPageTableEntries = (PVOID)&HyperWsl->UsedPageTableEntries
                                                [MiGetPteOffset( PointerPte )];
#else
                    UsedPageTableEntries = (PVOID)&HyperWsl->UsedPageTableEntries
                                                [MiGetPdeIndex(MiGetVirtualAddressMappedByPte(PointerPte))];
#endif
#endif

                }

                MiMakeSystemAddressValid (PointerPte, CurrentProcess);

                PteContents = *PointerPte;

                //
                // Check each PTE.
                //

                if (PteContents.u.Long == 0) {
                    NOTHING;
                }
                else if (PteContents.u.Hard.Valid == 1) {

                    //
                    // Valid.
                    //

                    if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

                        //
                        // A PTE just went from not present, not transition to
                        // present.  The share count and valid count must be
                        // updated in the new page table page which contains
                        // this PTE.
                        //

                        ASSERT (PageFrameIndex != (PFN_NUMBER)-1);
                        Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
                        Pfn2->u2.ShareCount += 1;

                        //
                        // Another zeroed PTE is being made non-zero.
                        //

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                        PointerPte += 1;
                        PointerNewPte += 1;
                        continue;
                    }

                    Pfn2 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
                    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                    WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                                    MmWorkingSetList,
                                                    Pfn2->u1.WsIndex,
                                                    FALSE);

                    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

                    if (Pfn2->u3.e1.PrototypePte == 1) {

                        //
                        // This is a prototype PTE.  The PFN database does
                        // not contain the contents of this PTE it contains
                        // the contents of the prototype PTE.  This PTE must
                        // be reconstructed to contain a pointer to the
                        // prototype PTE.
                        //
                        // The working set list entry contains information about
                        // how to reconstruct the PTE.
                        //

                        if (MmWsle[WorkingSetIndex].u1.e1.Protection != MM_ZERO_ACCESS) {

                            //
                            // The protection for the prototype PTE is in the
                            // WSLE.
                            //

                            TempPte.u.Long = 0;
                            TempPte.u.Soft.Protection =
                                MI_GET_PROTECTION_FROM_WSLE(&MmWsle[WorkingSetIndex]);
                            TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
                        }
                        else {

                            //
                            // The protection is in the prototype PTE.
                            //

                            TempPte.u.Long = MiProtoAddressForPte (
                                                            Pfn2->PteAddress);
                        }

                        TempPte.u.Proto.Prototype = 1;
                        MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                        //
                        // A PTE is now non-zero, increment the used page
                        // table entries counter.
                        //

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                        //
                        // Check to see if this is a fork prototype PTE,
                        // and if it is increment the reference count
                        // which is in the longword following the PTE.
                        //

                        if (MiLocateCloneAddress (CurrentProcess, (PVOID)Pfn2->PteAddress) !=
                                    NULL) {

                            //
                            // The reference count field, or the prototype PTE
                            // for that matter may not be in the working set.
                            //

                            CloneProto = (PMMCLONE_BLOCK)Pfn2->PteAddress;

                            //
                            // Make the fork prototype PTE location resident.
                            //

                            if (PAGE_ALIGN (TempCloneMapping) == PAGE_ALIGN (CloneProto)) {

                                ASSERT (CloneProto->CloneRefCount >= 1);
                                InterlockedIncrement (&CloneProto->CloneRefCount);
                            }
                            else {
                                
                                //
                                // Release the working set pushlock before
                                // accessing paged pool to prevent potential
                                // deadlock as we may need a page for the pool
                                // reference and this process' working set
                                // may contain all the trimmable pages.
                                //
                                // Undo the actions taken above since the PTE
                                // may not be valid when we reprocess it.
                                //

                                MI_WRITE_ZERO_PTE (PointerNewPte);
                                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                                UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                                if (TempCloneMapping != NULL) {
                                    MiUnlockPagedAddress (TempCloneMapping);
                                }

                                MiLockPagedAddress (CloneProto);

                                TempCloneMapping = CloneProto;

                                LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                                //
                                // Process this PTE again as the working set
                                // pushlock was released and reacquired.
                                //

                                continue;
                            }
                        }
                    }
                    else {

                        //
                        // This is a private page, create a fork prototype PTE
                        // which becomes the "prototype" PTE for this page.
                        // The protection is the same as that in the prototype
                        // PTE so the WSLE does not need to be updated.
                        //

                        MI_MAKE_VALID_PTE_WRITE_COPY (PointerPte);

                        MI_FLUSH_SINGLE_TB (VirtualAddress, FALSE);

                        ForkProtoPte->ProtoPte = *PointerPte;
                        ForkProtoPte->CloneRefCount = 2;

                        //
                        // Transform the PFN element to reference this new fork
                        // prototype PTE.
                        //

                        Pfn2->PteAddress = &ForkProtoPte->ProtoPte;
                        Pfn2->u3.e1.PrototypePte = 1;

                        ContainingPte = MiGetPteAddress(&ForkProtoPte->ProtoPte);
                        if (ContainingPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                            if (!NT_SUCCESS(MiCheckPdeForPagedPool (&ForkProtoPte->ProtoPte))) {
#endif
                                KeBugCheckEx (MEMORY_MANAGEMENT,
                                              0x61940, 
                                              (ULONG_PTR)&ForkProtoPte->ProtoPte,
                                              (ULONG_PTR)ContainingPte->u.Long,
                                              (ULONG_PTR)MiGetVirtualAddressMappedByPte(&ForkProtoPte->ProtoPte));
#if (_MI_PAGING_LEVELS < 3)
                            }
#endif
                        }
                        Pfn2->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPte);


                        //
                        // Increment the share count for the page containing the
                        // fork prototype PTEs as we have just placed a valid
                        // PTE into the page.
                        //

                        PfnForkPtePage = MI_PFN_ELEMENT (
                                            ContainingPte->u.Hard.PageFrameNumber );

                        MiUpForkPageShareCount (PfnForkPtePage);

                        //
                        // Change the protection in the PFN database to COPY
                        // on write, if writable.
                        //

                        MI_MAKE_PROTECT_WRITE_COPY (Pfn2->OriginalPte);

                        //
                        // Clear the protection field in the WSLE as the
                        // protection is obtained from the prototype PTE and
                        // we want to ensure the hash deleted bit is cleared.
                        //

                        MmWsle[WorkingSetIndex].u1.e1.Protection = MM_ZERO_ACCESS;

                        TempPte.u.Long = MiProtoAddressForPte (Pfn2->PteAddress);
                        TempPte.u.Proto.Prototype = 1;
                        MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                        //
                        // A PTE is now non-zero, increment the used page
                        // table entries counter.
                        //

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                        //
                        // One less private page (it's now shared).
                        //

                        CurrentProcess->NumberOfPrivatePages -= 1;

                        ForkProtoPte += 1;
                        NumberOfForkPtes += 1;
                    }
                }
                else if (PteContents.u.Soft.Prototype == 1) {

                    //
                    // Prototype PTE, check to see if this is a fork
                    // prototype PTE already.  Note that if COW is set,
                    // the PTE can just be copied (fork compatible format).
                    //

                    MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);

                    //
                    // A PTE is now non-zero, increment the used page
                    // table entries counter.
                    //

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                    //
                    // Check to see if this is a fork prototype PTE,
                    // and if it is increment the reference count
                    // which is in the longword following the PTE.
                    //

                    CloneProto = (PMMCLONE_BLOCK)(ULONG_PTR)MiPteToProto(PointerPte);

                    if (MiLocateCloneAddress (CurrentProcess, (PVOID)CloneProto) != NULL) {

                        //
                        // The reference count field, or the prototype PTE
                        // for that matter may not be in the working set.
                        //
                        // Ensure the fork prototype PTE is resident.
                        //

                        if (PAGE_ALIGN (TempCloneMapping) == PAGE_ALIGN (CloneProto)) {

                            ASSERT (CloneProto->CloneRefCount >= 1);
                            InterlockedIncrement (&CloneProto->CloneRefCount);
                        }
                        else {
                            
                            //
                            // Release the working set pushlock before
                            // accessing paged pool to prevent potential
                            // deadlock as we may need a page for the pool
                            // reference and this process' working set
                            // may contain all the trimmable pages.
                            //
                            // Undo the used PTE increment above since it will
                            // be done again when we reprocess it.
                            //

                            MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                            if (TempCloneMapping != NULL) {
                                MiUnlockPagedAddress (TempCloneMapping);
                            }

                            MiLockPagedAddress (CloneProto);

                            TempCloneMapping = CloneProto;

                            LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                            //
                            // Process this PTE again as the working set
                            // pushlock was released and reacquired.
                            //

                            continue;
                        }
                    }
                }
                else if (PteContents.u.Soft.Transition == 1) {

                    //
                    // Transition.
                    //

                    if (MiHandleForkTransitionPte (PointerPte,
                                                   PointerNewPte,
                                                   ForkProtoPte)) {
                        //
                        // PTE is no longer transition, try again.
                        //

                        continue;
                    }

                    //
                    // A PTE is now non-zero, increment the used page
                    // table entries counter.
                    //

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                    //
                    // One less private page (it's now shared).
                    //

                    CurrentProcess->NumberOfPrivatePages -= 1;

                    ForkProtoPte += 1;
                    NumberOfForkPtes += 1;
                }
                else {

                    //
                    // Page file format (may be demand zero).
                    //

                    if (IS_PTE_NOT_DEMAND_ZERO (PteContents)) {

                        if (PteContents.u.Soft.Protection == MM_DECOMMIT) {

                            //
                            // This is a decommitted PTE, just move it
                            // over to the new process.  Don't increment
                            // the count of private pages.
                            //

                            MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);
                        }
                        else {

                            //
                            // The PTE is not demand zero, move the PTE to
                            // a fork prototype PTE and make this PTE and
                            // the new processes PTE refer to the fork
                            // prototype PTE.
                            //

                            ForkProtoPte->ProtoPte = PteContents;

                            //
                            // Make the protection write-copy if writable.
                            //

                            MI_MAKE_PROTECT_WRITE_COPY (ForkProtoPte->ProtoPte);

                            ForkProtoPte->CloneRefCount = 2;

                            TempPte.u.Long =
                                 MiProtoAddressForPte (&ForkProtoPte->ProtoPte);

                            TempPte.u.Proto.Prototype = 1;

                            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                            MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                            //
                            // One less private page (it's now shared).
                            //

                            CurrentProcess->NumberOfPrivatePages -= 1;

                            ForkProtoPte += 1;
                            NumberOfForkPtes += 1;
                        }
                    }
                    else {

                        //
                        // The page is demand zero, make the new process's
                        // page demand zero.
                        //

                        MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);
                    }

                    //
                    // A PTE is now non-zero, increment the used page
                    // table entries counter.
                    //

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);
                }

                PointerPte += 1;
                PointerNewPte += 1;

            } while (PointerPte <= LastPte);
AllDone:
            NewVad = NewVad->u1.Parent;
        }
        Vad = MiGetNextVad (Vad);

    } // end while for VADs

    ASSERT (PhysicalViewCount == 0);

    //
    // Unmap the PD Page and hyper space page.
    //

#if (_MI_PAGING_LEVELS >= 4)
    MiUnmapSinglePage (PxeBase);
#endif

#if (_MI_PAGING_LEVELS >= 3)
    MiUnmapSinglePage (PpeBase);
#endif

#if !defined (_X86PAE_)
    MiUnmapSinglePage (PdeBase);
#else
    MmUnmapLockedPages (PdeBase, MdlPageDirectory);
#endif

#if (_MI_PAGING_LEVELS < 3)
    MiUnmapSinglePage (HyperBase);
#endif

    MiUnmapSinglePage (NewPteMappedAddress);

    //
    // Make the count of private pages match between the two processes.
    //

    ASSERT ((SPFN_NUMBER)CurrentProcess->NumberOfPrivatePages >= 0);

    ProcessToInitialize->NumberOfPrivatePages =
                                          CurrentProcess->NumberOfPrivatePages;

    ASSERT (NumberOfForkPtes <= CloneDescriptor->NumberOfPtes);

    if (NumberOfForkPtes != 0) {

        //
        // The number of fork PTEs is non-zero, set the values
        // into the structures.
        //

        CloneHeader->NumberOfPtes = NumberOfForkPtes;
        CloneDescriptor->NumberOfReferences = NumberOfForkPtes;
        CloneDescriptor->FinalNumberOfReferences = NumberOfForkPtes;
        CloneDescriptor->NumberOfPtes = NumberOfForkPtes;
    }
    else {

        //
        // There were no fork PTEs created.  Remove the clone descriptor
        // from this process and clean up the related structures.
        //

        MiRemoveClone (CurrentProcess, CloneDescriptor);

        UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

        ASSERT (CloneProtos == CloneDescriptor->CloneHeader->ClonePtes);

        ASSERT (CloneProtos != NULL);

        //
        // Unlock and free the paged pool.
        //

        for (i = 0; i < sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages; i += PAGE_SIZE) {
            MiUnlockPagedAddress ((PVOID)((PCHAR) CloneProtos + i));
        }

        ExFreePool (CloneProtos);

        CloneProtos = NULL;

        ExFreePool (CloneDescriptor->CloneHeader);

        //
        // Return the pool for the global structures referenced by the
        // clone descriptor.
        //

        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       CloneDescriptor->PagedPoolQuotaCharge);

        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        ExFreePool (CloneDescriptor);

        LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
    }

    //
    // As we have updated many PTEs to clear dirty bits, flush the
    // TB cache.  Note that this was not done every time we changed
    // a valid PTE so other threads could be modifying the address
    // space without causing copy on writes. Too bad, because an app
    // that is not synchronizing itself is going to have coherency problems
    // anyway.  Note that this cannot cause any system page corruption because
    // the address space mutex was (and is) still held throughout and is
    // not released until after we flush the TB now.
    //

    MiDownShareCountFlushEntireTb (PageFrameIndex);

    PageFrameIndex = (PFN_NUMBER)-1;

    //
    // Copy the clone descriptors from this process to the new process.
    //

    Clone = MiGetFirstClone (CurrentProcess);
    CloneList = &FirstNewClone;

    while (Clone != NULL) {

        //
        // Increment the count of processes referencing this clone block.
        //

        ASSERT (Clone->CloneHeader->NumberOfProcessReferences >= 1);

        InterlockedIncrement (&Clone->CloneHeader->NumberOfProcessReferences);

        do {
            NewClone = ExAllocatePoolWithTag (NonPagedPool,
                                              sizeof( MMCLONE_DESCRIPTOR),
                                              'dCmM');

            if (NewClone != NULL) {
                break;
            }

            //
            // There are insufficient resources to continue this operation,
            // however, to properly clean up at this point, all the
            // clone headers must be allocated, so when the cloned process
            // is deleted, the clone headers will be found.  If the pool
            // is not readily available, loop periodically trying for it.
            //
            // Release the working set mutex so this process can be trimmed
            // and reacquire after the delay.
            //

            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmShortTime);

            LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

        } while (TRUE);

        *NewClone = *Clone;

        //
        // Carefully update the FinalReferenceCount as this forking thread
        // may have begun while a faulting thread is waiting in
        // MiDecrementCloneBlockReference for the clone PTEs to inpage.
        // In this case, the ReferenceCount has been decremented but the
        // FinalReferenceCount hasn't been yet.  When the faulter awakes, he
        // will automatically take care of this process, but we must fix up
        // the child process now.  Otherwise the clone descriptor, clone header
        // and clone PTE pool allocations will leak and so will the charged
        // quota.
        //

        if (NewClone->FinalNumberOfReferences > NewClone->NumberOfReferences) {
            NewClone->FinalNumberOfReferences = NewClone->NumberOfReferences;
        }

        *CloneList = NewClone;

        CloneList = (PMMCLONE_DESCRIPTOR *)&NewClone->u1.Parent;
        Clone = MiGetNextClone (Clone);
    }

    *CloneList = NULL;

    ASSERT (CurrentProcess->ForkInProgress == PsGetCurrentThread ());
    CurrentProcess->ForkInProgress = NULL;

    //
    // Release the working set mutex and the address creation mutex from
    // the current process as all the necessary information is now
    // captured.
    //

    UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

    UNLOCK_ADDRESS_SPACE (CurrentProcess);

    if (TempCloneMapping != NULL) {
        MiUnlockPagedAddress (TempCloneMapping);
    }

    if (CloneProtos != NULL) {

        //
        // Unlock the paged pool.
        //

        for (i = 0; i < sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages; i += PAGE_SIZE) {
            MiUnlockPagedAddress ((PVOID)((PCHAR) CloneProtos + i));
        }
    }

    //
    // Attach to the process to initialize and insert the vad and clone
    // descriptors into the tree.
    //

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
        Attached = FALSE;
    }

    if (PsGetCurrentProcess() != ProcessToInitialize) {
        Attached = TRUE;
        KeStackAttachProcess (&ProcessToInitialize->Pcb, &ApcState);
    }

    CurrentProcess = ProcessToInitialize;

    //
    // We are now in the context of the new process, build the
    // VAD list and the clone list.
    //

    Vad = FirstNewVad;

    LOCK_WS (CurrentThread, CurrentProcess);

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Update the WSLEs for the page directories that were added.
    //

    PointerPpe = MiGetPpeAddress (0);
    PointerPpeLast = MiGetPpeAddress (MM_HIGHEST_USER_ADDRESS);
    PointerPxe = MiGetPxeAddress (0);

    while (PointerPpe <= PointerPpeLast) {

#if (_MI_PAGING_LEVELS >= 4)
        while (PointerPxe->u.Long == 0) {
            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            if (PointerPpe > PointerPpeLast) {
                goto WslesFinished;
            }
        }

        //
        // Update the WSLE for this page directory parent page (if we haven't
        // already done it in this loop).
        //

        ASSERT (PointerPxe->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPxe);

        PfnPdPage = MI_PFN_ELEMENT (PageFrameIndex);
    
        if (PfnPdPage->u1.Event == NULL) {
    
            do {
                WorkingSetIndex = MiAddValidPageToWorkingSet (PointerPpe,
                                                              PointerPxe,
                                                              PfnPdPage,
                                                              0);

                if (WorkingSetIndex != 0) {
                    break;
                }

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&Mm30Milliseconds);

            } while (TRUE);
        }

#endif

        if (PointerPpe->u.Long != 0) {

            ASSERT (PointerPpe->u.Hard.Valid == 1);

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPpe);

            PfnPdPage = MI_PFN_ELEMENT (PageFrameIndex);
        
            ASSERT (PfnPdPage->u1.Event == NULL);
        
            do {
                WorkingSetIndex = MiAddValidPageToWorkingSet (MiGetVirtualAddressMappedByPte (PointerPpe),
                                                              PointerPpe,
                                                              PfnPdPage,
                                                              0);
                if (WorkingSetIndex != 0) {
                    break;
                }

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&Mm30Milliseconds);

            } while (TRUE);
        }

        PointerPpe += 1;
#if (_MI_PAGING_LEVELS >= 4)
        if (MiIsPteOnPdeBoundary (PointerPpe)) {
            PointerPxe += 1;
            ASSERT (PointerPxe == MiGetPteAddress (PointerPpe));
        }
#endif
    }

#if (_MI_PAGING_LEVELS >= 4)
WslesFinished:
#endif
#endif

    if (CurrentProcess->PhysicalVadRoot != NULL) {

        RestartKey = NULL;

        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (CurrentProcess->PhysicalVadRoot, &RestartKey);

            if (PhysicalView == NULL) {
                break;
            }

            ASSERT (PhysicalView->VadType == VadDevicePhysicalMemory);

            PointerPde = MiGetPdeAddress (MI_VPN_TO_VA (PhysicalView->StartingVpn));
            LastPde = MiGetPdeAddress (MI_VPN_TO_VA (PhysicalView->EndingVpn));

            do {

                //
                // The PDE entry must still be in transition (they all are),
                // but we guaranteed that the page table page itself is still
                // resident because we've incremented the sharecount by the
                // number of physical mappings the page table page contains.
                //
                // The only reason the PDE would be valid here is if we've
                // already processed a different physical view that shares
                // the same page table page.
                //
                // Increment it once more here (to account for itself), make
                // the PDE valid and insert it into the working set list.
                //

                if (PointerPde->u.Hard.Valid == 0) {

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    
                    Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
    
                    //
                    // There can be no I/O in progress on this page, it cannot
                    // be trimmed and no one else can be accessing it, so the
                    // PFN lock is not needed to increment the share count here.
                    //
    
                    Pfn2->u2.ShareCount += 1;
    
                    MI_MAKE_TRANSITION_PTE_VALID (TempPte, PointerPde);
                    MI_SET_PTE_DIRTY (TempPte);
                    MI_WRITE_VALID_PTE (PointerPde, TempPte);
    
                    ASSERT (Pfn2->u1.Event == NULL);
                
                    do {
                        WorkingSetIndex = MiAddValidPageToWorkingSet (MiGetVirtualAddressMappedByPte (PointerPde),
                                                                      PointerPde,
                                                                      Pfn2,
                                                                      0);
                        if (WorkingSetIndex != 0) {
                            break;
                        }
        
                        KeDelayExecutionThread (KernelMode,
                                                FALSE,
                                                (PLARGE_INTEGER)&Mm30Milliseconds);
        
                    } while (TRUE);
                }

                PointerPde += 1;

            } while (PointerPde <= LastPde);

        } while (TRUE);
    }

    UNLOCK_WS (CurrentThread, CurrentProcess);

    status = STATUS_SUCCESS;

    while (Vad != NULL) {

        NextVad = Vad->u1.Parent;

        if (!NT_SUCCESS (status)) {
            Vad->u.VadFlags.CommitCharge = MM_MAX_COMMIT;
        }

        status2 = MiInsertVadCharges (Vad, CurrentProcess);

        if (!NT_SUCCESS (status2)) {

            //
            // Charging quota for the VAD failed, set the
            // remaining quota fields in this VAD and all
            // subsequent VADs to zero so the VADs can be
            // inserted and later deleted.
            //

            status = status2;

            //
            // Do the loop again for this VAD.
            //

            continue;
        }

        LOCK_WS (CurrentThread, CurrentProcess);

        MiInsertVad (Vad, CurrentProcess);

        UNLOCK_WS (CurrentThread, CurrentProcess);

        //
        // Update the current virtual size.
        //

        CurrentProcess->VirtualSize += PAGE_SIZE +
                            ((Vad->EndingVpn - Vad->StartingVpn) >> PAGE_SHIFT);

        Vad = NextVad;
    }

    //
    // Update the peak virtual size.
    //

    CurrentProcess->PeakVirtualSize = CurrentProcess->VirtualSize;

    CurrentProcess->CloneRoot = TargetCloneRoot;

    Clone = FirstNewClone;
    TotalPagedPoolCharge = 0;
    TotalNonPagedPoolCharge = 0;

    while (Clone != NULL) {

        NextClone = (PMMCLONE_DESCRIPTOR) Clone->u1.Parent;
        MiInsertClone (CurrentProcess, Clone);

        //
        // Calculate the paged pool and non-paged pool to charge for these
        // operations.
        //

        TotalPagedPoolCharge += Clone->PagedPoolQuotaCharge;
        TotalNonPagedPoolCharge += sizeof(MMCLONE_HEADER);

        Clone = NextClone;
    }

    if (!NT_SUCCESS (status)) {

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }

        return status;
    }

    status = PsChargeProcessPagedPoolQuota (CurrentProcess,
                                            TotalPagedPoolCharge);

    if (!NT_SUCCESS (status)) {

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
    }

    status = PsChargeProcessNonPagedPoolQuota (CurrentProcess,
                                               TotalNonPagedPoolCharge);

    if (!NT_SUCCESS (status)) {

        PsReturnProcessPagedPoolQuota (CurrentProcess, TotalPagedPoolCharge);

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
    }

    ASSERT ((ProcessToClone->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);
    ASSERT ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    return STATUS_SUCCESS;

    //
    // Error returns.
    //

ErrorReturn3:

#if (_MI_PAGING_LEVELS >= 4)
        if (PxeBase != NULL) {
            MiUnmapSinglePage (PxeBase);
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)
        if (PpeBase != NULL) {
            MiUnmapSinglePage (PpeBase);
        }
        if (PdeBase != NULL) {
            MiUnmapSinglePage (PdeBase);
        }
#else
        if (HyperBase != NULL) {
            MiUnmapSinglePage (HyperBase);
        }
        if (PdeBase != NULL) {
#if !defined (_X86PAE_)
            MiUnmapSinglePage (PdeBase);
#else
            MmUnmapLockedPages (PdeBase, MdlPageDirectory);
#endif
        }

#endif

ErrorReturn2:
        CurrentProcess->ForkInProgress = NULL;
        UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

        PhysicalView = PhysicalViewList;
        while (PhysicalView != NULL) {
            NextPhysicalView = (PMI_PHYSICAL_VIEW) PhysicalView->u1.Parent;
            ExFreePool (PhysicalView);
            PhysicalView = NextPhysicalView;
        }

        NewVad = FirstNewVad;
        while (NewVad != NULL) {
            Vad = NewVad->u1.Parent;

            ExFreePool (NewVad);
            NewVad = Vad;
        }

        if (CloneDescriptor != NULL) {
            ExFreePool (CloneDescriptor);
        }

        if (CloneHeader != NULL) {
            ExFreePool (CloneHeader);
        }

ErrorReturn1:

        UNLOCK_ADDRESS_SPACE (CurrentProcess);

        if (CloneProtos != NULL) {

            //
            // Unlock and free the paged pool.
            //

            for (i = 0; i < sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages; i += PAGE_SIZE) {
                MiUnlockPagedAddress ((PVOID)((PCHAR) CloneProtos + i));
            }

            ExFreePool (CloneProtos);

            PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
            PsReturnProcessPagedPoolQuota (CurrentProcess,
                                           sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        }

        if (TargetCloneRoot != NULL) {
            ExFreePool (TargetCloneRoot);
        }
        ASSERT ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);
        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
}

ULONG
MiDecrementCloneBlockReference (
    IN PMMCLONE_DESCRIPTOR CloneDescriptor,
    IN PMMCLONE_BLOCK CloneBlock OPTIONAL,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine decrements the reference count field of a "fork prototype
    PTE" (clone-block).  If the reference count becomes zero, the reference
    count for the clone-descriptor is decremented and if that becomes zero,
    it is deallocated and the number of processes count for the clone header is
    decremented.  If the number of processes count becomes zero, the clone
    header is deallocated.

Arguments:

    CloneDescriptor - Supplies the clone descriptor which describes the
                      clone block.

    CloneBlock - Supplies the clone block to decrement the reference count of.
                 If NULL is supplied then just the clone descriptor is
                 decremented.

    CurrentProcess - Supplies the current process.

    PteFlushList - Supplies a PTE list to TB flush if the PFN lock is going to
                   be released here.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    TRUE if the working set mutex was released, FALSE if it was not.

Environment:

    Kernel mode, APCs disabled, working set mutex and PFN lock held.
    In some cases, the address creation mutex is also held.

--*/

{
    PETHREAD CurrentThread;
    PMMCLONE_HEADER CloneHeader;
    ULONG MutexReleased;
    MMPTE CloneContents;
    PMMPFN Pfn3;
    PMMPFN Pfn4;
    LONG NewCount;
    LOGICAL WsHeldSafe;
    LOGICAL WsHeldShared;

    ASSERT (CurrentProcess == PsGetCurrentProcess ());

    MutexReleased = FALSE;

    //
    // Note carefully : the clone descriptor count is decremented *BEFORE*
    // dereferencing the pageable clone PTEs.  This is because the working
    // set mutex is released and reacquired if the clone PTEs need to be made
    // resident for the dereference.  And this opens a window where a fork
    // could begin.  This thread will wait for the fork to finish, but the
    // fork will copy the clone descriptors (including this one) and get a
    // stale descriptor reference count (too high by one) as our decrement
    // will only occur in our descriptor and not the forked one.
    //
    // Decrementing the clone descriptor count *BEFORE* potentially
    // releasing the working set mutex solves this entire problem.
    //
    // Note that after the decrement, the clone descriptor can
    // only be referenced here if the count dropped to exactly zero.  (If it
    // was nonzero, some other thread may drive it to zero and free it in the
    // gap where we release locks to inpage the clone block).
    //

    CloneDescriptor->NumberOfReferences -= 1;

    ASSERT (CloneDescriptor->NumberOfReferences >= 0);

    if (CloneDescriptor->NumberOfReferences == 0) {

        //
        // There are no longer any PTEs in this process which refer
        // to the fork prototype PTEs for this clone descriptor.
        // Remove the CloneDescriptor now so a fork won't see it either.
        //

        //
        // Flush TBs prior to releasing the PFN lock.
        //

        if ((ARGUMENT_PRESENT (PteFlushList)) && (PteFlushList->Count != 0)) {
            MiFlushPteList (PteFlushList);
        }

        UNLOCK_PFN (OldIrql);

        //
        // MiRemoveClone and its callees are pageable so release the PFN
        // lock now.
        //

        MiRemoveClone (CurrentProcess, CloneDescriptor);
        MutexReleased = TRUE;

        LOCK_PFN (OldIrql);
    }

    //
    // Now process the clone PTE block and any other descriptor cleanup that
    // may be needed.
    //

    if (ARGUMENT_PRESENT (CloneBlock)) {

        if (!MiIsAddressValid (CloneBlock, TRUE)) {
    
            //
            // Flush TBs prior to releasing the PFN lock.
            //
    
            if ((ARGUMENT_PRESENT (PteFlushList)) &&
                (PteFlushList->Count != 0)) {

                MiFlushPteList (PteFlushList);
            }
    
            MutexReleased = MiMakeSystemAddressValidPfnWs (CloneBlock,
                                                           CurrentProcess,
                                                           OldIrql);
        }
    }

    while ((CurrentProcess->ForkInProgress != NULL) &&
           (CurrentProcess->ForkInProgress != PsGetCurrentThread ())) {

        //
        // Flush TBs prior to releasing the PFN lock.
        //

        if ((ARGUMENT_PRESENT (PteFlushList)) && (PteFlushList->Count != 0)) {
            MiFlushPteList (PteFlushList);
        }

        UNLOCK_PFN (OldIrql);

        MiWaitForForkToComplete (CurrentProcess);

        LOCK_PFN (OldIrql);

        if (ARGUMENT_PRESENT (CloneBlock)) {
            MiMakeSystemAddressValidPfnWs (CloneBlock, CurrentProcess, OldIrql);
        }

        MutexReleased = TRUE;
    }

    if (ARGUMENT_PRESENT (CloneBlock)) {
        NewCount = InterlockedDecrement (&CloneBlock->CloneRefCount);
    
        ASSERT (NewCount >= 0);
    
        if (NewCount == 0) {
    
            CloneContents = CloneBlock->ProtoPte;
    
            if (CloneContents.u.Long != 0) {
    
                //
                // The last reference to a fork prototype PTE has been removed.
                // Deallocate any page file space and the transition page, if
                // any.
                //
    
                ASSERT (CloneContents.u.Hard.Valid == 0);
    
                //
                // Assert that the PTE is not in subsection format (doesn't
                // point to a file).
                //
    
                ASSERT (CloneContents.u.Soft.Prototype == 0);
    
                if (CloneContents.u.Soft.Transition == 1) {
    
                    //
                    // Prototype PTE in transition, put the page on
                    // the free list.
                    //
    
                    Pfn3 = MI_PFN_ELEMENT (CloneContents.u.Trans.PageFrameNumber);
                    Pfn4 = MI_PFN_ELEMENT (Pfn3->u4.PteFrame);
    
                    MI_SET_PFN_DELETED (Pfn3);
    
                    MiDecrementShareCount (Pfn4, Pfn3->u4.PteFrame);
    
                    //
                    // Check the reference count for the page, if the reference
                    // count is zero and the page is not on the freelist,
                    // move the page to the free list, if the reference
                    // count is not zero, ignore this page.
                    // When the reference count goes to zero, it will be placed
                    // on the free list.
                    //
    
                    if ((Pfn3->u3.e2.ReferenceCount == 0) &&
                        (Pfn3->u3.e1.PageLocation != FreePageList)) {
    
                        MiUnlinkPageFromList (Pfn3);
                        MiReleasePageFileSpace (Pfn3->OriginalPte);
                        MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&CloneContents));
                    }
                }
                else {
    
                    if (IS_PTE_NOT_DEMAND_ZERO (CloneContents)) {
                        MiReleasePageFileSpace (CloneContents);
                    }
                }
            }
        }
    }

    //
    // Decrement the final number of references to the clone descriptor.  The
    // decrement of the NumberOfReferences above serves to decide
    // whether to remove the clone descriptor from the process tree so that
    // a wait on a paged out clone PTE block doesn't let a fork copy the
    // descriptor while it is half-changed.
    //
    // The FinalNumberOfReferences serves as a way to distinguish which
    // thread (multiple threads may have collided waiting for the inpage
    // of the clone PTE block) is the last one to wake up from the wait as
    // only this one (it may not be the same one who drove NumberOfReferences
    // to zero) can finally free the pool safely.
    //

    CloneDescriptor->FinalNumberOfReferences -= 1;

    ASSERT (CloneDescriptor->FinalNumberOfReferences >= 0);

    if (CloneDescriptor->FinalNumberOfReferences == 0) {

        //
        // Flush TBs prior to releasing the PFN lock.
        //

        if ((ARGUMENT_PRESENT (PteFlushList)) && (PteFlushList->Count != 0)) {
            MiFlushPteList (PteFlushList);
        }

        UNLOCK_PFN (OldIrql);

        //
        // There are no longer any PTEs in this process which refer
        // to the fork prototype PTEs for this clone descriptor.
        // Decrement the process reference count in the CloneHeader.
        //

        //
        // The working set lock may have been acquired safely or unsafely
        // by our caller.  Handle both cases here and below.
        //

        CurrentThread = PsGetCurrentThread ();

        UNLOCK_WS_REGARDLESS (CurrentThread, CurrentProcess, WsHeldSafe, WsHeldShared);

        MutexReleased = TRUE;

        CloneHeader = CloneDescriptor->CloneHeader;

        NewCount = InterlockedDecrement (&CloneHeader->NumberOfProcessReferences);
        ASSERT (NewCount >= 0);

        //
        // If the count is zero, there are no more processes pointing
        // to this fork header so blow it away.
        //

        if (NewCount == 0) {

#if DBG
            ULONG i;

            CloneBlock = CloneHeader->ClonePtes;
            for (i = 0; i < CloneHeader->NumberOfPtes; i += 1) {
                ASSERT (CloneBlock->CloneRefCount == 0);
                CloneBlock += 1;
            }
#endif

            ExFreePool (CloneHeader->ClonePtes);

            ExFreePool (CloneHeader);
        }

        //
        // Return the pool for the global structures referenced by the
        // clone descriptor.
        //

        if ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0) {

            //
            // Fork succeeded so return quota that was taken out earlier.
            //

            PsReturnProcessPagedPoolQuota (CurrentProcess,
                                           CloneDescriptor->PagedPoolQuotaCharge);

            PsReturnProcessNonPagedPoolQuota (CurrentProcess,
                                              sizeof(MMCLONE_HEADER));
        }

        ExFreePool (CloneDescriptor);

        //
        // The working set lock may have been acquired safely or unsafely
        // by our caller.  Reacquire it in the same manner our caller did.
        //

        LOCK_WS_REGARDLESS (CurrentThread, CurrentProcess, WsHeldSafe, WsHeldShared);

        LOCK_PFN (OldIrql);
    }


    //
    // Make sure TBs were flushed prior to releasing the PFN lock.
    //

    ASSERT ((MutexReleased == FALSE) ||
            (!ARGUMENT_PRESENT (PteFlushList)) ||
            (PteFlushList->Count == 0));

    return MutexReleased;
}

LOGICAL
MiWaitForForkToComplete (
    IN PEPROCESS CurrentProcess
    )

/*++

Routine Description:

    This routine waits for the current process to complete a fork operation.

Arguments:

    CurrentProcess - Supplies the current process value.

Return Value:

    TRUE if the mutex was released and reacquired to wait.  FALSE if not.

Environment:

    Kernel mode, APCs disabled, working set mutex held.

--*/

{
    PETHREAD CurrentThread;
    LOGICAL WsHeldSafe;
    LOGICAL WsHeldShared;

    //
    // A fork operation is in progress and the count of clone-blocks
    // and other structures may not be changed.  Release the working
    // set mutex and wait for the address creation mutex which governs the
    // fork operation.
    //

    if (CurrentProcess->ForkInProgress == PsGetCurrentThread()) {
        return FALSE;
    }

    CurrentThread = PsGetCurrentThread ();

    //
    // The working set mutex may have been acquired safely or unsafely
    // by our caller.  Handle both cases here and below, carefully making sure
    // that the OldIrql left in the WS mutex on return is the same as on entry.
    //
    // Note it is ok to drop to PASSIVE or APC level here as long as it is
    // not lower than our caller was at.  Using the WorkingSetMutex whose irql
    // field was initialized by our caller ensures that the proper irql
    // environment is maintained (ie: the caller may be blocking APCs
    // deliberately).
    //

    UNLOCK_WS_REGARDLESS (CurrentThread, CurrentProcess, WsHeldSafe, WsHeldShared);

    //
    // Acquire the address creation mutex as this can only succeed when the
    // forking thread is done in MiCloneProcessAddressSpace.  Thus, acquiring
    // this mutex doesn't stop another thread from starting another fork, but
    // it does serve as a way to know the current fork is done (enough).
    //

    LOCK_ADDRESS_SPACE (CurrentProcess);

    UNLOCK_ADDRESS_SPACE (CurrentProcess);

    //
    // The working set lock may have been acquired safely or unsafely
    // by our caller.  Reacquire it in the same manner our caller did.
    //

    LOCK_WS_REGARDLESS (CurrentThread, CurrentProcess, WsHeldSafe, WsHeldShared);

    return TRUE;
}

VOID
MiUpControlAreaRefs (
    IN PMMVAD Vad
    )
{
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PSUBSECTION FirstSubsection;
    PSUBSECTION LastSubsection;

    ControlArea = Vad->ControlArea;

    LOCK_PFN (OldIrql);

    ControlArea->NumberOfMappedViews += 1;
    ControlArea->NumberOfUserReferences += 1;

    if ((ControlArea->u.Flags.Image == 0) &&
        (ControlArea->FilePointer != NULL) &&
        (ControlArea->u.Flags.PhysicalMemory == 0)) {

        FirstSubsection = MiLocateSubsection (Vad, Vad->StartingVpn);

        //
        // Note LastSubsection may be NULL for extendable VADs when
        // the EndingVpn is past the end of the section.  In this
        // case, all the subsections can be safely incremented.
        //
        // Note also that the reference must succeed because each
        // subsection's prototype PTEs are guaranteed to already 
        // exist by virtue of the fact that the creating process
        // already has this VAD currently mapping them.
        //

        LastSubsection = MiLocateSubsection (Vad, Vad->EndingVpn);

        while (FirstSubsection != LastSubsection) {
            MiReferenceSubsection ((PMSUBSECTION) FirstSubsection);
            FirstSubsection = FirstSubsection->NextSubsection;
        }

        if (LastSubsection != NULL) {
            MiReferenceSubsection ((PMSUBSECTION) LastSubsection);
        }
    }

    UNLOCK_PFN (OldIrql);
    return;
}


ULONG
MiDoneWithThisPageGetAnother (
    IN PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    )

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    ULONG ReleasedMutex;

    UNREFERENCED_PARAMETER (PointerPde);

    ReleasedMutex = FALSE;

    if (*PageFrameIndex != (PFN_NUMBER)-1) {

        //
        // Decrement the share count of the last page which
        // we operated on.
        //

        Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);

        LOCK_PFN (OldIrql);

        MiDecrementShareCount (Pfn1, *PageFrameIndex);
    }
    else {
        LOCK_PFN (OldIrql);
    }

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        ReleasedMutex = MiEnsureAvailablePageOrWait (CurrentProcess, OldIrql);
    }

    *PageFrameIndex = MiRemoveZeroPage (
                   MI_PAGE_COLOR_PTE_PROCESS (PointerPde,
                                              &CurrentProcess->NextPageColor));

    //
    // Temporarily mark the page as bad so that contiguous memory allocators
    // won't steal it when we release the PFN lock below.  This also prevents
    // the MiIdentifyPfn code from trying to identify it as we haven't filled
    // in all the fields yet.
    //

    Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);
    Pfn1->u3.e1.PageLocation = BadPageList;

    UNLOCK_PFN (OldIrql);
    return ReleasedMutex;
}

ULONG
MiLeaveThisPageGetAnother (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    )

{
    KIRQL OldIrql;
    ULONG ReleasedMutex;
    PMMPFN Pfn1;

    UNREFERENCED_PARAMETER (PointerPde);

    ReleasedMutex = FALSE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        ReleasedMutex = MiEnsureAvailablePageOrWait (CurrentProcess, OldIrql);
    }

    *PageFrameIndex = MiRemoveZeroPage (
                   MI_PAGE_COLOR_PTE_PROCESS (PointerPde,
                                              &CurrentProcess->NextPageColor));

    //
    // Temporarily mark the page as bad so that contiguous memory allocators
    // won't steal it when we release the PFN lock below.  This also prevents
    // the MiIdentifyPfn code from trying to identify it as we haven't filled
    // in all the fields yet.
    //

    Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);
    Pfn1->u3.e1.PageLocation = BadPageList;

    UNLOCK_PFN (OldIrql);
    return ReleasedMutex;
}

ULONG
MiHandleForkTransitionPte (
    IN PMMPTE PointerPte,
    IN PMMPTE PointerNewPte,
    IN PMMCLONE_BLOCK ForkProtoPte
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    MMPTE PteContents;
    PMMPTE ContainingPte;
    PFN_NUMBER PageTablePage;
    MMPTE TempPte;
    PMMPFN PfnForkPtePage;

    LOCK_PFN (OldIrql);

    //
    // Now that we have the PFN lock which prevents pages from
    // leaving the transition state, examine the PTE again to
    // ensure that it is still transition.
    //

    PteContents = *PointerPte;

    if ((PteContents.u.Soft.Transition == 0) ||
        (PteContents.u.Soft.Prototype == 1)) {

        //
        // The PTE is no longer in transition... do this loop again.
        //

        UNLOCK_PFN (OldIrql);
        return TRUE;
    }

    //
    // The PTE is still in transition, handle like a valid PTE.
    //

    Pfn2 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

    //
    // Assertion that PTE is not in prototype PTE format.
    //

    ASSERT (Pfn2->u3.e1.PrototypePte != 1);

    //
    // This is a private page in transition state,
    // create a fork prototype PTE
    // which becomes the "prototype" PTE for this page.
    //

    ForkProtoPte->ProtoPte = PteContents;

    //
    // Make the protection write-copy if writable.
    //

    MI_MAKE_PROTECT_WRITE_COPY (ForkProtoPte->ProtoPte);

    ForkProtoPte->CloneRefCount = 2;

    //
    // Transform the PFN element to reference this new fork
    // prototype PTE.
    //

    //
    // Decrement the share count for the page table
    // page which contains the PTE as it is no longer
    // valid or in transition.
    //

    Pfn2->PteAddress = &ForkProtoPte->ProtoPte;
    Pfn2->u3.e1.PrototypePte = 1;

    //
    // Make original PTE copy on write.
    //

    MI_MAKE_PROTECT_WRITE_COPY (Pfn2->OriginalPte);

    ContainingPte = MiGetPteAddress (&ForkProtoPte->ProtoPte);

    if (ContainingPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (&ForkProtoPte->ProtoPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940, 
                          (ULONG_PTR)&ForkProtoPte->ProtoPte,
                          (ULONG_PTR)ContainingPte->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(&ForkProtoPte->ProtoPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PageTablePage = Pfn2->u4.PteFrame;

    Pfn2->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPte);

    //
    // Increment the share count for the page containing
    // the fork prototype PTEs as we have just placed
    // a transition PTE into the page.
    //

    PfnForkPtePage = MI_PFN_ELEMENT (ContainingPte->u.Hard.PageFrameNumber);

    PfnForkPtePage->u2.ShareCount += 1;

    TempPte.u.Long = MiProtoAddressForPte (Pfn2->PteAddress);
    TempPte.u.Proto.Prototype = 1;

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);
    MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

    //
    // Decrement the share count for the page table
    // page which contains the PTE as it is no longer
    // valid or in transition.
    //

    Pfn3 = MI_PFN_ELEMENT (PageTablePage);

    MiDecrementShareCount (Pfn3, PageTablePage);

    UNLOCK_PFN (OldIrql);
    return FALSE;
}

VOID
MiDownShareCountFlushEntireTb (
    IN PFN_NUMBER PageFrameIndex
    )

{
    PMMPFN Pfn1;
    KIRQL OldIrql;

    if (PageFrameIndex != (PFN_NUMBER)-1) {

        //
        // Decrement the share count of the last page which
        // we operated on.
        //

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        MiDecrementShareCount (Pfn1, PageFrameIndex);
    }
    else {
        LOCK_PFN (OldIrql);
    }

    MI_FLUSH_PROCESS_TB (FALSE);

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiUpForkPageShareCount (
    IN PMMPFN PfnForkPtePage
    )
{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);
    PfnForkPtePage->u2.ShareCount += 1;

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiBuildForkPageTable (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PMMPTE PointerNewPde,
    IN PFN_NUMBER PdePhysicalPage,
    IN PMMPFN PfnPdPage,
    IN LOGICAL MakeValid
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
#if (_MI_PAGING_LEVELS >= 3)
    MMPTE TempPpe;
#endif

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    //
    // The entire TB must be flushed if we are changing cache attributes.
    //
    // KeFlushSingleTb cannot be used because we don't know
    // what virtual address(es) this physical frame was last mapped at.
    //

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
    }

    //
    // The PFN lock must be held while initializing the
    // frame to prevent those scanning the database for free
    // frames from taking it after we fill in the u2 field.
    //

    LOCK_PFN (OldIrql);

    Pfn1->OriginalPte = DemandZeroPde;
    Pfn1->u2.ShareCount = 1;
    Pfn1->u3.e2.ReferenceCount = 1;
    Pfn1->PteAddress = PointerPde;
    MI_SET_MODIFIED (Pfn1, 1, 0x10);
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    Pfn1->u3.e1.CacheAttribute = MiCached;
    Pfn1->u4.PteFrame = PdePhysicalPage;

    //
    // Increment the share count for the page containing
    // this PTE as the PTE is in transition.
    //

    PfnPdPage->u2.ShareCount += 1;

    UNLOCK_PFN (OldIrql);

    if (MakeValid == TRUE) {

#if (_MI_PAGING_LEVELS >= 3)

        //
        // Put the PPE into the valid state as it will point at a page
        // directory page that is valid (not transition) when the fork is
        // complete.  All the page table pages will be in transition, but
        // the page directories cannot be as they contain the PTEs for the
        // page tables.
        //
    
        TempPpe = ValidPdePde;

        MI_MAKE_VALID_PTE (TempPpe,
                           PageFrameIndex,
                           MM_READWRITE,
                           PointerPde);
    
        MI_SET_PTE_DIRTY (TempPpe);

        //
        // Make the PTE owned by user mode.
        //
    
        MI_SET_OWNER_IN_PTE (PointerNewPde, MI_PTE_OWNER_USER);

        MI_WRITE_VALID_PTE (PointerNewPde, TempPpe);
#endif
    }
    else {

        //
        // Put the PDE into the transition state as it is not
        // really mapped and decrement share count does not
        // put private pages into transition, only prototypes.
        //
    
        MI_WRITE_INVALID_PTE (PointerNewPde, TransitionPde);

        //
        // Make the PTE owned by user mode.
        //
    
        MI_SET_OWNER_IN_PTE (PointerNewPde, MI_PTE_OWNER_USER);

        PointerNewPde->u.Trans.PageFrameNumber = PageFrameIndex;
    }

    return;
}

