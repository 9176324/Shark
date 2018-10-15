/************************************************************************
                    Copyright (c) 1994-1997 Initio Corp.
    Initio Windows NT device driver are copyrighted by Initio Corporation.
    Your rights of ownership are subject to the limitations and restrictions
    imposed by the copyright laws. Initio makes no warranty  of any kind in
    regard to this material, including,  but not  limited to,  the  implied
    warranties of merchantability and fitness  for  a  particular  purpose.
    Initio is not liable for any errors contained  herein  or incidental or
    consequential damages in connection with furnishing, performance or use
    of this material.
  -----------------------------------------------------------------------
  Module: ini910u.c
  Description:  INI-9100U/UW PCI_UltraSCSI Bus Master Controller
  Revision History:
  11/09/94 Tim Chen, Initial Version 0.90A
  03/11/97 TC Delete MAX_PCI_CHANL,Since _HW_DEVICE_EXTENSION
              is allocated by OS, so Windows NT and Windows 95
              can support unlimited PCI chanl and 9400 crad
              TulFindAdapter: same reason, delete HcsInfo[]
              release ver 2.05
  03/19/97 TC Disable DEC Bridge ISA mode , release ver 2.06
  03/26/97 TC add LinkList support
          ConfigInfo->InterruptMode =  LevelSensitive;
  04/08/97 TC comment out "Disable DEC Bridge ISA mode", release ver 2.07
  04/14/97 TC reset_scsi: wait 5 sec instead of 2 sec
  05/14/97 hc Check NOVRAM for initio's signature and reversion number
  06/16/97 hc Confirm Initio's vendor&product ID
  07/18/97 hc v2.09
          - Support i-940c (9400)
          - Support i-935c (9401)
          - Address 0xxFB always program 0x80
          - Change NextRequest to NextLuRequest in ScsiPortNotification
  07/31/97 hc v2.10
          - Return -1 from int_busfree and int_scsi_rst
  08/05/97 hc v2.10
          - change timeout value for scsi_reset to 10 seconds
          - change delay after post_scsi from 0.5sec to 1 sec
          - comment out TAG_QFULL in status_msg
  08/11/97 hc - Set TagMsg to 0 while we setup autorequestsense CDB
  10/21/97 hc v2.11
            - Support LUN up to 32
          - Correct SCANNER on Win98 problem
          - Change ConfigInfo->AlignmentMask = 0x3 to 0x0;
  10/27/97 hc - Solve Tape drive problem
  12/04/97 hc v2.12
          - Support DTCT 0002134A
  12/09/97 hc -    Set SCSI configuration 1(0x94) to 0x17
          - Set FIFO threshold(0x54) to 1/2
  01/23/98 hc - Add PNP for window NT 5.0
  02/26/98 hc - Don't clear DMA FIFO and ABORT at the same time
  03/19/98 hc - Add ScsiRestartAdapter in SCSI_ADAPTER_CONTROL (power management)
              - v2.15
**********************************************************************/

#include "i950.h"
#define TAG_NUMBER      16

static char    *version[] = {
    "Windows NT/95 device driver version 2.16"};


static char    *copyright[] = {
    "Initio Windows NT/95 device driver are copyrighted by Initio Corporation. \
    Your rights of ownership are subject to the limitations and restrictions \
    imposed by the copyright laws. Initio makes no warranty of any kind in \
    regard to this material, including, but not limited to, the implied \
    warranties of merchantability and fitness for a particular purpose.\
    Initio is not liable for any errors contained herein or incidental or \
    consequential damages in connection with furnishing, performance or use \
    of this material. " };

// int    hwInitialized = 0;


char    tulip_scsi();
char    next_state();
char    xfer_data_in();
char    xfer_data_out();
char    xfer_pad_in();
char    xfer_pad_out();
char    status_msg();
char    bad_seq();

char    msgin();
char    msgin_discon();
char    msg_accept();
char    msgout_nop();
char    msgout_reject();
char    msgin_extend();
char    msgin_ignore_wid_resid();
char    msgout();
char    msgout_ide();
char    msgout_abort();
char    msgout_abort_tag();
char    wait_tulip();

int    tul_isr();
int    msgin_par_err();
int    msgin_sync();
int    msgout_sync();
char   int_busfree();
char   int_scsi_rst();
int    int_bad_seq();
int    int_resel();
int    wdtr_done();
int    post_scsi_rst();
int    wait_bus_free();
int    tulip_main();

void sync_done();
void init_scsi();
void select_atn();
void select_atn3();
void select_atn_stop();
void reset_scsi();
void init_reset_scsi();
int    init_tulip();
void AbortXPend();
void read_eeprom();
void se2_wait();
void TulPanic();
void tul_dis_sint();
void tul_en_sint();

int    tul_abort_srb();

void    ll_append_pend_scb();
void    ll_push_pend_scb();
TULSCB *ll_pop_pend_scb();
void    ll_unlink_pend_scb();

void    ll_append_busy_scb();
TULSCB *ll_pop_busy_scb();
void    ll_unlink_busy_scb();
TULSCB *ll_find_busy_scb();
TULSCB *ll_find_tagid_busy_scb();
void    ll_append_done_scb();
TULSCB *ll_pop_done_scb();


ULONG
DriverEntry(
IN PVOID DriverObject,
IN PVOID Argument2
);

ULONG
TulFindAdapter(
IN PVOID HwDeviceExtension,
IN PVOID Context,
IN PVOID BusInformation,
IN PCHAR ArgumentString,
IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
OUT PBOOLEAN Again
);


BOOLEAN
TulHwInitialize(
IN PVOID HwDeviceExtension
);

BOOLEAN
TulPutQueue(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK Srb
);

BOOLEAN
TulInterrupt(
IN PVOID HwDeviceExtension
);

BOOLEAN
TulResetBus(
IN PVOID HwDeviceExtension,
IN ULONG PathId
);

#ifndef WIN95
SCSI_ADAPTER_CONTROL_STATUS
TulAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
);
#endif

/* This function is called from TulPutQueue. */
VOID
BuildScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK Srb
);

VOID
BuildRstScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK Srb
);

VOID
BuildAbtScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK Srb
);

/* Logical unit extension */
typedef struct _TUL_LU_EXTENSION {
    PSCSI_REQUEST_BLOCK CurrentSrb;
} TUL_LU_EXTENSION, *PTUL_LU_EXTENSION;


/* function called during interrupt process time */
ScbPost(
IN HCS *hcsp,
IN TULSCB *scbp
);

VOID
GetPhyiAddr(
IN              PHCS    hcsp,
IN              uchar   *bufAddr,
IN              ulong   xferLength,
IN OUT  uchar   *phyAddr
);

UCHAR
TulErrXlate (
UCHAR scbHaStat
);


void se2_wait(HCS *hcsp)
{
    ScsiPortStallExecution(30); /* wait 30 us */
}


/******************************************************************
 Input: instruction for  Serial E2PROM
******************************************************************/
se2_instr(hcsp, instr )
HCS *hcsp;
UCHAR instr;
{
    int    i;
    UCHAR b;

    TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2DO);     /* cs+start bit */
    se2_wait(hcsp);
    TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2CLK | SE2DO); /* +CLK */
    se2_wait(hcsp);

    for ( i = 0; i < 8; i++) {

        if (instr & 0x80)
            b =  SE2CS | SE2DO;                           /* -CLK+dataBit */
        else
            b =  SE2CS ;                                  /* -CLK */

        TUL_WR( hcsp->Base + TUL_NVRAM, b);
        se2_wait(hcsp);

        TUL_WR( hcsp->Base + TUL_NVRAM, b | SE2CLK);        /* +CLK */
        se2_wait(hcsp);

        instr <<= 1;
    }

    TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS );                /* -CLK */
    se2_wait(hcsp);
    return 1;
}


/******************************************************************
 Function name  : se2_ew_en
 Description    : Enable erase/write state of serial EEPROM
******************************************************************/
se2_ew_en(hcsp)
HCS *hcsp;
{
    se2_instr(hcsp, 0x30 ); /* EWEN */
    TUL_WR( hcsp->Base + TUL_NVRAM, 0 );    /* -CS  */
    se2_wait(hcsp);
    return 1;
}


/******************************************************************
 Disable erase/write state of serial EEPROM
*************************************************************************/
se2_ew_ds(hcsp)
HCS *hcsp;
{
    se2_instr(hcsp, 0 );    /* EWDS */
    TUL_WR( hcsp->Base + TUL_NVRAM, 0 );    /* -CS  */
    se2_wait(hcsp);
    return 1;
}


/******************************************************************
    Input  :address of Serial E2PROM
    Output :value stored in  Serial E2PROM
*******************************************************************/
USHORT se2_rd(hcsp, adr)
HCS *hcsp;
ULONG adr;
{
    UCHAR instr, readByte;
    USHORT readWord;
    int    i;

    instr = (UCHAR)(  adr  | 0x80 );
    se2_instr( hcsp, instr );   /* READ INSTR */
    readWord = 0;

    for ( i = 15; i >= 0; i--) {
        TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2CLK );
        se2_wait(hcsp);
        TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS );

        readByte = TUL_RD( hcsp->Base, TUL_NVRAM );
        readByte &= SE2DI;
        readWord += (readByte << i);
        se2_wait(hcsp);
    }

    TUL_WR( hcsp->Base + TUL_NVRAM, 0 );    /* no chip select */
    se2_wait(hcsp);
    return readWord;
}


/******************************************************************
 Input: new value in  Serial E2PROM, address of Serial E2PROM
*******************************************************************/
se2_wr(hcsp, adr, writeWord)
HCS *hcsp;
UCHAR adr;
USHORT writeWord;
{
    UCHAR readByte;
    UCHAR instr;
    int    i;

    instr = (UCHAR)(  adr  | 0x40 );
    se2_instr( hcsp, instr );   /* WRITE INSTR */

    for ( i = 15; i >= 0; i--) {
        if (writeWord & 0x8000)
            TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2DO);
        else
            TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS );
        se2_wait(hcsp);
        TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2CLK );
        se2_wait(hcsp);
        writeWord <<= 1;
    }
    TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS );
    se2_wait(hcsp);

    TUL_WR( hcsp->Base + TUL_NVRAM, 0);
    se2_wait(hcsp);

    TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS);
    se2_wait(hcsp);

    for (; ; ) {
        TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS | SE2CLK);
        se2_wait(hcsp);
        TUL_WR( hcsp->Base + TUL_NVRAM, SE2CS );
        se2_wait(hcsp);
        if ( (readByte = TUL_RD( hcsp->Base, TUL_NVRAM )) & SE2DI ) {
            break;
        }
    }
    TUL_WR( hcsp->Base + TUL_NVRAM, 0);
    return 1;
}


/***********************************************************************
 Read SCSI H/A configuration parameters from serial EEPROM
************************************************************************/
se2_rd_all(hcsp, nvramp)
HCS *hcsp;
NVRAM *nvramp;
{
    int    i;
    ULONG chksum = 0;
    USHORT * np;


    np = (USHORT * )nvramp;
    for ( i = 0; i < 32; i++) {
        *np++ = se2_rd(hcsp, i );
    }

    /*---------------------- Is ckecksum ok ? ----------------------*/
    np = (USHORT * )nvramp;
    for ( i = 0; i < 31; i++)
        chksum += *np++;
    if ( nvramp->CheckSum != (USHORT)chksum ) {
        return - 1;
    }
    return 1;
}


//
// use this type to guarentee the alignment so the later cast from 
// a uchar buffer to a ushort buffer will always be ok
//

__declspec(align(2)) typedef UCHAR uchar_align2;

/***********************************************************************
 Update SCSI H/A configuration parameters from serial EEPROM
************************************************************************/
se2_update_all(hcsp, nvramp )   /* setup default pattern */
HCS *hcsp;
NVRAM *nvramp;
{
    uchar_align2  dftNvRam[64] = {
    /*----------header ---------------*/
    0x25,                           /* 0x0: Signature Byte 1  */
    0xc9,                           /* 0x1: Signature Byte 2  */
    0x40,                           /* 0x2: Size of Data Structure  */
    0x1,                            /* 0x3: Revision of Data Structure */
    /* ----Host Adapter Structure ----*/
    0x95,                           /* 0x4: ModelByte0 */
    0x00,                           /* 0x5: ModelByte1 */
    0x00,                           /* 0x6: ModelInfo  */
    0x01,                           /* 0x7: Num Of SCSI Channel    */
    0x03,                           /* 0x8: BIOSConfig1*/
    0,                              /* 0x9: BIOSConfig2*/
    0,                              /* 0xA: BIOSConfig3*/
    0,                              /* 0xB: BIOSConfig4*/
    /* ----SCSI channel 0 and target Structure ---- */
    7,                              /* 0xC: SCSIid     */
    0x13,                           /* 0xD: SCSIconfig1*/
    0,                              /* 0xE: SCSIconfig2*/
    0x8,                            /* 0xF: NumSCSItarget */
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
    0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68, 0x68,
        /* ---------- CheckSum ---------- */
    0, 0 };                                    /* ox3E, 0x3F checksum */

    int    i;
    ULONG chksum = 0;
    USHORT * np, *np1;


    /* Calculate checksum first */
    np = (USHORT * )dftNvRam;
    for ( i = 0; i < 31; i++)
        chksum += *np++;
    *np = (USHORT)chksum;
    se2_ew_en(hcsp);                /* Enable write  */

    np = (USHORT * )dftNvRam;
    np1 = (USHORT * )nvramp;
    for ( i = 0; i < 32; i++, np++, np1++) {
        if (*np != *np1 ) {
            se2_wr( hcsp, i, *np );
        }
    }

    se2_ew_ds(hcsp);        /* Disable write   */
    return 1;
}


/*************************************************************************
 Function name  : read_eeprom
**************************************************************************/
void read_eeprom( hcsp, nvramp )
HCS *hcsp;
NVRAM *nvramp;
{
    UCHAR gctrl;

    /*------Enable EEProm programming ---*/
    gctrl = TUL_RD( hcsp->Base, TUL_GCTRL );
    TUL_WR( hcsp->Base + TUL_GCTRL, gctrl | EEPRG );
    if ( se2_rd_all(hcsp, nvramp) != 1 )  {
        se2_update_all( hcsp, nvramp);  /* setup default pattern */
        se2_rd_all(hcsp, nvramp);               /* load again  */
    }
    /*------ Disable EEProm programming ---*/
    gctrl = TUL_RD( hcsp->Base, TUL_GCTRL );
    TUL_WR( hcsp->Base + TUL_GCTRL, gctrl & ~EEPRG );
} /* read_eeprom */


/***************************************************************************/
int    init_tulip(hcsp)
register HCS *hcsp;
{
    NVRAM nvram, *nvramp = &nvram;

    UCHAR     xfrPeriod[8] = {   /* fast 20 */
    /* nanosecond devide by 4 */
        12,              /* 50ns,  20M       */
        18,              /* 75ns,  13.3M */
        25,              /* 100ns, 10M   */
        31,              /* 125ns, 8M    */
        37,              /* 150ns, 6.6M  */
        43,              /* 175ns, 5.7M  */
        50,              /* 200ns, 5M    */
        62               /* 250ns, 4M    */
        };

    register TCS    *tcsp;
    UBYTE    readByte, *readBytep;
    ULONG    i, j;
    ULONG_PTR    TulSCmd;

    TulSCmd =  hcsp->Base + TUL_SCmd;

    hcsp->SCSIFunc = tulip_scsi;
    hcsp->semaph = 1;
    hcsp->JSStatus0 = 0;
    hcsp->JSStatus1 = 0;
    hcsp->Scb = 0;
    hcsp->ScbEnd = 0;
    hcsp->NxtPend = 0;
    hcsp->NxtContig = -1;
    hcsp->ActScb = NULL;
    hcsp->ActTcs = NULL;
    hcsp->FirstPend = NULL;
    hcsp->LastPend = NULL;
    hcsp->FirstBusy = NULL;
    hcsp->LastBusy = NULL;
    hcsp->FirstDone = NULL;
    hcsp->LastDone = NULL;

    TUL_WR(hcsp->Base + TUL_Mask, 0x0f);

    /*------------- get serial EEProm settting -------*/

    read_eeprom( hcsp, nvramp );
    if (nvramp->Signature != INI_SIGNATURE)
        return (-1);
    if ((nvramp->ModelByte0 != 0x95) && (nvramp->ModelByte0 != 0x94))
        return (-1);
    if (nvramp->Revision != 1)
        return (-1);
    /*----------- get H/A configuration --------------*/
    hcsp->HaId = nvramp->SCSIid;

    /* program HBA's SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId, hcsp->HaId << 4);

    hcsp->Config = nvramp->SCSIconfig1;

    /* mask all the interrupt */
    TUL_WR(hcsp->Base + TUL_Mask, 0x1F);

    /*--- Initialize the tulip ---*/
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_CHIP);

    /* program HBA's SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId, hcsp->HaId << 4);

    if (hcsp->Config & HCC_EN_PAR)
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT | TSC_EN_SCSI_PAR);
    else
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT );

    /* Enable HW reselect */
    TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT );

    TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);

    /* selection time out = 250 ms */
    TUL_WR(hcsp->Base + TUL_STimeOut, 153);


    /*--------- Enable SCSI terminator -----*/
    readByte = TUL_RD(hcsp->Base, TUL_XCtrl);
    readByte |= (hcsp->Config & (HCC_ACT_TERM1 | HCC_ACT_TERM2) );
    TUL_WR(hcsp->Base + TUL_XCtrl, readByte & 0xFE);

    /*----------- set auto termination ------*/
    readByte = TUL_RD(hcsp->Base, TUL_GCTRLH) & 0xFE;
    readByte |= (hcsp->Config & HCC_AUTO_TERM) >> 4;
    TUL_WR(hcsp->Base + TUL_GCTRLH, readByte );

    if (nvramp->NumSCSItarget != 8 ) {
        hcsp->wide_scsi_card = 1;
    } else
     {
        hcsp->wide_scsi_card = 0;
    }

    readBytep = (UCHAR * ) & (nvramp->Target0Config);
    tcsp = &hcsp->Tcs[0];
    for (i = 0; i < 16; tcsp++, readBytep++, i++) {
        tcsp->Flags = (USHORT) * readBytep;
        tcsp->xfrPeriodIdx = (tcsp->Flags & TCF_SCSI_RATE );
        tcsp->Flags = (*readBytep) & ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
        tcsp->Flags |= TCF_EN_TAG;
        if (hcsp->wide_scsi_card == 0)
            tcsp->Flags |= TCF_NO_WDTR;
        TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);
        if (hcsp->Config & HCC_EN_PAR)
            tcsp->sConfig = TSC_INITDEFAULT_ALT0 | TSC_EN_SCSI_PAR;
        else
            tcsp->sConfig = TSC_INITDEFAULT;

        tcsp->MaximumTags = TAG_NUMBER;

    } /* for */

    readByte = TUL_RD(hcsp->Base, TUL_DCtrl);
    TUL_WR(hcsp->Base + TUL_DCtrl, readByte | 0x80);

    readByte = TUL_RD(hcsp->Base, TUL_GCTRLH);        /* 12/09/97    */
    TUL_WR(hcsp->Base + TUL_GCTRLH, readByte | 0x02);    /* 12/09/97    */
    TUL_WR(hcsp->Base + TUL_SConf1, 0x17);        /* 12/09/97    */
    return (0);
}


/***************************************************************************/
void AbortXPend(hcsp)
register HCS    *hcsp;
{
    UCHAR readByte;
    if (TUL_RD(hcsp->Base, TUL_XStatus) & XPEND)     {
        readByte = TUL_RD(hcsp->Base, TUL_XCmd);
        TUL_WR(hcsp->Base + TUL_XCmd, readByte | TAX_X_ABT | TAX_X_CLR_FIFO);
        while (!( TUL_RD(hcsp->Base, TUL_Int) & XABT )) {
        }
    }
}


/***************************************************************************/
void reset_tulip(hcsp)
register HCS    *hcsp;
{
    register TCS    *tcsp;
    UBYTE    readByte;
    int    i;
    /* mask all the interrupt */
    TUL_WR(hcsp->Base + TUL_Mask, 0x1F);

    TUL_WR( hcsp->Base + TUL_SIntEnable, 0xFF);
    /* reset tulip chip */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_CHIP);

    /* program HBA's SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId, hcsp->HaId << 4);

    if (hcsp->Config & HCC_EN_PAR)
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT | TSC_EN_SCSI_PAR);
    else
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT );

    TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);

    /* selection time out = 250 ms */
    TUL_WR(hcsp->Base + TUL_STimeOut, 153);

    tcsp = &hcsp->Tcs[0];
    for (i = 0; i < 16; tcsp++, i++)   {
        tcsp->Flags  &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
        TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);
    } /* for */


    /* Enable HW reselect */
    TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT );

    /*--------- Enable SCSI terminator -----*/
    readByte = TUL_RD(hcsp->Base, TUL_XCtrl);
    readByte |= (hcsp->Config & (HCC_ACT_TERM1 | HCC_ACT_TERM2) );
    TUL_WR(hcsp->Base + TUL_XCtrl, readByte & 0xFE);


}


/***************************************************************************/
int    tul_abort_srb(hcsp, srbp)
HCS *hcsp;
PSCSI_REQUEST_BLOCK srbp;
{
    register TULSCB    *scbp;
    ULONG           i;
    uint            find = 0;

    TULSCB         * pTmpScb;

    pTmpScb = hcsp->FirstPend;

    while (pTmpScb != NULL) {
        if (srbp == pTmpScb->Srb) {
            if (pTmpScb->Status != 0) {
                pTmpScb->Status = 0;
                ll_unlink_pend_scb(hcsp, pTmpScb);
                return (1);
            }
        }
        pTmpScb = (PTULSCB)(pTmpScb->NextScb);
    }

    pTmpScb = hcsp->FirstBusy;
    while (pTmpScb != NULL) {
        if (srbp == pTmpScb->Srb) {
            pTmpScb->Status = 0;
            ll_unlink_busy_scb(hcsp, pTmpScb);
            return (11);
        }
        pTmpScb = (PTULSCB)(pTmpScb->NextScb);
    }
    return (0);
}


/***************************************************************************/
int    tul_exec_scb(hcsp, scbp)
HCS      *hcsp;
TULSCB   *scbp;
{
    scbp->Status = TULSCB_PEND;
    scbp->Mode = 0;

    scbp->SGIdx = 0;
    scbp->SGMax = scbp->SGLen;

    if (scbp->Opcode == AbortCmd )
        ll_push_pend_scb(hcsp, scbp);
    else
        ll_append_pend_scb(hcsp, scbp);

    if (hcsp->semaph == 1) {
        tul_dis_sint(hcsp);     /* disable Tulmin SCSI Int */
        hcsp->semaph = 0;

        tulip_main(hcsp);

        hcsp->semaph = 1;
        tul_en_sint(hcsp);      /* Enable tulip SCSI Int */
    }
    return 1;
}


/***************************************************************************/
int    tul_isr(hcsp)
HCS    *hcsp;
{
    UBYTE  stat, rtn;

    /* enter critical section */
    stat = TUL_RD(hcsp->Base, TUL_SStatus0 );
    if (stat & TSS_INT_PENDING) {

        if (hcsp->semaph == 1) {
            tul_dis_sint(hcsp);     /* disable Tulmin SCSI Int */
            hcsp->semaph = 0;

            hcsp->JSStatus0 = stat;

            tulip_main(hcsp);

            hcsp->semaph = 1;
            tul_en_sint(hcsp);      /* Enable tulip SCSI Int */

            return 1;
        }
        return 0;
    }
    return 0;
}


/***************************************************************************/
int    tulip_main(hcsp)
register HCS    *hcsp;
{
    register TULSCB    *scbp;
    register TCS    *tcsp;
    int    rtn;

    for (; ; ) {
        (*(hcsp->SCSIFunc))(hcsp);
        while (scbp = ll_pop_done_scb(hcsp)) {
            tcsp = &hcsp->Tcs[scbp->Target];
            if (!(scbp->Mode & SCM_RSENS)) {
                if (scbp->TaStat == 2) {
                    if (scbp->HaStat == HOST_DO_DU)
                        scbp->HaStat = 0;
                    tcsp->Flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
                    if (scbp->Flags & SCF_SENSE) {
                        UBYTE   len;

                        len = scbp->SenseLen;
                        if (len == 0)
                            len = 1;
                        scbp->BufLen = scbp->SenseLen;
                        scbp->BufPtr = scbp->SensePtr;
                        scbp->TagMsg = 0;
                        scbp->Ident &= ~DISC_ALLOW;
                        scbp->Flags &= ~(SCF_SG | SCF_DIR);

                        scbp->Mode = SCM_RSENS;
                        scbp->TaStat = 0;
                        scbp->CDBLen = 6;
                        scbp->CDB[0] = SCSICMD_RequestSense;
                        scbp->CDB[1] = 0;
                        scbp->CDB[2] = 0;
                        scbp->CDB[3] = 0;
                        scbp->CDB[4] = len;
                        scbp->CDB[5] = 0;
                        scbp->Status = TULSCB_PEND | TULSCB_CONTIG;
                        ll_push_pend_scb(hcsp, scbp);
                        break;
                    }
                }
            } else {   /* in request sense mode */
                if (scbp->TaStat == 2) {

                    /* check contition status again after sending
                    requset sense cmd 0x3*/
                    scbp->HaStat = HOST_BAD_PHAS;
                }
                scbp->TaStat = 2;
            }

            scbp->Flags |= SCF_DONE;
            if (scbp->Flags & SCF_POST) {
                (*scbp->Post)(hcsp, scbp);
            }
        } /* while */

        /* find_active: */


        hcsp->JSStatus0 = TUL_RD(hcsp->Base, TUL_SStatus0);

        if ((hcsp->JSStatus0 & TSS_INT_PENDING)) {
            continue;
        }
        if (hcsp->ActScb) {
            /* return to OS and wait for xfer_done_ISR/Selected_ISR */
            return 1; /* return to OS, enable interrupt */
        } else {
            if ((scbp = hcsp->FirstPend ) == NULL) {
                return 1; /* return to OS, enable interrupt */
            }
        }
    } /* for */
}


/***************************** mod_ALTPD ****************************/
char    mod_ALTPD(hcsp, tcsp)
register HCS    *hcsp;
register TCS    *tcsp;
{
    TUL_WR(hcsp->Base + TUL_SConfig, tcsp->sConfig);
    return 1;
}


/**************** tulip_scsi rtn *************************************/
char    tulip_scsi(hcsp)
register HCS    *hcsp;
{
    register TULSCB    *scbp;
    register TCS    *tcsp;
    UBYTE readByte;
    int    rtn;
    hcsp->JSStatus0 = TUL_RD(hcsp->Base, TUL_SStatus0);
    if (hcsp->JSStatus0 & TSS_INT_PENDING) {
        if (hcsp->JSInt = (TUL_RD(hcsp->Base, TUL_SInt)) ) {
            hcsp->Phase = hcsp->JSStatus0 & TSS_PH_MASK;
            hcsp->JSStatus1 = TUL_RD(hcsp->Base, TUL_SStatus1);
            if (hcsp->JSInt & TSS_SCSIRST_INT) {
                /* if SCSI bus reset detected */
                int_scsi_rst(hcsp);
                return 0;
            }
            /* Happens when msg_accept SCSI cmd 0xF find out bus free */
            if (hcsp->JSInt & TSS_DISC_INT) {
                return 0;
            }
            if (hcsp->JSInt & TSS_RESEL_INT) {
                /* if selected/reselected interrupt */
                int_resel(hcsp);
                return 0;
            }
            if (hcsp->JSInt & TSS_SEL_TIMEOUT) {
                /* if selected/reselected timeout interrupt */
                int_busfree(hcsp);
                return 0;
            }
            if (hcsp->JSInt & (TSS_FUNC_COMP | TSS_BUS_SERV)) {

                if ( (scbp = hcsp->ActScb)  != 0) {
                    next_state(hcsp, scbp);
                }
                return 1;
            }
            /* if selected  or scam selected*/
            TUL_WR( hcsp->Base + TUL_SSignal, 0);
        }
    }


    if (hcsp->ActScb != 0) {
        return 1;
    }

    if ( (scbp = hcsp->FirstPend) == NULL) {
        return 1;
    }

    tcsp = &hcsp->Tcs[scbp->Target];
    /* program HBA's SCSI ID & target SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId,
        (hcsp->HaId << 4) | (scbp->Target & 0x0F) );

    if (scbp->Opcode == ExecSCSI) {


        mod_ALTPD(hcsp, tcsp);
        TUL_WR(hcsp->Base + TUL_SPeriodOffset, tcsp->JS_PeriodOffset);

        if ( (tcsp->Flags & (TCF_WDTR_DONE | TCF_NO_WDTR)) == 0) {
            /* do wdtr negotiation */
            select_atn_stop(hcsp, scbp);
        } else if ( (tcsp->Flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0) {
            /* do sync negotiation */
            select_atn_stop(hcsp, scbp);
        } else {
            if (scbp->TagMsg) {
                select_atn3(hcsp, scbp);
            } else {
                select_atn(hcsp, scbp);
            }
        }

        if (scbp->Flags & SCF_POLL) {
            while (wait_tulip(hcsp) != -1) {
                if ( next_state(hcsp, scbp) == -1)
                    break;
            }
        }
    } else if (scbp->Opcode == BusDevRst) {
        select_atn_stop(hcsp, scbp);
        scbp->NxtStat = 8;
        if (scbp->Flags & SCF_POLL) {
            while (wait_tulip(hcsp) != -1) {
                if (next_state(hcsp, scbp) == -1)
                    break;
            }
        }
    } else if (scbp->Opcode == AbortCmd ) {
        select_atn_stop(hcsp, scbp);
        if (scbp->Flags & SCF_POLL) {
            while (wait_tulip(hcsp) != -1) {
                if (next_state(hcsp, scbp) == -1)
                    break;
            }
        }
    } else {
        ll_pop_pend_scb(hcsp);
        scbp->Status = TULSCB_DONE;        /* done */
        scbp->HaStat = 0x16;    /* bad command */
        ll_append_done_scb(hcsp, scbp);
    }
    return 0;
}



/***************** next_state rtn *****************************/
char    next_state(hcsp, scbp)
register HCS    *hcsp;
register TULSCB *scbp;
{
UCHAR     xfrPeriod[8] = {   /* fast 20 */
    /* nanosecond devide by 4 */
    12,              /* 50ns,  20M       */
    18,              /* 75ns,  13.3M */
    25,              /* 100ns, 10M   */
    31,              /* 125ns, 8M    */
    37,              /* 150ns, 6.6M  */
    43,              /* 175ns, 5.7M  */
    50,              /* 200ns, 5M    */
    62               /* 250ns, 4M    */
};

    TCS     * tcsp;
    UBYTE   tar;
    ULONG_PTR   TulSCmd, TulSFifo;
    long    h, cnt, xcnt, fifoCnt;
    char    phase;
    UCHAR   sta, ctrl;
    UCHAR readByte;
    TULSCB  * pTmpScb;

    tcsp = hcsp->ActTcs;
    TulSCmd =  hcsp->Base + TUL_SCmd;
    TulSFifo = hcsp->Base + TUL_SFifo;

    switch (scbp->NxtStat) {
    case 1:
        if (scbp != ll_pop_pend_scb(hcsp)) {
            TulPanic("case1: scbp != ll_pop_pend_scb(hcsp)\n");
        }
        scbp->Status = TULSCB_BUSY;
        ll_append_busy_scb(hcsp, scbp);
        if (hcsp->Phase == MSG_OUT) {
            if (scbp->Opcode == AbortCmd) {
                if (scbp->TagMsg == 0) {
                    msgout_abort(hcsp);
                } else
                 {
                    msgout_abort_tag(hcsp);
                }
                return 3;
            } else
             {
                /* enable BUS */
                TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_EN_BUS_IN);

                /* send out identify message */
                TUL_WR(hcsp->Base + TUL_SFifo, scbp->Ident);
                if (scbp->TagMsg != 0) {
                    TUL_WR(TulSFifo, scbp->TagMsg);
                    TUL_WR(TulSFifo, scbp->TagId);
                }

                if ((tcsp->Flags & (TCF_WDTR_DONE | TCF_NO_WDTR)) == 0) {
                    tcsp->Flags |= TCF_WDTR_DONE;
                    TUL_WR(TulSFifo, MSG_EXTEND);
                    TUL_WR(TulSFifo, 2); /* extended msg length */
                    TUL_WR(TulSFifo, 3);     /* sync request */
                    TUL_WR(TulSFifo, 1);     /* start from 16bit xfer */
                } else
                 {
                    if ((tcsp->Flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0 ) {
                        tcsp->Flags |= TCF_SYNC_DONE;
                        TUL_WR(TulSFifo, MSG_EXTEND);
                        TUL_WR(TulSFifo, 3); /* extended msg length */
                        TUL_WR(TulSFifo, 1); /* sync request */

                        TUL_WR(TulSFifo, xfrPeriod[tcsp->xfrPeriodIdx]);
                        TUL_WR(TulSFifo, MAX_OFFSET); /* REQ/ACK offset */
                    }
                }
                TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
                if (wait_tulip(hcsp) == -1)
                    return - 1;
                TUL_WR(hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);
                readByte = TUL_RD(hcsp->Base, TUL_SSignal);
                TUL_WR( hcsp->Base + TUL_SSignal, readByte & 0x47);
            
                goto state_3;
            }
        } else
         {
            /* not MSG_OUT phase, ATN on */
            TUL_WR( hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);
            readByte = TUL_RD(hcsp->Base, TUL_SSignal);
            TUL_WR( hcsp->Base + TUL_SSignal, readByte & 0x47);

            goto state_3;
        }
        break;
    case 2:
        if (scbp != ll_pop_pend_scb(hcsp)) {
            TulPanic("case2: scbp != ll_pop_pend_scb(hcsp)\n");
        }
        scbp->Status = TULSCB_BUSY;

        ll_append_busy_scb(hcsp, scbp);

        hcsp->JSStatus1 = TUL_RD( hcsp->Base, TUL_SStatus1);
        if (hcsp->JSStatus1 & TSS_CMD_PH_CMP) {
            goto state_4;
        }
        TUL_WR(hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);
        readByte = TUL_RD(hcsp->Base, TUL_SSignal);
        TUL_WR( hcsp->Base + TUL_SSignal, readByte & 0x47);

        goto state_3;

    case 3:
state_3:
        phase = hcsp->Phase;
        for (; ; ) {
            switch (phase) {
            case CMD_OUT:       /* Command phase                */
                for ( h = 0; h < (int)scbp->CDBLen; h++) {
                    TUL_WR( TulSFifo, scbp->CDB[h]);
                }
                TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
                if ((phase = wait_tulip(hcsp)) == -1)
                    return phase;
                if (phase == CMD_OUT) {
                    return  bad_seq(hcsp);
                }
                goto state_4;
            case MSG_IN:        /* Message in phase             */
                scbp->NxtStat = 0x3;
                
                if ((phase =  msgin(hcsp, scbp)) == -1)
                    return - 1;
                break;
            case STATUS:        /* Status phase                 */
                if ((phase =  status_msg(hcsp, scbp)) == -1) {
                    return - 1;
                }
                break;
            case MSG_OUT:       /* Message out phase            */
                if ((tcsp->Flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0 ) {
                    tcsp->Flags |= TCF_SYNC_DONE;
                    TUL_WR(TulSFifo, MSG_EXTEND);
                    TUL_WR(TulSFifo, 3); /* extended msg length */
                    TUL_WR(TulSFifo, 1); /* sync request */

                    TUL_WR(TulSFifo, xfrPeriod[tcsp->xfrPeriodIdx]);
                    TUL_WR(TulSFifo, MAX_OFFSET); /* REQ/ACK offset */

                    TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
                    if ((phase = wait_tulip(hcsp)) == -1)
                        return - 1;
                    TUL_WR(hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);
                    readByte = TUL_RD(hcsp->Base, TUL_SSignal);
                    TUL_WR( hcsp->Base + TUL_SSignal, readByte & 0x47);
                } else {
                    if ((phase =  msgout_nop(hcsp)) == -1)
                        return - 1;
                }
                break;
            default:
                return  bad_seq(hcsp);
            }
        } /* for */

    case 4:
state_4:
        if ((scbp->Flags & SCF_DIR) == SCF_NO_XF) {
            goto state_6;
        }

        phase = hcsp->Phase;
        for (; ; ) {
            if (scbp->BufLen == 0) {
                goto state_6;
            }
            switch (phase) {
            case DATA_IN:       /* Data in phase                */
                return  xfer_data_in(hcsp, scbp);
            case DATA_OUT:      /* Data out phase               */
                return  xfer_data_out(hcsp, scbp);
            case MSG_IN:        /* Message in phase             */
                scbp->NxtStat = 0x4;
                if ((phase =  msgin(hcsp, scbp)) == -1)
                    return phase;
                break;
            case STATUS:        /* Status phase                 */
                if ((phase = status_msg(hcsp, scbp)) == -1) {
                    if ( (!(scbp->Mode & SCM_RSENS)) &&
                        (!(scbp->TaStat == TARGET_BUSY)) ) {
                        /* not in auto req sense modeand target not busy*/
                        if (scbp->Srb->DataTransferLength -= scbp->BufLen)
                            if ((scbp->Flags & SCF_DIR) != SCF_NO_DCHK)
                                scbp->HaStat = HOST_DO_DU;    /* 10/21/97    */
                    }

                    return phase;
                }
                break;
            case MSG_OUT:       /* Message out phase            */
                if ( hcsp->JSStatus0 & TSS_PAR_ERROR ) {
                    scbp->BufLen = 0;
                    scbp->HaStat = HOST_DO_DU;
                    if ((phase =  msgout_ide(hcsp)) == -1)
                        return phase;
                    goto state_6;
                } else
                 {
                    if ((phase =  msgout_nop(hcsp)) == -1)
                        return phase;
                }
                break;
            default:
                return  bad_seq(hcsp);
            }
        }

    case 5:
        cnt = TUL_RDLONG(hcsp->Base, TUL_SCnt0) & 0xffffff;

        /************** DMA direction == DATA_IN **************************/
        if (TUL_RD(hcsp->Base, TUL_XCmd) & 0x20) { /* check direction */
            if ( hcsp->JSStatus0 & TSS_PAR_ERROR ) {
                scbp->HaStat = HOST_DO_DU;
            }
            /*data underrun: when mode
            sense cmd (0x1A) request 192 byte, but target drive xfer
            146 byte then change phase to STATUS, in this case, we need
            to clean up the DMA xfer  */
            sta = TUL_RD(hcsp->Base, TUL_XStatus);
            if ((sta & XPEND)) {               /* DMA xfer pending */

                /* force DMA xfer the left over bytes */
                ctrl = TUL_RD(hcsp->Base, TUL_XCtrl);
                TUL_WR(hcsp->Base + TUL_XCtrl, ctrl | 0x80 );  /* set bit 7 */
                /* wait until DMA xfer not pending */
                while ( (sta = TUL_RD(hcsp->Base, TUL_XStatus)) & XPEND ) {
                }
                TUL_WR(hcsp->Base + TUL_XCtrl, ctrl);  /* clear bit 7*/

            }       /* if (sta & XPEND) : DMA xfer pending */

        } else
         {
            /************** DMA direction == DATA OUT *******************/
            if (!(TUL_RD(hcsp->Base, TUL_SStatus1) & TSS_XFER_CMP)) {
                fifoCnt = TUL_RD(hcsp->Base, TUL_SFifoCnt) & 0x1f;
                if (tcsp->JS_PeriodOffset & TSC_WIDE_SCSI)
                    fifoCnt = fifoCnt << 1;
                cnt += fifoCnt;
                TUL_WR(hcsp->Base + TUL_SCtrl0,   TSC_FLUSH_FIFO);
            }

            /* st5_xfer_done: */
            /* if DMA xfer is pending, abort DMA xfer */
            if (TUL_RD(hcsp->Base, TUL_XStatus) & XPEND) {
                ULONG countdown = 500;
                /* ----------- ABORT ----------------------*/
/*                TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_ABT | TAX_X_CLR_FIFO);*/
                TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_ABT); /* 02/26/98 */
                /* wait Abort DMA xfer done */
                while (((TUL_RD(hcsp->Base, TUL_Int) & XABT) == 0) &&
                       (countdown != 0)) {
                    ScsiPortStallExecution(1);
                    countdown--;
                }
                if(countdown == 0) {
                    DebugPrint((0, "initio::nextstate - XABT bit was never set\n"));
                }
            }
        } /* DMA direction == DATA_OUT */

        xcnt = scbp->BufLen - cnt;
        scbp->BufLen = cnt;/* left byte cnt to be xfer */

        if (scbp->BufLen == 0) {
            goto state_6;
        }
        if (scbp->Flags & SCF_SG) {
            register SG *sgp;
            ULONG       i;

            sgp = &scbp->SGList[scbp->SGIdx];
            for (i = scbp->SGIdx; i < scbp->SGMax; sgp++, i++) {
                xcnt -= sgp->Len;
                if (xcnt < 0) {
                    xcnt += sgp->Len;
                    sgp->Ptr += (ULONG)xcnt;
                    sgp->Len -= (ULONG)xcnt;
                    scbp->BufPtr += ((ULONG)(i - scbp->SGIdx) << 3);
                    scbp->SGLen = (UBYTE)(scbp->SGMax - i);
                    scbp->SGIdx = (UWORD)i;
                    goto state_4;
                }
            } /* for */

            goto state_6;
        } else
         {
            scbp->BufPtr += xcnt;
        }
        goto state_4;

    case 6:
state_6:
        phase = hcsp->Phase;
        for (; ; ) {
            switch (phase) {
            case MSG_IN:
                scbp->NxtStat = 0x6;
                if ((phase  =  msgin(hcsp, scbp)) == -1)
                    return phase;
                break;
            case STATUS:
                if ((phase =  status_msg(hcsp, scbp)) == -1)
                    return phase;
                break;
            case MSG_OUT:
                if ((phase =  msgout_nop(hcsp)) == -1)
                    return phase;
                break;
            case DATA_IN:
                phase = xfer_pad_in(hcsp, scbp);
                break;
            case DATA_OUT:

                phase = xfer_pad_out(hcsp, scbp);
                break;
            default:
                return  bad_seq(hcsp);
            }
        } /* for */

    case 7: /* After xfer pad */
        if ( (cnt = TUL_RD(hcsp->Base, TUL_SFifoCnt) & 0x1F) != 0) {
            for (h = 0; h < cnt; h++)          /* flush SCSI FIFO */
                TUL_RD(hcsp->Base, TUL_SFifo);
        }

        phase = hcsp->Phase;

        if ((phase == DATA_IN) || (phase == DATA_OUT)) {
            return  bad_seq(hcsp);
        }
        goto state_6;


    case 8:                         /* bus device reset */
        if ((hcsp->Phase) != MSG_OUT) {
            return bad_seq(hcsp);
        }

        TUL_WR(TulSFifo, MSG_DEVRST);
        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);

        scbp->HaStat = 0;
        scbp->Status = TULSCB_DONE;                /* command done */
        tcsp = hcsp->ActTcs;

        tcsp->Flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
        tcsp->JS_PeriodOffset = 0;

        tar = scbp->Target;                     /* target */

        /* abort all SCB with same target */
        pTmpScb = hcsp->FirstBusy;
        while (pTmpScb != NULL) {
            if (pTmpScb->Status & TULSCB_BUSY) {
                if (pTmpScb->Target == tar) {
                    pTmpScb->HaStat = 0x1C;
                    pTmpScb->Status = TULSCB_DONE;  /* command done */
                }
            }
            pTmpScb = (PTULSCB)(pTmpScb->NextScb);
        }

        if (wait_tulip(hcsp) != -1) {
            return bad_seq(hcsp);
        }
        return - 1;

    default:
        return  bad_seq(hcsp);
    }/* switch */
}


/****** xfer_data_in rtn **************************************/
char    xfer_data_in(hcsp, scbp)
register HCS    *hcsp;
register TULSCB    *scbp;
{
    TUL_WRLONG(hcsp->Base + TUL_SCnt0, scbp->BufLen);

    if (scbp->Flags & SCF_SG) {           /* S/G xfer */
        TUL_WRLONG(hcsp->Base + TUL_XCntH, ((ULONG)scbp->SGLen) << 3);
        TUL_WRLONG(hcsp->Base + TUL_XAddH, scbp->BufPtr);
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_SG_IN);
    } else {
        TUL_WRLONG(hcsp->Base + TUL_XCntH, scbp->BufLen);
        TUL_WRLONG(hcsp->Base + TUL_XAddH, scbp->BufPtr);
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_IN);
    }

    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_DMA_IN);

    scbp->NxtStat = 0x5;
    return 0;
}


/****************** xfer_data_out rtn *******************************/
char    xfer_data_out(hcsp, scbp)
register HCS    *hcsp;
register TULSCB        *scbp;
{
    TUL_WRLONG(hcsp->Base + TUL_SCnt0, scbp->BufLen);

    if (scbp->Flags & SCF_SG)       {           /* S/G xfer */
        TUL_WRLONG(hcsp->Base + TUL_XCntH, ((ULONG)scbp->SGLen) << 3);
        TUL_WRLONG(hcsp->Base + TUL_XAddH, scbp->BufPtr);
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_SG_OUT);
    } else {
        TUL_WRLONG(hcsp->Base + TUL_XCntH, scbp->BufLen);
        TUL_WRLONG(hcsp->Base + TUL_XAddH, scbp->BufPtr);
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_OUT);
    }

    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_DMA_OUT);

    scbp->NxtStat = 0x5;

    return 0;
}


/*****************   xfer_pad_in rtn **********************************/
char    xfer_pad_in(hcsp, scbp)
register HCS    *hcsp;
register TULSCB    *scbp;
{
    ULONG_PTR   TulSCmd, TulSFifo;
    ULONG   cnt, i = 0;
    char    phase;
    TCS     * tcsp;
    UCHAR   readByte;


    TulSCmd =  hcsp->Base + TUL_SCmd;
    TulSFifo = hcsp->Base + TUL_SFifo;

    tcsp = &hcsp->Tcs[scbp->Target];

    phase = TUL_RD(hcsp->Base, TUL_SSignal) & 0x07;
    while (phase == DATA_IN) {
        if ((scbp->Flags & SCF_DIR) != SCF_NO_DCHK)  {
            scbp->HaStat = HOST_DO_DU;    /* over run */
        }
        cnt = 1;
        if (tcsp->JS_PeriodOffset & TSC_WIDE_SCSI)
            cnt = cnt << 1;
        TUL_WRLONG(hcsp->Base + TUL_SCnt0, cnt );


        TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
        if ((phase = wait_tulip(hcsp)) == -1)
            return phase;

        readByte = TUL_RD(hcsp->Base, TUL_SFifo);

        phase = TUL_RD(hcsp->Base, TUL_SSignal) & 0x07;


    } /* while (phase == DATA_IN))  */

    TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);   /* flush SCSI FIFO */

    return phase;
}


/*****************   xfer_pad_out rtn **********************************/
char    xfer_pad_out(hcsp, scbp)
register HCS    *hcsp;
register TULSCB    *scbp;
{
    ULONG_PTR   TulSCmd, TulSFifo;
    TCS     * tcsp;
    char    phase;
    int    i;


    phase = TUL_RD(hcsp->Base, TUL_SSignal) & 0x07;

    while (phase == DATA_OUT) {

        TulSCmd =  hcsp->Base + TUL_SCmd;
        TulSFifo = hcsp->Base + TUL_SFifo;

        if ((scbp->Flags & SCF_DIR) != SCF_NO_DCHK)  {
            scbp->HaStat = HOST_DO_DU;    /* over run */
        }

        tcsp = &hcsp->Tcs[scbp->Target];
        if (tcsp->JS_PeriodOffset & TSC_WIDE_SCSI)  {
            TUL_WRLONG(hcsp->Base + TUL_SCnt0, 0x2 );
        } else {
            TUL_WRLONG(hcsp->Base + TUL_SCnt0, 0x1);
        }
        TUL_WR(TulSFifo, 0);

        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
        if ((phase = wait_tulip(hcsp)) == -1)
            return phase;

        /* wait xfer_pad done or phase change */
        phase = TUL_RD(hcsp->Base, TUL_SSignal) & 0x07;
    } /* if (phase == DATA_OUT) */


    TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT ) ;/* Enable HW reselect */
    TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);   /* flush SCSI FIFO */

    return phase;
}


/**************** status_msg rtn ***************************/
char    status_msg(hcsp, scbp)            /* status & MSG_IN */
register HCS    *hcsp;
register TULSCB    *scbp;
{
    char    phase;
    UBYTE               msg;
    ULONG_PTR   TulSCmd;
    UCHAR   readByte;


    TulSCmd =  hcsp->Base + TUL_SCmd;

    TUL_WR(TulSCmd, TSC_CMD_COMP);

    if ((phase = wait_tulip(hcsp)) == -1) {
        return - 1;
    }

    /* get status */
    scbp->TaStat = TUL_RD(hcsp->Base, TUL_SFifo);

    hcsp->JSStatus0 = TUL_RD(hcsp->Base, TUL_SStatus0);
    hcsp->Phase = hcsp->JSStatus0 & TSS_PH_MASK;


    if (hcsp->Phase == MSG_IN) {
        /* handle MSG_IN phase */
        if (hcsp->JSStatus0 & TSS_PAR_ERROR) {
            /*------------------ parity_err: ------------------*/
            if ((phase = msg_accept(hcsp)) == -1) {
                return phase;
            }
            if (phase != MSG_OUT) {
                return bad_seq(hcsp);
            } else {
                TUL_WR(hcsp->Base + TUL_SFifo, MSG_PARITY);
                TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
                return wait_tulip(hcsp);
            }
        }
        if ( (msg = TUL_RD(hcsp->Base, TUL_SFifo)) == 0) {/* command complete */
            if (( scbp->TaStat & 0x18) == 0x10 ) {
                return bad_seq(hcsp); /* no link support */
            }

            hcsp->Flags |= HCF_EXPECT_DISC;
            if ((phase = msg_accept(hcsp)) != -1)
                return bad_seq(hcsp);
            ll_unlink_busy_scb(hcsp, scbp);
            scbp->Status = TULSCB_DONE;
            ll_append_done_scb(hcsp, scbp);
            return - 1;

        } else {
            /*--------------------- chk_link_msg: ---------------*/
            if ((msg != MSG_LINK_COMP) && (msg != MSG_LINK_FLAG)) {
                return bad_seq(hcsp);
            } else { /* link_msg */
                if (( scbp->TaStat & 0x18) != 0x10 )
                    return bad_seq(hcsp); /* bad link msg */
                return msg_accept(hcsp);
            }
        }
    } else {
        if (hcsp->Phase != MSG_OUT) {
            return bad_seq(hcsp);
        }

        if (hcsp->JSStatus0 & TSS_PAR_ERROR) {
            TUL_WR(hcsp->Base + TUL_SFifo, MSG_PARITY);
        } else {
            TUL_WR(hcsp->Base + TUL_SFifo, MSG_NOP);
        }
        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
        return wait_tulip(hcsp);
    }
}


/******************* int_resel rtn *************************/
/* scsi reselection */
int    int_resel(hcsp)
register HCS    *hcsp;
{
    int    phase = 0;
    UWORD   tar, lun;
    UBYTE   msg;
    UWORD   tarlun, scbp_tarlun;
    register TULSCB     *scbp;
    TCS     * tcsp;
    ULONG_PTR   TulSCmd;
    UBYTE readByte;

    TulSCmd =  hcsp->Base + TUL_SCmd;

    if ((scbp = hcsp->ActScb) != NULL) {
        if (scbp->Status & TULSCB_SELECT) {
            scbp->Status &= ~TULSCB_SELECT;
        }
        hcsp->ActScb = NULL;
    }


    /* ------------no active SCB, get target id-------- */
    tar =  TUL_RD(hcsp->Base, TUL_SBusId);
    tar &= 0x0F;

    /* ---------- check if parity err ---------------- */
    if (hcsp->JSStatus0 & TSS_PAR_ERROR) {
        if ((phase =  msgin_par_err(hcsp, scbp)) == -1)
            return - 1;
    }

    /*--------------- get ident ---------------------*/

    /* get LUN from Identify message */
    msg = TUL_RD(hcsp->Base, TUL_SIdent);
    lun = msg & 0x1F;        /* 10/21/97    & 0x7;        */

    tarlun = tar | lun << 8;
    tcsp = &hcsp->Tcs[tar];

    mod_ALTPD(hcsp, tcsp);
    TUL_WR(hcsp->Base + TUL_SPeriodOffset, tcsp->JS_PeriodOffset);


    /* ------------- tag queueing ? ------------------- */
    msg = 0;
    if (tcsp->Flags & TCF_EN_TAG) {
        /*------------ get Tag msg ------------------------*/
        if ((phase =  msg_accept(hcsp)) == -1)
            return phase;
        if (phase == MSG_IN) {
            TUL_WRLONG(hcsp->Base + TUL_SCnt0, 1);
            TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
            if ((phase = wait_tulip(hcsp)) == -1)
                return phase;
            msg = TUL_RD(hcsp->Base, TUL_SFifo);
            if ((phase =  msg_accept(hcsp)) == -1)
                return phase;
        }
    }

    if (( MSG_STAG <= msg <= MSG_OTAG) && (phase == MSG_IN)) {
        UBYTE       tag;

        /*------------ get Tag ID ------------------------*/
        TUL_WRLONG(hcsp->Base + TUL_SCnt0, 1);
        TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
        if ((phase = wait_tulip(hcsp)) == -1)
            return phase;
        tag = TUL_RD(hcsp->Base, TUL_SFifo);

        /* accept tagId */
        if ((phase =  msg_accept(hcsp)) == -1)
            return phase;
        if ( (scbp = ll_find_tagid_busy_scb(hcsp, tarlun, tag)) == NULL) {
            TulPanic("bug??");
        }
        scbp_tarlun = (scbp->Lun << 8) | scbp->Target;

        if ( scbp_tarlun != tarlun) {
            msgout_abort_tag(hcsp);
            return - 1;
        }
        if ( scbp->Status != TULSCB_BUSY) {           /* 3/24/95 */
            msgout_abort_tag(hcsp);
            return - 1;
        }
    } else /* no tag */
     {
        scbp = ll_find_busy_scb(hcsp, tarlun);
        if (scbp == NULL) {
            msgout_abort(hcsp);
            return - 1;
        }
        if (!(tcsp->Flags & TCF_EN_TAG)) {
            if ((phase =  msg_accept(hcsp)) == -1)
                return phase;
        }

    }
    hcsp->ActTcs = tcsp;
    hcsp->ActScb = scbp;

    next_state(hcsp, scbp);

    return phase;
}


/***************************************************************************/
int    msgin_par_err(hcsp, scbp)
register HCS    *hcsp;
register TULSCB *scbp;
{
    ULONG_PTR   TulSCmd;
    char    phase = 0;

    TulSCmd =  hcsp->Base + TUL_SCmd;

    TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);
    if ((phase =  msg_accept(hcsp)) == -1)
        return phase;

    if (phase == MSG_OUT) {
        TUL_WR(hcsp->Base + TUL_SFifo, MSG_PARITY);

        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
        if ((phase = wait_tulip(hcsp)) == -1)
            return phase;

        if (phase == MSG_IN) {
            TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
            if ((phase = wait_tulip(hcsp)) == -1)
                return phase;

            /* Is still parity error ? */
            if (!(hcsp->JSStatus0 & TSS_PAR_ERROR) )
                return phase;
        }
    }
    return bad_seq(hcsp);
}


/****************** int_busfree rtn ********************************************/
/* scsi bus free */
char    int_busfree(hcsp)
register HCS    *hcsp;
{
    register TULSCB    *scbp;
    ULONG_PTR   TulSCmd;

    TulSCmd =  hcsp->Base + TUL_SCmd;
    scbp = hcsp->ActScb;
    if (scbp = hcsp->ActScb) {
        hcsp->ActScb = NULL;
        hcsp->ActTcs = NULL;
        if (scbp->Status & TULSCB_SELECT)  { /* selection timeout */
            if (scbp != ll_pop_pend_scb(hcsp)) {
                TulPanic("int_busfree: scbp != ll_pop_pend_scb(hcsp)\n");
            }
            scbp->Status = TULSCB_DONE;
            scbp->HaStat = HOST_SEL_TOUT;
            ll_append_done_scb(hcsp, scbp);
        } else {   /* unexpected bus free */
            ll_unlink_busy_scb(hcsp, scbp);
            scbp->Status = TULSCB_DONE;
            scbp->HaStat = HOST_BUS_FREE;
            ll_append_done_scb(hcsp, scbp);
        }
    }
    TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);   /* flush SCSI FIFO */
    TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT ) ;/* Enable HW reselect */
    return - 1;                      /* 07/31/97     */
}


/***************************************************************************/
/* scsi bus reset */
char    int_scsi_rst(hcsp)
register HCS    *hcsp;
{
    int    i;

    /* if DMA xfer is pending, abort DMA xfer */
    if (TUL_RD(hcsp->Base, TUL_XStatus) & XPEND) {
        /* Abort Bus Master xfering */
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_ABT | TAX_X_CLR_FIFO);
        /* wait Abort DMA xfer done */
        while ((TUL_RD(hcsp->Base, TUL_Int) & XABT) == 0)  {
        }
    }

    /* reset tulip chip */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_CHIP);
    se2_wait(hcsp);                      /* wait 30 us */


    TUL_WR( hcsp->Base + TUL_SIntEnable, 0xFF);

    /* selection time out = 250 ms */
    TUL_WR(hcsp->Base + TUL_STimeOut, 153);

    /* Enable Initiator Mode ,phase latch,alternate sync period mode,
    disable SCSI reset */
    if (hcsp->Config & HCC_EN_PAR)
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT | TSC_EN_SCSI_PAR);
    else
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT );

    post_scsi_rst(hcsp);
    ScsiPortStallExecution(1000000); /* wait 1 sec            */

    return - 1;                      /* 07/31/97     */
}


/***************************************************************************/
char    bad_seq(hcsp)
register HCS    *hcsp;
{
    register TULSCB *scbp;

    if (scbp = hcsp->ActScb) {
        ll_unlink_busy_scb(hcsp, scbp);
        scbp->HaStat = HOST_BAD_PHAS;
        scbp->TaStat = 0;
        scbp->Status = TULSCB_DONE;        /* done */
        ll_append_done_scb(hcsp, scbp);
    }
    reset_scsi(hcsp);

    return - 1;
}


/*******************reset_scsi for initialize ***************************/
void init_reset_scsi(hcsp)
register HCS    *hcsp;
{
    int    i;
    TCS     * tcsp;

    /* reset tulip chip */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_CHIP);
    se2_wait(hcsp);                      /* wait 30 us */

    TUL_WR( hcsp->Base + TUL_SIntEnable, 0xFF);

    TUL_WR(hcsp->Base + TUL_SSignal,  0 );
    /* if DMA xfer is pending, abort DMA xfer */
    if (TUL_RD(hcsp->Base, TUL_XStatus) & XPEND) {
        /* ----------- ABORT ----------------------*/
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_ABT | TAX_X_CLR_FIFO);
        /* wait Abort DMA xfer done */
        while ((TUL_RD(hcsp->Base, TUL_Int) & XABT) == 0)
            ;
    }


    /* Reset SCSI bus */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_BUS);

    while ( (hcsp->JSInt = TUL_RD(hcsp->Base, TUL_SInt)) & TSS_SCSIRST_INT) {
    }

    ScsiPortStallExecution(10000000); /* wait 10 sec        */

    /* program HBA's SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId, hcsp->HaId << 4);

    if (hcsp->Config & HCC_EN_PAR)
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT | TSC_EN_SCSI_PAR);
    else
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT );

    /* Initialize the sync. xfer register values to an narrow, async. xfer */
    TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);

    /* selection time out = 250 ms */
    TUL_WR(hcsp->Base + TUL_STimeOut, 153);

    tcsp = &hcsp->Tcs[0];
    for (i = 0; i < 16; tcsp++, i++)   {
        tcsp->Flags &=  ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
    } /* for */

//    if (hwInitialized) {
        post_scsi_rst(hcsp);
//    }
}


/*******************reset_scsi rtn ***************************/
void reset_scsi(hcsp)
register HCS    *hcsp;
{
    int    i;
    TCS     * tcsp;

    /* reset tulip chip */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_CHIP);
    se2_wait(hcsp);                      /* wait 30 us */

    TUL_WR( hcsp->Base + TUL_SIntEnable, 0xFF);

    TUL_WR(hcsp->Base + TUL_SSignal,  0 );
    /* if DMA xfer is pending, abort DMA xfer */
    if (TUL_RD(hcsp->Base, TUL_XStatus) & XPEND) {
        /* ----------- ABORT ----------------------*/
        TUL_WR(hcsp->Base + TUL_XCmd, TAX_X_ABT | TAX_X_CLR_FIFO);
        /* wait Abort DMA xfer done */
        while ((TUL_RD(hcsp->Base, TUL_Int) & XABT) == 0)
            ;
    }


    /* Reset SCSI bus */
    TUL_WR( hcsp->Base + TUL_SCtrl0, TSC_RST_BUS);

    while ( (hcsp->JSInt = TUL_RD(hcsp->Base, TUL_SInt)) & TSS_SCSIRST_INT) {
    }

    ScsiPortStallExecution(5000000); /* wait 5 sec        */

    /* program HBA's SCSI ID */
    TUL_WR( hcsp->Base + TUL_SScsiId, hcsp->HaId << 4);

    if (hcsp->Config & HCC_EN_PAR)
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT | TSC_EN_SCSI_PAR);
    else
        TUL_WR(hcsp->Base + TUL_SConfig, TSC_INITDEFAULT );

    /* Initialize the sync. xfer register values to an narrow, async. xfer */
    TUL_WR(hcsp->Base + TUL_SPeriodOffset, 0x0);

    /* selection time out = 250 ms */
    TUL_WR(hcsp->Base + TUL_STimeOut, 153);

    tcsp = &hcsp->Tcs[0];
    for (i = 0; i < 16; tcsp++, i++)   {
        tcsp->Flags &=  ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
    } /* for */

//    if (hwInitialized) {
        post_scsi_rst(hcsp);
//    }
}


/***************************************************************************/
int    post_scsi_rst(hcsp)
register HCS    *hcsp;
{
    register TULSCB    *scbp;
    int    i;

    while ((scbp = ll_pop_busy_scb(hcsp)) != NULL) {
        scbp->Status = TULSCB_DONE;
        scbp->HaStat = HOST_BAD_PHAS;
        ll_append_done_scb(hcsp, scbp);
    }

    hcsp->ActScb = 0;
    hcsp->NxtPend = 0;
    hcsp->NxtContig = -1;

    for (i = 0; i < 16; i++) {
        hcsp->Tcs[i].Flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
    }

    return 1;
}


/***************************************************************************/
char    msgout_abort(hcsp)
register HCS    *hcsp;
{
    char    phase;


    TUL_WR(hcsp->Base + TUL_SSignal,
        (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);
    if ( (phase = msg_accept(hcsp)) == -1)
        return phase;


    if (phase != MSG_OUT) {
        return  bad_seq(hcsp);
    }
    TUL_WR(hcsp->Base + TUL_SFifo, MSG_ABORT);
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);

    hcsp->Flags |= HCF_EXPECT_DISC;
    if (wait_tulip(hcsp) != -1) {
        return  bad_seq(hcsp);
    }

    return - 1;
}


/***************************************************************************/
char    msgout_abort_tag(hcsp)
register HCS    *hcsp;
{
    char    phase;

    TUL_WR(hcsp->Base + TUL_SSignal,
        (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);
    if ( (phase = msg_accept(hcsp)) == -1)
        return phase;

    if (phase != MSG_OUT) {
        return  bad_seq(hcsp);
    }

    TUL_WR(hcsp->Base + TUL_SSignal,
        (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47));
    TUL_WR(hcsp->Base + TUL_SFifo, MSG_ABORT_TAG);
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);

    hcsp->Flags |= HCF_EXPECT_DISC;
    if (wait_tulip(hcsp) != -1) {
        return  bad_seq(hcsp);
    }

    return - 1;
}


/****************  msgin rtn *******************************/
char    msgin(hcsp, scbp)
register HCS    *hcsp;
register TULSCB *scbp;
{
    char    phase;
    UBYTE   imsg, readByte;
    ULONG_PTR   TulSCmd;
    register TCS    *tcsp;
    long    cnt;

    TulSCmd =  hcsp->Base + TUL_SCmd;
    if ( (cnt = TUL_RD(hcsp->Base, TUL_SFifoCnt) & 0x1F) != 0) {
        readByte = TUL_RD(hcsp->Base, TUL_SSignal);
        readByte &= 0x47; /* set BSYO,ATNO to 0,release ATN */
        TUL_WR(hcsp->Base + TUL_SSignal, readByte);/* deassert ATN */
        TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);  /* flush SCSI FIFO */
    }

    for (; ; ) {
        TUL_WRLONG(hcsp->Base + TUL_SCnt0, 1);
        TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
        if ((phase = wait_tulip(hcsp)) == -1)
            return - 1;
        imsg = TUL_RD(hcsp->Base, TUL_SFifo);
        switch (imsg) {
        case MSG_DISC:               /* Disconnect msg */
            return msgin_discon(hcsp);
            break;

        case MSG_SDP:
        case MSG_RESTORE:
        case MSG_NOP:
            phase =  msg_accept(hcsp);
            break;

        case MSG_REJ:
            tcsp = &hcsp->Tcs[scbp->Target];

            if ( (tcsp->Flags & (TCF_SYNC_DONE | TCF_NO_SYNC_NEGO)) == 0) {
                return  msg_accept(hcsp);
            }
            phase = msg_accept(hcsp);
            break;

        case MSG_EXTEND:               /* extended msg */
            phase =  msgin_extend(hcsp, scbp);
            break;
        case MSG_COMP:
            hcsp->Flags |= HCF_EXPECT_DISC;

            /* Check for disconnect. */
            if ((phase = msg_accept(hcsp)) == -1)
                return -1;
            
            ll_unlink_busy_scb(hcsp, hcsp->ActScb);
            hcsp->ActScb->Status = TULSCB_DONE;
            ll_append_done_scb(hcsp, hcsp->ActScb);
            break;
        default:
            phase =  msgout_reject(hcsp);
            break;
        }
        if (phase != MSG_IN)  {
            return phase;
        }
    }
    /* statement won't reach here */
}


/******************* msgin_discon rtn *******************/
char    msgin_discon(hcsp)
register HCS    *hcsp;
{
    hcsp->Flags |= HCF_EXPECT_DISC;
    return msg_accept(hcsp);
}


/************************ msgout_reject rtn ****************/
char    msgout_reject(hcsp)
register HCS    *hcsp;
{
    char    phase;

    TUL_WR(hcsp->Base + TUL_SSignal,
        (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);

    if ( (phase =  msg_accept(hcsp)) == -1) {
        return phase;
    }

    if (phase == MSG_OUT) {
        TUL_WR(hcsp->Base + TUL_SFifo, MSG_REJ); /* msg rej */
        TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);
        return wait_tulip(hcsp);
    }
    return phase;
}


/************** msgout_sync rtn **************************/
int    msgout_sync(hcsp)
register HCS    *hcsp;
/* UBYTE   msg[];        */
{
UCHAR     xfrPeriod[8] = {   /* fast 20 */
    /* nanosecond devide by 4 */
    12,              /* 50ns,  20M       */
    18,              /* 75ns,  13.3M */
    25,              /* 100ns, 10M   */
    31,              /* 125ns, 8M    */
    37,              /* 150ns, 6.6M  */
    43,              /* 175ns, 5.7M  */
    50,              /* 200ns, 5M    */
    62               /* 250ns, 4M    */
};

    char    phase;


    TUL_WR(hcsp->Base + TUL_SFifo, MSG_EXTEND);
    TUL_WR(hcsp->Base + TUL_SFifo, 3);
    TUL_WR(hcsp->Base + TUL_SFifo, 1);
    TUL_WR(hcsp->Base + TUL_SFifo, xfrPeriod[hcsp->ActTcs->xfrPeriodIdx]);
    TUL_WR(hcsp->Base + TUL_SFifo, MAX_OFFSET);
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);
    phase = wait_tulip(hcsp);
    TUL_WR(hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);
    TUL_WR(hcsp->Base + TUL_SSignal, TUL_RD(hcsp->Base, TUL_SSignal) & 0x47);

    return phase;
}


/***************************************************************************/
char    msgout_nop(hcsp)
register HCS    *hcsp;
{
    TUL_WR(hcsp->Base + TUL_SFifo, MSG_NOP); /* msg nop */
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);
    return wait_tulip(hcsp);
}


/***************************************************************************/
char    msgout_ide(hcsp)
register HCS    *hcsp;
{
    TUL_WR(hcsp->Base + TUL_SFifo, MSG_IDE); /* Initiator Detected Error */
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_XF_FIFO_OUT);
    return wait_tulip(hcsp);
}


/********************** msgin_extend rtn *************************************/
char    msgin_extend(hcsp, scbp)
register HCS    *hcsp;
register TULSCB *scbp;
{
#define MSG_BUFF_SIZE 10 

    char    phase;
    ULONG   len, idx;
    UBYTE   msg[MSG_BUFF_SIZE];
    ULONG_PTR   TulSCmd;
    ULONG_PTR   TulSFifo;
    register TCS    *tcsp;

    TulSCmd =  hcsp->Base + TUL_SCmd;
    TulSFifo =  hcsp->Base + TUL_SFifo;

    phase =  msg_accept(hcsp);
    if (phase != MSG_IN)
        return phase;

    TUL_WRLONG(hcsp->Base + TUL_SCnt0, 1);
    TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
    if ( (phase = wait_tulip(hcsp)) == -1)
        return - 1;
    len = TUL_RD(hcsp->Base, TUL_SFifo);

    msg[0] = (UBYTE)len;

    //
    // this should always be true, but if its not the lower check on idx
    // will prevent an overflow.  And the assert will catch this in testing.
    //
    ASSERT(msg[0] < MSG_BUFF_SIZE);

    for (idx = 1 ; (len != 0) && (idx < MSG_BUFF_SIZE); len--) {
        phase =  msg_accept(hcsp);
        if (phase != MSG_IN)
            return phase;

        TUL_WRLONG(hcsp->Base + TUL_SCnt0, 1);
        TUL_WR(TulSCmd, TSC_XF_FIFO_IN);
        if ( (phase = wait_tulip(hcsp)) == -1)
            return - 1;
        msg[idx++]  = TUL_RD(hcsp->Base, TUL_SFifo);
    }


    if (msg[1] == 1) {
        if (msg[0] != 3)
            return  msgout_reject(hcsp);
        if ((hcsp->ActTcs)->Flags & TCF_NO_SYNC_NEGO) {
            msg[3] = 0;
        } else {
            if ((msgin_sync(hcsp, msg) == 0) &&
                ( (hcsp->ActTcs)->Flags & TCF_SYNC_DONE ) ) {
                sync_done(hcsp, msg);
                return msg_accept(hcsp);
            }
        }

        TUL_WR(hcsp->Base + TUL_SSignal,
            (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);

        phase =  msg_accept(hcsp);
        if (phase != MSG_OUT) {
            TUL_WR(hcsp->Base + TUL_SSignal,
                (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47));
            return phase;
        }

        TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);

        sync_done(hcsp, msg);

        /* sync msg out */
        TUL_WR(TulSFifo, MSG_EXTEND);
        TUL_WR(TulSFifo, 3);
        TUL_WR(TulSFifo, 1);
        TUL_WR(TulSFifo, msg[2]);       /* period */
        TUL_WR(TulSFifo, msg[3]);       /* offset */
        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
        if ((phase = wait_tulip(hcsp)) == -1)
            return - 1;
        return phase;
    } else if (msg[1] == 3) {
        if (msg[0] != 2)
            return  msgout_reject(hcsp);

        if (msg[2] > 2)
            return  msgout_reject(hcsp);
        /*-------------- msg[2] <= 2 --------------------*/
        if (msg[2] != 2) {
            tcsp = &hcsp->Tcs[scbp->Target];
            if ( (tcsp->Flags & TCF_NO_WDTR) == 0) {
                wdtr_done(hcsp, msg);
                if ((tcsp->Flags & TCF_NO_SYNC_NEGO) == 0)      {
                    TUL_WR(hcsp->Base + TUL_SSignal,
                        (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);
                }
                return  msg_accept(hcsp);
            }
        } else
            msg[2] = 1;

        TUL_WR(hcsp->Base + TUL_SSignal,
            (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47) | TSC_SET_ATN);
        phase =  msg_accept(hcsp);
        if (phase != MSG_OUT) {
            TUL_WR(hcsp->Base + TUL_SSignal,
                (TUL_RD(hcsp->Base, TUL_SSignal) & 0x47));
            return phase;
        }

        /* WDTR msg out */
        TUL_WR(TulSFifo, MSG_EXTEND);
        TUL_WR(TulSFifo, 2);
        TUL_WR(TulSFifo, 3);                            /* WDTR */
        TUL_WR(TulSFifo, msg[2]);
        TUL_WR(TulSCmd, TSC_XF_FIFO_OUT);
        if ((phase = wait_tulip(hcsp)) == -1)
            return - 1;

        wdtr_done(hcsp, msg);

        return phase;
    }

    return  msgout_reject(hcsp);
}


/**************** msgin_sync rtn ************************/
int    msgin_sync(hcsp, msg)
register HCS    *hcsp;
UBYTE   msg[];
{
UCHAR     xfrPeriod[8] = {   /* fast 20 */
    /* nanosecond devide by 4 */
    12,              /* 50ns,  20M       */
    18,              /* 75ns,  13.3M */
    25,              /* 100ns, 10M   */
    31,              /* 125ns, 8M    */
    37,              /* 150ns, 6.6M  */
    43,              /* 175ns, 5.7M  */
    50,              /* 200ns, 5M    */
    62               /* 250ns, 4M    */
};

    UBYTE   Period;

    Period = xfrPeriod[hcsp->ActTcs->xfrPeriodIdx];
    if (msg[3] > MAX_OFFSET) {
        msg[3] = MAX_OFFSET;
        if (msg[2] < Period) {
            msg[2] = Period;
            return 1;
        }
        if (msg[2] > 50) {
            msg[3] = 0;
        }
        return 1;
    }

    if (msg[3] == 0)  {
        return 0;
    }
    if (msg[2] < Period) {
        msg[2] = Period;
        return 1;
    }

    if (msg[2] >= 59) {
        msg[3] = 0;
        return 1;
    }
    return 0;
}


/************* wdtr_done rtn *****************************************/
int    wdtr_done(hcsp, msg)
register HCS    *hcsp;
UBYTE   msg[];
{
    TCS         * tcsp;

    tcsp = hcsp->ActTcs;
    tcsp->Flags &= ~TCF_SYNC_DONE;
    tcsp->Flags |= TCF_WDTR_DONE;

    if (msg[2] == 1)  {
        tcsp->JS_PeriodOffset = 0x0 | TSC_WIDE_SCSI;
    } else if (msg[2] == 0) {
        tcsp->JS_PeriodOffset = 0x0 ;
    }
    mod_ALTPD(hcsp, tcsp);
    TUL_WR(hcsp->Base + TUL_SPeriodOffset, tcsp->JS_PeriodOffset);

    return 1;
}


/*************** sync_done rtn *************************************/
void  sync_done(hcsp, msg)
register HCS    *hcsp;
UBYTE   msg[];
{
UCHAR     xfrPeriod[8] = {   /* fast 20 */
    /* nanosecond devide by 4 */
    12,              /* 50ns,  20M       */
    18,              /* 75ns,  13.3M */
    25,              /* 100ns, 10M   */
    31,              /* 125ns, 8M    */
    37,              /* 150ns, 6.6M  */
    43,              /* 175ns, 5.7M  */
    50,              /* 200ns, 5M    */
    62               /* 250ns, 4M    */
};

    TCS     * tcsp;
    int    i;

    tcsp = hcsp->ActTcs;
    tcsp->Flags |= TCF_SYNC_DONE;

    if (msg[3]) {
        tcsp->JS_PeriodOffset |= msg[3];
        /*---------- decide period (bit 6-4 ) ----------------*/

        for (i = 0; i < 8; i++) {
            if (xfrPeriod[i] >= msg[2] )  /* pick the big one */
                break;
        }
        tcsp->JS_PeriodOffset |= (i << 4);
        /*--------------- setup HW register ----------------------- */
        if (hcsp->Config & HCC_EN_PAR)
            tcsp->sConfig =  TSC_INITDEFAULT | TSC_EN_SCSI_PAR;
        else
            tcsp->sConfig = TSC_INITDEFAULT ;
    } else {
        if (tcsp->JS_PeriodOffset & TSC_WIDE_SCSI) {
            if (hcsp->Config & HCC_EN_PAR)
                tcsp->sConfig = TSC_INITDEFAULT_ALT0 | TSC_EN_SCSI_PAR;
            else
                tcsp->sConfig = TSC_INITDEFAULT_ALT0 ;
        } else {
            if (hcsp->Config & HCC_EN_PAR)
                tcsp->sConfig = TSC_INITDEFAULT | TSC_EN_SCSI_PAR;
            else
                tcsp->sConfig =  TSC_INITDEFAULT ;
        }
    } /* if async mode */

    TUL_WR(hcsp->Base + TUL_SPeriodOffset, tcsp->JS_PeriodOffset);
    mod_ALTPD(hcsp, tcsp);
}


/*************** select_atn_stop rtn **********************************/
void  select_atn_stop(hcsp, scbp)
register HCS    *hcsp;
TULSCB             *scbp;
{
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_SELATNSTOP);
    scbp->Status |= TULSCB_SELECT;
    scbp->NxtStat = 0x1;
    hcsp->ActScb = scbp;
    hcsp->ActTcs = &hcsp->Tcs[scbp->Target];
}


/********** select_atn rtn ****************************************************/
void  select_atn(hcsp, scbp)
register HCS    *hcsp;
TULSCB             *scbp;
{
    int    i;
    ULONG_PTR   TulSFifo;


    TulSFifo =  hcsp->Base + TUL_SFifo;

    TUL_WR(TulSFifo, scbp->Ident);

    for ( i = 0; i < (int)scbp->CDBLen; i++) {
        TUL_WR(TulSFifo, scbp->CDB[i]);
    }
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_SEL_ATN);
    scbp->Status |= TULSCB_SELECT;
    scbp->NxtStat = 0x2;
    hcsp->ActScb = scbp;
    hcsp->ActTcs = &hcsp->Tcs[scbp->Target];
}


/*********** select_atn3 rtn ******************************************/
void  select_atn3(hcsp, scbp)
register HCS    *hcsp;
TULSCB             *scbp;
{
    int    i;
    ULONG_PTR  TulSFifo;

    TulSFifo =  hcsp->Base + TUL_SFifo;
    /*-- send out 3 msg bytes & CDB bytes -------*/
    TUL_WR(TulSFifo, scbp->Ident);
    TUL_WR(TulSFifo, scbp->TagMsg);
    TUL_WR(TulSFifo, scbp->TagId);
    for ( i = 0; i < (int)scbp->CDBLen; i++) {
        TUL_WR(TulSFifo, scbp->CDB[i]);
    }

    TUL_WR(hcsp->Base + TUL_SCmd, TSC_SEL_ATN3);
    scbp->Status |= TULSCB_SELECT;
    scbp->NxtStat = 0x2;
    hcsp->ActScb = scbp;
    hcsp->ActTcs = &hcsp->Tcs[scbp->Target];
}


/****************** msg_accept rtn ***********************************/
/*-------------------------------------------------------------------*/
char    msg_accept(hcsp)
register HCS    *hcsp;
{
    TUL_WR(hcsp->Base + TUL_SCmd, TSC_MSG_ACCEPT);
    return wait_tulip(hcsp);
}


/****************** wait_tulip rtn *************************************/
char    wait_tulip(hcsp)
register HCS    *hcsp;
{
    ULONG_PTR   TulSCmd;
    UCHAR    stat2, sigl;


    TulSCmd =  hcsp->Base + TUL_SCmd;

    /* wait for SCSI interrupt, get interrupt register */
    for (; ; ) {
        hcsp->JSStatus0 = TUL_RD(hcsp->Base, TUL_SStatus0);
        if ((hcsp->JSStatus0 & TSS_INT_PENDING) == 0)
            continue;
        hcsp->JSInt = TUL_RD(hcsp->Base, TUL_SInt);
        break;
    }

    hcsp->Phase = hcsp->JSStatus0 & TSS_PH_MASK;
    hcsp->JSStatus1 = TUL_RD(hcsp->Base, TUL_SStatus1);


    if (hcsp->JSInt & TSS_SCSIRST_INT) {
        return int_scsi_rst(hcsp);
    }

    if (hcsp->JSInt & TSS_SEL_TIMEOUT) {
        return int_busfree(hcsp);
    }

    /* Happens when msg_accept SCSI cmd 0xF find out bus free */
    if (hcsp->JSInt & TSS_DISC_INT ) {                  /* BUS disconnection */
        if (hcsp->Flags & HCF_EXPECT_DISC) {
            TUL_WR(hcsp->Base + TUL_SCtrl0, TSC_FLUSH_FIFO);
            TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT);
            hcsp->ActTcs = NULL;
            hcsp->ActScb = NULL;
            hcsp->Flags &= ~HCF_EXPECT_DISC;
        } else {
            int_busfree(hcsp);  /* unexpected bus free */
        }
        return - 1;
    }


    if (hcsp->JSInt & (TSS_FUNC_COMP | TSS_BUS_SERV)) {
        /* if not other interrupts then function complete or bus service */
        return (hcsp->Phase);
    }

    return ( hcsp->Phase);
}


/*******************************************************/
int    wait_bus_free(hcsp, scbp)
register HCS    *hcsp;
register TULSCB *scbp;
{

    TUL_WR(hcsp->Base + TUL_SCmd, TSC_MSG_ACCEPT);
    TUL_WR( hcsp->Base + TUL_SCtrl0,  TSC_FLUSH_FIFO);

    while ( (TUL_RD(hcsp->Base, TUL_SStatus0) & TSS_INT_PENDING) == 0)
        ;

    TUL_RD(hcsp->Base, TUL_SInt);

    TUL_WR(hcsp->Base + TUL_SCtrl1, TSC_HW_RESELECT ) ;/* Enable HW reselect */
    hcsp->ActTcs = NULL;
    hcsp->ActScb = NULL;
    return - 1;
}


/***************************************************************************/
int    int_bad_seq(hcsp)                /* target wrong phase */
register HCS    *hcsp;
{
    UBYTE   i;
    TULSCB     * scbp;

    reset_scsi(hcsp);

    while ((scbp = ll_pop_busy_scb(hcsp)) != NULL) {
        if (scbp->Status & (TULSCB_BUSY | TULSCB_SELECT)) {
            scbp->Status = TULSCB_DONE;
            scbp->HaStat = HOST_BAD_PHAS;
            ll_append_done_scb(hcsp, scbp);
        }
    }

    for (i = 0; i < 16; i++)        {
        hcsp->Tcs[i].Flags &= ~(TCF_SYNC_DONE | TCF_WDTR_DONE);
    }
    return - 1;
}



/***************************************************************************/
void tul_en_sint(hcsp)
register HCS    *hcsp;
{
    TUL_WR(hcsp->Base + TUL_Mask, 0xF);
}


/***************************************************************************/
void tul_dis_sint(hcsp)
register HCS    *hcsp;
{
    TUL_WR(hcsp->Base + TUL_Mask, 0x1F);
}


/***************************************************************************/
void TulPanic(errMsg)
char    *errMsg;
{
    while (1)
        ;
}


/**************************************************************************
 Routine Description:
              Installable driver initialization entry point for system.
 Arguments:
              Driver Object
 Return Value:
              Status from ScsiPortInitialize()
**************************************************************************/
ULONG
DriverEntry(
IN PVOID DriverObject,
IN PVOID Argument2
)
{
    HW_INITIALIZATION_DATA hwInitializationData;
    ULONG Status, Status1;
    ULONG i;
    ULONG AdpterNo;
    static int    cardNo = 0;
    UCHAR VendorId[4] = {
        '1', '1', '0', '1'        };
    UCHAR DeviceId[4] = {
        '9', '5', '0', '0'        };
    UCHAR DeviceId1[4] = {
        '9', '4', '0', '0'        };
    UCHAR DeviceId2[4] = {
        '9', '4', '0', '1'        };
    UCHAR VendorId3[4] = {
        '1', '3', '4', 'A'        };
    UCHAR DeviceId3[4] = {
        '0', '0', '0', '2'        };

    /* Zero out structure. */
    for (i = 0; i < sizeof(HW_INITIALIZATION_DATA); i++) {
        ((PUCHAR) & hwInitializationData)[i] = 0;
    }

    /* Set size of hwInitializationData. */
    hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

    /* Set entry points. */
    hwInitializationData.HwInitialize = TulHwInitialize;
    hwInitializationData.HwResetBus = TulResetBus;
    hwInitializationData.HwStartIo = TulPutQueue;
    hwInitializationData.HwInterrupt = TulInterrupt;
    hwInitializationData.HwFindAdapter = TulFindAdapter;
//#ifndef WIN95
#if 1
    hwInitializationData.HwAdapterControl = TulAdapterControl;
#endif

    hwInitializationData.NeedPhysicalAddresses = TRUE;
    hwInitializationData.AutoRequestSense = TRUE;

#if TAG_QUEUING
    hwInitializationData.TaggedQueuing = TRUE;
#else
    hwInitializationData.TaggedQueuing = FALSE;
#endif
    hwInitializationData.MultipleRequestPerLu = TRUE;

    /* Specify size of extensions. */
    hwInitializationData.DeviceExtensionSize = sizeof(HW_DEVICE_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = 0;

    hwInitializationData.AdapterInterfaceType = PCIBus;


    hwInitializationData.VendorId = VendorId;
    hwInitializationData.VendorIdLength = 4;
    hwInitializationData.DeviceId = DeviceId;
    hwInitializationData.DeviceIdLength = 4;


    hwInitializationData.NumberOfAccessRanges = 1;

    hwInitializationData.SrbExtensionSize = sizeof(TULSCB) + 4;

    AdpterNo = 0;


    Status = ScsiPortInitialize(DriverObject, Argument2,
        &hwInitializationData, &AdpterNo);
    hwInitializationData.DeviceId = DeviceId1;
    Status1 = ScsiPortInitialize(DriverObject, Argument2,
        &hwInitializationData, &AdpterNo);

    if (Status > Status1)
        Status = Status1;

    hwInitializationData.DeviceId = DeviceId2;
    Status1 = ScsiPortInitialize(DriverObject, Argument2,
        &hwInitializationData, &AdpterNo);
    if (Status > Status1)
        Status = Status1;

    hwInitializationData.VendorId = VendorId3;    /* For DTCT 3194    */
    hwInitializationData.DeviceId = DeviceId3;
    Status1 = ScsiPortInitialize(DriverObject, Argument2,
        &hwInitializationData, &AdpterNo);
    if (Status > Status1)
        Status = Status1;
    return( Status );

} /* end TulEntry() */


/**************************************************************************
 Routine Description:
              This function is called by the OS-specific port driver after
              the necessary storage has been allocated, to gather information
              about the adapter's configuration.

 Arguments:
              HwDeviceExtension - HBA miniport driver's adapter data storage
              Context - Register base address
              ConfigInfo - Configuration information structure describing HBA
              This structure is defined in PORT.H.

 Return Value:
              ULONG
**************************************************************************/
ULONG
TulFindAdapter(
IN PVOID HwDeviceExtension,
IN PVOID Context,
IN PVOID BusInformation,
IN PCHAR ArgumentString,
IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
OUT PBOOLEAN Again
)

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
//    PULONG    AdapterNoPtr = Context;
    ULONG_PTR     portFound;
    static UCHAR k = 0;
    PACCESS_RANGE   accessRange;
    PCI_COMMON_CONFIG PCICommonConfig;
    ULONG_PTR   pciAddress;
    char    NumPort = 0;

    *Again = FALSE;

    accessRange = &((*(ConfigInfo->AccessRanges))[0]);

    if ( ScsiPortValidateRange(deviceExtension,
        ConfigInfo->AdapterInterfaceType,
        ConfigInfo->SystemIoBusNumber,
        accessRange->RangeStart,
        accessRange->RangeLength,
        TRUE) )  /* TRUE: iniospace */ {
        pciAddress = (ULONG_PTR) ScsiPortGetDeviceBase(deviceExtension,
            ConfigInfo->AdapterInterfaceType,
            ConfigInfo->SystemIoBusNumber,
            accessRange->RangeStart,
            accessRange->RangeLength,
            TRUE);  /* TRUE: iniospace */

        if ((TUL_RDLONG(pciAddress, TUL_PVID) != 0x95001101) &&
            (TUL_RDLONG(pciAddress, TUL_PVID) != 0x94001101) &&
            (TUL_RDLONG(pciAddress, TUL_PVID) != 0x0002134A) &&
            (TUL_RDLONG(pciAddress, TUL_PVID) != 0x94011101))
            return(SP_RETURN_NOT_FOUND);    /* 06/17/97     */
        memset(hcsp, 0, sizeof(HCS));
        hcsp->Base = pciAddress;
        hcsp->Intr = (UCHAR)ConfigInfo->BusInterruptLevel;
        NumPort++;

    }

    if (NumPort == 0) {
        return(SP_RETURN_NOT_FOUND);
    }

    /* Hardware found, let's find out hardware configuration
       and fill out ConfigInfo table for WinNT
    */
    ConfigInfo->NumberOfBuses = 1;
    ConfigInfo->MaximumTransferLength = MAX_TRANSFER_SIZE;
    ConfigInfo->NumberOfPhysicalBreaks = MAX_SG_DESCRIPTORS - 1;
#if SG_SUPPORT
    ConfigInfo->ScatterGather = TRUE;
#else
    ConfigInfo->ScatterGather = FALSE;
#endif
    ConfigInfo->Master = TRUE;
    ConfigInfo->NeedPhysicalAddresses = TRUE;
    ConfigInfo->Dma32BitAddresses = TRUE;
    ConfigInfo->InterruptMode =  LevelSensitive;

#if TAG_QUEUING
    ConfigInfo->TaggedQueuing = TRUE;
#else
    ConfigInfo->TaggedQueuing = FALSE;
#endif

    ConfigInfo->AlignmentMask = 0x0;     /* 10/21/97        */

    portFound =     hcsp->Base;
//    ((UCHAR) * AdapterNoPtr)++;

    if (init_tulip(hcsp))
        return(SP_RETURN_NOT_FOUND);

    if (hcsp->wide_scsi_card)
        ConfigInfo->MaximumNumberOfTargets = 16;
    else
        ConfigInfo->MaximumNumberOfTargets = 8;


    ConfigInfo->InitiatorBusId[0] = hcsp->HaId;
    if (NumPort != 0)
        *Again = TRUE;

    return(SP_RETURN_FOUND);

} /* end TulFindAdapter() */


/***********************************************************************
 Routine Description:
              This routine is called from ScsiPortInitialize
              to set up the adapter so that it is ready to service requests.
 Arguments:
              HwDeviceExtension - HBA miniport driver's adapter data storage
 Return Value:
              TRUE - if initialization successful.
              FALSE - if initialization unsuccessful.
***********************************************************************/
BOOLEAN TulHwInitialize(
IN PVOID HwDeviceExtension
)
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);

//    hwInitialized = 0;
    if (hcsp->Config & HCC_SCSI_RESET) {
        init_reset_scsi(hcsp);
    }
    AbortXPend(hcsp);

    /* Enable tulip interrupt */
    TUL_WR(hcsp->Base + TUL_SIntEnable, 0xFF);
//    hwInitialized = 1;

    return( TRUE );
} /* end TulHwInitialize() */


/***********************************************************************
 Routine Description:
              This routine is called from the SCSI port driver to send a
              command to controller or target.
 Arguments:
              HwDeviceExtension - HBA miniport driver's adapter data storage
              Srb - IO request packet
 Return Value:
              TRUE
***********************************************************************/
BOOLEAN
TulPutQueue(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK srb
)

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
    PTULSCB scbp = srb->SrbExtension;
    int    retStatus, k;
    PSCSI_REQUEST_BLOCK AbortSRB;

    switch (srb->Function) {
    case SRB_FUNCTION_ABORT_COMMAND:
        ScsiDebugPrint(0, "TulPutQueue:SRB_FUNCTION_ABORT_COMMAND\n");

        AbortSRB = ScsiPortGetSrb(HwDeviceExtension, srb->PathId, srb->TargetId, srb->Lun, srb->QueueTag);
        if(!AbortSRB) {
            /* treat as spurious */
            return TRUE;
        }
        if ((AbortSRB != srb->NextSrb) || (AbortSRB->SrbStatus != SRB_STATUS_PENDING)) {
            srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
            ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
            return TRUE;
        }
        switch (tul_abort_srb ( hcsp, srb->NextSrb )) {
        case 11:
            /*--- send ABORT/ABORT_WITH_TAG msg to target ----*/
            /* Build abort target TULSCB */
            BuildAbtScb(HwDeviceExtension, srb->NextSrb);
            scbp = srb->NextSrb->SrbExtension;

            /* Execute abort msg out Command  */
            hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun]++;
            tul_exec_scb(hcsp, scbp);
            while (!(scbp->Flags & SCF_DONE))
                ;
            if (--hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] < 0)
                hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] = 0;
        case 1:
            srb->NextSrb->SrbStatus = SRB_STATUS_ABORTED;
            ScsiPortNotification(RequestComplete, HwDeviceExtension, srb->NextSrb);
            srb->SrbStatus = SRB_STATUS_SUCCESS;
            ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
            if ((hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] <
                hcsp->Tcs[srb->TargetId].MaximumTags) &&
                (scbp->TagMsg != 0))
                ScsiPortNotification(NextLuRequest, HwDeviceExtension, srb->PathId, srb->TargetId, srb->Lun);
            else
                ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
            return TRUE;
        default:
            srb->SrbStatus = SRB_STATUS_ABORT_FAILED;
        }
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_RESET_BUS:
        /* Reset SCSI bus. */
        /* ScsiDebugPrint(0,"SRB_FUNCTION_RESET_BUS,srb=%x \n",srb); */
        TulResetBus (hcsp, 0L);
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_EXECUTE_SCSI:
        /* Build TULSCB. */

        BuildScb(HwDeviceExtension, srb);
        scbp = srb->SrbExtension;

        hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun]++;
        tul_exec_scb(hcsp, scbp);

        /* Tell Tul a SRB is available now. */

        if ((hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] <
            hcsp->Tcs[srb->TargetId].MaximumTags) &&
            (scbp->TagMsg != 0)) {
            ScsiPortNotification(NextLuRequest, HwDeviceExtension, srb->PathId, srb->TargetId, srb->Lun);
        } else
            ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;
    case SRB_FUNCTION_RESET_DEVICE:

        /* Build device reset TULSCB */
        BuildRstScb(HwDeviceExtension, srb);
        scbp = srb->SrbExtension;

        /* Execute device reset Command  */
        hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun]++;
        tul_exec_scb(hcsp, scbp);
        srb->SrbStatus = SRB_STATUS_SUCCESS;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        if ((hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] <
            hcsp->Tcs[srb->TargetId].MaximumTags) &&
            (scbp->TagMsg != 0)) {
            ScsiPortNotification(NextLuRequest, HwDeviceExtension, srb->PathId, srb->TargetId, srb->Lun);
        } else
            ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;

    case SRB_FUNCTION_CLAIM_DEVICE:
    case SRB_FUNCTION_IO_CONTROL:
    case SRB_FUNCTION_RECEIVE_EVENT:
    case SRB_FUNCTION_RELEASE_QUEUE:
    case SRB_FUNCTION_ATTACH_DEVICE:
    case SRB_FUNCTION_RELEASE_DEVICE:
    case SRB_FUNCTION_SHUTDOWN:
    case SRB_FUNCTION_FLUSH:
    case SRB_FUNCTION_RELEASE_RECOVERY:
    case SRB_FUNCTION_TERMINATE_IO:
    case SRB_FUNCTION_FLUSH_QUEUE:
    default:
        srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
        ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
        ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
        return TRUE;
    } /* end switch */

} /* end TulPutQueue() */


/***********************************************************************
 Routine Description:
              This is the interrupt service routine for the Tulmin SCSI adapter.
              It reads the interrupt register to determine if the adapter is indeed
              the source of the interrupt and clears the interrupt at the device.
 Arguments:
              HwDeviceExtension - HBA miniport driver's adapter data storage
 Return Value:
***********************************************************************/
BOOLEAN
TulInterrupt(
IN PVOID HwDeviceExtension
)
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);


    if (tul_isr ( hcsp ))
        return(TRUE);
    else
        return (FALSE);
} /* end TulInterrupt() */



/***********************************************************************
 Routine Description:
              Build TULSCB for Tulmin.
 Arguments:
              DeviceExtenson
              SRB
 Return Value:
              Nothing.
***********************************************************************/
VOID
BuildScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK srb
)

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
    PTULSCB  scbp = srb->SrbExtension;
    ULONG length, xferLength, remainLength;
    PVOID virtualAddress;
    UCHAR i;
    int    k;

    ScsiDebugPrint(1, "BuildSCB\n");

    scbp->Status = 0;
    scbp->NxtStat = 0;
    scbp->Mode = 0;
    scbp->SGIdx = 0;
    scbp->SGMax = 0;
    scbp->SGPAddr = 0;
    scbp->Post       = ScbPost;
    scbp->Srb        = srb;
    scbp->Opcode     = ExecSCSI;
    scbp->setATN = 0;


    /*-------------- setup scbp->Flags -----------------*/
    scbp->Flags             = SCF_POST ;

    switch (srb->SrbFlags & SRB_FLAGS_UNSPECIFIED_DIRECTION) {
    case SRB_FLAGS_UNSPECIFIED_DIRECTION:  /* let device decide direction */
        scbp->Flags             |=  SCF_NO_DCHK;
        break;
    case SRB_FLAGS_NO_DATA_TRANSFER:
        scbp->Flags             |=  SCF_NO_XF;
        break;
    case SRB_FLAGS_DATA_IN:
        scbp->Flags             |=  SCF_DIN;
        break;
    case SRB_FLAGS_DATA_OUT:
        scbp->Flags             |=  SCF_DOUT;
        break;
    default:
        break;
    }

    scbp->Target = srb->TargetId;
    scbp->Lun        = srb->Lun;

    scbp->HaStat     = 0;
    scbp->TaStat     = 0;
    scbp->CDBLen = (UCHAR)srb->CdbLength;

    for (i = 0; i < scbp->CDBLen; i++) {
        scbp->CDB[i] = srb->Cdb[i];
    }

    scbp->TagMsg = 0;
    if ( (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) |
        !(hcsp->Tcs[srb->TargetId].Flags & TCF_DISCONNECT) ) {
        scbp->Ident      = 0x80 | srb->Lun;
    } else
     {
        scbp->Ident      = 0x80 | DISC_ALLOW | srb->Lun;

        if (srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE ) {
            scbp->TagMsg = srb->QueueAction;
            scbp->TagId =  srb->QueueTag;
        }
    }
    scbp->NextScb = 0;
    scbp->NxtBufPtr = 0;
    scbp->NxtBufLen = 0;

    /* Build SG in TULSCB if data transfer. */
    i = 0;
    if (srb->DataTransferLength > 0) {
        scbp->BufLen = srb->DataTransferLength;
        virtualAddress =  srb->DataBuffer;

#if SG_SUPPORT
        xferLength = srb->DataTransferLength;
        remainLength = xferLength;

        /* Build scatter gather list  */
        do {
            scbp->SGList[i].Ptr =
                (ulong)ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(HwDeviceExtension,
                srb,
                virtualAddress,
                &length));
            if ( length > remainLength )
                length = remainLength;
            scbp->SGList[i].Len = length;

            virtualAddress = (PUCHAR) virtualAddress + length;
            if (length >= remainLength)
                remainLength = 0;
            else
                remainLength -= length;
            i++;
        } while ( remainLength > 0);
        if (i > 1) {                           /* 08/09/96 */
            scbp->Flags |= SCF_SG;
            scbp->SGLen = i;
            virtualAddress = (PVOID) & (scbp->SGList[0]);
            scbp->BufPtr =
                ScsiPortConvertPhysicalAddressToUlong(
                ScsiPortGetPhysicalAddress(HwDeviceExtension,
                0,
                (PVOID)virtualAddress,
                &length));
        } else /* Turn off SG */            {
            scbp->SGLen = 0;
            scbp->Flags &= ~SCF_SG;
            scbp->BufPtr = scbp->SGList[0].Ptr;
            scbp->BufLen = scbp->SGList[0].Len;
        }

#else /* #if !SG_SUPPORT */
        scbp->BufPtr =
            (ulong)ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(HwDeviceExtension,
            srb,
            (PVOID)  virtualAddress,
            &length));
#endif

    }


    /* Convert sense buffer length and buffer into physical address */
    if ((srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) ||
        (srb->SenseInfoBufferLength <= 0)) {
        scbp->Flags     &= ~SCF_SENSE;  /* Disable auto request sense   */
        scbp->SenseLen = 0;
    } else
     {                               /* Auto requense sense          */
        scbp->Flags     |= SCF_SENSE;   /* Enable auto request sense    */
        scbp->SenseLen  = (UBYTE) srb->SenseInfoBufferLength;
        /* Sense Buf physical addr      */
        scbp->SensePtr  =  ScsiPortConvertPhysicalAddressToUlong(
            ScsiPortGetPhysicalAddress(HwDeviceExtension,
            srb,
            srb->SenseInfoBuffer,
            &length));

        /* Sense buffer can not be scatter-gathered */
        if (srb->SenseInfoBufferLength > length ) {
            scbp->SenseLen = (UBYTE) length;
        }
    }

} /* end BuildScb() */


/***********************************************************************
 Routine Description:
              Build Bus_device_reset TULSCB for Tulmin.
 Arguments:
              DeviceExtenson
              SRB
 Return Value:
              Nothing.
**********************************************************************/
VOID
BuildRstScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK srb
)

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
    PTULSCB  scbp = srb->SrbExtension;
    UCHAR i;
    int    k;

    scbp->Status =  0;
    scbp->NxtStat =         0;
    scbp->Mode =    0;
    scbp->SGIdx =   0;
    scbp->SGMax =   0;
    scbp->SGPAddr =         0;
    scbp->Post       = ScbPost;
    scbp->Srb        = srb;
    scbp->Opcode     = BusDevRst;
    scbp->Flags      = SCF_POST;
    scbp->Target    = srb->TargetId;
    scbp->Lun        = srb->Lun;

    scbp->BufPtr =          0;
    scbp->BufLen =          0;
    scbp->HaStat =  0;
    scbp->TaStat =  0;
    scbp->CDBLen =   (UCHAR)srb->CdbLength;
    for (i = 0; i < scbp->CDBLen; i++)
        scbp->CDB[i] = srb->Cdb[i];
    scbp->Ident      = 0x80 | srb->Lun;
    scbp->NextScb =           0;
    scbp->NxtBufPtr =  0;
    scbp->NxtBufLen =  0;

} /* end BuildRstdScb() */


/***********************************************************************
 Routine Description:
              Build abort target TULSCB for Tulmin.
 Arguments:
              DeviceExtenson
              SRB
 Return Value:
              Nothing.
**********************************************************************/
VOID
BuildAbtScb(
IN PVOID HwDeviceExtension,
IN PSCSI_REQUEST_BLOCK srb
)

{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
    PTULSCB  scbp = srb->SrbExtension;
    UCHAR i;
    int    k;

    scbp->Status =          0;
    scbp->NxtStat =         0;
    scbp->Mode =            0;
    scbp->SGIdx =           0;
    scbp->SGMax =           0;
    scbp->SGPAddr =         0;
    scbp->Post       = ScbPost;
    scbp->Srb        = srb;
    scbp->Opcode     = AbortCmd;
    scbp->Flags      = SCF_POST | SCF_POLL;
    scbp->Target     = srb->TargetId;
    scbp->Lun        = srb->Lun;

    scbp->BufPtr =   0;
    scbp->BufLen =   0;
    scbp->HaStat =   0;
    scbp->TaStat =   0;
    scbp->CDBLen =   (UCHAR)srb->CdbLength;
    for (i = 0; i < scbp->CDBLen; i++)
        scbp->CDB[i] = srb->Cdb[i];
    scbp->Ident      = 0x80 | srb->Lun;
    scbp->NextScb =           0;
    scbp->NxtBufPtr =  0;
    scbp->NxtBufLen =  0;

    scbp->TagMsg = 0;
    if ( (srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) |
        !(hcsp->Tcs[srb->TargetId].Flags & TCF_DISCONNECT) ) {
        scbp->Ident      = 0x80 | srb->Lun;
    } else
     {
        scbp->Ident      = 0x80 | DISC_ALLOW | srb->Lun;

        if (srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE ) {
            scbp->TagMsg = srb->QueueAction;
            scbp->TagId =  srb->QueueTag;
        }
    }

} /* end BuildAbtdScb() */

#ifndef WIN95

SCSI_ADAPTER_CONTROL_STATUS
TulAdapterControl(
    IN PVOID HwDeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);
    PSCSI_SUPPORTED_CONTROL_TYPE_LIST pControlTypeList;


    switch(ControlType) {
        case ScsiQuerySupportedControlTypes: {
            BOOLEAN supportedTypes[ScsiAdapterControlMax] = {
                TRUE,       // ScsiQuerySupportedControlTypes
                TRUE,       // ScsiStopAdapter
                TRUE,       // ScsiRestartAdapter
                FALSE,      // ScsiSetBootConfig
                FALSE       // ScsiSetRunningConfig
            };

            ULONG max = ScsiAdapterControlMax;
            ULONG i;

            pControlTypeList = (PSCSI_SUPPORTED_CONTROL_TYPE_LIST) Parameters;

            if(pControlTypeList->MaxControlType < max) {
                max = pControlTypeList->MaxControlType;
            }

            for(i = 0; i < max; i++) {
                pControlTypeList->SupportedTypeList[i] = supportedTypes[i];
            }


            break;

        }
        case ScsiStopAdapter: {
            //
            // Shut down all interrupts on the adapter.  They'll get re-enabled
            // by the initialization routines.
            //
            TUL_WR(hcsp->Base + TUL_SIntEnable, 0x00);
            tul_dis_sint(hcsp);


            break;
        }

        case ScsiRestartAdapter: {
            //
            // Enable all the interrupts on the adapter while port driver call
            // for power up an HBA that was shut down for power management
            //
            init_tulip(hcsp);
            TUL_WR(hcsp->Base + TUL_SIntEnable, 0xFF);
            tul_en_sint(hcsp);

            break;
        }

        default: {
            return ScsiAdapterControlUnsuccessful;
        }
    }

    return ScsiAdapterControlSuccess;
}

#endif

/***********************************************************************s
 Routine Description:
              Reset SCSI bus.
 Arguments:
              HwDeviceExtension - HBA miniport driver's adapter data storage
 Return Value:
              Nothing.
***********************************************************************/
BOOLEAN
TulResetBus(
IN PVOID HwDeviceExtension,
IN ULONG PathId
)
{
    PHW_DEVICE_EXTENSION deviceExtension = HwDeviceExtension;
    PHCS hcsp = &(deviceExtension->hcs);

    reset_scsi(hcsp);
    ScsiPortCompleteRequest(HwDeviceExtension,
        (UCHAR) PathId,
        0xFF,
        0xFF,
        (UCHAR) SRB_STATUS_BUS_RESET);
    return TRUE;

} /* end TulResetBus() */


/***********************************************************************
 Routine Description:
              Callback routine after SCSI done
 Arguments:
              hcsp - Pointer to chip configuration structure
              scbp - Pointer to a structure contains information about
                                the scbp just completed
 Return Value:
************************************************************************/
ScbPost(
HCS *hcsp, TULSCB *scbp
)
{
    PHW_DEVICE_EXTENSION HwDeviceExtension = (PHW_DEVICE_EXTENSION) hcsp;
    PSCSI_REQUEST_BLOCK srb = (PSCSI_REQUEST_BLOCK) scbp->Srb;

    if (( srb = scbp->Srb) == 0)     {
        return 0;
    }
    srb->ScsiStatus = scbp->TaStat;
    if ((srb->ScsiStatus != 0) && (scbp->HaStat == 0x00)) {                               /* Set ScsiStatus for SRB       */
        if (scbp->TaStat == TARGET_CHECK_CONDITION) {
            srb->SrbStatus = SRB_STATUS_ERROR;
            if (scbp->Flags & SCF_SENSE)
                srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
        } else
         {
            srb->DataTransferLength = 0;
            if (scbp->TaStat == TARGET_BUSY)    /* status busy      */ {
                srb->SrbStatus = SRB_STATUS_BUSY;       /* 08/30/96 */
            } else if (scbp->TaStat == TARGET_TAG_FULL) {
                if ((hcsp->Tcs[srb->TargetId].MaximumTags =
                    hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] - 1)
                     < 1)
                    hcsp->Tcs[srb->TargetId].MaximumTags = 1;

                srb->SrbStatus = SRB_STATUS_BUSY;       /* 08/07/97 */
            } else
             {
                srb->SrbStatus = SRB_STATUS_ERROR;
            }
        }
    } else
     { /* Scsi Status is ok, but host status may be not */
        /*      srb->ScsiStatus = 0;*/
        srb->SrbStatus = TulErrXlate(scbp->HaStat);
    }

    if (srb != NULL)                    /* 07/31/97     */ {
        if (--hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] < 0)
            hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] = 0;
        if ((hcsp->Tcs[srb->TargetId].RequestCount[srb->Lun] <
            hcsp->Tcs[srb->TargetId].MaximumTags) &&
            (scbp->TagMsg != 0)) {
            ScsiPortNotification(NextLuRequest, HwDeviceExtension, srb->PathId, srb->TargetId, srb->Lun);
        } else
            ScsiPortNotification(NextRequest, HwDeviceExtension, NULL);
    }
    /* Tell NT that srb has been completed   */
    ScsiPortNotification(RequestComplete, HwDeviceExtension, srb);
    return 1;
}


/***********************************************************************
 Routine Description:
              This routine translate Tul host status into SrbStatus which
              is requried by NT
 Arguments:
              TulErrCode - Error code defined by Tul host_stat
 Return Value:
              Error code defined by NT SCSI port driver
***********************************************************************/
UCHAR
TulErrXlate (
UCHAR scbHaStat
)

{
    switch (scbHaStat) {
    case 0:
        return ( SRB_STATUS_SUCCESS                  );

    case HOST_SEL_TOUT:
        return ( SRB_STATUS_SELECTION_TIMEOUT );

    case HOST_BUS_FREE:  /* unexpected bus free */
        return ( SRB_STATUS_UNEXPECTED_BUS_FREE      );

    case HOST_BAD_PHAS:
        return ( SRB_STATUS_PHASE_SEQUENCE_FAILURE );

    case HOST_DO_DU:
        return ( SRB_STATUS_DATA_OVERRUN             );

    case HOST_INV_CMD:
        return ( SRB_STATUS_INVALID_REQUEST          );

    case HOST_SCSI_RST:
        return ( SRB_STATUS_BUS_RESET                );

    case HOST_DEV_RST:
        return ( SRB_STATUS_UNEXPECTED_BUS_FREE      );

    case HOST_CANNOT_ALLOC_SCB:
        return ( SRB_STATUS_BUSY                     );

    default:
        return ( SRB_STATUS_TIMEOUT );
    }
}


/***************************************************************************/
void ll_append_pend_scb(HCS *hcsp, TULSCB *scbp)
{
    scbp->Status = TULSCB_PEND;
    scbp->NextScb = NULL;
    if (hcsp->LastPend != NULL) {
        hcsp->LastPend->NextScb = scbp;
        hcsp->LastPend = scbp;
    } else
     {
        hcsp->FirstPend = scbp;
        hcsp->LastPend = scbp;
    }
}


/***************************************************************************/
void ll_push_pend_scb(HCS *hcsp, TULSCB *scbp)
{

    scbp->Status = TULSCB_PEND;
    if ((scbp->NextScb = (hcsp->FirstPend)) != NULL) {
        hcsp->FirstPend = scbp;
    } else
     {
        hcsp->FirstPend = scbp;
        hcsp->LastPend = scbp;
    }
}


/***************************************************************************/
TULSCB *ll_pop_pend_scb(HCS *hcsp)
{
    TULSCB         * pTmpScb;

    if ((pTmpScb = hcsp->FirstPend) != NULL) {
        if ((hcsp->FirstPend = (PTULSCB)(pTmpScb->NextScb)) == NULL)
            hcsp->LastPend = NULL;
        pTmpScb->NextScb = NULL;
    }
    return (pTmpScb);
}


/***************************************************************************/
void ll_unlink_pend_scb(HCS *hcsp, TULSCB *scbp)
{
    TULSCB         * pTmpScb, *pPrevScb;


    pPrevScb = pTmpScb = hcsp->FirstPend;
    while (pTmpScb != NULL) {
        if (scbp == pTmpScb) {     /* Unlink this SCB              */
            if (pTmpScb == hcsp->FirstPend) {
                if ((hcsp->FirstPend = (pTmpScb->NextScb)) == NULL)
                    hcsp->LastPend = NULL;
            } else
             {
                pPrevScb->NextScb = pTmpScb->NextScb;
                if (pTmpScb == hcsp->LastPend)
                    hcsp->LastPend = pPrevScb;
            }
            pTmpScb->NextScb = NULL;
            break;
        }
        pPrevScb = pTmpScb;
        pTmpScb = (pTmpScb->NextScb);
    }

}


/***************************************************************************/
void ll_append_busy_scb(HCS *hcsp, TULSCB *scbp)
{

    scbp->Status = TULSCB_BUSY;
    scbp->NextScb = NULL;
    if (hcsp->LastBusy != NULL) {
        hcsp->LastBusy->NextScb = scbp;
        hcsp->LastBusy = scbp;
    } else
     {
        hcsp->FirstBusy = scbp;
        hcsp->LastBusy = scbp;
    }
}


/***************************************************************************/
TULSCB *ll_pop_busy_scb(HCS *hcsp)
{
    TULSCB         * pTmpScb;


    if ((pTmpScb = hcsp->FirstBusy) != NULL) {
        if ((hcsp->FirstBusy = pTmpScb->NextScb) == NULL)
            hcsp->LastBusy = NULL;
        pTmpScb->NextScb = NULL;
    }
    return (pTmpScb);
}


/***************************************************************************/
void ll_unlink_busy_scb(HCS *hcsp, TULSCB *scbp)
{
    TULSCB         * pTmpScb, *pPrevScb;


    pPrevScb = pTmpScb = hcsp->FirstBusy;
    while (pTmpScb != NULL) {
        if (scbp == pTmpScb) {                           /* Unlink this SCB              */
            if (pTmpScb == hcsp->FirstBusy) {
                if ((hcsp->FirstBusy = (pTmpScb->NextScb)) == NULL)
                    hcsp->LastBusy = NULL;
            } else
             {
                pPrevScb->NextScb = pTmpScb->NextScb;
                if (pTmpScb == hcsp->LastBusy)
                    hcsp->LastBusy = pPrevScb;
            }
            pTmpScb->NextScb = NULL;
            break;
        }
        pPrevScb = pTmpScb;
        pTmpScb = (pTmpScb->NextScb);
    }

}


/***************************************************************************/
TULSCB *ll_find_busy_scb(HCS *hcsp, WORD tarlun)
{
    TULSCB         * pTmpScb;
    WORD        scbp_tarlun;

    pTmpScb = hcsp->FirstBusy;
    while (pTmpScb != NULL) {
        scbp_tarlun = (pTmpScb->Lun << 8) | (pTmpScb->Target);
        if (scbp_tarlun == tarlun) {
            break;
        }
        pTmpScb = pTmpScb->NextScb;
    }
    return (pTmpScb);
}



#if TULDBG
void dbgtagid()
{
}


#endif

/***************************************************************************/
TULSCB *ll_find_tagid_busy_scb(HCS *hcsp, WORD tarlun, UBYTE tagid )
{
    TULSCB         * pTmpScb;
    WORD        scbp_tarlun;

    pTmpScb = hcsp->FirstBusy;
    while (pTmpScb != NULL) {
        scbp_tarlun = (pTmpScb->Lun << 8) | (pTmpScb->Target);
        if (scbp_tarlun == tarlun) {
            if ( pTmpScb->TagId == tagid)
                break;
#if TULDBG
            else {
                dbgtagid();
            }
#endif
        }
        pTmpScb = (PTULSCB)(pTmpScb->NextScb);
    }
    return (pTmpScb);
}


/***************************************************************************/
void ll_append_done_scb(HCS *hcsp, TULSCB *scbp)
{


    scbp->Status = TULSCB_DONE;
    scbp->NextScb = NULL;
    if (hcsp->LastDone != NULL) {
        hcsp->LastDone->NextScb = scbp;
        hcsp->LastDone = scbp;
    } else
     {
        hcsp->FirstDone = scbp;
        hcsp->LastDone = scbp;
    }
}


/***************************************************************************/
TULSCB *ll_pop_done_scb(HCS *hcsp)
{
    TULSCB         * pTmpScb;


    if ((pTmpScb = hcsp->FirstDone) != NULL) {
        if ((hcsp->FirstDone = (PTULSCB)(pTmpScb->NextScb)) == (PTULSCB)NULL)
            hcsp->LastDone = NULL;
        pTmpScb->NextScb = NULL;
    }
    return (pTmpScb);
}

