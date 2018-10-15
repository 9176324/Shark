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


#include "AnlgTunerFilter.h"
#include "EuropaGuids.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgTunerFilter::CAnlgTunerFilter()
{
    m_pAnlgPropTuner    = NULL;
    m_pAnlgMethodsTuner = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor. Deletes the pointer to the CAnlgPropTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgTunerFilter::~CAnlgTunerFilter()
{
    if( m_pAnlgPropTuner )
    {
        delete m_pAnlgPropTuner;
        m_pAnlgPropTuner = NULL;
    }
    if( m_pAnlgMethodsTuner )
    {
        delete m_pAnlgMethodsTuner;
        m_pAnlgMethodsTuner = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Initializes the Analog Tuner Filter 
//
// Return Value:
//  STATUS_UNSUCCESSFUL             Operation failed,
//  STATUS_INSUFFICIENT_RESOURCES   AnlgPropTuner or AnlgMethodsTuner object
//                                  creation failed,
//  STATUS_SUCCESS                  Crossbar filter has been initialized
//                                  successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgTunerFilter::Create
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //Create property class
    if( !m_pAnlgPropTuner )
    {
        m_pAnlgPropTuner = new (NON_PAGED_POOL) CAnlgPropTuner();
    }

    if( !m_pAnlgPropTuner )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgPropTuner creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //Create method class
    if( !m_pAnlgMethodsTuner )
    {
        m_pAnlgMethodsTuner = new (NON_PAGED_POOL) CAnlgMethodsTuner();
    }

    if( !m_pAnlgMethodsTuner )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgMethodsTuner creation failed, not enough memory"));
        delete m_pAnlgPropTuner;
        m_pAnlgPropTuner = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  De-initializes the analog Tuner Filter properties and methods.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Crossbar filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgTunerFilter::Close
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pAnlgPropTuner )
    {
        delete m_pAnlgPropTuner;
        m_pAnlgPropTuner = NULL;
    }

    if( m_pAnlgMethodsTuner )
    {
        delete m_pAnlgMethodsTuner;
        m_pAnlgMethodsTuner = NULL;
    }

    return STATUS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the pointer to the CAnlgPropTuner class.
//
// Return Value:
//  Pointer to the CAnlgPropTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTuner*  CAnlgTunerFilter::GetPropTuner()
{
    return m_pAnlgPropTuner;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the pointer to the CAnlgMethodsTuner class.
//
// Return Value:
//  Pointer to the CAnlgPropTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgMethodsTuner*  CAnlgTunerFilter::GetMethodsTuner()
{
    return m_pAnlgMethodsTuner;
}
