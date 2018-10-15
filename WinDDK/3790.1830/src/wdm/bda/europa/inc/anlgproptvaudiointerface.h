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

NTSTATUS AnlgTVAudioCapsGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TVAUDIO_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TVAUDIO_CAPS_S pData
);

NTSTATUS AnlgTVAudioAvailableModesGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TVAUDIO_S pRequest,
    IN OUT PKSPROPERTY_TVAUDIO_S pData
);

NTSTATUS AnlgTVAudioModeGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TVAUDIO_S pRequest,
    IN OUT PKSPROPERTY_TVAUDIO_S pData
);

NTSTATUS AnlgTVAudioModeSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TVAUDIO_S pRequest,
    IN OUT PKSPROPERTY_TVAUDIO_S pData
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Description of the TV audio filter properties corresponding to the
//  property set PROPSETID_VIDCAP_TVAUDIO.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(AnlgTVAudioFilterPropertyTable)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_CAPS,                        // 1
        AnlgTVAudioCapsGetHandler,                      // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_TVAUDIO_CAPS_S),              // MinProperty
        sizeof(KSPROPERTY_TVAUDIO_CAPS_S),              // MinData
        NULL,                                           // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES,   // 1
        AnlgTVAudioAvailableModesGetHandler,            // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_TVAUDIO_S),                   // MinProperty
        sizeof(KSPROPERTY_TVAUDIO_S),                   // MinData
        NULL,                                           // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_MODE,                        // 1
        AnlgTVAudioModeGetHandler,                      // GetSupported or
                                                        // Handler
        sizeof(KSPROPERTY_TVAUDIO_S),                   // MinProperty
        sizeof(KSPROPERTY_TVAUDIO_S),                   // MinData
        AnlgTVAudioModeSetHandler,                      // SetSupported or
                                                        // Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    )
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  TV audio filter properties table
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_SET_TABLE(AnlgTVAudioFilterPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_TVAUDIO,                      // Set
        SIZEOF_ARRAY(AnlgTVAudioFilterPropertyTable),   // PropertiesCount
        AnlgTVAudioFilterPropertyTable,                 // PropertyItems
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    )
};

