/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    isorwr.h

Abstract:

Environment:

    Kernel mode

Notes:

    Copyright (c) 2004 Microsoft Corporation.
    All Rights Reserved.

--*/

#ifndef _ISOUSB_RWR_H
#define _ISOUSB_RWR_H


typedef struct _MAIN_REQUEST_CONTEXT
{
    // List of all sub requests (SUB_REQUEST_CONTEXT structures) that
    // are allocated to handle the original main request Irp.
    //
    LIST_ENTRY  SubRequestList;

} MAIN_REQUEST_CONTEXT, *PMAIN_REQUEST_CONTEXT;

// The MAIN_REQUEST_CONTEXT structure is overlaid on top of the main
// request Irp Tail.Overlay.DriverContext structure instead of being
// allocated separately.  Make sure it fits!
//
C_ASSERT(sizeof(MAIN_REQUEST_CONTEXT) <=
         sizeof(((PIRP)0)->Tail.Overlay.DriverContext));



typedef struct _SUB_REQUEST_CONTEXT
{
    // Main request Irp that caused this sub request to be allocated.
    //
    PIRP        MainIrp;

    // List entry that links this sub request into the main request Irp
    // context SubRequestList.  The sub request will be inserted into
    // the list as long as the sub request is outstanding.  The sub
    // request is only removed from the list by the sub request
    // completion routine.
    //
    LIST_ENTRY  ListEntry;

    // List entry that is used only by the main request Irp cancel
    // routine to build a list of all currently outstanding sub
    // requests in order to cancel them.
    //
    LIST_ENTRY  CancelListEntry;

    // Reference count incremented (set to one) before calling the sub
    // request down the driver stack, decremented by the sub request
    // completion routine, and incremented/decremented by the main
    // request Irp cancl routine.  This is used to prevent the sub
    // request from being freed by either the sub request completion
    // routine or the main request Irp cancel routine while the sub
    // request is simultaneously being accessed by the other routine.
    // The sub request is freed by the routine that last accesses the
    // sub request and decrements the reference count to zero.
    //
    LONG        ReferenceCount;

    // Irp, Urb, and Mdl allocated to send the sub request down the
    // driver stack.
    //
    PIRP        SubIrp;

    PURB        SubUrb;

    PMDL        SubMdl;

} SUB_REQUEST_CONTEXT, *PSUB_REQUEST_CONTEXT;


NTSTATUS
IsoUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
PerformFullSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    );

NTSTATUS
PerformHighSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    );

NTSTATUS
IsoUsb_SubRequestComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

VOID
IsoUsb_CancelReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

ULONG
IsoUsb_GetCurrentFrame(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

NTSTATUS
IsoUsb_StopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    );

#endif
