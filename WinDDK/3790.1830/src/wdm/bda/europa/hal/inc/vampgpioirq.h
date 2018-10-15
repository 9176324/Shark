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
// @module   CVampGpioIrq | This module handles the GPIO interrupts.<nl>
// @end
// Filename: VampGpioIrq.h
// 
// Routines/Functions:
//
//  public:
//    AddOnGpioIrq
//    EnableIrq
//    DisableIrq
//    GetObjectStatus
//private:
//protected:
//    OnGpioIrq
//
//
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampGpioIrq | Virtual methods for the GPIO interrupt callbacks.  
//         These will be overloaded in any derived class.<nl>
// @end 
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_VampGpioIrq_H__INCLUDED_)
#define AFX_VampGpioIrq_H__INCLUDED_

#include "VampTypes.h"
#include "VampError.h"

class CVampGpioCallBacks
{
//@access public 
public:

    //@cmember This method will be called from the CVampGpioIrq object during an
    //interrupt and is reflecting a GPIO Irq. It needs to be
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnGPIO16() = NULL;

	//@cmember This method will be called from the CVampGpioIrq object during an
    //interrupt and is reflecting a GPIO Irq. It needs to be
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnGPIO18() = NULL;

	//@cmember This method will be called from the CVampGpioIrq object during an
    //interrupt and is reflecting a GPIO Irq. It needs to be
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnGPIO22() = NULL;

	//@cmember This method will be called from the CVampGpioIrq object during an
    //interrupt and is reflecting a GPIO Irq. It needs to be
    //safely callable at interrupt time.<nl>
    virtual VAMPRET OnGPIO23() = NULL;
};



class CVampGpioIrq
{
//@access private 
private:
    //@cmember Object pointer for the 'GPIO16' callback.<nl>
   	CVampGpioCallBacks *m_cb_GPIO16;

	//@cmember Object pointer for the 'GPIO18' callback.<nl>
   	CVampGpioCallBacks *m_cb_GPIO18;

	//@cmember Object pointer for the 'GPIO22' callback.<nl>
   	CVampGpioCallBacks *m_cb_GPIO22;

	//@cmember Object pointer for the 'GPIO23' callback.<nl>
   	CVampGpioCallBacks *m_cb_GPIO23;

	//@cmember Status of interrupt enable.<nl>
	DWORD m_dwEnabledGPIOStatus;

	//@cmember Error status flag, will be set during construction.<nl>
    DWORD m_dwObjectStatus;

//@access protected 
protected:
    //@cmember Deal with and reports GPIO interrupts.<nl>
    //Parameterlist:<nl>
    //DWORD dwIntType // interrupt type<nl>
    VAMPRET OnGpioIrq( IN DWORD dwIntType );

//@access public 
public:

    //@cmember Constructor.<nl>
    CVampGpioIrq();

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampGpioIrq();

    //@cmember Add callback function for GPIO interrupts.<nl>
    //Parameterlist:<nl>
    //eInterruptEnable eGpioEnableType // type of interrupt<nl>
    //CVampGpioCallBacks *cbObj // callback object<nl>
    VAMPRET AddOnGpioIrq(
            IN eInterruptEnable eGpioEnableType,
            IN CVampGpioCallBacks *cbObj );

    //@cmember Enables the desired interrupts.<nl>
    //Parameterlist:
    //eInterruptEnable eGpioEnableType // type of interrupt<nl>
    virtual VAMPRET EnableIrq( 
        IN eInterruptEnable eGpioEnableType )
    {
        return VAMPRET_SUCCESS;
    }

    //@cmember Disables the desired interrupts.<nl>
    //Parameterlist:<nl>
    //eInterruptEnable eGpioEnableType // type of interrupt<nl>
    virtual VAMPRET DisableIrq ( 
        IN eInterruptEnable eGpioEnableType )
    {
        return VAMPRET_SUCCESS;
    }
};

#endif // !defined(AFX_VampGpioIrq_H__INCLUDED_)
