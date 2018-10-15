/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    logging.c

Abstract:

    This module contains routines for trace logging.

--*/

#include "perfp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, PerfInfoReserveBytes)
#pragma alloc_text(PAGEWMI, PerfInfoLogBytes)
#endif //ALLOC_PRAGMA


NTSTATUS
PerfInfoReserveBytes(
    PPERFINFO_HOOK_HANDLE Hook,
    USHORT HookId,
    ULONG BytesToReserve
    )
/*++

Routine Description:

    Reserves memory for the hook via WMI and initializes the header.

Arguments:

    Hook - pointer to hook handle (used for reference decrement)
    HookId - Id for the hook
    BytesToLog - size of data in bytes

Return Value:

    STATUS_SUCCESS on success
    STATUS_UNSUCCESSFUL if the buffer memory couldn't be allocated.
--*/
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PPERFINFO_TRACE_HEADER PPerfTraceHeader = NULL;
    PWMI_BUFFER_HEADER PWmiBufferHeader = NULL;

    PERF_ASSERT((BytesToReserve + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data)) <= MAXUSHORT);
    PERF_ASSERT(Hook != NULL);

    PPerfTraceHeader = WmiReserveWithPerfHeader(BytesToReserve, &PWmiBufferHeader);

    if (PPerfTraceHeader != NULL) {
        PPerfTraceHeader->Packet.HookId = HookId;
        Hook->PerfTraceHeader = PPerfTraceHeader;
        Hook->WmiBufferHeader = PWmiBufferHeader;

        Status = STATUS_SUCCESS;
    } else {
        *Hook = PERF_NULL_HOOK_HANDLE;
    }

    return Status;
}


NTSTATUS
PerfInfoLogBytes(
    USHORT HookId,
    PVOID Data,
    ULONG BytesToLog
    )
/*++

Routine Description:

    Reserves memory for the hook, copies the data, and unref's the hook entry.

Arguments:

    HookId - Id for the hook
    Data - pointer to the data to be logged
    BytesToLog - size of data in bytes

Return Value:

    STATUS_SUCCESS on success
--*/
{
    PERFINFO_HOOK_HANDLE Hook;
    NTSTATUS Status;

    Status = PerfInfoReserveBytes(&Hook, HookId, BytesToLog);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlCopyMemory(PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PPERF_BYTE), Data, BytesToLog);
    PERF_FINISH_HOOK(Hook);

    return STATUS_SUCCESS;
}

