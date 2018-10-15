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
#include "AnlgPropProcampInterface.h"
#include "AnlgCaptureFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropProcamp object.
//
// Return Value:
//  NULL                    Operation failed,
//  >NULL                   Pointer to the CAnlgPropProcamp object
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropProcamp* GetAnlgPropProcamp
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

    CAnlgCaptureFilter* pAnlgCaptureFilter =
                    static_cast<CAnlgCaptureFilter*>(pKsFilter->Context);
    if( !pAnlgCaptureFilter )
    {
        //no AnlgCaptureFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgCaptureFilter object"));
        return NULL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = pAnlgCaptureFilter->GetPropProcamp();
    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return NULL;
    }

    return pAnlgPropProcamp;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for brightness control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS BrightnessGetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable were the
                                           //requested data has to be
                                           //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: BrightnessGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->BrightnessGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler for brightness control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS BrightnessSetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable containing
                                           //the new data to be set.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: BrightnessSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->BrightnessSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for contrast control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS ContrastGetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable were the
                                           //requested data has to be
                                           //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: ContrastGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->ContrastGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler for contrast control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS ContrastSetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable containing
                                           //the new data to be set.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: ContrastSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->ContrastSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for hue control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS HueGetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable were the
                                           //requested data has to be
                                           //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: HueGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->HueGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler for hue control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS HueSetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable containing
                                           //the new data to be set.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: HueSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->HueSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for saturation control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS SaturationGetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable were the
                                           //requested data has to be
                                           //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SaturationGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->SaturationGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler for saturation control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS SaturationSetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable containing
                                           //the new data to be set.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SaturationSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->SaturationSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for sharpness control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS SharpnessGetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable were the
                                           //requested data has to be
                                           //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SharpnessGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->SharpnessGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler for sharpness control.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS SharpnessSetHandler
(
    IN PIRP pIrp,                          //Pointer to the Irp that
                                           //contains the filter object.
    IN PKSIDENTIFIER pRequest,             //Pointer to a structure that
                                           //describes the property request.
    IN OUT PKSPROPERTY_VIDEOPROCAMP_S pData//Pointer to a variable containing
                                           //the new data to be set.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: SharpnessSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropProcamp* pAnlgPropProcamp = GetAnlgPropProcamp(pIrp);

    if( !pAnlgPropProcamp )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropProcamp object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropProcamp->SharpnessSetHandler(pIrp, pRequest, pData);
}

