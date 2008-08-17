/******************************************************************************

Program:        ----

File:           Socker.h

Function:       Header File For Socket.c

Description:    Contains Structures and defines required for socket use

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sat 08th August 1998

******************************************************************************/

/* Checks if file already included */
#ifndef __LW_SOCKET_H
#define __LW_SOCKET_H

/* Includes ----------------------------------------------------------------*/

/* Constants ---------------------------------------------------------------*/

/* SWI Names */
#define Socket_Creat       0x41200
#define Socket_Bind        0x41201
#define Socket_Listen      0x41202
#define Socket_Accept      0x41203
#define Socket_Connect     0x41204
#define Socket_Recv        0x41205
#define Socket_Recvfrom    0x41206
#define Socket_Recvmsg     0x41207
#define Socket_Send        0x41208
#define Socket_Sendto      0x41209
#define Socket_Sendmsg     0x4120A
#define Socket_Shutdown    0x4120B
#define Socket_Setsockopt  0x4120C
#define Socket_Getsockopt  0x4120D
#define Socket_Getpeername 0x4120E
#define Socket_Getsockname 0x4120F
#define Socket_Close       0x41210
#define Socket_Select      0x41211
#define Socket_Ioctl       0x41212
#define Socket_Read        0x41213
#define Socket_Write       0x41214
#define Socket_Stat        0x41215
#define Socket_Readv       0x41216
#define Socket_Writev      0x41217
#define Socket_Gettsize    0x41218
#define Socket_Sendtosm    0x41219
#define Resolver_GetHost   0x46001

/* Error code defines */
#define	EINTR	    	   4		/* Interrupted system call */
#define EBADF              9
#define EFAULT            14
#define EINVAL            22
#define EWOULDBLOCK       35
#define EINPROGRESS       36
#define EALREADY          37
#define EMSGSIZE          40
#define ENOPROTOOPT       42
#define EPROTONOSUPPORT   43
#define ESOCKTNOSUPPORT   44
#define EOPNOTSUPP        45
#define EAFNOSUPPORT      47
#define EADDRINUSE        48
#define ENETUNREACH       51
#define ECONNRESET        54
#define ENOBUFS           55
#define EISCONN           56
#define ENOTCONN          57
#define ECONNREFUSED      61
#define EHOSTUNREACH      65

/* Misc constants */
#define SOCK_STREAM       1
#define SOCK_DGRAM        2
#define SOCK_RAW          3
#define PF_INET           2
#define AF_INET           2

#define MSG_OOB           0x01
#define MSG_PEEK          0x02
#define MSG_EOR           0x08
#define MSG_TRUNC         0x10
#define MSG_WAITALL       0x40
#define MSG_DONTWAIT      0x80

#define SO_DEBUG          0x0001
#define SO_ACCEPTCONN     0x0002
#define SO_REUSEADDR      0x0004
#define SO_KEEPALIVE      0x0008
#define SO_DONTROUTE      0x0010
#define SO_BROADCAST      0x0020
#define SO_LINGER         0x0080

#define SO_OOBINLINE      0x0100
#define SO_SNDBUF         0x1001
#define SO_RCVBUF         0x1002
#define SO_SNDLOWAT       0x1003
#define SO_RCVLOWAT       0x1004
#define SO_SNDTIMEO       0x1005
#define SO_RCVTIMEO       0x1006

#define SO_ERROR          0x1007
#define SO_TYPE           0x1008

#define SOL_SOCKET        0xFFFF

#define IP_OPTIONS        1
#define IP_HDRINCL        2
#define IP_TOS            3
#define IP_TTL            4

#define IPPROTO_TCP       6
#define TCP_NODELAY       1
#define TCP_MAXSEG        2

#define FIOASYNC          0x8004667D
#define FIONBIO           0x8004667E
#define FIONREAD          0x4004667F

#define SIOCSIFADDR       0x8020690C
#define OSIOCGIFADDR      0xC020690D
#define SIOCGIFADDR       0xC0206921
#define SIOCSIFDSTADDR    0x8020690E
#define OSIOCGIFDSTADDR   0xC020690F
#define SIOCGIFDSTADDR    0xC0206922
#define SIOCSIFFLAGS      0x80206910
#define SIOCGIFFLAGS      0xC0206911
#define OSIOCGIFBRDADDR   0xC0206912
#define SIOCGIFBRDADDR    0xC0206923
#define SIOCSIFBRDADDR    0x80206913
#define OSIOCGIFCONF      0xC0206914
#define SIOCATMARK        0x80047307
#define SIOCGIFCONF       0xC0206924
#define OSIOCGIFNETMASK   0xC0206915
#define SIOCGIFNETMASK    0xC0206925
#define SIOCSIFNETMASK    0x80206916
#define SIOCGIFMETRIC     0xC0206917
#define SIOCSIFMETRIC     0x80206918
#define SIOCDIFADDR       0x80206919
#define SIOCAIFADDR       0x80206920

#define bzero(Ptr, n)    memset(Ptr, 0, n)
#define bcopy(Src, Dest, Len) memcpy(Dest, Src, Len)

/* Resolver info */
#define ResolverSuccess 0
#define ResolverFailed  1
#define ResolverWaiting 2

/* TypeDefines -------------------------------------------------------------*/

/* Internet address (a structure for historical reasons) */
struct in_addr
{
  unsigned long s_addr;
};

/* Socket address, internet style */
struct sockaddr_in
{
  short   sin_family;
  unsigned short sin_port;
  struct  in_addr sin_addr;
  char    sin_zero[8];
};

/* Host ent structure */
#define h_addr h_addr_list[0]  /* address, for backward compatibility */
struct hostent
{
  char *h_name;       /* official name of host */
  char **h_aliases;   /* alias list */
  int h_addrtype;     /* host address type */
  int h_length;       /* length of address */
  char **h_addr_list; /* list of addresses returned */
};

/* Address */
struct sockaddr
{
  unsigned sa_family;              /* address family */
  char     sa_data[14];            /* up to 14 bytes of direct address */
};

#endif  /* #ifndef __LW_SOCKET_H */
