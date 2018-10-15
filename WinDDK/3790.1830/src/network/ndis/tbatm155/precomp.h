/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved. 

   File:       precomp.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

	This file contains global definitions.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

--*/
#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4127)   // conditional expression is constant
#pragma warning(disable:4054)   // cast of function pointer to PVOID


#include <ndis.h>
#include <atm.h>

#include "version.h"
#include "support.h"
#include "hw.h"
#include "sar.h"
#include "peephole.h"
#include "sw.h"
#include "protos.h"
#include "debug.h"
#include "tbatmdet.h"
#include "tbatm155.h"
#include "tbmeteor.h"


