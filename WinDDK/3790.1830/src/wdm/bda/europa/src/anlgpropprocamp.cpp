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


#include "AnlgPropProcamp.h"
#include "defaults.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor of the procamp property class.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropProcamp::CAnlgPropProcamp()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor of the procamp property class.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropProcamp::~CAnlgPropProcamp()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the brightness property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::BrightnessGetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter
                                            //object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable were
                                            //the requested data has to be
                                            //returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    short wCurrentValue = 0;
    short wMinValue = 0;
    short wMaxValue = 0;
    short wDefValue = 0;
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->GetColor(BRIGHTNESS,
                                         &wCurrentValue,
                                         &wMinValue,
                                         &wMaxValue,
                                         &wDefValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pData->Value = wCurrentValue;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to set the brightness property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::BrightnessSetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable
                                            //containing the new data to
                                            //be set.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //check current process handle
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    CVampDevice *pVampDevice = GetVampDevice(pKsFilter);
    PEPROCESS pCurrentProcessHandle = pVampDevice->GetOwnerProcessHandle();
    if( (pCurrentProcessHandle != NULL) && (PsGetCurrentProcess() != pCurrentProcessHandle) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Set Handler was called from unregistered process"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //check if value is in valid range
    if( (pData->Value < MIN_VAMP_BRIGHTNESS_UNITS) ||
        (pData->Value > MAX_VAMP_BRIGHTNESS_UNITS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Value out of range"));
        return STATUS_UNSUCCESSFUL;
    }
    short wColorValue = 0;
    wColorValue = static_cast<short>(pData->Value);
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->SetColor(BRIGHTNESS,
                                         wColorValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the contrast property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::ContrastGetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable were
                                            //the requested data has to be
                                            //returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    short wCurrentValue = 0;
    short wMinValue = 0;
    short wMaxValue = 0;
    short wDefValue = 0;
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->GetColor(CONTRAST,
                                         &wCurrentValue,
                                         &wMinValue,
                                         &wMaxValue,
                                         &wDefValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pData->Value = wCurrentValue;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to set the contrast property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::ContrastSetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable
                                            //containing the new data to
                                            //be set.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //check if value is in valid range
    if( (pData->Value < MIN_VAMP_CONTRAST_UNITS) ||
        (pData->Value > MAX_VAMP_CONTRAST_UNITS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Value out of range"));
        return STATUS_UNSUCCESSFUL;
    }

    short wColorValue = 0;
    wColorValue = static_cast<short>(pData->Value);
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->SetColor(CONTRAST,
                                         wColorValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the hue property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::HueGetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable were
                                            //the requested data has to be
                                            //returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    short wCurrentValue = 0;
    short wMinValue = 0;
    short wMaxValue = 0;
    short wDefValue = 0;
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->GetColor(HUE,
                                         &wCurrentValue,
                                         &wMinValue,
                                         &wMaxValue,
                                         &wDefValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pData->Value = wCurrentValue;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to set the hue property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::HueSetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable
                                            //containing the new data to
                                            //be set.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //check if value is in valid range
    if( (pData->Value < MIN_VAMP_HUE_UNITS) ||
        (pData->Value > MAX_VAMP_HUE_UNITS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Value out of range"));
        return STATUS_UNSUCCESSFUL;
    }

    short wColorValue = 0;
    wColorValue = static_cast<short>(pData->Value);
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->SetColor(HUE,
                                         wColorValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the saturation property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::SaturationGetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure
                                            //that describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable were
                                            //the requested data has to be
                                            //returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    short wCurrentValue = 0;
    short wMinValue = 0;
    short wMaxValue = 0;
    short wDefValue = 0;
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->GetColor(SATURATION,
                                         &wCurrentValue,
                                         &wMinValue,
                                         &wMaxValue,
                                         &wDefValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pData->Value = wCurrentValue;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to set the saturation property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::SaturationSetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable
                                            //containing the new data to
                                            //be set.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //check if value is in valid range
    if( (pData->Value < MIN_VAMP_SATURATION_UNITS) ||
        (pData->Value > MAX_VAMP_SATURATION_UNITS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Value out of range"));
        return STATUS_UNSUCCESSFUL;
    }

    short wColorValue = 0;
    wColorValue = static_cast<short>(pData->Value);
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->SetColor(SATURATION,
                                         wColorValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the sharpness property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::SharpnessGetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable were
                                            //the requested data has to be
                                            //returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    short wCurrentValue = 0;
    short wMinValue = 0;
    short wMaxValue = 0;
    short wDefValue = 0;
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->GetColor(SHARPNESS,
                                         &wCurrentValue,
                                         &wMinValue,
                                         &wMaxValue,
                                         &wDefValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;
    pData->Value = wCurrentValue;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to set the sharpness property.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropProcamp::SharpnessSetHandler
(
    IN PIRP pIrp,                           //Pointer to the Irp that
                                            //contains the filter object.
    IN PKSPROPERTY pRequest,                //Pointer to a structure that
                                            //describes the property
                                            //request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData //Pointer to a variable
                                            //containing the new data to
                                            //be set.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //check if value is in valid range
    if( (pData->Value < MIN_VAMP_SHARPNESS_UNITS) ||
        (pData->Value > MAX_VAMP_SHARPNESS_UNITS) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Value out of range"));
        return STATUS_UNSUCCESSFUL;
    }

    short wColorValue = 0;
    wColorValue = static_cast<short>(pData->Value);
    //call HAL to get current color settings
    pVampDevResObj->m_pDecoder->SetColor(SHARPNESS,
                                         wColorValue); // void function
    pData->Capabilities = KSPROPERTY_VIDEOPROCAMP_FLAGS_MANUAL;

    return STATUS_SUCCESS;
}

