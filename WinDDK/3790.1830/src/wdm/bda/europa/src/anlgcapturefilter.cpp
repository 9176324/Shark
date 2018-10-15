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


#include "AnlgCaptureFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor of the analog capture filter.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgCaptureFilter::CAnlgCaptureFilter()
{
    m_pAnlgPropVideoDecoder = NULL;
    m_pAnlgPropProcamp      = NULL;
    m_pAnlgEventHandler     = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The destructor deletes the memory previously allocated by the contructor.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgCaptureFilter::~CAnlgCaptureFilter()
{
    if( m_pAnlgPropVideoDecoder )
    {
        delete m_pAnlgPropVideoDecoder;
    }
    if( m_pAnlgPropProcamp )
    {
        delete m_pAnlgPropProcamp;
    }
    if( m_pAnlgEventHandler )
    {
        delete m_pAnlgEventHandler;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates an instance of the CAnlgPropTVAudio class and CAnlgPropProcamp
//  and stores the pointer to member variables. It also creates an instance
//  of the CAnlgPropTVAudio class and CAnlgPropProcamp and stores the pointer
//  to member variables.
//
// Return Value:
//  STATUS_SUCCESS                  Initialized the analog capture filter
//                                  with success.
//  STATUS_INSUFFICIENT_RESOURCES   (a) An instance of the CAnlgPropTVAudio
//                                  or CAnlgPropProcamp already exists or
//                                  there is not enough memory to create it.
//                                  (b)An instance of the CAnlgEventHandler
//                                  already exists or there is not enough
//                                  memory to create it.
//  STATUS_UNSUCCESSFUL             Operation failed or Event Handler
//                                  initialisation failed.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgCaptureFilter::Create
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // Create properties
    if( !m_pAnlgPropVideoDecoder )
    {
        m_pAnlgPropVideoDecoder = new (NON_PAGED_POOL) CAnlgPropVideoDecoder();
    }

    if( !m_pAnlgPropVideoDecoder )
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: AnlgPropVideoDecoder creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( !m_pAnlgPropProcamp )
    {
        m_pAnlgPropProcamp = new (NON_PAGED_POOL) CAnlgPropProcamp();
    }

    if( !m_pAnlgPropProcamp )
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: AnlgPropProcamp creation failed, not enough memory"));
        delete m_pAnlgPropVideoDecoder;
        m_pAnlgPropVideoDecoder = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
#if 0
    // Create Event Handler
    if( !m_pAnlgEventHandler )
    {
        m_pAnlgEventHandler = new (NON_PAGED_POOL) CAnlgEventHandler();
    }

    if( !m_pAnlgEventHandler )
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: AnlgEventHandler creation failed, not enough memory"));

        delete m_pAnlgPropVideoDecoder;
        m_pAnlgPropVideoDecoder = NULL;

        delete m_pAnlgPropProcamp;
        m_pAnlgPropProcamp = NULL;

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( m_pAnlgEventHandler->Initialize(pKSFilter) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: AnlgEventHandler initialisation failed"));

        delete m_pAnlgPropVideoDecoder;
        m_pAnlgPropVideoDecoder = NULL;

        delete m_pAnlgPropProcamp;
        m_pAnlgPropProcamp = NULL;

        delete m_pAnlgEventHandler;
        m_pAnlgEventHandler = NULL;

        return STATUS_UNSUCCESSFUL;
    }
#endif

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  De-initializes the analog capture filter.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//  STATUS_SUCCESS          Crossbar filter has been initialized successfully
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgCaptureFilter::Close
(
    IN PKSFILTER pKSFilter  // Pointer to KSFILTER
)
{
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pAnlgPropVideoDecoder )
    {
        delete m_pAnlgPropVideoDecoder;
        m_pAnlgPropVideoDecoder = NULL;
    }

    if( m_pAnlgPropProcamp )
    {
        delete m_pAnlgPropProcamp;
        m_pAnlgPropProcamp = NULL;
    }

    if( m_pAnlgEventHandler )
    {
        delete m_pAnlgEventHandler;
        m_pAnlgEventHandler = NULL;
    }

    return STATUS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to access the Video Decoder property class.
//
// Return Value:
//  CAnlgPropVideoDecoder* Pointer to the video decoder object.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropVideoDecoder* CAnlgCaptureFilter::GetPropVideoDecoder()
{
    return m_pAnlgPropVideoDecoder;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to access the Propamp Property class.
//
// Return Value:
//  CAnlgPropProcamp* Pointer to the video decoder object.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropProcamp* CAnlgCaptureFilter::GetPropProcamp()
{
    return m_pAnlgPropProcamp;
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to access the Event handler class.
//
// Return Value:
//  CAnlgEventHandler* Pointer to the event handler object.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgEventHandler* CAnlgCaptureFilter::GetEventHandler()
{
    return m_pAnlgEventHandler;
}

