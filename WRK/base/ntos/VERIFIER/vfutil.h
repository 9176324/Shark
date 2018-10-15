/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfutil.h

Abstract:

    This header contains prototypes for various functions required to do driver
    verification.

--*/

typedef enum {

    VFMP_INSTANT = 0,
    VFMP_INSTANT_NONPAGED

} MEMORY_PERSISTANCE;

BOOLEAN
VfUtilIsMemoryRangeReadable(
    IN PVOID                Location,
    IN size_t               Length,
    IN MEMORY_PERSISTANCE   Persistance
    );

VOID
VfSetVerifierEnabled (
    LOGICAL Value
    );

