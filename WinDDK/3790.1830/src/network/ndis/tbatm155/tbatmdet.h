/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       tbatmdet.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               (510) 770-4920
               http://www.elisaresearch.com


Abstract:
   
   This file contains related definitions of CSR access.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/16/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __TBATMDET_H
#define __TBATMDET_H


#define TBATM155_WRITE_PORT(_VitrualAddr, _Value) {                    \
                                                                       \
   NdisWriteRegisterUlong((PULONG)(_VitrualAddr), (ULONG)(_Value));    \
}

#define TBATM155_READ_PORT(_VitrualAddr, _Value) {                     \
                                                                       \
   NdisReadRegisterUlong((PULONG)(_VitrualAddr), (PULONG)(_Value));    \
}


#endif // __TBATMDET_H

