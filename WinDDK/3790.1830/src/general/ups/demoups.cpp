/* DEMOUPS - UPS Minidriver Sample
 * Copyright (C) Microsoft Corporation, 2001, All rights reserved.
 * Copyright (C) American Power Conversion, 2001, All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * File:    demoups.cpp
 * 
 * Author:  Stephen Berard
 *
 * Description: 
 *   Demo UPS Minidriver implementation.  This minidriver provides
 *   a basic framework for a UPS Minidriver.
 *
 * Revision History:
 *   26Jun2001   Created
 */
#include <windows.h>
#include "demoups.h"

// Global value used to indicate that the UPS state has changed
HANDLE theStateChangedEvent;

// Global handle to DLL module
HINSTANCE theDLLModuleHandle;

/**
 * DllMain
 *
 * Description:
 *   This method is called when the DLL is loaded or unloaded.
 *
 * Parameters:   
 *   aHandle:   the DLL module handle
 *   aReason:   flag indicating the reason why the entry point was called
 *   aReserved: reserved
 *
 * Returns:
 *   TRUE
 *
 */
BOOL APIENTRY DllMain(HINSTANCE aHandle, DWORD  aReason, LPVOID aReserved) {

  switch (aReason) {
    // DLL initialization code goes here
    case DLL_PROCESS_ATTACH:
      theDLLModuleHandle = aHandle;
      DisableThreadLibraryCalls(theDLLModuleHandle);   
      break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
   
  return TRUE;
}


/**
 * UPSInit
 *
 * Description:
 *   Must be the first method called in the interface.  This method should do
 *   any initialization required and obtain the initial UPS state.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   UPS_INITOK: successful initialization
 *   UPS_INITUNKNOWNERROR: failed initialization
 *   
 */
UPSMINIDRIVER_API DWORD UPSInit() {
  DWORD init_err = UPS_INITOK;
  
  theStateChangedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (!theStateChangedEvent) {
      init_err = UPS_INITUNKNOWNERROR;
  }

  return init_err;
}


/**
 * UPSStop
 *
 * Description:
 *   Stops monitoring of the UPS.  This method should perform any necessary 
 *   cleanup.
 *
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 *   
 */
UPSMINIDRIVER_API void UPSStop(void) {
  UPSCancelWait();  

  if (theStateChangedEvent) {
      CloseHandle(theStateChangedEvent);
      theStateChangedEvent = NULL;
  }
}


/**
 * UPSWaitForStateChange
 *
 * Description:
 *   Blocks until the state of the UPS differs from the value passed in 
 *   via aState or anInterval milliseconds has expired.  If anInterval has 
 *   a value of INFINITE this function will never timeout
 *
 * Parameters:
 *   aState: defines the state to wait for a change from,
 *           possible values:
 *           UPS_ONLINE 
 *           UPS_ONBATTERY
 *           UPS_LOWBATTERY
 *           UPS_NOCOMM
 *
 *   anInterval: timeout in milliseconds, or INFINITE for
 *               no timeout interval
 *
 * Returns:
 *   None
 *   
 */
UPSMINIDRIVER_API void UPSWaitForStateChange(DWORD aState, DWORD anInterval) {

  // Wait for the UPS state to change.  Typically a separate thread would be 
  // used to monitor the UPS and then set theStateChangedEvent to indicate 
  // that the state has changed.  In this demo we only report UPS_ONLINE so
  // we don't have a monitoring thread.
  if (theStateChangedEvent) {
    WaitForSingleObject(theStateChangedEvent, anInterval);
  }
}


/**
 * UPSGetState
 *
 * Description:
 *   Returns the current state of the UPS.  This demo minidriver always returns
 *   UPS_ONLINE.
 *
 * Parameters:
 *   None
 *
 * Returns: 
 *   possible values:
 *           UPS_ONLINE 
 *           UPS_ONBATTERY
 *           UPS_LOWBATTERY
 *           UPS_NOCOMM
 *   
 */
UPSMINIDRIVER_API DWORD UPSGetState(void) {

  // Determine the UPS state and return it.
  // Demo UPS minidriver always returns Online
  return UPS_ONLINE;
}


/**
 * UPSCancelWait
 *
 * Description:
 *   Interrupts a pending calls to UPSWaitForStateChange without regard to 
 *   timout or state change
 * 
 * Parameters:
 *   None
 *
 * Returns:
 *   None
 *   
 */
UPSMINIDRIVER_API void UPSCancelWait(void) {
  // Send a signal to interupt anything waiting.
  if (theStateChangedEvent) {
      SetEvent(theStateChangedEvent);
  }
}


/**
 * UPSTurnOff
 *
 * Description:
 *   Attempts to turn off the outlets on the UPS after the specified delay.
 *   This demo minidriver ignores this call and simply returns.  
 *
 * Parameters:
 *   aTurnOffDelay: the minimum amount of time to wait before
 *                  turning off the outlets on the UPS
 *
 * Returns:
 *   None
 *   
 */
UPSMINIDRIVER_API void UPSTurnOff(DWORD aTurnOffDelay) {
  // Code to power off the UPS goes here
}




