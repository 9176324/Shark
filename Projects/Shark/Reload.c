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

#include "Detour.h"
#include "Except.h"
#include "Scan.h"

PGPBLOCK GpBlock;

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

    ULONG64 CaptureContext[] = {
        0x8d53000002d0ec81, 0x0000a4838f04245c, 0x8c00000094838c00, 0xc8938c000000bc8b,
        0x0000989b8c000000, 0x8c00000090a38c00, 0xb083890000008cab, 0x0000ac8b89000000,
        0x89000000a8938900, 0xa0b389000000b4ab, 0x00009cbb89000000, 0x000000c0838f9c00,
        0x8389000002dc838d, 0x02d4838b000000c4, 0x000000b883890000, 0x8b038900010007b8,
        0x8b0b8d000002d093, 0xff5152000002d883, 0xccccccccccccccd0
    };
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

    ULONG64 CaptureContext[] = {
        0x51000004d0ec8148, 0x80818f08244c8d48, 0x598c38498c000000, 0x8c42518c3c418c3a,
        0x41894840698c3e61, 0x0000008891894878, 0x4800000090998948, 0x8948000004e8818d,
        0xa989480000009881, 0xa8b18948000000a0, 0x00b0b98948000000, 0x0000b881894c0000,
        0x000000c089894c00, 0x4c000000c891894c, 0x894c000000d09989, 0xa9894c000000d8a1,
        0xe8b1894c000000e0, 0x00f0b9894c000000, 0x00010081ae0f0000, 0x0001a0817f0f6600,
        0x0001b0897f0f6600, 0x0001c0917f0f6600, 0x0001d0997f0f6600, 0x0001e0a17f0f6600,
        0x0001f0a97f0f6600, 0x000200b17f0f6600, 0x000210b97f0f6600, 0x0220817f0f446600,
        0x30897f0f44660000, 0x917f0f4466000002, 0x7f0f446600000240, 0x0f44660000025099,
        0x446600000260a17f, 0x6600000270a97f0f, 0x00000280b17f0f44, 0x000290b97f0f4466,
        0x418f9c3459ae0f00, 0x000004d8818b4844, 0xb8000000f8818948, 0x483041890010000b,
        0x8d48000004d0918b, 0x000004e0818b4809, 0xccccccccccccd0ff
    };
#endif // !_WIN64

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

    GpBlock->CaptureContext = (PVOID)GpBlock->_CaptureContext;

    RtlCopyMemory(
        GpBlock->_CaptureContext,
        CaptureContext,
        sizeof(CaptureContext));

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
        /// DbgPrint("[Shark] [Tiferet] < %p > Header\n", KdDebuggerDataBlock->Header);
        //DbgPrint("[Shark] [Tiferet] < %p > KernBase\n", KdDebuggerDataBlock->KernBase);
        /// DbgPrint("[Shark] [Tiferet] < %p > BreakpointWithStatus\n", KdDebuggerDataBlock->BreakpointWithStatus);
        /// DbgPrint("[Shark] [Tiferet] < %p > SavedContext\n", KdDebuggerDataBlock->SavedContext);
        /// DbgPrint("[Shark] [Tiferet] < %p > ThCallbackStack\n", KdDebuggerDataBlock->ThCallbackStack);
        /// DbgPrint("[Shark] [Tiferet] < %p > NextCallback\n", KdDebuggerDataBlock->NextCallback);
        /// DbgPrint("[Shark] [Tiferet] < %p > FramePointer\n", KdDebuggerDataBlock->FramePointer);
        // DbgPrint("[Shark] [Tiferet] < %p > PaeEnabled\n", KdDebuggerDataBlock->PaeEnabled);
        /// DbgPrint("[Shark] [Tiferet] < %p > KiCallUserMode\n", KdDebuggerDataBlock->KiCallUserMode);
        /// DbgPrint("[Shark] [Tiferet] < %p > KeUserCallbackDispatcher\n", KdDebuggerDataBlock->KeUserCallbackDispatcher);
        // DbgPrint("[Shark] [Tiferet] < %p > PsLoadedModuleList\n", KdDebuggerDataBlock->PsLoadedModuleList);
        // DbgPrint("[Shark] [Tiferet] < %p > PsActiveProcessHead\n", KdDebuggerDataBlock->PsActiveProcessHead);
        // DbgPrint("[Shark] [Tiferet] < %p > PspCidTable\n", KdDebuggerDataBlock->PspCidTable);
        /// DbgPrint("[Shark] [Tiferet] < %p > ExpSystemResourcesList\n", KdDebuggerDataBlock->ExpSystemResourcesList);
        /// DbgPrint("[Shark] [Tiferet] < %p > ExpPagedPoolDescriptor\n", KdDebuggerDataBlock->ExpPagedPoolDescriptor);
        /// DbgPrint("[Shark] [Tiferet] < %p > ExpNumberOfPagedPools\n", KdDebuggerDataBlock->ExpNumberOfPagedPools);
        /// DbgPrint("[Shark] [Tiferet] < %p > KeTimeIncrement\n", KdDebuggerDataBlock->KeTimeIncrement);
        /// DbgPrint("[Shark] [Tiferet] < %p > KeBugCheckCallbackListHead\n", KdDebuggerDataBlock->KeBugCheckCallbackListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > KiBugcheckData\n", KdDebuggerDataBlock->KiBugcheckData);
        /// DbgPrint("[Shark] [Tiferet] < %p > IopErrorLogListHead\n", KdDebuggerDataBlock->IopErrorLogListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > ObpRootDirectoryObject\n", KdDebuggerDataBlock->ObpRootDirectoryObject);
        /// DbgPrint("[Shark] [Tiferet] < %p > ObpTypeObjectType\n", KdDebuggerDataBlock->ObpTypeObjectType);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemCacheStart\n", KdDebuggerDataBlock->MmSystemCacheStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemCacheEnd\n", KdDebuggerDataBlock->MmSystemCacheEnd);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemCacheWs\n", KdDebuggerDataBlock->MmSystemCacheWs);
        // DbgPrint("[Shark] [Tiferet] < %p > MmPfnDatabase\n", KdDebuggerDataBlock->MmPfnDatabase);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemPtesStart\n", KdDebuggerDataBlock->MmSystemPtesStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemPtesEnd\n", KdDebuggerDataBlock->MmSystemPtesEnd);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSubsectionBase\n", KdDebuggerDataBlock->MmSubsectionBase);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmNumberOfPagingFiles\n", KdDebuggerDataBlock->MmNumberOfPagingFiles);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmLowestPhysicalPage\n", KdDebuggerDataBlock->MmLowestPhysicalPage);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmHighestPhysicalPage\n", KdDebuggerDataBlock->MmHighestPhysicalPage);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmNumberOfPhysicalPages\n", KdDebuggerDataBlock->MmNumberOfPhysicalPages);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmMaximumNonPagedPoolInBytes\n", KdDebuggerDataBlock->MmMaximumNonPagedPoolInBytes);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmNonPagedSystemStart\n", KdDebuggerDataBlock->MmNonPagedSystemStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmNonPagedPoolStart\n", KdDebuggerDataBlock->MmNonPagedPoolStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmNonPagedPoolEnd\n", KdDebuggerDataBlock->MmNonPagedPoolEnd);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPagedPoolStart\n", KdDebuggerDataBlock->MmPagedPoolStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPagedPoolEnd\n", KdDebuggerDataBlock->MmPagedPoolEnd);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPagedPoolInformation\n", KdDebuggerDataBlock->MmPagedPoolInformation);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPageSize\n", KdDebuggerDataBlock->MmPageSize);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSizeOfPagedPoolInBytes\n", KdDebuggerDataBlock->MmSizeOfPagedPoolInBytes);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmTotalCommitLimit\n", KdDebuggerDataBlock->MmTotalCommitLimit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmTotalCommittedPages\n", KdDebuggerDataBlock->MmTotalCommittedPages);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSharedCommit\n", KdDebuggerDataBlock->MmSharedCommit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmDriverCommit\n", KdDebuggerDataBlock->MmDriverCommit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmProcessCommit\n", KdDebuggerDataBlock->MmProcessCommit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPagedPoolCommit\n", KdDebuggerDataBlock->MmPagedPoolCommit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmExtendedCommit\n", KdDebuggerDataBlock->MmExtendedCommit);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmZeroedPageListHead\n", KdDebuggerDataBlock->MmZeroedPageListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmFreePageListHead\n", KdDebuggerDataBlock->MmFreePageListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmStandbyPageListHead\n", KdDebuggerDataBlock->MmStandbyPageListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmModifiedPageListHead\n", KdDebuggerDataBlock->MmModifiedPageListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmModifiedNoWritePageListHead\n", KdDebuggerDataBlock->MmModifiedNoWritePageListHead);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmAvailablePages\n", KdDebuggerDataBlock->MmAvailablePages);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmResidentAvailablePages\n", KdDebuggerDataBlock->MmResidentAvailablePages);
        /// DbgPrint("[Shark] [Tiferet] < %p > PoolTrackTable\n", KdDebuggerDataBlock->PoolTrackTable);
        /// DbgPrint("[Shark] [Tiferet] < %p > NonPagedPoolDescriptor\n", KdDebuggerDataBlock->NonPagedPoolDescriptor);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmHighestUserAddress\n", KdDebuggerDataBlock->MmHighestUserAddress);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemRangeStart\n", KdDebuggerDataBlock->MmSystemRangeStart);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmUserProbeAddress\n", KdDebuggerDataBlock->MmUserProbeAddress);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintCircularBuffer\n", KdDebuggerDataBlock->KdPrintCircularBuffer);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintCircularBufferEnd\n", KdDebuggerDataBlock->KdPrintCircularBufferEnd);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintWritePointer\n", KdDebuggerDataBlock->KdPrintWritePointer);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintRolloverCount\n", KdDebuggerDataBlock->KdPrintRolloverCount);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmLoadedUserImageList\n", KdDebuggerDataBlock->MmLoadedUserImageList);
        /// DbgPrint("[Shark] [Tiferet] < %p > NtBuildLab\n", KdDebuggerDataBlock->NtBuildLab);
        /// DbgPrint("[Shark] [Tiferet] < %p > KiNormalSystemCall\n", KdDebuggerDataBlock->KiNormalSystemCall);
        /// DbgPrint("[Shark] [Tiferet] < %p > KiProcessorBlock\n", KdDebuggerDataBlock->KiProcessorBlock);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmUnloadedDrivers\n", KdDebuggerDataBlock->MmUnloadedDrivers);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmLastUnloadedDriver\n", KdDebuggerDataBlock->MmLastUnloadedDriver);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmTriageActionTaken\n", KdDebuggerDataBlock->MmTriageActionTaken);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSpecialPoolTag\n", KdDebuggerDataBlock->MmSpecialPoolTag);
        /// DbgPrint("[Shark] [Tiferet] < %p > KernelVerifier\n", KdDebuggerDataBlock->KernelVerifier);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmVerifierData\n", KdDebuggerDataBlock->MmVerifierData);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmAllocatedNonPagedPool\n", KdDebuggerDataBlock->MmAllocatedNonPagedPool);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPeakCommitment\n", KdDebuggerDataBlock->MmPeakCommitment);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmTotalCommitLimitMaximum\n", KdDebuggerDataBlock->MmTotalCommitLimitMaximum);
        /// DbgPrint("[Shark] [Tiferet] < %p > CmNtCSDVersion\n", KdDebuggerDataBlock->CmNtCSDVersion);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmPhysicalMemoryBlock\n", KdDebuggerDataBlock->MmPhysicalMemoryBlock);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSessionBase\n", KdDebuggerDataBlock->MmSessionBase);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSessionSize\n", KdDebuggerDataBlock->MmSessionSize);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmSystemParentTablePage\n", KdDebuggerDataBlock->MmSystemParentTablePage);
        /// DbgPrint("[Shark] [Tiferet] < %p > MmVirtualTranslationBase\n", KdDebuggerDataBlock->MmVirtualTranslationBase);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadNextProcessor\n", KdDebuggerDataBlock->OffsetKThreadNextProcessor);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadTeb\n", KdDebuggerDataBlock->OffsetKThreadTeb);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadKernelStack\n", KdDebuggerDataBlock->OffsetKThreadKernelStack);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadInitialStack\n", KdDebuggerDataBlock->OffsetKThreadInitialStack);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadApcProcess\n", KdDebuggerDataBlock->OffsetKThreadApcProcess);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadState\n", KdDebuggerDataBlock->OffsetKThreadState);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadBStore\n", KdDebuggerDataBlock->OffsetKThreadBStore);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetKThreadBStoreLimit\n", KdDebuggerDataBlock->OffsetKThreadBStoreLimit);
        // DbgPrint("[Shark] [Tiferet] < %p > SizeEProcess\n", KdDebuggerDataBlock->SizeEProcess);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetEprocessPeb\n", KdDebuggerDataBlock->OffsetEprocessPeb);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetEprocessParentCID\n", KdDebuggerDataBlock->OffsetEprocessParentCID);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetEprocessDirectoryTableBase\n", KdDebuggerDataBlock->OffsetEprocessDirectoryTableBase);
        // DbgPrint("[Shark] [Tiferet] < %p > SizePrcb\n", KdDebuggerDataBlock->SizePrcb);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbDpcRoutine\n", KdDebuggerDataBlock->OffsetPrcbDpcRoutine);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbCurrentThread\n", KdDebuggerDataBlock->OffsetPrcbCurrentThread);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbMhz\n", KdDebuggerDataBlock->OffsetPrcbMhz);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbCpuType\n", KdDebuggerDataBlock->OffsetPrcbCpuType);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbVendorString\n", KdDebuggerDataBlock->OffsetPrcbVendorString);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbProcStateContext\n", KdDebuggerDataBlock->OffsetPrcbProcStateContext);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbNumber\n", KdDebuggerDataBlock->OffsetPrcbNumber);
        // DbgPrint("[Shark] [Tiferet] < %p > SizeEThread\n", KdDebuggerDataBlock->SizeEThread);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintCircularBufferPtr\n", KdDebuggerDataBlock->KdPrintCircularBufferPtr);
        /// DbgPrint("[Shark] [Tiferet] < %p > KdPrintBufferSize\n", KdDebuggerDataBlock->KdPrintBufferSize);
        /// DbgPrint("[Shark] [Tiferet] < %p > KeLoaderBlock\n", KdDebuggerDataBlock->KeLoaderBlock);
        // DbgPrint("[Shark] [Tiferet] < %p > SizePcr\n", KdDebuggerDataBlock->SizePcr);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrSelfPcr\n", KdDebuggerDataBlock->OffsetPcrSelfPcr);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrCurrentPrcb\n", KdDebuggerDataBlock->OffsetPcrCurrentPrcb);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrContainedPrcb\n", KdDebuggerDataBlock->OffsetPcrContainedPrcb);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrInitialBStore\n", KdDebuggerDataBlock->OffsetPcrInitialBStore);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrBStoreLimit\n", KdDebuggerDataBlock->OffsetPcrBStoreLimit);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrInitialStack\n", KdDebuggerDataBlock->OffsetPcrInitialStack);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPcrStackLimit\n", KdDebuggerDataBlock->OffsetPcrStackLimit);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbPcrPage\n", KdDebuggerDataBlock->OffsetPrcbPcrPage);
        // DbgPrint("[Shark] [Tiferet] < %p > OffsetPrcbProcStateSpecialReg\n", KdDebuggerDataBlock->OffsetPrcbProcStateSpecialReg);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR0Code\n", KdDebuggerDataBlock->GdtR0Code);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR0Data\n", KdDebuggerDataBlock->GdtR0Data);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR0Pcr\n", KdDebuggerDataBlock->GdtR0Pcr);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR3Code\n", KdDebuggerDataBlock->GdtR3Code);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR3Data\n", KdDebuggerDataBlock->GdtR3Data);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtR3Teb\n", KdDebuggerDataBlock->GdtR3Teb);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtLdt\n", KdDebuggerDataBlock->GdtLdt);
        // DbgPrint("[Shark] [Tiferet] < %p > GdtTss\n", KdDebuggerDataBlock->GdtTss);
        // DbgPrint("[Shark] [Tiferet] < %p > Gdt64R3CmCode\n", KdDebuggerDataBlock->Gdt64R3CmCode);
        // DbgPrint("[Shark] [Tiferet] < %p > Gdt64R3CmTeb\n", KdDebuggerDataBlock->Gdt64R3CmTeb);
        /// DbgPrint("[Shark] [Tiferet] < %p > IopNumTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopNumTriageDumpDataBlocks);
        /// DbgPrint("[Shark] [Tiferet] < %p > IopTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopTriageDumpDataBlocks);

        if (GpBlock->BuildNumber >= 10586) {
            // DbgPrint("[Shark] [Tiferet] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
        }
#endif // !PUBLIC

        ExFreePool(DumpHeader);
    }

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
        Block->PsLoadedModuleResource = (PERESOURCE)__RVA_TO_VA(ControlPc + 3);
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
            Block->KeServiceDescriptorTable = __RVA_TO_VA(ControlPc + 23);
            Block->KeServiceDescriptorTableShadow = __RVA_TO_VA(ControlPc + 30);
}
    }
#endif // !_WIN64
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
