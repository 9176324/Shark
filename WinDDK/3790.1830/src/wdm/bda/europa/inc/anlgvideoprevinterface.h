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

#include "AnlgVideoFormats.h"
#include "BasePinInterface.h"

//
// Description:
//  Number of VIDEO PREVIEW stream buffers (size of the queue). Must never be 
//  more than 5 due to Overlay mixer bug.
//
#define NUM_VD_PREV_STREAM_BUFFER 5

//////////////////////////////////////////////////////////////////////////////
//
//  C-Function prototypes (due to MS interface we still need C-Functions)
//
//////////////////////////////////////////////////////////////////////////////


NTSTATUS AnlgVideoPrevPinSetDataFormat
(
    IN PKSPIN pKSPin,
    IN OPTIONAL PKSDATAFORMAT pKSOldFormat,
    IN OPTIONAL PKSMULTIPLE_ITEM pKSOldAttributeList,
    IN const KSDATARANGE* pKSDataRange,
    IN OPTIONAL const KSATTRIBUTE_LIST* pKSAttributeRange
);


NTSTATUS AnlgVideoPrevIntersectDataFormat
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

NTSTATUS AnlgVideoPrevPinCreate
(
    IN PKSPIN pKSPin,
    IN PIRP   pIrp
);

//
// Description:
//  This is the dispatch table for the video preview pin.
//  It provides notifications about creation, closure,
//  processing, data formats, etc...
//
const KSPIN_DISPATCH AnlgVideoPrevPinDispatch =
{
    AnlgVideoPrevPinCreate,         // Create
    BasePinRemove,                  // Remove
    BasePinProcess,                 // Process
    NULL,                           // Pin Reset
    AnlgVideoPrevPinSetDataFormat,  // Pin Set Data Format
    BasePinSetDeviceState,          // SetDeviceState
    NULL,                           // Pin Connect
    NULL,                           // Pin Disconnect
    NULL,                           // Clock Dispatch
    NULL                            // Allocator Dispatch
};

//
// Description:
//  This is the list of data ranges supported on the video preview pin.
//  We support two: one RGB15, and one YUY2.
//
const PKSDATARANGE AnlgVideoPrevPinDataRanges [] =
{
    (PKSDATARANGE) &InterlacedFormat_50Hz_YUY2,
    (PKSDATARANGE) &Format_50Hz_YUY2,
    (PKSDATARANGE) &Format_60Hz_YUY2,
    //(PKSDATARANGE) &DeinterlacedFormat_60Hz_YUY2,
    (PKSDATARANGE) &Format_50Hz_RGB15,
    (PKSDATARANGE) &Format_60Hz_RGB15,
    //(PKSDATARANGE) &DeinterlacedFormat_60Hz_RGB15,
};

//
// Description:
//  AnlgVideoPrevPinAllocatorFraming:
//  This is the simple framing structure for the video preview pin.
// Remarks:
//  Note that this will be modified via KsEdit when the
//  actual preview format is determined.
//
DECLARE_SIMPLE_FRAMING_EX
(
    AnlgVideoPrevPinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    NUM_VD_PREV_STREAM_BUFFER,
    0,
    0,
    320*240*3 
);

//
// Description:
//  Pin descriptor of the analog video preview pin.
//
#define ANLG_VIDEO_PREV_PIN_DESCRIPTOR                                  \
{                                                                       \
    &AnlgVideoPrevPinDispatch,              /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        0,                                  /* Mediums count        */  \
        NULL,                               /* Mediums              */  \
        SIZEOF_ARRAY(AnlgVideoPrevPinDataRanges),/* Range Count     */  \
        AnlgVideoPrevPinDataRanges,         /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                 /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &PINNAME_VIDEO_PREVIEW,             /* Category             */  \
        &PINNAME_VIDEO_PREVIEW,             /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |                              \
    KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |                              \
    KSPIN_FLAG_GENERATE_MAPPINGS,           /* Flags                */  \
    1,                                      /* Instances Possible   */  \
    0,                                      /* Instances Necessary  */  \
    &AnlgVideoPrevPinAllocatorFraming,      /* Allocator Framing    */  \
    (PFNKSINTERSECTHANDLEREX)                                           \
        &AnlgVideoPrevIntersectDataFormat   /* IntersectHandler     */  \
}


