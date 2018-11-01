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

#define MANAGER_IMAGE_LINK L"Shark.sys"
#define MANAGER_SERVICE_LINK L"Shark"

#define MANAGER_DEVICE_LINK L"\\Device\\Shark"
#define MANAGER_SYMBOLIC_LINK L"\\DosDevices\\Shark"

    typedef enum _COMMAND_NUMBER {
        DISABLE_PATCHGUARD
    } API_NUMBER;

#define API_IO_TYPE ('  kS')

#define API_METHOD_DISABLE_PATCHGUARD CTL_CODE(API_IO_TYPE, DISABLE_PATCHGUARD, FILE_ANY_ACCESS, METHOD_BUFFERED)

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DEVICEDEFS_H_
