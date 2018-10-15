/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   mmfault.c

Abstract:

    This module contains the handlers for access check, page faults
    and write faults.

--*/

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MmGetExecuteOptions)
#pragma alloc_text(PAGE, MmGetImageInformation)
#endif

#define PROCESS_FOREGROUND_PRIORITY (9)

LONG MiDelayPageFaults;

#if DBG
ULONG MmProtoPteVadLookups = 0;
ULONG MmProtoPteDirect = 0;
ULONG MmAutoEvaluate = 0;

ULONG MmInjectUserInpageErrors;
ULONG MmInjectedUserInpageErrors;

PMMPTE MmPteHit = NULL;

LOGICAL
MiInPageAllowed (
    IN PVOID VirtualAddress,
    IN PVOID TrapInformation
    );

#endif


NTSTATUS
MmAccessFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    )

/*++

Routine Description:

    This function is called by the kernel on data or instruction
    access faults.  The access fault was detected due to either
    an access violation, a PTE with the present bit clear, or a
    valid PTE with the dirty bit clear and a write operation.

    Also note that the access violation and the page fault could
    occur because of the Page Directory Entry contents as well.

    This routine determines what type of fault it is and calls
    the appropriate routine to handle the page fault or the write
    fault.

Arguments:

    FaultStatus - Supplies fault status information bits.

    VirtualAddress - Supplies the virtual address which caused the fault.

    PreviousMode - Supplies the mode (kernel or user) in which the fault
                   occurred.

    TrapInformation - Opaque information about the trap, interpreted by the
                      kernel, not Mm.  Needed to allow fast interlocked access
                      to operate correctly.

Return Value:

    Returns the status of the fault handling operation.  Can be one of:
        - Success.
        - Access Violation.
        - Guard Page Violation.
        - In-page Error.

Environment:

    Kernel mode.

--*/

{
    PMMVAD ProtoVad;
    PMMPTE PointerPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE PointerProtoPte;
    ULONG ProtectionCode;
    MMPTE TempPte;
    PEPROCESS CurrentProcess;
    KIRQL PreviousIrql;
    KIRQL WsIrql;
    NTSTATUS status;
    ULONG ProtectCode;
    PETHREAD Thread;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WorkingSetIndex;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PPAGE_FAULT_NOTIFY_ROUTINE NotifyRoutine;
    PEPROCESS FaultProcess;
    PMMSUPPORT Ws;
    LOGICAL SessionAddress;
    PVOID UsedPageTableHandle;
    ULONG BarrierStamp;
    LOGICAL ApcNeeded;
    LOGICAL RecheckAccess;

    PointerProtoPte = NULL;

    //
    // If the address is not canonical then return FALSE as the caller (which
    // may be the kernel debugger) is not expecting to get an unimplemented
    // address bit fault.
    //

    if (MI_RESERVED_BITS_CANONICAL(VirtualAddress) == FALSE) {

        if (PreviousMode == UserMode) {
            return STATUS_ACCESS_VIOLATION;
        }

        if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
            return STATUS_ACCESS_VIOLATION;
        }

        KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                      (ULONG_PTR)VirtualAddress,
                      FaultStatus,
                      (ULONG_PTR)TrapInformation,
                      4);
    }

    PreviousIrql = KeGetCurrentIrql ();

    //
    // Get the pointer to the PDE and the PTE for this page.
    //

    PointerPte = MiGetPteAddress (VirtualAddress);
    PointerPde = MiGetPdeAddress (VirtualAddress);
    PointerPpe = MiGetPpeAddress (VirtualAddress);
    PointerPxe = MiGetPxeAddress (VirtualAddress);

#if DBG
    if (PointerPte == MmPteHit) {
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
            "MM: PTE hit at %p\n", MmPteHit);
        DbgBreakPoint ();
    }
#endif

    if (PreviousIrql > APC_LEVEL) {

        //
        // The PFN database lock is an executive spin-lock.  The pager could
        // get dirty faults or lock faults while servicing and it already owns
        // the PFN database lock.
        //

#if (_MI_PAGING_LEVELS < 3)
        MiCheckPdeForPagedPool (VirtualAddress);
#endif

        if (
#if (_MI_PAGING_LEVELS >= 4)
            (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS >= 3)
            (PointerPpe->u.Hard.Valid == 0) ||
#endif
            (PointerPde->u.Hard.Valid == 0) ||

            ((!MI_PDE_MAPS_LARGE_PAGE (PointerPde)) && (PointerPte->u.Hard.Valid == 0))) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

            KdPrint(("MM:***PAGE FAULT AT IRQL > 1  Va %p, IRQL %lx\n",
                     VirtualAddress,
                     PreviousIrql));

            if (TrapInformation != NULL) {
                MI_DISPLAY_TRAP_INFORMATION (TrapInformation);
            }

            //
            // Signal the fatal error to the trap handler.
            //

            return STATUS_IN_PAGE_ERROR | 0x10000000;

        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                (!MI_IS_PTE_EXECUTABLE (PointerPde))) {

                KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                              (ULONG_PTR) VirtualAddress,
                              (ULONG_PTR) PointerPde->u.Long,
                              (ULONG_PTR) TrapInformation,
                              3);
            }

            return STATUS_SUCCESS;
        }

        if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
            (PointerPte->u.Hard.CopyOnWrite != 0)) {

            KdPrint(("MM:***PAGE FAULT AT IRQL > 1  Va %p, IRQL %lx\n",
                     VirtualAddress,
                     PreviousIrql));

            if (TrapInformation != NULL) {
                MI_DISPLAY_TRAP_INFORMATION (TrapInformation);
            }

            //
            // use reserved bit to signal fatal error to trap handlers
            //

            return STATUS_IN_PAGE_ERROR | 0x10000000;
        }

        //
        // If PTE mappings with various protections are active and the faulting
        // address lies within these mappings, resolve the fault with
        // the appropriate protections.
        //

        if (!IsListEmpty (&MmProtectedPteList)) {

            if (MiCheckSystemPteProtection (
                                MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                                VirtualAddress) == TRUE) {

                return STATUS_SUCCESS;
            }
        }

        //
        // The PTE is valid and accessible, another thread must
        // have faulted the PTE in already, or the access bit
        // is clear and this is a access fault - blindly set the
        // access bit and dismiss the fault.
        //

        if (MI_FAULT_STATUS_INDICATES_WRITE(FaultStatus)) {

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

            if (((PointerPte->u.Long & MM_PTE_WRITE_MASK) == 0) &&
                ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0)) {
                
                KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)PointerPte->u.Long,
                              (ULONG_PTR)TrapInformation,
                              10);
            }
        }

        //
        // Ensure execute access is enabled in the PTE.
        //

        if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
            (!MI_IS_PTE_EXECUTABLE (PointerPte))) {

            KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                          (ULONG_PTR)VirtualAddress,
                          (ULONG_PTR)PointerPte->u.Long,
                          (ULONG_PTR)TrapInformation,
                          0);
        }

        MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, FALSE);
        return STATUS_SUCCESS;
    }

    ApcNeeded = FALSE;
    WsIrql = MM_NOIRQL;

    if (VirtualAddress >= MmSystemRangeStart) {

        //
        // This is a fault in the system address space.  User
        // mode access is not allowed.
        //

        if (PreviousMode == UserMode) {
            return STATUS_ACCESS_VIOLATION;
        }

#if (_MI_PAGING_LEVELS >= 4)
        if (PointerPxe->u.Hard.Valid == 0) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          7);
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)
        if (PointerPpe->u.Hard.Valid == 0) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          5);
        }
#endif

        if (PointerPde->u.Hard.Valid == 1) {

            if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

                if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                    (!MI_IS_PTE_EXECUTABLE (PointerPde))) {

                    KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                                  (ULONG_PTR) VirtualAddress,
                                  (ULONG_PTR) PointerPde->u.Long,
                                  (ULONG_PTR) TrapInformation,
                                  4);
                }

                return STATUS_SUCCESS;
            }

        }
        else {

#if (_MI_PAGING_LEVELS >= 3)
            if ((VirtualAddress >= (PVOID)PTE_BASE) &&
                (VirtualAddress < (PVOID)MiGetPteAddress (HYPER_SPACE))) {

                //
                // This is a user mode PDE entry being faulted in by the Mm
                // referencing the page table page.  This needs to be done
                // with the working set lock so that the PPE validity can be
                // relied on throughout the fault processing.
                //
                // The case when Mm faults in PPE entries by referencing the
                // page directory page is correctly handled by falling through
                // the below code.
                //
    
                goto UserFault;
            }
#else
            MiCheckPdeForPagedPool (VirtualAddress);
#endif

            if (PointerPde->u.Hard.Valid == 0) {

                if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                              (ULONG_PTR)VirtualAddress,
                              FaultStatus,
                              (ULONG_PTR)TrapInformation,
                              2);
            }

            //
            // The PDE is now valid, the PTE can be examined below.
            //
        }

        SessionAddress = MI_IS_SESSION_ADDRESS (VirtualAddress);

        TempPte = *PointerPte;

        if (TempPte.u.Hard.Valid == 1) {

            //
            // If PTE mappings with various protections are active
            // and the faulting address lies within these mappings,
            // resolve the fault with the appropriate protections.
            //

            if (!IsListEmpty (&MmProtectedPteList)) {

                if (MiCheckSystemPteProtection (
                        MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                        VirtualAddress) == TRUE) {

                    return STATUS_SUCCESS;
                }
            }

            //
            // Session space faults cannot early exit here because
            // it may be a copy on write which must be checked for
            // and handled below.
            //

            if (SessionAddress == FALSE) {

                //
                // Acquire the PFN lock, check to see if the address is
                // still valid if writable, update dirty bit.
                //

                LOCK_PFN (OldIrql);

                TempPte = *PointerPte;

                if (TempPte.u.Hard.Valid == 1) {

                    Pfn1 = MI_PFN_ELEMENT (TempPte.u.Hard.PageFrameNumber);

                    if ((MI_FAULT_STATUS_INDICATES_WRITE(FaultStatus)) &&
                        ((TempPte.u.Long & MM_PTE_WRITE_MASK) == 0) &&
                        ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0)) {
            
                        KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                      (ULONG_PTR)VirtualAddress,
                                      (ULONG_PTR)TempPte.u.Long,
                                      (ULONG_PTR)TrapInformation,
                                      11);
                    }

                    //
                    // Ensure execute access is enabled in the PTE.
                    //

                    if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                        (!MI_IS_PTE_EXECUTABLE (&TempPte))) {

                        KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                                      (ULONG_PTR)VirtualAddress,
                                      (ULONG_PTR)TempPte.u.Long,
                                      (ULONG_PTR)TrapInformation,
                                      1);
                    }

                    MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
                }
                UNLOCK_PFN (OldIrql);
                return STATUS_SUCCESS;
            }
        }

#if (_MI_PAGING_LEVELS < 3)

        //
        // Session data had its PDE validated above.  Session PTEs haven't
        // had the hardware PDE validated because the checks above would
        // have gone to the selfmap entry.
        //
        // So validate the real session PDE now if the fault was on a PTE.
        //

        if (MI_IS_SESSION_PTE (VirtualAddress)) {

            status = MiCheckPdeForSessionSpace (VirtualAddress);

            if (!NT_SUCCESS (status)) {

                //
                // This thread faulted on a session space access, but this
                // process does not have one.
                //
                // The system process which contains the worker threads
                // NEVER has a session space - if code accidentally queues a
                // worker thread that points to a session space buffer, a
                // fault will occur.
                //
                // The only exception to this is when the working set manager
                // attaches to a session to age or trim it.  However, the
                // working set manager will never fault and so the bugcheck
                // below is always valid.  Note that a worker thread can get
                // away with a bad access if it happens while the working set
                // manager is attached, but there's really no way to prevent
                // this case which is a driver bug anyway.
                //

                if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                              (ULONG_PTR)VirtualAddress,
                              FaultStatus,
                              (ULONG_PTR)TrapInformation,
                              6);
            }
        }

#endif

        //
        // Handle page table or hyperspace faults as if they were user faults.
        //

        if (MI_IS_PAGE_TABLE_OR_HYPER_ADDRESS (VirtualAddress)) {

#if (_MI_PAGING_LEVELS < 3)

            if (MiCheckPdeForPagedPool (VirtualAddress) == STATUS_WAIT_1) {
                return STATUS_SUCCESS;
            }

#endif

            goto UserFault;
        }

        //
        //
        // Acquire the relevant working set mutex.  While the mutex
        // is held, this virtual address cannot go from valid to invalid.
        //
        // Fall though to further fault handling.
        //

        Thread = PsGetCurrentThread ();

        if (SessionAddress == FALSE) {
            FaultProcess = NULL;
            Ws = &MmSystemCacheWs;

            if (Thread->SameThreadApcFlags & PS_SAME_THREAD_FLAGS_OWNS_A_WORKING_SET) {

                //
                // If any working set pushlock is held, it is illegal to fault.
                // The only time this can happen is due to the slist races,
                // so check for that now.  If that's not the cause, then
                // it's an IRQL > 1 bugcheck.
                //

                if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                return STATUS_IN_PAGE_ERROR | 0x10000000;
            }
        }
        else {
            FaultProcess = HYDRA_PROCESS;
            Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;

            if ((Thread->OwnsSessionWorkingSetExclusive) ||
                (Thread->OwnsSessionWorkingSetShared)) {

                if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                //
                // Recursively trying to acquire the session working set
                // mutex - cause an IRQL > 1 bugcheck.
                //

                return STATUS_IN_PAGE_ERROR | 0x10000000;
            }
        }

        //
        // Explicitly raise irql because MiDispatchFault will need
        // to release the working set mutex if an I/O is needed.
        //
        // Since releasing the mutex lowers irql to the value stored
        // in the mutex, we must ensure the stored value is at least
        // APC_LEVEL.  Otherwise a kernel special APC (which can
        // reference paged pool) can arrive when the mutex is released
        // which is before the I/O is actually queued --> ie: deadlock!
        //

        KeRaiseIrql (APC_LEVEL, &WsIrql);
        LOCK_WORKING_SET (Thread, Ws);

        TempPte = *PointerPte;

        if (TempPte.u.Hard.Valid != 0) {

            //
            // The PTE is already valid, this must be an access, dirty or
            // copy on write fault.
            //

            if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                ((TempPte.u.Long & MM_PTE_WRITE_MASK) == 0) &&
                (TempPte.u.Hard.CopyOnWrite == 0)) {

                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (&TempPte));

                if ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0) {
        
                    PLIST_ENTRY NextEntry;
                    PIMAGE_ENTRY_IN_SESSION Image;

                    Image = (PIMAGE_ENTRY_IN_SESSION) -1;

                    if (SessionAddress == TRUE) {

                        NextEntry = MmSessionSpace->ImageList.Flink;

                        while (NextEntry != &MmSessionSpace->ImageList) {

                            Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

                            if ((VirtualAddress >= Image->Address) &&
                                (VirtualAddress <= Image->LastAddress)) {

                                if (Image->ImageLoading) {

                                    ASSERT (Pfn1->u3.e1.PrototypePte == 1);

                                    TempPte.u.Hard.CopyOnWrite = 1;

                                    //
                                    // Temporarily make the page copy on write,
                                    // this will be stripped shortly and made
                                    // fully writable.  No TB flush is necessary
                                    // as the PTE will be processed below.
                                    //
                                    // Even though the page's current backing
                                    // is the image file, the modified writer
                                    // will convert it to pagefile backing
                                    // when it notices the change later.
                                    //

                                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
                                    Image = NULL;

                                }
                                break;
                            }
                            NextEntry = NextEntry->Flink;
                        }
                    }

                    if (Image != NULL) {
                        KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                      (ULONG_PTR)VirtualAddress,
                                      (ULONG_PTR)TempPte.u.Long,
                                      (ULONG_PTR)TrapInformation,
                                      12);
                    }
                }
            }

            //
            // Ensure execute access is enabled in the PTE.
            //

            if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                (!MI_IS_PTE_EXECUTABLE (&TempPte))) {

                KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)TempPte.u.Long,
                              (ULONG_PTR)TrapInformation,
                              2);
            }

            //
            // Set the dirty bit in the PTE and the page frame.
            //

            if ((SessionAddress == TRUE) &&
                (MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                (TempPte.u.Hard.Write == 0)) {

                //
                // Check for copy-on-write.
                //
    
                ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (VirtualAddress));

                if (TempPte.u.Hard.CopyOnWrite == 0) {
            
                    KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                  (ULONG_PTR)VirtualAddress,
                                  (ULONG_PTR)TempPte.u.Long,
                                  (ULONG_PTR)TrapInformation,
                                  13);
                }
    
                MiCopyOnWrite (VirtualAddress, PointerPte);
            }
            else {
                LOCK_PFN (OldIrql);
                MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
                UNLOCK_PFN (OldIrql);
            }

            UNLOCK_WORKING_SET (Thread, Ws);
            ASSERT (WsIrql != MM_NOIRQL);
            KeLowerIrql (WsIrql);
            return STATUS_SUCCESS;
        }

        if (TempPte.u.Soft.Prototype != 0) {

            if ((MmProtectFreedNonPagedPool == TRUE) &&

                ((VirtualAddress >= MmNonPagedPoolStart) &&
                 (VirtualAddress < (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) ||

                ((VirtualAddress >= MmNonPagedPoolExpansionStart) &&
                 (VirtualAddress < MmNonPagedPoolEnd))) {

                //
                // This is an access to previously freed
                // non paged pool - bugcheck!
                //

                if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                    goto KernelAccessViolation;
                }

                KeBugCheckEx (DRIVER_CAUGHT_MODIFYING_FREED_POOL,
                              (ULONG_PTR) VirtualAddress,
                              FaultStatus,
                              PreviousMode,
                              4);
            }

            //
            // This is a PTE in prototype format, locate the corresponding
            // prototype PTE.
            //

            PointerProtoPte = MiPteToProto (&TempPte);

            if ((SessionAddress == TRUE) &&
                (TempPte.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED)) {

                PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                         &ProtectionCode,
                                                         &ProtoVad);
                if (PointerProtoPte == NULL) {
                    UNLOCK_WORKING_SET (Thread, Ws);
                    ASSERT (WsIrql != MM_NOIRQL);
                    KeLowerIrql (WsIrql);
                    return STATUS_IN_PAGE_ERROR | 0x10000000;
                }
            }
        }
        else if ((TempPte.u.Soft.Transition == 0) &&
                 (TempPte.u.Soft.Protection == 0)) {

            //
            // Page file format.  If the protection is ZERO, this
            // is a page of free system PTEs - bugcheck!
            //

            if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                goto KernelAccessViolation;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          0);
        }
        else if (TempPte.u.Soft.Protection == MM_NOACCESS) {

            if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                goto KernelAccessViolation;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          1);
        }

        if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
            (PointerProtoPte == NULL) &&
            (SessionAddress == FALSE) &&
            (TempPte.u.Hard.Valid == 0)) {

            if (TempPte.u.Soft.Transition == 1) {
                ProtectionCode = (ULONG) TempPte.u.Trans.Protection;
            }
            else {
                ProtectionCode = (ULONG) TempPte.u.Soft.Protection;
            }
                
            if ((ProtectionCode & MM_READWRITE) == 0) {

                KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)TempPte.u.Long,
                              (ULONG_PTR)TrapInformation,
                              14);
            }
        }

        status = MiDispatchFault (FaultStatus,
                                  VirtualAddress,
                                  PointerPte,
                                  PointerProtoPte,
                                  FALSE,
                                  FaultProcess,
                                  TrapInformation,
                                  NULL);

        ASSERT (KeAreAllApcsDisabled () == TRUE);

        if (Ws->Flags.GrowWsleHash == 1) {
            MiGrowWsleHash (Ws);
        }

        UNLOCK_WORKING_SET (Thread, Ws);

        ASSERT (WsIrql != MM_NOIRQL);
        KeLowerIrql (WsIrql);

        if (((Ws->PageFaultCount & 0xFFF) == 0) &&
            (MmAvailablePages < MM_PLENTY_FREE_LIMIT)) {

            //
            // The system cache or this session is taking too many faults,
            // delay execution so the modified page writer gets a quick shot.
            //

            if (PsGetCurrentThread()->MemoryMaker == 0) {

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) &MmShortTime);
            }
        }
        goto ReturnStatus3;
    }

UserFault:

    //
    // FAULT IN USER SPACE OR PAGE DIRECTORY/PAGE TABLE PAGES.
    //

    Thread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (Thread);

    if (MiDelayPageFaults ||
        ((MmModifiedPageListHead.Total >= (MmModifiedPageMaximum + 100)) &&
        (MmAvailablePages < (1024*1024 / PAGE_SIZE)) &&
            (CurrentProcess->ModifiedPageCount > ((64*1024)/PAGE_SIZE)))) {

        //
        // This process has placed more than 64k worth of pages on the modified
        // list.  Delay for a short period and set the count to zero.
        //

        KeDelayExecutionThread (KernelMode,
                                FALSE,
             (CurrentProcess->Pcb.BasePriority < PROCESS_FOREGROUND_PRIORITY) ?
                                    (PLARGE_INTEGER)&MmHalfSecond : (PLARGE_INTEGER)&Mm30Milliseconds);
        CurrentProcess->ModifiedPageCount = 0;
    }

#if DBG
    if ((PreviousMode == KernelMode) &&
        (PAGE_ALIGN (VirtualAddress) != (PVOID) MM_SHARED_USER_DATA_VA) &&
        (TrapInformation != NULL)) {

        if ((MmInjectUserInpageErrors & 0x2) ||
            (CurrentProcess->Flags & PS_PROCESS_INJECT_INPAGE_ERRORS)) {

            if (!MiInPageAllowed (VirtualAddress, TrapInformation)) {
                MmInjectedUserInpageErrors += 1;
                status = STATUS_DEVICE_NOT_CONNECTED;
                LOCK_WS (Thread, CurrentProcess);
                goto ReturnStatus2;
            }
        }
    }
#endif



    //
    // Block APCs and acquire the working set mutex.  This prevents any
    // changes to the address space and it prevents valid PTEs from becoming
    // invalid.
    //

    LOCK_WS (Thread, CurrentProcess);

#if (_MI_PAGING_LEVELS >= 4)

    //
    // Locate the Extended Page Directory Parent Entry which maps this virtual
    // address and check for accessibility and validity.  The page directory
    // parent page must be made valid before any other checks are made.
    //

    if (PointerPxe->u.Hard.Valid == 0) {

        //
        // If the PXE is zero, check to see if there is a virtual address
        // mapped at this location, and if so create the necessary
        // structures to map it.
        //

        if ((PointerPxe->u.Long == MM_ZERO_PTE) ||
            (PointerPxe->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

                MI_BREAK_ON_AV (VirtualAddress, 0);

                goto ReturnStatus2;

            }

            //
            // Build a demand zero PXE and operate on it.
            //

#if (_MI_PAGING_LEVELS > 4)
            ASSERT (FALSE);     // UseCounts will need to be kept.
#endif

            MI_WRITE_INVALID_PTE (PointerPxe, DemandZeroPde);
        }

        //
        // The PXE is not valid, call the page fault routine passing
        // in the address of the PXE.  If the PXE is valid, determine
        // the status of the corresponding PPE.
        //

        status = MiDispatchFault (TRUE,  //page table page always written
                                  PointerPpe,   // Virtual address
                                  PointerPxe,   // PTE (PXE in this case)
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  TrapInformation,
                                  NULL);

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        if (PointerPxe->u.Hard.Valid == 0) {

            //
            // The PXE is not valid, return the status.
            //

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPxe, PointerPpe, FALSE);

        //
        // Now that the PXE is accessible, fall through and get the PPE.
        //
    }
#endif

#if (_MI_PAGING_LEVELS >= 3)

    //
    // Locate the Page Directory Parent Entry which maps this virtual
    // address and check for accessibility and validity.  The page directory
    // page must be made valid before any other checks are made.
    //

    if (PointerPpe->u.Hard.Valid == 0) {

        //
        // If the PPE is zero, check to see if there is a virtual address
        // mapped at this location, and if so create the necessary
        // structures to map it.
        //

        if ((PointerPpe->u.Long == MM_ZERO_PTE) ||
            (PointerPpe->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

                MI_BREAK_ON_AV (VirtualAddress, 1);

                goto ReturnStatus2;

            }

#if (_MI_PAGING_LEVELS >= 4)

            //
            // Increment the count of non-zero page directory parent entries
            // for this page directory parent.
            //

            if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#endif

            //
            // Build a demand zero PPE and operate on it.
            //

            MI_WRITE_INVALID_PTE (PointerPpe, DemandZeroPde);
        }

        //
        // The PPE is not valid, call the page fault routine passing
        // in the address of the PPE.  If the PPE is valid, determine
        // the status of the corresponding PDE.
        //

        status = MiDispatchFault (TRUE,  //page table page always written
                                  PointerPde,   //Virtual address
                                  PointerPpe,   // PTE (PPE in this case)
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  TrapInformation,
                                  NULL);

        ASSERT (KeAreAllApcsDisabled () == TRUE);

#if (_MI_PAGING_LEVELS >= 4)

        //
        // Note that the page directory parent page itself could have been
        // paged out or deleted while MiDispatchFault was executing without
        // the working set pushlock, so this must be checked for here in the
        // PXE.
        //

        if (PointerPxe->u.Hard.Valid == 0) {

            //
            // The PXE is not valid, return the status.
            //

            goto ReturnStatus1;
        }
#endif

        if (PointerPpe->u.Hard.Valid == 0) {

            //
            // The PPE is not valid, return the status.
            //

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPpe, PointerPde, FALSE);

        //
        // Now that the PPE is accessible, fall through and get the PDE.
        //
    }
#endif

    //
    // Locate the Page Directory Entry which maps this virtual
    // address and check for accessibility and validity.
    //

    //
    // Check to see if the page table page (PDE entry) is valid.
    // If not, the page table page must be made valid first.
    //

    if (PointerPde->u.Hard.Valid == 0) {

        //
        // If the PDE is zero, check to see if there is a virtual address
        // mapped at this location, and if so create the necessary
        // structures to map it.
        //

        if ((PointerPde->u.Long == MM_ZERO_PTE) ||
            (PointerPde->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

#if (_MI_PAGING_LEVELS < 3)

                MiCheckPdeForPagedPool (VirtualAddress);

                if (PointerPde->u.Hard.Valid == 1) {
                    status = STATUS_SUCCESS;
                }

#endif

                if (status == STATUS_ACCESS_VIOLATION) {
                    MI_BREAK_ON_AV (VirtualAddress, 2);
                }

                goto ReturnStatus2;

            }

#if (_MI_PAGING_LEVELS >= 3)

            //
            // Increment the count of non-zero page directory entries for this
            // page directory.
            //

            if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#if (_MI_PAGING_LEVELS >= 4)
            else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {
                PVOID RealVa;

                RealVa = MiGetVirtualAddressMappedByPte(VirtualAddress);

                if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                    //
                    // This is really a page directory page.  Increment the
                    // use count on the appropriate page directory.
                    //

                    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
                }
            }
#endif
#endif
            //
            // Build a demand zero PDE and operate on it.
            //

            MI_WRITE_INVALID_PTE (PointerPde, DemandZeroPde);
        }

        //
        // The PDE is not valid, call the page fault routine passing
        // in the address of the PDE.  If the PDE is valid, determine
        // the status of the corresponding PTE.
        //

        status = MiDispatchFault (TRUE,  //page table page always written
                                  PointerPte,   //Virtual address
                                  PointerPde,   // PTE (PDE in this case)
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  TrapInformation,
                                  NULL);

        ASSERT (KeAreAllApcsDisabled () == TRUE);

#if (_MI_PAGING_LEVELS >= 4)

        //
        // Note that the page directory parent page itself could have been
        // paged out or deleted while MiDispatchFault was executing without
        // the working set lock, so this must be checked for here in the PXE.
        //

        if (PointerPxe->u.Hard.Valid == 0) {

            //
            // The PXE is not valid, return the status.
            //

            goto ReturnStatus1;
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)

        //
        // Note that the page directory page itself could have been paged out
        // or deleted while MiDispatchFault was executing without the working
        // set lock, so this must be checked for here in the PPE.
        //

        if (PointerPpe->u.Hard.Valid == 0) {

            //
            // The PPE is not valid, return the status.
            //

            goto ReturnStatus1;
        }
#endif

        if (PointerPde->u.Hard.Valid == 0) {

            //
            // The PDE is not valid, return the status.
            //

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPde, PointerPte, FALSE);

        //
        // Now that the PDE is accessible, get the PTE - let this fall
        // through.
        //
    }
    else if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

        if (MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) {
            if (PointerPde->u.Hard.Write == 0) {
                status = STATUS_ACCESS_VIOLATION;
                MI_BREAK_ON_AV (VirtualAddress, 8);
                goto ReturnStatus2;
            }
        }

        //
        // Ensure execute access is enabled in the PDE if it is large.
        //

        if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
            (!MI_IS_PTE_EXECUTABLE (PointerPde))) {

#if defined (_WIN64)
            if (CurrentProcess->Wow64Process != NULL)
#elif defined(_X86PAE_)
            if (MmPaeMask != 0)
#else
            if (FALSE)
#endif
            {

                //
                // If stack or data execution is globally enabled or is
                // enabled by the current process flags, or stack or data
                // execution is not globally disabled and is not disabled
                // by the current process, then allow execution.
                //

                if (((KeFeatureBits & KF_GLOBAL_32BIT_EXECUTE) != 0) ||
                    (CurrentProcess->Pcb.Flags.ExecuteEnable != 0) ||
                    (((KeFeatureBits & KF_GLOBAL_32BIT_NOEXECUTE) == 0) &&
                     (CurrentProcess->Pcb.Flags.ExecuteDisable == 0))) {

                    status = STATUS_SUCCESS;

                    TempPte = *PointerPde;
#if defined (_AMD64_)
                    TempPte.u.Hard.NoExecute = 0;
#elif defined(_X86PAE_)
                    TempPte.u.Long &= ~MmPaeMask;
#endif

                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPde, TempPte);
                    goto ReturnStatus1;
                }
            }

            status = STATUS_ACCESS_VIOLATION;
            MI_BREAK_ON_AV (VirtualAddress, 0xB);
            goto ReturnStatus2;
        }

        status = STATUS_SUCCESS;
        goto ReturnStatus1;
    }

    //
    // The PDE is valid and accessible, get the PTE contents.
    //

    TempPte = *PointerPte;
    if (TempPte.u.Hard.Valid != 0) {

        MMPTE NewPteContents;

#if defined(_X86PAE_)

        //
        // Re-read the PTE with an interlocked operation to prevent
        // AWE updates (these are done with interlocked and do not
        // acquire the working set pushlock) from tearing our read
        // on these platforms.  If the PTE has changed just bail and
        // re-run the faulting instruction.
        //

        MMPTE BogusPte;
        
        BogusPte.u.Long = (ULONG64)-1;

        NewPteContents.u.Long = InterlockedCompareExchangePte (PointerPte,
                                                               BogusPte.u.Long,
                                                               BogusPte.u.Long);

        ASSERT (NewPteContents.u.Long != BogusPte.u.Long);

        if (NewPteContents.u.Long != TempPte.u.Long) {
            goto ReturnStatus2;
        }

#else

        NewPteContents.u.Long = TempPte.u.Long;

#endif

        if (MI_PDE_MAPS_LARGE_PAGE (&TempPte)) {

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          8);
        }

        //
        // The PTE is valid and accessible : this is either a copy on write
        // fault or an accessed/modified bit that needs to be set.
        //

        status = STATUS_SUCCESS;

        if ((TempPte.u.Hard.Owner == MI_PTE_OWNER_KERNEL) &&
            (VirtualAddress <= MmHighestUserAddress)) {

            //
            // Note this is only a valid check for
            // real PTEs (not PDEs or above).
            //

            status = STATUS_ACCESS_VIOLATION;
            MI_BREAK_ON_AV (VirtualAddress, 0xC);
        }
        else if (MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) {

            //
            // This was a write operation.  If the copy on write
            // bit is set in the PTE perform the copy on write,
            // else check to ensure write access to the PTE.
            //

            if (TempPte.u.Hard.CopyOnWrite != 0) {

                MiCopyOnWrite (VirtualAddress, PointerPte);

                status = STATUS_PAGE_FAULT_COPY_ON_WRITE;
                goto ReturnStatus2;
            }

            if (TempPte.u.Hard.Write == 0) {
                status = STATUS_ACCESS_VIOLATION;
                MI_BREAK_ON_AV (VirtualAddress, 3);
            }
        }
        else if (MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) {

            if (!MI_IS_PTE_EXECUTABLE (&TempPte)) {

                status = STATUS_ACCESS_VIOLATION;

#if defined (_WIN64)
                if (CurrentProcess->Wow64Process != NULL)
#elif defined(_X86PAE_)
                if (MmPaeMask != 0)
#else
                if (FALSE)
#endif
                {

                    //
                    // If stack or data execution is globally enabled or is
                    // enabled by the current process flags, or stack or data
                    // execution is not globally disabled and is not disabled
                    // by the current process, then allow execution.
                    //

                    if (((KeFeatureBits & KF_GLOBAL_32BIT_EXECUTE) != 0) ||
                        (CurrentProcess->Pcb.Flags.ExecuteEnable != 0) ||
                        (((KeFeatureBits & KF_GLOBAL_32BIT_NOEXECUTE) == 0) &&
                         (CurrentProcess->Pcb.Flags.ExecuteDisable == 0))) {

                        status = STATUS_SUCCESS;

#if defined (_AMD64_)
                        NewPteContents.u.Hard.NoExecute = 0;
#elif defined(_X86PAE_)
                        NewPteContents.u.Long &= ~MmPaeMask;
#endif

                        //
                        // The PTE must be written carefully since we have not
                        // determined if it is a regular PTE or an AWE PTE.  The
                        // difference is that AWE PTEs are updated with interlocked
                        // operations and typically do not acquire the working
                        // set pushlock to synchronize.  Note that if the PTE
                        // update below fails it must be an AWE PTE that the
                        // user is changing simultaneously - so just bail and
                        // re-run the faulting instruction.
                        //
    
                        InterlockedCompareExchangePte (PointerPte,
                                                       NewPteContents.u.Long,
                                                       TempPte.u.Long);
    
                        status = STATUS_SUCCESS;
                        goto ReturnStatus2;
                    }
                }

                if (status == STATUS_ACCESS_VIOLATION) {
                    MI_BREAK_ON_AV (VirtualAddress, 4);
                }
            }
        }
        else {

            //
            // The PTE is valid and accessible, another thread must
            // have updated the PTE already, or the access/modified bits
            // are clear and need to be updated.
            //

            NOTHING;
        }

        if (status == STATUS_SUCCESS) {

            ASSERT (TempPte.u.Hard.Valid != 0);

            if (CurrentProcess->AweInfo != NULL) {

                //
                // This process potentially has AWE mappings.  These cannot
                // be updated for access and dirty bits using the standard
                // code below.  Since there is never a need to update them
                // for those bits anyway, if they are detected here, then
                // we are done.
                //

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
    
                if (MI_IS_PFN (PageFrameIndex)) {
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    if (Pfn1->u4.AweAllocation == 1) {
                        goto ReturnStatus2;
                    }
                }
            }
#if defined(_X86_) || defined(_AMD64_)
#if !defined(NT_UP)

            //
            // The access bit is set (and TB inserted) automatically by the
            // processor if the valid bit is set so there is no need to
            // set it here in software.
            //
            // The modified bit is also set (and TB inserted) automatically
            // by the processor if the valid & write (writable for MP) bits
            // are set.
            //
            // So to avoid acquiring the PFN lock here, don't do anything
            // to the access bit if this was just a read (let the hardware
            // do it).  If it was a write, update the PTE only and defer the
            // PFN update (which requires the PFN lock) till later.  The only
            // side effect of this is that if the page already has a valid
            // copy in the paging file, this space will not be reclaimed until
            // later.  Later == whenever we trim or delete the physical memory.
            // The implication of this is not as severe as it sounds - because
            // the paging file space is always committed anyway for the life
            // of the page; by not reclaiming the actual location right here
            // it just means we cannot defrag as tightly as possible.
            //

            if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                (TempPte.u.Hard.Dirty == 0)) {

                if (MiSetDirtyBit (VirtualAddress, PointerPte, FALSE) == FALSE) {
                    status = STATUS_ACCESS_VIOLATION;
                    MI_BREAK_ON_AV (VirtualAddress, 0xA);
                }
            }
#endif
#else
            LOCK_PFN (OldIrql);

            ASSERT (PointerPte->u.Hard.Valid != 0);
            ASSERT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) == MI_GET_PAGE_FRAME_FROM_PTE (&TempPte));

            MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
            UNLOCK_PFN (OldIrql);
#endif
        }

        goto ReturnStatus2;
    }

    //
    // If the PTE is zero, check to see if there is a virtual address
    // mapped at this location, and if so create the necessary
    // structures to map it.
    //

    //
    // Check explicitly for demand zero pages.
    //

    if (TempPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE) {
        MiResolveDemandZeroFault (VirtualAddress,
                                  PointerPte,
                                  CurrentProcess,
                                  MM_NOIRQL);

        status = STATUS_PAGE_FAULT_DEMAND_ZERO;
        goto ReturnStatus1;
    }

    RecheckAccess = FALSE;
    ProtoVad = NULL;

    if ((TempPte.u.Long == MM_ZERO_PTE) ||
        (TempPte.u.Long == MM_ZERO_KERNEL_PTE)) {

        //
        // PTE is needs to be evaluated with respect to its virtual
        // address descriptor (VAD).  At this point there are 3
        // possibilities, bogus address, demand zero, or refers to
        // a prototype PTE.
        //

        PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                 &ProtectionCode,
                                                 &ProtoVad);
        if (ProtectionCode == MM_NOACCESS) {
            status = STATUS_ACCESS_VIOLATION;

            //
            // Check to make sure this is not a page table page for
            // paged pool which needs extending.
            //

#if (_MI_PAGING_LEVELS < 3)
            MiCheckPdeForPagedPool (VirtualAddress);
#endif

            if (PointerPte->u.Hard.Valid == 1) {
                status = STATUS_SUCCESS;
            }

            if (status == STATUS_ACCESS_VIOLATION) {
                MI_BREAK_ON_AV (VirtualAddress, 5);
            }

            goto ReturnStatus2;
        }

        if (MI_IS_GUARD (ProtectionCode)) {

            //
            // If this is a user mode SLIST fault then don't remove the
            // guard bit.
            //

            if (KeInvalidAccessAllowed (TrapInformation)) {
                status = STATUS_ACCESS_VIOLATION;
                goto ReturnStatus2;
            }
        }

        //
        // Increment the count of non-zero page table entries for this
        // page table.
        //

        if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
        }
#if (_MI_PAGING_LEVELS >= 3)
        else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {
            PVOID RealVa;

            RealVa = MiGetVirtualAddressMappedByPte(VirtualAddress);

            if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                //
                // This is really a page table page.  Increment the use count
                // on the appropriate page directory.
                //

                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#if (_MI_PAGING_LEVELS >= 4)
            else {

                RealVa = MiGetVirtualAddressMappedByPde(VirtualAddress);

                if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                    //
                    // This is really a page directory page.  Increment the use
                    // count on the appropriate page directory parent.
                    //

                    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
                }
            }
#endif
        }
#endif

        //
        // Is this page a guard page?
        //

        if (MI_IS_GUARD (ProtectionCode)) {

            //
            // This is a guard page exception.
            //

            PointerPte->u.Soft.Protection = ProtectionCode & ~MM_GUARD_PAGE;

            if (PointerProtoPte != NULL) {

                //
                // This is a prototype PTE, build the PTE to not
                // be a guard page.
                //

                PointerPte->u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
                PointerPte->u.Soft.Prototype = 1;
            }

            if ((Thread->ApcNeeded == 1) && (Thread->ActiveFaultCount == 0)) {
                Thread->ApcNeeded = 0;
                ApcNeeded = TRUE;
            }

            UNLOCK_WS (Thread, CurrentProcess);
            ASSERT (KeGetCurrentIrql() == PreviousIrql);

            if (ApcNeeded == TRUE) {
                ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
                IoRetryIrpCompletions ();
            }

            return MiCheckForUserStackOverflow (VirtualAddress, TrapInformation);
        }

        if (PointerProtoPte == NULL) {

            //
            // Assert that this is not for a PDE.
            //

            if (PointerPde == MiGetPdeAddress((PVOID)PTE_BASE)) {

                //
                // This PTE is really a PDE, set contents as such.
                //

                MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPde);
            }
            else {
                PointerPte->u.Soft.Protection = ProtectionCode;
            }

            //
            // If a fork operation is in progress and the faulting thread
            // is not the thread performing the fork operation, block until
            // the fork is completed.
            //

            if (CurrentProcess->ForkInProgress != NULL) {
                if (MiWaitForForkToComplete (CurrentProcess) == TRUE) {
                    status = STATUS_SUCCESS;
                    goto ReturnStatus1;
                }
            }

            LOCK_PFN (OldIrql);

            if ((MmAvailablePages >= MM_HIGH_LIMIT) ||
                (!MiEnsureAvailablePageOrWait (CurrentProcess, OldIrql))) {

                ULONG Color;
                Color = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                                &CurrentProcess->NextPageColor);
                PageFrameIndex = MiRemoveZeroPageIfAny (Color);

                if (PageFrameIndex == 0) {

                    PMMPTE ZeroPte;
                    PVOID ZeroAddress;

                    PageFrameIndex = MiRemoveAnyPage (Color);

                    UNLOCK_PFN (OldIrql);

                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    ZeroPte = MiReserveSystemPtes (1, SystemPteSpace);
    
                    if (ZeroPte != NULL) {
    
                        TempPte = ValidKernelPte;
                        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    
                        if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                        }
                        else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
                            MI_DISABLE_CACHING (TempPte);
                        }

                        MI_WRITE_VALID_PTE (ZeroPte, TempPte);
    
                        ZeroAddress = MiGetVirtualAddressMappedByPte (ZeroPte);
    
                        KeZeroSinglePage (ZeroAddress);
    
                        MiReleaseSystemPtes (ZeroPte, 1, SystemPteSpace);
                    }
                    else {
                        MiZeroPhysicalPage (PageFrameIndex);
                    }

#if MI_BARRIER_SUPPORTED

                    //
                    // Note the stamping must occur after the page is zeroed.
                    //

                    MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
                    Pfn1->u4.PteFrame = BarrierStamp;
#endif

                    LOCK_PFN (OldIrql);
                }
                else {
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                }

                //
                // This barrier check is needed after zeroing the page and
                // before setting the PTE valid.
                // Capture it now, check it at the last possible moment.
                //

                BarrierStamp = (ULONG)Pfn1->u4.PteFrame;

                MiInitializePfn (PageFrameIndex, PointerPte, 1);

                UNLOCK_PFN (OldIrql);

                CurrentProcess->NumberOfPrivatePages += 1;

                InterlockedIncrement (&KeGetCurrentPrcb()->MmDemandZeroCount);

                //
                // As this page is demand zero, set the modified bit in the
                // PFN database element and set the dirty bit in the PTE.
                //

                if (PointerPte <= MiHighestUserPte) {
                    MI_MAKE_VALID_USER_PTE (TempPte,
                                            PageFrameIndex,
                                            PointerPte->u.Soft.Protection,
                                            PointerPte);
                }
                else {

                    //
                    // Must be a usermode page directory (or higher) page -
                    // note in PAE mode, these cannot have the global bit
                    // set so need to work out another fast macro for these.
                    // This case should be relatively infrequent though.
                    //

                    MI_MAKE_VALID_PTE (TempPte,
                                       PageFrameIndex,
                                       PointerPte->u.Soft.Protection,
                                       PointerPte);
                }

                if (TempPte.u.Hard.Write != 0) {
                    MI_SET_PTE_DIRTY (TempPte);
                }

                MI_BARRIER_SYNCHRONIZE (BarrierStamp);

                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                ASSERT (Pfn1->u1.Event == NULL);

                WorkingSetIndex = MiAllocateWsle (&CurrentProcess->Vm,
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

                    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

                    MiTrimPte (VirtualAddress,
                               PointerPte,
                               Pfn1,
                               CurrentProcess,
                               ZeroPte);
                }
            }
            else {
                UNLOCK_PFN (OldIrql);
            }

            status = STATUS_PAGE_FAULT_DEMAND_ZERO;
            goto ReturnStatus1;
        }

        //
        // This is a prototype PTE.
        //

        if (ProtectionCode == MM_UNKNOWN_PROTECTION) {

            //
            // The protection field is stored in the prototype PTE.
            //

            TempPte.u.Long = MiProtoAddressForPte (PointerProtoPte);
        }
        else {
            TempPte = PrototypePte;
            TempPte.u.Soft.Protection = ProtectionCode;
        }
        MI_WRITE_INVALID_PTE (PointerPte, TempPte);
    }
    else {

        //
        // The PTE is non-zero and not valid, see if it is a prototype PTE.
        //

        ProtectionCode = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);

        if (TempPte.u.Soft.Prototype != 0) {
            if (TempPte.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {
#if DBG
                MmProtoPteVadLookups += 1;
#endif
                PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                         &ProtectCode,
                                                         &ProtoVad);

                if (PointerProtoPte == NULL) {
                    status = STATUS_ACCESS_VIOLATION;
                    MI_BREAK_ON_AV (VirtualAddress, 6);
                    goto ReturnStatus1;
                }
            }
            else {
#if DBG
                MmProtoPteDirect += 1;
#endif

                //
                // Protection is in the prototype PTE, indicate an
                // access check should not be performed on the current PTE.
                //

                PointerProtoPte = MiPteToProto (&TempPte);

                //
                // Check to see if the proto protection has been overridden.
                //

                if (TempPte.u.Proto.ReadOnly != 0) {
                    ProtectionCode = MM_READONLY;
                }
                else {
                    ProtectionCode = MM_UNKNOWN_PROTECTION;
                    if (CurrentProcess->CloneRoot != NULL) {
                        RecheckAccess = TRUE;
                    }
                }
            }
        }
    }

    if (ProtectionCode != MM_UNKNOWN_PROTECTION) {

        status = MiAccessCheck (PointerPte,
                                MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                                PreviousMode,
                                ProtectionCode,
                                TrapInformation,
                                FALSE);

        if (status != STATUS_SUCCESS) {

            if (status == STATUS_ACCESS_VIOLATION) {
                MI_BREAK_ON_AV (VirtualAddress, 7);
            }

            if ((Thread->ApcNeeded == 1) && (Thread->ActiveFaultCount == 0)) {
                Thread->ApcNeeded = 0;
                ApcNeeded = TRUE;
            }

            UNLOCK_WS (Thread, CurrentProcess);
            ASSERT (KeGetCurrentIrql() == PreviousIrql);

            if (ApcNeeded == TRUE) {
                ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
                IoRetryIrpCompletions ();
            }

            //
            // Check to see if this is a guard page violation
            // and if so, should the user's stack be extended.
            //

            if (status == STATUS_GUARD_PAGE_VIOLATION) {
                return MiCheckForUserStackOverflow (VirtualAddress, TrapInformation);
            }

            return status;
        }
    }

    //
    // This is a page fault, invoke the page fault handler.
    //

    status = MiDispatchFault (FaultStatus,
                              VirtualAddress,
                              PointerPte,
                              PointerProtoPte,
                              RecheckAccess,
                              CurrentProcess,
                              TrapInformation,
                              ProtoVad);

ReturnStatus1:

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
    if (CurrentProcess->Vm.Flags.GrowWsleHash == 1) {
        MiGrowWsleHash (&CurrentProcess->Vm);
    }

ReturnStatus2:

    PageFrameIndex = CurrentProcess->Vm.WorkingSetSize - CurrentProcess->Vm.MinimumWorkingSetSize;

    if ((Thread->ApcNeeded == 1) && (Thread->ActiveFaultCount == 0)) {
        Thread->ApcNeeded = 0;
        ApcNeeded = TRUE;
    }

    UNLOCK_WS (Thread, CurrentProcess);
    ASSERT (KeGetCurrentIrql() == PreviousIrql);

    if (ApcNeeded == TRUE) {
        ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
        IoRetryIrpCompletions ();
    }

    if (MmAvailablePages < MM_PLENTY_FREE_LIMIT) {

        if (((SPFN_NUMBER)PageFrameIndex > 100) &&
            (KeGetCurrentThread()->Priority >= LOW_REALTIME_PRIORITY)) {

            //
            // This thread is realtime and is well over the process'
            // working set minimum.  Delay execution so the trimmer & the
            // modified page writer get a quick shot at making pages.
            //

            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        }
    }

ReturnStatus3:

    //
    // Stop high priority threads from consuming the CPU on collided
    // faults for pages that are still marked with inpage errors.  All
    // the threads must let go of the page so it can be freed and the
    // inpage I/O reissued to the filesystem.  Issuing this delay after
    // releasing the working set mutex also makes this process eligible
    // for trimming if its resources are needed.
    //


    if (status != STATUS_SUCCESS) {
        if (!NT_SUCCESS (status)) {
            if (MmIsRetryIoStatus(status)) {
                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&MmShortTime);
                status = STATUS_SUCCESS;
            }
        }

        if (status != STATUS_SUCCESS) {
            NotifyRoutine = MmPageFaultNotifyRoutine;
            if (NotifyRoutine) {
                (*NotifyRoutine) (status, VirtualAddress, TrapInformation);
            }
        }
    }

    return status;

KernelAccessViolation:

    UNLOCK_WORKING_SET (Thread, Ws);

    ASSERT (WsIrql != MM_NOIRQL);
    KeLowerIrql (WsIrql);
    return STATUS_ACCESS_VIOLATION;
}

#if DBG

extern PVOID PsNtosImageEnd;
ULONG MmInpageFraction = 0x1F;      // Fail 1 out of every 32 inpages.

#define MI_INPAGE_BACKTRACE_LENGTH 6

typedef struct _MI_INPAGE_TRACES {

    PVOID FaultingAddress;
    PETHREAD Thread;
    PVOID StackTrace [MI_INPAGE_BACKTRACE_LENGTH];

} MI_INPAGE_TRACES, *PMI_INPAGE_TRACES;

#define MI_INPAGE_TRACE_SIZE 64

LONG MiInpageIndex;

MI_INPAGE_TRACES MiInpageTraces[MI_INPAGE_TRACE_SIZE];

VOID
FORCEINLINE
MiSnapInPageError (
    IN PVOID FaultingAddress
    )
{
    PMI_INPAGE_TRACES Information;
    ULONG Index;
    ULONG Hash;

    Index = InterlockedIncrement(&MiInpageIndex);
    Index &= (MI_INPAGE_TRACE_SIZE - 1);
    Information = &MiInpageTraces[Index];

    Information->FaultingAddress = FaultingAddress;
    Information->Thread = PsGetCurrentThread ();

    RtlZeroMemory (&Information->StackTrace[0], MI_INPAGE_BACKTRACE_LENGTH * sizeof(PVOID)); 

    RtlCaptureStackBackTrace (1, MI_INPAGE_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

LOGICAL
MiInPageAllowed (
    IN PVOID VirtualAddress,
    IN PVOID TrapInformation
    )
{
    LARGE_INTEGER CurrentTime;
    ULONG_PTR InstructionPointer;

    KeQueryTickCount (&CurrentTime);

    if ((CurrentTime.LowPart & MmInpageFraction) == 0) {

        if (MmInjectUserInpageErrors & 0x1) {
            MiSnapInPageError (VirtualAddress);
            return FALSE;
        }

#if defined(_X86_)
        InstructionPointer = ((PKTRAP_FRAME)TrapInformation)->Eip;
#elif defined(_AMD64_)
        InstructionPointer = ((PKTRAP_FRAME)TrapInformation)->Rip;
#else
        error
#endif

        if ((InstructionPointer >= (ULONG_PTR) PsNtosImageBase) &&
            (InstructionPointer < (ULONG_PTR) PsNtosImageEnd)) {

            MiSnapInPageError (VirtualAddress);
            return FALSE;
        }
    }

    return TRUE;
}
#endif

NTSTATUS
MmGetExecuteOptions (
    IN PULONG ExecuteOptions
    )

/*++

Routine Description:

    This function queries the current process execution options that control
    NX support and thunk emulation.

Arguments:

    ExecuteOptions - Supplies a pointer to a variable that receives the
        current process execute options.

Return Value:

    If the current process is a WIN64 process, then a failure status is
    returned. Otherwise, a success status is returned.
    
--*/

{

    PEPROCESS CurrentProcess;
    KEXECUTE_OPTIONS Options;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    //
    // Get the current process address.
    //

    CurrentProcess = PsGetCurrentProcess ();

#if defined (_WIN64)

    if (CurrentProcess->Wow64Process == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

#endif

    //
    // Query the current process execution options.
    //

    *((PUCHAR)&Options) = CurrentProcess->Pcb.ExecuteOptions;
    *ExecuteOptions = 0;
    if (Options.ExecuteDisable != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_DISABLE;
    }

    if (Options.ExecuteEnable != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_ENABLE;
    }

    if (Options.DisableThunkEmulation != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION;
    }

    if (Options.Permanent != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_PERMANENT;
    }

    if (Options.ExecuteDispatchEnable != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_EXECUTE_DISPATCH_ENABLE;
    }

    if (Options.ImageDispatchEnable != 0) {
        *ExecuteOptions |= MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE;
    }

    return STATUS_SUCCESS;
}

DECLSPEC_NOINLINE
NTSTATUS
MmSetExecuteOptions (
    IN ULONG ExecuteOptions
    )

/*++

Routine Description:

    This function sets the current process execution options that control NX
    support and thunk emulation.

Arguments:

    ExecuteOptions - Supplies the process execution options.

Return Value:

    If the current process is a WIN64 process, a invalid set of execute
    options is specified, or the permanent execute options bit is set,
    then a failure status is returned. Otherwise, a success status is
    returned.

--*/

{

    PEPROCESS CurrentProcess;
    KLOCK_QUEUE_HANDLE LockHandle;
    NTSTATUS Status;

    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);

    //
    // Make sure reserved bits are clear.
    //

    if ((ExecuteOptions & ~(MEM_EXECUTE_OPTION_VALID_FLAGS)) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Get the current process address.
    //

    CurrentProcess = PsGetCurrentProcess ();

#if defined (_WIN64)

    if (CurrentProcess->Wow64Process == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

#endif

    //
    // Raise IRQL to SYNCH_LEVEL and acquire the process lock.
    //
    // If the permanent flag is set in the current process, then the process
    // execute options cannot be set. Otherwise, set the process execution
    // options.
    //

    Status = STATUS_ACCESS_DENIED;
    KeAcquireInStackQueuedSpinLockRaiseToSynch (&CurrentProcess->Pcb.ProcessLock,
                                                &LockHandle);

    if (CurrentProcess->Pcb.Flags.Permanent == 0) {
        CurrentProcess->Pcb.Flags.ExecuteDisable = 0;
        if ((ExecuteOptions & MEM_EXECUTE_OPTION_DISABLE) != 0) {
            CurrentProcess->Pcb.Flags.ExecuteDisable = 1;
        }
    
        if ((ExecuteOptions & MEM_EXECUTE_OPTION_ENABLE) != 0) {
            CurrentProcess->Pcb.Flags.ExecuteEnable = 1;
        }

        if ((ExecuteOptions & MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION) != 0) {
            CurrentProcess->Pcb.Flags.DisableThunkEmulation = 1;
        }

        if ((ExecuteOptions & MEM_EXECUTE_OPTION_PERMANENT) != 0) {
            CurrentProcess->Pcb.Flags.Permanent = 1;
        }

        if ((ExecuteOptions & MEM_EXECUTE_OPTION_EXECUTE_DISPATCH_ENABLE) != 0) {
            CurrentProcess->Pcb.Flags.ExecuteDispatchEnable = 1;
        }

        if ((ExecuteOptions & MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE) != 0) {
            CurrentProcess->Pcb.Flags.ImageDispatchEnable = 1;
        }

        if (CurrentProcess->Pcb.Flags.ExecuteEnable != 0) {
            CurrentProcess->Pcb.Flags.ExecuteDispatchEnable = 1;
            CurrentProcess->Pcb.Flags.ImageDispatchEnable = 1;
        }

        Status = STATUS_SUCCESS;
    }

    KeReleaseInStackQueuedSpinLock (&LockHandle);
    return Status;
}

VOID
MmGetImageInformation (
    OUT PSECTION_IMAGE_INFORMATION ImageInformation
    )

/*++

Routine Description:

    This function returns the section image information for the current
    process.

Arguments:

    SectionObject - Supplies a pointer to a section object.

    ImageInformation - Supplies a pointer to a the data area that will receive
        the image information.

Return Value:

    None.

--*/

{

    PEPROCESS Process;
    PSECTION Section;

    //
    // Set section image information.
    //

    Process = PsGetCurrentProcess ();
    Section = Process->SectionObject;

    ASSERT(Section != NULL);

    *ImageInformation = *Section->Segment->u2.ImageInformation;
    return;
}

