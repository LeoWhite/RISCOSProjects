/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_utils.h

Function:       A telnet client for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Mon 24th July 2000
                     Creation.
                0.02 Fri 15th September 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module.

******************************************************************************/

// Check if header file is already included.
#ifndef  __BTT_UTILS_H
# define __BTT_UTILS_H
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

/** TypeDefines -------------------------------------------------------*/

/** Structures --------------------------------------------------------*/

/** Globals -----------------------------------------------------------*/
DECLARE char *strdup(const char *string);
DECLARE BOOL IsModulePresent(char *sName);


// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __BTT_SWIS_H
