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
// @module   VampVBasicStream | Driver interface for basic video and VBI 
//           streaming methods.
// @end
// Filename: VampVBasicStream.h
//
// Routines/Functions:
//
//  public:
//          CVampVBasicStream
//          ~CVampVBasicStream
//          Open
//          Close
//          Start
//          Stop
//          EnableTransfer
//          GetStatus
//          FrameRateToFSkip
//          GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_VAMPVBASICSTREAM_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_)
#define AFX_VAMPVBASICSTREAM_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "VampError.h"
#include "VampTypes.h"
#include "VampScaler.h"
#include "VampBuffer.h"
#include "AbstractStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampVBasicStream | Basic video and VBI streaming methods.
// @base public | CVampScaler
// @base public | CAbstractStream
// @base protected | CAbstractDPC
// @end
//////////////////////////////////////////////////////////////////////////////

class CVampVBasicStream : 
    public CVampScaler,
    public    CAbstractStream,
    protected CAbstractDPC
{
    //@access private
private:
    //@cmember Pointer on Stream Manager object
    CVampStreamManager *m_pManager;

    //@cmember Pointer on core stream object 
    CVampCoreStream *m_pCoreStream;

    //@cmember Stream status
    eStreamStatus m_eStreamStatus;

    //@cmember Planar stream flag 
    BOOL m_bPlanar;

    //@cmember Channel request flags 
    DWORD m_dwChannelRequest;

    //@cmember Stream information 
    tStreamParms m_StreamInfo;

    //@cmember Stream manager information 
    tStreamManagerInfo m_tStreamManagerInfo;

    //@cmember Line pitch
    DWORD m_dwPitch;

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

    // @cmember Buffer header object for overflow area. Created and deleted in
    // Open and Close methods.
    CVampBuffer* m_pOverflowBuffer;

    //@cmember Current byte order of the stream 
    eByteOrder m_nByteOrder;

    //@cmember Set stream default values.<nl>
    VAMPRET SetVStreamDefaults();

    //@cmember Enable stream transfer.<nl>
    VAMPRET EnableTransfer();
    
    //@cmember Disable stream transfer.<nl>
    VAMPRET DisableTransfer();
    
    //@cmember Callback method called from core stream,
    // calls CallOnDPC of the driver.<nl>
    //Parameterlist:<nl>
    //PVOID pStream // pointer to stream<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    VAMPRET CallOnDPC ( 
        PVOID pStream, 
        PVOID pParam1, 
        PVOID pParam2 );

    //@access protected
protected:
    //@cmember Pointer on device resources object
    CVampDeviceResources *m_pDevRes; // pointer to device resources

    //@cmember Pointer on CVampIo object
    CVampIo *m_pVampIo;

    //@cmember Calculates FSKIP register value out of desired frame rate.<nl>
    VAMPRET FrameRateToFSkip();

    //@cmember Calculates frame rate from FSKIP register value.<nl>
    VAMPRET FSkipToFrameRate();

    //@cmember Calculates the expected delta value for the stream timeline 
    //depending on the chosen stream frame rate.<nl>
    VOID SetDeltaTimeStream();

    //@cmember Adds the callback to the core stream object. <nl>
    //Parameterlist:<nl>
    //CCallOnDPC *pCBOnDPC // CCallOnDPC object pointer<nl>
    //PVOID pParam1 // pointer to first parameter<nl>
    //PVOID pParam2 // pointer to second parameter<nl>
    VAMPRET AddCBOnDPC (
        CCallOnDPC * pCBOnDPC, 
        PVOID pParam1, 
        PVOID pParam2 );

    //@cmember Sets a pointer to a COSDependTimeStamp object. <nl>
    //Parameterlist:<nl>
    //COSDependTimeStamp *pClock // COSDependTimeStamp pointer<nl>
    VOID SetClock (
        COSDependTimeStamp* pClock );

    //@access public
public:
    //@cmember Constructor.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevRes // Pointer on device resources object<nl>
	CVampVBasicStream(
        IN CVampDeviceResources *pDevRes );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    //@cmember Destructor.<nl>
	virtual ~CVampVBasicStream();

    //@cmember Opens a Video/VBI stream.<nl>
    //Parameterlist:<nl>
    //tStreamParams *p_Params // Pointer to stream parameters<nl>
    VAMPRET Open( 
        tStreamParms *p_Params );

    //@cmember Closes a Video/VBI Stream.<nl>
    VAMPRET Close();

    //@cmember Starts a Video/VBI Stream. This method adds the OnBufferComplete 
    //and OnDecoderChange methods to get a report regarding the buffer and decoder
    //status.<nl>
    virtual VAMPRET Start();

    //@cmember Stops a Video/VBI Stream.
    //Parameterlist:<nl>
    //BOOL bRelease // if TRUE: release Channels<nl>
    VAMPRET Stop(
        IN BOOL bRelease );

    //@cmember Get status info of Video/VBI Stream.<nl>
    eStreamStatus GetStatus()
    {
        return m_eStreamStatus;
    }

    //@cmember Add a new buffer to the core stream input queue.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to add<nl>
    VAMPRET AddBuffer(
        IN CVampBuffer *Buffer );

    //@cmember Removes a buffer from the core stream empty or done list.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to remove<nl>
    VAMPRET RemoveBuffer(
        IN CVampBuffer *Buffer );

    //@cmember Returns a done buffer from the core stream queue, or NULL if the
    //list is empty<nl>
    //Parameterlist:<nl>
    //CVampBuffer **pBuffer // buffer object to get<nl>
    VAMPRET GetNextDoneBuffer(
          OUT CVampBuffer **pBuffer );
    
    //@cmember Releases the last buffer from the input queue. This is the
    //buffer which was added last. Returns VAMPRET_BUFFER_IN_PROGRESS if
    //the buffer is currently beeing filled or VAMPRET_BUFFER_NOT_AVAILABLE if
    //the list is empty.
    VAMPRET ReleaseLastQueuedBuffer();

    //@cmember Moves all buffers from the empty list to the done list and
    //marks the buffer with the status cancelled.
    VAMPRET CancelAllBuffers();

   //@cmember Adjusts the clock, since between open and start
   // might be a big time difference. 
    VOID AdjustClock (); 

   //@cmember Store the current settings in the registry.
    VOID SaveSettings (); 

};

#endif // !defined(AFX_VAMPVBASICSTREAM_H__53003DA0_8222_11D3_A66F_00E01898355B__INCLUDED_)
