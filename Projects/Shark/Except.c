/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
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

#include <defs.h>

#include "Except.h"

VOID
NTAPI
CaptureImageExceptionValues(
    __in PVOID Base,
    __out PVOID * FunctionTable,
    __out PULONG TableSize
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig = NULL;
    ULONG LoadConfigSize = 0;
    PIMAGE_COR20_HEADER Cor20Header = NULL;
    ULONG Cor20HeaderSize = 0;

    NtHeaders = RtlImageNtHeader(Base);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            if (IMAGE_DLLCHARACTERISTICS_NO_SEH ==
                (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.DllCharacteristics &
                    IMAGE_DLLCHARACTERISTICS_NO_SEH)) {
                *FunctionTable = LongToPtr(-1);
                *TableSize = -1;
            }
            else {
                LoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY32)
                    RtlImageDirectoryEntryToData(
                        Base,
                        TRUE,
                        IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                        &LoadConfigSize);

                if (NULL != LoadConfig &&
                    LoadConfig->Size >= RTL_SIZEOF_THROUGH_FIELD(
                        IMAGE_LOAD_CONFIG_DIRECTORY32,
                        SEHandlerCount) &&
                    0 != LoadConfig->SEHandlerTable &&
                    0 != LoadConfig->SEHandlerCount) {
                    *FunctionTable = ULongToPtr(LoadConfig->SEHandlerTable);
                    *TableSize = LoadConfig->SEHandlerCount;
                }
                else {
                    Cor20Header = RtlImageDirectoryEntryToData(
                        Base,
                        TRUE,
                        IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                        &Cor20HeaderSize);

                    if (Cor20Header && ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) ==
                        COMIMAGE_FLAGS_ILONLY)) {
                        *FunctionTable = LongToPtr(-1);
                        *TableSize = -1;
                    }
                    else {
                        *FunctionTable = NULL;
                        *TableSize = 0;
                    }
                }
            }
        }
        else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            *FunctionTable = RtlImageDirectoryEntryToData(
                Base,
                TRUE,
                IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                TableSize);
        }
    }
}
