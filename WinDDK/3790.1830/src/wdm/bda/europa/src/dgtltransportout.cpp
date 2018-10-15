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


#include "34AVStrm.h"
#include "VampDevice.h"
#include "DgtlTransportOutInterface.h"
#include "DgtlTransportOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The constructor of the transport output pin initializes some
//  class internal member variables.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTransportOut::CDgtlTransportOut()
{
    m_pVampTSStream     = NULL;
    m_pSpinLockObj      = NULL;
    m_pIKsClock         = NULL;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The destructor of the transport output pin.
//  Nothing has to be done here.
//
// Return Value:
//  None.
//
//////////////////////////////////////////////////////////////////////////////
CDgtlTransportOut::~CDgtlTransportOut()
{
    if( m_pVampTSStream )
    {
        delete m_pVampTSStream;
        m_pVampTSStream = NULL;
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
//  The create pin is called when the transport output pin is created. It
//  creates a spin lock from the 34 HAL and initializes a new transport
//  stream object.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_INSUFFICIENT_RESOURCES    Unable to create class
//                                   CVampTransportStream
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Create
(
    IN PKSPIN pKSPin,       // Pointer to the pin object
    IN PIRP pIrp            // Pointer to the Irp that is involved with this
                            // operation.
)
{
    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    // get device resources object from pKSPin
    CVampDeviceResources* pDevResObj = GetDeviceResource(pKSPin);
    if( !pDevResObj )
    {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: Cannot get device resource object"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        // if pointer is valid check pointer pointed to by pDevResObj
        if( !(pDevResObj->m_pFactory) )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Cannot not get factory object"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    m_pSpinLockObj = pDevResObj->m_pFactory->CreateSpinLock(NON_PAGED_POOL);
    if( !m_pSpinLockObj )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: CreateSpinLock failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    m_pVampTSStream = new (NON_PAGED_POOL)
                            CVampTransportStream(pDevResObj, TS_Parallel);
    if( !m_pVampTSStream )
    {
        delete m_pSpinLockObj;
        m_pSpinLockObj = NULL;
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("Error: CVampTransportStream failed, not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        if( m_pVampTSStream->GetObjectStatus() != STATUS_OK )
        {
            delete m_pVampTSStream;
            m_pVampTSStream = NULL;
            delete m_pSpinLockObj;
            m_pSpinLockObj = NULL;
            _DbgPrintF(
                DEBUGLVL_ERROR,
                ("Error: VampTransportStream object status invalid"));
            return STATUS_UNSUCCESSFUL;
        }
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  The remove function is called when the transport output pin is removed.
//  It deletes the CVampTransportStream class object and the spin lock.
//  HAL access is performed by this function.
//
// Return Value:
//  This functions always returns with STATUS_SUCCESS
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Remove
(
    IN PKSPIN pKSPin,       // Pointer to the pin object
    IN PIRP pIrp            // Pointer to the Irp that is involved with this
                            // operation.
)
{
    if( m_pVampTSStream )
    {
        delete m_pVampTSStream;
        m_pVampTSStream = NULL;
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
//  This function opens a new stream at the transport stream object class.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Open
(
    IN PKSPIN pKSPin       // Pointer to the pin object
)
{
    //parameters valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pVampTSStream )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
                  ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    tStreamParms tStreamParams;
    RtlZeroMemory( &tStreamParams, sizeof(tStreamParams) );

    tStreamParams.nField        = EITHER_FIELD;
    tStreamParams.pCallbackFktn = dynamic_cast <CCallOnDPC*>(this);
    tStreamParams.pCBParam1     = pKSPin; // give Pin to Callback routine
    tStreamParams.pCBParam2     = NULL;
    tStreamParams.pTimeStamp    = NULL;
    //open
    if( m_pVampTSStream->Open(&tStreamParams) != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Open TS stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function closes the current stream at the transport stream object
//  class and de-allocates the memory of the stream buffer.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL         Operation failed due to invalid arguments.
//  STATUS_SUCCESS              Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Close
(
    IN PKSPIN pKSPin       // Pointer to the pin object
)
{
    CVampBuffer* pVampBuffer = NULL;

    if( !m_pVampTSStream || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    //aquire spin lock to make sure that we are the only one
    //to access the buffer queue at the moment
    m_pSpinLockObj->AcquireSpinLock(); // void function
    //cancel all buffer that are still in the queue
    if( m_pVampTSStream->CancelAllBuffers() != VAMPRET_SUCCESS )
    {
        //CancelAllBuffers failed
        m_pSpinLockObj->ReleaseSpinLock(); // void function
        _DbgPrintF(DEBUGLVL_ERROR,("Error: CancelAllBuffers failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //Get all the buffer from the Vamp HAL
    while(m_pVampTSStream->GetNextDoneBuffer(&pVampBuffer) ==
          VAMPRET_SUCCESS)
    {
        if( !pVampBuffer )
        {
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid buffer pointer"));
            return STATUS_UNSUCCESSFUL;
        }

        PKSSTREAM_POINTER pStreamPointer =
            static_cast <PKSSTREAM_POINTER>(pVampBuffer->GetContext());
        if( pStreamPointer )
        {
            //release system buffer
            KsStreamPointerDelete(pStreamPointer); // void function
            pStreamPointer = NULL;
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
    m_pSpinLockObj->ReleaseSpinLock(); // void function

    //close TS stream
    if( m_pVampTSStream->Close() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Close TS stream failed"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function starts the streaming on the transport output pin.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Start
(
    IN PKSPIN pKSPin       // Pointer to the pin object
)
{
    //parameters and clock object valid?
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pVampTSStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    DWORD dwIndex = 0;

    do //while system buffers available
    {
        ntStatus = Process(pKSPin);
        dwIndex++;
    }
    while(ntStatus == STATUS_SUCCESS);

    //check, if the number of inserted buffer is equal to
    //formerly demanded buffers
    if( dwIndex != NUM_TS_STREAM_BUFFER )
    {
        // This is not a bug but there is a Microsoft restriction
        // where sometimes we dont get all the requested buffers.
        _DbgPrintF( DEBUGLVL_VERBOSE,
                    ("Warning: Number of buffers do not match!"));
    }

    // try to get the reference to the master clock object
    if( !m_pIKsClock )
    {
        if( !NT_SUCCESS(KsPinGetReferenceClockInterface(pKSPin,
                                                        &m_pIKsClock)) )
        {
            // If we could not get an interface to the clock,
            // don't proceed
            m_pIKsClock = NULL;
            _DbgPrintF(DEBUGLVL_ERROR,("Error: No clock object available"));
            return STATUS_UNSUCCESSFUL;
        }
    }

    if(m_pVampTSStream->Reset() != VAMPRET_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Unable to reset hardware TS Interface"));
        return STATUS_UNSUCCESSFUL;
    }

    if( m_pVampTSStream->Start() != VAMPRET_SUCCESS )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Start transport stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function stops the streaming on the transport output pin.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Stop
(
    IN PKSPIN pKSPin       // Pointer to the pin object
)
{
    if( !m_pVampTSStream )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }
	VAMPRET vrResult = m_pVampTSStream->Stop(FALSE);
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
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Stop TS stream failed"));
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the DPC call back function of the transport stream.
//
// Return Value:
//  This function always returns VAMPRET_SUCCESS.
//
//////////////////////////////////////////////////////////////////////////////
VAMPRET CDgtlTransportOut::CallOnDPC
(
    PVOID pStream,      // Pointer to the stream the DPC function was
                        // called by.
    PVOID pParam1,      // Pointer to a additional parameter.
    PVOID pParam2       // Pointer to a additional parameter.
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

    if( !m_pVampTSStream || !m_pIKsClock || !m_pSpinLockObj )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return VAMPRET_GENERAL_ERROR;
    }

    m_pSpinLockObj->AcquireSpinLock(); // void function

    //get all done vamp buffer out of our queue
    if( m_pVampTSStream->GetNextDoneBuffer(&pVampBuffer) ==
            VAMPRET_SUCCESS )
    {
        m_pSpinLockObj->ReleaseSpinLock(); // void function

        if( !pVampBuffer )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Invalid buffer (invalid buffer pointer)"));
            return VAMPRET_GENERAL_ERROR;
        }
        //get buffer address out of the vampbuffer context
        pStreamPointer =
                    static_cast <PKSSTREAM_POINTER>(pVampBuffer->GetContext());
        if( !pStreamPointer )
        {
            _DbgPrintF( DEBUGLVL_BLAB,
                        ("Info: Invalid buffer (invalid streampointer)"));
            return VAMPRET_GENERAL_ERROR;
        }
        //set data context information
        pStreamPointer->StreamHeader->OptionsFlags =
                                            KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;

        pStreamPointer->StreamHeader->DataUsed = M2T_BUFFERSIZE;
        pStreamPointer->StreamHeader->Duration = 242000; // 24.2 mS per 8VSB field
        pStreamPointer->StreamHeader->PresentationTime.Numerator = 1;
        pStreamPointer->StreamHeader->PresentationTime.Denominator = 1;
        pStreamPointer->StreamHeader->OptionsFlags |=
            KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
            KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;
        pStreamPointer->StreamHeader->PresentationTime.Time = m_pIKsClock->GetTime();

        //give buffer back to the system
        KsStreamPointerDelete(pStreamPointer); // void function
        pStreamPointer = NULL;
        //delete Vamp buffer object
        delete pVampBuffer;
        pVampBuffer = NULL;

        //Make an asynchronous system call to the process function to insert
        //a new buffer into the queue. We cannot call Process here directly,
        //because we are still in the systems DPC context.
        KsPinAttemptProcessing(static_cast <PKSPIN>(pParam1), TRUE); // void function

        m_pSpinLockObj->AcquireSpinLock(); // void function
    }

    m_pSpinLockObj->ReleaseSpinLock(); // void function

    return VAMPRET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This function aquires a new buffer from the system, prepares it to meet
//  the requirements for SAA7134 TS streaming and passses it to the HAL.
//  HAL access is performed by this function.
//
// Return Value:
//  STATUS_UNSUCCESSFUL              Operation failed due to invalid
//                                   arguments.
//  STATUS_SUCCESS                   Operation succeeded.
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS CDgtlTransportOut::Process
(
    IN PKSPIN pKSPin // Pointer to the pin object.
)
{
    if( !pKSPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !m_pSpinLockObj || !m_pVampTSStream )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Important member not initialized"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PKSSTREAM_POINTER pStreamPointer =
        KsPinGetLeadingEdgeStreamPointer(   pKSPin,
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
            _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Streampointer cloning unsuccessful"));
            return STATUS_UNSUCCESSFUL;
        }
        //set vamp buffer format parameters
        tBufferFormat tBufferParams;
        RtlZeroMemory( &tBufferParams, sizeof(tBufferParams) ); // void function

        //set samples per line, should be 188
        tBufferParams.dwSamplesPerLine = M2T_BYTES_IN_LINE;
        //set default value (used for audio only)
        tBufferParams.dwChannelsPerSample = 1;
        //Y only format (grey) has one byte per pixel
        tBufferParams.dwBitsPerSample = 8;
        //set number of lines, should be 312
        tBufferParams.dwNrOfLines = M2T_LINES_IN_FIELD;
        //calculate pitch in bytes
        tBufferParams.lPitchInBytes =
            tBufferParams.dwSamplesPerLine * tBufferParams.dwBitsPerSample/8;
        //calculate the buffer length
        tBufferParams.lBufferLength =
            tBufferParams.lPitchInBytes * tBufferParams.dwNrOfLines;
        //dont change the byte order
        tBufferParams.ByteOrder = KEEP_AS_IS;
        //set buffer queued
        tBufferParams.BufferType = CVAMP_BUFFER_QUEUED;

        //is the buffer size correct?
        if( pStreamPointerClone->StreamHeader->FrameExtent <
            (static_cast <DWORD>(tBufferParams.lBufferLength)) )
        {
            //buffer size incorrect
            KsStreamPointerDelete(pStreamPointerClone); // void function
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
            KsStreamPointerDelete(pStreamPointerClone); // void function
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
            KsStreamPointerDelete(pStreamPointerClone); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer object status failed"));
            return STATUS_UNSUCCESSFUL;
        }

        //set KS stream pointer into the buffer context for use in the
        //CallOnDPC function
        pVampBuffer->SetContext(static_cast <PVOID>(pStreamPointerClone));
        //aquire spin lock to be the only one that has access to the buffer queue
        m_pSpinLockObj->AcquireSpinLock(); // void function
        //add buffer to Vamp dma queue
        if( m_pVampTSStream->AddBuffer(pVampBuffer) != VAMPRET_SUCCESS )
        {
            //de-allocate memory for the Vamp buffer object
            delete pVampBuffer;
            pVampBuffer = NULL;
            //give buffer back to the system, we cannot use it for some reason
            KsStreamPointerDelete(pStreamPointerClone); // void function
            m_pSpinLockObj->ReleaseSpinLock(); // void function
            _DbgPrintF(DEBUGLVL_ERROR,("Error: AddBuffer to TS stream failed"));
            return STATUS_UNSUCCESSFUL;
        }
        //release the spin lock to give others access to the buffer queue again
        m_pSpinLockObj->ReleaseSpinLock(); // void function
        //advance stream pointer to the next available data frame
        ntStatus = KsStreamPointerAdvance(pStreamPointer);

        if( (ntStatus != STATUS_DEVICE_NOT_READY) &&
            (ntStatus != STATUS_SUCCESS) )
        {
            _DbgPrintF( DEBUGLVL_ERROR,
                        ("Error: Video capture Streampointer advacement failed"));
        }
    }
    return ntStatus;
}

