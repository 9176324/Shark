/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    assert.c

Abstract:

    This module implements the RtlAssert function that is referenced by the
    debugging version of the ASSERT macro defined in NTDEF.H

--*/

#include <nt.h>
#include <ntrtl.h>
#include <zwapi.h>

//
// RtlAssert is not called unless the caller is compiled with DBG non-zero
// therefore it does no harm to always have this routine in the kernel.
// This allows checked drivers to be thrown on the system and have their
// asserts be meaningful.
//

#define RTL_ASSERT_ALWAYS_ENABLED 1

#ifdef _X86_
#pragma optimize("y", off)      // RtlCaptureContext needs EBP to be correct
#endif

#undef RtlAssert

typedef CONST CHAR * PCSTR;

#if defined(_M_AMD64)

DECLSPEC_NOINLINE

#endif

NTSYSAPI
VOID
NTAPI
RtlAssert(
    __in PVOID VoidFailedAssertion,
    __in PVOID VoidFileName,
    __in ULONG LineNumber,
    __in_opt PSTR MutableMessage
    )
{
#if DBG || RTL_ASSERT_ALWAYS_ENABLED
    char Response[ 2 ];

    CONST PCSTR FailedAssertion = (PCSTR)VoidFailedAssertion;
    CONST PCSTR FileName = (PCSTR)VoidFileName;
    CONST PCSTR Message  = (PCSTR)MutableMessage;

    CONTEXT Context;

    RtlCaptureContext( &Context );

    while (TRUE) {
        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );
        DbgPrompt( "Break repeatedly, break Once, Ignore, terminate Process, or terminate Thread (boipt)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
            case 'O':
            case 'o':
                DbgPrint( "Execute '.cxr %p' to dump context\n", &Context);
                DbgBreakPoint();
                if (Response[0] == 'o' || Response[0] == 'O')
                    return;
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                ZwTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;

            case 'T':
            case 't':
                ZwTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    ZwTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
#endif
}

#ifdef _X86_
#pragma optimize("", on)
#endif

