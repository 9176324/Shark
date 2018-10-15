////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Name       : urd34def.h
// Description: PCI driver for the Philips Saa7134, having the API's for the URD
// History    : 5/30/2000   FH      Created
////////////////////////////////////////////////////////////////////////////////

typedef enum
{ 
	URD34WDM_UNKNOWN, 
	URD34WDM_GET, 
	URD34WDM_SET, 
	URD34WDM_GET_PCI, 
	URD34WDM_SET_PCI 
} URD34WDMDIR; 

typedef struct _URD34WDMHEADER
{
	URD34WDMDIR		Direction; //indicates direction
	DWORD			dwAddr;//buffer for address 
	DWORD			dwData;//buffer for data

	_URD34WDMHEADER()
	{
		Direction	= URD34WDM_UNKNOWN;
		dwAddr		= 0;
		dwData		= 0;
	}

	_URD34WDMHEADER(URD34WDMDIR Dir)
	{
		Direction	= Dir;
		dwAddr		= 0;
		dwData		= 0;
	}

} URD34WDMHEADER, * PURD34WDMHEADER;

#define URD_INTERNAL_IOCTL \
	CTL_CODE (FILE_DEVICE_VIDEO, 0, METHOD_NEITHER, FILE_ANY_ACCESS)
