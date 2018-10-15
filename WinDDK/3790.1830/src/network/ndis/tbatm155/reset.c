/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       reset.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_RESET


UCHAR
tbAtm155CheckConnectionStatus(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine check connection of Meteor and set LED accordingly.
       1. turn ON/OFF of Led LED_LNKUP by check PHY chip.
       2. trun ON/OFF of Led LED_TXRX by checking Meteor Tx/Rx 
          report queue registers.
   
Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

   NDIS_STATUS_FAILURE     if the adapter is not supported.
   NDIS_STATUS_SUCCESS     otherwise.

--*/
{
   BOOLEAN                 fLinkUp = FALSE;
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   UCHAR                   LedValue = pHwInfo->LedVal;
   TX_REPORT_PTR           TxReportPtrReg;
   RX_REPORT_PTR           RxReportPtrReg;
   PXMIT_DMA_QUEUE         pXmitDmaQ = &pHwInfo->SarInfo->XmitDmaQ;

   do {
       //
       // Check if SAR is receiving/transmitting data.
       // if yes, turn on the LED, otherwise, turn the LED off.
       //
       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->Tx_Report_Ptr,
           &TxReportPtrReg.reg);

       TxReportPtrReg.reg = TxReportPtrReg.reg & ~0x3;

       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->Rx_Report_Ptr,
           &RxReportPtrReg.reg);

       RxReportPtrReg.reg = RxReportPtrReg.reg & ~0x3;

#if TB_CHK4HANG

       if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_EXPECT_TXIOC))
       {
           ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_EXPECT_TXIOC);

           if (TxReportPtrReg.reg == pHwInfo->PrevTxReportPa)
           {
               ADAPTER_SET_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE);
#if DBG
                DbgPrint("TBATM155: Adapter %p, Flags %x: tx hang!\n",
                        pAdapter, pAdapter->Flags);
#endif // DBG
           }
       }
       else
       {
           if ((pXmitDmaQ->RemainingTransmitSlots != pXmitDmaQ->MaximumTransmitSlots) &&
               (TxReportPtrReg.reg == pHwInfo->PrevTxReportPa))
           {
               ADAPTER_SET_FLAG(pAdapter, fADAPTER_EXPECT_TXIOC);
           }
       }

#endif // end of TB_CHK4HANG

       if ((TxReportPtrReg.reg != pHwInfo->PrevTxReportPa) ||
           (RxReportPtrReg.reg != pHwInfo->PrevRxReportPa))
       {
           LedValue |= LED_TXRX_ON_GREEN;

           //
           // Save the current pointer of report queues got from SAR.
           //
           pHwInfo->PrevTxReportPa = TxReportPtrReg.reg;
           pHwInfo->PrevRxReportPa = RxReportPtrReg.reg;
       }
       else
       {
           LedValue &= ~LED_TXRX_ON_GREEN;
       }

       //
       // Check if PHY is still link up the remote device or not.
       // if PHY is linked up with remote device, turn on the LED, 
       // otherwise, turn the LED off.
       //
       // For PLC2, handle in PLC2 interrupt service routine.
       //
       if (pHwInfo->fAdapterHw & TBATM155_PHY_SUNI_LITE)
       {
           fLinkUp = tbAtm155CheckSuniLiteLinkUp(pAdapter);
           LedValue &= ~(LED_LNKUP_ON_GREEN | LED_LNKUP_ON_ORANGE);

           if (fLinkUp == TRUE)
           {
               LedValue |= LED_LNKUP_ON_GREEN;
           }

       }

       if (pHwInfo->LedVal != LedValue)
       {
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               LED_OFFSET,
               LedValue,
               &Status);
       }

   } while (FALSE);

   return(LedValue);
}



BOOLEAN
TbAtm155CheckForHang(
	IN	NDIS_HANDLE	MiniportAdapterContext
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
   PADAPTER_BLOCK	pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   BOOLEAN			fReset = FALSE;
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   UCHAR           LedValue;

   NdisDprAcquireSpinLock(&pAdapter->lock);

   //
   //	Is the adapter currently resetting or shutting down?
   //
   if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS) &&
       !ADAPTER_TEST_FLAG(pAdapter, fADAPTER_SHUTTING_DOWN))
   {

       dbgDumpActivateVcInfo(pAdapter);

       //
       // Check LEDs based Meteor status.
       //
       // this is the implementation of the NDIS 4 feature for detecting
       // link status change. It effectively checks every two seconds
       // for link.
       //
       LedValue = tbAtm155CheckConnectionStatus(pAdapter);

       if ((pHwInfo->LedVal & LED_LNKUP_ALL_BITS) != (LedValue & LED_LNKUP_ALL_BITS))
       {
           //
           // Linkup status has been changed. Handle it.
           //
           if ((LedValue & LED_LNKUP_ON_GREEN) &&
               (NdisMediaStateConnected != pAdapter->MediaConnectStatus))
           {
               pAdapter->MediaConnectStatus = NdisMediaStateConnected;

               NdisDprReleaseSpinLock(&pAdapter->lock);

               NdisMIndicateStatus(
                   pAdapter->MiniportAdapterHandle,
                   NDIS_STATUS_MEDIA_CONNECT,
                   (PVOID)0,
                   0);
               // NOTE:
               // have to indicate status complete every time you 
               // indicate status
               NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);

               NdisDprAcquireSpinLock(&pAdapter->lock);

               DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO, ("NDIS_STATUS_MEDIA_CONNECT\n"));
           }
           else if (!(LedValue & LED_LNKUP_ON_GREEN) &&
                    (NdisMediaStateDisconnected != pAdapter->MediaConnectStatus))
           {
               pAdapter->MediaConnectStatus = NdisMediaStateDisconnected;

               NdisDprReleaseSpinLock(&pAdapter->lock);

               NdisMIndicateStatus(
                   pAdapter->MiniportAdapterHandle,
                   NDIS_STATUS_MEDIA_DISCONNECT,
                   (PVOID)0,
                   0);

               // NOTE:
               // have to indicate status complete every time you 
               // indicate status
               NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);

               NdisDprAcquireSpinLock(&pAdapter->lock);

               DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO, ("NDIS_STATUS_MEDIA_DISCONNECT\n"));
           }

       } // end of if

       NdisDprAcquireSpinLock(&pHwInfo->Lock);
       pHwInfo->LedVal = LedValue;     // save the current value.
       NdisDprReleaseSpinLock(&pHwInfo->Lock);

       //
       //	Was there a hardware failure?
       //
       if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE))
       {
           fReset = TRUE;
       }
   }

   NdisDprReleaseSpinLock(&pAdapter->lock);

   return(fReset);
}

VOID
tbAtm155CompletePacketQueue(
   IN  PPACKET_QUEUE   PacketQ
	)
/*++

Routine Description:

   This routine will complete the packets on the given queue back to their
   respective owners.  This is done in the case of a reset so the status that
   the packets are completed with is NDIS_STATUS_REQUEST_ABORTED.

Arguments:

   PacketQ	-	Pointer to the queue of packets to complete.

Return Value:

   None.

--*/
{
   PNDIS_PACKET        Packet;
   PVC_BLOCK           pVc;
   PPACKET_RESERVED    Reserved;
   PXMIT_SEG_INFO      pXmitSegInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ;


//   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
//       ("==>tbAtm155CompletePacketQueue\n"));

	while (!PACKET_QUEUE_EMPTY(PacketQ))
	{
		//
		//	Remove the packet from the queue.
		//
		RemovePacketFromHeadDpc(PacketQ, &Packet);

		dbgLogSendPacket(
			PACKET_RESERVED_FROM_PACKET(Packet)->pVc->DebugInfo,
			PacketQ->Head,
			0,
			104,
			'erQD');
		//
		//	Get a local copy of the vc that this packet belongs to.
		//
       Reserved = PACKET_RESERVED_FROM_PACKET(Packet);
       pVc = Reserved->pVc;
       pXmitSegInfo = pVc->XmitSegInfo;
       pXmitDmaQ = &pVc->Sar->XmitDmaQ;

		//
		//	complete the packet back to the client.
		//
       dbgLogSendPacket(
           pVc->DebugInfo,
           Packet,
           0,
           0,
           ' tsr');

       NdisMCoSendComplete(
           NDIS_STATUS_REQUEST_ABORTED,
           pVc->NdisVcHandle,
           Packet);

	    DBGPRINT(DBG_COMP_TXIOC, DBG_LEVEL_ERR,
		    ("tbAtm155CompletePacketQueue: %x (NDIS_STATUS_REQUEST_ABORTED)\n", Packet));

       //
       //	Remove the packet reference from the VC.
       //
       NdisAcquireSpinLock(&pVc->lock);
       tbAtm155DereferenceVc(pVc);
       NdisReleaseSpinLock(&pVc->lock);

	}

//   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
//   ("<==tbAtm155CompletePacketQueue\n"));

}



VOID
tbAtm155ProcessReset(
   IN  PADAPTER_BLOCK  pAdapter
	)
/*++

Routine Description:

	This routine will do the actual resetting of the adapter.
   All of the interrupts are disabled before getting into this routine.

Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PSAR_INFO           pSar = pHwInfo->SarInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ = &pSar->XmitDmaQ;
   PRECV_DMA_QUEUE     pRecvDmaQ = &pSar->RecvDmaQ;
   PLIST_ENTRY         Link;
   PLIST_ENTRY         NextLink;
   PVC_BLOCK           pVc;
   PXMIT_SEG_INFO      pXmitSegInfo;
   NDIS_STATUS         Status;
   PRECV_BUFFER_QUEUE  pSegCompleting;
   TB155PCISAR_CNTRL1  regControl1;
	

   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
           ("==>tbAtm155ProcessReset\n"));

   do {
       //
       //  Don't go through complete reset if there is no hardware 
       //  failure detected.
       //
       if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE))
       {
           //
           //  Walk the VC's that are opened on this adapter and mark them as
           //  resetting.  Also check to see if any VCs need to be deactivated.
           //
           Link = pAdapter->ActiveVcList.Flink;

           while (Link != &pAdapter->ActiveVcList)
           {
               NextLink = Link->Flink;

               //
               //	Get a pointer to the VC that this link represents.
               //
               pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);

               //
               //	Get the next.
               //
               Link = Link->Flink;

               NdisAcquireSpinLock(&pVc->lock);

               VC_CLEAR_FLAG(pVc, fVC_RESET_IN_PROGRESS);


               //
               //	Check to see if we need to deactivate the VC.
               //
               if ((1 == pVc->References) && VC_TEST_FLAG(pVc, fVC_DEACTIVATING))
               {
                   NdisReleaseSpinLock(&pVc->lock);
	
                   //
                   //	Finish VC deactivation.
                   //
                   TbAtm155DeactivateVcComplete(pVc->Adapter, pVc);
               }
               else
               {
                   NdisReleaseSpinLock(&pVc->lock);
               }

               Link = NextLink;
           }

           NdisAcquireSpinLock(&pAdapter->lock);

           ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS);
           ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE);

           NdisReleaseSpinLock(&pAdapter->lock);

           break;
       }

       //
       //  Detected hardware failure, let's reset hardware.
       //
       tbAtm155SoftresetNIC(pHwInfo);

       //
       //	Complete any packets that are either waiting on the transmit 
       //  complete for the segment or waiting for segment resources.
       //
       for (Link = pAdapter->ActiveVcList.Flink;
            Link != &pAdapter->ActiveVcList;
            Link = Link->Flink)
       {
           //
           //	Get a pointer to the VC that this link represents.
           //
           pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);

           ASSERT(NULL != pVc);
           

 #if DBG
	        DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
		             ("pVc:0x%lx, VC:%lx\n", pVc, pVc->VpiVci.Vci));
 #endif // end of DBG

           if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
           {
               //
               //  1. Clean up the queue of packets that are waiting
               //     for transmit resources.
               //  2. Unlock the transmit segment of this VC, so it can
               //     be re-used.
               //
               pXmitSegInfo = pVc->XmitSegInfo;
               tbAtm155CompletePacketQueue(&pXmitSegInfo->SegWait);
               tbAtm155CompletePacketQueue(&pXmitSegInfo->DmaCompleting);
           }
           
           if (VC_TEST_FLAG(pVc, fVC_RECEIVE))
           {
               pSegCompleting = &pVc->RecvSegInfo->SegCompleting;
           
               if (NULL == pSegCompleting->BufListHead)
               {
                   continue;
               }
           
               //
               //	Return the packet and it's resources back to it's receive
               //	buffer queue.
               //
               TbAtm155ReturnPacket(
                       (NDIS_HANDLE)pAdapter,
                       pSegCompleting->BufListHead->Packet);

               //
               //  Initialize the queue.
               //
               pSegCompleting->BufListHead = NULL;
               pSegCompleting->BufListTail = NULL;
               pSegCompleting->BufferCount = 0;
           }       
       }           

       //
       //	Clean up any packets waiting for DMA resources
       //  and initialize the PadTrailer counters.
       //
       tbAtm155CompletePacketQueue(&pXmitDmaQ->DmaWait);

       NdisAcquireSpinLock(&pXmitDmaQ->lock);
       pXmitDmaQ->RemainingTransmitSlots = pXmitDmaQ->MaximumTransmitSlots;

       NdisReleaseSpinLock(&pXmitDmaQ->lock);
       
       
       //
       //  1. Return the buffers in the 'Big' buffer queue which wait for 
       //     receiving data on network
       //  2. Initialize the index for Rx Big Slot FIFO
       //
       tbAtm155MergeRecvBuffers2FreeBufferQueue(
               pAdapter,
               &pSar->FreeBigBufferQ,
               &pRecvDmaQ->CompletingBigBufQ);
       
       pRecvDmaQ->RemainingReceiveBigSlots = pRecvDmaQ->MaximumReceiveBigSlots;
       
       //
       //  1. Return the buffers in the 'Small' buffer queue which wait for 
       //     receiving data on network
       //  2. Initialize the index for Rx Small Slot FIFO
       //
       tbAtm155MergeRecvBuffers2FreeBufferQueue(
               pAdapter,
               &pSar->FreeSmallBufferQ,
               &pRecvDmaQ->CompletingSmallBufQ);
       
       pRecvDmaQ->RemainingReceiveSmallSlots = pRecvDmaQ->MaximumReceiveSmallSlots;
       
       pRecvDmaQ->PrevRxReportQIndex    = 0;
       pXmitDmaQ->PrevTxReportQIndex    = 0;
           
       //
       //	re-initialize the SAR and related parameters.
       //
       tbAtm155InitSarParameters(pAdapter);
       Status = tbAtm155InitRegisters(pAdapter);

 #if DBG
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
                    ("Failed to Reset the registers\n"));
       }
 #endif // end of DBG
       
       //
       //	Walk through the VC's that are opened on this adapter and mark
       //	them out of resetting. Also check to see if any VCs need to 
       //  be deactivated.
       //
       Link = pAdapter->ActiveVcList.Flink;
       
       while (Link != &pAdapter->ActiveVcList)
       {
           NextLink = Link->Flink;
       
           //
           //	Get a pointer to the VC that link represents.
           //
           pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);
           
           //
           //	Get the next.
           //
           Link = Link->Flink;
           
           NdisAcquireSpinLock(&pVc->lock);
           
           VC_CLEAR_FLAG(pVc, fVC_RESET_IN_PROGRESS);
           
           //
           //	Check to see if we need to deactivate the VC.
           //
           if ((VC_TEST_FLAG(pVc, fVC_DEACTIVATING)) && (1 == pVc->References))
           {
               NdisReleaseSpinLock(&pVc->lock);

               DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR, ("pVc:"PTR_FORMAT"\n", pVc));
               //
               //	Finish VC deactivation.
               //
               TbAtm155DeactivateVcComplete(pVc->Adapter, pVc);
           }       
           else    
           {       
#if DBG
               if (1 != (pVc->References - pVc->PktsHoldsByNdis))
               {
                   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
                            ("pVc->References: 0x%x, pVc->PktsHoldsByNdis: 0x%x, pVc:"PTR_FORMAT"\n", 
                            pVc->References, pVc->PktsHoldsByNdis, pVc));
               }
               else if (pVc->References <= pVc->PktsHoldsByNdis)
               {
                   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
                            ("pVc->References: 0x%x, pVc->PktsHoldsByNdis: 0x%x, pVc:"PTR_FORMAT"\n", 
                            pVc->References, pVc->PktsHoldsByNdis, pVc));
                   DBGBREAK(DBG_COMP_RESET, DBG_LEVEL_ERR);
               }
#endif  // end of DBG

               Status = tbAtm155OpenVcChannels(pAdapter, pVc);
               if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
               {
                   pXmitSegInfo = pVc->XmitSegInfo;
                   pXmitSegInfo->BeOrBeingUsedPadTrailerBufs = 0;
                   pXmitSegInfo->FreePadTrailerBuffers = TBATM155_MAX_PADTRAILER_BUFFERS;
                   pXmitSegInfo->PadTrailerBufferIndex = 0;
               }

 #if DBG
               DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
                        ("After OpenVCChannel .. pVc:0x%lx, VC:%lx\n", pVc, pVc->VpiVci.Vci));
 #endif // end of DBG

               NdisReleaseSpinLock(&pVc->lock);
           }       

           Link = NextLink;
       }
       NdisAcquireSpinLock(&pAdapter->lock);

       ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS);
       ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE);
#if TB_CHK4HANG
       ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_EXPECT_TXIOC);
#endif // end of TB_CHK4HANG

       NdisReleaseSpinLock(&pAdapter->lock);

       //
       //  Post receive buffers from both pools.
       //
       tbAtm155QueueRecvBuffersToReceiveSlots(pAdapter, RECV_SMALL_BUFFER);
       tbAtm155QueueRecvBuffersToReceiveSlots(pAdapter, RECV_BIG_BUFFER);

       //
       //	Start Transmitter and Receiver new.
       //
       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           &regControl1.reg);

       regControl1.Tx_Enb = 1;
       regControl1.Rx_Enb = 1;

       TBATM155_WRITE_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           regControl1.reg);

   } while (FALSE);

   //
   //	Enable interrupts.
   //
   NdisMSynchronizeWithInterrupt(
       &pAdapter->HardwareInfo->Interrupt,
       (PVOID)TbAtm155EnableInterrupt,
       pAdapter);

   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
       ("<==tbAtm155ProcessReset\n"));
}

NDIS_STATUS
TbAtm155Reset(
   OUT PBOOLEAN    AddressingReset,
   IN  NDIS_HANDLE MiniportAdapterContext
   )
/*++

Routine Description:

   This routine will either reset the adapter or schedule a reset to happen.

Arguments:

   AddressingReset         -   Not used.
	MiniportAdapterContext  -   Pointer to the ADAPTER_BLOCK.

Return Value:

--*/
{
   PADAPTER_BLOCK      pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;

   *AddressingReset = FALSE;

   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
       ("==>TbAtm155Reset\n"));


   NdisAcquireSpinLock(&pAdapter->lock);

   //
   //	Are we currently processing a reset?  Or is there a reset
   //	already requested? Or are we shutting down the adapter?
   //
   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS) ||
       ADAPTER_TEST_FLAG(pAdapter, fADAPTER_SHUTTING_DOWN) ||
       ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_REQUESTED))
   {
       //
       //	Don't do it twice.
       //
       NdisReleaseSpinLock(&pAdapter->lock);

#if DBG
       if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_SHUTTING_DOWN))
       {
           DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
               ("Skipping reset cuz Adapter %x is shutting down\n", pAdapter));
		}
#endif

       DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
           ("<==TbAtm155Reset (NDIS_STATUS_RESET_IN_PROGRESS)\n"));

       return(NDIS_STATUS_RESET_IN_PROGRESS);
   }

   //
   //	Walk the active VC list and mark them as resetting.
   //
   if (!IsListEmpty(&pAdapter->ActiveVcList))
   {
       PLIST_ENTRY     Link;
       PVC_BLOCK       pVc;

       //
       //	Mark all active VC's as resetting.
       //
       for (Link = pAdapter->ActiveVcList.Flink;
            Link != &pAdapter->ActiveVcList;
            Link = Link->Flink)
       {
           //
           //	Get a pointer to the VC that this link represents.
           //
           pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);

           NdisDprAcquireSpinLock(&pVc->lock);

           VC_SET_FLAG(pVc, fVC_RESET_IN_PROGRESS);

           NdisDprReleaseSpinLock(&pVc->lock);
       }
   }

   //
   //	Are we currently processing an interrupt?
   //
   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_PROCESSING_INTERRUPTS) || 
       ADAPTER_TEST_FLAG(pAdapter, fADAPTER_PROCESSING_SENDPACKETS))
   {
       //
       //	Request a reset to happen when the interrupt processing is done.
       //
       ADAPTER_SET_FLAG(pAdapter, fADAPTER_RESET_REQUESTED);
       NdisReleaseSpinLock(&pAdapter->lock);

       DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
           ("<==TbAtm155Reset\n"));

       return(NDIS_STATUS_PENDING);
   }

   //
   //	Ok we can process the reset.
   //
   ADAPTER_SET_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS);

   NdisMSynchronizeWithInterrupt(
       &pAdapter->HardwareInfo->Interrupt,
       (PVOID)TbAtm155DisableInterrupt,
       pAdapter);

   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
           ("Calling tbAtm155ProcessReset in TbAtm155Reset\n"));

   NdisReleaseSpinLock(&pAdapter->lock);

   //
   //  this routine does the actual resetting.
   //
   tbAtm155ProcessReset(pAdapter);

   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
       ("<==TbAtm155Reset\n"));

   return(NDIS_STATUS_SUCCESS);
}


VOID
TbAtm155Halt(
	IN	NDIS_HANDLE	MiniportAdapterContext
	)
/*++

Routine Description:

   This routine is called when the adapter is being stopped.
   We can assume that there are no open VCs. we just need to clean up our
   software and hardware state.

Arguments:

   MiniportAdapterContext  -   Pointer to the ADAPTER_BLOCK.

Return Value:

   None.

--*/
{
   PADAPTER_BLOCK          pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   PSAR_INFO               pSar = pHwInfo->SarInfo;
   PXMIT_DMA_QUEUE         pXmitDmaQ = &pSar->XmitDmaQ;
   PRECV_DMA_QUEUE         pRecvDmaQ = &pSar->RecvDmaQ;


   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
           ("==>TbAtm155Halt\n"));

   NdisAcquireSpinLock(&pAdapter->lock);
	
   //
   //  Let's do soft reset.
   //
   tbAtm155SoftresetNIC(pHwInfo);

   NdisDprAcquireSpinLock(&pXmitDmaQ->lock);
   NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

   //
   //  Turn off the interrupts
   //
   pHwInfo->InterruptMask = 0;

   NdisMSynchronizeWithInterrupt(
       &pHwInfo->Interrupt,
       (PVOID)TbAtm155DisableInterrupt,
       pAdapter);

   NdisDprReleaseSpinLock(&pRecvDmaQ->lock);
   NdisDprReleaseSpinLock(&pXmitDmaQ->lock);

   NdisReleaseSpinLock(&pAdapter->lock);

   //
   //	Free the adapter resources.
   //
   tbAtm155FreeResources((PADAPTER_BLOCK)MiniportAdapterContext);

   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
           ("<==TbAtm155Halt\n"));
}



VOID
TbAtm155Shutdown(
   IN  PVOID   ShutdownContext
   )
/*++

Routine Description:

   This routine will stop the adapter.  We do NOT free any resources in this
   routine since we don't know what the state of the system is (it could be
   crashing).  So all we do is make sure that the adapter is stopped.

Arguments:

   ShutdownContext -   Pointer to the adapter block.

Return Value:

   None.

--*/
{
   PADAPTER_BLOCK          pAdapter = (PADAPTER_BLOCK)ShutdownContext;
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;


   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
       ("==>TbAtm155Shutdown\n"));

	ADAPTER_SET_FLAG(pAdapter, fADAPTER_SHUTTING_DOWN);

   tbAtm155SoftresetNIC(pHwInfo);

   //
   //	Turn off the interrupts
   //
   pHwInfo->InterruptMask = 0;
   TbAtm155DisableInterrupt(pAdapter);

   DBGPRINT(DBG_COMP_HALT, DBG_LEVEL_INFO,
       ("<==TbAtm155Shutdown\n"));

}


