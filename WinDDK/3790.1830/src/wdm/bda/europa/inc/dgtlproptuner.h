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

#include "BaseClass.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains the implementation of the Digital Tuner Filter
//  properties.
//////////////////////////////////////////////////////////////////////////////
class CDgtlPropTuner : public CBaseClass
{
public:
    CDgtlPropTuner();
    ~CDgtlPropTuner();

    //COFDM properties

    NTSTATUS COFDMDemodulatorNodeStartHandler
    (
        IN PIRP pIrp,
        IN PKSPROPERTY pKSRequest,
        IN OUT PVOID pData
    );

    NTSTATUS COFDMDemodulatorNodeStopHandler
    (
        IN PIRP pIrp,
        IN PKSPROPERTY pKSRequest,
        IN OUT PVOID pData
    );

    //RF Tuner node properties

    NTSTATUS RFTunerNodeGetFrequencyMultiplier
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PDWORD pData
    );

    NTSTATUS RFTunerNodeSetFrequencyMultiplier
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PDWORD pData
    );

    NTSTATUS RFTunerNodeGetFrequency
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PDWORD pData
    );

    NTSTATUS RFTunerNodeSetFrequency
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PDWORD pData
    );

    NTSTATUS RFTunerNodeGetSignalStrength
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PLONG pData
    );

    NTSTATUS RFTunerNodeGetSignalQuality
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PLONG pData
    );

    NTSTATUS RFTunerNodeGetSignalPresent
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PBOOL pData
    );

    NTSTATUS RFTunerNodeGetSignalLocked
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PBOOL pData
    );

    NTSTATUS RFTunerNodeGetSampleTime
    (
        IN PIRP pIrp,
        IN PKSP_NODE pKSRequest,
        IN OUT PLONG pData
    );

    NTSTATUS HW_InitializeTunerHardware
    (
        IN CVampDeviceResources* pDevResObj
    );

    NTSTATUS HW_TunerNodeSetFrequency
    (
        IN CVampDeviceResources* pDevResObj
    );

private:

    //RF tuner property functions

    DWORD m_dwFrequencyMultiplier;
    DWORD m_dwFrequency;

};
