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

#include "34AVStrm.h"
#include "OSDepend.h"
#include "AVDependDelayExecution.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependDelayExecution::CAVDependDelayExecution()
{
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependDelayExecution::~CAVDependDelayExecution()
{
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This method blocks the execution of the current thread for the required
//  time in milliseconds. The precision is better than 20 milliseconds.
//  Can only be called at IRQL = PASSIVE_LEVEL
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void CAVDependDelayExecution::DelayExecutionThread
(
    DWORD dwMilliSec    // Required time in units of milliseconds.
                        // The value must be within the range of 1 to 4000.
)
{
    // check if parameter is within valid range
    if( (dwMilliSec < 1) || (dwMilliSec > 4000) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Parameter out of range"));
        return;
    }
    // check current IRQ level
    if( KeGetCurrentIrql() != PASSIVE_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level not correct"));
        return;
    }
    LARGE_INTEGER SysTimeout;
    // calculate the relative (negative) time to wait in units
    // of 100 nanoseconds
    SysTimeout.QuadPart =
        (-1) * 10000 * static_cast <LONGLONG> (dwMilliSec);
    // call KeDelayExecutionThread, it is a system function that delays
    // actual thread execution for the specified time
    NTSTATUS ntStatus =
        KeDelayExecutionThread((static_cast <char> (KernelMode)),
                               FALSE,
                               &SysTimeout );
    // check if function call was successful
    if(ntStatus != STATUS_SUCCESS)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Delay execution thread failed"));
        return;
    }
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This method stops the CPU activity for the required time in real time
//  clock ticks (0.8 microseconds). The precision is better than 2.4
//  microseconds. Drivers that call this routine should minimize
//  the number of microseconds they specify (no more than 50).
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void CAVDependDelayExecution::DelayExecutionSystem
(
    WORD wMicroSec  // Microseconds to stop the CPU activities,
                    // The value must be within the range of 1 to 50.
)
{
    // check if parameter is within valid range
    if( (wMicroSec < 1) || (wMicroSec > 50) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Parameter out of range"));
        return;
    }
    // call KeStallExecutionProcessor, it is a processor-dependent
    // routine that busy-waits for at least the specified number of
    // microseconds, but not significantly longer
    KeStallExecutionProcessor( wMicroSec );
};
