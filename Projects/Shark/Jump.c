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

#include <Defs.h>

#include "Jump.h"

#include "Lock.h"

ULONG
NTAPI
DetourGetInstructionLength(
    __in PVOID ControlPc
)
{
    PCHAR TargetPc = NULL;
    LONG Extra = 0;
    ULONG Length = 0;

    __try {
        TargetPc = DetourCopyInstruction(
            NULL,
            NULL,
            ControlPc,
            NULL,
            &Extra);

        if (NULL != TargetPc) {
            Length += (ULONG)(TargetPc - ControlPc);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Length = 0;
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
    PMDL_LOCKER MdlLocker = NULL;

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

        MdlLocker = ProbeAndLockPages(
            *NewAddress,
            FIELD_OFFSET(JMPCODE32, Filler),
            KernelMode,
            IoWriteAccess);

        if (NULL != MdlLocker) {
            InterlockedExchange(MdlLocker->MappedAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)MdlLocker->MappedAddress + 2,
                (PCHAR)JmpCode32 + 2,
                FIELD_OFFSET(JMPCODE32, Filler) - 2);

            InterlockedExchange(MdlLocker->MappedAddress, *(PULONG)JmpCode32);

            UnlockPages(MdlLocker);
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
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
    PMDL_LOCKER MdlLocker = NULL;

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

        MdlLocker = ProbeAndLockPages(
            *NewAddress,
            FIELD_OFFSET(JMPCODE64, Filler),
            KernelMode,
            IoWriteAccess);

        if (NULL != MdlLocker) {
            InterlockedExchange(MdlLocker->MappedAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)MdlLocker->MappedAddress + 2,
                (PCHAR)JmpCode64 + 2,
                FIELD_OFFSET(JMPCODE64, Filler) - 2);

            InterlockedExchange(MdlLocker->MappedAddress, *(PULONG)JmpCode64);

            UnlockPages(MdlLocker);
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
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
    PMDL_LOCKER MdlLocker = NULL;

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

        MdlLocker = ProbeAndLockPages(
            *NewAddress,
            FIELD_OFFSET(JMPCODE, Filler),
            KernelMode,
            IoWriteAccess);

        if (NULL != MdlLocker) {
            InterlockedExchange(MdlLocker->MappedAddress, JUMP_SELF);

            RtlCopyMemory(
                (PCHAR)MdlLocker->MappedAddress + 2,
                (PCHAR)JmpCode + 2,
                FIELD_OFFSET(JMPCODE, Filler) - 2);

            InterlockedExchange(MdlLocker->MappedAddress, *(PULONG)JmpCode);

            UnlockPages(MdlLocker);
        }
        else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

        ExFreePool(JmpCode);
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}
