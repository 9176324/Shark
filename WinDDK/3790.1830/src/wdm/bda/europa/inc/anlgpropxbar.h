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
//
// Description:
//
//  This class contains the implementation of the Crossbar Filter properties.
//  The PROPSETID_VIDCAP_CROSSBAR property set is used to get or set
//  information pertaining to routing video and audio signals. The crossbar
//  is modeled after a general switching matrix with N inputs and M outputs.
//  Any of the input signals can be routed to one or more of the outputs.
//  A single crossbar can route both video and audio signals. Video pins
//  optionally can indicate an audio pin that is related to the video pin.
//  The following properties are members on the PROPSETID_VIDCAP_CROSSBAR
//  property set:
//      KSPROPERTY_CROSSBAR_CANROUTE        GetHandler
//      KSPROPERTY_CROSSBAR_CAPS            GetHandler
//      KSPROPERTY_CROSSBAR_PININFO         GetHandler
//      KSPROPERTY_CROSSBAR_ROUTE           GetHandler/SetHandler
//////////////////////////////////////////////////////////////////////////////
class CAnlgPropXBar : public CBaseClass
{
public:
    CAnlgPropXBar();
    ~CAnlgPropXBar();

    NTSTATUS XBarCapsGetHandler(IN PIRP pIrp,
                                IN PKSPROPERTY_CROSSBAR_CAPS_S pRequest,
                                IN OUT PKSPROPERTY_CROSSBAR_CAPS_S pData);

    NTSTATUS XBarPinInfoGetHandler(
                                IN PIRP pIrp,
                                IN PKSPROPERTY_CROSSBAR_PININFO_S pRequest,
                                IN OUT PKSPROPERTY_CROSSBAR_PININFO_S pData);

    NTSTATUS XBarCanRouteGetHandler(
                                IN PIRP pIrp,
                                IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
                                IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData);

    NTSTATUS XBarRouteGetHandler(
                                IN PIRP pIrp,
                                IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
                                IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData);

    NTSTATUS XBarRouteSetHandler(
                                IN PIRP pIrp,
                                IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,
                                IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData);

private:

    DWORD   m_dwRoutedVideoInPinIndex;
    DWORD   m_dwRoutedAudioInPinIndex;

    DWORD   m_dwNumOfInputPins;
    DWORD   m_dwNumOfOutputPins;

    DWORD   m_dwVideoTunerInIndex;
    DWORD   m_dwVideoCompositeInIndex;
    DWORD   m_dwVideoSVideoInIndex;

    DWORD   m_dwAudioTunerInIndex;
    DWORD   m_dwAudioCompositeInIndex;
    DWORD   m_dwAudioSVideoInIndex;

    DWORD   m_dwVideoOutIndex;
    DWORD   m_dwAudioOutIndex;
};
