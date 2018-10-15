/*++

Copyright (c) 1999  Microsoft Corporation

Module Name:

    ObjDesc.cpp

Abstract:

    Static object description data structures.

    This file includes initial descriptors for all filter, pin, and node
    objects exposed by this driver.  It also include descriptors for the
    properties, methods, and events on those objects.

--*/

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif // ALLOC_DATA_PRAGMA

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA


//
// Before defining structures for nodes, pins, and filters, 
// it may be useful to show the filter's template topology here. 
//
// The sample filter's topology:
//
//                     Node Type 0           Node Type 1
//                          |                     |
//                          v                     v   
//                   ---------------     --------------------
//   Antenna         |             |     |                  |             Transport 
//   In Pin  --------|  Tuner Node |--X--| Demodulator Node |------------   Out Pin
//     ^       ^     |             |  ^  |                  | ^                ^
//     |       |     ---------------  |  -------------------- |                |
//     |       |                      |                       |                |
//     |       -- Connection 0        -- Connection 1         -- Connection 2  |
//     |                                 Topology Joint                        |
//     ---- Pin Type 0                                           Pin Type 1 ----
//
//

//===========================================================================
//
//  Node  definitions
//
//  Nodes are special in that, though they are defined at the filter level,
//  they are actually associated with a pin type.  The filter's node
//  descriptor list is actually a list of node types.
//  
//  BDA allows a node type to appear only once in a template topology.
//  This means that a node in an actual filter topology can be uniquely
//  identified by specifying the node type along with the actual input and
//  output pin IDs of the the path that the node appears in.
//
//  Note that the dispatch routines for nodes actually point to
//  pin-specific methods because the context data associated with
//  a node is stored in the pin context.
//
//  Node property begin with a KSP_NODE structure that contains the node type of the node to which
//  that property applies. This begs the question:
//
//  "How is a node uniquely identified by only the node type?"
//
//  The BDA Support Library uses the concept of a topology joint to solve
//  this problem.  Nodes upstream of a topology joint have their properties
//  dispatched to the input pin of the path.  Properties for nodes
//  downstream of the joint are dispatched to the output pin of the path
//  containing the node.
//
//  Node properties and methods should only be accessed from the 
//  appropriate pin.  The BDA Support Library helps assure this by
//  automatically merging a node type's automation table onto the automation
//  table of the correct pin.  This pin is called the controlling pin
//  for that node type.
//
//===========================================================================

typedef enum {
    BDA_SAMPLE_TUNER_NODE = 0,
    BDA_SAMPLE_DEMODULATOR_NODE
}BDA_SAMPLE_NODES;


//===========================================================================
//
//  BDA Sample Tuner Node (Node Type 0) definitions
//
//  Define structures here for the Properties, Methods, and Events
//  available on the BDA Sample Tuner Node.
//
//  This node is associated with an antenna input pin, therefore, the node
//  properties should be set/retrieved using the antenna input pin.  The
//  BDA Support Library will automatically merge the node's automation
//  table into the automation table for the antenna input pin.
//
//  Properties and methods are dispatched to the Antenna class.
//
//===========================================================================


//
//  BDA Sample Tune Frequency Filter
//
//  Define dispatch routines for specific properties.
//
//  One property is used to get and set the tuner's center frequency. 
//  For this property, define dispatch routines to get and set the frequency.
//
//  Other properties can be used to get and set the tuner's frequency range,
//  as well as to report signal strength.
//
//  These properties must be supported by BDA and 
//  defined elsewhere (for example, in Bdamedia.h).
//
DEFINE_KSPROPERTY_TABLE(SampleTunerNodeFrequencyProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_FREQUENCY(
        CAntennaPin::GetCenterFrequency,
        CAntennaPin::PutCenterFrequency
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_FREQUENCY_MULTIPLIER(
        NULL, NULL
        ),

#ifdef SATELLITE_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_POLARITY(
        NULL, NULL
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_RANGE(
        NULL, NULL
        ),
#endif // SATELLITE_TUNER

#ifdef CHANNEL_BASED_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_TRANSPONDER(
        NULL, NULL
        ),
#endif // CHANNEL_BASED_TUNER

#ifdef DVBT_TUNER
    DEFINE_KSPROPERTY_ITEM_BDA_RF_TUNER_BANDWIDTH(
        NULL, NULL
        ),
#endif // DVBT_TUNER
};


//
//  BDA Signal Statistics Properties
//
//  Defines the dispatch routines for the Signal Statistics Properties
//  on the RF Tuner, Demodulator, and PID Filter Nodes
//
DEFINE_KSPROPERTY_TABLE(SampleRFSignalStatsProperties)
{
#ifdef OPTIONAL_SIGNAL_STATISTICS
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_STRENGTH(
        NULL, NULL
        ),
#endif // OPTIONAL_SIGNAL_STATISTICS
    
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_PRESENT(
        CAntennaPin::GetSignalStatus,
        NULL
        ),
};


//
//  Define the Property Sets for the sample tuner node from the 
//  previously defined node properties and from property sets
//  that BDA supports.
//  These supported property sets must be defined elsewhere 
//  (for example, in Bdamedia.h).
//
//  Associate the sample tuner node with the antenna input pin. 
//
DEFINE_KSPROPERTY_SET_TABLE(SampleTunerNodePropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaFrequencyFilter,    // Property Set defined elsewhere
        SIZEOF_ARRAY(SampleTunerNodeFrequencyProperties),  // Number of properties in the array
        SampleTunerNodeFrequencyProperties,  // Property set array
        0,      // FastIoCount
        NULL    // FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSignalStats,    // Property Set defined elsewhere
        SIZEOF_ARRAY(SampleRFSignalStatsProperties),  // Number of properties in the array
        SampleRFSignalStatsProperties,  // Property set array
        0,      // FastIoCount
        NULL    // FastIoTable
    )
};


//
//  Define the Automation Table for the BDA sample tuner node.
//
DEFINE_KSAUTOMATION_TABLE(SampleTunerNodeAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(SampleTunerNodePropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};



//===========================================================================
//
//  Sample Demodulator Node definitions
//
//  This structure defines the Properties, Methods, and Events
//  available on the BDA Demodulator Node.
//
//  This node is associated with a transport output pin and thus the node
//  properties should be set/put using the transport output pin.
//
//===========================================================================


//
//  BDA Signal Statistics Properties for Demodulator Node
//
//  Defines the dispatch routines for the Signal Statistics Properties
//  on the Demodulator Node.
//
DEFINE_KSPROPERTY_TABLE(SampleDemodSignalStatsProperties)
{
#ifdef OPTIONAL_SIGNAL_STATISTICS
    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_QUALITY(
        NULL, NULL
        ),
#endif // OPTIONAL_SIGNAL_STATISTICS

    DEFINE_KSPROPERTY_ITEM_BDA_SIGNAL_LOCKED(
        CTransportPin::GetSignalStatus,
        NULL
        ),
};


DEFINE_KSPROPERTY_TABLE(SampleAutoDemodProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_AUTODEMODULATE_START(
        NULL,
        CTransportPin::PutAutoDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_AUTODEMODULATE_STOP(
        NULL,
        CTransportPin::PutAutoDemodProperty
        )
};

#if !ATSC_RECEIVER

//
//  BDA Digital Demodulator Property Set for Demodulator Node
//
//  Defines the dispatch routines for the Digital Demodulator Properties
//  on the Demodulator Node.
//
DEFINE_KSPROPERTY_TABLE(SampleDigitalDemodProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_MODULATION_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_INNER_FEC_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_INNER_FEC_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_OUTER_FEC_TYPE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_OUTER_FEC_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SYMBOL_RATE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),

#if DVBS_RECEIVER
    DEFINE_KSPROPERTY_ITEM_BDA_SPECTRAL_INVERSION(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
#endif // DVBS_RECEIVER

#if DVBT_RECEIVER
    DEFINE_KSPROPERTY_ITEM_BDA_GUARD_INTERVAL(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_TRANSMISSION_MODE(
        CTransportPin::GetDigitalDemodProperty,
        CTransportPin::PutDigitalDemodProperty
        )
#endif // DVBT_RECEIVER
};

#endif // !ATSC_RECEIVER

//
//  Sample Demodulator Node Extension Properties
//
//  Define dispatch routines for a set of driver specific
//  demodulator node properties.  This is how venders add support
//  for properties that are specific to their hardware.  They can
//  access these properties through a KSProxy plugin.
//
DEFINE_KSPROPERTY_TABLE(BdaSampleDemodExtensionProperties)
{
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1(  // A read and write property
        CTransportPin::GetExtensionProperties, // or NULL if not method to get the property
        CTransportPin::PutExtensionProperties // or NULL if not method to put the property
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2(  //A read only property
        CTransportPin::GetExtensionProperties, // or NULL if not method to get the property
        NULL // or NULL if not method to put the property
        ),
    DEFINE_KSPROPERTY_ITEM_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3(  //A write only property
        NULL, // or NULL if not method to get the property
        CTransportPin::PutExtensionProperties // or NULL if not method to put the property
        ),
};


//
//  Demodulator Node Property Sets supported
//
//  This table defines all property sets supported by the
//  Demodulator Node associated with the transport output pin.
//
DEFINE_KSPROPERTY_SET_TABLE(SampleDemodNodePropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaAutodemodulate,                // Set
        SIZEOF_ARRAY(SampleAutoDemodProperties),   // PropertiesCount
        SampleAutoDemodProperties,                 // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    ),

#if !ATSC_RECEIVER

    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaDigitalDemodulator,                // Set
        SIZEOF_ARRAY(SampleDigitalDemodProperties),   // PropertiesCount
        SampleDigitalDemodProperties,                 // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    ),

#endif // !ATSC_RECEIVER


    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSignalStats,                // Set
        SIZEOF_ARRAY(SampleDemodSignalStatsProperties),   // PropertiesCount
        SampleDemodSignalStatsProperties,                 // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSampleDemodExtensionProperties, // Set
        SIZEOF_ARRAY(BdaSampleDemodExtensionProperties), // Number of properties in the array
        BdaSampleDemodExtensionProperties, // Property set array
        0,                                                                              // FastIoCount
        NULL                                                                      // FastIoTable
    ),

    //
    //  Additional property sets for the node can be added here.
    //
};


//
//  Demodulator Node Automation Table
//
//  This structure defines the Properties, Methods, and Events
//  available on the BDA Demodulator Node.
//  These are used to set the symbol rate, and Viterbi rate,
//  as well as to report signal lock and signal quality.
//
DEFINE_KSAUTOMATION_TABLE(SampleDemodulatorNodeAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES( SampleDemodNodePropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


//===========================================================================
//
//  Antenna Pin Definitions
//
//===========================================================================

//
//  The BDA Support Library automatically merges the RF tuner node properties,
//  methods, and events onto the antenna pin's automation table. It also
//  merges properties, methods, and events the that are require and
//  supplied by the BDA Support Library.
//

//  
//  The hardware vendor may want to provide driver specific properties,
//  methods, or events on the antenna pin or override those provided by
//  the BDA Support Library.  Such roperties, methods, and events will
//  be defined here.
//

//
//  Define the Automation Table for the antenna pin.
//
//
DEFINE_KSAUTOMATION_TABLE(AntennaAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


//
//  Dispatch Table for the antenna pin.
//
const
KSPIN_DISPATCH
AntennaPinDispatch =
{
    CAntennaPin::PinCreate,  // Create
    CAntennaPin::PinClose,  // Close
    NULL,  // Process siganl data
    NULL,  // Reset
    NULL,  // SetDataFormat
    CAntennaPin::PinSetDeviceState,  // SetDeviceState
    NULL,  // Connect
    NULL,  // Disconnect
    NULL,  // Clock
    NULL   // Allocator
};


//
//  Format the input antenna stream connection.
//
//  Used to connect the input antenna pin to a specific upstream filter,
//  such as the network provider.
//
const KS_DATARANGE_BDA_ANTENNA AntennaPinRange =
{
   // insert the KSDATARANGE and KSDATAFORMAT here
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

//  Format Ranges of Antenna Input Pin.
//
static PKSDATAFORMAT AntennaPinRanges[] =
{
    (PKSDATAFORMAT) &AntennaPinRange,

    // Add more formats here if additional antenna signal formats are supported.
    //
};


//===========================================================================
//
//  Transport Output Pin Definitions
//
//===========================================================================

//
//  The BDA Support Library automatically merges the RF tuner node properties,
//  methods, and events onto the antenna pin's automation table. It also
//  merges properties, methods, and events the that are require and
//  supplied by the BDA Support Library.
//

//  
//  The hardware vendor may want to provide driver specific properties,
//  methods, or events on the antenna pin or override those provided by
//  the BDA Support Library.  Such roperties, methods, and events will
//  be defined here.
//

//
//  Define the Automation Table for the transport pin.
//
//
DEFINE_KSAUTOMATION_TABLE(TransportAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//
//  Dispatch Table for the transport Output pin.
//
//  Since data on the transport is actually delivered to the
//  PCI bridge in hardware, this pin does not process data.
//
//  Connection of, and state transitions on, this pin help the
//  driver to determine when to allocate hardware resources for
//  each node.
//
const
KSPIN_DISPATCH
TransportPinDispatch =
{
    CTransportPin::PinCreate,     // Create
    CTransportPin::PinClose,      // Close
    NULL,                               // Process
    NULL,                               // Reset
    NULL,                               // SetDataFormat
    NULL,                               //SetDeviceState
    NULL,                               // Connect
    NULL,                               // Disconnect
    NULL,                               // Clock
    NULL                                // Allocator
};


//
//  Format the output Transport Stream Connection
//
//  Used to connect the output pin to a specific downstream filter
//
const KS_DATARANGE_BDA_TRANSPORT TransportPinRange =
{
   // insert the KSDATARANGE and KSDATAFORMAT here
    {
        sizeof( KS_DATARANGE_BDA_TRANSPORT),               // FormatSize
        0,                                                 // Flags - (N/A)
        0,                                                 // SampleSize - (N/A)
        0,                                                 // Reserved
        { STATIC_KSDATAFORMAT_TYPE_STREAM },               // MajorFormat
        { STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT },      // SubFormat
        { STATIC_KSDATAFORMAT_SPECIFIER_BDA_TRANSPORT }    // Specifier
    },
    // insert the BDA_TRANSPORT_INFO here
    {
        188,        //  ulcbPhyiscalPacket
        312 * 188,  //  ulcbPhyiscalFrame
        0,          //  ulcbPhyiscalFrameAlignment (no requirement)
        0           //  AvgTimePerFrame (not known)
    }
};

//  Format Ranges of Transport Output Pin.
//
static PKSDATAFORMAT TransportPinRanges[] =
{
    (PKSDATAFORMAT) &TransportPinRange,

    // Add more formats here if additional transport formats are supported.
    //
};


//  Medium GUIDs for the Transport Output Pin.
//
//  This insures contection to the correct Capture Filter pin.
//
// {F102C41F-7FA1-4842-A0C8-DC41176EC844}
#define GUID_BdaSWRcv   0xf102c41f, 0x7fa1, 0x4842, 0xa0, 0xc8, 0xdc, 0x41, 0x17, 0x6e, 0xc8, 0x44
const KSPIN_MEDIUM TransportPinMedium =
{
    GUID_BdaSWRcv, 0, 0
};


//===========================================================================
//
//  Filter  definitions
//
//  Define arrays here that contain the types of nodes and pins that are possible for the filter 
//  Define structures here that describe Properties, Methods, and Events available on the filter
//
//  Properties can be used to set and retrieve information for the filter.
//  Methods can be used to perform operations on the filter. 
//
//===========================================================================

//
//  Template Node Descriptors
//
//  Define an array that contains all the node types that are available in the template
//  topology of the filter.
//  These node types must be supported by BDA and 
//  defined elsewhere (for example, in Bdamedia.h).
//
const
KSNODE_DESCRIPTOR
NodeDescriptors[] =
{
    {
        &SampleTunerNodeAutomation, // Point to KSAUTOMATION_TABLE structure for the node's automation table
        &KSNODE_BDA_RF_TUNER, // Point to the guid that defines function of the node
        NULL // Point to the guid that represents the name of the topology node
    },
#if ATSC_RECEIVER
    {
        &SampleDemodulatorNodeAutomation, // Point to KSAUTOMATION_TABLE 
        &KSNODE_BDA_8VSB_DEMODULATOR, // Point to the guid that defines the topology node
        NULL // Point to the guid that represents the name of the topology node
    }
#elif DVBS_RECEIVER
    {
        &SampleDemodulatorNodeAutomation, // Point to KSAUTOMATION_TABLE 
        &KSNODE_BDA_QPSK_DEMODULATOR, // Point to the guid that defines the topology node
        NULL // Point to the guid that represents the name of the topology node
    }
#elif DVBT_RECEIVER
    {
        &SampleDemodulatorNodeAutomation, // Point to KSAUTOMATION_TABLE 
        &KSNODE_BDA_COFDM_DEMODULATOR, // Point to the guid that defines the topology node
        NULL // Point to the guid that represents the name of the topology node
    }
#elif CABLE_RECEIVER
    {
        &SampleDemodulatorNodeAutomation, // Point to KSAUTOMATION_TABLE 
        &KSNODE_BDA_QAM_DEMODULATOR, // Point to the guid that defines the topology node
        NULL // Point to the guid that represents the name of the topology node
    }
#endif
};


//
//  Initial Pin Descriptors
//
//  This data structure defines the pins that will appear on the filer
//  when it is first created.
//
//  All BDA filters should expose at lease one input pin to insure that
//  the filter can be properly inserted in a BDA receiver graph.  The
//  initial pins can be created in a number of ways.
//
//  The initial filters descriptor passed in to BdaInitFilter can include
//  a list of n pin descriptors that correspond to the first n of m pin
//  descriptors in the template filter descriptor.  This list of pin
//  descriptors will usually only include those input pins that are
//  ALWAYS exposed by the filter in question.
//
//  Alternatively, the driver may call BdaCreatePin from its filter Create
//  dispatch function for each pin that it ALWAYS wants exposed.
//
//  This filter uses an initial filter descriptor to force the Antenna
//  input pin to always be exposed.
//
const
KSPIN_DESCRIPTOR_EX
InitialPinDescriptors[] =
{
    //  Antenna Input Pin
    //
    {
        &AntennaPinDispatch,   // Point to the dispatch table for the input pin
        &AntennaAutomation,   // Point to the automation table for the input pin
        {  // Specify members of a KSPIN_DESCRIPTOR structure for the input pin
            0,  // Interfaces
            NULL,
            0,  // Mediums
            NULL,
            SIZEOF_ARRAY(AntennaPinRanges),
            AntennaPinRanges,  
            KSPIN_DATAFLOW_IN,  // specifies that data flow is into the pin
            KSPIN_COMMUNICATION_BOTH, // specifies that the pin factory instantiates pins 
                                                                               // that are both IRP sinks and IRP sources
            NULL,   //  Category GUID
            NULL,   // GUID of the localized Unicode string name for the pin type
            0
        },  // Specify flags
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,  // Specify the maximum number of possible instances of the input pin
        1,      // Specify the number of instances of this pin type that are necessary for proper functioning of this filter
        NULL,   // Point to KSALLOCATOR_FRAMING_EX structure for allocator framing
        CAntennaPin::IntersectDataFormat    // Point to the data intersection handler function
    }
};


//
//  Template Pin Descriptors
//
//  This data structure defines the pin types available in the filters
//  template topology.  These structures will be used to create a
//  KDPinFactory for a pin type when BdaCreatePin or BdaMethodCreatePin
//  are called.
//
//  This structure defines ALL pins the filter is capable of supporting,
//  including those pins which may only be created dynamically by a ring
//  3 component such as a Network Provider.
//
//
const
KSPIN_DESCRIPTOR_EX
TemplatePinDescriptors[] =
{
    //  Antenna Input Pin
    //
    {
        &AntennaPinDispatch,   // Point to the dispatch table for the input pin
        &AntennaAutomation,   // Point to the automation table for the input pin
        {  // Specify members of a KSPIN_DESCRIPTOR structure for the input pin
            0,  // Interfaces
            NULL,
            0,  // Mediums
            NULL,
            SIZEOF_ARRAY(AntennaPinRanges),
            AntennaPinRanges,  
            KSPIN_DATAFLOW_IN,  // specifies that data flow is into the pin
            KSPIN_COMMUNICATION_BOTH, // specifies that the pin factory instantiates pins 
                                                                               // that are both IRP sinks and IRP sources
            NULL,   //  Category GUID
            NULL,   // GUID of the localized Unicode string name for the pin type
            0
        },  // Specify flags
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,  // Specify the maximum number of possible instances of the input pin
        1,  // Specify the number of instances of this pin type that are necessary for proper functioning of this filter
        NULL,   // Point to KSALLOCATOR_FRAMING_EX structure for allocator framing
        CAntennaPin::IntersectDataFormat    // Point to the data intersection handler function
    },

    //  Tranport Output Pin
    //
    {
        &TransportPinDispatch,   // Point to the dispatch table for the output pin
        &TransportAutomation,   // Point to the automation table for the output pin
        {  // Specify members of a KSPIN_DESCRIPTOR structure for the output pin
            0,  // Interfaces
            NULL,
            1,  // Mediums
            &TransportPinMedium,
            SIZEOF_ARRAY(TransportPinRanges),
            TransportPinRanges,
//            0,
//            NULL,
            KSPIN_DATAFLOW_OUT, // specifies that data flow is out of the pin
            KSPIN_COMMUNICATION_BOTH, // specifies that the pin factory instantiates pins 
                                                                               // that are both IRP sinks and IRP sources
//            NULL,//Name
//            NULL,//Category
            (GUID *) &PINNAME_BDA_TRANSPORT,   //  Category GUID
            (GUID *) &PINNAME_BDA_TRANSPORT,   // GUID of the localized Unicode string 
                                                                                              // name for the pin type
            0
        },  // Specify flags
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
            KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
            KSPIN_FLAG_FIXED_FORMAT,
        1,  // Specify the maximum number of possible instances of the output pin
        0,  // Specify the number of instances of this pin type that are necessary for proper functioning of this filter
        NULL,   // Allocator Framing
        CTransportPin::IntersectDataFormat    // Point to the data intersection handler function
    }
};


//
//  BDA Device Topology Property Set
//
//  The BDA Support Library supplies a default implementation of the
//  BDA Device Topology Property Set.  If the driver needs to override
//  this default implemenation, the definitions for the override properties
//  will be defined here.
//


//
//  BDA Change Sync Method Set
//
//  The Change Sync Method Set is required on BDA filters.  Setting a
//  node property should not become effective on the underlying device
//  until CommitChanges is called.
//
//  The BDA Support Library provides routines that handle committing
//  changes to topology.  The BDA Support Library routines should be
//  called from the driver's implementation before the driver implementation
//  returns.
//
DEFINE_KSMETHOD_TABLE(BdaChangeSyncMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_START_CHANGES(
        CFilter::StartChanges, // Calls BdaStartChanges
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_CHECK_CHANGES(
        CFilter::CheckChanges, // Calls BdaCheckChanges
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_COMMIT_CHANGES(
        CFilter::CommitChanges, // Calls BdaCommitChanges
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_GET_CHANGE_STATE(
        CFilter::GetChangeState, // Calls BdaGetChangeState
        NULL
        )
};


//  Override the standard pin medium property set so that we can provide
//  device specific medium information.
//
//  Because the property is on a Pin Factory and not on a pin instance,
//  this is a filter property.
//
DEFINE_KSPROPERTY_TABLE( SampleFilterPropertyOverrides)
{

    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(
        CFilter::GetMedium
        )
};

DEFINE_KSPROPERTY_SET_TABLE(SampleFilterPropertySets)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Pin,                            // Property Set GUID
        SIZEOF_ARRAY(SampleFilterPropertyOverrides), // Number of Properties
        SampleFilterPropertyOverrides,               // Array of KSPROPERTY_ITEM structures 
        0,                                           // FastIoCount
        NULL                                         // FastIoTable
    )

    //
    //  Additional property sets for the filter can be added here.
    //
};


//
//  BDA Device Configuration Method Set
//
//  The BDA Support Library provides a default implementation of
//  the BDA Device Configuration Method Set.  In this example, the
//  driver overrides the CreateTopology method.  Note that the
//  support libraries CreateTopology method is called before the
//  driver's implementation returns.
//
DEFINE_KSMETHOD_TABLE(BdaDeviceConfigurationMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_CREATE_TOPOLOGY(
        CFilter::CreateTopology, // Calls BdaMethodCreateTopology
        NULL
        )
};


//
//  Define an array of method sets that the filter supports
//
DEFINE_KSMETHOD_SET_TABLE(FilterMethodSets)
{
    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaChangeSync,       // Method set GUID
        SIZEOF_ARRAY(BdaChangeSyncMethods), // Number of methods
        BdaChangeSyncMethods,               // Array of KSMETHOD_ITEM structures 
        0,                                  // FastIoCount
        NULL                                // FastIoTable
    ),

    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaDeviceConfiguration,       // Method set GUID
        SIZEOF_ARRAY(BdaDeviceConfigurationMethods), // Number of methods
        BdaDeviceConfigurationMethods,  // Array of KSMETHOD_ITEM structures 
        0,                                           // FastIoCount
        NULL                                         // FastIoTable
    )
};


//
//  Filter Automation Table
//
//  Lists all Property, Method, and Event Set tables for the filter
//
DEFINE_KSAUTOMATION_TABLE(FilterAutomation) {
    //DEFINE_KSAUTOMATION_PROPERTIES(SampleFilterPropertySets),
    DEFINE_KSAUTOMATION_PROPERTIES_NULL,
    DEFINE_KSAUTOMATION_METHODS(FilterMethodSets),
    DEFINE_KSAUTOMATION_EVENTS_NULL
};


//
//  Filter Dispatch Table
//
//  Lists the dispatch routines for major events at the filter
//  level.
//
const
KSFILTER_DISPATCH
FilterDispatch =
{
    CFilter::Create,        // Create
    CFilter::FilterClose,   // Close
    NULL,                   // Process
    NULL                    // Reset
};

//
//  Define the name GUID for our digital tuner filter.
//
//  NOTE!  You must use a different GUID for each type of filter that
//  your driver exposes.
//
#define STATIC_KSNAME_BdaSWTunerFilter\
    0x91b0cc87L, 0x9905, 0x4d65, 0xa0, 0xd1, 0x58, 0x61, 0xc6, 0xf2, 0x2c, 0xbf
DEFINE_GUIDSTRUCT("91B0CC87-9905-4d65-A0D1-5861C6F22CBF", KSNAME_BdaSWTunerFilter);
#define KSNAME_BdaSWTunerFilter DEFINE_GUIDNAMED(KSNAME_BdaSWTunerFilter)

//  Must match the KSSTRING used in the installation INFs interface sections
//  AND must match the KSNAME GUID above.
//
#define KSSTRING_BdaSWTunerFilter L"{91B0CC87-9905-4d65-A0D1-5861C6F22CBF}"


//
//  Define the Filter Factory Descriptor for the filter
//
//  This structure brings together all of the structures that define
//  the tuner filter as it appears when it is first instantiated.
//  Note that not all of the template pin and node types may be exposed as
//  pin and node factories when the filter is first instanciated.
//
//  If a driver exposes more than one filter, each filter must have a
//  unique ReferenceGuid.
//
DEFINE_KSFILTER_DESCRIPTOR(InitialFilterDescriptor)
{
    &FilterDispatch,        // Dispatch
    &FilterAutomation,  // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,  // Version
    0,                                 // Flags
    &KSNAME_BdaSWTunerFilter,  // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(InitialPinDescriptors), 
                                       // PinDescriptorsCount; must expose at least one pin
                                       // PinDescriptorSize; size of each item
                                       // PinDescriptors; table of pin descriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_NETWORK_TUNER),
                                       // CategoriesCount; number of categories in the table
                                       // Categories; table of categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,              
                                       // NodeDescriptorsCount; in this case, 0
                                       // NodeDescriptorSize; in this case, 0
                                       // NodeDescriptors; in this case, NULL
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,
    // Automatically fills in the connections table for a filter which defines no explicit connections
                                       // ConnectionsCount; number of connections in the table
                                       // Connections; table of connections
    NULL                        // ComponentId; in this case, no ID is provided
};


//===========================================================================
//
//  Define Filter Template Topology 
//
//===========================================================================

//
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
const
KSTOPOLOGY_CONNECTION TemplateFilterConnections[] =
{   // KSFILTER_NODE  is defined as ((ULONG)-1) in ks.h 
     // Indicate pin types by the item number is the TemplatePinDescriptors array.
     // Indicate node types by either the item number in the NodeDescriptors array
     // or the element in the BDA_SAMPLE_NODE enumeration.
    { KSFILTER_NODE, 0,                 BDA_SAMPLE_TUNER_NODE, 0},
    { BDA_SAMPLE_TUNER_NODE, 1,         BDA_SAMPLE_DEMODULATOR_NODE, 0},
    { BDA_SAMPLE_DEMODULATOR_NODE, 1,   KSFILTER_NODE, 1}
};


//
//  Template Joints between the Antenna and Transport Pin Types.
//
//  Lists the template joints between the Antenna Input Pin Type and
//  the Transport Output Pin Type.
//
//  In this case the RF Node is considered to belong to the antennea input
//  pin and the 8VSB Demodulator Node is considered to belong to the
//  tranport stream output pin.
//
const
ULONG   AntennaTransportJoints[] =
{
    1 // joint occurs between the two node types (second element in array)
       // indicates that 1st node is controlled by input pin and 2nd node by output pin
};

//
//  Define Template Pin Parings.
//
//  Array of BDA_PIN_PAIRING structures that are used to determine 
//  which nodes get duplicated when more than one output pin type is 
//  connected to a single input pin type or when more that one input pin 
//  type is connected to a single output pin type.
//  
const
BDA_PIN_PAIRING TemplatePinPairings[] =
{
    //  Input pin to Output pin Topology Joints
    //
    {
        0,  // ulInputPin; 0 element in the TemplatePinDescriptors array.
        1,  // ulOutputPin; 1 element in the TemplatePinDescriptors array.
        1,  // ulcMaxInputsPerOutput
        1,  // ulcMinInputsPerOutput
        1,  // ulcMaxOutputsPerInput
        1,  // ulcMinOutputsPerInput
        SIZEOF_ARRAY(AntennaTransportJoints),   // ulcTopologyJoints
        AntennaTransportJoints   // pTopologyJoints; array of joints
    }
    //  If applicable, list topology of joints between other pins.
    //
};


//
//  Define the Filter Factory Descriptor that the BDA support library uses 
//  to create template topology for the filter.
//
//  This KSFILTER_DESCRIPTOR structure combines the structures that 
//  define the topologies that the filter can assume as a result of
//  pin factory and topology creation methods.
//  Note that not all of the template pin and node types may be exposed as
//  pin and node factories when the filter is first instanciated.
//
DEFINE_KSFILTER_DESCRIPTOR(TemplateFilterDescriptor)
{
    &FilterDispatch,  // Dispatch
    &FilterAutomation,  // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,  // Version
    0,  // Flags
    &KSNAME_BdaSWTunerFilter,  // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(TemplatePinDescriptors),
                                       // PinDescriptorsCount; exposes all template pins 
                                       // PinDescriptorSize; size of each item
                                       // PinDescriptors; table of pin descriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_NETWORK_TUNER),
                                       // CategoriesCount; number of categories in the table
                                       // Categories; table of categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),  
                                      // NodeDescriptorsCount; exposes all template nodes
                                      // NodeDescriptorSize; size of each item
                                      // NodeDescriptors; table of node descriptors
    DEFINE_KSFILTER_CONNECTIONS(TemplateFilterConnections), 
                                      // ConnectionsCount; number of connections in the table
                                       // Connections; table of connections
    NULL                        // ComponentId; in this case, no ID is provided
};

//
//  Define BDA Template Topology Descriptor for the filter.
//
//  This structure combines the filter descriptor and pin pairings that
//  the BDA support library uses to create an instance of the filter.
//
const
BDA_FILTER_TEMPLATE
BdaFilterTemplate =
{
    &TemplateFilterDescriptor,
    SIZEOF_ARRAY(TemplatePinPairings),
    TemplatePinPairings
};


//===========================================================================
//
//  Define the Device 
//
//===========================================================================


//
//  Define Device Dispatch Table
//
//  List the dispatch routines for the major events that occur 
//  during the existence of the underlying device.
//
extern
const
KSDEVICE_DISPATCH
DeviceDispatch =
{
    CDevice::Create,    // Add
    CDevice::Start,     // Start
    NULL,               // PostStart
    NULL,               // QueryStop
    NULL,               // CancelStop
    NULL,               // Stop
    NULL,               // QueryRemove
    NULL,               // CancelRemove
    NULL,               // Remove
    NULL,               // QueryCapabilities
    NULL,               // SurpriseRemoval
    NULL,               // QueryPower
    NULL                // SetPower
};


//
//  Define Device Descriptor
//
//  Combines structures that define the device and any non-BDA
//  intial filter factories that can be created on it.
//  Note that this structure does not include the template topology
//  structures as they are specific to BDA.
//
extern
const
KSDEVICE_DESCRIPTOR
DeviceDescriptor =
{
    &DeviceDispatch,    // Dispatch
    0,      // SIZEOF_ARRAY( FilterDescriptors),   // FilterDescriptorsCount
    NULL,   // FilterDescriptors                   // FilterDescriptors
};


