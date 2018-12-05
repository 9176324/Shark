/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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

#include "Jump.h"

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

NTSTATUS
NTAPI
BuildJumpCode32(
    __in PVOID Function,
    __inout PVOID * NewAddress
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PJMPCODE JmpCode32 = NULL;
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    PVOID VirtualAddress = NULL;

    JmpCode32 = ExAllocatePool(
        NonPagedPool,
        sizeof(JMPCODE32));

    if (NULL != JmpCode32) {
        RtlFillMemory(
            JmpCode32,
            sizeof(JMPCODE32),
            0xcc);

        RtlCopyMemory(
            JmpCode32,
            JUMP_CODE,
            sizeof(JUMP_CODE) - 1);

        RtlCopyMemory(
            &JmpCode32->u1,
            &Function,
            sizeof(ULONG_PTR));

        PhysicalAddress = MmGetPhysicalAddress(*NewAddress);

        VirtualAddress = MmMapIoSpace(
            PhysicalAddress,
            FIELD_OFFSET(JMPCODE32, Filler),
            MmNonCached);

        if (NULL != VirtualAddress) {
            InterlockedExchange(VirtualAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)VirtualAddress + 2,
                (PCHAR)JmpCode32 + 2,
                FIELD_OFFSET(JMPCODE32, Filler) - 2);

            InterlockedExchange(VirtualAddress, *(PULONG)JmpCode32);

            MmUnmapIoSpace(
                VirtualAddress,
                FIELD_OFFSET(JMPCODE32, Filler));
        }
        else {
            Status = STATUS_INVALID_ADDRESS;
        }

        ExFreePool(JmpCode32);
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}

NTSTATUS
NTAPI
BuildJumpCode64(
    __in PVOID Function,
    __inout PVOID * NewAddress
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PJMPCODE JmpCode64 = NULL;
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    PVOID VirtualAddress = NULL;

    JmpCode64 = ExAllocatePool(
        NonPagedPool,
        sizeof(JMPCODE64));

    if (NULL != JmpCode64) {
        RtlFillMemory(
            JmpCode64,
            sizeof(JMPCODE64),
            0xcc);

        RtlCopyMemory(
            JmpCode64,
            JUMP_CODE,
            sizeof(JUMP_CODE) - 1);

        RtlCopyMemory(
            &JmpCode64->u1,
            &Function,
            sizeof(ULONG_PTR));

        PhysicalAddress = MmGetPhysicalAddress(*NewAddress);

        VirtualAddress = MmMapIoSpace(
            PhysicalAddress,
            FIELD_OFFSET(JMPCODE64, Filler),
            MmNonCached);

        if (NULL != VirtualAddress) {
            InterlockedExchange(VirtualAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)VirtualAddress + 2,
                (PCHAR)JmpCode64 + 2,
                FIELD_OFFSET(JMPCODE64, Filler) - 2);

            InterlockedExchange(VirtualAddress, *(PULONG)JmpCode64);

            MmUnmapIoSpace(
                VirtualAddress,
                FIELD_OFFSET(JMPCODE64, Filler));
        }
        else {
            Status = STATUS_INVALID_ADDRESS;
        }

        ExFreePool(JmpCode64);
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}

NTSTATUS
NTAPI
BuildJumpCode(
    __in PVOID Function,
    __inout PVOID * NewAddress
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PJMPCODE JmpCode = NULL;
    PHYSICAL_ADDRESS PhysicalAddress = { 0 };
    PVOID VirtualAddress = NULL;

    JmpCode = ExAllocatePool(
        NonPagedPool,
        sizeof(JMPCODE));

    if (NULL != JmpCode) {
        RtlFillMemory(
            JmpCode,
            sizeof(JMPCODE),
            0xcc);

        RtlCopyMemory(
            JmpCode,
            JUMP_CODE,
            sizeof(JUMP_CODE) - 1);

        RtlCopyMemory(
            &JmpCode->u1,
            &Function,
            sizeof(ULONG_PTR));

        PhysicalAddress = MmGetPhysicalAddress(*NewAddress);

        VirtualAddress = MmMapIoSpace(
            PhysicalAddress,
            FIELD_OFFSET(JMPCODE, Filler),
            MmNonCached);

        if (NULL != VirtualAddress) {
            InterlockedExchange(VirtualAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)VirtualAddress + 2,
                (PCHAR)JmpCode + 2,
                FIELD_OFFSET(JMPCODE, Filler) - 2);

            InterlockedExchange(VirtualAddress, *(PULONG)JmpCode);

            MmUnmapIoSpace(
                VirtualAddress,
                FIELD_OFFSET(JMPCODE, Filler));
        }
        else {
            Status = STATUS_INVALID_ADDRESS;
        }

        ExFreePool(JmpCode);
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}
