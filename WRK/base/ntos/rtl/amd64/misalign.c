/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    miaslign.c

Abstract:

    This module implements __misaligned_access(). 

--*/

#include "ntrtlp.h"


VOID
__misaligned_access (
    IN PVOID Address,
    IN LONG Size,
    IN LONG Alignment,
    IN BOOLEAN Write
    )
{
}

