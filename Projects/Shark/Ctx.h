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

#ifndef _CTX_H_
#define _CTX_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _CTX {
        USHORT Platform;
        PUSER_THREAD_START_ROUTINE StartRoutine;
        PVOID StartContext;
        NTSTATUS ReturnedStatus;
        KPROCESSOR_MODE Mode;
    } CTX, *PCTX;

    typedef struct _ATX {
        KAPC Apc;
        KEVENT Notify;
        CTX Ctx; 
    } ATX, *PATX;

    NTSTATUS
        NTAPI
        RemoteCall(
            __in HANDLE UniqueThread,
            __in USHORT Platform,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext,
            __in KPROCESSOR_MODE Mode
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_CTX_H_
