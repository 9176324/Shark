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

#ifndef _TYPES__
#define _TYPES__

#include "VampIfTypes.h"

class CUMCallback;

typedef
struct STREAMPARAMS {                   //@struct common UM stream input parameters
    eFieldType          nField;         //@field Field type - O, E or I fields
    CUMCallback*		pUMCBObj;		//@field Pointer to UM CB object class
    PVOID               pUMParam;		//@field Parameter for UM CB method
    COSDependTimeStamp* pTimeStamp;     //@field COSDependTimeStamp object
	union {
		struct
		{
          eVideoOutputType	nOutputType;	//@field video output types [VSB_A,VSB_B,VSB_AB,ITU,VIP11,VIP20,DMA]
		  eVideoStandard	nVideoStandard; //@field Video standard
          eVideoOutputFormat nFormat;		//@field Video format
		  tVideoRect		tVideoInputRect;//@field Acquisition rectangle
		  DWORD             dwOutputSizeX;  //@field Video output window size horizontal
		  DWORD             dwOutputSizeY;  //@field Video output window size vertical
		  union {
			  struct {
			  WORD   wFractionPart;     // @field fraction frame rate
			  WORD   wFrameRate;        // @field total frame rate
			  } FrameRateAndFraction; 
			  DWORD dwFrameRate;        //@field Frame rate, the high word contains the 
                                        // total frames, the low word the fraction
		  } u;
		  tStreamFlags	dwStreamFlags;  //@field Flags for the StreamManager to handle the video stream
		} tVStreamParms;
		// space future addings, e.g. audio layer or etc
	} u;
} tStreamParams;


#endif // _TYPES__
