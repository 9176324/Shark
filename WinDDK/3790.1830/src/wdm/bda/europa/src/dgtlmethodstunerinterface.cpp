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
#include "DgtlMethodsTunerInterface.h"
#include "DgtlTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CDgtlMethodsTuner object.
//
// Return Value:
//  NOT NULL                Pointer to the CDgtlMethodsTuner object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CDgtlMethodsTuner* GetDgtlMethodsTuner
(
    IN PIRP pIrp                                // Pointer to IRP that
                                                // contains the filter object.
)
{
    if( !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return NULL;
    }

    //get device resource object
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    if( !pKsFilter )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to KS filter"));
        return NULL;
    }

    CDgtlTunerFilter* pDgtlTunerFilter =
                        static_cast <CDgtlTunerFilter*>(pKsFilter->Context);
    if( !pDgtlTunerFilter )
    {
        //no DgtlTunerFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlTunerFilter object"));
        return NULL;
    }

    CDgtlMethodsTuner* pDgtlMethodsTuner =
                                pDgtlTunerFilter->GetMethodsTuner();
    if( !pDgtlMethodsTuner )
    {
        //no pDgtlMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlMethodsTuner object"));
        return NULL;
    }

    return pDgtlMethodsTuner;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterStartChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerFilterStartChanges called"));
    if( !pIrp || !pKSMethod )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlMethodsTuner* pDgtlMethodsTuner = GetDgtlMethodsTuner(pIrp);

    if( !pDgtlMethodsTuner )
    {
        //no DgtlMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlMethodsTuner->DgtlTunerFilterStartChanges(  pIrp,
                                                            pKSMethod,
                                                            pulChangeState);
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterCheckChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerFilterCheckChanges called"));
    if( !pIrp || !pKSMethod )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlMethodsTuner* pDgtlMethodsTuner = GetDgtlMethodsTuner(pIrp);

    if( !pDgtlMethodsTuner )
    {
        //no DgtlMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlMethodsTuner->DgtlTunerFilterCheckChanges(  pIrp,
                                                            pKSMethod,
                                                            pulChangeState);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterCommitChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerFilterCommitChanges called"));
    if( !pIrp || !pKSMethod )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlMethodsTuner* pDgtlMethodsTuner = GetDgtlMethodsTuner(pIrp);

    if( !pDgtlMethodsTuner )
    {
        //no DgtlMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlMethodsTuner->DgtlTunerFilterCommitChanges( pIrp,
                                                            pKSMethod,
                                                            pulChangeState);

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerFilterGetChangeState
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: DgtlTunerFilterGetChangeState called"));
    if( !pIrp || !pKSMethod )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlMethodsTuner* pDgtlMethodsTuner = GetDgtlMethodsTuner(pIrp);

    if( !pDgtlMethodsTuner )
    {
        //no DgtlMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlMethodsTuner->DgtlTunerFilterGetChangeState(pIrp,
                                                            pKSMethod,
                                                            pulChangeState);

}

