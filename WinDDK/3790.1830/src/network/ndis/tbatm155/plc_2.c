/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       plc_2.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

   This file contains Toshiba's PLC2 specific handling.


Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	05/09/97		awang		Initial of Toshiba ATM 155 Device Driver.
	05/27/97		awang		Uses LOF instead of Receive Status1 Reg
                               to check if disconnected.

--*/

#include "basetsd.h"
#include "precomp.h"
#include "plc_2.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_PLC2



NDIS_STATUS
tbAtm155InitPLC2PhyRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will initialize the PHY registers.

Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

   NDIS_STATUS_ADAPTER_NOT_FOUND   if the adapter is not supported.
   NDIS_STATUS_SUCCESS             otherwise.

--*/
{
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
   PPLC2_REGISTERS pPhy = ULongToPtr(pHwInfo->Phy);


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitPLC2PhyRegisters()\n"));

   // 
   //  Set control values of MMF & Plc-2 model through LED interface.
   // 
   pHwInfo->LedVal = fLED_PLC2_155MB_TXENB;

   //
   //	Set and clear the reset bit for the phy.
   //
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Plc2.confgContResetStop,
       fPLC2_MRI_RESET,
       &Status);

   NdisStallExecution(100);        // let's give a 100-ms-delay.

   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Plc2.confgCellManagement2,
       fPLC2_CMS2_DROP,
       &Status);

   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Plc2.confgContResetStop,
       (fPLC2_CONT_AUTOCSE | fPLC2_CONT_LOOCDE | fPLC2_CONT_OOLDE),
       &Status);

   // Enable an interrupt generation, when LOF status change is detected
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Plc2.stIntrMask1,
       fPLC2_INTIND1_LOF,
       &Status);

   // Enable an interrupt generation, when LRDI status change is detected
   TBATM155_PH_WRITE_DEV(
       pAdapter,
       &pPhy->Plc2.stIntrMask2,
       fPLC2_St2ALARM_LRDI,
       &Status);


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitPLC2PhyRegisters()\n"));

   return(Status);
}



VOID
tbAtm155PLC2Interrupt(
	IN	PADAPTER_BLOCK	pAdapter
	)
/*++

Routine Description:

   This routine will process the PHY PLC_2 error interrupts.

Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   None.

--*/
{
   PHARDWARE_INFO  pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
   BOOLEAN         fLinkUp = FALSE;
   UCHAR           LedValue;
   

   DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
       ("==>tbAtm155PLC2Interrupt()\n"));


   fLinkUp = tbAtm155CheckPLC2LinkUp(pAdapter);

   // 
   //  Only care about LinkUp LED status only.
   // 
   LedValue = 0;

   if (fLinkUp == TRUE)
   {
       LedValue |= LED_LNKUP_ON_GREEN;
   }

#if DBG
   DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO, 
           ("fLinkUp(%s), LedValue=0x%lx, pHwInfo->LedVal=0x%lx\n",
             (fLinkUp == TRUE)? "True":"False", LedValue, pHwInfo->LedVal));
#endif // end of DBG

   if ((pHwInfo->LedVal & LED_LNKUP_ALL_BITS) != LedValue)
   {
       //
       // Linkup status has been changed. Handle it.
       //
       if ((fLinkUp == TRUE) && 
           (pAdapter->MediaConnectStatus == NdisMediaStateDisconnected))
       {
           pAdapter->MediaConnectStatus = NdisMediaStateConnected;

           NdisMIndicateStatus(
               pAdapter->MiniportAdapterHandle,
               NDIS_STATUS_MEDIA_CONNECT,
               (PVOID)0,
               0);
           // NOTE:
           // have to indicate status complete every time you 
           // indicate status
           NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);

           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO, ("NDIS_STATUS_MEDIA_CONNECT\n"));

       }
       else if ((fLinkUp != TRUE) && 
                (pAdapter->MediaConnectStatus == NdisMediaStateConnected))
       {
           pAdapter->MediaConnectStatus = NdisMediaStateDisconnected;

           NdisMIndicateStatus(
               pAdapter->MiniportAdapterHandle,
               NDIS_STATUS_MEDIA_DISCONNECT,
               (PVOID)0,
               0);
           // NOTE:
           // have to indicate status complete every time you 
           // indicate status
           NdisMIndicateStatusComplete(pAdapter->MiniportAdapterHandle);

           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO, ("NDIS_STATUS_MEDIA_DISCONNECT\n"));

           NdisDprAcquireSpinLock(&pAdapter->lock);

           pAdapter->StatInfo.RecvCellsDropped++;

           NdisDprReleaseSpinLock(&pAdapter->lock);
       }

       // 
       //  Update LinkUp LEDs only.
       // 
       NdisDprAcquireSpinLock(&pHwInfo->Lock);
       pHwInfo->LedVal = (pHwInfo->LedVal & ~LED_LNKUP_ALL_BITS) | LedValue;         // save the current value.
       NdisDprReleaseSpinLock(&pHwInfo->Lock);

       TBATM155_PH_WRITE_DEV(
           pAdapter,
           LED_OFFSET,
           pHwInfo->LedVal,
           &Status);

   } // end of if (pHwInfo->LedVal != LedValue)

   DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
       ("<==tbAtm155PLC2Interrupt()\n"));
}


BOOLEAN
tbAtm155CheckPLC2LinkUp(
	IN PADAPTER_BLOCK	pAdapter
   )
/*++

Routine Description:


Arguments:

   pAdapter	-	Pointer to the ADAPTER_BLOCK.

Return Value:

   TRUE        Detect signal with far end device
   FALSE       Losing signal with far end device

--*/

{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   PPLC2_REGISTERS         pPhy = ULongToPtr(pHwInfo->Phy);
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   BOOLEAN                 fLinkUp = TRUE;
   USHORT	               Value;

   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
       ("==>tbAtm155CheckPLC2LinkUp()\n"));

   do
   {
       //
       //	Check OOF bit in the status register 1 in PLC_2
       //
       TBATM155_PH_READ_DEV(
           pAdapter,
           &pPhy->Plc2.stIntrStatus1,
		    &Value,
           &Status);

       if (Value & fPLC2_INTIND1_LOF)
       {
           // clear this interrup status bit.
           Value &= ~fPLC2_INTIND1_LOF;
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               &pPhy->Plc2.stIntrStatus1,
               Value,
               &Status);

#if DBG

           TBATM155_PH_READ_DEV(
               pAdapter,
               &pPhy->Plc2.stIntrStatus1,
		        &Value,
               &Status);

           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
               ("Plc2.stIntrStatus1 (%x)\n", Value));
           
#endif // end of DBG

       }

       //
       //	Check L_RDI bit in the Status2 Alarm register in PLC_2
       //  This bit indicates L-RDI state of receiving frame.
       //
       TBATM155_PH_READ_DEV(
           pAdapter,
           &pPhy->Plc2.stIntrStatus2,
		    &Value,
           &Status);

       if (Value & fPLC2_St2ALARM_LRDI)
       {
           // clear the interrup status bit.
           Value &= ~fPLC2_St2ALARM_LRDI;
           TBATM155_PH_WRITE_DEV(
               pAdapter,
               &pPhy->Plc2.stIntrStatus2,
               Value,
               &Status);

#if DBG

           TBATM155_PH_READ_DEV(
               pAdapter,
               &pPhy->Plc2.stIntrStatus2,
		        &Value,
               &Status);

           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
               ("Plc2.stIntrStatus2 (%x)\n", Value));
           
#endif // end of DBG

       }

       //
       //	Check OOF bit in the status register 1 in PLC_2
       //
       TBATM155_PH_READ_DEV(
           pAdapter,
           &pPhy->Plc2.stReceiveStatus1,
		    &Value,
           &Status);

#if DBG
           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
               ("Plc2.stReceiveStatus1 (%x)\n", Value));
#endif // end of DBG

       if (Value & fPLC2_INTIND1_LOF)
       {
           fLinkUp = FALSE;
           break;
       }

       //
       //	Check L_RDI bit in the Status2 Alarm register in PLC_2
       //  This bit indicates L-RDI state of receiving frame.
       //
       TBATM155_PH_READ_DEV(
           pAdapter,
           &pPhy->Plc2.stAlarmStatus2,
		    &Value,
           &Status);

#if DBG
           DBGPRINT(DBG_COMP_PLC2, DBG_LEVEL_INFO,
               ("Plc2.stAlarmStatus (%x)\n", Value));
#endif // end of DBG

       if (Value & fPLC2_St2ALARM_LRDI)
       {
           fLinkUp = FALSE;
       }

   } while (FALSE);

   DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
       ("<==tbAtm155CheckPLC2LinkUp\n"));

   return (fLinkUp);

}



//=====================================
#if DBG
//=====================================

VOID
dbgDumpAtm155PLC2Registers(
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
   PPLC2_REGISTERS     pPhy = ULongToPtr(pHwInfo->Phy);
   

	DbgPrint("==>dbgDumpAtm155PLC2Registers.\n");
   
   for (DestPort = (ULONG)(ULONG_PTR)&pPhy->Plc2, i = 0;
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

	DbgPrint("<==dbgDumpAtm155PLC2Registers.\n");

}


//=====================================
#endif
//=====================================

