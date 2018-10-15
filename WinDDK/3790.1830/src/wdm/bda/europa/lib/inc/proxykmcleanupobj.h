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

#pragma once

//
// Cleanup abstraction stores a resource along with its cleanup method
// and an associated file handle
//
class CProxyKMCleanupObj : public CVampListEntry
{
public:
	virtual HANDLE GetHandle() = NULL;
	virtual PVOID GetObj() = NULL;
};

class CProxyKMCleanupObj_CVampBuffer : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampBuffer(CVampBuffer* pVampBuffer, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampBuffer();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampBuffer* m_pVampBuffer;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CVampAudioStream : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampAudioStream(CVampAudioStream* pVampAudioStream, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampAudioStream();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampAudioStream* m_pVampAudioStream;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CVampVideoStream : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampVideoStream(CVampVideoStream* pVampVideoStream, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampVideoStream();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampVideoStream* m_pVampVideoStream;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CVampVbiStream : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampVbiStream(CVampVbiStream* pVampVbiStream, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampVbiStream();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampVbiStream* m_pVampVbiStream;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CVampTransportStream : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampTransportStream(CVampTransportStream* pVampTransportStream, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampTransportStream();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampTransportStream* m_pVampTransportStream;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CVampGPIOStatic : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CVampGPIOStatic(CVampGPIOStatic* pVampGPIOStatic, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CVampGPIOStatic();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CVampGPIOStatic* m_pVampGPIOStatic;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_CProxyKMCallback : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_CProxyKMCallback(CProxyKMCallback* pProxyKMCallback, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_CProxyKMCallback();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	CProxyKMCallback* m_pProxyKMCallback;
	HANDLE m_hCleanupHandle;
};

class CProxyKMCleanupObj_EventHandle : public CProxyKMCleanupObj
{
public:
	CProxyKMCleanupObj_EventHandle(HANDLE hEventHandle, HANDLE hCleanupHandle);
	virtual ~CProxyKMCleanupObj_EventHandle();
	HANDLE GetHandle();
	PVOID GetObj();
private:
	HANDLE m_hEventHandle;
	HANDLE m_hCleanupHandle;
};


