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

#include "VampDevice.h"

class CAnlgEventHandler;

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Base Stream pin of the Analog Capture Filter.
//  This class contains the virtual declaration of the common Interface for
//  all Analog Capture Filter Pins (Capture/Preview/VBI).
//  In detail this Interface provides functions to create and remove the
//  pin. The streaming handling is defined here too (open close, start, stop).
//
//////////////////////////////////////////////////////////////////////////////
class CBaseStream : public CVampListEntry, public CCallOnDPC
{
public:
        // Construction
    CBaseStream();

    // Implementation
    virtual ~CBaseStream();

    virtual NTSTATUS Create(IN PKSPIN,IN PIRP) = 0;
    virtual NTSTATUS Remove(IN PIRP) = 0;

    // Operations
    virtual NTSTATUS Open()     = 0;
    virtual NTSTATUS Start()    = 0;
    virtual NTSTATUS Stop()     = 0;
    virtual NTSTATUS Close()    = 0;
    virtual NTSTATUS Process()  = 0;

    CAnlgEventHandler* GetEventHandler();
    PKSPIN GetKSPin();

protected:
    // Attributes
    PKSPIN m_pKSPin;
};

