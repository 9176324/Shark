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
// @module   RegArray | Definition of the data type 'CRegArray', which
//           encapsulates an array of  memory mapped 32-bit registers. 
// @end
//
// Filename: RegArray.h
//
// Routines/Functions:
//
//  public:
//          CRegArray
//          ~CRegArray
//          operator[]
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_REGARRAY_H__8BC95780_8537_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_REGARRAY_H__8BC95780_8537_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Register.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CRegArray | The class defines an array of contigous memory mapped 
// 32-bit registers. 
// @base public | CRegister  
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CRegArray : public CRegister  
{
    // @access private 
private:
    // @cmember Size of the array.
    DWORD m_dwSize;

    // @access public 
public:
    // @cmember | CRegArray | ( IN PBYTE pBaseAddress, IN DWORD dwOffset, IN RegType eType,
    // IN DWORD dwSize) | Constructor of an uninitialized 32-bit register array.<nl>
    // Parameterlist:<nl>
    // PBYTE pBaseAddress // Base address of MMIO space<nl>
    // DWORD dwOffset // Offset to this register array<nl>
    // RegType eType // Access<nl>
    // DWORD dwSize // Size of register array 
	CRegArray( 
            IN PBYTE pBaseAddress,  
            IN DWORD dwOffset,      
            IN RegType eType,       
            IN DWORD dwSize ) :     
        CRegister( pBaseAddress, dwOffset, eType ), 
        m_dwSize ( dwSize )
    {}

    // @cmember Destructor.
	virtual ~CRegArray()
    {}

    // @cmember Performs an assignment of an register array variable to a 32-bit
    // register, useful in combination with the overloaded operators of the
    // CRegister class to set or get the register value, 
    // eg. <nl> 
    // CRegArray Rega( 0x200, RW, 10 ); <nl>
    // CRegister Reg( 0x204, RW ); <nl>
    // Rega[2] = 0x456; <nl>     
    //      will set the register at offset 0x208 immediately to 0x456  <nl>
    // Reg = Rega[2]; <nl>        
    //      will set the register at offset 0x204 immediately to 0x456 <nl>
    // DWORD dwval = (DWORD) Rega[2];  <nl>  
    //      will set 'dwval' to 0x456 <nl>
    // Parameterlist:<nl>
    // DWORD dwIndex // array index<nl>

    CRegister operator[]( 
        IN DWORD dwIndex )
    {
        CRegister Reg( m_pBaseAddress, m_dwOffset + ( dwIndex << 2 ), m_eType );
        return Reg;
    }

    // @cmember Returns the size of the register array.
    DWORD GetSizeOfRegSpace () 
    {
        return m_dwSize;
    }
};

#endif // !defined(AFX_REGARRAY_H__8BC95780_8537_11D3_A66F_00E01898355B__INCLUDED_)
