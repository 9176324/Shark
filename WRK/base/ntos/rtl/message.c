/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    message.c

Abstract:

    Message table resource accessing functions

--*/

#include "ntrtlp.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,RtlFindMessage)
#endif

NTSTATUS
RtlFindMessage(
    IN PVOID DllHandle,
    IN ULONG MessageTableId,
    IN ULONG MessageLanguageId,
    IN ULONG MessageId,
    OUT PMESSAGE_RESOURCE_ENTRY *MessageEntry
    )
{
    NTSTATUS Status;
    ULONG NumberOfBlocks;
    ULONG EntryIndex;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    PMESSAGE_RESOURCE_DATA  MessageData;
    PMESSAGE_RESOURCE_BLOCK MessageBlock;
    PCHAR s;
    ULONG_PTR ResourceIdPath[ 3 ];

    RTL_PAGED_CODE();

    ResourceIdPath[ 0 ] = MessageTableId;
    ResourceIdPath[ 1 ] = 1;
    ResourceIdPath[ 2 ] = MessageLanguageId;

    Status = LdrpSearchResourceSection_U( DllHandle,
                                          ResourceIdPath,
                                          3,
                                          0,
                                          (PVOID *)&ResourceDataEntry
                                        );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    Status = LdrpAccessResourceData( DllHandle,
                                     ResourceDataEntry,
                                     (PVOID *)&MessageData,
                                     NULL
                                   );
    if (!NT_SUCCESS( Status )) {
        return( Status );
        }

    NumberOfBlocks = MessageData->NumberOfBlocks;
    MessageBlock = &MessageData->Blocks[ 0 ];
    while (NumberOfBlocks--) {
        if (MessageId >= MessageBlock->LowId &&
            MessageId <= MessageBlock->HighId
           ) {
            s = (PCHAR)MessageData + MessageBlock->OffsetToEntries;
            EntryIndex = MessageId - MessageBlock->LowId;
            while (EntryIndex--) {
                s += ((PMESSAGE_RESOURCE_ENTRY)s)->Length;
                }

            *MessageEntry = (PMESSAGE_RESOURCE_ENTRY)s;
            return( STATUS_SUCCESS );
            }

        MessageBlock++;
        }

    return( STATUS_MESSAGE_NOT_FOUND );
}

