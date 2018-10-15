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
// @module   VampDecCB | Virtual methods for the interrupt callbacks. These 
//           will be overloaded in any derived class.
// @end
// Filename: VampDecCB.h
//
// Routines/Functions:
//
//  public:
//    OnDecoderChangeMVM
//    OnDecoderChangeFIDT
//    OnDecoderChangeINTL
//    OnDecoderChangeRDToCap
//
//    AddOnDecoderChangeMVM
//    AddOnDecoderChangeFIDT
//    AddOnDecoderChangeINTL
//    AddOnDecoderChangeRDToCap
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPDECCB_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPDECCB_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDecoderCallBacks | Virtual  interrupt callbacks methods.
// @end
//////////////////////////////////////////////////////////////////////////////


class CVampDecoderCallBacks
{
//@access public 
public:

    //@cmember This method will be called from the derived class object during an
    //interrupt reflecting a MacroVision mode change. It needs to be 
    //safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eMacroVisionMode eMVM //new MacroVision mode<nl>
    virtual VAMPRET OnDecoderChangeMVM( 
        IN eMacroVisionMode eMVM ) = NULL;


    //@cmember This method will be called from the derived class object during an
    //interrupt reflecting a field frequency change. It needs to be 
    //safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eFieldFrequency newFIDT //new field frequency<nl>
    virtual VAMPRET OnDecoderChangeFIDT( 
        IN eFieldFrequency newFIDT ) = NULL;


    //@cmember This method will be called from the derived class object during an
    //interrupt reflecting an interlaced / non interlaced change. It 
    //needs to be safely callable at interrupt time.<nl>
    //Parameterlist:<nl>
    //eInterlacedMode newINTL //new interlace mode<nl>
    virtual VAMPRET OnDecoderChangeINTL( 
        IN eInterlacedMode newINTL ) = NULL;

    //@cmember This method will be called from the derived class object during an
    //interrupt and signals that the device is ready to capture. It 
    //needs to be safely callable at interrupt time.<nl>
    virtual VAMPRET OnDecoderChangeRDToCap() = NULL;


    // @cmember Method to add the callback for the handling of the MacroVision
	// state change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // type of task<nl>
    virtual VAMPRET AddOnDecoderChangeMVM( 
        IN CVampDecoderCallBacks *cbObj,
		IN eDecoderIntTaskType TaskType ) = NULL;

    // @cmember Method to add the callback for the handling of the 50/60Hz field
	// frequency change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // type of task<nl>
    virtual VAMPRET AddOnDecoderChangeFIDT( 
        IN CVampDecoderCallBacks *cbObj,
		IN eDecoderIntTaskType TaskType ) = NULL;

    // @cmember Method to add the callback for the handling of the Interlace mode
	// change.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // type of task<nl>
    virtual VAMPRET AddOnDecoderChangeINTL( 
        IN CVampDecoderCallBacks *cbObj,
		IN eDecoderIntTaskType TaskType ) = NULL;

    // @cmember Method to add the callback for the handling of the ready for capture 
	// interrupt.<nl>
	// Parameterlist:<nl>
	// CVampDecoderCallBacks *cbObj // the callback object<nl>
    // eDecoderIntTaskType TaskType // type of task<nl>
    virtual VAMPRET AddOnDecoderChangeRDToCap( 
        IN CVampDecoderCallBacks *cbObj,
		IN eDecoderIntTaskType TaskType ) = NULL;

	// @cmember Array of isr object pointers for Macrovision callbacks
	CVampDecoderCallBacks *m_mvc_isr;

    // @cmember Array of isr object pointers for 'field frequency change' callbacks
	CVampDecoderCallBacks *m_fidt_isr;

    // @cmember Array of isr object pointers for 'interlace changed' callbacks
	CVampDecoderCallBacks *m_intl_isr;

    // @cmember Array of isr object pointers for 'ready for capture' callbacks
	CVampDecoderCallBacks *m_rdcap_isr;
};
#endif
