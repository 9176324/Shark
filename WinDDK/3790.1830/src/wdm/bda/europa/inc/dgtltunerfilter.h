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

#include "DgtlPropTuner.h"
#include "DgtlMethodsTuner.h"
#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class initializes the Digital Tuner Filter and creates the
//  CDgtlPropTuner class. The pointer to the CDgtlPropTuner class is returned
//  by a public member function.
//
//////////////////////////////////////////////////////////////////////////////
class CDgtlTunerFilter: public CBaseClass
{
public:
    CDgtlTunerFilter();
    ~CDgtlTunerFilter();

    NTSTATUS Create(IN PKSFILTER pKSFilter);
    NTSTATUS Close (IN PKSFILTER pKSFilter);

    CDgtlPropTuner*     GetPropTuner();
    CDgtlMethodsTuner*  GetMethodsTuner();
    KSSTATE             GetClientState();
    VOID                SetClientState(KSSTATE ClientState);

private:
    CDgtlPropTuner*     m_pDgtlPropTuner;
    CDgtlMethodsTuner*  m_pDgtlMethodsTuner;
    KSSTATE             m_TunerOutClientState;
};
