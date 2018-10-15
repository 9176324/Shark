//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1998
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
// @module   Defaults | Some default values

// Filename: Defaults.h
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

#ifndef DEFAULTS_H__
#define DEFAULTS_H__

// defaults
static const ULONG BrightnessDefault = 0x80;
static const ULONG ContrastDefault   = 0x44;
static const ULONG SaturationDefault = 0x40;
static const ULONG HueDefault        = 0x00;
static const ULONG SharpnessDefault  = 0x00;

#define MIN_VAMP_BRIGHTNESS_UNITS    0
#define MAX_VAMP_BRIGHTNESS_UNITS  255

#define MIN_VAMP_CONTRAST_UNITS   -128
#define MAX_VAMP_CONTRAST_UNITS    127

#define MIN_VAMP_HUE_UNITS        -128
#define MAX_VAMP_HUE_UNITS         127

#define MIN_VAMP_SATURATION_UNITS -128
#define MAX_VAMP_SATURATION_UNITS  127

#define MIN_VAMP_SHARPNESS_UNITS    -8
#define MAX_VAMP_SHARPNESS_UNITS     7

const int NumBuffers = 8;

// cool. Hardcoded for NTSC for now
//bt829 const int SamplingFreq = 28636363;
//saa711x const int SamplingFreq = 27000000;
//#define SamplingFreq       27000000
#define NTSCFrameDuration  333667
#define NTSCFieldDuration  NTSCFrameDuration/2

#define DefWidth     320
#define DefHeight    240

#define MaxInWidth   720
#define MinInWidth    80

#define MaxInHeight  480
#define MinInHeight   60

#define MaxOutWidth  320
#define MinOutWidth   80

#define MaxOutHeight 240
#define MinOutHeight  60


////////////////////////////////////////////////////////////////////////////////
// VBI sizes for the different TV norms
////////////////////////////////////////////////////////////////////////////////

// Set Samples * 2 (oversampled)
//#define VBISamples   720 * 2

/// cool. hack
//bt829 const int VREFDiscard = 8;
#define VREFDiscard  8


////////////////////////////////////////////////////////////////////////////////
// defaults used for the declared ranges: sampling rate and
// samples per line are notified (after initialisation) by the
// decoder
// 27MHz implies 720 2-byte pixels (treat as YUV-422 but actually its
// y0y0y1y1)
// 13.5MHz implies 720 1-byte pixels (use format YUV8)
#define  VBISamples_Default                             720*2
#define  VBISampFreq_Default                         27000000

// NTSC & PAL constants
#define  VBIStart_NTSC_Min                                  7
#define  VBIStart_NTSC                                     10
#define  VBIEnd_NTSC                                       21
#define  VBIEnd_NTSC_Max                                   21
#define  VBILines_NTSC      ( VBIEnd_NTSC - VBIStart_NTSC + 1)
#define  VBIStart_PAL_Min                                   4
#define  VBIStart_PAL                                       7
#define  VBIEnd_PAL                                        22
#define  VBIEnd_PAL_Max                                    22
#define  VBILines_PAL         ( VBIEnd_PAL - VBIStart_PAL + 1)

// defaults are PAL
#define  VBIStart                                VBIStart_PAL
#define  VBIEnd                                    VBIEnd_PAL
#define  VBILines                   ( VBIEnd - VBIStart + 1 )
#define  MaxVBIEnd   22

////////////////////////////////////////////////////////////////////////////////
// Video sizes for dynamic property sets
////////////////////////////////////////////////////////////////////////////////

// NTSC
#define NTSCWidth         720
#define NTSCFrameHeight   483
#define NTSCFieldHeight   240

#define DefWidthNTSC      320
#define DefHeightNTSC     240

#define MaxInWidthNTSC    720
#define MinInWidthNTSC     80

#define MaxInHeightNTSC   478
#define MinInHeightNTSC    60

#define MaxOutWidthNTSC   320
#define MinOutWidthNTSC    80

#define MaxOutHeightNTSC  240
#define MinOutHeightNTSC   60

// PAL / SECAM
#define PALWidth          720
#define PALFrameHeight    575
#define PALFieldHeight    288

#define DefWidthPAL       320
#define DefHeightPAL      240

#define MaxInWidthPAL     720
#define MinInWidthPAL      80

#define MaxInHeightPAL    574
#define MinInHeightPAL     60

#define MaxOutWidthPAL    320
#define MinOutWidthPAL     80

#define MaxOutHeightPAL   288
#define MinOutHeightPAL    60


#define FIRSTROW_PAL         0
#define NUMBEROFROWS_PAL   720
#define STARTLINE_PAL       24 
#define NUMBEROFLINES_PAL  288

#define FIRSTROW_NTSC        0
#define NUMBEROFROWS_NTSC  720
#define STARTLINE_NTSC      22 
#define NUMBEROFLINES_NTSC 240  

#define SCALEMAXLINES_NTSC 478  
#define SCALEMAXLINES_PAL  574  

#endif // DEFAULTS_H__

