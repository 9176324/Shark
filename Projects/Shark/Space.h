/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _SPACE_H_
#define _SPACE_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef _WIN64
#define GetPxeAddress(va) (NULL);
#define GetPpeAddress(va) (NULL);
#define GetVirtualAddressMappedByPxe(Pxe) (NULL);
#define GetVirtualAddressMappedByPpe(Ppe) (NULL);

    PMMPTE
        NTAPI
        _GetPdeAddress(
            __in PVOID VirtualAddress,
            __in PMMPTE PdeBase
        );

    PMMPTE
        NTAPI
        _GetPdeAddressPae(
            __in PVOID VirtualAddress,
            __in PMMPTE PdeBase
        );

    PMMPTE
        NTAPI
        _GetPteAddress(
            __in PVOID VirtualAddress,
            __in PMMPTE PteBase
        );

    PMMPTE
        NTAPI
        _GetPteAddressPae(
            __in PVOID VirtualAddress,
            __in PMMPTE PteBase
        );

    PVOID
        NTAPI
        _GetVirtualAddressMappedByPte(
            __in PMMPTE Pte
        );

    PVOID
        NTAPI
        _GetVirtualAddressMappedByPtePae(
            __in PMMPTE Pte
        );

    PVOID
        NTAPI
        _GetVirtualAddressMappedByPde(
            __in PMMPTE Pde
        );

    PVOID
        NTAPI
        _GetVirtualAddressMappedByPdePae(
            __in PMMPTE Pde
        );
#else
    PMMPTE
        NTAPI
        GetPxeAddress(
            __in PVOID VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPpeAddress(
            __in PVOID VirtualAddress
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPxe(
            __in PMMPTE Pxe
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPpe(
            __in PMMPTE Ppe
        );
#endif // !_WIN64

    VOID
        NTAPI
        InitializeSystemSpace(
            __inout PVOID Block
        );

    PMMPTE
        NTAPI
        GetPdeAddress(
            __in PVOID VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPteAddress(
            __in PVOID VirtualAddress
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPde(
            __in PMMPTE Pde
        );

    PVOID
        NTAPI
        GetVirtualAddressMappedByPte(
            __in PMMPTE Pte
        );

    //
    //  Miscellaneous support macros.
    //
    //      ULONG
    //      FlagOn (
    //          IN ULONG Flags,
    //          IN ULONG SingleFlag
    //          );
    //
    //      BOOLEAN
    //      BooleanFlagOn (
    //          IN ULONG Flags,
    //          IN ULONG SingleFlag
    //          );
    //
    //      VOID
    //      SetFlag (
    //          IN ULONG Flags,
    //          IN ULONG SingleFlag
    //          );
    //
    //      VOID
    //      ClearFlag (
    //          IN ULONG Flags,
    //          IN ULONG SingleFlag
    //          );
    //
    //      ULONG
    //      QuadAlign (
    //          IN ULONG Pointer
    //          );
    //

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#define BooleanFlagOn(F,SF) (    \
    (BOOLEAN)(((F) & (SF)) != 0) \
)

#define SetFlag(F,SF) { \
    (F) |= (SF);        \
}

#define ClearFlag(F,SF) { \
    (F) &= ~(SF);         \
}

#define QuadAlign(P) (             \
    ((((P)) + 7) & (-8)) \
)

#undef MM_PTE_WRITE_MASK
#define MM_PTE_WRITE_MASK 0x2

#ifndef _WIN64
#define MM_PTE_EXECUTE_MASK 0x8000000000000000UI64

#define PAE_ENABLE (FALSE != GpBlock->DebuggerDataBlock.PaeEnabled)

#define INSERT_EXECUTE_TO_VALID_PTE(PPTE) { \
            if(FALSE != PAE_ENABLE) \
                *(PULONG64)(PPTE) &= ~MM_PTE_EXECUTE_MASK; \
        }
#define REMOVE_EXECUTE_TO_VALID_PTE(PPTE) { \
            if(FALSE != PAE_ENABLE) \
                *(PULONG64)(PPTE) |= MM_PTE_EXECUTE_MASK; \
        }
#else
#define INSERT_EXECUTE_TO_VALID_PTE(PPTE) (PPTE)->u.Hard.NoExecute = 0;
#define REMOVE_EXECUTE_TO_VALID_PTE(PPTE) (PPTE)->u.Hard.NoExecute = 1;
#endif // !_WIN64

    VOID
        NTAPI
        _FlushSingleTb(
            __in PVOID VirtualAddress
        );

    VOID
        NTAPI
        FlushSingleTb(
            __in PVOID VirtualAddress
        );

    FORCEINLINE
        VOID
        NTAPI
        _FlushMultipleTb(
            __in PVOID VirtualAddress,
            __in SIZE_T RegionSize
        )
    {
        PFN_NUMBER NumberOfPages = 0;
        PFN_NUMBER PageFrameIndex = 0;

        NumberOfPages =
            BYTES_TO_PAGES(RegionSize +
            ((PCHAR)VirtualAddress - (PCHAR)PAGE_ALIGN(VirtualAddress)));

        for (PageFrameIndex = 0;
            PageFrameIndex < NumberOfPages;
            PageFrameIndex++) {
            _FlushSingleTb(
                (PCHAR)PAGE_ALIGN(VirtualAddress) + PAGE_SIZE * PageFrameIndex);
        }
    }

    VOID
        NTAPI
        FlushMultipleTb(
            __in PVOID VirtualAddress,
            __in SIZE_T RegionSize,
            __in BOOLEAN AllProcesors
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SPACE_H_
