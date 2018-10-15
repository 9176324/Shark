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

#include "AnlgEventHandler.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Base Filter class. Used to collect common function interfaces on capture 
//  filters.
//
//////////////////////////////////////////////////////////////////////////////
class CBaseFilter
{
public:

    // Operations
    virtual CAnlgEventHandler* GetEventHandler() = 0;
};

