/*
*
* Copyright (c) 2015-2018 by blindtiger ( blindtiger@foxmail.com )
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

#ifndef _DEVICEDEFS_H_
#define _DEVICEDEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#include <devioctl.h>

#define LOADER_IMAGE_STRING L"Shark.sys"
#define LOADER_SERVICE_STRING L"Shark"
#define LOADER_DEVICE_STRING L"\\Device\\Shark"
#define LOADER_SYMBOLIC_STRING L"\\DosDevices\\Shark"

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DEVICEDEFS_H_
