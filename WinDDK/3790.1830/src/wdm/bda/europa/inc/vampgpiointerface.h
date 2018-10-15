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
#include "VampGPIO.h"
#include <i2cgpio.h>

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class allows access through the GPIO interface of the VAMP device.
//
//////////////////////////////////////////////////////////////////////////////
class CVampGpioInterface
{
public:
    CVampGpioInterface(IN CVampGPIO* pVampGPIO);
    ~CVampGpioInterface();

    static CVampGpioInterface* m_pVampGpioInterface;

    static NTSTATUS STDMETHODCALLTYPE GpioOpen( PDEVICE_OBJECT Device,
                                                ULONG ToOpen,
                                                PGPIOControl ctrl);
    static NTSTATUS STDMETHODCALLTYPE GpioAccess(   PDEVICE_OBJECT pdo,
                                                    PGPIOControl ctrl);

private:
    NTSTATUS OnGpioOpen(PDEVICE_OBJECT Device,
                        ULONG ToOpen,
                        PGPIOControl ctrl);
    NTSTATUS OnGpioAccess(  PDEVICE_OBJECT pdo,
                            PGPIOControl ctrl);

    CVampGPIO* m_pVampGPIO;

    enum {GpioClientTimeout = 10000000}; // Timeout after 1 s

    DWORD         m_dwCurGpioCookie;
    DWORD         m_dwExpiredGpioCookie;
    LARGE_INTEGER m_LastGpioAccessTime;
};
