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
// @module   Register | Definition of the data type 'CRegister'. This class makes 
//  it easy to access the memory mapped 32-bit registers of the device.
// @end
// Filename: Register.h
// 
// Routines/Functions:
//
//  public:
//          CRegister
//          ~CRegister
//          GetOffset
//          GetAddress
//          GetType
//          GetShadow
//          operator DWORD
//          operator=
//          
//  private:
//
//  protected:
//          SetShadow
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_REGISTER_H__2F00F541_45B7_11D3_8013_00E01898355B__INCLUDED_)
#define AFX_REGISTER_H__2F00F541_45B7_11D3_8013_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OSDepend.h"

typedef enum { RW, RO, WO } RegType;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CRegister | This class defines a complete memory mapped 32-bit register.   
// @base public | COSDependRegisterAccess  
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CRegister : public COSDependRegisterAccess  
{

private:

protected:
//@access protected
    //@cmember Holds the base address of the register.
    PBYTE m_pBaseAddress;
    //@cmember Holds the shadow value of the register.
    DWORD m_dwShadow;

    // @cmember Keep a shadow of the register value, useful for write-only registers 
    // which can't be read.<nl>
    // Parameterlist:<nl>
    // DWORD dwValue // register value<nl>
    void SetShadow( 
        IN DWORD dwValue ) 
    { 
        m_dwShadow = dwValue; 
    }

public:
//@access public
    //@cmember Holds the type of the register (RW = read/write,
    // RO = read-only, WO = write-only). 
    RegType m_eType;

    //@cmember Holds the register's offset of the base address.
    DWORD m_dwOffset;

    //@cmember | CRegister | ( IN PBYTE pBaseAddress, IN DWORD dwOffset,IN RegType eType )|
    // Constructor of an uninitialized 32-bit register.<nl>
    // Parameterlist:<nl>
    // PBYTE pBaseAddress // base address of the memory space<nl>
    // DWORD dwOffset // register offset from base address<nl>
    // RegType eType // register type<nl>
    CRegister( 
            IN PBYTE pBaseAddress, 
            IN DWORD dwOffset, 
            IN RegType eType ) :
        m_pBaseAddress( pBaseAddress ),
        m_dwOffset( dwOffset ),
        m_eType( eType ),
        m_dwShadow ( 0 ) 
    {}

    //@cmember | CRegister | ( IN PBYTE pBaseAddress, IN DWORD dwOffset,IN RegType eType, IN DWORD dwInitValue )|
    // Constructor of an 32-bit register with initialization.<nl>
    // Parameterlist:<nl>
    // PBYTE pBaseAddress // base address of the memory space<nl>
    // DWORD dwOffset // register offset from base address<nl>
    // RegType eType // register type<nl>
    // DWORD dwInitValue // initialization value<nl>
    CRegister( 
            IN PBYTE pBaseAddress, 
            IN DWORD dwOffset, 
            IN RegType eType, 
            IN DWORD dwInitValue ) :
        m_pBaseAddress( pBaseAddress ),
        m_dwOffset( dwOffset ),
        m_eType( eType ),
        m_dwShadow ( dwInitValue ) 
    {
        *this = dwInitValue;
    }

    //@cmember  Destructor.
    virtual ~CRegister()
    {}

    //@cmember  Returns the offset of the register.
    DWORD GetOffset() 
    { 
        return ( m_dwOffset ); 
    }

    //@cmember  Returns the virtual base address of the register.
    PBYTE GetAddress() 
    { 
        return (PBYTE)( m_pBaseAddress + m_dwOffset ); 
    }

    //@cmember  Returns the type of the register (eg. read/write, read-only, write-only).
    RegType GetType() 
    { 
        return m_eType; 
    }

    //@cmember  Returns the shadow value of the register.
    DWORD GetShadow() 
    { 
        return m_dwShadow; 
    }

    //@cmember  Performs a read operation on the 32-bit register.
    operator DWORD();

    //@cmember  Performs a write operation on the 32-bit register.<nl>
    // Parameterlist:<nl>
    // DWORD dwValue // value to assign<nl>
    DWORD operator=( 
        IN DWORD dwValue );

    //@cmember  Performs a immediate write operation on the 32-bit register.<nl>
    // Parameterlist:<nl>
    // DWORD dwValue // value to assign immediately<nl>
    DWORD operator^=( 
        IN DWORD dwValue );
};

#endif // !defined(AFX_REGISTER_H__2F00F541_45B7_11D3_8013_00E01898355B__INCLUDED_)
