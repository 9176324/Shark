/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    wowinfo.c

Abstract:

    This module implements the routines to returns processor-specific information
    about the x86 emulation capability.

--*/

#include "exp.h"

NTSTATUS
ExpGetSystemEmulationProcessorInformation (
    OUT PSYSTEM_PROCESSOR_INFORMATION ProcessorInformation
    )

/*++

Routine Description:

    Retrieves the processor information of the emulation hardware.

Arguments:

    ProcessorInformation - Pointer to receive the processor's emulation information.

Return Value:

    NTSTATUS

--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    //
    //  Return INTEL processor architecture for compatibility.
    //  Use the native values for processor revision and level.
    //

    try {

        ProcessorInformation->ProcessorArchitecture = PROCESSOR_ARCHITECTURE_INTEL;
        ProcessorInformation->ProcessorLevel = KeProcessorLevel;
        ProcessorInformation->ProcessorRevision = KeProcessorRevision;
        ProcessorInformation->Reserved = 0;
        ProcessorInformation->ProcessorFeatureBits = KeFeatureBits;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        
        NtStatus = GetExceptionCode ();
    }

    return NtStatus;
}

