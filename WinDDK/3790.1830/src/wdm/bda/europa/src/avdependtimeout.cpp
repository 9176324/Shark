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
#include "AVDependTimeOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The global timer DPC callback. Thunks to a C++ member function.
//
// Return Value:
//  none
//
//////////////////////////////////////////////////////////////////////////////
void NTAPI TimerDPC
(
    IN  PKDPC   pDpc,               //Device resource object
    IN  PVOID   pDeferredContext,   //Points to a callback handler.
    IN  PVOID   pSystemArgument1,   //Additional argument, unused
    IN  PVOID   pSystemArgument2    //Additional argument, unused
)
{
    // check if parameter is valid
    if( !pDeferredContext )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return;
    }
    // cast callback object to abstract callback class
    COSDependAbstractCB* pCallbackObject =
        reinterpret_cast <COSDependAbstractCB *> (pDeferredContext);
    // call callback function on the callback object
    pCallbackObject->CallbackHandler();
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependTimeOut::CAVDependTimeOut()
{
    memset(&m_Dpc,      0, sizeof(m_Dpc));
    memset(&m_TimerObj, 0, sizeof(m_TimerObj));

    m_bObjectStatus = TRUE;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependTimeOut::~CAVDependTimeOut()
{
    memset(&m_Dpc, 0, sizeof(m_Dpc));

    BOOL bCanceled = FALSE;
    // take timer object out of queue, there is no way to check if the timer
    // is still in a queue, so we try to take it out anyway
    bCanceled = KeCancelTimer( &m_TimerObj );
    if( bCanceled == FALSE )
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Timer to cancel was not in queue"));
    }
    memset(&m_TimerObj, 0, sizeof(m_TimerObj));

    m_bObjectStatus = FALSE;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates and Initializes the time out object. If there is a time out object
//  that was created before the old time out object will be deleted.
//  The handler of the abstract callback class is called after the period of
//  time (in milliseconds). If the bPeriodic flag is TRUE, the time out object
//  will be set to recurrent mode. The time for the period is equal to
//  dwTimeToWait. Time must be between 50 and 2000 msec. Caller must be
//  running at passive level.
//
// Return Value:
//  FALSE       Memory cannot be allocated. For more details see description
//              of operator new.
//  TRUE        Created the time out object with success.
//
//////////////////////////////////////////////////////////////////////////////
BOOL CAVDependTimeOut::InitializeTimeOut
(
    DWORD dwTimeToWait, //Specifies the absolute time at which the timer is to
                        //expire. The expiration time is expressed in system
                        //time units (one milliecond intervals). Absolute
                        //expiration times track any changes in the system
                        //time. Time must be between 50 and 2000 msec.
    BOOL bPeriodic,     //Set the time out object to recurrent mode. The time
                        //for the period is equal to dwTimeToWait.
    COSDependAbstractCB* pAbstractCBObj //Pointer to callback object.
)
{
    // check if parameter is valid
    if( !pAbstractCBObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Callback object invalid"));
        return FALSE;
    }
    // check if parameter is within valid range
    if( (dwTimeToWait < 50) || (dwTimeToWait > 2000) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Time to wait out of range"));
        return FALSE;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level wrong"));
        return FALSE;
    }
    // initialize DPC object
    KeInitializeDpc(&m_Dpc,
                    static_cast <PKDEFERRED_ROUTINE> (&TimerDPC),
                    static_cast <PVOID> (pAbstractCBObj) );
    // initialize timer object
    KeInitializeTimer( &m_TimerObj );

    LARGE_INTEGER Timeout;
    // Specify the relative (negative) time to wait in units
    // of 100 nanoseconds
    Timeout.QuadPart =
        (-1) * 10000 * static_cast <LONGLONG> (dwTimeToWait);
    //check if periodic time out is selected
    if( bPeriodic )
    {
        // set timer into queue, periodic was selected, so we use
        // KeSetTimerEx that allows us to set a periodic recall
        BOOL bSuccess = KeSetTimerEx(&m_TimerObj,
                                     Timeout,
                                     static_cast <LONG> (dwTimeToWait),
                                     &m_Dpc);
        // check if timer was in the queue already
        if( bSuccess == TRUE )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Timer already in the queue"));
            return FALSE;
        }
    }
    else
    {
        // set timer into queue, periodic was not selected, so the use
        // of KeSetTimer is sufficient
        BOOL bSuccess = KeSetTimer(&m_TimerObj,
                                   Timeout,
                                   &m_Dpc);
        // check if timer was in the queue already
        if( bSuccess == TRUE )
        {
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Timer already in the queue"));
            return FALSE;
        }
    }
    return TRUE;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The status of the time out object.
//
// Return Value:
//  TRUE               The object was created without errors,
//  FALSE              Otherwise
//
//////////////////////////////////////////////////////////////////////////////
BOOL CAVDependTimeOut::GetObjectStatus()
{
    return m_bObjectStatus;
};

