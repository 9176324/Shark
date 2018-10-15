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
#include "DgtlTransportOutInterface.h"
#include "DgtlTransportOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the dispatch function for the pin create event of the transport
//  output pin. It creates a new pin by instantiating an object of class
//  CDgtlTransportOut and calling its Create() member function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_INSUFFICIENT_RESOURCES    Unable to create class
//                                   CVampTransportStream
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTransportOutCreate
(
    IN PKSPIN pKSPin,       // Pointer to the pin object
    IN PIRP pIrp            // Pointer to the Irp that is involved with this
                            // operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTransportOutCreate called"));
    if(!pKSPin || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    CDgtlTransportOut* pDgtlTransportOut =
                                    new (NON_PAGED_POOL) CDgtlTransportOut();
    if(!pDgtlTransportOut)
    {
        //memory allocation failed
        _DbgPrintF(DEBUGLVL_ERROR,
             ("Error: DgtlTransportOut creation failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( pDgtlTransportOut->Create(pKSPin, pIrp) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTransportOut creation failed"));
        delete pDgtlTransportOut;
        pDgtlTransportOut = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    //store the DgtlTransportOut object for later use in the
    //Context of KSPin
    pKSPin->Context = pDgtlTransportOut;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
// This is the dispatch function for the pin remove event of the transport
// output pin. It removes the current pin by calling the Remove() member
// function of the pin object and deleting the object itself.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTransportOutRemove
(
    IN PKSPIN pKSPin,       // Pointer to the pin object
    IN PIRP pIrp            // Pointer to the Irp that is involved with this
                            // operation.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTransportOutClose called"));
    //parameters valid?
    if(!pKSPin || !pIrp)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the pDgtlTransportOut object out of the Context of Pin
    CDgtlTransportOut* pDgtlTransportOut =
                        static_cast<CDgtlTransportOut*>(pKSPin->Context);
    if(!pDgtlTransportOut)
    {
        //no VampDevice object available
        _DbgPrintF(
         DEBUGLVL_ERROR,
         ("Error: DgtlTransportOutClose failed, no pDgtlTransportOut found"));
        return STATUS_UNSUCCESSFUL;
    }

    //call class function
    NTSTATUS Status = pDgtlTransportOut->Remove(pKSPin, pIrp);

    //de-allocate memory for our pDgtlTransportOut class
    delete pDgtlTransportOut;
    pDgtlTransportOut = NULL;
    //remove the pDgtlTransportOut object from the
    //context structure of Pin
    pKSPin->Context = NULL;

    if( Status != STATUS_SUCCESS)
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Class function called without success"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the pin dispatch function for the process event of the transport
//  output pin. It calls the Process() member function of the corresponding
//  pin object.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTransportOutProcess
(
    IN PKSPIN pKSPin       // Pointer to the pin object
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,(""));
    //_DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTransportOutProcess called"));
    if(!pKSPin)
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: DgtlTransportOutProcess: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the pDgtlTransportOut object out of
    //the Context of Pin
    CDgtlTransportOut* pDgtlTransportOut =
                            static_cast<CDgtlTransportOut*>(pKSPin->Context);
    if(!pDgtlTransportOut)
    {
        //no VampDevice object available
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: DgtlTransportOutProcess: DgtlTransportOutProcess failed,\
                     no pDgtlTransportOut found"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlTransportOut->Process(pKSPin);
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the pin dispatch function for the SETDEVICESTATE event of the
//  transport output pin. Dependend on the current and next state it calls
//  the member functions Open(), Close(), Start() and Stop() of the
//  corresponding pin object.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTransportOutSetDeviceState
(
    IN PKSPIN pKSPin,           // Pointer to KSPIN.
    IN KSSTATE ToState,         // State that has to be active after this call.
    IN KSSTATE FromState        // State that was active before this call.
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTransportOutSetDeviceState called"));
    _DbgPrintF(DEBUGLVL_BLAB,
           ("Info: From state 0x%02X to state 0x%02X", FromState, ToState));
    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the pDgtlTransportOut class object out of the KS pin context
    CDgtlTransportOut* pDgtlTransportOut =
                    static_cast <CDgtlTransportOut*> (pKSPin->Context);
    //class object found?
    if( !pDgtlTransportOut )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: No digital transport stream pin class object found"));
        return STATUS_UNSUCCESSFUL;
    }

    // get the KS device object out of the KS pin object
    // (Device where the pin is part of)
    PKSDEVICE pKSDevice = KsGetDevice(pKSPin);
    if( !pKSDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,("Error: Cannot get connection to KS device"));
        return NULL;
    }
    //get the VampDevice object out of the Context of KSDevice
    CVampDevice* pVampDevice = static_cast <CVampDevice*> (pKSDevice->Context);

    if (!pVampDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: cannot get connection to device object\n"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS status = STATUS_SUCCESS;
    switch(ToState)
    {
        case KSSTATE_ACQUIRE:
            _DbgPrintF(DEBUGLVL_BLAB,
                        ("Info: KSSTATE_ACQUIRE TS stream called"));
            switch(FromState)
            {
            case KSSTATE_RUN:
                //stream is running, stop it
                status = pDgtlTransportOut->Stop(pKSPin);
                break;
            case KSSTATE_PAUSE:
                //stream is opened/stopped, do nothing
                break;
            case KSSTATE_STOP:
                status = pVampDevice->SetOwnerProcessHandle(PsGetCurrentProcess());
                if (status != STATUS_SUCCESS) break;
                
                //stream is closed, aquire it
                status = pDgtlTransportOut->Open(pKSPin);
                if (status != STATUS_SUCCESS) pVampDevice->SetOwnerProcessHandle(NULL);
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                            ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_RUN:
            _DbgPrintF(DEBUGLVL_BLAB,("Info: KSSTATE_RUN TS stream called"));
            switch(FromState)
            {
            case KSSTATE_ACQUIRE:
                //stream is open, start it
                status = pDgtlTransportOut->Start(pKSPin);
                break;
            case KSSTATE_PAUSE:
                //stream is opened/stopped, start it
                status = pDgtlTransportOut->Start(pKSPin);
                break;
            case KSSTATE_STOP:
                //stream is closed, open and start it
                status = pDgtlTransportOut->Open(pKSPin);
                if(status == STATUS_SUCCESS)
                {
                    status = pDgtlTransportOut->Start(pKSPin);
                }
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                            ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_PAUSE:
            _DbgPrintF(DEBUGLVL_BLAB,
                        ("Info: KSSTATE_PAUSE TS stream called"));
            switch(FromState)
            {
            case KSSTATE_ACQUIRE:
                //stream is open, do nothing
                break;
            case KSSTATE_RUN:
                //stream is opened/started, stop it
                status = pDgtlTransportOut->Stop(pKSPin);
                break;
            case KSSTATE_STOP:
                //stream is closed, open it
                status = pDgtlTransportOut->Open(pKSPin);
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                            ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        case KSSTATE_STOP:
            _DbgPrintF(DEBUGLVL_BLAB,("Info: KSSTATE_STOP TS stream called"));
            pVampDevice->SetOwnerProcessHandle(NULL);
            switch(FromState)
            {
            case KSSTATE_ACQUIRE:
                //stream is open, close it
                status = pDgtlTransportOut->Close(pKSPin);
                break;
            case KSSTATE_RUN:
                //stream is opened/started, stop and close it
                status = pDgtlTransportOut->Stop(pKSPin);
                if(status == STATUS_SUCCESS)
                {
                    status = pDgtlTransportOut->Close(pKSPin);
                }
                break;
            case KSSTATE_PAUSE:
                //stream is opened, close it
                status = pDgtlTransportOut->Close(pKSPin);
                break;
            default:
                _DbgPrintF(DEBUGLVL_ERROR,
                            ("Error: Invalid state transition"));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
            break;
        default:
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid state transition"));
            status = STATUS_UNSUCCESSFUL;
            break;
    }
    return status;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the pin dispatch function for the framing allocator handler of
//  the transport output pin.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTransportOutAllocatorPropertyHandler
(
    IN PIRP pIrp,                           // Pointer to the Irp that is
                                            // involved with this operation.
    IN PKSPROPERTY  pKSProperty,            // Property information (not used
                                            // here)
    IN OUT PKSALLOCATOR_FRAMING pAllocator  // Pointer to the allocator
                                            // structure
)
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,
               ("Info: DgtlTransportOutAllocatorPropertyHandler called"));
    if(!pIrp || !pKSProperty || !pAllocator)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //fill allocator structure
    pAllocator->FileAlignment       = FILE_BYTE_ALIGNMENT;
    pAllocator->Frames              = NUM_TS_STREAM_BUFFER;
    pAllocator->FrameSize           = M2T_BYTES_IN_LINE * M2T_LINES_IN_FIELD;
    pAllocator->PoolType            = NonPagedPool;
    pAllocator->RequirementsFlags   = KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY;
    pAllocator->Reserved            = 0;

    return STATUS_SUCCESS;
}

