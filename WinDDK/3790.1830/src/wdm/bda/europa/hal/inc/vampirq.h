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
// @module   CVampIrq | This module manages all interrupt sources and
//           dispatches  the different conditions to the corresponding
//           interrupt methods, which are inherited from the interrupt
//           subclasses. The ResetCond() method, which resets the interrupt
//           source's condition bits, is overwritten, so all the handled bits
//           are touched. All untouched condition bits and IRQ's could be
//           passed to another IRQ handler in a chain. Missing interrupts from
//           the FIFO module are handled by an interrupt counter and a ring
//           buffer.<nl>
// @end
// Filename: VampIrq.h
// 
// Routines/Functions:
//
// public:
//    CVampIrq
//    ~CVampIrq
//    OnInterrupt
//    EnableIrq
//    DisableIrq
//    EnableIrq
//    DisableIrq
//    OnDPC
//    DealWithMultipleInterrupts
//    ResetCond
//    DisableAllIrqs
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VAMPIRQ_H__566E1F00_8184_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPIRQ_H__566E1F00_8184_11D3_A66F_00E01898355B__INCLUDED_


#include "VampTypes.h"

#include "VampDecoderIrq.h"
#include "VampPanicManager.h"
#include "VampFifoIrq.h"
#include "VampTrapLostIrq.h"
#include "VampDecCB.h"
#include "VampGpioIrq.h"
#include "VampIo.h"
#include "OSDepend.h"

class CVampDeviceResources;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampIrq | Main interrupt handler class.<nl>
// @base protected | CVampDecoderIrq
// @base protected | CVampPanicManager 
// @base protected | CVampTrapLostIrqRA1
// @base protected | CVampTrapLostIrqRA2
// @base protected | CVampTrapLostIrqRA3
// @base protected | CVampTrapLostIrqRA4
// @base protected | CVampGpioIrq
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampIrq :
    protected CVampDecoderIrq,
    protected CVampPanicManager,
    protected CVampTrapLostIrqRA1,
	protected CVampTrapLostIrqRA2,
	protected CVampTrapLostIrqRA3,
	protected CVampTrapLostIrqRA4,
	protected CVampGpioIrq
{
// @access private 
private:
    //@cmember Pointer to device resources object.
    CVampDeviceResources* m_pDevice;

    //@cmember Pointer to CVampIo object.
    CVampIo *m_pVampIo;

    // When an interrupt occurs we'll read the interrupt status register. The
    // value read will be used to reset the interrupt ( XOR in hardware )
    //@cmember Interrupt condition mask.
    DWORD    m_dwCondMask;

    //@cmember Current interrupt enable status 1, only used by DisableAllIrqs()
    // and ResetAllIrqs().
    DWORD    m_dwInterruptEnable1;
    //@cmember Current interrupt enable status 2, only used by DisableAllIrqs()
    // and ResetAllIrqs().
    DWORD    m_dwInterruptEnable2;

    //@cmember White Peak Off default value, retrieved during construction.
    DWORD    m_WpOff;
    //@cmember Color Peak Off default value, retrieved during construction.
    DWORD    m_ColPeakOff;

    //@cmember Error status flag, will be set during construction.
	DWORD    m_dwObjectStatus;

    //@cmember Deals with multiple interrupts. This method will be called when more than
	// one interrupt bit is set. It will step through the interrupt bits calling the
	// appropriate interrupt handling routines.<nl>
	VAMPRET DealWithMultipleInterrupts();

    //@cmember Resets the interrupt and the condition bits. This method overloads
    // the method from CVampFiFoIrq, CVampDecoderIrq and CVampPanicManager.<nl>
    VAMPRET ResetCond();

// @access protected 
protected:

    //@cmember Overloading method which enables an interrupt from the FIFO. This
    // method will be called when a callback function is added to the ISR list.<nl>
    //Parameterlist:<nl>
	//DWORD dwIntCond // Interrupt condition<nl>
	//eFifoArea eArea // RAM area<nl>
    VAMPRET EnableIrq( 
        IN DWORD dwIntCond, 
        IN eFifoArea eArea );

    //@cmember Overloading method which disables an interrupt from the FIFO. This
    // method will be called when a callback function is removed from the ISR list.<nl>
    //Parameterlist:<nl>
	//DWORD dwIntCond // Interrupt condition<nl>
	//eFifoArea eArea // RAM area<nl>
    VAMPRET DisableIrq( 
        IN DWORD dwIntCond, 
        IN eFifoArea eArea );

    //@cmember Overloading method which enables an interrupt from the decoder, scaler
    // or a PCI source. This method will be called when a callback function
    // is added to the ISR list.<nl>
    //Parameterlist:<nl>
	//eInterruptEnable eIntType // type of interrupt<nl>
    VAMPRET EnableIrq( 
        IN eInterruptEnable eIntType );
	
    //@cmember Overloading method which disables an interrupt. This method will be
    // called when a callback function is removed from the ISR list.<nl>
    //Parameterlist:<nl>
	//eInterruptEnable eIntType // type of interrupt<nl>
    VAMPRET DisableIrq( 
        IN eInterruptEnable eIntType );


// @access public 
public:

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevRes // Pointer to device resources object.<nl>
    CVampIrq( 
        IN CVampDeviceResources *pDevRes );

    //@cmember Destructor.<nl>
    virtual ~CVampIrq();

    //@cmember This is the main handler for the SAA7134 hardware interrupt. If an
    // associated ISR method has handled this IRQ, the involved status bit
    // will be reset and the handled condition, which is accumulated by the
    // ResetCond() method, will be also be reset. If no ISR is handled, the
    // condition the interrupt will be passed to the next IRQ handler.
	// Also checks whether the interrupt occured on the SAA7134.<nl>
    VAMPRET OnInterrupt();

    //@cmember Loops through its list of core streams and calls
    // CVampCoreStream::OnDPC(). This calls the stream's registered callback. <nl>
    VAMPRET OnDPC();

    //@cmember Disables all interrupts.<nl>
    VAMPRET DisableAllIrqs();

    //@cmember Resets all interrupts (eg. in the case of power management).<nl>
    VAMPRET ResetAllIrqs();

    //@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus ( )
	{
		return m_dwObjectStatus;
	}
};
#endif
