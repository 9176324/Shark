/*++

Copyright (c) 1999  Microsoft Corporation

Module Name:

    inpin.cpp

Abstract:

    Antenna input pin code.

--*/

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif // ALLOC_DATA_PRAGMA

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif // ALLOC_PRAGMA

/*
**    (Static) PinCreate() method of the CAntennaPin class
**
**    Creates the input pin object and
**    associates it with the filter object. 
**
*/
NTSTATUS
CAntennaPin::PinCreate(
    IN OUT PKSPIN pKSPin,
    IN PIRP Irp
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CAntennaPin*      pPin;
    CFilter*            pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PinCreate"));

    ASSERT(pKSPin);
    ASSERT(Irp);

    //  Obtain a pointer to the filter object for which the input pin is created.
    //
    pFilter = reinterpret_cast<CFilter*>(KsGetFilterFromIrp(Irp)->Context);

    //  Create the Antenna input pin object.
    //
    pPin = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CAntennaPin;  // Tags the allocated memory 
    if (pPin)
    {
        //  Link the input pin object to the filter object.
        //  That is, set the input pin's filter pointer data member to the obtained filter pointer.
        //
        pPin->SetFilter( pFilter);
    
        //  Link the Antenna input pin object to the passed in pointer to the KSPIN structure
        //  by assigning the pointer to the pin object to the KSPIN structure's context member.
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
** PinClose() method of the CAntennaPin class
**
**    Deletes the previously created input pin object.
**
*/
NTSTATUS
CAntennaPin::PinClose(
    IN OUT PKSPIN Pin,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PinClose"));

    ASSERT(Pin);
    ASSERT(Irp);

    //  Retrieve the Antenna input pin object from the passed in 
    //  KSPIN structure's context member.
    //
    CAntennaPin* pPin = reinterpret_cast<CAntennaPin*>(Pin->Context);

    ASSERT(pPin);

    delete pPin;

    return STATUS_SUCCESS;
}

/*
** IntersectDataFormat() method of the CAntennaPin class
**
**    Enables connection of the input pin with an upstream filter.
**
*/
NTSTATUS
CAntennaPin::IntersectDataFormat(
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
    if ( DataBufferSize < sizeof(KS_DATARANGE_BDA_ANTENNA) )
    {
        *DataSize = sizeof( KS_DATARANGE_BDA_ANTENNA );
        return STATUS_BUFFER_OVERFLOW;
    }
    else if (DataRange->FormatSize < sizeof(KS_DATARANGE_BDA_ANTENNA)) 
    {
	return STATUS_NO_MATCH;
    } else 
    {
        ASSERT(DataBufferSize == sizeof(KS_DATARANGE_BDA_ANTENNA));
	
        *DataSize = sizeof( KS_DATARANGE_BDA_ANTENNA );
        RtlCopyMemory( Data, (PVOID)DataRange, sizeof(KS_DATARANGE_BDA_ANTENNA));

        return STATUS_SUCCESS;
    }
}

/*
** PinSetDeviceState() method of the CAntennaPin class
**
**    Because the most upstream pin (input pin) is the last to transition, 
**    use this pin's state to set the state of the filter.
**    
**    Also, release filter resouces if the pin's state transitions to stop, and
**    acquire resources if the pin's state transitions from stop.
**
*/
NTSTATUS
CAntennaPin::PinSetDeviceState(
    IN PKSPIN Pin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PKSDEVICE       pKSDevice;
    CAntennaPin *   pPin;
    CDevice *       pDevice;

    _DbgPrintF( DEBUGLVL_VERBOSE,
                ("CAntennaPin::PinSetDeviceState"));

    ASSERT(Pin);

    //  Obtain a pointer to the device object from 
    //  the passed in pointer to the KSPIN structure.
    //
    pKSDevice = KsPinGetDevice( Pin);

    //  Obtain a pointer to the pin object from context member of
    //  the passed in pointer to the KSPIN structure.
    //
    pPin = reinterpret_cast<CAntennaPin*>(Pin->Context);
    ASSERT( pPin);

    //  Obtain a pointer to the device object from context member of
    //  the retrieved pointer to the KSDEVICE structure.
    //
    pDevice = reinterpret_cast<CDevice *>(pKSDevice->Context);
    ASSERT(pDevice);

    pPin->m_pFilter->SetDeviceState( pPin->m_KsState);

    if ((ToState == KSSTATE_STOP) && (FromState != KSSTATE_STOP))
    {
        //  Because the driver allocates resources on a filter wide basis,
        //  inform the filter to release resources when the last pin
        //  (that is, the most upstream pin) transitions to the stop state.
        //
        //  The input pin is the last pin to transition to the stop state,
        //  therefore inform the filter to release its resources.
        //
        Status = pPin->m_pFilter->ReleaseResources();
        pPin->m_KsState = ToState;
    }
    else if ((ToState == KSSTATE_ACQUIRE) && (FromState == KSSTATE_STOP))
    {
        //  Because the driver allocates resources on a filter wide basis,
        //  inform the filter to acquire resources when the last pin
        //  (that is, the most upstream pin) transitions from the stop state.
        //
        //  The input pin is the last pin to transition from the stop state,
        //  therefore inform the filter to acquire its resources.
        //
        Status = pPin->m_pFilter->AcquireResources();
        if (NT_SUCCESS( Status))
        {
            pPin->m_KsState = ToState;
        }
    }
    else if (ToState > KSSTATE_RUN)
    {
        _DbgPrintF( DEBUGLVL_TERSE,
                    ("CAntennaPin::PinSetDeviceState - Invalid Device State. ToState 0x%08x.  FromState 0x%08x.",
                     ToState, FromState));
        Status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        pPin->m_KsState = ToState;
    }

    return Status;
}

/*
** GetSignalStatus() method of the CAntennaPin class
**
**    Retrieves the value of the tuner node signal statistics properties.
**
*/
NTSTATUS
CAntennaPin::GetSignalStatus(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    CAntennaPin *               pPin;
    CFilter*                    pFilter;
    BDATUNER_DEVICE_STATUS      DeviceStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::GetSignalStatus"));

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
        //  Because the property dispatch table calls the CAntennaPin::GetTunerSignalStatus() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
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
            case KSPROPERTY_BDA_SIGNAL_PRESENT:
                *pulProperty = DeviceStatus.fCarrierPresent;
                break;
        
            default:
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }
    
    return Status;
}


/*
** GetCenterFrequency() method of the CAntennaPin class
**
**    Retrieves the value of the type 1 property of node 1.
**
*/
NTSTATUS
CAntennaPin::GetCenterFrequency(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CAntennaPin*    pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::GetCenterFrequency"));

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
        //  Because the property dispatch table calls the CAntennaPin::GetCenterFrequency() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT(pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        //  Retrieve the actual filter parameter. 
        //
        Status = pFilter->GetFrequency( pulProperty);
    }

    return Status;
}


/*
** PutCenterFrequency() method of the CAntennaPin class
**
**    Sets the value of the type 1 property of node 1 along with the 
**    resource of the filter to which the node belongs.
**
*/
NTSTATUS
CAntennaPin::PutCenterFrequency(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CAntennaPin*    pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PutCenterFrequency"));

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
        //  Because the property dispatch table calls the CAntennaPin::PutCenterFrequency() 
        //  method directly, the method must retrieve a pointer to the underlying pin object.
        //
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
        //  Retrieve the filter context from the pin context.
        //
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        //  Change the actual filter parameter. 
        //
        Status = pFilter->PutFrequency( *pulProperty);
    }

    return Status;
}

