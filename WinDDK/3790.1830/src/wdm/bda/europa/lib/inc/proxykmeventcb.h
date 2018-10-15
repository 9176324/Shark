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
// @module   ProxyKMEventCB | It is possible to register user mode callback functions
//           in the HAL, which are automatically called if a specific
//           interrupt occurred. This class is used to assure that these
//           callbacks are safely called at DPC level and to get asynchron
//           access to the user mode from the kernel mode.

// Filename: ProxyKMEventCB.h
//
// Routines/Functions:
//
//  public:
//
//  void Notify(PVOID pParam1, PVOID pParam2, eEventType tEvent);
//  VAMPRET GetNextCallback(PVOID* ppParam1, PVOID* ppParam2, eEventType* ptEvent);
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

#include "VampDeviceResources.h"
#include "VampBuffer.h"
#include "AbstractStream.h"
#include "ProxyOSDepend.h"
#include "ProxyKMLinkedCallbackList.h"

class CProxyKMEventCB : public CEventCallback
{
public:
	CProxyKMEventCB(HANDLE hEvent, COSDependSpinLock* pSpinLockObj);
	~CProxyKMEventCB();
	VAMPRET Notify(PVOID pParam1, PVOID pParam2, eEventType tEvent);
	VAMPRET GetNextCallback(
		PVOID* ppParam1, 
		PVOID* ppParam2, 
		eEventType* ptEvent);
	DWORD GetObjectStatus();
private:
	DWORD m_dwObjectStatus;
	CProxyOSDepend* m_pCProxyOSDependObj;
	CLinkedCallbackList* m_pLinkedCallbackListObj;
	COSDependSpinLock* m_pSpinLockObj;
};
