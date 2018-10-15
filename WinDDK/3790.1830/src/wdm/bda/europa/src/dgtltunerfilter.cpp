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


#include "DgtlTunerFilter.h"
#include "DgtlTunerFilterInterface.h"
#include "AVDependDelayExecution.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTunerFilter::CDgtlTunerFilter()
{
    m_pDgtlPropTuner    = NULL;
    m_pDgtlMethodsTuner = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor. Deletes the pointer to the CDgtlMethodsTuner and
//  CDgtlMethodsTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTunerFilter::~CDgtlTunerFilter()
{
    if( m_pDgtlPropTuner )
    {
        delete m_pDgtlPropTuner;
        m_pDgtlPropTuner = NULL;
    }
    if( m_pDgtlMethodsTuner )
    {
        delete m_pDgtlMethodsTuner;
        m_pDgtlMethodsTuner = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Initializes the digital Tuner Filter properties and methods.
//
// Return Value:
//  STATUS_UNSUCCESSFUL             Operation failed,
//  STATUS_INSUFFICIENT_RESOURCES   AnlgPropTuner or AnlgMethodsTuner object
//                                  creation failed,
//  STATUS_SUCCESS                  Filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTunerFilter::Create
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //  Initialize this filter instance 
    NTSTATUS Status = BdaInitFilter(pKSFilter, NULL);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: BdaInitFilter failed"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pDgtlPropTuner )
    {
        m_pDgtlPropTuner = new (NON_PAGED_POOL) CDgtlPropTuner();
    }

    if( !m_pDgtlPropTuner )
    {
        //memory allocation failed
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: DgtlPropTuner creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( !m_pDgtlMethodsTuner )
    {
        m_pDgtlMethodsTuner = new (NON_PAGED_POOL) CDgtlMethodsTuner();
    }

    if( !m_pDgtlMethodsTuner )
    {
        //memory allocation failed
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: DgtlMethodsTuner creation failed, not enough memory"));
        delete m_pDgtlPropTuner;
        m_pDgtlPropTuner = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  De-initializes the digital Tuner Filter properties and methods.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Crossbar filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTunerFilter::Close
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pDgtlPropTuner )
    {
        delete m_pDgtlPropTuner;
        m_pDgtlPropTuner = NULL;
    }

    if( m_pDgtlMethodsTuner )
    {
        delete m_pDgtlMethodsTuner;
        m_pDgtlMethodsTuner = NULL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the pointer to the CDgtlPropTuner class.
//
// Return Value:
//  Pointer to the CDgtlPropTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlPropTuner*  CDgtlTunerFilter::GetPropTuner()
{
    return m_pDgtlPropTuner;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Returns the pointer to the CDgtlMethodsTuner class.
//
// Return Value:
//  Pointer to the CDgtlPropTuner class.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlMethodsTuner*  CDgtlTunerFilter::GetMethodsTuner()
{
    return m_pDgtlMethodsTuner;
}


KSSTATE CDgtlTunerFilter::GetClientState
(
)
{
    return m_TunerOutClientState;
}

VOID CDgtlTunerFilter::SetClientState
(
    IN KSSTATE ClientState
)
{
    m_TunerOutClientState = ClientState;
}
