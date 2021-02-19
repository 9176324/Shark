/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
#include "Guard.h"
#include "Except.h"
#include "Scan.h"

void
NTAPI
InitializeGpBlock(
    __in PGPBLOCK Block
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING String = { 0 };
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    u32 SizeToLock = 0;
    u8ptr ControlPc = NULL;
    CONTEXT Context = { 0 };
    PDUMP_HEADER DumpHeader = NULL;
    PKDDEBUGGER_DATA64 KdDebuggerDataBlock = NULL;
    PKDDEBUGGER_DATA_ADDITION64 KdDebuggerDataAdditionBlock = NULL;
    ptr RoutineAddress = NULL;

#ifndef _WIN64
    // 6A 01                            push 1
    // 68 A0 D7 69 00                   push offset _PsLoadedModuleResource
    // E8 FB E4 E9 FF                   call _ExAcquireResourceSharedLite@8

    u8 PsLoadedModuleResource[] = "6A 01 68 ?? ?? ?? ?? E8";

    // 8B DA                            mov ebx, edx
    // F6 05 C8 E0 52 00 40             test byte ptr ds:dword_52E0C8, 40h
    // 0F 95 45 12                      setnz byte ptr [ebp + 12h]
    // 0F 85 8C 03 00 00                jnz loc_435C04
    // FF D3                            call ebx

    u8 PerfGlobalGroupMask[] =
        "8B DA F6 05 ?? ?? ?? ?? 40 0F 95 45 ?? 0F 85 ?? ?? ?? ?? FF D3";
#else
    // 48 89 A3 D8 01 00 00             mov [rbx + 1D8h], rsp
    // 8B F8                            mov edi, eax
    // C1 EF 07                         shr edi, 7
    // 83 E7 20                         and edi, 20h
    // 25 FF 0F 00 00                   and eax, 0FFFh
    // 4C 8D 15 C7 20 23 00             lea r10, KeServiceDescriptorTable
    // 4C 8D 1D 00 21 23 00             lea r11, KeServiceDescriptorTableShadow

    u8 KiSystemCall64[] =
        "48 89 A3 ?? ?? ?? ?? 8B F8 C1 EF 07 83 E7 20 25 FF 0F 00 00 4C 8D 15 ?? ?? ?? ?? 4C 8D 1D";

    // F7 05 3E C0 2D 00 40 00 00 00    test dword ptr cs:PerfGlobalGroupMask + 8, 40h
    // 0F 85 56 02 00 00                jnz loc_14007A2A6
    // 41 FF D2                         call r10

    u8 PerfGlobalGroupMask[] =
        "F7 05 ?? ?? ?? ?? 40 00 00 00 0F 85";

    // 48 8D 0D FD DA 19 00             rcx, PsLoadedModuleResource
    // E8 B8 B8 E3 FF call              ExReleaseResourceLite

    u8 PsLoadedModuleResource[] = "48 8D 0D ?? ?? ?? ?? E8";
#endif // !_WIN64

    Block->Linkage[0] = 0x33;
    Block->Linkage[1] = 0xc0;
    Block->Linkage[2] = 0xc3;

    PsGetVersion(NULL, NULL, &Block->BuildNumber, NULL);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > BuildNumber\n",
        Block->BuildNumber);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"PsInitialSystemProcess");

    RoutineAddress = MmGetSystemRoutineAddress(&String);

    RtlCopyMemory(
        &Block->PsInitialSystemProcess,
        RoutineAddress,
        sizeof(ptr));

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > PsInitialSystemProcess\n",
        Block->PsInitialSystemProcess);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"KeNumberProcessors");

    RoutineAddress = MmGetSystemRoutineAddress(&String);

    RtlCopyMemory(
        &Block->NumberProcessors,
        RoutineAddress,
        sizeof(u8));

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > NumberProcessors\n",
        Block->NumberProcessors);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"KeEnterCriticalRegion");

    Block->KeEnterCriticalRegion = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > KeEnterCriticalRegion\n",
        Block->KeEnterCriticalRegion);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"KeLeaveCriticalRegion");

    Block->KeLeaveCriticalRegion = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > KeLeaveCriticalRegion\n",
        Block->KeLeaveCriticalRegion);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExAcquireSpinLockShared");

    Block->ExAcquireSpinLockShared = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExAcquireSpinLockShared\n",
        Block->ExAcquireSpinLockShared);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExReleaseSpinLockShared");

    Block->ExReleaseSpinLockShared = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExReleaseSpinLockShared\n",
        Block->ExReleaseSpinLockShared);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"DbgPrint");

    Block->DbgPrint = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > DbgPrint\n",
        Block->DbgPrint);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"RtlCompareMemory");

    Block->RtlCompareMemory = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > RtlCompareMemory\n",
        Block->RtlCompareMemory);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"RtlRestoreContext");

    Block->RtlRestoreContext = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > RtlRestoreContext\n",
        Block->RtlRestoreContext);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExQueueWorkItem");

    Block->ExQueueWorkItem = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExQueueWorkItem\n",
        Block->ExQueueWorkItem);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExFreePoolWithTag");

    Block->ExFreePoolWithTag = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExFreePoolWithTag\n",
        Block->ExFreePoolWithTag);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"KeBugCheckEx");

    Block->KeBugCheckEx = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > KeBugCheckEx\n",
        Block->KeBugCheckEx);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExInterlockedRemoveHeadList");

    Block->ExInterlockedRemoveHeadList = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExInterlockedRemoveHeadList\n",
        Block->ExInterlockedRemoveHeadList);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExAcquireRundownProtection");

    Block->ExAcquireRundownProtection = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExAcquireRundownProtection\n",
        Block->ExAcquireRundownProtection);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExReleaseRundownProtection");

    Block->ExReleaseRundownProtection = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExReleaseRundownProtection\n",
        Block->ExReleaseRundownProtection);
#endif // DEBUG

    RtlInitUnicodeString(&String, L"ExWaitForRundownProtectionRelease");

    Block->ExWaitForRundownProtectionRelease = MmGetSystemRoutineAddress(&String);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExWaitForRundownProtectionRelease\n",
        Block->ExWaitForRundownProtectionRelease);
#endif // DEBUG

    Context.ContextFlags = CONTEXT_FULL;

    RtlCaptureContext(&Context);

    DumpHeader = __malloc(DUMP_BLOCK_SIZE);

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

        KdDebuggerDataBlock = (u8ptr)DumpHeader + KDDEBUGGER_DATA_OFFSET;

        RtlCopyMemory(
            &Block->DebuggerDataBlock,
            KdDebuggerDataBlock,
            sizeof(KDDEBUGGER_DATA64));

        KdDebuggerDataAdditionBlock = (PKDDEBUGGER_DATA_ADDITION64)(KdDebuggerDataBlock + 1);

        RtlCopyMemory(
            &Block->DebuggerDataAdditionBlock,
            KdDebuggerDataAdditionBlock,
            sizeof(KDDEBUGGER_DATA_ADDITION64));

        Block->PsLoadedModuleList =
            (PLIST_ENTRY)Block->DebuggerDataBlock.PsLoadedModuleList;

        Block->KernelDataTableEntry = CONTAINING_RECORD(
            Block->PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

#ifdef DEBUG
        /// vDbgPrint("[Shark] [Kernel] < %p > Header\n", KdDebuggerDataBlock->Header);
        //vDbgPrint("[Shark] [Kernel] < %p > KernBase\n", KdDebuggerDataBlock->KernBase);
        /// vDbgPrint("[Shark] [Kernel] < %p > BreakpointWithStatus\n", KdDebuggerDataBlock->BreakpointWithStatus);
        /// vDbgPrint("[Shark] [Kernel] < %p > SavedContext\n", KdDebuggerDataBlock->SavedContext);
        /// vDbgPrint("[Shark] [Kernel] < %p > ThCallbackStack\n", KdDebuggerDataBlock->ThCallbackStack);
        /// vDbgPrint("[Shark] [Kernel] < %p > NextCallback\n", KdDebuggerDataBlock->NextCallback);
        /// vDbgPrint("[Shark] [Kernel] < %p > FramePointer\n", KdDebuggerDataBlock->FramePointer);
        // vDbgPrint("[Shark] [Kernel] < %p > PaeEnabled\n", KdDebuggerDataBlock->PaeEnabled);
        /// vDbgPrint("[Shark] [Kernel] < %p > KiCallUserMode\n", KdDebuggerDataBlock->KiCallUserMode);
        /// vDbgPrint("[Shark] [Kernel] < %p > KeUserCallbackDispatcher\n", KdDebuggerDataBlock->KeUserCallbackDispatcher);
        // vDbgPrint("[Shark] [Kernel] < %p > PsLoadedModuleList\n", KdDebuggerDataBlock->PsLoadedModuleList);
        // vDbgPrint("[Shark] [Kernel] < %p > PsActiveProcessHead\n", KdDebuggerDataBlock->PsActiveProcessHead);
        // vDbgPrint("[Shark] [Kernel] < %p > PspCidTable\n", KdDebuggerDataBlock->PspCidTable);
        /// vDbgPrint("[Shark] [Kernel] < %p > ExpSystemResourcesList\n", KdDebuggerDataBlock->ExpSystemResourcesList);
        /// vDbgPrint("[Shark] [Kernel] < %p > ExpPagedPoolDescriptor\n", KdDebuggerDataBlock->ExpPagedPoolDescriptor);
        /// vDbgPrint("[Shark] [Kernel] < %p > ExpNumberOfPagedPools\n", KdDebuggerDataBlock->ExpNumberOfPagedPools);
        /// vDbgPrint("[Shark] [Kernel] < %p > KeTimeIncrement\n", KdDebuggerDataBlock->KeTimeIncrement);
        /// vDbgPrint("[Shark] [Kernel] < %p > KeBugCheckCallbackListHead\n", KdDebuggerDataBlock->KeBugCheckCallbackListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > KiBugcheckData\n", KdDebuggerDataBlock->KiBugcheckData);
        /// vDbgPrint("[Shark] [Kernel] < %p > IopErrorLogListHead\n", KdDebuggerDataBlock->IopErrorLogListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > ObpRootDirectoryObject\n", KdDebuggerDataBlock->ObpRootDirectoryObject);
        /// vDbgPrint("[Shark] [Kernel] < %p > ObpTypeObjectType\n", KdDebuggerDataBlock->ObpTypeObjectType);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemCacheStart\n", KdDebuggerDataBlock->MmSystemCacheStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemCacheEnd\n", KdDebuggerDataBlock->MmSystemCacheEnd);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemCacheWs\n", KdDebuggerDataBlock->MmSystemCacheWs);
        // vDbgPrint("[Shark] [Kernel] < %p > MmPfnDatabase\n", KdDebuggerDataBlock->MmPfnDatabase);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemPtesStart\n", KdDebuggerDataBlock->MmSystemPtesStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemPtesEnd\n", KdDebuggerDataBlock->MmSystemPtesEnd);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSubsectionBase\n", KdDebuggerDataBlock->MmSubsectionBase);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmNumberOfPagingFiles\n", KdDebuggerDataBlock->MmNumberOfPagingFiles);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmLowestPhysicalPage\n", KdDebuggerDataBlock->MmLowestPhysicalPage);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmHighestPhysicalPage\n", KdDebuggerDataBlock->MmHighestPhysicalPage);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmNumberOfPhysicalPages\n", KdDebuggerDataBlock->MmNumberOfPhysicalPages);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmMaximumNonPagedPoolInBytes\n", KdDebuggerDataBlock->MmMaximumNonPagedPoolInBytes);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmNonPagedSystemStart\n", KdDebuggerDataBlock->MmNonPagedSystemStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmNonPagedPoolStart\n", KdDebuggerDataBlock->MmNonPagedPoolStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmNonPagedPoolEnd\n", KdDebuggerDataBlock->MmNonPagedPoolEnd);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPagedPoolStart\n", KdDebuggerDataBlock->MmPagedPoolStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPagedPoolEnd\n", KdDebuggerDataBlock->MmPagedPoolEnd);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPagedPoolInformation\n", KdDebuggerDataBlock->MmPagedPoolInformation);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPageSize\n", KdDebuggerDataBlock->MmPageSize);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSizeOfPagedPoolInBytes\n", KdDebuggerDataBlock->MmSizeOfPagedPoolInBytes);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmTotalCommitLimit\n", KdDebuggerDataBlock->MmTotalCommitLimit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmTotalCommittedPages\n", KdDebuggerDataBlock->MmTotalCommittedPages);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSharedCommit\n", KdDebuggerDataBlock->MmSharedCommit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmDriverCommit\n", KdDebuggerDataBlock->MmDriverCommit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmProcessCommit\n", KdDebuggerDataBlock->MmProcessCommit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPagedPoolCommit\n", KdDebuggerDataBlock->MmPagedPoolCommit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmExtendedCommit\n", KdDebuggerDataBlock->MmExtendedCommit);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmZeroedPageListHead\n", KdDebuggerDataBlock->MmZeroedPageListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmFreePageListHead\n", KdDebuggerDataBlock->MmFreePageListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmStandbyPageListHead\n", KdDebuggerDataBlock->MmStandbyPageListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmModifiedPageListHead\n", KdDebuggerDataBlock->MmModifiedPageListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmModifiedNoWritePageListHead\n", KdDebuggerDataBlock->MmModifiedNoWritePageListHead);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmAvailablePages\n", KdDebuggerDataBlock->MmAvailablePages);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmResidentAvailablePages\n", KdDebuggerDataBlock->MmResidentAvailablePages);
        /// vDbgPrint("[Shark] [Kernel] < %p > PoolTrackTable\n", KdDebuggerDataBlock->PoolTrackTable);
        /// vDbgPrint("[Shark] [Kernel] < %p > NonPagedPoolDescriptor\n", KdDebuggerDataBlock->NonPagedPoolDescriptor);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmHighestUserAddress\n", KdDebuggerDataBlock->MmHighestUserAddress);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemRangeStart\n", KdDebuggerDataBlock->MmSystemRangeStart);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmUserProbeAddress\n", KdDebuggerDataBlock->MmUserProbeAddress);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintCircularBuffer\n", KdDebuggerDataBlock->KdPrintCircularBuffer);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintCircularBufferEnd\n", KdDebuggerDataBlock->KdPrintCircularBufferEnd);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintWritePointer\n", KdDebuggerDataBlock->KdPrintWritePointer);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintRolloverCount\n", KdDebuggerDataBlock->KdPrintRolloverCount);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmLoadedUserImageList\n", KdDebuggerDataBlock->MmLoadedUserImageList);
        /// vDbgPrint("[Shark] [Kernel] < %p > NtBuildLab\n", KdDebuggerDataBlock->NtBuildLab);
        /// vDbgPrint("[Shark] [Kernel] < %p > KiNormalSystemCall\n", KdDebuggerDataBlock->KiNormalSystemCall);
        /// vDbgPrint("[Shark] [Kernel] < %p > KiProcessorBlock\n", KdDebuggerDataBlock->KiProcessorBlock);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmUnloadedDrivers\n", KdDebuggerDataBlock->MmUnloadedDrivers);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmLastUnloadedDriver\n", KdDebuggerDataBlock->MmLastUnloadedDriver);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmTriageActionTaken\n", KdDebuggerDataBlock->MmTriageActionTaken);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSpecialPoolTag\n", KdDebuggerDataBlock->MmSpecialPoolTag);
        /// vDbgPrint("[Shark] [Kernel] < %p > KernelVerifier\n", KdDebuggerDataBlock->KernelVerifier);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmVerifierData\n", KdDebuggerDataBlock->MmVerifierData);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmAllocatedNonPagedPool\n", KdDebuggerDataBlock->MmAllocatedNonPagedPool);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPeakCommitment\n", KdDebuggerDataBlock->MmPeakCommitment);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmTotalCommitLimitMaximum\n", KdDebuggerDataBlock->MmTotalCommitLimitMaximum);
        /// vDbgPrint("[Shark] [Kernel] < %p > CmNtCSDVersion\n", KdDebuggerDataBlock->CmNtCSDVersion);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmPhysicalMemoryBlock\n", KdDebuggerDataBlock->MmPhysicalMemoryBlock);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSessionBase\n", KdDebuggerDataBlock->MmSessionBase);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSessionSize\n", KdDebuggerDataBlock->MmSessionSize);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmSystemParentTablePage\n", KdDebuggerDataBlock->MmSystemParentTablePage);
        /// vDbgPrint("[Shark] [Kernel] < %p > MmVirtualTranslationBase\n", KdDebuggerDataBlock->MmVirtualTranslationBase);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadNextProcessor\n", KdDebuggerDataBlock->OffsetKThreadNextProcessor);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadTeb\n", KdDebuggerDataBlock->OffsetKThreadTeb);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadKernelStack\n", KdDebuggerDataBlock->OffsetKThreadKernelStack);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadInitialStack\n", KdDebuggerDataBlock->OffsetKThreadInitialStack);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadApcProcess\n", KdDebuggerDataBlock->OffsetKThreadApcProcess);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadState\n", KdDebuggerDataBlock->OffsetKThreadState);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadBStore\n", KdDebuggerDataBlock->OffsetKThreadBStore);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetKThreadBStoreLimit\n", KdDebuggerDataBlock->OffsetKThreadBStoreLimit);
        // vDbgPrint("[Shark] [Kernel] < %p > SizeEProcess\n", KdDebuggerDataBlock->SizeEProcess);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetEprocessPeb\n", KdDebuggerDataBlock->OffsetEprocessPeb);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetEprocessParentCID\n", KdDebuggerDataBlock->OffsetEprocessParentCID);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetEprocessDirectoryTableBase\n", KdDebuggerDataBlock->OffsetEprocessDirectoryTableBase);
        // vDbgPrint("[Shark] [Kernel] < %p > SizePrcb\n", KdDebuggerDataBlock->SizePrcb);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbDpcRoutine\n", KdDebuggerDataBlock->OffsetPrcbDpcRoutine);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbCurrentThread\n", KdDebuggerDataBlock->OffsetPrcbCurrentThread);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbMhz\n", KdDebuggerDataBlock->OffsetPrcbMhz);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbCpuType\n", KdDebuggerDataBlock->OffsetPrcbCpuType);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbVendorString\n", KdDebuggerDataBlock->OffsetPrcbVendorString);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbProcStateContext\n", KdDebuggerDataBlock->OffsetPrcbProcStateContext);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbNumber\n", KdDebuggerDataBlock->OffsetPrcbNumber);
        // vDbgPrint("[Shark] [Kernel] < %p > SizeEThread\n", KdDebuggerDataBlock->SizeEThread);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintCircularBufferPtr\n", KdDebuggerDataBlock->KdPrintCircularBufferPtr);
        /// vDbgPrint("[Shark] [Kernel] < %p > KdPrintBufferSize\n", KdDebuggerDataBlock->KdPrintBufferSize);
        /// vDbgPrint("[Shark] [Kernel] < %p > KeLoaderBlock\n", KdDebuggerDataBlock->KeLoaderBlock);
        // vDbgPrint("[Shark] [Kernel] < %p > SizePcr\n", KdDebuggerDataBlock->SizePcr);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrSelfPcr\n", KdDebuggerDataBlock->OffsetPcrSelfPcr);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrCurrentPrcb\n", KdDebuggerDataBlock->OffsetPcrCurrentPrcb);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrContainedPrcb\n", KdDebuggerDataBlock->OffsetPcrContainedPrcb);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrInitialBStore\n", KdDebuggerDataBlock->OffsetPcrInitialBStore);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrBStoreLimit\n", KdDebuggerDataBlock->OffsetPcrBStoreLimit);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrInitialStack\n", KdDebuggerDataBlock->OffsetPcrInitialStack);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPcrStackLimit\n", KdDebuggerDataBlock->OffsetPcrStackLimit);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbPcrPage\n", KdDebuggerDataBlock->OffsetPrcbPcrPage);
        // vDbgPrint("[Shark] [Kernel] < %p > OffsetPrcbProcStateSpecialReg\n", KdDebuggerDataBlock->OffsetPrcbProcStateSpecialReg);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR0Code\n", KdDebuggerDataBlock->GdtR0Code);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR0Data\n", KdDebuggerDataBlock->GdtR0Data);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR0Pcr\n", KdDebuggerDataBlock->GdtR0Pcr);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR3Code\n", KdDebuggerDataBlock->GdtR3Code);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR3Data\n", KdDebuggerDataBlock->GdtR3Data);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtR3Teb\n", KdDebuggerDataBlock->GdtR3Teb);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtLdt\n", KdDebuggerDataBlock->GdtLdt);
        // vDbgPrint("[Shark] [Kernel] < %p > GdtTss\n", KdDebuggerDataBlock->GdtTss);
        // vDbgPrint("[Shark] [Kernel] < %p > Gdt64R3CmCode\n", KdDebuggerDataBlock->Gdt64R3CmCode);
        // vDbgPrint("[Shark] [Kernel] < %p > Gdt64R3CmTeb\n", KdDebuggerDataBlock->Gdt64R3CmTeb);
        /// vDbgPrint("[Shark] [Kernel] < %p > IopNumTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopNumTriageDumpDataBlocks);
        /// vDbgPrint("[Shark] [Kernel] < %p > IopTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopTriageDumpDataBlocks);

        if (Block->BuildNumber >= 10586) {
            // vDbgPrint("[Shark] [Kernel] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
        }
#endif // DEBUG

        __free(DumpHeader);
    }

#ifndef _WIN64
    Block->OffsetKProcessThreadListHead = 0x2c;

    if (Block->BuildNumber < 9200) {
        Block->OffsetKThreadThreadListEntry = 0x1e0;
    }
    else {
        Block->OffsetKThreadThreadListEntry = 0x1d4;
    }
#else
    Block->OffsetKProcessThreadListHead = 0x30;
    Block->OffsetKThreadThreadListEntry = 0x2f8;
#endif // !_WIN64

#ifndef _WIN64
    RtlInitUnicodeString(&String, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&String);

    ControlPc = ScanBytes(
        ControlPc,
        ControlPc + PAGE_SIZE,
        PsLoadedModuleResource);

    if (NULL != ControlPc) {
        Block->PsLoadedModuleResource = *(PERESOURCE *)(ControlPc + 3);
    }

    RtlInitUnicodeString(&String, L"KeServiceDescriptorTable");

    Block->KeServiceDescriptorTable = MmGetSystemRoutineAddress(&String);

    NtSection = FindSection(
        (ptr)Block->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        SectionBase =
            (u8ptr)Block->DebuggerDataBlock.KernBase + NtSection->VirtualAddress;

        SizeToLock = max(
            NtSection->SizeOfRawData,
            NtSection->Misc.VirtualSize);

        ControlPc = ScanBytes(
            SectionBase,
            (u8ptr)SectionBase + SizeToLock,
            PerfGlobalGroupMask);

        if (NULL != ControlPc) {
            Block->PerfInfoLogSysCallEntry = ControlPc + 0xd;

            RtlCopyMemory(
                Block->KiSystemServiceCopyEnd,
                Block->PerfInfoLogSysCallEntry,
                sizeof(Block->KiSystemServiceCopyEnd));

            Block->PerfGlobalGroupMask = UlongToPtr(*(u32 *)(ControlPc + 4) - 8);
}
    }
#else
    RtlInitUnicodeString(&String, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&String);

    ControlPc = ScanBytes(
        ControlPc,
        ControlPc + PAGE_SIZE,
        PsLoadedModuleResource);

    if (NULL != ControlPc) {
        Block->PsLoadedModuleResource = (PERESOURCE)__rva_to_va(ControlPc + 3);
    }

    NtSection = FindSection(
        (ptr)Block->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        SectionBase =
            (u8ptr)Block->DebuggerDataBlock.KernBase + NtSection->VirtualAddress;

        SizeToLock = max(
            NtSection->SizeOfRawData,
            NtSection->Misc.VirtualSize);

        ControlPc = ScanBytes(
            SectionBase,
            (u8ptr)SectionBase + SizeToLock,
            KiSystemCall64);

        if (NULL != ControlPc) {
            Block->KeServiceDescriptorTable = __rva_to_va(ControlPc + 23);
            Block->KeServiceDescriptorTableShadow = __rva_to_va(ControlPc + 30);

            ControlPc = ScanBytes(
                ControlPc,
                (u8ptr)SectionBase + SizeToLock,
                PerfGlobalGroupMask);

            if (NULL != ControlPc) {
                Block->PerfInfoLogSysCallEntry = ControlPc + 0xa;

                RtlCopyMemory(
                    Block->KiSystemServiceCopyEnd,
                    Block->PerfInfoLogSysCallEntry,
                    sizeof(Block->KiSystemServiceCopyEnd));

                Block->PerfGlobalGroupMask = __rva_to_va_ex(ControlPc + 2, 0 - sizeof(s32));
            }
        }
    }
#endif // !_WIN64

    RtlInitUnicodeString(&String, L"PsGetThreadProcessId");

    ControlPc = MmGetSystemRoutineAddress(&String);

    if (NULL != ControlPc) {
#ifndef _WIN64
        Block->OffsetKThreadProcessId = *(u32*)(ControlPc + 10);
#else
        Block->OffsetKThreadProcessId = *(u32*)(ControlPc + 3);
#endif // !_WIN64
    }

    InitializeListHead(&Block->LoadedPrivateImageList);
}

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
    PIMAGE_SECTION_HEADER FoundSection = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    ULONG SizeToLock = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FoundSection = IMAGE_FIRST_SECTION(NtHeaders);
        Offset = (ULONG)((ULONG_PTR)Address - (ULONG_PTR)ImageBase);

        for (Index = 0;
            Index < NtHeaders->FileHeader.NumberOfSections;
            Index++) {
            SizeToLock = max(
                FoundSection[Index].SizeOfRawData,
                FoundSection[Index].Misc.VirtualSize);

            if (Offset >= FoundSection[Index].VirtualAddress &&
                Offset < FoundSection[Index].VirtualAddress + SizeToLock) {
                NtSection = &FoundSection[Index];

                break;
            }
        }
    }

    return NtSection;
}

PIMAGE_SECTION_HEADER
NTAPI
FindSection(
    __in PVOID ImageBase,
    __in PCSTR SectionName
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PIMAGE_SECTION_HEADER FoundSection = NULL;
    ULONG Index = 0;
    ULONG Maximun = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FoundSection = IMAGE_FIRST_SECTION(NtHeaders);

        Maximun = min(strlen(SectionName), 8);

        for (Index = 0;
            Index < NtHeaders->FileHeader.NumberOfSections;
            Index++) {
            if (0 == _strnicmp(
                FoundSection[Index].Name,
                SectionName,
                Maximun)) {
                NtSection = &FoundSection[Index];

                break;
            }
        }
    }

    return NtSection;
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

    GpBlock.KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(GpBlock.PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(GpBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            GpBlock.PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)GpBlock.PsLoadedModuleList) {
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

    ExReleaseResourceLite(GpBlock.PsLoadedModuleResource);
    GpBlock.KeLeaveCriticalRegion();

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

    GpBlock.KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(GpBlock.PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(GpBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            (GpBlock.PsLoadedModuleList)->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)GpBlock.PsLoadedModuleList) {
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

    ExReleaseResourceLite(GpBlock.PsLoadedModuleResource);
    GpBlock.KeLeaveCriticalRegion();

    return Status;
}
