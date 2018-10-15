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
// @module   CVampPanicManager | Error handling module for interrupts.<nl>
// @end
// Filename: VampPanicManager.h
// 
// Routines/Functions:
//
//  public:
//    AddOnPCIAbort
//    AddOnLoadError
//    AddOnConfigError
//    AddOnTrigError
//    AddOnPWRFailure
//    AddOnPCIParityError
//    EnableIrq
//    DisableIrq
//    GetObjectStatus
//
//private:
//protected:
//    OnLoadError
//    OnConfigError
//    OnTrigError
//    OnPWRFailure
//    OnPCIParityError
//    OnPCIAbort
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampPanicManager | This class handles and dispatches interrupts
// on HW errors.<nl>
// @end 
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_VampPanicManager_H__INCLUDED_)
#define AFX_VampPanicManager_H__INCLUDED_

#include "VampTypes.h"
#include "VampError.h"

class CVampPanicCallBacks;


class CVampPanicManager
{

//@access private
private:

    //@cmember Object pointer for the 'PCI abort' callback.
	CVampPanicCallBacks *m_cb_PCIAbort;

    //@cmember Object pointer for the 'Load error' callback.
	CVampPanicCallBacks *m_cb_LoadError;

    //@cmember Object pointer for the 'Config error' callback.
	CVampPanicCallBacks *m_cb_ConfigError;

    //@cmember Object pointer for the 'Trigger error' callback.
	CVampPanicCallBacks *m_cb_TrigError;

    //@cmember Object pointer for the 'Power on error' callback.
	CVampPanicCallBacks *m_cb_PWRFailure;

    //@cmember Object pointer for the 'PCI parity error' callback.
	CVampPanicCallBacks *m_cb_PCIParityError;

    //@cmember Status of interrupt enable.
	DWORD m_dwEnabledStatus;

    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access protected
protected:

    //@cmember Deal with/report load error in scaler.
    VAMPRET OnLoadError ();

    //@cmember Deal with/report configuration error in scaler.
    VAMPRET OnConfigError ();

    //@cmember Deal with/report trigger error in scaler.
    VAMPRET OnTrigError ();

    //@cmember Deal with/report power-on-reset error.
    VAMPRET OnPWRFailure ();

    //@cmember Deal with/report PCI parity error.
    VAMPRET OnPCIParityError ();

    //@cmember Deal with/report PCI abort error.
    VAMPRET OnPCIAbort ();

// @access public
public:

    //@cmember Constructor.<nl>
    CVampPanicManager();

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampPanicManager();

    //@cmember  Handling of PCI abort received.<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
	VAMPRET AddOnPCIAbort( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember  Handling of a load error raised by the scaler.<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnLoadError( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember  Handling of a configuration error raised by the scaler.<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnConfigError( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember  Handling of a trigger error raised by the scaler.<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnTrigError( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember  Handling an error on the power on reset (positive edge triggered).<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnPWRFailure( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember  Handling a parity error on a PCI transaction.<nl>
    //Parameterlist:<nl>
    //CVampPanicCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnPCIParityError( 
        IN CVampPanicCallBacks *cbObj );

    //@cmember Enable the desired interrupts.<nl>
    //Parameterlist:<nl>
    //eInterruptEnable eIntType // type of interrupt<nl>
    virtual VAMPRET EnableIrq( 
        IN eInterruptEnable eIntType )
    {
        return VAMPRET_SUCCESS;
    }

    //@cmember Disable the desired interrupts.<nl>
    //Parameterlist:<nl>
    //eInterruptEnable eIntType // type of interrupt<nl>
    virtual VAMPRET DisableIrq( 
        IN eInterruptEnable eIntType )
    {
        return VAMPRET_SUCCESS;
    }

};

#endif // !defined(AFX_VampPanicManager_H__INCLUDED_)
