//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
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


#include "34avstrm.h"
#include "device.h"
#include "VampDevice.h"
#include "AnlgAudioOut.h"
#include "AnlgAudioOutInterface.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Constructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgAudioOut::CAnlgAudioOut()
{
    m_pVampAudioStream  = NULL;
    m_pIKsClock         = NULL;
    m_pSpinLockObj      = NULL;
    m_pKSPin            = NULL;
    m_dwDMASize         = 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Destructor.
//
//////////////////////////////////////////////////////////////////////////////
CAnlgAudioOut::~CAnlgAudioOut()
{
    if( m_pVampAudioStream )
    {
        delete m_pVampAudioStream;
        m_pVampAudioStream = NULL;
    }
    if( m_pIKsClock )
    {
        if( m_pIKsClock->Release() != 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: m_pIKsClock instances not zero"));
        }
        m_pIKsClock = NULL;
    }
    if( m_pSpinLockObj )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Creates the Audio stream object.
//  With the hardware interface (retrieved from the system,
//  stored in the Context of KSDevice by Add(), after DriverEntry()),
//  the audio stream is created. This creation (every access to data)
//  is managed in a synchronized way.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL             Operation failed,
//                                  (a) the function parameters are zero,
//                                  (b) hardware interface access failed
//  STATUS_INSUFFICIENT_RESOURCES   Operation failed,
//                                  (a) synchronization failed
//                                  (b) audio stream creation failed
//  STATUS_SUCCESS                  Creates the Audio stream object
//                                  with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Create
(
 IN PKSPIN pKSPin,   // Used for system support function calls.
 IN PIRP   pIrp      // Pointer to IRP for this request. (unused)
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Create"));

    // get device resources object from pKSPin
    CVampDeviceResources* pDevResObj = GetDeviceResource(pKSPin);
    if( !pDevResObj || !(pDevResObj->m_pAudioCtrl))
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    //
    // Audio HAL only supports one fixed buffer size.
    //
    DWORD dwDMASize = pDevResObj->m_pAudioCtrl->GetDMASize();
    if( dwDMASize == 0 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Device DMA size is zero"));
        return STATUS_UNSUCCESSFUL;
    }
    // Set member variables
    m_dwDMASize = dwDMASize;
    m_pKSPin    = pKSPin;

    // if pointer is valid check pointer pointed to by pDevResObj
    if( !(pDevResObj->m_pFactory) )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Cannot not get factory object"));
        return STATUS_UNSUCCESSFUL;
    }

    // create spin lock object
    m_pSpinLockObj = pDevResObj->m_pFactory->CreateSpinLock(NON_PAGED_POOL);
    if( !m_pSpinLockObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: CreateSpinLock failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    m_pVampAudioStream = new (NON_PAGED_POOL) CVampAudioStream(pDevResObj);
    if( !m_pVampAudioStream )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: CVampAudioStream failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        if( m_pVampAudioStream->GetObjectStatus() != STATUS_OK )
        {
            delete m_pVampAudioStream;
            m_pVampAudioStream = NULL;
            delete m_pSpinLockObj;
            m_pSpinLockObj = NULL;
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: Invalid CVampAudioStream object status"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Remove pin dispatch function. This function cleans up all, during
//  Create() allocated resources.
//  If existing, the audio stream and the synchronizing functionality are
//  not used any longer.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no audio stream object exists
//                          (d) no synchronization object exists
//  STATUS_SUCCESS          Remove pin dispatch function with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Remove
(
 IN PIRP pIrp        //Pointer to IRP for this request. (unused)
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Remove"));

    if( m_pVampAudioStream )
    {
        delete m_pVampAudioStream;
        m_pVampAudioStream = NULL;
    }

    if( m_pSpinLockObj )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Opens the audio stream object.
//  The 'open' is executed on the audio stream object.
//  The HAL internal 'Sample Rate Conversion' mechanism is to be disabled,
//  otherwise it collides with the MS conversion mechanism.
//  Sample rate is fixed to 32kHz only.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) the function parameters are zero,
//                          (b) hardware interface access failed
//                          (c) no audio stream object exists
//  STATUS_SUCCESS          Opened the audio stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Open
(
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Open"));

    //member variable valid?
    if( !m_pKSPin || !m_pVampAudioStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    tStreamParms tStreamParams;
    RtlZeroMemory( &tStreamParams, sizeof(tStreamParams) );

    tStreamParams.nField = EITHER_FIELD;
    tStreamParams.pCallbackFktn = dynamic_cast <CCallOnDPC*> (this);
    tStreamParams.pCBParam1  = m_pKSPin; // give Pin to Callback routine
    tStreamParams.pCBParam2  = NULL;
    tStreamParams.pTimeStamp = NULL;

    // disable GD layer completely
    if( m_pVampAudioStream->EnableRateAdjust(false) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: EnableRateAdjust failed"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pVampAudioStream->EnableCopy(false) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: EnableCopy failed"));
        return STATUS_UNSUCCESSFUL;
    }

    //open
    if( m_pVampAudioStream->Open(&tStreamParams) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Open audio stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    // Set rate -- device supports 32Khz only
    if( m_pVampAudioStream->SetRate( 32000) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: SetRate failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Closes the audio stream object.
//  The 'close' is executed on the audio stream object.
//  All stream related resources (vamp buffers) have to be released.
//  This release procedure has to be synchronized.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no audio stream object exists
//                              (c) release vamp buffers failed
//  STATUS_SUCCESS              Closed the audio stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Close
(
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Close"));

    CVampBuffer* pVampBuffer= NULL;

    if( !m_pVampAudioStream || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    //aquire spin lock to make sure that we are the only one
    //to access the buffer queue at the moment
    m_pSpinLockObj->AcquireSpinLock();
    //cancel all buffer that are still in the queue
    if( m_pVampAudioStream->CancelAllBuffers() != VAMPRET_SUCCESS )
    {
        //CancelAllBuffers failed
        m_pSpinLockObj->ReleaseSpinLock(); // void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CancelAllBuffers failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //Get all the buffer from the Vamp HAL
    while( m_pVampAudioStream->GetNextDoneBuffer(&pVampBuffer) ==
        VAMPRET_SUCCESS )
    {
        if( !pVampBuffer )
        {
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid buffer pointer"));
            return STATUS_UNSUCCESSFUL;
        }

        PKSSTREAM_POINTER pStreamPointer =
            static_cast <PKSSTREAM_POINTER> (pVampBuffer->GetContext());
        if( pStreamPointer )
        {
            //release system buffer
            KsStreamPointerDelete(pStreamPointer);
            //remove Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
        }
        else
        {
            //The streampointer within the buffer context is corrupted
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            //remove at least Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: (Buffer-)Stream pointer invalid!"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    //release spin lock to give others access to the buffer queue again
    m_pSpinLockObj->ReleaseSpinLock();

    //close audio stream
    if( m_pVampAudioStream->Close() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close audio stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Starts the audio stream.
//  The 'start' is executed on the audio stream object.
//  First the Vamp buffers (NUM_AUDIO_STREAM_BUFFER) are created
//  (using system buffer information/memory) and added to the stream.
//  Second the reference clock object is created here.
//  (The pin must have received notification of the master clock.)
//  The reference object is used to 'timestamp' the filled buffers.
//  Thirdly the stream is started.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no audio stream object exists
//                              (c) create reference clock object failed
//  STATUS_SUCCESS              Started the audio stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Start
(
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Start"));

    //member variables valid?
    if( !m_pKSPin || !m_pVampAudioStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    DWORD dwIndex = 0;

    do //while system buffers available
    {
        ntStatus = CAnlgAudioOut::Process();
        //increment buffer count index
        dwIndex++;
    }while( ntStatus == STATUS_SUCCESS );

    //check, if the number of inserted buffer is equal to
    //formerly demanded buffers
    if( dwIndex != NUM_AUDIO_STREAM_BUFFER )
    {
        // This is not a bug but there is a Microsoft restriction
        // where sometimes we dont get all the requested buffers.
        _DbgPrintF( DEBUGLVL_VERBOSE,
            ("Warning: Number of buffers do not match!"));
    }
    if( !m_pIKsClock )
    {
        //allocate reference clock, gets killed in "remove"
        if( !NT_SUCCESS(KsPinGetReferenceClockInterface(m_pKSPin,
            &m_pIKsClock)) )
        {
            // If we could not get an interface to the clock,
            // don't start the stream
            m_pIKsClock = NULL;
            _DbgPrintF(DEBUGLVL_BLAB,("Info: No clock object available"));
            // this was an error before, but if audio is used as a audio 
            // capture filter (with sample rate conversion and so forth)
            // we do not get the IKS clock, so we have to start without it
            // in that case
            // return STATUS_UNSUCCESSFUL;
        }
    }
    if( m_pVampAudioStream->Start() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Start audio stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  Stops the audio stream.
//  The 'stop' is executed on the audio stream object.
//  This function cleans up all, during Start() allocated resources.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no audio stream object exists
//                              (c) no reference clock object exists
//  STATUS_SUCCESS              Stopped the audio stream object with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Stop
(
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,("Info: CAnlgAudioOut::Stop"));

    //parameters valid?
    if( !m_pVampAudioStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }
    VAMPRET vrResult = m_pVampAudioStream->Stop(FALSE);
    // release the reference to the master clock
    // which was created during start
    if( m_pIKsClock )
    {
        if( m_pIKsClock->Release() != 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,
                ("Info: m_pIKsClock instances not zero"));
        }
        m_pIKsClock = NULL;
    }
    // check whether the stop function was successful or not
    if( vrResult != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop audio stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the callback function that is called after an interrupt in the
//  systems DPC level.
//  Get the next filled buffer from the hardware (in a synchronized way),
//  'timestamp' the buffer.
//  It attempts to initiate processing on Pin by sending a processing
//  dispatch call to Pin's processing object.
//  (asynchronous call to the Process function)
//  HAL access is performed by this function.
//
// Return Value:
//  This function is registered and called from within the system,
//  respectively VAMP RETURN values are used.
//  VAMPRET_NULL_POINTER_PARAMETER
//                              Operation failed
//                              (a) if pParam1 is NULL
//  VAMPRET_GENERAL_ERROR       Operation failed
//                              (a) Retrieved buffer pointer is NULL
//                              (b) Buffer address is NULL
//                                  (retrieved from buffer context)
//  VAMPRET_SUCCESS             Operation with success
//                              (a) if no more buffers are in the done queue
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET CAnlgAudioOut::CallOnDPC
(
 PVOID pStream,  //Pointer to stream object. (unused)
 PVOID pParam1,  //User defined parameter from the open function.
                 //Contains the associated KS pin in this case.
 PVOID pParam2   //User defined parameter from the open function.
                 //(unused)
 )
{
    //parameters valid?
    if( !pParam1 )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return VAMPRET_NULL_POINTER_PARAMETER;
    }

    CVampBuffer*        pVampBuffer     = NULL;
    PKSSTREAM_POINTER   pStreamPointer  = NULL;

    if( !m_pVampAudioStream || !m_pSpinLockObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }

    m_pSpinLockObj->AcquireSpinLock();

    //get all done vamp buffer out of our queue
    if( m_pVampAudioStream->GetNextDoneBuffer(&pVampBuffer) ==
        VAMPRET_SUCCESS )
    {
        m_pSpinLockObj->ReleaseSpinLock();

        if( !pVampBuffer )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: Invalid buffer (invalid buffer pointer)"));
            return VAMPRET_GENERAL_ERROR;
        }
        //get buffer address out of the vampbuffer context
        pStreamPointer =
            static_cast <PKSSTREAM_POINTER> (pVampBuffer->GetContext());
        if( !pStreamPointer )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: Invalid buffer (invalid streampointer)"));
            return VAMPRET_GENERAL_ERROR;
        }

        //set data context information
        pStreamPointer->StreamHeader->DataUsed = m_dwDMASize; // device DMA size
        pStreamPointer->StreamHeader->Duration =
            ( static_cast <LONGLONG> (pStreamPointer->StreamHeader->DataUsed) *
            10000000 ) / 128000; // 128000 = 32000*2*16 only
        pStreamPointer->StreamHeader->PresentationTime.Numerator = 1;
        pStreamPointer->StreamHeader->PresentationTime.Denominator = 1;

        if( m_pIKsClock )
        {
            pStreamPointer->StreamHeader->OptionsFlags =
                KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
            pStreamPointer->StreamHeader->PresentationTime.Time = 
                m_pIKsClock->GetTime();
        }
        else
        {
            pStreamPointer->StreamHeader->OptionsFlags =
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
            pStreamPointer->StreamHeader->PresentationTime.Time = 0;
        }

        //give buffer back to the system
        KsStreamPointerDelete(pStreamPointer);
        pStreamPointer = NULL;
        //delete Vamp buffer object
        delete pVampBuffer;
        pVampBuffer = NULL;
        //Make an asynchronous system call to the process function to insert
        //a new buffer into the queue. We cannot call Process here directly,
        //because we are still in the systems DPC context.
        KsPinAttemptProcessing(static_cast <PKSPIN> (pParam1), TRUE);

        m_pSpinLockObj->AcquireSpinLock(); // void function
    }

    m_pSpinLockObj->ReleaseSpinLock(); // void function

    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  It attempts to initiate processing on Pin.
//  A Vamp buffer is created (using system buffer information/memory)
//  and added to the stream.
//  HAL access is performed by this function.
//
//  Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed,
//                              (a) the function parameters are zero,
//                              (b) no 'next available system buffer'
//                              (c) couldn't clone the system buffer
//                              (d) create Vamp buffer object failed
//                              (e) couldn't add buffer to the stream
//  STATUS_SUCCESS              Process with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CAnlgAudioOut::Process
(
 )
{
    if( !m_pSpinLockObj || !m_pVampAudioStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    //get next available system buffer
    PKSSTREAM_POINTER pStreamPointer =
        KsPinGetLeadingEdgeStreamPointer(m_pKSPin,
        KSSTREAM_POINTER_STATE_LOCKED);

    if( !pStreamPointer )
    {
        //no system buffer available
        //This case can happen if it is the last pointer in the queue or
        //the system cannot give us the required buffer
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: No system buffer available"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        PKSSTREAM_POINTER pStreamPointerClone = NULL;
        CVampBuffer* pVampBuffer = NULL;
        //clone the system buffer
        ntStatus = KsStreamPointerClone(pStreamPointer,
            NULL,
            NULL,
            &pStreamPointerClone);
        if( !NT_SUCCESS(ntStatus) )
        {
            //no system buffer available
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: Streampointer cloning unsuccessful"));
            return STATUS_UNSUCCESSFUL;
        }
        //set vamp buffer format parameters
        tBufferFormat tBufferParams;
        RtlZeroMemory( &tBufferParams, sizeof(tBufferParams) );

        //set bits per samples
        tBufferParams.dwChannelsPerSample = 2; // stereo only
        //set bits per samples
        tBufferParams.dwBitsPerSample = 16; // 16 bit only
        //set samples per line
        //samples per line = device DMA size /
        // (channels per sample * bytes per sample)
        tBufferParams.dwSamplesPerLine = m_dwDMASize / 4; // 4 = 2*16/8bit only
        //set number of lines
        tBufferParams.dwNrOfLines = 1;
        //calculate pitch in bytes
        tBufferParams.lPitchInBytes = 
            static_cast <LONG> (m_dwDMASize); // device DMA size (in byte)
        //calculate the buffer length
        tBufferParams.lBufferLength = 
            static_cast <LONG> (m_dwDMASize); // device DMA size (in byte)
        //dont change the byte order
        tBufferParams.ByteOrder = BO_1234;
        //set buffer queued
        tBufferParams.BufferType = CVAMP_BUFFER_QUEUED;

        //is the buffer size correct?
        if( pStreamPointerClone->StreamHeader->FrameExtent <
            (static_cast <DWORD>(tBufferParams.lBufferLength)) )
        {
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer size incorrect"));
            return STATUS_UNSUCCESSFUL;
        }
        //construct vamp buffer object
        pVampBuffer = new (NON_PAGED_POOL) CVampBuffer(
            pStreamPointerClone->StreamHeader->Data,
            pStreamPointerClone->StreamHeader->FrameExtent,
            (PVOID) pStreamPointerClone->Offset,
            &tBufferParams,
            BUFFER_REGULAR_MAPPING_OS,
            false,
            false);
        if( !pVampBuffer )
        {
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: VampBuffer creation failed, not enough memory"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        if( pVampBuffer->GetObjectStatus() != STATUS_OK )
        {
            //de-allocate memory for the Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone);
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer object status failed"));
            return STATUS_UNSUCCESSFUL;
        }
        //set KS stream pointer into the buffer context for use in the
        //CallOnDPC function
        pVampBuffer->SetContext(static_cast <PVOID> (pStreamPointerClone));
        //aquire spin lock to be the only one that has access to the buffer queue
        m_pSpinLockObj->AcquireSpinLock();
        //add buffer to Vamp dma queue
        if( m_pVampAudioStream->AddBuffer(pVampBuffer) != VAMPRET_SUCCESS )
        {
            //de-allocate memory for the Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone);
            m_pSpinLockObj->ReleaseSpinLock();
            _DbgPrintF(DEBUGLVL_ERROR,("Error: AddBuffer to audio stream failed"));
            return STATUS_UNSUCCESSFUL;
        }
        m_pSpinLockObj->ReleaseSpinLock();
        //advance stream pointer to the next available data frame
        ntStatus = KsStreamPointerAdvance(pStreamPointer);

        if( (ntStatus != STATUS_DEVICE_NOT_READY) &&
            (ntStatus != STATUS_SUCCESS) )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                ("Error: Audio Streampointer advacement failed"));
        }

    }
    return ntStatus;
}

