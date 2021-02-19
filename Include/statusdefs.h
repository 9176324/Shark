/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Disk License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRAY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _STATUSDEFS_H_
#define _STATUSDEFS_H_

#include <typesdefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    //
    // STATUS
    //

    typedef s32 status;

    /*lint -save -e624 */  // Don't complain about different typedefs.
    /*lint -restore */  // Resume checking for different typedefs.

    //
    //  st values are 32 bit values layed out as follows:
    //
    //   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
    //   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    //  +---+-+-------------------------+-------------------------------+
    //  |Sev|C|       Facility          |               Code            |
    //  +---+-+-------------------------+-------------------------------+
    //
    //  where
    //
    //      Sev - is the severity code
    //
    //          00 - Success
    //          01 - Informational
    //          10 - Warning
    //          11 - Error
    //
    //      C - is the Customer code flag
    //
    //      Facility - is the facility code
    //
    //      Code - is the facility's status code
    //

    //
    // Generic test for success on any status value (non-negative numbers
    // indicate success).
    //

#define ST_SUCCESS(st) ((s32)(st) >= 0)

//
// Generic test for information on any status value.
//

#define ST_INFORMATION(st) ((u32)(st) >> 30 == 1)

//
// Generic test for warning on any status value.
//

#define ST_WARNING(st) ((u32)(st) >> 30 == 2)

//
// Generic test for error on any status value.
//

#define ST_ERROR(st) ((u32)(st) >> 30 == 3)

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_STATUSDEFS_H_
