/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    mtrr.h

Abstract:

    This module contains the i386 specific mtrr register 
    hardware definitions.

--*/

//
// MTRR MSR architecture definitions
//

#define MTRR_MSR_CAPABILITIES       0x0fe
#define MTRR_MSR_DEFAULT            0x2ff
#define MTRR_MSR_VARIABLE_BASE      0x200
#define MTRR_MSR_VARIABLE_MASK     (MTRR_MSR_VARIABLE_BASE+1)

#define MTRR_PAGE_SIZE              4096
#define MTRR_PAGE_MASK              (~(MTRR_PAGE_SIZE-1))

//
// Memory range types
//

#define MTRR_TYPE_UC            0
#define MTRR_TYPE_USWC          1
#define MTRR_TYPE_WT            4
#define MTRR_TYPE_WP            5
#define MTRR_TYPE_WB            6
#define MTRR_TYPE_MAX           7

//
// MTRR specific registers - capability register, default
// register, and variable mask and base register
//

#include "pshpack1.h"

typedef struct _MTRR_CAPABILITIES {
    union {
        struct {
            ULONG   VarCnt:8;
            ULONG   FixSupported:1;
            ULONG   Reserved_0:1;
            ULONG   UswcSupported:1;
            ULONG   Reserved_1:21;
            ULONG   Reserved_2;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_CAPABILITIES, *PMTRR_CAPABILITIES;

typedef struct _MTRR_DEFAULT {
    union {
        struct {
            ULONG   Type:8;
            ULONG   Reserved_0:2;
            ULONG   FixedEnabled:1;
            ULONG   MtrrEnabled:1;
            ULONG   Reserved_1:20;
            ULONG   Reserved_2;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_DEFAULT, *PMTRR_DEFAULT;

typedef struct _MTRR_VARIABLE_BASE {
    union {
        struct {
            ULONG       Type:8;
            ULONG       Reserved_0:4;
            ULONG       PhysBase_1:20;
            ULONG       PhysBase_2:4;
            ULONG       PhysBase_3:4;
            ULONG       Reserved_1:24;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_BASE, *PMTRR_VARIABLE_BASE;

typedef struct _MTRR_VARIABLE_MASK {
    union {
        struct {
            ULONG      Reserved_0:11;
            ULONG      Valid:1;
            ULONG      PhysMask_1:20;
            ULONG      PhysMask_2:4;
            ULONG      PhysMask_3:4;
            ULONG      Reserved_1:24;
        } hw;
        ULONGLONG   QuadPart;
    } u;
} MTRR_VARIABLE_MASK, *PMTRR_VARIABLE_MASK;

#include "poppack.h"

typedef struct _PROCESSOR_LOCKSTEP {
    ULONG               Processor;
    volatile ULONG      TargetCount;
    volatile ULONG      *TargetPhase;
} PROCESSOR_LOCKSTEP, *PPROCESSOR_LOCKSTEP;

VOID
KiLockStepExecution(
    IN PPROCESSOR_LOCKSTEP Context
    );

