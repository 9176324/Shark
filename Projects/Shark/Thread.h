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

#ifndef _THREAD_H_
#define _THREAD_H_

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    // Attributes

    // begin_rev
#define PS_ATTRIBUTE_NUMBER_MASK 0x0000ffff
#define PS_ATTRIBUTE_THREAD 0x00010000 // can be used with threads
#define PS_ATTRIBUTE_INPUT 0x00020000 // input only
#define PS_ATTRIBUTE_UNKNOWN 0x00040000
    // end_rev

    // private
    typedef enum _PS_ATTRIBUTE_NUM {
        PsAttributeParentProcess, // in HANDLE
        PsAttributeDebugPort, // in HANDLE
        PsAttributeToken, // in HANDLE
        PsAttributeClientId, // out PCLIENT_ID
        PsAttributeTebAddress, // out PTEB *
        PsAttributeImageName, // in PWSTR
        PsAttributeImageInfo, // out PSECTION_IMAGE_INFORMATION
        PsAttributeMemoryReserve, // in PPS_MEMORY_RESERVE
        PsAttributePriorityClass, // in UCHAR
        PsAttributeErrorMode, // in ULONG
        PsAttributeStdHandleInfo, // 10, in PPS_STD_HANDLE_INFO
        PsAttributeHandleList, // in PHANDLE
        PsAttributeGroupAffinity, // in PGROUP_AFFINITY
        PsAttributePreferredNode, // in PUSHORT
        PsAttributeIdealProcessor, // in PPROCESSOR_NUMBER
        PsAttributeUmsThread, // ? in PUMS_CREATE_THREAD_ATTRIBUTES
        PsAttributeMitigationOptions, // in UCHAR
        PsAttributeProtectionLevel,
        PsAttributeSecureProcess, // since THRESHOLD
        PsAttributeJobList,
        PsAttributeMax
    } PS_ATTRIBUTE_NUM;

    // begin_rev

#define PsAttributeValue(Number, Thread, Input, Unknown) \
    (((Number) & PS_ATTRIBUTE_NUMBER_MASK) | \
    ((Thread) ? PS_ATTRIBUTE_THREAD : 0) | \
    ((Input) ? PS_ATTRIBUTE_INPUT : 0) | \
    ((Unknown) ? PS_ATTRIBUTE_UNKNOWN : 0))

#define PS_ATTRIBUTE_PARENT_PROCESS \
    PsAttributeValue(PsAttributeParentProcess, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_DEBUG_PORT \
    PsAttributeValue(PsAttributeDebugPort, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_TOKEN \
    PsAttributeValue(PsAttributeToken, FALSE, TRUE, TRUE)
#define PS_ATTRIBUTE_CLIENT_ID \
    PsAttributeValue(PsAttributeClientId, TRUE, FALSE, FALSE)
#define PS_ATTRIBUTE_TEB_ADDRESS \
    PsAttributeValue(PsAttributeTebAddress, TRUE, FALSE, FALSE)
#define PS_ATTRIBUTE_IMAGE_NAME \
    PsAttributeValue(PsAttributeImageName, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_IMAGE_INFO \
    PsAttributeValue(PsAttributeImageInfo, FALSE, FALSE, FALSE)
#define PS_ATTRIBUTE_MEMORY_RESERVE \
    PsAttributeValue(PsAttributeMemoryReserve, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_PRIORITY_CLASS \
    PsAttributeValue(PsAttributePriorityClass, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_ERROR_MODE \
    PsAttributeValue(PsAttributeErrorMode, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_STD_HANDLE_INFO \
    PsAttributeValue(PsAttributeStdHandleInfo, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_HANDLE_LIST \
    PsAttributeValue(PsAttributeHandleList, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_GROUP_AFFINITY \
    PsAttributeValue(PsAttributeGroupAffinity, TRUE, TRUE, FALSE)
#define PS_ATTRIBUTE_PREFERRED_NODE \
    PsAttributeValue(PsAttributePreferredNode, FALSE, TRUE, FALSE)
#define PS_ATTRIBUTE_IDEAL_PROCESSOR \
    PsAttributeValue(PsAttributeIdealProcessor, TRUE, TRUE, FALSE)
#define PS_ATTRIBUTE_MITIGATION_OPTIONS \
    PsAttributeValue(PsAttributeMitigationOptions, FALSE, TRUE, TRUE)

    // end_rev

    // begin_private

    typedef struct _PS_ATTRIBUTE {
        ULONG Attribute;
        SIZE_T Size;

        union {
            ULONG Value;
            PVOID ValuePtr;
        };

        PSIZE_T ReturnLength;
    } PS_ATTRIBUTE, *PPS_ATTRIBUTE;

    typedef struct _PS_ATTRIBUTE_LIST {
        SIZE_T TotalLength;
        PS_ATTRIBUTE Attributes[1];
    } PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

    // begin_rev
#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED 0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH 0x00000002 // ?
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004
#define THREAD_CREATE_FLAGS_HAS_SECURITY_DESCRIPTOR 0x00000010 // ?
#define THREAD_CREATE_FLAGS_ACCESS_CHECK_IN_TARGET 0x00000020 // ?
#define THREAD_CREATE_FLAGS_INITIAL_THREAD 0x00000080
    // end_rev

    VOID
        NTAPI
        InitializeSystemThread(
            __inout PRELOADER_PARAMETER_BLOCK Block
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_THREAD_H_
