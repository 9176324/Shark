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
//  VBISupport | module description
//
// @end
//
// Filename: VBISupport.h
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

#include "VampBuffer.h"
#include "AbstractStream.h"
#include "VBIAbstract.h"
#include "VampVBIStream.h"

//number of VBI decoder VxDs
#define MAX_NUM_OF_VBI_VXD 2
//number of buffers used in VBI streaming
#define MAX_NUM_FIELD_BUFFER 32

class CVBISupport;
extern CVBISupport* g_pVBISupportObj;
extern void VBICallback(void);

//buffer context structure
typedef struct
{
	UCHAR* pBuffer;
	ULONG ulLineBitMask;
} tBufferContext;

//////////////////////////////////////////////////////////////////////////////
//
// @doc    
//  INTERNAL EXTERNAL
//
// @end
//
// @class
//  The global class CVBISupport handles the VBI stream on top of the Vamp HAL
//  interface. If a decoder VxD wants VBI data it calls a previous registered
//  callback function VBICallback() (registered during the VBISupport
//  constructor). It calls VBIDecoderCallback() on the global VBI support
//  class, to have a class environment as soon as possible. Then the VBI
//  stream is set up and started in the private StartVBIStream() function.
//  This function sets up the VBI streaming in the Vamp HAL and registers a
//  DPC callback function for two kinds of interrupts. One DPC routine
//  (Notify()) is called if the decoder standard has changed. In that case the
//  VBI stream is just stopped and restarted. The other DPC routine
//  (CallOnDPC()) is called if the hardware completed a whole VBI buffer. Then
//  every VxD that still wants data (checked with the RegisterCallback()
//  function) is called via its DecodeField() function. If there is no VxD
//  anymore that wants VBI data, the stream is stopped. 
//
// @end
//
//////////////////////////////////////////////////////////////////////////////
class CVBISupport : public CCallOnDPC, public CEventCallback
{
public:
	CVBISupport(CVampDeviceResources* pVampDeviceResources);
	virtual ~CVBISupport();
	VAMPRET CallOnDPC (PVOID pStream, PVOID pParam1, PVOID pParam2);
	VAMPRET Notify(PVOID pParam1, PVOID pParam2, eEventType tEvent);
	void VBIDecoderCallback(ULONG ulLineBitMask);
	void SetChannel(DWORD dwChannel);
private:
	void StartVBIStream(BOOL bCallDuringDPC = FALSE);
	void StopVBIStream(BOOL bCallDuringDPC = FALSE);
	BOOL InitVBIDecoder(CVBIAbstract* pVBIObject, BOOL bIsPAL, LONG lPitchInBytes);
	void GetStandard(BOOL* pbIsPAL, BOOL* pbIs50Hz);
	DWORD m_dwChannelNumber;
	CVampDeviceResources* m_pVampDeviceResources;
	CVBIAbstract* m_pVBIObject[MAX_NUM_OF_VBI_VXD];
	UCHAR* m_pFieldBuffer[MAX_NUM_FIELD_BUFFER];
	CVampBuffer* m_pVampFieldBuffer[MAX_NUM_FIELD_BUFFER];
	tBufferContext m_tContextBuffer[MAX_NUM_FIELD_BUFFER];
	CVampVbiStream* m_pVampVbiStreamObj;
	BOOL m_bVBIStreamRunning;
	COSDependSpinLock* m_pSpinLockObj;
	PVOID m_pEventHandler;
};

