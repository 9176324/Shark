/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    Common.h

Abstract:

    (See module header of Common.cpp)
    
Author:

    Abdullah Ustuner (AUstuner) 28-August-2002

[Notes:]

    Header file for Common.cpp
        
--*/

#ifndef COMMON_H
#define COMMON_H

#if defined(_X86_)
#define MCAPrintErrorRecord MCAPrintErrorRecordX86
#elif defined (_AMD64_)
#define MCAPrintErrorRecord MCAPrintErrorRecordAMD64
#else
#define MCAPrintErrorRecord MCAPrintErrorRecordIA64
#endif

//
// Function prototypes for Common.cpp
//
BOOL
MCAExtractErrorRecord(
    IN IWbemClassObject *Object,
    OUT PUCHAR *PRecordBuffer    
	);

BOOL
MCAInitialize(
	VOID
	);

BOOL
MCAInitializeCOMLibrary(
	VOID
	);

BOOL
MCAInitializeWMISecurity(
	VOID
	);

VOID
MCAPrintErrorRecordX86(
	PUCHAR PErrorRecordBuffer
	);

VOID
MCAPrintErrorRecordAMD64(
	PUCHAR PErrorRecordBuffer
	);

VOID
MCAPrintErrorRecordIA64(
	PUCHAR PErrorRecordBuffer
	);

#endif

