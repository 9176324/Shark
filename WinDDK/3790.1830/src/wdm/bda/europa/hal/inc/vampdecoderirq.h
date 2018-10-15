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
// @module   CVampDecoderIrq | This module handles all the interrupts coming
//           from the decoder. The interupts are dispatched by the CVampIrq
//           class to this.  For video there may 3 types of stream running
//           (Task A, B or AB). Additionally there may be a VBI stream (Task
//           AB). If a decoder status changes then all the streams need to be
//           notified of the change via the appropriate interrupt service
//           routine. Thus we have 4 possible ISR's, although a maximum of 3
//           can be running simultaneously When the interrupt occurs, all 
//           non-NULL ISR's will be called.<nl>
// @end
// Filename: VampDecoderIrq.h
// 
// Routines/Functions:
//
//  public:
//
//    AddOnDecoderChangeMVM
//    AddOnDecoderChangeFIDT
//    AddOnDecoderChangeINTL
//    AddOnDecoderChangeRDToCap
//    VAMPRET EnableIrq
//    VAMPRET DisableIrq
//
//    OnDecoderChangeMVM
//    OnDecoderChangeFIDT
//    OnDecoderChangeINTL
//    OnDecoderChangeRDToCap
//
//    GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampDecCB.h"
  

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDecoderIrq | Interface class for the decoder interrupts.<nl>
// @base private | CVampDecoderCallBacks
// @end 
//
//////////////////////////////////////////////////////////////////////////////
class CVampDecoderIrq : public CVampDecoderCallBacks
{
    // @access private 
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@access protected 
protected:
    //@cmember Method to add the callback for the handling of the MacroVision
	// state change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // interrupt task type<nl>
    VAMPRET AddOnDecoderChangeMVM ( 
        IN CVampDecoderCallBacks *cbObj );

    //@cmember Method to add the callback for the handling of the 50/60Hz field
	// frequency change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // interrupt task type<nl>
    VAMPRET AddOnDecoderChangeFIDT ( 
        IN CVampDecoderCallBacks *cbObj );

    //@cmember Method to add the callback for the handling of the Interlace mode
	// change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // interrupt task type<nl>
    VAMPRET AddOnDecoderChangeINTL ( 
        IN CVampDecoderCallBacks *cbObj );

    //@cmember Method to add the callback for the handling of the ready for capture 
	// interrupt.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // interrupt task type<nl>
    VAMPRET AddOnDecoderChangeRDToCap ( 
        IN CVampDecoderCallBacks *cbObj );
	//@cmember This method will be called from the CVampDecoderIrq object during an
    //interrupt and is reflecting a MacroVision mode change. It needs to be 
    //safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eMacroVisionMode eMVM // new MacroVision mode<nl>
    VAMPRET OnDecoderChangeMVM( 
        IN eMacroVisionMode eMVM );

    //@cmember This method will be called from the CVampDecoderIrq object during an
    //interrupt and is reflecting a field frequency change. It needs to be 
    //safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eFieldFrequency newFIDT // new field frequency<nl>
    VAMPRET OnDecoderChangeFIDT( 
        IN eFieldFrequency newFIDT );

    //@cmember This method will be called from the CVampDecoderIrq object during an
    //interrupt and is reflecting an interlaced / non interlaced change. It needs 
    //to be safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eInterlacedMode newINTL // new interlace mode<nl>
    VAMPRET OnDecoderChangeINTL( 
        IN eInterlacedMode newINTL );

    //@cmember This method will be called from the CVampDecoderIrq object during an
    //interrupt and is signals that the device is ready to capture. It needs 
    //to be safely callable at interrupt time.<nl>
    VAMPRET OnDecoderChangeRDToCap();


    //@cmember enable the desired interrupts.<nl>
    //Parameterlist:<nl>
	//eInterruptEnable eIntType // type of interrupt<nl>
    virtual VAMPRET EnableIrq  ( 
        IN eInterruptEnable eIntType )    
    {
        return VAMPRET_SUCCESS;
    }

    //@cmember disable the desired interrupts.<nl>
    //Parameterlist:<nl>
	//eInterruptEnable eIntType // type of interrupt<nl>
    virtual VAMPRET DisableIrq ( 
        IN eInterruptEnable eIntType )
    {
        return VAMPRET_SUCCESS;
    }

    eMacroVisionMode m_MVMode;

// @access public 
public:

    //@cmember Constructor.<nl>
    CVampDecoderIrq();

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampDecoderIrq();

};

