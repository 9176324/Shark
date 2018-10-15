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

#include "VampVbiStream.h"
#include "BaseStream.h"
#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Analog VBI out pin of the Analog Capture Filter.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgVBIOut : public CBaseStream, public CBaseClass
{
public:
    CAnlgVBIOut();

    ~CAnlgVBIOut();

    //dispatch routines
    NTSTATUS Create(IN PKSPIN, IN PIRP pIrp);
    NTSTATUS Remove(IN PIRP pIrp);

    //stream state routines
    NTSTATUS Open   ();
    NTSTATUS Start  ();
    NTSTATUS Stop   ();
    NTSTATUS Close  ();
    NTSTATUS Process();

    NTSTATUS SignalTvTunerChannelChange
    (
        IN PKS_TVTUNER_CHANGE_INFO pKsChannelChangeInfo
    );

    //HAL callback function

    VAMPRET CallOnDPC
    (
        PVOID pStream,
        PVOID pParam1,
        PVOID pParam2
    );

private:
    // implementation

    // fill in the header structures of the buffer
    // !! used only by CallOnDPC() !!
    NTSTATUS FillBufferHeader(IN OUT PKSSTREAM_HEADER pStreamHeader,
                              IN     CVampBuffer*     pVampBuffer);

    // variables and types
    CVampVbiStream*    m_pVampVBIStream;
    PIKSREFERENCECLOCK m_pIKsClock;
    COSDependSpinLock* m_pSpinLockObj;    // spin lock for buffer-queue access
    KS_VBIINFOHEADER   m_VBIInfoHeader;   // stores the agreed vbi format
    DWORD              m_dwPictureNumber; // counting processed fields
    DWORD              m_dwLostBuffers;   // counting dropped fileds

    KS_TVTUNER_CHANGE_INFO  m_TVTunerChangeInfo; // on tuner change, save the
                                                 // new tuner information
    bool                    m_bNewTunerInfo;     // flag to indicate there are
                                                 // new information available
    bool                    m_bVBIInfoChanged;
};


