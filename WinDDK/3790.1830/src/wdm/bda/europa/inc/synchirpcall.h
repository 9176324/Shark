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

#pragma once

NTSTATUS HandleCompletion(IN PDEVICE_OBJECT pDevice,
                          IN PIRP pIrp,
                          IN PVOID pContext);

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class handles sending and completion of IRP's in a syncronous way.
//
//////////////////////////////////////////////////////////////////////////////
class CSyncIrpCall
{
public:
    CSyncIrpCall();

    NTSTATUS  Call( IN PDEVICE_OBJECT pTarget, IN PIRP pIrp );
    NTSTATUS  OnCompletion(IN PDEVICE_OBJECT pDevice,
                           IN PIRP pIrp);
private:
    KEVENT    m_Event;
    bool      m_bCompleted;
    NTSTATUS  m_sCall;
    ULONG_PTR m_cInformation;
};
