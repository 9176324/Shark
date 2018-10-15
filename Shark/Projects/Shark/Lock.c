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

#include "Lock.h"

PMDL_LOCKER
NTAPI
ProbeAndLockPages(
    __in PVOID VirtualAddress,
    __in ULONG Length,
    __in KPROCESSOR_MODE AccessMode,
    __in LOCK_OPERATION Operation
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PMDL_LOCKER MdlLocker = NULL;

    MdlLocker = ExAllocatePool(
        NonPagedPool,
        sizeof(MDL_LOCKER));

    if (NULL != MdlLocker) {
        RtlZeroMemory(MdlLocker, sizeof(MDL_LOCKER));

        MdlLocker->MemoryDescriptorList = IoAllocateMdl(
            VirtualAddress,
            Length,
            FALSE,
            TRUE,
            NULL);

        if (NULL != MdlLocker->MemoryDescriptorList) {
            __try {
                MmProbeAndLockPages(
                    MdlLocker->MemoryDescriptorList,
                    AccessMode,
                    Operation);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                IoFreeMdl(MdlLocker->MemoryDescriptorList);

                MdlLocker->MemoryDescriptorList = NULL;

                Status = GetExceptionCode();

                RTL_SOFT_ASSERT(NT_SUCCESS(Status));
            }

            if (NULL != MdlLocker->MemoryDescriptorList) {
                MdlLocker->MappedAddress = MmGetSystemAddressForMdlSafe(
                    MdlLocker->MemoryDescriptorList,
                    NormalPagePriority);

                if (NULL == MdlLocker->MappedAddress) {
                    IoFreeMdl(MdlLocker->MemoryDescriptorList);
                    ExFreePool(MdlLocker);

                    MdlLocker = NULL;
                    Status = STATUS_INSUFFICIENT_RESOURCES;

                    RTL_SOFT_ASSERT(NT_SUCCESS(Status));
                }
            }
        }
    }
    else {
        Status = STATUS_NO_MEMORY;

        RTL_SOFT_ASSERT(NT_SUCCESS(Status));
    }

    return MdlLocker;
}

VOID
NTAPI
UnlockPages(
    __inout PMDL_LOCKER MdlLocker
)
{
    ASSERT(NULL != MdlLocker);

    MmUnlockPages(MdlLocker->MemoryDescriptorList);
    IoFreeMdl(MdlLocker->MemoryDescriptorList);
}
