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
// @module   VampPanicCB | Virtual methods for the interrupt callbacks. These 
//           will be overloaded in the driver.
// @end
// Filename: VampPanicCB.h
//
// Routines/Functions:
//
//  public:
//    OnPCIAbortISR
//    OnLoadErrorISR
//    OnConfigErrorISR
//    OnTrigErrorISR
//    OnPWRFailureISR
//    OnPCIParityErrorISR
//
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPPANICCB_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPPANICCB_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampPanicCallBacks | Virtual interrupt callback methods.
// @end
//
//////////////////////////////////////////////////////////////////////////////


class CVampPanicCallBacks
{
//@access public 
public:

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a PCI abort signal. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnPCIAbortISR() = NULL;

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a load error. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnLoadErrorISR() = NULL;

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a configuration error. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnConfigErrorISR() = NULL;

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a trigger error. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnTrigErrorISR() = NULL;

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a power failure. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnPWRFailureISR() = NULL;

    //@cmember This method will be called from the CVampPanicManager object during an
    //interrupt and is reflecting a PCI parity error. It needs to be 
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnPCIParityErrorISR() = NULL;
};
#endif
