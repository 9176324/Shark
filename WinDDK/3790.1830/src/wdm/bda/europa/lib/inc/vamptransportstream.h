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
// @module   VampTransportStream | This file describes the User Interface of
//           the Transport Stream class. The Transport Stream is a GPIO input
//           stream for digital TV. It can be a parallel stream or a serial
//           stream.
// @comm     It is assumed that the source type for the appropriate stream
//           type is stored in the registry and can not be changed
//           dynamically.
// @end
//
// Filename: VampTransportStream.h
//
// Routines/Functions:
//
//  public:
//      CVampTransportStream()
//      ~CVampTransportStream();
//      Open(IN tStreamParams StreamParams);
//      Close();
//      Start();
//      Stop();
//      GetStatus();
//      AddBuffer(IN CVampBuffer* pBuffer );
//      RemoveBuffer(IN CVampBuffer *Buffer );
//      GetNextDoneBuffer(IN OUT CVampBuffer **pBuffer );
//      ReleaseLastQueuedBuffer();
//      CancelAllBuffers();
//      Reset();
//      GetPinsConfiguration();
//      GetObjectStatus();
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////
#ifndef TRANSPORTSTREAM__
#define TRANSPORTSTREAM__

#include "VampGPIO.h"
#include "VampBuffer.h"
#include "AbstractStream.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc    INTERNAL EXTERNAL
// @class  CVampTransportStream | This class is the user interface for the
//         transport stream.
// @base public | CAbstractStream
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVampTransportStream : public CAbstractStream
{
// @access private 
private:

    // @cmember Pointer to the device resources object
    CVampDeviceResources *m_pVampDevRes;  

    // @cmember Pointer to the core stream object for the GPIO input stream
    CVampCoreStream *m_pVampCoreStream;

	// @cmember Buffer header object for overflow area. Created and deleted in
    // Open and Close methods.
    CVampBuffer* m_pOverflowBuffer;

    // @cmember Current stream status
    eStreamStatus m_eStreamStatus;  
    
    // @cmember Current stream type
	eTSStreamType m_nStreamType;
    
    // @cmember Channel request flag
    DWORD m_dwChannelRequest; 

	// @cmember Error status flag, will be set during construction.
    DWORD m_dwObjectStatus;

// @access public
public:
    // @cmember The constructor has to read the registry in order
    //to get information about the transport stream. These
    //informations are static and will never be changed
    //dynamically.<nl>
    //Parameterlist:<nl>
    //CVampDeviceResources *pDevRes // pointer on device resources object<nl>
    //eTSStreamType nStreamType // transport stream type<nl>
    CVampTransportStream(
                CVampDeviceResources* pDevRes,
                eTSStreamType nStreamType );

	//@cmember Returns the error status set by constructor.<nl>
    DWORD GetObjectStatus()
    {
        return m_dwObjectStatus;
    }

    // @cmember Destructor.<nl>
    virtual ~CVampTransportStream();

    // @cmember Open transport stream.<nl>
    //Parameterlist:
    //tStreamParams *p_Params // pointer to stream parameters<nl>
    VAMPRET Open(
        IN tStreamParms *pStreamParms );

    // @cmember Close transport stream.<nl>
    VAMPRET Close();

    // @cmember Start transport stream. If the return value is
    //VAMPRET_GPIO_CONFIG_ERR, you can call the function
    //GetPinsConfiguration() to have a look, how the GPIO pins are
    //specified in the EEPROM/Bootstrap. <nl>
    VAMPRET Start();

    // @cmember Stop transport stream.<nl>
    //Parameterlist:<nl>
    //BOOL bRelease // if TRUE: release Channels<nl>
    VAMPRET Stop(
        IN BOOL bRelease );

    // @cmember Return the current status of the stream.<nl>
    eStreamStatus   GetStatus();
	
    // @cmember Add a new buffer to the core stream input queue.<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to add<nl>
    VAMPRET AddBuffer(
        IN CVampBuffer* pBuffer );

    // @cmember Removes the buffer from a queue<nl>
    //Parameterlist:<nl>
    //CVampBuffer *Buffer // pointer on buffer object to remove<nl>
    VAMPRET RemoveBuffer(
        IN CVampBuffer *Buffer );

    //@cmember Returns a done buffer from the core stream queue, or NULL if the
    //list is empty<nl>
    //Parameterlist:<nl>
    //CVampBuffer **pBuffer // pointer on buffer object to remove<nl>
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

    // @cmember Reset transport stream hardware interface.<nl>
    VAMPRET Reset();

	//@cmember  Adjusts the clock, since between open and start might be a big
    //time difference.<nl>
    VOID AdjustClock(); 

    // @cmember Returns the EEPROM/Bootstrap information about the GPIO pins.
    //The configuration of each pin is in the appropriated bit. One
    //means InputOnly and zero means InputAndOutput.<nl>
	VAMPRET GetPinsConfiguration(
        DWORD* pdwPinsConfig );

};

#endif //TRANSPORTSTREAM__
