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
// @module   CVampFifoAlloc  | This module handles the partitioning of the FIFOs.<nl>
// @end
// Filename: VampFifoAlloc.h
// 
// Routines/Functions:
//
//  public:
//	    CVampFifoAlloc
//	    ~CVampFifoAlloc();
//      GetObjectStatus
//  protected:
//      SetFifoRamArea
//      GetFifoRamArea
//
//  private:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPFIFOALLOC_H__51D3B4E0_A8CC_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPFIFOALLOC_H__51D3B4E0_A8CC_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampIo.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampFifoAlloc | Methods for the partitioning of the FIFO.
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampFifoAlloc  
{
//@access private 
private:
    //@cmember Pointer on CVampIo object
    CVampIo *m_pVampIo;
    //@cmember Size of RAM area one
    DWORD m_dwEra1Size;
    //@cmember Size of RAM area two
    DWORD m_dwEra2Size;
    //@cmember Size of RAM area three
    DWORD m_dwEra3Size;
    //@cmember Size of RAM area four
    DWORD m_dwEra4Size;
    //@cmember Combined size of RAM area one to three
    DWORD m_dwEra123Size;

	//@cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

//@access protected 
protected:

    //@cmember Sets the size of the RAM areas 1-3. The 4th will get 
    //the rest of the available space.<nl>
    //Parameterlist:<nl>
    //DWORD dwEra1Size // size of RA one<nl>
    //DWORD dwEra2Size // size of RA two<nl>
    //DWORD dwEra3Size // size of RA three<nl>
    VAMPRET SetFifoRamArea(
        IN DWORD dwEra1Size,
        IN DWORD dwEra2Size,
        IN DWORD dwEra3Size );

    //@cmember Returns the size of the requested RAM area.<nl> 
    //Parameterlist:<nl>
    //eFifoArea nFifoArea // requested RA
    DWORD GetFifoRamArea(
        IN eFifoArea nFifoArea );

    //@cmember Sets the threshold of the RAM area(s).<nl>
    //Parameterlist:<nl>
    //eFifoArea nFifoArea // requested RA<nl>
    //DWORD dwThreshold // threshold value (0-3)<nl>
    VAMPRET SetFifoThreshold(
        IN eFifoArea nFifoArea,
        IN DWORD dwThreshold );

    //@cmember Request the threshold of a ram area.<nl> 
    //Parameterlist:<nl>
    //eFifoArea nFifoArea // requested RA<nl>
    //DWORD *dwThreshold // returned value of requested threshold<nl>
    VAMPRET GetFifoThreshold(
        IN eFifoArea nFifoArea,
        OUT DWORD *dwThreshold );

//@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampIo *pIO // pointer on CVampIo object<nl>
	CVampFifoAlloc(
        IN CVampIo *pIO );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
	virtual ~CVampFifoAlloc();

};

#endif // !defined(AFX_VAMPFIFOALLOC_H__51D3B4E0_A8CC_11D3_A66F_00E01898355B__INCLUDED_)
