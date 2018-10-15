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
#include "AnlgTVAudioFilterInterface.h"
#include "AnlgTVAudioFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the TV audio filter. This function
//  initializes an instance of the analog TV audio filter (CAnlgTVAudioFilter)
//  and stores the pointer to the filter context. It also initializes the
//  TV audio filter with the class function InitTVAudioFilter of the
//  CAnlgTVAudioFilter class.
//
// Return Value:
//  STATUS_SUCCESS                  Created the TV audio Filter with success
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) if the function parameters are zero,
//                                  (b) if the propetry classes could not
//                                  be created
//  STATUS_INSUFFICIENT_RESOURCES   NEW operation failed. TVAudio filter
//                                  could't be created.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Create Analog TVAudio Filter called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgTVAudioFilter* pAnlgTVAudioFilter =
                            new (NON_PAGED_POOL) CAnlgTVAudioFilter();
    if( !pAnlgTVAudioFilter )
    {
        //memory allocation failed
        _DbgPrintF(
           DEBUGLVL_ERROR,
           ("Error: AnlgTVAudioFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pAnlgTVAudioFilter->Create(pKSFilter) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTVAudioFilter creation failed"));
        delete pAnlgTVAudioFilter;
        pAnlgTVAudioFilter = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the AnlgTVAudioFilter object for later use in the
    //Context of KSFilter
    pKSFilter->Context = pAnlgTVAudioFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the close dispatch function of the TV audio filter. This function
//  deinitializes the instance of the analog TV audio filter
//  (CAnlgTVAudioFilter) which was stored on the filter context.
//  It also sets the filter context to NULL.
//  (there is no BdaDeInitFilter function).
//
// Return Value:
//  STATUS_SUCCESS          Closed the TV audio Filter with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgTVAudio filter pointer is zero
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Close Analog TVAudio Filter called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the AnlgTVAudioFilter object out of
    //the Context of KSFilter
    CAnlgTVAudioFilter* pAnlgTVAudioFilter =
                       static_cast <CAnlgTVAudioFilter*>(pKSFilter->Context);
    if( !pAnlgTVAudioFilter )
    {
        //no analog TVAudio filter object available
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: Close analog TVAudio filter failed, no AnlgTVAudioFilter found"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS Status = pAnlgTVAudioFilter->Close(pKSFilter);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close analog TVAudio filter failed"));
        //de-allocate memory for our pnlgTVAudioFilter class
        delete pAnlgTVAudioFilter;
        //remove the pAnlgTVAudioFilter object from the context structure
        //of KSFilter
        pKSFilter->Context = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //de-allocate memory for our AnlgTVAudioFilter class
    delete pAnlgTVAudioFilter;
    //remove the AnlgTVAudioFilter object from the context structure
    //of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}



