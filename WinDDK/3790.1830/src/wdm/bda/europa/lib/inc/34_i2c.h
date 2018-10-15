//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
// @module   I2C | Interface of CVampI2c, an I2C driver class, using
//           SAA7134 as master controller. I2C driver class to control all the
//           I2C devices...

// 
// Filename: 34_I2C.h
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
//          WriteSequence
//          ReadSequence
//          WriteSeq
//          WriteByte
//          ReadSeq
//          ReadByte
//          CombinedSeq
//          
//  private:
//
//  protected:
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _I2C__
#define _I2C__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampI2c | Several methods for controlling the GPIO I2C master.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampI2c  
{
//@access Public 
public:
    //@access Public functions 
    //@cmember Constructor<nl>
    //Parameterlist<nl>
    //DWORD dwDeviceIndex // Device Index<nl>
    //int nSlave // Slave id default to zero<nl> 
    CVampI2c( 
	IN DWORD dwDeviceIndex, 
        IN INT nSlave=0);
    //@cmember Destructor<nl>
	virtual ~CVampI2c();
    //@cmember Set slave addr of I2C device<nl>
    //Parameterlist<nl>
    //int nSlave    // Slave id default to zero<nl> 
    VOID SetSlave( 
        IN INT nSlave );
    //@cmember Get slave id of I2C device<nl>
    INT GetSlave();
    //@cmember Set timeout parameter<nl>
    //Parameterlist<nl>
    //DWORD dwTimeout  // Timeout value 0 ... 15 (max)<nl>
    VOID SetTimeout( 
        IN DWORD dwTimeout );
    //@cmember Get timeout parameter  0 ... 15 (max)<nl>
    DWORD GetTimeout();
    //@cmember Set I2C clock rate<nl>
    //Parameterlist<nl>
    //DWORD dwClockSel )    // 0=slow, 1=fast<nl>
    VOID SetClockSel( 
        IN DWORD dwClockSel );
    //@cmember Get I2C clock rate<nl>
    DWORD GetClockSel();
    //@cmember Write sequence to slave<nl>
    //Parameterlist<nl>
    //BYTE ucSubaddress  // Subaddress (register)<nl>
    //BYTE* pucSequenceWr  // Pointer to data sequence<nl>
    //INT nNumberOfBytesWr // Number of data bytes to send<nl>
    eI2cStatus WriteSequence(
        IN BYTE ucSubaddress,
        IN BYTE* pucSequenceWr,
        IN INT nNumberOfBytesWr );
    //@cmember Read sequence from slave<nl>
    //Parameterlist<nl>
    //BYTE ucSubaddress  // Subaddress (register)<nl>
    //BYTE* pucSequenceRd  // Pointer to data sequence<nl>
    //INT nNumberOfBytesRd // Number of data bytes to receive<nl>
    eI2cStatus ReadSequence(
        IN BYTE ucSubaddress,
        IN OUT BYTE* pucSequenceRd,
        IN INT nNumberOfBytesRd );
    //@cmember Read number of bytes from slave<nl>
    //Parameterlist<nl>
    //BYTE* pucSequenceRd  // Pointer to data sequence<nl>
    //INT nNumberOfBytesRd // Number of data bytes to receive<nl>
    eI2cStatus ReadSeq(
        IN OUT BYTE* pucSequenceRd,
        IN INT nNumberOfBytesRd );
    //@cmember Read one byte from slave<nl>
    //Parameterlist<nl>
    //BYTE* pucSequenceRd  // Pointer to data sequence<nl>
    //eI2cAttr eAttr // Attribute<nl>
    eI2cStatus ReadByte(
        IN OUT BYTE* pucSequenceRd,
        IN eI2cAttr eAttr );
    //@cmember Write number of bytes to slave<nl>
    //Parameterlist<nl>
    //BYTE* pucSequenceWr  // Pointer to data sequence<nl>
    //INT nNumberOfBytesWr // Number of data bytes to send<nl>
    eI2cStatus WriteSeq(
        IN OUT BYTE* pucSequenceWr,
        IN INT nNumberOfBytesWr );
    //@cmember Write one byte to slave<nl>
    //Parameterlist<nl>
    //BYTE* pucSequenceWr // Pointer to data sequence<nl>
    //eI2cAttr eAttr // Attribute<nl>
    eI2cStatus WriteByte(
        IN OUT BYTE* pucSequenceWr,
        IN eI2cAttr eAttr );
    //@cmember Read number of bytes from slave after <nl>
    //Write number of bytes to slave<nl>
    //Parameterlist<nl>
    //BYTE* pucSequenceWr // Pointer to data sequence<nl>
    //INT nNumberOfBytesWr // Number of data bytes to send<nl>
    //BYTE* pucSequenceRd  // Pointer to data sequence<nl>
    //INT nNumberOfBytesRd // Number of data bytes to receive<nl>
    eI2cStatus CombinedSeq(
        IN BYTE* pucSequenceWr, 
        IN INT nNumberOfBytesWr,
        IN OUT BYTE* pucSequenceRd, 
        IN INT nNumberOfBytesRd );
//@access Private 
private:
    //@access Private variables 
    //@cmember Slave address of device<nl>
    INT m_nSlave;
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
};

#endif // _I2C__
