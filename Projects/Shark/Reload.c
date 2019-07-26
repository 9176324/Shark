/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#include "Ctx.h"
#include "Detours.h"
#include "Except.h"
#include "Scan.h"

PGPBLOCK GpBlock;

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

VOID
NTAPI
InitializeGpBlock(
    __in PGPBLOCK Block
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING KernelString = { 0 };
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    ULONG SizeToLock = 0;
    PCHAR ControlPc = NULL;
    CONTEXT Context = { 0 };
    PDUMP_HEADER DumpHeader = NULL;
    PKDDEBUGGER_DATA64 KdDebuggerDataBlock = NULL;
    PKDDEBUGGER_DATA_ADDITION64 KdDebuggerDataAdditionBlock = NULL;
    UNICODE_STRING RoutineString = { 0 };
    PVOID RoutineAddress = NULL;

#ifndef _WIN64
    // 6A 01                            push 1
    // 68 A0 D7 69 00                   push offset _PsLoadedModuleResource
    // E8 FB E4 E9 FF                   call _ExAcquireResourceSharedLite@8

    CHAR PsLoadedModuleResource[] = "6a 01 68 ?? ?? ?? ?? e8 ?? ?? ?? ??";
#else
    // 48 89 A3 D8 01 00 00             mov [rbx + 1D8h], rsp
    // 8B F8                            mov edi, eax
    // C1 EF 07                         shr edi, 7
    // 83 E7 20                         and edi, 20h
    // 25 FF 0F 00 00                   and eax, 0FFFh
    // 4C 8D 15 C7 20 23 00             lea r10, KeServiceDescriptorTable
    // 4C 8D 1D 00 21 23 00             lea r11, KeServiceDescriptorTableShadow

    CHAR KiSystemCall64[] =
        "48 89 a3 ?? ?? ?? ?? 8b f8 c1 ef 07 83 e7 20 25 ff 0f 00 00 4c 8d 15 ?? ?? ?? ?? 4c 8d 1d ?? ?? ?? ??";

    // 48 8D 0D FD DA 19 00             rcx, PsLoadedModuleResource
    // E8 B8 B8 E3 FF call              ExReleaseResourceLite

    CHAR PsLoadedModuleResource[] = "48 8d 0d ?? ?? ?? ?? e8 ?? ?? ?? ??";
#endif // !_WIN64

    GpBlock->Linkage[0] = 0x33;
    GpBlock->Linkage[1] = 0xc0;
    GpBlock->Linkage[2] = 0xc3;

    PsGetVersion(NULL, NULL, &GpBlock->BuildNumber, NULL);

    RtlInitUnicodeString(&RoutineString, L"PsInitialSystemProcess");

    RoutineAddress = MmGetSystemRoutineAddress(&RoutineString);

    RtlCopyMemory(
        &GpBlock->PsInitialSystemProcess,
        RoutineAddress,
        sizeof(PVOID));

    RtlInitUnicodeString(&RoutineString, L"KeNumberProcessors");

    RoutineAddress = MmGetSystemRoutineAddress(&RoutineString);

    RtlCopyMemory(
        &GpBlock->NumberProcessors,
        RoutineAddress,
        sizeof(CCHAR));

    RtlInitUnicodeString(&RoutineString, L"KeEnterCriticalRegion");

    GpBlock->KeEnterCriticalRegion = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"KeLeaveCriticalRegion");

    GpBlock->KeLeaveCriticalRegion = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"ExAcquireSpinLockShared");

    GpBlock->ExAcquireSpinLockShared = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"ExReleaseSpinLockShared");

    GpBlock->ExReleaseSpinLockShared = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"DbgPrint");

    GpBlock->DbgPrint = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"RtlCompareMemory");

    GpBlock->RtlCompareMemory = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"RtlRestoreContext");

    GpBlock->RtlRestoreContext = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"ExQueueWorkItem");

    GpBlock->ExQueueWorkItem = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"ExFreePoolWithTag");

    GpBlock->ExFreePoolWithTag = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"KeBugCheckEx");

    GpBlock->KeBugCheckEx = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"KdDebuggerEnabled");

    GpBlock->KdDebuggerEnabled = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"KdDebuggerNotPresent");

    GpBlock->KdDebuggerNotPresent = MmGetSystemRoutineAddress(&RoutineString);

    RtlInitUnicodeString(&RoutineString, L"KdEnteredDebugger");

    GpBlock->KdEnteredDebugger = MmGetSystemRoutineAddress(&RoutineString);

    GpBlock->CaptureContext = (PVOID)GpBlock->_CaptureContext;

    RtlCopyMemory(
        GpBlock->_CaptureContext,
        _CaptureContext,
        sizeof(GpBlock->_CaptureContext));

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
            &GpBlock->DebuggerDataBlock,
            KdDebuggerDataBlock,
            sizeof(KDDEBUGGER_DATA64));

        KdDebuggerDataAdditionBlock = (PKDDEBUGGER_DATA_ADDITION64)(KdDebuggerDataBlock + 1);

        RtlCopyMemory(
            &GpBlock->DebuggerDataAdditionBlock,
            KdDebuggerDataAdditionBlock,
            sizeof(KDDEBUGGER_DATA_ADDITION64));

        GpBlock->PsLoadedModuleList =
            (PLIST_ENTRY)GpBlock->DebuggerDataBlock.PsLoadedModuleList;

        GpBlock->KernelDataTableEntry = CONTAINING_RECORD(
            GpBlock->PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

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

        if (GpBlock->BuildNumber >= 10586) {
            // DbgPrint("[Shark] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
        }
#endif // !PUBLIC

        ExFreePool(DumpHeader);
    }

#ifndef _WIN64
    GpBlock->OffsetKProcessThreadListHead = 0x2c;

    if (Block->BuildNumber < 9200) {
        GpBlock->OffsetKThreadThreadListEntry = 0x1e0;
    }
    else {
        GpBlock->OffsetKThreadThreadListEntry = 0x1d4;
    }
#else
    GpBlock->OffsetKProcessThreadListHead = 0x30;
    GpBlock->OffsetKThreadThreadListEntry = 0x2f8;
#endif // !_WIN64

#ifndef _WIN64
    RtlInitUnicodeString(&RoutineString, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&RoutineString);

    ControlPc = ScanBytes(
        ControlPc,
        ControlPc + PAGE_SIZE,
        PsLoadedModuleResource);

    if (NULL != ControlPc) {
        Block->PsLoadedModuleResource = *(PERESOURCE *)(ControlPc + 3);
    }

    RtlInitUnicodeString(&RoutineString, L"KeServiceDescriptorTable");

    Block->KeServiceDescriptorTable = MmGetSystemRoutineAddress(&RoutineString);
#else
    RtlInitUnicodeString(&RoutineString, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&RoutineString);

    ControlPc = ScanBytes(
        ControlPc,
        ControlPc + PAGE_SIZE,
        PsLoadedModuleResource);

    if (NULL != ControlPc) {
        Block->PsLoadedModuleResource = (PERESOURCE)RvaToVa(ControlPc + 3);
    }

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
            Block->KeServiceDescriptorTable = RvaToVa(ControlPc + 23);
            Block->KeServiceDescriptorTableShadow = RvaToVa(ControlPc + 30);
        }
    }
#endif // !_WIN64

    InitializeListHead(&Block->LoadedPrivateImageList);
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

    GpBlock->KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(GpBlock->PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(GpBlock->PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            GpBlock->PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)GpBlock->PsLoadedModuleList) {
            if (FALSE != RtlEqualUnicodeString(
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

    ExReleaseResourceLite(GpBlock->PsLoadedModuleResource);
    GpBlock->KeLeaveCriticalRegion();

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

    GpBlock->KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(GpBlock->PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(GpBlock->PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            (GpBlock->PsLoadedModuleList)->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)GpBlock->PsLoadedModuleList) {
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

    ExReleaseResourceLite(GpBlock->PsLoadedModuleResource);
    GpBlock->KeLeaveCriticalRegion();

    return Status;
}
