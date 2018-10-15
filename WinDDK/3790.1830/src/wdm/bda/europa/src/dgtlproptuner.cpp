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


#include "DgtlPropTuner.h"
#include "DgtlTunerFilterinterface.h"
#include "dgtltunerfilter.h"
#include "AVDependDelayExecution.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlPropTuner::CDgtlPropTuner()
{
    //RF tuner property functions

    m_dwFrequencyMultiplier = 1000;
    m_dwFrequency = 471250000;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlPropTuner::~CDgtlPropTuner()
{

}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  get handler for frequency multiplier property
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::HW_InitializeTunerHardware
(
    IN CVampDeviceResources* pDevResObj
)
{
    //*** initialize demodulator ***//

    UCHAR ucWriteValue[2];
    // channel decoder TDA10045
    pDevResObj->m_pI2c->SetSlave(0x10);
    //close I2C connection
    //CONFC4 0x00
    ucWriteValue[0] = 0x07;
    ucWriteValue[1] = 0x00;
    eI2cStatus I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: close I2C connection failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    // channel decoder TDA10045
    pDevResObj->m_pI2c->SetSlave(0x10);
    //AUTO 0x07
    ucWriteValue[0] = 0x01;
    ucWriteValue[1] = 0x07;

    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: AUTO 0x07 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFADC1 0x2e
    ucWriteValue[0] = 0x15;
    ucWriteValue[1] = 0x2E;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFADC1 0x2e failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFC1 0x01
    ucWriteValue[0] = 0x16;
    ucWriteValue[1] = 0x08;//masked 0x01 for URD readability
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFC1 0x01 failed I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFPLL_M_MSB 0x00
    ucWriteValue[0] = 0x2e;
    ucWriteValue[1] = 0x00;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFPLL_M_MSB 0x00 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFPLL_M_LSB 0x13
    ucWriteValue[0] = 0x2f;
    ucWriteValue[1] = 0x13;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFPLL_M_LSB 0x13 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFPLL_P 0x00
    ucWriteValue[0] = 0x2d;
    ucWriteValue[1] = 0x00;

    I2CStatus = SW_ERR;
    while(I2CStatus >= 9)
    {
        I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
        _DbgPrintF(
            DEBUGLVL_BLAB,
            ("Error: CONFPLL_P 0x00 failed, I2C access status: %d",I2CStatus));
        //return STATUS_UNSUCCESSFUL;
    }

    CAVDependDelayExecution* pDelay = new (NON_PAGED_POOL) CAVDependDelayExecution();
    //now we have to wait a second to get the PLL locked on the new value
    if( pDelay )
    {
        pDelay->DelayExecutionThread(100);
        delete pDelay;
        pDelay = NULL;
    }


    //CONFPLL_N 0x00
    ucWriteValue[0] = 0x30;
    ucWriteValue[1] = 0x00;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFPLL_N 0x00 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //UNSURW_MSB 0x46
    ucWriteValue[0] = 0x31;
    ucWriteValue[1] = 0x46;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: UNSURW_MSB 0x46 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //UNSURW_LSB 0x03
    ucWriteValue[0] = 0x32;
    ucWriteValue[1] = 0x03;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: UNSURW_LSB 0x03 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //WREF_MSB 0xa7
    ucWriteValue[0] = 0x33;
    ucWriteValue[1] = 0xa7;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: WREF_MSB 0xa7 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //WREF_MID 0xfa
    ucWriteValue[0] = 0x34;
    ucWriteValue[1] = 0xfa;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: WREF_MID 0xfa failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //WREF_LSB 0x83
    ucWriteValue[0] = 0x35;
    ucWriteValue[1] = 0x83;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: WREF_LSB 0x83 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //MUXOUT 0x00
    ucWriteValue[0] = 0x36;
    ucWriteValue[1] = 0x00;//masked 0x00 for URD readability
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: MUXOUT 0x00 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFADC2 0x07
    ucWriteValue[0] = 0x37;
    ucWriteValue[1] = 0x07;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONFADC2 0x07 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //IFOFFSET 0x00
    ucWriteValue[0] = 0x38;
    ucWriteValue[1] = 0x00;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: IFOFFSET 0x00 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //DSP software reset
    //CONF4 0x0a
    ucWriteValue[0] = 0x07;
    ucWriteValue[1] = 0x0a;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONF4 0x0a (software reset) failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    //CONFC4 0x00
    ucWriteValue[0] = 0x07;
    ucWriteValue[1] = 0x00;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if( I2CStatus >= 9 )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CONF4 0x00 (software reset) failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  get handler for frequency multiplier property
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeGetFrequencyMultiplier
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    *pData = m_dwFrequencyMultiplier;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  set handler for frequency multiplier property
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeSetFrequencyMultiplier
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    m_dwFrequencyMultiplier = *pData;
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  get handler for frequency property
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeGetFrequency
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PDWORD pData
)
{
    *pData = m_dwFrequency; //get rid of the multiplier here
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  set handler for frequency property
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeSetFrequency
(
    IN PIRP       pIrp,
    IN PKSP_NODE  pKSRequest,
    IN OUT PDWORD pData
)
{
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    _DbgPrintF(DEBUGLVL_BLAB,("Info: Frequency = %d", *pData));

    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pIrp);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    m_dwFrequency = *pData;

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

    if (pDgtlTunerFilter->GetClientState() >= KSSTATE_ACQUIRE)
    {
        return HW_TunerNodeSetFrequency(pDevResObj);
    }

    return STATUS_SUCCESS;
}

NTSTATUS CDgtlPropTuner::HW_TunerNodeSetFrequency
(
    IN CVampDeviceResources* pDevResObj
)
{
    //*** program tuner and demodulator ***//
    //declare local variables
    BYTE ucWriteValue[2];
    eI2cStatus I2CStatus;
    BYTE Byte[4];

    //calculate frequency
    DWORD dwIFFrequency = 36125000;
    DWORD dwOscillatorFrequency = dwIFFrequency + m_dwFrequency * m_dwFrequencyMultiplier;

#if TD1344
    //*** 1344 implementation ***//

//  DWORD dwResult = dwOscillatorFrequency / 500000;
//  BYTE bCtrlInfoOne = 0x93;

    DWORD dwResult = dwOscillatorFrequency / 166667;
    BYTE bCtrlInfoOne = 0x88;

//    DWORD dwResult = dwOscillatorFrequency / 62500;
//    BYTE bCtrlInfoOne = 0x85;

    BYTE bCtrlInfoTwo = 0;//see if ->

    if(m_dwFrequency * m_dwFrequencyMultiplier >= 550000000)
    {
        bCtrlInfoTwo = 0xEB;//11 10 1011
    }
    else
    {
        bCtrlInfoTwo = 0xAB;//10 10 1011
    }
#else
    //*** 1316 implementation ***//

    DWORD dwResult = dwOscillatorFrequency / 166667;
    BYTE bCtrlInfoOne = 0xCA;

//  DWORD dwResult = dwOscillatorFrequency / 62500;
//  BYTE bCtrlInfoOne = 0xC8;

//  DWORD dwResult = dwOscillatorFrequency / 50000;
//  BYTE bCtrlInfoOne = 0xCB;

    BYTE bCtrlInfoTwo = 0;//see if ->

    if(m_dwFrequency * m_dwFrequencyMultiplier < 94000000)
    {
        bCtrlInfoTwo = 0x61;//011 00001
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 124000000)
    {
        bCtrlInfoTwo = 0xA1;//101 00001
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 164000000)
    {
        bCtrlInfoTwo = 0xC1;//110 00001
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 254000000)
    {
        bCtrlInfoTwo = 0x62;//011 00010
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 384000000)
    {
        bCtrlInfoTwo = 0xA2;//101 00010
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 444000000)
    {
        bCtrlInfoTwo = 0xC2;//110 00010
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 584000000)
    {
        bCtrlInfoTwo = 0x6C;//011 01100
    }
    else if(m_dwFrequency * m_dwFrequencyMultiplier < 794000000)
    {
        bCtrlInfoTwo = 0xAC;//101 01100
    }
    else
    {
        bCtrlInfoTwo = 0xAC;//111 01100
    }
#endif

    BYTE bProgDivOne = static_cast<BYTE>(dwResult >> 8);
    BYTE bProgDivTwo = static_cast<BYTE>(dwResult & 0xFF);

    _DbgPrintF(DEBUGLVL_BLAB,("Info: Try at %x, %x, %x, %x", bProgDivOne, bProgDivTwo, bCtrlInfoOne, bCtrlInfoTwo));
    //got the td1344 tuner...

    // channel decoder TDA10045
    pDevResObj->m_pI2c->SetSlave(0x10);
    //close I2C connection
    //CONFC4 0x00
    ucWriteValue[0] = 0x07;
    ucWriteValue[1] = 0x00;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: close I2C connection failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }


    // IF PLL TDA9886
    pDevResObj->m_pI2c->SetSlave(0x86);

    //write_byte_1 0x40
    ucWriteValue[0] = 0x04;
    ucWriteValue[1] = 0x40;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue, 0x02);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: IF PLL TDA9886 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    // channel decoder TDA10045
    pDevResObj->m_pI2c->SetSlave(0x10);

    //CONFC4 0x02 (open I2C connection)
    ucWriteValue[0] = 0x07;
    ucWriteValue[1] = 0x02;
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(ucWriteValue,0x02);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: open I2C connection failed: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }

    // Tuner TD1344
    pDevResObj->m_pI2c->SetSlave(0xC2);

    //Tuner, set frequency
    Byte[0] = static_cast<BYTE>(bProgDivOne);
    Byte[1] = static_cast<BYTE>(bProgDivTwo);
    Byte[2] = static_cast<BYTE>(bCtrlInfoOne);
    Byte[3] = static_cast<BYTE>(bCtrlInfoTwo);
    I2CStatus = pDevResObj->m_pI2c->WriteSeq(Byte,0x04);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Tuner TD1344 failed, I2C access status: %d",I2CStatus));
        return STATUS_UNSUCCESSFUL;
    }   
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  get handler to see if signal is present
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeGetSignalPresent
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PBOOL pData
)
{
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pIrp);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** channel decoder TDA10045 ***
    pDevResObj->m_pI2c->SetSlave(0x10);
    // Read channel decoder status
    BYTE ucWrite = 0x06;//STATUS_CD
    BYTE ucRead = 0x00;
    eI2cStatus I2CStatus = pDevResObj->m_pI2c->CombinedSeq(&ucWrite,0x01, &ucRead, 0x01);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: I2C access failed, status: %d",I2CStatus));
    }
    if(ucRead == 0x2f)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal present"));
        *pData = TRUE;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal not present"));
        *pData = FALSE;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  get handler to see if signal is locked
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::RFTunerNodeGetSignalLocked
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PBOOL pData
)
{
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pIrp);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** channel decoder TDA10045 ***
    pDevResObj->m_pI2c->SetSlave(0x10);
    // Read channel decoder status
    BYTE ucWrite = 0x06;//STATUS_CD
    BYTE ucRead = 0x00;
    eI2cStatus I2CStatus = pDevResObj->m_pI2c->CombinedSeq(&ucWrite,0x01, &ucRead, 0x01);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: I2C access failed, status: %d",I2CStatus));
    }
    if(ucRead == 0x2f)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal locked"));
        *pData = TRUE;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal not locked"));
        *pData = FALSE;
    }
    return STATUS_SUCCESS;
}

NTSTATUS CDgtlPropTuner::RFTunerNodeGetSampleTime
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    //*** get device resource object ***//
    if( !pIrp || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    *pData = 100;
    return STATUS_SUCCESS;
}

NTSTATUS CDgtlPropTuner::RFTunerNodeGetSignalQuality
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    if( !pIrp || !pKSRequest || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pIrp);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** channel decoder TDA10045 ***
    pDevResObj->m_pI2c->SetSlave(0x10);
    // Read channel decoder status
    BYTE ucWrite = 0x06;//STATUS_CD
    BYTE ucRead = 0x00;
    eI2cStatus I2CStatus = pDevResObj->m_pI2c->CombinedSeq(&ucWrite,0x01, &ucRead, 0x01);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: I2C access failed, status: %d",I2CStatus));
    }
    if(ucRead == 0x2f)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal locked"));
        *pData = 100;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal not locked"));
        *pData = 0;
    }
    return STATUS_SUCCESS;
}

NTSTATUS CDgtlPropTuner::RFTunerNodeGetSignalStrength
(
    IN PIRP pIrp,
    IN PKSP_NODE pKSRequest,
    IN OUT PLONG pData
)
{
    //*** get device resource object ***//
    if( !pIrp || !pData )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    //get the device resource object out of the Irp
    CVampDeviceResources* pDevResObj = GetDeviceResource(pIrp);
    if( !pDevResObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }

    //*** channel decoder TDA10045 ***
    pDevResObj->m_pI2c->SetSlave(0x10);
    // Read channel decoder status
    BYTE ucWrite = 0x06;//STATUS_CD
    BYTE ucRead = 0x00;
    eI2cStatus I2CStatus = pDevResObj->m_pI2c->CombinedSeq(&ucWrite,0x01, &ucRead, 0x01);
    if(I2CStatus >= 9)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: I2C access failed, status: %d",I2CStatus));
    }
    if(ucRead == 0x2f)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal strength OK"));
        *pData = 100;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("Info: Signal strength not OK"));
        *pData = 0;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  start handler for demodulation
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::COFDMDemodulatorNodeStartHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pKSRequest,
    IN OUT PVOID pData
)
{
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  stop handler for demodulation
//
// Return Value:
//  returns NTSTATUS result
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlPropTuner::COFDMDemodulatorNodeStopHandler
(
    IN PIRP pIrp,
    IN PKSPROPERTY pKSRequest,
    IN OUT PVOID pData
)
{
    return STATUS_SUCCESS;
}
