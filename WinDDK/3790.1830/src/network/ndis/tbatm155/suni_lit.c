/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.


   File:       suni_lit.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains PMC S/UNI-Lite specific handling.


Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	05/09/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "basetsd.h"
#include "precomp.h"
#include "suni_lit.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_SUNI_LITE



NDIS_STATUS
tbAtm155InitSuniLitePhyRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will initialize the PHY registers.

Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

   NDIS_STATUS_FAILURE     if the adapter is not supported.
   NDIS_STATUS_SUCCESS     otherwise.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PSUNI_REGISTERS     pPhy = ULongToPtr(pHwInfo->Phy);


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitSuniLitPhyRegisters\n"));


   // 
   //  Set control values of MMF & Suni-Lite model through LED interface.
   // 
   pHwInfo->LedVal = fLED_SUNI_155MB_TXENB;

   //
   //	Set and clear the reset bit for the phy.
   //
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Suni.MasterResetIdentity,
       fSUNI_MRI_RESET,
       &Status);

   NdisStallExecution(100);        // let's give a 100-ms-delay.

   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Suni.MasterResetIdentity,
       0,
       &Status);

   NdisStallExecution(500);        // let's give a 100-ms-delay.

   //
   //	Clear the SUNI test mode.
   //
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Suni.MasterTest,
       0,
       &Status);

   //
   //	Enable SUNI RACP interrupts.
   //
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Suni.racpIntEnableStatus,
       (fSUNI_RACP_IES_FIFOE | fSUNI_RACP_IES_HCSE),
       &Status);

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitSuniLitPhyRegisters\n"));

   return(Status);
}


//
//
VOID
tbAtm155SuniInterrupt(
	IN	PADAPTER_BLOCK	pAdapter
	)
/*++

Routine Description:

   This routine will process the SUNI error interrupts.

Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   None.

--*/
{
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   USHORT	       Value;
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
   PSUNI_REGISTERS pPhy = ULongToPtr(pHwInfo->Phy);

   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("==>tbAtm155SuniInterrupt\n"));



   NdisDprAcquireSpinLock(&pAdapter->lock);

   //
   //  Read the SUNI Receive ATM Cell Processor interrupt.
   //  This denotes a cell error at the SUNI level.
   //
   TBATM155_PH_READ_DEV(
       pAdapter,
		&pPhy->Suni.racpIntEnableStatus,
		&Value,
       &Status);

   pAdapter->StatInfo.RecvCellsDropped++;

   NdisDprReleaseSpinLock(&pAdapter->lock);

   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("<==tbAtm155SuniInterrupt (racpIntEnableStatus: %x)\n", Value));
}


NDIS_STATUS
tbAtm155DetectPhyType(
	IN PADAPTER_BLOCK	pAdapter
   )
/*++

Routine Description:

   This routine detects the type of PHY controllers is used on
   the card.
   Note:
       When reach to this point the SAR registers of ATM155 Meteor
       have been set, DON'T call tbAtm155SoftresetNIC() which will
       reset all of the registers set earlier.

Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   NDIS_STATUS_FAILURE     detect something are unable to determine.
                           on the adapter.
   NDIS_STATUS_SUCCESS     otherwise.

--*/

{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   TB155PCISAR_CNTRL1      regControl1;
   USHORT                  phdata;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   PSUNI_REGISTERS         pPhy = ULongToPtr(pHwInfo->Phy);


   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("==>tbAtm155DetectPhyType()\n"));


   do
   {
       // 
       //  1. Initialize PHY chip through Meteor register.
       //  2. Clear the bit to exit from the reset state.
       // 
       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           &regControl1.reg);

       regControl1.Phy_Reset = 1;

       TBATM155_WRITE_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           regControl1.reg);

       NdisStallExecution(100);        // let's give a 10-ms-delay.

       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           &regControl1.reg);

       regControl1.Phy_Reset = 0;

       TBATM155_WRITE_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           regControl1.reg);

       TBATM155_READ_PORT(
           &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
           &regControl1.reg);

       if (regControl1.Phy_Reset)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Phy_reset bit is not reset.\n"));

           Status = NDIS_STATUS_FAILURE;
           break;
       }

       TBATM155_PH_READ_DEV(
           pAdapter,
           &pPhy->Suni.MasterResetIdentity,
           &phdata,
           &Status);

       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("err point 1.\n"));

           break;
       }

       //
       //  Clear these two flags to make sure they are not both set.
       //
       pHwInfo->fAdapterHw &= (UCHAR)(~(TBATM155_PHY_PLC_2 | TBATM155_PHY_SUNI_LITE));

       if (phdata == fSUNI_MRI_TYPE_ID)
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Detected S/UNI-Lite.\n"));
           pHwInfo->fAdapterHw |= TBATM155_PHY_SUNI_LITE;
       }
       else
       {
           DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
               ("Detected PLC_2.\n"));
           pHwInfo->fAdapterHw |= TBATM155_PHY_PLC_2;
       }

   } while (FALSE);

   DBGPRINT(DBG_COMP_INT, DBG_LEVEL_INFO,
       ("<==tbAtm155DetectPhyType()\n"));

   return (Status);

}


BOOLEAN
tbAtm155CheckSuniLiteLinkUp(
	IN PADAPTER_BLOCK	pAdapter
   )
/*++

Routine Description:


Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   TRUE    detect signal with far end device
   FALSE   losing signal with far end device

--*/

{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   PSUNI_REGISTERS         pPhy = ULongToPtr(pHwInfo->Phy);
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   BOOLEAN                 fLinkUp = TRUE;
   USHORT	               Value;


   do
   {
       // 
       //  Check if losing signal on the cable of receiving side.
       // 
       TBATM155_PH_READ_DEV(
           pAdapter,
		    &pPhy->Suni.rsopStatusIntStatus,
		    &Value,
           &Status);

       if (Value & fSUNI_RSOP_STATUS_LOSV)
       {
           fLinkUp = FALSE;
           break;
       }

       // 
       //  Check if defeat detected from far end device.
       // 
       TBATM155_PH_READ_DEV(
           pAdapter,
		    &pPhy->Suni.rlopControlStatus,
		    &Value,
           &Status);

       if (Value & fSUNI_RLOP_STATUS_RDIV)
       {
           fLinkUp = FALSE;
       }


   } while (FALSE);

   return (fLinkUp);
}



//=====================================
#if DBG
//=====================================

VOID
dbgDumpAtm155SuniLiteRegisters(
	IN PADAPTER_BLOCK	pAdapter
	)
/*++

Routine Description:

	This routine will dump ATM 155 PHY registers.

Arguments:

	pAdapter    -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{

   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   ULONG               DestPort;       
   ULONG               i;       
   USHORT              temp;       
   NDIS_STATUS         Status;
   PSUNI_REGISTERS     pPhy = ULongToPtr(pHwInfo->Phy);
   

	DbgPrint("==>dbgDumpAtm155SuniLiteRegisters.\n");

   for (DestPort = (ULONG)(ULONG_PTR)&pPhy->Suni, i = 0;
        i < 0x80;
        i++)
   {
       if (!(i & 0x7))
       {
		    DbgPrint("\n    0x0%lx:", i);
       }

       TBATM155_PH_READ_DEV(
           pAdapter,
           (DestPort + i),
           &temp,
           &Status);

       DbgPrint(" 0x%x,", temp);
       
   }

	DbgPrint("<==dbgDumpAtm155SuniLiteRegisters.\n");

}


//=====================================
#endif
//=====================================


