/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    dataamd.c

Abstract:

    This module contains the private hardware specific global storage for
    the memory management subsystem.

--*/

#include "mi.h"

//
// A zero Pte.
//

const MMPTE ZeroPte = { 0 };

//
// A kernel zero PTE.
//

const MMPTE ZeroKernelPte = {0x0};

MMPTE ValidKernelPte = { MM_PTE_VALID_MASK |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_GLOBAL_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_ACCESS_MASK };

const MMPTE ValidKernelPteLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_ACCESS_MASK };


MMPTE ValidUserPte = { MM_PTE_VALID_MASK |
                       MM_PTE_WRITE_MASK |
                       MM_PTE_OWNER_MASK |
                       MM_PTE_DIRTY_MASK |
                       MM_PTE_ACCESS_MASK };


const MMPTE ValidPtePte = { MM_PTE_VALID_MASK |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_DIRTY_MASK |
                            MM_PTE_ACCESS_MASK };


const MMPTE ValidPdePde = { MM_PTE_VALID_MASK |
                            MM_PTE_WRITE_MASK |
                            MM_PTE_DIRTY_MASK |
                            MM_PTE_ACCESS_MASK };


MMPTE ValidKernelPde = { MM_PTE_VALID_MASK |
                         MM_PTE_WRITE_MASK |
                         MM_PTE_GLOBAL_MASK |
                         MM_PTE_DIRTY_MASK |
                         MM_PTE_ACCESS_MASK };

const MMPTE ValidKernelPdeLocal = { MM_PTE_VALID_MASK |
                                    MM_PTE_WRITE_MASK |
                                    MM_PTE_DIRTY_MASK |
                                    MM_PTE_ACCESS_MASK };


MMPTE DemandZeroPde = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE DemandZeroPte = { MM_READWRITE << MM_PROTECT_FIELD_SHIFT };


const MMPTE TransitionPde = { MM_PTE_WRITE_MASK |
                              MM_PTE_OWNER_MASK |
                              MM_PTE_TRANSITION_MASK |
                              MM_READWRITE << MM_PROTECT_FIELD_SHIFT };

MMPTE PrototypePte = { (MI_PTE_LOOKUP_NEEDED << 32) |
                       MM_PTE_PROTOTYPE_MASK |
                       MM_READWRITE << MM_PROTECT_FIELD_SHIFT };

//
// PTE which generates an access violation when referenced.
//

const MMPTE NoAccessPte = {MM_NOACCESS << MM_PROTECT_FIELD_SHIFT };

//
// Pool start and end.
//

PVOID MmNonPagedPoolStart;

PVOID MmNonPagedPoolEnd = (PVOID)MM_NONPAGED_POOL_END;

PVOID MmPagedPoolStart =  (PVOID)MM_PAGED_POOL_START;

PVOID MmPagedPoolEnd;

//
// Color tables for free and zeroed pages.
//

PMMCOLOR_TABLES MmFreePagesByColor[2];

//
// Color tables for modified pages destined for the paging file.
//

MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS] = {
                            0, ModifiedPageList, MM_EMPTY_LIST, MM_EMPTY_LIST};

//
// Count of the number of modified pages destined for the paging file.
//

PFN_NUMBER MmTotalPagesForPagingFile = 0;

//
// PTE reserved for mapping pages for the debugger.
//

PMMPTE MmDebugPte;
PVOID MmDebugVa;

//
// 16 PTEs reserved for mapping MDLs (64k max).
//

PMMPTE MmCrashDumpPte;
PVOID MmCrashDumpVa;

ULONG_PTR MmBootImageSize;

