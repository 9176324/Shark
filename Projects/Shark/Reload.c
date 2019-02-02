/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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
#include "Detours.h"
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
        /// DbgPrint("[Shark] < %p > Header\n", KdDebuggerDataBlock->Header);
        //DbgPrint("[Shark] < %p > KernBase\n", KdDebuggerDataBlock->KernBase);
        /// DbgPrint("[Shark] < %p > BreakpointWithStatus\n", KdDebuggerDataBlock->BreakpointWithStatus);
        /// DbgPrint("[Shark] < %p > SavedContext\n", KdDebuggerDataBlock->SavedContext);
        /// DbgPrint("[Shark] < %p > ThCallbackStack\n", KdDebuggerDataBlock->ThCallbackStack);
        /// DbgPrint("[Shark] < %p > NextCallback\n", KdDebuggerDataBlock->NextCallback);
        /// DbgPrint("[Shark] < %p > FramePointer\n", KdDebuggerDataBlock->FramePointer);
        // DbgPrint("[Shark] < %p > PaeEnabled\n", KdDebuggerDataBlock->PaeEnabled);
        /// DbgPrint("[Shark] < %p > KiCallUserMode\n", KdDebuggerDataBlock->KiCallUserMode);
        /// DbgPrint("[Shark] < %p > KeUserCallbackDispatcher\n", KdDebuggerDataBlock->KeUserCallbackDispatcher);
        // DbgPrint("[Shark] < %p > PsLoadedModuleList\n", KdDebuggerDataBlock->PsLoadedModuleList);
        // DbgPrint("[Shark] < %p > PsActiveProcessHead\n", KdDebuggerDataBlock->PsActiveProcessHead);
        // DbgPrint("[Shark] < %p > PspCidTable\n", KdDebuggerDataBlock->PspCidTable);
        /// DbgPrint("[Shark] < %p > ExpSystemResourcesList\n", KdDebuggerDataBlock->ExpSystemResourcesList);
        /// DbgPrint("[Shark] < %p > ExpPagedPoolDescriptor\n", KdDebuggerDataBlock->ExpPagedPoolDescriptor);
        /// DbgPrint("[Shark] < %p > ExpNumberOfPagedPools\n", KdDebuggerDataBlock->ExpNumberOfPagedPools);
        /// DbgPrint("[Shark] < %p > KeTimeIncrement\n", KdDebuggerDataBlock->KeTimeIncrement);
        /// DbgPrint("[Shark] < %p > KeBugCheckCallbackListHead\n", KdDebuggerDataBlock->KeBugCheckCallbackListHead);
        /// DbgPrint("[Shark] < %p > KiBugcheckData\n", KdDebuggerDataBlock->KiBugcheckData);
        /// DbgPrint("[Shark] < %p > IopErrorLogListHead\n", KdDebuggerDataBlock->IopErrorLogListHead);
        /// DbgPrint("[Shark] < %p > ObpRootDirectoryObject\n", KdDebuggerDataBlock->ObpRootDirectoryObject);
        /// DbgPrint("[Shark] < %p > ObpTypeObjectType\n", KdDebuggerDataBlock->ObpTypeObjectType);
        /// DbgPrint("[Shark] < %p > MmSystemCacheStart\n", KdDebuggerDataBlock->MmSystemCacheStart);
        /// DbgPrint("[Shark] < %p > MmSystemCacheEnd\n", KdDebuggerDataBlock->MmSystemCacheEnd);
        /// DbgPrint("[Shark] < %p > MmSystemCacheWs\n", KdDebuggerDataBlock->MmSystemCacheWs);
        // DbgPrint("[Shark] < %p > MmPfnDatabase\n", KdDebuggerDataBlock->MmPfnDatabase);
        /// DbgPrint("[Shark] < %p > MmSystemPtesStart\n", KdDebuggerDataBlock->MmSystemPtesStart);
        /// DbgPrint("[Shark] < %p > MmSystemPtesEnd\n", KdDebuggerDataBlock->MmSystemPtesEnd);
        /// DbgPrint("[Shark] < %p > MmSubsectionBase\n", KdDebuggerDataBlock->MmSubsectionBase);
        /// DbgPrint("[Shark] < %p > MmNumberOfPagingFiles\n", KdDebuggerDataBlock->MmNumberOfPagingFiles);
        /// DbgPrint("[Shark] < %p > MmLowestPhysicalPage\n", KdDebuggerDataBlock->MmLowestPhysicalPage);
        /// DbgPrint("[Shark] < %p > MmHighestPhysicalPage\n", KdDebuggerDataBlock->MmHighestPhysicalPage);
        /// DbgPrint("[Shark] < %p > MmNumberOfPhysicalPages\n", KdDebuggerDataBlock->MmNumberOfPhysicalPages);
        /// DbgPrint("[Shark] < %p > MmMaximumNonPagedPoolInBytes\n", KdDebuggerDataBlock->MmMaximumNonPagedPoolInBytes);
        /// DbgPrint("[Shark] < %p > MmNonPagedSystemStart\n", KdDebuggerDataBlock->MmNonPagedSystemStart);
        /// DbgPrint("[Shark] < %p > MmNonPagedPoolStart\n", KdDebuggerDataBlock->MmNonPagedPoolStart);
        /// DbgPrint("[Shark] < %p > MmNonPagedPoolEnd\n", KdDebuggerDataBlock->MmNonPagedPoolEnd);
        /// DbgPrint("[Shark] < %p > MmPagedPoolStart\n", KdDebuggerDataBlock->MmPagedPoolStart);
        /// DbgPrint("[Shark] < %p > MmPagedPoolEnd\n", KdDebuggerDataBlock->MmPagedPoolEnd);
        /// DbgPrint("[Shark] < %p > MmPagedPoolInformation\n", KdDebuggerDataBlock->MmPagedPoolInformation);
        /// DbgPrint("[Shark] < %p > MmPageSize\n", KdDebuggerDataBlock->MmPageSize);
        /// DbgPrint("[Shark] < %p > MmSizeOfPagedPoolInBytes\n", KdDebuggerDataBlock->MmSizeOfPagedPoolInBytes);
        /// DbgPrint("[Shark] < %p > MmTotalCommitLimit\n", KdDebuggerDataBlock->MmTotalCommitLimit);
        /// DbgPrint("[Shark] < %p > MmTotalCommittedPages\n", KdDebuggerDataBlock->MmTotalCommittedPages);
        /// DbgPrint("[Shark] < %p > MmSharedCommit\n", KdDebuggerDataBlock->MmSharedCommit);
        /// DbgPrint("[Shark] < %p > MmDriverCommit\n", KdDebuggerDataBlock->MmDriverCommit);
        /// DbgPrint("[Shark] < %p > MmProcessCommit\n", KdDebuggerDataBlock->MmProcessCommit);
        /// DbgPrint("[Shark] < %p > MmPagedPoolCommit\n", KdDebuggerDataBlock->MmPagedPoolCommit);
        /// DbgPrint("[Shark] < %p > MmExtendedCommit\n", KdDebuggerDataBlock->MmExtendedCommit);
        /// DbgPrint("[Shark] < %p > MmZeroedPageListHead\n", KdDebuggerDataBlock->MmZeroedPageListHead);
        /// DbgPrint("[Shark] < %p > MmFreePageListHead\n", KdDebuggerDataBlock->MmFreePageListHead);
        /// DbgPrint("[Shark] < %p > MmStandbyPageListHead\n", KdDebuggerDataBlock->MmStandbyPageListHead);
        /// DbgPrint("[Shark] < %p > MmModifiedPageListHead\n", KdDebuggerDataBlock->MmModifiedPageListHead);
        /// DbgPrint("[Shark] < %p > MmModifiedNoWritePageListHead\n", KdDebuggerDataBlock->MmModifiedNoWritePageListHead);
        /// DbgPrint("[Shark] < %p > MmAvailablePages\n", KdDebuggerDataBlock->MmAvailablePages);
        /// DbgPrint("[Shark] < %p > MmResidentAvailablePages\n", KdDebuggerDataBlock->MmResidentAvailablePages);
        /// DbgPrint("[Shark] < %p > PoolTrackTable\n", KdDebuggerDataBlock->PoolTrackTable);
        /// DbgPrint("[Shark] < %p > NonPagedPoolDescriptor\n", KdDebuggerDataBlock->NonPagedPoolDescriptor);
        /// DbgPrint("[Shark] < %p > MmHighestUserAddress\n", KdDebuggerDataBlock->MmHighestUserAddress);
        /// DbgPrint("[Shark] < %p > MmSystemRangeStart\n", KdDebuggerDataBlock->MmSystemRangeStart);
        /// DbgPrint("[Shark] < %p > MmUserProbeAddress\n", KdDebuggerDataBlock->MmUserProbeAddress);
        /// DbgPrint("[Shark] < %p > KdPrintCircularBuffer\n", KdDebuggerDataBlock->KdPrintCircularBuffer);
        /// DbgPrint("[Shark] < %p > KdPrintCircularBufferEnd\n", KdDebuggerDataBlock->KdPrintCircularBufferEnd);
        /// DbgPrint("[Shark] < %p > KdPrintWritePointer\n", KdDebuggerDataBlock->KdPrintWritePointer);
        /// DbgPrint("[Shark] < %p > KdPrintRolloverCount\n", KdDebuggerDataBlock->KdPrintRolloverCount);
        /// DbgPrint("[Shark] < %p > MmLoadedUserImageList\n", KdDebuggerDataBlock->MmLoadedUserImageList);
        /// DbgPrint("[Shark] < %p > NtBuildLab\n", KdDebuggerDataBlock->NtBuildLab);
        /// DbgPrint("[Shark] < %p > KiNormalSystemCall\n", KdDebuggerDataBlock->KiNormalSystemCall);
        /// DbgPrint("[Shark] < %p > KiProcessorBlock\n", KdDebuggerDataBlock->KiProcessorBlock);
        /// DbgPrint("[Shark] < %p > MmUnloadedDrivers\n", KdDebuggerDataBlock->MmUnloadedDrivers);
        /// DbgPrint("[Shark] < %p > MmLastUnloadedDriver\n", KdDebuggerDataBlock->MmLastUnloadedDriver);
        /// DbgPrint("[Shark] < %p > MmTriageActionTaken\n", KdDebuggerDataBlock->MmTriageActionTaken);
        /// DbgPrint("[Shark] < %p > MmSpecialPoolTag\n", KdDebuggerDataBlock->MmSpecialPoolTag);
        /// DbgPrint("[Shark] < %p > KernelVerifier\n", KdDebuggerDataBlock->KernelVerifier);
        /// DbgPrint("[Shark] < %p > MmVerifierData\n", KdDebuggerDataBlock->MmVerifierData);
        /// DbgPrint("[Shark] < %p > MmAllocatedNonPagedPool\n", KdDebuggerDataBlock->MmAllocatedNonPagedPool);
        /// DbgPrint("[Shark] < %p > MmPeakCommitment\n", KdDebuggerDataBlock->MmPeakCommitment);
        /// DbgPrint("[Shark] < %p > MmTotalCommitLimitMaximum\n", KdDebuggerDataBlock->MmTotalCommitLimitMaximum);
        /// DbgPrint("[Shark] < %p > CmNtCSDVersion\n", KdDebuggerDataBlock->CmNtCSDVersion);
        /// DbgPrint("[Shark] < %p > MmPhysicalMemoryBlock\n", KdDebuggerDataBlock->MmPhysicalMemoryBlock);
        /// DbgPrint("[Shark] < %p > MmSessionBase\n", KdDebuggerDataBlock->MmSessionBase);
        /// DbgPrint("[Shark] < %p > MmSessionSize\n", KdDebuggerDataBlock->MmSessionSize);
        /// DbgPrint("[Shark] < %p > MmSystemParentTablePage\n", KdDebuggerDataBlock->MmSystemParentTablePage);
        /// DbgPrint("[Shark] < %p > MmVirtualTranslationBase\n", KdDebuggerDataBlock->MmVirtualTranslationBase);
        // DbgPrint("[Shark] < %p > OffsetKThreadNextProcessor\n", KdDebuggerDataBlock->OffsetKThreadNextProcessor);
        // DbgPrint("[Shark] < %p > OffsetKThreadTeb\n", KdDebuggerDataBlock->OffsetKThreadTeb);
        // DbgPrint("[Shark] < %p > OffsetKThreadKernelStack\n", KdDebuggerDataBlock->OffsetKThreadKernelStack);
        // DbgPrint("[Shark] < %p > OffsetKThreadInitialStack\n", KdDebuggerDataBlock->OffsetKThreadInitialStack);
        // DbgPrint("[Shark] < %p > OffsetKThreadApcProcess\n", KdDebuggerDataBlock->OffsetKThreadApcProcess);
        // DbgPrint("[Shark] < %p > OffsetKThreadState\n", KdDebuggerDataBlock->OffsetKThreadState);
        // DbgPrint("[Shark] < %p > OffsetKThreadBStore\n", KdDebuggerDataBlock->OffsetKThreadBStore);
        // DbgPrint("[Shark] < %p > OffsetKThreadBStoreLimit\n", KdDebuggerDataBlock->OffsetKThreadBStoreLimit);
        // DbgPrint("[Shark] < %p > SizeEProcess\n", KdDebuggerDataBlock->SizeEProcess);
        // DbgPrint("[Shark] < %p > OffsetEprocessPeb\n", KdDebuggerDataBlock->OffsetEprocessPeb);
        // DbgPrint("[Shark] < %p > OffsetEprocessParentCID\n", KdDebuggerDataBlock->OffsetEprocessParentCID);
        // DbgPrint("[Shark] < %p > OffsetEprocessDirectoryTableBase\n", KdDebuggerDataBlock->OffsetEprocessDirectoryTableBase);
        // DbgPrint("[Shark] < %p > SizePrcb\n", KdDebuggerDataBlock->SizePrcb);
        // DbgPrint("[Shark] < %p > OffsetPrcbDpcRoutine\n", KdDebuggerDataBlock->OffsetPrcbDpcRoutine);
        // DbgPrint("[Shark] < %p > OffsetPrcbCurrentThread\n", KdDebuggerDataBlock->OffsetPrcbCurrentThread);
        // DbgPrint("[Shark] < %p > OffsetPrcbMhz\n", KdDebuggerDataBlock->OffsetPrcbMhz);
        // DbgPrint("[Shark] < %p > OffsetPrcbCpuType\n", KdDebuggerDataBlock->OffsetPrcbCpuType);
        // DbgPrint("[Shark] < %p > OffsetPrcbVendorString\n", KdDebuggerDataBlock->OffsetPrcbVendorString);
        // DbgPrint("[Shark] < %p > OffsetPrcbProcStateContext\n", KdDebuggerDataBlock->OffsetPrcbProcStateContext);
        // DbgPrint("[Shark] < %p > OffsetPrcbNumber\n", KdDebuggerDataBlock->OffsetPrcbNumber);
        // DbgPrint("[Shark] < %p > SizeEThread\n", KdDebuggerDataBlock->SizeEThread);
        /// DbgPrint("[Shark] < %p > KdPrintCircularBufferPtr\n", KdDebuggerDataBlock->KdPrintCircularBufferPtr);
        /// DbgPrint("[Shark] < %p > KdPrintBufferSize\n", KdDebuggerDataBlock->KdPrintBufferSize);
        /// DbgPrint("[Shark] < %p > KeLoaderBlock\n", KdDebuggerDataBlock->KeLoaderBlock);
        // DbgPrint("[Shark] < %p > SizePcr\n", KdDebuggerDataBlock->SizePcr);
        // DbgPrint("[Shark] < %p > OffsetPcrSelfPcr\n", KdDebuggerDataBlock->OffsetPcrSelfPcr);
        // DbgPrint("[Shark] < %p > OffsetPcrCurrentPrcb\n", KdDebuggerDataBlock->OffsetPcrCurrentPrcb);
        // DbgPrint("[Shark] < %p > OffsetPcrContainedPrcb\n", KdDebuggerDataBlock->OffsetPcrContainedPrcb);
        // DbgPrint("[Shark] < %p > OffsetPcrInitialBStore\n", KdDebuggerDataBlock->OffsetPcrInitialBStore);
        // DbgPrint("[Shark] < %p > OffsetPcrBStoreLimit\n", KdDebuggerDataBlock->OffsetPcrBStoreLimit);
        // DbgPrint("[Shark] < %p > OffsetPcrInitialStack\n", KdDebuggerDataBlock->OffsetPcrInitialStack);
        // DbgPrint("[Shark] < %p > OffsetPcrStackLimit\n", KdDebuggerDataBlock->OffsetPcrStackLimit);
        // DbgPrint("[Shark] < %p > OffsetPrcbPcrPage\n", KdDebuggerDataBlock->OffsetPrcbPcrPage);
        // DbgPrint("[Shark] < %p > OffsetPrcbProcStateSpecialReg\n", KdDebuggerDataBlock->OffsetPrcbProcStateSpecialReg);
        // DbgPrint("[Shark] < %p > GdtR0Code\n", KdDebuggerDataBlock->GdtR0Code);
        // DbgPrint("[Shark] < %p > GdtR0Data\n", KdDebuggerDataBlock->GdtR0Data);
        // DbgPrint("[Shark] < %p > GdtR0Pcr\n", KdDebuggerDataBlock->GdtR0Pcr);
        // DbgPrint("[Shark] < %p > GdtR3Code\n", KdDebuggerDataBlock->GdtR3Code);
        // DbgPrint("[Shark] < %p > GdtR3Data\n", KdDebuggerDataBlock->GdtR3Data);
        // DbgPrint("[Shark] < %p > GdtR3Teb\n", KdDebuggerDataBlock->GdtR3Teb);
        // DbgPrint("[Shark] < %p > GdtLdt\n", KdDebuggerDataBlock->GdtLdt);
        // DbgPrint("[Shark] < %p > GdtTss\n", KdDebuggerDataBlock->GdtTss);
        // DbgPrint("[Shark] < %p > Gdt64R3CmCode\n", KdDebuggerDataBlock->Gdt64R3CmCode);
        // DbgPrint("[Shark] < %p > Gdt64R3CmTeb\n", KdDebuggerDataBlock->Gdt64R3CmTeb);
        /// DbgPrint("[Shark] < %p > IopNumTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopNumTriageDumpDataBlocks);
        /// DbgPrint("[Shark] < %p > IopTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopTriageDumpDataBlocks);

        if (ReloaderBlock->BuildNumber >= 10586) {
            // DbgPrint("[Shark] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
        }
#endif // !PUBLIC

        ExFreePool(DumpHeader);
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
            Block->ServiceDescriptorTableShadow = __RVA_TO_VA(ControlPc + 30);
        }
    }
#endif // !_WIN64

    InitializeListHead(&Block->LoadedPrivateImageList);

    if (NULL != Block->CoreDataTableEntry) {
        CaptureImageExceptionValues(
            Block->CoreDataTableEntry->DllBase,
            &Block->CoreDataTableEntry->ExceptionTable,
            &Block->CoreDataTableEntry->ExceptionTableSize);

        /*
#ifdef _WIN64
        InsertInvertedFunctionTable(
            Block->CoreDataTableEntry->DllBase,
            Block->CoreDataTableEntry->SizeOfImage);
#endif // _WIN64
        */

        InsertTailList(
            &Block->LoadedPrivateImageList,
            &Block->CoreDataTableEntry->InLoadOrderLinks);

        Block->CoreDataTableEntry->LoadCount++;
    }
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

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount--;
        }
        else {
            Status = FindEntryForKernelImage(
                &ImageFileName,
                &DataTableEntry);

            if (NT_SUCCESS(Status)) {
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
                                "[Shark] import procedure ordinal@%d not found\n",
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
                                "[Shark] import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);
            }
            else {
                DbgPrint(
                    "[Shark] import dll %hs not found\n",
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
    ULONG SizeToLock = 0;

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
                    SizeToLock = max(
                        NtSection[Index].SizeOfRawData,
                        NtSection[Index].Misc.VirtualSize);

                    RtlCopyMemory(
                        (PCHAR)ImageBase + NtSection[Index].VirtualAddress,
                        (PCHAR)ViewBase + NtSection[Index].PointerToRawData,
                        SizeToLock);
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
                CaptureImageExceptionValues(
                    DataTableEntry->DllBase,
                    &DataTableEntry->ExceptionTable,
                    &DataTableEntry->ExceptionTableSize);

                /*
#ifdef _WIN64
                InsertInvertedFunctionTable(
                    DataTableEntry->DllBase,
                    DataTableEntry->SizeOfImage);
#endif // _WIN64
                */

                InsertTailList(
                    &ReloaderBlock->LoadedPrivateImageList,
                    &DataTableEntry->InLoadOrderLinks);

                DataTableEntry->LoadCount++;
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
    DataTableEntry->LoadCount--;

    if (0 == DataTableEntry->LoadCount) {
        DereferenceKernelImageImports(DataTableEntry->DllBase);
        RemoveEntryList(&DataTableEntry->InLoadOrderLinks);

        ExFreePool(DataTableEntry);
    }
}

NTSTATUS
NTAPI
TouchMemory(
    __in PVOID Address,
    __in ULONG Length
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCHAR RegionStart = NULL;
    PCHAR RegionEnd = NULL;

    RegionStart = Address;
    RegionEnd = RegionStart + Length;

    __try {
        while (RegionStart < RegionEnd) {
            *(volatile UCHAR *)RegionStart;
            RegionStart = PAGE_ALIGN(RegionStart + PAGE_SIZE);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}

NTSTATUS
NTAPI
DumpImage(
    __in PDUMP_WORKER DumpWorker
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE FileHandle = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    SIZE_T Index = 0;
    ULONG SizeToLock = 0;
    GUID Guid = { 0 };
    WCHAR FilePathBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    LARGE_INTEGER ByteOffset = { 0 };

    if (0 != DumpWorker->Interval.QuadPart) {
        TRACE(KeDelayExecutionThread(KernelMode, FALSE, &DumpWorker->Interval));
    }

    Status = TouchMemory(DumpWorker->ImageBase, DumpWorker->ImageSize);

    if (TRACE(Status)) {
        NtHeaders = RtlImageNtHeader(DumpWorker->ImageBase);

        if (NULL != NtHeaders) {
            Status = ExUuidCreate(&Guid);

            if (TRACE(Status)) {
                swprintf(
                    (PCHAR)FilePathBuffer,
                    L"\\SystemRoot\\%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x.bin",
                    Guid.Data1,
                    Guid.Data2,
                    Guid.Data3,
                    Guid.Data4[0],
                    Guid.Data4[1],
                    Guid.Data4[2],
                    Guid.Data4[3],
                    Guid.Data4[4],
                    Guid.Data4[5],
                    Guid.Data4[6],
                    Guid.Data4[7]);

                RtlInitUnicodeString(&FilePath, FilePathBuffer);

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &FilePath,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

                Status = ZwCreateFile(
                    &FileHandle,
                    FILE_ALL_ACCESS,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OVERWRITE_IF,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                    NULL,
                    0);

                if (TRACE(Status)) {
                    NtSection = IMAGE_FIRST_SECTION(NtHeaders);

                    ByteOffset.QuadPart = 0;

                    TRACE(ZwWriteFile(
                        FileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        DumpWorker->ImageBase,
                        NtSection->VirtualAddress,
                        &ByteOffset,
                        NULL));

                    for (Index = 0;
                        Index < NtHeaders->FileHeader.NumberOfSections;
                        Index++) {
                        if (0 != NtSection[Index].VirtualAddress) {
                            ByteOffset.QuadPart = NtSection[Index].PointerToRawData;

                            SizeToLock = max(
                                NtSection[Index].SizeOfRawData,
                                NtSection[Index].Misc.VirtualSize);

                            TRACE(ZwWriteFile(
                                FileHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                (PCHAR)DumpWorker->ImageBase + NtSection[Index].VirtualAddress,
                                SizeToLock,
                                &ByteOffset,
                                NULL));
                        }
                    }

                    TRACE(ZwFlushBuffersFile(FileHandle, &IoStatusBlock));
                    TRACE(ZwClose(FileHandle));

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] dumped < %p - %08x > %wZ\n",
                        DumpWorker->ImageBase,
                        DumpWorker->ImageSize,
                        &FilePath);
#endif // !PUBLIC
                }
            }
        }
        else {
            Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
        }
    }

    ExFreePool(DumpWorker);

    return Status;
}

NTSTATUS
NTAPI
DumpFile(
    __in PUNICODE_STRING ImageFIleName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE SourceFileHandle = NULL;
    HANDLE DestinationFileHandle = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    FILE_STANDARD_INFORMATION StandardInformation = { 0 };
    LARGE_INTEGER ByteOffset = { 0 };
    PVOID Buffer = NULL;
    GUID Guid = { 0 };
    WCHAR FilePathBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };

    InitializeObjectAttributes(
        &ObjectAttributes,
        ImageFIleName,
        (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
        NULL,
        NULL);

    Status = ZwOpenFile(
        &SourceFileHandle,
        FILE_EXECUTE | FILE_READ_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        0);

    if (TRACE(Status)) {
        Status = ZwQueryInformationFile(
            SourceFileHandle,
            &IoStatusBlock,
            &StandardInformation,
            sizeof(FILE_STANDARD_INFORMATION),
            FileStandardInformation);

        if (TRACE(Status)) {
            Buffer = ExAllocatePool(
                PagedPool,
                StandardInformation.EndOfFile.LowPart);

            if (NULL != Buffer) {
                ByteOffset.QuadPart = 0;

                Status = ZwReadFile(
                    SourceFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    Buffer,
                    StandardInformation.EndOfFile.LowPart,
                    &ByteOffset,
                    NULL);

                if (TRACE(Status)) {
                    Status = ExUuidCreate(&Guid);

                    if (TRACE(Status)) {
                        swprintf(
                            (PCHAR)FilePathBuffer,
                            L"\\SystemRoot\\%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x.bin",
                            Guid.Data1,
                            Guid.Data2,
                            Guid.Data3,
                            Guid.Data4[0],
                            Guid.Data4[1],
                            Guid.Data4[2],
                            Guid.Data4[3],
                            Guid.Data4[4],
                            Guid.Data4[5],
                            Guid.Data4[6],
                            Guid.Data4[7]);

                        RtlInitUnicodeString(&FilePath, FilePathBuffer);

                        InitializeObjectAttributes(
                            &ObjectAttributes,
                            &FilePath,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL);

                        Status = ZwCreateFile(
                            &DestinationFileHandle,
                            FILE_ALL_ACCESS,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ,
                            FILE_OVERWRITE_IF,
                            FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                            NULL,
                            0);

                        if (TRACE(Status)) {
                            ByteOffset.QuadPart = 0;

                            TRACE(ZwWriteFile(
                                DestinationFileHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                Buffer,
                                StandardInformation.EndOfFile.LowPart,
                                &ByteOffset,
                                NULL));

                            TRACE(ZwFlushBuffersFile(DestinationFileHandle, &IoStatusBlock));
                            TRACE(ZwClose(DestinationFileHandle));

#ifndef PUBLIC
                            DbgPrint(
                                "[Shark] dumped %wZ to %wZ\n",
                                ImageFIleName,
                                &FilePath);
#endif // !PUBLIC
                        }
                    }
                }

                ExFreePool(Buffer);
            }
        }

        ZwClose(SourceFileHandle);
    }

    return Status;
}
