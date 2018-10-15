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

#include "VampTransportStream.h"
#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is the class for the transport output pin
//
//////////////////////////////////////////////////////////////////////////////
class CDgtlTransportOut : public CCallOnDPC, public CBaseClass
{
public:
    CDgtlTransportOut();
    ~CDgtlTransportOut();

    NTSTATUS    Create( IN PKSPIN pKSPin,
                        IN PIRP pIrp);
    NTSTATUS    Remove( IN PKSPIN pKSPin,
                        IN PIRP pIrp);

    NTSTATUS    Open(   IN PKSPIN pKSPin);
    NTSTATUS    Start(  IN PKSPIN pKSPin);
    NTSTATUS    Stop(   IN PKSPIN pKSPin);
    NTSTATUS    Close(  IN PKSPIN pKSPin);
    NTSTATUS    Process(IN PKSPIN pKSPin);

    VAMPRET     CallOnDPC(  PVOID pStream,
                            PVOID pParam1,
                            PVOID pParam2 );
private:
    CVampTransportStream*   m_pVampTSStream;
    COSDependSpinLock*      m_pSpinLockObj;
    PIKSREFERENCECLOCK      m_pIKsClock;
};
