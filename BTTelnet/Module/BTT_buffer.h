/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_buffer.h

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
#ifndef  __BTT_BUFFER_H
# define __BTT_BUFFER_H
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

/** Macros -------------------------------------------------------------*/

/** TypeDefines --------------------------------------------------------*/

/** Structures -------------------------------------------------------*/

/** Globals ----------------------------------------------------------*/

BTBuffer_t *bufferCreate(CARD32 size, BOOL circular);
void bufferDestroy(BTBuffer_t *buffer);
char *bufferReadPtr(BTBuffer_t *buffer);
char *bufferWritePtr(BTBuffer_t *buffer);
CARD32 bufferDrain(BTBuffer_t *buffer, CARD32 amount);
CARD32 bufferConsume(BTBuffer_t *buffer, CARD32 amount);
CARD32 bufferWrite(BTBuffer_t *buffer, char *data, CARD32 size);
CARD32 bufferRead(BTBuffer_t *buffer, char *data, CARD32 size, BOOL drain);
CARD32 bufferSize(BTBuffer_t *buffer);
CARD32 bufferFree(BTBuffer_t *buffer);
CARD32 bufferUsed(BTBuffer_t *buffer);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_BUFFER_H



