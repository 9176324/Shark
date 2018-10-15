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

#include "Europaguids.h"
#include "Mediums.h"
#include "DgtlTransportOutInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Lists the dispatch routines available for the transport input pin
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DISPATCH DgtlTransportInPinDispatch =
{
    NULL,                           // Create
    NULL,                           // Close
    NULL,                           // Process
    NULL,                           // Reset
    NULL,                           // SetDataFormat
    NULL,                           // SetDeviceState
    NULL,                           // Connect
    NULL,                           // Disconnect
    NULL,                           // Clock
    NULL                            // Allocator
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Describes the data range of the transport in pin.
//
//////////////////////////////////////////////////////////////////////////////
const KS_DATARANGE_BDA_TRANSPORT DgtlTransportInPinRange =
{
   // KSDATARANGE
    {
        sizeof( KS_DATARANGE_BDA_TRANSPORT),               // FormatSize
        0,                                                 // Flags - (N/A)
        0,                                                 // SampleSize -
                                                           // (N/A)
        0,                                                 // Reserved
        { STATIC_KSDATAFORMAT_TYPE_STREAM },               // MajorFormat
        { STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT },      // SubFormat
        { STATIC_KSDATAFORMAT_SPECIFIER_BDA_TRANSPORT }    // Specifier
    },

    {
        M2T_BYTES_IN_LINE,  // bytes in line
        M2T_BYTES_IN_LINE * M2T_LINES_IN_FIELD, // size
        0,                  //alignment requirements
        0                   //time per sample
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Format Ranges of Transport Input Pin.
//  Array that lists all pin ranges for the transport input pin
//
//////////////////////////////////////////////////////////////////////////////
const PKSDATAFORMAT DgtlTransportInPinRanges[] =
{
    (PKSDATAFORMAT) &DgtlTransportInPinRange
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Descriptor for the transport input pin, containing all definitions like
//  data range, connections, medium.
//
//////////////////////////////////////////////////////////////////////////////
#define DGTL_TRANSPORT_IN_PIN_DESCRIPTOR                               \
{                                                                      \
    &DgtlTransportInPinDispatch,             /* Dispatch           */  \
    NULL,                                    /* AutomationTable    */  \
    {                                                                  \
        0,                                   /* Interfaces count   */  \
        NULL,                                /* Interfaces         */  \
        SIZEOF_ARRAY(DgtlTransportInPinMedium),/* Mediums count    */  \
        DgtlTransportInPinMedium,            /* Mediums            */  \
        SIZEOF_ARRAY(DgtlTransportInPinRanges),/* Range Count      */  \
        DgtlTransportInPinRanges,            /* Ranges             */  \
        KSPIN_DATAFLOW_IN,                   /* Dataflow           */  \
        KSPIN_COMMUNICATION_BOTH,            /* Communication      */  \
        &PINNAME_BDA_TRANSPORT,              /* Category           */  \
        &PINNAME_BDA_TRANSPORT,              /* Name               */  \
        0                                    /* Reserved           */  \
    },                                                                 \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT        |                  \
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING   |                  \
    KSPIN_FLAG_FIXED_FORMAT,                 /* Flags              */  \
    1,                                       /* Instances Possible */  \
    1,                                       /* Instances Necessary*/  \
    NULL,                                    /* Allocator Framing  */  \
    NULL                                     /* IntersectHandler   */  \
}
