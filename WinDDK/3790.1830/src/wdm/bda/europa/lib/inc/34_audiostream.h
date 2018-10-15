//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
// @module   AudioStream | This file contains the audio user interface class
//           definition.
//           The audio user interface class implements the audio
//           SDK functionality of the VAMP device. 
//           It manages the audio path configuration as well as the stream
//           setup, including handling of different audio norm.  
//
//

//
//
// Filename: 34_AudioStream.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioStream
//      ~CVampAudioStream
//      SetLoopbackPath
//      SetI2SPath
//      SetStreamPath
//      GetPath
//      SetFormat
//      GetFormat
//      GetProperty
//      GetBufferSize
//      Open
//      Close
//      Start
//      Stop
//      GetStatus
//      AddBuffer
//      RemoveBuffer
//      GetNextDoneBuffer
//      ReleaseLastQueuedBuffer
//      CancelAllBuffers
//      GetObjectStatus
//      SetRate
//      SetAdjustmentRate
//      GetAdjustmentRate
//      EnableRateAdjust
//      EnableCopy
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUDSTREAM_H__
#define _AUDSTREAM_H__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"

#include "34_Buffer.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class   CVampAudioStream | The class is used to describe
//          the User Interface functionality.
//          A set of functions concern the audio path configuration
//          while others (Open, ...) refer to the Stream handling.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampAudioStream 
{
// @access  Public
public:
    // @access  Public functions
    // @cmember Constructor<nl>
    // Parameterlist:<nl>
     //DWORD dwDeviceIndex // Device Index<nl>
    CVampAudioStream( IN DWORD dwDeviceIndex );
    // @cmember Destructor<nl>
    virtual ~CVampAudioStream( VOID );
    // @cmember Set the Audio Loopback Path<nl> 
	// (hardware related configuration)<nl>
    // Parameterlist:<nl>
	// AInput tLoopbackInput // Audio Source selection<nl>
    VAMPRET SetLoopbackPath( IN AInput tLoopbackInput );
    // @cmember Set Audio I2S Path 
    // (hardware related configuration)<nl>
    // Parameterlist:<nl>
    // AInput tI2sInput // Audio Source selection<nl>
    VAMPRET SetI2SPath( IN AInput tI2sInput );
    // @cmember Set Audio Streaming Path 
    // (hardware related configuration)<nl>
    // This configuration has to be done before any DMA stream handling.<nl>
    // Parameterlist:<nl>
    // AInput tStreamInput // Audio Source selection<nl>
    VAMPRET SetStreamPath( IN AInput tStreamInput );
    // @cmember Retrieve the corresponding Audio Input 
    // to the demanded Output.<nl>
    // Parameterlist:<nl>
    // AOutput tOutput // Audio Destination selection<nl>
    AInput GetPath( IN AOutput tOutput );
    // @cmember Set Audio Format<nl>
    // Parameterlist:<nl>
    // AFormat tFormat // Audio Format<nl>
    VAMPRET SetFormat( IN AFormat tFormat );
    // @cmember Get Audio Format<nl>
    AFormat GetFormat( VOID );   
    // @cmember Get the streaming sample properties<nl>
    tSampleProperty GetProperty( VOID );
    // @cmember Get the DMA buffer size<nl>
    DWORD GetBufferSize( VOID );
    // @cmember Open the DMA stream<nl>
    // Parameterlist:<nl>
    // tStreamParams* ptParams // Audio Stream Parameters<nl>
    VAMPRET Open( IN tStreamParams* ptParams );
    // @cmember Close the DMA stream<nl>
    VAMPRET Close( VOID );
    // @cmember Start the DMA stream<nl>
    VAMPRET Start( VOID );
    // @cmember Stop the DMA stream<nl>
    // Parameterlist:<nl>
    //BOOL bRelease // TRUE: Release Channels<nl>
    VAMPRET Stop(IN BOOL bRelease );
    // @cmember Get the State of the DMA stream<nl>
    eStreamStatus GetStatus( VOID );
    // @cmember Add  a new buffer to the input queue<nl>
    // Parameterlist:<nl>
    // CVampBuffer* pBuffer // Pointer on Buffer object<nl>
    VAMPRET AddBuffer( IN CVampBuffer* pBuffer ); 
    // @cmember Removes the buffer from a queue<nl>
    // Parameterlist:<nl>
    // CVampBuffer* pBuffer // Buffer object (pointer) to remove<nl>
    VAMPRET RemoveBuffer( IN CVampBuffer* pBuffer );
    // @cmember Returns a done buffer from a queue, or NULL if the list is
    // empty<nl>
    // Parameterlist:<nl>
    // CVampBuffer** ppBuffer // Buffer object (pointer to pointer) to get<nl>
    VAMPRET GetNextDoneBuffer(OUT CVampBuffer** ppBuffer);
    // @cmember Releases the last buffer from the queued list.<nl>
    // This is the buffer which was added last. Returns
    // VAMPRET_BUFFER_IN_PROGRESS if thebuffer is currently beeing
    // filled or VAMPRET_BUFFER_NOT_AVAILABLE if the list is empty.<nl>
    VAMPRET ReleaseLastQueuedBuffer();
    // @cmember Moves all buffers from the empty list to the
    // done list and marks the buffer with the status cancelled.<nl>
    VAMPRET CancelAllBuffers();
    // @cmember Reads and returns the current status of this object.<nl>
    DWORD GetObjectStatus();    
    // @cmember Set the desired output sample rate<nl>
    // Parameterlist:<nl>
    // INT nRate // desired output sample rate<nl>
    VAMPRET SetRate(IN INT nRate);
    // @cmember Set the desired adjustment rate<nl>
    // Parameterlist:<nl>
    // INT nSamplesPerSecond // Adjustment rate<nl>
    VAMPRET SetAdjustmentRate(IN INT nSamplesPerSecond);
    // @cmember Get the desired adjustment rate<nl>
    // Parameterlist:<nl>
    // INT* pnSamplesPerSecond // Pointer to adjustment rate<nl>
    VAMPRET GetAdjustmentRate(OUT INT* pnSamplesPerSecond);
    // @cmember Enable the rate adjustment mechanism<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // Enable/disable flag<nl>
    VAMPRET	EnableRateAdjust(IN BOOL bEnable);
    // @cmember Enable the copy mechanism<nl>
    // Parameterlist:<nl>
    // BOOL bEnable // Enable/disable flag<nl>
    VAMPRET	EnableCopy(IN BOOL bEnable);
//@access Private
private:
    //@access Private variables
    //@cmember Status of stream<nl>
    eStreamStatus       m_eStreamStatus;
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex;
    // @cmember Represents the current status of this object.<nl>
    DWORD m_dwObjectStatus;
    //@cmember Pointer to corresponding KM Audio Stream object<nl>
    PVOID m_pKMAudioStream;
};

#endif  // _AUDSTREAM_H__
