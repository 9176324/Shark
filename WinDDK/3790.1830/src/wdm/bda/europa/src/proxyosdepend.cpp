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

#include "34AVStrm.h"
#include "ProxyKM.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor. Specific implementation of event signal mechanism.
//  Callers must be running in PASSIVE_LEVEL.
//
//////////////////////////////////////////////////////////////////////////////
CProxyOSDepend::CProxyOSDepend
(
    PVOID pEvent //Pointer to event handle
)
{
    // used the event member also as the "object status" and initialize it
    // to NULL, see check in SignalCommonEvent
    m_hEvent = NULL;
    // check if parameters are valid
    if( !pEvent )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Constructor failed"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return;
    }
    // check access validation on the event handle
    NTSTATUS Status =
        ObReferenceObjectByHandle(pEvent,
                                  GENERIC_READ | GENERIC_WRITE,
                                  NULL,
                                  static_cast <char> (KernelMode),
                                  &m_hEvent,
                                  NULL);
    if (Status != STATUS_SUCCESS)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Constructor failed"));
        m_hEvent = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//  Callers must be running <= DISPATCH_LEVEL.
//
//////////////////////////////////////////////////////////////////////////////
CProxyOSDepend::~CProxyOSDepend()
{
    if( m_hEvent )
    {
        if( KeGetCurrentIrql() > DISPATCH_LEVEL )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
            return;
        }
        // this function does not support a return value, so we
        // assume that it never fails
        ObDereferenceObject( m_hEvent );
        m_hEvent = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Signals the common event so the user mode can call a user mode callback
//  function.
//  Callers must be running <= DISPATCH_LEVEL.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void CProxyOSDepend::SignalCommonEvent()
{
    LONG lReturn = 0;
    // check if member is initialized correctly
    if( !m_hEvent )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return;
    }
    //  Set the event object to a signaled state if the event
    //  was not already signaled, function returns the previous state
    //  of the event object
    lReturn = KeSetEvent( static_cast <PKEVENT> (m_hEvent), 0, FALSE );
    if( lReturn != 0 )
    {
        _DbgPrintF(DEBUGLVL_BLAB,
            ("Info: Previous state of the event object was signaled"));
    }
}
