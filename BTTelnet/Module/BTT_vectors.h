/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_vectors.h

Function:       A telnet client for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sat 16th September 2000
                     Creation.

******************************************************************************/

// Check if header file is already included.
#ifndef  __BTT_VECTORS_H
# define __BTT_VECTORS_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

/** Constants ---------------------------------------------------------*/
#define CALLBACK_READ  (1 << 0)
#define CALLBACK_WRITE (1 << 1)
#define CALLBACK_WORK  (1 << 2)

#define CALLBACK_TIME  2

/** TypeDefines -------------------------------------------------------*/

/** Structures --------------------------------------------------------*/

/** Globals -----------------------------------------------------------*/
DECLARE void addCallafter(CARD32 reasons, CARD32 time);
DECLARE void addCallback(CARD32 reasons);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_VECTORS_H
