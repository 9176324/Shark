//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
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

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampIo | This class encapsulates the entire register area as well
//           the access to them. Furthermore it initializes all register
//           classes and register field classes during construction. 
// @end
// Filename: vampio.h
// 
// Routines/Functions:
//
//  public:
//          CVampIo
//          ~CVampIo
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VAMPIO_H__D1C1ABA0_6207_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPIO_H__D1C1ABA0_6207_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MunitGpio.h"
#include "MunitDma.h"
#include "MunitComDec.h"
#include "MunitScaler.h"
#include "MunitSc2Dma.h"
#include "MunitAudio.h"
#include "DeviceIo.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampIo | Memory units of the chip including the complete register 
//         set.
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CVampIo  
{
    //@access private
private:
    // @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@access public 
public:
    //@cmember Constructor.<nl> 
    //Parameterlist:<nl>
    //PBYTE pBaseAddress // base address of the register space<nl>
    CVampIo( 
        PBYTE pBaseAddress );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor. 
    virtual ~CVampIo();

    //@cmember Scaler memory unit class
    CMunitScaler MunitScaler;
    //@cmember Decoder memory unit class
    CMunitComDec MunitComDec;
    //@cmember Audio memory unit class
    CMunitAudio MunitAudio;
    //@cmember GPIO memory unit class
    CMunitGpio MunitGpio;
    //@cmember DMA memory unit class
    CMunitDma MunitDma;
    //@cmember Scaler to DMA memory unit class
    CMunitSc2Dma MunitSc2Dma;
    //@cmember General access to PCI space
    CDeviceIo   DeviceIo;
};



#endif // !defined(AFX_VAMPIO_H__D1C1ABA0_6207_11D3_A66F_00E01898355B__INCLUDED_)
