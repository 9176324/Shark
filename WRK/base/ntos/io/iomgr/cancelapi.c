/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cancelapi.c

Abstract:

    This module contains the cancel safe DDI set

--*/

#include "iomgr.h"

//
// The library exposes everything with the name "Wdmlib". This ensures drivers
// using the backward compatible Cancel DDI Lib won't opportunistically pick
// up the kernel exports just because they were built using the XP DDK.
//
#if CSQLIB

#define CSQLIB_DDI(x) Wdmlib##x

#else

#define CSQLIB_DDI(x) x

#endif

VOID
IopCsqCancelRoutine(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
/*++

Routine Description:

    This routine removes the IRP that's associated with a context from the queue.
    It's expected that this routine will be called from a timer or DPC or other threads which complete an
    IO. Note that the IRP associated with this context could already have been freed.

Arguments:

    Csq - Pointer to the cancel queue.
    Context - Context associated with Irp.


Return Value:

    Returns the IRP associated with the context. If the value is not NULL, the IRP was successfully
    retrieved and can be used safely. If the value is NULL, the IRP was already canceled.

--*/
{
    KIRQL   irql;
    PIO_CSQ_IRP_CONTEXT irpContext;
    PIO_CSQ cfq;

    UNREFERENCED_PARAMETER (DeviceObject);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    irpContext = Irp->Tail.Overlay.DriverContext[3];

    if (irpContext->Type == IO_TYPE_CSQ_IRP_CONTEXT) {
        cfq = irpContext->Csq;
    } else if ((irpContext->Type == IO_TYPE_CSQ) ||
                (irpContext->Type == IO_TYPE_CSQ_EX)) {
        cfq = (PIO_CSQ)irpContext;
    } else {

        //
        // Bad type
        //

        ASSERT(0);
        return;
    }

    ASSERT(cfq);

    cfq->ReservePointer = NULL; // Force drivers to be good citizens

    cfq->CsqAcquireLock(cfq, &irql);
    cfq->CsqRemoveIrp(cfq, Irp);


    //
    // Break the association if necessary.
    //

    if (irpContext != (PIO_CSQ_IRP_CONTEXT)cfq) {
        irpContext->Irp = NULL;

        Irp->Tail.Overlay.DriverContext[3] = NULL;
    }
    cfq->CsqReleaseLock(cfq, irql);

    cfq->CsqCompleteCanceledIrp(cfq, Irp);
}

NTSTATUS
CSQLIB_DDI(IoCsqInitialize)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP               CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    )
/*++

Routine Description:

    This routine initializes the Cancel queue

Arguments:

    Csq - Pointer to the cancel queue.


Return Value:

    The function returns STATUS_SUCCESS on successful initialization

--*/
{
    Csq->CsqInsertIrp = CsqInsertIrp;
    Csq->CsqRemoveIrp = CsqRemoveIrp;
    Csq->CsqPeekNextIrp = CsqPeekNextIrp;
    Csq->CsqAcquireLock = CsqAcquireLock;
    Csq->CsqReleaseLock = CsqReleaseLock;
    Csq->CsqCompleteCanceledIrp = CsqCompleteCanceledIrp;
    Csq->ReservePointer = NULL;

    Csq->Type = IO_TYPE_CSQ;

    return STATUS_SUCCESS;
}

NTSTATUS
CSQLIB_DDI(IoCsqInitializeEx)(
    IN PIO_CSQ                          Csq,
    IN PIO_CSQ_INSERT_IRP_EX            CsqInsertIrp,
    IN PIO_CSQ_REMOVE_IRP               CsqRemoveIrp,
    IN PIO_CSQ_PEEK_NEXT_IRP            CsqPeekNextIrp,
    IN PIO_CSQ_ACQUIRE_LOCK             CsqAcquireLock,
    IN PIO_CSQ_RELEASE_LOCK             CsqReleaseLock,
    IN PIO_CSQ_COMPLETE_CANCELED_IRP    CsqCompleteCanceledIrp
    )
/*++

Routine Description:

    This routine initializes the Cancel queue

Arguments:

    Csq - Pointer to the cancel queue.


Return Value:

    The function returns STATUS_SUCCESS on successful initialization

--*/
{
    Csq->CsqInsertIrp = (PIO_CSQ_INSERT_IRP)CsqInsertIrp;
    Csq->CsqRemoveIrp = CsqRemoveIrp;
    Csq->CsqPeekNextIrp = CsqPeekNextIrp;
    Csq->CsqAcquireLock = CsqAcquireLock;
    Csq->CsqReleaseLock = CsqReleaseLock;
    Csq->CsqCompleteCanceledIrp = CsqCompleteCanceledIrp;
    Csq->ReservePointer = NULL;

    Csq->Type = IO_TYPE_CSQ_EX;

    return STATUS_SUCCESS;
}

NTSTATUS
CSQLIB_DDI(IoCsqInsertIrpEx)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context,
    IN  PVOID               InsertContext
    )
/*++

Routine Description:

    This routine inserts the IRP into the queue and associates the context with the IRP.
    The context has to be in non-paged pool if the context will be used in a DPC or interrupt routine.
    The routine assumes that Irp->Tail.Overlay.DriverContext[3] is available for use by the APIs.
    It's ok to pass a NULL context if the driver assumes that it will always use IoCsqRemoveNextIrp to
    remove an IRP.

Arguments:

    Csq - Pointer to the cancel queue.
    Irp - Irp to be inserted
    Context - Context to be associated with Irp.
    InsertContext - Context passed to the driver's insert IRP routine.


Return Value:

    NTSTATUS - Status returned by the driver's insert IRP routine. If the driver's insert IRP
    routine returns an error status, the IRP is not inserted and the same status is returned
    by this API. This allows drivers to implement startIo kind of functionality.

--*/
{
    KIRQL           irql;
    PDRIVER_CANCEL  cancelRoutine;
    PVOID           originalDriverContext;
    NTSTATUS        status = STATUS_SUCCESS;

    //
    // Set the association between the context and the IRP.
    //

    if (Context) {
        Irp->Tail.Overlay.DriverContext[3] = Context;
        Context->Irp = Irp;
        Context->Csq = Csq;
        Context->Type = IO_TYPE_CSQ_IRP_CONTEXT;
    } else {
        Irp->Tail.Overlay.DriverContext[3] = Csq;
    }


    Csq->ReservePointer = NULL; // Force drivers to be good citizens

    originalDriverContext = Irp->Tail.Overlay.DriverContext[3];

    Csq->CsqAcquireLock(Csq, &irql);


    //
    // If the driver wants to fail the insert then do this.
    //

    if (Csq->Type == IO_TYPE_CSQ_EX) {

        PIO_CSQ_INSERT_IRP_EX   func;

        func = (PIO_CSQ_INSERT_IRP_EX)Csq->CsqInsertIrp;
        status = func(Csq, Irp, InsertContext);

        if (!NT_SUCCESS(status)) {

            Csq->CsqReleaseLock(Csq, irql);

            return status;
        }

    } else {
        Csq->CsqInsertIrp(Csq, Irp);
    }

    IoMarkIrpPending(Irp);

    cancelRoutine = IoSetCancelRoutine(Irp, IopCsqCancelRoutine);


    ASSERT(!cancelRoutine);

    if (Irp->Cancel) {

        cancelRoutine = IoSetCancelRoutine(Irp, NULL);

        if (cancelRoutine) {

            Csq->CsqRemoveIrp(Csq, Irp);

            if (Context) {
                Context->Irp = NULL;
            }

            Irp->Tail.Overlay.DriverContext[3] = NULL;


            Csq->CsqReleaseLock(Csq, irql);

            Csq->CsqCompleteCanceledIrp(Csq, Irp);

        } else {

            //
            // The cancel routine beat us to it.
            //

            Csq->CsqReleaseLock(Csq, irql);
        }

    } else {

        Csq->CsqReleaseLock(Csq, irql);

    }
    return status;
}

PIRP
CSQLIB_DDI(IoCsqRemoveNextIrp)(
    IN  PIO_CSQ   Csq,
    IN  PVOID     PeekContext
    )
/*++

Routine Description:

    This routine removes the next IRP from the queue. This routine will enumerate the queue
    and return an IRP that's not canceled. If an IRP in the queue is canceled it goes to the next
    IRP. If no IRP is available it returns a NULL. The IRP returned is safe and cannot be canceled.

Arguments:

    Csq - Pointer to the cancel queue.


Return Value:

    Returns the IRP or NULL.

--*/
{
    KIRQL   irql;
    PIO_CSQ_IRP_CONTEXT context;
    PDRIVER_CANCEL  cancelRoutine;
    PIRP    irp;


    irp = NULL;

    Csq->ReservePointer = NULL; // Force drivers to be good citizens
    Csq->CsqAcquireLock(Csq, &irql);

    irp = Csq->CsqPeekNextIrp(Csq, NULL, PeekContext);

    while (1) {

        //
        // This routine will return a pointer to the next IRP in the queue adjacent to
        // the irp passed as a parameter. If the irp is NULL, it returns the IRP at the head of
        // the queue.
        //

        if (!irp) {
            Csq->CsqReleaseLock(Csq, irql);
            return NULL;
        }

        cancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (!cancelRoutine) {
            irp = Csq->CsqPeekNextIrp(Csq, irp, PeekContext);
            continue;
        }

        Csq->CsqRemoveIrp(Csq, irp);    // Remove this IRP from the queue

        break;
    }

    context = irp->Tail.Overlay.DriverContext[3];
    if (context->Type == IO_TYPE_CSQ_IRP_CONTEXT) {
        context->Irp = NULL;
        ASSERT(context->Csq == Csq);
    }

    irp->Tail.Overlay.DriverContext[3] = NULL;


    Csq->CsqReleaseLock(Csq, irql);

    return irp;
}

PIRP
CSQLIB_DDI(IoCsqRemoveIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIO_CSQ_IRP_CONTEXT Context
    )
/*++

Routine Description:

    This routine removes the IRP that's associated with a context from the queue.
    It's expected that this routine will be called from a timer or DPC or other threads which complete an
    IO. Note that the IRP associated with this context could already have been freed.

Arguments:

    Csq - Pointer to the cancel queue.
    Context - Context associated with Irp.


Return Value:

    Returns the IRP associated with the context. If the value is not NULL, the IRP was successfully
    retrieved and can be used safely. If the value is NULL, the IRP was already canceled.

--*/
{
    KIRQL   irql;
    PIRP    irp;
    PDRIVER_CANCEL  cancelRoutine;

    Csq->ReservePointer = NULL; // Force drivers to be good citizens

    Csq->CsqAcquireLock(Csq, &irql);

    if (Context->Irp ) {

        ASSERT(Context->Csq == Csq);

        irp = Context->Irp;


        cancelRoutine = IoSetCancelRoutine(irp, NULL);
        if (!cancelRoutine) {
            Csq->CsqReleaseLock(Csq, irql);
            return NULL;
        }

        ASSERT(Context == irp->Tail.Overlay.DriverContext[3]);

        Csq->CsqRemoveIrp(Csq, irp);

        //
        // Break the association.
        //

        Context->Irp = NULL;
        irp->Tail.Overlay.DriverContext[3] = NULL;

        ASSERT(Context->Csq == Csq);

        Csq->CsqReleaseLock(Csq, irql);

        return irp;

    } else {

        Csq->CsqReleaseLock(Csq, irql);
        return NULL;
    }
}

VOID
CSQLIB_DDI(IoCsqInsertIrp)(
    IN  PIO_CSQ             Csq,
    IN  PIRP                Irp,
    IN  PIO_CSQ_IRP_CONTEXT Context
    )
{
    (VOID)CSQLIB_DDI(IoCsqInsertIrpEx)(Csq, Irp, Context, NULL);
}

