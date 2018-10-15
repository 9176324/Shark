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
// @module   VampDmaChannel | This module is used to set the DMA register sets.
// @end
// Filename: VampDmaChannel.h
// 
// Routines/Functions:
//
//  public:
//          CVampDmaChannel
//          ~CVampDmaChannel
//          SetEvenAddress
//          SetOddAddress
//          SetPitch
//          SetBurstLength
//          EnableMmu
//          SetPageTableAddress
//          SetByteSwapping
//          GetObjectStatus
//          
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPDMACHANNEL_H__0A8E8EE1_8C55_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPDMACHANNEL_H__0A8E8EE1_8C55_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampIo.h"
#include "VampTypes.h"
#include "VampBuffer.h"


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampDmaChannel | DMA channel (register set) access methods.
// @end 
//
//////////////////////////////////////////////////////////////////////////////

#undef TYPEID
#define TYPEID CVAMPDMACHANNEL

class CVampDmaChannel  
{
    //@access private
private:
    //@cmember Pointer on DMA register set object
    CDmaRegSet *m_pRegSet; 

    //@cmember Pointer on secondary DMA register set object 
    //(eg. for VBI parallel programming)
    CDmaRegSet *m_pRegSetSec;

    //@cmember Pointer on CVampIo object
    CVampIo *m_pIO;

    //@member. pointer to device resources
    tPageAddressInfo *m_p32BitPagePool;

	//@cmember Offset of the buffer's entry point (necessary for planar streams)<nl>
    DWORD m_dwBufferOffset;     
    
	//@cmember Pointer to 8Kb buffer which contains the MMU page table<nl>
    PDWORD m_pdwMmuLinearAddress;

	//@cmember Aligned pointer to a page which contains the Odd part of the MMU 
    //page table<nl>
    PDWORD m_pdwMmuLinearAddressOdd;

	//@cmember Aligned pointer to a page which contains the Even part of the MMU 
    //page table<nl>
    PDWORD m_pdwMmuLinearAddressEven;

    //@cmember Physical MMU page table address
    DWORD m_dwMmuPhysicalAddress;

    //@cmember Memory management object
	COSDependMemory *m_pMemObjMmu; 

    //@cmember Handle to MMU buffer
    PVOID m_pMmuHandle;

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    //@cmember Line pitch must be multiplied by this factor
    tFraction m_tPitchFactor;

    //@cmember Factor necessary for U and V parts of interlaced planar streams. 
    //Offsets of U/V even base addresses will be increased by (interlaced) pitch/2 
    //multiplied by this factor, which is different for each planar format.
    DWORD m_dwPlanarPitchFactor;

    //@cmember Allocates a MMU page table.
    VAMPRET AllocatePT();

    //@cmember Copy odd or even page table to static DMA channel page table.<nl>
    //Parameterlist:<nl>
    //PDWORD pdwSource // source address<nl>
    //PDWORD pdwDestination // destination address<nl>
    //DWORD dwNumber // number of DWORDs<nl>
    void CopyPageTable( 
        IN PDWORD pdwSource, 
        IN PDWORD pdwDestination, 
        IN DWORD dwNumber )
    {
        for ( DWORD i = 0; i < dwNumber; i++ ) {

            *pdwDestination++ = *pdwSource++;
        }
    }

    //@access public
public:
    //@cmember Primary DMA channel number 0...6
    DWORD m_dwChannel;
    //@cmember Secondary DMA channel number 0...6
    DWORD m_dwChannel2;
    //@cmember Flag for parallel programming of two DMA register sets
    BOOL m_bDual;
    //@cmember Flags for DMA status request of current channel(s)
    DWORD m_dwStatusChannel;

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampIo *pIO // pointer on CVampIo object<nl>
	CVampDmaChannel(
        IN CVampIo *pIO , IN tPageAddressInfo *p32BitPagePool);

    //@cmember Destructor.<nl>
	virtual ~CVampDmaChannel();

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Assignment of a primary and secondary DMA channel.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // primary DMA channel number <nl>
    //DWORD dwChannel2 // secondary DMA channel number <nl>
    void SetChannel( 
        IN DWORD dwChannel, 
        IN DWORD dwChannel2 );

    //@cmember Assignment of a DMA channel.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // DMA channel number <nl>
    void SetChannel( 
        IN DWORD dwChannel );

    //@cmember This method sets the current buffer entry offset. In case of a
    //planar stream we have one buffer for Y, U and V data. So we have to
    //set the start of the different data areas inside the buffer.<nl>
    //Parameterlist:<nl>
    //DWORD dwBufferOffset // offset of buffer entry point<nl>
    VOID SetBufferOffset( 
        IN DWORD dwBufferOffset )
    {
        m_dwBufferOffset = dwBufferOffset;
    }

    //@cmember Set even buffer address plus pitch.<nl>
    //Parameterlist:<nl>
    //IN CVampBuffer *pBuffer // even DMA buffer pointer<nl>
    //IN LONG lPitch // pitch to add for interlaced even field<nl>
    void SetEvenAddress( 
        IN CVampBuffer *pBuffer,
        IN LONG lPitch );

    //@cmember Set odd buffer address.<nl>
    //Parameterlist:<nl>
    //IN CVampBuffer *pBuffer // odd DMA buffer pointer<nl>
    void SetOddAddress( 
        IN CVampBuffer *pBuffer );

    //@cmember Set pitch factor.<nl>
    //Parameterlist:<nl>
    //tFraction tPitchFactor // pitch factor of buffer<nl>
    //DWORD dwPlanarPitchFactor // planar pitch factor<nl>
    void SetPitchFactor( 
        IN tFraction tPitchFactor,
        IN DWORD dwPlanarPitchFactor )
    {
        m_tPitchFactor = tPitchFactor;
        m_dwPlanarPitchFactor = dwPlanarPitchFactor;
    }
       
    //@cmember Set line pitch.<nl>
    //Parameterlist:<nl>
    //LONG lPitch // pitch of buffer<nl>
    void SetPitch( 
        IN LONG lPitch )
    {
        lPitch *= m_tPitchFactor.f.usNumerator;
        lPitch /= m_tPitchFactor.f.usDenominator;
        m_pRegSet->Pitch = ( m_bDual ? 
            ( m_pRegSetSec->Pitch = lPitch ) : lPitch );
           DBGPRINT((1,"SetPitch: %ld\n", lPitch));

    }

    //@cmember Set burst length.<nl>
    //Parameterlist:<nl>
    //DWORD dwBurst // burst length value<nl>
    void SetBurstLength( 
        IN DWORD dwBurst )
    {
        m_pRegSet->Burst = ( m_bDual ? 
            ( m_pRegSetSec->Burst = dwBurst ) : dwBurst );
    }

    //@cmember Switch MMU On or Off.<nl>
    //Parameterlist:<nl>
    //BOOL bMmuEnable // enable switch<nl>
    void EnableMmu( 
        IN BOOL bMmuEnable )
    {
        m_pRegSet->MapEn = ( bMmuEnable ? 1 : 0 );
        if ( m_bDual )
            m_pRegSetSec->MapEn = ( bMmuEnable ? 1 : 0 );
    }

    //@cmember Set the MMU page table address.<nl>
    //Parameterlist:<nl>
    //DWORD dwPTA // physical address of the MMU page table<nl>
    void SetPageTableAddress( 
        IN DWORD dwPta )
    {
        m_pRegSet->PTAdr = ( m_bDual ? 
            ( m_pRegSetSec->PTAdr = dwPta ) : dwPta );
    }

    //@cmember Set byte swapping during DMA transfer.<nl>
    //Parameterlist:<nl>
    //eByteOrder eSwap // byte order within a data DWORD<nl>
    void SetByteSwapping( 
        IN eByteOrder eSwap )
    {
        m_pRegSet->Swap = ( m_bDual ? 
            ( m_pRegSetSec->Swap = eSwap ) : eSwap );
    }

    //@cmember Set complete DMA register set according to buffer parameters.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // buffer object<nl>
    VAMPRET SetDmaRegset( 
        CVampBuffer *pBuffer );

    //@cmember Check if the channel is a VBI channel.<nl>
    BOOL IsVbiChannel()
    {
        return (( m_dwChannel == 2 ) || ( m_dwChannel == 3 )) ? TRUE : FALSE;
    }
};

#endif // !defined(AFX_VAMPDMACHANNEL_H__0A8E8EE1_8C55_11D3_A66F_00E01898355B__INCLUDED_)
