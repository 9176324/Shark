/*
 * Com_VDD.h
 *
 * function definitions used external to Com_VDD.c
 *
 * copyright 1992 by Microsoft Corporation
 *
 * revision history:
 *  24-Dec-1992 written by John Morgan
 *
 */

#ifdef WIN_32
#define WIN
#define FLAT_32
#define TRUE_IF_WIN32   1
#else
#define TRUE_IF_WIN32   0
#endif

#ifdef WIN
#define _WINDOWS
#include "windows.h"
#endif

BOOL VDDInitialize( PVOID, ULONG, PCONTEXT );
VOID VDDTerminateVDM( VOID );
VOID VDDInit( VOID );
VOID VDDDispatch( VOID );

