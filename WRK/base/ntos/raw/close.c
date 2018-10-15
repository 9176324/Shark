/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    Close.c

Abstract:

    This module implements the File Close routine for Raw called by the
    dispatch driver.

--*/

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawClose)
#endif

NTSTATUS
RawClose (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

/*++

Routine Description:

    This is the routine for closing a volume.

Arguments:

    Vcb - Supplies the volume being queried.

    Irp - Supplies the Irp being processed.

    IrpSp - Supplies parameters describing the read

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;
    BOOLEAN DeleteVolume = FALSE;

    PAGED_CODE();

    //
    //  This is a close operation.  If it is the last one, dismount.
    //

    //
    // Skip stream files as they are unopened fileobjects.
    // This might be a close from IopInvalidateVolumesForDevice
    // 
    if (IrpSp->FileObject->Flags & FO_STREAM_FILE) {
        RawCompleteRequest( Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

    Vcb->OpenCount -= 1;

    if (Vcb->OpenCount == 0) {

        DeleteVolume = RawCheckForDismount( Vcb, FALSE );
    }

    if (!DeleteVolume) {
        (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );
    }

    RawCompleteRequest( Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}

