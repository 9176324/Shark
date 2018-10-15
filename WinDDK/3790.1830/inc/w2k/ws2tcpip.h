/*	
**	WS2TCPIP.H - WinSock2 Extension for TCP/IP protocols
**
**	This file contains TCP/IP specific information for use
**	by WinSock2 compatible applications.
**
** Copyright (c) 1995-1999  Microsoft Corporation
**
**	To provide the backward compatibility, all the TCP/IP
**	specific definitions that were included in the WINSOCK.H
**	 file are now included in WINSOCK2.H file. WS2TCPIP.H
**	file includes only the definitions  introduced in the
**	"WinSock 2 Protocol-Specific Annex" document.
**
**	Rev 0.3	Nov 13, 1995
**      Rev 0.4	Dec 15, 1996
*/

#ifndef _WS2TCPIP_H_
#define _WS2TCPIP_H_

#if _MSC_VER > 1000
#pragma once
#endif

/* Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP */

struct ip_mreq {
	struct in_addr imr_multiaddr;	/* IP multicast address of group */
	struct in_addr imr_interface;	/* local IP address of interface */
};

/* TCP/IP specific Ioctl codes */

#define SIO_GET_INTERFACE_LIST  _IOR('t', 127, u_long)	
/* New IOCTL with address size independent address array */
#define SIO_GET_INTERFACE_LIST_EX  _IOR('t', 126, u_long)	

/* Option to use with [gs]etsockopt at the IPPROTO_IP level */

#define	IP_OPTIONS		1 /* set/get IP options */
#define	IP_HDRINCL		2 /* header is included with data */
#define	IP_TOS			3 /* IP type of service and preced*/
#define	IP_TTL			4 /* IP time to live */
#define	IP_MULTICAST_IF		9 /* set/get IP multicast i/f  */
#define	IP_MULTICAST_TTL       10 /* set/get IP multicast ttl */
#define	IP_MULTICAST_LOOP      11 /*set/get IP multicast loopback */
#define	IP_ADD_MEMBERSHIP      12 /* add an IP group membership */
#define	IP_DROP_MEMBERSHIP     13/* drop an IP group membership */
#define IP_DONTFRAGMENT     14 /* don't fragment IP datagrams */


/* Option to use with [gs]etsockopt at the IPPROTO_UDP level */

#define UDP_NOCHECKSUM	1

/* Option to use with [gs]etsockopt at the IPPROTO_TCP level */

#define  TCP_EXPEDITED_1122	0x0002


/* IPv6 definitions */

struct in_addr6 {
		u_char	s6_addr[16];	/* IPv6 address */
};

/* Old IPv6 socket address structure (retained for sockaddr_gen definition below) */

struct sockaddr_in6_old {
		short	sin6_family;   	/* AF_INET6 */
		u_short sin6_port;	/* Transport level port number */
		u_long	sin6_flowinfo;	/* IPv6 flow information */
		struct in_addr6 sin6_addr; /* IPv6 address */
};

/* IPv6 socket address structure, RFC 2553 */

struct sockaddr_in6 {
		short	sin6_family;   	/* AF_INET6 */
		u_short sin6_port;	/* Transport level port number */
		u_long	sin6_flowinfo;	/* IPv6 flow information */
		struct in_addr6 sin6_addr; /* IPv6 address */
        u_long sin6_scope_id;  /* set of interfaces for a scope */
};


typedef struct in_addr6 IN6_ADDR;
typedef struct in_addr6 *PIN6_ADDR;
typedef struct in_addr6 FAR *LPIN6_ADDR;

typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_in6 *PSOCKADDR_IN6;
typedef struct sockaddr_in6 FAR *LPSOCKADDR_IN6;


#define IN6ADDR_SETANY(x) {\
(x)->sin6_family = AF_INET6; \
(x)->sin6_port = 0; \
(x)->sin6_flowinfo = 0; \
*((u_long *)((x)->sin6_addr.s6_addr)    ) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) = 0; \
}
#define IN6ADDR_SETLOOPBACK(x) {\
(x)->sin6_family = AF_INET6; \
(x)->sin6_port = 0; \
(x)->sin6_flowinfo = 0; \
*((u_long *)((x)->sin6_addr.s6_addr)    ) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) = 0; \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) = 1; \
}

#define IN6ADDR_ISANY(x) ( \
(x)->sin6_family == AF_INET6 && \
*((u_long *)((x)->sin6_addr.s6_addr)    ) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) == 0 \
)


#define IN6ADDR_ISLOOPBACK(x) (\
(x)->sin6_family == AF_INET6 && \
*((u_long *)((x)->sin6_addr.s6_addr)    ) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 1) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 2) == 0 && \
*((u_long *)((x)->sin6_addr.s6_addr) + 3) == 1 \
)


typedef union sockaddr_gen{
		struct sockaddr Address;
		struct sockaddr_in  AddressIn;
		struct sockaddr_in6_old AddressIn6;
} sockaddr_gen;

/* Structure to keep interface specific information */

typedef struct _INTERFACE_INFO
{
	u_long		iiFlags;		/* Interface flags */
	sockaddr_gen	iiAddress;		/* Interface address */
	sockaddr_gen	iiBroadcastAddress; 	/* Broadcast address */
	sockaddr_gen	iiNetmask;		/* Network mask */
} INTERFACE_INFO, FAR * LPINTERFACE_INFO;

/* New structure that does not have dependency on the address size */
typedef struct _INTERFACE_INFO_EX
{
	u_long      iiFlags;        /* Interface flags */
	SOCKET_ADDRESS  iiAddress;          /* Interface address */
	SOCKET_ADDRESS  iiBroadcastAddress; /* Broadcast address */
	SOCKET_ADDRESS  iiNetmask;          /* Network mask */
} INTERFACE_INFO_EX, FAR * LPINTERFACE_INFO_EX;

/* Possible flags for the  iiFlags - bitmask  */

#define IFF_UP		0x00000001 /* Interface is up */
#define IFF_BROADCAST	0x00000002 /* Broadcast is  supported */
#define IFF_LOOPBACK	0x00000004 /* this is loopback interface */
#define IFF_POINTTOPOINT 0x00000008 /*this is point-to-point interface*/
#define IFF_MULTICAST	0x00000010 /* multicast is supported */


#endif	/* _WS2TCPIP_H_ */


