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

#include "EuropaGuids.h"

NTSTATUS XBarCapsGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_CROSSBAR_CAPS_S pRequest,
    IN OUT PKSPROPERTY_CROSSBAR_CAPS_S pData
);

NTSTATUS XBarPinInfoGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_CROSSBAR_PININFO_S pRequest,
    IN OUT PKSPROPERTY_CROSSBAR_PININFO_S pData
);

NTSTATUS XBarCanRouteGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData
);

NTSTATUS XBarRouteGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData
);

NTSTATUS XBarRouteSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Description of the crossbar filter properties corresponding to the
//  property set PROPSETID_VIDCAP_CROSSBAR.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(AnlgXBarFilterPropertyTable)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAPS,                       // 1
        XBarCapsGetHandler,                             // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),             // MinProperty
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),             // MinData
        NULL,                                           // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        sizeof(ULONG)                                   // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_PININFO,                    // 1
        XBarPinInfoGetHandler,                          // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),          // MinProperty
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),          // MinData
        NULL,                                           // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        sizeof(ULONG)                                   // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAN_ROUTE,                  // 1
        XBarCanRouteGetHandler,                         // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),            // MinProperty
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),            // MinData
        NULL,                                           // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        sizeof(ULONG)                                   // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_ROUTE,                      // 1
        XBarRouteGetHandler,                            // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),            // MinProperty
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),            // MinData
        XBarRouteSetHandler,                            // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        sizeof(ULONG)                                   // SerializedSize
    )
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Crossbar filter properties table
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_SET_TABLE(AnlgXBarFilterPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_CROSSBAR,                 // Set
        SIZEOF_ARRAY(AnlgXBarFilterPropertyTable),  // PropertiesCount
        AnlgXBarFilterPropertyTable,                // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    )
};

