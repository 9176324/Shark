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

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Defines the number of TS sttream buffers.
//
//////////////////////////////////////////////////////////////////////////////
#define NUM_TS_STREAM_BUFFER 5

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Defines the number of bytes for one transport packet.
//
//////////////////////////////////////////////////////////////////////////////
#define M2T_BYTES_IN_LINE     188 //must be smaller than 255


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Defines the number of transport packets per field
//
//////////////////////////////////////////////////////////////////////////////
#define M2T_LINES_IN_FIELD    312


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Defines the buffer size as a result of M2T_BYTES_IN_LINE and
//  M2T_LINES_IN_FIELD.
//
//////////////////////////////////////////////////////////////////////////////
#define M2T_BUFFERSIZE M2T_LINES_IN_FIELD * M2T_BYTES_IN_LINE//188*312

NTSTATUS DgtlTransportOutCreate
(
    IN PKSPIN pKSPin,
    IN PIRP pIrp
);

NTSTATUS DgtlTransportOutRemove
(
    IN PKSPIN pKSPin,
    IN PIRP pIrp
);

NTSTATUS DgtlTransportOutProcess
(
    IN PKSPIN pKSPin
);

NTSTATUS DgtlTransportOutSetDeviceState
(
    IN PKSPIN pKSPin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
);

NTSTATUS DgtlTransportOutAllocatorPropertyHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN OUT PKSALLOCATOR_FRAMING pAllocator
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Dispatch Table for the Transport Stream Output Pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DISPATCH DgtlTransportOutDispatch =
{
    DgtlTransportOutCreate,         // Create
    DgtlTransportOutRemove,         // Remove
    DgtlTransportOutProcess,        // Process
    NULL,                           // Reset
    NULL,                           // SetDataFormat
    DgtlTransportOutSetDeviceState, // SetDeviceState
    NULL,                           // Connect
    NULL,                           // Disconnect
    NULL,                           // Clock
    NULL                            // Allocator
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Defines the dispatch routines for the framing allocator
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(DgtlTransportOutAllocatorProperty)
{
    DEFINE_KSPROPERTY_ITEM_CONNECTION_ALLOCATORFRAMING(
        DgtlTransportOutAllocatorPropertyHandler
        )
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The output pin property table defines all property sets supported by the
//  output pin exposed by this filter.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_SET_TABLE(DgtlTransportOutPropertySet)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Connection,                         // Set
        SIZEOF_ARRAY(DgtlTransportOutAllocatorProperty), // PropertiesCount
        DgtlTransportOutAllocatorProperty,               // PropertyItems
        0,                                               // FastIoCount
        NULL                                             // FastIoTable
    )
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Lists all Properties, Methods, and Events for the transport output pin.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSAUTOMATION_TABLE(DgtlTransportOutAutomation)
{
    DEFINE_KSAUTOMATION_PROPERTIES(DgtlTransportOutPropertySet),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the format and data range for the transport output pin.
//
//////////////////////////////////////////////////////////////////////////////
const KS_DATARANGE_BDA_TRANSPORT DgtlTransportOutRange =
{
      {
         sizeof( KSDATARANGE ),             // FormatSize
         0,                                 // Flags
         M2T_BUFFERSIZE,                    // SampleSize = 58656
         0,                                 // Reserved
         { STATIC_KSDATAFORMAT_TYPE_STREAM },
                                            // MEDIASUBTYPE_MPEG2_TRANSPORT
         { STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT },
         { STATIC_KSDATAFORMAT_SPECIFIER_NONE },
      }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Lists all format and data ranges of transport output pin.
//
//////////////////////////////////////////////////////////////////////////////
static PKSDATAFORMAT DgtlTransportOutRanges[] =
{
    (PKSDATAFORMAT) &DgtlTransportOutRange,

    // Add more formats here if additional transport formats are supported.
    //
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the pin descriptor for the transport output pin. It contains
//  all the definitions and structure like dispatch table, automation table,
//  data ranges etc.
//
//////////////////////////////////////////////////////////////////////////////
#define DGTL_TRANSPORT_OUT_PIN_DESCRIPTOR                               \
{                                                                       \
    &DgtlTransportOutDispatch,                /* Dispatch           */  \
    &DgtlTransportOutAutomation,              /* AutomationTable    */  \
    {                                                                   \
        0,                                    /* Interfaces count   */  \
        NULL,                                 /* Interfaces         */  \
        0,                                    /* Mediums count      */  \
        NULL,                                 /* Mediums            */  \
        SIZEOF_ARRAY(DgtlTransportOutRanges), /* Range Count        */  \
        DgtlTransportOutRanges,               /* Ranges             */  \
        KSPIN_DATAFLOW_OUT,                   /* Dataflow           */  \
        KSPIN_COMMUNICATION_BOTH,             /* Communication      */  \
        &PINNAME_BDA_TRANSPORT,               /* Category           */  \
        &PINNAME_BDA_TRANSPORT,               /* Name               */  \
        0                                     /* Reserved           */  \
    },                                                                  \
    KSPIN_FLAG_FIXED_FORMAT |                                           \
    KSPIN_FLAG_GENERATE_MAPPINGS,             /* Flags              */  \
    1,                                        /* Instances Possible */  \
    1,                                        /* Instances Necessary*/  \
    NULL,                                     /* Allocator Framing  */  \
    NULL                                      /* IntersectHandler   */  \
}


