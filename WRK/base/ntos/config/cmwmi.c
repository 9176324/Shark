/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmwmi.c

Abstract:

    This module contains support for tracing registry system calls 

--*/

#include    "cmp.h"
#pragma hdrstop
#include    <evntrace.h>

VOID
CmpWmiDumpKcbTable(
    VOID
);

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
PCM_TRACE_NOTIFY_ROUTINE CmpTraceRoutine = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmSetTraceNotifyRoutine)
#pragma alloc_text(PAGE,CmpWmiDumpKcbTable)
#pragma alloc_text(PAGE,CmpWmiDumpKcb)
#endif


NTSTATUS
CmSetTraceNotifyRoutine(
    __in_opt PCM_TRACE_NOTIFY_ROUTINE NotifyRoutine,
    __in BOOLEAN Remove
    )
{
    if(Remove) {
        CmpTraceRoutine = NULL;
    } else {
        CmpTraceRoutine = NotifyRoutine;

        //
        // dump active kcbs to WMI
        //
        CmpWmiDumpKcbTable();
    }
    return STATUS_SUCCESS;
}

VOID
CmpWmiDumpKcbTable(
    VOID
)
/*++

Routine Description:

    Sends all kcbs addresses and names from the HashTable to WMI.

Arguments:

    none

Return Value:
    
    none

--*/
{
    ULONG                       i;
    PCM_KEY_HASH                Current;
    PCM_KEY_CONTROL_BLOCK       kcb;
    PUNICODE_STRING             KeyName;
    PCM_TRACE_NOTIFY_ROUTINE    TraceRoutine = CmpTraceRoutine;

    CM_PAGED_CODE();

    if( TraceRoutine == NULL ) {
        return;
    }

    CmpLockRegistry();

    for (i=0; i<CmpHashTableSize; i++) {
        CmpLockHashEntryByIndexShared(i);
        Current = CmpCacheTable[i].Entry;
        while (Current) {
            kcb = CONTAINING_RECORD(Current, CM_KEY_CONTROL_BLOCK, KeyHash);
            KeyName = CmpConstructName(kcb);
            if(KeyName != NULL) {
                (*TraceRoutine)(STATUS_SUCCESS,
                                kcb, 
                                0, 
                                0,
                                KeyName,
                                EVENT_TRACE_TYPE_REGKCBDMP);
	     
                ExFreePoolWithTag(KeyName, CM_NAME_TAG | PROTECTED_POOL);
            }
            Current = Current->NextHash;
        }
        CmpUnlockHashEntryByIndex(i);
    }

    CmpUnlockRegistry();
}

VOID
CmpWmiDumpKcb(
    PCM_KEY_CONTROL_BLOCK       kcb
)
/*++

Routine Description:

    dumps a single kcb

Arguments:

    none

Return Value:
    
    none

--*/
{
    PCM_TRACE_NOTIFY_ROUTINE    TraceRoutine = CmpTraceRoutine;
    PUNICODE_STRING             KeyName;

    CM_PAGED_CODE();

    if( TraceRoutine == NULL ) {
        return;
    }

    KeyName = CmpConstructName(kcb);
    if(KeyName != NULL) {
        (*TraceRoutine)(STATUS_SUCCESS,
                        kcb, 
                        0, 
                        0,
                        KeyName,
                        EVENT_TRACE_TYPE_REGKCBDMP);
 
        ExFreePoolWithTag(KeyName, CM_NAME_TAG | PROTECTED_POOL);
    }
}


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

