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
// @module   Transport Stream | This file describes the User Interface of the
//           Transport Stream class. The Transport Stream is a GPIO input
//           stream for digital TV. It can be a parallel stream or a serial
//           stream.
// @comm     It is assumed that the Source type for the appropriate stream
//           type is stored in the registry and can not be changed
//           dynamically.

// Filename: 34_TransportStream.h
//
// Routines/Functions:
//
//  public:
//      CVampTransportStream
//      ~CVampTransportStream
//      Open
//		Close
//      Start
//		Stop
//      GetStatus
//		AddBuffer
//		RemoveBuffer
//		GetNextDoneBuffer
//		ReleaseLastQueuedBuffer
//		CancelAllBuffers
//		Reset
//      GetPinsConfiguration
//      GetObjectStatus
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _TRANSPORTSTREAM__
#define _TRANSPORTSTREAM__

#include "34api.h"
#include "34_error.h"
#include "34_types.h"
#include "34_Buffer.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTransportStream | This class is the user interface for the
//         transport stream stuff.

//
//////////////////////////////////////////////////////////////////////////////

class P34_API CVampTransportStream 
{
// @access Public
public:
    //@access Public functions
    //@cmember Constructor<nl> 
    //Parameterlist:<nl>
    //DWORD dwDeviceIndex // Device Index<nl>
    //eTSStreamType nStreamType // Type of Stream<nl>
    CVampTransportStream(
                IN DWORD dwDeviceIndex, 
                IN eTSStreamType nStreamType);
    // @cmember Destructor.<nl>
    virtual ~CVampTransportStream();
    // @cmember Open a transport stream.<nl>
    //Parameterlist:<nl>
    //tStreamParms *ptStreamParams // Stream parameters<nl>
    VAMPRET       Open(IN tStreamParams* ptParams);
    // @cmember Close a transport stream.<nl>
    VAMPRET       Close();
    // @cmember Start a transport stream.<nl>
    VAMPRET       Start();
    // @cmember Stop a transport stream.<nl>
    // Parameterlist:<nl>
    // BOOL bRelease // TRUE: Release Channels<nl>
    VAMPRET       Stop(BOOL bRelease);
    // @cmember Get status info of Transport Stream<nl> 
    eStreamStatus   GetStatus();
    // @cmember Adds a new buffer to input queue.<nl>
    // Parameterlist:<nl>
    // CVampBuffer *Buffer // Buffer object (pointer) to add<nl>
    VAMPRET AddBuffer(
		IN CVampBuffer* pBuffer );            
	// @cmember Removes the buffer from a queue<nl>
    // Parameterlist:<nl>
    // CVampBuffer *pBuffer // Buffer object (pointer) to remove<nl>
    VAMPRET RemoveBuffer(
		IN CVampBuffer* pBuffer);
    // @cmember Returns a done buffer from a queue, or NULL if the list is
    // empty<nl>
    // Parameterlist:<nl>
    // CVampBuffer** ppBuffer // Buffer object (pointer to pointer) to get<nl>
    VAMPRET GetNextDoneBuffer(IN OUT CVampBuffer** ppBuffer);
    // @cmember Releases the last buffer from the queued list.
    //          This is the buffer which was added last. Returns
    //          VAMPRET_BUFFER_IN_PROGRESS if thebuffer is currently beeing
    //          filled or VAMPRET_BUFFER_NOT_AVAILABLE if the list is empty.
    VAMPRET ReleaseLastQueuedBuffer();
    // @cmember Moves all buffers from the empty list to the
    //          done list and marks the buffer with the status cancelled.
    VAMPRET CancelAllBuffers();
    // @cmember Reset a transport stream hardware interface.<nl>
    VAMPRET Reset();
    // @cmember Get the pin configuration.<nl>
    // Parameterlist:<nl>
    // PDWORD pdwPinConfig // pin configuration (pointer) to receive.<nl>
    VAMPRET GetPinsConfiguration(PDWORD pdwPinConfig);
    // @cmember Reads and returns the current status of this object
    DWORD GetObjectStatus();
// @access Private
private:
	// @access Private variables
    // @cmember Device Index.<nl>
    DWORD m_dwDeviceIndex; 
    // @cmember Represents the current status of this object
    DWORD m_dwObjectStatus;
	// @cmember Pointer to KM TransportStream object<nl>
    PVOID m_pKMTransportStream;
};

#endif // _TRANSPORTSTREAM__
