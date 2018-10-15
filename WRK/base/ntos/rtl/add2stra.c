//
// Copyright (c) Microsoft Corporation. All rights reserved. 
//
// You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
// If you do not agree to the terms, do not use the code.
//

#undef UNICODE
#undef _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>
#include <tchar.h>

#define RtlIpv6AddressToStringT RtlIpv6AddressToStringA
#define RtlIpv4AddressToStringT RtlIpv4AddressToStringA
#define RtlIpv6AddressToStringExT RtlIpv6AddressToStringExA
#define RtlIpv4AddressToStringExT RtlIpv4AddressToStringExA

#include "add2strt.h"

