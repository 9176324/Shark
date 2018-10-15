/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   prpmanf.c

Abstract:

   Code related to "custom"  properties 

Author:

Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.

Environment:

   Kernel mode only


Revision History:

--*/

/*++
    Process this file for a manufacturer's build only.  Otherwise for
    regular builds skip this.
    Contains functions to support properties needed by the camera
    manufacturer.

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




/*
** INTELCAM_GetCustomProperty()
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
INTELCAM_GetCustomProperty(
    PINTELCAM_DEVICE_CONTEXT DeviceContext,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
    
	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG PropertyID = pSPD->Property->Id;     // index of the property

    switch(PropertyID) {
        case  KSPROPERTY_CUSTOM_PROP_FIRMWARE_VER:
            return (pSrb->Status = GetDataFromCamera(FIRMWARE_VERSION,
                                                     DeviceContext,
                                                     pSrb,
                                                     0));
        default:
	    return STATUS_NOT_SUPPORTED;

    }
	
    return ntStatus;
}


//---------------------------------------------------------------------------
// GetDataFromCamera
//---------------------------------------------------------------------------
NTSTATUS
GetDataFromCamera(
    IN REQUEST_CUSTOM PropertyID,
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    PHW_STREAM_REQUEST_BLOCK pSrb,
    IN ULONG DefOrCurr
    )
/*++
    Routine Description:
    Arguments:
    Return Value:
--*/
{

    ULONG Value[4] = {0};
    USHORT Index = 0;
    ULONG Length;
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

  
    switch(PropertyID) {
        case FIRMWARE_VERSION:
            Index = INDEX_PREF_FWARE_VER;
            Length = sizeof(WORD);
            break;

        default:
            return STATUS_NOT_SUPPORTED;
    }


    ntStatus = USBCAMD_ControlVendorCommand(pDC,
                                     REQUEST_GET_PREFERENCE,
                                     0,
                                     Index,
                                     Value,
                                     &Length,
                                     TRUE,
									 NULL,
									 NULL);

    FormCustomPropertyData(pSrb, (PVOID)Value, PropertyID);

    return ntStatus;
}



/*
** INTELCAM_SetCustomProperty()
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
INTELCAM_SetCustomProperty(
    PINTELCAM_DEVICE_CONTEXT pDC,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    ULONG PropertyID = pSPD->Property->Id;     // index of the property

    switch(PropertyID) {
        case  KSPROPERTY_CUSTOM_PROP_FIRMWARE_VER:
			return STATUS_NOT_SUPPORTED;

        default:
	    return STATUS_NOT_SUPPORTED;

    }

    return ntStatus;	
}


NTSTATUS
FormCustomPropertyData(
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PVOID pData,
	IN REQUEST_CUSTOM ReqID
    )
{

	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    
    switch( ReqID )
	{
        case FIRMWARE_VERSION:
		{
            PKSPROPERTY_CUSTOM_PROP_S pCustomProp =
                (PKSPROPERTY_CUSTOM_PROP_S)pSPD->PropertyInfo;

            pSrb->ActualBytesTransferred = sizeof(KSPROPERTY_CUSTOM_PROP_S);

			RtlZeroMemory(pCustomProp, sizeof(KSPROPERTY_CUSTOM_PROP_S));

            if(pData)
				pCustomProp->Value = *(WORD *)pData;
			else
				return STATUS_NO_MEMORY;

			return STATUS_SUCCESS;
		}

		default:
		   return STATUS_NOT_SUPPORTED;
	}

}



