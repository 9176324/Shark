/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    imagedir.c

Abstract:

    The module contains the code to translate an image directory type to
    the address of the data for that entry.

--*/

#include "ntrtlp.h"

VOID
RtlpTouchMemory(
    IN PVOID Address,
    IN ULONG Length
    );

VOID
RtlpMakeStackTraceDataPresentForImage(
    IN PVOID ImageBase
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,RtlpTouchMemory)
#pragma alloc_text(PAGE,RtlMakeStackTraceDataPresent)
#pragma alloc_text(PAGE,RtlpMakeStackTraceDataPresentForImage)
#endif

PIMAGE_SECTION_HEADER
RtlSectionTableFromVirtualAddress (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )

/*++

Routine Description:

    This function locates a VirtualAddress within the image header
    of a file that is mapped as a file and returns a pointer to the
    section table entry for that virtual address

Arguments:

    NtHeaders - Supplies the pointer to the image or data file.

    Base - Supplies the base of the image or data file.

    Address - Supplies the virtual address to locate.

Return Value:

    NULL - The file does not contain data for the specified directory entry.

    NON-NULL - Returns the pointer of the section entry containing the data.

--*/

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if ((ULONG)Address >= NtSection->VirtualAddress &&
            (ULONG)Address < NtSection->VirtualAddress + NtSection->SizeOfRawData
           ) {
            return NtSection;
            }
        ++NtSection;
        }

    return NULL;
}


PVOID
RtlAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )

/*++

Routine Description:

    This function locates a VirtualAddress within the image header
    of a file that is mapped as a file and returns the seek address
    of the data the Directory describes.

Arguments:

    NtHeaders - Supplies the pointer to the image or data file.

    Base - Supplies the base of the image or data file.

    Address - Supplies the virtual address to locate.

Return Value:

    NULL - The file does not contain data for the specified directory entry.

    NON-NULL - Returns the address of the raw data the directory describes.

--*/

{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = RtlSectionTableFromVirtualAddress( NtHeaders,
                                                   Base,
                                                   Address
                                                 );
    if (NtSection != NULL) {
        return( ((PCHAR)Base + ((ULONG_PTR)Address - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
    else {
        return( NULL );
        }
}


PVOID
RtlpImageDirectoryEntryToData32 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    PIMAGE_NT_HEADERS32 NtHeaders
    )
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return( NULL );
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        return( NULL );
    }

    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return( NULL );
        }
    }

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( RtlAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
RtlpImageDirectoryEntryToData64 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    PIMAGE_NT_HEADERS64 NtHeaders
    )
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return( NULL );
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        return( NULL );
    }

    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return( NULL );
        }
    }

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( RtlAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
RtlImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )

/*++

Routine Description:

    This function locates a Directory Entry within the image header
    and returns either the virtual address or seek address of the
    data the Directory describes.

Arguments:

    Base - Supplies the base of the image or data file.

    MappedAsImage - FALSE if the file is mapped as a data file.
                  - TRUE if the file is mapped as an image.

    DirectoryEntry - Supplies the directory entry to locate.

    Size - Return the size of the directory.

Return Value:

    NULL - The file does not contain data for the specified directory entry.

    NON-NULL - Returns the address of the raw data the directory describes.

--*/

{
    PIMAGE_NT_HEADERS NtHeaders;

    if (LDR_IS_DATAFILE(Base)) {
        Base = LDR_DATAFILE_TO_VIEW(Base);
        MappedAsImage = FALSE;
        }

    NtHeaders = RtlImageNtHeader(Base);

    if (!NtHeaders)
        return NULL;

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return (RtlpImageDirectoryEntryToData32(Base,
                                                MappedAsImage,
                                                DirectoryEntry,
                                                Size,
                                                (PIMAGE_NT_HEADERS32)NtHeaders));
    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return (RtlpImageDirectoryEntryToData64(Base,
                                                MappedAsImage,
                                                DirectoryEntry,
                                                Size,
                                                (PIMAGE_NT_HEADERS64)NtHeaders));
    } else {
        return (NULL);
    }
}


VOID
RtlMakeStackTraceDataPresent(
    VOID
    )

/*++

Routine Description:

    This function walks the loaded user-mode images and makes present the
    portions of the data necessary to support a kernel-debugger stack
    dump of the user-mode stack.

    N.B. The necessary pieces are merely touched to be brought in.  In a low
         memory situation, some of the previously-touched pages may be paged
         out in order for the new ones to be brought in.  This routine is
         not guaranteed to do anything useful, however it does more often than
         not.

Arguments:

    None.

Return value:

    None.

--*/

{
    PPEB peb;
    PLIST_ENTRY head;
    PLIST_ENTRY next;
    PVOID imageBase;
    ULONG imageCount;
    LDR_DATA_TABLE_ENTRY UNALIGNED *ldrDataTableEntry;

    RTL_PAGED_CODE();

    //
    // The image list is in user mode and is not to be trusted.  The
    // surrounding try/except block will guard against most forms of
    // list corruption.  imageCount is used to bail in finite time
    // in the event of a cyclic image list.
    //

    imageCount = 0;
    try {

        peb = NtCurrentPeb();
        head = &peb->Ldr->InLoadOrderModuleList;

        ProbeForReadSmallStructure( head,
                                    sizeof(LIST_ENTRY),
                                    PROBE_ALIGNMENT(LIST_ENTRY) );

        next = head;
        while (imageCount < 1000) {

            next = next->Flink;
            if (next == head) {
                break;
            }
            imageCount += 1;

            //
            // Locate the base address of the image
            //

            ldrDataTableEntry = CONTAINING_RECORD(next,
                                                  LDR_DATA_TABLE_ENTRY,
                                                  InLoadOrderLinks);

            ProbeForReadSmallStructure( ldrDataTableEntry,
                                        sizeof(LDR_DATA_TABLE_ENTRY),
                                        PROBE_ALIGNMENT(LDR_DATA_TABLE_ENTRY) );

            imageBase = ldrDataTableEntry->DllBase;
            ProbeForReadSmallStructure (imageBase, sizeof (IMAGE_DOS_HEADER), sizeof (UCHAR));

            //
            // Make the stack trace data present for this image.  Use a
            // separate try/except block here so that subsequent images
            // will be processed in the event of a failure.
            //

            try {
                RtlpMakeStackTraceDataPresentForImage(imageBase);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
}

VOID
RtlpMakeStackTraceDataPresentForImage(
    IN PVOID ImageBase
    )

/*++

Routine Description:

    This function attempts to make present the portions of an image necessary
    to support a kernel-debugger stack dump of the user-mode stack.

Arguments:

    ImageBase - Supplies the VA of the base of the image to process.

Return value:

    None.

--*/

{
    PVOID directory;
    ULONG directorySize;
    PIMAGE_RUNTIME_FUNCTION_ENTRY functionEntry;
    PIMAGE_RUNTIME_FUNCTION_ENTRY lastFunctionEntry;
    PCHAR imageBase;

    RTL_PAGED_CODE();

    //
    // Make present the IMAGE_DIRECTORY_EXCEPTION section.
    //

    directory = RtlImageDirectoryEntryToData(ImageBase,
                                             TRUE,
                                             IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                             &directorySize);
    if (directory == NULL) {
        return;
    }

    RtlpTouchMemory(directory, directorySize);
}

VOID
RtlpTouchMemory(
    IN PVOID Address,
    IN ULONG Length
    )
/*++

Routine Description:

    This function touches all of the pages within a given region.

Arguments:

    Address - Supplies the VA of the start of the image to make present.

    Length - Supplies the length, in bytes, of the image to make present.

Return value:

    None.

--*/
{
    PCHAR regionStart;
    PCHAR regionEnd;

    RTL_PAGED_CODE();

    regionStart = Address;
    regionEnd = regionStart + Length;

    while (regionStart < regionEnd) {
        *(volatile UCHAR *)regionStart;
        regionStart = PAGE_ALIGN(regionStart + PAGE_SIZE);
    }
}

