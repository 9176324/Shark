/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   queryvm.c

Abstract:

    This module contains the routines which implement the
    NtQueryVirtualMemory service.

--*/

#include "mi.h"

extern POBJECT_TYPE IoFileObjectType;

NTSTATUS
MiGetWorkingSetInfo (
    IN PMEMORY_WORKING_SET_INFORMATION WorkingSetInfo,
    IN SIZE_T Length,
    IN PEPROCESS Process
    );

NTSTATUS
MiGetWorkingSetInfoList (
    __in PMEMORY_WORKING_SET_EX_INFORMATION WorkingSetInfo,
    __in SIZE_T Length,
    __in PEPROCESS Process
    );

NTSTATUS
NtQueryVirtualMemory(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in MEMORY_INFORMATION_CLASS MemoryInformationClass,
    __out_bcount(MemoryInformationLength) PVOID MemoryInformation,
    __in SIZE_T MemoryInformationLength,
    __out_opt PSIZE_T ReturnLength
    )

/*++

Routine Description:

    This function provides the capability to determine the state,
    protection, and type of a region of pages within the virtual address
    space of the subject process.

    The state of the first page within the region is determined and then
    subsequent entries in the process address map are scanned from the
    base address upward until either the entire range of pages has been
    scanned or until a page with a nonmatching set of attributes is
    encountered. The region attributes, the length of the region of pages
    with matching attributes, and an appropriate status value are
    returned.

    If the entire region of pages does not have a matching set of
    attributes, then the returned length parameter value can be used to
    calculate the address and length of the region of pages that was not
    scanned.

Arguments:


    ProcessHandle - An open handle to a process object.

    BaseAddress - The base address of the region of pages to be
                  queried. This value is rounded down to the next host-page-
                  address boundary.

    MemoryInformationClass - The memory information class about which
                             to retrieve information.

    MemoryInformation - A pointer to a buffer that receives the specified
                        information.  The format and content of the buffer
                        depend on the specified information class.


        MemoryBasicInformation - Data type is PMEMORY_BASIC_INFORMATION.

            MEMORY_BASIC_INFORMATION Structure


            ULONG RegionSize - The size of the region in bytes beginning at
                               the base address in which all pages have
                               identical attributes.

            ULONG State - The state of the pages within the region.

                State Values

                MEM_COMMIT - The state of the pages within the region
                             is committed.

                MEM_FREE - The state of the pages within the region
                           is free.

                MEM_RESERVE - The state of the pages within the
                              region is reserved.

            ULONG Protect - The protection of the pages within the region.

                Protect Values

                PAGE_NOACCESS - No access to the region of pages is allowed.
                                An attempt to read, write, or execute within
                                the region results in an access violation.

                PAGE_EXECUTE - Execute access to the region of pages
                               is allowed. An attempt to read or write within
                               the region results in an access violation.

                PAGE_READONLY - Read-only and execute access to the region
                                of pages is allowed. An attempt to write within
                                the region results in an access violation.

                PAGE_READWRITE - Read, write, and execute access to the region
                                 of pages is allowed. If write access to the
                                 underlying section is allowed, then a single
                                 copy of the pages are shared. Otherwise,
                                 the pages are shared read-only/copy-on-write.

                PAGE_GUARD - Read, write, and execute access to the
                             region of pages is allowed; however, access to
                             the region causes a "guard region entered"
                             condition to be raised in the subject process.

                PAGE_NOCACHE - Disable the placement of committed
                               pages into the data cache.

                PAGE_WRITECOMBINE - Disable the placement of committed
                                    pages into the data cache, combine the
                                    writes as well.

            ULONG Type - The type of pages within the region.

                Type Values

                MEM_PRIVATE - The pages within the region are private.

                MEM_MAPPED - The pages within the region are mapped
                             into the view of a section.

                MEM_IMAGE - The pages within the region are mapped
                            into the view of an image section.

    MemoryInformationLength - Specifies the length in bytes of
                              the memory information buffer.

    ReturnLength - An optional pointer which, if specified, receives the
                   number of bytes placed in the process information buffer.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode.

--*/

{
    ULONG LocalReturnLength;
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS TargetProcess;
    PETHREAD Thread;
    NTSTATUS Status;
    PMMVAD Vad;
    PVOID Va;
    PVOID NextVaToQuery;
    LOGICAL Found;
    SIZE_T TheRegionSize;
    ULONG NewProtect;
    ULONG NewState;
    PVOID FilePointer;
    ULONG_PTR BaseVpn;
    MEMORY_BASIC_INFORMATION Info;
    PMEMORY_BASIC_INFORMATION BasicInfo;
    LOGICAL Attached;
    LOGICAL Leaped;
    ULONG MemoryInformationLengthUlong;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    PVOID HighestVadAddress;
    PVOID HighestUserAddress;

    Found = FALSE;
    Leaped = TRUE;
    FilePointer = NULL;

    //
    // Make sure the user's buffer is large enough for the requested operation.
    //
    // Check argument validity.
    //

    switch (MemoryInformationClass) {
        case MemoryBasicInformation:
            if (MemoryInformationLength < sizeof(MEMORY_BASIC_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case MemoryWorkingSetInformation:
            if (MemoryInformationLength < sizeof(ULONG_PTR)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case MemoryWorkingSetExInformation:
            if (MemoryInformationLength < sizeof (MEMORY_WORKING_SET_EX_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case MemoryMappedFilenameInformation:
            break;

        default:
            return STATUS_INVALID_INFO_CLASS;
    }

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

        //
        // Check arguments.
        //

        try {

            ProbeForWrite(MemoryInformation,
                          MemoryInformationLength,
                          sizeof(ULONG_PTR));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong_ptr(ReturnLength);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // If an exception occurs during the probe or capture
            // of the initial values, then handle the exception and
            // return the exception code as the status value.
            //

            return GetExceptionCode();
        }
    }

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER;
    }

    HighestUserAddress = MM_HIGHEST_USER_ADDRESS;
    HighestVadAddress  = (PCHAR) MM_HIGHEST_VAD_ADDRESS;

#if defined(_WIN64)

    if (ProcessHandle == NtCurrentProcess()) {
        TargetProcess = PsGetCurrentProcessByThread(CurrentThread);
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&TargetProcess,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    //
    // If this is a wow64 process, then return the appropriate highest
    // user address depending on whether the process has been started with
    // a 2GB or a 4GB address space.
    //

    if (TargetProcess->Wow64Process != NULL) {

        if (TargetProcess->Flags & PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE) {
            HighestUserAddress = (PVOID) ((ULONG_PTR)_4gb - X64K - 1);
        }
        else {
            HighestUserAddress = (PVOID) ((ULONG_PTR)_2gb - X64K - 1);
        }

        HighestVadAddress  = (PCHAR)HighestUserAddress - X64K;

        if (BaseAddress > HighestUserAddress) {

            if (ProcessHandle != NtCurrentProcess()) {
                ObDereferenceObject (TargetProcess);
            }
            return STATUS_INVALID_PARAMETER;
        }
    }

#endif

    if ((BaseAddress > HighestVadAddress) ||
        (PAGE_ALIGN(BaseAddress) == (PVOID)MM_SHARED_USER_DATA_VA)) {

        //
        // Indicate a reserved area from this point on.
        //

        Status = STATUS_INVALID_ADDRESS;

        if (MemoryInformationClass == MemoryBasicInformation) {

            try {
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationBase =
                                      (PCHAR) HighestVadAddress + 1;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationProtect =
                                                                      PAGE_READONLY;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->BaseAddress =
                                                       PAGE_ALIGN(BaseAddress);
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->RegionSize =
                                    ((PCHAR)HighestUserAddress + 1) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->State = MEM_RESERVE;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Protect = PAGE_NOACCESS;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Type = MEM_PRIVATE;

                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
                }

                if (PAGE_ALIGN(BaseAddress) == (PVOID)MM_SHARED_USER_DATA_VA) {

                    //
                    // This is the page that is double mapped between
                    // user mode and kernel mode.
                    //

                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationBase =
                                (PVOID)MM_SHARED_USER_DATA_VA;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Protect =
                                                                 PAGE_READONLY;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->RegionSize =
                                                                 PAGE_SIZE;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->State =
                                                                 MEM_COMMIT;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // Just return success.
                //

                NOTHING;
            }

            Status = STATUS_SUCCESS;
        }
            
#if defined(_WIN64)
        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (TargetProcess);
        }
#endif
            
        return Status;
    }

#if !defined(_WIN64)

    if (ProcessHandle == NtCurrentProcess()) {
        TargetProcess = PsGetCurrentProcessByThread(CurrentThread);
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&TargetProcess,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

#endif

    if (MemoryInformationClass == MemoryWorkingSetExInformation) {

        Status = MiGetWorkingSetInfoList (
                            (PMEMORY_WORKING_SET_EX_INFORMATION) MemoryInformation,
                            MemoryInformationLength,
                            TargetProcess);

        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (TargetProcess);
        }

        //
        // If MiGetWorkingSetInfoList failed then inform the caller.
        //

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        try {

            if (ARGUMENT_PRESENT (ReturnLength)) {
                *ReturnLength = MemoryInformationLength;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        return STATUS_SUCCESS;
    }

    if (MemoryInformationClass == MemoryWorkingSetInformation) {

        Status = MiGetWorkingSetInfo (
                            (PMEMORY_WORKING_SET_INFORMATION) MemoryInformation,
                            MemoryInformationLength,
                            TargetProcess);

        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (TargetProcess);
        }

        //
        // If MiGetWorkingSetInfo failed then inform the caller.
        //

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        try {

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = ((((PMEMORY_WORKING_SET_INFORMATION)
                                    MemoryInformation)->NumberOfEntries - 1) *
                                        sizeof(ULONG_PTR)) +
                                        sizeof(MEMORY_WORKING_SET_INFORMATION);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return STATUS_SUCCESS;
    }

    //
    // If the specified process is not the current process, attach
    // to the specified process.
    //

    if (ProcessHandle != NtCurrentProcess()) {
        KeStackAttachProcess (&TargetProcess->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

    //
    // Get working set mutex and block APCs.
    //

    LOCK_ADDRESS_SPACE (TargetProcess);

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (TargetProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        UNLOCK_ADDRESS_SPACE (TargetProcess);
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
            ObDereferenceObject (TargetProcess);
        }
        return STATUS_PROCESS_IS_TERMINATING;
    }

    //
    // Locate the VAD that contains the base address or the VAD
    // which follows the base address.
    //

    if (TargetProcess->VadRoot.NumberGenericTableElements != 0) {

        Vad = (PMMVAD) TargetProcess->VadRoot.BalancedRoot.RightChild;
        BaseVpn = MI_VA_TO_VPN (BaseAddress);

        while (TRUE) {

            if (Vad == NULL) {
                break;
            }

            if ((BaseVpn >= Vad->StartingVpn) &&
                (BaseVpn <= Vad->EndingVpn)) {
                Found = TRUE;
                break;
            }

            if (BaseVpn < Vad->StartingVpn) {
                if (Vad->LeftChild == NULL) {
                    break;
                }
                Vad = Vad->LeftChild;
            }
            else {
                ASSERT (BaseVpn > Vad->EndingVpn);

                if (Vad->RightChild == NULL) {
                    break;
                }

                Vad = Vad->RightChild;
            }
        }
    }
    else {
        Vad = NULL;
        BaseVpn = 0;
    }

    if (!Found) {

        //
        // There is no virtual address allocated at the base
        // address.  Return the size of the hole starting at
        // the base address.
        //

        if (Vad == NULL) {
            TheRegionSize = (((PCHAR)HighestVadAddress + 1) - 
                                         (PCHAR)PAGE_ALIGN(BaseAddress));
        }
        else {
            if (Vad->StartingVpn < BaseVpn) {

                //
                // We are looking at the Vad which occupies the range
                // just before the desired range.  Get the next Vad.
                //

                Vad = MiGetNextVad (Vad);
                if (Vad == NULL) {
                    TheRegionSize = (((PCHAR)HighestVadAddress + 1) - 
                                                (PCHAR)PAGE_ALIGN(BaseAddress));
                }
                else {
                    TheRegionSize = (PCHAR)MI_VPN_TO_VA (Vad->StartingVpn) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
                }
            }
            else {
                TheRegionSize = (PCHAR)MI_VPN_TO_VA (Vad->StartingVpn) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
            }
        }

        UNLOCK_ADDRESS_SPACE (TargetProcess);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
            ObDereferenceObject (TargetProcess);
        }

        //
        // Establish an exception handler and write the information and
        // returned length.
        //

        if (MemoryInformationClass == MemoryBasicInformation) {
            BasicInfo = (PMEMORY_BASIC_INFORMATION) MemoryInformation;
            Found = FALSE;
            try {

                BasicInfo->AllocationBase = NULL;
                BasicInfo->AllocationProtect = 0;
                BasicInfo->BaseAddress = PAGE_ALIGN(BaseAddress);
                BasicInfo->RegionSize = TheRegionSize;
                BasicInfo->State = MEM_FREE;
                BasicInfo->Protect = PAGE_NOACCESS;
                BasicInfo->Type = 0;

                Found = TRUE;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {

                //
                // Just return success if the BasicInfo was successfully
                // filled in.
                //
                
                if (Found == FALSE) {
                    return GetExceptionCode ();
                }
            }

            return STATUS_SUCCESS;
        }
        return STATUS_INVALID_ADDRESS;
    }

    //
    // Found a VAD.
    //
   
    Va = PAGE_ALIGN(BaseAddress);
    Info.BaseAddress = Va;
    Info.AllocationBase = MI_VPN_TO_VA (Vad->StartingVpn);
    Info.AllocationProtect = MI_CONVERT_FROM_PTE_PROTECTION (
                                             Vad->u.VadFlags.Protection);
    
    //
    // There is a page mapped at the base address.
    //
    
    if ((Vad->u.VadFlags.PrivateMemory) || 
        (Vad->u.VadFlags.VadType == VadRotatePhysical)) {

        Info.Type = MEM_PRIVATE;
    }
    else {
        if (Vad->u.VadFlags.VadType == VadImageMap) {
            Info.Type = MEM_IMAGE;
        }
        else {
            Info.Type = MEM_MAPPED;
        }

        if (MemoryInformationClass == MemoryMappedFilenameInformation) {

            if (Vad->ControlArea != NULL) {
                FilePointer = Vad->ControlArea->FilePointer;
            }
            if (FilePointer == NULL) {
                FilePointer = (PVOID)1;
            }
            else {
                ObReferenceObject (FilePointer);
            }
        } 
    }

    Thread = PsGetCurrentThread ();

    LOCK_WS_SHARED (Thread, TargetProcess);

    Info.State = MiQueryAddressState (Va,
                                      Vad,
                                      TargetProcess,
                                      &Info.Protect,
                                      &NextVaToQuery);

    Va = NextVaToQuery;

    while (MI_VA_TO_VPN (Va) <= Vad->EndingVpn) {

        NewState = MiQueryAddressState (Va,
                                        Vad,
                                        TargetProcess,
                                        &NewProtect,
                                        &NextVaToQuery);

        if ((NewState != Info.State) || (NewProtect != Info.Protect)) {

            //
            // The state for this address does not match, calculate
            // size and return.
            //

            Leaped = FALSE;
            break;
        }
        Va = NextVaToQuery;
    }

    UNLOCK_WS_SHARED (Thread, TargetProcess);    

    //
    // We may have aggressively leaped past the end of the VAD.  Shorten the
    // Va here if we did.
    //

    if (Leaped == TRUE) {
        Va = MI_VPN_TO_VA (Vad->EndingVpn + 1);
    }

    Info.RegionSize = ((PCHAR)Va - (PCHAR)Info.BaseAddress);

    //
    // A range has been found, release the mutexes, detach from the
    // target process and return the information.
    //

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
        ObDereferenceObject (TargetProcess);
    }

    if (MemoryInformationClass == MemoryBasicInformation) {
        Found = FALSE;
        try {

            *(PMEMORY_BASIC_INFORMATION)MemoryInformation = Info;

            Found = TRUE;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // Just return success if the BasicInfo was successfully
            // filled in.
            //
                
            if (Found == FALSE) {
                return GetExceptionCode ();
            }
        }
        return STATUS_SUCCESS;
    }

    //
    // Try to return the name of the file that is mapped.
    //

    if (FilePointer == NULL) {
        return STATUS_INVALID_ADDRESS;
    }

    if (FilePointer == (PVOID)1) {
        return STATUS_FILE_INVALID;
    }

    MemoryInformationLengthUlong = (ULONG)MemoryInformationLength;

    if ((SIZE_T)MemoryInformationLengthUlong < MemoryInformationLength) {
        return STATUS_INVALID_PARAMETER_5;
    }
    
    //
    // We have a referenced pointer to the file.  Call ObQueryNameString
    // and get the file name.
    //

    Status = ObQueryNameString (FilePointer,
                                (POBJECT_NAME_INFORMATION) MemoryInformation,
                                 MemoryInformationLengthUlong,
                                 &LocalReturnLength);

    ObDereferenceObject (FilePointer);

    if (ARGUMENT_PRESENT (ReturnLength)) {
        try {
            *ReturnLength = LocalReturnLength;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
        }
    }

    return Status;
}


ULONG
MiQueryAddressState (
    IN PVOID Va,
    IN PMMVAD Vad,
    IN PEPROCESS TargetProcess,
    OUT PULONG ReturnedProtect,
    OUT PVOID *NextVaToQuery
    )

/*++

Routine Description:


Arguments:

Return Value:

    Returns the state (MEM_COMMIT, MEM_RESERVE, MEM_PRIVATE).

Environment:

    Kernel mode.  Address creation mutex held (exclusive) and working
    set pushlock held (shared).

    This routine may release and reacquire the working set pushlock, callers
    must be prepared to handle this.

--*/

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PETHREAD Thread;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    MMPTE PteContents;
    MMPTE CapturedProtoPte;
    PMMPTE ProtoPte;
    LOGICAL PteIsZero;
    ULONG State;
    ULONG Protect;
    ULONG Waited;
    LOGICAL PteDetected;
    PVOID NextVa;
    PFN_NUMBER PageFrameIndex;

    State = MEM_RESERVE;
    Protect = 0;

    PointerPxe = MiGetPxeAddress (Va);
    PointerPpe = MiGetPpeAddress (Va);
    PointerPde = MiGetPdeAddress (Va);
    PointerPte = MiGetPteAddress (Va);

    ASSERT ((Vad->StartingVpn <= MI_VA_TO_VPN (Va)) &&
            (Vad->EndingVpn >= MI_VA_TO_VPN (Va)));

    PteIsZero = TRUE;
    PteDetected = FALSE;

    *NextVaToQuery = (PVOID)((PCHAR)Va + PAGE_SIZE);

    do {

        if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {

#if (_MI_PAGING_LEVELS >= 4)
            NextVa = MiGetVirtualAddressMappedByPte (PointerPxe + 1);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
#endif
            break;
        }
    
#if (_MI_PAGING_LEVELS >= 4)
        Waited = 0;
#endif

        if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {
#if (_MI_PAGING_LEVELS >= 3)
            NextVa = MiGetVirtualAddressMappedByPte (PointerPpe + 1);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
#endif
            break;
        }
    
#if (_MI_PAGING_LEVELS < 4)
        Waited = 0;
#endif

        if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {
            NextVa = MiGetVirtualAddressMappedByPte (PointerPde + 1);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
            break;
        }

        if (Waited == 0) {
            PteDetected = TRUE;
        }

    } while (Waited != 0);

    if (PteDetected == TRUE) {

        //
        // A PTE exists at this address, see if it is zero.
        //

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            *ReturnedProtect = MI_CONVERT_FROM_PTE_PROTECTION (Vad->u.VadFlags.Protection);
            NextVa = MiGetVirtualAddressMappedByPte (PointerPde + 1);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
            return MEM_COMMIT;
        }

        PteContents = *PointerPte;

        if (PteContents.u.Long != 0) {

            PteIsZero = FALSE;

            //
            // There is a non-zero PTE at this address, use
            // it to build the information block.
            //

            if (MiIsPteDecommittedPage (&PteContents)) {
                ASSERT (Protect == 0);
                ASSERT (State == MEM_RESERVE);
            }
            else {
                State = MEM_COMMIT;
                if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

                    //
                    // Physical mapping, there is no corresponding
                    // PFN element to get the page protection from.
                    //

                    Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                             Vad->u.VadFlags.Protection);
                }
                else if ((Vad->u.VadFlags.VadType == VadRotatePhysical) &&
                         (PteContents.u.Hard.Valid)) {

                    ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (Va));

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    if ((!MI_IS_PFN (PageFrameIndex)) ||
                        (Pfn1->PteAddress != ProtoPte)) {

                        //
                        // This address in the view is currently pointing at
                        // the frame buffer (video RAM or AGP), protection is
                        // in the prototype (which may itself be paged out - so
                        // release the process working set pushlock before
                        // accessing it).
                        //

                        Thread = PsGetCurrentThread ();
        
                        UNLOCK_WS_SHARED (Thread, TargetProcess);

                        CapturedProtoPte = *ProtoPte;

                        ASSERT (CapturedProtoPte.u.Hard.Valid == 0);
                        ASSERT (CapturedProtoPte.u.Soft.Prototype == 0);

                        //
                        // PTE is either demand zero, pagefile or transition,
                        // in all cases protection is in the prototype PTE.
                        //

                        Protect = MI_CONVERT_FROM_PTE_PROTECTION (CapturedProtoPte.u.Soft.Protection);

                        LOCK_WS_SHARED (Thread, TargetProcess);
                    }
                    else {
                        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
                        Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                    Pfn1->OriginalPte.u.Soft.Protection);
                    }
                }
                else if (Vad->u.VadFlags.VadType == VadAwe) {

                    //
                    // This is an AWE frame - the original PTE field in the PFN
                    // actually contains the AweReferenceCount.  These pages are
                    // either noaccess, readonly or readwrite.
                    //

                    if (PteContents.u.Hard.Owner == MI_PTE_OWNER_KERNEL) {
                        Protect = PAGE_NOACCESS;
                    }
                    else if (PteContents.u.Hard.Write == 1) {
                        Protect = PAGE_READWRITE;
                    }
                    else {
                        Protect = PAGE_READONLY;
                    }
                    State = MEM_COMMIT;
                }
                else {

                    Protect = MiGetPageProtection (PointerPte);

                    //
                    // The PointerPte contents may have changed if
                    // MiGetPageProtection had to release the working set
                    // pushlock, thus we snapped it prior and can continue
                    // to use the old copy regardless.  Note also that the
                    // page table page containing PointerPte may now have
                    // been paged out as well.
                    //

                    if ((PteContents.u.Soft.Valid == 0) &&
                        (PteContents.u.Soft.Prototype == 1) &&
                        (Vad->u.VadFlags.PrivateMemory == 0) &&
                        (Vad->ControlArea != NULL)) {

                        //
                        // Make sure the prototype PTE is committed.  Note
                        // that the prototype PTE itself may be paged out,
                        // thus the process working set pushlock must be
                        // released before accessing it.
                        //

                        ProtoPte = MiGetProtoPteAddress (Vad,
                                                         MI_VA_TO_VPN (Va));

                        if (ProtoPte != NULL) {
                            Thread = PsGetCurrentThread ();
                            UNLOCK_WS_SHARED (Thread, TargetProcess);
                            CapturedProtoPte = *ProtoPte;
                            LOCK_WS_SHARED (Thread, TargetProcess);
                        }
                        else {
                            CapturedProtoPte.u.Long = 0;
                        }

                        if (CapturedProtoPte.u.Long == 0) {
                            State = MEM_RESERVE;
                            Protect = 0;
                        }
                    }
                }
            }
        }
    }

    if (PteIsZero) {

        //
        // There is no PDE at this address, the template from
        // the VAD supplies the information unless the VAD is
        // for an image file.  For image files the individual
        // protection is on the prototype PTE.
        //

        //
        // Get the default protection information.
        //

        State = MEM_RESERVE;
        Protect = 0;

        if (Vad->u.VadFlags.VadType == VadAwe) {
            NOTHING;
        }
        else if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

            //
            // Must be banked memory, just return reserved.
            //

            NOTHING;
        }
        else if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                 (Vad->ControlArea != NULL)) {

            //
            // This VAD refers to a section.  Even though the PTE is
            // zero, the actual page may be committed in the section.
            //

            *NextVaToQuery = (PVOID)((PCHAR)Va + PAGE_SIZE);

            ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (Va));

            if (ProtoPte != NULL) {

                Thread = PsGetCurrentThread ();

                UNLOCK_WS_SHARED (Thread, TargetProcess);

                CapturedProtoPte = *ProtoPte;

                if (CapturedProtoPte.u.Long != 0) {
                    State = MEM_COMMIT;
    
                    if (Vad->u.VadFlags.VadType != VadImageMap) {
                        Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                                  Vad->u.VadFlags.Protection);
                    }
                    else {
    
                        //
                        // This is an image file, the protection is in the
                        // prototype PTE.
                        //
    
                        if (CapturedProtoPte.u.Hard.Valid == 0) {
                            Protect = MI_CONVERT_FROM_PTE_PROTECTION (CapturedProtoPte.u.Soft.Protection);
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

                            PointerPde = MiGetPteAddress (ProtoPte);

                            LOCK_PFN (OldIrql);

                            if (PointerPde->u.Hard.Valid == 0) {
                                MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);
                            }

                            CapturedProtoPte = *ProtoPte;

                            ASSERT (CapturedProtoPte.u.Long != 0);

                            if (CapturedProtoPte.u.Hard.Valid) {
                                Pfn1 = MI_PFN_ELEMENT (CapturedProtoPte.u.Hard.PageFrameNumber);

                                Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                          Pfn1->OriginalPte.u.Soft.Protection);
                            }
                            else {
                                Protect = MI_CONVERT_FROM_PTE_PROTECTION (CapturedProtoPte.u.Soft.Protection);
                            }
                            UNLOCK_PFN (OldIrql);
                        }
                    }
                }

                LOCK_WS_SHARED (Thread, TargetProcess);
            }
        }
        else {

            //
            // Get the protection from the corresponding VAD.
            //

            if (Vad->u.VadFlags.MemCommit) {
                State = MEM_COMMIT;
                Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                            Vad->u.VadFlags.Protection);
            }
        }
    }

    *ReturnedProtect = Protect;
    return State;
}


NTSTATUS
MiGetWorkingSetInfo (
    IN PMEMORY_WORKING_SET_INFORMATION WorkingSetInfo,
    IN SIZE_T Length,
    IN PEPROCESS Process
    )

{
    PMDL Mdl;
    PMEMORY_WORKING_SET_INFORMATION Info;
    PMEMORY_WORKING_SET_BLOCK Entry;
#if DBG
    PMEMORY_WORKING_SET_BLOCK LastEntry;
#endif
    PMMWSLE Wsle;
    PMMWSLE LastWsle;
    WSLE_NUMBER WsSize;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    NTSTATUS status;
    LOGICAL Attached;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;

    //
    // Allocate an MDL to map the request.
    //

    Mdl = ExAllocatePoolWithTag (NonPagedPool,
                                 sizeof(MDL) + sizeof(PFN_NUMBER) +
                                     BYTES_TO_PAGES (Length) * sizeof(PFN_NUMBER),
                                 '  mM');

    if (Mdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize the MDL for the request.
    //

    MmInitializeMdl(Mdl, WorkingSetInfo, Length);

    CurrentThread = PsGetCurrentThread ();

    try {
        MmProbeAndLockPages (Mdl,
                             KeGetPreviousModeByThread (&CurrentThread->Tcb),
                             IoWriteAccess);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ExFreePool (Mdl);
        return GetExceptionCode();
    }

    Info = MmGetSystemAddressForMdlSafe (Mdl, NormalPagePriority);

    if (Info == NULL) {
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (PsGetCurrentProcessByThread (CurrentThread) != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

    status = STATUS_SUCCESS;

    LOCK_WS_SHARED (CurrentThread, Process);

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        status = STATUS_PROCESS_IS_TERMINATING;
    }
    else {
        WsSize = Process->Vm.WorkingSetSize;
        ASSERT (WsSize != 0);
        Info->NumberOfEntries = WsSize;
        if (sizeof(MEMORY_WORKING_SET_INFORMATION) + (WsSize-1) * sizeof(ULONG_PTR) > Length) {
            status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }

    if (!NT_SUCCESS(status)) {

        UNLOCK_WS_SHARED (CurrentThread, Process);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return status;
    }

    Wsle = MmWsle;
    LastWsle = &MmWsle[MmWorkingSetList->LastEntry];
    Entry = &Info->WorkingSetInfo[0];

#if DBG
    LastEntry = (PMEMORY_WORKING_SET_BLOCK)(
                            (PCHAR)Info + (Length & (~(sizeof(ULONG_PTR) - 1))));
#endif

    do {
        if (Wsle->u1.e1.Valid == 1) {
            Entry->VirtualPage = Wsle->u1.e1.VirtualPageNumber;
            PointerPte = MiGetPteAddress (Wsle->u1.VirtualAddress);
            ASSERT (PointerPte->u.Hard.Valid == 1);
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

#if defined(MI_MULTINODE)
            Entry->Node = Pfn1->u3.e1.PageColor;
#else
            Entry->Node = 0;
#endif
            Entry->Shared = Pfn1->u3.e1.PrototypePte;
            if (Pfn1->u3.e1.PrototypePte == 0) {
                Entry->ShareCount = 0;
                Entry->Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(&Pfn1->OriginalPte);
            }
            else {
                if (Pfn1->u2.ShareCount <= 7) {
                    Entry->ShareCount = Pfn1->u2.ShareCount;
                }
                else {
                    Entry->ShareCount = 7;
                }
                if (Wsle->u1.e1.Protection == MM_ZERO_ACCESS) {
                    Entry->Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(&Pfn1->OriginalPte);
                }
                else {
                    Entry->Protection = Wsle->u1.e1.Protection;
                }
            }
            Entry += 1;
        }
        Wsle += 1;
#if DBG
        ASSERT ((Entry < LastEntry) || (Wsle > LastWsle));
#endif
    } while (Wsle <= LastWsle);

    UNLOCK_WS_SHARED (CurrentThread, Process);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    MmUnlockPages (Mdl);
    ExFreePool (Mdl);
    return STATUS_SUCCESS;
}

NTSTATUS
MiGetWorkingSetInfoList (
    IN PMEMORY_WORKING_SET_EX_INFORMATION WorkingSetInfo,
    IN SIZE_T Length,
    IN PEPROCESS Process
    )
{
    PMDL Mdl;
    PVOID VirtualAddress;
    TABLE_SEARCH_RESULT SearchResult;
    PMI_PHYSICAL_VIEW PhysicalView;
    PMEMORY_WORKING_SET_EX_INFORMATION Info;
    MEMORY_WORKING_SET_EX_INFORMATION Entry;
    PMMVAD Vad;
    PMMWSLE Wsle;
    SIZE_T WsSize;
    WSLE_NUMBER WorkingSetIndex;
    PMMPTE PointerPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    NTSTATUS status;
    LOGICAL Attached;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;

    WsSize = Length / sizeof (MEMORY_WORKING_SET_EX_INFORMATION);

    if (WsSize == 0) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    //
    // Allocate an MDL to map the request.
    //

    Mdl = ExAllocatePoolWithTag (NonPagedPool,
                                 sizeof (MDL) + sizeof (PFN_NUMBER) +
                                     BYTES_TO_PAGES (Length) * sizeof (PFN_NUMBER),
                                 '  mM');

    if (Mdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initialize the MDL for the request.
    //

    MmInitializeMdl (Mdl, WorkingSetInfo, Length);

    CurrentThread = PsGetCurrentThread ();

    try {
        MmProbeAndLockPages (Mdl,
                             KeGetPreviousModeByThread (&CurrentThread->Tcb),
                             IoWriteAccess);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ExFreePool (Mdl);
        return GetExceptionCode();
    }

    Info = MmGetSystemAddressForMdlSafe (Mdl, NormalPagePriority);

    if (Info == NULL) {
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (PsGetCurrentProcessByThread (CurrentThread) != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

    status = STATUS_SUCCESS;

    LOCK_WS_SHARED (CurrentThread, Process);

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {

        UNLOCK_WS_SHARED (CurrentThread, Process);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return STATUS_PROCESS_IS_TERMINATING;
    }

    for ( ; WsSize != 0; Info->u1.Long = Entry.u1.Long, Info += 1, WsSize -= 1) {
        Entry.u1.Long = 0;

        VirtualAddress = Info->VirtualAddress;

        if (VirtualAddress > MM_HIGHEST_USER_ADDRESS) {
            continue;
        }

        if (Process->PhysicalVadRoot != NULL) {

            //
            // This process has a \Device\PhysicalMemory VAD so it must be
            // checked to see if the current address resides in it since
            // the PFN cannot be safely examined if it is one of these.
            //

            SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                               MI_VA_TO_VPN (VirtualAddress),
                                               (PMMADDRESS_NODE *) &PhysicalView);
            if ((SearchResult == TableFoundNode) &&
                (PhysicalView->VadType == VadDevicePhysicalMemory)) {

                ASSERT ((ULONG)PhysicalView->Vad->u.VadFlags.VadType == (ULONG)PhysicalView->VadType);

                //
                // The VA lies within a device physical memory VAD.
                // Extract the protection from the VAD now.
                //

                Entry.u1.VirtualAttributes.Valid = 1;
                Entry.u1.VirtualAttributes.Locked = 1;

                Entry.u1.VirtualAttributes.Win32Protection =
                            MI_CONVERT_FROM_PTE_PROTECTION (
                                     PhysicalView->Vad->u.VadFlags.Protection);

                continue;
            }
        }

        PointerPxe = MiGetPxeAddress (VirtualAddress);
        PointerPpe = MiGetPpeAddress (VirtualAddress);
        PointerPde = MiGetPdeAddress (VirtualAddress);
        PointerPte = MiGetPteAddress (VirtualAddress);

        if (
#if (_MI_PAGING_LEVELS>=4)
           (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS>=3)
           (PointerPpe->u.Hard.Valid == 0) ||
#endif
           (PointerPde->u.Hard.Valid == 0)) {

            continue;
        }

        PteContents = *PointerPde;

        if (MI_PDE_MAPS_LARGE_PAGE (&PteContents)) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents) + MiGetPteOffset (VirtualAddress);
            Entry.u1.VirtualAttributes.Valid = 1;
            Entry.u1.VirtualAttributes.LargePage = 1;
            SATISFY_OVERZEALOUS_COMPILER (PteContents.u.Long = 0);
        }
        else {

            //
            // Snap the contents since AWE PTEs may be changing underneath us.
            //

            PteContents = *PointerPte;

            if (PteContents.u.Hard.Valid) {
                Entry.u1.VirtualAttributes.Valid = 1;
            }
            else {
                continue;
            }

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }

        //
        // We have a valid address and the corresponding PFN.  Extract the
        // interesting information now.
        //

        ASSERT (MI_IS_PFN (PageFrameIndex));

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if (Pfn1->u3.e1.PrototypePte) {
            Entry.u1.VirtualAttributes.Shared = 1;
        }

        Entry.u1.VirtualAttributes.Node = Pfn1->u3.e1.PageColor;

        Entry.u1.VirtualAttributes.Priority = MI_GET_PFN_PRIORITY (Pfn1);

        //
        // AWE, large page and \Device\PhysicalMemory entries have no WSLE
        // and so must be handled carefully.
        //

        if (Entry.u1.VirtualAttributes.LargePage == 1) {

            Vad = MiLocateAddress (VirtualAddress);

            ASSERT (Vad != NULL);

            Entry.u1.VirtualAttributes.Win32Protection = MI_CONVERT_FROM_PTE_PROTECTION (Vad->u.VadFlags.Protection);

            Entry.u1.VirtualAttributes.Locked = 1;

            ASSERT (Entry.u1.VirtualAttributes.ShareCount == 0);

            if (Pfn1->u3.e1.PrototypePte) {

                ULONG_PTR ShareCount;

                ShareCount = Pfn1->u2.ShareCount;

                if (Vad->u.VadFlags.VadType == VadLargePageSection) {

                    //
                    // Prior to Longhorn, the pagefile-backed large section
                    // code would increment the share count internally and
                    // this not something we want to return back to applications
                    // because it would confuse them so explicitly set it
                    // here for these types of sections.  In Longhorn the share
                    // count is always 1 for these so no setting at this
                    // location in the code is needed.
                    //

                    ShareCount -= 1;
                }

                if (ShareCount > 7) {
                    ShareCount = 7;
                }

                Entry.u1.VirtualAttributes.ShareCount = ShareCount;
            }

            continue;
        }

        if (Pfn1->u4.AweAllocation == 1) {

            //
            // AWE page.
            //

            if (PteContents.u.Hard.Owner == MI_PTE_OWNER_USER) {
                if (PteContents.u.Hard.Write) {
                    Entry.u1.VirtualAttributes.Win32Protection = PAGE_READWRITE;
                }
                else {
                    Entry.u1.VirtualAttributes.Win32Protection = PAGE_READONLY;
                }
            }
            else {
                Entry.u1.VirtualAttributes.Win32Protection = PAGE_NOACCESS;
            }

            Entry.u1.VirtualAttributes.Locked = 1;
            ASSERT (Entry.u1.VirtualAttributes.ShareCount == 0);
            continue;
        }

        ASSERT (Pfn1->u1.WsIndex != 0);

        WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                        MmWorkingSetList,
                                        Pfn1->u1.WsIndex,
                                        FALSE);

        Wsle = MmWsle + WorkingSetIndex;

        ASSERT (Wsle->u1.e1.Valid == 1);

        if (WorkingSetIndex < MmWorkingSetList->FirstDynamic) {
            Entry.u1.VirtualAttributes.Locked = 1;
        }

        if (Pfn1->u3.e1.PrototypePte == 0) {
            Entry.u1.VirtualAttributes.ShareCount = 0;
            Entry.u1.VirtualAttributes.Win32Protection = MI_CONVERT_FROM_PTE_PROTECTION (MI_GET_PROTECTION_FROM_SOFT_PTE (&Pfn1->OriginalPte));
        }
        else {
            if (Pfn1->u2.ShareCount <= 7) {
                Entry.u1.VirtualAttributes.ShareCount = Pfn1->u2.ShareCount;
            }
            else {
                Entry.u1.VirtualAttributes.ShareCount = 7;
            }
            if (Wsle->u1.e1.Protection == MM_ZERO_ACCESS) {
                Entry.u1.VirtualAttributes.Win32Protection = MI_CONVERT_FROM_PTE_PROTECTION (MI_GET_PROTECTION_FROM_SOFT_PTE (&Pfn1->OriginalPte));
            }
            else {
                Entry.u1.VirtualAttributes.Win32Protection = MI_CONVERT_FROM_PTE_PROTECTION (Wsle->u1.e1.Protection);
            }
        }
    }

    UNLOCK_WS_SHARED (CurrentThread, Process);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    MmUnlockPages (Mdl);
    ExFreePool (Mdl);
    return STATUS_SUCCESS;
}

