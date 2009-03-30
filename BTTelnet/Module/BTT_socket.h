/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_socket.h

Function:       A telnet client module for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Thu 14th September 2000
                     Redesign to better move telnet processing inside
                     the module
******************************************************************************/

// Check if the header file has already been included
#ifndef  __BTT_SOCKET_H
# define __BTT_SOCKET_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

/** Constants ----------------------------------------------------------*/
// SWI names
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

// SSL
#define SSL_CLIENT_V2      0
#define SSL_SERVER_V2      1
#define SSL_CLIENT_V3      2
#define SSL_SERVER_V3      3
#define SSL_CLIENT_V23     4
#define SSL_SERVER_V23     5
#define SSL_CLIENT_TLS     6
#define SSL_SERVER_TLS     7

#define Secure_Connect     0x54280
#define Secure_Read        0x54282
#define Secure_Peek        0x54283
#define Secure_Write       0x54284
#define Secure_GetError    0x54285
#define Secure_Disconnect  0x54286
#define Secure_PollConnect 0x54293

// Error codes
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

// Misc
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

#define INADDR_ANY              (CARD32)0x00000000
#define INADDR_BROADCAST        (CARD32)0xffffffff      /* must be masked */
#define INADDR_NONE             0xffffffff              /* -1 return */

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

// Resolver info
#define RESOLVER_SUCCESS 0
#define RESOLVER_FAILED  1
#define RESOLVER_WAITING 2

// File descriptor code
#ifndef NBBY
# define NBBY    8               /* number of bits in a byte */
#endif

#ifndef FD_SETSIZE
# define FD_SETSIZE      256
#endif

typedef int fd_mask;
#define NFDBITS (sizeof(fd_mask) * NBBY)        /* bits per mask */
#ifndef howmany
#define howmany(x, y)   (((x)+((y)-1))/(y))
#endif

typedef struct fd_set {
  fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define FD_SET(n, p)      ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)      ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)    ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)        bzero((p), sizeof(fd_set))
#define FD_COPY(src, dst) bcopy(src, dst, sizeof(fd_set))

/** Macros -------------------------------------------------------------*/

/** TypeDefines --------------------------------------------------------*/

/** Structures -------------------------------------------------------*/
// Internet address
struct in_addr
{
  unsigned long s_addr;
};

// Internet socket
struct sockaddr_in
{
  short   sin_family;
  unsigned short sin_port;
  struct  in_addr sin_addr;
  char    sin_zero[8];
};

// Host ent structure
#define h_addr h_addr_list[0]  /* address, for backward compatibility */
struct hostent
{
  char *h_name;       /* official name of host */
  char **h_aliases;   /* alias list */
  int h_addrtype;     /* host address type */
  int h_length;       /* length of address */
  char **h_addr_list; /* list of addresses returned */
};

// Address
struct sockaddr
{
  unsigned sa_family;              /* address family */
  char     sa_data[14];            /* up to 14 bytes of direct address */
};

// Timeval
struct timeval {
  long tv_sec;		/* seconds */
  long tv_usec;	/* and microseconds */
};

/** Globals ----------------------------------------------------------*/
INT32 socketResolve(BTSession_t *ptr);
INT32 socketCreate(BTSession_t *ptr);
INT32 socketCreateSSL(BTSession_t *ptr);
INT32 socketConnectSSL(BTSession_t *ptr);
void socketClose(BTSession_t *ptr);
INT32 socketConnect(BTSession_t *ptr);
INT32 socketUpdate(BTSession_t *ptr);
INT32 socketWrite(BTSession_t *ptr);
INT32 socketRead(BTSession_t *ptr);
BOOL socketWork(BTSession_t *ptr);

/* Assembler included calls */
DECLARE CARD32 htonl(CARD32 value);
DECLARE CARD32 ntohl(CARD32 value);
DECLARE CARD16 htons(CARD16 value);
DECLARE CARD16 ntohs(CARD16 value);

DECLARE fd_set connected;
DECLARE fd_set writing;
DECLARE fd_set reading;
DECLARE fd_set working;
DECLARE CARD32 maxSocket   INIT(0);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_SOCKET_H



