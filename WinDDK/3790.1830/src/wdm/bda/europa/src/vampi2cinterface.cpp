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


#include "VampI2CInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Global pointer of the CVampI2CInterface object
//
//////////////////////////////////////////////////////////////////////////////
CVampI2CInterface* CVampI2CInterface::m_pVampI2CInterface=NULL;


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CVampI2CInterface::CVampI2CInterface
(
    IN CVampI2c* pVampI2C
)
{
    m_pVampI2C = pVampI2C;

    m_pVampI2CInterface = this;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CVampI2CInterface::~CVampI2CInterface()
{
    m_pVampI2C = NULL;
    m_pVampI2CInterface = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a static function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS STDMETHODCALLTYPE CVampI2CInterface::I2COpen
(
    PDEVICE_OBJECT pdo,
    ULONG ToOpen,
    PI2CControl ctrl
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: I2COpen called"));

    CVampI2CInterface* pThis = m_pVampI2CInterface;

    if( !pThis )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Missing VampI2CInterface object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        return pThis->OnI2COpen( pdo, ToOpen, ctrl );
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a static function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS STDMETHODCALLTYPE CVampI2CInterface::I2CAccess
(
    PDEVICE_OBJECT pdo,
    PI2CControl ctrl
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: I2CAccess called"));

    CVampI2CInterface* pThis = m_pVampI2CInterface;

    if( !pThis )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Missing VampI2CInterface object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        return pThis->OnI2CAccess( pdo, ctrl );
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a class function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampI2CInterface::OnI2COpen
(
    PDEVICE_OBJECT pdo,
    ULONG ToOpen,
    PI2CControl ctrl
)
{
    if( !m_pVampI2C )
    {
        ctrl->Status = I2C_STATUS_ERROR;
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    LARGE_INTEGER CurTime;
    KeQuerySystemTime( &CurTime );

    ctrl->Status = I2C_STATUS_NOERROR;

    // cookie is not NULL if I2C is open
    if( ToOpen && m_dwCurI2cCookie )
    {
        // Check time stamp against current time to detect if current
        // Cookie has timed out. If it has, remember the last timed out
        // cookie and grant the new requestor access.
        if( static_cast <DWORD>(CurTime.QuadPart - m_LastI2CAccessTime.QuadPart) >
            I2CClientTimeout )
        {
            m_dwExpiredI2cCookie = m_dwCurI2cCookie;
        }
        else
        {
            ctrl->dwCookie = 0;
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Timestamp error"));
            return STATUS_INVALID_HANDLE;
        }
        if( !m_pVampI2C->I2cErrorReset() )
        {
            ctrl->Status = I2C_STATUS_ERROR;
        }
    }

    // want to close ?
    if( !ToOpen )
    {
        if( m_dwCurI2cCookie == ctrl->dwCookie )
        {
            m_dwCurI2cCookie = 0;
            ctrl->dwCookie = 0;

            return STATUS_SUCCESS;
        }
        else
        {
            if( (m_dwExpiredI2cCookie != 0) &&
                (m_dwExpiredI2cCookie == ctrl->dwCookie) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }
            else
            {
                ctrl->dwCookie = 0;
            }
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Close failed"));
            return STATUS_INVALID_HANDLE;
        }
    }

    m_dwCurI2cCookie    = CurTime.LowPart;
    m_LastI2CAccessTime = CurTime;


    ctrl->dwCookie    = m_dwCurI2cCookie;
    ctrl->ClockRate   = m_pVampI2C->GetClockSel() * 1000;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a class function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampI2CInterface::OnI2CAccess
(
    PDEVICE_OBJECT pdo,
    PI2CControl ctrl
)
{
    if( !m_pVampI2C )
    {
        ctrl->Status = I2C_STATUS_ERROR;
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    ctrl->Status = I2C_STATUS_NOERROR;

    if( ctrl->dwCookie != m_dwCurI2cCookie )
    {
        if( (m_dwExpiredI2cCookie != 0) &&
            (m_dwExpiredI2cCookie == ctrl->dwCookie) )
        {
            ctrl->Status = I2C_STATUS_ERROR;
        }
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Time stamp error"));
        return STATUS_INVALID_HANDLE;
    }

    // Record time of this transaction to enable checking for timeout
    KeQuerySystemTime( &m_LastI2CAccessTime );

    // Check for valid combinations of I2C command & flags
    switch( ctrl->Command )
    {
    case I2C_COMMAND_NULL:
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_NOT_SUPPORTED;

    case I2C_COMMAND_READ:
        if( ctrl->Flags & ~(I2C_FLAGS_STOP | I2C_FLAGS_ACK) )
        {
            // Illegal combination of Command & Flags
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Illegal combination of command and flags"));
            return STATUS_INVALID_PARAMETER;
        }

        switch (ctrl->Flags & 0x0f)
        {
        case I2C_FLAGS_STOP:
            if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->ReadByte(&ctrl->Data, STOP)) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }
            break;
        default:
            if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->ReadByte(&ctrl->Data, CONTINUE)) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }
            break;
        }

        break;
    case I2C_COMMAND_WRITE:
        if( ctrl->Flags & ~(I2C_FLAGS_START | I2C_FLAGS_STOP |
                            I2C_FLAGS_ACK | I2C_FLAGS_DATACHAINING) )
        {
            // Illegal combination of Command & Flags
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Illegal combination of command and flags"));
            return STATUS_INVALID_PARAMETER;
        }

        switch (ctrl->Flags & 0x0f)
        {
        case I2C_FLAGS_START:
            // Always the 8-bit address
            m_pVampI2C->SetSlave( static_cast <int>(ctrl->Data & 0xFE) );

            if( ctrl->Data & ~0xFE )
            {
                // Read
                if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->ReadByte(&ctrl->Data, START)) )
                {
                    ctrl->Status = I2C_STATUS_ERROR;
                }
            }
            else
            {
                // Write
                m_pVampI2C->SetClockSel( ctrl->ClockRate / 1000 );
                if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->WriteByte(&ctrl->Data, START)) )
                {
                    ctrl->Status = I2C_STATUS_ERROR;
                }
            }

            break;
        case I2C_FLAGS_STOP:
            if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->WriteByte(&ctrl->Data, CONTINUE)) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }

            if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->WriteByte(&ctrl->Data, STOP)) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }

            break;
        default:
            if( m_pVampI2C->IsStatusError(
                            m_pVampI2C->WriteByte(&ctrl->Data, CONTINUE)) )
            {
                ctrl->Status = I2C_STATUS_ERROR;
            }

            break;
        }

        break;

    case I2C_COMMAND_STATUS:
        if( m_pVampI2C->IsStatusError(m_pVampI2C->GetI2cStatus()) )
        {
            ctrl->Status = I2C_STATUS_ERROR;
        }
        break;
    case I2C_COMMAND_RESET:
        if( !m_pVampI2C->I2cErrorReset() )
        {
            ctrl->Status = I2C_STATUS_ERROR;
        }
        break;

    default:
       _DbgPrintF(  DEBUGLVL_ERROR, ("Error: Invalid command"));
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}
