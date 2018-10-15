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
#include "AnlgMethodsTunerInterface.h"
#include "AnlgTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgMethodsTuner object.
//
// Return Value:
//  NOT NULL                Pointer to the CAnlgMethodsTuner object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CAnlgMethodsTuner* GetAnlgMethodsTuner
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

    CAnlgTunerFilter* pAnlgTunerFilter =
                        static_cast <CAnlgTunerFilter*>(pKsFilter->Context);
    if( !pAnlgTunerFilter )
    {
        //no AnlgXBarFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgTunerFilter object"));
        return NULL;
    }

    CAnlgMethodsTuner* pAnlgMethodsTuner =
                                    pAnlgTunerFilter->GetMethodsTuner();
    if( !pAnlgMethodsTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgMethodsTuner object"));
        return NULL;
    }

    return pAnlgMethodsTuner;
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
NTSTATUS AnlgTunerFilterStartChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFilterStartChanges called"));
    if( !pIrp || !pKSMethod || !pulChangeState )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgMethodsTuner* pAnlgMethodsTuner = GetAnlgMethodsTuner(pIrp);

    if( !pAnlgMethodsTuner )
    {
        //no AnlgMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgMethodsTuner->AnlgTunerFilterStartChanges(  pIrp,
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
NTSTATUS AnlgTunerFilterCheckChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFilterCheckChanges called"));
    if( !pIrp || !pKSMethod || !pulChangeState )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgMethodsTuner* pAnlgMethodsTuner = GetAnlgMethodsTuner(pIrp);

    if( !pAnlgMethodsTuner )
    {
        //no AnlgMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgMethodsTuner->AnlgTunerFilterCheckChanges(  pIrp,
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
NTSTATUS AnlgTunerFilterCommitChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFilterCommitChanges called"));
    if( !pIrp || !pKSMethod || !pulChangeState )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgMethodsTuner* pAnlgMethodsTuner = GetAnlgMethodsTuner(pIrp);

    if( !pAnlgMethodsTuner )
    {
        //no AnlgMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgMethodsTuner->AnlgTunerFilterCommitChanges( pIrp,
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
NTSTATUS AnlgTunerFilterGetChangeState
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: AnlgTunerFilterGetChangeState called"));
    if( !pIrp || !pKSMethod || !pulChangeState )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgMethodsTuner* pAnlgMethodsTuner = GetAnlgMethodsTuner(pIrp);

    if( !pAnlgMethodsTuner )
    {
        //no AnlgMethodsTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgMethodsTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgMethodsTuner->AnlgTunerFilterGetChangeState(pIrp,
                                                            pKSMethod,
                                                            pulChangeState);
}

