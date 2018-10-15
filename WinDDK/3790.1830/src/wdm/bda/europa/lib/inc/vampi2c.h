//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
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

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampI2c | Interface of CVampI2c, an I2C driver class, using
//           SAA7134 as a master controller. The I2C driver class controls all the
//           I2C devices.
// @end
// Filename: VampI2c.h
// 
// Routines/Functions:
//
//  public:
//          CVampI2c
//          ~CVampI2c
//          SetSlave
//          GetSlave
//          SetTimeout
//          GetTimeout
//          SetClockSel
//          GetClockSel
//          SetI2cStatus
//          GetI2cStatus
//          SetAttr
//          GetAttr
//          SetTransferByte
//          I2cBusy
//          WriteSequence
//          ReadSequence
//          IsStatusError
//          GetObjectStatus
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPI2C_H__83DC1AA0_6159_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPI2C_H__83DC1AA0_6159_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OSdepend.h"
#include "VampTypes.h"
#include "VampDeviceResources.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampI2c | Methods for the controlling of the GPIO I2C master.
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CVampI2c  
{
    //@access private 
private:
    //@cmember Slave address of device.
    int m_nSlave;
    //@cmember Pointer on the CVampDeviceResources object.
    CVampDeviceResources *m_pDevRes;
    //@cmember Pointer on CVampIo object.
    CVampIo *m_pVampIo;

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@cmember Check I2C busy condition.<nl>
    //Parameterlist:<nl>
    //eI2cStatus &eStatus // return variable, (ref.) status<nl>
    BOOL I2cBusy(
        OUT eI2cStatus &eStatus );

    //@cmember Check I2C error condition.<nl>
    //Parameterlist:<nl>
    //eI2cStatus &eStatus // return variable, (ref.) status<nl>
    BOOL I2cError(
        OUT eI2cStatus &eStatus );

    //@cmember Check I2C idle condition.<nl>
    //Parameterlist:<nl>
    //eI2cStatus &eStatus // return variable, (ref.) status<nl>
    BOOL I2cIdle(
        OUT eI2cStatus &eStatus );

    //@cmember Set I2C transfer register.<nl>
    //Parameterlist:<nl>
    //BYTE ucByte // address/RW or data byte<nl>
    //eI2cAttr eAttr // transfer attribute<nl>
    //eI2cStatus eStatus // I2C status to echo<nl>
    void SetI2cTransfer(
        IN BYTE ucByte,
        IN eI2cAttr eAttr,
        IN eI2cStatus eStatus )
    {
        m_pVampIo->MunitGpio.Iic_Transfer = 
            ( ucByte << m_pVampIo->MunitGpio.Iic_ByteReadWrite.m_ucStartBit ) | 
            ( eAttr << m_pVampIo->MunitGpio.Iic_Attr.m_ucStartBit ) | 
            ( eStatus << m_pVampIo->MunitGpio.Iic_Status.m_ucStartBit );
    }

    //@cmember Get transfer byte.<nl>
    BYTE GetTransferByte()
    {
        return (BYTE)((DWORD)m_pVampIo->MunitGpio.Iic_ByteReadWrite);
    }

    //@cmember Set I2C status.<nl>
    //Parameterlist:<nl>
    //eI2cStatus eStatus // I2C status to echo<nl>
    void SetI2cStatus( 
        IN eI2cStatus eStatus ) 
    {
        m_pVampIo->MunitGpio.Iic_Status = (DWORD)eStatus;
    }

    //@cmember Set I2C transfer attribute: NOP, STOP, CONTINUE, START.<nl>
    //Parameterlist:<nl>
    //eI2cAttr eAttr // transfer attribute<nl>
    void SetAttr( 
        IN eI2cAttr eAttr )
    {                                       
        m_pVampIo->MunitGpio.Iic_Attr = eAttr;
    }

    //@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer on CVampDeviceResources object<nl> 
    //int nSlave // slave id default to zero<nl>
    CVampI2c( 
        IN CVampDeviceResources *pDevice, 
        IN int nSlave = 0 );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
	virtual ~CVampI2c();

    //@cmember Set slave addr of I2C device.<nl>
    //Parameterlist:<nl>
    //int nSlave // slave id, default 0<nl> 
    void SetSlave( 
        IN int nSlave )
    { 
        m_nSlave = nSlave; 
    }

    //@cmember Get slave addr of I2C device.<nl>
    int GetSlave()
    { 
        return m_nSlave; 
    }

    //@cmember Set timeout parameter.<nl>
    //Parameterlist:<nl>
    //DWORD dwTimeout // time out value 0 ... 15 (max)
    void SetTimeout( 
        IN DWORD dwTimeout )
    {
        m_pVampIo->MunitGpio.Iic_Timer = dwTimeout;
    }

    //@cmember Get timeout parameter, 0 ... 15 (max).<nl>
    DWORD GetTimeout()
    { 
        return (DWORD)m_pVampIo->MunitGpio.Iic_Timer;
    }

    //@cmember Set I2C clock rate.<nl>
    //Parameterlist:<nl>
    //DWORD dwClockSel // 0=slow, 1=fast
    void SetClockSel( 
        IN DWORD dwClockSel )
    {
        m_pVampIo->MunitGpio.Iic_ClockSel = dwClockSel;
    }

    //@cmember Get I2C clock rate.<nl>
    DWORD GetClockSel()
    {
        return (DWORD)m_pVampIo->MunitGpio.Iic_ClockSel;
    }


    //@cmember Write sequence to slave.<nl>
    //Parameterlist:<nl>
    //BYTE ucSubaddress //subaddress (register)<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //int nNumberOfBytes //number of data bytes to send<nl>
    eI2cStatus WriteSequence(
        IN BYTE ucSubaddress,
        IN BYTE *pucSequence,
        IN int nNumberOfBytes );

    //@cmember Read sequence from slave.<nl>
    //Parameterlist:<nl>
    //BYTE ucSubaddress //subaddress (register)<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //int nNumberOfBytes //number of data bytes to receive<nl>
    eI2cStatus ReadSequence(
        IN BYTE ucSubaddress,
        OUT BYTE *pucSequence,
        IN int nNumberOfBytes );

    //@cmember Reset I2C bus from error condition. Returns TRUE if successful.<nl>
    BOOL I2cErrorReset();

    //@cmember Checks whether the returned status is an error status.<nl>
    //Parameterlist:<nl>
    //eI2cStatus eStatus // I2C status to echo<nl>
    BOOL IsStatusError(
        eI2cStatus eStatus ) 
    {
        return ( eStatus >= NO_DEVICE) ? TRUE : FALSE;
    }

    //@cmember Get I2C status.<nl>
    eI2cStatus GetI2cStatus()
    {
        return (eI2cStatus)((DWORD)m_pVampIo->MunitGpio.Iic_Status);
    }


    //@cmember Read nNumberOfBytes bytes from I2C.<nl>
    //Parameterlist:<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //int nNumberOfBytes //number of data bytes to receive<nl>
    eI2cStatus ReadSeq(
        OUT BYTE *pucSequence,   
        IN int nNumberOfBytes ); 

    //@cmember Read 1 byte from I2C.<nl>
    //Parameterlist:<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //eI2cAttr eAttr // transfer attribute<nl>
    eI2cStatus ReadByte(
        OUT BYTE *pucSequence,   
        IN eI2cAttr eAttr );

    //@cmember Write nNumberOfBytes bytes to I2C.<nl>
    //Parameterlist:<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //int nNumberOfBytes //number of data bytes to send<nl>
    eI2cStatus WriteSeq(
        IN BYTE *pucSequence,   
        IN int nNumberOfBytes );

    //@cmember Write 1 byte to I2C.<nl>
    //Parameterlist:<nl>
    //BYTE *pucSequence //pointer to data sequence<nl>
    //eI2cAttr eAttr // transfer attribute<nl>
    eI2cStatus WriteByte(
        IN BYTE *pucSequence,   
        IN eI2cAttr eAttr );

    //@cmember Write nNumberOfBytesWr to I2C, then read nNumberOfBytesRd.<nl>
    //Parameterlist:<nl>
    //BYTE *pucSequenceWr //pointer to write data sequence<nl>
    //int nNumberOfBytesWr //number of data bytes to send<nl>
    //BYTE *pucSequenceRd //pointer to read data sequence<nl>
    //int nNumberOfBytesRd //number of data bytes to read<nl>
    eI2cStatus CombinedSeq(
        IN BYTE *pucSequenceWr, 
        IN int nNumberOfBytesWr, 
        OUT BYTE *pucSequenceRd, 
        IN int nNumberOfBytesRd );


#if 0
    // get I2C transfer attribute<nl>
    eI2cAttr GetAttr()
    {
        return (eI2cAttr)((DWORD)m_pVampIo->MunitGpio.Iic_Attr);
    }

    // set transfer byte<nl>
    void SetTransferByte( 
        IN BYTE ucByte )
    {
        m_pVampIo->MunitGpio.Iic_ByteReadWrite = (DWORD)ucByte;
    }
#endif
};

#endif // !defined(AFX_VAMPI2C_H__83DC1AA0_6159_11D3_A66F_00E01898355B__INCLUDED_)
