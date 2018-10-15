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

#include "AnlgPropProcampInterface.h"
#include "AnlgPropVideoDecoderInterface.h"
#include "AnlgVideoIn.h"
#include "AnlgAudioIn.h"
#include "AnlgVideoCapInterface.h"
#include "AnlgVideoPrevInterface.h"
#include "AnlgAudioOutInterface.h"
#include "AnlgVBIOutInterface.h"

NTSTATUS AnlgCaptureFilterCreate(IN PKSFILTER pKSFilter,
                                 IN PIRP      pIrp);

NTSTATUS AnlgCaptureFilterClose(IN PKSFILTER pKSFilter,
                                IN PIRP      pIrp);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the analog capture filter. It provides
//  notification of creation, closure.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH AnlgCaptureFilterDispatch =
{
    AnlgCaptureFilterCreate,                // Filter Create
    AnlgCaptureFilterClose,                 // Filter Close
    NULL,                                   // Filter Process
    NULL                                    // Filter Reset
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Property set table for analog video decoder and procamp properties
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_SET_TABLE(AnlgVideoPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_VIDEOPROCAMP,         // Set
        SIZEOF_ARRAY(ProcampPropertyTable),     // PropertiesCount
        ProcampPropertyTable,                   // PropertyItems
        0,                                      // FastIoCount
        NULL                                    // FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_VIDEODECODER,         // Set
        SIZEOF_ARRAY(DecoderPropertyTable),     // PropertiesCount
        DecoderPropertyTable,                   // PropertyItems
        0,                                      // FastIoCount
        NULL                                    // FastIoTable
    )
//  ,
//    DEFINE_KSPROPERTY_SET
//    (
//        &PROPSETID_VIDCAP_VIDEOCOMPRESSION,     // Set
//        SIZEOF_ARRAY(CompressionPropertyTable), // PropertiesCount
//        CompressionPropertyTable,               // PropertyItems
//        0,                                      // FastIoCount
//        NULL                                    // FastIoTable
//    ),
//    DEFINE_KSPROPERTY_SET
//    (
//        &PROPSETID_VIDCAP_DROPPEDFRAMES,        // Set
//        SIZEOF_ARRAY(DroppedFramesPropertyTable),// PropertiesCount
//        DroppedFramesPropertyTable,             // PropertyItems
//        0,                                      // FastIoCount
//        NULL                                    // FastIoTable
//    )
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The list of pin descriptors on the capture filter.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX CaptureFilterPinDescriptors [] =
{
    ANLG_VIDEO_CAP_PIN_DESCRIPTOR,
    ANLG_VIDEO_PREV_PIN_DESCRIPTOR,
    ANLG_VBI_OUT_PIN_DESCRIPTOR,
    ANLG_AUDIO_OUT_PIN_DESCRIPTOR,
    ANLG_VIDEO_IN_PIN_DESCRIPTOR,
    ANLG_AUDIO_IN_PIN_DESCRIPTOR
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The list of pin descriptors on the WaveIn filter.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX WaveInPinDescriptors [] =
{
    ANLG_AUDIO_OUT_PIN_DESCRIPTOR,
    ANLG_AUDIO_IN_PIN_DESCRIPTOR
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The list of category GUIDs for the capture filter.
//
//////////////////////////////////////////////////////////////////////////////
const GUID CaptureFilterCategories [] =
{
    STATICGUIDOF (KSCATEGORY_VIDEO),
    STATICGUIDOF (KSCATEGORY_CAPTURE)
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The list of category GUIDs for the Wave-In filter.
//
//////////////////////////////////////////////////////////////////////////////
const GUID WaveInFilterCategories [] =
{
    STATICGUIDOF (KSCATEGORY_CAPTURE),
    STATICGUIDOF (KSCATEGORY_AUDIO)
    
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Automation table for the capture filter.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(AnlgCaptureFilterAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES(AnlgVideoPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  analog audio filter connection topology. This structure describes the
//  association between the input and the output pin. The input pin is
//  connected to the output pin.
//
//////////////////////////////////////////////////////////////////////////////
const KSTOPOLOGY_CONNECTION WaveInFilterConnections[] =
{
    {KSFILTER_NODE, 1, 0, 1},
	{0, 0,	KSFILTER_NODE, 0}
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
// Node automation table for WaveIn filter
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(WaveInNodeAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This array describes all Node Types available in the topology of the 
//  filter.
//
//////////////////////////////////////////////////////////////////////////////
const KSNODE_DESCRIPTOR WaveInFilterNodeDescriptors[] =
{
    {
        &WaveInNodeAutomation,         // PKSAUTOMATION_TABLE AutomationTable;
        &KSNODETYPE_VOLUME,           // Type
        NULL                            // Name
    }
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The capture filter descriptor brings together all of the structures
//  that define the tuner filter as it appears when it is instantiated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(AnlgCaptureFilterDescriptor)
{
    &AnlgCaptureFilterDispatch,                 // Dispatch
    &AnlgCaptureFilterAutomationTable,          // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                // Version
    0,                                          // Flags
    &VAMP_ANLG_CAP_FILTER,                      // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors),
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS, //(CaptureFilterConnections),
    NULL                                        // Component ID
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The WaveIn filter descriptor brings together all of the structures
//  that define the WaveIn filter as it appears when it is instantiated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(WaveInFilterDescriptor)
{
    &AnlgCaptureFilterDispatch,                 // Dispatch
    &AnlgCaptureFilterAutomationTable,          // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                // Version
    0,                                          // Flags
    &VAMP_ANLG_WAV_FILTER,                      // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS (WaveInPinDescriptors),
    DEFINE_KSFILTER_CATEGORIES (WaveInFilterCategories),
    DEFINE_KSFILTER_NODE_DESCRIPTORS(WaveInFilterNodeDescriptors),
    DEFINE_KSFILTER_CONNECTIONS(WaveInFilterConnections), 
    NULL                                        // Component ID
};
