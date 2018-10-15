/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    adtutil.c - Security Auditing - Utility Routines

Abstract:

    This Module contains miscellaneous utility routines private to the
    Security Auditing Component.

--*/

#include "pch.h"

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepRegQueryDwordValue)
#endif



NTSTATUS
SepRegQueryHelper(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    IN  ULONG  ValueType,
    IN  ULONG  ValueLength,
    OUT PVOID  ValueBuffer,
    OUT PULONG LengthRequired
    )
/*++

Routine Description:

    Open regkey KeyName, read the value specified by ValueName
    and return the value.

Arguments:

    KeyName        - name of key to open

    ValueName      - name of value to read

    ValueType      - type of value to read (REG_DWORD etc.)

    ValueLength    - size in bytes of the value to read

    ValueBuffer    - pointer to returned value

    LengthRequired - if the passed buffer is not sufficient to hold
                     the value, this param will return the actual size
                     in bytes required.

Return Value:

    NTSTATUS - Standard Nt Result Code

Notes:

--*/
{
    UNICODE_STRING usKey, usValue;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };

    //
    // we will read-in data up to 64 bytes in stack buffer
    //

    CHAR KeyInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    HANDLE hKey = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS CloseStatus;
    ULONG ResultLength;
        
    PAGED_CODE();

    RtlInitUnicodeString( &usKey, KeyName );
    
    InitializeObjectAttributes(
        &ObjectAttributes,
        &usKey,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL
        );

    Status = ZwOpenKey(
                 &hKey,
                 KEY_QUERY_VALUE | OBJ_KERNEL_HANDLE,
                 &ObjectAttributes
                 );

    if (NT_SUCCESS( Status ))
    {
        RtlInitUnicodeString( &usValue, ValueName );

        Status = ZwQueryValueKey(
                     hKey,
                     &usValue,
                     KeyValuePartialInformation,
                     KeyInfo,
                     sizeof(KeyInfo),
                     &ResultLength
                     );
        
        if (NT_SUCCESS( Status ))
        {
            pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;

            if (( pKeyInfo->Type == ValueType) &&
                ( pKeyInfo->DataLength == ValueLength ))
            {
                switch (ValueType)
                {
                    case REG_DWORD:
                        *((PULONG)ValueBuffer) = *((PULONG) (pKeyInfo->Data));
                        break;

                    case REG_BINARY:
                        RtlCopyMemory( ValueBuffer, pKeyInfo->Data, ValueLength );
                        break;
                        
                    default:
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                }
            }
            else
            {
                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }
        }

        CloseStatus = ZwClose(hKey);
        
        ASSERT( NT_SUCCESS( CloseStatus ));
    }

    return Status;
}


NTSTATUS
SepRegQueryDwordValue(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    OUT PULONG Value
    )
/*++

Routine Description:

    Open regkey KeyName, read a REG_DWORD value specified by ValueName
    and return the value.

Arguments:

    KeyName - name of key to open

    ValueName - name of value to read

    Value - pointer to returned value

Return Value:

    NTSTATUS - Standard Nt Result Code

Notes:

--*/
{
    
    return SepRegQueryHelper(
               KeyName,
               ValueName,
               REG_DWORD,
               sizeof(ULONG),
               Value,
               NULL
               );
}

