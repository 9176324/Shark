/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       debug.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "precomp.h"
#pragma hdrstop

//
//  Define module number for debug code
//
#define MODULE_NUMBER	MODULE_DEBUG


// ULONG	gTbAtm155DebugSystems = DBG_COMP_ALL;
// ULONG	gTbAtm155DebugSystems = DBG_COMP_VC | DBG_COMP_SPECIAL;
// ULONG	gTbAtm155DebugSystems = DBG_COMP_SEND | DBG_COMP_INT;
// ULONG	gTbAtm155DebugSystems = DBG_COMP_RECV;
ULONG	gTbAtm155DebugSystems = DBG_COMP_PLC2 | DBG_COMP_INIT | DBG_COMP_RESET ;

LONG   gTbAtm155DebugLevel = DBG_LEVEL_ERR;

#if DBG

#if _DBG_PACKET

VOID
dbgFreeDebugInformation(
	IN PTBATM155_LOG_INFO pLogInfo
	)
/*++

Routine Description:

	This routine will free log information that was allocated.

Arguments:

	pLogInfo	-	Pointer to the log to free.

Return Value:

	None.

--*/
{
	//
	//	Allocate the initial debug structure.
	//
	if (NULL != pLogInfo)
	{
		if (NULL != pLogInfo->RecvPacketLog)
		{
			NdisFreeSpinLock(&pLogInfo->RecvPacketLog->Lock);
			FREE_MEMORY(
				pLogInfo->RecvPacketLog,
				sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE));
		}

		if (NULL != pLogInfo->SendPacketLog)
		{
			NdisFreeSpinLock(&pLogInfo->SendPacketLog->Lock);
			FREE_MEMORY(
				pLogInfo->SendPacketLog,
				sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE));
		}

		if (NULL != pLogInfo->SpinLockLog)
		{
			NdisFreeSpinLock(&pLogInfo->SpinLockLog->Lock);
			FREE_MEMORY(
				pLogInfo->SpinLockLog,
				sizeof(SPIN_LOCK_LOG) + (sizeof(SPIN_LOCK_LOG_ENTRY) * LOG_SIZE));
		}
	
		FREE_MEMORY(pLogInfo, sizeof(TBATM155_LOG_INFO));
	}
}


VOID
dbgInitializeDebugInformation(
	OUT	PTBATM155_LOG_INFO	*pLogInfo
	)
/*++

Routine Description:

	This routine will allocate storage for the various logs the debug
	system uses.

Arguments:

	LogInfo	-	Pointer to the log information structure.

Return Value:

	NDIS_STATUS_SUCCESS
	NDIS_STATUS_FAILURE
	NDIS_STATUS_RESOURCES

--*/
{
	PTBATM155_LOG_INFO	LogInfo;
	NDIS_STATUS			Status;

	//
	//	Allocate the initial debug structure.
	//
	ALLOCATE_MEMORY(&Status, &LogInfo, sizeof(TBATM155_LOG_INFO), '11DA');
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			("Unable to allocate debug information!\n"));

		DBGBREAK(DBG_COMP_INIT, DBG_LEVEL_ERR);

		dbgFreeDebugInformation(LogInfo);

		*pLogInfo = NULL;

		return;
	}

	ASSERT(LogInfo != NULL);

	//
	//	Clear out the log memory.
	//
	ZERO_MEMORY(LogInfo, sizeof(TBATM155_LOG_INFO));

	//
	//	Allocate memory for the spin lock log.
	//
	ALLOCATE_MEMORY(
		&Status,
		&LogInfo->SpinLockLog,
		sizeof(SPIN_LOCK_LOG) + (sizeof(SPIN_LOCK_LOG_ENTRY) * LOG_SIZE),
		'21DA');

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			("Unable to allocate space for SpinLock logs\n"));

		DBGBREAK(DBG_COMP_INIT, DBG_LEVEL_ERR);

		dbgFreeDebugInformation(LogInfo);

		*pLogInfo = NULL;

		return;
	}

	ASSERT(LogInfo->SpinLockLog != NULL);

	//
	//	Initialize the spin lock log.
	//
	ZERO_MEMORY(
		LogInfo->SpinLockLog,
		sizeof(SPIN_LOCK_LOG) + (sizeof(SPIN_LOCK_LOG_ENTRY) * LOG_SIZE));

	LogInfo->SpinLockLog->Buffer = (PSPIN_LOCK_LOG_ENTRY)((PUCHAR)LogInfo->SpinLockLog + sizeof(SPIN_LOCK_LOG));
	LogInfo->SpinLockLog->CurrentEntry = (LOG_SIZE - 1);
	NdisAllocateSpinLock(&LogInfo->SpinLockLog->Lock);

	//
	//	Allocate memory for the send packet log.
	//
	ALLOCATE_MEMORY(
		&Status,
		&LogInfo->SendPacketLog,
		sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE),
		'31DA');

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			("Unable to allocate space for send packet logs\n"));

		DBGBREAK(DBG_COMP_INIT, DBG_LEVEL_ERR);

		dbgFreeDebugInformation(LogInfo);

		*pLogInfo = NULL;

		return;
	}

	ASSERT(LogInfo->SendPacketLog != NULL);

	//
	//	Initialize the packet log.
	//
	ZERO_MEMORY(
		LogInfo->SendPacketLog,
		sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE));

	LogInfo->SendPacketLog->Buffer = (PPACKET_LOG_ENTRY)((PUCHAR)LogInfo->SendPacketLog + sizeof(PACKET_LOG));
	LogInfo->SendPacketLog->CurrentEntry = (LOG_SIZE - 1);
	NdisAllocateSpinLock(&LogInfo->SendPacketLog->Lock);

	//
	//	Allocate memory for the receive packet log.
	//
	ALLOCATE_MEMORY(
		&Status,
		&LogInfo->RecvPacketLog,
		sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE),
		'41DA');

	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			("Unable to allocate space for receive packet logs\n"));

		DBGBREAK(DBG_COMP_INIT, DBG_LEVEL_ERR);

		dbgFreeDebugInformation(LogInfo);

		*pLogInfo = NULL;

		return;
	}

	ASSERT(LogInfo->RecvPacketLog != NULL);

	//
	//	Initialize the packet log.
	//
	ZERO_MEMORY(
		LogInfo->RecvPacketLog,
		sizeof(PACKET_LOG) + (sizeof(PACKET_LOG_ENTRY) * LOG_SIZE));

	LogInfo->RecvPacketLog->Buffer = (PPACKET_LOG_ENTRY)((PUCHAR)LogInfo->RecvPacketLog + sizeof(PACKET_LOG));
	LogInfo->RecvPacketLog->CurrentEntry = (LOG_SIZE - 1);
	NdisAllocateSpinLock(&LogInfo->RecvPacketLog->Lock);

	//
	//	Save the debug information with the miniport.
	//
	*pLogInfo = LogInfo;

	return;
}


VOID
dbgLogRecvPacket(
	IN	PTBATM155_LOG_INFO	pLog,
	IN	PVOID				Context1,
	IN	ULONG				Context2,
	IN	ULONG				Context3,
	IN	ULONG				Ident
	)
/*++

Routine Description:

	This routine will store 3 ULONGS of context information and an identifier
	in the receive packet log.

Arguments:

	pLog		-	Pointer to the log to store the information in.
	Context1	-	Context specific data.
	Context2	-	Context specific data.
	Context3	-	Context specific data.
	Ident		-	Identifer for the log entry.

Return Value:

	None.

--*/
{
	IF_DBG(DBG_COMP_RECV, DBG_LEVEL_LOG)
	{
		NdisAcquireSpinLock(&RPL_LOCK(pLog));
	
		RPL_HEAD(pLog) = &RPL_LOG(pLog)[RPL_CURRENT_ENTRY(pLog)];
		RPL_HEAD(pLog)->Context1 = Context1;
		RPL_HEAD(pLog)->Context2 = Context2;
		RPL_HEAD(pLog)->Context3 = Context3;
		RPL_HEAD(pLog)->Ident = Ident;
	
		if (RPL_CURRENT_ENTRY(pLog)-- == 0)
		{
			RPL_CURRENT_ENTRY(pLog) = (LOG_SIZE - 1);
		}

		NdisReleaseSpinLock(&RPL_LOCK(pLog));
	}
}

VOID
dbgLogSendPacket(
	IN	PTBATM155_LOG_INFO	pLog,
	IN	PVOID				Context1,
	IN	ULONG				Context2,
	IN	ULONG				Context3,
	IN	ULONG	 			Ident
	)
/*++

Routine Description:

	This routine will store 3 ULONGS of context information and an identifier
	in the send packet log.

Arguments:

	pLog		-	Pointer to the log to store the information in.
	Context1	-	Context specific data.
	Context2	-	Context specific data.
	Context3	-	Context specific data.
	Ident		-	Identifer for the log entry.

Return Value:

	None.

--*/
{
	IF_DBG(DBG_COMP_SEND, DBG_LEVEL_LOG)
	{
		NdisAcquireSpinLock(&SPL_LOCK(pLog));
	
		SPL_HEAD(pLog) = &SPL_LOG(pLog)[SPL_CURRENT_ENTRY(pLog)];
		SPL_HEAD(pLog)->Context1 = Context1;
		SPL_HEAD(pLog)->Context2 = Context2;
		SPL_HEAD(pLog)->Context3 = Context3;
		SPL_HEAD(pLog)->Ident = Ident;
	
		if (SPL_CURRENT_ENTRY(pLog)-- == 0)
		{
			SPL_CURRENT_ENTRY(pLog) = (LOG_SIZE - 1);
		}

		NdisReleaseSpinLock(&SPL_LOCK(pLog));
	}
}

#endif	// _DBG_PACKET

VOID
dbgDumpHardwareInformation(
	IN	PHARDWARE_INFO	pHwInfo
	)
/*++

Routine Description:

	This routine will dump the hardware information that is read from the
	eeprom.

Arguments:

	pHwInfo	-	Pointer to the hardware information structure.

Return Value:

--*/
{
	IF_DBG(DBG_COMP_INIT, DBG_LEVEL_INFO)
	{
		DbgPrint("Phy: 0x%x\n", pHwInfo->Phy);

		DbgPrint("PciConfigSpace: 0x%x\n", pHwInfo->PciConfigSpace);

		DbgPrint("PermanentAddress: %02x-%02x-%02x-%02x-%02x-%02x\n",
			pHwInfo->PermanentAddress[0],
			pHwInfo->PermanentAddress[1],
			pHwInfo->PermanentAddress[2],
			pHwInfo->PermanentAddress[3],
			pHwInfo->PermanentAddress[4],
			pHwInfo->PermanentAddress[5]);
	}
}


VOID
dbgDumpPciCommonConfig(
	IN PADAPTER_BLOCK	pAdapter
	)
/*++

Routine Description:

	This routine will dump the PCI config header.

Arguments:

	PciCommonConfig	-	 Pointer to memory block that contains the PCI header.

Return Value:

	None.

--*/
{
	if ((DBG_LEVEL_INFO >= gTbAtm155DebugLevel) &&
		((gTbAtm155DebugSystems & DBG_COMP_INIT) == DBG_COMP_INIT))
	{
		PCI_COMMON_CONFIG	PciCommonConfig;
		UINT				c;

		c = NdisReadPciSlotInformation(
				pAdapter->MiniportAdapterHandle,
               0,      // NDIS knows the real slot number
				0,
				&PciCommonConfig,
				sizeof(PCI_COMMON_CONFIG));
		if (c != sizeof(PCI_COMMON_CONFIG))
		{
			DbgPrint("Unable to read the entire PCI common config space\n");

			return;
		}

		//
		//	Display the PCI config info.
		//
		DbgPrint("PCI.VendorID = 0x%x\n", PciCommonConfig.VendorID);
		DbgPrint("PCI.DeviceID = 0x%x\n", PciCommonConfig.DeviceID);
		DbgPrint("PCI.Command = 0x%x\n", PciCommonConfig.Command);
		DbgPrint("PCI.Status = 0x%x\n", PciCommonConfig.Status);
		DbgPrint("PCI.RevisionID = 0x%x\n", PciCommonConfig.RevisionID);
		DbgPrint("PCI.ProgIf = 0x%x\n", PciCommonConfig.ProgIf);
		DbgPrint("PCI.SubClass = 0x%x\n", PciCommonConfig.SubClass);
		DbgPrint("PCI.BaseClass = 0x%x\n", PciCommonConfig.BaseClass);
		DbgPrint("PCI.CacheLineSize = 0x%x\n", PciCommonConfig.CacheLineSize);
		DbgPrint("PCI.LatencyTimer = 0x%x\n", PciCommonConfig.LatencyTimer);
		DbgPrint("PCI.HeaderType = 0x%x\n", PciCommonConfig.HeaderType);
		DbgPrint("PCI.BIST = 0x%x\n", PciCommonConfig.BIST);
	
		for (c = 0; c < PCI_TYPE0_ADDRESSES; c++)
		{
			DbgPrint("PCI.BaseAddresses[%u] = 0x%x\n", c, PciCommonConfig.u.type0.BaseAddresses[c]);
		}
	
		DbgPrint("PCI.SubVendorID = 0x%x\n", PciCommonConfig.u.type0.SubVendorID);
		DbgPrint("PCI.SubSystemID = 0x%x\n", PciCommonConfig.u.type0.SubSystemID);
	
		DbgPrint("PCI.ROMBaseAddress = 0x%x\n", PciCommonConfig.u.type0.ROMBaseAddress);
		DbgPrint("PCI.InterruptLine = 0x%x\n", PciCommonConfig.u.type0.InterruptLine);
		DbgPrint("PCI.InterruptPin = 0x%x\n", PciCommonConfig.u.type0.InterruptPin);
		DbgPrint("PCI.MinimumGrant = 0x%x\n", PciCommonConfig.u.type0.MinimumGrant);
		DbgPrint("PCI.MaximumLatency = 0x%x\n", PciCommonConfig.u.type0.MaximumLatency);
   	}
}


VOID
dbgDumpAtm155SarRegisters(
	IN PADAPTER_BLOCK	pAdapter
	)
/*++

Routine Description:

	This routine will dump ATM 155 SAR registers.

Arguments:

	pAdapter    -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PUCHAR              DestPort;       
   ULONG               i;       
   ULONG               temp;       
   

	DbgPrint("==>dbgDumpAtm155SarRegisters.\n");
   
   for (DestPort = (PUCHAR)pHwInfo->TbAtm155_SAR, i = 0x20;
        i <= 0x36;
        i++)
   {
       if (!(i & 0x1))
       {
		    DbgPrint("\n");
       }

       TBATM155_READ_PORT(
           (DestPort + i * 4),
           &temp);

		DbgPrint("  SAR(%x): 0x%lx,", i, temp);
       
   }

	DbgPrint("<==dbgDumpAtm155SarRegisters.\n");

}




VOID
dbgDumpAtm155EntryOfRxStat(
	IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               Vc
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
   ULONG               i;       
   NDIS_STATUS         Status;
   PVC_BLOCK           pVc;
   PRECV_SEG_INFO	    pRecvSegInfo;
   ULONG               Dest;
   USHORT              PhData;
   

	DbgPrint("==>dbgDumpAtm155EntryOfRxStat: Vc(%lx).\n", Vc);
   
   pVc = tbAtm155UnHashVc(pAdapter, Vc);
   pRecvSegInfo = pVc->RecvSegInfo;
   
   DbgPrint("Entry of Rx State:");
   for (Dest = pRecvSegInfo->pEntryOfRecvState, i = 0;
        i < SIZEOF_RX_STATE_ENTRY;
        i++, Dest++)
   {
       TBATM155_PH_READ_SRAM(pAdapter, Dest, &PhData, &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
	        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
		        ("Failed to open the entry of the Vc state table.\n") );

           break;
       }

       DbgPrint("(%d) %lx, ", i, (PhData&0xFFFF));

   }

	DbgPrint("\n<==dbgDumpAtm155EntryOfRxStat.\n");

}


VOID
dbgDumpAtm155TableSram(
	IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               StartDest,
   IN  ULONG               EndDest
	)
/*++

Routine Description:

	This routine dumps the table on ATM 155 on-board SRAM.

Arguments:

	pAdapter    -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{
   NDIS_STATUS     Status;
   ULONG           Dest;
   ULONG           PhData;

   for (Dest = StartDest; Dest < EndDest; Dest++)
   {
       if (!(Dest & 0x07))
       {
           if (!(Dest & 0x0ff))
           {
               // just for setting break point.
	            DbgPrint("\n");
	            DbgPrint("%lx: ", Dest);
           }
           else
           {
	            DbgPrint("\n%lx: ", Dest);
           }
       }

       TBATM155_PH_READ_SRAM(pAdapter, Dest, &PhData, &Status);

       if (NDIS_STATUS_SUCCESS != Status)
       {
	        DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
		        ("Failed to open the entry the table.\n") );

           break;
       }

       DbgPrint("%lx, ", (PhData&0xFFFF));

   }

}



VOID
dbgDumpAtm155Sram_1KVcs(
	IN  PADAPTER_BLOCK      pAdapter
	)
/*++

Routine Description:

	This routine dumps ATM 155 on-board SRAM supported up to 1K VCs.

Arguments:

	pAdapter    -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PSRAM_1K_VC_MODE     pSramAddrTbl;
   

	DbgPrint("==>dbgDumpAtm155Sram_1KVcs.\n");

   pSramAddrTbl = (PSRAM_1K_VC_MODE)pHwInfo->pSramAddrTbl;
   
	DbgPrint("Rx AAL5 big Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_Big_Free_Slot,
       pSramAddrTbl->pRx_AAL5_Small_Free_Slot
       );


	DbgPrint("\nRx AAL5 small Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_Small_Free_Slot,
       pSramAddrTbl->pRx_Raw_Free_Slot
       );

	DbgPrint("\nRx AAL5 raw Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_Raw_Free_Slot,
       pSramAddrTbl->pReserved1
       );

	DbgPrint("\nRx Vc State table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_VC_State_Tbl,
       pSramAddrTbl->pTx_VC_State_Tbl
       );

	DbgPrint("\nTx Vc State table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pTx_VC_State_Tbl,
       pSramAddrTbl->pABR_Parameter_Tbl
       );

	DbgPrint("\nABR parameter table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Parameter_Tbl,
       pSramAddrTbl->pABR_Value_Tbl
       );

	DbgPrint("\nABR value table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Value_Tbl,
       pSramAddrTbl->pRM_Cell_Data_Tbl
       );

	DbgPrint("\nRM cell table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRM_Cell_Data_Tbl,
       pSramAddrTbl->pTx_Slot_Descriptors
       );

	DbgPrint("\nTx slot descriptor table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pTx_Slot_Descriptors,
       pSramAddrTbl->pACR_LookUp_Tbl
       );

	DbgPrint("\nACR lookup table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pACR_LookUp_Tbl,
       pSramAddrTbl->pReserved2
       );

	DbgPrint("\nABR schedule table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Schedule_Tbl,
       pSramAddrTbl->pCBR_Schedule_Tbl_1
       );

	DbgPrint("\nCBR schedule table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pCBR_Schedule_Tbl_1,
       pSramAddrTbl->pEnd_Of_SRAM
       );


	DbgPrint("<==dbgDumpAtm155Sram_1KVcs.\n");

}




VOID
dbgDumpAtm155Sram_4KVcs(
	IN  PADAPTER_BLOCK      pAdapter
	)
/*++

Routine Description:

	This routine dumps ATM 155 on-board SRAM supported up to 4K VCs.

Arguments:

	pAdapter    -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PSRAM_4K_VC_MODE     pSramAddrTbl;
   

	DbgPrint("==>dbgDumpAtm155Sram_4KVcs.\n");

   pSramAddrTbl = (PSRAM_4K_VC_MODE)pHwInfo->pSramAddrTbl;
   
	DbgPrint("Rx AAL5 big Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_Big_Free_Slot,
       pSramAddrTbl->pRx_AAL5_Small_Free_Slot
       );


	DbgPrint("\nRx AAL5 small Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_Small_Free_Slot,
       pSramAddrTbl->pRx_Raw_Free_Slot
       );

	DbgPrint("\nRx AAL5 raw Free slot FIFO:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_Raw_Free_Slot,
       pSramAddrTbl->pACR_LookUp_Tbl
       );

	DbgPrint("\nACR lookup table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pACR_LookUp_Tbl,
       pSramAddrTbl->pReserved
       );

	DbgPrint("\nRx Vc State table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_VC_State_Tbl,
       pSramAddrTbl->pTx_VC_State_Tbl
       );

	DbgPrint("\nTx Vc State table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pTx_VC_State_Tbl,
       pSramAddrTbl->pABR_Parameter_Tbl
       );

	DbgPrint("\nABR parameter table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Parameter_Tbl,
       pSramAddrTbl->pABR_Value_Tbl
       );

	DbgPrint("\nABR value table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Value_Tbl,
       pSramAddrTbl->pRM_Cell_Data_Tbl
       );

	DbgPrint("\nRM cell table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRM_Cell_Data_Tbl,
       pSramAddrTbl->pTx_Slot_Descriptors
       );

	DbgPrint("\nTx slot descriptor table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pTx_Slot_Descriptors,
       pSramAddrTbl->pABR_Schedule_Tbl
       );


	DbgPrint("\nABR schedule table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pABR_Schedule_Tbl,
       pSramAddrTbl->pCBR_Schedule_Tbl_1
       );

	DbgPrint("\nCBR schedule table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pCBR_Schedule_Tbl_1,
       pSramAddrTbl->pRx_AAL5_B_Slot_Tags
       );

	DbgPrint("\nRx AAL5 Big Slot Tags Table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_B_Slot_Tags,
       pSramAddrTbl->pRx_AAL5_S_Slot_Tags
       );

	DbgPrint("\nRx AAL5 Small Slot Tags Table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_AAL5_S_Slot_Tags,
       pSramAddrTbl->pRx_Raw_Slot_Tags
       );

	DbgPrint("\nRx AAL5 Raw Slot Tags Table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_Raw_Slot_Tags,
       pSramAddrTbl->pRx_Slot_Tag_VC_State_Tbl
       );

	DbgPrint("\nRx Slot Tag VC State Table:\n");
   dbgDumpAtm155TableSram(
       pAdapter, 
       pSramAddrTbl->pRx_Slot_Tag_VC_State_Tbl,
       pSramAddrTbl->pEnd_Of_SRAM 
       );


	DbgPrint("<==dbgDumpAtm155Sram_4KVcs.\n");

}


#endif	// DBG


#if DBG_USING_LED
VOID
dbgSetLED(
	IN  PADAPTER_BLOCK      pAdapter,
	IN  UCHAR               SetLedValue,
	IN  UCHAR               MaskLedValue
	)
/*++

Routine Description:

	This routine sets LED for debugging time sensitive
   issues.

Arguments:

	pAdapter        -   Pointer to ADAPTER memory block.
	SetLedValue     -   LED setting
	MaskLedValue    -   Mask the current LED setting

Return Value:

	None.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS         Status;
   
   NdisAcquireSpinLock(&pHwInfo->Lock);
   pHwInfo->dbgLedVal = (pHwInfo->dbgLedVal & MaskLedValue) | SetLedValue;
   NdisReleaseSpinLock(&pHwInfo->Lock);

   TBATM155_PH_WRITE_DEV(pAdapter, LED_OFFSET, pHwInfo->dbgLedVal, &Status);
}


#endif // end of DBG_USING_LED



#if TB_DBG

VOID
dbgProcXmitDmaWaitQ(
   IN  PADAPTER_BLOCK  pAdapter
   )

/*++

Routine Description:

   This routine will either queue a packet to the transmit DMA queue or
   DMA the packet.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK

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
   PXMIT_SEG_INFO      pXmitSegInfo;
   PVC_BLOCK           pVc;


   //
   //	If a packet was not passed in to transmit then we need to
   //	grab them off of the pending queue.
   //
   NdisAcquireSpinLock(&pXmitDmaQ->lock);

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
       pVc = Reserved->pVc;
       pXmitSegInfo = pVc->XmitSegInfo;
       RemovePacketFromHead(&pXmitDmaQ->DmaWait, &tmpPacket);

       DbgPrint("dbgProcXmitDmaWaitQ(Vc:0x%lx)\n", pVc->VpiVci.Vci);

       if (pXmitSegInfo->FreePadTrailerBuffers == 0)
       {
           NdisMCoSendComplete(
               NDIS_STATUS_REQUEST_ABORTED,
               pVc->NdisVcHandle,
               tmpPacket);

           continue;
       }

       if (pXmitDmaQ->RemainingTransmitSlots < Reserved->PhysicalBufferCount)
       {
           NdisMCoSendComplete(
               NDIS_STATUS_REQUEST_ABORTED,
               pVc->NdisVcHandle,
               tmpPacket);

           continue;
       }

       DbgPrint("dbgProcXmitDmaWaitQ(Vc:0x%lx) is calling tbAtm155TransmitPacket\n", 
                pVc->VpiVci.Vci);

       //
       //	Transmit the packet.
       //
       tbAtm155TransmitPacket(pVc, tmpPacket);

   }

   NdisReleaseSpinLock(&pXmitDmaQ->lock);

} // end of dbgProcXmitDmaWaitQ


VOID
dbgDumpActivateVcInfo(
	IN  PADAPTER_BLOCK      pAdapter
	)
/*++

Routine Description:

	This routine displays info of VCs by going through the activate VC table
   issues.

Arguments:

	pAdapter        -   Pointer to ADAPTER memory block.

Return Value:

	None.

--*/
{
   PLIST_ENTRY         Link;
   PVC_BLOCK           pVc;
   PXMIT_SEG_INFO      pXmitSegInfo;
   PXMIT_DMA_QUEUE     pXmitDmaQ = &pAdapter->HardwareInfo->SarInfo->XmitDmaQ;


   if (DBG_PENDING_PKTS == (gTbAtm155DebugSystems & DBG_PENDING_PKTS))
   {
       //
       //	Walk through the VC's that are opened on this adapter and mark
       //	them out of resetting. Also check to see if any VCs need to 
       //  be deactivated.
       //
       DbgPrint("pAdapter->ActiveVcList:\n");
       for (Link = pAdapter->ActiveVcList.Flink;
            Link != &pAdapter->ActiveVcList;
            Link = Link->Flink)
       {
           //
           //	Get a pointer to the VC that link represents.
           //
           pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);
           
           NdisAcquireSpinLock(&pVc->lock);
           
           DbgPrint("Vc(0x%lx), VcRef:(%u), VcFlags:(0x%lx), pVc("PTR_FORMAT")\n", 
                     pVc->VpiVci.Vci, pVc->References, pVc->Flags, pVc);

           if (VC_TEST_FLAG(pVc, fVC_TRANSMIT))
           {
               pXmitSegInfo = pVc->XmitSegInfo;
               DbgPrint("DmaCompleting.Ref:(%u)\n", pXmitSegInfo->DmaCompleting.References);
               DbgPrint("SegWait.Ref:(%u)\n", pXmitSegInfo->SegWait.References);
               DbgPrint("FreePadTrailerBufs:(%u), BeOrBeingUsedPadTrailerBufs:(%u)\n", 
                         pXmitSegInfo->FreePadTrailerBuffers, pXmitSegInfo->BeOrBeingUsedPadTrailerBufs);
           }
           
           NdisReleaseSpinLock(&pVc->lock);

       }

       DbgPrint("pAdapter->InactiveVcList:\n");
       for (Link = pAdapter->InactiveVcList.Flink;
            Link != &pAdapter->InactiveVcList;
            Link = Link->Flink)
       {
           //
           //	Get a pointer to the VC that link represents.
           //
           pVc = CONTAINING_RECORD(Link, VC_BLOCK, Link);
           
           NdisAcquireSpinLock(&pVc->lock);
           
           DbgPrint("Vc(0x%lx), VcRef:(%u), VcFlags:(0x%lx), pVc("PTR_FORMAT")\n", 
                     pVc->VpiVci.Vci, pVc->References, pVc->Flags, pVc);

           NdisReleaseSpinLock(&pVc->lock);

       }

       NdisAcquireSpinLock(&pXmitDmaQ->lock);
       DbgPrint("RemainingTransmitSlots:(%u)\n", pXmitDmaQ->RemainingTransmitSlots);
       DbgPrint("DmaWait.Ref:(%u), AdapterFlags(0x%lx)\n\n", 
                pXmitDmaQ->DmaWait.References, pAdapter->Flags);
       NdisReleaseSpinLock(&pXmitDmaQ->lock);

   } // end of if
    
   if (DBG_XMIT_PKTS == (gTbAtm155DebugSystems & DBG_XMIT_PKTS))
   {
       dbgProcXmitDmaWaitQ(pAdapter);
   }

} 

    
#endif // end of TB_DBG
    

