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

#include "AnlgPropTVAudioInterface.h"

NTSTATUS AnlgTVAudioFilterCreate
(
    IN PKSFILTER Filter,    // Pointer to KSFILTER
    IN PIRP Irp             // Pointer to IRP
);

NTSTATUS AnlgTVAudioFilterClose
(
    IN PKSFILTER Filter,    // Pointer to KSFILTER
    IN PIRP Irp             // Pointer to IRP
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the TVAudio filter. It provides
//  notification of creation, closure, processing, and resets.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH AnlgTVAudioFilterDispatch =
{
    AnlgTVAudioFilterCreate,    // Filter Create
    AnlgTVAudioFilterClose,     // Filter Close
    NULL,                       // Filter Process
    NULL                        // Filter Reset
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  TV Audio Filter Automation Table
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(AnlgTVAudioFilterAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES(AnlgTVAudioFilterPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//   Pin descriptor of the TV audio input pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_TV_AUDIO_IN_PIN_DESCRIPTOR                                 \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        0,                                  /* Mediums count        */  \
        NULL,                               /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
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
//   Pin descriptor of the TV audio output pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_TV_AUDIO_OUT_PIN_DESCRIPTOR                                \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        0,                                  /* Mediums count        */  \
        NULL,                               /* Mediums              */  \
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
//  TV audio filter pin descriptors. This list contains all the pin
//  descriptors (input and output pins)
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX AnlgTVAudioFilterPinDescriptors[] =
{
    ANLG_TV_AUDIO_IN_PIN_DESCRIPTOR,    //Pin 0
    ANLG_TV_AUDIO_OUT_PIN_DESCRIPTOR    //Pin 1
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  TV audio filter connection topology. This structure describes the
//  association between the input and the output pin. The input pin is
//  connected to the output pin.
//
//////////////////////////////////////////////////////////////////////////////
const KSTOPOLOGY_CONNECTION TVAudioFilterConnections[] =
{
    {KSFILTER_NODE, 0, KSFILTER_NODE, 1},
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Filter Factory Descriptor for the TV audio filter. This structure brings
//  together all structures that define the TV audio filter as it appears
//  when it is instanciated for the first time. Note that not all of the
//  template pin and node types may be exposed as pin and node factories when
//  the filter is instanciated for the first time.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(AnlgTVAudioFilterDescriptor)
{
    &AnlgTVAudioFilterDispatch,                             // Dispatch
    &AnlgTVAudioFilterAutomation,                           // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                            // Version
    0,                                                      // Flags
    &VAMP_ANLG_TVAUDIO_FILTER,                              // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(AnlgTVAudioFilterPinDescriptors),
                                                            // PinDescriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_TVAUDIO),           // Categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,                  // NodeDescriptors
    DEFINE_KSFILTER_CONNECTIONS(TVAudioFilterConnections),  // Connections
    NULL                                                    // Component ID
};
