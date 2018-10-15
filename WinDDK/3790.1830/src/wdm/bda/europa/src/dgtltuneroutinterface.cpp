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
#include "ProxyKM.h"
#include "DgtlTunerOutInterface.h"
#include "DgtlTunerOut.h"


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerOutPinCreate
(
    IN PKSPIN pKSPin,
    IN PIRP pIrp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerOutPinCreate called"));
    //parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the target class object out of the KS pin context
    CDgtlTunerOut* pDgtlTunerOut = new (NON_PAGED_POOL) CDgtlTunerOut();
    //class object found?
    if( !pDgtlTunerOut )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: No DgtlTunerOut class object found"));
        return STATUS_UNSUCCESSFUL;
    }

    pKSPin->Context = pDgtlTunerOut;

    //call class function
    Status = pDgtlTunerOut->Create(pKSPin, pIrp);

    if( Status != STATUS_SUCCESS )
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
//  tbd
//
// Return Value:
//  tbd
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerOutPinRemove
(
    IN PKSPIN pKSPin,
    IN PIRP pIrp
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: DgtlTunerOutPinRemove called"));
    //parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the target class object out of the KS pin context
    CDgtlTunerOut* pDgtlTunerOut =
        static_cast <CDgtlTunerOut*> (pKSPin->Context);
    //class object found?
    if( !pDgtlTunerOut )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: No DgtlTunerOut object found"));
        return STATUS_UNSUCCESSFUL;
    }
    //call class function
    Status = pDgtlTunerOut->Remove(pKSPin, pIrp);

    if( Status != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Class function called without success"));
        return STATUS_UNSUCCESSFUL;
    }
    //de-allocate memory for our class object
    delete pDgtlTunerOut;
    pDgtlTunerOut = NULL;
    //remove the class object from the context structure of the KS pin
    pKSPin->Context = NULL;
    return STATUS_SUCCESS;
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
NTSTATUS DgtlTunerOutIntersectDataFormat
(
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
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF( DEBUGLVL_BLAB,
                ("Info: DgtlTunerOutIntersectDataFormat called"));

    if( DataBufferSize == 0 )
    {
        //data size only query
        *DataSize = sizeof(KS_DATARANGE_BDA_TRANSPORT);
        return STATUS_BUFFER_OVERFLOW;
    }
    else if( DataBufferSize == sizeof(KS_DATARANGE_BDA_TRANSPORT) )
    {
        if( Data )
        {
            *DataSize = sizeof(KS_DATARANGE_BDA_TRANSPORT);
            RtlCopyMemory(  Data,
                            DataRange,
                            sizeof(KS_DATARANGE_BDA_TRANSPORT)); // void function
            return STATUS_SUCCESS;
        }
    }
    return STATUS_BUFFER_TOO_SMALL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Callback function to set tuner output pin's device state.
//
// Return Value:
//  status
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS DgtlTunerOutSetDeviceState
(
    IN PKSPIN pKSPin,           // Pointer to KSPIN.
    IN KSSTATE ToState,         // State that has to be active after this call.
    IN KSSTATE FromState        // State that was active before this call.    
)
{
    CDgtlTunerOut *pDgtlTunerOut = static_cast<CDgtlTunerOut *>(pKSPin->Context);
    if (!pDgtlTunerOut) 
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Could not obtain pin context\n"));
        return STATUS_UNSUCCESSFUL;
    }

    return pDgtlTunerOut->SetDeviceState(pKSPin, ToState, FromState);
}