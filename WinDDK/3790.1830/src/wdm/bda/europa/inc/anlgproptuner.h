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

#include "BaseClass.h"

#define EUROPA_DEFAULT_COUNTRYCODE 33 
#define EUROPA_DEFAULT_CHANNEL 39

#define EUROPA_MIN_FREQUENCY 471250000
#define EUROPA_MIN_FREQUENCY_1316 87000000
#define EUROPA_MAX_FREQUENCY 855250000
#define EUROPA_ANALOG_FREQUENCY 615250000

#define EUROPA_TUNING_GRANULARITY 62500
#define EUROPA_SETTLING_TIME 1000 //milli seconds

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the Analog Tuner Filter
//  properties.
//////////////////////////////////////////////////////////////////////////////
class CAnlgPropTuner : public CBaseClass
{
public:
    CAnlgPropTuner();
    ~CAnlgPropTuner();

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

private:

    NTSTATUS TuneToFrequency
    (
        CVampDeviceResources* pVampDevResObj
    );

    NTSTATUS SaveDetectedStandardToRegistry
    (
        IN PKSFILTER pKSFilter,
        IN DWORD  dwStandard
    );

    DWORD       m_dwAnalogFrequency;
    DWORD       m_dwTuningFlags;
    DWORD       m_dwChannel;
    DWORD       m_dwCountry;
    DWORD       m_dwStandard;
    ASearchFlag m_dwSearchFlag;
	DWORD m_dwMinFrequency;
	DWORD m_dwMaxFrequency;
	DWORD m_nStandardsSupported;
};

