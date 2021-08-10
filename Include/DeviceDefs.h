/*
*
* Copyright (c) 2015 - 2021 by blindtiger ( blindtiger@foxmail.com )
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original Code is blindtiger.
*
*/

#ifndef _DEVICEDEFS_H_
#define _DEVICEDEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#include <devioctl.h>

#define LOADER_STRING L"Shark.sys"
#define SERVICE_STRING L"{107180F9-013A-45B4-BCE5-F046892D7426}"
#define DEVICE_STRING L"\\Device\\{94A4D943-9D91-4DFA-AA05-5486E61BF500}"
#define SYMBOLIC_STRING L"\\DosDevices\\{00081140-C743-454D-917B-C3F437C770DC}"

    FORCEINLINE
        u
        NTAPI
        GuardCall(
            __in_opt PGKERNEL_ROUTINE KernelRoutine,
            __in_opt PGSYSTEM_ROUTINE SystemRoutine,
            __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
            __in_opt PGNORMAL_ROUTINE NormalRoutine
        )
    {
        u Result = 0;

        __try {
            if (NULL != KernelRoutine) {
                Result = KernelRoutine(SystemRoutine, RundownRoutine, NormalRoutine);
            }
            else if (NULL != SystemRoutine) {
                Result = SystemRoutine(RundownRoutine, NormalRoutine);
            }
            else if (NULL != RundownRoutine) {
                Result = RundownRoutine(NormalRoutine);
            }
            else if (NULL != NormalRoutine) {
                Result = NormalRoutine();
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        return Result;
    }

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DEVICEDEFS_H_
