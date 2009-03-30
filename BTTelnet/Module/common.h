/******************************************************************************

Program:        BeyondThought Telnet

File:           Common.h

Function:       A telnet client module for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Sat 19th August 2000
                     Redesign to better move telnet processing inside
                     the module
******************************************************************************/

// Check if the header file has already been included
#ifndef  __BTT_COMMON_H
# define __BTT_COMMON_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

// Includes
#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "swis.h"

// API Include
#include "BTT_api.h"

// Memcheck includes
#ifdef USE_MEMCHECK
# include "MemCheck:MemCheck.h"
#endif

// Hierprof includes
#ifdef HierProf_PROFILE
# include "HierProf:HierProf.h"
#endif

/** Constants ----------------------------------------------------------*/

// Config values

// Pollword values

// Defines
#define EVENTV                  0x10
#define MOUSEV                  0x1A
#define TICKERV                 0x1C
#define DISABLE_EVENT           13
#define ENABLE_EVENT            14
#define INTERNET_EVENT          19

#define TELNET_POLL_LOOP        2

/** Macros -------------------------------------------------------------*/

#ifdef DEBUG
# define UNUSED(x) (x = x)
#else
# define UNUSED(x)
#endif

#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

#define destroy(x) (if(x) {free(x); x = NULL;})

/** TypeDefines --------------------------------------------------------*/

// Boolean values
#ifndef BOOL
# define BOOL unsigned int
# define TRUE (1==1)
# define FALSE !TRUE
#endif

typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;

// Max connections
#define MAX_CONNECTIONS 32

// Size of buffers
#define BUFFER_SIZE (4 * 1024)

/** Structures -------------------------------------------------------*/

// Buffer structure
typedef struct BTBuffer_s {
  CARD32 writePos;
  CARD32 readPos;
  CARD32 size;
  CARD32 used;
  BOOL   circular;
  char *data;
} BTBuffer_t;

// Session details
typedef struct BTSession_s {
  BTT_ID       ID;

  // Flags
  CARD32 flags;

  // Status
  CARD32 status;

  // Socket
  INT32 socket;

  // SSL
  INT32 ssl;

  // IPAddress
  INT32 ipAddress;

  // Data buffers
  BTBuffer_t *read;
  BTBuffer_t *write;
  BTBuffer_t *work;
  unsigned int workProcessed;

  // Hostname, terminal, port, username
  CARD32 port;
  char *hostname;
  char *terminal;
  char *userName;

  // Pollword
  INT32 pollword;
} BTSession_t;

/** Globals ----------------------------------------------------------*/
DECLARE __swi(XOS_Bit | OS_ReadMonotonicTime) int readMonotonic(void);


DECLARE void *pollword           INIT(NULL);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_COMMON_H



