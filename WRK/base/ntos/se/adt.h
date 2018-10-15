/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    adt.h

Abstract:

    Auditing - Defines, Function Prototypes and Macro Functions.
               These are public to the Security Component only.

--*/

#include <ntlsa.h>


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Auditing Routines visible to rest of Security Component outside Auditing //
// subcomponent.                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


BOOLEAN
SepAdtInitializePhase0();

BOOLEAN
SepAdtInitializePhase1();

VOID
SepAdtLogAuditRecord(
    IN PSE_ADT_PARAMETER_ARRAY AuditParameters
    );

NTSTATUS
SepAdtCopyToLsaSharedMemory(
    IN HANDLE LsaProcessHandle,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PVOID *LsaBufferAddress
    );

