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
//  This is the video decoder property class.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgPropVideoDecoder : public CBaseClass
{
public:
    CAnlgPropVideoDecoder();
    ~CAnlgPropVideoDecoder();

    NTSTATUS DecoderCapsGetHandler( IN PIRP pIrp,
                                IN PKSPROPERTY pRequest,
                                IN OUT PKSPROPERTY_VIDEODECODER_CAPS_S pData);

    NTSTATUS StandardGetHandler(IN PIRP pIrp,
                                IN PKSPROPERTY pRequest,
                                IN OUT PKSPROPERTY_VIDEODECODER_S pData);

    NTSTATUS StandardSetHandler(IN PIRP pIrp,
                                IN PKSPROPERTY pRequest,
                                IN OUT PKSPROPERTY_VIDEODECODER_S pData);

    NTSTATUS StatusGetHandler(IN PIRP pIrp,
                              IN PKSPROPERTY pRequest,
                              IN OUT PKSPROPERTY_VIDEODECODER_STATUS_S pData);

private:

    BOOL  m_bOutputEnableFlag;
    DWORD m_dwDecoderStandard;
    DWORD m_dwDefaultStandard;
    DWORD m_dwDefaultNumberOfLines;
    BOOL  m_bDefaultSignalLocked;
};
