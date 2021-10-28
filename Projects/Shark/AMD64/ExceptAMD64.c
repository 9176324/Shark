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
#include <devicedefs.h>

#include "Except.h"

#include "Scan.h"
#include "Guard.h"
#include "Space.h"

//
// ****** temp - define elsewhere ******
//

#define SIZE64_PREFIX 0x48
#define ADD_IMM8_OP 0x83
#define ADD_IMM32_OP 0x81
#define JMP_IMM8_OP 0xeb
#define JMP_IMM32_OP 0xe9
#define LEA_OP 0x8d
#define POP_OP 0x58
#define RET_OP 0xc3

//
// Define unwind operation codes.
//

typedef enum _AMD64_UNWIND_OP_CODES {
    AMD64_UWOP_PUSH_NONVOL = 0,
    AMD64_UWOP_ALLOC_LARGE,
    AMD64_UWOP_ALLOC_SMALL,
    AMD64_UWOP_SET_FPREG,
    AMD64_UWOP_SAVE_NONVOL,
    AMD64_UWOP_SAVE_NONVOL_FAR,
    AMD64_UWOP_SAVE_XMM,
    AMD64_UWOP_SAVE_XMM_FAR,
    AMD64_UWOP_SAVE_XMM128,
    AMD64_UWOP_SAVE_XMM128_FAR,
    AMD64_UWOP_PUSH_MACHFRAME
} AMD64_UNWIND_OP_CODES, *PAMD64_UNWIND_OP_CODES;

//
// Define lookup table for providing the number of slots used by each unwind
// code.
// 

u8 UnwindOpSlotTable[] = {
    1, // UWOP_PUSH_NONVOL
    2, // UWOP_ALLOC_LARGE (or 3, special cased in lookup code)
    1, // UWOP_ALLOC_SMALL
    1, // UWOP_SET_FPREG
    2, // UWOP_SAVE_NONVOL
    3, // UWOP_SAVE_NONVOL_FAR
    2, // UWOP_SAVE_XMM
    3, // UWOP_SAVE_XMM_FAR
    2, // UWOP_SAVE_XMM128
    3, // UWOP_SAVE_XMM128_FAR
    1 // UWOP_PUSH_MACHFRAME
};

void
NTAPI
InitializeExcept(
    __inout PRTB Block
)
{
    NOTHING;
}

void
NTAPI
InsertInvertedFunctionTable(
    __in ptr ImageBase,
    __in u32 SizeOfImage
)
{
    u32 CurrentSize = 0;
    u32 SizeOfTable = 0;
    u32 Index = 0;
    ptr FunctionTable = NULL;
    PFUNCTION_TABLE_ENTRY64 FunctionTableEntry = NULL;

    if (IPI_LEVEL != KeGetCurrentIrql()) {
        RtBlock.KeEnterCriticalRegion();

        ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);
    }

    FunctionTableEntry = (PFUNCTION_TABLE_ENTRY64)
        &RtBlock.PsInvertedFunctionTable->TableEntry;

    CurrentSize = RtBlock.PsInvertedFunctionTable->CurrentSize;

    if (RtBlock.DebuggerDataBlock.KernBase ==
        FunctionTableEntry[0].ImageBase) {
        Index = 1;
    }

    if (CurrentSize != RtBlock.PsInvertedFunctionTable->MaximumSize) {
        if (0 != CurrentSize) {
            for (;
                Index < CurrentSize;
                Index++) {
                if ((u)ImageBase < FunctionTableEntry[Index].ImageBase) {
                    RtlMoveMemory(
                        &FunctionTableEntry[Index + 1],
                        &FunctionTableEntry[Index],
                        (CurrentSize - Index) * sizeof(FUNCTION_TABLE_ENTRY64));

                    break;
                }
            }
        }

        CaptureImageExceptionValues(
            ImageBase,
            &FunctionTable,
            &SizeOfTable);

        FunctionTableEntry[Index].ImageBase = (u)ImageBase;
        FunctionTableEntry[Index].SizeOfImage = SizeOfImage;
        FunctionTableEntry[Index].FunctionTable = (u)FunctionTable;
        FunctionTableEntry[Index].SizeOfTable = SizeOfTable;

        RtBlock.PsInvertedFunctionTable->CurrentSize += 1;
    }
    else {
        RtBlock.PsInvertedFunctionTable->Overflow = TRUE;
    }

    if (IPI_LEVEL != KeGetCurrentIrql()) {
        ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);

        RtBlock.KeLeaveCriticalRegion();
    }
}

void
NTAPI
RemoveInvertedFunctionTable(
    __in ptr ImageBase
)
{
    u32 CurrentSize = 0;
    u32 Index = 0;
    PFUNCTION_TABLE_ENTRY64 FunctionTableEntry = NULL;

    if (IPI_LEVEL != KeGetCurrentIrql()) {
        RtBlock.KeEnterCriticalRegion();

        ExAcquireResourceExclusiveLite(RtBlock.PsLoadedModuleResource, TRUE);
    }

    FunctionTableEntry = (PFUNCTION_TABLE_ENTRY64)
        &RtBlock.PsInvertedFunctionTable->TableEntry;

    CurrentSize = RtBlock.PsInvertedFunctionTable->CurrentSize;

    for (Index = 0;
        Index < CurrentSize;
        Index += 1) {
        if ((u)ImageBase == FunctionTableEntry[Index].ImageBase) {
            RtlMoveMemory(
                &FunctionTableEntry[Index],
                &FunctionTableEntry[Index + 1],
                (CurrentSize - Index - 1) * sizeof(FUNCTION_TABLE_ENTRY64));

            RtBlock.PsInvertedFunctionTable->CurrentSize -= 1;

            break;
        }
    }

    if (IPI_LEVEL != KeGetCurrentIrql()) {
        ExReleaseResourceLite(RtBlock.PsLoadedModuleResource);

        RtBlock.KeLeaveCriticalRegion();
    }
}

PRUNTIME_FUNCTION
NTAPI
UnwindPrologue(
    __in u64 ImageBase,
    __in u64 ControlPc,
    __in u64 FrameBase,
    __in PRUNTIME_FUNCTION FunctionEntry,
    __inout PCONTEXT ContextRecord,
    __inout_opt PKNONVOLATILE_CONTEXT_POINTERS ContextPointers
)
/*++

Routine Description:

    This function processes unwind codes and reverses the state change
    effects of a prologue. If the specified unwind information contains
    chained unwind information, then that prologue is unwound recursively.
    As the prologue is unwound state changes are recorded in the specified
    context structure and optionally in the specified context pointers
    structures.

Arguments:

    ImageBase - Supplies the base address of the image that contains the
        function being unwound.

    ControlPc - Supplies the address where control left the specified
        function.

    FrameBase - Supplies the base of the stack frame subject function stack
         frame.

    FunctionEntry - Supplies the address of the function table entry for the
        specified function.

    ContextRecord - Supplies the address of a context record.

    ContextPointers - Supplies an optional pointer to a context pointers
        record.

Return Value:

--*/
{
    PM128A FloatingAddress;
    PM128A FloatingRegister;
    u32 FrameOffset;
    u32 Index;
    u64ptr IntegerAddress;
    u64ptr IntegerRegister;
    b MachineFrame;
    u32 OpInfo;
    u32 PrologOffset;
    u64ptr RegisterAddress;
    u64ptr ReturnAddress;
    u64ptr StackAddress;
    PUNWIND_CODE UnwindCode;
    PUNWIND_INFO UnwindInfo;
    u32 UnwindOp;

    //
    // Process the unwind codes.
    //

    FloatingRegister = &ContextRecord->Xmm0;
    IntegerRegister = &ContextRecord->Rax;
    Index = 0;
    MachineFrame = FALSE;
    PrologOffset = (u32)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);

    while (Index < UnwindInfo->CountOfCodes) {
        //
        // If the prologue offset is greater than the next unwind code offset,
        // then simulate the effect of the unwind code.
        //

        UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
        OpInfo = UnwindInfo->UnwindCode[Index].OpInfo;

        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            switch (UnwindOp) {
                //
                // Push nonvolatile integer register.
                //
                // The operation information is the register number of the
                // register than was pushed.
                //

            case AMD64_UWOP_PUSH_NONVOL:
                IntegerAddress = (u64ptr)(ContextRecord->Rsp);
                IntegerRegister[OpInfo] = *IntegerAddress;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;

                break;

                //
                // Allocate a large sized area on the stack.
                //
                // The operation information determines if the size is
                // 16- or 32-bits.
                //

            case AMD64_UWOP_ALLOC_LARGE:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset;

                if (OpInfo != 0) {
                    Index += 1;
                    FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                }
                else {
                    FrameOffset *= 8;
                }

                ContextRecord->Rsp += FrameOffset;

                break;

                //
                // Allocate a small sized area on the stack.
                //
                // The operation information is the size of the unscaled
                // allocation size (8 is the scale factor) minus 8.
                //

            case AMD64_UWOP_ALLOC_SMALL:
                ContextRecord->Rsp += (OpInfo * 8) + 8;

                break;

                //
                // Establish the the frame pointer register.
                //
                // The operation information is not used.
                //

            case AMD64_UWOP_SET_FPREG:
                ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
                ContextRecord->Rsp -= UnwindInfo->FrameOffset * 16;

                break;

                //
                // Save nonvolatile integer register on the stack using a
                // 16-bit displacment.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_NONVOL:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                IntegerAddress = (u64ptr)(FrameBase + FrameOffset);
                IntegerRegister[OpInfo] = *IntegerAddress;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                break;

                //
                // Save nonvolatile integer register on the stack using a
                // 32-bit displacment.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_NONVOL_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                IntegerAddress = (u64ptr)(FrameBase + FrameOffset);
                IntegerRegister[OpInfo] = *IntegerAddress;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
                }

                break;

                //
                // Save a nonvolatile XMM(64) register on the stack using a
                // 16-bit displacement.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_XMM:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                FloatingAddress = (PM128A)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = 0;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                //
                // Save a nonvolatile XMM(64) register on the stack using a
                // 32-bit displacement.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_XMM_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = (PM128A)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = 0;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                //
                // Save a nonvolatile XMM(128) register on the stack using a
                // 16-bit displacement.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_XMM128:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 16;
                FloatingAddress = (PM128A)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = FloatingAddress->High;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                //
                // Save a nonvolatile XMM(128) register on the stack using a
                // 32-bit displacement.
                //
                // The operation information is the register number.
                //

            case AMD64_UWOP_SAVE_XMM128_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = (PM128A)(FrameBase + FrameOffset);
                FloatingRegister[OpInfo].Low = FloatingAddress->Low;
                FloatingRegister[OpInfo].High = FloatingAddress->High;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
                }

                break;

                //
                // Push a machine frame on the stack.
                //
                // The operation information determines whether the machine
                // frame contains an error code or not.
                //

            case AMD64_UWOP_PUSH_MACHFRAME:
                MachineFrame = TRUE;
                ReturnAddress = (u64ptr)(ContextRecord->Rsp);
                StackAddress = (u64ptr)(ContextRecord->Rsp + (3 * 8));

                if (OpInfo != 0) {
                    ReturnAddress += 1;
                    StackAddress += 1;
                }

                ContextRecord->Rip = *ReturnAddress;
                ContextRecord->Rsp = *StackAddress;

                break;

                //
                // Unused codes.
                //

            default:
                break;
            }

            Index += 1;

        }
        else {
            //
            // Skip this unwind operation by advancing the slot index by the
            // number of slots consumed by this operation.
            //

            Index += UnwindOpSlotTable[UnwindOp];

            //
            // Special case any unwind operations that can consume a variable
            // number of slots.
            // 

            switch (UnwindOp) {
                //
                // A non-zero operation information indicates that an
                // additional slot is consumed.
                //

            case AMD64_UWOP_ALLOC_LARGE:
                if (OpInfo != 0) {
                    Index += 1;
                }

                break;

                //
                // No other special cases.
                //

            default:
                break;
            }
        }
    }

    //
    // If chained unwind information is specified, then recursively unwind
    // the chained information. Otherwise, determine the return address if
    // a machine frame was not encountered during the scan of the unwind
    // codes.
    //

    if ((UnwindInfo->Flags & UNW_FLAG_CHAININFO) != 0) {
        Index = UnwindInfo->CountOfCodes;

        if ((Index & 1) != 0) {
            Index += 1;
        }

        FunctionEntry = (PRUNTIME_FUNCTION)(*(u32ptr *)(&UnwindInfo->UnwindCode[Index]) + ImageBase);

        return UnwindPrologue(ImageBase,
            ControlPc,
            FrameBase,
            FunctionEntry,
            ContextRecord,
            ContextPointers);

    }
    else {
        if (MachineFrame == FALSE) {
            ContextRecord->Rip = *(u64ptr)(ContextRecord->Rsp);
            ContextRecord->Rsp += 8;
        }

        return FunctionEntry;
    }
}

PEXCEPTION_ROUTINE
NTAPI
VirtualUnwind(
    __in u32 HandlerType,
    __in u64 ImageBase,
    __in u64 ControlPc,
    __in PRUNTIME_FUNCTION FunctionEntry,
    __inout PCONTEXT ContextRecord,
    __out ptr * HandlerData,
    __out u64ptr EstablisherFrame,
    __inout_opt PKNONVOLATILE_CONTEXT_POINTERS ContextPointers
)
/*++

Routine Description:

    This function virtually unwinds the specified function by executing its
    prologue code backward or its epilogue code forward.

    If a context pointers record is specified, then the address where each
    nonvolatile registers is restored from is recorded in the appropriate
    element of the context pointers record.

Arguments:

    HandlerType - Supplies the handler type expected for the virtual unwind.
        This may be either an exception or an unwind handler.

    ImageBase - Supplies the base address of the image that contains the
        function being unwound.

    ControlPc - Supplies the address where control left the specified
        function.

    FunctionEntry - Supplies the address of the function table entry for the
        specified function.

    ContextRecord - Supplies the address of a context record.

    HandlerData - Supplies a pointer to a variable that receives a pointer
        the the language handler data.

    EstablisherFrame - Supplies a pointer to a variable that receives the
        the establisher frame pointer value.

    ContextPointers - Supplies an optional pointer to a context pointers
        record.

Return Value:

    If control did not leave the specified function in either the prologue
    or an epilogue and a handler of the proper type is associated with the
    function, then the address of the language specific exception handler
    is returned. Otherwise, NULL is returned.

--*/
{
    u64 BranchTarget;
    s32 Displacement;
    u32 FrameRegister;
    u32 Index;
    u32 InEpilogue;
    u64ptr IntegerAddress;
    u64ptr IntegerRegister;
    cptr NextByte;
    u32 PrologOffset;
    u32 RegisterNumber;
    PUNWIND_INFO UnwindInfo;

    //
    // If the specified function does not use a frame pointer, then the
    // establisher frame is the contents of the stack pointer. This may
    // not actually be the real establisher frame if control left the
    // function from within the prologue. In this case the establisher
    // frame may be not required since control has not actually entered
    // the function and prologue entries cannot refer to the establisher
    // frame before it has been established, i.e., if it has not been
    // established, then no save unwind codes should be encountered during
    // the unwind operation.
    //
    // If the specified function uses a frame pointer and control left the
    // function outside of the prologue or the unwind information contains
    // a chained information structure, then the establisher frame is the
    // contents of the frame pointer.
    //
    // If the specified function uses a frame pointer and control left the
    // function from within the prologue, then the set frame pointer unwind
    // code must be looked up in the unwind codes to detetermine if the
    // contents of the stack pointer or the contents of the frame pointer
    // should be used for the establisher frame. This may not atually be
    // the real establisher frame. In this case the establisher frame may
    // not be required since control has not actually entered the function
    // and prologue entries cannot refer to the establisher frame before it
    // has been established, i.e., if it has not been established, then no
    // save unwind codes should be encountered during the unwind operation.
    //
    // N.B. The correctness of these assumptions is based on the ordering of
    //      unwind codes.
    //

    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
    PrologOffset = (u32)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));

    if (UnwindInfo->FrameRegister == 0) {
        *EstablisherFrame = ContextRecord->Rsp;
    }
    else if ((PrologOffset >= UnwindInfo->SizeOfProlog) ||
        ((UnwindInfo->Flags &  UNW_FLAG_CHAININFO) != 0)) {
        *EstablisherFrame = (&ContextRecord->Rax)[UnwindInfo->FrameRegister];
        *EstablisherFrame -= UnwindInfo->FrameOffset * 16;
    }
    else {
        Index = 0;

        while (Index < UnwindInfo->CountOfCodes) {
            if (UnwindInfo->UnwindCode[Index].UnwindOp == UWOP_SET_FPREG) {
                break;
            }

            Index += 1;
        }

        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            *EstablisherFrame = (&ContextRecord->Rax)[UnwindInfo->FrameRegister];
            *EstablisherFrame -= UnwindInfo->FrameOffset * 16;
        }
        else {
            *EstablisherFrame = ContextRecord->Rsp;
        }
    }

    //
    // Check for epilogue.
    //
    // If the point at which control left the specified function is in an
    // epilogue, then emulate the execution of the epilogue forward and
    // return no exception handler.
    //

    IntegerRegister = &ContextRecord->Rax;
    NextByte = (cptr)ControlPc;

    //
    // Check for one of:
    //
    //   add rsp, imm8
    //       or
    //   add rsp, imm32
    //       or
    //   lea rsp, -disp8[fp]
    //       or
    //   lea rsp, -disp32[fp]
    //

    if ((NextByte[0] == SIZE64_PREFIX) &&
        (NextByte[1] == ADD_IMM8_OP) &&
        (NextByte[2] == 0xc4)) {
        //
        // add rsp, imm8.
        //

        NextByte += 4;

    }
    else if ((NextByte[0] == SIZE64_PREFIX) &&
        (NextByte[1] == ADD_IMM32_OP) &&
        (NextByte[2] == 0xc4)) {
        //
        // add rsp, imm32.
        //

        NextByte += 7;

    }
    else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
        (NextByte[1] == LEA_OP)) {
        FrameRegister = ((NextByte[0] & 0x7) << 3) | (NextByte[2] & 0x7);

        if ((FrameRegister != 0) &&
            (FrameRegister == UnwindInfo->FrameRegister)) {
            if ((NextByte[2] & 0xf8) == 0x60) {
                //
                // lea rsp, disp8[fp].
                //

                NextByte += 4;

            }
            else if ((NextByte[2] & 0xf8) == 0xa0) {
                //
                // lea rsp, disp32[fp].
                //

                NextByte += 7;
            }
        }
    }

    //
    // Check for any number of:
    //
    //   pop nonvolatile-integer-register[0..15].
    //

    while (TRUE) {
        if ((NextByte[0] & 0xf8) == POP_OP) {
            NextByte += 1;
        }
        else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
            ((NextByte[1] & 0xf8) == POP_OP)) {
            NextByte += 2;
        }
        else {
            break;
        }
    }

    //
    // If the next instruction is a return, then control is currently in
    // an epilogue and execution of the epilogue should be emulated.
    // Otherwise, execution is not in an epilogue and the prologue should
    // be unwound.
    //

    InEpilogue = FALSE;
    if (NextByte[0] == RET_OP) {
        //
        // A return is an unambiguous indication of an epilogue
        //

        InEpilogue = TRUE;
    }
    else if (NextByte[0] == JMP_IMM8_OP || NextByte[0] == JMP_IMM32_OP) {
        //
        // An unconditional branch to a target that is equal to the start of
        // or outside of this routine is logically a call to another function.
        // 

        BranchTarget = (u64)NextByte - ImageBase;

        if (NextByte[0] == JMP_IMM8_OP) {
            BranchTarget += 2 + (CHAR)NextByte[1];
        }
        else {
            BranchTarget += 5 + *((s32 UNALIGNED *)&NextByte[1]);
        }

        //
        // Now determine whether the branch target refers to code within this
        // function. If not, then it is an epilogue indicator.
        //

        if (BranchTarget <= FunctionEntry->BeginAddress ||
            BranchTarget > FunctionEntry->EndAddress) {
            InEpilogue = TRUE;
        }
    }

    if (InEpilogue != FALSE) {
        NextByte = (cptr)ControlPc;

        //
        // Emulate one of (if any):
        //
        //   add rsp, imm8
        //       or
        //   add rsp, imm32
        //       or                
        //   lea rsp, disp8[frame-register]
        //       or
        //   lea rsp, disp32[frame-register]
        //

        if ((NextByte[0] & 0xf8) == SIZE64_PREFIX) {
            if (NextByte[1] == ADD_IMM8_OP) {
                //
                // add rsp, imm8.
                //

                ContextRecord->Rsp += (CHAR)NextByte[3];
                NextByte += 4;
            }
            else if (NextByte[1] == ADD_IMM32_OP) {
                //
                // add rsp, imm32.
                //

                Displacement = NextByte[3] | (NextByte[4] << 8);
                Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
                ContextRecord->Rsp += Displacement;
                NextByte += 7;
            }
            else if (NextByte[1] == LEA_OP) {
                if ((NextByte[2] & 0xf8) == 0x60) {
                    //
                    // lea rsp, disp8[frame-register].
                    //

                    ContextRecord->Rsp = IntegerRegister[FrameRegister];
                    ContextRecord->Rsp += (CHAR)NextByte[3];
                    NextByte += 4;
                }
                else if ((NextByte[2] & 0xf8) == 0xa0) {
                    //
                    // lea rsp, disp32[frame-register].
                    //

                    Displacement = NextByte[3] | (NextByte[4] << 8);
                    Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
                    ContextRecord->Rsp = IntegerRegister[FrameRegister];
                    ContextRecord->Rsp += Displacement;
                    NextByte += 7;
                }
            }
        }

        //
        // Emulate any number of (if any):
        //
        //   pop nonvolatile-integer-register.
        //

        while (TRUE) {
            if ((NextByte[0] & 0xf8) == POP_OP) {
                //
                // pop nonvolatile-integer-register[0..7]
                //

                RegisterNumber = NextByte[0] & 0x7;
                IntegerAddress = (u64ptr)ContextRecord->Rsp;
                IntegerRegister[RegisterNumber] = *IntegerAddress;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[RegisterNumber] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;
                NextByte += 1;
            }
            else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
                ((NextByte[1] & 0xf8) == POP_OP)) {
                //
                // pop nonvolatile-integer-regiser[8..15]
                //

                RegisterNumber = ((NextByte[0] & 1) << 3) | (NextByte[1] & 0x7);
                IntegerAddress = (u64ptr)ContextRecord->Rsp;
                IntegerRegister[RegisterNumber] = *IntegerAddress;

                if (ARGUMENT_PRESENT(ContextPointers)) {
                    ContextPointers->IntegerContext[RegisterNumber] = IntegerAddress;
                }

                ContextRecord->Rsp += 8;
                NextByte += 2;

            }
            else {
                break;
            }
        }

        //
        // Emulate return and return null exception handler.
        //
        // Note: this instruction might in fact be a jmp, however
        //       we want to emulate a return regardless.
        //

        ContextRecord->Rip = *(u64ptr)(ContextRecord->Rsp);
        ContextRecord->Rsp += 8;
        return NULL;
    }

    //
    // Control left the specified function outside an epilogue. Unwind the
    // subject function and any chained unwind information.
    //

    FunctionEntry = UnwindPrologue(ImageBase,
        ControlPc,
        *EstablisherFrame,
        FunctionEntry,
        ContextRecord,
        ContextPointers);

    //
    // If control left the specified function outside of the prologue and
    // the function has a handler that matches the specified type, then
    // return the address of the language specific exception handler.
    // Otherwise, return NULL.
    //

    UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
    PrologOffset = (u32)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));

    if ((PrologOffset >= UnwindInfo->SizeOfProlog) &&
        ((UnwindInfo->Flags & HandlerType) != 0)) {
        Index = UnwindInfo->CountOfCodes;

        if ((Index & 1) != 0) {
            Index += 1;
        }

        *HandlerData = &UnwindInfo->UnwindCode[Index + 2];

        return (PEXCEPTION_ROUTINE)(*((u32ptr)&UnwindInfo->UnwindCode[Index]) + ImageBase);
    }
    else {
        return NULL;
    }
}
