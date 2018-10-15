/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   setmodfy.c

Abstract:

    This module contains the setting modify bit routine for memory management.
    x86 specific.

--*/

#include "mi.h"

#if defined (_X86PAE_)
extern PMMPTE MmSystemCacheWorkingSetListPte;
#endif


ULONG
FASTCALL
MiDetermineUserGlobalPteMask (
    IN PMMPTE Pte
    )

/*++

Routine Description:

    Builds a mask to OR with the PTE frame field.
    This mask has the valid and access bits set and
    has the global and owner bits set based on the
    address of the PTE.

    *******************  NOTE *********************************************
        THIS ROUTINE DOES NOT CHECK FOR PDEs WHICH NEED TO BE
        SET GLOBAL AS IT ASSUMES PDEs FOR SYSTEM SPACE ARE
        PROPERLY SET AT INITIALIZATION TIME!

Arguments:

    Pte - Supplies a pointer to the PTE in which to fill.

Return Value:

    Mask to OR into the frame to make a valid PTE.

Environment:

    Kernel mode, 386 specific.

--*/


{
    MMPTE Mask;

    Mask.u.Long = 0;
    Mask.u.Hard.Valid = 1;
    Mask.u.Hard.Accessed = 1;

#if defined (_X86PAE_)
    ASSERT (MmSystemCacheWorkingSetListPte != NULL);
#endif

    if (Pte <= MiHighestUserPte) {
        Mask.u.Hard.Owner = 1;
    }
    else if ((Pte < MiGetPteAddress (PTE_BASE)) ||
#if defined (_X86PAE_)
             (Pte >= MmSystemCacheWorkingSetListPte)
#else
             (Pte >= MiGetPteAddress (MM_SYSTEM_CACHE_WORKING_SET))
#endif
    ) {

        if (MI_IS_SESSION_PTE (Pte) == FALSE) {
#if defined (_X86PAE_)
          if ((Pte < (PMMPTE)PDE_BASE) || (Pte > (PMMPTE)PDE_TOP))
#endif
            Mask.u.Long |= MmPteGlobal.u.Long;
        }
    }
    else if ((Pte >= MiGetPdeAddress (NULL)) && (Pte <= MiHighestUserPde)) {
        Mask.u.Hard.Owner = 1;
    }

    //
    // Since the valid, accessed, global and owner bits are always in the
    // low dword of the PTE, returning a ULONG is ok.
    //

    return (ULONG)Mask.u.Long;
}

