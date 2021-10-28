/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original Code is blindtiger.
*
*/

#ifndef _SUPPORT_H_
#define _SUPPORT_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    status
        NTAPI
        RegistryOpenKey(
            __out ptr * KeyHandle,
            __in ACCESS_MASK DesiredAccess,
            __in wcptr KeyList
        );

    status
        NTAPI
        RegistryCreateSevice(
            __in wcptr ImageFileName,
            __in wcptr ServiceName
        );

    status
        NTAPI
        RegistryDeleteSevice(
            __in wcptr ServiceName
        );

    ptr
        NTAPI
        LdrGetSymbolOffline(
            __in ptr ImageBase,
            __in ptr ImageViewBase,
            __in_opt cptr ProcedureName,
            __in_opt u32 ProcedureNumber
        );

    status
        NTAPI
        SupInstall(
            void
        );

    void
        NTAPI
        SupUninstall(
            void
        );

    status
        NTAPI
        SupInit(
            void
        );

    void
        NTAPI
        SupTerm(
            void
        );

    status
        NTAPI
        SupLdrUnload(
            __in ptr ImageBase
        );

    status
        NTAPI
        SupSetVMForFastIoCtl(
            __in ptr Handler
        );

    status
        NTAPI
        SupLdrLoad(
            __in wcptr ImageFileName,
            __in cptr BaseName,
            __in u32 Operation
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SUPPORT_H_
