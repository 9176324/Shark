/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

ULONG
NTAPI
DetourGetInstructionLength(
    __in PVOID ControlPc
)
{
    PCHAR TargetPc = NULL;
    LONG Extra = 0;
    ULONG Length = 0;

    TargetPc = DetourCopyInstruction(
        NULL,
        NULL,
        ControlPc,
        NULL,
        &Extra);

    if (NULL != TargetPc) {
        Length += (ULONG)(TargetPc - ControlPc);
    }

    return Length;
}

VOID
NTAPI
DetourLockCopyInstruction(
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
DetourBuildJumpCode32(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    JMPCODE JmpCode32 = { 0 };

    if ((ULONG_PTR)Detour <= MAXULONG) {
        RtlCopyMemory(&JmpCode32, JUMP_CODE32, JUMP_CODE32_LENGTH);
        RtlCopyMemory(&JmpCode32.u1.JumpAddress, &Detour, sizeof(ULONG));

        DetourLockCopyInstruction(*Pointer, &JmpCode32, JUMP_CODE32_LENGTH);
    }
}

VOID
NTAPI
DetourBuildJumpCode64(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    JMPCODE JmpCode64 = { 0 };

    RtlCopyMemory(&JmpCode64, JUMP_CODE64, JUMP_CODE64_LENGTH);
    RtlCopyMemory(&JmpCode64.u1.JumpAddress, &Detour, sizeof(ULONG64));

    DetourLockCopyInstruction(*Pointer, &JmpCode64, JUMP_CODE64_LENGTH);
}

VOID
NTAPI
DetourBuildJumpCode(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    JMPCODE JmpCode = { 0 };

    RtlCopyMemory(&JmpCode, JUMP_CODE, JUMP_CODE_LENGTH);
    RtlCopyMemory(&JmpCode.u1.JumpAddress, &Detour, sizeof(ULONG_PTR));

    DetourLockCopyInstruction(*Pointer, &JmpCode, JUMP_CODE_LENGTH);
}

#ifdef _WIN64
PVOID
NTAPI
DetourCopySpecInst(
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
    }*OpCode;

    C_ASSERT(sizeof(*OpCode) == 1);

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
    }CopyOpCode;

    C_ASSERT(sizeof(CopyOpCode) == 13);

    Instruction = Src;

    Prefix = Instruction[0];
    Code = Instruction[1];
    OpCode = &Instruction[2];

    Instruction += 3;

    if (0 == OpCode->Mod) {
        GpReg = OpCode->Destination;

        if (5 == OpCode->Source) {
            RealAddress = __RVA_TO_VA(Instruction);

            if (NULL != Target) {
                *Target = NULL;
            }

            if (NULL != Extra) {
                *Extra += 6;
            }

            if (NULL != Dst &&
                NULL != DstPool) {
                if ((PCHAR)*DstPool - (PCHAR)Dst >= sizeof(CopyOpCode)) {
                    CopyOpCode.Prefix = Prefix;
                    CopyOpCode.Code.Inst = 0x17; // Reg <- Immediate
                    CopyOpCode.Code.GpReg = GpReg;

                    RtlCopyMemory(
                        &CopyOpCode.RealAddress,
                        &RealAddress,
                        sizeof(PVOID));

                    CopyOpCode.CopyPrefix = Prefix;
                    CopyOpCode.CopyInst = 0x8b; // Reg <- [Reg]
                    CopyOpCode.CopyMod.Destination = GpReg;
                    CopyOpCode.CopyMod.Source = GpReg;
                    CopyOpCode.CopyMod.Mod = 0;

                    RtlCopyMemory(
                        Dst,
                        &CopyOpCode,
                        sizeof(CopyOpCode));
                }
            }

            ReturnAddress = Instruction + sizeof(LONG);
        }
    }

    return ReturnAddress;
}
#endif // _WIN64

static
PVOID
NTAPI
DetourCreateHeader(
    __inout PVOID Address,
    __out_opt PCHAR * FunctionTable
)
{
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    PVOID Target = NULL;
    LONG Extra = 0;
    ULONG Length = 0;
    ULONG FunctionCount = 1;
    PCHAR PoolHeader = NULL;
    ULONG PoolLength = 0;

    if (NULL != FunctionTable) {
        *FunctionTable = NULL;
    }

    ControlPc = Address;

    while (Length < JUMP_CODE_LENGTH) {
        TargetPc = DetourCopyInstruction(
            NULL,
            NULL,
            ControlPc,
            &Target,
            &Extra);

        if (NULL != TargetPc) {
            if (7 == TargetPc - ControlPc) {
#ifdef _WIN64
                if ((0 == _CmpByte(ControlPc[0], 0x48) ||
                    0 == _CmpByte(ControlPc[0], 0x49)) &&
                    0 == _CmpByte(ControlPc[1], 0x8b)) {
                    TargetPc = DetourCopySpecInst(
                        NULL,
                        NULL,
                        ControlPc,
                        &Target,
                        &Extra);
                }
#endif // _WIN64
            }

#ifdef _WIN64
            if (NULL != Target) {
                FunctionCount++;
            }
#endif // _WIN64

            Length += TargetPc - ControlPc;
            PoolLength += TargetPc - ControlPc + Extra;

            if (Length >= JUMP_CODE_LENGTH) {
                ////////////////////////////////
                //
                // old code begin
                // ...
                // old code end
                //
                // length
                // old address
                //
                // copied instruction begin
                // ...
                // copied instruction end
                //
                // jmp table begin
                // first jmp old address
                // ...
                // jmp table end
                //
                ////////////////////////////////

                PoolHeader = ExAllocatePool(
                    NonPagedPool,
                    Length + sizeof(ULONG) + sizeof(ULONG_PTR) + PoolLength + JUMP_CODE_LENGTH * FunctionCount);

                if (NULL != PoolHeader) {
                    RtlZeroMemory(
                        PoolHeader,
                        PoolLength + JUMP_CODE_LENGTH * FunctionCount);

                    // save old code info and length on header
                    RtlCopyMemory(
                        PoolHeader,
                        Address,
                        Length);

                    RtlCopyMemory(
                        PoolHeader + Length,
                        &Length,
                        sizeof(ULONG));

                    RtlCopyMemory(
                        PoolHeader + Length + sizeof(ULONG),
                        &Address,
                        sizeof(ULONG_PTR));

                    PoolHeader += Length + sizeof(ULONG) + sizeof(ULONG_PTR);

                    if (NULL != FunctionTable) {
                        *FunctionTable = PoolHeader + PoolLength;
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

    return PoolHeader;
}

VOID
NTAPI
DetourAttach(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    ULONG Length = 0;
    LONG Extra = 0;
    PCHAR Target = NULL;
    PCHAR PoolHeader = NULL;
    ULONG PoolLength = 0;
    PCHAR Header = NULL;
    PCHAR FunctionTable = NULL;

#ifdef _WIN64
    ULONG FunctionIndex = 1;
    PCHAR FunctionEntry = NULL;
    ULONG Index = 0;
    PCHAR Instruction = NULL;
    ULONG InstructionLength = 0;
    PCHAR RealAddress = NULL;
#endif // _WIN64

    Header = *Pointer;

    PoolHeader = DetourCreateHeader(
        Header,
        &FunctionTable);

    if (NULL != PoolHeader) {
        ControlPc = Header;

        while (Length < JUMP_CODE_LENGTH) {
            TargetPc = DetourCopyInstruction(
                PoolHeader + PoolLength,
                &FunctionTable,
                ControlPc,
                &Target,
                &Extra);

            if (NULL != TargetPc) {
                if (7 == TargetPc - ControlPc) {
#ifdef _WIN64
                    if ((0 == _CmpByte(ControlPc[0], 0x48) ||
                        0 == _CmpByte(ControlPc[0], 0x49)) &&
                        0 == _CmpByte(ControlPc[1], 0x8b)) {
                        TargetPc = DetourCopySpecInst(
                            PoolHeader + PoolLength,
                            &FunctionTable,
                            ControlPc,
                            &Target,
                            &Extra);
                    }
#endif // _WIN64
                }

#ifdef _WIN64
                if (NULL != Target) {
                    Instruction = PoolHeader + PoolLength;
                    InstructionLength = TargetPc - ControlPc + Extra;

                    FunctionEntry =
                        FunctionTable + JUMP_CODE_LENGTH * FunctionIndex;

                    DetourBuildJumpCode(&FunctionEntry, Target);

                    for (Index = 0;
                        Index <= InstructionLength - sizeof(LONG);
                        Index++) {
                        RealAddress = __RVA_TO_VA(Instruction + Index);

                        if (*(PLONG)&Target == *(PLONG)&RealAddress) {
                            *(PLONG)(Instruction + Index) =
                                FunctionEntry - ((Instruction + Index) + sizeof(LONG));
                        }
                    }

                    FunctionIndex++;
                }
#endif // _WIN64

                Length += TargetPc - ControlPc;
                PoolLength += TargetPc - ControlPc + Extra;

                if (Length >= JUMP_CODE_LENGTH) {
                    DetourBuildJumpCode(&FunctionTable, TargetPc);
                    DetourBuildJumpCode(&Header, Detour);

                    *Pointer = PoolHeader;

                    break;
                }

                ControlPc = TargetPc;
            }
            else {
                break;
            }
        }
    }
}

VOID
NTAPI
DetourDetach(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    PCHAR PoolHeader = NULL;
    PCHAR Header = NULL;
    ULONG Length = 0;
    PJMPCODE JmpCode = NULL;

    Header = *(PCHAR*)((PCHAR)*Pointer - sizeof(ULONG_PTR));

    if (FIELD_OFFSET(JMPCODE, u1) == RtlCompareMemory(
        Header,
        JUMP_CODE,
        FIELD_OFFSET(JMPCODE, u1))) {
        JmpCode = Header;

        if ((ULONG_PTR)Detour ==
            (ULONG_PTR)JmpCode->u1.JumpAddress) {
            RtlCopyMemory(
                &Length,
                (PCHAR)*Pointer - sizeof(ULONG_PTR) - sizeof(ULONG),
                sizeof(ULONG));

            PoolHeader = (PCHAR)*Pointer - sizeof(ULONG_PTR) - sizeof(ULONG) - Length;

            DetourLockCopyInstruction(Header, PoolHeader, Length);

            ExFreePool(PoolHeader);
        }
    }
}

VOID
NTAPI
DetourAttachHotPatch(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    CHAR HotPatch[] = { 0xe9, 0xcc, 0xcc, 0xcc, 0xcc, 0xeb, 0xf9 };

    *Pointer = (PCHAR)*Pointer + HOT_PATCH_HEADER_LENGTH;

    *(PLONG)&HotPatch[1] =
        PtrToLong(Detour) -
        (PtrToLong((PCHAR)*Pointer - HOT_PATCH_HEADER_LENGTH));

    DetourLockCopyInstruction(
        (PCHAR)*Pointer - sizeof(HotPatch),
        HotPatch,
        sizeof(HotPatch) - HOT_PATCH_HEADER_LENGTH);

    DetourLockCopyInstruction(
        (PCHAR)*Pointer - HOT_PATCH_HEADER_LENGTH,
        (PCHAR)HotPatch + (sizeof(HotPatch) - HOT_PATCH_HEADER_LENGTH),
        HOT_PATCH_HEADER_LENGTH);
}

VOID
NTAPI
DetourDetachHotPatch(
    __inout PVOID * Pointer,
    __in PVOID Detour
)
{
    CHAR Fill[] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc };

    DetourLockCopyInstruction(
        (PCHAR)*Pointer - HOT_PATCH_HEADER_LENGTH,
        HOT_PATCH_HEADER,
        HOT_PATCH_HEADER_LENGTH);

    *Pointer = (PCHAR)*Pointer - HOT_PATCH_HEADER_LENGTH;

    DetourLockCopyInstruction(
        (PCHAR)*Pointer - sizeof(Fill),
        Fill,
        sizeof(Fill));
}
