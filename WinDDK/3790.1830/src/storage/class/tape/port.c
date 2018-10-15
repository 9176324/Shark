/*++

Copyright (C) Microsoft Corporation, 1990 - 1999

Module Name:

    port.c

Abstract:

    Implements TapeClassNotification to allow /GS
    support LIB to be miniport-agnostic

Authors:

    Jonathan Schwartz (JSchwart)    23-Oct-2003

Environment:

    kernel mode only

Notes:

    This module is a dll for the kernel.

Revision History:

--*/



#include "tape.h"
#include <srb.h>

VOID
TapeClassNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    va_list                 ap;

    va_start(ap, HwDeviceExtension);

    switch (NotificationType) {

        case QueryTickCount: {
            PLARGE_INTEGER TickCount;

            TickCount = va_arg (ap, PLARGE_INTEGER);

            KeQueryTickCount(TickCount);
            break;
        }

        case BufferOverrunDetected:
            KeBugCheckEx(DRIVER_OVERRAN_STACK_BUFFER, 0, 0, 0, 0);
            break;

        default: {
             ASSERT(0);
             break;
        }
    }

    va_end(ap);

} // end TapeClassNotification()

