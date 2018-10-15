/* DEMOUPS - UPS Minidriver Sample
 * Copyright (C) Microsoft Corporation, 2001, All rights reserved.
 * Copyright (C) American Power Conversion, 2001, All rights reserved.
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * File:    demoups.h
 * 
 * Author:  Stephen Berard
 *
 * Description: 
 *   DLL entry points for the Demo UPS Minidriver.
 *
 * Revision History:
 *   26Jun2001   Created
 */
#ifndef _INC_DEMOUPSDLL_H_
#define _INC_DEMPUPSDLL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UPSMINIDRIVER_API /* .def file used instead; __declspec(dllexport) */


// UPS MiniDriver Interface
UPSMINIDRIVER_API DWORD UPSInit();
UPSMINIDRIVER_API void  UPSStop(void);
UPSMINIDRIVER_API void  UPSWaitForStateChange(DWORD, DWORD);
UPSMINIDRIVER_API DWORD UPSGetState(void);
UPSMINIDRIVER_API void  UPSCancelWait(void);
UPSMINIDRIVER_API void  UPSTurnOff(DWORD);

// UPSGetState values
#define UPS_ONLINE 1
#define UPS_ONBATTERY 2
#define UPS_LOWBATTERY 4
#define UPS_NOCOMM 8


// UPSInit error values
#define UPS_INITUNKNOWNERROR    0
#define UPS_INITOK              1
#define UPS_INITNOSUCHDRIVER    2
#define UPS_INITBADINTERFACE    3
#define UPS_INITREGISTRYERROR   4
#define UPS_INITCOMMOPENERROR   5
#define UPS_INITCOMMSETUPERROR  6

#ifdef __cplusplus
}
#endif

#endif
