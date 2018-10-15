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

#ifndef _LOCK_H_
#define _LOCK_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _MDL_LOCKER {
        PMDL MemoryDescriptorList;
        PVOID MappedAddress;
    }MDL_LOCKER, *PMDL_LOCKER;

    PMDL_LOCKER
        NTAPI
        ProbeAndLockPages(
            __in PVOID VirtualAddress,
            __in ULONG Length,
            __in KPROCESSOR_MODE AccessMode,
            __in LOCK_OPERATION Operation
        );

    VOID
        NTAPI
        UnlockPages(
            __inout PMDL_LOCKER MdlLocker
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_LOCK_H_
