/******************************************************************************

Program:        BeyondThought Telnet

File:           main.c

Function:       A telnet client module for RISC OS

Description:    Handles multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Sun 13th August 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module

******************************************************************************/

// Includes
#define  PROTOTYPES_INTERNAL
#include "common.h"
#include "BTTelnet.h"
#include "BTT_session.h"
#include "BTT_socket.h"
#include "BTT_utils.h"

// Debug code
#include "BTDebug:BT_debug.h"


// Local procedures

/** Procedures --------------------------------------------------------------*/

/**
 * claimEvents
 *
 * Claims the events required by the module
 *
 * Return TRUE if successful, FALSE otherwise
 */
static BOOL claimEvents(void) {
  BOOL result = TRUE;

  /** Claim event vector */
  if(_swix(OS_Claim, _INR(0, 2), EVENTV, eventv, pollword))
    result = FALSE;

  /** Claim internet event */
  if(result && _swix(OS_Byte, _INR(0, 1), ENABLE_EVENT, INTERNET_EVENT))
    result = FALSE;


  /** Success */
  return result;
}

/**
 * releaseEvents
 *
 * Releases any events claimed by the module
 *
 * Returns TRUE if all finished successfully, FALSE otherwise
 */
static BOOL releaseEvents(void) {
  BOOL result = TRUE;

  /** Disable internet event */
  if(_swix(OS_Byte, _INR(0, 1), DISABLE_EVENT, INTERNET_EVENT))
    result = FALSE;

  /** Release Events */
  if(_swix(OS_Release, _INR(0, 2), EVENTV, eventv, pollword))
    result = FALSE;

  /** Success */
  return result;
}

/**
 * initialise
 *
 * Called to set up the module ready for use.
 *
 * Returns NULL if successful, error otherwise
 */
_kernel_oserror *initialise(const char *tail, int poduleBase, void *pw) {
  // Set up MemCheck code
# ifdef USE_MEMCHECK
  MemCheck_Init();
  MemCheck_RedirectToFilename(":4.$.BTTelnetLog");
  MemCheck_InterceptSCLStringFunctions();
  MemCheck_SetStoreMallocFunctions(1);
# endif

  // Take a copy of the pollword
  pollword = pw;

  // Start up debug session
  DBUG_START("BTTelnet", 80, 1024);

  // Chek for the presence of an Internet module
  if(!IsModulePresent("Internet")) {
    DBUGF("initialise: Internet module not found");
    return failedInternet;
  }

  /** Ensure the list of connect sockets is clear */
  FD_ZERO(&connected);
  FD_ZERO(&writing);
  FD_ZERO(&reading);

  // Claim sessions
  if(!sessionInitialise()) {
    DBUGF("initialise: Failed to initialise sessions.");
    return failedMemory;
  }

  // Claim events
  if(!claimEvents()) {
    releaseEvents();
    DBUGF("initialise: Failed to claim events.");
    return failedClaim;
  }

  // Success
  DBUGF("initialise: Initialised successfully.");
  return NULL;

  // Stop compiler warnings
  UNUSED(poduleBase);
  UNUSED(tail);
}

/**
 * finalise
 *
 * Releases events, frees memory and closes connections
 *
 * Returns NULL if successful, otherwise returns pointer to error.
 */
_kernel_oserror *finalise(int fatal, int poduleBase, void *pw) {
  // Free events
  releaseEvents();

  // Release sessions
  sessionFinalise();

  // Stop debugging
  DBUGF("finalise: Finalised successfully");
  DBUG_CLOSE();

  // Finalise memcheck
# ifdef USE_MEMCHECK
  MemCheck_OutputBlocksInfo();
  MemCheck_Finalise();
# endif

  // Return success
  return NULL;
}

