//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
// If you do not agree to the terms, do not use the code.
//


#ifndef _MSAUDITE_
#define _MSAUDITE_

  
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


//
// Define the severity codes
//


//
// MessageId: 0x00000000L (No symbolic name defined)
//
// MessageText:
//
//  Unused message ID
//






#define SE_ADT_MIN_CATEGORY_ID   1      
#define SE_ADT_MAX_CATEGORY_ID   9      


#define SE_ADT_MIN_AUDIT_ID      0x200  
#define SE_ADT_MAX_AUDIT_ID      0x5ff  





























//
// MessageId: SE_ADT_LAST_SYSTEM_MESSAGE
//
// MessageText:
//
//  Highest System-Defined Audit Message Value.
//
#define SE_ADT_LAST_SYSTEM_MESSAGE       ((ULONG)0x00000FFFL)























//
// MessageId: SE_CATEGID_SYSTEM
//
// MessageText:
//
//  System Event
//
#define SE_CATEGID_SYSTEM                ((ULONG)0x00000001L)

//
// MessageId: SE_CATEGID_LOGON
//
// MessageText:
//
//  Logon/Logoff
//
#define SE_CATEGID_LOGON                 ((ULONG)0x00000002L)

//
// MessageId: SE_CATEGID_OBJECT_ACCESS
//
// MessageText:
//
//  Object Access
//
#define SE_CATEGID_OBJECT_ACCESS         ((ULONG)0x00000003L)

//
// MessageId: SE_CATEGID_PRIVILEGE_USE
//
// MessageText:
//
//  Privilege Use
//
#define SE_CATEGID_PRIVILEGE_USE         ((ULONG)0x00000004L)

//
// MessageId: SE_CATEGID_DETAILED_TRACKING
//
// MessageText:
//
//  Detailed Tracking
//
#define SE_CATEGID_DETAILED_TRACKING     ((ULONG)0x00000005L)

//
// MessageId: SE_CATEGID_POLICY_CHANGE
//
// MessageText:
//
//  Policy Change
//
#define SE_CATEGID_POLICY_CHANGE         ((ULONG)0x00000006L)

//
// MessageId: SE_CATEGID_ACCOUNT_MANAGEMENT
//
// MessageText:
//
//  Account Management
//
#define SE_CATEGID_ACCOUNT_MANAGEMENT    ((ULONG)0x00000007L)

//
// MessageId: SE_CATEGID_DS_ACCESS
//
// MessageText:
//
//  Directory Service Access
//
#define SE_CATEGID_DS_ACCESS             ((ULONG)0x00000008L)

//
// MessageId: SE_CATEGID_ACCOUNT_LOGON
//
// MessageText:
//
//  Account Logon
//
#define SE_CATEGID_ACCOUNT_LOGON         ((ULONG)0x00000009L)






























//
// MessageId: SE_AUDITID_SYSTEM_RESTART
//
// MessageText:
//
//  Windows is starting up.
//
#define SE_AUDITID_SYSTEM_RESTART        ((ULONG)0x00000200L)











//
// MessageId: SE_AUDITID_SYSTEM_SHUTDOWN
//
// MessageText:
//
//  Windows is shutting down.
//  All logon sessions will be terminated by this shutdown.
//
#define SE_AUDITID_SYSTEM_SHUTDOWN       ((ULONG)0x00000201L)













//
// MessageId: SE_AUDITID_AUTH_PACKAGE_LOAD
//
// MessageText:
//
//  An authentication package has been loaded by the Local Security Authority.
//  This authentication package will be used to authenticate logon attempts.
//  %n
//  Authentication Package Name:%t%1
//
#define SE_AUDITID_AUTH_PACKAGE_LOAD     ((ULONG)0x00000202L)













//
// MessageId: SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER
//
// MessageText:
//
//  A trusted logon process has registered with the Local Security Authority.
//  This logon process will be trusted to submit logon requests.
//  %n
//  %n
//  Logon Process Name:%t%1
//
#define SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER ((ULONG)0x00000203L)













//
// MessageId: SE_AUDITID_AUDITS_DISCARDED
//
// MessageText:
//
//  Internal resources allocated for the queuing of audit messages have been exhausted,
//  leading to the loss of some audits.
//  %n
//  %tNumber of audit messages discarded:%t%1
//
#define SE_AUDITID_AUDITS_DISCARDED      ((ULONG)0x00000204L)























//
// MessageId: SE_AUDITID_AUDIT_LOG_CLEARED
//
// MessageText:
//
//  The audit log was cleared
//  %n
//  %tPrimary User Name:%t%1%n
//  %tPrimary Domain:%t%2%n
//  %tPrimary Logon ID:%t%3%n
//  %tClient User Name:%t%4%n
//  %tClient Domain:%t%5%n
//  %tClient Logon ID:%t%6%n
//
#define SE_AUDITID_AUDIT_LOG_CLEARED     ((ULONG)0x00000205L)













//
// MessageId: SE_AUDITID_NOTIFY_PACKAGE_LOAD
//
// MessageText:
//
//  An notification package has been loaded by the Security Account Manager.
//  This package will be notified of any account or password changes.
//  %n
//  Notification Package Name:%t%1
//
#define SE_AUDITID_NOTIFY_PACKAGE_LOAD   ((ULONG)0x00000206L)





















//
// MessageId: SE_AUDITID_LPC_INVALID_USE
//
// MessageText:
//
//  Invalid use of LPC port.%n
//  %tProcess ID: %1%n
//  %tImage File Name: %2%n
//  %tPrimary User Name:%t%3%n
//  %tPrimary Domain:%t%4%n
//  %tPrimary Logon ID:%t%5%n
//  %tClient User Name:%t%6%n
//  %tClient Domain:%t%7%n
//  %tClient Logon ID:%t%8%n
//  %tInvalid use: %9%n
//  %tServer Port Name:%t%10%n
//
#define SE_AUDITID_LPC_INVALID_USE       ((ULONG)0x00000207L)


















//
// MessageId: SE_AUDITID_SYSTEM_TIME_CHANGE
//
// MessageText:
//
//  The system time was changed.%n
//  Process ID:%t%t%1%n
//  Process Name:%t%t%2%n
//  Primary User Name:%t%3%n
//  Primary Domain:%t%t%4%n
//  Primary Logon ID:%t%t%5%n
//  Client User Name:%t%t%6%n
//  Client Domain:%t%t%7%n
//  Client Logon ID:%t%t%8%n
//  Previous Time:%t%t%10 %9%n
//  New Time:%t%t%12 %11%n
//
#define SE_AUDITID_SYSTEM_TIME_CHANGE    ((ULONG)0x00000208L)























//
// MessageId: SE_AUDITID_UNABLE_TO_LOG_EVENTS
//
// MessageText:
//
//  Unable to log events to security log:%n
//  %tStatus code:%t%t%1%n
//  %tValue of CrashOnAuditFail:%t%2%n
//
#define SE_AUDITID_UNABLE_TO_LOG_EVENTS  ((ULONG)0x00000209L)


















//
// MessageId: SE_AUDITID_AUDIT_COLLECTION_AGENT_ERROR
//
// MessageText:
//
//  The audit collection system has encountered an error.%n
//  %tComponent:%t%1%n
//  %tVersion:%t%2%n
//  %tStatus code:%t%3%n
//
#define SE_AUDITID_AUDIT_COLLECTION_AGENT_ERROR ((ULONG)0x0000020AL)
















//
// MessageId: SE_AUDITID_SECURITY_LOG_EXCEEDS_WARNING_LEVEL
//
// MessageText:
//
//  The security log is now %1 percent full.
//
#define SE_AUDITID_SECURITY_LOG_EXCEEDS_WARNING_LEVEL ((ULONG)0x0000020BL)


















//
// MessageId: SE_AUDITID_EVENT_LOG_AUTOBACKUP
//
// MessageText:
//
//  Event log auto-backup%n
//  %tLog:%t%1%n
//  %tFile:%t%2%n
//  %tStatus:%t%3%n
//
#define SE_AUDITID_EVENT_LOG_AUTOBACKUP  ((ULONG)0x0000020CL)
















//
// MessageId: SE_AUDITID_IPSEC_INBOUND_PACKET_INTEGRITY_CHECK_FAIL
//
// MessageText:
//
//  
//  IPSec inbound packet integrity check failed:%n
//  %tPacket Source:%t%1%n
//  %tInbound SA:%t%2%n
//  %tNumber Of Packets:%t%3%n
//  Received packet from over a security association that failed data integrity verification. This could be a temporary problem; if it persists it may indicate either a poor network condition or that packets are being modified in transit to the system.%n
//
#define SE_AUDITID_IPSEC_INBOUND_PACKET_INTEGRITY_CHECK_FAIL ((ULONG)0x00000360L)
















//
// MessageId: SE_AUDITID_IPSEC_INBOUND_PACKET_REPLAY_CHECK_FAIL
//
// MessageText:
//
//  
//  IPSec inbound packet replay check failed:%n
//  %tPacket Source:%t%1%n
//  %tInbound SA:%t%2%n 
//  %tNumber of Packets:%t%3%n
//  Received packet from over a security association with a sequence number for a packet already processed by the system.This could be a temporary problem; if it persists it may indicate a replay attack against the system.%n
//
#define SE_AUDITID_IPSEC_INBOUND_PACKET_REPLAY_CHECK_FAIL ((ULONG)0x00000361L)
















//
// MessageId: SE_AUDITID_IPSEC_INBOUND_PACKET_REPLAY_CHECK_FAIL_NON_DEFINITE
//
// MessageText:
//
//  
//  IPSec inbound packet replay check failed, inbound packet had too low a sequence number to ensure it was not a replay:%n
//  %tPacket Source:%t%1%n
//  %tInbound SA:%t%2%n 
//  %tNumber of Packets:%t%3%n
//
#define SE_AUDITID_IPSEC_INBOUND_PACKET_REPLAY_CHECK_FAIL_NON_DEFINITE ((ULONG)0x00000362L)
















//
// MessageId: SE_AUDITID_IPSEC_RECEIVED_INBOUND_CLEAR_TEXT
//
// MessageText:
//
//  
//  IPSec received inbound clear text packet that should have been secured:%n
//  %tPacket Source:%t%1%n
//  %tInbound SA:%t%2%n
//  %tNumber of Packets:%t%3%n
//
#define SE_AUDITID_IPSEC_RECEIVED_INBOUND_CLEAR_TEXT ((ULONG)0x00000363L)


























































//
// MessageId: SE_AUDITID_SUCCESSFUL_LOGON
//
// MessageText:
//
//  Successful Logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//  %tLogon Process:%t%5%n
//  %tAuthentication Package:%t%6%n
//  %tWorkstation Name:%t%7%n
//  %tLogon GUID:%t%8%n
//  %tCaller User Name:%t%9%n
//  %tCaller Domain:%t%10%n
//  %tCaller Logon ID:%t%11%n
//  %tCaller Process ID: %12%n
//  %tTransited Services: %13%n
//  %tSource Network Address:%t%14%n
//  %tSource Port:%t%15%n
//
#define SE_AUDITID_SUCCESSFUL_LOGON      ((ULONG)0x00000210L)




















//
// MessageId: SE_AUDITID_UNKNOWN_USER_OR_PWD
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tUnknown user name or bad password%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_UNKNOWN_USER_OR_PWD   ((ULONG)0x00000211L)




















//
// MessageId: SE_AUDITID_ACCOUNT_TIME_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount logon time restriction violation%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_ACCOUNT_TIME_RESTR    ((ULONG)0x00000212L)




















//
// MessageId: SE_AUDITID_ACCOUNT_DISABLED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount currently disabled%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_ACCOUNT_DISABLED      ((ULONG)0x00000213L)




















//
// MessageId: SE_AUDITID_ACCOUNT_EXPIRED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe specified user account has expired%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_ACCOUNT_EXPIRED       ((ULONG)0x00000214L)




















//
// MessageId: SE_AUDITID_WORKSTATION_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tUser not allowed to logon at this computer%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_WORKSTATION_RESTR     ((ULONG)0x00000215L)




















//
// MessageId: SE_AUDITID_LOGON_TYPE_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%tThe user has not been granted the requested%n
//  %t%tlogon type at this machine%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_LOGON_TYPE_RESTR      ((ULONG)0x00000216L)




















//
// MessageId: SE_AUDITID_PASSWORD_EXPIRED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe specified account's password has expired%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_PASSWORD_EXPIRED      ((ULONG)0x00000217L)




















//
// MessageId: SE_AUDITID_NETLOGON_NOT_STARTED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe NetLogon component is not active%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID:%t%10%n
//  %tTransited Services:%t%11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_NETLOGON_NOT_STARTED  ((ULONG)0x00000218L)




















//
// MessageId: SE_AUDITID_UNSUCCESSFUL_LOGON
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAn error occurred during logon%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tStatus code:%t%7%n
//  %tSubstatus code:%t%8%n
//  %tCaller User Name:%t%9%n
//  %tCaller Domain:%t%10%n
//  %tCaller Logon ID:%t%11%n
//  %tCaller Process ID:%t%12%n
//  %tTransited Services:%t%13%n
//  %tSource Network Address:%t%14%n
//  %tSource Port:%t%15%n
//
#define SE_AUDITID_UNSUCCESSFUL_LOGON    ((ULONG)0x00000219L)


































//
// MessageId: SE_AUDITID_LOGOFF
//
// MessageText:
//
//  User Logoff:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//
#define SE_AUDITID_LOGOFF                ((ULONG)0x0000021AL)




















//
// MessageId: SE_AUDITID_ACCOUNT_LOCKED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount locked out%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID: %10%n
//  %tTransited Services: %11%n
//  %tSource Network Address:%t%12%n
//  %tSource Port:%t%13%n
//
#define SE_AUDITID_ACCOUNT_LOCKED        ((ULONG)0x0000021BL)

































//
// MessageId: SE_AUDITID_NETWORK_LOGON
//
// MessageText:
//
//  Successful Network Logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//  %tLogon Process:%t%5%n
//  %tAuthentication Package:%t%6%n
//  %tWorkstation Name:%t%7%n
//  %tLogon GUID:%t%8%n
//  %tCaller User Name:%t%9%n
//  %tCaller Domain:%t%10%n
//  %tCaller Logon ID:%t%11%n
//  %tCaller Process ID: %12%n
//  %tTransited Services: %13%n
//  %tSource Network Address:%t%14%n
//  %tSource Port:%t%15%n
//
#define SE_AUDITID_NETWORK_LOGON         ((ULONG)0x0000021CL)


















//
// MessageId: SE_AUDITID_IPSEC_LOGON_SUCCESS
//
// MessageText:
//
//  IKE security association established.%n
//  Mode: %n%1%n
//  Peer Identity: %n%2%n
//  Filter: %n%3%n
//  Parameters: %n%4%n
//
#define SE_AUDITID_IPSEC_LOGON_SUCCESS   ((ULONG)0x0000021DL)
















//
// MessageId: SE_AUDITID_IPSEC_LOGOFF_QM
//
// MessageText:
//
//  IKE security association ended.%n
//  Mode: Data Protection (Quick mode)
//  Filter: %n%1%n
//  Inbound SPI: %n%2%n
//  Outbound SPI: %n%3%n
//
#define SE_AUDITID_IPSEC_LOGOFF_QM       ((ULONG)0x0000021EL)











//
// MessageId: SE_AUDITID_IPSEC_LOGOFF_MM
//
// MessageText:
//
//  IKE security association ended.%n
//  Mode: Key Exchange (Main mode)%n
//  Filter: %n%1%n
//
#define SE_AUDITID_IPSEC_LOGOFF_MM       ((ULONG)0x0000021FL)














//
// MessageId: SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST
//
// MessageText:
//
//  IKE security association establishment failed because peer could not authenticate.
//  The certificate trust could not be established.%n
//  Peer Identity: %n%1%n
//  Filter: %n%2%n
//
#define SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST ((ULONG)0x00000220L)














//
// MessageId: SE_AUDITID_IPSEC_AUTH_FAIL
//
// MessageText:
//
//  IKE peer authentication failed.%n
//  Peer Identity: %n%1%n
//  Filter: %n%2%n
//
#define SE_AUDITID_IPSEC_AUTH_FAIL       ((ULONG)0x00000221L)




















//
// MessageId: SE_AUDITID_IPSEC_ATTRIB_FAIL
//
// MessageText:
//
//  IKE security association establishment failed because peer
//  sent invalid proposal.%n
//  Mode: %n%1%n
//  Filter: %n%2%n
//  Attribute: %n%3%n
//  Expected value: %n%4%n
//  Received value: %n%5%n
//
#define SE_AUDITID_IPSEC_ATTRIB_FAIL     ((ULONG)0x00000222L)


















//
// MessageId: SE_AUDITID_IPSEC_NEGOTIATION_FAIL
//
// MessageText:
//
//  IKE security association negotiation failed.%n
//  Mode: %n%1%n
//  Filter: %n%2%n
//  Peer Identity: %n%3%n 
//  Failure Point: %n%4%n
//  Failure Reason: %n%5%n
//  Extra Status: %n%6%n
//
#define SE_AUDITID_IPSEC_NEGOTIATION_FAIL ((ULONG)0x00000223L)



























//
// MessageId: SE_AUDITID_DOMAIN_TRUST_INCONSISTENT
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tDomain sid inconsistent%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//  %tTransited Services:%t%7%n
//
#define SE_AUDITID_DOMAIN_TRUST_INCONSISTENT ((ULONG)0x00000224L)




















//
// MessageId: SE_AUDITID_ALL_SIDS_FILTERED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason: %tAll sids were filtered out%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package%t: %5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_ALL_SIDS_FILTERED     ((ULONG)0x00000225L)











//
// MessageId: SE_AUDITID_IPSEC_IKE_NOTIFICATION
//
// MessageText:
//
//  %1%n
//
#define SE_AUDITID_IPSEC_IKE_NOTIFICATION ((ULONG)0x00000226L)





























//
// MessageId: SE_AUDITID_BEGIN_LOGOFF
//
// MessageText:
//
//  User initiated logoff:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//
#define SE_AUDITID_BEGIN_LOGOFF          ((ULONG)0x00000227L)

















//
// MessageId: SE_AUDITID_LOGON_USING_EXPLICIT_CREDENTIALS
//
// MessageText:
//
//  Logon attempt using explicit credentials:%n
//  Logged on user:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon GUID:%t%4%n
//  User whose credentials were used:%n
//  %tTarget User Name:%t%5%n
//  %tTarget Domain:%t%6%n
//  %tTarget Logon GUID: %7%n%n
//  Target Server Name:%t%8%n
//  Target Server Info:%t%9%n
//  Caller Process ID:%t%10%n
//  Source Network Address:%t%11%n
//  Source Port:%t%12%n
//
#define SE_AUDITID_LOGON_USING_EXPLICIT_CREDENTIALS ((ULONG)0x00000228L)


















//
// MessageId: SE_AUDITID_AUTH_REPLAY_DETECTED
//
// MessageText:
//
//  %tUser Name:%t%1%n
//  %tDomain:%t%%t%2%n
//  %tRequest Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6%n
//  %tCaller User Name:%t%7%n
//  %tCaller Domain:%t%8%n
//  %tCaller Logon ID:%t%9%n
//  %tCaller Process ID: %10%n
//  %tTransited Services: %11%n
//
#define SE_AUDITID_AUTH_REPLAY_DETECTED  ((ULONG)0x00000229L)






















































//
// MessageId: SE_AUDITID_OPEN_HANDLE
//
// MessageText:
//
//  Object Open:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tHandle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tImage File Name:%t%8%n
//  %tPrimary User Name:%t%9%n
//  %tPrimary Domain:%t%10%n
//  %tPrimary Logon ID:%t%11%n
//  %tClient User Name:%t%12%n
//  %tClient Domain:%t%13%n
//  %tClient Logon ID:%t%14%n
//  %tAccesses:%t%15%n
//  %tPrivileges:%t%16%n
//  %tRestricted Sid Count:%t%17%n
//  %tAccess Mask:%t%18%n
//
#define SE_AUDITID_OPEN_HANDLE           ((ULONG)0x00000230L)


















//
// MessageId: SE_AUDITID_CLOSE_HANDLE
//
// MessageText:
//
//  Handle Closed:%n
//  %tObject Server:%t%1%n
//  %tHandle ID:%t%2%n
//  %tProcess ID:%t%3%n
//  %tImage File Name:%t%4%n
//
#define SE_AUDITID_CLOSE_HANDLE          ((ULONG)0x00000232L)




































//
// MessageId: SE_AUDITID_OPEN_OBJECT_FOR_DELETE
//
// MessageText:
//
//  Object Open for Delete:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tHandle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tPrimary User Name:%t%8%n
//  %tPrimary Domain:%t%9%n
//  %tPrimary Logon ID:%t%10%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//  %tAccesses:%t%t%14%n
//  %tPrivileges:%t%t%15%n
//  %tAccess Mask:%t%16%n
//
#define SE_AUDITID_OPEN_OBJECT_FOR_DELETE ((ULONG)0x00000233L)


















//
// MessageId: SE_AUDITID_DELETE_OBJECT
//
// MessageText:
//
//  Object Deleted:%n
//  %tObject Server:%t%1%n
//  %tHandle ID:%t%2%n
//  %tProcess ID:%t%3%n
//  %tImage File Name:%t%4%n
//
#define SE_AUDITID_DELETE_OBJECT         ((ULONG)0x00000234L)






































//
// MessageId: SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE
//
// MessageText:
//
//  Object Open:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tHandle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tProcess Name:%t%8%n
//  %tPrimary User Name:%t%9%n
//  %tPrimary Domain:%t%10%n
//  %tPrimary Logon ID:%t%11%n
//  %tClient User Name:%t%12%n
//  %tClient Domain:%t%13%n
//  %tClient Logon ID:%t%14%n
//  %tAccesses:%t%15%n
//  %tPrivileges:%t%16%n%n
//  %tProperties:%n%17%n
//  %tAccess Mask:%t%18%n
//
#define SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE ((ULONG)0x00000235L)


































//
// MessageId: SE_AUDITID_OBJECT_OPERATION
//
// MessageText:
//
//  Object Operation:%n
//  %tObject Server:%t%1%n
//  %tOperation Type:%t%2%n
//  %tObject Type:%t%3%n
//  %tObject Name:%t%4%n
//  %tHandle ID:%t%5%n
//  %tPrimary User Name:%t%6%n
//  %tPrimary Domain:%t%7%n
//  %tPrimary Logon ID:%t%8%n
//  %tClient User Name:%t%9%n
//  %tClient Domain:%t%10%n
//  %tClient Logon ID:%t%11%n
//  %tAccesses:%t%12%n
//  %tProperties:%n%t%13%n
//  %tAdditional Info:%t%14%n
//  %tAdditional Info2:%t%15%n
//  %tAccess Mask:%t%16%n
//
#define SE_AUDITID_OBJECT_OPERATION      ((ULONG)0x00000236L)


















//
// MessageId: SE_AUDITID_OBJECT_ACCESS
//
// MessageText:
//
//  Object Access Attempt:%n
//  %tObject Server:%t%1%n
//  %tHandle ID:%t%2%n
//  %tObject Type:%t%3%n
//  %tProcess ID:%t%4%n
//  %tImage File Name:%t%5%n
//  %tAccesses:%t%6%n
//  %tAccess Mask:%t%7%n
//
#define SE_AUDITID_OBJECT_ACCESS         ((ULONG)0x00000237L)


















//
// MessageId: SE_AUDITID_HARDLINK_CREATION
//
// MessageText:
//
//  Hard link creation attempt:%n
//  %tPrimary User Name:%t%1%n
//  %tPrimary Domain:%t%2%n
//  %tPrimary Logon ID:%t%3%n
//  %tFile Name:%t%4%n
//  %tLink Name:%t%5%n
//
#define SE_AUDITID_HARDLINK_CREATION     ((ULONG)0x00000238L)




























//
// MessageId: SE_AUDITID_AZ_CLIENTCONTEXT_CREATION
//
// MessageText:
//
//  Application client context creation attempt:%n
//  %tApplication Name:%t%1%n
//  %tApplication Instance ID:%t%2%n
//  %tClient Name:%t%3%n
//  %tClient Domain:%t%4%n
//  %tClient Context ID:%t%5%n
//  %tStatus:%t%6%n
//
#define SE_AUDITID_AZ_CLIENTCONTEXT_CREATION ((ULONG)0x00000239L)













































//
// MessageId: SE_AUDITID_AZ_ACCESSCHECK
//
// MessageText:
//
//  Application operation attempt:%n
//  %tApplication Name:%t%1%n
//  %tApplication Instance ID:%t%2%n
//  %tObject Name:%t%3%n
//  %tScope Names:%t%4%n
//  %tClient Name:%t%5%n
//  %tClient Domain:%t%6%n
//  %tClient Context ID:%t%7%n
//  %tRole:%t%8%n
//  %tGroups:%t%9%n
//  %tOperation Name:%t%10 (%11)%n
//
#define SE_AUDITID_AZ_ACCESSCHECK        ((ULONG)0x0000023AL)
























//
// MessageId: SE_AUDITID_AZ_CLIENTCONTEXT_DELETION
//
// MessageText:
//
//  Application client context deletion:%n
//  %tApplication Name:%t%1%n
//  %tApplication Instance ID:%t%2%n
//  %tClient Name:%t%3%n
//  %tClient Domain:%t%4%n
//  %tClient Context ID:%t%5%n
//
#define SE_AUDITID_AZ_CLIENTCONTEXT_DELETION ((ULONG)0x0000023BL)


























//
// MessageId: SE_AUDITID_AZ_APPLICATION_INITIALIZATION
//
// MessageText:
//
//  Application Initialized%n
//  %tApplication Name:%t%1%n
//  %tApplication Instance ID:%t%2%n
//  %tClient Name:%t%3%n
//  %tClient Domain:%t%4%n
//  %tClient ID:%t%5%n
//  %tPolicy Store URL:%t%6%n
//
#define SE_AUDITID_AZ_APPLICATION_INITIALIZATION ((ULONG)0x0000023CL)























//
// MessageId: SE_AUDITID_GENERIC_AUDIT_EVENT
//
// MessageText:
//
//  %nApplication-specific security event.%n
//  %tEvent Source:%t%1%n
//  %tEvent ID:%t%2%n
//  %t%t%3%n
//  %t%t%4%n
//  %t%t%5%n
//  %t%t%6%n
//  %t%t%7%n
//  %t%t%8%n
//  %t%t%9%n
//  %t%t%10%n
//  %t%t%11%n
//  %t%t%12%n
//  %t%t%13%n
//  %t%t%14%n
//  %t%t%15%n
//  %t%t%16%n
//  %t%t%17%n
//  %t%t%18%n
//  %t%t%19%n
//  %t%t%20%n
//  %t%t%21%n
//  %t%t%22%n
//  %t%t%23%n
//  %t%t%24%n
//  %t%t%25%n
//  %t%t%26%n
//  %t%t%27%n
//
#define SE_AUDITID_GENERIC_AUDIT_EVENT   ((ULONG)0x0000023DL)















































//
// MessageId: SE_AUDITID_ASSIGN_SPECIAL_PRIV
//
// MessageText:
//
//  Special privileges assigned to new logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tPrivileges:%t%4
//
#define SE_AUDITID_ASSIGN_SPECIAL_PRIV   ((ULONG)0x00000240L)


































//
// MessageId: SE_AUDITID_PRIVILEGED_SERVICE
//
// MessageText:
//
//  Privileged Service Called:%n
//  %tServer:%t%t%1%n
//  %tService:%t%t%2%n
//  %tPrimary User Name:%t%3%n
//  %tPrimary Domain:%t%4%n
//  %tPrimary Logon ID:%t%5%n
//  %tClient User Name:%t%6%n
//  %tClient Domain:%t%7%n
//  %tClient Logon ID:%t%8%n
//  %tPrivileges:%t%9
//
#define SE_AUDITID_PRIVILEGED_SERVICE    ((ULONG)0x00000241L)






























//
// MessageId: SE_AUDITID_PRIVILEGED_OBJECT
//
// MessageText:
//
//  Privileged object operation:%n
//  %tObject Server:%t%1%n
//  %tObject Handle:%t%2%n
//  %tProcess ID:%t%3%n
//  %tPrimary User Name:%t%4%n
//  %tPrimary Domain:%t%5%n
//  %tPrimary Logon ID:%t%6%n
//  %tClient User Name:%t%7%n
//  %tClient Domain:%t%8%n
//  %tClient Logon ID:%t%9%n
//  %tPrivileges:%t%10
//
#define SE_AUDITID_PRIVILEGED_OBJECT     ((ULONG)0x00000242L)










































//
// MessageId: SE_AUDITID_PROCESS_CREATED
//
// MessageText:
//
//  A new process has been created:%n
//  %tNew Process ID:%t%1%n
//  %tImage File Name:%t%2%n
//  %tCreator Process ID:%t%3%n
//  %tUser Name:%t%4%n
//  %tDomain:%t%t%5%n
//  %tLogon ID:%t%t%6%n
//
#define SE_AUDITID_PROCESS_CREATED       ((ULONG)0x00000250L)






















//
// MessageId: SE_AUDITID_PROCESS_EXIT
//
// MessageText:
//
//  A process has exited:%n
//  %tProcess ID:%t%1%n
//  %tImage File Name:%t%2%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_PROCESS_EXIT          ((ULONG)0x00000251L)



















//
// MessageId: SE_AUDITID_DUPLICATE_HANDLE
//
// MessageText:
//
//  A handle to an object has been duplicated:%n
//  %tSource Handle ID:%t%1%n
//  %tSource Process ID:%t%2%n
//  %tTarget Handle ID:%t%3%n
//  %tTarget Process ID:%t%4%n
//
#define SE_AUDITID_DUPLICATE_HANDLE      ((ULONG)0x00000252L)


































//
// MessageId: SE_AUDITID_INDIRECT_REFERENCE
//
// MessageText:
//
//  Indirect access to an object has been obtained:%n
//  %tObject Type:%t%1%n
//  %tObject Name:%t%2%n
//  %tProcess ID:%t%3%n
//  %tPrimary User Name:%t%4%n
//  %tPrimary Domain:%t%5%n
//  %tPrimary Logon ID:%t%6%n
//  %tClient User Name:%t%7%n
//  %tClient Domain:%t%8%n
//  %tClient Logon ID:%t%9%n
//  %tAccesses:%t%10%n
//  %tAccess Mask:%t%11%n
//
#define SE_AUDITID_INDIRECT_REFERENCE    ((ULONG)0x00000253L)

















//
// MessageId: SE_AUDITID_DPAPI_BACKUP
//
// MessageText:
//
//  Backup of data protection master key.
//  %n
//  %tKey Identifier:%t%t%1%n
//  %tRecovery Server:%t%t%2%n
//  %tRecovery Key ID:%t%t%3%n
//  %tFailure Reason:%t%t%4%n
//
#define SE_AUDITID_DPAPI_BACKUP          ((ULONG)0x00000254L)



















//
// MessageId: SE_AUDITID_DPAPI_RECOVERY
//
// MessageText:
//
//  Recovery of data protection master key.
//  %n
//  %tKey Identifier:%t%t%1%n
//  %tRecovery Reason:%t%t%3%n
//  %tRecovery Server:%t%t%2%n
//  %tRecovery Key ID:%t%t%4%n
//  %tFailure Reason:%t%t%5%n
//
#define SE_AUDITID_DPAPI_RECOVERY        ((ULONG)0x00000255L)




















//
// MessageId: SE_AUDITID_DPAPI_PROTECT
//
// MessageText:
//
//  Protection of auditable protected data.
//  %n
//  %tData Description:%t%t%2%n
//  %tKey Identifier:%t%t%1%n
//  %tProtected Data Flags:%t%3%n
//  %tProtection Algorithms:%t%4%n
//  %tFailure Reason:%t%t%5%n
//
#define SE_AUDITID_DPAPI_PROTECT         ((ULONG)0x00000256L)




















//
// MessageId: SE_AUDITID_DPAPI_UNPROTECT
//
// MessageText:
//
//  Unprotection of auditable protected data.
//  %n
//  %tData Description:%t%t%2%n
//  %tKey Identifier:%t%t%1%n
//  %tProtected Data Flags:%t%3%n
//  %tProtection Algorithms:%t%4%n
//  %tFailure Reason:%t%t%5%n
//
#define SE_AUDITID_DPAPI_UNPROTECT       ((ULONG)0x00000257L)





















//
// MessageId: SE_AUDITID_ASSIGN_TOKEN
//
// MessageText:
//
//  A process was assigned a primary token.%n
//  Assigning Process Information:%n
//  %tProcess ID:%t%1%n
//  %tImage File Name:%t%2%n
//  %tPrimary User Name:%t%3%n
//  %tPrimary Domain:%t%4%n
//  %tPrimary Logon ID:%t%5%n
//  New Process Information:%n
//  %tProcess ID:%t%6%n
//  %tImage File Name:%t%7%n
//  %tTarget User Name:%t%8%n
//  %tTarget Domain:%t%9%n
//  %tTarget Logon ID:%t%10%n
//
#define SE_AUDITID_ASSIGN_TOKEN          ((ULONG)0x00000258L)














//
// MessageId: SE_AUDITID_SERVICE_INSTALL
//
// MessageText:
//
//  Attempt to install service:%n
//  %tService Name:%t%1%n
//  %tService File Name:%t%2%n
//  %tService Type:%t%3%n
//  %tService Start Type:%t%4%n
//  %tService Account:%t%5%n
//  By:%n
//  %tUser Name:%t%6%n
//  %tDomain:%t%t%7%n
//  %tLogon ID:%t%t%8%n
//
#define SE_AUDITID_SERVICE_INSTALL       ((ULONG)0x00000259L)




















//
// MessageId: SE_AUDITID_JOB_CREATED
//
// MessageText:
//
//  Scheduled Task created:%n
//  %tFile Name:%t%1%n
//  %tCommand:%t%2%n
//  %tTriggers:%t%t%3%n
//  %tTime:%t%t%4 %5%n
//  %tFlags:%t%t%6%n
//  %tTarget User:%t%7%n
//  By:%n
//  %tUser:%t%t%8%n
//  %tDomain:%t%t%9%n
//  %tLogon ID:%t%t%10%n
//
#define SE_AUDITID_JOB_CREATED           ((ULONG)0x0000025AL)
















































//
// MessageId: SE_AUDITID_USER_RIGHT_ASSIGNED
//
// MessageText:
//
//  User Right Assigned:%n
//  %tUser Right:%t%1%n
//  %tAssigned To:%t%2%n
//  %tAssigned By:%n
//  %t  User Name:%t%3%n
//  %t  Domain:%t%t%4%n
//  %t  Logon ID:%t%5%n
//
#define SE_AUDITID_USER_RIGHT_ASSIGNED   ((ULONG)0x00000260L)





















//
// MessageId: SE_AUDITID_USER_RIGHT_REMOVED
//
// MessageText:
//
//  User Right Removed:%n
//  %tUser Right:%t%1%n
//  %tRemoved From:%t%2%n
//  %tRemoved By:%n
//  %t  User Name:%t%3%n
//  %t  Domain:%t%t%4%n
//  %t  Logon ID:%t%5%n
//
#define SE_AUDITID_USER_RIGHT_REMOVED    ((ULONG)0x00000261L)


















//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_ADD
//
// MessageText:
//
//  New Trusted Domain:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tEstablished By:%n
//  %t  User Name:%t%3%n
//  %t  Domain:%t%t%4%n
//  %t  Logon ID:%t%5%n
//  %tTrust Type:%t%6%n
//  %tTrust Direction:%t%7%n
//  %tTrust Attributes:%t%8%n
//  %tSID Filtering:%t%9%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_ADD    ((ULONG)0x00000262L)


















//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_REM
//
// MessageText:
//
//  Trusted Domain Removed:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tRemoved By:%n
//  %t  User Name:%t%3%n
//  %t  Domain:%t%t%4%n
//  %t  Logon ID:%t%5%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_REM    ((ULONG)0x00000263L)











































//
// MessageId: SE_AUDITID_POLICY_CHANGE
//
// MessageText:
//
//  Audit Policy Change:%n
//  New Policy:%n
//  %tSuccess%tFailure%n
//  %t    %3%t    %4%tLogon/Logoff%n
//  %t    %5%t    %6%tObject Access%n
//  %t    %7%t    %8%tPrivilege Use%n
//  %t    %13%t    %14%tAccount Management%n
//  %t    %11%t    %12%tPolicy Change%n
//  %t    %1%t    %2%tSystem%n
//  %t    %9%t    %10%tDetailed Tracking%n
//  %t    %15%t    %16%tDirectory Service Access%n
//  %t    %17%t    %18%tAccount Logon%n%n
//  Changed By:%n
//  %t  User Name:%t%19%n
//  %t  Domain Name:%t%20%n
//  %t  Logon ID:%t%21
//
#define SE_AUDITID_POLICY_CHANGE         ((ULONG)0x00000264L)
















//
// MessageId: SE_AUDITID_IPSEC_POLICY_START
//
// MessageText:
//
//  IPSec Services started: %t%1%n
//  Policy Source: %t%2%n
//  %3%n
//
#define SE_AUDITID_IPSEC_POLICY_START    ((ULONG)0x00000265L)














//
// MessageId: SE_AUDITID_IPSEC_POLICY_DISABLED
//
// MessageText:
//
//  IPSec Services disabled: %t%1%n
//  %2%n
//
#define SE_AUDITID_IPSEC_POLICY_DISABLED ((ULONG)0x00000266L)












//
// MessageId: SE_AUDITID_IPSEC_POLICY_CHANGED
//
// MessageText:
//
//  IPSec Services: %t%1%n
//
#define SE_AUDITID_IPSEC_POLICY_CHANGED  ((ULONG)0x00000267L)












//
// MessageId: SE_AUDITID_IPSEC_POLICY_FAILURE
//
// MessageText:
//
//  IPSec Services encountered a potentially serious failure.%n
//  %1%n
//
#define SE_AUDITID_IPSEC_POLICY_FAILURE  ((ULONG)0x00000268L)


















//
// MessageId: SE_AUDITID_KERBEROS_POLICY_CHANGE
//
// MessageText:
//
//  Kerberos Policy Changed:%n
//  Changed By:%n
//  %t  User Name:%t%1%n
//  %t  Domain Name:%t%2%n
//  %t  Logon ID:%t%3%n
//  Changes made:%n
//  ('--' means no changes, otherwise each change is shown as:%n
//  <ParameterName>: <new value> (<old value>))%n
//  %4%n
//
#define SE_AUDITID_KERBEROS_POLICY_CHANGE ((ULONG)0x00000269L)


















//
// MessageId: SE_AUDITID_EFS_POLICY_CHANGE
//
// MessageText:
//
//  Encrypted Data Recovery Policy Changed:%n
//  Changed By:%n
//  %t  User Name:%t%1%n
//  %t  Domain Name:%t%2%n
//  %t  Logon ID:%t%3%n
//  Changes made:%n
//  ('--' means no changes, otherwise each change is shown as:%n
//  <ParameterName>: <new value> (<old value>))%n
//  %4%n
//
#define SE_AUDITID_EFS_POLICY_CHANGE     ((ULONG)0x0000026AL)


















//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_MOD
//
// MessageText:
//
//  Trusted Domain Information Modified:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tModified By:%n
//  %t  User Name:%t%3%n
//  %t  Domain:%t%t%4%n
//  %t  Logon ID:%t%5%n
//  %tTrust Type:%t%6%n
//  %tTrust Direction:%t%7%n
//  %tTrust Attributes:%t%8%n
//  %tSID Filtering:%t%9%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_MOD    ((ULONG)0x0000026CL)





















//
// MessageId: SE_AUDITID_SYSTEM_ACCESS_GRANTED
//
// MessageText:
//
//  System Security Access Granted:%n
//  %tAccess Granted:%t%4%n
//  %tAccount Modified:%t%5%n
//  %tAssigned By:%n
//  %t  User Name:%t%1%n
//  %t  Domain:%t%t%2%n
//  %t  Logon ID:%t%3%n
//
#define SE_AUDITID_SYSTEM_ACCESS_GRANTED ((ULONG)0x0000026DL)





















//
// MessageId: SE_AUDITID_SYSTEM_ACCESS_REMOVED
//
// MessageText:
//
//  System Security Access Removed:%n
//  %tAccess Removed:%t%4%n
//  %tAccount Modified:%t%5%n
//  %tRemoved By:%n
//  %t  User Name:%t%1%n
//  %t  Domain:%t%t%2%n
//  %t  Logon ID:%t%3%n
//
#define SE_AUDITID_SYSTEM_ACCESS_REMOVED ((ULONG)0x0000026EL)




















//
// MessageId: SE_AUDITID_NAMESPACE_COLLISION
//
// MessageText:
//
//  Namespace collision detected:%n
//  %tTarget type:%t%1%n
//  %tTarget name:%t%2%n
//  %tForest Root:%t%3%n
//  %tTop Level Name:%t%4%n
//  %tDNS Name:%t%5%n
//  %tNetBIOS Name:%t%6%n
//  %tSID:%t%t%7%n
//  %tNew Flags:%t%8%n
//
#define SE_AUDITID_NAMESPACE_COLLISION   ((ULONG)0x00000300L)























//
// MessageId: SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_ADD
//
// MessageText:
//
//  Trusted Forest Information Entry Added:%n
//  %tForest Root:%t%1%n
//  %tForest Root SID:%t%2%n
//  %tOperation ID:%t{%3,%4}%n
//  %tEntry Type:%t%5%n
//  %tFlags:%t%t%6%n
//  %tTop Level Name:%t%7%n
//  %tDNS Name:%t%8%n
//  %tNetBIOS Name:%t%9%n
//  %tDomain SID:%t%10%n
//  %tAdded by%t:%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//
#define SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_ADD ((ULONG)0x00000301L)























//
// MessageId: SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_REM
//
// MessageText:
//
//  Trusted Forest Information Entry Removed:%n
//  %tForest Root:%t%1%n
//  %tForest Root SID:%t%2%n
//  %tOperation ID:%t{%3,%4}%n
//  %tEntry Type:%t%5%n
//  %tFlags:%t%t%6%n
//  %tTop Level Name:%t%7%n
//  %tDNS Name:%t%8%n
//  %tNetBIOS Name:%t%9%n
//  %tDomain SID:%t%10%n
//  %tRemoved by%t:%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//
#define SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_REM ((ULONG)0x00000302L)























//
// MessageId: SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_MOD
//
// MessageText:
//
//  Trusted Forest Information Entry Modified:%n
//  %tForest Root:%t%1%n
//  %tForest Root SID:%t%2%n
//  %tOperation ID:%t{%3,%4}%n
//  %tEntry Type:%t%5%n
//  %tFlags:%t%t%6%n
//  %tTop Level Name:%t%7%n
//  %tDNS Name:%t%8%n
//  %tNetBIOS Name:%t%9%n
//  %tDomain SID:%t%10%n
//  %tModified by%t:%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//
#define SE_AUDITID_TRUSTED_FOREST_INFO_ENTRY_MOD ((ULONG)0x00000303L)






























//
// MessageId: SE_AUDITID_SECURITY_LOG_CONFIG
//
// MessageText:
//
//  Configuration of security log for this session:
//  %tMaximum Log Size (KB): %1%n
//  %tAction to take on reaching max log size: %2%n
//  %tEvent age limit in days: %3%n
//
#define SE_AUDITID_SECURITY_LOG_CONFIG   ((ULONG)0x00000325L)













//
// MessageId: SE_AUDITID_PER_USER_AUDIT_TABLE_CREATION
//
// MessageText:
//
//  Per User Audit Policy was refreshed.%n
//  %tNumber of elements:%t%1%n
//  %tPolicy ID:%t%2%n
//
#define SE_AUDITID_PER_USER_AUDIT_TABLE_CREATION ((ULONG)0x00000326L)
















//
// MessageId: SE_AUDITID_PER_USER_AUDIT_TABLE_ELEMENT_CREATION
//
// MessageText:
//
//  Per user auditing policy set for user:%n
//  %tTarget user:%t%1%n
//  %tPolicy ID:%t%2%n
//  %tCategory Settings:%n
//  %t System:%t%3%n
//  %t Logon:%t%4%n
//  %t Object Access%t%5%n
//  %t Privilege Use:%t%6%n
//  %t Detailed Tracking:%t%7%n
//  %t Policy Change:%t%8%n
//  %t Account Management:%t%9%n
//  %t DS Access:%t%10%n
//  %t Account Logon:%t%11%n
//
#define SE_AUDITID_PER_USER_AUDIT_TABLE_ELEMENT_CREATION ((ULONG)0x00000327L)













//
// MessageId: SE_AUDITID_SECURITY_EVENT_SOURCE_REGISTERED
//
// MessageText:
//
//  A security event source has attempted to register.%n
//  %tPrimary User Name:%t%1%n
//  %tPrimary Domain:%t%2%n
//  %tPrimary Logon ID:%t%3%n
//  %tClient User Name:%t%4%n
//  %tClient Domain:%t%5%n
//  %tClient Logon ID:%t%6%n
//  %tSource Name:%t%7%n
//  %tProcess Id:%t%8%n
//  %tEvent Source Id:%t%9%n
//
#define SE_AUDITID_SECURITY_EVENT_SOURCE_REGISTERED ((ULONG)0x00000328L)













//
// MessageId: SE_AUDITID_SECURITY_EVENT_SOURCE_UNREGISTERED
//
// MessageText:
//
//  A security event source has attempted to unregister.%n
//  %tPrimary User Name:%t%1%n
//  %tPrimary Domain:%t%2%n
//  %tPrimary Logon ID:%t%3%n
//  %tClient User Name:%t%4%n
//  %tClient Domain:%t%5%n
//  %tClient Logon ID:%t%6%n
//  %tSource Name:%t%7%n
//  %tProcess Id:%t%8%n
//  %tEvent Source Id:%t%9%n
//
#define SE_AUDITID_SECURITY_EVENT_SOURCE_UNREGISTERED ((ULONG)0x00000329L)









































































































//
// MessageId: SE_AUDITID_USER_CREATED
//
// MessageText:
//
//  User Account Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges%t%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tDisplay Name:%t%9%n
//  %tUser Principal Name:%t%10%n
//  %tHome Directory:%t%11%n
//  %tHome Drive:%t%12%n
//  %tScript Path:%t%13%n
//  %tProfile Path:%t%14%n
//  %tUser Workstations:%t%15%n
//  %tPassword Last Set:%t%16%n
//  %tAccount Expires:%t%17%n
//  %tPrimary Group ID:%t%18%n
//  %tAllowedToDelegateTo:%t%19%n
//  %tOld UAC Value:%t%20%n
//  %tNew UAC Value:%t%21%n
//  %tUser Account Control:%t%22%n
//  %tUser Parameters:%t%23%n
//  %tSid History:%t%24%n
//  %tLogon Hours:%t%25%n
//
#define SE_AUDITID_USER_CREATED          ((ULONG)0x00000270L)






























//
// MessageId: SE_AUDITID_USER_ENABLED
//
// MessageText:
//
//  User Account Enabled:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_ENABLED          ((ULONG)0x00000272L)






















//
// MessageId: SE_AUDITID_USER_PWD_CHANGED
//
// MessageText:
//
//  Change Password Attempt:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_USER_PWD_CHANGED      ((ULONG)0x00000273L)






















//
// MessageId: SE_AUDITID_USER_PWD_SET
//
// MessageText:
//
//  User Account password set:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_PWD_SET          ((ULONG)0x00000274L)






















//
// MessageId: SE_AUDITID_USER_DISABLED
//
// MessageText:
//
//  User Account Disabled:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_DISABLED         ((ULONG)0x00000275L)






















//
// MessageId: SE_AUDITID_USER_DELETED
//
// MessageText:
//
//  User Account Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_USER_DELETED          ((ULONG)0x00000276L)






















//
// MessageId: SE_AUDITID_GLOBAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Global Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_CREATED  ((ULONG)0x00000277L)
























//
// MessageId: SE_AUDITID_GLOBAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Global Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_ADD      ((ULONG)0x00000278L)
























//
// MessageId: SE_AUDITID_GLOBAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Global Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_REM      ((ULONG)0x00000279L)






















//
// MessageId: SE_AUDITID_GLOBAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Global Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_GLOBAL_GROUP_DELETED  ((ULONG)0x0000027AL)






















//
// MessageId: SE_AUDITID_LOCAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Local Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_CREATED   ((ULONG)0x0000027BL)
























//
// MessageId: SE_AUDITID_LOCAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Local Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_ADD       ((ULONG)0x0000027CL)
























//
// MessageId: SE_AUDITID_LOCAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Local Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_REM       ((ULONG)0x0000027DL)






















//
// MessageId: SE_AUDITID_LOCAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Local Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_LOCAL_GROUP_DELETED   ((ULONG)0x0000027EL)






















//
// MessageId: SE_AUDITID_LOCAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Local Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_CHANGE    ((ULONG)0x0000027FL)






















//
// MessageId: SE_AUDITID_OTHER_ACCOUNT_CHANGE
//
// MessageText:
//
//  General Account Database Change:%n
//  %tType of change:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tObject ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//
#define SE_AUDITID_OTHER_ACCOUNT_CHANGE  ((ULONG)0x00000280L)






















//
// MessageId: SE_AUDITID_GLOBAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Global Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_CHANGE   ((ULONG)0x00000281L)






















//
// MessageId: SE_AUDITID_USER_CHANGE
//
// MessageText:
//
//  User Account Changed:%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%9%n
//  %tDisplay Name:%t%10%n
//  %tUser Principal Name:%t%11%n
//  %tHome Directory:%t%12%n
//  %tHome Drive:%t%13%n
//  %tScript Path:%t%14%n
//  %tProfile Path:%t%15%n
//  %tUser Workstations:%t%16%n
//  %tPassword Last Set:%t%17%n
//  %tAccount Expires:%t%18%n
//  %tPrimary Group ID:%t%19%n
//  %tAllowedToDelegateTo:%t%20%n
//  %tOld UAC Value:%t%21%n
//  %tNew UAC Value:%t%22%n
//  %tUser Account Control:%t%23%n
//  %tUser Parameters:%t%24%n
//  %tSid History:%t%25%n
//  %tLogon Hours:%t%26%n
//
#define SE_AUDITID_USER_CHANGE           ((ULONG)0x00000282L)






















//
// MessageId: SE_AUDITID_DOMAIN_POLICY_CHANGE
//
// MessageText:
//
//  Domain Policy Changed: %1 modified%n
//  %tDomain Name:%t%t%2%n
//  %tDomain ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tMin. Password Age:%t%8%n
//  %tMax. Password Age:%t%9%n
//  %tForce Logoff:%t%10%n
//  %tLockout Threshold:%t%11%n
//  %tLockout Observation Window:%t%12%n
//  %tLockout Duration:%t%13%n
//  %tPassword Properties:%t%14%n
//  %tMin. Password Length:%t%15%n
//  %tPassword History Length:%t%16%n
//  %tMachine Account Quota:%t%17%n  
//  %tMixed Domain Mode:%t%18%n
//  %tDomain Behavior Version:%t%19%n
//  %tOEM Information:%t%20%n
//
#define SE_AUDITID_DOMAIN_POLICY_CHANGE  ((ULONG)0x00000283L)




























//
// MessageId: SE_AUDITID_ACCOUNT_AUTO_LOCKED
//
// MessageText:
//
//  User Account Locked Out:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Account ID:%t%3%n
//  %tCaller Machine Name:%t%2%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_ACCOUNT_AUTO_LOCKED   ((ULONG)0x00000284L)
























//
// MessageId: SE_AUDITID_COMPUTER_CREATED
//
// MessageText:
//
//  Computer Account Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges%t%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tDisplay Name:%t%9%n
//  %tUser Principal Name:%t%10%n
//  %tHome Directory:%t%11%n
//  %tHome Drive:%t%12%n
//  %tScript Path:%t%13%n
//  %tProfile Path:%t%14%n
//  %tUser Workstations:%t%15%n
//  %tPassword Last Set:%t%16%n
//  %tAccount Expires:%t%17%n
//  %tPrimary Group ID:%t%18%n
//  %tAllowedToDelegateTo:%t%19%n
//  %tOld UAC Value:%t%20%n
//  %tNew UAC Value:%t%21%n
//  %tUser Account Control:%t%22%n
//  %tUser Parameters:%t%23%n
//  %tSid History:%t%24%n
//  %tLogon Hours:%t%25%n
//  %tDNS Host Name:%t%26%n
//  %tService Principal Names:%t%27%n 
//
#define SE_AUDITID_COMPUTER_CREATED      ((ULONG)0x00000285L)






















//
// MessageId: SE_AUDITID_COMPUTER_CHANGE
//
// MessageText:
//
//  Computer Account Changed:%n
//  %t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%9%n
//  %tDisplay Name:%t%10%n
//  %tUser Principal Name:%t%11%n
//  %tHome Directory:%t%12%n
//  %tHome Drive:%t%13%n
//  %tScript Path:%t%14%n
//  %tProfile Path:%t%15%n
//  %tUser Workstations:%t%16%n
//  %tPassword Last Set:%t%17%n
//  %tAccount Expires:%t%18%n
//  %tPrimary Group ID:%t%19%n
//  %tAllowedToDelegateTo:%t%20%n
//  %tOld UAC Value:%t%21%n
//  %tNew UAC Value:%t%22%n
//  %tUser Account Control:%t%23%n
//  %tUser Parameters:%t%24%n
//  %tSid History:%t%25%n
//  %tLogon Hours:%t%26%n
//  %tDNS Host Name:%t%27%n
//  %tService Principal Names:%t%28%n 
//
#define SE_AUDITID_COMPUTER_CHANGE       ((ULONG)0x00000286L)






















//
// MessageId: SE_AUDITID_COMPUTER_DELETED
//
// MessageText:
//
//  Computer Account Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_COMPUTER_DELETED      ((ULONG)0x00000287L)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Local Group Created:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED ((ULONG)0x00000288L)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Local Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE ((ULONG)0x00000289L)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Local Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD ((ULONG)0x0000028AL)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Local Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM ((ULONG)0x0000028BL)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Local Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED ((ULONG)0x0000028CL)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Global Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED ((ULONG)0x0000028DL)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Global Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE ((ULONG)0x0000028EL)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Global Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD ((ULONG)0x0000028FL)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Global Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM ((ULONG)0x00000290L)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Global Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED ((ULONG)0x00000291L)






















//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Universal Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED ((ULONG)0x00000292L)






















//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Universal Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE ((ULONG)0x00000293L)
























//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Universal Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD ((ULONG)0x00000294L)
























//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Universal Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM ((ULONG)0x00000295L)






















//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Universal Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED ((ULONG)0x00000296L)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Universal Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED ((ULONG)0x00000297L)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Universal Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE ((ULONG)0x00000298L)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Universal Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD ((ULONG)0x00000299L)
























//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Universal Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM ((ULONG)0x0000029AL)






















//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Universal Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED ((ULONG)0x0000029BL)
























//
// MessageId: SE_AUDITID_GROUP_TYPE_CHANGE
//
// MessageText:
//
//  Group Type Changed:%n
//  %t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_GROUP_TYPE_CHANGE     ((ULONG)0x0000029CL)






















//
// MessageId: SE_AUDITID_ADD_SID_HISTORY
//
// MessageText:
//
//  Add SID History:%n
//  %tSource Account Name:%t%1%n
//  %tSource Account ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//  %tSidList:%t%10%n
//
#define SE_AUDITID_ADD_SID_HISTORY       ((ULONG)0x0000029DL)














//
// MessageId: SE_AUDITID_ADD_SID_HISTORY_FAILURE
//
// MessageText:
//
//  Add SID History:%n
//  %tSource Account Name:%t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_ADD_SID_HISTORY_FAILURE ((ULONG)0x0000029EL)






















//
// MessageId: SE_AUDITID_ACCOUNT_UNLOCKED
//
// MessageText:
//
//  User Account Unlocked:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_ACCOUNT_UNLOCKED      ((ULONG)0x0000029FL)























//
// MessageId: SE_AUDITID_SECURE_ADMIN_GROUP
//
// MessageText:
//
//  Set ACLs of members in administrators groups:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURE_ADMIN_GROUP    ((ULONG)0x000002ACL)























//
// MessageId: SE_AUDITID_ACCOUNT_NAME_CHANGE
//
// MessageText:
//
//  Account Name Changed:%n 
//  %tOld Account Name:%t%1%n
//  %tNew Account Name:%t%2%n
//  %tTarget Domain:%t%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_ACCOUNT_NAME_CHANGE   ((ULONG)0x000002ADL)





















//
// MessageId: SE_AUDITID_PASSWORD_HASH_ACCESS
//
// MessageText:
//
//  Password of the following user accessed:%n
//  %tTarget User Name:%t%1%n
//  %tTarget User Domain:%t%t%2%n
//  By user:%n
//  %tCaller User Name:%t%3%n
//  %tCaller Domain:%t%t%4%n
//  %tCaller Logon ID:%t%t%5%n
//
#define SE_AUDITID_PASSWORD_HASH_ACCESS  ((ULONG)0x000002AEL)






















//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_CREATED
//
// MessageText:
//
//  Basic Application Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_CREATED ((ULONG)0x000002AFL)






















//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_CHANGE
//
// MessageText:
//
//  Basic Application Group Changed:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_CHANGE ((ULONG)0x000002B0L)




























//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_ADD
//
// MessageText:
//
//  Basic Application Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_ADD   ((ULONG)0x000002B1L)




























//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_REM
//
// MessageText:
//
//  Basic Application Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_REM   ((ULONG)0x000002B2L)




























//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_NM_ADD
//
// MessageText:
//
//  Basic Application Group Non-Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_NM_ADD ((ULONG)0x000002B3L)




























//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_NM_REM
//
// MessageText:
//
//  Basic Application Group Non-Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_APP_BASIC_GROUP_NM_REM ((ULONG)0x000002B4L)






















//
// MessageId: SE_AUDITID_APP_BASIC_GROUP_DELETED
//
// MessageText:
//
//  Basic Application Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_APP_BASIC_GROUP_DELETED ((ULONG)0x000002B5L)






















//
// MessageId: SE_AUDITID_APP_QUERY_GROUP_CREATED
//
// MessageText:
//
//  LDAP Query Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_APP_QUERY_GROUP_CREATED ((ULONG)0x000002B6L)






















//
// MessageId: SE_AUDITID_APP_QUERY_GROUP_CHANGE
//
// MessageText:
//
//  LDAP Query Group Changed:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//  Changed Attributes:%n
//  %tSam Account Name:%t%8%n
//  %tSid History:%t%9%n
//
#define SE_AUDITID_APP_QUERY_GROUP_CHANGE ((ULONG)0x000002B7L)






















//
// MessageId: SE_AUDITID_APP_QUERY_GROUP_DELETED
//
// MessageText:
//
//  LDAP Query Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_APP_QUERY_GROUP_DELETED ((ULONG)0x000002B8L)
















//
// MessageId: SE_AUDITID_PASSWORD_POLICY_API_CALLED
//
// MessageText:
//
//  Password Policy Checking API is called:%n
//  %tCaller Username:%t%1%n
//  %tCaller Domain:%t%2%n
//  %tCaller Logon ID:%t%3%n
//  %tCaller Workstation:%t%4%n
//  %tProvided User Name (unauthenticated):%t%5%n
//  %tStatus Code:%t%6%n
//
#define SE_AUDITID_PASSWORD_POLICY_API_CALLED ((ULONG)0x000002B9L)















//
// MessageId: SE_AUDITID_DSRM_PASSWORD_SET
//
// MessageText:
//
//  An attempt to set the Directory Services Restore Mode 
//  administrator password has been made.%n
//  %tCaller Username:%t%1%n
//  %tCaller Domain:%t%2%n
//  %tCaller Logon ID:%t%3%n
//  %tCaller Workstation:%t%4%n
//  %tStatus Code:%t%5%n
//
#define SE_AUDITID_DSRM_PASSWORD_SET     ((ULONG)0x000002BAL)


































//
// MessageId: SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tSource Addr:%t%3%n
//  %tNaming Context:%t%4%n
//  %tOptions:%t%5%n
//  %tStatus Code:%t%6%n
//
#define SE_AUDITID_REPLICA_SOURCE_NC_ESTABLISHED ((ULONG)0x00000340L)















//
// MessageId: SE_AUDITID_REPLICA_SOURCE_NC_REMOVED
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tSource Addr:%t%3%n
//  %tNaming Context:%t%4%n
//  %tOptions:%t%5%n
//  %tStatus Code:%t%6%n
//
#define SE_AUDITID_REPLICA_SOURCE_NC_REMOVED ((ULONG)0x00000341L)















//
// MessageId: SE_AUDITID_REPLICA_SOURCE_NC_MODIFIED
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tSource Addr:%t%3%n
//  %tNaming Context:%t%4%n
//  %tOptions:%t%5%n
//  %tStatus Code:%t%6%n
//
#define SE_AUDITID_REPLICA_SOURCE_NC_MODIFIED ((ULONG)0x00000342L)















//
// MessageId: SE_AUDITID_REPLICA_DEST_NC_MODIFIED
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tDest. Addr:%t%3%n
//  %tNaming Context:%t%4%n
//  %tOptions:%t%5%n
//  %tStatus Code:%t%6%n
//
#define SE_AUDITID_REPLICA_DEST_NC_MODIFIED ((ULONG)0x00000343L)















//
// MessageId: SE_AUDITID_REPLICA_SOURCE_NC_SYNC_BEGINS
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tNaming Context:%t%3%n
//  %tOptions:%t%4%n
//  %tSession ID:%t%5%n
//  %tStart USN:%t%6%n
//
#define SE_AUDITID_REPLICA_SOURCE_NC_SYNC_BEGINS ((ULONG)0x00000344L)















//
// MessageId: SE_AUDITID_REPLICA_SOURCE_NC_SYNC_ENDS
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tNaming Context:%t%3%n
//  %tOptions:%t%4%n
//  %tSession ID:%t%5%n
//  %tEnd USN:%t%6%n
//  %tStatus Code:%t%7%n
//
#define SE_AUDITID_REPLICA_SOURCE_NC_SYNC_ENDS ((ULONG)0x00000345L)




















//
// MessageId: SE_AUDITID_REPLICA_OBJ_ATTR_REPLICATION
//
// MessageText:
//
//  %tSession ID:%t%1%n
//  %tObject:%t%2%n
//  %tAttribute:%t%3%n
//  %tType of change:%t%4%n
//  %tNew Value:%t%5%n
//  %tUSN:%t%6%n
//  %tStatus Code:%t%7%n
//
#define SE_AUDITID_REPLICA_OBJ_ATTR_REPLICATION ((ULONG)0x00000346L)


















//
// MessageId: SE_AUDITID_REPLICA_FAILURE_EVENT_BEGIN
//
// MessageText:
//
//  %tReplication Event:%t%1%n
//  %tAudit Status Code:%t%2%n
//
#define SE_AUDITID_REPLICA_FAILURE_EVENT_BEGIN ((ULONG)0x00000347L)























//
// MessageId: SE_AUDITID_REPLICA_FAILURE_EVENT_END
//
// MessageText:
//
//  %tReplication Event:%t%1%n
//  %tAudit Status Code:%t%2%n
//  %tReplication Status Code:%t%3%n
//
#define SE_AUDITID_REPLICA_FAILURE_EVENT_END ((ULONG)0x00000348L)















//
// MessageId: SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVALv
//
// MessageText:
//
//  %tDestination DRA:%t%1%n
//  %tSource DRA:%t%2%n
//  %tObject:%t%3%n
//  %tOptions:%t%4%n
//  %tStatus Code:%t%5%n
//
#define SE_AUDITID_REPLICA_LINGERING_OBJECT_REMOVALv ((ULONG)0x00000349L)



















































//
// MessageId: SE_AUDITID_AS_TICKET
//
// MessageText:
//
//  Authentication Ticket Request:%n
//  %tUser Name:%t%t%1%n
//  %tSupplied Realm Name:%t%2%n
//  %tUser ID:%t%t%t%3%n
//  %tService Name:%t%t%4%n
//  %tService ID:%t%t%5%n
//  %tTicket Options:%t%t%6%n
//  %tResult Code:%t%t%7%n
//  %tTicket Encryption Type:%t%8%n
//  %tPre-Authentication Type:%t%9%n
//  %tClient Address:%t%t%10%n
//  %tCertificate Issuer Name:%t%11%n
//  %tCertificate Serial Number:%t%12%n
//  %tCertificate Thumbprint:%t%13%n
//
#define SE_AUDITID_AS_TICKET             ((ULONG)0x000002A0L)













//
// MessageId: SE_AUDITID_AS_TICKET_FAILURE
//
// MessageText:
//
//  Authentication Ticket Request Failed:%n
//  %tUser Name:%t%1%n
//  %tSupplied Realm Name:%t%2%n
//  %tService Name:%t%3%n
//  %tTicket Options:%t%4%n
//  %tFailure Code:%t%5%n
//  %tClient Address:%t%6%n
//
#define SE_AUDITID_AS_TICKET_FAILURE     ((ULONG)0x000002A4L)





























//
// MessageId: SE_AUDITID_TGS_TICKET_REQUEST
//
// MessageText:
//
//  Service Ticket Request:%n
//  %tUser Name:%t%t%1%n
//  %tUser Domain:%t%t%2%n
//  %tService Name:%t%t%3%n
//  %tService ID:%t%t%4%n
//  %tTicket Options:%t%t%5%n
//  %tTicket Encryption Type:%t%6%n
//  %tClient Address:%t%t%7%n
//  %tFailure Code:%t%t%8%n
//  %tLogon GUID:%t%t%9%n
//  %tTransited Services:%t%10%n
//
#define SE_AUDITID_TGS_TICKET_REQUEST    ((ULONG)0x000002A1L)























//
// MessageId: SE_AUDITID_TICKET_RENEW_SUCCESS
//
// MessageText:
//
//  Service Ticket Renewed:%n
//  %tUser Name:%t%1%n
//  %tUser Domain:%t%2%n
//  %tService Name:%t%3%n
//  %tService ID:%t%4%n
//  %tTicket Options:%t%5%n
//  %tTicket Encryption Type:%t%6%n
//  %tClient Address:%t%7%n
//
#define SE_AUDITID_TICKET_RENEW_SUCCESS  ((ULONG)0x000002A2L)

























//
// MessageId: SE_AUDITID_PREAUTH_FAILURE
//
// MessageText:
//
//  Pre-authentication failed:%n
//  %tUser Name:%t%1%n
//  %tUser ID:%t%t%2%n
//  %tService Name:%t%3%n
//  %tPre-Authentication Type:%t%4%n
//  %tFailure Code:%t%5%n
//  %tClient Address:%t%6%n
//
#define SE_AUDITID_PREAUTH_FAILURE       ((ULONG)0x000002A3L)












//
// MessageId: SE_AUDITID_TGS_TICKET_FAILURE
//
// MessageText:
//
//  Service Ticket Request Failed:%n
//  %tUser Name:%t%1%n
//  %tUser Domain:%t%2%n
//  %tService Name:%t%3%n
//  %tTicket Options:%t%4%n
//  %tFailure Code:%t%5%n
//  %tClient Address:%t%6%n
//
#define SE_AUDITID_TGS_TICKET_FAILURE    ((ULONG)0x000002A5L)






















//
// MessageId: SE_AUDITID_ACCOUNT_MAPPED
//
// MessageText:
//
//  Account Mapped for Logon.%n
//  Mapping Attempted By:%n 
//  %t%1%n
//  Client Name:%n
//  %t%2%n
//  %tMapped Name:%n
//  %t%3%n
//
#define SE_AUDITID_ACCOUNT_MAPPED        ((ULONG)0x000002A6L)













//
// MessageId: SE_AUDITID_ACCOUNT_NOT_MAPPED
//
// MessageText:
//
//  The name:%n
//  %t%2%n
//  could not be mapped for logon by:
//  %t%1%n
//
#define SE_AUDITID_ACCOUNT_NOT_MAPPED    ((ULONG)0x000002A7L)













//
// MessageId: SE_AUDITID_ACCOUNT_LOGON
//
// MessageText:
//
//  Logon attempt by:%t%1%n
//  Logon account:%t%2%n
//  Source Workstation:%t%3%n
//  Error Code:%t%4%n
//
#define SE_AUDITID_ACCOUNT_LOGON         ((ULONG)0x000002A8L)













//
// MessageId: SE_AUDITID_ACCOUNT_LOGON_FAILURE
//
// MessageText:
//
//  The logon to account: %2%n
//  by: %1%n
//  from workstation: %3%n
//  failed. The error code was: %4%n
//
#define SE_AUDITID_ACCOUNT_LOGON_FAILURE ((ULONG)0x000002A9L)






















//
// MessageId: SE_AUDITID_SESSION_RECONNECTED
//
// MessageText:
//
//  Session reconnected to winstation:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tSession Name:%t%4%n
//  %tClient Name:%t%5%n
//  %tClient Address:%t%6
//
#define SE_AUDITID_SESSION_RECONNECTED   ((ULONG)0x000002AAL)






















//
// MessageId: SE_AUDITID_SESSION_DISCONNECTED
//
// MessageText:
//
//  Session disconnected from winstation:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tSession Name:%t%4%n
//  %tClient Name:%t%5%n
//  %tClient Address:%t%6
//
#define SE_AUDITID_SESSION_DISCONNECTED  ((ULONG)0x000002ABL)



















































//
// MessageId: SE_AUDITID_CERTSRV_DENYREQUEST
//
// MessageText:
//
//  The certificate manager denied a pending certificate request.%n
//  %n
//  Request ID:%t%1
//
#define SE_AUDITID_CERTSRV_DENYREQUEST   ((ULONG)0x00000304L)












//
// MessageId: SE_AUDITID_CERTSRV_RESUBMITREQUEST
//
// MessageText:
//
//  Certificate Services received a resubmitted certificate request.%n
//  %n
//  Request ID:%t%1
//
#define SE_AUDITID_CERTSRV_RESUBMITREQUEST ((ULONG)0x00000305L)














//
// MessageId: SE_AUDITID_CERTSRV_REVOKECERT
//
// MessageText:
//
//  Certificate Services revoked a certificate.%n
//  %n
//  Serial No:%t%1%n
//  Reason:%t%2
//
#define SE_AUDITID_CERTSRV_REVOKECERT    ((ULONG)0x00000306L)
















//
// MessageId: SE_AUDITID_CERTSRV_PUBLISHCRL
//
// MessageText:
//
//  Certificate Services received a request to publish the certificate revocation list (CRL).%n
//  %n
//  Next Update:%t%1%n
//  Publish Base:%t%2%n
//  Publish Delta:%t%3
//
#define SE_AUDITID_CERTSRV_PUBLISHCRL    ((ULONG)0x00000307L)




















//
// MessageId: SE_AUDITID_CERTSRV_AUTOPUBLISHCRL
//
// MessageText:
//
//  Certificate Services published the certificate revocation list (CRL).%n
//  %n
//  Base CRL:%t%1%n
//  CRL No:%t%t%2%n
//  Key Container:%t%3%n
//  Next Publish:%t%4%n
//  Publish URLs:%t%5
//
#define SE_AUDITID_CERTSRV_AUTOPUBLISHCRL ((ULONG)0x00000308L)




















//
// MessageId: SE_AUDITID_CERTSRV_SETEXTENSION
//
// MessageText:
//
//  A certificate request extension changed.%n
//  %n
//  Request ID:%t%1%n
//  Name:%t%2%n
//  Type:%t%3%n
//  Flags:%t%4%n
//  Data:%t%5
//
#define SE_AUDITID_CERTSRV_SETEXTENSION  ((ULONG)0x00000309L)














//
// MessageId: SE_AUDITID_CERTSRV_SETATTRIBUTES
//
// MessageText:
//
//  One or more certificate request attributes changed.%n
//  %n
//  Request ID:%t%1%n
//  Attributes:%t%2
//
#define SE_AUDITID_CERTSRV_SETATTRIBUTES ((ULONG)0x0000030AL)










//
// MessageId: SE_AUDITID_CERTSRV_SHUTDOWN
//
// MessageText:
//
//  Certificate Services received a request to shut down.
//
#define SE_AUDITID_CERTSRV_SHUTDOWN      ((ULONG)0x0000030BL)












//
// MessageId: SE_AUDITID_CERTSRV_BACKUPSTART
//
// MessageText:
//
//  Certificate Services backup started.%n
//  Backup Type:%t%1
//
#define SE_AUDITID_CERTSRV_BACKUPSTART   ((ULONG)0x0000030CL)










//
// MessageId: SE_AUDITID_CERTSRV_BACKUPEND
//
// MessageText:
//
//  Certificate Services backup completed.
//
#define SE_AUDITID_CERTSRV_BACKUPEND     ((ULONG)0x0000030DL)










//
// MessageId: SE_AUDITID_CERTSRV_RESTORESTART
//
// MessageText:
//
//   Certificate Services restore started.
//
#define SE_AUDITID_CERTSRV_RESTORESTART  ((ULONG)0x0000030EL)










//
// MessageId: SE_AUDITID_CERTSRV_RESTOREEND
//
// MessageText:
//
//  Certificate Services restore completed.
//
#define SE_AUDITID_CERTSRV_RESTOREEND    ((ULONG)0x0000030FL)


















//
// MessageId: SE_AUDITID_CERTSRV_SERVICESTART
//
// MessageText:
//
//  Certificate Services started.%n
//  %n
//  Certificate Database Hash:%t%1%n
//  Private Key Usage Count:%t%2%n
//  CA Certificate Hash:%t%3%n
//  CA Public Key Hash:%t%4
//
#define SE_AUDITID_CERTSRV_SERVICESTART  ((ULONG)0x00000310L)


















//
// MessageId: SE_AUDITID_CERTSRV_SERVICESTOP
//
// MessageText:
//
//  Certificate Services stopped.%n
//  %n
//  Certificate Database Hash:%t%1%n
//  Private Key Usage Count:%t%2%n
//  CA Certificate Hash:%t%3%n
//  CA Public Key Hash:%t%4
//
#define SE_AUDITID_CERTSRV_SERVICESTOP   ((ULONG)0x00000311L)












//
// MessageId: SE_AUDITID_CERTSRV_SETSECURITY
//
// MessageText:
//
//  The security permissions for Certificate Services changed.%n
//  %n
//  %1
//
#define SE_AUDITID_CERTSRV_SETSECURITY   ((ULONG)0x00000312L)












//
// MessageId: SE_AUDITID_CERTSRV_GETARCHIVEDKEY
//
// MessageText:
//
//  Certificate Services retrieved an archived key.%n
//  %n
//  Request ID:%t%1
//
#define SE_AUDITID_CERTSRV_GETARCHIVEDKEY ((ULONG)0x00000313L)














//
// MessageId: SE_AUDITID_CERTSRV_IMPORTCERT
//
// MessageText:
//
//  Certificate Services imported a certificate into its database.%n
//  %n
//  Certificate:%t%1%n
//  Request ID:%t%2
//
#define SE_AUDITID_CERTSRV_IMPORTCERT    ((ULONG)0x00000314L)












//
// MessageId: SE_AUDITID_CERTSRV_SETAUDITFILTER
//
// MessageText:
//
//  The audit filter for Certificate Services changed.%n
//  %n
//  Filter:%t%1
//
#define SE_AUDITID_CERTSRV_SETAUDITFILTER ((ULONG)0x00000315L)
















//
// MessageId: SE_AUDITID_CERTSRV_NEWREQUEST
//
// MessageText:
//
//  Certificate Services received a certificate request.%n
//  %n
//  Request ID:%t%1%n
//  Requestor:%t%2%n
//  Attributes:%t%3
//
#define SE_AUDITID_CERTSRV_NEWREQUEST    ((ULONG)0x00000316L)






















//
// MessageId: SE_AUDITID_CERTSRV_REQUESTAPPROVED
//
// MessageText:
//
//  Certificate Services approved a certificate request and issued a certificate.%n
//  %n
//  Request ID:%t%1%n
//  Requestor:%t%2%n
//  Attributes:%t%3%n
//  Disposition:%t%4%n
//  SKI:%t%t%5%n
//  Subject:%t%6
//
#define SE_AUDITID_CERTSRV_REQUESTAPPROVED ((ULONG)0x00000317L)






















//
// MessageId: SE_AUDITID_CERTSRV_REQUESTDENIED
//
// MessageText:
//
//  Certificate Services denied a certificate request.%n
//  %n
//  Request ID:%t%1%n
//  Requestor:%t%2%n
//  Attributes:%t%3%n
//  Disposition:%t%4%n
//  SKI:%t%t%5%n
//  Subject:%t%6
//
#define SE_AUDITID_CERTSRV_REQUESTDENIED ((ULONG)0x00000318L)






















//
// MessageId: SE_AUDITID_CERTSRV_REQUESTPENDING
//
// MessageText:
//
//  Certificate Services set the status of a certificate request to pending.%n
//  %n
//  Request ID:%t%1%n
//  Requestor:%t%2%n
//  Attributes:%t%3%n
//  Disposition:%t%4%n
//  SKI:%t%t%5%n
//  Subject:%t%6
//
#define SE_AUDITID_CERTSRV_REQUESTPENDING ((ULONG)0x00000319L)














//
// MessageId: SE_AUDITID_CERTSRV_SETOFFICERRIGHTS
//
// MessageText:
//
//  The certificate manager settings for Certificate Services changed.%n
//  %n
//  Enable:%t%1%n
//  %n
//  %2
//
#define SE_AUDITID_CERTSRV_SETOFFICERRIGHTS ((ULONG)0x0000031AL)
















//
// MessageId: SE_AUDITID_CERTSRV_SETCONFIGENTRY
//
// MessageText:
//
//  A configuration entry changed in Certificate Services.%n
//  %n
//  Node:%t%1%n
//  Entry:%t%2%n
//  Value:%t%3
//
#define SE_AUDITID_CERTSRV_SETCONFIGENTRY ((ULONG)0x0000031BL)


















//
// MessageId: SE_AUDITID_CERTSRV_SETCAPROPERTY
//
// MessageText:
//
//  A property of Certificate Services changed.%n
//  %n
//  Property:%t%1%n
//  Index:%t%2%n
//  Type:%t%3%n
//  Value:%t%4
//
#define SE_AUDITID_CERTSRV_SETCAPROPERTY ((ULONG)0x0000031CL)
















//
// MessageId: SE_AUDITID_CERTSRV_KEYARCHIVED
//
// MessageText:
//
//  Certificate Services archived a key.%n
//  %n
//  Request ID:%t%1%n
//  Requestor:%t%2%n
//  KRA Hashes:%t%3
//
#define SE_AUDITID_CERTSRV_KEYARCHIVED   ((ULONG)0x0000031DL)












//
// MessageId: SE_AUDITID_CERTSRV_IMPORTKEY
//
// MessageText:
//
//  Certificate Services imported and archived a key.%n
//  %n
//  Request ID:%t%1
//
#define SE_AUDITID_CERTSRV_IMPORTKEY     ((ULONG)0x0000031EL)
















//
// MessageId: SE_AUDITID_CERTSRV_PUBLISHCACERT
//
// MessageText:
//
//  Certificate Services published the CA certificate to Active Directory.%n
//  %n
//  Certificate Hash:%t%1%n
//  Valid From:%t%2%n
//  Valid To:%t%3
//
#define SE_AUDITID_CERTSRV_PUBLISHCACERT ((ULONG)0x0000031FL)
















//
// MessageId: SE_AUDITID_CERTSRV_DELETEROW
//
// MessageText:
//
//  One or more rows have been deleted from the certificate database.%n
//  %n
//  Table ID:%t%1%n
//  Filter:%t%2%n
//  Rows Deleted:%t%3
//
#define SE_AUDITID_CERTSRV_DELETEROW     ((ULONG)0x00000320L)












//
// MessageId: SE_AUDITID_CERTSRV_ROLESEPARATIONSTATE
//
// MessageText:
//
//  Role separation enabled:%t%1
//
#define SE_AUDITID_CERTSRV_ROLESEPARATIONSTATE ((ULONG)0x00000321L)


























//
// MessageId: SE_AUDITID_CERTSRV_FULLTEMPLATE
//
// MessageText:
//
//  Certificate Services template:%n%1 v%2 (Schema V%3)%n%4%n%5%n%nDomain Controller:%t%6%n%nTemplate Content:%n%7%nSecurity Descriptor:%n%8
//
#define SE_AUDITID_CERTSRV_FULLTEMPLATE  ((ULONG)0x00000322L)


























//
// MessageId: SE_AUDITID_CERTSRV_UPDATEDTEMPLATE
//
// MessageText:
//
//  Certificate Services template updated:%n%1 v%2 (Schema V%3)%n%4%n%5%n%nDomain Controller:%t%6%n%nOld Template Content:%n%8%n%nNew Template Content:%n%7
//
#define SE_AUDITID_CERTSRV_UPDATEDTEMPLATE ((ULONG)0x00000323L)






























//
// MessageId: SE_AUDITID_CERTSRV_UPDATEDTEMPLATESECURITY
//
// MessageText:
//
//  Certificate Services template security updated:%n%1 v%2 (Schema V%3)%n%4%n%5%n%nDomain Controller:%t%6%n%nOld Template Content:%n%9%nOld Security Descriptor:%n%10%n%nNew Template Content:%n%7%nNew Security Descriptor:%n%8
//
#define SE_AUDITID_CERTSRV_UPDATEDTEMPLATESECURITY ((ULONG)0x00000324L)

  


#endif 
