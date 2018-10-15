/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    probe.c

Abstract:

    This module implements the probe for write function.

--*/

#include "exp.h"

#if defined(_WIN64)

#include <wow64t.h>

#endif

#undef ProbeForRead

VOID
ProbeForRead (
    __in CONST VOID *Address,
    __in SIZE_T Length,
    __in ULONG Alignment
    );

#pragma alloc_text(PAGE, ProbeForWrite)
#pragma alloc_text(PAGE, ProbeForRead)

VOID
ProbeForWrite (
    __inout_bcount(Length) PVOID Address,
    __in SIZE_T Length,
    __in ULONG Alignment
    )

/*++

Routine Description:

    This function probes a structure for write accessibility and ensures
    correct alignment of the structure. If the structure is not accessible
    or has incorrect alignment, then an exception is raised.

Arguments:

    Address - Supplies a pointer to the structure to be probed.

    Length - Supplies the length of the structure.

    Alignment - Supplies the required alignment of the structure expressed
        as the number of bytes in the primitive datatype (e.g., 1 for char,
        2 for short, 4 for long, and 8 for quad).

Return Value:

    None.

--*/

{

    ULONG_PTR EndAddress;
    ULONG_PTR StartAddress;

#define PageSize PAGE_SIZE

    //
    // If the structure has zero length, then do not probe the structure for
    // write accessibility or alignment.
    //

    if (Length != 0) {

        //
        // If the structure is not properly aligned, then raise a data
        // misalignment exception.
        //

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8) ||
               (Alignment == 16));

        StartAddress = (ULONG_PTR)Address;
        if ((StartAddress & (Alignment - 1)) == 0) {

            //
            // Compute the ending address of the structure and probe for
            // write accessibility.
            //

            EndAddress = StartAddress + Length - 1;
            if ((StartAddress <= EndAddress) &&
                (EndAddress < MM_USER_PROBE_ADDRESS)) {

                //
                // N.B. Only the contents of the buffer may be probed.
                //      Therefore the starting byte is probed for the
                //      first page, and then the first byte in the page
                //      for each succeeding page.
                //
                // If this is a Wow64 process, then the native page is 4K, which
                // could be smaller than the native page size/
                //

                EndAddress = (EndAddress & ~(PageSize - 1)) + PageSize;
                do {
                    *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;
                    StartAddress = (StartAddress & ~(PageSize - 1)) + PageSize;
                } while (StartAddress != EndAddress);

                return;

            } else {
                ExRaiseAccessViolation();
            }

        } else {
            ExRaiseDatatypeMisalignment();
        }
    }

    return;
}

VOID
ProbeForRead(
    __in_bcount(Length) VOID *Address,
    __in SIZE_T Length,
    __in ULONG Alignment
    )

/*++

Routine Description:

    This function probes a structure for read accessibility and ensures
    correct alignment of the structure. If the structure is not accessible
    or has incorrect alignment, then an exception is raised.

Arguments:

    Address - Supplies a pointer to the structure to be probed.

    Length - Supplies the length of the structure.

    Alignment - Supplies the required alignment of the structure expressed
        as the number of bytes in the primitive datatype (e.g., 1 for char,
        2 for short, 4 for long, and 8 for quad).

Return Value:

    None.

--*/

{

    PAGED_CODE();

    ASSERT((Alignment == 1) || (Alignment == 2) ||
           (Alignment == 4) || (Alignment == 8) ||
           (Alignment == 16));

    if (Length != 0) {
        if (((ULONG_PTR)Address & (Alignment - 1)) != 0) {
            ExRaiseDatatypeMisalignment();

        } else if ((((ULONG_PTR)Address + Length) > (ULONG_PTR)MM_USER_PROBE_ADDRESS) ||
                   (((ULONG_PTR)Address + Length) < (ULONG_PTR)Address)) {

            *(volatile UCHAR * const)MM_USER_PROBE_ADDRESS = 0;
        }
    }
}

