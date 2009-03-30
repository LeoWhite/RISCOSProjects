/*******************************************************************************

Program:        BeyondThought Telnet

File:           utils.c

Function:       A telnet client module for RISC OS

Description:    Manages a number of telnet connections to and from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Mon 24th July 2000
                     Creation.
                0.02 Fri 15th September 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module.

******************************************************************************/

/** Includes */
#include "assert.h"
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "BTT_utils.h"

/** Procedures -------------------------------------------------------------*/

/**
 * strdup
 *
 * Returns a duplicate of the passed in string.
 *
 * string - The string to duplicate.
 *
 * Returns NULL if failed, otherwise pointer to the new string.
 */
char *strdup(const char *string) {
  CARD32 length = 0;
  char  *new = NULL;

  /** Check validity */
  if(string != NULL) {
    length = strlen(string);

    /** Allocate memory */
    if((new = calloc(1, length + 1)) != NULL) {
      /** Copy string */
      memcpy(new, string, length);
    }
  }

  /** Return result */
  return new;
}

/**
 * IsModulePresent
 *
 * Checks if the specified module is present.
 *
 * sName - The name of the module.
 *
 * Returns TRUE if present, FALSE otherwise.
 */
BOOL IsModulePresent(char *sName) {
  assert(sName && *sName);

  if(_swix(OS_Module, _INR(0, 1), 18, sName)) {
    return FALSE;
  }

  // Success
  return TRUE;
}

