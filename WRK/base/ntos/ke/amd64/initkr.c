/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    initkr.c

Abstract:

    This module contains the code to initialize the kernel data structures
    and to initialize the idle thread, its process, the processor control
    block, and the processor control region.

--*/

#include "ki.h"
#include <kddll.h>

//
// Define default profile IRQL level.
//

KIRQL KiProfileIrql = PROFILE_LEVEL;

//
// Define the system cache flush size.
//

UCHAR KiCFlushSize = 0;

//
// Define the APIC mask, the number of logical processors per physical
// pyhsical processor, and the number of physical processors.
//

ULONG KiApicMask;
ULONG KiLogicalProcessors;
ULONG KiPhysicalProcessors;

//
// Define last branch control register MSR.
//

ULONG KeLastBranchMSR = 0;

//
// Define the MxCsr mask.
//

ULONG KiMxCsrMask = 0xFFBF;

//
// Define the prefetch retry flag. Each processor that requires prefetch
// retry will set bit 0 of prefetch retry. After all processors have been
// started, bit 7 of prefetch retry is cleared.
//
// If the end result is zero, then no processors require prefetch retry.
//

UCHAR KiPrefetchRetry = 0x80;

//
// Define the interrupt initialization data.
//
// Entries in the interrupt table must be in ascending vector # order.
//

typedef
VOID
(*KI_INTERRUPT_HANDLER) (
    VOID
    );

typedef struct _KI_INTINIT_REC {
    UCHAR Vector;
    UCHAR Dpl;
    UCHAR IstIndex;
    KI_INTERRUPT_HANDLER Handler;
} KI_INTINIT_REC, *PKI_INTINIT_REC;

#pragma data_seg("INITDATA")

KI_INTINIT_REC KiInterruptInitTable[] = {
    {0,  0, 0,             KiDivideErrorFault},
    {1,  0, 0,             KiDebugTrapOrFault},
    {2,  0, TSS_IST_NMI,   KiNmiInterrupt},
    {3,  3, 0,             KiBreakpointTrap},
    {4,  3, 0,             KiOverflowTrap},
    {5,  0, 0,             KiBoundFault},
    {6,  0, 0,             KiInvalidOpcodeFault},
    {7,  0, 0,             KiNpxNotAvailableFault},
    {8,  0, TSS_IST_PANIC, KiDoubleFaultAbort},
    {9,  0, 0,             KiNpxSegmentOverrunAbort},
    {10, 0, 0,             KiInvalidTssFault},
    {11, 0, 0,             KiSegmentNotPresentFault},
    {12, 0, 0,             KiStackFault},
    {13, 0, 0,             KiGeneralProtectionFault},
    {14, 0, 0,             KiPageFault},
    {16, 0, 0,             KiFloatingErrorFault},
    {17, 0, 0,             KiAlignmentFault},
    {18, 0, TSS_IST_MCA,   KiMcheckAbort},
    {19, 0, 0,             KiXmmException},
    {31, 0, 0,             KiApcInterrupt},
    {44, 3, 0,             KiRaiseAssertion},
    {45, 3, 0,             KiDebugServiceTrap},
    {47, 0, 0,             KiDpcInterrupt},
    {225, 0, 0,            KiIpiInterrupt},
    {0,  0, 0,             NULL}
};

#pragma data_seg()

//
// Define the unexpected interrupt array.
//

#pragma data_seg("RWEXEC")

UNEXPECTED_INTERRUPT KxUnexpectedInterrupt0[256];

#pragma data_seg()

//
// Define macro to initialize an IDT entry.
//
// KiInitializeIdtEntry (
//     IN PKIDTENTRY64 Entry,
//     IN PVOID Address,
//     IN USHORT Level
//     )
//
// Arguments:
//
//     Entry - Supplies a pointer to an IDT entry.
//
//     Address - Supplies the address of the vector routine.
//
//     Dpl - Descriptor privilege level.
//
//     Ist - Interrupt stack index.
//

#define KiInitializeIdtEntry(Entry, Address, Level, Index)                  \
    (Entry)->OffsetLow = (USHORT)((ULONG64)(Address));                      \
    (Entry)->Selector = KGDT64_R0_CODE;                                     \
    (Entry)->IstIndex = Index;                                              \
    (Entry)->Type = 0xe;                                                    \
    (Entry)->Dpl = (Level);                                                 \
    (Entry)->Present = 1;                                                   \
    (Entry)->OffsetMiddle = (USHORT)((ULONG64)(Address) >> 16);             \
    (Entry)->OffsetHigh = (ULONG)((ULONG64)(Address) >> 32)                 \

//
// Define forward referenced prototypes.
//

ULONG
KiFatalFilter (
    IN ULONG Code,
    IN PEXCEPTION_POINTERS Pointers
    );

VOID
KiSetCacheInformation (
    VOID
    );

VOID
KiSetCacheInformationAmd (
    VOID
    );
VOID
KiSetCacheInformationIntel (
    VOID
    );

VOID
KiSetCpuVendor (
    VOID
    );

VOID
KiSetFeatureBits (
    IN PKPRCB Prcb
    );

VOID
KiSetProcessorType (
    VOID
    );

#pragma alloc_text(INIT, KiFatalFilter)
#pragma alloc_text(INIT, KiInitializeBootStructures)
#pragma alloc_text(INIT, KiInitializeKernel)
#pragma alloc_text(INIT, KiInitMachineDependent)
#pragma alloc_text(INIT, KiSetCacheInformation)
#pragma alloc_text(INIT, KiSetCacheInformationAmd)
#pragma alloc_text(INIT, KiSetCacheInformationIntel)
#pragma alloc_text(INIT, KiSetCpuVendor)
#pragma alloc_text(INIT, KiSetFeatureBits)
#pragma alloc_text(INIT, KiSetProcessorType)

VOID
KeCompactServiceTable (
    IN PVOID Table,
    IN ULONG Limit,
    IN BOOLEAN Win32
    )

/*++

Routine Description:

    This function compacts the specified system service table into an array
    of 32-bit displacements with the number of arguments encoded in the low
    bits of the relative address.

Arguments:

    Table - Supplies the address of a system service table.

    Limit - Supplies the number of entries in the system service table.

    Win32 - Supplies a boolean variable that signifies whether the system
        service table is the win32k table or the kernel table.

Return Value:

    None.

--*/

{

    ULONG64 Base;
    ULONG Index;
    PULONG64 Input;
    PULONG Output;

    //
    // Copy and compact the specified system servicde table into an array
    // of relative addresses.
    //

    Base = (ULONG64)Table;
    Input = (PULONG64)Table;
    Output = (PULONG)Table;
    for (Index = 0; Index < Limit; Index += 1) {
        *Output = (ULONG)(*Input - Base);
        Input += 1;
        Output += 1;
    }

    //
    // If the specified system service table in the Win32 table, then copy
    // the status translation vector.
    //

    if (Win32 == TRUE) {
        memcpy((PUCHAR)Output, (PUCHAR)Input, Limit);
    }

    return;
}

VOID
KiInitializeKernel (
    IN PKPROCESS Process,
    IN PKTHREAD Thread,
    IN PVOID IdleStack,
    IN PKPRCB Prcb,
    IN CCHAR Number,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function gains control after the system has been booted, but before
    the system has been completely initialized. Its function is to initialize
    the kernel data structures, initialize the idle thread and process objects,
    complete the initialization of the processor control block (PRCB) and
    processor control region (PCR), call the executive initialization routine,
    then return to the system startup routine. This routine is also called to
    initialize the processor specific structures when a new processor is
    brought on line.

    N.B. Kernel initialization is called with interrupts disabled at IRQL
         HIGH_LEVEL and returns with with interrupts enabled at DISPATCH_LEVEL.

Arguments:

    Process - Supplies a pointer to a control object of type process for
        the specified processor.

    Thread - Supplies a pointer to a dispatcher object of type thread for
        the specified processor.

    IdleStack - Supplies a pointer the base of the real kernel stack for
        idle thread on the specified processor.

    Prcb - Supplies a pointer to a processor control block for the specified
        processor.

    Number - Supplies the number of the processor that is being
        initialized.

    LoaderBlock - Supplies a pointer to the loader parameter block.

Return Value:

    None.


--*/

{

    ULONG ApicId;
    ULONG64 DirectoryTableBase[2];
    ULONG FeatureBits;
    LONG64 Index;
    ULONG MxCsrMask;
    PKPRCB NextPrcb;
    KIRQL OldIrql;
    PCHAR Options;
    XMM_SAVE_AREA32 XmmSaveArea;

    //
    // Set CPU vendor.
    //

    KiSetCpuVendor();

    //
    // Set processor type.
    //

    KiSetProcessorType();

    //
    // get the processor feature bits.
    //

    FeatureBits = Prcb->FeatureBits;

    //
    // Retrieve MxCsr mask, if any.
    //

    RtlZeroMemory(&XmmSaveArea, sizeof(XmmSaveArea));
    KeSaveLegacyFloatingPointState(&XmmSaveArea);

    //
    // If this is the boot processor, then enable global pages, set the page
    // attributes table, set machine check enable, set large page enable, 
    // enable debug extensions, and set multithread information. Otherwise,
    // propagate multithread information.
    //
    // N.B. This only happens on the boot processor and at a time when there
    //      can be no coherency problem. On subsequent, processors this happens
    //      during the transistion into 64-bit mode which is also at a time
    //      that there can be no coherency problems.
    //

    if (Number == 0) {

        //
        // Retrieve the loader options.
        //
        // N.B. LoadOptions was upcased by the loader.
        //

        Options = LoaderBlock->LoadOptions;

        //
        // Flush the entire TB and enable global pages.
        //
    
        KeFlushCurrentTb();
    
        //
        // Set page attributes table and flush cache.
        //
    
        KiSetPageAttributesTable();
        WritebackInvalidate();

        //
        // Parse boot options to determine desired level of no execute
        // protection.
        //
        
        SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_OPTIN;
        if (strstr(Options, "NOEXECUTE=ALWAYSON") != NULL) {
            SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_ALWAYSON;
            FeatureBits |= KF_GLOBAL_32BIT_NOEXECUTE;

        } else if (strstr(Options, "NOEXECUTE=OPTOUT") != NULL) {
            SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_OPTOUT;
            FeatureBits |= KF_GLOBAL_32BIT_NOEXECUTE;

        } else if (strstr(Options, "NOEXECUTE=OPTIN") != NULL) {
            FeatureBits |= KF_GLOBAL_32BIT_NOEXECUTE;

        } else if (strstr(Options, "NOEXECUTE=ALWAYSOFF") != NULL) {
            SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_ALWAYSOFF;
            FeatureBits |= KF_GLOBAL_32BIT_EXECUTE;

        } else if (strstr(Options, "NOEXECUTE") != NULL) {
            FeatureBits |= KF_GLOBAL_32BIT_NOEXECUTE;

        } else if (strstr(Options, "EXECUTE") != NULL) {
            SharedUserData->NXSupportPolicy = NX_SUPPORT_POLICY_ALWAYSOFF;
            FeatureBits |= KF_GLOBAL_32BIT_EXECUTE;
        }

        //
        // If no execute protection is supported, then turn on no execute
        // protection for memory management. Otherwise, make sure the feature
        // bits reflect that no execute is not supported no matter what boot
        // options were specified.
        //
        // N.B. No execute protection is always enabled during processor
        //      initialization if it is present on the respective processor.
        //

        if ((FeatureBits & KF_NOEXECUTE) != 0) {
            MmPaeMask = 0x8000000000000000UI64;
            MmPaeErrMask = 0x8;
            SharedUserData->ProcessorFeatures[PF_NX_ENABLED] = TRUE;

        } else {
            FeatureBits &= ~KF_GLOBAL_32BIT_NOEXECUTE;
            FeatureBits |= KF_GLOBAL_32BIT_EXECUTE;
        }

        //
        // Set debugger extension and large page enable.
        //

        WriteCR4(ReadCR4() | CR4_DE | CR4_PSE);

        //
        // Flush the entire TB.
        //

        KeFlushCurrentTb();

        //
        // Set the multithread processor set and the multithread set master
        // for the boot processor, and set the number of logical processors
        // per physical processor.
        //

        Prcb->MultiThreadProcessorSet = Prcb->SetMember;
        Prcb->MultiThreadSetMaster = Prcb;

        //
        // Derive the appropriate MxCsr mask for processor zero.
        //

        if (XmmSaveArea.MxCsr_Mask != 0) {
            KiMxCsrMask = XmmSaveArea.MxCsr_Mask;

        } else {
            KiMxCsrMask = 0x0000FFBF;
        }

        //
        // Compact the system service table.
        //

        KeCompactServiceTable(&KiServiceTable[0], KiServiceLimit, FALSE);

    } else {

        //
        // If the system is not a multithread system, then initialize the
        // multithread processor set and multithread set master. Otherwise,
        // propagate multithread set information.
        //

        if (KiLogicalProcessors == 1) {
            Prcb->MultiThreadProcessorSet = Prcb->SetMember;
            Prcb->MultiThreadSetMaster = Prcb;
            KiPhysicalProcessors += 1;

        } else {
            ApicId = Prcb->InitialApicId & KiApicMask;
            for (Index = 0; Index < (LONG)KeNumberProcessors; Index += 1) {
                NextPrcb = KiProcessorBlock[Index];
                if ((NextPrcb->InitialApicId & KiApicMask) == ApicId) {
                    NextPrcb->MultiThreadProcessorSet |= Prcb->SetMember;
                    Prcb->MultiThreadSetMaster = NextPrcb->MultiThreadSetMaster;
                }
            }

            if (Prcb->MultiThreadSetMaster == NULL) {
                Prcb->MultiThreadProcessorSet = Prcb->SetMember;
                Prcb->MultiThreadSetMaster = Prcb;
                KiPhysicalProcessors += 1;

            } else {
                NextPrcb = Prcb->MultiThreadSetMaster;
                Prcb->MultiThreadProcessorSet = NextPrcb->MultiThreadProcessorSet;
            }
        }
    }

    //
    // set processor cache size information.
    //

    KiSetCacheInformation();

    //
    // Initialize power state information.
    //

    PoInitializePrcb(Prcb);

    //
    // If the initial processor is being initialized, then initialize the
    // per system data structures. Otherwise, check for a valid system.
    //

    if (Number == 0) {

        //
        // Set the default node for the boot processor.
        //

        KeNodeBlock[0] = &KiNode0;
        Prcb->ParentNode = KeNodeBlock[0];
        KiNode0.ProcessorMask = 1;
        KiNode0.NodeNumber = 0;

        //
        // Initialize the node block array with pointers to temporary node
        // blocks to be used during initialization.
        //

#if !defined(NT_UP)

        for (Index = 1; Index < MAXIMUM_CCNUMA_NODES; Index += 1) {
            KeNodeBlock[Index] = &KiNodeInit[Index];
            KeNodeBlock[Index]->NodeNumber = (UCHAR)Index;
        }

#endif

        //
        // Set global architecture and feature information.
        //

        KeProcessorArchitecture = PROCESSOR_ARCHITECTURE_AMD64;
        KeProcessorLevel = (USHORT)Prcb->CpuType;
        KeProcessorRevision = Prcb->CpuStep;
        KeFeatureBits = FeatureBits;
        KiCFlushSize = Prcb->CFlushSize;

        //
        // Lower IRQL to APC level.
        //

        KeLowerIrql(APC_LEVEL);

        //
        // Initialize kernel internal spinlocks
        //

        KeInitializeSpinLock(&KiFreezeExecutionLock);

        //
        // Performance architecture independent initialization.
        //

        KiInitSystem();

        //
        // Initialize idle thread process object and then set:
        //
        //  1. the process quantum.
        //

        DirectoryTableBase[0] = 0;
        DirectoryTableBase[1] = 0;
        InitializeListHead(&KiProcessListHead);
        KeInitializeProcess(Process,
                            (KPRIORITY)0,
                            (KAFFINITY)(-1),
                            &DirectoryTableBase[0],
                            TRUE);

        Process->QuantumReset = MAXCHAR;

    } else {

        //
        // Derive the appropriate MxCsr mask for this processor.
        //

        if (XmmSaveArea.MxCsr_Mask != 0) {
            MxCsrMask = XmmSaveArea.MxCsr_Mask;

        } else {
            MxCsrMask = 0x0000FFBF;
        }

        //
        // If the CPU feature bits are not identical or the number of logical
        // processors per physical processors are not identical, then bugcheck.
        //

        if ((FeatureBits != (KeFeatureBits & ~(KF_GLOBAL_32BIT_NOEXECUTE | KF_GLOBAL_32BIT_EXECUTE))) ||
            (MxCsrMask != KiMxCsrMask) ||
            (KiCFlushSize != Prcb->CFlushSize) || 
            (KiLogicalProcessors != Prcb->LogicalProcessorsPerPhysicalProcessor)) {

            KeBugCheckEx(MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED,
                         (ULONG64)FeatureBits,
                         (ULONG64)KeFeatureBits,
                         0,
                         0);
        }

        //
        // Lower IRQL to DISPATCH level.
        //

        KeLowerIrql(DISPATCH_LEVEL);
    }

    //
    // Set global processor features.
    //

    SharedUserData->TestRetInstruction = 0xc3;
    SharedUserData->ProcessorFeatures[PF_COMPARE_EXCHANGE_DOUBLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_MMX_INSTRUCTIONS_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_XMMI_INSTRUCTIONS_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_RDTSC_INSTRUCTION_AVAILABLE] = TRUE;
    SharedUserData->ProcessorFeatures[PF_PAE_ENABLED] = TRUE;
    SharedUserData->ProcessorFeatures[PF_XMMI64_INSTRUCTIONS_AVAILABLE] = TRUE;
    if ((FeatureBits & KF_3DNOW) != 0) {
        SharedUserData->ProcessorFeatures[PF_3DNOW_INSTRUCTIONS_AVAILABLE] = TRUE;
    }

    //
    // Initialize idle thread object and then set:
    //
    //      1. the next processor number to the specified processor.
    //      2. the thread priority to the highest possible value.
    //      3. the state of the thread to running.
    //      4. the thread affinity to the specified processor.
    //      5. the specified member in the process active processors set.
    //

    KeInitializeThread(Thread,
                       (PVOID)((ULONG64)IdleStack),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       Process);

    Thread->NextProcessor = Number;
    Thread->Priority = HIGH_PRIORITY;
    Thread->State = Running;
    Thread->Affinity = AFFINITY_MASK(Number);
    Thread->WaitIrql = DISPATCH_LEVEL;
    Process->ActiveProcessors |= AFFINITY_MASK(Number);

    //
    // Call the executive initialization routine.
    //

    try {
        ExpInitializeExecutive(Number, LoaderBlock);

    } except(KiFatalFilter(GetExceptionCode(), GetExceptionInformation())) {
        NOTHING;
    }

    //
    // If the initial processor is being initialized, then compute the timer
    // table reciprocal value, reset the PRCB values for the controllable DPC
    // behavior in order to reflect any registry overrides, and initialize the
    // global unwind history table.
    //

    if (Number == 0) {
        KiTimeIncrementReciprocal = KeComputeReciprocal((LONG)KeMaximumIncrement,
                                                        &KiTimeIncrementShiftCount);

        Prcb->MaximumDpcQueueDepth = KiMaximumDpcQueueDepth;
        Prcb->MinimumDpcRate = KiMinimumDpcRate;
        Prcb->AdjustDpcThreshold = KiAdjustDpcThreshold;
        RtlInitializeHistoryTable();
    }

    //
    // Raise IRQL to dispatch level, enable interrupts, and set the priority
    // of the idle thread to zero. This will have the effect of immediately
    // causing the phase one initialization thread to get scheduled. The idle
    // thread priority is then set of the lowest realtime priority.
    //

    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    _enable();
    KeSetPriorityThread(Thread, 0);
    Thread->Priority = LOW_REALTIME_PRIORITY;

    //
    // If the current processor is a secondary processor and a thread has
    // not been selected for execution, then set the appropriate bit in the
    // idle summary.
    //

#if !defined(NT_UP)

    OldIrql = KeRaiseIrqlToSynchLevel();
    KiAcquirePrcbLock(Prcb);
    if ((Number != 0) && (Prcb->NextThread == NULL)) {
        KiIdleSummary |= AFFINITY_MASK(Number);
    }

    KiReleasePrcbLock(Prcb);
    KeLowerIrql(OldIrql);

#endif

    //
    // Signal that this processor has completed its initialization.
    //

    LoaderBlock->Prcb = (ULONG64)NULL;
    return;
}

VOID
KiInitializeBootStructures (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This function initializes the boot structures for a processor. It is only
    called by the system start up code. Certain fields in the boot structures
    have already been initialized. In particular:

    The address and limit of the GDT and IDT in the PCR.

    The address of the system TSS in the PCR.

    The processor number in the PCR.

    The special registers in the PRCB.

    N.B. All uninitialized fields are zero.

Arguments:

    LoaderBlock - Supplies a pointer to the loader block that has been
        initialized for this processor.

Return Value:

    None.

--*/

{

    ULONG ApicMask;
    ULONG Bit;
    PKIDTENTRY64 IdtBase;
    ULONG64 Index;
    PKI_INTINIT_REC IntInitRec;
    LONG64 JumpOffset;
    PKPCR Pcr = KeGetPcr();
    PKPRCB Prcb = KeGetCurrentPrcb();
    UCHAR Number;
    PKTHREAD Thread;
    PKTSS64 TssBase;
    PUNEXPECTED_INTERRUPT UnexpectedInterrupt;

    //
    // Initialize the PCR major and minor version numbers.
    //

    Pcr->MajorVersion = PCR_MAJOR_VERSION;
    Pcr->MinorVersion = PCR_MINOR_VERSION;

    //
    // initialize the PRCB major and minor version numbers and build type.
    //

    Prcb->MajorVersion = PRCB_MAJOR_VERSION;
    Prcb->MinorVersion =  PRCB_MINOR_VERSION;
    Prcb->BuildType = 0;

#if DBG

    Prcb->BuildType |= PRCB_BUILD_DEBUG;

#endif

#if defined(NT_UP)

    Prcb->BuildType |= PRCB_BUILD_UNIPROCESSOR;

#endif

    //
    // Initialize the PRCB set member.
    //

    Number = Pcr->Prcb.Number;
    Prcb->SetMember = AFFINITY_MASK(Number);

    //
    // If this is processor zero, then initialize the address of the system
    // process and initial thread.
    //

    if (Number == 0) {
        LoaderBlock->Process = (ULONG64)&KiInitialProcess;
        LoaderBlock->Thread = (ULONG64)&KiInitialThread;
    }

    //
    // Initialize the PRCB scheduling thread address and the thread process
    // address.
    //

    Thread = (PVOID)LoaderBlock->Thread;
    Prcb->CurrentThread = Thread;
    Prcb->NextThread = NULL;
    Prcb->IdleThread = Thread;
    Thread->ApcState.Process = (PKPROCESS)LoaderBlock->Process;
    InitializeListHead(&Thread->ApcState.ApcListHead[KernelMode]);

    //
    // Initialize the processor block address.
    //

    KiProcessorBlock[Number] = Prcb;

    //
    // Initialize the PRCB address of the DPC stack.
    //

    Prcb->DpcStack = (PVOID)LoaderBlock->KernelStack;

    //
    // If this is processor zero, then initialize the IDT according to the
    // contents of interrupt initialization table. Otherwise, initialize the
    // secondary processor clock interrupt.
    //

    IdtBase = Pcr->IdtBase;
    if (Number == 0) {

        IntInitRec = KiInterruptInitTable;
        for (Index = 0; Index < MAXIMUM_IDTVECTOR; Index += 1) {

            //
            // If the vector is found in the initialization table then
            // set up the IDT entry accordingly and advance to the next
            // entry in the initialization table.
            //
            // Otherwise set the IDT to reference the unexpected interrupt
            // handler.
            // 

            if (Index == IntInitRec->Vector) {
                KiInitializeIdtEntry(&IdtBase[Index],
                                     IntInitRec->Handler,
                                     IntInitRec->Dpl,
                                     IntInitRec->IstIndex);

                IntInitRec += 1;

            } else {

                UnexpectedInterrupt = &KxUnexpectedInterrupt0[Index];

                UnexpectedInterrupt->PushImmOp = 0x68;
                UnexpectedInterrupt->PushImm = (UCHAR)Index;
                UnexpectedInterrupt->PushRbp = 0x55;
                UnexpectedInterrupt->JmpOp = 0xe9;

                JumpOffset =
                    (LONG64)KiUnexpectedInterrupt - 
                    ((LONG64)UnexpectedInterrupt +
                     RTL_SIZEOF_THROUGH_FIELD(UNEXPECTED_INTERRUPT,JmpOffset));

                UnexpectedInterrupt->JmpOffset = (LONG)JumpOffset;

                KiInitializeIdtEntry(&IdtBase[Index],
                                     UnexpectedInterrupt,
                                     0,
                                     0);
            }
        }

    } else {
        KiInitializeIdtEntry(&IdtBase[209], &KiSecondaryClockInterrupt, 0, 0);
    }

    //
    // Initialize the system TSS I/O Map.
    //

    TssBase = Pcr->TssBase;
    TssBase->IoMapBase = KiComputeIopmOffset(FALSE);

    //
    // Initialize the system call MSRs.
    //

    WriteMSR(MSR_STAR,
             ((ULONG64)KGDT64_R0_CODE << 32) | (((ULONG64)KGDT64_R3_CMCODE | RPL_MASK) << 48));

    WriteMSR(MSR_CSTAR, (ULONG64)&KiSystemCall32);
    WriteMSR(MSR_LSTAR, (ULONG64)&KiSystemCall64);
    WriteMSR(MSR_SYSCALL_MASK, EFLAGS_SYSCALL_CLEAR);

    //
    // Set processor feature bits.
    //

    KiSetFeatureBits(Prcb);

    //
    // If this is processor zero, then set the number of logical processors
    // per physical processor, the APIC mask, and the initial number of
    // physical processors.
    //

    if (Number == 0) {
        KiLogicalProcessors = Prcb->LogicalProcessorsPerPhysicalProcessor;
        if (KiLogicalProcessors == 1) {
            ApicMask = 0xffffffff;

        } else {
            ApicMask = (KiLogicalProcessors * 2) - 1;
            KeFindFirstSetLeftMember(ApicMask, &Bit);
            ApicMask = ~((1 << Bit) - 1);
        }

        KiApicMask = ApicMask;
        KiPhysicalProcessors = 1;
    }

    Prcb->ApicMask = KiApicMask;

    //
    // initialize the per processor lock data.
    //

    KiInitSpinLocks(Prcb, Number);

    //
    // Initialize the PRCB temporary pool look aside pointers.
    //

    ExInitPoolLookasidePointers();

    //
    // Initialize the HAL for this processor.
    //

    HalInitializeProcessor(Number, LoaderBlock);

    //
    // Set the appropriate member in the active processors set.
    //

    KeActiveProcessors |= AFFINITY_MASK(Number);

    //
    // Set the number of processors based on the maximum of the current
    // number of processors and the current processor number.
    //

    if ((Number + 1) > KeNumberProcessors) {
        KeNumberProcessors = Number + 1;
    }

    //
    // Initialize the processor control state in the PRCB.
    //

    KiSaveInitialProcessorControlState(&Prcb->ProcessorState);

    return;
}

ULONG
KiFatalFilter (
    IN ULONG Code,
    IN PEXCEPTION_POINTERS Pointers
    )

/*++

Routine Description:

    This function is executed if an unhandled exception occurs during
    phase 0 initialization. Its function is to bugcheck the system
    with all the context information still on the stack.

Arguments:

    Code - Supplies the exception code.

    Pointers - Supplies a pointer to the exception information.

Return Value:

    None - There is no return from this routine even though it appears there
    is.

--*/

{

    KeBugCheckEx(PHASE0_EXCEPTION, Code, (ULONG64)Pointers, 0, 0);
}


BOOLEAN
KiInitMachineDependent (
    VOID
    )

/*++

Routine Description:

    This function initializes machine dependent data structures and hardware.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG Size;
    NTSTATUS Status;
    BOOLEAN UseFrameBufferCaching;

    //
    // Query the HAL to determine if the write combining can be used for the
    // frame buffer.
    //

    Status = HalQuerySystemInformation(HalFrameBufferCachingInformation,
                                       sizeof(BOOLEAN),
                                       &UseFrameBufferCaching,
                                       &Size);

    //
    // If the status is successful and frame buffer caching is disabled,
    // then don't enable write combining.
    //

    if (!NT_SUCCESS(Status) || (UseFrameBufferCaching != FALSE)) {
        MmEnablePAT();
    }

    //
    // Verify division errata is not present.
    //

    if (KiDivide6432(KiTestDividend, 0xCB5FA3) != 0x5EE0B7E5) {
        KeBugCheck(UNSUPPORTED_PROCESSOR);
    }

    return TRUE;
}

VOID
KiSetCacheInformation (
    VOID
    )

/*++

Routine Description:

    This function sets the current processor cache information in the PCR and
    PRCB.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG AdjustedSize;
    UCHAR Associativity;
    PCACHE_DESCRIPTOR Cache;
    ULONG i;
    PKPCR Pcr;
    PKPRCB Prcb;
    ULONG Size;

    Pcr = KeGetPcr();
    Prcb = KeGetCurrentPrcb();

    //
    // Switch on processor vendor.
    //

    switch (Prcb->CpuVendor) {
    case CPU_AMD: 
        KiSetCacheInformationAmd();
        break;

    case CPU_INTEL:
        KiSetCacheInformationIntel();
        break;

    default:
        KeBugCheck(UNSUPPORTED_PROCESSOR);
        break;
    }

    //
    // Scan through the cache descriptors initialized by the processor 
    // specific code and compute cache parameters for page coloring 
    // and largest cache line size.
    //

    AdjustedSize = 0;
    Cache = Prcb->Cache;
    Pcr->SecondLevelCacheSize = 0;

    for (i = 0; i < Prcb->CacheCount; i++) {
        if ((Cache->Level >= 2) && 
            ((Cache->Type == CacheData) || (Cache->Type == CacheUnified))) {

            Associativity = Cache->Associativity;
            if (Associativity == CACHE_FULLY_ASSOCIATIVE) {

                //
                // Temporarily preserve existing behavior of treating
                // fully associative cache as a 16 way associative
                // cache until MM interprets the cache descriptors
                // directly.
                //

                Associativity = 16;
            }

            if (Associativity != 0) {
                Size = Cache->Size/Associativity;
                if (Size > AdjustedSize) {
                    AdjustedSize = Size;
                    Pcr->SecondLevelCacheSize = Cache->Size;
                    Pcr->SecondLevelCacheAssociativity = Associativity;
                }
            }

            //
            // If the line size is greater then the current largest line
            // size, then set the new largest line size.
            //

            if (Cache->LineSize > KeLargestCacheLine) {
                KeLargestCacheLine = Cache->LineSize;
            }
        }
        Cache++;
    }
    return;
}

VOID
KiSetCacheInformationAmd (
    VOID
    )

/*++

Routine Description:

    This function extracts the cache hierarchy information of an AMD
    processor and initializes cache descriptors in the PRCB.

Arguments:

    None.

Return Value:

    None.

--*/

{

    UCHAR Associativity;
    PCACHE_DESCRIPTOR Cache;
    CPU_INFO CpuInfo;
    ULONG Index;
    UCHAR Level;
    USHORT LineSize;
    AMD_L1_CACHE_INFO L1Info;
    AMD_L2_CACHE_INFO L2Info;
    PKPRCB Prcb;
    ULONG Size;
    PROCESSOR_CACHE_TYPE Type;

    //
    // Iterate through the cache levels and generate the cache information
    // for each level.
    //

    Prcb = KeGetCurrentPrcb();
    Prcb->CacheCount = 0;
    Cache = &Prcb->Cache[0];
    Index = 0;
    do {

        //
        // Switch on the cache level.
        //

        switch (Index) {
        
            //
            // Get L1 instruction and data cache information.
            //

        case 0:
        case 1:
            KiCpuId(0x80000005, 0, &CpuInfo);
            Level = 1;
            if (Index == 0) {
                L1Info.Ulong = CpuInfo.Ecx;
                Type = CacheData;

            } else {
                L1Info.Ulong = CpuInfo.Edx;
                Type = CacheInstruction;
            }

            LineSize = L1Info.LineSize;
            Size = L1Info.Size << 10;
            Associativity = L1Info.Associativity;
            break;

            //
            // Get L2 unified cache information.
            //

        case 2:
            KiCpuId(0x80000006, 0, &CpuInfo);
            Level = 2;
            Type = CacheUnified;
            L2Info.Ulong = CpuInfo.Ecx;
            LineSize = L2Info.LineSize;
            Size = L2Info.Size << 10;

            //
            // Switch on associativity.
            //

            switch (L2Info.Associativity) {

                //
                // L2 cache is not present
                //

            case 0x0:
                continue;

                //
                // L2 cache is two way associative.
                //

            case 0x2:
                Associativity = 2;
                break;

                //
                // L2 cache is four way associative.
                //

            case 0x4:
                Associativity = 4;
                break;

                //
                // L2 cache is eight way associative.
                //

            case 0x6:
                Associativity = 8;
                break;

                //
                // L2 cache is sixteen way associative.
                //

            case 0x8:
                Associativity = 16;
                break;

                //
                // L2 cache is fully associative.
                //

            case 0xf:
                Associativity = CACHE_FULLY_ASSOCIATIVE;
                break;

                //
                // Direct mapped.
                //

            default:
                Associativity = 1;
            }

            break;

            //
            // L3 cache is undefined.
            //

        default:
            continue;
        }
            
        //
        // Generate a cache descriptor for the current level in the PRCB.
        //

        Cache->Type = Type;
        Cache->Level = Level;
        Cache->Associativity = Associativity;
        Cache->LineSize = LineSize;
        Cache->Size = Size;
        Cache += 1;
        Prcb->CacheCount += 1;
        Index += 1;
    } while (Index < 3);
    return;
}

VOID
KiSetCacheInformationIntel (
    VOID
    )
/*++

Routine Description:

    This function extracts the cache hierarchy information of an Intel 
    processor and initializes cache descriptors in the PRCB.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PCACHE_DESCRIPTOR Cache;
    CPU_INFO CpuInfo;
    INTEL_CACHE_INFO_EAX CacheInfoEax;
    INTEL_CACHE_INFO_EBX CacheInfoEbx;
    ULONG Index;
    PKPRCB Prcb;
    ULONGLONG CacheSize;

    Prcb = KeGetCurrentPrcb();
    Prcb->CacheCount = 0;

    //
    // Check for the availability of deterministic cache parameter mechanism.
    // This cpuid function should be supported on all versions EM64T family
    // processors. 
    //

    KiCpuId(0, 0, &CpuInfo);
    if (CpuInfo.Eax < 3 || CpuInfo.Eax >= 0x80000000) {
        ASSERT(FALSE);
        return;
    }

    Cache = &Prcb->Cache[0];
    Index = 0;

    //
    // Enumerate the details of cache hierarchy by executing CPUID 
    // instruction repeatedly until no more cache information to be
    // returned.
    //
    
    do {
        KiCpuId(4, Index, &CpuInfo);
        Index += 1;
        CacheInfoEax.Ulong = CpuInfo.Eax;
        CacheInfoEbx.Ulong = CpuInfo.Ebx;

        if (CacheInfoEax.Type == IntelCacheNull) {
            break;
        }

        switch (CacheInfoEax.Type) {
        case IntelCacheData:
            Cache->Type = CacheData;
            break;
        case IntelCacheInstruction:
            Cache->Type = CacheInstruction;
            break;
        case IntelCacheUnified:
            Cache->Type = CacheUnified;
            break;
        case IntelCacheTrace:
            Cache->Type = CacheTrace;
            break;
        default:
            continue;
        }

        if (CacheInfoEax.FullyAssociative) {
            Cache->Associativity = CACHE_FULLY_ASSOCIATIVE;

        } else {
            Cache->Associativity = (UCHAR) CacheInfoEbx.Associativity + 1;
        }

        Cache->Level = (UCHAR) CacheInfoEax.Level;
        Cache->LineSize = (USHORT) (CacheInfoEbx.LineSize + 1);

        //
        // Cache size = Ways x Partitions x LineSize x Sets. 
        //
        // N.B. For fully-associative cache, the "Sets" returned 
        // from cpuid is actually the number of entries, not the
        // "Ways". Therefore the formula of evaluating the cache 
        // size below will still hold.
        //

        CacheSize = (CacheInfoEbx.Associativity + 1) *
                    (CacheInfoEbx.Partitions + 1) *
                    (CacheInfoEbx.LineSize + 1) * 
                    (CpuInfo.Ecx + 1);

        Cache->Size = (ULONG) CacheSize;
        ASSERT(CacheSize == Cache->Size);
        Cache++;
        Prcb->CacheCount++;
    } while (Prcb->CacheCount < RTL_NUMBER_OF(Prcb->Cache));
    return;
}

VOID
KiSetCpuVendor (
    VOID
    )

/*++

Routine Description:

    Set the current processor cpu vendor information in the PRCB.

Arguments:

    None.

Return Value:

    None.

--*/

{

    CPU_INFO CpuInformation;
    PKPRCB Prcb = KeGetCurrentPrcb();
    ULONG Temp;

    //
    // Get the CPU vendor string.
    //

    KiCpuId(0, 0, &CpuInformation);

    //
    // Copy vendor string to PRCB.
    //

    Temp = CpuInformation.Ecx;
    CpuInformation.Ecx = CpuInformation.Edx;
    CpuInformation.Edx = Temp;
    RtlCopyMemory(Prcb->VendorString,
                  &CpuInformation.Ebx,
                  sizeof(Prcb->VendorString) - 1);

    Prcb->VendorString[sizeof(Prcb->VendorString) - 1] = '\0';

    //
    // Check to determine the processor vendor.
    //

    if (strncmp((PCHAR)&CpuInformation.Ebx, "AuthenticAMD", 12) == 0) {
        Prcb->CpuVendor = CPU_AMD;

    } else if (strncmp((PCHAR)&CpuInformation.Ebx, "GenuineIntel", 12) == 0) {
        Prcb->CpuVendor = CPU_INTEL;

    } else {
        KeBugCheck(UNSUPPORTED_PROCESSOR);
    }

    return;
}

VOID
KiSetFeatureBits (
    IN PKPRCB Prcb
    )

/*++

Routine Description:

    Set the current processor feature bits in the PRCB.

Arguments:

    Prcb - Supplies a pointer to the current processor block.

Return Value:

    None.

--*/

{

    CPU_INFO InformationExtended;
    CPU_INFO InformationStandard;
    ULONG FeatureBits;

    //
    // Get CPU feature information.
    //

    KiCpuId(1, 0, &InformationStandard);
    KiCpuId(0x80000001, 0, &InformationExtended);

    //
    // Set the initial APIC ID and cache flush size.
    //

    Prcb->InitialApicId = (UCHAR)(InformationStandard.Ebx >> 24);
    Prcb->CFlushSize = ((UCHAR)(InformationStandard.Ebx >> 8)) << 3;

    //
    // If the required features are not present, then bugcheck.
    //

    if (((InformationStandard.Edx & HF_REQUIRED) != HF_REQUIRED) ||
        ((InformationExtended.Edx & XHF_SYSCALL) == 0)) {

        KeBugCheckEx(UNSUPPORTED_PROCESSOR, InformationStandard.Edx, 0, 0, 0);
    }

    FeatureBits = KF_REQUIRED;
    if ((InformationStandard.Edx & HF_DS) != 0) {
        FeatureBits |= KF_DTS;
    }

    //
    // Check the extended feature bits.
    //

    if ((InformationExtended.Edx & XHF_3DNOW) != 0) {
        FeatureBits |= KF_3DNOW;
    }

    //
    // Check for no execute protection.
    //
    // If the processor being initialized is the boot processor or the boot
    // processor is NX capable, then set the no execute feature bit for this
    // processor as appropriate.
    //

    if ((Prcb->Number == 0) ||
        ((KeFeatureBits & KF_NOEXECUTE) != 0)) {

        if ((InformationExtended.Edx & XHF_NOEXECUTE) != 0) {
            FeatureBits |= KF_NOEXECUTE;
        }
    }

    //
    // Check for fast floating/save restore and, if present, enable for the
    // current processor.
    //

    if ((InformationExtended.Edx & XHF_FFXSR) != 0) {
        WriteMSR(MSR_EFER, ReadMSR(MSR_EFER) | MSR_FFXSR);
    }

    //
    // Set number of logical processors per physical processor.
    //

    Prcb->LogicalProcessorsPerPhysicalProcessor = 1;
    if ((InformationStandard.Edx & HF_SMT) != 0) {
        Prcb->LogicalProcessorsPerPhysicalProcessor =
                                        (UCHAR)(InformationStandard.Ebx >> 16);
    }

    Prcb->FeatureBits = FeatureBits;
    return;
}              

VOID
KiSetProcessorType (
    VOID
    )

/*++

Routine Description:

    This function sets the current processor family and stepping in the PRCB.

Arguments:

    None.

Return Value:

    None.

--*/

{

    CPU_INFO CpuInformation;
    ULONG Family;
    ULONG Model;
    PKPRCB Prcb = KeGetCurrentPrcb();
    ULONG Stepping;
    union {
        struct {
            ULONG Stepping : 4;
            ULONG Model : 4;
            ULONG Family : 4;
            ULONG Reserved0 : 4;
            ULONG ExtendedModel : 4;
            ULONG ExtendedFamily : 8;
            ULONG Reserved1 : 4;
        };

        ULONG AsUlong;
    } Signature;

    //
    // Get cpu feature information.
    //

    KiCpuId(1, 0, &CpuInformation);
    Signature.AsUlong = CpuInformation.Eax;

    //
    // Set processor family and stepping information.
    //

    if (Signature.Family == 0xf) {
        Family = Signature.Family + Signature.ExtendedFamily;
        Model = (Signature.ExtendedModel << 4) | Signature.Model;

    } else {
        Family = Signature.Family;
        Model = Signature.Model;
    }

    //
    // Derive the extended model info for Intel family 6 processors.
    //

    if ((Prcb->CpuVendor == CPU_INTEL) && (Family == 6)) {
        Model |= (Signature.ExtendedModel << 4);
    }

    Stepping = Signature.Stepping;
    Prcb->CpuID = TRUE;
    Prcb->CpuType = (UCHAR)Family;
    Prcb->CpuStep = (USHORT)((Model << 8) | Stepping);

    //
    // Retrieve the microcode update signature
    //

    if (Prcb->CpuVendor == CPU_INTEL) {
        WriteMSR(MSR_BIOS_SIGN, 0);
        KiCpuId(1, 0, &CpuInformation);
        Prcb->UpdateSignature.QuadPart = ReadMSR(MSR_BIOS_SIGN);
    }

    //
    // Enable prefetch retry as appropriate.
    // 

    if ((Prcb->CpuVendor == CPU_AMD) &&
        (Family <= 0x0F) &&
        (Model <= 5) &&
        (Stepping <= 8)) {

        KiPrefetchRetry |= 0x01;
    }

    return;
}

