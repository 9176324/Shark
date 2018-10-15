/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   prpget.c

Abstract:

   Code related to "getting"  about properties 

Author:


Environment:

   Kernel mode only

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

Revision History:

--*/



#include "warn.h"
#include "wdm.h"

#include <strmini.h>
#include <ksmedia.h>

#include "usbdi.h"
#include "usbcamdi.h"

#include "intelcam.h"

#ifndef __PRPFTN_H__
#include "prpftn.h"
#endif

#ifndef __PRPOBJ_H__
#include "probj.h"
#endif

extern MaxPktSizePerInterface BusBWArray[BUS_BW_ARRAY_SIZE];


/*
** INTELCAM_GetCameraProperty()
**
** Arguments:
**
**  DeviceContext - driver context
**
** Returns:
**
**  NT status completion code 
**  
** Side Effects:  none
*/
NTSTATUS
INTELCAM_GetCameraProperty(
    PINTELCAM_DEVICE_CONTEXT pDeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG PropertyID = pSPD->Property->Id;     // index of the property
    PKSPROPERTY_VIDEOPROCAMP_S pData =
        (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;  // pointer to output data

    ASSERT(pSPD->PropertyOutputSize >= sizeof(KSPROPERTY_VIDEOPROCAMP_S));

    // Copy the input property info to the output property info
    RtlCopyMemory(pData, pSPD->Property, sizeof (KSPROPERTY_VIDEOPROCAMP_S));

    switch(PropertyID) {
        case  KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
            return (pSrb->Status = GetPropertyCtrl(REQ_BRIGHTNESS,pDeviceContext,pSrb));

        case  KSPROPERTY_VIDEOPROCAMP_SHARPNESS:
            return (pSrb->Status = GetPropertyCtrl(REQ_ENHANCEMENT,pDeviceContext,pSrb));

        case  KSPROPERTY_VIDEOPROCAMP_SATURATION:
            return (pSrb->Status = GetPropertyCtrl(REQ_SATURATION,pDeviceContext,pSrb));

        case  KSPROPERTY_VIDEOPROCAMP_CONTRAST:
            return (pSrb->Status = GetPropertyCtrl(REQ_EXPOSURE,pDeviceContext,pSrb));

        case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
            return (pSrb->Status = GetPropertyCtrl(REQ_WHITEBALANCE,pDeviceContext,pSrb));

        default:
            return STATUS_NOT_SUPPORTED;
    }
}

/*
** GetPropertyCtrl()
**
**  Retrieve current value of (brightness, contrast, saturation,sharpness,
**  whitebalance) from the camera , and stuff it in pSrb->CommandData.PropertyInfo
**  which is typecasted to PKSPROPERTY_VIDEOPROCAMP_S structure.
**
** Arguments:
**
**  pDC      - driver context
**  pSrb     - Stream Request Blaock Pointer
**
** Returns:
**
**  NT status completion code 
**  
** Side Effects:  none
*/
NTSTATUS
GetPropertyCtrl(
    IN REQUEST ReqID,
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PVOID Value;
    BOOLEAN Status;

    ASSERT(pDC);
    ASSERT(pSrb);

    INTELCAM_KdPrint(MAX_TRACE, ("enter GetPropertyCtrl\n"));
    Status = GetPropertyCurrent(pDC, &Value, ReqID);
    FormPropertyData(pSrb, (PVOID)&Value, ReqID);
    if( Status == TRUE )
        return (pSrb->Status = STATUS_SUCCESS);
    else
        return (pSrb->Status = STATUS_NOT_SUPPORTED);
}



/*
** INTELCAM_GetVideoControlProperty()
**
** Arguments:
**
**  DeviceContext - driver context
**
** Returns:
**
**  NT status completion code 
**  
** Side Effects:  none
*/
NTSTATUS
INTELCAM_GetVideoControlProperty(
    PINTELCAM_DEVICE_CONTEXT pDeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo; 
    ULONG PropertyID = pSPD->Property->Id;     // index of the property
    ULONG nS  = pSPD->PropertyOutputSize;        // size of data supplied


    INTELCAM_KdPrint(MAX_TRACE, ("enter GetVideoControlProperty\n"));

    switch(PropertyID) {

    case KSPROPERTY_VIDEOCONTROL_CAPS:
    {
        PKSPROPERTY_VIDEOCONTROL_CAPS_S pS = (PKSPROPERTY_VIDEOCONTROL_CAPS_S) pSPD->Property;    // pointer to the input data
        PKSPROPERTY_VIDEOCONTROL_CAPS_S pOutputData = (PKSPROPERTY_VIDEOCONTROL_CAPS_S) pSPD->PropertyInfo;

        ASSERT (pSPD->PropertyOutputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S));
      
        // ignore this request for video pin.
        if (pS->StreamIndex == STREAM_Still) {       
        	
			RtlZeroMemory(pOutputData, sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S));
        	
            pOutputData->VideoControlCaps = 
                  KS_VideoControlFlag_ExternalTriggerEnable
                | KS_VideoControlFlag_Trigger              
                ;
            pSrb->Status = STATUS_SUCCESS;
            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_CAPS_S);
        }
        else {
            ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
        }
    }
    break;
         
    case  KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE:
    {
        ULONG index, Numerator, Denominator;

        PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pInputData =
			(PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) pSPD->Property;  
        PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S pOutputData =
			(PKSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S) pSPD->PropertyInfo;  

        // ignore this request for still pin.
        if (pInputData->StreamIndex == STREAM_Still) {
            ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
            return ntStatus;
        }

        if (pDeviceContext->StreamOpen == TRUE)
		{
			RtlZeroMemory(pOutputData, sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S));
		
			pSrb->ActualBytesTransferred =
				sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S);

            index = pDeviceContext->CurrentProperty.RateIndex;               

			if (pDeviceContext->CurrentProperty.Format.lWidth == 160)
			{
                Numerator = 3;
                Denominator = 4;
			}
			else if (pDeviceContext->CurrentProperty.Format.lWidth == 320)
			{
                Numerator = 3;
                Denominator = 1;
			}
			else 
			{
                Numerator = 1;
                Denominator = 1;
			}
           
			pOutputData->CurrentActualFrameRate = 
                BusBWArray[index].QCIFFrameRate * Numerator / Denominator;

            if ( pDeviceContext->CurrentProperty.BusSaturation ) {
                pOutputData->CurrentMaxAvailableFrameRate = pOutputData->CurrentActualFrameRate;
            }
            else {
				pOutputData->CurrentMaxAvailableFrameRate =
								NUM_100NANOSEC_UNITS_PERFRAME(25)* Numerator / Denominator;
            }
			pSrb->Status = STATUS_SUCCESS;
		}
		else
		{
			ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
		}

		break;
    }
    
    case  KSPROPERTY_VIDEOCONTROL_FRAME_RATES:
    {
		ULONG Numerator, Denominator, i, j;

		PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S pData =
			(PKSPROPERTY_VIDEOCONTROL_FRAME_RATES_S) pSPD->Property;
	
		if (nS == 0)
		{
			// 7 below is the number of frame rates supported in all cases
			pSrb->ActualBytesTransferred =
					sizeof(KSMULTIPLE_ITEM) + ((FRAME_RATE_LIST_SIZE -1) * sizeof(LONGLONG));
			ntStatus = pSrb->Status = STATUS_BUFFER_OVERFLOW;
		}
		else if (nS >=
					sizeof(KSMULTIPLE_ITEM) + ((FRAME_RATE_LIST_SIZE -1) * sizeof(LONGLONG))) {
			//
			// assuming this has the Supported Frame rates Struct
			//
			if (pData->Dimensions.cx == 160)
			{
                Numerator = 3;
                Denominator = 4;
			}
			else if (pData->Dimensions.cx == 320)
			{
                Numerator = 3;
                Denominator = 1;
			}
			else 
			{
                Numerator = 1;
                Denominator = 1;
			}
			
			{
				PKSMULTIPLE_ITEM pOutputBuf =
					(PKSMULTIPLE_ITEM) pSPD->PropertyInfo;
				LONGLONG * pDataPtr = (LONGLONG*) (pOutputBuf + 1);
				
				// Copy the input property info to the output property info
				pOutputBuf->Size =  (FRAME_RATE_LIST_SIZE -1) * sizeof(LONGLONG);
				pOutputBuf->Count = FRAME_RATE_LIST_SIZE - 1;

				for (i = 0, j=1; i < pOutputBuf->Count; i++,j++){
					pDataPtr[i] = BusBWArray[j].QCIFFrameRate * Numerator / Denominator;
				}

			}
			
			
			pSrb->ActualBytesTransferred =
					sizeof(KSMULTIPLE_ITEM) + ((FRAME_RATE_LIST_SIZE - 1) * sizeof(LONGLONG));

		    pSrb->Status = STATUS_SUCCESS;
		}
		else
		{
			ntStatus = pSrb->Status = STATUS_BUFFER_TOO_SMALL;
		}
        break;
    }

    case KSPROPERTY_VIDEOCONTROL_MODE:
    {
        PKSPROPERTY_VIDEOCONTROL_MODE_S pOutputData  = (PKSPROPERTY_VIDEOCONTROL_MODE_S) pSPD->PropertyInfo;    // pointer to the data
		
		RtlZeroMemory(pOutputData, sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S));

        pOutputData->Mode = KS_VideoControlFlag_ExternalTriggerEnable | 
                           KS_VideoControlFlag_Trigger ;
		
        pSrb->Status = STATUS_SUCCESS;
        pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S);
    }

    break;

    default:
            ntStatus = pSrb->Status = STATUS_NOT_SUPPORTED;
    }
    return ntStatus;
}




















