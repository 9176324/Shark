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

#include "34AVStrm.h"
#include "VampI2c.h"
#include <i2cgpio.h>

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class allows access through the I2C interface of the VAMP device.
//
//////////////////////////////////////////////////////////////////////////////
class CVampI2CInterface
{
public:
    CVampI2CInterface(IN CVampI2c* pVampI2C);
    ~CVampI2CInterface();

    static CVampI2CInterface* m_pVampI2CInterface;

    static NTSTATUS STDMETHODCALLTYPE I2COpen(  PDEVICE_OBJECT Device,
                                                ULONG ToOpen,
                                                PI2CControl ctrl);

    static NTSTATUS STDMETHODCALLTYPE I2CAccess(PDEVICE_OBJECT pdo,
                                                PI2CControl ctrl);

private:
    NTSTATUS OnI2COpen( PDEVICE_OBJECT Device,
                        ULONG ToOpen,
                        PI2CControl ctrl);

    NTSTATUS OnI2CAccess(   PDEVICE_OBJECT pdo,
                            PI2CControl ctrl);

    CVampI2c* m_pVampI2C;

    enum {I2CClientTimeout  = 10000000}; // Timeout after 1 s

    DWORD         m_dwCurI2cCookie;
    DWORD         m_dwExpiredI2cCookie;
    LARGE_INTEGER m_LastI2CAccessTime;
};
