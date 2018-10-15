//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2000
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

#pragma once

extern "C" 
{
#include <wdm.h>
}

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unknown.h>
//
#include <ks.h>
#include <ksmedia.h>
//#include <kcom.h>
//
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdasup.h>

//
#if (DBG)
// Defines the modul name which will be shown in any DBG message.
#define STR_MODULENAME "34BDACap: "
#endif
#include <ksdebug.h>
//
