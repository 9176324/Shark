//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantibility or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   VampAudioBaseStream | This file contains the audio user interface
//           base class definition. The audio user interface class implements the
//           audio SDK functionality of the VAMP device. It manages the audio
//           path configuration as well as the stream setup, including
//           handling of different audio norm.
// @end
//
// Filename: VampAudioBaseStream.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioBaseStream
//      ~CVampAudioBaseStream
//		SetLoopbackPath
//		SetI2SPath
//		SetStreamPath
//		GetPath
//		SetFormat
//		GetFormat
//		GetProperty
//		GetBufferSize
//      Open
//      Close
//      Start
//      Stop
//      GetStatus
//		AddBuffer
//		RemoveBuffer
//		GetNextDoneBuffer
//		ReleaseLastQueuedBuffer
//		CancelAllBuffers
//		AdjustClock
//      GetObjectStatus
//
//  private:
//		CallOnDPC
//
//  protected:
//		AddCBOnDPC
//		SetClock
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUDBASESTREAM_H__
#define _AUDBASESTREAM_H__

#include "VampAudioDec.h"

class VampAudioDec;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class   CVampAudioBaseStream | The class is used to describe the User
//          Interface functionality. A set of functions concern the audio path
//          configuration while others (Open, ...) refer to the Stream
//          handling.
// @base public | CVampAbstract
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampAudioBaseStream : public CVampAbstract
{
// @access private 
private:
    //@cmember pointer on device resources object
    CVampDeviceResources* m_pDeviceResources;

    // @cmember pointer to CVampAudioDec class.<nl>
    CVampAudioDec* m_pAudioDec;
                               
    // @cmember Error status flag, will be set during construction.<nl>
    DWORD m_dwObjectStatus;

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

// @access protected
protected:
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

    // @access public
public:
    // @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources* pDevice // pointer on DeviceResources object<nl>
    CVampAudioBaseStream( 
        CVampDeviceResources* pDeviceResources );

    // @cmember Destructor.<nl>
    virtual ~CVampAudioBaseStream();

    // @cmember Set the Audio Loopback Path 
    // (hardware related configuration).<nl>
    // Parameterlist:<nl>
    // AInput tLoopbackInput // audio source selection<nl>
    VAMPRET SetLoopbackPath( 
        AInput tLoopbackInput );

    // @cmember Set Audio I2S Path 
    // (hardware related configuration)<nl>
    // Parameterlist:<nl>
    // AInput tI2sInput // audio source selection<nl>
    VAMPRET SetI2SPath( 
        AInput tI2sInput );

    // @cmember Set Audio Streaming Path 
    // (hardware related configuration).<nl>
    // This configuration has to be done before any DMA stream handling.<nl>
    // Parameterlist:<nl>
    // AInput tStreamInput // audio source selection<nl>
    VAMPRET SetStreamPath( 
        AInput tStreamInput );

    // @cmember Retrieve the corresponding Audio Input 
    // to the demanded Output.<nl>
    // Parameterlist:<nl>
    // AOutput tOutput // audio destination selection<nl>
    AInput GetPath( 
        AOutput tOutput );

    // @cmember Set Audio Format.<nl>
    // Parameterlist:<nl>
    // AFormat Format // audio format<nl>
    VAMPRET SetFormat( 
        AFormat tFormat );

    // @cmember Get Audio Format.<nl>
    AFormat GetFormat();

    // @cmember Get the streaming sample properties.<nl>
    tSampleProperty GetProperty();

    // @cmember Get the DMA buffer size.<nl>
    DWORD GetBufferSize();

    // stream functions
    //@cmember Opens a Audio stream.<nl>
    //Parameterlist:<nl>
    //tStreamParams* ptParams // stream parameters<nl>
    VAMPRET Open(
        IN tStreamParms *ptParams );

    //@cmember Closes a Audio Stream.<nl>
    VAMPRET Close();

    //@cmember Starts a Audio Stream.<nl>
    VAMPRET Start();
    
    //@cmember Stops a Audio Stream.<nl>
    //Parameterlist:<nl>
    //BOOL bRelease // if TRUE: release channels<nl>
    VAMPRET Stop(
        IN BOOL bRelease );

    //@cmember Get status info of Audio Stream.<nl>
    eStreamStatus GetStatus();

    //@cmember  Add a new buffer to input queue.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to add<nl>
    VAMPRET AddBuffer(
        IN CVampBuffer *pBuffer );

	// @cmember Removes the buffer from a queue.<nl>
    // Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to remove<nl>
    VAMPRET RemoveBuffer(
		IN CVampBuffer *pBuffer);

    // @cmember Returns a done buffer from a queue, or NULL if the list is
    // empty.<nl>
    // Parameterlist:<nl>
    // CVampBuffer **ppBuffer // buffer object to get<nl>
    VAMPRET GetNextDoneBuffer(
        OUT CVampBuffer **ppBuffer );

    // @cmember Releases the last buffer from the queued list.
    //          This is the buffer which was added last. Returns
    //          VAMPRET_BUFFER_IN_PROGRESS if thebuffer is currently beeing
    //          filled or VAMPRET_BUFFER_NOT_AVAILABLE if the list is empty.
    VAMPRET ReleaseLastQueuedBuffer();

    // @cmember Moves all buffers from the empty list to the
    //          done list and marks the buffer with the status cancelled.
    VAMPRET CancelAllBuffers();

    // @cmember  Adjusts the clock, since between open and start
    // might be a big time difference. 
    VOID AdjustClock(); 

    // @cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus();
};

#endif  // _AUDBASESTREAM_H__
