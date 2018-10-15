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
// @module   CVampTrapLostIrq | Interrupt handling for interrupts coming from
// the DMA module. The corresponding callback function will be called.
// @base protected | CVampFiFoIrq
// @end
// Filename: VampTrapLostIrq.h
//
// Routines/Functions:
//
//  public:
//      CVampFiFoIrqOnRA1
//      OnFiFo1
//      AddFiFoComplete1
//      GetObjectStatus
//
//      CVampFiFoIrqOnRA2
//      OnFiFo2
//      AddFiFoComplete2
//      GetObjectStatus
//
//      CVampFiFoIrqOnRA3
//      OnFiFo3
//      AddFiFoComplete3
//      GetObjectStatus
//
//      CVampFiFoIrqOnRA4
//      OnFiFo4
//      AddFiFoComplete4
//      GetObjectStatus
//
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_VampTrapLostIrq_H__INCLUDED_)
#define AFX_VampTrapLostIrq_H__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampTypes.h"
#include "VampError.h"
#include "VampFifoIrq.h"

class CVampCoreStream;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTrapLostIrqRA1 | Interrupt handling class for Fifo 1.<nl>
// @end
//////////////////////////////////////////////////////////////////////////////
class CVampTrapLostIrqRA1 :
    protected CVampFiFoIrq
{
// @access private
private:
	//@cmember The number of video fields to skip for Task A.
	DWORD m_dwSkipARef;

	//@cmember The number of video fields to skip for Task B.
	DWORD m_dwSkipBRef;

	//@cmember The number of video fields to process for Task A.
	DWORD m_dwProcessARef;

	//@cmember The number of video fields to process for TaskB.
	DWORD m_dwProcessBRef;

	//@cmember Indicates whether task A is active or not.
	BOOL m_bTaskAActive;

	//@cmember Indicates whether task B is active or not.
	BOOL m_bTaskBActive;

	//@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access protected
protected:

    //@cmember Handles all 8 interrupt conditions for Video/VBI, TaskA/TaskB
    //          and ODD/EVEN. The task doesn't play a role for VBI, so we have
    //          to handle up to 6 different callbacks.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    VAMPRET OnFiFo1( 
        IN DWORD dwIntCond, 
        IN DWORD dwLostInts );

    //@cmember This is the register method to add or remove an ISR to a specific
    // interrupt event. We may have to handle some missing interrupts due
    // to a very busy CPU, therefore this method builds a ring buffer for
    // plausibility checking. The elements of the buffer are the callback
    // and the interrupt condition. The element must be inserted in the
    // expected order! We store a pointer to the next expected element.
    // If the condition is not the expected one or the HW lost interrupt
    // counter is not zero, then we could step through our list and call the
    // missed OnCompletion methods until we reach the correct element. We
    // have to take care that we don’t run around - store the number of
    // current elements! After inserting the ISR, the interrupt will be
    // automatically enabled, the NULL function pointer will disable the
    // interrupt and remove the condition element from the plausibility
    // list. <nl>Returns: VAMPRET_SUCCESS or VAMPRET_CALLBACK_ALREADY_MAPPED.<nl>
    //Parameterlist:<nl>
    //CVampCoreStream *pCoreStream // core stream pointer<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //DWORD dwFSkip // to skip/process value<nl>
    VAMPRET AddFiFoComplete1( 
        IN CVampCoreStream *pCoreStream, 
        IN DWORD dwIntCond, 
        IN DWORD dwFSkip );

  	//@cmember Handles possible lost interrupts for FIFO 1 and call the other
  	// expected ISR's. Returns the number of lost buffers.<nl>
    //Parameterlist:<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    //DWORD dwIntCond // interrupt condition<nl>
  	DWORD TrapLostInterruptsRA1( 
        IN DWORD dwLostInts, 
        IN DWORD dwIntCond );

// @access public
public:

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //eFifoArea eArea // RAM area<nl>
    CVampTrapLostIrqRA1( 
        IN eFifoArea eArea );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampTrapLostIrqRA1();

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTrapLostIrqRA2 | Interrupt handling class for Fifo 2.<nl>
// @base protected | CVampFiFoIrq
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampTrapLostIrqRA2 :
    protected CVampFiFoIrq
{
//@access private
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access protected
protected:

    //@cmember Handles TaskA/TaskB and ODD/EVEN condition, that means 4 different
    // conditions. No VBI streaming, so we need to handle up to 4 different callbacks.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    VAMPRET OnFiFo2( 
        IN DWORD dwIntCond, 
        IN DWORD dwLostInts );

    //@cmember This is the register method to add or remove an ISR to a specific
    // interrupt event. See note above for AddFiFoComplete1.<nl>
    //Parameterlist:<nl>
    //CVampCoreStream *pCoreStream // core stream pointer<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    VAMPRET AddFiFoComplete2( 
        IN CVampCoreStream *pCoreStream, 
        IN DWORD dwIntCond );

  	//@cmember Handles possible lost interrupts for FIFO 2 and call the other
  	// expected ISR's. Returns the number of lost buffers.<nl>
    //Parameterlist:<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    //DWORD dwIntCond // interrupt condition<nl>
  	DWORD TrapLostInterruptsRA2( 
        IN DWORD dwLostInts, 
        IN DWORD dwIntCond );
    
// @access public
public:

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //eFifoArea eArea // RAM area<nl>
    CVampTrapLostIrqRA2( 
        IN eFifoArea eArea );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampTrapLostIrqRA2();

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTrapLostIrqRA3 | Interrupt handling class for Fifo 3.<nl>
// @base protected | CVampFiFoIrq
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampTrapLostIrqRA3 :
    protected CVampFiFoIrq
{
//@access private
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

//@access protected
protected:
    //@cmember Handles TaskA/TaskB and ODD/EVEN condition, that means 4 different
    // conditions. No VBI streaming, so we need to handle up to 4 different
    // callbacks or two in the case of transport streaming.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    VAMPRET OnFiFo3( 
        IN DWORD dwIntCond, 
        IN DWORD dwLostInts );

    //@cmember This is the register method to add or remove an ISR to a specific
    // interrupt event. See note above for AddFiFoComplete1.<nl>
    //Parameterlist:<nl>
    //CVampCoreStream *pCoreStream // core stream pointer<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    VAMPRET AddFiFoComplete3( 
        IN CVampCoreStream *pCoreStream, 
        IN DWORD dwIntCond );

  	//@cmember Handles possible lost interrupts for FIFO 3 and call the other
  	// expected ISR's. Returns the number of lost buffers.<nl>
    //Parameterlist:<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    //DWORD dwIntCond // interrupt condition<nl>
  	DWORD TrapLostInterruptsRA3( 
        IN DWORD dwLostInts, 
        IN DWORD dwIntCond );

// @access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //eFifoArea eArea // RAM area<nl>
    CVampTrapLostIrqRA3( IN eFifoArea eArea );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampTrapLostIrqRA3();

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTrapLostIrqRA4 | Interrupt handling class for Fifo 4.<nl>
// @base protected | CVampFiFoIrq
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampTrapLostIrqRA4 :
    protected CVampFiFoIrq
{
//@access private 
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access protected
protected:
    //@cmember Handles several interrupt conditions, only ODD/EVEN are necessary
    // for audio.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    VAMPRET OnFiFo4( 
        IN DWORD dwIntCond, 
        IN DWORD dwLostInts);

    //@cmember This is the register method to add or remove an ISR to a specific
    // interrupt event. See note above for AddFiFoComplete1.<nl>
    //Parameterlist:<nl>
    //CVampCoreStream *pCoreStream // core stream pointer<nl>
    //DWORD dwIntCond // interrupt condition<nl>
    VAMPRET AddFiFoComplete4( 
        IN CVampCoreStream *pCoreStream, 
        IN DWORD dwIntCond );

  	//@cmember Handles possible lost interrupts for FIFO 4 and call the other
  	// expected ISR's. Returns the number of lost buffers.<nl>
    //Parameterlist:<nl>
    //DWORD dwLostInts // number of lost interrupts<nl>
    //DWORD dwIntCond // interrupt condition<nl>
  	DWORD TrapLostInterruptsRA4( 
        IN DWORD dwLostInts, 
        IN DWORD dwIntCond );
    
// @access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //eFifoArea eArea // RAM area<nl>
    CVampTrapLostIrqRA4( 
        IN eFifoArea eArea );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampTrapLostIrqRA4();
};

#endif // !defined(AFX_VampTrapLostIrq_H__INCLUDED_)
