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
// @module   CVampFifoIrq | Interrupt handling for the Fifo's.<nl>
// @end
// Filename: VampFiFoIrq.h
// 
// Routines/Functions:
//
//  public:
//
//      GetObjectStatus
// 
//  private:
//
//  protected:
//
//      OnDPC
//      EnableIrq
//      DisableIrq
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_VampFiFoIrq_H__INCLUDED_)
#define AFX_VampFiFoIrq_H__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampTypes.h"
#include "VampError.h"
#include "OSDepend.h"

class CVampCoreStream;


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampFiFoIrq | Base class for FiFo interrupt handling.
// @end
//////////////////////////////////////////////////////////////////////////////
class CVampFiFoIrq
{
//@access private
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access protected
protected:
    //@cmember Array of (lost) interrupt elements which include a pointer
	//to a core stream object.
    tFifoIntElem  m_FifoIntElem[NUM_OF_FIFO_INT_CONDITIONS];

    //@cmember Flag field of Core stream object, which requesting a DPC.
    BYTE m_cDPCRequests;

	//@cmember Flag indicating whether the ISR's list is valid.
	BOOL m_bISRSListValid;

    //@cmember Loops through its list of core streams and calls
    //CCoreStream::OnDPC(). This calls the stream's registered callback. 
    VAMPRET OnDPC();

    //@cmember Pure function definition, overloaded by CVampIrq. By adding an ISR,
    //the specific interrupt will be automatically enabled.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //eFifoArea eArea // RAM area<nl>
    virtual VAMPRET EnableIrq (
        DWORD dwIntCond,
        eFifoArea eArea ) = 0;

    //@cmember Pure function definition, overloaded by CVampIrq. By removing a ISR,
    //the specific interrupt will be automatically disabled.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //eFifoArea eArea // RAM area<nl>
    virtual VAMPRET DisableIrq (
        DWORD dwIntCond,
        eFifoArea eArea ) = 0;

// @access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //eFifoArea eArea // RAM area<nl>
    CVampFiFoIrq( 
        eFifoArea eArea );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampFiFoIrq();

};

#endif // !defined(AFX_VampFiFoIrq_H__INCLUDED_)
