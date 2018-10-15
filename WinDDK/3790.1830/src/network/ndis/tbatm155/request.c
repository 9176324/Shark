/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       request.c

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
   3/25/98         jcao        Custom OIDs Support  (keyword:tboid)
   02/25/99        hhan        Example of WMI Support

--*/

#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_REQUEST

#include "tboid.h"

NDIS_OID gTbAtm155OidListSupported[] =
{
   OID_GEN_CO_SUPPORTED_LIST,
   OID_GEN_CO_HARDWARE_STATUS,
   OID_GEN_CO_MEDIA_SUPPORTED,
   OID_GEN_CO_MEDIA_IN_USE,
   OID_GEN_CO_LINK_SPEED,
   OID_GEN_CO_VENDOR_ID,
   OID_GEN_CO_VENDOR_DESCRIPTION,
   OID_GEN_CO_DRIVER_VERSION,
   OID_GEN_CO_PROTOCOL_OPTIONS,
   OID_GEN_CO_MAC_OPTIONS,
   OID_GEN_CO_MEDIA_CONNECT_STATUS,
   OID_GEN_CO_VENDOR_DRIVER_VERSION,
   OID_GEN_CO_MINIMUM_LINK_SPEED,
   OID_GEN_CO_SUPPORTED_GUIDS,
   OID_GEN_CO_XMIT_PDUS_OK,
   OID_GEN_CO_RCV_PDUS_OK,
   OID_GEN_CO_XMIT_PDUS_ERROR,
   OID_GEN_CO_RCV_PDUS_ERROR,
   OID_GEN_CO_RCV_PDUS_NO_BUFFER,
   OID_GEN_CO_RCV_CRC_ERROR,
   OID_ATM_SUPPORTED_VC_RATES,
   OID_ATM_SUPPORTED_SERVICE_CATEGORY,
   OID_ATM_SUPPORTED_AAL_TYPES,
   OID_ATM_HW_CURRENT_ADDRESS,
   OID_ATM_MAX_ACTIVE_VCS,
   OID_ATM_MAX_ACTIVE_VCI_BITS,
   OID_ATM_MAX_ACTIVE_VPI_BITS,
   OID_ATM_MAX_AAL0_PACKET_SIZE,
   OID_ATM_MAX_AAL5_PACKET_SIZE,
   OID_ATM_GET_NEAREST_FLOW,
   OID_ATM_RCV_CELLS_OK,
   OID_ATM_XMIT_CELLS_OK,
   OID_ATM_RCV_CELLS_DROPPED,
   OID_ATM_RCV_INVALID_VPI_VCI,
   OID_ATM_RCV_REASSEMBLY_ERROR,
   // tboid                            New OIDs defined in tboid.h
   OID_TBATM_READ_PEEPHOLE,
   OID_TBATM_READ_CSR,
   // custom oid WMI support
   OID_VENDOR_STRING
};

NDIS_GUID	gTbAtm155GuidListSupported[4] =
{
	{{0xfa214809, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
    OID_VENDOR_STRING,
    (ULONG) -1,
    (fNDIS_GUID_TO_OID|fNDIS_GUID_ANSI_STRING)
	},
	{{0xfa21480a, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
	 OID_ATM_RCV_INVALID_VPI_VCI,
	 4,
	 fNDIS_GUID_TO_OID
	},
	{{0xfa21480b, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
	 OID_GEN_CO_XMIT_PDUS_OK,
	 8,
	 fNDIS_GUID_TO_OID
	},
	{{0xfa21480c, 0xe35f, 0x11d0, 0x96, 0x92, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c},
	 OID_GEN_CO_RCV_PDUS_OK,
	 8,
	 fNDIS_GUID_TO_OID
	}
};

const UCHAR	gTbAtm155DriverDescription[] = "Toshiba ATM 155 Driver";

NDIS_STATUS
tbAtm155QueryInformation(
	IN		PADAPTER_BLOCK	pAdapter,
	IN		PVC_BLOCK		pVc,
	IN OUT	PNDIS_REQUEST	pRequest
	)
/*++

Routine Description:

	This routine will handle the general and ATM specific OID queries.

Arguments:

	pAdapter	-	Pointer to the adapter block to query.
	pVc			-	Optionally points to the specific VC to query.
	pRequest	-	Pointer to the request.

Return Value:

--*/
{
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
	PUCHAR                  DataToMove;
	ULONG	                DataLength;
	ULONG	                ulTemp;
	USHORT                  usTemp;
	ATM_VC_RATES_SUPPORTED  VcRates;
   PCO_CALL_PARAMETERS	    CallParameters;
	PCO_MEDIA_PARAMETERS	MediaParameters;
	PATM_MEDIA_PARAMETERS   pAtmMediaParms;
	ULONG	                PreScale;
	ULONG	                RateResolution;
	BOOLEAN                 fRoundUp;
	NDIS_CO_LINK_SPEED      CoLinkSpeed;
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   // tboid
   PTB_PEEPHOLE	        pPeepholePara;
   PTB_CSR                 pCsrPara;
   PUCHAR                  CsrVirtualAddr;



	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("==>tbAtm155QueryInformation\n"));

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("tbAtm155QueryInformation:\n   pAdapter - 0x%x\n   pVc - 0x%x\n   pRequest - 0x%x\n   OID - 0x%x\n",
		 pAdapter, pVc, pRequest, pRequest->DATA.QUERY_INFORMATION.Oid));

	//
	//	Setup the default to point to the
	//	ULONG type.
	//
	DataToMove = (PUCHAR)&ulTemp;
	DataLength = sizeof(ulTemp);

	//
	//	Process the OID.
	//
	switch (pRequest->DATA.QUERY_INFORMATION.Oid)
	{
		case OID_GEN_CO_SUPPORTED_LIST:

			//
			//	Setup the data pointer and length to copy the supported
			//	oid list into the information buffer passed to us.
			//
			DataToMove = (PUCHAR)gTbAtm155OidListSupported;
			DataLength = sizeof(gTbAtm155OidListSupported);

			break;

		case OID_GEN_CO_HARDWARE_STATUS:

			//
			//	Copy the hardware status into the users buffer.
			//
			DataToMove = (PUCHAR)&pAdapter->HardwareStatus;
			DataLength = sizeof(pAdapter->HardwareStatus);

			break;

		case OID_GEN_CO_MEDIA_SUPPORTED:
		case OID_GEN_CO_MEDIA_IN_USE:

			//
			//	We support ATM.
			//
			ulTemp = NdisMediumAtm;

			break;

		case OID_GEN_CO_MINIMUM_LINK_SPEED:
		case OID_GEN_CO_LINK_SPEED:

			//
			//	Link speed depends upon the type of adapter.
			//
			CoLinkSpeed.Inbound = CoLinkSpeed.Outbound =
										ATM_USER_DATA_RATE_SONET_155;

			DataToMove = (PUCHAR)&CoLinkSpeed;
			DataLength = sizeof(CoLinkSpeed);

			break;

		case OID_GEN_CO_VENDOR_ID:

			//
			//	Copy the first 3 bytes of the hardware address for
			//	the vendor id.
			//
			DataToMove[0] = pAdapter->HardwareInfo->PermanentAddress[0];
			DataToMove[1] = pAdapter->HardwareInfo->PermanentAddress[1];
			DataToMove[2] = pAdapter->HardwareInfo->PermanentAddress[2];
			DataToMove[3] = 0;

			break;

		case OID_GEN_CO_VENDOR_DESCRIPTION:

			DataToMove = (PUCHAR)gTbAtm155DriverDescription;
			DataLength = sizeof(gTbAtm155DriverDescription);

			break;

		case OID_GEN_CO_DRIVER_VERSION:

			//
			//	Return the version of NDIS that we expect.
			//
			usTemp = ((TBATM155_NDIS_MAJOR_VERSION << 8) |
							    TBATM155_NDIS_MINOR_VERSION);
			DataToMove = (PUCHAR)&usTemp;
			DataLength = sizeof(USHORT);

			break;

		case OID_GEN_CO_PROTOCOL_OPTIONS:

			//
			//	We don't support protocol options.
			//
			ulTemp = 0;

			break;

		case OID_GEN_CO_MAC_OPTIONS:

			//
			//	We don't support MAC options.
			//
			ulTemp = 0;

			break;

		case OID_GEN_CO_MEDIA_CONNECT_STATUS:

			//
			//	Return whether or not we have a link.
			//
			ulTemp = pAdapter->MediaConnectStatus;

			break;

		case OID_GEN_CO_VENDOR_DRIVER_VERSION:

			ulTemp = (TBATM155_DRIVER_MAJOR_VERSION << 8) |
						TBATM155_DRIVER_MINOR_VERSION;
	
			break;

		case OID_ATM_SUPPORTED_VC_RATES:

			//
			//	This OID is used to query the miniport for it's minimum and
			//	maximum supported VC rates in cells per second.
			//
			VcRates.MaxCellRate = pAdapter->MaximumBandwidth;
			VcRates.MinCellRate = pAdapter->MinimumCbrBandwidth;

			DataToMove = (PUCHAR)&VcRates;
			DataLength = sizeof(VcRates);

			break;

		case OID_ATM_SUPPORTED_SERVICE_CATEGORY:
			//
			//	This OID is used to query the miniport for the supported
			//	QoS.
			//
#if    ABR_CODE

			ulTemp = ATM_SERVICE_CATEGORY_CBR | ATM_SERVICE_CATEGORY_UBR |
                    ATM_SERVICE_CATEGORY_ABR;

#else     // not ABR_CODE

			ulTemp = ATM_SERVICE_CATEGORY_CBR | ATM_SERVICE_CATEGORY_UBR;

#endif     // end of ABR_CODE

			break;

		case OID_ATM_SUPPORTED_AAL_TYPES:
			//
			//	This OID is used to query the miniport for the AAL types
			//	supported (either in software or hardware).
			//
			ulTemp = AAL_TYPE_AAL5;

			break;

		case OID_ATM_HW_CURRENT_ADDRESS:

			//
			//	This OID is used to query the miniport for it's
			//	hardware address.
			//
			DataToMove = pAdapter->HardwareInfo->StationAddress;
			DataLength = ATM_MAC_ADDRESS_LENGTH;

			break;

		case OID_ATM_MAX_ACTIVE_VCS:
			//
			//	Maximum active VCS are 1K or 4K.
			//
			ulTemp = pAdapter->HardwareInfo->NumOfVCs;

			break;

		case OID_ATM_MAX_ACTIVE_VCI_BITS:
			//
			//	We support 10 bits of VCI addresses...
			//
			ulTemp = 10;

			break;

		case OID_ATM_MAX_ACTIVE_VPI_BITS:

			//
			//	Only VPI 0...
			//
			ulTemp = 0;

			break;

		case OID_ATM_MAX_AAL0_PACKET_SIZE:

			//
			//	This ones easy...
			//
			ulTemp = CELL_PAYLOAD_SIZE;

			break;

		case OID_ATM_MAX_AAL5_PACKET_SIZE:

			//	
			//	Return the max packet size that we can support on
			//	AAL5.
           //
           // This value should be larger than 18190.
           //

			ulTemp = (ULONG)MAX_AAL5_PDU_SIZE;

			break;							 	

		case OID_ATM_GET_NEAREST_FLOW:

			//
			//	They must pass us a VC to query this.
			//
			if (NULL == pVc)
			{
				Status = NDIS_STATUS_INVALID_DATA;

				break;
			}

			//
			//	Verify the size of the CO_CALL_PARAMETERS
			//
			if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength <
					(sizeof(CO_CALL_PARAMETERS) + sizeof(CO_MEDIA_PARAMETERS) + sizeof(CO_SPECIFIC_PARAMETERS) + sizeof(ATM_MEDIA_PARAMETERS)))
			{
				Status = NDIS_STATUS_INVALID_DATA;

				break;
			}

			//
			//	This is used to determine the flow rate that our hardware
			//	can handle that is the closest to what the caller wants.
			//
			CallParameters = (PCO_CALL_PARAMETERS)pRequest->DATA.QUERY_INFORMATION.InformationBuffer;
			MediaParameters = CallParameters->MediaParameters;

			//
			//	Validate that we have ATM_MEDIA_SPECIFIC information....
			//
			if ((MediaParameters->MediaSpecific.ParamType != ATM_MEDIA_SPECIFIC) ||
				(MediaParameters->MediaSpecific.Length != sizeof(ATM_MEDIA_PARAMETERS)))
			{
				DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
					("tbAtm155QueryInformation: OID_ATM_GET_NEAREST_FLOW has invalid ATM_MEDIA_PARAMETERS\n"));

				Status = NDIS_STATUS_INVALID_DATA;

				break;
			}

			//
			//	Get a pointer to the atm media parameters.
			//
			pAtmMediaParms = (PATM_MEDIA_PARAMETERS)MediaParameters->MediaSpecific.Parameters;

			fRoundUp = (BOOLEAN)((MediaParameters->Flags & ROUND_UP_FLOW) == ROUND_UP_FLOW);

			//
			//	Adjust the traffic parameters for both the receive and
			//	transmits.
			//
			if (MediaParameters->Flags & TRANSMIT_VC)
			{
				Status = tbAtm155AdjustTrafficParameters(
							pAdapter,
							&pAtmMediaParms->Transmit,
							fRoundUp,
							&PreScale,
							&RateResolution);
				if (NDIS_STATUS_SUCCESS != Status)
				{
					break;
				}
			}

			return(NDIS_STATUS_SUCCESS);

			break;

		case OID_GEN_CO_SUPPORTED_GUIDS:

			//
			//	Point to the list of supported guids.
			//
			DataToMove = (PUCHAR)gTbAtm155GuidListSupported;
			DataLength = sizeof(gTbAtm155GuidListSupported);

			break;

       // tboid++
       case OID_TBATM_READ_PEEPHOLE:

           //
           //  Vendor Specific OID, Peep-hole read
           //
           DataLength = sizeof(TB_PEEPHOLE);
           if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength < DataLength)
           {
               pRequest->DATA.QUERY_INFORMATION.BytesNeeded = DataLength;
               Status = NDIS_STATUS_INVALID_LENGTH;
               break;
           }

           pPeepholePara = pRequest->DATA.QUERY_INFORMATION.InformationBuffer;
           if ((pPeepholePara->access_device & 0x01) == 0x01)
           {
               //
               //  Peripheral, access devices via peep-hole
               //
               if (pPeepholePara->address <= 0x3ffff)
               {
                   TBATM155_PH_READ_DEV( pAdapter,
                               pPeepholePara->address,
                               &pPeepholePara->data,
                               &Status);
               }
               else
               {
                   Status = NDIS_STATUS_FAILURE;
               }
           }
           else
           {
               //
               // SRAM, access SRAM via peep-hole
               //
               if (pPeepholePara->address <= 0xffff)
               {
                   TBATM155_PH_READ_SRAM(
                               pAdapter,
                               pPeepholePara->address,
                               &pPeepholePara->data,
                               &Status);
               }
               else
               {
                   Status = NDIS_STATUS_FAILURE;
               }
           }

           if (NDIS_STATUS_SUCCESS == Status)
           {
               pPeepholePara->status = 1;
               pRequest->DATA.QUERY_INFORMATION.BytesWritten = DataLength;
           }
           else
           {
               pPeepholePara->status = 0;
           }

           return(Status);

       case OID_TBATM_READ_CSR:

           //
           //  Vendor Specific OID, CSR read
           //
           DataLength = sizeof(TB_CSR);
           if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength < DataLength)
           {
               pRequest->DATA.QUERY_INFORMATION.BytesNeeded = DataLength;
               Status = NDIS_STATUS_INVALID_LENGTH;
               break;
           }

           pCsrPara = pRequest->DATA.QUERY_INFORMATION.InformationBuffer;

           if (pCsrPara->address <= 0x36)
           {
               //
               // access CSR
               //
               CsrVirtualAddr = (PUCHAR)pHwInfo->TbAtm155_SAR + (pCsrPara->address << 2);
               TBATM155_READ_PORT(CsrVirtualAddr, &pCsrPara->data);

               pCsrPara->status = 1;
               pRequest->DATA.QUERY_INFORMATION.BytesWritten = DataLength;
           }
           else 	
           {
               Status = NDIS_STATUS_FAILURE;
               pCsrPara->status = 0;
           }

           return(Status);
        // tboid--

       case OID_VENDOR_STRING:

        	DataToMove = (PUCHAR)gTbAtm155DriverDescription;
        	DataLength = sizeof(gTbAtm155DriverDescription);
                break;

	    default:

			DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
				("tbAtm155QueryInformation: Invalid Query Information\n"));

			Status = NDIS_STATUS_NOT_SUPPORTED;

			break;
	}

	//
	//	Copy the data to the buffer provided if necessary.
	//
	if (NDIS_STATUS_SUCCESS == Status)
	{
		if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength < DataLength)
		{
			pRequest->DATA.QUERY_INFORMATION.BytesNeeded = DataLength;
			Status = NDIS_STATUS_INVALID_LENGTH;
		}
		else
		{
			NdisMoveMemory(
				pRequest->DATA.QUERY_INFORMATION.InformationBuffer,
				DataToMove,
				DataLength);

			pRequest->DATA.QUERY_INFORMATION.BytesWritten = DataLength;
		}
	}

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("<==tbAtm155QueryInformation (Status: 0x%lx)\n", Status));

	return(Status);
}


NDIS_STATUS
tbAtm155SetInformation(
	IN		PADAPTER_BLOCK	pAdapter,
	IN		PVC_BLOCK		pVc,
	IN OUT	PNDIS_REQUEST	pRequest
	)
/*++

Routine Description:

	This routine will process any set requests.

Arguments:

	pAdapter	-	Pointer to the ADAPTER_BLOCK.
	pVc			-	Pointer to the VC_BLOCK.
	pRequest	-	Pointer to the NDIS_REQUEST that identifies the request.

Return Value:

	NDIS_STATUS_NOT_SUPPORTED if we don't understand the request.

--*/
{
   // tboid
   NDIS_STATUS	    Status = NDIS_STATUS_SUCCESS;

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("==>tbAtm155SetInformation\n"));

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("   pAdapter: 0x%x\n   pVc: 0x%x\n   pRequest: 0x%x\n   OID: 0x%x\n",
		 pAdapter, pVc, pRequest, pRequest->DATA.SET_INFORMATION.Oid));

   // tboid++
   switch (pRequest->DATA.SET_INFORMATION.Oid)
   {
      // tboid--

		default:
			UNREFERENCED_PARAMETER(pAdapter);
			UNREFERENCED_PARAMETER(pVc);
			UNREFERENCED_PARAMETER(pRequest);

			Status = NDIS_STATUS_NOT_SUPPORTED;
	}

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("<==tbAtm155SetInformation\n"));

	return (Status);
}


NDIS_STATUS
tbAtm155QueryStatistics(
	IN		PADAPTER_BLOCK	pAdapter,
	IN		PVC_BLOCK		pVc,
	IN OUT	PNDIS_REQUEST	pRequest
	)
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
	NDIS_STATUS				Status;
	PULONG	   			DataToMove;
	ULONG						Data;
	ULONG	   				DataLength;
	PTBATM155_STATISTICS_INFO	pStatInfo;

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("==>tbAtm155QueryStatistics\n"));

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
		("tbAtm155QueryStatistics:\n   pAdapter - 0x%x\n   pVc - 0x%x\n   pRequest - 0x%x\n   OID - 0x%x\n",
		 pAdapter, pVc, pRequest, pRequest->DATA.QUERY_INFORMATION.Oid));

	//
	//	First see if the OID is handled by the QueryInformation handler.
	//
	Status = tbAtm155QueryInformation(pAdapter, pVc, pRequest);

	if ((NDIS_STATUS_SUCCESS == Status) ||
		(NDIS_STATUS_PENDING == Status) ||
		(NDIS_STATUS_INVALID_LENGTH == Status))
	{
		DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
			("tbAtm155QueryStatistics: Request handled by tbAtm155QueryInformation\n"));

		//
		//	We are done.
		//
		return(Status);
	}

	//
	//	Default the status to success for below.
	//
	Status = NDIS_STATUS_SUCCESS;

	//
	//	Default length.
	//
	DataToMove = &Data;
	DataLength = sizeof(ULONG);

	//
	//	The switch statement below will copy the statistic counter
	//	to the buffer so that it can finally be copied into the
	//	InformationBuffer.  If the VC_BLOCK passed in is NULL then the
	//	query is for global information.
	//
	pStatInfo = (NULL == pVc) ? &pAdapter->StatInfo : &pVc->StatInfo;

	switch (pRequest->DATA.QUERY_INFORMATION.Oid)
	{
		case OID_GEN_CO_XMIT_PDUS_OK:

			Data = pStatInfo->XmitPdusOk;

			break;

		case OID_GEN_CO_XMIT_PDUS_ERROR:

			Data = pStatInfo->XmitPdusError;

			break;

		case OID_GEN_CO_RCV_PDUS_OK:

			Data = pStatInfo->RecvPdusOk;

			break;

		case OID_GEN_CO_RCV_PDUS_ERROR:

			Data = pStatInfo->RecvPdusError;

			break;

		case OID_GEN_CO_RCV_PDUS_NO_BUFFER:

			Data = pStatInfo->RecvPdusNoBuffer;

			break;

		case OID_GEN_CO_RCV_CRC_ERROR:

			Data = pStatInfo->RecvCrcError;

			break;

		case OID_ATM_RCV_CELLS_OK:

			Data = pStatInfo->RecvCellsOk;

			break;

		case OID_ATM_XMIT_CELLS_OK:

			Data = pStatInfo->XmitCellsOk;

			break;

		case OID_ATM_RCV_CELLS_DROPPED:

			Data = pStatInfo->RecvCellsDropped;

			break;

		case OID_ATM_RCV_INVALID_VPI_VCI:

			Data = pStatInfo->RecvInvalidVpiVci;

			break;

		case OID_ATM_RCV_REASSEMBLY_ERROR:

			DataToMove = &pStatInfo->RecvReassemblyErr;

			break;

		default:

			DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_ERR,
				("tbAtm155QueryStatistics: Invalid Query Statistics\n"));

			Status = NDIS_STATUS_NOT_SUPPORTED;

			break;
	}

	//
	//	Copy the data to the buffer provided if necessary.
	//
	if (NDIS_STATUS_SUCCESS == Status)
	{
		if (pRequest->DATA.QUERY_INFORMATION.InformationBufferLength < DataLength)
		{
			pRequest->DATA.QUERY_INFORMATION.BytesNeeded = DataLength;
			Status = NDIS_STATUS_INVALID_LENGTH;
		}
		else
		{
			NdisMoveMemory(
				pRequest->DATA.QUERY_INFORMATION.InformationBuffer,
				DataToMove,
				DataLength);

			pRequest->DATA.QUERY_INFORMATION.BytesWritten = DataLength;
		}
	}

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("tbAtm155QueryStatistics returning: 0x%lx\n", Status));

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("<==tbAtm155QueryStatistics\n"));

	return(Status);
}


NDIS_STATUS
TbAtm155Request(
	IN		NDIS_HANDLE		MiniportAdapterContext,
	IN		NDIS_HANDLE		MiniportVcContext	OPTIONAL,
	IN	OUT	PNDIS_REQUEST	NdisCoRequest
	)
{
	PADAPTER_BLOCK	pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
	PVC_BLOCK		pVc = (PVC_BLOCK)MiniportVcContext;
	NDIS_STATUS		Status = NDIS_STATUS_FAILURE;

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("==>TbAtm155Request\n"));

	switch (NdisCoRequest->RequestType)
	{
		case NdisRequestQueryInformation:

			DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
				("TbAtm155Request:  NdisRequestQueryInformation\n"));

			Status = tbAtm155QueryInformation(pAdapter, pVc, NdisCoRequest);

			break;

		case NdisRequestSetInformation:

			DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
				("TbAtm155Request:  NdisRequestSetInformation\n"));

			Status = tbAtm155SetInformation(pAdapter, pVc, NdisCoRequest);

			break;

		case NdisRequestQueryStatistics:

			DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
				("TbAtm155Request:  NdisRequestQueryStatistics\n"));

			Status = tbAtm155QueryStatistics(pAdapter, pVc, NdisCoRequest);

			break;
	}

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("TbAtm155Request: Return status - 0x%x\n", Status));

	DBGPRINT(DBG_COMP_REQUEST, DBG_LEVEL_INFO,
		("<==TbAtm155Request\n"));

	return(Status);
}

