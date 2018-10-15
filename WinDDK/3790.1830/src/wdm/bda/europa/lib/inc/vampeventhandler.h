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
// @module   VampEventHandler| This module is responsible for the communication
//           between HAL and driver (application) due to certain events 
//           (interrupts) which may occur while the driver is running.<nl>
// @end
// Filename: VampEventHandler.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#if !defined(_VAMPEVENTHANDLER_H_)
#define _VAMPEVENTHANDLER_H_

#include "OSDepend.h"
#include "VampList.h"
#include "VampIrq.h"
#include "VampPanicCB.h"

class CEventCallback;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CDerivedList | List element, which contains the CEventCallback
//         object information.
// @base public | CVampListEntry
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CDerivedList : public CVampListEntry
{
//@access private 
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;
    //@cmember CEventCallback object.
    CEventCallback *m_pCEventCallback;
    //@cmember First callback parameter.
    PVOID m_pCBParam1;
    //@cmember Second callback parameter.
    PVOID m_pCBParam2;

//@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CEventCallback* pCEventCallback // callback object<nl>
    //PVOID pCBParam1 // first parameter<nl>
    //PVOID pCBParam2 // second parameter<nl>
    CDerivedList(  
        CEventCallback* pCEventCallback,
        PVOID pCBParam1,
        PVOID pCBParam2 );

    //@cmember Gets the callback object and the parameters out of the
    //member variables.<nl>
    //Parameterlist:<nl>
    //CEventCallback **ppCEventCallback // callback object<nl>
    //PVOID *ppCBParam1 // first parameter<nl>
    //PVOID *ppCBParam2 // second parameter<nl>
    VOID GetData(
        CEventCallback **ppCEventCallback,
        PVOID *ppCBParam1,
        PVOID *ppCBParam2 );

    //@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CEventCallback | The callback object is containing a notification
//         method for decoder changes, error reporting and GPIO input signal
//         changes. The object, which needs a certain notification, has to be
//         derived from this class. It has to implement the notify method. It
//         is  possible to have one notify method for several events. You are
//         not able to add notification methods for DMA transfer completion
//         events. These events need to be implemented via the stream class
//         open methods.
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CEventCallback
{
    // @access public 
public:
    //@cmember This is the pure definition of a method, which notifies an
    // event (means an interrupt occurred) to a client.<nl>
    //Parameterlist:<nl>
    //PVOID pCBParam1 // first parameter<nl>
    //PVOID pCBParam2 // second parameter<nl>
    //eEventType event // type of event<nl>
    virtual VAMPRET Notify(
        PVOID pCBParam1, 
		PVOID pCBParam2, 
		eEventType event ) = NULL;
};


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampEventHandler | This class accumulates certain events and
//         notifies clients via callback objects from type CEventCallback.
// @base public | CVampIrq
// @base private | CVampDecoderCallBacks
// @base private | CVampPanicCallBacks
// @base private | CVampGpioCallBacks
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampEventHandler : public CVampIrq,
                          public CVampPanicCallBacks,
                          public CVampGpioCallBacks
{
    // @access private
private:
    //@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@cmember Counter on attached event clients.<nl>
    DWORD m_dwClientsAttached;

    //@cmember List of events.<nl>
    CVampList<CDerivedList> m_alEvents[EN_MAX_EVENT];

    //@cmember Boolean array for all events.<nl>
    bool m_bEventTriggered[EN_MAX_EVENT];

    //@cmember Enable interrupt for event.<nl>
    //Parameterlist:<nl>
    //eEventType event // type of event<nl>
    //CVampEventHandler* ToThis // 'this' pointer to CVampEventHandler object<nl>
    VAMPRET EnsureInterruptEnabled(
        eEventType event, 
        CVampEventHandler* ToThis );


    //@cmember Sets event: macrovision mode changed.<nl>
    //Parameterlist:<nl>
    //eMacroVisionMode eMVM // new macrovision mode<nl>
    VAMPRET OnDecoderChangeMVM( 
        IN eMacroVisionMode eMVM );

    //@cmember Sets event: field frequency changed.<nl>
    //Parameterlist:<nl>
    //eFieldFrequency newFIDT // new field frequency<nl>
    VAMPRET OnDecoderChangeFIDT( 
        IN eFieldFrequency newFIDT );

    //@cmember Sets event: video mode changed (interlaced/single field).<nl>
    //Parameterlist:<nl>
    //eInterlacedMode newINTL // new video mode (interlaced/non interlaced)<nl>
    VAMPRET OnDecoderChangeINTL( 
        IN eInterlacedMode newINTL );

    //@cmember Sets event: ready to capture.<nl>
    VAMPRET OnDecoderChangeRDToCap();

    //@cmember Sets event: GPIO pin 16.<nl>
    VAMPRET OnGPIO16();

    //@cmember Sets event: GPIO pin 18.<nl>
    VAMPRET OnGPIO18();

    //@cmember Sets event: GPIO pin 22.<nl>
    VAMPRET OnGPIO22();

    //@cmember Sets event: GPIO pin 23.<nl>
    VAMPRET OnGPIO23();

    //@cmember Sets event: PCI abort error.<nl>
    VAMPRET OnPCIAbortISR();

    //@cmember Sets event: load error.<nl>
    VAMPRET OnLoadErrorISR();

    //@cmember Sets event: config error.<nl>
    VAMPRET OnConfigErrorISR();

    //@cmember Sets event: trigger error.<nl>
    VAMPRET OnTrigErrorISR();

    //@cmember Sets event: power_on reset failure.<nl>
    VAMPRET OnPWRFailureISR();

    //@cmember Sets event: parity error.<nl>
    VAMPRET OnPCIParityErrorISR();

    //@cmember Dummy method due to the virtual definition in base class.<nl>
    VAMPRET AddOnDecoderChangeMVM( 
        IN CVampDecoderCallBacks *cbObj,
        IN eDecoderIntTaskType TaskType )
    { 
        return VAMPRET_SUCCESS; 
    };

    //@cmember Dummy method due to the virtual definition in base class.<nl>
    VAMPRET AddOnDecoderChangeFIDT( 
        IN CVampDecoderCallBacks *cbObj,
        IN eDecoderIntTaskType TaskType )
    { 
        return VAMPRET_SUCCESS; 
    };

    //@cmember Dummy method due to the virtual definition in base class.<nl>
    VAMPRET AddOnDecoderChangeINTL ( 
        IN CVampDecoderCallBacks *cbObj,
        IN eDecoderIntTaskType TaskType )
    { 
        return VAMPRET_SUCCESS; 
    };

    //@cmember Dummy method due to the virtual definition in base class.<nl>
    VAMPRET AddOnDecoderChangeRDToCap( 
        IN CVampDecoderCallBacks *cbObj,
        IN eDecoderIntTaskType TaskType )
    { 
        return VAMPRET_SUCCESS; 
    };

    // @access protected
protected:
    //@cmember Pointer to DeviceResources object.
    CVampDeviceResources *m_pDevRes;

    // @access public 
public:
    //@cmember Constructor.<nl>
    CVampEventHandler ( IN CVampDeviceResources *pDevRes );

    //@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
    virtual ~CVampEventHandler ();

    //@cmember Adds a Callback object to an certain event. The Callback object is
    //          containing notifcation methods for decoder changes, error
    //          reporting and GPIO input signal changes. The client object, which
    //          needs a certain notification, has to derive a class from type
    //          CEventCallback. It has to implement the notify method. It is
    //          possible to have one notify method for several events. You are not
    //          able to add notification methods for DMA transfer complete events.
    //          These events need to be implemented via the stream class open
    //          methods.<nl>
    //Parameterlist:<nl>
    //PVOID* ppEventHandle // event handle<nl>
    //eEventType event // type of event<nl>
    //CEventCallback* pCallback // callback object<nl>
    //PVOID pCBParam1 // first parameter<nl>
    //PVOID pCBParam2 // second parameter<nl>
    VAMPRET AddEventNotify( 
        PVOID* ppEventHandle,
        eEventType event,
        CEventCallback* pCallback,
        PVOID pCBParam1 = NULL,
        PVOID pCBParam2 = NULL );

    //@cmember Removes a Callback object.<nl>
    //Parameterlist:<nl>
    //PVOID  pEventHandle // event handle<nl>
    VAMPRET RemoveEventNotify( 
        PVOID pEventHandle );

    //@cmember This method checks all event notification flags including
    //outstanding DMA transfer notifications.
    VAMPRET OnDPC ();
};
#endif //_VAMPEVENTHANDLER_H_
