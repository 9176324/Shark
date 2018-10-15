/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    decode.c

Abstract:

    This module implement the code necessary to dispatch exceptions to the
    proper mode and invoke the exception dispatcher.

--*/

#include "ki.h"

//
// Global data used for synchronization of the patch process.
// 

LONG volatile KiCodePatchCycle;

//
// Gather statistics on number of prefetch instructions patched and the
// number of unsuccessful attempts.
//

ULONG KiOpPrefetchPatchCount;
ULONG KiOpPrefetchPatchRetry;

//
// A flag for each instruction prefix
// 

#define OP_PREFIX_ES        (1 << 0)
#define OP_PREFIX_CS        (1 << 1)
#define OP_PREFIX_SS        (1 << 2)
#define OP_PREFIX_DS        (1 << 3)
#define OP_PREFIX_FS        (1 << 4)
#define OP_PREFIX_GS        (1 << 5)
#define OP_PREFIX_OPERAND   (1 << 6)
#define OP_PREFIX_ADDR      (1 << 7)
#define OP_PREFIX_LOCK      (1 << 8)
#define OP_PREFIX_REPNE     (1 << 9)
#define OP_PREFIX_REPE      (1 << 10)
#define OP_PREFIX_REX       (1 << 11)

//
// An identifier for each prefix group with more than a single
// prefix
// 

#define OP_PREFIXGRP_SEGMENT    (1 << 0)
#define OP_PREFIXGRP_REP        (1 << 1)

#define OP_TWOBYTE          0x0F
#define OP_NOREGEXT         0xFF

#define OP_MAXIMUM_LENGTH   15

//
// Flag definitions for DECODE_ENTRY.Flags
//

#define OPF_NONE            (0)
#define OPF_IMM8            (1 << 0)
#define OPF_IMM32           (1 << 1)
#define OPF_MODRM           (1 << 2)

#define OPF_GP              ((ULONG)1 << 31)
#define OPF_INVAL           ((ULONG)1 << 30)
#define OPF_DIV             ((ULONG)1 << 29)
#define OPF_AV              ((ULONG)1 << 28)

//
// A structure used to coordinate pausing of processors
//

typedef struct _KIOP_COPY_CONTEXT {

    PUCHAR Destination;
    LOGICAL CopyDirect;
    LOGICAL CopyPerformed;
    NTSTATUS Status;

#if !defined(NT_UP)

    ULONG ProcessorNumber;
    LONG volatile ProcessorsRunning;
    LONG volatile ProcessorsToResume;
    LOGICAL volatile Done;

#endif

    UCHAR Replacement;

} KIOP_COPY_CONTEXT, *PKIOP_COPY_CONTEXT;

//
// A table containing each of the possible legacy prefix bytes.
// The REX prefix is not included here, rather it is checked for
// separately.
// 

struct {

    //
    // The numeric value of the prefix byte
    //

    UCHAR Prefix;

    //
    // The flag associated with this prefix
    //

    ULONG Flag;

    //
    // If the prefix shares its group with other prefixes, the
    // group identifier is stored here to detect illegal use of
    // multiple prefix bytes from within the same group.
    //

    ULONG Group;

} const KiOpPrefixTable[] = {
    { 0x26, OP_PREFIX_ES,      OP_PREFIXGRP_SEGMENT },
    { 0x2e, OP_PREFIX_CS,      OP_PREFIXGRP_SEGMENT },
    { 0x36, OP_PREFIX_SS,      OP_PREFIXGRP_SEGMENT },
    { 0x3e, OP_PREFIX_DS,      OP_PREFIXGRP_SEGMENT },
    { 0x64, OP_PREFIX_FS,      OP_PREFIXGRP_SEGMENT },
    { 0x65, OP_PREFIX_GS,      OP_PREFIXGRP_SEGMENT },
    { 0x66, OP_PREFIX_OPERAND, 0 },
    { 0x67, OP_PREFIX_ADDR,    0 },
    { 0xf0, OP_PREFIX_LOCK,    0 },
    { 0xf2, OP_PREFIX_REPNE,   OP_PREFIXGRP_REP },
    { 0xf3, OP_PREFIX_REPE,    OP_PREFIXGRP_REP }
};

typedef struct _DECODE_ENTRY const *PDECODE_ENTRY;

//
// This is the context that is built and passed around most of the
// routines in this module to track the state of the decode process.
// 

typedef struct _DECODE_CONTEXT {
    PUCHAR Start;
    PUCHAR Next;
    PCONTEXT ContextRecord;
    PEXCEPTION_RECORD ExceptionRecord;
    PUCHAR OpCodeLocation;
    ULONG PrefixMask;
    ULONG GroupMask;
    UCHAR OpCode;
    BOOLEAN TwoByte;
    BOOLEAN CompatibilityMode;

    union {
        struct {
            UCHAR B:1;
            UCHAR X:1;
            UCHAR R:1;
            UCHAR W:1;
        };
        UCHAR Byte;
    } Rex;

    union {
        struct {
            UCHAR rm:3;
            UCHAR reg:3;
            UCHAR mod:2;
        };
        UCHAR Byte;
    } ModRM;

    union {
        struct {
            UCHAR base : 3;
            UCHAR index : 3;
            UCHAR scale : 2;
        };
        UCHAR Byte;
    } SIB;

    LONG Displacement;
    LONG64 Immediate;

    BOOLEAN ModRMRead;

    KPROCESSOR_MODE PreviousMode;
    PDECODE_ENTRY DecodeEntry;

    //
    // Set to TRUE if the offending instruction or context has been
    // fixed up in some way and should be retried.
    //

    BOOLEAN Retry;

} DECODE_CONTEXT, *PDECODE_CONTEXT;

typedef
NTSTATUS
(*PKOP_OPCODE_HANDLER) (
    IN PDECODE_CONTEXT DecodeContext
    );

//
// Entry used to build the decode tables.
// 

typedef struct _DECODE_ENTRY {

    //
    // OpCode is the first byte of a single byte opcode, or the
    // second byte of a two byte opcode.
    //

    UCHAR OpCode;

    //
    // Range is the number of sequential opcodes covered by this
    // entry.
    //

    UCHAR Range;

    //
    // Some opcodes are additionally identified by a specific prefix.
    // That prefix is supplied here, otherwise 0.
    //

    ULONG Prefix;

    //
    // Some opcodes are additionally identified by a value in the
    // ModRM's reg field.  If so that value is supplied here, otherwise
    // OP_NOREGEXT is specified.
    //

    UCHAR ModRMExtension;

    //
    // Additional OPF_ flags
    //

    ULONG Flags;

    //
    // Pointer to the handler for this opcode.
    //

    PKOP_OPCODE_HANDLER Handler;

} DECODE_ENTRY;

//
// Routines external to this module
//

VOID
KiCpuIdFault (
    VOID
    );

BOOLEAN
KiFilterFiberContext (
    PCONTEXT Context
    );

//
// Forwarded declarations for routines local to this module
// 

NTSTATUS
KiCheckForAtlThunk (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_Div (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_LSAHF (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS   
KiOp_MOVAPS (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_MOVDQA (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_PREF_NOP (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_PREFETCH3 (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_PREFETCHx (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_Priv (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOp_Illegal (
    IN PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOpDecode (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOpDecodeModRM (
    IN OUT PDECODE_CONTEXT DecodeContext
    );

NTSTATUS
KiOpFetchBytes (
    IN PDECODE_CONTEXT DecodeContext,
    IN ULONG Count,
    OUT PVOID Buffer
    );

NTSTATUS
KiOpFetchNextByte (
    IN PDECODE_CONTEXT DecodeContext,
    OUT PUCHAR NextByte
    );

NTSTATUS
KiOpIsPrefix (
    IN PDECODE_CONTEXT DecodeContext,
    IN UCHAR OpByte,
    OUT PBOOLEAN IsPrefix
    );

NTSTATUS
KiOpLocateDecodeEntry (
    IN OUT PDECODE_CONTEXT DecodeContext
    );

ULONG_PTR
KiOpSingleProcCopy (
    IN ULONG_PTR Context
    );

NTSTATUS
KiOpPatchCode (
    IN PDECODE_CONTEXT DecodeContext,
    OUT PUCHAR Destination,
    IN UCHAR Replacement
    );

BOOLEAN
KiOpPreprocessAccessViolation (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    );

//
// Two separate opcode tables, one for single byte opcodes and another
// for double byte opcodes.
//
// Entries may appear in any order.
//

DECODE_ENTRY const
KiOpOneByteTable[] = {
    { 0xfa, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // CLI
    { 0xfb, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // STI
    { 0xe4, 4,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // IN, OUT imm8
    { 0xec, 4,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // IN, OUT [dx]
    { 0x6c, 4,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // INS, OUTS
    { 0x9E, 2,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_LSAHF     },  // LAHF / SAHF  
    { 0xf4, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // HLT
    { 0xf6, 1,                 0,           6, OPF_MODRM, KiOp_Div       },  // DIV ax/(reg/mem8)
    { 0xf6, 1,                 0,           7, OPF_MODRM, KiOp_Div       },  // IDIV ax/(reg/mem8)
    { 0xf7, 1,                 0,           6, OPF_MODRM, KiOp_Div       },  // DIV [r|e]dx:[r|e]ax/(reg/mem[16|32|64])
    { 0xf7, 1,                 0,           7, OPF_MODRM, KiOp_Div       }   // IDIV [r|e]dx:[r|e]ax/(reg/mem[16|32|64])
};


DECODE_ENTRY const
KiOpTwoByteTable[] = {
    { 0x6F, 1, OP_PREFIX_OPERAND, OP_NOREGEXT, OPF_MODRM, KiOp_MOVDQA    },  // MOVDQA xmm1,xmm2/mem128
    { 0x7F, 1, OP_PREFIX_OPERAND, OP_NOREGEXT, OPF_MODRM, KiOp_MOVDQA    },  // MOVDQA xmm1/mem128,xmm2
    { 0x28, 1,                 0, OP_NOREGEXT, OPF_MODRM, KiOp_MOVAPS    },  // MOVAPS xmm1,xmm2/mem128
    { 0x29, 1,                 0, OP_NOREGEXT, OPF_MODRM, KiOp_MOVAPS    },  // MOVAPS xmm1,xmm2/mem128
    { 0x00, 1,                 0,           2, OPF_NONE,  KiOp_Priv      },  // LLDT
    { 0x00, 1,                 0,           3, OPF_NONE,  KiOp_Priv      },  // LTR
    { 0x01, 1,                 0,           2, OPF_NONE,  KiOp_Priv      },  // LGDT
    { 0x01, 1,                 0,           3, OPF_NONE,  KiOp_Priv      },  // LIGT
    { 0x01, 1,                 0,           6, OPF_NONE,  KiOp_Priv      },  // LMSW
    { 0x01, 1,                 0,           7, OPF_NONE,  KiOp_Priv      },  // INVLPG, SWAPGS
    { 0x08, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // INVD
    { 0x09, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // WBINVD
    { 0x0D, 1,                 0,           0, OPF_MODRM, KiOp_PREFETCH3 },  // PREFETCH
    { 0x0D, 1,                 0,           1, OPF_MODRM, KiOp_PREFETCH3 },  // PREFETCHW
    { 0x18, 1,                 0,           0, OPF_MODRM, KiOp_PREFETCHx },  // PREFETCH0
    { 0x18, 1,                 0,           1, OPF_MODRM, KiOp_PREFETCHx },  // PREFETCH1
    { 0x18, 1,                 0,           2, OPF_MODRM, KiOp_PREFETCHx },  // PREFETCH2
    { 0x18, 1,                 0,           3, OPF_MODRM, KiOp_PREFETCHx },  // PREFETCH3
    { 0x1f, 1,                 0,           1, OPF_MODRM, KiOp_PREF_NOP  },  // PREFETCH_NOP
    { 0x35, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // SYSEXIT
    { 0x20, 4,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // MOV to/from control or dbg reg
    { 0x30, 4,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // WRMSR, RDTSC, RDMSR, RDPMC
    { 0x06, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      },  // CLTS
    { 0xAA, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Illegal   },  // RSM
    { 0x07, 1,                 0, OP_NOREGEXT, OPF_NONE,  KiOp_Priv      }   // SYSRET
};

DECLSPEC_NOINLINE
BOOLEAN
KiPreprocessFault (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    A wide variety of exception conditions can result in a general
    protection fault.

    This routine attempts to further determine the cause of the fault
    and take appropriate action, which can include

        - Updating the context record with a more appropriate status

        - Emulating the instruction or otherwise modifying the caller's
          context and/or istream and returning

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

    TrapFrame - Supplies a pointer to the trap frame.

    PreviousMode - Supplies the execution mode at the time of the exception.

Return Value:

    TRUE - the caller's context should be updated and control returned
           to the caller.

    FALSE - the exception should be raised with (perhaps) an updated
            context record.

--*/

{
    DECODE_CONTEXT decodeContext;
    PDECODE_ENTRY decodeEntry;
    ULONG faultType;
    BOOLEAN retry;
    NTSTATUS status;

    //
    // Determine whether this is an internal exception and if so replace
    // it with the canonical one and continue processing.
    //

    switch (ExceptionRecord->ExceptionCode) {
        
        case KI_EXCEPTION_GP_FAULT:

            faultType = OPF_GP;
            ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
            break;

        case KI_EXCEPTION_INVALID_OP:
            faultType = OPF_INVAL;
            ExceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
            break;

        case KI_EXCEPTION_INTEGER_DIVIDE_BY_ZERO:
            faultType = OPF_DIV;
            ExceptionRecord->ExceptionCode = STATUS_INTEGER_DIVIDE_BY_ZERO;
            break;

        case KI_EXCEPTION_ACCESS_VIOLATION:
            retry = KiOpPreprocessAccessViolation(ExceptionRecord,
                                                  ContextRecord);
            if (retry != FALSE) {
                return TRUE;
            }
            faultType = OPF_AV;
            ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
            break;

        default:
            return FALSE;
    }

    //
    // Decode the offending instruction.  This tells us the length of the
    // instruction.
    //

    status = KiOpDecode(ExceptionRecord,
                        ContextRecord,
                        PreviousMode,
                        &decodeContext);

    //
    // Now that the instruction has been decoded, determine whether
    // any code patches have taken place between the time of the exception
    // and now.
    //
    // If so, re-execute the code that generated the exception.  If it was
    // not this code that was patched, then another exception will fire and
    // we'll try again.
    //

    if (TrapFrame->CodePatchCycle != KiCodePatchCycle) {
        return TRUE;
    }

    if (NT_SUCCESS(status)) {
    
        //
        // If this is an interesting instruction (i.e. it appears in the
        // decode tables) then call its handler.
        // 
    
        decodeEntry = decodeContext.DecodeEntry;
        if (decodeEntry != NULL) {
            status = decodeEntry->Handler(&decodeContext);
        }
    } else {
        decodeEntry = NULL;
    }
    
    //
    // Indicate whether the operation should be retried, and return.
    //

    if (NT_SUCCESS(status) && (decodeEntry != NULL)) {

        retry = decodeContext.Retry;
                       
    } else {

        if (faultType == OPF_GP) {
    
            //
            // If a specific action was not carried out for a general
            // protection fault then the default action is to raise an
            // access violation.
            //
    
            //
            // Set two parameters to "read", "unknown faulting VA"
            //
    
            ExceptionRecord->NumberParameters = 2;
            ExceptionRecord->ExceptionInformation[0] = 0;
            ExceptionRecord->ExceptionInformation[1] = (ULONG_PTR)-1;
        }

        retry = FALSE;
    }

    if ((retry == FALSE) && (faultType == OPF_AV)) {

        //
        // Check for an ATL thunk
        //

        if (NT_SUCCESS(KiCheckForAtlThunk(&decodeContext))) {
            retry = decodeContext.Retry;
        }
    }

    return retry;
}

NTSTATUS
KiOpDecode (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This routine decodes the instruction stream sufficiently to positively
    determine whether this is an opcode we are interested in and, if so,
    to positively identify it.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

    PreviousMode - Supplies the execution mode at the time of the exception.

    DecodeContext - Supplies a pointer to an uninitialized DECODE_CONTEXT.


Return Value:

    NTSTATUS - Status of the operation.

--*/

{
    CHAR byteVal;
    PDECODE_ENTRY decodeEntry;
    BOOLEAN isPrefix;
    LONG longVal;
    UCHAR opByte;
    ULONG prefixLength;
    PUCHAR rip;
    NTSTATUS status;
    SHORT shortVal;

    //
    // Initialize the decode context according to the parameters supplied.
    //
    // Also, make sure the instruction pointer refers to the user mode
    // data area.
    //

    RtlZeroMemory(DecodeContext,sizeof(*DecodeContext));

    rip = (PUCHAR)ContextRecord->Rip;

    //
    // Make sure the faulting address is valid.  Length of the region
    // is 1, as we don't yet know how the true length of the opcode.
    //

    if (PreviousMode == UserMode) {
        try {
            ProbeAndReadUchar(rip);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    DecodeContext->Start = rip;
    DecodeContext->Next = rip;
    DecodeContext->ExceptionRecord = ExceptionRecord;
    DecodeContext->ContextRecord = ContextRecord;
    DecodeContext->PreviousMode = PreviousMode;

    //
    // Record whether the fault occured in a compatibility mode segment.
    //

    if (ContextRecord->SegCs == (KGDT64_R3_CMCODE | RPL_MASK)) {

        ASSERT( PreviousMode == UserMode );
        DecodeContext->CompatibilityMode = TRUE;
    }

    //
    // Accumulate all of the prefix bytes
    //

    prefixLength = 0;
    do {

        status = KiOpFetchNextByte(DecodeContext,&opByte);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        status = KiOpIsPrefix(DecodeContext,opByte,&isPrefix);
        if (!NT_SUCCESS(status)) {
            return status;
        }

    } while (isPrefix != FALSE);

    //
    // opByte contains the first base byte of the opcode.  If this is
    // a two byte opcode, retrieve the second byte.
    //

    if (opByte == OP_TWOBYTE) {

        DecodeContext->TwoByte = TRUE;

        status = KiOpFetchNextByte(DecodeContext,&opByte);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    DecodeContext->OpCodeLocation = DecodeContext->Next - 1;
    DecodeContext->OpCode = opByte;
    status = KiOpLocateDecodeEntry(DecodeContext);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    //
    // If there was no decode entry then we are not interested in handling
    // this instruction.  This is not an error condition, so return
    // success.
    //

    decodeEntry = DecodeContext->DecodeEntry;
    if (decodeEntry == NULL) {
        return STATUS_SUCCESS;
    }

    //
    // Decode any address portion of the instruction
    // 

    if ((decodeEntry->Flags & OPF_MODRM) != 0) {
        status = KiOpDecodeModRM(DecodeContext);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    //
    // Followed by a possible 8-, 16- or 32-bit immediate
    //

    if ((decodeEntry->Flags & OPF_IMM8) != 0) {

        //
        // 8-bit displacement
        //

        status = KiOpFetchBytes(DecodeContext,
                                sizeof(byteVal),
                                &byteVal);
        if (!NT_SUCCESS(status)) {
            return status;
        }
        DecodeContext->Immediate = byteVal;

    } else if ((DecodeContext->DecodeEntry->Flags & OPF_IMM32) != 0) {

        if ((DecodeContext->PrefixMask & OP_PREFIX_OPERAND) != 0) {

            //
            // 16-bit immediate
            //

            status = KiOpFetchBytes(DecodeContext,
                                    sizeof(shortVal),
                                    &shortVal);
            if (!NT_SUCCESS(status)) {
                return status;
            }
            DecodeContext->Immediate = shortVal;

        } else {

            //
            // 32-bit immediate
            //
    
            status = KiOpFetchBytes(DecodeContext,
                                    sizeof(longVal),
                                    &longVal);
            if (!NT_SUCCESS(status)) {
                return status;
            }
            DecodeContext->Immediate = longVal;
        }
    }

    return status;
}


NTSTATUS
KiOpFetchBytes (
    IN PDECODE_CONTEXT DecodeContext,
    IN ULONG Count,
    OUT PVOID Buffer
    )

/*++

Routine Description:

    This routine safely retrieves bytes from the istream specified
    in DecodeContext.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT.

    Count - Supplies the number of bytes to capture from the instruction stream.

    Buffer - Supplies a pointer to the memory that is to contain the captured
             bytes.

Return Value:

    NTSTATUS - Status of the operation.

--*/

{
    NTSTATUS status;

    status = STATUS_SUCCESS;
    try {
        RtlCopyMemory(Buffer, DecodeContext->Next, Count);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }
    if (!NT_SUCCESS(status)) {
        return status;
    }
    DecodeContext->Next += Count;

    return STATUS_SUCCESS;
}

NTSTATUS
KiOpFetchNextByte (
    IN PDECODE_CONTEXT DecodeContext,
    OUT PUCHAR NextByte
    )

/*++

Routine Description:

    This routine safely retrieves a single byte from the istream specified
    in DecodeContext.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT.

    NextByte - Supplies a pointer to the memory that is to contain the captured
               byte.

Return Value:

    NTSTATUS - Status of the operation.

--*/

{
    return KiOpFetchBytes(DecodeContext, 1, NextByte);
}


NTSTATUS
KiOpIsPrefix (
    IN PDECODE_CONTEXT DecodeContext,
    IN UCHAR OpByte,
    OUT PBOOLEAN IsPrefix
    )

/*++

Routine Description:

    This routine determines whether the supplied byte is a prefix byte and
    updates DecodeContext accordingly.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

    OpByte - The possible prefix byte.

    IsPrefix - Supplies a pointer to the BOOLEAN that will be set according
               to whether OpByte is a prefix byte or not.

Return Value:

    NTSTATUS - Status of the operation.

    This routine will fail if an illegal instruction is detected.

--*/

{
    ULONG groupMask;
    ULONG index;
    BOOLEAN isPrefix;
    ULONG prefixMask;
    NTSTATUS status;

    status = STATUS_SUCCESS;
    isPrefix = FALSE;
    prefixMask = 0;
    groupMask = 0;

    //
    // Check for REX prefix
    //

    if ((DecodeContext->CompatibilityMode == FALSE) &&
        (OpByte & 0xf0) == 0x40) {

        prefixMask = OP_PREFIX_REX;
        DecodeContext->Rex.Byte = OpByte;

    } else {

        //
        // Check for other prefix bytes
        //

        for (index = 0; index < RTL_NUMBER_OF(KiOpPrefixTable); index += 1) {

            if (KiOpPrefixTable[index].Prefix == OpByte) {

                prefixMask = KiOpPrefixTable[index].Flag;
                groupMask = KiOpPrefixTable[index].Group;
                break;
            }
        }
    }

    if (prefixMask != 0) {

        //
        // A prefix byte has been found.  If this prefix byte has already
        // appeared in this instruction, or belongs to a group that has
        // already been represented by a previous prefix, then this is
        // an illegal instruction.
        // 

        if (((DecodeContext->PrefixMask & prefixMask) != 0) ||
            ((DecodeContext->GroupMask & groupMask) != 0)) {

            status = STATUS_ILLEGAL_INSTRUCTION;

        } else {

            DecodeContext->PrefixMask |= prefixMask;
            DecodeContext->GroupMask |= groupMask;
            isPrefix = TRUE;
        }
    }

    if (NT_SUCCESS(status)) {
        *IsPrefix = isPrefix;
    }

    return status;
}


NTSTATUS
KiOpLocateDecodeEntry (
    IN OUT PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    Given a decoded instruction, this routine attempts to locate the
    DECODE_ENTRY structure and update DecodeContext accordingly.

    Some opcodes require the rem field in the ModRM byte to be consulted
    before they can be identified.  In that case, this routine will
    fetch the ModRM byte from the instruction stream.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    NTSTATUS - Status of the operation.  Success unless a necessary
               ModRM byte could not be fetched from the istream.

--*/

{
    PDECODE_ENTRY decodeEntry;
    UCHAR opBase;
    UCHAR opLimit;
    NTSTATUS status;
    PDECODE_ENTRY tableEnd;

    if (DecodeContext->TwoByte == FALSE) {

        decodeEntry = KiOpOneByteTable;
        tableEnd = &decodeEntry[RTL_NUMBER_OF(KiOpOneByteTable)];

    } else {

        decodeEntry = KiOpTwoByteTable;
        tableEnd = &decodeEntry[RTL_NUMBER_OF(KiOpTwoByteTable)];
    }

    //
    // For each DECODE_ENTRY in the array, determine whether it matches
    // the current instruction.
    //

    decodeEntry -= 1;
    while (TRUE) {

        decodeEntry += 1;
        if (decodeEntry == tableEnd) {
            return STATUS_SUCCESS;
        }

        //
        // Check whether the opcode in question falls within the opcode
        // range represented by this DECODE_ENTRY
        //

        opBase = decodeEntry->OpCode;
        opLimit = opBase + decodeEntry->Range - 1;

        if ((DecodeContext->OpCode < opBase) ||
            (DecodeContext->OpCode > opLimit)) {
            continue;
        }

        //
        // Check whether this opcode relies on a specific prefix for
        // further identification and if so, whether the current
        // instruction stream includes that prefix.
        //

        if ((decodeEntry->Prefix != 0) &&
            (decodeEntry->Prefix & DecodeContext->PrefixMask) == 0) {
            continue;
        }

        //
        // Check whether this opcode relies on a specific value within
        // ModRM.reg and if so, whether the current instruction stream
        // includes that value.
        //

        if (decodeEntry->ModRMExtension != OP_NOREGEXT) {

            //
            // The opcode is defined to include a ModRM byte.  Read
            // it here if it hasn't yet been read.
            //

            if (DecodeContext->ModRMRead == FALSE) {
                status = KiOpFetchNextByte(DecodeContext,
                                           &DecodeContext->ModRM.Byte);
                if (!NT_SUCCESS(status)) {
                    return status;
                }
                DecodeContext->ModRMRead = TRUE;
            }

            if (decodeEntry->ModRMExtension != DecodeContext->ModRM.reg) {
                continue;
            }
        }

        //
        // All tests have passed, we have a match.
        // 

        DecodeContext->DecodeEntry = decodeEntry;
        return STATUS_SUCCESS;
    }
}

NTSTATUS
KiOpDecodeModRM (
    IN OUT PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This routine retrieves and parses the ModRM portion of the opcode and,
    optionally, an SIB byte and 8-, 16- or 32-bit offset.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT.

Return Value:

    NTSTATUS - Status of the operation.

--*/

{
    CHAR byteVal;
    NTSTATUS status;

    //
    // It is assumed that the opcode itself has been decoded.  It is also
    // possible that the ModRM byte itself has already been retrieved as
    // a result of further refining the opcode search.
    //

    if (DecodeContext->ModRMRead == FALSE) {

        status = KiOpFetchNextByte(DecodeContext,
                                   &DecodeContext->ModRM.Byte);
        if (!NT_SUCCESS(status)) {
            return status;
        }
        DecodeContext->ModRMRead = TRUE;
    }

    //
    // Next comes the SIB byte, if one exists
    //

    if (DecodeContext->ModRM.mod != 0x03 &&
        DecodeContext->ModRM.rm == 0x4) {

        //
        // The opcode contains an SIB byte as well
        //

        status = KiOpFetchNextByte(DecodeContext,
                                   &DecodeContext->SIB.Byte);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    //
    // Possibly followed by an 8- or 32-bit displacement.
    //
    // Mod == 2          -> 32-bit displacement
    // Mod == 0, rm == 5 -> 32-bit displacement
    // Mod == 1          ->  8-bit displacement
    //

    if (((DecodeContext->ModRM.mod == 0) && (DecodeContext->ModRM.rm == 5)) ||
        (DecodeContext->ModRM.mod == 2)) {

        //
        // 32-bit displacent
        //

        status = KiOpFetchBytes(DecodeContext,
                                sizeof(LONG), 
                                &DecodeContext->Displacement);
        if (!NT_SUCCESS(status)) {
            return status;
        }

    } else if (DecodeContext->ModRM.mod == 1) {

        //
        // 8-bit displacement
        //

        status = KiOpFetchBytes(DecodeContext,
                                sizeof(byteVal),
                                &byteVal);
        if (!NT_SUCCESS(status)) {
            return status;
        }
        DecodeContext->Displacement = byteVal;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
KiOpRetrieveRegMemAddress (
    IN OUT PDECODE_CONTEXT DecodeContext,
    OUT PVOID *OperandAddress,
    OUT KPROCESSOR_MODE *ProcessorMode
    )

/*++

Routine Description:

    After an instruction has been decoded, this routine calculates the
    address of an instruction's rm target.

    If rm specifies a register, OperandAddress will point to the location
    of that register within the context record.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT.

    OperandAddress - Supplies a pointer to the memory to receive the
                     calculated rm operand address.

    ProcessorMode - Supplies a pointer to the memory to receive the probe
                    mode for the OperandAddress pointer.  This may be
                    different than PreviousMode() if the operand is a
                    register.

Return Value:

    NTSTATUS - Status of the operation.

--*/

{
    PVOID address;
    LONG64 base;
    ULONG baseReg;
    LONG64 index;
    ULONG indexReg;
    KPROCESSOR_MODE processorMode;

    ASSERT( (DecodeContext->DecodeEntry->Flags & OPF_MODRM) != 0 );
    ASSERT( DecodeContext->ModRMRead != FALSE );

    processorMode = DecodeContext->PreviousMode;
    base = 0;
    index = 0;

    if (DecodeContext->ModRM.mod == 3) {

        //
        // Not a memory operand, value is found in a register (i.e. in the
        // context record)
        //

        baseReg = DecodeContext->ModRM.rm;
        if (DecodeContext->Rex.B) {
            baseReg += 8;
        }

        base = (LONG64)(&DecodeContext->ContextRecord->Rax + baseReg);
        processorMode = KernelMode;

    } else if (DecodeContext->ModRM.rm == 4) {

        //
        // Opcode includes an SIB byte.
        //

        if ((DecodeContext->SIB.base == 5) &&
            (DecodeContext->ModRM.mod == 0)) {

            NOTHING;

        } else {

            baseReg = DecodeContext->SIB.base;
            if (DecodeContext->Rex.B) {
                baseReg += 8;
            }

            base = *(&DecodeContext->ContextRecord->Rax + baseReg);
        }

        if (DecodeContext->SIB.index == 4) {

            NOTHING;

        } else {

            indexReg = DecodeContext->SIB.index;
            if (DecodeContext->Rex.X) {
                indexReg += 8;
            }

            index = *(&DecodeContext->ContextRecord->Rax + indexReg);
            index <<= DecodeContext->SIB.scale;
        }

    } else {

        baseReg = DecodeContext->ModRM.rm;
        if (DecodeContext->Rex.B) {
            baseReg += 8;
        }
        base = *(&DecodeContext->ContextRecord->Rax + baseReg);
    }

    address = (PVOID)(base + index + DecodeContext->Displacement);

    if ((DecodeContext->ModRM.mod == 0) &&
        (DecodeContext->ModRM.rm == 5)) {

        //
        // Displacement is RIP-relative, add in RIP of next instruction
        // 

        address = (PVOID)((LONG64)address + (LONG64)(DecodeContext->Next));
    }

    *OperandAddress = address;
    *ProcessorMode = processorMode;

    return STATUS_SUCCESS;
}


NTSTATUS
KiOp_Priv (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for instructions for which a PRIVELEGED_INSTRUCTION
    exception should be raised.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PEXCEPTION_RECORD exceptionRecord;

    exceptionRecord = DecodeContext->ExceptionRecord;
    exceptionRecord->ExceptionCode = STATUS_PRIVILEGED_INSTRUCTION;
    exceptionRecord->NumberParameters = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
KiOp_Illegal (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for instructions for which an ILLEGAL_INSTRUCTION
    exception should be raised.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PEXCEPTION_RECORD exceptionRecord;

    exceptionRecord = DecodeContext->ExceptionRecord;
    exceptionRecord->ExceptionCode = STATUS_ILLEGAL_INSTRUCTION;
    exceptionRecord->NumberParameters = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
KiCheckForAtlThunk (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for the C7 /0 opcode: mov reg/mem32, imm32.

    This marks the beginning of an x86 ATL thunk.  Control will branch
    here if the ATL thunk was built in memory marked no execute.  If that
    is the case, then the thunk code will be emulated and control returned
    to the thread.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PEXCEPTION_RECORD exceptionRecord;
    ULONG64 faultIndicator;

    //
    // Interested only in compatibility mode code.
    //

    if (DecodeContext->CompatibilityMode == FALSE) {
        return STATUS_SUCCESS;
    }

    //
    // Interested only in an instruction fetch fault.
    // 

    exceptionRecord = DecodeContext->ExceptionRecord;
    faultIndicator = exceptionRecord->ExceptionInformation[0];
    if ((faultIndicator & 0x8) == 0) {
        return STATUS_SUCCESS;
    }

    //
    // Where the fault address is the instruction
    // 

    if (exceptionRecord->ExceptionInformation[1] !=
        (ULONG64)(DecodeContext->Start)) {
        return STATUS_SUCCESS;
    }

    if (KiEmulateAtlThunk((ULONG *)&DecodeContext->ContextRecord->Rip,
                          (ULONG *)&DecodeContext->ContextRecord->Rsp,
                          (ULONG *)&DecodeContext->ContextRecord->Rax,
                          (ULONG *)&DecodeContext->ContextRecord->Rcx,
                          (ULONG *)&DecodeContext->ContextRecord->Rdx)) {

        //
        // An ATL thunk was recognized and emulated.
        // Indicate "resume execution".
        //

        DecodeContext->Retry = TRUE;
    }

    return STATUS_SUCCESS;
}

VOID
KiCheckForSListAddress (
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function is called from the APC and DPC interrupt code to check if
    the specified RIP lies within the SLIST pop code. If the specified RIP
    lies within the SLIST code, then RIP is reset to the SLIST resume address.

Arguments:

    TrapFrame - Supplies the address of a trap frame.

Return Value:

    None.

--*/

{

    ULONG64 Rip;

    //
    // Check for user mode 64-bit execution, user mode WOW64 32-bit execution,
    // and kernel mode 64-bit execution.
    //

    Rip = TrapFrame->Rip;
    if (TrapFrame->SegCs == (KGDT64_R3_CODE | RPL_MASK)) {
        if ((Rip > (ULONG64)KeUserPopEntrySListResume) &&
            (Rip <= (ULONG64)KeUserPopEntrySListEnd)) {

            TrapFrame->Rip = (ULONG64)KeUserPopEntrySListResume;
        }

    } else if (TrapFrame->SegCs == (KGDT64_R3_CMCODE | RPL_MASK)) {
        if ((Rip > (ULONG64)KeUserPopEntrySListResumeWow64) &&
            (Rip <= (ULONG64)KeUserPopEntrySListEndWow64)) {

            TrapFrame->Rip = (ULONG64)KeUserPopEntrySListResumeWow64;
        }

    } else if (TrapFrame->SegCs == KGDT64_R0_CODE) {
        if ((Rip > (ULONG64)&ExpInterlockedPopEntrySListResume) &&
            (Rip <= (ULONG64)&ExpInterlockedPopEntrySListEnd)) {

            TrapFrame->Rip = (ULONG64)&ExpInterlockedPopEntrySListResume;
        }
    }

    return;
}


NTSTATUS
KiOp_MOVDQA (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for MOVDQA instructions that resulted in a
    GP fault.  This can occur when supplied with a misaliged memory
    reference.

    This routine essentially performs an "alignment fixup" by replacing
    the instruction with the slower MOVDQU in the user's istream.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PKPROCESS currentProcess;
    PKTHREAD currentThread;
    NTSTATUS status;

    //
    // If the current process is wow64 or has autoalignment disabled, then
    // do not perform the transformation.
    //

    if (DecodeContext->CompatibilityMode != FALSE) {
        return STATUS_SUCCESS;
    }

    currentThread = KeGetCurrentThread();
    currentProcess = currentThread->ApcState.Process;
    if ((currentThread->AutoAlignment == FALSE) &&
        (currentProcess->AutoAlignment == FALSE)) {

        return STATUS_SUCCESS;
    }

    //
    // Replace:
    //
    // 66 0F 6F /r - MOVDQA xmm1,xmm2/mem128
    // 66 0F 7F /r - MOVDQA xmm1/mem128,xmm2
    //
    // With:
    //
    // F3 0F 6F /r - MOVDQU xmm1,xmm2/mem128
    // F3 0F 7F /r - MOVDQU xmm1/mem128,xmm2
    //

    status = KiOpPatchCode(DecodeContext,
                           DecodeContext->Start,
                           0xF3);

    if (NT_SUCCESS(status) || (status == STATUS_RETRY)) {
        DecodeContext->Retry = TRUE;
        status = STATUS_SUCCESS;
    }

    return status;
}


NTSTATUS
KiOp_MOVAPS (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for MOVAPS instructions that resulted in a
    GP fault.  This can occur when supplied with a misaliged memory
    reference.

    This routine essentially performs an "alignment fixup" by replacing
    the instruction with the slower MOVUPS in the user's istream.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PKPROCESS currentProcess;
    PKTHREAD currentThread;
    UCHAR newByte;
    NTSTATUS status;

    //
    // If the current process is wow64 or has autoalignment disabled, or
    // if the fault occured in kernel mode, then do not perform the
    // transformation.
    //

    if ((DecodeContext->CompatibilityMode != FALSE) ||
        (DecodeContext->PreviousMode != UserMode)) {

        return STATUS_SUCCESS;
    }

    currentThread = KeGetCurrentThread();
    currentProcess = currentThread->ApcState.Process;
    if ((currentThread->AutoAlignment == FALSE) &&
        (currentProcess->AutoAlignment == FALSE)) {

        return STATUS_SUCCESS;
    }

    //
    // Replace:
    //
    // 0F 28 /r - MOVAPS xmm1,xmm2/mem128
    // 0F 29 /r - MOVAPS xmm1/mem128,xmm2
    //
    // With:
    //
    // 0F 10 /r - MOVUPS xmm1,xmm2/mem128
    // 0F 11 /r - MOVUPS xmm1/mem128,xmm2
    //

    if (DecodeContext->OpCode == 0x28) {
        newByte = 0x10;
    } else {
        newByte = 0x11;
    }

    status = KiOpPatchCode(DecodeContext,
                           DecodeContext->OpCodeLocation,
                           newByte);

    if (NT_SUCCESS(status) || (status == STATUS_RETRY)) {
        DecodeContext->Retry = TRUE;
        status = STATUS_SUCCESS;
    }

    return status;
}


NTSTATUS
KiOp_PREF_NOP (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for PREF_NOP instructions.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    NTSTATUS exceptionCode;

    //
    // This is the two-byte modrm NOP that was once a PREFETCHW.  If this
    // instruction appears to have generated an illegal instruction fault,
    // it is because another processor had already patched it but the previous
    // encoding was in this processor's ICACHE.
    // 

    exceptionCode = DecodeContext->ExceptionRecord->ExceptionCode;

    if ((exceptionCode == STATUS_ILLEGAL_INSTRUCTION) &&
        (KiCodePatchCycle != 0)) {

        DecodeContext->Retry = TRUE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
KiOp_PREFETCH3 (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for PREFETCHW instructions that are executed
    on processors that do not support this instruction, resulting in
    an invalid opcode exception.

    The strategy for dealing with such an exception is to replace the
    instruction with a mod r/m nop instruction, which has an encoding
    identical to prefetchw except for the opcode itself.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    NTSTATUS exceptionCode;
    ULONG instructionLength;
    NTSTATUS status;

    //
    // The opcode has been decoded.  Transform PREFETCHW into
    // a mod r/m nop by replacing the 0x0D opcode with 0x1f.
    // 

    exceptionCode = DecodeContext->ExceptionRecord->ExceptionCode;
    instructionLength = (ULONG)(DecodeContext->Next -
                                DecodeContext->Start);

    if (exceptionCode == STATUS_ILLEGAL_INSTRUCTION) {

        status = KiOpPatchCode(DecodeContext,
                               DecodeContext->OpCodeLocation,
                               0x1f);
    
        if (!NT_SUCCESS(status)) {
    
            //
            // If the opcode could not be patched then simply
            // skip the instruction.
            //

            DecodeContext->ContextRecord->Rip += instructionLength;
            KiOpPrefetchPatchRetry += 1;
        } else {
            KiOpPrefetchPatchCount += 1;
        }
        DecodeContext->Retry = TRUE;

    } else if (exceptionCode == STATUS_ACCESS_VIOLATION) {

        //
        // Prefetch instructions should never generate an AV.  Advance
        // past the instruction.
        //

        DecodeContext->ContextRecord->Rip += instructionLength;
        DecodeContext->Retry = TRUE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
KiOp_PREFETCHx (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for the PREFETCHx instructions.

    The strategy for dealing with such an exception is to advance the
    instruction pointer past the instruction.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    NTSTATUS exceptionCode;
    ULONG instructionLength;

    exceptionCode = DecodeContext->ExceptionRecord->ExceptionCode;
    if (exceptionCode == STATUS_ACCESS_VIOLATION) {

        //
        // Prefetch instructions should never generate an AV.  Advance
        // past the instruction.
        //

        instructionLength = (ULONG)(DecodeContext->Next -
                                    DecodeContext->Start);

        DecodeContext->ContextRecord->Rip += instructionLength;
        DecodeContext->Retry = TRUE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
KiOp_Div (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for the integer DIV and IDIV instructions.

    The AMD64 processor generates a divide-by-zero fault when either the
    divisor operand is zero, or when the quotient is too large for the
    destination register.

    In the former case a STATUS_INTEGER_DIVIDE_BY_ZERO exception is raised,
    while in the latter case a STATUS_INTEGER_OVERFLOW should be raised.

    It is the job of this routine to determine which exception to raise.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    NTSTATUS status;
    KPROCESSOR_MODE captureMode;
    PVOID divisorAddress;
    LONG64 divisor;
    ULONG divisorSize;
    NTSTATUS *exceptionCode;

    if (KiFilterFiberContext( DecodeContext->ContextRecord ) != FALSE) {

        DecodeContext->Retry = TRUE;
        return STATUS_SUCCESS;
    }

    exceptionCode = &DecodeContext->ExceptionRecord->ExceptionCode;
    if (*exceptionCode != STATUS_INTEGER_DIVIDE_BY_ZERO) {

        //
        // No further processing for any other exception.
        //

        return STATUS_SUCCESS;
    }

    //
    // Raise STATUS_INTEGER_DIVIDE_BY_ZERO unless we are able to determine
    // that the divisor was non-zero.
    //

    status = KiOpRetrieveRegMemAddress( DecodeContext,
                                        &divisorAddress,
                                        &captureMode );
    if (NT_SUCCESS(status)) {

        if (DecodeContext->ModRM.reg == 6) {

            //
            // 8 bit divisor
            //

            divisorSize = 1;

        } else if ((DecodeContext->PrefixMask & OP_PREFIX_OPERAND) != 0) {

            //
            // 16 bit divisor
            //

            divisorSize = 2;

        } else if ((DecodeContext->Rex.W == 0)) {

            //
            // 32 bit divisor
            //

            divisorSize = 4;

        } else {

            //
            // 64 bit divisor
            //

            divisorSize = 8;
        }

        divisor = 0;
        try {

            if (captureMode == UserMode) {
                ProbeForRead(divisorAddress,divisorSize,1);
            } 

            switch (divisorSize) {
                case 8:
                    divisor = *(PULONG64)divisorAddress;
                    break;

                case 4:
                    divisor = *(PULONG)divisorAddress;
                    break;

                case 2:
                    divisor = *(PUSHORT)divisorAddress;
                    break;

                case 1:
                    divisor = *(PUCHAR)divisorAddress;
                    break;

                default:
                    __assume(FALSE);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (NT_SUCCESS(status) && (divisor != 0)) {

            //
            // The content of the divisor was successfully read and
            // determined to be non-zero.
            //

            *exceptionCode = STATUS_INTEGER_OVERFLOW;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
KiOp_LSAHF (
    IN PDECODE_CONTEXT DecodeContext
    )

/*++

Routine Description:

    This is the handler for the LAHF and SAHF instructions.

    Early versions of the AMD64 processor (and, currently, all versions
    of the EMT64 processor) generate an invalid instruction trap in response
    to executing one of these instructions.

    If previousmode was user, this routine emulates the instruction and
    returns to the caller.

    Otherwise, the exception is forwarded.

    N.B. These instructions will almost always be handled in a more direct
         fashion by KiProcessInvalidOpcodeFault().  Only in the case of a
         more complex opcode (prefixes, etc.) will control make it to
         this point.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT

Return Value:

    STATUS_SUCCESS

--*/

{
    PUCHAR AH;
    PUCHAR FL;
    ULONG instructionLength;

    //
    // Emulates a LAHF/SAHF instruction.
    //

    if (DecodeContext->ExceptionRecord->ExceptionCode !=
        STATUS_ILLEGAL_INSTRUCTION) {

        //
        // No further processing for any other exception.
        //

        return STATUS_SUCCESS;
    }

    AH = ((PUCHAR)&(DecodeContext->ContextRecord->Rax)) + 1;
    FL = (PUCHAR)&(DecodeContext->ContextRecord->EFlags);

    if (DecodeContext->OpCode == 0x9E) {

        //
        // Emulate SAHF
        //

        *FL = *AH;

    } else {

        //
        // Emulate LAHF
        //

        *AH = *FL;
    }

    instructionLength = (ULONG)(DecodeContext->Next -
                                DecodeContext->Start);

    DecodeContext->ContextRecord->Rip += instructionLength;
    DecodeContext->Retry = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS
KiOpPatchCode (
    IN PDECODE_CONTEXT DecodeContext,
    IN PUCHAR Destination,
    IN UCHAR Replacement
    )

/*++

Routine Description:

    This routine copies the specified data into the opcode that generated the
    exception.  This routine can be used to patch any ISTREAM that was executing
    below SYNCH_LEVEL.

    Patching the ISTREAM must be done with great care, there are several things
    to consider:

    1. No other CPU must be permitted to execute code while it is being patched.

    2. It is possible that P1 will patch the code while P0 is processing the
       exception generated as a result of executing the unpatched copy.  This
       situation must be dealt with correctly.

    3. Any pageable code must not be marked "dirty" as a result of the patch, as
       doing so will result in additional paging activity (i.e. it is cheaper to
       re-patch code that has been discarded than it is to page it out.)

    4. The code may lie within a read-only page.

    Solution 1: The code modification is made within a callback called from
    KeIpiGenericCall().  This ensures that any other processors are spinning
    within an IPI while the code modification is occurring.

    Solution 2: The global KiCodePatchCycle is used to deal with (2).  Each
    trap frame generated as a result of a general protection fault or an illegal
    instruction trap will include the value of KiCodePatchCycle as part of the
    trap frame.

    After the instruction is decoded, the global KiCodePatchCycle is
    compared with that captured in the trap frame.  If they differ, then the
    possibility exists that another processor has already patched the
    instruction after it generated a trap on this processor but before it was
    decoded.  Therefore, execution is retried.

    Solution 3,4: An MDL is made for the target address and the patch is
    performed through the aliased system address.

Arguments:

    DecodeContext - Supplies a pointer to the current DECODE_CONTEXT.

    Destination - Supplies a pointer to the copy destination.

    Replacement - Supplies the replacement byte value

Return Value:

    NTSTATUS - Status of operation

--*/

{
    PVOID baseAddress;
    KIOP_COPY_CONTEXT copyContext;
    PUCHAR dst;
    LARGE_INTEGER interval;
    struct {
        MDL mdl;
        PFN_NUMBER pfnArray[2];
    } mdl;
    KIRQL oldIrql;
    ULONG oldProtection;
    UCHAR reentryCount;
    SIZE_T regionSize;
    NTSTATUS status;
    PKTHREAD thread;
    static LONG64 spinLock = 0;

    thread = KeGetCurrentThread();

    reentryCount = ++(thread->CodePatchInProgress);
    if (reentryCount == 1) {

        //
        // Take the patch lock.  If not available, return indicating retry.
        //

        if (InterlockedCompareExchange64(&spinLock, (LONG64)thread, 0) != 0) {

            if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
    
                interval.QuadPart = -5 * 1000 * 10;     // 5ms, relative
                KeDelayExecutionThread( KernelMode,
                                        FALSE,
                                        &interval );
            }

            status = STATUS_RETRY;
            goto errorExit;
        }
    }

    if (DecodeContext->PreviousMode == UserMode) {

        //
        // Ensure that the target pages are writable
        // 

        baseAddress = Destination;
        regionSize = sizeof(UCHAR);

        status = ZwProtectVirtualMemory(NtCurrentProcess(),
                                        &baseAddress,
                                        &regionSize,
                                        PAGE_EXECUTE_READWRITE,
                                        &oldProtection);
        if (!NT_SUCCESS(status)) {
            goto errorExit;
        }

    }

    //
    // If the fault occured at IRQL < DISPATCH_LEVEL (whether in user-
    // or kernel-mode) then build a locked, system address for the target
    // so that the memory can be manipulated from within an IPI.
    //

    if (InitializationPhase == 0) {

        //
        // If init phase 0 hasn't been completed then it is not yet safe
        // to call MmProbeAndLockPages() or acquire a spinlock.
        //
        // However at this stage, these things can be presumed:
        //
        // - Only a single processor is running
        // - All code pages are writeable
        // - The fault occured in kernel mode
        //

        dst = Destination;
        copyContext.CopyDirect = TRUE;

    } else if (MmCanThreadFault() &&
               (reentryCount == 1) &&
               ((GetCallersEflags() & EFLAGS_IF_MASK) != 0)) {

        //
        // Build a system address for the target, the data will be
        // copied directly to that.
        //

        memset(&mdl,0,sizeof(mdl));
        MmInitializeMdl(&mdl.mdl, Destination, sizeof(UCHAR));
        status = STATUS_SUCCESS;
        try {
            MmProbeAndLockPages(&mdl.mdl,
                                DecodeContext->PreviousMode,
                                IoReadAccess);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
        }

        if (!NT_SUCCESS(status)) {
            goto revertProtection;
        }

        dst = MmGetSystemAddressForMdlSafe(&mdl.mdl,
                                           NormalPagePriority);
        if (dst == NULL) {
            MmUnlockPages(&mdl.mdl);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto revertProtection;
        }
        copyContext.CopyDirect = TRUE;

    } else {

        //
        // The fault occured at IRQL >= DISPATCH_LEVEL or with interrupts
        // disabled, in which case we know that the target is a locked,
        // kernel-mode address so MmDbgCopyMemory() will be used directly
        // on the destination.
        //

        dst = Destination;
        copyContext.CopyDirect = FALSE;
    }

    //
    // Further initialize copyContext and invoke KiOpSingleProcCopy()
    // simultaneously on all processors.
    // 

    copyContext.Replacement = Replacement;
    copyContext.Destination = dst;
    copyContext.CopyPerformed = FALSE;
    copyContext.Status = STATUS_SUCCESS;

#if defined(NT_UP)

    KeRaiseIrql(IPI_LEVEL-2,&oldIrql);
    KiOpSingleProcCopy((ULONG_PTR)&copyContext);
    KeLowerIrql(oldIrql);

#else

    copyContext.Done = FALSE;
    copyContext.ProcessorNumber = KeGetCurrentProcessorNumber();

    if ((KeNumberProcessors > 1) && (reentryCount <= 2)) {

        thread->CodePatchInProgress += 2;
        copyContext.ProcessorsRunning = KeNumberProcessors-1;
        copyContext.ProcessorsToResume = KeNumberProcessors-1;
        KeIpiGenericCall(KiOpSingleProcCopy,(ULONG_PTR)&copyContext);
        thread->CodePatchInProgress -= 2;

    } else {

        oldIrql = KeGetCurrentIrql();
        if (oldIrql < (IPI_LEVEL-1)) {
            KfRaiseIrql(IPI_LEVEL-1);
        }
        copyContext.ProcessorsRunning = 0;
        copyContext.ProcessorsToResume = 0;
        KiOpSingleProcCopy((ULONG_PTR)&copyContext);
        KeLowerIrql(oldIrql);
    }

#endif

    ASSERT(NT_SUCCESS(copyContext.Status));

    if (dst != Destination) {
        MmUnlockPages(&mdl.mdl);
    }

    if (copyContext.CopyPerformed) {
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_RETRY;
    }

revertProtection:

    if (DecodeContext->PreviousMode == UserMode) {

        //
        // Restore the page protection
        // 

        ZwProtectVirtualMemory(NtCurrentProcess(),
                               &baseAddress,
                               &regionSize,
                               oldProtection,
                               &oldProtection);
    }

    if (reentryCount == 1) {
        InterlockedAnd64( &spinLock, 0 );
    }

errorExit:
    thread->CodePatchInProgress -= 1;

    return status;
}


ULONG_PTR
KiOpSingleProcCopy (
    IN ULONG_PTR Context
    )

/*++

Routine Description:

    This routine is invoked via KeIpiGenericCall() on each processor in the
    system.  It's purpose is to ensure that one processor is modifying code
    at a time when all of the other processors are known to be not executing
    that code.

Arguments:

    Context - Supplies a pointer to the KIOP_COPY_CONTEXT structure.

Return Value:

    0

--*/
{
    PKIOP_COPY_CONTEXT copyContext;
    ULONG64 dst;
    PUCHAR src;
    NTSTATUS status;

    copyContext = (PKIOP_COPY_CONTEXT)Context;

#if !defined(NT_UP)

    if (copyContext->ProcessorNumber != KeGetCurrentProcessorNumber()) {

        //
        // All processors but one wait here until the routine is complete
        //

        InterlockedDecrement(&copyContext->ProcessorsRunning);
        while (copyContext->Done == FALSE) {
            KeYieldProcessor();
        }

        //
        // The working processor has indicated that the operation
        // is complete, acknowledge and return.
        //

        InterlockedDecrement(&copyContext->ProcessorsToResume);
        return 0;
    }

    //
    // This processor will do the work.  First wait until the other
    // processors are spinning.
    // 

    while (copyContext->ProcessorsRunning != 0) {
        KeYieldProcessor();
    }

#endif

    if (copyContext->CopyDirect != FALSE) {

        //
        // The target has been mapped to a system va, copy into that
        // directly.
        // 

        *(copyContext->Destination) = copyContext->Replacement;

        status = STATUS_SUCCESS;

    } else {

        //
        // The target could not be mapped to a system va.  Fortunately
        // means that the target is already locked, system VA.  However,
        // it may be on a read only page, so MmDbgCopyMemory() must be
        // used.
        //

        dst = (ULONG64)copyContext->Destination;
        src = &copyContext->Replacement;

        status = MmDbgCopyMemory(dst,
                                 src,
                                 sizeof(UCHAR),
                                 MMDBG_COPY_WRITE | MMDBG_COPY_UNSAFE);
    }

    //
    // If the copy was performed successfully, so indicate
    // and increment the KiCodePatchCycle global.
    // 

    if (NT_SUCCESS(status)) {
        copyContext->CopyPerformed = TRUE;
        KiCodePatchCycle += 1;
    } else {
        copyContext->Status = status;
    }

    //
    // Finished with the code page modifications, so release the other
    // processors and spin until they acknowledge.
    //

#if !defined(NT_UP)

    copyContext->Done = TRUE;
    while (copyContext->ProcessorsToResume != 0) {
        KeYieldProcessor();
    }

#endif

    return 0;
}


NTSTATUS
KiPreprocessKernelAccessFault (
    IN ULONG FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine is invoked from KiPageFault when these conditions are true:

    - The fault was not an execution fault
    - The execution mode at the time of the fault was kernel mode

    This routine will examine the faulting opcode.  If it is a prefetch
    instruction, then execution is resumed.

Arguments:

    FaultStatus - Supplies fault status information bits.

    VirtualAddress - Supplies the virtual address which caused the fault.

    PreviousMode - Supplies the mode (kernel or user) in which the fault
                   occurred.

    TrapFrame - Supplies a pointer to the KTRAP_FRAME generated by
                the fault handler.

Return Value:

    Returns

        - STATUS_SUCCESS if execution should be resumed
        - An error if normal fault handling should proceed

--*/

{
    LOGICAL prefix;
    PUCHAR rip;

    UNREFERENCED_PARAMETER(FaultStatus);
    UNREFERENCED_PARAMETER(VirtualAddress);
    UNREFERENCED_PARAMETER(PreviousMode);

    //
    // This code is executed on every kernel-mode DSTREAM page fault.
    // Performance is paramount.
    //
    // We wish to determine whether this page fault was generated by
    // a prefetch instruction.  Rather than using the rather heavyweight
    // KiOpDecode(), we use a brute-force decode approach.
    //
    // Note that we know this is a valid instruction, otherwise an invalid
    // opcode trap would have been generated.
    //

    //
    // First, skip any prefix bytes.  This routine is invoked for kernel
    // mode faults only, so compatibility mode code is not an option.
    // 

    rip = (PUCHAR)TrapFrame->Rip;
    do {
        switch (*rip) {        

            //
            // 0x0F is the first byte of a two-byte opcode.  Break out
            // of the loop and further qualify the opcode.
            //

            case 0x0F:
                rip += 1;
                prefix = FALSE;
                break;

            case 0x26:          // ES:
            case 0x2E:          // CS:
            case 0x36:          // SS:
            case 0x3E:          // DS:
            case 0x40:          // REX
            case 0x41:          // REX
            case 0x42:          // REX
            case 0x43:          // REX
            case 0x44:          // REX
            case 0x45:          // REX
            case 0x46:          // REX
            case 0x47:          // REX
            case 0x48:          // REX
            case 0x49:          // REX
            case 0x4A:          // REX
            case 0x4B:          // REX
            case 0x4C:          // REX
            case 0x4D:          // REX
            case 0x4E:          // REX
            case 0x4F:          // REX
            case 0x64:          // FS:
            case 0x65:          // GS:
            case 0x66:          // operand size
            case 0x67:          // address size
            case 0xF0:          // lock
            case 0xF2:          // repne
            case 0xF3:          // repe
                rip += 1;
                prefix = TRUE;
                break;

            default:
                return STATUS_ACCESS_VIOLATION;
        }

    } while (prefix != FALSE);

    //
    // Look for 0x0D (PREFETCH or PREFETCHW) or 0x18 (PREFEETCHlevel)
    //

    if (rip[0] == 0x0D) {

        //
        // PREFETCH and PREFETCHW are differentiated by a
        // ModRM.reg of 0 and 1, respectively.
        //

        if ((rip[1] & 0x1C) == 4) {

            //
            // PREFETCHW.  Ensure that this was a write fault.
            //

            if ((FaultStatus & EXCEPTION_WRITE_FAULT) == 0) {
                return STATUS_ACCESS_VIOLATION;
            }

        } else {

            //
            // PREFETCH.  Ensure that this was a read fault.
            //

            if ((FaultStatus & EXCEPTION_WRITE_FAULT) != 0) {
                return STATUS_ACCESS_VIOLATION;
            }
        }

    } else if (rip[0] == 0x18) {

        //
        // PREFETCHx.  Ensure that this was a read fault.
        //

        if ((FaultStatus & EXCEPTION_WRITE_FAULT) != 0) {
            return STATUS_ACCESS_VIOLATION;
        }

    } else {

        //
        // Some other instruction.
        // 

        return STATUS_ACCESS_VIOLATION;
    }

    return STATUS_SUCCESS;
}


LOGICAL
KiPreprocessInvalidOpcodeFault (
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This routine is invoked from KiInvalidOpcodeFault.  It will examine the
    faulting opcode.  If it is LAHF or SAHF, the instruction will be
    emulated and execution resumed.

Arguments:

    TrapFrame - Supplies a pointer to the KTRAP_FRAME generated by
                the fault handler.

Return Value:

    Returns non-zero if the instruction was emulated and should be resumed,
    zero otherwise.

--*/

{
    PUCHAR AH;
    PUCHAR FL;
    LOGICAL fixedUp;
    KIRQL oldIrql;
    UCHAR opcode;
    KPROCESSOR_MODE previousMode;
    PUCHAR rip;

    fixedUp = FALSE;

    //
    // Ensure that the trap frame isn't modified while we're here
    // 

    oldIrql = KeGetCurrentIrql();
    if (oldIrql < APC_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
    }

    //
    // Determine previous mode and capture the faulting opcode, probing
    // as appropriate.
    //

    if ((TrapFrame->SegCs & MODE_MASK) != 0) {
        previousMode = UserMode;
    } else {
        previousMode = KernelMode;
    }

    rip = (PUCHAR)(TrapFrame->Rip);
    if (previousMode == UserMode) {

        opcode = 0;
        try {
            opcode = ProbeAndReadUchar(rip);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

    } else {
        opcode = *rip;
    }

    //
    // If the opcode is neither SAHF nor LAHF then return FALSE.  If the
    // opcode capture failed, then opcode == 0 at this point.
    //
    // If this is in fact a more complex encoding of LAHF/SAHF (prefix,
    // etc.) then KiOp_LSAHF() will pick it up.
    // 
    // Otherwise, emulate the effect of the instruction, skip past, and
    // resume execution.
    // 
    
    AH = ((PUCHAR)&(TrapFrame->Rax)) + 1;
    FL = (PUCHAR)&(TrapFrame->EFlags);

    if (opcode == 0x9E) {

        //
        // Emulate SAHF
        //

        *FL = *AH;
        fixedUp = TRUE;

    } else if (opcode == 0x9F) {

        //
        // Emulate LAHF
        //

        *AH = *FL;
        fixedUp = TRUE;
    }

    if (fixedUp != FALSE) {
    
        TrapFrame->Rip = SANITIZE_VA(TrapFrame->Rip + 1,
                                     TrapFrame->SegCs,
                                     previousMode);
    }

    if (oldIrql < APC_LEVEL) {
        KeLowerIrql(oldIrql);
    }

    return fixedUp;
}


BOOLEAN
KiOpPreprocessAccessViolation (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PCONTEXT ContextRecord
    )

/*++

Routine Description:

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

Return Value:

    TRUE - the caller's context should be updated and control returned
           to the caller.

    FALSE - do not return control to the caller at this time, continue
            exception handling.

--*/

{
    USHORT codeSegment;
    PKTHREAD currentThread;
    PVOID faultVa;
    ULONG faultCount;
    ULONG64 slistFault;
    ULONG64 slistResume;

    codeSegment = ContextRecord->SegCs;
    switch (codeSegment) {
    
        case KGDT64_R3_CODE | RPL_MASK:
            slistFault = (ULONG64)KeUserPopEntrySListFault;
            slistResume = (ULONG64)KeUserPopEntrySListResume;
            break;

        case KGDT64_R3_CMCODE | RPL_MASK:
            slistFault = (ULONG64)KeUserPopEntrySListFaultWow64;
            slistResume = (ULONG64)KeUserPopEntrySListResumeWow64;
            break;

        case KGDT64_R0_CODE:
            slistFault = (ULONG64)&ExpInterlockedPopEntrySListFault;
            slistResume = (ULONG64)&ExpInterlockedPopEntrySListResume;
            break;

        default:
            return FALSE;
    }

    if (ContextRecord->Rip != slistFault) {
        return FALSE;
    }

    if (codeSegment == KGDT64_R0_CODE) {
        ContextRecord->Rip = slistResume;
        return TRUE;
    }

    currentThread = KeGetCurrentThread();
    faultVa = (PVOID)ExceptionRecord->ExceptionInformation[1];

    if (faultVa == currentThread->SListFaultAddress) {

        faultCount = currentThread->SListFaultCount;
        if (faultCount > KI_SLIST_FAULT_COUNT_MAXIMUM) {
            currentThread->SListFaultCount = 0;
            return FALSE;
        }

        currentThread->SListFaultCount = faultCount + 1;

    } else {

        currentThread->SListFaultCount = 0;
        currentThread->SListFaultAddress = faultVa;
    }

    ContextRecord->Rip = slistResume;
    return TRUE;

}


BOOLEAN
KeInvalidAccessAllowed (
    __in_opt PVOID TrapInformation
    )

/*++

Routine Description:

    Mm will pass a pointer to a trap frame prior to issuing a bugcheck on
    a pagefault.  This routine lets Mm know if it is ok to bugcheck.  The
    specific case we must protect are the interlocked pop sequences which can
    blindly access memory that may have been freed and/or reused prior to the
    access.  We don't want to bugcheck the system in these cases, so we check
    the instruction pointer here.

    For a usermode fault, Mm uses this routine for similar reasons, to determine
    whether a guard page fault should be ignored.

Arguments:

    TrapInformation - Supplies a trap frame pointer.  NULL means return False.

Return Value:

    True if the invalid access should be ignored, False otherwise.

--*/

{
    BOOLEAN result;
    ULONG64 slistFaultIP;
    PKTRAP_FRAME trapFrame;

    if (ARGUMENT_PRESENT(TrapInformation) == FALSE) {
        return FALSE;
    }

    trapFrame = TrapInformation;
    result = FALSE;

    switch (trapFrame->SegCs) {

        case KGDT64_R0_CODE:

            //
            // Fault occured in kernel mode
            //

            slistFaultIP = (ULONG64)&ExpInterlockedPopEntrySListFault;
            break;

        case KGDT64_R3_CMCODE | RPL_MASK:

            //
            // Fault occured in usermode, wow64
            //

            slistFaultIP = (ULONG64)KeUserPopEntrySListFaultWow64;
            break;

        case KGDT64_R3_CODE | RPL_MASK:

            //
            // Fault occured in native usermode
            //

            slistFaultIP = (ULONG64)KeUserPopEntrySListFault;
            break;

        default:
            return FALSE;
    }

    if (trapFrame->Rip == slistFaultIP) {
        result = TRUE;
    }

    return result;
}

