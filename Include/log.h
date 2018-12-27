/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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

#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef TRACE
#ifndef PUBLIC
#ifndef NTOS_KERNEL_RUNTIME
#define TRACE(exp) \
            (((NTSTATUS)exp) >= 0) ? \
                TRUE : \
                (vDbgPrint( \
                    TEXT("[Shark] %hs[%d] %hs failed < %08x >\n"), \
                    __FILE__, \
                    __LINE__, \
                    __FUNCDNAME__, \
                    exp), FALSE)

    VOID
        CDECL
        vDbgPrint(
            __in PCTSTR Format,
            ...
        );
#else
#define TRACE(exp) \
            (((NTSTATUS)exp) >= 0) ? \
                TRUE : \
                (DbgPrint( \
                    "[Shark] %hs[%d] %hs failed < %08x >\n", \
                    __FILE__, \
                    __LINE__, \
                    __FUNCDNAME__, \
                    exp), FALSE)
#endif // !NTOS_KERNEL_RUNTIME
#else
#define TRACE(exp) (((NTSTATUS)exp) >= 0)
#endif // !PUBLIC
#endif // !TRACE

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_LOG_H_
