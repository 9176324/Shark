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
// This is the procamp class of the analog capture filter
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgPropProcamp : public CBaseClass
{
public:
    CAnlgPropProcamp();
    ~CAnlgPropProcamp();


    NTSTATUS BrightnessGetHandler(  IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS BrightnessSetHandler(  IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS ContrastGetHandler(    IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS ContrastSetHandler(    IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS HueGetHandler(         IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS HueSetHandler(         IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS SaturationGetHandler(  IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS SaturationSetHandler(  IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS SharpnessGetHandler(   IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

    NTSTATUS SharpnessSetHandler(   IN PIRP pIrp,
                                    IN PKSPROPERTY pRequest,
                                    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData);

private:

};
