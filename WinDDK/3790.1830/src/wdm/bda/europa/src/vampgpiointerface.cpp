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


#include "VampGPIOInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Global pointer of the CVampGpioInterface object
//
//////////////////////////////////////////////////////////////////////////////
CVampGpioInterface* CVampGpioInterface::m_pVampGpioInterface=NULL;


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor
//
//////////////////////////////////////////////////////////////////////////////
CVampGpioInterface::CVampGpioInterface
(
    IN CVampGPIO* pVampGPIO
)
{
    m_pVampGPIO = pVampGPIO;

    m_pVampGpioInterface = this;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor
//
//////////////////////////////////////////////////////////////////////////////
CVampGpioInterface::~CVampGpioInterface()
{
    m_pVampGPIO = NULL;
    m_pVampGpioInterface = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a static function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS STDMETHODCALLTYPE CVampGpioInterface::GpioOpen
(
    PDEVICE_OBJECT pdo,
    ULONG ToOpen,
    PGPIOControl ctrl
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: GpioOpen called"));

    CVampGpioInterface* pThis = m_pVampGpioInterface;

    if( !pThis )
    {
       _DbgPrintF(DEBUGLVL_ERROR,("Error: Missing VampGpioInterface object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        return pThis->OnGpioOpen( pdo, ToOpen, ctrl );
    }
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a static function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS STDMETHODCALLTYPE CVampGpioInterface::GpioAccess
(
    PDEVICE_OBJECT pdo,
    PGPIOControl ctrl
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: GpioAccess called"));

    CVampGpioInterface* pThis = m_pVampGpioInterface;

    if( !pThis )
    {
       _DbgPrintF(DEBUGLVL_ERROR,("Error: Missing VampGpioInterface object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        return pThis->OnGpioAccess( pdo, ctrl );
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a class function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampGpioInterface::OnGpioOpen
(
    PDEVICE_OBJECT pdo,
    ULONG ToOpen,
    PGPIOControl ctrl
)
{
    ctrl->Status = GPIO_STATUS_ERROR;

    // This is the old open command and it is not supported any more
    if( ctrl->Command == GPIO_COMMAND_OPEN )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_NOT_SUPPORTED;
    }

    // Don't know what to do with query ????
    if( ctrl->Command == GPIO_COMMAND_QUERY )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_NOT_SUPPORTED;
    }

    // Check open or close conditions
    if( (ctrl->Command != GPIO_COMMAND_OPEN_PINS) &&
        (ctrl->Command != GPIO_COMMAND_CLOSE_PINS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_INVALID_PARAMETER;
    }

    if( (ctrl->Command == GPIO_COMMAND_OPEN_PINS) && !ToOpen )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_INVALID_PARAMETER;
    }

    if( (ctrl->Command == GPIO_COMMAND_CLOSE_PINS) && ToOpen )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_INVALID_PARAMETER;
    }

    LARGE_INTEGER CurTime;
    KeQuerySystemTime( &CurTime );

    ctrl->Status = GPIO_STATUS_NOERROR;

    // cookie is not NULL if Gpio is open
    if( ToOpen && m_dwCurGpioCookie )
    {
        // Check time stamp against current time to detect if current
        // Cookie has timed out. If it has remember the last timed out
        // cookie and grant the new requestor access.
        if( static_cast <DWORD>(CurTime.QuadPart - m_LastGpioAccessTime.QuadPart) >
            GpioClientTimeout )
        {
            m_dwExpiredGpioCookie = m_dwCurGpioCookie;
        }
        else
        {
            ctrl->dwCookie = 0;
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Timestamp error"));
            return STATUS_INVALID_HANDLE;
        }
    }

    // want to close ?
    if( !ToOpen )
    {
        if( m_dwCurGpioCookie == ctrl->dwCookie )
        {
            m_dwCurGpioCookie = 0;
            ctrl->dwCookie = 0;

            return STATUS_SUCCESS;
        }
        else
        {
            if( (m_dwExpiredGpioCookie != 0) &&
                (m_dwExpiredGpioCookie == ctrl->dwCookie) )
            {
                ctrl->Status = GPIO_STATUS_ERROR;
            }
            else
            {
                ctrl->dwCookie = 0;
            }
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Close failed"));
            return STATUS_INVALID_HANDLE;
        }
    }

    m_dwCurGpioCookie    = CurTime.LowPart;
    m_LastGpioAccessTime = CurTime;

    ctrl->dwCookie = m_dwCurGpioCookie;
    ctrl->nPins    = 28;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is a class function.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CVampGpioInterface::OnGpioAccess
(
    PDEVICE_OBJECT pdo,
    PGPIOControl ctrl
)
{
    VAMPRET retValue = VAMPRET_SUCCESS;
    ctrl->Status = GPIO_STATUS_ERROR;

    if( !m_pVampGPIO )
    {
        ctrl->Status = GPIO_STATUS_ERROR;
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    // SAA7134 supports 28 pins
//    if( ctrl->Flags != GPIO_FLAGS_BYTE )
//  {
//        _DbgPrintF(   DEBUGLVL_ERROR,("Error: Wrong flags"));
//        return STATUS_NOT_SUPPORTED;
//    }
//
//  if( ctrl->nBytes != 1 )
//    {
//        _DbgPrintF(   DEBUGLVL_ERROR,("Error: Number of bytes not supported"));
//      return STATUS_NOT_SUPPORTED;
//  }

    if( ctrl->nBytes > 4 )
    {
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Number of bytes not supported"));
        return STATUS_NOT_SUPPORTED;
    }

    ctrl->Status = GPIO_STATUS_NOERROR;

    if( ctrl->dwCookie != m_dwCurGpioCookie )
    {
        if( (m_dwExpiredGpioCookie != 0) &&
            (m_dwExpiredGpioCookie == ctrl->dwCookie) )
        {
            ctrl->Status = GPIO_STATUS_ERROR;
        }
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Time stamp error"));
        return STATUS_INVALID_HANDLE;
    }

    // Record time of this transaction to enable checking for timeout
    KeQuerySystemTime( &m_LastGpioAccessTime );

    if( ctrl->AsynchCompleteCallback )
    {
        ctrl->Status = GPIO_STATUS_NO_ASYNCH;
    }

    // Perform Gpio command
    switch( ctrl->Command )
    {
      case GPIO_COMMAND_READ_BUFFER:
        retValue = m_pVampGPIO->ReadFromGPIONr( *ctrl->Pins,ctrl->Buffer );
        break;

      case GPIO_COMMAND_WRITE_BUFFER:
        retValue = m_pVampGPIO->WriteToGPIONr( *ctrl->Pins,*ctrl->Buffer );
        break;

      default:
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Command not supported"));
        return STATUS_NOT_SUPPORTED;
    }

    if( retValue == VAMPRET_SUCCESS )
    {
        return STATUS_SUCCESS;
    }
    else
    {
        _DbgPrintF( DEBUGLVL_ERROR,("Error: HAL access failed"));
        return STATUS_UNSUCCESSFUL;
    }
}

