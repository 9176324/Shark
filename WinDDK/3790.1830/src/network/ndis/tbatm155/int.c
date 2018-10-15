/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       int.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:
	This routine handles interupt processing.

   Note:
       Currently, there are not all of interrupt handlers have
       been implemented.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER   MODULE_INT


VOID
TbAtm155EnableInterrupt(
   IN	NDIS_HANDLE	MiniportAdapterContext
   )
/*++

Routine Description:

   This routine will enable the 155 PCI controller interrupts.

Arguments:

   MiniportAdapterContext  -   This is a pointer to our miniport block.

Return Value:

--*/
{
   PADAPTER_BLOCK  pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;

   //
   //  Let's clear the flag first.
   //
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Enb,
       pHwInfo->InterruptMask);

	HW_CLEAR_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPTS_DISABLED);

}

VOID
TbAtm155DisableInterrupt(
   IN  NDIS_HANDLE MiniportAdapterContext
   )
/*++

Routine Description:

   This routine will disable the 155 PCI controller interrupts.

Arguments:

   MiniportAdapterContext  -   This is a pointer to our miniport block.

Return Value:

--*/
{
   PADAPTER_BLOCK  pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;


   TBATM155_WRITE_PORT(&pHwInfo->TbAtm155_SAR->Intr_Enb, 0);

   //
   //  Let's Set the flag.
   //
   HW_SET_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPTS_DISABLED);

}


VOID
tbAtm155ErrorInterrupt(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will process error interrupts.

Arguments:

   pAdapter        -   Pointer to the adapter block.
   InterruptStatus	-   This contains the error interrupts
                       that we need to process.

Return Value:

   None.

--*/
{
	PHARDWARE_INFO	pHwInfo = pAdapter->HardwareInfo;
	ULONG			InterruptStatus = pHwInfo->InterruptStatus;


	DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
		("==>tbAtm155ErrorInterrupt\n"));

	IF_DBG(DBG_COMP_INT, DBG_LEVEL_ERR)
	{
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
	        ("Error Interrupt\n"));
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("InterruptStatus: 0x%x\n", InterruptStatus));

		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("pHwInfo: 0x%x\n", pHwInfo));
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("pSar: 0x%x\n", pHwInfo->SarInfo));
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("pXmitDmaQ: 0x%x\n", &pHwInfo->SarInfo->XmitDmaQ));
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("pRecvDmaQ: 0x%x\n", &pHwInfo->SarInfo->RecvDmaQ));
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("pVc [head]: 0x%x\n", pAdapter->ActiveVcList.Flink));
	}

	//
	//	This will cause the adapter to be reset.
	//
	ADAPTER_SET_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE);

#if DBG
	DbgPrint("TBATM155: Adapter %p, Flags %x: error interrupt!\n",
	        pAdapter, pAdapter->Flags);
	DbgBreakPoint();
#endif

	if ((InterruptStatus & TBATM155_INT_TX_FATAL_ERR) == TBATM155_INT_TX_FATAL_ERR)
	{
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("InterruptStatus & TBATM155_INT_TX_FATAL_ERR\n"));

		pAdapter->TxFatalError++;

	}

	if ((InterruptStatus & TBATM155_INT_PCI_FATAL_ERR) == TBATM155_INT_PCI_FATAL_ERR)
	{
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("InterruptStatus & TBATM155_INT_PCI_FATAL_ERR\n"));

		pAdapter->PciErrorCount++;
	}

	if ((InterruptStatus & TBATM155_INT_TX_FREE_SLOT_UNFL) == TBATM155_INT_TX_FREE_SLOT_UNFL)
   	{
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("InterruptStatus & TBATM155_INT_TX_FREE_SLOT_UNFL\n"));

		pAdapter->TxFreeSlotUnflCount++;
	}

	if ((InterruptStatus & TBATM155_INT_RX_FREE_SLOT_OVFL) == TBATM155_INT_RX_FREE_SLOT_OVFL)
   	{
		DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
			("InterruptStatus & TBATM155_INT_RX_FREE_SLOT_OVFL\n"));

		pAdapter->RxFreeSlotOvflCount++;
   	}

	DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
		("<==tbAtm155ErrorInterrupt\n"));

}



VOID
TbAtm155ISR(
	OUT	PBOOLEAN	InterruptRecognized,
	OUT	PBOOLEAN	QueueDpc,
	IN	PVOID		MiniportAdapterContext
	)
/*++

Routine Description:

   This routine will service interrupts for the TbAtm155.
   All we need to do is determine if the interrupt that was generated
   belongs to us and whether or not it needs a DPC queued to service
   it.

Arguments:
   InterruptRecognized     -   TRUE if the interrupt belongs to the
                               TbAtm155.
   QueueDpc                -   TRUE if we need our
                               MiniportInterruptHandler called.
   MiniportAdapterContext  -   Pointer to our adapter block.

Return Value:

	None.

--*/
{
   PADAPTER_BLOCK	pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PHARDWARE_INFO	pHwInfo = pAdapter->HardwareInfo;
   ULONG			InterruptStatus;


	if (HW_TEST_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPTS_DISABLED))
	{
		*InterruptRecognized = FALSE;
		*QueueDpc = FALSE;

		return;
	}


   //
   //	Read the current interrupt status from the 155 PCI registers.
   //	By reading the Int_Status (Interrupt Status) register we don't ACK
   //	any of the interrupts we just determine if we need to queue
   //	a DPC.
   //
   TBATM155_READ_PORT(&pHwInfo->TbAtm155_SAR->Intr_Status, &InterruptStatus);

   if (0 == InterruptStatus)
   {
       //
       //	No interrupts belong to us!
       //
       *InterruptRecognized = FALSE;
       *QueueDpc = FALSE;

       return;
   }

   //
   //  The interrupt is ours, we need to make sure we don't
   //  get any more til we are done.
   //
   TBATM155_WRITE_PORT(&pHwInfo->TbAtm155_SAR->Intr_Enb, 0);

   HW_SET_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPTS_DISABLED);

   //
   //	Do the rest in a dpc.
   //
   *InterruptRecognized = TRUE;
   *QueueDpc = TRUE;
}

VOID
TbAtm155HandleInterrupt(
   IN  NDIS_HANDLE MiniportAdapterContext
   )
/*++

Routine Description:

   This is the workhorse routine.  It will loop processing interrupts
   until there are none left or it's spent too much time...

Arguments:

   MiniportAdapterContext  -   Pointer to the adapter block.

Return Value:

   None.

--*/
{
   //
   //	Pointer to the adapter block.
   //	
   PADAPTER_BLOCK      pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;

   //
   //  Pointer to our hardware information.
   //
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;

   //
   //  Pointer to the SAR information.
   //
   PSAR_INFO           pSar = pHwInfo->SarInfo;

   //
   //	Interrupt status codes to process.
   //
   ULONG               InterruptStatus;

   //
   //	Number of times we have looped through the interrupt status's.
   //
   UINT                InterruptCount;

   //
   //	This is set to TRUE if the adapter has received
   //	(and indicated) any packets.
   //
   BOOLEAN				fReceived = FALSE;
   BOOLEAN				fTmpReceived = FALSE;
   INTR_HLDOFF_REG     regIntrHldoff;

   //
   //	If the adapter is currently resetting then we need to bail.
   //
   NdisDprAcquireSpinLock(&pAdapter->lock);

   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS) ||
       ADAPTER_TEST_FLAG(pAdapter, fADAPTER_HARDWARE_FAILURE))
   {
       NdisDprReleaseSpinLock(&pAdapter->lock);

       DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
           ("<==TbAtm155HandleInterrupt_1\n"));

       return;

   }

   ASSERT(!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_REQUESTED));

   //
   //	Mark the adapter as processing interrupts.
   //
   ADAPTER_SET_FLAG(pAdapter, fADAPTER_PROCESSING_INTERRUPTS);

   NdisDprReleaseSpinLock(&pAdapter->lock);

   //
   //	Read the interrupt status.
   //
   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Status,
       &InterruptStatus);

   InterruptStatus &= TBATM155_REG_INT_VALID;

   //
   //	Clear the interrupt status register
   //
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Status,
       InterruptStatus);

   InterruptCount = 0;

   while (0 != InterruptStatus)
   {
       if (InterruptStatus & (TBATM155_INT_TX_IOC | TBATM155_INT_RX_IOC))
       {
           // 
           //  Reload the holdoff count and timer.
           // 
           regIntrHldoff.reg = 0;
           if (InterruptStatus & TBATM155_INT_TX_IOC)
           {
               //
               //  Set HoldOff count and timer of TxIOC interrupt.
               //
               regIntrHldoff.Tx_IOC_Hldoff_Wr = 1;
               regIntrHldoff.Tx_IOC_Hldoff_Cnt  = TX_IOC_HLDOFF_CNT;
               regIntrHldoff.Tx_IOC_Hldoff_Time = TX_IOC_HLDOFF_TIME;
           }

           if (InterruptStatus & TBATM155_INT_RX_IOC)
           {
               //
               //  Set HoldOff count and timer of RxIOC interrupt.
               //
               regIntrHldoff.Rx_IOC_Hldoff_Wr = 1;
               regIntrHldoff.Rx_IOC_Hldoff_Cnt  = RX_IOC_HLDOFF_CNT;
               regIntrHldoff.Rx_IOC_Hldoff_Time = RX_IOC_HLDOFF_TIME;
           }
                      
           TBATM155_WRITE_PORT(
               &pHwInfo->TbAtm155_SAR->Intr_Hldoff,
               regIntrHldoff.reg);

           // 
           //  Disable modifying holdoff count and timer fields.
           // 
           regIntrHldoff.reg = 0;

           TBATM155_WRITE_PORT(
               &pHwInfo->TbAtm155_SAR->Intr_Hldoff,
               regIntrHldoff.reg);
       }
       
       //
       //	Was there any error interrupts?
       //
       if ((InterruptStatus & TBATM155_REG_INT_ERROR) != 0)
       {
           //
           //	Save the interrupt status with the adapter block's
           //	hardware information to process when we are synchronized with
           //	the interrupt.
           //
           pHwInfo->InterruptStatus = InterruptStatus;

           //
           //	Process the error interrupts.
           //
           tbAtm155ErrorInterrupt(pAdapter);

           break;
       }

       //
       //	Was there a interrupt from the PHY chip?
       //
       if (InterruptStatus & TBATM155_INT_PHY_INTR)
       {
           if (pHwInfo->fAdapterHw & TBATM155_PHY_SUNI_LITE)
           {
               tbAtm155SuniInterrupt(pAdapter);
           }
           else
           {
               tbAtm155PLC2Interrupt(pAdapter);
           }

           InterruptStatus &= ~TBATM155_INT_PHY_INTR;
       }


       //
       //	If Invalid VC interrupts
       //
       if (InterruptStatus & (TBATM155_INT_RX_UNOPENED_VC | TBATM155_INT_RX_UNKNOWN_VC))
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Detected packets for unknown VC.\n"));

           NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvInvalidVpiVci);
           InterruptStatus &= ~(TBATM155_INT_RX_UNOPENED_VC | TBATM155_INT_RX_UNKNOWN_VC);
       }

       //
       //	If receive Host Access Error interrupt
       //
       if (InterruptStatus & TBATM155_INT_HOST_ACCESS_ERR)
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("InterruptStatus & TBATM155_INT_HOST_ACCESS_ERR\n"));

           InterruptStatus &= ~TBATM155_INT_HOST_ACCESS_ERR;
       }

       //
       //	If receive data FIFO overflow.
       //
       if (InterruptStatus & TBATM155_INT_RX_DATA_FIFO_OVFL)
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Detected receive data FIFO overflow.\n"));

           NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvCellsDropped);

           InterruptStatus &= ~TBATM155_INT_RX_DATA_FIFO_OVFL;
       }

       //
       //	If "Big" receive slots are running low.
       //
	    if (InterruptStatus & 
           (TBATM155_INT_RX_NO_BIG_SLOTS | TBATM155_INT_RX_BIG_SLOTS_LOW))
       {
		    DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
               ("Detected 'BIG' receive slots running low or no slots.\n"));

           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
               ("RemainingReceiveBigSlots: %ld\n",
                 pSar->RecvDmaQ.RemainingReceiveBigSlots));

           tbAtm155QueueRecvBuffersToReceiveSlots(
               pAdapter, 
               RECV_BIG_BUFFER);

           pSar->fBigllSlotsLowOrNone = TRUE;
           InterruptStatus &= ~(TBATM155_INT_RX_NO_BIG_SLOTS | TBATM155_INT_RX_BIG_SLOTS_LOW);
       }

       //
       //	If "Small" receive slots are running low.
       //
	    if (InterruptStatus & 
           (TBATM155_INT_RX_NO_SMALL_SLOTS | TBATM155_INT_RX_SMALL_SLOTS_LOW))
       {
		    DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
               ("Detected 'SMALL' receive slots running low or no slots.\n"));

           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
               ("RemainingReceiveSmallSlots: %ld\n",
                 pSar->RecvDmaQ.RemainingReceiveSmallSlots));

           tbAtm155QueueRecvBuffersToReceiveSlots(
               pAdapter,
               RECV_SMALL_BUFFER);

           pSar->fSmallSlotsLowOrNone = TRUE;
           InterruptStatus &= ~(TBATM155_INT_RX_NO_SMALL_SLOTS | TBATM155_INT_RX_SMALL_SLOTS_LOW);
       }

       //
       //	Check for receiving any packets.
       //
       if (InterruptStatus & TBATM155_INT_RX_IOC)
       {
           fTmpReceived = tbAtm155RxInterruptOnCompletion(pAdapter);

           //
           // Should not overwrite if fReceived has been set to TRUE.
           //
           if (fReceived == FALSE)
               fReceived = fTmpReceived;

           // Clear these flags regardless.
           pSar->fSmallSlotsLowOrNone = FALSE;
           pSar->fBigllSlotsLowOrNone = FALSE;

           InterruptStatus &= ~TBATM155_INT_RX_IOC;
       }

       //
       //	Check to see if packets have been transmitted.
       //
       if (InterruptStatus & TBATM155_INT_TX_IOC)
       {
           tbAtm155TxInterruptOnCompletion(pAdapter);

           InterruptStatus &= ~TBATM155_INT_TX_IOC;
       }

       //
       //	Increment the number of interrupts that we have processed.
       //
       InterruptCount++;
       if (MAXIMUM_INTERRUPTS == InterruptCount)
       {
           //
           //	Break out so we don't ack more interrupts
           //	(and miss them later).
           //
           break;
       }

       //
       //	Read the interrupt status register and see if there is anything
       //	new.
       //
		TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->Intr_Status,
           &InterruptStatus);

       InterruptStatus &= TBATM155_REG_INT_VALID;

       //
       //	Clear the interrupt status register
       //
		TBATM155_WRITE_PORT(
           &pHwInfo->TbAtm155_SAR->Intr_Status,
           InterruptStatus);
	}

	//
	//	Has the adapter received any packets?
	//
	if (fReceived == TRUE)
	{
       //
       //	Issue a receive complete to the transports.
       //
       NdisMCoReceiveComplete(pAdapter->MiniportAdapterHandle);
   }

   //
   //	Was a reset requested?
   //
   NdisDprAcquireSpinLock(&pAdapter->lock);

   ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_PROCESSING_INTERRUPTS);

   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_REQUESTED))
   {
       if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_PROCESSING_SENDPACKETS))
       {
        
           ADAPTER_SET_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS);
           ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_RESET_REQUESTED);

           NdisDprReleaseSpinLock(&pAdapter->lock);

           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_ERR,
                    ("Calling tbAtm155ProcessReset in TbAtm155HandleInterrupt\n"));

           tbAtm155ProcessReset(pAdapter);
           NdisMResetComplete(pAdapter->MiniportAdapterHandle, NDIS_STATUS_SUCCESS, FALSE);
       }
       else
       {
           //
           //   We are in Tbatm155SendPackets(). 
           //   The tbatm155ProcessReset() will be called in sendpackets handler,
           //   so don't enable interrupts now.
           //
           NdisDprReleaseSpinLock(&pAdapter->lock);
       }
   }
   else
   {
       NdisDprReleaseSpinLock(&pAdapter->lock);

       //
       //	Re-enable interrupts.
       //
       NdisMSynchronizeWithInterrupt(
           &pHwInfo->Interrupt,
           (PVOID)TbAtm155EnableInterrupt,
           pAdapter);
   }

}


