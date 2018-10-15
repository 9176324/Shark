/*++

Copyright (c) 1999  Microsoft Corporation

Module Name:

    outpin.cpp

Abstract:

    Transport Ouput pin code.

--*/

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif // ALLOC_DATA_PRAGMA

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA

/*
** (Static) PinCreate() method of the CTransportPin class
**
**    Creates the output pin object and
**    associates it with the filter object. 
**
*/
NTSTATUS
CTransportPin::PinCreate(
    IN OUT PKSPIN pKSPin,
    IN PIRP Irp
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CTransportPin*      pPin;
    CFilter*            pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PinCreate"));

    ASSERT(pKSPin);
    ASSERT(Irp);

    //  Obtain a pointer to the filter object for which the output pin is created.
    //
    pFilter = reinterpret_cast<CFilter*>(KsGetFilterFromIrp(Irp)->Context);

    //  Create the transport output pin object.
    //
    pPin = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CTransportPin;  // Tags the allocated memory 
    if (pPin)
    {
        //  Link the pin context to the filter context.
        //  That is, set the output pin's filter pointer data member to the obtained filter pointer.
        //
        pPin->SetFilter( pFilter);
    
        //  Link the pin context to the passed in pointer to the KSPIN structure.
        //
        pKSPin->Context = pPin;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


/*
** PinClose() method of the CTransportPin class
**
**    Deletes the previously created output pin object.
**
*/
NTSTATUS
CTransportPin::PinClose(
    IN OUT PKSPIN Pin,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PinClose"));

    ASSERT(Pin);
    ASSERT(Irp);

    //  Retrieve the transport output pin object from the passed in 
    //  KSPIN structure's context member.
    //
    CTransportPin* pPin = reinterpret_cast<CTransportPin*>(Pin->Context);

    ASSERT(pPin);

    delete pPin;

    return STATUS_SUCCESS;
}

/*
** IntersectDataFormat() method of the CTransportPin class
**
**    Enables connection of the output pin with a downstream filter.
**
*/
NTSTATUS
CTransportPin::IntersectDataFormat(
    IN PVOID pContext,
    IN PIRP pIrp,
    IN PKSP_PIN Pin,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
     )
{
    if ( DataBufferSize < sizeof(KS_DATARANGE_BDA_TRANSPORT) )
    {
        *DataSize = sizeof( KS_DATARANGE_BDA_TRANSPORT );
        return STATUS_BUFFER_OVERFLOW;
    }
    else if (DataRange -> FormatSize < sizeof (KS_DATARANGE_BDA_TRANSPORT)) 
    {
	return STATUS_NO_MATCH;
    } else
    {
        ASSERT(DataBufferSize == sizeof(KS_DATARANGE_BDA_TRANSPORT));

        *DataSize = sizeof( KS_DATARANGE_BDA_TRANSPORT );
        RtlCopyMemory( Data, (PVOID)DataRange, sizeof(KS_DATARANGE_BDA_TRANSPORT));

        return STATUS_SUCCESS;
    }
}

/*
** GetSignalStatus() method of the CTransportPin class
**
**    Retrieves the value of the demodulator node signal statistics properties.
**
*/
NTSTATUS
CTransportPin::GetSignalStatus(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    CTransportPin *             pPin;
    CFilter*                    pFilter;
    BDATUNER_DEVICE_STATUS      DeviceStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetSignalStatus"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


    //  Call the BDA support library to 
    //  validate that the node type is associated with this pin.
    //
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
        //  Obtain a pointer to the pin object.
        //
        //  Because the property dispatch table calls the CTransportPin::GetSignalStatus() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT(pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        Status = pFilter->GetStatus( &DeviceStatus);
        if (Status == STATUS_SUCCESS)
        {
            switch (pKSProperty->Id)
            {
            case KSPROPERTY_BDA_SIGNAL_LOCKED:
                *pulProperty = DeviceStatus.fSignalLocked;
                break;
        
            default:
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }

    return Status;
}


/*
** PutAutoDemodProperty() method of the CTransportPin class
**
**    Starts or Stops automatic demodulation.
**
*/
NTSTATUS
CTransportPin::PutAutoDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutAutoDemodProperty"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


    //  Call the BDA support library to 
    //  validate that the node type is associated with this pin.
    //
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
        //  Obtain a pointer to the pin object.
        //
        //  Because the property dispatch table calls the CTransportPin::PutAutoDemodProperty() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_AUTODEMODULATE_START:
            //  Start Demodulator if stopped.
            //  NOTE!  The default state of the demod should match the
            //         graph run state.  This property will only be set
            //         if KSPROPERTY_BDA_AUTODEMODULATE_STOP was previously
            //         set.
            break;
    
        case KSPROPERTY_BDA_AUTODEMODULATE_STOP:
            //  Stop Demodulator
            //  A demodulator stop/start sequence may be used in an
            //  attempt to retrain the demodulator after a channel change.
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

#if !ATSC_RECEIVER
/*
** PutDigitalDemodProperty() method of the CTransportPin class
**
**    Sets the value of the digital demodulator node properties.
**
*/
NTSTATUS
CTransportPin::PutDigitalDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutDigitalDemodProperty"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


    //  Call the BDA support library to 
    //  validate that the node type is associated with this pin.
    //
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
        //  Obtain a pointer to the pin object.
        //
        //  Because the property dispatch table calls the CTransportPin::PutDigitalDemodProperty() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_MODULATION_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_SYMBOL_RATE:
            break;
    
        case KSPROPERTY_BDA_SPECTRAL_INVERSION:
            break;
    
        case KSPROPERTY_BDA_GUARD_INTERVAL:
            break;
    
        case KSPROPERTY_BDA_TRANSMISSION_MODE:
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}


/*
** GetDigitalDemodProperty() method of the CTransportPin class
**
**    Gets the value of the digital demodulator node properties.
**
*/
NTSTATUS
CTransportPin::GetDigitalDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetDigitalDemodProperty"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


    //  Call the BDA support library to 
    //  validate that the node type is associated with this pin.
    //
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
        //  Obtain a pointer to the pin object.
        //
        //  Because the property dispatch table calls the CTransportPin::GetDigitalDemodProperty() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_MODULATION_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_SYMBOL_RATE:
            break;
    
        case KSPROPERTY_BDA_SPECTRAL_INVERSION:
            break;
    
        case KSPROPERTY_BDA_GUARD_INTERVAL:
            break;
    
        case KSPROPERTY_BDA_TRANSMISSION_MODE:
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}
#endif // !ATSC_RECEIVER


/*
** PutExtensionProperties() method of the CTransportPin class
**
**    Sets the value of the demodulator node extension properties.
**
*/
NTSTATUS
CTransportPin::PutExtensionProperties(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutExtensionProperties"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


    //  Call the BDA support library to 
    //  validate that the node type is associated with this pin.
    //
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
        //  Obtain a pointer to the pin object.
        //
        //  Because the property dispatch table calls the CTransportPin::PutExtensionProperties() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1:
            Status = pFilter->SetDemodProperty1(*pulProperty);
            break;
    
        case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2:
            Status = pFilter->SetDemodProperty1(*pulProperty);
            break;
    
	// KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3 does not have a SetHandler
        // according to declaration of BdaSampleDemodExtensionProperties

        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

/*
** GetExtensionProperties() method of the CTransportPin class
**
**    Retrieves the value of the demodulator node extension properties.
**
*/
NTSTATUS
CTransportPin::GetExtensionProperties(
    IN PIRP         Irp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin * pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetExtensionProperties"));

    ASSERT(Irp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);

    //  Obtain a pointer to the pin object.
    //
    //  Because the property dispatch table calls the CTransportPin::GetExtensionProperties() 
    //  method directly, the method must retrieve a pointer to the underlying pin object.
    //
    pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(Irp)->Context);
    ASSERT(pPin);

    //  Retrieve the filter context from the pin context.
    //
    pFilter = pPin->GetFilter();
    ASSERT( pFilter);

    switch (pKSProperty->Id)
    {
    case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1:
        Status = pFilter->GetDemodProperty1(pulProperty);
        break;

    // KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2 does not have a GetHandler
    // according to declaration of BdaSampleDemodExtensionProperties

    case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3:
        Status = pFilter->GetDemodProperty3(pulProperty);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

