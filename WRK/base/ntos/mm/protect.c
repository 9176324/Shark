/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   protect.c

Abstract:

    This module contains the routines which implement the
    NtProtectVirtualMemory service.

--*/

#include "mi.h"

#if defined (_WIN64)
#define _MI_RESET_USER_STACK_LIMIT 1
#endif

VOID
MiFlushTbAndCapture (
    IN PMMVAD FoundVad,
    IN PMMPTE PtePointer,
    IN ULONG ProtectionMask,
    IN PMMPFN Pfn1,
    IN LOGICAL CaptureDirtyBit
    );

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

extern CCHAR MmReadWrite[32];

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtProtectVirtualMemory)
#pragma alloc_text(PAGE,MiCheckSecuredVad)
#endif


NTSTATUS
NtProtectVirtualMemory(
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in WIN32_PROTECTION_MASK NewProtectWin32,
    __out PULONG OldProtect
    )

/*++

Routine Description:

    This routine changes the protection on a region of committed pages
    within the virtual address space of the subject process.  Setting
    the protection on a range of pages causes the old protection to be
    replaced by the specified protection value.

    Note if a virtual address is locked in the working set and the
    protection is changed to no access, the page is removed from the
    working set since valid pages can't be no access.

Arguments:

     ProcessHandle - An open handle to a process object.

     BaseAddress - The base address of the region of pages
                   whose protection is to be changed. This value is
                   rounded down to the next host page address
                   boundary.

     RegionSize - A pointer to a variable that will receive
                  the actual size in bytes of the protected region
                  of pages. The initial value of this argument is
                  rounded up to the next host page size boundary.

     NewProtectWin32 - The new protection desired for the specified region of pages.

     Protect Values

          PAGE_NOACCESS - No access to the specified region
                          of pages is allowed. An attempt to read,
                          write, or execute the specified region
                          results in an access violation.

          PAGE_EXECUTE - Execute access to the specified
                         region of pages is allowed. An attempt to
                         read or write the specified region results in
                         an access violation.

          PAGE_READONLY - Read only and execute access to the
                          specified region of pages is allowed. An
                          attempt to write the specified region results
                          in an access violation.

          PAGE_READWRITE - Read, write, and execute access to
                           the specified region of pages is allowed. If
                           write access to the underlying section is
                           allowed, then a single copy of the pages are
                           shared. Otherwise the pages are shared read
                           only/copy on write.

          PAGE_GUARD - Read, write, and execute access to the
                       specified region of pages is allowed,
                       however, access to the region causes a "guard
                       region entered" condition to be raised in the
                       subject process. If write access to the
                       underlying section is allowed, then a single
                       copy of the pages are shared. Otherwise the
                       pages are shared read only/copy on write.

          PAGE_NOCACHE - The page should be treated as uncached.
                         This is only valid for non-shared pages.

          PAGE_WRITECOMBINE - The page should be treated as write-combined.
                              This is only valid for non-shared pages.

     OldProtect - A pointer to a variable that will receive
                  the old protection of the first page within the
                  specified region of pages.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode.

--*/

{
    KAPC_STATE ApcState;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG Attached = FALSE;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    ULONG ProtectionMask;
    ULONG LastProtect;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    PAGED_CODE();

    //
    // Check the protection field.
    //

    ProtectionMask = MiMakeProtectionMask (NewProtectWin32);

    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

        //
        // Capture the region size and base address under an exception handler.
        //

        try {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
            ProbeForWriteUlong (OldProtect);

            //
            // Capture the region size and base address.
            //

            CapturedBase = *BaseAddress;
            CapturedRegionSize = *RegionSize;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // If an exception occurs during the probe or capture
            // of the initial values, then handle the exception and
            // return the exception code as the status value.
            //

            return GetExceptionCode();
        }
    }
    else {

        //
        // Capture the region size and base address.
        //

        CapturedRegionSize = *RegionSize;
        CapturedBase = *BaseAddress;
    }

    //
    // Make sure the specified starting and ending addresses are
    // within the user part of the virtual address space.
    //

    if (CapturedBase > MM_HIGHEST_USER_ADDRESS) {

        //
        // Invalid base address.
        //

        return STATUS_INVALID_PARAMETER_2;
    }

    if ((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)CapturedBase <
                  CapturedRegionSize) {

        //
        // Invalid region size;
        //

        return STATUS_INVALID_PARAMETER_3;
    }

    if (CapturedRegionSize == 0) {
        return STATUS_INVALID_PARAMETER_3;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        PreviousMode,
                                        (PVOID *)&Process,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    //
    // If the specified process is not the current process, attach
    // to the specified process.
    //

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    Status = MiProtectVirtualMemory (Process,
                                     &CapturedBase,
                                     &CapturedRegionSize,
                                     NewProtectWin32,
                                     &LastProtect);


    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    ObDereferenceObject (Process);

    //
    // Establish an exception handler and write the size and base
    // address.
    //

    try {

        *RegionSize = CapturedRegionSize;
        *BaseAddress = CapturedBase;
        *OldProtect = LastProtect;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    return Status;
}


NTSTATUS
MiProtectVirtualMemory (
    IN PEPROCESS Process,
    IN PVOID *BaseAddress,
    IN PSIZE_T RegionSize,
    IN WIN32_PROTECTION_MASK NewProtectWin32,
    IN PWIN32_PROTECTION_MASK LastProtect
    )

/*++

Routine Description:

    This routine changes the protection on a region of committed pages
    within the virtual address space of the subject process.  Setting
    the protection on a range of pages causes the old protection to be
    replaced by the specified protection value.

Arguments:

    Process - Supplies a pointer to the current process.

    BaseAddress - Supplies the starting address to protect.

    RegionsSize - Supplies the size of the region to protect.

    NewProtectWin32 - Supplies the new protection to set.

    LastProtect - Supplies the address of a kernel owned pointer to
                  store (without probing) the old protection into.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, APC_LEVEL or below.

--*/

{
    PETHREAD Thread;
    PMMVAD FoundVad;
    PVOID VirtualAddress;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    NTSTATUS Status;
    ULONG Attached;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerProtoPte;
    PMMPTE LastProtoPte;
    PMMPFN Pfn1;
    WIN32_PROTECTION_MASK CapturedOldProtect;
    ULONG ProtectionMask;
    MMPTE PteContents;
    ULONG Locked;
    PVOID Va;
    ULONG DoAgain;
    PVOID UsedPageTableHandle;
    WIN32_PROTECTION_MASK OriginalProtect;
    LOGICAL WsHeld;
#if defined (_MI_RESET_USER_STACK_LIMIT)
    PTEB Teb;
    PVOID StackLimit;
#endif
    Attached = FALSE;
    Locked = FALSE;

    //
    // Get the address creation mutex to block multiple threads from
    // creating or deleting address space at the same time.
    // Get the working set mutex so PTEs can be modified.
    // Block APCs so an APC which takes a page
    // fault does not corrupt various structures.
    //

    CapturedBase = *BaseAddress;
    CapturedRegionSize = *RegionSize;
    OriginalProtect = NewProtectWin32;

    //
    // Note the NO_CACHE, write combined and GUARD attributes are mutually
    // exclusive.  MiMakeProtectionMask enforces this.
    //

    ProtectionMask = MiMakeProtectionMask (NewProtectWin32);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

    StartingAddress = (PVOID)PAGE_ALIGN(CapturedBase);

#if defined (_MI_RESET_USER_STACK_LIMIT)

    //
    // If a guard page is being set above the current stack limit in the
    // caller's thread stack then he may be trying to recover from a
    // user initiated stack probe growth (ie, inside his exception handler).
    // Reset the stack limit for him automatically so all the broken usermode
    // code doesn't have to be updated to do so.
    //
    // Capture the stack limit (it's in user space) before acquiring any
    // mutexes.
    //

    Teb = NULL;
    StackLimit = 0;

    if ((MI_IS_GUARD (ProtectionMask)) &&
        (!KeIsAttachedProcess ()) &&
        (Process->Wow64Process == NULL)) {

        Teb = KeGetCurrentThread()->Teb;
        try {
            StackLimit = Teb->NtTib.StackLimit;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        StackLimit = PAGE_ALIGN (StackLimit);
    }

#endif

    LOCK_ADDRESS_SPACE (Process);

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorFound;
    }

    FoundVad = MiCheckForConflictingVad (Process, StartingAddress, EndingAddress);

    if (FoundVad == NULL) {

        //
        // No virtual address is reserved at the specified base address,
        // return an error.
        //

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    //
    // Ensure that the starting and ending addresses are all within
    // the same virtual address descriptor.
    //

    if ((MI_VA_TO_VPN (StartingAddress) < FoundVad->StartingVpn) ||
        (MI_VA_TO_VPN (EndingAddress) > FoundVad->EndingVpn)) {

        //
        // Not within the section virtual address descriptor,
        // return an error.
        //

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.VadType == VadLargePages) {

        //
        // These regions are always the same protection throughout.
        //

        if (ProtectionMask == FoundVad->u.VadFlags.Protection) {

            UNLOCK_ADDRESS_SPACE (Process);

            *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
            *BaseAddress = StartingAddress;
            *LastProtect = MI_CONVERT_FROM_PTE_PROTECTION (ProtectionMask);

            return STATUS_SUCCESS;
        }

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.VadType == VadAwe) {

        if ((ProtectionMask == MM_NOACCESS) ||
            (ProtectionMask == MM_READONLY) ||
            (ProtectionMask == MM_READWRITE)) {

            CapturedOldProtect = MiProtectAweRegion (StartingAddress,
                                                     EndingAddress,
                                                     ProtectionMask);

            UNLOCK_ADDRESS_SPACE (Process);

            *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
            *BaseAddress = StartingAddress;
            *LastProtect = CapturedOldProtect;

            return STATUS_SUCCESS;
        }

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

        //
        // Setting the protection of a physically mapped section is
        // not allowed as there is no corresponding PFN database element.
        //

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.NoChange == 1) {

        //
        // An attempt is made at changing the protection
        // of a secured VAD, check to see if the address range
        // to change allows the change.
        //

        Status = MiCheckSecuredVad (FoundVad,
                                    CapturedBase,
                                    CapturedRegionSize,
                                    ProtectionMask);

        if (!NT_SUCCESS (Status)) {
            goto ErrorFound;
        }
    }

    if (FoundVad->u.VadFlags.PrivateMemory == 0) {

        if (FoundVad->u.VadFlags.VadType == VadLargePageSection) {

            //
            // For now, these regions do not allow protection changes since
            // they are mapped by large pages.  We can consider whether we
            // want to split the pages into small mappings to provide individual
            // protections.
            //

            if (ProtectionMask == FoundVad->u.VadFlags.Protection) {

                UNLOCK_ADDRESS_SPACE (Process);

                *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
                *BaseAddress = StartingAddress;
                *LastProtect = MI_CONVERT_FROM_PTE_PROTECTION (ProtectionMask);

                return STATUS_SUCCESS;
            }

            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorFound;
        }

        //
        // For mapped sections, the NO_CACHE and write combined attributes
        // are not allowed.
        //

        //
        // PAGE_WRITECOPY is not valid for private pages and rotate physical
        // sections are private pages from the application's viewpoint.
        //

        if (FoundVad->u.VadFlags.VadType == VadRotatePhysical) {

            if ((NewProtectWin32 & PAGE_WRITECOPY) ||
                (NewProtectWin32 & PAGE_EXECUTE_WRITECOPY) ||
                (NewProtectWin32 & PAGE_NOACCESS) ||
                (NewProtectWin32 & PAGE_GUARD)) {

                Status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorFound;
            }
        }
        else {
                
            if (NewProtectWin32 & (PAGE_NOCACHE | PAGE_WRITECOMBINE)) {

                //
                // Not allowed.
                //

                Status = STATUS_INVALID_PARAMETER_4;
                goto ErrorFound;
            }

            //
            // Make sure the section page protection is compatible with
            // the specified page protection.
            //
    
            if ((FoundVad->ControlArea->u.Flags.Image == 0) &&
                (!MiIsPteProtectionCompatible ((MM_PROTECTION_MASK)FoundVad->u.VadFlags.Protection,
                                               OriginalProtect))) {
                Status = STATUS_SECTION_PROTECTION;
                goto ErrorFound;
            }
        }

        //
        // If this is a file mapping, then all pages must be
        // committed as there can be no sparse file maps. Images
        // can have non-committed pages if the alignment is greater
        // than the page size.
        //

        if ((FoundVad->ControlArea->u.Flags.File == 0) ||
            (FoundVad->ControlArea->u.Flags.Image == 1)) {

            VirtualAddress = MI_VPN_TO_VA (FoundVad->StartingVpn);

            if ((FoundVad->u.VadFlags.VadType == VadImageMap)
#if (_MI_PAGING_LEVELS>=4)
                && (MiGetPxeAddress(VirtualAddress)->u.Hard.Valid == 1)
#endif
#if (_MI_PAGING_LEVELS>=3)
                && (MiGetPpeAddress(VirtualAddress)->u.Hard.Valid == 1)
#endif
                && (MiGetPdeAddress(VirtualAddress)->u.Hard.Valid == 1)) {

                //
                // Handle this specially if it is an image view using
                // large pages.
                //

                PointerPde = MiGetPdeAddress (VirtualAddress);

                if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

                    if ((NewProtectWin32 == PAGE_EXECUTE_READWRITE) ||
                        (NewProtectWin32 == PAGE_READWRITE)) {

                        UNLOCK_ADDRESS_SPACE (Process);

                        *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
                        *BaseAddress = StartingAddress;
                        *LastProtect = MI_CONVERT_FROM_PTE_PROTECTION (ProtectionMask);

                        return STATUS_SUCCESS;
                    }
                    Status = STATUS_SECTION_PROTECTION;
                    goto ErrorFound;
                }
            }

            PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                                MI_VA_TO_VPN (StartingAddress));
            LastProtoPte = MiGetProtoPteAddress (FoundVad,
                                        MI_VA_TO_VPN (EndingAddress));

            //
            // Release the working set mutex and acquire the section
            // commit mutex.  Check all the prototype PTEs described by
            // the virtual address range to ensure they are committed.
            //

            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

            while (PointerProtoPte <= LastProtoPte) {

                //
                // Check to see if the prototype PTE is committed, if
                // not return an error.
                //

                if (PointerProtoPte->u.Long == 0) {

                    //
                    // Error, this prototype PTE is not committed.
                    //

                    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
                    Status = STATUS_NOT_COMMITTED;
                    goto ErrorFound;
                }
                PointerProtoPte += 1;
            }

            //
            // The range is committed, release the section commitment
            // mutex, acquire the working set mutex and update the local PTEs.
            //

            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
        }

        //
        // Set the protection on the section pages.
        //

        Status = MiSetProtectionOnSection (Process,
                                           FoundVad,
                                           StartingAddress,
                                           EndingAddress,
                                           NewProtectWin32,
                                           &CapturedOldProtect,
                                           FALSE,
                                           &Locked);

        //
        //      ***  WARNING ***
        //
        // The alternate PTE support routines called by MiSetProtectionOnSection
        // may have deleted the old (small) VAD and replaced it with a different
        // (large) VAD - if so, the old VAD is freed and cannot be referenced.
        //

        if (!NT_SUCCESS (Status)) {
            goto ErrorFound;
        }
    }
    else {

        //
        // Not a section, private.
        // For private pages, the WRITECOPY attribute is not allowed.
        //

        if ((NewProtectWin32 & PAGE_WRITECOPY) ||
            (NewProtectWin32 & PAGE_EXECUTE_WRITECOPY)) {

            //
            // Not allowed.
            //

            Status = STATUS_INVALID_PARAMETER_4;
            goto ErrorFound;
        }

        Thread = PsGetCurrentThread ();

        LOCK_WS_UNSAFE (Thread, Process);

        //
        // Ensure all of the pages are already committed as described
        // in the virtual address descriptor.
        //

        if ( !MiIsEntireRangeCommitted (StartingAddress,
                                        EndingAddress,
                                        FoundVad,
                                        Process)) {

            //
            // Previously reserved pages have been decommitted, or an error
            // occurred, release mutex and return status.
            //

            UNLOCK_WS_UNSAFE (Thread, Process);
            Status = STATUS_NOT_COMMITTED;
            goto ErrorFound;
        }

        //
        // The address range is committed, change the protection.
        //

        PointerPde = MiGetPdeAddress (StartingAddress);
        PointerPte = MiGetPteAddress (StartingAddress);
        LastPte = MiGetPteAddress (EndingAddress);

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

        //
        // Capture the protection for the first page.
        //

        if (PointerPte->u.Long != 0) {

            CapturedOldProtect = MiGetPageProtection (PointerPte);

            //
            // Make sure the page directory & table pages are still resident.
            //

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }
        else {

            //
            // Get the protection from the VAD.
            //

            CapturedOldProtect =
               MI_CONVERT_FROM_PTE_PROTECTION (FoundVad->u.VadFlags.Protection);
        }

        //
        // For all the PTEs in the specified address range, set the
        // protection depending on the state of the PTE.
        //

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);

                MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
            }

            PteContents = *PointerPte;

            if (PteContents.u.Long == 0) {

                //
                // Increment the count of non-zero page table entries
                // for this page table and the number of private pages
                // for the process.  The protection will be set as
                // if the PTE was demand zero.
                //

                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));

                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }

            if (PteContents.u.Hard.Valid == 1) {

                //
                // Set the protection into both the PTE and the original PTE
                // in the PFN database.
                //

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                if (Pfn1->u3.e1.PrototypePte == 1) {

                    //
                    // This PTE refers to a fork prototype PTE, make it
                    // private.
                    //

                    MiCopyOnWrite (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PointerPte);

                    //
                    // This may have released the working set mutex and
                    // the page directory and table pages may no longer be
                    // in memory.
                    //

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    //
                    // Do the loop again for the same PTE.
                    //

                    continue;
                }

                //
                // The PTE is a private page which is valid, if the
                // specified protection is no-access or guard page
                // remove the PTE from the working set.
                //

                if ((NewProtectWin32 & PAGE_NOACCESS) || (NewProtectWin32 & PAGE_GUARD)) {

                    //
                    // Remove the page from the working set.
                    //

                    Locked = MiRemovePageFromWorkingSet (PointerPte,
                                                         Pfn1,
                                                         &Process->Vm);

                    continue;
                }

                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

                //
                // Change the protection of the valid PTE and flush the TB.
                //

                MiFlushTbAndCapture (FoundVad,
                                     PointerPte,
                                     ProtectionMask,
                                     Pfn1,
                                     TRUE);
            }
            else if (PteContents.u.Soft.Prototype == 1) {

                //
                // This PTE refers to a fork prototype PTE, make the
                // page private.  This is accomplished by releasing
                // the working set mutex, reading the page thereby
                // causing a fault, and re-executing the loop. Hopefully,
                // this time, we'll find the page present and we'll
                // turn it into a private page.
                //
                // Note, that a TRY is used to catch guard
                // page exceptions and no-access exceptions.
                //

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                DoAgain = TRUE;

                while (PteContents.u.Hard.Valid == 0) {

                    UNLOCK_WS_UNSAFE (Thread, Process);
                    WsHeld = FALSE;

                    try {

                        *(volatile ULONG *)Va;

                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        if (GetExceptionCode() == STATUS_ACCESS_VIOLATION) {

                            //
                            // The prototype PTE must be noaccess.
                            //

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Thread, Process);
                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        Process,
                                                        MM_NOIRQL);

                            if (MiChangeNoAccessForkPte (PointerPte, ProtectionMask) == TRUE) {
                                DoAgain = FALSE;
                            }
                        }
                        else if (GetExceptionCode() == STATUS_IN_PAGE_ERROR) {

                            //
                            // Ignore this page and go on to the next one.
                            //

                            PointerPte += 1;
                            DoAgain = TRUE;

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Thread, Process);
                            break;
                        }
                    }

                    if (WsHeld == FALSE) {
                        LOCK_WS_UNSAFE (Thread, Process);
                    }

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    PteContents = *PointerPte;
                }

                if (DoAgain) {
                    continue;
                }
            }
            else if (PteContents.u.Soft.Transition == 1) {

                if (MiSetProtectionOnTransitionPte (PointerPte,
                                                    ProtectionMask)) {
                    continue;
                }
            }
            else {

                //
                // Must be page file space or demand zero.
                //

                PointerPte->u.Soft.Protection = ProtectionMask;
                ASSERT (PointerPte->u.Long != 0);
            }

            PointerPte += 1;

        } //end while

        UNLOCK_WS_UNSAFE (Thread, Process);

#if defined (_MI_RESET_USER_STACK_LIMIT)

        //
        // If a guard page is being set above the current stack limit in the
        // caller's thread stack then he may be trying to recover from a
        // user initiated stack probe growth (ie, inside his exception handler).
        // Reset the stack limit for him automatically so all the broken
        // usermode code doesn't have to be updated to do so.
        //

        if (StackLimit != 0) {
    
            if ((StackLimit < EndingAddress) &&
                (MI_VA_TO_VPN (StackLimit) >= FoundVad->StartingVpn) &&
                (MI_VA_TO_VPN (StackLimit) <= FoundVad->EndingVpn)) {
    
                StackLimit = (PVOID)((ULONG_PTR) EndingAddress + 1);
    
                if (MI_VA_TO_VPN (StackLimit) <= FoundVad->EndingVpn) {
    
                    //
                    // Stack limit is still within the VAD, so update the TEB.
                    //
    
                    try {
                        Teb->NtTib.StackLimit = StackLimit;
                    } except (EXCEPTION_EXECUTE_HANDLER) {
                        NOTHING;
                    }
                }
            }
        }

#endif

    }

    UNLOCK_ADDRESS_SPACE (Process);

    //
    // Common completion code.
    //

    *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
    *BaseAddress = StartingAddress;
    *LastProtect = CapturedOldProtect;

    if (Locked) {
        return STATUS_WAS_UNLOCKED;
    }

    return STATUS_SUCCESS;

ErrorFound:

    UNLOCK_ADDRESS_SPACE (Process);
    return Status;
}


NTSTATUS
MiSetProtectionOnSection (
    IN PEPROCESS Process,
    IN PMMVAD FoundVad,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN ULONG NewProtect,
    OUT PULONG CapturedOldProtect,
    IN ULONG DontCharge,
    OUT PULONG Locked
    )

/*++

Routine Description:

    This routine changes the protection on a region of committed pages
    within the virtual address space of the subject process.  Setting
    the protection on a range of pages causes the old protection to be
    replaced by the specified protection value.

Arguments:

    Process - Supplies a pointer to the current process.

    FoundVad - Supplies a pointer to the VAD containing the range to protect.

    StartingAddress - Supplies the starting address to protect.

    EndingAddress - Supplies the ending address to protect.

    NewProtect - Supplies the new protection to set.

    CapturedOldProtect - Supplies the address of a kernel owned pointer to
                store (without probing) the old protection into.

    DontCharge - Supplies TRUE if no quota or commitment should be charged.

    Locked - Receives TRUE if a locked page was removed from the working
             set (protection was guard page or no-access), FALSE otherwise.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, address creation mutex held, APCs disabled.

--*/

{
    KIRQL OldIrql;
    LOGICAL WsHeld;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE PointerProtoPte;
    PFN_NUMBER PageFrameIndex;
    PETHREAD Thread;
    PMMPFN Pfn1;
    MMPTE TempPte;
    ULONG ProtectionMask;
    ULONG ProtectionMaskNotCopy;
    ULONG NewProtectionMask;
    MMPTE PteContents;
    WSLE_NUMBER Index;
    PULONG Va;
    ULONG WriteCopy;
    ULONG DoAgain;
    ULONG Waited;
    SIZE_T QuotaCharge;
    PVOID UsedPageTableHandle;
    NTSTATUS Status;
    LOGICAL CaptureDirtyBit;

#if DBG

#define PTES_TRACKED 0x10

    ULONG PteIndex = 0;
    MMPTE PteTracker[PTES_TRACKED];
    MMPFN PfnTracker[PTES_TRACKED];
    SIZE_T PteQuotaCharge;
#endif

    PAGED_CODE();

    *Locked = FALSE;
    WriteCopy = FALSE;
    QuotaCharge = 0;

    //
    // Make the protection field.
    //

    ASSERT (FoundVad->u.VadFlags.PrivateMemory == 0);

    if ((FoundVad->u.VadFlags.VadType == VadImageMap) ||
        (FoundVad->u2.VadFlags2.CopyOnWrite == 1)) {

        if (NewProtect & PAGE_READWRITE) {
            NewProtect &= ~PAGE_READWRITE;
            NewProtect |= PAGE_WRITECOPY;
        }

        if (NewProtect & PAGE_EXECUTE_READWRITE) {
            NewProtect &= ~PAGE_EXECUTE_READWRITE;
            NewProtect |= PAGE_EXECUTE_WRITECOPY;
        }
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {

        //
        // Return the error.
        //

        return STATUS_INVALID_PAGE_PROTECTION;
    }

    //
    // Determine if copy on write is being set.
    //

    ProtectionMaskNotCopy = ProtectionMask;
    if ((ProtectionMask & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK) {
        WriteCopy = TRUE;
        ProtectionMaskNotCopy &= ~MM_PROTECTION_COPY_MASK;
    }

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    Thread = PsGetCurrentThread ();

    LOCK_WS_UNSAFE (Thread, Process);

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

    //
    // Capture the protection for the first page.
    //

    if (PointerPte->u.Long != 0) {

        if ((FoundVad->u.VadFlags.VadType == VadRotatePhysical) &&
             (PointerPte->u.Hard.Valid == 1)) {

            PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                    MI_VA_TO_VPN (
                                    MiGetVirtualAddressMappedByPte (PointerPte)));

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if ((!MI_IS_PFN (PageFrameIndex)) ||
                (Pfn1->PteAddress != PointerProtoPte)) {

                //
                // This address in the view is currently pointing at
                // the frame buffer (video RAM or AGP), protection is
                // in the prototype (which may itself be paged out - so
                // release the process working set pushlock before
                // accessing it).
                //

                UNLOCK_WS_UNSAFE (Thread, Process);

                TempPte = *PointerProtoPte;

                ASSERT (TempPte.u.Hard.Valid == 0);
                ASSERT (TempPte.u.Soft.Prototype == 0);

                //
                // PTE is either demand zero, pagefile or transition, in all
                // cases protection is in the prototype PTE.
                //

                *CapturedOldProtect =
                    MI_CONVERT_FROM_PTE_PROTECTION (TempPte.u.Soft.Protection);

                LOCK_WS_UNSAFE (Thread, Process);

                //
                // Ensure the PDE (and any table above it) are still resident.
                //

                MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
            }
            else {
                ASSERT (Pfn1->u3.e1.PrototypePte == 1);
                *CapturedOldProtect = MI_CONVERT_FROM_PTE_PROTECTION (
                                        Pfn1->OriginalPte.u.Soft.Protection);
            }
        }
        else {
            *CapturedOldProtect = MiGetPageProtection (PointerPte);
        }

        //
        // Ensure the PDE (and any table above it) are still resident.
        //

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
    }
    else {

        //
        // Get the protection from the VAD, unless image file.
        //

        if (FoundVad->u.VadFlags.VadType != VadImageMap) {

            //
            // This is not an image file, the protection is in the VAD.
            //

            *CapturedOldProtect =
                MI_CONVERT_FROM_PTE_PROTECTION(FoundVad->u.VadFlags.Protection);
        }
        else {

            //
            // This is an image file, the protection is in the
            // prototype PTE.
            //

            PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                    MI_VA_TO_VPN (
                                    MiGetVirtualAddressMappedByPte (PointerPte)));

            //
            // The prototype PTE may be paged out so release the process
            // working set pushlock before referencing the proto.
            //

            UNLOCK_WS_UNSAFE (Thread, Process);

            TempPte = *PointerProtoPte;

            if (TempPte.u.Hard.Valid == 0) {
                *CapturedOldProtect = MI_CONVERT_FROM_PTE_PROTECTION (TempPte.u.Soft.Protection);
            }
            else {

                //
                // The prototype PTE is valid but not in this
                // process (at least not before we released the
                // working set pushlock above).  So the PFN's
                // OriginalPte field contains the correct
                // protection value, but it may get trimmed at any
                // time.  Acquire the PFN lock so we can examine
                // it safely.
                //

                LOCK_PFN (OldIrql);

                if (MiGetPteAddress (PointerProtoPte)->u.Hard.Valid == 0) {
                    MiMakeSystemAddressValidPfn (PointerProtoPte, OldIrql);
                }

                TempPte = *PointerProtoPte;

                ASSERT (TempPte.u.Long != 0);

                if (TempPte.u.Hard.Valid) {
                    Pfn1 = MI_PFN_ELEMENT (TempPte.u.Hard.PageFrameNumber);

                    *CapturedOldProtect = MI_CONVERT_FROM_PTE_PROTECTION (
                                           Pfn1->OriginalPte.u.Soft.Protection);
                }
                else {
                    *CapturedOldProtect = MI_CONVERT_FROM_PTE_PROTECTION (TempPte.u.Soft.Protection);
                }
                UNLOCK_PFN (OldIrql);
            }

            LOCK_WS_UNSAFE (Thread, Process);

            //
            // Ensure the PDE (and any table above it) are still resident.
            //

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }
    }

    //
    // If the page protection is being changed to be copy-on-write, the
    // commitment and page file quota for the potentially dirty private pages
    // must be calculated and charged.  This must be done before any
    // protections are changed as the changes cannot be undone.
    //

    if (WriteCopy) {

        //
        // Calculate the charges.  If the page is shared and not write copy
        // it is counted as a charged page.
        //

        ASSERT (FoundVad->u.VadFlags.VadType != VadRotatePhysical);

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPxe = MiGetPdeAddress (PointerPde);

#if (_MI_PAGING_LEVELS >= 4)
retry:
#endif
                do {

                    while (!MiDoesPxeExistAndMakeValid(PointerPxe, Process, MM_NOIRQL, &Waited)) {

                        //
                        // No PXE exists for this address.  Therefore
                        // all the PTEs are shared and not copy on write.
                        // go to the next PXE.
                        //

                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerProtoPte = PointerPte;
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }
                        QuotaCharge += PointerPte - PointerProtoPte;
                    }
#if (_MI_PAGING_LEVELS >= 4)
                    Waited = 0;
#endif

                    while (!MiDoesPpeExistAndMakeValid(PointerPpe, Process, MM_NOIRQL, &Waited)) {

                        //
                        // No PPE exists for this address.  Therefore
                        // all the PTEs are shared and not copy on write.
                        // go to the next PPE.
                        //

                        PointerPpe += 1;
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerProtoPte = PointerPte;
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }

#if (_MI_PAGING_LEVELS >= 4)
                        if (MiIsPteOnPdeBoundary (PointerPpe)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            goto retry;
                        }
#endif
                        QuotaCharge += PointerPte - PointerProtoPte;
                    }

#if (_MI_PAGING_LEVELS < 4)
                    Waited = 0;
#endif

                    while (!MiDoesPdeExistAndMakeValid(PointerPde, Process, MM_NOIRQL, &Waited)) {

                        //
                        // No PDE exists for this address.  Therefore
                        // all the PTEs are shared and not copy on write.
                        // go to the next PDE.
                        //

                        PointerPde += 1;
                        PointerProtoPte = PointerPte;
                        PointerPpe = MiGetPteAddress (PointerPde);
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }
                        QuotaCharge += PointerPte - PointerProtoPte;
#if (_MI_PAGING_LEVELS >= 3)
                        if (MiIsPteOnPdeBoundary (PointerPde)) {
                            Waited = 1;
                            break;
                        }
#endif
                    }
                } while (Waited != 0);
            }

            PteContents = *PointerPte;

            if (PteContents.u.Long == 0) {

                //
                // The PTE has not been evaluated, assume copy on write.
                //

                QuotaCharge += 1;

            }
            else if (PteContents.u.Hard.Valid == 1) {
                if (PteContents.u.Hard.CopyOnWrite == 0) {

                    //
                    // See if this is a prototype PTE, if so charge it.
                    //

                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                    if (Pfn1->u3.e1.PrototypePte == 1) {
                        QuotaCharge += 1;
                    }
                }
            }
            else {

                if (PteContents.u.Soft.Prototype == 1) {

                    //
                    // This is a prototype PTE.  Charge if it is not
                    // in copy on write format.
                    //

                    if (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {
                        //
                        // Page protection is within the PTE.
                        //

                        if (!MI_IS_PTE_PROTECTION_COPY_WRITE(PteContents.u.Soft.Protection)) {
                            QuotaCharge += 1;
                        }
                    }
                    else {

                        //
                        // The PTE references the prototype directly, therefore
                        // it can't be copy on write.  Charge.
                        //

                        QuotaCharge += 1;
                    }
                }
            }
            PointerPte += 1;
        }

Done:

        //
        // If any quota is required, charge for it now.
        //
        // Note that the working set pushlock  must be released before charging.
        //

        if ((!DontCharge) && (QuotaCharge != 0)) {

            UNLOCK_WS_UNSAFE (Thread, Process);

            Status = PsChargeProcessPageFileQuota (Process, QuotaCharge);

            if (!NT_SUCCESS (Status)) {
                return STATUS_PAGEFILE_QUOTA_EXCEEDED;
            }

            if (Process->CommitChargeLimit) {
                if (Process->CommitCharge + QuotaCharge > Process->CommitChargeLimit) {
                    PsReturnProcessPageFileQuota (Process, QuotaCharge);
                    if (Process->Job) {
                        PsReportProcessMemoryLimitViolation ();
                    }
                    return STATUS_COMMITMENT_LIMIT;
                }
            }
            if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {

                if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, QuotaCharge) == FALSE) {
                    PsReturnProcessPageFileQuota (Process, QuotaCharge);
                    return STATUS_COMMITMENT_LIMIT;
                }
            }

            if (MiChargeCommitment (QuotaCharge, NULL) == FALSE) {
                if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                    PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)QuotaCharge);
                }
                PsReturnProcessPageFileQuota (Process, QuotaCharge);
                return STATUS_COMMITMENT_LIMIT;
            }

            //
            // Add the quota into the charge to the VAD.
            //

            MM_TRACK_COMMIT (MM_DBG_COMMIT_SET_PROTECTION, QuotaCharge);
            FoundVad->u.VadFlags.CommitCharge += QuotaCharge;
            Process->CommitCharge += QuotaCharge;
            if (Process->CommitCharge > Process->CommitChargePeak) {
                Process->CommitChargePeak = Process->CommitCharge;
            }
            MI_INCREMENT_TOTAL_PROCESS_COMMIT (QuotaCharge);

            LOCK_WS_UNSAFE (Thread, Process);
        }
    }

#if DBG
    PteQuotaCharge = QuotaCharge;
#endif

    //
    // For all the PTEs in the specified address range, set the
    // protection depending on the state of the PTE.
    //

    //
    // If the PTE was copy on write (but not written) and the
    // new protection is NOT copy-on-write, return page file quota
    // and commitment.
    //

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);

    //
    // Ensure the PDE (and any table above it) are still resident.
    //

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

    QuotaCharge = 0;

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {
            PointerPde = MiGetPteAddress (PointerPte);
            PointerPpe = MiGetPdeAddress (PointerPte);
            PointerPxe = MiGetPpeAddress (PointerPte);

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }

        PteContents = *PointerPte;

        if (PteContents.u.Long == 0) {

            //
            // Increment the count of non-zero page table entries
            // for this page table and the number of private pages
            // for the process.
            //

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));

            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

            //
            // The PTE is zero, set it into prototype PTE format
            // with the protection in the prototype PTE.
            //

            TempPte = PrototypePte;
            TempPte.u.Soft.Protection = ProtectionMask;
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        else if (PteContents.u.Hard.Valid == 1) {

            //
            // Set the protection into both the PTE and the original PTE
            // in the PFN database for private pages only.
            //

            NewProtectionMask = ProtectionMask;

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                PfnTracker[PteIndex] = *Pfn1;
                PteIndex += 1;
            }
#endif

            if ((NewProtect & PAGE_NOACCESS) ||
                (NewProtect & PAGE_GUARD)) {

                ASSERT (FoundVad->u.VadFlags.VadType != VadRotatePhysical);

                *Locked = MiRemovePageFromWorkingSet (PointerPte,
                                                      Pfn1,
                                                      &Process->Vm);
                continue;
            }

            CaptureDirtyBit = TRUE;

            SATISFY_OVERZEALOUS_COMPILER (PointerProtoPte = NULL);

            if (FoundVad->u.VadFlags.VadType == VadRotatePhysical) {

                //
                // Regardless of whether view is currently pointing at the
                // frame buffer or regular memory, the prototype PTE must
                // be updated here as well.  This is so the view rotation
                // (which is much more common than a protection change on
                // one of these ranges) can be fast.
                //

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                                        MI_VA_TO_VPN (Va));

                if (!MI_IS_PFN (PageFrameIndex)) {
                    Pfn1 = NULL;
                }

                if ((Pfn1 == NULL) || (Pfn1->PteAddress != PointerProtoPte)) {

                    //
                    // This address in the view is currently pointing at
                    // the frame buffer (video RAM or AGP), protection is
                    // in the prototype PTE.
                    //
                    // No WSLE to update since the regular memory equivalent is
                    // not valid, and the active framebuffer doesn't have one.
                    //

                    CaptureDirtyBit = FALSE;
                }
                else {

                    ASSERT (Pfn1->u3.e1.PrototypePte == 1);

                    //
                    // The true protection cannot be in the WSLE because for
                    // these types of regions we always update both the WSLE
                    // and the prototype PTE.
                    //

#if DBG
                    Index = MiLocateWsle ((PVOID)Va, 
                                          MmWorkingSetList,
                                          Pfn1->u1.WsIndex,
                                          FALSE);

                    ASSERT (MmWsle[Index].u1.e1.Protection == MM_ZERO_ACCESS);
#endif
                }
            }
            else if (Pfn1->u3.e1.PrototypePte == 1) {

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                //
                // Check to see if this is a prototype PTE.  This
                // is done by comparing the PTE address in the
                // PFN database to the PTE address indicated by the VAD.
                // If they are not equal, this is a fork prototype PTE.
                //

                if (Pfn1->PteAddress != MiGetProtoPteAddress (FoundVad,
                                                    MI_VA_TO_VPN ((PVOID)Va))) {

                    //
                    // This PTE refers to a fork prototype PTE, make it
                    // private.  But don't charge quota for it if the PTE
                    // was already copy on write (because it's already
                    // been charged for this case).
                    //

                    if (MiCopyOnWrite ((PVOID)Va, PointerPte) == TRUE) {

                        if ((WriteCopy) && (PteContents.u.Hard.CopyOnWrite == 0)) {
                            QuotaCharge += 1;
                        }
                    }

                    //
                    // Ensure the PDE (and any table above it) are still
                    // resident (they may have been trimmed when the working
                    // set mutex was released above).
                    //

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    //
                    // Do the loop again.
                    //

                    continue;
                }

                //
                // Update the protection field in the WSLE and the PTE.
                //
                // If the PTE is copy on write uncharge the
                // previously charged quota.
                //

                if ((!WriteCopy) && (PteContents.u.Hard.CopyOnWrite == 1)) {
                    QuotaCharge += 1;
                }

                //
                // The true protection may be in the WSLE, locate it.
                //

                Index = MiLocateWsle ((PVOID)Va, 
                                      MmWorkingSetList,
                                      Pfn1->u1.WsIndex,
                                      FALSE);

                MmWsle[Index].u1.e1.Protection = ProtectionMask;
            }
            else {

                //
                // Page is private (copy on written), protection mask
                // is stored in the original PTE field.
                //

                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMaskNotCopy;

                NewProtectionMask = ProtectionMaskNotCopy;
            }

            MI_SNAP_DATA (Pfn1, PointerPte, 7);

            //
            // Change the protection of the valid PTE and flush the TB.
            //

            MiFlushTbAndCapture (FoundVad,
                                 PointerPte,
                                 NewProtectionMask,
                                 Pfn1,
                                 CaptureDirtyBit);

            if (FoundVad->u.VadFlags.VadType == VadRotatePhysical) {

                //
                // Release the working set pushlock as the prototype PTE we
                // are about to update may be paged out and we don't want
                // to deadlock (this process may have all the trimmable pages).
                //
                // Because the address space mutex is still held, the section
                // cannot go away and thus the prototype PTE cannot go away.
                // The system working set pushlock is acquired to prevent it
                // from changing.
                //

                UNLOCK_WS_UNSAFE (Thread, Process);

                LOCK_SYSTEM_WS (Thread);
                LOCK_PFN (OldIrql);

                MiMakeSystemAddressValidPfnSystemWs (PointerProtoPte, OldIrql);

                //
                // Note the prototype PTE may be in any state since the
                // process working set pushlock was released.  Handle all of
                // them here.
                //

                ASSERT (PointerProtoPte->u.Long != 0);

                if (PointerProtoPte->u.Hard.Valid == 1) {
                    Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (PointerProtoPte));
                    Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                }
                else {
                    PointerProtoPte->u.Soft.Protection = ProtectionMask;
                }

                UNLOCK_PFN (OldIrql);
                UNLOCK_SYSTEM_WS (Thread);

                LOCK_WS_UNSAFE (Thread, Process);

                MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
            }
        }
        else if (PteContents.u.Soft.Prototype == 1) {

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                *(PULONG)(&PfnTracker[PteIndex]) = 0x88;
                PteIndex += 1;
            }
#endif

            //
            // The PTE is in prototype PTE format.
            //
            // Is it a fork prototype PTE?
            //

            Va = (PULONG)MiGetVirtualAddressMappedByPte (PointerPte);

            if ((PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
               (MiPteToProto (PointerPte) !=
                                 MiGetProtoPteAddress (FoundVad,
                                     MI_VA_TO_VPN ((PVOID)Va)))) {

                //
                // This PTE refers to a fork prototype PTE, make the
                // page private.  This is accomplished by releasing
                // the working set mutex, reading the page thereby
                // causing a fault, and re-executing the loop, hopefully,
                // this time, we'll find the page present and will
                // turn it into a private page.
                //
                // Note, that page with prototype = 1 cannot be
                // no-access.
                //

                DoAgain = TRUE;

                while (PteContents.u.Hard.Valid == 0) {

                    UNLOCK_WS_UNSAFE (Thread, Process);

                    WsHeld = FALSE;

                    try {

                        *(volatile ULONG *)Va;
                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        if (GetExceptionCode() != STATUS_GUARD_PAGE_VIOLATION) {

                            //
                            // The prototype PTE must be noaccess.
                            //

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Thread, Process);
                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        Process,
                                                        MM_NOIRQL);
                            if (MiChangeNoAccessForkPte (PointerPte, ProtectionMask) == TRUE) {
                                DoAgain = FALSE;
                            }
                        }
                    }

                    PointerPpe = MiGetPteAddress (PointerPde);
                    PointerPxe = MiGetPdeAddress (PointerPde);

                    if (WsHeld == FALSE) {
                        LOCK_WS_UNSAFE (Thread, Process);
                    }

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    PteContents = *PointerPte;
                }

                if (DoAgain) {
                    continue;
                }

            }
            else {

                //
                // If the new protection is not write-copy, the PTE
                // protection is not in the prototype PTE (can't be
                // write copy for sections), and the protection in
                // the PTE is write-copy, release the page file
                // quota and commitment for this page.
                //

                if ((!WriteCopy) &&
                    (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED)) {
                    if (MI_IS_PTE_PROTECTION_COPY_WRITE(PteContents.u.Soft.Protection)) {
                        QuotaCharge += 1;
                    }

                }

                //
                // The PTE is a prototype PTE.  Make the high part
                // of the PTE indicate that the protection field
                // is in the PTE itself.
                //

                MI_WRITE_INVALID_PTE (PointerPte, PrototypePte);
                PointerPte->u.Soft.Protection = ProtectionMask;
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

            ASSERT (FoundVad->u.VadFlags.VadType != VadRotatePhysical);
#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
                PfnTracker[PteIndex] = *Pfn1;
                PteIndex += 1;
            }
#endif

            //
            // This is a transition PTE. (Page is private)
            //

            if (MiSetProtectionOnTransitionPte (
                                        PointerPte,
                                        ProtectionMaskNotCopy)) {
                continue;
            }
        }
        else {

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                *(PULONG)(&PfnTracker[PteIndex]) = 0x99;
                PteIndex += 1;
            }
#endif

            //
            // Must be page file space or demand zero.
            //

            PointerPte->u.Soft.Protection = ProtectionMaskNotCopy;
        }

        PointerPte += 1;
    }

    UNLOCK_WS_UNSAFE (Thread, Process);

    //
    // Return the quota charge and the commitment, if any.
    //

    if ((QuotaCharge > 0) && (!DontCharge)) {

        MiReturnCommitment (QuotaCharge);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PROTECTION, QuotaCharge);
        PsReturnProcessPageFileQuota (Process, QuotaCharge);

#if DBG
        if (QuotaCharge > FoundVad->u.VadFlags.CommitCharge) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                "MMPROTECT QUOTA FAILURE: %p %p %x %p\n",
                PteTracker, PfnTracker, PteIndex, PteQuotaCharge);
            DbgBreakPoint ();
        }
#endif

        FoundVad->u.VadFlags.CommitCharge -= QuotaCharge;
        if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)QuotaCharge);
        }
        Process->CommitCharge -= QuotaCharge;

        MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - QuotaCharge);
    }

    return STATUS_SUCCESS;
}

WIN32_PROTECTION_MASK
MiGetPageProtection (
    IN PMMPTE PointerPte
    )

/*++

Routine Description:

    This routine returns the page protection of a non-zero PTE.
    It may release and reacquire the working set pushlock.

Arguments:

    PointerPte - Supplies a pointer to a non-zero PTE.

Return Value:

    Returns the protection code.

Environment:

    Kernel mode, working set and address creation pushlocks held.
    Note that the address creation mutex does not need to be held
    if the working set pushlock does not need to be released in the
    case of a prototype PTE.

--*/

{
    KIRQL OldIrql;
    PEPROCESS Process;
    LOGICAL WsHeldSafe;
    LOGICAL WsHeldShared;
    MMPTE PteContents;
    PMMPTE PointerPde;
    PMMPTE ProtoPte;
    PMMPFN Pfn1;
    PVOID Va;
    WSLE_NUMBER Index;
    PETHREAD Thread;
    MM_PROTECTION_MASK ProtectionMask;
    WIN32_PROTECTION_MASK ReturnProtection;

    PAGED_CODE ();

    ASSERT (!MI_IS_PTE_PROTOTYPE (PointerPte));

    PteContents = *PointerPte;

    ASSERT (PteContents.u.Long != 0);

    if ((PteContents.u.Soft.Valid == 0) && (PteContents.u.Soft.Prototype == 1)) {

        //
        // This PTE is in prototype format, the protection is
        // stored in the prototype PTE.
        //

        if (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

            //
            // The protection is within this PTE.
            //

            return MI_CONVERT_FROM_PTE_PROTECTION (
                                            PteContents.u.Soft.Protection);
        }

        //
        // Capture the prototype PTE contents.
        //
        // Note the prototype PTE itself may be paged out - so
        // release the process working set pushlock before accessing it.
        //

        Thread = PsGetCurrentThread ();
        Process = PsGetCurrentProcessByThread (Thread);

        //
        // The working set lock may have been acquired safely or unsafely
        // by our caller.  Handle both cases here and below.
        //

        UNLOCK_WS_REGARDLESS (Thread, Process, WsHeldSafe, WsHeldShared);

        ProtoPte = MiPteToProto (&PteContents);

        PteContents = *ProtoPte;

        //
        // The working set pushlock may have been released and the
        // page may no longer be in prototype format, but it doesn't
        // matter to us - we are going to return the protection from
        // the prototype PTE regardless (the protection cannot be
        // changed by the app since the address space pushlock is still
        // held.
        //

        if (PteContents.u.Hard.Valid) {

            //
            // The prototype PTE is valid but not in this
            // process (at least not before we released the
            // working set pushlock above).  So the PFN's
            // OriginalPte field contains the correct
            // protection value, but it may get trimmed at any
            // time.  Acquire the PFN lock so we can examine
            // it safely.
            //

            PointerPde = MiGetPteAddress (ProtoPte);

            LOCK_PFN (OldIrql);

            if (PointerPde->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);
            }

            PteContents = *ProtoPte;

            ASSERT (PteContents.u.Long != 0);

            if (PteContents.u.Hard.Valid) {
                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                ReturnProtection = MI_CONVERT_FROM_PTE_PROTECTION (
                                          Pfn1->OriginalPte.u.Soft.Protection);
            }
            else {
                ReturnProtection = MI_CONVERT_FROM_PTE_PROTECTION (PteContents.u.Soft.Protection);
            }
            UNLOCK_PFN (OldIrql);
        }
        else {

            //
            // The prototype PTE is not valid, return the
            // protection from the PTE.
            //

            ReturnProtection = MI_CONVERT_FROM_PTE_PROTECTION (PteContents.u.Soft.Protection);
        }

        //
        // The working set lock may have been acquired safely or unsafely
        // by our caller.  Reacquire it in the same manner our caller did.
        //

        LOCK_WS_REGARDLESS (Thread, Process, WsHeldSafe, WsHeldShared);

        return ReturnProtection;
    }

    if (PteContents.u.Hard.Valid) {

        //
        // The page is valid, the protection field is either in the
        // PFN database original PTE element or the WSLE.  If
        // the page is private, get it from the PFN original PTE
        // element, else use the WSLE.
        //

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

        if (Pfn1->u3.e1.PrototypePte == 0) {

            if (Pfn1->u4.AweAllocation == 0) {

                //
                // This is a private PTE or the PTE address is that of a
                // prototype PTE, hence the protection is in the original PTE.
                //

                return MI_CONVERT_FROM_PTE_PROTECTION (
                                      Pfn1->OriginalPte.u.Soft.Protection);
            }

            //
            // This is an AWE frame - the original PTE field in the PFN
            // actually contains the AweReferenceCount.  These pages are
            // either noaccess, readonly or readwrite.
            //

            if (PteContents.u.Hard.Owner == MI_PTE_OWNER_KERNEL) {
                return PAGE_NOACCESS;
            }

            if (PteContents.u.Hard.Write == 1) {
                return PAGE_READWRITE;
            }

            return PAGE_READONLY;
        }

        //
        // The PTE is a hardware PTE, get the protection from the WSLE.
        //

        Va = MiGetVirtualAddressMappedByPte (PointerPte);

        Process = PsGetCurrentProcess ();

        Index = MiLocateWsle (Va,
                              MmWorkingSetList,
                              Pfn1->u1.WsIndex,
                              FALSE);

        if (MmWsle[Index].u1.e1.Protection == MM_ZERO_ACCESS) {
            ProtectionMask = MI_GET_PROTECTION_FROM_SOFT_PTE (&Pfn1->OriginalPte);
        }
        else {
            ProtectionMask = (MM_PROTECTION_MASK) MmWsle[Index].u1.e1.Protection;
        }

        return MI_CONVERT_FROM_PTE_PROTECTION (ProtectionMask);
    }

    //
    // PTE is either demand zero or transition, in either
    // case protection is in PTE.
    //

    return MI_CONVERT_FROM_PTE_PROTECTION (PteContents.u.Soft.Protection);
}

ULONG
MiChangeNoAccessForkPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    )

/*++

Routine Description:

Arguments:

    PointerPte - Supplies a pointer to the current PTE.

    ProtectionMask - Supplies the protection mask to set.

Return Value:

    TRUE if the loop does NOT need to be repeated for this PTE, FALSE
    if it does need retrying.

Environment:

    Kernel mode, address creation mutex held, APCs disabled.

--*/

{
    PAGED_CODE();

    if (ProtectionMask == MM_NOACCESS) {

        //
        // No need to change the page protection.
        //

        return TRUE;
    }

    PointerPte->u.Proto.ReadOnly = 1;

    return FALSE;
}

VOID
MiFlushTbAndCapture (
    IN PMMVAD FoundVad,
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask,
    IN PMMPFN Pfn1,
    IN LOGICAL CaptureDirtyBit
    )

/*++

Routine Description:

    Non-pageable helper routine to change a PTE & flush the relevant TB entry.

Arguments:

    FoundVad - Supplies a writewatch VAD to update or NULL.

    PointerPte - Supplies the PTE to update.

    ProtectionMask - Supplies the new protection mask to use.

    Pfn1 - Supplies the PFN database entry to update.  NULL if there is no
           entry to update (ie: it's a PTE that points at dualport memory, etc).

    CaptureDirtyBit - Supplies TRUE if the dirty bit should be captured and
                      pagefile space released, etc.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    MMPTE TempPte;
    MMPTE PreviousPte;
    KIRQL OldIrql;
    PVOID VirtualAddress;
    LOGICAL RecomputePte;
    PFN_NUMBER PageFrameIndex;
    MEMORY_CACHING_TYPE CacheType;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    WSLE_NUMBER WorkingSetIndex;

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    //
    // Opportunistically compute the new PTE without the PFN lock.  If a
    // TB attribute conflict is detected, this will be redone while holding
    // the PFN lock below.
    //

    RecomputePte = FALSE;

    PreviousPte = *PointerPte;

    PageFrameIndex = (PFN_NUMBER) PreviousPte.u.Hard.PageFrameNumber;

    MI_MAKE_VALID_USER_PTE (TempPte,
                            PageFrameIndex,
                            ProtectionMask,
                            PointerPte);

    WorkingSetIndex = MI_GET_WORKING_SET_FROM_PTE (&PreviousPte);
    MI_SET_PTE_IN_WORKING_SET (&TempPte, WorkingSetIndex);

    LOCK_PFN (OldIrql);

    PreviousPte = *PointerPte;

    if (Pfn1 != NULL) {

        //
        // Ensure the requested attributes do not conflict with the
        // PFN attributes.
        //

        if (Pfn1->u3.e1.CacheAttribute == MiCached) {
            if (ProtectionMask & (MM_NOCACHE | MM_WRITECOMBINE)) {
                RecomputePte = TRUE;
                ProtectionMask &= ~(MM_NOCACHE | MM_WRITECOMBINE);
            }
        }
        else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
            if ((ProtectionMask & (MM_NOCACHE | MM_WRITECOMBINE)) != MM_NOCACHE) {
                RecomputePte = TRUE;
                ProtectionMask &= ~(MM_NOCACHE | MM_WRITECOMBINE);
                ProtectionMask |= MM_NOCACHE;
            }
        }
        else if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
            if ((ProtectionMask & (MM_NOCACHE | MM_WRITECOMBINE)) != MM_WRITECOMBINE) {
                RecomputePte = TRUE;
                ProtectionMask &= ~(MM_NOCACHE | MM_WRITECOMBINE);
                ProtectionMask |= MM_WRITECOMBINE;
            }
        }

        if (RecomputePte == TRUE) {
            MI_MAKE_VALID_USER_PTE (TempPte,
                                    PageFrameIndex,
                                    ProtectionMask,
                                    PointerPte);
    
            WorkingSetIndex = MI_GET_WORKING_SET_FROM_PTE (&PreviousPte);
            MI_SET_PTE_IN_WORKING_SET (&TempPte, WorkingSetIndex);
        }
    }
    else {

        //
        // This mapping is to an I/O space page (ie, not system
        // memory or AGP), so recompute the attributes factoring in
        // potential platform overrides.  This is because the
        // MI_MAKE_VALID_USER_PTE call assumes the memory is system -
        // the PTE masks were updated when we booted for this.
        //

        CacheType = MmCached;
        if (MI_IS_NOCACHE (ProtectionMask)) {
            CacheType = MmNonCached;
        }
        else if (MI_IS_WRITECOMBINE (ProtectionMask)) {
            CacheType = MmWriteCombined;
        }

        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 1);

        switch (CacheAttribute) {
            case MiCached:
                MI_ENABLE_CACHING (TempPte);
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

        //
        // If the protection indicates write privilege, then mark
        // the PTE as dirty and for x86/AMD64 MP Writable as well.
        //

        if (ProtectionMask & MM_PROTECTION_WRITE_MASK) {
            MI_SET_PTE_DIRTY (TempPte);
#if !defined(NT_UP)
#if defined(_X86_) || defined(_AMD64_)
            TempPte.u.Hard.Writable = 1;
#endif
#endif
        }
    }

    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    //
    // Flush the TB as we have changed the protection of a valid PTE.
    //

    MI_FLUSH_SINGLE_TB (VirtualAddress, FALSE);

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    //
    // A page protection is being changed, on certain
    // hardware the dirty bit should be ORed into the
    // modify bit in the PFN element.
    //

    if (CaptureDirtyBit == TRUE) {
        ASSERT (Pfn1 != NULL);
        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);
    }

    //
    // If the PTE indicates the page has been modified (this is different
    // from the PFN indicating this), then ripple it back to the write watch
    // bitmap now since we are still in the correct process context.
    //

    if (FoundVad->u.VadFlags.VadType == VadWriteWatch) {

        ASSERT ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH));
        ASSERT (Pfn1->u3.e1.PrototypePte == 0);

        if (MI_IS_PTE_DIRTY(PreviousPte)) {

            //
            // This process has (or had) write watch VADs.  Update the
            // bitmap now.
            //

            MiCaptureWriteWatchDirtyBit (PsGetCurrentProcess(), VirtualAddress);
        }
    }

    UNLOCK_PFN (OldIrql);
    return;
}

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    )

/*++

Routine Description:

    Non-pageable helper routine to change the protection of a transition PTE.

Arguments:

    PointerPte - Supplies a pointer to the PTE.

    ProtectionMask - Supplies the new protection mask.

Return Value:

    TRUE if the caller needs to retry, FALSE if the protection was successfully
    changed.

Environment:

    Kernel mode.  The PFN lock is needed to ensure the (private) page
    doesn't become non-transition.

--*/

{
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPFN Pfn1;

    LOCK_PFN (OldIrql);

    //
    // Make sure the page is still a transition page.
    //

    PteContents = *PointerPte;

    if ((PteContents.u.Soft.Prototype == 0) &&
        (PointerPte->u.Soft.Transition == 1)) {

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
        PointerPte->u.Soft.Protection = ProtectionMask;

        UNLOCK_PFN (OldIrql);

        return FALSE;
    }

    //
    // Do this loop again for the same PTE.
    //

    UNLOCK_PFN (OldIrql);
    return TRUE;
}

NTSTATUS
MiCheckSecuredVad (
    IN PMMVAD Vad,
    IN PVOID Base,
    IN SIZE_T Size,
    IN ULONG ProtectionMask
    )

/*++

Routine Description:

    This routine checks to see if the specified VAD is secured in such
    a way as to conflict with the address range and protection mask
    specified.

Arguments:

    Vad - Supplies a pointer to the VAD containing the address range.

    Base - Supplies the base of the range the protection starts at.

    Size - Supplies the size of the range.

    ProtectionMask - Supplies the protection mask being set.

Return Value:

    Status value.

Environment:

    Kernel mode.

--*/

{
    PVOID End;
    PLIST_ENTRY Next;
    PMMSECURE_ENTRY Entry;
    NTSTATUS Status = STATUS_SUCCESS;

    End = (PVOID)((PCHAR)Base + Size - 1);

    if (ProtectionMask < MM_SECURE_DELETE_CHECK) {
        if ((Vad->u.VadFlags.NoChange == 1) &&
            (Vad->u2.VadFlags2.SecNoChange == 1) &&
            (Vad->u.VadFlags.Protection != ProtectionMask)) {

            //
            // An attempt is made at changing the protection
            // of a SEC_NO_CHANGE section - return an error.
            //

            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto done;
        }
    }
    else {

        //
        // Deletion - set to no-access for check.  SEC_NOCHANGE allows
        // deletion, but does not allow page protection changes.
        //

        ProtectionMask = 0;
    }

    if (Vad->u2.VadFlags2.OneSecured) {

        if (((ULONG_PTR)Base <= ((PMMVAD_LONG)Vad)->u3.Secured.EndVpn) &&
             ((ULONG_PTR)End >= ((PMMVAD_LONG)Vad)->u3.Secured.StartVpn)) {

            //
            // This region conflicts, check the protections.
            //

            if (MI_IS_GUARD (ProtectionMask)) {
                Status = STATUS_INVALID_PAGE_PROTECTION;
                goto done;
            }

            if (Vad->u2.VadFlags2.ReadOnly) {
                if (MmReadWrite[ProtectionMask] < 10) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
            }
            else {
                if (MmReadWrite[ProtectionMask] < 11) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
            }
        }

    }
    else if (Vad->u2.VadFlags2.MultipleSecured) {

        Next = ((PMMVAD_LONG)Vad)->u3.List.Flink;
        do {
            Entry = CONTAINING_RECORD( Next,
                                       MMSECURE_ENTRY,
                                       List);

            if (((ULONG_PTR)Base <= Entry->EndVpn) &&
                ((ULONG_PTR)End >= Entry->StartVpn)) {

                //
                // This region conflicts, check the protections.
                //

                if (MI_IS_GUARD (ProtectionMask)) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
    
                if (Entry->u2.VadFlags2.ReadOnly) {
                    if (MmReadWrite[ProtectionMask] < 10) {
                        Status = STATUS_INVALID_PAGE_PROTECTION;
                        goto done;
                    }
                }
                else {
                    if (MmReadWrite[ProtectionMask] < 11) {
                        Status = STATUS_INVALID_PAGE_PROTECTION;
                        goto done;
                    }
                }
            }
            Next = Entry->List.Flink;
        } while (Entry->List.Flink != &((PMMVAD_LONG)Vad)->u3.List);
    }

done:
    return Status;
}

