//
// Copyright (c) Microsoft Corporation. All rights reserved. 
//
// You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
// If you do not agree to the terms, do not use the code.
//

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <stdio.h>
#include <tchar.h>

#define RtlIpv4AddressToStringT RtlIpv4AddressToStringW
#define RtlIpv6AddressToStringT RtlIpv6AddressToStringW
#define RtlIpv4AddressToStringExT RtlIpv4AddressToStringExW
#define RtlIpv6AddressToStringExT RtlIpv6AddressToStringExW

#include "add2strt.h"

