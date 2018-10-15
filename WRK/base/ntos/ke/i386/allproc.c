/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    allproc.c

Abstract:

    This module allocates and initializes kernel resources required
    to start a new processor, and passes a complete process_state
    structure to the hal to obtain a new processor.  This is done
    for every processor.

Environment:

    Kernel mode only.
    Phase 1 of bootup

--*/


#include "ki.h"
#include "pool.h"

extern BOOLEAN KiSMTProcessorsPresent;

#ifdef NT_UP

VOID
KeStartAllProcessors (
    VOID
    )
{
        // UP Build - this function is a nop
}

#else

static VOID
KiCloneDescriptor (
   IN PKDESCRIPTOR  pSrcDescriptorInfo,
   IN PKDESCRIPTOR  pDestDescriptorInfo,
   IN PVOID         Base
   );

static VOID
KiCloneSelector (
   IN ULONG    SrcSelector,
   IN PKGDTENTRY    pNGDT,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   );

PKPRCB
KiInitProcessorState (
   PKPROCESSOR_STATE  pProcessorState,
   PVOID               PerProcessorAllocation,
   ULONG               NewProcessorNumber,
   UCHAR               NodeNumber,
   ULONG               IdtOffset,
   ULONG               GdtOffset,
   PVOID            *ppStack,
   PVOID            *ppDpcStack
   );

BOOLEAN
KiInitProcessor (
   ULONG    NewProcessorNumber,
   PUCHAR  pNodeNumber,
   ULONG    IdtOffset,
   ULONG    GdtOffset,
   SIZE_T   ProcessorDataSize
   );

VOID
KiAdjustSimultaneousMultiThreadingCharacteristics(
    VOID
    );

VOID
KiProcessorStart(
    VOID
    );

BOOLEAN
KiStartWaitAcknowledge(
    VOID
    );

VOID 
KiSetHaltedNmiandDoubleFaultHandler(
    VOID
    );

VOID
KiDummyNmiHandler (
    VOID
    );

VOID
KiDummyDoubleFaultHandler(
    VOID
    );

VOID
KiClearBusyBitInTssDescriptor(
       IN ULONG DescriptorOffset
     );

VOID
KiHaltThisProcessor(
    VOID
    ) ;

#if defined(KE_MULTINODE)

NTSTATUS
KiNotNumaQueryProcessorNode(
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, KiNotNumaQueryProcessorNode)
#endif

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,KeStartAllProcessors)
#ifndef NT_UP
#pragma alloc_text(INIT,KiInitProcessorState)
#endif
#pragma alloc_text(INIT,KiCloneDescriptor)
#pragma alloc_text(INIT,KiCloneSelector)
#pragma alloc_text(INIT,KiAllProcessorsStarted)
#pragma alloc_text(INIT,KiAdjustSimultaneousMultiThreadingCharacteristics)
#pragma alloc_text(INIT,KiStartWaitAcknowledge)
#endif

enum {
    KcStartContinue,
    KcStartWait,
    KcStartGetId,
    KcStartDoNotStart,
    KcStartCommandError = 0xff
} KiProcessorStartControl = KcStartContinue;

ULONG KiProcessorStartData[4];

ULONG KiBarrierWait = 0;

#if defined(KE_MULTINODE)

PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode = KiNotNumaQueryProcessorNode;

//
// Statically preallocate enough KNODE structures to allow MM
// to allocate pages by node during system initialization.  As
// processors are brought online, real KNODE structures are
// allocated in the appropriate memory for the node.
//
// This statically allocated set will be deallocated once the
// system is initialized.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

KNODE KiNodeInit[MAXIMUM_CCNUMA_NODES];

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

#endif

#define ROUNDUP16(x)        (((x)+15) & ~15)

PKPRCB
KiInitProcessorState(
    PKPROCESSOR_STATE  pProcessorState,
    PVOID               PerProcessorAllocation,
    ULONG               NewProcessorNumber,
    UCHAR               NodeNumber,
    ULONG               IdtOffset,
    ULONG               GdtOffset,
    PVOID             *ppStack,
    PVOID             *ppDpcStack
    )
/*++

Routine Description:

    Called to initialize processor state during p0 bootup.

Return value:

    Prcb for new processor

--*/
{
    KDESCRIPTOR         Descriptor;
    KDESCRIPTOR         TSSDesc, DFTSSDesc, NMITSSDesc, PCRDesc;
    PKGDTENTRY          pGDT;
    PUCHAR              pThreadObject;
    PULONG              pTopOfStack;
    PKTSS               pTSS;
    PUCHAR              Base;
    PKPRCB              NewPrcb;

    ULONG               xCr0, xCr3, xEFlags;

    Base = (PUCHAR)PerProcessorAllocation;

    //
    // Give the new processor its own GDT.
    //

    _asm {
        sgdt    Descriptor.Limit
    }

    KiCloneDescriptor (&Descriptor,
                       &pProcessorState->SpecialRegisters.Gdtr,
                       Base + GdtOffset);

    pGDT = (PKGDTENTRY) pProcessorState->SpecialRegisters.Gdtr.Base;


    //
    // Give new processor its own IDT.
    //

    _asm {
        sidt    Descriptor.Limit
    }
    KiCloneDescriptor (&Descriptor,
                       &pProcessorState->SpecialRegisters.Idtr,
                       Base + IdtOffset);


    //
    // Give new processor its own TSS and PCR.
    //

    KiCloneSelector (KGDT_R0_PCR, pGDT, &PCRDesc, Base);
    RtlZeroMemory (Base, ROUNDUP16(sizeof(KPCR)));
    Base += ROUNDUP16(sizeof(KPCR));

    KiCloneSelector (KGDT_TSS, pGDT, &TSSDesc, Base);
    Base += ROUNDUP16(sizeof(KTSS));

    //
    // Idle Thread thread object.
    //

    pThreadObject = Base;
    RtlZeroMemory(Base, sizeof(ETHREAD));
    Base += ROUNDUP16(sizeof(ETHREAD));

    //
    // NMI TSS and double-fault TSS & stack.
    //

    KiCloneSelector (KGDT_DF_TSS, pGDT, &DFTSSDesc, Base);
    Base += ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps));

    KiCloneSelector (KGDT_NMI_TSS, pGDT, &NMITSSDesc, Base);
    Base += ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps));

    Base += DOUBLE_FAULT_STACK_SIZE;

    pTSS = (PKTSS)DFTSSDesc.Base;
    pTSS->Esp0 = (ULONG)Base;
    pTSS->Esp  = (ULONG)Base;

    pTSS = (PKTSS)NMITSSDesc.Base;
    pTSS->Esp0 = (ULONG)Base;
    pTSS->Esp  = (ULONG)Base;

    //
    // Set other SpecialRegisters in processor state.
    //

    _asm {
        mov     eax, cr0
        and     eax, NOT (CR0_AM or CR0_WP)
        mov     xCr0, eax
        mov     eax, cr3
        mov     xCr3, eax

        pushfd
        pop     eax
        mov     xEFlags, eax
        and     xEFlags, NOT EFLAGS_INTERRUPT_MASK
    }

    pProcessorState->SpecialRegisters.Cr0 = xCr0;
    pProcessorState->SpecialRegisters.Cr3 = xCr3;
    pProcessorState->ContextFrame.EFlags = xEFlags;


    pProcessorState->SpecialRegisters.Tr  = KGDT_TSS;
    pGDT[KGDT_TSS>>3].HighWord.Bytes.Flags1 = 0x89;

    //
    // Encode the processor number into the segment limit of the TEB
    // 6 bits in total. 4 in the high and 2 in the low limit.
    //
    pGDT[KGDT_R3_TEB>>3].LimitLow = (USHORT)((NewProcessorNumber&0x3)<<(16-2));
    pGDT[KGDT_R3_TEB>>3].HighWord.Bits.LimitHi = (NewProcessorNumber>>2);

#if defined(_X86PAE_)
    pProcessorState->SpecialRegisters.Cr4 = CR4_PAE;
#endif

    //
    // Allocate a DPC stack, idle thread stack and ThreadObject for
    // the new processor.
    //

    *ppStack = MmCreateKernelStack (FALSE, NodeNumber);
    *ppDpcStack = MmCreateKernelStack (FALSE, NodeNumber);

    //
    // Setup context - push variables onto new stack.
    //

    pTopOfStack = (PULONG) *ppStack;
    pTopOfStack[-1] = (ULONG) KeLoaderBlock;
    pProcessorState->ContextFrame.Esp = (ULONG) (pTopOfStack-2);
    pProcessorState->ContextFrame.Eip = (ULONG) KiSystemStartup;

    pProcessorState->ContextFrame.SegCs = KGDT_R0_CODE;
    pProcessorState->ContextFrame.SegDs = KGDT_R3_DATA;
    pProcessorState->ContextFrame.SegEs = KGDT_R3_DATA;
    pProcessorState->ContextFrame.SegFs = KGDT_R0_PCR;
    pProcessorState->ContextFrame.SegSs = KGDT_R0_DATA;


    //
    // Initialize new processor's PCR & Prcb.
    //

    KiInitializePcr (
        (ULONG)       NewProcessorNumber,
        (PKPCR)       PCRDesc.Base,
        (PKIDTENTRY)  pProcessorState->SpecialRegisters.Idtr.Base,
        (PKGDTENTRY)  pProcessorState->SpecialRegisters.Gdtr.Base,
        (PKTSS)       TSSDesc.Base,
        (PKTHREAD)    pThreadObject,
        (PVOID)       *ppDpcStack
    );

    NewPrcb = ((PKPCR)(PCRDesc.Base))->Prcb;

    //
    // Assume new processor will be the first processor in its
    // SMT set.   (Right choice for non SMT processors, adjusted
    // later if not correct).
    //

    NewPrcb->MultiThreadSetMaster = NewPrcb;

#if defined(KE_MULTINODE)

    //
    // If this is the first processor on this node, use the
    // space allocated for KNODE as the KNODE.
    //

    if (KeNodeBlock[NodeNumber] == &KiNodeInit[NodeNumber]) {
        Node = (PKNODE)Base;
        *Node = KiNodeInit[NodeNumber];
        KeNodeBlock[NodeNumber] = Node;
    }
    Base += ROUNDUP16(sizeof(KNODE));

    NewPrcb->ParentNode = Node;

#else

    NewPrcb->ParentNode = KeNodeBlock[0];

#endif

    ASSERT(((PUCHAR)PerProcessorAllocation + GdtOffset) == Base);

    //
    //  Adjust LoaderBlock so it has the next processors state
    //

    KeLoaderBlock->KernelStack = (ULONG) pTopOfStack;
    KeLoaderBlock->Thread = (ULONG) pThreadObject;
    KeLoaderBlock->Prcb = (ULONG) NewPrcb;


    //
    // Get CPUID(1) info from the starting processor.
    //

    KiProcessorStartData[0] = 1;
    KiProcessorStartControl = KcStartGetId;

    return NewPrcb;
}



VOID
KeStartAllProcessors (
    VOID
    )
/*++

Routine Description:

    Called by p0 during phase 1 of bootup.  This function implements
    the x86 specific code to contact the hal for each system processor.

Arguments:

Return Value:

    All available processors are sent to KiSystemStartup.

--*/
{
    KDESCRIPTOR         Descriptor;
    ULONG               NewProcessorNumber;
    SIZE_T              ProcessorDataSize;
    UCHAR               NodeNumber = 0;
    ULONG               IdtOffset;
    ULONG               GdtOffset;

#if defined(KE_MULTINODE)

    USHORT              ProcessorId;
    PKNODE              Node;
    NTSTATUS            Status;

#endif

    //
    // Do not start additional processors if the RELOCATEPHYSICAL loader
    // switch has been specified.
    // 

    if (KeLoaderBlock->LoadOptions != NULL) {
        if (strstr(KeLoaderBlock->LoadOptions, "RELOCATEPHYSICAL") != NULL) {
            return;
        }
    }

    //
    // If the boot processor has PII spec A27 errata (also present in
    // early Pentium Pro chips), then use only one processor to avoid
    // unpredictable eflags corruption.
    //
    // Note this only affects some (but not all) chips @ 333Mhz and below.
    //

    if (!(KeFeatureBits & KF_WORKING_PTE)) {
        return;
    }

#if defined(KE_MULTINODE)

    //
    // In the unlikely event that processor 0 is not on node
    // 0, fix it.
    //

    if (KeNumberNodes > 1) {
        Status = KiQueryProcessorNode(0,
                                      &ProcessorId,
                                      &NodeNumber);

        if (NT_SUCCESS(Status)) {

            //
            // Adjust the data structures to reflect that P0 is not on Node 0.
            //

            if (NodeNumber != 0) {

                ASSERT(KeNodeBlock[0] == &KiNode0);
                KeNodeBlock[0]->ProcessorMask &= ~1;
                KiNodeInit[0] = *KeNodeBlock[0];
                KeNodeBlock[0] = &KiNodeInit[0];

                KiNode0 = *KeNodeBlock[NodeNumber];
                KeNodeBlock[NodeNumber] = &KiNode0;
                KeNodeBlock[NodeNumber]->ProcessorMask |= 1;
            }
        }
    }

#endif

    //
    // Calculate the size of the per processor data.  This includes
    //   PCR (+PRCB)
    //   TSS
    //   Idle Thread Object
    //   NMI TSS
    //   Double Fault TSS
    //   Double Fault Stack
    //   GDT
    //   IDT
    //
    // If this is a multinode system, the KNODE structure is allocated
    // as well.   It isn't very big so we waste a few bytes for
    // processors that aren't the first in a node.
    //
    // A DPC and Idle stack are also allocated but these are done
    // separately.
    //

    ProcessorDataSize = ROUNDUP16(sizeof(KPCR))                 +
                        ROUNDUP16(sizeof(KTSS))                 +
                        ROUNDUP16(sizeof(ETHREAD))              +
                        ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps))   +
                        ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps))   +
                        ROUNDUP16(DOUBLE_FAULT_STACK_SIZE);

#if defined(KE_MULTINODE)

    ProcessorDataSize += ROUNDUP16(sizeof(KNODE));

#endif

    //
    // Add sizeof GDT
    //

    GdtOffset = ProcessorDataSize;
    _asm {
        sgdt    Descriptor.Limit
    }
    ProcessorDataSize += Descriptor.Limit + 1;

    //
    // Add sizeof IDT
    //

    IdtOffset = ProcessorDataSize;
    _asm {
        sidt    Descriptor.Limit
    }
    ProcessorDataSize += Descriptor.Limit + 1;

    //
    // Set barrier that will prevent any other processor from entering the
    // idle loop until all processors have been started.
    //

    KiBarrierWait = 1;

    //
    // Loop asking the HAL for the next processor.   Stop when the
    // HAL says there aren't any more.
    //

    for (NewProcessorNumber = 1;
         NewProcessorNumber < MAXIMUM_PROCESSORS;
         NewProcessorNumber++) {

        if (! KiInitProcessor(NewProcessorNumber, &NodeNumber, IdtOffset, GdtOffset, ProcessorDataSize) ) {
            break;
        }

        KiProcessorStartControl = KcStartContinue;

#if defined(KE_MULTINODE)

        Node->ProcessorMask |= 1 << NewProcessorNumber;

#endif

        //
        // Wait for processor to initialize in kernel, then loop for another.
        //

        while (*((volatile ULONG *) &KeLoaderBlock->Prcb) != 0) {
            KeYieldProcessor();
        }
    }

    //
    // All processors have been started.
    //

    KiAllProcessorsStarted();

    //
    // Reset and synchronize the performance counters of all processors, by
    // applying a null adjustment to the interrupt time.
    //

    KeAdjustInterruptTime (0);

    //
    // Allow all processors that were started to enter the idle loop and
    // begin execution.
    //

    KiBarrierWait = 0;
}



static VOID
KiCloneSelector (
   IN ULONG    SrcSelector,
   IN PKGDTENTRY    pNGDT,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   )

/*++

Routine Description:

    Makes a copy of the current selector's data, and updates the new
    GDT's linear address to point to the new copy.

Arguments:

    SrcSelector     -   Selector value to clone
    pNGDT           -   New gdt table which is being built
    DescDescriptor  -   descriptor structure to fill in with resulting memory
    Base            -   Base memory for the new descriptor.

Return Value:

    None.

--*/

{
    KDESCRIPTOR Descriptor;
    PKGDTENTRY  pGDT;
    ULONG       CurrentBase;
    ULONG       NewBase;
    ULONG       Limit;

    _asm {
        sgdt    fword ptr [Descriptor.Limit]    ; Get GDT's addr
    }

    pGDT   = (PKGDTENTRY) Descriptor.Base;
    pGDT  += SrcSelector >> 3;
    pNGDT += SrcSelector >> 3;

    CurrentBase = pGDT->BaseLow | (pGDT->HighWord.Bits.BaseMid << 16) |
                 (pGDT->HighWord.Bits.BaseHi << 24);

    Descriptor.Base  = CurrentBase;
    Descriptor.Limit = pGDT->LimitLow;
    if (pGDT->HighWord.Bits.Granularity & GRAN_PAGE) {
        Limit = (Descriptor.Limit << PAGE_SHIFT) - 1;
        Descriptor.Limit = (USHORT) Limit;
        ASSERT (Descriptor.Limit == Limit);
    }

    KiCloneDescriptor (&Descriptor, pDestDescriptor, Base);
    NewBase = pDestDescriptor->Base;

    pNGDT->BaseLow = (USHORT) NewBase & 0xffff;
    pNGDT->HighWord.Bits.BaseMid = (UCHAR) (NewBase >> 16) & 0xff;
    pNGDT->HighWord.Bits.BaseHi  = (UCHAR) (NewBase >> 24) & 0xff;
}



static VOID
KiCloneDescriptor (
   IN PKDESCRIPTOR  pSrcDescriptor,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   )

/*++

Routine Description:

    Makes a copy of the specified descriptor, and supplies a return
    descriptor for the new copy

Arguments:

    pSrcDescriptor  - descriptor to clone
    pDescDescriptor - the cloned descriptor
    Base            - Base memory for the new descriptor.

Return Value:

    None.

--*/
{
    ULONG   Size;

    Size = pSrcDescriptor->Limit + 1;
    pDestDescriptor->Limit = (USHORT) Size -1;
    pDestDescriptor->Base  = (ULONG)  Base;

    RtlCopyMemory(Base, (PVOID)pSrcDescriptor->Base, Size);
}


VOID
KiAdjustSimultaneousMultiThreadingCharacteristics(
    VOID
    )

/*++

Routine Description:

    This routine is called (possibly while the dispatcher lock is held)
    after processors are added to or removed from the system.   It runs
    thru the PRCBs for each processor in the system and adjusts scheduling
    data.

Arguments:

    None.

Return Value:

    None.

--*/

{
    ULONG ProcessorNumber;
    ULONG BuddyNumber;
    KAFFINITY ProcessorSet;
    PKPRCB Prcb;
    PKPRCB BuddyPrcb;
    ULONG ApicMask;
    ULONG ApicId;

    if (!KiSMTProcessorsPresent) {

        //
        // Nobody doing SMT, nothing to do.
        //

        return;
    }

    for (ProcessorNumber = 0;
         ProcessorNumber < (ULONG)KeNumberProcessors;
         ProcessorNumber++) {

        Prcb = KiProcessorBlock[ProcessorNumber];

        //
        // Skip processors which are not present or which do not
        // support Simultaneous Multi Threading.
        //

        if ((Prcb == NULL) ||
            (Prcb->LogicalProcessorsPerPhysicalProcessor == 1)) {
            continue;
        }

        //
        // Find all processors with the same physical processor APIC ID.
        // The APIC ID for the physical processor is the upper portion
        // of the APIC ID, the number of bits in the lower portion is
        // log 2 (number logical processors per physical rounded up to
        // a power of 2).
        //

        ApicId = Prcb->InitialApicId;

        //
        // Round number of logical processors up to a power of 2
        // then subtract one to get the logical processor apic mask.
        //

        ASSERT(Prcb->LogicalProcessorsPerPhysicalProcessor);
        ApicMask = Prcb->LogicalProcessorsPerPhysicalProcessor;

        ApicMask = ApicMask + ApicMask - 1;
        KeFindFirstSetLeftMember(ApicMask, &ApicMask);
        ApicMask = ~((1 << ApicMask) - 1);

        ApicId &= ApicMask;

        ProcessorSet = 1 << Prcb->Number;

        //
        // Examine each remaining processor to see if it is part of
        // the same set.
        //

        for (BuddyNumber = ProcessorNumber + 1;
             BuddyNumber < (ULONG)KeNumberProcessors;
             BuddyNumber++) {

            BuddyPrcb = KiProcessorBlock[BuddyNumber];

            //
            // Skip not present, not SMT.
            //

            if ((BuddyPrcb == NULL) ||
                (BuddyPrcb->LogicalProcessorsPerPhysicalProcessor == 1)) {
                continue;
            }

            //
            // Does this processor have the same ID as the one
            // we're looking for?
            //

            if (((BuddyPrcb->InitialApicId & ApicMask) != ApicId) ||
                (BuddyPrcb->ParentNode != Prcb->ParentNode)) {

                continue;
            }

            //
            // Match.
            //

            ASSERT(Prcb->LogicalProcessorsPerPhysicalProcessor ==
                   BuddyPrcb->LogicalProcessorsPerPhysicalProcessor);

            ProcessorSet |= 1 << BuddyPrcb->Number;
            BuddyPrcb->MultiThreadProcessorSet |= ProcessorSet;
        }

        Prcb->MultiThreadProcessorSet |= ProcessorSet;
    }
}


VOID
KiAllProcessorsStarted(
    VOID
    )

/*++

Routine Description:

    This routine is called once all processors in the system
    have been started.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG i;
    PKPRCB Prcb;
    PKNODE ParentNode;

    //
    // If the system contains Simultaneous Multi Threaded processors,
    // adjust grouping information now that each processor is started.
    //

    KiAdjustSimultaneousMultiThreadingCharacteristics();

#if defined(KE_MULTINODE)

    //
    // Make sure there are no references to the temporary nodes
    // used during initialization.
    //

    for (i = 0; i < KeNumberNodes; i++) {
        if (KeNodeBlock[i] == &KiNodeInit[i]) {

            //
            // No processor started on this node so no new node
            // structure has been allocated.   This is possible
            // if the node contains only memory or IO busses.  At
            // this time we need to allocate a permanent node
            // structure for the node.
            //

            KeNodeBlock[i] = ExAllocatePoolWithTag(NonPagedPool,
                                                   sizeof(KNODE),
                                                   '  eK');
            if (KeNodeBlock[i]) {
                *KeNodeBlock[i] = KiNodeInit[i];
            }
        }

        //
        // Set the node number.
        //

        KeNodeBlock[i]->NodeNumber = (UCHAR)i;
    }

    for (i = KeNumberNodes; i < MAXIMUM_CCNUMA_NODES; i++) {
        KeNodeBlock[i] = NULL;
    }

#endif

    //
    // Copy the node color and shifted color to the PRCB of each processor.
    //

    for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
        Prcb = KiProcessorBlock[i];
        ParentNode = Prcb->ParentNode;
        Prcb->NodeColor = ParentNode->Color;
        Prcb->NodeShiftedColor = ParentNode->MmShiftedColor;
        Prcb->SecondaryColorMask = MmSecondaryColorMask;
    }

    if (KeNumberNodes == 1) {

        //
        // For Non NUMA machines, Node 0 gets all processors.
        //

        KeNodeBlock[0]->ProcessorMask = KeActiveProcessors;
    }
}

#if defined(KE_MULTINODE)

NTSTATUS
KiNotNumaQueryProcessorNode(
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    )

/*++

Routine Description:

    This routine is a stub used on non NUMA systems to provide a
    consistent method of determining the NUMA configuration rather
    than checking for the presence of multiple nodes inline.

Arguments:

    ProcessorNumber supplies the system logical processor number.
    Identifier      supplies the address of a variable to receive
                    the unique identifier for this processor.
    NodeNumber      supplies the address of a variable to receive
                    the number of the node this processor resides on.

Return Value:

    Returns success.

--*/

{
    *Identifier = (USHORT)ProcessorNumber;
    *Node = 0;
    return STATUS_SUCCESS;
}

#endif

VOID
KiProcessorStart(
    VOID
    )

/*++

Routine Description:

    
    This routine is a called when a processor begins execution.
    It is used to pass processor characteristic information to 
    the boot processor and to control the starting or non-starting
    of this processor.

Arguments:

    None.

Return Value:

    None.

--*/

{
    while (TRUE) {
        switch (KiProcessorStartControl) {

        case KcStartContinue:
            return;

        case KcStartWait:
            KeYieldProcessor();
            break;

        case KcStartGetId:
            CPUID(KiProcessorStartData[0],
                  &KiProcessorStartData[0],
                  &KiProcessorStartData[1],
                  &KiProcessorStartData[2],
                  &KiProcessorStartData[3]);
            KiProcessorStartControl = KcStartWait;
            break;

        case KcStartDoNotStart:

            //
            // The boot processor has determined that this processor
            // should NOT be started.
            //
            // Acknowledge the command so the boot processor will
            // continue, disable interrupts (should already be 
            // the case here) and HALT the processor.
            //

            KiProcessorStartControl = KcStartWait;
            KiSetHaltedNmiandDoubleFaultHandler();
            _disable();
            while(1) {
                _asm { hlt };
            }

        default:

            //
            // Not much we can do with unknown commands.
            //

            KiProcessorStartControl = KcStartCommandError;
            break;
        }
    }
}

BOOLEAN
KiStartWaitAcknowledge(
    VOID
    )
{
    while (KiProcessorStartControl != KcStartWait) {
        if (KiProcessorStartControl == KcStartCommandError) {
            return FALSE;
        }
        KeYieldProcessor();
    }
    return TRUE;
}

VOID 
KiSetHaltedNmiandDoubleFaultHandler(
    VOID
    )

/*++

Routine Description:

    
    This routine is a called before the application processor that is not
    going to be started is put into halt. It is used to hook a dummy Nmi and 
    double fault handler.


Arguments:

    None.

Return Value:

    None.
--*/
{
    PKPCR Pcr;
    PKGDTENTRY GdtPtr;
    ULONG TssAddr;
   
    Pcr = KeGetPcr();
       
    GdtPtr  = (PKGDTENTRY)&(Pcr->GDT[Pcr->IDT[IDT_NMI_VECTOR].Selector >> 3]);
    TssAddr = (((GdtPtr->HighWord.Bits.BaseHi << 8) +
                 GdtPtr->HighWord.Bits.BaseMid) << 16) + GdtPtr->BaseLow;
    ((PKTSS)TssAddr)->Eip = (ULONG)KiDummyNmiHandler;


    GdtPtr  = (PKGDTENTRY)&(Pcr->GDT[Pcr->IDT[IDT_DFH_VECTOR].Selector >> 3]);
    TssAddr = (((GdtPtr->HighWord.Bits.BaseHi << 8) +
                   GdtPtr->HighWord.Bits.BaseMid) << 16) + GdtPtr->BaseLow;
    ((PKTSS)TssAddr)->Eip = (ULONG)KiDummyDoubleFaultHandler;

    return;

}


VOID
KiDummyNmiHandler (
    VOID
    )

/*++

Routine Description:

    This is the dummy handler that is executed by the processor
    that is not started. We are just being paranoid about clearing
    the busy bit for the NMI and Double Fault Handler TSS.


Arguments:

    None.

Return Value:

    Does not return
--*/
{
    KiClearBusyBitInTssDescriptor(NMI_TSS_DESC_OFFSET);
    KiHaltThisProcessor();


}



VOID
KiDummyDoubleFaultHandler(
    VOID
    )

/*++

Routine Description:

    This is the dummy handler that is executed by the processor
    that is not started. We are just being paranoid about clearing
    the busy bit for the NMI and Double Fault Handler TSS.


Arguments:

    None.

Return Value:

    Does not return
--*/
{
    KiClearBusyBitInTssDescriptor(DF_TSS_DESC_OFFSET);
    KiHaltThisProcessor();
}



VOID
KiClearBusyBitInTssDescriptor(
       IN ULONG DescriptorOffset
       )  
/*++

Routine Description:

    Called to clear busy bit in descriptor from the NMI and double
    Fault Handlers
    

Arguments:

    None.

Return Value:

    None.
--*/
{
    PKPCR Pcr;
    PKGDTENTRY GdtPtr;
    Pcr = KeGetPcr();
    GdtPtr =(PKGDTENTRY)(Pcr->GDT);
    GdtPtr =(PKGDTENTRY)((ULONG)GdtPtr + DescriptorOffset);
    GdtPtr->HighWord.Bytes.Flags1 = 0x89; // 32bit. dpl=0. present, TSS32, not busy

}


VOID
KiHaltThisProcessor(
    VOID
) 

/*++

Routine Description:

  After Clearing the busy bit (just being paranoid here) we halt
  this processor. 

Arguments:

    None.

Return Value:

    None.
--*/
{

    while(1) {
        _asm {
               hlt 
        }
    }
}
#endif      // !NT_UP

