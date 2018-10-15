/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ldtsup.c

Abstract:

    This module implements interfaces that support manipulation of i386 Ldts.
    These entry points only exist on i386 machines.

--*/

#include "ki.h"

//
// Low level assembler support procedures
//

VOID
KiLoadLdtr(
    VOID
    );

VOID
KiFlushDescriptors(
    VOID
    );

//
// Local service procedures
//


VOID
Ki386LoadTargetLdtr (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
Ki386FlushTargetDescriptors (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

typedef struct _LDTINFO {
    PKPROCESS Process;
    KGDTENTRY LdtDescriptor;
    ULONG Offset;
    LDT_ENTRY LdtEntry;
    PLDT_ENTRY Ldt;
} LDTINFO, *PLDTINFO;

VOID
Ke386SetLdtProcess (
    IN PKPROCESS Process,
    IN PLDT_ENTRY Ldt,
    IN ULONG Limit
    )

/*++

Routine Description:

    The specified LDT (which may be null) will be made the active Ldt of
    the specified process, for all threads thereof, on whichever
    processors they are running.  The change will take effect before the
    call returns.

    An Ldt address of NULL or a Limit of 0 will cause the process to
    receive the NULL Ldt.

    This function only exists on i386 and i386 compatible processors.

    No checking is done on the validity of Ldt entries.


    N.B.

    While a single Ldt structure can be shared among processes, any
    edits to the Ldt of one of those processes will only be synchronized
    for that process.  Thus, processes other than the one the change is
    applied to may not see the change correctly.

Arguments:

    Process - Pointer to KPROCESS object describing the process for
        which the Ldt is to be set.

    Ldt - Pointer to an array of LDT_ENTRYs (that is, a pointer to an
        Ldt.)

    Limit - Ldt limit (must be 0 mod 8)

Return Value:

    None.

--*/

{
    LDTINFO LdtInfo;
    KGDTENTRY LdtDescriptor;

    //
    // Compute the contents of the Ldt descriptor
    //

    if ((Ldt == NULL) || (Limit == 0)) {

        //
        //  Set up an empty descriptor
        //

        LdtDescriptor.LimitLow = 0;
        LdtDescriptor.BaseLow = 0;
        LdtDescriptor.HighWord.Bytes.BaseMid = 0;
        LdtDescriptor.HighWord.Bytes.Flags1 = 0;
        LdtDescriptor.HighWord.Bytes.Flags2 = 0;
        LdtDescriptor.HighWord.Bytes.BaseHi = 0;

    } else {

        //
        // Ensure that the unfilled fields of the selector are zero
        // N.B.  If this is not done, random values appear in the high
        //       portion of the Ldt limit.
        //

        LdtDescriptor.HighWord.Bytes.Flags1 = 0;
        LdtDescriptor.HighWord.Bytes.Flags2 = 0;

        //
        //  Set the limit and base
        //

        LdtDescriptor.LimitLow = (USHORT) ((ULONG) Limit - 1);
        LdtDescriptor.BaseLow = (USHORT)  ((ULONG) Ldt & 0xffff);
        LdtDescriptor.HighWord.Bytes.BaseMid = (UCHAR) (((ULONG)Ldt & 0xff0000) >> 16);
        LdtDescriptor.HighWord.Bytes.BaseHi =  (UCHAR) (((ULONG)Ldt & 0xff000000) >> 24);

        //
        //  Type is LDT, DPL = 0
        //

        LdtDescriptor.HighWord.Bits.Type = TYPE_LDT;
        LdtDescriptor.HighWord.Bits.Dpl = DPL_SYSTEM;

        //
        // Make it present
        //

        LdtDescriptor.HighWord.Bits.Pres = 1;

    }

    LdtInfo.Process       = Process;
    LdtInfo.LdtDescriptor = LdtDescriptor;

    KeGenericCallDpc (Ki386LoadTargetLdtr,
                      &LdtInfo);

    return;
}

VOID
Ki386LoadTargetLdtr (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
/*++

Routine Description:

    Reload local Ldt register and clear signal bit in TargetProcessor mask

Arguments:

    Dpc - DPC used to initiate this call
    DeferredContext - Context
    SystemArgument1 - System context, Used to signal completion of this call
    SystemArgument2 - System context

Return Value:

    none.

--*/

{
    PLDTINFO LdtInfo;

    UNREFERENCED_PARAMETER (Dpc);

    LdtInfo = DeferredContext;

    //
    // Make sure all DPC's are running so a load of the process
    // LdtDescriptor field can't be torn
    //

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

        //
        // Set the Ldt fields in the process object.
        //

        LdtInfo->Process->LdtDescriptor = LdtInfo->LdtDescriptor;
    }

    //
    // Make sure the field has been updated before we continue
    //

    KeSignalCallDpcSynchronize (SystemArgument2);

    //
    // Reload the LDTR register from currently active process object
    //

    KiLoadLdtr();

    //
    // Signal that all processing has been done
    //

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

VOID
Ke386SetDescriptorProcess (
    IN PKPROCESS Process,
    IN ULONG Offset,
    IN LDT_ENTRY LdtEntry
    )
/*++

Routine Description:

    The specified LdtEntry (which could be 0, not present, etc) will be
    edited into the specified Offset in the Ldt of the specified Process.
    This will be synchronized across all the processors executing the
    process.  The edit will take affect on all processors before the call
    returns.

    N.B.

    Editing an Ldt descriptor requires stalling all processors active
    for the process, to prevent accidental loading of descriptors in
    an inconsistent state.

Arguments:

    Process - Pointer to KPROCESS object describing the process for
        which the descriptor edit is to be performed.

    Offset - Byte offset into the Ldt of the descriptor to edit.
        Must be 0 mod 8.

    LdtEntry - Value to edit into the descriptor in hardware format.
        No checking is done on the validity of this item.

Return Value:

    none.

--*/

{

    PLDT_ENTRY Ldt;
    LDTINFO LdtInfo;

    //
    // Compute address of descriptor to edit. It is safe to fetch the process
    // LdtDescriptor here as we are always called with the PS LdtMutex held.
    //

    Ldt =
        (PLDT_ENTRY)
         ((Process->LdtDescriptor.HighWord.Bytes.BaseHi << 24) |
         ((Process->LdtDescriptor.HighWord.Bytes.BaseMid << 16) & 0xff0000) |
         (Process->LdtDescriptor.BaseLow & 0xffff));
    Offset = Offset / 8;


    LdtInfo.Process  = Process;
    LdtInfo.Offset   = Offset;
    LdtInfo.Ldt      = Ldt;
    LdtInfo.LdtEntry = LdtEntry;

    KeGenericCallDpc (Ki386FlushTargetDescriptors,
                      &LdtInfo);

    return;
}

VOID
Ki386FlushTargetDescriptors (
    PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function flushes the segment descriptors on the current processor.

Arguments:

    Dpc - DPC used to initiate this call
    DeferredContext - Context
    SystemArgument1 - System context, Used to signal completion of this call
    SystemArgument2 - System context

Return Value:

    none.

--*/

{
    PLDTINFO LdtInfo;

    UNREFERENCED_PARAMETER (Dpc);

    LdtInfo = DeferredContext;

    //
    // Flush the segment descriptors on the current processor.
    // This call removes all possible references to the LDT from
    // the segment registers.
    //

    KiFlushDescriptors ();

    //
    // Make sure all DPC's are running so a load of the process
    // LdtDescriptor field can't be torn
    //

    if (KeSignalCallDpcSynchronize (SystemArgument2)) {

        //
        // Update the LDT entry
        //

        LdtInfo->Ldt[LdtInfo->Offset] = LdtInfo->LdtEntry;
    }

    //
    // Wait until everyone has got to this point before continuing
    //

    KeSignalCallDpcSynchronize (SystemArgument2);


    //
    // Signal that all processing has been done
    //

    KeSignalCallDpcDone (SystemArgument1);
    return;
}

