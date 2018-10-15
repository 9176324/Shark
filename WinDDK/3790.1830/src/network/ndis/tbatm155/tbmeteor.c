/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       tbmeteor.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains routines of Toshiba ATM155 Meteor
   specific handling,
   1. Auto-detecting of the Meteor 1K (or 4K) suppoorting.
   2. Setup the database and then initialize accordingly.


Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	05/19/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER      MODULE_TBMETEOR



NDIS_STATUS
tbAtm155InitSRAM_1KVCs(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will initialize SRAM which supports up to 1K VCs.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK that describes the NIC.

Return Value:

   NDIS_STATUS_SUCCESS     -   initialize SRAM successful
   NDIS_STATUS_FAILURE     -   failed to initialize SRAM

--*/
{
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   ULONG               pSRAM;
   USHORT              phData;
   PSRAM_1K_VC_MODE    pSramAddrTbl;


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitSRAM_1KVCs\n"));

   pSramAddrTbl = (PSRAM_1K_VC_MODE)pHwInfo->pSramAddrTbl;

   do
   {
       //
       // Zero SRAM first before setting up tables in SRAM.
       //
       for (pSRAM = pSramAddrTbl->pRx_AAL5_Big_Free_Slot;
            pSRAM < pSramAddrTbl->pEnd_Of_SRAM;
            pSRAM++)
       {
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, 0, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Clear SRAM failed! \n"));

               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       

       //
       //  Initialize the Tx slot descriptors structure, containing 1K 4-word 
       //  entries for 1K VC, from 0xA000 to 0xAFFF in on-board SRAM.
       //  
       //  Write phData + 1, where phData is the VC of the entry.
       //
       for (pSRAM = pSramAddrTbl->pTx_Slot_Descriptors, phData = 1;
            pSRAM < pSramAddrTbl->pACR_LookUp_Tbl;
            pSRAM += SIZEOF_TX_SLOT_DESC, phData++)
       {
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, phData, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize Tx Slot Descriptor in SRAM failed! \n"));
               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       
       
       //
       //  Initialize the CBR schedule table #1 and CBR schedule table #2
       //  structure from 0xE000 to 0xFFFF in on-board SRAM.
       //  Each table contains 4K entries, and each entry is 1 word.
       //
       for (pSRAM = pSramAddrTbl->pCBR_Schedule_Tbl_1, 
            phData = CBR_SCHEDULE_ENTRY_EOT;
            pSRAM < pSramAddrTbl->pEnd_Of_SRAM;
            pSRAM += SIZEOF_CBR_SCHEDULE_ENTRY)
       {
           //
           // Initialize CBR schedule tables.
           //
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, phData, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize CBR Schedule tables in SRAM failed! \n"));
               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       
       
       
       //
       //  Initialize the ACR lookup table structure, containing 512 1-word 
       //  entries, from 0xB000 to 0xB1FF in on-board SRAM.
       //
       for (pSRAM = pSramAddrTbl->pACR_LookUp_Tbl, phData = 0;
            pSRAM < pSramAddrTbl->pReserved2;
            pSRAM += SIZEOF_ACR_LOOKUP_TBL, phData++)
       {
           TBATM155_PH_WRITE_SRAM(
                   pAdapter,
                   pSRAM, 
                   AcrLookUpTbl[phData], 
                   &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize CBR Schedule tables in SRAM failed! \n"));
               break;
           }
       }

   } while (FALSE);


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitSRAM_1KVCs\n"));

   return(Status);

}



NDIS_STATUS
tbAtm155InitSRAM_4KVCs(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will initialize SRAM which supports up to 4K VCs.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK that describes the NIC.

Return Value:

   NDIS_STATUS_SUCCESS     -   initialize SRAM successful
   NDIS_STATUS_FAILURE     -   failed to initialize SRAM

--*/
{
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   ULONG               pSRAM;
   USHORT              phData;
   PSRAM_4K_VC_MODE     pSramAddrTbl;


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitSRAM_4KVCs\n"));

   pSramAddrTbl = (PSRAM_4K_VC_MODE)pHwInfo->pSramAddrTbl;

   do
   {
       //
       // Zero SRAM first before setting up tables in SRAM.
       //
       for (pSRAM = pSramAddrTbl->pRx_AAL5_Big_Free_Slot;
            pSRAM < pSramAddrTbl->pEnd_Of_SRAM;
            pSRAM++)
       {
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, 0, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Clear SRAM failed! \n"));

               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       

       //
       //  Initialize the Tx slot descriptors structure, containing 1K 4-word 
       //  entries for 1K VC, from 0x28000 to 0x2C000 in on-board SRAM.
       //  
       //  Write phData + 1, where phData is the VC of the entry.
       //
       for (pSRAM = pSramAddrTbl->pTx_Slot_Descriptors, phData = 1;
            pSRAM < pSramAddrTbl->pABR_Schedule_Tbl;
            pSRAM += SIZEOF_TX_SLOT_DESC, phData++)
       {
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, phData, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize Tx Slot Descriptor in SRAM failed! \n"));
               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       
       
       //
       //  Initialize the CBR schedule table #1 and CBR schedule table #2
       //  structure from 0x2E000 to 0x30000 in on-board SRAM.
       //  Each table contains 4K entries, and each entry is 1 word.
       //
       for (pSRAM = pSramAddrTbl->pCBR_Schedule_Tbl_1, 
            phData = CBR_SCHEDULE_ENTRY_EOT;
            pSRAM < pSramAddrTbl->pRx_AAL5_B_Slot_Tags;
            pSRAM += SIZEOF_CBR_SCHEDULE_ENTRY)
       {
           //
           // Initialize CBR schedule tables.
           //
           TBATM155_PH_WRITE_SRAM(pAdapter, pSRAM, phData, &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize CBR Schedule tables in SRAM failed! \n"));
               break;
           }
       }
       
       if (NDIS_STATUS_SUCCESS != Status)
       {
           break;
       }       
       
       
       //
       //  Initialize the ACR lookup table structure, containing 512 1-word 
       //  entries, from 0x6000 to 0x6200 in on-board SRAM.
       //
       for (pSRAM = pSramAddrTbl->pACR_LookUp_Tbl, phData = 0;
            pSRAM < pSramAddrTbl->pReserved;
            pSRAM += SIZEOF_ACR_LOOKUP_TBL, phData++)
       {
           TBATM155_PH_WRITE_SRAM(
                   pAdapter,
                   pSRAM, 
                   AcrLookUpTbl[phData], 
                   &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Initialize CBR Schedule tables in SRAM failed! \n"));
               break;
           }
       }

   } while (FALSE);


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitSRAM_4KVCs\n"));

   return(Status);

}



NDIS_STATUS
tbAtm155IsIt4KVc(
	IN PADAPTER_BLOCK	pAdapter
   )

/*++

Routine Description:

   1. write 3 different sets of data onto the address 0 of SRAM.
   2. change address to be (address 0 + 1K VC SRAM size), and 
      read the data back. If ATM155 supports only up to 1K VC,
      the data will be identical, because address lines
      are only 15 bits which will make address wraparound to read
      the data from where we wrote to.
   3. Allocate and setup table for SRAM address layout.

Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   NDIS_STATUS_FAILURE     detect something are unable to determine.
                           on the adapter.
   NDIS_STATUS_SUCCESS     otherwise.

--*/

{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   USHORT              TestData[TEST_PATTERN_WORD] = { 0x55AA, 0x0AA55, 0x1234};
   ULONG               i, j;
   USHORT              data, NoOfMatch = 0;
   PSRAM_4K_VC_MODE    pSramAddrTbl_4KVC;
   PSRAM_1K_VC_MODE    pSramAddrTbl_1KVC;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   TB155PCISAR_CNTRL1  regControl1;


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("==>tbAtm155IsIt4KVc\n"));

   tbAtm155SoftresetNIC(pHwInfo);

   //
   // Set to support 4K VC mode.
   //
   regControl1.reg = 0;
   regControl1.VC_Mode_Sel = 3;            // Set 4K VCs regardless.

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
       regControl1.reg);

   // 
   // Start the test patterns now.
   // 
   for (j= 0; j < TEST_PATTERN_WORD; j++)
   {
       for (i= 0; i < TEST_WORD; i++)
       {
           //
           // Write test data into the beginning of SRAM
           //
           TBATM155_PH_WRITE_SRAM(pAdapter, i, TestData[j], &Status);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Write SRAM failed! \n"));
               break;
           }

           //
           // Read data from the offset of of 1K VC SRAM size.
           // If the meteor supports up to 1K VCs, the test data
           // test data will be read back due, to address wrapparound.
           //
           TBATM155_PH_READ_SRAM(
               pAdapter,
               (i +  SRAM_SIZE_1KVC),
               &data,
               &Status);

           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
                   ("Read SRAM failed! \n"));
               break;
           }

           if (data == TestData[j])
           {
               //
               // Data are idential. Let's update the counter.
               //
               NoOfMatch++;
           }
       }
   }

   //
   // If all of the data are identical to the test patterns, the adapter
   // is designed for 1K VC mode.
   //
   pHwInfo->fAdapterHw &= (UCHAR)(~(TBATM155_METEOR_4KVC | TBATM155_METEOR_1KVC));
   if (NDIS_STATUS_SUCCESS == Status)
   {
       if (NoOfMatch != (TEST_WORD * TEST_PATTERN_WORD))
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Toshiba Meteor supporting 4K VCs\n"));
           do
           {
               pHwInfo->fAdapterHw |= TBATM155_METEOR_4KVC;
               pHwInfo->NumOfVCs = MAX_VCS_4K;
               pHwInfo->MaxIdxOfRxReportQ = (MAX_VCS_4K*MAX_NUM_RX_SLOT_QUEUES)-1;
               pHwInfo->MaxIdxOfTxReportQ = (MAX_VCS_4K*MAX_NUM_TX_SLOT_QUEUES)-1;

               ALLOCATE_MEMORY(
                       &Status, 
                       (PVOID)&pHwInfo->pSramAddrTbl,
                       sizeof(SRAM_4K_VC_MODE),
                       '71DA');

               if (NDIS_STATUS_SUCCESS != Status)
               {
                   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                       ("Failed to allocate SRAM Address table of 4K VC).\n"));
                   break;
               }

               ZERO_MEMORY(pHwInfo->pSramAddrTbl, sizeof(SRAM_4K_VC_MODE));
           
               //
               // Setup addresses for the size of SRAM supported 4K VCs
               //
               pSramAddrTbl_4KVC = (PSRAM_4K_VC_MODE)pHwInfo->pSramAddrTbl;

               pSramAddrTbl_4KVC->pRx_AAL5_Big_Free_Slot    = 0;      
               pSramAddrTbl_4KVC->pRx_AAL5_Small_Free_Slot  = 0x2000; 
               pSramAddrTbl_4KVC->pRx_Raw_Free_Slot         = 0x4000; 
               pSramAddrTbl_4KVC->pACR_LookUp_Tbl           = 0x6000; 
               pSramAddrTbl_4KVC->pReserved                 = 0x6200; 
               pSramAddrTbl_4KVC->pRx_VC_State_Tbl          = 0x8000; 
               pSramAddrTbl_4KVC->pTx_VC_State_Tbl          = 0x10000;
               pSramAddrTbl_4KVC->pABR_Parameter_Tbl        = 0x18000;
               pSramAddrTbl_4KVC->pABR_Value_Tbl            = 0x20000;
               pSramAddrTbl_4KVC->pRM_Cell_Data_Tbl         = 0x24000;
               pSramAddrTbl_4KVC->pTx_Slot_Descriptors      = 0x28000;
               pSramAddrTbl_4KVC->pABR_Schedule_Tbl         = 0x2C000;
               pSramAddrTbl_4KVC->pCBR_Schedule_Tbl_1       = 0x2E000;
               pSramAddrTbl_4KVC->pCBR_Schedule_Tbl_2       = 0x2F000;
               pSramAddrTbl_4KVC->pRx_AAL5_B_Slot_Tags      = 0x30000;
               pSramAddrTbl_4KVC->pRx_AAL5_S_Slot_Tags      = 0x31000;
               pSramAddrTbl_4KVC->pRx_Raw_Slot_Tags         = 0x32000;
               pSramAddrTbl_4KVC->pRx_Slot_Tag_VC_State_Tbl = 0x33000;
               pSramAddrTbl_4KVC->pEnd_Of_SRAM              = 0x34000;

               //
               // Setup the pointers of some of SRAM tables.
               //
               pHwInfo->pSramRxAAL5SmallFsFIFo = pSramAddrTbl_4KVC->pRx_AAL5_Small_Free_Slot;
               pHwInfo->pSramRxAAL5BigFsFIFo   = pSramAddrTbl_4KVC->pRx_AAL5_Big_Free_Slot;
               pHwInfo->pSramRxVcStateTbl      = pSramAddrTbl_4KVC->pRx_VC_State_Tbl;
               pHwInfo->pSramTxVcStateTbl      = pSramAddrTbl_4KVC->pTx_VC_State_Tbl;
               pHwInfo->pSramAbrValueTbl       = pSramAddrTbl_4KVC->pABR_Value_Tbl;
               pHwInfo->pABR_Parameter_Tbl     = pSramAddrTbl_4KVC->pABR_Parameter_Tbl;
               pHwInfo->pSramCbrScheduleTbl_1  = 
                                   pSramAddrTbl_4KVC->pCBR_Schedule_Tbl_1;
               pHwInfo->pSramCbrScheduleTbl_2  = 
                                   pSramAddrTbl_4KVC->pCBR_Schedule_Tbl_2;

           } while (FALSE);

       }
       else
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Toshiba Meteor supporting 1K VCs\n"));
           do
           {
               pHwInfo->fAdapterHw |= TBATM155_METEOR_1KVC;
               pHwInfo->NumOfVCs = MAX_VCS_1K;
               pHwInfo->MaxIdxOfRxReportQ = (MAX_VCS_1K*MAX_NUM_RX_SLOT_QUEUES)-1;
               pHwInfo->MaxIdxOfTxReportQ = (MAX_VCS_1K*MAX_NUM_TX_SLOT_QUEUES)-1;

               ALLOCATE_MEMORY(
                       &Status, 
                       (PVOID)&pHwInfo->pSramAddrTbl,
                       sizeof(SRAM_1K_VC_MODE),
                       '71DA');
               if (NDIS_STATUS_SUCCESS != Status)
               {
                   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                       ("Failed to allocate SRAM Address table of 1K VC).\n"));
                   break;
               }
           
               ZERO_MEMORY(pHwInfo->pSramAddrTbl, sizeof(SRAM_1K_VC_MODE));

               //
               // Setup addresses for the size of SRAM supported 1K VCs
               //
               pSramAddrTbl_1KVC = (PSRAM_1K_VC_MODE)pHwInfo->pSramAddrTbl;

               pSramAddrTbl_1KVC->pRx_AAL5_Big_Free_Slot   = 0;
               pSramAddrTbl_1KVC->pRx_AAL5_Small_Free_Slot = 0x800;
               pSramAddrTbl_1KVC->pRx_Raw_Free_Slot        = 0x1000;
               pSramAddrTbl_1KVC->pReserved1               = 0x1800;
               pSramAddrTbl_1KVC->pRx_VC_State_Tbl         = 0x2000;
               pSramAddrTbl_1KVC->pTx_VC_State_Tbl         = 0x4000;
               pSramAddrTbl_1KVC->pABR_Parameter_Tbl       = 0x6000;
               pSramAddrTbl_1KVC->pABR_Value_Tbl           = 0x8000;
               pSramAddrTbl_1KVC->pRM_Cell_Data_Tbl        = 0x9000;
               pSramAddrTbl_1KVC->pTx_Slot_Descriptors     = 0x0A000;
               pSramAddrTbl_1KVC->pACR_LookUp_Tbl          = 0x0B000;
               pSramAddrTbl_1KVC->pReserved2               = 0x0B200;
               pSramAddrTbl_1KVC->pRx_AAL5_B_Slot_Tags     = 0x0B400;
               pSramAddrTbl_1KVC->pRx_AAL5_S_Slot_Tags     = 0x0B800;
               pSramAddrTbl_1KVC->pRx_Raw_Slot_Tags        = 0x0BC00;
               pSramAddrTbl_1KVC->pABR_Schedule_Tbl        = 0x0C000;
               pSramAddrTbl_1KVC->pCBR_Schedule_Tbl_1      = 0x0E000;
               pSramAddrTbl_1KVC->pCBR_Schedule_Tbl_2      = 0x0F000;
               pSramAddrTbl_1KVC->pEnd_Of_SRAM             = 0x10000;

               //
               // Setup the pointers of some of SRAM tables.
               //
               pHwInfo->pSramRxAAL5SmallFsFIFo = pSramAddrTbl_1KVC->pRx_AAL5_Small_Free_Slot;
               pHwInfo->pSramRxAAL5BigFsFIFo   = pSramAddrTbl_1KVC->pRx_AAL5_Big_Free_Slot;
               pHwInfo->pSramRxVcStateTbl      = pSramAddrTbl_1KVC->pRx_VC_State_Tbl;
               pHwInfo->pSramTxVcStateTbl      = pSramAddrTbl_1KVC->pTx_VC_State_Tbl;
               pHwInfo->pSramAbrValueTbl       = pSramAddrTbl_1KVC->pABR_Value_Tbl;
               pHwInfo->pABR_Parameter_Tbl     = pSramAddrTbl_1KVC->pABR_Parameter_Tbl;
               pHwInfo->pSramCbrScheduleTbl_1  = 
                                   pSramAddrTbl_1KVC->pCBR_Schedule_Tbl_1;
               pHwInfo->pSramCbrScheduleTbl_2  = 
                                   pSramAddrTbl_1KVC->pCBR_Schedule_Tbl_2;

           } while (FALSE);

       }

   } // end of IF

   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("<==tbAtm155IsIt4KVc\n"));

   return (Status);
}


