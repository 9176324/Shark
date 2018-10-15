//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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

#ifndef _EVENTHANDLER__
#define _EVENTHANDLER__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  EventHandler | Driver interface for Event handling.

// 
// Filename: 34_EventHandler.h
// 
// Routines/Functions:
//
//  public:
//          
//      CVampEventHandler
//      ~CVampEventHandler
//		AddEventNotify
//      RemoveEventNotify
//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampEventHandler  
{
//@access Public
public:
	//@access Public functions
    //@cmember Constructor<nl> 
    //Parameterlist:<nl>
    //DWORD dwDeviceIndex // Device Index<nl>
	CVampEventHandler(
       IN DWORD dwDeviceIndex );
    //@cmember Destructor<nl> 
	virtual ~CVampEventHandler();
	//@cmember Adds a Callback object to an certain event. The Callback object 
    //has a notifcation methods for decoder changes, error reporting and GPIO 
    //input signal changes.You are not able to add notification methods for 
	//DMA transfer complete events. This events need to be implemented via 
	//the stream classes AddOnXXX methods.<nl>
	//Parameterlist:<nl>
    //PVOID* ppEventHandle // Receives the event handle.<nl>
    //eEventType tEvent // The event that should be notified.<nl>
    //CUMCallback* pUMCBObj // Abstract class that contains the
                            // callback handler.<nl>
    //PVOID pUMParam //UM callback parameter.<nl>
	VAMPRET AddEventNotify(	IN OUT PVOID* ppEventHandle,
                            IN eEventType tEvent, 
							IN CUMCallback* pUMCBObj,
							IN PVOID pUMParam = NULL);
	//@cmember Removes a Callback object.<nl>
	//Parameterlist:<nl>
    //PVOID pEventHandle // Contains the event handle to the event that
                       // should be removed.<nl>
    VAMPRET RemoveEventNotify( IN PVOID pEventHandle);
//@access Private
private:
	//@access Private variables
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
    //@cmember Pointer to corresponding KM Event object<nl>
    PVOID m_pKMEvent;

};

#endif // _EVENTHANDLER__
