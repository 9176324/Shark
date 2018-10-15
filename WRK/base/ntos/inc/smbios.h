/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    smbios.h

Abstract:

    This module contains definitions that describe SMBIOS

--*/

#ifndef _SMBIOS_
#define _SMBIOS_

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)

//
// SMBIOS error codes
#define DMI_SUCCESS 0x00
#define DMI_UNKNOWN_FUNCTION 0x81
#define DMI_FUNCTION_NOT_SUPPORTED 0x82
#define DMI_INVALID_HANDLE 0x83
#define DMI_BAD_PARAMETER 0x84
#define DMI_INVALID_SUBFUNCTION 0x85
#define DMI_NO_CHANGE 0x86
#define DMI_ADD_STRUCTURE_FAILED 0x87

// @@BEGIN_DDKSPLIT

//
// SMBIOS registry values
//
#define SMBIOSPARENTKEYNAME L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter"
#define SMBIOSGLOBALKEYNAME L"\\Registry\\Machine\\Hardware\\Description\\System"

#define SMBIOSIDENTIFIERVALUENAME L"Identifier"
#define SMBIOSIDENTIFIERVALUEDATA L"PNP BIOS"
#define SMBIOSDATAVALUENAME     L"Configuration Data"

#define MAXSMBIOSKEYNAMESIZE 256

// @@END_DDKSPLIT

//
// SMBIOS table search
#define SMBIOS_EPS_SEARCH_SIZE      0x10000
#define SMBIOS_EPS_SEARCH_START     0x000f0000
#define SMBIOS_EPS_SEARCH_INCREMENT 0x10

#include <pshpack1.h>
typedef struct _SMBIOS_TABLE_HEADER
{
    UCHAR Signature[4];             // _SM_ (ascii)
    UCHAR Checksum;
    UCHAR Length;
    UCHAR MajorVersion;
    UCHAR MinorVersion;
    USHORT MaximumStructureSize;
    UCHAR EntryPointRevision;
    UCHAR Reserved[5];
    UCHAR Signature2[5];           // _DMI_ (ascii)
    UCHAR IntermediateChecksum;
    USHORT StructureTableLength;
    ULONG StructureTableAddress;
    USHORT NumberStructures;
    UCHAR Revision;
} SMBIOS_EPS_HEADER, *PSMBIOS_EPS_HEADER;

#define SMBIOS_EPS_SIGNATURE '_MS_'
#define DMI_EPS_SIGNATURE    'IMD_'

typedef struct _SMBIOS_STRUCT_HEADER
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;
    UCHAR Data[];
} SMBIOS_STRUCT_HEADER, *PSMBIOS_STRUCT_HEADER;


typedef struct _DMIBIOS_TABLE_HEADER
{
    UCHAR Signature2[5];           // _DMI_ (ascii)
    UCHAR IntermediateChecksum;
    USHORT StructureTableLength;
    ULONG StructureTableAddress;
    USHORT NumberStructures;
    UCHAR Revision;
} DMIBIOS_EPS_HEADER, *PDMIBIOS_EPS_HEADER;


//
// Definitions for the SMBIOS table BIOS INFORMATION
//
#define SMBIOS_BIOS_INFORMATION_TYPE 0
typedef struct _SMBIOS_BIOS_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Vendor;
    UCHAR       Version;
    USHORT      StartingAddressSegment;
    UCHAR       ReleaseDate;
    UCHAR       RomSize;
    UCHAR       Characteristics[8];
    UCHAR       CharacteristicsExtension0;
    UCHAR       CharacteristicsExtension1;
    UCHAR       SystemBiosMajorRelease;     // SMBIOS 2.3.6+
    UCHAR       SystemBiosMinorRelease;     // SMBIOS 2.3.6+
    UCHAR       ECFirmwareMajorRelease;     // SMBIOS 2.3.6+
    UCHAR       ECFirmwareMinorRelease;     // SMBIOS 2.3.6+
} SMBIOS_BIOS_INFORMATION_STRUCT, *PSMBIOS_BIOS_INFORMATION_STRUCT;

//
// If the Minor or Major Release are equal to this, then the system does not
// support the use of this field
//
// SMBIOS 2.3.6+
//
#define SMBIOS_BIOS_UNSUPPORTED_FIRMWARE_REVISION   0xFF
#define SMBIOS_BIOS_TARGETTED_CONTENT_ENABLED(X)    ((X & 0x4) != 0)

//
// Definitions for the SMBIOS table SYSTEM INFORMATION STRUCTURE
//
#define SMBIOS_SYSTEM_INFORMATION    1
typedef struct _SMBIOS_SYSTEM_INFORMATION_STRUCT
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;
    
    UCHAR Manufacturer;     // string
    UCHAR ProductName;      // string
    UCHAR Version;          // string
    UCHAR SerialNumber;     // string
    UCHAR Uuid[16];         // SMBIOS 2.1+
    UCHAR WakeupType;       // SMBIOS 2.1+
    UCHAR SKUNumber;        // SMBIOS 2.3.6+
    UCHAR Family;           // SMBIOS 2.3.6+
} SMBIOS_SYSTEM_INFORMATION_STRUCT, *PSMBIOS_SYSTEM_INFORMATION_STRUCT;

#define SMBIOS_SYSTEM_INFORMATION_LENGTH_20 8



//
// Definitions for the SMBIOS table BASE BOARD INFORMATION
//
#define SMBIOS_BASE_BOARD_INFORMATION_TYPE 2
typedef struct _SMBIOS_BASE_BOARD_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Manufacturer;
    UCHAR       Product;
    UCHAR       Version;
    UCHAR       SerialNumber;
    UCHAR       AssetTagNumber;
    UCHAR       FeatureFlags;
    UCHAR       Location;
    USHORT      ChassisHandle;
    UCHAR       BoardType;
    UCHAR       ObjectHandles;
} SMBIOS_BASE_BOARD_INFORMATION_STRUCT, *PSMBIOS_BASE_BOARD_INFORMATION_STRUCT;



//
// Definitions for the SMBIOS table BASE BOARD INFORMATION
//
#define SMBIOS_SYSTEM_CHASIS_INFORMATION_TYPE 3
typedef struct _SMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Manufacturer;
    UCHAR       ChasisType;
    UCHAR       Version;
    UCHAR       SerialNumber;
    UCHAR       AssetTagNumber;
    UCHAR       BootUpState;
    UCHAR       PowerSupplyState;
    UCHAR       ThernalState;
    UCHAR       SecurityStatus;
    ULONG       OEMDefined;
} SMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT, *PSMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT;


//
// Definitions for the SMBIOS table PROCESSOR INFORMATION
//
#define SMBIOS_PROCESSOR_INFORMATION_TYPE 4
typedef struct _SMBIOS_PROCESSOR_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       SocketDesignation;
    UCHAR       ProcessorType;
    UCHAR       ProcessorFamily;
    UCHAR       ProcessorManufacturer;
    ULONG       ProcessorID0;
    ULONG       ProcessorID1;
    UCHAR       ProcessorVersion;
    UCHAR       Voltage;
    USHORT      ExternalClock;
    USHORT      MaxSpeed;
    USHORT      CurrentSpeed;
    UCHAR       Status;
    UCHAR       ProcessorUpgrade;
    USHORT      L1CacheHandle;
    USHORT      L2CacheHandle;
    USHORT      L3CacheHandle;
    UCHAR       SerialNumber;
    UCHAR       AssetTagNumber;
} SMBIOS_PROCESSOR_INFORMATION_STRUCT, *PSMBIOS_PROCESSOR_INFORMATION_STRUCT;




//
// Definitions for the SMBIOS table SYSTEM EVENTLOG STRUCTURE
#define SMBIOS_SYSTEM_EVENTLOG 15

//
// ENUM for AccessMethod
//
#define ACCESS_METHOD_INDEXIO_1     0
#define ACCESS_METHOD_INDEXIO_2     1
#define ACCESS_METHOD_INDEXIO_3     2
#define ACCESS_METHOD_MEMMAP        3
#define ACCESS_METHOD_GPNV          4

typedef struct _LOGTYPEDESCRIPTOR
{
    UCHAR LogType;
    UCHAR DataFormatType;
} LOGTYPEDESCRIPTOR, *PLOGTYPEDESCRIPTOR;

typedef struct _ACCESS_METHOD_ADDRESS
{
    union
    {
        struct
        {
            USHORT IndexAddr;
            USHORT DataAddr;            
        } IndexIo;
        
        ULONG PhysicalAddress32;
        
        USHORT GPNVHandle;      
    } AccessMethodAddress;
} ACCESS_METHOD_ADDRESS, *PACCESS_METHOD_ADDRESS;

typedef struct _SMBIOS_SYSTEM_EVENTLOG_STRUCT
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;

    USHORT LogAreaLength;
    USHORT LogHeaderStartOffset;
    USHORT LogDataStartOffset;
    UCHAR AccessMethod;
    UCHAR LogStatus;
    ULONG LogChangeToken;
    ACCESS_METHOD_ADDRESS AccessMethodAddress;
    UCHAR LogHeaderFormat;
    UCHAR NumLogTypeDescriptors;
    UCHAR LenLogTypeDescriptors;
    LOGTYPEDESCRIPTOR LogTypeDescriptor[1];
    
} SMBIOS_SYSTEM_EVENTLOG_STRUCT, *PSMBIOS_SYSTEM_EVENTLOG_STRUCT;

#define SMBIOS_SYSTEM_EVENTLOG_LENGTH_20 0x14
#define SMBIOS_SYSTEM_EVENTLOG_LENGTH (FIELD_OFFSET(SMBIOS_SYSTEM_EVENTLOG_STRUCT, LogTypeDescriptor))

#define SMBIOS_MEMORY_DEVICE_TYPE   17
typedef struct _SMBIOS_MEMORY_DEVICE_STRUCT
{
    UCHAR	Type;
    UCHAR	Length;
    USHORT	Handle;
    USHORT	MemArrayHandle;
    USHORT	MemErrorInfoHandle;
    USHORT	TotalWidth;
    USHORT	DataWidth;
    USHORT	Size;
    UCHAR	FormFactor;
    UCHAR	DeviceSet;
    UCHAR	DeviceLocator;
    UCHAR	BankLocator;
    UCHAR	MemoryType;
    USHORT	TypeDetail;
    USHORT	Speed;
    UCHAR   Manufacturer;
    UCHAR   SerialNumber;
    UCHAR   AssetTagNumber;
    UCHAR   PartNumber;
} SMBIOS_MEMORY_DEVICE_STRUCT, *PSMBIOS_MEMORY_DEVICE_STRUCT;

#define SMBIOS_PORTABLE_BATTERY_TYPE    22
typedef struct _SMBIOS_PORTABLE_BATTERY_STRUCT
{
    UCHAR   Type;
    UCHAR   Length;
    USHORT  Handle;

    UCHAR   Location;           // String Index
    UCHAR   Manufacturer;       // String Index
    UCHAR   ManufactureDate;    // String Index
    UCHAR   SerialNumber;       // String Index
    UCHAR   DeviceName;         // String Index
    UCHAR   DeviceChemistry;    // Enum - See 3.3.23.1 of SMBIOS 2.3.3 spec
    USHORT  DesignCapacity;     // mWatt/hours
    USHORT  DeviceVoltage;      // mVolts
    UCHAR   SBDSVersionNumber;  // String Index
    UCHAR   MaximumError;       // Percentage
    USHORT  SBDSSerialNumber;   
    USHORT  SBDSManufacturerDate;// Packed format. See 3.3.23 of SMBIOS 2.3.3 spec
    UCHAR   SBDSDeviceChemistry;// String Index
    UCHAR   DeviceCapacityMult; // DesignCapacity Multiplier value
    ULONG   OEMSpecific;        // OEM Specific Value
} SMBIOS_PORTABLE_BATTERY_STRUCT, *PSMBIOS_PORTABLE_BATTERY_STRUCT;

#define SMBIOS_SYSTEM_POWER_SUPPLY_TYPE 39
typedef struct _SMBIOS_SYSTEM_POWER_SUPPLY_STRUCT
{
    UCHAR   Type;
    UCHAR   Length;
    USHORT  Handle;

    UCHAR   PowerUnitGroup;
    UCHAR   Location;                   // String Index
    UCHAR   DeviceName;                 // String Index
    UCHAR   Manufacturer;               // String Index
    UCHAR   SerialNumber;               // String Index
    UCHAR   AssetTagNumber;             // String Index
    UCHAR   ModelPartNumber;            // String Index
    UCHAR   RevisionLevel;              // String Index
    USHORT  MaximumPowerCapacity;       // Watts, 0x8000 if unknown
    USHORT  PowerSupplyCharacteristics; // See 3.3.40.1 of SMBIOS 2.3.3 spec
    USHORT  InputVoltageProbeHandle;
    USHORT  CoolingDeviceHandle;
    USHORT  InputCurrentProbleHandle;
} SMBIOS_SYSTEM_POWER_SUPPLY_STRUCT, *PSMBIOS_SYSTEM_POWER_SUPPLY_STRUCT;

//
// SYSID table search
//

#define SYSID_EPS_SEARCH_SIZE      0x20000
#define SYSID_EPS_SEARCH_START     0x000e0000
#define SYSID_EPS_SEARCH_INCREMENT 0x10

#define SYSID_EPS_SIGNATURE 'SYS_'
#define SYSID_EPS_SIGNATURE2 'DI'

typedef struct _SYSID_EPS_HEADER
{
    UCHAR Signature[7];           // _SYSID_ (ascii)
    UCHAR Checksum;
    USHORT Length;                // Length of SYSID_EPS_HEADER
    ULONG SysIdTableAddress;      // Physical Address of SYSID table
    USHORT SysIdCount;            // Count of SYSIDs in table
    UCHAR BiosRev;                // SYSID Bios revision
} SYSID_EPS_HEADER, *PSYSID_EPS_HEADER;

typedef struct _SYSID_TABLE_ENTRY
{
    UCHAR Type[6];                // _UUID_ or _1394_ (ascii)
    UCHAR Checksum;
    USHORT Length;                // Length of this table
    UCHAR Data[1];                // Variable length UUID/1394 data
} SYSID_TABLE_ENTRY, *PSYSID_TABLE_ENTRY;

#define SYSID_UUID_DATA_SIZE 16

typedef struct _SYSID_UUID_ENTRY
{
    UCHAR Type[6];                // _UUID_ (ascii)
    UCHAR Checksum;
    USHORT Length;                // Length of this table
    UCHAR UUID[SYSID_UUID_DATA_SIZE];  // UUID
} SYSID_UUID_ENTRY, *PSYSID_UUID_ENTRY;

#define SYSID_1394_DATA_SIZE 8

typedef struct _SYSID_1394_ENTRY
{
    UCHAR Type[6];                // _1394_ (ascii)
    UCHAR Checksum;
    USHORT Length;                // Length of this table
    UCHAR x1394Id[SYSID_1394_DATA_SIZE]; // 1394 ID
} SYSID_1394_ENTRY, *PSYSID_1394_ENTRY;

#define LARGEST_SYSID_TABLE_ENTRY (sizeof(SYSID_UUID_ENTRY))

#define SYSID_TYPE_UUID "_UUID_"
#define SYSID_TYPE_1394 "_1394_"
                                    
#include <poppack.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
#endif
