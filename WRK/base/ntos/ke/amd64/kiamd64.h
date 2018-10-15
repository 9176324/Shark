/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kiamd64.h

Abstract:

    This module contains the private (internal) platform specific header file
    for the kernel.

--*/

#if !defined(_KIAMD64_)
#define _KIAMD64_

#pragma warning(disable:4213)   // nonstandard extension : cast on l-value

//
// Define IPI request summary structure.
//
// N.B. The numeric order of the request definitions is important.
//

#define IPI_FLUSH_MULTIPLE_IMMEDIATE 1
#define IPI_FLUSH_PROCESS 2
#define IPI_FLUSH_SINGLE 3
#define IPI_FLUSH_ALL 4
#define IPI_FLUSH_MULTIPLE 5
#define IPI_PACKET_READY 6
#define IPI_INVALIDATE_ALL 7

#define IPI_REQUEST_SUMMARY 0xf

typedef struct _REQUEST_SUMMARY {
    union {
        struct {
            ULONG64 IpiRequest : 4;
            ULONG64 Reserved1 : 3;
            ULONG64 IpiSynchType : 1;
            ULONG64 Count : 8;
            LONG64 Parameter : 48;
        };

        LONG64 Summary;
    };

} REQUEST_SUMMARY, *PREQUEST_SUMMARY;

//
// Define get current ready summary macro.
//

#define KiGetCurrentReadySummary()                                           \
    __readgsdword(FIELD_OFFSET(KPCR, Prcb.ReadySummary))

//
// Define function prototypes.
//

VOID
KiAcquireSpinLockCheckForFreeze (
    IN PKSPIN_LOCK SpinLock,
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    );

VOID
KiInitializeBootStructures (
    PLOADER_PARAMETER_BLOCK LoaderBlock
    );

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    );

extern KIRQL KiProfileIrql;

#if !defined(NT_UP)

extern BOOLEAN KiResumeForReboot;

#endif

FORCEINLINE
ULONG64
KiGetActualStackLimit (
    IN PKTHREAD Thread
    )

{

    PKERNEL_STACK_CONTROL StackControl;

    StackControl = (PKERNEL_STACK_CONTROL)Thread->InitialStack;
    return StackControl->Current.ActualLimit;
}

//
// Define function prototypes for trap processing functions.
//

VOID
KiDivideErrorFault (
    VOID
    );

VOID
KiDebugTrapOrFault (
    VOID
    );

VOID
KiNmiInterrupt (
    VOID
    );

VOID
KiBreakpointTrap (
    VOID
    );

VOID
KiOverflowTrap (
    VOID
    );

VOID
KiBoundFault (
    VOID
    );

VOID
KiInvalidOpcodeFault (
    VOID
    );

VOID
KiNpxNotAvailableFault (
    VOID
    );

VOID
KiDoubleFaultAbort (
    VOID
    );

VOID
KiNpxSegmentOverrunAbort (
    VOID
    );

VOID
KiInvalidTssFault (
    VOID
    );

VOID
KiRaiseAssertion (
    VOID
    );

VOID
KiSaveInitialProcessorControlState (
    IN PKPROCESSOR_STATE ProcessorState
    );

VOID
KiSegmentNotPresentFault (
    VOID
    );

VOID
KiSetPageAttributesTable (
    VOID
    );

VOID
KiStackFault (
    VOID
    );

VOID
KiGeneralProtectionFault (
    VOID
    );

VOID
KiPageFault (
    VOID
    );

VOID
KiFloatingErrorFault (
    VOID
    );

VOID
KiAlignmentFault (
    VOID
    );

VOID
KiMcheckAbort (
    VOID
    );

VOID
KiXmmException (
    VOID
    );

VOID
KiApcInterrupt (
    VOID
    );

VOID
KiDebugServiceTrap (
    VOID
    );

ULONG
KiDivide6432 (
    ULONG64 Dividend,
    ULONG Divisor
    );

VOID
KiDpcInterrupt (
    VOID
    );

VOID
KiFreezeInterrupt (
    VOID
    );

VOID
KiIpiInterrupt (
    VOID
    );

VOID
KiIpiProcessRequests (
    VOID
    );

PKAFFINITY
KiIpiSendRequest (
    IN KAFFINITY TargetSet,
    IN ULONG64 Parameter,
    IN ULONG64 Count,
    IN ULONG64 RequestType
    );

__forceinline
VOID
KiIpiWaitForRequestBarrier (
    KAFFINITY volatile *Barrier
    )

/*++

Routine Description:

    This function waits until the specified barrier is set to zero.

Arguments:

    Barrier - Supplies a pointer to a request barrier.

Return Value:

    None.

--*/

{

    //
    // Wait until the request barrier is zero before proceeding.
    //

    while (*Barrier != 0) {
        KeYieldProcessor();
    }

    return;
}

VOID
KiSecondaryClockInterrupt (
    VOID
    );

VOID
KiSystemCall32 (
    VOID
    );

VOID
KiSystemCall64 (
    VOID
    );

VOID
KiInterruptDispatchLBControl (
    VOID
    );

VOID
KiInterruptDispatchNoLock (
    VOID
    );

VOID
KiInterruptDispatchNoEOI (
    VOID
    );

VOID
KiWaitForReboot (
    VOID
    );

__forceinline
BOOLEAN
KiSwapProcess (
    IN PKPROCESS NewProcess,
    IN PKPROCESS OldProcess
    )

/*++

Routine Description:

    This function swaps the address space to another process by flushing the
    the translation buffer and establishings a new directory table base. It
    also swaps the I/O permission map to the new process.

    N.B. There is code similar to this code in swap context.

    N.B. This code is executed at DPC level.

Arguments:

    NewProcess - Supplies a pointer to the new process object.

    Oldprocess - Supplies a pointer to the old process object.

Return Value:

    None.

--*/

{

    //
    // Clear the processor bit in the old process.
    //

#if !defined(NT_UP)

    PKPRCB Prcb;
    KAFFINITY SetMember;

    Prcb = KeGetCurrentPrcb();
    SetMember = Prcb->SetMember;
    InterlockedXor64((LONG64 volatile *)&OldProcess->ActiveProcessors, SetMember);

    ASSERT((OldProcess->ActiveProcessors & SetMember) == 0);

    //
    // Set the processor bit in the new process.
    //

    InterlockedXor64((LONG64 volatile *)&NewProcess->ActiveProcessors, SetMember);

    ASSERT((NewProcess->ActiveProcessors & SetMember) != 0);

#endif

    //
    // Load the new directory table base.
    //

    WriteCR3(NewProcess->DirectoryTableBase[0]);

#if defined(NT_UP)

    UNREFERENCED_PARAMETER(OldProcess);

#endif // !defined(NT_UP)

    return TRUE;
}

#if !defined(NT_UP)

FORCEINLINE
BOOLEAN
KiCheckForFreezeExecution (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function checks for a pending freeze execution request from
    another processor.  If such a request is pending then execution is
    frozen.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

Return Value:

    None.

--*/

{

    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();
    if (Prcb->IpiFrozen == TARGET_FREEZE) {
        KiFreezeTargetExecution(TrapFrame, ExceptionFrame);
        if (KiResumeForReboot != FALSE) {

            //
            // If previous mode was user, then set the code and stack
            // selectors and the stack pointer to valid kernel values.
            // 

            if (TrapFrame->SegCs != KGDT64_R0_CODE) {
                TrapFrame->SegCs = KGDT64_R0_CODE;
                TrapFrame->SegSs = KGDT64_R0_DATA;
                TrapFrame->Rsp = Prcb->RspBase;
            }

            TrapFrame->Rip = (ULONG64)KiWaitForReboot;

        }
        return TRUE;

    } else {
        return FALSE;
    }
}

#endif

//
// Define thread startup routine prototypes.
//

VOID
KiStartSystemThread (
    VOID
    );

VOID
KiStartUserThread (
    VOID
    );

VOID
KiStartUserThreadReturn (
    VOID
    );

PXMM_SAVE_AREA32
KxContextToKframes (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextRecord,
    IN ULONG ContextFlags,
    IN KPROCESSOR_MODE PreviousMode
    );

//
// Define unexpected interrupt structure and table.
//

#pragma pack(1)
typedef DECLSPEC_ALIGN(16)  struct _UNEXPECTED_INTERRUPT {
    UCHAR PushImmOp;
    ULONG PushImm;
    UCHAR PushRbp;
    UCHAR JmpOp;
    LONG JmpOffset;
} UNEXPECTED_INTERRUPT, *PUNEXPECTED_INTERRUPT;
#pragma pack()

extern UNEXPECTED_INTERRUPT KxUnexpectedInterrupt0[256];

#define PPI_BITS    2
#define PDI_BITS    9
#define PTI_BITS    9

#define PDI_MASK    ((1 << PDI_BITS) - 1)
#define PTI_MASK    ((1 << PTI_BITS) - 1)

#define KiGetPpeIndex(va) ((((ULONG)(va)) >> PPI_SHIFT) & PPI_MASK)
#define KiGetPdeIndex(va) ((((ULONG)(va)) >> PDI_SHIFT) & PDI_MASK)
#define KiGetPteIndex(va) ((((ULONG)(va)) >> PTI_SHIFT) & PTI_MASK)

extern KSPIN_LOCK KiNMILock;
extern ULONG KeAmd64MachineType;
extern ULONG KiLogicalProcessors;
extern ULONG KiMxCsrMask;
extern ULONG KiPhysicalProcessors;
extern UCHAR KiPrefetchRetry;
extern ULONG64 KiTestDividend;

#endif // !defined(_KIAMD64_)

