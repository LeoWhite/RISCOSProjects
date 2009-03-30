/******************************************************************************

Program:        Telnet

File:           Common.h

Function:       A telnet client using the BTTelnet module for RISC OS

Description:    An SSL capable telnet client

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sat 29th July 2000
                     Creation.

******************************************************************************/
// Check if header file is already included.
#ifndef  __COMMON_H
# define __COMMON_H
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

// API include
#include "BTT_api.h"

/** Constants ----------------------------------------------------------*/
#define USER_NAME_SIZE 256

/** Macros -------------------------------------------------------------*/
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#define MIN(a, b)  ((a) < (b) ? (a) : (b))

/** TypeDefines --------------------------------------------------------*/

// Boolean values
#ifndef BOOL
# define BOOL unsigned char
# define TRUE (1==1)
# define FALSE !TRUE
#endif

typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;

/** Structures -------------------------------------------------------*/

/** Globals ----------------------------------------------------------*/
DECLARE BTT_ID  session       INIT(-1);
DECLARE char    hostname[256];
DECLARE char    userName[USER_NAME_SIZE];
DECLARE INT32   port          INIT(-1);
DECLARE BOOL    task_window   INIT(FALSE);
DECLARE BOOL    line_mode     INIT(FALSE);
DECLARE char    key_buffer[256];
DECLARE CARD32  key_size      INIT(0);
DECLARE BOOL    echo          INIT(FALSE);

/** Procedure prototypes */
char *getError(int error);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_COMMON_H
