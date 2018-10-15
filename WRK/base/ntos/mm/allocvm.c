/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   allocvm.c

Abstract:

    This module contains the routines which implement the
    NtAllocateVirtualMemory service.

--*/

#include "mi.h"

const ULONG MMVADKEY = ' daV'; //Vad

NTSTATUS
MiResetVirtualMemory (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

VOID
MiFlushAcquire (
    IN PCONTROL_AREA ControlArea
    );

VOID
MiFlushRelease (
    IN PCONTROL_AREA ControlArea
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmCommitSessionMappedView)
#pragma alloc_text(PAGELK,MiResetVirtualMemory)
#if defined (_WIN64)
#pragma alloc_text(PAGELK,MiCreatePageDirectoriesForPhysicalRange)
#endif
#endif

SIZE_T MmTotalProcessCommit;        // Only used for debugging


NTSTATUS
NtAllocateVirtualMemory(
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __inout PSIZE_T RegionSize,
    __in ULONG AllocationType,
    __in ULONG Protect
    )

/*++

Routine Description:

    This function creates a region of pages within the virtual address
    space of a subject process.

Arguments:

    ProcessHandle - Supplies an open handle to a process object.

    BaseAddress - Supplies a pointer to a variable that will receive
                  the base address of the allocated region of pages.
                  If the initial value of this argument is not null,
                  then the region will be allocated starting at the
                  specified virtual address rounded down to the next
                  host page size address boundary. If the initial
                  value of this argument is null, then the operating
                  system will determine where to allocate the region.
        
    ZeroBits - Supplies the number of high order address bits that
               must be zero in the base address of the section view. The
               value of this argument must be less than or equal to the
               maximum number of zero bits and is only used when memory
               management determines where to allocate the view (i.e. when
               BaseAddress is null).

               If ZeroBits is zero, then no zero bit constraints are applied.

               If ZeroBits is greater than 0 and less than 32, then it is
               the number of leading zero bits from bit 31.  Bits 63:32 are
               also required to be zero.  This retains compatibility
               with 32-bit systems.
                
               If ZeroBits is greater than 32, then it is considered as
               a mask and then number of leading zero are counted out
               in the mask.  This then becomes the zero bits argument.

    RegionSize - Supplies a pointer to a variable that will receive
                 the actual size in bytes of the allocated region
                 of pages. The initial value of this argument
                 specifies the size in bytes of the region and is
                 rounded up to the next host page size boundary.

    AllocationType - Supplies a set of flags that describe the type
                     of allocation that is to be performed for the
                     specified region of pages. Flags are:
            
         MEM_COMMIT - The specified region of pages is to be committed.

         MEM_RESERVE - The specified region of pages is to be reserved.

         MEM_TOP_DOWN - The specified region should be created at the
                        highest virtual address possible based on ZeroBits.

         MEM_RESET - Reset the state of the specified region so
                     that if the pages are in a paging file, they
                     are discarded and if referenced later, pages of zeroes
                     are materialized.

                     If the pages are in memory and modified, they are marked
                     as not modified so they will not be written out to
                     the paging file.  The contents are NOT zeroed.

                     The Protect argument is ignored, but a valid protection
                     must be specified.

         MEM_PHYSICAL - The specified region of pages will map physical memory
                        directly via the AWE APIs.

         MEM_LARGE_PAGES - The specified region of pages will be allocated from
                           physically contiguous (non-paged) pages and be mapped
                           with a large TB entry.

         MEM_WRITE_WATCH - The specified private region is to be used for
                           write-watch purposes.

    Protect - Supplies the protection desired for the committed region of pages.

         PAGE_NOACCESS - No access to the committed region
                         of pages is allowed. An attempt to read,
                         write, or execute the committed region
                         results in an access violation.

         PAGE_EXECUTE - Execute access to the committed
                        region of pages is allowed. An attempt to
                        read or write the committed region results in
                        an access violation.

         PAGE_READONLY - Read only and execute access to the
                         committed region of pages is allowed. An
                         attempt to write the committed region results
                         in an access violation.

         PAGE_READWRITE - Read, write, and execute access to
                          the committed region of pages is allowed. If
                          write access to the underlying section is
                          allowed, then a single copy of the pages are
                          shared. Otherwise the pages are shared read
                          only/copy on write.

         PAGE_NOCACHE - The region of pages should be allocated
                        as non-cachable.

Return Value:

    Various NTSTATUS codes.

--*/

{
    PAWEINFO AweInfo;
    ULONG Locked;
    ULONG_PTR Alignment;
    PMMVAD Vad;
    PMMVAD FoundVad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    PVOID StartingAddress;
    PVOID EndingAddress;
    NTSTATUS Status;
    PVOID TopAddress;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    SIZE_T NumberOfPages;
    PMMPTE PointerPte;
    PMMPTE CommitLimitPte;
    MM_PROTECTION_MASK ProtectionMask;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE StartingPte;
    MMPTE TempPte;
    ULONG OldProtect;
    SIZE_T QuotaCharge;
    SIZE_T ExcessCharge;
    SIZE_T QuotaFree;
    SIZE_T CopyOnWriteCharge;
    LOGICAL Attached;
    LOGICAL ChargedExactQuota;
    MMPTE DecommittedPte;
    ULONG ChangeProtection;
    PVOID UsedPageTableHandle;
    PUCHAR Va;
    LOGICAL ChargedJobCommit;
    PMI_PHYSICAL_VIEW PhysicalView;
    PRTL_BITMAP BitMap;
    ULONG BitMapSize;
    ULONG BitMapBits;
    KAPC_STATE ApcState;
    SECTION Section;
    LARGE_INTEGER NewSize;
    PCONTROL_AREA ControlArea;
    PSEGMENT Segment;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    PAGED_CODE();

    Attached = FALSE;

    //
    // Check the zero bits argument for correctness.
    //

#if defined (_WIN64)

    if (ZeroBits >= 32) {

        //
        // ZeroBits is a mask instead of a count.  Translate it to a count now.
        //

        ZeroBits = 64 - RtlFindMostSignificantBit (ZeroBits) -1;        
    }
    else if (ZeroBits) {
        ZeroBits += 32;
    }

#endif

    if (ZeroBits > MM_MAXIMUM_ZERO_BITS) {
        return STATUS_INVALID_PARAMETER_3;
    }

    //
    // Check the AllocationType for correctness.
    //

    if ((AllocationType & ~(MEM_COMMIT | MEM_RESERVE | MEM_PHYSICAL |
                            MEM_LARGE_PAGES |
                            MEM_TOP_DOWN | MEM_RESET | MEM_WRITE_WATCH)) != 0) {
        return STATUS_INVALID_PARAMETER_5;
    }

    //
    // One of MEM_COMMIT, MEM_RESET or MEM_RESERVE must be set.
    //

    if ((AllocationType & (MEM_COMMIT | MEM_RESERVE | MEM_RESET)) == 0) {
        return STATUS_INVALID_PARAMETER_5;
    }

    if ((AllocationType & MEM_RESET) && (AllocationType != MEM_RESET)) {

        //
        // MEM_RESET may not be used with any other flag.
        //

        return STATUS_INVALID_PARAMETER_5;
    }

    //
    // All of these flags are generally rare so check for any of them being
    // set with one test.
    //

    if (AllocationType & (MEM_LARGE_PAGES | MEM_WRITE_WATCH | MEM_PHYSICAL)) {

        if (AllocationType & MEM_LARGE_PAGES) {
    
            //
            // Large page address spaces must be committed and cannot be combined
            // with physical, reset or write watch.
            //
    
            if ((AllocationType & MEM_COMMIT) == 0) {
                return STATUS_INVALID_PARAMETER_5;
            }
    
            if (AllocationType & (MEM_PHYSICAL | MEM_RESET | MEM_WRITE_WATCH)) {
                return STATUS_INVALID_PARAMETER_5;
            }
        }
        else if (AllocationType & MEM_WRITE_WATCH) {
    
            ASSERT ((AllocationType & (MEM_LARGE_PAGES)) == 0);
    
            if (AllocationType & MEM_PHYSICAL) {
                return STATUS_INVALID_PARAMETER_5;
            }
    
            //
            // Write watch address spaces can only be created with MEM_RESERVE.
            //
    
            if ((AllocationType & MEM_RESERVE) == 0) {
                return STATUS_INVALID_PARAMETER_5;
            }
        }
        else if (AllocationType & MEM_PHYSICAL) {
    
            ASSERT ((AllocationType & (MEM_LARGE_PAGES | MEM_WRITE_WATCH)) == 0);
    
            //
            // MEM_PHYSICAL must be used with MEM_RESERVE.
            // MEM_TOP_DOWN is optional.
            // Anything else is invalid.
            //
            // This memory is always read-write when allocated.
            //
    
            if ((AllocationType & MEM_RESERVE) == 0) {
                return STATUS_INVALID_PARAMETER_5;
            }
    
            if (AllocationType & ~(MEM_RESERVE | MEM_TOP_DOWN | MEM_PHYSICAL)) {
                return STATUS_INVALID_PARAMETER_5;
            }
    
            if (Protect != PAGE_READWRITE) {
                return STATUS_INVALID_PARAMETER_6;
            }
        }
    }

    //
    // Check the protection field.
    //

    ProtectionMask = MiMakeProtectionMask (Protect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ChangeProtection = FALSE;

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    //
    // Establish an exception handler, probe the specified addresses
    // for write access and capture the initial values.
    //

    try {

        if (PreviousMode != KernelMode) {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
        }

        //
        // Capture the base address.
        //

        CapturedBase = *BaseAddress;

        //
        // Capture the region size.
        //

        CapturedRegionSize = *RegionSize;

    } except (ExSystemExceptionFilter ()) {

        //
        // If an exception occurs during the probe or capture
        // of the initial values, then handle the exception and
        // return the exception code as the status value.
        //

        return GetExceptionCode ();
    }

    //
    // Make sure the specified starting and ending addresses are
    // within the user part of the virtual address space.
    //

    if (CapturedBase > MM_HIGHEST_VAD_ADDRESS) {

        //
        // Invalid base address.
        //

        return STATUS_INVALID_PARAMETER_2;
    }

    if ((((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) - (ULONG_PTR)CapturedBase) <
            CapturedRegionSize) {

        //
        // Invalid region size;
        //

        return STATUS_INVALID_PARAMETER_4;
    }

    if (CapturedRegionSize == 0) {

        //
        // Region size cannot be 0.
        //

        return STATUS_INVALID_PARAMETER_4;
    }

    //
    // Reference the specified process handle for VM_OPERATION access.
    //

    if (ProcessHandle == NtCurrentProcess ()) {
        Process = CurrentProcess;
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_VM_OPERATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&Process,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

    //
    // Check for privilege before attaching to prevent unprivileged apps
    // from dumping memory into a privileged process.
    //

    if (AllocationType & MEM_LARGE_PAGES) {

        if (!SeSinglePrivilegeCheck (SeLockMemoryPrivilege, PreviousMode)) {
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto ErrorReturn1;
        }
    }

    //
    // If the specified process is not the current process, attach
    // to the specified process.
    //

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    //
    // Get the address creation mutex to block multiple threads from
    // creating or deleting address space at the same time and
    // get the working set mutex so virtual address descriptors can
    // be inserted and walked.  Block APCs so an APC which takes a page
    // fault does not corrupt various structures.
    //

    QuotaCharge = 0;

    if ((CapturedBase == NULL) || (AllocationType & MEM_RESERVE)) {

        //
        // PAGE_WRITECOPY is not valid for private pages.
        //

        if ((Protect & PAGE_WRITECOPY) ||
            (Protect & PAGE_EXECUTE_WRITECOPY)) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn1;
        }

        Alignment = X64K;

        //
        // Reserve the address space.
        //

        if (CapturedBase == NULL) {

            //
            // No base address was specified.  This MUST be a reserve or
            // reserve and commit.
            //

            CapturedRegionSize = ROUND_TO_PAGES (CapturedRegionSize);

            //
            // If the number of zero bits is greater than zero, then calculate
            // the highest address.
            //

            if (ZeroBits != 0) {
                TopAddress = (PVOID)(((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT) >> ZeroBits);

                //
                // Keep the top address below the highest user vad address
                // regardless.
                //

                if (TopAddress > MM_HIGHEST_VAD_ADDRESS) {
                    Status = STATUS_INVALID_PARAMETER_3;
                    goto ErrorReturn1;
                }

            }
            else {
                TopAddress = (PVOID)MM_HIGHEST_VAD_ADDRESS;
            }

            //
            // Check whether the registry indicates that all applications
            // should be given virtual address ranges from the highest
            // address downwards in order to test 3GB-aware apps on 32-bit
            // machines and 64-bit apps on NT64.
            //

            if (Process->VmTopDown == 1) {
                AllocationType |= MEM_TOP_DOWN;
            }

            //
            // Note this calculation assumes the starting address will be
            // allocated on at least a page boundary.
            //

            NumberOfPages = BYTES_TO_PAGES (CapturedRegionSize);

            SATISFY_OVERZEALOUS_COMPILER (StartingAddress = NULL);
            SATISFY_OVERZEALOUS_COMPILER (EndingAddress = NULL);

            if (AllocationType & MEM_LARGE_PAGES) {

#ifdef _X86_
                if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
                    Status = STATUS_NOT_SUPPORTED;
                    goto ErrorReturn1;
                }
#endif

                //
                // Ensure the region size meets minimum size and alignment.
                //

                ASSERT (MM_MINIMUM_VA_FOR_LARGE_PAGE >= X64K);

                //
                // Ensure the size is a multiple of the minimum large page size.
                //

                if (CapturedRegionSize % MM_MINIMUM_VA_FOR_LARGE_PAGE) {
                    Status = STATUS_INVALID_PARAMETER_4;
                    goto ErrorReturn1;
                }

                //
                // Align the starting address to a natural boundary.
                //

                Alignment = MM_MINIMUM_VA_FOR_LARGE_PAGE;
            }
        }
        else {

            //
            // A non-NULL base address was specified.  Check to make sure
            // the specified base address to ending address is currently
            // unused.
            //

            EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                  CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

            if (AllocationType & MEM_LARGE_PAGES) {

#ifdef _X86_
                if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
                    Status = STATUS_NOT_SUPPORTED;
                    goto ErrorReturn1;
                }
#endif

                //
                // Ensure the region size meets minimum size and alignment.
                //

                ASSERT (MM_MINIMUM_VA_FOR_LARGE_PAGE >= X64K);

                //
                // Ensure the size is a multiple of the minimum large page size.
                //

                if (CapturedRegionSize % MM_MINIMUM_VA_FOR_LARGE_PAGE) {
                    Status = STATUS_INVALID_PARAMETER_4;
                    goto ErrorReturn1;
                }

                //
                // Align the starting and ending address to a natural boundary.
                //

                Alignment = MM_MINIMUM_VA_FOR_LARGE_PAGE;

                StartingAddress = (PVOID) MI_ALIGN_TO_SIZE (CapturedBase, Alignment);
                EndingAddress = (PVOID) ((ULONG_PTR)StartingAddress + CapturedRegionSize - 1);
            }
            else {

                //
                // Align the starting address on a 64k boundary.
                //

                StartingAddress = (PVOID)MI_64K_ALIGN (CapturedBase);
            }

            NumberOfPages = BYTES_TO_PAGES ((PCHAR)EndingAddress -
                                            (PCHAR)StartingAddress);

            SATISFY_OVERZEALOUS_COMPILER (TopAddress = NULL);
        }

        BitMapSize = 0;

        //
        // Allocate resources up front before acquiring mutexes to reduce
        // contention.
        //

        Vad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_SHORT), 'SdaV');

        if (Vad == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn1;
        }

        Vad->u.LongFlags = 0;

        //
        // Calculate the page file quota for this address range.
        //

        if (AllocationType & MEM_COMMIT) {
            QuotaCharge = NumberOfPages;
            Vad->u.VadFlags.MemCommit = 1;
        }

        Vad->u.VadFlags.Protection = ProtectionMask;
        Vad->u.VadFlags.PrivateMemory = 1;

        Vad->u.VadFlags.CommitCharge = QuotaCharge;

        SATISFY_OVERZEALOUS_COMPILER (BitMap = NULL);
        SATISFY_OVERZEALOUS_COMPILER (PhysicalView = NULL);
        SATISFY_OVERZEALOUS_COMPILER (Segment = NULL);

        AweInfo = NULL;
        if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {

            ASSERT ((AllocationType & MEM_WRITE_WATCH) == 0);

            if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {
                if (Process->AweInfo == NULL) {
                    AweInfo = MiAllocateAweInfo ();
                    if (AweInfo == NULL) {
                        ExFreePool (Vad);
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto ErrorReturn1;
                    }
                }
            }
            else {
    
                //
                // The working set mutex synchronizes the allocation of the
                // EPROCESS PhysicalVadRoot.  This table root is not deleted
                // until the process exits.
                //
    
                if ((Process->PhysicalVadRoot == NULL) &&
                    (MiCreatePhysicalVadRoot (Process, FALSE) == NULL)) {
    
                    ExFreePool (Vad);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorReturn1;
                }
            }

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_PHYSICAL_VIEW_KEY);

            if (PhysicalView == NULL) {
                if (AweInfo != NULL) {
                    MiFreeAweInfo (AweInfo);
                }
                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            if (AllocationType & MEM_PHYSICAL) {
                Vad->u.VadFlags.VadType = VadAwe;
            }
            else if (AllocationType & MEM_LARGE_PAGES) {
                Vad->u.VadFlags.VadType = VadLargePages;
            }

            PhysicalView->VadType = Vad->u.VadFlags.VadType;
            PhysicalView->Vad = Vad;
        }
        else if (AllocationType & MEM_WRITE_WATCH) {

            ASSERT (AllocationType & MEM_RESERVE);

#if defined (_WIN64)
            if (NumberOfPages >= _4gb) {

                //
                // The bitmap package only handles 32 bits.
                //

                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }
#endif

            //
            // The PhysicalVadRoot table root is not deleted until
            // the process exits.
            //

            if ((Process->PhysicalVadRoot == NULL) &&
                (MiCreatePhysicalVadRoot (Process, FALSE) == NULL)) {

                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            BitMapBits = (ULONG)NumberOfPages;

            BitMapSize = sizeof(RTL_BITMAP) + (ULONG)(((BitMapBits + 31) / 32) * 4);
            BitMap = ExAllocatePoolWithTag (NonPagedPool, BitMapSize, 'wwmM');

            if (BitMap == NULL) {
                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            //
            // Charge quota for the nonpaged pool for the bitmap.  This is
            // done here rather than by using ExAllocatePoolWithQuota
            // so the process object is not referenced by the quota charge.
            //
    
            Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                       BitMapSize);
    
            if (!NT_SUCCESS(Status)) {
                ExFreePool (Vad);
                ExFreePool (BitMap);
                goto ErrorReturn1;
            }

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_WRITEWATCH_VIEW_KEY);

            if (PhysicalView == NULL) {
                ExFreePool (Vad);
                ExFreePool (BitMap);
                PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            RtlInitializeBitMap (BitMap, (PULONG)(BitMap + 1), BitMapBits);
    
            RtlClearAllBits (BitMap);

            PhysicalView->Vad = Vad;
            PhysicalView->u.BitMap = BitMap;

            Vad->u.VadFlags.VadType = VadWriteWatch;
            PhysicalView->VadType = Vad->u.VadFlags.VadType;
        }

        //
        // Now acquire mutexes, check ranges and insert.
        //

        LOCK_ADDRESS_SPACE (Process);

        //
        // Make sure the address space was not deleted, if so,
        // return an error.
        //

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            Status = STATUS_PROCESS_IS_TERMINATING;
            if (AweInfo != NULL) {
                MiFreeAweInfo (AweInfo);
            }
            goto ErrorReleaseVad;
        }

        if (AweInfo != NULL) {
            MiInsertAweInfo (AweInfo);
        }

        //
        // Find a (or validate the) starting address.
        //

        if (CapturedBase == NULL) {

            if (AllocationType & MEM_TOP_DOWN) {

                //
                // Start from the top of memory downward.
                //

                Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                      CapturedRegionSize,
                                                      TopAddress,
                                                      Alignment,
                                                      &StartingAddress);
            }
            else {

                Status = MiFindEmptyAddressRange (CapturedRegionSize,
                                                  Alignment,
                                                  (ULONG)ZeroBits,
                                                  &StartingAddress);
            }

            if (!NT_SUCCESS (Status)) {
                goto ErrorReleaseVad;
            }

            //
            // Calculate the ending address based on the top address.
            //

            EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                  CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

            if (EndingAddress > TopAddress) {

                //
                // The allocation does not honor the zero bits argument.
                //

                Status = STATUS_NO_MEMORY;
                goto ErrorReleaseVad;
            }
        }
        else {

            //
            // See if a VAD overlaps with this starting/ending address pair.
            //

            if (MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress) == TRUE) {

                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReleaseVad;
            }
        }

        //
        // An unoccupied address range has been found, finish initializing
        // the virtual address descriptor to describe this range, then
        // insert it into the tree.
        //

        Vad->StartingVpn = MI_VA_TO_VPN (StartingAddress);
        Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);

        Status = MiInsertVadCharges (Vad, Process);

        if (!NT_SUCCESS(Status)) {

ErrorReleaseVad:

            //
            // The quota charge failed, deallocate the pool and return an error.
            //

            UNLOCK_ADDRESS_SPACE (Process);

            ExFreePool (Vad);

            if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {
                ExFreePool (PhysicalView);
            }
            else if (BitMapSize != 0) {
                ExFreePool (PhysicalView);
                ExFreePool (BitMap);
                PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
            }

            goto ErrorReturn1;
        }

        LOCK_WS_UNSAFE (CurrentThread, Process);

        MiInsertVad (Vad, Process);

        //
        // Initialize page directory and table pages for the physical range.
        //

        if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {

            if (AllocationType & MEM_LARGE_PAGES) {

                //
                // Temporarily make the VAD protection no access.  This allows
                // us to safely release the working set mutex while trying to
                // find contiguous memory to fill the large page range.
                // If another thread tries to access the large page VA range
                // before we find (and insert) a contiguous chunk, the thread
                // will get an AV.
                //

                Vad->u.VadFlags.Protection = MM_NOACCESS;

                ASSERT (((ULONG_PTR)StartingAddress % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);
                UNLOCK_WS_UNSAFE (CurrentThread, Process);

                Status = MiAllocateLargePages (StartingAddress,
                                               EndingAddress,
                                               ProtectionMask,
                                               FALSE);

                //
                // Restore the correct protection.
                //

                LOCK_WS_UNSAFE (CurrentThread, Process);

                Vad->u.VadFlags.Protection = ProtectionMask;
            }
            else if (MiCreatePageTablesForPhysicalRange (Process,
                                                         StartingAddress,
                                                         EndingAddress) == FALSE) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (!NT_SUCCESS (Status)) {

                PreviousVad = MiGetPreviousVad (Vad);
                NextVad = MiGetNextVad (Vad);

                MiRemoveVad (Vad, Process);

                UNLOCK_WS_UNSAFE (CurrentThread, Process);

                //
                // Return commitment for page table pages if possible.
                //

                MiReturnPageTablePageCommitment (StartingAddress,
                                                 EndingAddress,
                                                 Process,
                                                 PreviousVad,
                                                 NextVad);

                MiRemoveVadCharges (Vad, Process);

                UNLOCK_ADDRESS_SPACE (Process);

                ExFreePool (Vad);
                ExFreePool (PhysicalView);
                goto ErrorReturn1;
            }

            PhysicalView->StartingVpn = Vad->StartingVpn;
            PhysicalView->EndingVpn = Vad->EndingVpn;

            //
            // Insert the physical view into this process' list after
            // releasing the working set mutex.
            //

            UNLOCK_WS_UNSAFE (CurrentThread, Process);
            MiAweViewInserter (Process, PhysicalView);
            goto Inserted;
        }
        else if (BitMapSize != 0) {

            PhysicalView->StartingVpn = Vad->StartingVpn;
            PhysicalView->EndingVpn = Vad->EndingVpn;

            //
            // Insert the physical view descriptor now that the page table page
            // hierarchy is in place.  Note probes can find this descriptor
            // immediately once the working set mutex is released.
            //

            ASSERT (Process->PhysicalVadRoot != NULL);

            MiInsertNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);
            //
            // Mark this process as forever containing write-watch
            // address space(s).
            //

            PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_USING_WRITE_WATCH);
        }

        //
        // Unlock the working set lock, page faults can now be taken.
        //

        UNLOCK_WS_UNSAFE (CurrentThread, Process);

Inserted:

        //
        // Update the current virtual size in the process header, the
        // address space lock protects this operation.
        //

        CapturedRegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
        Process->VirtualSize += CapturedRegionSize;

        if (Process->VirtualSize > Process->PeakVirtualSize) {
            Process->PeakVirtualSize = Process->VirtualSize;
        }

        //
        // Release the address space lock, lower IRQL, detach, and dereference
        // the process object.
        //

        UNLOCK_ADDRESS_SPACE (Process);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }

        if (ProcessHandle != NtCurrentProcess ()) {
            ObDereferenceObject (Process);
        }

        //
        // Establish an exception handler and write the size and base
        // address.
        //

        try {

            *RegionSize = CapturedRegionSize;
            *BaseAddress = StartingAddress;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // Return success at this point even if the results
            // cannot be written.
            //

            NOTHING;
        }

        return STATUS_SUCCESS;
    }

    //
    // Commit previously reserved pages.  Note that these pages could
    // be either private or a section.
    //

    if (AllocationType == MEM_RESET) {

        //
        // Round up to page boundaries so good data is not reset.
        //

        EndingAddress = (PVOID)((ULONG_PTR)PAGE_ALIGN ((ULONG_PTR)CapturedBase +
                                    CapturedRegionSize) - 1);
        StartingAddress = (PVOID)PAGE_ALIGN((PUCHAR)CapturedBase + PAGE_SIZE - 1);
        if (StartingAddress > EndingAddress) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorReturn1;
        }
    }
    else {
        EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                CapturedRegionSize - 1) | (PAGE_SIZE - 1));
        StartingAddress = (PVOID)PAGE_ALIGN(CapturedBase);
    }

    CapturedRegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1;

    LOCK_ADDRESS_SPACE (Process);

    //
    // Make sure the address space was not deleted, if so,
    // return an error.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn0;
    }

    FoundVad = MiCheckForConflictingVad (Process, StartingAddress, EndingAddress);

    if (FoundVad == NULL) {

        //
        // No virtual address is reserved at the specified base address,
        // return an error.
        //

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
    }

    if ((FoundVad->u.VadFlags.VadType == VadAwe) ||
        (FoundVad->u.VadFlags.VadType == VadDevicePhysicalMemory) ||
        (FoundVad->u.VadFlags.VadType == VadLargePages)) {

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
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
        goto ErrorReturn0;
    }

    if (FoundVad->u.VadFlags.CommitCharge == MM_MAX_COMMIT) {

        //
        // This is a special VAD, don't let any commits occur.
        //

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
    }

    if (AllocationType == MEM_RESET) {

        Status = MiResetVirtualMemory (StartingAddress,
                                       EndingAddress,
                                       FoundVad,
                                       Process);

        UNLOCK_ADDRESS_SPACE (Process);

        goto done;
    }

    if (FoundVad->u.VadFlags.PrivateMemory == 0) {

        if (FoundVad->u.VadFlags.VadType == VadLargePageSection) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn0;
        }

        Status = STATUS_SUCCESS;

        //
        // The no cache and write combined options are not allowed for sections.
        //

        if ((Protect & (PAGE_NOCACHE | PAGE_WRITECOMBINE)) &&
            (FoundVad->u.VadFlags.VadType != VadRotatePhysical)) {

            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn0;
        }

        if (FoundVad->u.VadFlags.NoChange == 1) {

            //
            // An attempt is made at changing the protection
            // of a SEC_NO_CHANGE section.
            //

            Status = MiCheckSecuredVad (FoundVad,
                                        CapturedBase,
                                        CapturedRegionSize,
                                        ProtectionMask);

            if (!NT_SUCCESS (Status)) {
                goto ErrorReturn0;
            }
        }

        if (FoundVad->ControlArea->FilePointer != NULL) {
            if (FoundVad->u2.VadFlags2.ExtendableFile == 0) {

                //
                // Only page file backed sections can be committed.
                //

                Status = STATUS_ALREADY_COMMITTED;
                goto ErrorReturn0;
            }

            //
            // Commit the requested portions of the extendable file.
            //

            RtlZeroMemory (&Section, sizeof(SECTION));
            ControlArea = FoundVad->ControlArea;
            Section.Segment = ControlArea->Segment;
            Section.u.LongFlags = ControlArea->u.LongFlags;
            Section.InitialPageProtection = PAGE_READWRITE;
            NewSize.QuadPart = FoundVad->u2.VadFlags2.FileOffset;
            NewSize.QuadPart = NewSize.QuadPart << 16;
            NewSize.QuadPart += 1 +
                   ((PCHAR)EndingAddress - (PCHAR)MI_VPN_TO_VA (FoundVad->StartingVpn));
        
            //
            // The working set and address space mutexes must be
            // released prior to calling MmExtendSection otherwise
            // a deadlock with the filesystem can occur.
            //
            // Prevent the control area from being deleted while
            // the (potential) extension is ongoing.
            //

            MiFlushAcquire (ControlArea);

            UNLOCK_ADDRESS_SPACE (Process);
            
            Status = MmExtendSection (&Section, &NewSize, FALSE);
        
            if (NT_SUCCESS(Status)) {

                LOCK_ADDRESS_SPACE (Process);

                //
                // The Vad and/or the control area may have been changed
                // or deleted before the mutexes were regained above.
                // So everything must be revalidated.  Note that
                // if anything has changed, success is silently
                // returned just as if the protection change had failed.
                // It is the caller's fault if any of these has gone
                // away and they will suffer.
                //

                MiFlushRelease (ControlArea);

                if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
                    // Status = STATUS_PROCESS_IS_TERMINATING;
                    goto ErrorReturn0;
                }

                FoundVad = MiCheckForConflictingVad (Process,
                                                     StartingAddress,
                                                     EndingAddress);
        
                if (FoundVad == NULL) {
        
                    //
                    // No virtual address is reserved at the specified
                    // base address, return an error.
                    //
                    // Status = STATUS_CONFLICTING_ADDRESSES;

                    goto ErrorReturn0;
                }
        
                if (FoundVad->u.VadFlags.PrivateMemory == 1) {
                    goto ErrorReturn0;
                }

                if (ControlArea != FoundVad->ControlArea) {
                    goto ErrorReturn0;
                }

                ASSERT (FoundVad->ControlArea->FilePointer != NULL);
                ASSERT (FoundVad->u.VadFlags.VadType != VadAwe);
                ASSERT (FoundVad->u.VadFlags.VadType != VadLargePages);
        
                if (FoundVad->u.VadFlags.CommitCharge == MM_MAX_COMMIT) {

                    //
                    // This is a special VAD, no commits are allowed.
                    //
                    // Status = STATUS_CONFLICTING_ADDRESSES;

                    goto ErrorReturn0;
                }
        
                //
                // Ensure that the starting and ending addresses are
                // all within the same virtual address descriptor.
                //
        
                if ((MI_VA_TO_VPN (StartingAddress) < FoundVad->StartingVpn) ||
                    (MI_VA_TO_VPN (EndingAddress) > FoundVad->EndingVpn)) {
        
                    //
                    // Not within the section virtual address
                    // descriptor, return an error.
                    //
                    // Status = STATUS_CONFLICTING_ADDRESSES;

                    goto ErrorReturn0;
                }

                if (FoundVad->u.VadFlags.NoChange == 1) {
    
                    //
                    // An attempt is made at changing the protection
                    // of a SEC_NO_CHANGE section.
                    //
    
                    NTSTATUS Status2;

                    Status2 = MiCheckSecuredVad (FoundVad,
                                                 CapturedBase,
                                                 CapturedRegionSize,
                                                 ProtectionMask);
    
                    if (!NT_SUCCESS (Status2)) {
                        goto ErrorReturn0;
                    }
                }
    
                if (FoundVad->u2.VadFlags2.ExtendableFile == 0) {
                    goto ErrorReturn0;
                }

                MiSetProtectionOnSection (Process,
                                          FoundVad,
                                          StartingAddress,
                                          EndingAddress,
                                          Protect,
                                          &OldProtect,
                                          TRUE,
                                          &Locked);

                //
                //      ***  WARNING ***
                //
                // The alternate PTE support routines called by
                // MiSetProtectionOnSection may have deleted the old (small)
                // VAD and replaced it with a different (large) VAD - if so,
                // the old VAD is freed and cannot be referenced.
                //

                UNLOCK_ADDRESS_SPACE (Process);
            }
            else {
                MiFlushRelease (ControlArea);
            }

            goto ErrorReturn1;
        }

        StartingPte = MiGetProtoPteAddress (FoundVad,
                                            MI_VA_TO_VPN(StartingAddress));
        LastPte = MiGetProtoPteAddress (FoundVad,
                                        MI_VA_TO_VPN(EndingAddress));

        //
        // Check to ensure these pages can be committed if this
        // is a page file backed segment.  Note that page file quota
        // has already been charged for this.
        //

        PointerPte = StartingPte;
        QuotaCharge = 1 + LastPte - StartingPte;

        CopyOnWriteCharge = 0;

        //
        // PAGE_WRITECOPY is not valid for private pages and rotate physical
        // sections are private pages from the application's viewpoint.
        //

        if (FoundVad->u.VadFlags.VadType == VadRotatePhysical) {

            if ((Protect & PAGE_WRITECOPY) ||
                (Protect & PAGE_EXECUTE_WRITECOPY) ||
                (Protect & PAGE_NOACCESS) ||
                (Protect & PAGE_GUARD)) {

                Status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn0;
            }
        }
        else if (MI_IS_PTE_PROTECTION_COPY_WRITE (ProtectionMask)) {

            //
            // If the protection is copy on write, charge for
            // the copy on writes.
            //

            CopyOnWriteCharge = QuotaCharge;
        }

        //
        // Charge commitment for the range.
        //

        ChargedExactQuota = FALSE;
        ChargedJobCommit = FALSE;

        if (CopyOnWriteCharge != 0) {

            Status = PsChargeProcessPageFileQuota (Process, CopyOnWriteCharge);

            if (!NT_SUCCESS (Status)) {
                UNLOCK_ADDRESS_SPACE (Process);
                goto ErrorReturn1;
            }

            //
            // Note this job charging is unusual because it is not
            // followed by an immediate process charge.
            //

            if (Process->CommitChargeLimit) {
                if (Process->CommitCharge + CopyOnWriteCharge > Process->CommitChargeLimit) {
                    if (Process->Job) {
                        PsReportProcessMemoryLimitViolation ();
                    }
                    UNLOCK_ADDRESS_SPACE (Process);
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                    Status = STATUS_COMMITMENT_LIMIT;
                    goto ErrorReturn1;
                }
            }

            if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, CopyOnWriteCharge) == FALSE) {
                    UNLOCK_ADDRESS_SPACE (Process);
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                    Status = STATUS_COMMITMENT_LIMIT;
                    goto ErrorReturn1;
                }
                ChargedJobCommit = TRUE;
            }
        }

        if (MiChargeCommitment (QuotaCharge + CopyOnWriteCharge, NULL) == FALSE) {

            //
            // The commitment charging of quota failed, calculate the
            // exact quota taking into account pages that may already be
            // committed and retry the operation.
            //

            ChargedExactQuota = TRUE;

            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

            while (PointerPte <= LastPte) {

                //
                // Check to see if the prototype PTE is committed.
                // Note that prototype PTEs cannot be decommitted so
                // PTEs only need to be checked for zeroes.
                //

                if (PointerPte->u.Long != 0) {
                    QuotaCharge -= 1;
                }
                PointerPte += 1;
            }

            PointerPte = StartingPte;

            //
            // If the entire range is committed then there's nothing to charge.
            //

            if (QuotaCharge + CopyOnWriteCharge == 0) {
                KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
                QuotaFree = 0;
                goto FinishedCharging;
            }

            if (MiChargeCommitment (QuotaCharge + CopyOnWriteCharge, NULL) == FALSE) {

                //
                // Trying for the precise charge still failed,
                // so just return an error.
                //

                KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

                UNLOCK_ADDRESS_SPACE (Process);

                if (CopyOnWriteCharge != 0) {

                    if (ChargedJobCommit == TRUE) {
                        PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)CopyOnWriteCharge);
                    }
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                }

                Status = STATUS_COMMITMENT_LIMIT;
                goto ErrorReturn1;
            }
        }
        else {
            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);
        }

        //
        // Commit all the pages.
        //

        Segment = FoundVad->ControlArea->Segment;
        TempPte = Segment->SegmentPteTemplate;
        ASSERT (TempPte.u.Long != 0);

        QuotaFree = 0;

        while (PointerPte <= LastPte) {

            if (PointerPte->u.Long != 0) {

                //
                // Page is already committed, back out commitment.
                //

                QuotaFree += 1;
            }
            else {
                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            PointerPte += 1;
        }

        //
        // Subtract out any excess, then update the segment charges.
        // Note only segment commit is excess - process commit must
        // remain fully charged.
        //

        if (ChargedExactQuota == FALSE) {
            ASSERT (QuotaCharge >= QuotaFree);
            QuotaCharge -= QuotaFree;

            //
            // Return the QuotaFree excess commitment after the
            // mutexes are released to remove needless contention.
            //
        }
        else {

            //
            // Exact quota was charged so zero this to signify
            // there is no excess to return.
            //

            QuotaFree = 0;
        }

        if (QuotaCharge != 0) {
            Segment->NumberOfCommittedPages += QuotaCharge;
            InterlockedExchangeAddSizeT (&MmSharedCommit, QuotaCharge);

            MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_SEGMENT, QuotaCharge);
        }

        KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

        //
        // Update the per-process charges.
        //

        if (CopyOnWriteCharge != 0) {
            FoundVad->u.VadFlags.CommitCharge += CopyOnWriteCharge;
            Process->CommitCharge += CopyOnWriteCharge;

            MI_INCREMENT_TOTAL_PROCESS_COMMIT (CopyOnWriteCharge);

            if (Process->CommitCharge > Process->CommitChargePeak) {
                Process->CommitChargePeak = Process->CommitCharge;
            }

            MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_PROCESS, CopyOnWriteCharge);
        }

FinishedCharging:

        //
        // Change all the protections to be protected as specified.
        //

        MiSetProtectionOnSection (Process,
                                  FoundVad,
                                  StartingAddress,
                                  EndingAddress,
                                  Protect,
                                  &OldProtect,
                                  TRUE,
                                  &Locked);
    
        //
        //      ***  WARNING ***
        //
        // The alternate PTE support routines called by
        // MiSetProtectionOnSection may have deleted the old (small)
        // VAD and replaced it with a different (large) VAD - if so,
        // the old VAD is freed and cannot be referenced.
        //

        UNLOCK_ADDRESS_SPACE (Process);

        //
        // Return any excess segment commit that may have been charged.
        //

        if (QuotaFree != 0) {
            MiReturnCommitment (QuotaFree);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT, QuotaFree);
        }

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }

        if (ProcessHandle != NtCurrentProcess ()) {
            ObDereferenceObject (Process);
        }

        try {
            *RegionSize = CapturedRegionSize;
            *BaseAddress = StartingAddress;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // Return success at this point even if the results
            // cannot be written.
            //

            NOTHING;
        }

        return STATUS_SUCCESS;
    }

    ASSERT (FoundVad->u.VadFlags.VadType != VadRotatePhysical);

    //
    // PAGE_WRITECOPY is not valid for private pages.
    //

    if ((Protect & PAGE_WRITECOPY) ||
        (Protect & PAGE_EXECUTE_WRITECOPY)) {
        Status = STATUS_INVALID_PAGE_PROTECTION;
        goto ErrorReturn0;
    }

    //
    // Build a demand zero PTE with the proper protection.
    //

    TempPte.u.Long = 0;
    TempPte.u.Soft.Protection = ProtectionMask;

    DecommittedPte.u.Long = 0;
    DecommittedPte.u.Soft.Protection = MM_DECOMMIT;

    if (FoundVad->u.VadFlags.MemCommit) {
        CommitLimitPte = MiGetPteAddress (MI_VPN_TO_VA (FoundVad->EndingVpn));
    }
    else {
        CommitLimitPte = NULL;
    }

    //
    // The address range has not been committed, commit it now.
    // Note that for private pages, commitment is handled by
    // explicitly updating PTEs to contain Demand Zero entries.
    //

    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    //
    // Check to ensure these pages can be committed.
    //

    QuotaCharge = 1 + LastPte - PointerPte;

    //
    // Charge quota and commitment for the range.  Note that the Ps calls must
    // be made without holding the process working set pushlock so they must
    // always charge for the entire range and then return any excess.
    //

    if (Process->CommitChargeLimit) {
        if (Process->CommitCharge + QuotaCharge > Process->CommitChargeLimit) {
            if (Process->Job) {
                PsReportProcessMemoryLimitViolation ();
            }
            Status = STATUS_COMMITMENT_LIMIT;
            goto ErrorReturn0;
        }
    }

    ChargedJobCommit = FALSE;

    if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
        if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, QuotaCharge) == FALSE) {
            Status = STATUS_COMMITMENT_LIMIT;
            goto ErrorReturn0;
        }
        ChargedJobCommit = TRUE;
    }

    Status = PsChargeProcessPageFileQuota (Process, QuotaCharge);

    if (!NT_SUCCESS (Status)) {
        if (ChargedJobCommit == TRUE) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, 0 - QuotaCharge);
        }
        goto ErrorReturn0;
    }

    ChargedExactQuota = FALSE;
    ExcessCharge = 0;

    if (MiChargeCommitment (QuotaCharge, NULL) == FALSE) {

        Status = STATUS_COMMITMENT_LIMIT;

        LOCK_WS_UNSAFE (CurrentThread, Process);

        //
        // Quota charge failed, calculate the exact quota
        // taking into account pages that may already be
        // committed, subtract this from the total and retry the operation.
        //

        QuotaFree = MiCalculatePageCommitment (StartingAddress,
                                               EndingAddress,
                                               FoundVad,
                                               Process);

        if (QuotaFree != 0) {

            ChargedExactQuota = TRUE;
            QuotaCharge -= QuotaFree;
            ASSERT ((SSIZE_T)QuotaCharge >= 0);

            if ((QuotaCharge == 0) ||
                (MiChargeCommitment (QuotaCharge, NULL) == TRUE)) {

                //
                // All the pages are already committed or we were able to
                // charge for the difference so march on.
                //

                ExcessCharge = QuotaFree;
                Status = STATUS_SUCCESS;
            }
            else {

                //
                // Restore QuotaCharge to its entry value as this local is
                // used to return the Ps charges below in the failure path.
                //

                QuotaCharge += QuotaFree;
            }
        }

        if (!NT_SUCCESS (Status)) {

            UNLOCK_WS_UNSAFE (CurrentThread, Process);

            PsReturnProcessPageFileQuota (Process, QuotaCharge);

            if (ChargedJobCommit == TRUE) {
                PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, 0 - QuotaCharge);
            }

            Status = STATUS_COMMITMENT_LIMIT;
            goto ErrorReturn0;
        }
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_PROCESS2, QuotaCharge);

    FoundVad->u.VadFlags.CommitCharge += QuotaCharge;
    Process->CommitCharge += QuotaCharge;

    MI_INCREMENT_TOTAL_PROCESS_COMMIT (QuotaCharge);

    if (Process->CommitCharge > Process->CommitChargePeak) {
        Process->CommitChargePeak = Process->CommitCharge;
    }

    QuotaFree = 0;

    if (ChargedExactQuota == FALSE) {
        LOCK_WS_UNSAFE (CurrentThread, Process);
    }

    //
    // Fill in all the page directory and page table pages with the
    // demand zero PTE.
    //

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde = MiGetPteAddress (PointerPte);

            //
            // Pointing to the next page table page, make
            // a page table page exist and make it valid.
            //

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }

        if (PointerPte->u.Long == 0) {

            if (PointerPte <= CommitLimitPte) {

                //
                // This page is implicitly committed.
                //

                QuotaFree += 1;
            }

            //
            // Increment the count of non-zero page table entries
            // for this page table and the number of private pages
            // for the process.
            //

            Va = MiGetVirtualAddressMappedByPte (PointerPte);
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        else {
            if (PointerPte->u.Long == DecommittedPte.u.Long) {

                //
                // Only commit the page if it is already decommitted.
                //

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            else {
                QuotaFree += 1;

                //
                // Make sure the protection for the page is right.
                //

                if (!ChangeProtection) {

                    LOGICAL MayHaveReleasedWs;

                    if ((PointerPte->u.Soft.Valid == 0) &&
                        (PointerPte->u.Soft.Prototype == 1) &&
                        (PointerPte->u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED)) {

                        //
                        // MiGetPageProtection may need to release the working
                        // set pushlock to fault in the prototype PTE (this
                        // can only happen for forked processes).  So
                        // on return we must ensure the page table page is
                        // made resident if it was trimmed.
                        //

                        MayHaveReleasedWs = TRUE;
                    }
                    else {
                        MayHaveReleasedWs = FALSE;
                    }

                    if (Protect != MiGetPageProtection (PointerPte)) {
                        ChangeProtection = TRUE;
                    }

                    if (MayHaveReleasedWs == TRUE) {

                        //
                        // Just make the current page table valid.  If we are
                        // about to cross into a new one, we'll make that one
                        // resident at the top of the loop.
                        //

                        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
                    }
                }
            }
        }
        PointerPte += 1;
    }

    UNLOCK_WS_UNSAFE (CurrentThread, Process);

    if ((ChargedExactQuota == FALSE) && (QuotaFree != 0)) {
        ExcessCharge = QuotaFree;
    }

    if (ExcessCharge != 0) {

        if (ChargedExactQuota == FALSE) {
            FoundVad->u.VadFlags.CommitCharge -= ExcessCharge;
            ASSERT ((LONG_PTR)FoundVad->u.VadFlags.CommitCharge >= 0);
            Process->CommitCharge -= ExcessCharge;
        }

        UNLOCK_ADDRESS_SPACE (Process);

        if (ChargedExactQuota == FALSE) {
            MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - ExcessCharge);
            MiReturnCommitment (ExcessCharge);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM2, ExcessCharge);
        }

        PsReturnProcessPageFileQuota (Process, ExcessCharge);
        if (ChargedJobCommit) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)ExcessCharge);
        }
    }
    else {
        UNLOCK_ADDRESS_SPACE (Process);
    }

    //
    // Previously reserved pages have been committed or an error occurred.
    // Detach, dereference process and return status.
    //

done:

    if (ChangeProtection) {
        PVOID Start;
        SIZE_T Size;
        ULONG LastProtect;

        Start = StartingAddress;
        Size = CapturedRegionSize;
        MiProtectVirtualMemory (Process,
                                &Start,
                                &Size,
                                Protect,
                                &LastProtect);
    }

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (ProcessHandle != NtCurrentProcess ()) {
        ObDereferenceObject (Process);
    }

    //
    // Establish an exception handler and write the size and base
    // address.
    //

    try {

        *RegionSize = CapturedRegionSize;
        *BaseAddress = StartingAddress;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    return Status;

ErrorReturn0:
        UNLOCK_ADDRESS_SPACE (Process);

ErrorReturn1:
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        if (ProcessHandle != NtCurrentProcess ()) {
            ObDereferenceObject (Process);
        }
        return Status;
}

NTSTATUS
MmCommitSessionMappedView (
    __in_bcount(ViewSize) PVOID MappedAddress,
    __in SIZE_T ViewSize
    )

/*++

Routine Description:

    This function commits a region of pages within the session mapped
    view virtual address space.

Arguments:

    MappedAddress - Supplies the non-NULL address within a session mapped view
                    to begin committing pages at.  Note the backing section
                    must be pagefile backed.

    ViewSize - Supplies the actual size in bytes to be committed.

Return Value:

    Various NTSTATUS codes.

--*/

{
    PSUBSECTION Subsection;
    ULONG_PTR Base16;
    ULONG Hash;
    ULONG Size;
    ULONG count;
    PMMSESSION Session;
    PVOID ViewBaseAddress;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE StartingPte;
    MMPTE TempPte;
    SIZE_T QuotaCharge;
    SIZE_T QuotaFree;
    LOGICAL ChargedExactQuota;
    PCONTROL_AREA ControlArea;
    PSEGMENT Segment;

    PAGED_CODE();

    //
    // Make sure the specified starting and ending addresses are
    // within the session view portion of the virtual address space.
    //

    if (((ULONG_PTR)MappedAddress < MiSessionViewStart) ||
        ((ULONG_PTR)MappedAddress >= MiSessionViewStart + MmSessionViewSize)) {

        //
        // Invalid base address.
        //

        return STATUS_INVALID_PARAMETER_1;
    }

    if ((ULONG_PTR)MiSessionViewStart + MmSessionViewSize - (ULONG_PTR)MappedAddress <
        ViewSize) {

        //
        // Invalid region size;
        //

        return STATUS_INVALID_PARAMETER_2;
    }

    ASSERT (ViewSize != 0);

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    //
    // Commit previously reserved pages.
    //

    StartingAddress = (PVOID)PAGE_ALIGN (MappedAddress);

    EndingAddress = (PVOID)(((ULONG_PTR)MappedAddress +
                            ViewSize - 1) | (PAGE_SIZE - 1));

    ViewSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1;

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    Session = &MmSessionSpace->Session;

    ChargedExactQuota = FALSE;

    QuotaCharge = (MiGetPteAddress (EndingAddress) - MiGetPteAddress (StartingAddress) + 1);

    //
    // Get the session view mutex to prevent win32k referencing bugs where
    // they might be trying to delete the view at the same time in another
    // thread.  This also blocks APCs so an APC which takes a page
    // fault does not corrupt various structures.
    //

    count = 0;

    Base16 = (ULONG_PTR)StartingAddress >> 16;

    LOCK_SYSTEM_VIEW_SPACE (Session);

    Hash = (ULONG)(Base16 % Session->SystemSpaceHashKey);

    do {
            
        ViewBaseAddress = (PVOID)(Session->SystemSpaceViewTable[Hash].Entry & ~0xFFFF);

        Size = (ULONG) ((Session->SystemSpaceViewTable[Hash].Entry & 0xFFFF) * X64K);

        if ((StartingAddress >= ViewBaseAddress) &&
            (EndingAddress < (PVOID)((PCHAR)ViewBaseAddress + Size))) {

            break;
        }

        Hash += 1;
        if (Hash >= Session->SystemSpaceHashSize) {
            Hash = 0;
            count += 1;
            if (count == 2) {
                KeBugCheckEx (DRIVER_UNMAPPING_INVALID_VIEW,
                              (ULONG_PTR)StartingAddress,
                              2,
                              0,
                              0);
            }
        }
    } while (TRUE);

    ControlArea = Session->SystemSpaceViewTable[Hash].ControlArea;

    if (ControlArea->FilePointer != NULL) {

        //
        // Only page file backed sections can be committed.
        //

        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return STATUS_ALREADY_COMMITTED;
    }

    //
    // Session views always start at the beginning of the file which makes
    // calculating the corresponding prototype PTE here straightforward.
    //

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    StartingPte = Subsection->SubsectionBase;

    StartingPte += (((ULONG_PTR) StartingAddress - (ULONG_PTR) ViewBaseAddress) >> PAGE_SHIFT);

    LastPte = StartingPte + QuotaCharge;

    if (LastPte >= Subsection->SubsectionBase + Subsection->PtesInSubsection) {
        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return STATUS_INVALID_PARAMETER_2;
    }

    //
    // Charge commitment for the range.
    //

    PointerPte = StartingPte;

    do {
        if (MiChargeCommitment (QuotaCharge, NULL) == TRUE) {
            break;
        }

        //
        // Reduce the charge we are asking for if possible.
        //

        if (ChargedExactQuota == TRUE) {

            //
            // We have already tried for the precise charge,
            // so just return an error.
            //

            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

            UNLOCK_SYSTEM_VIEW_SPACE (Session);
            return STATUS_COMMITMENT_LIMIT;
        }

        //
        // The commitment charging of quota failed, calculate the
        // exact quota taking into account pages that may already be
        // committed and retry the operation.
        //

        KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

        while (PointerPte < LastPte) {

            //
            // Check to see if the prototype PTE is committed.
            // Note that prototype PTEs cannot be decommitted so
            // PTEs only need to be checked for zeroes.
            //

            if (PointerPte->u.Long != 0) {
                QuotaCharge -= 1;
            }
            PointerPte += 1;
        }

        PointerPte = StartingPte;

        ChargedExactQuota = TRUE;

        //
        // If the entire range is committed then there's nothing to charge.
        //

        if (QuotaCharge == 0) {
            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

            UNLOCK_SYSTEM_VIEW_SPACE (Session);
            return STATUS_SUCCESS;
        }

    } while (TRUE);

    if (ChargedExactQuota == FALSE) {
        KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);
    }

    //
    // Commit all the pages.
    //

    Segment = ControlArea->Segment;
    TempPte = Segment->SegmentPteTemplate;
    ASSERT (TempPte.u.Long != 0);

    QuotaFree = 0;

    while (PointerPte < LastPte) {

        if (PointerPte->u.Long != 0) {

            //
            // Page is already committed, back out commitment.
            //

            QuotaFree += 1;
        }
        else {
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        PointerPte += 1;
    }

    //
    // Subtract out any excess, then update the segment charges.
    // Note only segment commit is excess - process commit must
    // remain fully charged.
    //

    if (ChargedExactQuota == FALSE) {
        ASSERT (QuotaCharge >= QuotaFree);
        QuotaCharge -= QuotaFree;

        //
        // Return the QuotaFree excess commitment after the
        // mutexes are released to remove needless contention.
        //
    }
    else {

        //
        // Exact quota was charged so zero this to signify
        // there is no excess to return.
        //

        QuotaFree = 0;
    }

    if (QuotaCharge != 0) {
        Segment->NumberOfCommittedPages += QuotaCharge;
        InterlockedExchangeAddSizeT (&MmSharedCommit, QuotaCharge);

        MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_SEGMENT, QuotaCharge);
    }

    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

    //
    // Update the per-process charges.
    //

    UNLOCK_SYSTEM_VIEW_SPACE (Session);

    //
    // Return any excess segment commit that may have been charged.
    //

    if (QuotaFree != 0) {
        MiReturnCommitment (QuotaFree);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT, QuotaFree);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiResetVirtualMemory (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    )

/*++

Routine Description:


Arguments:

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

    Vad - Supplies the relevant VAD for the range.

    Process - Supplies the current process.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, APCs disabled, AddressCreation mutex held.

--*/

{
    PVOID TempVa;
    PMMPTE PointerPte;
    PMMPTE ProtoPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPte;
    MMPTE PteContents;
    ULONG Waited;
    ULONG First;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMCLONE_BLOCK CloneBlock;
    PETHREAD Thread;
    PFN_NUMBER PageFrameIndex;
#if DBG
    PMMCLONE_DESCRIPTOR CloneDescriptor;
#endif
    MMPTE_FLUSH_LIST PteFlushList;
#if defined(_X86_) || defined(_AMD64_)
    WSLE_NUMBER WsPfnIndex;
    WSLE_NUMBER WorkingSetIndex;
#endif

    if (Vad->u.VadFlags.PrivateMemory == 0) {

        if (Vad->ControlArea->FilePointer != NULL) {

            //
            // Only page file backed sections can be reset.
            //

            return STATUS_USER_MAPPED_FILE;
        }
    }

    OldIrql = MM_NOIRQL;

    First = TRUE;
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    PteFlushList.Count = 0;

    Thread = PsGetCurrentThread ();

    MmLockPageableSectionByHandle (ExPageLockHandle);

    //
    // Examine all the PTEs in the range.
    //

    LOCK_WS_UNSAFE (Thread, Process);

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte) || (First)) {

            if (PteFlushList.Count != 0) {
                MiFlushPteList (&PteFlushList);
            }

            if (MiIsPteOnPpeBoundary (PointerPte) || (First)) {

                if (MiIsPteOnPxeBoundary (PointerPte) || (First)) {

                    PointerPxe = MiGetPpeAddress (PointerPte);

                    if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                     Process,
                                                     OldIrql,
                                                     &Waited)) {

                        //
                        // This extended page directory parent entry is empty,
                        // go to the next one.
                        //

                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                        continue;
                    }
                }

                PointerPpe = MiGetPdeAddress (PointerPte);

                if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                 Process,
                                                 OldIrql,
                                                 &Waited)) {

                    //
                    // This page directory parent entry is empty,
                    // go to the next one.
                    //

                    PointerPpe += 1;
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    continue;
                }
            }

            //
            // Pointing to the next page table page, make
            // a page table page exist and make it valid.
            //

            First = FALSE;
            PointerPde = MiGetPteAddress (PointerPte);

            if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                             Process,
                                             OldIrql,
                                             &Waited)) {

                //
                // This page directory entry is empty, go to the next one.
                //

                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

                //
                // This page directory entry is large so skip it.
                //

                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }
        }

        PteContents = *PointerPte;
        ProtoPte = NULL;

        if ((PteContents.u.Hard.Valid == 0) &&
            (PteContents.u.Soft.Prototype == 1))  {

            //
            // This is a prototype PTE, evaluate the prototype PTE.  Note that
            // the fact it is a prototype PTE does not guarantee that this is a
            // regular or long VAD - it may be a short VAD in a forked process,
            // so check PrivateMemory before referencing the FirstPrototypePte
            // field.
            //

            if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                (Vad->FirstPrototypePte != NULL)) {

                ProtoPte = MiGetProtoPteAddress (Vad,
                                        MI_VA_TO_VPN (
                                        MiGetVirtualAddressMappedByPte (PointerPte)));
            }
            else {
                CloneBlock = (PMMCLONE_BLOCK) MiPteToProto (PointerPte);
                ProtoPte = (PMMPTE) CloneBlock;
#if DBG
                CloneDescriptor = MiLocateCloneAddress (Process, (PVOID)CloneBlock);
                ASSERT (CloneDescriptor != NULL);
#endif
            }

            if (OldIrql == MM_NOIRQL) {
                ASSERT (PteFlushList.Count == 0);
                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
            }

            //
            // The working set mutex may be released in order to make the
            // prototype PTE which resides in paged pool resident.  If this
            // occurs, the page directory and/or page table of the original
            // user address may get trimmed.  Account for that here.
            //

            if (MiGetPteAddress (ProtoPte)->u.Hard.Valid == 0) {

                if (PteFlushList.Count != 0) {
                    MiFlushPteList (&PteFlushList);
                }

                if (MiMakeSystemAddressValidPfnWs (ProtoPte, Process, OldIrql) != 0) {

                    //
                    // Working set pushlock and PFN lock were released
                    // and reacquired, restart from the top.
                    //

                    First = TRUE;
                    continue;
                }
            }

            PteContents = *ProtoPte;
        }

        if (PteContents.u.Hard.Valid == 1) {

            PageFrameIndex = (PFN_NUMBER) PteContents.u.Hard.PageFrameNumber;

            if (Vad->u.VadFlags.VadType == VadRotatePhysical) {

                if (!ProtoPte) {

                    if (!MI_IS_PFN (PageFrameIndex)) {

                        //
                        // This address in the view is currently pointing at
                        // the frame buffer, ignore the reset request.
                        //

                        PointerPte += 1;
                        continue;
                    }
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    if ((Pfn1->u3.e1.PrototypePte == 0) ||
                        (Pfn1->PteAddress < Vad->FirstPrototypePte) ||
                        (Pfn1->PteAddress > Vad->FirstPrototypePte)) {

                        //
                        // This address in the view is currently pointing at
                        // the AGP frame buffer, ignore the reset request.
                        //

                        PointerPte += 1;
                        continue;
                    }
                }
            }

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

#if defined(_X86_) || defined(_AMD64_)

            if (!ProtoPte) {

                //
                // The access bit is set (and TB inserted) automatically by the
                // processor if the valid bit is set so clear it here in both
                // the PTE and the WSLE so we know it's more worthwhile to trim
                // should we need the memory.  If the access bit is already
                // clear then just skip the WSLE search under the premise
                // that it is already getting aged.
                //

                if (MI_GET_ACCESSED_IN_PTE (&PteContents) == 1) {

                    MI_SET_ACCESSED_IN_PTE (PointerPte, 0);

                    WsPfnIndex = Pfn1->u1.WsIndex;
    
                    TempVa = MiGetVirtualAddressMappedByPte (PointerPte);

                    WorkingSetIndex = MiLocateWsle (TempVa,
                                                    MmWorkingSetList,
                                                    WsPfnIndex, 
                                                    FALSE);
  
                    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);
    
                    MmWsle[WorkingSetIndex].u1.e1.Age = 3;
                }
            }

#endif

            if (OldIrql == MM_NOIRQL) {
                ASSERT (PteFlushList.Count == 0);
                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
                continue;
            }

            if (Pfn1->u3.e2.ReferenceCount == 1) {

                //
                // Only this process has the page mapped.
                //

                MI_SET_MODIFIED (Pfn1, 0, 0x20);
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                // MI_SET_PFN_PRIORITY (Pfn1, 0); Don't do this until referencing it back in sets it properly
            }

            if (!ProtoPte) {

                if (MI_IS_PTE_DIRTY (PteContents)) {

                    //
                    // Clear the dirty bit and flush TB since it
                    // is NOT a prototype PTE.
                    //

                    MI_SET_ACCESSED_IN_PTE (&PteContents, 0);
                    MI_SET_PTE_CLEAN (PteContents);

                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

                    if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                        TempVa = MiGetVirtualAddressMappedByPte (PointerPte);
                        PteFlushList.FlushVa[PteFlushList.Count] = TempVa;
                        PteFlushList.Count += 1;
                    }
                }
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

            if (OldIrql == MM_NOIRQL) {

                //
                // This must be a private page (because the PFN lock is not
                // held).  If the page is clean, just march on to the next one.
                //

                ASSERT (!ProtoPte);
                ASSERT (PteFlushList.Count == 0);

                if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
                    PointerPte += 1;
                    continue;
                }

                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
                continue;
            }

            if ((Pfn1->u3.e1.PageLocation == ModifiedPageList) &&
                (Pfn1->u3.e2.ReferenceCount == 0)) {

                //
                // Remove from the modified list, release the page
                // file space and insert on the standby list.
                //

                MI_SET_MODIFIED (Pfn1, 0, 0x21);
                MiUnlinkPageFromList (Pfn1);
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;

                //
                // Only reset the priority if we're moving it so the
                // standby list counts will remain correct.
                //

                // MI_SET_PFN_PRIORITY (Pfn1, 0); Don't do this until referencing it back in sets it properly

                MiInsertPageInList (&MmStandbyPageListHead,
                                    MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
            }
        }
        else {
            if (PteContents.u.Soft.PageFileHigh != 0) {

                if (OldIrql == MM_NOIRQL) {

                    //
                    // This must be a private page (because the PFN
                    // lock is not held).
                    //

                    ASSERT (!ProtoPte);
                    ASSERT (PteFlushList.Count == 0);

                    LOCK_PFN (OldIrql);
                    ASSERT (OldIrql != MM_NOIRQL);
                }

                MiReleasePageFileSpace (PteContents);

                if (PteFlushList.Count != 0) {
                    MiFlushPteList (&PteFlushList);
                }

                if (ProtoPte) {
                    ProtoPte->u.Soft.PageFileHigh = 0;
                }

                UNLOCK_PFN (OldIrql);
                OldIrql = MM_NOIRQL;

                if (!ProtoPte) {
                    PointerPte->u.Soft.PageFileHigh = 0;
                }
            }
            else {
                if (OldIrql != MM_NOIRQL) {

                    if (PteFlushList.Count != 0) {
                        MiFlushPteList (&PteFlushList);
                    }

                    UNLOCK_PFN (OldIrql);
                    OldIrql = MM_NOIRQL;
                }
            }
        }
        PointerPte += 1;
    }
    if (OldIrql != MM_NOIRQL) {
        if (PteFlushList.Count != 0) {
            MiFlushPteList (&PteFlushList);
        }

        UNLOCK_PFN (OldIrql);
        OldIrql = MM_NOIRQL;
    }
    else {
        ASSERT (PteFlushList.Count == 0);
    }

    UNLOCK_WS_UNSAFE (Thread, Process);

    MmUnlockPageableImageSection (ExPageLockHandle);

    return STATUS_SUCCESS;
}

PFN_NUMBER
MiResidentPagesForSpan (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

/*++

Routine Description:

    This routine computes the resident available charge for the paging
    hierarchies for the specified virtual address range.

Arguments:

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

Return Value:

    The resident available that the caller should charge.

Environment:

    Kernel mode.

--*/

{
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE LastPpe;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PFN_NUMBER PagesNeeded;

    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPpe = MiGetPpeAddress (EndingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    //
    // Compute the resident available pages charge for all of the
    // page directory and table pages in case they all get locked in later.
    //

    if (LastPte != PointerPte) {
        PagesNeeded = MI_COMPUTE_PAGES_SPANNED (PointerPte,
                                                (LastPte - PointerPte) * sizeof (MMPTE));

#if (_MI_PAGING_LEVELS >= 3)
        if (LastPde != PointerPde) {
            PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPde,
                                                     (LastPde - PointerPde) * sizeof (MMPTE));
#if (_MI_PAGING_LEVELS >= 4)
            if (LastPpe != PointerPpe) {
                PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPpe,
                                                         (LastPpe - PointerPpe) * sizeof (MMPTE));
            }
#endif
        }
#endif
    }
    else {
        PagesNeeded = 1;
#if (_MI_PAGING_LEVELS >= 3)
        PagesNeeded += 1;
#endif
#if (_MI_PAGING_LEVELS >= 4)
        PagesNeeded += 1;
#endif
    }

    return PagesNeeded;
}


LOGICAL
MiCreatePageTablesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

/*++

Routine Description:

    This routine initializes page directory and page table pages for a
    user-controlled physical range of pages.

Arguments:

    Process - Supplies the current process.

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

Return Value:

    TRUE if the page tables were created, FALSE if not.

Environment:

    Kernel mode, APCs disabled, WorkingSetMutex and AddressCreation mutexes
    held.

--*/

{
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER PagesNeeded;

    PagesNeeded = MiResidentPagesForSpan (StartingAddress, EndingAddress);

    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    //
    // Charge resident available pages for all of the page directory and table
    // pages as they will not be paged until the VAD is freed.
    //

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)PagesNeeded > MI_NONPAGEABLE_MEMORY_AVAILABLE() - 20) {
        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE);

    UNLOCK_PFN (OldIrql);

    UsedPageTableHandle = NULL;

    //
    // Fill in all the page table pages with the zero PTE.
    //

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte) || UsedPageTableHandle == NULL) {

            PointerPde = MiGetPteAddress (PointerPte);

            //
            // Pointing to the next page table page, make
            // a page table page exist and make it valid.
            //
            // Note this ripples sharecounts through the paging hierarchy so
            // there is no need to up sharecounts to prevent trimming of the
            // page directory (and parent) page as making the page table
            // valid below does this automatically.
            //

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

            //
            // Up the sharecount so the page table page will not get
            // trimmed even if it has no currently valid entries.
            //

            PteContents = *PointerPde;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            LOCK_PFN (OldIrql);
            Pfn1->u2.ShareCount += 1;
            UNLOCK_PFN (OldIrql);

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);
        }

        ASSERT (PointerPte->u.Long == 0);

        //
        // Increment the count of non-zero page table entries
        // for this page table - even though this entry is still zero,
        // this is a special case.
        //

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        PointerPte += 1;
        StartingAddress = (PVOID)((PUCHAR)StartingAddress + PAGE_SIZE);
    }

    return TRUE;
}

VOID
MiDeletePageTablesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

/*++

Routine Description:

    This routine deletes page directory and page table pages for a
    user-controlled physical range of pages.

    Even though PTEs may be zero in this range, UsedPageTable counts were
    incremented for these special ranges and must be decremented now.

Arguments:

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, WorkingSetMutex and AddressCreation mutexes
    held.

--*/

{
    PVOID TempVa;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PagesNeeded;
    PEPROCESS CurrentProcess;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPpe;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif

    CurrentProcess = PsGetCurrentProcess();

    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);

    //
    // Each PTE is already zeroed - just delete the containing pages.
    //
    // Restore resident available pages for all of the page directory and table
    // pages as they can now be paged again.
    //

    PagesNeeded = MiResidentPagesForSpan (StartingAddress, EndingAddress);

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {

        ASSERT (PointerPte->u.Long == 0);

        PointerPte += 1;

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        if ((MiIsPteOnPdeBoundary(PointerPte)) || (PointerPte > LastPte)) {

            //
            // The virtual address is on a page directory boundary or it is
            // the last address in the entire range.
            //
            // If all the entries have been eliminated from the previous
            // page table page, delete the page table page itself.
            //

            PointerPde = MiGetPteAddress (PointerPte - 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);

            //
            // Down the sharecount on the finished page table page.
            //

            PteContents = *PointerPde;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u2.ShareCount > 1);
            Pfn1->u2.ShareCount -= 1;

            //
            // If all the entries have been eliminated from the previous
            // page table page, delete the page table page itself.
            //

            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                ASSERT (PointerPde->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 3)
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte(PointerPde);
                MiDeletePte (PointerPde,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
                if ((MiIsPteOnPpeBoundary(PointerPte)) || (PointerPte > LastPte)) {
    
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
                                     CurrentProcess,
                                     NULL,
                                     NULL,
                                     OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                        if ((MiIsPteOnPxeBoundary(PointerPte)) || (PointerPte > LastPte)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                                ASSERT (PointerPxe->u.Long != 0);
                                TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                                MiDeletePte (PointerPxe,
                                             TempVa,
                                             FALSE,
                                             CurrentProcess,
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

            if (PointerPte > LastPte) {
                break;
            }

            //
            // Release the PFN lock.  This prevents a single thread
            // from forcing other high priority threads from being
            // blocked while a large address range is deleted.
            //

            UNLOCK_PFN (OldIrql);
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE ((PVOID)((PUCHAR)StartingAddress + PAGE_SIZE));
            LOCK_PFN (OldIrql);
        }

        StartingAddress = (PVOID)((PUCHAR)StartingAddress + PAGE_SIZE);
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_FREE_USER_PAGE_TABLE);

    return;
}
#if defined (_WIN64)

LOGICAL
MiCreatePageDirectoriesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

/*++

Routine Description:

    This routine initializes page directories for a
    user-controlled large page range.

Arguments:

    Process - Supplies the current process.

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

Return Value:

    TRUE if the page tables were created, FALSE if not.

Environment:

    Kernel mode, APCs disabled, AddressCreation mutex held.

--*/

{
    PETHREAD CurrentThread;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PVOID UsedPageDirHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER PagesNeeded;

    CurrentThread = PsGetCurrentThread ();

    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    PointerPde = MiGetPdeAddress (StartingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);

    PagesNeeded = MiResidentPagesForSpan (StartingAddress, EndingAddress);

    //
    // Subtract off the page table charge since none will be used.
    //

    if (LastPte != PointerPte) {
        PagesNeeded -= MI_COMPUTE_PAGES_SPANNED (PointerPte,
                                                 (LastPte - PointerPte) * sizeof (MMPTE));
    }
    else {
        PagesNeeded -= 1;
    }

    //
    // Charge resident available pages for all of the page directory
    // pages as they will not be paged until the VAD is freed.
    //

    MmLockPageableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)PagesNeeded > MI_NONPAGEABLE_MEMORY_AVAILABLE() - 20) {
        UNLOCK_PFN (OldIrql);
        MmUnlockPageableImageSection (ExPageLockHandle);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE);

    UNLOCK_PFN (OldIrql);

    UsedPageDirHandle = NULL;

    //
    // Fill in all the page table pages with the zero PTE.
    //

    LOCK_WS_UNSAFE (CurrentThread, Process);

    while (PointerPde <= LastPde) {

        if (MiIsPteOnPdeBoundary (PointerPde) || UsedPageDirHandle == NULL) {

            PointerPpe = MiGetPteAddress (PointerPde);

            //
            // Pointing to the next page table page, make
            // a page table page exist and make it valid.
            //
            // Note this ripples sharecounts through the paging hierarchy so
            // there is no need to up sharecounts to prevent trimming of the
            // page directory (and parent) page as making the page table
            // valid below does this automatically.
            //

            MiMakePdeExistAndMakeValid (PointerPpe, Process, MM_NOIRQL);

            //
            // Up the sharecount so the page directory page will not get
            // trimmed even if it has no currently valid entries.
            //

            PteContents = *PointerPpe;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            LOCK_PFN (OldIrql);
            Pfn1->u2.ShareCount += 1;
            UNLOCK_PFN (OldIrql);

            UsedPageDirHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPde));
        }

        ASSERT (PointerPde->u.Long == 0);

        //
        // Increment the count of non-zero page directory entries
        // for this page directory - even though this entry is still zero,
        // this is a special case.
        //

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageDirHandle);

        PointerPde += 1;
    }

    UNLOCK_WS_UNSAFE (CurrentThread, Process);

    MmUnlockPageableImageSection (ExPageLockHandle);

    return TRUE;
}

VOID
MiDeletePageDirectoriesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

/*++

Routine Description:

    This routine deletes page directory pages for a
    user-controlled large page range of pages.

    Even though PDEs may be zero in this range, UsedPageDir counts were
    incremented for these special ranges and must be decremented now.

Arguments:

    StartingAddress - Supplies the starting address of the range.

    EndingAddress - Supplies the ending address of the range.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, WorkingSetMutex and AddressCreation mutexes
    held.

--*/

{
    PVOID TempVa;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PagesNeeded;
    PEPROCESS CurrentProcess;
    PVOID UsedPageDirHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPpe;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif

    CurrentProcess = PsGetCurrentProcess();

    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);

    UsedPageDirHandle = MI_GET_USED_PTES_HANDLE (PointerPte);

    //
    // Each PDE is already zeroed - just delete the containing pages.
    //
    // Restore resident available pages for all of the page directory
    // pages as they can now be paged again.
    //

    PagesNeeded = MiResidentPagesForSpan (StartingAddress, EndingAddress);

    //
    // Subtract off the page table charge since none were used.
    //

    if (LastPte != PointerPte) {
        PagesNeeded -= MI_COMPUTE_PAGES_SPANNED (PointerPte,
                                                 (LastPte - PointerPte) * sizeof (MMPTE));
    }
    else {
        PagesNeeded -= 1;
    }

    LOCK_PFN (OldIrql);

    while (PointerPde <= LastPde) {

        ASSERT (PointerPde->u.Long == 0);

        PointerPde += 1;

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirHandle);

        if ((MiIsPteOnPdeBoundary (PointerPde)) || (PointerPde > LastPde)) {

            //
            // The virtual address is on a page directory parent boundary or
            // it is the last address in the entire range.
            //
            // If all the entries have been eliminated from the previous
            // page directory page, delete the page directory page itself.
            //

            PointerPpe = MiGetPteAddress (PointerPde - 1);
            ASSERT (PointerPpe->u.Hard.Valid == 1);

            //
            // Down the sharecount on the finished page directory page.
            //

            PteContents = *PointerPpe;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u2.ShareCount > 1);
            Pfn1->u2.ShareCount -= 1;

            //
            // If all the entries have been eliminated from the previous
            // page directory page, delete the page directory page itself.
            //

            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirHandle) == 0) {
                ASSERT (PointerPpe->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 4)
                UsedPageDirHandle = MI_GET_USED_PTES_HANDLE (PointerPde - 1);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                MiDeletePte (PointerPpe,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                if ((MiIsPteOnPpeBoundary (PointerPde)) || (PointerPde > LastPde)) {
    
                    PointerPxe = MiGetPteAddress (PointerPpe);
                    ASSERT (PointerPxe->u.Hard.Valid == 1);
    
                    //
                    // If all the entries have been eliminated from the previous
                    // page directory parent page, delete it too.
                    //
    
                    if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirHandle) == 0) {
                        ASSERT (PointerPxe->u.Long != 0);

                        UsedPageDirHandle = MI_GET_USED_PTES_HANDLE (PointerPpe);
                        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirHandle);

                        TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                        MiDeletePte (PointerPxe,
                                     TempVa,
                                     FALSE,
                                     CurrentProcess,
                                     NULL,
                                     NULL,
                                     OldIrql);
                    }
                }
#endif
            }

            if (PointerPde > LastPde) {
                break;
            }

            //
            // Release the PFN lock.  This prevents a single thread
            // from forcing other high priority threads from being
            // blocked while a large address range is deleted.
            //

            UNLOCK_PFN (OldIrql);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            UsedPageDirHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
            LOCK_PFN (OldIrql);
        }
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_FREE_USER_PAGE_TABLE);

    return;
}
#endif

