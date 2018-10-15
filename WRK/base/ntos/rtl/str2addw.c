//
// Copyright (c) Microsoft Corporation. All rights reserved. 
//
// You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
// If you do not agree to the terms, do not use the code.
//

// Without this define, link errors can occur due to missing _pctype and
// __mb_cur_max
//
#define _CTYPE_DISABLE_MACROS

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <stdlib.h>
#include <tchar.h>

#define RtlIpv4StringToAddressT RtlIpv4StringToAddressW
#define RtlIpv6StringToAddressT RtlIpv6StringToAddressW
#define RtlIpv4StringToAddressExT RtlIpv4StringToAddressExW
#define RtlIpv6StringToAddressExT RtlIpv6StringToAddressExW

#include "str2addt.h"

