/*
*
* Copyright (c) 2015 - 2021 by blindtiger ( blindtiger@foxmail.com )
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

#ifndef _VER_H_
#define _VER_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define VER_DEBUG                   2
#define VER_PRERELEASE              0
#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_FILETYPE                VFT_DRV
#define VER_FILESUBTYPE             VFT2_DRV_SYSTEM

#define VER_COMPANYNAME_STR         "blindtiger"
#define VER_PRODUCTNAME_STR         "Shark"
#define VER_LEGALCOPYRIGHT_YEARS    "2019"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (c) " VER_LEGALCOPYRIGHT_YEARS " " VER_COMPANYNAME_STR
#define VER_LEGALTRADEMARKS_STR     "Copyright (c) " VER_LEGALCOPYRIGHT_YEARS " " VER_COMPANYNAME_STR

#define VER_PRODUCTVERSION          1.0.0.0
#define VER_PRODUCTVERSION_STR      "1.0.0.0"
#define VER_PRODUCTVERSION_W        (0x0200)
#define VER_PRODUCTVERSION_DW       (0x0200)

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_VER_H_
