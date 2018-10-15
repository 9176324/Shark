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
#include "AVDependQueryPerformanceCounter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependQueryPerformanceCounter::CAVDependQueryPerformanceCounter()
{
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependQueryPerformanceCounter::~CAVDependQueryPerformanceCounter()
{
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  this function provides the finest grained running count available in the
//  system.
//
// Return Value:
//  Returns the performance counter value in units of ticks
//
//////////////////////////////////////////////////////////////////////////////
LONGLONG CAVDependQueryPerformanceCounter::QueryPerformanceCounter()
{
    LARGE_INTEGER liFreq;
    // initialize variable
    liFreq.QuadPart = 0;
    // get current time dependant on processor clock
    LARGE_INTEGER liTime = KeQueryPerformanceCounter( &liFreq );
    // avoid zero division if processor clock is not available (very unlikely)
    if(liFreq.QuadPart == 0)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No processor clock found"));
        return 0;
    }
    // cast processor clock to unsigned longlong
    ULONGLONG ullCastedProcessorClock =
        static_cast <ULONGLONG> (liFreq.QuadPart);
    // convert time into a format that is independant to processor clock
    ULONGLONG ullConvertedTime =
        KSCONVERT_PERFORMANCE_TIME(ullCastedProcessorClock, liTime);
    // return curent time, calculated independant to processor clock

    return ( static_cast <LONGLONG> (ullConvertedTime / 10000) );
};


