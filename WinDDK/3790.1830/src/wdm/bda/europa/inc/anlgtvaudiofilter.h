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

#include "AnlgPropTVAudio.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class initializes the Analog TV audio Filter and creates the
//  CAnlgPropTVAudio class. The pointer to the CAnlgPropTVAudio class is
//  returned by a function, in order to access the TV audio properties.
//  (The access to the Property via the Filter is performed, due to possible
//  multiple property classes within a Filter.)
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgTVAudioFilter
{
public:
    CAnlgTVAudioFilter();
    ~CAnlgTVAudioFilter();

    NTSTATUS Create(IN PKSFILTER pKSFilter);
    NTSTATUS Close (IN PKSFILTER pKSFilter);

    CAnlgPropTVAudio*   GetPropTVAudio();

private:
    CAnlgPropTVAudio*   m_pAnlgPropTVAudio;
};
