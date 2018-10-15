/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   acceschk.c

Abstract:

    This module contains the access check routines for memory management.

--*/

#include "mi.h"

#if defined(_WIN64)
#include "wow64t.h"
#if PAGE_SIZE != PAGE_SIZE_X86NT
#define EMULATE_USERMODE_STACK_4K       1
#else
#define PAGE_4K 0x1000
#define PAGE_4K_ALIGN(Va) ((PVOID)((ULONG_PTR)(Va) & ~(PAGE_4K - 1)))
#define ROUND_TO_4K_PAGES(Size)  (((ULONG_PTR)(Size) + PAGE_4K - 1) & ~(PAGE_4K - 1))
#endif
#endif

#pragma alloc_text(PAGE, MiCheckForUserStackOverflow)

//
// MmReadWrite yields 0 if no-access, 10 if read-only, 11 if read-write.
// It is indexed by a page protection.  The value of this array is added
// to the !WriteOperation value.  If the value is 10 or less an access
// violation is issued (read-only - write_operation) = 9,
// (read_only - read_operation) = 10, etc.
//

CCHAR MmReadWrite[32] = {1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11 };

NTSTATUS
MiClearStackGuardBits (
    IN PVOID CurrentSP,
    IN PVOID StackBase,
    IN PVOID DeallocationStack
    );

NTSTATUS
MiClearBackingStoreGuardBits (
    IN PVOID CurrentBSP,
    IN PVOID StackBase,
    IN PVOID DeallocationBStore
    );


NTSTATUS
MiAccessCheck (
    IN PMMPTE PointerPte,
    IN ULONG_PTR WriteOperation,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Protection,
    IN PVOID TrapInformation,
    IN BOOLEAN CallerHoldsPfnLock
    )

/*++

Routine Description:



Arguments:

    PointerPte - Supplies the pointer to the PTE which caused the
                 page fault.

    WriteOperation - Supplies nonzero if the operation is a write, 0 if
                     the operation is a read.

    PreviousMode - Supplies the previous mode, one of UserMode or KernelMode.

    Protection - Supplies the protection mask to check.

    TrapInformation - Supplies the trap information.

    CallerHoldsPfnLock - Supplies TRUE if the PFN lock is held, FALSE otherwise.

Return Value:

    Returns TRUE if access to the page is allowed, FALSE otherwise.

Environment:

    Kernel mode, APCs disabled.

--*/

{
    MMPTE PteContents;
    KIRQL OldIrql;
    PMMPFN Pfn1;

    //
    // Check to see if the owner bit allows access to the previous mode.
    // Access is not allowed if the owner is kernel and the previous
    // mode is user.  Access is also disallowed if the write operation
    // is true and the write field in the PTE is false.
    //

    //
    // If both an access violation and a guard page violation could
    // occur for the page, the access violation must be returned.
    //

    if (PreviousMode == UserMode) {
        if (PointerPte > MiHighestUserPte) {
            return STATUS_ACCESS_VIOLATION;
        }
    }

    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 1) {

        //
        // Valid pages cannot be guard page violations.
        //

        if (WriteOperation != 0) {

            if ((PteContents.u.Hard.Write == 1) ||
                (PteContents.u.Hard.CopyOnWrite == 1)) {

                return STATUS_SUCCESS;
            }
            return STATUS_ACCESS_VIOLATION;
        }

        return STATUS_SUCCESS;
    }

    if (WriteOperation != 0) {
        WriteOperation = 1;
    }

    if ((MmReadWrite[Protection] - (CCHAR)WriteOperation) < 10) {
        return STATUS_ACCESS_VIOLATION;
    }

    //
    // Check for a guard page fault.
    //

    if (MI_IS_GUARD (Protection)) {

        //
        // If this thread is attached to a different process,
        // return an access violation rather than a guard
        // page exception.  This prevents problems with unwanted
        // stack expansion and unexpected guard page behavior
        // from debuggers.
        //

        if (KeIsAttachedProcess ()) {
            return STATUS_ACCESS_VIOLATION;
        }

        //
        // If this is a user mode SLIST fault then don't remove the guard bit.
        //

        if (KeInvalidAccessAllowed (TrapInformation)) {
            return STATUS_ACCESS_VIOLATION;
        }

        //
        // Check to see if this is a transition PTE. If so, the
        // PFN database original contents field needs to be updated.
        //

        if ((PteContents.u.Soft.Transition == 1) &&
            (PteContents.u.Soft.Prototype == 0)) {

            //
            // Acquire the PFN lock and check to see if the
            // PTE is still in the transition state. If so,
            // update the original PTE in the PFN database.
            //

            SATISFY_OVERZEALOUS_COMPILER (OldIrql = PASSIVE_LEVEL);

            if (CallerHoldsPfnLock == FALSE) {
                LOCK_PFN (OldIrql);
            }

            PteContents = *PointerPte;
            if ((PteContents.u.Soft.Transition == 1) &&
                (PteContents.u.Soft.Prototype == 0)) {

                //
                // Still in transition, update the PFN database.
                //

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                //
                // Note that forked processes using guard pages only take the
                // guard page fault when the first thread in either process
                // access the address.  This seems to be the best behavior we
                // can provide users of this API as we must allow the first
                // thread to make forward progress and the guard attribute is
                // stored in the shared fork prototype PTE.
                //

                if (PteContents.u.Soft.Protection == MM_NOACCESS) {
                    ASSERT ((Pfn1->u3.e1.PrototypePte == 1) &&
                            (MiLocateCloneAddress (PsGetCurrentProcess (), Pfn1->PteAddress) != NULL));
                    if (CallerHoldsPfnLock == FALSE) {
                        UNLOCK_PFN (OldIrql);
                    }
                    return STATUS_ACCESS_VIOLATION;
                }

                ASSERT ((Pfn1->u3.e1.PrototypePte == 0) ||
                        (MiLocateCloneAddress (PsGetCurrentProcess (), Pfn1->PteAddress) != NULL));
                Pfn1->OriginalPte.u.Soft.Protection =
                                      Protection & ~MM_GUARD_PAGE;
            }
            if (CallerHoldsPfnLock == FALSE) {
                UNLOCK_PFN (OldIrql);
            }
        }

        PointerPte->u.Soft.Protection = Protection & ~MM_GUARD_PAGE;

        return STATUS_GUARD_PAGE_VIOLATION;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
FASTCALL
MiCheckForUserStackOverflow (
    IN PVOID FaultingAddress,
    IN PVOID OpaqueTrapInformation
    )

/*++

Routine Description:

    This routine checks to see if the faulting address is within
    the stack limits and if so tries to create another guard
    page on the stack.  A stack overflow is returned if the
    creation of a new guard page fails or if the stack is in
    the following form:

            +----------------+ DeallocationBStore
            |                | last page of bstore (always no access)
            +----------------+
            |   allocated    |
            |                |
      ^     |                |
      |     |                |
     BSP    |                |
    growth  |                |
            |      ...       |
            |      ...       |

    stack   +----------------+  StackBase
    growth  |                |
      |     |                |
      v     |                |
            |   allocated    |
            |                |
            |      ...       |
            |                |
            +----------------+
            | old guard page | <- faulting address is in this page.
            +----------------+
            |                |
            +----------------+
            |                | last page of stack (always no access)
            +----------------+ DeallocationStack

    In this case, the page before the last page is committed, but
    not as a guard page and a STACK_OVERFLOW condition is returned.

Arguments:

    FaultingAddress - Supplies the virtual address of the page which
                      was a guard page.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode. No mutexes held.

--*/

{
    PTEB Teb;
    PPEB Peb;
    PMMVAD Vad;
    ULONG GlobalFlag;
    ULONG_PTR NextPage;
    SIZE_T RegionSize;
    SIZE_T AdditionalSize;
    NTSTATUS status;
    PVOID DeallocationStack;
    PVOID MaximumAddress;
    PVOID *StackLimit;
    PVOID StackBase;
    PETHREAD Thread;
    ULONG_PTR PageSize;
    PEPROCESS Process;
    ULONG ProtectionFlags;
    ULONG OldProtection;
    PVOID NextVaToQuery;
    PKTRAP_FRAME TrapFrame;
#if defined(_WIN64)
    PTEB32 Teb32;

    Teb32 = NULL;
#endif

    //
    // Make sure we are not recursing with the address space mutex held.
    //

    Thread = PsGetCurrentThread ();

    if (Thread->AddressSpaceOwner == 1) {
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        return STATUS_GUARD_PAGE_VIOLATION;
    }

    //
    // If this thread is attached to a different process,
    // return an access violation rather than a guard
    // page exception.  This prevents problems with unwanted
    // stack expansion and unexpected guard page behavior
    // from debuggers.
    //
    // Note we must bail when attached because we reference fields in our
    // TEB below which have no relationship to the virtual address space
    // of the process we are attached to.  Rather than introduce random
    // behavior into applications, it's better to consistently return an AV
    // for this scenario (one thread trying to grow another's stack).
    //

    if (KeIsAttachedProcess ()) {
        return STATUS_GUARD_PAGE_VIOLATION;
    }

    TrapFrame = (PKTRAP_FRAME) OpaqueTrapInformation;

    Teb = Thread->Tcb.Teb;

    //
    // Use an exception handler as the TEB is within the user's
    // address space.
    //

    try {

        StackBase = Teb->NtTib.StackBase;
        DeallocationStack = Teb->DeallocationStack;

        RegionSize = (SIZE_T) Teb->GuaranteedStackBytes;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // An exception has occurred during the referencing of the
        // TEB or TIB, just return a guard page violation and
        // don't deal with the stack overflow.
        //

        return STATUS_GUARD_PAGE_VIOLATION;
    }

    DeallocationStack = PAGE_ALIGN (DeallocationStack);

    RegionSize = ROUND_TO_PAGES (RegionSize);

    Process = NULL;

    PageSize = PAGE_SIZE;
    ProtectionFlags = PAGE_READWRITE | PAGE_GUARD;

    StackLimit = &Teb->NtTib.StackLimit;

    MaximumAddress = DeallocationStack;

    //
    // If there is a stack guarantee (RegionSize nonzero), then increase
    // our guard page size by 1 so that even a subsequent fault that occurs
    // midway (instead of at the beginning) through the first guard page
    // will have the extra page to preserve the guarantee.
    //
    // At the same time, reduce the MaximumAddress so that the
    // highest possible guard page can still be used (otherwise our
    // size increase would falsely disallow this).
    //

    if (RegionSize != 0) {
        RegionSize += PAGE_SIZE;
        MaximumAddress = (PVOID) ((PCHAR) DeallocationStack - PAGE_SIZE);
    }

    if (RegionSize < GUARD_PAGE_SIZE) {
        RegionSize = GUARD_PAGE_SIZE;
    }

    AdditionalSize = RegionSize - GUARD_PAGE_SIZE;

    NextPage = (ULONG_PTR) PAGE_ALIGN (FaultingAddress) - RegionSize;

    //
    // The faulting address must be below the stack base and
    // above the stack limit.
    //

    if ((FaultingAddress >= StackBase) ||
        (FaultingAddress < DeallocationStack)) {

        //
        // Not within the native stack.
        //

#if defined (_WIN64)

        //
        // Also check for the 32-bit stack if this is a Wow64 process.
        //

        Process = PsGetCurrentProcessByThread (Thread);

        if (Process->Wow64Process != NULL) {

            try {

                Teb32 = WOW64_GET_TEB32 (Teb);

                if (Teb32 == NULL) {
                    ExRaiseStatus (STATUS_GUARD_PAGE_VIOLATION);
                }

                ProbeForReadSmallStructure (Teb32,
                                            sizeof(TEB32),
                                            sizeof(ULONG));

                StackBase = (PVOID) (ULONG_PTR) Teb32->NtTib.StackBase;
                DeallocationStack = (PVOID) (ULONG_PTR) Teb32->DeallocationStack;
                RegionSize = (SIZE_T) Teb32->GuaranteedStackBytes;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // An exception has occurred during the referencing of the
                // TEB or TIB, just return a guard page violation and
                // don't deal with the stack overflow.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }

            DeallocationStack = PAGE_4K_ALIGN (DeallocationStack);

            if ((FaultingAddress >= StackBase) ||
                (FaultingAddress < DeallocationStack)) {

                //
                // Not within the stack.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }

            PageSize = PAGE_4K;
            MaximumAddress = DeallocationStack;

            //
            // If there is a stack guarantee (RegionSize nonzero), then
            // increase our guard page size by 1 so that even a subsequent
            // fault that occurs midway (instead of at the beginning)
            // through the first guard page will have the extra page
            // to preserve the guarantee.
            //
            // At the same time, reduce the MaximumAddress so that the
            // highest possible guard page can still be used (otherwise our
            // size increase would falsely disallow this).
            //

            if (RegionSize != 0) {
                RegionSize += PAGE_4K;
                MaximumAddress = (PVOID) ((PCHAR) DeallocationStack - PAGE_4K);
            }

            RegionSize = ROUND_TO_4K_PAGES (RegionSize);

            if (RegionSize < PAGE_4K) {
                RegionSize = PAGE_4K;
            }

            StackLimit = (PVOID *)&Teb32->NtTib.StackLimit;

            AdditionalSize = RegionSize - PAGE_4K;

            NextPage = (ULONG_PTR) PAGE_4K_ALIGN (FaultingAddress) - RegionSize;
        
#if EMULATE_USERMODE_STACK_4K

            //
            // Don't set the guard bit in the native PTE - just set
            // it in the AltPte.
            //

            ProtectionFlags &= ~PAGE_GUARD;
#endif

            goto ExtendTheStack;
        }

#endif
        //
        // Not within the stack.
        //

        return STATUS_GUARD_PAGE_VIOLATION;
    }

#if defined (_WIN64)
ExtendTheStack:
#endif

    //
    // If the image was marked for no stack extensions,
    // return stack overflow immediately.
    //

    Process = PsGetCurrentProcessByThread (Thread);
    Peb = Process->Peb;

    try {
        GlobalFlag = Peb->NtGlobalFlag;
    } except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // An exception has occurred during the referencing of the
        // PEB, just return a guard page violation and
        // don't deal with the stack overflow.
        //

        return STATUS_GUARD_PAGE_VIOLATION;
    }

    //
    // NextPage is within the current stack, if there is ample
    // room for another guard page then attempt to commit it.
    //

    if ((NextPage - PageSize) <= (ULONG_PTR) MaximumAddress) {

        //
        // There is no more room for expansion, attempt to
        // commit the page(s) before the last page of the stack.
        //

        NextPage = (ULONG_PTR) DeallocationStack + PageSize;

        status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                          (PVOID *)&NextPage,
                                          0,
                                          &RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE);

        if (NT_SUCCESS (status)) {

            try {
#if defined(_WIN64)
                if (Teb32 != NULL) {
                    *(PULONG) StackLimit = (ULONG) NextPage;
                }
                else
#endif
                *StackLimit = (PVOID) NextPage;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // An exception has occurred during the referencing of the
                // TIB, just return a guard page violation and
                // don't deal with the stack overflow.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }
        }

        return STATUS_STACK_OVERFLOW;
    }

    if ((GlobalFlag & FLG_DISABLE_STACK_EXTENSION)
#if defined(_WIN64)
        //
        // Don't do this for emulation processes because it's not worth the
        // work to clear the guard bits in the native stack, the emulation
        // stack and the backing store PTEs.
        //
        && (Teb32 == NULL)
#endif
        ) {

        //
        // Since guard pages may get inserted anywhere in the stack (including
        // prior to the committed region), make sure the PTE is not committed
        // before failing to grow it.
        //

        LOCK_ADDRESS_SPACE (Process);

        Vad = MiCheckForConflictingVad (Process,
                                        NextPage,
                                        NextPage);

        if (Vad != NULL) {

            LOCK_WS_SHARED (Thread, Process);
    
            if (MiQueryAddressState ((PVOID) NextPage,
                                     Vad,
                                     Process,
                                     &OldProtection,
                                     &NextVaToQuery) != MEM_RESERVE) {
    
                //
                // The state is committed (not reserved) so allow the growth.
                //

                GlobalFlag = 0;
            }

            UNLOCK_WS_SHARED (Thread, Process);
        }

        UNLOCK_ADDRESS_SPACE (Process);

        if (GlobalFlag & FLG_DISABLE_STACK_EXTENSION) {

            //
            // We're handing back an overflow so remove any guard bits now
            // so the app can handle it.
            //

            MiClearStackGuardBits (FaultingAddress,
                                   StackBase,
                                   DeallocationStack);

            return STATUS_STACK_OVERFLOW;
        }
    }

    try {
#if defined(_WIN64)

        if (Teb32 != NULL) {

            //
            // Update the 32-bit stack limit.
            //

            *(PULONG) StackLimit = (ULONG) (NextPage + RegionSize);
        }
        else
#endif
        *StackLimit = (PVOID)(NextPage + RegionSize);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // An exception has occurred during the referencing of the
        // TEB or TIB, just return a guard page violation and
        // don't deal with the stack overflow.
        //

        return STATUS_GUARD_PAGE_VIOLATION;
    }

    //
    // Set the guard page(s).  For Wow64 processes the protection
    // will not contain the PAGE_GUARD bit.  This is ok since in these
    // cases we will set the bit for the top emulated 4K page.
    //

    status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                      (PVOID *)&NextPage,
                                      0,
                                      &RegionSize,
                                      MEM_COMMIT,
                                      ProtectionFlags);

    if (NT_SUCCESS (status) || (status == STATUS_ALREADY_COMMITTED)) {

#if EMULATE_USERMODE_STACK_4K

        if (Teb32 != NULL) {
            
            LOCK_ADDRESS_SPACE (Process);

            MiProtectFor4kPage ((PVOID)NextPage,
                                RegionSize,
                                (MM_READWRITE | MM_GUARD_PAGE),
                                ALT_CHANGE,
                                Process);

            UNLOCK_ADDRESS_SPACE (Process);
        }

#endif

        //
        // The guard page is now committed or stack space is
        // already present, return success.
        //

        return STATUS_PAGE_FAULT_GUARD_PAGE;
    }

    //
    // The attempt to commit guard page(s) failed.  If there are any
    // guard pages between the base of the attempt and the faulting
    // stack address, then strip their guard bits now so the application
    // can handle the overflow exception without hitting successive
    // exceptions.  Note if an application desires successive exceptions,
    // it must explicitly set guard page protection on the desired
    // virtual addresses upon return.
    //

    if (AdditionalSize != 0) {

        NextPage += PageSize;

        RegionSize -= PageSize;

        ZwProtectVirtualMemory (NtCurrentProcess (),
                                (PVOID *)&NextPage,
                                &RegionSize,
                                PAGE_READWRITE,
                                &OldProtection);
    }

    return STATUS_STACK_OVERFLOW;
}

NTSTATUS
MiClearStackGuardBits (
    IN PVOID CurrentSP,
    IN PVOID StackBase,
    IN PVOID DeallocationStack
    )
{
    PTEB Teb;
    PMMVAD Vad;
    ULONG_PTR NextPage;
    SIZE_T RegionSize;
    NTSTATUS status;
    PVOID *StackLimit;
    PETHREAD Thread;
    ULONG_PTR PageSize;
    PEPROCESS Process;
    ULONG OldProtection;
    PVOID NextVaToQuery;
#if defined (_WIN64)
    PTEB32 Teb32;

    Teb32 = NULL;
#endif

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

    Teb = Thread->Tcb.Teb;
    StackLimit = &Teb->NtTib.StackLimit;

    PageSize = PAGE_SIZE;

    if ((CurrentSP >= StackBase) || (CurrentSP < DeallocationStack)) {

        //
        // Not within the native stack.
        //
        // Also check for the 32-bit stack if this is a Wow64 process.
        //

        StackLimit = NULL;

#if defined (_WIN64)
        if (Process->Wow64Process != NULL) {

            try {

                Teb32 = WOW64_GET_TEB32 (Teb);

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // An exception has occurred during the referencing of the
                // TEB or TIB, just return a guard page violation and
                // don't deal with the stack overflow.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }

            if (Teb32 == NULL) {
                return STATUS_GUARD_PAGE_VIOLATION;
            }

            try {
                ProbeForReadSmallStructure (Teb32,
                                            sizeof(TEB32),
                                            sizeof(ULONG));

                StackBase = (PVOID) (ULONG_PTR) Teb32->NtTib.StackBase;
                DeallocationStack = (PVOID) (ULONG_PTR) Teb32->DeallocationStack;

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // An exception has occurred during the referencing of the
                // TEB or TIB, just return a guard page violation and
                // don't deal with the stack overflow.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }

            if ((CurrentSP >= StackBase) || (CurrentSP < DeallocationStack)) {

                //
                // Not within the stack.
                //

                return STATUS_GUARD_PAGE_VIOLATION;
            }

#if EMULATE_USERMODE_STACK_4K
            PageSize = PAGE_4K;
#endif
            StackLimit = (PVOID *)&Teb32->NtTib.StackLimit;
        }
#endif
    }

    if (StackLimit == NULL) {
        return STATUS_STACK_OVERFLOW;
    }

    if ((CurrentSP < StackBase) && (CurrentSP >= DeallocationStack)) {

        RegionSize = 0;
        SATISFY_OVERZEALOUS_COMPILER (NextPage = 0);

        LOCK_ADDRESS_SPACE (Process);

        Vad = MiCheckForConflictingVad (Process, CurrentSP, CurrentSP);

        if (Vad != NULL) {

            CurrentSP = (PVOID)((ULONG_PTR)CurrentSP & ~(PageSize - 1));

            if (DeallocationStack < MI_VPN_TO_VA (Vad->StartingVpn)) {
                DeallocationStack = MI_VPN_TO_VA (Vad->StartingVpn);
            }

            LOCK_WS_SHARED (Thread, Process);
    
            while (CurrentSP >= DeallocationStack) {

                if (MiQueryAddressState (CurrentSP,
                                         Vad,
                                         Process,
                                         &OldProtection,
                                         &NextVaToQuery) == MEM_RESERVE) {
                    break;
                }
    
                //
                // The state is committed (not reserved) so check
                // for a guard bit.
                //

                if (OldProtection == (PAGE_GUARD | PAGE_READWRITE)) {
                    if (RegionSize == 0) {
                        NextPage = (ULONG_PTR) CurrentSP;
                        RegionSize = 1;
                    }
                }

                CurrentSP = (PVOID)((PCHAR) CurrentSP - PageSize);
            }

            UNLOCK_WS_SHARED (Thread, Process);
        }

        UNLOCK_ADDRESS_SPACE (Process);

        if (RegionSize != 0) {

            RegionSize = NextPage - (ULONG_PTR) CurrentSP;
            CurrentSP = (PVOID)((PCHAR) CurrentSP + PageSize);

            status = ZwAllocateVirtualMemory (NtCurrentProcess (),
                                              &CurrentSP,
                                              0,
                                              &RegionSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);
            if (NT_SUCCESS (status)) {

                try {

#if defined (_WIN64)
                    if (Teb32 != NULL) {
                        *(PULONG) StackLimit = (ULONG) (ULONG_PTR) CurrentSP;
                    }
                    else
#endif
                        *StackLimit = (PVOID) CurrentSP;

                } except (EXCEPTION_EXECUTE_HANDLER) {

                    //
                    // An exception has occurred during the referencing
                    // of the TEB, just return a guard page violation
                    // instead.
                    //

                    return STATUS_GUARD_PAGE_VIOLATION;
                }
            }
        }
    }

    return STATUS_STACK_OVERFLOW;
}

