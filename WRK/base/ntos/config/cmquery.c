/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmquery.c

Abstract:

    This module contains the object name query method for the registry.

--*/

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpQueryKeyName)
#endif

NTSTATUS
CmpQueryKeyName(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    )
/*++

Routine Description:

    This routine interfaces to the NT Object Manager.  It is invoked when
    the object system wishes to discover the name of an object that
    belongs to the registry.

Arguments:

    Object - pointer to a Key, thus -> KEY_BODY.

    HasObjectName - indicates whether the object manager knows about a name
        for this object

    ObjectNameInfo - place where we report the name

    Length - maximum length they can deal with

    ReturnLength - supplies variable to receive actual length

    Mode - Processor mode of the caller

Return Value:

    STATUS_SUCCESS

    STATUS_INFO_LENGTH_MISMATCH

--*/

{
    PUNICODE_STRING         Name = NULL;
    PWCHAR                  t;
    PWCHAR                  s;
    ULONG                   l;
    NTSTATUS                status;
    PCM_KEY_CONTROL_BLOCK   kcb;
    BOOLEAN                 UnlockKcb = TRUE;

    UNREFERENCED_PARAMETER(HasObjectName);
    UNREFERENCED_PARAMETER(Mode);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_PARSE,"CmpQueryKeyName:\n"));

    BEGIN_LOCK_CHECKPOINT;
    CmpLockRegistry();
    //
    // sanitize the kcb, for cases where we get a callback for se audit
    //
    kcb = (PCM_KEY_CONTROL_BLOCK)(((PCM_KEY_BODY)Object)->KeyControlBlock);
    if( (ULONG_PTR)kcb & 1 ) {
        kcb = (PCM_KEY_CONTROL_BLOCK)((ULONG_PTR)kcb ^ 1);
        ASSERT_KCB_LOCKED(kcb);
        UnlockKcb = FALSE;
    } else {
        CmpLockKCBShared(kcb);
    }

    if ( kcb->Delete) {
        if( UnlockKcb ) {
            CmpUnlockKCB(kcb);
        }
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }
    Name = CmpConstructName(kcb);

    if( UnlockKcb ) {
        CmpUnlockKCB(kcb);
    }

    //
    // don't need the lock anymore
    //
    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

    if (Name == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

    if (Length <= sizeof(OBJECT_NAME_INFORMATION)) {
        *ReturnLength = Name->Length + sizeof(WCHAR) + sizeof(OBJECT_NAME_INFORMATION);
        ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
        return STATUS_INFO_LENGTH_MISMATCH;  // they can't even handle null
    }

    t = (PWCHAR)(ObjectNameInfo + 1);
    s = Name->Buffer;
    l = Name->Length;
    l += sizeof(WCHAR);     // account for null

    *ReturnLength = l + sizeof(OBJECT_NAME_INFORMATION);
    if (l > Length - sizeof(OBJECT_NAME_INFORMATION)) {
        l = Length - sizeof(OBJECT_NAME_INFORMATION);
        status = STATUS_INFO_LENGTH_MISMATCH;
        if( l < sizeof(WCHAR) ) {
            ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
            return status;  // they can't even handle null
        } 
    } else {
        status = STATUS_SUCCESS;
    }

    l -= sizeof(WCHAR);

    //
    // The ObjectNameInfo buffer is a usermode buffer, so make sure we have an
    // exception handler in case a malicious app changes the protection out from
    // under us.
    //
    // Note the object manager is responsible for probing the buffer and ensuring
    // that a top-level exception handler returns the correct error code. We just
    // need to make sure we drop our lock.
    //
    try {
        RtlCopyMemory(t, s, l);
        t[l/sizeof(WCHAR)] = UNICODE_NULL;
        ObjectNameInfo->Name.Length = (USHORT)l;
        ObjectNameInfo->Name.MaximumLength = ObjectNameInfo->Name.Length;
        ObjectNameInfo->Name.Buffer = t;
    } finally {
        ExFreePoolWithTag(Name, CM_NAME_TAG | PROTECTED_POOL);
    }
    return status;
}
