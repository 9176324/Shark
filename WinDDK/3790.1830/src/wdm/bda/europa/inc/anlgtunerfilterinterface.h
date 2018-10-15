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

#include "AnlgPropTunerInterface.h"
#include "AnlgMethodsTunerInterface.h"
#include "Mediums.h"

NTSTATUS AnlgTunerFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
);

NTSTATUS AnlgTunerFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the analog tuner filter. It provides
//  notification of creation, closure, processing and resets.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH AnlgTunerFilterDispatch =
{
    AnlgTunerFilterCreate,        // Filter Create
    AnlgTunerFilterClose,         // Filter Close
    NULL,                         // Filter Process
    NULL                          // Filter Reset
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the video output tuner pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_TUNER_VIDEO_OUT_PIN_DESCRIPTOR                             \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarTunerVideoInMedium),/* Mediums count   */  \
        AnlgXBarTunerVideoInMedium,         /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                 /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        NULL,                               /* Category             */  \
        NULL,                               /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT        |                   \
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING   |                   \
    KSPIN_FLAG_FIXED_FORMAT,                /* Flags                */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    NULL                                    /* IntersectHandler     */  \
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the audio output tuner pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_TUNER_AUDIO_OUT_PIN_DESCRIPTOR                             \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgTVAudioInMedium),  /* Mediums count        */  \
        AnlgTVAudioInMedium,                /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                 /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        NULL,                               /* Category             */  \
        NULL,                               /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT        |                   \
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING   |                   \
    KSPIN_FLAG_FIXED_FORMAT,                /* Flags                */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    NULL                                    /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Analog tuner filter pin descriptors. This list contains all the pin
//  descriptors.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX AnlgTunerFilterPinDescriptors[] =
{
    ANLG_TUNER_VIDEO_OUT_PIN_DESCRIPTOR,	//Pin 0
    ANLG_TUNER_AUDIO_OUT_PIN_DESCRIPTOR		//Pin 1
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Analog tuner filter automation table. This structure describes the 
//  properties, methods and events of the filter.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(AnlgTunerFilterAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES(AnlgTunerFilterPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS(AnlgTunerFilterMethodSets),
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Filter Factory Descriptor for the analog tuner filter. This structure 
//  brings together all of the structures that define the analog tuner filter 
//  as it appears when it is first instanciated. Note that not all of the 
//  template pin and node types may be exposed as pin and node factories when 
//  the filter is first instanciated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(AnlgTunerFilterDescriptor)
{
    &AnlgTunerFilterDispatch,                           // Dispatch
    &AnlgTunerFilterAutomation,                         // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                        // Version
    0,                                                  // Flags
    &VAMP_ANLG_TUNER_FILTER,                            // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(AnlgTunerFilterPinDescriptors),
                                                        // PinDescriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_TVTUNER),       // Categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,              // NodeDescriptors
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,                // Connections
    NULL                                                // ComponentId
};
