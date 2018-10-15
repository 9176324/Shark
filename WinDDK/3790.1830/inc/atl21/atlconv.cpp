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
	#error atlconv.cpp requires atlconv.h to be included first
#endif

#ifndef _ATL_NO_CONVERSIONS
/////////////////////////////////////////////////////////////////////////////
// Global UNICODE<>ANSI translation helpers
LPWSTR WINAPI AtlA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
	_ASSERTE(lpa != NULL);
	_ASSERTE(lpw != NULL);
	
	if (lpw == NULL || lpa == NULL)
		return NULL;

	// verify that no illegal character present
	// since lpw was allocated based on the size of lpa
	// don't worry about the number of chars
	lpw[0] = '\0';
	int ret = MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
	if(ret == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}		
	return lpw;
}

LPSTR WINAPI AtlW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
	_ASSERTE(lpw != NULL);
	_ASSERTE(lpa != NULL);
	
	if (lpa == NULL || lpw == NULL)
		return NULL;

	// verify that no illegal character present
	// since lpa was allocated based on the size of lpw
	// don't worry about the number of chars
	lpa[0] = '\0';
	int ret = WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
	if(ret == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}		
	return lpa;
}

#if defined(_WINGDI_) && !defined(NOGDI)

// don't include this code when building DLL
LPDEVMODEW AtlDevModeA2W(LPDEVMODEW lpDevModeW, LPDEVMODEA lpDevModeA)
{
	if (lpDevModeA == NULL || lpDevModeW == NULL)
		return NULL;
	_ASSERTE(lpDevModeW != NULL);
	AtlA2WHelper(lpDevModeW->dmDeviceName, (LPCSTR)lpDevModeA->dmDeviceName, 32);
	memcpy(&lpDevModeW->dmSpecVersion, &lpDevModeA->dmSpecVersion,
		offsetof(DEVMODEW, dmFormName) - offsetof(DEVMODEW, dmSpecVersion));
	AtlA2WHelper(lpDevModeW->dmFormName, (LPCSTR)lpDevModeA->dmFormName, 32);
	memcpy(&lpDevModeW->dmLogPixels, &lpDevModeA->dmLogPixels,
		sizeof(DEVMODEW) - offsetof(DEVMODEW, dmLogPixels));
	if (lpDevModeA->dmDriverExtra != 0)
		memcpy(lpDevModeW+1, lpDevModeA+1, lpDevModeA->dmDriverExtra);
	lpDevModeW->dmSize = sizeof(DEVMODEW);
	return lpDevModeW;
}

LPTEXTMETRICW AtlTextMetricA2W(LPTEXTMETRICW lptmW, LPTEXTMETRICA lptmA)
{
	if (lptmA == NULL || lptmW == NULL)
		return NULL;
	_ASSERTE(lptmW != NULL);
	memcpy(lptmW, lptmA, sizeof(LONG) * 11);
	memcpy(&lptmW->tmItalic, &lptmA->tmItalic, sizeof(BYTE) * 5);
	
	if(MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmFirstChar, 1, &lptmW->tmFirstChar, 1) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}

	if(MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmLastChar, 1, &lptmW->tmLastChar, 1) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	if(MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmDefaultChar, 1, &lptmW->tmDefaultChar, 1) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	if(MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&lptmA->tmBreakChar, 1, &lptmW->tmBreakChar, 1) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	return lptmW;
}

LPTEXTMETRICA AtlTextMetricW2A(LPTEXTMETRICA lptmA, LPTEXTMETRICW lptmW)
{
	if (lptmW == NULL || lptmA == NULL)
		return NULL;
	_ASSERTE(lptmA != NULL);
	memcpy(lptmA, lptmW, sizeof(LONG) * 11);
	memcpy(&lptmA->tmItalic, &lptmW->tmItalic, sizeof(BYTE) * 5);

	if(WideCharToMultiByte(CP_ACP, 0, &lptmW->tmFirstChar, 1, (LPSTR)&lptmA->tmFirstChar, 1, NULL, NULL) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	if(WideCharToMultiByte(CP_ACP, 0, &lptmW->tmLastChar, 1, (LPSTR)&lptmA->tmLastChar, 1, NULL, NULL) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	if(WideCharToMultiByte(CP_ACP, 0, &lptmW->tmDefaultChar, 1, (LPSTR)&lptmA->tmDefaultChar, 1, NULL, NULL) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	if(WideCharToMultiByte(CP_ACP, 0, &lptmW->tmBreakChar, 1, (LPSTR)&lptmA->tmBreakChar, 1, NULL, NULL) == 0)
	{
		_ASSERTE(FALSE);
		return NULL;
	}
	
	return lptmA;
}

LPDEVMODEA AtlDevModeW2A(LPDEVMODEA lpDevModeA, LPDEVMODEW lpDevModeW)
{
	if (lpDevModeW == NULL || lpDevModeA == NULL)
		return NULL;
	_ASSERTE(lpDevModeA != NULL);
	AtlW2AHelper((LPSTR)lpDevModeA->dmDeviceName, lpDevModeW->dmDeviceName, 32);
	memcpy(&lpDevModeA->dmSpecVersion, &lpDevModeW->dmSpecVersion,
		offsetof(DEVMODEA, dmFormName) - offsetof(DEVMODEA, dmSpecVersion));
	AtlW2AHelper((LPSTR)lpDevModeA->dmFormName, lpDevModeW->dmFormName, 32);
	memcpy(&lpDevModeA->dmLogPixels, &lpDevModeW->dmLogPixels,
		sizeof(DEVMODEA) - offsetof(DEVMODEA, dmLogPixels));
	if (lpDevModeW->dmDriverExtra != 0)
		memcpy(lpDevModeA+1, lpDevModeW+1, lpDevModeW->dmDriverExtra);
	lpDevModeA->dmSize = sizeof(DEVMODEA);
	return lpDevModeA;
}

#endif //_WINGDI_
#endif //!_ATL_NO_CONVERSIONS

