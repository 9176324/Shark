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
* The Initial Developer of the Original Code is blindtiger.
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
    __in PRTB Rtb
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    u32 SizeToLock = 0;
    u8ptr ControlPc = NULL;
    PDUMP_HEADER DumpHeader = NULL;
    PKDDEBUGGER_DATA64 KdDebuggerDataBlock = NULL;
    PKDDEBUGGER_DATA_ADDITION64 KdDebuggerDataAdditionBlock = NULL;
    ptr RoutineAddress = NULL;
    PLIST_ENTRY ActiveProcessEntry = NULL;
    s32 Number = -1;
    u8ptr TargetPc = NULL;
    u32 FirstLength = 0;
    u32 Length = 0;
    CONTEXT Context = { 0 };
    UNICODE_STRING String = { 0 };

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

    Rtb->Linkage[0] = 0x33;
    Rtb->Linkage[1] = 0xc0;
    Rtb->Linkage[2] = 0xc3;

    PsGetVersion(NULL, NULL, &Rtb->BuildNumber, NULL);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > BuildNumber\n",
            Rtb->BuildNumber);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"PsInitialSystemProcess");

    RoutineAddress = MmGetSystemRoutineAddress(&String);

    RtlCopyMemory(
        &Rtb->PsInitialSystemProcess,
        RoutineAddress,
        sizeof(ptr));

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > PsInitialSystemProcess\n",
            Rtb->PsInitialSystemProcess);
#endif // DEBUG                              
    }

    RtlInitUnicodeString(&String, L"KeNumberProcessors");

    RoutineAddress = MmGetSystemRoutineAddress(&String);

    RtlCopyMemory(
        &Rtb->NumberProcessors,
        RoutineAddress,
        sizeof(u8));

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > NumberProcessors\n",
            Rtb->NumberProcessors);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"KeEnterCriticalRegion");

    Rtb->KeEnterCriticalRegion = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > KeEnterCriticalRegion\n",
            Rtb->KeEnterCriticalRegion);
#endif // DEBUG                              
    }

    RtlInitUnicodeString(&String, L"KeLeaveCriticalRegion");

    Rtb->KeLeaveCriticalRegion = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > KeLeaveCriticalRegion\n",
            Rtb->KeLeaveCriticalRegion);
#endif // DEBUG              
    }

    RtlInitUnicodeString(&String, L"ExAcquireSpinLockShared");

    Rtb->ExAcquireSpinLockShared = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExAcquireSpinLockShared\n",
            Rtb->ExAcquireSpinLockShared);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExReleaseSpinLockShared");

    Rtb->ExReleaseSpinLockShared = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExReleaseSpinLockShared\n",
            Rtb->ExReleaseSpinLockShared);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"DbgPrint");

    Rtb->DbgPrint = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > DbgPrint\n",
            Rtb->DbgPrint);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"KeWaitForSingleObject");

    Rtb->KeWaitForSingleObject = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > KeWaitForSingleObject\n",
            Rtb->KeWaitForSingleObject);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"RtlCompareMemory");

    Rtb->RtlCompareMemory = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > RtlCompareMemory\n",
            Rtb->RtlCompareMemory);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"RtlRestoreContext");

    Rtb->RtlRestoreContext = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > RtlRestoreContext\n",
            Rtb->RtlRestoreContext);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExQueueWorkItem");

    Rtb->ExQueueWorkItem = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExQueueWorkItem\n",
            Rtb->ExQueueWorkItem);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExFreePoolWithTag");

    Rtb->ExFreePoolWithTag = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExFreePoolWithTag\n",
            Rtb->ExFreePoolWithTag);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"KeBugCheckEx");

    Rtb->KeBugCheckEx = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > KeBugCheckEx\n",
            Rtb->KeBugCheckEx);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExInterlockedRemoveHeadList");

    Rtb->ExInterlockedRemoveHeadList = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExInterlockedRemoveHeadList\n",
            Rtb->ExInterlockedRemoveHeadList);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExAcquireRundownProtection");

    Rtb->ExAcquireRundownProtection = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExAcquireRundownProtection\n",
            Rtb->ExAcquireRundownProtection);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExReleaseRundownProtection");

    Rtb->ExReleaseRundownProtection = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExReleaseRundownProtection\n",
            Rtb->ExReleaseRundownProtection);
#endif // DEBUG
    }

    RtlInitUnicodeString(&String, L"ExWaitForRundownProtectionRelease");

    Rtb->ExWaitForRundownProtectionRelease = MmGetSystemRoutineAddress(&String);

    if (CmdReload !=
        (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > ExWaitForRundownProtectionRelease\n",
            Rtb->ExWaitForRundownProtectionRelease);
#endif // DEBUG
    }

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
            &Rtb->DebuggerDataBlock,
            KdDebuggerDataBlock,
            sizeof(KDDEBUGGER_DATA64));

        KdDebuggerDataAdditionBlock = (PKDDEBUGGER_DATA_ADDITION64)(KdDebuggerDataBlock + 1);

        RtlCopyMemory(
            &Rtb->DebuggerDataAdditionBlock,
            KdDebuggerDataAdditionBlock,
            sizeof(KDDEBUGGER_DATA_ADDITION64));

        Rtb->PsLoadedModuleList =
            (PLIST_ENTRY)Rtb->DebuggerDataBlock.PsLoadedModuleList;

        Rtb->KernelDataTableEntry = CONTAINING_RECORD(
            Rtb->PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        if (CmdReload !=
            (Rtb->Operation & CmdReload)) {
#ifdef DEBUG
            /// vDbgPrint("[SHARK] < %p > Header\n", KdDebuggerDataBlock->Header);
            //vDbgPrint("[SHARK] < %p > KernBase\n", KdDebuggerDataBlock->KernBase);
            /// vDbgPrint("[SHARK] < %p > BreakpointWithStatus\n", KdDebuggerDataBlock->BreakpointWithStatus);
            /// vDbgPrint("[SHARK] < %p > SavedContext\n", KdDebuggerDataBlock->SavedContext);
            /// vDbgPrint("[SHARK] < %p > ThCallbackStack\n", KdDebuggerDataBlock->ThCallbackStack);
            /// vDbgPrint("[SHARK] < %p > NextCallback\n", KdDebuggerDataBlock->NextCallback);
            /// vDbgPrint("[SHARK] < %p > FramePointer\n", KdDebuggerDataBlock->FramePointer);
            // vDbgPrint("[SHARK] < %p > PaeEnabled\n", KdDebuggerDataBlock->PaeEnabled);
            /// vDbgPrint("[SHARK] < %p > KiCallUserMode\n", KdDebuggerDataBlock->KiCallUserMode);
            /// vDbgPrint("[SHARK] < %p > KeUserCallbackDispatcher\n", KdDebuggerDataBlock->KeUserCallbackDispatcher);
            // vDbgPrint("[SHARK] < %p > PsLoadedModuleList\n", KdDebuggerDataBlock->PsLoadedModuleList);
            // vDbgPrint("[SHARK] < %p > PsActiveProcessHead\n", KdDebuggerDataBlock->PsActiveProcessHead);
            // vDbgPrint("[SHARK] < %p > PspCidTable\n", KdDebuggerDataBlock->PspCidTable);
            /// vDbgPrint("[SHARK] < %p > ExpSystemResourcesList\n", KdDebuggerDataBlock->ExpSystemResourcesList);
            /// vDbgPrint("[SHARK] < %p > ExpPagedPoolDescriptor\n", KdDebuggerDataBlock->ExpPagedPoolDescriptor);
            /// vDbgPrint("[SHARK] < %p > ExpNumberOfPagedPools\n", KdDebuggerDataBlock->ExpNumberOfPagedPools);
            /// vDbgPrint("[SHARK] < %p > KeTimeIncrement\n", KdDebuggerDataBlock->KeTimeIncrement);
            /// vDbgPrint("[SHARK] < %p > KeBugCheckCallbackListHead\n", KdDebuggerDataBlock->KeBugCheckCallbackListHead);
            /// vDbgPrint("[SHARK] < %p > KiBugcheckData\n", KdDebuggerDataBlock->KiBugcheckData);
            /// vDbgPrint("[SHARK] < %p > IopErrorLogListHead\n", KdDebuggerDataBlock->IopErrorLogListHead);
            /// vDbgPrint("[SHARK] < %p > ObpRootDirectoryObject\n", KdDebuggerDataBlock->ObpRootDirectoryObject);
            /// vDbgPrint("[SHARK] < %p > ObpTypeObjectType\n", KdDebuggerDataBlock->ObpTypeObjectType);
            /// vDbgPrint("[SHARK] < %p > MmSystemCacheStart\n", KdDebuggerDataBlock->MmSystemCacheStart);
            /// vDbgPrint("[SHARK] < %p > MmSystemCacheEnd\n", KdDebuggerDataBlock->MmSystemCacheEnd);
            /// vDbgPrint("[SHARK] < %p > MmSystemCacheWs\n", KdDebuggerDataBlock->MmSystemCacheWs);
            // vDbgPrint("[SHARK] < %p > MmPfnDatabase\n", KdDebuggerDataBlock->MmPfnDatabase);
            /// vDbgPrint("[SHARK] < %p > MmSystemPtesStart\n", KdDebuggerDataBlock->MmSystemPtesStart);
            /// vDbgPrint("[SHARK] < %p > MmSystemPtesEnd\n", KdDebuggerDataBlock->MmSystemPtesEnd);
            /// vDbgPrint("[SHARK] < %p > MmSubsectionBase\n", KdDebuggerDataBlock->MmSubsectionBase);
            /// vDbgPrint("[SHARK] < %p > MmNumberOfPagingFiles\n", KdDebuggerDataBlock->MmNumberOfPagingFiles);
            /// vDbgPrint("[SHARK] < %p > MmLowestPhysicalPage\n", KdDebuggerDataBlock->MmLowestPhysicalPage);
            /// vDbgPrint("[SHARK] < %p > MmHighestPhysicalPage\n", KdDebuggerDataBlock->MmHighestPhysicalPage);
            /// vDbgPrint("[SHARK] < %p > MmNumberOfPhysicalPages\n", KdDebuggerDataBlock->MmNumberOfPhysicalPages);
            /// vDbgPrint("[SHARK] < %p > MmMaximumNonPagedPoolInBytes\n", KdDebuggerDataBlock->MmMaximumNonPagedPoolInBytes);
            /// vDbgPrint("[SHARK] < %p > MmNonPagedSystemStart\n", KdDebuggerDataBlock->MmNonPagedSystemStart);
            /// vDbgPrint("[SHARK] < %p > MmNonPagedPoolStart\n", KdDebuggerDataBlock->MmNonPagedPoolStart);
            /// vDbgPrint("[SHARK] < %p > MmNonPagedPoolEnd\n", KdDebuggerDataBlock->MmNonPagedPoolEnd);
            /// vDbgPrint("[SHARK] < %p > MmPagedPoolStart\n", KdDebuggerDataBlock->MmPagedPoolStart);
            /// vDbgPrint("[SHARK] < %p > MmPagedPoolEnd\n", KdDebuggerDataBlock->MmPagedPoolEnd);
            /// vDbgPrint("[SHARK] < %p > MmPagedPoolInformation\n", KdDebuggerDataBlock->MmPagedPoolInformation);
            /// vDbgPrint("[SHARK] < %p > MmPageSize\n", KdDebuggerDataBlock->MmPageSize);
            /// vDbgPrint("[SHARK] < %p > MmSizeOfPagedPoolInBytes\n", KdDebuggerDataBlock->MmSizeOfPagedPoolInBytes);
            /// vDbgPrint("[SHARK] < %p > MmTotalCommitLimit\n", KdDebuggerDataBlock->MmTotalCommitLimit);
            /// vDbgPrint("[SHARK] < %p > MmTotalCommittedPages\n", KdDebuggerDataBlock->MmTotalCommittedPages);
            /// vDbgPrint("[SHARK] < %p > MmSharedCommit\n", KdDebuggerDataBlock->MmSharedCommit);
            /// vDbgPrint("[SHARK] < %p > MmDriverCommit\n", KdDebuggerDataBlock->MmDriverCommit);
            /// vDbgPrint("[SHARK] < %p > MmProcessCommit\n", KdDebuggerDataBlock->MmProcessCommit);
            /// vDbgPrint("[SHARK] < %p > MmPagedPoolCommit\n", KdDebuggerDataBlock->MmPagedPoolCommit);
            /// vDbgPrint("[SHARK] < %p > MmExtendedCommit\n", KdDebuggerDataBlock->MmExtendedCommit);
            /// vDbgPrint("[SHARK] < %p > MmZeroedPageListHead\n", KdDebuggerDataBlock->MmZeroedPageListHead);
            /// vDbgPrint("[SHARK] < %p > MmFreePageListHead\n", KdDebuggerDataBlock->MmFreePageListHead);
            /// vDbgPrint("[SHARK] < %p > MmStandbyPageListHead\n", KdDebuggerDataBlock->MmStandbyPageListHead);
            /// vDbgPrint("[SHARK] < %p > MmModifiedPageListHead\n", KdDebuggerDataBlock->MmModifiedPageListHead);
            /// vDbgPrint("[SHARK] < %p > MmModifiedNoWritePageListHead\n", KdDebuggerDataBlock->MmModifiedNoWritePageListHead);
            /// vDbgPrint("[SHARK] < %p > MmAvailablePages\n", KdDebuggerDataBlock->MmAvailablePages);
            /// vDbgPrint("[SHARK] < %p > MmResidentAvailablePages\n", KdDebuggerDataBlock->MmResidentAvailablePages);
            /// vDbgPrint("[SHARK] < %p > PoolTrackTable\n", KdDebuggerDataBlock->PoolTrackTable);
            /// vDbgPrint("[SHARK] < %p > NonPagedPoolDescriptor\n", KdDebuggerDataBlock->NonPagedPoolDescriptor);
            /// vDbgPrint("[SHARK] < %p > MmHighestUserAddress\n", KdDebuggerDataBlock->MmHighestUserAddress);
            /// vDbgPrint("[SHARK] < %p > MmSystemRangeStart\n", KdDebuggerDataBlock->MmSystemRangeStart);
            /// vDbgPrint("[SHARK] < %p > MmUserProbeAddress\n", KdDebuggerDataBlock->MmUserProbeAddress);
            /// vDbgPrint("[SHARK] < %p > KdPrintCircularBuffer\n", KdDebuggerDataBlock->KdPrintCircularBuffer);
            /// vDbgPrint("[SHARK] < %p > KdPrintCircularBufferEnd\n", KdDebuggerDataBlock->KdPrintCircularBufferEnd);
            /// vDbgPrint("[SHARK] < %p > KdPrintWritePointer\n", KdDebuggerDataBlock->KdPrintWritePointer);
            /// vDbgPrint("[SHARK] < %p > KdPrintRolloverCount\n", KdDebuggerDataBlock->KdPrintRolloverCount);
            /// vDbgPrint("[SHARK] < %p > MmLoadedUserImageList\n", KdDebuggerDataBlock->MmLoadedUserImageList);
            /// vDbgPrint("[SHARK] < %p > NtBuildLab\n", KdDebuggerDataBlock->NtBuildLab);
            /// vDbgPrint("[SHARK] < %p > KiNormalSystemCall\n", KdDebuggerDataBlock->KiNormalSystemCall);
            /// vDbgPrint("[SHARK] < %p > KiProcessorBlock\n", KdDebuggerDataBlock->KiProcessorBlock);
            /// vDbgPrint("[SHARK] < %p > MmUnloadedDrivers\n", KdDebuggerDataBlock->MmUnloadedDrivers);
            /// vDbgPrint("[SHARK] < %p > MmLastUnloadedDriver\n", KdDebuggerDataBlock->MmLastUnloadedDriver);
            /// vDbgPrint("[SHARK] < %p > MmTriageActionTaken\n", KdDebuggerDataBlock->MmTriageActionTaken);
            /// vDbgPrint("[SHARK] < %p > MmSpecialPoolTag\n", KdDebuggerDataBlock->MmSpecialPoolTag);
            /// vDbgPrint("[SHARK] < %p > KernelVerifier\n", KdDebuggerDataBlock->KernelVerifier);
            /// vDbgPrint("[SHARK] < %p > MmVerifierData\n", KdDebuggerDataBlock->MmVerifierData);
            /// vDbgPrint("[SHARK] < %p > MmAllocatedNonPagedPool\n", KdDebuggerDataBlock->MmAllocatedNonPagedPool);
            /// vDbgPrint("[SHARK] < %p > MmPeakCommitment\n", KdDebuggerDataBlock->MmPeakCommitment);
            /// vDbgPrint("[SHARK] < %p > MmTotalCommitLimitMaximum\n", KdDebuggerDataBlock->MmTotalCommitLimitMaximum);
            /// vDbgPrint("[SHARK] < %p > CmNtCSDVersion\n", KdDebuggerDataBlock->CmNtCSDVersion);
            /// vDbgPrint("[SHARK] < %p > MmPhysicalMemoryBlock\n", KdDebuggerDataBlock->MmPhysicalMemoryBlock);
            /// vDbgPrint("[SHARK] < %p > MmSessionBase\n", KdDebuggerDataBlock->MmSessionBase);
            /// vDbgPrint("[SHARK] < %p > MmSessionSize\n", KdDebuggerDataBlock->MmSessionSize);
            /// vDbgPrint("[SHARK] < %p > MmSystemParentTablePage\n", KdDebuggerDataBlock->MmSystemParentTablePage);
            /// vDbgPrint("[SHARK] < %p > MmVirtualTranslationBase\n", KdDebuggerDataBlock->MmVirtualTranslationBase);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadNextProcessor\n", KdDebuggerDataBlock->OffsetKThreadNextProcessor);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadTeb\n", KdDebuggerDataBlock->OffsetKThreadTeb);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadKernelStack\n", KdDebuggerDataBlock->OffsetKThreadKernelStack);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadInitialStack\n", KdDebuggerDataBlock->OffsetKThreadInitialStack);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadApcProcess\n", KdDebuggerDataBlock->OffsetKThreadApcProcess);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadState\n", KdDebuggerDataBlock->OffsetKThreadState);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadBStore\n", KdDebuggerDataBlock->OffsetKThreadBStore);
            // vDbgPrint("[SHARK] < %p > OffsetKThreadBStoreLimit\n", KdDebuggerDataBlock->OffsetKThreadBStoreLimit);
            // vDbgPrint("[SHARK] < %p > SizeEProcess\n", KdDebuggerDataBlock->SizeEProcess);
            // vDbgPrint("[SHARK] < %p > OffsetEprocessPeb\n", KdDebuggerDataBlock->OffsetEprocessPeb);
            // vDbgPrint("[SHARK] < %p > OffsetEprocessParentCID\n", KdDebuggerDataBlock->OffsetEprocessParentCID);
            // vDbgPrint("[SHARK] < %p > OffsetEprocessDirectoryTableBase\n", KdDebuggerDataBlock->OffsetEprocessDirectoryTableBase);
            // vDbgPrint("[SHARK] < %p > SizePrcb\n", KdDebuggerDataBlock->SizePrcb);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbDpcRoutine\n", KdDebuggerDataBlock->OffsetPrcbDpcRoutine);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbCurrentThread\n", KdDebuggerDataBlock->OffsetPrcbCurrentThread);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbMhz\n", KdDebuggerDataBlock->OffsetPrcbMhz);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbCpuType\n", KdDebuggerDataBlock->OffsetPrcbCpuType);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbVendorString\n", KdDebuggerDataBlock->OffsetPrcbVendorString);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbProcStateContext\n", KdDebuggerDataBlock->OffsetPrcbProcStateContext);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbNumber\n", KdDebuggerDataBlock->OffsetPrcbNumber);
            // vDbgPrint("[SHARK] < %p > SizeEThread\n", KdDebuggerDataBlock->SizeEThread);
            /// vDbgPrint("[SHARK] < %p > KdPrintCircularBufferPtr\n", KdDebuggerDataBlock->KdPrintCircularBufferPtr);
            /// vDbgPrint("[SHARK] < %p > KdPrintBufferSize\n", KdDebuggerDataBlock->KdPrintBufferSize);
            /// vDbgPrint("[SHARK] < %p > KeLoaderBlock\n", KdDebuggerDataBlock->KeLoaderBlock);
            // vDbgPrint("[SHARK] < %p > SizePcr\n", KdDebuggerDataBlock->SizePcr);
            // vDbgPrint("[SHARK] < %p > OffsetPcrSelfPcr\n", KdDebuggerDataBlock->OffsetPcrSelfPcr);
            // vDbgPrint("[SHARK] < %p > OffsetPcrCurrentPrcb\n", KdDebuggerDataBlock->OffsetPcrCurrentPrcb);
            // vDbgPrint("[SHARK] < %p > OffsetPcrContainedPrcb\n", KdDebuggerDataBlock->OffsetPcrContainedPrcb);
            // vDbgPrint("[SHARK] < %p > OffsetPcrInitialBStore\n", KdDebuggerDataBlock->OffsetPcrInitialBStore);
            // vDbgPrint("[SHARK] < %p > OffsetPcrBStoreLimit\n", KdDebuggerDataBlock->OffsetPcrBStoreLimit);
            // vDbgPrint("[SHARK] < %p > OffsetPcrInitialStack\n", KdDebuggerDataBlock->OffsetPcrInitialStack);
            // vDbgPrint("[SHARK] < %p > OffsetPcrStackLimit\n", KdDebuggerDataBlock->OffsetPcrStackLimit);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbPcrPage\n", KdDebuggerDataBlock->OffsetPrcbPcrPage);
            // vDbgPrint("[SHARK] < %p > OffsetPrcbProcStateSpecialReg\n", KdDebuggerDataBlock->OffsetPrcbProcStateSpecialReg);
            // vDbgPrint("[SHARK] < %p > GdtR0Code\n", KdDebuggerDataBlock->GdtR0Code);
            // vDbgPrint("[SHARK] < %p > GdtR0Data\n", KdDebuggerDataBlock->GdtR0Data);
            // vDbgPrint("[SHARK] < %p > GdtR0Pcr\n", KdDebuggerDataBlock->GdtR0Pcr);
            // vDbgPrint("[SHARK] < %p > GdtR3Code\n", KdDebuggerDataBlock->GdtR3Code);
            // vDbgPrint("[SHARK] < %p > GdtR3Data\n", KdDebuggerDataBlock->GdtR3Data);
            // vDbgPrint("[SHARK] < %p > GdtR3Teb\n", KdDebuggerDataBlock->GdtR3Teb);
            // vDbgPrint("[SHARK] < %p > GdtLdt\n", KdDebuggerDataBlock->GdtLdt);
            // vDbgPrint("[SHARK] < %p > GdtTss\n", KdDebuggerDataBlock->GdtTss);
            // vDbgPrint("[SHARK] < %p > Gdt64R3CmCode\n", KdDebuggerDataBlock->Gdt64R3CmCode);
            // vDbgPrint("[SHARK] < %p > Gdt64R3CmTeb\n", KdDebuggerDataBlock->Gdt64R3CmTeb);
            /// vDbgPrint("[SHARK] < %p > IopNumTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopNumTriageDumpDataBlocks);
            /// vDbgPrint("[SHARK] < %p > IopTriageDumpDataBlocks\n", KdDebuggerDataBlock->IopTriageDumpDataBlocks);

            if (Rtb->BuildNumber >= 10586) {
                // vDbgPrint("[SHARK] < %p > PteBase\n", KdDebuggerDataAdditionBlock->PteBase);
            }
#endif // DEBUG
        }

        __free(DumpHeader);
    }

#ifndef _WIN64
    Rtb->OffsetKProcessThreadListHead = 0x2c;

    if (Rtb->BuildNumber < 9200) {
        Rtb->OffsetKThreadThreadListEntry = 0x1e0;
    }
    else {
        Rtb->OffsetKThreadThreadListEntry = 0x1d4;
    }
#else
    Rtb->OffsetKProcessThreadListHead = 0x30;
    Rtb->OffsetKThreadThreadListEntry = 0x2f8;
#endif // !_WIN64

#ifndef _WIN64
    RtlInitUnicodeString(&String, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&String);

    ControlPc = ScanBytes(
        ControlPc,
        ControlPc + PAGE_SIZE,
        PsLoadedModuleResource);

    if (NULL != ControlPc) {
        Rtb->PsLoadedModuleResource = *(PERESOURCE *)(ControlPc + 3);
    }

    RtlInitUnicodeString(&String, L"KeServiceDescriptorTable");

    Rtb->KeServiceDescriptorTable = MmGetSystemRoutineAddress(&String);

    NtSection = LdrFindSection(
        (ptr)Rtb->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        SectionBase =
            (u8ptr)Rtb->DebuggerDataBlock.KernBase + NtSection->VirtualAddress;

        SizeToLock = max(
            NtSection->SizeOfRawData,
            NtSection->Misc.VirtualSize);

        ControlPc = ScanBytes(
            SectionBase,
            (u8ptr)SectionBase + SizeToLock,
            PerfGlobalGroupMask);

        if (NULL != ControlPc) {
            Rtb->PerfInfoLogSysCallEntry = ControlPc + 0xd;

            RtlCopyMemory(
                Rtb->KiSystemServiceCopyEnd,
                Rtb->PerfInfoLogSysCallEntry,
                sizeof(Rtb->KiSystemServiceCopyEnd));

            Rtb->PerfGlobalGroupMask = UlongToPtr(*(u32 *)(ControlPc + 4) - 8);
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
        Rtb->PsLoadedModuleResource = (PERESOURCE)__rva_to_va(ControlPc + 3);
    }

    NtSection = LdrFindSection(
        (ptr)Rtb->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        SectionBase =
            (u8ptr)Rtb->DebuggerDataBlock.KernBase + NtSection->VirtualAddress;

        SizeToLock = max(
            NtSection->SizeOfRawData,
            NtSection->Misc.VirtualSize);

        ControlPc = ScanBytes(
            SectionBase,
            (u8ptr)SectionBase + SizeToLock,
            KiSystemCall64);

        if (NULL != ControlPc) {
            Rtb->KeServiceDescriptorTable = __rva_to_va(ControlPc + 23);
            Rtb->KeServiceDescriptorTableShadow = __rva_to_va(ControlPc + 30);

            ControlPc = ScanBytes(
                ControlPc,
                (u8ptr)SectionBase + SizeToLock,
                PerfGlobalGroupMask);

            if (NULL != ControlPc) {
                Rtb->PerfInfoLogSysCallEntry = ControlPc + 0xa;

                RtlCopyMemory(
                    Rtb->KiSystemServiceCopyEnd,
                    Rtb->PerfInfoLogSysCallEntry,
                    sizeof(Rtb->KiSystemServiceCopyEnd));

                Rtb->PerfGlobalGroupMask = __rva_to_va_ex(ControlPc + 2, 0 - sizeof(s32));
            }
        }
    }
#endif // !_WIN64

    RtlInitUnicodeString(&String, L"PsGetThreadProcessId");

    ControlPc = MmGetSystemRoutineAddress(&String);

    if (NULL != ControlPc) {
#ifndef _WIN64
        Rtb->OffsetKThreadProcessId = *(u32*)(ControlPc + 10);
#else
        Rtb->OffsetKThreadProcessId = *(u32*)(ControlPc + 3);
#endif // !_WIN64
    }

    InitializeListHead(&Rtb->LoadedModuleList);

    if (FALSE == IsListEmpty(
        (PLIST_ENTRY)Rtb->DebuggerDataBlock.PsActiveProcessHead)) {
        ActiveProcessEntry =
            ((PLIST_ENTRY)Rtb->DebuggerDataBlock.PsActiveProcessHead)->Flink;

        while ((u)ActiveProcessEntry !=
            (u)Rtb->DebuggerDataBlock.PsActiveProcessHead) {
            if ((u)PsGetCurrentThreadProcessId() ==
                __rduptr((u)ActiveProcessEntry - sizeof(u))) {

                Rtb->OffsetEProcessActiveProcessLinks =
                    (u)ActiveProcessEntry - (u)PsGetCurrentThreadProcess();

                break;
            }

            ActiveProcessEntry = ActiveProcessEntry->Flink;
        }
    }

    RtlInitUnicodeString(&String, L"ZwClose");

    ControlPc = MmGetSystemRoutineAddress(&String);

    FirstLength = DetourGetInstructionLength(ControlPc);

    TargetPc = ControlPc + FirstLength;

    while (TRUE) {
        Length = DetourGetInstructionLength(TargetPc);

        if (FirstLength == Length) {
#ifndef _WIN64
            if (0 == _cmpbyte(TargetPc[0], ControlPc[0]) &&
                1 == *(u32ptr)&TargetPc[1] - *(u32ptr)&ControlPc[1]) {
                Rtb->NameInterval = TargetPc - ControlPc;

                break;
            }
#else
            if (FirstLength == RtlCompareMemory(
                TargetPc,
                ControlPc,
                FirstLength)) {
                Rtb->NameInterval = TargetPc - ControlPc;

                break;
            }
#endif // !_WIN64
        }

        TargetPc += Length;
    }
}

u32
NTAPI
LdrGetPlatform(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 Platform = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        Platform = NtHeaders->OptionalHeader.Magic;
    }

    return Platform;
}

ptr
NTAPI
LdrGetEntryPoint(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 Offset = 0;
    ptr EntryPoint = NULL;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (0 != Offset) {
            EntryPoint = (u8ptr)ImageBase + Offset;
        }
    }

    return EntryPoint;
}

u32
NTAPI
LdrGetTimeStamp(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 TimeStamp = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        TimeStamp = NtHeaders->FileHeader.TimeDateStamp;
    }

    return TimeStamp;
}

u16
NTAPI
LdrGetSubsystem(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u16 Subsystem = 0;

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

u32
NTAPI
LdrGetSize(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 SizeOfImage = 0;

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
    __in ptr ImageBase,
    __in ptr Address
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 Index = 0;
    u32 Offset = 0;
    PIMAGE_SECTION_HEADER FoundSection = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    u32 SizeToLock = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FoundSection = IMAGE_FIRST_SECTION(NtHeaders);
        Offset = (u32)((u)Address - (u)ImageBase);

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
LdrFindSection(
    __in ptr ImageBase,
    __in u8ptr SectionName
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PIMAGE_SECTION_HEADER FoundSection = NULL;
    u32 Index = 0;
    u32 Maximun = 0;
    u8 Name[IMAGE_SIZEOF_SHORT_NAME] = { 0 };

    strcpy_s(Name, IMAGE_SIZEOF_SHORT_NAME, SectionName);

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FoundSection = IMAGE_FIRST_SECTION(NtHeaders);

        for (Index = 0;
            Index < NtHeaders->FileHeader.NumberOfSections;
            Index++) {
            if (0 == _strnicmp(
                FoundSection[Index].Name,
                Name,
                IMAGE_SIZEOF_SHORT_NAME)) {
                NtSection = &FoundSection[Index];

                break;
            }
        }
    }

    return NtSection;
}

FORCEINLINE
u32
NTAPI
LdrGetRelocCount(
    __in u32 SizeOfBlock
)
{
    u32 Count = 0;

    Count = (SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(u16);

    return Count;
}

PIMAGE_BASE_RELOCATION
NTAPI
LdrRelocBlock(
    __in ptr VA,
    __in u32 Count,
    __in u16ptr NextOffset,
    __in s Diff
)
{
    u16ptr FixupVA = NULL;
    u16 Offset = 0;
    u16 Type = 0;

    while (Count--) {
        Offset = *NextOffset & 0xfff;
        FixupVA = (u8ptr)VA + Offset;
        Type = (*NextOffset >> 12) & 0xf;

        switch (Type) {
        case IMAGE_REL_BASED_ABSOLUTE: {
            break;
        }

        case IMAGE_REL_BASED_HIGH: {
            FixupVA[1] += (u16)((Diff >> 16) & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_LOW: {
            FixupVA[0] += (u16)(Diff & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_HIGHLOW: {
            *(u32ptr)FixupVA += (u32)Diff;
            break;
        }

        case IMAGE_REL_BASED_HIGHADJ: {
            FixupVA[0] += NextOffset[1] & 0xffff;
            FixupVA[1] += (u16)((Diff >> 16) & 0xffff);

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
            *(uptr)FixupVA += Diff;
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

void
NTAPI
LdrRelocImage(
    __in ptr ImageBase,
    __in s Diff
)
{
    PIMAGE_BASE_RELOCATION RelocDirectory = NULL;
    u32 Size = 0;
    ptr VA = 0;

    RelocDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_BASERELOC,
        &Size);

    if (0 != Size) {
        if (0 != Diff) {
            while (0 != Size) {
                VA = (u8ptr)ImageBase + RelocDirectory->VirtualAddress;
                Size -= RelocDirectory->SizeOfBlock;

                RelocDirectory = LdrRelocBlock(
                    VA,
                    LdrGetRelocCount(RelocDirectory->SizeOfBlock),
                    (u16ptr)(RelocDirectory + 1),
                    Diff);
            }
        }
    }
}

status
NTAPI
FindEntryForImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    status Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    RtBlock.KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(RtBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            RtBlock.PsLoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((u)FoundDataTableEntry !=
            (u)RtBlock.PsLoadedModuleList) {
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

    if (Status < 0) {
        if (FALSE == IsListEmpty(&RtBlock.LoadedModuleList)) {
            FoundDataTableEntry = CONTAINING_RECORD(
                RtBlock.LoadedModuleList.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);

            while ((u)FoundDataTableEntry != (u)&RtBlock.LoadedModuleList) {
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
    }

    ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);
    RtBlock.KeLeaveCriticalRegion();

    return Status;
}

status
NTAPI
FindEntryForImageAddress(
    __in ptr Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    status Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    RtBlock.KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);

    if (FALSE == IsListEmpty(RtBlock.PsLoadedModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            (RtBlock.PsLoadedModuleList)->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((u)FoundDataTableEntry !=
            (u)RtBlock.PsLoadedModuleList) {
            if ((u)Address >= (u)FoundDataTableEntry->DllBase &&
                (u)Address < (u)FoundDataTableEntry->DllBase +
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

    if (Status < 0) {
        if (FALSE == IsListEmpty(&RtBlock.LoadedModuleList)) {
            FoundDataTableEntry = CONTAINING_RECORD(
                RtBlock.LoadedModuleList.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);

            while ((u)FoundDataTableEntry != (u)&RtBlock.LoadedModuleList) {
                if ((u)Address >= (u)FoundDataTableEntry->DllBase &&
                    (u)Address < (u)FoundDataTableEntry->DllBase +
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
    }

    ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);
    RtBlock.KeLeaveCriticalRegion();

    return Status;
}

ptr
NTAPI
LdrForward(
    __in cptr ForwarderData
)
{
    status Status = STATUS_SUCCESS;
    cptr Separator = NULL;
    cptr ImageName = NULL;
    cptr ProcedureName = NULL;
    u32 ProcedureNumber = 0;
    ptr ProcedureAddress = NULL;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING String = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    Separator = strchr(ForwarderData, '.');

    if (NULL != Separator) {
        ImageName = __malloc(Separator - ForwarderData);

        if (NULL != ImageName) {
            RtlCopyMemory(
                ImageName,
                ForwarderData,
                Separator - ForwarderData);

            String.Buffer = ImageName;
            String.Length = Separator - ForwarderData;
            String.MaximumLength = Separator - ForwarderData;

            Status = RtlAnsiStringToUnicodeString(
                &ImageFileName,
                &String,
                TRUE);

            if (TRACE(Status)) {
                RtBlock.KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);

                if (FALSE == IsListEmpty(RtBlock.PsLoadedModuleList)) {
                    FoundDataTableEntry = CONTAINING_RECORD(
                        RtBlock.PsLoadedModuleList->Flink,
                        KLDR_DATA_TABLE_ENTRY,
                        InLoadOrderLinks);

                    while ((u)FoundDataTableEntry !=
                        (u)RtBlock.PsLoadedModuleList) {
                        if (FALSE != RtlPrefixUnicodeString(
                            &ImageFileName,
                            &FoundDataTableEntry->BaseDllName,
                            TRUE)) {
                            DataTableEntry = FoundDataTableEntry;
                            Status = STATUS_SUCCESS;

                            break;
                        }

                        FoundDataTableEntry = CONTAINING_RECORD(
                            FoundDataTableEntry->InLoadOrderLinks.Flink,
                            KLDR_DATA_TABLE_ENTRY,
                            InLoadOrderLinks);
                    }
                }

                if (Status < 0) {
                    if (FALSE == IsListEmpty(&RtBlock.LoadedModuleList)) {
                        FoundDataTableEntry = CONTAINING_RECORD(
                            RtBlock.LoadedModuleList.Flink,
                            KLDR_DATA_TABLE_ENTRY,
                            InLoadOrderLinks);

                        while ((u)FoundDataTableEntry != (u)&RtBlock.LoadedModuleList) {
                            if (FALSE != RtlPrefixUnicodeString(
                                &ImageFileName,
                                &FoundDataTableEntry->BaseDllName,
                                TRUE)) {
                                DataTableEntry = FoundDataTableEntry;
                                Status = STATUS_SUCCESS;

                                break;
                            }

                            FoundDataTableEntry = CONTAINING_RECORD(
                                FoundDataTableEntry->InLoadOrderLinks.Flink,
                                KLDR_DATA_TABLE_ENTRY,
                                InLoadOrderLinks);
                        }
                    }
                }

                ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);
                RtBlock.KeLeaveCriticalRegion();

                if (NT_SUCCESS(Status)) {
                    Separator += 1;
                    ProcedureName = Separator;

                    if (Separator[0] != '@') {
                        ProcedureAddress = LdrGetSymbol(
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
                            ProcedureAddress = LdrGetSymbol(
                                DataTableEntry->DllBase,
                                NULL,
                                ProcedureNumber);
                        }
                    }
                }

                RtlFreeUnicodeString(&ImageFileName);
            }

            __free(ImageName);
        }
    }

    return ProcedureAddress;
}

ptr
NTAPI
LdrGetSymbol(
    __in ptr ImageBase,
    __in_opt cptr ProcedureName,
    __in_opt u32 ProcedureNumber
)
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    u32 Size = 0;
    u32ptr NameTable = NULL;
    u16ptr OrdinalTable = NULL;
    u32ptr AddressTable = NULL;
    cptr NameTableName = NULL;
    u16 HintIndex = 0;
    ptr ProcedureAddress = NULL;

    ExportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable = (u8ptr)ImageBase + ExportDirectory->AddressOfNames;
        OrdinalTable = (u8ptr)ImageBase + ExportDirectory->AddressOfNameOrdinals;
        AddressTable = (u8ptr)ImageBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            if (ProcedureNumber >= ExportDirectory->Base &&
                ProcedureNumber < MAXSHORT) {
                ProcedureAddress = (u8ptr)ImageBase +
                    AddressTable[ProcedureNumber - ExportDirectory->Base];
            }
            else {
                for (HintIndex = 0;
                    HintIndex < ExportDirectory->NumberOfNames;
                    HintIndex++) {
                    NameTableName = (u8ptr)ImageBase + NameTable[HintIndex];

                    if (0 == _stricmp(
                        ProcedureName,
                        NameTableName)) {
                        ProcedureAddress = (u8ptr)ImageBase +
                            AddressTable[OrdinalTable[HintIndex]];
                    }
                }
            }
        }

        if ((u)ProcedureAddress >= (u)ExportDirectory &&
            (u)ProcedureAddress < (u)ExportDirectory + Size) {
            ProcedureAddress = LdrForward(ProcedureAddress);
        }
    }

    return ProcedureAddress;
}

s32
NTAPI
LdrNameToNumber(
    __in cptr String
)
{
    status Status = STATUS_SUCCESS;
    ptr Handle = NULL;
    ptr Section = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    ptr ViewBase = NULL;
    u ViewSize = 0;
    u8ptr TargetPc = NULL;
    s32 Number = -1;

    RtlInitUnicodeString(
        &ImageFileName,
        L"\\SystemRoot\\System32\\ntdll.dll");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &ImageFileName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

    Status = ZwOpenFile(
        &Handle,
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
            &Section,
            SECTION_MAP_READ | SECTION_MAP_EXECUTE,
            &ObjectAttributes,
            NULL,
            PAGE_EXECUTE,
            SEC_IMAGE,
            Handle);

        if (TRACE(Status)) {
            Status = ZwMapViewOfSection(
                Section,
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
                TargetPc = LdrGetSymbol(
                    ViewBase,
                    String,
                    0);

                if (NULL != TargetPc) {

#ifndef _WIN64
                    Number = __rds32(TargetPc + 1);
#else
                    Number = __rds32(TargetPc + 4);
#endif // !_WIN64
                }

                ZwUnmapViewOfSection(NtCurrentProcess(), ViewBase);
            }

            ZwClose(Section);
        }

        ZwClose(Handle);
    }

    return Number;
}

ptr
NTAPI
LdrNameToAddress(
    __in cptr String
)
{
    ptr RoutineAddress = NULL;
    UNICODE_STRING RoutineString = { 0 };
    s32 Number = -1;
    u8ptr ControlPc = NULL;
    u8ptr TargetPc = NULL;
    u32 FirstLength = 0;
    u32 Length = 0;

    if (0 == _cmpbyte(String[0], 'Z') &&
        0 == _cmpbyte(String[1], 'w')) {
        RtlInitUnicodeString(&RoutineString, L"ZwClose");

        ControlPc = MmGetSystemRoutineAddress(&RoutineString);

        if (NULL != ControlPc) {
            Number = LdrNameToNumber("NtClose");

            RoutineAddress = ControlPc +
                RtBlock.NameInterval * (s)(LdrNameToNumber(String) - Number);
        }
    }
    else if (0 == _cmpbyte(String[0], 'N') &&
        0 == _cmpbyte(String[1], 't')) {
        Number = LdrNameToNumber(String);

#ifndef _WIN64
        RoutineAddress = UlongToPtr(RtBlock.KeServiceDescriptorTable[0].Base[Number]);
#else
        RoutineAddress = (u8ptr)RtBlock.KeServiceDescriptorTable[0].Base +
            (((s32ptr)RtBlock.KeServiceDescriptorTable[0].Base)[Number] >> 4);
#endif // !_WIN64
    }

    return RoutineAddress;
}

ptr
NTAPI
LdrLoadImport(
    __in cptr ImageName
)
{
    status Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING String = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    ptr ImageBase = NULL;

    RtlInitAnsiString(&String, ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &String,
        TRUE);

    if (TRACE(Status)) {
        Status = FindEntryForImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            ImageBase = DataTableEntry->DllBase;
        }

        RtlFreeUnicodeString(&ImageFileName);
    }

    return ImageBase;
}

void
NTAPI
LdrSnapThunk(
    __in ptr ImageBase
)
{
    status Status = STATUS_SUCCESS;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    u32 Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    cptr ImageName = NULL;
    ptr ImportBase = NULL;
    u16 Ordinal = 0;
    ptr FunctionAddress = NULL;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (u8ptr)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (u8ptr)ImageBase + ImportDirectory->FirstThunk;
            ImageName = (u8ptr)ImageBase + ImportDirectory->Name;

            ImportBase = LdrLoadImport(ImageName);

            if (NULL != ImportBase) {
                do {
                    if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                        Ordinal = (u16)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                        FunctionAddress = LdrGetSymbol(
                            ImportBase,
                            NULL,
                            Ordinal);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (u)FunctionAddress;
                        }
                        else {
                            vDbgPrint(
                                "[SHARK] import procedure ordinal@%d not found\n",
                                Ordinal);
                        }
                    }
                    else {
                        ImportByName = (u8ptr)ImageBase + OriginalThunk->u1.AddressOfData;

                        if ((0 == _cmpbyte(ImportByName->Name[0], 'Z') &&
                            0 == _cmpbyte(ImportByName->Name[1], 'w')) ||
                            (0 == _cmpbyte(ImportByName->Name[0], 'N') &&
                                0 == _cmpbyte(ImportByName->Name[1], 't'))) {
                            FunctionAddress = LdrNameToAddress(ImportByName->Name);
                        }
                        else {
                            FunctionAddress = LdrGetSymbol(
                                ImportBase,
                                ImportByName->Name,
                                0);
                        }

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (u)FunctionAddress;
                        }
                        else {
                            vDbgPrint(
                                "[SHARK] import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);
            }
            else {
                vDbgPrint(
                    "[SHARK] import %hs not found\n",
                    ImageName);
            }

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

void
NTAPI
LdrEnumerateThunk(
    __in ptr ImageBase
)
{
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    u32 Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    cptr ImportImageName = NULL;
    u16 Ordinal = 0;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (u8ptr)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (u8ptr)ImageBase + ImportDirectory->FirstThunk;
            ImportImageName = (u8ptr)ImageBase + ImportDirectory->Name;

            do {
                if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                    Ordinal = (u16)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                    vDbgPrint(
                        "[SHARK] < %p > %s @%d\n",
                        Thunk->u1.Function,
                        ImportImageName,
                        Ordinal);
                }
                else {
                    ImportByName = (u8ptr)ImageBase + OriginalThunk->u1.AddressOfData;

                    vDbgPrint(
                        "[SHARK] < %p > %s %s\n",
                        Thunk->u1.Function,
                        ImportImageName,
                        ImportByName->Name);
                }

                OriginalThunk++;
                Thunk++;
            } while (OriginalThunk->u1.Function);

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

void
NTAPI
LdrReplaceThunk(
    __in ptr ImageBase,
    __in_opt cptr ImageName,
    __in_opt cptr ProcedureName,
    __in_opt u32 ProcedureNumber,
    __in ptr ProcedureAddress
)
{
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    u32 Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    cptr ImportImageName = NULL;
    u16 Ordinal = 0;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (u8ptr)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (u8ptr)ImageBase + ImportDirectory->FirstThunk;
            ImportImageName = (u8ptr)ImageBase + ImportDirectory->Name;

            if (NULL != ImageName) {
                if (0 != _stricmp(ImportImageName, ImageName)) {
                    ImportDirectory++;

                    continue;
                }
            }

            do {
                if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                    Ordinal = (u16)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                    if (0 != ProcedureNumber &&
                        Ordinal == ProcedureNumber) {
                        MapLockedCopyInstruction(
                            &Thunk->u1.Function, &ProcedureAddress, sizeof(ptr));
                    }
                }
                else {
                    ImportByName = (u8ptr)ImageBase + OriginalThunk->u1.AddressOfData;

                    if (NULL != ProcedureName &&
                        0 == _stricmp(
                            ImportByName->Name,
                            ProcedureName)) {
                        MapLockedCopyInstruction(
                            &Thunk->u1.Function, &ProcedureAddress, sizeof(ptr));
                    }
                }

                OriginalThunk++;
                Thunk++;
            } while (OriginalThunk->u1.Function);

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

ptr
NTAPI
LdrAllocate(
    __in ptr ViewBase
)
{
    ptr ImageBase = NULL;
    u32 SizeOfEntry = 0;

    SizeOfEntry = LdrGetSize(ViewBase) + PAGE_SIZE;

    ImageBase = __malloc(SizeOfEntry);

    if (NULL != ImageBase) {
        ImageBase = (u8ptr)ImageBase + PAGE_SIZE;
    }

    return ImageBase;
}

u32
NTAPI
LdrMakeProtection(
    __in PIMAGE_SECTION_HEADER NtSection
)
{
    u8 Protection[] = {
        PAGE_NOACCESS,
        PAGE_EXECUTE,
        PAGE_READONLY,
        PAGE_EXECUTE_READ,
        PAGE_READWRITE,
        PAGE_EXECUTE_READWRITE,
        PAGE_READWRITE,
        PAGE_EXECUTE_READWRITE
    };

    return Protection[NtSection->Characteristics >> 29];
}

s
NTAPI
LdrSetImageBase(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    s Diff = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Diff = (s)ImageBase
                - (s)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;

            ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase =
                (u)ImageBase;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Diff = (s64)ImageBase
                - (s64)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;

            ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase =
                (u64)ImageBase;
        }
    }

    return Diff;
}

ptr
NTAPI
LdrMapSection(
    __in ptr ViewBase,
    __in u32 Flags
)
{
    ptr ImageBase = NULL;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    s Diff = 0;
    u Index = 0;
    u32 SizeToLock = 0;

    NtHeaders = RtlImageNtHeader(ViewBase);

    if (NULL != NtHeaders) {
        ImageBase = LdrAllocate(ViewBase);

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
                        (u8ptr)ImageBase + NtSection[Index].VirtualAddress,
                        (u8ptr)ViewBase
                        + (LDRP_REDIRECTED == (Flags & LDRP_REDIRECTED) ?
                            NtSection[Index].VirtualAddress : NtSection[Index].PointerToRawData),
                        SizeToLock);
                }
            }

            Diff = LdrSetImageBase(ImageBase);

            if (0 != Diff) {
                LdrRelocImage(ImageBase, Diff);
            }

            LdrSnapThunk(ImageBase);
        }
    }

    return ImageBase;
}

PKLDR_DATA_TABLE_ENTRY
NTAPI
LdrLoad(
    __in ptr ViewBase,
    __in wcptr ImageName,
    __in u32 Flags
)
{
    status Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING ImageFileName = { 0 };
    ptr ImageBase = NULL;
    wcptr BaseName = NULL;

    BaseName = wcsrchr(ImageName, L'\\');

    if (NULL == BaseName) {
        BaseName = ImageName;
    }
    else {
        BaseName++;
    }

    RtlInitUnicodeString(&ImageFileName, BaseName);

    Status = FindEntryForImage(
        &ImageFileName,
        &DataTableEntry);

    if (NT_SUCCESS(Status)) {
        DataTableEntry->LoadCount++;
    }
    else {
        ImageBase = LdrMapSection(ViewBase, Flags);

        if (NULL != ImageBase) {
            DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)
                ((u8ptr)ImageBase - PAGE_SIZE);

            RtlZeroMemory(
                DataTableEntry,
                sizeof(KLDR_DATA_TABLE_ENTRY) +
                MAXIMUM_FILENAME_LENGTH * sizeof(wc) * 2);

            DataTableEntry->DllBase = ImageBase;
            DataTableEntry->SizeOfImage = LdrGetSize(ImageBase);
            DataTableEntry->EntryPoint = LdrGetEntryPoint(ImageBase);
            DataTableEntry->Flags = Flags & ~LDRP_ENTRY_INSERTED;
            DataTableEntry->LoadCount = 0;

            DataTableEntry->FullDllName.Buffer = DataTableEntry + 1;

            DataTableEntry->FullDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(wc);

            wcscpy(DataTableEntry->FullDllName.Buffer, SystemRootDirectory);
            wcscat(DataTableEntry->FullDllName.Buffer, BaseName);

            DataTableEntry->FullDllName.Length =
                wcslen(DataTableEntry->FullDllName.Buffer) * sizeof(wc);

            DataTableEntry->BaseDllName.Buffer =
                DataTableEntry->FullDllName.Buffer + MAXIMUM_FILENAME_LENGTH;

            DataTableEntry->BaseDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(wc);

            wcscpy(DataTableEntry->BaseDllName.Buffer, BaseName);

            DataTableEntry->BaseDllName.Length =
                wcslen(DataTableEntry->BaseDllName.Buffer) * sizeof(wc);

            if (LDRP_ENTRY_INSERTED == (Flags & LDRP_ENTRY_INSERTED)) {
                if (CmdPgClear ==
                    (RtBlock.Operation & CmdPgClear)) {
                    CaptureImageExceptionValues(
                        DataTableEntry->DllBase,
                        &DataTableEntry->ExceptionTable,
                        &DataTableEntry->ExceptionTableSize);

#ifdef _WIN64
                    InsertInvertedFunctionTable(
                        DataTableEntry->DllBase,
                        DataTableEntry->SizeOfImage);
#endif // _WIN64
                }

                RtBlock.KeEnterCriticalRegion();
                ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);

                InsertTailList(
                    &RtBlock.LoadedModuleList,
                    &DataTableEntry->InLoadOrderLinks);

                ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);
                RtBlock.KeLeaveCriticalRegion();

                DataTableEntry->LoadCount++;
            }
        }
    }

    return DataTableEntry;
}

void
NTAPI
LdrUnload(
    __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
)
{
    status Status = STATUS_SUCCESS;

    DataTableEntry->LoadCount--;

    if (0 == DataTableEntry->LoadCount) {
        RtBlock.KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);

        RemoveEntryList(&DataTableEntry->InLoadOrderLinks);

        ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);
        RtBlock.KeLeaveCriticalRegion();

        if (CmdPgClear ==
            (RtBlock.Operation & CmdPgClear)) {
#ifdef _WIN64
            RemoveInvertedFunctionTable(DataTableEntry->DllBase);
#endif // _WIN64
        }

        __free(DataTableEntry);
    }
}

void
NTAPI
DumpImageWorker(
    __in ptr ImageBase,
    __in u32 SizeOfImage,
    __in PUNICODE_STRING ImageFIleName
)
{
    status Status = STATUS_SUCCESS;
    ptr FileHandle = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    u Index = 0;
    u32 SizeToLock = 0;
    GUID Guid = { 0 };
    wc FilePathBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    wc FileBaseBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    u32 FileBaseNumberOfElements = 0;
    wc FileExtBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    u32 FileExtNumberOfElements = 0;
    LARGE_INTEGER ByteOffset = { 0 };
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        Status = ExUuidCreate(&Guid);

        if (TRACE(Status)) {
            Index = ImageFIleName->Length / sizeof(wc);

            do {
                Index--;

                if (L'.' == ImageFIleName->Buffer[Index]) {
                    FileExtNumberOfElements =
                        ImageFIleName->Length - Index * sizeof(wc);

                    RtlCopyMemory(
                        FileExtBuffer,
                        &ImageFIleName->Buffer[Index],
                        FileExtNumberOfElements);

                    break;
                }
            } while (Index > 0);

            Index = ImageFIleName->Length / sizeof(wc);

            do {
                Index--;

                if (L'\\' == ImageFIleName->Buffer[Index]) {
                    FileBaseNumberOfElements =
                        ImageFIleName->Length - Index * sizeof(wc);

                    RtlCopyMemory(
                        FileBaseBuffer,
                        &ImageFIleName->Buffer[Index],
                        FileBaseNumberOfElements);

                    break;
                }
            } while (Index > 0);

            swprintf(
                FilePathBuffer,
                L"\\??\\c:");

            RtlInitUnicodeString(&FilePath, FilePathBuffer);

            RtlCopyMemory(
                (u8ptr)FilePath.Buffer + FilePath.Length,
                FileBaseBuffer,
                FileBaseNumberOfElements - FileExtNumberOfElements);

            RtlInitUnicodeString(&FilePath, FilePathBuffer);

            swprintf(
                (u8ptr)FilePath.Buffer + FilePath.Length,
                L".{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
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

            RtlCopyMemory(
                (u8ptr)FilePath.Buffer + FilePath.Length,
                FileExtBuffer,
                FileExtNumberOfElements);

            RtlInitUnicodeString(&FilePath, FilePathBuffer);

            for (Index = 0;
                Index < (FilePath.Length / sizeof(wc));
                Index++) {
                FilePath.Buffer[Index] = towlower(FilePath.Buffer[Index]);
            }

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
                    ImageBase,
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

                        PointerPte =
                            GetPteAddress((u8ptr)ImageBase +
                                NtSection[Index].VirtualAddress);

                        PointerPde =
                            GetPdeAddress((u8ptr)ImageBase +
                                NtSection[Index].VirtualAddress);

                        if (0 != PointerPde->u.Hard.Valid) {
                            if (0 != PointerPde->u.Hard.LargePage ||
                                (0 == PointerPde->u.Hard.LargePage && 0 != PointerPte->u.Hard.Valid)) {
                                TRACE(ZwWriteFile(
                                    FileHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    (u8ptr)ImageBase + NtSection[Index].VirtualAddress,
                                    SizeToLock,
                                    &ByteOffset,
                                    NULL));
                            }
                        }
                    }
                }

                TRACE(ZwClose(FileHandle));

#ifdef DEBUG
                vDbgPrint(
                    "[SHARK] dumped < %p - %08x > %wZ\n",
                    ImageBase,
                    SizeOfImage,
                    &FilePath);
#endif // DEBUG
            }
        }
    }
}

status
NTAPI
DumpFileWorker(
    __in PUNICODE_STRING ImageFIleName
)
{
    status Status = STATUS_SUCCESS;
    ptr SourceFileHandle = NULL;
    ptr DestinationFileHandle = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    FILE_STANDARD_INFORMATION StandardInformation = { 0 };
    LARGE_INTEGER ByteOffset = { 0 };
    ptr Buffer = NULL;
    GUID Guid = { 0 };
    wc FilePathBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    wc FileBaseBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    u32 FileBaseNumberOfElements = 0;
    wc FileExtBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };
    u32 FileExtNumberOfElements = 0;
    u32 Index = 0;

    InitializeObjectAttributes(
        &ObjectAttributes,
        ImageFIleName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
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
            Buffer =
                __malloc(StandardInformation.EndOfFile.LowPart);

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
                        Index = ImageFIleName->Length / sizeof(wc);

                        do {
                            Index--;

                            if (L'.' == ImageFIleName->Buffer[Index]) {
                                FileExtNumberOfElements =
                                    ImageFIleName->Length - Index * sizeof(wc);

                                RtlCopyMemory(
                                    FileExtBuffer,
                                    &ImageFIleName->Buffer[Index],
                                    FileExtNumberOfElements);

                                break;
                            }
                        } while (Index > 0);

                        Index = ImageFIleName->Length / sizeof(wc);

                        do {
                            Index--;

                            if (L'\\' == ImageFIleName->Buffer[Index]) {
                                FileBaseNumberOfElements =
                                    ImageFIleName->Length - Index * sizeof(wc);

                                RtlCopyMemory(
                                    FileBaseBuffer,
                                    &ImageFIleName->Buffer[Index],
                                    FileBaseNumberOfElements);

                                break;
                            }
                        } while (Index > 0);

                        swprintf(
                            FilePathBuffer,
                            L"\\??\\c:");

                        RtlInitUnicodeString(&FilePath, FilePathBuffer);

                        RtlCopyMemory(
                            (u8ptr)FilePath.Buffer + FilePath.Length,
                            FileBaseBuffer,
                            FileBaseNumberOfElements - FileExtNumberOfElements);

                        RtlInitUnicodeString(&FilePath, FilePathBuffer);

                        swprintf(
                            (u8ptr)FilePath.Buffer + FilePath.Length,
                            L".{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
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

                        RtlCopyMemory(
                            (u8ptr)FilePath.Buffer + FilePath.Length,
                            FileExtBuffer,
                            FileExtNumberOfElements);

                        RtlInitUnicodeString(&FilePath, FilePathBuffer);

                        for (Index = 0;
                            Index < (FilePath.Length / sizeof(wc));
                            Index++) {
                            FilePath.Buffer[Index] = towlower(FilePath.Buffer[Index]);
                        }

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

#ifdef DEBUG
                            vDbgPrint(
                                "[SHARK] dumped %wZ to %wZ\n",
                                ImageFIleName,
                                &FilePath);
#endif // DEBUG
                        }
                    }
                }

                __free(Buffer);
            }
        }

        ZwClose(SourceFileHandle);
    }

    return Status;
}
