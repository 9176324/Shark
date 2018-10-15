/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   sessload.c

Abstract:

    This module contains the routines which implement the loading of
    session space drivers.

--*/

#include "mi.h"

//
// This tracks allocated group virtual addresses.  The term SESSIONWIDE is used
// to denote data that is the same across all sessions (as opposed to
// per-session data which can vary from session to session).
//
// Since each driver loaded into a session space is linked and fixed up
// against the system image, it must remain at the same virtual address
// across the system regardless of the session.
//
// Access to these structures are generally guarded by the MmSystemLoadLock.
//

RTL_BITMAP MiSessionWideVaBitMap;

ULONG MiSessionUserCollisions;

NTSTATUS
MiSessionRemoveImage (
    IN PVOID BaseAddress
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiSessionWideInitializeAddresses)

#pragma alloc_text(PAGE, MiSessionWideReserveImageAddress)
#pragma alloc_text(PAGE, MiRemoveImageSessionWide)
#pragma alloc_text(PAGE, MiShareSessionImage)

#pragma alloc_text(PAGE, MiSessionLookupImage)
#pragma alloc_text(PAGE, MiSessionUnloadAllImages)
#endif


LOGICAL
MiMarkImageInSystem (
    IN PCONTROL_AREA ControlArea
    )

/*++

Routine Description:

    This routine marks the given image as mapped into system space.

Arguments:

    ControlArea - Supplies the relevant control area.

Return Value:

    TRUE on success, FALSE on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.

--*/

{
    LOGICAL Status;
    KIRQL OldIrql;

    ASSERT (ControlArea->u.Flags.ImageMappedInSystemSpace == 0);

    Status = TRUE;

    //
    // Lock synchronization is not needed for our callers as they always hold
    // the system load mutant - but it is needed to modify this field in the
    // control area as other threads may be modifying other bits in the flags.
    //

    LOCK_PFN (OldIrql);

    //
    // Before handling the relocations for this image, ensure it
    // is not mapped in user space anywhere.  Note we have 1 user
    // reference at this point, so any beyond that are someone
    // elses and force us to pagefile-back this image.
    //

    if (ControlArea->NumberOfUserReferences <= 1) {

        ControlArea->u.Flags.ImageMappedInSystemSpace = 1;
        
        //
        // This flag is set so when the image is removed from the loaded
        // module list, the control area is destroyed.  This is required
        // because images mapped in session space inherit their PTE protections
        // from the shared prototype PTEs.
        //
        // Consider the following scenario :
        //
        // If image A is loaded at its based (preferred) address, and then
        // unloaded.  Image B is not rebased properly and then loads at
        // image A's preferred address.  Image A then reloads.
        //
        // Now image A cannot use the original prototype PTEs which enforce
        // readonly code, etc, because fixups will need to be done on it.
        //
        // Setting DeleteOnClose solves this problem by simply destroying
        // the entire control area on last unload.
        //
    
        ControlArea->u.Flags.DeleteOnClose = 1;
    }
    else {
        Status = FALSE;
    }

    UNLOCK_PFN (OldIrql);

    return Status;
}

NTSTATUS
MiShareSessionImage (
    IN PVOID MappedBase,
    IN PSECTION Section
    )

/*++

Routine Description:

    This routine maps the given image into the current session space.
    This allows the image to be executed backed by the image file in the
    filesystem and allow code and read-only data to be shared.

Arguments:

    MappedBase - Supplies the base address the image is to be mapped at.

    Section - Supplies a pointer to a section.

Return Value:

    Returns STATUS_SUCCESS on success, various NTSTATUS codes on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.

--*/

{
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PFN_NUMBER NumberOfPtes;
    PMMPTE StartPte;
#if DBG
    PMMPTE EndPte;
#endif
    SIZE_T AllocationSize;
    NTSTATUS Status;
    SIZE_T CommittedPages;
    LOGICAL Relocated;
    PIMAGE_ENTRY_IN_SESSION DriverImage;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    if (MappedBase != Section->Segment->BasedAddress) {
        Relocated = TRUE;
    }
    else {
        Relocated = FALSE;
    }

    ASSERT (BYTE_OFFSET (MappedBase) == 0);

    //
    // Check to see if a purge operation is in progress and if so, wait
    // for the purge to complete.  In addition, up the count of mapped
    // views for this control area.
    //

    ControlArea = Section->Segment->ControlArea;

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    Status = MiCheckPurgeAndUpMapCount (ControlArea, FALSE);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NumberOfPtes = Section->Segment->TotalNumberOfPtes;
    AllocationSize = NumberOfPtes << PAGE_SHIFT;

    //
    // Calculate the PTE ranges and amount.
    //

    StartPte = MiGetPteAddress (MappedBase);

    //
    // The image commitment will be the same as the number of PTEs if the
    // image was not linked with native page alignment for the subsections.
    //
    // If it is linked native, then the commit will just be the number of
    // writable pages.  Note for this case, if we need to relocate it, then
    // we need to charge for the full number of PTEs and bump the commit
    // charge in the segment so that the return on unload is correct also.
    //

    ASSERT (Section->Segment->u1.ImageCommitment != 0);
    ASSERT (Section->Segment->u1.ImageCommitment <= NumberOfPtes);

    if (Relocated == TRUE) {
        CommittedPages = NumberOfPtes;
    }
    else {
        CommittedPages = Section->Segment->u1.ImageCommitment;
    }

    if (MiChargeCommitment (CommittedPages, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);

        //
        // Don't bother releasing the page tables or their commit here, another
        // load will happen shortly or the whole session will go away.  On
        // session exit everything will be released automatically.
        //

        MiDereferenceControlArea (ControlArea);
        return STATUS_NO_MEMORY;
    }

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                 CommittedPages);

    //
    // Make sure we have page tables for the PTE
    // entries we must fill in the session space structure.
    //

    Status = MiSessionCommitPageTables (MappedBase,
                                        (PVOID)((PCHAR)MappedBase + AllocationSize));

    if (!NT_SUCCESS(Status)) {

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MiDereferenceControlArea (ControlArea);
        MiReturnCommitment (CommittedPages);

        return STATUS_NO_MEMORY;
    }

#if DBG
    EndPte = StartPte + NumberOfPtes;
    while (StartPte < EndPte) {
        ASSERT (StartPte->u.Long == 0);
        StartPte += 1;
    }

    StartPte = MiGetPteAddress (MappedBase);
#endif

    //
    // If the image was linked with subsection alignment of >= PAGE_SIZE,
    // then all of the prototype PTEs were initialized to proper protections by
    // the initial section creation.  The protections in these PTEs are used
    // to fill the actual PTEs as each address is faulted on.
    //
    // If the image has less than PAGE_SIZE section alignment, then 
    // section creation uses a single subsection to map the entire file and
    // sets all the prototype PTEs to copy on write.  For this case, the
    // appropriate permissions are set by the MiWriteProtectSystemImage below.
    //

    //
    // Initialize the PTEs to point at the prototype PTEs.
    //

    Status = MiAddMappedPtes (StartPte, NumberOfPtes, ControlArea);

    if (!NT_SUCCESS (Status)) {

        //
        // Regardless of whether the PTEs were mapped, leave the control area
        // marked as mapped in system space so user applications cannot map the
        // file as an image as clearly the intent is to run it as a driver.
        //

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MiDereferenceControlArea (ControlArea);
        MiReturnCommitment (CommittedPages);
    	return Status;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_SHARED_IMAGE, CommittedPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_SYSMAPPED_PAGES_COMMITTED, (ULONG)CommittedPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_SYSMAPPED_PAGES_ALLOC, (ULONG)NumberOfPtes);

    //
    // No session space image faults may be taken until these fields of the
    // image entry are initialized.
    //

    DriverImage = MiSessionLookupImage (MappedBase);
    ASSERT (DriverImage);

    DriverImage->LastAddress = (PVOID)((PCHAR)MappedBase + AllocationSize - 1);
    DriverImage->PrototypePtes = Subsection->SubsectionBase;

    //
    // The loaded module list section reference protects the image from
    // being purged while it's in use.
    //

    MiDereferenceControlArea (ControlArea);

    return STATUS_SUCCESS;
}


NTSTATUS
MiSessionInsertImage (
    IN PVOID BaseAddress
    )

/*++

Routine Description:

    This routine allocates an image entry for the specified address in the
    current session space.

Arguments:

    BaseAddress - Supplies the base address for the executable image.

Return Value:

    STATUS_SUCCESS or various NTSTATUS error codes on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.
    
    Note both the system load resource and the session working set
    mutex must be held to modify the list of images in this session.
    Either may be held to safely walk the list.

--*/

{
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;
    PIMAGE_ENTRY_IN_SESSION NewImage;
    PETHREAD Thread;
    PMMSUPPORT Ws;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    Thread = PsGetCurrentThread ();

    //
    // Create and initialize a new image entry prior to acquiring the session
    // space ws mutex.  This is to reduce the amount of time the mutex is held.
    // If an existing entry is found this allocation is just discarded.
    //

    NewImage = ExAllocatePoolWithTag (NonPagedPool,
                                      sizeof(IMAGE_ENTRY_IN_SESSION),
                                      'iHmM');

    if (NewImage == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (NewImage, sizeof(IMAGE_ENTRY_IN_SESSION));

    NewImage->Address = BaseAddress;
    NewImage->ImageCountInThisSession = 1;

    //
    // Check to see if the address is already loaded.
    //

    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;

    LOCK_WORKING_SET (Thread, Ws);

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {
        Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddress) {
            Image->ImageCountInThisSession += 1;
            UNLOCK_WORKING_SET (Thread, Ws);
            ExFreePool (NewImage);
            return STATUS_ALREADY_COMMITTED;
        }
        NextEntry = NextEntry->Flink;
    }

    //
    // Insert the image entry into the session space structure.
    //

    InsertTailList (&MmSessionSpace->ImageList, &NewImage->Link);

    UNLOCK_WORKING_SET (Thread, Ws);

    return STATUS_SUCCESS;
}


NTSTATUS
MiSessionRemoveImage (
    PVOID BaseAddr
    )

/*++

Routine Description:

    This routine removes the given image entry from the current session space.

Arguments:

    BaseAddress - Supplies the base address for the executable image.

Return Value:

    Returns STATUS_SUCCESS on success, STATUS_NOT_FOUND if the image is not
    in the current session space.

Environment:

    Kernel mode, APC_LEVEL and below.

    Note both the system load resource and the session working set
    mutex must be held to modify the list of images in this session.
    Either may be held to safely walk the list.

--*/

{
    PETHREAD Thread;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;
    PMMSUPPORT Ws;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    Thread = PsGetCurrentThread ();

    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;

    LOCK_WORKING_SET (Thread, Ws);

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddr) {

            RemoveEntryList (NextEntry);

            UNLOCK_WORKING_SET (Thread, Ws);

            ASSERT (MmSessionSpace->ImageLoadingCount >= 0);

            if (Image->ImageLoading == TRUE) {
                ASSERT (MmSessionSpace->ImageLoadingCount > 0);
                InterlockedDecrement (&MmSessionSpace->ImageLoadingCount);
            }

            ExFreePool (Image);
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_WORKING_SET (Thread, Ws);

    return STATUS_NOT_FOUND;
}


PIMAGE_ENTRY_IN_SESSION
MiSessionLookupImage (
    IN PVOID BaseAddress
    )

/*++

Routine Description:

    This routine looks up the image entry within the current session by the 
    specified base address.

Arguments:

    BaseAddress - Supplies the base address for the executable image.

Return Value:

    The image entry within this session on success or NULL on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.

    Note both the system load resource and the session working set
    mutex must be held to modify the list of images in this session.
    Either may be held to safely walk the list.

--*/

{
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

    SYSLOAD_LOCK_OWNED_BY_ME ();

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddress) {
            return Image;
        }

        NextEntry = NextEntry->Flink;
    }

    return NULL;
}


VOID
MiSessionUnloadAllImages (
    VOID
    )

/*++

Routine Description:

    This routine dereferences each image that has been loaded in the
    current session space.

    As each image is dereferenced, checks are made:

    If this session's reference count to the image reaches zero, the VA
    range in this session is deleted.  If the reference count to the image
    in the SESSIONWIDE list drops to zero, then the SESSIONWIDE's VA
    reservation is removed and the address space is made available to any
    new image.

    If this is the last systemwide reference to the driver then the driver
    is deleted from memory.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.  This is called in one of two contexts:
        1. the last thread in the last process of the current session space.
        2. or by any thread in the SMSS process.

    Note both the system load resource and the session working set
    mutex must be held to modify the list of images in this session.
    Either may be held to safely walk the list.

--*/

{
    NTSTATUS Status;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Module;
    PKLDR_DATA_TABLE_ENTRY ImageHandle;

    ASSERT (MmSessionSpace->ReferenceCount == 0);

    //
    // The session's working set lock does not need to be acquired here since
    // no thread can be faulting on these addresses.
    //

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Module = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        //
        // Lookup the image entry in the system PsLoadedModuleList,
        // unload the image and delete it.
        //

        ImageHandle = MiLookupDataTableEntry (Module->Address, FALSE);

        ASSERT (ImageHandle);

        Status = MmUnloadSystemImage (ImageHandle);

        //
        // Restart the search at the beginning since the entry has been deleted.
        //

        ASSERT (MmSessionSpace->ReferenceCount == 0);

        NextEntry = MmSessionSpace->ImageList.Flink;
    }
}


VOID
MiSessionWideInitializeAddresses (
    VOID
    )

/*++

Routine Description:

    This routine is called at system initialization to set up the group
    address list.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    PVOID Bitmap;
    SIZE_T NumberOfPages;

    NumberOfPages = (MiSessionImageEnd - MiSessionImageStart) >> PAGE_SHIFT;

    Bitmap = ExAllocatePoolWithTag (PagedPool,
                                    ((NumberOfPages + 31) / 32) * 4,
                                    '  mM');

    if (Bitmap == NULL) {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      0x301);
    }

    RtlInitializeBitMap (&MiSessionWideVaBitMap,
                         Bitmap,
                         (ULONG) NumberOfPages);

    RtlClearAllBits (&MiSessionWideVaBitMap);

    return;
}

NTSTATUS
MiSessionWideReserveImageAddress (
    IN PSECTION Section,
    OUT PVOID *AssignedAddress,
    OUT PSECTION *NewSectionPointer
    )

/*++

Routine Description:

    This routine allocates a range of virtual address space within
    session space.  This address range is unique system-wide and in this
    manner, code and pristine data of session drivers can be shared across
    multiple sessions.

    This routine does not actually commit pages, but reserves the virtual
    address region for the named image.  An entry is created here and attached
    to the current session space to track the loaded image.  Thus if all
    the references to a given range go away, the range can then be reused.

Arguments:

    Section - Supplies the section (and thus, the preferred address that the
              driver has been linked (rebased) at.  If this address is
              available, the driver will require no relocation.  The section
              is also used to derive the number of bytes to reserve.

    AssignedAddress - Supplies a pointer to a variable that receives the
                      allocated address if the routine succeeds.

Return Value:

    Returns STATUS_SUCCESS on success, various NTSTATUS codes on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.

--*/

{
    ULONG StartPosition;
    ULONG NumberOfPtes;
    NTSTATUS Status;
    PWCHAR pName;
    PVOID NewAddress;
    ULONG_PTR SessionSpaceEnd;
    PVOID PreferredAddress;
    PCONTROL_AREA ControlArea;
    PIMAGE_ENTRY_IN_SESSION Image;
    PSESSION_GLOBAL_SUBSECTION_INFO GlobalSubs;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION);
    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);
    ASSERT (Section->u.Flags.Image == 1);

    *NewSectionPointer = NULL;

    ControlArea = Section->Segment->ControlArea;

    if (ControlArea->u.Flags.ImageMappedInSystemSpace == 1) {

        //
        // We are going to add a new entry to the loaded module list.  We
        // have a section in hand.  The case which must be handled carefully is:
        //
        // When a session image unloads, its entry is removed from the
        // loaded module list, but the section object itself may continue
        // to live on due to other references on the object.  For session
        // images, the relocations and import snaps, verifier updates, etc,
        // are done directly to the prototype PTEs (the modified ones become
        // backed by the pagefile) at whatever address the image is loaded at.
        //
        // Thus, if the image's load address the second time around is different
        // from the first time around, this presents problems.  Likewise, any
        // image it imports from is an issue, since if their load addresses
        // change, the IAT snaps need updating.  This only gets more complicated
        // with recursive imports, imports where only some of the images have
        // lingering object reference counts, etc.
        //
        // There is a lingering object reference to this image since it
        // was last unloaded.  It's ok just to fail this since it
        // can't be a user-generated reference and any kernel/driver
        // SEC_IMAGE reference would be extremely unusual (and short lived).
        //

        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_IMAGE_ZOMBIE);

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        return STATUS_CONFLICTING_ADDRESSES;
    }

    pName = NULL;

    StartPosition = NO_BITS_FOUND;
    PreferredAddress = Section->Segment->BasedAddress;
    NumberOfPtes = Section->Segment->TotalNumberOfPtes;

    SessionSpaceEnd = MiSessionImageEnd;

    //
    // Try to put the module into its requested address so it can be shared.
    //
    // If the requested address is not properly aligned or not in the session
    // space region, pick an address for it.  This image will not be shared.
    //

    if ((BYTE_OFFSET (PreferredAddress) == 0) &&
        (PreferredAddress >= (PVOID) MiSessionImageStart) &&
	    (PreferredAddress < (PVOID) MiSessionImageEnd)) {

        StartPosition = (ULONG) ((ULONG_PTR) PreferredAddress - MiSessionImageStart) >> PAGE_SHIFT;

        if (RtlAreBitsClear (&MiSessionWideVaBitMap,
                             StartPosition,
                             NumberOfPtes) == TRUE) {

            RtlSetBits (&MiSessionWideVaBitMap,
                        StartPosition,
                        NumberOfPtes);
        }
        else {
            PreferredAddress = NULL;
        }
    }
    else {
        PreferredAddress = NULL;
    }

    if (PreferredAddress == NULL) {

        StartPosition = RtlFindClearBitsAndSet (&MiSessionWideVaBitMap,
                                                NumberOfPtes,
                                                0);

        if (StartPosition == NO_BITS_FOUND) {
            MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_IMAGE_VA_SPACE);
            return STATUS_NO_MEMORY;
        }

    }

    NewAddress = (PVOID) (MiSessionImageStart + ((ULONG_PTR) StartPosition << PAGE_SHIFT));

    //
    // Create an entry for this image in the current session space.
    //

    Status = MiSessionInsertImage (NewAddress);

    if (!NT_SUCCESS (Status)) {

Failure1:
        ASSERT (RtlAreBitsSet (&MiSessionWideVaBitMap,
                               StartPosition,
                               NumberOfPtes) == TRUE);

        RtlClearBits (&MiSessionWideVaBitMap,
                      StartPosition,
                      NumberOfPtes);

        return Status;
    }

    *AssignedAddress = NewAddress;

    GlobalSubs = NULL;

    //
    // This is the first load of this image in any session, so mark this
    // image so that copy-on-write flows through into read-write until
    // relocations (if any) and import image resolution is finished updating
    // all parts of this image.  This way all future instantiations of this
    // image won't need to reprocess them (and can share the pages).
    // This is deliberately done this way so that any concurrent usermode
    // access to this image does not receive direct read-write privilege.
    //
    // Note images with less than native page subsection alignment are
    // currently marked copy on write for the entire image.  Native page
    // aligned images have individual subsections with associated
    // permissions.  Both image types get temporarily mapped read-write in
    // their first session mapping.
    //
    // After everything (relocations & image imports) is done, then
    // the real permissions (based on the PE header) can be applied and
    // the real PTEs automatically inherit the proper permissions
    // from the prototype PTEs.
    //
    // Since the fixups are only done once, they can then be
    // shared by any subsequent driver instantiations.  Note that
    // any fixed-up pages are never written to the image, but are
    // instead converted to pagefile backing by the modified writer.
    //

    if (MiMarkImageInSystem (ControlArea) == FALSE) {

        ULONG Count;
        SIZE_T ViewSize;
        PVOID SrcVa;
        PVOID DestVa;
        PVOID SourceVa;
        PVOID DestinationVa;
        MMPTE PteContents;
        PMMPTE ProtoPte;
        PMMPTE PointerPte;
        PMMPTE LastPte;
        PFN_NUMBER ResidentPages;
        HANDLE NewSectionHandle;
        LARGE_INTEGER MaximumSectionSize;
        OBJECT_ATTRIBUTES ObjectAttributes;
        PSUBSECTION Subsection;
        PSUBSECTION SubsectionBase;
        PMMPTE PrototypePteBase;

        if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
            (ControlArea->u.Flags.Rom == 0)) {
            Subsection = (PSUBSECTION)(ControlArea + 1);
        }
        else {
            Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }

        SubsectionBase = Subsection;
        PrototypePteBase = Subsection->SubsectionBase;

        //
        // Count the number of global subsections.
        //

        Count = 0;

        do {

            if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {
                Count += 1;
            }

            Subsection = Subsection->NextSubsection;

        } while (Subsection != NULL);

        //
        // Allocate pool to store the global subsection information as this
        // multi subsection image is going to be converted to a single
        // subsection pagefile section.
        //

        if (Count != 0) {

            GlobalSubs = ExAllocatePoolWithTag (PagedPool,
                                                (Count + 1) * sizeof (SESSION_GLOBAL_SUBSECTION_INFO),
                                                'sGmM');

            if (GlobalSubs == NULL) {
                MiSessionRemoveImage (NewAddress);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Failure1;
            }

            GlobalSubs[Count].PteCount = 0;     // NULL-terminate the list.
            Count -= 1;

            Subsection = SubsectionBase;

            do {

                if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {

                    GlobalSubs[Count].PteIndex = Subsection->SubsectionBase - PrototypePteBase;
                    GlobalSubs[Count].PteCount = Subsection->PtesInSubsection;
                    GlobalSubs[Count].Protection = Subsection->u.SubsectionFlags.Protection;

                    if (Count == 0) {
                        break;
                    }
                    Count -= 1;
                }

                Subsection = Subsection->NextSubsection;

            } while (Subsection != NULL);
            ASSERT (Count == 0);
        }

        MaximumSectionSize.QuadPart = ((ULONG64) NumberOfPtes) << PAGE_SHIFT;
        ViewSize = 0;

        InitializeObjectAttributes (&ObjectAttributes,
                                    NULL,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

        //
        // Create a pagefile-backed section to copy the image into.
        //

        Status = ZwCreateSection (&NewSectionHandle,
                                  SECTION_ALL_ACCESS,
                                  &ObjectAttributes,
                                  &MaximumSectionSize,
                                  PAGE_EXECUTE_READWRITE,
                                  SEC_COMMIT,
                                  NULL);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

        //
        // Now reference the section handle.  If this fails something is
        // very wrong because it is a kernel handle.
        //
        // N.B.  ObRef sets SectionPointer to NULL on failure.
        //

        Status = ObReferenceObjectByHandle (NewSectionHandle,
                                            SECTION_MAP_EXECUTE,
                                            MmSectionObjectType,
                                            KernelMode,
                                            (PVOID *) NewSectionPointer,
                                            (POBJECT_HANDLE_INFORMATION) NULL);

        ZwClose (NewSectionHandle);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

        //
        // Map the destination.  Deliberately put the destination in system
        // space and the source in session space to increase the chances
        // that enough virtual address space can be found.
        //

        Status = MmMapViewInSystemSpace (*NewSectionPointer,
                                         &DestinationVa,
                                         &ViewSize);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            ObDereferenceObject (*NewSectionPointer);
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

        //
        // Map the source.  Note this choice of session view space is crucial
        // for the mapping because MiResolveDemandZeroFault explictly checks
        // for fault addresses in this range to decide whether to provide a
        // zero (vs just a free) page to the caller.
        //

        Status = MmMapViewInSessionSpace (Section, &SourceVa, &ViewSize);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MmUnmapViewInSystemSpace (DestinationVa);
            ObDereferenceObject (*NewSectionPointer);
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

        //
        // Copy the pristine executable.
        //

        ProtoPte = Section->Segment->PrototypePte;
        LastPte = ProtoPte + NumberOfPtes;
        SrcVa = SourceVa;
        DestVa = DestinationVa;

        while (ProtoPte < LastPte) {

            PteContents = *ProtoPte;

            if ((PteContents.u.Hard.Valid == 1) ||
                (PteContents.u.Soft.Protection != MM_NOACCESS)) {

                RtlCopyMemory (DestVa, SrcVa, PAGE_SIZE);
            }
            else {

                //
                // The source PTE is no access, just leave the destination
                // PTE as demand zero.
                //
            }

            ProtoPte += 1;
            SrcVa = ((PCHAR)SrcVa + PAGE_SIZE);
            DestVa = ((PCHAR)DestVa + PAGE_SIZE);
        }

        Status = MmUnmapViewInSystemSpace (DestinationVa);

        if (!NT_SUCCESS (Status)) {
            ASSERT (FALSE);
        }

        //
        // Delete the source image pages as the BSS ones have been expanded
        // into private demand zero as part of the copy above.  If we don't
        // delete them all, the private demand zero would go on the modified
        // list with a PTE address pointing at the session view space which
        // will have been reused.
        //

        PointerPte = MiGetPteAddress (SourceVa);

        MiDeleteSystemPageableVm (PointerPte,
                                 NumberOfPtes,
                                 MI_DELETE_FLUSH_TB,
                                 &ResidentPages);

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE);

        Status = MmUnmapViewInSessionSpace (SourceVa);

        if (!NT_SUCCESS (Status)) {
            ASSERT (FALSE);
        }

        //
        // Our caller will be using the new pagefile-backed section we
        // just created.  Copy over useful fields now and then dereference
        // the entry section.
        //

        ((PSECTION)*NewSectionPointer)->Segment->u1.ImageCommitment =
                                        Section->Segment->u1.ImageCommitment;

        ((PSECTION)*NewSectionPointer)->Segment->BasedAddress =
                                        Section->Segment->BasedAddress;

        ObDereferenceObject (Section);
    }
    else {
        *NewSectionPointer = NULL;
    }

    Image = MiSessionLookupImage (NewAddress);

    if (Image != NULL) {

        ASSERT (Image->GlobalSubs == NULL);
        Image->GlobalSubs = GlobalSubs;

        ASSERT (Image->ImageLoading == FALSE);
        Image->ImageLoading = TRUE;

        ASSERT (MmSessionSpace->ImageLoadingCount >= 0);
        InterlockedIncrement (&MmSessionSpace->ImageLoadingCount);
    }
    else {
        ASSERT (FALSE);
    }

    return STATUS_SUCCESS;
}

VOID
MiRemoveImageSessionWide (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry OPTIONAL,
    IN PVOID BaseAddress,
    IN ULONG_PTR NumberOfBytes
    )

/*++

Routine Description:

    Delete the image space region from the current session space.
    This dereferences the globally allocated SessionWide region.
    
    The SessionWide region will be deleted if the reference count goes to zero.
    
Arguments:

    DataTableEntry - Supplies the (optional) loader entry.

    BaseAddress - Supplies the address the driver is loaded at.

    NumberOfBytes - Supplies the number of bytes used by the driver.

Return Value:

    Returns STATUS_SUCCESS on success, STATUS_NOT_FOUND on failure.

Environment:

    Kernel mode, APC_LEVEL and below, MmSystemLoadLock held.

--*/

{
    ULONG StartPosition;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    //
    // There is no data table entry if we are encountering an error during
    // the driver's first load (one hasn't been created yet).  But we still
    // need to clear out the in-use bits.
    //

    if ((DataTableEntry == NULL) || (DataTableEntry->LoadCount == 1)) {

        StartPosition = (ULONG)(((ULONG_PTR) BaseAddress - MiSessionImageStart) >> PAGE_SHIFT);

        ASSERT (RtlAreBitsSet (&MiSessionWideVaBitMap,
                               StartPosition,
                               (ULONG) (NumberOfBytes >> PAGE_SHIFT)) == TRUE);

        RtlClearBits (&MiSessionWideVaBitMap,
                      StartPosition,
                      (ULONG) (NumberOfBytes >> PAGE_SHIFT));
    }

    //
    // Remove the image reference from the current session space.
    //

    MiSessionRemoveImage (BaseAddress);

    return;
}

