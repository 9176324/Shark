/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    lookup.c

Abstract:

    This module implements function table lookup for platforms with table
    base exception handling.

--*/

#include "ntrtlp.h"

//
// Define external data.
//

#if !defined(_X86_)
#pragma alloc_text(INIT, RtlInitializeHistoryTable)
#endif

//
// Define global unwind history table to hold the constant unwind entries
// for exception dispatch followed by unwind.
//

#if !defined(_X86_)

UNWIND_HISTORY_TABLE RtlpUnwindHistoryTable = {
    0, UNWIND_HISTORY_TABLE_NONE, - 1, 0};

VOID
RtlInitializeHistoryTable (
    VOID
    )

/*++

Routine Description:

    This function initializes the global unwind history table.

Arguments:

    None.

Return Value:

    None.

--*/

{

    ULONG64 BeginAddress;
    ULONG64 ControlPc;
    ULONG64 EndAddress;
    PVOID *FunctionAddressTable;
    PRUNTIME_FUNCTION FunctionEntry;
    ULONG64 Gp;
    ULONG64 ImageBase;
    ULONG Index;

    //
    // Lookup function entries from the function address table until a NULL
    // entry is encountered or the unwind history table is full.
    //

    FunctionAddressTable = &RtlpFunctionAddressTable[0];
    Index = 0;
    while ((Index < UNWIND_HISTORY_TABLE_SIZE) &&
           (*FunctionAddressTable != NULL)) {

        ControlPc = (ULONG64)*FunctionAddressTable++;
        FunctionEntry = RtlLookupFunctionEntry(ControlPc,
                                               &ImageBase,
                                               NULL
                                               );

        ASSERT(FunctionEntry != NULL);

        BeginAddress = FunctionEntry->BeginAddress + ImageBase;
        EndAddress = FunctionEntry->EndAddress + ImageBase;
        RtlpUnwindHistoryTable.Entry[Index].ImageBase = ImageBase;

        RtlpUnwindHistoryTable.Entry[Index].FunctionEntry = FunctionEntry;
        if (BeginAddress < RtlpUnwindHistoryTable.LowAddress) {
            RtlpUnwindHistoryTable.LowAddress = BeginAddress;
        }

        if (EndAddress > RtlpUnwindHistoryTable.HighAddress) {
            RtlpUnwindHistoryTable.HighAddress = EndAddress;
        }

        Index += 1;
    }

    RtlpUnwindHistoryTable.Count = Index;
    return;
}

PRUNTIME_FUNCTION
RtlpSearchInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG SizeOfTable
    )

/*++

Routine Description:

    This function searches for a matching entry in an inverted function
    table using the specified control PC value.

    N.B. It is assumed that appropriate locks are held when this routine
         is called.

Arguments:

    InvertedTable - Supplies a pointer to an inverted function table.

    ControlPc - Supplies a PC value to to use in searching the inverted
        function table.

    ImageBase - Supplies a pointer to a variable that receives the base
         address of the corresponding module.

    SizeOfTable - Supplies a pointer to a variable that receives the size
         of the function table in bytes.

Return Value:

    If a matching entry is located in the specified function table, then
    the function table address is returned as the function value. Otherwise,
    a value of NULL is returned.

--*/

{

    PVOID Bound;
    LONG High;
    ULONG Index;
    PINVERTED_FUNCTION_TABLE_ENTRY InvertedEntry;
    LONG Low;
    LONG Middle;

    //
    // If there are any entries in the specified inverted function table,
    // then search the table for a matching entry.
    //

    if (InvertedTable->CurrentSize != 0) {
        Low = 0;
        High = InvertedTable->CurrentSize - 1;
        while (High >= Low) {

            //
            // Compute next probe index and test entry. If the specified
            // control PC is greater than of equal to the beginning address
            // and less than the ending address of the inverted function
            // table entry, then return the address of the function table.
            // Otherwise, continue the search.
            //

            Middle = (Low + High) >> 1;
            InvertedEntry = &InvertedTable->TableEntry[Middle];
            Bound = (PVOID)((ULONG_PTR)InvertedEntry->ImageBase + InvertedEntry->SizeOfImage);
            if (ControlPc < InvertedEntry->ImageBase) {
                High = Middle - 1;

            } else if (ControlPc >= Bound) {
                Low = Middle + 1;

            } else {
                *ImageBase = InvertedEntry->ImageBase;
                *SizeOfTable = InvertedEntry->SizeOfTable;
                return InvertedEntry->FunctionTable;
            }
        }
    }

    return NULL;
}

#endif

VOID
RtlCaptureImageExceptionValues (
    IN  PVOID Base,
    OUT PVOID *FunctionTable,
    OUT PULONG TableSize
    )

/*++

Routine Description:

    This function queries the image exception information.

Arguments:

    Base - Supplies the base address of the image.

    FunctionTable - Supplies a pointer to a variable that receives the address
        of the function table, if any.

    Gp - Supplies a pointer to a variable that receives the GP pointer value,
        if any.

    TableSize - Supplies a pointer to a variable that receives the size of the
        function table, if any.

Return Value:

    None.

--*/

{

#if defined(_X86_)

    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig;
    ULONG LoadConfigSize;

    NtHeaders = RtlImageNtHeader(Base);
    if (NtHeaders->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {

        //
        // SEH is not possible.
        //

        *FunctionTable = (PCHAR)LongToPtr(-1);
        *TableSize = (ULONG)-1;

    } else {
        LoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY32)
                         RtlImageDirectoryEntryToData(Base,
                                                      TRUE,
                                                      IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                      &LoadConfigSize);
        if (LoadConfig && 
            (LoadConfigSize == 0x40) &&     // 0x40 is what Win2k expects - anything else is an error.
            LoadConfig->Size >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount) &&
            LoadConfig->SEHandlerTable &&
            LoadConfig->SEHandlerCount) {

            *FunctionTable = (PVOID)LoadConfig->SEHandlerTable;
            *TableSize = LoadConfig->SEHandlerCount;

        } else {

            //
            // Check iof the image is an ILONLY COR image.
            //

            PIMAGE_COR20_HEADER Cor20Header;
            ULONG Cor20HeaderSize;
            Cor20Header = RtlImageDirectoryEntryToData(Base,
                                                       TRUE,
                                                       IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                       &Cor20HeaderSize);

            if (Cor20Header && ((Cor20Header->Flags & COMIMAGE_FLAGS_ILONLY) == COMIMAGE_FLAGS_ILONLY)) {
        
                //
                // SEH is not possible.
                //
        
                *FunctionTable = (PCHAR)LongToPtr(-1);
                *TableSize = (ULONG)-1;

            } else {
                *FunctionTable = 0;
                *TableSize = 0;
            }
        }
    }

#else

    *FunctionTable = RtlImageDirectoryEntryToData(Base,
                                                 TRUE,
                                                 IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                 TableSize);
#endif

}

#if defined(_X86_)

PVOID

#else

PRUNTIME_FUNCTION

#endif

RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG SizeOfTable
    )

/*++

Routine Description:

    This function looks up the control PC in the loaded module list, and
    returns the image base, the size of the function table, and the address
    of the function table.

Arguments:

    ControlPc - Supplies an address in the module to be looked up.

    ImageBase - Supplies a pointer to a variable that receives the base
         address of the corresponding module.

    SizeOfTable - Supplies a pointer to a variable that receives the size
         of the function table in bytes.

Return Value:

    If a module is found that contains the specified control PC value and
    that module contains a function table, then the address of the function
    table is returned as the function value. Otherwise, NULL is returned.

--*/

{
    PVOID Base;
    ULONG_PTR Bound;

    PKLDR_DATA_TABLE_ENTRY Entry;

    PLIST_ENTRY Next;

#if defined(_X86_)

    PVOID FunctionTable;

#else

    PRUNTIME_FUNCTION FunctionTable;

#endif

    KIRQL OldIrql;

    //
    // Acquire the loaded module list spinlock and scan the list for the
    // specified PC value if the list has been initialized.
    //

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < SYNCH_LEVEL) {
        KeRaiseIrqlToSynchLevel();
    }

    ExAcquireSpinLockAtDpcLevel(&PsLoadedModuleSpinLock);

#ifndef _X86_

    FunctionTable = RtlpSearchInvertedFunctionTable(&PsInvertedFunctionTable,
                                                    ControlPc,
                                                    &Base,
                                                    SizeOfTable);

    if ((FunctionTable == NULL) &&
        (PsInvertedFunctionTable.Overflow != FALSE))

#endif

    {
        Next = PsLoadedModuleList.Flink;
        if (Next != NULL) {
            while (Next != &PsLoadedModuleList) {
                Entry = CONTAINING_RECORD(Next,
                                          KLDR_DATA_TABLE_ENTRY,
                                          InLoadOrderLinks);
    
                Base = Entry->DllBase;
                Bound = (ULONG_PTR)Base + Entry->SizeOfImage;
                if (((ULONG_PTR)ControlPc >= (ULONG_PTR)Base) &&
                    ((ULONG_PTR)ControlPc < Bound)) {
                    FunctionTable = Entry->ExceptionTable;
                    *SizeOfTable = Entry->ExceptionTableSize;
                    break;
                }

                Next = Next->Flink;
            }
        }
    }

    //
    // Release the loaded module list spin lock.
    //

    ExReleaseSpinLockFromDpcLevel(&PsLoadedModuleSpinLock);
    KeLowerIrql(OldIrql);

    //
    // Set the image base address and return the function table address.
    //

    *ImageBase = Base;
    return FunctionTable;
}

#if !defined(_X86_)

VOID
RtlInsertInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase,
    ULONG SizeOfImage
    )

/*++

Routine Description:

    This function inserts an entry in an inverted function table if there
    is room in the table. Otherwise, no operation is performed.

    N.B. It is assumed that appropriate locks are held when this routine
         is called.

    N.B. If the inverted function table overflows, then it is treated as
         a cache. This is unlikely to happen, however.

Arguments:

    InvertedTable - Supplies a pointer to the inverted function table in
        which the specified entry is to be inserted.

    ImageBase - Supplies the base address of the containing image.

    SizeOfImage - Supplies the size of the image.

Return Value:

    None.

--*/

{

    ULONG CurrentSize;
    PRUNTIME_FUNCTION FunctionTable;
    ULONG Index;
    ULONG SizeOfTable;

    //
    // If the inverted table is not full, then insert the entry in the
    // specified inverted table.
    //

    CurrentSize = InvertedTable->CurrentSize;
    if (CurrentSize != InvertedTable->MaximumSize) {

        //
        // If the inverted table has no entries, then insert the new entry as
        // the first entry. Otherwise, search the inverted table for the proper
        // insert position, shuffle the table, and insert the new entry.
        //
    
        Index = 0;
        if (CurrentSize != 0) {
            for (Index = 0; Index < CurrentSize; Index += 1) {
                if (ImageBase < InvertedTable->TableEntry[Index].ImageBase) {
                    break;
                }
            }

            //
            // If the new entry does not go at the end of the specified table,
            // then shuffle the table down to make room for the new entry.
            //

            if (Index != CurrentSize) {
                RtlMoveMemory(&InvertedTable->TableEntry[Index + 1],
                              &InvertedTable->TableEntry[Index],
                              (CurrentSize - Index) * sizeof(INVERTED_FUNCTION_TABLE_ENTRY));
            }
        }
    
        //
        // Insert the specified entry in the specified inverted function table.
        //
    
        FunctionTable = RtlImageDirectoryEntryToData (ImageBase,
                                                      TRUE,
                                                      IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                      &SizeOfTable);

        InvertedTable->TableEntry[Index].FunctionTable = FunctionTable;
        InvertedTable->TableEntry[Index].ImageBase = ImageBase;
        InvertedTable->TableEntry[Index].SizeOfImage = SizeOfImage;
        InvertedTable->TableEntry[Index].SizeOfTable = SizeOfTable;
        InvertedTable->CurrentSize += 1;

    } else {
        InvertedTable->Overflow = TRUE;
    }

    return;
}

VOID
RtlRemoveInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase
    )

/*++

Routine Description:

    This routine removes an entry from an inverted function table.

    N.B. It is assumed that appropriate locks are held when this routine
         is called.

Arguments:

    InvertedTable - Supplies a pointer to the inverted function table from
        which the specified entry is to be removed.

    ImageBase - Supplies the base address of the containing image. 

Return Value:

    None.

--*/

{

    ULONG CurrentSize;
    ULONG Index;

    //
    // Search for an entry in the specified inverted table that matches the
    // image base.
    //
    // N.B. It is possible a matching entry is not in the inverted table
    //      the table was full when an attempt was made to insert the
    //      corresponding entry.
    //

    CurrentSize = InvertedTable->CurrentSize;
    for (Index = 0; Index < CurrentSize; Index += 1) {
        if (ImageBase == InvertedTable->TableEntry[Index].ImageBase) {
            break;
        }
    }

    //
    // If the entry was found in the inverted table, then remove the entry
    // and reduce the size of the table.
    //

    if (Index != CurrentSize) {

        //
        // If the size of the table is not one, then shuffle the table and
        // remove the specified entry.
        //
    
        if (CurrentSize != 1) {
            RtlMoveMemory(&InvertedTable->TableEntry[Index],
                          &InvertedTable->TableEntry[Index + 1],
                          (CurrentSize - Index - 1) * sizeof(INVERTED_FUNCTION_TABLE_ENTRY));
        }
    
        //
        // Reduce the size of the inverted table.
        //
    
        InvertedTable->CurrentSize -= 1;
    }

    return;
}

#endif      // !_X86_

