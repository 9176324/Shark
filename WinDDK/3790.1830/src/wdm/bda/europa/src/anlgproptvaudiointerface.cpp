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
#include "AnlgPropTVAudioInterface.h"
#include "AnlgTVAudioFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropTVAudio object.
//
// Return Value:
//  NOT NULL                Pointer to the CAnlgPropTVAudio object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTVAudio* GetAnlgPropTVAudio
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

    CAnlgTVAudioFilter* pAnlgTVAudioFilter =
                        static_cast <CAnlgTVAudioFilter*>(pKsFilter->Context);
    if( !pAnlgTVAudioFilter )
    {
        //no AnlgTVAudioFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgTVAudioFilter object"));
        return NULL;
    }

    CAnlgPropTVAudio* pAnlgPropTVAudio = pAnlgTVAudioFilter->GetPropTVAudio();
    if( !pAnlgPropTVAudio )
    {
        //no AnlgPropTVAudio object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTVAudio object"));
        return NULL;
    }

    return pAnlgPropTVAudio;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for TV audio capabilities (KSPROPERTY_TVAUDIO_CAPS
//  property). This function calls the class function AnlgTVAudioCapsGetHandler
//  of the CAnlgPropTVAudio class. The CAnlgPropTVAudio class pointer is stored
//  to the filter class CAnlgPropTVAudioFilter which is stored to the filter
//  context.
//
// Return Value:
//  STATUS_SUCCESS          The capabilities of the TV audio filter returned
//                          with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropTVAudio object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioCapsGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_CAPS_S pRequest, // Pointer to
                                            // PKSPROPERTY_TVAUDIO_CAPS_S
    IN OUT PKSPROPERTY_TVAUDIO_CAPS_S pData // Pointer to
                                            // PKSPROPERTY_TVAUDIO_CAPS_S
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTVAudioCapsGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTVAudio* pAnlgPropTVAudio = GetAnlgPropTVAudio(pIrp);

    if( !pAnlgPropTVAudio )
    {
        //no AnlgPropTVAudio object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTVAudio object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTVAudio->AnlgTVAudioCapsGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for TV audio available modes
//  (KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES property).
//  This function calls the class function AnlgTVAudioAvailableModesGetHandler
//  of the CAnlgPropTVAudio class. The CAnlgPropTVAudio class pointer is
//  stored to the filter class CAnlgPropTVAudioFilter which is stored to the
//  filter context.
//
// Return Value:
//  STATUS_SUCCESS          the available modes of the TV audio filter
//                          returned with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropTVAudio object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioAvailableModesGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));

    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: AnlgTVAudioAvailableModesGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTVAudio* pAnlgPropTVAudio = GetAnlgPropTVAudio(pIrp);

    if( !pAnlgPropTVAudio )
    {
        //no AnlgPropTVAudio object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTVAudio object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTVAudio->AnlgTVAudioAvailableModesGetHandler(pIrp,
                                                                 pRequest,
                                                                 pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for TV audio mode (KSPROPERTY_TVAUDIO_MODE property).
//  This function calls the class function AnlgTVAudioModeGetHandler
//  of the CAnlgPropTVAudio class. The CAnlgPropTVAudio class pointer is
//  stored to the filter class CAnlgPropTVAudioFilter which is stored to the
//  filter context.
//
// Return Value:
//  STATUS_SUCCESS          The mode of the TV audio filter
//                          returned with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropTVAudio object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioModeGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTVAudioModeGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTVAudio* pAnlgPropTVAudio = GetAnlgPropTVAudio(pIrp);

    if( !pAnlgPropTVAudio )
    {
        //no AnlgPropTVAudio object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTVAudio object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTVAudio->AnlgTVAudioModeGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler function for TV audio mode (KSPROPERTY_TVAUDIO_MODE property).
//  This function calls the class function AnlgTVAudioModeSetHandler
//  of the CAnlgPropTVAudio class. The CAnlgPropTVAudio class pointer is
//  stored to the filter class CAnlgPropTVAudioFilter which is stored to the
//  filter context.
//
// Return Value:
//  STATUS_SUCCESS          Set the mode of the TV audio filter with success
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropTVAudio object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgTVAudioModeSetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP
    IN PKSPROPERTY_TVAUDIO_S pRequest,      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
    IN OUT PKSPROPERTY_TVAUDIO_S pData      // Pointer to
                                            // PKSPROPERTY_TVAUDIO_S
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgTVAudioModeSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropTVAudio* pAnlgPropTVAudio = GetAnlgPropTVAudio(pIrp);

    if( !pAnlgPropTVAudio )
    {
        //no AnlgPropTVAudio object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropTVAudio object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropTVAudio->AnlgTVAudioModeSetHandler(pIrp, pRequest, pData);
}
