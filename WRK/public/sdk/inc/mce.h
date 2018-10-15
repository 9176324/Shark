/*++ BUILD Version: 0011    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    mce.h

Abstract:

    This header file defines the Machine Check Errors definitions.

--*/

#ifndef _MCE_
#define _MCE_

//
// HalMcaLogInformation
//

#if defined(_X86_) || defined(_AMD64_)

//
// ADDR register for each MCA bank
//

typedef union _MCI_ADDR{
    struct {
        ULONG Address;
        ULONG Reserved;
    };

    ULONGLONG   QuadPart;
} MCI_ADDR, *PMCI_ADDR;


typedef enum {
    HAL_MCE_RECORD,
    HAL_MCA_RECORD
} MCA_EXCEPTION_TYPE;


#if defined(_AMD64_)

//
// STATUS register for each MCA bank.
//

typedef union _MCI_STATS {
    struct {
        USHORT  McaErrorCode;
        USHORT  ModelErrorCode;
        ULONG   OtherInformation : 25;
        ULONG   ContextCorrupt : 1;
        ULONG   AddressValid : 1;
        ULONG   MiscValid : 1;
        ULONG   ErrorEnabled : 1;
        ULONG   UncorrectedError : 1;
        ULONG   StatusOverFlow : 1;
        ULONG   Valid : 1;
    } MciStatus;

    ULONG64 QuadPart;
} MCI_STATS, *PMCI_STATS;

#endif // _AMD64_

#if defined(_X86_)

//
// STATUS register for each MCA bank.
//

typedef union _MCI_STATS {
    struct {
        USHORT  McaCod;
        USHORT  MsCod;
        ULONG   OtherInfo : 25;
        ULONG   Damage : 1;
        ULONG   AddressValid : 1;
        ULONG   MiscValid : 1;
        ULONG   Enabled : 1;
        ULONG   UnCorrected : 1;
        ULONG   OverFlow : 1;
        ULONG   Valid : 1;
    } MciStats;

    ULONGLONG QuadPart;

} MCI_STATS, *PMCI_STATS;

#endif // _X86_

//
// MCA exception log entry
// Defined as a union to contain MCA specific log or Pentium style MCE info.
//

#define MCA_EXTREG_V2MAX       24  // X86: Max. Number of extended registers

#if defined(_X86_) || defined(_AMD64_)

typedef struct _MCA_EXCEPTION {

    // Begin Version 1 stuff
    ULONG               VersionNumber;      // Version number of this record type
    MCA_EXCEPTION_TYPE  ExceptionType;      // MCA or MCE
    LARGE_INTEGER       TimeStamp;          // exception recording timestamp
    ULONG               ProcessorNumber;
    ULONG               Reserved1;

    union {
        struct {
            UCHAR           BankNumber;
            UCHAR           Reserved2[7];
            MCI_STATS       Status;
            MCI_ADDR        Address;
            ULONGLONG       Misc;
        } Mca;

        struct {
            ULONGLONG       Address;        // physical addr of cycle causing the error
            ULONGLONG       Type;           // cycle specification causing the error
        } Mce;
    } u;
    // End   Version 1 stuff

    // Begin Version 2 stuff
    ULONG                   ExtCnt;
    ULONG                   Reserved3;
    ULONGLONG               ExtReg[MCA_EXTREG_V2MAX];
    // End   Version 2 stuff

} MCA_EXCEPTION, *PMCA_EXCEPTION;

typedef MCA_EXCEPTION CMC_EXCEPTION, *PCMC_EXCEPTION;    // Corrected Machine Check
typedef MCA_EXCEPTION CPE_EXCEPTION, *PCPE_EXCEPTION;    // Corrected Platform Error

#define MCA_EXCEPTION_V1_SIZE FIELD_OFFSET(MCA_EXCEPTION, ExtCnt)
#define MCA_EXCEPTION_V2_SIZE sizeof(struct _MCA_EXCEPTION)

#endif // _X86_ || _AMD64_

//
// ERRORS: ERROR_SEVERITY definitions
//
// One day the MS compiler will support typed enums with type != int so this
// type of enums (UCHAR, __int64) could be defined...
//

#if defined(_AMD64_)

typedef UCHAR ERROR_SEVERITY, *PERROR_SEVERITY;

typedef enum _ERROR_SEVERITY_VALUE  {
    ErrorRecoverable = 0,
    ErrorFatal       = 1,
    ErrorCorrected   = 2,
    ErrorOthers      = 3,   // [3,...] values are reserved
} ERROR_SEVERITY_VALUE;

#endif

#endif // defined(_X86_) || defined(_AMD64_)

#endif // _MCE_

