/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    adtutil.h - Security Auditing - Utility Routines

Abstract:

    This Module contains miscellaneous utility routines private to the
    Security Auditing Component.

--*/

NTSTATUS
SepRegQueryDwordValue(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    OUT PULONG Value
    );

NTSTATUS
SepRegQueryHelper(
    IN  PCWSTR KeyName,
    IN  PCWSTR ValueName,
    IN  ULONG  ValueType,
    IN  ULONG  ValueLength,
    OUT PVOID  ValueBuffer,
    OUT PULONG LengthRequired
    );

