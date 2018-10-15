
#include "windows.h"
#include <vddsvc.h>


/* entry function of VDD */
BOOL VDDInitialize(HANDLE hVdd, DWORD dwReason, LPVOID lpReserved);

#define VDDOPEN    1
#define VDDCLOSE   2
#define VDDINFO    3


#define DOSIOCTL_GETINFO 0
#define FILE_DEVICE_KRNLDRVR 0x80ff
#define IOCTL_KRNLDRVR_GET_INFORMATION  CTL_CODE(FILE_DEVICE_KRNLDRVR, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

