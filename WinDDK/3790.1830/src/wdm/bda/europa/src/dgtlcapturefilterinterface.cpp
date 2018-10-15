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
#include "DgtlCaptureFilterInterface.h"
#include "DgtlCaptureFilter.h"
#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Dispatch function that is called when the filter is created.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Filter has been successfully created.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlCaptureFilterCreate
(
    IN PKSFILTER  pKSFilter,//Pointer to the KS filter
                            //object
    IN PIRP       pIrp      //Pointer to the Irp involved
                            //with this operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Create Digital Capture Filter called"));
    if(!pKSFilter || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlCaptureFilter* pDgtlCaptureFilter =
                            new (NON_PAGED_POOL) CDgtlCaptureFilter();
    if(!pDgtlCaptureFilter)
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: DgtlCaptureFilter creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //store the DgtlCaptureFilter object for later use
    //in the Context of KSDevice
    pKSFilter->Context = pDgtlCaptureFilter;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Dispatch function that is called when the filter is to be closed.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Filter has been successfully closed.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlCaptureFilterClose
(
    IN PKSFILTER  pKSFilter,//Pointer to the KS filter
                            //object
    IN PIRP       pIrp      //Pointer to the Irp involved
                            //with this operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: Close Digital Capture Filter called"));
    if(!pKSFilter || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the DgtlCaptureFilter object out of
    //the Context of KSFilter
    CDgtlCaptureFilter* pDgtlCaptureFilter =
                    static_cast <CDgtlCaptureFilter*>(pKSFilter->Context);
    if(!pDgtlCaptureFilter)
    {
        //no digital filter object available
        _DbgPrintF(DEBUGLVL_ERROR,
        ("Error: Close digital capture filter failed, no DgtlCaptureFilter found"));
        return STATUS_UNSUCCESSFUL;
    }
    //de-allocate memory for our DgtlCaptureFilter class
    delete pDgtlCaptureFilter;
    //remove the DgtlCaptureFilter object from the
    //context structure of KSFilter
    pKSFilter->Context = NULL;

    return STATUS_SUCCESS;
}
