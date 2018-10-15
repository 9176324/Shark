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
// @module   VampBaseCoreStream | The base core stream class manages a list of
//           buffers. It splits the buffers into two queues. One for all
//           incoming unprocessed buffers, and one queue for all done buffers.
//           All hardware dependent methods like setting registers have to be
//           defined in a derived class like CVampCoreStream by adding new
//           methods or overloading base methods.
// @end
// Filename: VampBaseCoreStream.h
//
// Routines/Functions:
//
//  public:
//          CVampBaseCoreStream
//          ~CVampBaseCoreStream
//          AddBuffer
//          GetNextDoneBuffer
//          ReleaseLastQueuedBuffer
//          RemoveBuffer
//          CancelAllBuffers
//          CheckOutputQueue
//          AddCBOnDPC
//          CallOnDPC
//          SetClock
//          AdjustClock
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VAMPBASECORESTREAM_H__DC3584A0_D40B_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPBASECORESTREAM_H__DC3584A0_D40B_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "OSDepend.h"
#include "VampTypes.h"
#include "VampBuffer.h"
#include "AbstractStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampBaseCoreStream | Buffer handling and core stream methods
//         regarding all streams.
// @base public | CVampAbstract
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampBaseCoreStream : public CVampAbstract
{

    //@access private 
private:
    // @cmember Interrupt syncronized access object pointer for locking queues
    //          and active area.
    COSDependSyncCall* m_pSyncObject;

    // @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    // in-process area: access serialized by COSDependSyncCall
    // @cmember Current in-process even-field buffer. Buffers are 
    // moved to here when the stream is started, and also at interrupt
    // time when a previous buffer has completed. 
    // Cannot be cancelled.
    CVampBuffer* m_pActiveEven;

    // @cmember Current in-process odd-field buffer
    CVampBuffer* m_pActiveOdd;

    //@access protected 
protected:
    // @cmember True if last field to complete was Odd
    BOOL m_bLastCompletionOdd;

    //@cmember Type of buffer: static or queued<nl>
    eBufferType m_eBufferType;

    //@cmember Type of fields: odd / even / interlaced / don't care<nl>
    eFieldType m_eFieldType;
    
    // @cmember Overflow buffer, created and deleted elsewhere.
    // If set, used to avoid pausing stream (we switch to this
    // when the buffer queue is empty)
    CVampBuffer* m_pOverflow;

    // interrupt-level code must call the Locked functions
    // or we may deadlock

    // access to in-process area -- these methods
    // are called while holding the int-sync

    // @cmember Complete an active buffer. If queued,
    // remove to full list and replace with next from empty list.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    VAMPRET CompleteLocked( 
        BOOL bOdd );

    // @cmember Move a buffer from the empty list to 
    // the active area (to m_pActiveOdd if bOdd, otherwise to m_pActiveEven).
    // Returns buffer that was set active.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    CVampBuffer* NextToActiveLocked( 
        BOOL bOdd );

    // @cmember Copy one active buffer to the other, used in interlaced mode. 
    // If bFromOdd is true, odd is the valid source.<nl>
    //Parameterlist:<nl>
    //BOOL bFromOdd // odd-even flag<nl>
    CVampBuffer* CopyActiveLocked( 
        BOOL bFromOdd );

    // @cmember Set the active buffer IN_USE and return it.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    CVampBuffer* ActiveLocked( 
        BOOL bOdd );

    // @cmember Remove all active buffers to the done list with cancelled status.
    VAMPRET CancelAllActiveLocked();

    // @cmember Replace all active buffers on
    // the empty list (eg on Pause).
    VAMPRET RequeueAllActiveLocked();

   	//@access public 
public:
    // @cmember Queue of empty buffers ready for filling.
    CSafeBufferQueue m_listEmpty;

    // @cmember Queue of full buffers ready for collection.
    CSafeBufferQueue m_listFull;

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    // COSDependSyncCall *pSyncObject // used to serialize access<nl>
    CVampBaseCoreStream( 
        COSDependSyncCall* pSyncObject );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
	virtual ~CVampBaseCoreStream();

    //@cmember Adds a buffer to a queue of unprocessed buffers, regardless whether
    //it is a 'static' or 'queued' buffer. If static, it builds a list with at 
    //least one element.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // buffer object<nl>
    virtual VAMPRET AddBuffer( 
        IN CVampBuffer *pBuffer );

    //@cmember Returns the next buffer from the done list, or NULL if the 
    //list is empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer **pBuffer // pointer to completed buffer (or NULL)<nl>
    virtual VAMPRET GetNextDoneBuffer( 
        IN OUT CVampBuffer **pBuffer );

    //@cmember Releases the last buffer from the queued list. This is the
    //buffer which was added last. Returns VAMPRET_BUFFER_IN_PROGRESS if the
    //buffer is currently beeing filled or VAMPRET_BUFFER_NOT_AVAILABLE if the
    //list is empty.
    virtual VAMPRET ReleaseLastQueuedBuffer();

    //@cmember Removes the buffer from the emty or done list. 
    //Returns VAMPRET_BUFFER_IN_PROGRESS if the buffer is currently 
    //being filled or VAMPRET_BUFFER_NOT_AVAILABLE if the buffer is not found.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // buffer to be canceled<nl>
    virtual VAMPRET RemoveBuffer(
        IN CVampBuffer *pBuffer );

    //@cmember Moves all buffers from the empty list to the done list and marks 
    //the buffer with the status cancelled.<nl>
    //Note: This function must not be called as long as transfer is enabled.
    virtual VAMPRET CancelAllBuffers();

    //@cmember Checks the output queue and returns the number of available buffers. 
    virtual DWORD CheckOutputQueue()
    {
        // check if output queue empty
        return m_listFull.Count();
    }

    //@cmember This method gives the driver the opportunity to add a callback
    //         function to the core stream CallOnDPC method. The callback 
    //         will be called after an OnDPC call. To remove  the Callback,
    //         call this method with a NULL pointer.
    //It returns VAMPRET_SUCCESS or a warning VAMPRET_CALLBACK_ALREADY_MAPPED
    //if a callback is already mapped to the CallOnDPC method. <nl>
    //Parameterlist:<nl>
    //CCallOnDPC *pCBOnDPC // CCallOnDPC object pointer<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    virtual VAMPRET AddCBOnDPC( 
        CCallOnDPC *pCBOnDPC, 
        PVOID pParam1, 
        PVOID pParam2 );

    //@cmember VAMPRET | OnDPC | ( PVOID pStream = NULL ) | This callback will be added
    //to the corestream object and will be called back after OnDPC<nl>
    //Parameterlist:<nl>
    //PVOID pStream // pointer to stream<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    virtual VAMPRET CallOnDPC( 
        PVOID pStream = NULL, 
        PVOID pParam1 = NULL, 
        PVOID pParam2 = NULL );

    //@cmember Sets a pointer to a COSDependTimeStamp object. <nl>
    //Parameterlist:<nl>
    //COSDependTimeStamp *pClock // COSDependTimeStamp pointer<nl>
    virtual VOID SetClock( 
        COSDependTimeStamp* pClock ) 
    {
        m_pClock = pClock;   
    }

   //@cmember Adjusts the clock, since between open and start
   // might be a big time difference. 
    virtual VOID AdjustClock() 
    {
        m_pClock->SetStartTime();
    }
    
   //@cmember Returns the type of the current buffer.
    eBufferType GetBufferType() 
    {
        return m_eBufferType;
    }

   //@cmember Returns TRUE, if a callback on DPC is set.
    BOOL IsCBOnDPC () 
    {
        return ( m_pCBOnDPC ? TRUE : FALSE );
    }

    // @cmember Complete an active buffer (odd if bOdd true, else even).
    // If queued, move to done queue and replace with next.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    VAMPRET Complete( 
        BOOL bOdd );

    // @cmember Moves the next empty buffer into the active area
    // (to m_pActiveOdd if bOdd, otherwise to m_bActiveEven).
    // Returns the buffer that is set active.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    CVampBuffer* NextToActive( 
        BOOL bOdd );
    
    // @cmember Copy one active buffer to the other 
    // (used in interlaced mode). 
    // If bFromOdd is true, odd is the valid source.
    // Obtains int spinlock and calls CopyActiveLocked.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    CVampBuffer* CopyActive( 
        BOOL bFromOdd );

    // @cmember Acquire the lock, set the active buffer in use and
    // return it.<nl>
    //Parameterlist:<nl>
    //BOOL bOdd // odd-even flag<nl>
    CVampBuffer* Active( 
        BOOL bOdd );

    // @cmember Check input queue length (acquires lock in queue method).
    virtual int InputCount() 
    {
        return m_listEmpty.Count();
    }

    // @cmember Acquire the interrupt spinlock and then move all
    // active buffers to the done list with cancelled status.
    VAMPRET CancelAllActive();
    
    // @cmember Acquire the interrupt spinlock and then move all
    // active buffers to the empty list (eg during pause).
    VAMPRET RequeueAllActive();
    
    //@cmember This method sets the field type of the stream: odd fields,
    //even fields, interlaced or don't care.<nl>
    //Parameterlist:<nl>
    //eFieldType nFieldType // type of fields to process<nl>
    virtual VOID SetFieldType( 
        IN eFieldType nFieldType )
    {
        m_eFieldType = nFieldType;
    }

    //@cmember This method returns the field type of the stream: odd fields, 
    //even fields, interlaced or don't care.<nl>
    virtual eFieldType GetFieldType()
    {
        return m_eFieldType;
    }


    //@cmember Specify an overflow buffer to be used instead of pausing the
    // stream when buffer queue is empty. This permits a higher
    // frame rate in low-buffer-count video preview situations.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to overflow buffer object<nl>
    virtual VAMPRET SetOverflowBuffer(
        CVampBuffer* pBuffer );

};

#endif // !defined(AFX_VAMPBASECORESTREAM_H__DC3584A0_D40B_11D3_A66F_00E01898355B__INCLUDED_)
