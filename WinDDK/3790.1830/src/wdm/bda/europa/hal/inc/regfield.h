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
// @module   Regfield | Definition of the data type 'CRegfield', which
//           encapsulates a bitfield of a 32-bit register.
// @end 
// Filename: Regfield.h
//
// Routines/Functions:
//
//  public:
//          CRegField
//          ~CRegField
//          operator DWORD
//          operator=
//
//  private:
//          MakeAMask
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_REGFIELD_H__4AF2D820_45E6_11D3_8013_00E01898355B__INCLUDED_)
#define AFX_REGFIELD_H__4AF2D820_45E6_11D3_8013_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Register.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CRegField | This class defines the data type of a register field,
//         which is a set of contiguous bits within a 32-bit register. 
// @base public | CRegister
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CRegField : public CRegister
{
private:
//@access private 
    //@cmember For internal use only. Mask out the register bits which are not concerned.
    DWORD MakeAMask();

public:
//@access public 
    //@cmember Start position of bit field.
    BYTE m_ucStartBit;
    //@cmember Width of bit field.
    BYTE m_ucFieldWidth;

    // @cmember | CRegField |( IN PBYTE pBaseAddress,IN DWORD dwOffset,IN RegType eType, 
    // BYTE ucStart,IN BYTE ucWidth ) | Constructor of an uninitialized register field, 
    // defining the position in the 32-bit register.<nl>
    // Parameterlist:<nl>
    // PBYTE pBaseAddress // Base address of MMIO space<nl>
    // DWORD dwOffset // Offset to this register<nl>
    // RegType eType // Access<nl> 
    // BYTE ucStart // Start of bit field<nl> 
    // BYTE ucWidth // Size of bit field<nl>   
    CRegField(
            IN PBYTE pBaseAddress, 
            IN DWORD dwOffset, 
            IN RegType eType, 
            IN BYTE ucStart, 
            IN BYTE ucWidth ) :
        CRegister( pBaseAddress, dwOffset, eType ),
        m_ucStartBit( ucStart ),
        m_ucFieldWidth( ucWidth )
    {}

    // @cmember | CRegField |( IN PBYTE pBaseAddress,IN DWORD dwOffset,IN RegType eType, 
    // BYTE ucStart,IN BYTE ucWidth ) | Constructor of an register field with 
	// initialization, defining the position in the 32-bit register.<nl>
    // Parameterlist:<nl>
    // PBYTE pBaseAddress // Base address of MMIO space<nl>
    // DWORD dwOffset // Offset to this register<nl>
    // RegType eType  // Access<nl> 
    // BYTE ucStart // Start of bit field<nl> 
    // BYTE ucWidth // Size of bit field<nl>  
    // DWORD dwInitValue //it value
    CRegField( 
            PBYTE pBaseAddress, 
            DWORD dwOffset, 
            RegType eType, 
            BYTE ucStart, 
            BYTE ucWidth, 
            DWORD dwInitValue ) :
        CRegister( pBaseAddress, dwOffset, eType ),
        m_ucStartBit( ucStart ),
        m_ucFieldWidth( ucWidth )
    {
        *this = dwInitValue;    
    }

    // @cmember Destructor.
    virtual ~CRegField() {}

    // @cmember Performs a read operation on the register field.
    operator DWORD();

    // @cmember Performs a write operation on the register field.
    DWORD operator=( 
        DWORD dwValue );

    // @cmember Performs a immediate write operation on the register field.
    DWORD operator^=( 
        DWORD dwValue );
};




#endif // !defined(AFX_REGFIELD_H__4AF2D820_45E6_11D3_8013_00E01898355B__INCLUDED_)

