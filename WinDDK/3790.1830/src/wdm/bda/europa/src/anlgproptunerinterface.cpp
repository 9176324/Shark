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
#include "AnlgPropTunerInterface.h"
#include "AnlgTunerFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropTuner object.
//
// Return Value:
//  NOT NULL                Pointer to the CAnlgPropTuner object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTuner* GetAnlgPropTuner
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

    CAnlgPropTuner* pAnlgPropTuner = pAnlgTunerFilter->GetPropTuner();
    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return NULL;
    }

    return pAnlgPropTuner;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerCapsGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_CAPS_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerCapsGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerCapsGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerModeCapsGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_MODE_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_CAPS_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerModeCapsGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerModeCapsGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner mode
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerModeGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerModeGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerModeGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerModeSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerModeSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerModeSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner standard
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerStandardGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerModeGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerStandardGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner standard
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerStandardSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerModeSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerStandardSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner frequency
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerFrequencyGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFrequencyGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerFrequencyGetHandler(pIrp,
                                                        pRequest,
                                                        pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner Frequency
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerFrequencySetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerFrequencySetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerFrequencySetHandler(pIrp,
                                                        pRequest,
                                                        pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner input
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerInputGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerInputGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerInputGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner input
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerInputSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerInputSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerInputSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner status
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTunerStatusGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_STATUS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STATUS_S pData
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTunerStatusGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTuner* pAnlgPropTuner = GetAnlgPropTuner(pIrp);

    if( !pAnlgPropTuner )
    {
        //no AnlgPropTuner object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTuner object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTuner->AnlgTunerStatusGetHandler(pIrp, pRequest, pData);
}
