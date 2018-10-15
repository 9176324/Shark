/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    transpos.h


Abstract:

    This module contains defines and prototypes for the transpos.c module.


Author:

    22-Dec-1993 Wed 15:49:08 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/



#ifndef _TRANSPOS_
#define _TRANSPOS_


typedef struct _TPINFO {
    PPDEV   pPDev;
    LPBYTE  pSrc;
    LPBYTE  pDest;
    LONG    cbSrcScan;
    LONG    cbDestScan;
    DWORD   cySrc;
    DWORD   DestXStart;
    } TPINFO, *PTPINFO;


#define _RL_TPI(pTPInfo, cScan)                                         \
{                                                                       \
    pTPInfo->pDest      += (pTPInfo->cbDestScan * cScan)                \
    pTPInfo->cbDestScan  = -pTBInfo->cbDestScan;                        \
    pTPInfo->pSrc       += (pTPInfo->cbSrcScan * (pTPInfo->cySrc - 1)); \
    pTPInfo->cbSrcScan   = -pTBInfo->cbSrcScan;                         \
}


//
// Function prototypes exported by transpos.c
//

BOOL
TransPos4BPP(
    PTPINFO pTPInfo
    );

BOOL
TransPos1BPP(
    PTPINFO pTPInfo
    );



#endif  // _TRANSPOS_

