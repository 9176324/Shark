/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntrtlamd64.h

Abstract:

    AMD64 specific parts of ntrtlp.h.

--*/

#ifndef _NTRTLAMD64_
#define _NTRTLAMD64_

//
// Define exception routine function prototypes.
//

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONG64 EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    );

EXCEPTION_DISPOSITION
RtlpExecuteHandlerForUnwind (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN ULONG_PTR EstablisherFrame,
    IN OUT PCONTEXT ContextRecord,
    IN OUT PDISPATCHER_CONTEXT DispatcherContext
    );

BOOLEAN
RtlpIsFrameInBounds (
    IN OUT PULONG64 LowLimit,
    IN ULONG64 StackFrame,
    IN OUT PULONG64 HighLimit
    );

#endif // _NTRTLAMD64_

