/*++                 

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    provider.c

Abstract:
    
    This module registers all of the firmware providers supported by the
    kernel.

--*/

#include "wmikmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,WmipRegisterFirmwareProviders)
#endif

NTSTATUS
__cdecl
WmipRawSMBiosTableHandler(
    PSYSTEM_FIRMWARE_TABLE_INFORMATION TableInfo
    );

NTSTATUS
__cdecl
WmipFirmwareTableHandler(
    PSYSTEM_FIRMWARE_TABLE_INFORMATION TableInfo
    );

VOID
WmipRegisterFirmwareProviders(
                             )
/*++

Routine Description:

    This routine is called to register the correct set of data providers
    with the Executive part of the kernel.
    
    Currently, the SMBIOS raw provider is supported on all platforms. 
    The Firmware provider is only supported on X86 and X64.
    
Arguments:

    None
    
Return Value:

    None         

--*/ {
    SYSTEM_FIRMWARE_TABLE_HANDLER   TableHandler;

    //
    // Register the SMBIOS raw provider.
    //
    TableHandler.ProviderSignature = 'RSMB';    // (Raw SMBIOS)
    TableHandler.Register = TRUE;
    TableHandler.FirmwareTableHandler = &WmipRawSMBiosTableHandler;
    TableHandler.DriverObject = IoPnpDriverObject;
    NtSetSystemInformation(
        SystemRegisterFirmwareTableInformationHandler,
        (PVOID) &TableHandler,
        sizeof(SYSTEM_FIRMWARE_TABLE_HANDLER)
        );

#if defined(_X86_) || defined(_AMD64_)
    //
    // Register the Firmware provider.
    //
    TableHandler.ProviderSignature = 'FIRM';    // (Firmware)
    TableHandler.Register = TRUE;
    TableHandler.FirmwareTableHandler = &WmipFirmwareTableHandler;
    TableHandler.DriverObject = IoPnpDriverObject;
    NtSetSystemInformation(
        SystemRegisterFirmwareTableInformationHandler,
        (PVOID) &TableHandler,
        sizeof(SYSTEM_FIRMWARE_TABLE_HANDLER)
        );
#endif
}

//
// This array defines the legal ranges of memory that can be returned by 
// the FirmwareTableHandler. The format is starting address and length.
//
// Addresses over 4 gigabytes are specifically not included.
//
ULONG   WmipFirmwareTableArray[] = {
    0xC0000,    0x20000,        // Option ROM => C:0000 - D:FFFF
    0xE0000,    0x20000,        // System ROM => E:0000 - F:FFFF
};

NTSTATUS
__cdecl
WmipFirmwareTableHandler(
    PSYSTEM_FIRMWARE_TABLE_INFORMATION TableInfo
    )
/*++

Routine Description:

    This routine handles request for firmware information. 
    
    Currently, the only requested range is the E0000-FFFFF range, but
    we also support handing back the C0000-DFFFF range.
    
Arguments:

     TableInfo  - Information about the request
     
Return Value:

    NTSTATUS        

--*/ {
    PULONG              Buffer;
    PHYSICAL_ADDRESS    Address;
    PCHAR               Firmware;
    ULONG               TableCount;
    ULONG               TableIndex = 0;
    ULONG               TableSize = 0;
    ULONG               i,j;

    ASSERT( TableInfo != NULL );

    if (TableInfo == NULL) {
        return STATUS_INVALID_PARAMETER_1;
    }

    //
    // Calculate the number of tables supported and their length.
    //
    TableCount = sizeof(WmipFirmwareTableArray) / (sizeof(ULONG) * 2);

    //
    // Handle the enumeration case.
    //
    if (TableInfo->Action == SystemFirmwareTable_Enumerate) {

        //
        // We only handle the enumerate of one table.
        //
        TableSize = TableCount * sizeof(ULONG);
        if (TableInfo->TableBufferLength < TableSize) {

            TableInfo->TableBufferLength = TableSize;
            return STATUS_BUFFER_TOO_SMALL;

        }
        TableInfo->TableBufferLength = TableSize;

        //
        // Fill in the starting address so that they can be used as "IDs".
        //
        Buffer = (PULONG) TableInfo->TableBuffer;
        for (i = j = 0; i < TableCount; i++, j += 2) {

            Buffer[i] = WmipFirmwareTableArray[j];

        }
        return STATUS_SUCCESS;

    }

    ASSERT(TableInfo->Action == SystemFirmwareTable_Get);
    if (TableInfo->Action != SystemFirmwareTable_Get) {

        return STATUS_INVALID_PARAMETER_1;

    }

    //
    // Make sure that the length field is correct and that the
    // ID requested matches one of the supported starting addresses.
    //
    TableIndex = TableCount;
    for (i = j = 0; i < TableCount; i++, j += 2) {

        //
        // Determine if the starting location matches the specified ID.
        // If it doesn't, then no match exits.
        //
        if (WmipFirmwareTableArray[j] != TableInfo->TableID) {

            continue;

        }

        //
        // We have a match, so check the size.
        //
        TableSize = WmipFirmwareTableArray[j+1];
        if (TableInfo->TableBufferLength < WmipFirmwareTableArray[j+1]) {

            TableInfo->TableBufferLength = TableSize;
            return STATUS_BUFFER_TOO_SMALL;

        }
        TableInfo->TableBufferLength = TableSize;

        //
        // Remember what the matching index for the table is.
        //
        TableIndex = j;
        break;

    }

    //
    // If we failed to find a match, than abort.
    //
    if (i == TableCount) {

        return STATUS_INVALID_PARAMETER_1;

    }

    //
    // Map in the relevant window.
    //
    Address.QuadPart = WmipFirmwareTableArray[TableIndex];
    Firmware = MmMapIoSpace( Address, TableSize, MmNonCached );
    if (Firmware == NULL) {

        return STATUS_NOT_FOUND;

    }

    //
    // Copy the buffer.
    //
    RtlCopyMemory( TableInfo->TableBuffer, Firmware, TableSize );

    //
    // Unamp the firmware pointer.
    //
    MmUnmapIoSpace( Firmware, TableSize );
    return STATUS_SUCCESS;
}

NTSTATUS
__cdecl
WmipRawSMBiosTableHandler(
    PSYSTEM_FIRMWARE_TABLE_INFORMATION TableInfo
    )
/*++

Routine Description:

    This routine handles request for raw SMBIOS information. 
    
    Currently, this returns a single table.
    
Arguments:

     TableInfo  - Information about the request
     
Return Value:

    NTSTATUS        

--*/ {
    NTSTATUS            status;
    PUCHAR              Buffer = NULL;
    PUCHAR              SMBios = NULL;
    PSMBIOSVERSIONINFO  VersionInfo = NULL;
    PULONG              TableSizePtr = NULL;
    ULONG               HeaderSize = 0;
    ULONG               SizeNeeded = 0;
    ULONG               TableSize = 0;
    ASSERT( TableInfo != NULL );

    if (TableInfo == NULL) {

        return STATUS_INVALID_PARAMETER_1;

    }

    if (TableInfo->Action == SystemFirmwareTable_Enumerate) {

        //
        // We only handle the enumerate of one table.
        //
        if (TableInfo->TableBufferLength < sizeof(ULONG)) {

            TableInfo->TableBufferLength = sizeof(ULONG);
            return STATUS_BUFFER_TOO_SMALL;

        }
        TableInfo->TableBufferLength = sizeof(ULONG);

        //
        // The "TableID" of the only table is '0'. Make sure that we
        // indicate that only one table exists.
        //
        TableSizePtr = (PULONG) TableInfo->TableBuffer;
        TableSizePtr[0] = 0;
        return STATUS_SUCCESS;
    }

    ASSERT(TableInfo->Action == SystemFirmwareTable_Get);
    if (TableInfo->Action != SystemFirmwareTable_Get) {

        return STATUS_INVALID_PARAMETER_1;

    }

    //
    // Calculate the amount of space required.
    //
    status = WmipGetSMBiosTableData( NULL, &TableSize, NULL );
    if (status != STATUS_BUFFER_TOO_SMALL || TableSize == 0) {

        TableInfo->TableBufferLength = 0;
        return status;

    }
    HeaderSize = sizeof(SMBIOSVERSIONINFO) + sizeof(ULONG);
    SizeNeeded = HeaderSize + TableSize;

    //
    // If the caller didn't provide enough memory, then fail the call.
    //
    if (TableInfo->TableBufferLength < SizeNeeded) {

        TableInfo->TableBufferLength = SizeNeeded;
        return STATUS_BUFFER_TOO_SMALL;

    }

    //
    // Setup all the pointers that we will need for getting a real copy
    // of the tables.
    //
    Buffer = (PUCHAR) TableInfo->TableBuffer;
    VersionInfo = (PSMBIOSVERSIONINFO) Buffer;
    TableSizePtr = (PULONG) (Buffer + sizeof(SMBIOSVERSIONINFO));
    SMBios = Buffer + HeaderSize;

    //
    // Get the full copy of the tables this time around.
    //
    status = WmipGetSMBiosTableData( SMBios, &TableSize, VersionInfo );
    if (NT_SUCCESS(status)) {

        *TableSizePtr = TableSize;

    }
    TableInfo->TableBufferLength = HeaderSize + TableSize;
    return status;
}

