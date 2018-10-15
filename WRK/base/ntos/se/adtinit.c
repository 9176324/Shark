/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    adtinit.c

Abstract:

    Auditing - Initialization Routines

--*/

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepAdtValidateAuditBounds)
#pragma alloc_text(PAGE,SepAdtInitializeBounds)
#pragma alloc_text(INIT,SepAdtInitializeCrashOnFail)
#pragma alloc_text(INIT,SepAdtInitializePrivilegeAuditing)
#pragma alloc_text(INIT,SepAdtInitializeAuditingOptions)
#endif


BOOLEAN
SepAdtValidateAuditBounds(
    ULONG Upper,
    ULONG Lower
    )

/*++

Routine Description:

    Examines the audit queue high and low water mark values and performs
    a general sanity check on them.

Arguments:

    Upper - High water mark.

    Lower - Low water mark.

Return Value:

    TRUE - values are acceptable.

    FALSE - values are unacceptable.


--*/

{
    PAGED_CODE();

    if ( Lower >= Upper ) {
        return( FALSE );
    }

    if ( Lower < 16 ) {
        return( FALSE );
    }

    if ( (Upper - Lower) < 16 ) {
        return( FALSE );
    }

    return( TRUE );
}


VOID
SepAdtInitializeBounds(
    VOID
    )

/*++

Routine Description:

    Queries the registry for the high and low water mark values for the
    audit log.  If they are not found or are unacceptable, returns without
    modifying the current values, which are statically initialized.

Arguments:

    None.

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    PSEP_AUDIT_BOUNDS AuditBounds;
    UCHAR Buffer[8];
    

    PAGED_CODE();

    Status = SepRegQueryHelper(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 L"Bounds",
                 REG_BINARY,
                 8,             // 8 bytes
                 Buffer,
                 NULL
                 );

    if (!NT_SUCCESS( Status )) {

        //
        // Didn't work, take the defaults
        //

        return;
    }

    AuditBounds = (PSEP_AUDIT_BOUNDS) Buffer;

    //
    // Sanity check what we got back
    //

    if(SepAdtValidateAuditBounds( AuditBounds->UpperBound,
                                  AuditBounds->LowerBound ))
    {
        //
        // Take what we got from the registry.
        //

        SepAdtMaxListLength = AuditBounds->UpperBound;
        SepAdtMinListLength = AuditBounds->LowerBound;
    }
}



NTSTATUS
SepAdtInitializeCrashOnFail(
    VOID
    )

/*++

Routine Description:

    Reads the registry to see if the user has told us to crash if an audit fails.

Arguments:

    None.

Return Value:

    STATUS_SUCCESS

--*/

{
    NTSTATUS Status;
    ULONG    CrashOnAuditFail = 0;

    PAGED_CODE();

    SepCrashOnAuditFail = FALSE;

    //
    // Check the value of the CrashOnAudit flag in the registry.
    //

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 CRASH_ON_AUDIT_FAIL_VALUE,
                 &CrashOnAuditFail
                 );

    //
    // If the key isn't there, don't turn on CrashOnFail.
    //

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        return( STATUS_SUCCESS );
    }


    if (NT_SUCCESS( Status )) {

        if ( CrashOnAuditFail == LSAP_CRASH_ON_AUDIT_FAIL) {
            SepCrashOnAuditFail = TRUE;
        }
    }

    return( STATUS_SUCCESS );
}


BOOLEAN
SepAdtInitializePrivilegeAuditing(
    VOID
    )

/*++

Routine Description:

    Checks to see if there is an entry in the registry telling us to do full privilege auditing
    (which currently means audit everything we normall audit, plus backup and restore privileges).

Arguments:

    None

Return Value:

    BOOLEAN - TRUE if Auditing has been initialized correctly, else FALSE.

--*/

{
    HANDLE KeyHandle;
    NTSTATUS Status;
    NTSTATUS TmpStatus;
    OBJECT_ATTRIBUTES Obja;
    ULONG ResultLength;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    CHAR KeyInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(BOOLEAN)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    BOOLEAN Verbose;

    PAGED_CODE();

    //
    // Query the registry to set up the privilege auditing filter.
    //

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa");

    InitializeObjectAttributes( &Obja,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

    Status = NtOpenKey(
                 &KeyHandle,
                 KEY_QUERY_VALUE | KEY_SET_VALUE,
                 &Obja
                 );


    if (!NT_SUCCESS( Status )) {

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

            return ( SepInitializePrivilegeFilter( FALSE ));

        } else {

            return( FALSE );
        }
    }

    RtlInitUnicodeString( &ValueName, FULL_PRIVILEGE_AUDITING );

    Status = NtQueryValueKey(
                 KeyHandle,
                 &ValueName,
                 KeyValuePartialInformation,
                 KeyInfo,
                 sizeof(KeyInfo),
                 &ResultLength
                 );

    TmpStatus = NtClose(KeyHandle);
    ASSERT(NT_SUCCESS(TmpStatus));

    if (!NT_SUCCESS( Status )) {

        Verbose = FALSE;

    } else {

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;
        Verbose = (BOOLEAN) *(pKeyInfo->Data);
    }

    return ( SepInitializePrivilegeFilter( Verbose ));
}


VOID
SepAdtInitializeAuditingOptions(
    VOID
    )

/*++

Routine Description:

    Initialize options that control auditing.
    (please refer to note in adtp.h near the def. of SEP_AUDIT_OPTIONS)

Arguments:

    None

Return Value:

    None

--*/

{
    NTSTATUS Status;
    ULONG OptionValue = 0;

    PAGED_CODE();

    //
    // initialize the default value
    //

    SepAuditOptions.DoNotAuditCloseObjectEvents = FALSE;

    //
    // if the value is present and set to 1, set the global
    // auditing option accordingly
    //

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa\\AuditingOptions",
                 L"DoNotAuditCloseObjectEvents",
                 &OptionValue
                 );

    if (NT_SUCCESS(Status) && OptionValue)
    {
        SepAuditOptions.DoNotAuditCloseObjectEvents = TRUE;
    }

    return;
}

