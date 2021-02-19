/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
#define GetVaMappedByPxe(Pxe) (NULL);
#define GetVaMappedByPpe(Ppe) (NULL);

    PMMPTE
        NTAPI
        _GetPdeAddress(
            __in ptr VirtualAddress,
            __in PMMPTE PdeBase
        );

    PMMPTE
        NTAPI
        _GetPdeAddressPae(
            __in ptr VirtualAddress,
            __in PMMPTE PdeBase
        );

    PMMPTE
        NTAPI
        _GetPteAddress(
            __in ptr VirtualAddress,
            __in PMMPTE PteBase
        );

    PMMPTE
        NTAPI
        _GetPteAddressPae(
            __in ptr VirtualAddress,
            __in PMMPTE PteBase
        );

    ptr
        NTAPI
        _GetVaMappedByPte(
            __in PMMPTE Pte
        );

    ptr
        NTAPI
        _GetVaMappedByPtePae(
            __in PMMPTE Pte
        );

    ptr
        NTAPI
        _GetVaMappedByPde(
            __in PMMPTE Pde
        );

    ptr
        NTAPI
        _GetVaMappedByPdePae(
            __in PMMPTE Pde
        );
#else
    PMMPTE
        NTAPI
        GetPxeAddress(
            __in ptr VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPpeAddress(
            __in ptr VirtualAddress
        );

    ptr
        NTAPI
        GetVaMappedByPxe(
            __in PMMPTE Pxe
        );

    ptr
        NTAPI
        GetVaMappedByPpe(
            __in PMMPTE Ppe
        );
#endif // !_WIN64

    void
        NTAPI
        InitializeSpace(
            __inout ptr Block
        );

    PMMPTE
        NTAPI
        GetPdeAddress(
            __in ptr VirtualAddress
        );

    PMMPTE
        NTAPI
        GetPteAddress(
            __in ptr VirtualAddress
        );

    ptr
        NTAPI
        GetVaMappedByPde(
            __in PMMPTE Pde
        );

    ptr
        NTAPI
        GetVaMappedByPte(
            __in PMMPTE Pte
        );

    //
    //  Miscellaneous support macros.
    //
    //      u32
    //      FlagOn (
    //          IN u32 Flags,
    //          IN u32 SingleFlag
    //          );
    //
    //      b
    //      BooleanFlagOn (
    //          IN u32 Flags,
    //          IN u32 SingleFlag
    //          );
    //
    //      void
    //      SetFlag (
    //          IN u32 Flags,
    //          IN u32 SingleFlag
    //          );
    //
    //      void
    //      ClearFlag (
    //          IN u32 Flags,
    //          IN u32 SingleFlag
    //          );
    //
    //      u32
    //      QuadAlign (
    //          IN u32 Pointer
    //          );
    //

#define FlagOn(F,SF) ( \
    (((F) & (SF)))     \
)

#define BooleanFlagOn(F,SF) (    \
    (b)(((F) & (SF)) != 0) \
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

#define PAE_ENABLE (FALSE != GpBlock.DebuggerDataBlock.PaeEnabled)

#define INSERT_EXECUTE_TO_VALID_PTE(PPTE) { \
            if(FALSE != PAE_ENABLE) \
                *(u64ptr)(PPTE) &= ~MM_PTE_EXECUTE_MASK; \
        }
#define REMOVE_EXECUTE_TO_VALID_PTE(PPTE) { \
            if(FALSE != PAE_ENABLE) \
                *(u64ptr)(PPTE) |= MM_PTE_EXECUTE_MASK; \
        }
#else
#define INSERT_EXECUTE_TO_VALID_PTE(PPTE) (PPTE)->u.Hard.NoExecute = 0;
#define REMOVE_EXECUTE_TO_VALID_PTE(PPTE) (PPTE)->u.Hard.NoExecute = 1;
#endif // !_WIN64

    void
        NTAPI
        _FlushSingleTb(
            __in ptr VirtualAddress
        );

    void
        NTAPI
        FlushSingleTb(
            __in ptr VirtualAddress
        );

    FORCEINLINE
        void
        NTAPI
        _FlushMultipleTb(
            __in ptr VirtualAddress,
            __in u RegionSize
        )
    {
        PFN_NUMBER NumberOfPages = 0;
        PFN_NUMBER PageFrameIndex = 0;

        NumberOfPages =
            BYTES_TO_PAGES(RegionSize +
            ((u8ptr)VirtualAddress - (u8ptr)PAGE_ALIGN(VirtualAddress)));

        for (PageFrameIndex = 0;
            PageFrameIndex < NumberOfPages;
            PageFrameIndex++) {
            _FlushSingleTb(
                (u8ptr)PAGE_ALIGN(VirtualAddress) + PAGE_SIZE * PageFrameIndex);
        }
    }

    void
        NTAPI
        FlushMultipleTb(
            __in ptr VirtualAddress,
            __in u RegionSize,
            __in b AllProcesors
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SPACE_H_
