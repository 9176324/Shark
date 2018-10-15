/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   creasect.c

Abstract:

    This module contains the routines which implement the
    NtCreateSection and NtOpenSection.

--*/

#include "mi.h"

const ULONG MMCONTROL = 'aCmM';
const ULONG MMTEMPORARY = 'xxmM';

#define MM_SIZE_OF_LARGEST_IMAGE ((ULONG)0x77000000)

#define MM_MAXIMUM_IMAGE_HEADER (2 * PAGE_SIZE)

extern SIZE_T MmAllocationFragment;

//
// The maximum number of image object (object table entries) is
// the number which will fit into the MM_MAXIMUM_IMAGE_HEADER with
// the start of the PE image header in the last word of the first.
//

#define MM_MAXIMUM_IMAGE_SECTIONS                       \
     ((MM_MAXIMUM_IMAGE_HEADER - (PAGE_SIZE + sizeof(IMAGE_NT_HEADERS))) /  \
            sizeof(IMAGE_SECTION_HEADER))

#if DBG
ULONG MiMakeImageFloppy[2];
ULONG_PTR MiMatchSectionBase;
#endif

extern POBJECT_TYPE IoFileObjectType;

CCHAR MmImageProtectionArray[16] = {
                                    MM_NOACCESS,
                                    MM_EXECUTE,
                                    MM_READONLY,
                                    MM_EXECUTE_READ,
                                    MM_WRITECOPY,
                                    MM_EXECUTE_WRITECOPY,
                                    MM_WRITECOPY,
                                    MM_EXECUTE_WRITECOPY,
                                    MM_NOACCESS,
                                    MM_EXECUTE,
                                    MM_READONLY,
                                    MM_EXECUTE_READ,
                                    MM_READWRITE,
                                    MM_EXECUTE_READWRITE,
                                    MM_READWRITE,
                                    MM_EXECUTE_READWRITE };


VOID
MiUpdateImageHeaderPage (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER PageFrameNumber,
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL MarkModified
    );

CCHAR
MiGetImageProtection (
    IN ULONG SectionCharacteristics
    );

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    );

LOGICAL
MiCheckDosCalls (
    IN PIMAGE_OS2_HEADER Os2Header,
    IN ULONG HeaderSize
    );

VOID
MiInsertImageSectionObject (
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA ControlArea
    );

LOGICAL
MiFlushDataSection (
    IN PFILE_OBJECT File
    );

PVOID
MiCopyHeaderIfResident (
    IN PFILE_OBJECT File,
    IN PFN_NUMBER ImagePageFrameNumber
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiCreateImageFileMap)
#pragma alloc_text(PAGE,NtCreateSection)
#pragma alloc_text(PAGE,NtOpenSection)
#pragma alloc_text(PAGE,MiGetImageProtection)
#pragma alloc_text(PAGE,MiVerifyImageHeader)
#pragma alloc_text(PAGE,MiCheckDosCalls)
#pragma alloc_text(PAGE,MiCreatePagingFileMap)
#pragma alloc_text(PAGE,MiCreateDataFileMap)
#endif

#pragma pack (1)
typedef struct _PHARLAP_CONFIG {
    UCHAR  uchCopyRight[0x32];
    USHORT usType;
    USHORT usRsv1;
    USHORT usRsv2;
    USHORT usSign;
} CONFIGPHARLAP, *PCONFIGPHARLAP;
#pragma pack ()

NTSTATUS
NtCreateSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PLARGE_INTEGER MaximumSize,
    __in ULONG SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle
    )

/*++

Routine Description:

    This function creates a section object and opens a handle to the object
    with the specified desired access.

Arguments:

    SectionHandle - A pointer to a variable that will
        receive the section object handle value.

    DesiredAccess - The desired types of access for the section.

    DesiredAccess Flags

         EXECUTE - Execute access to the section is desired.

         READ - Read access to the section is desired.

         WRITE - Write access to the section is desired.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

    MaximumSize - Supplies the maximum size of the section in bytes.
                  This value is rounded up to the host page size and
                  specifies the size of the section (page file
                  backed section) or the maximum size to which a
                  file can be extended or mapped (file backed
                  section).

    SectionPageProtection - Supplies the protection to place on each page
                            in the section.  One of PAGE_READ, PAGE_READWRITE,
                            PAGE_EXECUTE, or PAGE_WRITECOPY and, optionally,
                            PAGE_NOCACHE or PAGE_WRITECOMBINE
                            may be specified.

    AllocationAttributes - Supplies a set of flags that describe the
                           allocation attributes of the section.

        AllocationAttributes Flags

        SEC_BASED - The section is a based section and will be
                    allocated at the same virtual address in each process
                    address space that receives the section.  This does not
                    imply that addresses are reserved for based sections.
                    Rather if the section cannot be mapped at the based address
                    an error is returned.


        SEC_RESERVE - All pages of the section are set to the reserved state.

        SEC_COMMIT - All pages of the section are set to the commit state.

        SEC_IMAGE - The file specified by the file handle is an
                    executable image file.

        SEC_FILE - The file specified by the file handle is a mapped
                   file.  If a file handle is supplied and neither
                   SEC_IMAGE or SEC_FILE is supplied, SEC_FILE is
                   assumed.

        SEC_NO_CHANGE - Once the file is mapped, the protection cannot
                        be changed nor can the view be unmapped.  The
                        view is unmapped when the process is deleted.
                        Cannot be used with SEC_IMAGE.

        SEC_LARGE_PAGES - The section will be created using large pages.
                          This physical memory is allocated immediately upon
                          section creation (instead of upon access).  If
                          contiguous physical memory for the large page(s)
                          cannot be found, then the section creation will fail
                          (rather than use small pages).

                          Since this physical memory is allocated up front and
                          is nonpaged (to ensure it remains physically
                          contiguous and in a large page), privilege is
                          required.  Since section mappers can extend the
                          section life indefinitely, section creators should
                          ensure proper access permissions are used.

                          Note subsequent mapped views must specify a large
                          page aligned start address and length (or must
                          have available virtual space so the system can find
                          one) for them to automatically receive large page(s)
                          for the view.  Callers of the map APIs do not need
                          to specify large pages - the system will automatically
                          provide the mapping in large pages (assuming
                          satisfactory alignment) or small pages if not.

    FileHandle - Supplies an optional handle of an open file object.
                 If the value of this handle is NULL, then the
                 section will be backed by a paging file.  Otherwise
                 the section is backed by the specified data file.

Return Value:

    NTSTATUS.

--*/

{
    NTSTATUS Status;
    PVOID Section;
    HANDLE Handle;
    LARGE_INTEGER LargeSize;
    LARGE_INTEGER CapturedSize;
    ULONG RetryCount;
    PCONTROL_AREA ControlArea;

    if ((AllocationAttributes & ~(SEC_COMMIT | SEC_RESERVE | SEC_BASED |
            SEC_LARGE_PAGES | SEC_IMAGE | SEC_NOCACHE |
            SEC_NO_CHANGE)) != 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & (SEC_COMMIT | SEC_RESERVE | SEC_IMAGE)) == 0) {
        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & SEC_IMAGE) &&
            (AllocationAttributes & (SEC_COMMIT | SEC_RESERVE |
                            SEC_LARGE_PAGES |
                            SEC_NOCACHE | SEC_NO_CHANGE))) {

        return STATUS_INVALID_PARAMETER_6;
    }

    if ((AllocationAttributes & SEC_COMMIT) &&
            (AllocationAttributes & SEC_RESERVE)) {
        return STATUS_INVALID_PARAMETER_6;
    }

    //
    // Check the SectionProtection Flag.
    //

    if ((SectionPageProtection & PAGE_NOCACHE) ||
        (SectionPageProtection & PAGE_WRITECOMBINE) ||
        (SectionPageProtection & PAGE_GUARD) ||
        (SectionPageProtection & PAGE_NOACCESS)) {

        //
        // No cache is only specified through the SEC_NOCACHE
        // option in the allocation attributes.
        //

        return STATUS_INVALID_PAGE_PROTECTION;
    }


    if (KeGetPreviousMode() != KernelMode) {
        try {
            ProbeForWriteHandle(SectionHandle);

            if (ARGUMENT_PRESENT (MaximumSize)) {

#if !defined (_WIN64)

                //
                // Note we only probe for byte alignment because prior to 2195,
                // we never probed at all!  We don't want to break user apps
                // that had bad alignment if they worked before.
                //

                ProbeForReadSmallStructure(MaximumSize, sizeof(LARGE_INTEGER), sizeof(UCHAR));
#else
                ProbeForReadSmallStructure(MaximumSize, sizeof(LARGE_INTEGER), PROBE_ALIGNMENT (LARGE_INTEGER));
#endif
                LargeSize = *MaximumSize;
            }
            else {
                ZERO_LARGE (LargeSize);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
    else {
        if (ARGUMENT_PRESENT (MaximumSize)) {
            LargeSize = *MaximumSize;
        }
        else {
            ZERO_LARGE (LargeSize);
        }
    }

    RetryCount = 0;

retry:

    CapturedSize = LargeSize;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    Status = MmCreateSection (&Section,
                              DesiredAccess,
                              ObjectAttributes,
                              &CapturedSize,
                              SectionPageProtection,
                              AllocationAttributes,
                              FileHandle,
                              NULL);


    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    if (!NT_SUCCESS(Status)) {
        if ((Status == STATUS_FILE_LOCK_CONFLICT) &&
            (RetryCount < 3)) {

            //
            // The file system may have prevented this from working
            // due to log file flushing.  Delay and try again.
            //

            RetryCount += 1;

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);

            goto retry;

        }
        return Status;
    }

    ControlArea = ((PSECTION)Section)->Segment->ControlArea;

    if ((ControlArea != NULL) && (ControlArea->FilePointer != NULL)) {
        CcZeroEndOfLastPage (ControlArea->FilePointer);
    }

    //
    // Note if the insertion fails, Ob will dereference the object for us.
    //

    Status = ObInsertObject (Section,
                             NULL,
                             DesiredAccess,
                             0,
                             (PVOID *)NULL,
                             &Handle);

    if (NT_SUCCESS(Status)) {
        try {
            *SectionHandle = Handle;
        } except (EXCEPTION_EXECUTE_HANDLER) {

            //
            // If the write attempt fails, then do not report an error.
            // When the caller attempts to access the handle value,
            // an access violation will occur.
            //
        }
    }

    return Status;
}

NTSTATUS
MmCreateSection (
    __deref_out PVOID *SectionObject,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in PLARGE_INTEGER InputMaximumSize,
    __in WIN32_PROTECTION_MASK SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle,
    __in_opt PFILE_OBJECT FileObject
    )

/*++

Routine Description:

    This function creates a section object and opens a handle to the object
    with the specified desired access.

Arguments:

    Section - A pointer to a variable that will
              receive the section object address.

    DesiredAccess - The desired types of access for the section.

    DesiredAccess Flags

         EXECUTE - Execute access to the section is desired.

         READ - Read access to the section is desired.

         WRITE - Write access to the section is desired.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

    InputMaximumSize - Supplies the maximum size of the section in bytes.
                       This value is rounded up to the host page size and
                       specifies the size of the section (page file
                       backed section) or the maximum size to which a
                       file can be extended or mapped (file backed
                       section).

    SectionPageProtection - Supplies the protection to place on each page
                            in the section.  One of PAGE_READ, PAGE_READWRITE,
                            PAGE_EXECUTE, or PAGE_WRITECOPY and, optionally,
                            PAGE_NOCACHE or PAGE_WRITECOMBINE may be specified.

                            Note the NO_CACHE, write combined and GUARD
                            attributes are mutually exclusive.
                            MiMakeProtectionMask enforces this.

    AllocationAttributes - Supplies a set of flags that describe the
                           allocation attributes of the section.

        AllocationAttributes Flags

        SEC_BASED - The section is a based section and will be
                    allocated at the same virtual address in each process
                    address space that receives the section.  This does not
                    imply that addresses are reserved for based sections.
                    Rather if the section cannot be mapped at the based address
                    an error is returned.

        SEC_RESERVE - All pages of the section are set to the
                      reserved state.

        SEC_COMMIT - All pages of the section are set to the commit state.

        SEC_IMAGE - The file specified by the file handle is an
                    executable image file.

        SEC_FILE - The file specified by the file handle is a mapped
                   file.  If a file handle is supplied and neither
                   SEC_IMAGE or SEC_FILE is supplied, SEC_FILE is
                   assumed.

        SEC_LARGE_PAGES - The section will be created using large pages.
                          This physical memory is allocated immediately upon
                          section creation (instead of upon access).  If
                          contiguous physical memory for the large page(s)
                          cannot be found, then the section creation will fail
                          (rather than use small pages).

                          Since this physical memory is allocated up front and
                          is nonpaged (to ensure it remains physically
                          contiguous and in a large page), privilege is
                          required.  Since section mappers can extend the
                          section life indefinitely, section creators should
                          ensure proper access permissions are used.

                          Note subsequent mapped views must specify a large
                          page aligned start address and length (or must
                          have available virtual space so the system can find
                          one) for them to automatically receive large page(s)
                          for the view.  Callers of the map APIs do not need
                          to specify large pages - the system will automatically
                          provide the mapping in large pages (assuming
                          satisfactory alignment) or small pages if not.

    FileHandle - Supplies an optional handle of an open file object.
                 If the value of this handle is null, then the
                 section will be backed by a paging file. Otherwise
                 the section is backed by the specified data file.

    FileObject - Supplies an optional pointer to the file object.  If this
                 value is NULL and the FileHandle is NULL, then there is
                 no file to map (image or mapped file).  If this value
                 is specified, then the File is to be mapped as a MAPPED FILE
                 and NO file size checking will be performed.

                 ONLY THE SYSTEM CACHE SHOULD PROVIDE A FILE OBJECT WITH THE
                 CALL!! as this is optimized to not check the size, only do
                 data mapping, no protection check, etc.

    Note - Only one of FileHandle or File should be specified!

Return Value:

    Returns the relevant NTSTATUS code.

--*/

{
    SECTION Section;
    PSECTION NewSection;
    PSUBSECTION Subsection;
    ULONG SubsectionSize;
    PSEGMENT Segment;
    PSEGMENT NewSegment;
    KPROCESSOR_MODE PreviousMode;
    KIRQL OldIrql;
    NTSTATUS Status;
    NTSTATUS Status2;
    PCONTROL_AREA ControlArea;
    PCONTROL_AREA NewControlArea;
    PCONTROL_AREA SegmentControlArea;
    ACCESS_MASK FileDesiredAccess;
    PFILE_OBJECT File;
    PEVENT_COUNTER Event;
    ULONG IgnoreFileSizing;
    ULONG ProtectionMask;
    ULONG ProtectMaskForAccess;
    ULONG FileAcquired;
    PEVENT_COUNTER SegmentEvent;
    LOGICAL FileSizeChecked;
    LARGE_INTEGER TempSectionSize;
    UINT64 EndOfFile;
    ULONG IncrementedRefCount;
    SIZE_T ControlAreaSize;
    PUINT64 MaximumSize;
    PMM_AVL_TABLE SectionBasedRoot;
    LOGICAL GlobalNeeded;
    PFILE_OBJECT ChangeFileReference;
    SIZE_T SizeOfSection;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
#if DBG
    PVOID PreviousSectionPointer;

    PreviousSectionPointer = (PVOID)-1;
#endif

    NewControlArea = (PCONTROL_AREA)-1;

    UNREFERENCED_PARAMETER (DesiredAccess);

    IgnoreFileSizing = FALSE;
    FileAcquired = FALSE;
    FileSizeChecked = FALSE;
    IncrementedRefCount = FALSE;

    MaximumSize = (PUINT64) InputMaximumSize;

    //
    // Check allocation attributes flags.
    //

    File = NULL;

    ASSERT ((AllocationAttributes & ~(SEC_COMMIT | SEC_RESERVE | SEC_BASED |
            SEC_LARGE_PAGES | SEC_IMAGE | SEC_NOCACHE | SEC_NO_CHANGE)) == 0);

    ASSERT ((AllocationAttributes & (SEC_COMMIT | SEC_RESERVE | SEC_IMAGE)) != 0);

    ASSERT (!((AllocationAttributes & SEC_IMAGE) &&
            (AllocationAttributes & (SEC_COMMIT | SEC_RESERVE |
                            SEC_NOCACHE | SEC_NO_CHANGE))));

    ASSERT (!((AllocationAttributes & SEC_COMMIT) &&
            (AllocationAttributes & SEC_RESERVE)));

    ASSERT (!((SectionPageProtection & PAGE_NOCACHE) ||
        (SectionPageProtection & PAGE_WRITECOMBINE) ||
        (SectionPageProtection & PAGE_GUARD) ||
        (SectionPageProtection & PAGE_NOACCESS)));

    if (AllocationAttributes & SEC_NOCACHE) {
        SectionPageProtection |= PAGE_NOCACHE;
    }

    //
    // Check the protection field.
    //

    ProtectionMask = MiMakeProtectionMask (SectionPageProtection);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ProtectMaskForAccess = ProtectionMask & 0x7;

    FileDesiredAccess = MmMakeFileAccess[ProtectMaskForAccess];

    //
    // Get previous processor mode and probe output arguments if necessary.
    //

    PreviousMode = KeGetPreviousMode ();

    Section.InitialPageProtection = SectionPageProtection;
    Section.Segment = NULL;

    //
    // Initializing Segment is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check
    // for use of uninitialized variables.
    //

    Segment = (PSEGMENT)-1;

    if (ARGUMENT_PRESENT(FileHandle) || ARGUMENT_PRESENT(FileObject)) {

        if (AllocationAttributes & SEC_LARGE_PAGES) {
            return STATUS_INVALID_PARAMETER_6;
        }

        //
        // Only one of FileHandle or FileObject should be supplied.
        // If a FileObject is supplied, this must be from the
        // file system and therefore the file's size should not
        // be checked.
        //

        if (ARGUMENT_PRESENT(FileObject)) {
            IgnoreFileSizing = TRUE;
            File = FileObject;

            //
            // Quick check to see if a control area already exists.
            //

            if (File->SectionObjectPointer->DataSectionObject) {

                ChangeFileReference = NULL;

                LOCK_PFN (OldIrql);
                ControlArea =
                    (PCONTROL_AREA)(File->SectionObjectPointer->DataSectionObject);

                if ((ControlArea != NULL) &&
                    (!ControlArea->u.Flags.BeingDeleted) &&
                    (!ControlArea->u.Flags.BeingCreated)) {

                    //
                    // Control area exists and is not being deleted,
                    // reference it.
                    //

                    NewSegment = ControlArea->Segment;
                    if ((ControlArea->NumberOfSectionReferences == 0) &&
                        (ControlArea->NumberOfMappedViews == 0) &&
                        (ControlArea->ModifiedWriteCount == 0)) {

                        //
                        // Dereference the current file object (after releasing
                        // the PFN lock) and reference this one.
                        //

                        ASSERT (ControlArea->FilePointer != NULL);
                        ChangeFileReference = ControlArea->FilePointer;
                        ControlArea->FilePointer = FileObject;
                    }
                    ControlArea->u.Flags.Accessed = 1;
                    ControlArea->NumberOfSectionReferences += 1;
                    if (ControlArea->DereferenceList.Flink != NULL) {

                        //
                        // Remove this from the list of unused segments.
                        //

                        RemoveEntryList (&ControlArea->DereferenceList);

                        MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

                        ControlArea->DereferenceList.Flink = NULL;
                        ControlArea->DereferenceList.Blink = NULL;
                    }
                    UNLOCK_PFN (OldIrql);

                    //
                    // Inform the object manager to defer this deletion by
                    // queueing it to another thread to eliminate deadlocks
                    // with the redirector.
                    //

                    if (ChangeFileReference != NULL) {
                        ObDereferenceObjectDeferDelete (ChangeFileReference);
                        ObReferenceObject (FileObject);
                    }

                    IncrementedRefCount = TRUE;
                    Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;

                    goto ReferenceObject;
                }
                UNLOCK_PFN (OldIrql);
            }

            ObReferenceObject (FileObject);

        }
        else {

            Status = ObReferenceObjectByHandle (FileHandle,
                                                FileDesiredAccess,
                                                IoFileObjectType,
                                                PreviousMode,
                                                (PVOID *)&File,
                                                NULL);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }

            //
            // If this file doesn't have a section object pointer,
            // return an error.
            //

            if (File->SectionObjectPointer == NULL) {
                ObDereferenceObject (File);
                return STATUS_INVALID_FILE_FOR_SECTION;
            }
        }

        //
        // Check to see if the specified file already has a section.
        // If not, indicate in the file object's pointer to an FCB that
        // a section is being built.  This synchronizes segment creation
        // for the file.
        //

        if (AllocationAttributes & SEC_IMAGE) {

            //
            // This control area is always just a place holder - the real one
            // is allocated in MiCreateImageFileMap and will be allocated
            // with the correct size and this one freed in a short while.
            //
            // This place holder must always be allocated as a large control
            // area so that it can be chained for the per-session case.
            //

            ControlAreaSize = sizeof(LARGE_CONTROL_AREA) + sizeof(SUBSECTION);

            //
            // For image sections, make sure that Cc has released all references
            // to the section due to a previous data section mapping.  This will
            // decrease the chance that the image will need to be backed by 
            // the pagefile because writeable data sections remain on the file.
            //
            
            CcWaitForUninitializeCacheMap (File);
        }
        else {

            //
            // Data files are mapped with larger subsections than images or
            // pagefile-backed shared memory.  Factor that in here.
            //

            ControlAreaSize = sizeof(CONTROL_AREA) + sizeof(MSUBSECTION);
        }

        NewControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                                ControlAreaSize,
                                                MMCONTROL);

        if (NewControlArea == NULL) {
            ObDereferenceObject (File);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (NewControlArea, ControlAreaSize);

        NewSegment = NULL;

        //
        // We only need the file resource if this was a user request, i.e. not
        // a call from the cache manager or file system.
        //

        if (ARGUMENT_PRESENT (FileHandle)) {

            PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;

            Status = FsRtlAcquireToCreateMappedSection (File, SectionPageProtection);

            if (!NT_SUCCESS(Status)) {
                ExFreePool (NewControlArea);
                ObDereferenceObject (File);
                return Status;
            }

            IoSetTopLevelIrp (tempIrp);
            FileAcquired = TRUE;
        }

        //
        // Initializing GlobalNeeded is not needed for correctness, but
        // without it the compiler cannot compile this code W4 to check
        // for use of uninitialized variables.
        //

        GlobalNeeded = FALSE;
        SegmentEvent = NULL;

RecheckSegment:

        LOCK_PFN (OldIrql);

        if (AllocationAttributes & SEC_IMAGE) {

            ControlArea = MiFindImageSectionObject (File, TRUE, &GlobalNeeded);

        }
        else {
            ControlArea =
               (PCONTROL_AREA)(File->SectionObjectPointer->DataSectionObject);
        }

        if (ControlArea != NULL) {

            //
            // A segment already exists for this file.  Make sure that it
            // is not in the process of being deleted, or being created.
            //

            if ((ControlArea->u.Flags.BeingDeleted) ||
                (ControlArea->u.Flags.BeingCreated)) {

                //
                // The segment object is in the process of being deleted or
                // created.
                // Check to see if another thread is waiting for the deletion,
                // otherwise create an event object to wait upon.
                //

                if (ControlArea->WaitingForDeletion == NULL) {

                    //
                    // Initialize an event and put its address
                    // in the control area.
                    //

                    if (SegmentEvent == NULL) {

                        //
                        // Allocate an event to wait as the segment
                        // is in the process of being deleted.  This event
                        // cannot be allocated with the PFN database locked
                        // as pool expansion would deadlock.
                        //

                        UNLOCK_PFN (OldIrql);

                        SegmentEvent = MiGetEventCounter ();

                        if (SegmentEvent == NULL) {
                            if (FileAcquired) {
                                IoSetTopLevelIrp (NULL);
                                FsRtlReleaseFile (File);
                            }
                            ExFreePool (NewControlArea);
                            ObDereferenceObject (File);
                            return STATUS_INSUFFICIENT_RESOURCES;
                        }

                        goto RecheckSegment;
                    }

                    ControlArea->WaitingForDeletion = SegmentEvent;
                    Event = SegmentEvent;
                    SegmentEvent = NULL;
                }
                else {
                    Event = ControlArea->WaitingForDeletion;

                    //
                    // No interlock is needed for the RefCount increment as
                    // no thread can be decrementing it since it is still
                    // pointed to by the control area.
                    //

                    Event->RefCount += 1;
                }

                //
                // Release the PFN lock, the file lock, and wait for the event.
                //

                UNLOCK_PFN (OldIrql);
                if (FileAcquired) {
                    IoSetTopLevelIrp (NULL);
                    FsRtlReleaseFile (File);
                }

                KeWaitForSingleObject (&Event->Event,
                                       WrVirtualMemory,
                                       KernelMode,
                                       FALSE,
                                       (PLARGE_INTEGER)NULL);

                //
                // Before this event can be set, the control area
                // WaitingForDeletion field must be cleared (and may be
                // reinitialized to something else), but cannot be reset
                // to our local event.  This allows us to dereference the
                // event count lock free.
                //

                MiFreeEventCounter (Event);

                if (FileAcquired) {
                    Status = FsRtlAcquireToCreateMappedSection (File, SectionPageProtection);

                    if (NT_SUCCESS(Status)) {
                        PIRP tempIrp = (PIRP)FSRTL_FSP_TOP_LEVEL_IRP;
                        IoSetTopLevelIrp (tempIrp);
                    }
                    else {
                        ExFreePool (NewControlArea);
                        ObDereferenceObject (File);
                        return Status;
                    }
                }

                goto RecheckSegment;
            }

            //
            // There is already a segment for this file, have
            // this section refer to that segment.
            // No need to reference the file object any more.
            //

            if ((ControlArea->u.Flags.ImageMappedInSystemSpace) &&
                (AllocationAttributes & SEC_IMAGE) &&
                (KeGetPreviousMode () != KernelMode)) {

                UNLOCK_PFN (OldIrql);

                if (SegmentEvent != NULL) {
                    MiFreeEventCounter (SegmentEvent);
                }
                if (FileAcquired) {
                    IoSetTopLevelIrp (NULL);
                    FsRtlReleaseFile (File);
                }
                ExFreePool (NewControlArea);
                ObDereferenceObject (File);
                return STATUS_CONFLICTING_ADDRESSES;
            }

            NewSegment = ControlArea->Segment;
            ControlArea->u.Flags.Accessed = 1;
            ControlArea->NumberOfSectionReferences += 1;
            if (ControlArea->DereferenceList.Flink != NULL) {

                //
                // Remove this from the list of unused segments.
                //

                RemoveEntryList (&ControlArea->DereferenceList);

                MI_UNUSED_SEGMENTS_REMOVE_CHARGE (ControlArea);

                ControlArea->DereferenceList.Flink = NULL;
                ControlArea->DereferenceList.Blink = NULL;
            }
            IncrementedRefCount = TRUE;

            //
            // If this reference was not from the cache manager
            // up the count of user references.
            //

            if (IgnoreFileSizing == FALSE) {
                ControlArea->NumberOfUserReferences += 1;
            }
        }
        else {

            //
            // There is no segment associated with this file object.
            // Set the file object to refer to the new control area.
            //

            ControlArea = NewControlArea;
            ControlArea->u.Flags.BeingCreated = 1;

            if (AllocationAttributes & SEC_IMAGE) {
                if (GlobalNeeded == TRUE) {
                    ControlArea->u.Flags.GlobalOnlyPerSession = 1;
                }

                MiInsertImageSectionObject (File, ControlArea);
            }
            else {
#if DBG
                PreviousSectionPointer = File->SectionObjectPointer;
#endif
                File->SectionObjectPointer->DataSectionObject = (PVOID) ControlArea;
            }
        }

        UNLOCK_PFN (OldIrql);

        if (SegmentEvent != NULL) {
            MiFreeEventCounter (SegmentEvent);
        }

        if (NewSegment != NULL) {

            //
            // A segment already exists for this file object.
            // If we're creating an imagemap, flush the data section
            // if there is one.
            //

            if (AllocationAttributes & SEC_IMAGE) {
                MiFlushDataSection (File);
            }

            //
            // Deallocate the new control area as it won't be needed.
            // Dereference the file object later when we're done with it.
            //

            ExFreePool (NewControlArea);

            //
            // The section is in paged pool, this can't be set until
            // the PFN mutex has been released.
            //

            if ((!IgnoreFileSizing) && (ControlArea->u.Flags.Image == 0)) {

                //
                // The file size in the segment may not match the current
                // file size, query the file system and get the file
                // size.
                //

                Status = FsRtlGetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

                if (!NT_SUCCESS (Status)) {

                    if (FileAcquired) {
                        IoSetTopLevelIrp(NULL);
                        FsRtlReleaseFile (File);
                        FileAcquired = FALSE;
                    }

                    ObDereferenceObject (File);
                    goto UnrefAndReturn;
                }

                if (EndOfFile == 0 && *MaximumSize == 0) {

                    //
                    // Can't map a zero length without specifying the maximum
                    // size as non-zero.
                    //

                    Status = STATUS_MAPPED_FILE_SIZE_ZERO;

                    if (FileAcquired) {
                        IoSetTopLevelIrp(NULL);
                        FsRtlReleaseFile (File);
                        FileAcquired = FALSE;
                    }

                    ObDereferenceObject (File);
                    goto UnrefAndReturn;
                }
            }
            else {

                //
                // The size is okay in the segment.
                //

                EndOfFile = (UINT64) NewSegment->SizeOfSegment;
            }

            if (FileAcquired) {
                IoSetTopLevelIrp(NULL);
                FsRtlReleaseFile (File);
                FileAcquired = FALSE;
            }

            ObDereferenceObject (File);

            if (*MaximumSize == 0) {

                Section.SizeOfSection.QuadPart = (LONGLONG)EndOfFile;
                FileSizeChecked = TRUE;
            }
            else if (EndOfFile >= *MaximumSize) {

                //
                // EndOfFile is greater than the MaximumSize,
                // use the specified maximum size.
                //

                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
                FileSizeChecked = TRUE;
            }
            else {

                //
                // Need to extend the section, make sure the file was
                // opened for write access.
                //

                if (((SectionPageProtection & PAGE_READWRITE) |
                    (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

                    Status = STATUS_SECTION_TOO_BIG;
                    goto UnrefAndReturn;
                }
                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
            }
        }
        else {

            //
            // The file does not have an associated segment, create a segment
            // object.
            //

            if (AllocationAttributes & SEC_IMAGE) {

                Status = MiCreateImageFileMap (File, &Segment);

            }
            else {

                Status = MiCreateDataFileMap (File,
                                              &Segment,
                                              MaximumSize,
                                              SectionPageProtection,
                                              AllocationAttributes,
                                              IgnoreFileSizing);
                ASSERT (PreviousSectionPointer == File->SectionObjectPointer);
            }

            if (!NT_SUCCESS(Status)) {

                //
                // Lock the PFN database and check to see if another thread has
                // tried to create a segment to the file object at the same
                // time.
                //

                LOCK_PFN (OldIrql);

                Event = ControlArea->WaitingForDeletion;
                ControlArea->WaitingForDeletion = NULL;
                ASSERT (ControlArea->u.Flags.FilePointerNull == 0);
                ControlArea->u.Flags.FilePointerNull = 1;

                if (AllocationAttributes & SEC_IMAGE) {
                    MiRemoveImageSectionObject (File, ControlArea);
                }
                else {
                    File->SectionObjectPointer->DataSectionObject = NULL;
                }
                ControlArea->u.Flags.BeingCreated = 0;

                UNLOCK_PFN (OldIrql);

                if (FileAcquired) {
                    IoSetTopLevelIrp(NULL);
                    FsRtlReleaseFile (File);
                }

                ExFreePool (NewControlArea);

                ObDereferenceObject (File);

                if (Event != NULL) {

                    //
                    // Signal any waiters that the segment structure exists.
                    //

                    KeSetEvent (&Event->Event, 0, FALSE);
                }

                return Status;
            }

            //
            // If the size was specified as zero, set the section size
            // from the created segment size.  This solves problems with
            // race conditions when multiple sections
            // are created for the same mapped file with varying sizes.
            //

            if (*MaximumSize == 0) {
                Section.SizeOfSection.QuadPart = (LONGLONG)Segment->SizeOfSegment;
            }
            else {
                Section.SizeOfSection.QuadPart = (LONGLONG)*MaximumSize;
            }
        }
    }
    else {

        //
        // No file handle exists, this is a page file backed section.
        //

        if (AllocationAttributes & SEC_IMAGE) {
            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        if (AllocationAttributes & SEC_LARGE_PAGES) {

            if ((AllocationAttributes & SEC_COMMIT) == 0) {
                return STATUS_INVALID_PARAMETER_6;
            }

            if (!SeSinglePrivilegeCheck (SeLockMemoryPrivilege,
                                         KeGetPreviousMode())) {

                return STATUS_PRIVILEGE_NOT_HELD;
            }
        }

        Status = MiCreatePagingFileMap (&NewSegment,
                                        MaximumSize,
                                        ProtectionMask,
                                        AllocationAttributes);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        //
        // Set the section size from the created segment size.  This
        // solves problems with race conditions when multiple sections
        // are created for the same mapped file with varying sizes.
        //

        Section.SizeOfSection.QuadPart = (LONGLONG)NewSegment->SizeOfSegment;
        ControlArea = NewSegment->ControlArea;

        //
        // Set IncrementedRefCount so any failures from this point before the
        // object is created will result in the control area & segment getting
        // torn down - otherwise these could leak.  This is because pagefile
        // backed sections are not (and should not be) added to the
        // dereference segment cache.
        //

        IncrementedRefCount = 1;
    }

    if (NewSegment == NULL) {

        //
        // A new segment had to be created.  Lock the PFN database and
        // check to see if any other threads also tried to create a new segment
        // for this file object at the same time.
        //

        NewSegment = Segment;

        SegmentControlArea = Segment->ControlArea;

        ASSERT (File != NULL);

        LOCK_PFN (OldIrql);

        Event = ControlArea->WaitingForDeletion;
        ControlArea->WaitingForDeletion = NULL;

        if (AllocationAttributes & SEC_IMAGE) {

            //
            // Change the control area in the file object pointer.
            //

            MiRemoveImageSectionObject (File, NewControlArea);
            MiInsertImageSectionObject (File, SegmentControlArea);

            ControlArea = SegmentControlArea;
        }
        else if (SegmentControlArea->u.Flags.Rom == 1) {
            ASSERT (File->SectionObjectPointer->DataSectionObject == NewControlArea);
            File->SectionObjectPointer->DataSectionObject = SegmentControlArea;

            ControlArea = SegmentControlArea;
        }

        ControlArea->u.Flags.BeingCreated = 0;

        UNLOCK_PFN (OldIrql);

        if ((AllocationAttributes & SEC_IMAGE) ||
            (SegmentControlArea->u.Flags.Rom == 1)) {

            //
            // Deallocate the pool used for the original control area.
            //

            ExFreePool (NewControlArea);
        }

        if (Event != NULL) {

            //
            // Signal any waiters that the segment structure exists.
            //

            KeSetEvent (&Event->Event, 0, FALSE);
        }
    }

    //
    // Being created has now been cleared allowing other threads
    // to reference the segment.  Release the resource on the file.
    //

    if (FileAcquired) {
        IoSetTopLevelIrp(NULL);
        FsRtlReleaseFile (File);
        FileAcquired = FALSE;
    }

ReferenceObject:

    //
    // Now that the segment object is created, make the section object
    // refer to the segment object.
    //

    Section.Segment = NewSegment;
    Section.u.LongFlags = ControlArea->u.LongFlags;

    //
    // Update the count of writable user sections so the transaction semantics
    // can be supported.  Note that no lock synchronization is needed here as
    // the transaction manager must already check for any open writable handles
    // to the file - and no writable sections can be created without a writable
    // file handle.  So all that needs to be provided is a way for the
    // transaction manager to know that there are lingering user views or
    // created sections still open that have write access.
    //
    // This must be done before creating the object so a rogue user program
    // that suspends this thread cannot subvert a transaction.
    //

    if ((FileObject == NULL) &&
        (SectionPageProtection & (PAGE_READWRITE|PAGE_EXECUTE_READWRITE)) &&
        (ControlArea->u.Flags.Image == 0) &&
        (ControlArea->FilePointer != NULL)) {

        Section.u.Flags.UserWritable = 1;

        InterlockedIncrement ((PLONG)&ControlArea->WritableUserReferences);
    }

    //
    // Create the section object now.  The section object is created
    // now so that the error handling when the section object cannot
    // be created is simplified.
    //

    if ((ControlArea->u.Flags.Image == 1) || (ControlArea->FilePointer == NULL)) {
        PagedPoolCharge = sizeof (SECTION) + NewSegment->TotalNumberOfPtes * sizeof(MMPTE);
        SubsectionSize = sizeof (SUBSECTION);
    }
    else {
        PagedPoolCharge = 0;
        SubsectionSize = sizeof (MSUBSECTION);
    }

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {

        NonPagedPoolCharge = sizeof (CONTROL_AREA);
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        NonPagedPoolCharge = sizeof(LARGE_CONTROL_AREA);
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    do {
        NonPagedPoolCharge += SubsectionSize;
        Subsection = Subsection->NextSubsection;
    } while (Subsection != NULL);

    Status = ObCreateObject (PreviousMode,
                             MmSectionObjectType,
                             ObjectAttributes,
                             PreviousMode,
                             NULL,
                             sizeof(SECTION),
                             PagedPoolCharge,
                             NonPagedPoolCharge,
                             (PVOID *)&NewSection);

    if (!NT_SUCCESS(Status)) {

        if ((FileObject == NULL) &&
            (SectionPageProtection & (PAGE_READWRITE|PAGE_EXECUTE_READWRITE)) &&
            (ControlArea->u.Flags.Image == 0) &&
            (ControlArea->FilePointer != NULL)) {

            ASSERT (Section.u.Flags.UserWritable == 1);

            InterlockedDecrement ((PLONG)&ControlArea->WritableUserReferences);
        }

        goto UnrefAndReturn;
    }

    RtlCopyMemory (NewSection, &Section, sizeof(SECTION));
    NewSection->Address.StartingVpn = 0;

    if (!IgnoreFileSizing) {

        //
        // Indicate that the cache manager is not the owner of this
        // section.
        //

        NewSection->u.Flags.UserReference = 1;

        if (AllocationAttributes & SEC_NO_CHANGE) {

            //
            // Indicate that once the section is mapped, no protection
            // changes or freeing the mapping is allowed.
            //

            NewSection->u.Flags.NoChange = 1;
        }

        if (((SectionPageProtection & PAGE_READWRITE) |
            (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

            //
            // This section does not support WRITE access, indicate
            // that changing the protection to WRITE results in COPY_ON_WRITE.
            //

            NewSection->u.Flags.CopyOnWrite = 1;
        }

        if (AllocationAttributes & SEC_BASED) {

            NewSection->u.Flags.Based = 1;

            SectionBasedRoot = &MmSectionBasedRoot;

            //
            // This section is based at a unique address system wide.
            // Ensure it does not wrap the virtual address space as the
            // SECTION structure would have to widen to accommodate this and
            // it's not worth the performance penalty for the very few isolated
            // cases that would want this.  Note that sections larger than the
            // address space can easily be created - it's just that beyond a
            // certain point you shouldn't specify SEC_BASED (anything this big
            // couldn't use a SEC_BASED section for anything anyway).
            //

            if ((UINT64)NewSection->SizeOfSection.QuadPart > (UINT64)MmHighSectionBase) {
                ObDereferenceObject (NewSection);
                return STATUS_NO_MEMORY;
            }

#if defined(_WIN64)
            SizeOfSection = NewSection->SizeOfSection.QuadPart;
#else
            SizeOfSection = NewSection->SizeOfSection.LowPart;
#endif

            //
            // Get the allocation base mutex.
            //

            KeAcquireGuardedMutex (&MmSectionBasedMutex);

            Status2 = MiFindEmptyAddressRangeDownBasedTree (
                                            SizeOfSection,
                                            MmHighSectionBase,
                                            X64K,
                                            SectionBasedRoot,
                                            (PVOID *)&NewSection->Address.StartingVpn);
            if (!NT_SUCCESS(Status2)) {
                KeReleaseGuardedMutex (&MmSectionBasedMutex);
                ObDereferenceObject (NewSection);
                return Status2;
            }

            NewSection->Address.EndingVpn = NewSection->Address.StartingVpn +
                                                SizeOfSection - 1;

            MiInsertBasedSection (NewSection);
            KeReleaseGuardedMutex (&MmSectionBasedMutex);
        }
    }

    //
    // If the cache manager is creating the section, set the was
    // purged flag as the file size can change.
    //

    ControlArea->u.Flags.WasPurged |= IgnoreFileSizing;

    //
    // Check to see if the section is for a data file and the size
    // of the section is greater than the current size of the
    // segment.
    //

    if (((ControlArea->u.Flags.WasPurged == 1) && (!IgnoreFileSizing)) &&
                      (!FileSizeChecked)
                            ||
        ((UINT64)NewSection->SizeOfSection.QuadPart >
                                NewSection->Segment->SizeOfSegment)) {

        TempSectionSize = NewSection->SizeOfSection;

        NewSection->SizeOfSection.QuadPart = (LONGLONG)NewSection->Segment->SizeOfSegment;

        //
        // Even if the caller didn't specify extension rights, we enable it here
        // temporarily to make the section correct.  Use a temporary section
        // instead of temporarily editing the real section to avoid opening
        // a security window that other concurrent threads could exploit.
        //

        if (((NewSection->InitialPageProtection & PAGE_READWRITE) |
            (NewSection->InitialPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {
                SECTION     WritableSection;

                *(PSECTION)&WritableSection = *NewSection;

                Status = MmExtendSection (&WritableSection,
                                          &TempSectionSize,
                                          IgnoreFileSizing);

                NewSection->SizeOfSection = WritableSection.SizeOfSection;
        }
        else {
            Status = MmExtendSection (NewSection,
                                      &TempSectionSize,
                                      IgnoreFileSizing);
        }

        if (!NT_SUCCESS(Status)) {
            ObDereferenceObject (NewSection);
            return Status;
        }
    }

    *SectionObject = (PVOID)NewSection;

    return Status;

UnrefAndReturn:

    //
    // Unreference the control area, if it was referenced and return
    // the error status.
    //

    if (FileAcquired) {
        IoSetTopLevelIrp(NULL);
        FsRtlReleaseFile (File);
    }

    if (IncrementedRefCount) {
        LOCK_PFN (OldIrql);
        ControlArea->NumberOfSectionReferences -= 1;
        if (!IgnoreFileSizing) {
            ASSERT ((LONG)ControlArea->NumberOfUserReferences > 0);
            ControlArea->NumberOfUserReferences -= 1;
        }
        MiCheckControlArea (ControlArea, OldIrql);
    }
    return Status;
}

LOGICAL
MiMakeControlAreaRom (
    IN PFILE_OBJECT File,
    IN PLARGE_CONTROL_AREA ControlArea,
    IN PFN_NUMBER PageFrameNumber
    )

/*++

Routine Description:

    This function marks the control area as ROM-backed.  It can fail if the
    parallel control area (image vs data) is currently active as ROM-backed
    as the same PFNs cannot be used for both control areas simultaneously.

Arguments:

    ControlArea - Supplies the relevant control area.

    PageFrameNumber - Supplies the starting physical page frame number.

Return Value:

    TRUE if the control area was marked as ROM-backed, FALSE if not.

--*/

{
    LOGICAL ControlAreaMarked;
    PCONTROL_AREA OtherControlArea;
    KIRQL OldIrql;

    ControlAreaMarked = FALSE;

    LOCK_PFN (OldIrql);

    if (ControlArea->u.Flags.Image == 1) {
        OtherControlArea = (PCONTROL_AREA) File->SectionObjectPointer->DataSectionObject;
    }
    else {
        OtherControlArea = (PCONTROL_AREA) File->SectionObjectPointer->ImageSectionObject;
    }

    //
    // This could be made smarter (ie: throw away the other control area if it's
    // not in use) but for now, keep it simple.
    //

    if ((OtherControlArea == NULL) || (OtherControlArea->u.Flags.Rom == 0)) {
        ControlArea->u.Flags.Rom = 1;
        ControlArea->StartingFrame = PageFrameNumber;
        ControlAreaMarked = TRUE;
    }

    UNLOCK_PFN (OldIrql);

    return ControlAreaMarked;
}

ULONG MiImageFailure;

#define MI_BAD_IMAGE(x)     MiImageFailure = x;


NTSTATUS
MiCreateImageFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment
    )

/*++

Routine Description:

    This function creates the necessary structures to allow the mapping
    of an image file.

    The image file is opened and verified for correctness, a segment
    object is created and initialized based on data in the image
    header.

Arguments:

    File - Supplies the file object for the image file.

    Segment - Returns the segment object.

Return Value:

    NTSTATUS.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    LOGICAL CheckSplitPages;
    LOGICAL MarkModified;
    LOGICAL SingleSubsection;
    NTSTATUS Status;
    ULONG EndingAddress;
    PFN_NUMBER NumberOfPtes;
    SIZE_T SizeOfSegment;
    ULONG SectionVirtualSize;
    ULONG SizeOfRawData;
    ULONG i;
    ULONG j;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;
    PMMPTE PointerPte;
    PMMPTE BasePte;
    MMPTE TempPte;
    MMPTE TempPteDemandZero;
    PVOID Base;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    ULONG SizeOfImage;
    ULONG SizeOfHeaders;
    ULONG MarkHeaderModified;
#if defined (_WIN64)
    PIMAGE_NT_HEADERS32 NtHeader32;
#endif
    PIMAGE_DATA_DIRECTORY ComPlusDirectoryEntry;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    PSEGMENT NewSegment;
    ULONG SectorOffset;
    ULONG NumberOfSubsections;
    PFN_NUMBER PageFrameNumber;
    LOGICAL GlobalPerSession;
    LARGE_INTEGER StartingOffset;
    PCHAR ExtendedHeader;
    PPFN_NUMBER Page;
    ULONG_PTR PreferredImageBase;
    ULONG_PTR NextVa;
    ULONG_PTR ImageBase;
    KEVENT InPageEvent;
    PMDL Mdl;
    ULONG ImageFileSize;
    ULONG OffsetToSectionTable;
    ULONG ImageAlignment;
    ULONG RoundingAlignment;
    ULONG FileAlignment;
    LOGICAL ImageCommit;
    LOGICAL SectionCommit;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER EndOfFile;
    ULONG NtHeaderSize;
    ULONG SubsectionsAllocated;
    PLARGE_CONTROL_AREA LargeControlArea;
    PSUBSECTION NewSubsection;
    ULONG OriginalProtection;
    ULONG LoaderFlags;
    ULONG FileHeaderNumberOfSections;
    ULONG TempNumberOfSubsections;
    PIMAGE_SECTION_HEADER TempSectionTableEntry;
    ULONG AdditionalSubsections;
    ULONG AdditionalPtes;
    ULONG AdditionalBasePtes;
    ULONG NewSubsectionsAllocated;
    PSEGMENT OldSegment;
    PMMPTE NewPointerPte;
    PMMPTE OldPointerPte;
    PFN_NUMBER OrigNumberOfPtes;
    PCONTROL_AREA NewControlArea;
    SIZE_T CommitCharged;
    LOGICAL ActiveDataReferences;
    PFN_NUMBER StackMdl[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_IMAGE_HEADER / PAGE_SIZE];
    PMMPFN ImagePages;

#define InvalidAlignmentAllowed FALSE

    PAGED_CODE ();

    MarkModified = TRUE;
    MarkHeaderModified = 0;

    Status = FsRtlGetFileSize (File, &EndOfFile);

    if (!NT_SUCCESS (Status)) {

        MI_BAD_IMAGE (1);
        if (Status == STATUS_FILE_IS_A_DIRECTORY) {

            //
            // Can't map a directory as a section - return an error.
            //

            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        return Status;
    }

    if (EndOfFile.HighPart != 0) {

        //
        // File too big. Return error.
        //

        return STATUS_INVALID_FILE_FOR_SECTION;
    }

    CommitCharged = 0;
    ExtendedHeader = NULL;
    ControlArea = NULL;
    NewSegment = NULL;

    Mdl = (PMDL) &StackMdl;

    //
    // Read in the file header.
    //
    // Create an event and MDL for the read operation.
    //

    KeInitializeEvent (&InPageEvent, NotificationEvent, FALSE);

    MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    PageFrameNumber = MiGetPageForHeader (TRUE);

    ASSERT (PageFrameNumber != 0);

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    ASSERT (Pfn1->u1.Flink == 0);
    ImagePages = Pfn1;

    Page = (PPFN_NUMBER)(Mdl + 1);
    *Page = PageFrameNumber;

    ZERO_LARGE (StartingOffset);

    CcZeroEndOfLastPage (File);

    //
    // Flush the data section if there is one.
    //
    // At the same time, capture whether there are any references to the
    // data control area.  If so, flip the pages from the image control
    // area into pagefile backings to prevent anyone else's data writes
    // from changing the file after it is validated (this can happen if the
    // pages from the image control area need to be re-paged in later).
    //

    ActiveDataReferences = MiFlushDataSection (File);

    BasePte = NULL;
    Base = MiCopyHeaderIfResident (File, PageFrameNumber);

    if (Base == NULL) {

        ASSERT (Mdl->MdlFlags & MDL_PAGES_LOCKED);

        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   NULL);

            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (2);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

        BasePte = MiReserveSystemPtes (1, SystemPteSpace);

        if (BasePte == NULL) {
            MI_BAD_IMAGE (0x25);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

        ASSERT (BasePte->u.Hard.Valid == 0);

        Base = MiGetVirtualAddressMappedByPte (BasePte);

        TempPte = ValidKernelPte;
        TempPte.u.Hard.PageFrameNumber = PageFrameNumber;

        MI_WRITE_VALID_PTE (BasePte, TempPte);

        if (IoStatus.Information != PAGE_SIZE) {

            //
            // A full page was not read from the file, zero any remaining
            // bytes because this page will be cached if the section
            // creation succeeds.
            //

            if (IoStatus.Information < sizeof (IMAGE_DOS_HEADER)) {
                Status = STATUS_INVALID_IMAGE_NOT_MZ;
                MI_BAD_IMAGE (0x26);
                goto BadPeImageSegment;
            }

            RtlZeroMemory ((PVOID)((PCHAR)Base + IoStatus.Information),
                           PAGE_SIZE - IoStatus.Information);

            //
            // Mark the PFN as modified so we don't lose the zeroing if
            // the page gets trimmed.
            //

            MarkHeaderModified = 1;
        }
    }
    else {
        IoStatus.Information = PAGE_SIZE;

        if (MI_HYPER_ADDRESS (Base)) {
            BasePte = MiGetPteAddress (Base);
        }
    }

    DosHeader = (PIMAGE_DOS_HEADER) Base;

    //
    // Check to determine if this is an NT image (PE format) or
    // a DOS image, Win-16 image, or OS/2 image.  If the image is
    // not NT format, return an error indicating which image it
    // appears to be.
    //

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

        Status = STATUS_INVALID_IMAGE_NOT_MZ;
        goto BadPeImageSegment;
    }

#ifndef i386
    if (((ULONG)DosHeader->e_lfanew & 3) != 0) {

        //
        // The image header is not aligned on a longword boundary.
        // Report this as an invalid protected mode image.
        //

        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (4);
        goto BadPeImageSegment;
    }
#endif

    if ((ULONG)DosHeader->e_lfanew > EndOfFile.LowPart) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (5);
        goto BadPeImageSegment;
    }

    if (((ULONG)DosHeader->e_lfanew +
            sizeof(IMAGE_NT_HEADERS) +
            (16 * sizeof(IMAGE_SECTION_HEADER))) <= (ULONG)DosHeader->e_lfanew) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (6);
        goto BadPeImageSegment;
    }

    if (((ULONG)DosHeader->e_lfanew +
            sizeof(IMAGE_NT_HEADERS) +
            (16 * sizeof(IMAGE_SECTION_HEADER))) > PAGE_SIZE) {

        //
        // The PE header is not within the page already read or the
        // objects are in another page.
        // Build another MDL and read an additional 8k.
        //

        ExtendedHeader = ExAllocatePoolWithTag (NonPagedPool,
                                                MM_MAXIMUM_IMAGE_HEADER,
                                                MMTEMPORARY);
        if (ExtendedHeader == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            MI_BAD_IMAGE (7);
            goto BadPeImageSegment;
        }

        //
        // Build an MDL for the operation.
        //

        MmInitializeMdl (Mdl, ExtendedHeader, MM_MAXIMUM_IMAGE_HEADER);

        MmBuildMdlForNonPagedPool (Mdl);

        StartingOffset.LowPart = PtrToUlong(PAGE_ALIGN ((ULONG)DosHeader->e_lfanew));

        KeClearEvent (&InPageEvent);

        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {

            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   NULL);

            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (8);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

        if (IoStatus.Information != MM_MAXIMUM_IMAGE_HEADER) {

            //
            // 2 full pages were not read from the file, no need to zero
            // any remaining bytes (unless we were to cache it), but do
            // check that the amount read covers the minimum NT header.
            //

            if (IoStatus.Information < BYTE_OFFSET ((ULONG)DosHeader->e_lfanew) + sizeof (IMAGE_NT_HEADERS)) {
                Status = STATUS_INVALID_IMAGE_PROTECT;
                MI_BAD_IMAGE (0x18);
                goto BadPeImageSegment;
            }
        }

        NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)ExtendedHeader +
                          BYTE_OFFSET((ULONG)DosHeader->e_lfanew));

        NtHeaderSize = (ULONG)IoStatus.Information -
                            (ULONG)(BYTE_OFFSET((ULONG)DosHeader->e_lfanew));
    }
    else if ((ULONG)DosHeader->e_lfanew + sizeof (IMAGE_NT_HEADERS) > IoStatus.Information) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        MI_BAD_IMAGE (0x27);
        goto BadPeImageSegment;
    }
    else {
        NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)DosHeader +
                                          (ULONG)DosHeader->e_lfanew);
        NtHeaderSize = (ULONG) IoStatus.Information - (ULONG)DosHeader->e_lfanew;
    }

    FileHeader = &NtHeader->FileHeader;
    FileHeaderNumberOfSections = FileHeader->NumberOfSections;

    //
    // Check to see if this is an NT image or a DOS or OS/2 image.
    //

    Status = MiVerifyImageHeader (NtHeader, DosHeader, NtHeaderSize);
    if (Status != STATUS_SUCCESS) {
        MI_BAD_IMAGE (9);
        goto BadPeImageSegment;
    }

    CheckSplitPages = FALSE;

#if defined(_WIN64)

    //
    // Don't let bogus headers cause system alignment faults.
    //

    if (FileHeader->SizeOfOptionalHeader & (sizeof (ULONG_PTR) - 1)) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xE);
        goto BadPeImageSegment;
    }

    if (NtHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        NtHeader32 = NULL;

#endif
        ImageAlignment = NtHeader->OptionalHeader.SectionAlignment;
        FileAlignment = NtHeader->OptionalHeader.FileAlignment - 1;
        SizeOfImage = NtHeader->OptionalHeader.SizeOfImage;
        LoaderFlags = NtHeader->OptionalHeader.LoaderFlags;
        ImageBase = NtHeader->OptionalHeader.ImageBase;
        SizeOfHeaders = NtHeader->OptionalHeader.SizeOfHeaders;

        //
        // Read in the COM+ directory entry.
        //

        ComPlusDirectoryEntry = &NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

#if defined (_WIN64)
    }
    else {

        //
        // The image is 32-bit.  All code below this point must check
        // if NtHeader is NULL.  If it is, then the image is PE32 and
        // NtHeader32 must be used.
        //

        NtHeader32 = (PIMAGE_NT_HEADERS32) NtHeader;
        NtHeader = NULL;

        ImageAlignment = NtHeader32->OptionalHeader.SectionAlignment;
        FileAlignment = NtHeader32->OptionalHeader.FileAlignment - 1;
        SizeOfImage = NtHeader32->OptionalHeader.SizeOfImage;
        LoaderFlags = NtHeader32->OptionalHeader.LoaderFlags;
        ImageBase = NtHeader32->OptionalHeader.ImageBase;
        SizeOfHeaders = NtHeader32->OptionalHeader.SizeOfHeaders;

        //
        // Read in the COM+ directory entry.
        //

        ComPlusDirectoryEntry = &NtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
    }
#endif

    NumberOfPtes = BYTES_TO_PAGES (SizeOfImage);

    if (NumberOfPtes == 0) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xA);
        goto BadPeImageSegment;
    }

#if defined (_WIN64)
    if (NumberOfPtes >= _4gb) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xB);
        goto BadPeImageSegment;
    }
#endif

    //
    // Set the appropriate bit for .NET images inside the image's
    // section loader flags.
    //

    if ((ComPlusDirectoryEntry->VirtualAddress != 0) &&
        (ComPlusDirectoryEntry->Size != 0)) {

        LoaderFlags |= IMAGE_LOADER_FLAGS_COMPLUS;
    }

    RoundingAlignment = ImageAlignment;
    NumberOfSubsections = FileHeader->NumberOfSections;

    if ((ImageAlignment >= PAGE_SIZE) || (CheckSplitPages == TRUE)) {

        //
        // Allocate a control area and a subsection for each section
        // header plus one for the image header which has no section.
        //

        SubsectionsAllocated = NumberOfSubsections + 1;

        ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(CONTROL_AREA) +
                                                    (sizeof(SUBSECTION) *
                                                    SubsectionsAllocated),
                                            'iCmM');
        SingleSubsection = FALSE;
    }
    else {

        //
        // The image alignment is less than the page size,
        // map the image with a single subsection.
        //

        ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                      sizeof(CONTROL_AREA) + sizeof(SUBSECTION),
                      MMCONTROL);

        SubsectionsAllocated = 1;
        SingleSubsection = TRUE;
    }

    if (ControlArea == NULL) {

        //
        // The requested pool could not be allocated.
        //

        Status = STATUS_INSUFFICIENT_RESOURCES;
        MI_BAD_IMAGE (0xC);
        goto BadPeImageSegment;
    }

    //
    // Zero the control area and the FIRST subsection.
    //

    RtlZeroMemory (ControlArea, sizeof(CONTROL_AREA) + sizeof(SUBSECTION));

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    Subsection = (PSUBSECTION)(ControlArea + 1);

    SizeOfSegment = sizeof(SEGMENT) + (sizeof(MMPTE) * ((ULONG)NumberOfPtes - 1)) +
                    sizeof(SECTION_IMAGE_INFORMATION);

    NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                        SizeOfSegment,
                                        MMSECT);

    if (NewSegment == NULL) {

        //
        // The requested pool could not be allocated.
        //

        MI_BAD_IMAGE (0xD);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto BadPeImageSegment;
    }

    *Segment = NewSegment;

    RtlZeroMemory (NewSegment, sizeof(SEGMENT));

    NewSegment->PrototypePte = &NewSegment->ThePtes[0];

    PointerPte = &NewSegment->ThePtes[0];

    Pfn1->u2.Blink = (PFN_NUMBER) PointerPte;

    NewSegment->ControlArea = ControlArea;
    NewSegment->u2.ImageInformation =
        (PSECTION_IMAGE_INFORMATION)((PCHAR)NewSegment + sizeof(SEGMENT) +
                                       (sizeof(MMPTE) * (NumberOfPtes - 1)));
    NewSegment->TotalNumberOfPtes = (ULONG) NumberOfPtes;
    NewSegment->NonExtendedPtes = (ULONG) NumberOfPtes;
    NewSegment->SizeOfSegment = NumberOfPtes * PAGE_SIZE;

    RtlZeroMemory (NewSegment->u2.ImageInformation,
                   sizeof (SECTION_IMAGE_INFORMATION));

    NewSegment->u2.ImageInformation->ImageFileSize = EndOfFile.LowPart;

#if DBG

    //
    // Zero fill the prototype PTEs so they can be checked in the error
    // path to make sure no valid entries got left behind.
    //

    if (NumberOfPtes != 0) {
        MiZeroMemoryPte (PointerPte, NumberOfPtes);
    }

#endif

//
// This code is built twice on the Win64 build - once for PE32+ and once for
// PE32 images.
//
#define INIT_IMAGE_INFORMATION(OptHdr) {                            \
    NewSegment->u2.ImageInformation->TransferAddress =              \
                    (PVOID)((ULONG_PTR)((OptHdr).ImageBase) +       \
                            (OptHdr).AddressOfEntryPoint);          \
    NewSegment->u2.ImageInformation->MaximumStackSize =             \
                            (OptHdr).SizeOfStackReserve;            \
    NewSegment->u2.ImageInformation->CommittedStackSize =           \
                            (OptHdr).SizeOfStackCommit;             \
    NewSegment->u2.ImageInformation->SubSystemType =                \
                            (OptHdr).Subsystem;                     \
    NewSegment->u2.ImageInformation->SubSystemMajorVersion = (USHORT)((OptHdr).MajorSubsystemVersion); \
    NewSegment->u2.ImageInformation->SubSystemMinorVersion = (USHORT)((OptHdr).MinorSubsystemVersion); \
    NewSegment->u2.ImageInformation->DllCharacteristics =           \
                            (OptHdr).DllCharacteristics;            \
    NewSegment->u2.ImageInformation->ImageContainsCode =            \
                            (BOOLEAN)(((OptHdr).SizeOfCode != 0) || \
                                      ((OptHdr).AddressOfEntryPoint != 0)); \
    }

#if defined (_WIN64)
    if (NtHeader) {
#endif
        INIT_IMAGE_INFORMATION(NtHeader->OptionalHeader);
#if defined (_WIN64)
    }
    else {

        //
        // The image is 32-bit so use the 32-bit header
        //

        INIT_IMAGE_INFORMATION(NtHeader32->OptionalHeader);
    }
#endif
    #undef INIT_IMAGE_INFORMATION

    NewSegment->u2.ImageInformation->ImageCharacteristics =
                            FileHeader->Characteristics;
    NewSegment->u2.ImageInformation->Machine = FileHeader->Machine;
    NewSegment->u2.ImageInformation->LoaderFlags = LoaderFlags;

    ControlArea->Segment = NewSegment;
    ControlArea->NumberOfSectionReferences = 1;
    ControlArea->NumberOfUserReferences = 1;
    ControlArea->u.Flags.BeingCreated = 1;
    ControlArea->u.Flags.Image = 1;
    ControlArea->u.Flags.File = 1;

    if ((ActiveDataReferences == TRUE) ||
        (IoIsDeviceEjectable(File->DeviceObject)) ||
        ((FileHeader->Characteristics &
                IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP) &&
         (FILE_REMOVABLE_MEDIA & File->DeviceObject->Characteristics)) ||
        ((FileHeader->Characteristics &
                IMAGE_FILE_NET_RUN_FROM_SWAP) &&
         (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics))) {

        //
        // This file resides on a floppy disk or a removable media or
        // network with flags set indicating it should be copied
        // to the paging file.
        //

        ControlArea->u.Flags.FloppyMedia = 1;
    }

#if DBG
    if (MiMakeImageFloppy[0] & 0x1) {
        MiMakeImageFloppy[1] += 1;
        ControlArea->u.Flags.FloppyMedia = 1;
    }
#endif

    if (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics) {

        //
        // This file resides on a redirected drive.
        //

        ControlArea->u.Flags.Networked = 1;
    }

    ControlArea->FilePointer = File;

    //
    // Build the subsection and prototype PTEs for the image header.
    //

    Subsection->ControlArea = ControlArea;
    NextVa = ImageBase;

    if ((NextVa & (X64K - 1)) != 0) {

        //
        // Image header is not aligned on a 64k boundary.
        //

        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0xF);
        goto BadPeImageSegment;
    }

    NewSegment->BasedAddress = (PVOID) NextVa;

#if DBG
    if (NextVa != 0 && NextVa == MiMatchSectionBase) {
        DbgPrintEx (DPFLTR_MM_ID,
                    DPFLTR_INFO_LEVEL, 
                    "MM: NewSegment %p being created\n",
                    NewSegment);
        DbgBreakPoint ();
    }
#endif

    if (SizeOfHeaders >= SizeOfImage) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        MI_BAD_IMAGE (0x10);
        goto BadPeImageSegment;
    }

    if (CheckSplitPages == FALSE) {
        Subsection->PtesInSubsection = MI_ROUND_TO_SIZE (SizeOfHeaders, ImageAlignment) >> PAGE_SHIFT;
    }
    else {
        Subsection->PtesInSubsection =
            (ULONG) MI_COMPUTE_PAGES_SPANNED (0,
                        MI_ROUND_TO_SIZE (SizeOfHeaders, ImageAlignment));
    }

    PointerPte = NewSegment->PrototypePte;
    Subsection->SubsectionBase = PointerPte;

    TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
    TempPte.u.Soft.Prototype = 1;

    NewSegment->SegmentPteTemplate = TempPte;
    SectorOffset = 0;

    if (SingleSubsection == TRUE) {

        //
        // The image is aligned on less than a page size boundary.
        // Initialize the single subsection to refer to the image.
        //

        PointerPte = NewSegment->PrototypePte;

        Subsection->PtesInSubsection = (ULONG) NumberOfPtes;

#if !defined (_WIN64)

        //
        // Note this only needs to be checked for 32-bit systems as NT64
        // does a much more extensive reallocation of images that are built
        // with alignment less than PAGE_SIZE.
        //

        if ((UINT64)SizeOfImage < (UINT64)EndOfFile.QuadPart) {

            //
            // Images that have a size of image (according to the header) that's
            // smaller than the actual file only get that many prototype PTEs.
            // Initialize the subsection properly so no one can read off the
            // end as that would corrupt the PFN database element's original
            // PTE entry.
            //

            Subsection->NumberOfFullSectors = (SizeOfImage >> MMSECTOR_SHIFT);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                  SizeOfImage & MMSECTOR_MASK;
        }
        else {
#endif
            Subsection->NumberOfFullSectors =
                                (ULONG)(EndOfFile.QuadPart >> MMSECTOR_SHIFT);

            ASSERT ((ULONG)(EndOfFile.HighPart & 0xFFFFF000) == 0);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                  EndOfFile.LowPart & MMSECTOR_MASK;
#if !defined (_WIN64)
        }
#endif

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_WRITECOPY;

        //
        // Set all the PTEs to the execute-read-write protection.
        // The section will control access to these and the segment
        // must provide a method to allow other users to map the file
        // for various protections.
        //

        TempPte.u.Soft.Protection = MM_EXECUTE_WRITECOPY;

        NewSegment->SegmentPteTemplate = TempPte;

        TempPteDemandZero.u.Long = 0;
        TempPteDemandZero.u.Soft.Protection = MM_EXECUTE_WRITECOPY;
        SectorOffset = 0;

        for (i = 0; i < NumberOfPtes; i += 1) {

            //
            // Set prototype PTEs.
            //

            if (SectorOffset < EndOfFile.LowPart) {

                //
                // Data resides on the disk, refer to the control section.
                //

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);

            }
            else {

                //
                // Data does not reside on the disk, use Demand zero pages.
                //

                MI_WRITE_INVALID_PTE (PointerPte, TempPteDemandZero);
            }

            SectorOffset += PAGE_SIZE;
            PointerPte += 1;
        }

        NewSegment->u1.ImageCommitment = NumberOfPtes;
    }
    else {

        //
        // Alignment is PAGE_SIZE or greater (or the image is a 32-bit
        // 4K aligned image being run with ALTPTE support).
        //

        if (SizeOfHeaders == 0) {
            MI_BAD_IMAGE (0x31);
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto BadPeImageSegment;
        }

        if (MI_ROUND_WOULD_WRAP (SizeOfHeaders, ImageAlignment)) {
            MI_BAD_IMAGE (0x32);
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto BadPeImageSegment;
        }

        if (Subsection->PtesInSubsection > NumberOfPtes) {

            //
            // Inconsistent image, size does not agree with header.
            //

            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x11);
            goto BadPeImageSegment;
        }

        NumberOfPtes -= Subsection->PtesInSubsection;

        Subsection->NumberOfFullSectors = SizeOfHeaders >> MMSECTOR_SHIFT;

        Subsection->u.SubsectionFlags.SectorEndOffset =
            SizeOfHeaders & MMSECTOR_MASK;

        Subsection->u.SubsectionFlags.ReadOnly = 1;
        Subsection->u.SubsectionFlags.Protection = MM_READONLY;

        TempPte.u.Soft.Protection = MM_READONLY;
        NewSegment->SegmentPteTemplate = TempPte;

        for (i = 0; i < Subsection->PtesInSubsection; i += 1) {

            //
            // Set all the prototype PTEs to refer to the control section.
            //

            if (SectorOffset < SizeOfHeaders) {
                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            else {
                MI_WRITE_ZERO_PTE (PointerPte);
            }
            SectorOffset += PAGE_SIZE;
            PointerPte += 1;
        }

        if (CheckSplitPages == FALSE) {
            NextVa += (i * PAGE_SIZE);
        }
    }

    //
    // Build the additional subsections.
    //

    PreferredImageBase = ImageBase;

    //
    // At this point the object table is read in (if it was not
    // already read in) and may displace the image header.
    //

    SectionTableEntry = NULL;
    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              FileHeader->SizeOfOptionalHeader;

    if ((BYTE_OFFSET(NtHeader) + OffsetToSectionTable +
#if defined (_WIN64)
                BYTE_OFFSET(NtHeader32) +
#endif
                ((NumberOfSubsections + 1) *
                sizeof (IMAGE_SECTION_HEADER))) <= NtHeaderSize) {

        //
        // Section tables are within the header which was read.
        //

#if defined(_WIN64)
        if (NtHeader32) {
            SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader32 +
                                    OffsetToSectionTable);
        }
        else
#endif
        {
            SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                    OffsetToSectionTable);
        }

    }
    else {

        //
        // Has an extended header been read in and are the object
        // tables resident?
        //

        if (ExtendedHeader != NULL) {

#if defined(_WIN64)
            if (NtHeader32) {
                SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader32 +
                                        OffsetToSectionTable);
            }
            else
#endif
            {
                SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                                        OffsetToSectionTable);
            }

            //
            // Is the whole range of object tables mapped by the
            // extended header?
            //

            if (NtHeaderSize < OffsetToSectionTable + (NumberOfSubsections * sizeof (IMAGE_SECTION_HEADER))) {
                SectionTableEntry = NULL;
            }
        }
    }

    if (SectionTableEntry == NULL) {

        SIZE_T BytesForSectionTableEntries;

        if ((ULONG)DosHeader->e_lfanew + OffsetToSectionTable + (NumberOfSubsections * sizeof (IMAGE_SECTION_HEADER)) <= (ULONG)DosHeader->e_lfanew) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x34);
            goto BadPeImageSegment;
        }

        BytesForSectionTableEntries = 
                    BYTE_OFFSET (DosHeader->e_lfanew + OffsetToSectionTable) +
                        (NumberOfSubsections * sizeof (IMAGE_SECTION_HEADER));

        if (BytesForSectionTableEntries > MM_MAXIMUM_IMAGE_HEADER) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x35);
            goto BadPeImageSegment;
        }

        //
        // The section table entries are not in the same
        // pages as the other data already read in.  Read in
        // the object table entries.
        //

        if (ExtendedHeader == NULL) {
            ExtendedHeader = ExAllocatePoolWithTag (NonPagedPool,
                                                    MM_MAXIMUM_IMAGE_HEADER,
                                                    MMTEMPORARY);
            if (ExtendedHeader == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                MI_BAD_IMAGE (0x12);
                goto BadPeImageSegment;
            }

            //
            // Build an MDL for the operation.
            //

            MmInitializeMdl (Mdl, ExtendedHeader, MM_MAXIMUM_IMAGE_HEADER);

            MmBuildMdlForNonPagedPool (Mdl);
        }

        StartingOffset.LowPart = PtrToUlong(PAGE_ALIGN (
                                    (ULONG)DosHeader->e_lfanew +
                                    OffsetToSectionTable));

        SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)ExtendedHeader +
                                BYTE_OFFSET((ULONG)DosHeader->e_lfanew +
                                OffsetToSectionTable));

        KeClearEvent (&InPageEvent);
        Status = IoPageRead (File,
                             Mdl,
                             &StartingOffset,
                             &InPageEvent,
                             &IoStatus);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject (&InPageEvent,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)NULL);
            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (!NT_SUCCESS(Status)) {
            MI_BAD_IMAGE (0x13);
            if ((Status != STATUS_FILE_LOCK_CONFLICT) && (Status != STATUS_FILE_IS_OFFLINE)) {
                Status = STATUS_INVALID_FILE_FOR_SECTION;
            }
            goto BadPeImageSegment;
        }

        //
        // Ensure that the entire section table entry span was read.
        //

        if (IoStatus.Information < BYTE_OFFSET ((ULONG) DosHeader->e_lfanew + OffsetToSectionTable) + (NumberOfSubsections * sizeof (IMAGE_SECTION_HEADER))) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x28);
            goto BadPeImageSegment;
        }

        //
        // From this point on NtHeader is only valid if it
        // was in the first page of the image, otherwise reading in
        // the object tables wiped it out.
        //
    }

    if ((SingleSubsection == TRUE) && (InvalidAlignmentAllowed == FALSE)) {

        //
        // The image header is no longer valid.
        //
        // Loop through all sections and make sure there is no
        // uninitialized data.
        //

        Status = STATUS_SUCCESS;

        while (NumberOfSubsections > 0) {
            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

            //
            // If the raw pointer + raw size overflows a long word,
            // return an error.
            //

            if (SectionTableEntry->PointerToRawData +
                        SectionTableEntry->SizeOfRawData <
                SectionTableEntry->PointerToRawData) {

                KdPrint(("MMCREASECT: invalid section/file size %Z\n",
                    &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x14);
                break;
            }

            //
            // If the virtual size and address does not match the rawdata
            // and invalid alignments not allowed return an error.
            //

            if (((SectionTableEntry->PointerToRawData !=
                  SectionTableEntry->VirtualAddress))
                            ||
                   (SectionVirtualSize > SectionTableEntry->SizeOfRawData)) {

                KdPrint(("MMCREASECT: invalid BSS/Trailingzero %Z\n",
                        &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x15);
                break;
            }

            SectionTableEntry += 1;
            NumberOfSubsections -= 1;
        }


        if (!NT_SUCCESS(Status)) {
            goto BadPeImageSegment;
        }

        goto PeReturnSuccess;
    }

    if ((SingleSubsection == TRUE) && (InvalidAlignmentAllowed == TRUE)) {

        TempNumberOfSubsections = NumberOfSubsections;
        TempSectionTableEntry = SectionTableEntry;

        //
        // The image header is no longer valid.
        //
        // Loop through all sections and make sure there is no
        // uninitialized data.
        //
        // Also determine if there are shared data sections and
        // the number of extra PTEs they require.
        //

        AdditionalSubsections = 0;
        AdditionalPtes = 0;
        AdditionalBasePtes = 0;
        RoundingAlignment = PAGE_SIZE;

        while (TempNumberOfSubsections > 0) {
            ULONG EndOfSection;
            ULONG ExtraPages;

            if (TempSectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = TempSectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = TempSectionTableEntry->Misc.VirtualSize;
            }

            EndOfSection = TempSectionTableEntry->PointerToRawData +
                           TempSectionTableEntry->SizeOfRawData;

            //
            // If the raw pointer + raw size overflows a long word, return an error.
            //

            if (EndOfSection < TempSectionTableEntry->PointerToRawData) {

                KdPrint(("MMCREASECT: invalid section/file size %Z\n",
                    &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x16);

                goto BadPeImageSegment;
            }

            //
            // If the section goes past SizeOfImage then allocate
            // additional PTEs.  On x86 this is handled by the subsection
            // mapping.  Note the additional data must be in memory so
            // it can be shuffled around later.
            //

            if ((EndOfSection <= EndOfFile.LowPart) &&
                (EndOfSection > SizeOfImage)) {

                //
                // Allocate enough PTEs to cover the end of this section.
                // Maximize with any other sections that extend beyond
                // SizeOfImage.
                //

                if (MI_ROUND_WOULD_WRAP (EndOfSection, RoundingAlignment)) {
                    Status = STATUS_INVALID_IMAGE_FORMAT;
                    MI_BAD_IMAGE (0x2A);
                    goto BadPeImageSegment;
                }

                ExtraPages = MI_ROUND_TO_SIZE (EndOfSection, RoundingAlignment) >> PAGE_SHIFT;
                if ((ExtraPages > OrigNumberOfPtes) &&
                    (ExtraPages - OrigNumberOfPtes > AdditionalBasePtes)) {

                    AdditionalBasePtes = ExtraPages - (ULONG) OrigNumberOfPtes;
                }
            }

            //
            // Count number of shared data sections and additional PTEs needed.
            //

            if ((TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) &&
                (!(TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
                 (TempSectionTableEntry->Characteristics & IMAGE_SCN_MEM_WRITE))) {
                ULONG PtesInThisSection;

                if (MI_ROUND_WOULD_WRAP (SectionVirtualSize, RoundingAlignment)) {
                    Status = STATUS_INVALID_IMAGE_FORMAT;
                    MI_BAD_IMAGE (0x2D);
                    goto BadPeImageSegment;
                }

                PtesInThisSection =
                    MI_ROUND_TO_SIZE (SectionVirtualSize, RoundingAlignment) >> PAGE_SHIFT;

                if ((PtesInThisSection != 0) && 
                    (AdditionalPtes + PtesInThisSection <= AdditionalPtes)) {

                    Status = STATUS_INVALID_IMAGE_FORMAT;
                    MI_BAD_IMAGE (0x2E);
                    goto BadPeImageSegment;
                }

                AdditionalPtes += PtesInThisSection;
                AdditionalSubsections += 1;
            }

            TempSectionTableEntry += 1;
            TempNumberOfSubsections -= 1;
        }

        if (AdditionalBasePtes == 0 && (AdditionalSubsections == 0 || AdditionalPtes == 0)) {

            //
            // No shared data sections.
            //

            goto PeReturnSuccess;
        }

        if (AdditionalBasePtes + AdditionalPtes < AdditionalBasePtes) {
            MI_BAD_IMAGE (0x2F);
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto BadPeImageSegment;
        }

        if (OrigNumberOfPtes + AdditionalBasePtes + AdditionalPtes < OrigNumberOfPtes) {
            MI_BAD_IMAGE (0x30);
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto BadPeImageSegment;
        }

        //
        // Check that the new number of PTEs of PTEs cannot cause the
        // segment prototype PTE allocation size to wrap.  Since originally
        // the number of PTEs came from the ULONG SizeOfImage in the
        // OptionalHeader, we know it was ok then.  But adding various
        // subsection counts above may have pushed it over the limit.
        //

        if (((MAXULONG_PTR - (sizeof (SEGMENT) + sizeof (SECTION_IMAGE_INFORMATION))) / sizeof (MMPTE)) < OrigNumberOfPtes + AdditionalBasePtes + AdditionalPtes - 1) {
            MI_BAD_IMAGE (0x33);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

        //
        // There are additional base PTEs or shared data sections.
        // For shared sections, allocate new PTEs for these sections
        // at the end of the image.  The usermode wow loader will change
        // fixups to point to the new pages.
        //
        // First reallocate the control area.
        //

        NewSubsectionsAllocated = SubsectionsAllocated + AdditionalSubsections;

        NewControlArea = ExAllocatePoolWithTag(NonPagedPool,
                                    (ULONG) (sizeof(CONTROL_AREA) +
                                            (sizeof(SUBSECTION) *
                                                NewSubsectionsAllocated)),
                                                'iCmM');
        if (NewControlArea == NULL) {
            MI_BAD_IMAGE (0x17);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

        //
        // Copy the old control area to the new one, modify some fields.
        //

        RtlCopyMemory (NewControlArea, ControlArea,
                        sizeof(CONTROL_AREA) +
                            sizeof(SUBSECTION) * SubsectionsAllocated);

        //
        // Now allocate a new segment that has the newly calculated number
        // of PTEs, initialize it from the previously allocated new segment,
        // and overwrite the fields that should be changed.
        //

        OldSegment = NewSegment;

        OrigNumberOfPtes += AdditionalBasePtes;
        PointerPte += AdditionalBasePtes;

        SizeOfSegment = sizeof(SEGMENT) +
                     (sizeof(MMPTE) * (OrigNumberOfPtes + AdditionalPtes - 1)) +
                        sizeof(SECTION_IMAGE_INFORMATION);

        NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                            SizeOfSegment,
                                            MMSECT);

        if (NewSegment == NULL) {

            //
            // The requested pool could not be allocated.
            //

            MI_BAD_IMAGE (0x18);
            ExFreePool (NewControlArea);
            ExFreePool (OldSegment);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto BadPeImageSegment;
        }

        *Segment = NewSegment;
        RtlCopyMemory (NewSegment, OldSegment, sizeof(SEGMENT));

        //
        // Align the prototype PTEs on the proper boundary.
        //

        NewPointerPte = &NewSegment->ThePtes[0];
        NewSegment->PrototypePte = &NewSegment->ThePtes[0];

        PointerPte = NewSegment->PrototypePte +
                                       (PointerPte - OldSegment->PrototypePte);

        NewSegment->ControlArea = NewControlArea;

        //
        // This reallocation code is not zeroing the extra PTEs and this got
        // flagged for invalid PTE states in an error path below.  The error
        // path (checked only) now only checks if no extra subsections were
        // allocated.
        //

        NewSegment->SegmentFlags.ExtraSharedWowSubsections = 1;
        NewSegment->u2.ImageInformation =
            (PSECTION_IMAGE_INFORMATION)((PCHAR)NewSegment + sizeof(SEGMENT) +
                     (sizeof(MMPTE) * (OrigNumberOfPtes + AdditionalPtes - 1)));

        NewSegment->TotalNumberOfPtes = (ULONG)(OrigNumberOfPtes + AdditionalPtes);
        NewSegment->NonExtendedPtes = (ULONG)(OrigNumberOfPtes + AdditionalPtes);
        NewSegment->SizeOfSegment = (UINT64)(OrigNumberOfPtes + AdditionalPtes) * PAGE_SIZE;

        RtlCopyMemory (NewSegment->u2.ImageInformation,
                       OldSegment->u2.ImageInformation,
                       sizeof (SECTION_IMAGE_INFORMATION));

        //
        // Adjust the prototype PTE pointer of the PE header page to point
        // at the new segment.
        //

        ASSERT (Pfn1->u2.Blink == (PFN_NUMBER) OldSegment->PrototypePte);
        Pfn1->u2.Blink = (PFN_NUMBER) NewSegment->PrototypePte;

        //
        // Now change the fields in the subsections to account for the new
        // control area and the new segment. Also change the PTEs in the
        // newly allocated segment to point to the new subsections.
        //

        NewControlArea->Segment = NewSegment;

        Subsection = (PSUBSECTION)(ControlArea + 1);
        NewSubsection = (PSUBSECTION)(NewControlArea + 1);
        NewSubsection->PtesInSubsection += AdditionalBasePtes;

        for (i = 0; i < SubsectionsAllocated; i += 1) {

            //
            // Note: SubsectionsAllocated is always 1 (for wx86), so this loop
            // is executed only once.
            //

            NewSubsection->ControlArea = (PCONTROL_AREA) NewControlArea;

            NewSubsection->SubsectionBase = NewSegment->PrototypePte +
                    (Subsection->SubsectionBase - OldSegment->PrototypePte);

            NewPointerPte = NewSegment->PrototypePte;
            OldPointerPte = OldSegment->PrototypePte;

            TempPte.u.Long = MiGetSubsectionAddressForPte (NewSubsection);
            TempPte.u.Soft.Prototype = 1;

            for (j = 0; j < OldSegment->TotalNumberOfPtes+AdditionalBasePtes; j += 1) {

                if ((OldPointerPte->u.Soft.Prototype == 1) &&
                    (MiGetSubsectionAddress (OldPointerPte) == Subsection)) {
                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (OldPointerPte);
                    TempPte.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (NewPointerPte, TempPte);
                }
                else if (i == 0) {

                    //
                    // Since the outer for loop is executed only once, there
                    // is no need for the i == 0 above, but it is safer to
                    // have it. If the code changes later and other sections
                    // are added, their PTEs will get initialized here as
                    // DemandZero and if they are not DemandZero, they will be
                    // overwritten in a later iteration of the outer loop.
                    // For now, this else if clause will be executed only
                    // for DemandZero PTEs.
                    //

                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (OldPointerPte);
                    TempPteDemandZero.u.Long = 0;
                    TempPteDemandZero.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (NewPointerPte, TempPteDemandZero);
                }

                NewPointerPte += 1;

                //
                // Stop incrementing the OldPointerPte at the last entry
                // and use it for the additional base PTEs.
                //

                if (j < OldSegment->TotalNumberOfPtes - 1) {
                    OldPointerPte += 1;
                }
            }

            Subsection += 1;
            NewSubsection += 1;
        }


        RtlZeroMemory (NewSubsection,
                            sizeof(SUBSECTION) * AdditionalSubsections);

        ExFreePool (OldSegment);
        ExFreePool (ControlArea);
        ControlArea = (PCONTROL_AREA) NewControlArea;

        //
        // Adjust some variables that are used below.
        // PointerPte has already been set above before OldSegment was freed.
        //

        SubsectionsAllocated = NewSubsectionsAllocated;
        Subsection = NewSubsection - 1; // Points to last used subsection.
        NumberOfPtes = AdditionalPtes;  // # PTEs that haven't yet been used in
                                        // previous subsections.

        //
        // Additional base PTEs have been added.  Only continue if there are
        // additional subsections to process.
        //

        if (AdditionalSubsections == 0 || AdditionalPtes == 0) {
            // no shared data sections
            goto PeReturnSuccess;
        }
    }

    ImageFileSize = EndOfFile.LowPart + 1;

    while (NumberOfSubsections > 0) {

        if ((InvalidAlignmentAllowed == FALSE) ||
            ((SectionTableEntry->Characteristics & IMAGE_SCN_MEM_SHARED) &&
             (!(SectionTableEntry->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
              (SectionTableEntry->Characteristics & IMAGE_SCN_MEM_WRITE)))) {

            //
            // Handle case where virtual size is 0.
            //

            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            }
            else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

            //
            // Fix for Borland linker problem.  The SizeOfRawData can
            // be a zero, but the PointerToRawData is not zero.
            // Set it to zero.
            //

            if (SectionTableEntry->SizeOfRawData == 0) {
                SectionTableEntry->PointerToRawData = 0;
            }

            //
            // If the section information wraps return an error.
            //

            if (SectionTableEntry->PointerToRawData +
                        SectionTableEntry->SizeOfRawData <
                SectionTableEntry->PointerToRawData) {

                MI_BAD_IMAGE (0x19);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            Subsection->NextSubsection = (Subsection + 1);

            Subsection += 1;
            Subsection->ControlArea = ControlArea;
            Subsection->NextSubsection = NULL;
            Subsection->UnusedPtes = 0;

            if (((NextVa != (PreferredImageBase + SectionTableEntry->VirtualAddress)) && (InvalidAlignmentAllowed == FALSE)) ||
                (SectionVirtualSize == 0)) {

                //
                // The specified virtual address does not align
                // with the next prototype PTE.
                //

                MI_BAD_IMAGE (0x1A);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            if (MI_ROUND_WOULD_WRAP (SectionVirtualSize, RoundingAlignment)) {
                MI_BAD_IMAGE (0x2B);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            Subsection->PtesInSubsection =
                MI_ROUND_TO_SIZE (SectionVirtualSize, RoundingAlignment)
                                                                >> PAGE_SHIFT;

            if (Subsection->PtesInSubsection > NumberOfPtes) {
                //
                // Inconsistent image, size does not agree with
                // object tables.
                //

                MI_BAD_IMAGE (0x1B);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            NumberOfPtes -= Subsection->PtesInSubsection;

            Subsection->u.LongFlags = 0;
            Subsection->StartingSector =
                          SectionTableEntry->PointerToRawData >> MMSECTOR_SHIFT;

            //
            // Align ending sector on file align boundary.
            //

            EndingAddress = (SectionTableEntry->PointerToRawData +
                                         SectionTableEntry->SizeOfRawData +
                                         FileAlignment) & ~FileAlignment;

            if (EndingAddress < SectionTableEntry->PointerToRawData) {
                MI_BAD_IMAGE (0x36);
                Status = STATUS_INVALID_IMAGE_FORMAT;
                goto BadPeImageSegment;
            }

            Subsection->NumberOfFullSectors = (ULONG)
                             ((EndingAddress >> MMSECTOR_SHIFT) -
                             Subsection->StartingSector);

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                        (ULONG) EndingAddress & MMSECTOR_MASK;

            Subsection->SubsectionBase = PointerPte;

            //
            // Build both a demand zero PTE and a PTE pointing to the
            // subsection.
            //

            TempPte.u.Long = 0;
            TempPteDemandZero.u.Long = 0;

            TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
            TempPte.u.Soft.Prototype = 1;
            ImageFileSize = SectionTableEntry->PointerToRawData +
                                        SectionTableEntry->SizeOfRawData;
            TempPte.u.Soft.Protection =
                     MiGetImageProtection (SectionTableEntry->Characteristics);
            TempPteDemandZero.u.Soft.Protection = TempPte.u.Soft.Protection;

            if (SectionTableEntry->PointerToRawData == 0) {
                TempPte = TempPteDemandZero;
            }

            Subsection->u.SubsectionFlags.ReadOnly = 1;
            Subsection->u.SubsectionFlags.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);

            //
            // Assume the subsection will be unwritable and therefore
            // won't be charged for any commitment.
            //

            SectionCommit = FALSE;
            ImageCommit = FALSE;

            if (TempPte.u.Soft.Protection & MM_PROTECTION_WRITE_MASK) {
                if ((TempPte.u.Soft.Protection & MM_COPY_ON_WRITE_MASK)
                                                == MM_COPY_ON_WRITE_MASK) {

                    //
                    // This page is copy on write, charge ImageCommitment
                    // for all pages in this subsection.
                    //

                    ImageCommit = TRUE;
                }
                else {

                    //
                    // This page is write shared, charge commitment when
                    // the mapping completes.
                    //

                    SectionCommit = TRUE;
                    Subsection->u.SubsectionFlags.GlobalMemory = 1;
                    ControlArea->u.Flags.GlobalMemory = 1;
                }
            }

            NewSegment->SegmentPteTemplate = TempPte;
            SectorOffset = 0;
            SizeOfRawData = SectionTableEntry->SizeOfRawData;

            for (i = 0; i < Subsection->PtesInSubsection; i += 1) {

                //
                // Set all the prototype PTEs to refer to the control section.
                //

                if (SectorOffset < SectionVirtualSize) {

                    //
                    // Make PTE accessible.
                    //

                    if (SectionCommit) {
                        NewSegment->NumberOfCommittedPages += 1;
                    }
                    if (ImageCommit) {
                        NewSegment->u1.ImageCommitment += 1;
                    }

                    if (SectorOffset < SizeOfRawData) {

                        //
                        // Data resides on the disk, use the subsection
                        // format PTE.
                        //

                        MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                    }
                    else {

                        //
                        // Demand zero pages.
                        //

                        MI_WRITE_INVALID_PTE (PointerPte, TempPteDemandZero);
                    }
                }
                else {

                    //
                    // No access pages.
                    // This can only occur for images with section alignment
                    // greater than the native PAGE_SIZE.
                    //

                    ASSERT (CheckSplitPages == FALSE);
                    MI_WRITE_ZERO_PTE (PointerPte);
                }

                SectorOffset += PAGE_SIZE;
                PointerPte += 1;
                NextVa += PAGE_SIZE;
            }
        }

        SectionTableEntry += 1;
        NumberOfSubsections -= 1;
    }

    ASSERT ((PointerPte > NewSegment->PrototypePte) &&
            (PointerPte <= NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));

    if (InvalidAlignmentAllowed == FALSE) {

        //
        // Account for the number of subsections that really are mapped.
        //

        ASSERT ((ImageAlignment >= PAGE_SIZE) || (CheckSplitPages == TRUE));

        //
        // If the file size is not as big as the image claimed to be,
        // return an error.
        //

        if (ImageFileSize > EndOfFile.LowPart) {

            //
            // Invalid image size.
            //

            KdPrint(("MMCREASECT: invalid image size - file size %lx - image size %lx\n %Z\n",
                EndOfFile.LowPart, ImageFileSize, &File->FileName));
            Status = STATUS_INVALID_IMAGE_FORMAT;
            MI_BAD_IMAGE (0x21);
            goto BadPeImageSegment;
        }

        //
        // The total number of PTEs was decremented as sections were built,
        // make sure that there are less than 64k's worth at this point.
        //

        if (NumberOfPtes >= (ImageAlignment >> PAGE_SHIFT)) {

            if ((CheckSplitPages == TRUE) && (NumberOfPtes == 0)) {
                NOTHING;
            }
            else {

                //
                // Inconsistent image, size does not agree with object tables.
                //

                KdPrint(("MMCREASECT: invalid image - PTE left %lx, image name %Z\n",
                    NumberOfPtes, &File->FileName));

                Status = STATUS_INVALID_IMAGE_FORMAT;
                MI_BAD_IMAGE (0x22);
                goto BadPeImageSegment;
            }
        }

        //
        // Set any remaining PTEs to no access.
        //

        if (NumberOfPtes != 0) {
            MiZeroMemoryPte (PointerPte, NumberOfPtes);
        }

        if ((ExtendedHeader == NULL) &&
            (SizeOfHeaders < PAGE_SIZE)) {

            ULONG MaxToZero = PAGE_SIZE;

            //
            // Zero remaining portion of header.
            //

            if (SizeOfHeaders < MaxToZero) {

                RtlZeroMemory ((PVOID)((PCHAR)Base + SizeOfHeaders),
                               MaxToZero - SizeOfHeaders);

                //
                // Mark the PFN as modified so we don't lose the zeroing if
                // the page gets trimmed.
                //

                MarkHeaderModified = 1;
            }
        }
    }

    CommitCharged = NewSegment->NumberOfCommittedPages;

    if (CommitCharged != 0) {

        //
        // Commit the pages for the image section.
        //

        if (MiChargeCommitment (CommitCharged, NULL) == FALSE) {
            MI_BAD_IMAGE (0x23);
            Status = STATUS_COMMITMENT_LIMIT;
            CommitCharged = 0;
            goto BadPeImageSegment;
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_IMAGE, CommitCharged);
        Status = STATUS_SUCCESS;

        InterlockedExchangeAddSizeT (&MmSharedCommit, CommitCharged);
    }

PeReturnSuccess:

    //
    // If this image is global per session (or is going to be executed directly
    // from ROM), then allocate a large control area.  Note this doesn't need
    // to be done for systemwide global control areas or non-global control
    // areas.
    //

    GlobalPerSession = FALSE;
    if ((ControlArea->u.Flags.GlobalMemory) &&
        ((LoaderFlags & IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL) == 0)) {

        GlobalPerSession = TRUE;
    }

    if (GlobalPerSession == TRUE) {

        LargeControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                            (ULONG)(sizeof(LARGE_CONTROL_AREA) +
                                                    (sizeof(SUBSECTION) *
                                                    SubsectionsAllocated)),
                                                    'iCmM');
        if (LargeControlArea == NULL) {

            //
            // The requested pool could not be allocated.
            //

            MI_BAD_IMAGE (0x24);
            Status = STATUS_INSUFFICIENT_RESOURCES;

            goto BadPeImageSegment;
        }

        //
        // Copy the normal control area into our larger one, fix the linkages,
        // Fill in the additional fields in the new one and free the old one.
        //

        RtlCopyMemory (LargeControlArea, ControlArea, sizeof(CONTROL_AREA));

        ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

        Subsection = (PSUBSECTION)(ControlArea + 1);
        NewSubsection = (PSUBSECTION)(LargeControlArea + 1);

        for (i = 0; i < SubsectionsAllocated; i += 1) {
            RtlCopyMemory (NewSubsection, Subsection, sizeof(SUBSECTION));
            NewSubsection->ControlArea = (PCONTROL_AREA) LargeControlArea;
            NewSubsection->NextSubsection = (NewSubsection + 1);

            PointerPte = NewSegment->PrototypePte;

            TempPte.u.Long = MiGetSubsectionAddressForPte (NewSubsection);
            TempPte.u.Soft.Prototype = 1;

            for (j = 0; j < NewSegment->TotalNumberOfPtes; j += 1) {

                ASSERT (PointerPte->u.Hard.Valid == 0);

                if ((PointerPte->u.Soft.Prototype == 1) &&
                    (MiGetSubsectionAddress (PointerPte) == Subsection)) {

                    OriginalProtection = MI_GET_PROTECTION_FROM_SOFT_PTE (PointerPte);

                    TempPte.u.Soft.Protection = OriginalProtection;
                    MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                }

                PointerPte += 1;
            }

            Subsection += 1;
            NewSubsection += 1;
        }

        (NewSubsection - 1)->NextSubsection = NULL;

        NewSegment->ControlArea = (PCONTROL_AREA) LargeControlArea;

        if (GlobalPerSession == TRUE) {
            LargeControlArea->u.Flags.GlobalOnlyPerSession = 1;

            LargeControlArea->SessionId = 0;
            InitializeListHead (&LargeControlArea->UserGlobalList);
        }

        ExFreePool (ControlArea);

        ControlArea = (PCONTROL_AREA) LargeControlArea;
    }

    if (BasePte != NULL) {
        MiReleaseSystemPtes (BasePte, 1, SystemPteSpace);
    }

    //
    // Turn the image header and any straddler pages into transition pages
    // within the prototype PTEs.
    //
    // Note this could not be done earlier because otherwise these pages
    // could have been trimmed (and reused) before we even finished
    // initializing the segment.  This would be especially bad for the page
    // that contains the PE header because we'd still have it mapped and
    // be looking at its section header entries, etc while the page
    // was being reused !
    //

    PointerPte = NewSegment->PrototypePte;

    Pfn1 = ImagePages;

    do {

        Pfn2 = (PMMPFN) Pfn1->u1.Flink;

        PointerPte = (PMMPTE) Pfn1->u2.Blink;

        //
        // Note straddler pages must always get marked modified.  The PE
        // header only gets marked modified if it was shared with a straddler.
        //

        //
        // The list is reversed, so the last entry (Pfn2 == NULL) is
        // the PE header.
        //

        if (Pfn2 == NULL) {
                        
            ASSERT (PointerPte == NewSegment->PrototypePte);

            if (MarkHeaderModified == 0) {

                //
                // The PE header was not shared with a straddler.
                //

                MarkModified = FALSE;
            }
        }

        ASSERT ((PointerPte >= NewSegment->PrototypePte) &&
                (PointerPte < NewSegment->PrototypePte + NewSegment->TotalNumberOfPtes));

        //
        // Mark straddler pages as modified so they will be written
        // and retrieved from the pagefile (and never from the
        // filesystem again).  Put the straddler page on the modified
        // list and set the transition bit in the prototype PTE.
        //

        MiUpdateImageHeaderPage (PointerPte,
                                 Pfn1 - MmPfnDatabase,
                                 ControlArea,
                                 MarkModified);

        Pfn1 = Pfn2;

    } while (Pfn1 != NULL);

    if (ExtendedHeader != NULL) {
        ExFreePool (ExtendedHeader);
    }

    return STATUS_SUCCESS;


    //
    // Error returns from image verification.
    //

BadPeImageSegment:

    ASSERT (!NT_SUCCESS (Status));

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (NewSegment != NULL) {

        if (CommitCharged != 0) {

            ASSERT (CommitCharged == NewSegment->NumberOfCommittedPages);

            MiReturnCommitment (CommitCharged);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_IMAGE_NO_LARGE_CA, CommitCharged);
            InterlockedExchangeAddSizeT (&MmSharedCommit, 0 - CommitCharged);
        }

#if DBG

        //
        // The wow reallocation code doesn't zero the extra PTEs so skip this
        // check for that case.
        //

        if (NewSegment->SegmentFlags.ExtraSharedWowSubsections == 0) {

            PointerPte = NewSegment->PrototypePte;

            for (i = 0; i < NewSegment->TotalNumberOfPtes; i += 1) {

                TempPte.u.Long = PointerPte->u.Long;

                ASSERT ((TempPte.u.Hard.Valid == 0) &&
                        ((TempPte.u.Soft.Prototype == 1) ||
                        (TempPte.u.Soft.Transition == 0)));

                PointerPte += 1;
            }
        }

#endif

    }

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (BasePte != NULL) {
        MiReleaseSystemPtes (BasePte, 1, SystemPteSpace);
    }

    Pfn1 = ImagePages;

    do {
        Pfn2 = (PMMPFN) Pfn1->u1.Flink;

        Pfn1->u2.Blink = 0;     // Clear prototype PTE pointer before freeing

        MiRemoveImageHeaderPage (Pfn1 - MmPfnDatabase);

        Pfn1 = Pfn2;

    } while (Pfn1 != NULL);

    ASSERT ((ControlArea == NULL) || (ControlArea->NumberOfPfnReferences == 0));

    if (NewSegment != NULL) {
        ExFreePool (NewSegment);
    }

    if (ControlArea != NULL) {
        ExFreePool (ControlArea);
    }

    if (ExtendedHeader != NULL) {
        ExFreePool (ExtendedHeader);
    }

    return Status;
}


LOGICAL
MiCheckDosCalls (
    IN PIMAGE_OS2_HEADER Os2Header,
    IN ULONG HeaderSize
    )

/*++

Routine Description:

    This routine checks for DOS calls.

Arguments:

    Os2Header - Supplies a kernelmode pointer to the OS2 header.

    HeaderSize - Supplies the length in bytes of the mapped header.

Return Value:

    Returns TRUE if this is a Win-16 image, FALSE if not.

--*/

{
    PUCHAR ImportTable;
    UCHAR EntrySize;
    USHORT ModuleCount;
    USHORT ModuleSize;
    USHORT i;
    PUSHORT ModuleTable;

    PAGED_CODE();

    //
    // If there are no modules to check return immediately.
    //

    ModuleCount = Os2Header->ne_cmod;

    if (ModuleCount == 0) {
        return FALSE;
    }

    //
    // exe headers are notorious for having junk values for offsets
    // in the import table and module table so the header must be very
    // carefully validated.
    //

    //
    // Find out where the Module ref table is. Mod table has two byte
    // for each entry in import table. These two bytes tell the offset
    // in the import table for that entry.
    //

    ModuleTable = (PUSHORT)((PCHAR)Os2Header + (ULONG)Os2Header->ne_modtab);

    //
    // Make sure that the module table fits within the passed-in header.
    // Note that each module table entry is 2 bytes long.
    //

    if (((ULONG)Os2Header->ne_modtab + (ModuleCount * 2)) > HeaderSize) {
        return FALSE;
    }

    //
    // Now search individual entries for DOSCALLs.
    //

    for (i = 0; i < ModuleCount; i += 1) {

        ModuleSize = *((UNALIGNED USHORT *)ModuleTable);

        //
        // Import table has count byte followed by the string where count
        // is the string length.
        //

        ImportTable = (PUCHAR)((PCHAR)Os2Header +
                      (ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize);

        //
        // Make sure the offset is within the valid range.
        //

        if (((ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize) >= HeaderSize) {
            return FALSE;
        }

        EntrySize = *ImportTable++;

        //
        // 0 is a bad size, bail out.
        //

        if (EntrySize == 0) {
            return FALSE;
        }

        //
        // Make sure the offset is within the valid range.
        // The sizeof(UCHAR) is included in the check because ImportTable
        // was incremented above and is used in the RtlEqualMemory
        // comparison below.
        //

        if (((ULONG)Os2Header->ne_imptab + (ULONG)ModuleSize +
                        (ULONG)EntrySize + sizeof(UCHAR)) > HeaderSize) {
            return FALSE;
        }

        //
        // If size matches compare DOSCALLS.
        //

        if (EntrySize == 8) {
            if (RtlEqualMemory (ImportTable, "DOSCALLS", 8)) {
                return TRUE;
            }
        }

        //
        // Move on to the next module table entry.  Each entry is 2 bytes.
        //

        ModuleTable = (PUSHORT)((PCHAR)ModuleTable + 2);
    }

    return FALSE;
}


NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    )

/*++

Routine Description:

    This function checks for various inconsistencies in the image header.

Arguments:

    NtHeader - Supplies a pointer to the NT header of the image.

    DosHeader - Supplies a pointer to the DOS header of the image.

    NtHeaderSize - Supplies the size in bytes of the NT header.

Return Value:

    NTSTATUS.

--*/

{
    PCONFIGPHARLAP PharLapConfigured;
    PUCHAR         pb;
    LONG           pResTableAddress;

    PAGED_CODE();

    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) {
        if ((USHORT)NtHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE) {

            //
            // Check to see if this is a win-16 image.
            //

            if ((!MiCheckDosCalls ((PIMAGE_OS2_HEADER)NtHeader, NtHeaderSize)) &&
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 2)
                                ||
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 0)  &&
                  (((((PIMAGE_OS2_HEADER)NtHeader)->ne_expver & 0xff00) ==
                        0x200)  ||
                ((((PIMAGE_OS2_HEADER)NtHeader)->ne_expver & 0xff00) ==
                        0x300))))) {

                //
                // This is a win-16 image.
                //

                return STATUS_INVALID_IMAGE_WIN_16;
            }

            // The following OS/2 headers types go to NTDOS
            //
            // - exetype == 5 means binary is for Dos 4.0.
            //                e.g Borland Dos extender type
            //
            // - OS/2 apps which have no import table entries
            //   cannot be meant for the OS/2 ss.
            //                e.g. QuickC for dos binaries
            //
            //  - "old" Borland Dosx BC++ 3.x, Paradox 4.x
            //     exe type == 1
            //     DosHeader->e_cs * 16 + DosHeader->e_ip + 0x200 - 10
            //     contains the string " mode EXE$"
            //     but import table is empty, so we don't make special check
            //

            if (((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 5  ||
                ((PIMAGE_OS2_HEADER)NtHeader)->ne_enttab ==
                  ((PIMAGE_OS2_HEADER)NtHeader)->ne_imptab) {

                return STATUS_INVALID_IMAGE_PROTECT;
            }


            //
            // Borland Dosx types: exe type 1
            //
            //  - "new" Borland Dosx BP7.0
            //     exe type == 1
            //     DosHeader + 0x200 contains the string "16STUB"
            //     0x200 happens to be e_parhdr*16
            //

            if (((PIMAGE_OS2_HEADER)NtHeader)->ne_exetyp == 1 &&
                RtlEqualMemory((PUCHAR)DosHeader + 0x200, "16STUB", 6)) {

                return STATUS_INVALID_IMAGE_PROTECT;
            }

            //
            // Check for PharLap extended header which we run as a dos app.
            // The PharLap config block is pointed to by the SizeofHeader
            // field in the DosHdr.
            // The following algorithm for detecting a pharlap exe
            // was recommended by PharLap Software Inc.
            //

            PharLapConfigured =(PCONFIGPHARLAP) ((PCHAR)DosHeader +
                                      ((ULONG)DosHeader->e_cparhdr << 4));

            if ((PCHAR)PharLapConfigured <
                       (PCHAR)DosHeader + PAGE_SIZE - sizeof(CONFIGPHARLAP)) {
                if (RtlEqualMemory(&PharLapConfigured->uchCopyRight[0x18],
                                   "Phar Lap Software, Inc.", 24) &&
                    (PharLapConfigured->usSign == 0x4b50 ||  // stub loader type 2
                     PharLapConfigured->usSign == 0x4f50 ||  // bindable 286|DosExtender
                     PharLapConfigured->usSign == 0x5650  )) // bindable 286|DosExtender (Adv)
                  {
                    return STATUS_INVALID_IMAGE_PROTECT;
                }
            }



            //
            // Check for Rational extended header which we run as a dos app.
            // We look for the rational copyright at:
            //     wCopyRight = *(DosHeader->e_cparhdr*16 + 30h)
            //     pCopyRight = wCopyRight + DosHeader->e_cparhdr*16
            //     "Copyright (C) Rational Systems, Inc."
            //

            pb = ((PUCHAR)DosHeader + ((ULONG)DosHeader->e_cparhdr << 4));

            if ((ULONG_PTR)pb < (ULONG_PTR)DosHeader + PAGE_SIZE - 0x30 - sizeof(USHORT)) {
                pb += *(PUSHORT)(pb + 0x30);
                if ((ULONG_PTR)pb < (ULONG_PTR)DosHeader + PAGE_SIZE - 36 &&
                     RtlEqualMemory(pb,
                                    "Copyright (C) Rational Systems, Inc.",
                                    36))
                   {
                    return STATUS_INVALID_IMAGE_PROTECT;
                }
            }

            //
            // Check for lotus 123 family of applications. Starting
            // with 123 3.0 (till recently shipped 123 3.4), every
            // exe header is bound but is meant for DOS. This can
            // be checked via, a string signature in the extended
            // header. <len byte>"1-2-3 Preloader" is the string
            // at ne_nrestab offset.
            //

            pResTableAddress = ((PIMAGE_OS2_HEADER)NtHeader)->ne_nrestab;
            if (pResTableAddress > DosHeader->e_lfanew &&
                ((ULONG)((pResTableAddress+16) - DosHeader->e_lfanew) <
                            NtHeaderSize) &&
                RtlEqualMemory(
                    ((PUCHAR)NtHeader + 1 +
                             (ULONG)(pResTableAddress - DosHeader->e_lfanew)),
                    "1-2-3 Preloader",
                    15)) {
                    return STATUS_INVALID_IMAGE_PROTECT;
            }

            return STATUS_INVALID_IMAGE_NE_FORMAT;
        }

        if ((USHORT)NtHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE) {

            //
            // This is a LE (OS/2) image. We don't support it, so give it to
            // DOS subsystem. There are cases (Rbase.exe) which have a LE
            // header but actually it is suppose to run under DOS. When we
            // do support LE format, some work needs to be done here to
            // decide whether to give it to VDM or OS/2.
            //

            return STATUS_INVALID_IMAGE_PROTECT;
        }
        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if ((NtHeader->FileHeader.Machine == 0) &&
        (NtHeader->FileHeader.SizeOfOptionalHeader == 0)) {

        //
        // This is a bogus DOS app which has a 32-bit portion
        // masquerading as a PE image.
        //

        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if (!(NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }

#ifdef i386

    //
    // Make sure the image header is aligned on a Long word boundary.
    //

    if (((ULONG_PTR)NtHeader & 3) != 0) {
        return STATUS_INVALID_IMAGE_FORMAT;
    }
#endif

#define VALIDATE_NTHEADER(Hdr) {                                    \
    /* File alignment must be multiple of 512 and power of 2. */    \
    if (((((Hdr)->OptionalHeader).FileAlignment & 511) != 0) &&     \
        (((Hdr)->OptionalHeader).FileAlignment !=                   \
         ((Hdr)->OptionalHeader).SectionAlignment)) {               \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).FileAlignment == 0) {               \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    /* Section alignment must be a power of 2 to use MI_ROUND */    \
    if (((((Hdr)->OptionalHeader).SectionAlignment - 1) &           \
          ((Hdr)->OptionalHeader).SectionAlignment) != 0) {         \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((((Hdr)->OptionalHeader).FileAlignment - 1) &              \
          ((Hdr)->OptionalHeader).FileAlignment) != 0) {            \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).SectionAlignment < ((Hdr)->OptionalHeader).FileAlignment) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader).SizeOfImage > MM_SIZE_OF_LARGEST_IMAGE) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if ((Hdr)->FileHeader.NumberOfSections > MM_MAXIMUM_IMAGE_SECTIONS) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) && \
        !((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_I386))  { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
                                                                    \
    if (((Hdr)->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) && \
        !(((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64) || \
          ((Hdr)->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64))) { \
        return STATUS_INVALID_IMAGE_FORMAT;                         \
    }                                                               \
}

    if (NtHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) {

        //
        // Image doesn't have the right magic value in its optional header.
        //

#if defined (_WIN64)
        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

            //
            // PE32 image.  Validate it as such.
            //

            PIMAGE_NT_HEADERS32 NtHeader32 = (PIMAGE_NT_HEADERS32)NtHeader;

            VALIDATE_NTHEADER(NtHeader32);
            return STATUS_SUCCESS;
        }
#else /* !defined(_WIN64) */
        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {

            //
            // 64bit image on a 32bit machine.
            //
            return STATUS_INVALID_IMAGE_WIN_64;
        }
#endif
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    VALIDATE_NTHEADER(NtHeader);
    #undef VALIDATE_NTHEADER

    return STATUS_SUCCESS;
}

NTSTATUS
MiCreateDataFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG IgnoreFileSizing
    )

/*++

Routine Description:

    This function creates the necessary structures to allow the mapping
    of a data file.

    The data file is accessed to verify desired access, a segment
    object is created and initialized.

Arguments:

    File - Supplies the file object for the image file.

    Segment - Returns the segment object.

    MaximumSize - Supplies the maximum size for the mapping.

    SectionPageProtection - Supplies the initial page protection.

    AllocationAttributes - Supplies the allocation attributes for the mapping.

    IgnoreFileSizing - Supplies TRUE if the cache manager is specifying the
                       file size and so it does not need to be validated.

Return Value:

    NTSTATUS.

--*/

{
    NTSTATUS Status;
    ULONG j;
    ULONG Size;
    UINT64 PartialSize;
    PCONTROL_AREA ControlArea;
    PMAPPED_FILE_SEGMENT NewSegment;
    PMSUBSECTION Subsection;
    PMSUBSECTION ExtendedSubsection;
    MMPTE TempPte;
    UINT64 EndOfFile;
    UINT64 LastFileChunk;
    UINT64 FileOffset;
    UINT64 NumberOfPtesForEntireFile;
    ULONG ExtendedSubsections;
    PMSUBSECTION Last;
    ULONG NumberOfNewSubsections;
    SIZE_T AllocationFragment;

    PAGED_CODE();

    ExtendedSubsections = 0;

    // *************************************************************
    // Create mapped file section.
    // *************************************************************

    if (!IgnoreFileSizing) {

        Status = FsRtlGetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

        if (Status == STATUS_FILE_IS_A_DIRECTORY) {

            //
            // Can't map a directory as a section. Return error.
            //

            return STATUS_INVALID_FILE_FOR_SECTION;
        }

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        if (EndOfFile == 0 && *MaximumSize == 0) {

            //
            // Can't map a zero length without specifying the maximum
            // size as non-zero.
            //

            return STATUS_MAPPED_FILE_SIZE_ZERO;
        }

        //
        // Make sure this file is big enough for the section.
        //

        if (*MaximumSize > EndOfFile) {

            //
            // If the maximum size is greater than the end-of-file,
            // and the user did not request page_write or page_execute_readwrite
            // to the section, reject the request.
            //

            if (((SectionPageProtection & PAGE_READWRITE) |
                (SectionPageProtection & PAGE_EXECUTE_READWRITE)) == 0) {

                return STATUS_SECTION_TOO_BIG;
            }

            //
            // Check to make sure that the allocation size large enough
            // to contain all the data, if not set a new allocation size.
            //

            EndOfFile = *MaximumSize;

            Status = FsRtlSetFileSize (File, (PLARGE_INTEGER)&EndOfFile);

            if (!NT_SUCCESS (Status)) {
                return Status;
            }
        }
    }
    else {

        //
        // Ignore the file size, this call is from the cache manager.
        //

        EndOfFile = *MaximumSize;
    }

    //
    // Calculate the number of prototype PTE chunks to build for this section.
    // A subsection is also allocated for each chunk as all the prototype PTEs
    // in any given chunk are initially encoded to point at the same subsection.
    //
    // The maximum total section size is 16PB (2^54).  This is because the
    // StartingSector4132 field in each subsection, ie: 2^42-1 bits of file
    // offset where the offset is in 4K (not pagesize) units.  Thus, a
    // subsection may describe a *BYTE* file start offset of maximum
    // 2^54 - 4K.
    //
    // Each subsection can span at most 16TB - 64K.  This is because the
    // NumberOfFullSectors and various other fields in the subsection are
    // ULONGs.  In reality, this is a nonissue as far as maximum section size
    // is concerned because any number of subsections can be chained together
    // and in fact, subsections are allocated to span less to facilitate
    // efficient dynamic prototype PTE trimming and reconstruction.
    //

    if (EndOfFile > MI_MAXIMUM_SECTION_SIZE) {
        return STATUS_SECTION_TOO_BIG;
    }

    NumberOfPtesForEntireFile = (EndOfFile + PAGE_SIZE - 1) >> PAGE_SHIFT;

    NewSegment = ExAllocatePoolWithTag (PagedPool,
                                        sizeof(MAPPED_FILE_SEGMENT),
                                        'mSmM');

    if (NewSegment == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Allocate the subsection memory in smaller sizes so the corresponding
    // prototype PTEs can be trimmed later if paged pool virtual address
    // space becomes scarce.  Note the size is snapped locally so it can
    // be changed dynamically without locking.
    //

    AllocationFragment = MmAllocationFragment;

    ASSERT (MiGetByteOffset (AllocationFragment) == 0);
    ASSERT (AllocationFragment >= PAGE_SIZE);
    ASSERT64 (AllocationFragment < _4gb);

    Size = (ULONG) AllocationFragment;
    PartialSize = NumberOfPtesForEntireFile * sizeof(MMPTE);

    NumberOfNewSubsections = 0;
    ExtendedSubsection = NULL;

    //
    // Initializing Last is not needed for correctness, but without it the
    // compiler cannot compile this code W4 to check for use of uninitialized
    // variables.
    //

    Last = NULL;

    ControlArea = (PCONTROL_AREA)File->SectionObjectPointer->DataSectionObject;

    do {

        if (PartialSize < (UINT64) AllocationFragment) {
            PartialSize = (UINT64) ROUND_TO_PAGES (PartialSize);
            Size = (ULONG) PartialSize;
        }

        if (ExtendedSubsection == NULL) {
            ExtendedSubsection = (PMSUBSECTION)(ControlArea + 1);

            //
            // Control area and first subsection were zeroed at allocation time.
            //
        }
        else {

            ExtendedSubsection = ExAllocatePoolWithTag (NonPagedPool,
                                                        sizeof(MSUBSECTION),
                                                        'cSmM');

            if (ExtendedSubsection == NULL) {
                ExFreePool (NewSegment);

                //
                // Free all the previous allocations and return an error.
                //

                ExtendedSubsection = (PMSUBSECTION)(ControlArea + 1);
                ExtendedSubsection = (PMSUBSECTION) ExtendedSubsection->NextSubsection;
                while (ExtendedSubsection != NULL) {
                    Last = (PMSUBSECTION) ExtendedSubsection->NextSubsection;
                    ExFreePool (ExtendedSubsection);
                    ExtendedSubsection = Last;
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlZeroMemory (ExtendedSubsection, sizeof(MSUBSECTION));
            Last->NextSubsection = (PSUBSECTION) ExtendedSubsection;
        }

        NumberOfNewSubsections += 1;

        ExtendedSubsection->PtesInSubsection = Size / sizeof(MMPTE);

        Last = ExtendedSubsection;
        PartialSize -= Size;
    } while (PartialSize != 0);

    *Segment = (PSEGMENT) NewSegment;
    RtlZeroMemory (NewSegment, sizeof(MAPPED_FILE_SEGMENT));

    NewSegment->LastSubsectionHint = ExtendedSubsection;

    //
    // Control area and first subsection were zeroed at allocation time.
    //

    ControlArea->Segment = (PSEGMENT) NewSegment;
    ControlArea->NumberOfSectionReferences = 1;

    if (IgnoreFileSizing == FALSE) {

        //
        // This reference is not from the cache manager.
        //

        ControlArea->NumberOfUserReferences = 1;
    }
    else {

        //
        // Set the was purged flag to indicate that the
        // file size was not explicitly set.
        //

        ControlArea->u.Flags.WasPurged = 1;
    }

    ControlArea->u.Flags.BeingCreated = 1;
    ControlArea->u.Flags.File = 1;

    if (FILE_REMOTE_DEVICE & File->DeviceObject->Characteristics) {

        //
        // This file resides on a redirected drive.
        //

        ControlArea->u.Flags.Networked = 1;
    }

    if (AllocationAttributes & SEC_NOCACHE) {
        ControlArea->u.Flags.NoCache = 1;
    }

    ControlArea->FilePointer = File;

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    Subsection = (PMSUBSECTION)(ControlArea + 1);

    //
    // Loop through all the subsections and fill in the PTEs.
    //

    TempPte.u.Long = MiGetSubsectionAddressForPte (Subsection);
    TempPte.u.Soft.Prototype = 1;

    //
    // Set all the PTEs to the execute-read-write protection.
    // The section will control access to these and the segment
    // must provide a method to allow other users to map the file
    // for various protections.
    //

    TempPte.u.Soft.Protection = MM_EXECUTE_READWRITE;

    NewSegment->ControlArea = ControlArea;
    NewSegment->SizeOfSegment = EndOfFile;
    NewSegment->TotalNumberOfPtes = (ULONG) NumberOfPtesForEntireFile;
    if (NumberOfPtesForEntireFile >= 0x100000000) {
        NewSegment->SegmentFlags.TotalNumberOfPtes4132 = (ULONG_PTR)(NumberOfPtesForEntireFile >> 32);
    }

    NewSegment->SegmentPteTemplate = TempPte;

    if (Subsection->NextSubsection != NULL) {

        //
        // Multiple segments and subsections.
        // Align first so it is a multiple of the allocation size.
        //

        NewSegment->NonExtendedPtes =
          (Subsection->PtesInSubsection & ~(((ULONG)AllocationFragment >> PAGE_SHIFT) - 1));
    }
    else {
        NewSegment->NonExtendedPtes = NewSegment->TotalNumberOfPtes;
    }

    Subsection->PtesInSubsection = NewSegment->NonExtendedPtes;

    FileOffset = 0;

    do {

        //
        // Loop through all the subsections to initialize them.
        //

        Subsection->ControlArea = ControlArea;

        Mi4KStartForSubsection(&FileOffset, Subsection);

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_READWRITE;

        if (Subsection->NextSubsection == NULL) {

            LastFileChunk = (EndOfFile >> MM4K_SHIFT) - FileOffset;

            //
            // Note this next line restricts the number of bytes mapped by
            // a single subsection to 16TB-4K.  Multiple subsections can always
            // be chained together to support an overall file of size 16K TB.
            //

            Subsection->NumberOfFullSectors = (ULONG)LastFileChunk;

            Subsection->u.SubsectionFlags.SectorEndOffset =
                                 (ULONG) EndOfFile & MM4K_MASK;

            j = Subsection->PtesInSubsection;

            Subsection->PtesInSubsection = (ULONG)(
                NumberOfPtesForEntireFile -
                                (FileOffset >> (PAGE_SHIFT - MM4K_SHIFT)));

            MI_CHECK_SUBSECTION (Subsection);

            Subsection->UnusedPtes = j - Subsection->PtesInSubsection;
        }
        else {
            Subsection->NumberOfFullSectors =
                Subsection->PtesInSubsection << (PAGE_SHIFT - MM4K_SHIFT);

            MI_CHECK_SUBSECTION (Subsection);
        }

        FileOffset += (((UINT64)Subsection->PtesInSubsection) <<
                                        (PAGE_SHIFT - MM4K_SHIFT));
        Subsection = (PMSUBSECTION) Subsection->NextSubsection;
    } while (Subsection != NULL);

    return STATUS_SUCCESS;
}

VOID
MiUpdateLargeProtoPfns (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN PMMPFN Pfn1,
    IN MMPTE OriginalPte
    )
{
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PMMPTE PteFramePointer;
    PFN_NUMBER PdeFrame;

    PteFramePointer = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS < 3)
    if (PteFramePointer->u.Hard.Valid == 0) {
        MiCheckPdeForPagedPool (PointerPte);
    }
#endif
    LOCK_PFN (OldIrql);

    if (PteFramePointer->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
    }

    ASSERT (PteFramePointer->u.Hard.Valid == 1);
    PdeFrame = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    Pfn2 = MI_PFN_ELEMENT (PdeFrame);

    while (PointerPte < LastPte) {

        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
        ASSERT (Pfn1->u4.AweAllocation == 1);

        Pfn1->u3.e1.PrototypePte = 1;
        Pfn1->PteAddress = PointerPte;
        Pfn1->u4.PteFrame = PdeFrame;
        Pfn1->OriginalPte = OriginalPte;

        //
        // Also update the share count on the page containing the
        // prototype PTEs.  This prevents the page from getting trimmed
        // which is good because we wouldn't know to update the PteFrame
        // of our physical pages then anyway !
        //

        Pfn2->u2.ShareCount += 1;

        PointerPte += 1;
        if (MiIsPteOnPdeBoundary (PointerPte)) {

            if (PointerPte == LastPte) {
                break;
            }

            PteFramePointer = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS < 3)
            if (PteFramePointer->u.Hard.Valid == 0) {
                MiCheckPdeForPagedPool (PointerPte);
            }
#endif
            if (PteFramePointer->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
            }

            ASSERT (PteFramePointer->u.Hard.Valid == 1);

            PdeFrame = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
            Pfn2 = MI_PFN_ELEMENT (PdeFrame);
        }
        Pfn1 += 1;
    }
    UNLOCK_PFN (OldIrql);

    return;
}

NTSTATUS
MiCreatePagingFileMap (
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN MM_PROTECTION_MASK ProtectionMask,
    IN ULONG AllocationAttributes
    )

/*++

Routine Description:

    This function creates the necessary structures to allow the mapping
    of a paging file.

Arguments:

    Segment - Returns the segment object.

    MaximumSize - Supplies the maximum size for the mapping.

    ProtectionMask - Supplies the initial page protection.

    AllocationAttributes - Supplies the allocation attributes for the mapping.

Return Value:

    NTSTATUS.

--*/

{
    PMMPFN Pfn1;
    UINT64 MaximumFileSize;
    PFN_NUMBER NumberOfPtes;
    PFN_NUMBER PageFrameIndex;
    SIZE_T SizeOfSegment;
    PCONTROL_AREA ControlArea;
    PSEGMENT NewSegment;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PSUBSECTION Subsection;
    MMPTE TempPte;
    MMPTE TempValidPte;
    PMI_LARGEPAGE_MEMORY_RUN LargePageInfo;
    PMI_LARGEPAGE_MEMORY_RUN LargePageListHead;

    PAGED_CODE();

    //*******************************************************************
    // Create a section backed by the paging file.
    //*******************************************************************

    if (*MaximumSize == 0) {
        return STATUS_INVALID_PARAMETER_4;
    }

    //
    // Limit page file backed sections to the amount of pool that could
    // possibly be allocated to hold the prototype PTEs.  Note this may
    // be larger than the size of any *single* pagefile.
    //

#if defined (_WIN64)

    //
    // Limit the maximum size to the number of PTEs that can be stored in
    // the NonExtendedPtes field in the segment so that segment deletion
    // which uses this will use the proper value.
    //

    MaximumFileSize = ((UINT64)1 << (32 + PAGE_SHIFT)) - sizeof (ULONG_PTR) - sizeof(SEGMENT);
#else
    MaximumFileSize = MAXULONG_PTR - sizeof(SEGMENT);
#endif

    MaximumFileSize /= sizeof(MMPTE);
    MaximumFileSize <<= PAGE_SHIFT;

    if (*MaximumSize > MaximumFileSize) {
        return STATUS_SECTION_TOO_BIG;
    }

    //
    // Create the segment object.
    //
    // Calculate the number of prototype PTEs to build for this segment.
    //

    LargePageListHead = NULL;
    NumberOfPtes = (PFN_NUMBER) ((*MaximumSize + PAGE_SIZE - 1) >> PAGE_SHIFT);

    if (AllocationAttributes & SEC_COMMIT) {

        //
        // Commit the pages for the section.
        //

        ASSERT (ProtectionMask != 0);

        if (MiChargeCommitment (NumberOfPtes, NULL) == FALSE) {
            return STATUS_COMMITMENT_LIMIT;
        }

        if (AllocationAttributes & SEC_LARGE_PAGES) {

#ifdef _X86_
            if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
                return STATUS_NOT_SUPPORTED;
            }
#endif
            //
            // Ensure the requested size is a large page multiple.
            //

            if (NumberOfPtes % (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) {
                MiReturnCommitment (NumberOfPtes);
                return STATUS_INVALID_PARAMETER_4;
            }

            LargePageListHead = MiAllocateLargeZeroPages (NumberOfPtes,
                                                          ProtectionMask);

            if (LargePageListHead == NULL) {
                MiReturnCommitment (NumberOfPtes);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    SizeOfSegment = sizeof(SEGMENT) + sizeof(MMPTE) * (NumberOfPtes - 1);

    NewSegment = ExAllocatePoolWithTag (PagedPool | POOL_MM_ALLOCATION,
                                        SizeOfSegment,
                                        MMSECT);

    if (NewSegment == NULL) {

        //
        // The requested pool could not be allocated.
        //

        if (AllocationAttributes & SEC_COMMIT) {
            if (LargePageListHead != NULL) {
                MiReturnLargePages (LargePageListHead);
            }
            MiReturnCommitment (NumberOfPtes);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *Segment = NewSegment;

    ControlArea = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof(CONTROL_AREA) + sizeof(SUBSECTION),
                                         MMCONTROL);

    if (ControlArea == NULL) {

        //
        // The requested pool could not be allocated.
        //

        ExFreePool (NewSegment);

        if (AllocationAttributes & SEC_COMMIT) {
            if (AllocationAttributes & SEC_LARGE_PAGES) {
                if (LargePageListHead != NULL) {
                    MiReturnLargePages (LargePageListHead);
                }
            }
            MiReturnCommitment (NumberOfPtes);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Zero control area and first subsection.
    //

    RtlZeroMemory (ControlArea, sizeof(CONTROL_AREA) + sizeof(SUBSECTION));

    ControlArea->Segment = NewSegment;
    ControlArea->NumberOfSectionReferences = 1;
    ControlArea->NumberOfUserReferences = 1;

    if (AllocationAttributes & SEC_BASED) {
        ControlArea->u.Flags.Based = 1;
    }

    if (AllocationAttributes & SEC_RESERVE) {
        ControlArea->u.Flags.Reserve = 1;
    }

    if (AllocationAttributes & SEC_COMMIT) {
        ControlArea->u.Flags.Commit = 1;
    }

    Subsection = (PSUBSECTION)(ControlArea + 1);

    Subsection->ControlArea = ControlArea;
    Subsection->PtesInSubsection = (ULONG)NumberOfPtes;
    Subsection->u.SubsectionFlags.Protection = ProtectionMask;

    //
    // Align the prototype PTEs on the proper boundary.
    //

    PointerPte = &NewSegment->ThePtes[0];

    //
    // Zero the segment header.
    //

    RtlZeroMemory (NewSegment, sizeof(SEGMENT));

    NewSegment->PrototypePte = PointerPte;

    NewSegment->ControlArea = ControlArea;

    //
    // Record the process that created this segment for the performance
    // analysis tools.
    //

    NewSegment->u1.CreatingProcess = PsGetCurrentProcess ();

    NewSegment->SizeOfSegment = (UINT64)NumberOfPtes * PAGE_SIZE;
    NewSegment->TotalNumberOfPtes = (ULONG)NumberOfPtes;
    NewSegment->NonExtendedPtes = (ULONG)NumberOfPtes;

    Subsection->SubsectionBase = PointerPte;
    TempPte.u.Long = 0;

    if (AllocationAttributes & SEC_COMMIT) {
        TempPte.u.Soft.Protection = ProtectionMask;

        //
        // Record commitment charging.
        //

        MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGEFILE_BACKED_SHMEM, NumberOfPtes);

        NewSegment->NumberOfCommittedPages = NumberOfPtes;

        InterlockedExchangeAddSizeT (&MmSharedCommit, NumberOfPtes);

        if (AllocationAttributes & SEC_LARGE_PAGES) {

            NewSegment->SegmentFlags.LargePages = 1;

            //
            // Fill all the prototype PTEs with valid entries.  Note this is
            // unusual for prototype PTEs because these entries are valid
            // with a nonzero share and and reference count even though no
            // real page table entries are currently mapping them.  This is
            // to prevent premature trimming of them.
            //

            MI_MAKE_VALID_PTE (TempValidPte,
                               0,
                               ProtectionMask,
                               NULL);

            if (ProtectionMask & MM_PROTECTION_WRITE_MASK) {
                MI_SET_PTE_DIRTY (TempValidPte);
            }

            while (LargePageListHead != NULL) {

                LargePageInfo = LargePageListHead;
                LargePageListHead = LargePageListHead->Next;

                PageFrameIndex = LargePageInfo->BasePage;
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (LargePageInfo->PageCount != 0);
                LastPte = PointerPte + LargePageInfo->PageCount;

                do {
                    TempValidPte.u.Hard.PageFrameNumber = PageFrameIndex;
                    PageFrameIndex += 1;
                    Pfn1 += 1;
                    *PointerPte = TempValidPte;
                    PointerPte += 1;
                } while (PointerPte < LastPte);

                MiUpdateLargeProtoPfns (PointerPte - LargePageInfo->PageCount,
                                        LastPte,
                                        Pfn1 - LargePageInfo->PageCount,
                                        TempPte);

                ExFreePool (LargePageInfo);
            }
        }
    }

    NewSegment->SegmentPteTemplate.u.Soft.Protection = ProtectionMask;

    //
    // Set all the prototype PTEs to either no access or demand zero
    // depending on the commit flag.
    //

    if ((AllocationAttributes & SEC_LARGE_PAGES) == 0) {
        MiFillMemoryPte (PointerPte, NumberOfPtes, TempPte.u.Long);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NtOpenSection(
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    )

/*++

Routine Description:

    This function opens a handle to a section object with the specified
    desired access.

Arguments:


    Sectionhandle - Supplies a pointer to a variable that will
                    receive the section object handle value.

    DesiredAccess - Supplies the desired types of access  for the
                    section.

    DesiredAccess Flags

         EXECUTE - Execute access to the section is desired.

         READ - Read access to the section is desired.

         WRITE - Write access to the section is desired.

    ObjectAttributes - Supplies a pointer to an object attributes structure.

Return Value:

    NTSTATUS.

--*/

{
    HANDLE Handle;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();
    //
    // Get previous processor mode and probe output arguments if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForWriteHandle(SectionHandle);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Open handle to the section object with the specified desired
    // access.
    //

    Status = ObOpenObjectByName (ObjectAttributes,
                                 MmSectionObjectType,
                                 PreviousMode,
                                 NULL,
                                 DesiredAccess,
                                 NULL,
                                 &Handle);

    try {
        *SectionHandle = Handle;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return Status;
    }

    return Status;
}

CCHAR
MiGetImageProtection (
    IN ULONG SectionCharacteristics
    )

/*++

Routine Description:

    This function takes a section characteristic mask from the
    image and converts it to an PTE protection mask.

Arguments:

    SectionCharacteristics - Supplies the characteristics mask from the
                             image.

Return Value:

    Returns the protection mask for the PTE.

--*/

{
    ULONG Index;

    PAGED_CODE();

    Index = 0;
    if (SectionCharacteristics & IMAGE_SCN_MEM_EXECUTE) {
        Index |= 1;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_READ) {
        Index |= 2;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_WRITE) {
        Index |= 4;
    }
    if (SectionCharacteristics & IMAGE_SCN_MEM_SHARED) {
        Index |= 8;
    }

    return MmImageProtectionArray[Index];
}

PFN_NUMBER
MiGetPageForHeader (
    LOGICAL ZeroPage
    )

/*++

Routine Description:

    This non-pageable function acquires the PFN lock, removes
    a page and updates the PFN database as though the page was
    ready to be deleted if the reference count is decremented.

Arguments:

    ZeroPage - Supplies TRUE if the caller requires a zero-filled page.

Return Value:

    Returns the physical page frame number.

--*/

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameNumber;
    PMMPFN Pfn1;
    PEPROCESS Process;
    ULONG PageColor;
    LOGICAL TbFlushNeeded;

    TbFlushNeeded = FALSE;
    Process = PsGetCurrentProcess ();

    PageColor = MI_PAGE_COLOR_VA_PROCESS ((PVOID)X64K,
                                          &Process->NextPageColor);

    //
    // Lock the PFN database and get a page.
    //

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    //
    // Remove page for 64k alignment.
    //

    if (ZeroPage) {
        PageFrameNumber = MiRemoveZeroPage (PageColor);
    }
    else {
        PageFrameNumber = MiRemoveAnyPage (PageColor);
    }

    //
    // Increment the reference count for the page so the
    // paging I/O will work, and so this page cannot be stolen from us.
    //

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);
    Pfn1->u3.e2.ReferenceCount += 1;

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        TbFlushNeeded = TRUE;
        Pfn1->u3.e1.CacheAttribute = MiCached;
    }

    //
    // Don't need the PFN lock for the fields below...
    //

    UNLOCK_PFN (OldIrql);

    ASSERT (Pfn1->u1.Flink == 0);
    ASSERT (Pfn1->u2.Blink == 0);

    Pfn1->OriginalPte.u.Long = 0;
    Pfn1->PteAddress = (PVOID) (ULONG_PTR)X64K;
    MI_SET_PFN_DELETED (Pfn1);

    if (TbFlushNeeded == TRUE) {
        MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
    }

    return PageFrameNumber;
}

VOID
MiUpdateImageHeaderPage (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER PageFrameNumber,
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL MarkModified
    )

/*++

Routine Description:

    This non-pageable function acquires the PFN lock, and
    turns the specified prototype PTE into a transition PTE
    referring to the specified physical page.  It then
    decrements the reference count causing the page to
    be placed on the standby or modified list.

Arguments:

    PointerPte - Supplies the PTE to set into the transition state.

    PageFrameNumber - Supplies the physical page.

    ControlArea - Supplies the control area for the prototype PTEs.

    MarkModified - Supplies TRUE if the PFN should be marked modified (this
                   will give it pagefile-backing on trim vs filesystem backing).

Return Value:

    None.

--*/

{
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    KIRQL OldIrql;

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    PointerPde = MiGetPteAddress (PointerPte);
    LOCK_PFN (OldIrql);

    if (PointerPde->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
    }

    MiInitializeTransitionPfn (PageFrameNumber, PointerPte);

    if (MarkModified == TRUE) {
        MI_SET_MODIFIED (Pfn1, 1, 0x19);
    }

    if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {
        ControlArea->NumberOfPfnReferences += 1;
    }

    //
    // Add the page to the standby list.
    //

    MiDecrementReferenceCount (Pfn1, PageFrameNumber);

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiRemoveImageHeaderPage (
    IN PFN_NUMBER PageFrameNumber
    )

/*++

Routine Description:

    This non-pageable function acquires the PFN lock, and decrements
    the reference count thereby causing the physical page to
    be deleted.

Arguments:

    PageFrameNumber - Supplies the PFN to decrement.

Return Value:

    None.

--*/
{
    PMMPFN Pfn1;
    KIRQL OldIrql;

    Pfn1 = MI_PFN_ELEMENT (PageFrameNumber);

    LOCK_PFN (OldIrql);
    MiDecrementReferenceCount (Pfn1, PageFrameNumber);
    UNLOCK_PFN (OldIrql);
    return;
}

PCONTROL_AREA
MiFindImageSectionObject (
    IN PFILE_OBJECT File,
    IN LOGICAL PfnLockHeld,
    OUT PLOGICAL GlobalNeeded
    )

/*++

Routine Description:

    This function searches the control area chains (if any) for an existing
    cache of the specified image file.  For non-global control areas, there is
    no chain and the control area is shared for all callers and sessions.
    Likewise for systemwide global control areas.

    However, for global PER-SESSION control areas, we must do the walk.

Arguments:

    File - Supplies the file object for the image file.

    PfnLockHeld - Indicates whether the PFN database is locked upon entering this routine.

    GlobalNeeded - Supplies a pointer to store whether a global control area is
                   required as a placeholder.  This can only be set when there
                   is already some global control area in the list - ie: our
                   caller should only rely on this when this function returns
                   NULL so the caller knows what kind of control area to
                   insert.

Return Value:

    Returns the matching control area or NULL if one does not exist.

Environment:

    Must be holding the PFN lock.

--*/

{
    PCONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA LargeControlArea;
    PLIST_ENTRY Head, Next;
    ULONG SessionId;
    KIRQL OldIrql;
    
    *GlobalNeeded = FALSE;

    if (PfnLockHeld == FALSE) {
        LOCK_PFN(OldIrql);
    } else {
        SATISFY_OVERZEALOUS_COMPILER(OldIrql = PASSIVE_LEVEL);
        MM_PFN_LOCK_ASSERT();
    }

    //
    // Get first (if any) control area pointer.
    //

    ControlArea = (PCONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

    //
    // If no control area, or the control area is not session global,
    // then our job is easy.  Note, however, that they each require different
    // return values as they represent different states.
    //

    if ((ControlArea == NULL) || (ControlArea->u.Flags.GlobalOnlyPerSession == 0)) {
        goto ControlAreaFound;
    }

    LargeControlArea = (PLARGE_CONTROL_AREA) ControlArea;

    //
    // Get the current session ID and search for a matching control area.
    //

    SessionId = MmGetSessionId (PsGetCurrentProcess());

    if (LargeControlArea->SessionId == SessionId) {
        // ControlArea and LargeControlArea are the same.
        goto ControlAreaFound;
    }

    //
    // Must search the control area list for a matching session ID.
    //

    Head = &LargeControlArea->UserGlobalList;
    
    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {

        LargeControlArea = CONTAINING_RECORD (Next, LARGE_CONTROL_AREA, UserGlobalList);

        ASSERT (LargeControlArea->u.Flags.GlobalOnlyPerSession == 1);

        if (LargeControlArea->SessionId == SessionId) {
            ControlArea = (PCONTROL_AREA) LargeControlArea;
            goto ControlAreaFound;
        }
    }
    //
    // No match, so tell our caller to create a new global control area.
    //  
    ControlArea = NULL;
    *GlobalNeeded = TRUE;
    
ControlAreaFound:
    if (PfnLockHeld == FALSE) {
        UNLOCK_PFN(OldIrql);
    }
    
    return ControlArea;
}

VOID
MiInsertImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA InputControlArea
    )

/*++

Routine Description:

    This function inserts the control area into the file's section object
    pointers.  For non-global control areas, there is no chain and the
    control area is shared for all callers and sessions.
    Likewise for systemwide global control areas.

    However, for global PER-SESSION control areas, we must do a list insertion.

Arguments:

    File - Supplies the file object for the image file.

    InputControlArea - Supplies the control area to insert.

Return Value:

    None.

Environment:

    Must be holding the PFN lock.

--*/

{
    PLIST_ENTRY Head;
    PLARGE_CONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA FirstControlArea;
#if DBG
    PLIST_ENTRY Next;
    PLARGE_CONTROL_AREA NextControlArea;
#endif

    MM_PFN_LOCK_ASSERT();

    ControlArea = (PLARGE_CONTROL_AREA) InputControlArea;

    //
    // If this is not a global-per-session control area or just a placeholder
    // control area (with no chain already in place) then just put it in.
    //

    FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

    if (FirstControlArea == NULL) {
        if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
            File->SectionObjectPointer->ImageSectionObject = (PVOID)ControlArea;
            return;
        }
    }

    //
    // A per-session control area needs to be inserted...
    //

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 1);

    ControlArea->SessionId = MmGetSessionId (PsGetCurrentProcess());

    //
    // If the control area list is empty, just initialize links for this entry.
    //

    if (File->SectionObjectPointer->ImageSectionObject == NULL) {
        InitializeListHead (&ControlArea->UserGlobalList);
    }
    else {

        //
        // Insert new entry before the current first entry.  The control area
        // must be in the midst of creation/deletion or have a valid session
        // ID to be inserted.
        //

        ASSERT (ControlArea->u.Flags.BeingDeleted ||
                ControlArea->u.Flags.BeingCreated ||
                ControlArea->SessionId != (ULONG)-1);

        FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

        Head = &FirstControlArea->UserGlobalList;

#if DBG
        //
        // Ensure no duplicate session IDs exist in the list.
        //

        for (Next = Head->Flink; Next != Head; Next = Next->Flink) {
            NextControlArea = CONTAINING_RECORD (Next, LARGE_CONTROL_AREA, UserGlobalList);
            ASSERT (NextControlArea->SessionId != (ULONG)-1 &&
                    NextControlArea->SessionId != ControlArea->SessionId);
        }
#endif

        InsertTailList (Head, &ControlArea->UserGlobalList);
    }

    //
    // Update first control area pointer.
    //

    File->SectionObjectPointer->ImageSectionObject = (PVOID) ControlArea;
}

VOID
MiRemoveImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA InputControlArea
    )

/*++

Routine Description:

    This function searches the control area chains (if any) for an existing
    cache of the specified image file.  For non-global control areas, there is
    no chain and the control area is shared for all callers and sessions.
    Likewise for systemwide global control areas.

    However, for global PER-SESSION control areas, we must do the walk.

    Upon finding the specified control area, we unlink it.

Arguments:

    File - Supplies the file object for the image file.

    InputControlArea - Supplies the control area to remove.

Return Value:

    None.

Environment:

    Must be holding the PFN lock.

--*/

{
#if DBG
    PLIST_ENTRY Head;
#endif
    PLIST_ENTRY Next;
    PLARGE_CONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA FirstControlArea;
    PLARGE_CONTROL_AREA NextControlArea;

    MM_PFN_LOCK_ASSERT();

    ControlArea = (PLARGE_CONTROL_AREA) InputControlArea;

    FirstControlArea = (PLARGE_CONTROL_AREA)(File->SectionObjectPointer->ImageSectionObject);

    //
    // Get a pointer to the first control area.  If this is not a
    // global-per-session control area, then there is no list, so we're done.
    //

    if (FirstControlArea->u.Flags.GlobalOnlyPerSession == 0) {
        ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

        File->SectionObjectPointer->ImageSectionObject = NULL;
        return;
    }

    //
    // A list may exist.  Walk it as necessary and delete the requested entry.
    //

    if (FirstControlArea == ControlArea) {

        //
        // The first entry is the one to remove.  If it is the only entry
        // in the list, then the new first entry pointer will be NULL.
        // Otherwise, get a pointer to the next entry and unlink the current.
        //

        if (IsListEmpty (&FirstControlArea->UserGlobalList)) {
            NextControlArea = NULL;
        }
        else {
            Next = FirstControlArea->UserGlobalList.Flink;
            RemoveEntryList (&FirstControlArea->UserGlobalList);
            NextControlArea = CONTAINING_RECORD (Next,
                                                 LARGE_CONTROL_AREA,
                                                 UserGlobalList);

            ASSERT (NextControlArea->u.Flags.GlobalOnlyPerSession == 1);
        }

        File->SectionObjectPointer->ImageSectionObject = (PVOID)NextControlArea;
        return;
    }

    //
    // Remove the entry, note that the ImageSectionObject need not be updated
    // as the entry is not at the head.
    //

#if DBG
    Head = &FirstControlArea->UserGlobalList;

    for (Next = Head->Flink; Next != Head; Next = Next->Flink) {

        NextControlArea = CONTAINING_RECORD (Next,
                                             LARGE_CONTROL_AREA,
                                             UserGlobalList);

        ASSERT (NextControlArea->u.Flags.GlobalOnlyPerSession == 1);

        if (NextControlArea == ControlArea) {
            break;
        }
    }
    ASSERT (Next != Head);
#endif

    RemoveEntryList (&ControlArea->UserGlobalList);
}

LOGICAL
MiFlushDataSection (
    IN PFILE_OBJECT File
    )

/*++

Routine Description:

    This routine flushes the data section if there is one.

Arguments:

    File - Supplies the file object.

Return Value:

    TRUE if there is a data section that may be in use, FALSE if not.

Environment:

    Kernel mode, APC_LEVEL and below.

--*/

{
    KIRQL OldIrql;
    IO_STATUS_BLOCK IoStatus;
    PCONTROL_AREA ControlArea;
    LOGICAL DataInUse;

    DataInUse = FALSE;

    LOCK_PFN (OldIrql);

    ControlArea = (PCONTROL_AREA) File->SectionObjectPointer->DataSectionObject;

    if (ControlArea) {

        if ((ControlArea->NumberOfSectionReferences != 0) ||
            (ControlArea->NumberOfMappedViews != 0)) {

            DataInUse = TRUE;
        }

        if (ControlArea->NumberOfSystemCacheViews) {
            UNLOCK_PFN (OldIrql);
            CcFlushCache (File->SectionObjectPointer,
                          NULL,
                          0,
                          &IoStatus);

        }
        else {
            UNLOCK_PFN (OldIrql);
            MmFlushSection (File->SectionObjectPointer,
                            NULL,
                            0,
                            &IoStatus,
                            TRUE);
        }
    }
    else {
        UNLOCK_PFN (OldIrql);
    }

    return DataInUse;
}


PVOID
MiCopyHeaderIfResident (
    IN PFILE_OBJECT File,
    IN PFN_NUMBER ImagePageFrameNumber
    )

/*++

Routine Description:

    This routine copies the image header from the data section if there is
    one and the page is already resident or in transition.

Arguments:

    File - Supplies the file object.

    ImagePageFrameNumber - Supplies the image frame to copy the data into.

Return Value:

    Virtual address of the image page frame number if successful, NULL if not.

Environment:

    Kernel mode, APC_LEVEL and below.

--*/

{
    PMMPFN Pfn1;
    PVOID DataPage;
    PVOID ImagePage;
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PMMPTE ImagePte;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;
    PEPROCESS Process;
    PSUBSECTION Subsection;
    PSECTION_OBJECT_POINTERS SectionObjectPointer;

    //
    // Take a quick (safely unsynchronized) look to see whether to bother
    // mapping the image header page at all - if there's no data section
    // object, then skip it and just return.
    //

    SectionObjectPointer = File->SectionObjectPointer;
    if (SectionObjectPointer == NULL) {
        return NULL;
    }

    ControlArea = (PCONTROL_AREA) SectionObjectPointer->DataSectionObject;

    if (ControlArea == NULL) {
        return NULL;
    }

    //
    // There's a data section, so map the target page.
    //

    ImagePte = MiReserveSystemPtes (1, SystemPteSpace);

    if (ImagePte == NULL) {
        return NULL;
    }

    ImagePage = MiGetVirtualAddressMappedByPte (ImagePte);

    ASSERT (ImagePte->u.Hard.Valid == 0);

    PteContents = ValidKernelPte;
    PteContents.u.Hard.PageFrameNumber = ImagePageFrameNumber;

    MI_WRITE_VALID_PTE (ImagePte, PteContents);

    LOCK_PFN (OldIrql);

    //
    // Now that we are synchronized via the PFN lock, take a safe look.
    //

    SectionObjectPointer = File->SectionObjectPointer;
    if (SectionObjectPointer == NULL) {
        UNLOCK_PFN (OldIrql);
        if (ImagePte != NULL) {
            MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
        }
        return NULL;
    }

    ControlArea = (PCONTROL_AREA) SectionObjectPointer->DataSectionObject;

    if (ControlArea == NULL) {
        UNLOCK_PFN (OldIrql);
        if (ImagePte != NULL) {
            MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
        }
        return NULL;
    }

    if ((ControlArea->u.Flags.BeingCreated) ||
        (ControlArea->u.Flags.BeingDeleted)) {

        UNLOCK_PFN (OldIrql);
        if (ImagePte != NULL) {
            MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
        }
        return NULL;
    }

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION) (ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    //
    // If the prototype PTEs have been tossed (or never created) then we
    // don't have any data to copy.
    //

    PointerPte = Subsection->SubsectionBase;

    if (PointerPte == NULL) {
        UNLOCK_PFN (OldIrql);
        if (ImagePte != NULL) {
            MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
        }
        return NULL;
    }

    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {

        //
        // We have no reference to the data section so if we can't do this
        // without relinquishing the PFN lock, then don't bother.
        // ie: the entire control area and everything can be freed
        // while a call to MiMakeSystemAddressValidPfn releases the lock.
        //

        UNLOCK_PFN (OldIrql);
        if (ImagePte != NULL) {
            MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
        }
        return NULL;
    }

    PteContents = *PointerPte;

    if ((PteContents.u.Hard.Valid == 1) ||
       ((PteContents.u.Soft.Prototype == 0) &&
         (PteContents.u.Soft.Transition == 1))) {

        if (PteContents.u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        }
        else {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if (Pfn1->u3.e1.ReadInProgress != 0) {
                UNLOCK_PFN (OldIrql);
                if (ImagePte != NULL) {
                    MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
                }
                return NULL;
            }
        }

        Process = PsGetCurrentProcess ();

        DataPage = MiMapPageInHyperSpaceAtDpc (Process, PageFrameIndex);

        RtlCopyMemory (ImagePage, DataPage, PAGE_SIZE);

        MiUnmapPageInHyperSpaceFromDpc (Process, DataPage);

        UNLOCK_PFN (OldIrql);

        //
        // If the new page had been used to hold code in a previous
        // life we must ensure the instruction cache is kept coherent.
        // The data copy above is not sufficient on platforms where
        // the I-cache & D-cache are not kept coherent in hardware
        // so sweep it here for them.
        //

        KeSweepIcacheRange (TRUE, ImagePage, PAGE_SIZE);

        return ImagePage;
    }

    //
    // The data page is not resident, so return NULL and the caller will take
    // the long way.
    //

    UNLOCK_PFN (OldIrql);

    if (ImagePte != NULL) {
        MiReleaseSystemPtes (ImagePte, 1, SystemPteSpace);
    }

    return NULL;
}

