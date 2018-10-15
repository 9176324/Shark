/****************************************************************************
** COPYRIGHT (C) 1994-1997 INTEL CORPORATION                               **
** DEVELOPED FOR MICROSOFT BY INTEL CORP., HILLSBORO, OREGON               **
** HTTP://WWW.INTEL.COM/                                                   **
** THIS FILE IS PART OF THE INTEL ETHEREXPRESS PRO/100B(TM) AND            **
** ETHEREXPRESS PRO/100+(TM) NDIS 5.0 MINIPORT SAMPLE DRIVER               **
****************************************************************************/

#ifndef _NIC_DEF_H
#define _NIC_DEF_H

#if !defined(WIN2K)

#include "xfilter.h" // for ETH_* macros

#else

#define ETH_LENGTH_OF_ADDRESS 6

//
// ZZZ This is a little-endian specific check.
//
#define ETH_IS_MULTICAST(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))

//
// Check whether an address is broadcast.
//
#define ETH_IS_BROADCAST(Address)               \
    ((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)))

//
// This macro is used to copy from one network address to
// another.
//
#define ETH_COPY_NETWORK_ADDRESS(_D, _S) \
{ \
    *((ULONG UNALIGNED *)(_D)) = *((ULONG UNALIGNED *)(_S)); \
    *((USHORT UNALIGNED *)((UCHAR *)(_D)+4)) = *((USHORT UNALIGNED *)((UCHAR *)(_S)+4)); \
}

#endif



// packet and header sizes
#define NIC_MAX_PACKET_SIZE             1514
#define NIC_MIN_PACKET_SIZE             60
#define NIC_HEADER_SIZE                 14

// multicast list size                          
#define NIC_MAX_MCAST_LIST              32

 
// media type, we use ethernet, change if necessary
#define NIC_MEDIA_TYPE                  NdisMedium802_3

#define NIC_INTERRUPT_MODE              NdisInterruptLevelSensitive 

// NIC PCI Device and vendor IDs 
#define NIC_PCI_DEVICE_ID               0x1229
#define NIC_PCI_VENDOR_ID               0x8086

 // IO space length
#define NIC_MAP_IOSPACE_LENGTH          sizeof(CSR_STRUC)

// PCS config space including the Device Specific part of it/
#define NIC_PCI_E100_HDR_LENGTH         0xe2

// define some types for convenience
// TXCB_STRUC, RFD_STRUC and CSR_STRUC are hardware specific structures
// hardware TCB (Transmit Control Block) structure
typedef TXCB_STRUC                      HW_TCB; 
typedef PTXCB_STRUC                     PHW_TCB;

// hardware RFD (Receive Frame Descriptor) structure                         
typedef RFD_STRUC                       HW_RFD;  
typedef PRFD_STRUC                      PHW_RFD;               

// hardware CSR (Control Status Register) structure                         
typedef CSR_STRUC                       HW_CSR;                                       
typedef PCSR_STRUC                      PHW_CSR;                                      
// change to your company name instead of using Microsoft
#define NIC_VENDOR_DESC                 "Microsoft"

// number of TCBs per processor - min, default and max
#define NIC_MIN_TCBS                    1
#define NIC_DEF_TCBS                    32
#define NIC_MAX_TCBS                    64

// max number of physical fragments supported per TCB
#define NIC_MAX_PHYS_BUF_COUNT          8     

// number of RFDs - min, default and max
#define NIC_MIN_RFDS                    4
#define NIC_DEF_RFDS                    20
#define NIC_MAX_RFDS                    1024

// only grow the RFDs up to this number
#define NIC_MAX_GROW_RFDS               128 

// How many intervals before the RFD list is shrinked?
#define NIC_RFD_SHRINK_THRESHOLD        10

// local data buffer size (to copy send packet data into a local buffer)
#define NIC_BUFFER_SIZE                 1520

// max lookahead size
#define NIC_MAX_LOOKAHEAD               (NIC_MAX_PACKET_SIZE - NIC_HEADER_SIZE)

// max number of send packets the MiniportSendPackets function can accept                            
#define NIC_MAX_SEND_PACKETS            10

// supported filters
#define NIC_SUPPORTED_FILTERS (     \
    NDIS_PACKET_TYPE_DIRECTED       | \
    NDIS_PACKET_TYPE_MULTICAST      | \
    NDIS_PACKET_TYPE_BROADCAST      | \
    NDIS_PACKET_TYPE_PROMISCUOUS    | \
    NDIS_PACKET_TYPE_ALL_MULTICAST)

// Threshold for a remove 
#define NIC_HARDWARE_ERROR_THRESHOLD    5

// The CheckForHang intervals before we decide the send is stuck
#define NIC_SEND_HANG_THRESHOLD         5        

// NIC specific macros                                        
#define NIC_RFD_GET_STATUS(_HwRfd) ((_HwRfd)->RfdCbHeader.CbStatus)
#define NIC_RFD_STATUS_COMPLETED(_Status) ((_Status) & RFD_STATUS_COMPLETE)
#define NIC_RFD_STATUS_SUCCESS(_Status) ((_Status) & RFD_STATUS_OK)
#define NIC_RFD_GET_PACKET_SIZE(_HwRfd) (((_HwRfd)->RfdActualCount) & RFD_ACT_COUNT_MASK)
#define NIC_RFD_VALID_ACTUALCOUNT(_HwRfd) ((((_HwRfd)->RfdActualCount) & (RFD_EOF_BIT | RFD_F_BIT)) == (RFD_EOF_BIT | RFD_F_BIT))

#define ListNext(_pL)                       (_pL)->Flink

#define ListPrev(_pL)                       (_pL)->Blink

// Constants for various purposes of KeStallExecutionProcessor

#define NIC_DELAY_POST_RESET            20
// Wait 5 milliseconds for the self-test to complete
#define NIC_DELAY_POST_SELF_TEST_MS     5

                                      
// delay used for link detection to minimize the init time
// change this value to match your hardware 
#define NIC_LINK_DETECTION_DELAY        ((LONGLONG) -MILLISECONDS_TO_100NS * 100) // 100ms
#define NIC_CHECK_FOR_HANG_DELAY        ((LONGLONG) -MILLISECONDS_TO_100NS * 400) // 400ms

// MP_TCB flags
#define fMP_TCB_IN_USE                         0x00000001
#define fMP_TCB_USE_LOCAL_BUF                  0x00000002
#define fMP_TCB_MULTICAST                      0x00000004  // a hardware workaround using multicast
               
// MP_RFD flags                           
#define fMP_RFD_RECV_PEND                      0x00000001
#define fMP_RFD_ALLOC_PEND                     0x00000002
#define fMP_RFD_RECV_READY                     0x00000004
#define fMP_RFD_RESOURCES                      0x00000008

// MP_ADAPTER flags               
#define fMP_ADAPTER_SCATTER_GATHER             0x00000001
#define fMP_ADAPTER_SEND_SGL_LOOKASIDE         0x00000002
#define fMP_ADAPTER_RECV_LOOKASIDE             0x00000004
#define fMP_ADAPTER_INTERRUPT_IN_USE           0x00000008

#define fMP_ADAPTER_NON_RECOVER_ERROR          0x00800000

#define fMP_ADAPTER_RESET_IN_PROGRESS          0x01000000
#define fMP_ADAPTER_NO_CABLE                    0x02000000 
#define fMP_ADAPTER_HARDWARE_ERROR             0x04000000
#define fMP_ADAPTER_REMOVE_IN_PROGRESS         0x08000000

#define fMP_ADAPTER_HALT_IN_PROGRESS           0x10000000
#define fMP_ADAPTER_LINK_DETECTION             0x20000000
                                 
#define fMP_ADAPTER_FAIL_SEND_MASK             0x1ff00000                
#define fMP_ADAPTER_NOT_READY_MASK             0x3ff00000    

#define MP_SHOULD_FAIL_SEND(_M)   ((_M)->Flags & fMP_ADAPTER_FAIL_SEND_MASK) 
#define MP_IS_NOT_READY(_M)       ((_M)->Flags & fMP_ADAPTER_NOT_READY_MASK)
#define MP_IS_READY(_M)           !((_M)->Flags & fMP_ADAPTER_NOT_READY_MASK)

#define NIC_INTERRUPT_DISABLED(_adapter) \
   (_adapter->CSRAddress->ScbCommandHigh & SCB_INT_MASK)
   
#define NIC_INTERRUPT_ACTIVE(_adapter) \
      (((_adapter->CSRAddress->ScbStatus & SCB_ALL_INTERRUPT_BITS) != SCB_ALL_INTERRUPT_BITS) \
       && (_adapter->CSRAddress->ScbStatus & SCB_ACK_MASK))


#define NIC_ACK_INTERRUPT(_adapter, _value) { \
   _value = _adapter->CSRAddress->ScbStatus & SCB_ACK_MASK; \
   _adapter->CSRAddress->ScbStatus = _value; }        

#define NIC_IS_RECV_READY(_adapter) \
    ((_adapter->CSRAddress->ScbStatus & SCB_RUS_MASK) == SCB_RUS_READY)

//-------------------------------------------------------------------------
// NON_TRANSMIT_CB -- Generic Non-Transmit Command Block
//-------------------------------------------------------------------------
typedef struct _NON_TRANSMIT_CB
{
    union
    {
        MULTICAST_CB_STRUC  Multicast;
        CONFIG_CB_STRUC     Config;
        IA_CB_STRUC         Setup;
        DUMP_CB_STRUC       Dump;
        FILTER_CB_STRUC     Filter;
    }   NonTxCb;

} NON_TRANSMIT_CB, *PNON_TRANSMIT_CB;    

typedef enum _MEDIA_STATE {

    Connected = 0,
    Disconnected

} MEDIA_STATE;

#define ALIGN_16                   16

#ifndef MIN
#define MIN(a, b)   ((a) > (b) ? b: a)
#endif

//
// The driver should put the data(after Ethernet header) at 8-bytes boundary
//
#define ETH_DATA_ALIGN                      8   // the data(after Ethernet header) should be 8-byte aligned
// 
// Shift HW_RFD 0xA bytes to make Tcp data 8-byte aligned
// Since the ethernet header is 14 bytes long. If a packet is at 0xA bytes 
// offset, its data(ethernet user data) will be at 8 byte boundary
// 
#define HWRFD_SHIFT_OFFSET                0xA   // Shift HW_RFD 0xA bytes to make Tcp data 8-byte aligned

//
// The driver has to allocate more data then HW_RFD needs to allow shifting data
// 
#define MORE_DATA_FOR_ALIGN         (ETH_DATA_ALIGN + HWRFD_SHIFT_OFFSET)
//
// Get a 8-bytes aligned memory address from a given the memory address.
// If the given address is not 8-bytes aligned, return  the closest bigger memory address
// which is 8-bytes aligned. 
// 
#define DATA_ALIGN(_Va)             ((PVOID)(((ULONG_PTR)(_Va) + (ETH_DATA_ALIGN - 1)) & ~(ETH_DATA_ALIGN - 1)))
//
// Get the number of bytes the final address shift from the original address
// 
#define BYTES_SHIFT(_NewVa, _OrigVa) ((PUCHAR)(_NewVa) - (PUCHAR)(_OrigVa))

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))
    

//--------------------------------------
// Some utility macros        
//--------------------------------------
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define MP_ALIGNMEM(_p, _align) (((_align) == 0) ? (_p) : (PUCHAR)(((ULONG_PTR)(_p) + ((_align)-1)) & (~((ULONG_PTR)(_align)-1))))
#define MP_ALIGNMEM_PHYS(_p, _align) (((_align) == 0) ?  (_p) : (((ULONG)(_p) + ((_align)-1)) & (~((ULONG)(_align)-1))))
#define MP_ALIGNMEM_PA(_p, _align) (((_align) == 0) ?  (_p).QuadPart : (((_p).QuadPart + ((_align)-1)) & (~((ULONGLONG)(_align)-1))))

#define GetListHeadEntry(ListHead)  ((ListHead)->Flink)
#define GetListTailEntry(ListHead)  ((ListHead)->Blink)
#define GetListFLink(ListEntry)     ((ListEntry)->Flink)

#define IsSListEmpty(ListHead)  (((PSINGLE_LIST_ENTRY)ListHead)->Next == NULL)

#define MP_EXIT goto exit

//--------------------------------------
// Macros for flag and ref count operations       
//--------------------------------------
#define MP_SET_FLAG(_M, _F)         ((_M)->Flags |= (_F))   
#define MP_CLEAR_FLAG(_M, _F)       ((_M)->Flags &= ~(_F))
#define MP_CLEAR_FLAGS(_M)          ((_M)->Flags = 0)
#define MP_TEST_FLAG(_M, _F)        (((_M)->Flags & (_F)) != 0)
#define MP_TEST_FLAGS(_M, _F)       (((_M)->Flags & (_F)) == (_F))

#if 0 // Not implemented
//--------------------------------------
// Coalesce Tx buffer for local data copying                     
//--------------------------------------
typedef struct _MP_TXBUF
{
    SINGLE_LIST_ENTRY       SList;
    PMDL                    Mdl;

    ULONG                   AllocSize;
    PVOID                   AllocVa;
    PHYSICAL_ADDRESS        AllocPa; 

    PUCHAR                  pBuffer;
    PHYSICAL_ADDRESS        BufferPa;
    ULONG                   BufferSize;

} MP_TXBUF, *PMP_TXBUF;

#endif

//--------------------------------------
// TCB (Transmit Control Block)
//--------------------------------------
typedef struct _MP_TCB
{
    struct _MP_TCB    *Next;
    ULONG             Flags;
    ULONG             Count;
    PIRP              Irp;

    PHW_TCB           HwTcb;            // ptr to HW TCB VA
    ULONG             HwTcbPhys;        // ptr to HW TCB PA
    PHW_TCB           PrevHwTcb;        // ptr to previous HW TCB VA

    PTBD_STRUC        HwTbd;            // ptr to first TBD 
    ULONG             HwTbdPhys;        // ptr to first TBD PA

    ULONG             PhysBufCount;                                 
    ULONG             BufferCount;   
    PMDL              FirstBuffer;                              
    ULONG             PacketLength;


} MP_TCB, *PMP_TCB;

//--------------------------------------
// RFD (Receive Frame Descriptor)
//--------------------------------------
typedef struct _MP_RFD
{
    LIST_ENTRY              List;
    PVOID                   Buffer;          // Pointer to Buffer
    PMDL                    Mdl;
    PHW_RFD                 HwRfd;               // ptr to hardware RFD
    PHW_RFD                 OriginalHwRfd;       // ptr to shared memory buffer
    PHYSICAL_ADDRESS        HwRfdPa;             // physical address of RFD   
    PHYSICAL_ADDRESS        OriginalHwRfdPa;     // Original physical address allocated by NDIS
    ULONG                   HwRfdPhys;          // lower part of HwRfdPa 
    
    ULONG                   Flags;
    ULONG                   PacketSize;         // total size of receive frame
} MP_RFD, *PMP_RFD;



//--------------------------------------
// Structure for Power Management Info
//--------------------------------------
typedef struct _MP_POWER_MGMT
{


    // List of Wake Up Patterns
    LIST_ENTRY              PatternList;

    // Current Power state of the adapter
    UINT                    PowerState;

    // Is PME_En on this adapter
    BOOLEAN                 PME_En;

    // Wake-up capabailities of the adapter
    BOOLEAN                 bWakeFromD0;
    BOOLEAN                 bWakeFromD1;
    BOOLEAN                 bWakeFromD2;
    BOOLEAN                 bWakeFromD3Hot;
    BOOLEAN                 bWakeFromD3Aux;
    // Pad
    BOOLEAN                 Pad[2];

} MP_POWER_MGMT, *PMP_POWER_MGMT;



typedef struct _MP_WAKE_PATTERN 
{
    // Link to the next Pattern
    LIST_ENTRY      linkListEntry;

    // E100 specific signature of the pattern
    ULONG           Signature;

    // Size of this allocation
    ULONG           AllocationSize;

    // Pattern - This contains the NDIS_PM_PACKET_PATTERN
    UCHAR           Pattern[1];
    
} MP_WAKE_PATTERN , *PMP_WAKE_PATTERN ;


//--------------------------------------
// Macros specific to miniport adapter structure 
//--------------------------------------
#define MP_TCB_RESOURCES_AVAIABLE(_M) ((_M)->nBusySend < (_M)->NumTcb)

#define MP_SET_HARDWARE_ERROR(adapter)    MP_SET_FLAG(adapter, fMP_ADAPTER_HARDWARE_ERROR) 
#define MP_SET_NON_RECOVER_ERROR(adapter) MP_SET_FLAG(adapter, fMP_ADAPTER_NON_RECOVER_ERROR)

#define MP_OFFSET(field)   ((UINT)FIELD_OFFSET(MP_ADAPTER,field))
#define MP_SIZE(field)     sizeof(((PMP_ADAPTER)0)->field)


//--------------------------------------
// Stall execution and wait with timeout
//--------------------------------------
/*++
    _condition  - condition to wait for 
    _timeout_ms - timeout value in milliseconds
    _result     - TRUE if condition becomes true before it times out
--*/
#define MP_STALL_AND_WAIT(_condition, _timeout_ms, _result)     \
{                                                               \
    int counter;                                                \
    _result = FALSE;                                            \
    for(counter = _timeout_ms * 50; counter != 0; counter--)    \
    {                                                           \
        if(_condition)                                          \
        {                                                       \
            _result = TRUE;                                     \
            break;                                              \
        }                                                       \
        KeStallExecutionProcessor(20);                                 \
    }                                                           \
}

__inline VOID MP_STALL_EXECUTION(
   IN ULONG MsecDelay) //Eliyas UINT to ULONG
{
    // Delay in 100 usec increments
    MsecDelay *= 10;
    while (MsecDelay)
    {
        KeStallExecutionProcessor(100);
        MsecDelay--;
    }
}

typedef struct _FDO_DATA FDO_DATA, *PFDO_DATA;


NTSTATUS
NICInitializeDeviceExtension(
    IN OUT PFDO_DATA FdoData
    );

NTSTATUS
NICGetDeviceInformation(
    IN OUT PFDO_DATA FdoData
);

NTSTATUS
NICAllocateDeviceResources(
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    );

NTSTATUS
NICMapHWResources(
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    );

NTSTATUS
NICUnmapHWResources(
    IN OUT PFDO_DATA FdoData
    );


NTSTATUS
NICFreeDeviceResources(
    IN OUT PFDO_DATA FdoData
    );

NTSTATUS 
NICInitializeAdapter(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS
NICReadAdapterInfo(
    IN PFDO_DATA FdoData
    );

NTSTATUS 
NICSelfTest(
    IN  PFDO_DATA     FdoData
    );

VOID 
HwSoftwareReset(
    IN  PFDO_DATA     FdoData
    );

BOOLEAN 
NICInterruptHandler(
    IN PKINTERRUPT Interupt, 
    IN PVOID ServiceContext
    );

#if 0
NTSTATUS
ReadWriteConfigSpace(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          ReadOrWrite, // 0 for read 1 for write
    IN PVOID          Buffer,
    IN ULONG          Offset,
    IN ULONG          Length
    );
#endif

NTSTATUS
GetPCIBusInterfaceStandard(
    IN  PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD    BusInterfaceStandard
    );


NTSTATUS 
NICAllocAdapterMemory(
    IN  PFDO_DATA     FdoData
    );

VOID 
NICFreeAdapterMemory(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS
NICAllocRfd(
    IN  PFDO_DATA     FdoData,
    IN  PMP_RFD         pMpRfd
    );

VOID 
NICFreeRfd(
    IN  PFDO_DATA     FdoData,
    IN  PMP_RFD         pMpRfd
    );

VOID 
NICReturnRFD(
    IN  PFDO_DATA FdoData,
    IN  PMP_RFD		pMpRfd
    );

NTSTATUS
HwConfigure(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS 
HwSetupIAAddress(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS 
HwClearAllCounters(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS 
NICInitRecv(
    IN  PFDO_DATA     FdoData
    );

VOID 
NICInitSend(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS 
NICLinkDetection(
    IN  PFDO_DATA                   FdoData
    );

NTSTATUS
NICHandleQueryOidRequest(
    IN  PFDO_DATA                   FdoData,
    IN  PIRP                        Irp,
    OUT PULONG                      BytesWritten
    );

NTSTATUS
NICHandleSetOidRequest(
    IN  PFDO_DATA                   FdoData,
    IN  PIRP                        Irp
    );


VOID 
NICServiceIndicateStatusIrp(
    IN PFDO_DATA        FdoData
    );


NTSTATUS 
NICGetStatsCounters(
    IN  PFDO_DATA     FdoData, 
    IN  NDIS_OID        Oid,
    OUT PULONG64        pCounter);
    
NTSTATUS 
NICSetPacketFilter(
    IN  PFDO_DATA     FdoData,
    IN  ULONG           PacketFilter);

NTSTATUS 
NICSetMulticastList(
    IN  PFDO_DATA     FdoData);
    
ULONG 
NICGetMediaConnectStatus(
    IN  PFDO_DATA     FdoData);
    
NTSTATUS 
NICWrite(
    IN  PFDO_DATA     FdoData,
    IN  PIRP          Irp
);

VOID 
NICWritePacket(
    IN  PFDO_DATA   FdoData,
    IN  PIRP    Irp,
    IN  BOOLEAN       bFromQueue
    );

NTSTATUS 
NICSendPacket(
    IN  PFDO_DATA     FdoData,
    IN  PMP_TCB         pMpTcb,
    IN  PSCATTER_GATHER_LIST   pFragList);

NTSTATUS 
NICStartSend(
    IN  PFDO_DATA     FdoData,
    IN  PMP_TCB         pMpTcb);;

VOID
NICProcessSGList(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,//unused
    IN  PSCATTER_GATHER_LIST    pSGL,
    IN  PVOID                   Context
    );


VOID 
NICDpcForIsr(
    IN PKDPC Dpc,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP  SystemArgument1,
    IN PVOID SystemArgument2
    );

NTSTATUS 
NICHandleSendInterrupt(
    IN  PFDO_DATA  FdoData
    );

VOID 
NICFreeQueuedSendPackets(
    IN  PFDO_DATA     FdoData
    );


VOID
NICFreeBusySendPackets(
    IN  PFDO_DATA  FdoData
    );

VOID
NICCompleteSendRequest(
    PFDO_DATA FdoData,
    PIRP Irp,
    NTSTATUS Status,
    ULONG   Information,
    BOOLEAN AtDispatchLevel
    );

VOID 
NICShutdown(
    IN  PFDO_DATA  FdoData
    );


VOID 
NICHandleRecvInterrupt(
    IN  PFDO_DATA  FdoData
    );

NTSTATUS 
NICStartRecv(
    IN  PFDO_DATA  FdoData
    );

VOID 
NICResetRecv(
    IN  PFDO_DATA   FdoData
    );

VOID
NICServiceReadIrps(
    PFDO_DATA FdoData,
    PMP_RFD *PacketArray,
    ULONG PacketArrayCount
    );



VOID 
NICWatchDogTimerDpc(
    IN  PVOID	    SystemSpecific1,
    IN  PVOID	    FunctionContext,
    IN  PVOID	    SystemSpecific2, 
    IN  PVOID	    SystemSpecific3
    );

BOOLEAN 
NICCheckForHang(
    IN  PFDO_DATA     FdoData
    );

NTSTATUS 
NICReset(
    IN PFDO_DATA FdoData
    );

MEDIA_STATE
NICIndicateMediaState(
    IN PFDO_DATA FdoData
    );


MEDIA_STATE 
NICGetMediaState(
    IN PFDO_DATA FdoData
    );

VOID 
NICExtractPMInfoFromPciSpace(
    PFDO_DATA FdoData,
    PUCHAR pPciConfig
    );

NTSTATUS
NICSetPower(
    PFDO_DATA     FdoData ,
    DEVICE_POWER_STATE   PowerState 
    );

VOID
NICFillPoMgmtCaps (
    IN PFDO_DATA                 FdoData, 
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    );

NTSTATUS
NICAddWakeUpPattern(
    IN PFDO_DATA  FdoData,
    IN PVOID        InformationBuffer, 
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded   
    );

NTSTATUS
NICRemoveWakeUpPattern(
    IN PFDO_DATA  FdoData,
    IN PVOID        InformationBuffer, 
    IN UINT         InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    );

VOID
NICRemoveAllWakeUpPatterns(
    PFDO_DATA FdoData
    );

NTSTATUS
NICConfigureForWakeUp(
    IN PFDO_DATA FdoData,
    IN BOOLEAN  AddPattern
    );

VOID
NICGetDeviceInfSettings(
    IN OUT  PFDO_DATA   FdoData
    );

VOID
NICAllocRfdWorkItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
);

VOID
NICFreeRfdWorkItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            Context
);

typedef 
USHORT
(*PREAD_PORT)(
    IN USHORT *Register
    );

typedef 
VOID
(*PWRITE_PORT)(
    IN USHORT *Register,
    IN USHORT  Value
    );


#endif

