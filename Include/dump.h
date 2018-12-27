/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _DUMP_H_
#define _DUMP_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define USERMODE_CRASHDUMP_SIGNATURE    'RESU'
#define USERMODE_CRASHDUMP_VALID_DUMP32 'PMUD'
#define USERMODE_CRASHDUMP_VALID_DUMP64 '46UD'

    //
    // usermode crash dump data types
    //
#define DMP_EXCEPTION                 1 // obsolete
#define DMP_MEMORY_BASIC_INFORMATION  2
#define DMP_THREAD_CONTEXT            3
#define DMP_MODULE                    4
#define DMP_MEMORY_DATA               5
#define DMP_DEBUG_EVENT               6
#define DMP_THREAD_STATE              7
#define DMP_DUMP_FILE_HANDLE          8

    //
    // Define the information required to process memory dumps.
    //

    typedef enum _DUMP_TYPES {
        DUMP_TYPE_INVALID = -1,
        DUMP_TYPE_UNKNOWN = 0,
        DUMP_TYPE_FULL = 1,
        DUMP_TYPE_SUMMARY = 2,
        DUMP_TYPE_HEADER = 3,
        DUMP_TYPE_TRIAGE = 4,
    } DUMP_TYPE;

    //
    // Signature and Valid fields.
    //

#define DUMP_SIGNATURE32   ('EGAP')
#define DUMP_VALID_DUMP32  ('PMUD')

#define DUMP_SIGNATURE64   ('EGAP')
#define DUMP_VALID_DUMP64  ('46UD')

#define DUMP_SUMMARY_SIGNATURE  ('PMDS')
#define DUMP_SUMMARY_VALID      ('PMUD')

#define DUMP_SUMMARY_VALID_KERNEL_VA                     (1)
#define DUMP_SUMMARY_VALID_CURRENT_USER_VA               (2)

    //
    // Define the dump header structure. You cannot change these
    // defines without breaking the debuggers, so don't.
    //

#define DMP_PHYSICAL_MEMORY_BLOCK_SIZE_32   (700)
#define DMP_CONTEXT_RECORD_SIZE_32          (1200)
#define DMP_RESERVED_0_SIZE_32              (1768)
#define DMP_RESERVED_2_SIZE_32              (16)
#define DMP_RESERVED_3_SIZE_32              (56)

#define DMP_PHYSICAL_MEMORY_BLOCK_SIZE_64   (700)
#define DMP_CONTEXT_RECORD_SIZE_64          (3000)
#define DMP_RESERVED_0_SIZE_64              (4016)

#define DMP_HEADER_COMMENT_SIZE             (128)

    // Unset WriterStatus value from the header fill.
#define DUMP_WRITER_STATUS_UNINITIALIZED    DUMP_SIGNATURE32

    // WriterStatus codes for the dbgeng.dll dump writers.
    enum {
        DUMP_DBGENG_SUCCESS,
        DUMP_DBGENG_NO_MODULE_LIST,
        DUMP_DBGENG_CORRUPT_MODULE_LIST,
    };

#ifndef _WIN64
#define DUMP_BLOCK_SIZE 0x20000
#else
#define DUMP_BLOCK_SIZE 0x40000
#endif // !_WIN64

#ifdef NTOS_KERNEL_RUNTIME
#define NOEXTAPI
#endif // NTOS_KERNEL_RUNTIME

#include <wdbgexts.h>

    typedef struct _KDDEBUGGER_DATA_ADDITION64 {

        // Longhorn addition

        ULONG64   VfCrashDataBlock;
        ULONG64   MmBadPagesDetected;
        ULONG64   MmZeroedPageSingleBitErrorsDetected;

        // Windows 7 addition

        ULONG64   EtwpDebuggerData;
        USHORT    OffsetPrcbContext;

        // Windows 8 addition

        USHORT    OffsetPrcbMaxBreakpoints;
        USHORT    OffsetPrcbMaxWatchpoints;

        ULONG     OffsetKThreadStackLimit;
        ULONG     OffsetKThreadStackBase;
        ULONG     OffsetKThreadQueueListEntry;
        ULONG     OffsetEThreadIrpList;

        USHORT    OffsetPrcbIdleThread;
        USHORT    OffsetPrcbNormalDpcState;
        USHORT    OffsetPrcbDpcStack;
        USHORT    OffsetPrcbIsrStack;

        USHORT    SizeKDPC_STACK_FRAME;

        // Windows 8.1 Addition

        USHORT    OffsetKPriQueueThreadListHead;
        USHORT    OffsetKThreadWaitReason;

        // Windows 10 RS1 Addition

        USHORT    Padding;
        ULONG64   PteBase;

        // Windows 10 RS5 Addition

        ULONG64 RetpolineStubFunctionTable;
        ULONG RetpolineStubFunctionTableSize;
        ULONG RetpolineStubOffset;
        ULONG RetpolineStubSize;

    }KDDEBUGGER_DATA_ADDITION64, *PKDDEBUGGER_DATA_ADDITION64;

    typedef struct _DUMP_HEADER {
        ULONG Signature;
        ULONG ValidDump;
        ULONG MajorVersion;
        ULONG MinorVersion;
        ULONG_PTR DirectoryTableBase;
        ULONG_PTR PfnDataBase;
        PLIST_ENTRY PsLoadedModuleList;
        PLIST_ENTRY PsActiveProcessHead;
        ULONG MachineImageType;
        ULONG NumberProcessors;
        ULONG BugCheckCode;
        ULONG_PTR BugCheckParameter1;
        ULONG_PTR BugCheckParameter2;
        ULONG_PTR BugCheckParameter3;
        ULONG_PTR BugCheckParameter4;
        CHAR VersionUser[32];

#ifndef _WIN64
        ULONG PaeEnabled;
#endif // !_WIN64

        struct _KDDEBUGGER_DATA64 * KdDebuggerDataBlock;
    } DUMP_HEADER, *PDUMP_HEADER;

#ifndef _WIN64
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, Signature) == 0);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, ValidDump) == 4);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MajorVersion) == 8);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MinorVersion) == 0xc);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, DirectoryTableBase) == 0x10);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PfnDataBase) == 0x14);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PsLoadedModuleList) == 0x18);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PsActiveProcessHead) == 0x1c);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MachineImageType) == 0x20);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, NumberProcessors) == 0x24);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckCode) == 0x28);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter1) == 0x2c);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter2) == 0x30);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter3) == 0x34);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter4) == 0x38);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PaeEnabled) == 92);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, KdDebuggerDataBlock) == 96);
#else
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, Signature) == 0);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, ValidDump) == 4);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MajorVersion) == 8);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MinorVersion) == 0xc);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, DirectoryTableBase) == 0x10);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PfnDataBase) == 0x18);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PsLoadedModuleList) == 0x20);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, PsActiveProcessHead) == 0x28);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, MachineImageType) == 0x30);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, NumberProcessors) == 0x34);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckCode) == 0x38);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter1) == 0x40);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter2) == 0x48);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter3) == 0x50);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, BugCheckParameter4) == 0x58);
    C_ASSERT(FIELD_OFFSET(DUMP_HEADER, KdDebuggerDataBlock) == 0x80);
#endif // !_WIN64

#ifndef _WIN64
#define KDDEBUGGER_DATA_OFFSET 0x1068
#else
#define KDDEBUGGER_DATA_OFFSET 0x2080
#endif // !_WIN64

#ifdef NTOS_KERNEL_RUNTIME
    ULONG
        NTAPI
        KeCapturePersistentThreadState(
            __in PCONTEXT Context,
            __in_opt PKTHREAD Thread,
            __in ULONG BugCheckCode,
            __in ULONG_PTR BugCheckParameter1,
            __in ULONG_PTR BugCheckParameter2,
            __in ULONG_PTR BugCheckParameter3,
            __in ULONG_PTR BugCheckParameter4,
            __in PDUMP_HEADER DumpHeader
        );
#endif // NTOS_KERNEL_RUNTIME

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DUMP_H_
