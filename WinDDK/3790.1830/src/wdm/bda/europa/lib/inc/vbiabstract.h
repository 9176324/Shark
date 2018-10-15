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
// @doc
//  INTERNAL EXTERNAL
//
// @module
//  VBIAbstract | Abstract class environment for VBI decoder.
//
// @end
//
// Filename: VBIAbstract.h
//
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#pragma once    // Specifies that the file, in which the pragma resides, will
                // be included (opened) only once by the compiler in a
                // build.

//////////////////////////////////////////////////////////////////////////////
//
// @doc
//  INTERNAL EXTERNAL
//
// @class
//  VBI abstract class interface. The virtual functions have to be implemented
//  from all deriven classes, to have one generic interface to every VBI
//  decoder.
//
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVBIAbstract
{
public:
    // @mfunc
    //  To ensure that the VBI VxD is loaded and ready to accept service
    //  calls, this service is called before calling any other VBI Decoder VxD
    //  services. The Windows VMM returns with Carry Flag cleared and AX set
    //  to 0 if the VxD is not loaded. Currently, the Intel VBI Decoder is a
    //  statically loaded VxD, so if this call fails, VBI Decode is not
    //  possible during the course of the Windows session. If Intel makes the
    //  VBI Decoder dynamically loadable, the Capture Driver can respond to a
    //  failure of this call by calling the VMM service to load the VBI Decode
    //  VxD.
	//
    // @parm
    //  pusVersion | version of decoder VxD
	//
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL GetVersion
            (
                USHORT* pusVersion //version of decoder VxD
            ) = NULL;
    // @mfunc
    //  This call is used to determine initially whether the application is
    //  hungry for VBI data, and to set up callbacks to the Capture Driver
    //  from the VBI Decoder whenever the application hungry status changes.
    //  See VBICallback for specifics. This call may also be used in a polling
    //  fashion, without setting up the callback, if pFunction contains 0. A
    //  previously set up callback may also be torn down by using this call
    //  with pFunction set to 0.
    //
    // @parm
    //  pFunction          | Capture driver callback function
    //  pulRqstLineBitMask | Current state of whether any VBI data is being
    //                      requested
    //
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL RegisterCallback
            (
                void* pFunction,          //Capture driver callback function
                ULONG* pulRqstLineBitMask //Current state of whether any VBI
                                          //        data is being requested
            ) = NULL;
    // @mfunc
    //  The oversampling ratio is the ratio of the effective A/D sample rate
    //  over the VBI data (bit) rate. The oversampling ratio must be conveyed
    //  to the VBI Decoder before decoding can begin. It also must be conveyed
    //  before the Line Pitch, so the VBI Decoder can verify that the Line
    //  Pitch is suitable in view of the Oversampling Ratio. The format of the
    //  Oversampling Ratio is a combined integer and remainder value, with 8
    //  bits for the integer, and 24 bits for the remainder. Bit 23 represents
    //  1/2, bit 22 represents 1/4, etc.  o bits 24-31: Oversample Integer o
    //  bits 0-23: Oversample Remainder  As an example, for 5.00 oversampling
    //  ratio, the value would be 0x05000000. For an oversampling ratio of
    //  5.375, the value would be 0x05600000. The VBI Decoder currently
    //  supports a range of oversampling ratios between 4 and 8. If the ratio
    //  conveyed is not suitable, the VBI Decoder will return an error by
    //  setting the Carry Flag.
    //
    // @parm
    //  ulRatio | oversample ratio
    //
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL SetOversampleRatio
            (
                ULONG ulRatio //oversample ratio
            ) = NULL;
    // @mfunc
    //  The Line Pitch is the distance (in bytes) between the beginnings of
    //  successive scan lines within a field of VBI data in PC memory.
    //  Captured lines must appear contiguous within a field in PC memory,
    //  even if some scan lines from the television signal were skipped during
    //  capture. The Line Pitch must be conveyed to the VBI Decoder before
    //  decoding can begin. It also must be conveyed after the Oversampling
    //  Ratio, so the VBI Decoder can verify that the Line Pitch is suitable
    //  in view of the Oversampling Ratio. If the Line Pitch conveyed is not
    //  suitable, i.e. if it is too small based on the Oversampling Ratio, or
    //  if the Oversampling Ratio has not yet been set by the Capture Driver,
    //  the VBI Decoder will return an error by setting the Carry Flag.
    //
    // @parm
    //  ulPitch | Pitch of line in memory
    //
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL SetLinePitch
            (
                ULONG ulPitch //Pitch of line in memory
            ) = NULL;
    // @mfunc
    //  The TV Channel Number indicates the current Capture Tuner channel. A
    //  value of 0 may be used if the capture hardware is receiving its video
    //  signal from a baseband source. The Channel Number must be conveyed to
    //  the VBI Decoder before decoding can begin, and must also be conveyed
    //  whenever the video channel or source is changed. One important
    //  function of this service is to reset parameters internal to the VBI
    //  Decoder for anti-ghosting support, each time the television signal
    //  source characteristics change. In addition, this service resets
    //  internal statistics for monitoring the decoder performance and signal
    //  quality. If there is a need to simply reset these parameters and
    //  statistics, without actually changing channels, this service may be
    //  called repeatedly with the same Channel Number. Another function of
    //  this service is to make sure the correct channel number is inserted
    //  into all data packets flowing up to the application. For example, the
    //  current Intel Intercast Viewer application will throw away any data
    //  arriving in packets marked with a channel number different from the
    //  channel selected by the TV viewer. This prevents "spillover" of data
    //  when switching channels.
    //
    // @parm
    //  ulChannel | Selected channel
    //
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL SetChannel
            (
                ULONG ulChannel //Selected channel
            ) = NULL;
    // @mfunc
    //  This service processes one entire television field of VBI data, and
    //  sends any data which has been requested by the application on up
    //  through the VBI stack. The FieldBase indicates the location, in PC
    //  memory, of the first scan line of captured data. Each captured line
    //  must start at a memory location one Line Pitch away from the previous
    //  captured line. Captured lines must appear contiguous within the field
    //  in PC memory, even if some lines from the television signal were
    //  intensionally skipped during capture.
    //
    // @parm
    //  pucFieldBase  | 32-bit Linear Address offset pointer to base of field
    //                  data
    //  ulLineBitMask | The LineBitMask indicates to the VBI Decoder which
    //                  lines were actually captured by the Capture Driver.
    //                  Each bit in the 32-bit value corresponds to a line of
    //                  the same number. Since lines 1-9 are never available
    //                  for VBI data, bits 1-9 are reserved and must always be
    //                  set to 0. Similarly, bits 0 and 22-31are reserved and
    //                  must always be set to 0.
    //  ulEvenField   | The EvenField indicator is used by the Closed Caption
    //                  decoder (within the VBI Decoder) to differentiate
    //                  between Closed Caption and Extended Data services.
    //  ulFieldTag    | The FieldTag is an optional 8-bit value which simply
    //                  gets attached to the decoded packet. It may be useful
    //                  for debugging or packet tracing purposes. If not used,
    //                  it should be set to 0.
    //
    // @rdesc
    //  boolean indicates success of operation
    //
    // @end
    //
    virtual BOOL DecodeField
            (
                UCHAR* pucFieldBase, //32-bit Linear Address offset pointer to
                                     //       base of field data
                ULONG ulLineBitMask, //The LineBitMask indicates to the VBI
                                     //    Decoder which lines were actually
                                     //    captured by the Capture Driver.
                                     //    Each bit in the 32-bit value
                                     //    corresponds to a line of the same
                                     //    number. Since lines 1-9 are never
                                     //    available for VBI data, bits 1-9
                                     //    are reserved and must always be set
                                     //    to 0. Similarly, bits 0 and
                                     //    22-31are reserved and must always
                                     //    be set to 0.
                ULONG ulEvenField,   //The EvenField indicator is used by the
                                     //    Closed Caption decoder (within the
                                     //    VBI Decoder) to differentiate
                                     //    between Closed Caption and Extended
                                     //    Data services.
                ULONG ulFieldTag = 0 //The FieldTag is an optional 8-bit value
                                     //    which simply gets attached to the
                                     //    decoded packet. It may be useful
                                     //    for debugging or packet tracing
                                     //    purposes. If not used, it should be
                                     //    set to 0.
            ) = NULL;
};
