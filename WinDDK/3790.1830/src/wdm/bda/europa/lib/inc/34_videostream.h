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
// @module   VideoStream | Driver interface for video streaming.

// 
// Filename: 34_VideoStream.h
//
// Routines/Functions:
//
//  public:
//      CVampVideoStream
//      ~CVampVideoStream
//      SetVisibleRect
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
//	    GetObjectStatus
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _VIDEOSTREAM__
#define _VIDEOSTREAM__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"
#include "34_Buffer.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampVideoStream | Video streaming methods.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampVideoStream 
{
//@access Public
public:
	//@access Public functions
    //@cmember Constructor<nl>
    //Parameterlist<nl>
    //DWORD dwDeviceIndex // Device Index<nl>
	CVampVideoStream(
        IN DWORD dwDeviceIndex );     
    //@cmember Destructor<nl>
	virtual ~CVampVideoStream();              
    //@cmember Set video input rect and clip list constructor<nl>
    //Parameterlist<nl>
    //tRectList* ptRectList		// Ptr to visible rectangle list<nl>
    VAMPRET SetClipList(
        IN tRectList* ptRectList );                       
	// stream functions
    //@cmember Opens a Video stream<nl>
    //Parameterlist:<nl>
    //tStreamParams* ptParams	// Stream parameters<nl>
    VAMPRET Open(
        IN tStreamParams* ptParams );
    //@cmember Closes a Video Stream.<nl>
    VAMPRET Close();
    //@cmember Starts a Video Stream.<nl>
    VAMPRET Start();			
    //@cmember Stops a Video Stream<nl>
    //Parameterlist:<nl>
    //BOOL bRelease // TRUE: Release Channels<nl>
    VAMPRET Stop(
        IN BOOL bRelease );
    //@cmember Get status info of Video Stream<nl>
    eStreamStatus GetStatus();
    //@cmember  Add a new buffer to input queue<nl>
    //Parameterlist:<nl>
    //CVampBuffer* pBuffer // Buffer object (pointer) to add<nl>
    VAMPRET AddBuffer(
        IN CVampBuffer* pBuffer );
	// @cmember Removes the buffer from a queue<nl>
    // Parameterlist:<nl>
    // CVampBuffer* pBuffer // Buffer object (pointer) to remove<nl>
    VAMPRET RemoveBuffer(
		IN CVampBuffer* pBuffer);
    // @cmember Returns a done buffer from a queue, or NULL if the list is
    // empty<nl>
    // Parameterlist:<nl>
    // CVampBuffer** pBuffer // Buffer object (pointer to pointer) to get<nl>
    VAMPRET GetNextDoneBuffer(OUT CVampBuffer** ppBuffer);
    // @cmember Releases the last buffer from the queued list.
    //          This is the buffer which was added last. Returns
    //          VAMPRET_BUFFER_IN_PROGRESS if thebuffer is currently beeing
    //          filled or VAMPRET_BUFFER_NOT_AVAILABLE if the list is empty.
    VAMPRET ReleaseLastQueuedBuffer();
    // @cmember Moves all buffers from the empty list to the
    //          done list and marks the buffer with the status cancelled.
    VAMPRET CancelAllBuffers();
    // @cmember Reads and returns the current status of this object
    DWORD GetObjectStatus();
    // @cmember Set clipping mode.<nl>
    // Parameterlist:<nl>
    // eClipMode nClipMode // Clipping mode to set
    VOID SetClipMode(
        eClipMode nClipMode );
//@access Private
private:
	//@access Private variables
    //@cmember Status of the Video stream<nl>
    eStreamStatus m_eStreamStatus;
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
    // @cmember Represents the current status of this object.<nl>
    DWORD m_dwObjectStatus;
    //@cmember Pointer to corresponding KM Video Stream object<nl>
    PVOID m_pKMVideoStream;
};

#endif // _VIDEOSTREAM__
