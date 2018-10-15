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
#include "BaseStream.h"
#include "AnlgCaptureFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CBaseStream::CBaseStream()
{
    m_pKSPin = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CBaseStream::~CBaseStream()
{
    m_pKSPin = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the Analog Event Handler object
//  This function identifies the filter that belongs to the given pin
//  and returns an pointer to the filters event handler instance.
//
// Return Value:
//  CAnlgEventHandler*      Pointer to a CAnlgEventHandler object.
//  NULL                    Operation failed,
//                          (a) Base stream pin not set or invalid
//                          (b) No parent filter found
//                          (c) No event handler found
//
//////////////////////////////////////////////////////////////////////////////
CAnlgEventHandler* CBaseStream::GetEventHandler()
{
    if( !m_pKSPin )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Base stream pin not set or invalid"));
        return NULL;
    }

    PKSFILTER pKSFilter = KsPinGetParentFilter(m_pKSPin);

    CBaseFilter* pParentFilter =
        static_cast <CBaseFilter*>(pKSFilter->Context);
    if( !pParentFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No parent filter found"));
        return NULL;
    }

    CAnlgEventHandler* pEventHandler = pParentFilter->GetEventHandler();
    if( !pEventHandler )
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: No event handler found"));
        return NULL;
    }
    return pEventHandler;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the associated pin object
//
// Return Value: m_pKSPin
//////////////////////////////////////////////////////////////////////////////
PKSPIN CBaseStream::GetKSPin() 
{
    return m_pKSPin;
}
