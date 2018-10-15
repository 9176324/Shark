//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 1999
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// @doc      INTERNAL EXTERNAL
// @module   7134regs.h | This class defines the SAA713X register structure. It is
// useful only for debugging purposes. Don't use this to access the register, use
// the IO classes instead!
// @end
// Filename: 7134regs.h
// 
// Routines/Functions:
//
//  public:
//
//  private:
//
//  protected:
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __7134REGS_H
#define __7134REGS_H


typedef signed		long  S32;
typedef unsigned	long  U32;
typedef unsigned	short U16;
typedef unsigned	char  U8;


#include "SecondaryStructure.h"


typedef struct 
{

#include "PrimaryStructure.h"

} NONVOLATILE_VAMPREGS;

typedef volatile NONVOLATILE_VAMPREGS VAMPREGS;


#endif
