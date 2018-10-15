/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntrtl386.h

Abstract:

    i386 specific parts of ntrtlp.h

--*/

//
// Exception handling procedure prototypes.
//
VOID
RtlpCaptureContext (
    OUT PCONTEXT ContextRecord
    );

VOID
RtlpUnlinkHandler (
    PEXCEPTION_REGISTRATION_RECORD UnlinkPointer
    );

PEXCEPTION_REGISTRATION_RECORD
RtlpGetRegistrationHead (
    VOID
    );

//
//  Record dump procedures.
//

VOID
RtlpContextDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );

VOID
RtlpExceptionReportDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );

VOID
RtlpExceptionRegistrationDump(
    IN PVOID Object,
    IN ULONG Control OPTIONAL
    );

