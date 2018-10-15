//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
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


#pragma once

#include "VampList.h"
#include "BaseStream.h"
#include "BaseClass.h"

class CAnlgCaptureFilter;

#define STANDARD_CHANGE_EVENT  0
#define TERMINATE_THREAD_EVENT 1

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is the Analog Event Handler class.
//
//////////////////////////////////////////////////////////////////////////////
class CAnlgEventHandler : public CEventCallback,
                          private CBaseClass,
                          private CVampList<CBaseStream>
{
public:
    CAnlgEventHandler();
    virtual ~CAnlgEventHandler();

    NTSTATUS    Initialize  (PKSFILTER);
    NTSTATUS    Register    (PKSPIN);
    NTSTATUS    DeRegister  (PKSPIN);
    NTSTATUS    Process     ();
    VAMPRET     Notify      (PVOID, PVOID, eEventType);

    KEVENT      m_kevAnlgVideoChangeDetected;
    KEVENT      m_kevTerminateEventThread;
    KEVENT      m_kevEventThreadStarted;
    KEVENT      m_kevEventThreadTerminated;

private:
    NTSTATUS    AddNotify();
    NTSTATUS    RemoveNotify();

    PVOID       m_pEventHandle;
    PKSFILTER   m_pKSFilter;

};

void EventHandleSystemThread
(
   IN PVOID pContext
);

