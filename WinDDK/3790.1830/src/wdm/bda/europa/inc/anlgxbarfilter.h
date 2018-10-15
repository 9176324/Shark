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

#include "AnlgPropXBar.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class initializes the Analog Crossbar Filter and creates the
//  CAnlgPropXBar class. The pointer to the CAnlgPropXBar class is returned
//  by a public member function.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgXBarFilter
{
public:
    CAnlgXBarFilter();
    ~CAnlgXBarFilter();

    NTSTATUS Create(IN PKSFILTER pKSFilter);
    NTSTATUS Close (IN PKSFILTER pKSFilter);

    CAnlgPropXBar*  GetPropXBar();

private:
    CAnlgPropXBar*  m_pAnlgPropXBar;
};
