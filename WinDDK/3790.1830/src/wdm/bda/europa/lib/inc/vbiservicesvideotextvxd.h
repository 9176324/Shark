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
// @end
//
// @module   
//  VideoTextServices | Encapsulates VideoTextServices in a class
//                      environment.
//
// @end
//
// Filename: VideoTextServices.h
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

#include "VBIAbstract.h"
#include "VBISettings.h"

//////////////////////////////////////////////////////////////////////////////
//
// @doc
//  INTERNAL EXTERNAL
//
// @end
//
// @class
//  This class is deriven from the abstract VBI decoder interface see
//  VBIAbstract. It implements the interface functions to have one interface
//  to different decoder VxDs. 
//
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVBIServicesVideotextVxD : public CVBIAbstract
{
public:
    BOOL GetVersion(USHORT* pusVersion);
    BOOL RegisterCallback(void* pFunction, ULONG* pulRqstLineBitMask);
    BOOL SetOversampleRatio(ULONG ulRatio);
    BOOL SetLinePitch(ULONG ulPitch);
    BOOL SetChannel(ULONG ulChannel);
    BOOL DecodeField(
        UCHAR* pucFieldBase,
        ULONG ulLineBitMask,
        ULONG evenField,
        ULONG ulFieldTag = 0);
private:
    BOOL Set_VideoStandard(BOOL bPal, BOOL bRawData);
    BOOL Set_SampleFreq(DWORD dwSampleFreq);
	BOOL MemCopyField(UCHAR* pucFieldBase);
	BOOL SetVBINumLines(DWORD dwStartLine, DWORD dwNumOfLines);
};


