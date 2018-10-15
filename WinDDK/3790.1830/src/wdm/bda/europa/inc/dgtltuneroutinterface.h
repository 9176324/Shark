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

#include "34AVStrm.h"
#include "DgtlTransportOutInterface.h"

NTSTATUS DgtlTunerOutPinCreate
(
	IN PKSPIN pKSFilter,
    IN PIRP pIrp
);

NTSTATUS DgtlTunerOutPinRemove
(
	IN PKSPIN pKSFilter,
    IN PIRP pIrp
);

NTSTATUS DgtlTunerOutSetDeviceState
(
    IN PKSPIN pKSPin,           // Pointer to KSPIN.
    IN KSSTATE ToState,         // State that has to be active after this call.
    IN KSSTATE FromState        // State that was active before this call.    
);

NTSTATUS DgtlTunerOutIntersectDataFormat
(
    IN PVOID pContext,
    IN PIRP pIrp,
    IN PKSP_PIN pKSPin,
    IN PKSDATARANGE pKSDataRange,
    IN PKSDATARANGE pKSMatchingDataRange,
    IN ULONG ulDataBufferSize,
    OUT OPTIONAL PVOID pData,
    OUT PULONG pulDataSize
);

//
//  Dispatch Table for the digital tuner Output Pin
//
//  This pin does not process any data.  A connection
//  on this pin enables the driver to determine which
//  Tuning Space is connected to the input.  This information
//  may then be used to select a physical antenna jack
//  on the card.
//
const KSPIN_DISPATCH DgtlTunerOutPinDispatch =
{
    DgtlTunerOutPinCreate,     // Create
    DgtlTunerOutPinRemove,     // Remove
    NULL,                      // Process
    NULL,                      // Reset
    NULL,                      // SetDataFormat
    DgtlTunerOutSetDeviceState,// SetDeviceState
    NULL,                      // Connect
    NULL,                      // Disconnect
    NULL,                      // Clock
    NULL                       // Allocator
};

//
//  Format of an Antenna Connection
//
//  Used when connecting the antenna input pin to the Network Provider.
//
const KS_DATARANGE_BDA_TRANSPORT DgtlTunerOutPinRange =
{
   // KSDATARANGE
    {
        sizeof( KS_DATARANGE_BDA_TRANSPORT),// FormatSize
        0,                                  // Flags - (N/A)
        0,                                  // SampleSize - (N/A)
        0,                                  // Reserved
        { STATIC_KSDATAFORMAT_TYPE_STREAM },// MajorFormat
        { STATIC_KSDATAFORMAT_TYPE_MPEG2_TRANSPORT },   // SubFormat
        { STATIC_KSDATAFORMAT_SPECIFIER_BDA_TRANSPORT } // Specifier
    },
    {
        M2T_BYTES_IN_LINE,                                            // bytes in line
        M2T_BYTES_IN_LINE * M2T_LINES_IN_FIELD,                                      // size
        0,                                              // alignment BDArequirement
        0                                               // time per sample
    }
};
//  Format Ranges of Antenna Output Pin.
//
const PKSDATAFORMAT DgtlTunerOutPinRanges[] =
{
    (PKSDATAFORMAT) &DgtlTunerOutPinRange,

    // Add more formats here if additional antenna signal formats are supported.
    //
};

#define DGTL_TUNER_OUT_PIN_DESCRIPTOR                                       \
{                                                                           \
    &DgtlTunerOutPinDispatch,                   /* Dispatch             */  \
    NULL,                                       /* AutomationTable      */  \
    {                                                                       \
        0,                                      /* Interfaces count     */  \
        NULL,                                   /* Interfaces           */  \
        SIZEOF_ARRAY(DgtlTunerOutPinMedium),    /* Mediums count        */  \
        DgtlTunerOutPinMedium,                  /* Mediums              */  \
        SIZEOF_ARRAY(DgtlTunerOutPinRanges),    /* Range Count          */  \
        DgtlTunerOutPinRanges,                  /* Ranges               */  \
        KSPIN_DATAFLOW_OUT,                     /* Dataflow             */  \
        KSPIN_COMMUNICATION_BOTH,               /* Communication        */  \
        &PINNAME_BDA_TRANSPORT,                 /* Category             */  \
        &PINNAME_BDA_TRANSPORT,                 /* Name                 */  \
        0                                       /* Reserved             */  \
    },                                                                      \
    KSPIN_FLAG_DO_NOT_USE_STANDARD_TRANSPORT        |                       \
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING   |                       \
    KSPIN_FLAG_FIXED_FORMAT,                    /* Flags                */  \
    1,                                          /* Instances Possible   */  \
    1,                                          /* Instances Necessary  */  \
    NULL,                                       /* Allocator Framing    */  \
    DgtlTunerOutIntersectDataFormat             /* IntersectHandler     */  \
}
