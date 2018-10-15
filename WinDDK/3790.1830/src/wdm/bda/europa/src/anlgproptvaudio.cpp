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


#include "34AVStrm.h"
#include "Device.h"
#include "VampDevice.h"
#include "AnlgPropTVAudio.h"
#include "AnlgPropTVAudioInterface.h"
#include "Mediums.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTVAudio::CAnlgPropTVAudio()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTVAudio::~CAnlgPropTVAudio()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the TV audio capabilities of the device
//  (capabilities (Bitmask of KS_TVAUDIO_MODE_*) and the input and output
//  medium of the TVAudio) in the pData parameter. The available modes are
//  retrieved from the hardware.
// Settings:
//  The hardware is capable of
//      KS_TVAUDIO_MODE_MONO
//      KS_TVAUDIO_MODE_STEREO
//      KS_TVAUDIO_MODE_LANG_A
//      KS_TVAUDIO_MODE_LANG_B
//
// Return Value:
//  STATUS_SUCCESS          The TV audio capabilities of the device returned
//                          with success.
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          if the function parameters are zero,
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTVAudio::AnlgTVAudioCapsGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_CAPS_S pRequest, // Pointer to
                                            // PKSPROPERTY_TVAUDIO_CAPS_S
    IN OUT PKSPROPERTY_TVAUDIO_CAPS_S pData // Pointer to
                                            // PKSPROPERTY_TVAUDIO_CAPS_S
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // This should be device dependent.
    // The SAA7134 has no function to ask for that.
    pData->Capabilities =   KS_TVAUDIO_MODE_MONO    |
                            KS_TVAUDIO_MODE_STEREO  |
                            KS_TVAUDIO_MODE_LANG_A  |
                            KS_TVAUDIO_MODE_LANG_B;

    PKSFILTER pFilter = KsGetFilterFromIrp(pIrp);
    if (!pFilter)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get filter from Irp"));
        return STATUS_UNSUCCESSFUL;
    }
    PKSDEVICE pKSDevice = KsFilterGetDevice(pFilter);
    if (!pKSDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get device from filter"));
        return STATUS_UNSUCCESSFUL;
    }
    CVampDevice *pVampDevice = (CVampDevice *) pKSDevice->Context;
    if (!pVampDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get device context"));
        return STATUS_UNSUCCESSFUL;
    }
    UINT uiMediumId = pVampDevice->GetDriverMediumInstanceCount();

    pData->InputMedium  = AnlgTVAudioInMedium[0];
    pData->InputMedium.Id = uiMediumId;

    pData->OutputMedium = AnlgXBarTunerAudioInMedium[0];
    pData->OutputMedium.Id = uiMediumId;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the available TV audio modes of the device
//  (available modes (Bitmask of KS_TVAUDIO_MODE_*) in the pData parameter.
//  HAL access is performed by this function.
// Settings:
//  The mode can be one of the following KS_TVAUDIO_MODE_Xxx values
//  (or a combination of the values):
//  Value Meaning
//  KS_TV_AUDIO_MODE_MONO   Audio supports mono.
//  KS_TVAUDIO_MODE_STEREO  Audio supports stereo.
//  KS_TVAUDIO_MODE_LANG_A  Audio supports the primary language.
//  KS_TVAUDIO_MODE_LANG_B  Audio supports the second language.
//  The available mode is retrieved from the hardware format according
//  to the following table:
//  Mode                                              Vamp
//  KS_TV_AUDIO_MODE_MONO                             FORMAT_MONO
//  KS_TV_AUDIO_MODE_MONO  and KS_TVAUDIO_MODE_STEREO FORMAT_STEREO
//  KS_TVAUDIO_MODE_LANG_A and
//  KS_TVAUDIO_MODE_LANG_B and KS_TVAUDIO_MODE_MONO   FORMAT_CHANNEL_A
//  KS_TVAUDIO_MODE_LANG_A and
//  KS_TVAUDIO_MODE_LANG_B and KS_TVAUDIO_MODE_MONO   FORMAT_CHANNEL_B
//  KS_TV_AUDIO_MODE_MONO                             other
//
// Return Value:
//  STATUS_SUCCESS          The available TVaudio modes of the device retuned
//                          with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) hardware access or link failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTVAudio::AnlgTVAudioAvailableModesGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    AFormat AudioFormat = pVampDevResObj->m_pAudioCtrl->GetFormat();
    switch(AudioFormat)
    {
    case FORMAT_MONO:
        pData->Mode = KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found mono"));
        break;
    case FORMAT_STEREO:
        pData->Mode =   KS_TVAUDIO_MODE_MONO    |
                        KS_TVAUDIO_MODE_STEREO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found stereo"));
        break;
    case FORMAT_CHANNEL_A:
        pData->Mode =   KS_TVAUDIO_MODE_LANG_A  |
                        KS_TVAUDIO_MODE_LANG_B | KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found a"));
        break;
    case FORMAT_CHANNEL_B:
        pData->Mode =   KS_TVAUDIO_MODE_LANG_A  |
                        KS_TVAUDIO_MODE_LANG_B | KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found b"));
        break;
    case FORMAT_UNDEFINED:
    default:
        pData->Mode = KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found nothing?"));
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the TV audio mode of the device (KS_TVAUDIO_MODE_*)
//  in the pData parameter.
//  HAL access is performed by this function.
// Settings:
//  The mode can be one of the following KS_TVAUDIO_MODE_Xxx values
//  (or a combination of the values):
//  Value Meaning
//  KS_TV_AUDIO_MODE_MONO   Audio supports mono.
//  KS_TVAUDIO_MODE_STEREO  Audio supports stereo.
//  KS_TVAUDIO_MODE_LANG_A  Audio supports the primary language.
//  KS_TVAUDIO_MODE_LANG_B  Audio supports the second language.
//  KS_TVAUDIO_MODE_LANG_C  Audio supports the third language.
//  The mode is retrieved from the hardware format according
//  to the following table:
//  Mode                                              Vamp
//  KS_TV_AUDIO_MODE_MONO                             FORMAT_MONO
//  KS_TVAUDIO_MODE_STEREO                            FORMAT_STEREO
//  KS_TVAUDIO_MODE_LANG_A and KS_TVAUDIO_MODE_MONO   FORMAT_CHANNEL_A
//  KS_TVAUDIO_MODE_LANG_B and KS_TVAUDIO_MODE_MONO   FORMAT_CHANNEL_B
//  KS_TV_AUDIO_MODE_MONO                             other
//
// Return Value:
//  STATUS_SUCCESS          The TV audio mode of the device has been returned
//                          with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) hardware access or link failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTVAudio::AnlgTVAudioModeGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    AFormat AudioFormat = pVampDevResObj->m_pAudioCtrl->GetFormat();
    switch(AudioFormat)
    {
    case FORMAT_MONO:
        pData->Mode = KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found mono"));
        break;
    case FORMAT_STEREO:
        pData->Mode = KS_TVAUDIO_MODE_STEREO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found stereo"));
        break;
    case FORMAT_CHANNEL_A:
        pData->Mode = KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found a"));
        break;
    case FORMAT_CHANNEL_B:
        pData->Mode = KS_TVAUDIO_MODE_LANG_B | KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found b"));
        break;
    case FORMAT_UNDEFINED:
    default:
        pData->Mode = KS_TVAUDIO_MODE_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: found nothing"));
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set the TV audio mode of the device (KS_TVAUDIO_MODE_*)
//  in the pData parameter.
//  HAL access is performed by this function.
// Settings:
//  The mode can be one of the following KS_TVAUDIO_MODE_Xxx values:
//  Value Meaning
//  KS_TV_AUDIO_MODE_MONO   Audio supports mono.
//  KS_TVAUDIO_MODE_STEREO  Audio supports stereo.
//  KS_TVAUDIO_MODE_LANG_A  Audio supports the primary language.
//  KS_TVAUDIO_MODE_LANG_B  Audio supports the second language.
//  KS_TVAUDIO_MODE_LANG_C  Audio supports the third language.
//  The hardware format is set according to the mode to be set.
//  to the following table:
//  Mode                                            Vamp
//  KS_TV_AUDIO_MODE_MONO                           FORMAT_MONO
//  KS_TVAUDIO_MODE_STEREO                          FORMAT_STEREO
//  KS_TVAUDIO_MODE_LANG_A and KS_TVAUDIO_MODE_MONO FORMAT_CHANNEL_A
//  KS_TVAUDIO_MODE_LANG_B and KS_TVAUDIO_MODE_MONO FORMAT_CHANNEL_B
//  other                                           FORMAT_MONO
//
// Return Value:
//  STATUS_SUCCESS          Set the TV audio mode of the device with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) hardware access or link failed
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTVAudio::AnlgTVAudioModeSetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //check current process handle
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    CVampDevice *pVampDevice = GetVampDevice(pKsFilter);
    PEPROCESS pCurrentProcessHandle = pVampDevice->GetOwnerProcessHandle();
    if( (pCurrentProcessHandle != NULL) && (PsGetCurrentProcess() != pCurrentProcessHandle) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Set Handler was called from unregistered process"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    AFormat AudioFormat = FORMAT_MONO;
    switch(pRequest->Mode)
    {
    case KS_TVAUDIO_MODE_MONO:
        AudioFormat = FORMAT_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: set mono"));
        break;
    case KS_TVAUDIO_MODE_STEREO:
        AudioFormat = FORMAT_STEREO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: set stereo"));
        break;
    case KS_TVAUDIO_MODE_LANG_A | KS_TVAUDIO_MODE_MONO:
        AudioFormat = FORMAT_CHANNEL_A;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: set a"));
        break;
    case KS_TVAUDIO_MODE_LANG_B | KS_TVAUDIO_MODE_MONO:
        AudioFormat = FORMAT_CHANNEL_B;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: set b"));
        break;
    default:
        AudioFormat = FORMAT_MONO;
        _DbgPrintF(DEBUGLVL_BLAB,("Info: set nothing"));
    }
    VAMPRET vResult = pVampDevResObj->m_pAudioCtrl->SetFormat(AudioFormat);
    if( vResult != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot set audio format"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

