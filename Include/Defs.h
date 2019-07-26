/*
*
* Copyright (c) 2015-2015 - 2019 by blindtiger ( blindtiger@foxmail.com )
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _DEFS_H_
#define _DEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define SERIAL "9ARX-3J9H-HCQK"

#ifndef PUBLIC
// #define PUBLIC
#endif // !PUBLIC

#ifndef NTOS_KERNEL_RUNTIME
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <windowsx.h>

#define PAGE_SIZE 0x1000
#else
#include <ntos.h>
#include <zwapi.h>
#endif // !NTOS_KERNEL_RUNTIME

#ifdef _WIN64
#include <wow64t.h>
#endif // _WIN64

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <arccodes.h>
#include <ntddkbd.h>
#include <ntddmou.h>

#define SystemRootDirectory L"\\SystemRoot\\System32\\"
#define Wow64SystemRootDirectory L"\\SystemRoot\\SysWOW64\\"

#define ServicesDirectory L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DEFS_H_
