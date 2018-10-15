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
// @module   DeviceIo | This module provides the complete register space of
//           the device as a single CRegArray object. It is useful for URD
//           programming. 
// @end
//
// Filename: DeviceIo.h
//
// Routines/Functions:
//
//  public:
//          CDeviceIo
//          ~CDeviceIo
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _DEVICEIO_INCLUDE_
#define _DEVICEIO_INCLUDE_


#include "RegArray.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @class    DeviceIo | This class encapsulates the entire register area as 
//           as a register array to get easy access for URD support. 
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CDeviceIo
{
public:
    //@cmember Register space of the device as array of DWORD registers. 
	//Size: 256 DWORDS
    CRegArray RegSpace;

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //PBYTE pbBaseAddress // base address of memory mapped register area<nl>
    CDeviceIo ( 
		PBYTE pbBaseAddress );

    //@cmember Destructor.<nl>
    virtual ~CDeviceIo();
};



#endif _DEVICEIO_
