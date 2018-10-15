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
#include "DgtlPropTunerInterface.h"
#include "DgtlTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CDgtlPropTuner object.
//
// Return Value:
//  NOT NULL                Pointer to the CDgtlPropTuner object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CDgtlPropTuner* GetDgtlPropTuner
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
        //no pDgtlTunerFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlTunerFilter object"));
        return NULL;
    }

    CDgtlPropTuner* pDgtlPropTuner = pDgtlTunerFilter->GetPropTuner();
    if( !pDgtlPropTuner )
    {
        //no pDgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return NULL;
    }

    return pDgtlPropTuner;
}

//COFDM demodulator property functions

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  start handler for demodulation
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS COFDMDemodulatorNodeStartHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pKSRequest,
    IN OUT PVOID pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: COFDMDemodulatorNodeStartHandler called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->COFDMDemodulatorNodeStartHandler(pIrp,
                                                            pKSRequest,
                                                            pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  stop handler for demodulation
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS COFDMDemodulatorNodeStopHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pKSRequest,
    IN OUT PVOID pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: COFDMDemodulatorNodeStopHandler called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->COFDMDemodulatorNodeStopHandler(pIrp,
                                                            pKSRequest,
                                                            pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for frequency multiplier property
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeGetFrequencyMultiplier
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: RFTunerNodeGetFrequencyMultiplier called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetFrequencyMultiplier(pIrp,
                                                            pKSRequest,
                                                            pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for frequency multiplier property
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeSetFrequencyMultiplier
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: RFTunerNodeSetFrequencyMultiplier called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeSetFrequencyMultiplier(pIrp,
                                                            pKSRequest,
                                                            pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for frequency property
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeGetFrequency
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetFrequency called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetFrequency(pIrp,
                                                   pKSRequest,
                                                   pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for frequency property
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeSetFrequency
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    LARGE_INTEGER StartTime;
    KeQuerySystemTime( &StartTime );

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeSetFrequency called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeSetFrequency(pIrp,
                                                   pKSRequest,
                                                   pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler to see if signal is present
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeGetSignalPresent
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PBOOL pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetSignalPresent called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetSignalPresent(pIrp,
                                                       pKSRequest,
                                                       pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler to see if signal is locked
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS RFTunerNodeGetSignalLocked
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PBOOL pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetSignalLocked called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetSignalLocked(pIrp,
                                                      pKSRequest,
                                                      pData);
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
NTSTATUS RFTunerNodeGetSampleTime
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetSampleTime called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetSampleTime(pIrp,
                                                    pKSRequest,
                                                    pData);
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
NTSTATUS RFTunerNodeGetSignalQuality
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetSignalQuality called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetSignalQuality(pIrp,
                                                       pKSRequest,
                                                       pData);
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
NTSTATUS RFTunerNodeGetSignalStrength
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: RFTunerNodeGetSignalStrength called"));
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CDgtlPropTuner* pDgtlPropTuner = GetDgtlPropTuner(pIrp);

    if( !pDgtlPropTuner )
    {
        //no DgtlPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get DgtlPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlPropTuner->RFTunerNodeGetSignalStrength(pIrp,
                                                        pKSRequest,
                                                        pData);
}
