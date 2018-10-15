/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfstack.h

Abstract:

    This header contains prototypes for verifying drivers don't improperly use
    thread stacks.

--*/

VOID
FASTCALL
VfStackSeedStack(
    IN  ULONG   Seed
    );

