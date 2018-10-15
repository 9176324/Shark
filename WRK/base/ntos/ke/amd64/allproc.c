
/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    allproc.c

Abstract:

    This module allocates and initializes kernel resources required to
    start a new processor, and passes a complete process state structure
    to the hal to obtain a new processor.

--*/

#include "ki.h"
#include "pool.h"

//
// Define local macros.
//

#define ROUNDUP64(x) (((x) + 63) & ~63)

//
// Define prototypes for forward referenced functions.
//

#if !defined(NT_UP)

VOID
KiCopyDescriptorMemory (
   IN PKDESCRIPTOR Source,
   IN PKDESCRIPTOR Destination,
   IN PVOID Base
   );

VOID
KiSetDescriptorBase (
   IN USHORT Selector,
   IN PKGDTENTRY64 GdtBase,
   IN PVOID Base
   );

//
// Define processor node query function address.
//
// N.B. This function address is filled in during kernel initialization
//      if the host system is a multinode system.
//

PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode;

#pragma alloc_text(INIT, KiCopyDescriptorMemory)
#pragma alloc_text(INIT, KiSetDescriptorBase)

//
// Dummy NUMA node structures used during system initialization.
//

#pragma data_seg("INITDATA")

KNODE KiNodeInit[MAXIMUM_CCNUMA_NODES];

#pragma data_seg()

#endif // !defined(NT_UP)

#pragma alloc_text(INIT, KeStartAllProcessors)

ULONG KiBarrierWait = 0;
ULONG KeRegisteredProcessors;


VOID
KeStartAllProcessors (
    VOID
    )

/*++

Routine Description:

    This function is called during phase 1 initialization on the master boot
    processor to start all of the other registered processors.

Arguments:

    None.

Return Value:

    None.

--*/

{

#if !defined(NT_UP)

    ULONG AllocationSize;
    PUCHAR Base;
    PKPCR CurrentPcr = KeGetPcr();
    PVOID DataBlock;
    PVOID DpcStack;
    PKGDTENTRY64 GdtBase;
    ULONG GdtOffset;
    ULONG IdtOffset;
    UCHAR Index;
    PVOID KernelStack;
    ULONG LogicalProcessors;
    ULONG MaximumProcessors;
    PKNODE Node;
    UCHAR NodeNumber = 0;
    UCHAR Number;
    KIRQL OldIrql;
    PKNODE OldNode;
    PKNODE ParentNode;
    PKPCR PcrBase;
    PKPRCB Prcb;
    USHORT ProcessorId;
    KPROCESSOR_STATE ProcessorState;
    PKTSS64 SysTssBase;
    PKGDTENTRY64 TebBase;
    PETHREAD Thread;

    //
    // Ensure that prefetch instructions in the IPI path are patched out
    // if necessary before starting other processors.
    //

    OldIrql = KeRaiseIrqlToSynchLevel();
    KiIpiSendRequest(1, 0, 0, IPI_FLUSH_SINGLE);
    KeLowerIrql(OldIrql);

    //
    // Do not start additional processors if the relocate physical loader
    // switch has been specified.
    // 

    if (KeLoaderBlock->LoadOptions != NULL) {
        if (strstr(KeLoaderBlock->LoadOptions, "RELOCATEPHYSICAL") != NULL) {
            return;
        }
    }

    //
    // If this a multinode system and processor zero is not on node zero,
    // then move it to the appropriate node.
    //

    if (KeNumberNodes > 1) {
        if (NT_SUCCESS(KiQueryProcessorNode(0, &ProcessorId, &NodeNumber))) {
            if (NodeNumber != 0) {
                KiNode0.ProcessorMask = 0;
                KiNodeInit[0] = KiNode0;
                KeNodeBlock[0] = &KiNodeInit[0];
                KiNode0 = *KeNodeBlock[NodeNumber];
                KeNodeBlock[NodeNumber] = &KiNode0;
                KiNode0.ProcessorMask = 1;
            }

        } else {
            goto StartFailure;
        }
    }

    //
    // Calculate the size of the per processor data structures.
    //
    // This includes:
    //
    //   PCR (including the PRCB)
    //   System TSS
    //   Idle Thread Object
    //   Double Fault Stack
    //   Machine Check Stack
    //   NMI Stack
    //   Multinode structure
    //   GDT
    //   IDT
    //
    // A DPC and Idle stack are also allocated, but they are done separately.
    //

    AllocationSize = ROUNDUP64(sizeof(KPCR)) +
                     ROUNDUP64(sizeof(KTSS64)) +
                     ROUNDUP64(sizeof(ETHREAD)) +
                     ROUNDUP64(DOUBLE_FAULT_STACK_SIZE) +
                     ROUNDUP64(KERNEL_MCA_EXCEPTION_STACK_SIZE) +
                     ROUNDUP64(NMI_STACK_SIZE) +
                     ROUNDUP64(sizeof(KNODE));

    //
    // Save the offset of the GDT in the allocation structure and add in
    // the size of the GDT.
    //

    GdtOffset = AllocationSize;
    AllocationSize +=
            CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Gdtr.Limit + 1;

    //
    // Save the offset of the IDT in the allocation structure and add in
    // the size of the IDT.
    //

    IdtOffset = AllocationSize;
    AllocationSize +=
            CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Idtr.Limit + 1;

    //
    // If the registered number of processors is greater than the maximum
    // number of processors supported, then only allow the maximum number
    // of supported processors.
    //

    if (KeRegisteredProcessors > MAXIMUM_PROCESSORS) {
        KeRegisteredProcessors = MAXIMUM_PROCESSORS;
    }

    //
    // Set barrier that will prevent any other processor from entering the
    // idle loop until all processors have been started.
    //

    KiBarrierWait = 1;

    //
    // Initialize the fixed part of the processor state that will be used to
    // start processors. Each processor starts in the system initialization
    // code with address of the loader parameter block as an argument.
    //

    RtlZeroMemory(&ProcessorState, sizeof(KPROCESSOR_STATE));
    ProcessorState.ContextFrame.Rcx = (ULONG64)KeLoaderBlock;
    ProcessorState.ContextFrame.Rip = (ULONG64)KiSystemStartup;
    ProcessorState.ContextFrame.SegCs = KGDT64_R0_CODE;
    ProcessorState.ContextFrame.SegDs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegEs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegFs = KGDT64_R3_CMTEB | RPL_MASK;
    ProcessorState.ContextFrame.SegGs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegSs = KGDT64_R0_DATA;

    //
    // Check to determine if hyper-threading is really enabled. Intel chips
    // claim to be hyper-threaded with the number of logical processors
    // greater than one even when hyper-threading is disabled in the BIOS.
    //

    LogicalProcessors = KiLogicalProcessors;
    if (HalIsHyperThreadingEnabled() == FALSE) {
        LogicalProcessors = 1;
    }

    //
    // If the total number of logical processors has not been set with
    // the /NUMPROC loader option, then set the maximum number of logical
    // processors to the number of registered processors times the number
    // of logical processors per registered processor.
    //
    // N.B. The number of logical processors is never allowed to exceed
    //      the number of registered processors times the number of logical
    //      processors per physical processor.
    //

    MaximumProcessors = KeNumprocSpecified;
    if (MaximumProcessors == 0) {
        MaximumProcessors = KeRegisteredProcessors * LogicalProcessors;
    }

    //
    // Loop trying to start a new processors until a new processor can't be
    // started or an allocation failure occurs.
    //
    // N.B. The below processor start code relies on the fact a physical
    //      processor is started followed by all its logical processors.
    //      The HAL guarantees this by sorting the ACPI processor table
    //      by APIC id.
    //

    Index = 0;
    Number = 0;
    while ((Index < (MAXIMUM_PROCESSORS - 1)) &&
           ((ULONG)KeNumberProcessors < MaximumProcessors) &&
           ((ULONG)KeNumberProcessors / LogicalProcessors) < KeRegisteredProcessors) {

        //
        // If this is a multinode system and current processor does not
        // exist on any node, then skip it.
        //

        Index += 1;
        if (KeNumberNodes > 1) {
            if (!NT_SUCCESS(KiQueryProcessorNode(Index, &ProcessorId, &NodeNumber))) {
                continue;
            }
        }

        //
        // Increment the processor number.
        //

        Number += 1;

        //
        // Allocate memory for the new processor specific data. If the
        // allocation fails, then stop starting processors.
        //

        DataBlock = MmAllocateIndependentPages(AllocationSize, NodeNumber);
        if (DataBlock == NULL) {
            goto StartFailure;
        }

        //
        // Allocate a pool tag table for the new processor.
        //

        if (ExCreatePoolTagTable(Number, NodeNumber) == NULL) {
            goto StartFailure;
        }

        //
        // Zero the allocated memory.
        //

        Base = (PUCHAR)DataBlock;
        RtlZeroMemory(DataBlock, AllocationSize);

        //
        // Copy and initialize the GDT for the next processor.
        //

        KiCopyDescriptorMemory(&CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Gdtr,
                               &ProcessorState.SpecialRegisters.Gdtr,
                               Base + GdtOffset);

        GdtBase = (PKGDTENTRY64)ProcessorState.SpecialRegisters.Gdtr.Base;

        //
        // Encode the processor number in the upper 6 bits of the compatibility
        // mode TEB descriptor.
        //

        TebBase = (PKGDTENTRY64)((PCHAR)GdtBase + KGDT64_R3_CMTEB);
        TebBase->Bits.LimitHigh = Number >> 2;
        TebBase->LimitLow = ((Number & 0x3) << 14) | (TebBase->LimitLow & 0x3fff);

        //
        // Copy and initialize the IDT for the next processor.
        //

        KiCopyDescriptorMemory(&CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Idtr,
                               &ProcessorState.SpecialRegisters.Idtr,
                               Base + IdtOffset);

        //
        // Set the PCR base address for the next processor, set the processor
        // number, and set the processor speed.
        //
        // N.B. The PCR address is passed to the next processor by computing
        //      the containing address with respect to the PRCB.
        //

        PcrBase = (PKPCR)Base;
        PcrBase->ObsoleteNumber = Number;
        PcrBase->Prcb.Number = Number;
        PcrBase->Prcb.MHz = KeGetCurrentPrcb()->MHz;
        Base += ROUNDUP64(sizeof(KPCR));

        //
        // Set the system TSS descriptor base for the next processor.
        //

        SysTssBase = (PKTSS64)Base;
        KiSetDescriptorBase(KGDT64_SYS_TSS / 16, GdtBase, SysTssBase);
        Base += ROUNDUP64(sizeof(KTSS64));

        //
        // Initialize the panic stack address for double fault and NMI.
        //

        Base += DOUBLE_FAULT_STACK_SIZE;
        SysTssBase->Ist[TSS_IST_PANIC] = (ULONG64)Base;

        //
        // Initialize the machine check stack address.
        //

        Base += KERNEL_MCA_EXCEPTION_STACK_SIZE;
        SysTssBase->Ist[TSS_IST_MCA] = (ULONG64)Base;

        //
        // Initialize the NMI stack address.
        //

        Base += NMI_STACK_SIZE;
        SysTssBase->Ist[TSS_IST_NMI] = (ULONG64)Base;

        //
        // Idle Thread thread object.
        //

        Thread = (PETHREAD)Base;
        Base += ROUNDUP64(sizeof(ETHREAD));

        //
        // Set other special registers in the processor state.
        //

        ProcessorState.SpecialRegisters.Cr0 = ReadCR0();
        ProcessorState.SpecialRegisters.Cr3 = ReadCR3();
        ProcessorState.ContextFrame.EFlags = 0;
        ProcessorState.SpecialRegisters.Tr  = KGDT64_SYS_TSS;
        GdtBase[KGDT64_SYS_TSS / 16].Bytes.Flags1 = 0x89;
        ProcessorState.SpecialRegisters.Cr4 = ReadCR4();

        //
        // Allocate a kernel stack and a DPC stack for the next processor.
        //

        KernelStack = MmCreateKernelStack(FALSE, NodeNumber);
        if (KernelStack == NULL) {
            goto StartFailure;
        }

        DpcStack = MmCreateKernelStack(FALSE, NodeNumber);
        if (DpcStack == NULL) {
            goto StartFailure;
        }

        //
        // Initialize the kernel stack for the system TSS.
        //
        // N.B. System startup must be called with a stack pointer that is
        //      8 mod 16.
        //

        SysTssBase->Rsp0 = (ULONG64)KernelStack - sizeof(PVOID) * 4;
        ProcessorState.ContextFrame.Rsp = (ULONG64)KernelStack - 8;

        //
        // If this is the first processor on this node, then use the space
        // already allocated for the node. Otherwise, the space allocated
        // is not used.
        //

        Node = KeNodeBlock[NodeNumber];
        OldNode = Node;
        if (Node == &KiNodeInit[NodeNumber]) {
            Node = (PKNODE)Base;
            *Node = KiNodeInit[NodeNumber];
            KeNodeBlock[NodeNumber] = Node;
        }

        Base += ROUNDUP64(sizeof(KNODE));

        //
        // Set the parent node address.
        //

        PcrBase->Prcb.ParentNode = Node;

        //
        // Adjust the loader block so it has the next processor state. Ensure
        // that the kernel stack has space for home registers for up to four
        // parameters.
        //

        KeLoaderBlock->KernelStack = (ULONG64)DpcStack - (sizeof(PVOID) * 4);
        KeLoaderBlock->Thread = (ULONG64)Thread;
        KeLoaderBlock->Prcb = (ULONG64)(&PcrBase->Prcb);

        //
        // Attempt to start the next processor. If a processor cannot be
        // started, then deallocate memory and stop starting processors.
        //

        if (HalStartNextProcessor(KeLoaderBlock, &ProcessorState) == 0) {

            //
            // Restore the old node address in the node address array before
            // freeing the allocated data block (the node structure lies
            // within the data block).
            //

            *OldNode = *Node;
            KeNodeBlock[NodeNumber] = OldNode;
            ExDeletePoolTagTable(Number);
            MmFreeIndependentPages(DataBlock, AllocationSize);
            MmDeleteKernelStack(KernelStack, FALSE);
            MmDeleteKernelStack(DpcStack, FALSE);
            break;
        }

        Node->ProcessorMask |= AFFINITY_MASK(Number);

        //
        // Wait for processor to initialize.
        //

        while (*((volatile ULONG64 *)&KeLoaderBlock->Prcb) != 0) {
            KeYieldProcessor();
        }
    }

    //
    // All processors have been started. If this is a multinode system, then
    // allocate any missing node structures.
    //

    if (KeNumberNodes > 1) {
        for (Index = 0; Index < KeNumberNodes; Index += 1) {
            if (KeNodeBlock[Index] == &KiNodeInit[Index]) {
                Node = ExAllocatePoolWithTag(NonPagedPool, sizeof(KNODE), '  eK');
                if (Node != NULL) {
                    *Node = KiNodeInit[Index];
                    KeNodeBlock[Index] = Node;

                } else {
                    goto StartFailure;
                }
            }
        }

    } else if (KiNode0.ProcessorMask != KeActiveProcessors) {
        goto StartFailure;
    }

    //
    // Clear node structure address for nonexistent nodes.
    //

    for (Index = KeNumberNodes; Index < MAXIMUM_CCNUMA_NODES; Index += 1) {
        KeNodeBlock[Index] = NULL;
    }

    //
    // Copy the node color and shifted color to the PRCB of each processor.
    //

    for (Index = 0; Index < (ULONG)KeNumberProcessors; Index += 1) {
        Prcb = KiProcessorBlock[Index];
        ParentNode = Prcb->ParentNode;
        Prcb->NodeColor = ParentNode->Color;
        Prcb->NodeShiftedColor = ParentNode->MmShiftedColor;
        Prcb->SecondaryColorMask = MmSecondaryColorMask;
    }

    //
    // Reset the initialization bit in prefetch retry.
    //

    KiPrefetchRetry &= ~0x80;

    //
    // Reset and synchronize the performance counters of all processors, by
    // applying a null adjustment to the interrupt time.
    //

    KeAdjustInterruptTime(0);

    //
    // Allow all processors that were started to enter the idle loop and
    // begin execution.
    //

    KiBarrierWait = 0;

#endif //

    return;

    //
    // The failure to allocate memory or a unsuccessful status was returned
    // during the attempt to start processors. This is considered fatal since
    // something is very wrong.
    //

#if !defined(NT_UP)

StartFailure:
    KeBugCheckEx(PHASE1_INITIALIZATION_FAILED, 0, 0, 20, 0);

#endif

}

#if !defined(NT_UP)

VOID
KiSetDescriptorBase (
   IN USHORT Selector,
   IN PKGDTENTRY64 GdtBase,
   IN PVOID Base
   )

/*++

Routine Description:

    This function sets the base address of a descriptor to the specified
    base address.

Arguments:

    Selector - Supplies the selector for the descriptor.

    GdtBase - Supplies a pointer to the GDT.

    Base - Supplies a pointer to the base address.

Return Value:

    None.

--*/

{

    GdtBase = &GdtBase[Selector];
    GdtBase->BaseLow = (USHORT)((ULONG64)Base);
    GdtBase->Bytes.BaseMiddle = (UCHAR)((ULONG64)Base >> 16);
    GdtBase->Bytes.BaseHigh = (UCHAR)((ULONG64)Base >> 24);
    GdtBase->BaseUpper = (ULONG)((ULONG64)Base >> 32);
    return;
}

VOID
KiCopyDescriptorMemory (
   IN PKDESCRIPTOR Source,
   IN PKDESCRIPTOR Destination,
   IN PVOID Base
   )

/*++

Routine Description:

    This function copies the specified descriptor memory to the new memory
    and initializes a descriptor for the new memory.

Arguments:

    Source - Supplies a pointer to the source descriptor that describes
        the memory to copy.

    Destination - Supplies a pointer to the destination descriptor to be
        initialized.

    Base - Supplies a pointer to the new memory.

Return Value:

    None.

--*/

{

    Destination->Limit = Source->Limit;
    Destination->Base = Base;
    RtlCopyMemory(Base, Source->Base, Source->Limit + 1);
    return;
}

#endif // !defined(NT_UP)

