// This is a part of the Active Template Library.
// Copyright (C) 1996-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLCONV_H__
#define __ATLCONV_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif // _INC_MALLOC

#pragma pack(push,8)

namespace ATL
{
namespace _ATL_SAFE_ALLOCA_IMPL
{
// Following code is to avoid alloca causing a stack overflow.
// It is intended for use from the _ATL_SAFE_ALLOCA macros 
// or Conversion macros.
__declspec(selectany) DWORD _Atlosplatform = 0;
inline BOOL _AtlGetVersionEx()
{
	OSVERSIONINFO osi;
	memset(&osi, 0, sizeof(OSVERSIONINFO));
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osi);
	_Atlosplatform = osi.dwPlatformId;
	return TRUE;
}

// From VC7 CRT sources.
#define _ATL_MIN_STACK_REQ_WIN9X 0x11000
#define _ATL_MIN_STACK_REQ_WINNT 0x2000

/***
* void _resetstkoflw(void) - Recovers from Stack Overflow
*
* Purpose:
*       Sets the guard page to its position before the stack overflow.
*
* Exit:
*       Returns nonzero on success, zero on failure
*
*******************************************************************************/

inline int _Atlresetstkoflw(void)
{
	static BOOL bTemp = _AtlGetVersionEx();
	
    LPBYTE pStack, pGuard, pStackBase, pMaxGuard, pMinGuard;
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    DWORD PageSize;
    DWORD flNewProtect;
    DWORD flOldProtect;

    // Use _alloca() to get the current stack pointer

    pStack = (LPBYTE)_alloca(1);

    // Find the base of the stack.

    if (VirtualQuery(pStack, &mbi, sizeof mbi) == 0)
        return 0;
    pStackBase = (LPBYTE)mbi.AllocationBase;

    // Find the page just below where the stack pointer currently points.
    // This is the highest potential guard page.

    GetSystemInfo(&si);
    PageSize = si.dwPageSize;

    pMaxGuard = (LPBYTE) (((DWORD_PTR)pStack & ~(DWORD_PTR)(PageSize - 1))
                       - PageSize);

    // If the potential guard page is too close to the start of the stack
    // region, abandon the reset effort for lack of space.  Win9x has a
    // larger reserved stack requirement.

    pMinGuard = pStackBase + ((_Atlosplatform == VER_PLATFORM_WIN32_WINDOWS)
                              ? _ATL_MIN_STACK_REQ_WIN9X
                              : _ATL_MIN_STACK_REQ_WINNT);

    if (pMaxGuard < pMinGuard)
        return 0;

    // On a non-Win9x system, do nothing if a guard page is already present,
    // else set up the guard page to the bottom of the committed range.
    // For Win9x, just set guard page below the current stack page.

    if (_Atlosplatform != VER_PLATFORM_WIN32_WINDOWS) {

        // Find first block of committed memory in the stack region

        pGuard = pStackBase;
        do {
            if (VirtualQuery(pGuard, &mbi, sizeof mbi) == 0)
                return 0;
            pGuard = pGuard + mbi.RegionSize;
        } while ((mbi.State & MEM_COMMIT) == 0);
        pGuard = (LPBYTE)mbi.BaseAddress;

        // If first committed block is already marked as a guard page,
        // there is nothing that needs to be done, so return success.

        if (mbi.Protect & PAGE_GUARD)
            return 1;

        // Fail if the first committed block is above the highest potential
        // guard page.  Should never happen.

        if (pMaxGuard < pGuard)
            return 0;

        VirtualAlloc(pGuard, PageSize, MEM_COMMIT, PAGE_READWRITE);
    }
    else {
        pGuard = pMaxGuard;
    }

    // Enable the new guard page.

    flNewProtect = _Atlosplatform == VER_PLATFORM_WIN32_WINDOWS
                   ? PAGE_NOACCESS
                   : PAGE_READWRITE | PAGE_GUARD;

    return VirtualProtect(pGuard, PageSize, flNewProtect, &flOldProtect);
}

#pragma warning(push)
#pragma warning(disable: 4068)	// turn off unknown pragma warning so prefast pragmas won't show
#pragma prefast(push)
#pragma prefast(suppress:515, "Atlresetstkoflw is the same as resetstkoflw")

// Verifies if sufficient space is available on the stack.
inline bool _AtlVerifyStackAvailable(SIZE_T Size)
{
    bool bStackAvailable = true;

    __try
    {
        PVOID p = _alloca(Size);
        p;
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH)
    {
        bStackAvailable = false;
        _Atlresetstkoflw();
    }
    return bStackAvailable;
}

#pragma prefast(pop)
#pragma warning(pop)

// Helper Classes to manage heap buffers for _ATL_SAFE_ALLOCA

// Default allocator used by ATL
class _CCRTAllocator
{
public :
	static void * Allocate(SIZE_T nRequestedSize)
	{
		return malloc(nRequestedSize);
	}
	static void Free(void* p)
	{
		free(p);
	}
};

template < class Allocator>
class CAtlSafeAllocBufferManager
{
private :
	struct CAtlSafeAllocBufferNode
	{
		CAtlSafeAllocBufferNode* m_pNext;
		void* GetData()
		{
			return (this + 1);
		}
	};

	CAtlSafeAllocBufferNode* m_pHead;
public :
	
	CAtlSafeAllocBufferManager() : m_pHead(NULL) {};
	void* Allocate(SIZE_T nRequestedSize)
	{
		CAtlSafeAllocBufferNode *p = (CAtlSafeAllocBufferNode*)Allocator::Allocate(nRequestedSize + sizeof(CAtlSafeAllocBufferNode));
		if (p == NULL)
			return NULL;
		
		// Add buffer to the list
		p->m_pNext = m_pHead;
		m_pHead = p;
		
		return p->GetData();
	}
	~CAtlSafeAllocBufferManager()
	{
		// Walk the list and free the buffers
		while (m_pHead != NULL)
		{
			CAtlSafeAllocBufferNode* p = m_pHead;
			m_pHead = m_pHead->m_pNext;
			Allocator::Free(p);
		}
	}
};

// Use one of the following macros before using _ATL_SAFE_ALLOCA
// EX version allows specifying a different heap allocator
#define USES_ATL_SAFE_ALLOCA_EX(x)	ATL::_ATL_SAFE_ALLOCA_IMPL::CAtlSafeAllocBufferManager<x> _AtlSafeAllocaManager

#ifndef USES_ATL_SAFE_ALLOCA
#define USES_ATL_SAFE_ALLOCA		USES_ATL_SAFE_ALLOCA_EX(ATL::_ATL_SAFE_ALLOCA_IMPL::_CCRTAllocator)
#endif

// nRequestedSize - requested size in bytes 
// nThreshold - size in bytes beyond which memory is allocated from the heap.

// Defining _ATL_SAFE_ALLOCA_ALWAYS_ALLOCATE_THRESHOLD_SIZE always allocates the size specified
// for threshold if the stack space is available irrespective of requested size.
// This available for testing purposes. It will help determine the max stack usage due to _alloca's

#ifdef _ATL_SAFE_ALLOCA_ALWAYS_ALLOCATE_THRESHOLD_SIZE
#define _ATL_SAFE_ALLOCA(nRequestedSize, nThreshold)	\
	(((nRequestedSize) <= (nThreshold) && ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nThreshold) ) ?	\
		_alloca(nThreshold) :	\
		((ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nThreshold)) ? _alloca(nThreshold) : 0),	\
			_AtlSafeAllocaManager.Allocate(nRequestedSize))
#else
#define _ATL_SAFE_ALLOCA(nRequestedSize, nThreshold)	\
	(((nRequestedSize) <= (nThreshold) && ATL::_ATL_SAFE_ALLOCA_IMPL::_AtlVerifyStackAvailable(nRequestedSize) ) ?	\
		_alloca(nRequestedSize) :	\
		_AtlSafeAllocaManager.Allocate(nRequestedSize))
#endif

// Use 1024 bytes as the default threshold in ATL
#ifndef _ATL_SAFE_ALLOCA_DEF_THRESHOLD
#define _ATL_SAFE_ALLOCA_DEF_THRESHOLD	1024
#endif

}	// namespace _ATL_SAFE_ALLOCA_IMPL

}	// namespace ATL

// Make sure MFC's afxconv.h hasn't already been loaded to do this
#ifndef USES_CONVERSION

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#ifndef _DEBUG
#define USES_CONVERSION int _convert; _convert
#else
#define USES_CONVERSION int _convert = 0
#endif

#endif // _ATL_EX_CONVERSION_MACROS_ONLY

#ifndef _DEBUG
	#define USES_CONVERSION_EX int _convert_ex; _convert_ex; UINT _acp_ex = CP_ACP; _acp_ex; LPCWSTR _lpw_ex; _lpw_ex; LPCSTR _lpa_ex; _lpa_ex; USES_ATL_SAFE_ALLOCA
#else
	#define USES_CONVERSION_EX int _convert_ex = 0; _convert_ex; UINT _acp_ex = CP_ACP; _acp_ex; LPCWSTR _lpw_ex = NULL; _lpw_ex; LPCSTR _lpa_ex = NULL; _lpa_ex; USES_ATL_SAFE_ALLOCA
#endif


/////////////////////////////////////////////////////////////////////////////
// Global UNICODE<>ANSI translation helpers
LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars);
LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars);

#ifndef ATLA2WHELPER
#define ATLA2WHELPER AtlA2WHelper
#define ATLW2AHELPER AtlW2AHelper
#endif

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#define A2W(lpa) (\
	((LPCSTR)lpa == NULL) ? NULL : (\
		_convert = (lstrlenA(lpa)+1),\
		ATLA2WHELPER((LPWSTR) alloca(_convert*2), (LPCSTR)lpa, _convert)))

#define W2A(lpw) (\
	((LPCWSTR)lpw == NULL) ? NULL : (\
		_convert = (lstrlenW(lpw)+1)*2,\
		ATLW2AHELPER((LPSTR) alloca(_convert), lpw, _convert)))

#endif // _ATL_EX_CONVERSION_MACROS_ONLY

// The call to _alloca will not cause stack overflow if _AtlVerifyStackAvailable returns TRUE.
#define A2W_EX(lpa, nThreshold) (\
	((_lpa_ex = lpa) == NULL) ? NULL : (\
		_convert_ex = (lstrlenA(_lpa_ex)+1),\
		ATLA2WHELPER(	\
			(LPWSTR)_ATL_SAFE_ALLOCA(_convert_ex * sizeof(WCHAR), nThreshold), \
			_lpa_ex, \
			_convert_ex)))

#define W2A_EX(lpw, nThreshold) (\
	((_lpw_ex = lpw) == NULL) ? NULL : (\
		_convert_ex = (lstrlenW(_lpw_ex)+1) * sizeof(WCHAR),\
		ATLW2AHELPER(	\
			(LPSTR)_ATL_SAFE_ALLOCA(_convert_ex, nThreshold), \
			_lpw_ex, \
			_convert_ex)))
#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#define A2CW(lpa) ((LPCWSTR)A2W(lpa))
#define W2CA(lpw) ((LPCSTR)W2A(lpw))

#endif	// _ATL_EX_CONVERSION_MACROS_ONLY

#define A2CW_EX(lpa, nChar) ((LPCWSTR)A2W_EX(lpa, nChar))
#define W2CA_EX(lpw, nChar) ((LPCSTR)W2A_EX(lpw, nChar))

#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline int ocslen(LPCOLESTR x) { return lstrlenW(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return lstrcpyW(dest, src); }

	inline LPCOLESTR T2COLE_EX(LPCTSTR lp, UINT) { return lp; }
	inline LPCTSTR OLE2CT_EX(LPCOLESTR lp, UINT) { return lp; }
	inline LPOLESTR T2OLE_EX(LPTSTR lp, UINT) { return lp; }
	inline LPTSTR OLE2T_EX(LPOLESTR lp, UINT) { return lp; }	

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY	

#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline int ocslen(LPCOLESTR x) { return lstrlen(x); }
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src) { return lstrcpy(dest, src); }

	inline LPCOLESTR T2COLE_EX(LPCTSTR lp, UINT) { return lp; }
	inline LPCTSTR OLE2CT_EX(LPCOLESTR lp, UINT) { return lp; }
	inline LPOLESTR T2OLE_EX(LPTSTR lp, UINT) { return lp; }
	inline LPTSTR OLE2T_EX(LPOLESTR lp, UINT) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline LPCOLESTR T2COLE(LPCTSTR lp) { return lp; }
	inline LPCTSTR OLE2CT(LPCOLESTR lp) { return lp; }
	inline LPOLESTR T2OLE(LPTSTR lp) { return lp; }
	inline LPTSTR OLE2T(LPOLESTR lp) { return lp; }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#else
	inline int ocslen(LPCOLESTR x) { return lstrlenW(x); }
	//lstrcpyW doesn't work on Win95, so we do this
	inline OLECHAR* ocscpy(LPOLESTR dest, LPCOLESTR src)
	{return (LPOLESTR) memcpy(dest, src, (lstrlenW(src)+1)*sizeof(WCHAR));}
	//CharNextW doesn't work on Win95 so we use this

	#define T2COLE_EX(lpa, nChar) A2CW_EX(lpa, nChar)
	#define T2OLE_EX(lpa, nChar) A2W_EX(lpa, nChar)
	#define OLE2CT_EX(lpo, nChar) W2CA_EX(lpo, nChar)
	#define OLE2T_EX(lpo, nChar) W2A_EX(lpo, nChar)

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	#define T2COLE(lpa) A2CW(lpa)
	#define T2OLE(lpa) A2W(lpa)
	#define OLE2CT(lpo) W2CA(lpo)
	#define OLE2T(lpo) W2A(lpo)
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#endif

#ifdef OLE2ANSI
	inline LPOLESTR A2OLE_EX(LPSTR lp, UINT) { return lp;}
	inline LPSTR OLE2A_EX(LPOLESTR lp, UINT) { return lp;}
	#define W2OLE_EX W2A_EX
	#define OLE2W_EX A2W_EX
	inline LPCOLESTR A2COLE_EX(LPCSTR lp, UINT) { return lp;}
	inline LPCSTR OLE2CA_EX(LPCOLESTR lp, UINT) { return lp;}
	#define W2COLE_EX W2CA_EX
	#define OLE2CW_EX A2CW_EX

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline LPOLESTR A2OLE(LPSTR lp) { return lp;}
	inline LPSTR OLE2A(LPOLESTR lp) { return lp;}
	#define W2OLE W2A
	#define OLE2W A2W
	inline LPCOLESTR A2COLE(LPCSTR lp) { return lp;}
	inline LPCSTR OLE2CA(LPCOLESTR lp) { return lp;}
	#define W2COLE W2CA
	#define OLE2CW A2CW
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#else
	inline LPOLESTR W2OLE_EX(LPWSTR lp, UINT) { return lp; }
	inline LPWSTR OLE2W_EX(LPOLESTR lp, UINT) { return lp; }
	#define A2OLE_EX A2W_EX
	#define OLE2A_EX W2A_EX
	inline LPCOLESTR W2COLE_EX(LPCWSTR lp, UINT) { return lp; }
	inline LPCWSTR OLE2CW_EX(LPCOLESTR lp, UINT) { return lp; }
	#define A2COLE_EX A2CW_EX
	#define OLE2CA_EX W2CA_EX

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY
	inline LPOLESTR W2OLE(LPWSTR lp) { return lp; }
	inline LPWSTR OLE2W(LPOLESTR lp) { return lp; }
	#define A2OLE A2W
	#define OLE2A W2A
	inline LPCOLESTR W2COLE(LPCWSTR lp) { return lp; }
	inline LPCWSTR OLE2CW(LPCOLESTR lp) { return lp; }
	#define A2COLE A2CW
	#define OLE2CA W2CA
#endif // _ATL_EX_CONVERSION_MACROS_ONLY

#endif

#ifdef _UNICODE

	#define T2A_EX W2A_EX
	#define A2T_EX A2W_EX
	inline LPWSTR T2W_EX(LPTSTR lp, UINT) { return lp; }
	inline LPTSTR W2T_EX(LPWSTR lp, UINT) { return lp; }
	#define T2CA_EX W2CA_EX
	#define A2CT_EX A2CW_EX
	inline LPCWSTR T2CW_EX(LPCTSTR lp, UINT) { return lp; }
	inline LPCTSTR W2CT_EX(LPCWSTR lp, UINT) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY
	#define T2A W2A
	#define A2T A2W
	inline LPWSTR T2W(LPTSTR lp) { return lp; }
	inline LPTSTR W2T(LPWSTR lp) { return lp; }
	#define T2CA W2CA
	#define A2CT A2CW
	inline LPCWSTR T2CW(LPCTSTR lp) { return lp; }
	inline LPCTSTR W2CT(LPCWSTR lp) { return lp; }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY	

#else
	#define T2W_EX A2W_EX
	#define W2T_EX W2A_EX
	inline LPSTR T2A_EX(LPTSTR lp, UINT) { return lp; }
	inline LPTSTR A2T_EX(LPSTR lp, UINT) { return lp; }
	#define T2CW_EX A2CW_EX
	#define W2CT_EX W2CA_EX
	inline LPCSTR T2CA_EX(LPCTSTR lp, UINT) { return lp; }
	inline LPCTSTR A2CT_EX(LPCSTR lp, UINT) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	#define T2W A2W
	#define W2T W2A
	inline LPSTR T2A(LPTSTR lp) { return lp; }
	inline LPTSTR A2T(LPSTR lp) { return lp; }
	#define T2CW A2CW
	#define W2CT W2CA
	inline LPCSTR T2CA(LPCTSTR lp) { return lp; }
	inline LPCTSTR A2CT(LPCSTR lp) { return lp; }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#endif

inline BSTR A2WBSTR(LPCSTR lp, int nLen = -1)
{
	if (lp == NULL || nLen == 0)
		return NULL;
	USES_CONVERSION_EX;
	BSTR str = NULL;
	int nConvertedLen = MultiByteToWideChar(_acp_ex, 0, lp,
		nLen, NULL, NULL);
	int nAllocLen = nConvertedLen;
	if (nLen == -1)
		nAllocLen -= 1;  // Don't allocate terminating '\0'
	str = ::SysAllocStringLen(NULL, nAllocLen);
	if (str != NULL)
	{
		int nResult;
		nResult = MultiByteToWideChar(_acp_ex, 0, lp, nLen, str, nConvertedLen);
		if(nResult != nConvertedLen)
		{
			SysFreeString(str);
			return NULL;
		}
	}
	return str;
}

inline BSTR OLE2BSTR(LPCOLESTR lp) {return ::SysAllocString(lp);}
#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR

	inline BSTR T2BSTR_EX(LPCTSTR lp) {return ::SysAllocString(lp);}
	inline BSTR A2BSTR_EX(LPCSTR lp) {return A2WBSTR(lp);}
	inline BSTR W2BSTR_EX(LPCWSTR lp) {return ::SysAllocString(lp);}

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
	inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
	inline BSTR W2BSTR(LPCWSTR lp) {return W2BSTR_EX(lp); }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR

	inline BSTR T2BSTR_EX(LPCTSTR lp) {return ::SysAllocString(lp);}
	inline BSTR A2BSTR_EX(LPCSTR lp) {return ::SysAllocString(lp);}
	inline BSTR W2BSTR_EX(LPCWSTR lp) {USES_CONVERSION_EX; return ::SysAllocString(W2COLE_EX(lp));}

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
	inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
	inline BSTR W2BSTR(LPCWSTR lp) {USES_CONVERSION; return ::SysAllocString(W2COLE(lp));}
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#else

	inline BSTR T2BSTR_EX(LPCTSTR lp) {return A2WBSTR(lp);}
	inline BSTR A2BSTR_EX(LPCSTR lp) {return A2WBSTR(lp);}
	inline BSTR W2BSTR_EX(LPCWSTR lp) {return ::SysAllocString(lp);}

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY	
	inline BSTR T2BSTR(LPCTSTR lp) {return T2BSTR_EX(lp); }
	inline BSTR A2BSTR(LPCSTR lp) {return A2BSTR_EX(lp); }
	inline BSTR W2BSTR(LPCWSTR lp) {return W2BSTR_EX(lp); }
#endif // _ATL_EX_CONVERSION_MACROS_ONLY		

#endif

#if defined(_WINGDI_) && !defined(NOGDI)
/////////////////////////////////////////////////////////////////////////////
// Global UNICODE<>ANSI translation helpers
LPDEVMODEW AtlDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA);
LPDEVMODEA AtlDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW);
LPTEXTMETRICW AtlTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA pltmA);
LPTEXTMETRICA AtlTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW pltmW);

#ifndef ATLDEVMODEA2W
#define ATLDEVMODEA2W AtlDevModeA2W
#define ATLDEVMODEW2A AtlDevModeW2A
#define ATLTEXTMETRICA2W AtlTextMetricA2W
#define ATLTEXTMETRICW2A AtlTextMetricW2A
#endif

// Requires USES_CONVERSION_EX or USES_ATL_SAFE_ALLOCA macro before using the _EX versions of the macros
#define DEVMODEW2A_EX(lpw)\
	((lpw == NULL) ? NULL : ATLDEVMODEW2A((LPDEVMODEA)_ATL_SAFE_ALLOCA(sizeof(DEVMODEA)+lpw->dmDriverExtra, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lpw))
#define DEVMODEA2W_EX(lpa)\
	((lpa == NULL) ? NULL : ATLDEVMODEA2W((LPDEVMODEW)_ATL_SAFE_ALLOCA(sizeof(DEVMODEW)+lpa->dmDriverExtra, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lpa))
#define TEXTMETRICW2A_EX(lptmw)\
	((lptmw == NULL) ? NULL : ATLTEXTMETRICW2A((LPTEXTMETRICA)_ATL_SAFE_ALLOCA(sizeof(TEXTMETRICA), _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lptmw))
#define TEXTMETRICA2W_EX(lptma)\
	((lptma == NULL) ? NULL : ATLTEXTMETRICA2W((LPTEXTMETRICW)_ATL_SAFE_ALLOCA(sizeof(TEXTMETRICW), _ATL_SAFE_ALLOCA_DEF_THRESHOLD), lptma))

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

#define DEVMODEW2A(lpw)\
	((lpw == NULL) ? NULL : ATLDEVMODEW2A((LPDEVMODEA)alloca(sizeof(DEVMODEA)+lpw->dmDriverExtra), lpw))
#define DEVMODEA2W(lpa)\
	((lpa == NULL) ? NULL : ATLDEVMODEA2W((LPDEVMODEW)alloca(sizeof(DEVMODEW)+lpa->dmDriverExtra), lpa))
#define TEXTMETRICW2A(lptmw)\
	((lptmw == NULL) ? NULL : ATLTEXTMETRICW2A((LPTEXTMETRICA)alloca(sizeof(TEXTMETRICA)), lptmw))
#define TEXTMETRICA2W(lptma)\
	((lptma == NULL) ? NULL : ATLTEXTMETRICA2W((LPTEXTMETRICW)alloca(sizeof(TEXTMETRICW)), lptma))

#endif	// _ATL_EX_CONVERSION_MACROS_ONLY

#ifdef OLE2ANSI
	#define DEVMODEOLE DEVMODEA
	#define LPDEVMODEOLE LPDEVMODEA
	#define TEXTMETRICOLE TEXTMETRICA
	#define LPTEXTMETRICOLE LPTEXTMETRICA
#else
	#define DEVMODEOLE DEVMODEW
	#define LPDEVMODEOLE LPDEVMODEW
	#define TEXTMETRICOLE TEXTMETRICW
	#define LPTEXTMETRICOLE LPTEXTMETRICW
#endif

#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline LPDEVMODEW DEVMODEOLE2T_EX(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE_EX(LPDEVMODEW lp) { return lp; }
	inline LPTEXTMETRICW TEXTMETRICOLE2T_EX(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE_EX(LPTEXTMETRICW lp) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

	inline LPDEVMODEW DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODEW lp) { return lp; }
	inline LPTEXTMETRICW TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRICW lp) { return lp; }
#endif	// _ATL_EX_CONVERSION_MACROS_ONLY
	
#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline LPDEVMODE DEVMODEOLE2T_EX(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE_EX(LPDEVMODE lp) { return lp; }
	inline LPTEXTMETRIC TEXTMETRICOLE2T_EX(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE_EX(LPTEXTMETRIC lp) { return lp; }

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

	inline LPDEVMODE DEVMODEOLE2T(LPDEVMODEOLE lp) { return lp; }
	inline LPDEVMODEOLE DEVMODET2OLE(LPDEVMODE lp) { return lp; }
	inline LPTEXTMETRIC TEXTMETRICOLE2T(LPTEXTMETRICOLE lp) { return lp; }
	inline LPTEXTMETRICOLE TEXTMETRICT2OLE(LPTEXTMETRIC lp) { return lp; }
#endif	// _ATL_EX_CONVERSION_MACROS_ONLY

#else
	#define DEVMODEOLE2T_EX(lpo) DEVMODEW2A_EX(lpo)
	#define DEVMODET2OLE_EX(lpa) DEVMODEA2W_EX(lpa)
	#define TEXTMETRICOLE2T_EX(lptmw) TEXTMETRICW2A_EX(lptmw)
	#define TEXTMETRICT2OLE_EX(lptma) TEXTMETRICA2W_EX(lptma)

#ifndef _ATL_EX_CONVERSION_MACROS_ONLY

	#define DEVMODEOLE2T(lpo) DEVMODEW2A(lpo)
	#define DEVMODET2OLE(lpa) DEVMODEA2W(lpa)
	#define TEXTMETRICOLE2T(lptmw) TEXTMETRICW2A(lptmw)
	#define TEXTMETRICT2OLE(lptma) TEXTMETRICA2W(lptma)
#endif	// _ATL_EX_CONVERSION_MACROS_ONLY

#endif

#endif //_WINGDI_

#else //!USES_CONVERSION

// if USES_CONVERSION already defined (i.e. MFC_VER < 4.21 )
// flip this switch to avoid atlconv.cpp
#define _ATL_NO_CONVERSIONS

#endif //!USES_CONVERSION

// Define these even if MFC already included
#if defined(_UNICODE)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNextW(lp);}
#elif defined(OLE2ANSI)
// in these cases the default (TCHAR) is the same as OLECHAR
	inline LPOLESTR CharNextO(LPCOLESTR lp) {return CharNext(lp);}
#else
	inline LPOLESTR CharNextO(LPCOLESTR lp) {return (LPOLESTR)(lp+1);}
#endif

#pragma pack(pop)

#endif // __ATLCONV_H__

/////////////////////////////////////////////////////////////////////////////

