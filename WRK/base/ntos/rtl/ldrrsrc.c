/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ldrrsrc.c

Abstract: 

    Loader API calls for accessing resource sections.

--*/

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,LdrAccessResource)
#pragma alloc_text(PAGE,LdrpAccessResourceData)
#pragma alloc_text(PAGE,LdrpAccessResourceDataNoMultipleLanguage)
#pragma alloc_text(PAGE,LdrFindEntryForAddress)
#pragma alloc_text(PAGE,LdrFindResource_U)
#pragma alloc_text(PAGE,LdrFindResourceEx_U)
#pragma alloc_text(PAGE,LdrFindResourceDirectory_U)
#pragma alloc_text(PAGE,LdrpCompareResourceNames_U)
#pragma alloc_text(PAGE,LdrpSearchResourceSection_U)
#pragma alloc_text(PAGE,LdrEnumResources)
#endif

#define USE_RC_CHECKSUM

// winuser.h
#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#define RT_VERSION                         16
#define RT_MANIFEST                        24
#define CREATEPROCESS_MANIFEST_RESOURCE_ID  1
#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID 2
#define MINIMUM_RESERVED_MANIFEST_RESOURCE_ID 1
#define MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID 16

#define LDRP_MIN(x,y) (((x)<(y)) ? (x) : (y))

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    || (x) == STATUS_RESOURCE_DATA_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_TYPE_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_NAME_NOT_FOUND  \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_WARNING_LEVEL)

NTSTATUS
LdrAccessResource(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

/*++

Routine Description:

    This function locates the address of the specified resource in the
    specified DLL and returns its address.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceDataEntry - Supplies a pointer to the resource data entry in
        the resource data section of the image file specified by the
        DllHandle parameter.  This pointer should have been one returned
        by the LdrFindResource function.

    Address - Optional pointer to a variable that will receive the
        address of the resource specified by the first two parameters.

    Size - Optional pointer to a variable that will receive the size of
        the resource specified by the first two parameters.

--*/

{

    NTSTATUS Status;
    RTL_PAGED_CODE();

    Status =
        LdrpAccessResourceData(
          DllHandle,
          ResourceDataEntry,
          Address,
          Size
          );

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
#endif
    return Status;
}

NTSTATUS
LdrpAccessResourceDataNoMultipleLanguage(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

/*++

Routine Description:

    This function returns the data necessary to actually examine the
    contents of a particular resource, without allowing for the .mui
    feature. It used to be the tail of LdrpAccessResourceData, from
    which it is now called.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceDataEntry - Supplies a pointer to the resource data entry in
        the resource data directory of the image file specified by the
        DllHandle parameter.  This pointer should have been one returned
        by the LdrFindResource function.

    Address - Optional pointer to a variable that will receive the
        address of the resource specified by the first two parameters.

    Size - Optional pointer to a variable that will receive the size of
        the resource specified by the first two parameters.

--*/

{
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    ULONG ResourceSize;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR VirtualAddressOffset;
    PIMAGE_SECTION_HEADER NtSection;
    NTSTATUS Status = STATUS_SUCCESS;

    RTL_PAGED_CODE();

    try {
        ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData(DllHandle,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                         &ResourceSize
                                         );
        if (!ResourceDirectory) {
            return STATUS_RESOURCE_DATA_NOT_FOUND;
        }

        if (LDR_IS_DATAFILE(DllHandle)) {
            ULONG ResourceRVA;
            DllHandle = LDR_DATAFILE_TO_VIEW(DllHandle);
            NtHeaders = RtlImageNtHeader( DllHandle );
            if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                ResourceRVA=((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress;
            } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                ResourceRVA=((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress;
            } else {
                ResourceRVA = 0;
            }

            if (!ResourceRVA) {
                return STATUS_RESOURCE_DATA_NOT_FOUND;
                }

            VirtualAddressOffset = (ULONG_PTR)DllHandle + ResourceRVA - (ULONG_PTR)ResourceDirectory;

            //
            // Now, we must check to see if the resource is not in the
            // same section as the resource table.  If it's in .rsrc1,
            // we've got to adjust the RVA in the ResourceDataEntry
            // to point to the correct place in the non-VA data file.
            //
            NtSection = RtlSectionTableFromVirtualAddress( NtHeaders, DllHandle, ResourceRVA);

            if (!NtSection) {
                return STATUS_RESOURCE_DATA_NOT_FOUND;
            }

            if ( ResourceDataEntry->OffsetToData > NtSection->Misc.VirtualSize ) {
                ULONG rva;

                rva = NtSection->VirtualAddress;
                NtSection = RtlSectionTableFromVirtualAddress(NtHeaders,
                                                             DllHandle,
                                                             ResourceDataEntry->OffsetToData
                                                             );
                if (!NtSection) {
                    return STATUS_RESOURCE_DATA_NOT_FOUND;
                }
                VirtualAddressOffset +=
                        ((ULONG_PTR)NtSection->VirtualAddress - rva) -
                        ((ULONG_PTR)RtlAddressInSectionTable ( NtHeaders, DllHandle, NtSection->VirtualAddress ) - (ULONG_PTR)ResourceDirectory);
            }
        } else {
            VirtualAddressOffset = 0;
        }

        if (ARGUMENT_PRESENT( Address )) {
            *Address = (PVOID)( (PCHAR)DllHandle +
                                (ResourceDataEntry->OffsetToData - VirtualAddressOffset)
                              );
        }

        if (ARGUMENT_PRESENT( Size )) {
            *Size = ResourceDataEntry->Size;
        }

    }    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
#endif
    return Status;
}


NTSTATUS
LdrpAccessResourceData(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

/*++

Routine Description:

    This function returns the data necessary to actually examine the
    contents of a particular resource.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceDataEntry - Supplies a pointer to the resource data entry in
   the resource data directory of the image file specified by the
        DllHandle parameter.  This pointer should have been one returned
        by the LdrFindResource function.

    Address - Optional pointer to a variable that will receive the
        address of the resource specified by the first two parameters.

    Size - Optional pointer to a variable that will receive the size of
        the resource specified by the first two parameters.

--*/

{
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    ULONG ResourceSize;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS Status = STATUS_SUCCESS;

    RTL_PAGED_CODE();

    Status =
        LdrpAccessResourceDataNoMultipleLanguage(
            DllHandle,
            ResourceDataEntry,
            Address,
            Size
            );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
    return Status;
}


NTSTATUS
LdrFindEntryForAddress(
    IN PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *TableEntry
    )
/*++

Routine Description:

    This function returns the load data table entry that describes the virtual
    address range that contains the passed virtual address.

Arguments:

    Address - Supplies a 32-bit virtual address.

    TableEntry - Supplies a pointer to the variable that will receive the
        address of the loader data table entry.


Return Value:

    Status

--*/
{
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID ImageBase;
    PVOID EndOfImage;
    NTSTATUS Status;

    Ldr = NtCurrentPeb()->Ldr;
    if (Ldr == NULL) {
        Status = STATUS_NO_MORE_ENTRIES;
        goto Exit;
        }

    Entry = (PLDR_DATA_TABLE_ENTRY) Ldr->EntryInProgress;
    if (Entry != NULL) {
        NtHeaders = RtlImageNtHeader( Entry->DllBase );
        if (NtHeaders != NULL) {
            ImageBase = (PVOID)Entry->DllBase;

            EndOfImage = (PVOID)
                ((ULONG_PTR)ImageBase + NtHeaders->OptionalHeader.SizeOfImage);

            if ((ULONG_PTR)Address >= (ULONG_PTR)ImageBase && (ULONG_PTR)Address < (ULONG_PTR)EndOfImage) {
                *TableEntry = Entry;
                Status = STATUS_SUCCESS;
                goto Exit;
                }
            }
        }

    Head = &Ldr->InMemoryOrderModuleList;
    Next = Head->Flink;
    while ( Next != Head ) {
        Entry = CONTAINING_RECORD( Next, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );

        NtHeaders = RtlImageNtHeader( Entry->DllBase );
        if (NtHeaders != NULL) {
            ImageBase = (PVOID)Entry->DllBase;

            EndOfImage = (PVOID)
                ((ULONG_PTR)ImageBase + NtHeaders->OptionalHeader.SizeOfImage);

            if ((ULONG_PTR)Address >= (ULONG_PTR)ImageBase && (ULONG_PTR)Address < (ULONG_PTR)EndOfImage) {
                *TableEntry = Entry;
                Status = STATUS_SUCCESS;
                goto Exit;
                }
            }

        Next = Next->Flink;
        }

    Status = STATUS_NO_MORE_ENTRIES;
Exit:
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
    return( Status );
}


NTSTATUS
LdrFindResource_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    )

/*++

Routine Description:

    This function locates the address of the specified resource in the
    specified DLL and returns its address.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceIdPath - Supplies a pointer to an array of 32-bit resource
        identifiers.  Each identifier is either an integer or a pointer
        to a STRING structure that specifies a resource name.  The array
        is used to traverse the directory structure contained in the
        resource section in the image file specified by the DllHandle
        parameter.

    ResourceIdPathLength - Supplies the number of elements in the
        ResourceIdPath array.

    ResourceDataEntry - Supplies a pointer to a variable that will
        receive the address of the resource data entry in the resource
        data section of the image file specified by the DllHandle
        parameter.
--*/

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      0,                // Look for a leaf node, ineaxt lang match
      (PVOID *)ResourceDataEntry
      );
}

NTSTATUS
LdrFindResourceEx_U(
    IN ULONG Flags,
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    )

/*++

Routine Description:

    This function locates the address of the specified resource in the
    specified DLL and returns its address.

Arguments:
    Flags -
        LDRP_FIND_RESOURCE_DIRECTORY
        searching for a resource directory, otherwise the caller is
        searching for a resource data entry.

        LDR_FIND_RESOURCE_LANGUAGE_EXACT
        searching for a resource with, and only with, the language id
        specified in ResourceIdPath, otherwise the caller wants the routine
        to come up with default when specified langid is not found.

        LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION
        searching for a resource version in both main and alternative
        module paths

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceIdPath - Supplies a pointer to an array of 32-bit resource
        identifiers.  Each identifier is either an integer or a pointer
        to a STRING structure that specifies a resource name.  The array
        is used to traverse the directory structure contained in the
        resource section in the image file specified by the DllHandle
        parameter.

    ResourceIdPathLength - Supplies the number of elements in the
        ResourceIdPath array.

    ResourceDataEntry - Supplies a pointer to a variable that will
        receive the address of the resource data entry in the resource
        data section of the image file specified by the DllHandle
        parameter.
--*/

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      Flags,
      (PVOID *)ResourceDataEntry
      );
}



NTSTATUS
LdrFindResourceDirectory_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DIRECTORY *ResourceDirectory
    )

/*++

Routine Description:

    This function locates the address of the specified resource directory in
    specified DLL and returns its address.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource
        directory is contained in.

    ResourceIdPath - Supplies a pointer to an array of 32-bit resource
        identifiers.  Each identifier is either an integer or a pointer
        to a STRING structure that specifies a resource name.  The array
        is used to traverse the directory structure contained in the
        resource section in the image file specified by the DllHandle
        parameter.

    ResourceIdPathLength - Supplies the number of elements in the
        ResourceIdPath array.

    ResourceDirectory - Supplies a pointer to a variable that will
        receive the address of the resource directory specified by
        ResourceIdPath in the resource data section of the image file
        the DllHandle parameter.
--*/

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      LDRP_FIND_RESOURCE_DIRECTORY,                 // Look for a directory node
      (PVOID *)ResourceDirectory
      );
}


LONG
LdrpCompareResourceNames_U(
    IN ULONG_PTR ResourceName,
    IN const IMAGE_RESOURCE_DIRECTORY* ResourceDirectory,
    IN const IMAGE_RESOURCE_DIRECTORY_ENTRY* ResourceDirectoryEntry
    )
{
    LONG li;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;

    if (ResourceName & LDR_RESOURCE_ID_NAME_MASK) {
        if (!ResourceDirectoryEntry->NameIsString) {
            return( -1 );
            }

        ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
            ((PCHAR)ResourceDirectory + ResourceDirectoryEntry->NameOffset);

        li = wcsncmp( (LPWSTR)ResourceName,
            ResourceNameString->NameString,
            ResourceNameString->Length
          );

        if (!li && wcslen((PWSTR)ResourceName) != ResourceNameString->Length) {
       return( 1 );
       }

   return(li);
        }
    else {
        if (ResourceDirectoryEntry->NameIsString) {
            return( 1 );
            }

        return( (ULONG)(ResourceName - ResourceDirectoryEntry->Name) );
        }
}

// Language ids are 16bits so any value with any bits
// set above 16 should be ok, and this value only has
// to fit in a ULONG_PTR. 0x10000 should be sufficient.
// The value used is actually 0xFFFF regardless of 32bit or 64bit,
// I guess assuming this is not an actual langid, which it isn't,
// due to the relatively small number of languages, around 70.
#define  USE_FIRSTAVAILABLE_LANGID   (0xFFFFFFFF & ~LDR_RESOURCE_ID_NAME_MASK)

NTSTATUS
LdrpSearchResourceSection_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN ULONG Flags,
    OUT PVOID *ResourceDirectoryOrData
    )

/*++

Routine Description:

    This function locates the address of the specified resource in the
    specified DLL and returns its address.

Arguments:

    DllHandle - Supplies a handle to the image file that the resource is
        contained in.

    ResourceIdPath - Supplies a pointer to an array of 32-bit resource
        identifiers.  Each identifier is either an integer or a pointer
        to a null terminated string (PSZ) that specifies a resource
        name.  The array is used to traverse the directory structure
        contained in the resource section in the image file specified by
        the DllHandle parameter.

    ResourceIdPathLength - Supplies the number of elements in the
        ResourceIdPath array.

    Flags -
        LDRP_FIND_RESOURCE_DIRECTORY
        searching for a resource directory, otherwise the caller is
        searching for a resource data entry.

        LDR_FIND_RESOURCE_LANGUAGE_EXACT
        searching for a resource with, and only with, the language id
        specified in ResourceIdPath, otherwise the caller wants the routine
        to come up with default when specified langid is not found.

        LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION
        searching for a resource version in main and alternative
        modules paths

    FindDirectoryEntry - Supplies a boolean that is TRUE if caller is
        searching for a resource directory, otherwise the caller is
        searching for a resource data entry.

    ExactLangMatchOnly - Supplies a boolean that is TRUE if caller is
        searching for a resource with, and only with, the language id
        specified in ResourceIdPath, otherwise the caller wants the routine
        to come up with default when specified langid is not found.

    ResourceDirectoryOrData - Supplies a pointer to a variable that will
        receive the address of the resource directory or data entry in
        the resource data section of the image file specified by the
        DllHandle parameter.
--*/

{
    NTSTATUS Status;
    PIMAGE_RESOURCE_DIRECTORY LanguageResourceDirectory, ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntLow;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntMiddle;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntHigh;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceEntry;
    USHORT n, half;
    LONG dir;
    ULONG size;
    ULONG_PTR ResourceIdRetry;
    ULONG RetryCount;
    LANGID NewLangId;
    const ULONG_PTR* IdPath = ResourceIdPath;
    ULONG IdPathLength = ResourceIdPathLength;
    BOOLEAN fIsNeutral = FALSE;
    LANGID GivenLanguage;

    RTL_PAGED_CODE();

    try {
        TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData(DllHandle,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                         &size
                                         );
        if (!TopResourceDirectory) {
            return( STATUS_RESOURCE_DATA_NOT_FOUND );
        }

        ResourceDirectory = TopResourceDirectory;
        ResourceIdRetry = USE_FIRSTAVAILABLE_LANGID;
        RetryCount = 0;
        ResourceEntry = NULL;
        LanguageResourceDirectory = NULL;
        while (ResourceDirectory != NULL && ResourceIdPathLength--) {
            //
            // If search path includes a language id, then attempt to
            // match the following language ids in this order:
            //
            //   (0)  use given language id
            //   (1)  use primary language of given language id
            //   (2)  use id 0  (neutral resource)
            //   (4)  use user UI language
            //
            // If the PRIMARY language id is ZERO, then ALSO attempt to
            // match the following language ids in this order:
            //
            //   (3)  use thread language id for console app
            //   (4)  use user UI language
            //   (5)  use lang id of TEB for windows app if it is different from user locale
            //   (6)  use UI lang from exe resource
            //   (7)  use primary UI lang from exe resource
            //   (8)  use Install Language
            //   (9)  use lang id from user's locale id
            //   (10)  use primary language of user's locale id
            //   (11) use lang id from system default locale id
            //   (12) use lang id of system default locale id
            //   (13) use primary language of system default locale id
            //   (14) use US English lang id
            //   (15) use any lang id that matches requested info
            //
            if (ResourceIdPathLength == 0 && IdPathLength == 3) {
                LanguageResourceDirectory = ResourceDirectory;
                }

            if (LanguageResourceDirectory != NULL) {
                GivenLanguage = (LANGID)IdPath[ 2 ];
                fIsNeutral = (PRIMARYLANGID( GivenLanguage ) == LANG_NEUTRAL);
TryNextLangId:
                switch( RetryCount++ ) {
                    case 0:     // Use given language id
                        NewLangId = GivenLanguage;
                        break;

                    case 1:     // Use primary language of given language id
                        NewLangId = PRIMARYLANGID( GivenLanguage );
                        break;

                    case 2:     // Use id 0  (neutral resource)
                        NewLangId = 0;
                        break;

                    case 3:     // Use user's default UI language
                        NewLangId = (LANGID)ResourceIdRetry;
                        break;

                    case 4:     // Use native UI language
                        if ( !fIsNeutral ) {
                            // Stop looking - Not in the neutral case
                            goto ReturnFailure;
                            break;
                        }
                        NewLangId = PsInstallUILanguageId;
                        break;

                    case 5:     // Use default system locale
                        NewLangId = LANGIDFROMLCID(PsDefaultSystemLocaleId);
                        break;

                    case 6:
                        // Use US English language
                        NewLangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
                        break;

                    case 7:     // Take any lang id that matches
                        NewLangId = USE_FIRSTAVAILABLE_LANGID;
                        break;

                    default:    // No lang ids to match
                        goto ReturnFailure;
                        break;
                }

                //
                // If looking for a specific language id and same as the
                // one we just looked up, then skip it.
                //
                if (NewLangId != USE_FIRSTAVAILABLE_LANGID &&
                    NewLangId == ResourceIdRetry
                   ) {
                    goto TryNextLangId;
                    }

                //
                // Try this new language Id
                //
                ResourceIdRetry = (ULONG_PTR)NewLangId;
                ResourceIdPath = &ResourceIdRetry;
                ResourceDirectory = LanguageResourceDirectory;
                }

            n = ResourceDirectory->NumberOfNamedEntries;
            ResourceDirEntLow = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
            if (!(*ResourceIdPath & LDR_RESOURCE_ID_NAME_MASK)) { // No string(name),so we need ID
                ResourceDirEntLow += n;
                n = ResourceDirectory->NumberOfIdEntries;
                }

            if (!n) {
                ResourceDirectory = NULL;
                goto NotFound;  // Resource directory contains zero types or names or langID.
                }

            if (LanguageResourceDirectory != NULL &&
                *ResourceIdPath == USE_FIRSTAVAILABLE_LANGID
               ) {
                ResourceDirectory = NULL;
                ResourceIdRetry = ResourceDirEntLow->Name;
                ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                    ((PCHAR)TopResourceDirectory +
                            ResourceDirEntLow->OffsetToData
                    );

                break;
                }

            ResourceDirectory = NULL;
            ResourceDirEntHigh = ResourceDirEntLow + n - 1;
            while (ResourceDirEntLow <= ResourceDirEntHigh) {
                if ((half = (n >> 1)) != 0) {
                    ResourceDirEntMiddle = ResourceDirEntLow;
                    if (*(PUCHAR)&n & 1) {
                        ResourceDirEntMiddle += half;
                        }
                    else {
                        ResourceDirEntMiddle += half - 1;
                        }
                    dir = LdrpCompareResourceNames_U( *ResourceIdPath,
                                                      TopResourceDirectory,
                                                      ResourceDirEntMiddle
                                                    );
                    if (!dir) {
                        if (ResourceDirEntMiddle->DataIsDirectory) {
                            ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                    ((PCHAR)TopResourceDirectory +
                                    ResourceDirEntMiddle->OffsetToDirectory
                                );
                            }
                        else {
                            ResourceDirectory = NULL;
                            ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                ((PCHAR)TopResourceDirectory +
                                 ResourceDirEntMiddle->OffsetToData
                                );
                            }

                        break;
                        }
                    else {
                        if (dir < 0) {  // Order in the resource: Name, ID.
                            ResourceDirEntHigh = ResourceDirEntMiddle - 1;
                            if (*(PUCHAR)&n & 1) {
                                n = half;
                                }
                            else {
                                n = half - 1;
                                }
                            }
                        else {
                            ResourceDirEntLow = ResourceDirEntMiddle + 1;
                            n = half;
                            }
                        }
                    }
                else {
                    if (n != 0) {
                        dir = LdrpCompareResourceNames_U( *ResourceIdPath,
                          TopResourceDirectory,
                                                          ResourceDirEntLow
                                                        );
                        if (!dir) {   // find, or it fail to set ResourceDirectory so go to NotFound.
                            if (ResourceDirEntLow->DataIsDirectory) {
                                ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                                    ((PCHAR)TopResourceDirectory +
                                        ResourceDirEntLow->OffsetToDirectory
                                    );
                                }
                            else {
                                ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                    ((PCHAR)TopResourceDirectory +
                      ResourceDirEntLow->OffsetToData
                                    );
                                }
                            }
                        }

                    break;
                    }
                }

            ResourceIdPath++;
            }

        if (ResourceEntry != NULL && !(Flags & LDRP_FIND_RESOURCE_DIRECTORY)) {
            *ResourceDirectoryOrData = (PVOID)ResourceEntry;
            Status = STATUS_SUCCESS;
            }
        else
        if (ResourceDirectory != NULL && (Flags & LDRP_FIND_RESOURCE_DIRECTORY)) {
            *ResourceDirectoryOrData = (PVOID)ResourceDirectory;
            Status = STATUS_SUCCESS;
            }
        else {
NotFound:
            switch( IdPathLength - ResourceIdPathLength) {
                case 3:     Status = STATUS_RESOURCE_LANG_NOT_FOUND; break;
                case 2:     Status = STATUS_RESOURCE_NAME_NOT_FOUND; break;
                case 1:     Status = STATUS_RESOURCE_TYPE_NOT_FOUND; break;
                default:    Status = STATUS_INVALID_PARAMETER; break;
                }
            }

        if (Status == STATUS_RESOURCE_LANG_NOT_FOUND &&
            LanguageResourceDirectory != NULL
           ) {
            ResourceEntry = NULL;
            goto TryNextLangId;
ReturnFailure: ;
            Status = STATUS_RESOURCE_LANG_NOT_FOUND;
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}


NTSTATUS
LdrEnumResources(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN OUT PULONG NumberOfResources,
    OUT PLDR_ENUM_RESOURCE_ENTRY Resources OPTIONAL
    )
{
    NTSTATUS Status;
    PIMAGE_RESOURCE_DIRECTORY TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY TypeResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY NameResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY LangResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY TypeResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY NameResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY LangResourceDirectoryEntry;
    ULONG TypeDirectoryIndex, NumberOfTypeDirectoryEntries;
    ULONG NameDirectoryIndex, NumberOfNameDirectoryEntries;
    ULONG LangDirectoryIndex, NumberOfLangDirectoryEntries;
    BOOLEAN ScanTypeDirectory;
    BOOLEAN ScanNameDirectory;
    BOOLEAN ReturnThisResource;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    ULONG_PTR TypeResourceNameOrId;
    ULONG_PTR NameResourceNameOrId;
    ULONG_PTR LangResourceNameOrId;
    PLDR_ENUM_RESOURCE_ENTRY ResourceInfo;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    ULONG ResourceIndex, MaxResourceIndex;
    ULONG Size;

    ResourceIndex = 0;
    if (!ARGUMENT_PRESENT( Resources )) {
        MaxResourceIndex = 0;
        }
    else {
        MaxResourceIndex = *NumberOfResources;
        }
    *NumberOfResources = 0;

    TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData( DllHandle,
                                      TRUE,
                                      IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                      &Size
                                    );
    if (!TopResourceDirectory) {
        return STATUS_RESOURCE_DATA_NOT_FOUND;
        }

    TypeResourceDirectory = TopResourceDirectory;
    TypeResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(TypeResourceDirectory+1);
    NumberOfTypeDirectoryEntries = TypeResourceDirectory->NumberOfNamedEntries +
                                   TypeResourceDirectory->NumberOfIdEntries;
    TypeDirectoryIndex = 0;
    Status = STATUS_SUCCESS;
    for (TypeDirectoryIndex=0;
         TypeDirectoryIndex<NumberOfTypeDirectoryEntries;
         TypeDirectoryIndex++, TypeResourceDirectoryEntry++
        ) {
        if (ResourceIdPathLength > 0) {
            ScanTypeDirectory = LdrpCompareResourceNames_U( ResourceIdPath[ 0 ],
                                                            TopResourceDirectory,
                                                            TypeResourceDirectoryEntry
                                                          ) == 0;
            }
        else {
            ScanTypeDirectory = TRUE;
            }
        if (ScanTypeDirectory) {
            if (!TypeResourceDirectoryEntry->DataIsDirectory) {
                return STATUS_INVALID_IMAGE_FORMAT;
                }
            if (TypeResourceDirectoryEntry->NameIsString) {
                ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                    ((PCHAR)TopResourceDirectory + TypeResourceDirectoryEntry->NameOffset);

                TypeResourceNameOrId = (ULONG_PTR)ResourceNameString;
                }
            else {
                TypeResourceNameOrId = (ULONG_PTR)TypeResourceDirectoryEntry->Id;
                }

            NameResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                ((PCHAR)TopResourceDirectory + TypeResourceDirectoryEntry->OffsetToDirectory);
            NameResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(NameResourceDirectory+1);
            NumberOfNameDirectoryEntries = NameResourceDirectory->NumberOfNamedEntries +
                                           NameResourceDirectory->NumberOfIdEntries;
            for (NameDirectoryIndex=0;
                 NameDirectoryIndex<NumberOfNameDirectoryEntries;
                 NameDirectoryIndex++, NameResourceDirectoryEntry++
                ) {
                if (ResourceIdPathLength > 1) {
                    ScanNameDirectory = LdrpCompareResourceNames_U( ResourceIdPath[ 1 ],
                                                                    TopResourceDirectory,
                                                                    NameResourceDirectoryEntry
                                                                  ) == 0;
                    }
                else {
                    ScanNameDirectory = TRUE;
                    }
                if (ScanNameDirectory) {
                    if (!NameResourceDirectoryEntry->DataIsDirectory) {
                        return STATUS_INVALID_IMAGE_FORMAT;
                        }

                    if (NameResourceDirectoryEntry->NameIsString) {
                        ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                            ((PCHAR)TopResourceDirectory + NameResourceDirectoryEntry->NameOffset);

                        NameResourceNameOrId = (ULONG_PTR)ResourceNameString;
                        }
                    else {
                        NameResourceNameOrId = (ULONG_PTR)NameResourceDirectoryEntry->Id;
                        }

                    LangResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                        ((PCHAR)TopResourceDirectory + NameResourceDirectoryEntry->OffsetToDirectory);

                    LangResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(LangResourceDirectory+1);
                    NumberOfLangDirectoryEntries = LangResourceDirectory->NumberOfNamedEntries +
                                                   LangResourceDirectory->NumberOfIdEntries;
                    LangDirectoryIndex = 0;
                    for (LangDirectoryIndex=0;
                         LangDirectoryIndex<NumberOfLangDirectoryEntries;
                         LangDirectoryIndex++, LangResourceDirectoryEntry++
                        ) {
                        if (ResourceIdPathLength > 2) {
                            ReturnThisResource = LdrpCompareResourceNames_U( ResourceIdPath[ 2 ],
                                                                             TopResourceDirectory,
                                                                             LangResourceDirectoryEntry
                                                                           ) == 0;
                            }
                        else {
                            ReturnThisResource = TRUE;
                            }
                        if (ReturnThisResource) {
                            if (LangResourceDirectoryEntry->DataIsDirectory) {
                                return STATUS_INVALID_IMAGE_FORMAT;
                                }

                            if (LangResourceDirectoryEntry->NameIsString) {
                                ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                                    ((PCHAR)TopResourceDirectory + LangResourceDirectoryEntry->NameOffset);

                                LangResourceNameOrId = (ULONG_PTR)ResourceNameString;
                                }
                            else {
                                LangResourceNameOrId = (ULONG_PTR)LangResourceDirectoryEntry->Id;
                                }

                            ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                    ((PCHAR)TopResourceDirectory + LangResourceDirectoryEntry->OffsetToData);

                            ResourceInfo = &Resources[ ResourceIndex++ ];
                            if (ResourceIndex <= MaxResourceIndex) {
                                ResourceInfo->Path[ 0 ].NameOrId = TypeResourceNameOrId;
                                ResourceInfo->Path[ 1 ].NameOrId = NameResourceNameOrId;
                                ResourceInfo->Path[ 2 ].NameOrId = LangResourceNameOrId;
                                ResourceInfo->Data = (PVOID)((ULONG_PTR)DllHandle + ResourceDataEntry->OffsetToData);
                                ResourceInfo->Size = ResourceDataEntry->Size;
                                ResourceInfo->Reserved = 0;
                                }
                            else {
                                Status = STATUS_INFO_LENGTH_MISMATCH;
                                }
                            }
                        }
                    }
                }
            }
        }

    *NumberOfResources = ResourceIndex;
    return Status;
}

