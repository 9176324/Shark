/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    arc.h

Abstract:

    This header file defines the ARC status codes.

--*/

#ifndef _ARCCODES_
#define _ARCCODES_

//
// Define ARC status codes.
//

typedef enum _ARC_CODES {
    ESUCCESS,       //  0
    E2BIG,          //  1
    EACCES,         //  2
    EAGAIN,         //  3
    EBADF,          //  4
    EBUSY,          //  5
    EFAULT,         //  6
    EINVAL,         //  7
    EIO,            //  8
    EISDIR,         //  9
    EMFILE,         // 10
    EMLINK,         // 11
    ENAMETOOLONG,   // 12
    ENODEV,         // 13
    ENOENT,         // 14
    ENOEXEC,        // 15
    ENOMEM,         // 16
    ENOSPC,         // 17
    ENOTDIR,        // 18
    ENOTTY,         // 19
    ENXIO,          // 20
    EROFS,          // 21
    EWRONGARCH,     // 22
    EMAXIMUM        // 23
    } ARC_CODES;

#endif // ARCCODES

