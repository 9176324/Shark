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

#include "AnlgPropVideoDecoder.h"
#include "AnlgPropProcamp.h"
#include "AnlgEventHandler.h"
#include "BaseFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Class of the analog capture filter object.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgCaptureFilter : public CBaseFilter
{
public:
    CAnlgCaptureFilter();
    ~CAnlgCaptureFilter();

    NTSTATUS Create(IN PKSFILTER pKSFilter);
    NTSTATUS Close (IN PKSFILTER pKSFilter);

    CAnlgPropVideoDecoder*  GetPropVideoDecoder();
    CAnlgPropProcamp*       GetPropProcamp();
    CAnlgEventHandler*      GetEventHandler();

private:
    CAnlgPropVideoDecoder*  m_pAnlgPropVideoDecoder;
    CAnlgPropProcamp*       m_pAnlgPropProcamp;
    CAnlgEventHandler*      m_pAnlgEventHandler;
};
