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

#include "EuropaGuids.h"
#include "Mediums.h"

NTSTATUS AnlgVideoInPinProcess(IN PKSPIN pKSPin);
NTSTATUS SaveDetectedStandardToRegistry(
                            IN PKSPIN pKSPin,
                            IN PKS_TVTUNER_CHANGE_INFO pChannelChangeInfo );

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the video in pin.
//  The video input pin is not a real streaming pin, nevertheless the process
//  function gets called for the so called channel info structure to inform
//  the vbi pin about a channel change (see process function for more
//  details).
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DISPATCH AnlgVideoInPinDispatch =
{
    NULL,                   // Create
    NULL,                   // Remove
    AnlgVideoInPinProcess,  // Process
    NULL,                   // pKSPin Reset
    NULL,                   // pKSPin Set Data Format
    NULL,                   // SetDeviceState
    NULL,                   // pKSPin Connect
    NULL,                   // pKSPin Disconnect
    NULL,                   // Clock Dispatch
    NULL                    // Allocator Dispatch
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the decoder input format of the video pin.
//
//////////////////////////////////////////////////////////////////////////////
const KS_DATARANGE_ANALOGVIDEO DecoderInputFormat=
{
    {
        sizeof (KS_DATARANGE_ANALOGVIDEO),      // FormatSize
        0,                                      // Flags
        0,                                      // SampleSize
        0,                                      // Reserved

        STATIC_KSDATAFORMAT_TYPE_ANALOGVIDEO,   // aka MEDIATYPE_AnalogVideo
        STATIC_KSDATAFORMAT_SUBTYPE_NONE,       // ie. Wildcard
        STATIC_KSDATAFORMAT_SPECIFIER_ANALOGVIDEO, // aka FORMAT_AnalogVideo
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the list of data ranges supported on the video pin.
//
//////////////////////////////////////////////////////////////////////////////
const PKSDATARANGE AnlgVideoInPinDataRanges[] =
{
    (PKSDATARANGE)&DecoderInputFormat
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This define describes the video input pin.
//  It is a collection of data like the pin medium, range, category, name etc.
//
// Remarks:
//  Instances Necessary must be "1" because the audio pin need to know
//  if a TV channel change occurs. The new detected standard will be
//  written to the registry. The channel change info is being proccessed
//  by this pin. Also, it forces the graph builder to pull in the crossbar
//  and TV Tuner filters with applications like AMCap.
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_VIDEO_IN_PIN_DESCRIPTOR                                    \
{                                                                       \
    &AnlgVideoInPinDispatch,        /* Dispatch                     */  \
    NULL,                           /* AutomationTable              */  \
    {                                                                   \
        0,                          /* Interfaces count             */  \
        NULL,                       /* Interfaces                   */  \
        SIZEOF_ARRAY(AnlgXBarVideoOutMedium),/* Mediums count       */  \
        AnlgXBarVideoOutMedium,              /* Mediums             */  \
        SIZEOF_ARRAY(AnlgVideoInPinDataRanges),   /* Range Count    */  \
        AnlgVideoInPinDataRanges,                 /* Ranges         */  \
        KSPIN_DATAFLOW_IN,          /* Dataflow                     */  \
        KSPIN_COMMUNICATION_BOTH,   /* Communication                */  \
        &PINNAME_VIDEO_ANALOGVIDEOIN, /* Category                   */  \
        &PINNAME_VIDEO_ANALOGVIDEOIN, /* Name                       */  \
        0                           /* Reserved                     */  \
    },                                                                  \
    KSPIN_FLAG_USE_STANDARD_TRANSPORT,/* Flag for channel change info!*/\
    1,                              /* Instances Possible           */  \
    1,                              /* Instances Necessary          */  \
    NULL,                           /* Allocator Framing            */  \
    NULL                            /* IntersectHandler             */  \
}

