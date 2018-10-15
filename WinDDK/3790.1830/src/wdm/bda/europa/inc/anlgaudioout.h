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

#include "VampDeviceResources.h"
#include "VampAudioStream.h"
#include "BaseStream.h"
#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Analog audio out pin of the Analog Capture Filter.
//  This class contains the implementation of the audio pin. Within this
//  class the pin can be created and removed. The data format can be set and
//  the streaming functionality is implemented here (open close, start,
//  stop...). The HW informs this class for each filled buffer and this class
//  returns this buffer to the system.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgAudioOut : public CBaseStream, public CBaseClass
{
public:
    CAnlgAudioOut();
    ~CAnlgAudioOut();

    //dispatch routines
    NTSTATUS Create(IN PKSPIN, IN PIRP pIrp);
    NTSTATUS Remove(IN PIRP pIrp);

    //stream state routines
    NTSTATUS Open   ();
    NTSTATUS Start  ();
    NTSTATUS Stop   ();
    NTSTATUS Close  ();
    NTSTATUS Process();

    //HAL callback function
    VAMPRET CallOnDPC(PVOID, PVOID, PVOID);
private:
    CVampAudioStream*  m_pVampAudioStream;
    PIKSREFERENCECLOCK m_pIKsClock;
    COSDependSpinLock* m_pSpinLockObj;
	DWORD m_dwDMASize;
};
