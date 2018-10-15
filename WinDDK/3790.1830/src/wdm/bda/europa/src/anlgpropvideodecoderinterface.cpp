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
#include "AnlgPropVideoDecoderInterface.h"
#include "AnlgCaptureFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropVideoDecoder object.
//
// Return Value:
//  NULL                    Operation failed,
//  >NULL                   Pointer to the CAnlgPropVideoDecoder object
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropVideoDecoder* GetAnlgPropVideoDecoder
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

    CAnlgPropVideoDecoder* pAnlgPropVideoDecoder =
                                    pAnlgCaptureFilter->GetPropVideoDecoder();
    if( !pAnlgPropVideoDecoder )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropVideoDecoder object"));
        return NULL;
    }

    return pAnlgPropVideoDecoder;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler for the decoder capabilities.
//  The requested information is returned via pData.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DecoderCapsGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_CAPS_S pData //Pointer to a variable were
                                                 //the requestet data has to
                                                 //be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DecoderCapsGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropVideoDecoder* pAnlgPropVideoDecoder =
                                                GetAnlgPropVideoDecoder(pIrp);

    if( !pAnlgPropVideoDecoder )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropVideoDecoder object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropVideoDecoder->DecoderCapsGetHandler(pIrp,
                                                        pRequest,
                                                        pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The get handler for the video standard reads the default standard from
//  the registry to update class internal members (m_dwDefaultNumberOfLines)
//  Then it calls 'StatusGetHandler' to get the current decoder standard
//  from the hardware.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS StandardGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_S pData      //Pointer to a variable were the
                                                 //requestet data has to be
                                                 //returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: StandardGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropVideoDecoder* pAnlgPropVideoDecoder =
                                                GetAnlgPropVideoDecoder(pIrp);

    if( !pAnlgPropVideoDecoder )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropVideoDecoder object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropVideoDecoder->StandardGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The set handler for the video standard updates the class internal member
//  and finally sets the standard at the hardware using 'StatusSetHandler'.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS StandardSetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_S pData      //Pointer to a variable were
                                                 //the requestet data has to
                                                 //be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: StandardSetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropVideoDecoder* pAnlgPropVideoDecoder =
                                                GetAnlgPropVideoDecoder(pIrp);

    if( !pAnlgPropVideoDecoder )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropVideoDecoder object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropVideoDecoder->StandardSetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The status get handler reads the current status from the hardware
//  and returns the information by pData.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS StatusGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_STATUS_S pData //Pointer to a variable were
                                                   //the requestet data has to
                                                   //be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: StatusGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropVideoDecoder* pAnlgPropVideoDecoder =
                                                GetAnlgPropVideoDecoder(pIrp);

    if( !pAnlgPropVideoDecoder )
    {
        //no AnlgPropVideoDecoder object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropVideoDecoder object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropVideoDecoder->StatusGetHandler(pIrp, pRequest, pData);
}
