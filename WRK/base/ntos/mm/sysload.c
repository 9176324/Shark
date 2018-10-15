/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   sysload.c

Abstract:

    This module contains the code to load DLLs into the system portion of
    the address space and calls the DLL at its initialization entry point.

--*/

#include "mi.h"

KMUTANT MmSystemLoadLock;

LONG MmTotalSystemDriverPages;

ULONG MmDriverCommit;

LONG MiFirstDriverLoadEver = 0;

//
// This key is set to TRUE to make more memory below 16mb available for drivers.
// It can be cleared via the registry.
//

LOGICAL MmMakeLowMemory = TRUE;

//
// Enabled via the registry to identify drivers which unload without releasing
// resources or still have active timers, etc.
//

PUNLOADED_DRIVERS MmUnloadedDrivers;

ULONG MmLastUnloadedDriver;
ULONG MiTotalUnloads;
ULONG MiUnloadsSkipped;

//
// This can be set by the registry.
//

ULONG MmEnforceWriteProtection = 1;

//
// Referenced by ke\bugcheck.c.
//

PVOID ExPoolCodeStart;
PVOID ExPoolCodeEnd;
PVOID MmPoolCodeStart;
PVOID MmPoolCodeEnd;
PVOID MmPteCodeStart;
PVOID MmPteCodeEnd;

NTSTATUS
LookupEntryPoint (
    IN PVOID DllBase,
    IN PSZ NameOfEntryPoint,
    OUT PVOID *AddressOfEntryPoint
    );

PVOID
MiCacheImageSymbols (
    IN PVOID ImageBase
    );

NTSTATUS
MiResolveImageReferences (
    PVOID ImageBase,
    IN PUNICODE_STRING ImageFileDirectory,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    OUT PCHAR *MissingProcedureName,
    OUT PWSTR *MissingDriverName,
    OUT PLOAD_IMPORTS *LoadedImports
    );

NTSTATUS
MiSnapThunk (
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA NameThunk,
    OUT PIMAGE_THUNK_DATA AddrThunk,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN LOGICAL SnapForwarder,
    OUT PCHAR *MissingProcedureName
    );

NTSTATUS
MiLoadImageSection (
    IN OUT PSECTION *InputSectionPointer,
    OUT PVOID *ImageBase,
    IN PUNICODE_STRING ImageFileName,
    IN ULONG LoadInSessionSpace,
    IN PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry
    );

VOID
MiEnablePagingOfDriver (
    IN PVOID ImageHandle
    );

VOID
MiSetPagingOfDriver (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    );

PVOID
MiLookupImageSectionByName (
    IN PVOID Base,
    IN LOGICAL MappedAsImage,
    IN PCHAR SectionName,
    OUT PULONG SectionSize
    );

VOID
MiClearImports (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

NTSTATUS
MiBuildImportsForBootDrivers (
    VOID
    );

LONG
MiMapCacheExceptionFilter (
    OUT PNTSTATUS Status,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

NTSTATUS
MiDereferenceImports (
    IN PLOAD_IMPORTS ImportList
    );

LOGICAL
MiCallDllUnloadAndUnloadDll (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

PVOID
MiLocateExportName (
    IN PVOID DllBase,
    IN PCHAR FunctionName
    );

VOID
MiRememberUnloadedDriver (
    IN PUNICODE_STRING DriverName,
    IN PVOID Address,
    IN ULONG Length
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiLocateKernelSections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiCaptureImageExceptionValues (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiUpdateThunks (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PVOID OldAddress,
    IN PVOID NewAddress,
    IN ULONG NumberOfBytes
    );

PVOID
MiFindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    );

LOGICAL
MiUseLargeDriverPage (
    IN ULONG NumberOfPtes,
    IN OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING PrefixedImageName,
    IN ULONG Pass
    );

VOID
MiRundownHotpatchList (
    PVOID PatchHead
    );

VOID
MiSessionProcessGlobalSubsections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

MM_PROTECTION_MASK
MiComputeDriverProtection (
    IN LOGICAL SessionDriver,
    IN ULONG SectionProtection
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmCheckSystemImage)
#pragma alloc_text(PAGE,MmLoadSystemImage)
#pragma alloc_text(PAGE,MiResolveImageReferences)
#pragma alloc_text(PAGE,MiSnapThunk)
#pragma alloc_text(PAGE,MiEnablePagingOfDriver)
#pragma alloc_text(PAGE,MmPageEntireDriver)
#pragma alloc_text(PAGE,MiDereferenceImports)
#pragma alloc_text(PAGE,MiCallDllUnloadAndUnloadDll)
#pragma alloc_text(PAGE,MiLocateExportName)
#pragma alloc_text(PAGE,MiClearImports)
#pragma alloc_text(PAGE,MmGetSystemRoutineAddress)
#pragma alloc_text(PAGE,MiFindExportedRoutineByName)
#pragma alloc_text(PAGE,MmCallDllInitialize)
#pragma alloc_text(PAGE,MmResetDriverPaging)
#pragma alloc_text(PAGE,MmUnloadSystemImage)
#pragma alloc_text(PAGE,MiLoadImageSection)
#pragma alloc_text(PAGE,MiRememberUnloadedDriver)
#pragma alloc_text(PAGE,MiUseLargeDriverPage)
#pragma alloc_text(PAGE,MiMakeEntireImageCopyOnWrite)
#pragma alloc_text(PAGE,MiWriteProtectSystemImage)
#pragma alloc_text(PAGE,MiSessionProcessGlobalSubsections)
#pragma alloc_text(PAGE,MiCaptureImageExceptionValues)
#pragma alloc_text(PAGE,MiComputeDriverProtection)
#pragma alloc_text(INIT,MiBuildImportsForBootDrivers)
#pragma alloc_text(INIT,MiReloadBootLoadedDrivers)
#pragma alloc_text(INIT,MiUpdateThunks)
#pragma alloc_text(INIT,MiInitializeLoadedModuleList)
#pragma alloc_text(INIT,MiLocateKernelSections)

#if !defined(NT_UP)
#pragma alloc_text(PAGE,MmVerifyImageIsOkForMpUse)
#endif

#endif

VOID
MiProcessLoaderEntry (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN LOGICAL Insert
    )

/*++

Routine Description:

    This function is a nonpaged wrapper which acquires the PsLoadedModuleList
    lock to insert a new entry.

Arguments:

    DataTableEntry - Supplies the loaded module list entry to insert/remove.

    Insert - Supplies TRUE if the entry should be inserted, FALSE if the entry
             should be removed.

Return Value:

    None.

Environment:

    Kernel mode.  Normal APCs disabled (critical region held).

--*/

{
    KIRQL OldIrql;

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);
    OldIrql = KeRaiseIrqlToSynchLevel ();
    ExAcquireSpinLockAtDpcLevel (&PsLoadedModuleSpinLock);

    if (Insert == TRUE) {
        InsertTailList (&PsLoadedModuleList, &DataTableEntry->InLoadOrderLinks);

#if defined (_WIN64)

        RtlInsertInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry->DllBase,
                                        DataTableEntry->SizeOfImage);

#endif

    }
    else {

#if defined (_WIN64)

        RtlRemoveInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry->DllBase);

#endif

        RemoveEntryList (&DataTableEntry->InLoadOrderLinks);
    }

    ExReleaseSpinLockFromDpcLevel (&PsLoadedModuleSpinLock);
    KeLowerIrql (OldIrql);
    ExReleaseResourceLite (&PsLoadedModuleResource);
}

typedef struct _MI_LARGE_PAGE_DRIVER_ENTRY {
    LIST_ENTRY Links;
    UNICODE_STRING BaseName;
} MI_LARGE_PAGE_DRIVER_ENTRY, *PMI_LARGE_PAGE_DRIVER_ENTRY;

LIST_ENTRY MiLargePageDriverList;

ULONG MiLargePageAllDrivers;

VOID
MiInitializeDriverLargePageList (
    VOID
    )

/*++

Routine Description:

    Parse the registry settings and set up the list of driver names that we'll
    try to load in large pages.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, Phase 0 Initialization.

    Nonpaged pool exists but not paged pool.

    The PsLoadedModuleList has not been set up yet AND the boot drivers
    have NOT been relocated to their final resting places.

--*/
{
    PWCHAR Start;
    PWCHAR End;
    PWCHAR Walk;
    ULONG NameLength;
    PMI_LARGE_PAGE_DRIVER_ENTRY Entry;

    InitializeListHead (&MiLargePageDriverList);

    if (MmLargePageDriverBufferLength == (ULONG)-1) {
        return;
    }

    Start = MmLargePageDriverBuffer;
    End = MmLargePageDriverBuffer + (MmLargePageDriverBufferLength - sizeof(WCHAR)) / sizeof(WCHAR);

    while (Start < End) {
        if (UNICODE_WHITESPACE(*Start)) {
            Start += 1;
            continue;
        }

        if (*Start == (WCHAR)'*') {
            MiLargePageAllDrivers = 1;
            break;
        }

        for (Walk = Start; Walk < End; Walk += 1) {
            if (UNICODE_WHITESPACE(*Walk)) {
                break;
            }
        }

        //
        // Got a string - add it to our list.
        //

        NameLength = (ULONG)(Walk - Start) * sizeof (WCHAR);


        Entry = ExAllocatePoolWithTag (NonPagedPool,
                                       sizeof (MI_LARGE_PAGE_DRIVER_ENTRY),
                                       'pLmM');

        if (Entry == NULL) {
            break;
        }

        Entry->BaseName.Buffer = Start;
        Entry->BaseName.Length = (USHORT) NameLength;
        Entry->BaseName.MaximumLength = (USHORT) NameLength;

        InsertTailList (&MiLargePageDriverList, &Entry->Links);

        Start = Walk + 1;
    }

    return;
}

LOGICAL
MiUseLargeDriverPage (
    IN ULONG NumberOfPtes,
    IN OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING BaseImageName,
    IN ULONG Pass
    )

/*++

Routine Description:

    This routine checks whether the specified image should be loaded into
    a large page address space, and if so, tries to load it.

Arguments:

    NumberOfPtes - Supplies the number of PTEs to map for the image.

    ImageBaseAddress - Supplies the current address the image header is at,
                       and returns the (new) address for the image header.

    BaseImageName - Supplies the base path name of the image to load.

    Pass - Supplies 0 when called from Phase for the boot drivers, 1 otherwise.

Return Value:

    TRUE if large pages were used, FALSE if not.

--*/

{
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ResidentPages;
    PLIST_ENTRY NextEntry;
    PVOID SmallVa;
    PVOID LargeVa;
    PVOID LargeBaseVa;
    LOGICAL UseLargePages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MMPTE PteContents;
    PMMPTE SmallPte;
    PMMPTE LastSmallPte;
    PMI_LARGE_PAGE_DRIVER_ENTRY LargePageDriverEntry;
#ifdef _X86_
    ULONG ProcessorFeatures;
#endif

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
    ASSERT (*ImageBaseAddress >= MmSystemRangeStart);

#ifdef _X86_
    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        return FALSE;
    }

    //
    // Capture cr4 to see if large page support has been enabled in the chip
    // yet (late in Phase 1).  Large page PDEs cannot be used until then.
    //
    // mov     eax, cr4
    //

    _asm {
        _emit 00fh
        _emit 020h
        _emit 0e0h
        mov     ProcessorFeatures, eax
    }

    if ((ProcessorFeatures & CR4_PSE) == 0) {
        return FALSE;
    }
#endif

    //
    // Check the number of free system PTEs left to prevent a runaway registry
    // key from exhausting all the system PTEs.
    //

    if (MmTotalFreeSystemPtes[SystemPteSpace] < 16 * (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) {

        return FALSE;
    }

    if (MiLargePageAllDrivers == 0) {

        UseLargePages = FALSE;

        //
        // Check to see if this name exists in the large page image list.
        //

        NextEntry = MiLargePageDriverList.Flink;

        while (NextEntry != &MiLargePageDriverList) {

            LargePageDriverEntry = CONTAINING_RECORD (NextEntry,
                                                      MI_LARGE_PAGE_DRIVER_ENTRY,
                                                      Links);

            if (RtlEqualUnicodeString (BaseImageName,
                                       &LargePageDriverEntry->BaseName,
                                       TRUE)) {

                UseLargePages = TRUE;
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        if (UseLargePages == FALSE) {
            return FALSE;
        }
    }

    //
    // First try to get physically contiguous memory for this driver.
    // Note we must allocate the entire large page here even though we will
    // almost always only use a portion of it.  This is to ensure no other
    // frames in it can be mapped with a different cache attribute.  After
    // updating the cache attribute lists, we'll immediately free the excess.
    // Note that the driver's INIT section will be subsequently freed
    // and clearly the cache attribute lists must be correct to support that
    // as well.
    //
    // Don't take memory below 16MB as we want to leave that available for
    // ISA drivers supporting older hardware that may require it.
    //

    NumberOfPages = (PFN_NUMBER) MI_ROUND_TO_SIZE (
                        NumberOfPtes,
                        MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

    PageFrameIndex = MiFindContiguousPages ((16 * 1024 * 1024) >> PAGE_SHIFT,
                                            MmHighestPossiblePhysicalPage,
                                            MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT,
                                            NumberOfPages,
                                            MmCached);

    //
    // If a contiguous range is not available then large pages cannot be used
    // for this driver at this time.
    //

    if (PageFrameIndex == 0) {
        return FALSE;
    }

    //
    // Add the contiguous range to the must-be-cached list so that the excess
    // memory (and INIT section) can be safely freed to the page lists.
    //

    if (MiAddCachedRange (PageFrameIndex, PageFrameIndex + NumberOfPages - 1) == FALSE) {
        MiFreeContiguousPages (PageFrameIndex, NumberOfPages);
        return FALSE;
    }

    //
    // Try to get large virtual address space for this driver.
    //

    LargeVa = MiMapWithLargePages (PageFrameIndex,
                                   NumberOfPages,
                                   MM_EXECUTE_READWRITE,
                                   MmCached);

    if (LargeVa == NULL) {
        MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + NumberOfPages - 1);
        MiFreeContiguousPages (PageFrameIndex, NumberOfPages);
        return FALSE;
    }

    LargeBaseVa = LargeVa;

    //
    // Copy the driver a page at a time as in rare cases, it may have holes.
    //

    SmallPte = MiGetPteAddress (*ImageBaseAddress);
    LastSmallPte = SmallPte + NumberOfPtes;

    SmallVa = MiGetVirtualAddressMappedByPte (SmallPte);

    while (SmallPte < LastSmallPte) {

        PteContents = *SmallPte;

        if (PteContents.u.Hard.Valid == 1) {
            RtlCopyMemory (LargeVa, SmallVa, PAGE_SIZE);
        }
        else {

            //
            // Retain this page in the large page mapping to simplify unload -
            // ie: it can always free a single contiguous range.
            //
        }

        SmallPte += 1;

        LargeVa = (PVOID) ((PCHAR)LargeVa + PAGE_SIZE);
        SmallVa = (PVOID) ((PCHAR)SmallVa + PAGE_SIZE);
    }

    //
    // Inform our caller of the new (large page) address so the loader data
    // table entry gets created with it & fixups done accordingly, etc.
    //

    *ImageBaseAddress = LargeBaseVa;

    if (Pass != 0) {

        //
        // The system is fully booted so get rid of the original mapping now.
        // Otherwise, we're in Phase 0, so the caller gets rid of the original
        // mapping.
        //

        SmallPte -= NumberOfPtes;

        //
        // No explicit TB flush is done here, instead the system PTE management
        // code will handle this in a lazy fashion.
        //

        PagesRequired = MiDeleteSystemPageableVm (SmallPte,
                                                 NumberOfPtes,
                                                 0,
                                                 &ResidentPages);

        //
        // Non boot-loaded drivers have system PTEs and commit charged.
        //

        MiReleaseSystemPtes (SmallPte, (ULONG)NumberOfPtes, SystemPteSpace);

        InterlockedExchangeAdd (&MmTotalSystemDriverPages,
                                0 - (ULONG)(PagesRequired - ResidentPages));

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

        MiReturnCommitment (PagesRequired);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, PagesRequired);
    }

    //
    // Note the unused trailing portion (and their resident available charge)
    // of the large page mapping are kept as they would have to be mapped
    // cached to avoid TB attribute conflicts, and therefore cannot be placed
    // on the general purpose freelists.
    //

    return TRUE;
}

        
VOID
MiCaptureImageExceptionValues (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

/*++

Routine Description:

    This function stores the exception table information from the image in the
    loader data table entry.

Arguments:

    DataTableEntry - Supplies the kernel's data table entry.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL or below, arbitrary process context.

--*/

{
    PVOID CurrentBase;
    PIMAGE_NT_HEADERS NtHeader;

    CurrentBase = (PVOID) DataTableEntry->DllBase;

    NtHeader = RtlImageNtHeader (CurrentBase);

#if defined(_X86_)
    if (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {
        DataTableEntry->ExceptionTable = (PCHAR)LongToPtr(-1);
        DataTableEntry->ExceptionTableSize = (ULONG)-1;
    } else {
        PIMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig;
        ULONG LoadConfigSize;
        if (IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG < NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
            LoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY32)((PCHAR)CurrentBase +
                    NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);
            LoadConfigSize = NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size;
            if (LoadConfig && 
                LoadConfigSize &&
                LoadConfig->Size >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount) &&
                LoadConfig->SEHandlerTable &&
                LoadConfig->SEHandlerCount
                )
            {
                DataTableEntry->ExceptionTable = (PVOID)LoadConfig->SEHandlerTable;
                DataTableEntry->ExceptionTableSize = LoadConfig->SEHandlerCount;
            } else {
                DataTableEntry->ExceptionTable = 0;
                DataTableEntry->ExceptionTableSize = 0;
            }
        }
    }
#else
    if (IMAGE_DIRECTORY_ENTRY_EXCEPTION < NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
        DataTableEntry->ExceptionTable = (PCHAR)CurrentBase + 
                NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
        DataTableEntry->ExceptionTableSize = 
                NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;
    }
#endif
}


NTSTATUS
MmLoadSystemImage (
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    IN PUNICODE_STRING LoadedBaseName OPTIONAL,
    IN ULONG LoadFlags,
    OUT PVOID *ImageHandle,
    OUT PVOID *ImageBaseAddress
    )

/*++

Routine Description:

    This routine reads the image pages from the specified section into
    the system and returns the address of the DLL's header.

    At successful completion, the Section is referenced so it remains
    until the system image is unloaded.

Arguments:

    ImageFileName - Supplies the full path name (including the image name)
                    of the image to load.

    NamePrefix - If present, supplies the prefix to use with the image name on
                 load operations.  This is used to load the same image multiple
                 times, by using different prefixes.

    LoadedBaseName - If present, supplies the base name to use on the
                     loaded image instead of the base name found on the
                     image name.

    LoadFlags - Supplies a combination of bit flags as follows:

        MM_LOAD_IMAGE_IN_SESSION :
                       - Supplies whether to load this image in session space.
                         Each session gets a different copy of this driver with
                         pages shared as much as possible via copy on write.

        MM_LOAD_IMAGE_AND_LOCKDOWN :
                       - Supplies TRUE if the image pages should be made
                         non-pageable.

    ImageHandle - Returns an opaque pointer to the referenced section object
                  of the image that was loaded.

    ImageBaseAddress - Returns the image base within the system.

Return Value:

    Status of the load operation.

Environment:

    Kernel mode, APC_LEVEL or below, arbitrary process context.

--*/

{
    LONG OldValue;
    ULONG i;
    ULONG DebugInfoSize;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    PIMAGE_DEBUG_DIRECTORY DebugDir;
    PNON_PAGED_DEBUG_INFO ssHeader;
    PMMPTE PointerPte;
    SIZE_T DataTableEntrySize;
    PWSTR BaseDllNameBuffer;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    KLDR_DATA_TABLE_ENTRY TempDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry;
    NTSTATUS Status;
    PSECTION SectionPointer;
    PIMAGE_NT_HEADERS NtHeaders;
    UNICODE_STRING PrefixedImageName;
    UNICODE_STRING BaseName;
    UNICODE_STRING BaseDirectory;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle;
    HANDLE SectionHandle;
    IO_STATUS_BLOCK IoStatus;
    PCHAR NameBuffer;
    PLIST_ENTRY NextEntry;
    ULONG NumberOfPtes;
    PCHAR MissingProcedureName;
    PWSTR MissingDriverName;
    PWSTR PrintableMissingDriverName;
    PLOAD_IMPORTS LoadedImports;
    LOGICAL AlreadyOpen;
    LOGICAL IssueUnloadOnFailure;
    LOGICAL LoadLockOwned;
    ULONG SectionAccess;
    PKTHREAD CurrentThread;

    PAGED_CODE();

    if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

        ASSERT (NamePrefix == NULL);
        ASSERT (LoadedBaseName == NULL);

        if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
            return STATUS_NO_MEMORY;
        }
    }

    LoadLockOwned = FALSE;
    LoadedImports = (PLOAD_IMPORTS) NO_IMPORTS_USED;
    SectionPointer = NULL;
    FileHandle = (HANDLE)0;
    MissingProcedureName = NULL;
    MissingDriverName = NULL;
    IssueUnloadOnFailure = FALSE;
    FoundDataTableEntry = NULL;

    NameBuffer = ExAllocatePoolWithTag (NonPagedPool,
                                        MAXIMUM_FILENAME_LENGTH,
                                        'nLmM');

    if (NameBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Initializing these is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check
    // for use of uninitialized variables.
    //

    SATISFY_OVERZEALOUS_COMPILER (NumberOfPtes = (ULONG)-1);
    DataTableEntry = NULL;

    //
    // Get name roots.
    //

    if (ImageFileName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR) {
        PWCHAR p;
        ULONG l;

        p = &ImageFileName->Buffer[ImageFileName->Length>>1];
        while (*(p-1) != OBJ_NAME_PATH_SEPARATOR) {
            p--;
        }
        l = (ULONG)(&ImageFileName->Buffer[ImageFileName->Length>>1] - p);
        l *= sizeof(WCHAR);
        BaseName.Length = (USHORT)l;
        BaseName.Buffer = p;
    }
    else {
        BaseName.Length = ImageFileName->Length;
        BaseName.Buffer = ImageFileName->Buffer;
    }

    BaseName.MaximumLength = BaseName.Length;
    BaseDirectory = *ImageFileName;
    BaseDirectory.Length = (USHORT)(BaseDirectory.Length - BaseName.Length);
    BaseDirectory.MaximumLength = BaseDirectory.Length;
    PrefixedImageName = *ImageFileName;

    //
    // If there's a name prefix, add it to the PrefixedImageName.
    //

    if (NamePrefix) {
        PrefixedImageName.MaximumLength = (USHORT)(BaseDirectory.Length + NamePrefix->Length + BaseName.Length);

        PrefixedImageName.Buffer = ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    PrefixedImageName.MaximumLength,
                                    'dLmM');

        if (!PrefixedImageName.Buffer) {
            ExFreePool (NameBuffer);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        PrefixedImageName.Length = 0;
        RtlAppendUnicodeStringToString(&PrefixedImageName, &BaseDirectory);
        RtlAppendUnicodeStringToString(&PrefixedImageName, NamePrefix);
        RtlAppendUnicodeStringToString(&PrefixedImageName, &BaseName);

        //
        // Alter the basename to match.
        //

        BaseName.Buffer = PrefixedImageName.Buffer + BaseDirectory.Length / sizeof(WCHAR);
        BaseName.Length = (USHORT)(BaseName.Length + NamePrefix->Length);
        BaseName.MaximumLength = (USHORT)(BaseName.MaximumLength + NamePrefix->Length);
    }

    //
    // If there's a loaded base name, use it instead of the base name.
    //

    if (LoadedBaseName) {
        BaseName = *LoadedBaseName;
    }

#if DBG
    if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
            "MM:SYSLDR Loading %wZ (%wZ) %s\n",
            &PrefixedImageName,
            &BaseName,
            (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) ? "in session space" : " ");
    }
#endif

    AlreadyOpen = FALSE;

ReCheckLoaderList:

    //
    // Arbitrary process context so prevent suspend APCs now.
    //

    ASSERT (LoadLockOwned == FALSE);
    LoadLockOwned = TRUE;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    //
    // Check to see if this name already exists in the loader database.
    //

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        if (RtlEqualUnicodeString (&PrefixedImageName,
                                   &DataTableEntry->FullDllName,
                                   TRUE)) {
            break;
        }

        NextEntry = NextEntry->Flink;
    }

    if (NextEntry != &PsLoadedModuleList) {

        //
        // Found a match in the loaded module list.  See if it's acceptable.
        //
        // If this thread already loaded the image below and upon rechecking
        // finds some other thread did so also, then get rid of our object
        // now and use the other thread's inserted entry instead.
        //

        if (SectionPointer != NULL) {
            ObDereferenceObject (SectionPointer);
            SectionPointer = NULL;
        }

        if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) {

            if (MI_IS_SESSION_ADDRESS (DataTableEntry->DllBase) == TRUE) {

                //
                // The caller is trying to load a driver in systemwide space
                // that has already been loaded in session space.  This is
                // not allowed.
                //

                Status = STATUS_CONFLICTING_ADDRESSES;
            }
            else {
                *ImageHandle = DataTableEntry;
                *ImageBaseAddress = DataTableEntry->DllBase;
                Status = STATUS_IMAGE_ALREADY_LOADED;
            }
            goto return2;
        }

        if (MI_IS_SESSION_ADDRESS (DataTableEntry->DllBase) == FALSE) {

            //
            // The caller is trying to load a driver in session space
            // that has already been loaded in system space.  This is
            // not allowed.
            //

            Status = STATUS_CONFLICTING_ADDRESSES;
            goto return2;
        }

        AlreadyOpen = TRUE;

        //
        // This image has already been loaded systemwide.  If it's
        // already been loaded in this session space as well, just
        // bump the reference count using the already allocated
        // address.  Otherwise, insert it into this session space.
        //

        Status = MiSessionInsertImage (DataTableEntry->DllBase);

        if (!NT_SUCCESS (Status)) {

            if (Status == STATUS_ALREADY_COMMITTED) {

                //
                // This driver's already been loaded in this session.
                //

                ASSERT (DataTableEntry->LoadCount >= 1);

                *ImageHandle = DataTableEntry;
                *ImageBaseAddress = DataTableEntry->DllBase;

                Status = STATUS_SUCCESS;
            }

            //
            // The LoadCount should generally not be 0 here, but it is
            // possible in the case where an attempt has been made to
            // unload a DLL on last dereference, but the DLL refused to
            // unload.
            //

            goto return2;
        }

        //
        // This driver is already loaded in the system, but not in
        // this particular session - share it now.
        //

        FoundDataTableEntry = DataTableEntry;

        DataTableEntry->LoadCount += 1;

        ASSERT (DataTableEntry->SectionPointer != NULL);

        SectionPointer = DataTableEntry->SectionPointer;
    }
    else if (SectionPointer == NULL) {

        //
        // This image is not already loaded.
        //
        // A NULL SectionPointer indicates this thread didn't already load
        // this image below either, so go and get it.
        //
        // Release the load lock first as getting the image is not cheap.
        //

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;

        InterlockedOr (&MiFirstDriverLoadEver, 0x1);

        //
        // Check and see if a user wants to replace this binary
        // via a transfer through the kernel debugger.  If this
        // fails just continue on with the existing file.
        //

        if ((KdDebuggerEnabled) && (KdDebuggerNotPresent == FALSE)) {

            Status = KdPullRemoteFile (ImageFileName,
                                       FILE_ATTRIBUTE_NORMAL,
                                       FILE_OVERWRITE_IF,
                                       FILE_SYNCHRONOUS_IO_NONALERT);

            if (NT_SUCCESS (Status)) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
                    "MmLoadSystemImage: Pulled %wZ from kd\n",
                          ImageFileName);
            }
        }

        DataTableEntry = NULL;

        //
        // Attempt to open the driver image itself.  If this fails, then the
        // driver image cannot be located, so nothing else matters.
        //

        InitializeObjectAttributes (&ObjectAttributes,
                                    ImageFileName,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

        Status = ZwOpenFile (&FileHandle,
                             FILE_EXECUTE,
                             &ObjectAttributes,
                             &IoStatus,
                             FILE_SHARE_READ | FILE_SHARE_DELETE,
                             0);

        if (!NT_SUCCESS (Status)) {

#if DBG
            if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                    "MmLoadSystemImage: cannot open %wZ\n",
                    ImageFileName);
            }
#endif
            //
            // File not found.
            //

            goto return2;
        }

        Status = MmCheckSystemImage (FileHandle, FALSE);

        if ((Status == STATUS_IMAGE_CHECKSUM_MISMATCH) ||
            (Status == STATUS_IMAGE_MP_UP_MISMATCH) ||
            (Status == STATUS_INVALID_IMAGE_PROTECT)) {

            goto return1;
        }

        //
        // Now attempt to create an image section for the file.  If this fails,
        // then the driver file is not an image.  Session space drivers are
        // shared text with copy on write data, so don't allow writes here.
        //

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
            SectionAccess = SECTION_MAP_READ | SECTION_MAP_EXECUTE;
        }
        else {
            SectionAccess = SECTION_ALL_ACCESS;
        }

        InitializeObjectAttributes (&ObjectAttributes,
                                    NULL,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

        Status = ZwCreateSection (&SectionHandle,
                                  SectionAccess,
                                  &ObjectAttributes,
                                  (PLARGE_INTEGER) NULL,
                                  PAGE_EXECUTE,
                                  SEC_IMAGE,
                                  FileHandle);

        if (!NT_SUCCESS(Status)) {
            goto return1;
        }

        //
        // Now reference the section handle.  If this fails something is
        // very wrong because it is a kernel handle.
        //
        // N.B.  ObRef sets SectionPointer to NULL on failure.
        //

        Status = ObReferenceObjectByHandle (SectionHandle,
                                            SECTION_MAP_EXECUTE,
                                            MmSectionObjectType,
                                            KernelMode,
                                            (PVOID *) &SectionPointer,
                                            (POBJECT_HANDLE_INFORMATION) NULL);

        ZwClose (SectionHandle);

        if (!NT_SUCCESS (Status)) {
            goto return1;
        }

        if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) &&
            (SectionPointer->Segment->ControlArea->u.Flags.FloppyMedia == 0)) {

            //
            // Check with all of the drivers along the path to win32k.sys to
            // ensure that they are willing to follow the rules required
            // of them and to give them a chance to lock down code and data
            // that needs to be locked.  If any of the drivers along the path
            // refuses to participate, fail the win32k.sys load.
            //
            // It is assumed that all session drivers live on the same physical
            // drive, so when the very first session driver is loaded, this
            // check can be made.
            //
            // Note that this is important because these drivers are always
            // paged directly in/out from the filesystem so the drive
            // containing the filesystem better not get removed !
            //

            //
            // This is skipped for the WinPE removable media boot case because
            // the user may be running WinPE in RAM and want to swap out the
            // boot media.  In this case, the control area is marked as
            // FloppyMedia (even if it was CD-based) as all the pages have
            // already been converted to pagefile-backed.
            //

            do {
                OldValue = MiFirstDriverLoadEver;

                if (OldValue & 0x2) {
                    break;
                }

                if (InterlockedCompareExchange (&MiFirstDriverLoadEver, OldValue | 0x2, OldValue) == OldValue) {

                    Status = PpPagePathAssign (SectionPointer->Segment->ControlArea->FilePointer);

                    if (!NT_SUCCESS (Status)) {

                        KdPrint (("PpPagePathAssign FAILED for %wZ: %x\n",
                                 ImageFileName, Status));

                        //
                        // Failing the insertion of win32k.sys' device
                        // in the pagefile path is commented out until
                        // the storage drivers have been modified to
                        // correctly handle this request.  If this is
                        // added later, add code here to release relevant
                        // resources for this error path.
                        //
                    }

                    break;
                }
            } while (TRUE);
        }

        //
        // Anything may have changed while the load lock was released.
        // So before using the section we just created, go back and see
        // if any other threads have slipped through and did it already.
        //

        goto ReCheckLoaderList;
    }
    else {
        DataTableEntry = NULL;
    }

    //
    // Load the driver from the filesystem and pick a virtual address for it.
    // All session images are paged directly to and from the filesystem so these
    // images remain busy.
    //

    Status = MiLoadImageSection (&SectionPointer,
                                 ImageBaseAddress,
                                 ImageFileName,
                                 LoadFlags & MM_LOAD_IMAGE_IN_SESSION,
                                 FoundDataTableEntry);

    ASSERT (Status != STATUS_ALREADY_COMMITTED);

    NumberOfPtes = SectionPointer->Segment->TotalNumberOfPtes;

    //
    // Normal drivers are dereferenced here and their images can then be
    // overwritten.  This is ok because we've already read the whole thing
    // into memory and from here until reboot (or unload), we back them
    // with the pagefile.
    //
    // Session space drivers are the exception - these images
    // are inpaged from the filesystem and we need to keep our reference to
    // the file so that it doesn't get overwritten.
    //

    if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) {

        if (NT_SUCCESS (Status)) {

            //
            // Move the driver into a large page if requested via the registry.
            //

            MiUseLargeDriverPage (SectionPointer->Segment->TotalNumberOfPtes,
                                  ImageBaseAddress,
                                  &BaseName,
                                  1);
        }

        ObDereferenceObject (SectionPointer);
        SectionPointer = NULL;
    }

    //
    // The module LoadCount will be 1 here if the module was just loaded.
    // The LoadCount will be >1 if it was attached to by a session (as opposed
    // to just loaded).
    //

    if (!NT_SUCCESS (Status)) {

        if (AlreadyOpen == TRUE) {

            //
            // We're failing and we were just attaching to an already loaded
            // driver.  We don't want to go through the forced unload path
            // because we've already deleted the address space so
            // decrement our reference and clear the DataTableEntry.
            //

            ASSERT (DataTableEntry != NULL);
            DataTableEntry->LoadCount -= 1;
            DataTableEntry = NULL;
        }
        goto return1;
    }

    //
    // Error recovery from this point out for sessions works as follows:
    //
    // For sessions, we may or may not have a DataTableEntry at this point.
    // If we do, it's because we're attaching to a driver that has already
    // been loaded - and the DataTableEntry->LoadCount has been bumped - so
    // the error recovery from here on out is to just call
    // MmUnloadSystemImage with the DataTableEntry.
    //
    // If this is the first load of a given driver into a session space, we
    // have no DataTableEntry at this point.  The view has already been mapped
    // and committed and the group/session addresses reserved for this DLL.
    // The error recovery path handles all this because
    // MmUnloadSystemImage zeroes the relevant fields in the
    // LDR_DATA_TABLE_ENTRY so that MmUnloadSystemImage works properly.
    //

    IssueUnloadOnFailure = TRUE;

    if (AlreadyOpen == FALSE) {

        if (((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) ||
            (*ImageBaseAddress != SectionPointer->Segment->BasedAddress)) {

            //
            // Apply the fixups to the section.  Note session images need only
            // be fixed up once per insertion in the loaded module list.
            //
    
            try {
                Status = LdrRelocateImage (*ImageBaseAddress,
                                           "SYSLDR",
                                           STATUS_SUCCESS,
                                           STATUS_CONFLICTING_ADDRESSES,
                                           STATUS_INVALID_IMAGE_FORMAT);
    
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode ();
                KdPrint(("MM:sysload - LdrRelocateImage failed status %lx\n",
                          Status));
            }
    
            if (!NT_SUCCESS(Status)) {
    
                //
                // Unload the system image and dereference the section.
                //
    
                goto return1;
            }
        }

        DebugInfoSize = 0;
        DataDirectory = NULL;
        DebugDir = NULL;

        NtHeaders = RtlImageNtHeader (*ImageBaseAddress);

        //
        // Create a loader table entry for this driver before resolving the
        // references so that any circular references can resolve properly.
        //

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

            DebugInfoSize = sizeof (NON_PAGED_DEBUG_INFO);

            if (IMAGE_DIRECTORY_ENTRY_DEBUG <
                NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {

                DataDirectory = &NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

                if (DataDirectory->VirtualAddress &&
                    DataDirectory->Size &&
                    (DataDirectory->VirtualAddress + DataDirectory->Size) <
                        NtHeaders->OptionalHeader.SizeOfImage) {

                    DebugDir = (PIMAGE_DEBUG_DIRECTORY)
                               ((PUCHAR)(*ImageBaseAddress) +
                                   DataDirectory->VirtualAddress);

                    DebugInfoSize += DataDirectory->Size;

                    for (i = 0;
                         i < DataDirectory->Size/sizeof(IMAGE_DEBUG_DIRECTORY);
                         i += 1) {

                        if ((DebugDir+i)->AddressOfRawData &&
                            (DebugDir+i)->AddressOfRawData <
                                NtHeaders->OptionalHeader.SizeOfImage &&
                            ((DebugDir+i)->AddressOfRawData +
                                (DebugDir+i)->SizeOfData) <
                                NtHeaders->OptionalHeader.SizeOfImage) {

                            DebugInfoSize += (DebugDir+i)->SizeOfData;
                        }
                    }
                }

                DebugInfoSize = MI_ROUND_TO_SIZE(DebugInfoSize, sizeof(ULONG));
            }
        }

        DataTableEntrySize = sizeof (KLDR_DATA_TABLE_ENTRY) +
                             DebugInfoSize +
                             BaseName.Length + sizeof(UNICODE_NULL);

        DataTableEntry = ExAllocatePoolWithTag (NonPagedPool,
                                                DataTableEntrySize,
                                                'dLmM');

        if (DataTableEntry == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto return1;
        }

        //
        // Initialize the flags and load count.
        //

        DataTableEntry->Flags = LDRP_LOAD_IN_PROGRESS;
        DataTableEntry->LoadCount = 1;
        DataTableEntry->LoadedImports = (PVOID)LoadedImports;
        DataTableEntry->PatchInformation = NULL;

        if ((NtHeaders->OptionalHeader.MajorOperatingSystemVersion >= 5) &&
            (NtHeaders->OptionalHeader.MajorImageVersion >= 5)) {
            DataTableEntry->Flags |= LDRP_ENTRY_NATIVE;
        }

        ssHeader = (PNON_PAGED_DEBUG_INFO) ((ULONG_PTR)DataTableEntry +
                                            sizeof (KLDR_DATA_TABLE_ENTRY));

        BaseDllNameBuffer = (PWSTR) ((ULONG_PTR)ssHeader + DebugInfoSize);

        //
        // If loading a session space image, store away some debug data.
        //

        DataTableEntry->NonPagedDebugInfo = NULL;

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

            DataTableEntry->NonPagedDebugInfo = ssHeader;
            DataTableEntry->Flags |= LDRP_NON_PAGED_DEBUG_INFO;

            ssHeader->Signature = NON_PAGED_DEBUG_SIGNATURE;
            ssHeader->Flags = 1;
            ssHeader->Size = DebugInfoSize;
            ssHeader->Machine = NtHeaders->FileHeader.Machine;
            ssHeader->Characteristics = NtHeaders->FileHeader.Characteristics;
            ssHeader->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
            ssHeader->CheckSum = NtHeaders->OptionalHeader.CheckSum;
            ssHeader->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
            ssHeader->ImageBase = (ULONG_PTR) *ImageBaseAddress;

            if (DebugDir) {

                RtlCopyMemory (ssHeader + 1,
                               DebugDir,
                               DataDirectory->Size);

                DebugInfoSize = DataDirectory->Size;

                for (i = 0;
                     i < DataDirectory->Size/sizeof(IMAGE_DEBUG_DIRECTORY);
                     i += 1) {

                    if ((DebugDir + i)->AddressOfRawData &&
                        (DebugDir+i)->AddressOfRawData <
                            NtHeaders->OptionalHeader.SizeOfImage &&
                        ((DebugDir+i)->AddressOfRawData +
                            (DebugDir+i)->SizeOfData) <
                            NtHeaders->OptionalHeader.SizeOfImage) {

                        RtlCopyMemory ((PUCHAR)(ssHeader + 1) +
                                          DebugInfoSize,
                                      (PUCHAR)(*ImageBaseAddress) +
                                          (DebugDir + i)->AddressOfRawData,
                                      (DebugDir + i)->SizeOfData);

                        //
                        // Reset the offset in the debug directory to point to
                        //

                        (((PIMAGE_DEBUG_DIRECTORY)(ssHeader + 1)) + i)->
                            AddressOfRawData = DebugInfoSize;

                        DebugInfoSize += (DebugDir+i)->SizeOfData;
                    }
                    else {
                        (((PIMAGE_DEBUG_DIRECTORY)(ssHeader + 1)) + i)->
                            AddressOfRawData = 0;
                    }
                }
            }
        }

        //
        // Initialize the address of the DLL image file header and the entry
        // point address.
        //

        DataTableEntry->DllBase = *ImageBaseAddress;
        DataTableEntry->EntryPoint =
            ((PCHAR)*ImageBaseAddress + NtHeaders->OptionalHeader.AddressOfEntryPoint);
        DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;
        DataTableEntry->CheckSum = NtHeaders->OptionalHeader.CheckSum;
        DataTableEntry->SectionPointer = (PVOID) SectionPointer;

        //
        // Store the DLL name.
        //

        DataTableEntry->BaseDllName.Buffer = BaseDllNameBuffer;

        DataTableEntry->BaseDllName.Length = BaseName.Length;
        DataTableEntry->BaseDllName.MaximumLength = BaseName.Length;
        RtlCopyMemory (DataTableEntry->BaseDllName.Buffer,
                       BaseName.Buffer,
                       BaseName.Length);
        DataTableEntry->BaseDllName.Buffer[BaseName.Length/sizeof(WCHAR)] = UNICODE_NULL;

        DataTableEntry->FullDllName.Buffer = ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                         PrefixedImageName.Length + sizeof(UNICODE_NULL),
                                                         'TDmM');

        if (DataTableEntry->FullDllName.Buffer == NULL) {

            //
            // Pool could not be allocated, just set the length to 0.
            //

            DataTableEntry->FullDllName.Length = 0;
            DataTableEntry->FullDllName.MaximumLength = 0;
        }
        else {
            DataTableEntry->FullDllName.Length = PrefixedImageName.Length;
            DataTableEntry->FullDllName.MaximumLength = PrefixedImageName.Length;
            RtlCopyMemory (DataTableEntry->FullDllName.Buffer,
                           PrefixedImageName.Buffer,
                           PrefixedImageName.Length);
            DataTableEntry->FullDllName.Buffer[PrefixedImageName.Length/sizeof(WCHAR)] = UNICODE_NULL;
        }

        //
        // Capture the exception table data info
        //

        MiCaptureImageExceptionValues (DataTableEntry);

        //
        // Acquire the loaded module list resource and insert this entry
        // into the list.
        //

        MiProcessLoaderEntry (DataTableEntry, TRUE);

        MissingProcedureName = NameBuffer;
    
        try {
    
            //
            // Resolving the image references results in other DLLs being
            // loaded if they are referenced by the module that was just loaded.
            // An example is when an OEM printer or FAX driver links with
            // other general libraries.  This is not a problem for session space
            // because the general libraries do not have the global data issues
            // that win32k.sys and the video drivers do.  So we just call the
            // standard kernel reference resolver and any referenced libraries
            // get loaded into system global space.  Code in the routine
            // restricts which libraries can be referenced by a driver.
            //
    
            Status = MiResolveImageReferences (*ImageBaseAddress,
                                               &BaseDirectory,
                                               NamePrefix,
                                               &MissingProcedureName,
                                               &MissingDriverName,
                                               &LoadedImports);
    
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
            KdPrint(("MM:sysload - ResolveImageReferences failed status %x\n",
                        Status));
        }

        if (!NT_SUCCESS (Status)) {
#if DBG
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                ASSERT (MissingProcedureName == NULL);
            }
    
            if ((Status == STATUS_DRIVER_ORDINAL_NOT_FOUND) ||
                (Status == STATUS_OBJECT_NAME_NOT_FOUND) ||
                (Status == STATUS_DRIVER_ENTRYPOINT_NOT_FOUND)) {
    
                if ((ULONG_PTR)MissingProcedureName & ~((ULONG_PTR) (X64K-1))) {
    
                    //
                    // If not an ordinal, print string.
                    //
    
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MissingProcedureName %s\n", MissingProcedureName);
                }
                else {
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MissingProcedureName 0x%p\n", MissingProcedureName);
                }
            }
    
            if (MissingDriverName != NULL) {
                PrintableMissingDriverName = (PWSTR)((ULONG_PTR)MissingDriverName & ~0x1);
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                    "MissingDriverName %ws\n", PrintableMissingDriverName);
            }
#endif
            MiProcessLoaderEntry (DataTableEntry, FALSE);

            if (DataTableEntry->FullDllName.Buffer != NULL) {
                ExFreePool (DataTableEntry->FullDllName.Buffer);
            }
            ExFreePool (DataTableEntry);

            DataTableEntry = NULL;

            goto return1;
        }

        //
        // Reinitialize the flags and update the loaded imports.
        //

        DataTableEntry->Flags |= (LDRP_SYSTEM_MAPPED | LDRP_ENTRY_PROCESSED | LDRP_MM_LOADED);
        DataTableEntry->Flags &= ~LDRP_LOAD_IN_PROGRESS;
        DataTableEntry->LoadedImports = LoadedImports;

        MiApplyDriverVerifier (DataTableEntry, NULL);

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

            //
            // Attempt to lookup the win32k system service table. If the
            // service table is found, then compact the table into an array
            // of relative 32-bit pointers.
            //

#if defined(_AMD64_)

            PULONG Limit;
            BOOLEAN Match;
            UNICODE_STRING Name;
            NTSTATUS Status1;
            NTSTATUS Status2;
            PVOID Table;

            RtlInitUnicodeString (&Name, L"win32k.sys");
            Match = RtlEqualUnicodeString (&DataTableEntry->BaseDllName,
                                           &Name,
                                           TRUE);

            if (Match == TRUE) {
                Status1 = LookupEntryPoint (DataTableEntry->DllBase,
                                            "W32pServiceTable",
                                            &Table);
    
                Status2 = LookupEntryPoint (DataTableEntry->DllBase,
                                            "W32pServiceLimit",
                                            &Limit);

                if (NT_SUCCESS(Status1) && NT_SUCCESS(Status2)) {
                    KeCompactServiceTable (Table, *Limit, TRUE);

                } else {
                    DbgBreakPoint ();
                }
            }

#endif

            //
            // The session image was mapped entirely read-write on initial
            // creation.  Now that the relocations (if any), image
            // resolution and import table updates are complete, correct
            // permissions can be applied.
            //
            // Make the entire image copy on write.  The subsequent call
            // to MiWriteProtectSystemImage will make various portions
            // readonly.  Then apply flip various subsections into global
            // shared mode if their attributes specify it.
            //

            PointerPte = MiGetPteAddress (DataTableEntry->DllBase);

            MiSetSystemCodeProtection (PointerPte,
                                       PointerPte + NumberOfPtes - 1,
                                       MM_EXECUTE_WRITECOPY);
        }

        MiWriteProtectSystemImage (DataTableEntry->DllBase);

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
            MiSessionProcessGlobalSubsections (DataTableEntry);
        }

        if (PsImageNotifyEnabled) {
            IMAGE_INFO ImageInfo;

            ImageInfo.Properties = 0;
            ImageInfo.ImageAddressingMode = IMAGE_ADDRESSING_MODE_32BIT;
            ImageInfo.SystemModeImage = TRUE;
            ImageInfo.ImageSize = DataTableEntry->SizeOfImage;
            ImageInfo.ImageBase = *ImageBaseAddress;
            ImageInfo.ImageSelector = 0;
            ImageInfo.ImageSectionNumber = 0;

            PsCallImageNotifyRoutines(ImageFileName, (HANDLE)NULL, &ImageInfo);
        }

        if (MiCacheImageSymbols (*ImageBaseAddress)) {

            //
            //  TEMP TEMP TEMP rip out when debugger converted
            //

            ANSI_STRING AnsiName;
            UNICODE_STRING UnicodeName;

            //
            //  \SystemRoot is 11 characters in length
            //
            if (PrefixedImageName.Length > (11 * sizeof (WCHAR )) &&
                !_wcsnicmp (PrefixedImageName.Buffer, (const PUSHORT)L"\\SystemRoot", 11)) {
                UnicodeName = PrefixedImageName;
                UnicodeName.Buffer += 11;
                UnicodeName.Length -= (11 * sizeof (WCHAR));
                sprintf (NameBuffer, "%ws%wZ", &SharedUserData->NtSystemRoot[2], &UnicodeName);
            }
            else {
                sprintf (NameBuffer, "%wZ", &BaseName);
            }
            RtlInitString (&AnsiName, NameBuffer);
            DbgLoadImageSymbols (&AnsiName,
                                 *ImageBaseAddress,
                                 (ULONG_PTR) -1);

            DataTableEntry->Flags |= LDRP_DEBUG_SYMBOLS_LOADED;
        }
    }

    //
    // Flush the instruction cache on all systems in the configuration.
    //

    KeSweepIcache (TRUE);
    *ImageHandle = DataTableEntry;
    Status = STATUS_SUCCESS;

    //
    // Session images are always paged by default.
    // Non-session images get paged now.
    //

    if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
        MI_LOG_SESSION_DATA_START (DataTableEntry);
    }
    else if ((LoadFlags & MM_LOAD_IMAGE_AND_LOCKDOWN) == 0) {

        ASSERT (SectionPointer == NULL);

        MiEnablePagingOfDriver (DataTableEntry);
    }

return1:

    if (!NT_SUCCESS(Status)) {

        if (IssueUnloadOnFailure == TRUE) {

            if (DataTableEntry == NULL) {

                RtlZeroMemory (&TempDataTableEntry, sizeof (KLDR_DATA_TABLE_ENTRY));

                DataTableEntry = &TempDataTableEntry;

                DataTableEntry->DllBase = *ImageBaseAddress;
                DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;
                DataTableEntry->LoadCount = 1;
                DataTableEntry->LoadedImports = LoadedImports;

                if ((AlreadyOpen == FALSE) && (SectionPointer != NULL)) {
                    DataTableEntry->SectionPointer = (PVOID) SectionPointer;
                }
            }
#if DBG
            else {

                //
                // If DataTableEntry is NULL, then we are unloading before one
                // got created.  Once a LDR_DATA_TABLE_ENTRY is created, the
                // load cannot fail, so if it exists here, at least one other
                // session contains this image as well.
                //

                ASSERT (DataTableEntry->LoadCount > 1);
            }
#endif

            MmUnloadSystemImage ((PVOID)DataTableEntry);
        }

        if ((AlreadyOpen == FALSE) && (SectionPointer != NULL)) {

            //
            // This is needed for failed win32k.sys loads or any session's
            // load of the first instance of a driver.
            //

            ObDereferenceObject (SectionPointer);
        }
    }

    if (LoadLockOwned == TRUE) {
        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;
    }

    if (FileHandle) {
        ZwClose (FileHandle);
    }

    if (!NT_SUCCESS(Status)) {

        UNICODE_STRING ErrorStrings[4];
        ULONG UniqueErrorValue;
        ULONG StringSize;
        ULONG StringCount;
        ANSI_STRING AnsiString;
        UNICODE_STRING ProcedureName = {0};
        UNICODE_STRING DriverName;
        ULONG i;
        PWCHAR temp;
        PWCHAR ptr;
        ULONG PacketSize;
        SIZE_T length;
        PIO_ERROR_LOG_PACKET ErrLog;

        //
        // The driver could not be loaded - log an event with the details.
        //

        StringSize = 0;

        *(&ErrorStrings[0]) = *ImageFileName;
        StringSize += (ImageFileName->Length + sizeof(UNICODE_NULL));
        StringCount = 1;

        UniqueErrorValue = 0;

        PrintableMissingDriverName = (PWSTR)((ULONG_PTR)MissingDriverName & ~0x1);
        if ((Status == STATUS_DRIVER_ORDINAL_NOT_FOUND) ||
            (Status == STATUS_DRIVER_ENTRYPOINT_NOT_FOUND) ||
            (Status == STATUS_OBJECT_NAME_NOT_FOUND) ||
            (Status == STATUS_PROCEDURE_NOT_FOUND)) {

            ErrorStrings[1].Buffer = L"cannot find";
            length = wcslen(ErrorStrings[1].Buffer) * sizeof(WCHAR);
            ErrorStrings[1].Length = (USHORT) length;
            StringSize += (ULONG)(length + sizeof (UNICODE_NULL));
            StringCount += 1;

            RtlInitUnicodeString (&DriverName, PrintableMissingDriverName);

            StringSize += (DriverName.Length + sizeof(UNICODE_NULL));
            StringCount += 1;
            *(&ErrorStrings[2]) = *(&DriverName);

            if ((ULONG_PTR)MissingProcedureName & ~((ULONG_PTR) (X64K-1))) {

                //
                // If not an ordinal, pass as a Unicode string
                //

                RtlInitAnsiString (&AnsiString, MissingProcedureName);
                if (NT_SUCCESS (RtlAnsiStringToUnicodeString (&ProcedureName, &AnsiString, TRUE))) {
                    StringSize += (ProcedureName.Length + sizeof(UNICODE_NULL));
                    StringCount += 1;
                    *(&ErrorStrings[3]) = *(&ProcedureName);
                }
                else {
                    goto GenericError;
                }
            }
            else {

                //
                // Just pass ordinal values as is in the UniqueErrorValue.
                //

                UniqueErrorValue = PtrToUlong (MissingProcedureName);
            }
        }
        else {

GenericError:

            UniqueErrorValue = (ULONG) Status;

            if (MmIsRetryIoStatus(Status)) {

                //
                // Coalesce the various low memory values into just one.
                //

                Status = STATUS_INSUFFICIENT_RESOURCES;

            }
            else {

                //
                // Ideally, the real failing status should be returned. However,
                // we need to do a full release worth of testing (ie Longhorn)
                // before making that change.
                //

                Status = STATUS_DRIVER_UNABLE_TO_LOAD;
            }

            ErrorStrings[1].Buffer = L"failed to load";
            length = wcslen(ErrorStrings[1].Buffer) * sizeof(WCHAR);
            ErrorStrings[1].Length = (USHORT) length;
            StringSize += (ULONG)(length + sizeof (UNICODE_NULL));
            StringCount += 1;
        }

        PacketSize = sizeof (IO_ERROR_LOG_PACKET) + StringSize;

        //
        // Enforce I/O manager interface (ie: UCHAR) size restrictions.
        //

        if (PacketSize < MAXUCHAR) {

            ErrLog = IoAllocateGenericErrorLogEntry ((UCHAR)PacketSize);

            if (ErrLog != NULL) {

                //
                // Fill it in and write it out as a single string.
                //

                ErrLog->ErrorCode = STATUS_LOG_HARD_ERROR;
                ErrLog->FinalStatus = Status;
                ErrLog->UniqueErrorValue = UniqueErrorValue;

                ErrLog->StringOffset = (USHORT) sizeof (IO_ERROR_LOG_PACKET);

                temp = (PWCHAR) ((PUCHAR) ErrLog + ErrLog->StringOffset);

                for (i = 0; i < StringCount; i += 1) {

                    ptr = ErrorStrings[i].Buffer;

                    RtlCopyMemory (temp, ptr, ErrorStrings[i].Length);
                    temp += (ErrorStrings[i].Length / sizeof (WCHAR));

                    *temp = L' ';
                    temp += 1;
                }

                *(temp - 1) = UNICODE_NULL;
                ErrLog->NumberOfStrings = 1;

                IoWriteErrorLogEntry (ErrLog);
            }
        }

        //
        // The only way this pointer has the low bit set is if we are expected
        // to free the pool containing the name.  Typically the name points at
        // a loaded module list entry and so no one has to free it and in this
        // case the low bit will NOT be set.  If the module could not be found
        // and was therefore not loaded, then we left a piece of pool around
        // containing the name since there is no loaded module entry already -
        // this must be released now.
        //

        if ((ULONG_PTR)MissingDriverName & 0x1) {
            ExFreePool (PrintableMissingDriverName);
        }

        if (ProcedureName.Buffer != NULL) {
            RtlFreeUnicodeString (&ProcedureName);
        }
        ExFreePool (NameBuffer);
        return Status;
    }

return2:

    if (LoadLockOwned == TRUE) {
        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;
    }

    if (NamePrefix) {
        ExFreePool (PrefixedImageName.Buffer);
    }

    ExFreePool (NameBuffer);

    return Status;
}

VOID
MiReturnFailedSessionPages (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

/*++

Routine Description:

    This routine is a nonpaged wrapper which undoes session image loads
    that failed midway through reading in the pages.

Arguments:

    PointerPte - Supplies the starting PTE for the range to unload.

    LastPte - Supplies the ending PTE for the range to unload.

Return Value:

    None.

--*/

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {
        if (PointerPte->u.Hard.Valid == 1) {

            //
            // Delete the page.
            //

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

            //
            // Set the pointer to PTE as empty so the page
            // is deleted when the reference count goes to zero.
            //

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

            MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementShareCount (Pfn1, PageFrameIndex);

            MI_WRITE_ZERO_PTE (PointerPte);
        }
        PointerPte += 1;
    }

    UNLOCK_PFN (OldIrql);
}


NTSTATUS
MiLoadImageSection (
    IN OUT PSECTION *InputSectionPointer,
    OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING ImageFileName,
    IN ULONG LoadInSessionSpace,
    IN PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry
    )

/*++

Routine Description:

    This routine loads the specified image into the kernel part of the
    address space.

Arguments:

    InputSectionPointer - Supplies the section object for the image.  This may
                          be replaced by a pagefile-backed section (for
                          protection purposes) for session images if it is
                          determined that the image section is concurrently
                          being accessed by a user app.

    ImageBaseAddress - Returns the address that the image header is at.

    ImageFileName - Supplies the full path name (including the image name)
                    of the image to load.

    LoadInSessionSpace - Supplies nonzero to load this image in session space.
                         Each session gets a different copy of this driver with
                         pages shared as much as possible via copy on write.

                         Supplies zero if this image should be loaded in global
                         space.

    FoundDataTableEntry - Supplies the loader data table entry if the image
                          has already been loaded.  This can only happen for
                          session space.  It means this driver has already
                          been loaded into a different session, so this session
                          still needs to map it.

Return Value:

    Status of the operation.

--*/

{
    KAPC_STATE ApcState;
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ActualPagesUsed;
    PSECTION SectionPointer;
    PSECTION NewSectionPointer;
    PVOID OpaqueSession;
    PMMPTE ProtoPte;
    PMMPTE LastProto;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PEPROCESS Process;
    PEPROCESS TargetProcess;
    ULONG NumberOfPtes;
    MMPTE PteContents;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID UserVa;
    PVOID SystemVa;
    NTSTATUS Status;
    NTSTATUS ExceptionStatus;
    PVOID Base;
    ULONG_PTR ViewSize;
    LARGE_INTEGER SectionOffset;
    LOGICAL LoadSymbols;
    PVOID BaseAddress;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;

    PAGED_CODE();

#if !DBG
    UNREFERENCED_PARAMETER (ImageFileName);
#endif

    SectionPointer = *InputSectionPointer;

    NumberOfPtes = SectionPointer->Segment->TotalNumberOfPtes;

    if (LoadInSessionSpace != 0) {

        //
        // Allocate a unique systemwide session space virtual address for
        // the driver.
        //

        if (FoundDataTableEntry == NULL) {

            Status = MiSessionWideReserveImageAddress (SectionPointer,
                                                       &BaseAddress,
                                                       &NewSectionPointer);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

            if (NewSectionPointer != NULL) {
                SectionPointer = NewSectionPointer;
                *InputSectionPointer = NewSectionPointer;
            }
        }
        else {
            BaseAddress = FoundDataTableEntry->DllBase;
        }

#if DBG
        if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
                "MM: MiLoadImageSection: Image %wZ, BasedAddress 0x%p, Allocated Session BaseAddress 0x%p\n",
                ImageFileName,
                SectionPointer->Segment->BasedAddress,
                BaseAddress);
        }
#endif

        //
        // Session images are mapped backed directly by the file image.
        // All pristine pages of the image will be shared across all
        // sessions, with each page treated as copy-on-write on first write.
        //
        // NOTE: This makes the file image "busy", a different behavior
        // as normal kernel drivers are backed by the paging file only.
        //

        Status = MiShareSessionImage (BaseAddress, SectionPointer);

        if (!NT_SUCCESS (Status)) {
            MiRemoveImageSessionWide (FoundDataTableEntry,
                                      BaseAddress,
                                      NumberOfPtes << PAGE_SHIFT);
            return Status;
        }

        *ImageBaseAddress = BaseAddress;

        return Status;
    }

    ASSERT (FoundDataTableEntry == NULL);

    //
    // Calculate the number of pages required to load this image.
    //
    // Start out by charging for everything and subtract out any gap
    // pages after the image loads successfully.
    //

    PagesRequired = NumberOfPtes;
    ActualPagesUsed = 0;

    //
    // See if ample pages exist to load this image.
    //

    if (MiChargeResidentAvailable (PagesRequired, MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE) == FALSE) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Reserve the necessary system address space.
    //

    FirstPte = MiReserveSystemPtes (NumberOfPtes, SystemPteSpace);

    if (FirstPte == NULL) {
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PointerPte = FirstPte;
    SystemVa = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MiChargeCommitment (PagesRequired, NULL) == FALSE) {
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1);
        MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_DRIVER_PAGES, PagesRequired);

    InterlockedExchangeAdd ((PLONG)&MmDriverCommit, (LONG) PagesRequired);

    //
    // Map a view into the user portion of the address space.
    //

    Process = PsGetCurrentProcess ();

    //
    // Since callees are not always in the context of the system process,
    // attach here when necessary to guarantee the driver load occurs in a
    // known safe address space to prevent security holes.
    //

    OpaqueSession = NULL;

    KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);

    ZERO_LARGE (SectionOffset);
    Base = NULL;
    ViewSize = 0;

    if (NtGlobalFlag & FLG_ENABLE_KDEBUG_SYMBOL_LOAD) {
        LoadSymbols = TRUE;
        NtGlobalFlag &= ~FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
    }
    else {
        LoadSymbols = FALSE;
    }

    TargetProcess = PsGetCurrentProcess ();

    Status = MmMapViewOfSection (SectionPointer,
                                 TargetProcess,
                                 &Base,
                                 0,
                                 0,
                                 &SectionOffset,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_EXECUTE);

    if (LoadSymbols) {
        NtGlobalFlag |= FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
    }

    if (Status == STATUS_IMAGE_MACHINE_TYPE_MISMATCH) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
    }

    if (!NT_SUCCESS(Status)) {

        KeUnstackDetachProcess (&ApcState);

        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE2);

        MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);
        MiReturnCommitment (PagesRequired);

        return Status;
    }

    //
    // Allocate a physical page(s) and copy the image data.
    // Note for session drivers, the physical pages have already
    // been allocated and just data copying is done here.
    //

    ControlArea = SectionPointer->Segment->ControlArea;

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    ASSERT (Subsection->SubsectionBase != NULL);
    ProtoPte = Subsection->SubsectionBase;

    *ImageBaseAddress = SystemVa;

    UserVa = Base;
    TempPte = ValidKernelPte;
    TempPte.u.Long |= MM_PTE_EXECUTE;

    LastProto = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
    LastPte = PointerPte + NumberOfPtes;

    ExceptionStatus = STATUS_SUCCESS;

    while (PointerPte < LastPte) {

        if (ProtoPte >= LastProto) {

            //
            // Handle extended subsections.
            //

            Subsection = Subsection->NextSubsection;
            ProtoPte = Subsection->SubsectionBase;
            LastProto = &Subsection->SubsectionBase[
                                        Subsection->PtesInSubsection];
        }

        PteContents = *ProtoPte;
        if ((PteContents.u.Hard.Valid == 1) ||
            (PteContents.u.Soft.Protection != MM_NOACCESS)) {

            ActualPagesUsed += 1;

            PageFrameIndex = MiAllocatePfn (PointerPte, MM_EXECUTE);

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);

            ASSERT (MI_PFN_ELEMENT (PageFrameIndex)->u1.WsIndex == 0);

            try {

                RtlCopyMemory (SystemVa, UserVa, PAGE_SIZE);

            } except (MiMapCacheExceptionFilter (&ExceptionStatus,
                                                 GetExceptionInformation())) {

                //
                // An exception occurred, unmap the view and
                // return the error to the caller.
                //

#if DBG
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                    "MiLoadImageSection: Exception 0x%x copying driver SystemVa 0x%p, UserVa 0x%p\n",ExceptionStatus,SystemVa,UserVa);
#endif

                MiReturnFailedSessionPages (FirstPte, PointerPte);

                MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                                 MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE3);

                MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);

                MiReturnCommitment (PagesRequired);

                Status = MiUnmapViewOfSection (TargetProcess, Base, 0);

                ASSERT (NT_SUCCESS (Status));

                //
                // Purge the section as we want these pages on the freelist
                // instead of at the tail of standby, as we're completely
                // done with the section.  This is because other valuable
                // standby pages end up getting reused (especially during
                // bootup) when the section pages are the ones that really
                // will never be referenced again.
                //
                // Note this isn't done for session images as they're
                // inpaged directly from the filesystem via the section.
                //

                MmPurgeSection (ControlArea->FilePointer->SectionObjectPointer,
                                NULL,
                                0,
                                FALSE);

                KeUnstackDetachProcess (&ApcState);

                return ExceptionStatus;
            }
        }
        else {

            //
            // The PTE is no access.
            //

            MI_WRITE_ZERO_PTE (PointerPte);
        }

        ProtoPte += 1;
        PointerPte += 1;
        SystemVa = ((PCHAR)SystemVa + PAGE_SIZE);
        UserVa = ((PCHAR)UserVa + PAGE_SIZE);
    }

    Status = MiUnmapViewOfSection (TargetProcess, Base, 0);
    ASSERT (NT_SUCCESS (Status));

    //
    // Purge the section as we want these pages on the freelist instead of
    // at the tail of standby, as we're completely done with the section.
    // This is because other valuable standby pages end up getting reused
    // (especially during bootup) when the section pages are the ones that
    // really will never be referenced again.
    //

    MmPurgeSection (ControlArea->FilePointer->SectionObjectPointer,
                    NULL,
                    0,
                    FALSE);

    KeUnstackDetachProcess (&ApcState);

    //
    // Return any excess resident available and commit.
    //

    if (PagesRequired != ActualPagesUsed) {
        ASSERT (PagesRequired > ActualPagesUsed);
        PagesRequired -= ActualPagesUsed;

        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                        MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE_EXCESS);

        MiReturnCommitment (PagesRequired);
    }

    return Status;
}

VOID
MmFreeDriverInitialization (
    IN PVOID ImageHandle
    )

/*++

Routine Description:

    This routine removes the pages that relocate and debug information from
    the address space of the driver.

    NOTE:  This routine looks at the last sections defined in the image
           header and if that section is marked as DISCARDABLE in the
           characteristics, it is removed from the image.  This means
           that all discardable sections at the end of the driver are
           deleted.

Arguments:

    SectionObject - Supplies the section object for the image.

Return Value:

    None.

--*/

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER NumberOfPtes;
    PVOID Base;
    PVOID StartVa;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    PIMAGE_SECTION_HEADER FoundSection;
    PFN_NUMBER PagesDeleted;

    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)ImageHandle;
    Base = DataTableEntry->DllBase;

    ASSERT (MI_IS_SESSION_ADDRESS (Base) == FALSE);

    NumberOfPtes = DataTableEntry->SizeOfImage >> PAGE_SHIFT;
    LastPte = MiGetPteAddress (Base) + NumberOfPtes;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    NtSection += NtHeaders->FileHeader.NumberOfSections;

    FoundSection = NULL;
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {
        NtSection -= 1;
        if ((NtSection->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0) {
            FoundSection = NtSection;
        }
        else {

            //
            // There was a non discardable section between this
            // section and the last non discardable section, don't
            // discard this section and don't look any more.
            //

            break;
        }
    }

    if (FoundSection != NULL) {

        StartVa = (PVOID) (ROUND_TO_PAGES (
                            (PCHAR)Base + FoundSection->VirtualAddress));

        PointerPte = MiGetPteAddress (StartVa);

        NumberOfPtes = (PFN_NUMBER)(LastPte - PointerPte);

        if (NumberOfPtes != 0) {

            if (MI_IS_PHYSICAL_ADDRESS (StartVa)) {

                //
                // Don't free the INIT code for a driver mapped by large pages
                // because if it unloads later, we'd have to deal with
                // discontiguous ranges of pages to free.
                //

                return;
            }
            else {
                PagesDeleted = MiDeleteSystemPageableVm (PointerPte,
                                                        NumberOfPtes,
                                                        MI_DELETE_FLUSH_TB,
                                                        NULL);
            }

            MI_INCREMENT_RESIDENT_AVAILABLE (PagesDeleted,
                                            MM_RESAVAIL_FREE_DRIVER_INITIALIZATION);

            MiReturnCommitment (PagesDeleted);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_INIT_CODE, PagesDeleted);

            InterlockedExchangeAdd ((PLONG)&MmDriverCommit,
                                    (LONG) (0 - PagesDeleted));
        }
    }

    return;
}

LOGICAL
MiChargeResidentAvailable (
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Id
    )

/*++

Routine Description:

    This routine is a nonpaged wrapper to charge resident available pages.

Arguments:

    NumberOfPages - Supplies the number of pages to charge.

    Id - Supplies a tracking ID for debugging purposes.

Return Value:

    TRUE if the pages were charged, FALSE if not.

--*/

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    if (MI_NONPAGEABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)NumberOfPages) {
        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, Id);

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

VOID
MiFlushPteListFreePfns (
    IN PVOID *VaFlushList,
    IN ULONG FlushCount
    )

/*++

Routine Description:

    This routine flushes all the PTEs in the PTE flush list.
    If the list has overflowed, the entire TB is flushed.

    This routine also decrements the sharecounts on the relevant PFNs.

Arguments:

    VaFlushList - Supplies a pointer to the list to be flushed.

    Count - Supplies a count of entries to be flushed.

Return Value:

    None.

Environment:

    Kernel mode, working set mutex held, APC_LEVEL.

--*/

{
    ULONG i;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE PreviousPte;
    KIRQL OldIrql;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    ASSERT (FlushCount != 0);

    //
    // Put the PTEs in transition and decrement the number of
    // valid PTEs within the containing page table page.  Note
    // that for a private page, the page table page is still
    // needed because the page is in transition.
    //

    LOCK_PFN (OldIrql);

    for (i = 0; i < FlushCount; i += 1) {

        PointerPte = MiGetPteAddress (VaFlushList[i]);

        TempPte = *PointerPte;
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn = MI_PFN_ELEMENT (PageFrameIndex);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                      Pfn->OriginalPte.u.Soft.Protection);

        PreviousPte = *PointerPte;

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn);

        MiDecrementShareCount (Pfn, PageFrameIndex);
    }

    //
    // Flush the relevant entries from the translation buffer.
    //

    if (FlushCount == 1) {
        MI_FLUSH_SINGLE_TB (VaFlushList[0], TRUE);
    }
    else if (FlushCount < MM_MAXIMUM_FLUSH_COUNT) {
        MI_FLUSH_MULTIPLE_TB (FlushCount, &VaFlushList[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (0x19);
    }

    UNLOCK_PFN (OldIrql);

    return;
}

VOID
MiEnablePagingOfDriver (
    IN PVOID ImageHandle
    )

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PVOID Base;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER FoundSection;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;

    //
    // Don't page kernel mode code if customer does not want it paged.
    //

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

    //
    // If the driver has pageable code, make it paged.
    //

    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY) ImageHandle;
    Base = DataTableEntry->DllBase;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)((PCHAR)NtHeaders +
#if defined (_WIN64)
                        FIELD_OFFSET (IMAGE_NT_HEADERS64, OptionalHeader)
#else
                        FIELD_OFFSET (IMAGE_NT_HEADERS32, OptionalHeader)
#endif
                        );

    FoundSection = IMAGE_FIRST_SECTION (NtHeaders);

    i = NtHeaders->FileHeader.NumberOfSections;

    PointerPte = NULL;

    //
    // Initializing LastPte is not needed for correctness, but without it
    // the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    LastPte = NULL;

    while (i > 0) {
#if DBG
            if ((*(PULONG)FoundSection->Name == 'tini') ||
                (*(PULONG)FoundSection->Name == 'egap')) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                    "driver %wZ has lower case sections (init or pagexxx)\n",
                    &DataTableEntry->FullDllName);
            }
#endif

        //
        // Mark as pageable any section which starts with the
        // first 4 characters PAGE or .eda (for the .edata section).
        //

        if ((*(PULONG)FoundSection->Name == 'EGAP') ||
           (*(PULONG)FoundSection->Name == 'ade.')) {

            //
            // This section is pageable, save away the start and end.
            //

            if (PointerPte == NULL) {

                //
                // Previous section was NOT pageable, get the start address.
                //

                PointerPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                   (PCHAR)Base + FoundSection->VirtualAddress)));
            }

            //
            // Generally, SizeOfRawData is larger than VirtualSize for each
            // section because it includes the padding to get to the subsection
            // alignment boundary.  However, if the image is linked with
            // subsection alignment == native page alignment, the linker will
            // have VirtualSize be much larger than SizeOfRawData because it
            // will account for all the bss.
            //
    
            Span = FoundSection->SizeOfRawData;
    
            if (Span < FoundSection->Misc.VirtualSize) {
                Span = FoundSection->Misc.VirtualSize;
            }

            LastPte = MiGetPteAddress ((PCHAR)Base +
                                       FoundSection->VirtualAddress +
                                       (OptionalHeader->SectionAlignment - 1) +
                                       Span - PAGE_SIZE);

        }
        else {

            //
            // This section is not pageable, if the previous section was
            // pageable, enable it.
            //

            if (PointerPte != NULL) {
                MiSetPagingOfDriver (PointerPte, LastPte);
                PointerPte = NULL;
            }
        }
        i -= 1;
        FoundSection += 1;
    }
    if (PointerPte != NULL) {
        MiSetPagingOfDriver (PointerPte, LastPte);
    }
}


VOID
MiSetPagingOfDriver (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

/*++

Routine Description:

    This routine marks the specified range of PTEs as pageable.

Arguments:

    PointerPte - Supplies the starting PTE.

    LastPte - Supplies the ending PTE.

Return Value:

    None.

Environment:

    Kernel Mode, IRQL of APC_LEVEL or below.

    This routine could be made PAGELK but it is a high frequency routine
    so it is actually better to keep it nonpaged to avoid bringing in the
    entire PAGELK section.

--*/

{
    PVOID Base;
    PFN_NUMBER PageCount;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    PETHREAD Thread;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];

    PAGED_CODE ();

    Base = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MI_IS_PHYSICAL_ADDRESS (Base)) {

        //
        // No need to lock physical addresses.
        //

        return;
    }

    ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (Base) == FALSE);

    PageCount = 0;
    FlushCount = 0;
    Thread = PsGetCurrentThread ();

    LOCK_WORKING_SET (Thread, &MmSystemCacheWs);

    while (PointerPte <= LastPte) {

        //
        // Check to make sure this PTE has not already been
        // made pageable (or deleted).  It is pageable if it
        // is not valid, or if the PFN database wsindex element
        // is non zero.
        //

        if (PointerPte->u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn->u2.ShareCount == 1);

            //
            // If the wsindex is nonzero, then this page is already pageable
            // and has a WSLE entry.  Ignore it here and let the trimmer
            // take it if memory comes under pressure.
            //

            if (Pfn->u1.WsIndex == 0) {

                //
                // Original PTE may need to be set for drivers loaded
                // via ntldr.
                //

                if (Pfn->OriginalPte.u.Long == 0) {
                    Pfn->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
                    Pfn->OriginalPte.u.Soft.Protection |= MM_EXECUTE;
                }

                VaFlushList[FlushCount] = Base;
                FlushCount += 1;

                if (FlushCount == MM_MAXIMUM_FLUSH_COUNT) {
                    MiFlushPteListFreePfns ((PVOID *)&VaFlushList, FlushCount);
                    FlushCount = 0;
                }

                PageCount += 1;
            }
        }
        Base = (PVOID)((PCHAR)Base + PAGE_SIZE);
        PointerPte += 1;
    }

    if (FlushCount != 0) {
        MiFlushPteListFreePfns ((PVOID *)&VaFlushList, FlushCount);
    }

    UNLOCK_WORKING_SET (Thread, &MmSystemCacheWs);

    if (PageCount != 0) {
        InterlockedExchangeAdd (&MmTotalSystemDriverPages, (LONG) PageCount);

        MI_INCREMENT_RESIDENT_AVAILABLE (PageCount,
                                         MM_RESAVAIL_FREE_SET_DRIVER_PAGING);
    }
}


PVOID
MmPageEntireDriver (
    __in PVOID AddressWithinSection
    )

/*++

Routine Description:

    This routine allows a driver to page out all of its code and
    data regardless of the attributes of the various image sections.

    Note, this routine can be called multiple times with no
    intervening calls to MmResetDriverPaging.

Arguments:

    AddressWithinSection - Supplies an address within the driver, e.g.
                           DriverEntry.

Return Value:

    Base address of driver.

Environment:

    Kernel mode, APC_LEVEL or below.

--*/

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PVOID BaseAddress;

    PAGED_CODE();

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, FALSE);

    if (DataTableEntry == NULL) {
        return NULL;
    }

    //
    // Don't page kernel mode code if disabled via registry.
    //

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return DataTableEntry->DllBase;
    }

    if (DataTableEntry->SectionPointer != NULL) {

        //
        // Driver is mapped as an image (ie: session space), this is always
        // pageable.
        //

        ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == TRUE);

        return DataTableEntry->DllBase;
    }

    //
    // Force any active DPCs to clear the system before we page the driver.
    //

    KeFlushQueuedDpcs ();

    BaseAddress = DataTableEntry->DllBase;
    FirstPte = MiGetPteAddress (BaseAddress);
    LastPte = (FirstPte - 1) + (DataTableEntry->SizeOfImage >> PAGE_SHIFT);

    ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == FALSE);

    MiSetPagingOfDriver (FirstPte, LastPte);

    return BaseAddress;
}


VOID
MmResetDriverPaging (
    __in PVOID AddressWithinSection
    )

/*++

Routine Description:

    This routines resets the driver paging to what the image specified.
    Hence image sections such as the IAT, .text, .data will be locked
    down in memory.

    Note, there is no requirement that MmPageEntireDriver was called.

Arguments:

    AddressWithinSection - Supplies an address within the driver, e.g.
                           DriverEntry.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL or below.

--*/

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PVOID Base;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER FoundSection;

    PAGED_CODE();

    //
    // Don't page kernel mode code if disabled via registry.
    //

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

    if (MI_IS_PHYSICAL_ADDRESS (AddressWithinSection)) {
        return;
    }

    //
    // If the driver has pageable code, make it paged.
    //

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, FALSE);

    if (DataTableEntry->SectionPointer != NULL) {

        //
        // Driver is mapped by image hence already paged.
        //

        ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == TRUE);

        return;
    }

    Base = DataTableEntry->DllBase;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    FoundSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    i = NtHeaders->FileHeader.NumberOfSections;
    PointerPte = NULL;

    while (i > 0) {
#if DBG
            if ((*(PULONG)FoundSection->Name == 'tini') ||
                (*(PULONG)FoundSection->Name == 'egap')) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                    "driver %wZ has lower case sections (init or pagexxx)\n",
                    &DataTableEntry->FullDllName);
            }
#endif

        //
        // Don't lock down code for sections marked as discardable or
        // sections marked with the first 4 characters PAGE or .eda
        // (for the .edata section) or INIT.
        //

        if (((FoundSection->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0) ||
           (*(PULONG)FoundSection->Name == 'EGAP') ||
           (*(PULONG)FoundSection->Name == 'ade.') ||
           (*(PULONG)FoundSection->Name == 'TINI')) {

            NOTHING;

        }
        else {

            //
            // This section is non-pageable.
            //

            PointerPte = MiGetPteAddress (
                                   (PCHAR)Base + FoundSection->VirtualAddress);

            //
            // Generally, SizeOfRawData is larger than VirtualSize for each
            // section because it includes the padding to get to the subsection
            // alignment boundary.  However, if the image is linked with
            // subsection alignment == native page alignment, the linker will
            // have VirtualSize be much larger than SizeOfRawData because it
            // will account for all the bss.
            //
    
            Span = FoundSection->SizeOfRawData;
    
            if (Span < FoundSection->Misc.VirtualSize) {
                Span = FoundSection->Misc.VirtualSize;
            }

            LastPte = MiGetPteAddress ((PCHAR)Base +
                                       FoundSection->VirtualAddress +
                                       (Span - 1));

            ASSERT (PointerPte <= LastPte);

            MiLockCode (PointerPte, LastPte, MM_LOCK_BY_NONPAGE);
        }
        i -= 1;
        FoundSection += 1;
    }
    return;
}


VOID
MiClearImports (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )
/*++

Routine Description:

    Free up the import list and clear the pointer.  This stops the
    recursion performed in MiDereferenceImports().

Arguments:

    DataTableEntry - provided for the driver.

Return Value:

    Status of the import list construction operation.

--*/

{
    PAGED_CODE();

    if (DataTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {
        return;
    }

    if (DataTableEntry->LoadedImports == (PVOID)NO_IMPORTS_USED) {
        NOTHING;
    }
    else if (SINGLE_ENTRY(DataTableEntry->LoadedImports)) {
        NOTHING;
    }
    else {
        //
        // free the memory
        //
        ExFreePool ((PVOID)DataTableEntry->LoadedImports);
    }

    //
    // stop the recursion
    //
    DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
}

VOID
MiRememberUnloadedDriver (
    IN PUNICODE_STRING DriverName,
    IN PVOID Address,
    IN ULONG Length
    )

/*++

Routine Description:

    This routine saves information about unloaded drivers so that ones that
    forget to delete lookaside lists or queues can be caught.

Arguments:

    DriverName - Supplies a Unicode string containing the driver's name.

    Address - Supplies the address the driver was loaded at.

    Length - Supplies the number of bytes the driver load spanned.

Return Value:

    None.

--*/

{
    PUNLOADED_DRIVERS Entry;
    ULONG NumberOfBytes;

    if (DriverName->Length == 0) {

        //
        // This is an aborted load and the driver name hasn't been filled
        // in yet.  No need to save it.
        //

        return;
    }

    //
    // Serialization is provided by the caller, so just update the list now.
    // Note the allocations are nonpaged so they can be searched at bugcheck
    // time.
    //

    if (MmUnloadedDrivers == NULL) {
        NumberOfBytes = MI_UNLOADED_DRIVERS * sizeof (UNLOADED_DRIVERS);

        MmUnloadedDrivers = (PUNLOADED_DRIVERS)ExAllocatePoolWithTag (NonPagedPool,
                                                                      NumberOfBytes,
                                                                      'TDmM');
        if (MmUnloadedDrivers == NULL) {
            return;
        }
        RtlZeroMemory (MmUnloadedDrivers, NumberOfBytes);
        MmLastUnloadedDriver = 0;
    }
    else if (MmLastUnloadedDriver >= MI_UNLOADED_DRIVERS) {
        MmLastUnloadedDriver = 0;
    }

    Entry = &MmUnloadedDrivers[MmLastUnloadedDriver];

    //
    // Free the old entry as we recycle into the new.
    //

    RtlFreeUnicodeString (&Entry->Name);

    Entry->Name.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                DriverName->Length,
                                                'TDmM');

    if (Entry->Name.Buffer == NULL) {
        Entry->Name.MaximumLength = 0;
        Entry->Name.Length = 0;
        MiUnloadsSkipped += 1;
        return;
    }

    RtlCopyMemory(Entry->Name.Buffer, DriverName->Buffer, DriverName->Length);
    Entry->Name.Length = DriverName->Length;
    Entry->Name.MaximumLength = DriverName->MaximumLength;

    Entry->StartAddress = Address;
    Entry->EndAddress = (PVOID)((PCHAR)Address + Length);

    KeQuerySystemTime (&Entry->CurrentTime);

    MiTotalUnloads += 1;
    MmLastUnloadedDriver += 1;
}

PUNICODE_STRING
MmLocateUnloadedDriver (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This routine attempts to find the specified virtual address in the
    unloaded driver list.

Arguments:

    VirtualAddress - Supplies a virtual address that might be within a driver
                     that has already unloaded.

Return Value:

    A pointer to a Unicode string containing the unloaded driver's name.

Environment:

    Kernel mode, bugcheck time.

--*/

{
    PUNLOADED_DRIVERS Entry;
    ULONG i;
    ULONG Index;

    //
    // No serialization is needed because we've crashed.
    //

    if (MmUnloadedDrivers == NULL) {
        return NULL;
    }

    Index = MmLastUnloadedDriver - 1;

    for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1) {
        if (Index >= MI_UNLOADED_DRIVERS) {
            Index = MI_UNLOADED_DRIVERS - 1;
        }
        Entry = &MmUnloadedDrivers[Index];
        if (Entry->Name.Buffer != NULL) {
            if ((VirtualAddress >= Entry->StartAddress) &&
                (VirtualAddress < Entry->EndAddress)) {
                    return &Entry->Name;
            }
        }
        Index -= 1;
    }

    return NULL;
}


NTSTATUS
MmUnloadSystemImage (
    IN PVOID ImageHandle
    )

/*++

Routine Description:

    This routine unloads a previously loaded system image and returns
    the allocated resources.

Arguments:

    ImageHandle - Supplies a pointer to the section object of the
                  image to unload.

Return Value:

    Various NTSTATUS codes.

Environment:

    Kernel mode, APC_LEVEL or below, arbitrary process context.

--*/

{
    PFN_NUMBER PageFrameIndex;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ResidentPages;
    PMMPTE PointerPte;
    PFN_NUMBER NumberOfPtes;
    PFN_NUMBER RoundedNumberOfPtes;
    PVOID BasedAddress;
    SIZE_T NumberOfBytes;
    LOGICAL MustFree;
    SIZE_T CommittedPages;
    LOGICAL ViewDeleted;
    PIMAGE_ENTRY_IN_SESSION DriverImage;
    NTSTATUS Status;
    PSECTION SectionPointer;
    PKTHREAD CurrentThread;

    ViewDeleted = FALSE;
    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)ImageHandle;
    BasedAddress = DataTableEntry->DllBase;

    //
    // Arbitrary process context so prevent suspend APCs now.
    //

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    if (DataTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {

        //
        // Any driver loaded at boot that did not have its import list
        // and LoadCount reconstructed cannot be unloaded because we don't
        // know how many other drivers may be linked to it.
        //

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        return STATUS_SUCCESS;
    }

    ASSERT (DataTableEntry->LoadCount != 0);

    if (MI_IS_SESSION_IMAGE_ADDRESS (BasedAddress)) {

        //
        // A printer driver may be referenced multiple times for the
        // same session space.  Only unload the last reference.
        //

        DriverImage = MiSessionLookupImage (BasedAddress);

        ASSERT (DriverImage);

        ASSERT (DriverImage->ImageCountInThisSession);

        DriverImage->ImageCountInThisSession -= 1;

        if (DriverImage->ImageCountInThisSession != 0) {

            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);

            return STATUS_SUCCESS;
        }

        //
        // The reference count for this image has dropped to zero in this
        // session, so we can delete this session's view of the image.
        //

        NumberOfBytes = DataTableEntry->SizeOfImage;

        //
        // Free the session space taken up by the image, unmapping it from
        // the current VA space - note this does not remove page table pages
        // from the session PageTables[].  Each data page is only freed
        // if there are no other references to it (ie: from any other
        // sessions).
        //

        PointerPte = MiGetPteAddress (BasedAddress);
        LastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)BasedAddress + NumberOfBytes));

        PagesRequired = MiDeleteSystemPageableVm (PointerPte,
                                                 (PFN_NUMBER)(LastPte - PointerPte),
                                                 MI_DELETE_FLUSH_TB,
                                                 &ResidentPages);

        //
        // Note resident available is returned here without waiting for load
        // count to reach zero because it is charged each time a session space
        // driver locks down its code or data regardless of whether it is really
        // the same copy-on-write backing page(s) that some other session has
        // already locked down.
        //

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE);

        SectionPointer = (PSECTION) DataTableEntry->SectionPointer;

        ASSERT (SectionPointer != NULL);
        ASSERT (SectionPointer->Segment->u1.ImageCommitment != 0);

        if (BasedAddress != SectionPointer->Segment->BasedAddress) {
            CommittedPages = SectionPointer->Segment->TotalNumberOfPtes;
        }
        else {
            CommittedPages = SectionPointer->Segment->u1.ImageCommitment;
        }

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_COMMIT_IMAGE_UNLOAD,
            (ULONG)CommittedPages);

        ViewDeleted = TRUE;

        //
        // Return the commitment we took out on the pagefile when the
        // image was allocated.
        //

        MiReturnCommitment (CommittedPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD, CommittedPages);

        //
        // Tell the session space image handler that we are releasing
        // our claim to the image.
        //

        ASSERT (DataTableEntry->LoadCount != 0);

        MiRemoveImageSessionWide (DataTableEntry,
                                  BasedAddress,
                                  DataTableEntry->SizeOfImage);

        ASSERT (MiSessionLookupImage (BasedAddress) == NULL);
    }

    ASSERT (DataTableEntry->LoadCount != 0);

    DataTableEntry->LoadCount -= 1;

    if (DataTableEntry->LoadCount != 0) {

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        return STATUS_SUCCESS;
    }

    if (MmVerifierData.Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) {
        VerifierDeadlockFreePool (DataTableEntry->DllBase, DataTableEntry->SizeOfImage);
    }

    if (DataTableEntry->Flags & LDRP_IMAGE_VERIFYING) {
        MiVerifyingDriverUnloading (DataTableEntry);
    }

    if (MiActiveVerifierThunks != 0) {
        MiVerifierCheckThunks (DataTableEntry);
    }

    //
    // Unload symbols from debugger.
    //

    if (DataTableEntry->Flags & LDRP_DEBUG_SYMBOLS_LOADED) {

        //
        //  TEMP TEMP TEMP rip out when debugger converted
        //

        ANSI_STRING AnsiName;

        Status = RtlUnicodeStringToAnsiString (&AnsiName,
                                               &DataTableEntry->BaseDllName,
                                               TRUE);

        if (NT_SUCCESS (Status)) {
            DbgUnLoadImageSymbols (&AnsiName, BasedAddress, (ULONG)-1);
            RtlFreeAnsiString (&AnsiName);
        }
    }

    //
    // No unload can happen till after Mm has finished Phase 1 initialization.
    // Therefore, large pages are already in effect (if this platform supports
    // it).
    //

    if (ViewDeleted == FALSE) {

        NumberOfPtes = DataTableEntry->SizeOfImage >> PAGE_SHIFT;

        if (MmSnapUnloads) {
            MiRememberUnloadedDriver (&DataTableEntry->BaseDllName,
                                      BasedAddress,
                                      (ULONG)(NumberOfPtes << PAGE_SHIFT));
        }

        if (DataTableEntry->Flags & LDRP_SYSTEM_MAPPED) {

            if (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (BasedAddress))) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPdeAddress (BasedAddress)) + MiGetPteOffset (BasedAddress);

                RoundedNumberOfPtes = MI_ROUND_TO_SIZE (NumberOfPtes,
                                      MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
                MiUnmapLargePages (BasedAddress,
                                   RoundedNumberOfPtes << PAGE_SHIFT);

                //
                // MiFreeContiguousPages is going to return commitment
                // and resident available so don't do it here.
                //

                MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + RoundedNumberOfPtes - 1);
                MiFreeContiguousPages (PageFrameIndex, RoundedNumberOfPtes);
                PagesRequired = NumberOfPtes;
            }
            else {
                PointerPte = MiGetPteAddress (BasedAddress);

                //
                // No explicit TB flush is done here, instead the system PTE
                // management code will handle this in a lazy fashion.
                //

                PagesRequired = MiDeleteSystemPageableVm (PointerPte,
                                                         NumberOfPtes,
                                                         0,
                                                         &ResidentPages);

                //
                // Note that drivers loaded at boot that have not been relocated
                // have no system PTEs or commit charged.
                //

                MiReleaseSystemPtes (PointerPte,
                                     (ULONG)NumberOfPtes,
                                     SystemPteSpace);

                MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

                MiReturnCommitment (PagesRequired);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, PagesRequired);

                InterlockedExchangeAdd (&MmTotalSystemDriverPages,
                    0 - (ULONG)(PagesRequired - ResidentPages));
            }

            if (DataTableEntry->SectionPointer != NULL) {
                InterlockedExchangeAdd ((PLONG)&MmDriverCommit,
                                        (LONG) (0 - PagesRequired));
            }
        }
        else {

            //
            // This must be a boot driver that was not relocated into
            // system PTEs.  If large or super pages are enabled, the
            // image pages must be freed without referencing the
            // non-existent page table pages.  If large/super pages are
            // not enabled, note that system PTEs were not used to map the
            // image and thus, cannot be freed.

            //
            // This is further complicated by the fact that the INIT and/or
            // discardable portions of these images may have already been freed.
            //

            MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPtes,
                                     MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

            MiReturnCommitment (NumberOfPtes);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, NumberOfPtes);
        }
    }

    //
    // Search the loaded module list for the data table entry that describes
    // the DLL that was just unloaded. It is possible an entry is not in the
    // list if a failure occurred at a point in loading the DLL just before
    // the data table entry was generated.
    //

    if (DataTableEntry->InLoadOrderLinks.Flink != NULL) {
        MiProcessLoaderEntry (DataTableEntry, FALSE);
        MustFree = TRUE;
    }
    else {
        MustFree = FALSE;
    }

    //
    // Handle unloading of any dependent DLLs that we loaded automatically
    // for this image.
    //

    MiDereferenceImports ((PLOAD_IMPORTS)DataTableEntry->LoadedImports);

    MiClearImports (DataTableEntry);

    //
    // Free this loader entry.
    //

    if (MustFree == TRUE) {

        if (DataTableEntry->FullDllName.Buffer != NULL) {
            ExFreePool (DataTableEntry->FullDllName.Buffer);
        }

        //
        // Dereference the section object (session images only).
        //

        if (DataTableEntry->SectionPointer != NULL) {
            ObDereferenceObject (DataTableEntry->SectionPointer);
        }

        if (DataTableEntry->PatchInformation) {
            MiRundownHotpatchList ((PVOID)DataTableEntry->PatchInformation);
        }

        ExFreePool (DataTableEntry);
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

    return STATUS_SUCCESS;
}


NTSTATUS
MiBuildImportsForBootDrivers (
    VOID
    )

/*++

Routine Description:

    Construct an import list chain for boot-loaded drivers.
    If this cannot be done for an entry, its chain is set to LOADED_AT_BOOT.

    If a chain can be successfully built, then this driver's DLLs
    will be automatically unloaded if this driver goes away (provided
    no other driver is also using them).  Otherwise, on driver unload,
    its dependent DLLs would have to be explicitly unloaded.

    Note that the incoming LoadCount values are not correct and thus, they
    are reinitialized here.

Arguments:

    None.

Return Value:

    Various NTSTATUS codes.

--*/

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry2;
    PLIST_ENTRY NextEntry2;
    ULONG i;
    ULONG j;
    ULONG ImageCount;
    PVOID *ImageReferences;
    PVOID LastImageReference;
    PULONG_PTR ImportThunk;
    ULONG_PTR BaseAddress;
    ULONG_PTR LastAddress;
    ULONG ImportSize;
    ULONG ImportListSize;
    PLOAD_IMPORTS ImportList;
    LOGICAL UndoEverything;
    PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY HalDataTableEntry;
    UNICODE_STRING KernelString;
    UNICODE_STRING HalString;

    PAGED_CODE();

    ImageCount = 0;

    KernelDataTableEntry = NULL;
    HalDataTableEntry = NULL;

#define KERNEL_NAME L"ntoskrnl.exe"

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

#define HAL_NAME L"hal.dll"

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

    NextEntry = PsLoadedModuleList.Flink;

    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (RtlEqualUnicodeString (&KernelString,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

            KernelDataTableEntry = CONTAINING_RECORD(NextEntry,
                                                     KLDR_DATA_TABLE_ENTRY,
                                                     InLoadOrderLinks);
        }
        else if (RtlEqualUnicodeString (&HalString,
                                        &DataTableEntry->BaseDllName,
                                        TRUE)) {

            HalDataTableEntry = CONTAINING_RECORD(NextEntry,
                                                  KLDR_DATA_TABLE_ENTRY,
                                                  InLoadOrderLinks);
        }

        //
        // Initialize these properly so error recovery is simplified.
        //

        if (DataTableEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL) {
            if ((DataTableEntry == HalDataTableEntry) || (DataTableEntry == KernelDataTableEntry)) {
                DataTableEntry->LoadCount = 1;
            }
            else {
                DataTableEntry->LoadCount = 0;
            }
        }
        else {
            DataTableEntry->LoadCount = 1;
        }

        DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;

        ImageCount += 1;
        NextEntry = NextEntry->Flink;
    }

    if (KernelDataTableEntry == NULL || HalDataTableEntry == NULL) {
        return STATUS_NOT_FOUND;
    }

    ImageReferences = (PVOID *) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                       ImageCount * sizeof (PVOID),
                                                       'TDmM');

    if (ImageReferences == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UndoEverything = FALSE;

    NextEntry = PsLoadedModuleList.Flink;

    for ( ; NextEntry != &PsLoadedModuleList; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        ImportThunk = (PULONG_PTR)RtlImageDirectoryEntryToData (
                                           DataTableEntry->DllBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_IAT,
                                           &ImportSize);

        if (ImportThunk == NULL) {
            DataTableEntry->LoadedImports = NO_IMPORTS_USED;
            continue;
        }

        RtlZeroMemory (ImageReferences, ImageCount * sizeof (PVOID));

        ImportSize /= sizeof(PULONG_PTR);

        BaseAddress = 0;

        //
        // Initializing these locals is not needed for correctness, but
        // without it the compiler cannot compile this code W4 to check
        // for use of uninitialized variables.
        //

        j = 0;
        LastAddress = 0;

        for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {

            //
            // Check the hint first.
            //

            if (BaseAddress != 0) {
                if (*ImportThunk >= BaseAddress && *ImportThunk < LastAddress) {
                    ASSERT (ImageReferences[j]);
                    continue;
                }
            }

            j = 0;
            NextEntry2 = PsLoadedModuleList.Flink;

            while (NextEntry2 != &PsLoadedModuleList) {

                DataTableEntry2 = CONTAINING_RECORD(NextEntry2,
                                                    KLDR_DATA_TABLE_ENTRY,
                                                    InLoadOrderLinks);

                BaseAddress = (ULONG_PTR) DataTableEntry2->DllBase;
                LastAddress = BaseAddress + DataTableEntry2->SizeOfImage;

                if (*ImportThunk >= BaseAddress && *ImportThunk < LastAddress) {
                    ImageReferences[j] = DataTableEntry2;
                    break;
                }

                NextEntry2 = NextEntry2->Flink;
                j += 1;
            }

            if (*ImportThunk < BaseAddress || *ImportThunk >= LastAddress) {
                if (*ImportThunk) {
#if DBG
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MM: broken import linkage %p %p %p\n",
                        DataTableEntry,
                        ImportThunk,
                        *ImportThunk);
                    DbgBreakPoint ();
#endif
                    UndoEverything = TRUE;
                    goto finished;
                }

                BaseAddress = 0;
            }
        }

        ImportSize = 0;

        //
        // Initializing LastImageReference is not needed for correctness, but
        // without it the compiler cannot compile this code W4 to check
        // for use of uninitialized variables.
        //

        LastImageReference = NULL;

        for (i = 0; i < ImageCount; i += 1) {

            if ((ImageReferences[i] != NULL) &&
                (ImageReferences[i] != KernelDataTableEntry) &&
                (ImageReferences[i] != HalDataTableEntry)) {

                    LastImageReference = ImageReferences[i];
                    ImportSize += 1;
            }
        }

        if (ImportSize == 0) {
            DataTableEntry->LoadedImports = NO_IMPORTS_USED;
        }
        else if (ImportSize == 1) {
            DataTableEntry->LoadedImports = POINTER_TO_SINGLE_ENTRY (LastImageReference);
            ((PKLDR_DATA_TABLE_ENTRY)LastImageReference)->LoadCount += 1;
        }
        else {
            ImportListSize = ImportSize * sizeof(PVOID) + sizeof(SIZE_T);

            ImportList = (PLOAD_IMPORTS) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                                ImportListSize,
                                                                'TDmM');

            if (ImportList == NULL) {
                UndoEverything = TRUE;
                break;
            }

            ImportList->Count = ImportSize;

            j = 0;
            for (i = 0; i < ImageCount; i += 1) {

                if ((ImageReferences[i] != NULL) &&
                    (ImageReferences[i] != KernelDataTableEntry) &&
                    (ImageReferences[i] != HalDataTableEntry)) {

                        ImportList->Entry[j] = ImageReferences[i];
                        ((PKLDR_DATA_TABLE_ENTRY)ImageReferences[i])->LoadCount += 1;
                        j += 1;
                }
            }

            ASSERT (j == ImportSize);

            DataTableEntry->LoadedImports = ImportList;
        }
    }

finished:

    ExFreePool ((PVOID)ImageReferences);

    //
    // The kernel and HAL are never unloaded.
    //

    if ((KernelDataTableEntry->LoadedImports != NO_IMPORTS_USED) &&
        (!POINTER_TO_SINGLE_ENTRY(KernelDataTableEntry->LoadedImports))) {
            ExFreePool ((PVOID)KernelDataTableEntry->LoadedImports);
    }

    if ((HalDataTableEntry->LoadedImports != NO_IMPORTS_USED) &&
        (!POINTER_TO_SINGLE_ENTRY(HalDataTableEntry->LoadedImports))) {
            ExFreePool ((PVOID)HalDataTableEntry->LoadedImports);
    }

    KernelDataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
    HalDataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;

    if (UndoEverything == TRUE) {

        //
        // An error occurred and this is an all or nothing operation so
        // roll everything back.
        //

        NextEntry = PsLoadedModuleList.Flink;
        while (NextEntry != &PsLoadedModuleList) {
            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            ImportList = DataTableEntry->LoadedImports;
            if (ImportList == LOADED_AT_BOOT || ImportList == NO_IMPORTS_USED ||
                SINGLE_ENTRY(ImportList)) {
                    NOTHING;
            }
            else {
                ExFreePool (ImportList);
            }

            DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
            DataTableEntry->LoadCount = 1;
            NextEntry = NextEntry->Flink;
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}


LOGICAL
MiCallDllUnloadAndUnloadDll(
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

/*++

Routine Description:

    All the references from other drivers to this DLL have been cleared.
    The only remaining issue is that this DLL must support being unloaded.
    This means having no outstanding DPCs, allocated pool, etc.

    If the DLL has an unload routine that returns SUCCESS, then we clean
    it up and free up its memory now.

    Note this routine is NEVER called for drivers - only for DLLs that were
    loaded due to import references from various drivers.

Arguments:

    DataTableEntry - provided for the DLL.

Return Value:

    TRUE if the DLL was successfully unloaded, FALSE if not.

--*/

{
    PMM_DLL_UNLOAD Func;
    NTSTATUS Status;
    LOGICAL Unloaded;

    PAGED_CODE();

    Unloaded = FALSE;

    Func = (PMM_DLL_UNLOAD) (ULONG_PTR) MiLocateExportName (DataTableEntry->DllBase, "DllUnload");

    if (Func) {

        //
        // The unload function was found in the DLL so unload it now.
        //

        Status = Func();

        if (NT_SUCCESS(Status)) {

            //
            // Set up the reference count so the import DLL looks like a regular
            // driver image is being unloaded.
            //

            ASSERT (DataTableEntry->LoadCount == 0);
            DataTableEntry->LoadCount = 1;

            MmUnloadSystemImage ((PVOID)DataTableEntry);
            Unloaded = TRUE;
        }
    }

    return Unloaded;
}


PVOID
MiLocateExportName (
    IN PVOID DllBase,
    IN PCHAR FunctionName
    )

/*++

Routine Description:

    This function is invoked to locate a function name in an export directory.

Arguments:

    DllBase - Supplies the image base.

    FunctionName - Supplies the the name to be located.

Return Value:

    The address of the located function or NULL.

--*/

{
    PVOID Func;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PULONG Addr;
    ULONG ExportSize;
    LONG Low;
    LONG Middle;
    LONG High;
    LONG Result;
    USHORT OrdinalNumber;

    PAGED_CODE();

    Func = NULL;

    //
    // Locate the DLL's export directory.
    //

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                DllBase,
                                TRUE,
                                IMAGE_DIRECTORY_ENTRY_EXPORT,
                                &ExportSize);

    if (ExportDirectory) {

        NameTableBase =  (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);
        NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

        //
        // Look in the export name table for the specified function name.
        //

        Low = 0;
        Middle = 0;
        High = ExportDirectory->NumberOfNames - 1;

        while (High >= Low) {

            //
            // Compute the next probe index and compare the export name entry
            // with the specified function name.
            //

            Middle = (Low + High) >> 1;
            Result = strcmp(FunctionName,
                            (PCHAR)((PCHAR)DllBase + NameTableBase[Middle]));

            if (Result < 0) {
                High = Middle - 1;
            }
            else if (Result > 0) {
                Low = Middle + 1;
            }
            else {
                break;
            }
        }

        //
        // If the high index is less than the low index, then a matching table
        // entry was not found.  Otherwise, get the ordinal number from the
        // ordinal table and location the function address.
        //

        if (High >= Low) {

            OrdinalNumber = NameOrdinalTableBase[Middle];
            Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
            Func = (PVOID)((ULONG_PTR)DllBase + Addr[OrdinalNumber]);

            //
            // If the function address is w/in range of the export directory,
            // then the function is forwarded, which is not allowed, so ignore
            // it.
            //

            if ((ULONG_PTR)Func > (ULONG_PTR)ExportDirectory &&
                (ULONG_PTR)Func < ((ULONG_PTR)ExportDirectory + ExportSize)) {
                Func = NULL;
            }
        }
    }

    return Func;
}


NTSTATUS
MiDereferenceImports (
    IN PLOAD_IMPORTS ImportList
    )

/*++

Routine Description:

    Decrement the reference count on each DLL specified in the image import
    list.  If any DLL's reference count reaches zero, then free the DLL.

    No locks may be held on entry as MmUnloadSystemImage may be called.

    The parameter list is freed here as well.

Arguments:

    ImportList - Supplies the list of DLLs to dereference.

Return Value:

    Status of the dereference operation.

--*/

{
    ULONG i;
    LOGICAL Unloaded;
    PVOID SavedImports;
    LOAD_IMPORTS SingleTableEntry;
    PKLDR_DATA_TABLE_ENTRY ImportTableEntry;

    PAGED_CODE();

    if (ImportList == LOADED_AT_BOOT || ImportList == NO_IMPORTS_USED) {
        return STATUS_SUCCESS;
    }

    if (SINGLE_ENTRY(ImportList)) {
        SingleTableEntry.Count = 1;
        SingleTableEntry.Entry[0] = SINGLE_ENTRY_TO_POINTER(ImportList);
        ImportList = &SingleTableEntry;
    }

    for (i = 0; i < ImportList->Count && ImportList->Entry[i]; i += 1) {
        ImportTableEntry = ImportList->Entry[i];

        if (ImportTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {

            //
            // Skip this one - it was loaded by ntldr.
            //

            continue;
        }

#if DBG
        {
            ULONG ImageCount;
            PLIST_ENTRY NextEntry;
            PKLDR_DATA_TABLE_ENTRY DataTableEntry;

            //
            // Assert that the first 2 entries are never dereferenced as
            // unloading the kernel or HAL would be fatal.
            //

            NextEntry = PsLoadedModuleList.Flink;

            ImageCount = 0;
            while (NextEntry != &PsLoadedModuleList && ImageCount < 2) {
                DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks);
                ASSERT (ImportTableEntry != DataTableEntry);
                ASSERT (DataTableEntry->LoadCount == 1);
                NextEntry = NextEntry->Flink;
                ImageCount += 1;
            }
        }
#endif

        ASSERT (ImportTableEntry->LoadCount >= 1);

        ImportTableEntry->LoadCount -= 1;

        if (ImportTableEntry->LoadCount == 0) {

            //
            // Unload this dependent DLL - we only do this to non-referenced
            // non-boot-loaded drivers.  Stop the import list recursion prior
            // to unloading - we know we're done at this point.
            //
            // Note we can continue on afterwards without restarting
            // regardless of which locks get released and reacquired
            // because this chain is private.
            //

            SavedImports = ImportTableEntry->LoadedImports;

            ImportTableEntry->LoadedImports = (PVOID)NO_IMPORTS_USED;

            Unloaded = MiCallDllUnloadAndUnloadDll ((PVOID)ImportTableEntry);

            if (Unloaded == TRUE) {

                //
                // This DLL was unloaded so recurse through its imports and
                // attempt to unload all of those too.
                //

                MiDereferenceImports ((PLOAD_IMPORTS)SavedImports);

                if ((SavedImports != (PVOID)LOADED_AT_BOOT) &&
                    (SavedImports != (PVOID)NO_IMPORTS_USED) &&
                    (!SINGLE_ENTRY(SavedImports))) {

                        ExFreePool (SavedImports);
                }
            }
            else {
                ImportTableEntry->LoadedImports = SavedImports;
            }
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MiResolveImageReferences (
    PVOID ImageBase,
    IN PUNICODE_STRING ImageFileDirectory,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    OUT PCHAR *MissingProcedureName,
    OUT PWSTR *MissingDriverName,
    OUT PLOAD_IMPORTS *LoadedImports
    )

/*++

Routine Description:

    This routine resolves the references from the newly loaded driver
    to the kernel, HAL and other drivers.

Arguments:

    ImageBase - Supplies the address of which the image header resides.

    ImageFileDirectory - Supplies the directory to load referenced DLLs.

Return Value:

    Status of the image reference resolution.

--*/

{
    PCHAR MissingProcedureStorageArea;
    PVOID ImportBase;
    ULONG ImportSize;
    ULONG ImportListSize;
    ULONG Count;
    ULONG i;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PIMAGE_IMPORT_DESCRIPTOR Imp;
    NTSTATUS st;
    ULONG ExportSize;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_THUNK_DATA NameThunk;
    PIMAGE_THUNK_DATA AddrThunk;
    PSZ ImportName;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY SingleEntry;
    ANSI_STRING AnsiString;
    UNICODE_STRING ImportName_U;
    UNICODE_STRING ImportDescriptorName_U;
    UNICODE_STRING DllToLoad;
    UNICODE_STRING DllToLoad2;
    PVOID Section;
    PVOID BaseAddress;
    LOGICAL PrefixedNameAllocated;
    LOGICAL ReferenceImport;
    ULONG LinkWin32k = 0;
    ULONG LinkNonWin32k = 0;
    PLOAD_IMPORTS ImportList;
    PLOAD_IMPORTS CompactedImportList;
    LOGICAL Loaded;
    UNICODE_STRING DriverDirectory;

    PAGED_CODE();

    *LoadedImports = NO_IMPORTS_USED;

    MissingProcedureStorageArea = *MissingProcedureName;

    ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) RtlImageDirectoryEntryToData (
                        ImageBase,
                        TRUE,
                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                        &ImportSize);

    if (ImportDescriptor == NULL) {
        return STATUS_SUCCESS;
    }

    // Count the number of imports so we can allocate enough room to
    // store them all chained off this module's LDR_DATA_TABLE_ENTRY.
    //

    Count = 0;
    for (Imp = ImportDescriptor; Imp->Name && Imp->OriginalFirstThunk; Imp += 1) {
        Count += 1;
    }

    if (Count != 0) {
        ImportListSize = Count * sizeof(PVOID) + sizeof(SIZE_T);

        ImportList = (PLOAD_IMPORTS) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                             ImportListSize,
                                             'TDmM');

        //
        // Zero it so we can recover gracefully if we fail in the middle.
        // If the allocation failed, just don't build the import list.
        //

        if (ImportList != NULL) {
            RtlZeroMemory (ImportList, ImportListSize);
            ImportList->Count = Count;
        }
    }
    else {
        ImportList = NULL;
    }

    Count = 0;
    while (ImportDescriptor->Name && ImportDescriptor->OriginalFirstThunk) {

        ImportName = (PSZ)((PCHAR)ImageBase + ImportDescriptor->Name);

        //
        // A driver can link with win32k.sys if and only if it is a GDI
        // driver.
        // Also display drivers can only link to win32k.sys (and BBT ...).
        //
        // So if we get a driver that links to win32k.sys and has more
        // than one set of imports, we will fail to load it.
        //

        LinkWin32k = LinkWin32k |
             (!_strnicmp(ImportName, "win32k", sizeof("win32k") - 1));

        //
        // We don't want to count coverage, win32k and irt (BBT) since
        // display drivers CAN link against these.
        //

        LinkNonWin32k = LinkNonWin32k |
            ((_strnicmp(ImportName, "win32k", sizeof("win32k") - 1)) &&

#if defined(_AMD64_)

             (_strnicmp(ImportName, "ntoskrnl", sizeof("ntoskrnl") - 1)) &&

#endif

             (_strnicmp(ImportName, "dxapi", sizeof("dxapi") - 1)) &&
             (_strnicmp(ImportName, "coverage", sizeof("coverage") - 1)) &&
             (_strnicmp(ImportName, "irt", sizeof("irt") - 1)));


        if (LinkNonWin32k && LinkWin32k) {
            MiDereferenceImports (ImportList);
            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        if ((!_strnicmp(ImportName, "ntdll",    sizeof("ntdll") - 1))    ||
            (!_strnicmp(ImportName, "winsrv",   sizeof("winsrv") - 1))   ||
            (!_strnicmp(ImportName, "advapi32", sizeof("advapi32") - 1)) ||
            (!_strnicmp(ImportName, "kernel32", sizeof("kernel32") - 1)) ||
            (!_strnicmp(ImportName, "user32",   sizeof("user32") - 1))   ||
            (!_strnicmp(ImportName, "gdi32",    sizeof("gdi32") - 1)) ) {

            MiDereferenceImports (ImportList);

            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        if ((!_strnicmp(ImportName, "ntoskrnl", sizeof("ntoskrnl") - 1)) ||
            (!_strnicmp(ImportName, "win32k", sizeof("win32k") - 1))     ||
            (!_strnicmp(ImportName, "hal",   sizeof("hal") - 1))) {

            //
            // These imports don't get refcounted because we don't
            // ever want to unload them.
            //

            ReferenceImport = FALSE;
        }
        else {
            ReferenceImport = TRUE;
        }

        RtlInitAnsiString (&AnsiString, ImportName);
        st = RtlAnsiStringToUnicodeString (&ImportName_U, &AnsiString, TRUE);

        if (!NT_SUCCESS(st)) {
            MiDereferenceImports (ImportList);
            if (ImportList != NULL) {
                ExFreePool (ImportList);
            }
            return st;
        }

        if (NamePrefix &&
            (_strnicmp(ImportName, "ntoskrnl", sizeof("ntoskrnl") - 1) &&
             _strnicmp(ImportName, "hal", sizeof("hal") - 1))) {

            ImportDescriptorName_U.MaximumLength = (USHORT)(ImportName_U.Length + NamePrefix->Length);
            ImportDescriptorName_U.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                ImportDescriptorName_U.MaximumLength,
                                                'TDmM');
            if (!ImportDescriptorName_U.Buffer) {
                RtlFreeUnicodeString (&ImportName_U);
                MiDereferenceImports (ImportList);
                if (ImportList != NULL) {
                    ExFreePool (ImportList);
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            ImportDescriptorName_U.Length = 0;
            RtlAppendUnicodeStringToString(&ImportDescriptorName_U, NamePrefix);
            RtlAppendUnicodeStringToString(&ImportDescriptorName_U, &ImportName_U);
            PrefixedNameAllocated = TRUE;
        }
        else {
            ImportDescriptorName_U = ImportName_U;
            PrefixedNameAllocated = FALSE;
        }

        Loaded = FALSE;

ReCheck:
        NextEntry = PsLoadedModuleList.Flink;
        ImportBase = NULL;

        //
        // Initializing DataTableEntry is not needed for correctness
        // but without it the compiler cannot compile this code
        // W4 to check for use of uninitialized variables.
        //

        DataTableEntry = NULL;

        while (NextEntry != &PsLoadedModuleList) {

            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            if (RtlEqualUnicodeString (&ImportDescriptorName_U,
                                       &DataTableEntry->BaseDllName,
                                       TRUE)) {

                ImportBase = DataTableEntry->DllBase;

                //
                // Only bump the LoadCount if this thread did not initiate
                // the load below.  If this thread initiated the load, then
                // the LoadCount has already been bumped as part of the
                // load - we only want to increment it here if we are
                // "attaching" to a previously loaded DLL.
                //

                if ((Loaded == FALSE) && (ReferenceImport == TRUE)) {

                    //
                    // Only increment the load count on the import if it is not
                    // circular (ie: the import is not from the original
                    // caller).
                    //

                    if ((DataTableEntry->Flags & LDRP_LOAD_IN_PROGRESS) == 0) {
                        DataTableEntry->LoadCount += 1;
                    }
                }

                break;
            }
            NextEntry = NextEntry->Flink;
        }

        if (ImportBase == NULL) {

            //
            // The DLL name was not located, attempt to load this dll.
            //

            DllToLoad.MaximumLength = (USHORT)(ImportName_U.Length +
                                        ImageFileDirectory->Length +
                                        sizeof(WCHAR));

            DllToLoad.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                               DllToLoad.MaximumLength,
                                               'TDmM');

            if (DllToLoad.Buffer) {
                DllToLoad.Length = ImageFileDirectory->Length;
                RtlCopyMemory (DllToLoad.Buffer,
                               ImageFileDirectory->Buffer,
                               ImageFileDirectory->Length);

                RtlAppendStringToString ((PSTRING)&DllToLoad,
                                         (PSTRING)&ImportName_U);

                //
                // Add NULL termination in case the load fails so the name
                // can be returned as the PWSTR MissingDriverName.
                //

                DllToLoad.Buffer[(DllToLoad.MaximumLength - 1) / sizeof (WCHAR)] =
                    UNICODE_NULL;

                st = MmLoadSystemImage (&DllToLoad,
                                        NamePrefix,
                                        NULL,
                                        FALSE,
                                        &Section,
                                        &BaseAddress);

                if (NT_SUCCESS(st)) {

                    //
                    // No need to keep the temporary name buffer around now
                    // that there is a loaded module list entry for this DLL.
                    //

                    ExFreePool (DllToLoad.Buffer);
                }
                else {

                    if ((st == STATUS_OBJECT_NAME_NOT_FOUND) &&
                        (NamePrefix == NULL) &&
                        (MI_IS_SESSION_ADDRESS (ImageBase))) {

#define DRIVERS_SUBDIR_NAME L"drivers\\"

                        DriverDirectory.Buffer = (const PUSHORT) DRIVERS_SUBDIR_NAME;
                        DriverDirectory.Length = sizeof (DRIVERS_SUBDIR_NAME) - sizeof (WCHAR);
                        DriverDirectory.MaximumLength = sizeof DRIVERS_SUBDIR_NAME;

                        //
                        // The DLL file was not located, attempt to load it
                        // from the drivers subdirectory.  This makes it
                        // possible for drivers like win32k.sys to link to
                        // drivers that reside in the drivers subdirectory
                        // (like dxapi.sys).
                        //

                        DllToLoad2.MaximumLength = (USHORT)(ImportName_U.Length +
                                                    DriverDirectory.Length +
                                                    ImageFileDirectory->Length +
                                                    sizeof(WCHAR));

                        DllToLoad2.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                           DllToLoad2.MaximumLength,
                                                           'TDmM');

                        if (DllToLoad2.Buffer) {
                            DllToLoad2.Length = ImageFileDirectory->Length;
                            RtlCopyMemory (DllToLoad2.Buffer,
                                           ImageFileDirectory->Buffer,
                                           ImageFileDirectory->Length);

                            RtlAppendStringToString ((PSTRING)&DllToLoad2,
                                                     (PSTRING)&DriverDirectory);

                            RtlAppendStringToString ((PSTRING)&DllToLoad2,
                                                     (PSTRING)&ImportName_U);

                            //
                            // Add NULL termination in case the load fails
                            // so the name can be returned as the PWSTR
                            // MissingDriverName.
                            //

                            DllToLoad2.Buffer[(DllToLoad2.MaximumLength - 1) / sizeof (WCHAR)] =
                                UNICODE_NULL;

                            st = MmLoadSystemImage (&DllToLoad2,
                                                    NULL,
                                                    NULL,
                                                    FALSE,
                                                    &Section,
                                                    &BaseAddress);

                            ExFreePool (DllToLoad.Buffer);

                            DllToLoad.Buffer = DllToLoad2.Buffer;
                            DllToLoad.Length = DllToLoad2.Length;
                            DllToLoad.MaximumLength = DllToLoad2.MaximumLength;

                            if (NT_SUCCESS(st)) {
                                ExFreePool (DllToLoad.Buffer);
                                goto LoadFinished;
                            }
                        }
                        else {
                            Section = NULL;
                            BaseAddress = NULL;
                            st = STATUS_INSUFFICIENT_RESOURCES;
                            goto LoadFinished;
                        }
                    }

                    //
                    // Return the temporary name buffer to our caller so
                    // the name of the DLL that failed to load can be displayed.
                    // Set the low bit of the pointer so our caller knows to
                    // free this buffer when he's done displaying it (as opposed
                    // to loaded module list entries which should not be freed).
                    //

                    *MissingDriverName = DllToLoad.Buffer;
                    *(PULONG)MissingDriverName |= 0x1;

                    //
                    // Set this to NULL so the hard error prints properly.
                    //

                    *MissingProcedureName = NULL;
                }
            }
            else {

                //
                // Initializing Section and BaseAddress is not needed for
                // correctness but without it the compiler cannot compile
                // this code W4 to check for use of uninitialized variables.
                //

                Section = NULL;
                BaseAddress = NULL;
                st = STATUS_INSUFFICIENT_RESOURCES;
            }

LoadFinished:

            //
            // Call any needed DLL initialization now.
            //

            if (NT_SUCCESS(st)) {
#if DBG
                PLIST_ENTRY Entry;
#endif
                PKLDR_DATA_TABLE_ENTRY TableEntry;

                Loaded = TRUE;

                TableEntry = (PKLDR_DATA_TABLE_ENTRY) Section;
                ASSERT (BaseAddress == TableEntry->DllBase);

#if DBG
                //
                // Lookup the dll's table entry in the loaded module list.
                // This is expected to always succeed.
                //

                Entry = PsLoadedModuleList.Blink;
                while (Entry != &PsLoadedModuleList) {
                    TableEntry = CONTAINING_RECORD (Entry,
                                                    KLDR_DATA_TABLE_ENTRY,
                                                    InLoadOrderLinks);

                    if (BaseAddress == TableEntry->DllBase) {
                        ASSERT (TableEntry == (PKLDR_DATA_TABLE_ENTRY) Section);
                        break;
                    }
                    ASSERT (TableEntry != (PKLDR_DATA_TABLE_ENTRY) Section);
                    Entry = Entry->Blink;
                }

                ASSERT (Entry != &PsLoadedModuleList);
#endif

                //
                // Call the dll's initialization routine if it has
                // one.  This routine will reapply verifier thunks to
                // any modules that link to this one if necessary.
                //

                st = MmCallDllInitialize (TableEntry, &PsLoadedModuleList);

                //
                // If the module could not be properly initialized,
                // unload it.
                //

                if (!NT_SUCCESS(st)) {
                    MmUnloadSystemImage ((PVOID)TableEntry);
                    Loaded = FALSE;
                }
            }

            if (!NT_SUCCESS(st)) {

                RtlFreeUnicodeString (&ImportName_U);
                if (PrefixedNameAllocated == TRUE) {
                    ExFreePool (ImportDescriptorName_U.Buffer);
                }
                MiDereferenceImports (ImportList);
                if (ImportList != NULL) {
                    ExFreePool (ImportList);
                }
                return st;
            }

            goto ReCheck;
        }

        if ((ReferenceImport == TRUE) && (ImportList)) {

            //
            // Only add the image providing satisfying our imports to the
            // import list if the reference is not circular (ie: the import
            // is not from the original caller).
            //

            if ((DataTableEntry->Flags & LDRP_LOAD_IN_PROGRESS) == 0) {
                ImportList->Entry[Count] = DataTableEntry;
                Count += 1;
            }
        }

        RtlFreeUnicodeString (&ImportName_U);
        if (PrefixedNameAllocated) {
            ExFreePool (ImportDescriptorName_U.Buffer);
        }

        *MissingDriverName = DataTableEntry->BaseDllName.Buffer;

        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                    ImportBase,
                                    TRUE,
                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                    &ExportSize);

        if (!ExportDirectory) {
            MiDereferenceImports (ImportList);
            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;
        }

        //
        // Walk through the IAT and snap all the thunks.
        //

        if (ImportDescriptor->OriginalFirstThunk) {

            NameThunk = (PIMAGE_THUNK_DATA)((PCHAR)ImageBase + (ULONG)ImportDescriptor->OriginalFirstThunk);
            AddrThunk = (PIMAGE_THUNK_DATA)((PCHAR)ImageBase + (ULONG)ImportDescriptor->FirstThunk);

            while (NameThunk->u1.AddressOfData) {

                st = MiSnapThunk (ImportBase,
                                  ImageBase,
                                  NameThunk++,
                                  AddrThunk++,
                                  ExportDirectory,
                                  ExportSize,
                                  FALSE,
                                  MissingProcedureName);

                if (!NT_SUCCESS(st) ) {
                    MiDereferenceImports (ImportList);
                    if (ImportList) {
                        ExFreePool (ImportList);
                    }
                    return st;
                }
                *MissingProcedureName = MissingProcedureStorageArea;
            }
        }

        ImportDescriptor += 1;
    }

    //
    // All the imports are successfully loaded so establish and compact
    // the import unload list.
    //

    if (ImportList) {

        //
        // Blank entries occur for things like the kernel, HAL & win32k.sys
        // that we never want to unload.  Especially for things like
        // win32k.sys where the reference count can really hit 0.
        //

        //
        // Initializing SingleEntry is not needed for correctness
        // but without it the compiler cannot compile this code
        // W4 to check for use of uninitialized variables.
        //

        SingleEntry = NULL;

        Count = 0;
        for (i = 0; i < ImportList->Count; i += 1) {
            if (ImportList->Entry[i]) {
                SingleEntry = POINTER_TO_SINGLE_ENTRY(ImportList->Entry[i]);
                Count += 1;
            }
        }

        if (Count == 0) {

            ExFreePool(ImportList);
            ImportList = NO_IMPORTS_USED;
        }
        else if (Count == 1) {
            ExFreePool(ImportList);
            ImportList = (PLOAD_IMPORTS)SingleEntry;
        }
        else if (Count != ImportList->Count) {

            ImportListSize = Count * sizeof(PVOID) + sizeof(SIZE_T);

            CompactedImportList = (PLOAD_IMPORTS)
                                        ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                        ImportListSize,
                                        'TDmM');
            if (CompactedImportList) {
                CompactedImportList->Count = Count;

                Count = 0;
                for (i = 0; i < ImportList->Count; i += 1) {
                    if (ImportList->Entry[i]) {
                        CompactedImportList->Entry[Count] = ImportList->Entry[i];
                        Count += 1;
                    }
                }

                ExFreePool(ImportList);
                ImportList = CompactedImportList;
            }
        }

        *LoadedImports = ImportList;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
MiSnapThunk(
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA NameThunk,
    OUT PIMAGE_THUNK_DATA AddrThunk,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN LOGICAL SnapForwarder,
    OUT PCHAR *MissingProcedureName
    )

/*++

Routine Description:

    This function snaps a thunk using the specified Export Section data.
    If the section data does not support the thunk, then the thunk is
    partially snapped (Dll field is still non-null, but snap address is
    set).

Arguments:

    DllBase - Base of DLL being snapped to.

    ImageBase - Base of image that contains the thunks to snap.

    Thunk - On input, supplies the thunk to snap.  When successfully
            snapped, the function field is set to point to the address in
            the DLL, and the DLL field is set to NULL.

    ExportDirectory - Supplies the Export Section data from a DLL.

    SnapForwarder - Supplies TRUE if the snap is for a forwarder, and therefore
                    Address of Data is already setup.

Return Value:

    STATUS_SUCCESS or STATUS_DRIVER_ENTRYPOINT_NOT_FOUND or
        STATUS_DRIVER_ORDINAL_NOT_FOUND

--*/

{
    BOOLEAN Ordinal;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    USHORT HintIndex;
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;
    NTSTATUS Status;
    PCHAR MissingProcedureName2;
    CHAR NameBuffer[ MAXIMUM_FILENAME_LENGTH ];

    PAGED_CODE();

    //
    // Determine if snap is by name, or by ordinal
    //

    Ordinal = (BOOLEAN)IMAGE_SNAP_BY_ORDINAL(NameThunk->u1.Ordinal);

    if (Ordinal && !SnapForwarder) {

        OrdinalNumber = (USHORT)(IMAGE_ORDINAL(NameThunk->u1.Ordinal) -
                         ExportDirectory->Base);

        *MissingProcedureName = (PCHAR)(ULONG_PTR)OrdinalNumber;

    }
    else {

        //
        // Change AddressOfData from an RVA to a VA.
        //

        if (!SnapForwarder) {
            NameThunk->u1.AddressOfData = (ULONG_PTR)ImageBase + NameThunk->u1.AddressOfData;
        }

        strncpy (*MissingProcedureName,
                 (const PCHAR)&((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name[0],
                 MAXIMUM_FILENAME_LENGTH - 1);

        //
        // Lookup Name in NameTable
        //

        NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);
        NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

        //
        // Before dropping into binary search, see if
        // the hint index results in a successful
        // match. If the hint index is zero, then
        // drop into binary search.
        //

        HintIndex = ((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Hint;
        if ((ULONG)HintIndex < ExportDirectory->NumberOfNames &&
            !strcmp((PSZ)((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name,
             (PSZ)((PCHAR)DllBase + NameTableBase[HintIndex]))) {
            OrdinalNumber = NameOrdinalTableBase[HintIndex];

        }
        else {

            //
            // Lookup the import name in the name table using a binary search.
            //

            Low = 0;
            Middle = 0;
            High = ExportDirectory->NumberOfNames - 1;

            while (High >= Low) {

                //
                // Compute the next probe index and compare the import name
                // with the export name entry.
                //

                Middle = (Low + High) >> 1;
                Result = strcmp((const PCHAR)&((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name[0],
                                (PCHAR)((PCHAR)DllBase + NameTableBase[Middle]));

                if (Result < 0) {
                    High = Middle - 1;
                }
                else if (Result > 0) {
                    Low = Middle + 1;
                }
                else {
                    break;
                }
            }

            //
            // If the high index is less than the low index, then a matching
            // table entry was not found. Otherwise, get the ordinal number
            // from the ordinal table.
            //

            if (High < Low) {
                return STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;
            }
            else {
                OrdinalNumber = NameOrdinalTableBase[Middle];
            }
        }
    }

    //
    // If OrdinalNumber is not within the Export Address Table,
    // then DLL does not implement function. Snap to LDRP_BAD_DLL.
    //

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        Status = STATUS_DRIVER_ORDINAL_NOT_FOUND;

    }
    else {

        MissingProcedureName2 = NameBuffer;

        Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
        *(PULONG_PTR)&AddrThunk->u1.Function = (ULONG_PTR)DllBase + Addr[OrdinalNumber];

        // AddrThunk s/b used from here on.

        Status = STATUS_SUCCESS;

        if (((ULONG_PTR)AddrThunk->u1.Function > (ULONG_PTR)ExportDirectory) &&
            ((ULONG_PTR)AddrThunk->u1.Function < ((ULONG_PTR)ExportDirectory + ExportSize)) ) {

            UNICODE_STRING UnicodeString;
            ANSI_STRING ForwardDllName;

            PLIST_ENTRY NextEntry;
            PKLDR_DATA_TABLE_ENTRY DataTableEntry;
            ULONG LocalExportSize;
            PIMAGE_EXPORT_DIRECTORY LocalExportDirectory;

            Status = STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;

            //
            // Include the dot in the length so we can do prefix later on.
            //

            ForwardDllName.Buffer = (PCHAR)AddrThunk->u1.Function;
            ForwardDllName.Length = (USHORT)(strchr(ForwardDllName.Buffer, '.') -
                                           ForwardDllName.Buffer + 1);
            ForwardDllName.MaximumLength = ForwardDllName.Length;

            if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                        &ForwardDllName,
                                                        TRUE))) {

                NextEntry = PsLoadedModuleList.Flink;

                while (NextEntry != &PsLoadedModuleList) {

                    DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                       KLDR_DATA_TABLE_ENTRY,
                                                       InLoadOrderLinks);

                    //
                    // We have to do a case INSENSITIVE comparison for
                    // forwarder because the linker just took what is in the
                    // def file, as opposed to looking in the exporting
                    // image for the name.
                    // we also use the prefix function to ignore the .exe or
                    // .sys or .dll at the end.
                    //

                    if (RtlPrefixString((PSTRING)&UnicodeString,
                                        (PSTRING)&DataTableEntry->BaseDllName,
                                        TRUE)) {

                        LocalExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
                            RtlImageDirectoryEntryToData (DataTableEntry->DllBase,
                                                         TRUE,
                                                         IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                         &LocalExportSize);

                        if (LocalExportDirectory != NULL) {

                            IMAGE_THUNK_DATA thunkData;
                            PIMAGE_IMPORT_BY_NAME addressOfData;
                            SIZE_T length;

                            //
                            // One extra byte for NULL termination.
                            //

                            length = strlen(ForwardDllName.Buffer +
                                                ForwardDllName.Length) + 1;

                            addressOfData = (PIMAGE_IMPORT_BY_NAME)
                                ExAllocatePoolWithTag (PagedPool,
                                                      length +
                                                   sizeof(IMAGE_IMPORT_BY_NAME),
                                                   '  mM');

                            if (addressOfData) {

                                RtlCopyMemory(&(addressOfData->Name[0]),
                                              ForwardDllName.Buffer +
                                                  ForwardDllName.Length,
                                              length);

                                addressOfData->Hint = 0;

                                *(PULONG_PTR)&thunkData.u1.AddressOfData =
                                                    (ULONG_PTR)addressOfData;

                                Status = MiSnapThunk (DataTableEntry->DllBase,
                                                     ImageBase,
                                                     &thunkData,
                                                     &thunkData,
                                                     LocalExportDirectory,
                                                     LocalExportSize,
                                                     TRUE,
                                                     &MissingProcedureName2);

                                ExFreePool (addressOfData);

                                AddrThunk->u1 = thunkData.u1;
                            }
                        }

                        break;
                    }

                    NextEntry = NextEntry->Flink;
                }

                RtlFreeUnicodeString (&UnicodeString);
            }

        }

    }
    return Status;
}

NTSTATUS
MmCheckSystemImage (
    IN HANDLE ImageFileHandle,
    IN LOGICAL PurgeSection
    )

/*++

Routine Description:

    This function ensures the checksum for a system image is correct
    and matches the data in the image.

Arguments:

    ImageFileHandle - Supplies the file handle of the image.  This is a kernel
                      handle (ie: cannot be tampered with by the user).

    PurgeSection - Supplies TRUE if the data section mapping the image should
                   be purged prior to returning.  Note that the first page
                   could be used to speed up subsequent image section creation,
                   but generally the cost of useless data pages sitting in
                   transition is costly.  Better to put the pages immediately
                   on the free list to preserve the transition cache for more
                   useful pages.

Return Value:

    Status value.

--*/

{
    NTSTATUS Status;
    NTSTATUS Status2;
    HANDLE Section;
    PVOID ViewBase;
    SIZE_T ViewSize;
    IO_STATUS_BLOCK IoStatusBlock;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    FILE_STANDARD_INFORMATION StandardInfo;
    PSECTION SectionPointer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    KAPC_STATE ApcState;
    ULONG SectionType;
    SIZE_T NumberOfBytes;

    PAGED_CODE();

    SectionType = SEC_IMAGE;

retry:

    InitializeObjectAttributes (&ObjectAttributes,
                                NULL,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL);

    Status = ZwCreateSection (&Section,
                              SECTION_MAP_EXECUTE,
                              &ObjectAttributes,
                              NULL,
                              PAGE_EXECUTE,
                              SectionType,
                              ImageFileHandle);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ViewBase = NULL;
    ViewSize = 0;

    //
    // Since callees are not always in the context of the system process,
    // attach here when necessary to guarantee the driver load occurs in a
    // known safe address space to prevent security holes.
    //

    KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);

    Status = ZwMapViewOfSection (Section,
                                 NtCurrentProcess (),
                                 (PVOID *)&ViewBase,
                                 0L,
                                 0L,
                                 NULL,
                                 &ViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_EXECUTE);

    if (!NT_SUCCESS(Status)) {
        KeUnstackDetachProcess (&ApcState);
        ZwClose (Section);
        return Status;
    }

    //
    // Now the image is mapped as a data file... Calculate its size and then
    // check its checksum.
    //

    Status = ZwQueryInformationFile (ImageFileHandle,
                                     &IoStatusBlock,
                                     &StandardInfo,
                                     sizeof(StandardInfo),
                                     FileStandardInformation);

    if (NT_SUCCESS(Status)) {

        if (SectionType == SEC_IMAGE) {
            NumberOfBytes = ViewSize;
        }
        else {
            NumberOfBytes = StandardInfo.EndOfFile.LowPart;
        }

        try {

            if (!LdrVerifyMappedImageMatchesChecksum (
                                        ViewBase,
                                        NumberOfBytes,
                                        StandardInfo.EndOfFile.LowPart)) {

                Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
                goto out;
            }

            NtHeaders = RtlImageNtHeader (ViewBase);

            if (NtHeaders == NULL) {
                Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
                goto out;
            }

            FileHeader = &NtHeaders->FileHeader;

            //
            // Detect configurations inadvertently trying to load 32-bit
            // drivers on NT64 or mismatched platform architectures, etc.
            //

            if ((FileHeader->Machine != IMAGE_FILE_MACHINE_NATIVE) ||
                (NtHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)) {
                Status = STATUS_INVALID_IMAGE_PROTECT;
                goto out;
            }

#if !defined(NT_UP)
            if (!MmVerifyImageIsOkForMpUse (ViewBase)) {
                Status = STATUS_IMAGE_MP_UP_MISMATCH;
                goto out;
            }
#endif
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
        }
    }

out:

    ZwUnmapViewOfSection (NtCurrentProcess (), ViewBase);

    KeUnstackDetachProcess (&ApcState);

    if ((Status == STATUS_IMAGE_CHECKSUM_MISMATCH) &&
        (SectionType == SEC_IMAGE)) {

        //
        // We speculatively tried to verify the checksum using the image
        // mapping but this did not match.  This can occur if the image has
        // a self-signed certificate appended to it (this is a rare occurrence)
        // as the certificate data will be present in the data file but is
        // not mapped when the file is used as an image.  So fall back to
        // mapping the file as data and recompute.
        //
        // Note there are other cases above that can get us to this check but
        // it's always ok to retry as data ...
        //

        ZwClose (Section);
        SectionType = SEC_COMMIT;
        goto retry;
    }

    if ((PurgeSection == TRUE) && (SectionType == SEC_COMMIT)) {

        Status2 = ObReferenceObjectByHandle (Section,
                                             SECTION_MAP_EXECUTE,
                                             MmSectionObjectType,
                                             KernelMode,
                                             (PVOID *) &SectionPointer,
                                             (POBJECT_HANDLE_INFORMATION) NULL);

        if (NT_SUCCESS (Status2)) {

            MmPurgeSection (SectionPointer->Segment->ControlArea->FilePointer->SectionObjectPointer,
                            NULL,
                            0,
                            FALSE);
            ObDereferenceObject (SectionPointer);
        }
    }

    ZwClose (Section);
    return Status;
}

#if !defined(NT_UP)
BOOLEAN
MmVerifyImageIsOkForMpUse (
    IN PVOID BaseAddress
    )
{
    PIMAGE_NT_HEADERS NtHeaders;

    PAGED_CODE();

    NtHeaders = RtlImageNtHeader (BaseAddress);

    if ((NtHeaders != NULL) &&
        (KeNumberProcessors > 1) &&
        (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_UP_SYSTEM_ONLY)) {

        return FALSE;
    }

    return TRUE;
}
#endif


PFN_NUMBER
MiDeleteSystemPageableVm (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes,
    IN ULONG Flags,
    OUT PPFN_NUMBER ResidentPages OPTIONAL
    )

/*++

Routine Description:

    This function deletes pageable system address space (paged pool
    or driver pageable sections).

Arguments:

    PointerPte - Supplies the start of the PTE range to delete.

    NumberOfPtes - Supplies the number of PTEs in the range.

    Flags - Supplies flags indicating what the caller desires.

    ResidentPages - If not NULL, the number of resident pages freed is
                    returned here.

Return Value:

    Returns the number of pages actually freed.

--*/

{
    ULONG TimeStamp;
    PMMSUPPORT Ws;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER ValidPages;
    PFN_NUMBER PagesRequired;
    WSLE_NUMBER WsIndex;
    KIRQL OldIrql;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    MMWSLENTRY Locked;
    PFN_NUMBER PageTableFrameIndex;
    LOGICAL WsHeld;
    PETHREAD Thread;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    ValidPages = 0;
    PagesRequired = 0;
    FlushCount = 0;
    WsHeld = FALSE;

    Thread = PsGetCurrentThread ();

    if (MI_IS_SESSION_PTE (PointerPte)) {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
    }
    else {
        Ws = &MmSystemCacheWs;
    }

#if defined(_X86PAE_)

    //
    // PAE PTEs are written in 2 separate writes so the working set pushlock
    // must always be held even to examine PTEs in prototype format - because
    // a trim could be happening in parallel (writing the low half and then
    // the upper half which would cause otherwise cause us to read the
    // PTE contents split in the middle).
    //

    WsHeld = TRUE;
    LOCK_WORKING_SET (Thread, Ws);

#endif

    while (NumberOfPtes != 0) {
        PteContents = *PointerPte;

        if (PteContents.u.Long != 0) {

            if (PteContents.u.Hard.Valid == 1) {

                //
                // Once the working set mutex is acquired, it is deliberately
                // held until all the pages have been freed.  This is because
                // when paged pool is running low on large servers, we need the
                // segment dereference thread to be able to free large amounts
                // quickly.  Typically this thread will free 64k chunks and we
                // don't want to have to contend for the mutex 16 times to do
                // this as there may be thousands of other threads also trying
                // for it.
                //

                if (WsHeld == FALSE) {
                    WsHeld = TRUE;
                    LOCK_WORKING_SET (Thread, Ws);
                }

                PteContents = *PointerPte;
                if (PteContents.u.Hard.Valid == 0) {
                    continue;
                }

                //
                // Delete the page.
                //

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                //
                // Check to see if this is a pageable page in which
                // case it needs to be removed from the working set list.
                //

                WsIndex = Pfn1->u1.WsIndex;
                if (WsIndex == 0) {
                    ValidPages += 1;
                    if (Ws != &MmSystemCacheWs) {
                        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_DELVA, 1);
                        InterlockedDecrementSizeT (&MmSessionSpace->NonPageablePages);
                    }
                }
                else {
                    if (Ws == &MmSystemCacheWs) {
                        MiRemoveWsle (WsIndex, MmSystemCacheWorkingSetList);
                        MiReleaseWsle (WsIndex, &MmSystemCacheWs);
                    }
                    else {
                        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                        WsIndex = MiLocateWsle (VirtualAddress,
                                              MmSessionSpace->Vm.VmWorkingSetList,
                                              WsIndex,
                                              TRUE);

                        ASSERT (WsIndex != WSLE_NULL_INDEX);

                        //
                        // Check to see if this entry is locked in
                        // the working set or locked in memory.
                        //

                        Locked = MmSessionSpace->Wsle[WsIndex].u1.e1;

                        MiRemoveWsle (WsIndex, MmSessionSpace->Vm.VmWorkingSetList);

                        MiReleaseWsle (WsIndex, &MmSessionSpace->Vm);

                        if (Locked.LockedInWs == 1 || Locked.LockedInMemory == 1) {

                            //
                            // This entry is locked.
                            //

                            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_DELVA, 1);
                            InterlockedDecrementSizeT (&MmSessionSpace->NonPageablePages);
                            ValidPages += 1;

                            ASSERT (WsIndex < MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic);
                            MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic -= 1;

                            if (WsIndex != MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic) {
                                WSLE_NUMBER Entry;
                                PVOID SwapVa;

                                Entry = MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic;
                                ASSERT (MmSessionSpace->Wsle[Entry].u1.e1.Valid);
                                SwapVa = MmSessionSpace->Wsle[Entry].u1.VirtualAddress;
                                SwapVa = PAGE_ALIGN (SwapVa);

                                MiSwapWslEntries (Entry,
                                                  WsIndex,
                                                  &MmSessionSpace->Vm,
                                                  FALSE);
                            }
                        }
                        else {
                            ASSERT (WsIndex >= MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic);
                        }
                    }
                }

                //
                // Check if this is a prototype PTE.
                //

                if (Pfn1->u3.e1.PrototypePte == 1) {

                    PMMPTE PointerPde;

                    ASSERT (Ws != &MmSystemCacheWs);

                    //
                    // Capture the state of the modified bit for this PTE.
                    //

                    PointerPde = MiGetPteAddress (PointerPte);

                    if (PointerPde->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                        if (!NT_SUCCESS (MiCheckPdeForPagedPool (PointerPte))) {
#endif
                            KeBugCheckEx (MEMORY_MANAGEMENT,
                                          0x61940,
                                          (ULONG_PTR)PointerPte,
                                          (ULONG_PTR)PointerPde->u.Long,
                                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
                        }
#endif
                    }

                    PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

                    LOCK_PFN (OldIrql);

                    MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);
                }
                else {
                    PageTableFrameIndex = Pfn1->u4.PteFrame;
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

                    LOCK_PFN (OldIrql);

                    MI_SET_PFN_DELETED (Pfn1);
                }

                //
                // Decrement the share count for the containing page table page.
                //

                MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                //
                // Decrement the share count for the physical page.
                //

                MiDecrementShareCount (Pfn1, PageFrameIndex);

                UNLOCK_PFN (OldIrql);

                MI_WRITE_ZERO_PTE (PointerPte);

                if (Flags & MI_DELETE_FLUSH_TB) {

                    //
                    // Flush the TB for this virtual address.
                    //

                    if (FlushCount != MM_MAXIMUM_FLUSH_COUNT) {

                        VaFlushList[FlushCount] =
                                    MiGetVirtualAddressMappedByPte (PointerPte);
                        FlushCount += 1;
                    }
                }
                else {

                    //
                    // Our caller will lazy flush this entry so stamp the time 
                    // for him.  Since zero is used by our caller as an
                    // indicator no flush is needed, ensure that if the
                    // inserted field (which may be truncated) is zero, that
                    // a flush of this address is done before we return.
                    //

                    TimeStamp = KeReadTbFlushTimeStamp ();

                    PointerPte->u.Soft.PageFileHigh = TimeStamp;

                    if (PointerPte->u.Soft.PageFileHigh == 0) {

                        if (FlushCount != MM_MAXIMUM_FLUSH_COUNT) {

                            VaFlushList[FlushCount] =
                                    MiGetVirtualAddressMappedByPte (PointerPte);
                            FlushCount += 1;
                        }
                    }
                }
            }
            else if (PteContents.u.Soft.Prototype) {

                ASSERT (Ws != &MmSystemCacheWs);

                MI_WRITE_ZERO_PTE (PointerPte);
            }
            else if (PteContents.u.Soft.Transition == 1) {

                LOCK_PFN (OldIrql);

                PteContents = *PointerPte;

                if (PteContents.u.Soft.Transition == 0) {
                    UNLOCK_PFN (OldIrql);
                    continue;
                }

                //
                // Transition, release page.
                //

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);

                //
                // Set the pointer to PTE as empty so the page
                // is deleted when the reference count goes to zero.
                //

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                MI_SET_PFN_DELETED (Pfn1);

                PageTableFrameIndex = Pfn1->u4.PteFrame;
                Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                //
                // Check the reference count for the page, if the reference
                // count is zero, move the page to the free list, if the
                // reference count is not zero, ignore this page.  When the
                // reference count goes to zero, it will be placed on the
                // free list.
                //

                if (Pfn1->u3.e2.ReferenceCount == 0) {
                    MiUnlinkPageFromList (Pfn1);
                    MiReleasePageFileSpace (Pfn1->OriginalPte);
                    MiInsertPageInFreeList (PageFrameIndex);
                }

                MI_WRITE_ZERO_PTE (PointerPte);
                UNLOCK_PFN (OldIrql);
            }
            else {

                //
                // Demand zero, release page file space.
                //
                if (PteContents.u.Soft.PageFileHigh != 0) {
                    LOCK_PFN (OldIrql);
                    MiReleasePageFileSpace (PteContents);
                    UNLOCK_PFN (OldIrql);
                }

                MI_WRITE_ZERO_PTE (PointerPte);
            }

            PagesRequired += 1;
        }

        NumberOfPtes -= 1;
        PointerPte += 1;
    }

    if (WsHeld == TRUE) {
        UNLOCK_WORKING_SET (Thread, Ws);
    }

    //
    // Some callers flush the TB themselves, but for the rest we must flush
    // here.
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
        MI_FLUSH_ENTIRE_TB (0x16);
    }

    if (ARGUMENT_PRESENT (ResidentPages)) {
        *ResidentPages = ValidPages;
    }

    return PagesRequired;
}

VOID
MiMarkSectionWritable (
    IN PIMAGE_SECTION_HEADER SectionTableEntry
    )

/*++

Routine Description:

    This function is a nonpaged helper routine that updates the characteristics
    field of the argument section table entry and marks the page dirty so
    that subsequent session loads share the same copy.

Arguments:

     SectionTableEntry - Supplies the relevant section table entry.

Return Value:

     None.

--*/

{
    PEPROCESS Process;
    PMMPTE PointerPte;
    ULONG FreeBit;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
    PULONG Characteristics;

    //
    // Modify the PE header through hyperspace and mark the header page
    // dirty so subsequent sections pick up the same copy.
    //
    // Note this makes the entire .rdata (.sdata on IA64) writable
    // instead of just the import tables.
    //

    Process = PsGetCurrentProcess ();

    PointerPte = MiGetPteAddress (&SectionTableEntry->Characteristics);
    LOCK_PFN (OldIrql);

    MiMakeSystemAddressValidPfn (&SectionTableEntry->Characteristics, OldIrql);
    ASSERT (PointerPte->u.Hard.Valid == 1);

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    Characteristics = MiMapPageInHyperSpaceAtDpc (Process, PageFrameIndex);
    Characteristics = (PULONG)((PCHAR)Characteristics + MiGetByteOffset (&SectionTableEntry->Characteristics));

    *Characteristics |= IMAGE_SCN_MEM_WRITE;

    MiUnmapPageInHyperSpaceFromDpc (Process, Characteristics);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    MI_SET_MODIFIED (Pfn1, 1, 0x7);

    if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
        (Pfn1->u3.e1.WriteInProgress == 0)) {

        FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

        if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
            MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
        }

        Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
    }

    UNLOCK_PFN (OldIrql);

    return;
}

VOID
MiMakeEntireImageCopyOnWrite (
    IN PSUBSECTION Subsection
    )

/*++

Routine Description:

    This function sets the protection of all prototype PTEs to copy on write.

Arguments:

     Subsection - Supplies the base subsection for the entire image.

Return Value:

     None.

--*/

{
    PMMPTE PointerPte;
    PMMPTE ProtoPte;
    PMMPTE LastProtoPte;
    MMPTE PteContents;

    //
    // Note this is only called for image control areas that have at least
    // PAGE_SIZE subsection alignment, and so the first
    // subsection which maps the header can always be skipped.
    //

    do {
        Subsection = Subsection->NextSubsection;

        if (Subsection == NULL) {
            break;
        }

        //
        // Don't mark global subsections as copy on write even when the
        // image is relocated.  This is easily distinguishable because
        // it is the only subsection that is marked readwrite.
        //

        if (Subsection->u.SubsectionFlags.Protection == MM_READWRITE) {
            continue;
        }

        ProtoPte = Subsection->SubsectionBase;
        LastProtoPte = Subsection->SubsectionBase + Subsection->PtesInSubsection;

        PointerPte = ProtoPte;

        MmLockPagedPool (ProtoPte, Subsection->PtesInSubsection * sizeof (MMPTE));

        do {
            PteContents = *PointerPte;
            ASSERT (PteContents.u.Hard.Valid == 0);
            if (PteContents.u.Long != 0) {
                if ((PteContents.u.Soft.Prototype == 0) &&
                    (PteContents.u.Soft.Transition == 1)) {
                    if (MiSetProtectionOnTransitionPte (PointerPte, MM_EXECUTE_WRITECOPY)) {
                        continue;
                    }
                }
                else {
                    PointerPte->u.Soft.Protection = MM_EXECUTE_WRITECOPY;
                }
            }
            PointerPte += 1;
        } while (PointerPte < LastProtoPte);

        MmUnlockPagedPool (ProtoPte, Subsection->PtesInSubsection * sizeof (MMPTE));

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_WRITECOPY;

    } while (TRUE);

    return;
}


VOID
MiSetSystemCodeProtection (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG ProtectionMask
    )

/*++

Routine Description:

    This function sets the protection of system code as specified.

Arguments:

    FirstPte - Supplies the starting PTE.

    LastPte - Supplies the ending PTE.

    ProtectionMask - Supplies the desired protection mask.

Return Value:

    None.

Environment:

    Kernel Mode, APC_LEVEL or below.

--*/

{
    KIRQL OldIrql;
    MMPTE PteContents;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerProtoPte;
    PMMPFN Pfn1;
    LOGICAL SessionAddress;
    PVOID VirtualAddress;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    PETHREAD CurrentThread;
    PMMSUPPORT Ws;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    FlushCount = 0;

#if defined(_X86_)
    ASSERT (MI_IS_PHYSICAL_ADDRESS(MiGetVirtualAddressMappedByPte(FirstPte)) == 0);
#endif

    CurrentThread = PsGetCurrentThread ();

    PointerPte = FirstPte;

    if (MI_IS_SESSION_ADDRESS (MiGetVirtualAddressMappedByPte(FirstPte))) {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
        SessionAddress = TRUE;
    }
    else {
        Ws = &MmSystemCacheWs;
        SessionAddress = FALSE;
    }

    LOCK_WORKING_SET (CurrentThread, Ws);

    //
    // Set these PTEs to the specified protection.
    //
    // Note that the write bit may already be off (in the valid PTE) if the
    // page has already been inpaged from the paging file and has not since
    // been dirtied.
    //

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {

        PteContents = *PointerPte;

        if (PteContents.u.Long == 0) {
            PointerPte += 1;
            continue;
        }

        if (PteContents.u.Hard.Valid == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            if (Pfn1->u3.e1.PrototypePte == 1) {

                //
                // This must be a session address.  The prototype PTE contains
                // the protection that is pushed out to the real PTE after
                // it's been trimmed so update that too.
                //

                PointerProtoPte = Pfn1->PteAddress;

                PointerPde = MiGetPteAddress (PointerProtoPte);

                if (PointerPde->u.Hard.Valid == 0) {

                    if (SessionAddress == TRUE) {

                        //
                        // Unlock the session working set and lock the
                        // system working set as we need to make the backing
                        // prototype PTE valid.
                        //

                        UNLOCK_PFN (OldIrql);

                        UNLOCK_WORKING_SET (CurrentThread, Ws);

                        LOCK_WORKING_SET (CurrentThread, &MmSystemCacheWs);

                        LOCK_PFN (OldIrql);
                    }

                    MiMakeSystemAddressValidPfnSystemWs (PointerProtoPte,
                                                         OldIrql);

                    if (SessionAddress == TRUE) {

                        //
                        // Unlock the system working set and lock the
                        // session working set as we have made the backing
                        // prototype PTE valid and can now handle the
                        // original session PTE.
                        //

                        UNLOCK_PFN (OldIrql);

                        UNLOCK_WORKING_SET (CurrentThread, &MmSystemCacheWs);

                        LOCK_WORKING_SET (CurrentThread, Ws);

                        LOCK_PFN (OldIrql);
                    }

                    //
                    // The world may have changed while we waited.
                    //

                    continue;
                }
            }
            else {
                if ((ProtectionMask & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK) {
                    //
                    // This page is already private so ignore the copy on
                    // write attribute (or anything else).  This is very
                    // unusual.
                    //

                    PointerPte += 1;
                    continue;
                }
                PointerProtoPte = NULL;
            }

            Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

            MI_MAKE_VALID_PTE (TempPte,
                               PteContents.u.Hard.PageFrameNumber,
                               ProtectionMask,
                               PointerPte);

            //
            // Note the dirty and write bits get turned off here.
            // Any existing pagefile addresses for clean pages are preserved.
            //

            if (MI_IS_PTE_DIRTY (PteContents)) {
                MI_CAPTURE_DIRTY_BIT_TO_PFN (&PteContents, Pfn1);
            }

            //
            // If the new protection is also directly writable, then preserve
            // the dirty and write bits so subsequent accesses do not fault.
            //

            if ((ProtectionMask == MM_READWRITE) ||
                (ProtectionMask == MM_EXECUTE_READWRITE)) {

                if (PteContents.u.Hard.Dirty == 1) {
                    TempPte.u.Hard.Dirty = 1;
                }

#if !defined(NT_UP)
#if defined(_X86_) || defined(_AMD64_)
                TempPte.u.Hard.Writable = 1;
#endif
#endif
            }

            if (PteContents.u.Hard.Accessed == 1) {
                TempPte.u.Hard.Accessed = 1;
            }

            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

            if (PointerProtoPte != NULL) {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerProtoPte, TempPte);
            }

            if (FlushCount < MM_MAXIMUM_FLUSH_COUNT) {
                VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                VaFlushList[FlushCount] = VirtualAddress;
                FlushCount += 1;
            }

        }
        else if (PteContents.u.Soft.Prototype == 1) {

            //
            // WITH REGARDS TO SESSION SPACE :
            //
            // Nothing needs to be done if the image was linked with
            // greater than or equal to PAGE_SIZE subsection alignment
            // because image section creation assigned proper protections
            // to each subsection.
            //
            // However, if the image had less than PAGE_SIZE subsection
            // alignment, then image creation uses a single copyonwrite
            // subsection to control the entire image, so individual
            // protections need to be applied now.  Note well - this must
            // only be done *ONCE* when the image is first loaded - subsequent
            // loads of this image in other sessions do not need to update
            // the common prototype PTEs.
            //

            PointerProtoPte = MiPteToProto (PointerPte);

            ASSERT (!MI_IS_PHYSICAL_ADDRESS (PointerProtoPte));
            PointerPde = MiGetPteAddress (PointerProtoPte);

            if (PointerPde->u.Hard.Valid == 0) {

                if (SessionAddress == TRUE) {

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (CurrentThread, Ws);

                    LOCK_WORKING_SET (CurrentThread, &MmSystemCacheWs);

                    LOCK_PFN (OldIrql);
                }

                MiMakeSystemAddressValidPfnSystemWs (PointerProtoPte, OldIrql);

                if (SessionAddress == TRUE) {

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (CurrentThread, &MmSystemCacheWs);

                    LOCK_WORKING_SET (CurrentThread, Ws);

                    LOCK_PFN (OldIrql);
                }

                //
                // The world may have changed while we waited.
                //

                continue;
            }

            PteContents = *PointerProtoPte;

            if (PteContents.u.Long != 0) {

                if (PteContents.u.Hard.Valid == 1) {
                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
                    Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                }
                else {
                    PointerProtoPte->u.Soft.Protection = ProtectionMask;
    
                    if ((PteContents.u.Soft.Prototype == 0) &&
                        (PteContents.u.Soft.Transition == 1)) {
                        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
                        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                    }
                }
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

            //
            // This page is already private so if copy on write is specified,
            // skip this page.  This is very unusual.
            //

            if ((ProtectionMask & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK) {
                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                PointerPte->u.Soft.Protection = ProtectionMask;
            }
        }
        else {

            //
            // Must be page file space or demand zero.
            //

            //
            // This page is already private so if copy on write is specified,
            // skip this page.  This is very unusual.
            //

            if ((ProtectionMask & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK) {
                PointerPte->u.Soft.Protection = ProtectionMask;
            }
        }
        PointerPte += 1;
    }

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
        MI_FLUSH_ENTIRE_TB (0x17);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WORKING_SET (CurrentThread, Ws);

    return;
}

MM_PROTECTION_MASK
MiComputeDriverProtection (
    IN LOGICAL SessionDriver,
    IN ULONG SectionProtection
    )

/*++

Routine Description:

    This function computes the driver protection mask for an image.

Arguments:

    SessionDriver - Supplies TRUE if this is for a session space driver.

    SectionProtection - Supplies protection from the image header.

Return Value:

    None.

--*/

{
    MM_PROTECTION_MASK NewProtection;

    NewProtection = MM_ZERO_ACCESS;

    if (SectionProtection != 0) {

        //
        // Always make images executable (unless they're completely
        // no-access) for compatibility.
        //

#if !defined (_WIN64)
        SectionProtection |= IMAGE_SCN_MEM_EXECUTE;
#endif

        if (MmEnforceWriteProtection == 0) {
            SectionProtection |= (IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE);
        }
    }

    if (SectionProtection & IMAGE_SCN_MEM_EXECUTE) {
        NewProtection |= MM_EXECUTE;
    }

    if (SectionProtection & IMAGE_SCN_MEM_READ) {
        NewProtection |= MM_READONLY;
    }

    if (SectionProtection & IMAGE_SCN_MEM_WRITE) {

        if (SessionDriver == TRUE) {

            if (NewProtection & MM_EXECUTE) {
                NewProtection = MM_EXECUTE_WRITECOPY;
            }
            else {
                NewProtection = MM_WRITECOPY;
            }
        }
        else {
            if (NewProtection & MM_EXECUTE) {
                NewProtection = MM_EXECUTE_READWRITE;
            }
            else {
                NewProtection = MM_READWRITE;
            }
        }
    }

    if (NewProtection == MM_ZERO_ACCESS) {
        NewProtection = MM_NOACCESS;
    }

    return NewProtection;
}

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    )

/*++

Routine Description:

    This function sets the protection of a system component as specified.

Arguments:

    DllBase - Supplies the base address of the system component.

Return Value:

    None.

--*/

{
    ULONG RelevantSectionProtections;
    ULONG LastSectionProtection;
    ULONG SectionProtection;
    ULONG MergedSectionProtection;
    ULONG NumberOfSubsections;
    ULONG SectionVirtualSize;
    ULONG OffsetToSectionTable;
    PFN_NUMBER NumberOfPtes;
    ULONG_PTR VirtualAddress;
    PVOID LastVirtualAddress;
    PMMPTE PointerPte;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PMMPTE LastImagePte;
    PMMPTE MergedPte;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    MM_PROTECTION_MASK LastProtection;
    MM_PROTECTION_MASK MergedProtection;
    LOGICAL SessionDriver;

    PAGED_CODE();

    if (MI_IS_PHYSICAL_ADDRESS (DllBase)) {
        return;
    }

    NtHeader = RtlImageNtHeader (DllBase);

    if (NtHeader == NULL) {
        return;
    }

    if (MI_IS_SESSION_ADDRESS (DllBase) == 0) {

        //
        // Images prior to Win2000 were not protected from stepping all over
        // their (and others) code and readonly data.  Here we somewhat
        // preserve that behavior, but don't allow them to step on anyone else.
        //

        if (NtHeader->OptionalHeader.MajorOperatingSystemVersion < 5) {
            return;
        }

        if (NtHeader->OptionalHeader.MajorImageVersion < 5) {
            return;
        }
        SessionDriver = FALSE;
    }
    else {
        SessionDriver = TRUE;
    }

    //
    // Don't include IMAGE_SCN_MEM_SHARED because it only applies to session
    // drivers and that's handled by MiSessionProcessGlobalSubsections.
    //

    RelevantSectionProtections = (IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE);

    //
    // If the image has section alignment of at least PAGE_SIZE, then
    // the image section was created with individual subsections and
    // proper permissions already applied to the prototype PTEs.  However,
    // our caller may have been changing the individual PTE protections
    // in order to relocate the image, so march on regardless of section
    // alignment.
    //

    NumberOfPtes = BYTES_TO_PAGES (NtHeader->OptionalHeader.SizeOfImage);

    FileHeader = &NtHeader->FileHeader;

    NumberOfSubsections = FileHeader->NumberOfSections;

    ASSERT (NumberOfSubsections != 0);

    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              FileHeader->SizeOfOptionalHeader;

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            OffsetToSectionTable);

    //
    // Verify the image contains subsections ordered by increasing virtual
    // address and that there are no overlaps.
    //

    LastVirtualAddress = DllBase;

    for ( ; NumberOfSubsections > 0; NumberOfSubsections -= 1, SectionTableEntry += 1) {

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        }
        else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        VirtualAddress = (ULONG_PTR)DllBase + SectionTableEntry->VirtualAddress;
        if ((PVOID)VirtualAddress <= LastVirtualAddress) {

            //
            // Subsections are not in an increasing virtual address ordering.
            // No protection is provided for such a poorly linked image.
            //

            KdPrint (("MM:sysload - Image at %p is badly linked\n", DllBase));
            return;
        }
        LastVirtualAddress = (PVOID)((PCHAR)VirtualAddress + SectionVirtualSize - 1);
    }

    NumberOfSubsections = FileHeader->NumberOfSections;
    ASSERT (NumberOfSubsections != 0);

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            OffsetToSectionTable);

    LastVirtualAddress = (PVOID)((ULONG_PTR)(SectionTableEntry + NumberOfSubsections) - 1);

    //
    // Set writable PTE here so the image headers are excluded.  This is
    // needed so that locking down of sections can continue to edit the
    // image headers for counts.
    //

    LastSectionProtection = (IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ);

    FirstPte = MiGetPteAddress (DllBase);
    LastImagePte = MiGetPteAddress(DllBase) + NumberOfPtes;

    MergedPte = NULL;
    MergedSectionProtection = 0;

    for ( ; NumberOfSubsections > 0; NumberOfSubsections -= 1, SectionTableEntry += 1) {

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        }
        else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        VirtualAddress = (ULONG_PTR)DllBase + SectionTableEntry->VirtualAddress;

        PointerPte = MiGetPteAddress ((PVOID)VirtualAddress);

        if ((MergedPte != NULL) && (PointerPte > MergedPte)) {

            //
            // Apply the merged protection to the prior straddling PTE.
            //

            MergedProtection = MiComputeDriverProtection (SessionDriver,
                                                          MergedSectionProtection);
            MiSetSystemCodeProtection (MergedPte, MergedPte, MergedProtection);

            if (MergedPte == FirstPte) {

                //
                // The merged PTE overlapped with the first PTE of the prior
                // subsection, so increment the first PTE now.
                //

                FirstPte += 1;
            }

            MergedPte = NULL;
            MergedSectionProtection = 0;
        }

        if (PointerPte >= LastImagePte) {

            //
            // Skip relocation subsections (which aren't given VA space).
            //

            break;
        }

        SectionProtection = (SectionTableEntry->Characteristics & RelevantSectionProtections);

        if (SectionProtection == LastSectionProtection) {

            //
            // Merge adjacent subsections that have the same protection.
            //

            LastVirtualAddress = (PVOID)((PCHAR)VirtualAddress + SectionVirtualSize - 1);
            continue;
        }

        //
        // This section has different permissions from the previous one so
        // set the protections on the previous one now.
        //

        //
        // Update the PTE protections.  Make sure if it's sharing
        // a PTE that the last PTE is set to the most relaxed
        // permission combination.
        //

        LastPte = (PVOID) MiGetPteAddress (LastVirtualAddress);

        if (LastPte == PointerPte) {

            //
            // Don't include the last PTE from the previous subsection as it
            // overlaps with this one (and may overlap with the next subsection
            // if this one is small!).
            //

            LastPte -= 1;

            //
            // Compute the most relaxed permission for the straddling PTE
            // by merging subsection protections, but don't apply it till
            // we have ascertained that there are no more subsections which
            // can overlap into this PTE.
            //

            if (MergedPte != NULL) {
                ASSERT (MergedPte == PointerPte);
            }

            MergedSectionProtection |= (SectionProtection | LastSectionProtection);

            MergedPte = PointerPte;
        }

        if (LastPte >= FirstPte) {

            ASSERT (FirstPte < LastImagePte);

            if (LastPte >= LastImagePte) {
                LastPte = LastImagePte - 1;
            }

            ASSERT (LastPte >= FirstPte);

            LastProtection = MiComputeDriverProtection (SessionDriver,
                                                        LastSectionProtection);

            MiSetSystemCodeProtection (FirstPte, LastPte, LastProtection);
        }

        //
        // Initialize variables to describe the current subsection as it
        // is the start of a new run.
        //

        FirstPte = PointerPte;
        LastVirtualAddress = (PVOID)((PCHAR)VirtualAddress + SectionVirtualSize - 1);
        LastSectionProtection = SectionProtection;
    }

    if (MergedPte != NULL) {

        //
        // Apply the merged protection to the prior straddling PTE.
        //

        MergedProtection = MiComputeDriverProtection (SessionDriver,
                                                      MergedSectionProtection);

        MiSetSystemCodeProtection (MergedPte, MergedPte, MergedProtection);

        if (MergedPte == FirstPte) {

            //
            // The merged PTE overlapped with the first PTE of the prior
            // subsection, so increment the first PTE now.
            //

            FirstPte += 1;
        }

        MergedPte = NULL;
        MergedSectionProtection = 0;
    }

    //
    // Set the protections on the last subsection's PTEs now.
    //

    LastPte = (PVOID) MiGetPteAddress (LastVirtualAddress);

    if ((FirstPte < LastImagePte) && (LastPte >= FirstPte)) {

        if (LastPte >= LastImagePte) {
            LastPte = LastImagePte - 1;
        }

        ASSERT (LastPte >= FirstPte);

        LastProtection = MiComputeDriverProtection (SessionDriver,
                                                    LastSectionProtection);

        MiSetSystemCodeProtection (FirstPte, LastPte, LastProtection);
    }

    return;
}


VOID
MiSessionProcessGlobalSubsections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

/*++

Routine Description:

    This function sets the protection of a session driver's subsections
    to globally shared if their PE header specifies them as such.

Arguments:

    DataTableEntry - Supplies the loaded module list entry for the driver.

Return Value:

    None.

--*/

{
    PVOID DllBase;
    PSUBSECTION Subsection;
    PMMPTE RealPteBase;
    PMMPTE PrototypePteBase;
    PCONTROL_AREA ControlArea;
    PIMAGE_NT_HEADERS NtHeader;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    LOGICAL GlobalSubsectionSupport;
    PIMAGE_ENTRY_IN_SESSION Image;
    ULONG Count;

    PAGED_CODE();

    Image = MiSessionLookupImage (DataTableEntry->DllBase);

    if (Image != NULL) {
        ASSERT (MmSessionSpace->ImageLoadingCount >= 0);

        if (Image->ImageLoading == TRUE) {
            Image->ImageLoading = FALSE;
            ASSERT (MmSessionSpace->ImageLoadingCount > 0);
            InterlockedDecrement (&MmSessionSpace->ImageLoadingCount);
        }
    }
    else {
        ASSERT (FALSE);
    }

    DllBase = DataTableEntry->DllBase;

    ControlArea = ((PSECTION)DataTableEntry->SectionPointer)->Segment->ControlArea;

    ASSERT (MI_IS_PHYSICAL_ADDRESS(DllBase) == FALSE);

    ASSERT (MI_IS_SESSION_ADDRESS(DllBase));

    NtHeader = RtlImageNtHeader (DllBase);

    ASSERT (NtHeader);

    if (NtHeader->OptionalHeader.SectionAlignment < PAGE_SIZE) {
        if (Image->GlobalSubs != NULL) {
            ExFreePool (Image->GlobalSubs);
            Image->GlobalSubs = NULL;
        }
        return;
    }

    //
    // Win XP and Win2000 did not support global shared subsections
    // for session images.  To ensure backwards compatibility for existing
    // drivers, ensure that only newer ones get this feature.
    //

    GlobalSubsectionSupport = FALSE;

    if (NtHeader->OptionalHeader.MajorOperatingSystemVersion > 5) {
        GlobalSubsectionSupport = TRUE;
    }
    else if (NtHeader->OptionalHeader.MajorOperatingSystemVersion == 5) {

        if (NtHeader->OptionalHeader.MinorOperatingSystemVersion > 1) {
            GlobalSubsectionSupport = TRUE;
        }
        else if (NtHeader->OptionalHeader.MinorOperatingSystemVersion == 1) {
            if (NtHeader->OptionalHeader.MajorImageVersion > 5) {
                GlobalSubsectionSupport = TRUE;
            }
            else if (NtHeader->OptionalHeader.MajorImageVersion == 5) {
                if (NtHeader->OptionalHeader.MinorImageVersion > 1) {
                    GlobalSubsectionSupport = TRUE;
                }
                else if (NtHeader->OptionalHeader.MinorImageVersion == 1) {
                    if (NtHeader->OptionalHeader.MajorSubsystemVersion > 5) {
                        GlobalSubsectionSupport = TRUE;
                    }
                    else if (NtHeader->OptionalHeader.MajorSubsystemVersion == 5) {
                        if (NtHeader->OptionalHeader.MinorSubsystemVersion >= 2) {
                            GlobalSubsectionSupport = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (GlobalSubsectionSupport == FALSE) {
        if (Image->GlobalSubs != NULL) {
            ExFreePool (Image->GlobalSubs);
            Image->GlobalSubs = NULL;
        }
        return;
    }

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    RealPteBase = MiGetPteAddress (DllBase);
    PrototypePteBase = Subsection->SubsectionBase;

    //
    // Loop through all the subsections.
    //

    if (ControlArea->u.Flags.Image == 1) {

        do {

            if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {
    
                PointerPte = RealPteBase + (Subsection->SubsectionBase - PrototypePteBase);
                LastPte = PointerPte + Subsection->PtesInSubsection - 1;
    
                MiSetSystemCodeProtection (PointerPte,
                                           LastPte,
                                           Subsection->u.SubsectionFlags.Protection);
            }
    
            Subsection = Subsection->NextSubsection;
    
        } while (Subsection != NULL);
    }
    else if (Image->GlobalSubs != NULL) {

        Count = 0;
        ASSERT (Subsection->NextSubsection == NULL);

        while (Image->GlobalSubs[Count].PteCount != 0) {

            PointerPte = RealPteBase + Image->GlobalSubs[Count].PteIndex;
            LastPte = PointerPte + Image->GlobalSubs[Count].PteCount - 1;

            MiSetSystemCodeProtection (PointerPte,
                                       LastPte,
                                       Image->GlobalSubs[Count].Protection);
    
            Count += 1;
        }

        ExFreePool (Image->GlobalSubs);
        Image->GlobalSubs = NULL;
    }

    return;
}


VOID
MiUpdateThunks (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PVOID OldAddress,
    IN PVOID NewAddress,
    IN ULONG NumberOfBytes
    )

/*++

Routine Description:

    This function updates the IATs of all the loaded modules in the system
    to handle a newly relocated image.

Arguments:

    LoaderBlock - Supplies a pointer to the system loader block.

    OldAddress - Supplies the old address of the DLL which was just relocated.

    NewAddress - Supplies the new address of the DLL which was just relocated.

    NumberOfBytes - Supplies the number of bytes spanned by the DLL.

Return Value:

    None.

--*/

{
    PULONG_PTR ImportThunk;
    ULONG_PTR OldAddressHigh;
    ULONG_PTR AddressDifference;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ULONG_PTR i;
    ULONG ImportSize;

    //
    // Note this routine must not call any modules outside the kernel.
    // This is because that module may itself be the one being relocated right
    // now.
    //

    OldAddressHigh = (ULONG_PTR)((PCHAR)OldAddress + NumberOfBytes - 1);
    AddressDifference = (ULONG_PTR)NewAddress - (ULONG_PTR)OldAddress;

    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        ImportThunk = (PULONG_PTR) RtlImageDirectoryEntryToData (
                                           DataTableEntry->DllBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_IAT,
                                           &ImportSize);

        if (ImportThunk == NULL) {
            continue;
        }

        ImportSize /= sizeof(PULONG_PTR);

        for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {
            if (*ImportThunk >= (ULONG_PTR)OldAddress && *ImportThunk <= OldAddressHigh) {
                *ImportThunk += AddressDifference;
            }
        }
    }
}


VOID
MiReloadBootLoadedDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    The kernel, HAL and boot drivers are relocated by the loader.
    All the boot drivers are then relocated again here.

    This function relocates osloader-loaded images into system PTEs.  This
    gives these images the benefits that all other drivers already enjoy,
    including :

    1. Paging of the drivers (this is more than 500K today).
    2. Write-protection of their text sections.
    3. Automatic unload of drivers on last dereference.

    Note care must be taken when processing HIGHADJ relocations more than once.

Arguments:

    LoaderBlock - Supplies a pointer to the system loader block.

Return Value:

    None.

Environment:

    Kernel mode, Phase 0 Initialization.

--*/

{
    LOGICAL UsedLargePage;
    LOGICAL HasRelocations;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    ULONG_PTR i;
    ULONG RoundedNumberOfPtes;
    ULONG NumberOfPtes;
    ULONG NumberOfLoaderPtes;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LoaderPte;
    MMPTE PteContents;
    MMPTE TempPte;
    PVOID LoaderImageAddress;
    PVOID NewImageAddress;
    NTSTATUS Status;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PteFramePage;
    PMMPTE PteFramePointer;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PCHAR RelocatedVa;
    PCHAR NonRelocatedVa;
    LOGICAL StopMoving;

#if !defined (_X86_)

    //
    // Only try to preserve low memory on x86 machines.
    //

    MmMakeLowMemory = FALSE;
#endif
    StopMoving = FALSE;

    i = 0;
    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

        //
        // Skip the kernel and the HAL.  Note their relocation sections will
        // be automatically reclaimed.
        //

        i += 1;
        if (i <= 2) {
            continue;
        }

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        NtHeader = RtlImageNtHeader (DataTableEntry->DllBase);

        //
        // Ensure that the relocation section exists and that the loader
        // hasn't freed it already.
        //

        if (NtHeader == NULL) {
            continue;
        }

        FileHeader = &NtHeader->FileHeader;

        if (FileHeader->Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
            continue;
        }

        if (IMAGE_DIRECTORY_ENTRY_BASERELOC >= NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
            continue;
        }

        DataDirectory = &NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        if (DataDirectory->VirtualAddress == 0) {
            HasRelocations = FALSE;
        }
        else {

            if (DataDirectory->VirtualAddress + DataDirectory->Size > DataTableEntry->SizeOfImage) {

                //
                // The relocation section has already been freed, the user must
                // be using an old loader that didn't save the relocations.
                //

                continue;
            }
            HasRelocations = TRUE;
        }

        LoaderImageAddress = DataTableEntry->DllBase;
        LoaderPte = MiGetPteAddress(DataTableEntry->DllBase);
        NumberOfLoaderPtes = (ULONG)((ROUND_TO_PAGES(DataTableEntry->SizeOfImage)) >> PAGE_SHIFT);

        LOCK_PFN (OldIrql);

        PointerPte = LoaderPte;
        LastPte = PointerPte + NumberOfLoaderPtes;

        while (PointerPte < LastPte) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            //
            // Mark the page as modified so boot drivers that call
            // MmPageEntireDriver don't lose their unmodified data !
            //

            MI_SET_MODIFIED (Pfn1, 1, 0x14);

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);

        NumberOfPtes = NumberOfLoaderPtes;

        NewImageAddress = LoaderImageAddress;

        UsedLargePage = MiUseLargeDriverPage (NumberOfPtes,
                                              &NewImageAddress,
                                              &DataTableEntry->BaseDllName,
                                              0);

        if (UsedLargePage == TRUE) {

            //
            // This image has been loaded into a large page mapping.
            //

            RelocatedVa = NewImageAddress;
            NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;
            PointerPte -= NumberOfPtes;
            goto Fixup;
        }

        //
        // Extra PTEs are allocated here to map the relocation section at the
        // new address so the image can be relocated.
        //

        PointerPte = MiReserveSystemPtes (NumberOfPtes, SystemPteSpace);

        if (PointerPte == NULL) {
            continue;
        }

        LastPte = PointerPte + NumberOfPtes;

        NewImageAddress = MiGetVirtualAddressMappedByPte (PointerPte);

        //
        // This assert is important because the assumption is made that PTEs
        // (not superpages) are mapping these drivers.
        //

        ASSERT (InitializationPhase == 0);

        //
        // If the system is configured to make low memory available for ISA
        // type drivers, then copy the boot loaded drivers now.  Otherwise
        // only PTE adjustment is done.  Presumably some day when ISA goes
        // away this code can be removed.
        //

        RelocatedVa = NewImageAddress;
        NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;

        while (PointerPte < LastPte) {

            PteContents = *LoaderPte;
            ASSERT (PteContents.u.Hard.Valid == 1);

            if (MmMakeLowMemory == TRUE) {
#if DBG
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (LoaderPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->u1.WsIndex == 0);
#endif
                LOCK_PFN (OldIrql);

                if (MmAvailablePages < MM_HIGH_LIMIT) {
                    MiEnsureAvailablePageOrWait (NULL, OldIrql);
                }

                PageFrameIndex = MiRemoveAnyPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

                if (PageFrameIndex < (16*1024*1024)/PAGE_SIZE) {

                    //
                    // If the frames cannot be replaced with high pages
                    // then stop copying.
                    //

#if defined (_MI_MORE_THAN_4GB_)
                  if (MiNoLowMemory == 0)
#endif
                    StopMoving = TRUE;
                }

                MI_MAKE_VALID_KERNEL_PTE (TempPte,
                                          PageFrameIndex,
                                          MM_EXECUTE_READWRITE,
                                          PointerPte);

                MI_SET_PTE_DIRTY (TempPte);
                MI_SET_ACCESSED_IN_PTE (&TempPte, 1);

                MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPte, TempPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                MI_SET_MODIFIED (Pfn1, 1, 0x15);

                //
                // Initialize the WsIndex just like the original page had it.
                //

                Pfn1->u1.WsIndex = 0;

                UNLOCK_PFN (OldIrql);
                RtlCopyMemory (RelocatedVa, NonRelocatedVa, PAGE_SIZE);
                RelocatedVa += PAGE_SIZE;
                NonRelocatedVa += PAGE_SIZE;
            }
            else {
                MI_MAKE_VALID_KERNEL_PTE (TempPte,
                                          PteContents.u.Hard.PageFrameNumber,
                                          MM_EXECUTE_READWRITE,
                                          PointerPte);

                MI_SET_PTE_DIRTY (TempPte);

                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }

            PointerPte += 1;
            LoaderPte += 1;
        }
        PointerPte -= NumberOfPtes;

Fixup:

        ASSERT (*(PULONG)NewImageAddress == *(PULONG)LoaderImageAddress);

        //
        // Image is now mapped at the new address.  Relocate it (again).
        //

        NtHeader->OptionalHeader.ImageBase = (ULONG_PTR)LoaderImageAddress;
        if ((MmMakeLowMemory == TRUE) || (UsedLargePage == TRUE)) {
            PIMAGE_NT_HEADERS NtHeader2;

            NtHeader2 = (PIMAGE_NT_HEADERS)((PCHAR)NtHeader + (RelocatedVa - NonRelocatedVa));
            NtHeader2->OptionalHeader.ImageBase = (ULONG_PTR)LoaderImageAddress;
        }

        if (HasRelocations == TRUE) {
            Status = (NTSTATUS)LdrRelocateImage(NewImageAddress,
                                            (CONST PCHAR)"SYSLDR",
                                            (ULONG)STATUS_SUCCESS,
                                            (ULONG)STATUS_CONFLICTING_ADDRESSES,
                                            (ULONG)STATUS_INVALID_IMAGE_FORMAT
                                            );

            if (!NT_SUCCESS(Status)) {

                if (UsedLargePage == TRUE) {
                    ASSERT (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (NewImageAddress)));
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPdeAddress (NewImageAddress)) + MiGetPteOffset (NewImageAddress);

                    RoundedNumberOfPtes = MI_ROUND_TO_SIZE (NumberOfPtes,
                                              MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
                    MiUnmapLargePages (NewImageAddress,
                                       RoundedNumberOfPtes << PAGE_SHIFT);

                    MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + RoundedNumberOfPtes - 1);
                    MiFreeContiguousPages (PageFrameIndex, RoundedNumberOfPtes);
                }

                if (MmMakeLowMemory == TRUE) {

                    while (PointerPte < LastPte) {

                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

                        MI_SET_PFN_DELETED (Pfn1);
                        MiDecrementShareCount (Pfn1, PageFrameIndex);

                        PointerPte += 1;
                    }
                    PointerPte -= NumberOfPtes;
                }

                MiReleaseSystemPtes (PointerPte, NumberOfPtes, SystemPteSpace);

                if (StopMoving == TRUE) {
                    MmMakeLowMemory = FALSE;
                }

                continue;
            }
        }

        //
        // Update the IATs for all other loaded modules that reference this one.
        //

        NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;
        DataTableEntry->DllBase = NewImageAddress;

        MiUpdateThunks (LoaderBlock,
                        LoaderImageAddress,
                        NewImageAddress,
                        DataTableEntry->SizeOfImage);


        //
        // Update the loaded module list entry.
        //

        DataTableEntry->Flags |= LDRP_SYSTEM_MAPPED;
        DataTableEntry->DllBase = NewImageAddress;
        DataTableEntry->EntryPoint =
            (PVOID)((PCHAR)NewImageAddress + NtHeader->OptionalHeader.AddressOfEntryPoint);
        DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;

        //
        // Update the exception table data info
        //

        MiCaptureImageExceptionValues (DataTableEntry);

        //
        // Update the PFNs of the image to support trimming.
        // Note that the loader addresses are freed now so no references
        // to it are permitted after this point.
        //

        LoaderPte = MiGetPteAddress (NonRelocatedVa);

        LOCK_PFN (OldIrql);

        while (PointerPte < LastPte) {

            ASSERT ((UsedLargePage == TRUE) || (PointerPte->u.Hard.Valid == 1));

            if ((MmMakeLowMemory == TRUE) || (UsedLargePage == TRUE)) {

                ASSERT (LoaderPte->u.Hard.Valid == 1);
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (LoaderPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                //
                // Decrement the share count on the original page table
                // page so it can be freed.
                //

                MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

                MI_SET_PFN_DELETED (Pfn1);
                MiDecrementShareCount (Pfn1, PageFrameIndex);
                LoaderPte += 1;
            }
            else {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                //
                // Decrement the share count on the original page table
                // page so it can be freed.
                //

                MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
                Pfn1->PteAddress->u.Long = 0;

                //
                // Chain the PFN entry to its new page table.
                //

                PteFramePointer = MiGetPteAddress(PointerPte);
                PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);

                Pfn1->u4.PteFrame = PteFramePage;
                Pfn1->PteAddress = PointerPte;

                //
                // Increment the share count for the page table page that now
                // contains the PTE that was copied.
                //

                Pfn2 = MI_PFN_ELEMENT (PteFramePage);
                Pfn2->u2.ShareCount += 1;
            }

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);

        //
        // The physical pages mapping the relocation section are freed
        // later with the rest of the initialization code spanned by the
        // DataTableEntry->SizeOfImage.
        //

        if (StopMoving == TRUE) {
            MmMakeLowMemory = FALSE;
        }
    }

    //
    // Flush the instruction cache on all systems in the configuration.
    //

    KeSweepIcache (TRUE);
}

#if defined(_X86_) || defined(_AMD64_)
PMMPTE MiKernelResourceStartPte;
PMMPTE MiKernelResourceEndPte;
#endif

VOID
MiLocateKernelSections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

/*++

Routine Description:

    This function locates the resource section in the kernel so it can be
    made readwrite if we bugcheck later, as the bugcheck code will write
    into it.

Arguments:

    DataTableEntry - Supplies the kernel's data table entry.

Return Value:

    None.

Environment:

    Kernel mode, Phase 0 Initialization.

--*/

{
    ULONG Span;
    PVOID CurrentBase;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    LONG i;
    PMMPTE PointerPte;
    PVOID SectionBaseAddress;

    CurrentBase = (PVOID) DataTableEntry->DllBase;

    NtHeader = RtlImageNtHeader (CurrentBase);

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            sizeof(ULONG) +
                            sizeof(IMAGE_FILE_HEADER) +
                            NtHeader->FileHeader.SizeOfOptionalHeader);

    //
    // From the image header, locate the section named '.rsrc'.
    //

    i = NtHeader->FileHeader.NumberOfSections;

    PointerPte = NULL;

    while (i > 0) {

        SectionBaseAddress = SECTION_BASE_ADDRESS(SectionTableEntry);

        //
        // Generally, SizeOfRawData is larger than VirtualSize for each
        // section because it includes the padding to get to the subsection
        // alignment boundary.  However, if the image is linked with
        // subsection alignment == native page alignment, the linker will
        // have VirtualSize be much larger than SizeOfRawData because it
        // will account for all the bss.
        //

        Span = SectionTableEntry->SizeOfRawData;

        if (Span < SectionTableEntry->Misc.VirtualSize) {
            Span = SectionTableEntry->Misc.VirtualSize;
        }

#if defined(_X86_) || defined(_AMD64_)
        if (*(PULONG)SectionTableEntry->Name == 'rsr.') {

            MiKernelResourceStartPte = MiGetPteAddress ((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);

            MiKernelResourceEndPte = MiGetPteAddress (ROUND_TO_PAGES((ULONG_PTR)CurrentBase +
                         SectionTableEntry->VirtualAddress + Span));
            break;
        }
#endif
        if (*(PULONG)SectionTableEntry->Name == 'LOOP') {
            if (*(PULONG)&SectionTableEntry->Name[4] == 'EDOC') {
                ExPoolCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                ExPoolCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
            }
            else if (*(PUSHORT)&SectionTableEntry->Name[4] == 'IM') {
                MmPoolCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                MmPoolCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
            }
        }
        else if ((*(PULONG)SectionTableEntry->Name == 'YSIM') &&
                 (*(PULONG)&SectionTableEntry->Name[4] == 'ETPS')) {
                MmPteCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                MmPteCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
        }

        i -= 1;
        SectionTableEntry += 1;
    }
}

VOID
MmMakeKernelResourceSectionWritable (
    VOID
    )

/*++

Routine Description:

    This function makes the kernel's resource section readwrite so the bugcheck
    code can write into it.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.  Any IRQL.

--*/

{
#if defined(_X86_) || defined(_AMD64_)
    MMPTE TempPte;
    MMPTE PteContents;
    PMMPTE PointerPte;

    if (MiKernelResourceStartPte == NULL) {
        return;
    }

    PointerPte = MiKernelResourceStartPte;

    if (MI_IS_PHYSICAL_ADDRESS (MiGetVirtualAddressMappedByPte (PointerPte))) {

        //
        // Mapped physically, doesn't need to be made readwrite.
        //

        return;
    }

    //
    // Since the entry state and IRQL are unknown, just go through the
    // PTEs without a lock and make them all readwrite.
    //

    do {
        PteContents = *PointerPte;
#if defined(NT_UP)
        if (PteContents.u.Hard.Write == 0)
#else
        if (PteContents.u.Hard.Writable == 0)
#endif
        {
            MI_MAKE_VALID_KERNEL_PTE (TempPte,
                                      PteContents.u.Hard.PageFrameNumber,
                                      MM_READWRITE,
                                      PointerPte);
#if !defined(NT_UP)
            TempPte.u.Hard.Writable = 1;
#endif
            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
        }
        PointerPte += 1;
    } while (PointerPte < MiKernelResourceEndPte);

    //
    // Don't do this more than once.
    //

    MiKernelResourceStartPte = NULL;

    //
    // Only flush this processor as the state of the others is unknown.
    //

    MI_FLUSH_CURRENT_TB ();
#endif
}

#ifdef i386
PVOID PsNtosImageBase = (PVOID)0x80100000;
#else
PVOID PsNtosImageBase;
#endif

#if DBG
PVOID PsNtosImageEnd;
#endif

#if defined (_WIN64)

INVERTED_FUNCTION_TABLE PsInvertedFunctionTable = {
    0, MAXIMUM_INVERTED_FUNCTION_TABLE_SIZE, FALSE};

#endif

LIST_ENTRY PsLoadedModuleList;
ERESOURCE PsLoadedModuleResource;

LOGICAL
MiInitializeLoadedModuleList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function initializes the loaded module list based on the LoaderBlock.

Arguments:

    LoaderBlock - Supplies a pointer to the system loader block.

Return Value:

    None.

Environment:

    Kernel mode, Phase 0 Initialization.

--*/

{
    SIZE_T CommittedPages;
    SIZE_T DataTableEntrySize;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY NextEntryEnd;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry1;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry2;

    CommittedPages = 0;

    //
    // Initialize the loaded module list executive resource and spin lock.
    //

    ExInitializeResourceLite (&PsLoadedModuleResource);
    KeInitializeSpinLock (&PsLoadedModuleSpinLock);

    InitializeListHead (&PsLoadedModuleList);

    //
    // Scan the loaded module list and allocate and initialize a data table
    // entry for each module. The data table entry is inserted in the loaded
    // module list and the initialization order list in the order specified
    // in the loader parameter block. The data table entry is inserted in the
    // memory order list in memory order.
    //

    NextEntry = LoaderBlock->LoadOrderListHead.Flink;
    NextEntryEnd = &LoaderBlock->LoadOrderListHead;

    DataTableEntry2 = CONTAINING_RECORD (NextEntry,
                                         KLDR_DATA_TABLE_ENTRY,
                                         InLoadOrderLinks);

    PsNtosImageBase = DataTableEntry2->DllBase;

#if DBG
    PsNtosImageEnd = (PVOID) ((ULONG_PTR) DataTableEntry2->DllBase + DataTableEntry2->SizeOfImage);
#endif

    MiLocateKernelSections (DataTableEntry2);

    while (NextEntry != NextEntryEnd) {

        DataTableEntry2 = CONTAINING_RECORD(NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        //
        // Allocate a data table entry.
        //

        DataTableEntrySize = sizeof (KLDR_DATA_TABLE_ENTRY) +
            DataTableEntry2->BaseDllName.MaximumLength + sizeof(UNICODE_NULL);

        DataTableEntry1 = ExAllocatePoolWithTag (NonPagedPool,
                                                 DataTableEntrySize,
                                                 'dLmM');

        if (DataTableEntry1 == NULL) {
            return FALSE;
        }

        //
        // Copy the data table entry.
        //

        *DataTableEntry1 = *DataTableEntry2;

        //
        // Clear fields we may use later so they don't inherit irrelevant
        // loader values.
        //

        ((PKLDR_DATA_TABLE_ENTRY)DataTableEntry1)->NonPagedDebugInfo = NULL;
        DataTableEntry1->PatchInformation = NULL;

        DataTableEntry1->FullDllName.Buffer = ExAllocatePoolWithTag (PagedPool,
            DataTableEntry2->FullDllName.MaximumLength + sizeof(UNICODE_NULL),
            'TDmM');

        if (DataTableEntry1->FullDllName.Buffer == NULL) {
            ExFreePool (DataTableEntry1);
            return FALSE;
        }

        DataTableEntry1->BaseDllName.Buffer = (PWSTR)((ULONG_PTR)DataTableEntry1 + sizeof (KLDR_DATA_TABLE_ENTRY));

        //
        // Copy the strings.
        //

        RtlCopyMemory (DataTableEntry1->FullDllName.Buffer,
                       DataTableEntry2->FullDllName.Buffer,
                       DataTableEntry1->FullDllName.MaximumLength);

        RtlCopyMemory (DataTableEntry1->BaseDllName.Buffer,
                       DataTableEntry2->BaseDllName.Buffer,
                       DataTableEntry1->BaseDllName.MaximumLength);

        DataTableEntry1->BaseDllName.Buffer[DataTableEntry1->BaseDllName.Length/sizeof(WCHAR)] = UNICODE_NULL;

        //
        // Always charge commitment regardless of whether we were able to
        // relocate the driver, use large pages, etc.
        //

        CommittedPages += (DataTableEntry1->SizeOfImage >> PAGE_SHIFT);

        //
        // Calculate exception pointers
        //

        MiCaptureImageExceptionValues(DataTableEntry1);

        //
        // Insert the data table entry in the load order list in the order
        // they are specified.
        //

        InsertTailList (&PsLoadedModuleList,
                        &DataTableEntry1->InLoadOrderLinks);

#if defined (_WIN64)

        RtlInsertInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry1->DllBase,
                                        DataTableEntry1->SizeOfImage);

#endif

        NextEntry = NextEntry->Flink;
    }

    //
    // Charge commitment for each boot loaded driver so that if unloads
    // later, the return will balance.  Note that the actual number of
    // free pages is not changing now so the commit limits need to be
    // bumped by the same amount.
    //
    // Resident available does not need to be charged here because it
    // has been already (by virtue of being snapped from available pages).
    //

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LOAD_SYSTEM_IMAGE_TEMP, CommittedPages);

    MmTotalCommittedPages += CommittedPages;
    MmTotalCommitLimit += CommittedPages;
    MmTotalCommitLimitMaximum += CommittedPages;

    MiBuildImportsForBootDrivers ();

    return TRUE;
}

NTSTATUS
MmCallDllInitialize (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PLIST_ENTRY ModuleListHead
    )

/*++

Routine Description:

    This function calls the DLL's initialize routine.

Arguments:

    DataTableEntry - Supplies the kernel's data table entry.

Return Value:

    Various NTSTATUS error codes.

Environment:

    Kernel mode.

--*/

{
    NTSTATUS st;
    PWCHAR Dot;
    PMM_DLL_INITIALIZE Func;
    UNICODE_STRING RegistryPath;
    UNICODE_STRING ImportName;
    ULONG ThunksAdded;

    Func = (PMM_DLL_INITIALIZE)(ULONG_PTR)MiLocateExportName (DataTableEntry->DllBase, "DllInitialize");

    if (!Func) {
        return STATUS_SUCCESS;
    }

    ImportName.MaximumLength = DataTableEntry->BaseDllName.Length;
    ImportName.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                               ImportName.MaximumLength,
                                               'TDmM');

    if (ImportName.Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ImportName.Length = DataTableEntry->BaseDllName.Length;
    RtlCopyMemory (ImportName.Buffer,
                   DataTableEntry->BaseDllName.Buffer,
                   ImportName.Length);

    RegistryPath.MaximumLength = (USHORT)(CmRegistryMachineSystemCurrentControlSetServices.Length +
                                    ImportName.Length +
                                    2*sizeof(WCHAR));

    RegistryPath.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                 RegistryPath.MaximumLength,
                                                 'TDmM');

    if (RegistryPath.Buffer == NULL) {
        ExFreePool (ImportName.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RegistryPath.Length = CmRegistryMachineSystemCurrentControlSetServices.Length;
    RtlCopyMemory (RegistryPath.Buffer,
                   CmRegistryMachineSystemCurrentControlSetServices.Buffer,
                   CmRegistryMachineSystemCurrentControlSetServices.Length);

    RtlAppendUnicodeToString (&RegistryPath, (const PUSHORT)L"\\");
    Dot = wcschr (ImportName.Buffer, L'.');
    if (Dot) {
        ImportName.Length = (USHORT)((Dot - ImportName.Buffer) * sizeof(WCHAR));
    }

    RtlAppendUnicodeStringToString (&RegistryPath, &ImportName);
    ExFreePool (ImportName.Buffer);

    //
    // Save the number of verifier thunks currently added so we know
    // if this activation adds any.  To extend the thunk list, the module
    // performs an NtSetSystemInformation call which calls back to the
    // verifier's MmAddVerifierThunks, which increments MiVerifierThunksAdded.
    //

    ThunksAdded = MiVerifierThunksAdded;

    //
    // Invoke the DLL's initialization routine.
    //

    st = Func (&RegistryPath);

    ExFreePool (RegistryPath.Buffer);

    //
    // If the module's initialization routine succeeded, and if it extended
    // the verifier thunk list, and this is boot time, reapply the verifier
    // to the loaded modules.
    //
    // Note that boot time is the special case because after boot time, Mm
    // loads all the DLLs itself and a DLL initialize is thus guaranteed to
    // complete and add its thunks before the importing driver load finishes.
    // Since the importing driver is only thunked after its load finishes,
    // ordering implicitly guarantees that all DLL-registered thunks are
    // properly factored in to the importing driver.
    //
    // Boot time is special because the loader (not the Mm) already loaded
    // the DLLs *AND* the importing drivers so we have to look over our
    // shoulder and make it all right after the fact.
    //

    if ((NT_SUCCESS(st)) &&
        (MiFirstDriverLoadEver == 0) &&
        (MiVerifierThunksAdded != ThunksAdded)) {

        MiReApplyVerifierToLoadedModules (ModuleListHead);
    }

    return st;
}

NTKERNELAPI
PVOID
MmGetSystemRoutineAddress (
    __in PUNICODE_STRING SystemRoutineName
    )

/*++

Routine Description:

    This function returns the address of the argument function pointer if
    it is in the kernel or HAL, NULL if it is not.

Arguments:

    SystemRoutineName - Supplies the name of the desired routine.

Return Value:

    Non-NULL function pointer if successful.  NULL if not.

Environment:

    Kernel mode, PASSIVE_LEVEL, arbitrary process context.

--*/

{
    PKTHREAD CurrentThread;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ANSI_STRING AnsiString;
    PLIST_ENTRY NextEntry;
    UNICODE_STRING KernelString;
    UNICODE_STRING HalString;
    PVOID FunctionAddress;
    LOGICAL Found;
    ULONG EntriesChecked;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    EntriesChecked = 0;
    FunctionAddress = NULL;

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

    do {
        Status = RtlUnicodeStringToAnsiString (&AnsiString,
                                               SystemRoutineName,
                                               TRUE);

        if (NT_SUCCESS (Status)) {
            break;
        }

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

    } while (TRUE);

    //
    // Arbitrary process context so prevent suspend APCs now.
    //

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

    //
    // Check only the kernel and the HAL for exports.
    //

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        Found = FALSE;

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (RtlEqualUnicodeString (&KernelString,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

            Found = TRUE;
            EntriesChecked += 1;

        }
        else if (RtlEqualUnicodeString (&HalString,
                                        &DataTableEntry->BaseDllName,
                                        TRUE)) {

            Found = TRUE;
            EntriesChecked += 1;
        }

        if (Found == TRUE) {

            FunctionAddress = MiFindExportedRoutineByName (DataTableEntry->DllBase,
                                                           &AnsiString);

            if (FunctionAddress != NULL) {
                break;
            }

            if (EntriesChecked == 2) {
                break;
            }
        }

        NextEntry = NextEntry->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);

    RtlFreeAnsiString (&AnsiString);

    return FunctionAddress;
}

PVOID
MiFindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    )

/*++

Routine Description:

    This function searches the argument module looking for the requested
    exported function name.

Arguments:

    DllBase - Supplies the base address of the requested module.

    AnsiImageRoutineName - Supplies the ANSI routine name being searched for.

Return Value:

    The virtual address of the requested routine or NULL if not found.

--*/

{
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    LONG High;
    LONG Low;
    LONG Middle;
    LONG Result;
    ULONG ExportSize;
    PVOID FunctionAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    PAGED_CODE();

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                DllBase,
                                TRUE,
                                IMAGE_DIRECTORY_ENTRY_EXPORT,
                                &ExportSize);

    if (ExportDirectory == NULL) {
        return NULL;
    }

    //
    // Initialize the pointer to the array of RVA-based ansi export strings.
    //

    NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);

    //
    // Initialize the pointer to the array of USHORT ordinal numbers.
    //

    NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

    //
    // Lookup the desired name in the name table using a binary search.
    //

    Low = 0;
    Middle = 0;
    High = ExportDirectory->NumberOfNames - 1;

    while (High >= Low) {

        //
        // Compute the next probe index and compare the import name
        // with the export name entry.
        //

        Middle = (Low + High) >> 1;

        Result = strcmp (AnsiImageRoutineName->Buffer,
                         (PCHAR)DllBase + NameTableBase[Middle]);

        if (Result < 0) {
            High = Middle - 1;
        }
        else if (Result > 0) {
            Low = Middle + 1;
        }
        else {
            break;
        }
    }

    //
    // If the high index is less than the low index, then a matching
    // table entry was not found. Otherwise, get the ordinal number
    // from the ordinal table.
    //

    if (High < Low) {
        return NULL;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];

    //
    // If the OrdinalNumber is not within the Export Address Table,
    // then this image does not implement the function.  Return not found.
    //

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        return NULL;
    }

    //
    // Index into the array of RVA export addresses by ordinal number.
    //

    Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

    FunctionAddress = (PVOID)((PCHAR)DllBase + Addr[OrdinalNumber]);

    //
    // Forwarders are not used by the kernel and HAL to each other.
    //

    ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
            (FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));

    return FunctionAddress;
}

