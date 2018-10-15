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
#include "AnlgXBarFilterInterface.h"
#include "AnlgXBarFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the crossbar filter. This function
//  creates an instance of the analog crossbar filter (CAnlgXBarFilter)
//  and stores the pointer to the filter context. It also initializes the
//  crossbar filter with the class function InitXBarFilter() of the
//  CAnlgXBarFilter class.
//
// Return Value:
//  STATUS_SUCCESS                  Crossbar filter has been created
//                                  successfully
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) if the function parameters are zero,
//                                  (b) if the propetry classes could not
//                                  be created
//  STATUS_INSUFFICIENT_RESOURCES   NEW operation failed. Crossbar filter
//                                  could't be created.

//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgXBarFilterCreate
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Create Analog Crossbar Filter called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgXBarFilter* pAnlgXBarFilter = new (NON_PAGED_POOL) CAnlgXBarFilter();
    if( !pAnlgXBarFilter )
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
              ("Error: AnlgXBarFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pAnlgXBarFilter->Create(pKSFilter) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgXBarFilter creation failed"));
        delete pAnlgXBarFilter;
        pAnlgXBarFilter = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the AnlgXBarFilter object for later use in the
    //Context of KSFilter
    pKSFilter->Context = pAnlgXBarFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the close dispatch function of the crossbar filter. This function
//  deinitializes the instance of the analog crossbar filter
//  (CAnlgXBarFilter), which was stored on the filter context. It also sets
//  the filter context to NULL (there is no BdaDeInitFilter function).
//
// Return Value:
//  STATUS_SUCCESS          Closed the Crossbar Filter with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgXBarFilter filter pointer is zero
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgXBarFilterClose
(
    IN PKSFILTER pKSFilter, // Pointer to KSFILTER
    IN PIRP pIrp            // Pointer to IRP
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Close Analog Crossbar Filter called"));
    if( !pKSFilter || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the AnlgXBarFilter object out of the Context of KSFilter
    CAnlgXBarFilter* pAnlgXBarFilter =
                        static_cast <CAnlgXBarFilter*>(pKSFilter->Context);
    if( !pAnlgXBarFilter )
    {
        //no AnlgXBarFilter object available
        _DbgPrintF(
          DEBUGLVL_ERROR,
          ("Error: Close analog crossbar filter failed, no AnlgXBarFilter found"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS Status = pAnlgXBarFilter->Close(pKSFilter);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Close analog crossbar filter failed"));
        //de-allocate memory for our AnlgXBarFilter class
        delete pAnlgXBarFilter;
        //remove the AnlgXBarFilter object from the context structure
        //of KSFilter
        pKSFilter->Context = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //de-allocate memory for our AnlgXBarFilter class
    delete pAnlgXBarFilter;
    //remove the AnlgXBarFilter object from the context structure
    //of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}
