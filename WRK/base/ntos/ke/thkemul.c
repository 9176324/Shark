/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    thkemul.c

Abstract:

    This module implements machine independent functions involved with
    emulating known code sequences in NX regions.

--*/

#include "ki.h"

LOGICAL
KiEmulateAtlThunk (
    IN OUT ULONG *InstructionPointer,
    IN OUT ULONG *StackPointer,
    IN OUT ULONG *Eax,
    IN OUT ULONG *Ecx,
    IN OUT ULONG *Edx
    )

/*++

Routine Description:

    This routine is called to determine whether the 32-bit X86 IStream
    contains a recognized ATL thunk sequence and if so, performs
    the emulation.

Arguments:

    InstructionPointer - Supplies a pointer to the value of the 32-bit
        instruction pointer at the time of the fault.

    StackPointer - Supplies a pointer to the value of the 32-bit stack
        pointer at the time of the fault.

    Ecx - Supplies a pointer to the value of ecx at the time of the fault.

    Edx - Supplies a pointer to the value of edx at the time of the fault.

Return Value:

    Returns TRUE if an ATL thunk was recognized and emulated, FALSE if not.


    It is up to the caller to first ensure:

    - The fault occured while executing 32-bit code
    - The fault occured as a result of attempting to execute NX code

--*/

{
    LONG branchTarget;
    LONG imm32;
    PVOID rip;
    PUCHAR rsp;
    BOOLEAN safeThunkCall;
    BOOLEAN *safeThunkCallPtr;
    LOGICAL validThunk;

    //
    // Three types of ATL thunks of interest
    //

    #pragma pack(1)

    struct {
        LONG Mov;           // 0x042444C7   mov [esp+4], imm32
        LONG MovImmediate;
        UCHAR Jmp;          // 0xe9         jmp imm32
        LONG JmpImmediate;
    } *thunk1;

    struct {
        UCHAR Mov;          // 0xb9         mov ecx, imm32
        LONG EcxImmediate;
        UCHAR Jmp;          // 0xe9         jmp imm32
        LONG JmpImmediate;
    } *thunk2;

    struct {
        UCHAR MovEdx;       // 0xba         mov edx, imm32
        LONG EdxImmediate;
        UCHAR MovEcx;       // 0xb9         mov ecx, imm32
        LONG EcxImmediate;
        USHORT JmpEcx;      // 0xe1ff       jmp ecx
    } *thunk3;

    struct {
        UCHAR MovEcx;       // 0xb9         mov ecx, imm32
        LONG EcxImmediate;
        UCHAR MovEax;       // 0xb8         mov eax, imm32
        LONG EaxImmediate;
        USHORT JmpEax;      // 0xe0ff       jmp eax
    } *thunk4;

    struct {
        UCHAR PopEcx;       // 0x59
        UCHAR PopEax;       // 0x58
        UCHAR PushEcx;      // 0x51
        UCHAR Jmp[3];       // 0xFF 0x60 0x04   jmp [eax+4]
    } *thunk7;

    #pragma pack()

    rip = UlongToPtr(*InstructionPointer);
    rsp = UlongToPtr(*StackPointer);

    thunk1 = rip;
    thunk2 = rip;
    thunk3 = rip;
    thunk4 = rip;
    thunk7 = rip;

    validThunk = FALSE;

    //
    // If thunk emulation is disabled, then do not attempt to emulate any
    // thunks.
    //

    if (KiQueryNxThunkEmulationState() != 0) {
        return FALSE;
    }

    //
    // Carefully examine the instruction stream.  If it matches a known
    // ATL thunk template, then emulate it.
    //

    try {
        ProbeAndReadUchar((PUCHAR)rip);

#if defined(_WIN64)
        safeThunkCallPtr = &NtCurrentTeb32()->SafeThunkCall;
#else
        safeThunkCallPtr = &NtCurrentTeb()->SafeThunkCall;
#endif

        safeThunkCall = *safeThunkCallPtr;
        if (safeThunkCall != FALSE) {
            *safeThunkCallPtr = FALSE;
        }

        if ((thunk1->Mov == 0x042444C7) && (thunk1->Jmp == 0xe9)) {

            //
            // Type 1 thunk.
            //

            //
            // emulate: jmp imm32
            //

            imm32 = thunk1->JmpImmediate +
                    PtrToUlong(rip) +
                    sizeof(*thunk1);

            //
            // Determine if it is safe to emulate this code stream.
            //

            if ((MmCheckForSafeExecution (rip,
                                          rsp,
                                          UlongToPtr (imm32),
                                          TRUE) == FALSE) ||
                (safeThunkCall == FALSE)) {
                goto Done;
            }
            
            //
            // emulate: mov [esp+4], imm32
            // 

            ProbeAndWriteUlong((PULONG)(rsp+4), thunk1->MovImmediate);
            *InstructionPointer = imm32;
            validThunk = TRUE;

        } else if ((thunk2->Mov == 0xb9) && (thunk2->Jmp == 0xe9)) {

            //
            // Type 2 thunk.
            //

            //
            // emulate: jmp imm32
            //

            imm32 = thunk2->JmpImmediate +
                    PtrToUlong(rip) +
                    sizeof(*thunk2);

            //
            // Determine if it is safe to emulate this code stream.
            //

            if ((MmCheckForSafeExecution (rip,
                                          rsp,
                                          UlongToPtr (imm32),
                                          TRUE) == FALSE) ||
                (safeThunkCall == FALSE)) {
                goto Done;
            }

            //
            // emulate: mov ecx, imm32
            //

            *Ecx = thunk2->EcxImmediate;


            *InstructionPointer = imm32;
            validThunk = TRUE;

        } else if ((thunk3->MovEdx == 0xba) &&
                   (thunk3->MovEcx == 0xb9) &&
                   (thunk3->JmpEcx == 0xe1ff)) {

            //
            // Type 3 thunk.
            //

            //
            // emulate: mov ecx, imm32
            //

            imm32 = thunk3->EcxImmediate;

            //
            // Determine if it is safe to emulate this code stream.
            //

            if (MmCheckForSafeExecution (rip,
                                         rsp,
                                         UlongToPtr (imm32),
                                         FALSE) == FALSE) {
                goto Done;
            }

            //
            // emulate: mov edx, imm32
            //

            *Edx = thunk3->EdxImmediate;

            *Ecx = imm32;

            //
            // emulate: jmp ecx
            //

            *InstructionPointer = imm32;
            validThunk = TRUE;

        } else if ((thunk4->MovEcx == 0xb9) &&
                   (thunk4->MovEax == 0xb8) &&
                   (thunk4->JmpEax == 0xe0ff)) {

            //
            // Type 4 thunk
            //
            
            //
            // emulate: mov eax, imm32
            //

            imm32 = thunk4->EaxImmediate;

            //
            // Determine if it is safe to emulate this code stream.
            //

            if ((MmCheckForSafeExecution (rip,
                                          rsp,
                                          UlongToPtr (imm32),
                                          TRUE) == FALSE) ||
                (safeThunkCall == FALSE)) {
                goto Done;
            }

            //
            // emulate: mov ecx, imm32
            //

            *Ecx = thunk4->EcxImmediate;

            *Eax = imm32;

            //
            // emulate: jmp eax
            //

            *InstructionPointer = imm32;
            validThunk = TRUE;

        } else if (thunk7->PopEcx == 0x59 &&
                   thunk7->PopEax == 0x58 &&
                   thunk7->PushEcx == 0x51 &&
                   thunk7->Jmp[0] == 0xFF &&
                   thunk7->Jmp[1] == 0x60 &&
                   thunk7->Jmp[2] == 0x04) {

            //
            // Type 7 thunk
            //
            // This is used by VB6
            //

            //
            // First determine and validate the branch target
            //

            imm32 = ProbeAndReadUlong((PULONG)(rsp+4));
            branchTarget = ProbeAndReadUlong((PULONG)(UlongToPtr(imm32+4)));

            //
            // Determine if it is safe to emulate this code stream.
            //

            if (MmCheckForSafeExecution(rip,
                                        rsp,
                                        UlongToPtr(branchTarget),
                                        FALSE) == FALSE) {
                goto Done;
            }

            //
            // Emulate: pop ecx
            //

            *Ecx = *(PULONG)rsp;
            rsp += 4;

            //
            // Emulate: pop eax
            //          push ecx
            //

            *Eax = *(PULONG)rsp;
            *(PULONG)rsp = *Ecx;

            //
            // Emulate: jmp [eax+4]
            //

            *InstructionPointer = branchTarget;
            *StackPointer = PtrToUlong(rsp);
            validThunk = TRUE;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

Done:
    return validThunk;
}

