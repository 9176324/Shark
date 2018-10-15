/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       send.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

Author:

   A. Wang

Environment:

   Kernel mode

Revision History:

   09/23/96        kyleb       Fix map register freeing bug
                               Fix Deactivate VC complete bug.
   01/07/97        awang       Initial of Toshiba ATM 155 Device Driver.
   02/16/99        hhan        Use synchronize mode to allocate xmit buffer.
   03/03/97        hhan        Added in 'ScatterGetherDMA'.
--*/

#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_SEND

VOID
tbAtm155TransmitPacket(
	IN	PVC_BLOCK       pVc,
  	IN  PNDIS_PACKET    Packet
   )
/*++

Routine Description:

   This routine will set up the DMA operation.

   Assume: FreePadTrailerBuffers > 0

   - There are two ways to send a packet
     1. post the tx buffer(s) from NDIS directly to SAR.
     2. if Remaining slot are running low, copy tx data in buffer(s) to
        pre-allocated tx buffer and then post the pre-allocate buffer
        to SAR.

   - After post Tx data of the packet, ALWAYS post a PadTrailerBuffers
     for AAL5 packets.

Arguments:

Return Value:

--*/
{                           	
   PADAPTER_BLOCK              pAdapter = pVc->Adapter;
   PHARDWARE_INFO              pHwInfo = pVc->HwInfo;
   PTBATM155_SAR               TbAtm155Sar = pHwInfo->TbAtm155_SAR;
   PSAR_INFO                   pSar = pHwInfo->SarInfo;
   PXMIT_SEG_INFO              pXmitSegInfo = pVc->XmitSegInfo;
   PXMIT_DMA_QUEUE             pXmitDmaQ = &pSar->XmitDmaQ;
   PPACKET_RESERVED            Reserved = PACKET_RESERVED_FROM_PACKET(Packet);
   UINT                        PacketLength;
   UINT                        c;
   PMEDIA_SPECIFIC_INFORMATION pMediaInfo;
   ULONG                       SizeMediaInfo;
   PATM_AAL_OOB_INFO           pAtmOob;
   AAL5_PDU_TRAILER            Trailer;
   ULONG                       Padding = 0;
	PSCATTER_GATHER_LIST		psgl;
   
   TX_PENDING_SLOTS_CTL        TxPendingSlotCtrlReg;
   ULONG                       TxSlotNum = 0;
   PALLOCATION_MAP             pPadTrailerBuffer;

   dbgLogSendPacket(Reserved->pVc->DebugInfo, Packet, 0, 0, ' cdq');

   //
   //	Place the packet on the completing queue.
   //
   InsertPacketAtTail(&pXmitSegInfo->DmaCompleting, Packet);

   //
   //	Clear the trailer first.
   //
   ZERO_MEMORY(&Trailer, sizeof(AAL5_PDU_TRAILER));

   //
   //	Set defaults for the pdu trailer
   //
   Trailer.UserToUserIndication = 0;
   Trailer.CommonPartIndicator = 0;

   //
   //	Do we have any oob data in the packet?
   //
   NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(Packet, &pMediaInfo, &SizeMediaInfo);

   //
   //	If there is OOB data then it's already been verified.	
   //
   if (0 != SizeMediaInfo)
   {
       pAtmOob = (PATM_AAL_OOB_INFO)pMediaInfo->ClassInformation;

       //
       //	The contents depends upon the AAL type.
       //
       if (AAL_TYPE_AAL5 == pAtmOob->AalType)
       {
           Trailer.UserToUserIndication = pAtmOob->ATM_AAL5_INFO.UserToUserIndication;
           Trailer.CommonPartIndicator = pAtmOob->ATM_AAL5_INFO.CommonPartIndicator;
       }
   }

   psgl = NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ScatterGatherListPacketInfo);

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
            ("TbAtm155TransmitPacket: Pkt %x, psgl %x\n", Packet, psgl));

   //
   //  Copy the initial control register value of Tx_Pending_Slots
   //
   TxPendingSlotCtrlReg.reg = pXmitSegInfo->InitXmitSlotDesc.reg;

	for (c = 0; c < psgl->NumberOfElements; c++)
	{
       DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
           ("TbAtm155TransmitPacket: Pkt %x, elem %d, Length %d\n",
            Packet, c, psgl->Elements[c].Length));

       //
       //	Initialize the descriptor with the physical buffer info.
       //  and write to Tx_Pending_slots registers of 155 PCI SAR 
       //
       TxPendingSlotCtrlReg.Slot_Size = psgl->Elements[c].Length;

       TBATM155_WRITE_PORT(
               &TbAtm155Sar->Tx_Pending_Slots[TxSlotNum].Cntrl,
               TxPendingSlotCtrlReg.reg);

       TBATM155_WRITE_PORT(
               &TbAtm155Sar->Tx_Pending_Slots[TxSlotNum].Base_Addr,
               NdisGetPhysicalAddressLow(psgl->Elements[c].Address));

       //               
       //  Move to next slot registers for the next posting.
       //               
       TxSlotNum = (TxSlotNum + 1) % MAX_SLOTS_NUMBER;

   } // end of FOR

    //
    //	Update the total posted Tx pending slots.
    //
    Reserved->PhysicalBufferCount = (UCHAR)psgl->NumberOfElements;

    //
    //	Update the total remaining Tx free slots.
    //
    pXmitDmaQ->RemainingTransmitSlots -= psgl->NumberOfElements;



    //
	//	If this was an AAL5 packet then we need to
	//	copy out any padding and the AAL5 pdu trailer.
	//
	if (AAL_TYPE_AAL5 == pVc->AALType)
	{
		//
		//	Get the first packet buffer and the number of physical
		//	segments that comprise the packet.
		//
		NdisQueryPacket(Packet, NULL, NULL, NULL, &PacketLength);

       //
       //	Build the AAL5 trailer information.
       //
       Trailer.Length = TBATM155_SWAP_USHORT((USHORT)PacketLength);

       //
       //	Determine the amount of padding bytes.
       //
       pPadTrailerBuffer = &pXmitSegInfo->PadTrailerBuffers[pXmitSegInfo->PadTrailerBufferIndex];
                                                     
       DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO, ("Va=0x%lX\n", pPadTrailerBuffer->Va));

       //
       // copy Trailer to the buffer.
       //

       Padding = (ULONG)Reserved->PaddingBytesNeeded;

       NdisMoveMemory(
           (PUCHAR)pPadTrailerBuffer->Va + Padding, 
           (PVOID)&Trailer,
           (ULONG)sizeof(AAL5_PDU_TRAILER));
		
       //
       //  Let's to write the last fragment out
       //
       //	Initialize the descriptor with the physical buffer info.
       //  and write to Tx_Pending_slots registers of 155 PCI SAR 
		//
       TxPendingSlotCtrlReg.Slot_Size = Padding + sizeof(AAL5_PDU_TRAILER);
       TxPendingSlotCtrlReg.EOP = 1;

       TBATM155_WRITE_PORT(
           &TbAtm155Sar->Tx_Pending_Slots[TxSlotNum].Cntrl,
           TxPendingSlotCtrlReg.reg);

       TBATM155_WRITE_PORT(
           &TbAtm155Sar->Tx_Pending_Slots[TxSlotNum].Base_Addr,
           pPadTrailerBuffer->Pa);

       //
       //	Flush it out.
       //
       NdisFlushBuffer(
           pPadTrailerBuffer->FlushBuffer, 
           TRUE
       );

       Reserved->PadTrailerBufIndexInUse = pXmitSegInfo->PadTrailerBufferIndex;
       pXmitSegInfo->FreePadTrailerBuffers--;
       pXmitSegInfo->PadTrailerBufferIndex++;

       if(pXmitSegInfo->PadTrailerBufferIndex == TBATM155_MAX_PADTRAILER_BUFFERS)
       {
           pXmitSegInfo->PadTrailerBufferIndex = 0;
       }

       //
       //  Update the total posted Tx pending slots.
       //
       Reserved->PhysicalBufferCount++;

       //
       //	Update the total remaining Tx free slots.
       //
       pXmitDmaQ->RemainingTransmitSlots--;

   }

#if TB_CHK4HANG
   ADAPTER_SET_FLAG(pAdapter, fADAPTER_EXPECT_TXIOC);
#endif // end of TB_CHK4HANG

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("<==tbAtm155TransmitPacket\n"));
}


NDIS_STATUS
tbAtm155ProcessTransmitDmaWaitQueue(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PNDIS_PACKET    Packet,
   IN  BOOLEAN         fLimitedNoPkts
   )
/*++

Routine Description:

   This routine will either queue a packet to the transmit DMA queue or
   DMA the packet.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK
   Packet      -   if equal to NULL, this routine is called in
                   the interrupt service routine 
                   tbAtm155TxInterruptOnCompletion().
                   Otherwise, this routine is called from foreground.

Return Value:

   NDIS_STATUS_SUCCESS if the packet was successfully sent to the
                       transmit DMA engine.
   NDIS_STATUS_PENDING if it was queued waiting for DMA resources.

--*/
{
   PSAR_INFO           pSar = pAdapter->HardwareInfo->SarInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ = &pSar->XmitDmaQ;
   PNDIS_PACKET        tmpPacket;
   PPACKET_RESERVED    Reserved;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PXMIT_SEG_INFO      pXmitSegInfo;
   PVC_BLOCK           pVc;
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;

   //
   //	If a packet was not passed in to transmit then we need to
   //	grab them off of the pending queue.
   //
   NdisAcquireSpinLock(&pXmitDmaQ->lock);

   if (ARGUMENT_PRESENT(Packet))
   {
       Reserved = PACKET_RESERVED_FROM_PACKET(Packet);
       pVc = Reserved->pVc;
       pXmitSegInfo = pVc->XmitSegInfo;

	    if (AAL_TYPE_AAL5 == pVc->AALType)
       {
           NdisAcquireSpinLock(&pXmitSegInfo->lock);
           pXmitSegInfo->BeOrBeingUsedPadTrailerBufs++;
           NdisReleaseSpinLock(&pXmitSegInfo->lock);
       }

       //
       //	Are there transmit DMA resources available?
       //
       if (pXmitDmaQ->RemainingTransmitSlots < Reserved->PhysicalBufferCount)
       {
           //
           //  Too bad, there is no resources available.
           //  Queue to DmaWait list to handle later.
           //
           dbgLogSendPacket(pVc->DebugInfo, Packet, 1, 0, ' wdq');

           InsertPacketAtTail(&pXmitDmaQ->DmaWait, Packet);

           NdisReleaseSpinLock(&pXmitDmaQ->lock);

           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("<==tbAtm155ProcessTransmitDmaWaitQueue(NDIS_STATUS_PENDING)\n"));

           return(NDIS_STATUS_PENDING);
           
       }

       //
       //	Are there any other packets waiting for the DMA engine?
       //
       if (!PACKET_QUEUE_EMPTY(&pXmitDmaQ->DmaWait))
       {
           //
           //	There are other packets waiting for the available DMA
           //	resources so this guy goes to the end of the list.
           //
           dbgLogSendPacket(pVc->DebugInfo, Packet, 2, 0, ' wdq');
           InsertPacketAtTail(&pXmitDmaQ->DmaWait, Packet);
           Status = NDIS_STATUS_PENDING;
       }
       else
       {
           //
           //	Setup the packet for DMA if there are available
           //  free PadTrailerBuffers.
           //
           tbAtm155TransmitPacket(Reserved->pVc, Packet);
       }
   }

   //
   //	Loop through the packets that are waiting for dma resources.
   //
   while (!PACKET_QUEUE_EMPTY(&pXmitDmaQ->DmaWait))
   {
       //
       //	Are there enough resources to process the
       //	packet?
       //
       Reserved = PACKET_RESERVED_FROM_PACKET(pXmitDmaQ->DmaWait.Head);
       pXmitSegInfo = Reserved->pVc->XmitSegInfo;

       if (pXmitSegInfo->FreePadTrailerBuffers == 0)
       {
           //
           // Bad!! We should never get here.
           //
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_FATAL,
               ("ProcessTxDmaWaitQueue: BAD!!(UsedPTB=%u, VC=%lx)\n",
               pXmitSegInfo->BeOrBeingUsedPadTrailerBufs, Reserved->pVc->VpiVci.Vci));

           break;                      // no chance, get out.
       }

       if (pXmitDmaQ->RemainingTransmitSlots < Reserved->PhysicalBufferCount)
       {
           break;                  // no chance, get out.
       }

       dbgLogSendPacket(Reserved->pVc->DebugInfo, Packet, 0, 0, 'WDQD');
       RemovePacketFromHead(&pXmitDmaQ->DmaWait, &tmpPacket);

       //
       //	Transmit the packet.
       //
       tbAtm155TransmitPacket(Reserved->pVc, tmpPacket);

       if (TRUE == fLimitedNoPkts)
       {
           TBATM155_WRITE_PORT(
               &pHwInfo->TbAtm155_SAR->Intr_Status,
               0);

           TBATM155_WRITE_PORT(
               &pHwInfo->TbAtm155_SAR->Intr_Status,
               0);
       }

   }

   NdisReleaseSpinLock(&pXmitDmaQ->lock);
   return(Status);
}



NDIS_STATUS
tbAtm155ProcessSegmentQueue(
   IN  PXMIT_SEG_INFO      pXmitSegInfo,
   IN  PNDIS_PACKET        Packet
	)
/*++

Routine Description:

   This routine will do one of two things depending on whether or not a valid
   Packet was passed in.  If a packet is passed in then we will determine if
   there are enough resources for segmentation of this packet.  If not then
   we will process the queue of packets that are waiting for segmentation
   resources.

Arguments:

   pXmitSegInfo    -   Pointer to the segmentation information to process
                       packets for.
   Packet          -	Pointer to the packet to acquire segmentation resources
                       for or NULL if we are to process the pending queue.

Return Value:

   NDIS_STATUS_SUCCESS if the packet was successfully sent to the DMA engine.
   NDIS_STATUS_PENDING if the packet was queued some where due to resource
                       constraints.
--*/
{
   PNDIS_PACKET        tmpPacket;
   PPACKET_RESERVED    Reserved;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PSAR_INFO           pSar = pXmitSegInfo->Adapter->HardwareInfo->SarInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ = &pSar->XmitDmaQ;


   NdisAcquireSpinLock(&pXmitSegInfo->lock);

   //
   //	Are they trying to send a specific packet?
   //
   if (ARGUMENT_PRESENT(Packet))
   {
       Reserved = PACKET_RESERVED_FROM_PACKET(Packet);

       //
       //	1. If there is segmentation memory available? 
       //	2. If there is FreePadTrailerBuffers available? 
       //
       if ((pXmitDmaQ->RemainingTransmitSlots < Reserved->PhysicalBufferCount) ||
           (pXmitSegInfo->BeOrBeingUsedPadTrailerBufs >= TBATM155_MAX_PADTRAILER_BUFFERS))
       {

           // 
           //  Insert the packet to DmaWait queue if
           //  1. there is allocated FreePadTrailerBuffers available,
           //  2. AAL5 packet, and
           //  3. SegWait queue of the VC is empty.
           //
           if ((pXmitSegInfo->BeOrBeingUsedPadTrailerBufs < TBATM155_MAX_PADTRAILER_BUFFERS) &&
	            (AAL_TYPE_AAL5 == Reserved->pVc->AALType) &&
               (PACKET_QUEUE_EMPTY(&pXmitSegInfo->SegWait)))
           {
               //
               // If we come to this point, it means there is no packets
               // of this VC have been sent out. So we queue to DmaWait
               // to make sure this packet will be sent out when there
               // are resources available.
               //
               InsertPacketAtTailDpc(&pXmitDmaQ->DmaWait, Packet);
               pXmitSegInfo->BeOrBeingUsedPadTrailerBufs++;

               NdisReleaseSpinLock(&pXmitSegInfo->lock);

               DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                   ("<==tbAtm155ProcessSegmentQueue(NDIS_STATUS_PENDING_1)\n"));

               return(NDIS_STATUS_PENDING);
           }

           if ((pXmitSegInfo->BeOrBeingUsedPadTrailerBufs >= TBATM155_MAX_PADTRAILER_BUFFERS) ||
               (pXmitDmaQ->RemainingTransmitSlots < MIN_SLOTS_REQUIED_PER_PACKET))
           {
               //
               //	No room available in the SAR. We need to queue this
               //	and do it later.
               //
               dbgLogSendPacket(
                   PACKET_RESERVED_FROM_PACKET(Packet)->pVc->DebugInfo,
                   Packet,
                   2,
                   0,
                   ' wsQ');

               InsertPacketAtTailDpc(&pXmitSegInfo->SegWait, Packet);
               NdisReleaseSpinLock(&pXmitSegInfo->lock);

               DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                   ("<==tbAtm155ProcessSegmentQueue(NDIS_STATUS_PENDING)\n"));

               return(NDIS_STATUS_PENDING);
           }
       }

       //
       //	Are there any other packets that might be waiting for these
       //	resources?
       //
       if (PACKET_QUEUE_EMPTY(&pXmitSegInfo->SegWait) &&
           (pXmitSegInfo->BeOrBeingUsedPadTrailerBufs < TBATM155_MAX_PADTRAILER_BUFFERS))
       {
           //
           //	Packet queue is empty so we can try and send this packet to
           //	the transmit DMA engine.
           //             
           NdisReleaseSpinLock(&pXmitSegInfo->lock);

           dbgLogSendPacket(Reserved->pVc->DebugInfo, Packet, 0, 0, 'qwdp');

           tbAtm155ProcessTransmitDmaWaitQueue(
               pXmitSegInfo->Adapter,
               Packet,
               FALSE);

           dbgLogSendPacket( Reserved->pVc->DebugInfo, Packet, 0, 0, 'QWDP');

           NdisAcquireSpinLock(&pXmitSegInfo->lock);
           Status = NDIS_STATUS_SUCCESS;
       }
       else
       {
           //
           //	Packet queue is not empty so we need to place this packet at
           //	the end of the queue then we can process the packet queue from
           //	the beginning (remember we have already determined that we have
           //	resources in the segmentation memory).
           //
           dbgLogSendPacket(
               PACKET_RESERVED_FROM_PACKET(Packet)->pVc->DebugInfo,
               Packet,
               3,
               0,
               ' wsQ');

           InsertPacketAtTailDpc(&pXmitSegInfo->SegWait, Packet);
           Status = NDIS_STATUS_PENDING;
       }
   }
   else
   {
       //
       //	Are there any packets to process?
       //
       if (PACKET_QUEUE_EMPTY(&pXmitSegInfo->SegWait))
       {
           // No, the queue is empty.
           NdisReleaseSpinLock(&pXmitSegInfo->lock);

           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
               ("<==tbAtm155ProcessSegmentQueue(NDIS_STATUS_SUCCESS)\n"));

           return(NDIS_STATUS_SUCCESS);
       }
   }

   //
   //	Process the segmentation wait queue.
   //
   while(!PACKET_QUEUE_EMPTY(&pXmitSegInfo->SegWait))
   {
       //
       //	Determine if we have the segmentation resources necessary
       //	to process the packet.
       //
       Reserved = PACKET_RESERVED_FROM_PACKET(pXmitSegInfo->SegWait.Head);

       if (pXmitDmaQ->RemainingTransmitSlots < Reserved->PhysicalBufferCount)
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR, ("Not enough resources.\n"));
           break;
       }

       if ((AAL_TYPE_AAL5 == Reserved->pVc->AALType) &&
           (pXmitSegInfo->BeOrBeingUsedPadTrailerBufs >= TBATM155_MAX_PADTRAILER_BUFFERS))
       {
           //
           // All of padtrailerbuffer have been reserved.
           // Don't dequeue this packet from the Segwait queue.
           //
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO, ("no PadTrailerBuffers.\n"));
           break;
       }
		
       //
       //	We have enough resources, remove the packet from the waiting
       //	queue and note the resources that it will need.
       //
       dbgLogSendPacket(Reserved->pVc->DebugInfo, Packet, 1, 0, 'wsQD');

       RemovePacketFromHead(&pXmitSegInfo->SegWait, &tmpPacket);

       NdisReleaseSpinLock(&pXmitSegInfo->lock);

       dbgLogSendPacket(Reserved->pVc->DebugInfo, tmpPacket, 0, 0, 'qwdp');

       tbAtm155ProcessTransmitDmaWaitQueue(
           pXmitSegInfo->Adapter,
           tmpPacket,
           FALSE);

       dbgLogSendPacket(Reserved->pVc->DebugInfo, tmpPacket, 0, 0, 'QWDP');

       NdisAcquireSpinLock(&pXmitSegInfo->lock);
   }

   NdisReleaseSpinLock(&pXmitSegInfo->lock);

   return(Status);
}


VOID
tbAtm155QueuePacketsToDmaWaitQueue(
   IN  PXMIT_SEG_INFO      pXmitSegInfo
	)

/*++

Routine Description:

   This routine will determine if there are enough resources to transmit
   the waiting queue of this VC.
   If there are enough resources, packets are moved from SegWait queue
   and insert to DmaWait queue to wait to be transmitted later.
   This routine only queues waiting packets to DmaWait queue.


Arguments:

   pXmitSegInfo    -   Pointer to the segmentation information to process
                       packets for.

Return Value:

--*/
{
   PSAR_INFO           pSar = pXmitSegInfo->Adapter->HardwareInfo->SarInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ = &pSar->XmitDmaQ;
   PPACKET_RESERVED    Reserved;
	PNDIS_PACKET		tmpPacket;
   ULONG               CurrentRemainingTxSlots;


   NdisAcquireSpinLock(&pXmitSegInfo->lock);

   CurrentRemainingTxSlots = pSar->XmitDmaQ.RemainingTransmitSlots;

   //
   //	Process the segmentation wait queue.
   //
   while (!PACKET_QUEUE_EMPTY(&pXmitSegInfo->SegWait))
   {
       //
       //	Determine if we have the segmentation resources necessary
       //	to process the packet.
       //
       Reserved = PACKET_RESERVED_FROM_PACKET(pXmitSegInfo->SegWait.Head);

       if (CurrentRemainingTxSlots < (ULONG)Reserved->PhysicalBufferCount)
       {
           // we don't have enough resources to process the packet.
           break;
       }
		
       dbgLogSendPacket(Reserved->pVc->DebugInfo, tmpPacket, 0, 0, 'qptd');

       //
       //	Are there available FreePadTrailerBuffers for padding & trailer?
       //
       if (pXmitSegInfo->BeOrBeingUsedPadTrailerBufs >= TBATM155_MAX_PADTRAILER_BUFFERS)
       {
           //
           //  Too bad, there is no resources available for this Vc.
           //
	        break;
       }

       //
       //  Remove from SegWait queue.
       //
       RemovePacketFromHead(&pXmitSegInfo->SegWait, &tmpPacket);

       //
       //  Insert into DmaWait queue to be transmitted.
       //
       InsertPacketAtTail(&pXmitDmaQ->DmaWait, tmpPacket);

       CurrentRemainingTxSlots -= (ULONG)Reserved->PhysicalBufferCount;
       pXmitSegInfo->BeOrBeingUsedPadTrailerBufs++;

       dbgLogSendPacket(Reserved->pVc->DebugInfo, tmpPacket, 0, 0, 'qptd');

   }   // end of while loop

   NdisReleaseSpinLock(&pXmitSegInfo->lock);
}


VOID
tbAtm155TxInterruptOnCompletion(
   IN  PADAPTER_BLOCK		pAdapter
	)
/*++

Routine Description:

   This routine will process the Transmit Interrupt On Completion
   for the different transmit buffers.  This interrupt will be generated
   when a packet has been sent out from a Tx Active Slot list of a VC.

Arguments:

pAdapter   -   Pointer to the adapter block.

Return Value:

   None.

--*/
{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   PSAR_INFO               pSar = pHwInfo->SarInfo;
   PXMIT_DMA_QUEUE         pXmitDmaQ = &pSar->XmitDmaQ;
   PPACKET_RESERVED        Reserved;
   PNDIS_PACKET            Packet;
   PVC_BLOCK               pVc;
   PTX_REPORT_QUEUE        pTxReportQ = &pAdapter->TxReportQueue;
   TX_REPORT_PTR           TxReportPtrReg;
   ULONG                   CurrentTxReportPtrPa;
   UINT                    CurrentTxReportQIdx;
   UINT                    i;
   PTX_REPORT_QUEUE_ENTRY  TxReportQueue;
	ULONG					Vci;
   PXMIT_SEG_INFO          pXmitSegInfo;


   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("==>TbAtm155TxInterruptOnCompletion\n"));

   NdisDprAcquireSpinLock(&pXmitDmaQ->lock);

   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->Tx_Report_Ptr,
       &TxReportPtrReg.reg);

   //
   //  Got the position of 155 PCI SAR's internal Tx report queue.
   //  This pointer indicates the address in host memory to whcih 155 SAR
   //  will write the next Tx report. So we always move the pointer
   //  (or index) to the previous entry to make sure the entry in the
   //  report queue is valid.
   //
   CurrentTxReportPtrPa = TxReportPtrReg.reg & ~0x3;

   //
   //  Calcuate the index of Tx Report Queue which is pointed by controller.
   //  Save the index to database for XmitDmaQ.
   //  Handling Rules:
   //      1. Only handle up to the entry before this one.
   //      2. Each entry of report queue indicates of a transmitted packet.
   //
   CurrentTxReportQIdx =
           (CurrentTxReportPtrPa - 
           NdisGetPhysicalAddressLow(pTxReportQ->TxReportQptrPa)) /
           sizeof(TX_REPORT_QUEUE_ENTRY);

   //
   //  Get the index handled last time.
   //
   TxReportQueue = (PTX_REPORT_QUEUE_ENTRY)pTxReportQ->TxReportQptrVa;

   DBGPRINT(DBG_COMP_TXIOC, DBG_LEVEL_ERR, 
            ("TxIOC CurTxRQIdx(%x), PrevTxRQIdx(%x)\n", 
            CurrentTxReportQIdx, pXmitDmaQ->PrevTxReportQIndex));

   for (i = pXmitDmaQ->PrevTxReportQIndex; 
        i != CurrentTxReportQIdx;
        i = (i == pHwInfo->MaxIdxOfTxReportQ)? 0 : ++i)
   {
       //
       //  Get the VC of the packet been sent out.
       //
       Vci = (ULONG) TxReportQueue[i].TxReportQDWord.VC;

       //
       //  There are more packets have been sent out.
       //  Let's get information from the queue.
       //
       pVc = tbAtm155UnHashVc(pAdapter, Vci);
       if ((NULL == pVc) || (NULL == pVc->XmitSegInfo))
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("There is no VC activated for: %u\n", Vci));

           continue;
       }

       if (!VC_TEST_FLAG(pVc, fVC_ACTIVE) && !VC_TEST_FLAG(pVc, fVC_DEACTIVATING))
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("The VC is activated for: %u\n", Vci));

           continue;
       }
       
       pXmitSegInfo = pVc->XmitSegInfo;

       NdisDprAcquireSpinLock(&pXmitSegInfo->lock);

       //
       //	Are there any packets to complete?
       //
       if (PACKET_QUEUE_EMPTY(&pXmitSegInfo->DmaCompleting))
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
           ("There is no any packets to complete of VC: %lx\n", Vci));

           NdisDprReleaseSpinLock(&pXmitSegInfo->lock);
           continue;
       }

       //
       //  Get VC information from VcHashTable based on the VC reported
       //
       Reserved = PACKET_RESERVED_FROM_PACKET(pVc->XmitSegInfo->DmaCompleting.Head);

       //
       // A packet has been sent out.
       //
       RemovePacketFromHead(&pXmitSegInfo->DmaCompleting, &Packet);



       if (Reserved->PadTrailerBufIndexInUse != 0x0FF)
       {
           pXmitSegInfo->FreePadTrailerBuffers++;

           //
           // Update the number of padtrailerbuffers to allow queuing 
           // more packets of this VC to the "dmawait" queue later.
           //
           pXmitSegInfo->BeOrBeingUsedPadTrailerBufs--;

       }

       NdisDprReleaseSpinLock(&pXmitSegInfo->lock);  

       pXmitDmaQ->RemainingTransmitSlots += (ULONG) Reserved->PhysicalBufferCount;

       dbgLogSendPacket(
           pVc->DebugInfo,
           Packet,
           0,
           0,
           'pmoc');

       //
       //	Complete the packet back to the protocol.
       //  Since we always return STATUS_PENDING to the protocol's
       //  send request, this function need to be called here.
       //

       //
       // Release spinlock before calling NdisMCoSendComplete to avoid a 
       // deadlock problem. After NdisMCoSendComplete is finished, 
       // re-aquire spinlock.
       //

       NdisDprReleaseSpinLock(&pXmitDmaQ->lock);

       NdisMCoSendComplete(
           NDIS_STATUS_SUCCESS,
           pVc->NdisVcHandle,
           Packet);

	    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
		    ("TbAtm155TxInterruptOnCompletion: Pkt %x\n", Packet));

	    DBGPRINT(DBG_COMP_TXIOC, DBG_LEVEL_INFO,
		    ("TbAtm155TxIOC: Pkt %x VC:%lx(OK)\n", Packet, pVc->VpiVci.Vci));

       NdisDprAcquireSpinLock(&pXmitDmaQ->lock);

       //
       //  Check if any packets are waiting in the
       //  waiting segment queue, SegWait, of the VC.
       //
       tbAtm155QueuePacketsToDmaWaitQueue(pXmitSegInfo);
       NdisDprAcquireSpinLock(&pVc->lock);
       NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.XmitPdusOk);
       pVc->StatInfo.XmitPdusOk++;

       //
       //	Dereference the VC's reference count.
       //
       //	If we only have the creation reference and the VC
       //	is waiting to be deactivated then we are done here....
       //
       tbAtm155DereferenceVc(pVc);

       if ((1 == pVc->References) && (VC_TEST_FLAG(pVc, fVC_DEACTIVATING)))
       {
           NdisDprReleaseSpinLock(&pVc->lock);
           NdisDprReleaseSpinLock(&pXmitDmaQ->lock);

           //
           //	This VC is finished.
           //
           TbAtm155DeactivateVcComplete(pVc->Adapter, pVc);
           NdisDprAcquireSpinLock(&pXmitDmaQ->lock);
		}
       else
       {
           NdisDprReleaseSpinLock(&pVc->lock);
       }

   } // end of FOR

   //
   //  Update to handled index of Tx report queue.
   //
   pXmitDmaQ->PrevTxReportQIndex = CurrentTxReportQIdx;

   DBGPRINT(DBG_COMP_SEND_ERR, DBG_LEVEL_ERR,
       ("RemainingXmitSlots 0x%lx\n", pXmitDmaQ->RemainingTransmitSlots));

   NdisDprReleaseSpinLock(&pXmitDmaQ->lock);

   //
   //	Send packets out from DmaWait queues.
   //
   tbAtm155ProcessTransmitDmaWaitQueue(pAdapter, NULL, TRUE);

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("<==TbAtm155TxInterruptOnCompletion\n"));

}


VOID
TbAtm155SendPackets(
   IN  NDIS_HANDLE     MiniportVcContext,
   IN  PPNDIS_PACKET   PacketArray,
   IN  UINT            NumberOfPackets
   )
/*++

Routine Description:

   This routine is the entry point to send packets on this miniport.

Arguments:

   MiniportVcContext   -   This is the pointer to our VC_BLOCK.
   PacketArray         -   Array of PNDIS_PACKETS to send on a given VC.
   NumberOfPackets     -   Number of packets in the PacketArray.

Return Value:

   Status is set for each packet in the PacketArray.

--*/
{
   PVC_BLOCK           pVc = (PVC_BLOCK)MiniportVcContext;
   UINT                c;
   PPNDIS_PACKET       CurrentPkt;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   ULONG               VcReferences;
   PNDIS_PACKET        pErrorPacket = NULL;
   PNDIS_PACKET        pNextErrorPacket = NULL;
   BOOLEAN             fDeactivationComplete = FALSE;
   NDIS_HANDLE         NdisVcHandle;
   PADAPTER_BLOCK      pAdapter = pVc->Adapter;
#if DBG
   ULONG               dbgVcFlags;
#endif // end of DBG   


   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("==>TbAtm155SendPackets\n"));

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("PacketArray: 0x%lx, NumberOfPackets: %ld\n", PacketArray, NumberOfPackets));

   do {

       NdisAcquireSpinLock(&pAdapter->lock);
       //
       // Set this flag to prevent handling reset while we are processing send packets. 
       //
       ADAPTER_SET_FLAG(pAdapter, fADAPTER_PROCESSING_SENDPACKETS);
       NdisReleaseSpinLock(&pAdapter->lock);


       NdisAcquireSpinLock(&pVc->lock);
       NdisVcHandle = pVc->NdisVcHandle;

       //
       //	Is the adapter resetting?
       //
#if DBG
       dbgVcFlags = pVc->Flags;
#endif // end of DBG   


       if (VC_TEST_FLAG(pVc, fVC_RESET_IN_PROGRESS) ||
           !VC_TEST_FLAG(pVc, (fVC_ACTIVE | fVC_TRANSMIT)) ||
           ADAPTER_TEST_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE))
       {
#if DBG
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR, ("pVc(%lx), Flags(%lx), dbgVcFlags(%lx)\n", 
                    pVc->VpiVci.Vci, pVc->Flags, dbgVcFlags));
#endif // end of DBG   

           //
           //	What status should we complete the packet with?
           //
#if TB_CHK4HANG
           
           Status = NDIS_STATUS_RESET_IN_PROGRESS;
           if (!VC_TEST_FLAG(pVc, (fVC_ACTIVE | fVC_TRANSMIT)))
           {
               Status = NDIS_STATUS_FAILURE;
           }
#else
           Status = VC_TEST_FLAG(pVc, fVC_RESET_IN_PROGRESS) ?
                       NDIS_STATUS_RESET_IN_PROGRESS : NDIS_STATUS_FAILURE;
#endif // end of TB_CHK4HANG

           NdisReleaseSpinLock(&pVc->lock);

			//
			//	Complete the packets.
			//
			for (c = 0, CurrentPkt = PacketArray;
			    (c < NumberOfPackets);
			    c++, CurrentPkt++)
			{
			    NDIS_SET_PACKET_STATUS(PacketArray[c], Status);

			    //
			    //  Place the packet on the error list. If pErrorPacket is NULL
			    //  it will initialize the Tail of the Linked list.
			    //	
			    PACKET_RESERVED_FROM_PACKET(*CurrentPkt)->Next = pErrorPacket;
				
			    pErrorPacket = *CurrentPkt;
			}
	
			DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
			    ("tbAtm155SendPackets: Failing due to invalid VC(%lx) state.\n",
			    pVc->VpiVci.Vci));

			DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
			    ("<==TbAtm155SendPackets\n"));
	
			break;
       }

       //
       //	Reference the VC so that it doesn't go away while we are sending.
       //	We don't need the VC_BLOCK lock acquired for this whole routine.
       //
       tbAtm155ReferenceVc(pVc);

       //
       //	Add references for all submitted packets. In case we fail one or more
       //	packets, we'll dereference those later.
       //
       //	We must add these references right here, because we may encounter
       //	send completion interrupts before this routine finishes, and those
       //	can cause the VC to be dereferenced away.
       //
       pVc->References += NumberOfPackets;
	
       NdisReleaseSpinLock(&pVc->lock);

       //
       //	We reference the VC block for every packet that gets queued.
       //
       VcReferences = 0;

       //
       //	Start processing the packet array.
       //
       for (c = 0, CurrentPkt = PacketArray;
            c < NumberOfPackets;
            c++, CurrentPkt++)
       {
           PNDIS_BUFFER                Buffer;
           UINT                        PacketLength;
           UINT                        PhysicalBufferCount;
           UINT                        BufferCount;
           USHORT                      DataSize;
		    USHORT						DataSizeMod;
           UCHAR                       NumOfPaddingBytes;
           PPACKET_RESERVED            Reserved;
           PMEDIA_SPECIFIC_INFORMATION pMediaInfo;
           ULONG                       SizeMediaInfo;
           PATM_AAL_OOB_INFO           pAtmOob;

			Reserved = PACKET_RESERVED_FROM_PACKET(*CurrentPkt);
			Reserved->Next = NULL;

           //
           //	If there is any OOB data then it had better be correct.
           //
           NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(*CurrentPkt, &pMediaInfo, &SizeMediaInfo);
           if ((0 != SizeMediaInfo) && (NULL != pMediaInfo))
           {
               pAtmOob = (PATM_AAL_OOB_INFO)pMediaInfo->ClassInformation;

               //
               //	We have media specific information.
               //	verify the type, size, and AAL type it is for.
               //
               if ((NdisClassAtmAALInfo != pMediaInfo->ClassId) ||
                   (sizeof(ATM_AAL_OOB_INFO) != pMediaInfo->Size) ||
                   (pVc->AALType != ((PATM_AAL_OOB_INFO)pMediaInfo->ClassInformation)->AalType))
               {
                   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
                       ("Packet contains OOB data that is invalid!\n"));

                   NDIS_SET_PACKET_STATUS(*CurrentPkt, NDIS_STATUS_FAILURE);

                   //
                   //	Place the packet on the error list.
                   //
                   if (NULL != pErrorPacket)
                   {
                       Reserved->Next = pErrorPacket;
                   }

                   pErrorPacket = *CurrentPkt;

                   continue;
               }
           }

           //
           //	Get the information that we need from the packet.
           //
           NdisQueryPacket(
               *CurrentPkt,
               &PhysicalBufferCount,
               &BufferCount,
               &Buffer,
               &PacketLength);

           //
           //	If this is an AAL5 transmission then we need to
           //	determine if any padding is needed.  This must be on a
           //	cell granularity.
           //
           if (AAL_TYPE_AAL5 == pVc->AALType)
           {
               if (MAX_AAL5_PDU_SIZE < (ULONG)PacketLength)
               {
                   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
                       ("PacketLength is larger than SegmentSize.\n"));

                   NDIS_SET_PACKET_STATUS(*CurrentPkt, NDIS_STATUS_FAILURE);
                   //
                   //	Place the packet on the error list.
                   //
                   if (NULL != pErrorPacket)
                   {
                       Reserved->Next = pErrorPacket;
                   }

                   pErrorPacket = *CurrentPkt;
				    continue;
               }

               //
               //  For AAL5, we need to calculate the padding bytes.
               //
               DataSize = (USHORT)PacketLength;
               DataSizeMod = DataSize % CELL_PAYLOAD_SIZE;

               if (DataSizeMod > 40)
               {
                   NumOfPaddingBytes  = (UCHAR)(96 - DataSizeMod);
               }
               else
               {
                   NumOfPaddingBytes  = (UCHAR)(48 - DataSizeMod);
               }

               //
               //  Save the padding bytes for this AAL5 packet.
               //
               NumOfPaddingBytes -= sizeof(AAL5_PDU_TRAILER);

               DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                   ("NumOfPaddingBytes: 0x%lx, PacketLength: 0x%lx\n",
                   NumOfPaddingBytes, PacketLength));

               //
               //	We also need an extra buffer for the AAL5 PDU trailer.
               //
               PhysicalBufferCount++;

           }
           else
           {
               // 
               //  There is no padding byte needed for AAL5 packet.
               //
               NumOfPaddingBytes = 0;
           }

           //
           //	Save the amount of segmentation room, number of physical buffers,
           //	and the pointer to the VC_BLOCK that is need for the transmit of
           //	the current packet in our packet reserved section.
           //
           //  Initialize the database of chained reserved packet.
           //
           Reserved->PaddingBytesNeeded = NumOfPaddingBytes;
           Reserved->PhysicalBufferCount = (UCHAR)PhysicalBufferCount;
           Reserved->PadTrailerBufIndexInUse = 0x0FF;  // the buffer is not used.
           Reserved->pVc = pVc;
	
           //
           //	The packet is always pending.
           //
           NDIS_SET_PACKET_STATUS(*CurrentPkt, NDIS_STATUS_PENDING);

    	    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
		        ("TbAtm155SendPacketPending: Pkt %x\n", *CurrentPkt));
           //
           //	We are going to keep the packet so we can increment the reference
           //	count on the VC.
           //
           VcReferences++;

           //
           //	Send the packet to the segmentation engine.  This routine will
           //	continue sending the packet on if there are card resources
           //	available.
           //
           dbgLogSendPacket(pVc->DebugInfo, *CurrentPkt, 0, 0, ' qsp');
           tbAtm155ProcessSegmentQueue(pVc->XmitSegInfo, *CurrentPkt);
       }

       //
       //	Add references to the VC_BLOCK for every packet that was queued.
       //
       NdisAcquireSpinLock(&pVc->lock);

       //
       //	Fix the references to reflect the packets that we did not
       //	queue for sending.
       //
       ASSERT(VcReferences <= NumberOfPackets);
       pVc->References -= (NumberOfPackets - VcReferences);

		//
		//	Remove the reference for the send path.
		//
       tbAtm155DereferenceVc(pVc);
       if ((1 == pVc->References) && (VC_TEST_FLAG(pVc, fVC_DEACTIVATING)))
       {
           //
           //	This VC is finished. Don't complete deactivation right now,
           //	but set a flag and do it below. This is so that we complete
           //	any errored packets before the deactivate-complete.
           //
           fDeactivationComplete = TRUE;
       }

       NdisReleaseSpinLock(&pVc->lock);

	} while (FALSE);

	//
	//	Did any packets error out?
	//
	while (NULL != pErrorPacket)
	{
		pNextErrorPacket = PACKET_RESERVED_FROM_PACKET(pErrorPacket)->Next;

		NdisInterlockedIncrement((PLONG)&pVc->Adapter->StatInfo.XmitPdusError);
		NdisInterlockedIncrement((PLONG)&pVc->StatInfo.XmitPdusError);

       NdisMCoSendComplete(
           NDIS_GET_PACKET_STATUS(pErrorPacket),
           NdisVcHandle,
           pErrorPacket);

	    DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
		    ("TbAtm155SendPacketError: Pkt %x\n", pErrorPacket));

		pErrorPacket = pNextErrorPacket;
	}

	if (fDeactivationComplete)
	{
       TbAtm155DeactivateVcComplete(pVc->Adapter, pVc);
	}


   NdisAcquireSpinLock(&pAdapter->lock);

   ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_PROCESSING_SENDPACKETS);

   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_REQUESTED) && !ADAPTER_TEST_FLAG(pAdapter, fADAPTER_PROCESSING_INTERRUPTS))
   {
       ADAPTER_SET_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS);
       ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RESET_REQUESTED);

       NdisReleaseSpinLock(&pAdapter->lock);

       DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
           ("Calling tbAtm155ProcessReset in TbAtm155SendPackets\n"));
 
       tbAtm155ProcessReset(pAdapter);
       NdisMResetComplete(pAdapter->MiniportAdapterHandle, NDIS_STATUS_SUCCESS, FALSE);
   }
   else
   {
       NdisReleaseSpinLock(&pAdapter->lock);
   }

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("<==TbAtm155SendPackets %x\n", NDIS_GET_PACKET_STATUS(PacketArray[0])));
}


/*
 * AlignStructure
 *
 * Align a structure within a bloc of allocated memory
 * on a specified boundary
 *
 */
VOID
AlignStructure (
   IN  PALLOCATION_MAP     Map,
   IN  ULONG               Boundary
   )
{
   ULONG   AlignmentOffset;

   AlignmentOffset = Boundary - ((ULONG)((ULONG_PTR)Map->AllocVa % Boundary));
   Map->Va = (PUCHAR)Map->AllocVa + AlignmentOffset;
   Map->Pa = NdisGetPhysicalAddressLow(Map->AllocPa)+ AlignmentOffset;
}


VOID
tbAtm155FreeTransmitBuffers(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PVC_BLOCK           pVc
   )
/*++

Routine Description:

   This routine will free up transmit buffers and it's resources.
   If the VC that these resources belong to is deactivating then we also
   need to check to see if we can complete the deactivation.

Arguments:

   pAdapter    -   Pointer to the adapter block.
   pVc         -   Pointer to the VC block that we are going to free.

Return Value:

   None.

--*/
{
   UINT                i;
   PXMIT_SEG_INFO      pXmitSegInfo = pVc->XmitSegInfo;

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("==>tbAtm155FreeTransmitBuffers\n"));

   //
   //	Free the allocated shared memory.
   //
   if(NULL != pXmitSegInfo->Va)
   {
       NdisMFreeSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pXmitSegInfo->Size,
           TRUE,
           pXmitSegInfo->Va,
           pXmitSegInfo->Pa);

       pXmitSegInfo->Va = NULL;
   }

   for (i = 0; i < TBATM155_MAX_PADTRAILER_BUFFERS; i ++)
	{
       if (pXmitSegInfo->PadTrailerBuffers[i].FlushBuffer != (PNDIS_BUFFER)NULL)
       {
           NdisFreeBuffer(pXmitSegInfo->PadTrailerBuffers[i].FlushBuffer);
       }

       pXmitSegInfo->PadTrailerBuffers[i].AllocVa = NULL;
       pXmitSegInfo->PadTrailerBuffers[i].FlushBuffer = (PNDIS_BUFFER)NULL;

   }   // end of FOR

   //
   //	Free the buffer pools
   //
   if (NULL != pXmitSegInfo->hFlushBufferPool)
   {
       NdisFreeBufferPool(pXmitSegInfo->hFlushBufferPool);
       pXmitSegInfo->hFlushBufferPool = NULL;
   }

   if (NULL != pXmitSegInfo->pTransmitContext)
   {
       FREE_MEMORY(pXmitSegInfo->pTransmitContext, sizeof(TRANSMIT_CONTEXT));
       pXmitSegInfo->pTransmitContext = NULL;
   }

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("<==tbAtm155FreeTransmitBuffers\n"));

}


NDIS_STATUS
tbAtm155AllocateTransmitBuffers(
   IN  PVC_BLOCK       pVc
   )
/*++

Routine Description:

   This routine will allocate a pool of transmit buffers, NDIS_BUFFERs.

Arguments:

   pVc     -   Pointer to the VC block.

Return Value:

   NDIS_STATUS_SUCCESS     if the allocation was successful.
   NDIS_STATUS_RESOURCES   if the allocation failed due to memory constraints.

--*/
{

   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   ULONG               DmaAlignmentRequired;
   ULONG               TotalBufferSize;
   PADAPTER_BLOCK	    pAdapter = pVc->Adapter;
   PXMIT_SEG_INFO      pXmitSegInfo = pVc->XmitSegInfo;
   PTRANSMIT_CONTEXT   pTransmitContext;


   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("==>tbAtm155AllocateTransmitBuffers\n"));


   do
   {
       //
       //	Allocate a buffer pool.
       //
       NdisAllocateBufferPool(
           &Status,
           &pXmitSegInfo->hFlushBufferPool,
           TBATM155_MAX_PADTRAILER_BUFFERS);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("Unable to allocate a NDIS_BUFFER pool for Tx flush buffers.\n"));

           break;
       }

       //
       //	Get the DMA alignment that we need.
       //
       
       DmaAlignmentRequired = gDmaAlignmentRequired;

       if (DmaAlignmentRequired < TBATM155_MIN_DMA_ALIGNMENT)
       {
           DmaAlignmentRequired = TBATM155_MIN_DMA_ALIGNMENT;
       }

       TotalBufferSize = MAX_PADDING_BYTES + 
                         sizeof(AAL5_PDU_TRAILER) +
                         DmaAlignmentRequired;

       DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
           ("TotalBufferSize= 0x%lx\n", TotalBufferSize));

       //
       //  Use NdisMAllocateSharedMemoryAsync instead of 
       //  NdisMAllocateSharedMemory to avoid a problem when 
       //  this routine is called by DISPATCH level.
       //
       //	Allocate context buffer for call back
       //
       ALLOCATE_MEMORY(
           &Status,
           &pTransmitContext,
           sizeof(TRANSMIT_CONTEXT),
           '8ADA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("Unable to allocate a context buffer\n"));

           break;
       }

       pXmitSegInfo->pTransmitContext = pTransmitContext;

       ZERO_MEMORY(pTransmitContext, sizeof(TRANSMIT_CONTEXT));

       pTransmitContext->Signature = XMIT_BUFFER_SIG;
       pTransmitContext->BufferSize = TotalBufferSize;
       pTransmitContext->DmaAlignment = DmaAlignmentRequired;
       pTransmitContext->pVc = pVc;

       Status = NdisMAllocateSharedMemoryAsync(
                       pAdapter->MiniportAdapterHandle,
                       (TotalBufferSize * TBATM155_MAX_PADTRAILER_BUFFERS),
                       TRUE,
                       pTransmitContext);

       if (NDIS_STATUS_PENDING != Status)
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("Unable to allocate pad and trailer buffer for transmit\n"));

           break;
       }

   } while (FALSE);			

   //
   //	If we failed the routine then we need to free the resources
   //	that were allocated.
   //
   if (NDIS_STATUS_PENDING != Status)
   {
       tbAtm155FreeTransmitBuffers(pAdapter, pVc);
   }

   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateTransmitBuffers\n"));

   return(Status);

}

VOID
TbAtm155AllocateXmitBufferComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PTRANSMIT_CONTEXT       pTransmitContext
   )
{
   PADAPTER_BLOCK          pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   ULONG                   DmaAlignmentRequired;
   ULONG                   TotalBufferSize;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   PVC_BLOCK               pVc;
   PXMIT_SEG_INFO          pXmitSegInfo;
   NDIS_PHYSICAL_ADDRESS   AllocatedPhysicalAddress;
   ULONG                   PhysicalAddressLow;
   UINT                    i = 0;

   pVc = pTransmitContext->pVc;

   ASSERT(NULL != pVc);

   pXmitSegInfo = pVc->XmitSegInfo;
   TotalBufferSize = pTransmitContext->BufferSize;
   DmaAlignmentRequired = pTransmitContext->DmaAlignment;

   pXmitSegInfo->Va = VirtualAddress;
   pXmitSegInfo->Pa = *PhysicalAddress;
   pXmitSegInfo->Size = TotalBufferSize;

   do 
   {

       NdisAcquireSpinLock(&pVc->lock);

       // 
       //  Check if we have encountered fVC_ERROR and/or 
       //  fVC_MEM_OUT_OF_RESOURCES issues.
       //
       if (VC_TEST_FLAG(pVc, fVC_ERROR) || VC_TEST_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES))
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR, ("fVC_ERROR\n"));

           Status = NDIS_STATUS_INVALID_DATA;
           if (VC_TEST_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES))
           {
               // 
               //  Detected out of memory resources when allocating
               //  Rx buffers. Set the flag.
               // 
               Status = NDIS_STATUS_RESOURCES;
           }

           VC_CLEAR_FLAG(pVc, (fVC_ERROR | fVC_MEM_OUT_OF_RESOURCES));

           if (NULL != VirtualAddress)
           {
               NdisReleaseSpinLock(&pVc->lock);

               NdisMFreeSharedMemory(
                   pAdapter->MiniportAdapterHandle,
                   Length,
                   TRUE,
                   VirtualAddress,
                   *PhysicalAddress);
               
               pXmitSegInfo->Va = NULL;               
               
               NdisAcquireSpinLock(&pVc->lock);

           }

           VC_SET_FLAG(pVc, fVC_TRANSMIT);

           NdisReleaseSpinLock(&pVc->lock);

           TbAtm155ActivateVcComplete(
               pTransmitContext->pVc,
               Status);

           break;
       }

       NdisReleaseSpinLock(&pVc->lock);

       if (NULL != VirtualAddress)
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                   ("AllcateVa=%lX  AllocatePa=%8X  Size=%8X  Alignment=%8X\n",
                   VirtualAddress,
                   NdisGetPhysicalAddressLow(*PhysicalAddress),
                   Length,
                   DmaAlignmentRequired));
    
           for (i = 0; i < TBATM155_MAX_PADTRAILER_BUFFERS; i++)
           {
               pXmitSegInfo->PadTrailerBuffers[i].AllocSize = TotalBufferSize;
               
               //
               //  Calculate the virtual address
               //
               pXmitSegInfo->PadTrailerBuffers[i].AllocVa = 
                                   (PUCHAR)VirtualAddress + (i * TotalBufferSize);

               //
               //  Calculate the physical address
               //
               AllocatedPhysicalAddress = *PhysicalAddress;
               PhysicalAddressLow = NdisGetPhysicalAddressLow(*PhysicalAddress)+i*TotalBufferSize;
               NdisSetPhysicalAddressLow(AllocatedPhysicalAddress, PhysicalAddressLow);
               pXmitSegInfo->PadTrailerBuffers[i].AllocPa = AllocatedPhysicalAddress;
    
               AlignStructure (
                   &pXmitSegInfo->PadTrailerBuffers[i],
                   DmaAlignmentRequired);
    
               DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,
                       ("Va=%8X  Pa=%8X\n", pXmitSegInfo->PadTrailerBuffers[i].Va, pXmitSegInfo->PadTrailerBuffers[i].Pa));

               //
               //  Allocate an NDIS flush buffer for each transmit buffer
               //
               NdisAllocateBuffer(
                   &Status,
                   &pXmitSegInfo->PadTrailerBuffers[i].FlushBuffer,
                   pXmitSegInfo->hFlushBufferPool,
                   (PVOID)pXmitSegInfo->PadTrailerBuffers[i].Va,
                   (TotalBufferSize - DmaAlignmentRequired));
               if (Status != NDIS_STATUS_SUCCESS)
               {
                   DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
                       ("Unable to allocate a flush memory for PadTrailerBuffers\n"));

                   break;
               }
    
               pXmitSegInfo->FreePadTrailerBuffers++;
    
           }   // end of FOR

       }
       else
       {
           DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_ERR,
               ("Failed to allocate PadTrailerBuffers(%d)\n", i));
           Status = NDIS_STATUS_RESOURCES;
       }
    
       ASSERT(NULL != pTransmitContext->pVc);
    
       //
       //  Clear the flag since allocation of PadTrailerBuffers 
       //  has been done.
       //
       NdisAcquireSpinLock(&pVc->lock);

       VC_SET_FLAG(pVc, fVC_TRANSMIT);
       VC_CLEAR_FLAG(pVc, fVC_ALLOCATING_TXBUF);

       if (VC_TEST_FLAG(pVc, fVC_ALLOCATING_RXBUF))
       {
           if (NDIS_STATUS_RESOURCES == Status)
           {
               //
               // NDIS_STATUS_RESOURCES is reported.
               //
               VC_SET_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES);
           }

           NdisReleaseSpinLock(&pVc->lock);
       }
       else
       {
           // 
           //  Detected out of resources while allocating Rx buffers.
           // 
           if (VC_TEST_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES))
           {
               VC_CLEAR_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES);
               Status = NDIS_STATUS_RESOURCES;
           }

           // 
           //  Since not waiting for Receive buffers allocated,
           //  this VC is activated.
           // 
           NdisReleaseSpinLock(&pVc->lock);
           TbAtm155ActivateVcComplete(pTransmitContext->pVc, Status);
       }

   } while (FALSE);

   return;

}



