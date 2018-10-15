/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    exceptn.c

Abstract:

    This module implement the code necessary to dispatch exceptions to the
    proper mode and invoke the exception dispatcher.

--*/

#include "ki.h"
#include "kix86.h"

#define FN_BITS_PER_TAGWORD     16
#define FN_TAG_EMPTY            0x3
#define FN_TAG_MASK             0x3
#define FX_TAG_VALID            0x1
#define NUMBER_OF_FP_REGISTERS  8
#define BYTES_PER_FP_REGISTER   10
#define BYTES_PER_FX_REGISTER   16

extern UCHAR VdmUserCr0MapIn[];
extern BOOLEAN KeI386FxsrPresent;
extern BOOLEAN KeI386XMMIPresent;

BOOLEAN
KiCheckForAtlThunk (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context
    );

VOID
Ki386AdjustEsp0(
    IN PKTRAP_FRAME TrapFrame
    );

BOOLEAN
KiEm87StateToNpxFrame(
    OUT PFLOATING_SAVE_AREA NpxFrmae
    );

BOOLEAN
KiNpxFrameToEm87State(
    IN PFLOATING_SAVE_AREA NpxFrmae
    );

ULONG
KiEspFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine fetches the correct esp from a trapframe, accounting
    for whether the frame is a user or kernel mode frame, and whether
    it has been edited.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile context
        should be copied into the context record.

Return Value:

    Value of Esp.

--*/

{
    if (((TrapFrame->SegCs & MODE_MASK) != KernelMode) ||
         (TrapFrame->EFlags & EFLAGS_V86_MASK)) {

        //  User mode frame, real value of Esp is always in HardwareEsp.

        return TrapFrame->HardwareEsp;

    } else {

        if ((TrapFrame->SegCs & FRAME_EDITED) == 0) {

            //  Kernel mode frame which has had esp edited,
            //  value of Esp is in TempEsp.

            return TrapFrame->TempEsp;

        } else {

            //  Kernel mode frame has has not had esp edited, compute esp.

            return (ULONG)&TrapFrame->HardwareEsp;
        }
    }
}

VOID
KiEspToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG Esp
    )

/*++

Routine Description:

    This routine sets the specified value Esp into the trap frame,
    accounting for whether the frame is a user or kernel mode frame,
    and whether it has been edited before.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile context
        should be copied into the context record.

    Esp - New value for Esp.

Return Value:

    None.

--*/
{
    ULONG   OldEsp;
    KIRQL   OldIrql;

    //
    // Raise to at least APC_LEVEL so we can test SegCs and EFLAGS
    // as an atomic operation since NtSetContextThread may modify these.
    //
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

    OldEsp = KiEspFromTrapFrame(TrapFrame);

    if (((TrapFrame->SegCs & MODE_MASK) != KernelMode) ||
         (TrapFrame->EFlags & EFLAGS_V86_MASK)) {

        //
        //  User mode trap frame
        //

        TrapFrame->HardwareEsp = Esp;

    } else {

        //
        //  Kernel mode esp can't be lowered or iret emulation will fail
        //

        if (Esp < OldEsp)
            KeBugCheckEx(SET_OF_INVALID_CONTEXT,
                         Esp, OldEsp, (ULONG_PTR)TrapFrame, 0);

        //
        //  Edit frame, setting edit marker as needed.
        //

        if ((TrapFrame->SegCs & FRAME_EDITED) == 0) {

            //  Kernel frame that has already been edited,
            //  store value in TempEsp.

            TrapFrame->TempEsp = Esp;

        } else {

            //  Kernel frame for which Esp is being edited first time.
            //  Save real SegCs, set marked in SegCs, save Esp value.

            if (OldEsp != Esp) {

                TrapFrame->TempSegCs = TrapFrame->SegCs;
                TrapFrame->SegCs = TrapFrame->SegCs & ~FRAME_EDITED;
                TrapFrame->TempEsp = Esp;
            }
        }
    }

    //
    // Restore original IRQL
    //
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

ULONG
KiSegSsFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine fetches the correct ss from a trapframe, accounting
    for whether the frame is a user or kernel mode frame.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile context
        should be copied into the context record.

Return Value:

    Value of SegSs.

--*/

{
    if (TrapFrame->EFlags & EFLAGS_V86_MASK){
        return TrapFrame->HardwareSegSs;
    } else if ((TrapFrame->SegCs & MODE_MASK) != KernelMode) {

        //
        // It's user mode.  The HardwareSegSs contains R3 data selector.
        //

        return TrapFrame->HardwareSegSs | RPL_MASK;
    } else {
        return KGDT_R0_DATA;
    }
}

VOID
KiSegSsToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG SegSs
    )

/*++

Routine Description:

    It turns out that in a flat system there are only two legal values
    for SS.  Therefore, this procedure forces the appropriate one
    of those values to be used.  The legal SS value is a function of
    which CS value is already set.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile context
        should be copied into the context record.

    SegSs - value of SS caller would like to set.

Return Value:

    Nothing.

--*/

{
    SegSs &= SEGMENT_MASK;  // Throw away the high order trash bits

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        TrapFrame->HardwareSegSs = SegSs;
    } else if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {

        //
        // If user mode, we simply put SegSs to trapframe.  If the SegSs
        // is a bogus value.  The trap0d handler will be able to detect
        // this and handle it appropriately.
        //

        TrapFrame->HardwareSegSs = SegSs | RPL_MASK;
    }

    //
    //  else {
    //      The frame is a kernel mode frame, which does not have
    //      a place to store SS.  Therefore, do nothing.
    //
}

VOID
KeContextFromKframes (
    __in PKTRAP_FRAME TrapFrame,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __inout PCONTEXT ContextFrame
    )

/*++

Routine Description:

    This routine moves the selected contents of the specified trap and exception frames
    frames into the specified context frame according to the specified context
    flags.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile context
        should be copied into the context record.

    ExceptionFrame - Supplies a pointer to an exception frame from which context
        should be copied into the context record. This argument is ignored since
        there is no exception frame on NT386.

    ContextFrame - Supplies a pointer to the context frame that receives the
        context copied from the trap and exception frames.

Return Value:

    None.

--*/

{

    PFX_SAVE_AREA NpxFrame;
    BOOLEAN StateSaved;
    ULONG i;
    struct _FPSaveBuffer {
        UCHAR               Buffer[15];
        FLOATING_SAVE_AREA  SaveArea;
    } FloatSaveBuffer;
    PFLOATING_SAVE_AREA PSaveArea;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER( ExceptionFrame );

    //
    // This routine is called at both PASSIVE_LEVEL by exception dispatch
    // and at APC_LEVEL by NtSetContextThread. We raise to APC_LEVEL to
    // make the trap frame capture atomic.
    //
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

    //
    // Set control information if specified.
    //

    if ((ContextFrame->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        //
        // Set registers ebp, eip, cs, eflag, esp and ss.
        //

        ContextFrame->Ebp = TrapFrame->Ebp;
        ContextFrame->Eip = TrapFrame->Eip;

        if (((TrapFrame->SegCs & FRAME_EDITED) == 0) &&
            ((TrapFrame->EFlags & EFLAGS_V86_MASK) == 0)) {
            ContextFrame->SegCs = TrapFrame->TempSegCs & SEGMENT_MASK;
        } else {
            ContextFrame->SegCs = TrapFrame->SegCs & SEGMENT_MASK;
        }
        ContextFrame->EFlags = TrapFrame->EFlags;
        ContextFrame->SegSs = KiSegSsFromTrapFrame(TrapFrame);
        ContextFrame->Esp = KiEspFromTrapFrame(TrapFrame);
    }

    //
    // Set segment register contents if specified.
    //

    if ((ContextFrame->ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

        //
        // Set segment registers gs, fs, es, ds.
        //
        // These values are junk most of the time, but useful
        // for debugging under certain conditions.  Therefore,
        // we report whatever was in the frame.
        //
        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            ContextFrame->SegGs = TrapFrame->V86Gs & SEGMENT_MASK;
            ContextFrame->SegFs = TrapFrame->V86Fs & SEGMENT_MASK;
            ContextFrame->SegEs = TrapFrame->V86Es & SEGMENT_MASK;
            ContextFrame->SegDs = TrapFrame->V86Ds & SEGMENT_MASK;
        }
        else {
            if (TrapFrame->SegCs == KGDT_R0_CODE) {
                //
                // Trap frames created from R0_CODE traps do not save
                // the following selectors.  Set them in the frame now.
                //

                TrapFrame->SegGs = 0;
                TrapFrame->SegFs = KGDT_R0_PCR;
                TrapFrame->SegEs = KGDT_R3_DATA | RPL_MASK;
                TrapFrame->SegDs = KGDT_R3_DATA | RPL_MASK;
            }

            ContextFrame->SegGs = TrapFrame->SegGs & SEGMENT_MASK;
            ContextFrame->SegFs = TrapFrame->SegFs & SEGMENT_MASK;
            ContextFrame->SegEs = TrapFrame->SegEs & SEGMENT_MASK;
            ContextFrame->SegDs = TrapFrame->SegDs & SEGMENT_MASK;
        }

    }

    //
    // Set integer register contents if specified.
    //

    if ((ContextFrame->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        //
        // Set integer registers edi, esi, ebx, edx, ecx, eax
        //

        ContextFrame->Edi = TrapFrame->Edi;
        ContextFrame->Esi = TrapFrame->Esi;
        ContextFrame->Ebx = TrapFrame->Ebx;
        ContextFrame->Ecx = TrapFrame->Ecx;
        ContextFrame->Edx = TrapFrame->Edx;
        ContextFrame->Eax = TrapFrame->Eax;
    }

    if (((ContextFrame->ContextFlags & CONTEXT_EXTENDED_REGISTERS) ==
        CONTEXT_EXTENDED_REGISTERS) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

        //
        // This is the base TrapFrame, and the NpxFrame is on the base
        // of the kernel stack, just above it in memory.
        //

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {
            KiFlushNPXState (NULL);
            RtlCopyMemory( (PVOID)&(ContextFrame->ExtendedRegisters[0]),
                           (PVOID)&(NpxFrame->U.FxArea),                    
                           MAXIMUM_SUPPORTED_EXTENSION
                         );
        }
    }

    //
    // Fetch floating register contents if requested, and type of target
    // is user.  (system frames have no fp state, so ignore request)
    //
    if ( ((ContextFrame->ContextFlags & CONTEXT_FLOATING_POINT) ==
          CONTEXT_FLOATING_POINT) &&
         ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

        //
        // This is the base TrapFrame, and the NpxFrame is on the base
        // of the kernel stack, just above it in memory.
        //

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {

            //
            // Force the coprocessors state to the save area and copy it
            // to the context frame.
            //

            if (KeI386FxsrPresent == TRUE) {

                //
                // FP state save was done using fxsave. Get the save
                // area in fnsave format
                //
                // Save area must be 16 byte aligned so we cushion it with
                // 15 bytes (in the locals declaration above) and round
                // down to align.
                //

                ULONG_PTR Temp;
                Temp = (ULONG_PTR)&FloatSaveBuffer.SaveArea;
                Temp &= ~0xf;
                PSaveArea = (PFLOATING_SAVE_AREA)Temp;
                KiFlushNPXState (PSaveArea);
            } else {

                PSaveArea = (PFLOATING_SAVE_AREA)&(NpxFrame->U.FnArea);
                KiFlushNPXState (NULL);

            }

            ContextFrame->FloatSave.ControlWord   = PSaveArea->ControlWord;
            ContextFrame->FloatSave.StatusWord    = PSaveArea->StatusWord;
            ContextFrame->FloatSave.TagWord       = PSaveArea->TagWord;
            ContextFrame->FloatSave.ErrorOffset   = PSaveArea->ErrorOffset;
            ContextFrame->FloatSave.ErrorSelector = PSaveArea->ErrorSelector;
            ContextFrame->FloatSave.DataOffset    = PSaveArea->DataOffset;
            ContextFrame->FloatSave.DataSelector  = PSaveArea->DataSelector;
            ContextFrame->FloatSave.Cr0NpxState   = NpxFrame->Cr0NpxState;

            for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
                ContextFrame->FloatSave.RegisterArea[i] = PSaveArea->RegisterArea[i];
            }

        } else {

            //
            // The 80387 is being emulated by the R3 emulator.
            // ** The only time the Npx state is ever obtained or set is
            // ** for userlevel handling.  Current Irql must be 0 or 1.
            // Go slurp the emulator's R3 data and generate the
            // floating point context
            //

            StateSaved = KiEm87StateToNpxFrame(&ContextFrame->FloatSave);
            if (StateSaved) {
                ContextFrame->FloatSave.Cr0NpxState = NpxFrame->Cr0NpxState;
            } else {

                //
                // The floatingpoint state can not be determined.
                // Remove the floatingpoint flag from the context frame flags.
                //

                ContextFrame->ContextFlags &= (~CONTEXT_FLOATING_POINT) | CONTEXT_i386;
            }
        }
    }

    //
    // Fetch Dr register contents if requested.
    //

    if ((ContextFrame->ContextFlags & CONTEXT_DEBUG_REGISTERS) ==
        CONTEXT_DEBUG_REGISTERS) {
         
        //
        // Care is now taken to ensure that the DebugActive/Dr7 value is set on
        // any valid set of a legal DR value, ensuring the values on the kernel
        // stack cannot become trash.
        //

        if ((TrapFrame->Dr7 & ~DR7_RESERVED_MASK) != 0) {
            ContextFrame->Dr0 = TrapFrame->Dr0;
            ContextFrame->Dr1 = TrapFrame->Dr1;
            ContextFrame->Dr2 = TrapFrame->Dr2;
            ContextFrame->Dr3 = TrapFrame->Dr3;
            ContextFrame->Dr6 = TrapFrame->Dr6;
            ContextFrame->Dr7 = KiUpdateDr7 (TrapFrame->Dr7);
        } else {
            ContextFrame->Dr0 = 0;
            ContextFrame->Dr1 = 0;
            ContextFrame->Dr2 = 0;
            ContextFrame->Dr3 = 0;
            ContextFrame->Dr6 = 0;
            ContextFrame->Dr7 = 0;
        }
    }

    //
    // Lower IRQL if we had to raise it
    //
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

VOID
KeContextToKframes (
    __inout PKTRAP_FRAME TrapFrame,
    __inout PKEXCEPTION_FRAME ExceptionFrame,
    __in PCONTEXT ContextFrame,
    __in ULONG ContextFlags,
    __in KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    This routine moves the selected contents of the specified context frame into
    the specified trap and exception frames according to the specified context
    flags.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame that receives the volatile
        context from the context record.

    ExceptionFrame - Supplies a pointer to an exception frame that receives
        the nonvolatile context from the context record. This argument is
        ignored since there is no exception frame on NT386.

    ContextFrame - Supplies a pointer to a context frame that contains the
        context that is to be copied into the trap and exception frames.

    ContextFlags - Supplies the set of flags that specify which parts of the
        context frame are to be copied into the trap and exception frames.

    PreviousMode - Supplies the processor mode for which the trap and exception
        frames are being built.

Return Value:

    None.

--*/

{

    PFX_SAVE_AREA     NpxFrame;
    ULONG i;
    ULONG j;
    ULONG TagWord;
    BOOLEAN StateSaved;
    BOOLEAN ModeChanged;
    KIRQL   OldIrql;
#if DBG
    PKPCR   Pcr;
#endif
#if DBG
    KIRQL   OldIrql2;
#endif

    UNREFERENCED_PARAMETER( ExceptionFrame );

    //
    // This routine is called at both PASSIVE_LEVEL by exception dispatch
    // and at APC_LEVEL by NtSetContextThread. We raise to APC_LEVEL to
    // make the trap frame modifications atomic.
    //
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

    //
    // Set control information if specified.
    //

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        if ((ContextFrame->EFlags & EFLAGS_V86_MASK) !=
            (TrapFrame->EFlags & EFLAGS_V86_MASK)) {
            ModeChanged = TRUE;
        } else {
            ModeChanged = FALSE;
        }


        //
        // Set registers eflag, ebp, eip, cs, esp and ss.
        // Eflags is set first, so that the auxiliary routines
        // can check the v86 bit to determine as well as cs, to
        // determine if the frame is kernel or user mode. (v86 mode cs
        // can have any value)
        //

        TrapFrame->EFlags = SANITIZE_FLAGS(ContextFrame->EFlags, PreviousMode);
        TrapFrame->Ebp = ContextFrame->Ebp;
        TrapFrame->Eip = ContextFrame->Eip;
        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            TrapFrame->SegCs = ContextFrame->SegCs;
        } else {
            TrapFrame->SegCs = SANITIZE_SEG(ContextFrame->SegCs, PreviousMode);
            if (PreviousMode != KernelMode && TrapFrame->SegCs < 8) {

                //
                // If user mode and the selector value is less than 8, we
                // know it is an invalid selector.  Set it to flat user
                // mode selector.  Another reason we need to check for this
                // is that any cs value less than 8 causes our exit kernel
                // macro to treat its exit trap frame as an edited frame.
                //

                TrapFrame->SegCs = KGDT_R3_CODE | RPL_MASK;
            }
        }


        KiSegSsToTrapFrame(TrapFrame, ContextFrame->SegSs);
        KiEspToTrapFrame(TrapFrame, ContextFrame->Esp);
        if (ModeChanged) {
            Ki386AdjustEsp0(TrapFrame);             // realign esp0 in the tss
        }
    }

    //
    // Set segment register contents if specified.
    //

    if ((ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

        //
        // Set segment registers gs, fs, es, ds.
        //

        //
        // There's only one legal value for DS and ES, so simply set it.
        // This allows KeContextFromKframes to report the real values in
        // the frame. (which are junk most of the time, but sometimes useful
        // for debugging.)
        // Only 2 legal values for FS, let either one be set.
        // Force GS to be 0 to deal with entry via SysCall and exit
        // via exception.
        //
        // For V86 mode, the FS, GS, DS, and ES registers must be properly
        // set from the supplied context.
        //

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            TrapFrame->V86Fs = ContextFrame->SegFs;
            TrapFrame->V86Es = ContextFrame->SegEs;
            TrapFrame->V86Ds = ContextFrame->SegDs;
            TrapFrame->V86Gs = ContextFrame->SegGs;
        } else if (((TrapFrame->SegCs & MODE_MASK) == KernelMode)) {

            //
            // set up the standard selectors
            //

            TrapFrame->SegFs = SANITIZE_SEG(ContextFrame->SegFs, PreviousMode);
            TrapFrame->SegEs = KGDT_R3_DATA | RPL_MASK;
            TrapFrame->SegDs = KGDT_R3_DATA | RPL_MASK;
            TrapFrame->SegGs = 0;
        } else {

            //
            // If user mode, we simply return whatever left in context frame
            // and let trap 0d handle it (if later we trap while popping the
            // trap frame.) V86 mode also get handled here.
            //

            TrapFrame->SegFs = ContextFrame->SegFs;
            TrapFrame->SegEs = ContextFrame->SegEs;
            TrapFrame->SegDs = ContextFrame->SegDs;
            if (TrapFrame->SegCs == (KGDT_R3_CODE | RPL_MASK)) {
                TrapFrame->SegGs = 0;
            } else {
                TrapFrame->SegGs = ContextFrame->SegGs;
            }
        }
    }
    //
    // Set integer registers contents if specified.
    //

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        //
        // Set integer registers edi, esi, ebx, edx, ecx, eax.
        //
        //  Can NOT call RtlCopyMemory here because the regs aren't
        //  contiguous in pusha frame, and we don't want to export
        //  bits of junk into context record.
        //

        TrapFrame->Edi = ContextFrame->Edi;
        TrapFrame->Esi = ContextFrame->Esi;
        TrapFrame->Ebx = ContextFrame->Ebx;
        TrapFrame->Ecx = ContextFrame->Ecx;
        TrapFrame->Edx = ContextFrame->Edx;
        TrapFrame->Eax = ContextFrame->Eax;

    }

    //
    // Set extended register contents if requested, and type of target
    // is user.  (system frames have no extended state, so ignore request)
    //

    if (((ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

        //
        // This is the base TrapFrame, and the NpxFrame is on the base
        // of the kernel stack, just above it in memory.
        //

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {
            KiFlushNPXState (NULL);
            RtlCopyMemory( (PVOID)&(NpxFrame->U.FxArea),
                      (PVOID)&(ContextFrame->ExtendedRegisters[0]),
                           MAXIMUM_SUPPORTED_EXTENSION
                         );
            //
            // Make sure only valid floating state bits are moved to Cr0NpxState.
            //

            NpxFrame->Cr0NpxState &= ~(CR0_EM | CR0_MP | CR0_TS);

            //
            // Make sure all reserved bits are clear in MXCSR so we don't get a GP
            // fault when doing an FRSTOR on this state.
            //
            NpxFrame->U.FxArea.MXCsr = SANITIZE_MXCSR(NpxFrame->U.FxArea.MXCsr);

            //
            // Only let VDMs turn on the EM bit.  The kernel can't do
            // anything for FLAT apps
            //
            if (PsGetCurrentProcess()->VdmObjects != NULL) {
                NpxFrame->Cr0NpxState |= ContextFrame->FloatSave.Cr0NpxState &
                                      (CR0_EM | CR0_MP);
            }
        }
    }

    //
    // Set floating register contents if requested, and type of target
    // is user.  (system frames have no fp state, so ignore request)
    //

    if (((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

        //
        // This is the base TrapFrame, and the NpxFrame is on the base
        // of the kernel stack, just above it in memory.
        //

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {

            //
            // Set coprocessor stack, control and status registers
            //

            KiFlushNPXState (NULL);

            if (KeI386FxsrPresent == TRUE) {

                //
                // Restore FP state in the fxrstor format
                //

                NpxFrame->U.FxArea.ControlWord   =
                                    (USHORT)ContextFrame->FloatSave.ControlWord;
                NpxFrame->U.FxArea.StatusWord    =
                                    (USHORT)ContextFrame->FloatSave.StatusWord;

                //
                // Construct the tag word from fnsave format to fxsave format
                //

                NpxFrame->U.FxArea.TagWord = 0; // Mark every register invalid

                TagWord = ContextFrame->FloatSave.TagWord;

                for (i = 0; i < FN_BITS_PER_TAGWORD; i+=2) {

                    if (((TagWord >> i) & FN_TAG_MASK) != FN_TAG_EMPTY) {

                        //
                        // This register is valid
                        //

                        NpxFrame->U.FxArea.TagWord |= (FX_TAG_VALID << (i/2));
                    }
                }

                NpxFrame->U.FxArea.ErrorOffset   =
                                        ContextFrame->FloatSave.ErrorOffset;
                NpxFrame->U.FxArea.ErrorSelector =
                               (ContextFrame->FloatSave.ErrorSelector & 0xFFFF);
                NpxFrame->U.FxArea.ErrorOpcode =
                    (USHORT)((ContextFrame->FloatSave.ErrorSelector >> 16) & 0xFFFF);
                NpxFrame->U.FxArea.DataOffset    =
                                ContextFrame->FloatSave.DataOffset;
                NpxFrame->U.FxArea.DataSelector  =
                                ContextFrame->FloatSave.DataSelector;

                //
                // Fxrstor format has each FP register in 128 bits (16 bytes)
                // where as fnsave saves each FP register in 80 bits (10 bytes)
                //
                RtlZeroMemory ((PVOID)&NpxFrame->U.FxArea.RegisterArea[0],
                               SIZE_OF_FX_REGISTERS
                              );

                for (i = 0; i < NUMBER_OF_FP_REGISTERS; i++) {
                    for (j = 0; j < BYTES_PER_FP_REGISTER; j++) {
                        NpxFrame->U.FxArea.RegisterArea[i*BYTES_PER_FX_REGISTER+j] =
                                ContextFrame->FloatSave.RegisterArea[i*BYTES_PER_FP_REGISTER+j];
                    }
                }

            } else {
                NpxFrame->U.FnArea.ControlWord   =
                                        ContextFrame->FloatSave.ControlWord;
                NpxFrame->U.FnArea.StatusWord    =
                                        ContextFrame->FloatSave.StatusWord;
                NpxFrame->U.FnArea.TagWord       =
                                        ContextFrame->FloatSave.TagWord;
                NpxFrame->U.FnArea.ErrorOffset   =
                                        ContextFrame->FloatSave.ErrorOffset;
                NpxFrame->U.FnArea.ErrorSelector =
                                        ContextFrame->FloatSave.ErrorSelector;
                NpxFrame->U.FnArea.DataOffset    =
                                        ContextFrame->FloatSave.DataOffset;
                NpxFrame->U.FnArea.DataSelector  =
                                        ContextFrame->FloatSave.DataSelector;

                for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
                    NpxFrame->U.FnArea.RegisterArea[i] =
                            ContextFrame->FloatSave.RegisterArea[i];
                }

            }

            //
            // Make sure only valid floating state bits are moved to Cr0NpxState.
            //

            NpxFrame->Cr0NpxState &= ~(CR0_EM | CR0_MP | CR0_TS);

            //
            // Only let VDMs turn on the EM bit.  The kernel can't do
            // anything for FLAT apps
            //
            if (PsGetCurrentProcess()->VdmObjects != NULL) {
                NpxFrame->Cr0NpxState |= ContextFrame->FloatSave.Cr0NpxState &
                                      (CR0_EM | CR0_MP);
            }

        } else {

            if (PsGetCurrentProcess()->VdmObjects != NULL) {

                //
                // This is a special hack to allow SetContext for VDMs to
                // turn on/off it's CR0_EM bit.
                //

                NpxFrame->Cr0NpxState &= ~(CR0_MP | CR0_TS | CR0_EM | CR0_PE);
                NpxFrame->Cr0NpxState |=
                    VdmUserCr0MapIn[ContextFrame->FloatSave.Cr0NpxState & (CR0_EM | CR0_MP)];

            } else {

                //
                // The 80387 is being emulated by the R3 emulator.
                // ** The only time the Npx state is ever obtained or set is
                // ** for userlevel handling.  Current Irql must be 0 or 1.
                // And the context being set must be for the current thread.
                // Go smash the floatingpoint context into the R3 emulator's
                // data area.
                //
#if DBG
                OldIrql2 = KeRaiseIrqlToSynchLevel();
                Pcr = KeGetPcr();
                ASSERT (Pcr->Prcb->CurrentThread->Teb == Pcr->NtTib.Self);
                KeLowerIrql (OldIrql2);
#endif

                StateSaved = KiNpxFrameToEm87State(&ContextFrame->FloatSave);
                if (StateSaved) {

                    //
                    // Make sure only valid floating state bits are moved to
                    // Cr0NpxState.  Since we are emulating, don't allow
                    // resetting CR0_EM.
                    //

                    NpxFrame->Cr0NpxState &= ~(CR0_MP | CR0_TS);
                    NpxFrame->Cr0NpxState |=
                        ContextFrame->FloatSave.Cr0NpxState & CR0_MP;
                }
            }
        }
    }

    //
    // Set debug register state if specified.  If previous mode is user
    // mode (i.e. it's a user frame we're setting) and if effect will be to
    // cause any debug register to obtain a valid value, then mark 
    // the thread and the Dr7 field accordingly.
    //

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {
        UCHAR NewMask;
        ULONG Index;
        ULONG DrValue;

        NewMask = 0;

        //
        // Process debug registers Dr0-Dr3, Dr6, and finally Dr7.
        //
        
        for (Index = 0; Index <=3; Index += 1) {
            DrValue = SANITIZE_DRADDR (*((PULONG)((ULONG_PTR)ContextFrame + 
                KiDebugRegisterContextOffsets[Index])), PreviousMode);
            
            if ((*((PULONG)((ULONG_PTR)TrapFrame + KiDebugRegisterTrapOffsets[Index])) = DrValue) != 0)
            {
                NewMask |= DR_MASK (Index);
            }
        }
        
        if ((TrapFrame->Dr6 = SANITIZE_DR6 (ContextFrame->Dr6, PreviousMode)) != 0) {
            NewMask |= DR_MASK (6);
        }

        TrapFrame->Dr7 = SANITIZE_DR7(ContextFrame->Dr7, PreviousMode);
        (VOID) KiRecordDr7 (&TrapFrame->Dr7, &NewMask);

        if (PreviousMode != KernelMode) {
            KeGetCurrentThread()->Header.DebugActive = (BOOLEAN) NewMask;
        }
    }

    //
    // If thread is supposed to have IOPL, then force it on in eflags
    //
    if (KeGetCurrentThread()->Iopl) {
        TrapFrame->EFlags |= (EFLAGS_IOPL_MASK & -1);  // IOPL = 3
    }

    //
    // Lower IRQL if we had to raise it
    //
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }

    return;
}

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    )

/*++

Routine Description:

    This function is called to dispatch an exception to the proper mode and
    to cause the exception dispatcher to be called. If the previous mode is
    kernel, then the exception dispatcher is called directly to process the
    exception. Otherwise the exception record, exception frame, and trap
    frame contents are copied to the user mode stack. The contents of the
    exception frame and trap are then modified such that when control is
    returned, execution will commense in user mode in a routine which will
    call the exception dispatcher.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ExceptionFrame - Supplies a pointer to an exception frame. For NT386,
        this should be NULL.

    TrapFrame - Supplies a pointer to a trap frame.

    PreviousMode - Supplies the previous processor mode.

    FirstChance - Supplies a boolean value that specifies whether this is
        the first (TRUE) or second (FALSE) chance for the exception.

Return Value:

    None.

--*/

{
    CONTEXT ContextFrame;
    EXCEPTION_RECORD ExceptionRecord1, ExceptionRecord2;
    LONG Length;
    ULONG UserStack1;
    ULONG UserStack2;

    //
    // Move machine state from trap and exception frames to a context frame,
    // and increment the number of exceptions dispatched.
    //

    KeGetCurrentPrcb()->KeExceptionDispatchCount += 1;
    ContextFrame.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

    if ((PreviousMode == UserMode) || KdDebuggerEnabled) {
        //
        // For usermode exceptions always try to dispatch the floating
        // point state.  This allows exception handlers & debuggers to
        // examine/edit the npx context if required.  Plus it allows
        // exception handlers to use fp instructions without destroying
        // the npx state at the time of the exception.
        //
        // Note: If there's no 80387, ContextTo/FromKFrames will use the
        // emulator's current state.  If the emulator can not give the
        // current state, then the context_floating_point bit will be
        // turned off by ContextFromKFrames.
        //

        ContextFrame.ContextFlags |= CONTEXT_FLOATING_POINT;
        if (KeI386XMMIPresent) {
            ContextFrame.ContextFlags |= CONTEXT_EXTENDED_REGISTERS;
        }
    }

    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextFrame);

    //
    // if it is BREAK_POINT exception, we subtract 1 from EIP and report
    // the updated EIP to user.  This is because Cruiser requires EIP
    // points to the int 3 instruction (not the instruction following int 3).
    // In this case, BreakPoint exception is fatal. Otherwise we will step
    // on the int 3 over and over again, if user does not handle it
    //
    // if the BREAK_POINT occured in V86 mode, the debugger running in the
    // VDM will expect CS:EIP to point after the exception (the way the
    // processor left it.  this is also true for protected mode dos
    // app debuggers.  We will need a way to detect this.
    //
    //

    switch (ExceptionRecord->ExceptionCode) {
        case STATUS_BREAKPOINT:
            ContextFrame.Eip--;
            break;

        case KI_EXCEPTION_ACCESS_VIOLATION:
            ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
            if (PreviousMode == UserMode) {
                if (KiCheckForAtlThunk(ExceptionRecord,&ContextFrame) != FALSE) {
                    goto Handled1;
                }

                if ((SharedUserData->ProcessorFeatures[PF_NX_ENABLED] == TRUE) &&
                    (ExceptionRecord->ExceptionInformation [0] == EXCEPTION_EXECUTE_FAULT)) {
                    
                    if (((KeFeatureBits & KF_GLOBAL_32BIT_EXECUTE) != 0) ||
                        (PsGetCurrentProcess()->Pcb.Flags.ExecuteEnable != 0) ||
                        (((KeFeatureBits & KF_GLOBAL_32BIT_NOEXECUTE) == 0) &&
                         (PsGetCurrentProcess()->Pcb.Flags.ExecuteDisable == 0))) {
                        ExceptionRecord->ExceptionInformation [0] = 0;
                    }
                }
            }
            break;
    }

    //
    // Select the method of handling the exception based on the previous mode.
    //

    ASSERT ((
             !((PreviousMode == KernelMode) &&
             (ContextFrame.EFlags & EFLAGS_V86_MASK))
           ));

    if (PreviousMode == KernelMode) {

        //
        // Previous mode was kernel.
        //
        // If the kernel debugger is active, then give the kernel debugger the
        // first chance to handle the exception. If the kernel debugger handles
        // the exception, then continue execution. Else attempt to dispatch the
        // exception to a frame based handler. If a frame based handler handles
        // the exception, then continue execution.
        //
        // If a frame based handler does not handle the exception,
        // give the kernel debugger a second chance, if it's present.
        //
        // If the exception is still unhandled, call KeBugCheck().
        //

        if (FirstChance == TRUE) {

            if ((KiDebugRoutine != NULL) &&
               (((KiDebugRoutine) (TrapFrame,
                                   ExceptionFrame,
                                   ExceptionRecord,
                                   &ContextFrame,
                                   PreviousMode,
                                   FALSE)) != FALSE)) {

                goto Handled1;
            }

            // Kernel debugger didn't handle exception.

            if (RtlDispatchException(ExceptionRecord, &ContextFrame) == TRUE) {
                goto Handled1;
            }
        }

        //
        // This is the second chance to handle the exception.
        //

        if ((KiDebugRoutine != NULL) &&
            (((KiDebugRoutine) (TrapFrame,
                                ExceptionFrame,
                                ExceptionRecord,
                                &ContextFrame,
                                PreviousMode,
                                TRUE)) != FALSE)) {

            goto Handled1;
        }

        KeBugCheckEx(
            KERNEL_MODE_EXCEPTION_NOT_HANDLED,
            ExceptionRecord->ExceptionCode,
            (ULONG)ExceptionRecord->ExceptionAddress,
            (ULONG)TrapFrame,
            0);

    } else {

        //
        // Previous mode was user.
        //
        // If this is the first chance and the current process has a debugger
        // port, then send a message to the debugger port and wait for a reply.
        // If the debugger handles the exception, then continue execution. Else
        // transfer the exception information to the user stack, transition to
        // user mode, and attempt to dispatch the exception to a frame based
        // handler. If a frame based handler handles the exception, then continue
        // execution with the continue system service. Else execute the
        // NtRaiseException system service with FirstChance == FALSE, which
        // will call this routine a second time to process the exception.
        //
        // If this is the second chance and the current process has a debugger
        // port, then send a message to the debugger port and wait for a reply.
        // If the debugger handles the exception, then continue execution. Else
        // if the current process has a subsystem port, then send a message to
        // the subsystem port and wait for a reply. If the subsystem handles the
        // exception, then continue execution. Else terminate the process.
        //


        if (FirstChance == TRUE) {

            //
            // This is the first chance to handle the exception.
            //

            if ((KiDebugRoutine != NULL)  &&
                ((PsGetCurrentProcess()->DebugPort == NULL &&
                  !KdIgnoreUmExceptions) ||
                 (KdIsThisAKdTrap(ExceptionRecord, &ContextFrame, UserMode)))) {
                //
                // Now dispatch the fault to the kernel debugger.
                //

                if ((((KiDebugRoutine) (TrapFrame,
                                        ExceptionFrame,
                                        ExceptionRecord,
                                        &ContextFrame,
                                        PreviousMode,
                                        FALSE)) != FALSE)) {

                    goto Handled1;
                }
            }

            if (DbgkForwardException(ExceptionRecord, TRUE, FALSE)) {
                goto Handled2;
            }

            //
            // Transfer exception information to the user stack, transition
            // to user mode, and attempt to dispatch the exception to a frame
            // based handler.

            ExceptionRecord1.ExceptionCode = 0; // satisfy no_opt compilation

        repeat:
            try {

                //
                // If the SS segment is not 32 bit flat, there is no point
                // to dispatch exception to frame based exception handler.
                //

                if (TrapFrame->HardwareSegSs != (KGDT_R3_DATA | RPL_MASK) ||
                    TrapFrame->EFlags & EFLAGS_V86_MASK ) {
                    ExceptionRecord2.ExceptionCode = STATUS_ACCESS_VIOLATION;
                    ExceptionRecord2.ExceptionFlags = 0;
                    ExceptionRecord2.NumberParameters = 0;
                    ExRaiseException(&ExceptionRecord2);
                }

                //
                // Compute length of context record and new aligned user stack
                // pointer.
                //

                UserStack1 = (ContextFrame.Esp & ~CONTEXT_ROUND) - CONTEXT_ALIGNED_SIZE;

                //
                // Probe user stack area for writability and then transfer the
                // context record to the user stack.
                //

                ProbeForWrite((PCHAR)UserStack1, CONTEXT_ALIGNED_SIZE, CONTEXT_ALIGN);
                RtlCopyMemory((PULONG)UserStack1, &ContextFrame, sizeof(CONTEXT));

                //
                // Compute length of exception record and new aligned stack
                // address.
                //

                Length = (sizeof(EXCEPTION_RECORD) - (EXCEPTION_MAXIMUM_PARAMETERS -
                         ExceptionRecord->NumberParameters) * sizeof(ULONG) +3) &
                         (~3);
                UserStack2 = UserStack1 - Length;

                //
                // Probe user stack area for writeability and then transfer the
                // context record to the user stack area.
                // N.B. The probing length is Length+8 because there are two
                //      arguments need to be pushed to user stack later.
                //

                ProbeForWrite((PCHAR)(UserStack2 - 8), Length + 8, sizeof(ULONG));
                RtlCopyMemory((PULONG)UserStack2, ExceptionRecord, Length);

                //
                // Push address of exception record, context record to the
                // user stack.  They are the two parameters required by
                // _KiUserExceptionDispatch.
                //

                *(PULONG)(UserStack2 - sizeof(ULONG)) = UserStack1;
                *(PULONG)(UserStack2 - 2*sizeof(ULONG)) = UserStack2;

                //
                // Set new stack pointer to the trap frame.
                //

                KiSegSsToTrapFrame(TrapFrame, KGDT_R3_DATA);
                KiEspToTrapFrame(TrapFrame, (UserStack2 - sizeof(ULONG)*2));

                //
                // Force correct R3 selectors into TrapFrame.
                //

                TrapFrame->SegCs = SANITIZE_SEG(KGDT_R3_CODE, PreviousMode);
                TrapFrame->SegDs = SANITIZE_SEG(KGDT_R3_DATA, PreviousMode);
                TrapFrame->SegEs = SANITIZE_SEG(KGDT_R3_DATA, PreviousMode);
                TrapFrame->SegFs = SANITIZE_SEG(KGDT_R3_TEB, PreviousMode);
                TrapFrame->SegGs = 0;

                //
                // Set the address of the exception routine that will call the
                // exception dispatcher and then return to the trap handler.
                // The trap handler will restore the exception and trap frame
                // context and continue execution in the routine that will
                // call the exception dispatcher.
                //

                TrapFrame->Eip = (ULONG)KeUserExceptionDispatcher;
                return;

            } except (KiCopyInformation(&ExceptionRecord1,
                        (GetExceptionInformation())->ExceptionRecord)) {

                //
                // If the exception is a stack overflow, then attempt
                // to raise the stack overflow exception. Otherwise,
                // the user's stack is not accessible, or is misaligned,
                // and second chance processing is performed.
                //

                if (ExceptionRecord1.ExceptionCode == STATUS_STACK_OVERFLOW) {
                    ExceptionRecord1.ExceptionAddress = ExceptionRecord->ExceptionAddress;
                    RtlCopyMemory((PVOID)ExceptionRecord,
                                  &ExceptionRecord1, sizeof(EXCEPTION_RECORD));
                    goto repeat;
                }
            }
        }

        //
        // This is the second chance to handle the exception.
        //

        if (DbgkForwardException(ExceptionRecord, TRUE, TRUE)) {
            goto Handled2;
        } else if (DbgkForwardException(ExceptionRecord, FALSE, TRUE)) {
            goto Handled2;
        } else {
            ZwTerminateProcess(NtCurrentProcess(), ExceptionRecord->ExceptionCode);
            KeBugCheckEx(
                KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                ExceptionRecord->ExceptionCode,
                (ULONG)ExceptionRecord->ExceptionAddress,
                (ULONG)TrapFrame,
                0);
        }
    }

    //
    // Move machine state from context frame to trap and exception frames and
    // then return to continue execution with the restored state.
    //

Handled1:

    KeContextToKframes(TrapFrame, ExceptionFrame, &ContextFrame,
                       ContextFrame.ContextFlags, PreviousMode);

    //
    // Exception was handled by the debugger or the associated subsystem
    // and state was modified, if necessary, using the get state and set
    // state capabilities. Therefore the context frame does not need to
    // be transferred to the trap and exception frames.
    //

Handled2:
    return;
}

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    )

/*++

Routine Description:

    This function is called from an exception filter to copy the exception
    information from one exception record to another when an exception occurs.

Arguments:

    ExceptionRecord1 - Supplies a pointer to the destination exception record.

    ExceptionRecord2 - Supplies a pointer to the source exception record.

Return Value:

    A value of EXCEPTION_EXECUTE_HANDLER is returned as the function value.

--*/

{

    //
    // Copy one exception record to another and return value that causes
    // an exception handler to be executed.
    //

    RtlCopyMemory((PVOID)ExceptionRecord1,
                  (PVOID)ExceptionRecord2,
                  sizeof(EXCEPTION_RECORD));

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
KeRaiseUserException(
    IN NTSTATUS ExceptionCode
    )

/*++

Routine Description:

    This function causes an exception to be raised in the calling thread's user-mode
    context. It does this by editing the trap frame the kernel was entered with to
    point to trampoline code that raises the requested exception.

Arguments:

    ExceptionCode - Supplies the status value to be used as the exception
        code for the exception that is to be raised.

Return Value:

    The status value that should be returned by the caller.

--*/

{
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;
    PTEB Teb;
    ULONG PreviousEsp;

    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    if (TrapFrame == NULL || ((TrapFrame->SegCs & MODE_MASK) != UserMode)) {
        return ExceptionCode;
    }

    Teb = (PTEB)Thread->Teb;

    //
    // In order to create the correct call stack, we push the old return
    // address onto the stack. The status code to be raised is passed
    // in the TEB.
    //

    try {
        Teb->ExceptionCode = ExceptionCode;

        PreviousEsp = KiEspFromTrapFrame (TrapFrame) - sizeof (ULONG);

        ProbeForWriteSmallStructure ((PLONG)PreviousEsp, sizeof (LONG), sizeof (UCHAR));
        *(PLONG)PreviousEsp = TrapFrame->Eip;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return(ExceptionCode);
    }

    KiEspToTrapFrame (TrapFrame, PreviousEsp);

    TrapFrame->Eip = (ULONG)KeRaiseUserExceptionDispatcher;

    return ExceptionCode;
}


BOOLEAN
KiCheckForAtlThunk (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT Context
    )

/*++

Routine Description:

    This routine will determine whether an access violation was raised due to
    an attempt to execute an ATL thunk in a no-execute, non-stack area.

    If so, the thunk will be emulated and execution resumed.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    Context - Supplies a pointer to a context frame.

Return Value:

    TRUE - Context was updated to reflect the emulated ATL thunk, resume
           execution.

    FALSE - Not an ATL thunk, continue raising the exception.

--*/

{
    ULONG faultIndicator;

    //
    // Interested only in an instruction fetch fault.
    // 

    faultIndicator = ExceptionRecord->ExceptionInformation[0];
    if ((faultIndicator & 0x8) == 0) {
        return FALSE;
    }

    //
    // Where the fault address is the instruction
    // 

    if (ExceptionRecord->ExceptionInformation[1] != Context->Eip) {
        return FALSE;
    }

    if (KiEmulateAtlThunk(&Context->Eip,
                          &Context->Esp,
                          &Context->Eax,
                          &Context->Ecx,
                          &Context->Edx)) {

        return TRUE;
    } else {
        return FALSE;
    }
}

