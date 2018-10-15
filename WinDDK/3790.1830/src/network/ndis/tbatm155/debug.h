/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       debug.h
 
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

#ifndef __DEBUG_H
#define __DEBUG_H

#define DBG_LEVEL_INFO         0x0000
#define DBG_LEVEL_LOG          0x0800
#define DBG_LEVEL_WARN         0x1000
#define DBG_LEVEL_ERR          0x2000
#define DBG_LEVEL_FATAL        0x3000

#define DBG_COMP_INIT          0x00000001
#define DBG_COMP_SEND          0x00000002
#define DBG_COMP_RECV          0x00000004
#define DBG_COMP_REQUEST       0x00000008
#define DBG_COMP_UNLOAD        0x00000010
#define DBG_COMP_LOCKS         0x00000020
#define DBG_COMP_VC            0x00000040
#define DBG_COMP_HALT          0x00000080
#define DBG_COMP_INT           0x00000100
#define DBG_COMP_RESET         0x00000200
#define DBG_COMP_LOCK          0x00000400
#define DBG_COMP_PEEPHOLE      0x00000800
#define DBG_COMP_SPECIAL       0x00001000
#define DBG_COMP_PLC2          0x00002000
#define DBG_COMP_NRXBUFFOUND   0x00004000
#define DBG_COMP_TXIOC         0x00008000
#define DBG_COMP_RXIOC         0x00010000
#define DBG_COMP_SEND_ERR      0x00020000
#define DBG_PENDING_PKTS       0x00040000
#define DBG_XMIT_PKTS          0x00080000
#define DBG_COMP_ALL           0xFFFFFFFF

extern	ULONG   gTbAtm155DebugSystems;
extern	LONG    gTbAtm155DebugLevel;

// Define a macro so DbgPrint can work on win9x, 32-bit/64-bit NT's
#ifdef _WIN64
#define PTR_FORMAT      "%p"
#else
#define PTR_FORMAT      "%x"
#endif
                            

#if DBG

VOID
dbgDumpHardwareInformation(
   IN  PHARDWARE_INFO  HwInfo
   );


VOID
dbgDumpPciCommonConfig(
   IN  PADAPTER_BLOCK      pAdapter
	);


VOID
dbgDumpAtm155SarRegisters(
   IN  PADAPTER_BLOCK      pAdapter
   );

VOID
dbgDumpAtm155SuniLiteRegisters(
   IN  PADAPTER_BLOCK      pAdapter
   );


VOID
dbgDumpAtm155EntryOfRxStat(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               Vc
   );

VOID
dbgDumpAtm155Sram(
   IN  PADAPTER_BLOCK      pAdapter
   );

#if TB_DBG

 VOID
 dbgDumpAtm155TableSram(
	IN  PADAPTER_BLOCK      pAdapter,
   IN  ULONG               StartDest,
   IN  ULONG               EndDest
	);

#endif // end of TB_DBG


extern	ULONG   gTbAtm155DebugInformationOffset;

#define DBGPRINT(Component, Level, Fmt)                            \
{                                                                  \
   if ((Level >= gTbAtm155DebugLevel) &&                           \
       ((gTbAtm155DebugSystems & Component) == Component))         \
   {                                                               \
       DbgPrint("***TbAtm155*** (%x, %d) ",                        \
               MODULE_NUMBER >> 16, __LINE__);                     \
       DbgPrint Fmt;                                               \
   }                                                               \
}


#define DBGBREAK(Component, Level)												\
{																				\
   if ((Level >= gTbAtm155DebugLevel) && (gTbAtm155DebugSystems & Component))	\
   {																			\
       DbgPrint("***TbAtm155*** DbgBreak @ %x, %d\n", MODULE_NUMBER, __LINE__);	\
       DbgBreakPoint();														\
   }																			\
}


#define IF_DBG(Component, Level)   if ((Level >= gTbAtm155DebugLevel) && (gTbAtm155DebugSystems & Component))

//
//	PACKET LOGGING DEFINITIONS.
//
#if _DBG_PACKET

typedef struct _PACKET_LOG_ENTRY
{
   PVOID   Context1;
   ULONG   Context2;
   ULONG   Context3;
   ULONG   Ident;
}
	PACKET_LOG_ENTRY,
	*PPACKET_LOG_ENTRY;

typedef struct _SPIN_LOCK_LOG_ENTRY
{
   ULONG       Ident;          //  Module & line number.
   ULONG       Function;       //  Acquire or release.
   ULONG       SpinLock;       //  Pointer to the spinlock.
   PVOID       Context;
}
	SPIN_LOCK_LOG_ENTRY,
	*PSPIN_LOCK_LOG_ENTRY;

#define LOG_SIZE       1024

typedef struct _PACKET_LOG
{
   UINT                CurrentEntry;
   PPACKET_LOG_ENTRY   Head;
   NDIS_SPIN_LOCK      Lock;
   PPACKET_LOG_ENTRY   Buffer;
}
   PACKET_LOG,
   *PPACKET_LOG;

typedef struct _SPIN_LOCK_LOG
{
   UINT                    CurrentEntry;
   PSPIN_LOCK_LOG_ENTRY    Head;
   NDIS_SPIN_LOCK          Lock;
   PSPIN_LOCK_LOG_ENTRY    Buffer;
}
   SPIN_LOCK_LOG,
   *PSPIN_LOCK_LOG;

typedef struct _TbAtm155_LOG_INFO
{
   PSPIN_LOCK_LOG      SpinLockLog;
   PPACKET_LOG         SendPacketLog;
   PPACKET_LOG         RecvPacketLog;
}
   TbAtm155_LOG_INFO,
   *PTbAtm155_LOG_INFO;

#define NUMBER_OF_LOGS     4

//
//  Macros for referencing the logs.
//
#define SL_CURRENT_ENTRY(_Log)     (_Log)->SpinLockLog->CurrentEntry
#define SL_HEAD(_Log)              (_Log)->SpinLockLog->Head
#define SL_LOCK(_Log)              (_Log)->SpinLockLog->Lock
#define SL_LOG(_Log)               (_Log)->SpinLockLog->Buffer

#define SPL_CURRENT_ENTRY(_Log)    (_Log)->SendPacketLog->CurrentEntry
#define SPL_HEAD(_Log)             (_Log)->SendPacketLog->Head
#define SPL_LOCK(_Log)             (_Log)->SendPacketLog->Lock
#define SPL_LOG(_Log)              (_Log)->SendPacketLog->Buffer

#define RPL_CURRENT_ENTRY(_Log)    (_Log)->RecvPacketLog->CurrentEntry
#define RPL_HEAD(_Log)             (_Log)->RecvPacketLog->Head
#define RPL_LOCK(_Log)             (_Log)->RecvPacketLog->Lock
#define RPL_LOG(_Log)              (_Log)->RecvPacketLog->Buffer

VOID
dbgLogRecvPacket(
   IN  PTbAtm155_LOG_INFO  pLog,
   IN  PVOID               Context1,
   IN  ULONG               Context2,
   IN  ULONG               Context3,
   IN  ULONG               Ident
   );

VOID
dbgLogSendPacket(
   IN  PTbAtm155_LOG_INFO  pLog,
   IN  PVOID               Context1,
   IN  ULONG               Context2,
   IN  ULONG               Context3,
   IN  ULONG               Ident
   );

VOID
dbgInitializeDebugInformation(
   OUT PTbAtm155_LOG_INFO  *pLog
   );

VOID
dbgFreeDebugInformation(
   IN  PTbAtm155_LOG_INFO   pLogInfo
   );

#else

#define dbgLogRecvPacket(_log, _Context1, _Context2, _Context3, _Ident)
#define dbgLogSendPacket(_log, _Context1, _Context2, _Context3, _Ident)
#define dbgInitializeDebugInformation(_pLogInfo)
#define	dbgFreeDebugInformation(_pLogInfo)

#endif // _DBG_PACKET

#else

#define dbgDumpHardwareInformation(HwInfo)
#define dbgDumpPciCommonConfig(_PciCommonConfig)

#define dbgDumpAtm155SarRegisters(pAdapter)
#define dbgDumpAtm155PhyRegisters(pAdapter)
#define dbgDumpAtm155EntryOfRxStat(pAdapter, Vc)
#define dbgDumpAtm155Sram(pAdapter)

#define dbgInitializeDebugInformation(_pLogInfo)
#define dbgFreeDebugInformation(_pLogInfo)

#define DBGPRINT(Component, Level, Fmt)

#define DBGBREAK(Component, Level)

#define IF_DBG(Component, Level)	if (FALSE)

#define dbgLogRecvPacket(_log, _Context1, _Context2, _Context3, _Ident)
#define dbgLogSendPacket(_log, _Context1, _Context2, _Context3, _Ident)

#endif

#if DBG_USING_LED

VOID
dbgSetLED(
	IN  PADAPTER_BLOCK      pAdapter,
	IN  UCHAR               SetLedValue,
	IN  UCHAR               MaskLedValue
	);

#endif // end of DBG_USING_LED

#if TB_DBG

VOID
dbgDumpActivateVcInfo(
	IN  PADAPTER_BLOCK      pAdapter
	);

#else

#define dbgDumpActivateVcInfo(pAdapter)

#endif // end of TB_DBG


#endif  //  __DEBUG_H

