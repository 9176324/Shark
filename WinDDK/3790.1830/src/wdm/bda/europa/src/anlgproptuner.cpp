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
#include "Device.h"
#include "VampDevice.h"
#include "AnlgPropTuner.h"
#include "AnlgTunerFilterInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTuner::CAnlgPropTuner()
{
    m_dwTuningFlags = KS_TUNER_TUNING_EXACT;
    m_dwStandard = KS_AnalogVideo_None;
    m_dwSearchFlag = BG_FLAG;
#if TD1344
    //1344
    m_dwMinFrequency = EUROPA_MIN_FREQUENCY;
    m_dwMaxFrequency = EUROPA_MAX_FREQUENCY;
    m_dwAnalogFrequency = EUROPA_ANALOG_FREQUENCY;
    m_dwChannel = EUROPA_DEFAULT_CHANNEL;
    m_dwCountry = EUROPA_DEFAULT_COUNTRYCODE;
	m_nStandardsSupported = KS_AnalogVideo_PAL_B    |
                            KS_AnalogVideo_PAL_G    |
                            KS_AnalogVideo_PAL_I    |
                            KS_AnalogVideo_PAL_M    |
                            KS_AnalogVideo_SECAM_B  |
                            KS_AnalogVideo_SECAM_D  |
                            KS_AnalogVideo_SECAM_G  |
                            KS_AnalogVideo_SECAM_K  |
                            KS_AnalogVideo_SECAM_K1 |
                            KS_AnalogVideo_SECAM_L  |
                            KS_AnalogVideo_SECAM_L1;
#else
    //1316
    m_dwMinFrequency = EUROPA_MIN_FREQUENCY_1316;
    m_dwMaxFrequency = EUROPA_MAX_FREQUENCY;
    m_dwAnalogFrequency = EUROPA_ANALOG_FREQUENCY;
    m_dwChannel = EUROPA_DEFAULT_CHANNEL;
    m_dwCountry = EUROPA_DEFAULT_COUNTRYCODE;
	m_nStandardsSupported = KS_AnalogVideo_PAL_B    |
                            KS_AnalogVideo_PAL_G    |
                            KS_AnalogVideo_PAL_I    |
                            KS_AnalogVideo_PAL_M    |
                            KS_AnalogVideo_SECAM_B  |
                            KS_AnalogVideo_SECAM_D  |
                            KS_AnalogVideo_SECAM_G  |
                            KS_AnalogVideo_SECAM_K  |
                            KS_AnalogVideo_SECAM_K1 |
                            KS_AnalogVideo_SECAM_L  |
                            KS_AnalogVideo_SECAM_L1;
#endif
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgPropTuner::~CAnlgPropTuner()
{
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  sets the desired frequency via I2C on the tuner
//
// Return Value:
//  returns status of tuning
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::TuneToFrequency
(
    CVampDeviceResources* pVampDevResObj
)
{
    //*** start to program tuner ***//

    //calculate frequency
    DWORD dwIFFrequency = 38900000;
    DWORD dwOscillatorFrequency = dwIFFrequency + m_dwAnalogFrequency;

#if TD1344
    //*** 1344 implementation ***//

//  DWORD dwResult = dwOscillatorFrequency / 500000;
//  BYTE bCtrlInfoOne = 0x93;

//  DWORD dwResult = dwOscillatorFrequency / 166667;
//  BYTE bCtrlInfoOne = 0x88;

    DWORD dwResult = dwOscillatorFrequency / 62500;
    BYTE bCtrlInfoOne = 0x85;

    BYTE bCtrlInfoTwo = 0;//see if ->

    if(m_dwAnalogFrequency >= 550000000)
    {
        bCtrlInfoTwo = 0xEB;//11 10 1011
    }
    else
    {
        bCtrlInfoTwo = 0xAB;//10 10 1011
    }
#else
    //*** 1316 implementation ***//

//  DWORD dwResult = dwOscillatorFrequency / 166667;
//  BYTE bCtrlInfoOne = 0xCA;

    DWORD dwResult = dwOscillatorFrequency / 62500;
    BYTE bCtrlInfoOne = 0xC8;

//  DWORD dwResult = dwOscillatorFrequency / 50000;
//  BYTE bCtrlInfoOne = 0xCB;

    BYTE bCtrlInfoTwo = 0;//see if ->

    if(m_dwAnalogFrequency <= 130000000)
    {
        bCtrlInfoTwo = 0x61;//011 00001
    }
    else if(m_dwAnalogFrequency < 160000000)
    {
        bCtrlInfoTwo = 0xA1;//101 00001
    }
    else if(m_dwAnalogFrequency < 200000000)
    {
        bCtrlInfoTwo = 0xC1;//110 00001
    }
    else if(m_dwAnalogFrequency < 290000000)
    {
        bCtrlInfoTwo = 0x62;//011 00010
    }
    else if(m_dwAnalogFrequency < 420000000)
    {
        bCtrlInfoTwo = 0xA2;//101 00010
    }
    else if(m_dwAnalogFrequency < 480000000)
    {
        bCtrlInfoTwo = 0xC2;//110 00010
    }
    else if(m_dwAnalogFrequency < 620000000)
    {
        bCtrlInfoTwo = 0x64;//011 00100
    }
    else if(m_dwAnalogFrequency < 830000000)
    {
        bCtrlInfoTwo = 0xA4;//101 00100
    }
    else
    {
        bCtrlInfoTwo = 0xE4;//111 00100
    }
#endif

    BYTE bProgDivOne = static_cast<BYTE>(dwResult >> 8);
    BYTE bProgDivTwo = static_cast<BYTE>(dwResult & 0xFF);

    BYTE ucWriteValue[2];
    // Channel decoder TDA10045 (close tuner connection)
    pVampDevResObj->m_pI2c->SetSlave(0x10);
    ucWriteValue[0] = 0x07;//CONFC4
    ucWriteValue[1] = 0x00;
    pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

    // IF PLL TDA9886 (switch to analog mode)
    pVampDevResObj->m_pI2c->SetSlave(0x86);
    switch(m_dwStandard)
    {
    case KS_AnalogVideo_PAL_B:
    case KS_AnalogVideo_PAL_D:
    case KS_AnalogVideo_PAL_G:
    case KS_AnalogVideo_PAL_H:
    case KS_AnalogVideo_PAL_I:
    case KS_AnalogVideo_PAL_M:
    case KS_AnalogVideo_PAL_N:
        //set IF Pll to PAL standard
        ucWriteValue[0] = 0x04;//Write_Byte_1
        ucWriteValue[1] = 0x54;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x01;//Write_Byte_2
        ucWriteValue[1] = 0x70;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x02;//Write_Byte_3
        ucWriteValue[1] = 0x09;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        break;
    case KS_AnalogVideo_SECAM_B:
    case KS_AnalogVideo_SECAM_D:
    case KS_AnalogVideo_SECAM_G:
    case KS_AnalogVideo_SECAM_H:
    case KS_AnalogVideo_SECAM_K:
    case KS_AnalogVideo_SECAM_K1:
        //set IF Pll to SECAM B/G/D/K/K1 standard
        ucWriteValue[0] = 0x04;//Write_Byte_1
        ucWriteValue[1] = 0x54;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x01;//Write_Byte_2
        ucWriteValue[1] = 0x70;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x02;//Write_Byte_3
        ucWriteValue[1] = 0x0B;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        break;
    case KS_AnalogVideo_SECAM_L:
        //set IF Pll to SECAM L standard
        ucWriteValue[0] = 0x04;//Write_Byte_1
        ucWriteValue[1] = 0x44;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x01;//Write_Byte_2
        ucWriteValue[1] = 0x70;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x02;//Write_Byte_3
        ucWriteValue[1] = 0x0B;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        break;
    case KS_AnalogVideo_SECAM_L1:
        //set IF Pll to SECAM L' standard
        ucWriteValue[0] = 0x04;//Write_Byte_1
        ucWriteValue[1] = 0x44;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x01;//Write_Byte_2
        ucWriteValue[1] = 0x70;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x02;//Write_Byte_3
        ucWriteValue[1] = 0x13;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        break;
    default:
        //set IF Pll to Pal standard
        ucWriteValue[0] = 0x04;//Write_Byte_1
        ucWriteValue[1] = 0x54;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x01;//Write_Byte_2
        ucWriteValue[1] = 0x70;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

        ucWriteValue[0] = 0x02;//Write_Byte_3
        ucWriteValue[1] = 0x0B;
        pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        break;
    }
    //got the old tuner td1344...
    // Channel decoder TDA10045 (open tuner connection)
    pVampDevResObj->m_pI2c->SetSlave(0x10);

    ucWriteValue[0] = 0x07;//CONFC4
    ucWriteValue[1] = 0x02;
    pVampDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);

    // Tuner TD1344
    pVampDevResObj->m_pI2c->SetSlave(0xC2);
    BYTE Byte[4] = {bProgDivOne, bProgDivTwo, bCtrlInfoOne, bCtrlInfoTwo};
    pVampDevResObj->m_pI2c->WriteSeq(Byte,0x04);

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerCapsGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_CAPS_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->ModesSupported = KSPROPERTY_TUNER_MODE_TV;

    PKSFILTER pFilter = KsGetFilterFromIrp(Irp);
    if (!pFilter)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get filter from Irp"));
        return STATUS_UNSUCCESSFUL;
    }
    PKSDEVICE pKSDevice = KsFilterGetDevice(pFilter);
    if (!pKSDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get device from filter"));
        return STATUS_UNSUCCESSFUL;
    }
    CVampDevice *pVampDevice = (CVampDevice *) pKSDevice->Context;
    if (!pVampDevice)
    {
        _DbgPrintF(DEBUGLVL_ERROR, ("Error: Unable to get device context"));
        return STATUS_UNSUCCESSFUL;
    }
    UINT uiMediumId = pVampDevice->GetDriverMediumInstanceCount();

    pData->VideoMedium = AnlgXBarTunerVideoInMedium[0];
    pData->VideoMedium.Id = uiMediumId;

    pData->TVAudioMedium = AnlgTVAudioInMedium[0];
    pData->TVAudioMedium.Id = uiMediumId;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerModeCapsGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_CAPS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_CAPS_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    if(pRequest->Mode != KSPROPERTY_TUNER_MODE_TV)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Tuning mode not supported"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->Mode = KSPROPERTY_TUNER_MODE_TV;

    pData->StandardsSupported = KS_AnalogVideo_PAL_B    |
                                KS_AnalogVideo_PAL_G    |
                                KS_AnalogVideo_PAL_I    |
                                KS_AnalogVideo_PAL_M    |
                                KS_AnalogVideo_SECAM_B  |
                                KS_AnalogVideo_SECAM_D  |
                                KS_AnalogVideo_SECAM_G  |
                                KS_AnalogVideo_SECAM_K  |
                                KS_AnalogVideo_SECAM_K1 |
                                KS_AnalogVideo_SECAM_L  |
                                KS_AnalogVideo_SECAM_L1; //support PAL, SECAM

    pData->MinFrequency = EUROPA_MIN_FREQUENCY;
    pData->MaxFrequency = EUROPA_MAX_FREQUENCY;

    pData->TuningGranularity = EUROPA_TUNING_GRANULARITY;
    pData->NumberOfInputs = 1;
    pData->SettlingTime = EUROPA_SETTLING_TIME;
    pData->Strategy = KS_TUNER_STRATEGY_DRIVER_TUNES;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner mode
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerModeGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->Mode = KSPROPERTY_TUNER_MODE_TV; 
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner capabilities
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerModeSetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_MODE_S pRequest,
    IN OUT PKSPROPERTY_TUNER_MODE_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    if(pRequest->Mode != KSPROPERTY_TUNER_MODE_TV)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: AnlgTunerModeSetHandler failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner standard
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerStandardGetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
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
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //get standard from video decoder
    eVideoStandard VideoStandard = pVampDevResObj->m_pDecoder->GetStandard();
    switch(VideoStandard)
    {
    case PAL:
        pData->Standard = KS_AnalogVideo_PAL_B;
        break;
    case NTSC_M:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NTSC_443_50:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case PAL_N:
        pData->Standard = KS_AnalogVideo_PAL_N;
        break;
    case NTSC_N:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case SECAM:
        pData->Standard = KS_AnalogVideo_SECAM_L;
        break;
    case PAL_443_60:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NTSC_443_60:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case PAL_M:
        pData->Standard = KS_AnalogVideo_PAL_M;
        break;
    case NTSC_JAP:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case PAL_50HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NTSC_50HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NTSC_60HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case PAL_60HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case SECAM_50HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NO_COLOR_50HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    case NO_COLOR_60HZ:
        pData->Standard = KS_AnalogVideo_None;
        break;
    default:
        pData->Standard = KS_AnalogVideo_None;
        break;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner standard
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerStandardSetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_STANDARD_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STANDARD_S pData
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

	if( (pRequest->Standard & m_nStandardsSupported) == 0 )
	{
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Invalid standard"));
        return STATUS_UNSUCCESSFUL;
	}

    //store standard for later use
    m_dwStandard = pRequest->Standard;
    PKSFILTER pKSFilter = KsGetFilterFromIrp(pIrp);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // save the new standard to the registry ( for audio detection )
    NTSTATUS ntStatus = SaveDetectedStandardToRegistry( pKSFilter,
                                                        pRequest->Standard );
    if( ntStatus != STATUS_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Could not write standard to registry"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function stores the new video standard to the registry. The audio
//  detection will check the entry in the registry after the channel change.
//
// Return Value:
//  STATUS_SUCCESS          Wrote the new standard to the registry.
//  STATUS_UNSUCCESSFUL     Any error case.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::SaveDetectedStandardToRegistry
(
    IN PKSFILTER pKSFilter, //KS filter, used for system support function calls
    IN DWORD     dwStandard //new standard to set to the registry
)
{
    //parameters valid?
    if( !pKSFilter )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the VampDevice object out of the KSFilter object
    CVampDevice* pVampDevice = GetVampDevice(pKSFilter);
    if( !pVampDevice )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get connection to device object"));
        return STATUS_UNSUCCESSFUL;
    }

    DWORD dwSearchFlag = 0;
    //translate standard for audio purposes
    switch(dwStandard)
    {
        case KS_AnalogVideo_PAL_B:
        case KS_AnalogVideo_PAL_G:
        case KS_AnalogVideo_SECAM_B:
        case KS_AnalogVideo_SECAM_G:
            dwSearchFlag = BG_FLAG;
            break;
        case KS_AnalogVideo_PAL_D:
        case KS_AnalogVideo_SECAM_D:
        case KS_AnalogVideo_SECAM_K:
        case KS_AnalogVideo_SECAM_K1:
            dwSearchFlag = DK_FLAG;
            break;
        case KS_AnalogVideo_NTSC_M:
        case KS_AnalogVideo_NTSC_M_J:
        case KS_AnalogVideo_PAL_M:
            dwSearchFlag = M_FLAG;
            break;
        case KS_AnalogVideo_PAL_I:
            dwSearchFlag = I_FLAG;
            break;
        case KS_AnalogVideo_SECAM_L:
        case KS_AnalogVideo_SECAM_L1:
            dwSearchFlag = L_FLAG;
            break;
        default:
            dwSearchFlag = BG_FLAG;
            break;
    }
    //provide standard for audio in the registry
    //get the registry access object out of the VampDevice object
    COSDependRegistry* pOSDepRegObj =
        pVampDevice->GetRegistryAccessObject();
    if( !pOSDepRegObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get registry access object"));
        return STATUS_UNSUCCESSFUL;
    }
    VAMPRET VampReturnValue =
        pOSDepRegObj->WriteRegistry("00",
                                    dwSearchFlag,
                                    "Audio\\StandardTable");
    // check the return value
    if( VAMP_ERROR(VampReturnValue) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot write to the registry"));
        return STATUS_UNSUCCESSFUL;
    }
    // return with success
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner frequency
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerFrequencyGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->Frequency = m_dwAnalogFrequency;
    pData->LastFrequency = m_dwAnalogFrequency;
    pData->TuningFlags = m_dwTuningFlags;
    pData->VideoSubChannel;//not used
    pData->AudioSubChannel;//not used
    pData->Channel = m_dwChannel;
    pData->Country = m_dwCountry;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner Frequency
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerFrequencySetHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY_TUNER_FREQUENCY_S pRequest,
    IN OUT PKSPROPERTY_TUNER_FREQUENCY_S pData
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

    _DbgPrintF(DEBUGLVL_BLAB,("Info: pRequest->Frequency %x",pRequest->Frequency));
    if((pRequest->Frequency < 471250000) || (pRequest->Frequency > 855250000))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Frequency out of range"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the device resource object out of the Irp
    CVampDeviceResources* pVampDevResObj = GetDeviceResource(pIrp);
    if( !pVampDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //store given information
    pData->LastFrequency;//not used
    pData->VideoSubChannel;//not used
    pData->AudioSubChannel;//not used
    m_dwChannel = pData->Channel;
    m_dwCountry = pData->Country;
    m_dwAnalogFrequency = pRequest->Frequency;
    m_dwTuningFlags = pRequest->TuningFlags;
    //*** set new channel on tuner ***//
    TuneToFrequency(pVampDevResObj);
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner input
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerInputGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->InputIndex = 0;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  set handler for analog tuner input
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerInputSetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_INPUT_S pRequest,
    IN OUT PKSPROPERTY_TUNER_INPUT_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //check current process handle
    PKSFILTER pKsFilter = KsGetFilterFromIrp(Irp);
    CVampDevice *pVampDevice = GetVampDevice(pKsFilter);
    PEPROCESS pCurrentProcessHandle = pVampDevice->GetOwnerProcessHandle();
    if( (pCurrentProcessHandle != NULL) && (PsGetCurrentProcess() != pCurrentProcessHandle) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: Set Handler was called from unregistered process"));
        return STATUS_UNSUCCESSFUL;
    }

    if(pRequest->InputIndex != 0)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Only one tuner input supported"));
        return STATUS_UNSUCCESSFUL;
    }
    pRequest->InputIndex;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  get handler for analog tuner status
//
// Return Value:
//
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgPropTuner::AnlgTunerStatusGetHandler
(
    IN PIRP Irp,
    IN PKSPROPERTY_TUNER_STATUS_S pRequest,
    IN OUT PKSPROPERTY_TUNER_STATUS_S pData
)
{
    if( !Irp || !pRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    pData->CurrentFrequency = m_dwAnalogFrequency;
    pData->PLLOffset;//not used
    pData->SignalStrength;//not used
    pData->Busy = FALSE;//always accept incoming tune requests
    return STATUS_SUCCESS;
}
