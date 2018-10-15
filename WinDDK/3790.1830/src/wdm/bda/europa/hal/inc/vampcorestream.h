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
// @module   VampCoreStream | This is the core for all streaming objects,
//           namely: transport stream, audio stream, video stream (Task A and
//           B) and VBI stream. The core stream class manages a list of
//           buffers. It splits the buffers into two queues, one for all
//           incoming unprocessed buffers, and one for all done buffers.
//           Furthermore it accesses the appropriate DMA register set.
// @end
// Filename: VampCoreStream.h
// 
// Routines/Functions:
//
//  public:
//          CVampCoreStream
//          ~CVampCoreStream
//          GetChannel
//          ReleaseChannel
//          AddBuffer
//          GetNextDoneBuffer
//          ReleaseLastQueuedBuffer
//          RemoveBuffer
//          CancelAllBuffers
//          OnCompletion
//          SetClock
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPCORESTREAM_H__B3D41843_7C91_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPCORESTREAM_H__B3D41843_7C91_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampBaseCoreStream.h"
#include "VampDmaChannel.h"
#include "VampDeviceResources.h"
class CVampDeviceResources;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampCoreStream | Buffer handling and core methods regarding all
//         streams.
// @base public | CVampBaseCoreStream
// @end 
//////////////////////////////////////////////////////////////////////////////

class CVampCoreStream : public CVampBaseCoreStream
{
   	//@access private 
private:
    PVOID m_pCB_Context;
    BOOL m_bEitherFieldInit;

	//@cmember Expected timeline of the stream regarding the frame rate<nl>
    t100nsTime m_TimeLineStream;

	//@cmember Time stamp, actual timeline of the completed buffers<nl>
    t100nsTime m_TimeLineBuffer;

	//@cmember Expected time difference between two consecutive buffers<nl>
    t100nsTime m_DeltaTimeStream;

    // @cmember In interlaced mode we save 2 fields (odd and even) into the
    //          same buffer. After completion of the first (odd) part we try
    //          to set the odd base address of the DMA channel to the next
    //          buffer, while the second (even) field is transferred to the
    //          current buffer. If the next buffer is not yet available, the
    //          m_bBufferPending flag is set to TRUE.
    BOOL m_bBufferPending;
    
    //@cmember Check, if the buffers are too fast or too slow regarding the desired 
    //         frame rate of the stream. Buffers which come too fast will be not
    //         put into the output queue. If buffers come too slow, there will be
    //         dropped frames reported.<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // buffer object<nl>.
    BOOL CheckBufferInTime(
        IN CVampBuffer* pBuffer);

   	//@access protected 
protected:
	//@cmember pointer to the common CVampDeviceResources object.<nl>
    CVampDeviceResources *m_pDevice;
    
	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    // @cmember Video format, needed for calculation of pitch and buffer
    //          offset in case of planar
    eVideoOutputFormat m_nFormat;

   	//@access public 
public:
    // @cmember Indicates that the stream was paused due to a lack of buffers and
    // should be restarted, when more buffers are available.
    // Set and cleared at interrupt level only, but can be checked
    // automically at lower levels.
    bool m_bAutoPaused;

    //@cmember Pointer to the current DMA register set (channel)<nl>
    CVampDmaChannel *m_pDmaChannel;

    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevice // pointer to device resources object<nl>
	CVampCoreStream( 
        IN CVampDeviceResources *pDevice );

    //@cmember Destructor.<nl>
	virtual ~CVampCoreStream();

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Assignment of a DMA channel object to the appropriate DMA
    //         register set according to dwChannel.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // DMA channel number <nl>
    virtual VAMPRET GetChannel( 
        IN DWORD dwChannel );

    //@cmember Assignment of a primary and secondary DMA channel object to the
    //         appropriate DMA register sets according to dwChannel1 and
    //         dwChannel2.<nl>
    //Parameterlist:<nl>
    //DWORD dwChannel // primary DMA channel number <nl>
    //DWORD dwChannel2 // secondary DMA channel number <nl>
    virtual VAMPRET GetChannel( 
        IN DWORD dwChannel, 
        IN DWORD dwChannel2 );

    //@cmember Releases DMA channel object(s).<nl>
    virtual VAMPRET ReleaseChannel();

    //@cmember This method sets the current buffer entry offset. Not necessary 
    //here, only needed for planar core streams.<nl>
    //Parameterlist:<nl>
    //DWORD dwBufferSize // size of buffer<nl>
    virtual VOID SetBufferOffset( 
        IN LONG dwBufferSize )
    {
    }

    //@cmember This method sets the current video output format<nl>
    //Parameterlist:<nl>
    //eVideoOutputFormat nFormat // video output format<nl>
    virtual VOID SetVFormat( 
        IN eVideoOutputFormat nFormat )
    {
        m_nFormat = nFormat;
    }

    //@cmember Adds a buffer to a queue of unprocessed buffers by calling the
    //basic AddBuffer method. There is a check whether the stream has
    //been stopped because we went out of buffers. In this case the
    //stream is re-started again.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *pBuffer // buffer object pointer<nl>
    virtual VAMPRET AddBuffer( 
        IN CVampBuffer *pBuffer );

    //@cmember Called by ISR at interrupt time to process a buffer completion. 
    //Checks the status of the queue. Moves any completed buffers to the done 
    //queue. Changes the status and adds TimeStamp to the CVampBuffer object. 
    //Swaps pointer to next buffer, if there is a buffer of the appropriate 
    //type, or signals a dropped frame, if no buffer is available. Triggers an 
    //OnBufferComplete call, if the done list is not empty (or optionally for 
    //every frame in static mode).<nl>
    //Parameterlist:<nl>
    //eInterruptCondition nIntCond // interrupt condition<nl>
    //DWORD dwLostBuffers // number of lost buffers<nl>
	//eFifoOverflowStatus FifoOverflowStatus // FIFO overflow status<nl>
    //BOOL bRecovered // indicates recovered buffer<nl>
    //eFifoArea nFifo // actual FIFO area<nl>
    virtual VAMPRET OnCompletion(
        IN eInterruptCondition nIntCond,
        IN DWORD dwLostBuffers,
		IN eFifoOverflowStatus FifoOverflowStatus,
        IN BOOL bRecovered,
        IN eFifoArea nFifo );

    //@cmember Returns TRUE in case of dual channel programming (VBI).
    virtual BOOL DualDmaChannel()
    {
        if ( m_pDmaChannel )
            return m_pDmaChannel->m_bDual;
        else
            return FALSE;
    }

    //@cmember Get the main status of the chip, according to input parameter
    //flags dwType. The input parameters are flags, which can be 'OR'ed, 
    //reflecting the types of the status information register.<nl> 
    // Parameterlist:<nl> 
    // IN DWORD dwType // requested status type (DMA_STS_xxx...)<nl>
    DWORD CVampCoreStream::GetDmaMainStatus( 
        IN DWORD dwType );

    //@cmember Get the current status of the DMA transfer, according to input
    // parameter flags dwType and dwChannel. The input parameters are
    // flags which can be 'OR'ed, reflecting the types of the status information
    // register and the according DMA channels or FIFOs. Some types are dedicated
    // to a DMA channel, some to a FIFO. The parameter defines are 
    // transparent for both, Channels and FIFOs.<nl>
    //Parameterlist:<nl>
    // IN DWORD dwType // requested status type (DMA_STS_xxx...)<nl>
    // IN DWORD dwChannel // requested DMA channel or FIFO (DMA_STS_CHANNELx, DMA_STS_FIFOx...)<nl>
    DWORD GetDmaChannelStatus( 
        IN DWORD dwType, 
        IN DWORD dwChannel );

    //@cmember Get the current status of the FIFO transfer, according to input
    // parameter flags dwType and dwFifo. The input parameters are
    // flags which can be 'OR'ed, reflecting the types of the status information
    // register and the according DMA channels or FIFOs.<nl>
    //Parameterlist:<nl>
    // IN DWORD dwType // requested status type (DMA_STS_xxx...)<nl>
    // IN DWORD dwChannel // requested DMA channel or FIFO (DMA_STS_CHANNELx, DMA_STS_FIFOx...)<nl>
    // OUT DWORD *pdwDmaStatus // status value<nl>
    VAMPRET GetDmaFifoStatus( 
        IN DWORD dwType, 
        IN DWORD dwFifo,
        OUT DWORD *pdwFifoStatus );

    //@cmember Set the DMA main control bits to dwValue, according to input
    //         parameter flags dwType. The type parameter flags represent a
    //         single register field, the dwValue parameter is the value of
    //         this RegField (not a mask of the complete register).<nl>
    //Parameterlist:<nl>
    // IN DWORD dwType // type to set (DMA_CTR_xxx...)<nl>
    // IN DWORD dwValue // value<nl>
    VAMPRET SetDmaMainControl( 
        IN DWORD dwType, 
        IN DWORD dwValue );

    //@cmember Get the current DMA main control bits, according to input
    //         parameter flags dwType. The input parameters are flags which
    //         can be 'OR'ed, reflecting the types of the main control RegFields.
    //         The returned bits keep their position within the 32-bit
    //         MainControl register<nl>
    //Parameterlist:<nl>
    // IN DWORD dwType // requested type (DMA_CTR_xxx...)<nl>
    DWORD GetDmaMainControl( 
        IN DWORD dwType );

    //@cmember Set the DMA register set(s) according to stream start conditions.<nl>
    virtual VAMPRET SetDmaStartRegs();

    //@cmember Set the DMA address registers at interrupt time.<nl>
    //Parameterlist:<nl>
    //eFieldType nDmaFieldType // field type<nl>
    //CVampBuffer* pBuffer // buffer object<nl>.
    VAMPRET SetDmaAddressRegs( 
        IN eFieldType nDmaFieldType,
        IN CVampBuffer* pBuffer );

    //@cmember Set the expected delta value for the stream timeline regarding on the
    //         desired stream frame rate.<nl>
    //Parameterlist:<nl>
    //t100nsTime deltaTimeStream // time stamp delta value in units of 100nsec<nl>
    virtual VOID SetDeltaTimeStream( t100nsTime deltaTimeStream )
    {
        m_TimeLineStream = 0;
        m_DeltaTimeStream = deltaTimeStream;
    }

    // @cmember Restart stream from paused when more buffers become available, after
    // the stream was automatically paused at interrupt time due to lack of buffers.
    // Must be called under interrupt sync locking
    virtual VAMPRET AutoRestartLocked( void );

    // @cmember Handle stream stop by moving active but unfilled buffers
    // back onto the empty queue
    virtual VAMPRET StopStreaming();

    //@cmember This method sets the field type of the stream: odd fields,
    //even fields, interlaced or don't care.<nl>
    //Parameterlist:<nl>
    //eFieldType nFieldType // type of fields to process<nl>
    virtual VOID SetFieldType( 
        IN eFieldType nFieldType )
    {
        m_eFieldType = nFieldType;
    }
};

#endif // !defined(AFX_VAMPCORESTREAM_H__B3D41843_7C91_11D3_A66F_00E01898355B__INCLUDED_)
