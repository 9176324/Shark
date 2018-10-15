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

//////////////////////////////////////////////////////////////////////////////
// Description:
//
//  This class contains the implementation of the TVAudio Filter properties.
//  The PROPSETID_VIDCAP_TVAUDIO property set is used to control settings
//  unique to audio from television sources. This includes secondary audio
//  program (SAP), and stereo or mono selection. These controls are generally
//  found on devices external to the system audio mixer.
//  The following properties are members of the PROPSETID_VIDCAP_TVAUDIO
//  property set:
//      KSPROPERTY_TVAUDIO_CAPS
//      KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES
//      KSPROPERTY_TVAUDIO_MODE
//////////////////////////////////////////////////////////////////////////////
class CAnlgPropTVAudio : public CBaseClass
{
public:
    CAnlgPropTVAudio();
    ~CAnlgPropTVAudio();


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

private:

};
