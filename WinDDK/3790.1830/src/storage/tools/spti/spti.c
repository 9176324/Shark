/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spti.c

Abstract:

    Win32 application that can communicate directly with SCSI devices via
    IOCTLs.  

Author:


Environment:

    User mode.

Notes:


Revision History:

--*/

#include <windows.h>
#include <devioctl.h>
#include <ntdddisk.h>
#include <ntddscsi.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "spti.h"

#define NAME_COUNT  25

#define BOOLEAN_TO_STRING(_b_) \
( (_b_) ? "True" : "False" )

#if defined(_X86_)
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#elif defined(_AMD64_)
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#elif defined(_IA64_)
    #define PAGE_SIZE 0x2000
    #define PAGE_SHIFT 13L
#else
    // undefined platform?
    #define PAGE_SIZE  0x1000
    #define PAGE_SHIFT 12L
#endif


PUCHAR BusTypeStrings[] = {
    "Unknown",
    "Scsi",
    "Atapi",
    "Ata",
    "1394",
    "Ssa",
    "Fibre",
    "Usb",
    "RAID",
    "Not Defined",
};
#define NUMBER_OF_BUS_TYPE_STRINGS (sizeof(BusTypeStrings)/sizeof(BusTypeStrings[0]))

VOID
__cdecl
main(
    int argc,
    char *argv[]
    )

{
    BOOL status = 0;
    DWORD accessMode = 0, shareMode = 0;
    HANDLE fileHandle = NULL;
    ULONG alignmentMask = 0; // default == no alignment requirement
    PUCHAR dataBuffer = NULL;
    SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;
    SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;    
    UCHAR string[NAME_COUNT + 1];

    ULONG length = 0,
          errorCode = 0,
          returned = 0,
          sectorSize = 512;

    if ((argc < 2) || (argc > 3)) {
       printf("Usage:  %s <port-name> [-mode]\n", argv[0] );
       printf("Examples:\n");
       printf("    spti g:       (open the disk class driver in SHARED READ/WRITE mode)\n");    
       printf("    spti Scsi2:   (open the miniport driver for the 3rd host adapter)\n");
       printf("    spti Tape0 w  (open the tape class driver in SHARED WRITE mode)\n");
       printf("    spti i: c     (open the CD-ROM class driver in SHARED READ mode)\n");
       return;
    }

    _snprintf(string, NAME_COUNT, "\\\\.\\%s", argv[1]);
    string[NAME_COUNT] = 0;

    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
    accessMode = GENERIC_WRITE | GENERIC_READ;       // default

    if (argc == 3) {

        switch(tolower(argv[2][0])) {
            case 'r':
                shareMode = FILE_SHARE_READ;
                break;

            case 'w':
                shareMode = FILE_SHARE_WRITE;
                break;

            case 'c':
                shareMode = FILE_SHARE_READ;
                sectorSize = 2048;
                break;

            default:
                printf("%s is an invalid mode.\n", argv[2]);
                puts("\tr = read");
                puts("\tw = write");
                puts("\tc = read CD (2048 byte sector mode)");
                return;
        }
    }

    fileHandle = CreateFile(string,
       accessMode,
       shareMode,
       NULL,
       OPEN_EXISTING,
       0,
       NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        errorCode = GetLastError();
        printf("Error opening %s. Error: %d\n",
               string, errorCode);
        PrintError(errorCode);
        return;
    }

    //
    // Get the alignment requirements
    //

    status = GetAlignmentMaskForDevice(fileHandle, &alignmentMask);
    if (!status ) {
        errorCode = GetLastError();
        printf("Error getting device and/or adapter properties; "
               "error was %d\n", errorCode);
        PrintError(errorCode);
        return;
    }

    printf("\n"
           "            *****     Detected Alignment Mask    *****\n"
           "            *****             was %08x       *****\n\n\n",
           alignmentMask);

    puts("            ***** MODE SENSE -- return all pages *****");
    puts("            *****      with SenseInfo buffer     *****\n");

    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = 0;
    sptwb.spt.TargetId = 1;
    sptwb.spt.Lun = 0;
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 192;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptwb.spt.SenseInfoOffset = 
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.spt.Cdb[2] = MODE_SENSE_RETURN_ALL;
    sptwb.spt.Cdb[4] = 192;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       sptwb.spt.DataTransferLength;

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,&sptwb,length);

    printf("            ***** MODE SENSE -- return all pages *****\n");
    printf("            *****    without SenseInfo buffer    *****\n\n");

    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = 0;
    sptwb.spt.TargetId = 1;
    sptwb.spt.Lun = 0;
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = 0;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 192;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptwb.spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.spt.Cdb[2] = MODE_SENSE_RETURN_ALL;
    sptwb.spt.Cdb[4] = 192;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       sptwb.spt.DataTransferLength;

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,&sptwb,length);
    
    printf("            *****      TEST UNIT READY      *****\n");
    printf("            *****    DataBufferLength = 0   *****\n\n");


    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = 0;
    sptwb.spt.TargetId = 1;
    sptwb.spt.Lun = 0;
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 0;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset = 0;
    sptwb.spt.SenseInfoOffset = 
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = SCSIOP_TEST_UNIT_READY;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);

    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,&sptwb,length);

    //
    //  Do a mode sense with a bad data buffer offset.  This will fail.
    //
    printf("            *****      MODE SENSE -- return all pages      *****\n");
    printf("            *****   bad DataBufferOffset -- should fail    *****\n\n");

    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = 0;
    sptwb.spt.TargetId = 1;
    sptwb.spt.Lun = 0;
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = 0;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 192;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset = 0;
    sptwb.spt.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.spt.Cdb[2] = MODE_SENSE_RETURN_ALL;
    sptwb.spt.Cdb[4] = 192;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       sptwb.spt.DataTransferLength;
    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,&sptwb,length);


    //
    // Get caching mode sense page.
    //
    printf("            *****               MODE SENSE                  *****\n");
    printf("            *****     return caching mode sense page        *****\n\n");

    ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

    sptwb.spt.Length = sizeof(SCSI_PASS_THROUGH);
    sptwb.spt.PathId = 0;
    sptwb.spt.TargetId = 1;
    sptwb.spt.Lun = 0;
    sptwb.spt.CdbLength = CDB6GENERIC_LENGTH;
    sptwb.spt.SenseInfoLength = SPT_SENSE_LENGTH;
    sptwb.spt.DataIn = SCSI_IOCTL_DATA_IN;
    sptwb.spt.DataTransferLength = 192;
    sptwb.spt.TimeOutValue = 2;
    sptwb.spt.DataBufferOffset =
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf);
    sptwb.spt.SenseInfoOffset = 
       offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucSenseBuf);
    sptwb.spt.Cdb[0] = SCSIOP_MODE_SENSE;
    sptwb.spt.Cdb[1] = 0x08; // target shall not return any block descriptors
    sptwb.spt.Cdb[2] = MODE_PAGE_CACHING;
    sptwb.spt.Cdb[4] = 192;
    length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS,ucDataBuf) +
       sptwb.spt.DataTransferLength;
    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH,
                             &sptwb,
                             sizeof(SCSI_PASS_THROUGH),
                             &sptwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,&sptwb,length);

    printf("            *****       WRITE DATA BUFFER operation         *****\n");

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    dataBuffer = AllocateAlignedBuffer(sectorSize,alignmentMask);
    FillMemory(dataBuffer,sectorSize/2,'N');
    FillMemory(dataBuffer + sectorSize/2,sectorSize/2,'T');

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = SPT_SENSE_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
    sptdwb.sptd.DataTransferLength = sectorSize;
    sptdwb.sptd.TimeOutValue = 2;
    sptdwb.sptd.DataBuffer = dataBuffer;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = SCSIOP_WRITE_DATA_BUFF;
    sptdwb.sptd.Cdb[1] = 2;                         // Data mode
    sptdwb.sptd.Cdb[7] = (UCHAR)(sectorSize >> 8);  // Parameter List length
    sptdwb.sptd.Cdb[8] = 0;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,
       (PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptdwb,length);
    if ((sptdwb.sptd.ScsiStatus == 0) && (status != 0)) {
       PrintDataBuffer(dataBuffer,sptdwb.sptd.DataTransferLength);
       }

    printf("            *****       READ DATA BUFFER operation         *****\n");

    ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    ZeroMemory(dataBuffer,sectorSize);

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = SPT_SENSE_LENGTH;
    sptdwb.sptd.DataTransferLength = sectorSize;
    sptdwb.sptd.TimeOutValue = 2;
    sptdwb.sptd.DataBuffer = dataBuffer;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = SCSIOP_READ_DATA_BUFF;
    sptdwb.sptd.Cdb[1] = 2;                         // Data mode
    sptdwb.sptd.Cdb[7] = (UCHAR)(sectorSize >> 8);  // Parameter List length
    sptdwb.sptd.Cdb[8] = 0;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(fileHandle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

    PrintStatusResults(status,returned,
       (PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptdwb,length);
    if ((sptdwb.sptd.ScsiStatus == 0) && (status != 0)) {
       PrintDataBuffer(dataBuffer,sptdwb.sptd.DataTransferLength);
       }
}

VOID
PrintError(ULONG ErrorCode)
{
    UCHAR errorBuffer[80];
    ULONG count;

    count = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  ErrorCode,
                  0,
                  errorBuffer,
                  sizeof(errorBuffer),
                  NULL
                  );

    if (count != 0) {
        printf("%s\n", errorBuffer);
    } else {
        printf("Format message failed.  Error: %d\n", GetLastError());
    }
}

VOID
PrintDataBuffer(PUCHAR DataBuffer, ULONG BufferLength)
{
    ULONG Cnt;

    printf("      00  01  02  03  04  05  06  07   08  09  0A  0B  0C  0D  0E  0F\n");
    printf("      ---------------------------------------------------------------\n");
    for (Cnt = 0; Cnt < BufferLength; Cnt++) {
       if ((Cnt) % 16 == 0) {
          printf(" %03X  ",Cnt);
          }
       printf("%02X  ", DataBuffer[Cnt]);
       if ((Cnt+1) % 8 == 0) {
          printf(" ");
          }
       if ((Cnt+1) % 16 == 0) {
          printf("\n");
          }
       }
    printf("\n\n");
}

VOID
PrintAdapterDescriptor(PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptor)
{
    ULONG trueMaximumTransferLength;
    PUCHAR busType;

    if (AdapterDescriptor->BusType <= NUMBER_OF_BUS_TYPE_STRINGS) {
        busType = BusTypeStrings[AdapterDescriptor->BusType];
    } else {
        busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS-1];
    }

    // subtract one page, as transfers do not always start on a page boundary
    if (AdapterDescriptor->MaximumPhysicalPages > 1) {
        trueMaximumTransferLength = AdapterDescriptor->MaximumPhysicalPages - 1;
    } else {
        trueMaximumTransferLength = 1;
    }
    // make it into a byte value
    trueMaximumTransferLength <<= PAGE_SHIFT;

    // take the minimum of the two
    if (trueMaximumTransferLength > AdapterDescriptor->MaximumTransferLength) {
        trueMaximumTransferLength = AdapterDescriptor->MaximumTransferLength;
    }

    // always allow at least a single page transfer
    if (trueMaximumTransferLength < PAGE_SIZE) {
        trueMaximumTransferLength = PAGE_SIZE;
    }

    puts("\n            ***** STORAGE ADAPTER DESCRIPTOR DATA *****");
    printf("              Version: %08x\n"
           "            TotalSize: %08x\n"
           "MaximumTransferLength: %08x (bytes)\n"
           " MaximumPhysicalPages: %08x\n"
           "  TrueMaximumTransfer: %08x (bytes)\n"
           "        AlignmentMask: %08x\n"
           "       AdapterUsesPio: %s\n"
           "     AdapterScansDown: %s\n"
           "      CommandQueueing: %s\n"
           "  AcceleratedTransfer: %s\n"
           "             Bus Type: %s\n"
           "    Bus Major Version: %04x\n"
           "    Bus Minor Version: %04x\n",
           AdapterDescriptor->Version,
           AdapterDescriptor->Size,
           AdapterDescriptor->MaximumTransferLength,
           AdapterDescriptor->MaximumPhysicalPages,
           trueMaximumTransferLength,
           AdapterDescriptor->AlignmentMask,
           BOOLEAN_TO_STRING(AdapterDescriptor->AdapterUsesPio),
           BOOLEAN_TO_STRING(AdapterDescriptor->AdapterScansDown),
           BOOLEAN_TO_STRING(AdapterDescriptor->CommandQueueing),
           BOOLEAN_TO_STRING(AdapterDescriptor->AcceleratedTransfer),
           busType,
           AdapterDescriptor->BusMajorVersion,
           AdapterDescriptor->BusMinorVersion);


    

    printf("\n\n");
}

VOID
PrintDeviceDescriptor(PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptor)
{
    PUCHAR vendorId = "";
    PUCHAR productId = "";
    PUCHAR productRevision = "";
    PUCHAR serialNumber = "";
    PUCHAR busType;

    if (DeviceDescriptor->BusType <= NUMBER_OF_BUS_TYPE_STRINGS) {
        busType = BusTypeStrings[DeviceDescriptor->BusType];
    } else {
        busType = BusTypeStrings[NUMBER_OF_BUS_TYPE_STRINGS-1];
    }

    if ((DeviceDescriptor->ProductIdOffset != 0) &&
        (DeviceDescriptor->ProductIdOffset != -1)) {
        productId        = (PUCHAR)(DeviceDescriptor);
        productId       += (ULONG_PTR)DeviceDescriptor->ProductIdOffset;
    }
    if ((DeviceDescriptor->VendorIdOffset != 0) &&
        (DeviceDescriptor->VendorIdOffset != -1)) {
        vendorId         = (PUCHAR)(DeviceDescriptor);
        vendorId        += (ULONG_PTR)DeviceDescriptor->VendorIdOffset;
    }
    if ((DeviceDescriptor->ProductRevisionOffset != 0) &&
        (DeviceDescriptor->ProductRevisionOffset != -1)) {
        productRevision  = (PUCHAR)(DeviceDescriptor);
        productRevision += (ULONG_PTR)DeviceDescriptor->ProductRevisionOffset;
    }
    if ((DeviceDescriptor->SerialNumberOffset != 0) &&
        (DeviceDescriptor->SerialNumberOffset != -1)) {
        serialNumber     = (PUCHAR)(DeviceDescriptor);
        serialNumber    += (ULONG_PTR)DeviceDescriptor->SerialNumberOffset;
    }


    puts("\n            ***** STORAGE DEVICE DESCRIPTOR DATA *****");
    printf("              Version: %08x\n"
           "            TotalSize: %08x\n"
           "           DeviceType: %08x\n"
           "   DeviceTypeModifier: %08x\n"
           "       RemovableMedia: %s\n"
           "      CommandQueueing: %s\n"
           "            Vendor Id: %s\n"
           "           Product Id: %s\n"
           "     Product Revision: %s\n"
           "        Serial Number: %s\n"
           "             Bus Type: %s\n"
           "       Raw Properties: %s\n",
           DeviceDescriptor->Version,
           DeviceDescriptor->Size,
           DeviceDescriptor->DeviceType,
           DeviceDescriptor->DeviceTypeModifier,
           BOOLEAN_TO_STRING(DeviceDescriptor->RemovableMedia),
           BOOLEAN_TO_STRING(DeviceDescriptor->CommandQueueing),
           vendorId,
           productId,
           productRevision,
           serialNumber,
           busType,
           (DeviceDescriptor->RawPropertiesLength ? "Follows" : "None"));
    if (DeviceDescriptor->RawPropertiesLength != 0) {
        PrintDataBuffer(DeviceDescriptor->RawDeviceProperties,
                        DeviceDescriptor->RawPropertiesLength);
    }
    printf("\n\n");
}


PUCHAR
AllocateAlignedBuffer(ULONG size, ULONG AlignmentMask)
{
    PUCHAR ptr;

    // NOTE: This routine does not allow for a way to free
    //       memory.  This is an excercise left for the reader.
    UINT_PTR    align64 = (UINT_PTR)AlignmentMask;

    if (AlignmentMask == 0) {
       ptr = malloc(size);
    } else {
       ptr = malloc(size + AlignmentMask);
       ptr = (PUCHAR)(((UINT_PTR)ptr + align64) & ~align64);
    }
    
    if (ptr == NULL) {
       printf("Memory allocation error.  Terminating program\n");
       exit(1);
    } else {
       return ptr;
    }
}

VOID
PrintStatusResults(
    BOOL status,DWORD returned,PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb,
    ULONG length)
{
    ULONG errorCode;

    if (!status ) {
       printf( "Error: %d  ",
          errorCode = GetLastError() );
       PrintError(errorCode);
       return;
       }
    if (psptwb->spt.ScsiStatus) {
       PrintSenseInfo(psptwb);
       return;
       }
    else {
       printf("Scsi status: %02Xh, Bytes returned: %Xh, ",
          psptwb->spt.ScsiStatus,returned);
       printf("Data buffer length: %Xh\n\n\n",
          psptwb->spt.DataTransferLength);
       PrintDataBuffer((PUCHAR)psptwb,length);
       }
}

VOID
PrintSenseInfo(PSCSI_PASS_THROUGH_WITH_BUFFERS psptwb)
{
    int i;

    printf("Scsi status: %02Xh\n\n",psptwb->spt.ScsiStatus);
    if (psptwb->spt.SenseInfoLength == 0) {
       return;
       }
    printf("Sense Info -- consult SCSI spec for details\n");
    printf("-------------------------------------------------------------\n");
    for (i=0; i < psptwb->spt.SenseInfoLength; i++) {
       printf("%02X ",psptwb->ucSenseBuf[i]);
       }
    printf("\n\n");
}

BOOL
GetAlignmentMaskForDevice(
    IN HANDLE DeviceHandle,
    OUT PULONG AlignmentMask
    )
{
    PSTORAGE_ADAPTER_DESCRIPTOR adapterDescriptor = NULL;
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor = NULL;
    STORAGE_DESCRIPTOR_HEADER header = {0};

    BOOL ok = TRUE;
    BOOL failed = TRUE;
    ULONG i;

    *AlignmentMask = 0; // default to no alignment

    // Loop twice:
    //  First, get size required for storage adapter descriptor
    //  Second, allocate and retrieve storage adapter descriptor
    //  Third, get size required for storage device descriptor
    //  Fourth, allocate and retrieve storage device descriptor
    for (i=0;i<4;i++) {

        PVOID buffer;
        ULONG bufferSize;
        ULONG returnedData;
        
        STORAGE_PROPERTY_QUERY query = {0};

        switch(i) {
            case 0: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageAdapterProperty;
                bufferSize = sizeof(STORAGE_DESCRIPTOR_HEADER);
                buffer = &header;
                break;
            }
            case 1: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageAdapterProperty;
                bufferSize = header.Size;
                if (bufferSize != 0) {
                    adapterDescriptor = LocalAlloc(LPTR, bufferSize);
                    if (adapterDescriptor == NULL) {
                        goto Cleanup;
                    }
                }
                buffer = adapterDescriptor;
                break;
            }
            case 2: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageDeviceProperty;
                bufferSize = sizeof(STORAGE_DESCRIPTOR_HEADER);
                buffer = &header;
                break;
            }
            case 3: {
                query.QueryType = PropertyStandardQuery;
                query.PropertyId = StorageDeviceProperty;
                bufferSize = header.Size;

                if (bufferSize != 0) {
                    deviceDescriptor = LocalAlloc(LPTR, bufferSize);
                    if (deviceDescriptor == NULL) {
                        goto Cleanup;
                    }
                }
                buffer = deviceDescriptor;
                break;
            }
        }

        // buffer can be NULL if the property queried DNE.
        if (buffer != NULL) {
            RtlZeroMemory(buffer, bufferSize);
            
            // all setup, do the ioctl
            ok = DeviceIoControl(DeviceHandle,
                                 IOCTL_STORAGE_QUERY_PROPERTY,
                                 &query,
                                 sizeof(STORAGE_PROPERTY_QUERY),
                                 buffer,
                                 bufferSize,
                                 &returnedData,
                                 FALSE);
            if (!ok) {
                if (GetLastError() == ERROR_MORE_DATA) {
                    // this is ok, we'll ignore it here
                } else if (GetLastError() == ERROR_INVALID_FUNCTION) {
                    // this is also ok, the property DNE
                } else if (GetLastError() == ERROR_NOT_SUPPORTED) {
                    // this is also ok, the property DNE
                } else {
                    // some unexpected error -- exit out
                    goto Cleanup;
                }
                // zero it out, just in case it was partially filled in.
                RtlZeroMemory(buffer, bufferSize);
            }
        }
    } // end i loop

    // adapterDescriptor is now allocated and full of data.
    // deviceDescriptor is now allocated and full of data.
    
    if (adapterDescriptor == NULL) {
        printf("   ***** No adapter descriptor supported on the device *****\n");
    } else {
        PrintAdapterDescriptor(adapterDescriptor);
        *AlignmentMask = adapterDescriptor->AlignmentMask;
    }
    
    if (deviceDescriptor == NULL) {
        printf("   ***** No device descriptor supported on the device  *****\n");
    } else {
        PrintDeviceDescriptor(deviceDescriptor);
    }
    
    failed = FALSE;

Cleanup:
    if (adapterDescriptor != NULL) {
        LocalFree( adapterDescriptor );
    }
    if (deviceDescriptor != NULL) {
        LocalFree( deviceDescriptor );
    }
    return (!failed);

}


