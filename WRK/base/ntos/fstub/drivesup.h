/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    drivesup.c

Abstract:

    Default handlers for hal functions which don't get handlers
    installed by the hal

--*/

#ifndef _DRIVESUP_H_
#define _DRIVESUP_H_

#define BOOTABLE_PARTITION  0
#define PRIMARY_PARTITION   1
#define LOGICAL_PARTITION   2
#define FT_PARTITION        3
#define OTHER_PARTITION     4

#endif // _DRIVESUP_H_

