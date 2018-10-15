/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   mapview.c

Abstract:

    This module contains the routines which implement the
    NtMapViewOfSection service.

--*/

#include "mi.h"
#if defined(_WIN64)
#include <wow64t.h>
#endif

const ULONG MMDB = 'bDmM';
extern const ULONG MMVADKEY;

#if DBG
#define MI_BP_BADMAPS() TRUE
#else
ULONG MiStopBadMaps;
#define MI_BP_BADMAPS() (MiStopBadMaps & 0x1)
#endif

NTSTATUS
MiSetPageModified (
    IN PMMVAD Vad,
    IN PVOID Address
    );

extern LIST_ENTRY MmLoadedUserImageList;

ULONG   MiSubsectionsConvertedToDynamic;

#define ROUND_TO_PAGES64(Size)  (((UINT64)(Size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

MMSESSION   MmSession;

NTSTATUS
MiMapViewOfImageSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType,
    IN SIZE_T ImageCommitment
    );

NTSTATUS
MiMapViewOfDataSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG ProtectionMask,
    IN SIZE_T CommitSize,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    );

VOID
MiRemoveMappedPtes (
    IN PVOID BaseAddress,
    IN ULONG NumberOfPtes,
    IN PCONTROL_AREA ControlArea,
    IN PMMSUPPORT WorkingSetInfo
    );

NTSTATUS
MiMapViewInSystemSpace (
    IN PVOID Section,
    IN PMMSESSION Session,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );

NTSTATUS
MiUnmapViewInSystemSpace (
    IN PMMSESSION Session,
    IN PVOID MappedBase
    );

VOID
MiFillSystemPageDirectory (
    PVOID Base,
    SIZE_T NumberOfBytes
    );

VOID
MiLoadUserSymbols (
    IN PCONTROL_AREA ControlArea,
    IN PVOID StartingAddress,
    IN PEPROCESS Process
    );

#if DBG
VOID
VadTreeWalk (
    VOID
    );

VOID
MiDumpConflictingVad(
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad
    );
#endif //DBG


PVOID
MiCacheImageSymbols(
    IN PVOID ImageBase
    );

PVOID
MiInsertInSystemSpace (
    IN PMMSESSION Session,
    IN ULONG SizeIn64k,
    IN PCONTROL_AREA ControlArea
    );

ULONG
MiRemoveFromSystemSpace (
    IN PMMSESSION Session,
    IN PVOID Base,
    OUT PCONTROL_AREA *ControlArea
    );

VOID
MiInsertPhysicalViewAndRefControlArea (
    IN PEPROCESS Process,
    IN PCONTROL_AREA ControlArea,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtMapViewOfSection)
#pragma alloc_text(PAGE,MmMapViewOfSection)
#pragma alloc_text(PAGE,MmSecureVirtualMemory)
#pragma alloc_text(PAGE,MmUnsecureVirtualMemory)
#pragma alloc_text(PAGE,MiUnsecureVirtualMemory)
#pragma alloc_text(PAGE,MiCacheImageSymbols)
#pragma alloc_text(PAGE,NtAreMappedFilesTheSame)
#pragma alloc_text(PAGE,MiLoadUserSymbols)
#pragma alloc_text(PAGE,MiInsertInSystemSpace)
#pragma alloc_text(PAGE,MmMapViewInSystemSpace)
#pragma alloc_text(PAGE,MmMapViewInSessionSpace)
#pragma alloc_text(PAGE,MiUnmapViewInSystemSpace)
#pragma alloc_text(PAGE,MmUnmapViewInSystemSpace)
#pragma alloc_text(PAGE,MmUnmapViewInSessionSpace)
#pragma alloc_text(PAGE,MiMapViewInSystemSpace)
#pragma alloc_text(PAGE,MiRemoveFromSystemSpace)
#pragma alloc_text(PAGE,MiInitializeSystemSpaceMap)
#pragma alloc_text(PAGE,MiFreeSessionSpaceMap)
#pragma alloc_text(PAGE,MmGetSessionMappedViewInformation)
#if DBG
#pragma alloc_text(PAGE,MiDumpConflictingVad)
#endif
#endif


NTSTATUS
NtMapViewOfSection(
    __in HANDLE SectionHandle,
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __in SIZE_T CommitSize,
    __inout_opt PLARGE_INTEGER SectionOffset,
    __inout PSIZE_T ViewSize,
    __in SECTION_INHERIT InheritDisposition,
    __in ULONG AllocationType,
    __in WIN32_PROTECTION_MASK Win32Protect
    )

/*++

Routine Description:

    This function maps a view in the specified subject process to
    the section object.

Arguments:

    SectionHandle - Supplies an open handle to a section object.

    ProcessHandle - Supplies an open handle to a process object.

    BaseAddress - Supplies a pointer to a variable that will receive
                  the base address of the view. If the initial value
                  of this argument is not null, then the view will
                  be allocated starting at the specified virtual
                  address rounded down to the next 64kb address
                  boundary. If the initial value of this argument is
                  null, then the operating system will determine
                  where to allocate the view using the information
                  specified by the ZeroBits argument value and the
                  section allocation attributes (i.e. based and
                  tiled).
        
    ZeroBits - Supplies the number of high order address bits that
               must be zero in the base address of the section
               view. The value of this argument must be less than
               or equal to the maximum number of zero bits and is only
               used when memory management determines where to allocate
               the view (i.e. when BaseAddress is null).
        
               If ZeroBits is zero, then no zero bit constraints are applied.

               If ZeroBits is greater than 0 and less than 32, then it is
               the number of leading zero bits from bit 31.  Bits 63:32 are
               also required to be zero.  This retains compatibility
               with 32-bit systems.
                
               If ZeroBits is greater than 32, then it is considered as
               a mask and the number of leading zeroes are counted out
               in the mask.  This then becomes the zero bits argument.

    CommitSize - Supplies the size of the initially committed region
                 of the view in bytes. This value is rounded up to
                 the next host page size boundary.

    SectionOffset - Supplies the offset from the beginning of the
                    section to the view in bytes. This value is
                    rounded down to the next host page size boundary.

    ViewSize - Supplies a pointer to a variable that will receive
               the actual size in bytes of the view. If the value
               of this argument is zero, then a view of the
               section will be mapped starting at the specified
               section offset and continuing to the end of the
               section. Otherwise the initial value of this
               argument specifies the size of the view in bytes
               and is rounded up to the next host page size
               boundary.
        
    InheritDisposition - Supplies a value that specifies how the
                         view is to be shared by a child process created
                         with a create process operation.

        InheritDisposition Values

         ViewShare - Inherit view and share a single copy
                     of the committed pages with a child process
                     using the current protection value.

         ViewUnmap - Do not map the view into a child process.

    AllocationType - Supplies the type of allocation.

         MEM_TOP_DOWN
         MEM_DOS_LIM
         MEM_LARGE_PAGES
         MEM_RESERVE - for file mapped sections only.

    Win32Protect - Supplies the protection desired for the region of
                   initially committed pages.

        Win32Protect Values

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
                          the region of committed pages is allowed. If
                          write access to the underlying section is
                          allowed, then a single copy of the pages are
                          shared. Otherwise the pages are shared read
                          only/copy on write.

Return Value:

    Various NTSTATUS codes.

--*/

{
    PSECTION Section;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PVOID CapturedBase;
    SIZE_T CapturedViewSize;
    LARGE_INTEGER CapturedOffset;
    ULONGLONG HighestPhysicalAddressInPfnDatabase;
    ACCESS_MASK DesiredSectionAccess;
    ULONG ProtectMaskForAccess;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    PAGED_CODE();

    //
    // Check the zero bits argument for correctness.
    //

#if defined (_WIN64)

    if (ZeroBits >= 32) {

        //
        // ZeroBits is a mask instead of a count.  Translate it to a count now.
        //

        ZeroBits = 64 - RtlFindMostSignificantBit (ZeroBits) - 1;
    }
    else if (ZeroBits) {
        ZeroBits += 32;
    }

#endif

    if (ZeroBits > MM_MAXIMUM_ZERO_BITS) {
        return STATUS_INVALID_PARAMETER_4;
    }

    //
    // Check the inherit disposition flags.
    //

    if ((InheritDisposition > ViewUnmap) ||
        (InheritDisposition < ViewShare)) {
        return STATUS_INVALID_PARAMETER_8;
    }

    //
    // Check the allocation type field.
    //

#ifdef i386

    //
    // Only allow DOS_LIM support for i386.  The MEM_DOS_LIM flag allows
    // map views of data sections to be done on 4k boundaries rather
    // than 64k boundaries.
    //

    if ((AllocationType & ~(MEM_TOP_DOWN | MEM_LARGE_PAGES | MEM_DOS_LIM |
           SEC_NO_CHANGE | MEM_RESERVE)) != 0) {
        return STATUS_INVALID_PARAMETER_9;
    }
#else
    if ((AllocationType & ~(MEM_TOP_DOWN | MEM_LARGE_PAGES |
           SEC_NO_CHANGE | MEM_RESERVE)) != 0) {
        return STATUS_INVALID_PARAMETER_9;
    }

#endif //i386

    //
    // Check the protection field.
    //

    ProtectMaskForAccess = MiMakeProtectionMask (Win32Protect);
    if (ProtectMaskForAccess == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ProtectMaskForAccess = ProtectMaskForAccess & 0x7;

    DesiredSectionAccess = MmMakeSectionAccess[ProtectMaskForAccess];

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
            ProbeForWriteUlong_ptr (ViewSize);
        }

        if (ARGUMENT_PRESENT (SectionOffset)) {
            if (PreviousMode != KernelMode) {
                ProbeForWriteSmallStructure (SectionOffset,
                                             sizeof(LARGE_INTEGER),
                                             PROBE_ALIGNMENT (LARGE_INTEGER));
            }
            CapturedOffset = *SectionOffset;
        }
        else {
            ZERO_LARGE (CapturedOffset);
        }

        //
        // Capture the base address.
        //

        CapturedBase = *BaseAddress;

        //
        // Capture the region size.
        //

        CapturedViewSize = *ViewSize;

    } except (ExSystemExceptionFilter()) {

        //
        // If an exception occurs during the probe or capture
        // of the initial values, then handle the exception and
        // return the exception code as the status value.
        //

        return GetExceptionCode();
    }

    //
    // Make sure the specified starting and ending addresses are
    // within the user part of the virtual address space.
    //

    if (CapturedBase > MM_HIGHEST_VAD_ADDRESS) {

        //
        // Invalid base address.
        //

        return STATUS_INVALID_PARAMETER_3;
    }

    if (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS - (ULONG_PTR)CapturedBase) <
                                                        CapturedViewSize) {

        //
        // Invalid region size;
        //

        return STATUS_INVALID_PARAMETER_3;

    }

    if (((ULONG_PTR)CapturedBase + CapturedViewSize) > ((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits)) {

        //
        // Desired Base and zero_bits conflict.
        //

        return STATUS_INVALID_PARAMETER_4;
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
    // Reference the section object, if a view is mapped to the section
    // object, the object is not dereferenced as the virtual address
    // descriptor contains a pointer to the section object.
    //

    Status = ObReferenceObjectByHandle (SectionHandle,
                                        DesiredSectionAccess,
                                        MmSectionObjectType,
                                        PreviousMode,
                                        (PVOID *)&Section,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        goto ErrorReturn1;
    }

    //
    // Check to see if this the section backs physical memory, if
    // so DON'T align the offset on a 64K boundary, just a 4k boundary.
    //

    if (Section->Segment->ControlArea->u.Flags.PhysicalMemory) {
        CapturedOffset.LowPart = CapturedOffset.LowPart & ~(PAGE_SIZE - 1);

        //
        // No usermode mappings past the end of the PFN database are allowed.
        // Address wrap is checked in the common path.
        //

        if (PreviousMode != KernelMode) {

            HighestPhysicalAddressInPfnDatabase = (ULONGLONG)MmHighestPhysicalPage << PAGE_SHIFT;

            if ((ULONGLONG)(CapturedOffset.QuadPart + CapturedViewSize) > HighestPhysicalAddressInPfnDatabase) {
                Status = STATUS_INVALID_PARAMETER_6;
                goto ErrorReturn;
            }
        }

    }
    else {

        //
        // Make sure alignments are correct for specified address
        // and offset into the file.
        //

        if ((AllocationType & MEM_DOS_LIM) == 0) {
            if (((ULONG_PTR)CapturedBase & (X64K - 1)) != 0) {
                Status = STATUS_MAPPED_ALIGNMENT;
                goto ErrorReturn;
            }

            if ((ARGUMENT_PRESENT (SectionOffset)) &&
                ((CapturedOffset.LowPart & (X64K - 1)) != 0)) {
                Status = STATUS_MAPPED_ALIGNMENT;
                goto ErrorReturn;
            }
        }
    }

    Status = MmMapViewOfSection ((PVOID)Section,
                                 Process,
                                 &CapturedBase,
                                 ZeroBits,
                                 CommitSize,
                                 &CapturedOffset,
                                 &CapturedViewSize,
                                 InheritDisposition,
                                 AllocationType,
                                 Win32Protect);

    if (!NT_SUCCESS(Status) ) {

        if ((Status == STATUS_CONFLICTING_ADDRESSES) &&
            (Section->Segment->ControlArea->u.Flags.Image) &&
            (Process == CurrentProcess)) {

            DbgkMapViewOfSection (Section,
                                  CapturedBase,
                                  CapturedOffset.LowPart,
                                  CapturedViewSize);
        }
        goto ErrorReturn;
    }

    //
    // Any time the current process maps an image file,
    // a potential debug event occurs. DbgkMapViewOfSection
    // handles these events.
    //

    if ((Section->Segment->ControlArea->u.Flags.Image) &&
        (Process == CurrentProcess)) {

        if (Status != STATUS_IMAGE_NOT_AT_BASE) {
            DbgkMapViewOfSection (Section,
                                  CapturedBase,
                                  CapturedOffset.LowPart,
                                  CapturedViewSize);
        }
    }

    //
    // Establish an exception handler and write the size and base address.
    //

    try {

        *ViewSize = CapturedViewSize;
        *BaseAddress = CapturedBase;

        if (ARGUMENT_PRESENT(SectionOffset)) {
            *SectionOffset = CapturedOffset;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

ErrorReturn:
    ObDereferenceObject (Section);

ErrorReturn1:
    ObDereferenceObject (Process);
    return Status;
}

NTSTATUS
MmMapViewOfSection (
    __in PVOID SectionToMap,
    __in PEPROCESS Process,
    __deref_inout_bcount(*CapturedViewSize) PVOID *CapturedBase,
    __in ULONG_PTR ZeroBits,
    __in SIZE_T CommitSize,
    __inout PLARGE_INTEGER SectionOffset,
    __inout PSIZE_T CapturedViewSize,
    __in SECTION_INHERIT InheritDisposition,
    __in ULONG AllocationType,
    __in WIN32_PROTECTION_MASK Win32Protect
    )

/*++

Routine Description:

    This function maps a view in the specified subject process to
    the section object.

    This function is a kernel mode interface to allow LPC to map
    a section given the section pointer to map.

    This routine assumes all arguments have been probed and captured.

    ********************************************************************
    ********************************************************************
    ********************************************************************

    NOTE:

    CapturedViewSize, SectionOffset, and CapturedBase must be
    captured in non-paged system space (i.e., kernel stack).

    ********************************************************************
    ********************************************************************
    ********************************************************************

Arguments:

    SectionToMap - Supplies a pointer to the section object.

    Process - Supplies a pointer to the process object.

    BaseAddress - Supplies a pointer to a variable that will receive
                  the base address of the view. If the initial value
                  of this argument is not NULL, then the view will
                  be allocated starting at the specified virtual
                  address rounded down to the next 64kb address
                  boundary. If the initial value of this argument is
                  NULL, then the operating system will determine
                  where to allocate the view using the information
                  specified by the ZeroBits argument value and the
                  section allocation attributes (i.e. based and
                  tiled).

    ZeroBits - Supplies the number of high order address bits that
               must be zero in the base address of the section
               view. The value of this argument must be less than
               21 and is only used when the operating system
               determines where to allocate the view (i.e. when
               BaseAddress is NULL).

    CommitSize - Supplies the size of the initially committed region
                 of the view in bytes. This value is rounded up to
                 the next host page size boundary.

    SectionOffset - Supplies the offset from the beginning of the
                    section to the view in bytes. This value is
                    rounded down to the next host page size boundary.

    ViewSize - Supplies a pointer to a variable that will receive
               the actual size in bytes of the view. If the value
               of this argument is zero, then a view of the
               section will be mapped starting at the specified
               section offset and continuing to the end of the
               section. Otherwise the initial value of this
               argument specifies the size of the view in bytes
               and is rounded up to the next host page size boundary.

    InheritDisposition - Supplies a value that specifies how the
                         view is to be shared by a child process created
                         with a create process operation.

    AllocationType - Supplies the type of allocation.

    Win32Protect - Supplies the protection desired for the region of
                   initially committed pages.

Return Value:

    NTSTATUS.

--*/
{
    KAPC_STATE ApcState;
    LOGICAL Attached;
    PSECTION Section;
    PCONTROL_AREA ControlArea;
    ULONG ProtectionMask;
    NTSTATUS status;
    SIZE_T ImageCommitment;
    LARGE_INTEGER TempViewSize;

    PAGED_CODE();

    Attached = FALSE;

    Section = (PSECTION)SectionToMap;

    if (Section->u.Flags.Image == 0) {

        //
        // This is not an image section, make sure the section page
        // protection is compatible with the specified page protection.
        //

        if (!MiIsProtectionCompatible (Section->InitialPageProtection,
                                       Win32Protect)) {
            return STATUS_SECTION_PROTECTION;
        }
    }

    //
    // Check to make sure the view size plus the offset is less
    // than the size of the section.
    //

    if ((ULONGLONG) (SectionOffset->QuadPart + *CapturedViewSize) <
        (ULONGLONG)SectionOffset->QuadPart) {

        return STATUS_INVALID_VIEW_SIZE;
    }

    if (((ULONGLONG) (SectionOffset->QuadPart + *CapturedViewSize) >
                 (ULONGLONG)Section->SizeOfSection.QuadPart) &&
        ((AllocationType & MEM_RESERVE) == 0)) {

        return STATUS_INVALID_VIEW_SIZE;
    }

    if (*CapturedViewSize == 0) {

        //
        // Set the view size to be size of the section less the offset.
        //

        TempViewSize.QuadPart = Section->SizeOfSection.QuadPart -
                                                SectionOffset->QuadPart;

        *CapturedViewSize = (SIZE_T)TempViewSize.QuadPart;

        if (

#if !defined(_WIN64)

            (TempViewSize.HighPart != 0) ||

#endif

            (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS - (ULONG_PTR)*CapturedBase) <
                                                        *CapturedViewSize)) {

            //
            // Invalid region size;
            //

            return STATUS_INVALID_VIEW_SIZE;
        }
    }

    //
    // Check commit size.
    //

    if ((CommitSize > *CapturedViewSize) &&
        ((AllocationType & MEM_RESERVE) == 0)) {

        return STATUS_INVALID_PARAMETER_5;
    }

    //
    // Check to make sure the section is not smaller than the view size.
    //

    if ((LONGLONG)*CapturedViewSize > Section->SizeOfSection.QuadPart) {
        if ((AllocationType & MEM_RESERVE) == 0) {
            return STATUS_INVALID_VIEW_SIZE;
        }
    }

    if (AllocationType & MEM_RESERVE) {
        if (((Section->InitialPageProtection & PAGE_READWRITE) |
            (Section->InitialPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

            return STATUS_SECTION_PROTECTION;
        }
    }

    if (Section->u.Flags.NoCache) {
        Win32Protect |= PAGE_NOCACHE;
        Win32Protect &= ~PAGE_WRITECOMBINE;
    }

    if (Section->u.Flags.WriteCombined) {
        Win32Protect |= PAGE_WRITECOMBINE;
        Win32Protect &= ~PAGE_NOCACHE;
    }

    //
    // Check the protection field.
    //

    ProtectionMask = MiMakeProtectionMask (Win32Protect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ControlArea = Section->Segment->ControlArea;

    //
    // If the specified process is not the current process, attach
    // to the specified process.
    //

    if (PsGetCurrentProcess() != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    //
    // Get the address creation mutex to block multiple threads
    // creating or deleting address space at the same time.
    //

    LOCK_ADDRESS_SPACE (Process);

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }
    
    //
    // Map the view base on the type.
    //

    if (ControlArea->u.Flags.PhysicalMemory) {

        status = MiMapViewOfPhysicalSection (ControlArea,
                                             Process,
                                             CapturedBase,
                                             SectionOffset,
                                             CapturedViewSize,
                                             ProtectionMask,
                                             ZeroBits,
                                             AllocationType);
    }
    else if (ControlArea->u.Flags.Image) {
        if (AllocationType & MEM_RESERVE) {
            status = STATUS_INVALID_PARAMETER_9;
        }
        else if (Win32Protect & PAGE_WRITECOMBINE) {
            status = STATUS_INVALID_PARAMETER_10;
        }
        else {

            ImageCommitment = Section->Segment->u1.ImageCommitment;

            status = MiMapViewOfImageSection (ControlArea,
                                              Process,
                                              CapturedBase,
                                              SectionOffset,
                                              CapturedViewSize,
                                              Section,
                                              InheritDisposition,
                                              ZeroBits,
                                              AllocationType,
                                              ImageCommitment);
        }
    }
    else {

        //
        // Not an image section, therefore it is a data section.
        //

        if (Win32Protect & PAGE_WRITECOMBINE) {
            status = STATUS_INVALID_PARAMETER_10;
        }
        else {
            
            status = MiMapViewOfDataSection (ControlArea,
                                             Process,
                                             CapturedBase,
                                             SectionOffset,
                                             CapturedViewSize,
                                             Section,
                                             InheritDisposition,
                                             ProtectionMask,
                                             CommitSize,
                                             ZeroBits,
                                             AllocationType);
        }
    }

ErrorReturn:
    UNLOCK_ADDRESS_SPACE (Process);

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    return status;
}

VOID
MiInsertPhysicalViewAndRefControlArea (
    IN PEPROCESS Process,
    IN PCONTROL_AREA ControlArea,
    IN PMI_PHYSICAL_VIEW PhysicalView
    )

/*++

Routine Description:

    This is a nonpaged helper routine to insert a physical view and reference
    the control area accordingly.

Arguments:

    Process - Supplies a pointer to the process.

    ControlArea - Supplies a pointer to the control area.

    PhysicalView - Supplies a pointer to the physical view.

Return Value:

    None.

Environment:

    Kernel Mode, address creation and working set mutex held.

--*/

{
    KIRQL OldIrql;

    //
    // Increment the count of the number of views for the
    // section object.  This requires the PFN lock to be held.
    //

    ASSERT (PhysicalView->Vad->u.VadFlags.VadType == VadDevicePhysicalMemory);

    ASSERT (Process->PhysicalVadRoot != NULL);

    MiInsertNode ((PMMADDRESS_NODE)PhysicalView, Process->PhysicalVadRoot);

    LOCK_PFN (OldIrql);

    ControlArea->NumberOfMappedViews += 1;
    ControlArea->NumberOfUserReferences += 1;

    ASSERT (ControlArea->NumberOfSectionReferences != 0);

    UNLOCK_PFN (OldIrql);
}

//
// Nonpaged wrapper.
//
LOGICAL
MiCheckCacheAttributes (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER BadFrameStart;
    PFN_NUMBER BadFrameEnd;
    LOGICAL AttemptedMapOfUnownedFrame;
#if DBG
#define BACKTRACE_LENGTH 8
    ULONG i;
    ULONG Hash;
    PVOID StackTrace [BACKTRACE_LENGTH];
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PUNICODE_STRING BadDriverName;
#endif

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    BadFrameStart = 0;
    BadFrameEnd = 0;
    AttemptedMapOfUnownedFrame = FALSE;

    LOCK_PFN (OldIrql);

    do {

        if (MI_IS_PFN (PageFrameIndex)) {

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            switch (Pfn1->u3.e1.CacheAttribute) {

                case MiCached:
                    if (CacheAttribute != MiCached) {
                        UNLOCK_PFN (OldIrql);
                        return FALSE;
                    }
                    break;

                case MiNonCached:
                    if (CacheAttribute != MiNonCached) {
                        UNLOCK_PFN (OldIrql);
                        return FALSE;
                    }
                    break;

                case MiWriteCombined:
                    if (CacheAttribute != MiWriteCombined) {
                        UNLOCK_PFN (OldIrql);
                        return FALSE;
                    }
                    break;

                case MiNotMapped:
                    if (AttemptedMapOfUnownedFrame == FALSE) {
                        BadFrameStart = PageFrameIndex;
                        AttemptedMapOfUnownedFrame = TRUE;
                    }
                    BadFrameEnd = PageFrameIndex;
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }
        }
        PageFrameIndex += 1;
        NumberOfPages -= 1;
    } while (NumberOfPages != 0);

    UNLOCK_PFN (OldIrql);

    if (AttemptedMapOfUnownedFrame == TRUE) {

#if DBG

        BadDriverName = NULL;

        RtlZeroMemory (StackTrace, sizeof (StackTrace));

        RtlCaptureStackBackTrace (1, BACKTRACE_LENGTH, StackTrace, &Hash);

        for (i = 0; i < BACKTRACE_LENGTH; i += 1) {

            if (StackTrace[i] <= MM_HIGHEST_USER_ADDRESS) {
                break;
            }

            DataTableEntry = MiLookupDataTableEntry (StackTrace[i], FALSE);

            if ((DataTableEntry != NULL) && ((PVOID)DataTableEntry != (PVOID)PsLoadedModuleList.Flink)) {
                //
                // Found the bad caller.
                //

                BadDriverName = &DataTableEntry->FullDllName;
            }
        }

        if (BadDriverName != NULL) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                  "*******************************************************************************\n"
                  "*\n"
                  "* %wZ is mapping physical memory %p->%p\n"
                  "* that it does not own.  This can cause internal CPU corruption.\n"
                  "*\n"
                  "*******************************************************************************\n",
                BadDriverName,
                BadFrameStart << PAGE_SHIFT,
                (BadFrameEnd << PAGE_SHIFT) | (PAGE_SIZE - 1));
        }
        else {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                  "*******************************************************************************\n"
                  "*\n"
                  "* A driver is mapping physical memory %p->%p\n"
                  "* that it does not own.  This can cause internal CPU corruption.\n"
                  "*\n"
                  "*******************************************************************************\n",
                BadFrameStart << PAGE_SHIFT,
                (BadFrameEnd << PAGE_SHIFT) | (PAGE_SIZE - 1));
        }

#else
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
              "*******************************************************************************\n"
              "*\n"
              "* A driver is mapping physical memory %p->%p\n"
              "* that it does not own.  This can cause internal CPU corruption.\n"
              "* A checked build will stop in the kernel debugger\n"
              "* so this problem can be fully debugged.\n"
              "*\n"
              "*******************************************************************************\n",
            BadFrameStart << PAGE_SHIFT,
            (BadFrameEnd << PAGE_SHIFT) | (PAGE_SIZE - 1));
#endif

        if (MI_BP_BADMAPS()) {
            DbgBreakPoint ();
        }
    }

    return TRUE;
}

NTSTATUS
MiMapViewOfPhysicalSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN ULONG ProtectionMask,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    )

/*++

Routine Description:

    This routine maps the specified physical section into the
    specified process's address space.

Arguments:

    see MmMapViewOfSection above...

    ControlArea - Supplies the control area for the section.

    Process - Supplies the process pointer which is receiving the section.

    ProtectionMask - Supplies the initial page protection-mask.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, address creation mutex held.

--*/

{
    KIRQL OldIrql;
    PMMVAD_LONG Vad;
    PETHREAD Thread;
    CSHORT IoMapping;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PVOID HighestUserAddress;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LastPde;
    MMPTE TempPte;
    PMMPFN Pfn2;
    SIZE_T PhysicalViewSize;
    ULONG_PTR Alignment;
    PVOID UsedPageTableHandle;
    PMI_PHYSICAL_VIEW PhysicalView;
    NTSTATUS Status;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER StartingPageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MEMORY_CACHING_TYPE CacheType;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    LOGICAL TryLargePages;

    //
    // Physical memory section.
    //

    if (AllocationType & (MEM_RESERVE | MEM_LARGE_PAGES)) {
        return STATUS_INVALID_PARAMETER_9;
    }

    if ((MI_IS_GUARD (ProtectionMask)) ||
        (ProtectionMask == MM_NOACCESS)) {

        //
        // These cannot be represented with valid PTEs so reject these
        // requests.  Note copy on write doesn't make any sense for these
        // either, but those can be safely rejected if/when the application
        // really does try to write to the mapping.
        //

        return STATUS_INVALID_PAGE_PROTECTION;
    }

    TryLargePages = FALSE;

    if (((SectionOffset->QuadPart & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1)) == 0) &&
#ifdef _X86_
        (KeFeatureBits & KF_LARGE_PAGE) &&
#endif
        (ZeroBits == 0) &&
        ((*CapturedViewSize & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1)) == 0)) {
    }

    Alignment = X64K;

    if (*CapturedBase == NULL) {

        //
        // Attempt to locate address space starting on a 64k boundary.
        //

#if !defined (_MI_MORE_THAN_4GB_)
        ASSERT (SectionOffset->HighPart == 0);
#endif

        if (TryLargePages == TRUE) {

            PhysicalViewSize = *CapturedViewSize;
    
            //
            // Check whether the registry indicates that all applications
            // should be given virtual address ranges from the highest
            // address downwards in order to test 3GB-aware apps on 32-bit
            // machines and 64-bit apps on NT64.
            //
    
            if ((AllocationType & MEM_TOP_DOWN) || (Process->VmTopDown == 1)) {
    
                HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
    
                Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                      PhysicalViewSize,
                                                      HighestUserAddress,
                                                      MM_MINIMUM_VA_FOR_LARGE_PAGE,
                                                      &StartingAddress);
            }
            else {
    
                Status = MiFindEmptyAddressRange (PhysicalViewSize,
                                                  MM_MINIMUM_VA_FOR_LARGE_PAGE,
                                                  (ULONG)ZeroBits,
                                                  &StartingAddress);
            }
    
            if (NT_SUCCESS (Status)) {

                EndingAddress = (PVOID)((ULONG_PTR)StartingAddress +
                                        PhysicalViewSize - 1L);
                goto GotRange;
            }
        }

        TryLargePages = FALSE;

        PhysicalViewSize = *CapturedViewSize +
                               (SectionOffset->LowPart & (X64K - 1));

        //
        // Check whether the registry indicates that all applications
        // should be given virtual address ranges from the highest
        // address downwards in order to test 3GB-aware apps on 32-bit
        // machines and 64-bit apps on NT64.
        //

        if ((AllocationType & MEM_TOP_DOWN) || (Process->VmTopDown == 1)) {

            if (ZeroBits != 0) {
                HighestUserAddress = (PVOID)((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits);
                if (HighestUserAddress > MM_HIGHEST_VAD_ADDRESS) {
                    HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
                }
            }
            else {
                HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
            }

            Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                  PhysicalViewSize,
                                                  HighestUserAddress,
                                                  Alignment,
                                                  &StartingAddress);
        }
        else {

            Status = MiFindEmptyAddressRange (PhysicalViewSize,
                                              Alignment,
                                              (ULONG)ZeroBits,
                                              &StartingAddress);
        }

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                PhysicalViewSize - 1L) | (PAGE_SIZE - 1L));
        StartingAddress = (PVOID)((ULONG_PTR)StartingAddress +
                                     (SectionOffset->LowPart & (X64K - 1)));

        if (ZeroBits > 0) {
            if (EndingAddress > (PVOID)((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits)) {
                return STATUS_NO_MEMORY;
            }
        }
    }
    else {

        //
        // Check to make sure the specified base address to ending address
        // is currently unused.
        //

        StartingAddress = (PVOID)((ULONG_PTR)MI_64K_ALIGN(*CapturedBase) +
                                    (SectionOffset->LowPart & (X64K - 1)));
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                *CapturedViewSize - 1L) | (PAGE_SIZE - 1L));

        if (MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress) == TRUE) {
            return STATUS_CONFLICTING_ADDRESSES;
        }

        if ((ULONG_PTR)*CapturedBase & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1)) {
            TryLargePages = FALSE;
        }
    }

GotRange:

    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    PointerPde = MiGetPdeAddress (StartingAddress);

    //
    // If a noncachable mapping is requested, none of the physical pages in the
    // range can reside in a large cached page.  Otherwise we would be
    // creating an incoherent overlapping TB entry as the same physical
    // page would be mapped by 2 different TB entries with different
    // cache attributes.
    //

    StartingPageFrameIndex = (PFN_NUMBER) (SectionOffset->QuadPart >> PAGE_SHIFT);

    //
    // Build the PTE.
    //

    MI_MAKE_VALID_USER_PTE (TempPte,
                            StartingPageFrameIndex,
                            ProtectionMask,
                            PointerPte);

    if (TempPte.u.Hard.Write) {
        MI_SET_PTE_DIRTY (TempPte);
    }

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    //
    // See if the mapping is to I/O space.  If so, the cache attribute may
    // need to be updated to work around hardware platform problems.
    //

    IoMapping = 1;

    if (MI_IS_PFN (StartingPageFrameIndex)) {
        IoMapping = 0;
    }

    //
    // For non I/O space mappings, we already have the correct attributes
    // in the PTE.  But we still need to compute the CacheAttribute variable
    // so we'll know if the cache needs to be swept, etc.
    //

    CacheType = MmCached;

    if (MI_IS_WRITECOMBINE (ProtectionMask)) {
        CacheType = MmWriteCombined;
    }
    else if (MI_IS_NOCACHE (ProtectionMask)) {
        CacheType = MmNonCached;
    }

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

    NumberOfPages = LastPte - PointerPte + 1;

    if (CacheAttribute != MiCached) {

        if (CacheAttribute == MiWriteCombined) {
            MI_SET_PTE_WRITE_COMBINE (TempPte);
        }
        else if (CacheAttribute == MiNonCached) {
            MI_DISABLE_CACHING (TempPte);
        }

        PageFrameIndex = StartingPageFrameIndex;

        while (PageFrameIndex < StartingPageFrameIndex + NumberOfPages) {
            if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {
                MiNonCachedCollisions += 1;
                return STATUS_CONFLICTING_ADDRESSES;
            }
            PageFrameIndex += 1;
        }
    }

    //
    // An unoccupied address range has been found, build the virtual
    // address descriptor to describe this range.
    //

    //
    // The working set mutex synchronizes the allocation of the
    // EPROCESS PhysicalVadRoot.  This table root is not deleted until
    // the process exits.
    //

    if ((Process->PhysicalVadRoot == NULL) &&
        (MiCreatePhysicalVadRoot (Process, FALSE) == NULL)) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Attempt to allocate the pool and charge quota during the Vad insertion.
    //

    PhysicalView = (PMI_PHYSICAL_VIEW)ExAllocatePoolWithTag (NonPagedPool,
                                                             sizeof(MI_PHYSICAL_VIEW),
                                                             MI_PHYSICAL_VIEW_KEY);
    if (PhysicalView == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Vad = (PMMVAD_LONG)ExAllocatePoolWithTag (NonPagedPool,
                                              sizeof(MMVAD_LONG),
                                              'ldaV');
    if (Vad == NULL) {
        ExFreePool (PhysicalView);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Vad, sizeof(MMVAD_LONG));
    Vad->StartingVpn = MI_VA_TO_VPN (StartingAddress);
    Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);
    Vad->ControlArea = ControlArea;
    Vad->u2.VadFlags2.Inherit = MM_VIEW_UNMAP;
    Vad->u2.VadFlags2.LongVad = 1;
    Vad->u.VadFlags.VadType = VadDevicePhysicalMemory;
    Vad->u.VadFlags.Protection = ProtectionMask;

    PhysicalView->Vad = (PMMVAD) Vad;
    PhysicalView->StartingVpn = Vad->StartingVpn;
    PhysicalView->EndingVpn = Vad->EndingVpn;
    PhysicalView->VadType = VadDevicePhysicalMemory;

    //
    // Set the last contiguous PTE field in the Vad to the page frame
    // number of the starting physical page.
    //

    Vad->LastContiguousPte = (PMMPTE) StartingPageFrameIndex;

    Vad->FirstPrototypePte = (PMMPTE) StartingPageFrameIndex;

    ASSERT (Vad->FirstPrototypePte <= Vad->LastContiguousPte);

    //
    // Ensure no page frame cache attributes conflict.
    //

    if (MiCheckCacheAttributes (StartingPageFrameIndex, NumberOfPages, CacheAttribute) == FALSE) {
        Status = STATUS_CONFLICTING_ADDRESSES;
        goto Failure1;
    }

    //
    // Insert the VAD.  This could fail due to quota charges.
    //

    Thread = PsGetCurrentThread ();

    Status = MiInsertVadCharges ((PMMVAD) Vad, Process);

    if (!NT_SUCCESS (Status)) {

Failure1:
        ExFreePool (PhysicalView);

        //
        // The pool allocation succeeded but the quota charge failed,
        // deallocate the pool and return an error.
        //

        ExFreePool (Vad);
        return Status;
    }

    LOCK_WS_UNSAFE (Thread, Process);

    MiInsertVad ((PMMVAD) Vad, Process);

    //
    // Build the PTEs in the address space.
    //

    MI_PREPARE_FOR_NONCACHED (CacheAttribute);

    if (TryLargePages == TRUE) {

#if (_MI_PAGING_LEVELS >= 3)

        //
        // Temporarily make the VAD protection no access.  This allows
        // us to safely release the working set mutex while trying to
        // find contiguous memory to fill the large page range.
        // If another thread tries to access the large page VA range
        // before we find (and insert) a contiguous chunk, the thread
        // will get an AV.
        //

        Vad->u.VadFlags.Protection = MM_NOACCESS;

        UNLOCK_WS_UNSAFE (Thread, Process);

        //
        // Lock down the page directory page(s) as these entries are not
        // going to be paged.
        //

        Status = MiAllocateLargePages (StartingAddress,
                                       EndingAddress,
                                       ProtectionMask,
                                       TRUE);

        if (!NT_SUCCESS (Status)) {

            PMMVAD NextVad;
            PMMVAD PreviousVad;

            PreviousVad = MiGetPreviousVad (Vad);
            NextVad = MiGetNextVad (Vad);

            MiRemoveVadCharges ((PMMVAD) Vad, Process);

            //
            // Return commitment for page table pages if possible.
            //

            MiReturnPageTablePageCommitment (StartingAddress,
                                             EndingAddress,
                                             Process,
                                             PreviousVad,
                                             NextVad);

            LOCK_WS_UNSAFE (Thread, Process);

            MiRemoveVad ((PMMVAD) Vad, Process);

            UNLOCK_WS (Thread, Process);

            goto Failure1;
        }

        LOCK_WS_UNSAFE (Thread, Process);

        Vad->u.VadFlags.Protection = ProtectionMask;

#endif

        MI_SET_ACCESSED_IN_PTE (&TempPte, 1);

        MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPte);

        LastPde = PointerPde + NumberOfPages / PTE_PER_PAGE;

        while (PointerPde < LastPde) {
    
            ASSERT (PointerPde->u.Long == 0);
    
            MI_WRITE_VALID_PTE (PointerPde, TempPte);
    
            TempPte.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
    
            PointerPde += 1;
        }
    }
    else {
    
        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
    
        Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
    
        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);
    
        while (PointerPte <= LastPte) {
    
            if (MiIsPteOnPdeBoundary (PointerPte)) {
    
                PointerPde = MiGetPteAddress (PointerPte);
    
                MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
    
                Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));
            }
    
            //
            // Increment the count of non-zero page table entries for this
            // page table and the number of private pages for the process.
            //
    
            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
    
            ASSERT (PointerPte->u.Long == 0);
    
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
    
            LOCK_PFN (OldIrql);
            Pfn2->u2.ShareCount += 1;
            UNLOCK_PFN (OldIrql);
    
            PointerPte += 1;
            TempPte.u.Hard.PageFrameNumber += 1;
        }
    }
    
    //
    // Increment the count of the number of views for the
    // section object.  This requires the PFN lock to be held.
    // At the same time, insert the physical view descriptor now that
    // the page table page hierarchy is in place.  Note probes can find
    // this descriptor immediately.
    //

    MiInsertPhysicalViewAndRefControlArea (Process, ControlArea, PhysicalView);

    UNLOCK_WS_UNSAFE (Thread, Process);

    //
    // Update the current virtual size in the process header.
    //

    *CapturedViewSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
    Process->VirtualSize += *CapturedViewSize;

    if (Process->VirtualSize > Process->PeakVirtualSize) {
        Process->PeakVirtualSize = Process->VirtualSize;
    }

    *CapturedBase = StartingAddress;

    return STATUS_SUCCESS;
}


VOID
MiSetControlAreaSymbolsLoaded (
    IN PCONTROL_AREA ControlArea
    )

/*++

Routine Description:

    This is a nonpaged helper routine to mark the specified control area as
    having its debug symbols loaded.

Arguments:

    ControlArea - Supplies a pointer to the control area.

Return Value:

    None.

--*/

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);
    ControlArea->u.Flags.DebugSymbolsLoaded = 1;
    UNLOCK_PFN (OldIrql);
}

VOID
MiLoadUserSymbols (
    IN PCONTROL_AREA ControlArea,
    IN PVOID StartingAddress,
    IN PEPROCESS Process
    )

/*++

Routine Description:

    This routine loads symbols for user space executables and DLLs.

Arguments:

    ControlArea - Supplies the control area for the section.

    StartingAddress - Supplies the virtual address the section is mapped at.

    Process - Supplies the process pointer which is receiving the section.

Return Value:

    None.

Environment:

    Kernel Mode, address creation mutex held.

--*/

{
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    PKLDR_DATA_TABLE_ENTRY Entry;
    PIMAGE_NT_HEADERS NtHeaders;
    PUNICODE_STRING FileName;
    ANSI_STRING AnsiName;
    NTSTATUS Status;
    PKTHREAD CurrentThread;

    //
    //  TEMP TEMP TEMP rip out ANSI conversion when debugger converted.
    //

    FileName = (PUNICODE_STRING)&ControlArea->FilePointer->FileName;

    if (FileName->Length == 0) {
        return;
    }

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);

    Head = &MmLoadedUserImageList;
    Next = Head->Flink;

    while (Next != Head) {
        Entry = CONTAINING_RECORD (Next,
                                   KLDR_DATA_TABLE_ENTRY,
                                   InLoadOrderLinks);

        if (Entry->DllBase == StartingAddress) {
            Entry->LoadCount += 1;
            break;
        }
        Next = Next->Flink;
    }

    if (Next == Head) {
        Entry = ExAllocatePoolWithTag (NonPagedPool,
                                            sizeof (*Entry) +
                                            FileName->Length +
                                            sizeof (UNICODE_NULL ),
                                            MMDB);
        if (Entry != NULL) {

            RtlZeroMemory (Entry, sizeof(*Entry));

            try {
                NtHeaders = RtlImageNtHeader (StartingAddress);
                if (NtHeaders != NULL) {
#if defined(_WIN64)
                    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                        Entry->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
                        Entry->CheckSum = NtHeaders->OptionalHeader.CheckSum;
                    }
                    else {
                        PIMAGE_NT_HEADERS32 NtHeaders32 = (PIMAGE_NT_HEADERS32)NtHeaders;

                        Entry->SizeOfImage = NtHeaders32->OptionalHeader.SizeOfImage;
                        Entry->CheckSum = NtHeaders32->OptionalHeader.CheckSum;
                    }
#else
                    Entry->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
                    Entry->CheckSum = NtHeaders->OptionalHeader.CheckSum;
#endif
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }

            Entry->DllBase = StartingAddress;
            Entry->FullDllName.Buffer = (PWSTR)(Entry+1);
            Entry->FullDllName.Length = FileName->Length;
            Entry->FullDllName.MaximumLength = (USHORT)
                (Entry->FullDllName.Length + sizeof (UNICODE_NULL ));

            RtlCopyMemory (Entry->FullDllName.Buffer,
                           FileName->Buffer,
                           FileName->Length);

            Entry->FullDllName.Buffer[ Entry->FullDllName.Length / sizeof( WCHAR )] = UNICODE_NULL;
            Entry->LoadCount = 1;
            InsertTailList (&MmLoadedUserImageList,
                            &Entry->InLoadOrderLinks);

        }
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);

    Status = RtlUnicodeStringToAnsiString (&AnsiName,
                                           FileName,
                                           TRUE);

    if (NT_SUCCESS( Status)) {
        DbgLoadImageSymbols (&AnsiName,
                             StartingAddress,
                             (ULONG_PTR)Process);
        RtlFreeAnsiString (&AnsiName);
    }
    return;
}


VOID
MiDereferenceControlArea (
    IN PCONTROL_AREA ControlArea
    )

/*++

Routine Description:

    This is a nonpaged helper routine to dereference the specified control area.

Arguments:

    ControlArea - Supplies a pointer to the control area.

Return Value:

    None.

--*/

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);
    ControlArea->NumberOfMappedViews -= 1;
    ControlArea->NumberOfUserReferences -= 1;

    //
    // Check to see if the control area (segment) should be deleted.
    // This routine releases the PFN lock.
    //

    MiCheckControlArea (ControlArea, OldIrql);
}


NTSTATUS
MiMapViewOfImageSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType,
    IN SIZE_T ImageCommitment
    )

/*++

Routine Description:

    This routine maps the specified Image section into the
    specified process's address space.

Arguments:

    see MmMapViewOfSection above...

    ControlArea - Supplies the control area for the section.

    Process - Supplies the process pointer which is receiving the section.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, address creation mutex held.

--*/

{
    PMMVAD Vad;
    PAWEINFO AweInfo;
    PETHREAD Thread;
    SIZE_T LargeCapturedViewSize;
    PVOID VirtualAddress;
    PVOID StartingAddress;
    PVOID LargeStartingAddress;
    PVOID OutputStartingAddress;
    PVOID EndingAddress;
    PVOID LargeEndingAddress;
    PVOID HighestUserAddress;
    PSUBSECTION FirstSubsection;
    PSUBSECTION Subsection;
    ULONG PteOffset;
    NTSTATUS Status;
    NTSTATUS ReturnedStatus;
    PMMPTE ProtoPte;
    PVOID BasedAddress;
    SIZE_T NeededViewSize;
    SIZE_T OutputViewSize;
    SIZE_T LargeOutputViewSize;
    LOGICAL TryLargePages;

    //
    // Image file.
    //
    // Locate the first subsection (text) and create a virtual
    // address descriptor to map the entire image here.
    //

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    //
    // Check to see if a purge operation is in progress and if so, wait
    // for the purge to complete.  In addition, up the count of mapped
    // views for this control area.
    //

    Status = MiCheckPurgeAndUpMapCount (ControlArea, TRUE);
    
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    //
    // Capture the based address to the stack, to prevent page faults.
    //

    BasedAddress = ControlArea->Segment->BasedAddress;

    if (*CapturedViewSize == 0) {
        *CapturedViewSize =
            (ULONG_PTR)(Section->SizeOfSection.QuadPart - SectionOffset->QuadPart);
    }

    //
    // Check for privilege if a large page mapping is requested.
    //

    TryLargePages = FALSE;

    if ((AllocationType & MEM_LARGE_PAGES) &&
        (SectionOffset->QuadPart == 0) &&
#ifdef _X86_
        (KeFeatureBits & KF_LARGE_PAGE) &&
#endif
        (SeSinglePrivilegeCheck (SeLockMemoryPrivilege, KeGetPreviousMode()))) {

        //
        // Since large page mappings essentially separate this instance of
        // the image from any others, the image section must not have any
        // global subsections in it.
        //

        FirstSubsection = Subsection;

        do {

            if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {
                break;
            }
    
            Subsection = Subsection->NextSubsection;
    
        } while (Subsection != NULL);

        if (Subsection == NULL) {
            TryLargePages = TRUE;
        }

        Subsection = FirstSubsection;
    }

Top:

    ReturnedStatus = STATUS_SUCCESS;

    //
    // Determine if a specific base was specified.
    //

    if (*CapturedBase != NULL) {

        //
        // Captured base is not NULL.
        //
        // Check to make sure the specified address range is currently unused
        // and within the user address space.
        //

        StartingAddress = MI_64K_ALIGN(*CapturedBase);
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                       *CapturedViewSize - 1) | (PAGE_SIZE - 1));

        //
        // If a large page mapping is requested, see if the virtual address
        // space can accommodate it.
        //
        // Note any large page image mappings are built with the *ENTIRE*
        // image being read-write-execute (backed by the pagefile), so there
        // is no text protection, etc.
        //
        // A second point worth noting is that even if the address space
        // can accommodate mapping this image large (ie: no adjacent VADs are
        // within the large page boundaries this image would require), the
        // fact that this image's VAD is expanded here to large page boundaries
        // means some subsequent DLL load in this image may fail if it is
        // rebased to an address that falls within this large page allocation
        // and cannot be relocated.
        //

        if (TryLargePages == TRUE) {
            LargeStartingAddress = MI_ALIGN_TO_SIZE (StartingAddress,
                                                     MM_MINIMUM_VA_FOR_LARGE_PAGE);
            LargeEndingAddress = (PVOID)((ULONG_PTR)EndingAddress |
                                                   (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1));
            LargeCapturedViewSize = (ULONG_PTR) LargeEndingAddress - (ULONG_PTR) LargeStartingAddress + 1;

            Vad = (PMMVAD) TRUE;

            if (((PVOID)LargeCapturedViewSize <= MM_HIGHEST_VAD_ADDRESS) &&
                (StartingAddress >= MM_LOWEST_USER_ADDRESS) &&
                (LargeStartingAddress <= MM_HIGHEST_VAD_ADDRESS) &&
                (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) -
                                    (ULONG_PTR)LargeStartingAddress >= LargeCapturedViewSize) &&

                (LargeEndingAddress <= MM_HIGHEST_VAD_ADDRESS)) {

                Vad = (PMMVAD) (ULONG_PTR) MiCheckForConflictingVadExistence (Process, LargeStartingAddress, LargeEndingAddress);
            }

            //
            // If the Vad is NULL then virtual address space exists to
            // map this image with large pages.
            //
            // See if ample pages exist as the image pages will be
            // immediately copied into large pages and thus never paged.
            //

            if ((Vad == NULL) && (TryLargePages == TRUE)) {

                if (Process->AweInfo == NULL) {
                    AweInfo = MiAllocateAweInfo ();
                    if (AweInfo != NULL) {
                        MiInsertAweInfo (AweInfo);
                    }
                }

                if (Process->AweInfo != NULL) {
                    Status = MiAllocateLargePages (LargeStartingAddress,
                                                   LargeEndingAddress,
                                                   MM_EXECUTE_READWRITE,
                                                   FALSE);
    
                    if (NT_SUCCESS (Status)) {
                        ImageCommitment = LargeCapturedViewSize >> PAGE_SHIFT;
    
                        if (((ULONG_PTR)StartingAddress -
                            (ULONG_PTR)MI_64K_ALIGN(SectionOffset->LowPart)) != (ULONG_PTR)BasedAddress) {
                            ReturnedStatus = STATUS_IMAGE_NOT_AT_BASE;
                        }
    
                        goto AllocateVad;
                    }
                }
            }

            //
            // A conflict was discovered or physical pages were not available.
            // Fall through and try to map the view with small pages instead.
            //

            TryLargePages = FALSE;
        }

        if ((StartingAddress <= MM_HIGHEST_VAD_ADDRESS) &&
            (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) -
                                (ULONG_PTR)StartingAddress >= *CapturedViewSize) &&

            (EndingAddress <= MM_HIGHEST_VAD_ADDRESS)) {

            if (MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress) == TRUE) {
                MiDereferenceControlArea (ControlArea);
                return STATUS_CONFLICTING_ADDRESSES;
            }
        }
        else {
            MiDereferenceControlArea (ControlArea);
            return STATUS_CONFLICTING_ADDRESSES;
        }

        //
        // A conflicting VAD was not found and the specified address range is
        // within the user address space.  If the image will not reside at its
        // base address, then set a special return status.
        //

        if (((ULONG_PTR)StartingAddress -
            (ULONG_PTR)MI_64K_ALIGN(SectionOffset->LowPart)) != (ULONG_PTR)BasedAddress) {
            ReturnedStatus = STATUS_IMAGE_NOT_AT_BASE;
        }
    }
    else {

        //
        // Captured base is NULL.
        //
        // If the captured view size is greater than the largest size that
        // can fit in the user address space, then it is not possible to map
        // the image.
        //

        if ((PVOID)*CapturedViewSize > MM_HIGHEST_VAD_ADDRESS) {
            MiDereferenceControlArea (ControlArea);
            return STATUS_NO_MEMORY;
        }

        //
        // Check to make sure the specified address range is currently unused
        // and within the user address space.
        //

        StartingAddress = (PVOID)((ULONG_PTR)BasedAddress +
                                    (ULONG_PTR)MI_64K_ALIGN(SectionOffset->LowPart));

        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                    *CapturedViewSize - 1) | (PAGE_SIZE - 1));

        //
        // If a large page mapping is requested, see if the virtual address
        // space can accommodate it.
        //
        // Note any large page image mappings are built with the *ENTIRE*
        // image being read-write-execute (backed by the pagefile), so there
        // is no text protection, etc.
        //
        // A second point worth noting is that even if the address space
        // can accommodate mapping this image large (ie: no adjacent VADs are
        // within the large page boundaries this image would require), the
        // fact that this image's VAD is expanded here to large page boundaries
        // means some subsequent DLL load in this image may fail if it is
        // rebased to an address that falls within this large page allocation
        // and cannot be relocated.
        //

        if (TryLargePages == TRUE) {
            LargeStartingAddress = MI_ALIGN_TO_SIZE (StartingAddress,
                                                     MM_MINIMUM_VA_FOR_LARGE_PAGE);
            LargeEndingAddress = (PVOID)((ULONG_PTR)EndingAddress |
                                                   (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1));
            LargeCapturedViewSize = (ULONG_PTR) LargeEndingAddress - (ULONG_PTR) LargeStartingAddress + 1;

            Vad = (PMMVAD) TRUE;

            if (((PVOID)LargeCapturedViewSize <= MM_HIGHEST_VAD_ADDRESS) &&
                (StartingAddress >= MM_LOWEST_USER_ADDRESS) &&
                (LargeStartingAddress <= MM_HIGHEST_VAD_ADDRESS) &&
                (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) -
                                    (ULONG_PTR)LargeStartingAddress >= LargeCapturedViewSize) &&

                (LargeEndingAddress <= MM_HIGHEST_VAD_ADDRESS)) {

                Vad = (PMMVAD) (ULONG_PTR) MiCheckForConflictingVadExistence (Process, LargeStartingAddress, LargeEndingAddress);
            }

            //
            // If the Vad is NULL then virtual address space exists to
            // map this image with large pages.
            //
            // See if ample pages exist as the image pages will be
            // immediately copied into large pages and thus never paged.
            //

            if ((Vad == NULL) && (TryLargePages == TRUE)) {

                if (Process->AweInfo == NULL) {
                    AweInfo = MiAllocateAweInfo ();
                    if (AweInfo != NULL) {
                        MiInsertAweInfo (AweInfo);
                    }
                }

                if (Process->AweInfo != NULL) {
                    Status = MiAllocateLargePages (LargeStartingAddress,
                                                   LargeEndingAddress,
                                                   MM_EXECUTE_READWRITE,
                                                   FALSE);
    
                    if (NT_SUCCESS (Status)) {
                        ImageCommitment = LargeCapturedViewSize >> PAGE_SHIFT;
                        goto AllocateVad;
                    }
                }
            }

            //
            // A conflict was discovered or physical pages were not available.
            // Fall through and try to map the view with small pages instead.
            //

            TryLargePages = FALSE;
        }

        Vad = (PMMVAD) TRUE;
        NeededViewSize = *CapturedViewSize;

        if ((StartingAddress >= MM_LOWEST_USER_ADDRESS) &&
            (StartingAddress <= MM_HIGHEST_VAD_ADDRESS) &&
            (((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) -
                                (ULONG_PTR)StartingAddress >= *CapturedViewSize) &&

            (EndingAddress <= MM_HIGHEST_VAD_ADDRESS)) {

            Vad = (PMMVAD) (ULONG_PTR) MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress);
        }

        //
        // If the VAD address is not NULL, then a conflict was discovered.
        // Attempt to select another address range in which to map the image.
        //

        if (Vad != NULL) {

            //
            // The image could not be mapped at its natural base address
            // try to find another place to map it.
            //
            // If the system has been biased to an alternate base address to
            // allow 3gb of user address space, then make sure the high order
            // address bit is zero.
            //

            if ((MmVirtualBias != 0) && (ZeroBits == 0)) {
                ZeroBits = 1;
            }

            ReturnedStatus = STATUS_IMAGE_NOT_AT_BASE;

            //
            // Find a starting address on a 64k boundary.
            //
            // Check whether the registry indicates that all applications
            // should be given virtual address ranges from the highest
            // address downwards in order to test 3GB-aware apps on 32-bit
            // machines and 64-bit apps on NT64.
            //

            if (Process->VmTopDown == 1) {

                if (ZeroBits != 0) {
                    HighestUserAddress = (PVOID)((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits);
                    if (HighestUserAddress > MM_HIGHEST_VAD_ADDRESS) {
                        HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
                    }
                }
                else {
                    HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
                }

                Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                      NeededViewSize,
                                                      HighestUserAddress,
                                                      X64K,
                                                      &StartingAddress);
            }
            else {
                Status = MiFindEmptyAddressRange (NeededViewSize,
                                                  X64K,
                                                  (ULONG)ZeroBits,
                                                  &StartingAddress);
            }

            if (!NT_SUCCESS (Status)) {
                MiDereferenceControlArea (ControlArea);
                return Status;
            }

            EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                        *CapturedViewSize - 1) | (PAGE_SIZE - 1));
        }
    }

    LargeStartingAddress = StartingAddress;
    LargeEndingAddress = EndingAddress;

AllocateVad:

    Thread = PsGetCurrentThread ();

    //
    // Allocate and initialize a VAD for the specified address range.
    //

    Vad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD), MMVADKEY);

    if (Vad == NULL) {
        if (TryLargePages == TRUE) {
            MiReleasePhysicalCharges (
                BYTES_TO_PAGES ((PCHAR)LargeEndingAddress + 1 - (PCHAR)LargeStartingAddress),
                Process);

            LOCK_WS_UNSAFE (Thread, Process);
            MiFreeLargePages (LargeStartingAddress, LargeEndingAddress, FALSE);
            UNLOCK_WS_UNSAFE (Thread, Process);
        }
        MiDereferenceControlArea (ControlArea);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Vad, sizeof(MMVAD));
    Vad->StartingVpn = MI_VA_TO_VPN (LargeStartingAddress);
    Vad->EndingVpn = MI_VA_TO_VPN (LargeEndingAddress);
    Vad->u2.VadFlags2.Inherit = (InheritDisposition == ViewShare);
    Vad->u.VadFlags.VadType = VadImageMap;

    //
    // Set the protection in the VAD as EXECUTE_WRITE_COPY.
    //

    Vad->u.VadFlags.Protection = MM_EXECUTE_WRITECOPY;
    Vad->ControlArea = ControlArea;

    //
    // Set the first prototype PTE field in the Vad.
    //

    SectionOffset->LowPart = SectionOffset->LowPart & ~(X64K - 1);
    PteOffset = (ULONG)(SectionOffset->QuadPart >> PAGE_SHIFT);

    Vad->FirstPrototypePte = &Subsection->SubsectionBase[PteOffset];
    Vad->LastContiguousPte = MM_ALLOCATION_FILLS_VAD;

    //
    // NOTE: the full commitment is charged even if a partial map of an
    // image is being done.  This saves from having to run through the
    // entire image (via prototype PTEs) and calculate the charge on
    // a per page basis for the partial map.
    //

    Vad->u.VadFlags.CommitCharge = (SIZE_T)ImageCommitment; // ****** temp

    ASSERT (Vad->FirstPrototypePte <= Vad->LastContiguousPte);

    Status = MiInsertVadCharges (Vad, Process);

    if (!NT_SUCCESS (Status)) {

        MiDereferenceControlArea (ControlArea);

        if (TryLargePages == TRUE) {
            MiReleasePhysicalCharges (
                BYTES_TO_PAGES ((PCHAR)LargeEndingAddress + 1 - (PCHAR)LargeStartingAddress),
                Process);
            LOCK_WS_UNSAFE (Thread, Process);
            MiFreeLargePages (LargeStartingAddress, LargeEndingAddress, FALSE);
            UNLOCK_WS_UNSAFE (Thread, Process);
        }

        //
        // The quota charge in InsertVad failed,
        // deallocate the pool and return an error.
        //

        ExFreePool (Vad);
        return Status;
    }

    LOCK_WS_UNSAFE (Thread, Process);

    MiInsertVad (Vad, Process);

    UNLOCK_WS_UNSAFE (Thread, Process);

    OutputStartingAddress = StartingAddress;
    OutputViewSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
    LargeOutputViewSize = (PCHAR)LargeEndingAddress - (PCHAR)LargeStartingAddress + 1L;

    //
    // Update the current virtual size in the process header.
    //

    Process->VirtualSize += LargeOutputViewSize;

    if (Process->VirtualSize > Process->PeakVirtualSize) {
        Process->PeakVirtualSize = Process->VirtualSize;
    }

    if (TryLargePages == TRUE) {

        PVOID NullCapturedBase = NULL;
        PVOID SmallVirtualAddress;
        SIZE_T NullCapturedViewSize = 0;
        LARGE_INTEGER SmallSectionOffset;
        
        SmallSectionOffset = *SectionOffset;

        //
        // The image has already been mapped with large (zero) pages.
        // Update the image map using the prototype PTEs, initializing
        // the PTEs so on return, no faults will be taken on this view.
        //
        // Note we keep our reference to the image so it stays "busy".
        // Even though this view is not affected by any changes to the
        // image, this provides the same semantics to callers as small views.
        //

        ProtoPte = Vad->FirstPrototypePte;
        VirtualAddress = StartingAddress;

        //
        // Map the view with small pages to make it easy to copy the data
        // into the large page view.  Note that the ordinary user can see the
        // data in the small view (but cannot protect or delete the small
        // view since we hold the address space mutex throughout), this is ok.
        //

        Status = MiMapViewOfImageSection (ControlArea,
                                          Process,
                                          &NullCapturedBase,
                                          &SmallSectionOffset,
                                          &NullCapturedViewSize,
                                          Section,
                                          InheritDisposition,
                                          0,
                                          0,
                                          ImageCommitment);

        if (!NT_SUCCESS (Status)) {
            MiUnmapViewOfSection (Process,
                                  VirtualAddress,
                                  UNMAP_ADDRESS_SPACE_HELD);

            Status = MiCheckPurgeAndUpMapCount (ControlArea, TRUE);
            
            if (!NT_SUCCESS (Status)) {
                return Status;
            }

            TryLargePages = FALSE;
            goto Top;
        }

        SmallVirtualAddress = NullCapturedBase;

        while (VirtualAddress < EndingAddress) {

            //
            // If the prototype PTE is valid, copy it.
            //
            // Otherwise, it is in the pagefile, transition or
            // in prototype PTE format - if the permissions indicate
            // that it is accessible, copy it.
            //

            if ((ProtoPte->u.Hard.Valid == 1) ||
                (ProtoPte->u.Soft.Protection != MM_NOACCESS)) {

                //
                // These accesses could get in-page errors from
                // the network (or disk).
                //

                try {

                    RtlCopyMemory (VirtualAddress,
                                   SmallVirtualAddress,
                                   PAGE_SIZE);

                } except (EXCEPTION_EXECUTE_HANDLER) {
                    Status = GetExceptionCode ();
                }

                if (!NT_SUCCESS (Status)) {

                    //
                    // An in page error must have occurred copying the image,
                    // This must be treated as terminal now - so the app
                    // doesn't later see zeroes when referencing this page.
                    //
                    //
                    // N.B.  There are no race conditions with the user
                    // deleting/substituting this mapping from another
                    // thread as the address space mutex is still held.
                    //

                    //
                    // Unmap the large page view.
                    //

                    MiUnmapViewOfSection (Process,
                                          VirtualAddress,
                                          UNMAP_ADDRESS_SPACE_HELD);

                    //
                    // Unmap the small page view.
                    //

                    MiUnmapViewOfSection (Process,
                                          NullCapturedBase,
                                          UNMAP_ADDRESS_SPACE_HELD);

                    Status = MiCheckPurgeAndUpMapCount (ControlArea, TRUE);
                    
                    if (!NT_SUCCESS (Status)) {
                        return Status;
                    }

                    TryLargePages = FALSE;

                    goto Top;
                }
            }
            ProtoPte += 1;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
            SmallVirtualAddress = (PVOID)((PCHAR)SmallVirtualAddress + PAGE_SIZE);
        }

        //
        // Unmap the small page view.
        //

        MiUnmapViewOfSection (Process,
                              NullCapturedBase,
                              UNMAP_ADDRESS_SPACE_HELD);

        //
        // If the new pages had been used to hold code in a previous
        // life we must ensure the instruction cache is kept coherent.
        // The data copy above is not sufficient on platforms where
        // the I-cache & D-cache are not kept coherent in hardware
        // so sweep it here for them.
        //

        KeSweepIcacheRange (TRUE,
                            StartingAddress,
                            ((ULONG_PTR) EndingAddress - (ULONG_PTR) StartingAddress));
    }
    else if (ControlArea->u.Flags.FloppyMedia) {

        //
        // The image resides on a floppy disk, in-page all
        // pages from the floppy and mark them as modified so
        // they migrate to the paging file rather than reread
        // them from the floppy disk which may have been removed.
        //

        ProtoPte = Vad->FirstPrototypePte;

        //
        // This could get an in-page error from the floppy.
        //

        VirtualAddress = StartingAddress;

        while (VirtualAddress < EndingAddress) {

            //
            // If the prototype PTE is valid, transition or
            // in prototype PTE format, bring the page into
            // memory and set the modified bit.
            //

            if ((ProtoPte->u.Hard.Valid == 1) ||

                (((ProtoPte->u.Soft.Prototype == 1) ||
                 (ProtoPte->u.Soft.Transition == 1)) &&
                 (ProtoPte->u.Soft.Protection != MM_NOACCESS))
                
                ) {

                Status = MiSetPageModified (Vad, VirtualAddress);

                if (!NT_SUCCESS (Status)) {

                    //
                    // An in page error must have occurred touching the image,
                    // Ignore the error and continue to the next page - unless
                    // it's being run over a network.  If it's being run over
                    // a net and the control area is marked as floppy, then
                    // the image must be marked NET_RUN_FROM_SWAP, so any
                    // inpage error must be treated as terminal now - so the app
                    // doesn't later spontaneously abort when referencing
                    // this page.  This provides app writers with a way to
                    // mark their app in a way which is robust regardless of
                    // network conditions.
                    //

                    if (ControlArea->u.Flags.Networked) {

                        //
                        // N.B.  There are no race conditions with the user
                        // deleting/substituting this mapping from another
                        // thread as the address space mutex is still held.
                        //

                        MiUnmapViewOfSection (Process,
                                              VirtualAddress,
                                              UNMAP_ADDRESS_SPACE_HELD);

                        return Status;
                    }
                }
            }
            ProtoPte += 1;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
        }
    }

    *CapturedViewSize = OutputViewSize;
    *CapturedBase = OutputStartingAddress;

    ASSERT (NT_SUCCESS (ReturnedStatus));

    //
    // Check to see if this image is for the architecture of the current
    // machine.
    //

    if (ControlArea->Segment->u2.ImageInformation->ImageContainsCode &&
        ((ControlArea->Segment->u2.ImageInformation->Machine <
                                      USER_SHARED_DATA->ImageNumberLow) ||
         (ControlArea->Segment->u2.ImageInformation->Machine >
                                      USER_SHARED_DATA->ImageNumberHigh)
        )
       ) {
#if defined (_WIN64)

        //
        // If this is a wow64 process then allow i386 images.
        //

        if (!Process->Wow64Process ||
            ControlArea->Segment->u2.ImageInformation->Machine != IMAGE_FILE_MACHINE_I386) {
            return STATUS_IMAGE_MACHINE_TYPE_MISMATCH;
        }
#else   //!_WIN64
        return STATUS_IMAGE_MACHINE_TYPE_MISMATCH;
#endif
    }

    if (PsImageNotifyEnabled) {

        IMAGE_INFO ImageInfo;

        if ((StartingAddress < MmHighestUserAddress) &&
            (Process->UniqueProcessId) &&
            (Process != PsInitialSystemProcess)) {

            ImageInfo.Properties = 0;
            ImageInfo.ImageAddressingMode = IMAGE_ADDRESSING_MODE_32BIT;
            ImageInfo.ImageBase = StartingAddress;
            ImageInfo.ImageSize = OutputViewSize;
            ImageInfo.ImageSelector = 0;
            ImageInfo.ImageSectionNumber = 0;

            PsCallImageNotifyRoutines (&ControlArea->FilePointer->FileName,
                                       Process->UniqueProcessId,
                                       &ImageInfo);
        }
    }

    ASSERT (ControlArea->u.Flags.Image);

    if ((NtGlobalFlag & FLG_ENABLE_KDEBUG_SYMBOL_LOAD) &&
        (ReturnedStatus != STATUS_IMAGE_NOT_AT_BASE) &&
        (ControlArea->u.Flags.DebugSymbolsLoaded == 0)) {

        if (MiCacheImageSymbols (StartingAddress)) {

            MiSetControlAreaSymbolsLoaded (ControlArea);

            MiLoadUserSymbols (ControlArea, StartingAddress, Process);
        }
    }

    return ReturnedStatus;
}

NTSTATUS
MiAddViewsForSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    )

/*++

Routine Description:

    This non-pageable wrapper routine maps the views into the specified section.

Arguments:

    StartMappedSubsection - Supplies the mapped subsection to start at.

    LastPteOffset - Supplies the last PTE offset to end the views at.
                    Supplies zero if views are desired from the supplied
                    subsection to the end of the file.

Return Value:

    NTSTATUS.

Environment:

    Kernel Mode, address creation mutex optionally held if called in process
    context.  APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    NTSTATUS Status;
    ULONG Waited;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    LOCK_PFN (OldIrql);

    //
    // This routine returns with the PFN lock released !
    //

    Status = MiAddViewsForSection (StartMappedSubsection,
                                   LastPteOffset,
                                   OldIrql,
                                   &Waited);

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    return Status;
}

LOGICAL
MiReferenceSubsection (
    IN PMSUBSECTION MappedSubsection
    )

/*++

Routine Description:

    This non-pageable routine reference counts the specified subsection if
    its prototype PTEs are already setup, otherwise it returns FALSE.

Arguments:

    MappedSubsection - Supplies the mapped subsection to start at.

Return Value:

    NTSTATUS.

Environment:

    Kernel Mode, PFN lock held.

--*/

{
    ASSERT ((MappedSubsection->ControlArea->u.Flags.Image == 0) &&
            (MappedSubsection->ControlArea->FilePointer != NULL) &&
            (MappedSubsection->ControlArea->u.Flags.PhysicalMemory == 0));

    MM_PFN_LOCK_ASSERT();

    //
    // Note the control area is not necessarily active at this point.
    //

    if (MappedSubsection->SubsectionBase == NULL) {

        //
        // No prototype PTEs exist, caller will have to go the long way.
        //

        return FALSE;
    }

    //
    // The mapping base exists so the number of mapped views can be
    // safely incremented.  This prevents a trim from starting after
    // we release the lock.
    //

    MappedSubsection->NumberOfMappedViews += 1;

    MI_SNAP_SUB (MappedSubsection, 0x4);

    if (MappedSubsection->DereferenceList.Flink != NULL) {

        //
        // Remove this from the list of unused subsections.
        //

        RemoveEntryList (&MappedSubsection->DereferenceList);

        MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);

        MappedSubsection->DereferenceList.Flink = NULL;
    }

    //
    // Set the access bit so an already ongoing trim won't blindly
    // delete the prototype PTEs on completion of a mapped write.
    // This can happen if the current thread dirties some pages and
    // then deletes the view before the trim write finishes - this
    // bit informs the trimming thread that a rescan is needed so
    // that writes are not lost.
    //

    MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;

    return TRUE;
}

NTSTATUS
MiAddViewsForSection (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    )

/*++

Routine Description:

    This non-pageable routine maps the views into the specified section.

    N.B. This routine may release and reacquire the PFN lock !

    N.B. This routine always returns with the PFN lock released !

    N.B. Callers may pass in view lengths that exceed the length of
         the section and this must succeed.  Thus MiAddViews must check
         for this and know to stop the references at the end.  More
         importantly, MiRemoveViews must also contain the same logic.

Arguments:

    StartMappedSubsection - Supplies the mapped subsection to start at.

    LastPteOffset - Supplies the number of PTEs (beginning at the supplied
                    subsection) to provide views for.  Supplies zero if
                    views are desired from the supplied subsection to the
                    end of the file.

    Waited - Supplies a pointer to a ULONG to increment if the PFN lock is
             released and reacquired.

Return Value:

    NTSTATUS, PFN lock released.

Environment:

    Kernel Mode, PFN lock held.

--*/

{
    MMPTE TempPte;
    ULONG Size;
    PMMPTE ProtoPtes;
    PMSUBSECTION MappedSubsection;

    *Waited = 0;

    MappedSubsection = StartMappedSubsection;

    ASSERT ((MappedSubsection->ControlArea->u.Flags.Image == 0) &&
            (MappedSubsection->ControlArea->FilePointer != NULL) &&
            (MappedSubsection->ControlArea->u.Flags.PhysicalMemory == 0));

    MM_PFN_LOCK_ASSERT();

    do {

        //
        // Note the control area must be active at this point.
        //

        ASSERT (MappedSubsection->ControlArea->DereferenceList.Flink == NULL);

        if (MappedSubsection->SubsectionBase != NULL) {

            //
            // The mapping base exists so the number of mapped views can be
            // safely incremented.  This prevents a trim from starting after
            // we release the lock.
            //

            MappedSubsection->NumberOfMappedViews += 1;

            MI_SNAP_SUB (MappedSubsection, 0x5);

            if (MappedSubsection->DereferenceList.Flink != NULL) {

                //
                // Remove this from the list of unused subsections.
                //

                RemoveEntryList (&MappedSubsection->DereferenceList);

                MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);

                MappedSubsection->DereferenceList.Flink = NULL;
            }

            //
            // Set the access bit so an already ongoing trim won't blindly
            // delete the prototype PTEs on completion of a mapped write.
            // This can happen if the current thread dirties some pages and
            // then deletes the view before the trim write finishes - this
            // bit informs the trimming thread that a rescan is needed so
            // that writes are not lost.
            //

            MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;
        }
        else {

            ASSERT (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0);
            ASSERT (MappedSubsection->NumberOfMappedViews == 0);

            MI_SNAP_SUB (MappedSubsection, 0x6);

            //
            // No prototype PTEs currently exist for this subsection.
            // Allocate and populate them properly now.
            //

            UNLOCK_PFN (OldIrql);
            *Waited = 1;

            Size = (MappedSubsection->PtesInSubsection + MappedSubsection->UnusedPtes) * sizeof(MMPTE);

            ASSERT (Size != 0);

            ProtoPtes = (PMMPTE)ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                                       Size,
                                                       MMSECT);

            if (ProtoPtes == NULL) {
                MI_SNAP_SUB (MappedSubsection, 0x7);
                goto Failed;
            }

            //
            // Fill in the prototype PTEs for this subsection.
            //

            TempPte.u.Long = MiGetSubsectionAddressForPte (MappedSubsection);
            TempPte.u.Soft.Prototype = 1;

            //
            // Set all the PTEs to the initial execute-read-write protection.
            // The section will control access to these and the segment
            // must provide a method to allow other users to map the file
            // for various protections.
            //

            TempPte.u.Soft.Protection = MappedSubsection->ControlArea->Segment->SegmentPteTemplate.u.Soft.Protection;

            MiFillMemoryPte (ProtoPtes, Size / sizeof (MMPTE), TempPte.u.Long);

            LOCK_PFN (OldIrql);

            //
            // Now that the mapping base is guaranteed to be nonzero (shortly),
            // the number of mapped views can be safely incremented.  This
            // prevents a trim from starting after we release the lock.
            //

            MappedSubsection->NumberOfMappedViews += 1;

            MI_SNAP_SUB (MappedSubsection, 0x8);

            //
            // Set the access bit so an already ongoing trim won't blindly
            // delete the prototype PTEs on completion of a mapped write.
            // This can happen if the current thread dirties some pages and
            // then deletes the view before the trim write finishes - this
            // bit informs the trimming thread that a rescan is needed so
            // that writes are not lost.
            //

            MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;

            //
            // Check to make sure another thread didn't do this already while
            // the lock was released.
            //

            if (MappedSubsection->SubsectionBase == NULL) {

                ASSERT (MappedSubsection->NumberOfMappedViews == 1);

                MappedSubsection->SubsectionBase = ProtoPtes;
            }
            else {
                if (MappedSubsection->DereferenceList.Flink != NULL) {
    
                    //
                    // Remove this from the list of unused subsections.
                    //
    
                    ASSERT (MappedSubsection->NumberOfMappedViews == 1);

                    RemoveEntryList (&MappedSubsection->DereferenceList);
    
                    MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);
    
                    MappedSubsection->DereferenceList.Flink = NULL;
                }
                else {
                    ASSERT (MappedSubsection->NumberOfMappedViews > 1);
                }

                //
                // This unlock and release of pool could be postponed until
                // the end of this routine when the lock is released anyway
                // but this should be a rare case anyway so don't bother.
                //

                UNLOCK_PFN (OldIrql);
                ExFreePool (ProtoPtes);
                LOCK_PFN (OldIrql);
            }
            MI_SNAP_SUB (MappedSubsection, 0x9);
        }

        if (LastPteOffset != 0) {
            ASSERT ((LONG)MappedSubsection->PtesInSubsection > 0);
            ASSERT ((UINT64)LastPteOffset > 0);
            if (LastPteOffset <= (UINT64) MappedSubsection->PtesInSubsection) {
                break;
            }
            LastPteOffset -= MappedSubsection->PtesInSubsection;
        }

        MappedSubsection = (PMSUBSECTION) MappedSubsection->NextSubsection;

    } while (MappedSubsection != NULL);

    UNLOCK_PFN (OldIrql);

    return STATUS_SUCCESS;

Failed:

    LOCK_PFN (OldIrql);

    //
    // A prototype PTE pool allocation failed.  Carefully undo any allocations
    // and references done so far.
    //

    while (StartMappedSubsection != MappedSubsection) {
        ASSERT ((LONG_PTR)StartMappedSubsection->NumberOfMappedViews >= 1);
        StartMappedSubsection->NumberOfMappedViews -= 1;
        ASSERT (StartMappedSubsection->u.SubsectionFlags.SubsectionStatic == 0);
        ASSERT (StartMappedSubsection->DereferenceList.Flink == NULL);
        MI_SNAP_SUB (MappedSubsection, 0xA);
        if (StartMappedSubsection->NumberOfMappedViews == 0) {

            //
            // Insert this subsection into the unused subsection list.
            // Since it's not likely there are any resident protos at this
            // point, enqueue each subsection at the front.
            //

            InsertHeadList (&MmUnusedSubsectionList,
                            &StartMappedSubsection->DereferenceList);
            MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
        }
        StartMappedSubsection = (PMSUBSECTION) StartMappedSubsection->NextSubsection;
    }

    UNLOCK_PFN (OldIrql);

    return STATUS_INSUFFICIENT_RESOURCES;
}


VOID
MiRemoveViewsFromSection (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    )

/*++

Routine Description:

    This non-pageable routine removes the views from the specified section if
    the reference count reaches zero.

    N.B. Callers may pass in view lengths that exceed the length of
         the section and this must succeed.  Thus MiAddViews checks
         for this and knows to stop the references at the end.  More
         importantly, MiRemoveViews must also contain the same logic.

Arguments:

    StartMappedSubsection - Supplies the mapped subsection to start at.

    LastPteOffset - Supplies the number of PTEs (beginning at the supplied
                    subsection) to remove.  Supplies zero to remove views
                    from the supplied subsection to the end of the file.

Return Value:

    None.

Environment:

    Kernel Mode, PFN lock held.

--*/

{
    PMSUBSECTION MappedSubsection;

    MappedSubsection = StartMappedSubsection;

    ASSERT ((MappedSubsection->ControlArea->u.Flags.Image == 0) &&
            (MappedSubsection->ControlArea->FilePointer != NULL) &&
            (MappedSubsection->ControlArea->u.Flags.PhysicalMemory == 0));

    MM_PFN_LOCK_ASSERT();

    do {

        //
        // Note the control area must be active at this point.
        //

        ASSERT (MappedSubsection->ControlArea->DereferenceList.Flink == NULL);
        ASSERT (MappedSubsection->SubsectionBase != NULL);
        ASSERT (MappedSubsection->DereferenceList.Flink == NULL);

        ASSERT (((LONG_PTR)MappedSubsection->NumberOfMappedViews >= 1) ||
                (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1));

        MappedSubsection->NumberOfMappedViews -= 1;

        MI_SNAP_SUB (MappedSubsection, 0x3);

        if ((MappedSubsection->NumberOfMappedViews == 0) &&
            (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0)) {

            //
            // Insert this subsection into the unused subsection list.
            //

            InsertTailList (&MmUnusedSubsectionList,
                            &MappedSubsection->DereferenceList);
            MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
        }

        if (LastPteOffset != 0) {
            if (LastPteOffset <= (UINT64)MappedSubsection->PtesInSubsection) {
                break;
            }
            LastPteOffset -= MappedSubsection->PtesInSubsection;
        }

        MappedSubsection = (PMSUBSECTION) MappedSubsection->NextSubsection;

    } while (MappedSubsection != NULL);

    return;
}


VOID
MiRemoveViewsFromSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    )

/*++

Routine Description:

    This non-pageable routine removes the views from the specified section if
    the reference count reaches zero.

Arguments:

    StartMappedSubsection - Supplies the mapped subsection to start at.

    LastPteOffset - Supplies the number of PTEs (beginning at the supplied
                    subsection) to remove.  Supplies zero to remove views
                    from the supplied subsection to the end of the file.

Return Value:

    None.

Environment:

    Kernel Mode, address creation mutex optionally held if called in process
    context.  APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    LOCK_PFN (OldIrql);

    MiRemoveViewsFromSection (StartMappedSubsection, LastPteOffset);

    UNLOCK_PFN (OldIrql);
}
#if DBG
extern PMSUBSECTION MiActiveSubsection;
#endif

VOID
MiConvertStaticSubsections (
    IN PCONTROL_AREA ControlArea
    )
{
    PMSUBSECTION MappedSubsection;

    ASSERT (ControlArea->u.Flags.Image == 0);
    ASSERT (ControlArea->FilePointer != NULL);
    ASSERT (ControlArea->u.Flags.PhysicalMemory == 0);

    if (ControlArea->u.Flags.Rom == 0) {
        MappedSubsection = (PMSUBSECTION)(ControlArea + 1);
    }
    else {
        MappedSubsection = (PMSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    do {
        MI_SNAP_SUB (MappedSubsection, 0xB);
        if (MappedSubsection->DereferenceList.Flink != NULL) {

            // 
            // This subsection is already on the dereference subsection list.
            // This is the expected case.
            // 

            NOTHING;
        }
        else if (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1) {

            // 
            // This subsection was not put on the dereference subsection list
            // because it was created as part of an extension or nowrite.
            // Since this is the last segment dereference, convert the
            // subsection and its prototype PTEs to dynamic now (iff nowrite
            // is clear).
            // 

            MappedSubsection->u.SubsectionFlags.SubsectionStatic = 0;
            MappedSubsection->u2.SubsectionFlags2.SubsectionConverted = 1;
            MappedSubsection->NumberOfMappedViews = 1;

            MiRemoveViewsFromSection (MappedSubsection, 
                                      MappedSubsection->PtesInSubsection);

            MiSubsectionsConvertedToDynamic += 1;
        }
        else if (MappedSubsection->SubsectionBase == NULL) {

            //
            // This subsection has already had its prototype PTEs reclaimed
            // (or never allocated), hence it is not on any reclaim lists.
            //

            NOTHING;
        }
        else {

            // 
            // This subsection is being processed by the dereference
            // segment thread right now !  The dereference thread sets the
            // mapped view count to 1 when it starts processing the subsection.
            // The subsequent flush then increases it to 2 while in progress.
            // So the count must be either 1 or 2 at this point.
            // 

            ASSERT (MappedSubsection == MiActiveSubsection);
            ASSERT ((MappedSubsection->NumberOfMappedViews == 1) ||
                    (MappedSubsection->NumberOfMappedViews == 2));
        }
        MappedSubsection = (PMSUBSECTION) MappedSubsection->NextSubsection;
    } while (MappedSubsection != NULL);
}

VOID
MiMapLargePageSection (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN MM_PROTECTION_MASK ProtectionMask
    )

/*++

Routine Description:

    This routine maps the specified section into the specified process's
    address space using large page mappings.

Arguments:

    Process - Supplies the relevant process.

    StartingAddress - Supplies the starting virtual address.

    EndingAddress - Supplies the ending virtual address.

    Vad - Supplies the relevant VAD.

    ProtectionMask - Supplies the initial page protection-mask.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, address creation pushlock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    KIRQL OldIrql;
    PSEGMENT Segment;
    MMPTE TempPte;
    MMPTE TempPde;
    PMMPTE ProtoPte;
    PCONTROL_AREA ControlArea;
    PFN_NUMBER PageFrameIndex;
    PETHREAD Thread;
    PMMPTE PointerPde;
    PMMPTE EndPde;
#if DBG
    PMMPTE EndProto;
#endif

    PointerPde = MiGetPdeAddress (StartingAddress);
    EndPde = MiGetPdeAddress (EndingAddress);

    Vad->u.VadFlags.VadType = VadLargePageSection;

    ControlArea = Vad->ControlArea;
    Segment = ControlArea->Segment;

    ASSERT (Segment->SegmentFlags.LargePages == 1);

    ProtoPte = Vad->FirstPrototypePte;

    MI_MAKE_VALID_PTE (TempPde,
                       0,
                       ProtectionMask,
                       MiGetPteAddress (StartingAddress));

    if (TempPde.u.Hard.Write) {
        MI_SET_PTE_DIRTY (TempPde);
        MI_SET_ACCESSED_IN_PTE (&TempPde, 1);
    }

    MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);

    Thread = PsGetCurrentThread ();

    do {
        TempPte = *ProtoPte;
        ASSERT (TempPte.u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (PageFrameIndex % (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT) == 0);

        TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

        EndPfn = Pfn1 + (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

#if DBG
        EndProto = ProtoPte + (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

        do {
            ProtoPte += 1;
            TempPte = *ProtoPte;
            ASSERT (TempPte.u.Hard.Valid == 1);

            ASSERT (MI_GET_PAGE_FRAME_FROM_PTE (&TempPte) == PageFrameIndex + 1);
            PageFrameIndex += 1;
            ASSERT (MI_PFN_ELEMENT (PageFrameIndex)->PteAddress == ProtoPte);

        } while (ProtoPte + 1 < EndProto);
        ProtoPte -= (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
        ProtoPte += 1;
#endif

        LOCK_PFN (OldIrql);

        do {
            ASSERT (Pfn1->u2.ShareCount >= 1);
            Pfn1->u2.ShareCount += 1;
            Pfn1 += 1;
        } while (Pfn1 < EndPfn);

        UNLOCK_PFN (OldIrql);

        LOCK_WS_UNSAFE (Thread, Process);

        MI_WRITE_VALID_PTE (PointerPde, TempPde);

        UNLOCK_WS_UNSAFE (Thread, Process);

        PointerPde += 1;
        ProtoPte += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

    } while (PointerPde <= EndPde);

    return;
}


NTSTATUS
MiMapViewOfDataSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG ProtectionMask,
    IN SIZE_T CommitSize,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    )

/*++

Routine Description:

    This routine maps the specified mapped file or pagefile-backed section
    into the specified process's address space.

Arguments:

    see MmMapViewOfSection above...

    ControlArea - Supplies the control area for the section.

    Process - Supplies the process pointer which is receiving the section.

    ProtectionMask - Supplies the initial page protection-mask.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, address creation mutex held.

--*/

{
    PMMVAD Vad;
    LOGICAL TryLargePages;
    SIZE_T VadSize;
    PETHREAD Thread;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PSUBSECTION Subsection;
    UINT64 PteOffset;
    UINT64 LastPteOffset;
    UINT64 TotalNumberOfPtes;
    PVOID HighestUserAddress;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    MMPTE TempPte;
    ULONG_PTR Alignment;
    SIZE_T QuotaCharge;
    SIZE_T QuotaExcess;
    PMMPTE TheFirstPrototypePte;
    PVOID CapturedStartingVa;
    ULONG CapturedCopyOnWrite;
    NTSTATUS Status;
    PSEGMENT Segment;
    SIZE_T SizeOfSection;

    QuotaCharge = 0;
    Segment = ControlArea->Segment;

    if ((AllocationType & MEM_RESERVE) && (ControlArea->FilePointer == NULL)) {
        return STATUS_INVALID_PARAMETER_9;
    }

    //
    // Check to see if there is a purge operation ongoing for
    // this segment.
    //

    if ((AllocationType & MEM_DOS_LIM) != 0) {
        if ((*CapturedBase == NULL) ||
            (AllocationType & MEM_RESERVE)) {

            //
            // If MEM_DOS_LIM is specified, the address to map the
            // view MUST be specified as well.
            //

            return STATUS_INVALID_PARAMETER_3;
        }
        Alignment = PAGE_SIZE;
    }
    else {
        Alignment = X64K;
    }

    //
    // Check to see if a purge operation is in progress and if so, wait
    // for the purge to complete.  In addition, up the count of mapped
    // views for this control area.
    //

    Status = MiCheckPurgeAndUpMapCount (ControlArea, FALSE);
    
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    if (*CapturedViewSize == 0) {

        SectionOffset->LowPart &= ~((ULONG)Alignment - 1);
        *CapturedViewSize = (ULONG_PTR)(Section->SizeOfSection.QuadPart -
                                    SectionOffset->QuadPart);
    }
    else {
        *CapturedViewSize += SectionOffset->LowPart & (Alignment - 1);
        SectionOffset->LowPart &= ~((ULONG)Alignment - 1);
    }

    ASSERT ((SectionOffset->LowPart & ((ULONG)Alignment - 1)) == 0);

    if ((LONG_PTR)*CapturedViewSize <= 0) {

        //
        // Section offset or view size past size of section.
        //

        MiDereferenceControlArea (ControlArea);

        return STATUS_INVALID_VIEW_SIZE;
    }

    //
    // Calculate the first prototype PTE field so it can be stored in the Vad.
    //

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    PteOffset = (SectionOffset->QuadPart >> PAGE_SHIFT);

    //
    // Make sure the PTEs are not in the extended part of the segment.
    //

    TotalNumberOfPtes = (((UINT64)Segment->SegmentFlags.TotalNumberOfPtes4132) << 32) | Segment->TotalNumberOfPtes;

    if (PteOffset >= TotalNumberOfPtes) {
        MiDereferenceControlArea (ControlArea);
        return STATUS_INVALID_VIEW_SIZE;
    }

    LastPteOffset = ((SectionOffset->QuadPart + *CapturedViewSize + PAGE_SIZE - 1) >> PAGE_SHIFT);
    ASSERT (LastPteOffset >= PteOffset);

    while (PteOffset >= (UINT64)Subsection->PtesInSubsection) {
        PteOffset -= Subsection->PtesInSubsection;
        LastPteOffset -= Subsection->PtesInSubsection;
        Subsection = Subsection->NextSubsection;
        ASSERT (Subsection != NULL);
    }

    if (ControlArea->FilePointer != NULL) {

        //
        // Increment the view count for every subsection spanned by this view.
        //
        // N.B. Callers may pass in view lengths that exceed the length of
        // the section and this must succeed.  Thus MiAddViews must check
        // for this and know to stop the references at the end.  More
        // importantly, MiRemoveViews must also contain the same logic.
        //

        Status = MiAddViewsForSectionWithPfn ((PMSUBSECTION)Subsection,
                                              LastPteOffset);

        if (!NT_SUCCESS (Status)) {
            MiDereferenceControlArea (ControlArea);
            return Status;
        }
    }

    ASSERT (Subsection->SubsectionBase != NULL);
    TheFirstPrototypePte = &Subsection->SubsectionBase[PteOffset];

    //
    // Calculate the quota for the specified pages.
    //

    if ((ControlArea->FilePointer == NULL) &&
        (CommitSize != 0) &&
        (Segment->NumberOfCommittedPages < TotalNumberOfPtes)) {

        PointerPte = TheFirstPrototypePte;
        LastPte = PointerPte + BYTES_TO_PAGES(CommitSize);

        //
        // Charge quota for the entire requested range.  If the charge succeeds,
        // excess is returned when the PTEs are actually filled in.
        //

        QuotaCharge = LastPte - PointerPte;
    }

    CapturedStartingVa = (PVOID)Section->Address.StartingVpn;
    CapturedCopyOnWrite = Section->u.Flags.CopyOnWrite;

    //
    // If the requested size and start address are large page multiples and
    // this is a large page segment, then map it with large pages.
    // Otherwise, fall back to small pages.
    //

    TryLargePages = FALSE;

    if ((Segment->SegmentFlags.LargePages == 1) &&
        ((*CapturedViewSize % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0) &&
        ((AllocationType & MEM_RESERVE) == 0) &&
#ifdef _X86_
        (KeFeatureBits & KF_LARGE_PAGE) &&
#endif
        (!MI_IS_GUARD (ProtectionMask)) &&
        (ProtectionMask != MM_NOACCESS) &&
        (MI_IS_PTE_PROTECTION_COPY_WRITE (ProtectionMask) == 0)) {

        TryLargePages = TRUE;
    }

    if ((*CapturedBase == NULL) && (CapturedStartingVa == NULL)) {

        //
        // The section is not based,
        // find an empty range starting on a 64k boundary.
        //

        if ((AllocationType & MEM_TOP_DOWN) || (Process->VmTopDown == 1)) {

            if (ZeroBits != 0) {
                HighestUserAddress = (PVOID)((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits);
                if (HighestUserAddress > MM_HIGHEST_VAD_ADDRESS) {
                    HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
                }
            }
            else {
                HighestUserAddress = MM_HIGHEST_VAD_ADDRESS;
            }

            if (TryLargePages == TRUE) {
                Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                      *CapturedViewSize,
                                                      HighestUserAddress,
                                                      MM_MINIMUM_VA_FOR_LARGE_PAGE,
                                                      &StartingAddress);
                if (NT_SUCCESS (Status)) {
                    goto FoundRange;
                }
                TryLargePages = FALSE;
            }

            Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                  *CapturedViewSize,
                                                  HighestUserAddress,
                                                  Alignment,
                                                  &StartingAddress);
        }
        else {

            if (TryLargePages == TRUE) {
                Status = MiFindEmptyAddressRange (*CapturedViewSize,
                                                  MM_MINIMUM_VA_FOR_LARGE_PAGE,
                                                  (ULONG)ZeroBits,
                                                  &StartingAddress);
                if (NT_SUCCESS (Status)) {
                    goto FoundRange;
                }
                TryLargePages = FALSE;
            }

            Status = MiFindEmptyAddressRange (*CapturedViewSize,
                                              Alignment,
                                              (ULONG)ZeroBits,
                                              &StartingAddress);
        }

        if (!NT_SUCCESS (Status)) {
            if (ControlArea->FilePointer != NULL) {
                MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                                 LastPteOffset);
            }

            MiDereferenceControlArea (ControlArea);
            return Status;
        }

FoundRange:
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                    *CapturedViewSize - 1L) | (PAGE_SIZE - 1L));

        if (ZeroBits != 0) {
            if (EndingAddress > (PVOID)((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT >> ZeroBits)) {
                if (ControlArea->FilePointer != NULL) {
                    MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                                     LastPteOffset);
                }
                MiDereferenceControlArea (ControlArea);
                return STATUS_NO_MEMORY;
            }
        }
    }
    else {

        if (*CapturedBase == NULL) {

            //
            // The section is based.
            //

#if defined(_WIN64)
            SizeOfSection = SectionOffset->QuadPart;
#else
            SizeOfSection = SectionOffset->LowPart;
#endif

            StartingAddress = (PVOID)((PCHAR)CapturedStartingVa + SizeOfSection);
        }
        else {
            StartingAddress = MI_ALIGN_TO_SIZE (*CapturedBase, Alignment);
        }

        if ((ULONG_PTR) StartingAddress % MM_MINIMUM_VA_FOR_LARGE_PAGE) {
            TryLargePages = FALSE;
        }

        //
        // Check to make sure the specified base address to ending address
        // is currently unused.
        //

        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                   *CapturedViewSize - 1L) | (PAGE_SIZE - 1L));

        if (MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress) == TRUE) {
            if (ControlArea->FilePointer != NULL) {
                MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                                 LastPteOffset);
            }
            MiDereferenceControlArea (ControlArea);

            return STATUS_CONFLICTING_ADDRESSES;
        }
    }

    //
    // An unoccupied address range has been found, build the virtual
    // address descriptor to describe this range.
    //

    if (AllocationType & MEM_RESERVE) {
        VadSize = sizeof (MMVAD_LONG);
        Vad = ExAllocatePoolWithTag (NonPagedPool, VadSize, 'ldaV');
    }
    else {
        VadSize = sizeof (MMVAD);
        Vad = ExAllocatePoolWithTag (NonPagedPool, VadSize, MMVADKEY);
    }

    if (Vad == NULL) {
        if (ControlArea->FilePointer != NULL) {
            MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                             LastPteOffset);
        }
        MiDereferenceControlArea (ControlArea);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (Vad, VadSize);

    Vad->StartingVpn = MI_VA_TO_VPN (StartingAddress);
    Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);
    Vad->FirstPrototypePte = TheFirstPrototypePte;

    //
    // Set the protection in the PTE template field of the VAD.
    //

    Vad->ControlArea = ControlArea;

    Vad->u2.VadFlags2.Inherit = (InheritDisposition == ViewShare);
    Vad->u.VadFlags.Protection = ProtectionMask;
    Vad->u2.VadFlags2.CopyOnWrite = CapturedCopyOnWrite;

    //
    // Note that MEM_DOS_LIM significance is lost here, but those
    // files are not mapped MEM_RESERVE.
    //

    Vad->u2.VadFlags2.FileOffset = (ULONG)(SectionOffset->QuadPart >> 16);

    if ((AllocationType & SEC_NO_CHANGE) || (Section->u.Flags.NoChange)) {
        Vad->u.VadFlags.NoChange = 1;
        Vad->u2.VadFlags2.SecNoChange = 1;
    }

    if (AllocationType & MEM_RESERVE) {
        PMMEXTEND_INFO ExtendInfo;
        PMMVAD_LONG VadLong;

        Vad->u2.VadFlags2.LongVad = 1;

        KeAcquireGuardedMutexUnsafe (&MmSectionBasedMutex);
        ExtendInfo = Segment->ExtendInfo;
        if (ExtendInfo) {
            ExtendInfo->ReferenceCount += 1;
        }
        else {

            ExtendInfo = ExAllocatePoolWithTag (NonPagedPool,
                                                sizeof(MMEXTEND_INFO),
                                                'xCmM');
            if (ExtendInfo == NULL) {
                KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);

                if (ControlArea->FilePointer != NULL) {
                    MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                                     LastPteOffset);
                }
                MiDereferenceControlArea (ControlArea);
        
                //
                // The pool allocation succeeded, but the quota charge
                // in InsertVad failed, deallocate the pool and return
                // an error.
                //
    
                ExFreePool (Vad);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            ExtendInfo->ReferenceCount = 1;
            ExtendInfo->CommittedSize = Segment->SizeOfSegment;
            Segment->ExtendInfo = ExtendInfo;
        }
        if (ExtendInfo->CommittedSize < (UINT64)Section->SizeOfSection.QuadPart) {
            ExtendInfo->CommittedSize = (UINT64)Section->SizeOfSection.QuadPart;
        }
        KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);
        Vad->u2.VadFlags2.ExtendableFile = 1;

        VadLong = (PMMVAD_LONG) Vad;

        ASSERT (VadLong->u4.ExtendedInfo == NULL);
        VadLong->u4.ExtendedInfo = ExtendInfo;
    }

    //
    // If the page protection is write-copy or execute-write-copy
    // charge for each page in the view as it may become private.
    //

    if (MI_IS_PTE_PROTECTION_COPY_WRITE(ProtectionMask)) {
        Vad->u.VadFlags.CommitCharge = (BYTES_TO_PAGES ((PCHAR) EndingAddress -
                           (PCHAR) StartingAddress));
    }

    PteOffset += (Vad->EndingVpn - Vad->StartingVpn);

    if (PteOffset < Subsection->PtesInSubsection) {
        Vad->LastContiguousPte = &Subsection->SubsectionBase[PteOffset];

    }
    else {
        Vad->LastContiguousPte = &Subsection->SubsectionBase[
                                    (Subsection->PtesInSubsection - 1) +
                                    Subsection->UnusedPtes];
    }

    if (QuotaCharge != 0) {
        if (MiChargeCommitment (QuotaCharge, NULL) == FALSE) {
            if (ControlArea->FilePointer != NULL) {
                MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                                 LastPteOffset);
            }
            MiDereferenceControlArea (ControlArea);
    
            ExFreePool (Vad);
            return STATUS_COMMITMENT_LIMIT;
        }
    }

#if defined (_WIN64)
    //
    // See if ample pages for the page table pages as the target pages
    // will be mapped physically with large pages and thus never be paged.
    //

    if (TryLargePages == TRUE) {

        TryLargePages = MiCreatePageDirectoriesForPhysicalRange (Process,
                                                                 StartingAddress,
                                                                 EndingAddress);

        if (TryLargePages == FALSE) {
            MiDereferenceControlArea (ControlArea);
            ExFreePool (Vad);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
#endif

    ASSERT (Vad->FirstPrototypePte <= Vad->LastContiguousPte);

    Thread = PsGetCurrentThread ();

    Status = MiInsertVadCharges (Vad, Process);

    if (!NT_SUCCESS (Status)) {

        if (ControlArea->FilePointer != NULL) {
            MiRemoveViewsFromSectionWithPfn ((PMSUBSECTION)Subsection,
                                             LastPteOffset);
        }
#if defined (_WIN64)
        else if (TryLargePages == TRUE) {
            LOCK_WS_UNSAFE (Thread, Process);
            MiDeletePageDirectoriesForPhysicalRange (StartingAddress,
                                                     EndingAddress);
            UNLOCK_WS_UNSAFE (Thread, Process);
        }
#endif

        MiDereferenceControlArea (ControlArea);

        //
        // The quota charge failed, deallocate the pool and return an error.
        //

        ExFreePool (Vad);
        if (QuotaCharge != 0) {
            MiReturnCommitment (QuotaCharge);
        }
        return Status;
    }

    LOCK_WS_UNSAFE (Thread, Process);

    MiInsertVad (Vad, Process);

    UNLOCK_WS_UNSAFE (Thread, Process);

    if (TryLargePages == TRUE) {
        MiMapLargePageSection (Process,
                               StartingAddress,
                               EndingAddress,
                               Vad,
                               ProtectionMask);
    }

    //
    // Stash the first mapped address for the performance analysis tools.
    // Note this is not synchronized across multiple processes but that's ok.
    //

    if (ControlArea->FilePointer == NULL) {
        if (Segment->u2.FirstMappedVa == NULL) {
            Segment->u2.FirstMappedVa = StartingAddress;
        }
    }

#if DBG
    if (!(AllocationType & MEM_RESERVE)) {
        ASSERT(((ULONG_PTR)EndingAddress - (ULONG_PTR)StartingAddress) <=
                ROUND_TO_PAGES64(Segment->SizeOfSegment));
    }
#endif

    //
    // If a commit size was specified, make sure those pages are committed.
    //

    if (QuotaCharge != 0) {

        PointerPte = Vad->FirstPrototypePte;
        LastPte = PointerPte + BYTES_TO_PAGES(CommitSize);
        TempPte = Segment->SegmentPteTemplate;
        QuotaExcess = 0;

        KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

        while (PointerPte < LastPte) {

            if (PointerPte->u.Long == 0) {

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            else {
                QuotaExcess += 1;
            }
            PointerPte += 1;
        }

        ASSERT (QuotaCharge >= QuotaExcess);
        QuotaCharge -= QuotaExcess;

        MM_TRACK_COMMIT (MM_DBG_COMMIT_MAPVIEW_DATA, QuotaCharge);

        Segment->NumberOfCommittedPages += QuotaCharge;

        ASSERT (Segment->NumberOfCommittedPages <= TotalNumberOfPtes);

        KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

        InterlockedExchangeAddSizeT (&MmSharedCommit, QuotaCharge);

        if (QuotaExcess != 0) {
            MiReturnCommitment (QuotaExcess);
        }
    }

    //
    // Update the current virtual size in the process header.
    //

    *CapturedViewSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
    Process->VirtualSize += *CapturedViewSize;

    if (Process->VirtualSize > Process->PeakVirtualSize) {
        Process->PeakVirtualSize = Process->VirtualSize;
    }

    *CapturedBase = StartingAddress;

    //
    // Update the count of writable user mappings so the transaction semantics
    // can be supported.  Note that no lock synchronization is needed here as
    // the transaction manager must already check for any open writable handles
    // to the file - and no writable sections can be created without a writable
    // file handle.  So all that needs to be provided is a way for the
    // transaction manager to know that there are lingering views or created
    // sections still open that have write access.
    //

    if (((ProtectionMask == MM_READWRITE) || (ProtectionMask == MM_EXECUTE_READWRITE))
            &&
        (ControlArea->FilePointer != NULL)) {

        InterlockedIncrement ((PLONG)&ControlArea->WritableUserReferences);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiCheckPurgeAndUpMapCount (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL FailIfSystemViews
    )

/*++

Routine Description:

    This routine synchronizes with any on going purge operations
    on the same segment (identified via the control area).  If
    another purge operation is occurring, the function blocks until
    it is completed.

    When this function returns the MappedView and the NumberOfUserReferences
    count for the control area will be incremented thereby referencing
    the control area.

Arguments:

    ControlArea - Supplies the control area for the segment to be purged.

    FailIfSystemViews - Supplies TRUE if the request should fail when there
                        are active kernel mappings.  When the image is mapped
                        in system or session space as a driver, copy on write
                        does not work so user processes are not allowed to
                        map the image.

Return Value:

    STATUS_SUCCESS if the synchronization was successful.
    NTSTATUS if the synchronization did not occur due to low resources, etc.

Environment:

    Kernel Mode.

--*/

{
    NTSTATUS Status;
    KIRQL OldIrql;
    PEVENT_COUNTER PurgedEvent;
    PEVENT_COUNTER WaitEvent;
    PKTHREAD CurrentThread;

    PurgedEvent = NULL;

    if (FailIfSystemViews == TRUE) {
        ASSERT (ControlArea->u.Flags.Image != 0);
    }

    LOCK_PFN (OldIrql);

    while (ControlArea->u.Flags.BeingPurged != 0) {

        //
        // A purge operation is in progress.
        //

        if (PurgedEvent == NULL) {

            //
            // Release the locks and allocate pool for the event.
            //

            UNLOCK_PFN (OldIrql);
            PurgedEvent = MiGetEventCounter ();
            if (PurgedEvent == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            LOCK_PFN (OldIrql);
            continue;
        }

        if (ControlArea->WaitingForDeletion == NULL) {
            ControlArea->WaitingForDeletion = PurgedEvent;
            WaitEvent = PurgedEvent;
            PurgedEvent = NULL;
        }
        else {
            WaitEvent = ControlArea->WaitingForDeletion;

            //
            // No interlock is needed for the RefCount increment as
            // no thread can be decrementing it since it is still
            // pointed to by the control area.
            //

            WaitEvent->RefCount += 1;
        }

        //
        // Release the PFN lock and wait for the event.
        //

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);
        UNLOCK_PFN_AND_THEN_WAIT(OldIrql);

        KeWaitForSingleObject (&WaitEvent->Event,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        //
        // Before this event can be set, the control area WaitingForDeletion
        // field must be cleared (and may be reinitialized to something else),
        // but cannot be reset to our local event.  This allows us to
        // dereference the event count lock free.
        //

        ASSERT (WaitEvent != ControlArea->WaitingForDeletion);

        MiFreeEventCounter (WaitEvent);

        LOCK_PFN (OldIrql);
        KeLeaveCriticalRegionThread (CurrentThread);
    }

    if ((FailIfSystemViews == TRUE) &&
        (ControlArea->u.Flags.ImageMappedInSystemSpace) &&
        (KeGetPreviousMode() != KernelMode)) {

        UNLOCK_PFN (OldIrql);
        Status = STATUS_CONFLICTING_ADDRESSES;
    }
    else {

        //
        // Indicate another file is mapped for the segment.
        //

        ControlArea->NumberOfMappedViews += 1;
        ControlArea->NumberOfUserReferences += 1;
        ASSERT (ControlArea->NumberOfSectionReferences != 0);

        UNLOCK_PFN (OldIrql);
        Status = STATUS_SUCCESS;
    }

    if (PurgedEvent != NULL) {
        MiFreeEventCounter (PurgedEvent);
    }

    return Status;
}

typedef struct _NTSYM {
    struct _NTSYM *Next;
    PVOID SymbolTable;
    ULONG NumberOfSymbols;
    PVOID StringTable;
    USHORT Flags;
    USHORT EntrySize;
    ULONG MinimumVa;
    ULONG MaximumVa;
    PCHAR MapName;
    ULONG MapNameLen;
} NTSYM, *PNTSYM;

PVOID
MiCacheImageSymbols (
    IN PVOID ImageBase
    )
{
    ULONG DebugSize;
    PVOID DebugDirectory;

    PAGED_CODE ();

    DebugDirectory = NULL;

    try {

        DebugDirectory = RtlImageDirectoryEntryToData (ImageBase,
                                                       TRUE,
                                                       IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                       &DebugSize);
        //
        // If using remote KD, ImageBase is what it wants to see.
        //

    } except (EXCEPTION_EXECUTE_HANDLER) {

        NOTHING;
    }

    return DebugDirectory;
}


NTSYSAPI
NTSTATUS
NTAPI
NtAreMappedFilesTheSame (
    __in PVOID File1MappedAsAnImage,
    __in PVOID File2MappedAsFile
    )

/*++

Routine Description:

    This routine compares the two files mapped at the specified
    addresses to see if they are both the same file.

Arguments:

    File1MappedAsAnImage - Supplies an address within the first file which
        is mapped as an image file.

    File2MappedAsFile - Supplies an address within the second file which
        is mapped as either an image file or a data file.

Return Value:


    STATUS_SUCCESS is returned if the two files are the same.

    STATUS_NOT_SAME_DEVICE is returned if the files are different.

    Other status values can be returned if the addresses are not mapped as
    files, etc.

Environment:

    User mode callable system service.

--*/

{
    PMMVAD FoundVad1;
    PMMVAD FoundVad2;
    NTSTATUS Status;
    PEPROCESS Process;
    PCONTROL_AREA ControlArea1;
    PCONTROL_AREA ControlArea2;
    LOGICAL DummyGlobalNeeded;

    Process = PsGetCurrentProcess();

    LOCK_ADDRESS_SPACE (Process);

    FoundVad1 = MiLocateAddress (File1MappedAsAnImage);
    FoundVad2 = MiLocateAddress (File2MappedAsFile);

    if ((FoundVad1 == NULL) || (FoundVad2 == NULL)) {

        //
        // No virtual address is allocated at the specified base address,
        // return an error.
        //

        Status = STATUS_INVALID_ADDRESS;
        goto ErrorReturn;
    }

    //
    // Check file names.
    //

    if ((FoundVad1->u.VadFlags.PrivateMemory == 1) ||
        (FoundVad2->u.VadFlags.PrivateMemory == 1)) {
        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn;
    }

    ControlArea1 = FoundVad1->ControlArea;
    ControlArea2 = FoundVad2->ControlArea;

    if ((ControlArea1 == NULL) || (ControlArea2 == NULL)) {
        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn;
    }
    
    if ((ControlArea1->FilePointer == NULL) ||
        (ControlArea2->FilePointer == NULL)) {
        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn;
    }

    //
    // Be sure the correct image section object is retrieved for the file
    // mapped as either data or an image, particularly in the case of
    // multiple sessions. The actual list need only be searched if the
    // image section is flagged as global only per session, allowing
    // reduction of PFN lock hold times in the common case.
    //

    if ((PVOID) ControlArea1 == 
        ControlArea2->FilePointer->SectionObjectPointer->ImageSectionObject) {
        Status = STATUS_SUCCESS;
    }
    else if ((ControlArea1->u.Flags.GlobalOnlyPerSession == 1) && 
             ((PVOID) ControlArea1 == MiFindImageSectionObject (ControlArea2->FilePointer, FALSE, &DummyGlobalNeeded))) {    

        Status = STATUS_SUCCESS;
    }
    else {
        Status = STATUS_NOT_SAME_DEVICE;
    }

ErrorReturn:

    UNLOCK_ADDRESS_SPACE (Process);
    return Status;
}



NTSTATUS
MiSetPageModified (
    IN PMMVAD Vad,
    IN PVOID Address
    )

/*++

Routine Description:

    This routine sets the modified bit in the PFN database for the
    pages that correspond to the specified address range.

    Note that the dirty bit in the PTE is cleared by this operation.

Arguments:

    Vad - Supplies the VAD to charge.

    Address - Supplies the user address to access.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode.  Address space mutex held.

--*/

{
    PMMPFN Pfn1;
    NTSTATUS Status;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    SIZE_T RealCharge;
    MMPTE PteContents;
    KIRQL OldIrql;
    PMMPTE PointerPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    LOGICAL ReturnCommitment;
    LOGICAL ChargedJobCommit;

    //
    // Charge commitment up front even though we may not use it because
    // failing to get it later would make things messy.
    //

    RealCharge = 1;
    ReturnCommitment = FALSE;
    ChargedJobCommit = FALSE;

    CurrentThread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    Status = PsChargeProcessPageFileQuota (CurrentProcess, RealCharge);

    if (!NT_SUCCESS (Status)) {
        return STATUS_COMMITMENT_LIMIT;
    }

    if (CurrentProcess->CommitChargeLimit) {
        if (CurrentProcess->CommitCharge + RealCharge > CurrentProcess->CommitChargeLimit) {
            if (CurrentProcess->Job) {
                PsReportProcessMemoryLimitViolation ();
            }
            PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
            return STATUS_COMMITMENT_LIMIT;
        }
    }

    if (CurrentProcess->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {

        if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES,
                                    RealCharge) == FALSE) {

            PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
            return STATUS_COMMITMENT_LIMIT;
        }
        ChargedJobCommit = TRUE;
    }

    if (MiChargeCommitment (RealCharge, NULL) == FALSE) {
        if (ChargedJobCommit == TRUE) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES,
                                    -(SSIZE_T)RealCharge);
        }
        PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
        return STATUS_COMMITMENT_LIMIT;
    }

    CurrentProcess->CommitCharge += RealCharge;

    if (CurrentProcess->CommitCharge > CurrentProcess->CommitChargePeak) {
        CurrentProcess->CommitChargePeak = CurrentProcess->CommitCharge;
    }

    MI_INCREMENT_TOTAL_PROCESS_COMMIT (RealCharge);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_INSERT_VAD, RealCharge);

    PointerPte = MiGetPteAddress (Address);
    PointerPde = MiGetPdeAddress (Address);
    PointerPpe = MiGetPpeAddress (Address);
    PointerPxe = MiGetPxeAddress (Address);

    do {

        try {

            *(volatile CCHAR *)Address;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            CurrentProcess->CommitCharge -= RealCharge;
            if (ChargedJobCommit == TRUE) {
                PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES,
                                        -(SSIZE_T)RealCharge);
            }
            MiReturnCommitment (RealCharge);
            PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
            return GetExceptionCode ();
        }

        LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

#if (_MI_PAGING_LEVELS >= 4)
        if ((PointerPxe->u.Hard.Valid == 0) ||
            (PointerPpe->u.Hard.Valid == 0) ||
            (PointerPde->u.Hard.Valid == 0) ||
            (PointerPte->u.Hard.Valid == 0))
#elif (_MI_PAGING_LEVELS >= 3)
        if ((PointerPpe->u.Hard.Valid == 0) ||
            (PointerPde->u.Hard.Valid == 0) ||
            (PointerPte->u.Hard.Valid == 0))
#else
        if ((PointerPde->u.Hard.Valid == 0) ||
            (PointerPte->u.Hard.Valid == 0))
#endif
        {
            //
            // Page is no longer valid.
            //

            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

            continue;
        }

        break;

    } while (TRUE);

    PteContents = *PointerPte;

    ASSERT (PteContents.u.Hard.Valid == 1);

    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

    LOCK_PFN (OldIrql);

    MI_SET_MODIFIED (Pfn1, 1, 0x8);

    if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {
        if (Pfn1->u3.e1.WriteInProgress == 0) {
            MiReleasePageFileSpace (Pfn1->OriginalPte);
            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }

        //
        // We didn't need to (and shouldn't have) charged commitment for
        // this page as it was already pagefile backed (ie: someone else
        // already charged commitment for it earlier).
        //

        ReturnCommitment = TRUE;
    }

#ifdef NT_UP
    if (MI_IS_PTE_DIRTY (PteContents)) {
#endif //NT_UP
        MI_SET_PTE_CLEAN (PteContents);

        //
        // Clear the dirty bit in the PTE so new writes can be tracked.
        //

        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

        MI_FLUSH_SINGLE_TB (Address, FALSE);
#ifdef NT_UP
    }
#endif //NT_UP

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

    if (ReturnCommitment == TRUE) {
        CurrentProcess->CommitCharge -= RealCharge;
        if (ChargedJobCommit == TRUE) {
            PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)RealCharge);
        }
        MiReturnCommitment (RealCharge);
        PsReturnProcessPageFileQuota (CurrentProcess, RealCharge);
    }
    else {

        //
        // Commit has been charged for the copied page, add the charge
        // to the VAD now so it is automatically returned when the VAD is
        // deleted later.
        //

        MM_TRACK_COMMIT (MM_DBG_COMMIT_IMAGE, 1);

        ASSERT (Vad->u.VadFlags.CommitCharge != MM_MAX_COMMIT);
        Vad->u.VadFlags.CommitCharge += 1;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MmMapViewInSystemSpace (
    __in PVOID Section,
    __deref_inout_bcount(*ViewSize) PVOID *MappedBase,
    __inout PSIZE_T ViewSize
    )

/*++

Routine Description:

    This routine maps the specified section into the system's address space.

Arguments:

    Section - Supplies a pointer to the section to map.

    *MappedBase - Returns the address where the section was mapped.

    ViewSize - Supplies the size of the view to map.  If this
               is specified as zero, the whole section is mapped.
               Returns the actual size mapped.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, APC_LEVEL or below.

--*/

{
    PMMSESSION  Session;

    PAGED_CODE();

    Session = &MmSession;

    return MiMapViewInSystemSpace (Section,
                                   Session,
                                   MappedBase,
                                   ViewSize);
}


NTSTATUS
MmMapViewInSessionSpace (
    __in PVOID Section,
    __deref_inout_bcount(*ViewSize) PVOID *MappedBase,
    __inout PSIZE_T ViewSize
    )

/*++

Routine Description:

    This routine maps the specified section into the current process's
    session address space.

Arguments:

    Section - Supplies a pointer to the section to map.

    *MappedBase - Returns the address where the section was mapped.

    ViewSize - Supplies the size of the view to map.  If this
               is specified as zero, the whole section is mapped.
               Returns the actual size mapped.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, APC_LEVEL or below.

--*/

{
    PMMSESSION Session;

    PAGED_CODE();

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    ASSERT (MmIsAddressValid(MmSessionSpace) == TRUE);
    Session = &MmSessionSpace->Session;

    return MiMapViewInSystemSpace (Section,
                                   Session,
                                   MappedBase,
                                   ViewSize);
}


NTSTATUS
MiMapViewInSystemSpace (
    IN PVOID Section,
    IN PMMSESSION Session,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    )

/*++

Routine Description:

    This routine maps the specified section into the system's address space.

Arguments:

    Section - Supplies a pointer to the section to map.

    Session - Supplies the session data structure for this view.

    *MappedBase - Returns the address where the section was mapped.

    ViewSize - Supplies the size of the view to map.  If this
               is specified as zero, the whole section is mapped.
               Returns the actual size mapped.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, IRQL of APC_LEVEL or below.
    
--*/

{
    PVOID Base;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PCONTROL_AREA NewControlArea;
    ULONG StartBit;
    ULONG SizeIn64k;
    ULONG NewSizeIn64k;
    PMMPTE BasePte;
    PFN_NUMBER NumberOfPtes;
    SIZE_T NumberOfBytes;
    SIZE_T SectionSize;
    NTSTATUS Status;

    PAGED_CODE();

    //
    // Check to see if a purge operation is in progress and if so, wait
    // for the purge to complete.  In addition, up the count of mapped
    // views for this control area.
    //

    ControlArea = ((PSECTION)Section)->Segment->ControlArea;

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

#if defined (_WIN64)
    SectionSize = ((PSECTION)Section)->SizeOfSection.QuadPart;
#else
    SectionSize = ((PSECTION)Section)->SizeOfSection.LowPart;
#endif

    if (*ViewSize == 0) {

        *ViewSize = SectionSize;

    }
    else if (*ViewSize > SectionSize) {

        //
        // Section offset or view size past size of section.
        //

        MiDereferenceControlArea (ControlArea);
        return STATUS_INVALID_VIEW_SIZE;
    }

    //
    // Calculate the first prototype PTE field in the Vad.
    //

    SizeIn64k = (ULONG)((*ViewSize / X64K) + ((*ViewSize & (X64K - 1)) != 0));

    //
    // 4GB-64K is the maximum individual view size allowed since we encode
    // this into the bottom 16 bits of the MMVIEW entry.
    //

    if (SizeIn64k >= X64K) {
        MiDereferenceControlArea (ControlArea);
        return STATUS_INVALID_VIEW_SIZE;
    }

    Base = MiInsertInSystemSpace (Session, SizeIn64k, ControlArea);

    if (Base == NULL) {
        MiDereferenceControlArea (ControlArea);
        return STATUS_NO_MEMORY;
    }

    NumberOfBytes = (SIZE_T)SizeIn64k * X64K;

    if (Session == &MmSession) {
        MiFillSystemPageDirectory (Base, NumberOfBytes);
        Status = STATUS_SUCCESS;
    }
    else {

        Status = MiSessionCommitPageTables (Base,
                                (PVOID)((ULONG_PTR)Base + NumberOfBytes));
    }

    if (!NT_SUCCESS (Status)) {

bail:

        MiDereferenceControlArea (ControlArea);

        StartBit = (ULONG) (((ULONG_PTR)Base - (ULONG_PTR)Session->SystemSpaceViewStart) >> 16);

        LOCK_SYSTEM_VIEW_SPACE (Session);

        NewSizeIn64k = MiRemoveFromSystemSpace (Session, Base, &NewControlArea);

        ASSERT (ControlArea == NewControlArea);
        ASSERT (SizeIn64k == NewSizeIn64k);

        RtlClearBits (Session->SystemSpaceBitMap, StartBit, SizeIn64k);

        UNLOCK_SYSTEM_VIEW_SPACE (Session);

        return Status;
    }

    BasePte = MiGetPteAddress (Base);
    NumberOfPtes = BYTES_TO_PAGES (*ViewSize);

    //
    // Setup PTEs to point to prototype PTEs.
    //

    Status = MiAddMappedPtes (BasePte, NumberOfPtes, ControlArea);

    if (!NT_SUCCESS (Status)) {
        goto bail;
    }

    *MappedBase = Base;

    return STATUS_SUCCESS;
}

NTSTATUS
MmGetSessionMappedViewInformation (
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG Length,
    IN PULONG SessionId OPTIONAL
    )

/*++

Routine Description:

    This function returns information about all the per-session mapped
    views in the system.

Arguments:

    SystemInformation - A pointer to a buffer which receives the specified
                        information.

    SystemInformationLength - Specifies the length in bytes of the system
                              information buffer.

    Length - Receives the number of bytes placed (or would have been placed)
             in the system information buffer.

    SessionId - Session Id (-1 indicates enumerate all sessions).

Environment:

    Kernel mode.

Return Value:

    Returns one of the following status codes:

        STATUS_SUCCESS - normal, successful completion.

        STATUS_INVALID_INFO_CLASS - The SystemInformationClass parameter
            did not specify a valid value.

        STATUS_INFO_LENGTH_MISMATCH - The value of the SystemInformationLength
            parameter did not match the length required for the information
            class requested by the SystemInformationClass parameter.

        STATUS_ACCESS_VIOLATION - Either the SystemInformation buffer pointer
            or the Length pointer value specified an invalid address.

        STATUS_WORKING_SET_QUOTA - The process does not have sufficient
            working set to lock the specified output structure in memory.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources exist
            for this request to complete.

--*/

{
    PMMSESSION Session;
    KAPC_STATE ApcState;
    PVOID MappedAddress;
    PVOID OpaqueSession;
    PVOID LockVariable;
    ULONG ContiguousFree;
    ULONG Index;
    ULONG Hint;
    ULONG TotalFree;
    ULONG TotalSize;
    ULONG CurrentSessionId;
    ULONG StartingIndex;
    NTSTATUS status;
    NTSTATUS status1;
    PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION SessionMappedViewInfo;

    *Length = 0;
    TotalSize = 0;
    status = STATUS_SUCCESS;
    SessionMappedViewInfo = NULL;

    if (SystemInformationLength > 0) {

        status1 = ExLockUserBuffer (SystemInformation,
                                    SystemInformationLength,
                                    KeGetPreviousMode(),
                                    IoWriteAccess,
                                    &MappedAddress,
                                    &LockVariable);

        if (!NT_SUCCESS(status1)) {
            return status1;
        }

    }
    else {

        //
        // This indicates the caller just wants to know the size of the
        // buffer to allocate but is not prepared to accept any data content
        // in this instance.
        //

        MappedAddress = NULL;
        LockVariable = NULL;
    }

    for (OpaqueSession = MmGetNextSession (NULL);
         OpaqueSession != NULL;
         OpaqueSession = MmGetNextSession (OpaqueSession)) {

        SessionMappedViewInfo = (PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION)
                                ((PUCHAR)MappedAddress + TotalSize);

        //
        // If a specific session was requested, only extract that one.
        //

        CurrentSessionId = MmGetSessionId (OpaqueSession);

        if ((*SessionId == 0xFFFFFFFF) || (CurrentSessionId == *SessionId)) {

            //
            // Attach to session now to perform operations...
            //

            if (NT_SUCCESS (MmAttachSession (OpaqueSession, &ApcState))) {

                //
                // Session is still alive so include it.
                //

                TotalSize += sizeof (SYSTEM_SESSION_MAPPED_VIEW_INFORMATION);

                if (TotalSize > SystemInformationLength) {

                    status = STATUS_INFO_LENGTH_MISMATCH;

                }
                else {

                    //
                    // Get mapped view information for each session.
                    //

                    Session = &MmSessionSpace->Session;

                    Hint = 0;
                    TotalFree = 0;

                    LOCK_SYSTEM_VIEW_SPACE (Session);

                    ContiguousFree = RtlFindLongestRunClear (
                                                    Session->SystemSpaceBitMap,
                                                    &StartingIndex);

                    do {
                        Index = RtlFindClearBits (Session->SystemSpaceBitMap,
                                                  1,
                                                  Hint);

                        if ((Index < Hint) || (Index == NO_BITS_FOUND)) {
                            break;
                        }

                        TotalFree += 1;
                        Hint = Index + 1;

                    } while (TRUE);

                    UNLOCK_SYSTEM_VIEW_SPACE (Session);

                    SessionMappedViewInfo->NumberOfBytesAvailable =
                            (SIZE_T)TotalFree * X64K;

                    SessionMappedViewInfo->NumberOfBytesAvailableContiguous =
                            (SIZE_T)ContiguousFree * X64K;

                    SessionMappedViewInfo->SessionId = CurrentSessionId;
                    SessionMappedViewInfo->ViewFailures = Session->BitmapFailures;

                    //
                    // Point to next session.
                    //

                    SessionMappedViewInfo->NextEntryOffset = TotalSize;
                }

                //
                // Detach from session.
                //

                MmDetachSession (OpaqueSession, &ApcState);
            }

            //
            // Bail if only this session was of interest.
            //

            if (*SessionId != 0xFFFFFFFF) {
                MmQuitNextSession (OpaqueSession);
                break;
            }
        }
    }

    if ((NT_SUCCESS (status)) && (SessionMappedViewInfo != NULL)) {
        SessionMappedViewInfo->NextEntryOffset = 0;
    }

    if (MappedAddress != NULL) {
        ExUnlockUserBuffer (LockVariable);
    }

    *Length = TotalSize;

    return status;
}

VOID
MiFillSystemPageDirectory (
    IN PVOID Base,
    IN SIZE_T NumberOfBytes
    )

/*++

Routine Description:

    This routine allocates page tables and fills the system page directory
    entries for the specified virtual address range.

Arguments:

    Base - Supplies the virtual address of the view.

    NumberOfBytes - Supplies the number of bytes the view spans.

Return Value:

    None.

Environment:

    Kernel Mode, IRQL of dispatch level.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

--*/

{
    PMMPTE FirstPde;
    PMMPTE LastPde;
    PMMPTE FirstSystemPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
#if (_MI_PAGING_LEVELS < 3)
    ULONG i;
#endif

    PAGED_CODE();

    //
    // CODE IS ALREADY LOCKED BY CALLER.
    //

    FirstPde = MiGetPdeAddress (Base);
    LastPde = MiGetPdeAddress ((PVOID)(((PCHAR)Base) + NumberOfBytes - 1));

    PointerPpe = MiGetPpeAddress (Base);
    PointerPxe = MiGetPxeAddress (Base);

#if (_MI_PAGING_LEVELS >= 3)
    FirstSystemPde = FirstPde;
#else
    FirstSystemPde = &MmSystemPagePtes[((ULONG_PTR)FirstPde &
                     (PD_PER_SYSTEM * (PDE_PER_PAGE * sizeof(MMPTE)) - 1)) / sizeof(MMPTE) ];
#endif

    do {

#if (_MI_PAGING_LEVELS >= 4)
        if (PointerPxe->u.Hard.Valid == 0) {

            //
            // No page directory page exists, get a page and map it in.
            //

            TempPte = ValidKernelPde;

            LOCK_PFN (OldIrql);

            if (PointerPxe->u.Hard.Valid == 0) {

                if ((MmAvailablePages < MM_HIGH_LIMIT) &&
                    (MiEnsureAvailablePageOrWait (NULL, OldIrql))) {

                    //
                    // PFN_LOCK was dropped, redo this loop as another process
                    // could have made this PDE valid.
                    //

                    UNLOCK_PFN (OldIrql);
                    continue;
                }

                MiChargeCommitmentCantExpand (1, TRUE);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY, 1);
                PageFrameIndex = MiRemoveZeroPage (
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPxe));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPxe, TempPte);
            }
            UNLOCK_PFN (OldIrql);
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)
        if (PointerPpe->u.Hard.Valid == 0) {

            //
            // No page directory page exists, get a page and map it in.
            //

            TempPte = ValidKernelPde;

            LOCK_PFN (OldIrql);

            if (PointerPpe->u.Hard.Valid == 0) {

                if ((MmAvailablePages < MM_HIGH_LIMIT) &&
                    (MiEnsureAvailablePageOrWait (NULL, OldIrql))) {

                    //
                    // PFN_LOCK was dropped, redo this loop as another process
                    // could have made this PDE valid.
                    //

                    UNLOCK_PFN (OldIrql);
                    continue;
                }

                MiChargeCommitmentCantExpand (1, TRUE);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY, 1);
                PageFrameIndex = MiRemoveZeroPage (
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPpe));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPpe, TempPte);

            }
            UNLOCK_PFN (OldIrql);
        }
#endif

        if (FirstSystemPde->u.Hard.Valid == 0) {

            //
            // No page table page exists, get a page and map it in.
            //

            TempPte = ValidKernelPde;

            LOCK_PFN (OldIrql);

            if (FirstSystemPde->u.Hard.Valid == 0) {

                if ((MmAvailablePages < MM_HIGH_LIMIT) &&
                    (MiEnsureAvailablePageOrWait (NULL, OldIrql))) {

                    //
                    // PFN_LOCK was dropped, redo this loop as another process
                    // could have made this PDE valid.
                    //

                    UNLOCK_PFN (OldIrql);
                    continue;
                }

                MiChargeCommitmentCantExpand (1, TRUE);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY, 1);
                PageFrameIndex = MiRemoveZeroPage (
                                    MI_GET_PAGE_COLOR_FROM_PTE (FirstSystemPde));
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

#if (_MI_PAGING_LEVELS >= 3)
                MiInitializePfnAndMakePteValid (PageFrameIndex, FirstPde, TempPte);
                ASSERT (FirstPde->u.Hard.Valid == 1);
                ASSERT (FirstSystemPde->u.Hard.Valid == 1);
#else
                i = (FirstPde - MiGetPdeAddress(0)) / PDE_PER_PAGE;
                MiInitializePfnForOtherProcess (PageFrameIndex,
                                                FirstPde,
                                                MmSystemPageDirectory[i]);

                //
                // The FirstPde and FirstSystemPde may be identical even on
                // 32-bit machines if we are currently running in the
                // system process, so check for the valid bit first so we
                // don't assert on a checked build.
                //

                MI_WRITE_VALID_PTE (FirstSystemPde, TempPte);

                if (FirstPde->u.Hard.Valid == 0) {
                    MI_WRITE_VALID_PTE (FirstPde, TempPte);
                }
#endif
            }
            UNLOCK_PFN (OldIrql);
        }

        FirstSystemPde += 1;
        FirstPde += 1;
#if (_MI_PAGING_LEVELS >= 3)
        if (MiIsPteOnPdeBoundary (FirstPde)) {
            PointerPpe = MiGetPteAddress (FirstPde);
            if (MiIsPteOnPpeBoundary (FirstPde)) {
                PointerPxe = MiGetPdeAddress (FirstPde);
            }
        }
#endif
    } while (FirstPde <= LastPde);
}

NTSTATUS
MmUnmapViewInSystemSpace (
    __in PVOID MappedBase
    )

/*++

Routine Description:

    This routine unmaps the specified section from the system's address space.

Arguments:

    MappedBase - Supplies the address of the view to unmap.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, IRQL of dispatch level.

--*/

{
    PAGED_CODE();

    return MiUnmapViewInSystemSpace (&MmSession, MappedBase);
}

NTSTATUS
MmUnmapViewInSessionSpace (
    __in PVOID MappedBase
    )

/*++

Routine Description:

    This routine unmaps the specified section from the system's address space.

Arguments:

    MappedBase - Supplies the address of the view to unmap.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, IRQL of dispatch level.

--*/

{
    PMMSESSION Session;

    PAGED_CODE();

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    ASSERT (MmIsAddressValid(MmSessionSpace) == TRUE);
    Session = &MmSessionSpace->Session;

    return MiUnmapViewInSystemSpace (Session, MappedBase);
}

NTSTATUS
MiUnmapViewInSystemSpace (
    IN PMMSESSION Session,
    IN PVOID MappedBase
    )

/*++

Routine Description:

    This routine unmaps the specified section from the system's address space.

Arguments:

    Session - Supplies the session data structure for this view.

    MappedBase - Supplies the address of the view to unmap.

Return Value:

    Status of the map view operation.

Environment:

    Kernel Mode, IRQL of dispatch level.

--*/

{
    ULONG StartBit;
    ULONG Size;
    PCONTROL_AREA ControlArea;
    PMMSUPPORT Ws;

    PAGED_CODE();

    if (Session == &MmSession) {
        Ws = &MmSystemCacheWs;
    }
    else {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
    }

    StartBit =  (ULONG) (((ULONG_PTR)MappedBase - (ULONG_PTR)Session->SystemSpaceViewStart) >> 16);

    LOCK_SYSTEM_VIEW_SPACE (Session);

    Size = MiRemoveFromSystemSpace (Session, MappedBase, &ControlArea);

    RtlClearBits (Session->SystemSpaceBitMap, StartBit, Size);

    //
    // Zero PTEs.
    //

    Size = Size * (X64K >> PAGE_SHIFT);

    MiRemoveMappedPtes (MappedBase, Size, ControlArea, Ws);

    UNLOCK_SYSTEM_VIEW_SPACE (Session);

    return STATUS_SUCCESS;
}


PVOID
MiInsertInSystemSpace (
    IN PMMSESSION Session,
    IN ULONG SizeIn64k,
    IN PCONTROL_AREA ControlArea
    )

/*++

Routine Description:

    This routine creates a view in system space for the specified control
    area (file mapping).

Arguments:

    SizeIn64k - Supplies the size of the view to be created.

    ControlArea - Supplies a pointer to the control area for this view.

Return Value:

    Base address where the view was mapped, NULL if the view could not be
    mapped.

Environment:

    Kernel Mode.

--*/

{

    PVOID Base;
    ULONG_PTR Entry;
    ULONG Hash;
    ULONG i;
    ULONG AllocSize;
    PMMVIEW OldTable;
    ULONG StartBit;
    ULONG NewHashSize;
    POOL_TYPE PoolType;

    PAGED_CODE();

    ASSERT (SizeIn64k < X64K);

    //
    // CODE IS ALREADY LOCKED BY CALLER.
    //

    LOCK_SYSTEM_VIEW_SPACE (Session);

    if (Session->SystemSpaceHashEntries + 8 > Session->SystemSpaceHashSize) {

        //
        // Less than 8 free slots, reallocate and rehash.
        //

        NewHashSize = Session->SystemSpaceHashSize << 1;

        AllocSize = sizeof(MMVIEW) * NewHashSize;

        OldTable = Session->SystemSpaceViewTable;

        //
        // The SystemSpaceViewTable for system (not session) space is only
        // allocated from nonpaged pool so it can be safely torn down during
        // clean shutdowns.  Otherwise it could be allocated from paged pool
        // just like the session SystemSpaceViewTable.
        //

        if (Session == &MmSession) {
            PoolType = NonPagedPool;
        }
        else {
            PoolType = PagedPool;
        }

        Session->SystemSpaceViewTable = ExAllocatePoolWithTag (PoolType,
                                                               AllocSize,
                                                               '  mM');

        if (Session->SystemSpaceViewTable == NULL) {
            Session->SystemSpaceViewTable = OldTable;
        }
        else {
            RtlZeroMemory (Session->SystemSpaceViewTable, AllocSize);

            Session->SystemSpaceHashSize = NewHashSize;
            Session->SystemSpaceHashKey = Session->SystemSpaceHashSize - 1;

            for (i = 0; i < (Session->SystemSpaceHashSize / 2); i += 1) {
                if (OldTable[i].Entry != 0) {
                    Hash = (ULONG) ((OldTable[i].Entry >> 16) % Session->SystemSpaceHashKey);

                    while (Session->SystemSpaceViewTable[Hash].Entry != 0) {
                        Hash += 1;
                        if (Hash >= Session->SystemSpaceHashSize) {
                            Hash = 0;
                        }
                    }
                    Session->SystemSpaceViewTable[Hash] = OldTable[i];
                }
            }
            ExFreePool (OldTable);
        }
    }

    if (Session->SystemSpaceHashEntries == Session->SystemSpaceHashSize) {

        //
        // There are no free hash slots to place a new entry into even
        // though there may still be unused virtual address space.
        //

        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return NULL;
    }

    StartBit = RtlFindClearBitsAndSet (Session->SystemSpaceBitMap,
                                       SizeIn64k,
                                       0);

    if (StartBit == NO_BITS_FOUND) {
        Session->BitmapFailures += 1;
        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return NULL;
    }

    Base = (PVOID)((PCHAR)Session->SystemSpaceViewStart + ((ULONG_PTR)StartBit * X64K));

    Entry = (ULONG_PTR) MI_64K_ALIGN(Base) + SizeIn64k;

    Hash = (ULONG) ((Entry >> 16) % Session->SystemSpaceHashKey);

    while (Session->SystemSpaceViewTable[Hash].Entry != 0) {
        Hash += 1;
        if (Hash >= Session->SystemSpaceHashSize) {
            Hash = 0;
        }
    }

    Session->SystemSpaceHashEntries += 1;

    Session->SystemSpaceViewTable[Hash].Entry = Entry;
    Session->SystemSpaceViewTable[Hash].ControlArea = ControlArea;

    UNLOCK_SYSTEM_VIEW_SPACE (Session);
    return Base;
}


ULONG
MiRemoveFromSystemSpace (
    IN PMMSESSION Session,
    IN PVOID Base,
    OUT PCONTROL_AREA *ControlArea
    )

/*++

Routine Description:

    This routine looks up the specified view in the system space hash
    table and unmaps the view from system space and the table.

Arguments:

    Session - Supplies the session data structure for this view.

    Base - Supplies the base address for the view.  If this address is
           NOT found in the hash table, the system bugchecks.

    ControlArea - Returns the control area corresponding to the base
                  address.

Return Value:

    Size of the view divided by 64k.

Environment:

    Kernel Mode, system view hash table locked.

--*/

{
    ULONG_PTR Base16;
    ULONG Hash;
    ULONG Size;
    ULONG count;

    PAGED_CODE();

    count = 0;

    Base16 = (ULONG_PTR)Base >> 16;
    Hash = (ULONG)(Base16 % Session->SystemSpaceHashKey);

    while ((Session->SystemSpaceViewTable[Hash].Entry >> 16) != Base16) {
        Hash += 1;
        if (Hash >= Session->SystemSpaceHashSize) {
            Hash = 0;
            count += 1;
            if (count == 2) {
                KeBugCheckEx (DRIVER_UNMAPPING_INVALID_VIEW,
                              (ULONG_PTR)Base,
                              1,
                              0,
                              0);
            }
        }
    }

    Session->SystemSpaceHashEntries -= 1;
    Size = (ULONG) (Session->SystemSpaceViewTable[Hash].Entry & 0xFFFF);
    Session->SystemSpaceViewTable[Hash].Entry = 0;
    *ControlArea = Session->SystemSpaceViewTable[Hash].ControlArea;
    return Size;
}


LOGICAL
MiInitializeSystemSpaceMap (
    PVOID InputSession OPTIONAL
    )

/*++

Routine Description:

    This routine initializes the tables for mapping views into system space.
    Views are kept in a multiple of 64k bytes in a growable hashed table.

Arguments:

    InputSession - Supplies NULL if this is the initial system session
                   (non-Hydra), a valid session pointer (the pointer must
                   be in global space, not session space) for Hydra session
                   initialization.

Return Value:

    TRUE on success, FALSE on failure.

Environment:

    Kernel Mode, initialization.

--*/

{
    SIZE_T AllocSize;
    SIZE_T Size;
    PCHAR ViewStart;
    PMMSESSION Session;
    POOL_TYPE PoolType;

    if (ARGUMENT_PRESENT (InputSession)) {
        Session = (PMMSESSION)InputSession;
        ViewStart = (PCHAR) MiSessionViewStart;
        Size = MmSessionViewSize;
    }
    else {
        Session = &MmSession;
        ViewStart = (PCHAR)MiSystemViewStart;
        Size = MmSystemViewSize;
    }

    //
    // We are passed a system global address for the address of the session.
    // Save a global pointer to the mutex below because multiple sessions will
    // generally give us a session-space (not a global space) pointer to the
    // MMSESSION in subsequent calls.  We need the global pointer for the mutex
    // field for the kernel primitives to work properly.
    //

    Session->SystemSpaceViewLockPointer = &Session->SystemSpaceViewLock;
    KeInitializeGuardedMutex (Session->SystemSpaceViewLockPointer);

    //
    // If the kernel image has not been biased to allow for 3gb of user space,
    // then the system space view starts at the defined place. Otherwise, it
    // starts 16mb above the kernel image.
    //

    Session->SystemSpaceViewStart = ViewStart;

    MiCreateBitMap (&Session->SystemSpaceBitMap, Size / X64K, NonPagedPool);
    if (Session->SystemSpaceBitMap == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        return FALSE;
    }

    RtlClearAllBits (Session->SystemSpaceBitMap);

    //
    // Build the view table.
    //

    Session->SystemSpaceHashSize = 31;
    Session->SystemSpaceHashKey = Session->SystemSpaceHashSize - 1;
    Session->SystemSpaceHashEntries = 0;

    AllocSize = sizeof(MMVIEW) * Session->SystemSpaceHashSize;
    ASSERT (AllocSize < PAGE_SIZE);

    //
    // The SystemSpaceViewTable for system (not session) space is only
    // allocated from nonpaged pool so it can be safely torn down during
    // clean shutdowns.  Otherwise it could be allocated from paged pool
    // just like the session SystemSpaceViewTable.
    //

    if (Session == &MmSession) {
        PoolType = NonPagedPool;
    }
    else {
        PoolType = PagedPool;
    }

    Session->SystemSpaceViewTable = ExAllocatePoolWithTag (PoolType,
                                                           AllocSize,
                                                           '  mM');

    if (Session->SystemSpaceViewTable == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_SESSION_PAGED_POOL);
        MiRemoveBitMap (&Session->SystemSpaceBitMap);
        return FALSE;
    }

    RtlZeroMemory (Session->SystemSpaceViewTable, AllocSize);

    return TRUE;
}


VOID
MiFreeSessionSpaceMap (
    VOID
    )

/*++

Routine Description:

    This routine frees the tables used for mapping session views.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel Mode.  The caller must be in the correct session context.

--*/

{
    PMMSESSION Session;

    PAGED_CODE();

    Session = &MmSessionSpace->Session;

    //
    // Check for leaks of objects in the view table.
    //

    if (Session->SystemSpaceViewTable && Session->SystemSpaceHashEntries) {

        KeBugCheckEx (SESSION_HAS_VALID_VIEWS_ON_EXIT,
                      (ULONG_PTR)MmSessionSpace->SessionId,
                      Session->SystemSpaceHashEntries,
                      (ULONG_PTR)&Session->SystemSpaceViewTable[0],
                      Session->SystemSpaceHashSize);
    }

    if (Session->SystemSpaceViewTable) {
        ExFreePool (Session->SystemSpaceViewTable);
        Session->SystemSpaceViewTable = NULL;
    }

    if (Session->SystemSpaceBitMap) {
        MiRemoveBitMap (&Session->SystemSpaceBitMap);
    }
}


HANDLE
MmSecureVirtualMemory (
    __in_bcount(Size) PVOID Address,
    __in SIZE_T Size,
    __in ULONG ProbeMode
    )

/*++

Routine Description:

    This routine probes the requested address range and protects
    the specified address range from having its protection made
    more restricted and being deleted.

    MmUnsecureVirtualMemory is used to allow the range to return
    to a normal state.

Arguments:

    Address - Supplies the base address to probe and secure.

    Size - Supplies the size of the range to secure.

    ProbeMode - Supplies one of PAGE_READONLY or PAGE_READWRITE.

Return Value:

    Returns a handle to be used to unsecure the range.
    If the range could not be locked because of protection
    problems or noncommitted memory, the value (HANDLE)0
    is returned.

Environment:

    Kernel Mode.

--*/

{
    return MiSecureVirtualMemory (Address, Size, ProbeMode, FALSE);
}


HANDLE
MiSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode,
    IN LOGICAL AddressSpaceMutexHeld
    )

/*++

Routine Description:

    This routine probes the requested address range and protects
    the specified address range from having its protection made
    more restricted and being deleted.

    MmUnsecureVirtualMemory is used to allow the range to return
    to a normal state.

Arguments:

    Address - Supplies the base address to probe and secure.

    Size - Supplies the size of the range to secure.

    ProbeMode - Supplies one of PAGE_READONLY or PAGE_READWRITE.

    AddressSpaceMutexHeld - Supplies TRUE if the mutex is already held, FALSE
                            if not.

Return Value:

    Returns a handle to be used to unsecure the range.
    If the range could not be locked because of protection
    problems or noncommitted memory, the value (HANDLE)0
    is returned.

Environment:

    Kernel Mode.

--*/

{
    PETHREAD Thread;
    ULONG_PTR EndAddress;
    PVOID StartAddress;
    CHAR Temp;
    ULONG Probe;
    HANDLE Handle;
    PMMVAD Vad;
    PMMVAD VadParent;
    PMMVAD_LONG NewVad;
    PMMSECURE_ENTRY Secure;
    PEPROCESS Process;
    PMMPTE PointerPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    ULONG Waited;
#if defined(_WIN64)
    ULONG_PTR PageSize;
#else
    #define PageSize PAGE_SIZE
#endif

    PAGED_CODE();

    if ((ULONG_PTR)Address + Size > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS || (ULONG_PTR)Address + Size <= (ULONG_PTR)Address) {
        return NULL;
    }

    Handle = NULL;

    Probe = (ProbeMode == PAGE_READONLY);

    Thread = PsGetCurrentThread ();
    Process = PsGetCurrentProcessByThread (Thread);

#if defined(_WIN64)
    if (Process->Wow64Process != NULL) {
        PageSize = PAGE_SIZE_X86NT;
    }
    else {
        PageSize = PAGE_SIZE;
    }
#endif

    StartAddress = Address;

    if (AddressSpaceMutexHeld == FALSE) {
        LOCK_ADDRESS_SPACE (Process);
    }

    //
    // Check for a private committed VAD first instead of probing to avoid all
    // the page faults and zeroing.  If we find one, then we run the PTEs
    // instead.
    //

    if (Size >= 64 * 1024) {
        EndAddress = (ULONG_PTR)StartAddress + Size - 1;
        Vad = MiLocateAddress (StartAddress);

        if (Vad == NULL) {
            goto Return1;
        }

        if ((Vad->u.VadFlags.VadType == VadAwe) ||
            (Vad->u.VadFlags.VadType == VadLargePages)) {
            goto Return1;
        }

        if (Vad->u.VadFlags.MemCommit == 0) {
            goto LongWay;
        }

        if (Vad->u.VadFlags.PrivateMemory == 0) {
            goto LongWay;
        }

        if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {
            goto LongWay;
        }

        ASSERT (Vad->u.VadFlags.Protection);

        if ((MI_VA_TO_VPN (StartAddress) < Vad->StartingVpn) ||
            (MI_VA_TO_VPN (EndAddress) > Vad->EndingVpn)) {
            goto Return1;
        }

        if (Vad->u.VadFlags.Protection == MM_NOACCESS) {
            goto LongWay;
        }

        if (ProbeMode == PAGE_READONLY) {
            if (Vad->u.VadFlags.Protection > MM_EXECUTE_WRITECOPY) {
                goto LongWay;
            }
        }
        else {
            if (Vad->u.VadFlags.Protection != MM_READWRITE &&
                Vad->u.VadFlags.Protection != MM_EXECUTE_READWRITE) {

                goto LongWay;
            }
        }

        PointerPde = MiGetPdeAddress (StartAddress);
        PointerPpe = MiGetPteAddress (PointerPde);
        PointerPxe = MiGetPdeAddress (PointerPde);
        PointerPte = MiGetPteAddress (StartAddress);
        LastPte = MiGetPteAddress ((PVOID)EndAddress);

        LOCK_WS_UNSAFE (Thread, Process);

        //
        // Check individual page permissions.
        //

        do {

            while (MiDoesPxeExistAndMakeValid (PointerPxe,
                                               Process,
                                               MM_NOIRQL,
                                               &Waited) == FALSE) {
                //
                // Extended page directory parent entry is empty, go
                // to the next one.
                //

                PointerPxe += 1;
                PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    UNLOCK_WS_UNSAFE (Thread, Process);
                    goto EditVad;
                }
            }

#if (_MI_PAGING_LEVELS >= 4)
            Waited = 0;
#endif

            while (MiDoesPpeExistAndMakeValid (PointerPpe,
                                               Process,
                                               MM_NOIRQL,
                                               &Waited) == FALSE) {
                //
                // Page directory parent entry is empty, go to the next one.
                //

                PointerPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    UNLOCK_WS_UNSAFE (Thread, Process);
                    goto EditVad;
                }
#if (_MI_PAGING_LEVELS >= 4)
                if (MiIsPteOnPdeBoundary (PointerPpe)) {
                    PointerPxe = MiGetPteAddress(PointerPpe);
                    Waited = 1;
                    goto restart;
                }
#endif
            }

#if (_MI_PAGING_LEVELS < 4)
            Waited = 0;
#endif

            while (MiDoesPdeExistAndMakeValid (PointerPde,
                                               Process,
                                               MM_NOIRQL,
                                               &Waited) == FALSE) {
                //
                // This page directory entry is empty, go to the next one.
                //

                PointerPde += 1;
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    UNLOCK_WS_UNSAFE (Thread, Process);
                    goto EditVad;
                }
#if (_MI_PAGING_LEVELS >= 3)
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    PointerPxe = MiGetPteAddress(PointerPpe);
                    Waited = 1;
                    goto restart;
                }
#endif
            }

            if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
                UNLOCK_WS_UNSAFE (Thread, Process);
                goto LongWay;
            }

#if (_MI_PAGING_LEVELS >= 3)
restart:
            PointerPxe = PointerPxe;        // satisfy the compiler
#endif

        } while (Waited != 0);

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPxe = MiGetPdeAddress (PointerPde);

                do {

                    while (MiDoesPxeExistAndMakeValid (PointerPxe,
                                                       Process,
                                                       MM_NOIRQL,
                                                       &Waited) == FALSE) {
                        //
                        // Page directory parent entry is empty, go to the next one.
                        //

                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            UNLOCK_WS_UNSAFE (Thread, Process);
                            goto EditVad;
                        }
                    }

#if (_MI_PAGING_LEVELS >= 4)
                    Waited = 0;
#endif

                    while (MiDoesPpeExistAndMakeValid (PointerPpe,
                                                       Process,
                                                       MM_NOIRQL,
                                                       &Waited) == FALSE) {
                        //
                        // Page directory parent entry is empty, go to the next one.
                        //

                        PointerPpe += 1;
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            UNLOCK_WS_UNSAFE (Thread, Process);
                            goto EditVad;
                        }
#if (_MI_PAGING_LEVELS >= 4)
                        if (MiIsPteOnPdeBoundary (PointerPpe)) {
                            PointerPxe = MiGetPteAddress (PointerPpe);
                            Waited = 1;
                            goto restart2;
                        }
#endif
                    }

#if (_MI_PAGING_LEVELS < 4)
                    Waited = 0;
#endif

                    while (MiDoesPdeExistAndMakeValid (PointerPde,
                                                       Process,
                                                       MM_NOIRQL,
                                                       &Waited) == FALSE) {
                        //
                        // This page directory entry is empty, go to the next one.
                        //

                        PointerPde += 1;
                        PointerPpe = MiGetPteAddress (PointerPde);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                        if (PointerPte > LastPte) {
                            UNLOCK_WS_UNSAFE (Thread, Process);
                            goto EditVad;
                        }
#if (_MI_PAGING_LEVELS >= 3)
                        if (MiIsPteOnPdeBoundary (PointerPde)) {
                            PointerPxe = MiGetPteAddress (PointerPpe);
                            Waited = 1;
                            goto restart2;
                        }
#endif
                    }

                    if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
                        UNLOCK_WS_UNSAFE (Thread, Process);
                        goto LongWay;
                    }

#if (_MI_PAGING_LEVELS >= 3)
restart2:
            PointerPxe = PointerPxe;        // satisfy the compiler
#endif
                } while (Waited != 0);
            }
            if (PointerPte->u.Long) {
                UNLOCK_WS_UNSAFE (Thread, Process);
                goto LongWay;
            }
            PointerPte += 1;
        }
        UNLOCK_WS_UNSAFE (Thread, Process);
    }
    else {
LongWay:

        //
        // Mark this thread as the address space mutex owner so it cannot
        // sneak its stack in as the argument region and trick us into
        // trying to grow it if the reference faults (as this would cause
        // a deadlock since this thread already owns the address space mutex).
        // Note this would have the side effect of not allowing this thread
        // to fault on guard pages in other data regions while the accesses
        // below are ongoing - but that could only happen in an APC and
        // those are blocked right now anyway.
        //

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        ASSERT (Thread->AddressSpaceOwner == 0);
        Thread->AddressSpaceOwner = 1;

        try {

            if (ProbeMode == PAGE_READONLY) {

                EndAddress = (ULONG_PTR)Address + Size - 1;
                EndAddress = (EndAddress & ~(PageSize - 1)) + PageSize;

                do {
                    Temp = *(volatile CHAR *)Address;
                    Address = (PVOID)(((ULONG_PTR)Address & ~(PageSize - 1)) + PageSize);
                } while ((ULONG_PTR)Address != EndAddress);
            }
            else {
                ProbeForWrite (Address, Size, 1);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            ASSERT (KeAreAllApcsDisabled () == TRUE);
            ASSERT (Thread->AddressSpaceOwner == 1);
            Thread->AddressSpaceOwner = 0;
            goto Return1;
        }

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        ASSERT (Thread->AddressSpaceOwner == 1);
        Thread->AddressSpaceOwner = 0;

        //
        // Locate VAD and add in secure descriptor.
        //

        EndAddress = (ULONG_PTR)StartAddress + Size - 1;
        Vad = MiLocateAddress (StartAddress);

        if (Vad == NULL) {
            goto Return1;
        }

        if ((Vad->u.VadFlags.VadType == VadAwe) ||
            (Vad->u.VadFlags.VadType == VadLargePages)) {
            goto Return1;
        }

        if ((MI_VA_TO_VPN (StartAddress) < Vad->StartingVpn) ||
            (MI_VA_TO_VPN (EndAddress) > Vad->EndingVpn)) {

            //
            // Not within the section virtual address descriptor,
            // return an error.
            //

            goto Return1;
        }
    }

EditVad:

    //
    // Round the start and end addresses to the appropriate page boundary
    // (factoring in NT64 wow64 process page size as well).  This is so
    // they can be properly compared during attempted protection changes.
    //

    EndAddress |= (PageSize - 1);

    StartAddress = MI_ALIGN_TO_SIZE (StartAddress, PageSize);

    //
    // If this is a short or regular VAD, it needs to be reallocated as
    // a large VAD.  Note that a short VAD that was previously converted
    // to a long VAD here will still be marked as private memory, thus to
    // handle this case the NoChange bit must also be tested.
    //

    if (((Vad->u.VadFlags.PrivateMemory) && (Vad->u.VadFlags.NoChange == 0)) 
        ||
        (Vad->u2.VadFlags2.LongVad == 0)) {

        if (Vad->u.VadFlags.PrivateMemory == 0) {
            ASSERT (Vad->u2.VadFlags2.OneSecured == 0);
            ASSERT (Vad->u2.VadFlags2.MultipleSecured == 0);
        }

        NewVad = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(MMVAD_LONG),
                                        'ldaV');
        if (NewVad == NULL) {
            goto Return1;
        }

        RtlZeroMemory (NewVad, sizeof(MMVAD_LONG));

        if (Vad->u.VadFlags.PrivateMemory) {
            RtlCopyMemory (NewVad, Vad, sizeof(MMVAD_SHORT));
        }
        else {
            RtlCopyMemory (NewVad, Vad, sizeof(MMVAD));
        }

        NewVad->u.VadFlags.NoChange = 1;
        NewVad->u2.VadFlags2.OneSecured = 1;

        NewVad->u2.VadFlags2.LongVad = 1;
        NewVad->u2.VadFlags2.ReadOnly = Probe;

        NewVad->u3.Secured.StartVpn = (ULONG_PTR)StartAddress;
        NewVad->u3.Secured.EndVpn = EndAddress;

        //
        // Replace the current VAD with this expanded VAD.
        //

        LOCK_WS_UNSAFE (Thread, Process);

        VadParent = (PMMVAD) SANITIZE_PARENT_NODE (Vad->u1.Parent);
        ASSERT (VadParent != NULL);

        if (VadParent != Vad) {
            if (VadParent->RightChild == Vad) {
                VadParent->RightChild = (PMMVAD) NewVad;
            }
            else {
                ASSERT (VadParent->LeftChild == Vad);
                VadParent->LeftChild = (PMMVAD) NewVad;
            }
        }
        else {
            Process->VadRoot.BalancedRoot.RightChild = (PMMADDRESS_NODE) NewVad;
        }
        if (Vad->LeftChild) {
            Vad->LeftChild->u1.Parent = (PMMVAD) MI_MAKE_PARENT (NewVad, Vad->LeftChild->u1.Balance);
        }
        if (Vad->RightChild) {
            Vad->RightChild->u1.Parent = (PMMVAD) MI_MAKE_PARENT (NewVad, Vad->RightChild->u1.Balance);
        }
        if (Process->VadRoot.NodeHint == Vad) {
            Process->VadRoot.NodeHint = (PMMVAD) NewVad;
        }
        if (Process->VadFreeHint == Vad) {
            Process->VadFreeHint = (PMMVAD) NewVad;
        }

        if ((Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) ||
            (Vad->u.VadFlags.VadType == VadRotatePhysical) ||
            (Vad->u.VadFlags.VadType == VadWriteWatch)) {

            MiPhysicalViewAdjuster (Process, Vad, (PMMVAD) NewVad);
        }

        UNLOCK_WS_UNSAFE (Thread, Process);
        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }

        ExFreePool (Vad);

        //
        // Or in the low bit to denote the secure entry is in the VAD.
        //

        Handle = (HANDLE)((ULONG_PTR)&NewVad->u2.LongFlags2 | 0x1);

        return Handle;
    }

    //
    // This is already a large VAD, add the secure entry.
    //

    ASSERT (Vad->u2.VadFlags2.LongVad == 1);

    if (Vad->u2.VadFlags2.OneSecured) {

        //
        // This VAD already is secured.  Move the info out of the
        // block into pool.
        //

        Secure = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof (MMSECURE_ENTRY),
                                        'eSmM');
        if (Secure == NULL) {
            goto Return1;
        }

        ASSERT (Vad->u.VadFlags.NoChange == 1);
        Vad->u2.VadFlags2.OneSecured = 0;
        Vad->u2.VadFlags2.MultipleSecured = 1;
        Secure->u2.LongFlags2 = (ULONG) Vad->u.LongFlags;
        Secure->StartVpn = ((PMMVAD_LONG) Vad)->u3.Secured.StartVpn;
        Secure->EndVpn = ((PMMVAD_LONG) Vad)->u3.Secured.EndVpn;

        InitializeListHead (&((PMMVAD_LONG)Vad)->u3.List);
        InsertTailList (&((PMMVAD_LONG)Vad)->u3.List, &Secure->List);
    }

    if (Vad->u2.VadFlags2.MultipleSecured) {

        //
        // This VAD already has a secured element in its list, allocate and
        // add in the new secured element.
        //

        Secure = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof (MMSECURE_ENTRY),
                                        'eSmM');
        if (Secure == NULL) {
            goto Return1;
        }

        Secure->u2.LongFlags2 = 0;
        Secure->u2.VadFlags2.ReadOnly = Probe;
        Secure->StartVpn = (ULONG_PTR)StartAddress;
        Secure->EndVpn = EndAddress;

        InsertTailList (&((PMMVAD_LONG)Vad)->u3.List, &Secure->List);
        Handle = (HANDLE)Secure;

    }
    else {

        //
        // This list does not have a secure element.  Put it in the VAD.
        // The VAD may be either a regular VAD or a long VAD (it cannot be
        // a short VAD) at this point.  If it is a regular VAD, it must be
        // reallocated as a long VAD before any operation can proceed so
        // the secured range can be inserted.
        //

        Vad->u.VadFlags.NoChange = 1;
        Vad->u2.VadFlags2.OneSecured = 1;
        Vad->u2.VadFlags2.ReadOnly = Probe;
        ((PMMVAD_LONG)Vad)->u3.Secured.StartVpn = (ULONG_PTR)StartAddress;
        ((PMMVAD_LONG)Vad)->u3.Secured.EndVpn = EndAddress;

        //
        // Or in the low bit to denote the secure entry is in the VAD.
        //

        Handle = (HANDLE)((ULONG_PTR)&Vad->u2.LongFlags2 | 0x1);
    }

Return1:
    if (AddressSpaceMutexHeld == FALSE) {
        UNLOCK_ADDRESS_SPACE (Process);
    }
    return Handle;
}


VOID
MmUnsecureVirtualMemory (
    __in HANDLE SecureHandle
    )

/*++

Routine Description:

    This routine unsecures memory previously secured via a call to
    MmSecureVirtualMemory.

Arguments:

    SecureHandle - Supplies the handle returned in MmSecureVirtualMemory.

Return Value:

    None.

Environment:

    Kernel Mode.

--*/

{
    MiUnsecureVirtualMemory (SecureHandle, FALSE);
}


VOID
MiUnsecureVirtualMemory (
    IN HANDLE SecureHandle,
    IN LOGICAL AddressSpaceMutexHeld
    )

/*++

Routine Description:

    This routine unsecures memory previously secured via a call to
    MmSecureVirtualMemory.

Arguments:

    SecureHandle - Supplies the handle returned in MmSecureVirtualMemory.

    AddressSpaceMutexHeld - Supplies TRUE if the mutex is already held, FALSE
                            if not.

Return Value:

    None.

Environment:

    Kernel Mode.

--*/

{
    PMMSECURE_ENTRY Secure;
    PEPROCESS Process;
    PMMVAD_LONG Vad;

    PAGED_CODE();

    Secure = (PMMSECURE_ENTRY)SecureHandle;
    Process = PsGetCurrentProcess ();

    if (AddressSpaceMutexHeld == FALSE) {
        LOCK_ADDRESS_SPACE (Process);
    }

    //
    // Make sure the address space was not deleted (ie, the thread that
    // secured the virtual memory was attached).  If the process has exited,
    // then our VAD (or MMSECURE) pointer has already been freed so just
    // return.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {

        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }

        return;
    }

    if ((ULONG_PTR)Secure & 0x1) {
        Secure = (PMMSECURE_ENTRY) ((ULONG_PTR)Secure & ~0x1);
        Vad = CONTAINING_RECORD (Secure,
                                 MMVAD_LONG,
                                 u2.LongFlags2);
    }
    else {
        Vad = (PMMVAD_LONG) MiLocateAddress ((PVOID)Secure->StartVpn);
    }

    ASSERT (Vad);
    ASSERT (Vad->u.VadFlags.NoChange == 1);
    ASSERT (Vad->u2.VadFlags2.LongVad == 1);

    if (Vad->u2.VadFlags2.OneSecured) {
        ASSERT (Secure == (PMMSECURE_ENTRY)&Vad->u2.LongFlags2);
        Vad->u2.VadFlags2.OneSecured = 0;
        ASSERT (Vad->u2.VadFlags2.MultipleSecured == 0);
        if (Vad->u2.VadFlags2.SecNoChange == 0) {

            //
            // No more secure entries in this list, remove the state.
            //

            Vad->u.VadFlags.NoChange = 0;
        }
        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
    }
    else {
        ASSERT (Vad->u2.VadFlags2.MultipleSecured == 1);

        if (Secure == (PMMSECURE_ENTRY)&Vad->u2.LongFlags2) {

            //
            // This was a single block that got converted into a list.
            // Reset the entry.
            //

            Secure = CONTAINING_RECORD (Vad->u3.List.Flink,
                                        MMSECURE_ENTRY,
                                        List);
        }
        RemoveEntryList (&Secure->List);
        if (IsListEmpty (&Vad->u3.List)) {

            //
            // No more secure entries, reset the state.
            //

            Vad->u2.VadFlags2.MultipleSecured = 0;

            if ((Vad->u2.VadFlags2.SecNoChange == 0) &&
               (Vad->u.VadFlags.PrivateMemory == 0)) {

                //
                // No more secure entries in this list, remove the state
                // if and only if this VAD is not private.  If this VAD
                // is private, removing the state NoChange flag indicates
                // that this is a short VAD which it no longer is.
                //

                Vad->u.VadFlags.NoChange = 0;
            }
        }
        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
        ExFreePool (Secure);
    }

    return;
}

#if DBG
VOID
MiDumpConflictingVad (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad
    )
{
    if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
            "MM: [%p ... %p) conflicted with Vad %p\n",
                  StartingAddress, EndingAddress, Vad);
        if ((Vad->u.VadFlags.PrivateMemory == 1) ||
            (Vad->ControlArea == NULL)) {
            return;
        }
        if (Vad->ControlArea->u.Flags.Image)
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                      "    conflict with %Z image at [%p .. %p)\n",
                      &Vad->ControlArea->FilePointer->FileName,
                      MI_VPN_TO_VA (Vad->StartingVpn),
                      MI_VPN_TO_VA_ENDING (Vad->EndingVpn)
                    );
        else
        if (Vad->ControlArea->u.Flags.File)
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                     "    conflict with %Z file at [%p .. %p)\n",
                      &Vad->ControlArea->FilePointer->FileName,
                      MI_VPN_TO_VA (Vad->StartingVpn),
                      MI_VPN_TO_VA_ENDING (Vad->EndingVpn)
                    );
        else
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                     "    conflict with section at [%p .. %p)\n",
                      MI_VPN_TO_VA (Vad->StartingVpn),
                      MI_VPN_TO_VA_ENDING (Vad->EndingVpn)
                    );
    }
}
#endif

