/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

    send.c

Abstract:

    NDIS protocol entry points and utility routines to handle sending
    data.

Environment:

    Kernel mode only.

Revision History:

    arvindm     4/10/2000    Created

--*/

#include "precomp.h"

#define __FILENUMBER 'DNES'




NTSTATUS
NdisProtWrite(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp
    )
/*++

Routine Description:

    Dispatch routine to handle IRP_MJ_WRITE. 

Arguments:

    pDeviceObject - pointer to our device object
    pIrp - Pointer to request packet

Return Value:

    NT status code.

--*/
{
    PIO_STACK_LOCATION      pIrpSp;
    ULONG                   DataLength;
    NTSTATUS                NtStatus;
    NDIS_STATUS             Status;
    PNDISPROT_OPEN_CONTEXT   pOpenContext;
    PNDIS_PACKET            pNdisPacket;
    PNDIS_BUFFER            pNdisBuffer;
    NDISPROT_ETH_HEADER UNALIGNED *pEthHeader;
#ifdef NDIS51
    PVOID                   CancelId;
#endif

    UNREFERENCED_PARAMETER(pDeviceObject);

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pOpenContext = pIrpSp->FileObject->FsContext;

    pNdisPacket = NULL;

    do
    {
        if (pOpenContext == NULL)
        {
            DEBUGP(DL_WARN, ("Write: FileObject %p not yet associated with a device\n",
                pIrpSp->FileObject));
            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }
               
        NPROT_STRUCT_ASSERT(pOpenContext, oc);

        if (pIrp->MdlAddress == NULL)
        {
            DEBUGP(DL_FATAL, ("Write: NULL MDL address on IRP %p\n", pIrp));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }
        //
        // Try to get a virtual address for the MDL.
        //
#ifndef WIN9X
        pEthHeader = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);

        if (pEthHeader == NULL)
        {
            DEBUGP(DL_FATAL, ("Write: MmGetSystemAddr failed for"
                    " IRP %p, MDL %p\n",
                    pIrp, pIrp->MdlAddress));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
#else
        pEthHeader = MmGetSystemAddressForMdl(pIrp->MdlAddress);   // for Win9X
#endif

        //
        // Sanity-check the length.
        //
        DataLength = MmGetMdlByteCount(pIrp->MdlAddress);
        if (DataLength < sizeof(NDISPROT_ETH_HEADER))
        {
            DEBUGP(DL_WARN, ("Write: too small to be a valid packet (%d bytes)\n",
                DataLength));
            NtStatus = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if (DataLength > (pOpenContext->MaxFrameSize + sizeof(NDISPROT_ETH_HEADER)))
        {
            DEBUGP(DL_WARN, ("Write: Open %p: data length (%d)"
                    " larger than max frame size (%d)\n",
                    pOpenContext, DataLength, pOpenContext->MaxFrameSize));

            NtStatus = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

        //
        // To prevent applications from sending packets with spoofed
        // mac address, we will do the following check to make sure the source 
        // address in the packet is same as the current MAC address of the NIC.
        //
        if ((pIrp->RequestorMode == UserMode) && 
            !NPROT_MEM_CMP(pEthHeader->SrcAddr, pOpenContext->CurrentAddress, NPROT_MAC_ADDR_LEN))
        {
            DEBUGP(DL_WARN, ("Write: Failing with invalid Source address"));
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }
                
        NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

        if (!NPROT_TEST_FLAGS(pOpenContext->Flags, NUIOO_BIND_FLAGS, NUIOO_BIND_ACTIVE))
        {
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            DEBUGP(DL_FATAL, ("Write: Open %p is not bound"
            " or in low power state\n", pOpenContext));

            NtStatus = STATUS_INVALID_HANDLE;
            break;
        }

        //
        //  Allocate a send packet.
        //
        NPROT_ASSERT(pOpenContext->SendPacketPool != NULL);
        NdisAllocatePacket(
            &Status,
            &pNdisPacket,
            pOpenContext->SendPacketPool);
        
        if (Status != NDIS_STATUS_SUCCESS)
        {
            NPROT_RELEASE_LOCK(&pOpenContext->Lock);

            DEBUGP(DL_FATAL, ("Write: open %p, failed to alloc send pkt\n",
                    pOpenContext));
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        //
        //  Allocate a send buffer if necessary.
        //
        if (pOpenContext->bRunningOnWin9x)
        {
            NdisAllocateBuffer(
                &Status,
                &pNdisBuffer,
                pOpenContext->SendBufferPool,
                pEthHeader,
                DataLength);

            if (Status != NDIS_STATUS_SUCCESS)
            {
                NPROT_RELEASE_LOCK(&pOpenContext->Lock);

                NdisFreePacket(pNdisPacket);

                DEBUGP(DL_FATAL, ("Write: open %p, failed to alloc send buf\n",
                        pOpenContext));
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }
        else
        {
            pNdisBuffer = pIrp->MdlAddress;
        }

        NdisInterlockedIncrement((PLONG)&pOpenContext->PendedSendCount);

        NPROT_REF_OPEN(pOpenContext);  // pended send

        IoMarkIrpPending(pIrp);

        //
        //  Initialize the packet ref count. This packet will be freed
        //  when this count goes to zero.
        //
        NPROT_SEND_PKT_RSVD(pNdisPacket)->RefCount = 1;

#ifdef NDIS51

        //
        //  NDIS 5.1 supports cancelling sends. We set up a cancel ID on
        //  each send packet (which maps to a Write IRP), and save the
        //  packet pointer in the IRP. If the IRP gets cancelled, we use
        //  NdisCancelSendPackets() to cancel the packet.
        //

        CancelId = NPROT_GET_NEXT_CANCEL_ID();
        NDIS_SET_PACKET_CANCEL_ID(pNdisPacket, CancelId);
        pIrp->Tail.Overlay.DriverContext[0] = (PVOID)pOpenContext;
        pIrp->Tail.Overlay.DriverContext[1] = (PVOID)pNdisPacket;

        NPROT_INSERT_TAIL_LIST(&pOpenContext->PendedWrites, &pIrp->Tail.Overlay.ListEntry);

        IoSetCancelRoutine(pIrp, NdisProtCancelWrite);

#endif // NDIS51

        NPROT_RELEASE_LOCK(&pOpenContext->Lock);

        //
        //  Set a back pointer from the packet to the IRP.
        //
        NPROT_IRP_FROM_SEND_PKT(pNdisPacket) = pIrp;

        NtStatus = STATUS_PENDING;

        pNdisBuffer->Next = NULL;
        NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);

#if SEND_DBG
        {
            PUCHAR      pData;

#ifndef WIN9X
            pData = MmGetSystemAddressForMdlSafe(pNdisBuffer, NormalPagePriority);
            NPROT_ASSERT(pEthHeader == pData);
#else
            pData = MmGetSystemAddressForMdl(pNdisBuffer);  // Win9x
#endif

            DEBUGP(DL_VERY_LOUD, 
                ("Write: MDL %p, MdlFlags %x, SystemAddr %p, %d bytes\n",
                    pIrp->MdlAddress, pIrp->MdlAddress->MdlFlags, pData, DataLength));

            DEBUGPDUMP(DL_VERY_LOUD, pData, MIN(DataLength, 48));
        }
#endif // SEND_DBG

        NdisSendPackets(pOpenContext->BindingHandle, &pNdisPacket, 1);

    }
    while (FALSE);

    if (NtStatus != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

    return (NtStatus);
}


#ifdef NDIS51

VOID
NdisProtCancelWrite(
    IN PDEVICE_OBJECT               pDeviceObject,
    IN PIRP                         pIrp
    )
/*++

Routine Description:

    Cancel a pending write IRP. This routine attempt to cancel the NDIS send.

Arguments:

    pDeviceObject - pointer to our device object
    pIrp - IRP to be cancelled

Return Value:

    None

--*/
{
    PNDISPROT_OPEN_CONTEXT       pOpenContext;
    PLIST_ENTRY                 pIrpEntry;
    PNDIS_PACKET                pNdisPacket;

    UNREFERENCED_PARAMETER(pDeviceObject);
    
    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    //
    //  The NDIS packet representing this Write IRP.
    //
    pNdisPacket = NULL;

    pOpenContext = (PNDISPROT_OPEN_CONTEXT) pIrp->Tail.Overlay.DriverContext[0];
    NPROT_STRUCT_ASSERT(pOpenContext, oc);

    //
    //  Try to locate the IRP in the pended write queue. The send completion
    //  routine may be running and might have removed it from there.
    //
    NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

    for (pIrpEntry = pOpenContext->PendedWrites.Flink;
         pIrpEntry != &pOpenContext->PendedWrites;
         pIrpEntry = pIrpEntry->Flink)
    {
        if (pIrp == CONTAINING_RECORD(pIrpEntry, IRP, Tail.Overlay.ListEntry))
        {
            pNdisPacket = (PNDIS_PACKET) pIrp->Tail.Overlay.DriverContext[1];

            //
            //  Place a reference on this packet so that it won't get
            //  freed/reused until we are done with it.
            //
            NPROT_REF_SEND_PKT(pNdisPacket);
            break;
        }
    }

    NPROT_RELEASE_LOCK(&pOpenContext->Lock);

    if (pNdisPacket != NULL)
    {
        //
        //  Either the send completion routine hasn't run, or we got a peak
        //  at the IRP/packet before it had a chance to take it out of the
        //  pending IRP queue.
        //
        //  We do not complete the IRP here - note that we didn't dequeue it
        //  above. This is because we always want the send complete routine to
        //  complete the IRP. And this in turn is because the packet that was
        //  prepared from the IRP has a buffer chain pointing to data associated
        //  with this IRP. Therefore we cannot complete the IRP before the driver
        //  below us is done with the data it pointed to.
        //

        //
        //  Request NDIS to cancel this send. The result of this call is that
        //  our SendComplete handler will be called (if not already called).
        //
        DEBUGP(DL_INFO, ("CancelWrite: cancelling pkt %p on Open %p\n",
            pNdisPacket, pOpenContext));
        NdisCancelSendPackets(
            pOpenContext->BindingHandle,
            NDIS_GET_PACKET_CANCEL_ID(pNdisPacket)
            );
        
        //
        //  It is now safe to remove the reference we had placed on the packet.
        //
        NPROT_DEREF_SEND_PKT(pNdisPacket);
    }
    //
    //  else the send completion routine has already picked up this IRP.
    //
}

#endif // NDIS51


VOID
NdisProtSendComplete(
    IN NDIS_HANDLE                  ProtocolBindingContext,
    IN PNDIS_PACKET                 pNdisPacket,
    IN NDIS_STATUS                  Status
    )
/*++

Routine Description:

    NDIS entry point called to signify completion of a packet send.
    We pick up and complete the Write IRP corresponding to this packet.

    NDIS 5.1: 

Arguments:

    ProtocolBindingContext - pointer to open context
    pNdisPacket - packet that completed send
    Status - status of send

Return Value:

    None

--*/
{
    PIRP                        pIrp;
    PIO_STACK_LOCATION          pIrpSp;
    PNDISPROT_OPEN_CONTEXT       pOpenContext;

    pOpenContext = (PNDISPROT_OPEN_CONTEXT)ProtocolBindingContext;
    NPROT_STRUCT_ASSERT(pOpenContext, oc);

    pIrp = NPROT_IRP_FROM_SEND_PKT(pNdisPacket);

    if (pOpenContext->bRunningOnWin9x)
    {
        //
        //  We would have attached our own NDIS_BUFFER. Take it out
        //  and free it.
        //
#ifndef NDIS51
        PNDIS_BUFFER                pNdisBuffer;
        PVOID                       VirtualAddr;
        UINT                        BufferLength;
        UINT                        TotalLength;
#endif

#ifdef NDIS51
        NPROT_ASSERT(FALSE); // NDIS 5.1 not on Win9X!
#else
        NdisGetFirstBufferFromPacket(
            pNdisPacket,
            &pNdisBuffer,
            &VirtualAddr,
            &BufferLength,
            &TotalLength);

        NPROT_ASSERT(pNdisBuffer != NULL);
        NdisFreeBuffer(pNdisBuffer);
#endif
    }


#ifdef NDIS51
    IoSetCancelRoutine(pIrp, NULL);

    NPROT_ACQUIRE_LOCK(&pOpenContext->Lock);

    NPROT_REMOVE_ENTRY_LIST(&pIrp->Tail.Overlay.ListEntry);

    NPROT_RELEASE_LOCK(&pOpenContext->Lock);
#endif

    //
    //  We are done with the NDIS_PACKET:
    //
    NPROT_DEREF_SEND_PKT(pNdisPacket);

    //
    //  Complete the Write IRP with the right status.
    //
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    if (Status == NDIS_STATUS_SUCCESS)
    {
        pIrp->IoStatus.Information = pIrpSp->Parameters.Write.Length;
        pIrp->IoStatus.Status = STATUS_SUCCESS;
    }
    else
    {
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    }

    DEBUGP(DL_INFO, ("SendComplete: packet %p/IRP %p/Length %d "
                    "completed with status %x\n",
                    pNdisPacket, pIrp, pIrp->IoStatus.Information, pIrp->IoStatus.Status));

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    NdisInterlockedDecrement((PLONG)&pOpenContext->PendedSendCount);

    NPROT_DEREF_OPEN(pOpenContext); // send complete - dequeued send IRP
}



