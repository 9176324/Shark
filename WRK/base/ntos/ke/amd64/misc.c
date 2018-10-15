/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    misc.c

Abstract:

    This module implements machine dependent miscellaneous kernel functions.

--*/

#include "ki.h"

PKPRCB
KeQueryPrcbAddress (
    __in ULONG Number
    )

/*++

Routine Description:

    This function returns the processor block address for the specified
    processor number.

Arguments:

    Number - Supplies the number of the processor.

Return Value:

    The processor block address for the specified processor.

--*/

{
    ASSERT(Number <= (ULONG)KeNumberProcessors);

    return KiProcessorBlock[Number];
}

VOID
KeRestoreProcessorSpecificFeatures (
    VOID
    )

/*++

Routine Description:

    Restore processor specific features.  This routine is called
    when processors have been restored to a powered on state to
    restore those things which are not part of the processor's
    "normal" context which may have been lost.  For example, this
    routine is called when a system is resumed from hibernate or
    suspend.

Arguments:

    None.

Return Value:

    None.

--*/

{

    CPU_INFO InformationExtended;

    // 
    // Restore the page attribute table for the current processor.
    // 

    KiSetPageAttributesTable();

    //
    // Check for fast floating/save restore and, if present, enable for the
    // current processor.
    //

    KiCpuId(0x80000001, 0, &InformationExtended);
    if ((InformationExtended.Edx & XHF_FFXSR) != 0) {
        WriteMSR(MSR_EFER, ReadMSR(MSR_EFER) | MSR_FFXSR);
    }

    return;
}

VOID
KeSaveStateForHibernate (
    __out PKPROCESSOR_STATE ProcessorState
    )

/*++

Routine Description:

    Saves all processor-specific state that must be preserved
    across an S4 state (hibernation).

Arguments:

    ProcessorState - Supplies the KPROCESSOR_STATE where the current CPU's
        state is to be saved.

Return Value:

    None.

--*/

{

    //
    // Save processor specific state for hibernation.
    //

    RtlCaptureContext(&ProcessorState->ContextFrame);
    ProcessorState->SpecialRegisters.MsrGsBase = ReadMSR(MSR_GS_BASE);
    ProcessorState->SpecialRegisters.MsrGsSwap = ReadMSR(MSR_GS_SWAP);
    ProcessorState->SpecialRegisters.MsrStar = ReadMSR(MSR_STAR);
    ProcessorState->SpecialRegisters.MsrLStar = ReadMSR(MSR_LSTAR);
    ProcessorState->SpecialRegisters.MsrCStar = ReadMSR(MSR_CSTAR);
    ProcessorState->SpecialRegisters.MsrSyscallMask = ReadMSR(MSR_SYSCALL_MASK);
    ProcessorState->ContextFrame.Rip = (ULONG_PTR)_ReturnAddress();
    ProcessorState->ContextFrame.Rsp = (ULONG_PTR)&ProcessorState;
    KiSaveProcessorControlState(ProcessorState);
    return;
}

VOID
KiProcessNMI (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame
    )

/*++

Routine Description:

    This function processes a nonmaskable interrupt (NMI).

    N.B. This function is called from the NMI trap routine with interrupts
         disabled.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame.

    ExceptionFrame - Supplies a pointer to an exception frame.

Return Value:

    None.

--*/

{
    static LONG64 NmiInProgress = 0;
    ULONG Number;

    //
    // First check for a pending freeze execution request from another
    // processor.
    //

#if !defined(NT_UP)

    if (KiCheckForFreezeExecution(TrapFrame, ExceptionFrame) != FALSE) {
        return;
    }

#endif

    //
    // Return immediately if this processor is already processing an NMI.
    //

    Number = KeGetCurrentPrcb()->Number;
    if (InterlockedBitTestAndSet64(&NmiInProgress, Number) != FALSE) {
        return;
    }

    //
    // Process NMI callback functions.
    //
    // If no callback function handles the NMI, then let the HAL handle it.
    //

    if (KiHandleNmi() == FALSE) {
        KiAcquireSpinLockCheckForFreeze(&KiNMILock, TrapFrame, ExceptionFrame);
        KeBugCheckActive = TRUE;
        HalHandleNMI(NULL);
        KeBugCheckActive = FALSE;
        KeReleaseSpinLockFromDpcLevel(&KiNMILock);
    }

    InterlockedBitTestAndReset64(&NmiInProgress, Number);

    return;
}

#if !defined(NT_UP)

VOID
KiWaitForReboot (
    VOID
    )

/*++

Routine Description:

    Frozen processors are resumed to this routine in the event that
    a .reboot is being processed.

Arguments:

    None.

Return Value:

    Never returns.

--*/

{
    while (TRUE) {
        NOTHING;
    }
}

#endif

