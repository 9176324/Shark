/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    prpset.c

Abstract:

    code to set various properties of the USB camera 

Author:


Environment:

    kernel mode only

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
#include "prpftn.h"



/*
** INTELCAM_SetCameraProperty()
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
INTELCAM_SetCameraProperty(
    PINTELCAM_DEVICE_CONTEXT pDeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS status; 

	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG PropertyID = pSPD->Property->Id;     // index of the property

    switch (PropertyID) {
        case  KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS:
            status = SetPropertyCtrl(REQ_BRIGHTNESS,pDeviceContext,pSrb);
            break;
            
        case  KSPROPERTY_VIDEOPROCAMP_SHARPNESS:
            status = SetPropertyCtrl(REQ_ENHANCEMENT,pDeviceContext,pSrb);
            break;

        case  KSPROPERTY_VIDEOPROCAMP_SATURATION:
            status = SetPropertyCtrl(REQ_SATURATION,pDeviceContext,pSrb);
            break;

        case  KSPROPERTY_VIDEOPROCAMP_CONTRAST:
            status = SetPropertyCtrl(REQ_EXPOSURE,pDeviceContext,pSrb);
            break;

        case KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE:
            status = SetPropertyCtrl(REQ_WHITEBALANCE,pDeviceContext,pSrb);
            break;

        default:
            status = STATUS_NOT_SUPPORTED;
    }

    pSrb->Status = status;
    pSrb->ActualBytesTransferred = (status == STATUS_SUCCESS ? sizeof (KSPROPERTY_VIDEOPROCAMP_S) : 0);  
    
    return status;

}




/*
** SetPropertyCtrl()
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
SetPropertyCtrl(
    IN REQUEST ReqID,
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    PKSPROPERTY_VIDEOPROCAMP_S pData =
        (PKSPROPERTY_VIDEOPROCAMP_S) pSPD->PropertyInfo;  // pointer to input data

    USHORT RegAddr;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length;
    LONG Value;

    ASSERT(pDC);
    ASSERT(pSrb);
    ASSERT(pSPD);


    Value = pData->Value; 
    Length = 0;

    switch (ReqID)
    {
        case REQ_BRIGHTNESS:
            
            if ( (Value >= 0) && (Value <= MAX_BRIGHTNESS_IRE_UNITS) ) {
                pDC->CurrentProperty.Brightness = Value;
                RegAddr = INDEX_PREF_BRIGHTNESS;
                Value /= STEPPING_DELTA_BRIGHTNESS; // map to camera 
            }
            else {
                Status = STATUS_INVALID_PARAMETER;
	        }
            break;

        case REQ_ENHANCEMENT:
            if ( (Value >= 0) && (Value <= MAX_ENHANCEMENT_MISC_UNITS) ) {
                pDC->CurrentProperty.Sharpness = Value;
                RegAddr = INDEX_PREF_ENHANCEMENT;
                Value /= STEPPING_DELTA_ENHANCEMENT;  // map to camera 
            }
            else {
                Status = STATUS_INVALID_PARAMETER;
	        }
            break;

        case REQ_EXPOSURE:
            if ( (Value >= 0) && (Value <= MAX_CONTRAST_MISC_UNITS) ) {
                pDC->CurrentProperty.Contrast = Value;
                RegAddr = INDEX_PREF_EXPOSURE;
                Value /= STEPPING_DELTA_CONTRAST;  // map to camera 
            }
            else {
                Status = STATUS_INVALID_PARAMETER;
	        }
            break;

        case REQ_SATURATION:
            if ( (Value >= 0) && (Value <= MAX_SATURATION_MISC_UNITS) ) {
                pDC->CurrentProperty.Saturation = Value;
                RegAddr = INDEX_PREF_SATURATION;
                Value /= STEPPING_DELTA_SATURATION; // map to camera 
            }
            else {
                Status = STATUS_INVALID_PARAMETER;
	        }
            break;

        case REQ_WHITEBALANCE:
            if ( (Value >= 0) && (Value <= MAX_WHITEBALANCE_MISC_UNITS) ) {
                pDC->CurrentProperty.WhiteBalance = Value;
                RegAddr = INDEX_PREF_WHITEBALANCE;
                Value /= STEPPING_DELTA_WHITEBALANCE; // map to camera 
            }
            else {
                Status = STATUS_INVALID_PARAMETER;
	        }
            break;

        default:
            INTELCAM_KdPrint(MIN_TRACE,("SetPropertyCtrl: Invalid Property\n"));
            Status = STATUS_NOT_SUPPORTED;
   }

    //
    // Set camera to requested videoprocamp value...
    //

    if (NT_SUCCESS(Status) ) {
        Status = USBCAMD_ControlVendorCommand(pDC,
                                              REQUEST_SET_PREFERENCE,
                                              (USHORT)Value,
                                              RegAddr,
                                              NULL,
                                              &Length,
                                              FALSE,
										      NULL,
										      NULL);

        if(!NT_SUCCESS(Status)) {
            INTELCAM_KdPrint(MIN_TRACE, ("ERROR: Setting property <0x%x>",RegAddr));
        }
    }
    return Status;
}



/*
** INTELCAM_SetVideoControlProperty()
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
INTELCAM_SetVideoControlProperty(
    PINTELCAM_DEVICE_CONTEXT pDeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    ULONG ntStatus = STATUS_SUCCESS;
    PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG Id = pSPD->Property->Id;              // index of the property
    PKSPROPERTY_VIDEOCONTROL_MODE_S pS = (PKSPROPERTY_VIDEOCONTROL_MODE_S) pSPD->Property;

    ASSERT (pSPD->PropertyInputSize >= sizeof (KSPROPERTY_VIDEOCONTROL_MODE_S));
    
    switch (Id) {

        case KSPROPERTY_VIDEOCONTROL_MODE:
            // ignore this request for video pin.
            if (pS->StreamIndex == STREAM_Still) {
                if (( pS->Mode & KS_VideoControlFlag_Trigger) || 
                    (pS->Mode & KS_VideoControlFlag_ExternalTriggerEnable ) ) {
                        INTELCAM_KdPrint(MIN_TRACE,("Trigger Button Pressed\n"));
                        pSrb->Status = ntStatus;
                        // set the soft trigger button satatus to true.
                        pDeviceContext->SoftTrigger = TRUE;
                }
            }
    
        break;

        default:
   
              pSrb->Status = ntStatus = STATUS_NOT_SUPPORTED;
    }
    return ntStatus;
}

