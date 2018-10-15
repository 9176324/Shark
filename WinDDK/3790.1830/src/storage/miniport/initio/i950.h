/*************************************************************************
		    Copyright (c) 1994-1995 Initio Corp.
    Initio Windows NT/95/97 device driver are copyrighted by Initio Corporation.
    Your rights of ownership are subject to the limitations and restrictions
    imposed by the copyright laws. Initio makes no warranty  of any kind in
    regard to this material, including,  but not  limited to,  the  implied
    warranties of merchantability and fitness  for  a  particular  purpose.
    Initio is not liable for any errors contained  herein  or incidental or
    consequential damages in connection with furnishing, performance or use
    of this material.
  -----------------------------------------------------------------------
 Module: i950.h
 Description: winNT/win95/win97 mini port driver 
 History:     02/27/97 TC ver 2.04 release to Microsoft Memphis team
	  03/11/97 TC ver 2.05 delete MAX_PCI_CHANL,Since _HW_DEVICE_EXTENSION
	      is allocated by OS, so Windows NT and Windows 95 can
	      to support unlimited PCI chanl and 9400 crad
      03/26/97 TC    add LinkList support
*************************************************************************/
#ifndef _I950_H_
#define _I950_H_


#include "miniport.h"           /*  from NT DDK */
#include "scsi.h"               /*  from NT DDK */

#define  TULDBG          0       
#define  DISABLE_ISA     0

#define  LINKLIST        1
#define  SG_SUPPORT      1
#define  TAG_QUEUING     1


#define ULONG           unsigned long
#define ulong           unsigned long
#define uint            unsigned long
#define USHORT          unsigned short
#define ushort          unsigned short
#define UWORD           unsigned short
#define uword           unsigned short
#define UCHAR           unsigned char
#define uchar           unsigned char
#define UBYTE           unsigned char
#define ubyte           unsigned char

#ifndef NULL
#define NULL     0            /* zero          */
#endif
#ifndef TRUE
#define TRUE     (1)          /* boolean true  */
#endif
#ifndef FALSE
#define FALSE    (0)          /* boolean false */
#endif

#define isodd_word( val )   ( ( ( ( uint )val) & ( uint )0x0001 ) != 0 )



/******************************************************************
    Serial EEProm
*******************************************************************/
/* bit definition for TUL_SE2 */
#define         SE2CS           0x008
#define         SE2CLK          0x004
#define         SE2DO           0x002
#define         SE2DI           0x001

typedef struct _NVRAM {
    /*----------header ---------------*/
    USHORT  Signature;                              /* 0,1: Signature */
    UCHAR   Size;                                   /* 2:   Size of data structure */
    UCHAR   Revision;                               /* 3:   Revision of data structure */
    /* ----Host Adapter Structure ----*/
    UCHAR   ModelByte0;                             /* 4:   Model number (byte 0) */
    UCHAR   ModelByte1;                             /* 5:   Model number (byte 1) */
    UCHAR   ModelInfo;                              /* 6:   Model information         */
    UCHAR   NumOfCh;                                /* 7:   Number of SCSI channel*/
    UCHAR   BIOSConfig1;                    /* 8:   BIOS configuration 1  */
    UCHAR   BIOSConfig2;                    /* 9:   BIOS configuration 2  */
    UCHAR   HAConfig1;                              /* A:   Hoat adapter configuration 1 */
    UCHAR   HAConfig2;                              /* B:   Hoat adapter configuration 2 */
    /* ----SCSI channel Structure ---- */
    /* from "CTRL-I SCSI Host Adapter SetUp menu "  */
    UCHAR   SCSIid;                                 /* C:   Channel SCSIId;           */
    UCHAR   SCSIconfig1;                    /* D:   Channel SCSI config 1 */
    UCHAR   SCSIconfig2;                    /* E:   Channel SCSI config 2 */
    UCHAR   NumSCSItarget;                  /* F:   # of SCSI target      */
    /* ----SCSI target Structure ----  */
    /* from "CTRL-I SCSI device SetUp menu "                                          */
    UCHAR   Target0Config;                  /* 0x10:Target 0 configuration          */
    UCHAR   Target1Config;                  /* 0x11:Target 1 configuration      */
    UCHAR   Target2Config;                  /* 0x12:Target 2 configuration      */
    UCHAR   Target3Config;                  /* 0x13:Target 3 configuration      */
    UCHAR   Target4Config;                  /* 0x14:Target 4 configuration      */
    UCHAR   Target5Config;                  /* 0x15:Target 5 configuration      */
    UCHAR   Target6Config;                  /* 0x16:Target 6 configuration      */
    UCHAR   Target7Config;                  /* 0x17:Target 7 configuration      */
    UCHAR   reserved[38];                   /* for SCSI channel 1-3             */
    /* ---------- CheckSum ----------       */
    USHORT  CheckSum;                       /* 0x3E, 0x3F: Checksum of NVRam        */
    } NVRAM, *PNVRAM;

/* Bios Configuration for nvram->BIOSConfig1                            */
#define NBC1_ENABLE             0x01    /* BIOS enable                      */
#define NBC1_8DRIVE             0x02    /* Support more than 2 drives       */
#define NBC1_REMOVABLE          0x04    /* Support removable drive                      */
#define NBC1_INT19              0x08    /* Intercept int 19h                */
#define NBC1_NOBIOSSCAN         0x10    /* Dynamic BIOS scan                */
#define NBC1_DISCONNECT         0x20    /* Enable SCSI disconnect           */

/* Bit definition for nvram->SCSIconfig1                                */
#define NCC1_BUSRESET           0x01    /* Reset SCSI bus at power up       */
#define NCC1_PARITYCHK          0x02    /* SCSI parity enable                           */
#define NCC1_ACTTERM1           0x04    /* Enable active terminator 1       */
#define NCC1_ACTTERM2           0x08    /* Enable active terminator 2       */
#define NCC1_AUTOTERM           0x10    /* Enable auto termination */
#define NCC1_PWRMGR             0x80    /* Enable power management          */

/* Bit definition for nvram->TargetxConfig                              */
#define NTC_SYNC                0x01    /* SYNC_NEGO                        */
#define NTC_FASTSCSI            0x02    /* Fast SCSI                        */
#define NTC_1GIGA               0x04    /* 255 head / 63 sectors (64/32)    */
#define NTC_SPINUP              0x08    /* Start disk drive                 */
#define NTC_NOBIOSSCAN          0x10    /* Under dynamic BIOS scan          */

/*      Default NVRam values                                                */
#define INI_SIGNATURE           0xC925
#define NBC1_DEFAULT            (NBC1_ENABLE | NBC1_NOBIOSSCAN)
#define NCC1_DEFAULT            (NCC1_BUSRESET | NCC1_ACTTERM1 |NCC1_ACTTERM2 | NCC1_AUTOTERM | NCC1_PARITYCHK)
#define NTC_DEFAULT             (NTC_FASTSCSI | NTC_SPINUP)

/*  SCSI related definition               */
#define DISC_ALLOW                      0x40
#define SCSICMD_RequestSense            0x03


/*------------------------------------------------------------------*/
/*                              PCI                                                             */
/*------------------------------------------------------------------*/
#define PCI_FUNCTION_ID         0xB1
#define PCI_BIOS_PRESENT        0x01
#define FIND_PCI_DEVICE         0x02
#define FIND_PCI_CLASS_CODE     0x03
#define GENERATE_SPECIAL_CYCLE  0x06
#define READ_CONFIG_BYTE        0x08
#define READ_CONFIG_WORD        0x09
#define READ_CONFIG_DWORD       0x0A
#define WRITE_CONFIG_BYTE       0x0B
#define WRITE_CONFIG_WORD       0x0C
#define WRITE_CONFIG_DWORD      0x0D

#define SUCCESSFUL              0x00
#define FUNC_NOT_SUPPORTED      0x81
#define BAD_VENDOR_ID           0x83    /* Bad vendor ID                */
#define DEVICE_NOT_FOUND        0x86    /* PCI device not found         */
#define BAD_REGISTER_NUMBER     0x87

#define MAX_PCI_DEVICES         21      /* Maximum devices supportted   */
/* #define MAX_PCI_CHANL           12       support 3 ini-9400 */

#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned long

typedef void(* CALL_FUNCTION)();        /* Function type                */

typedef struct _BIOS32_ENTRY_STRUCTURE {
    DWORD Signatures;                       /* Should be "_32_"             */
    DWORD BIOS32Entry;                      /* 32-bit physical address      */
    BYTE  Revision;                 /* Revision level, should be 0  */
    BYTE  Length;                   /* Multiply of 16, should be 1  */
    BYTE  CheckSum;                 /* Checksum of whole structure  */
    BYTE  Reserved[5];                      /* Reserved                     */
} BIOS32_ENTRY_STRUCTURE, *PBIOS32_ENTRY_STRUCTURE;

typedef struct
   {
   union
      {
      unsigned int eax;
      struct
     {
     unsigned short ax;
     } word;
      struct
     {
     unsigned char al;
     unsigned char ah;
     } byte;
      } eax;
   union
      {
      unsigned int ebx;
      struct
     {
     unsigned short bx;
     } word;
      struct
     {
     unsigned char bl;
     unsigned char bh;
     } byte;
      } ebx;
   union
      {
      unsigned int ecx;
      struct
     {
     unsigned short cx;
     } word;
      struct
     {
     unsigned char cl;
     unsigned char ch;
     } byte;
      } ecx;
   union
      {
      unsigned int edx;
      struct
     {
     unsigned short dx;
     } word;
      struct
     {
     unsigned char dl;
     unsigned char dh;
     } byte;
      } edx;
   union
      {
      unsigned int edi;
      struct
     {
     unsigned short di;
     } word;
      } edi;
   union
      {
      unsigned int esi;
      struct
     {
     unsigned short si;
     } word;
      } esi;
   } REGS;

typedef union                           /* Union define for mechanism 1 */
    {
    struct
    {
    unsigned char RegNum;
    unsigned char FcnNum:3;
    unsigned char DeviceNum:5;
    unsigned char BusNum;
    unsigned char Reserved:7;
    unsigned char Enable:1;
    }               sConfigAdr;
    unsigned long   lConfigAdr;
    } CONFIG_ADR;

typedef union                           /* Union define for mechanism 2 */
    {
    struct
    {
    unsigned char RegNum;
    unsigned char DeviceNum;
    unsigned short Reserved;
    }               sHostAdr;
    unsigned long   lHostAdr;
    } HOST_ADR;

typedef struct _HCSinfo {
    ULONG  base;
    UCHAR  vec;
    UCHAR  bios; /* High byte of BIOS address */
    USHORT BaseAndBios;   /* high byte: pHcsInfo->bios,low byte:pHcsInfo->base */
    } HCSINFO ;
/*
typedef struct _TargetInfo {
    ULONG ID;
    ULONG scbCnt;
    }TARGETINFO;
*/      

/*------------------------------------------------------*/
/*                Tulip                                 */
/*------------------------------------------------------*/
/* from tid and lun to taregt_ix */
#define TUL_TIDLUN_TO_IX( tid, lun )  ( uchar )( (tid) + ((lun)<<3) )
/* from tid to target_id */
#define TUL_TID_TO_TARGET_ID( tid )   ( uchar )( 0x01 << (tid) )

#define MAX_OFFSET      15 
/**********************************************************/
/*          Tulip Configuration Register Set              */
/**********************************************************/
#define TUL_PVID        0x00            /* Vendor ID                    */
    #define TUL_VENDOR_ID           0x1101  /* Tulip vendor ID     */

#define TUL_PDID        0x02            /* Device ID                    */
    #define TUL_DEVICE_ID           0x9500  /* Tulip device ID     */

#define TUL_COMMAND     0x04            /* Command                              */
    /* bit definition for Command register of Configuration Space Header */
    #define BUSMS           0x04                    /* BUS MASTER Enable  */
    #define IOSPA           0x01                    /* IO Space Enable      */
#define TUL_STATUS              0x06                    /* status register */
#define TUL_REVISION    0x08            /* Revision number              */
#define TUL_BASE        0x10            /* Base address                 */
#define TUL_BIOS        0x30            /* Expansion ROM base address   */
#define TUL_INT_NUM     0x3C            /* Interrupt line               */
#define TUL_INT_PIN     0x3D            /* Interrupt pin                */


/**********************************************************/
/*                    Tulip Register Set                  */
/**********************************************************/
#define TUL_HACFG0      0x40    /* H/A Configuration Register 0         */
#define TUL_HACFG1      0x41    /* H/A Configuration Register 1         */
    #define WIDE_SCSI_CARD  0x40 /* bit 6 tighed to high if wide card   */
#define TUL_HACFG2      0x42    /* H/A Configuration Register 2         */
#define TUL_HACFG3      0x43    /* H/A Configuration Register 3         */

#define TUL_SDCFG0      0x44    /* SCSI Device Configuration 0          */
#define TUL_SDCFG1      0x45    /* SCSI Device Configuration 1          */
#define TUL_SDCFG2      0x46    /* SCSI Device Configuration 2          */
#define TUL_SDCFG3      0x47    /* SCSI Device Configuration 3          */

#define TUL_GINTS       0x50    /* Global Interrupt Status Register     */
#define TUL_GIMSK       0x52    /* Global Interrupt MASK Register       */

#define TUL_GCTRL       0x54    /* Global Control Register                      */
    /* ; bit definition for TUL_GCTRL */
    #define EXTSG           0x80
    #define EXTAD           0x60
    #define SEG4K           0x08
    #define EEPRG           0x04
    #define MRMUL           0x02

#define TUL_GCTRLH      0x55    /* Global Control Register high byte    */
    #define ATDEN           0x01    /* Autotermination detection enable */

#define TUL_GSTAT       0x56    /* Global Status Register */

#define TUL_DMACFG              0x5B    /* DMA configuration */
#define TUL_NVRAM       0x5D    /* Nvram port address */


/**********************************************************/
/*            Pass-Through SCSI Register Set              */
/**********************************************************/
#define TUL_SCnt0               0x80    /* 00 R/W Transfer Counter Low  */
#define TUL_SCnt1               0x81    /* 01 R/W Transfer Counter Mid  */
#define TUL_SCnt2               0x82    /* 02 R/W Transfer Count High   */
#define TUL_SFifoCnt    0x83    /* 03 R   FIFO counter                  */
#define TUL_SIntEnable  0x84    /* 03 W   Interrupt enble               */

#define TUL_SInt        0x84    /* 04 R   Interrupt Status Register    */
    /* bit definition for Tulip SCSI Interrupt Register */
    #define TSS_RESEL_INT           0x80            /* Reselected interrupt  */
    #define TSS_SEL_TIMEOUT         0x40            /* Selected/reselected timeout */
    #define TSS_BUS_SERV            0x20   
    #define TSS_SCSIRST_INT         0x10            /* SCSI bus reset detected */
    #define TSS_DISC_INT            0x08            /* Disconnected interrupt  */
    #define TSS_SEL_INT             0x04            /* Select interrupt        */
    #define TSS_SCAM_SEL            0x02            /* SCAM selected                   */
    #define TSS_FUNC_COMP           0x01

#define TUL_SCtrl0              0x85    /* 05 W   Control 0                             */
    /*bit definition for Tulip SCSI Control 0 Register */
    #define TSC_FLUSH_FIFO          0x50            /* Clear FIFO & reset OFFset counter */
    #define TSC_FLUSHFIFO           0x10            /* Clear FIFO  */
    #define TSC_RST_SEQ                     0x20            /* Reset sequence counter       */
    #define TSC_ABT_CMD                     0x04            /* Abort command (sequence)     */
    #define TSC_RST_CHIP            0x02            /* Reset SCSI Chip                      */
    #define TSC_RST_BUS                     0x01            /* Reset SCSI Bus                       */

#define TUL_SStatus0    0x85    /* 05 R   Status 0                              */
    /* bit definition for Tulip SCSI Status 0 Register                                      */
    #define TSS_INT_PENDING         0x80            /* Interrupt pending            */
    #define TSS_SEQ_ACTIVE          0x40            /* Sequencer active                     */
    #define TSS_XFER_CNT            0x20            /* Transfer counter zero        */
    #define TSS_FIFO_EMPTY          0x10            /* FIFO empty                           */
    #define TSS_PAR_ERROR           0x08            /* SCSI parity error            */
    #define TSS_PH_MASK                     0x07            /* SCSI phase mask                      */

#define TUL_SCtrl1              0x86    /* 06 W   Control 1                             */
    /* bit definition for Tulip SCSI Control 1 Register                                     */
    #define TSC_EN_SCAM             0x80            /* Enable SCAM                          */
    #define TSC_NIDARB              0x40            /* no ID for arbitration    */
    #define TSC_EN_LOW_RESELECT     0x20            /* Enable low level reselect   */
    #define TSC_PWDN                0x10            /* Power down mode                      */
    #define TSC_WIDE_CPU            0x08            /* Wide CPU                                     */
    #define TSC_HW_RESELECT         0x04            /* Enable HW reselect           */
    #define TSC_EN_BUS_OUT          0x02            /* Enable SCSI data bus out latch */
    #define TSC_EN_BUS_IN           0x01            /* Enable SCSI data bus in latch  */

#define TUL_SStatus1    0x86    /* 06 R   Status 1                              */
    /* bit definition for Tulip SCSI Status 1 Register                                      */
    #define TSS_STATUS_RCV          0x80            /* Status received                      */
    #define TSS_MSG_SEND            0x40            /* Message sent                         */
    #define TSS_CMD_PH_CMP          0x20            /* Data phase done                      */
    #define TSS_DATA_PH_CMP         0x10            /* Data phase done                      */
    #define TSS_STATUS_SEND         0x08            /* Status sent                          */
    #define TSS_XFER_CMP            0x04            /* Transfer completed           */
    #define TSS_SEL_CMP             0x02            /* Selection completed          */
    #define TSS_ARB_CMP             0x01            /* Arbitration completed        */

#define TUL_SConfig             0x87    /* 07 W   Configuration                 */
    /* bit definition for Tulip SCSI Configuration Register                         */
    #define TSC_EN_LATCH            0x80            /* Enable phase latch           */
    #define TSC_INITIATOR           0x40            /* Initiator mode                       */
    #define TSC_EN_SCSI_PAR         0x20            /* Enable SCSI parity           */
    #define TSC_DMA_8BIT            0x10            /* Alternate dma 8-bits mode */
    #define TSC_DMA_16BIT           0x08            /* Alternate dma 16-bits mode */
    #define TSC_EN_WDACK            0x04            /* Enable DACK while wide SCSI xfer      */
    #define TSC_ALT_PERIOD          0x02            /* Alternate sync period (Fast 20) mode */
    #define TSC_DIS_SCSIRST         0x01            /* Disable SCSI bus reset us  */
    #define TSC_INITDEFAULT         TSC_INITIATOR | TSC_EN_LATCH | TSC_ALT_PERIOD | TSC_DIS_SCSIRST
    #define TSC_INITDEFAULT_ALT0    TSC_INITIATOR | TSC_EN_LATCH | TSC_DIS_SCSIRST
    #define TSC_WIDE_SCSI           0x80            /* Enable Wide SCSI                     */

#define TUL_SStatus2    0x87    /* 07 R   Status 2                              */
    /* bit definition for Tulip SCSI Status 2 Register  */
    #define TSS_CMD_ABTED           0x80            /* Command aborted      */
    #define TSS_OFFSET_0            0x40            /* Offset counter zero  */
    #define TSS_FIFO_FULL           0x20            /* FIFO full                    */
    #define TSS_TIMEOUT_0           0x10            /* Timeout counter zero */
    #define TSS_BUSY_RLS            0x08            /* Busy release                 */
    #define TSS_PH_MISMATCH         0x04            /* Phase mismatch               */
    #define TSS_SCSI_BUS_EN         0x02            /* SCSI data bus enable */
    #define TSS_SCSIRST                     0x01            /* SCSI bus reset in progress  */

#define TUL_SPeriodOffset       0x88    /* 08 W   wide/narrow,Sync. Transfer Period & Offset */
#define TUL_SOffsetRoomCtr      0x88    /* 08 R   Offset and room counter */
#define TUL_SScsiId             0x89    /* 09 W   SCSI ID                               */
#define TUL_SBusId              0x89    /* 09 R   SCSI BUS ID                   */
#define TUL_STimeOut    0x8A    /* 0A W   Sel/Resel Time Out Register */
#define TUL_SIdent              0x8A    /* 0A R   Identify Message Register       */
#define TUL_SAvail              0x8A    /* 0A R   Availiable Counter Register */
#define TUL_SData               0x8B    /* 0B R/W SCSI data in/out                        */
#define TUL_SFifo               0x8C    /* 0C R/W FIFO                                            */

#define TUL_SSignal             0x90    /* 10 R/W SCSI signal in/out              */
    /* bit definition for Tulip SCSI signal Register */
    #define TSC_RST_ACK             0x00            /* Release ACK signal */
    #define TSC_RST_ATN             0x00            /* Release ATN signal */
    #define TSC_RST_BSY             0x00            /* Release BSY signal */

    #define TSC_SET_REQ             0x80            /* REQ signal */
    #define TSC_SET_ACK             0x40            /* ACK signal */
    #define TSC_SET_BSY             0x20            /* BSY signal */
    #define TSC_SET_SEL             0x10            /* SEL signal */
    #define TSC_SET_ATN             0x08            /* ATN signal */
    #define TSC_SET_MSG             0x04            /* MSG signal */
    #define TSC_SET_CD              0x02            /* C/D signal */
    #define TSC_SET_IO              0x01            /* I/O signal */

    #define TSC_REQI                0x80            /* REQ signal */
    #define TSC_ACKI                0x40            /* ACK signal */
    #define TSC_BSYI                0x20            /* BSY signal */
    #define TSC_SELI                0x10            /* SEL signal */
    #define TSC_ATNI                0x08            /* ATN signal */
    #define TSC_MSGI                0x04            /* MSG signal */
    #define TSC_CDI                 0x02            /* C/D signal */
    #define TSC_IOI                 0x01            /* I/O signal */

#define TUL_SCmd                0x91    /* 11 R/W Command                                         */
    /* Command Codes of Tulip SCSI Command register */
    /*#define TSC_EN_RESEL_1         0x8D */                /* Enable Reselection */
    #define TSC_EN_RESEL     0x80           /* Enable Reselection              */
    #define TSC_CMD_COMP     0x84           /* Command Complete Sequence   */
    #define TSC_SEL                  0x01           /* Select Without ATN Sequence */
    #define TSC_SEL_ATN              0x11           /* Select With ATN Sequence        */
    #define TSC_SEL_ATN_DMA  0x51           /* Select With ATN Sequence with DMA  */
    #define TSC_SEL_ATN3     0x31           /* Select With ATN3 Sequence              */
    #define TSC_SEL_ATNSTOP  0x12           /* Select With ATN and Stop Sequence  */
    #define TSC_SELATNSTOP   0x1E           /* Select With ATN and Stop Sequence  */

    #define TSC_SEL_ATN_DIRECT_IN    0x95           /* Select With ATN Sequence       */
    #define TSC_SEL_ATN_DIRECT_OUT   0x15           /* Select With ATN Sequence       */
    #define TSC_SEL_ATN3_DIRECT_IN   0xB5           /* Select With ATN3 Sequence  */
    #define TSC_SEL_ATN3_DIRECT_OUT  0x35           /* Select With ATN3 Sequence  */
    #define TSC_XF_DMA_OUT_DIRECT    0x06           /* DMA Xfer Infomation out        */
    #define TSC_XF_DMA_IN_DIRECT     0x86           /* DMA Xfer Infomation in         */

    #define TSC_XF_DMA_OUT   0x43           /* DMA Xfer Infomation out                        */
    #define TSC_XF_DMA_IN    0xC3           /* DMA Xfer Infomation in                         */
    #define TSC_XF_FIFO_OUT  0x03           /* FIFO Xfer Infomation out                       */
    #define TSC_XF_FIFO_IN   0x83           /* FIFO Xfer Infomation in                        */
    #define TSC_MSG_ACCEPT   0x0F           /* Message Accept */
#define TUL_STest0              0x92    /* 12 R/W Test0                                           */
#define TUL_STest1              0x93    /* 12 R/W Test1                                           */
#define    TUL_SConf1        0x94    /* SCSI Configuration 1        */

/**********************************************************/
/*                                      DMA operating Register Set                */
/**********************************************************/
#define TUL_XAddH       0xC0    /*DMA Transfer Physical Address         */
#define TUL_XAddW       0xC8    /*DMA Current Transfer Physical Address */
#define TUL_XCntH       0xD0    /*DMA Transfer Counter                          */
#define TUL_XCntH2      0xD2    /*DMA Transfer Counter                          */
#define TUL_XCntW       0xD4    /*DMA Current Transfer Counter          */

#define TUL_XCmd        0xD8    /*DMA Command Register                          */
/* Command Codes of DMA Command register */
    #define TAX_X_FORC      0x02
    #define TAX_X_ABT       0x04
    #define TAX_X_CLR_FIFO  0x08
    #define TAX_X_IN        0x21
    #define TAX_X_OUT       0x01
    #define TAX_SG_IN       0xA1
    #define TAX_SG_OUT      0x81

#define TUL_Int         0xDC    /*Interrupt Register */
    /* Tulip DMA Interrupt Register   */
    #define XCMP                    0x01
    #define FCMP                    0x02
    #define XABT                    0x04
    #define XERR                    0x08
    #define SCMP                    0x10
    #define IPEND                   0x80

#define TUL_XStatus     0xDD    /*DMA status Register */
    /* Tulip DMA Status Register */
    #define XPEND                   0x01            /* Transfer pending */
    #define FEMPTY                  0x02            /* FIFO empty */

#define TUL_XFifoCnt    0xDE    /*DMA FIFO count     */
#define TUL_Mask        0xE0    /*Interrupt Mask Register */
#define TUL_XCtrl       0xE4    /*DMA Control Register    */
#define TUL_XFifo       0xE8    /*DMA FIFO                */
#define TUL_WCtrl       0xF7    /*Bus master wait state control */
#define TUL_DCtrl       0xFB    /*DMA delay control */



/*******************  SCSI Phase Codes*********************/
#define DATA_OUT        0x0
#define DATA_IN         0x1
#define CMD_OUT         0x2
#define STATUS          0x3
#define MSG_OUT         0x6
#define MSG_IN          0x7




/***********************************************************************
	Scatter-Gather Element Structure
************************************************************************/
typedef struct SG_Struc         {
    ULONG   Ptr;            /* Data Pointer */
    ULONG   Len;            /* Data Length */
} SG;

/* Scatter/Gather Segment Descriptor Definition */
typedef struct _SGD {
    ULONG   Length;
    ULONG   Address;
} SGD, *PSGD;
#define TUL_MAX_SG_LIST         49        // 65


/***********************************************************************
	SCSI Control Block
************************************************************************/

typedef struct tul_scb {   /* Scsi_Ctrl_Blk      */
    UBYTE   Status; /*0 */
    UBYTE   NxtStat;/*1 */
    UBYTE   Mode;   /*2 */
    UBYTE   SgLock; /*3 */
    UWORD   SGIdx;  /*4 */
    UWORD   SGMax;  /*6 */
    ULONG   XferLen;/*8 */
    ULONG   TotalXLen; /*c */
    ULONG   SGPAddr;        /*10 SG List/Sense Buf phy. Addr. */
    ULONG   SensePtr;   /*14 Sense Buf virtual addr */

    void    (*Post)();                      /*18 POST routine */

    SCSI_REQUEST_BLOCK      *Srb;   /*1C SRB Pointer */
    UBYTE   Opcode;                         /*20 TULSCB command code */
    UBYTE   Flags;                          /*21 TULSCB Flags */
    UBYTE   Target;                         /*22 Target Id */
    UBYTE   Lun;                            /*23 Lun */
    ULONG   BufPtr;                         /*24 Data Buffer Pointer */
    ULONG   BufLen;                         /*28 Data Allocation Length */
    UBYTE   SGLen;                          /*2C SG list # */
    UBYTE   SenseLen;                       /*2D Sense Allocation Length */
    UBYTE   HaStat;                         /*2E */
    UBYTE   TaStat;                         /*2F */

    UBYTE   CDBLen;                         /*30 CDB Length */
    UBYTE   Ident;                          /*31 Identify */
    UBYTE   TagMsg;                         /*32 Tag Message */
    UBYTE   TagId;                          /*33 Queue Tag */

    UBYTE   CDB[12];                        /*34 */

    UBYTE   RetryCount;
    UBYTE   IOCTLState;
    UWORD   XferSG;         /* number of SG been xferred */

    struct tul_scb *NextScb;        /* Pointer to next TULSCB */
    ULONG   NxtBufPtr;      /* Next Data Buffer Pointer */
    ULONG   NxtBufLen;      /* Next Data Allocation Length */


    ULONG   scbpAryIdx; /* scb array index of hcsp->scb[] and hcsp->scbpAry[] */
    USHORT  tobeBusFree;
    USHORT  setATN;                 
    SG      SGList[TUL_MAX_SG_LIST];        /*40 Start of SG list */
} TULSCB, *PTULSCB;

/* Bit Definition for TULSCB_Status */
#define TULSCB_RENT     0x01
#define TULSCB_PEND     0x02
#define TULSCB_CONTIG   0x04            /* Contigent Allegiance */
#define TULSCB_SELECT   0x08
#define TULSCB_BUSY     0x10
#define TULSCB_DONE     0x20



/* Opcodes of TULSCB_Opcode */
#define ExecSCSI        0x1
#define BusDevRst       0x2
#define AbortCmd        0x3

/* Bit Definition for TULSCB_Mode */
#define SCM_RSENS       0x01            /* request sense mode */


/* Bit Definition for TULSCB_Flags */
#define SCF_DONE        0x01
#define SCF_POST        0x02
#define SCF_SENSE       0x04
#define SCF_DIR         0x18
#define SCF_NO_DCHK     0x00
#define SCF_DIN         0x08
#define SCF_DOUT        0x10
#define SCF_NO_XF       0x18
#define SCF_POLL        0x40
#define SCF_SG          0x80

/* Error Codes for TULSCB_HaStat */
#define HOST_SEL_TOUT   0x11
#define HOST_DO_DU       0x12
#define HOST_BUS_FREE   0x13
#define HOST_BAD_PHAS   0x14
#define HOST_INV_CMD    0x16
#define HOST_SCSI_RST   0x1B
#define HOST_DEV_RST    0x1C
#define HOST_CANNOT_ALLOC_SCB 0x1D


/* Error Codes for TULSCB_TaStat */
#define TARGET_CHECK_CONDITION  0x02
#define TARGET_BUSY             0x08
#define TARGET_TAG_FULL         0x28


/* SCSI MESSAGE */
#define MSG_COMP        0x00
#define MSG_EXTEND      0x01
#define MSG_SDP         0x02
#define MSG_RESTORE     0x03
#define MSG_DISC        0x04
#define MSG_IDE         0x05
#define MSG_ABORT       0x06
#define MSG_REJ         0x07
#define MSG_NOP         0x08
#define MSG_PARITY      0x09
#define MSG_LINK_COMP   0x0A
#define MSG_LINK_FLAG   0x0B
#define MSG_DEVRST              0x0C
#define MSG_ABORT_TAG   0x0D

/* Queue tag msg: Simple_quque_tag, Head_of_queue_tag, Ordered_queue_tag */
#define MSG_STAG        0x20
#define MSG_HTAG        0x21
#define MSG_OTAG        0x22

#define MSG_IGNOREWIDE  0x23

#define MSG_IDENT   0x80

/***********************************************************************
	Target Device Control Structure
**********************************************************************/

typedef struct Tar_Ctrl_Struc {
    USHORT  Flags;              
    UBYTE   JS_PeriodOffset; /* bit6-4: sync xfer period, bit 3-0:sync offset */    
    UBYTE   Period;             
    UBYTE   xfrPeriodIdx;
    UBYTE   sConfig;
    UBYTE   rsvd;
    char    MaximumTags;                    /* 08/07/97     */
    char    RequestCount[8];            /* 07/31/97     */
} TCS;

/* Bit Definition for TCF_Flags */

#define TCF_SCSI_RATE                   0x007
#define TCF_DISCONNECT                  0x008
#define TCF_NO_SYNC_NEGO                0x010
#define TCF_NO_WDTR                     0x020
#define TCF_EN_255                      0x040
#define TCF_EN_START                    0x080

#define TCF_WDTR_DONE                   0x100
#define TCF_SYNC_DONE                   0x200
#define TCF_BUSY                        0x400
#define TCF_EN_TAG                      0x800

/***********************************************************************
	      Host Adapter Control Structure
************************************************************************/
#define MAX_TUL_SCB             64    /* Maximum TUL_SCB       */

typedef struct i910_ha {   /* Ha_Ctrl_Struc */
    ULONG_PTR   Base;                           /* 00 */
    UBYTE   Index;                                                  /* 04 */
    UBYTE   HaId;                           /* 05 */
    UBYTE   Intr;                           /* 06 */
    UBYTE   Config;                         /* 07 */
    UBYTE   wide_scsi_card;                 /* 08 */
    UWORD   IdMask;                         /* 09 */
    UBYTE   semaph;                         /* 0B */
    UBYTE   Flags;                          /* 0C */
    UBYTE   Phase;                          /* 0D */
    UBYTE   JSStatus0;                      /* 0E*/
    UBYTE   JSInt;                          /* 0F */
    UBYTE   JSStatus1;                      /* 0x10 */
    UBYTE   HCS_SConf1;                     /* 0x11 */
    void    (*SCSIFunc)();                  /* 0x12 */
    TULSCB  *Scb;                           /* 0x16 */
    TULSCB  *ScbEnd;                        /* 0x1A */
    LONG    NxtPend;                        /* 0x1E */
    LONG    NxtContig;                      /* 0x22 */
    TULSCB  *ActScb;                        /* 0x26 */
    TCS     *ActTcs;                        /* 0x2A */
    TCS     Tcs[16];                        /* 0x2E */
    TULSCB     *FirstPend;
    TULSCB     *LastPend;
    TULSCB     *FirstBusy;
    TULSCB     *LastBusy;
    TULSCB     *FirstDone;
    TULSCB     *LastDone;   
    UBYTE   Msg[8];                         
} HCS, *PHCS;

/* Bit Definition for HCB_Config */
#define         HCC_SCSI_RESET  0x01
#define         HCC_EN_PAR      0x02
#define         HCC_ACT_TERM1   0x04
#define         HCC_ACT_TERM2   0x08
#define         HCC_EN_PWR      0x80
#define         HCC_AUTO_TERM   0x10

/* Bit Definition for HCB_Flags */
#define HCF_EXPECT_DISC         0x01


/* Scatter/Gather Segment List Definitions */
/* Adapter limits */
#define MAX_SG_DESCRIPTORS TUL_MAX_SG_LIST
#define MAX_TRANSFER_SIZE  (MAX_SG_DESCRIPTORS - 1) * 4096
#define SEGMENT_LIST_SIZE   MAX_SG_DESCRIPTORS * sizeof(SGD)
#define NONCACHED_EXTENSION     256
/* INI910 specific port driver device object extension. */
typedef struct _HW_DEVICE_EXTENSION {
    HCS   hcs;
    PVOID NonCachedBuf;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;


#define inp( x )        ScsiPortReadPortUchar( (PUCHAR)x )
#define inpw( x )       ScsiPortReadPortUshort( (PUSHORT)x )
#define inpl( x )       ScsiPortReadPortUlong( (PULONG)x )
#define outp( x, y )    ScsiPortWritePortUchar( (PUCHAR)x,  (UCHAR)y )
#define outpw( x, y )   ScsiPortWritePortUshort( (PUSHORT)x, (USHORT)y )
#define outpl( x, y )   ScsiPortWritePortUlong( (PULONG)x,  (ULONG)y )
#define TUL_RD(x,y)  (UCHAR)(ScsiPortReadPortUchar(  (PUCHAR)((ULONG_PTR)((ULONG_PTR)x+(UCHAR)y)) ))
#define TUL_RDSHORT(x,y)  (USHORT)(ScsiPortReadPortUshort(  (PUSHORT)((ULONG_PTR)((ULONG_PTR)x+(UCHAR)y)) ))
#define TUL_RDLONG(x,y) (long)(ScsiPortReadPortUlong( (PULONG)((ULONG_PTR)((ULONG_PTR)x+(UCHAR)y)) ))
#define TUL_WR(     adr,data) ScsiPortWritePortUchar( (PUCHAR)(adr), (UCHAR)(data))
#define TUL_WRSHORT(adr,data) ScsiPortWritePortUshort((PUSHORT)(adr),(USHORT)(data))
#define TUL_WRLONG( adr,data) ScsiPortWritePortUlong( (PULONG)(adr), (ULONG)(data))



#endif  /* _I950_H_ */

