/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pctohdr.c

Abstract:

    This module implements code to locate the file header for an image or
    dll given a PC value that lies within the image.

    N.B. This routine is conditionalized for user mode and kernel mode.

--*/

#include "ntos.h"

PVOID
RtlPcToFileHeader(
    IN PVOID PcValue,
    OUT PVOID *BaseOfImage
    )

/*++

Routine Description:

    This function returns the base of an image that contains the
    specified PcValue. An image contains the PcValue if the PcValue
    is within the ImageBase, and the ImageBase plus the size of the
    virtual image.

Arguments:

    PcValue - Supplies a PcValue.  All of the modules mapped into the
        calling processes address space are scanned to compute which
        module contains the PcValue.

    BaseOfImage - Returns the base address for the image containing the
        PcValue.  This value must be added to any relative addresses in
        the headers to locate portions of the image.

Return Value:

    NULL - No image was found that contains the PcValue.

    NON-NULL - Returns the base address of the image that contain the
        PcValue.

--*/

{
    extern LIST_ENTRY PsLoadedModuleList;
    extern KSPIN_LOCK PsLoadedModuleSpinLock;

    PVOID Base;
    ULONG_PTR Bounds;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Next;
    KIRQL OldIrql;

    //
    // Acquire the loaded module list spinlock and scan the list for the
    // specified PC value if the list has been initialized.
    //

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < SYNCH_LEVEL) {
        KeRaiseIrqlToSynchLevel();
    }

    ExAcquireSpinLockAtDpcLevel(&PsLoadedModuleSpinLock);
    Next = PsLoadedModuleList.Flink;
    if (Next != NULL) {
        while (Next != &PsLoadedModuleList) {
            Entry = CONTAINING_RECORD(Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InLoadOrderLinks);

            Next = Next->Flink;
            Base = Entry->DllBase;
            Bounds = (ULONG_PTR)Base + Entry->SizeOfImage;
            if (((ULONG_PTR)PcValue >= (ULONG_PTR)Base) && ((ULONG_PTR)PcValue < Bounds)) {
                ExReleaseSpinLockFromDpcLevel(&PsLoadedModuleSpinLock);
                KeLowerIrql(OldIrql);
                *BaseOfImage = Base;
                return Base;
            }
        }
    }

    //
    // Release the loaded module list spin lock and return NULL.
    //

    ExReleaseSpinLockFromDpcLevel(&PsLoadedModuleSpinLock);
    KeLowerIrql(OldIrql);
    *BaseOfImage = NULL;
    return NULL;
}

