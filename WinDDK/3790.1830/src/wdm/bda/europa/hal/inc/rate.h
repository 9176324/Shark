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
// Rate.h
//
// Definition of Audio Rate Adjustment layer for SAA7134 HAL
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SAA7134_RATE_H_
#define _SAA7134_RATE_H_

#include "AudioLayer.h"

/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define abs(a)              (((a) < 0) ? (-a) : (a))

class AClockAdjuster;

// an implementation of AClockAdjuster that
// interpolates timestamps assuming that the sample-rate is being
// modified but the times should remain the same
class CSampleRateClock : public AClockAdjuster
{
public:
    CSampleRateClock(COSDependRegistry* pRegistry, COSDependTimeStamp* pClock,
        CVampDeviceResources* pDevice);
    ~CSampleRateClock();
    
    // called by Layer
    void SetRates(long nInput, long nAdjust);
    void StartSample(LONGLONG  llTime, int nSamples);
    void EndSample(LONGLONG llTime, int nSamples);
    void ProcessData(int nSamples);
    long IncomingRate() {
        return m_nRateInput;
    }

    // called by Layer Manager
    void Reset();
    LONGLONG Timestamp();
protected:
    long m_nRateInput;       // average input rate
    long m_nRate;           // corrected output rate
    long m_nAdjust;
    long m_cSamplesSinceLast;
    LONGLONG m_cReceived;
    LONGLONG m_cReceivedLastAverage;
    BOOL m_bSeenTime;
    BOOL m_bInputRateSet;
    LONGLONG m_tLast;
    LONGLONG m_tFirst;
    COSDependTimeStamp* m_pClock;
    COSDependRegistry* m_pRegistry;
    CVampDeviceResources* m_pDevice;
};
                  
// abstraction of the sample-rate adjustment layers
// to allow run-time choice between implementation
class ARateAdjust : public ALayer
{
public:
    // set absolute rate (by system clock)
    virtual void SetRate(int nSamplesPerSecond, BOOL bEnable) = 0;


    // signed count of samples to add or subtract each second
    // to incoming stream
    virtual void SetAdjustmentRate(int nSamplesPerSecond) = 0;
    virtual int GetAdjustmentRate() = 0;
};

// an audio layer that interpolates or removes
// isolated samples in order to adjust the sampling
// rate. This affects the pitch.
class CInterpolateAudio : public ARateAdjust
{
public:
    CInterpolateAudio();
    ~CInterpolateAudio();
    
    void SetManager(ALayerManager* pmgr, AClockAdjuster* pClockAdjuster, WAVEFORMATEX* pwfx);
    void Start();
    void Stop();
    void Flush();
    void Process(BYTE* pStart, int cBytes, LONGLONG tThis, BOOL bDiscont);

    // set absolute rate (by system clock)
    void SetRate(int nSamplesPerSecond, BOOL bEnable);

    // signed count of samples to add or subtract each second
    void SetAdjustmentRate(int nSamplesPerSecond);
    int GetAdjustmentRate();
protected:

    ALayerManager* m_pManager;
    AClockAdjuster* m_pClockAdjuster;
    WAVEFORMATEX m_wfx;
    int m_nInsertionRate;

    BOOL m_bDiscontOnNext;

    // we work insertion on a per-second basis
    // m_nConsumed shows how many samples have been consumed in this current
    // second. m_nOps shows how many insert or delete ops have been 
    // done this second. Both are reset when m_nConsumed reaches 
    // m_wfx.nSamplesPerSecond;
    // m_cLeftInBlock is the bytes till next op.
    int m_nConsumed;
    int m_nOps;
    int m_cLeftInBlock;
    

    // current output buffer
    PVOID m_pCurrent;
    BYTE* m_pCurrentData;
    int m_cCurrentLeft;
    BYTE m_bLastSample[16]; // must be greater than nBlockAlign

    int CopyModified(BYTE* pIncoming, int cBytes);
    void CalcCopySize();
};


// an alternative implementation that uses time-granulation
// to implement sample rate changes without pitch change
class CTimeGrain : public ARateAdjust
{
public:
    CTimeGrain();
    ~CTimeGrain();

    void SetManager(ALayerManager* pmgr, AClockAdjuster* pClockAdjuster, WAVEFORMATEX* pwfx);
    void Start();
    void Stop();
    void Flush();
    void Process(BYTE* pStart, int cBytes, LONGLONG tThis, BOOL bDiscont);

    // set absolute rate (by system clock)
    void SetRate(int nSamplesPerSecond, BOOL bEnable);

    // signed count of samples to add or subtract each second
    void SetAdjustmentRate(int nSamplesPerSecond);
    int GetAdjustmentRate();

protected:
    ALayerManager* m_pManager;
    AClockAdjuster* m_pClockAdjuster;
    WAVEFORMATEX m_wfx;
    int m_nInsertionRate;
    BOOL m_bDiscontOnNext;

    int m_nSamplesPerGrain;
    BYTE* m_pGrain;
    
    // current output buffer
    PVOID m_pCurrent;
    BYTE* m_pCurrentData;
    int m_cCurrentLeft;

    // byte status values indicating position in copy loop
    int m_nCopiesPerOp;
    int m_nCopiesDone;
    int m_nGrainSaved;
    int m_nGrainCopied;

    int Output(BYTE*& pStart, int& cSource, int& cThis);
};

#endif // _SAA7134_RATE_H_

