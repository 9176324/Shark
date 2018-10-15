//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "AnlgAudioFormats.h"
#include "EuropaGuids.h"
#include "Mediums.h"


//////////////////////////////////////////////////////////////////////////////
//
// Description:
// This is the list of data ranges supported on the audio pin.
//
//////////////////////////////////////////////////////////////////////////////
const PKSDATARANGE AnlgAudioInPinDataRanges[] =
{
    (PKSDATARANGE) &PinAnalogIoRange
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
// This define describes the audio input pin.
// It is a collection of data like the pin medium, range, category, name etc.
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_AUDIO_IN_PIN_DESCRIPTOR                                    \
{                                                                       \
    NULL,                           /* Dispatch                     */  \
    NULL,                           /* AutomationTable              */  \
    {                                                                   \
        0,                          /* Interfaces count             */  \
        NULL,                       /* Interfaces                   */  \
        SIZEOF_ARRAY(AnlgXBarAudioOutMedium), /* Mediums count      */  \
        AnlgXBarAudioOutMedium,               /* Mediums            */  \
        SIZEOF_ARRAY(AnlgAudioInPinDataRanges), /* Range Count      */  \
        AnlgAudioInPinDataRanges,               /* Ranges           */  \
        KSPIN_DATAFLOW_IN,          /* Dataflow                     */  \
        KSPIN_COMMUNICATION_BOTH,   /* Communication                */  \
        &PINNAME_BDA_ANALOG_AUDIO,  /* Category                     */  \
        &VAMP_ANLG_AUDIO_IN_PIN,    /* Name                         */  \
        0                           /* Reserved                     */  \
    },                                                                  \
    0,                              /* Flags                        */  \
    1,                              /* Instances Possible           */  \
    0,                              /* Instances Necessary          */  \
    NULL,                           /* Allocator Framing            */  \
    NULL                            /* IntersectHandler             */  \
}
