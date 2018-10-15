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
// @module  Info | This module contains the Device Information access. 

//
// Filename: 34_Info.h
//
// Routines/Functions:
//
//  public:
//
//          CDeviceInfo
//          ~CDeviceInfo
//          GetNumOfDevices
//          GetInfoOfDevice
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _INFO__
#define _INFO__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class CDeviceInfo | This class abstracts the access to the 
//			Device information.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CDeviceInfo
{
// @access Public
public:
    // @access Public functions
    // @cmember Constructor<nl>
    CDeviceInfo();
    // @cmember Destructor.<nl>
    virtual ~CDeviceInfo();
    // @cmember Get number of Devices<nl>
    DWORD GetNumOfDevices();    
    // @cmember Get Device Information<nl>
    // Parameterlist<nl>
    // DWORD dwDevNum // Device Number (Index reference)<nl>
    tDeviceParams GetInfoOfDevice(IN DWORD dwDeviceIndex);    
};

#endif // _INFO__
