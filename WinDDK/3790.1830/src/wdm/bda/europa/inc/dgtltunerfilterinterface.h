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

#include "DgtlPropTunerInterface.h"
#include "DgtlMethodsTunerInterface.h"
#include "DgtlTunerOutInterface.h"
#include "Mediums.h"

NTSTATUS DgtlTunerFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
);

NTSTATUS DgtlTunerFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the digital tuner filter. It provides
//  notification of creation, closure, processing and resets.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH DgtlTunerFilterDispatch =
{
    DgtlTunerFilterCreate,        // Filter Create
    DgtlTunerFilterClose,         // Filter Close
    NULL,                         // Filter Process
    NULL                          // Filter Reset
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Digital tuner filter automation table. This structure describes the
//  properties, methods and events of the filter.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(DgtlTunerFilterAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS(DgtlTunerFilterMethodSets),
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(RFTunerNodeAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES(RFTunerNodeProperties),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(COFDMDemodulatorNodeAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES(COFDMDemodulatorNodeProperties),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


const KSPIN_DISPATCH DgtlTunerInPinDispatch =
{
    NULL,               // Create
    NULL,                // Close
    NULL,               // Process
    NULL,               // Reset
    NULL,               // SetDataFormat
    NULL,               // SetDeviceState
    NULL,               // Connect
    NULL,               // Disconnect
    NULL,               // Clock
    NULL                // Allocator
};

const KS_DATARANGE_BDA_ANTENNA DgtlTunerInPinRange =
{
   // KSDATARANGE
    {
        sizeof( KS_DATARANGE_BDA_ANTENNA), // FormatSize
        0,                                 // Flags - (N/A)
        0,                                 // SampleSize - (N/A)
        0,                                 // Reserved
        { STATIC_KSDATAFORMAT_TYPE_BDA_ANTENNA },  // MajorFormat
        { STATIC_KSDATAFORMAT_SUBTYPE_NONE },  // SubFormat
        { STATIC_KSDATAFORMAT_SPECIFIER_NONE } // Specifier
    }
};

static PKSDATAFORMAT DgtlTunerInPinRanges[] =
{
    (PKSDATAFORMAT) &DgtlTunerInPinRange

    // Add more formats here if additional antenna signal formats are supported.
    //
};

//
//  Antenna Pin Automation Table
//
//  Lists all Property, Method, and Event Set tables for the antenna pin
//
DEFINE_KSAUTOMATION_TABLE(DgtlTunerInPinAutomation) 
{
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

#define DGTL_TUNER_IN_PIN_DESCRIPTOR                                        \
{                                                                           \
    &DgtlTunerInPinDispatch,                    /* Dispatch             */  \
    &DgtlTunerInPinAutomation,                  /* AutomationTable      */  \
    {                                                                       \
        0,                                      /* Interfaces count     */  \
        NULL,                                   /* Interfaces           */  \
        0,                                      /* Mediums count        */  \
        NULL,                                   /* Mediums              */  \
        SIZEOF_ARRAY(DgtlTunerInPinRanges),     /* Range Count          */  \
        DgtlTunerInPinRanges,                   /* Ranges               */  \
        KSPIN_DATAFLOW_IN,                      /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,               /* Communication        */  \
        NULL,                                   /* Category             */  \
        NULL,                                   /* Name                 */  \
        0                                       /* Reserved             */  \
    },                                                                      \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT        |                       \
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING   |                       \
    KSPIN_FLAG_FIXED_FORMAT,                    /* Flags                */  \
    1,                                          /* Instances Possible   */  \
    1,                                          /* Instances Necessary  */  \
    NULL,                                       /* Allocator Framing    */  \
    NULL                                        /* IntersectHandler     */  \
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Digital tuner filter pin descriptors. This list contains all the pin
//  descriptors.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX DgtlTunerFilterPinDescriptors[] =
{
    DGTL_TUNER_IN_PIN_DESCRIPTOR,       //Pin 0
    DGTL_TUNER_OUT_PIN_DESCRIPTOR       //Pin 1
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This array describes all Node Types available in the template
//  topology of the filter.
//
//////////////////////////////////////////////////////////////////////////////
const KSNODE_DESCRIPTOR DgtlTunerFilterNodeDescriptors[] =
{
    {
        &RFTunerNodeAutomation,         // PKSAUTOMATION_TABLE AutomationTable;
        &KSNODE_BDA_RF_TUNER,           // Type
        NULL                            // Name
    },
    {
        &COFDMDemodulatorNodeAutomation,// PKSAUTOMATION_TABLE AutomationTable;
        &KSNODE_BDA_COFDM_DEMODULATOR,  // Type
        NULL                            // Name
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Lists the Connections that are possible between pin types and
//  node types.  This, together with the Template Filter Descriptor, and
//  the Pin Pairings, describe how topologies can be created in the filter.
//
//////////////////////////////////////////////////////////////////////////////
const KSTOPOLOGY_CONNECTION DgtlTunerFilterConnections[] =
{
    { KSFILTER_NODE, 0, 0,             1 },
    {  0,            0, 1,             1 },
    {  1,            0, KSFILTER_NODE, 1 }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Filter Factory Descriptor for the digital tuner filter. This structure
//  brings together all of the structures that define the analog tuner filter
//  as it appears when it is first instanciated. Note that not all of the
//  template pin and node types may be exposed as pin and node factories when
//  the filter is first instanciated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(DgtlTunerFilterDescriptor)
{
    &DgtlTunerFilterDispatch,                           // Dispatch
    &DgtlTunerFilterAutomation,                         // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                        // Version
    0,                                                  // Flags
    &VAMP_DGTL_TUNER_FILTER,                            // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(DgtlTunerFilterPinDescriptors),
                                                        // PinDescriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_NETWORK_TUNER),
                                                        // Categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS(DgtlTunerFilterNodeDescriptors),
                                                        // NodeDescriptors
    DEFINE_KSFILTER_CONNECTIONS(DgtlTunerFilterConnections),
                                                        // Connections
    NULL                                                // ComponentId
};

//  Define BDA Template Topology Connections
//
//  Lists the Connections that are possible between pin types and
//  node types.  This, together with the Template Filter Descriptor, and
//  the Pin Pairings, describe how topologies can be created in the filter.
//
//                 ===========         ============
//  AntennaPin ----| RF Node |--Joint--|Demod Node|----TransportPin
//                 ===========         ============
//
//  The RF Node of this filter is controlled by the Antenna input pin.
//  RF properties will be set as NODE properties (with NodeType == 0)
//  on the filter's Antenna Pin
//
//  The Demodulator Node of this filter is controlled by the Transport output pin.
//  Demod properties will be set as NODE properties (with NodeType == 1)
//  on the filter's Transport Pin

//
//  Template Joints between the Antenna and Transport Pin Types.
//
//  Lists the template joints between the Antenna Input Pin Type and
//  the Transport Output Pin Type.
//
const ULONG AntennaTransportJoints[] =
{
    1
};

//
//  Template Pin Parings.
//
//  These are indexes into the template connections structure that
//  are used to determine which nodes get duplicated when more than
//  one output pin type is connected to a single input pin type or when
//  more that one input pin type is connected to a single output pin
//  type.
//
const BDA_PIN_PAIRING DgtlTunerFilterPinPairings[] =
{
    //  Antenna to Transport Topology Joints
    //
    {
        0,  // ulInputPin
        1,  // ulOutputPin
        1,  // ulcMaxInputsPerOutput
        1,  // ulcMinInputsPerOutput
        1,  // ulcMaxOutputsPerInput
        1,  // ulcMinOutputsPerInput
        SIZEOF_ARRAY(AntennaTransportJoints),   // ulcTopologyJoints
        AntennaTransportJoints                  // pTopologyJoints
    }
};
//
//  BDA Template Topology Descriptor for the filter.
//
//  This structure define the pin and node types that may be created
//  on the filter.
//
const BDA_FILTER_TEMPLATE BdaTemplateDgtlTunerFilter =
{
    &DgtlTunerFilterDescriptor,
    SIZEOF_ARRAY(DgtlTunerFilterPinPairings),
    DgtlTunerFilterPinPairings
};
