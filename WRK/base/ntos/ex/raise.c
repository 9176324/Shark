/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    raise.c

Abstract:

    This module implements routines to raise datatype misalignment and
    access violation for probe code.

    N.B. These routines are provided as function to save space in the
        probe macros.

    N.B. Since these routines are *only* called from the probe macros,
        it is assumed that the calling code is pageable.

--*/

#include "exp.h"

//
// Define function sections.
//

#pragma alloc_text(PAGE, ExRaiseAccessViolation)
#pragma alloc_text(PAGE, ExRaiseDatatypeMisalignment)

DECLSPEC_NOINLINE
VOID
ExRaiseAccessViolation (
    VOID
    )

/*++

Routine Description:

    This function raises an access violation exception.

    N.B. There is not return from this function.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ExRaiseStatus(STATUS_ACCESS_VIOLATION);
}

DECLSPEC_NOINLINE
VOID
ExRaiseDatatypeMisalignment (
    VOID
    )

/*++

Routine Description:

    This function raises a datatype misalignment exception.

    N.B. There is not return from this function.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ExRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
}

