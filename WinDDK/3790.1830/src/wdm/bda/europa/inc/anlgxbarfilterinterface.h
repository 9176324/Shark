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

#include "AnlgPropXBarInterface.h"
#include "Mediums.h"

NTSTATUS AnlgXBarFilterCreate
(
    IN PKSFILTER Filter,    // Pointer to KSFILTER
    IN PIRP Irp             // Pointer to IRP
);

NTSTATUS AnlgXBarFilterClose
(
    IN PKSFILTER Filter,    // Pointer to KSFILTER
    IN PIRP Irp             // Pointer to IRP
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the crossbar filter. It provides
//  notification of creation, closure, processing and resets.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH AnlgXBarFilterDispatch =
{
    AnlgXBarFilterCreate,         // Filter Create
    AnlgXBarFilterClose,          // Filter Close
    NULL,                         // Filter Process
    NULL                          // Filter Reset
};

// these Pin descriptors are ignored from the system so do not add or change
// anything here, refer to the XBarPinInfoGetHandler

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the video input tuner pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_VIDEO_TUNER_IN_PIN_DESCRIPTOR                         \
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
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_TUNER,           /* Category             */  \
        &VAMP_ANLG_XBAR_IN_TUNER_VIDEO,     /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the video input composite pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_VIDEO_COMPOSITE_IN_PIN_DESCRIPTOR                     \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarWildcard),     /* Mediums count        */  \
        AnlgXBarWildcard,                   /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_COMPOSITE,       /* Category             */  \
        &VAMP_ANLG_XBAR_IN_COMPOSITE_VIDEO, /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    0,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the video input S-Video pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_VIDEO_SVIDEO_IN_PIN_DESCRIPTOR                        \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarWildcard),     /* Mediums count        */  \
        AnlgXBarWildcard,                   /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_SVIDEO,          /* Category             */  \
        &VAMP_ANLG_XBAR_IN_SVIDEO_VIDEO,    /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    0,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the audio input tuner pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_AUDIO_TUNER_IN_PIN_DESCRIPTOR                         \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarTunerAudioInMedium),/* Mediums count   */  \
        AnlgXBarTunerAudioInMedium,         /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_TUNER,           /* Category             */  \
        &VAMP_ANLG_XBAR_IN_TUNER_AUDIO,     /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the audio input composite pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_AUDIO_COMPOSITE_IN_PIN_DESCRIPTOR                     \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarWildcard),     /* Mediums count        */  \
        AnlgXBarWildcard,                   /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_COMPOSITE,       /* Category             */  \
        &VAMP_ANLG_XBAR_IN_COMPOSITE_AUDIO, /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    0,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the audio input S-Video pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_AUDIO_SVIDEO_IN_PIN_DESCRIPTOR                        \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarWildcard),     /* Mediums count        */  \
        AnlgXBarWildcard,                   /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                  /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_IN_SVIDEO,          /* Category             */  \
        &VAMP_ANLG_XBAR_IN_SVIDEO_AUDIO,    /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    0,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the video output pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_VIDEO_OUT_PIN_DESCRIPTOR                              \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarVideoOutMedium),/* Mediums count       */  \
        AnlgXBarVideoOutMedium,             /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                 /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_OUT,                /* Category             */  \
        &VAMP_ANLG_XBAR_OUT_VIDEO,          /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Pin descriptor of the audio output pin
//
//////////////////////////////////////////////////////////////////////////////
#define ANLG_XBAR_AUDIO_OUT_PIN_DESCRIPTOR                              \
{                                                                       \
    NULL,                                   /* Dispatch             */  \
    NULL,                                   /* AutomationTable      */  \
    {                                                                   \
        0,                                  /* Interfaces count     */  \
        NULL,                               /* Interfaces           */  \
        SIZEOF_ARRAY(AnlgXBarAudioOutMedium),/* Mediums count       */  \
        AnlgXBarAudioOutMedium,             /* Mediums              */  \
        0,                                  /* Range Count          */  \
        NULL,                               /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                 /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,           /* Communication        */  \
        &VAMP_ANLG_XBAR_OUT,                /* Category             */  \
        &VAMP_ANLG_XBAR_OUT_AUDIO,          /* Name                 */  \
        0                                   /* Reserved             */  \
    },                                                                  \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT, /* Flags              */  \
    1,                                      /* Instances Possible   */  \
    1,                                      /* Instances Necessary  */  \
    NULL,                                   /* Allocator Framing    */  \
    0                                       /* IntersectHandler     */  \
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Crossbar filter pin descriptors. This list contains all the pin
//  descriptors (input and output pins)
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX AnlgXBarFilterPinDescriptors [] =
{
    ANLG_XBAR_VIDEO_TUNER_IN_PIN_DESCRIPTOR,    //Pin 0
    ANLG_XBAR_VIDEO_COMPOSITE_IN_PIN_DESCRIPTOR,//Pin 1
    ANLG_XBAR_VIDEO_SVIDEO_IN_PIN_DESCRIPTOR,   //Pin 2
    ANLG_XBAR_AUDIO_TUNER_IN_PIN_DESCRIPTOR,    //Pin 3
    ANLG_XBAR_AUDIO_COMPOSITE_IN_PIN_DESCRIPTOR,//Pin 4
    ANLG_XBAR_AUDIO_SVIDEO_IN_PIN_DESCRIPTOR,   //Pin 5
    ANLG_XBAR_VIDEO_OUT_PIN_DESCRIPTOR,         //Pin 6
    ANLG_XBAR_AUDIO_OUT_PIN_DESCRIPTOR          //Pin 7
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Crossbar filter automation table. This structure describes the properties,
//  methods and events of the filter. The crossbar contains only properties.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(AnlgXBarFilterAutomationTable)
{
    DEFINE_KSAUTOMATION_PROPERTIES(AnlgXBarFilterPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Crossbar filter connection topology. This structure describes the
//  association between the input and the output pins. All the video input
//  pins (tuner, composite,...) are able to be connected to the video output
//  pin and all audio input pins to the audio output pin.
//
//////////////////////////////////////////////////////////////////////////////
const KSTOPOLOGY_CONNECTION XBarFilterConnections[] =
{
    {KSFILTER_NODE, 0, KSFILTER_NODE, 6},
    {KSFILTER_NODE, 1, KSFILTER_NODE, 6},
    {KSFILTER_NODE, 2, KSFILTER_NODE, 6},
    {KSFILTER_NODE, 3, KSFILTER_NODE, 7},
    {KSFILTER_NODE, 4, KSFILTER_NODE, 7},
    {KSFILTER_NODE, 5, KSFILTER_NODE, 7}
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Filter Factory Descriptor for the crossbar filter. This structure brings
//  together all of the structures that define the crossbar filter as it
//  appears when it is first instanciated. Note that not all of the template
//  pin and node types may be exposed as pin and node factories when the
//  filter is first instanciated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(AnlgXBarFilterDescriptor)
{
    &AnlgXBarFilterDispatch,                            // Dispatch
    &AnlgXBarFilterAutomationTable,                     // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                        // Version
    0,                                                  // Flags
    &VAMP_ANLG_XBAR_FILTER,                             // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(AnlgXBarFilterPinDescriptors),
                                                        // PinDescriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_CROSSBAR),      // Categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,              // NodeDescriptors
    DEFINE_KSFILTER_CONNECTIONS(XBarFilterConnections), // Connections
    NULL                                                // Component ID
};

