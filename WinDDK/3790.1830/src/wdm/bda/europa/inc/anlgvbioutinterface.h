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

#include "AnlgVbiOutFormats.h"
#include "BasePinInterface.h"
#include "VampDeviceResources.h"

//
// Description:
//  Number of VBI stream buffers (size of the queue).
//
#define NUM_VBI_STREAM_BUFFER 5

//////////////////////////////////////////////////////////////////////////////
//
//  C-Function prototypes (due to MS interface we still need C-Functions)
//  Implemented in "AnlgVbiOutInterface.cpp"
//
//////////////////////////////////////////////////////////////////////////////

NTSTATUS AnlgVBIOutPinSetDataFormat
(
    IN                PKSPIN           pKSPin,
    IN OPTIONAL       PKSDATAFORMAT    pKSOldFormat,
    IN OPTIONAL       PKSMULTIPLE_ITEM pKSOldAttributeList,
    IN          const KSDATARANGE      *pKSDataRange,
    IN OPTIONAL const KSATTRIBUTE_LIST *pKSAttributeRange
);

NTSTATUS AnlgVBIOutIntersectDataFormat
(
    IN           PKSFILTER    pKSFilter,
    IN           PIRP         pIrp,
    IN           PKSP_PIN     pPinInstance,
    IN           PKSDATARANGE pCallerDataRange,
    IN           PKSDATARANGE pDescriptorDataRange,
    IN           DWORD        dwBufferSize,
    OUT OPTIONAL PVOID        pData,
    OUT          PDWORD       pdwDataSize
);

NTSTATUS AnlgVBIOutPinCreate
(
    IN PKSPIN pKSPin,
    IN PIRP   pIrp
);


//
// function for private use!
// passes back the Vamp device resource object
// which is capsulated in the KS filter.
//

CVampDeviceResources* AnlgVBIOutGetVampDevResObj
(
    IN  PKSFILTER pKSFilter
);

//////////////////////////////////////////////////////////////////////////////
//
//  VBI definitions and pin descriptions
//
//////////////////////////////////////////////////////////////////////////////

//
// Description:
//  This is the dispatch table for the vbi pin. It provides notifications
//  about creation, closure, processing, data formats, etc...
//
const KSPIN_DISPATCH AnlgVBIOutPinDispatch =
{
    AnlgVBIOutPinCreate,  // Create
    BasePinRemove,        // Remove
    BasePinProcess,       // Process
    NULL,                       // Pin Reset
    AnlgVBIOutPinSetDataFormat, // Pin Set Data Format
    BasePinSetDeviceState,// SetDeviceState
    NULL,                       // Pin Connect
    NULL,                       // Pin Disconnect
    NULL,                       // Clock Dispatch
    NULL                        // Allocator Dispatch
};

//
// Description:
//  This is the list of data ranges supported on the VBI pin. We support
//  one PAL (50Hz) and one NTSC (60Hz).
//
const PKSDATARANGE AnlgVBIOutPinDataRanges [] =
{
    (PKSDATARANGE) &FormatVBI_NTSC,
    (PKSDATARANGE) &FormatVBI_PAL

};

//
// Description:
//  (AnlgVBIOutPinAllocatorFraming)
//  This is the simple framing structure for the capture pin. Note that this
//  will be modified via KsEdit when the actual capture format is determined.
//
DECLARE_SIMPLE_FRAMING_EX
(
    AnlgVBIOutPinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | \
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    NUM_VBI_STREAM_BUFFER,
    0,
    0,
    VBI_SAMPLES_PER_LINE*VBI_LINES_PAL*1
);

//
// Description:
//  This structure describes the VBI pin with all of its properties.
//
#define ANLG_VBI_OUT_PIN_DESCRIPTOR                                 \
{                                                                   \
    &AnlgVBIOutPinDispatch,         /* Dispatch                   */\
    NULL,                           /* AutomationTable            */\
    {                                                               \
        0,                          /* Interfaces count           */\
        NULL,                       /* Interfaces                 */\
        0,                          /* Mediums count              */\
        NULL,                       /* Mediums                    */\
        SIZEOF_ARRAY(AnlgVBIOutPinDataRanges),/* Range Count      */\
        AnlgVBIOutPinDataRanges,    /* Ranges                     */\
        KSPIN_DATAFLOW_OUT,         /* Dataflow                   */\
        KSPIN_COMMUNICATION_BOTH,   /* Communication              */\
        &PINNAME_VIDEO_VBI,         /* Category                   */\
        &PINNAME_VIDEO_VBI,         /* Name                       */\
        0                           /* Reserved                   */\
    },                                                              \
    KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |                          \
    KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |                          \
    KSPIN_FLAG_GENERATE_MAPPINGS,           /* Flags              */\
    1,                                     /* Instances Possible  */\
    0,                                     /* Instances Necessary */\
    &AnlgVBIOutPinAllocatorFraming,        /* Allocator Framing   */\
    (PFNKSINTERSECTHANDLEREX)                                       \
        &AnlgVBIOutIntersectDataFormat     /* IntersectHandler    */\
}
