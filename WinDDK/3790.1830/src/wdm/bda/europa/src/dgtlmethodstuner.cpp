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


#include "DgtlMethodsTuner.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlMethodsTuner::CDgtlMethodsTuner()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlMethodsTuner::~CDgtlMethodsTuner()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlMethodsTuner::DgtlTunerFilterStartChanges
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
                    ("Error: DgtlTunerFilterStartChanges failed"));
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlMethodsTuner::DgtlTunerFilterCheckChanges
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
                    ("Error: DgtlTunerFilterCheckChanges failed"));
        // return ntStatus;
    }

    ntStatus = BdaCheckChanges(pIrp);
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
NTSTATUS CDgtlMethodsTuner::DgtlTunerFilterCommitChanges
(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
)
{
    NTSTATUS ntStatus = DgtlTunerFilterCheckChanges(pIrp,
                                                    pKSMethod,
                                                    pulChangeState);

    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTunerFilterCommitChanges failed"));
        // return ntStatus;
    }

    ntStatus = BdaCheckChanges(pIrp);
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTunerFilterCommitChanges failed"));
    }
    return ntStatus;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlMethodsTuner::DgtlTunerFilterGetChangeState
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
                    ("Error: DgtlTunerFilterCommitChanges failed"));
        return ntStatus;
    }
    *pulChangeState = static_cast <int> (topologyChangeState);

    return STATUS_SUCCESS;
}

