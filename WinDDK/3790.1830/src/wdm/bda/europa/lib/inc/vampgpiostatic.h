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
// @module   GPIOStatic | This file contains the user interface to use the
//           GPIO Pins as static.
// @end
//
// Filename: GPIOStatic.h
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
//      GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef GPIOSTATIC__
#define GPIOSTATIC__

#include "VampGPIO.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class CVampGPIOStatic | This class can be used by the user for GPIO pin
//        access.
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampGPIOStatic 
{
// @access private 
private:
    // @cmember Pointer to the device resources object.<nl>
    CVampDeviceResources* m_pVampDevRes;  

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;
    
    // @access public 
public:
    // @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // VampDeviceResources* pDevRes // pointer to the device resources object<nl>
    CVampGPIOStatic(
            CVampDeviceResources* pDevRes );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    // @cmember Destructor.<nl>
    virtual ~CVampGPIOStatic();

    // @cmember This function writes a value to the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nPinNr // pin number<nl>
    // BYTE ucValue // value<nl>
    VAMPRET WriteToPinNr(
        BYTE nPinNr, 
        BYTE ucValue );

    // @cmember This function writes a value to the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nGPIONr // pin number<nl>
    // BYTE ucValue // value<nl>
    VAMPRET WriteToGPIONr(
        BYTE nGPIONr, 
        BYTE ucValue );

    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nPinNr // pin number<nl>
    // BYTE *pbValue // value pointer<nl>
    VAMPRET ReadFromPinNr(
        BYTE nPinNr, 
        BYTE *pbValue );

    // @cmember This function reads a value from the GPIO pin. It fails if the
    //          pin is already in use or the direction can not be set.<nl>
    // Parameterlist:<nl>
    // BYTE nGPIONr // pin number<nl>
    // BYTE *pbValue // value pointer<nl>
    VAMPRET ReadFromGPIONr(
        BYTE nGPIONr, 
        BYTE *pbValue );

    // @cmember Returns the EEPROM/Bootstrap information about the GPIO pins.
    //          The configuration of each pin is in the appropriated bit. One
    //          means InputOnly and zero means InputAndOutput.<nl>
    // Parameterlist:<nl>
    // DWORD *pdwPinsConfig // pointer on pin configuration<nl>
     VAMPRET GetPinsConfiguration(
         DWORD* pdwPinsConfig );

	 // @cmember Reset for external ICs (GPIO28).<nl>
     VAMPRET Reset();
};

#endif // GPIOSTATIC__
