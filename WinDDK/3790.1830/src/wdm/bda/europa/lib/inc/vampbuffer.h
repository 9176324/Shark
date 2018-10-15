///////////////////////////////////////////////////////////////////////////////
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
// @module   VampBuffer | Buffer handling for video, VBI, transport and audio
//           streams. The buffer objects will be created by the driver and
//           passed to the HAL.
// @end
// Filename: VampBuffer.h
// 
// Routines/Functions:
//
//  public:<nl>
//	CVampBuffer
//  ~CVampBuffer
//  GetMMUTable
//  AllocMemory
//  GetMMUMemoryAddress
//  GetMemoryAddress
//  GetPitch
//  SetFormat
//  GetFormat
//  SetFieldFormat
//  GetFieldFormat
//  SetDroppedFrames
//  GetDroppedFrames
//  SetLostSamples
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
//
// VampBuffer.h: interface for the CVampBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VAMPBUFFER_H__7EB5B3A0_7CAD_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPBUFFER_H__7EB5B3A0_7CAD_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OSDepend.h"
#include "VampError.h"
#include "VampTypes.h"
#include "VampList.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampBuffer |  This class is a repository of information about a 
//         single unit of transfer. A buffer object can be initialized to use
//         a  previously allocated buffer. To allow the core stream class to
//         correctly  process buffer completions, the buffer will maintain a
//         property describing its type.<nl>
//
//@base  public | CVampListEntry
// @end 
// 
//
//////////////////////////////////////////////////////////////////////////////

class CVampBuffer : public CVampListEntry
{
    //@access private 
private:
    //@cmember Memory management object (buffer)
	COSDependMemory *m_pMemObjBuffer;
    //@cmember Linear overflow buffer address
    PVOID m_pOverflowMemoryAddress;
    //@cmember Handle to DMA buffer memory
    PVOID m_pDmaMemoryHandle;
    //@cmember Physical buffer address
    DWORD m_dwDmaPhysicalAddress;
    //@cmember Linear MMU table address
    PDWORD m_pdwMmuLinearAddress;
    //@cmember Number of MMU table entries
    DWORD m_dwMmuNumberOfEntries;
    //@member unaligned MMU buffer
    PVOID m_pMmuAllocation;
    //@cmember MMU enabled flag
    BOOL  m_bMmuEnabled;
    //@cmember Size of the buffer
    size_t m_BufferSize;
    //@cmember Pointer on a driver / user context
    PVOID m_pDeviceContext;
    //@member pointer to MDL
    PVOID m_pMapping;
    //@cmember Dropped frames counter regarding the actual buffer
    DWORD m_dwDroppedFrames;

    //@cmember Link to planar Y buffer object
    CVampBuffer *m_pBufferY;
    //@cmember Link to planar U buffer object
    CVampBuffer *m_pBufferU;
    //@cmember Link to planar V buffer object
    CVampBuffer *m_pBufferV;

    //@cmember Buffer format (pitch, size etc.)
    tBufferFormat m_tBufferFormat;

    //@cmember Buffer status
    tBufferStatus m_tBufferStatus;

    //@cmember TRUE, if linear address is a user-mode address
    BOOL m_bUserMode;

	// @cmember Error status flag, will be set during construction.
	DWORD m_dwObjectStatus;

    // @cmember TRUE, if overflow buffer
    BOOL m_bOverflow;

    // @cmember Offset from aligned start (beginning of page) to actual start
    // of buffer
    DWORD m_dwAlignmentOffset;

//friends to this are
friend class CVampCoreStream;
friend class CVampBaseCoreStream;
friend class CVampPlanarCoreStream;
friend class CVampVBasicStream;
friend class CVampDmaChannel;

    // @cmember Sets overflow status.<nl>
    //Parameterlist:<nl>
    //BOOL bOverflow // overflow buffer flag<nl>
    VOID SetOverflowMode( 
        BOOL bOverflow ) 
    {
	    m_bOverflow = bOverflow;
    }

    //@cmember Returns the number of MMU table entries.<nl>
    DWORD GetMmuNumberOfEntries()  
    {
        return m_dwMmuNumberOfEntries;
    }

    //@cmember Returns the physical DMA memory address.<nl>
    DWORD GetDmaPhysicalAddress()
    {
        return (DWORD)m_dwDmaPhysicalAddress;
    }

    //@cmember Returns distance between lines (pitch) in bytes.<nl>
    LONG GetPitch()
    {
        return m_tBufferFormat.lPitchInBytes;
    }

    //@cmember Returns the status of MMU enable bit.<nl>
    BOOL IsMmuEnabled()
    {
        return m_bMmuEnabled;
    }

    //@cmember Sets the number of lost buffers.<nl>
    //Parameterlist:<nl>
    //DWORD dwLostBuffer // lost buffer count<nl>
    VOID SetLostSamples(
        DWORD dwLostBuffer )
    {
        m_tBufferStatus.dwLostBuffer = dwLostBuffer;
    }

    //@access public 
public:

    //@cmember Constructor, which gets a buffer area.<nl>
    //Parameterlist:<nl>
    //PVOID pBufferAddress // pointer to transmitted buffer<nl>
    //size_t NrOfBytes // size of buffer in bytes<nl>
    //tBufferFormat nBufferFormat // buffer format header<nl>
    //BOOL bContiguousMemoryBuffer // contiguous buffer flag<nl>
    CVampBuffer (
        PVOID pBufferAddress,
        size_t NrOfBytes,
        PVOID pMapping,
        tBufferFormat* pBufferFormat,
        UCHAR bBufferMappingInfo = BUFFER_REGULAR_MAPPING_OS,
        BOOL bContiguousMemoryBuffer = false,
        BOOL bUserMode = false );

    //@cmember Constructor for preallocated buffer (used by WDM).<nl>
    CVampBuffer ();

    //@cmember Destructor.<nl>
	~CVampBuffer();

    //@cmember Uninitialize preallocated buffer (used by WDM).<nl>
    void UnlockBuffer();

    //@cmember Sets pointer to Y buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferY // planar Y buffer object<nl>
    VOID SetBufferY(
        CVampBuffer *pBufferY ) 
    {
        m_pBufferY = pBufferY;
    }

    //@cmember Sets pointer to U buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferU // planar U buffer<nl>
    VOID SetBufferU(
        CVampBuffer *pBufferU ) 
    {
        m_pBufferU = pBufferU;
    }

    //@cmember Sets pointer to V buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferV // planar V buffer<nl>
    VOID SetBufferV(
        CVampBuffer *pBufferV ) 
    {
        m_pBufferV = pBufferV;
    }

    //@cmember Gets pointer to Y buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferY // planar Y buffer object<nl>
    CVampBuffer *GetBufferY()
    {
        return m_pBufferY;
    }

    //@cmember Gets pointer to U buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferU // planar U buffer<nl>
    CVampBuffer *GetBufferU()
    {
        return m_pBufferU;
    }

    //@cmember Gets pointer to V buffer object.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBufferV // planar V buffer<nl>
    CVampBuffer *GetBufferV()
    {
        return m_pBufferV;
    }

    //@cmember Sets pointer on a driver / user context.<nl>
    //Parameterlist:<nl>
    //PVOID pDevContext // driver / user context<nl>
    VOID SetContext(
        PVOID pDevContext ) 
    {
        m_pDeviceContext = pDevContext;
    }


    //@cmember Returns a pointer on a driver / user context.<nl>
    PVOID GetContext() 
    {
        return (m_pDeviceContext);
    }

    //@cmember Gets the buffer status attributes.<nl>
    tBufferStatus GetStatus()
    {
        return m_tBufferStatus;
    }

    //@cmember Gets the buffer format attributes.<nl>
    tBufferFormat GetFormat()
    {
        return m_tBufferFormat;
    }

    //@cmember Returns the time stamp.<nl>
    LONGLONG GetTimeStamp() 
    {
        return m_tBufferStatus.llTimeStamp;
    }

    //@cmember Gets the field type format attribute.<nl>
    eFieldType GetFieldFormat()
    {
        return m_tBufferStatus.nFieldType;
    }

    //@cmember Gets the byte order of a field/frame.<nl>
    eByteOrder GetByteOrder()
    {
        return m_tBufferFormat.ByteOrder;
    }

	//@cmember Sets the byte order of a field/frame.<nl>
    //Parameterlist:<nl>
    //eByteOrder ByteOrder // byte order within a data DWORD<nl>
    VOID SetByteOrder(
        eByteOrder ByteOrder )
    {
        m_tBufferFormat.ByteOrder = ByteOrder;
    }

    //@cmember Returns the buffer size.<nl>
    LONG GetActualBufferSize() 
    {
        return (DWORD)m_tBufferStatus.lActualBufferLength;
    }

    //@cmember Returns he actual status flags.<nl>
    tBufferStatusFlags GetBufferStatusFlags() 
    {
        return m_tBufferStatus.Status;
    }

    //@cmember Sets the time stamp.<nl>
    //Parameterlist:<nl>
    //LONGLONG llTimeStamp // time stamp value<nl>
    VOID SetTimeStamp( 
        LONGLONG llTimeStamp ) 
    {
        m_tBufferStatus.llTimeStamp = llTimeStamp;
    }

    //@cmember Set an actual status flag, but combine it logically with 
    //the current flags<nl>
    //Parameterlist:<nl>
    //tBufferStatus Status // status flag to add<nl>
    VOID SetBufferStatusFlags(
        tBufferStatusFlags Status ) 
    {
        m_tBufferStatus.Status = Status;
    }

    //@cmember Sets the actual buffer size.<nl>
    //Parameterlist:<nl>
    //LONG lActualBufferLength // size of buffer<nl> 
    VOID SetActualBufferSize( 
        LONG lActualBufferLength ) 
    {
        m_tBufferStatus.lActualBufferLength = lActualBufferLength;
    }

    //@cmember Checks, whether the actual time stamp is valid.<nl>
    BOOL IsTimeStampValid() 
    {
        return (BOOL)(( m_tBufferStatus.Status & CVAMP_BUFFER_VALIDTIMESTAMP )
                     && ( m_tBufferStatus.Status & CVAMP_BUFFER_DONE ));
    }

    //@cmember Set the number of dropped frames at the time the actual 
    //buffer is completed.<nl>
    //Parameterlist:<nl>
    //DWORD dwDroppedFrames // number of dropped frames<nl>
    VOID SetDroppedFrames( 
        DWORD dwDroppedFrames )
    {
        m_dwDroppedFrames = dwDroppedFrames;
    }

    //@cmember Get the number of dropped frames at the time the actual buffer is completed.<nl>
    DWORD GetDroppedFrames()
    {
        return m_dwDroppedFrames;
    }

    //@cmember Sets the field type format attribute.<nl>
    //Parameterlist:<nl>
    //eFieldType nFieldType // field type<nl>
    VOID SetFieldFormat(
        eFieldType nFieldType )
    {
        m_tBufferStatus.nFieldType = nFieldType;
    }

    //@cmember Returns the buffer size.<nl>
    DWORD GetBufferSize() 
    {
        return (DWORD)m_BufferSize;
    }

    //@cmember Returns the linear MMU table address.<nl>
    PDWORD GetMmuLinearAddress() 
    {
        return m_pdwMmuLinearAddress;
    }

    // @cmember Returns a linear address that is valid in system mode.<nl>
    PVOID GetSystemAddress();

    //@cmember Sets the buffer type (queued or static) for this buffer. The
    //         first buffer queued sets the core stream format.<nl>
    //Parameterlist:<nl>
    //eBufferType type // buffer type<nl>
    VOID SetBufferType(
        eBufferType type )
    {
        m_tBufferFormat.BufferType = type;
    }

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus ()
	{
		return m_dwObjectStatus;
	}

    VOID AdjustDmaPhysicalAddress(
        LONG lOffset)
    {
        m_dwDmaPhysicalAddress= (m_dwDmaPhysicalAddress + lOffset);
    }

    // @cmember Adjusts the Pitch generally used for planar buffers.<nl>
    //Parameterlist:<nl>
    //tFraction PitchFactor // pitch factor<nl>
    VOID AdjustPitch (
        tFraction PitchFactor )
    {
        m_tBufferFormat.lPitchInBytes *= PitchFactor.f.usNumerator;
        m_tBufferFormat.lPitchInBytes /= PitchFactor.f.usDenominator;
    }

    // @cmember TRUE, if buffer is overflow buffer.
    BOOL IsOverflowBuffer() 
    {
	    return m_bOverflow;
    }

#if DBG
    int m_BufferNumber;
#endif

};

// A interrupt-safe list of buffers

// --- helper functions and templates ----

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class CBaseSyncCall | Manages a callback to a function serialized with the
//        interrupt routine, using the  o/s-dependent sync call object. <nl>
//        This base class performs the conversion from static callback to
//        class member, allowing derived classes to store  additional context
//        information.
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CBaseSyncCall
{	
    //@access private 
private:
    // @cmember Called by the drivers SyncCall method with locked interrupts.
    //          Calls the SyncRoutine method of the adjacent SyncCall class
    //          and gives back its return value.<nl>
    //Parameterlist:<nl>
    //PVOID pContext // context pointer passed by o/s-dependent SyncCall<nl>
    static BOOL DispatchSync( 
        PVOID pContext );

    //@access public 
public:
    // @cmember Calls the o/s-dependent SyncCall method of the object, which
    //          is passed in the paraneter. This method calls back the local
    //          DispatchSync method.<nl>
    //Parameterlist:<nl>
    //COSDependSyncCall* pSyncObj // pointer on o/s-dependent Sync object<nl>
    BOOL SyncCall( 
        COSDependSyncCall* pSyncObj );

    // @cmember Pure definition of the SyncRoutine method.<nl>
    virtual BOOL SyncRoutine() = 0;

};


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class SyncCall1 | Template for a callback object. Specific callback
//        objects can be created for any function with one argument and a
//        return value.
// @base public | CBaseSyncCall
// @end 
//
//////////////////////////////////////////////////////////////////////////////

template <class Arg,    // type of parameter to function
	  class Ret,        // type of return value from function
	  class Caller>		// class of which function is a member
  class CSyncCall1 : public CBaseSyncCall
{
    //@access protected 
protected:
	// @cmember Template parameter: type of parameter to function.
	Arg m_Arg1;
	// @cmember Template parameter: type of return value from function.
	Ret m_Ret;
	// @cmember Template parameter: class of which the function is a member.
	Caller* m_pTarget;

	// @cmember | Ret | (Caller::* m_pfn)(Arg) | Definition of the Sync function.<nl>
	Ret (Caller::* m_pfn)( Arg );

	// @cmember Pointer to o/s-dependent Sync object.
	COSDependSyncCall *m_pSyncObj;

    //@access public 
public:
    // @cmember | CSyncCall1 | (COSDependSyncCall* pSyncObj, Ret (Caller::* pfn)(Arg)) | 
	//Constructor. Set o/s-dependent sync object pointer and function to be called.<nl>
    //Parameterlist:<nl>
    //COSDependSyncCall* pSyncObj // pointer on o/s-dependent Sync object<nl>
    //Ret (Caller::* pfn)(Arg) // function to be called on pTarget<nl>
	CSyncCall1(
	    COSDependSyncCall* pSyncObj,
		Ret (Caller::* pfn)(Arg)) :
	      m_pfn(pfn),
	      m_pTarget(NULL),
	      m_pSyncObj(pSyncObj)
	{
	}

    // @cmember Calls the SyncCall method of the CBaseSyncCall class with the given parameters.<nl>
    //Parameterlist:<nl>
    //Caller* pTarget // pointer to object to be called<nl>
    //Arg arg1 // parameter to callback function<nl>
	Ret Call(
	    Caller* pTarget, 
	    Arg arg1 ) 
    {	
	    m_Arg1 = arg1;
	    m_pTarget = pTarget;
	    SyncCall( m_pSyncObj );
	    return m_Ret;
	}

    // @cmember SyncRoutine to be called interrupt safe by the driver.<nl>
	BOOL SyncRoutine() 
    {
	    if (m_pTarget) 
        {
		    m_Ret = (m_pTarget->*m_pfn)( m_Arg1 );
	    }
	    return TRUE;
	}

};

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class SyncCall0 | Template for a callback object function which takes no
//        parameters.
// @base public | CBaseSyncCall
// @end 
//
//////////////////////////////////////////////////////////////////////////////

// Template for a callback function that takes
// no parameters
template <class Ret,	// type of return value from function
	  class Caller>		// class of which function is a member
  class CSyncCall0 : public CBaseSyncCall
{
    //@access protected 
protected:
	// @cmember Template parameter: type of return value from function.
	Ret m_Ret;
	// @cmember Template parameter: class of which the function is a member.
	Caller* m_pTarget;

	// @cmember | Ret | (Caller::* m_pfn)() | Definition of the Sync function.<nl>
	Ret (Caller::* m_pfn)();

	// @cmember Pointer to o/s-dependent Sync object.
	COSDependSyncCall *m_pSyncObj;

    //@access public 
public:
    // @cmember | CSyncCall1 | (COSDependSyncCall* pSyncObj, Ret (Caller::* pfn)()) | 
	//Constructor. Set o/s-dependent sync object pointer and function to be called.<nl>
    //Parameterlist:<nl>
    //COSDependSyncCall* pSyncObj // pointer on o/s-dependent Sync object<nl>
    //Ret (Caller::* pfn)() // function to be called on pTarget<nl>
	CSyncCall0(
	    COSDependSyncCall* pSyncObj,
		Ret (Caller::* pfn)()) :
	      m_pfn(pfn),
	      m_pTarget(NULL),
	      m_pSyncObj(pSyncObj)
	{
	}

    // @cmember Calls the SyncCall method of the CBaseSyncCall class with the given parameters.<nl>
    //Parameterlist:<nl>
    //Caller* pTarget // pointer to object to be called<nl>
	Ret Call(Caller* pTarget) 
	{
	    m_pTarget = pTarget;
	    SyncCall( m_pSyncObj );
	    return m_Ret;
	}

    // @cmember SyncRoutine to be called interrupt safe by the driver.<nl>
	BOOL SyncRoutine() 
    {
	    if ( m_pTarget ) 
        {
		    m_Ret = (m_pTarget->*m_pfn)();
	    }
	    return TRUE;
	}
};


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class CSafeBufferQueue | Safe buffer queue class. Contains the methods to
//        add and remove buffers to/from the queue, which are called in a
//        interrupt-safe way.
// @end 
//
//////////////////////////////////////////////////////////////////////////////

class CSafeBufferQueue
{
    //@access private 
private:

    // locked functions are accessible to base core stream
    // because we can't acquire them recursively
    friend class CVampBaseCoreStream;

	// @cmember Buffer list.
    CVampList<CVampBuffer> m_list;

	// @cmember Pointer to o/s-dependent Sync object.
    COSDependSyncCall *m_pSyncObj;

    // @cmember Adds a buffer to the tail of the list. Returns TRUE
    // if list was empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to add<nl>
    BOOL AddLocked( 
		CVampBuffer *pBuffer );

    // @cmember Remove the next buffer from the list and return it.<nl>
    CVampBuffer *RemoveNextLocked();

    // @cmember Remove a buffer from the list and return it.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to remove.<nl>
    CVampBuffer *RemoveLocked( 
		CVampBuffer *pBuffer );

    // @cmember Count buffers which are currently in the list.<nl>
    int CountLocked();

    // @cmember Inserts a buffer to the head of the list. Returns TRUE
    // if list was empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to add<nl>
    BOOL AddToHeadLocked( 
		CVampBuffer *pBuffer );

    //@access public 
public:
    //@cmember Constructor, which gets a o/s-dependent Sync object.<nl>
    //Parameterlist:<nl>
	//COSDependSyncCall* pSyncObj // o/s-dependent Sync object<nl>
    CSafeBufferQueue( 
		COSDependSyncCall *pSyncObj );

    //@cmember Destructor.<nl>
    ~CSafeBufferQueue();

    // @cmember Adds a buffer to the tail of the list. Returns TRUE
    // if list was empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to add<nl>
    BOOL Add( 
		CVampBuffer *pBuffer );

    // @cmember Remove the next buffer from the list and return it.<nl>
    CVampBuffer *RemoveNext();

    // @cmember Remove a buffer from the list and return it.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to remove.<nl>
    CVampBuffer *Remove( 
		CVampBuffer *pBuffer );

    // @cmember Count buffers which are currently in the list.<nl>
    int Count();

    // @cmember Inserts a buffer to the head of the list. Returns TRUE
    // if list was empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to buffer object to add<nl>
    BOOL AddToHead( 
		CVampBuffer *pBuffer );

};


#endif // !defined(AFX_VAMPBUFFER_H__7EB5B3A0_7CAD_11D3_A66F_00E01898355B__INCLUDED_)
