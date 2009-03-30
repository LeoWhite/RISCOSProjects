/******************************************************************************

Program:        BeyondThought Telnet

File:           Sessions.c

Function:       A telnet client module for RISCOS

Description:    Routines to create and manage sessions.

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Mon 17th July 2000
                     Creation.
                0.02 Fri 15th September 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module.

******************************************************************************/

/** Includes */
#include "assert.h"
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "BTT_session.h"
#include "BTT_buffer.h"
#include "BTT_socket.h"
#include "BTT_telnet.h"
#include "BTT_utils.h"

#include "BTDebug:BT_debug.h"

/** Procedure prototypes */
static BTSession_t *sessionGetFree(void);

/** Local variables */
static CARD32 nextSession = 0;

/** Procedures ------------------------------------------------------------*/

/**
 * sessionCreate
 *
 * Finds a free session, allocates memory buffers etc.
 *
 * Returns pointer to session or NULL if failed.
 */
BTSession_t *sessionCreate(char *hostname, INT32 port, CARD32 flags, char *terminal, char *userName) {
  BTSession_t *session = NULL;

  DBUGF("sessionCreate: Attempting to create new sesson.");

  /** Attempt to find a free sesson */
  if((session = sessionGetFree()) != NULL) {
    DBUGF("sessionCreate: Successfully found a session %d", session->ID);

    /** Create buffers. */
    if(((session->read  = bufferCreate(BUFFER_SIZE, FALSE)) != NULL) &&
       ((session->write = bufferCreate(BUFFER_SIZE, FALSE)) != NULL) &&
       ((session->work  = bufferCreate(BUFFER_SIZE, FALSE)) != NULL))
    {
      DBUGF("sessionCreate: Created buffers");

      /** Ensure flags are clear */
      session->flags  = flags;
      session->socket = -1;

      /** Copy hostname */
      if(hostname) {
        char *temp = NULL;

        /** Does it start with a scheme? */
        if((temp = strstr(hostname, ":\\")) != NULL) {
          hostname = (temp + 3);
        }
      }

      session->hostname = strdup(hostname ? hostname : "127.0.0.1");

      /** Copy terminal */
      session->terminal = strdup(terminal ? terminal : "vt100");

      /** Copy username */
      session->userName = userName ? strdup(userName) : NULL;

      /** Extract port */
      if(port < 0) {
        session->port = 23;
      }
      else {
        session->port = port;
      }

      /** Clear pollword */
      session->pollword = 0;

      /** Did everything get allocated? */
      if(session->hostname && session->terminal &&
         ((userName == NULL && session->userName == NULL) || (userName != NULL && session->userName != NULL))) {
        DBUGF("sessionCreate: Successfully created session, setting to resolving");
        session->status = BTT_STATUS_RESOLVING;
      }
      else {
        DBUGF("sessionCreate: Failed to copy hostname or terminal");

        sessionDestroy(session);
        session = NULL;
      }
    }
    else {
      DBUGF("sessionCreate: Failed to fcreate buffers");
      sessionDestroy(session);
      session = NULL;
    }
  }

  DBUGF("sessionCreate: Returning sesson %p", session);

  /** Return session */
  return session;
}

/**
 * sessionDestroy
 *
 * Frees up buffers, resets session details and marks as free again.
 *
 * session - The session to destroy
 */
void sessionDestroy(BTSession_t *session) {
  assert(session);

  /** Is there a socket to close? */
  if(session->socket != -1) {
    /** Close the socket */
    FD_CLR(session->socket, &connected);
    socketClose(session);
  }

  /** Free buffers */
  if(session->read) {
    bufferDestroy(session->read);
  }
  if(session->write) {
    bufferDestroy(session->write);
  }
  if(session->work) {
    bufferDestroy(session->work);
  }

  /** Free hostname and terminal */
  if(session->hostname) {
    free(session->hostname);
  }
  if(session->terminal) {
    free(session->terminal);
  }
  if(session->userName) {
    free(session->userName);
  }

  /** Wipe the rest of the structure */
  memset(session, 0, sizeof(BTSession_t));

  /** Set ID and socket to -1 */
  session->ID = -1;
  session->socket = -1;
}

/**
 * sessionGetFree
 *
 * Searches through the sessions for a free one.
 *
 * Returns Pointer to session of NULL if none available
 */
static BTSession_t *sessionGetFree(void) {
  int i = 0;
  BTSession_t *session = NULL;

  DBUGF("sessionGetFree: Searching from 0 to %d", MAX_CONNECTIONS);

  /** Search through sessions */
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if(sessions[i]->ID == -1) {
      /** Found a free session */
      sessions[i]->ID = nextSession++;
      session = sessions[i];
      DBUGF("sessionGetFree: Found a free session, ID = %d", sessions[i]->ID);

      i = MAX_CONNECTIONS;
    }
    else {
      DBUGF("sessionGetFree: Checked session %d", sessions[i]->ID);
    }
  }

  DBUGF("sessionGetFree: Returning session %p", session);

  /** Return session. */
  return session;
}

/**
 * sessionFind
 *
 * Checks if the ID passed in points to a valid sesson.
 *
 * ID - The ID to search for.
 *
 * Returns a pointer to the session, or NULL.
 */
BTSession_t *sessionFind(BTT_ID ID) {
  int i;
  BTSession_t *result = NULL;

  assert(ID >= 0 && ID < nextSession);

  /** Search through sessions. */
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if(sessions[i]->ID == ID) {
      result = sessions[i];
      i = MAX_CONNECTIONS;
    }
  }

  /** Return result. */
  return result;
}

/**
 * sessionFindBySocket
 *
 * Looks for the session that owns this socket.
 *
 * socket - The socket to find the session for.
 *
 * Returns NULL if not found, pointer to the session otherwise.
 */
BTSession_t *sessionFindBySocket(INT32 socket) {
  int i;
  BTSession_t *result = NULL;

  assert(socket >= 0);

  /** Search through sessions. */
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if(sessions[i]->socket == socket) {
      result = sessions[i];
      i = MAX_CONNECTIONS;
    }
  }

  /** Return result. */
  return result;
}

/**
 * sessionInitialise
 *
 * Sets up sessions ready for use.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */
BOOL sessionInitialise(void) {
  BOOL failed = FALSE;

  assert(sessions == NULL);

  /** Attempt to allocat memory for sessions */
  if((sessions = calloc(MAX_CONNECTIONS, sizeof(BTSession_t *))) != NULL) {
    int i;

    /** Allocate room for each sesson. */
    for(i = 0; i < MAX_CONNECTIONS; i++) {
      if((sessions[i] = calloc(1, sizeof(BTSession_t))) == NULL) {
        failed = TRUE;
      }
      else {
        /** Set up values */
        sessions[i]->ID     = -1;
        sessions[i]->socket = -1;
      }
    }
  }

  /** Were we successful? */
  if(failed) {
    sessionFinalise();
  }

  /** return result */
  return !failed;
}

/**
 * sessionFinalise
 *
 * Frees up memory used by sessions.
 */
void sessionFinalise(void) {
  int i;

  assert(sessions);

  /** Free session data */
  for(i = 0; i < MAX_CONNECTIONS; i++) {
    if(sessions[i] != NULL) {
      if(sessions[i]->ID != -1) {
        sessionDestroy(sessions[i]);
      }
      free(sessions[i]);
    }
  }

  /** Free pointer array */
  free(sessions);
}

