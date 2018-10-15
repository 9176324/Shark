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
// @doc      INTERNAL
// @module   KMProxy | Provides generic (OS independent) functionality to
//           communicate with the HAL from the user mode. Kernel mode drivers
//           that link this library have to do the following things:<nl>     
//           1) implement the device kernel mode I/O control and call
//           IOCTL_Command(PIOCTL_COMMAND) on every user mode proxy command
//           (the commands are automatically sorted to their corresponding
//           command group function and will then be processed into the
//           HAL).<nl>                                                       
//           2) The device params structure and the registry access object has
//           to be calculated/constructed and must be passed through the
//           AddDevice function into the kernel mode proxy for every detected
//           device during boot sequence or later, but before the first call
//           to any I/O control.<nl>                                         
//           3) There is one OS dependent function called
//           SignalCommonEvent(PVOID hRingNullEvent) (defined in the
//           ProxyOSDepend.h header) that must be implemented to have the DPC
//           callback functionality.

// Filename: KMProxy.h
//
// Routines/Functions:
//
//  public:
//
//  CProxyKM();
//  VAMPRET IOCTL_Command(PIOCTL_COMMAND);
//  VAMPRET IOCTL_Audio(PIOCTL_COMMAND);
//  VAMPRET IOCTL_Video(PIOCTL_COMMAND);
//  VAMPRET IOCTL_VBI(PIOCTL_COMMAND);
//  VAMPRET IOCTL_Transport(PIOCTL_COMMAND);
//  VAMPRET IOCTL_GPIOStatic(PIOCTL_COMMAND);
//  VAMPRET IOCTL_I2c(PIOCTL_COMMAND);
//  VAMPRET IOCTL_Decoder(PIOCTL_COMMAND);
//  VAMPRET IOCTL_Buffer(PIOCTL_COMMAND);
//  VAMPRET IOCTL_DeviceInfo(PIOCTL_COMMAND);
//  VAMPRET IOCTL_VirtualMachine(PIOCTL_COMMAND);
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

//HAL header
#include "VampDeviceResources.h"
#include "VampAudioStream.h"
#include "VampVideoStream.h"
#include "VampVbiStream.h"
#include "VampTransportStream.h"
#include "VampGPIOStatic.h"
#include "VampI2c.h"
#include "VampDecoder.h"
#include "VampBuffer.h"

//KM proxy header
#include "ProxyInterface.h"
#include "ProxyKMCallback.h"
#include "ProxyKMCleanupObj.h"
#include "ProxyKMCleanupList.h"

//defs to have easy access to the HAL objects that are defined PVOID (the HAL
//     objects are defined PVOID to signal that they must not be accessed from
//     the user directly)
#define P_KM_AUDIO_STREAM_IN      ((CVampAudioStream*)(pInBuffer->pKMAudioStream))
#define P_KM_AUDIO_STREAM_OUT     ((CVampAudioStream*)(pOutBuffer->pKMAudioStream))
#define P_KM_VIDEO_STREAM_IN      ((CVampVideoStream*)(pInBuffer->pKMVideoStream))
#define P_KM_VIDEO_STREAM_OUT     ((CVampVideoStream*)(pOutBuffer->pKMVideoStream))
#define P_KM_VBI_STREAM_IN        ((CVampVbiStream*)(pInBuffer->pKMVbiStream))
#define P_KM_VBI_STREAM_OUT       ((CVampVbiStream*)(pOutBuffer->pKMVbiStream))
#define P_KM_TRANSPORT_STREAM_IN  ((CVampTransportStream*)(pInBuffer->pKMTransportStream))
#define P_KM_TRANSPORT_STREAM_OUT ((CVampTransportStream*)(pOutBuffer->pKMTransportStream))
#define P_KM_GPIO_STATIC_IN       ((CVampGPIOStatic*)(pInBuffer->pKMGPIOStatic))
#define P_KM_GPIO_STATIC_OUT      ((CVampGPIOStatic*)(pOutBuffer->pKMGPIOStatic))
#define P_KM_I2C                  ((CVampI2c*)(pInBuffer->pKMI2c))
#define P_KM_BUFFER_IN            ((CVampBuffer*)(pInBuffer->pKMBuffer))
#define P_KM_BUFFER_OUT           ((CVampBuffer*)(pOutBuffer->pKMBuffer))
#define PP_KM_BUFFER              ((CVampBuffer**)&(pOutBuffer->pKMBuffer))
#define P_KM_EVENT_IN             ((CProxyKMCallback*)(pInBuffer->pKMEventObj))
#define P_KM_EVENT_OUT            ((CProxyKMCallback*)(pOutBuffer->pKMEventObj))
#define P_KM_OSDEPEND_DELAY       ((COSDependDelayExecution*)(pInBuffer->pKMOSDependDelayObj))

class CProxyKM
{
public:
    CProxyKM();
    virtual ~CProxyKM();

    VAMPRET IOCTL_Command(PIOCTL_COMMAND pDIOCParams, HANDLE hFile = NULL);
	VAMPRET AddDevice(
		OUT DWORD* pdwDeviceIndex,
		IN tDeviceParams* tInfoOfDevices,
		IN COSDependObjectFactory* pFactoryObj,
		IN COSDependRegistry* pRegistryAccess,
		IN COSDependSyncCall* pSyncCallObj);
	VAMPRET RemoveDevice(
		IN DWORD dwDeviceIndex);
	VOID Cleanup(HANDLE hFile);
	CVampDeviceResources* GetDeviceResourceObject(DWORD dwDeviceIndex);
	COSDependRegistry* GetRegistryAccessObject(DWORD dwDeviceIndex);
	tDeviceParams GetInfoOfDevice(DWORD dwDeviceIndex);
	VAMPRET GetNumberOfDevices(PDWORD pdwNumOfDevices);
private:
    VAMPRET IOCTL_Audio(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_Video(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_VBI(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_Transport(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_GPIOStatic(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_I2c(PIOCTL_COMMAND);
    VAMPRET IOCTL_Decoder(PIOCTL_COMMAND);
    VAMPRET IOCTL_Buffer(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_DeviceInfo(PIOCTL_COMMAND);
    VAMPRET IOCTL_VirtualMachine(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
    VAMPRET IOCTL_EventHandler(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);

#if TEST_OSDEPEND
    VAMPRET IOCTL_TestOSDepend(PIOCTL_COMMAND pDIOCParams, HANDLE hCleanupHandle);
#endif

    DWORD m_dwNumOfDevices;
    tDeviceParams m_tInfoOfDevices[MAX_NUM_OF_DEVICES];
	COSDependRegistry *m_pRegistryAccess[MAX_NUM_OF_DEVICES];
    CVampDeviceResources* m_pVampDeviceResources[MAX_NUM_OF_DEVICES];
	COSDependSpinLock* m_pSpinLockObj;
	CProxyKMCleanupList m_ProxyKMCleanupList;
};
