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
// @module   VampAudioDec | This file contains the audio Decoder class
//           definition. 
// @end
//
//
// Filename: VampAudioDec.h
//
// Routines/Functions:
//
//  public:
//      CVampAudioDec
//      ~CVampAudioDec
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
//      AdjustClock
//      GetObjectStatus
//      AddCBOnDPC
//      SetClock
//	    CallOnDPC
//
//  private:
//      InitStream
//	    DeinitStream
//	    ResetStream
//
//  protected:
//      InitStandard
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _AUDDEC_H__
#define _AUDDEC_H__

#include "VampDeviceResources.h"
#include "VampBuffer.h"

#include "AbstractStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class   CVampAudioDec | The audio decoder class implements the audio decoder
//           of a VAMP device. It manages the video standard dependant audio
//           data handling. The audio stream is used for capture of the audio
//           data into system memory. Data Stream handling operations (e.g.
//           open, close, start and stop) are supported as well.
// @base public | CAbstractStream
// @base public | CAbstractDPC
// @end
//
//////////////////////////////////////////////////////////////////////////////

class CVampAudioDec : public CAbstractStream,
                      public CAbstractDPC    
{
// @access private 
private:
    // @cmember	Audio standard selection
    AStandard m_tStandard;      
    
    // @cmember	Audio stream state variable
    eStreamStatus m_tStreamStatus;     

    // @cmember Error status flag, will be set during construction
    DWORD m_dwObjectStatus;

    // @cmember Buffer header object for overflow area. Created and deleted in
    // Open and Close methods.
    CVampBuffer* m_pOverflowBuffer;

    // @cmember Pointer to the Core stream object
    CVampCoreStream *m_pVampCoreStream; 

    // @cmember	DeviceResource object
    CVampDeviceResources* m_pDeviceResources;  
    
    //@cmember Channel request flags 
    DWORD m_dwChannelFlags;

    // @cmember	FSkip value for desired stream<nl>
    DWORD m_dwFSkip;       
    
    // @cmember	Initalize the Audio stream.<nl>
    // Parameterlist:<nl>
    // AStandard Standard // Audio standard selection<nl>
    VAMPRET InitStream( 
        AStandard tStandard );

    // @cmember	Deinitalize the Audio stream.<nl>
    VAMPRET DeinitStream();

    // @cmember	Reset the Audio stream.<nl>
    VAMPRET ResetStream();		

// @access public
public:
    // @cmember Constructor.<nl>
    // Parameterlist:<nl>
    // CVampDeviceResources* pDevice // pointer on DeviceResources object<nl>
    CVampAudioDec( 
        CVampDeviceResources *pDeviceResources );

    // @cmember Destructor.<nl>
    virtual ~CVampAudioDec();

    // stream functions
    //@cmember Opens a Audio stream.<nl>
    //Parameterlist:<nl>
    //tStreamParams* ptParams // stream parameters<nl>
    VAMPRET Open( 
        IN tStreamParms* ptParams );

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
        IN CVampBuffer *pBuffer );

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

    // @cmember Adjusts the clock, since between open and start
    // might be a big time difference. 
    VOID AdjustClock ();

    // @cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus();

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

};

#endif  // _AUDDEC_H__
