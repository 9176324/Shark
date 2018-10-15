/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   prpobj.h

Abstract:


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

#ifndef __PRPOBJ_H__
#define __PRPOBJ_H__

#if defined( mmioFOURCC )
    #undef mmioFOURCC
#endif // mmioFOURCC

#include "windef.h"
#include "mmsystem.h"


#define MAX_BRIGHTNESS_IRE_UNITS   108      // approx. measured value on UCAM1
#define STEPS_BRIGHTNESS 10 
#define STEPPING_DELTA_BRIGHTNESS    \
           ((MAX_BRIGHTNESS_IRE_UNITS+2)/STEPS_BRIGHTNESS)

#define MAX_ENHANCEMENT_MISC_UNITS  108     // arbitrary 
#define STEPS_ENHANCEMENT 10 
#define STEPPING_DELTA_ENHANCEMENT    \
           ((MAX_ENHANCEMENT_MISC_UNITS+2)/STEPS_ENHANCEMENT)   // Sharpness

#define MAX_SATURATION_MISC_UNITS   108     // check??
#define STEPS_SATURATION 10 
#define STEPPING_DELTA_SATURATION    \
           ((MAX_SATURATION_MISC_UNITS+2)/STEPS_SATURATION)

#define MAX_CONTRAST_MISC_UNITS  108        // arbitrary
#define STEPS_CONTRAST 10 
#define STEPPING_DELTA_CONTRAST    \
           ((MAX_CONTRAST_MISC_UNITS+2)/STEPS_CONTRAST)


#define MAX_WHITEBALANCE_MISC_UNITS 372     // arbitrary
#define STEPS_WHITEBALANCE 32 
#define STEPPING_DELTA_WHITEBALANCE    \
           ((MAX_WHITEBALANCE_MISC_UNITS)/STEPS_WHITEBALANCE)

//
//  Custom properties
//
typedef enum _REQUEST_CUSTOM {
    FIRMWARE_VERSION = 3,
} REQUEST_CUSTOM;


typedef enum {
    KSPROPERTY_CUSTOM_PROP_FIRMWARE_VER            // R O
} KSPROPERTY_CUSTOM_PROP;


typedef struct {
    KSPROPERTY Property;
    ULONG   Value;                       // Value to set or get
} KSPROPERTY_CUSTOM_PROP_S, *PKSPROPERTY_CUSTOM_PROP_S;

//
// Format four character codes.
//
#define FCC_FORMAT_YUV12N mmioFOURCC('I','Y','U','V')
#define FCC_FORMAT_YUV12A mmioFOURCC('I','4','2','0')

//
// Format.
//
typedef struct _FORMAT {
    FOURCC Fcc;
    LONG lWidth;
    LONG lHeight;
} FORMAT;

//
// Control request IDs.
//
typedef enum _REQUEST {
    REQ_BRIGHTNESS      = 1,
    REQ_DIB             = 2,
    REQ_ENHANCEMENT     = 3,
    REQ_EXPOSURE        = 4,
    REQ_FORMAT          = 5,
    REQ_PAN             = 6,
    REQ_RATE            = 7,
    REQ_SATURATION      = 8,
    REQ_SCALE           = 9,
    REQ_SEEK            = 10,
    REQ_WHITEBALANCE    = 11
} REQUEST;


//
// Defines all modifiable properties.
//
typedef struct _USBCAMD_PROPERTY {
    FORMAT Format;          // Current format
    LONG RateIndex;         // Current frame rate index into BusBWArray
    BOOLEAN BusSaturation;  // Set this flag when requested BW exceed avail BW.
    
    // store current VideoProcAmp vlaues here.
    LONG Brightness;
    LONG Contrast;
    LONG Saturation;
    LONG Sharpness;
    LONG WhiteBalance;

} USBCAMD_PROPERTY;



#endif

