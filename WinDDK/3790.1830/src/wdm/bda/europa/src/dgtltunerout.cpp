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

#include "dgtltunerfilter.h"
#include "DgtlTunerOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTunerOut::CDgtlTunerOut()
{
    m_bFirstTimeAcquire = 1;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTunerOut::~CDgtlTunerOut()
{

}

NTSTATUS CDgtlTunerOut::Create
(
    IN PKSPIN pKSPin,   //Used for system support function calls.
    IN PIRP   pIrp      //Pointer to IRP for this request. (unused)
)
{
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    CDgtlTunerFilter* pDgtlTunerFilter =
        static_cast <CDgtlTunerFilter*>(pKsFilter->Context);
    if( !pDgtlTunerFilter )
    {
        //no pDgtlTunerFilter object available
        _DbgPrintF( DEBUGLVL_VERBOSE,
            ("Warning: Cannot get DgtlTunerFilter object"));
        return STATUS_UNSUCCESSFUL;
    }
    pDgtlTunerFilter->SetClientState(KSSTATE_STOP);

    return STATUS_SUCCESS;
}



NTSTATUS CDgtlTunerOut::Remove
(
    IN PKSPIN pKSPin,   //Used for system support function calls.
    IN PIRP   pIrp      //Pointer to IRP for this request. (unused)
)
{
    PKSFILTER pKsFilter = KsGetFilterFromIrp(pIrp);
    CDgtlTunerFilter* pDgtlTunerFilter =
        static_cast <CDgtlTunerFilter*>(pKsFilter->Context);
    if( !pDgtlTunerFilter )
    {
        //no pDgtlTunerFilter object available
        _DbgPrintF( DEBUGLVL_VERBOSE,
            ("Warning: Cannot get DgtlTunerFilter object"));
        return STATUS_UNSUCCESSFUL;
    }
    pDgtlTunerFilter->SetClientState(KSSTATE_STOP);

    return STATUS_SUCCESS;
}

NTSTATUS CDgtlTunerOut::SetDeviceState
(
    IN PKSPIN pKSPin,           // Pointer to KSPIN.
    IN KSSTATE ToState,         // State that has to be active after this call.
    IN KSSTATE FromState        // State that was active before this call.    
)
{
    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pKSPin);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }            

    PKSFILTER pKsFilter = KsPinGetParentFilter(pKSPin);
    CDgtlTunerFilter* pDgtlTunerFilter =
        static_cast <CDgtlTunerFilter*>(pKsFilter->Context);
    if( !pDgtlTunerFilter )
    {
        //no pDgtlTunerFilter object available
        _DbgPrintF( DEBUGLVL_VERBOSE,
            ("Warning: Cannot get DgtlTunerFilter object"));
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


    if ((FromState == KSSTATE_STOP) && (ToState == KSSTATE_ACQUIRE))
    {
        if (pVampDevice->SetOwnerProcessHandle(PsGetCurrentProcess()) != STATUS_SUCCESS)
            return STATUS_UNSUCCESSFUL;

        // program hardware
        CDgtlPropTuner* pDgtlPropTuner = pDgtlTunerFilter->GetPropTuner();
        if( !pDgtlPropTuner )
        {
            //no pDgtlPropTuner object available
            _DbgPrintF( DEBUGLVL_VERBOSE,
                ("Warning: Cannot get DgtlPropTuner object"));
            return STATUS_UNSUCCESSFUL;
        }
        if (m_bFirstTimeAcquire) 
        {
            m_bFirstTimeAcquire = 0;
            if (pDgtlPropTuner->HW_InitializeTunerHardware(pDevResObj) != STATUS_SUCCESS)
                return STATUS_UNSUCCESSFUL;
        }

        if (pDgtlPropTuner->HW_TunerNodeSetFrequency(pDevResObj) != STATUS_SUCCESS)
            return STATUS_UNSUCCESSFUL;
        pDgtlTunerFilter->SetClientState(KSSTATE_ACQUIRE);

    } else if (ToState == KSSTATE_STOP) {
        pVampDevice->SetOwnerProcessHandle(NULL);
        pDgtlTunerFilter->SetClientState(KSSTATE_STOP);
    }

    return STATUS_SUCCESS;
}
