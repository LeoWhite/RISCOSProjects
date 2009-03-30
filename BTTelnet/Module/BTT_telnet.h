/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_telnet.h

Function:       A telnet client module for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 17th September 2000
                     Creation.

******************************************************************************/

// Check if the header file has already been included
#ifndef  __BTT_TELNET_H
# define __BTT_TELNET_H
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

/** Telnet specific **/
#define BEL        7
#define BS         8
#define HT         9
#define LF         10
#define VT         11
#define FF         12
#define CR         13
#define REDRAW     18
#define eraseline  21
#define DEL        127

/* Telnet defines */
#define IAC     255             /* interpret as command: */
#define DONT    254             /* you are not to use option */
#define DO      253             /* please, you use option */
#define WONT    252             /* I won't use option */
#define WILL    251             /* I will use option */
#define SB      250             /* interpret as subnegotiation */
#define GA      249             /* you may reverse the line */
#define EL      248             /* erase the current line */
#define EC      247             /* erase the current character */
#define AYT     246             /* are you there */
#define AO      245             /* abort output--but let prog finish */
#define IP      244             /* interrupt process--permanently */
#define BREAK   243             /* break */
#define DM      242             /* data mark--for connect. cleaning */
#define NOP     241             /* nop */
#define SE      240             /* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */

#define SYNCH   242             /* for telfunc calls */

/* telnet options */
#define TELOPT_BINARY   0       /* 8-bit data path */
#define TELOPT_ECHO     1       /* echo */
#define TELOPT_RCP      2       /* prepare to reconnect */
#define TELOPT_SGA      3       /* suppress go ahead */
#define TELOPT_NAMS     4       /* approximate message size */
#define TELOPT_STATUS   5       /* give status */
#define TELOPT_TM       6       /* timing mark */
#define TELOPT_RCTE     7       /* remote controlled transmission and echo */
#define TELOPT_NAOL     8       /* negotiate about output line width */
#define TELOPT_NAOP     9       /* negotiate about output page size */
#define TELOPT_NAOCRD   10      /* negotiate about CR disposition */
#define TELOPT_NAOHTS   11      /* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD   12      /* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD   13      /* negotiate about formfeed disposition */
#define TELOPT_NAOVTS   14      /* negotiate about vertical tab stops */
#define TELOPT_NAOVTD   15      /* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD   16      /* negotiate about output LF disposition */
#define TELOPT_XASCII   17      /* extended ascic character set */
#define TELOPT_LOGOUT   18      /* force logout */
#define TELOPT_BM       19      /* byte macro */
#define TELOPT_DET      20      /* data entry terminal */
#define TELOPT_SUPDUP   21      /* supdup protocol */
#define TELOPT_SUPDUPOUTPUT 22  /* supdup output */
#define TELOPT_SNDLOC   23      /* send location */
#define TELOPT_TTYPE    24      /* terminal type */
#define TELOPT_EOR      25      /* end or record */

#define TELOPT_AUTH     37      /** Authenticaton */
#define TELOPT_EXOPL    255     /* extended-options-list */

/** telnet options */
#define IS         0
#define SEND       1
#define REPLY      2
#define AUTH_NAME       3

#define AUTH_NULL        0
#define AUTH_KERBEROS_V4 1
#define AUTH_KERBEROS_V5 2
#define AUTH_SPX         3
#define AUTH_RSA         6
#define AUTH_SSL         7
#define AUTH_LOKI        10

#define AUTH_SSL_START   1
#define AUTH_SSL_ACCEPT  2
#define AUTH_SSL_REJECT  3


/** Q Option defines */
#define Q_NO         0
#define Q_WANTNO     1
#define Q_WANTYES    2
#define Q_YES        3

#define Q_EMPTY      0
#define Q_OPPOSITE   1

/** Macros -------------------------------------------------------------*/

/** TypeDefines --------------------------------------------------------*/

/** Structures -------------------------------------------------------*/

/** Globals ----------------------------------------------------------*/
DECLARE BOOL telnetWork(BTSession_t *ptr);
DECLARE unsigned int telnetWrite(BTSession_t *ptr, char *buffer, unsigned int length);
DECLARE void telnetSendNegotiation(BTSession_t *ptr);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_TELNET_H



