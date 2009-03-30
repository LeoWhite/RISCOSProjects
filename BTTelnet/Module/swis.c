/******************************************************************************

Program:        BeyondThought Telnet

File:           swis.c

Function:       A telnet client module for RISC OS

Description:    Manages a number of telnet connections to and from this machine.

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Sun 17th September 2000
                     Redesign to move more of the telnet processing
                     code into the module.

******************************************************************************/

/** Includes */
#include "common.h"
#include "BTTelnet.h"
#include "BTT_buffer.h"
#include "BTT_session.h"
#include "BTT_socket.h"
#include "BTT_telnet.h"

/** Debug code? */
#include "BTDebug:BT_debug.h"

/** Procedures --------------------------------------------------------------*/

/**
 * swi_open
 *
 * Opens a telnet connection to the specifed host.
 *
 * On Entry:
 *
 *  R0 - NULL = Connect to localhost
 *       Else = Hostname to connect to.
 *
 *  R1 - -1   = Default telnet port
 *       >=0  = Port to connect to.
 *
 *  R2 - Flags.
 *
 *  R3 - NULL = VT100
 *       else = Terminal type
 *
 *  R4 - NULL = nothing
 *       else = User login name
 *
 *  On Exit:
 *
 *    R0 - >= 0 Resource ID
 *       - <  = Error
 *
 *     R1 = Pointer to pollword
 */
_kernel_oserror *swi_open(int number, _kernel_swi_regs *r, void *pw) {
  BTSession_t *session = NULL;
  int result;
  char *hostname = NULL, *terminal = NULL, *userName = NULL;


#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  /** Get the hostname and terminal type. */
  hostname = (char *)r->r[0];
  terminal = (char *)r->r[3];
  userName = (char *)r->r[4];

#ifdef USE_MEMCHECK
  if(hostname) {
    MemCheck_RegisterMiscBlock_String(hostname);
  }
  if(terminal) {
    MemCheck_RegisterMiscBlock_String(terminal);
  }
  if(userName) {
    MemCheck_RegisterMiscBlock_String(userName);
  }

#endif

  DBUGF("swi_open: Attempting to connect to %s:%d, terminal = \"%s\" username = \"%s\"",
        hostname ? hostname : "localhost", (r->r[1] < 0) ? 23 : r->r[1],
        terminal ? terminal : "vt100", userName ? userName : "[NULL]");


  /** Attempt to find a free session. */
  if((session = sessionCreate(hostname, r->r[1], r->r[2], terminal, userName)) != NULL) {
    DBUGF("swi_open: Successfully created sesson.");

    /** Got a free session, start connection process */
    if((result = socketUpdate(session)) >= 0) {
      /** Set ID result. */
      r->r[0] = session->ID;
      r->r[1] = (int)&session->pollword;

      DBUGF("swi_open: Successfully created session %d", session->ID);
    }
    else {
      DBUGF("swi_open: Failed to connect to remote host. Error = %d", result);

      /** Failed */
      r->r[0] = result;

      sessionDestroy(session);
    }
  }
  else {
    DBUGF("swi_open: Failed to find a free session");

    /** Failed to find a sesson. */
    r->r[0] = BTT_ERROR_NO_SESSIONS;
  }

  DBUGF("swi_open: ID = %d", r->r[0]);

#ifdef USE_MEMCHECK
  if(hostname) {
    MemCheck_UnRegisterMiscBlock(hostname);
  }
  if(terminal) {
    MemCheck_UnRegisterMiscBlock(terminal);
  }
  if(userName) {
    MemCheck_UnRegisterMiscBlock(userName);
  }

  MemCheck_UnRegisterMiscBlock(r);

#endif


  /** Finished */
  return NULL;
}

/**
 * swi_close
 *
 * Closes a connection and frees up any memory allocated to it.
 *
 * On Entry:
 *   R0 - Session ID.
 *
 * On Exit:
 *    R0 - < 0 = Error
 */
_kernel_oserror *swi_close(int number, _kernel_swi_regs *r, void *pw) {
  BTSession_t *session = NULL;

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  DBUGF("swi_close: Attempting to close session %d", r->r[0]);

  /** Is it a valid session */
  if((session = sessionFind(r->r[0])) != NULL) {
    DBUGF("swi_close: Successfully found session.");
    sessionDestroy(session);
    r->r[0] = 0;
  }
  else {
    DBUGF("swi_close: faile to find session pointer.");
    r->r[0] = BTT_ERROR_INVALID_ID;
  }

#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(r);
#endif

  /** Return success */
  return NULL;
}

/**
 * swi_read
 *
 * Reads upto the specified amount from a socket (Read buffer)
 *
 * On Entry:
 *
 *  R0 - SessionID
 *  R1 - Pointer to buffer to fill
 *  R2 - Maximum amount to read (Size of buffer)
 *
 * On Exit:
 *
 *  R0 - <  0 Error
 *       >= 0 Amount read
 */
_kernel_oserror *swi_read(int number, _kernel_swi_regs *r, void *pw) {
  BTSession_t *session = NULL;
  char *buffer = NULL;

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  DBUGF("swi_read: Attempting to read %d bytes from session %d", r->r[2], r->r[0]);

  buffer = (char *)r->r[1];
  if(buffer == NULL || r->r[2] <= 0) {
    r->r[0] = BTT_ERROR_INVALID_ARGS;
  }
  else {
#ifdef USE_MEMCHECK
    MemCheck_RegisterMiscBlock(buffer, r->r[2]);
#endif

    /** Attempt to locate session */
    if((session = sessionFind(r->r[0])) != NULL) {
      /** Are we connected yet? */
      if(!(session->status & BTT_STATUS_CONNECTED)) {
        r->r[0] = BTT_ERROR_NOT_CONNECTED;
        DBUGF("swi_read: Session is not connected");
      }
      else {
        /** Read data */
        r->r[0] = bufferRead(session->read, buffer, r->r[2], TRUE);
        DBUGF("swi_read: Read %d bytes of data", r->r[0]);

        /** Anything left? */
        if(bufferUsed(session->read) == 0) {
          /** nothing left to read */
          session->status &= ~BTT_STATUS_INCOMING;
        }
      }
    }
    else {
      DBUGF("swi_read: Failed to locate session");
      r->r[0] = BTT_ERROR_INVALID_ID;
    }
  }

#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(buffer);
  MemCheck_UnRegisterMiscBlock(r);
#endif

  /** Return success */
  return NULL;
}

/**
 * swi_write
 *
 * On Entry:
 *
 *  R0 - Session ID
 *  R1 - Pointer to buffer
 *  R2 - Amount to write
 *
 * On Exit:
 *
 *  R0 - <0 = Error
 *  R1 - >0 = Amount sent
 */
_kernel_oserror *swi_write(int number, _kernel_swi_regs *r, void *pw) {
  BTSession_t *session = NULL;
  char *buffer = NULL;

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  DBUGF("swi_write: Attempting to write %d bytes to session %d", r->r[2], r->r[0]);

  buffer = (char *)r->r[1];
  if(buffer == NULL || r->r[2] <= 0) {
    r->r[0] = BTT_ERROR_INVALID_ARGS;
  }
  else {
#ifdef USE_MEMCHECK
    MemCheck_RegisterMiscBlock(buffer, r->r[2]);
#endif

    /** Attempt to locate session */
    if((session = sessionFind(r->r[0])) != NULL) {
      /** Are we connected yet? */
      if(!(session->status & BTT_STATUS_CONNECTED)) {
        r->r[0] = BTT_ERROR_NOT_CONNECTED;
        DBUGF("swi_write: Session is not connected");
      }
      else {
        /** Write data */
        r->r[1] = telnetWrite(session, buffer, r->r[2]);
        DBUGF("swi_write: Sent %d bytes of data", r->r[1]);

        /** success */
        r->r[0] = 0;
      }
    }
    else {
      DBUGF("swi_write: Failed to locate session");
      r->r[0] = BTT_ERROR_INVALID_ID;
    }
  }

#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(buffer);
  MemCheck_UnRegisterMiscBlock(r);
#endif

  /** Success */
  return NULL;
}

/**
 * swi_status
 *
 * Returns the status of a session.
 *
 * On Entry:
 *
 *  R0 - Sesson ID
 *
 * On Exit:
 *
 *  R0 - <  0  Error
 *       >= 0  Status of session
 *  R1 - Amount waiting to be read
 *  R2 - Amount waiting to be sent
 */
_kernel_oserror *swi_status(int number, _kernel_swi_regs *r, void *pw) {
  BTSession_t *session = NULL;

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

 // DBUGF("swi_status: Attempting to get status for session %d", r->r[0]);

  /** Attempt to locate sessoin */
  if((session = sessionFind(r->r[0])) != NULL) {
    /** Update state */
    if((r->r[0] = socketUpdate(session)) >= 0) {
      /** Copy status */
      r->r[0] = session->status;
      r->r[1] = bufferUsed(session->read);
      r->r[2] = bufferUsed(session->write);
    }
  }
  else {
    DBUGF("swi_status: Failed to locate session.");
    r->r[0] = BTT_ERROR_INVALID_ID;
  }

#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(r);
#endif

  /** Return success */
  return NULL;
}

/**
 * swi_configure
 *
 * Configure a session.
 *
 * On Entry:
 *
 * R0 - Session ID
 * R1 - Configuration value
 *      Rest dependant on configuration value
 *
 * On Exit:
 *
 * R0 - <0 = Error.
 *      Rest dependant on Configuration value
 */
_kernel_oserror *swi_configure(int number, _kernel_swi_regs *r, void *pw) {

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  DBUGF("swi_configure: Called for session %d", r->r[0]);

#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(r);
#endif

  /** Return success */
  return NULL;
}
