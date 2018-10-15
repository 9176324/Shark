/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    init.h

Abstract:

    Header file for the INIT subcomponent of NTOS

--*/

#ifndef _INIT_
#define _INIT_

// begin_ntosp
#define INIT_SYSTEMROOT_LINKNAME "\\SystemRoot"
#define INIT_SYSTEMROOT_DLLPATH  "\\SystemRoot\\System32"
#define INIT_SYSTEMROOT_BINPATH  "\\SystemRoot\\System32"
// end_ntosp

#define INIT_WINPEMODE_NONE                 0x00000000
#define INIT_WINPEMODE_REGULAR              0x00000001
#define INIT_WINPEMODE_INRAM                0x80000000
#define INIT_WINPEMODE_READONLY_MEDIA       0x00000100
#define INIT_WINPEMODE_REMOVABLE_MEDIA      0x00000200  

extern UNICODE_STRING NtSystemRoot;
// begin_ntosp
extern ULONG NtBuildNumber;
// end_ntosp
extern const ULONG NtMajorVersion;
extern const ULONG NtMinorVersion;
extern ULONG CmNtCSDVersion;
extern ULONG CmNtCSDReleaseType;
extern ULONG CmNtSpBuildNumber;
extern UNICODE_STRING CmVersionString;
extern UNICODE_STRING CmCSDVersionString;

extern const CHAR NtBuildLab[];

extern NLSTABLEINFO InitTableInfo;
extern ULONG InitNlsTableSize;
extern PVOID InitNlsTableBase;
extern ULONG InitAnsiCodePageDataOffset;
extern ULONG InitOemCodePageDataOffset;
extern ULONG InitUnicodeCaseTableDataOffset;
extern PVOID InitNlsSectionPointer;
extern BOOLEAN InitSafeModeOptionPresent;
extern ULONG InitSafeBootMode;

extern BOOLEAN InitIsWinPEMode;
extern ULONG InitWinPEModeType;

#if defined(_M_IX86) || defined(_M_AMD64)

VOID
KiSystemStartup(
    IN PVOID LoaderBlock
    );

#else

VOID
KiSystemStartup( VOID );

#endif

VOID
Phase1Initialization(
    IN PVOID Context
    );

typedef
BOOLEAN
(*PTESTFCN)( VOID );

extern PTESTFCN TestFunction;
extern ULONG InitializationPhase;

#if DBG
extern BOOLEAN ForceNonPagedPool;
extern ULONG MmDebug;
#endif // DBG

#endif // _INIT_
