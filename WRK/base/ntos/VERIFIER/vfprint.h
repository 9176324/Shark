/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    vfprint.h

Abstract:

    This header exposes prototypes required when outputting various data types
    to the debugger.

--*/

VOID
VfPrintDumpIrpStack(
    IN PIO_STACK_LOCATION IrpSp
    );

VOID
VfPrintDumpIrp(
    IN PIRP IrpToFlag
    );

