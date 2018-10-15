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


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Interface functions for our class environment, to support C interface
//  from Microsoft. The following functions are all global, so they have to
//  have a unique name and must never use any global/static resources
//  to avoid multiboard problems.
//
//////////////////////////////////////////////////////////////////////////////

#include "34avstrm.h"
#include "device.h"
#include "AnlgAudioOutInterface.h"
#include "AnlgAudioOut.h"

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the SetDataFormat dispatch function of the analog audio out pin.
//  If called the first time, it creates an analog audio out pin object
//  and stores the analog audio out pin pointer in the pin context.
//  If not called the first time, it retrieves the analog audio out pin
//  pointer from the pin context.
//  It also calls the setting of the data format on the instance
//  of the analog audio out pin.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed,
//                       (a) if the function parameters are zero,
//                       (b) if the analog audio out pin pointer is zero
//                       (c) SetDataFormat failed
//  STATUS_INSUFFICIENT_RESOURCES
//                       Operation failed,
//                       if the analog audio out pin could't be created
//  STATUS_INVALID_DEVICE_STATE
//                       SetDataFormat on an audio out pin Operation failed,
//                       if the Pin DeviceState is not KSSTATE_STOP
//  STATUS_SUCCESS       SetDataFormat on an audio out pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgAudioOutPinSetDataFormat
(
 IN PKSPIN pKSPin,                                      //Pointer to KSPIN
 IN OPTIONAL PKSDATAFORMAT pKSOldFormat,                //Pointer to previous format
                                                        //structure, is 0 during the
                                                        //first call.
 IN OPTIONAL PKSMULTIPLE_ITEM pKSOldAttributeList,      //Previous attributes.
 IN const KSDATARANGE* pKSDataRange,                    //Pointer to matching data range
 IN OPTIONAL const KSATTRIBUTE_LIST* pKSAttributeRange  //New attributes.
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgAudioOutPinSetDataFormat called"));

    if (!(IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) ||
        pKSPin->ConnectionFormat->FormatSize < sizeof(KSDATAFORMAT_WAVEFORMATEX)) 
    {
        return STATUS_UNSUCCESSFUL;
    }

    // Test the device state. We allow format changes only in STOP case.
    if( pKSPin->DeviceState != KSSTATE_STOP )
    {
        // We don't accept dynamic format changes
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid device state"));
        return STATUS_INVALID_DEVICE_STATE;
    }

    PKSDATAFORMAT_WAVEFORMATEX pNewFormat = (PKSDATAFORMAT_WAVEFORMATEX) pKSPin->ConnectionFormat;
    if ((pNewFormat->WaveFormatEx.cbSize + sizeof(KSDATAFORMAT_WAVEFORMATEX)) != 
        pNewFormat->DataFormat.FormatSize) 
    {
        return STATUS_UNSUCCESSFUL;
    }

    
    if (pNewFormat->WaveFormatEx.nSamplesPerSec != EUROPA_SAMPLE_FREQUENCY) 
    {
        return STATUS_UNSUCCESSFUL;
    }

    if( !NT_SUCCESS(KsEdit (pKSPin, &pKSPin->Descriptor, 'pmaV')) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: KsEdit failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //if the edits proceeded without running out of memory, adjust
    //the framing based on the audio info header.
    if( !NT_SUCCESS(KsEdit(pKSPin,
        &(pKSPin->Descriptor->AllocatorFraming),'pmaV')) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: 2nd KsEdit failed"));
        return STATUS_UNSUCCESSFUL;
    }
    // We've KsEdit'ed this...  I'm safe to cast away constness as
    // long as the edit succeeded.
    PKSALLOCATOR_FRAMING_EX Framing =
        const_cast <PKSALLOCATOR_FRAMING_EX>
        (pKSPin->Descriptor->AllocatorFraming);

    Framing -> FramingItem [0].Frames = NUM_AUDIO_STREAM_BUFFER;
    // get device resources object from pKSPin
    CVampDeviceResources* pDevResObj = GetVampDevResObj(pKSPin);
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

    Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
        Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
        Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
        Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
        dwDMASize;

    Framing -> FramingItem [0].PhysicalRange.Stepping =
        Framing -> FramingItem [0].FramingRange.Range.Stepping =
        0;

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the handler for comparison of a data range.
//  The AudioIntersectDataFormat is the IntersectHandler function
//  of the analog audio out pin.(Needs no class environment to intersect
//  the format, because the format that is supported is already
//  given in the filter descriptor [ANLG_AUDIO_OUT_PIN_DESCRIPTOR]).
// Settings:
//  A match can occur under three conditions:
//  (1) if the major format of the range passed is a wildcard
//      or matches a pin factory range,
//  (2) if the sub-format is a wildcard or matches,
//  (3) if the specifier is a wildcard or matches.
//  Since the data range size may be variable, it is not validated beyond
//  checking that it is at least the size of a data range structure.
//  (KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
//
//  Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          if the function parameters are zero,
//  STATUS_NO_MATCH         Operation failed,
//                          if no matching range was found
//  STATUS_BUFFER_OVERFLOW  Operation failed,
//                          if buffer size is zero
//  STATUS_BUFFER_TOO_SMALL Operation failed,
//                          if buffer size is too small
//  STATUS_SUCCESS          AudioIntersectDataFormat with success
//                          if a matching range is found
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AudioIntersectDataFormat
(
 IN PKSFILTER pKSFilter,                 //Pointer to KS filter structure.
 IN PIRP pIrp,                           //Pointer to data intersection
                                         //property request.
 IN PKSP_PIN pPinInstance,               //Pointer to structure indicating
                                         //pin in question.
 IN PKSDATARANGE pCallerDataRange,       //Pointer to the caller data
                                         //structure that should be
                                         //intersected.
 IN PKSDATARANGE pDescriptorDataRange,   //Pointer to the descriptor data
                                         //structure, see AnlgAudioFormats.h.
 IN DWORD dwBufferSize,                  //Size in bytes of the target
                                         //buffer structure. For size
                                         //queries, this value will be zero.
 OUT OPTIONAL PVOID pData,               //Pointer to the target data
                                         //structure representing the best
                                         //format in the intersection.
 OUT PDWORD pdwDataSize                  //Pointer to size of target data
                                         //format structure.
 )
{
    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AudioIntersectHandler called"));

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    if( !pKSFilter || !pIrp || !pPinInstance ||
        !pCallerDataRange || !pDescriptorDataRange || !pdwDataSize)
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }


    if (IsEqualGUID(pCallerDataRange->Specifier, 
        KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) &&
        (pCallerDataRange->FormatSize >= sizeof(KSDATARANGE_AUDIO))) 
    {

        //*** start intersection ***//


        PKSDATARANGE_AUDIO pDescriptorAudioDataRange =
            reinterpret_cast <PKSDATARANGE_AUDIO> (pDescriptorDataRange);

        PKSDATARANGE_AUDIO pCallerAudioDataRange =
            reinterpret_cast <PKSDATARANGE_AUDIO>(pCallerDataRange);


        if( (pCallerAudioDataRange->MaximumChannels <
            pDescriptorAudioDataRange->MaximumChannels)        ||
            (pCallerAudioDataRange->MaximumSampleFrequency <
            pDescriptorAudioDataRange->MinimumSampleFrequency) ||
            (pCallerAudioDataRange->MinimumSampleFrequency >
            pDescriptorAudioDataRange->MaximumSampleFrequency) ||
            (pCallerAudioDataRange->MaximumBitsPerSample <
            pDescriptorAudioDataRange->MinimumBitsPerSample)   ||
            (pCallerAudioDataRange->MinimumBitsPerSample >
            pDescriptorAudioDataRange->MaximumBitsPerSample) )
        {

            _DbgPrintF(DEBUGLVL_ERROR,
                ("Warning: [PinIntersectHandler]  STATUS_NO_MATCH"));
            return STATUS_NO_MATCH;
        }

        // return the size
        *pdwDataSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);

        if( dwBufferSize == 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,("Info: Size only query"));
            return STATUS_BUFFER_OVERFLOW;
        }

        if( dwBufferSize < sizeof(*pdwDataSize) )
        {
            //
            // Buffer is too small.
            //
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer too small"));
            return STATUS_BUFFER_TOO_SMALL;
        }

        KSDATAFORMAT_WAVEFORMATEX* pAudioDataFormat = 
            static_cast <PKSDATAFORMAT_WAVEFORMATEX>(pData);

        if( pAudioDataFormat == 0 )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: [PinIntersectHandler]  STATUS_BUFFER_TOO_SMALL"));
            return STATUS_BUFFER_TOO_SMALL;
        }

        //
        // All the guids are in the descriptor's data range.
        //
        RtlCopyMemory(
            &pAudioDataFormat->DataFormat,
            pDescriptorDataRange,
            sizeof(pAudioDataFormat->DataFormat));

        pAudioDataFormat->DataFormat.FormatSize = sizeof(*pAudioDataFormat);

        pAudioDataFormat->WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;

        WORD wMaxChannels1 = static_cast <WORD> (pDescriptorAudioDataRange->MaximumChannels);
        WORD wMaxChannels2 = static_cast <WORD> (pCallerAudioDataRange->MaximumChannels);
        pAudioDataFormat->WaveFormatEx.nChannels = 
            ((wMaxChannels1 <= wMaxChannels2) ? wMaxChannels1 : wMaxChannels2);

        WORD wMaxSampleFrequency1 = static_cast <WORD> (pDescriptorAudioDataRange->MaximumSampleFrequency);
        WORD wMaxSampleFrequency2 = static_cast <WORD> (pCallerAudioDataRange->MaximumSampleFrequency);
        pAudioDataFormat->WaveFormatEx.nSamplesPerSec =
            ((wMaxSampleFrequency1 <= wMaxSampleFrequency2) ? wMaxSampleFrequency1 : wMaxSampleFrequency2);

        WORD wBitsPerSample1 = static_cast <WORD> (pDescriptorAudioDataRange->MaximumBitsPerSample);
        WORD wBitsPerSample2 = static_cast <WORD> (pCallerAudioDataRange->MaximumBitsPerSample);
        pAudioDataFormat->WaveFormatEx.wBitsPerSample = 
            ((wBitsPerSample1 <= wBitsPerSample2) ? wBitsPerSample1 : wBitsPerSample2);

        pAudioDataFormat->WaveFormatEx.nBlockAlign = 
            (pAudioDataFormat->WaveFormatEx.wBitsPerSample *
            pAudioDataFormat->WaveFormatEx.nChannels) / 8 ;

        pAudioDataFormat->WaveFormatEx.nAvgBytesPerSec =
            pAudioDataFormat->WaveFormatEx.nBlockAlign *
            pAudioDataFormat->WaveFormatEx.nSamplesPerSec;

        pAudioDataFormat->WaveFormatEx.cbSize = 0;

        pAudioDataFormat->DataFormat.SampleSize = pAudioDataFormat->WaveFormatEx.nBlockAlign;
        Status = STATUS_SUCCESS;

    } else if (IsEqualGUID(pCallerDataRange->Specifier, 
        KSDATAFORMAT_SPECIFIER_WILDCARD))
    {


        //*** start intersection ***//


        PKSDATARANGE_AUDIO pDescriptorAudioDataRange =
            reinterpret_cast <PKSDATARANGE_AUDIO> (pDescriptorDataRange);

        // return the size
        *pdwDataSize = sizeof(KSDATAFORMAT_WAVEFORMATEX);

        if( dwBufferSize == 0 )
        {
            _DbgPrintF(DEBUGLVL_BLAB,("Info: Size only query"));
            return STATUS_BUFFER_OVERFLOW;
        }

        if( dwBufferSize < sizeof(*pdwDataSize) )
        {
            //
            // Buffer is too small.
            //
            _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer too small"));
            return STATUS_BUFFER_TOO_SMALL;
        }


        KSDATAFORMAT_WAVEFORMATEX* pAudioDataFormat = 
            static_cast <PKSDATAFORMAT_WAVEFORMATEX>(pData);

        if( pAudioDataFormat == 0 )
        {
            _DbgPrintF(DEBUGLVL_ERROR,
                ("Error: [PinIntersectHandler]  STATUS_BUFFER_TOO_SMALL"));
            return STATUS_BUFFER_TOO_SMALL;
        }

        //
        // All the guids are in the descriptor's data range.
        //
        RtlCopyMemory(
            &pAudioDataFormat->DataFormat,
            pDescriptorDataRange,
            sizeof(pAudioDataFormat->DataFormat));

        pAudioDataFormat->DataFormat.FormatSize = sizeof(*pAudioDataFormat);

        pAudioDataFormat->WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;

        pAudioDataFormat->WaveFormatEx.nChannels = 
            static_cast <WORD> (pDescriptorAudioDataRange->MaximumChannels);

        pAudioDataFormat->WaveFormatEx.nSamplesPerSec =
            pDescriptorAudioDataRange->MaximumSampleFrequency;

        pAudioDataFormat->WaveFormatEx.nBlockAlign = 
            static_cast <WORD>( (pDescriptorAudioDataRange->MaximumBitsPerSample *
            pDescriptorAudioDataRange->MaximumChannels) / 8 );

        pAudioDataFormat->WaveFormatEx.nAvgBytesPerSec =
            pAudioDataFormat->WaveFormatEx.nBlockAlign *
            pDescriptorAudioDataRange->MaximumSampleFrequency;

        pAudioDataFormat->WaveFormatEx.wBitsPerSample = 
            static_cast <WORD> (pDescriptorAudioDataRange->MaximumBitsPerSample);

        pAudioDataFormat->WaveFormatEx.cbSize = 0;

        pAudioDataFormat->DataFormat.SampleSize = pAudioDataFormat->WaveFormatEx.nBlockAlign;
        Status = STATUS_SUCCESS;

    } 
    else 
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Unsupported format request"));
        *pdwDataSize = 0;
        Status = STATUS_NO_MATCH;
    }
    return Status;
}
//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the create dispatch function of the stream pin.
//  It stores the class pointer of the pin class in the pin context.
//
// Return Value:
//  STATUS_UNSUCCESSFUL     Operation failed,
//                          (a) if the function parameters are zero,
//                          (b) if the pin pointer is zero
//                          (c) if stream Create failed
//  STATUS_INSUFFICIENT_RESOURCES
//                          Operation failed,
//                          if stream Create failed
//  STATUS_SUCCESS          Created pin derived from base pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgAudioOutPinCreate
(
 IN PKSPIN pKSPin,   //KS pin, used for system support function calls.
 IN PIRP   pIrp      //Pointer to IRP for this request.
 )
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgAudioOutPinCreate called"));
    //  parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // construct class environment
    CAnlgAudioOut* pAnlgAudioOutPin = new (NON_PAGED_POOL) CAnlgAudioOut();
    if( !pAnlgAudioOutPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //store class object pointer into the context of KS pin
    pKSPin->Context = pAnlgAudioOutPin;
    //call class function
    Status = pAnlgAudioOutPin->Create(pKSPin, pIrp);
    if( Status != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
            ("Error: Class function called without success"));
        delete pAnlgAudioOutPin;
        pAnlgAudioOutPin = NULL;
        pKSPin->Context = NULL;
        return Status;
    }
    return STATUS_SUCCESS;
}
