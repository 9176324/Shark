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

#include "DgtlTransportIn.h"
#include "DgtlTransportOutInterface.h"

NTSTATUS DgtlCaptureFilterCreate(IN PKSFILTER pKSFilter,
                                 IN PIRP      pIrp);

NTSTATUS DgtlCaptureFilterClose(IN PKSFILTER pKSFilter,
                                IN PIRP      pIrp);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch table for the digital capture filter. It provides
//  notification of creation, closure.
//
//////////////////////////////////////////////////////////////////////////////
const KSFILTER_DISPATCH DgtlCaptureFilterDispatch =
{
    DgtlCaptureFilterCreate,                // Filter Create
    DgtlCaptureFilterClose,                 // Filter Close
    NULL,                                   // Filter Process
    NULL                                    // Filter Reset
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This data structure defines the pins that will appear on the filter
//  when it is created.
//
//////////////////////////////////////////////////////////////////////////////
const KSPIN_DESCRIPTOR_EX TSPinDescriptors[] =
{
    DGTL_TRANSPORT_IN_PIN_DESCRIPTOR,
    DGTL_TRANSPORT_OUT_PIN_DESCRIPTOR
};



//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This item describes the connection topology of the digital capture
//  filter. It shows that the input is directly connected to the output.
//
//////////////////////////////////////////////////////////////////////////////
const KSTOPOLOGY_CONNECTION DgtlCaptureFilterConnections[] =
{
    { KSFILTER_NODE, 0, KSFILTER_NODE, 1 } //no nodes, directly connect input to output
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This structure brings together all of the structures that define
//  the digital capture filter as it appears when it is instanciated.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSFILTER_DESCRIPTOR(DgtlCaptureFilterDescriptor)
{
    &DgtlCaptureFilterDispatch,                         // Dispatch
    NULL,                                               // AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,                        // Version
    0,                                                  // Flags
    &VAMP_DGTL_CAPTURE_FILTER,                              // ReferenceGuid
    DEFINE_KSFILTER_PIN_DESCRIPTORS(TSPinDescriptors),  // PinDescriptors
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_BDA_RECEIVER_COMPONENT),
                                                        // Categories
    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,              // NodeDescriptors
    DEFINE_KSFILTER_CONNECTIONS(DgtlCaptureFilterConnections),
                                                        // Connections
    NULL                                                // ComponentId
};








