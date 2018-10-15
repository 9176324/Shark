/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       hw.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __HW_H
#define __HW_H

//
//	New types and forward pointers...
//
typedef struct _HARDWARE_INFO          HARDWARE_INFO, *PHARDWARE_INFO;
typedef struct	_ADAPTER_BLOCK          ADAPTER_BLOCK, *PADAPTER_BLOCK;
typedef struct	_VC_BLOCK               VC_BLOCK, *PVC_BLOCK;
typedef struct	_RECV_BUFFER_HEADER     RECV_BUFFER_HEADER, *PRECV_BUFFER_HEADER;
	
typedef struct	_XMIT_SEG_INFO          XMIT_SEG_INFO, *PXMIT_SEG_INFO;
typedef struct	_SAR_INFO               SAR_INFO, *PSAR_INFO;

typedef struct	_XMIT_DMA_QUEUE         XMIT_DMA_QUEUE, *PXMIT_DMA_QUEUE;

typedef struct	_RECV_SEG_INFO          RECV_SEG_INFO, *PRECV_SEG_INFO;
typedef struct	_RECV_DMA_QUEUE         RECV_DMA_QUEUE, *PRECV_DMA_QUEUE;

typedef struct _RECV_BUFFER_INFO       RECV_BUFFER_INFO, *PRECV_BUFFER_INFO;

typedef struct _RECV_BUFFER_POOL       RECV_BUFFER_POOL, *PRECV_BUFFER_POOL;

#define BIT(x)                 (1 << (x))


//
//	VC range supported by the TbAtm155
//
#define MAX_VCS_1K             1024
#define MAX_VCS_4K             MAX_VCS_1K * 4
#define MIN_VCS                0

//
// The maximum entries in CBR schedle table
//
#define MAX_ENTRIES_CBR_TBL    4096

#define CELL_PAYLOAD_SIZE	    48
#define CELL_HEADER_SIZE	    4           // Excepting the HEC

#define MAX_AAL5_PDU_SIZE      (20 * 1024)


//
//	RAM memory block supported.
//
#define BLOCK_1K        1024
#define BLOCK_2K        BLOCK_1K * 2
#define BLOCK_4K        BLOCK_1K * 4
#define BLOCK_8K        BLOCK_1K * 8
#define BLOCK_10K       BLOCK_1K * 10
#define BLOCK_15K       BLOCK_1K * 15
#define BLOCK_16K       BLOCK_1K * 16
#define BLOCK_32K       BLOCK_1K * 32
#define BLOCK_64K       BLOCK_1K * 64
#define BLOCK_128K      BLOCK_1K * 128


//
//	Maximum number of CBR VC supported to be activated
//	in this drivers
//
#define MAX_SUPPORTED_CBR_VC   26


//
//	Maximum number of times we will loop handling
//	interrupt status codes in TbAtm155HandleInterrupt
//
#define MAXIMUM_INTERRUPTS	    4



//
//	DMA capabilities.
//
#define MAXIMUM_MAP_REGISTERS			    MAX_VCS_4K
#define MINIMUM_MAP_REGISTERS			    32
#define DEFAULT_TOTAL_RECEIVE_BUFFERS		300
// Note: make sure don't be larger than 25.
#define DEFAULT_RECEIVE_BUFFERS		    20              
//
//  For SlotTag reason, so new, limited the maximum number of receive buffers 
//  to be this number.
//
#define MAXIMUM_RECEIVE_BUFFERS            4000
#define MINIMUM_RECEIVE_BUFFERS		    100

#define DEFAULT_RECEIVE_BIG_BUFFER_SIZE	(1024 * 10)
#define DEFAULT_RECEIVE_SMALL_BUFFER_SIZE	(1024 * 2)

#define NIC_MAX_PHYSICAL_BUFFERS		    20

#ifdef _WIN64
#define TBATM155_MIN_DMA_ALIGNMENT		    8
#else
#define TBATM155_MIN_DMA_ALIGNMENT		    4
#endif



//
// Definitions of Adapter's configuration
//     Types of Toshiba Meteors
//     Types of PHY controllers
//
#define TBATM155_METEOR_4KVC       BIT(7)
#define TBATM155_METEOR_1KVC       BIT(6)

#define TBATM155_PHY_SUNI_LITE     BIT(3)
#define TBATM155_PHY_PLC_2         BIT(2)

//
//	Interface Speeds
//
#define ATM_155				    (149760000)

//
// NIC data rate
//
#define CELL_SIZE                  (53 * 8)
#define TBATM155_MAX_BANDWIDTH     (ATM_155 / CELL_SIZE)
#define TBATM155_MIN_CBR_BANDWIDTH (8000 / CELL_SIZE)
#define TBATM155_MIN_ABR_BANDWIDTH (4200 / CELL_SIZE)
#define TBATM155_CELL_CLOCK_RATE   (TBATM155_MAX_BANDWIDTH / MAX_ENTRIES_CBR_TBL)


// 5208 cells/second.

//
//	PCI id's
//
#define TOSHIBA_PCI_VENDOR_ID				0x102F
#define TBATM155_PCI_DEVICE_ID				0x0020


// #define PCI_DEVICE_SPECIFIC_OFFSET	0x40

typedef	union	_PCI_DEVICE_CONFIG
{
	struct
	{
		UCHAR	SoftwareReset:1;
		UCHAR	EnableInterrupt:1;
		UCHAR	TargetSwapBytes:1;
		UCHAR	MasterSwapBytes:1;
		UCHAR	EnableIncrement:1;
		UCHAR	SoftwareInterrupt:1;
		UCHAR	TestDMA:1;
		UCHAR	Reserved:1;
	};

	UCHAR	reg;
}
	PCI_DEVICE_CONFIG,
	*PPCI_DEVICE_CONFIG;

typedef	union	_PCI_DEVICE_STATUS
{
	struct
	{
		UCHAR	VoltageSense:1;
		UCHAR	IllegalByteEnable:1;
		UCHAR	IllegalWrite:1;
		UCHAR	IllegalOverlap:1;
		UCHAR	IllegalDescriptor:1;
		UCHAR	Reserved:3;
	};

	UCHAR	reg;
}
	PCI_DEVICE_STATUS,
	*PPCI_DEVICE_STATUS;

typedef	union	_PCI_DEVICE_INTERRUPT_STATUS
{
	struct
	{
		UCHAR	DprInt:1;
		UCHAR	reserved:2;
		UCHAR	StaInt:1;
		UCHAR	RtaInt:1;
		UCHAR	RmaInt:1;
		UCHAR	SseInt:1;
		UCHAR	DpeInt:1;
	};

	UCHAR	reg;
}
	PCI_DEVICE_INTERRUPT_STATUS,
	*PPCI_DEVICE_INTERRUPT_STATUS;

typedef	union	_PCI_DEVICE_ENABLE_PCI_INTERRUPT
{
	struct
	{
		UCHAR	EnableDprInt:1;
		UCHAR	reserved:2;
		UCHAR	EnableStaInt:1;
		UCHAR	EnableRtaInt:1;
		UCHAR	EnableRmaInt:1;
		UCHAR	EnableSseInt:1;
		UCHAR	EnableDpeInt:1;
	};

	UCHAR	reg;
}
	PCI_DEVICE_ENABLE_PCI_INTERRUPT,
	*PPCI_DEVICE_ENABLE_PCI_INTERRUPT;

typedef	union	_PCI_DEVICE_GENERAL_PURPOSE_IO_REGISTERS
{
	struct
	{
		UCHAR	GPIOREG0:1;
		UCHAR	GPIOREG1:1;
		UCHAR	GPIOREG2:1;
		UCHAR	GPIOREG3:1;
		UCHAR	reserved:4;
	};

	UCHAR	reg;
}
	PCI_DEVICE_GENERAL_PURPOSE_IO_REGISTERS,
	*PPCI_DEVICE_GENERAL_PURPOSE_IO_REGISTERS;

typedef	union	_PCI_DEVICE_GENERAL_PURPOSE_IOCTL
{
	struct
	{
		UCHAR	GPIOCTL0:1;
		UCHAR	GPIOCTL1:1;
		UCHAR	GPIOCTL2:1;
		UCHAR	GPIOCTL3:1;
		UCHAR	reserved:4;
	};

	UCHAR	reg;
}
	PCI_DEVICE_GENERAL_PURPOSE_IOCTL,
	*PPCI_DEVICE_GENERAL_PURPOSE_IOCTL;

typedef	union	_PCI_DEVICE_DMA_CONTROL
{
	struct
	{
		UCHAR	StopOnPerr:1;
		UCHAR	DualAddressCycleEnable:1;
		UCHAR	CacheThresholdEnable:1;
		UCHAR	MemoryReadCmdEnable:1;
		UCHAR	Reserved:4;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DMA_CONTROL,
	*PPCI_DEVICE_DMA_CONTROL;

typedef	union	_PCI_DEVICE_DMA_STATUS
{
	struct
	{
		UCHAR	FifoEmpty:1;
		UCHAR	FifoFull:1;
		UCHAR	FifoThreshold:1;
		UCHAR	HostDmaDone:1;
		UCHAR	FifoCacheThreshold:1;
		UCHAR	Reserved:3;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DMA_STATUS,
	*PPCI_DEVICE_DMA_STATUS;

typedef	union	_PCI_DEVICE_DMA_DIAG
{
	struct
	{
		UCHAR	HostDmaEnable:1;
		UCHAR	DataPathDirection:1;
		UCHAR	Reserved:6;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DMA_DIAG,
	*PPCI_DEVICE_DMA_DIAG;

typedef	union	_PCI_DEVICE_HOST_COUNT
{
	struct
	{
		ULONG	hcLowLow:8;
		ULONG	hcLowHigh:8;
		ULONG	hcHighLow:8;
		ULONG	Reserved:8;
	};

	ULONG	reg;
}
	PCI_DEVICE_HOST_COUNT,
	*PPCI_DEVICE_HOST_COUNT;

typedef	union	_PCI_DEVICE_DATA_FIFO_READ_ADDRESS
{
	struct
	{
	   UCHAR	MasterFifoPointer:7;
	   UCHAR	Reserved:1;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DATA_FIFO_READ_ADDRESS,
	*PPCI_DEVICE_DATA_FIFO_READ_ADDRESS;

typedef union  _PCI_DEVICE_DATA_FIFO_WRITE_ADDRESS
{
	struct
	{
		UCHAR	MasterFifoPointer:7;
		UCHAR	Reserved:1;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DATA_FIFO_WRITE_ADDRESS,
	*PPCI_DEVICE_DATA_FIFO_WRITE_ADDRESS;

typedef	union	_PCI_DEVICE_DATA_FIFO_THRESHOLD
{
	struct
	{
		UCHAR	Reserved0:2;
		UCHAR	DFTHRSH:4;
		UCHAR	Reserved1:2;
	};

	UCHAR	reg;
}
	PCI_DEVICE_DATA_FIFO_THRESHOLD,
	*PPCI_DEVICE_DATA_FIFO_THRESHOLD;

typedef union  _PCI_DEVICE_LOW_HOST_ADDRESS
{
	struct
	{
		UCHAR	lhaLowLow;
		UCHAR	lhaLowHigh;
		UCHAR	lhaHighLow;
		UCHAR	lhaHighHigh;
	};

	ULONG	reg;
}
	PCI_DEVICE_LOW_HOST_ADDRESS,
	*PPCI_DEVICE_LOW_HOST_ADDRESS;

typedef	union	_PCI_DEVICE_HIGH_HOST_ADDRESS
{
	struct
	{
		ULONG	hhaLowLow:8;
		ULONG	hhaLowHigh:8;
		ULONG	hhaHighLow:8;
		ULONG	hhaHighHigh:8;
	};

	ULONG	reg;
}
	PCI_DEVICE_HIGH_HOST_ADDRESS,
	*PPCI_DEVICE_HIGH_HOST_ADDRESS;

typedef	union	_PCI_DEVICE_FIFO_DATA_REGISTER
{
	struct
	{
		ULONG	dfrLowLow:8;
		ULONG	dfrLowHigh:8;
		ULONG	dfrHighLow:8;
		ULONG	dfrHighHigh:8;
	};

	ULONG	reg;
}
	PCI_DEVICE_FIFO_DATA_REGISTER,
	*PPCI_DEVICE_FIFO_DATA_REGISTER;

typedef	union	_PCI_DEVICE_DATA_ADDRESS
{
	struct
	{
		ULONG	daLowLow:8;
		ULONG	daLowHigh:8;
		ULONG	daHighLow:8;
		ULONG	daHighHigh:8;
	};

	ULONG	reg;
}
	PCI_DEVICE_DATA_ADDRESS,
	*PPCI_DEVICE_DATA_ADDRESS;

typedef	union	_PCI_DEVICE_DATA_PORT
{
	struct
	{
		ULONG	dpLowLow:8;
		ULONG	dpLowHigh:8;
		ULONG	dpHighLow:8;
		ULONG	dpHighHigh:8;
	};

	ULONG	reg;
}
	PCI_DEVICE_DATA_PORT,
	*PPCI_DEVICE_DATA_PORT;


#define PCI_DEVICE_CONFIG_OFFSET			0x40
#define PCI_DEVICE_STATUS_OFFSET			0x41
#define PCI_INTERRUPT_STATUS_OFFSET		0x44
#define PCI_ENABLE_INTERRUPT_OFFSET		0x45
#define PCI_GENERAL_PURPOSE_IO_PORT_OFFSET	0x46
#define PCI_GENERAL_PURPOSE_IOCTL_OFFSET	0x47
#define PCI_DMA_CONTROL_OFFSET				0x4C
#define PCI_DMA_STATUS_OFFSET				0x4D
#define PCI_DMA_DIAGNOSTIC_OFFSET			0x4E
#define PCI_HOST_COUNT0_OFFSET				0x50
#define PCI_HOST_COUNT1_OFFSET				0x51
#define PCI_HOST_COUNT2_OFFSET				0x52
#define PCI_DATA_FIFO_READ_ADDRESS_OFFSET	0x54
#define PCI_DATA_FIFO_WRITE_ADDRESS_OFFSET	0x55
#define PCI_DATA_FIFO_THRESHOLD_OFFSET		0x56
#define PCI_LOW_HOST_ADDRESS0_OFFSET		0x58
#define PCI_LOW_HOST_ADDRESS1_OFFSET		0x59
#define PCI_LOW_HOST_ADDRESS2_OFFSET		0x5A
#define PCI_LOW_HOST_ADDRESS3_OFFSET		0x5B
#define PCI_HIGH_HOST_ADDRESS0_OFFSET		0x5C
#define PCI_HIGH_HOST_ADDRESS1_OFFSET		0x5D
#define PCI_HIGH_HOST_ADDRESS2_OFFSET		0x5E
#define PCI_HIGH_HOST_ADDRESS3_OFFSET		0x5F
#define PCI_FIFO_DATA_REGISTER0_OFFSET		0x60
#define PCI_FIFO_DATA_REGISTER1_OFFSET		0x61
#define PCI_FIFO_DATA_REGISTER2_OFFSET		0x62
#define PCI_FIFO_DATA_REGISTER3_OFFSET		0x63


#define SET_PCI_DEV_CFG(_HwInfo, _reg)					NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DEVICE_CONFIG_OFFSET), (_reg))
#define GET_PCI_DEV_CFG(_HwInfo, _reg)					NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DEVICE_CONFIG_OFFSET), (PUCHAR)(_reg))
			

#define SET_PCI_DEV_STATUS(_HwInfo, _reg)				NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DEVICE_STATUS_OFFSET), (_reg))
#define GET_PCI_DEV_STATUS(_HwInfo, _reg)				NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DEVICE_STATUS_OFFSET), (PUCHAR)(_reg))
		
#define SET_PCI_DEV_INT_STATUS(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_INTERRUPT_STATUS_OFFSET), (_reg))
#define GET_PCI_DEV_INT_STATUS(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_INTERRUPT_STATUS_OFFSET), (PUCHAR)(_reg))
		
#define SET_PCI_DEV_ENABLE_INT(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_ENABLE_INTERRUPT_OFFSET), (_reg))
#define GET_PCI_DEV_ENABLE_INT(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_ENABLE_INTERRUPT_OFFSET), (PUCHAR)(_reg))
		
		
#define SET_PCI_DEV_GP_IO_REG(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_GENERAL_PURPOSE_IO_PORT_OFFSET), (_reg))
#define GET_PCI_DEV_GP_IO_REG(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_GENERAL_PURPOSE_IO_PORT_OFFSET), (PUCHAR)(_reg))
		
#define SET_PCI_DEV_GP_IOCTL(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_GENERAL_PURPOSE_IOCTL_OFFSET), (_reg))
#define GET_PCI_DEV_GP_IOCTL(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_GENERAL_PURPOSE_IOCTL_OFFSET), (PUCHAR)(_
		
#define SET_PCI_DEV_DMA_CONTROL(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_CONTROL_OFFSET), (_reg))
#define GET_PCI_DEV_DMA_CONTROL(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_CONTROL_OFFSET), (PUCHAR)(_reg))
		
#define SET_PCI_DEV_DMA_STATUS(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_STATUS_OFFSET), (_reg))
#define GET_PCI_DEV_DMA_STATUS(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_STATUS_OFFSET), (PUCHAR)(_reg))
		
		
#define SET_PCI_DEV_DMA_DIAG(_HwInfo, _reg)			NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_DIAGNOSTIC_OFFSET), (_reg))
#define GET_PCI_DEV_DMA_DIAG(_HwInfo, _reg)			NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DMA_DIAGNOSTIC_OFFSET), (PUCHAR)(_reg))
		
#define SET_PCI_DEV_DATA_READ_ADDRESS(_HwInfo, _reg)	NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_READ_ADDRESS_OFFSET), (_reg))
#define GET_PCI_DEV_DATA_READ_ADDRESS(_HwInfo, _reg)	NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_READ_ADDRESS_OFFSET), (PUCHAR)(_reg))

#define SET_PCI_DEV_DATA_WRITE_ADDRESS(_HwInfo, _reg)	NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_WRITE_ADDRESS_OFFSET), (_reg))
#define GET_PCI_DEV_DATA_WRITE_ADDRESS(_HwInfo, _reg)	NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_WRITE_ADDRESS_OFFSET), (PUCHAR)(_reg))

#define SET_PCI_DEV_DATA_THRESHOLD(_HwInfo, _reg)		NdisWriteRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_THRESHOLD_OFFSET), (_reg))
#define GET_PCI_DEV_DATA_THRESHOLD(_HwInfo, _reg)		NdisReadRegisterUchar(((_HwInfo)->PciConfigSpace + PCI_DATA_FIFO_THRESHOLD_OFFSET), (PUCHAR)(_reg))


//
//	This structure is used to build the AAL5 trailer.
//
typedef struct _AAL5_PDU_TRAILER
{
	union
	{
		struct
		{
			UCHAR	UserToUserIndication;
			UCHAR	CommonPartIndicator;
			USHORT	Length;
			ULONG	CRC;
		};

		struct
		{
			ULONG	Register0;
			ULONG	Register1;
		};
	};
}
	AAL5_PDU_TRAILER,
	*PAAL5_PDU_TRAILER;


#define MAX_PADDING_BYTES  47
#define MAX_APPEND_BYTES   MAX_PADDING_BYTES + sizeof(AAL5_PDU_TRAILER)


#endif // __HW_H



