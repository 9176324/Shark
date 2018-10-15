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

//COFDM properties

NTSTATUS COFDMDemodulatorNodeStartHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY Request,
    IN OUT PVOID pData
);

NTSTATUS COFDMDemodulatorNodeStopHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY Request,
    IN OUT PVOID pData
);

DEFINE_KSPROPERTY_TABLE(COFDMDemodulatorAutodemodulate)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_AUTODEMODULATE_START,        // 1
        COFDMDemodulatorNodeStartHandler,           // GetSupported or Handler
        sizeof(KSPROPERTY),                         // MinProperty
        0,                                          // MinData
        NULL,                                       // SetSupported or Handler
        NULL,                                       // Values
        0,                                          // RelationsCount
        NULL,                                       // Relations
        NULL,                                       // SupportHandler
        0                                           // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_AUTODEMODULATE_STOP,         // 1
        COFDMDemodulatorNodeStopHandler,            // GetSupported or Handler
        sizeof(KSPROPERTY),                         // MinProperty
        0,                                          // MinData
        NULL,                                       // SetSupported or Handler
        NULL,                                       // Values
        0,                                          // RelationsCount
        NULL,                                       // Relations
        NULL,                                       // SupportHandler
        0                                           // SerializedSize
    )
};

DEFINE_KSPROPERTY_SET_TABLE(COFDMDemodulatorNodeProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaAutodemodulate,             // Set
        SIZEOF_ARRAY(COFDMDemodulatorAutodemodulate),// PropertiesCount
        COFDMDemodulatorAutodemodulate,             // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    )
};

//RF Tuner node properties

NTSTATUS RFTunerNodeGetFrequencyMultiplier
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PDWORD pData
);

NTSTATUS RFTunerNodeSetFrequencyMultiplier
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PDWORD pData
);

NTSTATUS RFTunerNodeGetFrequency
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PDWORD pData
);

NTSTATUS RFTunerNodeSetFrequency
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PDWORD pData
);

NTSTATUS RFTunerNodeGetSignalStrength
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PLONG pData
);

NTSTATUS RFTunerNodeGetSignalQuality
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PLONG pData
);

NTSTATUS RFTunerNodeGetSignalPresent
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PBOOL pData
);

NTSTATUS RFTunerNodeGetSignalLocked
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PBOOL pData
);

NTSTATUS RFTunerNodeGetSampleTime
(
    IN PIRP Irp,
    IN PKSP_NODE Request,
    IN OUT PLONG pData
);

DEFINE_KSPROPERTY_TABLE(RFTunerBdaFrequencyFilter)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_RF_TUNER_FREQUENCY_MULTIPLIER,   // 1
        RFTunerNodeGetFrequencyMultiplier,              // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(DWORD),                                  // MinData
        RFTunerNodeSetFrequencyMultiplier,              // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_RF_TUNER_FREQUENCY,              // 1
        RFTunerNodeGetFrequency,                        // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(DWORD),                                  // MinData
        RFTunerNodeSetFrequency,                        // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    )
};

DEFINE_KSPROPERTY_TABLE(RFTunerBdaSignalStats)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_SIGNAL_STRENGTH,                 // 1
        RFTunerNodeGetSignalStrength,                   // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(LONG),                                   // MinData
        NULL,                                           // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_SIGNAL_QUALITY,                  // 1
        RFTunerNodeGetSignalQuality,                    // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(LONG),                                   // MinData
        NULL,                                           // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_SIGNAL_PRESENT,                  // 1
        RFTunerNodeGetSignalPresent,                    // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(BOOL),                                   // MinData
        NULL,                                           // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_SIGNAL_LOCKED,                   // 1
        RFTunerNodeGetSignalLocked,                     // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(BOOL),                                   // MinData
        NULL,                                           // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_BDA_SAMPLE_TIME,                     // 1
        RFTunerNodeGetSampleTime,                       // GetSupported or Handler
        sizeof(KSP_NODE),                               // MinProperty
        sizeof(LONG),                                   // MinData
        NULL,                                           // SetSupported or Handler
        NULL,                                           // Values
        0,                                              // RelationsCount
        NULL,                                           // Relations
        NULL,                                           // SupportHandler
        0                                               // SerializedSize
    )
};

DEFINE_KSPROPERTY_SET_TABLE(RFTunerNodeProperties)
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaFrequencyFilter,                // Set
        SIZEOF_ARRAY(RFTunerBdaFrequencyFilter),        // PropertiesCount
        RFTunerBdaFrequencyFilter,                      // PropertyItems
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_BdaSignalStats,                    // Set
        SIZEOF_ARRAY(RFTunerBdaSignalStats),            // PropertiesCount
        RFTunerBdaSignalStats,                          // PropertyItems
        0,                                              // FastIoCount
        NULL                                            // FastIoTable
    )
};
