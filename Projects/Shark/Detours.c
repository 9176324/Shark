/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#include "Detours.h"

#include "Scan.h"
#include "Reload.h"

VOID
NTAPI
DetourMapLockedCopyInstruction(
    __in PVOID Destination,
    __in PVOID Source,
    __in ULONG Length
)
{
    CHAR Instruction[4] = { 0 };
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    PVOID VirtualAddress = NULL;

    if (Length > sizeof(ULONG)) {
        RtlCopyMemory(Instruction, Source, sizeof(ULONG));
    }
    else {
        RtlCopyMemory(Instruction, Source, Length);

        RtlCopyMemory(
            Instruction + Length,
            (PCHAR)Destination + Length,
            sizeof(ULONG) - Length);
    }

    PhysicalAddress = MmGetPhysicalAddress(Destination);

    VirtualAddress = MmMapIoSpace(
        PhysicalAddress,
        Length,
        MmNonCached);

    if (NULL != VirtualAddress) {
        if (Length > sizeof(ULONG)) {
            InterlockedExchange(VirtualAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)VirtualAddress + sizeof(ULONG),
                (PCHAR)Source + sizeof(ULONG),
                Length - sizeof(ULONG));
        }

        InterlockedExchange(VirtualAddress, *(PULONG)Instruction);

        MmUnmapIoSpace(VirtualAddress, Length);
    }
}

VOID
NTAPI
DetourMapLockedBuildJumpCode(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    DETOUR_BODY DetourBody = { 0 };

    SetDetourBody(&DetourBody, Detour);

    DetourMapLockedCopyInstruction(
        *Pointer,
        &DetourBody,
        DETOUR_BODY_CODE_LENGTH);
}

VOID
NTAPI
DetourLockedCopyInstruction(
    __in PVOID Destination,
    __in PVOID Source,
    __in ULONG Length
)
{
    CHAR Instruction[4] = { 0 };

    if (Length > sizeof(LONG)) {
        RtlCopyMemory(Instruction, Source, sizeof(LONG));
    }
    else {
        RtlCopyMemory(Instruction, Source, Length);

        RtlCopyMemory(
            Instruction + Length,
            (PCHAR)Destination + Length,
            sizeof(LONG) - Length);
    }

    if (Length > sizeof(LONG)) {
        InterlockedExchange(Destination, JUMP_SELF);

        RtlCopyMemory(
            (PCHAR)Destination + sizeof(LONG),
            (PCHAR)Source + sizeof(LONG),
            Length - sizeof(LONG));
    }

    InterlockedExchange(Destination, *(PLONG)Instruction);
}

VOID
NTAPI
DetourLockedBuildJumpCode(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    DETOUR_BODY DetourBody = { 0 };

    SetDetourBody(&DetourBody, Detour);

    DetourLockedCopyInstruction(
        *Pointer,
        &DetourBody,
        DETOUR_BODY_CODE_LENGTH);
}

#ifdef _WIN64
PVOID
NTAPI
DisCopy8B(
    __in_opt PVOID Dst,
    __in_opt PVOID * DstPool,
    __in PVOID Src,
    __out_opt PVOID * Target,
    __out LONG * Extra
)
{
    PCHAR Instruction = NULL;
    UCHAR Prefix = 0;
    UCHAR Code = 0;
    ULONG Length = 0;
    ULONG GpReg = 0;
    PVOID RealAddress = 0;
    PVOID ReturnAddress = NULL;

    struct {
        UCHAR Source : 3;
        UCHAR Destination : 3;
        UCHAR Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    struct DECLSPEC_ALIGN(1) {
        UCHAR Prefix;

        struct {
            UCHAR GpReg : 3;
            UCHAR Inst : 5;
        }Code;

        UCHAR RealAddress[8];

        UCHAR CopyPrefix;
        UCHAR CopyInst;

        struct {
            UCHAR Source : 3;
            UCHAR Destination : 3;
            UCHAR Mod : 2;
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
        // [disp32]

        GpReg = ModRM->Destination;
        RealAddress = RvaToVa(Instruction);

        if (NULL != Target) {
            *Target = NULL;
        }

        if (NULL != Extra) {
            *Extra += 6;
        }

        if (NULL != Dst && NULL != DstPool) {
            if ((PCHAR)*DstPool - (PCHAR)Dst >= sizeof(CopyOpCode)) {
                CopyOpCode = Dst;

                CopyOpCode->Prefix = Prefix;
                CopyOpCode->Code.Inst = 0x17; // Reg <- Immediate
                CopyOpCode->Code.GpReg = GpReg;

                *(PVOID *)&CopyOpCode->RealAddress = RealAddress;

                CopyOpCode->CopyPrefix = Prefix;
                CopyOpCode->CopyInst = 0x8b; // Reg <- [Reg]
                CopyOpCode->CopyMod.Destination = GpReg;
                CopyOpCode->CopyMod.Source = GpReg;
                CopyOpCode->CopyMod.Mod = 0;
            }
        }

        ReturnAddress = Instruction + sizeof(LONG);
    }

    return ReturnAddress;
}
#endif // _WIN64

#ifndef _WIN64
PPATCH_HEADER
NTAPI
HotpatchAttach(
    __inout PVOID * Pointer,
    __in PVOID Hotpatch
)
{
    PHOTPATCH_OBJECT HotpatchObjct = NULL;
    PHOTPATCH_BODY HotpatchBody = NULL;
    LONG Relative = 0;

    HotpatchObjct = ExAllocatePool(
        NonPagedPool,
        sizeof(HOTPATCH_OBJECT));

    if (NULL != HotpatchObjct) {
        RtlZeroMemory(HotpatchObjct, sizeof(HOTPATCH_OBJECT));

        HotpatchBody = CONTAINING_RECORD(
            *Pointer,
            HOTPATCH_BODY,
            JmpSelf);

        DetourMapLockedCopyInstruction(
            &HotpatchBody->Jmp,
            HOTPATCH_BODY_CODE,
            HOTPATCH_BODY_CODE_LENGTH - HOTPATCH_MASK_LENGTH);

        Relative =
            PtrToLong(Hotpatch) - PtrToLong(&HotpatchBody->JmpSelf);

        DetourMapLockedCopyInstruction(
            &HotpatchBody->Hotpatch,
            &Relative,
            sizeof(LONG));

        DetourMapLockedCopyInstruction(
            &HotpatchBody->JmpSelf,
            HOTPATCH_BODY_CODE + FIELD_OFFSET(HOTPATCH_BODY, JmpSelf),
            RTL_FIELD_SIZE(HOTPATCH_BODY, JmpSelf));

        HotpatchObjct->Header.Entry = HotpatchBody + 1;
        HotpatchObjct->Header.ProgramCounter = *Pointer;
        HotpatchObjct->Header.Target = Hotpatch;

        DetourMapLockedCopyInstruction(
            Pointer,
            &HotpatchObjct->Header.Entry,
            sizeof(PVOID));
    }

    return HotpatchObjct;
}

VOID
NTAPI
HotpatchDetach(
    __inout PVOID * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in PVOID Hotpatch
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

    DetourMapLockedCopyInstruction(
        &HotpatchBody->JmpSelf,
        HOTPATCH_MASK,
        HOTPATCH_MASK_LENGTH);

    DetourMapLockedCopyInstruction(
        Pointer,
        &HotpatchObjct->Header.ProgramCounter,
        sizeof(PVOID));

    ExFreePool(HotpatchObjct);
}
#endif // !_WIN64

PPATCH_HEADER
NTAPI
DetourAttach(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    PDETOUR_OBJECT DetourObjct = NULL;
    PCHAR DetourBody = NULL;
    PCHAR Import = NULL;
    PVOID Address = NULL;
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    ULONG Length = 0;
    LONG Extra = 0;
    PCHAR Target = NULL;
    ULONG BytesCopied = 0;
    PCHAR Header = NULL;
    ULONG FunctionCount = 1;
    ULONG CodeLength = 0;

#ifdef _WIN64
    ULONG FunctionIndex = 1;
    PCHAR FunctionEntry = NULL;
    ULONG Index = 0;
    PCHAR Instruction = NULL;
    ULONG InstructionLength = 0;
    PCHAR RealAddress = NULL;
#endif // _WIN64

    struct {
        UCHAR B : 1;
        UCHAR X : 1;
        UCHAR R : 1;

        // 0 = Operand size determined by CS.D
        // 1 = 64 Bit Operand Size

        UCHAR W : 1;
        UCHAR Reserved : 4; // always 0100
    }*Rex;

    struct {
        UCHAR Source : 3;
        UCHAR Destination : 3;
        UCHAR Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    Address = *Pointer;
    *Pointer = NULL;

    ControlPc = Address;

    while (Length < DETOUR_BODY_CODE_LENGTH) {
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
                    0 == _CmpByte(ControlPc[1], 0x8b)) {
                    ModRM = &ControlPc[2];

                    if (0 == ModRM->Mod &&
                        5 == ModRM->Source) {
                        // [disp32]

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

            if (Length >= DETOUR_BODY_CODE_LENGTH) {
                DetourObjct = ExAllocatePool(
                    NonPagedPool,
                    sizeof(DETOUR_OBJECT) +
                    Length +
                    CodeLength +
                    DETOUR_BODY_CODE_LENGTH * FunctionCount);

                if (NULL != DetourObjct) {
                    RtlZeroMemory(
                        DetourObjct,
                        sizeof(DETOUR_OBJECT) +
                        Length +
                        CodeLength +
                        DETOUR_BODY_CODE_LENGTH * FunctionCount);

                    DetourObjct->Original = DetourObjct + 1;
                    DetourObjct->Length = Length;

                    DetourBody = (PCHAR)DetourObjct->Original + DetourObjct->Length;
                    Import = DetourBody + CodeLength;

                    RtlCopyMemory(DetourObjct->Original, Address, DetourObjct->Length);

                    ControlPc = Address;
                    Length = 0;

                    while (Length < DETOUR_BODY_CODE_LENGTH) {
                        TargetPc = DetourCopyInstruction(
                            DetourBody + BytesCopied,
                            &Import,
                            ControlPc,
                            &Target,
                            &Extra);

                        if (NULL != TargetPc) {
#ifdef _WIN64
                            if (7 == TargetPc - ControlPc) {
                                Rex = ControlPc;

                                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                                    0 == _CmpByte(ControlPc[1], 0x8b)) {
                                    ModRM = &ControlPc[2];

                                    if (0 == ModRM->Mod &&
                                        5 == ModRM->Source) {
                                        // [disp32]

                                        TargetPc = DisCopy8B(
                                            DetourBody + BytesCopied,
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
                                Instruction = DetourBody + BytesCopied;
                                InstructionLength = TargetPc - ControlPc + Extra;

                                FunctionEntry =
                                    Import + DETOUR_BODY_CODE_LENGTH * FunctionIndex;

                                DetourLockedBuildJumpCode(&FunctionEntry, Target);

                                for (Index = 0;
                                    Index <= InstructionLength - sizeof(LONG);
                                    Index++) {
                                    RealAddress = RvaToVa(Instruction + Index);

                                    if (*(PLONG)&Target == *(PLONG)&RealAddress) {
                                        *(PLONG)(Instruction + Index) =
                                            FunctionEntry - ((Instruction + Index) + sizeof(LONG));
                                    }
                                }

                                FunctionIndex++;
                            }
#endif // _WIN64

                            Length += TargetPc - ControlPc;
                            BytesCopied += TargetPc - ControlPc + Extra;

                            if (Length >= DETOUR_BODY_CODE_LENGTH) {
                                DetourLockedBuildJumpCode(&Import, TargetPc);
                                DetourMapLockedBuildJumpCode(&Address, Detour);

                                DetourObjct->Header.Entry = DetourBody;
                                DetourObjct->Header.ProgramCounter = Address;
                                DetourObjct->Header.Target = Detour;

                                DetourLockedCopyInstruction(
                                    Pointer,
                                    &DetourObjct->Header.Entry,
                                    sizeof(PVOID));

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

    return DetourObjct;
}

VOID
NTAPI
DetourDetach(
    __inout PVOID * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in PVOID Detour
)
{
    PDETOUR_OBJECT DetourObjct = NULL;

    DetourObjct = CONTAINING_RECORD(
        PatchHeader,
        DETOUR_OBJECT,
        Header);

    if (PatchHeader->Target == Detour &&
        *Pointer == DetourObjct->Header.Entry) {
        DetourMapLockedCopyInstruction(
            DetourObjct->Header.ProgramCounter,
            DetourObjct->Original,
            DetourObjct->Length);

        DetourLockedCopyInstruction(
            Pointer,
            &DetourObjct->Header.ProgramCounter,
            sizeof(PVOID));

        ExFreePool(DetourObjct);
    }
}

PPATCH_HEADER
NTAPI
DetourGuardAttach(
    __inout PVOID * Pointer,
    __in PGUARD Guard,
    __in_opt PVOID Parameter,
    __in_opt PVOID Reserved
)
{
    PGUARD_OBJECT GuardObject = NULL;
    PCHAR DetourBody = NULL;
    PCHAR Import = NULL;
    PVOID Address = NULL;
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    ULONG Length = 0;
    LONG Extra = 0;
    PCHAR Target = NULL;
    ULONG BytesCopied = 0;
    PCHAR Header = NULL;
    ULONG FunctionCount = 1;
    ULONG CodeLength = 0;

#ifdef _WIN64
    ULONG FunctionIndex = 1;
    PCHAR FunctionEntry = NULL;
    ULONG Index = 0;
    PCHAR Instruction = NULL;
    ULONG InstructionLength = 0;
    PCHAR RealAddress = NULL;
#endif // _WIN64

    struct {
        UCHAR B : 1;
        UCHAR X : 1;
        UCHAR R : 1;

        // 0 = Operand size determined by CS.D
        // 1 = 64 Bit Operand Size

        UCHAR W : 1;
        UCHAR Reserved : 4; // always 0100
    }*Rex;

    struct {
        UCHAR Source : 3;
        UCHAR Destination : 3;
        UCHAR Mod : 2;
    }*ModRM;

    C_ASSERT(sizeof(*ModRM) == 1);

    Address = *Pointer;
    *Pointer = NULL;

    ControlPc = Address;

    while (Length < DETOUR_BODY_CODE_LENGTH) {
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
                    0 == _CmpByte(ControlPc[1], 0x8b)) {
                    ModRM = &ControlPc[2];

                    if (0 == ModRM->Mod &&
                        5 == ModRM->Source) {
                        // [disp32]

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

            if (Length >= DETOUR_BODY_CODE_LENGTH) {
                GuardObject = ExAllocatePool(
                    NonPagedPool,
                    sizeof(GUARD_OBJECT) +
                    Length +
                    CodeLength +
                    DETOUR_BODY_CODE_LENGTH * FunctionCount);

                if (NULL != GuardObject) {
                    RtlZeroMemory(
                        GuardObject,
                        sizeof(GUARD_OBJECT) +
                        Length +
                        CodeLength +
                        DETOUR_BODY_CODE_LENGTH * FunctionCount);

                    GuardObject->Original = GuardObject + 1;
                    GuardObject->Length = Length;

                    DetourBody = (PCHAR)GuardObject->Original + GuardObject->Length;
                    Import = DetourBody + CodeLength;

                    RtlCopyMemory(GuardObject->Original, Address, GuardObject->Length);

                    ControlPc = Address;
                    Length = 0;

                    while (Length < DETOUR_BODY_CODE_LENGTH) {
                        TargetPc = DetourCopyInstruction(
                            DetourBody + BytesCopied,
                            &Import,
                            ControlPc,
                            &Target,
                            &Extra);

                        if (NULL != TargetPc) {
#ifdef _WIN64
                            if (7 == TargetPc - ControlPc) {
                                Rex = ControlPc;

                                if ((4 == Rex->Reserved && 1 == Rex->W) &&
                                    0 == _CmpByte(ControlPc[1], 0x8b)) {
                                    ModRM = &ControlPc[2];

                                    if (0 == ModRM->Mod &&
                                        5 == ModRM->Source) {
                                        // [disp32]

                                        TargetPc = DisCopy8B(
                                            DetourBody + BytesCopied,
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
                                Instruction = DetourBody + BytesCopied;
                                InstructionLength = TargetPc - ControlPc + Extra;

                                FunctionEntry =
                                    Import + DETOUR_BODY_CODE_LENGTH * FunctionIndex;

                                DetourLockedBuildJumpCode(&FunctionEntry, Target);

                                for (Index = 0;
                                    Index <= InstructionLength - sizeof(LONG);
                                    Index++) {
                                    RealAddress = RvaToVa(Instruction + Index);

                                    if (*(PLONG)&Target == *(PLONG)&RealAddress) {
                                        *(PLONG)(Instruction + Index) =
                                            FunctionEntry - ((Instruction + Index) + sizeof(LONG));
                                    }
                                }

                                FunctionIndex++;
                            }
#endif // _WIN64

                            Length += TargetPc - ControlPc;
                            BytesCopied += TargetPc - ControlPc + Extra;

                            if (Length >= DETOUR_BODY_CODE_LENGTH) {
                                DetourLockedBuildJumpCode(&Import, TargetPc);

                                SetGuardBody(
                                    &GuardObject->Body,
                                    Reserved,
                                    Parameter,
                                    Guard,
                                    DetourBody,
                                    Address,
                                    GpBlock->CaptureContext);

                                DetourMapLockedBuildJumpCode(&Address, &GuardObject->Body);

                                GuardObject->Header.Entry = DetourBody;
                                GuardObject->Header.ProgramCounter = Address;
                                GuardObject->Header.Target = Guard;

                                DetourLockedCopyInstruction(
                                    Pointer,
                                    &GuardObject->Header.Entry,
                                    sizeof(PVOID));

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

VOID
NTAPI
DetourGuardDetach(
    __inout PVOID * Pointer,
    __in PPATCH_HEADER PatchHeader,
    __in PGUARD Guard
)
{
    PGUARD_OBJECT GuardObject = NULL;

    GuardObject = CONTAINING_RECORD(
        PatchHeader,
        GUARD_OBJECT,
        Header);

    if (PatchHeader->Target == Guard &&
        *Pointer == GuardObject->Header.Entry) {
        DetourMapLockedCopyInstruction(
            GuardObject->Header.ProgramCounter,
            GuardObject->Original,
            GuardObject->Length);

        DetourLockedCopyInstruction(
            Pointer,
            &GuardObject->Header.ProgramCounter,
            sizeof(PVOID));

        ExFreePool(GuardObject);
    }
}
