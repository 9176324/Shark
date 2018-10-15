/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    CorrectedEngine.h

Abstract:

    (See module header of CorrectedEngine.cpp)
    
Author:

    Abdullah Ustuner (AUstuner) 28-August-2002

[Notes:]

    Header file for CorrectedEngine.cpp
        
--*/

#ifndef CORRECTEDENGINE_H
#define CORRECTEDENGINE_H

class MCAObjectSink;

//
// Function prototypes for CorrectedEngine.cpp
//
BOOL
MCACreateProcessedEvent(
	VOID
	);

VOID
MCAErrorReceived(
	IN IWbemClassObject *ErrorObject
	);

BOOL
MCAGetCorrectedError(
	VOID
	);

BOOL
MCARegisterCMCConsumer(
	MCAObjectSink *pCMCSink
	);

BOOL
MCARegisterCPEConsumer(
	MCAObjectSink *pCPESink
	);

#endif

