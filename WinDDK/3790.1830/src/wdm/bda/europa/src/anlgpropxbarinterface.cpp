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
#include "AnlgPropXBarInterface.h"
#include "AnlgXBarFilter.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function returns the pointer to the CAnlgPropXBar object.
//
// Return Value:
//  NOT NULL                Pointer to the CAnlgPropXBar object
//  NULL                    Operation failed,
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropXBar* GetAnlgPropXBar
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

    CAnlgXBarFilter* pAnlgXBarFilter =
                        static_cast <CAnlgXBarFilter*>(pKsFilter->Context);
    if( !pAnlgXBarFilter )
    {
        //no AnlgXBarFilter object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgXBarFilter object"));
        return NULL;
    }

    CAnlgPropXBar* pAnlgPropXBar = pAnlgXBarFilter->GetPropXBar();
    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return NULL;
    }

    return pAnlgPropXBar;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for crossbar capabilities (KSPROPERTY_CROSSBAR_CAPS
//  property). This function calls the class function XBarCapsGetHandler of
//  the CAnlgPropXBar class. The CAnlgPropXBar class pointer is stored to
//  the filter class CAnlgXBarFilter, which is stored to the filter context.
//
// Return Value:
//  STATUS_SUCCESS          Capabilities of the crossbar filter has been
//                          returned successfully
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropXBar object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS XBarCapsGetHandler
(
    IN PIRP pIrp,                           // Pointer to IRP that contains
                                            // the filter object.
    IN PKSPROPERTY_CROSSBAR_CAPS_S pRequest,// KSPROPERTY_CROSSBAR_CAPS_S
                                            // that describes the property
                                            // request.
    IN OUT PKSPROPERTY_CROSSBAR_CAPS_S pData// Pointer to
                                            // KSPROPERTY_CROSSBAR_CAPS_S
                                            // were the number of input and
                                            // output pins has to be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: XBarCapsGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropXBar* pAnlgPropXBar = GetAnlgPropXBar(pIrp);

    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropXBar->XBarCapsGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for crossbar pin information
//  (KSPROPERTY_CROSSBAR_PININFO property). This function calls the class
//  function XBarPinInfoGetHandler of the CAnlgPropXBar class. The
//  CAnlgPropXBar class pointer is stored to the filter class CAnlgXBarFilter,
//  which is stored to the filter context.
//
// Return Value:
//  STATUS_SUCCESS          Pin information of the crossbar filter has been
//                          returned successfully
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropXBar object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS XBarPinInfoGetHandler
(
    IN PIRP pIrp,                              // Pointer to IRP that contains
                                               // the filter object.
    IN PKSPROPERTY_CROSSBAR_PININFO_S pRequest,// Pointer to
                                               // KSPROPERTY_CROSSBAR_PININFO_S
                                               // that describes the property
                                               // request.
    IN OUT PKSPROPERTY_CROSSBAR_PININFO_S pData// Pointer to
                                               // KSPROPERTY_CROSSBAR_PININFO_S
                                               // were the requestet data has
                                               // to be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: XBarPinInfoGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropXBar* pAnlgPropXBar = GetAnlgPropXBar(pIrp);

    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropXBar->XBarPinInfoGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for crossbar routing capabilities
//  (KSPROPERTY_CROSSBAR_CAN_ROUTE property). This function calls the class
//  function XBarCanRouteGetHandler of the CAnlgPropXBar class. The
//  CAnlgPropXBar class pointer is stored to the filter class CAnlgXBarFilter,
//  which is stored to the filter context.
//
// Return Value:
//  STATUS_SUCCESS          Route information of the crossbar filter has been
//                          returned successfully
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropXBar object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS XBarCanRouteGetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP that contains
                                             // the filter object.
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the requested
                                             // combination of input and
                                             // output pin
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the ability of routing
                                             // has to be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: XBarCanRouteGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropXBar* pAnlgPropXBar = GetAnlgPropXBar(pIrp);

    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropXBar->XBarCanRouteGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Get handler function for crossbar routing (KSPROPERTY_CROSSBAR_ROUTE
//  property). This function calls the class function XBarRouteGetHandler of
//  the CAnlgPropXBar class. The CAnlgPropXBar class pointer is stored to the
//  filter class CAnlgXBarFilter, which is stored to the filter context.
//
// Return Value:
//  STATUS_SUCCESS          Routing the crossbar filter has been done
//                          successfully.
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropXBar object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS XBarRouteGetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP that contains
                                             // the filter object.
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the requested
                                             // output pin
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the associated input
                                             // pin has to be returned
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: XBarRouteGetHandler called"));
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropXBar* pAnlgPropXBar = GetAnlgPropXBar(pIrp);

    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropXBar->XBarRouteGetHandler(pIrp, pRequest, pData);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Set handler function for crossbar routing (KSPROPERTY_CROSSBAR_ROUTE
//  property). This function calls the class function XBarRouteSetHandler of
//  the CAnlgPropXBar class. The CAnlgPropXBar class pointer is stored to the
//  filter class CAnlgXBarFilter, which is stored to the filter context.
//
// Return Value:
//  STATUS_SUCCESS          Route of the crossbar filter has been set
//                          successfully.
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the AnlgPropXBar object is missing
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS XBarRouteSetHandler
(
    IN PIRP pIrp,                            // Pointer to IRP that contains
                                             // the filter object.
    IN PKSPROPERTY_CROSSBAR_ROUTE_S pRequest,// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // that describes the combination
                                             // of input and output pin to
                                             // route
    IN OUT PKSPROPERTY_CROSSBAR_ROUTE_S pData// Pointer to
                                             // KSPROPERTY_CROSSBAR_ROUTE_S
                                             // were the ability of routing
                                             // has to be returned.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: XBarRouteSetHandler called"));

    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    CAnlgPropXBar* pAnlgPropXBar = GetAnlgPropXBar(pIrp);

    if( !pAnlgPropXBar )
    {
        //no AnlgPropXBar object available
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get AnlgPropXBar object"));
        return STATUS_UNSUCCESSFUL;
    }

    return pAnlgPropXBar->XBarRouteSetHandler(pIrp, pRequest, pData);
}
