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

#include "Guard.h"

#include "Scan.h"
#include "Reload.h"

#pragma section( ".guard", read, write )

#pragma pack (push, 1)
__declspec(allocate(".guard")) struct {
    RTL_BITMAP BitMap;
    u8 Bits[0x100];
    u8 Bytes[4 * PAGE_SIZE - 0x100 - sizeof(RTL_BITMAP)];
}Trampoline = { 0x100, __utop(&Trampoline.Bytes) };
#pragma pack (pop)

C_ASSERT(sizeof(Trampoline) == 4 * PAGE_SIZE);

void
NTAPI
InitializeGuardTrampoline(
    void
)
{
    PMMPTE PointerPte = NULL;
    PFN_NUMBER NumberOfPages = 0;

    PointerPte = GetPteAddress(&Trampoline);
    NumberOfPages = BYTES_TO_PAGES(sizeof(Trampoline));

    while (NumberOfPages--) {
        PointerPte[NumberOfPages].u.Hard.NoExecute = 0;
    }

    FlushMultipleTb(&Trampoline, sizeof(Trampoline), TRUE);
}

ptr
NTAPI
GuardAllocateTrampoline(
    __in u8 NumberOfBytes
)
{
    ptr Result = NULL;
    u32 HintIndex = 0;

    NumberOfBytes = (NumberOfBytes + 7) & ~7;

    HintIndex = RtlFindClearBitsAndSet(
        &Trampoline.BitMap,
        NumberOfBytes / 8,
        HintIndex);

    if (MAXULONG != HintIndex) {
        Result = Trampoline.Bytes + HintIndex * 8;
    }

    return Result;
}

void
NTAPI
GuardFreeTrampoline(
    __in ptr BaseAddress,
    __in u8 NumberOfBytes
)
{
    u32 HintIndex = 0;

    NumberOfBytes = (NumberOfBytes + 7) & ~7;

    HintIndex =
        ((u8ptr)BaseAddress - Trampoline.Bytes) / 8;

    RtlClearBits(
        &Trampoline.BitMap,
        HintIndex,
        NumberOfBytes / 8);
}

void
NTAPI
MapLockedCopyInstruction(
    __in ptr Destination,
    __in ptr Source,
    __in u32 Length
)
{
    s8 Instruction[4] = { 0 };
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    ptr VirtualAddress = NULL;

    if (Length > sizeof(u32)) {
        RtlCopyMemory(Instruction, Source, sizeof(u32));
    }
    else {
        RtlCopyMemory(Instruction, Source, Length);

        RtlCopyMemory(
            Instruction + Length,
            (u8ptr)Destination + Length,
            sizeof(u32) - Length);
    }

    PhysicalAddress = MmGetPhysicalAddress(Destination);

    VirtualAddress = MmMapIoSpace(
        PhysicalAddress,
        Length,
        MmNonCached);

    if (NULL != VirtualAddress) {
        if (Length > sizeof(u32)) {
            InterlockedExchange(VirtualAddress, JUMP_SELF);

            RtlCopyMemory(
                (u8ptr)VirtualAddress + sizeof(u32),
                (u8ptr)Source + sizeof(u32),
                Length - sizeof(u32));
        }

        InterlockedExchange(VirtualAddress, *(u32ptr)Instruction);

        MmUnmapIoSpace(VirtualAddress, Length);
    }
}

void
NTAPI
MapLockedBuildJumpCode(
    __inout ptr * Pointer,
    __in ptr Guard
)
{
    GUARD_BODY GuardBody = { 0 };

    SetGuardBody(&GuardBody, Guard);

    MapLockedCopyInstruction(
        *Pointer,
        &GuardBody,
        GUARD_BODY_CODE_LENGTH);
}

void
NTAPI
LockedCopyInstruction(
    __in ptr Destination,
    __in ptr Source,
    __in u32 Length
)
{
    s8 Instruction[4] = { 0 };

    if (Length > sizeof(s32)) {
        RtlCopyMemory(Instruction, Source, sizeof(s32));
    }
    else {
        RtlCopyMemory(Instruction, Source, Length);

        RtlCopyMemory(
            Instruction + Length,
            (u8ptr)Destination + Length,
            sizeof(s32) - Length);
    }

    if (Length > sizeof(s32)) {
        InterlockedExchange(Destination, JUMP_SELF);

        RtlCopyMemory(
            (u8ptr)Destination + sizeof(s32),
            (u8ptr)Source + sizeof(s32),
            Length - sizeof(s32));
    }

    InterlockedExchange(Destination, *(s32ptr)Instruction);
}

void
NTAPI
LockedBuildJumpCode(
    __inout ptr * Pointer,
    __in ptr Guard
)
{
    GUARD_BODY GuardBody = { 0 };

    SetGuardBody(&GuardBody, Guard);

    LockedCopyInstruction(
        *Pointer,
        &GuardBody,
        GUARD_BODY_CODE_LENGTH);
}

#ifdef _WIN64
ptr
NTAPI
DisCopy8B(
    __in_opt ptr Dst,
    __in_opt ptr * DstPool,
    __in ptr Src,
    __out_opt ptr * Target,
    __out s32 * Extra
)
{
    PCHAR Instruction = NULL;
    u8 Prefix = 0;
    u8 Code = 0;
    u32 Length = 0;
    u32 GpReg = 0;
    ptr RealAddress = 0;
    ptr ReturnAddress = NULL;

    struct {
        u8 Source : 3;
        u8 Destination : 3;
        u8 Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    struct DECLSPEC_ALIGN(1) {
        u8 Prefix;

        struct {
            u8 GpReg : 3;
            u8 Inst : 5;
        }Code;

        u8 RealAddress[8];

        u8 CopyPrefix;
        u8 CopyInst;

        struct {
            u8 Source : 3;
            u8 Destination : 3;
            u8 Mod : 2;
        }CopyMod;
    }*CopyOpCode;

    C_ASSERT(sizeof(*CopyOpCode) == 13);

    Instruction = Src;

    Prefix = Instruction[0];
    Code = Instruction[1];
    ModRM = &Instruction[2];

    Instruction += 3;

    if (0 == ModRM->Mod &&
        5 == ModRM->Source) {
        // [--][--]
        // [--][--] + disp8
        // [--][--] + disp32

        GpReg = ModRM->Destination;
        RealAddress = __rva_to_va(Instruction);

        if (NULL != Target) {
            *Target = NULL;
        }

        if (NULL != Extra) {
            *Extra += 6;
        }

        if (NULL != Dst && NULL != DstPool) {
            if ((u8ptr)*DstPool - (u8ptr)Dst >= sizeof(CopyOpCode)) {
                CopyOpCode = Dst;

                CopyOpCode->Prefix = Prefix;
                CopyOpCode->Code.Inst = 0x17; // Reg <- Immediate
                CopyOpCode->Code.GpReg = GpReg;

                *(ptr *)&CopyOpCode->RealAddress = RealAddress;

                CopyOpCode->CopyPrefix = Prefix;
                CopyOpCode->CopyInst = 0x8b; // Reg <- [Reg]
                CopyOpCode->CopyMod.Destination = GpReg;
                CopyOpCode->CopyMod.Source = GpReg;
                CopyOpCode->CopyMod.Mod = 0;
            }
        }

        ReturnAddress = Instruction + sizeof(s32);
    }

    return ReturnAddress;
}
#endif // _WIN64

#ifndef _WIN64
PPATCH_HEADER
NTAPI
HotpatchAttach(
    __inout ptr * Pointer,
    __in ptr Hotpatch
)
{
    PHOTPATCH_OBJECT HotpatchObjct = NULL;
    PHOTPATCH_BODY HotpatchBody = NULL;
    s32 Relative = 0;

    HotpatchObjct = GuardAllocateTrampoline(sizeof(HOTPATCH_OBJECT));

    if (NULL != HotpatchObjct) {
        RtlZeroMemory(HotpatchObjct, sizeof(HOTPATCH_OBJECT));

        HotpatchObjct->Header.Length = sizeof(HOTPATCH_OBJECT);

        HotpatchBody = CONTAINING_RECORD(
            *Pointer,
            HOTPATCH_BODY,
            JmpSelf);

        MapLockedCopyInstruction(
            &HotpatchBody->Jmp,
            HOTPATCH_BODY_CODE,
            HOTPATCH_BODY_CODE_LENGTH - HOTPATCH_MASK_LENGTH);

        Relative =
            PtrToLong(Hotpatch) - PtrToLong(&HotpatchBody->JmpSelf);

        MapLockedCopyInstruction(
            &HotpatchBody->Hotpatch,
            &Relative,
            sizeof(s32));

        MapLockedCopyInstruction(
            &HotpatchBody->JmpSelf,
            HOTPATCH_BODY_CODE + FIELD_OFFSET(HOTPATCH_BODY, JmpSelf),
            RTL_FIELD_SIZE(HOTPATCH_BODY, JmpSelf));

        HotpatchObjct->Header.Entry = HotpatchBody + 1;
        HotpatchObjct->Header.ProgramCounter = *Pointer;
        HotpatchObjct->Header.Target = Hotpatch;

        MapLockedCopyInstruction(
            Pointer,
            &HotpatchObjct->Header.Entry,
            sizeof(ptr));
    }

    return HotpatchObjct;
}

void
NTAPI
HotpatchDetach(
    __inout ptr * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in ptr Hotpatch
)
{
    PHOTPATCH_OBJECT HotpatchObjct = NULL;
    PHOTPATCH_BODY HotpatchBody = NULL;

    HotpatchObjct = CONTAINING_RECORD(
        PatchHeader,
        HOTPATCH_OBJECT,
        Header);

    HotpatchBody = CONTAINING_RECORD(
        HotpatchObjct->Header.ProgramCounter,
        HOTPATCH_BODY,
        JmpSelf);

    MapLockedCopyInstruction(
        &HotpatchBody->JmpSelf,
        HOTPATCH_MASK,
        HOTPATCH_MASK_LENGTH);

    MapLockedCopyInstruction(
        Pointer,
        &HotpatchObjct->Header.ProgramCounter,
        sizeof(ptr));

    GuardFreeTrampoline(HotpatchObjct, HotpatchObjct->Header.Length);
}
#endif // !_WIN64

PPATCH_HEADER
NTAPI
GuardAttach(
    __inout ptr * Pointer,
    __in ptr Guard
)
{
    PGUARD_OBJECT GuardObject = NULL;
    u8ptr GuardBody = NULL;
    u8ptr Import = NULL;
    ptr Address = NULL;
    u8ptr ControlPc = NULL;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    s32 Extra = 0;
    u8ptr Target = NULL;
    u32 BytesCopied = 0;
    u8ptr Header = NULL;
    u32 FunctionCount = 1;
    u32 CodeLength = 0;

#ifdef _WIN64
    u32 FunctionIndex = 1;
    u8ptr FunctionEntry = NULL;
    u32 Index = 0;
    u8ptr Instruction = NULL;
    u32 InstructionLength = 0;
    u8ptr RealAddress = NULL;
#endif // _WIN64

    struct {
        u8 B : 1;
        u8 X : 1;
        u8 R : 1;

        // 0 = Operand size determined by CS.D
        // 1 = 64 Bit Operand Size

        u8 W : 1;
        u8 Reserved : 4; // always 0100
    }*Rex;

    struct {
        u8 Source : 3;
        u8 Destination : 3;
        u8 Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    Address = *Pointer;
    *Pointer = NULL;

    ControlPc = Address;

    while (Length < GUARD_BODY_CODE_LENGTH) {
        TargetPc = DetourCopyInstruction(
            NULL,
            NULL,
            ControlPc,
            &Target,
            &Extra);

        if (NULL != TargetPc) {
#ifdef _WIN64
            if (7 == TargetPc - ControlPc) {
                Rex = ControlPc;

                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                    0 == _cmpbyte(ControlPc[1], 0x8b)) {
                    ModRM = &ControlPc[2];

                    if (0 == ModRM->Mod &&
                        5 == ModRM->Source) {
                        // [--][--]
                        // [--][--] + disp8
                        // [--][--] + disp32

                        TargetPc = DisCopy8B(
                            NULL,
                            NULL,
                            ControlPc,
                            &Target,
                            &Extra);
                    }
                }
            }
#endif // _WIN64

#ifdef _WIN64
            if (NULL != Target) {
                FunctionCount++;
            }
#endif // _WIN64

            Length += TargetPc - ControlPc;
            CodeLength += TargetPc - ControlPc + Extra;

            if (Length >= GUARD_BODY_CODE_LENGTH) {
                GuardObject = GuardAllocateTrampoline(
                    sizeof(GUARD_OBJECT) +
                    Length +
                    CodeLength +
                    GUARD_BODY_CODE_LENGTH * FunctionCount);

                if (NULL != GuardObject) {
                    RtlZeroMemory(
                        GuardObject,
                        sizeof(GUARD_OBJECT)
                        + Length
                        + CodeLength
                        + GUARD_BODY_CODE_LENGTH * FunctionCount);

                    GuardObject->Header.Length = sizeof(GUARD_OBJECT)
                        + Length
                        + CodeLength
                        + GUARD_BODY_CODE_LENGTH * FunctionCount;

                    GuardObject->Original = GuardObject + 1;
                    GuardObject->Length = Length;

                    GuardBody = (u8ptr)GuardObject->Original + GuardObject->Length;
                    Import = GuardBody + CodeLength;

                    RtlCopyMemory(GuardObject->Original, Address, GuardObject->Length);

                    ControlPc = Address;
                    Length = 0;

                    while (Length < GUARD_BODY_CODE_LENGTH) {
                        TargetPc = DetourCopyInstruction(
                            GuardBody + BytesCopied,
                            &Import,
                            ControlPc,
                            &Target,
                            &Extra);

                        if (NULL != TargetPc) {
#ifdef _WIN64
                            if (7 == TargetPc - ControlPc) {
                                Rex = ControlPc;

                                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                                    0 == _cmpbyte(ControlPc[1], 0x8b)) {
                                    ModRM = &ControlPc[2];

                                    if (0 == ModRM->Mod &&
                                        5 == ModRM->Source) {
                                        // [--][--]
                                        // [--][--] + disp8
                                        // [--][--] + disp32

                                        TargetPc = DisCopy8B(
                                            GuardBody + BytesCopied,
                                            &Import,
                                            ControlPc,
                                            &Target,
                                            &Extra);
                                    }
                                }
                            }
#endif // _WIN64

#ifdef _WIN64
                            if (NULL != Target) {
                                Instruction = GuardBody + BytesCopied;
                                InstructionLength = TargetPc - ControlPc + Extra;

                                FunctionEntry =
                                    Import + GUARD_BODY_CODE_LENGTH * FunctionIndex;

                                LockedBuildJumpCode(&FunctionEntry, Target);

                                for (Index = 0;
                                    Index <= InstructionLength - sizeof(s32);
                                    Index++) {
                                    RealAddress = __rva_to_va(Instruction + Index);

                                    if (*(s32ptr)&Target == *(s32ptr)&RealAddress) {
                                        *(s32ptr)(Instruction + Index) =
                                            FunctionEntry - ((Instruction + Index) + sizeof(s32));
                                    }
                                }

                                FunctionIndex++;
                            }
#endif // _WIN64

                            Length += TargetPc - ControlPc;
                            BytesCopied += TargetPc - ControlPc + Extra;

                            if (Length >= GUARD_BODY_CODE_LENGTH) {
                                LockedBuildJumpCode(&Import, TargetPc);
                                MapLockedBuildJumpCode(&Address, Guard);

                                GuardObject->Header.Entry = GuardBody;
                                GuardObject->Header.ProgramCounter = Address;
                                GuardObject->Header.Target = Guard;

                                LockedCopyInstruction(
                                    Pointer,
                                    &GuardObject->Header.Entry,
                                    sizeof(ptr));

                                break;
                            }

                            ControlPc = TargetPc;
                        }
                        else {
                            break;
                        }
                    }
                }

                break;
            }

            ControlPc = TargetPc;
        }
        else {
            break;
        }
    }

    return GuardObject;
}

void
NTAPI
GuardDetach(
    __inout ptr * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in ptr Guard
)
{
    PGUARD_OBJECT GuardObject = NULL;

    GuardObject = CONTAINING_RECORD(
        PatchHeader,
        GUARD_OBJECT,
        Header);

    if (PatchHeader->Target == Guard &&
        *Pointer == GuardObject->Header.Entry) {
        MapLockedCopyInstruction(
            GuardObject->Header.ProgramCounter,
            GuardObject->Original,
            GuardObject->Length);

        LockedCopyInstruction(
            Pointer,
            &GuardObject->Header.ProgramCounter,
            sizeof(ptr));

        GuardFreeTrampoline(GuardObject, GuardObject->Header.Length);
    }
}

PPATCH_HEADER
NTAPI
SafeGuardAttach(
    __inout ptr * Pointer,
    __in PGUARD_CALLBACK Callback,
    __in_opt ptr CaptureContext,
    __in_opt ptr Parameter,
    __in_opt ptr Reserved
)
{
    PSAFEGUARD_OBJECT GuardObject = NULL;
    u8ptr GuardBody = NULL;
    u8ptr Import = NULL;
    ptr Address = NULL;
    u8ptr ControlPc = NULL;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    s32 Extra = 0;
    u8ptr Target = NULL;
    u32 BytesCopied = 0;
    u8ptr Header = NULL;
    u32 FunctionCount = 1;
    u32 CodeLength = 0;

#ifdef _WIN64
    u32 FunctionIndex = 1;
    u8ptr FunctionEntry = NULL;
    u32 Index = 0;
    u8ptr Instruction = NULL;
    u32 InstructionLength = 0;
    u8ptr RealAddress = NULL;
#endif // _WIN64

    struct {
        u8 B : 1;
        u8 X : 1;
        u8 R : 1;

        // 0 = Operand size determined by CS.D
        // 1 = 64 Bit Operand Size

        u8 W : 1;
        u8 Reserved : 4; // always 0100
    }*Rex;

    struct {
        u8 Source : 3;
        u8 Destination : 3;
        u8 Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    Address = *Pointer;
    *Pointer = NULL;

    ControlPc = Address;

    while (Length < GUARD_BODY_CODE_LENGTH) {
        TargetPc = DetourCopyInstruction(
            NULL,
            NULL,
            ControlPc,
            &Target,
            &Extra);

        if (NULL != TargetPc) {
#ifdef _WIN64
            if (7 == TargetPc - ControlPc) {
                Rex = ControlPc;

                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                    0 == _cmpbyte(ControlPc[1], 0x8b)) {
                    ModRM = &ControlPc[2];

                    if (0 == ModRM->Mod &&
                        5 == ModRM->Source) {
                        // [--][--]
                        // [--][--] + disp8
                        // [--][--] + disp32

                        TargetPc = DisCopy8B(
                            NULL,
                            NULL,
                            ControlPc,
                            &Target,
                            &Extra);
                    }
                }
            }
#endif // _WIN64

#ifdef _WIN64
            if (NULL != Target) {
                FunctionCount++;
            }
#endif // _WIN64

            Length += TargetPc - ControlPc;
            CodeLength += TargetPc - ControlPc + Extra;

            if (Length >= GUARD_BODY_CODE_LENGTH) {
                GuardObject = GuardAllocateTrampoline(
                    sizeof(SAFEGUARD_OBJECT) +
                    Length +
                    CodeLength +
                    GUARD_BODY_CODE_LENGTH * FunctionCount);

                if (NULL != GuardObject) {
                    RtlZeroMemory(
                        GuardObject,
                        sizeof(SAFEGUARD_OBJECT)
                        + Length
                        + CodeLength
                        + GUARD_BODY_CODE_LENGTH * FunctionCount);

                    GuardObject->Header.Length = sizeof(SAFEGUARD_OBJECT)
                        + Length
                        + CodeLength
                        + GUARD_BODY_CODE_LENGTH * FunctionCount;

                    GuardObject->Original = GuardObject + 1;
                    GuardObject->Length = Length;

                    GuardBody = (u8ptr)GuardObject->Original + GuardObject->Length;
                    Import = GuardBody + CodeLength;

                    RtlCopyMemory(GuardObject->Original, Address, GuardObject->Length);

                    ControlPc = Address;
                    Length = 0;

                    while (Length < GUARD_BODY_CODE_LENGTH) {
                        TargetPc = DetourCopyInstruction(
                            GuardBody + BytesCopied,
                            &Import,
                            ControlPc,
                            &Target,
                            &Extra);

                        if (NULL != TargetPc) {
#ifdef _WIN64
                            if (7 == TargetPc - ControlPc) {
                                Rex = ControlPc;

                                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                                    0 == _cmpbyte(ControlPc[1], 0x8b)) {
                                    ModRM = &ControlPc[2];

                                    if (0 == ModRM->Mod &&
                                        5 == ModRM->Source) {
                                        // [--][--]
                                        // [--][--] + disp8
                                        // [--][--] + disp32

                                        TargetPc = DisCopy8B(
                                            GuardBody + BytesCopied,
                                            &Import,
                                            ControlPc,
                                            &Target,
                                            &Extra);
                                    }
                                }
                            }
#endif // _WIN64

#ifdef _WIN64
                            if (NULL != Target) {
                                Instruction = GuardBody + BytesCopied;
                                InstructionLength = TargetPc - ControlPc + Extra;

                                FunctionEntry =
                                    Import + GUARD_BODY_CODE_LENGTH * FunctionIndex;

                                LockedBuildJumpCode(&FunctionEntry, Target);

                                for (Index = 0;
                                    Index <= InstructionLength - sizeof(s32);
                                    Index++) {
                                    RealAddress = __rva_to_va(Instruction + Index);

                                    if (*(s32ptr)&Target == *(s32ptr)&RealAddress) {
                                        *(s32ptr)(Instruction + Index) =
                                            FunctionEntry - ((Instruction + Index) + sizeof(s32));
                                    }
                                }

                                FunctionIndex++;
                            }
#endif // _WIN64

                            Length += TargetPc - ControlPc;
                            BytesCopied += TargetPc - ControlPc + Extra;

                            if (Length >= GUARD_BODY_CODE_LENGTH) {
                                LockedBuildJumpCode(&Import, TargetPc);

                                CaptureContext =
                                    NULL == CaptureContext ?
                                    _CaptureContext : CaptureContext;

                                SetStackBody(
                                    &GuardObject->Body,
                                    Reserved,
                                    Parameter,
                                    Callback,
                                    GuardBody,
                                    Address,
                                    CaptureContext);

                                MapLockedBuildJumpCode(&Address, &GuardObject->Body);

                                GuardObject->Header.Entry = GuardBody;
                                GuardObject->Header.ProgramCounter = Address;
                                GuardObject->Header.Target = Callback;

                                LockedCopyInstruction(
                                    Pointer,
                                    &GuardObject->Header.Entry,
                                    sizeof(ptr));

                                break;
                            }

                            ControlPc = TargetPc;
                        }
                        else {
                            break;
                        }
                    }
                }

                break;
            }

            ControlPc = TargetPc;
        }
        else {
            break;
        }
    }

    return GuardObject;
}

void
NTAPI
SafeGuardDetach(
    __inout ptr * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in PGUARD_CALLBACK Callback
)
{
    PSAFEGUARD_OBJECT GuardObject = NULL;

    GuardObject = CONTAINING_RECORD(
        PatchHeader,
        SAFEGUARD_OBJECT,
        Header);

    if (PatchHeader->Target == Callback &&
        *Pointer == GuardObject->Header.Entry) {
        MapLockedCopyInstruction(
            GuardObject->Header.ProgramCounter,
            GuardObject->Original,
            GuardObject->Length);

        LockedCopyInstruction(
            Pointer,
            &GuardObject->Header.ProgramCounter,
            sizeof(ptr));

        GuardFreeTrampoline(GuardObject, GuardObject->Header.Length);
    }
}
