#ifndef _aha154dt_h_
#define _aha154dt_h_

// AHA154xSetupDataGuid - AHA154SetupData
// AHA154x Setup Data Description (Operation Code 0Dh)
#define Aha154xWmi_SetupData_Guid \
    { 0xea992010,0xb75b,0x11d0, { 0xa3,0x07,0x00,0xaa,0x00,0x6c,0x3f,0x30 } }

#if ! (defined(MIDL_PASS))
DEFINE_GUID(AHA154xSetupDataGuid_GUID, \
            0xea992010,0xb75b,0x11d0,0xa3,0x07,0x00,0xaa,0x00,0x6c,0x3f,0x30);
#endif


typedef struct _AHA154SetupData
{
    // 
    UCHAR SdtAndParityStatus;
    #define AHA154SetupData_SdtAndParityStatus_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SdtAndParityStatus_ID 1

    // 
    UCHAR TransferSpeed;
    #define AHA154SetupData_TransferSpeed_SIZE sizeof(UCHAR)
    #define AHA154SetupData_TransferSpeed_ID 2

    // 
    UCHAR BusOnTime;
    #define AHA154SetupData_BusOnTime_SIZE sizeof(UCHAR)
    #define AHA154SetupData_BusOnTime_ID 3

    // 
    UCHAR BusOffTime;
    #define AHA154SetupData_BusOffTime_SIZE sizeof(UCHAR)
    #define AHA154SetupData_BusOffTime_ID 4

    // 
    UCHAR NumberOfMailboxes;
    #define AHA154SetupData_NumberOfMailboxes_SIZE sizeof(UCHAR)
    #define AHA154SetupData_NumberOfMailboxes_ID 5

    // 
    UCHAR MailboxAddressMsb;
    #define AHA154SetupData_MailboxAddressMsb_SIZE sizeof(UCHAR)
    #define AHA154SetupData_MailboxAddressMsb_ID 6

    // 
    UCHAR MailboxAddressMid;
    #define AHA154SetupData_MailboxAddressMid_SIZE sizeof(UCHAR)
    #define AHA154SetupData_MailboxAddressMid_ID 7

    // 
    UCHAR MailboxAddressLsb;
    #define AHA154SetupData_MailboxAddressLsb_SIZE sizeof(UCHAR)
    #define AHA154SetupData_MailboxAddressLsb_ID 8

    // 
    UCHAR SyncTransferAgreements0;
    #define AHA154SetupData_SyncTransferAgreements0_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements0_ID 9

    // 
    UCHAR SyncTransferAgreements1;
    #define AHA154SetupData_SyncTransferAgreements1_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements1_ID 10

    // 
    UCHAR SyncTransferAgreements2;
    #define AHA154SetupData_SyncTransferAgreements2_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements2_ID 11

    // 
    UCHAR SyncTransferAgreements3;
    #define AHA154SetupData_SyncTransferAgreements3_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements3_ID 12

    // 
    UCHAR SyncTransferAgreements4;
    #define AHA154SetupData_SyncTransferAgreements4_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements4_ID 13

    // 
    UCHAR SyncTransferAgreements5;
    #define AHA154SetupData_SyncTransferAgreements5_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements5_ID 14

    // 
    UCHAR SyncTransferAgreements6;
    #define AHA154SetupData_SyncTransferAgreements6_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements6_ID 15

    // 
    UCHAR SyncTransferAgreements7;
    #define AHA154SetupData_SyncTransferAgreements7_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SyncTransferAgreements7_ID 16

    // 
    UCHAR DisconnectionOption;
    #define AHA154SetupData_DisconnectionOption_SIZE sizeof(UCHAR)
    #define AHA154SetupData_DisconnectionOption_ID 17

    // 
    CHAR CustomerBanner[20];
    #define AHA154SetupData_CustomerBanner_SIZE sizeof(CHAR[20])
    #define AHA154SetupData_CustomerBanner_ID 18

    // 
    UCHAR AutoRetryOption;
    #define AHA154SetupData_AutoRetryOption_SIZE sizeof(UCHAR)
    #define AHA154SetupData_AutoRetryOption_ID 19

    // 
    UCHAR SwitchesOnBoard;
    #define AHA154SetupData_SwitchesOnBoard_SIZE sizeof(UCHAR)
    #define AHA154SetupData_SwitchesOnBoard_ID 20

    // 
    USHORT FirmwareChecksum;
    #define AHA154SetupData_FirmwareChecksum_SIZE sizeof(USHORT)
    #define AHA154SetupData_FirmwareChecksum_ID 21

    // 
    UCHAR BiosMailboxAddressMsb;
    #define AHA154SetupData_BiosMailboxAddressMsb_SIZE sizeof(UCHAR)
    #define AHA154SetupData_BiosMailboxAddressMsb_ID 22

    // 
    UCHAR BiosMailboxAddressMid;
    #define AHA154SetupData_BiosMailboxAddressMid_SIZE sizeof(UCHAR)
    #define AHA154SetupData_BiosMailboxAddressMid_ID 23

    // 
    UCHAR BiosMailboxAddressLsb;
    #define AHA154SetupData_BiosMailboxAddressLsb_SIZE sizeof(UCHAR)
    #define AHA154SetupData_BiosMailboxAddressLsb_ID 24

    // 
    UCHAR Reserved[211];
    #define AHA154SetupData_Reserved_SIZE sizeof(UCHAR[211])
    #define AHA154SetupData_Reserved_ID 25

} AHA154SetupData, *PAHA154SetupData;

#define AHA154SetupData_SIZE (FIELD_OFFSET(AHA154SetupData, Reserved) + AHA154SetupData_Reserved_SIZE)

#endif

