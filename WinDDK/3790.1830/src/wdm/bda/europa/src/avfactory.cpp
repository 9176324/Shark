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
#include "AVFactory.h"
#include "AVDependDelayExecution.h"
#include "AVDependQueryPerformanceCounter.h"
#include "AVDependSpinLock.h"
#include "AVDependTimeOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of COSDependSpinLock. We do not allow paged pool
//  allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the COSDependSpinLock object
//
//////////////////////////////////////////////////////////////////////////////
COSDependSpinLock* CVampFactory::CreateSpinLock
(
    eVampPoolType tPoolType //The memory pool type, must be NON_PAGED_POOL.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CreateSpinLock called"));
    // check if parameter valid
    if( tPoolType != NON_PAGED_POOL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // return allocated spin lock object, if the memory is not available NULL
    // is returned by the new operator
    return new (tPoolType) CAVDependSpinLock();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of COSDependTimeOut. We do not allow paged pool
//  allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the COSDependTimeOut object
//
//////////////////////////////////////////////////////////////////////////////
COSDependTimeOut* CVampFactory::CreateTimeOut
(
    eVampPoolType tPoolType //The memory pool type, must be NON_PAGED_POOL.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CreateTimeOut called"));
    // check if parameter valid
    if( tPoolType != NON_PAGED_POOL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // return allocated time out object, if the memory is not available NULL
    // is returned by the new operator
    return new (tPoolType) CAVDependTimeOut();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of COSDependQueryPerformanceCounter. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the COSDependQueryPerformanceCounter object
//
//////////////////////////////////////////////////////////////////////////////
COSDependQueryPerformanceCounter* CVampFactory::CreateQueryPerformanceCounter
(
    eVampPoolType tPoolType //The memory pool type, must be NON_PAGED_POOL.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CreateQueryPerformanceCounter called"));
    // check if parameter valid
    if( tPoolType != NON_PAGED_POOL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // return allocated performance counter object, if the memory is not
    // available NULL is returned by the new operator
    return new (tPoolType) CAVDependQueryPerformanceCounter();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of COSDependDelayExecution. We do not allow
//  paged pool allocations here, because it does not make any sense.
//
// Return Value:
//  Pointer of the COSDependDelayExecution object
//
//////////////////////////////////////////////////////////////////////////////
COSDependDelayExecution* CVampFactory::CreateDelayExecution
(
    eVampPoolType tPoolType //The memory pool type, must be NON_PAGED_POOL.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CreateDelayExecution called"));
    // check if parameter valid
    if( tPoolType != NON_PAGED_POOL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }
    // return allocated delay execution object, if the memory is not
    // available NULL is returned by the new operator
    return new (tPoolType) CAVDependDelayExecution();
};

