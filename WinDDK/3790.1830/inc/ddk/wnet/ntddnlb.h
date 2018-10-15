//depot/dnsrv_dev/net/published/inc/ntddnlb.w#3 - edit change 56921 (text)
/*++

Copyright (c) 2001 Microsoft Corporation

Module Name:

    ntddnlb.h

Abstract:

    This header describes the structures and interfaces required to interact
    with the NLB intermediate device driver.

Revision History:

--*/

#ifndef __NTDDNLB_H__
#define __NTDDNLB_H__

/*
 * Note: This file is intended for inclusion by kernel-mode components that
 *       wish to utilize the public kernel-mode NLB connection notification 
 *       interface or the public kernel-mode NLB hook IOCTLs.  In order to 
 *       include only those definitions that are relevant to user-level 
 *       programs (i.e., registry key names), ensure that the following 
 *       macro is defined prior to including this file either via the 
 *       compiler command line:
 *
 *       -DNTDDNLB_USER_MODE_DEFINITIONS_ONLY=1
 *
 *       or in the source file(s) before inclusion of this file:
 *
 *       #define NTDDNLB_USER_MODE_DEFINITIONS_ONLY
 *
 *       Failing to define this macro before including this file in a user-
 *       level program will otherwise result in compilation failure.
 */

/*
  This registry key can be used to configure NLB to allow inter-host 
  communication in a unicast cluster under the following conditions:

  (i)  EVERY host in the cluster is properly configured with a Dedicated
       IP address, where "properly configured" implies:

       (a) The Dedicated IP address configured in NLB is valid
       (b) The Dedicated IP address configured in NLB has been added to
           to the TCP/IP properties IP address list OF THE NLB ADAPTER
       (c) The Dedicated IP address is the FIRST IP address listed in
           the TCP/IP properties IP address list OF THE NLB ADAPTER

       If any of these conditions is not met, do not expect inter-host 
       communication to work between all hosts, nor under all circumstances.

  (ii) The host is operating in unicast mode

  If the host is operating in eith multicast mode (multicast or IGMP multicast, 
  this feature is not needed, as hosts can already communication between one 
  another without additional support.

  This registry key will not exist by default, but can be created and
  set on a per-adapter basis under the following registry hive:
        
    HKLM\System\CurrentControlSet\Services\WLBS\Parameters\Interface\{GUID}\

  where {GUID} is the GUID of the particular NLB instance (use the "ClusterIP
  Address" key in this hive to identify clusters).  This key is boolean in 
  nature and is interpreted as follows:

    = 0 -> The feature is not active
    > 0 -> The feature is active

  ** This feature is intended solely for the purpose of inter-host communication 
  within a clustered firewall application (for example, Microsoft ISA Server).  
  Though its use outside of these types of applications is not precluded, ITS 
  USE OUTSIDE OF THESE APPLICATIONS IS EXPLICITLY NOT SUPPORTED BY MICROSOFT.
*/
#define NLB_INTERHOST_COMM_SUPPORT_KEY    L"UnicastInterHostCommSupport"

/*
  This registry key instructs NLB which notification mechanism to use.
  When deciding what notification(s) to use for connection management,
  NLB checks the following, in this order:

  (i)  NLB first looks for the EnableTCPNotification registry key under 
        
       HKLM\System\CurrentControlSet\Services\WLBS\Parameters\Global\
        
       This key has three possible values that instruct NLB on which 
       notifications to listen for.  They are:
        
         0 = Do not use any connection notifications.
         1 = Use the TCP connection notifications. 
         2 = Use the NLB public connection notifications.

  (ii) If the EnableTCPNotification registry is not present, NLB defaults
       to using TCP notifications.

  Note: The name of the key is EnableTCPNotifications for legacy reasons
  although it controls multiple notifications covering multiple protocols. 
*/
#define NLB_CONNECTION_CALLBACK_KEY       L"EnableTCPNotification"

#define NLB_CONNECTION_CALLBACK_NONE      0
#define NLB_CONNECTION_CALLBACK_TCP       1
#define NLB_CONNECTION_CALLBACK_ALTERNATE 2

#if !defined (NTDDNLB_USER_MODE_DEFINITIONS_ONLY)

/* This is the public callback object on which NLB will listen for connection
   callbacks.  Currently, only TCP (protocol=6) notifications are accepted by
   NLB.  To notify NLB when connections change state, open the callback object,
   and call ExNotifyCallback with the following parameters:
   
   CallbackObject - The handle to the NLB public callback object.
   Argument1 - A pointer to an NLBConnectionInfo block, defined below.
   Argument2 - NULL (This parameter is currently unused).

   For TCP connections, NLB needs to be notified of the following state changes:

   CLOSED -> SYN_RCVD: A new incoming connection is being established.  This 
   notification requires the IP interface index on which the SYN was received.
   NLB will create state on the appropriate interface to track this TCP connection.

   CLOSED -> SYN_SENT: A new outgoing connection is being established.  At this
   time, it is unknown on which interface the connection will ultimately be
   established, so the IP interface index is NOT required for this notification.
   NLB will create temporary state to track this connection should it return
   on an NLB interface.

   SYN_SENT -> ESTAB: An outgoing connection has been established.  This nofication
   requires the IP interface index on which the connection was ultimately established.
   If the interface was NLB, state will be created to track the new connection; if
   the interface was not NLB, the temporary state created by the SYN_SENT notification
   is cleaned up.

   SYN_RCVD -> ESTAB: An incoming connection has been established.  This notification 
   is not currently required by NLB.

   SYN_SENT -> CLOSED: An outgoing connection has been prematurely terminated (the 
   connection never reached the ESTABlished state).  This notification does not require
   the IP interface index.  NLB will destroy any state created to track this connection.

   SYN_RCVD -> CLOSED: An outgoing connection has been prematurely terminated (the
   connection never reached the ESTABlished state).  This notification does not require
   the IP interface index.  NLB will destroy any state created to track this connection.
   
   ESTAB -> CLOSED: A connection has been *completely* terminated (i.e., the connection 
   has gone through TIME_WAIT, if necessary, already).  This notification does not 
   require the IP interface index.  NLB will destroy any state created to track this 
   connection.
*/
#define NLB_CONNECTION_CALLBACK_NAME      L"\\Callback\\NLBConnectionCallback"

#include <ndis.h>
#include <ntddndis.h>
#include <devioctl.h>

#define NLB_TCPIP_PROTOCOL_TCP            6 /* IP protocol ID for TCP. */

#define NLB_TCP_CLOSED                    1 /* The TCP connection is/was CLOSED. */
#define NLB_TCP_SYN_SENT                  3 /* The TCP connection is/was in SYN_SENT. */
#define NLB_TCP_SYN_RCVD                  4 /* The TCP connection is/was in SYN_RCVD. */
#define NLB_TCP_ESTAB                     5 /* The TCP connection is/was ESTABlished. */

/* Force default alignment on the callback buffers. */
#pragma pack(push)
#pragma pack()
typedef struct NLBTCPAddressInfo {
    ULONG             RemoteIPAddress;     /* The remote (client) IP address, in network byte order. */
    ULONG             LocalIPAddress;      /* The local (server) IP address, in network byte order. */
    USHORT            RemotePort;          /* The remote (client) TCP port, in network byte order. */
    USHORT            LocalPort;           /* The local (server) TCP port, in network byte order. */
} NLBTCPAddressInfo;

typedef struct NLBTCPConnectionInfo {
    ULONG             PreviousState;        /* The previous state for the connection, as defined above. */
    ULONG             CurrentState;         /* The new state for the connection, as defined above. */
    ULONG             IPInterface;          /* The IP interface index on which the connection was, or is being, established. */
    NLBTCPAddressInfo Address;              /* A pointer to a block containing the IP tuple for the connection. */
} NLBTCPConnectionInfo;

typedef struct NLBConnectionInfo {
    UCHAR                      Protocol;    /* The protocol of the connection (currently, only TCP is supported). */
    union {
        NLBTCPConnectionInfo * pTCPInfo;    /* A pointer to the TCP connection information block. */
    };
} NLBConnectionInfo;
#pragma pack(pop)

#define NLB_DEVICE_NAME            L"\\Device\\WLBS"                            /* The NLB device name for use in ZwCreateFile, for instance. */

/* 
   This IOCTL registers or de-registers a kernel-mode hook with NLB.
   
   Returns:
   o STATUS_SUCCESS - if the (de)registration succeeds.
   o STATUS_INVALID_PARAMETER - if a parameter is invalid. E.g.,
       - The I/O buffers are missing or the incorrect size.
       - The HookIdentifier does not match a known NLB hook GUID.
       - The HookTable entry is non-NULL, but the DeregisterCallback is NULL.
       - The HookTable entry is non-NULL, but all hook function pointers are NULL.
       - The HookTable entry is NULL, but no function is registered for this hook.
   o STATUS_ACCESS_DENIED - if the operation will NOT be permitted by NLB. E.g.,
       - The request to (de)register a hook does not come from kernel-mode.
       - The de-register information provided is for a hook that was registered
         by a different component, as identified by the RegisteringEntity.
       - The specified hook has already been registered by somebody (anybody).
         Components wishing to change their hook must first de-register it.
*/
#define NLB_IOCTL_REGISTER_HOOK    CTL_CODE(0xc0c0, 18, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define NLB_HOOK_IDENTIFIER_LENGTH 39                                           /* 39 is sufficient for {GUID}. */

#define NLB_FILTER_HOOK_INTERFACE  L"{069267c4-7eee-4aff-832c-02e22e00f96f}"    /* The filter interface includes hooks for influencing the NLB
                                                                                   load-balancing decision on either the send path, receive path,
                                                                                   or both.  This hook will be called for any packet for which
                                                                                   NLB would normally apply load-balancing policy.  Components 
                                                                                   registering this interface should use an NLB_FILTER_HOOK_TABLE
                                                                                   as the hook table in the NLB_IOCTL_REGISTER_HOOK_REQUEST. */

/* The de-register callback must be specifed for all register 
   operations.  This function is called by NLB whenever a 
   registered hook is de-registered, either gracefully by the 
   registrar, or forcefully by NLB itself (as a result of the 
   NLB device driver getting unloaded). */
typedef VOID (* NLBHookDeregister) (PWCHAR pHookIdentifier, HANDLE RegisteringEntity, ULONG Flags);

/* Bit settings for the Flags field of the de-register callback. */
#define NLB_HOOK_DEREGISTER_FLAGS_FORCED 0x00000001

/* This enumerated type is the feedback for all filter hooks. */
typedef enum {
    NLB_FILTER_HOOK_PROCEED_WITH_HASH,                                          /* Continue to load-balance normally; i.e., the hook has no specific feedback. */
    NLB_FILTER_HOOK_REVERSE_HASH,                                               /* Use reverse hashing (use destination parameters, rather than source). */
    NLB_FILTER_HOOK_FORWARD_HASH,                                               /* Use conventional forward hashing (use source parameters). */
    NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY,                                     /* By-pass load-balancing and accept the packet unconditionally. */
    NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY                                      /* By-pass load-balancing and reject the packet unconditionally. */
} NLB_FILTER_HOOK_DIRECTIVE;

/* 
   Filter hooks:

   The adapter GUID (1st parameter) will allow the hook consumer to
   determine the adapter on which the packet is being sent or received.  
   Note that the length parameters are not necesarily indicative of the 
   actual length of the media header or payload themselves, but rather 
   indicate how much of the buffers pointed to are contiguously 
   accessible from the provided pointer.  For instance, the payload
   length may just be the length of an IP header, meaning that only
   the IP header can be found at that pointer.  However, it might
   be equal to the total size of the packet payload, in which case, 
   that pointer can be used to access subsequent pieces of the 
   packet, such as the TCP header.  If the payload length provided
   is not sufficient to find all necessary packet information, the
   packet pointer can be used to traverse the packet buffers manually 
   to try and find the information needed.  However, note that the 
   packet may not always be available (it may be NULL).
*/

/* The send filter hook is invoked for every packet sent on any
   adapter to which NLB is bound for which NLB would normally 
   apply load-balancing policy.  ARPs, for instance, are not 
   filtered by NLB, so such packets would not be indicated to 
   this hook. */
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBSendFilterHook) (
    const WCHAR *       pAdapter,                                               /* The GUID of the adapter on which the packet is being sent. */
    const NDIS_PACKET * pPacket,                                                /* A pointer to the NDIS packet, which CAN be NULL if not available. */
    const UCHAR *       pMediaHeader,                                           /* A pointer to the media header (ethernet, since NLB supports only ethernet). */
    ULONG               cMediaHeaderLength,                                     /* The length of contiguous memory accessible from the media header pointer. */
    const UCHAR *       pPayload,                                               /* A pointer to the payload of the packet. */
    ULONG               cPayloadLength,                                         /* The length of contiguous memory accesible from the payload pointer. */
    ULONG               Flags);                                                 /* Hook-related flags including whether or not the cluster is stopped. */

/* The receive filter hook is invoked for every packet received 
   on any adapter to which NLB is bound for which NLB would 
   normally apply load-balancing policy.  Some protocols, such
   as ARP, or NLB-specific packets not normally seen by the 
   protocol(s) bound to NLB (heartbeats, remote control requests)
   are not filtered by NLB and will not be indicated to the hook. */
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBReceiveFilterHook) (
    const WCHAR *       pAdapter,                                               /* The GUID of the adapter on which the packet was received. */
    const NDIS_PACKET * pPacket,                                                /* A pointer to the NDIS packet, which CAN be NULL if not available. */
    const UCHAR *       pMediaHeader,                                           /* A pointer to the media header (ethernet, since NLB supports only ethernet). */
    ULONG               cMediaHeaderLength,                                     /* The length of contiguous memory accessible from the media header pointer. */
    const UCHAR *       pPayload,                                               /* A pointer to the payload of the packet. */
    ULONG               cPayloadLength,                                         /* The length of contiguous memory accesible from the payload pointer. */
    ULONG               Flags);                                                 /* Hook-related flags including whether or not the cluster is stopped. */

/* The query filter hook is invoked in cases where the NLB driver
   needs to invoke its hashing algorithm and therefore needs to 
   know whether or not the hook will influence the way in which 
   manner NLB performs the hash, if at all. */
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBQueryFilterHook) (
    const WCHAR *       pAdapter,                                               /* The GUID of the adapter on which the packet was received. */
    ULONG               ServerIPAddress,                                        /* The server IP address of the "packet" in NETWORK byte order. */
    USHORT              ServerPort,                                             /* The server port of the "packet" (if applicable to the Protocol) in HOST byte order. */
    ULONG               ClientIPAddress,                                        /* The client IP address of the "packet" in NETWORK byte order. */
    USHORT              ClientPort,                                             /* The client port of the "packet" (if applicable to the Protocol) in HOST byte order. */
    UCHAR               Protocol,                                               /* The IP protocol of the "packet"; TCP, UDP, ICMP, GRE, etc. */
    BOOLEAN             bReceiveContext,                                        /* A boolean to indicate whether the packet is being processed in send or receive context. */
    ULONG               Flags);                                                 /* Hook-related flags including whether or not the cluster is stopped. */

/* Bit settings for the Flags field of the filter hooks. */
#define NLB_FILTER_HOOK_FLAGS_STOPPED  0x00000001
#define NLB_FILTER_HOOK_FLAGS_DRAINING 0x00000002

/* Force default alignment on the IOCTL buffers. */
#pragma pack(push)
#pragma pack()
/* This table contains function pointers to register or de-register
   a packet filter hook.  To register a hook, set the appropriate
   function pointer.  Those not being specified (for instance if
   you want to register a receive hook, but not a send hook) should
   be set to NULL.  The QueryHook should ONLY be specified if in 
   conjunction with setting either the send or receive hook; i.e.
   a user may not ONLY register the QueryHook.  Further, if regis-
   tering a send or receive hook (or both), the QueryHook MUST be
   provided in order for NLB to query the hook response for cases
   where a hashing decision is needed, but we are not in the context
   of sending or receiving a packet; most notably, in a connection
   up or down notification from IPSec or TCP. */
typedef struct {
    NLBSendFilterHook    SendHook;
    NLBQueryFilterHook   QueryHook;
    NLBReceiveFilterHook ReceiveHook;
} NLB_FILTER_HOOK_TABLE, * PNLB_FILTER_HOOK_TABLE;

/* This is the input buffer for the hook (de)register IOCTL.  There is
   no corresponding output buffer.  This structure identifies the hook 
   interface being (de)registered, the entity registering the hook and
   all appropriate function pointers (callbacks).  Note that hooks are
   registered in groups, called interfaces, which prevents different
   related hooks from being owned by different entities (for example,
   it prevents one entity owned the send hook, but another owned the 
   receive hook).  Interfaces are identified by a GUID, and to set any
   hook in the interface requires ownership of the entire interface - 
   even if not all hooks in the interface are being specified.  The hook
   table should be a pointer to a hook table of the type required by the
   specified hook identifier.  To de-register a hook, set the hook table 
   pointer to NULL. 

   Note: The HookTable pointer does NOT need to be valid following the 
   completion of the IOCTL.  That is, this pointer is only referenced
   within the context of the IOCTL.
*/
typedef struct {
    WCHAR             HookIdentifier[NLB_HOOK_IDENTIFIER_LENGTH];               /* The GUID identifying the hook interface being registered. */
    HANDLE            RegisteringEntity;                                        /* The open file handle on the NLB driver, which uniquely identifies the registrar. */
    PVOID             HookTable;                                                /* A pointer to the appropriate hook table containing the hook function pointers. */
    NLBHookDeregister DeregisterCallback;                                       /* The de-register callback function, which MUST be non-NULL if the operation is a registration. */
} NLB_IOCTL_REGISTER_HOOK_REQUEST, * PNLB_IOCTL_REGISTER_HOOK_REQUEST;
#pragma pack(pop)

#endif /* !USER_MODE_DEFINITIONS_ONLY */

#endif /* __NTDDNLB_H__ */

