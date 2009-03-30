/*******************************************************************************

Program:      BeyondThought Telnet

File:	      BTT_sessions.h

Function:       A telnet client for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Mon 17th July 2000
                     Creation.
                0.02 Fri 15th September 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module.

*******************************************************************************/

/** Check if we have already been included */
#ifndef  __BTT_SESSIONS_H
# define __BTT_SESSIONS_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif


/** Constants ------------------------------------------------------*/

/** TypeDefines ----------------------------------------------------*/

/** Structures -----------------------------------------------------*/

/** Globals --------------------------------------------------------*/
DECLARE BTSession_t *sessionCreate(char *hostname, INT32 port, CARD32 flags, char *terminal, char *userName);
DECLARE void sessionDestroy(BTSession_t *session);
DECLARE BTSession_t *sessionFind(BTT_ID ID);
DECLARE BTSession_t *sessionFindBySocket(INT32 socket);
DECLARE BOOL sessionInitialise(void);
DECLARE void sessionFinalise(void);


DECLARE BTSession_t **sessions   INIT(NULL);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_SESSIONS_H
