#ifndef _hbapiwmi_h_
#define _hbapiwmi_h_

// MSFC_HBAPortStatistics - MSFC_HBAPortStatistics


//***************************************************************************
//
//  hbapiwmi.h
// 
//  Module: WDM classes to expose HBA api data from drivers
//
//  Purpose: Contains WDM classes that specify the HBA data to be exposed 
//           via the HBA api set.
//
//  NOTE: This file contains information that is based upon an earlier
//        revision of the HBAAPI 2.0 specification. Since the
//        specification has not yet been approved, the information may
//        change in the next release of this file.
//
//  Copyright (c) 2001 Microsoft Corporation
//
//***************************************************************************


#define MSFC_HBAPortStatisticsGuid \
    { 0x3ce7904f,0x459f,0x480d, { 0x9a,0x3c,0x01,0x3e,0xde,0x3b,0xdd,0xe8 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_HBAPortStatistics_GUID, \
            0x3ce7904f,0x459f,0x480d,0x9a,0x3c,0x01,0x3e,0xde,0x3b,0xdd,0xe8);
#endif


typedef struct _MSFC_HBAPortStatistics
{
    // 
    LONGLONG SecondsSinceLastReset;
    #define MSFC_HBAPortStatistics_SecondsSinceLastReset_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_SecondsSinceLastReset_ID 1

    // 
    LONGLONG TxFrames;
    #define MSFC_HBAPortStatistics_TxFrames_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_TxFrames_ID 2

    // 
    LONGLONG TxWords;
    #define MSFC_HBAPortStatistics_TxWords_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_TxWords_ID 3

    // 
    LONGLONG RxFrames;
    #define MSFC_HBAPortStatistics_RxFrames_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_RxFrames_ID 4

    // 
    LONGLONG RxWords;
    #define MSFC_HBAPortStatistics_RxWords_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_RxWords_ID 5

    // 
    LONGLONG LIPCount;
    #define MSFC_HBAPortStatistics_LIPCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_LIPCount_ID 6

    // 
    LONGLONG NOSCount;
    #define MSFC_HBAPortStatistics_NOSCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_NOSCount_ID 7

    // 
    LONGLONG ErrorFrames;
    #define MSFC_HBAPortStatistics_ErrorFrames_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_ErrorFrames_ID 8

    // 
    LONGLONG DumpedFrames;
    #define MSFC_HBAPortStatistics_DumpedFrames_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_DumpedFrames_ID 9

    // 
    LONGLONG LinkFailureCount;
    #define MSFC_HBAPortStatistics_LinkFailureCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_LinkFailureCount_ID 10

    // 
    LONGLONG LossOfSyncCount;
    #define MSFC_HBAPortStatistics_LossOfSyncCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_LossOfSyncCount_ID 11

    // 
    LONGLONG LossOfSignalCount;
    #define MSFC_HBAPortStatistics_LossOfSignalCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_LossOfSignalCount_ID 12

    // 
    LONGLONG PrimitiveSeqProtocolErrCount;
    #define MSFC_HBAPortStatistics_PrimitiveSeqProtocolErrCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_PrimitiveSeqProtocolErrCount_ID 13

    // 
    LONGLONG InvalidTxWordCount;
    #define MSFC_HBAPortStatistics_InvalidTxWordCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_InvalidTxWordCount_ID 14

    // 
    LONGLONG InvalidCRCCount;
    #define MSFC_HBAPortStatistics_InvalidCRCCount_SIZE sizeof(LONGLONG)
    #define MSFC_HBAPortStatistics_InvalidCRCCount_ID 15

} MSFC_HBAPortStatistics, *PMSFC_HBAPortStatistics;

#define MSFC_HBAPortStatistics_SIZE (FIELD_OFFSET(MSFC_HBAPortStatistics, InvalidCRCCount) + MSFC_HBAPortStatistics_InvalidCRCCount_SIZE)

// MSFC_FibrePortHBAStatistics - MSFC_FibrePortHBAStatistics
// This class exposes statistical information associated with a Fibre Channel port. There should be one instance of this class for each port.
#define MSFC_FibrePortHBAStatisticsGuid \
    { 0x27efaba4,0x362a,0x4f20, { 0x92,0x0b,0xed,0x66,0xe2,0x80,0xfc,0xf5 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_FibrePortHBAStatistics_GUID, \
            0x27efaba4,0x362a,0x4f20,0x92,0x0b,0xed,0x66,0xe2,0x80,0xfc,0xf5);
#endif


typedef struct _MSFC_FibrePortHBAStatistics
{
    // Unique identifier for the port. This idenitifer must be unique among all ports on all adapters. The same value for the identifier must be used for the same port in other classes that expose port information
    ULONGLONG UniquePortId;
    #define MSFC_FibrePortHBAStatistics_UniquePortId_SIZE sizeof(ULONGLONG)
    #define MSFC_FibrePortHBAStatistics_UniquePortId_ID 1

    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define MSFC_FibrePortHBAStatistics_HBAStatus_SIZE sizeof(ULONG)
    #define MSFC_FibrePortHBAStatistics_HBAStatus_ID 2

    // 
    MSFC_HBAPortStatistics Statistics;
    #define MSFC_FibrePortHBAStatistics_Statistics_SIZE sizeof(MSFC_HBAPortStatistics)
    #define MSFC_FibrePortHBAStatistics_Statistics_ID 3

} MSFC_FibrePortHBAStatistics, *PMSFC_FibrePortHBAStatistics;

#define MSFC_FibrePortHBAStatistics_SIZE (FIELD_OFFSET(MSFC_FibrePortHBAStatistics, Statistics) + MSFC_FibrePortHBAStatistics_Statistics_SIZE)

// MSFC_HBAPortAttributesResults - MSFC_HBAPortAttributesResults
#define MSFC_HBAPortAttributesResultsGuid \
    { 0xa76bd4e3,0x9961,0x4d9b, { 0xb6,0xbe,0x86,0xe6,0x98,0x26,0x0f,0x68 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_HBAPortAttributesResults_GUID, \
            0xa76bd4e3,0x9961,0x4d9b,0xb6,0xbe,0x86,0xe6,0x98,0x26,0x0f,0x68);
#endif


typedef struct _MSFC_HBAPortAttributesResults
{
    // 
    UCHAR NodeWWN[8];
    #define MSFC_HBAPortAttributesResults_NodeWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_HBAPortAttributesResults_NodeWWN_ID 1

    // 
    UCHAR PortWWN[8];
    #define MSFC_HBAPortAttributesResults_PortWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_HBAPortAttributesResults_PortWWN_ID 2

    // 
    ULONG PortFcId;
    #define MSFC_HBAPortAttributesResults_PortFcId_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortFcId_ID 3

    // 
    ULONG PortType;
    #define MSFC_HBAPortAttributesResults_PortType_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortType_ID 4

    // 
    ULONG PortState;
    #define MSFC_HBAPortAttributesResults_PortState_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortState_ID 5

    // 
    ULONG PortSupportedClassofService;
    #define MSFC_HBAPortAttributesResults_PortSupportedClassofService_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortSupportedClassofService_ID 6

    // 
    UCHAR PortSupportedFc4Types[32];
    #define MSFC_HBAPortAttributesResults_PortSupportedFc4Types_SIZE sizeof(UCHAR[32])
    #define MSFC_HBAPortAttributesResults_PortSupportedFc4Types_ID 7

    // 
    UCHAR PortActiveFc4Types[32];
    #define MSFC_HBAPortAttributesResults_PortActiveFc4Types_SIZE sizeof(UCHAR[32])
    #define MSFC_HBAPortAttributesResults_PortActiveFc4Types_ID 8

    // 
    ULONG PortSupportedSpeed;
    #define MSFC_HBAPortAttributesResults_PortSupportedSpeed_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortSupportedSpeed_ID 9

    // 
    ULONG PortSpeed;
    #define MSFC_HBAPortAttributesResults_PortSpeed_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortSpeed_ID 10

    // 
    ULONG PortMaxFrameSize;
    #define MSFC_HBAPortAttributesResults_PortMaxFrameSize_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_PortMaxFrameSize_ID 11

    // 
    UCHAR FabricName[8];
    #define MSFC_HBAPortAttributesResults_FabricName_SIZE sizeof(UCHAR[8])
    #define MSFC_HBAPortAttributesResults_FabricName_ID 12

    // 
    ULONG NumberofDiscoveredPorts;
    #define MSFC_HBAPortAttributesResults_NumberofDiscoveredPorts_SIZE sizeof(ULONG)
    #define MSFC_HBAPortAttributesResults_NumberofDiscoveredPorts_ID 13

} MSFC_HBAPortAttributesResults, *PMSFC_HBAPortAttributesResults;

#define MSFC_HBAPortAttributesResults_SIZE (FIELD_OFFSET(MSFC_HBAPortAttributesResults, NumberofDiscoveredPorts) + MSFC_HBAPortAttributesResults_NumberofDiscoveredPorts_SIZE)

// MSFC_FibrePortHBAAttributes - MSFC_FibrePortHBAAttributes
// This class exposes attribute information associated with a Fibre Channel port. There should be one instance of this class for each port.
#define MSFC_FibrePortHBAAttributesGuid \
    { 0x61b397fd,0xf5ae,0x4950, { 0x97,0x58,0x0e,0xe5,0x98,0xe3,0xc6,0xe6 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_FibrePortHBAAttributes_GUID, \
            0x61b397fd,0xf5ae,0x4950,0x97,0x58,0x0e,0xe5,0x98,0xe3,0xc6,0xe6);
#endif


typedef struct _MSFC_FibrePortHBAAttributes
{
    // Unique identifier for the port. This idenitifer must be unique among all ports on all adapters. The same value for the identifier must be used for the same port in other classes that expose port information
    ULONGLONG UniquePortId;
    #define MSFC_FibrePortHBAAttributes_UniquePortId_SIZE sizeof(ULONGLONG)
    #define MSFC_FibrePortHBAAttributes_UniquePortId_ID 1

    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define MSFC_FibrePortHBAAttributes_HBAStatus_SIZE sizeof(ULONG)
    #define MSFC_FibrePortHBAAttributes_HBAStatus_ID 2

    // 
    MSFC_HBAPortAttributesResults Attributes;
    #define MSFC_FibrePortHBAAttributes_Attributes_SIZE sizeof(MSFC_HBAPortAttributesResults)
    #define MSFC_FibrePortHBAAttributes_Attributes_ID 3

} MSFC_FibrePortHBAAttributes, *PMSFC_FibrePortHBAAttributes;

#define MSFC_FibrePortHBAAttributes_SIZE (FIELD_OFFSET(MSFC_FibrePortHBAAttributes, Attributes) + MSFC_FibrePortHBAAttributes_Attributes_SIZE)

// MSFC_FibrePortHBAMethods - MSFC_FibrePortHBAMethods
// This class exposes operations that can be performed on a Fibre Channel port. There should be one instance of this class for each port.
#define MSFC_FibrePortHBAMethodsGuid \
    { 0xe693553e,0xedf6,0x4d57, { 0xbf,0x08,0xef,0xca,0xae,0x1a,0x2e,0x1c } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_FibrePortHBAMethods_GUID, \
            0xe693553e,0xedf6,0x4d57,0xbf,0x08,0xef,0xca,0xae,0x1a,0x2e,0x1c);
#endif

//
// Method id definitions for MSFC_FibrePortHBAMethods
#define ResetStatistics     1

// MSFC_FCAdapterHBAAttributes - MSFC_FCAdapterHBAAttributes
// This class exposes attribute information associated with a fibre channel adapter. There should be one instance of this class for each adapter
#define MSFC_FCAdapterHBAAttributesGuid \
    { 0xf8f3ea26,0xab2c,0x4593, { 0x8b,0x84,0xc5,0x64,0x28,0xe6,0xbe,0xdb } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_FCAdapterHBAAttributes_GUID, \
            0xf8f3ea26,0xab2c,0x4593,0x8b,0x84,0xc5,0x64,0x28,0xe6,0xbe,0xdb);
#endif


typedef struct _MSFC_FCAdapterHBAAttributes
{
    // Unique identifier for the adapter. This idenitifer must be unique among all adapters. The same value for the identifier must be used for the same adapter in other classes that expose adapter information
    ULONGLONG UniqueAdapterId;
    #define MSFC_FCAdapterHBAAttributes_UniqueAdapterId_SIZE sizeof(ULONGLONG)
    #define MSFC_FCAdapterHBAAttributes_UniqueAdapterId_ID 1

    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define MSFC_FCAdapterHBAAttributes_HBAStatus_SIZE sizeof(ULONG)
    #define MSFC_FCAdapterHBAAttributes_HBAStatus_ID 2

    // 
    UCHAR NodeWWN[8];
    #define MSFC_FCAdapterHBAAttributes_NodeWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_FCAdapterHBAAttributes_NodeWWN_ID 3

    // 
    ULONG VendorSpecificID;
    #define MSFC_FCAdapterHBAAttributes_VendorSpecificID_SIZE sizeof(ULONG)
    #define MSFC_FCAdapterHBAAttributes_VendorSpecificID_ID 4

    // 
    ULONG NumberOfPorts;
    #define MSFC_FCAdapterHBAAttributes_NumberOfPorts_SIZE sizeof(ULONG)
    #define MSFC_FCAdapterHBAAttributes_NumberOfPorts_ID 5



   //******************************************************************
   //
   //  The string type is variable length (up to MaxLen).              
   //  Each string starts with a ushort that holds the strings length  
   //  (in bytes) followed by the WCHARs that make up the string.      
   //
   //  This struct is packed so the offset for SerialNumber must be    
   //  computed from the offset of Manufacturer.                       
   //   i.e. SerialNumber = Manufacturer+sizeof(USHORT)+Manufacturer[0]
   //
   //  similarly Model's offset must be computed from SerialNumber etc.
   //
   //******************************************************************


    // 
    WCHAR Manufacturer[64 + 1];
    #define MSFC_FCAdapterHBAAttributes_Manufacturer_ID 6

    // 
    WCHAR SerialNumber[64 + 1];
    #define MSFC_FCAdapterHBAAttributes_SerialNumber_ID 7

    // 
    WCHAR Model[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_Model_ID 8

    // 
    WCHAR ModelDescription[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_ModelDescription_ID 9

    // 
    WCHAR NodeSymbolicName[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_NodeSymbolicName_ID 10

    // 
    WCHAR HardwareVersion[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_HardwareVersion_ID 11

    // 
    WCHAR DriverVersion[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_DriverVersion_ID 12

    // 
    WCHAR OptionROMVersion[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_OptionROMVersion_ID 13

    // 
    WCHAR FirmwareVersion[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_FirmwareVersion_ID 14

    // 
    WCHAR DriverName[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_DriverName_ID 15

    // 
    WCHAR MfgDomain[256 + 1];
    #define MSFC_FCAdapterHBAAttributes_MfgDomain_ID 16

} MSFC_FCAdapterHBAAttributes, *PMSFC_FCAdapterHBAAttributes;

// HBAFC3MgmtInfo - HBAFC3MgmtInfo
// This class exposes FC3 Management information associated with a fibre channel adapter. There should be one instance of this class for each adapter
#define HBAFC3MgmtInfoGuid \
    { 0x5966a24f,0x6aa5,0x418e, { 0xb7,0x5c,0x2f,0x21,0x4d,0xfb,0x4b,0x18 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAFC3MgmtInfo_GUID, \
            0x5966a24f,0x6aa5,0x418e,0xb7,0x5c,0x2f,0x21,0x4d,0xfb,0x4b,0x18);
#endif


typedef struct _HBAFC3MgmtInfo
{
    // Unique identifier for the adapter. This idenitifer must be unique among all adapters. The same value for the identifier must be used for the same adapter in other classes that expose adapter information
    ULONGLONG UniqueAdapterId;
    #define HBAFC3MgmtInfo_UniqueAdapterId_SIZE sizeof(ULONGLONG)
    #define HBAFC3MgmtInfo_UniqueAdapterId_ID 1

    // 
    UCHAR wwn[8];
    #define HBAFC3MgmtInfo_wwn_SIZE sizeof(UCHAR[8])
    #define HBAFC3MgmtInfo_wwn_ID 2

    // 
    ULONG unittype;
    #define HBAFC3MgmtInfo_unittype_SIZE sizeof(ULONG)
    #define HBAFC3MgmtInfo_unittype_ID 3

    // 
    ULONG PortId;
    #define HBAFC3MgmtInfo_PortId_SIZE sizeof(ULONG)
    #define HBAFC3MgmtInfo_PortId_ID 4

    // 
    ULONG NumberOfAttachedNodes;
    #define HBAFC3MgmtInfo_NumberOfAttachedNodes_SIZE sizeof(ULONG)
    #define HBAFC3MgmtInfo_NumberOfAttachedNodes_ID 5

    // 
    USHORT IPVersion;
    #define HBAFC3MgmtInfo_IPVersion_SIZE sizeof(USHORT)
    #define HBAFC3MgmtInfo_IPVersion_ID 6

    // 
    USHORT UDPPort;
    #define HBAFC3MgmtInfo_UDPPort_SIZE sizeof(USHORT)
    #define HBAFC3MgmtInfo_UDPPort_ID 7

    // 
    UCHAR IPAddress[16];
    #define HBAFC3MgmtInfo_IPAddress_SIZE sizeof(UCHAR[16])
    #define HBAFC3MgmtInfo_IPAddress_ID 8

    // 
    USHORT reserved;
    #define HBAFC3MgmtInfo_reserved_SIZE sizeof(USHORT)
    #define HBAFC3MgmtInfo_reserved_ID 9

    // 
    USHORT TopologyDiscoveryFlags;
    #define HBAFC3MgmtInfo_TopologyDiscoveryFlags_SIZE sizeof(USHORT)
    #define HBAFC3MgmtInfo_TopologyDiscoveryFlags_ID 10

    // 
    ULONG reserved1;
    #define HBAFC3MgmtInfo_reserved1_SIZE sizeof(ULONG)
    #define HBAFC3MgmtInfo_reserved1_ID 11

} HBAFC3MgmtInfo, *PHBAFC3MgmtInfo;

#define HBAFC3MgmtInfo_SIZE (FIELD_OFFSET(HBAFC3MgmtInfo, reserved1) + HBAFC3MgmtInfo_reserved1_SIZE)

// HBAScsiID - HBAScsiID
#define HBAScsiIDGuid \
    { 0xa76f5058,0xb1f0,0x4622, { 0x9e,0x88,0x5c,0xc4,0x1e,0x34,0x45,0x4a } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAScsiID_GUID, \
            0xa76f5058,0xb1f0,0x4622,0x9e,0x88,0x5c,0xc4,0x1e,0x34,0x45,0x4a);
#endif


typedef struct _HBAScsiID
{
    // 
    ULONG ScsiBusNumber;
    #define HBAScsiID_ScsiBusNumber_SIZE sizeof(ULONG)
    #define HBAScsiID_ScsiBusNumber_ID 1

    // 
    ULONG ScsiTargetNumber;
    #define HBAScsiID_ScsiTargetNumber_SIZE sizeof(ULONG)
    #define HBAScsiID_ScsiTargetNumber_ID 2

    // 
    ULONG ScsiOSLun;
    #define HBAScsiID_ScsiOSLun_SIZE sizeof(ULONG)
    #define HBAScsiID_ScsiOSLun_ID 3



   //******************************************************************
   //
   //  The string type is variable length (up to MaxLen).              
   //  Each string starts with a ushort that holds the strings length  
   //  (in bytes) followed by the WCHARs that make up the string.      
   //
   //  This struct is packed so the actual size of this struct is      
   //  variable.
   //
   //******************************************************************


    // 
    WCHAR OSDeviceName[256 + 1];
    #define HBAScsiID_OSDeviceName_ID 4

} HBAScsiID, *PHBAScsiID;

// MSFC_FC4STATISTICS - MSFC_FC4STATISTICS
#define MSFC_FC4STATISTICSGuid \
    { 0xca8e7fe6,0xb85e,0x497f, { 0x88,0x58,0x9b,0x5d,0x93,0xa6,0x6f,0xe1 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_FC4STATISTICS_GUID, \
            0xca8e7fe6,0xb85e,0x497f,0x88,0x58,0x9b,0x5d,0x93,0xa6,0x6f,0xe1);
#endif


typedef struct _MSFC_FC4STATISTICS
{
    // 
    ULONGLONG InputRequests;
    #define MSFC_FC4STATISTICS_InputRequests_SIZE sizeof(ULONGLONG)
    #define MSFC_FC4STATISTICS_InputRequests_ID 1

    // 
    ULONGLONG OutputRequests;
    #define MSFC_FC4STATISTICS_OutputRequests_SIZE sizeof(ULONGLONG)
    #define MSFC_FC4STATISTICS_OutputRequests_ID 2

    // 
    ULONGLONG ControlRequests;
    #define MSFC_FC4STATISTICS_ControlRequests_SIZE sizeof(ULONGLONG)
    #define MSFC_FC4STATISTICS_ControlRequests_ID 3

    // 
    ULONGLONG InputMegabytes;
    #define MSFC_FC4STATISTICS_InputMegabytes_SIZE sizeof(ULONGLONG)
    #define MSFC_FC4STATISTICS_InputMegabytes_ID 4

    // 
    ULONGLONG OutputMegabytes;
    #define MSFC_FC4STATISTICS_OutputMegabytes_SIZE sizeof(ULONGLONG)
    #define MSFC_FC4STATISTICS_OutputMegabytes_ID 5

} MSFC_FC4STATISTICS, *PMSFC_FC4STATISTICS;

#define MSFC_FC4STATISTICS_SIZE (FIELD_OFFSET(MSFC_FC4STATISTICS, OutputMegabytes) + MSFC_FC4STATISTICS_OutputMegabytes_SIZE)

// MSFC_TM - MSFC_TM
// 
#define MSFC_TMGuid \
    { 0x8cf4c7eb,0xa286,0x409d, { 0x9e,0xb9,0x29,0xd7,0xe0,0xe9,0xf4,0xfa } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_TM_GUID, \
            0x8cf4c7eb,0xa286,0x409d,0x9e,0xb9,0x29,0xd7,0xe0,0xe9,0xf4,0xfa);
#endif


typedef struct _MSFC_TM
{
    // 
    ULONG tm_sec;
    #define MSFC_TM_tm_sec_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_sec_ID 1

    // 
    ULONG tm_min;
    #define MSFC_TM_tm_min_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_min_ID 2

    // 
    ULONG tm_hour;
    #define MSFC_TM_tm_hour_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_hour_ID 3

    // 
    ULONG tm_mday;
    #define MSFC_TM_tm_mday_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_mday_ID 4

    // 
    ULONG tm_mon;
    #define MSFC_TM_tm_mon_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_mon_ID 5

    // 
    ULONG tm_year;
    #define MSFC_TM_tm_year_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_year_ID 6

    // 
    ULONG tm_wday;
    #define MSFC_TM_tm_wday_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_wday_ID 7

    // 
    ULONG tm_yday;
    #define MSFC_TM_tm_yday_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_yday_ID 8

    // 
    ULONG tm_isdst;
    #define MSFC_TM_tm_isdst_SIZE sizeof(ULONG)
    #define MSFC_TM_tm_isdst_ID 9

} MSFC_TM, *PMSFC_TM;

#define MSFC_TM_SIZE (FIELD_OFFSET(MSFC_TM, tm_isdst) + MSFC_TM_tm_isdst_SIZE)

// MSFC_EventBuffer - MSFC_EventBuffer
// 
#define MSFC_EventBufferGuid \
    { 0x623f4588,0xcf01,0x4f0e, { 0xb1,0x97,0xab,0xbe,0xe5,0xe0,0xcf,0xf3 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_EventBuffer_GUID, \
            0x623f4588,0xcf01,0x4f0e,0xb1,0x97,0xab,0xbe,0xe5,0xe0,0xcf,0xf3);
#endif


typedef struct _MSFC_EventBuffer
{
    // 
    ULONG EventType;
    #define MSFC_EventBuffer_EventType_SIZE sizeof(ULONG)
    #define MSFC_EventBuffer_EventType_ID 1

    // 
    ULONG EventInfo[4];
    #define MSFC_EventBuffer_EventInfo_SIZE sizeof(ULONG[4])
    #define MSFC_EventBuffer_EventInfo_ID 2

} MSFC_EventBuffer, *PMSFC_EventBuffer;

#define MSFC_EventBuffer_SIZE (FIELD_OFFSET(MSFC_EventBuffer, EventInfo) + MSFC_EventBuffer_EventInfo_SIZE)

// MSFC_HBAAdapterMethods - MSFC_HBAAdapterMethods
// This class exposes port discovery operations that can be by a fibre channel adapter. There should be one instance of this class for each adapter
#define MSFC_HBAAdapterMethodsGuid \
    { 0xdf87d4ed,0x4612,0x4d12, { 0x85,0xfb,0x83,0x57,0x4e,0xc3,0x4b,0x7c } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_HBAAdapterMethods_GUID, \
            0xdf87d4ed,0x4612,0x4d12,0x85,0xfb,0x83,0x57,0x4e,0xc3,0x4b,0x7c);
#endif

//
// Method id definitions for MSFC_HBAAdapterMethods
#define GetDiscoveredPortAttributes     1
typedef struct _GetDiscoveredPortAttributes_IN
{
    // 
    ULONG PortIndex;
    #define GetDiscoveredPortAttributes_IN_PortIndex_SIZE sizeof(ULONG)
    #define GetDiscoveredPortAttributes_IN_PortIndex_ID 1

    // 
    ULONG DiscoveredPortIndex;
    #define GetDiscoveredPortAttributes_IN_DiscoveredPortIndex_SIZE sizeof(ULONG)
    #define GetDiscoveredPortAttributes_IN_DiscoveredPortIndex_ID 2

} GetDiscoveredPortAttributes_IN, *PGetDiscoveredPortAttributes_IN;

#define GetDiscoveredPortAttributes_IN_SIZE (FIELD_OFFSET(GetDiscoveredPortAttributes_IN, DiscoveredPortIndex) + GetDiscoveredPortAttributes_IN_DiscoveredPortIndex_SIZE)

typedef struct _GetDiscoveredPortAttributes_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetDiscoveredPortAttributes_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetDiscoveredPortAttributes_OUT_HBAStatus_ID 3

    // 
    MSFC_HBAPortAttributesResults PortAttributes;
    #define GetDiscoveredPortAttributes_OUT_PortAttributes_SIZE sizeof(MSFC_HBAPortAttributesResults)
    #define GetDiscoveredPortAttributes_OUT_PortAttributes_ID 4

} GetDiscoveredPortAttributes_OUT, *PGetDiscoveredPortAttributes_OUT;

#define GetDiscoveredPortAttributes_OUT_SIZE (FIELD_OFFSET(GetDiscoveredPortAttributes_OUT, PortAttributes) + GetDiscoveredPortAttributes_OUT_PortAttributes_SIZE)

#define GetPortAttributesByWWN     2
typedef struct _GetPortAttributesByWWN_IN
{
    // 
    UCHAR wwn[8];
    #define GetPortAttributesByWWN_IN_wwn_SIZE sizeof(UCHAR[8])
    #define GetPortAttributesByWWN_IN_wwn_ID 1

} GetPortAttributesByWWN_IN, *PGetPortAttributesByWWN_IN;

#define GetPortAttributesByWWN_IN_SIZE (FIELD_OFFSET(GetPortAttributesByWWN_IN, wwn) + GetPortAttributesByWWN_IN_wwn_SIZE)

typedef struct _GetPortAttributesByWWN_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetPortAttributesByWWN_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetPortAttributesByWWN_OUT_HBAStatus_ID 2

    // 
    MSFC_HBAPortAttributesResults PortAttributes;
    #define GetPortAttributesByWWN_OUT_PortAttributes_SIZE sizeof(MSFC_HBAPortAttributesResults)
    #define GetPortAttributesByWWN_OUT_PortAttributes_ID 3

} GetPortAttributesByWWN_OUT, *PGetPortAttributesByWWN_OUT;

#define GetPortAttributesByWWN_OUT_SIZE (FIELD_OFFSET(GetPortAttributesByWWN_OUT, PortAttributes) + GetPortAttributesByWWN_OUT_PortAttributes_SIZE)

#define RefreshInformation     3
#define SendCTPassThru     4
typedef struct _SendCTPassThru_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendCTPassThru_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendCTPassThru_IN_PortWWN_ID 1

    // 
    ULONG RequestBufferCount;
    #define SendCTPassThru_IN_RequestBufferCount_SIZE sizeof(ULONG)
    #define SendCTPassThru_IN_RequestBufferCount_ID 2

    // 
    UCHAR RequestBuffer[1];
    #define SendCTPassThru_IN_RequestBuffer_ID 3

} SendCTPassThru_IN, *PSendCTPassThru_IN;

typedef struct _SendCTPassThru_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendCTPassThru_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendCTPassThru_OUT_HBAStatus_ID 4

    // 
    ULONG TotalResponseBufferCount;
    #define SendCTPassThru_OUT_TotalResponseBufferCount_SIZE sizeof(ULONG)
    #define SendCTPassThru_OUT_TotalResponseBufferCount_ID 5

    // 
    ULONG ActualResponseBufferCount;
    #define SendCTPassThru_OUT_ActualResponseBufferCount_SIZE sizeof(ULONG)
    #define SendCTPassThru_OUT_ActualResponseBufferCount_ID 6

    // 
    UCHAR ResponseBuffer[1];
    #define SendCTPassThru_OUT_ResponseBuffer_ID 7

} SendCTPassThru_OUT, *PSendCTPassThru_OUT;

#define SendRNID     5
typedef struct _SendRNID_IN
{
    // 
    UCHAR wwn[8];
    #define SendRNID_IN_wwn_SIZE sizeof(UCHAR[8])
    #define SendRNID_IN_wwn_ID 1

    // 
    ULONG wwntype;
    #define SendRNID_IN_wwntype_SIZE sizeof(ULONG)
    #define SendRNID_IN_wwntype_ID 2

} SendRNID_IN, *PSendRNID_IN;

#define SendRNID_IN_SIZE (FIELD_OFFSET(SendRNID_IN, wwntype) + SendRNID_IN_wwntype_SIZE)

typedef struct _SendRNID_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendRNID_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendRNID_OUT_HBAStatus_ID 3

    // 
    ULONG ResponseBufferCount;
    #define SendRNID_OUT_ResponseBufferCount_SIZE sizeof(ULONG)
    #define SendRNID_OUT_ResponseBufferCount_ID 4

    // 
    UCHAR ResponseBuffer[1];
    #define SendRNID_OUT_ResponseBuffer_ID 5

} SendRNID_OUT, *PSendRNID_OUT;

#define SendRNIDV2     6
typedef struct _SendRNIDV2_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendRNIDV2_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendRNIDV2_IN_PortWWN_ID 1

    // 
    UCHAR DestWWN[8];
    #define SendRNIDV2_IN_DestWWN_SIZE sizeof(UCHAR[8])
    #define SendRNIDV2_IN_DestWWN_ID 2

    // 
    ULONG DestFCID;
    #define SendRNIDV2_IN_DestFCID_SIZE sizeof(ULONG)
    #define SendRNIDV2_IN_DestFCID_ID 3

    // 
    ULONG NodeIdDataFormat;
    #define SendRNIDV2_IN_NodeIdDataFormat_SIZE sizeof(ULONG)
    #define SendRNIDV2_IN_NodeIdDataFormat_ID 4

} SendRNIDV2_IN, *PSendRNIDV2_IN;

#define SendRNIDV2_IN_SIZE (FIELD_OFFSET(SendRNIDV2_IN, NodeIdDataFormat) + SendRNIDV2_IN_NodeIdDataFormat_SIZE)

typedef struct _SendRNIDV2_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendRNIDV2_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendRNIDV2_OUT_HBAStatus_ID 5

    // 
    ULONG TotalRspBufferSize;
    #define SendRNIDV2_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendRNIDV2_OUT_TotalRspBufferSize_ID 6

    // 
    ULONG ActualRspBufferSize;
    #define SendRNIDV2_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendRNIDV2_OUT_ActualRspBufferSize_ID 7

    // 
    UCHAR RspBuffer[1];
    #define SendRNIDV2_OUT_RspBuffer_ID 8

} SendRNIDV2_OUT, *PSendRNIDV2_OUT;

#define GetFC3MgmtInfo     7
typedef struct _GetFC3MgmtInfo_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetFC3MgmtInfo_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetFC3MgmtInfo_OUT_HBAStatus_ID 1

    // 
    HBAFC3MgmtInfo MgmtInfo;
    #define GetFC3MgmtInfo_OUT_MgmtInfo_SIZE sizeof(HBAFC3MgmtInfo)
    #define GetFC3MgmtInfo_OUT_MgmtInfo_ID 2

} GetFC3MgmtInfo_OUT, *PGetFC3MgmtInfo_OUT;

#define GetFC3MgmtInfo_OUT_SIZE (FIELD_OFFSET(GetFC3MgmtInfo_OUT, MgmtInfo) + GetFC3MgmtInfo_OUT_MgmtInfo_SIZE)

#define SetFC3MgmtInfo     8
typedef struct _SetFC3MgmtInfo_IN
{
    // 
    HBAFC3MgmtInfo MgmtInfo;
    #define SetFC3MgmtInfo_IN_MgmtInfo_SIZE sizeof(HBAFC3MgmtInfo)
    #define SetFC3MgmtInfo_IN_MgmtInfo_ID 1

} SetFC3MgmtInfo_IN, *PSetFC3MgmtInfo_IN;

#define SetFC3MgmtInfo_IN_SIZE (FIELD_OFFSET(SetFC3MgmtInfo_IN, MgmtInfo) + SetFC3MgmtInfo_IN_MgmtInfo_SIZE)

typedef struct _SetFC3MgmtInfo_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SetFC3MgmtInfo_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SetFC3MgmtInfo_OUT_HBAStatus_ID 2

} SetFC3MgmtInfo_OUT, *PSetFC3MgmtInfo_OUT;

#define SetFC3MgmtInfo_OUT_SIZE (FIELD_OFFSET(SetFC3MgmtInfo_OUT, HBAStatus) + SetFC3MgmtInfo_OUT_HBAStatus_SIZE)

#define SendRPL     9
typedef struct _SendRPL_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendRPL_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendRPL_IN_PortWWN_ID 1

    // 
    UCHAR AgentWWN[8];
    #define SendRPL_IN_AgentWWN_SIZE sizeof(UCHAR[8])
    #define SendRPL_IN_AgentWWN_ID 2

    // 
    ULONG agent_domain;
    #define SendRPL_IN_agent_domain_SIZE sizeof(ULONG)
    #define SendRPL_IN_agent_domain_ID 3

    // 
    ULONG portIndex;
    #define SendRPL_IN_portIndex_SIZE sizeof(ULONG)
    #define SendRPL_IN_portIndex_ID 4

} SendRPL_IN, *PSendRPL_IN;

#define SendRPL_IN_SIZE (FIELD_OFFSET(SendRPL_IN, portIndex) + SendRPL_IN_portIndex_SIZE)

typedef struct _SendRPL_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendRPL_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendRPL_OUT_HBAStatus_ID 5

    // 
    ULONG TotalRspBufferSize;
    #define SendRPL_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendRPL_OUT_TotalRspBufferSize_ID 6

    // 
    ULONG ActualRspBufferSize;
    #define SendRPL_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendRPL_OUT_ActualRspBufferSize_ID 7

    // 
    UCHAR RspBuffer[1];
    #define SendRPL_OUT_RspBuffer_ID 8

} SendRPL_OUT, *PSendRPL_OUT;

#define SendRPS     10
typedef struct _SendRPS_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendRPS_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendRPS_IN_PortWWN_ID 1

    // 
    UCHAR AgentWWN[8];
    #define SendRPS_IN_AgentWWN_SIZE sizeof(UCHAR[8])
    #define SendRPS_IN_AgentWWN_ID 2

    // 
    UCHAR ObjectWWN[8];
    #define SendRPS_IN_ObjectWWN_SIZE sizeof(UCHAR[8])
    #define SendRPS_IN_ObjectWWN_ID 3

    // 
    ULONG AgentDomain;
    #define SendRPS_IN_AgentDomain_SIZE sizeof(ULONG)
    #define SendRPS_IN_AgentDomain_ID 4

    // 
    ULONG ObjectPortNumber;
    #define SendRPS_IN_ObjectPortNumber_SIZE sizeof(ULONG)
    #define SendRPS_IN_ObjectPortNumber_ID 5

} SendRPS_IN, *PSendRPS_IN;

#define SendRPS_IN_SIZE (FIELD_OFFSET(SendRPS_IN, ObjectPortNumber) + SendRPS_IN_ObjectPortNumber_SIZE)

typedef struct _SendRPS_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendRPS_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendRPS_OUT_HBAStatus_ID 6

    // 
    ULONG TotalRspBufferSize;
    #define SendRPS_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendRPS_OUT_TotalRspBufferSize_ID 7

    // 
    ULONG ActualRspBufferSize;
    #define SendRPS_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendRPS_OUT_ActualRspBufferSize_ID 8

    // 
    UCHAR RspBuffer[1];
    #define SendRPS_OUT_RspBuffer_ID 9

} SendRPS_OUT, *PSendRPS_OUT;

#define SendSRL     11
typedef struct _SendSRL_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendSRL_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendSRL_IN_PortWWN_ID 1

    // 
    UCHAR WWN[8];
    #define SendSRL_IN_WWN_SIZE sizeof(UCHAR[8])
    #define SendSRL_IN_WWN_ID 2

    // 
    ULONG Domain;
    #define SendSRL_IN_Domain_SIZE sizeof(ULONG)
    #define SendSRL_IN_Domain_ID 3

} SendSRL_IN, *PSendSRL_IN;

#define SendSRL_IN_SIZE (FIELD_OFFSET(SendSRL_IN, Domain) + SendSRL_IN_Domain_SIZE)

typedef struct _SendSRL_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendSRL_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendSRL_OUT_HBAStatus_ID 4

    // 
    ULONG TotalRspBufferSize;
    #define SendSRL_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendSRL_OUT_TotalRspBufferSize_ID 5

    // 
    ULONG ActualRspBufferSize;
    #define SendSRL_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendSRL_OUT_ActualRspBufferSize_ID 6

    // 
    UCHAR RspBuffer[1];
    #define SendSRL_OUT_RspBuffer_ID 7

} SendSRL_OUT, *PSendSRL_OUT;

#define SendLIRR     12
typedef struct _SendLIRR_IN
{
    // 
    UCHAR SourceWWN[8];
    #define SendLIRR_IN_SourceWWN_SIZE sizeof(UCHAR[8])
    #define SendLIRR_IN_SourceWWN_ID 1

    // 
    UCHAR DestWWN[8];
    #define SendLIRR_IN_DestWWN_SIZE sizeof(UCHAR[8])
    #define SendLIRR_IN_DestWWN_ID 2

    // 
    UCHAR Function;
    #define SendLIRR_IN_Function_SIZE sizeof(UCHAR)
    #define SendLIRR_IN_Function_ID 3

    // 
    UCHAR Type;
    #define SendLIRR_IN_Type_SIZE sizeof(UCHAR)
    #define SendLIRR_IN_Type_ID 4

} SendLIRR_IN, *PSendLIRR_IN;

#define SendLIRR_IN_SIZE (FIELD_OFFSET(SendLIRR_IN, Type) + SendLIRR_IN_Type_SIZE)

typedef struct _SendLIRR_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendLIRR_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendLIRR_OUT_HBAStatus_ID 5

    // 
    ULONG TotalRspBufferSize;
    #define SendLIRR_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendLIRR_OUT_TotalRspBufferSize_ID 6

    // 
    ULONG ActualRspBufferSize;
    #define SendLIRR_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendLIRR_OUT_ActualRspBufferSize_ID 7

    // 
    UCHAR RspBuffer[1];
    #define SendLIRR_OUT_RspBuffer_ID 8

} SendLIRR_OUT, *PSendLIRR_OUT;

#define GetFC4Statistics     13
typedef struct _GetFC4Statistics_IN
{
    // 
    UCHAR PortWWN[8];
    #define GetFC4Statistics_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define GetFC4Statistics_IN_PortWWN_ID 1

    // 
    UCHAR FC4Type;
    #define GetFC4Statistics_IN_FC4Type_SIZE sizeof(UCHAR)
    #define GetFC4Statistics_IN_FC4Type_ID 2

} GetFC4Statistics_IN, *PGetFC4Statistics_IN;

#define GetFC4Statistics_IN_SIZE (FIELD_OFFSET(GetFC4Statistics_IN, FC4Type) + GetFC4Statistics_IN_FC4Type_SIZE)

typedef struct _GetFC4Statistics_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetFC4Statistics_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetFC4Statistics_OUT_HBAStatus_ID 3

    // 
    MSFC_FC4STATISTICS FC4Statistics;
    #define GetFC4Statistics_OUT_FC4Statistics_SIZE sizeof(MSFC_FC4STATISTICS)
    #define GetFC4Statistics_OUT_FC4Statistics_ID 4

} GetFC4Statistics_OUT, *PGetFC4Statistics_OUT;

#define GetFC4Statistics_OUT_SIZE (FIELD_OFFSET(GetFC4Statistics_OUT, FC4Statistics) + GetFC4Statistics_OUT_FC4Statistics_SIZE)

#define GetFCPStatistics     14
typedef struct _GetFCPStatistics_IN
{
    // 
    HBAScsiID ScsiId;
    #define GetFCPStatistics_IN_ScsiId_SIZE sizeof(HBAScsiID)
    #define GetFCPStatistics_IN_ScsiId_ID 1

} GetFCPStatistics_IN, *PGetFCPStatistics_IN;

#define GetFCPStatistics_IN_SIZE (FIELD_OFFSET(GetFCPStatistics_IN, ScsiId) + GetFCPStatistics_IN_ScsiId_SIZE)

typedef struct _GetFCPStatistics_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetFCPStatistics_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetFCPStatistics_OUT_HBAStatus_ID 2

    // 
    MSFC_FC4STATISTICS FC4Statistics;
    #define GetFCPStatistics_OUT_FC4Statistics_SIZE sizeof(MSFC_FC4STATISTICS)
    #define GetFCPStatistics_OUT_FC4Statistics_ID 3

} GetFCPStatistics_OUT, *PGetFCPStatistics_OUT;

#define GetFCPStatistics_OUT_SIZE (FIELD_OFFSET(GetFCPStatistics_OUT, FC4Statistics) + GetFCPStatistics_OUT_FC4Statistics_SIZE)

#define ScsiInquiry     15
typedef struct _ScsiInquiry_IN
{
    // 
    UCHAR Cdb[6];
    #define ScsiInquiry_IN_Cdb_SIZE sizeof(UCHAR[6])
    #define ScsiInquiry_IN_Cdb_ID 1

    // 
    UCHAR HbaPortWWN[8];
    #define ScsiInquiry_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiInquiry_IN_HbaPortWWN_ID 2

    // 
    UCHAR DiscoveredPortWWN[8];
    #define ScsiInquiry_IN_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiInquiry_IN_DiscoveredPortWWN_ID 3

    // 
    ULONGLONG FcLun;
    #define ScsiInquiry_IN_FcLun_SIZE sizeof(ULONGLONG)
    #define ScsiInquiry_IN_FcLun_ID 4

} ScsiInquiry_IN, *PScsiInquiry_IN;

#define ScsiInquiry_IN_SIZE (FIELD_OFFSET(ScsiInquiry_IN, FcLun) + ScsiInquiry_IN_FcLun_SIZE)

typedef struct _ScsiInquiry_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define ScsiInquiry_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define ScsiInquiry_OUT_HBAStatus_ID 5

    // 
    ULONG ResponseBufferSize;
    #define ScsiInquiry_OUT_ResponseBufferSize_SIZE sizeof(ULONG)
    #define ScsiInquiry_OUT_ResponseBufferSize_ID 6

    // 
    ULONG SenseBufferSize;
    #define ScsiInquiry_OUT_SenseBufferSize_SIZE sizeof(ULONG)
    #define ScsiInquiry_OUT_SenseBufferSize_ID 7

    // 
    UCHAR ScsiStatus;
    #define ScsiInquiry_OUT_ScsiStatus_SIZE sizeof(UCHAR)
    #define ScsiInquiry_OUT_ScsiStatus_ID 8

    // 
    UCHAR ResponseBuffer[1];
    #define ScsiInquiry_OUT_ResponseBuffer_ID 9

    // 
//  UCHAR SenseBuffer[1];
    #define ScsiInquiry_OUT_SenseBuffer_ID 10

} ScsiInquiry_OUT, *PScsiInquiry_OUT;

#define ScsiReadCapacity     16
typedef struct _ScsiReadCapacity_IN
{
    // 
    UCHAR Cdb[10];
    #define ScsiReadCapacity_IN_Cdb_SIZE sizeof(UCHAR[10])
    #define ScsiReadCapacity_IN_Cdb_ID 1

    // 
    UCHAR HbaPortWWN[8];
    #define ScsiReadCapacity_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiReadCapacity_IN_HbaPortWWN_ID 2

    // 
    UCHAR DiscoveredPortWWN[8];
    #define ScsiReadCapacity_IN_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiReadCapacity_IN_DiscoveredPortWWN_ID 3

    // 
    ULONGLONG FcLun;
    #define ScsiReadCapacity_IN_FcLun_SIZE sizeof(ULONGLONG)
    #define ScsiReadCapacity_IN_FcLun_ID 4

} ScsiReadCapacity_IN, *PScsiReadCapacity_IN;

#define ScsiReadCapacity_IN_SIZE (FIELD_OFFSET(ScsiReadCapacity_IN, FcLun) + ScsiReadCapacity_IN_FcLun_SIZE)

typedef struct _ScsiReadCapacity_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define ScsiReadCapacity_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define ScsiReadCapacity_OUT_HBAStatus_ID 5

    // 
    ULONG ResponseBufferSize;
    #define ScsiReadCapacity_OUT_ResponseBufferSize_SIZE sizeof(ULONG)
    #define ScsiReadCapacity_OUT_ResponseBufferSize_ID 6

    // 
    ULONG SenseBufferSize;
    #define ScsiReadCapacity_OUT_SenseBufferSize_SIZE sizeof(ULONG)
    #define ScsiReadCapacity_OUT_SenseBufferSize_ID 7

    // 
    UCHAR ScsiStatus;
    #define ScsiReadCapacity_OUT_ScsiStatus_SIZE sizeof(UCHAR)
    #define ScsiReadCapacity_OUT_ScsiStatus_ID 8

    // 
    UCHAR ResponseBuffer[1];
    #define ScsiReadCapacity_OUT_ResponseBuffer_ID 9

    // 
//  UCHAR SenseBuffer[1];
    #define ScsiReadCapacity_OUT_SenseBuffer_ID 10

} ScsiReadCapacity_OUT, *PScsiReadCapacity_OUT;

#define ScsiReportLuns     17
typedef struct _ScsiReportLuns_IN
{
    // 
    UCHAR Cdb[12];
    #define ScsiReportLuns_IN_Cdb_SIZE sizeof(UCHAR[12])
    #define ScsiReportLuns_IN_Cdb_ID 1

    // 
    UCHAR HbaPortWWN[8];
    #define ScsiReportLuns_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiReportLuns_IN_HbaPortWWN_ID 2

    // 
    UCHAR DiscoveredPortWWN[8];
    #define ScsiReportLuns_IN_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define ScsiReportLuns_IN_DiscoveredPortWWN_ID 3

} ScsiReportLuns_IN, *PScsiReportLuns_IN;

#define ScsiReportLuns_IN_SIZE (FIELD_OFFSET(ScsiReportLuns_IN, DiscoveredPortWWN) + ScsiReportLuns_IN_DiscoveredPortWWN_SIZE)

typedef struct _ScsiReportLuns_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define ScsiReportLuns_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define ScsiReportLuns_OUT_HBAStatus_ID 4

    // 
    ULONG ResponseBufferSize;
    #define ScsiReportLuns_OUT_ResponseBufferSize_SIZE sizeof(ULONG)
    #define ScsiReportLuns_OUT_ResponseBufferSize_ID 5

    // 
    ULONG SenseBufferSize;
    #define ScsiReportLuns_OUT_SenseBufferSize_SIZE sizeof(ULONG)
    #define ScsiReportLuns_OUT_SenseBufferSize_ID 6

    // 
    UCHAR ScsiStatus;
    #define ScsiReportLuns_OUT_ScsiStatus_SIZE sizeof(UCHAR)
    #define ScsiReportLuns_OUT_ScsiStatus_ID 7

    // 
    UCHAR ResponseBuffer[1];
    #define ScsiReportLuns_OUT_ResponseBuffer_ID 8

    // 
//  UCHAR SenseBuffer[1];
    #define ScsiReportLuns_OUT_SenseBuffer_ID 9

} ScsiReportLuns_OUT, *PScsiReportLuns_OUT;

#define GetEventBuffer     18
typedef struct _GetEventBuffer_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetEventBuffer_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetEventBuffer_OUT_HBAStatus_ID 1

    // 
    ULONG EventCount;
    #define GetEventBuffer_OUT_EventCount_SIZE sizeof(ULONG)
    #define GetEventBuffer_OUT_EventCount_ID 2

    // 
    MSFC_EventBuffer Events[1];
    #define GetEventBuffer_OUT_Events_ID 3

} GetEventBuffer_OUT, *PGetEventBuffer_OUT;

#define SendRLS     19
typedef struct _SendRLS_IN
{
    // 
    UCHAR PortWWN[8];
    #define SendRLS_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SendRLS_IN_PortWWN_ID 1

    // 
    UCHAR DestWWN[8];
    #define SendRLS_IN_DestWWN_SIZE sizeof(UCHAR[8])
    #define SendRLS_IN_DestWWN_ID 2

} SendRLS_IN, *PSendRLS_IN;

#define SendRLS_IN_SIZE (FIELD_OFFSET(SendRLS_IN, DestWWN) + SendRLS_IN_DestWWN_SIZE)

typedef struct _SendRLS_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SendRLS_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SendRLS_OUT_HBAStatus_ID 3

    // 
    ULONG TotalRspBufferSize;
    #define SendRLS_OUT_TotalRspBufferSize_SIZE sizeof(ULONG)
    #define SendRLS_OUT_TotalRspBufferSize_ID 4

    // 
    ULONG ActualRspBufferSize;
    #define SendRLS_OUT_ActualRspBufferSize_SIZE sizeof(ULONG)
    #define SendRLS_OUT_ActualRspBufferSize_ID 5

    // 
    UCHAR RspBuffer[1];
    #define SendRLS_OUT_RspBuffer_ID 6

} SendRLS_OUT, *PSendRLS_OUT;


// HBAFCPID - HBAFCPID
#define HBAFCPIDGuid \
    { 0xff02bc96,0x7fb0,0x4bac, { 0x8f,0x97,0xc7,0x1e,0x49,0x5f,0xa6,0x98 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAFCPID_GUID, \
            0xff02bc96,0x7fb0,0x4bac,0x8f,0x97,0xc7,0x1e,0x49,0x5f,0xa6,0x98);
#endif


typedef struct _HBAFCPID
{
    // 
    ULONG Fcid;
    #define HBAFCPID_Fcid_SIZE sizeof(ULONG)
    #define HBAFCPID_Fcid_ID 1

    // 
    UCHAR NodeWWN[8];
    #define HBAFCPID_NodeWWN_SIZE sizeof(UCHAR[8])
    #define HBAFCPID_NodeWWN_ID 2

    // 
    UCHAR PortWWN[8];
    #define HBAFCPID_PortWWN_SIZE sizeof(UCHAR[8])
    #define HBAFCPID_PortWWN_ID 3

    // 
    ULONGLONG FcpLun;
    #define HBAFCPID_FcpLun_SIZE sizeof(ULONGLONG)
    #define HBAFCPID_FcpLun_ID 4

} HBAFCPID, *PHBAFCPID;

#define HBAFCPID_SIZE (FIELD_OFFSET(HBAFCPID, FcpLun) + HBAFCPID_FcpLun_SIZE)

// HBAFCPScsiEntry - HBAFCPScsiEntry
#define HBAFCPScsiEntryGuid \
    { 0x77ca1248,0x1505,0x4221, { 0x8e,0xb6,0xbb,0xb6,0xec,0x77,0x1a,0x87 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAFCPScsiEntry_GUID, \
            0x77ca1248,0x1505,0x4221,0x8e,0xb6,0xbb,0xb6,0xec,0x77,0x1a,0x87);
#endif


typedef struct _HBAFCPScsiEntry
{
    // 
    HBAFCPID FCPId;
    #define HBAFCPScsiEntry_FCPId_SIZE sizeof(HBAFCPID)
    #define HBAFCPScsiEntry_FCPId_ID 1

    // 
    UCHAR Luid[256];
    #define HBAFCPScsiEntry_Luid_SIZE sizeof(UCHAR[256])
    #define HBAFCPScsiEntry_Luid_ID 2



   //******************************************************************
   //
   //  HBAScsiID is variable length since it contains a string.        
   //  This makes HBAFCPScsiEntry variable length.                     
   //
   //******************************************************************


    // 
    HBAScsiID ScsiId;
    #define HBAFCPScsiEntry_ScsiId_SIZE sizeof(HBAScsiID)
    #define HBAFCPScsiEntry_ScsiId_ID 3

} HBAFCPScsiEntry, *PHBAFCPScsiEntry;

#define HBAFCPScsiEntry_SIZE (FIELD_OFFSET(HBAFCPScsiEntry, ScsiId) + HBAFCPScsiEntry_ScsiId_SIZE)

// HBAFCPBindingEntry - HBAFCPBindingEntry
#define HBAFCPBindingEntryGuid \
    { 0xfceff8b7,0x9d6b,0x4115, { 0x84,0x22,0x05,0x99,0x24,0x51,0xa6,0x29 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAFCPBindingEntry_GUID, \
            0xfceff8b7,0x9d6b,0x4115,0x84,0x22,0x05,0x99,0x24,0x51,0xa6,0x29);
#endif


typedef struct _HBAFCPBindingEntry
{
    // 
    ULONG Type;
    #define HBAFCPBindingEntry_Type_SIZE sizeof(ULONG)
    #define HBAFCPBindingEntry_Type_ID 1

    // 
    HBAFCPID FCPId;
    #define HBAFCPBindingEntry_FCPId_SIZE sizeof(HBAFCPID)
    #define HBAFCPBindingEntry_FCPId_ID 2

    // 
    HBAScsiID ScsiId;
    #define HBAFCPBindingEntry_ScsiId_SIZE sizeof(HBAScsiID)
    #define HBAFCPBindingEntry_ScsiId_ID 3

} HBAFCPBindingEntry, *PHBAFCPBindingEntry;

#define HBAFCPBindingEntry_SIZE (FIELD_OFFSET(HBAFCPBindingEntry, ScsiId) + HBAFCPBindingEntry_ScsiId_SIZE)

// HBAFCPBindingEntry2 - HBAFCPBindingEntry2
#define HBAFCPBindingEntry2Guid \
    { 0x3a1e7679,0x4b1f,0x4f31, { 0xa8,0xae,0xfe,0x92,0x78,0x73,0x09,0x24 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(HBAFCPBindingEntry2_GUID, \
            0x3a1e7679,0x4b1f,0x4f31,0xa8,0xae,0xfe,0x92,0x78,0x73,0x09,0x24);
#endif


typedef struct _HBAFCPBindingEntry2
{
    // Ways of performing persistent binding
    ULONG Type;
    #define HBAFCPBindingEntry2_Type_SIZE sizeof(ULONG)
    #define HBAFCPBindingEntry2_Type_ID 1

    // 
    HBAFCPID FCPId;
    #define HBAFCPBindingEntry2_FCPId_SIZE sizeof(HBAFCPID)
    #define HBAFCPBindingEntry2_FCPId_ID 2

    // 
    UCHAR Luid[256];
    #define HBAFCPBindingEntry2_Luid_SIZE sizeof(UCHAR[256])
    #define HBAFCPBindingEntry2_Luid_ID 3

    // 
    HBAScsiID ScsiId;
    #define HBAFCPBindingEntry2_ScsiId_SIZE sizeof(HBAScsiID)
    #define HBAFCPBindingEntry2_ScsiId_ID 4

} HBAFCPBindingEntry2, *PHBAFCPBindingEntry2;

#define HBAFCPBindingEntry2_SIZE (FIELD_OFFSET(HBAFCPBindingEntry2, ScsiId) + HBAFCPBindingEntry2_ScsiId_SIZE)

// MSFC_HBAFCPInfo - MSFC_HBAFCPInfo
// This class exposes operations associated with FCP on a Fibre Channel adapter. There should be one instance of this class for each adapter.
#define MSFC_HBAFCPInfoGuid \
    { 0x7a1fc391,0x5b23,0x4c19, { 0xb0,0xeb,0xb1,0xae,0xf5,0x90,0x50,0xc3 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_HBAFCPInfo_GUID, \
            0x7a1fc391,0x5b23,0x4c19,0xb0,0xeb,0xb1,0xae,0xf5,0x90,0x50,0xc3);
#endif

//
// Method id definitions for MSFC_HBAFCPInfo
#define GetFcpTargetMapping     1
typedef struct _GetFcpTargetMapping_IN
{
    // 
    UCHAR HbaPortWWN[8];
    #define GetFcpTargetMapping_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define GetFcpTargetMapping_IN_HbaPortWWN_ID 1

    // 
    ULONG InEntryCount;
    #define GetFcpTargetMapping_IN_InEntryCount_SIZE sizeof(ULONG)
    #define GetFcpTargetMapping_IN_InEntryCount_ID 2

} GetFcpTargetMapping_IN, *PGetFcpTargetMapping_IN;

#define GetFcpTargetMapping_IN_SIZE (FIELD_OFFSET(GetFcpTargetMapping_IN, InEntryCount) + GetFcpTargetMapping_IN_InEntryCount_SIZE)

typedef struct _GetFcpTargetMapping_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetFcpTargetMapping_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetFcpTargetMapping_OUT_HBAStatus_ID 3

    // 
    ULONG TotalEntryCount;
    #define GetFcpTargetMapping_OUT_TotalEntryCount_SIZE sizeof(ULONG)
    #define GetFcpTargetMapping_OUT_TotalEntryCount_ID 4

    // 
    ULONG OutEntryCount;
    #define GetFcpTargetMapping_OUT_OutEntryCount_SIZE sizeof(ULONG)
    #define GetFcpTargetMapping_OUT_OutEntryCount_ID 5

    // 
    HBAFCPScsiEntry Entry[1];
    #define GetFcpTargetMapping_OUT_Entry_ID 6

} GetFcpTargetMapping_OUT, *PGetFcpTargetMapping_OUT;

#define GetFcpPersistentBinding     2
typedef struct _GetFcpPersistentBinding_IN
{
    // 
    ULONG InEntryCount;
    #define GetFcpPersistentBinding_IN_InEntryCount_SIZE sizeof(ULONG)
    #define GetFcpPersistentBinding_IN_InEntryCount_ID 1

} GetFcpPersistentBinding_IN, *PGetFcpPersistentBinding_IN;

#define GetFcpPersistentBinding_IN_SIZE (FIELD_OFFSET(GetFcpPersistentBinding_IN, InEntryCount) + GetFcpPersistentBinding_IN_InEntryCount_SIZE)

typedef struct _GetFcpPersistentBinding_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetFcpPersistentBinding_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetFcpPersistentBinding_OUT_HBAStatus_ID 2

    // 
    ULONG TotalEntryCount;
    #define GetFcpPersistentBinding_OUT_TotalEntryCount_SIZE sizeof(ULONG)
    #define GetFcpPersistentBinding_OUT_TotalEntryCount_ID 3

    // 
    ULONG OutEntryCount;
    #define GetFcpPersistentBinding_OUT_OutEntryCount_SIZE sizeof(ULONG)
    #define GetFcpPersistentBinding_OUT_OutEntryCount_ID 4

    // 
    HBAFCPBindingEntry Entry[1];
    #define GetFcpPersistentBinding_OUT_Entry_ID 5

} GetFcpPersistentBinding_OUT, *PGetFcpPersistentBinding_OUT;

#define GetBindingCapability     3
typedef struct _GetBindingCapability_IN
{
    // 
    UCHAR PortWWN[8];
    #define GetBindingCapability_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define GetBindingCapability_IN_PortWWN_ID 1

} GetBindingCapability_IN, *PGetBindingCapability_IN;

#define GetBindingCapability_IN_SIZE (FIELD_OFFSET(GetBindingCapability_IN, PortWWN) + GetBindingCapability_IN_PortWWN_SIZE)

typedef struct _GetBindingCapability_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetBindingCapability_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetBindingCapability_OUT_HBAStatus_ID 2

    // Ways of performing persistent binding
    ULONG BindType;
    #define GetBindingCapability_OUT_BindType_SIZE sizeof(ULONG)
    #define GetBindingCapability_OUT_BindType_ID 3

} GetBindingCapability_OUT, *PGetBindingCapability_OUT;

#define GetBindingCapability_OUT_SIZE (FIELD_OFFSET(GetBindingCapability_OUT, BindType) + GetBindingCapability_OUT_BindType_SIZE)

#define GetBindingSupport     4
typedef struct _GetBindingSupport_IN
{
    // 
    UCHAR PortWWN[8];
    #define GetBindingSupport_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define GetBindingSupport_IN_PortWWN_ID 1

} GetBindingSupport_IN, *PGetBindingSupport_IN;

#define GetBindingSupport_IN_SIZE (FIELD_OFFSET(GetBindingSupport_IN, PortWWN) + GetBindingSupport_IN_PortWWN_SIZE)

typedef struct _GetBindingSupport_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetBindingSupport_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetBindingSupport_OUT_HBAStatus_ID 2

    // Ways of performing persistent binding
    ULONG BindType;
    #define GetBindingSupport_OUT_BindType_SIZE sizeof(ULONG)
    #define GetBindingSupport_OUT_BindType_ID 3

} GetBindingSupport_OUT, *PGetBindingSupport_OUT;

#define GetBindingSupport_OUT_SIZE (FIELD_OFFSET(GetBindingSupport_OUT, BindType) + GetBindingSupport_OUT_BindType_SIZE)

#define SetBindingSupport     5
typedef struct _SetBindingSupport_IN
{
    // 
    UCHAR PortWWN[8];
    #define SetBindingSupport_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SetBindingSupport_IN_PortWWN_ID 1

    // Ways of performing persistent binding
    ULONG BindType;
    #define SetBindingSupport_IN_BindType_SIZE sizeof(ULONG)
    #define SetBindingSupport_IN_BindType_ID 2

} SetBindingSupport_IN, *PSetBindingSupport_IN;

#define SetBindingSupport_IN_SIZE (FIELD_OFFSET(SetBindingSupport_IN, BindType) + SetBindingSupport_IN_BindType_SIZE)

typedef struct _SetBindingSupport_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SetBindingSupport_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SetBindingSupport_OUT_HBAStatus_ID 3

} SetBindingSupport_OUT, *PSetBindingSupport_OUT;

#define SetBindingSupport_OUT_SIZE (FIELD_OFFSET(SetBindingSupport_OUT, HBAStatus) + SetBindingSupport_OUT_HBAStatus_SIZE)

#define GetPersistentBinding2     6
typedef struct _GetPersistentBinding2_IN
{
    // 
    UCHAR PortWWN[8];
    #define GetPersistentBinding2_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define GetPersistentBinding2_IN_PortWWN_ID 1

    // 
    ULONG InEntryCount;
    #define GetPersistentBinding2_IN_InEntryCount_SIZE sizeof(ULONG)
    #define GetPersistentBinding2_IN_InEntryCount_ID 2

} GetPersistentBinding2_IN, *PGetPersistentBinding2_IN;

#define GetPersistentBinding2_IN_SIZE (FIELD_OFFSET(GetPersistentBinding2_IN, InEntryCount) + GetPersistentBinding2_IN_InEntryCount_SIZE)

typedef struct _GetPersistentBinding2_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define GetPersistentBinding2_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define GetPersistentBinding2_OUT_HBAStatus_ID 3

    // 
    ULONG TotalEntryCount;
    #define GetPersistentBinding2_OUT_TotalEntryCount_SIZE sizeof(ULONG)
    #define GetPersistentBinding2_OUT_TotalEntryCount_ID 4

    // 
    ULONG OutEntryCount;
    #define GetPersistentBinding2_OUT_OutEntryCount_SIZE sizeof(ULONG)
    #define GetPersistentBinding2_OUT_OutEntryCount_ID 5

    // 
    HBAFCPBindingEntry2 Bindings[1];
    #define GetPersistentBinding2_OUT_Bindings_ID 6

} GetPersistentBinding2_OUT, *PGetPersistentBinding2_OUT;



//*********************************************************************
//
//  A call to HBA_SetPersistentBindingV2 will call SetPersistentEntry
//  once for each binding entry.
//  Each binding entry that SetPersistentEntry accepts will be stored
//  in the registry.
//
//  Persistent bindings are stored in the registry under:
//
//     System\CurrentControlSet\Control\Storage\FC\<PortWWN>
//
//         under the REG_BINARY key Bindings  is the struct:
//
//             typedef struct {
//                 ULONG            Version;
//                 HBA_FCPBINDING2  Bindings;
//             } HBAP_PERSISTENT_BINDINGS, *PHBAP_PERSISTENT_BINDINGS;
//
//  This is done so that storport capable drivers may have access to
//  this information during boot
//
//********************************************************************

#define HBA_REGISTRY_BINDING_VERSION        (1)
#define HBA_REGISTRY_BINDING_RELATIVE_PATH  L"System\\CurrentControlSet\\Control\\Storage\\FC"
#define HBA_REGISTRY_BINDING_KEY            L"Bindings"


#define SetPersistentEntry     7
typedef struct _SetPersistentEntry_IN
{
    // 
    UCHAR PortWWN[8];
    #define SetPersistentEntry_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define SetPersistentEntry_IN_PortWWN_ID 1

    // 
    HBAFCPBindingEntry2 Binding;
    #define SetPersistentEntry_IN_Binding_SIZE sizeof(HBAFCPBindingEntry2)
    #define SetPersistentEntry_IN_Binding_ID 2

} SetPersistentEntry_IN, *PSetPersistentEntry_IN;

#define SetPersistentEntry_IN_SIZE (FIELD_OFFSET(SetPersistentEntry_IN, Binding) + SetPersistentEntry_IN_Binding_SIZE)

typedef struct _SetPersistentEntry_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define SetPersistentEntry_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define SetPersistentEntry_OUT_HBAStatus_ID 3

} SetPersistentEntry_OUT, *PSetPersistentEntry_OUT;

#define SetPersistentEntry_OUT_SIZE (FIELD_OFFSET(SetPersistentEntry_OUT, HBAStatus) + SetPersistentEntry_OUT_HBAStatus_SIZE)

#define RemovePersistentEntry     8
typedef struct _RemovePersistentEntry_IN
{
    // 
    UCHAR PortWWN[8];
    #define RemovePersistentEntry_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define RemovePersistentEntry_IN_PortWWN_ID 1

    // 
    HBAFCPBindingEntry2 Binding;
    #define RemovePersistentEntry_IN_Binding_SIZE sizeof(HBAFCPBindingEntry2)
    #define RemovePersistentEntry_IN_Binding_ID 2

} RemovePersistentEntry_IN, *PRemovePersistentEntry_IN;

#define RemovePersistentEntry_IN_SIZE (FIELD_OFFSET(RemovePersistentEntry_IN, Binding) + RemovePersistentEntry_IN_Binding_SIZE)

typedef struct _RemovePersistentEntry_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define RemovePersistentEntry_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define RemovePersistentEntry_OUT_HBAStatus_ID 3

} RemovePersistentEntry_OUT, *PRemovePersistentEntry_OUT;

#define RemovePersistentEntry_OUT_SIZE (FIELD_OFFSET(RemovePersistentEntry_OUT, HBAStatus) + RemovePersistentEntry_OUT_HBAStatus_SIZE)


// MSFC_AdapterEvent - MSFC_AdapterEvent
// This class surfaces HBA adapter events




//
// Event types. These match the definitions in hbaapi.h and must be kept in sync
//
	/* Adapter Level Events */
#define HBA_EVENT_ADAPTER_UNKNOWN	0x100
#define HBA_EVENT_ADAPTER_ADD		0x101
#define HBA_EVENT_ADAPTER_REMOVE	0x102
#define HBA_EVENT_ADAPTER_CHANGE	0x103

	/* Port Level Events */
#define HBA_EVENT_PORT_UNKNOWN		0x200
#define HBA_EVENT_PORT_OFFLINE		0x201
#define HBA_EVENT_PORT_ONLINE		0x202
#define HBA_EVENT_PORT_NEW_TARGETS	0x203
#define HBA_EVENT_PORT_FABRIC		0x204
	
	/* Port Statistics Events */
#define HBA_EVENT_PORT_STAT_THRESHOLD	0x301
#define HBA_EVENT_PORT_STAT_GROWTH	0x302

	/* Target Level Events */
#define HBA_EVENT_TARGET_UNKNOWN	0x400
#define HBA_EVENT_TARGET_OFFLINE	0x401
#define HBA_EVENT_TARGET_ONLINE		0x402
#define HBA_EVENT_TARGET_REMOVED	0x403

	/* Fabric Link  Events */
#define HBA_EVENT_LINK_UNKNOWN		0x500
#define HBA_EVENT_LINK_INCIDENT		0x501

#define MSFC_AdapterEventGuid \
    { 0xe9e47403,0xd1d7,0x43f8, { 0x8e,0xe3,0x53,0xcd,0xbf,0xff,0x56,0x46 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_AdapterEvent_GUID, \
            0xe9e47403,0xd1d7,0x43f8,0x8e,0xe3,0x53,0xcd,0xbf,0xff,0x56,0x46);
#endif


typedef struct _MSFC_AdapterEvent
{
    // Event Type
    ULONG EventType;
    #define MSFC_AdapterEvent_EventType_SIZE sizeof(ULONG)
    #define MSFC_AdapterEvent_EventType_ID 1

    // Adapter WWN
    UCHAR PortWWN[8];
    #define MSFC_AdapterEvent_PortWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_AdapterEvent_PortWWN_ID 2

} MSFC_AdapterEvent, *PMSFC_AdapterEvent;

#define MSFC_AdapterEvent_SIZE (FIELD_OFFSET(MSFC_AdapterEvent, PortWWN) + MSFC_AdapterEvent_PortWWN_SIZE)

// MSFC_PortEvent - MSFC_PortEvent
// This class surfaces HBA port events
#define MSFC_PortEventGuid \
    { 0x095fbe97,0x3876,0x48ef, { 0x8a,0x04,0x1c,0x55,0x93,0x5d,0x0d,0xf5 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_PortEvent_GUID, \
            0x095fbe97,0x3876,0x48ef,0x8a,0x04,0x1c,0x55,0x93,0x5d,0x0d,0xf5);
#endif


typedef struct _MSFC_PortEvent
{
    // Type of event
    ULONG EventType;
    #define MSFC_PortEvent_EventType_SIZE sizeof(ULONG)
    #define MSFC_PortEvent_EventType_ID 1

    // Fabric port id
    ULONG FabricPortId;
    #define MSFC_PortEvent_FabricPortId_SIZE sizeof(ULONG)
    #define MSFC_PortEvent_FabricPortId_ID 2

    // Port WWN
    UCHAR PortWWN[8];
    #define MSFC_PortEvent_PortWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_PortEvent_PortWWN_ID 3

} MSFC_PortEvent, *PMSFC_PortEvent;

#define MSFC_PortEvent_SIZE (FIELD_OFFSET(MSFC_PortEvent, PortWWN) + MSFC_PortEvent_PortWWN_SIZE)

// MSFC_TargetEvent - MSFC_TargetEvent
// This class surfaces HBA target events
#define MSFC_TargetEventGuid \
    { 0xcfa6ef26,0x8675,0x4e27, { 0x9a,0x0b,0xb4,0xa8,0x60,0xdd,0xd0,0xf3 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_TargetEvent_GUID, \
            0xcfa6ef26,0x8675,0x4e27,0x9a,0x0b,0xb4,0xa8,0x60,0xdd,0xd0,0xf3);
#endif


typedef struct _MSFC_TargetEvent
{
    // Type of event
    ULONG EventType;
    #define MSFC_TargetEvent_EventType_SIZE sizeof(ULONG)
    #define MSFC_TargetEvent_EventType_ID 1

    // Port WWN
    UCHAR PortWWN[8];
    #define MSFC_TargetEvent_PortWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_TargetEvent_PortWWN_ID 2

    // Discovered Port WWN
    UCHAR DiscoveredPortWWN[8];
    #define MSFC_TargetEvent_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_TargetEvent_DiscoveredPortWWN_ID 3

} MSFC_TargetEvent, *PMSFC_TargetEvent;

#define MSFC_TargetEvent_SIZE (FIELD_OFFSET(MSFC_TargetEvent, DiscoveredPortWWN) + MSFC_TargetEvent_DiscoveredPortWWN_SIZE)

// MSFC_LinkEvent - MSFC_LinkEvent
// This class surfaces HBA link events
#define MSFC_LinkEventGuid \
    { 0xc66015ee,0x014b,0x498a, { 0x94,0x51,0x99,0xfe,0xad,0x0a,0xb4,0x51 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_LinkEvent_GUID, \
            0xc66015ee,0x014b,0x498a,0x94,0x51,0x99,0xfe,0xad,0x0a,0xb4,0x51);
#endif


typedef struct _MSFC_LinkEvent
{
    // Type of event
    ULONG EventType;
    #define MSFC_LinkEvent_EventType_SIZE sizeof(ULONG)
    #define MSFC_LinkEvent_EventType_ID 1

    // Discovered Port WWN
    UCHAR AdapterWWN[8];
    #define MSFC_LinkEvent_AdapterWWN_SIZE sizeof(UCHAR[8])
    #define MSFC_LinkEvent_AdapterWWN_ID 2

    // Size of RLIR buffer
    ULONG RLIRBufferSize;
    #define MSFC_LinkEvent_RLIRBufferSize_SIZE sizeof(ULONG)
    #define MSFC_LinkEvent_RLIRBufferSize_ID 3

    // Size of RLIR buffer
    UCHAR RLIRBuffer[1];
    #define MSFC_LinkEvent_RLIRBuffer_ID 4

} MSFC_LinkEvent, *PMSFC_LinkEvent;

// MSFC_EventControl - MSFC_EventControl
// This class defines the port statistics thresholds that will cause a HBA_EVENT_PORT_STAT_THRESHOLD event to occur
#define MSFC_EventControlGuid \
    { 0xa251ccb3,0x5ab0,0x411b, { 0x87,0x71,0x54,0x30,0xef,0x53,0xa2,0x6c } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(MSFC_EventControl_GUID, \
            0xa251ccb3,0x5ab0,0x411b,0x87,0x71,0x54,0x30,0xef,0x53,0xa2,0x6c);
#endif

//
// Method id definitions for MSFC_EventControl
#define AddTarget     10
typedef struct _AddTarget_IN
{
    // 
    UCHAR HbaPortWWN[8];
    #define AddTarget_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define AddTarget_IN_HbaPortWWN_ID 1

    // 
    UCHAR DiscoveredPortWWN[8];
    #define AddTarget_IN_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define AddTarget_IN_DiscoveredPortWWN_ID 2

    // 
    ULONG AllTargets;
    #define AddTarget_IN_AllTargets_SIZE sizeof(ULONG)
    #define AddTarget_IN_AllTargets_ID 3

} AddTarget_IN, *PAddTarget_IN;

#define AddTarget_IN_SIZE (FIELD_OFFSET(AddTarget_IN, AllTargets) + AddTarget_IN_AllTargets_SIZE)

typedef struct _AddTarget_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define AddTarget_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define AddTarget_OUT_HBAStatus_ID 4

} AddTarget_OUT, *PAddTarget_OUT;

#define AddTarget_OUT_SIZE (FIELD_OFFSET(AddTarget_OUT, HBAStatus) + AddTarget_OUT_HBAStatus_SIZE)

#define RemoveTarget     11
typedef struct _RemoveTarget_IN
{
    // 
    UCHAR HbaPortWWN[8];
    #define RemoveTarget_IN_HbaPortWWN_SIZE sizeof(UCHAR[8])
    #define RemoveTarget_IN_HbaPortWWN_ID 1

    // 
    UCHAR DiscoveredPortWWN[8];
    #define RemoveTarget_IN_DiscoveredPortWWN_SIZE sizeof(UCHAR[8])
    #define RemoveTarget_IN_DiscoveredPortWWN_ID 2

    // 
    ULONG AllTargets;
    #define RemoveTarget_IN_AllTargets_SIZE sizeof(ULONG)
    #define RemoveTarget_IN_AllTargets_ID 3

} RemoveTarget_IN, *PRemoveTarget_IN;

#define RemoveTarget_IN_SIZE (FIELD_OFFSET(RemoveTarget_IN, AllTargets) + RemoveTarget_IN_AllTargets_SIZE)

typedef struct _RemoveTarget_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define RemoveTarget_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define RemoveTarget_OUT_HBAStatus_ID 4

} RemoveTarget_OUT, *PRemoveTarget_OUT;

#define RemoveTarget_OUT_SIZE (FIELD_OFFSET(RemoveTarget_OUT, HBAStatus) + RemoveTarget_OUT_HBAStatus_SIZE)

#define AddPort     20
typedef struct _AddPort_IN
{
    // 
    UCHAR PortWWN[8];
    #define AddPort_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define AddPort_IN_PortWWN_ID 1

} AddPort_IN, *PAddPort_IN;

#define AddPort_IN_SIZE (FIELD_OFFSET(AddPort_IN, PortWWN) + AddPort_IN_PortWWN_SIZE)

typedef struct _AddPort_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define AddPort_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define AddPort_OUT_HBAStatus_ID 2

} AddPort_OUT, *PAddPort_OUT;

#define AddPort_OUT_SIZE (FIELD_OFFSET(AddPort_OUT, HBAStatus) + AddPort_OUT_HBAStatus_SIZE)

#define RemovePort     21
typedef struct _RemovePort_IN
{
    // 
    UCHAR PortWWN[8];
    #define RemovePort_IN_PortWWN_SIZE sizeof(UCHAR[8])
    #define RemovePort_IN_PortWWN_ID 1

} RemovePort_IN, *PRemovePort_IN;

#define RemovePort_IN_SIZE (FIELD_OFFSET(RemovePort_IN, PortWWN) + RemovePort_IN_PortWWN_SIZE)

typedef struct _RemovePort_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define RemovePort_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define RemovePort_OUT_HBAStatus_ID 2

} RemovePort_OUT, *PRemovePort_OUT;

#define RemovePort_OUT_SIZE (FIELD_OFFSET(RemovePort_OUT, HBAStatus) + RemovePort_OUT_HBAStatus_SIZE)

#define AddLink     30
typedef struct _AddLink_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define AddLink_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define AddLink_OUT_HBAStatus_ID 1

} AddLink_OUT, *PAddLink_OUT;

#define AddLink_OUT_SIZE (FIELD_OFFSET(AddLink_OUT, HBAStatus) + AddLink_OUT_HBAStatus_SIZE)

#define RemoveLink     31
typedef struct _RemoveLink_OUT
{
    // HBA Status result for the query operation
    ULONG HBAStatus;
    #define RemoveLink_OUT_HBAStatus_SIZE sizeof(ULONG)
    #define RemoveLink_OUT_HBAStatus_ID 1

} RemoveLink_OUT, *PRemoveLink_OUT;

#define RemoveLink_OUT_SIZE (FIELD_OFFSET(RemoveLink_OUT, HBAStatus) + RemoveLink_OUT_HBAStatus_SIZE)


#endif

