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
#include "DgtlTunerFilterInterface.h"
#include "DgtlTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the digital tuner filter.
//  This function creates an instance of the digital tuner filter
//  (CDgtlTunerFilter) and stores the pointer to the filter context.
//  It also initializes the digital tuner filter with the class function
//  InitTunerFilter() of the CDgtlTunerFilter class.
//
// Return Value:
//  STATUS_SUCCESS                  Digital tuner filter has been created
//                                  successfully
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) if the function parameters are zero,
//                                  (b) if the propetry classes could not
//                                  be created
//  STATUS_INSUFFICIENT_RESOURCES   NEW operation failed. Digital tuner filter
//                                  could't be created.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerFilterCreate called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlTunerFilter* pDgtlTunerFilter =
                                    new (NON_PAGED_POOL) CDgtlTunerFilter();
    if( !pDgtlTunerFilter )
    {
        //memory allocation failed
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: DgtlTunerFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pDgtlTunerFilter->Create(pKSFilter) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTunerFilter creation failed"));
        delete pDgtlTunerFilter;
        pDgtlTunerFilter = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the DgtlTunerFilter object for later use in the
    //Context of KSFilter
    pKSFilter->Context = pDgtlTunerFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the close dispatch function of the digital tuner filter.
//  This function deinitializes the instance of the digital tuner filter
//  (CDgtlTunerFilter), which was stored on the filter context. It also sets
//  the filter context to NULL (there is no BdaDeInitFilter function).
//
// Return Value:
//  STATUS_SUCCESS          Closed the digital tuner filter with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the DgtlTunerFilter filter pointer is zero
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerFilterClose called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the DgtlTunerFilter object out of the Context of KSFilter
    CDgtlTunerFilter* pDgtlTunerFilter =
                        static_cast <CDgtlTunerFilter*>(pKSFilter->Context);
    if( !pDgtlTunerFilter )
    {
        //no DgtlTunerFilter object available
        _DbgPrintF(
         DEBUGLVL_ERROR,
         ("Error: Close digital tuner filter failed, no DgtlTunerFilter found"));
        return STATUS_UNSUCCESSFUL;
    }
    NTSTATUS Status = pDgtlTunerFilter->Close(pKSFilter);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Close digital tuner filter failed"));
        //de-allocate memory for our DgtlTunerFilter class
        delete pDgtlTunerFilter;
        //remove the DgtlTunerFilter object from the context structure
        //of KSFilter
        pKSFilter->Context = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //de-allocate memory for our DgtlTunerFilter class
    delete pDgtlTunerFilter;
    //remove the DgtlTunerFilter object from the context structure of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}

