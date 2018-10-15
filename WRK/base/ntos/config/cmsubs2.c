/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cmsubs2.c

Abstract:

    This module various support routines for the configuration manager.

    The routines in this module are independent enough to be linked into
    any other program.  The routines in cmsubs.c are not.

--*/

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpQueryKeyData)
#pragma alloc_text(PAGE,CmpQueryKeyDataFromCache)
#pragma alloc_text(PAGE,CmpQueryKeyValueData)
#endif

//
// Define alignment macro.
//

#define ALIGN_OFFSET(Offset) (ULONG) \
        ((((ULONG)(Offset) + sizeof(ULONG)-1)) & (~(sizeof(ULONG) - 1)))

#define ALIGN_OFFSET64(Offset) (ULONG) \
        ((((ULONG)(Offset) + sizeof(ULONGLONG)-1)) & (~(sizeof(ULONGLONG) - 1)))

//
// Data transfer workers
//


NTSTATUS
CmpQueryKeyData(
    PHHIVE                  Hive,
    PCM_KEY_NODE            Node,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
    )
/*++

Routine Description:

    Do the actual copy of data for a key into caller's buffer.

    If KeyInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Node - Supplies pointer to node whose subkeys are to be found

    KeyInformationClass - Specifies the type of information returned in
        Buffer.  One of the following types:

        KeyBasicInformation - return last write time, title index, and name.
            (see KEY_BASIC_INFORMATION structure)

        KeyNodeInformation - return last write time, title index, name, class.
            (see KEY_NODE_INFORMATION structure)

    KeyInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyInformation in bytes.

    ResultLength - Number of bytes actually written into KeyInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS            status;
    PCELL_DATA          pclass;
    ULONG               requiredlength;
    LONG                leftlength;
    ULONG               offset;
    ULONG               minimumlength;
    PKEY_INFORMATION    pbuffer;
    USHORT              NameLength;

    pbuffer = (PKEY_INFORMATION)KeyInformation;
    NameLength = CmpHKeyNameLen(Node);

    switch (KeyInformationClass) {

    case KeyBasicInformation:

        //
        // LastWriteTime, TitleIndex, NameLength, Name
        //

        requiredlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name) +
                         NameLength;

        minimumlength = FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyBasicInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyBasicInformation.TitleIndex = 0;

            pbuffer->KeyBasicInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;

            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (Node->Flags & KEY_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyBasicInformation.Name,
                                      leftlength,
                                      Node->Name,
                                      Node->NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyBasicInformation.Name[0]),
                    &(Node->Name[0]),
                    requiredlength
                    );
            }
        }

        break;


    case KeyNodeInformation:
        //
        // LastWriteTime, TitleIndex, ClassOffset, ClassLength
        // NameLength, Name, Class
        //
        requiredlength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name) +
                         NameLength +
                         Node->ClassLength;

        minimumlength = FIELD_OFFSET(KEY_NODE_INFORMATION, Name);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyNodeInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyNodeInformation.TitleIndex = 0;

            pbuffer->KeyNodeInformation.ClassLength =
                Node->ClassLength;

            pbuffer->KeyNodeInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                status = STATUS_BUFFER_OVERFLOW;
            }

            if (Node->Flags & KEY_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyNodeInformation.Name,
                                      leftlength,
                                      Node->Name,
                                      Node->NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyNodeInformation.Name[0]),
                    &(Node->Name[0]),
                    requiredlength
                    );
            }

            if (Node->ClassLength > 0) {

                offset = FIELD_OFFSET(KEY_NODE_INFORMATION, Name) +
                            NameLength;
                offset = ALIGN_OFFSET(offset);

                pbuffer->KeyNodeInformation.ClassOffset = offset;

                pclass = HvGetCell(Hive, Node->Class);
                if( pclass == NULL ) {
                    //
                    // we couldn't map this cell
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                pbuffer = (PKEY_INFORMATION)((PUCHAR)pbuffer + offset);

                leftlength = (((LONG)Length - (LONG)offset) < 0) ?
                                    0 :
                                    Length - offset;

                requiredlength = Node->ClassLength;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                RtlCopyMemory(
                    pbuffer,
                    pclass,
                    requiredlength
                    );

                HvReleaseCell(Hive,Node->Class);

            } else {
                pbuffer->KeyNodeInformation.ClassOffset = (ULONG)-1;
            }
        }

        break;


    case KeyFullInformation:
        //
        // LastWriteTime, TitleIndex, ClassOffset, ClassLength,
        // SubKeys, MaxNameLen, MaxClassLen, Values, MaxValueNameLen,
        // MaxValueDataLen, Class
        //
        requiredlength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class) +
                         Node->ClassLength;

        minimumlength = FIELD_OFFSET(KEY_FULL_INFORMATION, Class);

        *ResultLength = requiredlength;

        status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyFullInformation.LastWriteTime =
                Node->LastWriteTime;

            pbuffer->KeyFullInformation.TitleIndex = 0;

            pbuffer->KeyFullInformation.ClassLength =
                Node->ClassLength;

            if (Node->ClassLength > 0) {

                pbuffer->KeyFullInformation.ClassOffset =
                        FIELD_OFFSET(KEY_FULL_INFORMATION, Class);

                pclass = HvGetCell(Hive, Node->Class);
                if( pclass == NULL ) {
                    //
                    // we couldn't map this cell
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                leftlength = Length - minimumlength;
                requiredlength = Node->ClassLength;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    status = STATUS_BUFFER_OVERFLOW;
                }

                RtlCopyMemory(
                    &(pbuffer->KeyFullInformation.Class[0]),
                    pclass,
                    requiredlength
                    );

                HvReleaseCell(Hive,Node->Class);

            } else {
                pbuffer->KeyFullInformation.ClassOffset = (ULONG)-1;
            }

            pbuffer->KeyFullInformation.SubKeys =
                Node->SubKeyCounts[Stable] +
                Node->SubKeyCounts[Volatile];

            pbuffer->KeyFullInformation.Values =
                Node->ValueList.Count;

            pbuffer->KeyFullInformation.MaxNameLen =
                Node->MaxNameLen;

            pbuffer->KeyFullInformation.MaxClassLen =
                Node->MaxClassLen;

            pbuffer->KeyFullInformation.MaxValueNameLen =
                Node->MaxValueNameLen;

            pbuffer->KeyFullInformation.MaxValueDataLen =
                Node->MaxValueDataLen;

        }

        break;


    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    return status;
}

NTSTATUS
CmpQueryKeyDataFromCache(
    PCM_KEY_CONTROL_BLOCK   Kcb,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   KeyInformation,
    ULONG                   Length,
    PULONG                  ResultLength
    )
/*++

Routine Description:

    Do the actual copy of data for a key into caller's buffer.

    If KeyInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

    Works only for the information cached into kcb. I.e. KeyBasicInformation
    and KeyCachedInfo


Arguments:

    Kcb - Supplies pointer to the kcb to be queried

    KeyInformationClass - Specifies the type of information returned in
        Buffer.  One of the following types:

        KeyBasicInformation - return last write time, title index, and name.
            (see KEY_BASIC_INFORMATION structure)

        KeyCachedInformation - return last write time, title index, name ....
            (see KEY_CACHED_INFORMATION structure)

    KeyInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyInformation in bytes.

    ResultLength - Number of bytes actually written into KeyInformation.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS            status;
    PKEY_INFORMATION    pbuffer;
    ULONG               requiredlength;
    USHORT              NameLength;
    PCM_KEY_NODE        Node; // this is to be used only in case of cache incoherency

    CM_PAGED_CODE();

    //
    // we cannot afford to return the kcb NameBlock as the key name
    // for KeyBasicInformation as there are lots of callers expecting
    // the name to be case-sensitive; KeyCachedInformation is new
    // and used only by the Win32 layer, which is not case sensitive
    // Note: future clients of KeyCachedInformation must be made aware 
    // that name is NOT case-sensitive
    //
    ASSERT( KeyInformationClass == KeyCachedInformation );

    // 
    // we are going to need the nameblock; if it is NULL, bail out
    //
    if( Kcb->NameBlock == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pbuffer = (PKEY_INFORMATION)KeyInformation;
    
    if (Kcb->NameBlock->Compressed) {
        NameLength = CmpCompressedNameSize(Kcb->NameBlock->Name,Kcb->NameBlock->NameLength);
    } else {
        NameLength = Kcb->NameBlock->NameLength;
    }
    
    // Assume success
    status = STATUS_SUCCESS;

    switch (KeyInformationClass) {

    case KeyCachedInformation:

        //
        // LastWriteTime, TitleIndex, 
        // SubKeys, MaxNameLen, Values, MaxValueNameLen,
        // MaxValueDataLen, Name
        //
        requiredlength = sizeof(KEY_CACHED_INFORMATION);

        *ResultLength = requiredlength;

        if (Length < requiredlength) {

            status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyCachedInformation.LastWriteTime = Kcb->KcbLastWriteTime;

            pbuffer->KeyCachedInformation.TitleIndex = 0;

            pbuffer->KeyCachedInformation.NameLength = NameLength;

            pbuffer->KeyCachedInformation.Values = Kcb->ValueCache.Count;
            
            pbuffer->KeyCachedInformation.MaxNameLen = Kcb->KcbMaxNameLen;
            
            pbuffer->KeyCachedInformation.MaxValueNameLen = Kcb->KcbMaxValueNameLen;
            
            pbuffer->KeyCachedInformation.MaxValueDataLen = Kcb->KcbMaxValueDataLen;

            if( !(Kcb->ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
                // there is some cached info
                if( Kcb->ExtFlags & CM_KCB_NO_SUBKEY ) {
                    pbuffer->KeyCachedInformation.SubKeys = 0;
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_ONE ) {
                    pbuffer->KeyCachedInformation.SubKeys = 1;
                } else if( Kcb->ExtFlags & CM_KCB_SUBKEY_HINT ) {
                    pbuffer->KeyCachedInformation.SubKeys = Kcb->IndexHint->Count;
                } else {
                    pbuffer->KeyCachedInformation.SubKeys = Kcb->SubKeyCount;
                }
            } else {
                //
                // kcb cache is not coherent; get the info from knode
                // 
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Kcb cache incoherency detected, kcb = %p\n",Kcb));

                Node = (PCM_KEY_NODE)HvGetCell(Kcb->KeyHive,Kcb->KeyCell);
                if( Node == NULL ) {
                    //
                    // couldn't map view for this cell
                    //
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                pbuffer->KeyCachedInformation.SubKeys = Node->SubKeyCounts[Stable] + Node->SubKeyCounts[Volatile];
                HvReleaseCell(Kcb->KeyHive,Kcb->KeyCell);

            }

        }

        break;

    default:
        status = STATUS_INVALID_PARAMETER;
        break;
    }

    return status;
}

VALUE_SEARCH_RETURN_TYPE
CmpQueryKeyValueData(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    PPCM_CACHED_VALUE   ContainingList,
    PCM_KEY_VALUE       ValueKey,
    BOOLEAN             ValueCached,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID               KeyValueInformation,
    ULONG               Length,
    PULONG              ResultLength,
    NTSTATUS            *status
    )
/*++

Routine Description:

    Do the actual copy of data for a key value into caller's buffer.

    If KeyValueInformation is not long enough to hold all requested data,
    STATUS_BUFFER_OVERFLOW will be returned, and ResultLength will be
    set to the number of bytes actually required.

Arguments:

    Hive - supplies a pointer to the hive control structure for the hive

    Cell - supplies index of node to whose sub keys are to be found

    KeyValueInformationClass - Specifies the type of information returned in
        KeyValueInformation.  One of the following types:

    KeyValueInformation -Supplies pointer to buffer to receive the data.

    Length - Length of KeyInformation in bytes.

    ResultLength - Number of bytes actually written into KeyInformation.

Return Value:

    NTSTATUS

--*/
{
    PKEY_VALUE_INFORMATION pbuffer;
    PCELL_DATA  pcell;
    LONG        leftlength;
    ULONG       requiredlength;
    ULONG       minimumlength;
    ULONG       offset;
    ULONG       base;
    ULONG       realsize;
    PUCHAR      datapointer;
    BOOLEAN     small;
    USHORT      NameLength;
    BOOLEAN     BufferAllocated = FALSE;
    HCELL_INDEX CellToRelease = HCELL_NIL;
    PHHIVE      Hive;
    VALUE_SEARCH_RETURN_TYPE SearchValue = SearchSuccess;

    Hive = KeyControlBlock->KeyHive;
    pbuffer = (PKEY_VALUE_INFORMATION)KeyValueInformation;

    pcell = (PCELL_DATA) ValueKey;
    NameLength = CmpValueNameLen(&pcell->u.KeyValue);

    switch (KeyValueInformationClass) {

    case KeyValueBasicInformation:

        //
        // TitleIndex, Type, NameLength, Name
        //
        requiredlength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name) +
                         NameLength;

        minimumlength = FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name);

        *ResultLength = requiredlength;

        *status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            *status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValueBasicInformation.TitleIndex = 0;

            pbuffer->KeyValueBasicInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValueBasicInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                *status = STATUS_BUFFER_OVERFLOW;
            }

            if (pcell->u.KeyValue.Flags & VALUE_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyValueBasicInformation.Name,
                                      requiredlength,
                                      pcell->u.KeyValue.Name,
                                      pcell->u.KeyValue.NameLength);
            } else {
                RtlCopyMemory(&(pbuffer->KeyValueBasicInformation.Name[0]),
                              &(pcell->u.KeyValue.Name[0]),
                              requiredlength);
            }
        }

        break;



    case KeyValueFullInformation:
    case KeyValueFullInformationAlign64:

        //
        // TitleIndex, Type, DataOffset, DataLength, NameLength,
        // Name, Data
        //
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);

        requiredlength = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name) +
                         NameLength +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_FULL_INFORMATION, Name);
        offset = 0;
        if (realsize > 0) {
            base = requiredlength - realsize;

#if defined(_WIN64)

            offset = ALIGN_OFFSET64(base);

#else

            if (KeyValueInformationClass == KeyValueFullInformationAlign64) {
                offset = ALIGN_OFFSET64(base);

            } else {
                offset = ALIGN_OFFSET(base);
            }

#endif

            if (offset > base) {
                requiredlength += (offset - base);
            }

#if DBG && defined(_WIN64)

            //
            // Some clients will have passed in a structure that they "know"
            // will be exactly the right size.  The fact that alignment
            // has changed on NT64 may cause these clients to have problems.
            //
            // The solution is to fix the client, but print out some debug
            // spew here if it looks like this is the case.  This problem
            // isn't particularly easy to spot from the client end.
            //

            if((KeyValueInformationClass == KeyValueFullInformation) &&
                (Length != minimumlength) &&
                (requiredlength > Length) &&
                ((requiredlength - Length) <=
                                (ALIGN_OFFSET64(base) - ALIGN_OFFSET(base)))) {

                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"ntos/config-64 KeyValueFullInformation: "
                                                                 "Possible client buffer size problem.\n"));

                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"    Required size = %d\n", requiredlength));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"    Supplied size = %d\n", Length));
            }

#endif

        }

        *ResultLength = requiredlength;

        *status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            *status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValueFullInformation.TitleIndex = 0;

            pbuffer->KeyValueFullInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValueFullInformation.DataLength =
                realsize;

            pbuffer->KeyValueFullInformation.NameLength =
                NameLength;

            leftlength = Length - minimumlength;
            requiredlength = NameLength;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                *status = STATUS_BUFFER_OVERFLOW;
            }

            if (pcell->u.KeyValue.Flags & VALUE_COMP_NAME) {
                CmpCopyCompressedName(pbuffer->KeyValueFullInformation.Name,
                                      requiredlength,
                                      pcell->u.KeyValue.Name,
                                      pcell->u.KeyValue.NameLength);
            } else {
                RtlCopyMemory(
                    &(pbuffer->KeyValueFullInformation.Name[0]),
                    &(pcell->u.KeyValue.Name[0]),
                    requiredlength
                    );
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    SearchValue = CmpGetValueDataFromCache(KeyControlBlock, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease);
                    if( SearchValue != SearchSuccess ) {
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        *status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                pbuffer->KeyValueFullInformation.DataOffset = offset;

                leftlength = (((LONG)Length - (LONG)offset) < 0) ?
                                    0 :
                                    Length - offset;

                requiredlength = realsize;

                if (leftlength < (LONG)requiredlength) {
                    requiredlength = leftlength;
                    *status = STATUS_BUFFER_OVERFLOW;
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));

                if( datapointer != NULL ) {
                    try {
                        RtlCopyMemory(
                            ((PUCHAR)pbuffer + offset),
                            datapointer,
                            requiredlength
                            );
                    } finally {
                        if( BufferAllocated == TRUE ) {
                            ExFreePool(datapointer);
                        }
                        if( CellToRelease != HCELL_NIL ) {
                            HvReleaseCell(Hive,CellToRelease);
                        }
                    }
                }

            } else {
                pbuffer->KeyValueFullInformation.DataOffset = (ULONG)-1;
            }
        }

        break;


    case KeyValuePartialInformation:

        //
        // TitleIndex, Type, DataLength, Data
        //
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);
        requiredlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

        *ResultLength = requiredlength;

        *status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            *status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValuePartialInformation.TitleIndex = 0;

            pbuffer->KeyValuePartialInformation.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValuePartialInformation.DataLength =
                realsize;

            leftlength = Length - minimumlength;
            requiredlength = realsize;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                *status = STATUS_BUFFER_OVERFLOW;
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    SearchValue = CmpGetValueDataFromCache(KeyControlBlock, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease);
                    if( SearchValue != SearchSuccess ) {
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        *status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));

                if( datapointer != NULL ) {
                    try {
                        RtlCopyMemory((PUCHAR)&(pbuffer->KeyValuePartialInformation.Data[0]),
                                      datapointer,
                                      requiredlength);
                    } finally {
                        if( BufferAllocated == TRUE ) {
                            ExFreePool(datapointer);
                        }
                        if(CellToRelease != HCELL_NIL) {
                            HvReleaseCell(Hive,CellToRelease);
                        }
                    }
                }
            }
        }

        break;
    case KeyValuePartialInformationAlign64:

        //
        // TitleIndex, Type, DataLength, Data
        //
        small = CmpIsHKeyValueSmall(realsize, pcell->u.KeyValue.DataLength);
        requiredlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, Data) +
                         realsize;

        minimumlength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, Data);

        *ResultLength = requiredlength;

        *status = STATUS_SUCCESS;

        if (Length < minimumlength) {

            *status = STATUS_BUFFER_TOO_SMALL;

        } else {

            pbuffer->KeyValuePartialInformationAlign64.Type =
                pcell->u.KeyValue.Type;

            pbuffer->KeyValuePartialInformationAlign64.DataLength =
                realsize;

            leftlength = Length - minimumlength;
            requiredlength = realsize;

            if (leftlength < (LONG)requiredlength) {
                requiredlength = leftlength;
                *status = STATUS_BUFFER_OVERFLOW;
            }

            if (realsize > 0) {

                if (small == TRUE) {
                    datapointer = (PUCHAR)(&(pcell->u.KeyValue.Data));
                } else {
                    SearchValue = CmpGetValueDataFromCache(KeyControlBlock, ContainingList, pcell, ValueCached,&datapointer,&BufferAllocated,&CellToRelease);
                    if( SearchValue != SearchSuccess ) {
                        ASSERT( datapointer == NULL );
                        ASSERT( BufferAllocated == FALSE );
                        *status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                ASSERT((small ? (requiredlength <= CM_KEY_VALUE_SMALL) : TRUE));
                if( datapointer != NULL ) {
                    try {
                        RtlCopyMemory((PUCHAR)&(pbuffer->KeyValuePartialInformationAlign64.Data[0]),
                                      datapointer,
                                      requiredlength);
                    } finally {
                        if( BufferAllocated == TRUE ) {
                            ExFreePool(datapointer);
                        }
                        if(CellToRelease != HCELL_NIL) {
                            HvReleaseCell(Hive,CellToRelease);
                        }
                    }
                }
            }
        }

        break;

    default:
        *status = STATUS_INVALID_PARAMETER;
        break;
    }
    return SearchValue;
}
