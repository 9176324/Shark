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

NTSTATUS AnlgTunerCapsGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_CAPS_S pData
);

NTSTATUS AnlgTunerModeCapsGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_CAPS_S pData
);

NTSTATUS AnlgTunerModeGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
);

NTSTATUS AnlgTunerModeSetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
);

NTSTATUS AnlgTunerStandardGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
);

NTSTATUS AnlgTunerStandardSetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
);

NTSTATUS AnlgTunerFrequencyGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
);

NTSTATUS AnlgTunerFrequencySetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
);

NTSTATUS AnlgTunerInputGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
);

NTSTATUS AnlgTunerInputSetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
);

NTSTATUS AnlgTunerStatusGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_STATUS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STATUS_S pData
);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Description of the analog tuner filter properties corresponding to the
//  property set PROPSETID_TUNER.
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_TABLE(AnlgTunerFilterPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_CAPS,						    // 1
		AnlgTunerCapsGetHandler,						// GetSupported or 
														// Handler
		sizeof(KSPROPERTY_TUNER_CAPS_S),				// MinProperty
		sizeof(KSPROPERTY_TUNER_CAPS_S),				// MinData
		NULL,											// SetSupported or 
														// Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0									            // SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_MODE_CAPS,						// 1
		AnlgTunerModeCapsGetHandler,					// GetSupported or 
														// Handler
		sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),			// MinProperty
		sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),			// MinData
		NULL,											// SetSupported or 
														// Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_MODE,							// 1
		AnlgTunerModeGetHandler,						// GetSupported or 
														// Handler
		sizeof(KSPROPERTY_TUNER_MODE_S),				// MinProperty
		sizeof(KSPROPERTY_TUNER_MODE_S),				// MinData
		AnlgTunerModeSetHandler,						// SetSupported or 
														// Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_STANDARD,						// 1
		AnlgTunerStandardGetHandler,					// GetSupported or Handler
		sizeof(KSPROPERTY_TUNER_STANDARD_S),			// MinProperty
		sizeof(KSPROPERTY_TUNER_STANDARD_S),			// MinData
		AnlgTunerStandardSetHandler,					// SetSupported or Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_FREQUENCY,						// 1
		AnlgTunerFrequencyGetHandler,					// GetSupported or Handler
		sizeof(KSPROPERTY_TUNER_FREQUENCY_S),			// MinProperty
		sizeof(KSPROPERTY_TUNER_FREQUENCY_S),			// MinData
		AnlgTunerFrequencySetHandler,					// SetSupported or Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_INPUT,							// 1
		AnlgTunerInputGetHandler,						// GetSupported or 
														// Handler
		sizeof(KSPROPERTY_TUNER_INPUT_S),				// MinProperty
		sizeof(KSPROPERTY_TUNER_INPUT_S),				// MinData
		AnlgTunerInputSetHandler,						// SetSupported or 
														// Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM								
	(
		KSPROPERTY_TUNER_STATUS,						// 1
		AnlgTunerStatusGetHandler,						// GetSupported or 
														// Handler
		sizeof(KSPROPERTY_TUNER_STATUS_S),				// MinProperty
		sizeof(KSPROPERTY_TUNER_STATUS_S),				// MinData
		NULL,											// SetSupported or 
														// Handler
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		NULL,											// SupportHandler
		0												// SerializedSize
	)
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Analog Tuner filter properties table
//
//////////////////////////////////////////////////////////////////////////////
DEFINE_KSPROPERTY_SET_TABLE(AnlgTunerFilterPropertySetTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_TUNER,							// Set
        SIZEOF_ARRAY(AnlgTunerFilterPropertyTable), // PropertiesCount
        AnlgTunerFilterPropertyTable,               // PropertyItems
        0,                                          // FastIoCount
        NULL                                        // FastIoTable
    )
};
