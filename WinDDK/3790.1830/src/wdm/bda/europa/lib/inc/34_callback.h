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
// @module   UMCallback | This class is a pure virtual class with the 
//			 definition of UMCallback().

// 
// Filename: 34_UMCallback.h
// 
// Routines/Functions:
//
//  public:
//       UMCallback
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _UMCALLBACK__
#define _UMCALLBACK__

//#pragma once

#include "34_types.h"
 
//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CUMCallback | Callback class
//         Pure definition of callback method, The callback object is containing a
//         notifcation method to signal completed buffers. The object, which needs a 
//         notification has to be derived from this class. 
//         It has to implement the UMCallback method. <nl>

//////////////////////////////////////////////////////////////////////////////
class CUMCallback
{
//@access public members
public:
    //@access Public functions
    //@cmember UMCallback<nl>
    //Parameterlist:<nl>
    //PVOID pUMParam // <nl>
    //eEventType tEvent // <nl>
    virtual VOID UMCallback ( PVOID pUMParam, eEventType tEvent ) = NULL;
};

#endif _UMCALLBACK__
