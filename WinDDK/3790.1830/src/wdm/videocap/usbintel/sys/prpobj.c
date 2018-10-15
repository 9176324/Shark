/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   prpobj.c

Abstract:

   Code related to "getting"  about properties 

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

#include "warn.h"
#include "wdm.h"
#include <strmini.h>
#include <ksmedia.h>
#include "usbdi.h"
#include "usbcamdi.h"
#include "intelcam.h"
#include "prpobj.h"


extern const GUID PROPSETID_CUSTOM_PROP = 
    {0x3dc2e820, 0x4713, 0x11d2, {0xba, 0x41, 0x0, 0xa0, 0xc9, 0xd, 0x2b, 0x5}};


//
// Registry subky and values wide character strings.
//
WCHAR wszSettings[]     = L"Settings";
WCHAR wszBrightness[]   = L"Brightness";
WCHAR wszContrast[]     = L"Contrast";
WCHAR wszSaturation[]   = L"Saturation";
WCHAR wszSharpness[]    = L"Sharpness";
WCHAR wszWhiteBalance[] = L"WhiteBalance";


//
// First define all of the ranges and stepping values
//

// ------------------------------------------------------------------------
KSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] = 
{
    {
        STEPPING_DELTA_BRIGHTNESS,   // SteppingDelta
        0,                           // Reserved
        0,                           // Minimum in (IRE * 100) units
        MAX_BRIGHTNESS_IRE_UNITS     // Maximum in (IRE * 100) units
    }
};

const LONG BrightnessDefault = (5 * STEPPING_DELTA_BRIGHTNESS); 

KSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (BrightnessRangeAndStep),
            SIZEOF_ARRAY (BrightnessRangeAndStep),
            0
        },
		(PVOID) BrightnessRangeAndStep,
	},


    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (BrightnessDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BrightnessDefault
	}


};

KSPROPERTY_VALUES BrightnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BrightnessMembersList),
    BrightnessMembersList
};


////////////
////
////   SHARPNESS
////
////////////

KSPROPERTY_STEPPING_LONG SharpnessRangeAndStep [] = 
{
    {
        STEPPING_DELTA_ENHANCEMENT,       // SteppingDelta
        0,                                // Reserved
        0,                                // Minimum 
        MAX_ENHANCEMENT_MISC_UNITS        // Maximum 
    }
};

const LONG SharpnessDefault = (7 * STEPPING_DELTA_ENHANCEMENT); 

KSPROPERTY_MEMBERSLIST SharpnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (SharpnessRangeAndStep),
            SIZEOF_ARRAY (SharpnessRangeAndStep),
            0
        },
        (PVOID) SharpnessRangeAndStep
    },
	
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (SharpnessDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SharpnessDefault
	}

};

KSPROPERTY_VALUES SharpnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SharpnessMembersList),
    SharpnessMembersList
};


////////////
////
////   SATURATION
////
////////////

KSPROPERTY_STEPPING_LONG SaturationRangeAndStep [] = 
{
    {
        STEPPING_DELTA_SATURATION,        // SteppingDelta
        0,                                // Reserved
        0,                                // Minimum 
        MAX_SATURATION_MISC_UNITS         // Maximum 
    }
};

const LONG SaturationDefault = (7 * STEPPING_DELTA_SATURATION); 

KSPROPERTY_MEMBERSLIST SaturationMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (SaturationRangeAndStep),
            SIZEOF_ARRAY (SaturationRangeAndStep),
            0
        },
        (PVOID) SaturationRangeAndStep
    },

    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (SaturationDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SaturationDefault
	}

};

KSPROPERTY_VALUES SaturationValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SaturationMembersList),
    SaturationMembersList
};


////////////
////
////   CONTRAST (EXPOSURE_COMPENSATION )
////
////////////

KSPROPERTY_STEPPING_LONG ContrastRangeAndStep [] = 
{
    {
        STEPPING_DELTA_CONTRAST,          // SteppingDelta
        0,                                // Reserved
        0,                                // Minimum 
        MAX_CONTRAST_MISC_UNITS           // Maximum 
    }
};

const LONG ContrastDefault = (4 * STEPPING_DELTA_CONTRAST); 

KSPROPERTY_MEMBERSLIST ContrastMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (ContrastRangeAndStep),
            SIZEOF_ARRAY (ContrastRangeAndStep),
            0
        },
        (PVOID) ContrastRangeAndStep
    },    

    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (ContrastDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ContrastDefault
	}

};

KSPROPERTY_VALUES ContrastValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ContrastMembersList),
    ContrastMembersList
};


////////////
////
////   WHITEBALANCE
////
////////////

KSPROPERTY_STEPPING_LONG WhiteBalanceRangeAndStep [] = 
{
    {
        STEPPING_DELTA_WHITEBALANCE,    // SteppingDelta
        0,                              // Reserved
        0,                              // Minimum 
        MAX_WHITEBALANCE_MISC_UNITS     // Maximum 
    }
};

const LONG WhiteBalanceDefault = (23 * STEPPING_DELTA_WHITEBALANCE); 

KSPROPERTY_MEMBERSLIST WhiteBalanceMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (WhiteBalanceRangeAndStep),
            SIZEOF_ARRAY (WhiteBalanceRangeAndStep),
            0
        },
        (PVOID) WhiteBalanceRangeAndStep
    },    

    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WhiteBalanceDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WhiteBalanceDefault
	}

};

KSPROPERTY_VALUES WhiteBalanceValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (WhiteBalanceMembersList),
    WhiteBalanceMembersList
};

// ------------------------------------------------------------------------
DEFINE_KSPROPERTY_TABLE(VideoProcAmpProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &BrightnessValues,                      // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_SHARPNESS,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &SharpnessValues,                     // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_SATURATION,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &SaturationValues,                      // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),
    
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_CONTRAST,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &ContrastValues,                        // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),      // MinData
        TRUE,                                   // SetSupported or Handler
        &WhiteBalanceValues,                    // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    )
};



DEFINE_KSPROPERTY_TABLE(CustomProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CUSTOM_PROP_FIRMWARE_VER,    // PropertyId
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_CUSTOM_PROP_S),       // MinProperty
        sizeof(KSPROPERTY_CUSTOM_PROP_S),       // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        sizeof(ULONG)                           // SerializedSize
    )
};
    


DEFINE_KSPROPERTY_TABLE(FrameRateProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_CAPS,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S), // MinProperty
        sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S), // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        0                                       // SerializedSize
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),      // MinProperty
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),      // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        0                                       // SerializedSize
    ),
	
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_FRAME_RATES,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_FRAME_RATES_S),    // MinProperty
        0 ,                                     // MinData
        FALSE,                                  // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        0                                       // SerializedSize
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_MODE,
        TRUE,                                   // GetSupported or Handler
        sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S), // MinProperty
        sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S), // MinData
        TRUE,                                   // SetSupported or Handler
        NULL,                                   // Values
        0,                                      // RelationsCount
        NULL,                                   // Relations
        NULL,                                   // SupportHandler
        0                                       // SerializedSize
    ),

};
    

//
// All of the property sets supported by the adapter
//

DEFINE_KSPROPERTY_SET_TABLE(AdapterPropertyTable)
{

   DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_CUSTOM_PROP,                   // Set
        SIZEOF_ARRAY(CustomProperties),           // PropertiesCount
        CustomProperties,                         // PropertyItem
        0,                                        // FastIoCount
        NULL                                      // FastIoTable
    ),
   
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOPROCAMP,                 // Set
        SIZEOF_ARRAY(VideoProcAmpProperties),           // PropertiesCount
        VideoProcAmpProperties,                         // PropertyItem
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOCONTROL,           // Set
        SIZEOF_ARRAY(FrameRateProperties),        // PropertiesCount
        FrameRateProperties,                      // PropertyItem
        0,                                        // FastIoCount
        NULL                                      // FastIoTable
    )

};


#define NUMBER_OF_ADAPTER_PROPERTY_SETS (SIZEOF_ARRAY (AdapterPropertyTable))


//
// Format/resolution combinations.  The first resolution under
// each format is the default.
//
static const FORMAT _Format[] =
{
    { FCC_FORMAT_YUV12N, 176,144 },
    { FCC_FORMAT_YUV12N, 320,240 },
    {(FOURCC)-1, -1,-1 }
};

//
// Frame rates.
//
static const ULONG _Rate[] =
{
    1000000/25,                     // Default
    0,                              // Zero
    1000000/6,
    1000000/10,
    1000000/12,
    1000000/15,
    1000000/18,
    1000000/20,
    (ULONG)-1
};


/*
** INTELCAM_GetStreamPropertiesArray()
**
** Returns the stream class defined properties fror the camera
**
**  see stream class documentation
**
** Arguments:
**
** DeviceContext - camera specific context
**
** NumberOfArrayEntries - pointer to ulong , should be filled
**      in with the number of elements in the array returned.
**
** Returns:
**
** Side Effects:  none
*/

PVOID   
INTELCAM_GetAdapterPropertyTable(
    PULONG NumberOfArrayEntries
    )
{

    INTELCAM_KdPrint (MAX_TRACE, ("GetStreamFormatsArray\n")); 
    
    *NumberOfArrayEntries = NUMBER_OF_ADAPTER_PROPERTY_SETS;
    
    return (PVOID) AdapterPropertyTable;
}




NTSTATUS
FormPropertyData(
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb,
    IN PVOID pData,
	IN REQUEST ReqID
    )
{

	PSTREAM_PROPERTY_DESCRIPTOR pSPD = pSrb->CommandData.PropertyInfo;
    
    switch( ReqID )
	{
        case REQ_BRIGHTNESS:
        case REQ_WHITEBALANCE:
        case REQ_ENHANCEMENT:
        case REQ_EXPOSURE:
        case REQ_SATURATION:
		{
            PKSPROPERTY_VIDEOPROCAMP_S pPropertyInfo =
                (PKSPROPERTY_VIDEOPROCAMP_S)pSPD->PropertyInfo;

            pSrb->ActualBytesTransferred = sizeof (KSPROPERTY_VIDEOPROCAMP_S);

			RtlZeroMemory(pPropertyInfo, sizeof(KSPROPERTY_VIDEOPROCAMP_S));

			pPropertyInfo->Flags = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
			pPropertyInfo->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL; // no Auto mode

			if(pData)
				pPropertyInfo->Value = *(WORD *)pData;
			else
				return STATUS_NO_MEMORY;

			return STATUS_SUCCESS;
		}

		default:
		   return STATUS_NOT_SUPPORTED;
	}

}


/*
** GetPropertyCurrent()
**
** Arguments:
**
**  pDC - driver context
**
** Returns:
**
**  BOOLEAN 
**  
** Side Effects:  none
*/
BOOLEAN
GetPropertyCurrent(
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    OUT PVOID pProperty,
    IN REQUEST ReqID
    )
/*++
    Routine Description:
    Arguments:
    Return Value:
--*/
{
    NTSTATUS Status = STATUS_NOT_SUPPORTED;

    ASSERT(pDC);
    ASSERT(pProperty);         

    switch ( ReqID )
    {
        case REQ_BRIGHTNESS:
            *((LONG*)pProperty) = pDC->CurrentProperty.Brightness;
            break;
        
        case REQ_ENHANCEMENT:
            *((LONG*)pProperty) = pDC->CurrentProperty.Sharpness;
            break;

        case REQ_EXPOSURE:
            *((LONG*)pProperty) = pDC->CurrentProperty.Contrast;
            break;

        case REQ_SATURATION:
            *((LONG*)pProperty) = pDC->CurrentProperty.Saturation;
            break;

        case REQ_WHITEBALANCE:
            *((LONG*)pProperty) = pDC->CurrentProperty.WhiteBalance;
            break;
        INTELCAM_KdPrint(MAX_TRACE, ("PROP: Getting Curr property - %d\n",  *((ULONG*)pProperty) ));
    }

    return TRUE;

}


NTSTATUS 
CreateRegistryKeySingle(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    NTSTATUS status;
    UNICODE_STRING ustr;
    OBJECT_ATTRIBUTES objectAttributes;

    RtlInitUnicodeString(&ustr, pwszSection);
	   InitializeObjectAttributes(
		      &objectAttributes,
		      &ustr,
		      OBJ_CASE_INSENSITIVE,
		      hKey,
		      NULL
		      );

    status = 
	       ZwCreateKey(
		          phKeySection,
		          desiredAccess,
		          &objectAttributes,
		          0,
		          NULL,				            /* optional*/
		          REG_OPTION_NON_VOLATILE,
		          NULL
		          );         

    return status;
}



NTSTATUS 
CreateRegistrySubKey(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    )
{
    UNICODE_STRING ustr;
    USHORT usPos = 1;             // Skip first backslash
    static WCHAR wSep = '\\';
    NTSTATUS status = STATUS_SUCCESS;

    RtlInitUnicodeString(&ustr, pwszSection);

    while(usPos < ustr.Length) {
        if(ustr.Buffer[usPos] == wSep) {

            // NULL terminate our partial string
            ustr.Buffer[usPos] = UNICODE_NULL;
            status = 
                CreateRegistryKeySingle(
                    hKey,
                    desiredAccess,
                    ustr.Buffer,
                    phKeySection
                    );
            ustr.Buffer[usPos] = wSep;

            if(NT_SUCCESS(status)) {
                ZwClose(*phKeySection);
            } else {
                break;
            }
        }
        usPos++;
    }

    // Create the full key
    if(NT_SUCCESS(status)) {
        status = 
            CreateRegistryKeySingle(
                 hKey,
                 desiredAccess,
                 ustr.Buffer,
                 phKeySection
                 );
    }

    return status;
}


NTSTATUS 
GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN PULONG DataLength
    )

/*++

Routine Description:
    
    This routine gets the specified value out of the registry

Arguments:

    Handle - Handle to location in registry

    KeyNameString - registry key we're looking for

    KeyNameStringLength - length of registry key we're looking for

    Data - where to return the data

    DataLength - how big the data is

Return Value:

    status is returned from ZwQueryValueKey

--*/

{
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;


    RtlInitUnicodeString(&keyName, KeyNameString);
    
    length = sizeof(KEY_VALUE_FULL_INFORMATION) + 
            KeyNameStringLength + *DataLength;
            
    fullInfo = ExAllocatePool(PagedPool, length); 
     
    if (fullInfo) { 
       
        status = ZwQueryValueKey(
                    Handle,
                   &keyName,
                    KeyValueFullInformation,
                    fullInfo,
                    length,
                   &length
                    );
                        
        if (NT_SUCCESS(status)){

            if (fullInfo->DataLength == *DataLength) {

                RtlCopyMemory(
                    Data,
                    ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                    fullInfo->DataLength
                    );
            }
            else {

                status = STATUS_INVALID_BUFFER_SIZE;
            }
        }            

        ExFreePool(fullInfo);

    }        
    
    return (status);

}



NTSTATUS
SetRegistryKeyValue(
   HANDLE hKey,
   PWCHAR pwszEntry, 
   LONG nValue
   )
{
    NTSTATUS status;
    UNICODE_STRING ustr;

    RtlInitUnicodeString(&ustr, pwszEntry);

    status =	      
        ZwSetValueKey(
		          hKey,
		          &ustr,
		          0,			/* optional */
		          REG_DWORD,
		          &nValue,
		          sizeof(nValue)
		          );         

   return status;
}


BOOL
GetPropertyValuesFromRegistry(
    PVOID pDC
    )
{
    NTSTATUS Status;
    HANDLE hPDOKey, hKeySettings;
    ULONG ulLength; 
    
    PINTELCAM_DEVICE_CONTEXT pDevExt = pDC;
    ulLength = sizeof(LONG);

    INTELCAM_KdPrint(MAX_TRACE, ("GetPropertyValuesFromRegistry \n"));

    //
    // Registry key: 
    //   Windows 2000:
    //   HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\
    //   {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x
    //
    // Win98:
    //    HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Class\Image\000x
    // 
    Status = 
        IoOpenDeviceRegistryKey(
            pDevExt->pPnPDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_READ, 
            &hPDOKey);

    ASSERT(Status == STATUS_SUCCESS);

    //
    // loop through our table of strings,
    // reading the registry for each.
    //
    if(NT_SUCCESS(Status)) {

        // Create or open the settings key
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

            // Brightness
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszBrightness, 
                sizeof(wszBrightness), 
                (PVOID) &pDevExt->CurrentProperty.Brightness, 
                &ulLength);

            INTELCAM_KdPrint(MAX_TRACE, ("Brightness Reg. Value = %d \n",
                                            pDevExt->CurrentProperty.Brightness));

            // Contrast
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszContrast, 
                sizeof(wszContrast), 
                (PVOID) &pDevExt->CurrentProperty.Contrast, 
                &ulLength);

            INTELCAM_KdPrint(MAX_TRACE, ("Contrast Reg. Value = %d \n",
                                            pDevExt->CurrentProperty.Contrast));

            // Saturation
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszSaturation, 
                sizeof(wszSaturation), 
                (PVOID) &pDevExt->CurrentProperty.Saturation, 
                &ulLength);

            INTELCAM_KdPrint(MAX_TRACE, ("Saturation Reg. Value = %d \n",
                                            pDevExt->CurrentProperty.Saturation));

            // Sharpness
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszSharpness, 
                sizeof(wszSharpness), 
                (PVOID) &pDevExt->CurrentProperty.Sharpness, 
                &ulLength);

            INTELCAM_KdPrint(MAX_TRACE, ("Sharpness Reg. Value = %d \n",
                                            pDevExt->CurrentProperty.Sharpness));

            // WhiteBalance
            Status = GetRegistryKeyValue(
                hKeySettings, 
                wszWhiteBalance, 
                sizeof(wszWhiteBalance), 
                (PVOID) &pDevExt->CurrentProperty.WhiteBalance, 
                &ulLength);

            INTELCAM_KdPrint(MAX_TRACE, ("WhiteBalance Reg. Value = %d \n",
                                            pDevExt->CurrentProperty.WhiteBalance));

            // close settings subkey and device key.

            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {

            INTELCAM_KdPrint(MAX_TRACE, ("CreateRegistrySubKey failed with Status=%x\n", Status));
        }

        ZwClose(hPDOKey);

    } else {

        INTELCAM_KdPrint(MIN_TRACE, ("IoOpenDeviceRegistryKey failed with Status=%x\n", Status));

    }

    // Not implemented so always return FALSE to use the defaults.
    return FALSE;
}

//---------------------------------------------------------------------------
// INTELCAM_CameraToDriverDefaults
//---------------------------------------------------------------------------
NTSTATUS
INTELCAM_CameraToDriverDefaults(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID pDeviceContext
    )
/*++

Routine Description:

Arguments:

    DeviceContext - points to the driver specific DeviceContext


Return Value:

    NT status code

--*/
{
    PINTELCAM_DEVICE_CONTEXT pDC = pDeviceContext;
    HW_STREAM_REQUEST_BLOCK Srb;
    STREAM_PROPERTY_DESCRIPTOR PropertyDescriptor;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOL status;
    KSPROPERTY_VIDEOPROCAMP_S Video; 

    //
    //  Reset videoprocamp values in dev. extension.
    // 
    
    pDC->CurrentProperty.Brightness     = -1;
    pDC->CurrentProperty.Sharpness      = -1;
    pDC->CurrentProperty.Contrast       = -1;
    pDC->CurrentProperty.WhiteBalance   = -1;
    pDC->CurrentProperty.Saturation     = -1;

    //
    // Get the actual values for the controls from registry.
    //

    status = GetPropertyValuesFromRegistry(pDC);

    //
	// Fill in Property Descriptor field
	//
    Srb.CommandData.PropertyInfo = &PropertyDescriptor;
	PropertyDescriptor.Property = &Video.Property;
	PropertyDescriptor.PropertyInfo = &Video;
	PropertyDescriptor.PropertyInputSize = sizeof(KSPROPERTY_VIDEOPROCAMP_S);
	PropertyDescriptor.PropertyOutputSize = sizeof(KSPROPERTY_VIDEOPROCAMP_S);
	
	Video.Property.Set = PROPSETID_VIDCAP_VIDEOPROCAMP;

    // set brightness
	Video.Property.Id = KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS;
    if ( (pDC->CurrentProperty.Brightness >= 0) &&  
        (pDC->CurrentProperty.Brightness <= MAX_BRIGHTNESS_IRE_UNITS) ) {
        Video.Value = pDC->CurrentProperty.Brightness;
    }
    else {
        Video.Value = BrightnessDefault;
	}
    // now set the camera to this value.
	SetPropertyCtrl(REQ_BRIGHTNESS, pDC, &Srb);

    // set White Balance
	Video.Property.Id = KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE;
    if ( (pDC->CurrentProperty.WhiteBalance >= 0) &&  
        (pDC->CurrentProperty.WhiteBalance <= MAX_WHITEBALANCE_MISC_UNITS ) ) {
        Video.Value = pDC->CurrentProperty.WhiteBalance;
    }
    else {
        Video.Value = WhiteBalanceDefault;
	}
    // now set the camera to this value.
	SetPropertyCtrl(REQ_WHITEBALANCE, pDC, &Srb);

    // set Contrast
	Video.Property.Id = KSPROPERTY_VIDEOPROCAMP_CONTRAST;
    if ( (pDC->CurrentProperty.Contrast >= 0) &&  
        (pDC->CurrentProperty.Contrast <= MAX_CONTRAST_MISC_UNITS ) ) {
        Video.Value = pDC->CurrentProperty.Contrast;
    }
    else {
        Video.Value =  ContrastDefault;
	}
    // now set the camera to this value.
	SetPropertyCtrl(REQ_EXPOSURE, pDC, &Srb);

    // set Saturation
	Video.Property.Id = KSPROPERTY_VIDEOPROCAMP_SATURATION;
    if ( (pDC->CurrentProperty.Saturation >= 0) &&  
        (pDC->CurrentProperty.Saturation <= MAX_SATURATION_MISC_UNITS ) ) {
        Video.Value = pDC->CurrentProperty.Saturation;
    }
    else {
        Video.Value = SaturationDefault;
	}
    // now set the camera to this value.
	SetPropertyCtrl(REQ_SATURATION, pDC, &Srb);

    // set Sharpness
	Video.Property.Id = KSPROPERTY_VIDEOPROCAMP_SHARPNESS;
    if ( (pDC->CurrentProperty.Sharpness >= 0) &&  
        (pDC->CurrentProperty.Sharpness <= MAX_ENHANCEMENT_MISC_UNITS  ) ) {
        Video.Value = pDC->CurrentProperty.Sharpness;
    }
    else {
        Video.Value = SharpnessDefault;
	}
    // now set the camera to this value.
	SetPropertyCtrl(REQ_ENHANCEMENT, pDC, &Srb);

    return ntStatus = STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
// INTELCAM_SaveControlsToRegistry
//---------------------------------------------------------------------------
NTSTATUS
INTELCAM_SaveControlsToRegistry(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID pDeviceContext
    )
/*++

Routine Description:

    This function saves the camera controls - brightness,
	WhiteBalance, Saturation, Sharpness, and Contrast values
	to the registry.  These will be read by the driver at
	startchannel time and restored.

Arguments:

Return Value:

    NT status code

--*/
{
    PINTELCAM_DEVICE_CONTEXT pDC = pDeviceContext;
    NTSTATUS Status ;
    HANDLE hPDOKey, hKeySettings;

    INTELCAM_KdPrint(MAX_TRACE, ("SetPropertyValuesToRegistry \n"));

    //
    // Registry key: 
    //   HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\
    //   {6BDD1FC6-810F-11D0-BEC7-08002BE2092F\000x
    //
    Status = 
        IoOpenDeviceRegistryKey(
            pDC->pPnPDeviceObject, 
            PLUGPLAY_REGKEY_DRIVER,
            STANDARD_RIGHTS_WRITE, 
            &hPDOKey);

    ASSERT(Status == STATUS_SUCCESS);

    //
    // loop through our table of strings,
    // reading the registry for each.
    //
    if(NT_SUCCESS(Status)) {

        // Create or open the settings key
        Status =         
            CreateRegistrySubKey(
                hPDOKey,
                KEY_ALL_ACCESS,
                wszSettings,
                &hKeySettings
                );

        if(NT_SUCCESS(Status)) {

            // Brightness
            Status = SetRegistryKeyValue(hKeySettings,wszBrightness,
                                         pDC->CurrentProperty.Brightness);
            INTELCAM_KdPrint(MAX_TRACE, ("Set Brightness Registry Value to %d \n",
                                            pDC->CurrentProperty.Brightness));
            // Contrast
            Status = SetRegistryKeyValue(hKeySettings,wszContrast,
                                         pDC->CurrentProperty.Contrast);
            INTELCAM_KdPrint(MAX_TRACE, ("Set Contrast Registry Value to %d \n",
                                            pDC->CurrentProperty.Contrast));

            // Saturation
            Status = SetRegistryKeyValue(hKeySettings,wszSaturation,
                                         pDC->CurrentProperty.Saturation);
            INTELCAM_KdPrint(MAX_TRACE, ("Set Saturation Registry Value to %d \n",
                                            pDC->CurrentProperty.Saturation));

            // Sharpness
            Status = SetRegistryKeyValue(hKeySettings,wszSharpness,
                                         pDC->CurrentProperty.Sharpness);
            INTELCAM_KdPrint(MAX_TRACE, ("Set Sharpness Registry Value to %d \n",
                                            pDC->CurrentProperty.Sharpness));

            // WhiteBalance
            Status = SetRegistryKeyValue(hKeySettings,wszWhiteBalance,
                                         pDC->CurrentProperty.WhiteBalance);
            INTELCAM_KdPrint(MAX_TRACE, ("Set WhiteBalance Registry Value to %d \n",
                                            pDC->CurrentProperty.WhiteBalance));

            // save settings subkey and device key.

            ZwClose(hKeySettings);
            ZwClose(hPDOKey);

            return TRUE;

        } else {
            INTELCAM_KdPrint(MIN_TRACE, ("CreateRegistrySubKey failed with Status=%x\n", Status));
        }

        ZwClose(hPDOKey);

    } else {
        INTELCAM_KdPrint(MIN_TRACE, ("IoOpenDeviceRegistryKey failed with Status=%x\n", Status));
    }

    return Status;
}










