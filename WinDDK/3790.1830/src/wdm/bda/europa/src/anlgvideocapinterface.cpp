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
#include "AnlgVideoCapInterface.h"
#include "AnlgVideoCap.h"

/*
** MultiplyCheckOverflow()
**
**   Perform a 32-bit unsigned multiplication and return if the multiplication
**   did not overflow.
**
** Arguments:
**
**   a - first operand
**   b - second operand
**   pab - result
**
** Returns:
**
**   TRUE - no overflow
**   FALSE - overflow occurred
**
*/

extern BOOL
MultiplyCheckOverflow (
    ULONG a,
    ULONG b,
    ULONG *pab
    );

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the SetDataFormat dispatch function of the analog
//  video capture pin. If called the first time, it creates an analog
//  video capture pin object and stores the analog video capture pin
//  pointer in the pin context. If not called the first time, it retrieves
//  the video capture out pin pointer from the pin context.
//  It also calls the setting of the data format on the instance
//  of the analog video capture pin.
//
// Return Value:
//  STATUS_UNSUCCESSFUL  Operation failed,
//                       (a) if the function parameters are zero,
//                       (b) if the analog video capture pin pointer is zero
//                       (c) SetDataFormat failed
//  STATUS_INSUFFICIENT_RESOURCES
//                       Operation failed,
//                       if the analog video capture pin could't be created
//  STATUS_INVALID_DEVICE_STATE
//                       SetDataFormat on an video capture pin failed,
//                       if the Pin DeviceState is not KSSTATE_STOP
//  STATUS_SUCCESS       SetDataFormat on an video capture pin with success
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVideoCapPinSetDataFormat
(
 IN PKSPIN pKSPin,                   //KS pin, used for system support
                                     //function calls.
 IN OPTIONAL PKSDATAFORMAT pKSOldFormat,
                                    //Pointer to previous format
                                    //structure. Is 0 during the
                                    //first call.
 IN OPTIONAL PKSMULTIPLE_ITEM pKSOldAttributeList,//Previous attributes.
 IN const KSDATARANGE *pKSDataRange,//Pointer to matching range
 IN OPTIONAL const KSATTRIBUTE_LIST *pKSAttributeRange
                                    //New attributes.
 )
{

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVideoCapPinSetDataFormat called"));

    PKS_DATAFORMAT_VIDEOINFOHEADER  pConnectionFormat;
    PKS_DATAFORMAT_VIDEOINFOHEADER2 pConnectionFormat2;

    if( pKSPin->DeviceState != KSSTATE_STOP )
    {
        // We don't accept dynamic format changes
        // Video is a special case because this function is called twice
        // for YUV formats therefore this is not a bug but we dont allow
        // dynamic format change
        _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Invalid device state"));
        return STATUS_INVALID_DEVICE_STATE;
    }

    if( !pKSPin->ConnectionFormat )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid connection format"));
        return STATUS_UNSUCCESSFUL;
    }


    if (IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO) &&
        pKSPin->ConnectionFormat->FormatSize >= sizeof(KS_DATAFORMAT_VIDEOINFOHEADER)) 
    {

        pConnectionFormat =  reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER>(pKSPin->ConnectionFormat);

        PKS_VIDEOINFOHEADER  pVideoInfoHdrToVerify =
            &pConnectionFormat->VideoInfoHeader;

        PKS_DATARANGE_VIDEO             pKSDataRangeVideo = (PKS_DATARANGE_VIDEO) pKSDataRange;
        KS_VIDEO_STREAM_CONFIG_CAPS    *pConfigCaps = &pKSDataRangeVideo->ConfigCaps;
        RECT                            rcImage;

        if ((pVideoInfoHdrToVerify->rcTarget.right -
            pVideoInfoHdrToVerify->rcTarget.left <= 0) ||
            (pVideoInfoHdrToVerify->rcTarget.bottom -
            pVideoInfoHdrToVerify->rcTarget.top <= 0)) {

                rcImage.left = rcImage.top = 0;
                rcImage.right = pVideoInfoHdrToVerify->bmiHeader.biWidth;
                rcImage.bottom = pVideoInfoHdrToVerify->bmiHeader.biHeight;
            }
        else {
            rcImage = pVideoInfoHdrToVerify->rcTarget;
        }

        //
        // Check that bmiHeader.biSize is valid since we use it later.
        //
        {
            ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
                pVideoInfoHdrToVerify
                );

            ULONG DataFormatSize = FIELD_OFFSET (
                KS_DATAFORMAT_VIDEOINFOHEADER, VideoInfoHeader
                ) + VideoHeaderSize;

            if (
                VideoHeaderSize < pVideoInfoHdrToVerify->bmiHeader.biSize ||
                DataFormatSize < VideoHeaderSize ||
                DataFormatSize > pConnectionFormat ->DataFormat.FormatSize
                ) 
            {

                return STATUS_UNSUCCESSFUL;
            }
        }

        //
        // Compute the minimum size of our buffers to validate against.
        // The image synthesis routines synthesize |biHeight| rows of
        // biWidth pixels in either RGB24 or UYVY.  In order to ensure
        // safe synthesis into the buffer, we need to know how large an
        // image this will produce.
        //
        // I do this explicitly because of the method that the data is
        // synthesized.  A variation of this may or may not be necessary
        // depending on the mechanism the driver in question fills the 
        // capture buffers.  The important thing is to ensure that they
        // aren't overrun during capture.
        //
        {
            ULONG ImageSize;

            if (!MultiplyCheckOverflow (
                (ULONG)pVideoInfoHdrToVerify->bmiHeader.biWidth,
                (ULONG)abs (pVideoInfoHdrToVerify->bmiHeader.biHeight),
                &ImageSize
                )) 
            {

                return STATUS_UNSUCCESSFUL;
            }

            //
            // We only support KS_BI_RGB (24) and KS_BI_YUV422 (16), so
            // this is valid for those formats.
            //
            if (!MultiplyCheckOverflow (
                ImageSize,
                (ULONG)(pVideoInfoHdrToVerify->bmiHeader.biBitCount / 8),
                &ImageSize
                )) 
            {
                return STATUS_UNSUCCESSFUL;
            }

            //
            // Valid for the formats we use.  Otherwise, this would be
            // checked later.
            //
            if (pVideoInfoHdrToVerify->bmiHeader.biSizeImage <
                ImageSize) 
            {
                return STATUS_UNSUCCESSFUL;
            }

        }


    } else if (IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO2) &&
        pKSPin->ConnectionFormat->FormatSize >= sizeof(KS_DATAFORMAT_VIDEOINFOHEADER2)) 
    {

        pConnectionFormat2 = reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER2> (pKSPin->ConnectionFormat);

        PKS_VIDEOINFOHEADER2  pVideoInfoHdrToVerify =
            &pConnectionFormat2->VideoInfoHeader2;

        PKS_DATARANGE_VIDEO             pKSDataRangeVideo = (PKS_DATARANGE_VIDEO) pKSDataRange;
        KS_VIDEO_STREAM_CONFIG_CAPS    *pConfigCaps = &pKSDataRangeVideo->ConfigCaps;
        RECT                            rcImage;

        if ((pVideoInfoHdrToVerify->rcTarget.right -
            pVideoInfoHdrToVerify->rcTarget.left <= 0) ||
            (pVideoInfoHdrToVerify->rcTarget.bottom -
            pVideoInfoHdrToVerify->rcTarget.top <= 0)) 
        {

            rcImage.left = rcImage.top = 0;
            rcImage.right = pVideoInfoHdrToVerify->bmiHeader.biWidth;
            rcImage.bottom = pVideoInfoHdrToVerify->bmiHeader.biHeight;
        }
        else 
        {
            rcImage = pVideoInfoHdrToVerify->rcTarget;
        }

        //
        // Check that bmiHeader.biSize is valid since we use it later.
        //
        {
            ULONG VideoHeaderSize = EUROPA_KS_SIZE_VIDEOHEADER2 (
                pVideoInfoHdrToVerify
                );

            ULONG DataFormatSize = FIELD_OFFSET (
                KS_DATAFORMAT_VIDEOINFOHEADER2, VideoInfoHeader2
                ) + VideoHeaderSize;

            if (
                VideoHeaderSize < pVideoInfoHdrToVerify->bmiHeader.biSize ||
                DataFormatSize < VideoHeaderSize ||
                DataFormatSize > pConnectionFormat2 ->DataFormat.FormatSize
                ) 
            {

                return STATUS_UNSUCCESSFUL;
            }
        }

        //
        // Compute the minimum size of our buffers to validate against.
        // The image synthesis routines synthesize |biHeight| rows of
        // biWidth pixels in either RGB24 or UYVY.  In order to ensure
        // safe synthesis into the buffer, we need to know how large an
        // image this will produce.
        //
        // I do this explicitly because of the method that the data is
        // synthesized.  A variation of this may or may not be necessary
        // depending on the mechanism the driver in question fills the 
        // capture buffers.  The important thing is to ensure that they
        // aren't overrun during capture.
        //
        {
            ULONG ImageSize;

            if (!MultiplyCheckOverflow (
                (ULONG)pVideoInfoHdrToVerify->bmiHeader.biWidth,
                (ULONG)abs (pVideoInfoHdrToVerify->bmiHeader.biHeight),
                &ImageSize
                )) 
            {

                return STATUS_UNSUCCESSFUL;
            }

            //
            // We only support KS_BI_RGB (24) and KS_BI_YUV422 (16), so
            // this is valid for those formats.
            //
            if (!MultiplyCheckOverflow (
                ImageSize,
                (ULONG)(pVideoInfoHdrToVerify->bmiHeader.biBitCount / 8),
                &ImageSize
                )) 
            {
                return STATUS_UNSUCCESSFUL;
            }

            //
            // Valid for the formats we use.  Otherwise, this would be
            // checked later.
            //
            if (pVideoInfoHdrToVerify->bmiHeader.biSizeImage <
                ImageSize) 
            {
                return STATUS_UNSUCCESSFUL;
            }

        }


    } 
    else 
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format size detected"));
        return STATUS_UNSUCCESSFUL;
    }

    if( !NT_SUCCESS(KsEdit (pKSPin, &pKSPin->Descriptor, 'pmaV')) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: KsEdit failed"));
        return STATUS_UNSUCCESSFUL;
    }

    //if the edits proceeded without running out of memory, adjust
    //the framing based on the video info header.
    if( !NT_SUCCESS(KsEdit(pKSPin,
        &(pKSPin->Descriptor->AllocatorFraming),'pmaV')) )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: 2nd KsEdit failed"));
        return STATUS_UNSUCCESSFUL;
    }
    //we've KsEdit'ed this...  I'm safe to cast away constness as
    //long as the edit succeeded.
    PKSALLOCATOR_FRAMING_EX Framing = const_cast
        <PKSALLOCATOR_FRAMING_EX> (pKSPin->Descriptor->AllocatorFraming);

    Framing -> FramingItem [0].Frames = NUM_VD_CAP_STREAM_BUFFER;
    Framing -> FramingItem [0].PhysicalRange.Stepping =
        Framing -> FramingItem [0].FramingRange.Range.Stepping =
        0;

    // some of the framing parameters need special cast dependend to 
    // the format specifier
    if( IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO) )
    {
        PKS_DATAFORMAT_VIDEOINFOHEADER pConnectionFormat =  reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER>(pKSPin->ConnectionFormat);
        //It is not necessary to check again because the
        //pKSPin->ConnectionFormat is already checked before and the
        //reinterpret can not return zero.

        // The physical and optimal ranges must be biSizeImage.  We only
        // support one frame size, precisely the size of each Capture
        // image.
        //
        Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
            Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
            pConnectionFormat->VideoInfoHeader.bmiHeader.biSizeImage;
    }
    else if( IsEqualGUID(pKSPin->ConnectionFormat->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO2) )
    {
        PKS_DATAFORMAT_VIDEOINFOHEADER2 pConnectionFormat2 = reinterpret_cast
            <PKS_DATAFORMAT_VIDEOINFOHEADER2> (pKSPin->ConnectionFormat);
        //It is not necessary to check again because the
        //pKSPin->ConnectionFormat is already checked before and the
        //reinterpret can not return zero.

        // The physical and optimal ranges must be biSizeImage.  We only
        // support one frame size, precisely the size of each Capture
        // image.
        //
        Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
            Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
            pConnectionFormat2->VideoInfoHeader2.bmiHeader.biSizeImage;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid format specifier detected"));
        return STATUS_NO_MATCH;
    }

    return STATUS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//  This is the handler for comparison of a data range.
//  The AnlgVideoCapIntersectDataFormat is the IntersectHandler function
//  of the analog video capture out pin.(Needs no class environment to intersect
//  the format, because the format that is supported is already
//  given in the filter descriptor [ANLG_VIDEO_CAP_PIN_DESCRIPTOR]).
// Settings:
//  A match occurs under three conditions:
//  (1) if the major format of the range passed matches a pin factory range,
//  (2) if the sub-format matches,
//  (3) if the specifier is KSDATAFORMAT_SPECIFIER_VIDEOINFO.
//  Since the data range size may be variable, it is not validated beyond
//  checking that it is at least the size of a data range structure.
//  (KSDATAFORMAT_SPECIFIER_VIDEOINFO)
//  (a) If the pCallerDataRange FormatSize is equal to
//      KS_DATARANGE_VIDEO the VideoInfoHeader structure is used.
//  (b) If the pCallerDataRange FormatSize is equal to
//      KS_DATARANGE_VIDEO2 the VideoInfoHeader2 structure is used.
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
//  STATUS_SUCCESS          AnlgVideoCapIntersectDataFormat with success
//                          if a matching range is found
//
//////////////////////////////////////////////////////////////////////////////
NTSTATUS AnlgVideoCapIntersectDataFormat
(
 IN PKSFILTER pKSFilter,                 //Pointer to KS filter structure.
 IN PIRP pIrp,                           //Pointer to data intersection
                                         //property request.
 IN PKSP_PIN pPinInstance,               //Pinter to structure indicating
                                         //pin in question.
 IN PKSDATARANGE pCallerDataRange,       //Pointer to the caller data
                                         //structure that should be
                                         //intersected.
 IN PKSDATARANGE pDescriptorDataRange,   //Pointer to the descriptor data
                                         //structure, see AnlgVideoFormats.h.
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
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVideoCapIntersectDataFormat called"));

    //invalid parameters?
    if( !pKSFilter || !pIrp || !pPinInstance ||
        !pCallerDataRange || !pDescriptorDataRange || !pdwDataSize )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    //set output data size
    if (IsEqualGUID(pDescriptorDataRange->Specifier,
        KSDATAFORMAT_SPECIFIER_VIDEOINFO)) {    

            //*** start intersection ***//

            //check if given datarange GUID matches to our datarange information
            if( pCallerDataRange->FormatSize < sizeof(KS_DATARANGE_VIDEO) )
            {
                _DbgPrintF(DEBUGLVL_ERROR,
                    ("Error: Invalid format structure size"));
                return STATUS_NO_MATCH;
            }

            //map given data ranges on video information structures
            //for some reason only a subpart of the KS_DATARANGE_VIDEO is used for intersection
            //this subpart is casted to KS_DATAFORMAT_VIDEOINFOHEADER and contains only
            //KSDATAFORMAT and KS_VIDEOINFOHEADER, see pTargetVideoDataRange
            PKS_DATARANGE_VIDEO pCallerVideoDataRange =
                reinterpret_cast <PKS_DATARANGE_VIDEO> (pCallerDataRange);
            PKS_DATARANGE_VIDEO pDescriptorVideoDataRange =
                reinterpret_cast <PKS_DATARANGE_VIDEO> (pDescriptorDataRange);
            PKS_DATAFORMAT_VIDEOINFOHEADER pTargetVideoDataRange =
                reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> (pData);

            //check if all other important fields match
            if( pCallerVideoDataRange->bFixedSizeSamples        !=
                pDescriptorVideoDataRange->bFixedSizeSamples        ||
                pCallerVideoDataRange->bTemporalCompression     !=
                pDescriptorVideoDataRange->bTemporalCompression     ||
                pCallerVideoDataRange->StreamDescriptionFlags   !=
                pDescriptorVideoDataRange->StreamDescriptionFlags   ||
                pCallerVideoDataRange->MemoryAllocationFlags    !=
                pDescriptorVideoDataRange->MemoryAllocationFlags    ||

                (RtlCompareMemory(&pCallerVideoDataRange->ConfigCaps,
                &pDescriptorVideoDataRange->ConfigCaps,
                sizeof(KS_VIDEO_STREAM_CONFIG_CAPS))) !=
                sizeof(KS_VIDEO_STREAM_CONFIG_CAPS)    )
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Important parameter does not match"));
                return STATUS_NO_MATCH;
            }

            {
                ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER(
                    &pCallerVideoDataRange->VideoInfoHeader
                    );
                ULONG DataRangeSize = 
                    FIELD_OFFSET(KS_DATARANGE_VIDEO, VideoInfoHeader) +
                    VideoHeaderSize;
                if (
                    VideoHeaderSize < pCallerVideoDataRange->
                    VideoInfoHeader.bmiHeader.biSize ||
                    DataRangeSize < VideoHeaderSize ||
                    DataRangeSize > pCallerVideoDataRange->
                    DataRange.FormatSize
                    ) {
                        return STATUS_INVALID_PARAMETER;
                    }
            }

            ULONG FormatSize;
            FormatSize = sizeof(KSDATAFORMAT) + 
                KS_SIZE_VIDEOHEADER (&pCallerVideoDataRange->VideoInfoHeader);

            *pdwDataSize = FormatSize;

            //check for size only query
            if( dwBufferSize == 0 )
            {
                _DbgPrintF(DEBUGLVL_BLAB,("Info: Size only query"));
                return STATUS_BUFFER_OVERFLOW;
            }


            //check if output buffer size is sufficient
            if( dwBufferSize < *pdwDataSize )
            {
                _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer too small"));
                return STATUS_BUFFER_TOO_SMALL;
            }

            //copy the data range structure from our decriptor into the target format buffer,
            //for some reason two different names where given to the same structure
            //KSDATARANGE equals exactly KSDATAFORMAT
            pTargetVideoDataRange->DataFormat =
                static_cast <KSDATAFORMAT> (pDescriptorVideoDataRange->DataRange);

            //as mentioned above the target data range structure differs from the
            //caller and the descriptor structures, so the size is also different
            //and has to be set correctly
            pTargetVideoDataRange->DataFormat.FormatSize = *pdwDataSize;

            //copy the video info header structure from the caller into the target
            //buffer,we have to check at this time whether the requested caller
            //video format fits our capabilities (not implemented right now)
            RtlCopyMemory(
                &pTargetVideoDataRange->VideoInfoHeader,
                &pCallerVideoDataRange->VideoInfoHeader,
                KS_SIZE_VIDEOHEADER (&pCallerVideoDataRange->VideoInfoHeader));

            //If there is a format change (e.g. a new resolution) the size is not updated
            //automatically, so we have to calculate it here. There is a macro that multiplies
            //the width and height and that also aligns the size to DWORDs
            pTargetVideoDataRange->VideoInfoHeader.bmiHeader.biSizeImage =
                pTargetVideoDataRange->DataFormat.SampleSize =
                KS_DIBSIZE(pTargetVideoDataRange->VideoInfoHeader.bmiHeader);

            Status = STATUS_SUCCESS;
        } else if (IsEqualGUID(pDescriptorDataRange->Specifier,
            KSDATAFORMAT_SPECIFIER_VIDEOINFO2)) {

                //*** start intersection ***//

                //check if given datarange GUID matches to our datarange information
                if( pCallerDataRange->FormatSize < sizeof(KS_DATARANGE_VIDEO2) )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,
                        ("Error: Invalid format structure size"));
                    return STATUS_NO_MATCH;
                }

                //map given data ranges on video information structures
                //for some reason only a subpart of the KS_DATARANGE_VIDEO is
                //used for intersection this subpart is casted to
                //KS_DATAFORMAT_VIDEOINFOHEADER and contains only
                //KSDATAFORMAT and KS_VIDEOINFOHEADER, see pTargetVideoDataRange
                PKS_DATARANGE_VIDEO2 pCallerVideoDataRange =
                    reinterpret_cast <PKS_DATARANGE_VIDEO2> (pCallerDataRange);
                PKS_DATARANGE_VIDEO2 pDescriptorVideoDataRange =
                    reinterpret_cast <PKS_DATARANGE_VIDEO2> (pDescriptorDataRange);
                PKS_DATAFORMAT_VIDEOINFOHEADER2 pTargetVideoDataRange =
                    reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER2> (pData);

                //check if all other important fields match
                if( pCallerVideoDataRange->bFixedSizeSamples        !=
                    pDescriptorVideoDataRange->bFixedSizeSamples        ||
                    pCallerVideoDataRange->bTemporalCompression     !=
                    pDescriptorVideoDataRange->bTemporalCompression     ||
                    pCallerVideoDataRange->StreamDescriptionFlags   !=
                    pDescriptorVideoDataRange->StreamDescriptionFlags   ||
                    pCallerVideoDataRange->MemoryAllocationFlags    !=
                    pDescriptorVideoDataRange->MemoryAllocationFlags    ||

                    (RtlCompareMemory(&pCallerVideoDataRange->ConfigCaps,
                    &pDescriptorVideoDataRange->ConfigCaps,
                    sizeof(KS_VIDEO_STREAM_CONFIG_CAPS))) !=
                    sizeof(KS_VIDEO_STREAM_CONFIG_CAPS)    )
                {
                    _DbgPrintF(DEBUGLVL_VERBOSE,("Warning: Important parameter does not match"));
                    return STATUS_NO_MATCH;
                }

                {
                    ULONG VideoHeaderSize = EUROPA_KS_SIZE_VIDEOHEADER2(
                        &pCallerVideoDataRange->VideoInfoHeader
                        );
                    ULONG DataRangeSize = 
                        FIELD_OFFSET(KS_DATARANGE_VIDEO2, VideoInfoHeader) +
                        VideoHeaderSize;
                    if (
                        VideoHeaderSize < pCallerVideoDataRange->
                        VideoInfoHeader.bmiHeader.biSize ||
                        DataRangeSize < VideoHeaderSize ||
                        DataRangeSize > pCallerVideoDataRange->
                        DataRange.FormatSize
                        ) {
                            return STATUS_INVALID_PARAMETER;
                        }
                }

                ULONG FormatSize;
                FormatSize = sizeof(KSDATAFORMAT) + 
                    EUROPA_KS_SIZE_VIDEOHEADER2 (&pCallerVideoDataRange->VideoInfoHeader);


                *pdwDataSize = FormatSize;

                //check for size only query
                if( dwBufferSize == 0 )
                {
                    _DbgPrintF(DEBUGLVL_BLAB,("Info: Size only query"));
                    return STATUS_BUFFER_OVERFLOW;
                }

                //check if output buffer size is sufficient
                if( dwBufferSize < *pdwDataSize )
                {
                    _DbgPrintF(DEBUGLVL_ERROR,("Error: Buffer too small"));
                    return STATUS_BUFFER_TOO_SMALL;
                }

                //copy the data range structure from our decriptor into the target
                //format buffer, for some reason two different names where given
                //to the same structure KSDATARANGE equals exactly KSDATAFORMAT
                pTargetVideoDataRange->DataFormat =
                    static_cast <KSDATAFORMAT> (pDescriptorVideoDataRange->DataRange);
                //as mentioned above the target data range structure differs from the
                //caller and the descriptor structures, so the size is also different
                //and has to be set correctly
                pTargetVideoDataRange->DataFormat.FormatSize = *pdwDataSize;

                //copy the video info header structure from the caller into the target
                //buffer, we have to check at this time whether the requested caller
                //video formatfits our capabilities (not implemented right now)
                RtlCopyMemory(
                    &pTargetVideoDataRange->VideoInfoHeader2,
                    &pCallerVideoDataRange->VideoInfoHeader,
                    EUROPA_KS_SIZE_VIDEOHEADER2 (&pCallerVideoDataRange->VideoInfoHeader));

                //If there is a format change (e.g. a new resolution) the size is
                //not updated automatically, so we have to calculate it here.
                //There is a macro that multiplies the width and height and that
                //also aligns the size to DWORDs
                pTargetVideoDataRange->VideoInfoHeader2.bmiHeader.biSizeImage =
                    pTargetVideoDataRange->DataFormat.SampleSize =
                    KS_DIBSIZE(pTargetVideoDataRange->VideoInfoHeader2.bmiHeader);

                Status = STATUS_SUCCESS;
            } else {
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
NTSTATUS AnlgVideoCapPinCreate
(
    IN PKSPIN pKSPin,   //KS pin, used for system support function calls.
    IN PIRP   pIrp      //Pointer to IRP for this request.
)
{
    NTSTATUS Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,(""));
    _DbgPrintF(DEBUGLVL_BLAB,("Info: AnlgVideoCapPinCreate called"));
    //  parameters valid?
    if( !pKSPin || !pIrp )
    {
        _DbgPrintF(DEBUGLVL_ERROR,("Error: Invalid argument"));
        return STATUS_UNSUCCESSFUL;
    }
    // construct class environment
    CAnlgVideoCap* pAnlgVideoCapPin = new (NON_PAGED_POOL) CAnlgVideoCap();
    if( !pAnlgVideoCapPin )
    {
        _DbgPrintF(DEBUGLVL_ERROR,
            ("Error: not enough memory"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    //store class object pointer into the context of KS pin
    pKSPin->Context = pAnlgVideoCapPin;
    //call class function
    Status = pAnlgVideoCapPin->Create(pKSPin, pIrp);
    if( Status != STATUS_SUCCESS )
    {
        _DbgPrintF( DEBUGLVL_ERROR,
                    ("Error: Class function called without success"));
        delete pAnlgVideoCapPin;
        pAnlgVideoCapPin = NULL;
        pKSPin->Context = NULL;
        return Status;
    }
    return STATUS_SUCCESS;
}
