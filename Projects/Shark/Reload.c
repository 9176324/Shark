/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>

#include "Reload.h"

#include "Except.h"
#include "Jump.h"
#include "Scan.h"

PRELOADER_PARAMETER_BLOCK ReloaderBlock;

ULONG
NTAPI
GetPlatform(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Platform = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        Platform = NtHeaders->OptionalHeader.Magic;
    }

    return Platform;
}

PVOID
NTAPI
GetAddressOfEntryPoint(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Offset = 0;
    PVOID EntryPoint = NULL;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (0 != Offset) {
            EntryPoint = (PCHAR)ImageBase + Offset;
        }
    }

    return EntryPoint;
}

ULONG
NTAPI
GetTimeStamp(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG TimeStamp = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        TimeStamp = NtHeaders->FileHeader.TimeDateStamp;
    }

    return TimeStamp;
}

USHORT
NTAPI
GetSubsystem(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    USHORT Subsystem = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Subsystem = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.Subsystem;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Subsystem = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.Subsystem;
        }
    }

    return Subsystem;
}

ULONG
NTAPI
GetSizeOfImage(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG SizeOfImage = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            SizeOfImage = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfImage;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            SizeOfImage = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfImage;
        }
    }

    return SizeOfImage;
}

PIMAGE_SECTION_HEADER
NTAPI
SectionTableFromVirtualAddress(
    __in PVOID ImageBase,
    __in PVOID Address
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Index = 0;
    ULONG Offset = 0;
    PIMAGE_SECTION_HEADER FountSection = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    ULONG SizeToLock = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FountSection = IMAGE_FIRST_SECTION(NtHeaders);
        Offset = (ULONG)((ULONG_PTR)Address - (ULONG_PTR)ImageBase);

        for (Index = 0;
            Index < NtHeaders->FileHeader.NumberOfSections;
            Index++) {
            SizeToLock = max(
                FountSection[Index].SizeOfRawData,
                FountSection[Index].Misc.VirtualSize);

            if (Offset >= FountSection[Index].VirtualAddress &&
                Offset < FountSection[Index].VirtualAddress + SizeToLock) {
                NtSection = &FountSection[Index];
                break;
            }
        }
    }

    return NtSection;
}

FORCEINLINE
ULONG
NTAPI
GetRelocCount(
    __in ULONG SizeOfBlock
)
{
    ULONG Count = 0;

    Count = (SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);

    return Count;
}

PIMAGE_BASE_RELOCATION
NTAPI
RelocationBlock(
    __in PVOID VA,
    __in ULONG Count,
    __in PUSHORT NextOffset,
    __in LONG_PTR Diff
)
{
    PUSHORT FixupVA = NULL;
    USHORT Offset = 0;
    USHORT Type = 0;

    while (Count--) {
        Offset = *NextOffset & 0xfff;
        FixupVA = (PCHAR)VA + Offset;
        Type = (*NextOffset >> 12) & 0xf;

        switch (Type) {
        case IMAGE_REL_BASED_ABSOLUTE: {
            break;
        }

        case IMAGE_REL_BASED_HIGH: {
            FixupVA[1] += (USHORT)((Diff >> 16) & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_LOW: {
            FixupVA[0] += (USHORT)(Diff & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_HIGHLOW: {
            *(PULONG)FixupVA += (ULONG)Diff;
            break;
        }

        case IMAGE_REL_BASED_HIGHADJ: {
            FixupVA[0] += NextOffset[1] & 0xffff;
            FixupVA[1] += (USHORT)((Diff >> 16) & 0xffff);

            ++NextOffset;
            --Count;
            break;
        }

        case IMAGE_REL_BASED_MIPS_JMPADDR:
        case IMAGE_REL_BASED_SECTION:
        case IMAGE_REL_BASED_REL32:
            // case IMAGE_REL_BASED_VXD_RELATIVE:
            // case IMAGE_REL_BASED_MIPS_JMPADDR16: 

        case IMAGE_REL_BASED_IA64_IMM64: {
            break;
        }

        case IMAGE_REL_BASED_DIR64: {
            *(PULONG_PTR)FixupVA += Diff;
            break;
        }

        default: {
            return NULL;
        }
        }

        ++NextOffset;
    }

    return (PIMAGE_BASE_RELOCATION)NextOffset;
}

VOID
NTAPI
RelocateImage(
    __in PVOID ImageBase,
    __in LONG_PTR Diff
)
{
    PIMAGE_BASE_RELOCATION RelocDirectory = NULL;
    ULONG Size = 0;
    PVOID VA = 0;

    RelocDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_BASERELOC,
        &Size);

    if (0 != Size) {
        if (0 != Diff) {
            while (0 != Size) {
                VA = (PCHAR)ImageBase + RelocDirectory->VirtualAddress;
                Size -= RelocDirectory->SizeOfBlock;

                RelocDirectory = RelocationBlock(
                    VA,
                    GetRelocCount(RelocDirectory->SizeOfBlock),
                    (PUSHORT)(RelocDirectory + 1),
                    Diff);
            }
        }
    }
}

VOID
NTAPI
InitializeLoadedModuleList(
    __in PRELOADER_PARAMETER_BLOCK Block
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING KernelString = { 0 };
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    ULONG SizeToLock = 0;
    PCHAR ControlPc = NULL;
    UNICODE_STRING RoutineString = { 0 };
    CONTEXT Context = { 0 };
    PDUMP_HEADER DumpHeader = NULL;
    PKDDEBUGGER_DATA64 KdDebuggerDataBlock = NULL;
    PKDDEBUGGER_DATA_ADDITION64 KdDebuggerDataAdditionBlock = NULL;

#ifdef _WIN64
    // 48 89 A3 D8 01 00 00             mov [rbx + 1D8h], rsp
    // 8B F8                            mov edi, eax
    // C1 EF 07                         shr edi, 7
    // 83 E7 20                         and edi, 20h
    // 25 FF 0F 00 00                   and eax, 0FFFh
    // 4C 8D 15 C7 20 23 00             lea r10, KeServiceDescriptorTable
    // 4C 8D 1D 00 21 23 00             lea r11, KeServiceDescriptorTableShadow

    CHAR KiSystemCall64[] =
        "48 89 a3 ?? ?? ?? ?? 8b f8 c1 ef 07 83 e7 20 25 ff 0f 00 00 4c 8d 15 ?? ?? ?? ?? 4c 8d 1d ?? ?? ?? ??";
#endif // _WIN64

    Context.ContextFlags = CONTEXT_FULL;

    RtlCaptureContext(&Context);

    DumpHeader = ExAllocatePool(NonPagedPool, DUMP_BLOCK_SIZE);

    if (NULL != DumpHeader) {
        KeCapturePersistentThreadState(
            &Context,
            NULL,
            0,
            0,
            0,
            0,
            0,
            DumpHeader);

        KdDebuggerDataBlock = (PCHAR)DumpHeader + KDDEBUGGER_DATA_OFFSET;

        RtlCopyMemory(
            &ReloaderBlock->DebuggerDataBlock,
            KdDebuggerDataBlock,
            sizeof(KDDEBUGGER_DATA64));

        KdDebuggerDataAdditionBlock = (PKDDEBUGGER_DATA_ADDITION64)(KdDebuggerDataBlock + 1);

        RtlCopyMemory(
            &ReloaderBlock->DebuggerDataAdditionBlock,
            KdDebuggerDataAdditionBlock,
            sizeof(KDDEBUGGER_DATA_ADDITION64));

#ifndef PUBLIC
        // DbgPrint("[Sefirot] [Tiferet] < %p > Header\n", KdDebuggerDataBlock->Header);
        DbgPrint("[Sefirot] [Tiferet] < %p > KernBase\n", KdDebuggerDataBlock->KernBase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > BreakpointWithStatus\n", KdDebuggerDataBlock->BreakpointWithStatus);
        // DbgPrint("[Sefirot] [Tiferet] < %p > SavedContext\n", KdDebuggerDataBlock->SavedContext);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ThCallbackStack\n", KdDebuggerDataBlock->ThCallbackStack);
        // DbgPrint("[Sefirot] [Tiferet] < %p > NextCallback\n", KdDebuggerDataBlock->NextCallback);
        // DbgPrint("[Sefirot] [Tiferet] < %p > FramePointer\n", KdDebuggerDataBlock->FramePointer);
        DbgPrint("[Sefirot] [Tiferet] < %p > PaeEnabled\n", KdDebuggerDataBlock->PaeEnabled);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KiCallUserMode\n", KdDebuggerDataBlock->KiCallUserMode);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KeUserCallbackDispatcher\n", KdDebuggerDataBlock->KeUserCallbackDispatcher);
        DbgPrint("[Sefirot] [Tiferet] < %p > PsLoadedModuleList\n", KdDebuggerDataBlock->PsLoadedModuleList);
        DbgPrint("[Sefirot] [Tiferet] < %p > PsActiveProcessHead\n", KdDebuggerDataBlock->PsActiveProcessHead);
        DbgPrint("[Sefirot] [Tiferet] < %p > PspCidTable\n", KdDebuggerDataBlock->PspCidTable);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ExpSystemResourcesList\n", KdDebuggerDataBlock->ExpSystemResourcesList);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ExpPagedPoolDescriptor\n", KdDebuggerDataBlock->ExpPagedPoolDescriptor);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ExpNumberOfPagedPools\n", KdDebuggerDataBlock->ExpNumberOfPagedPools);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KeTimeIncrement\n", KdDebuggerDataBlock->KeTimeIncrement);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KeBugCheckCallbackListHead\n", KdDebuggerDataBlock->KeBugCheckCallbackListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KiBugcheckData\n", KdDebuggerDataBlock->KiBugcheckData);
        // DbgPrint("[Sefirot] [Tiferet] < %p > IopErrorLogListHead\n", KdDebuggerDataBlock->IopErrorLogListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ObpRootDirectoryObject\n", KdDebuggerDataBlock->ObpRootDirectoryObject);
        // DbgPrint("[Sefirot] [Tiferet] < %p > ObpTypeObjectType\n", KdDebuggerDataBlock->ObpTypeObjectType);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemCacheStart\n", KdDebuggerDataBlock->MmSystemCacheStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemCacheEnd\n", KdDebuggerDataBlock->MmSystemCacheEnd);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemCacheWs\n", KdDebuggerDataBlock->MmSystemCacheWs);
        DbgPrint("[Sefirot] [Tiferet] < %p > MmPfnDatabase\n", KdDebuggerDataBlock->MmPfnDatabase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemPtesStart\n", KdDebuggerDataBlock->MmSystemPtesStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemPtesEnd\n", KdDebuggerDataBlock->MmSystemPtesEnd);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSubsectionBase\n", KdDebuggerDataBlock->MmSubsectionBase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmNumberOfPagingFiles\n", KdDebuggerDataBlock->MmNumberOfPagingFiles);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmLowestPhysicalPage\n", KdDebuggerDataBlock->MmLowestPhysicalPage);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmHighestPhysicalPage\n", KdDebuggerDataBlock->MmHighestPhysicalPage);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmNumberOfPhysicalPages\n", KdDebuggerDataBlock->MmNumberOfPhysicalPages);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmMaximumNonPagedPoolInBytes\n", KdDebuggerDataBlock->MmMaximumNonPagedPoolInBytes);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmNonPagedSystemStart\n", KdDebuggerDataBlock->MmNonPagedSystemStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmNonPagedPoolStart\n", KdDebuggerDataBlock->MmNonPagedPoolStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmNonPagedPoolEnd\n", KdDebuggerDataBlock->MmNonPagedPoolEnd);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPagedPoolStart\n", KdDebuggerDataBlock->MmPagedPoolStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPagedPoolEnd\n", KdDebuggerDataBlock->MmPagedPoolEnd);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPagedPoolInformation\n", KdDebuggerDataBlock->MmPagedPoolInformation);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPageSize\n", KdDebuggerDataBlock->MmPageSize);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSizeOfPagedPoolInBytes\n", KdDebuggerDataBlock->MmSizeOfPagedPoolInBytes);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmTotalCommitLimit\n", KdDebuggerDataBlock->MmTotalCommitLimit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmTotalCommittedPages\n", KdDebuggerDataBlock->MmTotalCommittedPages);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSharedCommit\n", KdDebuggerDataBlock->MmSharedCommit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmDriverCommit\n", KdDebuggerDataBlock->MmDriverCommit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmProcessCommit\n", KdDebuggerDataBlock->MmProcessCommit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPagedPoolCommit\n", KdDebuggerDataBlock->MmPagedPoolCommit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmExtendedCommit\n", KdDebuggerDataBlock->MmExtendedCommit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmZeroedPageListHead\n", KdDebuggerDataBlock->MmZeroedPageListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmFreePageListHead\n", KdDebuggerDataBlock->MmFreePageListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmStandbyPageListHead\n", KdDebuggerDataBlock->MmStandbyPageListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmModifiedPageListHead\n", KdDebuggerDataBlock->MmModifiedPageListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmModifiedNoWritePageListHead\n", KdDebuggerDataBlock->MmModifiedNoWritePageListHead);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmAvailablePages\n", KdDebuggerDataBlock->MmAvailablePages);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmResidentAvailablePages\n", KdDebuggerDataBlock->MmResidentAvailablePages);
        // DbgPrint("[Sefirot] [Tiferet] < %p > PoolTrackTable\n", KdDebuggerDataBlock->PoolTrackTable);
        // DbgPrint("[Sefirot] [Tiferet] < %p > NonPagedPoolDescriptor\n", KdDebuggerDataBlock->NonPagedPoolDescriptor);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmHighestUserAddress\n", KdDebuggerDataBlock->MmHighestUserAddress);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemRangeStart\n", KdDebuggerDataBlock->MmSystemRangeStart);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmUserProbeAddress\n", KdDebuggerDataBlock->MmUserProbeAddress);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintCircularBuffer\n", KdDebuggerDataBlock->KdPrintCircularBuffer);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintCircularBufferEnd\n", KdDebuggerDataBlock->KdPrintCircularBufferEnd);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintWritePointer\n", KdDebuggerDataBlock->KdPrintWritePointer);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintRolloverCount\n", KdDebuggerDataBlock->KdPrintRolloverCount);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmLoadedUserImageList\n", KdDebuggerDataBlock->MmLoadedUserImageList);
        // DbgPrint("[Sefirot] [Tiferet] < %p > NtBuildLab\n", KdDebuggerDataBlock->NtBuildLab);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KiNormalSystemCall\n", KdDebuggerDataBlock->KiNormalSystemCall);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KiProcessorBlock\n", KdDebuggerDataBlock->KiProcessorBlock);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmUnloadedDrivers\n", KdDebuggerDataBlock->MmUnloadedDrivers);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmLastUnloadedDriver\n", KdDebuggerDataBlock->MmLastUnloadedDriver);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmTriageActionTaken\n", KdDebuggerDataBlock->MmTriageActionTaken);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSpecialPoolTag\n", KdDebuggerDataBlock->MmSpecialPoolTag);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KernelVerifier\n", KdDebuggerDataBlock->KernelVerifier);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmVerifierData\n", KdDebuggerDataBlock->MmVerifierData);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmAllocatedNonPagedPool\n", KdDebuggerDataBlock->MmAllocatedNonPagedPool);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPeakCommitment\n", KdDebuggerDataBlock->MmPeakCommitment);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmTotalCommitLimitMaximum\n", KdDebuggerDataBlock->MmTotalCommitLimitMaximum);
        // DbgPrint("[Sefirot] [Tiferet] < %p > CmNtCSDVersion\n", KdDebuggerDataBlock->CmNtCSDVersion);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmPhysicalMemoryBlock\n", KdDebuggerDataBlock->MmPhysicalMemoryBlock);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSessionBase\n", KdDebuggerDataBlock->MmSessionBase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSessionSize\n", KdDebuggerDataBlock->MmSessionSize);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmSystemParentTablePage\n", KdDebuggerDataBlock->MmSystemParentTablePage);
        // DbgPrint("[Sefirot] [Tiferet] < %p > MmVirtualTranslationBase\n", KdDebuggerDataBlock->MmVirtualTranslationBase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadNextProcessor\n", KdDebuggerDataBlock->OffsetKThreadNextProcessor);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadTeb\n", KdDebuggerDataBlock->OffsetKThreadTeb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadKernelStack\n", KdDebuggerDataBlock->OffsetKThreadKernelStack);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadInitialStack\n", KdDebuggerDataBlock->OffsetKThreadInitialStack);
        DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadApcProcess\n", KdDebuggerDataBlock->OffsetKThreadApcProcess);
        DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadState\n", KdDebuggerDataBlock->OffsetKThreadState);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadBStore\n", KdDebuggerDataBlock->OffsetKThreadBStore);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetKThreadBStoreLimit\n", KdDebuggerDataBlock->OffsetKThreadBStoreLimit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > SizeEProcess\n", KdDebuggerDataBlock->SizeEProcess);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetEprocessPeb\n", KdDebuggerDataBlock->OffsetEprocessPeb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetEprocessParentCID\n", KdDebuggerDataBlock->OffsetEprocessParentCID);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetEprocessDirectoryTableBase\n", KdDebuggerDataBlock->OffsetEprocessDirectoryTableBase);
        // DbgPrint("[Sefirot] [Tiferet] < %p > SizePrcb\n", KdDebuggerDataBlock->SizePrcb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbDpcRoutine\n", KdDebuggerDataBlock->OffsetPrcbDpcRoutine);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbCurrentThread\n", KdDebuggerDataBlock->OffsetPrcbCurrentThread);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbMhz\n", KdDebuggerDataBlock->OffsetPrcbMhz);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbCpuType\n", KdDebuggerDataBlock->OffsetPrcbCpuType);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbVendorString\n", KdDebuggerDataBlock->OffsetPrcbVendorString);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbProcStateContext\n", KdDebuggerDataBlock->OffsetPrcbProcStateContext);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbNumber\n", KdDebuggerDataBlock->OffsetPrcbNumber);
        // DbgPrint("[Sefirot] [Tiferet] < %p > SizeEThread\n", KdDebuggerDataBlock->SizeEThread);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintCircularBufferPtr\n", KdDebuggerDataBlock->KdPrintCircularBufferPtr);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KdPrintBufferSize\n", KdDebuggerDataBlock->KdPrintBufferSize);
        // DbgPrint("[Sefirot] [Tiferet] < %p > KeLoaderBlock\n", KdDebuggerDataBlock->KeLoaderBlock);
        // DbgPrint("[Sefirot] [Tiferet] < %p > SizePcr\n", KdDebuggerDataBlock->SizePcr);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrSelfPcr\n", KdDebuggerDataBlock->OffsetPcrSelfPcr);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrCurrentPrcb\n", KdDebuggerDataBlock->OffsetPcrCurrentPrcb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrContainedPrcb\n", KdDebuggerDataBlock->OffsetPcrContainedPrcb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrInitialBStore\n", KdDebuggerDataBlock->OffsetPcrInitialBStore);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrBStoreLimit\n", KdDebuggerDataBlock->OffsetPcrBStoreLimit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrInitialStack\n", KdDebuggerDataBlock->OffsetPcrInitialStack);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPcrStackLimit\n", KdDebuggerDataBlock->OffsetPcrStackLimit);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbPcrPage\n", KdDebuggerDataBlock->OffsetPrcbPcrPage);
        // DbgPrint("[Sefirot] [Tiferet] < %p > OffsetPrcbProcStateSpecialReg\n", KdDebuggerDataBlock->OffsetPrcbProcStateSpecialReg);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR0Code\n", KdDebuggerDataBlock->GdtR0Code);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR0Data\n", KdDebuggerDataBlock->GdtR0Data);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR0Pcr\n", KdDebuggerDataBlock->GdtR0Pcr);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR3Code\n", KdDebuggerDataBlock->GdtR3Code);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR3Data\n", KdDebuggerDataBlock->GdtR3Data);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtR3Teb\n", KdDebuggerDataBlock->GdtR3Teb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtLdt\n", KdDebuggerDataBlock->GdtLdt);
        // DbgPrint("[Sefirot] [Tiferet] < %p > GdtTss\n", KdDebuggerDataBlock->GdtTss);
        // DbgPrint("[Sefirot] [Tiferet] < %p > Gdt64R3CmCode\n", KdDebuggerDataBlock->Gdt64R3CmCode);
        // DbgPrint("[Sefirot] [Tiferet] < %p > Gdt64R3CmTeb\n", KdDebuggerDataBlock->Gdt64R3CmTeb);
        // DbgPrint("[Sefirot] [Tiferet] < %p > IopNumTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopNumTriageDumpDataBlocks);
        // DbgPrint("[Sefirot] [Tiferet] < %p > IopTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopTriageDumpDataBlocks);

        if (ReloaderBlock->BuildNumber >= 10586) {
            DbgPrint("[Sefirot] [Tiferet] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
        }
#endif // !PUBLIC

        ExFreePool(DumpHeader);
    }

    InitializeListHead(&Block->LoadedPrivateImageList);

    if (NULL != Block->CoreDataTableEntry) {
        Block->CoreDataTableEntry->LoadCount++;

        InsertTailList(
            &Block->LoadedPrivateImageList,
            &Block->CoreDataTableEntry->InLoadOrderLinks);

        CaptureImageExceptionValues(
            Block->CoreDataTableEntry->DllBase,
            &Block->CoreDataTableEntry->ExceptionTable,
            &Block->CoreDataTableEntry->ExceptionTableSize);
    }

#ifndef _WIN64
    RtlInitUnicodeString(&RoutineString, L"KeServiceDescriptorTable");

    Block->ServiceDescriptorTable = MmGetSystemRoutineAddress(&RoutineString);
#else
    NtHeaders = RtlImageNtHeader((PVOID)Block->DebuggerDataBlock.KernBase);

    if (NULL != NtHeaders) {
        NtSection = IMAGE_FIRST_SECTION(NtHeaders);

        SectionBase =
            (PCHAR)Block->DebuggerDataBlock.KernBase + NtSection[0].VirtualAddress;

        SizeToLock = max(
            NtSection[0].SizeOfRawData,
            NtSection[0].Misc.VirtualSize);

        ControlPc = ScanBytes(
            SectionBase,
            (PCHAR)SectionBase + SizeToLock,
            KiSystemCall64);

        if (NULL != ControlPc) {
            Block->ServiceDescriptorTable = __RVA_TO_VA(ControlPc + 23);
        }
    }
#endif // !_WIN64
}

NTSTATUS
NTAPI
FindEntryForKernelPrivateImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty(&ReloaderBlock->LoadedPrivateImageList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            ReloaderBlock->LoadedPrivateImageList.Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry != (ULONG_PTR)&ReloaderBlock->LoadedPrivateImageList) {
            if (RtlEqualUnicodeString(
                ImageFileName,
                &FoundDataTableEntry->BaseDllName,
                TRUE)) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty((PLIST_ENTRY)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            ((PLIST_ENTRY)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList)->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList) {
            if (RtlEqualUnicodeString(
                ImageFileName,
                &FoundDataTableEntry->BaseDllName,
                TRUE)) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelPrivateImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty(&ReloaderBlock->LoadedPrivateImageList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            ReloaderBlock->LoadedPrivateImageList.Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)&ReloaderBlock->LoadedPrivateImageList) {
            if ((ULONG_PTR)Address >= (ULONG_PTR)FoundDataTableEntry->DllBase &&
                (ULONG_PTR)Address < (ULONG_PTR)FoundDataTableEntry->DllBase +
                FoundDataTableEntry->SizeOfImage) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty((PLIST_ENTRY)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            ((PLIST_ENTRY)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList)->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)ReloaderBlock->DebuggerDataBlock.PsLoadedModuleList) {
            if ((ULONG_PTR)Address >= (ULONG_PTR)FoundDataTableEntry->DllBase &&
                (ULONG_PTR)Address < (ULONG_PTR)FoundDataTableEntry->DllBase +
                FoundDataTableEntry->SizeOfImage) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

PVOID
NTAPI
GetKernelForwarder(
    __in PSTR ForwarderData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTR Separator = NULL;
    PSTR ImageName = NULL;
    PSTR ProcedureName = NULL;
    ULONG ProcedureNumber = 0;
    PVOID ProcedureAddress = NULL;
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    Separator = strchr(
        ForwarderData,
        '.');

    if (Separator) {
        ImageName = ExAllocatePool(
            NonPagedPool,
            Separator - ForwarderData);

        if (ImageName) {
            RtlCopyMemory(
                ImageName,
                ForwarderData,
                Separator - ForwarderData);

            RtlInitAnsiString(
                &TempImageFileName,
                ImageName);

            Status = RtlAnsiStringToUnicodeString(
                &ImageFileName,
                &TempImageFileName,
                TRUE);

            if (TRACE(Status)) {
                Status = FindEntryForKernelPrivateImage(
                    &ImageFileName,
                    &DataTableEntry);

                if (TRACE(Status)) {
                    Separator += 1;
                    ProcedureName = Separator;

                    if (Separator[0] != '#') {
                        ProcedureAddress = GetKernelProcedureAddress(
                            DataTableEntry->DllBase,
                            ProcedureName,
                            0);
                    }
                    else {
                        Separator += 1;

                        if (RtlCharToInteger(
                            Separator,
                            0,
                            &ProcedureNumber) >= 0) {
                            ProcedureAddress = GetKernelProcedureAddress(
                                DataTableEntry->DllBase,
                                NULL,
                                ProcedureNumber);
                        }
                    }
                }
                else {
                    Status = FindEntryForKernelImage(
                        &ImageFileName,
                        &DataTableEntry);

                    if (TRACE(Status)) {
                        Separator += 1;
                        ProcedureName = Separator;

                        if (Separator[0] != '#') {
                            ProcedureAddress = GetKernelProcedureAddress(
                                DataTableEntry->DllBase,
                                ProcedureName,
                                0);
                        }
                        else {
                            Separator += 1;

                            if (RtlCharToInteger(
                                Separator,
                                0,
                                &ProcedureNumber) >= 0) {
                                ProcedureAddress = GetKernelProcedureAddress(
                                    DataTableEntry->DllBase,
                                    NULL,
                                    ProcedureNumber);
                            }
                        }
                    }
                }

                RtlFreeUnicodeString(&ImageFileName);
            }

            ExFreePool(ImageName);
        }
    }

    return ProcedureAddress;
}

PVOID
NTAPI
GetKernelProcedureAddress(
    __in PVOID ImageBase,
    __in_opt PSTR ProcedureName,
    __in_opt ULONG ProcedureNumber
)
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    ULONG Size = 0;
    PULONG NameTable = NULL;
    PUSHORT OrdinalTable = NULL;
    PULONG AddressTable = NULL;
    PSTR NameTableName = NULL;
    USHORT HintIndex = 0;
    PVOID ProcedureAddress = NULL;

    ExportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable = (PCHAR)ImageBase + ExportDirectory->AddressOfNames;
        OrdinalTable = (PCHAR)ImageBase + ExportDirectory->AddressOfNameOrdinals;
        AddressTable = (PCHAR)ImageBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            if (ProcedureNumber >= ExportDirectory->Base &&
                ProcedureNumber < MAXSHORT) {
                ProcedureAddress = (PCHAR)ImageBase +
                    AddressTable[ProcedureNumber - ExportDirectory->Base];
            }
            else {
                for (HintIndex = 0;
                    HintIndex < ExportDirectory->NumberOfNames;
                    HintIndex++) {
                    NameTableName = (PCHAR)ImageBase + NameTable[HintIndex];

                    if (0 == _stricmp(
                        ProcedureName,
                        NameTableName)) {
                        ProcedureAddress = (PCHAR)ImageBase +
                            AddressTable[OrdinalTable[HintIndex]];
                    }
                }
            }
        }

        if ((ULONG_PTR)ProcedureAddress >= (ULONG_PTR)ExportDirectory &&
            (ULONG_PTR)ProcedureAddress < (ULONG_PTR)ExportDirectory + Size) {
            ProcedureAddress = GetKernelForwarder(ProcedureAddress);
        }
    }

    return ProcedureAddress;
}

VOID
NTAPI
DereferenceKernelImage(
    __in PSTR ImageName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    RtlInitAnsiString(
        &TempImageFileName,
        ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempImageFileName,
        TRUE);


    if (TRACE(Status)) {
        Status = FindEntryForKernelPrivateImage(
            &ImageFileName,
            &DataTableEntry);

        if (TRACE(Status)) {
            DataTableEntry->LoadCount--;
        }
        else {
            Status = FindEntryForKernelImage(
                &ImageFileName,
                &DataTableEntry);

            if (TRACE(Status)) {
                DataTableEntry->LoadCount--;
            }
        }

        RtlFreeUnicodeString(&ImageFileName);
    }
}

VOID
NTAPI
DereferenceKernelImageImports(
    __in PVOID ImageBase
)
{
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    ULONG Size = 0;
    PSTR ImportImageName = NULL;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            ImportImageName = (PCHAR)ImageBase + ImportDirectory->Name;

            DereferenceKernelImage(ImportImageName);

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

PVOID
NTAPI
ReferenceKernelImage(
    __in PVOID ImageName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    PVOID ImageBase = NULL;

    RtlInitAnsiString(&TempImageFileName, ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempImageFileName,
        TRUE);

    if (TRACE(Status)) {
        Status = FindEntryForKernelPrivateImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount++;
            ImageBase = DataTableEntry->DllBase;
        }
        else {
            Status = FindEntryForKernelImage(
                &ImageFileName,
                &DataTableEntry);

            if (NT_SUCCESS(Status)) {
                DataTableEntry->LoadCount++;
                ImageBase = DataTableEntry->DllBase;
            }
        }

        RtlFreeUnicodeString(&ImageFileName);
    }

    return ImageBase;
}

LONG
NTAPI
NameToNumber(
    __in PSTR String
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE FileHandle = NULL;
    HANDLE SectionHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    PVOID ViewBase = NULL;
    SIZE_T ViewSize = 0;
    PCHAR TargetPc = NULL;
    LONG Number = -1;

    RtlInitUnicodeString(
        &ImageFileName,
        L"\\SystemRoot\\System32\\ntdll.dll");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &ImageFileName,
        (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
        NULL,
        NULL);

    Status = ZwOpenFile(
        &FileHandle,
        FILE_EXECUTE,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        0);

    if (TRACE(Status)) {
        InitializeObjectAttributes(
            &ObjectAttributes,
            NULL,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            NULL);

        Status = ZwCreateSection(
            &SectionHandle,
            SECTION_MAP_READ | SECTION_MAP_EXECUTE,
            &ObjectAttributes,
            NULL,
            PAGE_EXECUTE,
            SEC_IMAGE,
            FileHandle);

        if (TRACE(Status)) {
            Status = ZwMapViewOfSection(
                SectionHandle,
                ZwCurrentProcess(),
                &ViewBase,
                0L,
                0L,
                NULL,
                &ViewSize,
                ViewShare,
                0L,
                PAGE_EXECUTE);

            if (TRACE(Status)) {
                TargetPc = GetKernelProcedureAddress(
                    ViewBase,
                    String,
                    0);

                if (NULL != TargetPc) {
#ifndef _WIN64
                    Number = *(PLONG)(TargetPc + 1);
#else
                    Number = *(PLONG)(TargetPc + 4);
#endif // !_WIN64
                }

                ZwUnmapViewOfSection(NtCurrentProcess(), ViewBase);
            }

            ZwClose(SectionHandle);
        }

        ZwClose(FileHandle);
    }

    return Number;
}

PVOID
NTAPI
NameToAddress(
    __in PSTR String
)
{
    PVOID RoutineAddress = NULL;
    UNICODE_STRING RoutineString = { 0 };
    LONG Number = -1;
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    ULONG FirstLength = 0;
    ULONG Length = 0;

    static ULONG Interval;

    if (0 == _CmpByte(String[0], 'Z') &&
        0 == _CmpByte(String[1], 'w')) {
        RtlInitUnicodeString(&RoutineString, L"ZwClose");

        ControlPc = MmGetSystemRoutineAddress(&RoutineString);

        if (NULL != ControlPc) {
            Number = NameToNumber("NtClose");

            if (0 == Interval) {
                FirstLength = DetourGetInstructionLength(ControlPc);

                TargetPc = ControlPc + FirstLength;

                while (TRUE) {
                    Length = DetourGetInstructionLength(TargetPc);

                    if (FirstLength == Length) {
#ifndef _WIN64
                        if (0 == _CmpByte(TargetPc[0], ControlPc[0]) &&
                            1 == *(PLONG)&TargetPc[1] - *(PLONG)&ControlPc[1]) {
                            Interval = TargetPc - ControlPc;
                            break;
                        }
#else
                        if (FirstLength == RtlCompareMemory(
                            TargetPc,
                            ControlPc,
                            FirstLength)) {
                            Interval = TargetPc - ControlPc;
                            break;
                        }
#endif // !_WIN64
                    }

                    TargetPc += Length;
                }
            }

            RoutineAddress = ControlPc +
                Interval * (LONG_PTR)(NameToNumber(String) - Number);
        }
    }
    else if (0 == _CmpByte(String[0], 'N') &&
        0 == _CmpByte(String[1], 't')) {
        Number = NameToNumber(String);

#ifndef _WIN64
        RoutineAddress = UlongToPtr(ReloaderBlock->ServiceDescriptorTable[0].Base[Number]);
#else
        RoutineAddress = (PCHAR)ReloaderBlock->ServiceDescriptorTable[0].Base +
            (((PLONG)ReloaderBlock->ServiceDescriptorTable[0].Base)[Number] >> 4);
#endif // !_WIN64
    }

    return RoutineAddress;
}

VOID
NTAPI
SnapKernelThunk(
    __in PVOID ImageBase
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    ULONG Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    PSTR ImportImageName = NULL;
    PVOID ImportImageBase = NULL;
    USHORT Ordinal = 0;
    PVOID FunctionAddress = NULL;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (PCHAR)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (PCHAR)ImageBase + ImportDirectory->FirstThunk;
            ImportImageName = (PCHAR)ImageBase + ImportDirectory->Name;

            ImportImageBase = ReferenceKernelImage(ImportImageName);

            if (NULL != ImportImageBase) {
                do {
                    if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                        Ordinal = (USHORT)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                        FunctionAddress = GetKernelProcedureAddress(
                            ImportImageBase,
                            NULL,
                            Ordinal);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[Sefirot] [Tiferet] import procedure ordinal@%d not found\n",
                                Ordinal);
                        }
                    }
                    else {
                        ImportByName = (PCHAR)ImageBase + OriginalThunk->u1.AddressOfData;

                        if ((0 == _CmpByte(ImportByName->Name[0], 'Z') &&
                            0 == _CmpByte(ImportByName->Name[1], 'w')) ||
                            (0 == _CmpByte(ImportByName->Name[0], 'N') &&
                                0 == _CmpByte(ImportByName->Name[1], 't'))) {
                            FunctionAddress = NameToAddress(ImportByName->Name);
                        }
                        else {
                            FunctionAddress = GetKernelProcedureAddress(
                                ImportImageBase,
                                ImportByName->Name,
                                0);
                        }

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[Sefirot] [Tiferet] import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);
            }
            else {
                DbgPrint(
                    "[Sefirot] [Tiferet] import dll %hs not found\n",
                    ImportImageName);
            }

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

PVOID
NTAPI
AllocateKernelPrivateImage(
    __in PVOID ViewBase
)
{
    PVOID ImageBase = NULL;
    ULONG SizeOfEntry = 0;

    SizeOfEntry = GetSizeOfImage(ViewBase) + PAGE_SIZE;

    ImageBase = ExAllocatePool(NonPagedPool, SizeOfEntry);

    if (NULL != ImageBase) {
        ImageBase = (PCHAR)ImageBase + PAGE_SIZE;
    }

    return ImageBase;
}

PVOID
NTAPI
MapKernelPrivateImage(
    __in PVOID ViewBase
)
{
    PVOID ImageBase = NULL;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    LONG_PTR Diff = 0;
    SIZE_T Index = 0;

    NtHeaders = RtlImageNtHeader(ViewBase);

    if (NULL != NtHeaders) {
        ImageBase = AllocateKernelPrivateImage(ViewBase);

        if (NULL != ImageBase) {
            RtlZeroMemory(
                ImageBase,
                NtHeaders->OptionalHeader.SizeOfImage);

            NtSection = IMAGE_FIRST_SECTION(NtHeaders);

            RtlCopyMemory(
                ImageBase,
                ViewBase,
                NtSection->VirtualAddress);

            for (Index = 0;
                Index < NtHeaders->FileHeader.NumberOfSections;
                Index++) {
                if (0 != NtSection[Index].VirtualAddress) {
                    RtlCopyMemory(
                        (PCHAR)ImageBase + NtSection[Index].VirtualAddress,
                        (PCHAR)ViewBase + NtSection[Index].PointerToRawData,
                        NtSection[Index].SizeOfRawData);
                }
            }

            NtHeaders = RtlImageNtHeader(ImageBase);

            Diff = (LONG_PTR)ImageBase - NtHeaders->OptionalHeader.ImageBase;
            NtHeaders->OptionalHeader.ImageBase = (ULONG_PTR)ImageBase;

            RelocateImage(ImageBase, Diff);
            SnapKernelThunk(ImageBase);
        }
    }

    return ImageBase;
}

PKLDR_DATA_TABLE_ENTRY
NTAPI
LoadKernelPrivateImage(
    __in PVOID ViewBase,
    __in PCWSTR ImageName,
    __in BOOLEAN Insert
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING ImageFileName = { 0 };
    PVOID ImageBase = NULL;
    PCWSTR BaseName = NULL;

    BaseName = wcsrchr(ImageName, L'\\');

    if (NULL == BaseName) {
        BaseName = ImageName;
    }
    else {
        BaseName++;
    }

    RtlInitUnicodeString(&ImageFileName, BaseName);

    Status = FindEntryForKernelPrivateImage(
        &ImageFileName,
        &DataTableEntry);

    if (NT_SUCCESS(Status)) {
        DataTableEntry->LoadCount++;
    }
    else {
        ImageBase = MapKernelPrivateImage(ViewBase);

        if (NULL != ImageBase) {
            DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)
                ((PCHAR)ImageBase - PAGE_SIZE);

            RtlZeroMemory(
                DataTableEntry,
                sizeof(KLDR_DATA_TABLE_ENTRY) +
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) * 2);

            DataTableEntry->DllBase = ImageBase;
            DataTableEntry->SizeOfImage = GetSizeOfImage(ImageBase);
            DataTableEntry->EntryPoint = GetAddressOfEntryPoint(ImageBase);
            DataTableEntry->Flags = LDRP_STATIC_LINK;
            DataTableEntry->LoadCount = 0;

            DataTableEntry->FullDllName.Buffer = DataTableEntry + 1;

            DataTableEntry->FullDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

            wcscpy(DataTableEntry->FullDllName.Buffer, SystemDirectory);
            wcscat(DataTableEntry->FullDllName.Buffer, BaseName);

            DataTableEntry->FullDllName.Length =
                wcslen(DataTableEntry->FullDllName.Buffer) * sizeof(WCHAR);

            DataTableEntry->BaseDllName.Buffer =
                DataTableEntry->FullDllName.Buffer + MAXIMUM_FILENAME_LENGTH;

            DataTableEntry->BaseDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

            wcscpy(DataTableEntry->BaseDllName.Buffer, BaseName);

            DataTableEntry->BaseDllName.Length =
                wcslen(DataTableEntry->BaseDllName.Buffer) * sizeof(WCHAR);

            if (FALSE != Insert) {
                DataTableEntry->LoadCount++;

                CaptureImageExceptionValues(
                    DataTableEntry->DllBase,
                    &DataTableEntry->ExceptionTable,
                    &DataTableEntry->ExceptionTableSize);

                InsertTailList(
                    &ReloaderBlock->LoadedPrivateImageList,
                    &DataTableEntry->InLoadOrderLinks);
            }
        }
    }

    return DataTableEntry;
}

VOID
NTAPI
UnloadKernelPrivateImage(
    __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
)
{
    if (0 == DataTableEntry->LoadCount) {
        DereferenceKernelImageImports(DataTableEntry->DllBase);
        RemoveEntryList(&DataTableEntry->InLoadOrderLinks);

        ExFreePool(DataTableEntry);
    }
    else {
        DataTableEntry->LoadCount--;

        if (0 == DataTableEntry->LoadCount) {
            DereferenceKernelImageImports(DataTableEntry->DllBase);
            RemoveEntryList(&DataTableEntry->InLoadOrderLinks);

            ExFreePool(DataTableEntry);
        }
    }
}
