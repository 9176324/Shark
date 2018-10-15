/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfrandom.h

Abstract:

    This header exposes support for random number generation as needed by the
    verifier.

--*/

VOID
VfRandomInit(
    VOID
    );

ULONG
FASTCALL
VfRandomGetNumber(
    IN  ULONG   Minimum,
    IN  ULONG   Maximum
    );

