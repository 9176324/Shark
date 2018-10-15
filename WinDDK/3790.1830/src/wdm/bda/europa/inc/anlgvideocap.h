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

#include "BaseStream.h"
#include "BaseClass.h"

//
// Description:
//  stream input parameters for PAL
//
#define PAL_FRAME_RESOLUTION_X          0
#define PAL_FRAME_RESOLUTION_Y          24
#define PAL_FRAME_RESOLUTION_WIDTH      720
#define PAL_FRAME_RESOLUTION_HEIGHT     288
#define PAL_FRAMERATE                   25
#define PAL_FRACTION                    0

//
// Description:
//  stream input parameters for NTSC
//
#define NTSC_FRAME_RESOLUTION_X         0
#define NTSC_FRAME_RESOLUTION_Y         22
#define NTSC_FRAME_RESOLUTION_WIDTH     720
#define NTSC_FRAME_RESOLUTION_HEIGHT    240
#define NTSC_FRAMERATE                  30
#define NTSC_FRACTION                   0

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Analog video capture out pin of the Analog Capture Filter.
//  This class contains the implementation of the capture pin. Within this
//  class the pin can be created and removed. The data format can be set and
//  the streaming functionality is implemented here (open close, start,
//  stop...). The HW informs this class for each filled buffer and this class
//  returns this buffer to the system.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgVideoCap : public CBaseStream, public CBaseClass
{
public:
    CAnlgVideoCap();
    ~CAnlgVideoCap();

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
    VAMPRET CallOnDPC
    (
        PVOID pStream,
        PVOID pParam1,
        PVOID pParam2
    );


private:
    CVampVideoStream*   m_pVampVDStream;
    PIKSREFERENCECLOCK  m_pIKsClock;
    KS_VIDEOINFOHEADER  m_VideoInfoHeader;
    KS_VIDEOINFOHEADER2 m_VideoInfoHeader2;
    COSDependSpinLock*  m_pSpinLockObj;

    DWORD m_dwPictureNumber;
    DWORD m_dwCurrentFormatSize;
};
