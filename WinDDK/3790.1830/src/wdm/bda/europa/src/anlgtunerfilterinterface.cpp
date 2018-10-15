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
#include "AnlgTunerFilterInterface.h"
#include "AnlgTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the analog tuner filter.
//  This function creates an instance of the analog tuner filter
//  (CAnlgTunerFilter) and stores the pointer to the filter context.
//  It also initializes the analog tuner filter with the class function
//  InitTunerFilter() of the CAnlgTunerFilter class.
//
// Return Value:
//  STATUS_SUCCESS                  Analog tuner filter has been created
//                                  successfully
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) if the function parameters are zero,
//                                  (b) if the propetry classes could not
//                                  be created
//  STATUS_INSUFFICIENT_RESOURCES   NEW operation failed. Analog tuner filter
//                                  could't be created.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFilterCreate called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgTunerFilter* pAnlgTunerFilter =
                            new (NON_PAGED_POOL) CAnlgTunerFilter();
    if( !pAnlgTunerFilter )
    {
        //memory allocation failed
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AnlgTunerFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pAnlgTunerFilter->Create(pKSFilter) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilter creation failed"));
        delete pAnlgTunerFilter;
        pAnlgTunerFilter = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the AnlgTunerFilter object for later use in the
    //Context of KSFilter
    pKSFilter->Context = pAnlgTunerFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the close dispatch function of the analog tuner filter.
//  This function deinitializes the instance of the analog tuner filter
//  (CAnlgTunerFilter), which was stored on the filter context. It also sets
//  the filter context to NULL (there is no BdaDeInitFilter function).
//
// Return Value:
//  STATUS_SUCCESS          Closed the analog tuner filter with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgTunerFilter filter pointer is zero
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFilterClose called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the AnlgTunerFilter object out of the Context of KSFilter
    CAnlgTunerFilter* pAnlgTunerFilter =
                        static_cast <CAnlgTunerFilter*>(pKSFilter->Context);
    if( !pAnlgTunerFilter )
    {
        //no AnlgTunerFilter object available
        _DbgPrintF(
          DEBUGLVL_ERROR,
          ("Error: Close analog tuner filter failed, no AnlgTunerFilter found"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS Status = pAnlgTunerFilter->Close(pKSFilter);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close analog tuner filter failed"));
        //de-allocate memory for our AnlgTunerFilter class
        delete pAnlgTunerFilter;
        //remove the AnlgTunerFilter object from the context structure of KSFilter
        pKSFilter->Context = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //de-allocate memory for our AnlgTunerFilter class
    delete pAnlgTunerFilter;
    //remove the AnlgTunerFilter object from the context structure of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}

