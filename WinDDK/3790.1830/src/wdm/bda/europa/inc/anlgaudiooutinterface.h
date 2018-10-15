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
#include "BasePinInterface.h"

//
// Description:
//  Number of AUDIO stream buffers (size of the queue).
//
#define NUM_AUDIO_STREAM_BUFFER 5

//////////////////////////////////////////////////////////////////////////////
//
//  C-Function prototypes (due to MS interface we still need C-Functions)
//  Implemented in "AnlgVbiOutInterface.cpp"
//
//////////////////////////////////////////////////////////////////////////////

NTSTATUS AnlgAudioOutPinSetDataFormat
(
    IN PKSPIN pKSPin,
    IN OPTIONAL PKSDATAFORMAT pKSOldFormat,
    IN OPTIONAL PKSMULTIPLE_ITEM pKSOldAttributeList,
    IN const KSDATARANGE* pKSDataRange,
    IN OPTIONAL const KSATTRIBUTE_LIST* pKSAttributeRange
);

NTSTATUS AudioIntersectDataFormat
(
    IN PKSFILTER pKSFilter,
    IN PIRP pIrp,
    IN PKSP_PIN pPinInstance,
    IN PKSDATARANGE pCallerDataRange,
    IN PKSDATARANGE pDescriptorDataRange,
    IN DWORD dwBufferSize,
    OUT OPTIONAL PVOID pData,
    OUT PDWORD pdwDataSize
);

NTSTATUS AnlgAudioOutPinCreate
(
    IN PKSPIN pKSPin,
    IN PIRP   pIrp
);

//
// Description:
//  This is the dispatch table for the audio pin. It provides notifications
//  about creation, closure, processing, data formats, etc...
//
const KSPIN_DISPATCH AnlgAudioOutPinDispatch =
{
    AnlgAudioOutPinCreate,          // Create
    BasePinRemove,                  // Remove
    BasePinProcess,                 // Process
    NULL,                           // Pin Reset
    AnlgAudioOutPinSetDataFormat,   // Pin Set Data Format
    BasePinSetDeviceState,          // SetDeviceState
    NULL,                           // Pin Connect
    NULL,                           // Pin Disconnect
    NULL,                           // Clock Dispatch
    NULL                            // Allocator Dispatch
};

//
// Description:
//  Indicates the data range that is supported on the audio out pin.
//  Pointer to the Audio Data Formats (PinWaveIoRange).
//
const PKSDATARANGE AnlgAudioOutPinDataRanges [] =
{
    (PKSDATARANGE) &PinWaveIoRange
};

//
// Description:
//  AnlgAudioOutPinAllocatorFraming:
//  This is the simple framing structure for the audio pin.
//  Note that this will be modified via KsEdit when the actual
//  audio format is determined.
// Remarks:
//  Note that this will be modified via KsEdit when the
//  actual audio format is determined.
//
DECLARE_SIMPLE_FRAMING_EX
(
    AnlgAudioOutPinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    NUM_AUDIO_STREAM_BUFFER,
    0,
    0,
    320*240*3 
    );

//
// Description:
//  Pin descriptor of the analog audio output pin.
//
#define ANLG_AUDIO_OUT_PIN_DESCRIPTOR                                   \
{                                                                       \
    &AnlgAudioOutPinDispatch,                   /* Dispatch           */\
    NULL,                                       /* AutomationTable    */\
    {                                                                   \
        0,                                      /* Interfaces count   */\
        NULL,                                   /* Interfaces         */\
        0,                                      /* Mediums count      */\
        NULL,                                   /* Mediums            */\
        SIZEOF_ARRAY(AnlgAudioOutPinDataRanges),/* Range Count        */\
        AnlgAudioOutPinDataRanges,              /* Ranges             */\
        KSPIN_DATAFLOW_OUT,                     /* Dataflow           */\
        KSPIN_COMMUNICATION_BOTH,               /* Communication      */\
        &PINNAME_VIDEO_CAPTURE,                 /* Category           */\
        &VAMP_ANLG_AUDIO_OUT_PIN,               /* Name               */\
        0                                       /* Reserved           */\
    },                                                                  \
    KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |                              \
    KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |                              \
    KSPIN_FLAG_GENERATE_MAPPINGS,               /* Flags              */\
    1,                                          /* Instances Possible */\
    0,                                          /* Instances Necessary*/\
    &AnlgAudioOutPinAllocatorFraming,           /* Allocator Framing  */\
    (PFNKSINTERSECTHANDLEREX)                                           \
        &AudioIntersectDataFormat               /* IntersectHandler   */\
}

