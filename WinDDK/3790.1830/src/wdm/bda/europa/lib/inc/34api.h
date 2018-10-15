#include <windows.h>

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SAA7134_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// 34API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef P34API_EXPORTS
#define P34_API __declspec(dllexport) //__stdcall
#else
#define P34_API __declspec(dllimport) //__stdcall
#endif
/*
// example for usage
// This class is exported from the 34API.dll
class P34_API CSAA7134 {
public:
	CSAA7134(void);
	// TODO: add your methods here.
};

extern P34_API int nSAA7134;

P34_API int fnSAA7134(void);
*/
