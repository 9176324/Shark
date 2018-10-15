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
// @doc      INTERNAL
// @module   Event | It is possible to register user mode callback functions
//           in the HAL, which are automatically called if a specific
//           interrupt occurred. This class is used to assure that these
//           callbacks are safely called at DPC level and to get asynchron
//           access to the user mode from the kernel mode.

// Filename: ProxyKMCallback.h
//
// Routines/Functions:
//
//  public:
//
//  VAMPRET GetNextCallback(PVOID* ppParam1, PVOID* ppParam2, eEventType* ptEvent);
//  DWORD GetObjectStatus();
//  CProxyKMEventCB* GetEventCBObj();
//  CProxyKMStreamCB* GetStreamCBObj();
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

#include "ProxyKMLinkedCallbackList.h"
#include "ProxyKMEventCB.h"
#include "ProxyKMStreamCB.h"

class CProxyKMCallback
{
public:
    CProxyKMCallback(HANDLE hEvent,	COSDependSpinLock* pSpinLockObj);
    ~CProxyKMCallback();
    VAMPRET GetNextCallback(
        PVOID* ppParam1,
        PVOID* ppParam2,
        eEventType* ptEvent);
    DWORD GetObjectStatus();
    CProxyKMEventCB* GetEventCBObj();
    CProxyKMStreamCB* GetStreamCBObj();
private:
    DWORD m_dwObjectStatus;
    CProxyKMEventCB* m_pEventCBObj;
    CProxyKMStreamCB* m_pStreamCBObj;
};
