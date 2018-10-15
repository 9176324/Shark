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
// @module   VampAudioStream | This file contains the audio user interface class
//           definition.
//           The audio user interface class implements the audio
//           SDK functionality of the VAMP device. 
//			 It manages the audio path configuration as well as the stream
//			 setup, including handling of different audio norm.  
// @end
//
// Filename: VampAudioStream.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioStream
//      ~CVampAudioStream
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
//		GetObjectStatus
//		SetRate
//		SetAdjustmentRate
//		GetAdjustmentRate
//		EnableRateAdjust
//		EnableCopy
//
//  private:
//		CallOnDPC
//
//  protected:
//		AddCBOnDPC
//		SetClock
//

//////////////////////////////////////////////////////////////////////////////

#ifndef _AUDSTREAM_H__
#define _AUDSTREAM_H__


//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class   CVampAudioStream | The class is used to describe
//          the User Interface functionality.
//			A set of functions concern the audio path configuration
//			while others (Open, ...) refer to the Stream handling.
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampAudioBaseStream;
class ARateAdjust; 
class AClockAdjuster;

class CVampAudioStream 
{
    // @access protected
protected:
    //@cmember pointer on device resources object
    CVampDeviceResources *m_pDevice;

    //@cmember pointer on audio base stream object
    CVampAudioBaseStream *m_pAudioStream;

    //@cmember pointer on abstract stream object
    CVampAbstract* m_pStream;

    //@cmember pointer on ARateAdjust object
    ARateAdjust *m_pAdjuster;

    //@cmember pointer on AClockAdjuster object
    AClockAdjuster *m_pClockAdjuster;

    //@cmember enable flag
    BOOL m_bEnable;

    //@cmember copy flag
    BOOL m_bDoCopy;

    // @access public
public:
    // @cmember Constructor<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources* pDevice // pointer on DeviceResources object<nl>
    CVampAudioStream( 
        CVampDeviceResources *pDeviceResources );

    // @cmember Destructor<nl>
    virtual ~CVampAudioStream();

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
    // AInput tStreamInput // audio Source selection<nl>
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
    AFormat GetFormat( VOID );

    // @cmember Get the streaming sample properties.<nl>
    tSampleProperty GetProperty( VOID );

    // @cmember Get the DMA buffer size.<nl>
    DWORD GetBufferSize( VOID );

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

    // @cmember Set the desired output sample rate.<nl>
    // Parameterlist:<nl>
    // INT nRate // desired output sample rate<nl>
    VAMPRET SetRate(
        IN INT nRate );

    // @cmember Set the desired adjustment rate.<nl>
    // Parameterlist:<nl>
    // INT nSamplesPerSecond // adjustment rate<nl>
    VAMPRET SetAdjustmentRate(
        IN INT nSamplesPerSecond );

    // @cmember Get the desired adjustment rate.<nl>
    // Parameterlist:<nl>
    // INT *pnSamplesPerSecond // pointer to adjustment rate<nl>
    VAMPRET GetAdjustmentRate(
        IN INT *pnSamplesPerSecond );

    // @cmember Enable the rate adjustment mechanism.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable/disable flag<nl>
    VAMPRET	EnableRateAdjust(
        IN BOOL bEnable );

    // @cmember Enable the copy mechanism.<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // enable/disable flag<nl>
    VAMPRET	EnableCopy(
        IN BOOL bEnable );

};

#endif  // _AUDSTREAM_H__

