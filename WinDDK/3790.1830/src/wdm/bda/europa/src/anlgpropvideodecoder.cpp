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


#include "AnlgPropVideoDecoder.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Contructor of the class CAnlgPropVideoDecoder.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////////////
CAnlgPropVideoDecoder::CAnlgPropVideoDecoder()
{
    m_bOutputEnableFlag         = FALSE;
    m_dwDecoderStandard         = static_cast<int>(KS_AnalogVideo_None);
    m_dwDefaultStandard         = static_cast<int>(KS_AnalogVideo_None);
    m_dwDefaultNumberOfLines    = 0;
    m_bDefaultSignalLocked      = FALSE;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor of the class CAnlgPropVideoDecoder.
//
// Return Value:
//  None.
//////////////////////////////////////////////////////////////////////////////
CAnlgPropVideoDecoder::~CAnlgPropVideoDecoder()
{

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
NTSTATUS CAnlgPropVideoDecoder::DecoderCapsGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_CAPS_S pData //Pointer to a variable were
                                                 //the requested data has to
                                                 //be returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    pData->StandardsSupported = 0x000ffff7;
    pData->Capabilities = 
                static_cast<int>(KS_VIDEODECODER_FLAGS_CAN_INDICATE_LOCKED);
    pData->SettlingTime = 10;//lets spend 10 ms before lock signal rises up
                             //after input change
    pData->HSyncPerVSync = 6;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Member function to get the video standard HW or from the he registry
//  if available.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropVideoDecoder::StandardGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_S pData      //Pointer to a variable were
                                                 //the requested data has to
                                                 //be returned.
)
{
    if( !pIrp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the VampDevice object out of the pIrp
    CVampDevice* pVampDevice = GetVampDevice(pIrp);
    if( !pVampDevice )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get connection to device object"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the registry access object out of the VampDevice object
    COSDependRegistry* pOSDepRegObj = pVampDevice->GetRegistryAccessObject();
    if( !pOSDepRegObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Cannot get registry access object"));
        return STATUS_UNSUCCESSFUL;
    }
    char cValueName[] = "Preferred Video Standard";
    char cRegSubKeyPath[] = "Decoder";
    VAMPRET vRet;
    vRet = pOSDepRegObj->ReadRegistry(  cValueName,
                                        &m_dwDefaultStandard,
                                        cRegSubKeyPath,
                                        static_cast<int>(KS_AnalogVideo_None));
    if( vRet != VAMPRET_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: No default video standard defined"));
    }

    switch(m_dwDefaultStandard)
    {
    case KS_AnalogVideo_PAL_B:
    case KS_AnalogVideo_PAL_D:
    case KS_AnalogVideo_PAL_G:
    case KS_AnalogVideo_PAL_H:
    case KS_AnalogVideo_PAL_I:
    case KS_AnalogVideo_PAL_N:
    case KS_AnalogVideo_PAL_N_COMBO:
    case KS_AnalogVideo_SECAM_B:
    case KS_AnalogVideo_SECAM_D:
    case KS_AnalogVideo_SECAM_G:
    case KS_AnalogVideo_SECAM_H:
    case KS_AnalogVideo_SECAM_K:
    case KS_AnalogVideo_SECAM_K1:
    case KS_AnalogVideo_SECAM_L:
    case KS_AnalogVideo_SECAM_L1:
    case KS_AnalogVideo_NTSC_433:
        m_dwDefaultNumberOfLines = 625;
        m_bDefaultSignalLocked = TRUE;
        break;
    case KS_AnalogVideo_NTSC_M:
    case KS_AnalogVideo_NTSC_M_J:
    case KS_AnalogVideo_PAL_60:
    case KS_AnalogVideo_PAL_M:
        m_dwDefaultNumberOfLines = 525;
        m_bDefaultSignalLocked = TRUE;
        break;
    default:
        m_dwDefaultNumberOfLines = 0;
        m_bDefaultSignalLocked = FALSE;
        break;
    }
    //call GetStatus to receive the actual standard from the HW,
    //fake request and data structure, we do not need these informations here
    KSPROPERTY FakeRequest;
    KSPROPERTY_VIDEODECODER_STATUS_S FakeData;
    if( StatusGetHandler(pIrp,&FakeRequest,&FakeData) != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: StandardGetHandler failed"));
        return STATUS_UNSUCCESSFUL;
    }

    pData->Value = m_dwDecoderStandard;

    return STATUS_SUCCESS;
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
NTSTATUS CAnlgPropVideoDecoder::StandardSetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_S pData      //Pointer to a variable were
                                                 //the requested data has to
                                                 //be returned.
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

    m_dwDecoderStandard = pData->Value;
    _DbgPrintF( DEBUGLVL_VERBOSE,
                ("Warning: Video standard is not set to HW"));

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The status get handler reads the current status from the hardware
//  and returns the information by pData.
//  Hardware access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed due to invalid arguments.
//  STATUS_SUCCESS       Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropVideoDecoder::StatusGetHandler
(
    IN PIRP pIrp,                                //Pointer to the Irp that
                                                 //contains the filter object.
    IN PKSPROPERTY pRequest,                     //Pointer to a structure that
                                                 //describes the property
                                                 //request.
    IN OUT PKSPROPERTY_VIDEODECODER_STATUS_S pData //Pointer to a variable
                                                   //were the requested data
                                                   //has to be returned.
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

    eVideoStandard VideoStandard = pVampDevResObj->m_pDecoder->GetStandard();
    switch(VideoStandard)
    {
    case PAL_50HZ:
        m_dwDecoderStandard = static_cast<int>(KS_AnalogVideo_PAL_B);
        pData->SignalLocked = TRUE;
        pData->NumberOfLines = 625;
        break;
    case PAL_60HZ:
        m_dwDecoderStandard = static_cast<int>(KS_AnalogVideo_PAL_60);
        pData->SignalLocked = TRUE;
        pData->NumberOfLines = 525;
        break;
    case NTSC_50HZ:
        m_dwDecoderStandard = static_cast<int>(KS_AnalogVideo_NTSC_433);
        pData->SignalLocked = TRUE;
        pData->NumberOfLines = 625;
        break;
    case NTSC_60HZ:
        m_dwDecoderStandard = static_cast<int>(KS_AnalogVideo_NTSC_M);
        pData->SignalLocked = TRUE;
        pData->NumberOfLines = 525;
        break;
    case SECAM_50HZ:
        m_dwDecoderStandard = static_cast<int>(KS_AnalogVideo_SECAM_L);
        pData->SignalLocked = TRUE;
        pData->NumberOfLines = 625;
        break;
    case PAL:
    case NTSC_443_50:
    case PAL_N:
    case NTSC_N:
    case SECAM:
    case NTSC_M:
    case PAL_443_60:
    case NTSC_443_60:
    case PAL_M:
    case NTSC_JAP:
    case AUTOSTD:
    case NO_COLOR_50HZ:
    case NO_COLOR_60HZ:
    default:
        //see registry entry "default standard",
        //already done in GetStandard handler
        m_dwDecoderStandard = m_dwDefaultStandard;
        pData->SignalLocked =
                    static_cast<unsigned long>(m_bDefaultSignalLocked);
        pData->NumberOfLines = m_dwDefaultNumberOfLines;
        break;
    }
    return STATUS_SUCCESS;
}
