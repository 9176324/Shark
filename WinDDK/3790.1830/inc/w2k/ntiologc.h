/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ntiologc.h

Abstract:

    Constant definitions for the I/O error code log values.

Author:

    Jeff Havens (jhavens) 21-Aug-1991

Revision History:

--*/

#ifndef _NTIOLOGC_
#define _NTIOLOGC_

//
//  Status values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-------------------------+-------------------------------+
//  |Sev|C|       Facility          |               Code            |
//  +---+-+-------------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: IO_ERR_RETRY_SUCCEEDED
//
// MessageText:
//
//  A retry was successful on %1.
//
#define IO_ERR_RETRY_SUCCEEDED           ((NTSTATUS)0x00040001L)

//
// MessageId: IO_ERR_INSUFFICIENT_RESOURCES
//
// MessageText:
//
//  The driver could not allocate something necessary for the request for %1.
//
#define IO_ERR_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0040002L)

//
// MessageId: IO_ERR_CONFIGURATION_ERROR
//
// MessageText:
//
//  Driver or device is incorrectly configured for %1.
//
#define IO_ERR_CONFIGURATION_ERROR       ((NTSTATUS)0xC0040003L)

//
// MessageId: IO_ERR_DRIVER_ERROR
//
// MessageText:
//
//  Driver detected an internal error in its data structures for %1.
//
#define IO_ERR_DRIVER_ERROR              ((NTSTATUS)0xC0040004L)

//
// MessageId: IO_ERR_PARITY
//
// MessageText:
//
//  A parity error was detected on %1.
//
#define IO_ERR_PARITY                    ((NTSTATUS)0xC0040005L)

//
// MessageId: IO_ERR_SEEK_ERROR
//
// MessageText:
//
//  The device, %1, had a seek error.
//
#define IO_ERR_SEEK_ERROR                ((NTSTATUS)0xC0040006L)

//
// MessageId: IO_ERR_BAD_BLOCK
//
// MessageText:
//
//  The device, %1, has a bad block.
//
#define IO_ERR_BAD_BLOCK                 ((NTSTATUS)0xC0040007L)

//
// MessageId: IO_ERR_OVERRUN_ERROR
//
// MessageText:
//
//  An overrun occurred on %1.
//
#define IO_ERR_OVERRUN_ERROR             ((NTSTATUS)0xC0040008L)

//
// MessageId: IO_ERR_TIMEOUT
//
// MessageText:
//
//  The device, %1, did not respond within the timeout period.
//
#define IO_ERR_TIMEOUT                   ((NTSTATUS)0xC0040009L)

//
// MessageId: IO_ERR_SEQUENCE
//
// MessageText:
//
//  The driver detected an unexpected sequence by the device, %1.
//
#define IO_ERR_SEQUENCE                  ((NTSTATUS)0xC004000AL)

//
// MessageId: IO_ERR_CONTROLLER_ERROR
//
// MessageText:
//
//  The driver detected a controller error on %1.
//
#define IO_ERR_CONTROLLER_ERROR          ((NTSTATUS)0xC004000BL)

//
// MessageId: IO_ERR_INTERNAL_ERROR
//
// MessageText:
//
//  The driver detected an internal driver error on %1.
//
#define IO_ERR_INTERNAL_ERROR            ((NTSTATUS)0xC004000CL)

//
// MessageId: IO_ERR_INCORRECT_IRQL
//
// MessageText:
//
//  The driver was configured with an incorrect interrupt for %1.
//
#define IO_ERR_INCORRECT_IRQL            ((NTSTATUS)0xC004000DL)

//
// MessageId: IO_ERR_INVALID_IOBASE
//
// MessageText:
//
//  The driver was configured with an invalid I/O base address for %1.
//
#define IO_ERR_INVALID_IOBASE            ((NTSTATUS)0xC004000EL)

//
// MessageId: IO_ERR_NOT_READY
//
// MessageText:
//
//  The device, %1, is not ready for access yet.
//
#define IO_ERR_NOT_READY                 ((NTSTATUS)0xC004000FL)

//
// MessageId: IO_ERR_INVALID_REQUEST
//
// MessageText:
//
//  The request is incorrectly formatted for %1.
//
#define IO_ERR_INVALID_REQUEST           ((NTSTATUS)0xC0040010L)

//
// MessageId: IO_ERR_VERSION
//
// MessageText:
//
//  The wrong version of the driver has been loaded.
//
#define IO_ERR_VERSION                   ((NTSTATUS)0xC0040011L)

//
// MessageId: IO_ERR_LAYERED_FAILURE
//
// MessageText:
//
//  The driver beneath this one has failed in some way for %1.
//
#define IO_ERR_LAYERED_FAILURE           ((NTSTATUS)0xC0040012L)

//
// MessageId: IO_ERR_RESET
//
// MessageText:
//
//  The device, %1, has been reset.
//
#define IO_ERR_RESET                     ((NTSTATUS)0xC0040013L)

//
// MessageId: IO_ERR_PROTOCOL
//
// MessageText:
//
//  A transport driver received a frame which violated the protocol.
//
#define IO_ERR_PROTOCOL                  ((NTSTATUS)0xC0040014L)

//
// MessageId: IO_ERR_MEMORY_CONFLICT_DETECTED
//
// MessageText:
//
//  A conflict has been detected between two drivers which claimed two overlapping
//  memory regions.
//  Driver %2, with device <%3>, claimed a memory range with starting address
//  in data address 0x28 and 0x2c, and length in data address 0x30.
//
#define IO_ERR_MEMORY_CONFLICT_DETECTED  ((NTSTATUS)0xC0040015L)

//
// MessageId: IO_ERR_PORT_CONFLICT_DETECTED
//
// MessageText:
//
//  A conflict has been detected between two drivers which claimed two overlapping
//  Io port regions.
//  Driver %2, with device <%3>, claimed an IO port range with starting address
//  in data address 0x28 and 0x2c, and length in data address 0x30.
//
#define IO_ERR_PORT_CONFLICT_DETECTED    ((NTSTATUS)0xC0040016L)

//
// MessageId: IO_ERR_DMA_CONFLICT_DETECTED
//
// MessageText:
//
//  A conflict has been detected between two drivers which claimed equivalent DMA
//  channels.
//  Driver %2, with device <%3>, claimed the DMA Channel in data address 0x28, with
//  optinal port in data address 0x2c.
//
#define IO_ERR_DMA_CONFLICT_DETECTED     ((NTSTATUS)0xC0040017L)

//
// MessageId: IO_ERR_IRQ_CONFLICT_DETECTED
//
// MessageText:
//
//  A conflict has been detected between two drivers which claimed equivalent IRQs.
//  Driver %2, with device <%3>, claimed an interrupt with Level in data address
//  0x28, vector in data address 0x2c and Affinity in data address 0x30.
//
#define IO_ERR_IRQ_CONFLICT_DETECTED     ((NTSTATUS)0xC0040018L)

//
// MessageId: IO_ERR_BAD_FIRMWARE
//
// MessageText:
//
//  The driver has detected a device with old or out-of-date firmware.  The
//  device will not be used.
//
#define IO_ERR_BAD_FIRMWARE              ((NTSTATUS)0xC0040019L)

//
// MessageId: IO_WRN_BAD_FIRMWARE
//
// MessageText:
//
//  The driver has detected that device %1 has old or out-of-date firmware.
//  Reduced performance may result.
//
#define IO_WRN_BAD_FIRMWARE              ((NTSTATUS)0x8004001AL)

//
// MessageId: IO_ERR_DMA_RESOURCE_CONFLICT
//
// MessageText:
//
//  The device could not allocate one or more required resources due to conflicts
//  with other devices.  The device DMA setting of '%2' could not be
//  satisified due to a conflict with Driver '%3'.
//
#define IO_ERR_DMA_RESOURCE_CONFLICT     ((NTSTATUS)0xC004001BL)

//
// MessageId: IO_ERR_INTERRUPT_RESOURCE_CONFLICT
//
// MessageText:
//
//  The device could not allocate one or more required resources due to conflicts
//  with other devices.  The device interrupt setting of '%2' could not be
//  satisified due to a conflict with Driver '%3'.
//
#define IO_ERR_INTERRUPT_RESOURCE_CONFLICT ((NTSTATUS)0xC004001CL)

//
// MessageId: IO_ERR_MEMORY_RESOURCE_CONFLICT
//
// MessageText:
//
//  The device could not allocate one or more required resources due to conflicts
//  with other devices.  The device memory setting of '%2' could not be
//  satisified due to a conflict with Driver '%3'.
//
#define IO_ERR_MEMORY_RESOURCE_CONFLICT  ((NTSTATUS)0xC004001DL)

//
// MessageId: IO_ERR_PORT_RESOURCE_CONFLICT
//
// MessageText:
//
//  The device could not allocate one or more required resources due to conflicts
//  with other devices.  The device port setting of '%2' could not be
//  satisified due to a conflict with Driver '%3'.
//
#define IO_ERR_PORT_RESOURCE_CONFLICT    ((NTSTATUS)0xC004001EL)

//
// MessageId: IO_BAD_BLOCK_WITH_NAME
//
// MessageText:
//
//  The file %2 on device %1 contains a bad disk block.
//
#define IO_BAD_BLOCK_WITH_NAME           ((NTSTATUS)0xC004001FL)

//
// MessageId: IO_WRITE_CACHE_ENABLED
//
// MessageText:
//
//  The driver detected that the device %1 has its write cache enabled. Data corruption
//  may occur.
//
#define IO_WRITE_CACHE_ENABLED           ((NTSTATUS)0x80040020L)

//
// MessageId: IO_RECOVERED_VIA_ECC
//
// MessageText:
//
//  Data was recovered using error correction code on device %1.
//
#define IO_RECOVERED_VIA_ECC             ((NTSTATUS)0x80040021L)

//
// MessageId: IO_WRITE_CACHE_DISABLED
//
// MessageText:
//
//  The driver disabled the write cache on device %1.
//
#define IO_WRITE_CACHE_DISABLED          ((NTSTATUS)0x80040022L)

//
// MessageId: IO_FILE_QUOTA_THRESHOLD
//
// MessageText:
//
//  A user hit their quota threshold on volume %2.
//
#define IO_FILE_QUOTA_THRESHOLD          ((NTSTATUS)0x40040024L)

//
// MessageId: IO_FILE_QUOTA_LIMIT
//
// MessageText:
//
//  A user hit their quota limit on volume %2.
//
#define IO_FILE_QUOTA_LIMIT              ((NTSTATUS)0x40040025L)

//
// MessageId: IO_FILE_QUOTA_STARTED
//
// MessageText:
//
//  The system has started rebuilding the user disk quota information on
//  device %1 with label "%2".
//
#define IO_FILE_QUOTA_STARTED            ((NTSTATUS)0x40040026L)

//
// MessageId: IO_FILE_QUOTA_SUCCEEDED
//
// MessageText:
//
//  The system has successfully rebuilt the user disk quota information on
//  device %1 with label "%2".
//
#define IO_FILE_QUOTA_SUCCEEDED          ((NTSTATUS)0x40040027L)

//
// MessageId: IO_FILE_QUOTA_FAILED
//
// MessageText:
//
//  The system has encounted an error rebuilding the user disk quota
//  information on device %1 with label "%2".
//
#define IO_FILE_QUOTA_FAILED             ((NTSTATUS)0x80040028L)

//
// MessageId: IO_FILE_SYSTEM_CORRUPT
//
// MessageText:
//
//  The file system structure on the disk is corrupt and unusable.
//  Please run the chkdsk utility on the device %1 with label "%2".
//
#define IO_FILE_SYSTEM_CORRUPT           ((NTSTATUS)0xC0040029L)

//
// MessageId: IO_FILE_QUOTA_CORRUPT
//
// MessageText:
//
//  The user disk quota information is unusable.
//  To ensure accuracy, the file system quota information on the device %1 with label "%2" will
//  be rebuilt.
//
#define IO_FILE_QUOTA_CORRUPT            ((NTSTATUS)0xC004002AL)

//
// MessageId: IO_SYSTEM_SLEEP_FAILED
//
// MessageText:
//
//  The system sleep operation failed
//
#define IO_SYSTEM_SLEEP_FAILED           ((NTSTATUS)0xC004002BL)

//
// MessageId: IO_DUMP_POINTER_FAILURE
//
// MessageText:
//
//  The system could not get file retrieval pointers for the dump file.
//
#define IO_DUMP_POINTER_FAILURE          ((NTSTATUS)0xC004002CL)

//
// MessageId: IO_DUMP_DRIVER_LOAD_FAILURE
//
// MessageText:
//
//  The system could not sucessfully load the crash dump driver.
//
#define IO_DUMP_DRIVER_LOAD_FAILURE      ((NTSTATUS)0xC004002DL)

//
// MessageId: IO_DUMP_INITIALIZATION_FAILURE
//
// MessageText:
//
//  Crash dump initialization failed!
//
#define IO_DUMP_INITIALIZATION_FAILURE   ((NTSTATUS)0xC004002EL)

//
// MessageId: IO_DUMP_DUMPFILE_CONFLICT
//
// MessageText:
//
//  A valid crash dump was found in the paging file while trying to configure
//  a direct dump. Direct dump is disabled! This occurs when the direct dump
//  option is set in the registry but a stop error occured before configuration
//  completed
//
#define IO_DUMP_DUMPFILE_CONFLICT        ((NTSTATUS)0xC004002FL)

//
// MessageId: IO_DUMP_DIRECT_CONFIG_FAILED
//
// MessageText:
//
//  Direct dump configuration failed. Validate the filename and make sure the target device
//  is not a Fault Tolerant set member, remote, or floppy device. The failure may
//  be because there is not enough room on the dump device to create the dump file.
//
#define IO_DUMP_DIRECT_CONFIG_FAILED     ((NTSTATUS)0xC0040030L)

//
// MessageId: IO_DUMP_PAGE_CONFIG_FAILED
//
// MessageText:
//
//  Configuring the Page file for crash dump failed. Make sure there is a page
//  file on the boot partition and that is large enough to contain all physical
//  memory.
//
#define IO_DUMP_PAGE_CONFIG_FAILED       ((NTSTATUS)0xC0040031L)

//
// MessageId: IO_LOST_DELAYED_WRITE
//
// MessageText:
//
//  {Lost Delayed-Write Data}
//  The system was attempting to transfer file data from buffers to %1.
//  The write operation failed, and only some of the data may have been
//  written to the file.
//
#define IO_LOST_DELAYED_WRITE            ((NTSTATUS)0x80040032L)

//
// MessageId: IO_WARNING_PAGING_FAILURE
//
// MessageText:
//
//  An error was detected on device %1 during a paging operation.
//
#define IO_WARNING_PAGING_FAILURE        ((NTSTATUS)0x80040033L)

//
// MessageId: IO_WRN_FAILURE_PREDICTED
//
// MessageText:
//
//  The driver has detected that device %1 has predicted that it will fail. 
//  Immediately back up your data and replace your hard disk drive. A failure 
//  may be imminent.
//
#define IO_WRN_FAILURE_PREDICTED         ((NTSTATUS)0x80040034L)

//
// MessageId: IO_WARNING_INTERRUPT_STILL_PENDING
//
// MessageText:
//
//  A pending interrupt was detected on device %1 during a timeout operation.  A
//  large number of these warnings may indicate that the system is not correctly 
//  receiving or processing interrupts from the device.
//
#define IO_WARNING_INTERRUPT_STILL_PENDING ((NTSTATUS)0x80040035L)

//
// MessageId: IO_DRIVER_CANCEL_TIMEOUT
//
// MessageText:
//
//  An Io Request to the device %1 did not complete or canceled within the
//  specific timeout. This can occur if the device driver does not set a 
//  cancel routine for a given IO request packet.
//
#define IO_DRIVER_CANCEL_TIMEOUT         ((NTSTATUS)0x80040036L)

//
// MessageId: IO_FILE_SYSTEM_CORRUPT_WITH_NAME
//
// MessageText:
//
//  The file system structure on the disk is corrupt and unusable.
//  Please run the chkdsk utility on the volume %2.
//
#define IO_FILE_SYSTEM_CORRUPT_WITH_NAME ((NTSTATUS)0xC0040037L)

#endif /* _NTIOLOGC_ */

