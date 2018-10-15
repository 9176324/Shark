/**************************************************************************

    AVStream Simulated Hardware Sample

    Copyright (c) 2001, Microsoft Corporation.

    File:

        TStream.cpp

    Abstract:


    History:

        created 1/16/2001

**************************************************************************/

#include "BDACap.h"

/**************************************************************************

    Constants

**************************************************************************/

/**************************************************************************

    LOCKED CODE

**************************************************************************/

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif // ALLOC_PRAGMA


void
CTsSynthesizer::
SynthesizeTS (
    )

/*++

Routine Description:

    Synthesize a transport stream.  The synthesized packets should be placed
    into the current synthesis buffer.

Arguments:

    None

Return Value:

    None

--*/

{
    
    //
    // Copy the synthesized transport stream to the synthesis buffer
    //

}

