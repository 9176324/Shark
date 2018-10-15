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
//////////////////////////////////////////////////////////////////////////////
#if !defined(AFX_OSDEPEND_H__C1E2A9C8_AAE9_4720_BFE5_66EEB6724512__INCLUDED_)
#define AFX_OSDEPEND_H__C1E2A9C8_AAE9_4720_BFE5_66EEB6724512__INCLUDED_

#include "OSTypes.h"
#include "DbgDefs.h"
#include "VampTypes.h"
#include "VampError.h"

void * __cdecl operator new( size_t nSize,  eVampPoolType iType );

void __cdecl operator delete( PVOID pMem );


LONG _stdcall KMDCompareString(IN PCHAR  String1, IN PCHAR  String2, BOOL  CaseInsensitive  );

int __cdecl KMDsnprintf(PCHAR buffer, size_t count, const PCHAR format, ...);


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class handles memory operations like locking/unlocking pages and 
//  getting page tables for the MMU.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependMemory  
{
private:
    ULONG  m_cBytes;
    ULONG* m_pulVirtualPTAddress;
    PVOID m_pMDL;
	DWORD m_dwObjectStatus;

public:
    COSDependMemory();
    virtual ~COSDependMemory();
    PVOID LockPages( PVOID pVirtualAddress, size_t NumberOfBytes );
    PVOID LockUserPages( PVOID pVirtualAddress, size_t NumberOfBytes );
    void  UnlockUserPages( PVOID pHandle );

    VAMPRET GetPageTable ( 
        PVOID pHandle, 
        PDWORD *ppdwPageEntries, 
        DWORD *pdwNumberOfEntries );

    VAMPRET GetPageTableFromMappings(
        PVOID pMemList,
        PDWORD *ppdwPageEntries,     
        DWORD *pdwNumberOfEntries); 

    PVOID GetSystemAddress( PVOID pHandle );
    void  UnlockPages ( PVOID pHandle );
    DWORD GetObjectStatus();
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This is an abstract callback class which can be used to install a user 
//  defined handler into any asynchronous event class 
//  (such as COSDependTimeOut). Just derive your class from this one and 
//  implement the handler. Then post it into the event class.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependAbstractCB
{
public:
	//callback handler, gets called from any asyncronous event class
    virtual void CallbackHandler() = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class handles timer events. The handler of the abstract callback 
//  class is called after the period of time (in milliseconds) which is set in
//  the InitializeTimeOut function.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependTimeOut
{
public:
	//  the destructor needs to be virtual, because otherwise the destructor 
	//  of the deriven class never gets called!
	virtual ~COSDependTimeOut(){};
	//  this function initializes the requested time out
	// Parameters:
	//  dwTimeToWait   - time to wait in milliseconds
	//  bPeriodic      - should time out be periodic or just a single event
	//  pAbstractCBObj - callback class, contains the callback handler
	// Return Value:
	//  TRUE   initialization was successful
	//  FALSE  initialization was unsuccessful
    virtual BOOL InitializeTimeOut(
        DWORD dwTimeToWait,
        BOOL bPeriodic,
        COSDependAbstractCB* pAbstractCBObj ) = NULL;

	//  returns status of this object
	// Return Value:         
	//  TRUE   object is valid
	//  FALSE  object is invalid
    virtual BOOL GetObjectStatus() = NULL;
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//  This class is obsolete and should be removed (it was needed for the 
//  sample rate conversion in the HAL)
//
// Remarks:         
//  none
//
//////////////////////////////////////////////////////////////////////////////
class COSDependQueryPerformanceCounter
{
public:
	//  the destructor needs to be virtual, because otherwise the destructor 
	//  of the deriven class never gets called!
	virtual ~COSDependQueryPerformanceCounter(){};
	//  returns the system performance counter
	//Return value:
	//  performance counter
    virtual LONGLONG QueryPerformanceCounter() = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class puts the current thread or the whole system in a wait state 
//  for a given time.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependDelayExecution
{
public:
	//  the destructor needs to be virtual, because otherwise the destructor 
	//  of the deriven class never gets called!
	virtual ~COSDependDelayExecution(){};
	//  This method blocks the execution of the current thread for the 
	//  required time in milliseconds. The precision is better than 20 
	//  milliseconds.
	// Parameters:
	//  dwMilliSec - required time in units of milliseconds. The value must 
	//               be in the range of 1 to 4000.
    virtual void DelayExecutionThread(DWORD dwMilliSec) = NULL;

	//  This method stops the CPU activity for the required time in real 
	//  time clock ticks (0.8 microseconds). The precision is better than 2.4
	//  microseconds. Be careful, Drivers that call this routine should 
	//  minimize the number of microseconds they specify (no more than 50).
	//  If a driver must wait for a longer interval, it should use another 
	//  synchronization mechanism.
	// Parameters:
	//  wMicroSec - microseconds to stop the CPU activities. The value 
	//              must be within the range of 1 to 50.
    virtual void DelayExecutionSystem(WORD wMicroSec) = NULL;
};


//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class contains spin lock methods. A spin lock is a synchronization 
//  mechanism that allows  protecting a shared resource against a second 
//  access. It is only allowed to hold one spin lock at a time. This method  
//  needs to be safely callable at interrupt time. If bAtDispatchLevel=TRUE, 
//  the class uses optimized  spin lock methods. It acquires a spin lock when 
//  the caller is already running at IRQL DISPATCH_LEVEL.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependSpinLock 
{
public:
	//  the destructor needs to be virtual, because otherwise the destructor 
	//  of the deriven class never gets called!
	virtual ~COSDependSpinLock(){};
	//  aquires a spinlock
    virtual void AcquireSpinLock () = NULL;

	//  releases a spinlock
    virtual void ReleaseSpinLock () = NULL; 
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Spin lock locking mechanism that uses destructor to ensure that unlock 
//  takes place. Borrowed from ksutil.h.
//
//////////////////////////////////////////////////////////////////////////////
class COSAutoSpinLock
{
public:
    COSAutoSpinLock
    (
        COSDependSpinLock *pLock //Pointer to class OSDependSpinLock.
    ) 
    : m_pSpinLock (pLock)
    {
        m_pSpinLock->AcquireSpinLock();
    }


    virtual ~COSAutoSpinLock() 
    {
        m_pSpinLock->ReleaseSpinLock();
    }

private:
    COSDependSpinLock *m_pSpinLock;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description: 
//
//  This class allows the HAL to create instances of COSDepend classes 
//  without knowing how the class is implemented in the driver.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependObjectFactory
{
public:
	//  creates spinlock object
	// Parameters:
	//  tPoolType - memory pool in that the object should be 
	//              created
	// Return value:
	//  pointer to created object
    virtual COSDependSpinLock* 
		CreateSpinLock(eVampPoolType tPoolType) = NULL;

	//  creates timeout object
	// Parameters:
	//  tPoolType - memory pool in that the object should be 
	//              created 
	// Return value:
	//  pointer to created object
    virtual COSDependTimeOut* 
		CreateTimeOut(eVampPoolType tPoolType) = NULL;

	//  creates queryperformancecounter object
	// Parameters:
	//  tPoolType - memory pool in that the object should be 
	//              created
	// Return value:
	//  pointer to created object
    virtual COSDependQueryPerformanceCounter* 
        CreateQueryPerformanceCounter(eVampPoolType tPoolType) = NULL;

	//  creates delayexecution object
	// Parameters:
	//  tPoolType - memory pool in that the object should be 
	//              created
	// Return value:
	//  pointer to created object
    virtual COSDependDelayExecution* 
		CreateDelayExecution(eVampPoolType tPoolType) = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class abstracts the access to  memory mapped registers.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependRegisterAccess
{
public:
    COSDependRegisterAccess();
	//  destructor
    virtual ~COSDependRegisterAccess();
    
    DWORD GetReg(PBYTE pAddress);
    void  SetReg(PBYTE pAddress,
                 DWORD dwValue);
};

//////////////////////////////////////////////////////////////////////////////
//
// Description: 
//
//  This class provides registry access handling. The registry path is
//  supplied from CVampDeviceResources.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependRegistry  
{
private:
    PVOID m_pvHandle;

public:
    COSDependRegistry( PVOID pRegistryHandle );

    ~COSDependRegistry();

    VAMPRET ReadRegistry(
        PCHAR pszValueName,
        PCHAR pszTargetBuffer,
        DWORD dwTargetBufferLength,
        PCHAR pszRegistrySubkeyPath = NULL,
        PCHAR pszDefaultString = NULL );

    VAMPRET ReadRegistry(
        PCHAR pszValueName,
        PDWORD pdwTargetBuffer,
        PCHAR pszRegistrySubkeyPath = NULL,
        DWORD dwDefaultValue = 0 );

    VAMPRET WriteRegistry(
        PCHAR pszValueName,
        PCHAR pszString,
        PCHAR pszRegistrySubkeyPath = NULL );

    VAMPRET WriteRegistry(
        PCHAR pszValueName,
        DWORD dwValue,
        PCHAR pszRegistrySubkeyPath = NULL );

};

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  Abstract class, which defines the  time stamp methods required by the HAL. 
//  A driver stream object needs an implementation of this class and has to add
//  an object via Stream->SetClock().
//
//////////////////////////////////////////////////////////////////////////////
class COSDependTimeStamp 
{
public:

	// Parameters:
	//  pllTime
	// Return value:
    virtual VAMPRET GetTimeStamp(tOpaqueTimestamp *pllTime) = NULL;
    
	// Parameters:
	//  tTime 
	//  tAdjust 
	// Return value:
    virtual tOpaqueTimestamp AdjustTimeStamp(tOpaqueTimestamp tTime, 
                                             t100nsTime tAdjust ) = NULL;

	// Parameters:
	//  tStart 
	//  tEnd 
	// Return value:
    virtual t100nsTime Difference(tOpaqueTimestamp tStart, 
                                  tOpaqueTimestamp tEnd) = NULL;

	// Parameters:
	//  tTime 
	// Return value:
    virtual t100nsTime ConvertTimestamp (tOpaqueTimestamp tTime) = NULL;

    virtual void SetStartTime() = NULL;
};

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//  typedef for the syncronisation routine
//
//////////////////////////////////////////////////////////////////////////////
typedef BOOL (*VAMP_SYNC_FN)(PVOID pContext);
    
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class serializes a function with the execution of the device interrupt 
//  routine. When the SyncCall method is called, the pfn parameter is called 
//  back with the interrupt locked out by an interrupt spinlock or other 
//  o/s-dependent method.
//
///////////////////////////////////////////////////////////////////////////////
class COSDependSyncCall
{	
public:		
	//  this is the syncronisation routine that blocks other interrupts 
	//  from processing
	// Parameters:
	//  pfn - syncronisation function pointer
	//  pContext - context (class environment) to call the syncronization 
	//  function pointer
	// Return value:
	//  TRUE   for successful blocking
	//  FALSE  for unsuccessful blocking
    virtual BOOL SyncCall(VAMP_SYNC_FN pfn,
                          PVOID pContext ) = NULL;
};

#if DBG

//
// the maximum length for a debug prolog string
//
#define MAX_DEBUGPROLOG_LENGTH 256

//
// the maximum length for a debug string
//
#define MAX_DEBUGSTRING_LENGTH 256

//////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This class is for runtime debugging. The COSDependDebug methods should be 
//  accessed only via macros.
//
//////////////////////////////////////////////////////////////////////////////
class COSDependDebug  
{
private:
  	BYTE m_ucDebugLevel[LAST_DEBUG_CLASS_ID + 1];
	CHAR m_pszDebugProlog[MAX_DEBUGPROLOG_LENGTH];
    CHAR m_pszDebugString[MAX_DEBUGSTRING_LENGTH];
    DWORD m_dwClassID;
    COSDependRegistry* m_pRegistryAccess;

public:
    COSDependDebug(
        COSDependRegistry* pRegistryAccess );

    virtual ~COSDependDebug();

    void Debug_Print_Prolog(
        PCHAR pszFile, 
        int nLine );

    void Debug_Print( 
        PCHAR pszDebugString, 
        ... );

    void Debug_Print(BYTE 
        ucDebugLevel, 
        PCHAR format, 
        ... );

    void Debug_Break( 
        BYTE ucDebugBreakLevel );

	void OnDebugLevelChange();

	void SetCurrentClassID(
        DWORD dwClassID );
};


extern COSDependDebug* g_pDebugObject;

//  Defines a debug break macro. This macro stops the target machine.
//  levels which are lower then the module level will stop the machine.
// Parameters:
//  level - debug level for this debug output
#define DBGBREAK(level) \
    if(g_pDebugObject) g_pDebugObject->Debug_Break(level); 

//
//  Defines a Debug print macro. 
//  Eg: DBGPRINT((1,"CVampExampleModule - Constructor\n"))<nl>
// Parameters:
//  args - arguments for a debug call
#define DBGPRINT(args) { \
    if(g_pDebugObject) { \
		g_pDebugObject->SetCurrentClassID(TYPEID); \
        g_pDebugObject->Debug_Print_Prolog( __FILE__, __LINE__ ); \
        g_pDebugObject->Debug_Print args ; } }

#else // DBG

#define DBGBREAK(arg)
#define DBGPRINT(arg)

#endif // DBG

#endif // !defined(AFX_OSDEPEND_H__C1E2A9C8_AAE9_4720_BFE5_66EEB6724512__INCLUDED_)
