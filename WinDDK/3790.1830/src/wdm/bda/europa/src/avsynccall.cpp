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
#include "OSDepend.h"
#include "AVSyncCall.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor, gets the interrupt object created by IoConnectInterrupt.
//
//////////////////////////////////////////////////////////////////////////////
CAVSyncCall::CAVSyncCall
(
    PKINTERRUPT pIntObject
)
{
    m_pIntObject = NULL;
    if( !pIntObject )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,
            ("Warning: pIntObject must be set in InsertIRQObject function"));
        return;
    }
    m_pIntObject = pIntObject;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function uses KeSynchronizeExecution to syncronize calls with the
//  interrupt routine. Caller must be running at passive level and calls
//  may not be recursively.
//
// Return Value:
//  TRUE        on success
//  FALSE       otherwise
//
//////////////////////////////////////////////////////////////////////////////
BOOL CAVSyncCall::SyncCall
(
    VAMP_SYNC_FN pCallbackFunction,
    PVOID pContext
)
{
    // check if parameters are valid
    if( !pCallbackFunction || !pContext )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return FALSE;
    }
    // check if member is initialized
    if( !m_pIntObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Member not initialized"));
        return FALSE;
    }
    // like other spinlock-based functions, this may
    // not be called recursively (except on uniprocessor systems)
    BOOL bSuccess =
        KeSynchronizeExecution
        (
            m_pIntObject,
            reinterpret_cast <PKSYNCHRONIZE_ROUTINE> (pCallbackFunction),
            pContext
        );
    if(!bSuccess)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: KeSynchronizeExecution failed"));
        return bSuccess;
    }
    return bSuccess;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function inserts one IRQ object which will be used during the
//  SyncCall function. This function is not in the interface of OSDepend, but
//  it is very important for WDM drivers as long as it is not possible to
//  "pause" interrupts once they are connected (see VampDevice->Start).
//
// Return Value:
//  none
//
//////////////////////////////////////////////////////////////////////////////
void CAVSyncCall::InsertIRQObject
(
    PKINTERRUPT pIntObject
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: InsertIRQObject called"));
    // check if parameter valid
    if( !pIntObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    // check if member is initialized already
    if( m_pIntObject )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Interrupt object already initialized"));
        return;
    }
    m_pIntObject = pIntObject;
    return;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Removes the IRQ object.
//
// Return Value:
//  none
//
//////////////////////////////////////////////////////////////////////////////
void CAVSyncCall::RemoveIRQObject()
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RemoveIRQObject called"));
    m_pIntObject = NULL;
    return;
}
