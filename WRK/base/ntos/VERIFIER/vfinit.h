/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfinit.h

Abstract:

    This header exposes the routines necessary to initialize the driver verifier.

--*/

VOID
FASTCALL
VfInitVerifier(
    IN  ULONG   MmFlags
    );

