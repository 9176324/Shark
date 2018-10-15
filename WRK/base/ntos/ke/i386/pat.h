/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pat.h

Abstract:

    This module contains the i386 specific Page Attribute
    Table (PAT) register hardware definitions.

--*/

//
// PAT MSR architecture definitions
//

//
// PAT model specific register
//

#define PAT_MSR       0x277

//
// PAT memory attributes
//

#define PAT_TYPE_STRONG_UC  0       // corresponds to PPro PCD=1,PWT=1
#define PAT_TYPE_USWC       1
#define PAT_TYPE_WT         4
#define PAT_TYPE_WP         5
#define PAT_TYPE_WB         6
#define PAT_TYPE_WEAK_UC    7       // corresponds to PPro PCD=1,PWT=0
#define PAT_TYPE_MAX        8       

#include "pshpack1.h"

typedef union _PAT {
    struct {
        UCHAR Pat[8];
    } hw;
    ULONGLONG   QuadPart;
} PAT, *PPAT;

#include "poppack.h"

