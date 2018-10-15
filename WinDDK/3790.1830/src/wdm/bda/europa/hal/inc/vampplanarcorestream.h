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
// @module   VampPlanarCoreStream | This is the core for planar video streams,
//           which has to open three core stream objects:
//           Y-CoreStream, U-CoreStream, and V-CoreStream.
// @end
// Filename: VampPlanarCoreStream.h
// 
// Routines/Functions:
//
//  public:
//          CVampPlanarCoreStream
//          ~CVampPlanarCoreStream
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VAMPPLANARCORESTREAM_H__D4B61F00_A665_11D4_A670_00E01898355B__INCLUDED_)
#define AFX_VAMPPLANARCORESTREAM_H__D4B61F00_A665_11D4_A670_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "VampCoreStream.h"
#include "VampDeviceResources.h"

class CVampDeviceResources;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampPlanarCoreStream | Buffer handling and core methods regarding 
//         planar streams.
// @base public | CVampCoreStream
// @end 
//////////////////////////////////////////////////////////////////////////////

class CVampPlanarCoreStream : public CVampCoreStream
{
   	//@access private 
private:
	// Error status flag, will be set during construction.
    // DWORD m_dwObjectStatus;

	// @cmember Core stream pointer for Y data.
    CVampCoreStream *m_pCoreStreamY;

	// @cmember Core stream pointer for U data.
    CVampCoreStream *m_pCoreStreamU;

	// @cmember Core stream pointer for V data.
    CVampCoreStream *m_pCoreStreamV;

    // @cmember Queue of buffer pool
    CSafeBufferQueue m_listPool;

    // @cmember Queue of input buffers
    CSafeBufferQueue m_listInput;

    // @cmember Queue of output buffers
    CSafeBufferQueue m_listOutput;

    // @cmember Flags for Fifo Counter
    DWORD m_dwFifoFlags;

    // @cmember Sets and returns the count flags of the affected FIFOs while
    // planar stream is running. Necessary to avoid a mismatch of Y, U and V
    // sections within a planar buffer.<nl>
    //Parameterlist:<nl>
    //eFifoArea nFifo // current FIFO number<nl>
    DWORD FifoCounter( 
        IN eFifoArea nFifo );

    // @cmember Pitch factor for Y channel
    tFraction m_tChannelPitchFactorY;

    // @cmember Pitch factor for U channel
    tFraction m_tChannelPitchFactorU;

    // @cmember Pitch factor for V channel
    tFraction m_tChannelPitchFactorV;

    // @cmember Planar pitch factor
    DWORD m_dwPlanarPitchFactor;

    // @cmember Indicator for a running planar stream
    BOOL m_bRunning;

    // @cmember Next expected FiFo completion
    eFifoArea m_nextFifo;
    
    //@access public 
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer to device resources object<nl>
	CVampPlanarCoreStream( 
        IN CVampDeviceResources *pDevice );

    //@cmember Destructor.<nl>
	virtual ~CVampPlanarCoreStream();

	//cmember Returns the error status set by constructor.<nl>
    //DWORD GetObjectStatus()
    //{
    //    return m_dwObjectStatus;
    //}

    //@cmember Assignment of a DMA channel object to the appropriate DMA
    //         register set according to dwChannel.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // DMA channel number <nl>
    VAMPRET GetChannel( 
        IN DWORD dwChannel );
    
    //@cmember Assignment of a primary and secondary DMA channel object to the
    //         appropriate DMA register sets according to dwChannel1 and
    //         dwChannel2.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // primary DMA channel number <nl>
    //DWORD dwChannel2 // secondary DMA channel number <nl>
    VAMPRET GetChannel( 
        IN DWORD dwChannel, 
        IN DWORD dwChannel2 );

    //@cmember Releases DMA channel object(s).<nl>
    VAMPRET ReleaseChannel();


    //@cmember This method sets the current video output format<nl>
    //Parameterlist:<nl>
    //eVideoOutputFormat nFormat // video output format<nl>
    VOID SetVFormat( 
        IN eVideoOutputFormat nFormat );

    //@cmember This method sets the current buffer entry offset. In case of a
    //planar stream we have one buffer for Y, U and V data. So we have to
    //set the start of the different data areas inside the buffer.<nl>
    //Parameterlist:<nl>
    //LONG lBufferOffset // offset of data in buffer<nl>
    VOID SetBufferOffset( 
        IN LONG lBufferOffset );

    //@cmember Set the DMA register set(s) according to start conditions<nl>
    VAMPRET SetDmaStartRegs();

    //@cmember Set the DMA address registers at interrupt time.<nl>
    //Parameterlist:<nl>
    //eFieldType nDmaFieldType // field type<nl>
    //CVampBuffer* pBuffer // buffer object<nl>.
    VAMPRET SetDmaAddressRegs( 
        IN eFieldType nDmaFieldType,
        IN CVampBuffer* pBuffer );


    //@cmember Copy the buffer parameters of an input buffer to a pool buffer.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBufferY // Y buffer object (input buffer)<nl>
    //CVampBuffer* pBufferU // U buffer object (U pool buffer)<nl>
    //CVampBuffer* pBufferV // V buffer object (V pool buffer)<nl>
    VAMPRET SetPlanarBuffer(
        IN CVampBuffer *pBufferY,
        IN CVampBuffer *pBufferU,
        IN CVampBuffer *pBufferV );

    //@cmember Copy the buffer status of a pool buffer to an input buffer.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBufferSrc // source buffer object (pool buffer)<nl>
    //CVampBuffer* pBufferDst // destination buffer object (input buffer)<nl>
    VAMPRET GetPlanarBufferStatus(
        IN CVampBuffer *pBufferSrc,
        IN CVampBuffer *pBufferDst );

    //@cmember Adds a buffer to a queue of unprocessed buffers by calling the
    //basic AddBuffer method. There is a check whether the stream has
    //been stopped because it went out of buffers. In this case the
    //stream is re-started again.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // buffer object pointer<nl>
    VAMPRET AddBuffer( 
        IN CVampBuffer *pBuffer );

    // @cmember Restart stream from paused when more buffers are available, if
    // the stream was automatically paused at interrupt time due to lack of buffers.
    // Must be called under interrupt sync locking.
    VAMPRET AutoRestartLocked();

    //@cmember Called by ISR at interrupt time to process a buffer completion. 
    //Checks the status of the queue. Moves any completed buffers to the done 
    //queue. Changes the status and adds TimeStamp to the CVampBuffer object. 
    //Swaps pointer to next buffer, if there is a buffer of the appropriate 
    //type, or signals a dropped frame, if no buffer is available. Triggers an 
    //OnBufferComplete call, if the done list is not empty (or optionally for 
    //every frame in static mode).<nl>
    //Parameterlist:
    //eInterruptCondition nIntCond // interrupt condition<nl>
    //DWORD dwLostBuffers // number of lost buffers<nl>
	//eFifoOverflowStatus FifoOverflowStatus // FIFO overflow status<nl>
    //BOOL bRecovered // indicates recovered buffer<nl>
    //eFifoArea nFifo // actual FIFO area<nl>
    VAMPRET OnCompletion(
        IN eInterruptCondition nIntCond,
        IN DWORD dwLostBuffers,
		IN eFifoOverflowStatus FifoOverflowStatus,
        IN BOOL bRecovered,
        IN eFifoArea nFifo );

    //@cmember Returns the next buffer from the done list, or NULL if the 
    //list is empty.<nl>
    //Parameterlist:<nl>
    //CVampBuffer **pBuffer // pointer to filled buffer or NULL<nl>
    VAMPRET GetNextDoneBuffer( 
        IN OUT CVampBuffer **ppBuffer );

    //@cmember Releases the last buffer from the queued list. This is the
    //buffer which was added last. Returns VAMPRET_BUFFER_IN_PROGRESS if the
    //buffer is currently beeing filled or VAMPRET_BUFFER_NOT_AVAILABLE if the
    //list is empty.<nl>
    VAMPRET ReleaseLastQueuedBuffer()
    {
        // not yet used
        return VAMPRET_NO_BUFFER_AVAILABLE;
    }

    //@cmember Removes the buffer from the emty or done list. 
    //Returns VAMPRET_BUFFER_IN_PROGRESS if the buffer is currently 
    //being filled or VAMPRET_BUFFER_NOT_AVAILABLE if the buffer is not found.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // pointer to canceled buffer<nl>
    VAMPRET RemoveBuffer(
        IN CVampBuffer *pBuffer );

    //@cmember Moves all buffers from the empty list to the done list and marks 
    //the buffer with the status cancelled.<nl>
    //Note: This function must not be called as long as transfer is enabled.<nl>
    VAMPRET CancelAllBuffers();

    //@cmember Set the expected delta value for the stream timeline regarding on the
    //         desired stream frame rate.<nl>
    //Parameterlist:<nl>
    //t100nsTime deltaTimeStream // time stamp delta value in units of 100nsec<nl>
    VOID SetDeltaTimeStream( 
        t100nsTime deltaTimeStream );

    //@cmember Sets a pointer to a COSDependTimeStamp object. <nl>
    //Parameterlist:<nl>
    //COSDependTimeStamp *pClock // COSDependTimeStamp pointer<nl>
    VOID SetClock( 
        COSDependTimeStamp* pClock ); 

    //@cmember  Adjusts the clock, since between open and start
    // might be a big time difference. <nl>
    VOID AdjustClock(); 
    
    //@cmember Specify an overflow buffer to be used instead of pausing the
    // stream when buffer queue is empty. This permits a higher
    // frame rate in low-buffer-count video preview situations.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // pointer to overflow buffer object<nl>
    VAMPRET SetOverflowBuffer(
        CVampBuffer* pBuffer );

    //@cmember Check input queue length (acquires lock in queue method).
    int InputCount(); 

    //@cmember Returns TRUE in case of dual channel programming (VBI).
    BOOL DualDmaChannel()
    {
        if ( m_pCoreStreamY )
            return m_pCoreStreamY->m_pDmaChannel->m_bDual;
        else
            return FALSE;
    }

    // @cmember Handle stream stop by moving active but unfilled buffers
    // back onto the empty queue.
    VAMPRET StopStreaming();

    //@cmember This method sets the field type of the stream: odd fields,
    //even fields, interlaced or don't care.<nl>
    //Parameterlist:<nl>
    //eFieldType nFieldType // type of fields to process<nl>
    VOID SetFieldType( 
        IN eFieldType nFieldType );
};

#endif // !defined(AFX_VAMPPLANARCORESTREAM_H__D4B61F00_A665_11D4_A670_00E01898355B__INCLUDED_)
