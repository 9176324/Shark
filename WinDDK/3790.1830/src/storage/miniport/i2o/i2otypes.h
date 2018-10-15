/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2otypes.h

Abstract:

    This file contains the data-type information used by the I2O miniport.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/

#ifndef __INCi2otypesh
#define __INCi2otypesh

#define I2OTYPES_REV 1_5_5


//
// Pragma macros. These are to assure appropriate alignment between
// host/IOP as defined by the I2O Specification. Each one of the shared
// header files includes these macros.
//

#define PRAGMA_ALIGN_PUSH   
#define PRAGMA_ALIGN_POP    


//
// Setup the basics
// 
typedef    char            S8;
typedef    short           S16;
typedef    long            S32;

typedef    unsigned char   U8;
typedef    unsigned short  U16;
typedef    unsigned long   U32;


//
// Bitfields
// 
typedef    U32      BF;

//
// 64 bit defines 
//
typedef struct _S64 {
   U32 LowPart;
   S32 HighPart;
} S64;

typedef struct _U64 {
   U32 LowPart;
   U32 HighPart;
} U64;

//
// Pointer to Basics 
// 
typedef    VOID  *PVOID;
typedef    S8    *PS8;
typedef    S16   *PS16;
typedef    S32   *PS32;
typedef    S64   *PS64;

//
// Pointer to Unsigned Basics
// 
typedef    U8                  *PU8;
typedef    U16                 *PU16;
typedef    U32                 *PU32;
typedef    U64                 *PU64;

//
// misc
//
typedef S32             I2O_ARG;
typedef U32             I2O_COUNT;
typedef U32             I2O_USECS;
typedef U32             I2O_ADDR32;
typedef U32             I2O_SIZE;

#endif /* __INCi2otypesh */

