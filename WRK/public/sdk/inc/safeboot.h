/*++

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    safeboot.h

Abstract:

    This module contains all safe boot code.

--*/

#ifndef _SAFEBOOT_
#define _SAFEBOOT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// define the safeboot options
//

#define SAFEBOOT_MINIMAL                1
#define SAFEBOOT_NETWORK                2
#define SAFEBOOT_DSREPAIR               3

#define SAFEBOOT_LOAD_OPTION_W          L"SAFEBOOT:"
#define SAFEBOOT_MINIMAL_STR_W          L"MINIMAL"
#define SAFEBOOT_NETWORK_STR_W          L"NETWORK"
#define SAFEBOOT_DSREPAIR_STR_W         L"DSREPAIR"
#define SAFEBOOT_ALTERNATESHELL_STR_W   L"(ALTERNATESHELL)"

#define SAFEBOOT_LOAD_OPTION_A          "SAFEBOOT:"
#define SAFEBOOT_MINIMAL_STR_A          "MINIMAL"
#define SAFEBOOT_NETWORK_STR_A          "NETWORK"
#define SAFEBOOT_DSREPAIR_STR_A         "DSREPAIR"
#define SAFEBOOT_ALTERNATESHELL_STR_A   "(ALTERNATESHELL)"

#define BOOTLOG_STRSIZE     256

#ifdef __cplusplus
}
#endif

#endif // _NTPNPAPI_

