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
#include "AnlgPropXBar.h"
#include "AnlgPropXBarInterface.h"
#include "AnlgXBarFilterInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
// Remarks:
//  The values of the member variables are very strong related to the order
//  of the AnlgXBarFilterPinDescriptors structure. If something changed there
//  these values have to be updated.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropXBar::CAnlgPropXBar()
{
    // Initialize member variables

    // These values are related to the AnlgXBarFilterPinDescriptors structure.
    m_dwNumOfInputPins          = 6;
    m_dwNumOfOutputPins         = 2;

    // These values are the index of the structure
    // AnlgXBarFilterPinDescriptors and are only true if the input video pins,
    // the audio input pins, the video output pins and the audio output pins
    // have exactly this order
    m_dwVideoTunerInIndex       = 0;
    m_dwVideoCompositeInIndex   = 1;
    m_dwVideoSVideoInIndex      = 2;

    m_dwAudioTunerInIndex       = 3;
    m_dwAudioCompositeInIndex   = 4;
    m_dwAudioSVideoInIndex      = 5;

    m_dwVideoOutIndex           = 0;
    m_dwAudioOutIndex           = 1;

    m_dwRoutedVideoInPinIndex = m_dwVideoTunerInIndex;
    m_dwRoutedAudioInPinIndex = m_dwAudioTunerInIndex;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropXBar::~CAnlgPropXBar()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the crossbar capabilities of the device (the number of input and
//  output pins on the crossbar filter).
//
// Return Value:
//  STATUS_SUCCESS          Filter capabilities has been returned
//                          successfully
//  STATUS_UNSUCCESSFUL     Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropXBar::XBarCapsGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_CROSSBAR_CAPS_S pRequest,// Pointer to
                                            // KSPROPERTY_CROSSBAR_CAPS_S
                                            // that describes the property
                                            // request.
    IN OUT PKSPROPERTY_CROSSBAR_CAPS_S pData// Pointer to
                                            // KSPROPERTY_CROSSBAR_CAPS_S
                                            // were the number of input and
                                            // output pins has to be returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    pData->NumberOfInputs = m_dwNumOfInputPins;
    pData->NumberOfOutputs = m_dwNumOfOutputPins;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the type of physical connection represented by the pin (Data
//  Direction Flow /Medium GUIDs/Pin Type). For video pins, this property
//  will also indicate whether or not there is an audio pin associated with
//  a particular video pin.
//
// Remarks:
//  All the returned pData informations are very strong related to the order
//  of the AnlgXBarFilterPinDescriptors structure. If something changed there
//  this function has to be updated.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Pin information of filter has been returned
//                          successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropXBar::XBarPinInfoGetHandler
(
    IN PIRP pIrp,                              // Pointer to IRP
    IN PKSPROPERTY_CROSSBAR_PININFO_S pRequest,// Pointer to
                                               // KSPROPERTY_CROSSBAR_PININFO_S
                                               // that describes the property
                                               // request.
    IN OUT PKSPROPERTY_CROSSBAR_PININFO_S pData// Pointer to
                                               // KSPROPERTY_CROSSBAR_PININFO_S
                                               // were the requested data has
                                               // to be returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

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

    pData->Direction = AnlgXBarFilterPinDescriptors[pRequest->Index].
                       PinDescriptor.DataFlow;

    switch(pRequest->Direction)
    {
    case KSPIN_DATAFLOW_IN:

        pData->Index    = pRequest->Index;
        pData->Medium   = AnlgXBarFilterPinDescriptors[pData->Index].
                          PinDescriptor.Mediums[0];
        pData->Medium.Id = uiMediumId;

        switch(pRequest->Index)
        {
            // Video Input Pins
            case 0:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Video_Tuner);
                pData->RelatedPinIndex = m_dwAudioTunerInIndex;
                break;
            }
            case 1:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Video_Composite);
                pData->RelatedPinIndex = m_dwAudioCompositeInIndex;
                break;
            }
            case 2:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Video_SVideo);
                pData->RelatedPinIndex = m_dwAudioSVideoInIndex;
                break;
            }
            // Audio Input Pins
            case 3:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Audio_Tuner);
                pData->RelatedPinIndex = m_dwVideoTunerInIndex;
                break;
            }
            case 4:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Audio_Line);
                pData->RelatedPinIndex = m_dwVideoCompositeInIndex;
                break;
            }
            case 5:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Audio_Line);
                pData->RelatedPinIndex = m_dwVideoSVideoInIndex;
                break;
            }
            default:
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid request index"));
                return STATUS_UNSUCCESSFUL;
            }
        }
        break;
    case KSPIN_DATAFLOW_OUT:
        pData->Index = pRequest->Index + m_dwNumOfInputPins;
        pData->Medium = AnlgXBarFilterPinDescriptors[pData->Index].
                        PinDescriptor.Mediums[0];
        pData->Medium.Id = uiMediumId;

        switch(pRequest->Index)
        {
            // Video Output Pins
            case 0:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Video_VideoDecoder);
                pData->RelatedPinIndex = m_dwAudioOutIndex;
                break;
            }
            // Audio Output Pins
            case 1:
            {
                pData->PinType =
                        static_cast <ULONG>(KS_PhysConn_Audio_AudioDecoder);
                pData->RelatedPinIndex = m_dwVideoOutIndex;
                break;
            }
            default:
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid request index"));
                return STATUS_UNSUCCESSFUL;
            }
        }
        break;
    default:
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid dataflow direction"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Determines whether the device is capable of supporting a specified
//  routing. E.g. if the tuner video input pin can be routed to the video
//  output pin.
//
// Remarks:
//  This function is very strong related to the order of the
//  AnlgXBarFilterPinDescriptors structure. If something changed there
//  this function has to be updated.

// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Routing information of the filter has been
//                          returned successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropXBar::XBarCanRouteGetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the requested
                                             // combination of input and
                                             // output pin
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the ability of routing
                                             // has to be returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //is unsigned so we do not have to check for smaller than 0
    if( (pRequest->IndexInputPin == 0xffffffff) &&
        (pRequest->IndexOutputPin == m_dwAudioOutIndex) )
    {
        // special case for audio mute
        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }

    //check input pin range
    if( pRequest->IndexInputPin > m_dwAudioSVideoInIndex )
    {
        pData->CanRoute = FALSE;

        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Requested input index out of range"));
        return STATUS_SUCCESS;
    }

    //check output pin range
    if( pRequest->IndexOutputPin > m_dwAudioOutIndex )
    {
        pData->CanRoute = FALSE;

        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Requested output index out of range"));
        return STATUS_SUCCESS;
    }

    if( (pRequest->IndexInputPin <= m_dwVideoSVideoInIndex) &&
        (pRequest->IndexOutputPin == m_dwVideoOutIndex) )
    {
        //video pin to video decoder out
        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }

    if( (pRequest->IndexInputPin > m_dwVideoSVideoInIndex) &&
        (pRequest->IndexOutputPin == m_dwAudioOutIndex) )
    {
        //audio pin to audio decoder out
        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Requested route is not possible"));
    pData->CanRoute = FALSE;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the input pin associated to the requested output pin.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Route of the filter has been returned
//                          successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropXBar::XBarRouteGetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the requested
                                             // output pin
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the associated input
                                             // pin has to be returned
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

    eVideoSource VideoSource = 
        pVampDevResObj->m_pDecoder->GetVideoSource(NULL);

    switch (VideoSource)
    {
    case TUNER:
        m_dwRoutedVideoInPinIndex = m_dwVideoTunerInIndex;
        break;
    case COMPOSITE_1:
        m_dwRoutedVideoInPinIndex = m_dwVideoCompositeInIndex;
        break;
    case S_VIDEO_1:
        m_dwRoutedVideoInPinIndex = m_dwVideoSVideoInIndex;
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Cannot get video source\n"));
        return STATUS_UNSUCCESSFUL;
    }

    AInput AudioSource = 
        pVampDevResObj->m_pAudioCtrl->GetPath(OUT_STREAM);

    switch(AudioSource)
    {
    case IN_AUDIO_TUNER:
        m_dwRoutedAudioInPinIndex = m_dwAudioTunerInIndex;
        break;
    case IN_AUDIO_COMPOSITE_1:
        m_dwRoutedAudioInPinIndex = m_dwAudioCompositeInIndex;
        break;
    case IN_AUDIO_S_VIDEO_1:
        m_dwRoutedAudioInPinIndex = m_dwAudioSVideoInIndex;
        break;
    case IN_AUDIO_DISABLED:
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Cannot get audio source\n"));
        return STATUS_UNSUCCESSFUL;
    }

    switch(pRequest->IndexOutputPin)
    {
    case 0:
        //video output pin
        pData->IndexInputPin = m_dwRoutedVideoInPinIndex;
        break;
    case 1:
        //audio output pin
        pData->IndexInputPin = m_dwRoutedAudioInPinIndex;
        break;
    default:
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid request index"));
        return STATUS_UNSUCCESSFUL;
    };

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Routes a video or audio stream by specifying an output pin index and an
//  input pin index. The HAL is called for setting the audio stream path and
//  the video input source. As a special case, an audio output pin should mute
//  the output audio stream when routed to an input pin index of -1. This is
//  typically used to mute the audio during a channel change.
//
// Remarks:
//  This function is very strong related to the order of the
//  AnlgXBarFilterPinDescriptors structure. If something changed there
//  this function has to be updated.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Routing of the crossbar filter has been set
//                          successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropXBar::XBarRouteSetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP that contains
                                             // the filter object.
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the combination
                                             // of input and output pin to
                                             // route
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the ability of routing
                                             // has to be returned.
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

    if( (pRequest->IndexInputPin == 0xffffffff) &&
        (pRequest->IndexOutputPin == m_dwAudioOutIndex) )
    {
        //set audio to mute
        VAMPRET vSuccess =
            pVampDevResObj->m_pAudioCtrl->SetStreamPath(IN_AUDIO_DISABLED);
        if( vSuccess != VAMPRET_SUCCESS )
        {
            _DbgPrintF(
                    DEBUGLVL_ERROR,
                    ("Error: Failed to set audio stream source to mute"));
            pData->CanRoute = FALSE;
            return STATUS_UNSUCCESSFUL;
        }
        vSuccess =
            pVampDevResObj->m_pAudioCtrl->SetLoopbackPath(IN_AUDIO_DISABLED);
        if( vSuccess != VAMPRET_SUCCESS )
        {
            _DbgPrintF(
                    DEBUGLVL_ERROR,
                    ("Error: Failed to set audio loopback source to mute"));
            pData->CanRoute = FALSE;
            return STATUS_UNSUCCESSFUL;
        }

        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }

    //check input pin range
    if( pRequest->IndexInputPin > m_dwAudioSVideoInIndex )
    {
        pData->CanRoute = FALSE;

        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Requested input index out of range"));
        return STATUS_SUCCESS;
    }

    //check output pin range
    if( pRequest->IndexOutputPin > m_dwAudioOutIndex )
    {
        pData->CanRoute = FALSE;

        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Requested output index out of range"));
        return STATUS_SUCCESS;
    }

    if( (pRequest->IndexInputPin <= m_dwVideoSVideoInIndex) &&
        (pRequest->IndexOutputPin == m_dwVideoOutIndex) )
    {
        //video pin to video decoder out
        eVideoSource VideoSource;
        switch(pRequest->IndexInputPin)
        {
        case 0:
            VideoSource = TUNER;
            m_dwRoutedVideoInPinIndex = m_dwVideoTunerInIndex;
            break;
        case 1:
            VideoSource = COMPOSITE_1;
            m_dwRoutedVideoInPinIndex = m_dwVideoCompositeInIndex;
            break;
        case 2:
            VideoSource = S_VIDEO_1;
            m_dwRoutedVideoInPinIndex = m_dwVideoSVideoInIndex;
            break;
        default:
            VideoSource = TUNER;
            m_dwRoutedVideoInPinIndex = m_dwVideoTunerInIndex;
            break;
        }
        VAMPRET vSuccess =
                pVampDevResObj->m_pDecoder->SetVideoSource(VideoSource);
        if( vSuccess != VAMPRET_SUCCESS )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Failed to set video source"));
            pData->CanRoute = FALSE;
            return STATUS_UNSUCCESSFUL;
        }

        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }

    if( (pRequest->IndexInputPin > m_dwVideoSVideoInIndex) &&
        (pRequest->IndexOutputPin == m_dwAudioOutIndex) )
    {
        //audio pin to audio decoder out
        AInput AudioSource;
        switch(pRequest->IndexInputPin)
        {
        case 3:
            AudioSource = IN_AUDIO_TUNER;
            m_dwRoutedAudioInPinIndex = m_dwAudioTunerInIndex;
            break;
        case 4:
            AudioSource = IN_AUDIO_COMPOSITE_1;
            m_dwRoutedAudioInPinIndex = m_dwAudioCompositeInIndex;
            break;
        case 5:
            AudioSource = IN_AUDIO_S_VIDEO_1;
            m_dwRoutedAudioInPinIndex = m_dwAudioSVideoInIndex;
            break;
        default:
            AudioSource = IN_AUDIO_TUNER;
            m_dwRoutedAudioInPinIndex = m_dwAudioTunerInIndex;
            break;
        }
        VAMPRET vSuccess =
                pVampDevResObj->m_pAudioCtrl->SetStreamPath(AudioSource);
        if( vSuccess != VAMPRET_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                        ("Error: Failed to set audio stream source"));
            pData->CanRoute = FALSE;
            return STATUS_UNSUCCESSFUL;
        }
        vSuccess = pVampDevResObj->m_pAudioCtrl->SetLoopbackPath(AudioSource);
        if( vSuccess != VAMPRET_SUCCESS )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                        ("Error: Failed to set audio loopback source"));
            pData->CanRoute = FALSE;
            return STATUS_UNSUCCESSFUL;
        }

        pData->CanRoute = TRUE;
        return STATUS_SUCCESS;
    }
    _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Requested route is not possible"));
    pData->CanRoute = FALSE;
    return STATUS_SUCCESS;
}
