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


#include "34avstrm.h"
#include "AnlgXBarFilter.h"
#include "EuropaGuids.h"
#include "Mediums.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgXBarFilter::CAnlgXBarFilter()
{
    m_pAnlgPropXBar = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor. Deletes the pointer to the CAnlgPropXBar class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgXBarFilter::~CAnlgXBarFilter()
{
    if( m_pAnlgPropXBar )
    {
        delete m_pAnlgPropXBar;
        m_pAnlgPropXBar = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Initializes the Crossbar Filter. It also creates an instance of the
//  CAnlgPropXBar class and stores the pointer to a member variable.
//
// Return Value:
//  STATUS_UNSUCCESSFUL             Operation failed,
//  STATUS_INSUFFICIENT_RESOURCES   AnlgPropXBar object creation failed,
//  STATUS_SUCCESS                  Crossbar filter has been initialized
//                                  successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgXBarFilter::Create
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pAnlgPropXBar )
    {
        m_pAnlgPropXBar = new (NON_PAGED_POOL) CAnlgPropXBar();
    }

    if( !m_pAnlgPropXBar )
    {
        _DbgPrintF(
                DEBUGLVL_ERROR,
                ("Error: AnlgPropXBar creation failed, not enough memory"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  De-initializes the analog crossbar filter properties.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Crossbar filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgXBarFilter::Close
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pAnlgPropXBar )
    {
        delete m_pAnlgPropXBar;
        m_pAnlgPropXBar = NULL;
    }

    return STATUS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the pointer to the CAnlgPropXBar class.
//
// Return Value:
//  Pointer to the CAnlgPropXBar class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropXBar*  CAnlgXBarFilter::GetPropXBar()
{
    return m_pAnlgPropXBar;
}

