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

#pragma once

NTSTATUS AnlgTunerFilterStartChanges
(
	IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
);

NTSTATUS AnlgTunerFilterCheckChanges
(
	IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
);

NTSTATUS AnlgTunerFilterCommitChanges
(
	IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
);

NTSTATUS AnlgTunerFilterGetChangeState
(
	IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
);

//  Defines the dispatch routines for the filter level
//  Change Sync methods
//
DEFINE_KSMETHOD_TABLE(AnlgTunerFilterBdaChangeSyncMethods)
{
    DEFINE_KSMETHOD_ITEM_BDA_START_CHANGES(
        AnlgTunerFilterStartChanges,
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_CHECK_CHANGES(
        AnlgTunerFilterCheckChanges,
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_COMMIT_CHANGES(
        AnlgTunerFilterCommitChanges,
        NULL
        ),
    DEFINE_KSMETHOD_ITEM_BDA_GET_CHANGE_STATE(
        AnlgTunerFilterGetChangeState,
        NULL
        )
};

//
//  Filter Level Method Sets supported
//
//  This table defines all method sets supported by the
//  tuner filter exposed by this driver.
//
DEFINE_KSMETHOD_SET_TABLE(AnlgTunerFilterMethodSets)
{
    DEFINE_KSMETHOD_SET
    (
        &KSMETHODSETID_BdaChangeSync,						// Set
        SIZEOF_ARRAY(AnlgTunerFilterBdaChangeSyncMethods),  // PropertiesCount
        AnlgTunerFilterBdaChangeSyncMethods,                // PropertyItems
        0,													// FastIoCount
        NULL												// FastIoTable
    )
};
