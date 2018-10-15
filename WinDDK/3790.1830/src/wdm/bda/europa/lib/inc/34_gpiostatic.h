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

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   GPIOStatic | This file contains the user interface to use the
//           GPIO Pins as static.

// Filename: 34_GPIOStatic.h
//
// Routines/Functions:
//
//  public:
//      CVampGPIOStatic
//      ~CVampGPIOStatic
//      WriteToPinNr
//      WriteToGPIONr
//      ReadFromPinNr
//      ReadFromGPIONr
//      GetPinsConfiguration
//	    GetObjectStatus
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _GPIOSTATIC__
#define _GPIOSTATIC__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampGPIOStatic | This file contains the user interface to use the
//         GPIO Pins as static.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampGPIOStatic {
// @access Public
public:
    // @access Public functions
    // @cmember Constructor.<nl>
    // Parameterlist<nl>
    // DWORD dwDeviceIndex // Device Index<nl>
    CVampGPIOStatic(
		IN DWORD dwDeviceIndex );
    // @cmember Destructor.<nl>
    virtual ~CVampGPIOStatic(
            VOID);
    // @cmember This function writes a value to the Pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist<nl>
    // BYTE nPinNr // Pin number to be written to<nl> 
    // BYTE ucValue // Value to be written<nl>
    VAMPRET WriteToPinNr(BYTE nPinNr, BYTE ucValue);
    // @cmember This function writes a value to the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist<nl>
    // BYTE nGPIONr // GPIO pin number to be written to<nl> 
    // BYTE ucValue // Value to be written<nl>
    VAMPRET WriteToGPIONr(BYTE nGPIONr, BYTE ucValue);
    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist<nl>
    // BYTE nPinNr // Pin number to be read from<nl> 
    // BYTE* pbValue // Pointer to Value to be read<nl>
    VAMPRET ReadFromPinNr(BYTE nPinNr, BYTE* pbValue);
    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist<nl>
    // BYTE nGPIONr // GPIO pin number to be read from<nl> 	
    // BYTE* pbValue // Pointer to Value to be read<nl>
    VAMPRET ReadFromGPIONr(BYTE nGPIONr, BYTE* pbValue);
    // @cmember Returns the EEPROM/Bootstrap information about the GPIO pins.
    //          The configuration of each pin is in the approriated bit. One
    //          means InputOnly and zero means InputAndOutput.<nl>
    // Parameterlist<nl>
    // PDWORD pdwPinConfig // pin configuration (pointer) to add.<nl>
    VAMPRET GetPinsConfiguration(PDWORD pdwPinConfig);
    // @cmember Reset for external ICs (GPIO28).<nl>
    VAMPRET Reset();
	// @cmember Reads and returns the current status of this object
    DWORD GetObjectStatus();
// @access Private
private:
    // @access Private variables
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
    // @cmember Represents the current status of this object
    DWORD m_dwObjectStatus;
    // @cmember Pointer to corresponding KM GPIO object<nl>
    PVOID m_pKMGPIOStatic;
};

#endif // _GPIOSTATIC__
