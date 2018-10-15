//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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
// @doc      INTERNAL
// @module   CWDMLayerMgr | Definition of audio layer management.
// @end
//////////////////////////////////////////////////////////////////////////////

// 
// The HAL core-stream model allows you to add processing
// layers in software on top of a data source.
// This is used in the audio section to do processing such as
// sample-rate adjustment
//
// The audio-layer code consists of three components:
// -- a Layer Manager that handles the core stream methods and interfaces
//    to the underlying layer
// -- a processing layer that acts as a transform filter on the data
// -- a Clock Adjuster that generates time-stamps for the output
//    buffers (based on the input timestamps).
//
// The processing layer interface is intended to be generic so that
// modules could be tested in a DirectShow environment before being
// incorporated into the kernel-mode HAL.

#ifndef __VAMP_AudioLayer_H__
#define  __VAMP_AudioLayer_H__

#include "ostypes.h"
#include "VampDeviceResources.h"
#include "AbstractStream.h"
#include "VampAudioBaseStream.h"
#include "VampAudioStream.h"

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

//
// abstraction of services that layer manager provides to 
// the Processor module

class ALayerManager
{
public:
    virtual ~ALayerManager() {}

    virtual BOOL GetBuffer(PVOID* ppContext, BYTE** ppStart, int* pcBytes) = 0;
    virtual void Deliver(PVOID pContext, BOOL bDiscont) = 0;
    virtual void Discard(PVOID pContext) = 0;
};

// the clock adjuster is told about the beginning and end of timestamped buffers
// and is given each output buffer to timestamp
class AClockAdjuster
{
public:
    virtual ~AClockAdjuster() {}

    // called by Layer
    virtual void SetRates(long nInput, long nAdjust) = 0;
    virtual void StartSample(LONGLONG  llTime, int nSamples) = 0;
    virtual void EndSample(LONGLONG llTime, int nSamples) = 0;
    virtual void ProcessData(int nSamples) = 0;
    virtual long IncomingRate() = 0;

    // called by Layer Manager
    virtual void Reset() = 0;
    virtual LONGLONG Timestamp() = 0;
};


// definition of the processing layer interface
class ALayer
{
public:
    virtual ~ALayer() {}

    virtual void SetManager(ALayerManager* pmgr, AClockAdjuster* pClockAdjuster, WAVEFORMATEX* pwfx) = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Flush() = 0;

    virtual void Process(BYTE* pStart, int cBytes, LONGLONG tThis, BOOL bDiscont) = 0;
};


// the standard layer manager
class CWDMLayerMgr : public ALayerManager, public CVampAbstract
{
public:
    CWDMLayerMgr(CVampDeviceResources* pDevice);
    ~CWDMLayerMgr();

    VAMPRET Open(ALayer* pLayer, AClockAdjuster* pClock, CVampAudioBaseStream* pBase, WAVEFORMATEX* pwfx);

    // Abstract core stream methods
    VAMPRET AddBuffer(IN CVampBuffer *Buffer );
    VAMPRET RemoveBuffer(IN CVampBuffer *Buffer );
    VAMPRET GetNextDoneBuffer(IN OUT CVampBuffer **pBuffer );
    VAMPRET ReleaseLastQueuedBuffer()
    {
        return VAMPRET_FAIL;
    }
    VAMPRET CancelAllBuffers();

    VOID AdjustClock ( ) {}

    // these should be part of the abstract core stream, but for some reason 
    // appear only in CVampBaseCoreStream
    VAMPRET AddCBOnDPC (CCallOnDPC * pCBOnDPC, PVOID pParam1, PVOID pParam2);
    VAMPRET OnCompletion(PVOID pStream = NULL);
    VOID SetClock (COSDependTimeStamp* pClock );

    // Audio stream methods
    VAMPRET Start();
    void Stop();

    // services for layer
    BOOL GetBuffer(PVOID* ppContext, BYTE** ppStart, int* pcBytes);
    void Deliver(PVOID pContext, BOOL bDiscont);
    void Discard(PVOID pContext);
protected:
    VAMPRET CallOnDPC(PVOID pStream, PVOID pParam1, PVOID pParam2);

    CCallOnDPC *m_pCBOnDPC;
    PVOID m_pCallOnDPCParam1;
    PVOID m_pCallOnDPCParam2;
//    COSDependSpinLock m_lockQueue;
    COSDependSpinLock* m_plockQueue;
    CVampList<CVampBuffer> m_listEmpty;
    CVampList<CVampBuffer> m_listFull;

    CVampAudioBaseStream* m_pBaseStream;
    WAVEFORMATEX m_wfx;
    ALayer* m_pLayer;
    AClockAdjuster* m_pClockAdjuster;
};


#endif // __VAMP_AudioLayer_H__

