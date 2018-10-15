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


#include "AnlgTVAudioFilter.h"
#include "EuropaGuids.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgTVAudioFilter::CAnlgTVAudioFilter()
{
    m_pAnlgPropTVAudio = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor. Deletes the pointer to the CAnlgPropTVAudio class.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgTVAudioFilter::~CAnlgTVAudioFilter()
{
    if( m_pAnlgPropTVAudio )
    {
        delete m_pAnlgPropTVAudio;
        m_pAnlgPropTVAudio = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Initializes the TV audio Filter. It also creates an instance of the
//  CAnlgPropTVAudio class and stores the pointer to a member variable.
//
// Return Value:
//  STATUS_SUCCESS                  Initialized the TV audio filter with
//                                  success.
//  STATUS_INSUFFICIENT_RESOURCES   AnlgPropTuner or AnlgMethodsTuner object
//                                  creation failed,
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  if the function parameter is zero 
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgTVAudioFilter::Create
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pAnlgPropTVAudio )
    {
        m_pAnlgPropTVAudio = new (NON_PAGED_POOL) CAnlgPropTVAudio();
    }

    if( !m_pAnlgPropTVAudio )
    {
        //memory allocation failed
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgPropTVAudio creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  De-initializes the analog TVAudio Filter properties.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Crossbar filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgTVAudioFilter::Close
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pAnlgPropTVAudio )
    {
        delete m_pAnlgPropTVAudio;
        m_pAnlgPropTVAudio = NULL;
    }

    return STATUS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropTVAudio class.
//  (The Property is accessed via the Filter, due to possible
//  multiple property classes within a Filter.)
//
// Return Value:
//  Not NULL        Pointer to the CAnlgPropTVAudio class
//  NULL            otherwise
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTVAudio* CAnlgTVAudioFilter::GetPropTVAudio()
{
    return m_pAnlgPropTVAudio;
}

