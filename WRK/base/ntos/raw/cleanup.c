/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    Cleanup.c

Abstract:

    This module implements the File Cleanup routine for Raw called by the
    dispatch driver.

--*/

#include "RawProcs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawCleanup)
#endif


NTSTATUS
RawCleanup (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

/*++

Routine Description:

    This is the routine for cleaning up a handle.

Arguments:

    Vcb - Supplies the volume being queried.

    Irp - Supplies the Irp being processed.

    IrpSp - Supplies parameters describing the read

Return Value:

    NTSTATUS - The return status for the operation

--*/

{
    NTSTATUS Status;

    PAGED_CODE();

    //
    //  This is a Cleanup operation.  All we have to do is deal with
    //  share access.
    //

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

    IoRemoveShareAccess( IrpSp->FileObject, &Vcb->ShareAccess );

    //
    //  If the volume has been dismounted then the close count should be one.
    //  we will let the volume dismount complete at this point if so.
    //

    if (FlagOn( Vcb->VcbState,  VCB_STATE_FLAG_DISMOUNTED )) {

        ASSERT( Vcb->OpenCount == 1 );

        //
        //  Float this Vcb and Vpb while we wait for the close.  
        //  We know the Vcb won't go away in this call because our
        //  reference keeps the OpenCount above zero.
        //

        RawCheckForDismount( Vcb, FALSE );
    }

    (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );

    RawCompleteRequest( Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}

