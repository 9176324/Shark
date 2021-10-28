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
#include <supdefs.h>

#define KernelString L"Shark.sys"
#define SupportString L"VBoxDrv.sys"
#define KernelPortString L"\\{41F4CD92-4AF0-4447-903D-CD994368059A}"

#define CmdClose                0x00000000ul
#define CmdNull                 0x00000001ul
#define CmdPgClear              0x00000002ul
#define CmdVmxOn                0x00000004ul
#define CmdBugCheck             0x00008000ul
#define CmdReload               0x00010000ul

#define MAXIMUM_USER_FUNCTION_TABLE_SIZE 512
#define MAXIMUM_KERNEL_FUNCTION_TABLE_SIZE 256

    typedef struct _FUNCTION_TABLE_ENTRY32 {
        u32 FunctionTable;
        u32 ImageBase;
        u32 SizeOfImage;
        u32 SizeOfTable;
    } FUNCTION_TABLE_ENTRY32, *PFUNCTION_TABLE_ENTRY32;

    C_ASSERT(sizeof(FUNCTION_TABLE_ENTRY32) == 0x10);

    typedef struct _FUNCTION_TABLE_ENTRY64 {
        u64 FunctionTable;
        u64 ImageBase;
        u32 SizeOfImage;
        u32 SizeOfTable;
    } FUNCTION_TABLE_ENTRY64, *PFUNCTION_TABLE_ENTRY64;

    C_ASSERT(sizeof(FUNCTION_TABLE_ENTRY64) == 0x18);

    typedef struct _FUNCTION_TABLE {
        u32 CurrentSize;
        u32 MaximumSize;
        u32 Epoch;
        u8 Overflow;
        u32 TableEntry[1];
    } FUNCTION_TABLE, *PFUNCTION_TABLE;

    C_ASSERT(FIELD_OFFSET(FUNCTION_TABLE, TableEntry) == 0x10);

    typedef struct _FUNCTION_TABLE_LEGACY {
        u32 CurrentSize;
        u32 MaximumSize;
        u8 Overflow;
        u32 TableEntry[1];
    } FUNCTION_TABLE_LEGACY, *PFUNCTION_TABLE_LEGACY;

    C_ASSERT(FIELD_OFFSET(FUNCTION_TABLE_LEGACY, TableEntry) == 0xc);

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
