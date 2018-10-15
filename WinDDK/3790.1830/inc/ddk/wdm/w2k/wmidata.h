#ifndef _wmidata_h_
#define _wmidata_h_

// MSWmi_MofData - MSWmi_MofData
// This class supplies the binary mof information
#define MSWmi_MofDataGuid \
    { 0x05901221,0xd566,0x11d1, { 0xb2,0xf0,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSWmi_MofData_GUID, \
            0x05901221,0xd566,0x11d1,0xb2,0xf0,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSWmi_MofData
{
    // 
    ULONG Unused1;
    #define MSWmi_MofData_Unused1_SIZE sizeof(ULONG)
    #define MSWmi_MofData_Unused1_ID 1

    // 
    ULONG Unused2;
    #define MSWmi_MofData_Unused2_SIZE sizeof(ULONG)
    #define MSWmi_MofData_Unused2_ID 2

    // 
    ULONG Size;
    #define MSWmi_MofData_Size_SIZE sizeof(ULONG)
    #define MSWmi_MofData_Size_ID 3

    // 
    ULONG Unused4;
    #define MSWmi_MofData_Unused4_SIZE sizeof(ULONG)
    #define MSWmi_MofData_Unused4_ID 4

    // 
    UCHAR BinaryMofData[1];
    #define MSWmi_MofData_BinaryMofData_ID 5

} MSWmi_MofData, *PMSWmi_MofData;

// MSWmi_ProviderInfo - MSWmi_ProviderInfo
// This class supplies additional information about a data provider. Querying this class with an instance name returned from another class query will return additional information about the instance
#define MSWmi_ProviderInfoGuid \
    { 0xc7bf35d0,0xaadb,0x11d1, { 0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSWmi_ProviderInfo_GUID, \
            0xc7bf35d0,0xaadb,0x11d1,0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


// Warning: Header for class MSWmi_ProviderInfo cannot be created
typedef struct _MSWmi_ProviderInfo
{
    char VariableData[1];

} MSWmi_ProviderInfo, *PMSWmi_ProviderInfo;

// MSWmi_PnPDeviceId - MSWmi_PnPDeviceId
// This class supplies the PnPDeviceId for a specific device
#define DATA_PROVIDER_PNPID_GUID \
    { 0xc7bf35d2,0xaadb,0x11d1, { 0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSWmi_PnPDeviceId_GUID, \
            0xc7bf35d2,0xaadb,0x11d1,0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSWmi_PnPDeviceId
{
    // PnP Device Id for the device. This property is useful for mapping from the wmi namespace to the cimv2 namespace classes using the view provider
    CHAR VariableData[1];
    #define MSWmi_PnPDeviceId_PnPDeviceId_ID 1

} MSWmi_PnPDeviceId, *PMSWmi_PnPDeviceId;

// MSWmi_PnPInstanceNames - MSWmi_PnPInstanceNames
// This class supplies the Instance names associated with a PnP Device Instance Id
#define DATA_PROVIDER_PNPID_INSTANCE_NAMES_GUID \
    { 0xc7bf35d3,0xaadb,0x11d1, { 0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSWmi_PnPInstanceNames_GUID, \
            0xc7bf35d3,0xaadb,0x11d1,0xbf,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSWmi_PnPInstanceNames
{
    // Count of instance names associated with this PnPId
    ULONG Count;
    #define MSWmi_PnPInstanceNames_Count_SIZE sizeof(ULONG)
    #define MSWmi_PnPInstanceNames_Count_ID 1

    // Wmi Instance Names for the device. This property is useful for mapping from the wmi namespace to the cimv2 namespace classes using the view provider
    WCHAR InstanceNameList[1];
    #define MSWmi_PnPInstanceNames_InstanceNameList_ID 2

} MSWmi_PnPInstanceNames, *PMSWmi_PnPInstanceNames;

// MSSmBios_RawSMBiosTables - MSSmBios_RawSMBiosTables
// Raw SMBIOS Tables
#define MSSmBios_RawSMBiosTablesGuid \
    { 0x8f680850,0xa584,0x11d1, { 0xbf,0x38,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSSmBios_RawSMBiosTables_GUID, \
            0x8f680850,0xa584,0x11d1,0xbf,0x38,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSSmBios_RawSMBiosTables
{
    // 
    BOOLEAN Used20CallingMethod;
    #define MSSmBios_RawSMBiosTables_Used20CallingMethod_SIZE sizeof(BOOLEAN)
    #define MSSmBios_RawSMBiosTables_Used20CallingMethod_ID 1

    // 
    UCHAR SmbiosMajorVersion;
    #define MSSmBios_RawSMBiosTables_SmbiosMajorVersion_SIZE sizeof(UCHAR)
    #define MSSmBios_RawSMBiosTables_SmbiosMajorVersion_ID 2

    // 
    UCHAR SmbiosMinorVersion;
    #define MSSmBios_RawSMBiosTables_SmbiosMinorVersion_SIZE sizeof(UCHAR)
    #define MSSmBios_RawSMBiosTables_SmbiosMinorVersion_ID 3

    // 
    UCHAR DmiRevision;
    #define MSSmBios_RawSMBiosTables_DmiRevision_SIZE sizeof(UCHAR)
    #define MSSmBios_RawSMBiosTables_DmiRevision_ID 4

    // 
    ULONG Size;
    #define MSSmBios_RawSMBiosTables_Size_SIZE sizeof(ULONG)
    #define MSSmBios_RawSMBiosTables_Size_ID 5

    // 
    UCHAR SMBiosData[1];
    #define MSSmBios_RawSMBiosTables_SMBiosData_ID 6

} MSSmBios_RawSMBiosTables, *PMSSmBios_RawSMBiosTables;

// MSPower_DeviceEnable - MSPower_DeviceEnable
// The control sets whether the device should dynamically power on and off while the system is working.
#define MSPower_DeviceEnableGuid \
    { 0x827c0a6f,0xfeb0,0x11d0, { 0xbd,0x26,0x00,0xaa,0x00,0xb7,0xb3,0x2a } }

DEFINE_GUID(MSPower_DeviceEnable_GUID, \
            0x827c0a6f,0xfeb0,0x11d0,0xbd,0x26,0x00,0xaa,0x00,0xb7,0xb3,0x2a);


typedef struct _MSPower_DeviceEnable
{
    // 
    BOOLEAN Enable;
    #define MSPower_DeviceEnable_Enable_SIZE sizeof(BOOLEAN)
    #define MSPower_DeviceEnable_Enable_ID 1

} MSPower_DeviceEnable, *PMSPower_DeviceEnable;

// MSPower_DeviceWakeEnable - MSPower_DeviceWakeEnable
// This control indicates whether the device should be configured to wake a sleeping system.
#define MSPower_DeviceWakeEnableGuid \
    { 0xa9546a82,0xfeb0,0x11d0, { 0xbd,0x26,0x00,0xaa,0x00,0xb7,0xb3,0x2a } }

DEFINE_GUID(MSPower_DeviceWakeEnable_GUID, \
            0xa9546a82,0xfeb0,0x11d0,0xbd,0x26,0x00,0xaa,0x00,0xb7,0xb3,0x2a);


typedef struct _MSPower_DeviceWakeEnable
{
    // 
    BOOLEAN Enable;
    #define MSPower_DeviceWakeEnable_Enable_SIZE sizeof(BOOLEAN)
    #define MSPower_DeviceWakeEnable_Enable_ID 1

} MSPower_DeviceWakeEnable, *PMSPower_DeviceWakeEnable;

// MSNdis_NetworkAddress - MSNdis_NetworkAddress
#define MSNdis_NetworkAddressGuid \
    { 0xb5bd98b7,0x0201,0x11d1, { 0xa5,0x0e,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSNdis_NetworkAddress_GUID, \
            0xb5bd98b7,0x0201,0x11d1,0xa5,0x0e,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSNdis_NetworkAddress
{
    // 
    UCHAR Address[6];
    #define MSNdis_NetworkAddress_Address_SIZE sizeof(UCHAR[6])
    #define MSNdis_NetworkAddress_Address_ID 1

} MSNdis_NetworkAddress, *PMSNdis_NetworkAddress;

// MSNdis_NetworkShortAddress - MSNdis_NetworkShortAddress
#define MSNdis_NetworkShortAddressGuid \
    { 0xb5bd98b8,0x0201,0x11d1, { 0xa5,0x0e,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSNdis_NetworkShortAddress_GUID, \
            0xb5bd98b8,0x0201,0x11d1,0xa5,0x0e,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSNdis_NetworkShortAddress
{
    // 
    UCHAR Address[2];
    #define MSNdis_NetworkShortAddress_Address_SIZE sizeof(UCHAR[2])
    #define MSNdis_NetworkShortAddress_Address_ID 1

} MSNdis_NetworkShortAddress, *PMSNdis_NetworkShortAddress;

// MSNdis_NetworkLinkSpeed - MSNdis_NetworkLinkSpeed
#define MSNdis_NetworkLinkSpeedGuid \
    { 0x60fc6b57,0x0f66,0x11d1, { 0x96,0xa7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NetworkLinkSpeed_GUID, \
            0x60fc6b57,0x0f66,0x11d1,0x96,0xa7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_NetworkLinkSpeed
{
    // 
    ULONG Outbound;
    #define MSNdis_NetworkLinkSpeed_Outbound_SIZE sizeof(ULONG)
    #define MSNdis_NetworkLinkSpeed_Outbound_ID 1

    // 
    ULONG Inbound;
    #define MSNdis_NetworkLinkSpeed_Inbound_SIZE sizeof(ULONG)
    #define MSNdis_NetworkLinkSpeed_Inbound_ID 2

} MSNdis_NetworkLinkSpeed, *PMSNdis_NetworkLinkSpeed;

// MSNdis_EnumerateAdapter - MSNdis_EnumerateAdapter
// NDIS Enumerate Adapter
#define MSNdis_EnumerateAdapterGuid \
    { 0x981f2d7f,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EnumerateAdapter_GUID, \
            0x981f2d7f,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EnumerateAdapter
{
    // Device name.
    CHAR VariableData[1];
    #define MSNdis_EnumerateAdapter_DeviceName_ID 1

} MSNdis_EnumerateAdapter, *PMSNdis_EnumerateAdapter;

// MSNdis_NotifyAdapterRemoval - MSNdis_NotifyAdapterRemoval
// NDIS Notify Adapter Removal
#define MSNdis_NotifyAdapterRemovalGuid \
    { 0x981f2d80,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NotifyAdapterRemoval_GUID, \
            0x981f2d80,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_NotifyAdapterRemoval
{
    // Device name.
    CHAR VariableData[1];
    #define MSNdis_NotifyAdapterRemoval_DeviceName_ID 1

} MSNdis_NotifyAdapterRemoval, *PMSNdis_NotifyAdapterRemoval;

// MSNdis_NotifyAdapterArrival - MSNdis_NotifyAdapterArrival
// NDIS Notify Adapter Arrival
#define MSNdis_NotifyAdapterArrivalGuid \
    { 0x981f2d81,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NotifyAdapterArrival_GUID, \
            0x981f2d81,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_NotifyAdapterArrival
{
    // Device name.
    CHAR VariableData[1];
    #define MSNdis_NotifyAdapterArrival_DeviceName_ID 1

} MSNdis_NotifyAdapterArrival, *PMSNdis_NotifyAdapterArrival;

// MSNdis_NdisEnumerateVc - MSNdis_NdisEnumerateVc
// NDIS Enumerate VC
#define MSNdis_NdisEnumerateVcGuid \
    { 0x981f2d82,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NdisEnumerateVc_GUID, \
            0x981f2d82,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_NdisEnumerateVc cannot be created
typedef struct _MSNdis_NdisEnumerateVc
{
    char VariableData[1];

} MSNdis_NdisEnumerateVc, *PMSNdis_NdisEnumerateVc;

// MSNdis_NotifyVcRemoval - MSNdis_NotifyVcRemoval
// NDIS Notify VC Removal
#define MSNdis_NotifyVcRemovalGuid \
    { 0x981f2d79,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NotifyVcRemoval_GUID, \
            0x981f2d79,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_NotifyVcRemoval cannot be created
typedef struct _MSNdis_NotifyVcRemoval
{
    char VariableData[1];

} MSNdis_NotifyVcRemoval, *PMSNdis_NotifyVcRemoval;

// MSNdis_NotifyVcArrival - MSNdis_NotifyVcArrival
// NDIS Notify VC Arrival
#define MSNdis_NotifyVcArrivalGuid \
    { 0x182f9e0c,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_NotifyVcArrival_GUID, \
            0x182f9e0c,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_NotifyVcArrival cannot be created
typedef struct _MSNdis_NotifyVcArrival
{
    char VariableData[1];

} MSNdis_NotifyVcArrival, *PMSNdis_NotifyVcArrival;

// MSNdis_HardwareStatus - MSNdis_HardwareStatus
// NDIS Hardware Status
#define MSNdis_HardwareStatusGuid \
    { 0x5ec10354,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_HardwareStatus_GUID, \
            0x5ec10354,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_HardwareStatus
{
    // Current hardware status of the underlying NIC.
    ULONG NdisHardwareStatus;
    #define MSNdis_HardwareStatus_NdisHardwareStatus_SIZE sizeof(ULONG)
    #define MSNdis_HardwareStatus_NdisHardwareStatus_ID 1

} MSNdis_HardwareStatus, *PMSNdis_HardwareStatus;

// MSNdis_MediaSupported - MSNdis_MediaSupported
// NDIS Media Types Supported
#define MSNdis_MediaSupportedGuid \
    { 0x5ec10355,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MediaSupported_GUID, \
            0x5ec10355,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MediaSupported
{
    // Number of media types supported.
    ULONG NumberElements;
    #define MSNdis_MediaSupported_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_MediaSupported_NumberElements_ID 1

    // List of media types the NIC supports.
    ULONG NdisMediaSupported[1];
    #define MSNdis_MediaSupported_NdisMediaSupported_ID 2

} MSNdis_MediaSupported, *PMSNdis_MediaSupported;

// MSNdis_MediaInUse - MSNdis_MediaInUse
// NDIS Media Types In Use
#define MSNdis_MediaInUseGuid \
    { 0x5ec10356,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MediaInUse_GUID, \
            0x5ec10356,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MediaInUse
{
    // Number of media types in use.
    ULONG NumberElements;
    #define MSNdis_MediaInUse_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_MediaInUse_NumberElements_ID 1

    // List of media types the NIC is currently supporting.
    ULONG NdisMediaInUse[1];
    #define MSNdis_MediaInUse_NdisMediaInUse_ID 2

} MSNdis_MediaInUse, *PMSNdis_MediaInUse;

// MSNdis_MaximumLookahead - MSNdis_MaximumLookahead
// NDIS Maximum Lookahead Supported
#define MSNdis_MaximumLookaheadGuid \
    { 0x5ec10357,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MaximumLookahead_GUID, \
            0x5ec10357,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MaximumLookahead
{
    // The maximum number of bytes the NIC can always provide as lookahead data.
    ULONG NdisMaximumLookahead;
    #define MSNdis_MaximumLookahead_NdisMaximumLookahead_SIZE sizeof(ULONG)
    #define MSNdis_MaximumLookahead_NdisMaximumLookahead_ID 1

} MSNdis_MaximumLookahead, *PMSNdis_MaximumLookahead;

// MSNdis_MaximumFrameSize - MSNdis_MaximumFrameSize
// NDIS Maximum Frame Size
#define MSNdis_MaximumFrameSizeGuid \
    { 0x5ec10358,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MaximumFrameSize_GUID, \
            0x5ec10358,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MaximumFrameSize
{
    // The maximum network packet size in bytes the NIC supports, not including a header.
    ULONG NdisMaximumFrameSize;
    #define MSNdis_MaximumFrameSize_NdisMaximumFrameSize_SIZE sizeof(ULONG)
    #define MSNdis_MaximumFrameSize_NdisMaximumFrameSize_ID 1

} MSNdis_MaximumFrameSize, *PMSNdis_MaximumFrameSize;

// MSNdis_LinkSpeed - MSNdis_LinkSpeed
// NDIS Link Speed
#define MSNdis_LinkSpeedGuid \
    { 0x5ec10359,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_LinkSpeed_GUID, \
            0x5ec10359,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_LinkSpeed
{
    // The maximum speed of the NIC (kbps).
    ULONG NdisLinkSpeed;
    #define MSNdis_LinkSpeed_NdisLinkSpeed_SIZE sizeof(ULONG)
    #define MSNdis_LinkSpeed_NdisLinkSpeed_ID 1

} MSNdis_LinkSpeed, *PMSNdis_LinkSpeed;

// MSNdis_TransmitBufferSpace - MSNdis_TransmitBufferSpace
// NDIS Transmit Buffer Space
#define MSNdis_TransmitBufferSpaceGuid \
    { 0x5ec1035a,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TransmitBufferSpace_GUID, \
            0x5ec1035a,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TransmitBufferSpace
{
    // The amount of memory, in bytes, on the NIC available for buffering transmit data.
    ULONG NdisTransmitBufferSpace;
    #define MSNdis_TransmitBufferSpace_NdisTransmitBufferSpace_SIZE sizeof(ULONG)
    #define MSNdis_TransmitBufferSpace_NdisTransmitBufferSpace_ID 1

} MSNdis_TransmitBufferSpace, *PMSNdis_TransmitBufferSpace;

// MSNdis_ReceiveBufferSpace - MSNdis_ReceiveBufferSpace
// NDIS Receive Buffer Space
#define MSNdis_ReceiveBufferSpaceGuid \
    { 0x5ec1035b,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_ReceiveBufferSpace_GUID, \
            0x5ec1035b,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_ReceiveBufferSpace
{
    // The amount of memory on the NIC available for buffering receive data.
    ULONG NdisReceiveBufferSpace;
    #define MSNdis_ReceiveBufferSpace_NdisReceiveBufferSpace_SIZE sizeof(ULONG)
    #define MSNdis_ReceiveBufferSpace_NdisReceiveBufferSpace_ID 1

} MSNdis_ReceiveBufferSpace, *PMSNdis_ReceiveBufferSpace;

// MSNdis_TransmitBlockSize - MSNdis_TransmitBlockSize
// NDIS Transmit Block Size
#define MSNdis_TransmitBlockSizeGuid \
    { 0x5ec1035c,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TransmitBlockSize_GUID, \
            0x5ec1035c,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TransmitBlockSize
{
    // The minimum number of bytes that a single net packet occupies in the transmit buffer space of the NIC.
    ULONG NdisTransmitBlockSize;
    #define MSNdis_TransmitBlockSize_NdisTransmitBlockSize_SIZE sizeof(ULONG)
    #define MSNdis_TransmitBlockSize_NdisTransmitBlockSize_ID 1

} MSNdis_TransmitBlockSize, *PMSNdis_TransmitBlockSize;

// MSNdis_ReceiveBlockSize - MSNdis_ReceiveBlockSize
// NDIS Receive Block Size
#define MSNdis_ReceiveBlockSizeGuid \
    { 0x5ec1035d,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_ReceiveBlockSize_GUID, \
            0x5ec1035d,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_ReceiveBlockSize
{
    // The amount of storage, in bytes, that a single packet occupies in the receive buffer space of the NIC.
    ULONG NdisReceiveBlockSize;
    #define MSNdis_ReceiveBlockSize_NdisReceiveBlockSize_SIZE sizeof(ULONG)
    #define MSNdis_ReceiveBlockSize_NdisReceiveBlockSize_ID 1

} MSNdis_ReceiveBlockSize, *PMSNdis_ReceiveBlockSize;

// MSNdis_VendorID - MSNdis_VendorID
// NDIS Vendor ID
#define MSNdis_VendorIDGuid \
    { 0x5ec1035e,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_VendorID_GUID, \
            0x5ec1035e,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_VendorID
{
    // A three-byte IEEE-registered vendor code, followed by a single byte the vendor assigns to identify a particular NIC.
    ULONG NdisVendorID;
    #define MSNdis_VendorID_NdisVendorID_SIZE sizeof(ULONG)
    #define MSNdis_VendorID_NdisVendorID_ID 1

} MSNdis_VendorID, *PMSNdis_VendorID;

// MSNdis_VendorDescription - MSNdis_VendorDescription
// NDIS Vendor Description
#define MSNdis_VendorDescriptionGuid \
    { 0x5ec1035f,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_VendorDescription_GUID, \
            0x5ec1035f,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_VendorDescription
{
    // Zero-terminated string describing the NIC.
    CHAR VariableData[1];
    #define MSNdis_VendorDescription_NdisVendorDescription_ID 1

} MSNdis_VendorDescription, *PMSNdis_VendorDescription;

// MSNdis_CurrentPacketFilter - MSNdis_CurrentPacketFilter
// NDIS Current Packet Filter
#define MSNdis_CurrentPacketFilterGuid \
    { 0x5ec10360,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CurrentPacketFilter_GUID, \
            0x5ec10360,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CurrentPacketFilter
{
    // Current packet types that will be received and indicated by the NIC.
    ULONG NdisCurrentPacketFilter;
    #define MSNdis_CurrentPacketFilter_NdisCurrentPacketFilter_SIZE sizeof(ULONG)
    #define MSNdis_CurrentPacketFilter_NdisCurrentPacketFilter_ID 1

} MSNdis_CurrentPacketFilter, *PMSNdis_CurrentPacketFilter;

// MSNdis_CurrentLookahead - MSNdis_CurrentLookahead
// NDIS Current Lookahead
#define MSNdis_CurrentLookaheadGuid \
    { 0x5ec10361,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CurrentLookahead_GUID, \
            0x5ec10361,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CurrentLookahead
{
    // The number of bytes of received packet data, excluding the header, that will be indicated to the protocol driver.
    ULONG NdisCurrentLookahead;
    #define MSNdis_CurrentLookahead_NdisCurrentLookahead_SIZE sizeof(ULONG)
    #define MSNdis_CurrentLookahead_NdisCurrentLookahead_ID 1

} MSNdis_CurrentLookahead, *PMSNdis_CurrentLookahead;

// MSNdis_DriverVersion - MSNdis_DriverVersion
// NDIS Driver Version
#define MSNdis_DriverVersionGuid \
    { 0x5ec10362,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_DriverVersion_GUID, \
            0x5ec10362,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_DriverVersion
{
    // The NDIS version in use by the NIC driver.
    USHORT NdisDriverVersion;
    #define MSNdis_DriverVersion_NdisDriverVersion_SIZE sizeof(USHORT)
    #define MSNdis_DriverVersion_NdisDriverVersion_ID 1

} MSNdis_DriverVersion, *PMSNdis_DriverVersion;

// MSNdis_MaximumTotalSize - MSNdis_MaximumTotalSize
// NDIS Maximum Packet Total Size
#define MSNdis_MaximumTotalSizeGuid \
    { 0x5ec10363,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MaximumTotalSize_GUID, \
            0x5ec10363,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MaximumTotalSize
{
    // The maximum total packet length, in bytes, the NIC supports, including the header.
    ULONG NdisMaximumTotalSize;
    #define MSNdis_MaximumTotalSize_NdisMaximumTotalSize_SIZE sizeof(ULONG)
    #define MSNdis_MaximumTotalSize_NdisMaximumTotalSize_ID 1

} MSNdis_MaximumTotalSize, *PMSNdis_MaximumTotalSize;

// MSNdis_MacOptions - MSNdis_MacOptions
// NDIS MAC Options
#define MSNdis_MacOptionsGuid \
    { 0x5ec10365,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MacOptions_GUID, \
            0x5ec10365,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MacOptions
{
    // A bitmask that defines optional properties of the underlying driver or its NIC.
    ULONG NdisMacOptions;
    #define MSNdis_MacOptions_NdisMacOptions_SIZE sizeof(ULONG)
    #define MSNdis_MacOptions_NdisMacOptions_ID 1

} MSNdis_MacOptions, *PMSNdis_MacOptions;

// MSNdis_MediaConnectStatus - MSNdis_MediaConnectStatus
// NDIS Media Connect Status
#define MSNdis_MediaConnectStatusGuid \
    { 0x5ec10366,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MediaConnectStatus_GUID, \
            0x5ec10366,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MediaConnectStatus
{
    // The connection status of the NIC on the network.
    ULONG NdisMediaConnectStatus;
    #define MSNdis_MediaConnectStatus_NdisMediaConnectStatus_SIZE sizeof(ULONG)
    #define MSNdis_MediaConnectStatus_NdisMediaConnectStatus_ID 1

} MSNdis_MediaConnectStatus, *PMSNdis_MediaConnectStatus;

// MSNdis_MaximumSendPackets - MSNdis_MaximumSendPackets
// NDIS Maximum Send Packets
#define MSNdis_MaximumSendPacketsGuid \
    { 0x5ec10367,0xa61a,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_MaximumSendPackets_GUID, \
            0x5ec10367,0xa61a,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_MaximumSendPackets
{
    // The maximum number of send packets the MiniportSendPackets function can accept.
    ULONG NdisMaximumSendPackets;
    #define MSNdis_MaximumSendPackets_NdisMaximumSendPackets_SIZE sizeof(ULONG)
    #define MSNdis_MaximumSendPackets_NdisMaximumSendPackets_ID 1

} MSNdis_MaximumSendPackets, *PMSNdis_MaximumSendPackets;

// MSNdis_VendorDriverVersion - MSNdis_VendorDriverVersion
// NDIS Vendor's Driver Version
#define MSNdis_VendorDriverVersionGuid \
    { 0x447956f9,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_VendorDriverVersion_GUID, \
            0x447956f9,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_VendorDriverVersion
{
    // The vendor-assigned version number of the NIC driver.
    ULONG NdisVendorDriverVersion;
    #define MSNdis_VendorDriverVersion_NdisVendorDriverVersion_SIZE sizeof(ULONG)
    #define MSNdis_VendorDriverVersion_NdisVendorDriverVersion_ID 1

} MSNdis_VendorDriverVersion, *PMSNdis_VendorDriverVersion;

// MSNdis_TransmitsOk - MSNdis_TransmitsOk
// NDIS Transmits OK
#define MSNdis_TransmitsOkGuid \
    { 0x447956fa,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TransmitsOk_GUID, \
            0x447956fa,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TransmitsOk
{
    // The number of frames transmitted without errors
    ULONG NdisTransmitsOk;
    #define MSNdis_TransmitsOk_NdisTransmitsOk_SIZE sizeof(ULONG)
    #define MSNdis_TransmitsOk_NdisTransmitsOk_ID 1

} MSNdis_TransmitsOk, *PMSNdis_TransmitsOk;

// MSNdis_ReceivesOk - MSNdis_ReceivesOk
// NDIS Receives OK
#define MSNdis_ReceivesOkGuid \
    { 0x447956fb,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_ReceivesOk_GUID, \
            0x447956fb,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_ReceivesOk
{
    // The number of frames the NIC receives without errors and indicates to bound protocols.
    ULONG NdisReceivesOk;
    #define MSNdis_ReceivesOk_NdisReceivesOk_SIZE sizeof(ULONG)
    #define MSNdis_ReceivesOk_NdisReceivesOk_ID 1

} MSNdis_ReceivesOk, *PMSNdis_ReceivesOk;

// MSNdis_TransmitsError - MSNdis_TransmitsError
// NDIS Transmit Errors
#define MSNdis_TransmitsErrorGuid \
    { 0x447956fc,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TransmitsError_GUID, \
            0x447956fc,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TransmitsError
{
    // The number of frames a NIC fails to transmit.
    ULONG NdisTransmitsError;
    #define MSNdis_TransmitsError_NdisTransmitsError_SIZE sizeof(ULONG)
    #define MSNdis_TransmitsError_NdisTransmitsError_ID 1

} MSNdis_TransmitsError, *PMSNdis_TransmitsError;

// MSNdis_ReceiveError - MSNdis_ReceiveError
// NDIS Receive Errors
#define MSNdis_ReceiveErrorGuid \
    { 0x447956fd,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_ReceiveError_GUID, \
            0x447956fd,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_ReceiveError
{
    // The number of frames a NIC receives but does not indicate to the protocols due to errors.
    ULONG NdisReceiveError;
    #define MSNdis_ReceiveError_NdisReceiveError_SIZE sizeof(ULONG)
    #define MSNdis_ReceiveError_NdisReceiveError_ID 1

} MSNdis_ReceiveError, *PMSNdis_ReceiveError;

// MSNdis_ReceiveNoBuffer - MSNdis_ReceiveNoBuffer
// NDIS Receive No Buffer
#define MSNdis_ReceiveNoBufferGuid \
    { 0x447956fe,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_ReceiveNoBuffer_GUID, \
            0x447956fe,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_ReceiveNoBuffer
{
    // The number of frames the NIC cannot receive due to lack of NIC receive buffer space.
    ULONG NdisReceiveNoBuffer;
    #define MSNdis_ReceiveNoBuffer_NdisReceiveNoBuffer_SIZE sizeof(ULONG)
    #define MSNdis_ReceiveNoBuffer_NdisReceiveNoBuffer_ID 1

} MSNdis_ReceiveNoBuffer, *PMSNdis_ReceiveNoBuffer;

// MSNdis_CoHardwareStatus - MSNdis_CoHardwareStatus
// CoNDIS Hardware Status
#define MSNdis_CoHardwareStatusGuid \
    { 0x791ad192,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoHardwareStatus_GUID, \
            0x791ad192,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoHardwareStatus
{
    // Current hardware status of the underlying NIC.
    ULONG NdisCoHardwareStatus;
    #define MSNdis_CoHardwareStatus_NdisCoHardwareStatus_SIZE sizeof(ULONG)
    #define MSNdis_CoHardwareStatus_NdisCoHardwareStatus_ID 1

} MSNdis_CoHardwareStatus, *PMSNdis_CoHardwareStatus;

// MSNdis_CoMediaSupported - MSNdis_CoMediaSupported
// CoNDIS Media Types Supported
#define MSNdis_CoMediaSupportedGuid \
    { 0x791ad193,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoMediaSupported_GUID, \
            0x791ad193,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoMediaSupported
{
    // Number of media types supported.
    ULONG NumberElements;
    #define MSNdis_CoMediaSupported_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_CoMediaSupported_NumberElements_ID 1

    // List of media types the NIC supports.
    ULONG NdisCoMediaSupported[1];
    #define MSNdis_CoMediaSupported_NdisCoMediaSupported_ID 2

} MSNdis_CoMediaSupported, *PMSNdis_CoMediaSupported;

// MSNdis_CoMediaInUse - MSNdis_CoMediaInUse
// CoNDIS Media Types In Use
#define MSNdis_CoMediaInUseGuid \
    { 0x791ad194,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoMediaInUse_GUID, \
            0x791ad194,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoMediaInUse
{
    // Number of media types in use.
    ULONG NumberElements;
    #define MSNdis_CoMediaInUse_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_CoMediaInUse_NumberElements_ID 1

    // List of media types the NIC is currently supporting.
    ULONG NdisCoMediaInUse[1];
    #define MSNdis_CoMediaInUse_NdisCoMediaInUse_ID 2

} MSNdis_CoMediaInUse, *PMSNdis_CoMediaInUse;

// MSNdis_CoLinkSpeed - MSNdis_CoLinkSpeed
// CoNDIS Link Speed
#define MSNdis_CoLinkSpeedGuid \
    { 0x791ad195,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoLinkSpeed_GUID, \
            0x791ad195,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoLinkSpeed
{
    // The maximum inbound and outbound speeds of the NIC (kbps).
    MSNdis_NetworkLinkSpeed NdisCoLinkSpeed;
    #define MSNdis_CoLinkSpeed_NdisCoLinkSpeed_SIZE sizeof(MSNdis_NetworkLinkSpeed)
    #define MSNdis_CoLinkSpeed_NdisCoLinkSpeed_ID 1

} MSNdis_CoLinkSpeed, *PMSNdis_CoLinkSpeed;

// MSNdis_CoVendorId - MSNdis_CoVendorId
// CoNDIS Vendor ID
#define MSNdis_CoVendorIdGuid \
    { 0x791ad196,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoVendorId_GUID, \
            0x791ad196,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoVendorId
{
    // A three-byte IEEE-registered vendor code, followed by a single byte the vendor assigns to identify a particular NIC.
    ULONG NdisCoVendorID;
    #define MSNdis_CoVendorId_NdisCoVendorID_SIZE sizeof(ULONG)
    #define MSNdis_CoVendorId_NdisCoVendorID_ID 1

} MSNdis_CoVendorId, *PMSNdis_CoVendorId;

// MSNdis_CoVendorDescription - MSNdis_CoVendorDescription
// CoNDIS Vendor Description
#define MSNdis_CoVendorDescriptionGuid \
    { 0x791ad197,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoVendorDescription_GUID, \
            0x791ad197,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoVendorDescription
{
    // Zero-terminated string describing the NIC.
    CHAR VariableData[1];
    #define MSNdis_CoVendorDescription_NdisCoVendorDescription_ID 1

} MSNdis_CoVendorDescription, *PMSNdis_CoVendorDescription;

// MSNdis_CoDriverVersion - MSNdis_CoDriverVersion
// CoNDIS Driver Version
#define MSNdis_CoDriverVersionGuid \
    { 0x791ad198,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoDriverVersion_GUID, \
            0x791ad198,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoDriverVersion
{
    // The NDIS version in use by the NIC driver.
    USHORT NdisCoDriverVersion;
    #define MSNdis_CoDriverVersion_NdisCoDriverVersion_SIZE sizeof(USHORT)
    #define MSNdis_CoDriverVersion_NdisCoDriverVersion_ID 1

} MSNdis_CoDriverVersion, *PMSNdis_CoDriverVersion;

// MSNdis_CoMacOptions - MSNdis_CoMacOptions
// CoNDIS MAC Options
#define MSNdis_CoMacOptionsGuid \
    { 0x791ad19a,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoMacOptions_GUID, \
            0x791ad19a,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoMacOptions
{
    // A bitmask that defines optional properties of the underlying driver or its NIC.
    ULONG NdisCoMacOptions;
    #define MSNdis_CoMacOptions_NdisCoMacOptions_SIZE sizeof(ULONG)
    #define MSNdis_CoMacOptions_NdisCoMacOptions_ID 1

} MSNdis_CoMacOptions, *PMSNdis_CoMacOptions;

// MSNdis_CoMediaConnectStatus - MSNdis_CoMediaConnectStatus
// CoNDIS Media Connect Status
#define MSNdis_CoMediaConnectStatusGuid \
    { 0x791ad19b,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoMediaConnectStatus_GUID, \
            0x791ad19b,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoMediaConnectStatus
{
    // The connection status of the NIC on the network.
    ULONG NdisCoMediaConnectStatus;
    #define MSNdis_CoMediaConnectStatus_NdisCoMediaConnectStatus_SIZE sizeof(ULONG)
    #define MSNdis_CoMediaConnectStatus_NdisCoMediaConnectStatus_ID 1

} MSNdis_CoMediaConnectStatus, *PMSNdis_CoMediaConnectStatus;

// MSNdis_CoVendorDriverVersion - MSNdis_CoVendorDriverVersion
// CoNDIS Vendor's Driver Version
#define MSNdis_CoVendorDriverVersionGuid \
    { 0x791ad19c,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoVendorDriverVersion_GUID, \
            0x791ad19c,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoVendorDriverVersion
{
    // The vendor-assigned version number of the NIC driver.
    ULONG NdisCoVendorDriverVersion;
    #define MSNdis_CoVendorDriverVersion_NdisCoVendorDriverVersion_SIZE sizeof(ULONG)
    #define MSNdis_CoVendorDriverVersion_NdisCoVendorDriverVersion_ID 1

} MSNdis_CoVendorDriverVersion, *PMSNdis_CoVendorDriverVersion;

// MSNdis_CoMinimumLinkSpeed - MSNdis_CoMinimumLinkSpeed
// CoNDIS Minimum Link Speed
#define MSNdis_CoMinimumLinkSpeedGuid \
    { 0x791ad19d,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoMinimumLinkSpeed_GUID, \
            0x791ad19d,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoMinimumLinkSpeed
{
    // The maximum inbound and outbound speeds of the NIC (kbps).
    MSNdis_NetworkLinkSpeed NdisCoMinimumLinkSpeed;
    #define MSNdis_CoMinimumLinkSpeed_NdisCoMinimumLinkSpeed_SIZE sizeof(MSNdis_NetworkLinkSpeed)
    #define MSNdis_CoMinimumLinkSpeed_NdisCoMinimumLinkSpeed_ID 1

} MSNdis_CoMinimumLinkSpeed, *PMSNdis_CoMinimumLinkSpeed;

// MSNdis_CoTransmitPdusOk - MSNdis_CoTransmitPdusOk
// CoNDIS Transmits PDUs OK
#define MSNdis_CoTransmitPdusOkGuid \
    { 0x0a214805,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoTransmitPdusOk_GUID, \
            0x0a214805,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoTransmitPdusOk
{
    // The number of PDUs transmitted without errors
    ULONG NdisCoTransmitPdusOk;
    #define MSNdis_CoTransmitPdusOk_NdisCoTransmitPdusOk_SIZE sizeof(ULONG)
    #define MSNdis_CoTransmitPdusOk_NdisCoTransmitPdusOk_ID 1

} MSNdis_CoTransmitPdusOk, *PMSNdis_CoTransmitPdusOk;

// MSNdis_CoReceivePdusOk - MSNdis_CoReceivePdusOk
// CoNDIS Receive PDUs OK
#define MSNdis_CoReceivePdusOkGuid \
    { 0x0a214806,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoReceivePdusOk_GUID, \
            0x0a214806,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoReceivePdusOk
{
    // The number of PDUs the NIC receives without errors and indicates to bound protocols.
    ULONG NdisCoReceivePdusOk;
    #define MSNdis_CoReceivePdusOk_NdisCoReceivePdusOk_SIZE sizeof(ULONG)
    #define MSNdis_CoReceivePdusOk_NdisCoReceivePdusOk_ID 1

} MSNdis_CoReceivePdusOk, *PMSNdis_CoReceivePdusOk;

// MSNdis_CoTransmitPduErrors - MSNdis_CoTransmitPduErrors
// CoNDIS Transmit PDU Errors
#define MSNdis_CoTransmitPduErrorsGuid \
    { 0x0a214807,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoTransmitPduErrors_GUID, \
            0x0a214807,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoTransmitPduErrors
{
    // The number of PDUs a NIC fails to transmit.
    ULONG NdisCoTransmitPduErrors;
    #define MSNdis_CoTransmitPduErrors_NdisCoTransmitPduErrors_SIZE sizeof(ULONG)
    #define MSNdis_CoTransmitPduErrors_NdisCoTransmitPduErrors_ID 1

} MSNdis_CoTransmitPduErrors, *PMSNdis_CoTransmitPduErrors;

// MSNdis_CoReceivePduErrors - MSNdis_CoReceivePduErrors
// CoNDIS Receive PDU Errors
#define MSNdis_CoReceivePduErrorsGuid \
    { 0x0a214808,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoReceivePduErrors_GUID, \
            0x0a214808,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoReceivePduErrors
{
    // The number of PDUs a NIC receives but does not indicate to the protocols due to errors.
    ULONG NdisCoReceivePduErrors;
    #define MSNdis_CoReceivePduErrors_NdisCoReceivePduErrors_SIZE sizeof(ULONG)
    #define MSNdis_CoReceivePduErrors_NdisCoReceivePduErrors_ID 1

} MSNdis_CoReceivePduErrors, *PMSNdis_CoReceivePduErrors;

// MSNdis_CoReceivePdusNoBuffer - MSNdis_CoReceivePdusNoBuffer
// CoNDIS Receive PDUs No Buffer
#define MSNdis_CoReceivePdusNoBufferGuid \
    { 0x0a214809,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_CoReceivePdusNoBuffer_GUID, \
            0x0a214809,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_CoReceivePdusNoBuffer
{
    // The number of PDUs the NIC cannot receive due to lack of NIC receive buffer space.
    ULONG NdisCoReceivePdusNoBuffer;
    #define MSNdis_CoReceivePdusNoBuffer_NdisCoReceivePdusNoBuffer_SIZE sizeof(ULONG)
    #define MSNdis_CoReceivePdusNoBuffer_NdisCoReceivePdusNoBuffer_ID 1

} MSNdis_CoReceivePdusNoBuffer, *PMSNdis_CoReceivePdusNoBuffer;

// MSNdis_AtmSupportedVcRates - MSNdis_AtmSupportedVcRates
// NDIS ATM Supported VC Rates
#define MSNdis_AtmSupportedVcRatesGuid \
    { 0x791ad19e,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmSupportedVcRates_GUID, \
            0x791ad19e,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmSupportedVcRates
{
    // Minimum cell rate supported.
    ULONG MinCellRate;
    #define MSNdis_AtmSupportedVcRates_MinCellRate_SIZE sizeof(ULONG)
    #define MSNdis_AtmSupportedVcRates_MinCellRate_ID 1

    // Maximum cell rate supported.
    ULONG MaxCellRate;
    #define MSNdis_AtmSupportedVcRates_MaxCellRate_SIZE sizeof(ULONG)
    #define MSNdis_AtmSupportedVcRates_MaxCellRate_ID 2

} MSNdis_AtmSupportedVcRates, *PMSNdis_AtmSupportedVcRates;

// MSNdis_AtmSupportedServiceCategory - MSNdis_AtmSupportedServiceCategory
// NDIS ATM Supported Service Category
#define MSNdis_AtmSupportedServiceCategoryGuid \
    { 0x791ad19f,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmSupportedServiceCategory_GUID, \
            0x791ad19f,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmSupportedServiceCategory
{
    // Bit mask defining the service categories supported by the hardware.
    ULONG NdisAtmSupportedServiceCategory;
    #define MSNdis_AtmSupportedServiceCategory_NdisAtmSupportedServiceCategory_SIZE sizeof(ULONG)
    #define MSNdis_AtmSupportedServiceCategory_NdisAtmSupportedServiceCategory_ID 1

} MSNdis_AtmSupportedServiceCategory, *PMSNdis_AtmSupportedServiceCategory;

// MSNdis_AtmSupportedAalTypes - MSNdis_AtmSupportedAalTypes
// NDIS ATM Supported AAL Types
#define MSNdis_AtmSupportedAalTypesGuid \
    { 0x791ad1a0,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmSupportedAalTypes_GUID, \
            0x791ad1a0,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmSupportedAalTypes
{
    // Bit mask defining the AAL types supported by the hardware.
    ULONG NdisAtmSupportedAalTypes;
    #define MSNdis_AtmSupportedAalTypes_NdisAtmSupportedAalTypes_SIZE sizeof(ULONG)
    #define MSNdis_AtmSupportedAalTypes_NdisAtmSupportedAalTypes_ID 1

} MSNdis_AtmSupportedAalTypes, *PMSNdis_AtmSupportedAalTypes;

// MSNdis_AtmHardwareCurrentAddress - MSNdis_AtmHardwareCurrentAddress
// NDIS ATM Hardware Current Address
#define MSNdis_AtmHardwareCurrentAddressGuid \
    { 0x791ad1a1,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmHardwareCurrentAddress_GUID, \
            0x791ad1a1,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmHardwareCurrentAddress
{
    // The address of the NIC encoded in the hardware.
    MSNdis_NetworkAddress NdisAtmHardwareCurrentAddress;
    #define MSNdis_AtmHardwareCurrentAddress_NdisAtmHardwareCurrentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_AtmHardwareCurrentAddress_NdisAtmHardwareCurrentAddress_ID 1

} MSNdis_AtmHardwareCurrentAddress, *PMSNdis_AtmHardwareCurrentAddress;

// MSNdis_AtmMaxActiveVcs - MSNdis_AtmMaxActiveVcs
// NDIS ATM Maximum Active VCs
#define MSNdis_AtmMaxActiveVcsGuid \
    { 0x791ad1a2,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxActiveVcs_GUID, \
            0x791ad1a2,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxActiveVcs
{
    // Maximum number of active VCs the adapter can support.
    ULONG NdisAtmMaxActiveVcs;
    #define MSNdis_AtmMaxActiveVcs_NdisAtmMaxActiveVcs_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxActiveVcs_NdisAtmMaxActiveVcs_ID 1

} MSNdis_AtmMaxActiveVcs, *PMSNdis_AtmMaxActiveVcs;

// MSNdis_AtmMaxActiveVciBits - MSNdis_AtmMaxActiveVciBits
// NDIS ATM Maximum Active VCI Bits
#define MSNdis_AtmMaxActiveVciBitsGuid \
    { 0x791ad1a3,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxActiveVciBits_GUID, \
            0x791ad1a3,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxActiveVciBits
{
    // The number of bits controllable in the VCI field of the cell header.
    ULONG NdisAtmMaxActiveVciBits;
    #define MSNdis_AtmMaxActiveVciBits_NdisAtmMaxActiveVciBits_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxActiveVciBits_NdisAtmMaxActiveVciBits_ID 1

} MSNdis_AtmMaxActiveVciBits, *PMSNdis_AtmMaxActiveVciBits;

// MSNdis_AtmMaxActiveVpiBits - MSNdis_AtmMaxActiveVpiBits
// NDIS ATM Maximum Active VPI Bits
#define MSNdis_AtmMaxActiveVpiBitsGuid \
    { 0x791ad1a4,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxActiveVpiBits_GUID, \
            0x791ad1a4,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxActiveVpiBits
{
    // The number of bits controllable in the VPI field of the cell header.
    ULONG NdisAtmMaxActiveVpiBits;
    #define MSNdis_AtmMaxActiveVpiBits_NdisAtmMaxActiveVpiBits_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxActiveVpiBits_NdisAtmMaxActiveVpiBits_ID 1

} MSNdis_AtmMaxActiveVpiBits, *PMSNdis_AtmMaxActiveVpiBits;

// MSNdis_AtmMaxAal0PacketSize - MSNdis_AtmMaxAal0PacketSize
// NDIS ATM Maximum AAL0 Packet Size
#define MSNdis_AtmMaxAal0PacketSizeGuid \
    { 0x791ad1a5,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxAal0PacketSize_GUID, \
            0x791ad1a5,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxAal0PacketSize
{
    // Maximum supported size for AAL0 packets.
    ULONG NdisAtmMaxAal0PacketSize;
    #define MSNdis_AtmMaxAal0PacketSize_NdisAtmMaxAal0PacketSize_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxAal0PacketSize_NdisAtmMaxAal0PacketSize_ID 1

} MSNdis_AtmMaxAal0PacketSize, *PMSNdis_AtmMaxAal0PacketSize;

// MSNdis_AtmMaxAal1PacketSize - MSNdis_AtmMaxAal1PacketSize
// NDIS ATM Maximum AAL1 Packet Size
#define MSNdis_AtmMaxAal1PacketSizeGuid \
    { 0x791ad1a6,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxAal1PacketSize_GUID, \
            0x791ad1a6,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxAal1PacketSize
{
    // Maximum supported size for AAL1 packets.
    ULONG NdisAtmMaxAal1PacketSize;
    #define MSNdis_AtmMaxAal1PacketSize_NdisAtmMaxAal1PacketSize_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxAal1PacketSize_NdisAtmMaxAal1PacketSize_ID 1

} MSNdis_AtmMaxAal1PacketSize, *PMSNdis_AtmMaxAal1PacketSize;

// MSNdis_AtmMaxAal34PacketSize - MSNdis_AtmMaxAal34PacketSize
// NDIS ATM Maximum AAL3/4 Packet Size
#define MSNdis_AtmMaxAal34PacketSizeGuid \
    { 0x791ad1a7,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxAal34PacketSize_GUID, \
            0x791ad1a7,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxAal34PacketSize
{
    // Maximum supported size for AAL3/4 packets.
    ULONG NdisAtmMaxAal34PacketSize;
    #define MSNdis_AtmMaxAal34PacketSize_NdisAtmMaxAal34PacketSize_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxAal34PacketSize_NdisAtmMaxAal34PacketSize_ID 1

} MSNdis_AtmMaxAal34PacketSize, *PMSNdis_AtmMaxAal34PacketSize;

// MSNdis_AtmMaxAal5PacketSize - MSNdis_AtmMaxAal5PacketSize
// NDIS ATM Maximum AAL5 Packet Size
#define MSNdis_AtmMaxAal5PacketSizeGuid \
    { 0x791ad191,0xe35c,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmMaxAal5PacketSize_GUID, \
            0x791ad191,0xe35c,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmMaxAal5PacketSize
{
    // Maximum supported size for AAL5 packets.
    ULONG NdisAtmMaxAal5PacketSize;
    #define MSNdis_AtmMaxAal5PacketSize_NdisAtmMaxAal5PacketSize_SIZE sizeof(ULONG)
    #define MSNdis_AtmMaxAal5PacketSize_NdisAtmMaxAal5PacketSize_ID 1

} MSNdis_AtmMaxAal5PacketSize, *PMSNdis_AtmMaxAal5PacketSize;

// MSNdis_AtmReceiveCellsOk - MSNdis_AtmReceiveCellsOk
// NDIS ATM Receive Cells OK
#define MSNdis_AtmReceiveCellsOkGuid \
    { 0x0a21480a,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmReceiveCellsOk_GUID, \
            0x0a21480a,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmReceiveCellsOk
{
    // The number of cells the NIC has received without errors.
    ULONGLONG NdisAtmReceiveCellsOk;
    #define MSNdis_AtmReceiveCellsOk_NdisAtmReceiveCellsOk_SIZE sizeof(ULONGLONG)
    #define MSNdis_AtmReceiveCellsOk_NdisAtmReceiveCellsOk_ID 1

} MSNdis_AtmReceiveCellsOk, *PMSNdis_AtmReceiveCellsOk;

// MSNdis_AtmTransmitCellsOk - MSNdis_AtmTransmitCellsOk
// NDIS ATM Transmit Cells OK
#define MSNdis_AtmTransmitCellsOkGuid \
    { 0x0a21480b,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmTransmitCellsOk_GUID, \
            0x0a21480b,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmTransmitCellsOk
{
    // The number of cells the NIC has transmitted without errors.
    ULONGLONG NdisAtmTransmitCellsOk;
    #define MSNdis_AtmTransmitCellsOk_NdisAtmTransmitCellsOk_SIZE sizeof(ULONGLONG)
    #define MSNdis_AtmTransmitCellsOk_NdisAtmTransmitCellsOk_ID 1

} MSNdis_AtmTransmitCellsOk, *PMSNdis_AtmTransmitCellsOk;

// MSNdis_AtmReceiveCellsDropped - MSNdis_AtmReceiveCellsDropped
// NDIS ATM Receive Cells Dropped
#define MSNdis_AtmReceiveCellsDroppedGuid \
    { 0x0a21480c,0xe35f,0x11d0, { 0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_AtmReceiveCellsDropped_GUID, \
            0x0a21480c,0xe35f,0x11d0,0x96,0x92,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_AtmReceiveCellsDropped
{
    // The number of receive cells the NIC has dropped.
    ULONGLONG NdisAtmReceiveCellsDropped;
    #define MSNdis_AtmReceiveCellsDropped_NdisAtmReceiveCellsDropped_SIZE sizeof(ULONGLONG)
    #define MSNdis_AtmReceiveCellsDropped_NdisAtmReceiveCellsDropped_ID 1

} MSNdis_AtmReceiveCellsDropped, *PMSNdis_AtmReceiveCellsDropped;

// MSNdis_EthernetPermanentAddress - MSNdis_EthernetPermanentAddress
// NDIS Ethernet Permanent Address
#define MSNdis_EthernetPermanentAddressGuid \
    { 0x447956ff,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetPermanentAddress_GUID, \
            0x447956ff,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetPermanentAddress
{
    // The address of the NIC encoded in the hardware.
    MSNdis_NetworkAddress NdisPermanentAddress;
    #define MSNdis_EthernetPermanentAddress_NdisPermanentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_EthernetPermanentAddress_NdisPermanentAddress_ID 1

} MSNdis_EthernetPermanentAddress, *PMSNdis_EthernetPermanentAddress;

// MSNdis_EthernetCurrentAddress - MSNdis_EthernetCurrentAddress
// NDIS Ethernet Current Address
#define MSNdis_EthernetCurrentAddressGuid \
    { 0x44795700,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetCurrentAddress_GUID, \
            0x44795700,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetCurrentAddress
{
    // The address the NIC is currently using.
    MSNdis_NetworkAddress NdisCurrentAddress;
    #define MSNdis_EthernetCurrentAddress_NdisCurrentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_EthernetCurrentAddress_NdisCurrentAddress_ID 1

} MSNdis_EthernetCurrentAddress, *PMSNdis_EthernetCurrentAddress;

// MSNdis_EthernetMulticastList - MSNdis_EthernetMulticastList
// NDIS Ethernet Multicast List
#define MSNdis_EthernetMulticastListGuid \
    { 0x44795701,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetMulticastList_GUID, \
            0x44795701,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetMulticastList
{
    // Number of multicast addresses enabled on the NIC.
    ULONG NumberElements;
    #define MSNdis_EthernetMulticastList_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_EthernetMulticastList_NumberElements_ID 1

    // The multicast address list on the NIC enabled for packet reception.
    MSNdis_NetworkAddress NdisMulticastList[1];
    #define MSNdis_EthernetMulticastList_NdisMulticastList_ID 2

} MSNdis_EthernetMulticastList, *PMSNdis_EthernetMulticastList;

// MSNdis_EthernetMaximumMulticastListSize - MSNdis_EthernetMaximumMulticastListSize
// Adpater Ethernet Maximum Multicast List Size
#define MSNdis_EthernetMaximumMulticastListSizeGuid \
    { 0x44795702,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetMaximumMulticastListSize_GUID, \
            0x44795702,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetMaximumMulticastListSize
{
    // The maximum number of multicast addresses the NIC driver can manage.
    ULONG NdisEthernetMaximumMulticastListSize;
    #define MSNdis_EthernetMaximumMulticastListSize_NdisEthernetMaximumMulticastListSize_SIZE sizeof(ULONG)
    #define MSNdis_EthernetMaximumMulticastListSize_NdisEthernetMaximumMulticastListSize_ID 1

} MSNdis_EthernetMaximumMulticastListSize, *PMSNdis_EthernetMaximumMulticastListSize;

// MSNdis_EthernetMacOptions - MSNdis_EthernetMacOptions
// NDIS Ethernet MAC Options
#define MSNdis_EthernetMacOptionsGuid \
    { 0x44795703,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetMacOptions_GUID, \
            0x44795703,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetMacOptions
{
    // Features supported by the underlying driver, which could be emulating Ethernet.
    ULONG NdisEthernetMacOptions;
    #define MSNdis_EthernetMacOptions_NdisEthernetMacOptions_SIZE sizeof(ULONG)
    #define MSNdis_EthernetMacOptions_NdisEthernetMacOptions_ID 1

} MSNdis_EthernetMacOptions, *PMSNdis_EthernetMacOptions;

// MSNdis_EthernetReceiveErrorAlignment - MSNdis_EthernetReceiveErrorAlignment
// NDIS Ethernet Receive Error Alignment
#define MSNdis_EthernetReceiveErrorAlignmentGuid \
    { 0x44795704,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetReceiveErrorAlignment_GUID, \
            0x44795704,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetReceiveErrorAlignment
{
    // The number of frames received with alignment errors.
    ULONG NdisEthernetReceiveErrorAlignment;
    #define MSNdis_EthernetReceiveErrorAlignment_NdisEthernetReceiveErrorAlignment_SIZE sizeof(ULONG)
    #define MSNdis_EthernetReceiveErrorAlignment_NdisEthernetReceiveErrorAlignment_ID 1

} MSNdis_EthernetReceiveErrorAlignment, *PMSNdis_EthernetReceiveErrorAlignment;

// MSNdis_EthernetOneTransmitCollision - MSNdis_EthernetOneTransmitCollision
// NDIS Ethernet One Transmit collision
#define MSNdis_EthernetOneTransmitCollisionGuid \
    { 0x44795705,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetOneTransmitCollision_GUID, \
            0x44795705,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetOneTransmitCollision
{
    // The number of frames successfully transmitted after exactly one collision.
    ULONG NdisEthernetOneTransmitCollision;
    #define MSNdis_EthernetOneTransmitCollision_NdisEthernetOneTransmitCollision_SIZE sizeof(ULONG)
    #define MSNdis_EthernetOneTransmitCollision_NdisEthernetOneTransmitCollision_ID 1

} MSNdis_EthernetOneTransmitCollision, *PMSNdis_EthernetOneTransmitCollision;

// MSNdis_EthernetMoreTransmitCollisions - MSNdis_EthernetMoreTransmitCollisions
// NDIS Ethernet More Transmit collisions
#define MSNdis_EthernetMoreTransmitCollisionsGuid \
    { 0x44795706,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_EthernetMoreTransmitCollisions_GUID, \
            0x44795706,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_EthernetMoreTransmitCollisions
{
    // The number of frames successfully transmitted after more than one collision.
    ULONG NdisEthernetMoreTransmitCollisions;
    #define MSNdis_EthernetMoreTransmitCollisions_NdisEthernetMoreTransmitCollisions_SIZE sizeof(ULONG)
    #define MSNdis_EthernetMoreTransmitCollisions_NdisEthernetMoreTransmitCollisions_ID 1

} MSNdis_EthernetMoreTransmitCollisions, *PMSNdis_EthernetMoreTransmitCollisions;

// MSNdis_TokenRingPermanentAddress - MSNdis_TokenRingPermanentAddress
// NDIS Token Ring Permanent Address
#define MSNdis_TokenRingPermanentAddressGuid \
    { 0x44795707,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingPermanentAddress_GUID, \
            0x44795707,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingPermanentAddress
{
    // The address of the NIC encoded in the hardware.
    MSNdis_NetworkAddress NdisPermanentAddress;
    #define MSNdis_TokenRingPermanentAddress_NdisPermanentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_TokenRingPermanentAddress_NdisPermanentAddress_ID 1

} MSNdis_TokenRingPermanentAddress, *PMSNdis_TokenRingPermanentAddress;

// MSNdis_TokenRingCurrentAddress - MSNdis_TokenRingCurrentAddress
// NDIS Token Ring Current Address
#define MSNdis_TokenRingCurrentAddressGuid \
    { 0x44795708,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingCurrentAddress_GUID, \
            0x44795708,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingCurrentAddress
{
    // The address the NIC is currently using.
    MSNdis_NetworkAddress NdisCurrentAddress;
    #define MSNdis_TokenRingCurrentAddress_NdisCurrentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_TokenRingCurrentAddress_NdisCurrentAddress_ID 1

} MSNdis_TokenRingCurrentAddress, *PMSNdis_TokenRingCurrentAddress;

// MSNdis_TokenRingCurrentFunctional - MSNdis_TokenRingCurrentFunctional
// NDIS Token Ring Current Functional Address
#define MSNdis_TokenRingCurrentFunctionalGuid \
    { 0x44795709,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingCurrentFunctional_GUID, \
            0x44795709,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingCurrentFunctional
{
    // The functional address enabled on the NIC for packet reception.
    ULONG NdisTokenRingCurrentFunctional;
    #define MSNdis_TokenRingCurrentFunctional_NdisTokenRingCurrentFunctional_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingCurrentFunctional_NdisTokenRingCurrentFunctional_ID 1

} MSNdis_TokenRingCurrentFunctional, *PMSNdis_TokenRingCurrentFunctional;

// MSNdis_TokenRingCurrentGroup - MSNdis_TokenRingCurrentGroup
// NDIS Token Ring Current Group Address
#define MSNdis_TokenRingCurrentGroupGuid \
    { 0x4479570a,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingCurrentGroup_GUID, \
            0x4479570a,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingCurrentGroup
{
    // The group address enabled on the NIC for packet reception.
    ULONG NdisTokenRingCurrentGroup;
    #define MSNdis_TokenRingCurrentGroup_NdisTokenRingCurrentGroup_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingCurrentGroup_NdisTokenRingCurrentGroup_ID 1

} MSNdis_TokenRingCurrentGroup, *PMSNdis_TokenRingCurrentGroup;

// MSNdis_TokenRingLastOpenStatus - MSNdis_TokenRingLastOpenStatus
// NDIS Token Ring Last Open Status
#define MSNdis_TokenRingLastOpenStatusGuid \
    { 0x4479570b,0xa61b,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingLastOpenStatus_GUID, \
            0x4479570b,0xa61b,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingLastOpenStatus
{
    // The last open error status returned for a protocol's call to NdisOpenAdapter.
    ULONG NdisTokenRingLastOpenStatus;
    #define MSNdis_TokenRingLastOpenStatus_NdisTokenRingLastOpenStatus_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingLastOpenStatus_NdisTokenRingLastOpenStatus_ID 1

} MSNdis_TokenRingLastOpenStatus, *PMSNdis_TokenRingLastOpenStatus;

// MSNdis_TokenRingCurrentRingStatus - MSNdis_TokenRingCurrentRingStatus
// NDIS Token Ring Current Ring Status
#define MSNdis_TokenRingCurrentRingStatusGuid \
    { 0x890a36ec,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingCurrentRingStatus_GUID, \
            0x890a36ec,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingCurrentRingStatus
{
    // The last ring status indicated with an NDIS_RING_XXX status code.
    ULONG NdisTokenRingCurrentRingStatus;
    #define MSNdis_TokenRingCurrentRingStatus_NdisTokenRingCurrentRingStatus_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingCurrentRingStatus_NdisTokenRingCurrentRingStatus_ID 1

} MSNdis_TokenRingCurrentRingStatus, *PMSNdis_TokenRingCurrentRingStatus;

// MSNdis_TokenRingCurrentRingState - MSNdis_TokenRingCurrentRingState
// NDIS Token Ring Current Ring State.
#define MSNdis_TokenRingCurrentRingStateGuid \
    { 0xacf14032,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingCurrentRingState_GUID, \
            0xacf14032,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingCurrentRingState
{
    // The state of the NIC driver with repsect to entering the ring.
    ULONG NdisTokenRingCurrentRingState;
    #define MSNdis_TokenRingCurrentRingState_NdisTokenRingCurrentRingState_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingCurrentRingState_NdisTokenRingCurrentRingState_ID 1

} MSNdis_TokenRingCurrentRingState, *PMSNdis_TokenRingCurrentRingState;

// MSNdis_TokenRingLineErrors - MSNdis_TokenRingLineErrors
// NDIS Token Ring Line Errors
#define MSNdis_TokenRingLineErrorsGuid \
    { 0xacf14033,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingLineErrors_GUID, \
            0xacf14033,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingLineErrors
{
    // Number of frames with an invalid FCS or a code violation.
    ULONG NdisTokenRingLineErrors;
    #define MSNdis_TokenRingLineErrors_NdisTokenRingLineErrors_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingLineErrors_NdisTokenRingLineErrors_ID 1

} MSNdis_TokenRingLineErrors, *PMSNdis_TokenRingLineErrors;

// MSNdis_TokenRingLostFrames - MSNdis_TokenRingLostFrames
// NDIS Token Ring Lost Frames
#define MSNdis_TokenRingLostFramesGuid \
    { 0xacf14034,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_TokenRingLostFrames_GUID, \
            0xacf14034,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_TokenRingLostFrames
{
    // The number of frames transmitted that have not circled the ring within the maximum ring latency.
    ULONG NdisTokenRingLostFrames;
    #define MSNdis_TokenRingLostFrames_NdisTokenRingLostFrames_SIZE sizeof(ULONG)
    #define MSNdis_TokenRingLostFrames_NdisTokenRingLostFrames_ID 1

} MSNdis_TokenRingLostFrames, *PMSNdis_TokenRingLostFrames;

// MSNdis_FddiLongPermanentAddress - MSNdis_FddiLongPermanentAddress
// NDIS FDDI Long Permanent Address
#define MSNdis_FddiLongPermanentAddressGuid \
    { 0xacf14035,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLongPermanentAddress_GUID, \
            0xacf14035,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLongPermanentAddress
{
    // The long address of the NIC encoded in the hardware.
    MSNdis_NetworkAddress NdisPermanentAddress;
    #define MSNdis_FddiLongPermanentAddress_NdisPermanentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_FddiLongPermanentAddress_NdisPermanentAddress_ID 1

} MSNdis_FddiLongPermanentAddress, *PMSNdis_FddiLongPermanentAddress;

// MSNdis_FddiLongCurrentAddress - MSNdis_FddiLongCurrentAddress
// NDIS FDDI Long Current Address
#define MSNdis_FddiLongCurrentAddressGuid \
    { 0xacf14036,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLongCurrentAddress_GUID, \
            0xacf14036,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLongCurrentAddress
{
    // The long address the NIC is currently using.
    MSNdis_NetworkAddress NdisCurrentAddress;
    #define MSNdis_FddiLongCurrentAddress_NdisCurrentAddress_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_FddiLongCurrentAddress_NdisCurrentAddress_ID 1

} MSNdis_FddiLongCurrentAddress, *PMSNdis_FddiLongCurrentAddress;

// MSNdis_FddiLongMulticastList - MSNdis_FddiLongMulticastList
// NDIS FDDI Long Multicast List
#define MSNdis_FddiLongMulticastListGuid \
    { 0xacf14037,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLongMulticastList_GUID, \
            0xacf14037,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLongMulticastList
{
    // Number of multicast addresses enabled on the NIC.
    ULONG NumberElements;
    #define MSNdis_FddiLongMulticastList_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_FddiLongMulticastList_NumberElements_ID 1

    // The multicast long address list on the NIC enabled for packet reception.
    MSNdis_NetworkAddress NdisMulticastList[1];
    #define MSNdis_FddiLongMulticastList_NdisMulticastList_ID 2

} MSNdis_FddiLongMulticastList, *PMSNdis_FddiLongMulticastList;

// MSNdis_FddiLongMaximumListSize - MSNdis_FddiLongMaximumListSize
// NDIS FDDI Long Maximum List Size
#define MSNdis_FddiLongMaximumListSizeGuid \
    { 0xacf14038,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLongMaximumListSize_GUID, \
            0xacf14038,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLongMaximumListSize
{
    // The maximum number of multicast long addresses the NIC driver can manage.
    ULONG NdisFddiLongMaximumListSize;
    #define MSNdis_FddiLongMaximumListSize_NdisFddiLongMaximumListSize_SIZE sizeof(ULONG)
    #define MSNdis_FddiLongMaximumListSize_NdisFddiLongMaximumListSize_ID 1

} MSNdis_FddiLongMaximumListSize, *PMSNdis_FddiLongMaximumListSize;

// MSNdis_FddiShortPermanentAddress - MSNdis_FddiShortPermanentAddress
// NDIS FDDI Short Permanent Address
#define MSNdis_FddiShortPermanentAddressGuid \
    { 0xacf14039,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiShortPermanentAddress_GUID, \
            0xacf14039,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiShortPermanentAddress
{
    // The short address of the NIC encoded in the hardware.
    MSNdis_NetworkShortAddress NdisPermanentAddress;
    #define MSNdis_FddiShortPermanentAddress_NdisPermanentAddress_SIZE sizeof(MSNdis_NetworkShortAddress)
    #define MSNdis_FddiShortPermanentAddress_NdisPermanentAddress_ID 1

} MSNdis_FddiShortPermanentAddress, *PMSNdis_FddiShortPermanentAddress;

// MSNdis_FddiShortCurrentAddress - MSNdis_FddiShortCurrentAddress
// NDIS FDDI Short Current Address
#define MSNdis_FddiShortCurrentAddressGuid \
    { 0xacf1403a,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiShortCurrentAddress_GUID, \
            0xacf1403a,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiShortCurrentAddress
{
    // The short address the NIC is currently using.
    MSNdis_NetworkShortAddress NdisCurrentAddress;
    #define MSNdis_FddiShortCurrentAddress_NdisCurrentAddress_SIZE sizeof(MSNdis_NetworkShortAddress)
    #define MSNdis_FddiShortCurrentAddress_NdisCurrentAddress_ID 1

} MSNdis_FddiShortCurrentAddress, *PMSNdis_FddiShortCurrentAddress;

// MSNdis_FddiShortMulticastList - MSNdis_FddiShortMulticastList
// NDIS FDDI Short Multicast List
#define MSNdis_FddiShortMulticastListGuid \
    { 0xacf1403b,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiShortMulticastList_GUID, \
            0xacf1403b,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiShortMulticastList
{
    // Number of multicast short addresses enabled on the NIC.
    ULONG NumberElements;
    #define MSNdis_FddiShortMulticastList_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_FddiShortMulticastList_NumberElements_ID 1

    // The multicast short address list on the NIC enabled for packet reception.
    MSNdis_NetworkShortAddress NdisMulticastList[1];
    #define MSNdis_FddiShortMulticastList_NdisMulticastList_ID 2

} MSNdis_FddiShortMulticastList, *PMSNdis_FddiShortMulticastList;

// MSNdis_FddiShortMaximumListSize - MSNdis_FddiShortMaximumListSize
// NDIS FDDI Short Maximum List Size
#define MSNdis_FddiShortMaximumListSizeGuid \
    { 0xacf1403c,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiShortMaximumListSize_GUID, \
            0xacf1403c,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiShortMaximumListSize
{
    // The maximum number of multicast short addresses the NIC driver can manage.
    ULONG NdisFddiShortMaximumListSize;
    #define MSNdis_FddiShortMaximumListSize_NdisFddiShortMaximumListSize_SIZE sizeof(ULONG)
    #define MSNdis_FddiShortMaximumListSize_NdisFddiShortMaximumListSize_ID 1

} MSNdis_FddiShortMaximumListSize, *PMSNdis_FddiShortMaximumListSize;

// MSNdis_FddiAttachmentType - MSNdis_FddiAttachmentType
// NDIS FDDI Attachment Type
#define MSNdis_FddiAttachmentTypeGuid \
    { 0xacf1403d,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiAttachmentType_GUID, \
            0xacf1403d,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiAttachmentType
{
    // Defines the attachment of the NIC to the network.
    ULONG NdisFddiAttachmentType;
    #define MSNdis_FddiAttachmentType_NdisFddiAttachmentType_SIZE sizeof(ULONG)
    #define MSNdis_FddiAttachmentType_NdisFddiAttachmentType_ID 1

} MSNdis_FddiAttachmentType, *PMSNdis_FddiAttachmentType;

// MSNdis_FddiUpstreamNodeLong - MSNdis_FddiUpstreamNodeLong
// NDIS FDDI Upstream Node Long
#define MSNdis_FddiUpstreamNodeLongGuid \
    { 0xacf1403e,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiUpstreamNodeLong_GUID, \
            0xacf1403e,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiUpstreamNodeLong
{
    // The long address of the station above this NIC on the ring or zero if the address is unknown.
    MSNdis_NetworkAddress NdisFddiUpstreamNodeLong;
    #define MSNdis_FddiUpstreamNodeLong_NdisFddiUpstreamNodeLong_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_FddiUpstreamNodeLong_NdisFddiUpstreamNodeLong_ID 1

} MSNdis_FddiUpstreamNodeLong, *PMSNdis_FddiUpstreamNodeLong;

// MSNdis_FddiDownstreamNodeLong - MSNdis_FddiDownstreamNodeLong
// NDIS FDDI Downstream Node Long
#define MSNdis_FddiDownstreamNodeLongGuid \
    { 0xacf1403f,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiDownstreamNodeLong_GUID, \
            0xacf1403f,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiDownstreamNodeLong
{
    // The long address of the station below this NIC on the ring or zero if the address is unknown.
    MSNdis_NetworkAddress NdisFddiDownstreamNodeLong;
    #define MSNdis_FddiDownstreamNodeLong_NdisFddiDownstreamNodeLong_SIZE sizeof(MSNdis_NetworkAddress)
    #define MSNdis_FddiDownstreamNodeLong_NdisFddiDownstreamNodeLong_ID 1

} MSNdis_FddiDownstreamNodeLong, *PMSNdis_FddiDownstreamNodeLong;

// MSNdis_FddiFrameErrors - MSNdis_FddiFrameErrors
// NDIS FDDI Frame Errors
#define MSNdis_FddiFrameErrorsGuid \
    { 0xacf14040,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiFrameErrors_GUID, \
            0xacf14040,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiFrameErrors
{
    // The number of frames detected in error by this NIC that have not been detected in error by another device on the ring.
    ULONG NdisFddiFrameErrors;
    #define MSNdis_FddiFrameErrors_NdisFddiFrameErrors_SIZE sizeof(ULONG)
    #define MSNdis_FddiFrameErrors_NdisFddiFrameErrors_ID 1

} MSNdis_FddiFrameErrors, *PMSNdis_FddiFrameErrors;

// MSNdis_FddiFramesLost - MSNdis_FddiFramesLost
// NDIS FDDI Frames Lost
#define MSNdis_FddiFramesLostGuid \
    { 0xacf14041,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiFramesLost_GUID, \
            0xacf14041,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiFramesLost
{
    // The number of times this NIC detected a format error during frame reception such that the frame was stripped.
    ULONG NdisFddiFramesLost;
    #define MSNdis_FddiFramesLost_NdisFddiFramesLost_SIZE sizeof(ULONG)
    #define MSNdis_FddiFramesLost_NdisFddiFramesLost_ID 1

} MSNdis_FddiFramesLost, *PMSNdis_FddiFramesLost;

// MSNdis_FddiRingManagmentState - MSNdis_FddiRingManagmentState
// NDIS FDDI Ring Management State
#define MSNdis_FddiRingManagmentStateGuid \
    { 0xacf14042,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiRingManagmentState_GUID, \
            0xacf14042,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiRingManagmentState
{
    // Defines the current state of the Ring Management state machine.
    ULONG NdisFddiRingManagmentState;
    #define MSNdis_FddiRingManagmentState_NdisFddiRingManagmentState_SIZE sizeof(ULONG)
    #define MSNdis_FddiRingManagmentState_NdisFddiRingManagmentState_ID 1

} MSNdis_FddiRingManagmentState, *PMSNdis_FddiRingManagmentState;

// MSNdis_FddiLctFailures - MSNdis_FddiLctFailures
// NDIS FDDI LCT Failures
#define MSNdis_FddiLctFailuresGuid \
    { 0xacf14043,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLctFailures_GUID, \
            0xacf14043,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLctFailures
{
    // The count of the consecutive times the link confidence test (LCT) has failed during connection management.
    ULONG NdisFddiLctFailures;
    #define MSNdis_FddiLctFailures_NdisFddiLctFailures_SIZE sizeof(ULONG)
    #define MSNdis_FddiLctFailures_NdisFddiLctFailures_ID 1

} MSNdis_FddiLctFailures, *PMSNdis_FddiLctFailures;

// MSNdis_FddiLemRejects - MSNdis_FddiLemRejects
// NDIS FDDI LEM Rejects
#define MSNdis_FddiLemRejectsGuid \
    { 0xacf14044,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLemRejects_GUID, \
            0xacf14044,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLemRejects
{
    // The link error monitor (LEM) count of times that a link was rejected.
    ULONG NdisFddiLemRejects;
    #define MSNdis_FddiLemRejects_NdisFddiLemRejects_SIZE sizeof(ULONG)
    #define MSNdis_FddiLemRejects_NdisFddiLemRejects_ID 1

} MSNdis_FddiLemRejects, *PMSNdis_FddiLemRejects;

// MSNdis_FddiLConnectionState - MSNdis_FddiLConnectionState
// NDIS FDDI LConnect State
#define MSNdis_FddiLConnectionStateGuid \
    { 0xacf14045,0xa61c,0x11d0, { 0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_FddiLConnectionState_GUID, \
            0xacf14045,0xa61c,0x11d0,0x8d,0xd4,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_FddiLConnectionState
{
    // Defines the state of this port's Physical Connection Management (PCM) state machine.
    ULONG NdisFddiLConnectionState;
    #define MSNdis_FddiLConnectionState_NdisFddiLConnectionState_SIZE sizeof(ULONG)
    #define MSNdis_FddiLConnectionState_NdisFddiLConnectionState_ID 1

} MSNdis_FddiLConnectionState, *PMSNdis_FddiLConnectionState;

// MSNdis_StatusResetStart - MSNdis_StatusResetStart
// NDIS Status Reset Start
#define MSNdis_StatusResetStartGuid \
    { 0x981f2d76,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusResetStart_GUID, \
            0x981f2d76,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_StatusResetStart cannot be created
typedef struct _MSNdis_StatusResetStart
{
    char VariableData[1];

} MSNdis_StatusResetStart, *PMSNdis_StatusResetStart;

// MSNdis_StatusResetEnd - MSNdis_StatusResetEnd
// NDIS Status Reset End
#define MSNdis_StatusResetEndGuid \
    { 0x981f2d77,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusResetEnd_GUID, \
            0x981f2d77,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_StatusResetEnd cannot be created
typedef struct _MSNdis_StatusResetEnd
{
    char VariableData[1];

} MSNdis_StatusResetEnd, *PMSNdis_StatusResetEnd;

// MSNdis_StatusMediaConnect - MSNdis_StatusMediaConnect
// NDIS Status Media Connect
#define MSNdis_StatusMediaConnectGuid \
    { 0x981f2d7d,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusMediaConnect_GUID, \
            0x981f2d7d,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_StatusMediaConnect cannot be created
typedef struct _MSNdis_StatusMediaConnect
{
    char VariableData[1];

} MSNdis_StatusMediaConnect, *PMSNdis_StatusMediaConnect;

// MSNdis_StatusMediaDisconnect - MSNdis_StatusMediaDisconnect
// NDIS Status Media Disconnect
#define MSNdis_StatusMediaDisconnectGuid \
    { 0x981f2d7e,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusMediaDisconnect_GUID, \
            0x981f2d7e,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


// Warning: Header for class MSNdis_StatusMediaDisconnect cannot be created
typedef struct _MSNdis_StatusMediaDisconnect
{
    char VariableData[1];

} MSNdis_StatusMediaDisconnect, *PMSNdis_StatusMediaDisconnect;

// MSNdis_StatusMediaSpecificIndication - MSNdis_StatusMediaSpecificIndication
// NDIS Status Media Specific Indication
#define MSNdis_StatusMediaSpecificIndicationGuid \
    { 0x981f2d84,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusMediaSpecificIndication_GUID, \
            0x981f2d84,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_StatusMediaSpecificIndication
{
    // Number of bytes for media specific status indication
    ULONG NumberElements;
    #define MSNdis_StatusMediaSpecificIndication_NumberElements_SIZE sizeof(ULONG)
    #define MSNdis_StatusMediaSpecificIndication_NumberElements_ID 1

    // Media specific status information.
    UCHAR NdisStatusMediaSpecificIndication[1];
    #define MSNdis_StatusMediaSpecificIndication_NdisStatusMediaSpecificIndication_ID 2

} MSNdis_StatusMediaSpecificIndication, *PMSNdis_StatusMediaSpecificIndication;

// MSNdis_StatusLinkSpeedChange - MSNdis_StatusLinkSpeedChange
// NDIS Status Link Speed Change
#define MSNdis_StatusLinkSpeedChangeGuid \
    { 0x981f2d85,0xb1f3,0x11d0, { 0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusLinkSpeedChange_GUID, \
            0x981f2d85,0xb1f3,0x11d0,0x8d,0xd7,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_StatusLinkSpeedChange
{
    // New inbound and outbound link speeds for the adapter.
    MSNdis_NetworkLinkSpeed NdisStatusLinkSpeedChange;
    #define MSNdis_StatusLinkSpeedChange_NdisStatusLinkSpeedChange_SIZE sizeof(MSNdis_NetworkLinkSpeed)
    #define MSNdis_StatusLinkSpeedChange_NdisStatusLinkSpeedChange_ID 1

} MSNdis_StatusLinkSpeedChange, *PMSNdis_StatusLinkSpeedChange;

// MSNdis_StatusProtocolBind - MSNdis_StatusProtocolBind
// NDIS Protocol Bind Notification
#define MSNdis_StatusProtocolBindGuid \
    { 0x5413531c,0xb1f3,0x11d0, { 0xd7,0x8d,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusProtocolBind_GUID, \
            0x5413531c,0xb1f3,0x11d0,0xd7,0x8d,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_StatusProtocolBind
{
    // Name of transport bound
    CHAR VariableData[1];
    #define MSNdis_StatusProtocolBind_Transport_ID 1

} MSNdis_StatusProtocolBind, *PMSNdis_StatusProtocolBind;

// MSNdis_StatusProtocolUnbind - MSNdis_StatusProtocolUnbind
// NDIS Protocol Unbind Notification
#define MSNdis_StatusProtocolUnbindGuid \
    { 0x6e3ce1ec,0xb1f3,0x11d0, { 0xd7,0x8d,0x00,0xc0,0x4f,0xc3,0x35,0x8c } }

DEFINE_GUID(MSNdis_StatusProtocolUnbind_GUID, \
            0x6e3ce1ec,0xb1f3,0x11d0,0xd7,0x8d,0x00,0xc0,0x4f,0xc3,0x35,0x8c);


typedef struct _MSNdis_StatusProtocolUnbind
{
    // Name of transport unbound
    CHAR VariableData[1];
    #define MSNdis_StatusProtocolUnbind_Transport_ID 1

} MSNdis_StatusProtocolUnbind, *PMSNdis_StatusProtocolUnbind;

// MSKeyboard_PortInformation - KEYBOARD_PORT_WMI_STD_DATA
// Keyboard port driver information
#define KEYBOARD_PORT_WMI_STD_DATA_GUID \
    { 0x4731f89a,0x71cb,0x11d1, { 0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSKeyboard_PortInformation_GUID, \
            0x4731f89a,0x71cb,0x11d1,0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _KEYBOARD_PORT_WMI_STD_DATA
{

// I8042 ConnectorSerial Connector
#define KEYBOARD_PORT_WMI_STD_I8042 0
// USB Connector
#define KEYBOARD_PORT_WMI_STD_SERIAL 1
#define KEYBOARD_PORT_WMI_STD_USB 2

    // How the keyboard is connected to the computer
    ULONG ConnectorType;
    #define KEYBOARD_PORT_WMI_STD_DATA_ConnectorType_SIZE sizeof(ULONG)
    #define KEYBOARD_PORT_WMI_STD_DATA_ConnectorType_ID 1

    // The DataQueueSize property indicates the size of the data queue.
    ULONG DataQueueSize;
    #define KEYBOARD_PORT_WMI_STD_DATA_DataQueueSize_SIZE sizeof(ULONG)
    #define KEYBOARD_PORT_WMI_STD_DATA_DataQueueSize_ID 2

    // Number of errors that occured on this device
    ULONG ErrorCount;
    #define KEYBOARD_PORT_WMI_STD_DATA_ErrorCount_SIZE sizeof(ULONG)
    #define KEYBOARD_PORT_WMI_STD_DATA_ErrorCount_ID 3

    // The NumberOfFunctionKeys property indicates the number of function keys on the keyboard.
    ULONG FunctionKeys;
    #define KEYBOARD_PORT_WMI_STD_DATA_FunctionKeys_SIZE sizeof(ULONG)
    #define KEYBOARD_PORT_WMI_STD_DATA_FunctionKeys_ID 4

    // The NumberOfIndicators property indicates the number of indicator leds on the keyboard.
    ULONG Indicators;
    #define KEYBOARD_PORT_WMI_STD_DATA_Indicators_SIZE sizeof(ULONG)
    #define KEYBOARD_PORT_WMI_STD_DATA_Indicators_ID 5

} KEYBOARD_PORT_WMI_STD_DATA, *PKEYBOARD_PORT_WMI_STD_DATA;

// MSMouse_PortInformation - POINTER_PORT_WMI_STD_DATA
// Mouse port driver information
#define POINTER_PORT_WMI_STD_DATA_GUID \
    { 0x4731f89c,0x71cb,0x11d1, { 0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSMouse_PortInformation_GUID, \
            0x4731f89c,0x71cb,0x11d1,0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _POINTER_PORT_WMI_STD_DATA
{

// I8042 ConnectorSerial Connector
#define POINTER_PORT_WMI_STD_I8042 0
// USB Connector
#define POINTER_PORT_WMI_STD_SERIAL 1
#define POINTER_PORT_WMI_STD_USB 2

    // How the mouse is connected to the computer
    ULONG ConnectorType;
    #define POINTER_PORT_WMI_STD_DATA_ConnectorType_SIZE sizeof(ULONG)
    #define POINTER_PORT_WMI_STD_DATA_ConnectorType_ID 1

    // The DataQueueSize property indicates the size of the data queue.
    ULONG DataQueueSize;
    #define POINTER_PORT_WMI_STD_DATA_DataQueueSize_SIZE sizeof(ULONG)
    #define POINTER_PORT_WMI_STD_DATA_DataQueueSize_ID 2

    // Number of errors that occured on this device
    ULONG ErrorCount;
    #define POINTER_PORT_WMI_STD_DATA_ErrorCount_SIZE sizeof(ULONG)
    #define POINTER_PORT_WMI_STD_DATA_ErrorCount_ID 3

    // The NumberOfButtons property indicates the number of buttons on the pointing device.
    ULONG Buttons;
    #define POINTER_PORT_WMI_STD_DATA_Buttons_SIZE sizeof(ULONG)
    #define POINTER_PORT_WMI_STD_DATA_Buttons_ID 4


// Standard Mouse
#define POINTER_PORT_WMI_STD_MOUSE 0
// Standard Pointer
#define POINTER_PORT_WMI_STD_POINTER 1
// Standard Absolute Pointer
#define POINTER_PORT_WMI_ABSOLUTE_POINTER 2
// Tablet
#define POINTER_PORT_WMI_TABLET 3
// Touch Screen
#define POINTER_PORT_WMI_TOUCH_SCRENE 4
// Pen
#define POINTER_PORT_WMI_PEN 5
// Track Ball
#define POINTER_PORT_WMI_TRACK_BALL 6
// Other
#define POINTER_PORT_WMI_OTHER 256

    // The HardwareType property indicates the hardware type of the pointing device.
    ULONG HardwareType;
    #define POINTER_PORT_WMI_STD_DATA_HardwareType_SIZE sizeof(ULONG)
    #define POINTER_PORT_WMI_STD_DATA_HardwareType_ID 5

} POINTER_PORT_WMI_STD_DATA, *PPOINTER_PORT_WMI_STD_DATA;

// MSMouse_ClassInformation - MSMouse_ClassInformation
// Mouse class driver information
#define MSMouse_ClassInformationGuid \
    { 0x4731f89b,0x71cb,0x11d1, { 0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSMouse_ClassInformation_GUID, \
            0x4731f89b,0x71cb,0x11d1,0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSMouse_ClassInformation
{
    // An identification number for the device
    ULONGLONG DeviceId;
    #define MSMouse_ClassInformation_DeviceId_SIZE sizeof(ULONGLONG)
    #define MSMouse_ClassInformation_DeviceId_ID 1

} MSMouse_ClassInformation, *PMSMouse_ClassInformation;

// MSKeyboard_ClassInformation - MSKeyboard_ClassInformation
// Keyboard class driver information
#define MSKeyboard_ClassInformationGuid \
    { 0x4731f899,0x71cb,0x11d1, { 0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSKeyboard_ClassInformation_GUID, \
            0x4731f899,0x71cb,0x11d1,0xa5,0x2c,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSKeyboard_ClassInformation
{
    // An identification number for the device
    ULONGLONG DeviceId;
    #define MSKeyboard_ClassInformation_DeviceId_SIZE sizeof(ULONGLONG)
    #define MSKeyboard_ClassInformation_DeviceId_ID 1

} MSKeyboard_ClassInformation, *PMSKeyboard_ClassInformation;

// MSAcpi_ThermalZoneTemperature - MSAcpi_ThermalZoneTemperature
// ThermalZone temperature information
#define MSAcpi_ThermalZoneTemperatureGuid \
    { 0xa1bc18c0,0xa7c8,0x11d1, { 0xbf,0x3c,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSAcpi_ThermalZoneTemperature_GUID, \
            0xa1bc18c0,0xa7c8,0x11d1,0xbf,0x3c,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSAcpi_ThermalZoneTemperature
{
    // Thermal information change stamp
    ULONG ThermalStamp;
    #define MSAcpi_ThermalZoneTemperature_ThermalStamp_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_ThermalStamp_ID 1

    // First thermal constant
    ULONG ThermalConstant1;
    #define MSAcpi_ThermalZoneTemperature_ThermalConstant1_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_ThermalConstant1_ID 2

    // Second thermal constant
    ULONG ThermalConstant2;
    #define MSAcpi_ThermalZoneTemperature_ThermalConstant2_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_ThermalConstant2_ID 3

    // Reserved
    ULONG Reserved;
    #define MSAcpi_ThermalZoneTemperature_Reserved_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_Reserved_ID 4

    // Sampling period
    ULONG SamplingPeriod;
    #define MSAcpi_ThermalZoneTemperature_SamplingPeriod_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_SamplingPeriod_ID 5

    // Temperature at thermal zone in tenths of degrees Kelvin
    ULONG CurrentTemperature;
    #define MSAcpi_ThermalZoneTemperature_CurrentTemperature_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_CurrentTemperature_ID 6

    // Temperature (in tenths of degrees Kelvin) at which the OS must activate CPU throttling (ie, enable passive cooling)
    ULONG PassiveTripPoint;
    #define MSAcpi_ThermalZoneTemperature_PassiveTripPoint_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_PassiveTripPoint_ID 7

    // Temperature (in tenths of degrees Kelvin) at which the OS must shutdown the system (ie, critical temperature)
    ULONG CriticalTripPoint;
    #define MSAcpi_ThermalZoneTemperature_CriticalTripPoint_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_CriticalTripPoint_ID 8

    // Count of active trip points
    ULONG ActiveTripPointCount;
    #define MSAcpi_ThermalZoneTemperature_ActiveTripPointCount_SIZE sizeof(ULONG)
    #define MSAcpi_ThermalZoneTemperature_ActiveTripPointCount_ID 9

    // Temperature levels (in tenths of degrees Kelvin) at which the OS must activate active cooling
    ULONG ActiveTripPoint[10];
    #define MSAcpi_ThermalZoneTemperature_ActiveTripPoint_SIZE sizeof(ULONG[10])
    #define MSAcpi_ThermalZoneTemperature_ActiveTripPoint_ID 10

} MSAcpi_ThermalZoneTemperature, *PMSAcpi_ThermalZoneTemperature;

// MSDiskDriver_Geometry - WMI_DISK_GEOMETRY
// Disk Geometry
#define MSDiskDriver_GeometryGuid \
    { 0x25007f51,0x57c2,0x11d1, { 0xa5,0x28,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSDiskDriver_Geometry_GUID, \
            0x25007f51,0x57c2,0x11d1,0xa5,0x28,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _WMI_DISK_GEOMETRY
{
    // 
    LONGLONG Cylinders;
    #define WMI_DISK_GEOMETRY_Cylinders_SIZE sizeof(LONGLONG)
    #define WMI_DISK_GEOMETRY_Cylinders_ID 1

    // 
    ULONG MediaType;
    #define WMI_DISK_GEOMETRY_MediaType_SIZE sizeof(ULONG)
    #define WMI_DISK_GEOMETRY_MediaType_ID 2

    // 
    ULONG TracksPerCylinder;
    #define WMI_DISK_GEOMETRY_TracksPerCylinder_SIZE sizeof(ULONG)
    #define WMI_DISK_GEOMETRY_TracksPerCylinder_ID 3

    // 
    ULONG SectorsPerTrack;
    #define WMI_DISK_GEOMETRY_SectorsPerTrack_SIZE sizeof(ULONG)
    #define WMI_DISK_GEOMETRY_SectorsPerTrack_ID 4

    // 
    ULONG BytesPerSector;
    #define WMI_DISK_GEOMETRY_BytesPerSector_SIZE sizeof(ULONG)
    #define WMI_DISK_GEOMETRY_BytesPerSector_ID 5

} WMI_DISK_GEOMETRY, *PWMI_DISK_GEOMETRY;

// MSDiskDriver_PerformanceData - WMI_DISK_PERFORMANCE
// Disk performance statistics
#define MSDiskDriver_PerformanceDataGuid \
    { 0xbdd865d2,0xd7c1,0x11d0, { 0xa5,0x01,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSDiskDriver_PerformanceData_GUID, \
            0xbdd865d2,0xd7c1,0x11d0,0xa5,0x01,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _WMI_DISK_PERFORMANCE
{
    // Number of bytes read on disk
    LONGLONG BytesRead;
    #define WMI_DISK_PERFORMANCE_BytesRead_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_BytesRead_ID 1

    // Number of bytes written on disk
    LONGLONG BytesWritten;
    #define WMI_DISK_PERFORMANCE_BytesWritten_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_BytesWritten_ID 2

    // Amount off time spent reading from disk
    LONGLONG ReadTime;
    #define WMI_DISK_PERFORMANCE_ReadTime_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_ReadTime_ID 3

    // Amount off time spent writing to disk
    LONGLONG WriteTime;
    #define WMI_DISK_PERFORMANCE_WriteTime_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_WriteTime_ID 4

    // Amount off disk idle time
    LONGLONG IdleTime;
    #define WMI_DISK_PERFORMANCE_IdleTime_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_IdleTime_ID 5

    // Number of read operations from disk
    ULONG ReadCount;
    #define WMI_DISK_PERFORMANCE_ReadCount_SIZE sizeof(ULONG)
    #define WMI_DISK_PERFORMANCE_ReadCount_ID 6

    // Number of write operations to disk
    ULONG WriteCount;
    #define WMI_DISK_PERFORMANCE_WriteCount_SIZE sizeof(ULONG)
    #define WMI_DISK_PERFORMANCE_WriteCount_ID 7

    // Number of requests waiting in the disk queue
    ULONG QueueDepth;
    #define WMI_DISK_PERFORMANCE_QueueDepth_SIZE sizeof(ULONG)
    #define WMI_DISK_PERFORMANCE_QueueDepth_ID 8

    // Number of split IO operations
    ULONG SplitCount;
    #define WMI_DISK_PERFORMANCE_SplitCount_SIZE sizeof(ULONG)
    #define WMI_DISK_PERFORMANCE_SplitCount_ID 9

    // 
    LONGLONG QueryTime;
    #define WMI_DISK_PERFORMANCE_QueryTime_SIZE sizeof(LONGLONG)
    #define WMI_DISK_PERFORMANCE_QueryTime_ID 10

    // 
    ULONG StorageDeviceNumber;
    #define WMI_DISK_PERFORMANCE_StorageDeviceNumber_SIZE sizeof(ULONG)
    #define WMI_DISK_PERFORMANCE_StorageDeviceNumber_ID 11

    // 
    USHORT StorageManagerName[8];
    #define WMI_DISK_PERFORMANCE_StorageManagerName_SIZE sizeof(USHORT[8])
    #define WMI_DISK_PERFORMANCE_StorageManagerName_ID 12

} WMI_DISK_PERFORMANCE, *PWMI_DISK_PERFORMANCE;

// MSDiskDriver_Performance - MSDiskDriver_Performance
// Disk performance statistics
#define MSDiskDriver_PerformanceGuid \
    { 0xbdd865d1,0xd7c1,0x11d0, { 0xa5,0x01,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSDiskDriver_Performance_GUID, \
            0xbdd865d1,0xd7c1,0x11d0,0xa5,0x01,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _MSDiskDriver_Performance
{
    // Performance Data Information
    WMI_DISK_PERFORMANCE PerfData;
    #define MSDiskDriver_Performance_PerfData_SIZE sizeof(WMI_DISK_PERFORMANCE)
    #define MSDiskDriver_Performance_PerfData_ID 1

    // Internal device name
    CHAR VariableData[1];
    #define MSDiskDriver_Performance_DeviceName_ID 2

} MSDiskDriver_Performance, *PMSDiskDriver_Performance;

// MSStorageDriver_FailurePredictStatus - STORAGE_FAILURE_PREDICT_STATUS
// Storage Device Failure Prediction Status
#define WMI_STORAGE_FAILURE_PREDICT_STATUS_GUID \
    { 0x78ebc102,0x4cf9,0x11d2, { 0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSStorageDriver_FailurePredictStatus_GUID, \
            0x78ebc102,0x4cf9,0x11d2,0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _STORAGE_FAILURE_PREDICT_STATUS
{
    // 
    ULONG Reason;
    #define STORAGE_FAILURE_PREDICT_STATUS_Reason_SIZE sizeof(ULONG)
    #define STORAGE_FAILURE_PREDICT_STATUS_Reason_ID 1

    // TRUE if drive is predictiing failure. In this case it is critical that the disk is backed up immediately
    BOOLEAN PredictFailure;
    #define STORAGE_FAILURE_PREDICT_STATUS_PredictFailure_SIZE sizeof(BOOLEAN)
    #define STORAGE_FAILURE_PREDICT_STATUS_PredictFailure_ID 2

} STORAGE_FAILURE_PREDICT_STATUS, *PSTORAGE_FAILURE_PREDICT_STATUS;

// MSStorageDriver_FailurePredictData - STORAGE_FAILURE_PREDICT_DATA
// Storage Device Failure Prediction Data
#define WMI_STORAGE_FAILURE_PREDICT_DATA_GUID \
    { 0x78ebc103,0x4cf9,0x11d2, { 0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSStorageDriver_FailurePredictData_GUID, \
            0x78ebc103,0x4cf9,0x11d2,0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _STORAGE_FAILURE_PREDICT_DATA
{
    // 
    ULONG Length;
    #define STORAGE_FAILURE_PREDICT_DATA_Length_SIZE sizeof(ULONG)
    #define STORAGE_FAILURE_PREDICT_DATA_Length_ID 1

    // Vendor specific failure prediction data
    UCHAR VendorSpecific[512];
    #define STORAGE_FAILURE_PREDICT_DATA_VendorSpecific_SIZE sizeof(UCHAR[512])
    #define STORAGE_FAILURE_PREDICT_DATA_VendorSpecific_ID 2

} STORAGE_FAILURE_PREDICT_DATA, *PSTORAGE_FAILURE_PREDICT_DATA;

// MSStorageDriver_FailurePredictEvent - STORAGE_FAILURE_PREDICT_EVENT
// Storage Device Failure Prediction Event
#define WMI_STORAGE_PREDICT_FAILURE_EVENT_GUID \
    { 0x78ebc104,0x4cf9,0x11d2, { 0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSStorageDriver_FailurePredictEvent_GUID, \
            0x78ebc104,0x4cf9,0x11d2,0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _STORAGE_FAILURE_PREDICT_EVENT
{
    // 
    ULONG Length;
    #define STORAGE_FAILURE_PREDICT_EVENT_Length_SIZE sizeof(ULONG)
    #define STORAGE_FAILURE_PREDICT_EVENT_Length_ID 1

    // Vendor specific failure prediction data
    UCHAR VendorSpecific[1];
    #define STORAGE_FAILURE_PREDICT_EVENT_VendorSpecific_ID 2

} STORAGE_FAILURE_PREDICT_EVENT, *PSTORAGE_FAILURE_PREDICT_EVENT;

// MSStorageDriver_FailurePredictFunction - STORAGE_FAILURE_PREDICT_FUNCTION
// Storage Device Failure Prediction Functions
#define WMI_STORAGE_FAILURE_PREDICT_FUNCTION_GUID \
    { 0x78ebc105,0x4cf9,0x11d2, { 0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSStorageDriver_FailurePredictFunction_GUID, \
            0x78ebc105,0x4cf9,0x11d2,0xba,0x4a,0x00,0xa0,0xc9,0x06,0x29,0x10);

//
// Method id definitions for MSStorageDriver_FailurePredictFunction
#define AllowPerformanceHit     1
#define EnableDisableHardwareFailurePrediction     2
#define EnableDisableFailurePredictionPolling     3
#define GetFailurePredictionCapability     4
#define EnableOfflineDiags     5

// Warning: Header for class STORAGE_FAILURE_PREDICT_FUNCTION cannot be created
typedef struct _STORAGE_FAILURE_PREDICT_FUNCTION
{
    char VariableData[1];

} STORAGE_FAILURE_PREDICT_FUNCTION, *PSTORAGE_FAILURE_PREDICT_FUNCTION;

// MSIde_PortDeviceInfo - MSIde_PortDeviceInfo
// Scsi Address
#define MSIde_PortDeviceInfoGuid \
    { 0x53f5630f,0xb6bf,0x11d0, { 0x94,0xf2,0x00,0xa0,0xc9,0x1e,0xfb,0x8b } }

DEFINE_GUID(MSIde_PortDeviceInfo_GUID, \
            0x53f5630f,0xb6bf,0x11d0,0x94,0xf2,0x00,0xa0,0xc9,0x1e,0xfb,0x8b);


typedef struct _MSIde_PortDeviceInfo
{
    // Scsi Bus Number
    UCHAR Bus;
    #define MSIde_PortDeviceInfo_Bus_SIZE sizeof(UCHAR)
    #define MSIde_PortDeviceInfo_Bus_ID 1

    // Scsi Target ID
    UCHAR Target;
    #define MSIde_PortDeviceInfo_Target_SIZE sizeof(UCHAR)
    #define MSIde_PortDeviceInfo_Target_ID 2

    // Scsi Lun
    UCHAR Lun;
    #define MSIde_PortDeviceInfo_Lun_SIZE sizeof(UCHAR)
    #define MSIde_PortDeviceInfo_Lun_ID 3

} MSIde_PortDeviceInfo, *PMSIde_PortDeviceInfo;

// MSSerial_PortName - MSSerial_PortName
// Serial Port Name
#define SERIAL_PORT_WMI_NAME_GUID \
    { 0xa0ec11a8,0xb16c,0x11d1, { 0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d } }

DEFINE_GUID(MSSerial_PortName_GUID, \
            0xa0ec11a8,0xb16c,0x11d1,0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d);


typedef struct _MSSerial_PortName
{
    // Serial Port Name
    CHAR VariableData[1];
    #define MSSerial_PortName_PortName_ID 1

} MSSerial_PortName, *PMSSerial_PortName;

// MSSerial_CommInfo - SERIAL_WMI_COMM_DATA
// Serial Communications Information
#define SERIAL_PORT_WMI_COMM_GUID \
    { 0xedb16a62,0xb16c,0x11d1, { 0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d } }

DEFINE_GUID(MSSerial_CommInfo_GUID, \
            0xedb16a62,0xb16c,0x11d1,0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d);


typedef struct _SERIAL_WMI_COMM_DATA
{
    // The BaudRate property indicates the baud rate for this serial port
    ULONG BaudRate;
    #define SERIAL_WMI_COMM_DATA_BaudRate_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_BaudRate_ID 1

    // The BitsPerByte property indicates the number of bits per byte for the serial port
    ULONG BitsPerByte;
    #define SERIAL_WMI_COMM_DATA_BitsPerByte_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_BitsPerByte_ID 2


// None
#define SERIAL_WMI_PARITY_NONE 0
// Odd
#define SERIAL_WMI_PARITY_ODD 1
// Even
#define SERIAL_WMI_PARITY_EVEN 2
// Space
#define SERIAL_WMI_PARITY_SPACE 3
// Mark
#define SERIAL_WMI_PARITY_MARK 4

    // The Parity property indicates the type of parity used
    ULONG Parity;
    #define SERIAL_WMI_COMM_DATA_Parity_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_Parity_ID 3

    // The ParityCheckEnabled property determines whether parity checking is enabled
    BOOLEAN ParityCheckEnable;
    #define SERIAL_WMI_COMM_DATA_ParityCheckEnable_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_ParityCheckEnable_ID 4


// 1
#define SERIAL_WMI_STOP_1 0
// 1.5
#define SERIAL_WMI_STOP_1_5 1
// 2
#define SERIAL_WMI_STOP_2 2

    // The StopBits property indicates the number of stop bits for the serial port
    ULONG StopBits;
    #define SERIAL_WMI_COMM_DATA_StopBits_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_StopBits_ID 5

    // The XOffCharacter property indicates the XOff character for the serial port
    ULONG XoffCharacter;
    #define SERIAL_WMI_COMM_DATA_XoffCharacter_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_XoffCharacter_ID 6

    // The XOffXmitThreshold property indicates the XOff transmit threshold for the serial port
    ULONG XoffXmitThreshold;
    #define SERIAL_WMI_COMM_DATA_XoffXmitThreshold_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_XoffXmitThreshold_ID 7

    // The XOnCharacter property indicates the XOn character
    ULONG XonCharacter;
    #define SERIAL_WMI_COMM_DATA_XonCharacter_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_XonCharacter_ID 8

    // The XOnXMitThreshold property indicates the XOn transmit threshold
    ULONG XonXmitThreshold;
    #define SERIAL_WMI_COMM_DATA_XonXmitThreshold_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_XonXmitThreshold_ID 9

    // The MaximumBaudRate property indicates the maximum baud rate of the serial port
    ULONG MaximumBaudRate;
    #define SERIAL_WMI_COMM_DATA_MaximumBaudRate_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_MaximumBaudRate_ID 10

    // The MaximumOutputBufferSize property indicates the maximum output buffer size (in bytes)
    ULONG MaximumOutputBufferSize;
    #define SERIAL_WMI_COMM_DATA_MaximumOutputBufferSize_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_MaximumOutputBufferSize_ID 11

    // The MaximumInputBufferSize property indicates the maximum input buffer size (in bytes)
    ULONG MaximumInputBufferSize;
    #define SERIAL_WMI_COMM_DATA_MaximumInputBufferSize_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMM_DATA_MaximumInputBufferSize_ID 12

    // The Support16BitMode property determines whether 16-bit mode is supported on the Win32 serial port
    BOOLEAN Support16BitMode;
    #define SERIAL_WMI_COMM_DATA_Support16BitMode_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_Support16BitMode_ID 13

    // The SupportDTRDSR property determines whether Data Terminal Ready (DTR) and Data Set Ready (DSR) signals are supported on the Win32 serial port.
    BOOLEAN SupportDTRDSR;
    #define SERIAL_WMI_COMM_DATA_SupportDTRDSR_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SupportDTRDSR_ID 14

    // The SupportIntervalTimeouts property determines whether interval timeouts are supported on the serial port
    BOOLEAN SupportIntervalTimeouts;
    #define SERIAL_WMI_COMM_DATA_SupportIntervalTimeouts_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SupportIntervalTimeouts_ID 15

    // The SupportParityCheck property determines whether parity checking is supported on the Win32 serial port
    BOOLEAN SupportParityCheck;
    #define SERIAL_WMI_COMM_DATA_SupportParityCheck_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SupportParityCheck_ID 16

    // The SupportRTSCTS property determines whether Ready To Send (RTS) and Clear To Send (CTS) signals are supported on the serial port
    BOOLEAN SupportRTSCTS;
    #define SERIAL_WMI_COMM_DATA_SupportRTSCTS_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SupportRTSCTS_ID 17

    // The SupportXOnXOff property determines whether software flow control is supported on the serial port
    BOOLEAN SupportXonXoff;
    #define SERIAL_WMI_COMM_DATA_SupportXonXoff_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SupportXonXoff_ID 18

    // The SettableBaudRate property determines whether the baud rate can be set on the serial port
    BOOLEAN SettableBaudRate;
    #define SERIAL_WMI_COMM_DATA_SettableBaudRate_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableBaudRate_ID 19

    // The SettableDataBits property determines whether the number of data bits can be set on the Win32 serial port
    BOOLEAN SettableDataBits;
    #define SERIAL_WMI_COMM_DATA_SettableDataBits_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableDataBits_ID 20

    // The SettableFlowControl property determines whether the flow control can be set on the serial port
    BOOLEAN SettableFlowControl;
    #define SERIAL_WMI_COMM_DATA_SettableFlowControl_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableFlowControl_ID 21

    // The SettableParity property determines whether the parity can be set on the serial port
    BOOLEAN SettableParity;
    #define SERIAL_WMI_COMM_DATA_SettableParity_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableParity_ID 22

    // The SettableParityCheck property determines whether parity checking can be set on the serial port
    BOOLEAN SettableParityCheck;
    #define SERIAL_WMI_COMM_DATA_SettableParityCheck_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableParityCheck_ID 23

    // The SettableStopBits property determines whether the number of stop bits can be set on the serial port
    BOOLEAN SettableStopBits;
    #define SERIAL_WMI_COMM_DATA_SettableStopBits_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_SettableStopBits_ID 24

    // The IsBusy property determines whether the serial port is busy
    BOOLEAN IsBusy;
    #define SERIAL_WMI_COMM_DATA_IsBusy_SIZE sizeof(BOOLEAN)
    #define SERIAL_WMI_COMM_DATA_IsBusy_ID 25

} SERIAL_WMI_COMM_DATA, *PSERIAL_WMI_COMM_DATA;

// MSStorageDriver_ScsiInfoExceptions - STORAGE_SCSI_INFO_EXCEPTIONS
// Data for Scsi Info Exceptions Mode Page
#define WMI_STORAGE_SCSI_INFO_EXCEPTIONS_GUID \
    { 0x1101d829,0x167b,0x4ebf, { 0xac,0xae,0x28,0xca,0xb7,0xc3,0x48,0x02 } }

DEFINE_GUID(MSStorageDriver_ScsiInfoExceptions_GUID, \
            0x1101d829,0x167b,0x4ebf,0xac,0xae,0x28,0xca,0xb7,0xc3,0x48,0x02);


typedef struct _STORAGE_SCSI_INFO_EXCEPTIONS
{
    // The returned Parameter Savable bit of 1 indicates that page parameter data is savable.
    BOOLEAN PageSavable;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_PageSavable_SIZE sizeof(BOOLEAN)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_PageSavable_ID 1

    // Bit flags: Perf set to zero indicates that informational exception operations that are the cause of delays are acceptable. DExcpt set to zero indicates information exception operations shall be enabled. Test of one instructs the drive to create false drive failures. LogErr bit of zero indicates that logging of informational exception conditions are vendor specific.
    UCHAR Flags;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_Flags_SIZE sizeof(UCHAR)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_Flags_ID 2

    // The Method of Reporting Informational Exceptions (MRIE) indicates the methods that shall be used by the target to report information exception conditions.
    UCHAR MRIE;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_MRIE_SIZE sizeof(UCHAR)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_MRIE_ID 3

    // Buffer padding to 32 bits, do not use
    UCHAR Padding;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_Padding_SIZE sizeof(UCHAR)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_Padding_ID 4

    // Period in 100ms increments for reproting that an information exception condition has occurred.
    ULONG IntervalTimer;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_IntervalTimer_SIZE sizeof(ULONG)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_IntervalTimer_ID 5

    // Indicates the number of times to report an informational exception condition to the application client. A value of zero indications there is no limit.
    ULONG ReportCount;
    #define STORAGE_SCSI_INFO_EXCEPTIONS_ReportCount_SIZE sizeof(ULONG)
    #define STORAGE_SCSI_INFO_EXCEPTIONS_ReportCount_ID 6

} STORAGE_SCSI_INFO_EXCEPTIONS, *PSTORAGE_SCSI_INFO_EXCEPTIONS;

// MSSerial_HardwareConfiguration - SERIAL_WMI_HW_DATA
// Hardware configuration for serial port
#define SERIAL_PORT_WMI_HW_GUID \
    { 0x270b9b86,0xb16d,0x11d1, { 0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d } }

DEFINE_GUID(MSSerial_HardwareConfiguration_GUID, \
            0x270b9b86,0xb16d,0x11d1,0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d);


typedef struct _SERIAL_WMI_HW_DATA
{
    // The IRQNumber property indicates the number of the IRQ resource
    ULONG IrqNumber;
    #define SERIAL_WMI_HW_DATA_IrqNumber_SIZE sizeof(ULONG)
    #define SERIAL_WMI_HW_DATA_IrqNumber_ID 1

    // The Vector property indicates the vector of the IRQ resource
    ULONG IrqVector;
    #define SERIAL_WMI_HW_DATA_IrqVector_SIZE sizeof(ULONG)
    #define SERIAL_WMI_HW_DATA_IrqVector_ID 2

    // The IRQLevel property indicates the level of the IRQ resource
    ULONG IrqLevel;
    #define SERIAL_WMI_HW_DATA_IrqLevel_SIZE sizeof(ULONG)
    #define SERIAL_WMI_HW_DATA_IrqLevel_ID 3

    // The AffinityMask property indicates the affinity mask of the IRQ resource
    ULONG IrqAffinityMask;
    #define SERIAL_WMI_HW_DATA_IrqAffinityMask_SIZE sizeof(ULONG)
    #define SERIAL_WMI_HW_DATA_IrqAffinityMask_ID 4


// Latched
#define SERIAL_WMI_INTTYPE_LATCHED 0
// Level
#define SERIAL_WMI_INTTYPE_LEVEL 1

    // The InterruptType property indicates the interrupt type of the IRQ resource
    ULONG InterruptType;
    #define SERIAL_WMI_HW_DATA_InterruptType_SIZE sizeof(ULONG)
    #define SERIAL_WMI_HW_DATA_InterruptType_ID 5

    // The BaseIOAddress is the base IO address for the serial port
    ULONGLONG BaseIOAddress;
    #define SERIAL_WMI_HW_DATA_BaseIOAddress_SIZE sizeof(ULONGLONG)
    #define SERIAL_WMI_HW_DATA_BaseIOAddress_ID 6

} SERIAL_WMI_HW_DATA, *PSERIAL_WMI_HW_DATA;

// MSSerial_PerformanceInformation - SERIAL_WMI_PERF_DATA
// Performance information for serial port
#define SERIAL_PORT_WMI_PERF_GUID \
    { 0x56415acc,0xb16d,0x11d1, { 0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d } }

DEFINE_GUID(MSSerial_PerformanceInformation_GUID, \
            0x56415acc,0xb16d,0x11d1,0xbd,0x98,0x00,0xa0,0xc9,0x06,0xbe,0x2d);


typedef struct _SERIAL_WMI_PERF_DATA
{
    // The ReceivedCount property indicates the number of bytes received in the current session
    ULONG ReceivedCount;
    #define SERIAL_WMI_PERF_DATA_ReceivedCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_ReceivedCount_ID 1

    // The TransmittedCount property indicates the number of bytes transmitted in the current session
    ULONG TransmittedCount;
    #define SERIAL_WMI_PERF_DATA_TransmittedCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_TransmittedCount_ID 2

    // The FrameErrorCount property indicates the number of framing errors that occurred in the current session
    ULONG FrameErrorCount;
    #define SERIAL_WMI_PERF_DATA_FrameErrorCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_FrameErrorCount_ID 3

    // The SerialOverrunCount property indicates the number of serial overrun errors that occurred in the current session
    ULONG SerialOverrunErrorCount;
    #define SERIAL_WMI_PERF_DATA_SerialOverrunErrorCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_SerialOverrunErrorCount_ID 4

    // The BufferOverrunCount property indicates the number of buffer overrun errors that occurred in the current session
    ULONG BufferOverrunErrorCount;
    #define SERIAL_WMI_PERF_DATA_BufferOverrunErrorCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_BufferOverrunErrorCount_ID 5

    // The ParityErrorCount property indicates the number of parity errors that occurred in the current session
    ULONG ParityErrorCount;
    #define SERIAL_WMI_PERF_DATA_ParityErrorCount_SIZE sizeof(ULONG)
    #define SERIAL_WMI_PERF_DATA_ParityErrorCount_ID 6

} SERIAL_WMI_PERF_DATA, *PSERIAL_WMI_PERF_DATA;

// MSSerial_CommProperties - SERIAL_WMI_COMMPROP
// Communication properties for serial port
#define SERIAL_PORT_WMI_PROPERTIES_GUID \
    { 0x8209ec2a,0x2d6b,0x11d2, { 0xba,0x49,0x00,0xa0,0xc9,0x06,0x29,0x10 } }

DEFINE_GUID(MSSerial_CommProperties_GUID, \
            0x8209ec2a,0x2d6b,0x11d2,0xba,0x49,0x00,0xa0,0xc9,0x06,0x29,0x10);


typedef struct _SERIAL_WMI_COMMPROP
{
    // Specifies the size, in bytes, of the entire data packet, regardless of the amount of data requested
    USHORT wPacketLength;
    #define SERIAL_WMI_COMMPROP_wPacketLength_SIZE sizeof(USHORT)
    #define SERIAL_WMI_COMMPROP_wPacketLength_ID 1

    // Specifies the version of the structure
    USHORT wPacketVersion;
    #define SERIAL_WMI_COMMPROP_wPacketVersion_SIZE sizeof(USHORT)
    #define SERIAL_WMI_COMMPROP_wPacketVersion_ID 2

    // Specifies a bitmask indicating which services are implemented by this provider. The SP_SERIALCOMM value is always specified for communications providers, including modem providers.
    ULONG dwServiceMask;
    #define SERIAL_WMI_COMMPROP_dwServiceMask_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwServiceMask_ID 3

    // Reserved; do not use.
    ULONG dwReserved1;
    #define SERIAL_WMI_COMMPROP_dwReserved1_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwReserved1_ID 4

    // Specifies the maximum size, in bytes, of the driver's internal output buffer. A value of zero indicates that no maximum value is imposed by the serial provider
    ULONG dwMaxTxQueue;
    #define SERIAL_WMI_COMMPROP_dwMaxTxQueue_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwMaxTxQueue_ID 5

    // Specifies the maximum size, in bytes, of the driver's internal input buffer. A value of zero indicates that no maximum value is imposed by the serial provider
    ULONG dwMaxRxQueue;
    #define SERIAL_WMI_COMMPROP_dwMaxRxQueue_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwMaxRxQueue_ID 6

    // Specifies the maximum allowable baud rate, in bits per second (bps). This member can be one of the following values: Value Meaning
    ULONG dwMaxBaud;
    #define SERIAL_WMI_COMMPROP_dwMaxBaud_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwMaxBaud_ID 7

    // Specifies the specific communications provider type
    ULONG dwProvSubType;
    #define SERIAL_WMI_COMMPROP_dwProvSubType_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwProvSubType_ID 8

    // Specifies a bitmask indicating the capabilities offered by the provider. This member can be one of the following values
    ULONG dwProvCapabilities;
    #define SERIAL_WMI_COMMPROP_dwProvCapabilities_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwProvCapabilities_ID 9

    // Specifies a bitmask indicating the communications parameter that can be changed
    ULONG dwSettableParams;
    #define SERIAL_WMI_COMMPROP_dwSettableParams_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwSettableParams_ID 10

    // Specifies a bitmask indicating the baud rates that can be used
    ULONG dwSettableBaud;
    #define SERIAL_WMI_COMMPROP_dwSettableBaud_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwSettableBaud_ID 11

    // Specifies a bitmask indicating the number of data bits that can be set
    USHORT wSettableData;
    #define SERIAL_WMI_COMMPROP_wSettableData_SIZE sizeof(USHORT)
    #define SERIAL_WMI_COMMPROP_wSettableData_ID 12

    // Specifies a bitmask indicating the stop bit and parity settings that can be selected.
    USHORT wSettableStopParity;
    #define SERIAL_WMI_COMMPROP_wSettableStopParity_SIZE sizeof(USHORT)
    #define SERIAL_WMI_COMMPROP_wSettableStopParity_ID 13

    // Specifies the size, in bytes, of the driver's internal output buffer. A value of zero indicates that the value is unavailable.
    ULONG dwCurrentTxQueue;
    #define SERIAL_WMI_COMMPROP_dwCurrentTxQueue_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwCurrentTxQueue_ID 14

    // Specifies the size, in bytes, of the driver's internal input buffer. A value of zero indicates that the value is unavailable.
    ULONG dwCurrentRxQueue;
    #define SERIAL_WMI_COMMPROP_dwCurrentRxQueue_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwCurrentRxQueue_ID 15

    // Specifies provider-specific data.
    ULONG dwProvSpec1;
    #define SERIAL_WMI_COMMPROP_dwProvSpec1_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwProvSpec1_ID 16

    // Specifies provider-specific data.
    ULONG dwProvSpec2;
    #define SERIAL_WMI_COMMPROP_dwProvSpec2_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwProvSpec2_ID 17

    // Number of bytes of provider specific data
    ULONG dwProvCharSize;
    #define SERIAL_WMI_COMMPROP_dwProvCharSize_SIZE sizeof(ULONG)
    #define SERIAL_WMI_COMMPROP_dwProvCharSize_ID 18

    // Specifies provider-specific data. Applications should ignore this member unless they have detailed information about the format of the data required by the provider.
    UCHAR wcProvChar[1];
    #define SERIAL_WMI_COMMPROP_wcProvChar_ID 19

} SERIAL_WMI_COMMPROP, *PSERIAL_WMI_COMMPROP;

// MSParallel_AllocFreeCounts - PARPORT_WMI_ALLOC_FREE_COUNTS
// The allocate and free counts track the port sharing of the parallel port. If the allocate count equals the free count then the port is idle. If the allocate count is greater than the free count (free count + 1) then some other driver in the system has acquired exclusive access to that port. If the allocate count stays constant at freecount+1 for an arbitrarily long period of time, then some driver may have illegally locked the port preventing other drivers from accessing the port.
#define PARPORT_WMI_ALLOCATE_FREE_COUNTS_GUID \
    { 0x4bbb69ea,0x6853,0x11d2, { 0x8e,0xce,0x00,0xc0,0x4f,0x8e,0xf4,0x81 } }

DEFINE_GUID(MSParallel_AllocFreeCounts_GUID, \
            0x4bbb69ea,0x6853,0x11d2,0x8e,0xce,0x00,0xc0,0x4f,0x8e,0xf4,0x81);


typedef struct _PARPORT_WMI_ALLOC_FREE_COUNTS
{
    // Port allocation count
    ULONG PortAllocates;
    #define PARPORT_WMI_ALLOC_FREE_COUNTS_PortAllocates_SIZE sizeof(ULONG)
    #define PARPORT_WMI_ALLOC_FREE_COUNTS_PortAllocates_ID 1

    // Port free count
    ULONG PortFrees;
    #define PARPORT_WMI_ALLOC_FREE_COUNTS_PortFrees_SIZE sizeof(ULONG)
    #define PARPORT_WMI_ALLOC_FREE_COUNTS_PortFrees_ID 2

} PARPORT_WMI_ALLOC_FREE_COUNTS, *PPARPORT_WMI_ALLOC_FREE_COUNTS;

// MSParallel_DeviceBytesTransferred - PARALLEL_WMI_LOG_INFO
// Bytes transferred for each mode for the device
#define PARALLEL_WMI_BYTES_TRANSFERRED_GUID \
    { 0x89fef2d6,0x654b,0x11d2, { 0x9e,0x15,0x00,0xc0,0x4f,0x8e,0xf4,0x81 } }

DEFINE_GUID(MSParallel_DeviceBytesTransferred_GUID, \
            0x89fef2d6,0x654b,0x11d2,0x9e,0x15,0x00,0xc0,0x4f,0x8e,0xf4,0x81);


typedef struct _PARALLEL_WMI_LOG_INFO
{
    // Reserved
    ULONG Flags1;
    #define PARALLEL_WMI_LOG_INFO_Flags1_SIZE sizeof(ULONG)
    #define PARALLEL_WMI_LOG_INFO_Flags1_ID 1

    // Reserved
    ULONG Flags2;
    #define PARALLEL_WMI_LOG_INFO_Flags2_SIZE sizeof(ULONG)
    #define PARALLEL_WMI_LOG_INFO_Flags2_ID 2

    // Reserved
    ULONG spare[2];
    #define PARALLEL_WMI_LOG_INFO_spare_SIZE sizeof(ULONG[2])
    #define PARALLEL_WMI_LOG_INFO_spare_ID 3

    // Bytes writtem using SPP mode
    LONGLONG SppWriteCount;
    #define PARALLEL_WMI_LOG_INFO_SppWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_SppWriteCount_ID 4

    // Bytes writtem using nibble mode
    LONGLONG NibbleReadCount;
    #define PARALLEL_WMI_LOG_INFO_NibbleReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_NibbleReadCount_ID 5

    // Bytes writtem using bounded Ecp mode
    LONGLONG BoundedEcpWriteCount;
    #define PARALLEL_WMI_LOG_INFO_BoundedEcpWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_BoundedEcpWriteCount_ID 6

    // Bytes read using bounded Ecp mode
    LONGLONG BoundedEcpReadCount;
    #define PARALLEL_WMI_LOG_INFO_BoundedEcpReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_BoundedEcpReadCount_ID 7

    // Bytes writtem using hardware Ecp mode
    LONGLONG HwEcpWriteCount;
    #define PARALLEL_WMI_LOG_INFO_HwEcpWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_HwEcpWriteCount_ID 8

    // Bytes read using hardware Ecp mode
    LONGLONG HwEcpReadCount;
    #define PARALLEL_WMI_LOG_INFO_HwEcpReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_HwEcpReadCount_ID 9

    // Bytes writtem using software Ecp mode
    LONGLONG SwEcpWriteCount;
    #define PARALLEL_WMI_LOG_INFO_SwEcpWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_SwEcpWriteCount_ID 10

    // Bytes read using software Ecp mode
    LONGLONG SwEcpReadCount;
    #define PARALLEL_WMI_LOG_INFO_SwEcpReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_SwEcpReadCount_ID 11

    // Bytes writtem using hardware Epp mode
    LONGLONG HwEppWriteCount;
    #define PARALLEL_WMI_LOG_INFO_HwEppWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_HwEppWriteCount_ID 12

    // Bytes read using hardware Epp mode
    LONGLONG HwEppReadCount;
    #define PARALLEL_WMI_LOG_INFO_HwEppReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_HwEppReadCount_ID 13

    // Bytes writtem using software Epp mode
    LONGLONG SwEppWriteCount;
    #define PARALLEL_WMI_LOG_INFO_SwEppWriteCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_SwEppWriteCount_ID 14

    // Bytes read using software Epp mode
    LONGLONG SwEppReadCount;
    #define PARALLEL_WMI_LOG_INFO_SwEppReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_SwEppReadCount_ID 15

    // Bytes read using byte (bidirectional / PS/2) mode
    LONGLONG ByteReadCount;
    #define PARALLEL_WMI_LOG_INFO_ByteReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_ByteReadCount_ID 16

    // Bytes read using channelized Nibble mode (IEEE 1284.3)
    LONGLONG ChannelNibbleReadCount;
    #define PARALLEL_WMI_LOG_INFO_ChannelNibbleReadCount_SIZE sizeof(LONGLONG)
    #define PARALLEL_WMI_LOG_INFO_ChannelNibbleReadCount_ID 17

} PARALLEL_WMI_LOG_INFO, *PPARALLEL_WMI_LOG_INFO;

// MSRedbook_DriverInformation - REDBOOK_WMI_STD_DATA
// Digital Audio Filter Driver Information (redbook)
#define GUID_REDBOOK_WMI_STD_DATA \
    { 0xb90550e7,0xae0a,0x11d1, { 0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30 } }

DEFINE_GUID(MSRedbook_DriverInformationGuid, \
            0xb90550e7,0xae0a,0x11d1,0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30);


typedef struct _REDBOOK_WMI_STD_DATA
{
    // NumberOfBuffers*SectorsPerRead*2352 is the amount of memory used to reduce skipping.
    ULONG NumberOfBuffers;
    #define REDBOOK_WMI_NUMBER_OF_BUFFERS_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_NUMBER_OF_BUFFERS_ID 1

    // Sectors (2352 bytes each) per read.
    ULONG SectorsPerRead;
    #define REDBOOK_WMI_SECTORS_PER_READ_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_SECTORS_PER_READ_ID 2

    // Bitwise mask of supported sectors per read for this drive.  The lowest bit is one sector reads.  If all bits are set, there are no restrictions.
    ULONG SectorsPerReadMask;
    #define REDBOOK_WMI_SECTORS_PER_READ_MASK_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_SECTORS_PER_READ_MASK_ID 3

    // Maximum sectors per read (depends on both adapter and drive).
    ULONG MaximumSectorsPerRead;
    #define REDBOOK_WMI_MAX_SECTORS_PER_READ_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_MAX_SECTORS_PER_READ_ID 4

    // PlayEnabled indicates the drive is currently using the RedBook filter.
    BOOLEAN PlayEnabled;
    #define REDBOOK_WMI_PLAY_ENABLED_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_PLAY_ENABLED_ID 5

    // CDDASupported indicates the drive supports digital audio for some sector sizes.
    BOOLEAN CDDASupported;
    #define REDBOOK_WMI_CDDA_SUPPORTED_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_CDDA_SUPPORTED_ID 6

    // CDDAAccurate indicates the drive acccurately reads digital audio.  This ensures the highest quality audio
    BOOLEAN CDDAAccurate;
    #define REDBOOK_WMI_CDDA_ACCURATE_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_CDDA_ACCURATE_ID 7

    // Reserved for future use
    BOOLEAN Reserved1;
    #define REDBOOK_WMI_STD_DATA_Reserved1_SIZE sizeof(BOOLEAN)
    #define REDBOOK_WMI_STD_DATA_Reserved1_ID 8

} REDBOOK_WMI_STD_DATA, *PREDBOOK_WMI_STD_DATA;

// MSRedbook_Performance - REDBOOK_WMI_PERF_DATA
// Digital Audio Filter Driver Performance Data (redbook)
#define GUID_REDBOOK_WMI_PERF_DATA \
    { 0xb90550e8,0xae0a,0x11d1, { 0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30 } }

DEFINE_GUID(MSRedbook_PerformanceGuid, \
            0xb90550e8,0xae0a,0x11d1,0xa5,0x71,0x00,0xc0,0x4f,0xa3,0x47,0x30);


typedef struct _REDBOOK_WMI_PERF_DATA
{
    // Seconds spent ready to read, but unused. (*1E-7)
    LONGLONG TimeReadDelay;
    #define REDBOOK_WMI_PERF_TIME_READING_DELAY_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_READING_DELAY_ID 1

    // Seconds spent reading data from source. (*1E-7)
    LONGLONG TimeReading;
    #define REDBOOK_WMI_PERF_TIME_READING_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_READING_ID 2

    // Seconds spent ready to stream, but unused. (*1E-7)
    LONGLONG TimeStreamDelay;
    #define REDBOOK_WMI_PERF_TIME_STREAMING_DELAY_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_STREAMING_DELAY_ID 3

    // Seconds spent streaming data. (*1E-7)
    LONGLONG TimeStreaming;
    #define REDBOOK_WMI_PERF_TIME_STREAMING_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_TIME_STREAMING_ID 4

    // Number of bytes of data read and streamed.
    LONGLONG DataProcessed;
    #define REDBOOK_WMI_PERF_DATA_PROCESSED_SIZE sizeof(LONGLONG)
    #define REDBOOK_WMI_PERF_DATA_PROCESSED_ID 5

    // Number of times the stream has paused due to insufficient stream buffers.
    ULONG StreamPausedCount;
    #define REDBOOK_WMI_PERF_STREAM_PAUSED_COUNT_SIZE sizeof(ULONG)
    #define REDBOOK_WMI_PERF_STREAM_PAUSED_COUNT_ID 6

} REDBOOK_WMI_PERF_DATA, *PREDBOOK_WMI_PERF_DATA;

// RegisteredGuids - RegisteredGuids
// Enumerates Guids registered with WMI. The InstanceName is the Guid.
#define RegisteredGuidsGuid \
    { 0xe3dff7bd,0x3915,0x11d2, { 0x91,0x03,0x00,0xc0,0x4f,0xb9,0x98,0xa2 } }

DEFINE_GUID(RegisteredGuids_GUID, \
            0xe3dff7bd,0x3915,0x11d2,0x91,0x03,0x00,0xc0,0x4f,0xb9,0x98,0xa2);


typedef struct _RegisteredGuids
{
    // Type of guid
    ULONG GuidType;
    #define RegisteredGuids_GuidType_SIZE sizeof(ULONG)
    #define RegisteredGuids_GuidType_ID 1

    // If Trace guid and enabled, indicates the LoggerId to which this Guid is currently logging data
    ULONG LoggerId;
    #define RegisteredGuids_LoggerId_SIZE sizeof(ULONG)
    #define RegisteredGuids_LoggerId_ID 2

    // If trace guid and If enabled, indicates the level of logging
    ULONG EnableLevel;
    #define RegisteredGuids_EnableLevel_SIZE sizeof(ULONG)
    #define RegisteredGuids_EnableLevel_ID 3

    // If trace guid and enabled, indicates the flags currently used in logging
    ULONG EnableFlags;
    #define RegisteredGuids_EnableFlags_SIZE sizeof(ULONG)
    #define RegisteredGuids_EnableFlags_ID 4

    // Indicates whether this Guid is enabled currently. For data guids this means if collection is enabled, For event guids this means if events are enabled. For Trace control guids this means the trace logging is enabled.
    BOOLEAN IsEnabled;
    #define RegisteredGuids_IsEnabled_SIZE sizeof(BOOLEAN)
    #define RegisteredGuids_IsEnabled_ID 5

} RegisteredGuids, *PRegisteredGuids;

#endif

