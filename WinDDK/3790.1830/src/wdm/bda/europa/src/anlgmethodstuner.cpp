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


#include "AnlgMethodsTuner.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgMethodsTuner::CAnlgMethodsTuner()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgMethodsTuner::~CAnlgMethodsTuner()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgMethodsTuner::AnlgTunerFilterStartChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    NTSTATUS ntStatus = BdaStartChanges(pIrp);

    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilterStartChanges failed"));
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgMethodsTuner::AnlgTunerFilterCheckChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    BDA_CHANGE_STATE    topologyChangeState;
    NTSTATUS ntStatus = BdaGetChangeState( pIrp, &topologyChangeState);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilterCheckChanges failed"));
        // return ntStatus;
    }

    ntStatus = BdaCheckChanges( pIrp);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTunerFilterCheckChanges failed"));
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgMethodsTuner::AnlgTunerFilterCommitChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    NTSTATUS ntStatus = AnlgTunerFilterCheckChanges(pIrp,
                                                    pKSMethod,
                                                    pulChangeState);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilterCommitChanges failed"));
        // return ntStatus;
    }

    ntStatus = BdaCommitChanges( pIrp);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilterCommitChanges failed"));
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgMethodsTuner::AnlgTunerFilterGetChangeState
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    BDA_CHANGE_STATE    topologyChangeState;
    NTSTATUS ntStatus = BdaGetChangeState(pIrp, &topologyChangeState);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: AnlgTunerFilterGetChangeState failed"));
        return ntStatus;
    }
    *pulChangeState = topologyChangeState;

    return STATUS_SUCCESS;
}

