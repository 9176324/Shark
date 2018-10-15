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
#include "AVDependSpinLock.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependSpinLock::CAVDependSpinLock()
{
    m_dwSpinLock = 0;
    m_ucSpinLockLevel = 0;
    // initialize spin lock object before first call to acquire / release
    // is made, the system function does not support any return values so we
    // assume that it never fails
    KeInitializeSpinLock( &m_dwSpinLock );
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CAVDependSpinLock::~CAVDependSpinLock()
{
	// no default release spin lock to system
	m_dwSpinLock = 0;
    m_ucSpinLockLevel = 0;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Acquires a spin lock so the caller can synchronize the access to data in a
//  safe way by raising IRQL. 
//  Caller must be running at IRQL <= DISPATCH_LEVEL.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void CAVDependSpinLock::AcquireSpinLock()
{
    // check current IRQ level
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level too high"));
        return;
    }
    // acquire spin lock from system

    KeAcquireSpinLock( &m_dwSpinLock, &m_ucSpinLockLevel );
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Releases the spin lock and restores the original IRQL at which the caller
//  was running. Caller must be running at IRQL = DISPATCH_LEVEL.
//
// Return Value:
//  none.
//
//////////////////////////////////////////////////////////////////////////////
void CAVDependSpinLock::ReleaseSpinLock()
{
    // check current IRQ level
    if( KeGetCurrentIrql() != DISPATCH_LEVEL )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IRQ level wrong"));
        return;
    }
    // release spin lock to system

    KeReleaseSpinLock( &m_dwSpinLock, m_ucSpinLockLevel );
};

