/**************************************************************************

    AVStream Simulated Hardware Sample

    Copyright (c) 2001, Microsoft Corporation.

    File:

        filter.cpp

    Abstract:

        This file contains the filter level implementation for the 
        capture filter.

    History:

        created 3/12/2001

**************************************************************************/

#include "BDACap.h"

/**************************************************************************

    PAGEABLE CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA


NTSTATUS
CCaptureFilter::
DispatchCreate (
    IN PKSFILTER Filter,
    IN PIRP Irp
    )

/*++

Routine Description:

    This is the creation dispatch for the capture filter.  It creates
    the CCaptureFilter object, associates it with the AVStream filter
    object, and bag the CCaptureFilter for later cleanup.

Arguments:

    Filter -
        The AVStream filter being created

    Irp -
        The creation Irp

Return Value:
    
    Success / failure

--*/

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CCaptureFilter *CapFilter = new (NonPagedPool, 'RysI') CCaptureFilter (Filter);

    if (!CapFilter) {
        //
        // Return failure if we couldn't create the filter.
        //
        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {
        //
        // Add the item to the object bag if we we were successful. 
        // Whenever the filter closes, the bag is cleaned up and we will be
        // freed.
        //
        Status = KsAddItemToObjectBag (
            Filter -> Bag,
            reinterpret_cast <PVOID> (CapFilter),
            reinterpret_cast <PFNKSFREE> (CCaptureFilter::Cleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapFilter;
        } else {
            Filter -> Context = reinterpret_cast <PVOID> (CapFilter);
        }

    }

    return Status;

}

/**************************************************************************

    DESCRIPTOR AND DISPATCH LAYOUT

**************************************************************************/

GUID g_PINNAME_VIDEO_CAPTURE = {STATIC_PINNAME_VIDEO_CAPTURE};

//
// CaptureFilterCategories:
//
// The list of category GUIDs for the capture filter.
//
const
GUID
CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT] = {
    STATICGUIDOF (KSCATEGORY_BDA_RECEIVER_COMPONENT)
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

//
// CaptureFilterPinDescriptors:
//
// The list of pin descriptors on the capture filter.  
//
const 
KSPIN_DESCRIPTOR_EX
CaptureFilterPinDescriptors [CAPTURE_FILTER_PIN_COUNT] = {
    //
    // Capture Input Pin
    //
    {
        &InputPinDispatch,
        NULL,             
        {
            NULL,                           // Interfaces (NULL, 0 == default)
            0,
            1,  // Mediums
            &TransportPinMedium,
            SIZEOF_ARRAY (CaptureInPinDataRanges), // Range Count
            CaptureInPinDataRanges,           // Ranges
            KSPIN_DATAFLOW_IN,              // Dataflow
            KSPIN_COMMUNICATION_BOTH,       // Communication
            NULL,         // Category
            NULL,         // Name
            0                               // Reserved
        },
        
        KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT | 
        KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING | 
        KSPIN_FLAG_FIXED_FORMAT,
        1,      // InstancesPossible
        1,      // InstancesNecessary
        NULL,   // Allocator Framing
        NULL    // PinIntersectHandler
    },

    //
    // Capture Output Pin
    //
    {
        &CapturePinDispatch,
        NULL,             
        {
            NULL,                           // Interfaces (NULL, 0 == default)
            0,
            NULL,                           // Mediums (NULL, 0 == default)
            0,
            SIZEOF_ARRAY (CaptureOutPinDataRanges), // Range Count
            CaptureOutPinDataRanges,           // Ranges
            KSPIN_DATAFLOW_OUT,             // Dataflow
            KSPIN_COMMUNICATION_BOTH,       // Communication
            NULL,         // Category
            NULL,         // Name
            0                               // Reserved
        },
        
        KSPIN_FLAG_GENERATE_MAPPINGS |      // Pin Flags
        KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,
        1,                                  // Instances Possible
        1,                                  // Instances Necessary
        &CapturePinAllocatorFraming,        // Allocator Framing
        NULL                                // Format Intersect Handler
    }
    
};

//
// CaptureFilterDispatch:
//
// This is the dispatch table for the capture filter.  It provides notification
// of creation, closure, processing (for filter-centrics, not for the capture
// filter), and resets (for filter-centrics, not for the capture filter).
//
const 
KSFILTER_DISPATCH
CaptureFilterDispatch = {
    CCaptureFilter::DispatchCreate,         // Filter Create
    NULL,                                   // Filter Close
    NULL,                                   // Filter Process
    NULL                                    // Filter Reset
};

//
//  Define the name GUID for our digital capture filter.
//
//  NOTE!  You must use a different GUID for each type of filter that
//  your driver exposes.
//
#define STATIC_KSNAME_BdaSWCaptureFilter\
    074649feL, 0x2dd8, 0x4c12, 0x8a, 0x23, 0xbd, 0x82, 0x8b, 0xad, 0xff, 0xfa
DEFINE_GUIDSTRUCT("074649FE-2DD8-4C12-8A23-BD828BADFFFA", KSNAME_BdaSWCaptureFilter);
#define KSNAME_BdaSWCaptureFilter DEFINE_GUIDNAMED(KSNAME_BdaSWCaptureFilter)


//  Must match the KSSTRING used in the installation INFs interface sections
//  AND must match the KSNAME GUID above.
//
#define KSSTRING_BdaSWCaptureFilter L"{074649FE-2DD8-4C12-8A23-BD828BADFFFA}"

// Create a connection through the capture filter so that graph render will
// work.
//
const
KSTOPOLOGY_CONNECTION FilterConnections[] =
{   // KSFILTER_NODE  is defined as ((ULONG)-1) in ks.h 
    { KSFILTER_NODE, 0,                 KSFILTER_NODE, 1 }
};

//
// CaptureFilterDescription:
//
// The descriptor for the capture filter.
//
const 
KSFILTER_DESCRIPTOR 
CaptureFilterDescriptor = {
    &CaptureFilterDispatch,                 // Dispatch Table
    NULL,                                   // Automation Table
    KSFILTER_DESCRIPTOR_VERSION,            // Version
    0,                                      // Flags
    &KSNAME_BdaSWCaptureFilter,             // Reference GUID

    // Pin Descriptor Table
    //
    // PinDescriptorsCount; exposes all template pins 
    // PinDescriptorSize; size of each item
    // PinDescriptors; table of pin descriptors
    //
    DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors),

    // Category Table
    //
    // CategoriesCount; number of categories in the table
    // Categories; table of categories
    //
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),

    //  Node Descriptor Table
    //
    // NodeDescriptorsCount; exposes all template nodes
    // NodeDescriptorSize; size of each item
    // NodeDescriptors; table of node descriptors
    //
    0,
    sizeof (KSNODE_DESCRIPTOR),
    NULL,

    //  Filter connection table
    //
    // ConnectionsCount; number of connections in the table
    // Connections; table of connections
    //
    DEFINE_KSFILTER_CONNECTIONS(FilterConnections), 

    NULL                                    // Component ID
};


