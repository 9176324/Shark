/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    kevutil.c

Abstract:

    This module implements various utilities required to do driver verification.

--*/

#include "ki.h"

#pragma alloc_text(PAGEVRFY, KevUtilAddressToFileHeader)

NTSTATUS
KevUtilAddressToFileHeader(
    IN  PVOID                   Address,
    OUT UINT_PTR                *OffsetIntoImage,
    OUT PUNICODE_STRING         *DriverName,
    OUT BOOLEAN                 *InVerifierList
    )

/*++

Routine Description:

    This function returns the name of a driver based on the specified
    Address. In addition, the offset into the driver is returned along
    with an indication as to whether the driver is among the list of those
    being verified.

Arguments:

    Address         - Supplies an address to resolve to a driver name.

    OffsetIntoImage - Receives the offset relative to the base of the driver.

    DriverName      - Receives a pointer to the name of the driver.

    InVerifierList  - Receives TRUE if the driver is in the verifier list,
                      FALSE otherwise.

Return Value:

    NTSTATUS (On failure, OffsetIntoImage receives NULL, DriverName receives
              NULL, and InVerifierList receives FALSE).

--*/

{

    PLIST_ENTRY pModuleListHead, next;
    PLDR_DATA_TABLE_ENTRY pDataTableEntry;
    UINT_PTR bounds, pCurBase;

    //
    // Preinit for failure
    //

    *DriverName = NULL;
    *InVerifierList = FALSE;
    *OffsetIntoImage = 0;

    //
    // Set initial values for the module walk
    //

    pModuleListHead = &PsLoadedModuleList;

    //
    // It would be nice if we could call MiLookupDataTableEntry, but it's
    // pageable, so we do what the bugcheck stuff does...
    //

    next = pModuleListHead->Flink;
    if (next != NULL) {
        while (next != pModuleListHead) {

            //
            // Extract the data table entry
            //
            pDataTableEntry = CONTAINING_RECORD(
                next,
                LDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks
                );

            next = next->Flink;
            pCurBase = (UINT_PTR) pDataTableEntry->DllBase;
            bounds = pCurBase + pDataTableEntry->SizeOfImage;
            if ((UINT_PTR)Address >= pCurBase && (UINT_PTR)Address < bounds) {

                //
                // We have a match, record it and get out of here.
                //

                *OffsetIntoImage = (UINT_PTR) Address - pCurBase;
                *DriverName = &pDataTableEntry->BaseDllName;

                //
                // Now record whether this is in the verifying table.
                //

                if (pDataTableEntry->Flags & LDRP_IMAGE_VERIFYING) {

                    *InVerifierList = TRUE;
                }

                return STATUS_SUCCESS;
            }
        }
    }

    return STATUS_UNSUCCESSFUL;
}

