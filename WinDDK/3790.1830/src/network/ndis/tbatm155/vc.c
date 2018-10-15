/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       vc.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

	This module contains the routines that allow for the creation, deletion,
	activation and deactivation of virtual circuits.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Cleanup.
								Added support for NdisAllocateMemoryWithTag
								Fixed/cleanup transmit segment allocation code.
								Insert the VC_BLOCK in the hash table before
									the receiver is activated for the VC.
	10/01/96		kyleb		Added async VC completion and async receive
								buffer allocation.
	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.
   02/16/99        hhan        Use synchronize mode to allocate xmit buffer.
   03/04/99        hhan        Added support for MAX_AAL5_

--*/

#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER	MODULE_VC

NDIS_STATUS
TbAtm155CreateVc(
   IN  NDIS_HANDLE         MiniportAdapterContext,
   IN  NDIS_HANDLE         NdisVcHandle,
   OUT PNDIS_HANDLE        MiniportVcContext
   )
/*++

Routine Description:

   This is the NDIS 4.1 handler to create a VC.  This will allocate necessary
   system resources for the VC.

Arguments:

   MiniportAdapterContext  -   Pointer to our ADAPTER_BLOCK.
   NdisVcHandle            -   Handle that NDIS uses to identify the VC that
                               is about to be created.
   MiniportVcContext       -   Storage to hold context information about
                               the newly created VC.

Return Value:

   NDIS_STATUS_SUCCESS     if we successfully create the new VC.
	NDIS_STATUS_RESOURCES   if we are unable to allocate system memory for
                           driver use.

--*/
{
   PADAPTER_BLOCK      pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PVC_BLOCK           pVc;
   NDIS_STATUS         Status;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>TbAtm155CreateVc\n"));

   //
   //	I'm paranoid.
   //
   *MiniportVcContext = NULL;

   //
   //	Allocate memory for the VC.
   //
   ALLOCATE_MEMORY(&Status, &pVc, sizeof(VC_BLOCK), '90DA');
   if (NDIS_STATUS_SUCCESS != Status)
   {
       return(NDIS_STATUS_RESOURCES);
   }

   //
   //	Initialize memory.
   //
   ZERO_MEMORY(pVc, sizeof(VC_BLOCK));

   //
   //	Attempt to allocate storage for the debug logs.
   //
   dbgInitializeDebugInformation(&pVc->DebugInfo);

   //
   //	Save a pointer to the adapter block with the vc.
   //
   pVc->Adapter = pAdapter;
   pVc->HwInfo = pAdapter->HardwareInfo;
   pVc->NdisVcHandle = NdisVcHandle;
   pVc->Sar = pAdapter->HardwareInfo->SarInfo;
   pVc->XmitDmaQ = &pAdapter->HardwareInfo->SarInfo->XmitDmaQ;
   pVc->RecvDmaQ = &pAdapter->HardwareInfo->SarInfo->RecvDmaQ;
   pVc->References = 1;

   NdisAllocateSpinLock(&pVc->lock);

   NdisAcquireSpinLock(&pAdapter->lock);

   //
   //	Add the VC to the adapter's inactive list.
   //
   InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);

   //
   //	This adapter has another reference...
   //
   tbAtm155ReferenceAdapter(pAdapter);

   NdisReleaseSpinLock(&pAdapter->lock);

   //
   //	Return the pointer to the new VC as the context.
   //
   *MiniportVcContext = (PNDIS_HANDLE)pVc;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==TbAtm155CreateVc (Status: 0x%lx)\n", Status));

   return(Status);
}

NDIS_STATUS
TbAtm155DeleteVc(
   IN  NDIS_HANDLE MiniportVcContext
   )
/*++

Routine Description:

   This is the NDIS 4.1 handler to delete a given VC. This routine will
   free any resources that are associated with the VC.  For the VC to
   be deleted it MUST be deactivated first.

Arguments:

   MiniportVcContext   -   Pointer to the VC_BLOCK describing the VC that
                           is to be deleted.

Return Value:

   NDIS_STATUS_SUCCESS     if the VC is successfully deleted.

--*/
{
   PVC_BLOCK       pVc = (PVC_BLOCK)MiniportVcContext;
   PADAPTER_BLOCK  pAdapter = pVc->Adapter;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>TbAtm155DeleteVc, Vc=(%lx)\n", pVc->VpiVci.Vci));

   NdisAcquireSpinLock(&pAdapter->lock);

   NdisDprAcquireSpinLock(&pVc->lock);

   //
   //	Verify that this VC is inactive.
   //
   if (VC_TEST_FLAG(pVc, fVC_ACTIVE) || VC_TEST_FLAG(pVc, fVC_DEACTIVATING))
   {
		DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
			("DeleteVc: Vc is either active [%u] or is deactivating [%u]\n", 
             VC_TEST_FLAG(pVc, fVC_ACTIVE), VC_TEST_FLAG(pVc, fVC_DEACTIVATING)));

       //	Cannot delete a VC that is still active.
       //
       NdisDprReleaseSpinLock(&pVc->lock);
       NdisReleaseSpinLock(&pAdapter->lock);


       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
           ("<==TbAtm155DeleteVc (NDIS_STATUS_FAILURE)\n"));

       return(NDIS_STATUS_FAILURE);
   }

   //
   //	If a VC is deactive then it had better have only the creation
   //	reference count on it!
   //
   ASSERT(1 == pVc->References);

   //
   //	Remove the VC from the inactive list.
   //
   RemoveEntryList(&pVc->Link);

   NdisDprReleaseSpinLock(&pVc->lock);

   //
   //	Dereference the adapter.
   //
   tbAtm155DereferenceAdapter(pAdapter);

   NdisReleaseSpinLock(&pAdapter->lock);

   //
   //	Free any logs that were allocated.
   //
   dbgFreeDebugInformation(pVc->DebugInfo);

   //
   //	Clean up the resources that were allocated on behalf of the
   //	VC_BLOCK.
   //
   NdisFreeSpinLock(&pVc->lock);

#if DBG
   if (VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
   {
       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR, 
               ("FREE_MEMORY: pVc(%lx) of VC(0x%lx) Flags(0x%lx)\n", 
               pVc, pVc->VpiVci.Vci, pVc->Flags));

       DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);
   }           
#endif // end of DBG           

   //
   //	Free the memory that was taken by the vc.
   //
   FREE_MEMORY(pVc, sizeof(VC_BLOCK));

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==TbAtm155DeleteVc (NDIS_STATUS_SUCCESS)\n"));

   return(NDIS_STATUS_SUCCESS);
}


ULONG
tbAtm155NextPowerOf2(
   ULONG RequestedSize
	)
/*++

Routine Description:

   If the RequestedSize is not a power of 2 then this routine will find the
   next highest...

Arguments:

   RequestedSize   -   Value to find the power of 2 for.

Return Value:

--*/
{
   ULONG	Value;

   if ((RequestedSize > 0) && (RequestedSize <= BLOCK_1K))
   {
       Value = BLOCK_1K;
   }
   else if ((RequestedSize > BLOCK_1K) && (RequestedSize <= BLOCK_2K))
   {
       Value = BLOCK_2K;
   }
   else if ((RequestedSize > BLOCK_2K) && (RequestedSize <= BLOCK_4K))
   {
       Value = BLOCK_4K;
   }
   else if ((RequestedSize > BLOCK_4K) && (RequestedSize <= BLOCK_8K))
   {
       Value = BLOCK_8K;
   }
   else if ((RequestedSize > BLOCK_8K) && (RequestedSize <= BLOCK_10K))
   {
       Value = BLOCK_10K;
   }
   else
   {
       Value = BLOCK_16K;
   }

   return(Value);

}


NDIS_STATUS
tbAtm155WriteEntryOfTheSramTbl(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  ULONG           Dest,
   IN  PUSHORT         pPhData,
   IN	USHORT          LengthOfEntry
	)
/*++

Routine Description:

   This routine will write data to the entry to the specified table
   on on-board SRAM.

Arguments:

   pAdapter            -   Pointer to the adapter to program.
   Dest                -   Point to the entry of the table on SRAM
   pPhData             -   Point to the starting address of data array
                           to be written onto SRAM.
   LengthOfEntry       -   Number of words to be written to the table.

Return Value:

   NDIS_STATUS_SUCCESS     if the entry of VC state table is opened
                           successfully.

--*/
{
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

//
//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
//       ("==>tbAtm155WriteEntryOfTheSramTbl\n"));
//
//
//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
//            ("Dest=0x%lx, pPhData=0x%lx, LengthOfEntry=%d\n",
//              Dest, pPhData, LengthOfEntry));
//

   for (
       ;
       (LengthOfEntry > 0) && (NDIS_STATUS_SUCCESS == Status);
       LengthOfEntry--, Dest++, pPhData++)
   {

       TBATM155_PH_WRITE_SRAM(pAdapter, Dest, *pPhData, &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
	        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
		        ("Failed to open the entry of the Vc state table.\n") );

           break;
       }
   }

//
//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
//       ("<==tbAtm155WriteEntryOfTheSramTbl.\n"));
//

   return (Status);

}



NDIS_STATUS
tbAtm155AdjustTrafficParameters(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PATM_FLOW_PARAMETERS    pFlow,
   IN  BOOLEAN                 fRoundFlowUp,
   OUT PULONG                  pPreScaleVal,
   OUT PULONG                  pNumOfEntries
   )
/*++

Routine Description:

Arguments:

Return Value:

   NDIS_STATUS_SUCCESS     if we successfully allocate the bandwidth.
	NDIS_STATUS_FAILURE     if we are unable to allocate bandwidth for 
                           the flow parameters.

--*/
{
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   ULONG           PreScale_Val;
   ULONG           NumOfEntriesNeeded;
   ULONG           ActualPeakCellRate;
   BOOLEAN         fDone;
   ULONG           c;
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>tbAtm155AdjustTrafficParameters\n"));

   do
   {

#if    AW_QOS
       //
       //	The Number of available CBR VCs
       //  9/17/97     - Please read the note where declares this
       //                variable (in SW.H).
       //	
       if (pAdapter->NumOfAvailableCbrVc == 0)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("There is not enough remaining bandwidth to support even the minimum required bandwidth\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

#endif // end of AW_QOS

       //
       //	Verify the low end of the bandwidth
       //
       if (pAdapter->RemainingTransmitBandwidth < pAdapter->MinimumCbrBandwidth)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("There is not enough remaining bandwidth to support even the minimum required bandwidth\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       //
       //	Make sure we can satisfy the peak cell rate.
       //
       if (pFlow->PeakCellRate < pAdapter->MinimumCbrBandwidth)
           pFlow->PeakCellRate = pAdapter->MinimumCbrBandwidth;

       if ((pFlow->PeakCellRate > pAdapter->RemainingTransmitBandwidth) ||
           (pFlow->PeakCellRate < pAdapter->MinimumCbrBandwidth))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("PeakCellRate is not within remaining and minimum\n"));

           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("PeakCellRate %u, RemainingBW %u\n", pFlow->PeakCellRate, pAdapter->RemainingTransmitBandwidth));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       //
       //	Determine the pre-scale for the required cell rate.
	    //	We find a closer value.
	    //
       PreScale_Val = 0;
       NumOfEntriesNeeded = 1;
       if (pFlow->PeakCellRate < pHwInfo->CellClockRate)
       {
           //
           //  Rate    = Schedule_table_rate / (PreScale_Val + 1);
           //  PreScale_Val = (Schedule_table_rate / Rate) - 1;
           //  
           PreScale_Val = pHwInfo->CellClockRate / pFlow->PeakCellRate;
           c = pHwInfo->CellClockRate % pFlow->PeakCellRate;
           if ((c == 0) || (c < (pFlow->PeakCellRate/2)))
           {
               //
               // if c = 0 --> Actual PCR == pFlow->PeakCellRate
               //
               // if c < (pFlow->PeakCellRate/2)
               //           --> Actual PCR > pFlow->PeakCellRate
               // if c > (pFlow->PeakCellRate/2)
               //           --> Actual PCR < pFlow->PeakCellRate
               //
               PreScale_Val--;
           }

           if (PreScale_Val > MAX_PRESCALE_VAL)
           {
               PreScale_Val = MAX_PRESCALE_VAL;
           }
       }
       else
       {
           // 
           // Calculate the number of entries are needed in CBR schedule table.
           // 
           NumOfEntriesNeeded = pFlow->PeakCellRate / pHwInfo->CellClockRate;
           c = pFlow->PeakCellRate % pHwInfo->CellClockRate;

           if (c > (pFlow->PeakCellRate / 2))
           {
               //
               // if c = 0 --> Actual PCR == pFlow->PeakCellRate
               //
               // if c < (pFlow->PeakCellRate/2)
               //           --> Actual PCR < pFlow->PeakCellRate
               // if c > (pFlow->PeakCellRate/2)
               //           --> Actual PCR > pFlow->PeakCellRate
               //
               NumOfEntriesNeeded++;
           }
       }


       //
       //	Do any rounding necessary....
       fDone = FALSE;

       do
       {
           //
           //	Determine the actual PCR.
           //
           ActualPeakCellRate = (pHwInfo->CellClockRate * NumOfEntriesNeeded) /
                                (PreScale_Val + 1);

           //
           //	Round up or down?
           //
           if (fRoundFlowUp)
           {
               if (ActualPeakCellRate < pFlow->PeakCellRate)
               {
                   if (PreScale_Val > 0)
                   {
                       PreScale_Val--;
                   }
                   else
                   {
                       NumOfEntriesNeeded++;
                   }
               }
               else
               {
                   fDone = TRUE;
               }
           }
           else    
           {
               // 
               // Round flow down 
               // 
               if (ActualPeakCellRate > pFlow->PeakCellRate)
               {
                   if (NumOfEntriesNeeded > 1)
                   {
                       NumOfEntriesNeeded--;
                   }
                   else
                   {       
                       if (PreScale_Val < MAX_PRESCALE_VAL)
                       {
                           PreScale_Val++;
                       }
                       else
                       {
                           fDone = TRUE;
                       }
                   }
               }
               else
               {
                   fDone = TRUE;
               }
           }
	    } while (!fDone);

       ActualPeakCellRate = (pHwInfo->CellClockRate * NumOfEntriesNeeded) /
                            (PreScale_Val + 1);

       if ((ActualPeakCellRate > pAdapter->MinimumCbrBandwidth) &&
           (ActualPeakCellRate < pAdapter->RemainingTransmitBandwidth))
       {
           if ((ULONG)(MAX_ENTRIES_CBR_TBL - NumOfEntriesNeeded) > 
                      pAdapter->TotalEntriesUsedonCBRs)
           {
               *pPreScaleVal = PreScale_Val;
               *pNumOfEntries = NumOfEntriesNeeded;

               //
               //	Save the actual cell rate with the flow parameters...
               //	
               pFlow->PeakCellRate = ActualPeakCellRate;
           }
           else
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Not enough entries on CBR schedule table.!\n"));
               DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);

               Status = NDIS_STATUS_FAILURE;
           }
       }
       else
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("The adjust bandwidth is out of supporting range.!\n"));
           DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);

           Status = NDIS_STATUS_FAILURE;
       }

   } while (FALSE);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155AdjustTrafficParameters (Status: 0x%lx)\n", Status));

   return(Status);
}


NDIS_STATUS
tbAtm155SetCbrTblsInSRAM(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc,
   IN  BOOLEAN         fOpenVC
	)
/*++

Routine Description:

   This routine will setup the tables for CBR flow on on-board SRAM.

Arguments:

   pAdapter    -   Pointer to the adapter to program.
   pVc         -   Pointer to the VC_BLOCK that describes the VC we
                   are handling.
   fOpenVC     -   TRUE:  add more entries for the VC to CBR schedule table
                   FALSE: eliminate entries for the VC from CBR schedule table

Return Value:

   NDIS_STATUS_SUCCESS     if successful.

--*/
{
   PHARDWARE_INFO                  pHwInfo = pAdapter->HardwareInfo;
   TB155PCISAR_CNTRL1              regControl1;
   TBATM155_CBR_SCHEDULE_ENTRY     phData;
   ULONG                           pCurrCbrTbl, pStCbrTbl, pAddr;
   USHORT                          Idx;
   NDIS_STATUS                     Status;
   ULONG                           pEndCbrTbl;
   USHORT                          CellInterval, Curr_CI;
   USHORT                          Count_Fract_CI, Indx_Fract_CI;
   ULONG                           pTemp;
   BOOLEAN                         fFound1stEntry = FALSE;
   TBATM155_CBR_SCHEDULE_ENTRY     phOpenVc;
   ULONG                           pEntryAddr_1st;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("==>tbAtm155SetCbrTblsInSRAM\n"));

   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
       &regControl1.reg);

   if (regControl1.CBR_ST_Sel == 0)
   {
       //
       // 1. Set CBR schedule table 1 is currently used.
       // 2. Set the CBR schedule table 2 is selected. 
       // 3. Set CBR_ST_Sel bit to select CBR schedule table 2.
       //
       pCurrCbrTbl = pHwInfo->pSramCbrScheduleTbl_1;
       pStCbrTbl  = pHwInfo->pSramCbrScheduleTbl_2;
       regControl1.CBR_ST_Sel = 1;
   }
   else
   {
       //
       // 1. Set CBR schedule table 2 is Currently used.
       // 2. Set the CBR schedule table 1 is selected.
       // 3. Set CBR_ST_Sel bit to select CBR schedule table 1.
       //
       pCurrCbrTbl = pHwInfo->pSramCbrScheduleTbl_2;
       pStCbrTbl  = pHwInfo->pSramCbrScheduleTbl_1;
       regControl1.CBR_ST_Sel = 0;
   }

   do
   {   
       // 
       // 1. Copy contents of Schedule tables from "Current" to "Select"
       // 2. Find the 1st "active=0" entry as well.
       // 
       for (Idx = 0, pAddr = pStCbrTbl;
            Idx < MAX_CBR_SCHEDULE_ENTRIES;
            Idx++, pCurrCbrTbl++, pAddr++)
       {
           TBATM155_PH_READ_SRAM(pAdapter, pCurrCbrTbl, &phData.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed rd CBR Schedule table (copy)! \n"));
               break;
           }
       
           if ((fOpenVC == FALSE) && (phData.VC == pVc->VpiVci.Vci))
           {
               //
               // if this entry is not void or the VC is de-activated.
               //
               phData.data = 0;
           }

           if (fFound1stEntry == FALSE)
           {
               if (phData.Active == 0)
               {
                   // 
                   // Found the 1st of the "active=0 entry.
                   // 1. remember the address 
                   // 2. set the flag to indicate the entry has been found.
                   // 
                   pEntryAddr_1st = pAddr;
                   fFound1stEntry = TRUE;
               }
           }

           //
           // copy this entry to destination also reset the EOT regardless.
           //
           phData.EOT = 0;
           TBATM155_PH_WRITE_SRAM(pAdapter, pAddr, phData.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed wr CBR Schedule table (Copy)! \n"));
               break;
           }

       } // end of FOR

       //
       //  Remember the end of destination CBR table to use later.
       //
       pEndCbrTbl = pAddr - 1;

       if ((NDIS_STATUS_SUCCESS != Status) || (fOpenVC != TRUE))
       {
           //
           // Exit, due to either
           // 1. error detected, or
           // 2. don't need to handle activate any entries for a new VC.
           //
           break;
       }

       //
       // Calculate all of the variables for handling CDV, Cell Delay
       // Variation, correctly.
       //
       CellInterval = (USHORT)(MAX_CBR_SCHEDULE_ENTRIES / pVc->CbrNumOfEntris);
       Curr_CI = CellInterval;

       Count_Fract_CI = 0;
       if (MAX_CBR_SCHEDULE_ENTRIES % pVc->CbrNumOfEntris)
       {
           // 
           // Handle the fraction of the difference between the theorical
           // interval and actual interval on CBR schedule table.
           // 
           Count_Fract_CI = (USHORT) (pVc->CbrNumOfEntris / 
                                     (MAX_CBR_SCHEDULE_ENTRIES % pVc->CbrNumOfEntris));
           Count_Fract_CI = Count_Fract_CI + 1;
           Indx_Fract_CI = Count_Fract_CI;
       }

       //
       // Set up the necessary entry data of the activated VC.
       //
       phOpenVc.data = 0;
       phOpenVc.VC = (USHORT)pVc->VpiVci.Vci;
       phOpenVc.Active = 1;

       // 
       // Set up the entries of the VC
       // 
       for (pAddr = pEntryAddr_1st, Idx = (USHORT)pVc->CbrNumOfEntris;
            Idx > 0;
            Curr_CI = CellInterval, Idx--)
       {
           TBATM155_PH_READ_SRAM(pAdapter, pAddr, &phData.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed wr CBR Schedule table (Add more Entries)! \n"));
               break;
           }

           if (phData.Active == 1)
           {
               //
               // Let's find an available CBR entry around this pAddr.
               //
               for (pTemp = 1; TRUE; pTemp++)
               {
                   //
                   // Let's looking for available entry around 
                   // pAddr (+/-) pTemp
                   // Make sure not over the beginning and the end of 
                   // the CBR table.
                   //
                   if ((pAddr + pTemp) <= pEndCbrTbl)
                   {
                       //
                       // Check if (pAddr + pTemp) entry is available
                       //
                       TBATM155_PH_READ_SRAM(
                           pAdapter, 
                           (pAddr + pTemp), 
                           &phData.data, 
                           &Status);

                       if (NDIS_STATUS_SUCCESS != Status)
                       {
                           DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                               ("Failed wr CBR Schedule table (Add more Entries)! \n"));
                           break;
                       }

                       if (phData.Active == 0)
                       {
                           //
                           // Adjust Cell interval for the next entry.
                           //
                           pAddr += pTemp;
                           if (Curr_CI > (USHORT) pTemp)
                           {
                               Curr_CI = (USHORT)(Curr_CI - (USHORT) pTemp);
                           }
                           else
                           {
                               Curr_CI = 1;
                           }
                           break;
                       }
                   }

                   if ((pAddr - pTemp) >= pStCbrTbl) 
                   {
                       //
                       // Check if (pAddr - pTemp) entry is available
                       //
                       TBATM155_PH_READ_SRAM(
                           pAdapter, 
                           (pAddr - pTemp), 
                           &phData.data, 
                           &Status);

                       if (NDIS_STATUS_SUCCESS != Status)
                       {
                           DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                               ("Failed wr CBR Schedule table (Add more Entries)! \n"));
                           break;
                       }

                       if (phData.Active == 0)
                       {
                           //
                           // Adjust Cell interval for the next entry.
                           //
                           pAddr -= pTemp;
                           Curr_CI += (USHORT) pTemp;
                           break;
                       }
                   }

                   if (((pAddr + pTemp) > pEndCbrTbl) || ((pAddr - pTemp) < pStCbrTbl))
                   {
                       Status = NDIS_STATUS_FAILURE;

                       DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_ERR,
                               ("Failed to write CBR Schedule table! \n"));

                       DBGBREAK(DBG_COMP_SPECIAL, DBG_LEVEL_FATAL);

                       break;
                   }
               } // end of FOR (pTemp = 1; TRUE; pTemp++)
           } // end of (phData.Active == 1)

           if (NDIS_STATUS_SUCCESS != Status)
           {
               break;
           }

           TBATM155_PH_WRITE_SRAM(pAdapter, pAddr, phOpenVc.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed wr CBR Schedule table (Copy)! \n"));
               break;
           }

           if (Count_Fract_CI > 0) 
           {
               Indx_Fract_CI--;
               if (Indx_Fract_CI == 0)
               {
                   Indx_Fract_CI = Count_Fract_CI;
                   Curr_CI++;
               }
           }

           pAddr += (ULONG) Curr_CI;
           if (pAddr > pEndCbrTbl)
           {
               //
               // wrap around. But we should never get into this case.
               //
               pAddr = pEntryAddr_1st + (CellInterval / 2);
           }       

       } // end of FOR 

       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }
   
   } while (FALSE);

   if (NDIS_STATUS_SUCCESS == Status)
   {
       do
       {
           //
           // Set EOT to the CBR table now.
           //
           TBATM155_PH_READ_SRAM(pAdapter, pEndCbrTbl, &phData.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed rd CBR Schedule table (copy)! \n"));
               break;
           }

           phData.EOT = 1;
       
           TBATM155_PH_WRITE_SRAM(pAdapter, pEndCbrTbl, phData.data, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Failed wr CBR Schedule table (Add more Entries)! \n"));
               break;
           }

           //
           // Select to the new setup CBR schedule table.
           //
           TBATM155_WRITE_PORT(
               &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
               regControl1.reg);

       } while (FALSE);

   }

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("<==tbAtm155SetCbrTblsInSRAM (Status: 0x%lx)\n", Status));

   return(Status);
}


NDIS_STATUS
tbAtm155SetUbrTblsInSRAM(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
	)
/*++

Routine Description:

   This routine will setup the tables for UBR flow on on-board SRAM.

Arguments:

   pAdapter    -   Pointer to the adapter to program.
   pVc         -   Pointer to the VC_BLOCK that describes the VC we
                   are handling.

Return Value:

   NDIS_STATUS_SUCCESS     if the entry of VC state table is opened
                           successfully.

--*/
{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   PATM_FLOW_PARAMETERS    pXmitFlow = &pVc->Transmit;
   ULONG                   rate_FP = 0;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("==>tbAtm155SetUbrTblsInSRAM\n"));

   do
   {
       //
       // Set PCR for UBR to Word0.Acr field.
       // Convert PCR to Floating Point.
       // 
       tbAtm155ConvertToFP(&pXmitFlow->PeakCellRate, &(USHORT)rate_FP);

       TBATM155_PH_WRITE_SRAM(
           pAdapter,
           (pHwInfo->pSramAbrValueTbl + (pVc->VpiVci.Vci * SIZEOF_ABR_VALUE_ENTRY)),
           (ULONG)rate_FP,
           &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Failed to write word0.Acr of ABR value table.\n"));
           break;
       }

       TBATM155_PH_WRITE_SRAM(
           pAdapter,
           (pHwInfo->pSramAbrValueTbl + 1 + (pVc->VpiVci.Vci * SIZEOF_ABR_VALUE_ENTRY)),
           0,
           &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Failed to write word1.Fraction of ABR value table.\n"));
       }

   } while (FALSE);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("<==tbAtm155SetUbrTblsInSRAM (Status: 0x%lx)\n", Status));

   return(Status);
}


NDIS_STATUS
tbAtm155SetAbrTblsInSRAM(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
	)
/*++

Routine Description:

   This routine will setup the tables for ABR flow on on-board SRAM.

Arguments:

   pAdapter    -   Pointer to the adapter to program.
   pVc         -   Pointer to the VC_BLOCK that describes the VC we
                   are handling.

Return Value:

   NDIS_STATUS_SUCCESS     if the entry of VC state table is opened
                           successfully.

--*/
{
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;


//=================================
#if    ABR_CODE
//=================================

   USHORT                  rate_FP;
   USHORT                  logVal;
   ULONG                   temp;
   ULONG                   phAddr;
   TBATM155_ABR_PARAM_ENTRY    AbrParamEntry;



   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("==>tbAtm155SetAbrTblsInSRAM\n"));

   do
   {
       //
       //	Verify the low end of the bandwidth
       //
       if (pAdapter->RemainingTransmitBandwidth < pAdapter->MinimumAbrBandwidth)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("There is not enough remaining bandwidth to support even the minimum required bandwidth\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       //
       //	Make sure we can satisfy the minimum cell rate.
       //
       if (pFlow->PeakCellRate < pAdapter->pAdapter->MinimumAbrBandwidth)
           pFlow->PeakCellRate = pAdapter->pAdapter->MinimumAbrBandwidth;

       if ((pFlow->MinimumCellRate > pAdapter->RemainingTransmitBandwidth) ||
           (pFlow->MinimumCellRate < pAdapter->MinimumAbrBandwidth))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("CellRate is not within remaining and minimum\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       if ((pFlow->MissingRMCellCount < TBATM155_MINIMUM_CRM) || 
           (pFlow->MissingRMCellCount > TBATM155_MAXIMUM_CRM))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Missing RM cell count is not in range.\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       //
       // Get the log value of Crm
       //
       for (logVal = 0, temp = 2;
            logVal <= TBATM155_MAXIMUM_LOG_CRMX;
            logVal++, temp *= 2)
       {
           if (temp <= pFlow->MissingRMCellCount)
           {
               // BINGO!!
               break;
           }
       }

       NdisZeroMemory (
           (PVOID)(&AbrParamEntry),
           sizeof(TBATM155_ABR_PARAM_ENTRY));

       AbrParamEntry.AbrParamW0.Crmx = logVal;
       AbrParamEntry.AbrParamW0.RDFx = pFlow->RateDecreaseFactor;
       AbrParamEntry.AbrParamW0.CDFx = pFlow->CutoffDecreaseFactor;

       // 
       // Convert PCR to Floating Point.
       // 
       temp = (ULONG)(pFlow->PeakCellRate * pFlow->RateIncreaseFactor);
       tbAtm155ConvertToFP(&temp, &rate_FP);
       AbrParamEntry.AbrParamW1.AIR = rate_FP;

       tbAtm155ConvertToFP(&pFlow->InitialCellRate, &rate_FP);
       AbrParamEntry.AbrParamW2.ICR = rate_FP;

       tbAtm155ConvertToFP(&pFlow->MinimumCellRate, &rate_FP);
       AbrParamEntry.AbrParamW3.MCR = rate_FP;

       tbAtm155ConvertToFP(&pFlow->PeakCellRate, &rate_FP);
       AbrParamEntry.AbrParamW4.PeakCellRate = rate_FP;

       //
       // Calculate the starting address of the entry for the ABR VC
       //
       phAddr = pHwInfo->pABR_Parameter_Tbl +
                        (pVc->VpiVci.Vci * SIZEOF_ABR_PARAM_ENTRY);


       //
       //  Set the entry of Tx Vc State table.
       //
       Status = tbAtm155WriteEntryOfTheSramTbl(
                   pAdapter,
                   phAddr,
                   (PUSHORT)&AbrParamEntry,
                   SIZEOF_ABR_PARAM_ENTRY);

	    if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Failed to fill the entry in ABR parameter table in SRAM.\n"));

           break;
       }

   } while (FALSE);


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("<==tbAtm155SetAbrTblsInSRAM (Status: 0x%lx)\n", Status));

   return(Status);



//=================================
#else     // not ABR_CODE
//=================================


   UNREFERENCED_PARAMETER(pAdapter);
   UNREFERENCED_PARAMETER(pVc);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("==>tbAtm155SetAbrTblsInSRAM\n"));


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("<==tbAtm155SetAbrTblsInSRAM (Status: 0x%lx)\n", Status));

   return(Status);

//=================================
#endif     // end of ABR_CODE
//=================================

}


VOID
tbAtm155FreeReceiveSegment(
   IN  PVC_BLOCK       pVc
   )
/*++

Routine Description:

   This routine will clean up a receive segment that was (partially)
   allocated.

Arguments:

   pVc -   Pointer to the VC_BLOCK that the receive segment belongs to.

Return Value:

   None.

--*/
{
   PADAPTER_BLOCK          pAdapter = pVc->Adapter;
   PHARDWARE_INFO	        pHwInfo = pVc->HwInfo;
   PRECV_SEG_INFO	        pRecvSegInfo;
   NDIS_STATUS             Status;
   PRECV_BUFFER_QUEUE      pFreeBufQ;

   //
   //	Did we get a receive segment allocated?
   //
   if (NULL != pVc->RecvSegInfo)
   {
       pRecvSegInfo = pVc->RecvSegInfo;

#if   DBG
       if (NULL != pRecvSegInfo->SegCompleting.BufListHead)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
           ("There are still Rx buffer queued in the list.\n"));
           DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);
       }
#endif    // end of DBG

       if (pRecvSegInfo->pEntryOfRecvState)
       {
           //
           //  Clear Open flag of the VC entry in Rx State table
           //  in SRAM to indicate that VC has been closed by the driver.
           //

           pRecvSegInfo->InitRecvState.RxStateWord0.Open = 0;

           TBATM155_PH_WRITE_SRAM(
               pAdapter,
               pRecvSegInfo->pEntryOfRecvState,
               pRecvSegInfo->InitRecvState.RxStateWord0.data,
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("Failed to close the VC in the Rx VC State table in SRAM.\n"));
           }
       }

       //
       //  Check if any Rx buffers are pending for this Vc
       //
       if (NULL != pRecvSegInfo->SegCompleting.BufListHead)
       {
           //
           //  Free up the pending Rx buffer(s) to Free list.
           //
           pFreeBufQ = &pHwInfo->SarInfo->FreeBigBufferQ;
           if (pVc->RecvBufType == RECV_SMALL_BUFFER)
           {
               pFreeBufQ = &pHwInfo->SarInfo->FreeSmallBufferQ;
           }

           tbAtm155MergeRecvBuffers2FreeBufferQueue(
                   pAdapter,
                   pFreeBufQ,
                   &pRecvSegInfo->SegCompleting);
       }

       //
       //	Free the receive buffer information.
       //
       if (NULL != pRecvSegInfo->pRecvBufferInfo)
       {
           tbAtm155FreeReceiveBufferInfo(
               pVc->Adapter,
               pRecvSegInfo->pRecvBufferInfo);
       }

       NdisFreeSpinLock(&pRecvSegInfo->lock);
       FREE_MEMORY(pRecvSegInfo, sizeof(RECV_SEG_INFO));
       pVc->RecvSegInfo = NULL;
   }
}


NDIS_STATUS
tbAtm155AllocateReceiveSegment(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PVC_BLOCK               pVc,
   IN  PATM_MEDIA_PARAMETERS   pAtmMediaParms
   )
/*++

Routine Description:

   This routine will allocate a receive segment for the VC.

Arguments:

   pAdapter        -   Pointer to the ADAPTER_BLOCK.
   pVc             -   Pointer to the VC_BLOCK.
   pAtmMediaParms  -   Pointer to the ATM_MEDIA_PARAMETERS that describe
                       the characteristics of the VC.

Return Value:

   NDIS_STATUS_INVALID_DATA    if the media parameters are incorrect.
   NDIS_STATUS_RESOURCES       if memory cannot be allocated.
   NDIS_STATUS_SUCCESS         if we have successfully allocated and
                               activated the receiver.

--*/
{
   NDIS_STATUS				        Status = NDIS_STATUS_SUCCESS;
   PHARDWARE_INFO			        pHwInfo = pAdapter->HardwareInfo;
   PATM_FLOW_PARAMETERS	        pRecvFlow = &pAtmMediaParms->Receive;
   PSAR_INFO                       pSar = pHwInfo->SarInfo;
   ULONG					        RcvSegmentSize;
   PRECV_SEG_INFO			        pRecvSegInfo = NULL;
   PTBATM155_RX_STATE_ENTRY        pRecvState;
   PTBATM155_REGISTRY_PARAMETER    pRegistryParameters;
   ULONG                           MaxReceiveBufferSize;


//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>tbAtm155AllocateReceiveSegment\n"));

   do
   {

       //
       //	Get a local copy of the receive buffer size.
       //
       MaxReceiveBufferSize =
           pAdapter->RegistryParameters[TbAtm155BigReceiveBufferSize].Value;

       //
       //	Verify PDU.
       //

       if ((0 == pRecvFlow->MaxSduSize) ||               
           (MAX_AAL5_PDU_SIZE < pRecvFlow->MaxSduSize))  

       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid Sdu size in pRecvFlow->MaxSduSize (0x%lx)\n",
                 pRecvFlow->MaxSduSize));

           Status = NDIS_STATUS_INVALID_DATA;
           break;
       }

       //
       //	Allocate memory for the receive segment information.
       //
       ALLOCATE_MEMORY(&Status, &pRecvSegInfo, sizeof(RECV_SEG_INFO), '01DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Unable to allocate storage for the receive segment information\n"));

           break;
       }

       //
       //	Initialize the receive segment information.
       //
       ZERO_MEMORY(pRecvSegInfo, sizeof(RECV_SEG_INFO));
       NdisAllocateSpinLock(&pRecvSegInfo->lock);

       //
       //	Save the receive segment information with the VC.
       //
       pVc->RecvSegInfo = pRecvSegInfo;

       //
       //	Save the receive flow parameters.
       //
       pVc->Receive = *pRecvFlow;

       //
       //	Align the receive segment size. 
       //
       RcvSegmentSize = tbAtm155NextPowerOf2(pVc->Receive.MaxSduSize);

       //
       //	Fill in the local VCI table entry but do not yet program/enable
       //	any hardware.
       //
       pRecvSegInfo->pAdapter = pAdapter;
       pRecvSegInfo->SegmentSize = RcvSegmentSize;

       //
       //	Poin to small and big free receive buffer queues.
       //
       pRecvSegInfo->FreeSmallBufQ = &pSar->FreeSmallBufferQ;
       pRecvSegInfo->FreeBigBufQ = &pSar->FreeBigBufferQ;

       //
       //	Allocate a receive buffer information structure.
       //
       Status = tbAtm155AllocateReceiveBufferInfo(
                   &pRecvSegInfo->pRecvBufferInfo,
                   pVc);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Failed to allocate RECV_BUFFER_INFO for a VC.\n"));

           break;
       }

       //
       //	By setting this flag we know that we need to complete
       //	the VC activation once the receive pool has been allocated.
       //
       RECV_INFO_SET_FLAG(
           pRecvSegInfo->pRecvBufferInfo,
           fRECV_INFO_COMPLETE_VC_ACTIVATION);

       //
       //  Get the starting address of the VC entry of Rx State table.
       //
       pRecvSegInfo->pEntryOfRecvState = 
                       pHwInfo->pSramRxVcStateTbl +
                       (pVc->VpiVci.Vci * SIZEOF_RX_STATE_ENTRY);

       pRecvState = &pRecvSegInfo->InitRecvState;

       NdisZeroMemory (
           (PVOID)(pRecvState),
           sizeof(TBATM155_RX_STATE_ENTRY));

       //
       //  Set Open in the init value to indicate the Vc has been
       //  opened by driver.
       //
       pRecvState->RxStateWord4.Rx_AAL5_CRC_Low  = 0x0FFFF;
       pRecvState->RxStateWord5.Rx_AAL5_CRC_High = 0x0FFFF;

       //
       //  Set Open in the init value to indicate the Vc has been
       //  opened by driver later
       //
       pRecvState->RxStateWord0.Open = 1;

       pRegistryParameters = pAdapter->RegistryParameters;

       if (RcvSegmentSize > (pRegistryParameters[TbAtm155SmallReceiveBufferSize].Value))
       {

           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Allocate Big receive buffers\n"));

           //
           //  uses big buffers for this VC.
           //
	        pVc->RecvBufType = RECV_BIG_BUFFER;

           //
           //  Initialize register values of Rx_State_Entry (in Off-Chip RAM)
           //
           pRecvState->RxStateWord0.Slot_Type = 1;


           if (pSar->AllocatedBigRecvBuffers >= pSar->MaxRecvBufferCount)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("AllocatedBigRecvBuffers >= MaxRecvBufferCount\n"));
               Status = NDIS_STATUS_SUCCESS;
               break;
           }

           //
           //	Allocate the receive buffer pool.
           //
           Status = tbAtm155AllocateReceiveBufferPool(
                       pAdapter,
                       pRecvSegInfo->pRecvBufferInfo,
                       (ULONG)DEFAULT_RECEIVE_BUFFERS,
                       pRegistryParameters[TbAtm155BigReceiveBufferSize].Value);
       }
       else
       {

           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Allocate Small receive buffers\n"));

           //
           //  uses small buffers for this VC.
           //
	        pVc->RecvBufType = RECV_SMALL_BUFFER;

           //
           //  The Rx_State_Entry in Off-Chip RAM has been set to init values.
           //
           //  pRecvState->Slot_Type = 0;
           //

           if (pSar->AllocatedSmallRecvBuffers >= pSar->MaxRecvBufferCount)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("AllocatedSmallRecvBuffers >= MaxRecvBufferCount\n"));
               Status = NDIS_STATUS_SUCCESS;
               break;
           }

           //
           //	Allocate the receive buffer pool.
           //
           Status = tbAtm155AllocateReceiveBufferPool(
                       pAdapter,
                       pRecvSegInfo->pRecvBufferInfo,
                       (ULONG)DEFAULT_RECEIVE_BUFFERS,
                       pRegistryParameters[TbAtm155SmallReceiveBufferSize].Value);
       }

       if (NDIS_STATUS_PENDING != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Can't to allocate RECV_BUFFER_POOL for VC %u\n", pVc->VpiVci.Vci));

           break;
       }
		
   } while (FALSE);

   //
   //	If there was a failure then clean up.
   //
   if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
   {
       tbAtm155FreeReceiveSegment(pVc);
   }

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateReceiveSegment (Status: 0x%lx)\n", Status));

   return(Status);
}

VOID
tbAtm155FreeTransmitSegment(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
	)
/*++

Routine Description:

   This routine will free up transmit resources for the VC.

Arguments:

Return Value:

--*/
{
   PXMIT_SEG_INFO          pXmitSegInfo;
   PATM_FLOW_PARAMETERS    pXmitFlow = &pVc->Transmit;
   NDIS_STATUS             Status;

   //
   //	Did we get a transmit segment allocated?
   //
   if (NULL != pVc->XmitSegInfo)
   {
       pXmitSegInfo = pVc->XmitSegInfo;

 #if   DBG
       if ((pXmitSegInfo->DmaCompleting.References) ||
           (pXmitSegInfo->SegWait.References))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
           ("There are still Tx buffer queued in the list.\n"));
           DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);
       }
 #endif    // end of DBG


//=================================
#if    ABR_CODE
//=================================


       if (NULL != pXmitFlow)
       {
           switch (pXmitFlow->ServiceCategory) {

               case ATM_SERVICE_CATEGORY_CBR:
                   //
                   // Modify the CBR schedule table accordingly.
                   //
                   Status = tbAtm155SetCbrTblsInSRAM(pAdapter, pVc, FALSE);

                   //
                   //  re-adjust Trasnmit remaining bandwidth.
                   //
                   pAdapter->RemainingTransmitBandwidth += pXmitFlow->PeakCellRate;
                   pAdapter->TotalEntriesUsedonCBRs -= pVc->CbrNumOfEntris;
                   pVc->CbrNumOfEntris = 0;
                   pVc->CbrPreScaleVal = 0;

#if    AW_QOS
                   //
                   //	The Number of available CBR VCs
                   //  9/17/97     - Please read the note where declares this
                   //                variable (in SW.H).
                   //	
                   pAdapter->NumOfAvailableCbrVc++;

#endif // end of AW_QOS

                   break;

               case ATM_SERVICE_CATEGORY_ABR:
                   pAdapter->RemainingTransmitBandwidth += pXmitFlow->MinimumCellRate;
                   break;
           }

       }


//=================================
#else     // not ABR_CODE
//=================================


       if ((NULL != pXmitFlow) &&
           (ATM_SERVICE_CATEGORY_CBR == pXmitFlow->ServiceCategory))
       {
           //
           // Modify the CBR schedule table accordingly.
           //
           Status = tbAtm155SetCbrTblsInSRAM(pAdapter, pVc, FALSE);

           //
           //  re-adjust Trasnmit remaining bandwidth.
           //
           pAdapter->RemainingTransmitBandwidth += pXmitFlow->PeakCellRate;
           pAdapter->TotalEntriesUsedonCBRs -= pVc->CbrNumOfEntris;
           pVc->CbrNumOfEntris = 0;
           pVc->CbrPreScaleVal = 0;

#if    AW_QOS

           //
           //	The Number of available CBR VCs
           //  9/17/97     - Please read the note where declares this
           //                variable (in SW.H).
           //	
           pAdapter->NumOfAvailableCbrVc++;

#endif // end of AW_QOS

       }


//=================================
#endif     // end of ABR_CODE
//=================================

       tbAtm155FreeTransmitBuffers(pAdapter, pVc);
       NdisFreeSpinLock(&pXmitSegInfo->lock);

       FREE_MEMORY(pXmitSegInfo, sizeof(XMIT_SEG_INFO));
       pVc->XmitSegInfo = NULL;
   }

   //
   //  Just clear the flag.
   //
   VC_CLEAR_FLAG(pVc, fVC_TRANSMIT);

}


NDIS_STATUS
tbAtm155AllocateTransmitSegment(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PVC_BLOCK               pVc,
   IN  PCO_MEDIA_PARAMETERS    pMediaParameters,
   IN  PATM_MEDIA_PARAMETERS   pAtmMediaParms
   )
/*++

Routine Description:

   This routine will allocate transmit resources for the VC.

Arguments:

   pAdapter        -   Pointer to the ADAPTER_BLOCK.
	pVc             -   Pointer to the VC_BLOCK that we are allocating transmit
                       resources for.
   pMediaParameters

Return Value:

--*/
{
   NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;
   PHARDWARE_INFO              pHwInfo = pAdapter->HardwareInfo;
   PATM_FLOW_PARAMETERS        pXmitFlow = &pAtmMediaParms->Transmit;
   ULONG                       PreScaleVal = 0;
   ULONG                       NumOfEntries = 0;
   PXMIT_SEG_INFO              pXmitSegInfo = NULL;
   PTBATM155_TX_STATE_ENTRY    pXmitState;
   ULONG                       RequestedSize;

   do
   {
       //
       //	Verify PDU.
       //
       if ((0 == pXmitFlow->MaxSduSize) ||
           (MAX_AAL5_PDU_SIZE < pXmitFlow->MaxSduSize))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid Sdu size in pXmitFlow->MaxSduSize (0x%lx)\n",
                 pXmitFlow->MaxSduSize));

           return(NDIS_STATUS_INVALID_DATA);
       }

	    //
	    //	Allocate memory for the transmit segment information.
	    //
	    ALLOCATE_MEMORY(&Status, &pXmitSegInfo, sizeof(XMIT_SEG_INFO), '51DA');
	    if (NDIS_STATUS_SUCCESS != Status)
	    {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Unable to allocate storage for the transmit segment information\n"));

           break;
       }

       //
       //	Initialize the transmit segment information.
       //
       ZERO_MEMORY(pXmitSegInfo, sizeof(XMIT_SEG_INFO));
       NdisAllocateSpinLock(&pXmitSegInfo->lock);


       //
       //	Save the transmit segment information to the VC.
       //
       pVc->XmitSegInfo = pXmitSegInfo;

       //
       //	Get the requested maximum service data unit size.
       //
       RequestedSize = pXmitFlow->MaxSduSize;

       //
       //	If the VC is CBR then we need to adjust the traffic parameters.
       //
       if (ATM_SERVICE_CATEGORY_CBR == pXmitFlow->ServiceCategory)
       {
           //
           //	Determine bandwidth adjustments if any....
           //
           Status = tbAtm155AdjustTrafficParameters(
                       pAdapter,
                       pXmitFlow,
                       (BOOLEAN)((pMediaParameters->Flags & ROUND_UP_FLOW) == ROUND_UP_FLOW),
                       &PreScaleVal,
                       &NumOfEntries);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Unable to adjust the traffic paramters for VC: 0x%x\n", pVc->VpiVci.Vci));

               Status = NDIS_STATUS_INVALID_DATA;
               break;
           }

           //
           //  Setup the necessary parameters for CBR Schedule table.
           //
           pVc->CbrNumOfEntris = NumOfEntries;
           pVc->CbrPreScaleVal = (UCHAR)PreScaleVal;
           pAdapter->TotalEntriesUsedonCBRs += NumOfEntries;

                      
#if    AW_QOS

           //
           //	The Number of available CBR VCs
           //  9/17/97     - Please read the note where declares this
           //                variable (in SW.H).
           //	
           pAdapter->NumOfAvailableCbrVc--;

#endif // end of AW_QOS

       }                                            

       //
       //	Pre build a Toshiba ATM155 PCI transmit slot descriptor.
       //
       pXmitSegInfo->InitXmitSlotDesc.reg = 0;
       pXmitSegInfo->InitXmitSlotDesc.VC = pVc->VpiVci.Vci;

     	//
       //	Save the flow parameters that we were registered with.
       //
       pVc->Transmit = *pXmitFlow;

       //
       //	Pre build a 155 PCI ATM transmit buffer information block.
       //
       pXmitSegInfo->Adapter = pAdapter;
       pXmitSegInfo->pEntryOfXmitState = 
                       pHwInfo->pSramTxVcStateTbl +
                       (pVc->VpiVci.Vci * SIZEOF_TX_STATE_ENTRY);

       pXmitState = &pXmitSegInfo->InitXmitState;

       NdisZeroMemory (
           (PVOID)(pXmitState),
           sizeof(TBATM155_TX_STATE_ENTRY));

	    switch (pXmitFlow->ServiceCategory) {

           case ATM_SERVICE_CATEGORY_CBR:
               pXmitState->TxStateWord0.RC = TX_STAT_RC_CBR;
               pXmitState->TxStateWord0.prescal_val = (USHORT)PreScaleVal;
		        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("ATM_SERVICE_CATEGORY_CBR\n"));
               break;

           case ATM_SERVICE_CATEGORY_UBR:
               pXmitState->TxStateWord0.RC = TX_STAT_RC_UBR;
		        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("ATM_SERVICE_CATEGORY_UBR\n"));
               break;

           case ATM_SERVICE_CATEGORY_ABR:

//=================================
#if    ABR_CODE
//=================================

               //
               // Set to ABR parameter base set 1 always for now.
               //
               pXmitState->TxStateWord0.ABS = 0;
               pXmitState->TxStateWord0.RC = TX_STAT_RC_ABR;

//=================================
#else     // not ABR_CODE
//=================================

               pXmitState->TxStateWord0.RC = TX_STAT_RC_UBR;

//=================================
#endif     // end of ABR_CODE
//=================================

		        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("ATM_SERVICE_CATEGORY_ABR\n"));
               break;

           default:
               Status = NDIS_STATUS_INVALID_DATA;
               break;
       }

       if (NDIS_STATUS_SUCCESS != Status) {
		    DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid Service Category for VC: 0x%x\n", pVc));
           break;
       }
           
       if (pAtmMediaParms->AALType == AAL_TYPE_AAL5)
       {
           pXmitState->TxStateWord0.chtype = TX_STAT_CHTYPE_AAL5;
       }
       else
       {
           pXmitState->TxStateWord0.chtype = TX_STAT_CHTYPE_RAW;
       }
       pXmitState->TxStateWord2.tx_AAL5_CRC_Low  = 0x0FFFF;
       pXmitState->TxStateWord3.tx_AAL5_CRC_High = 0x0FFFF;

       if (pAtmMediaParms->DefaultCLP == 1)
       {
           pXmitState->TxStateWord0.CLP = 1;
       }
       else
       {
           pXmitState->TxStateWord0.CLP = 0;
       }

	    //
	    //	Do we need to complete the packet when it is done being
	    //	segmented or can we do it early?
	    //  
	    if ((pMediaParameters->Flags & INDICATE_END_OF_TX) == INDICATE_END_OF_TX)
	    {
		    VC_SET_FLAG(pVc, fVC_INDICATE_END_OF_TX);
	    }


	    //
	    //	Initialize the queues for packets waiting for segmentation and
	    //	packets waiting for transmit.
	    //
       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, ("InitializePacketQueue\n"));
	    tbAtm155InitializePacketQueue(&pXmitSegInfo->SegWait);

       //
       //  Allocate the buffers for transmission.
       //
       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, ("AllocateTransmitBuffers\n"));
       Status = tbAtm155AllocateTransmitBuffers(pVc);

       //
       // NdisMAllocateSharedMemoryAsync is used in tbAtm155AllocateTransmitBuffers,
       // so we need to handle NDIS_STATUS_PENDING.
       //

       if (NDIS_STATUS_PENDING != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Failed of tbAtm155AllocateTransmitBuffers() for VC: 0x%x\n", pVc));
           break;
       }

   } while (FALSE);

   //
   //	If there was a failure then clean up.
   //

   if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
   {
       if (NULL != pVc->XmitSegInfo)
       {
           tbAtm155FreeTransmitBuffers(pAdapter, pVc);

           FREE_MEMORY(pVc->XmitSegInfo, sizeof(XMIT_SEG_INFO));
           pVc->XmitSegInfo = NULL;
       }
   }

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateTransmitSegment (Status: 0x%lx)\n", Status));

   return(Status);
}


NDIS_STATUS
tbAtm155ValidateVpiVci(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PATM_MEDIA_PARAMETERS   pAtmMediaParms
   )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   PVC_BLOCK       pCurrentVc;
   BOOLEAN         fInvalidVc = FALSE;
   PLIST_ENTRY     Link;
   //
   //	We only support VPI of 0!
   //
   if (pAtmMediaParms->ConnectionId.Vpi != 0)
   {
       return(NDIS_STATUS_FAILURE);
   }

   if ((pAtmMediaParms->ConnectionId.Vci > (ULONG)(pHwInfo->NumOfVCs - 1)))
   {
		DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
           ("<==tbAtm155ValidateVpiVci (NDIS_STATUS_FAILURE)\n"));

       return(NDIS_STATUS_FAILURE);
   }

   //
   //	See if we have a VC with the given VPI/VCI
   //
   Link = pAdapter->ActiveVcList.Flink;
   while (Link != &pAdapter->ActiveVcList)
   {
       pCurrentVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);
       if ((pCurrentVc->VpiVci.Vpi == pAtmMediaParms->ConnectionId.Vpi) &&
           (pCurrentVc->VpiVci.Vci == pAtmMediaParms->ConnectionId.Vci))
       {
           fInvalidVc = TRUE;
           break;
       }
       
       Link = Link->Flink;
       
   }

   //
   //	Did we find a VC with the given VPI/VCI.
   //
   if (fInvalidVc)
   {
       return(NDIS_STATUS_FAILURE);
   }

   return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
TbAtm155ActivateVc(
   IN      NDIS_HANDLE             MiniportVcContext,
   IN OUT  PCO_CALL_PARAMETERS     CoCallParameters
   )
/*++

Routine Description:

   This is the NDIS 5.0 handler to activate a given VC.  This will allocate
   hardware resources, e.g. QoS, for a VC that was already created.

Arguments:

   MiniportVcContext   -   Pointer to the VC_BLOCK representing the VC to
                           activate.
   MediaParameters     -   ATM parameters (in our case) that are used to
                           describe the VC.

Return Value:

   NDIS_STATUS_SUCCESS	    if the VC is successfully activated.

--*/
{
   PVC_BLOCK               pVc = (PVC_BLOCK)MiniportVcContext;
   PADAPTER_BLOCK          pAdapter = pVc->Adapter;
   PATM_MEDIA_PARAMETERS   pAtmMediaParms;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   PCO_MEDIA_PARAMETERS    pMediaParameters;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>TbAtm155ActivateVc\n"));

   NdisAcquireSpinLock(&pAdapter->lock);

   //
   //	If the adapter is resetting then refuse to do this.
   //
   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_REQUESTED) ||
       ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS))
   {
       NdisReleaseSpinLock(&pAdapter->lock);

       return(NDIS_STATUS_RESET_IN_PROGRESS);
   }

   NdisDprAcquireSpinLock(&pVc->lock);

   //
   //	If the VC is already active then we will need to
   //	re-activate the VC with new parameters.....
   //
   if (VC_TEST_FLAG(pVc, fVC_ACTIVE))
   {
       //
       //	Ok, to handle a re-activate we will first deactivate the
       //	vc then activate it with the new parameters.  This will
       //	most likely cause it to pend....
       //
       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
           ("tbAtm155ActivateVc currently doesn't support re-activation of VCs.\n"));

       DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);

       NdisDprReleaseSpinLock(&pVc->lock);
       NdisReleaseSpinLock(&pAdapter->lock);


       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
           ("<==TbAtm155ActivateVc (NDIS_STATUS_FAILURE)\n"));

       return(NDIS_STATUS_FAILURE);
   }

   do
   {
       //
       //	Get a convenient pointer to the media parameters.
       //	
       pMediaParameters = CoCallParameters->MediaParameters;

       //
       //	Are there any media specific parameters that we recognize?
       //
       if ((pMediaParameters->MediaSpecific.ParamType != ATM_MEDIA_SPECIFIC) ||
       	(pMediaParameters->MediaSpecific.Length != sizeof(ATM_MEDIA_PARAMETERS)))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid media parameters for vc creation\n"));

           Status = NDIS_STATUS_INVALID_DATA;

           break;
       }

       //
       //	Get a convenient pointer to the ATM parameters.
       //
       pAtmMediaParms = (PATM_MEDIA_PARAMETERS)pMediaParameters->MediaSpecific.Parameters;

       //
       //	We only handle CBR & UBR VCs.
       //
       if (((pAtmMediaParms->Transmit.ServiceCategory != ATM_SERVICE_CATEGORY_UBR) &&
           (pAtmMediaParms->Transmit.ServiceCategory != ATM_SERVICE_CATEGORY_CBR)) ||
           ((pAtmMediaParms->Receive.ServiceCategory != ATM_SERVICE_CATEGORY_UBR) &&
           (pAtmMediaParms->Receive.ServiceCategory != ATM_SERVICE_CATEGORY_CBR)))
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid service category specified for ActivateVC\n"));

           Status = NDIS_STATUS_INVALID_DATA;

           break;
       }

       //
       //	Validate the VPI/VCI
       //
       Status = tbAtm155ValidateVpiVci(pAdapter, pAtmMediaParms);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Unable to validate the VCI/VPI\n"));

           break;
       }

       //
       //	Save the VCI with our VC information.
       //
       pVc->VpiVci = pAtmMediaParms->ConnectionId;

       //
       //	Check the AAL type.
       //
       if (pAtmMediaParms->AALType != AAL_TYPE_AAL5)
       {
           Status = NDIS_STATUS_INVALID_DATA;

           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Invalid AAL type (only 5 supported)\n"));

           break;
       }

       //
       //	Save the AAL information with our VC.
       //
       pVc->AALType = pAtmMediaParms->AALType;

       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
            ("Vc=0x%lx, pMediaParameters->Flags=0x%lx\n",
              pVc->VpiVci.Vci, pMediaParameters->Flags));


#if DBG
              
       //
       //  This debugging code for detecting if there are race 
       //  coditions while activating a VC.
       //
       if ((pMediaParameters->Flags & TRANSMIT_VC) == TRANSMIT_VC)
       {
		    VC_SET_FLAG(pVc, fVC_DBG_TXVC);
       }
       if ((pMediaParameters->Flags & RECEIVE_VC) == RECEIVE_VC)
       {
		    VC_SET_FLAG(pVc, fVC_DBG_RXVC);
       }

#endif // end of DBG


       //
       //	Verify that we can support the given VC parameters.
       //
       if ((pMediaParameters->Flags & TRANSMIT_VC) == TRANSMIT_VC)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Allocating transmit information for VC\n"));

           //
           //	Allocate transmit resources.
           //
           Status = tbAtm155AllocateTransmitSegment(
                       pAdapter,
                       pVc,
                       pMediaParameters,
                       pAtmMediaParms);

           //
           // Process status NDIS_STATUS_PENDING because
           // NdisMAllocateSharedMemoryAsync is used in 
           // tbAtm155AllocateTransmitBuffers.
           //

           if (NDIS_STATUS_PENDING == Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("tbAtm155ActivateVc pended!\n"));
               //
               //	Save the CO_CALL_PARAMETERS with the VC for later
               //	completion.
               //
               pVc->CoCallParameters = CoCallParameters;

               VC_SET_FLAG(pVc, fVC_ALLOCATING_TXBUF);
               if ((pMediaParameters->Flags & RECEIVE_VC) != RECEIVE_VC)
               {
                   break;
               }
           }
           else if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Failed to allocate transmit segment for the VC\n"));

               break;
           }
       
       }

       if ((pMediaParameters->Flags & RECEIVE_VC) == RECEIVE_VC)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
               ("Allocating receive information for VC\n"));

           //
           //	Allocate receive resources.
           //
           Status = tbAtm155AllocateReceiveSegment(
                           pAdapter,
                           pVc,
                           pAtmMediaParms);

           if (NDIS_STATUS_PENDING == Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
                   ("tbAtm155ActivateVc pended!\n"));
               //
               //	Save the CO_CALL_PARAMETERS with the VC for later
               //	completion.
               //
               pVc->CoCallParameters = CoCallParameters;

               VC_SET_FLAG(pVc, fVC_ALLOCATING_RXBUF);

               break;
           }
           else if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Failed to allocate receive segment for the VC\n"));

               break;
           }
  
           VC_SET_FLAG(pVc, fVC_RECEIVE);
           
       }

       //
       // If tbAtm155AllocateTransmitSegment is successfully called, the following
       // procedure will be done in TbAtmAllocateComplete.
       //

       if ((pMediaParameters->Flags & TRANSMIT_VC) != TRANSMIT_VC)
       {
          //
          //	Remove the VC from the in-active queue and place on the active.
          //
          RemoveEntryList(&pVc->Link);
          InsertHeadList(&pAdapter->ActiveVcList, &pVc->Link);

          //
          //	Complete the VC.
          //
          Status = tbAtm155OpenVcChannels(pAdapter, pVc);
 
          if (NDIS_STATUS_SUCCESS != Status)
          {
              DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                      ("tbAtm155OpenVcChannels failed!\n"));

              RemoveEntryList(&pVc->Link);
              InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);
              break;
          }

          if (NULL != pVc->RecvSegInfo)
          {
              NdisDprAcquireSpinLock(&pVc->RecvSegInfo->lock);

              if (pVc->RecvBufType == RECV_BIG_BUFFER)
              {
                  if (pAdapter->NumOfAvailVc_B)
                  {
                      pAdapter->NumOfAvailVc_B--;
                  }
                  else
                  {
                      DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                          ("NumOfAvailVc_B(ig) == 0 failed!\n"));

                      Status = NDIS_STATUS_RESOURCES;
                  }
              }
              else
              {
                  if (pAdapter->NumOfAvailVc_S)
                  {
                      pAdapter->NumOfAvailVc_S--;
                  }
                  else
                  {
                      DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                          ("NumOfAvailVc_S(mall) == 0 failed!\n"));

                      Status = NDIS_STATUS_RESOURCES;
                  }
              }

              NdisDprReleaseSpinLock(&pVc->RecvSegInfo->lock);
          }

          if (NDIS_STATUS_SUCCESS != Status)
          {
              RemoveEntryList(&pVc->Link);
              InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);
              break;
          }

          //
          //	Mark the VC as currently active.
          //
          VC_SET_FLAG(pVc, fVC_ACTIVE);

          //
          //    Insert this VC into the hash list.
          //    If we insert this VC into the list too earlier, we might
          //    encounter a deadlock in tbatm155RxInterruptOnCompletion().
          //
          tbAtm155HashVc(pAdapter, pVc);
       }

   } while (FALSE);

   //
   //	Clean up if necessary.
   //

   //
   // If tbAtm155AllocateTransmitSegment is successfully called, and 
   // tbAtm155AllocateReceiveSegment returns NDIS_STATUS_SUCCESS, we will
   // return NDIS_STATUS_PENDING so TbAtmAllocateComplete can be called.
   //

   if (VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
   {
       if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
       {
          VC_SET_FLAG(pVc, fVC_ERROR);
       }

       Status = NDIS_STATUS_PENDING;
   }
   else 
   {
       if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
       {
           ASSERT(pVc->References == 1);

           //
           //	Remove the VC from the hash list.
           //
           tbAtm155RemoveHash(pAdapter, pVc);
    
           //
           //	Do we need to free the transmit segment?
           //
           if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
           {
               tbAtm155FreeTransmitSegment(pAdapter, pVc);
           }
    
           if (VC_TEST_FLAG(pVc, fVC_RECEIVE))
           {
               //
               //	Clean up allocated receive resources.
               //
               tbAtm155FreeReceiveSegment(pVc);
               VC_CLEAR_FLAG(pVc, fVC_RECEIVE);
           }

           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("TbAtm155ActivateVC failed (Status=%lx)!\n", Status));

       }
   }


   NdisDprReleaseSpinLock(&pVc->lock);
   NdisReleaseSpinLock(&pAdapter->lock);


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==TbAtm155ActivateVc (Status: 0x%lx)\n", Status));

   return(Status);
}



NDIS_STATUS
tbAtm155OpenVcChannels(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
	)
/*++

Routine Description:

   This routine will open the entries of Rx and/or Tx state tables
   on on-board SRAM.

Arguments:

   pAdapter    -   Pointer to the adapter to program.
   pVc         -   Pointer to the VC_BLOCK that describes the VC we
                   are handling.

Return Value:

   NDIS_STATUS_SUCCESS     if the entry of VC state table is opened
                           successfully.

--*/
{
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
	PXMIT_SEG_INFO          pXmitSegInfo = pVc->XmitSegInfo;
	PRECV_SEG_INFO          pRecvSegInfo = pVc->RecvSegInfo;
   PATM_FLOW_PARAMETERS    pXmitFlow = &pVc->Transmit;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO, 
               ("==>tbAtm155OpenVcChannels (Vc: %lx)\n", pVc->VpiVci.Vci));

   do {

#if DBG

       //
       //  This debugging code for detecting if there are race 
       //  coditions while activating a VC.
       //
       if (VC_TEST_FLAG(pVc, fVC_DBG_TXVC))
       {
           if (!VC_TEST_FLAG(pVc, fVC_TRANSMIT))
           {
               DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);
           }
           VC_CLEAR_FLAG(pVc, fVC_DBG_TXVC);
       }
       if (VC_TEST_FLAG(pVc, fVC_DBG_RXVC))
       {
           if (!VC_TEST_FLAG(pVc, fVC_RECEIVE))
           {
               DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);
           }
           VC_CLEAR_FLAG(pVc, fVC_DBG_RXVC);
       }

#endif // end of DBG


       if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
       {
           //
           // 1. Setup the necessary tables on SRAM depending
           //    upon the VC's flow.
           // 2. If OK, open the Vc connection in State table.
           //
	        switch (pXmitFlow->ServiceCategory) {

               case ATM_SERVICE_CATEGORY_CBR:
           
                   Status = tbAtm155SetCbrTblsInSRAM(pAdapter, pVc, TRUE);

                   if (NDIS_STATUS_SUCCESS == Status)
                   {
                       //
                       //  Adjust Trasnmit remaining bandwidth.
                       //
                       pAdapter->RemainingTransmitBandwidth -= pXmitFlow->PeakCellRate;
                   }

                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO, 
                           ("ATM_SERVICE_CATEGORY_CBR\n"));
                   break;

               case ATM_SERVICE_CATEGORY_ABR:
                   Status = tbAtm155SetAbrTblsInSRAM(pAdapter, pVc);
                   if (NDIS_STATUS_SUCCESS == Status)
                   {
                       pAdapter->RemainingTransmitBandwidth -= pXmitFlow->MinimumCellRate;
                   }

                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO, 
                           ("ATM_SERVICE_CATEGORY_ABR\n"));
                   break;

               case ATM_SERVICE_CATEGORY_UBR:
                   Status = tbAtm155SetUbrTblsInSRAM(pAdapter, pVc);
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO, 
                           ("ATM_SERVICE_CATEGORY_UBR\n"));
                   break;

               default:
                   Status = NDIS_STATUS_FAILURE;
                   break;

           }
   
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Failed to set tables in SRAM.\n"));
               DBGBREAK(DBG_COMP_VC, DBG_LEVEL_ERR);

               break;
           }
           //
           //  Set the entry of Tx Vc State table.
           //
           Status = tbAtm155WriteEntryOfTheSramTbl(
                       pAdapter,
                       pXmitSegInfo->pEntryOfXmitState,
                       (PUSHORT)&pXmitSegInfo->InitXmitState,
                       8);

	        if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Failed to open the VC in the Tx VC State table in SRAM.\n"));

               break;
           }

       }          

       if (VC_TEST_FLAG(pVc, fVC_RECEIVE))
       {
           //
           //  Set the entry of Rx Vc State table.
           //
           Status = tbAtm155WriteEntryOfTheSramTbl(
                       pAdapter,
                       pRecvSegInfo->pEntryOfRecvState,
                       (PUSHORT)&pRecvSegInfo->InitRecvState,
                       8);

	        if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
                   ("Failed to open the VC in the Rx VC State table in SRAM.\n"));

               break;
           }

           //
           //	if the adapter is currently resetting or shutting down,
           //  don't post any receive buffer to receive slots.
           //
           if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS) &&
               !ADAPTER_TEST_FLAG(pAdapter, fADAPTER_SHUTTING_DOWN))
           {
               //
               //  Post buffer onto the controller then queue the buffers to
               //  DmaCompleting list if there are more Rx slots available.
               //
               tbAtm155QueueRecvBuffersToReceiveSlots(
                           pAdapter,
                           pVc->RecvBufType);
           }

       }

   } while (FALSE);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155OpenVcChannels\n"));

   return (Status);

}



VOID
TbAtm155ActivateVcComplete(
   IN  PVC_BLOCK       pVc,
   IN  NDIS_STATUS     Status
   )
/*++

Routine Description:

   This routine will complete the VC activation.

Arguments:

   pVc -   Pointer to the VC_BLOCK that describes the VC we are activating.

Return Value:

   None.

--*/
{
   PADAPTER_BLOCK          pAdapter = pVc->Adapter;
   PRECV_SEG_INFO	        pRecvSegInfo = pVc->RecvSegInfo;


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>tbAtm155ActivateVcComplete (Status: 0x%x)\n", Status));

   NdisAcquireSpinLock(&pAdapter->lock);
   NdisDprAcquireSpinLock(&pVc->lock);

   do
   {

       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }

       //
       //	Remove the VC from the in-active queue and place on the active.
       //
       RemoveEntryList(&pVc->Link);
       InsertHeadList(&pAdapter->ActiveVcList, &pVc->Link);
	
       Status = tbAtm155OpenVcChannels(pAdapter, pVc);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           RemoveEntryList(&pVc->Link);
           InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);
           break;
       }

       if (NULL != pRecvSegInfo)
       {
           NdisDprAcquireSpinLock(&pRecvSegInfo->lock);

           if (pVc->RecvBufType == RECV_BIG_BUFFER)
           {
               if (pAdapter->NumOfAvailVc_B)
               {
                   pAdapter->NumOfAvailVc_B--;
               }
               else
               {
                   Status = NDIS_STATUS_RESOURCES;
               }
           }
           else
           {
               if (pAdapter->NumOfAvailVc_S)
               {
                   pAdapter->NumOfAvailVc_S--;
               }
               else
               {
                   Status = NDIS_STATUS_RESOURCES;
               }
           }

           NdisDprReleaseSpinLock(&pRecvSegInfo->lock);
       }

       if (NDIS_STATUS_SUCCESS != Status)
       {
           RemoveEntryList(&pVc->Link);
           InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);
           break;
       }

       //
       //	Mark the VC as currently active.
       //
       //  If memory allocation are done, set fVC_ACTIVE to
       //  indicate the VC has been activated.
       //
       if (!VC_TEST_FLAG(pVc, fVC_ALLOCATING_RXBUF) &&
           !VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
       {           
           VC_SET_FLAG(pVc, fVC_ACTIVE);

           //
           //    Insert this VC into the hash list.
           //    If we insert this VC into the list too earlier, we might
           //    encounter a deadlock in tbatm155RxInterruptOnCompletion().
           //
           tbAtm155HashVc(pAdapter, pVc);

       }           

   } while (FALSE); 

   if (NDIS_STATUS_SUCCESS != Status)
   {
       tbAtm155RemoveHash(pAdapter, pVc);

       //
       //	Clean up allocated transmit resources.
       //
       if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
       {
           tbAtm155FreeTransmitSegment(pAdapter, pVc);

           VC_CLEAR_FLAG(pVc, fVC_TRANSMIT);           
       }

       //
       //	Clean up allocated receive resources.
       //
       if (VC_TEST_FLAG(pVc, fVC_RECEIVE))
       {
           tbAtm155FreeReceiveSegment(pVc);
           VC_CLEAR_FLAG(pVc, fVC_RECEIVE);
       }
   }

   NdisDprReleaseSpinLock(&pVc->lock);
   NdisReleaseSpinLock(&pAdapter->lock);

   //
   //	Complete the VC activation.
   //

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155ActivateVcComplete  (Status: 0x%x), (Vc: 0x%lx)\n",
       Status, pVc->VpiVci.Vci));

   NdisMCoActivateVcComplete(
       Status,
       pVc->NdisVcHandle,
       pVc->CoCallParameters);

   return;
}


NDIS_STATUS
TbAtm155DeactivateVc(
   IN  NDIS_HANDLE     MiniportVcContext
	)
/*++

Routine Description:

   This is the NDIS 4.1 handler to deactivate a given VC.
   This does not free any resources, but simply marks the VC as unusable.

Arguments:

   MiniportVcContext   -   Pointer to our VC_BLOCK that was allocated in
                           TbAtm155CreateVc().

Return Value:

   NDIS_STATUS_SUCCESS     if we successfully deactivate the VC.

--*/
{
   PVC_BLOCK           pVc = (PVC_BLOCK)MiniportVcContext;
   PADAPTER_BLOCK      pAdapter = pVc->Adapter;
   NDIS_STATUS         Status = NDIS_STATUS_PENDING;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>TbAtm155DeactivateVc, VC=(0x%lx)\n", pVc->VpiVci.Vci));

   NdisAcquireSpinLock(&pAdapter->lock);

   if (ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS))
   {
       //
       //	We are resetting. Return NDIS_STATUS_PENDING to
       //  get more time from atmuni for resttting.
       //
       NdisDprAcquireSpinLock(&pVc->lock);

       if (!VC_TEST_FLAG(pVc, fVC_DEACTIVATING))
       {
           VC_CLEAR_FLAG(pVc, fVC_ACTIVE);
           VC_SET_FLAG(pVc, fVC_DEACTIVATING);
       }

       DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
               ("==>TbAtm155DeactivateVc, VC=(0x%lx), flag=((0x%lx))\n", 
               pVc->VpiVci.Vci, pVc->Flags));

       NdisDprReleaseSpinLock(&pVc->lock);
       NdisReleaseSpinLock(&pAdapter->lock);
       return(Status);
   }

   NdisDprAcquireSpinLock(&pVc->lock);

   do
   {

       //
       //	If the VC is already deactivating then we are done.
       //
       if (VC_TEST_FLAG(pVc, fVC_DEACTIVATING))
       {
           Status = NDIS_STATUS_CLOSING;
           break;
       }

       //
       //	Mark the VC.
       //
       VC_CLEAR_FLAG(pVc, fVC_ACTIVE);
       VC_SET_FLAG(pVc, fVC_DEACTIVATING);

       //
       //	Can't deactivate a VC with outstanding references....
       //
       if (pVc->References > 1)
       {
           DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
               ("Bad value of pVc->References (%u), Vc=(%lx)\n",
                 pVc->References, pVc->VpiVci.Vci));

           break;
       }

       //
       //	Release the locks before calling deactivate complete routine.
       //
       NdisDprReleaseSpinLock(&pVc->lock);
       NdisReleaseSpinLock(&pAdapter->lock);

       //
       //	Complete the deactivation of the VC.
       //
       TbAtm155DeactivateVcComplete(pAdapter, pVc);

       //
       //	We are done.  The above routine will free the adapter and
       //	Vc locks before calling the completion handler.
       //
       return(NDIS_STATUS_PENDING);

   } while (FALSE);

   NdisDprReleaseSpinLock(&pVc->lock);
   NdisReleaseSpinLock(&pAdapter->lock);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==TbAtm155DeactivateVc\n"));

   return(NDIS_STATUS_PENDING);
}


VOID
TbAtm155DeactivateVcComplete(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
   )
/*++

Routine Description:

   This routine is called to complete the deactivation of the VC.
   this does NOT call NdisMCoDeactivateVcComplete() it is simply a place
   to put common code...

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK owning the VC.
   pVc         -   Pointer to the VC that is being deactivated.

Return Value:

   None

--*/
{
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
   PRECV_SEG_INFO	pRecvSegInfo;


   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>tbAtm155DeactivateVcComplete VC=(0x%lx)\n", pVc->VpiVci.Vci));

   NdisAcquireSpinLock(&pAdapter->lock);
   NdisDprAcquireSpinLock(&pVc->lock);

   //
   //	If we are resetting then this VC deactivation will get taken care of
   //	later.
   //
   if (VC_TEST_FLAG(pVc, fVC_RESET_IN_PROGRESS))
   {
		DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
            ("tbAtm155DeactivateVcComplete: Adapter reset in progress (VC:0x%lx), (pVc: "PTR_FORMAT")\n",
            pVc->VpiVci.Vci, pVc));
       NdisDprReleaseSpinLock(&pVc->lock);
       NdisReleaseSpinLock(&pAdapter->lock);

       return;
   }

   ASSERT(1 == pVc->References);

   //
   //	Remove the VC from the active list and place it on the inactive list.
   //
   VC_CLEAR_FLAG(pVc, fVC_DEACTIVATING);
   RemoveEntryList(&pVc->Link);
   InsertHeadList(&pAdapter->InactiveVcList, &pVc->Link);

   //
   //	Remove the vc from the hash table.
   //
   tbAtm155RemoveHash(pAdapter, pVc);

   pRecvSegInfo = pVc->RecvSegInfo;
   if (NULL != pRecvSegInfo)
   {
       NdisDprAcquireSpinLock(&pRecvSegInfo->lock);

       if (pVc->RecvBufType == RECV_BIG_BUFFER)
       {
           pAdapter->NumOfAvailVc_B++;
       }
       else
       {
           pAdapter->NumOfAvailVc_S++;
       }

       NdisDprReleaseSpinLock(&pRecvSegInfo->lock);
   }

   //
   //	Free up an transmit resources...
   //
   if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
   {
       tbAtm155FreeTransmitSegment(pAdapter, pVc);
   }

   //
   //	Free up any receive resources...
   //
   if (VC_TEST_FLAG(pVc, fVC_RECEIVE))
   {
       tbAtm155FreeReceiveSegment(pVc);
       VC_CLEAR_FLAG(pVc, fVC_RECEIVE);       
   }
   

   NdisDprReleaseSpinLock(&pVc->lock);
   NdisReleaseSpinLock(&pAdapter->lock);

   NdisMCoDeactivateVcComplete(Status, pVc->NdisVcHandle);

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155DeactivateVcComplete (Status: 0x%x)\n", Status));
}

