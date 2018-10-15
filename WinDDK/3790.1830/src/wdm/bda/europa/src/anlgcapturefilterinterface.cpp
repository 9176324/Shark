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


#include "34AVStrm.h"
#include "AnlgCaptureFilterInterface.h"
#include "AnlgCaptureFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Dispatch function that is called when the filter is created.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Filter has been created successful.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgCaptureFilterCreate
(
    IN PKSFILTER  pKSFilter,//Pointer to the KS filter
                            //object
    IN PIRP       pIrp      //Pointer to the Irp involved
                            //with this operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Create Analog Capture Filter called"));
    if(!pKSFilter || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgCaptureFilter* pAnlgCaptureFilter =
                new (NON_PAGED_POOL) CAnlgCaptureFilter();
    if(!pAnlgCaptureFilter)
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: AnlgCaptureFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pAnlgCaptureFilter->Create(pKSFilter) != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: AnlgCaptureFilter creation failed"));
        delete pAnlgCaptureFilter;
        pAnlgCaptureFilter = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the AnlgCaptureFilter object for later use in the
    //Context of KSFilter
    pKSFilter->Context = pAnlgCaptureFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Dispatch function that is called when the filter is to be closed.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Filter has been closed successful.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgCaptureFilterClose
(
    IN PKSFILTER  pKSFilter,//Pointer to the KS filter
                            //object
    IN PIRP       pIrp      //Pointer to the Irp involved
                            //with this operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Close Analog Capture Filter called"));
    if(!pKSFilter || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the AnlgCaptureFilter object out of
    //the Context of KSFilter
    CAnlgCaptureFilter* pAnlgCaptureFilter =
                    static_cast <CAnlgCaptureFilter*> (pKSFilter->Context);
    if(!pAnlgCaptureFilter)
    {
        //no analog capture filter object available
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Close analog capture filter failed, no AnlgCaptureFilter found"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS Status = pAnlgCaptureFilter->Close(pKSFilter);
    if( !NT_SUCCESS(Status) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close analog capture filter failed"));
        //de-allocate memory for our AnlgCaptureFilter class
        delete pAnlgCaptureFilter;
        //remove the AnlgCaptureFilter object from the context structure
        //of KSFilter
        pKSFilter->Context = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //de-allocate memory for our AnlgCaptureFilter class
    delete pAnlgCaptureFilter;
    //remove the AnlgCaptureFilter object from the context structure
    //of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}

