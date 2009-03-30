/******************************************************************************

Program:        BeyondThought Telnet

File:           socket.c

Function:       A telnet client module for RISC OS

Description:    Handles multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Sun 20th August 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module

******************************************************************************/

// Includes
#include "assert.h"
#include "common.h"
#include "BTT_buffer.h"
#define  PROTOTYPES_INTERNAL
#include "BTT_socket.h"
#include "BTT_telnet.h"
#include "BTT_vectors.h"

// Debug code?
#include "BTDebug:BT_debug.h"


// Local procedures
CARD32 inet_pton(const char *hostname, INT32 *dst);

/** Procedures --------------------------------------------------------------*/

/**
 * socketResolve
 *
 * Resolves the hostname passed into the session
 *
 * ptr - Pointer to session data.
 *
 * Returns > 0 - Success
 *           0 - Waiting for resolve.
 *         < 0 - Failed to resolve.
 */
INT32 socketResolve(BTSession_t *ptr) {
  INT32 temp;
  INT32 result = -1;

  DBUGF("socketResolve: Entered");

  assert(ptr);
  assert(ptr->status & BTT_STATUS_RESOLVING);
  assert(ptr->hostname);
  assert(ptr->hostname[0] != '\0');

  DBUGF("socket_resolve: Attempting to resolve address \"%s\"", ptr->hostname);

  // Clear flag
  ptr->status &= ~BTT_STATUS_RESOLVING;

  // Is it a hostname or a numeric address?
  if(sscanf(ptr->hostname, "%i.%i.%i.%i", &temp, &temp, &temp, &temp) == 4) {
    DBUGF("socket_resolve: Hostname is in numeric format.");

    if(inet_pton(ptr->hostname, &ptr->ipAddress) == 0) {
      // Failed
      DBUGF("socket_resolve: Failed to resolve address");
      return -1;
    }

    DBUGF("socket_resolve: Successfully resolved numeric address.");

    // Move onto connection phase
    ptr->status |= BTT_STATUS_CREATING;

    // Success
    result = 1;
  }
  else {
    INT32 status = -1;
    struct hostent *he;

    DBUGF("socket_resolve: Calling resolver.");

    // resolve session
    if(_swix(Resolver_GetHost, _IN(0) | _OUTR(0, 1), ptr->hostname, &status, &he)) {
      DBUGF("socket_resolve: Failed to call resolver.");
      status = -1;
    }

    // Successfuly?
    switch(status) {
      // Lookup was successful.
      case 0:
        DBUGF("socket_resolve: Successfully resolved address.");
#       ifdef USE_MEMCHECK
        MemCheck_SetReadChecking(0);
#       endif

        // Copy address
        bcopy(he->h_addr, &ptr->ipAddress, 4);

#       ifdef USE_MEMCHECK
        MemCheck_SetReadChecking(1);
#       endif

        // Move onto connection phase
        ptr->status |= BTT_STATUS_CREATING;
        result = 1;
      break;

      // Resolving address
      case EINPROGRESS:
        DBUGF("socket_resolve: DNS call blocking");

        // Return to resolving
        ptr->status |= BTT_STATUS_RESOLVING;

        result = 0;
      break;

      // Host not found
      case -1:
      case -2:
      default:
        DBUGF("socket_resolve: Failed to resolve address");
        result = -1;
      break;
    }
  }

  // Return result
  return result;
}

/**
 * inet_pton
 *
 * Converts a hostname to its numeric address.
 *
 * hostname - The hostname to process.
 * dst      - Where to place the result
 *
 * Returns 0 if failed
 *         non-zero otherwise
 *
 */
CARD32 inet_pton(const char *hostname, INT32 *dst) {
  static const char digits[] = "0123456789";
  INT32 saw_digit = 0, octets = 0, ch;
  INT8 tmp[4], *tp;

  // Initialise pointer
  *(tp = tmp) = 0;

  while((ch = *hostname++) != '\0') {
    const char *pch;

    // Is it a valid number?
    if((pch = strchr(digits, ch)) != NULL) {
      CARD32 new = *tp * 10 + (pch - digits);

      // is it valid?
      if(new > 255)
        return 0;

      *tp = new;
      if(!saw_digit) {
        if(++octets > 4)
          return 0;
        saw_digit = 1;
      }
    }
    else if(ch == '.' && saw_digit) {
      if(octets == 4)
        return 0;
      *++tp = 0;
      saw_digit = 0;
    }
    else
      return 0;
  }

  // Read in enough?
  if(octets < 4)
    return 0;

  // Copy result
  memcpy(dst, tmp, 4);

  // return result
  return 1;
}

/**
 * socketCreate
 *
 * Attempts to create a socket
 *
 * ptr - Pointer to session details.
 *
 * Returns < 0 if failed
 *         >= 0 if success
 */
INT32 socketCreate(BTSession_t *ptr) {
  INT32 result = -1;

  assert(ptr);
  assert(ptr->status & BTT_STATUS_CREATING);

  // Clear flag
  ptr->status &= ~BTT_STATUS_CREATING;

  // Attempt to create socket
  if(_swix(Socket_Creat, _INR(0, 2) | _OUT(0), AF_INET, SOCK_STREAM, 0, &ptr->socket)) {
    // Failed to create socket
    DBUGF("socket_create: Failed to create socket");
    ptr->status |= BTT_STATUS_ABORTED;
  }
  else {
    // mark as non blocking
    INT32 flag = 1;

    if(_swix(Socket_Ioctl, _INR(0, 2), ptr->socket, FIONBIO, &flag)) {
      DBUGF("socket_create: failed to mark socket as non blocking");
      ptr->status |= BTT_STATUS_ABORTED;
    }
    else {
      // Success
      DBUGF("socket_create: Successfully created socket (%d).", ptr->socket);
      ptr->status |= BTT_STATUS_CONNECTING;
      result = 1;
    }
  }

  // Finished
  return result;
}

/**
 * socketCreateSSL
 *
 * Creates a SSL session out of an already connected session.
 *
 * ptr - The session to create an SSL session on.
 *
 * Returns >0 if successful
 *          0 if blocked
 *         <0 if error
 */
INT32 socketCreateSSL(BTSession_t *ptr) {
  INT32 result = -1;

  assert(ptr);
  assert(ptr->status & BTT_STATUS_CREATING_SSL);
  assert(ptr->socket != -1);

  // Clear flag
  ptr->status &= ~BTT_STATUS_CREATING_SSL;

  // Attempt to create an SSL context.
  if(_swix(Secure_Connect, _INR(0, 1) | _OUT(0), SSL_CLIENT_V23, ptr->socket, &ptr->ssl)) {
    DBUGF("socket_create_ssl: Failed to create SSL context");
    result = BTT_ERROR_SSL_FAILED;
  }
  else {
    DBUGF("socket_create_ssl: Successfully created SSL context");
    ptr->status |= BTT_STATUS_CONNECTING_SSL;
    result = 1;
  }

  return result;
}

/**
 * socketConnectSSL
 *
 * Attempts to establish a connection with the remote server.
 *
 * ptr - The session to connect on
 *
 * Returns >0 if success
 *          0 if blocked
 *         <- if failed
 */
INT32 socketConnectSSL(BTSession_t *ptr) {
  INT32 result = -1;
  BOOL doNegotiate = FALSE;

  assert(ptr);
  assert(ptr->ssl);
  assert(ptr->status & BTT_STATUS_CONNECTING_SSL);
  assert(ptr->socket != -1);

  // Clear flag
  ptr->status &= ~BTT_STATUS_CONNECTING_SSL;

  DBUGF("socketConnectSSL: Connecting SSL");

  // Attempt connection
  if(_swix(Secure_PollConnect, _IN(0) | _OUT(0), ptr->ssl, &result)) {
    DBUGF("socket_connect_ssl: Error occured whilst connecing");
    result = BTT_ERROR_SSL_FAILED;
  }
  else {
    if(result == -1) {
      DBUGF("socket_connect_ssl: Connection not yet established.");
      ptr->status |= BTT_STATUS_CONNECTING_SSL;
      result = 0;
    }
    else if(result != 1) {
      DBUGF("socket_connect_ssl: Unknown SSL error code.");
      result = BTT_ERROR_SSL_FAILED;
    }
    else {
      DBUGF("socket_connect_ssl: Adding socket (%d) to the list of connected sockets.", ptr->socket);
      // Move onto telnet negotiating
      if(ptr->flags & BTT_FLAGS_RAW)
        ptr->status |= (BTT_STATUS_CONNECTED | BTT_STATUS_SSL | BTT_STATUS_LOCAL_ECHO);
      else {
        if(!(ptr->status & BTT_STATUS_NEGOTIATING)) {
          doNegotiate = TRUE;
        }
        ptr->status |= (BTT_STATUS_NEGOTIATING | BTT_STATUS_CONNECTED | BTT_STATUS_SSL);
      }

      // Add to list of connected sockets.
      FD_SET(ptr->socket, &connected);
      maxSocket = MAX(maxSocket, ptr->socket);

      result = 1;

      // If we haven't negotiated yet, start
      if(doNegotiate) {
        telnetSendNegotiation(ptr);
      }
    }
  }

  DBUGF("socketConnectSSL: Leaving, result = %d", result);

  // Return result
  return result;
}

/**
 * socketClose
 *
 * Closes the socket used by the session
 *
 * ptr - the session to close.
 */
void socketClose(BTSession_t *ptr) {
  assert(ptr);

  // Is the socket open?
  if(ptr->socket != -1) {
    DBUGF("socket_close: Closing socket (%d) for session %d", ptr->socket, ptr->ID);

    // Disconnect SSL?
    if(ptr->status & BTT_STATUS_SSL && ptr->ssl) {
      if(_swix(Secure_Disconnect, _IN(0), ptr->ssl)) {
        DBUGF("socket_close: error occured when shutting down SSL connection");
      }
      ptr->ssl = NULL;
      ptr->status &= ~ BTT_STATUS_SSL;
    }

    // Shutdown the socket
    if(_swix(Socket_Shutdown, _INR(0, 1), ptr->socket, 2)) {
      DBUGF("socket_close: Error occured when sjutting down socket.");
    }

    if(_swix(Socket_Close, _IN(0), ptr->socket)) {
      DBUGF("socket_close: Error occured when closing socket");
    }

    // Mark as closed
    FD_CLR(ptr->socket, &connected);
    FD_CLR(ptr->socket, &reading);
    FD_CLR(ptr->socket, &writing);
    FD_CLR(ptr->socket, &working);
    ptr->socket = -1;
    ptr->status &= ~BTT_STATUS_CONNECTED;
    ptr->status |= BTT_STATUS_FINISHED;

    DBUGF("socket_close: Socket closed successfully.");
  }
}

/**
 * socketConnect
 *
 * Attempts to connect a socket to a remote end
 *
 * ptr - The session to connect with.
 *
 * Returns >0 if successful,
 *          0 if blocked
 *         <0 if failed.
 */
INT32 socketConnect(BTSession_t *ptr) {
  struct sockaddr_in serv_addr;
  _kernel_oserror *error;
  INT32 flag = 1;
  BOOL doNegotiate = FALSE;

  assert(ptr);
  assert(ptr->status & BTT_STATUS_CONNECTING);

  // Clear connecting flag
  ptr->status &= ~BTT_STATUS_CONNECTING;

  // Ensure structure is nulled
  bzero(&serv_addr, sizeof(serv_addr));

  // Fill in port
  DBUGF("socket_connect: Port = %d", ptr->port);
  serv_addr.sin_port = htons(ptr->port);

  // Copy across IP Address
  bcopy(&ptr->ipAddress, &serv_addr.sin_addr, 4);

  // Always do an INET connection
  serv_addr.sin_family = AF_INET;


  // Attempt connection
  if((error = _swix(Socket_Connect, _INR(0, 2), ptr->socket, &serv_addr, sizeof(serv_addr))) != NULL) {
    int result = BTT_ERROR_CONN_FAILED;

#   ifdef USE_MEMCHECK
     MemCheck_SetReadChecking(0);
#   endif

    // Failed to connect, are we blocking?
    if(error->errnum == EINPROGRESS || error->errnum == EALREADY) {
      DBUGF("socket_connect: Blocked whilst connecting.");

      // Attempt to connect again at a later date.
      ptr->status |= BTT_STATUS_CONNECTING;

      // Return blocking
      result = 0;
    }
    else if(error->errnum == EISCONN) {
      DBUGF("socket_connect: Socket is already connected!");
    }
    else {
      // Unknown error message.
      DBUGF("socket_connect: Failed to connec, error = %d, \"%s\"", error->errnum, error->errmess);

      ptr->status |= BTT_STATUS_ABORTED;

      // Was the connection refused?
      if(error->errnum == ECONNRESET)
        result = BTT_ERROR_CONN_LOST;
      else if(error->errnum == ECONNREFUSED) {
        // Server refused connetion
        result = BTT_ERROR_CONN_CLOSED;
      }
    }


    // Return if not already connected
    if(error->errnum != EISCONN) {
#   ifdef USE_MEMCHECK
       MemCheck_SetReadChecking(1);
#   endif
      return result;
    }

#   ifdef USE_MEMCHECK
       MemCheck_SetReadChecking(1);
#   endif
  }

  // mark the socket as async for Internet events
  flag = 1;
  if(_swix(Socket_Ioctl, _INR(0, 2), ptr->socket, FIOASYNC, &flag)) {
    DBUGF("socket_connect: Failed to mar socket as non-blocking.");
    ptr->status |= BTT_STATUS_ABORTED;

    // Failed
    return -1;
  }

  // Do we need to activate SSL now?
  if(ptr->flags & BTT_FLAGS_FORCE_SSL) {
    ptr->status |= BTT_STATUS_CREATING_SSL;
  }
  else {
    // Raw or telnet?
    if(ptr->status & BTT_FLAGS_RAW)
      ptr->status |= (BTT_STATUS_CONNECTED | BTT_STATUS_LOCAL_ECHO);
    else {
      if(!(ptr->status & BTT_STATUS_NEGOTIATING)) {
        doNegotiate = TRUE;
      }
      ptr->status |= (BTT_STATUS_NEGOTIATING | BTT_STATUS_CONNECTED);
    }
  }

  // Add to list of sockets
  DBUGF("session_connect: Adding socket %d to list of connected sockets", ptr->socket);
  FD_SET(ptr->socket, &connected);
  maxSocket = MAX(maxSocket, ptr->socket);

  // If we haven't negotiated yet, start
  if(doNegotiate) {
    telnetSendNegotiation(ptr);
  }

  // Success
  return 1;
}

/**
 * socketUpdate
 *
 * Updates the state of a socket.
 *
 * ptr - The session to update.
 *
 * Returns < 0 if error occured
 *         >= 0 otherwise
 */
INT32 socketUpdate(BTSession_t *ptr) {
  int result = -1;
  BOOL doNegotiate = FALSE;

  assert(ptr);

  // Have we finished?
  if(ptr->status & BTT_STATUS_FINISHED) {
    DBUGF("socket_update: Connection is finished");
    result = BTT_ERROR_NOT_CONNECTED;
  }
  else if(ptr->status & BTT_STATUS_RESOLVING) {
    DBUGF("socketUpdate: Resolving hostname \"%s\"", ptr->hostname);

    // Busy resolving
    if((result = socketResolve(ptr)) < 0) {
      DBUGF("socket_update: Failed to resolve hostname.");
      result = BTT_ERROR_DNS_FAILED;
    }
    else if(result == 0) {
      DBUGF("socket_update: DNS_resolve blocking");
    }
  }
  else if(ptr->status & BTT_STATUS_CREATING) {
    // Attempting to create the socket
    if((result = socketCreate(ptr)) < 0) {
      DBUGF("socket_update: Failed to create socket.");
      result = BTT_ERROR_CONN_FAILED;
    }
  }
  else if(ptr->status & BTT_STATUS_CONNECTING) {
    if((result = socketConnect(ptr)) < 0) {
      DBUGF("socket_update: Failed to connect to remote host.");
    }
    else if(result == 0) {
      DBUGF("socket_update: Blocked whilst connecting.");
    }
  }
  else if(ptr->status & BTT_STATUS_CREATING_SSL) {
    // Attempting to create an SSL connection

    if((result = socketCreateSSL(ptr)) < 0) {
      DBUGF("socket_update: Failed to create SSL socket.");

      if(!(ptr->flags & BTT_FLAGS_FORCE_SSL)) {
        if(ptr->flags & BTT_FLAGS_RAW)
          ptr->status |= (BTT_STATUS_CONNECTED | BTT_STATUS_LOCAL_ECHO);
        else {
          if(!(ptr->status & BTT_STATUS_NEGOTIATING)) {
            doNegotiate = TRUE;
          }
          ptr->status |= (BTT_STATUS_NEGOTIATING | BTT_STATUS_CONNECTED);
        }

        // Add to list of connected sockets
        DBUGF("session_connect_ssl: Adding socket %d to list of connected sockets", ptr->socket);
        FD_SET(ptr->socket, &connected);
        maxSocket = MAX(maxSocket, ptr->socket);

        result = 0;
      }
    }
    else if(result == 0) {
      DBUGF("socket_update: Blocked whilst SSL creating");
    }
    else {
      DBUGF("Successfully created SSL connection");
    }
  }
  else if(ptr->status & BTT_STATUS_CONNECTING_SSL) {
    // Connecting to an SSL server.
    if((result = socketConnectSSL(ptr)) < 0) {
      DBUGF("socket_update: Failed to connect SSL socket.");

      if(!(ptr->flags & BTT_FLAGS_FORCE_SSL)) {
        if(ptr->flags & BTT_FLAGS_RAW)
          ptr->status |= (BTT_STATUS_CONNECTED | BTT_STATUS_LOCAL_ECHO);
        else {
          if(!(ptr->status & BTT_STATUS_NEGOTIATING)) {
            doNegotiate = TRUE;
          }
          ptr->status |= (BTT_STATUS_NEGOTIATING | BTT_STATUS_CONNECTED);
        }

        // Add to list of connected sockets.
        DBUGF("session_connect_ssl: Adding socket %d to list of connected sockets", ptr->socket);
        FD_SET(ptr->socket, &connected);
        maxSocket = MAX(maxSocket, ptr->socket);

        result = 0;
      }
    }
    else if(result == 0) {
      DBUGF("socket_update: Blocked whilst connecting to SSL.");
    }
  }
  else {
    result = 0;
  }

  // If we haven't negotiated yet, start
  if(doNegotiate) {
    telnetSendNegotiation(ptr);
  }

  // Return result
  return result;
}

/**
 * socketWrite
 *
 * Writes data to the server.
 *
 * ptr - The session to write data fom.
 *
 * Returns < 0 if filed
 *           0 if blocked or nothing to send.
 *         > 0 amount sent
 */
INT32 socketWrite(BTSession_t *ptr) {
  INT32 available = 0, sent = 0;
  _kernel_oserror *error;

  assert(ptr);
  assert(ptr->socket >= 0);
  assert(ptr->status & BTT_STATUS_CONNECTED);

  // Remove from list of writing sockets
  FD_CLR(ptr->socket, &writing);

  // Is there anything to write?
  if((available = bufferUsed(ptr->write)) == 0) {
    DBUGF("socketWrite: Nothing to send for session %d", ptr->ID);
    return 0;
  }

  DBUGF("socketWrite: Attempting to send \"%*s\", SSL = %d.", available, bufferReadPtr(ptr->write),
        (ptr->flags & BTT_STATUS_SSL) ? 1 : 0);

  // Attempt to write data
  if((error = _swix((ptr->status & BTT_STATUS_SSL) ? Secure_Write : Socket_Write, _INR(0, 2) | _OUT(0),
                    (ptr->status & BTT_STATUS_SSL) ? ptr->ssl : ptr->socket,
                    bufferReadPtr(ptr->write), available, &sent)) != NULL)
  {
    int result;

#   ifdef USE_MEMCHECK
     MemCheck_SetReadChecking(0);
#   endif

    // Did connection block?
    if(error->errnum == EWOULDBLOCK) {
      DBUGF("socketWrite: Write would have blocked, amount sent = %d", sent);
      result = 0;
    }
    else {
      DBUGF("socketWrite: Write failed, error = %d, \"%s\"", error->errnum, error->errmess);
      result = -1;
    }

#   ifdef USE_MEMCHECK
     MemCheck_SetReadChecking(1);
#   endif

    // return
    return result;
  }

  // Check result
  if(sent < 0) {
    DBUGF("socketWrite: Sent returned -1");
    return -1;
  }

  DBUGF("socketWrite: Sent %d bytes of data out of %d", sent, available);

  // Drain buffer
  bufferDrain(ptr->write, sent);

  // Is there anything left in the buffer?
  if(bufferUsed(ptr->write) > 0) {
    DBUGF("socketWrite: More to send, setting write flag");
    FD_SET(ptr->socket, &writing);
    addCallback(CALLBACK_WRITE);
    ptr->status |= BTT_STATUS_OUTGOING;
  }
  else {
    ptr->status &= ~BTT_STATUS_OUTGOING;
  }

  // Return amount sent
  return sent;
}

/**
 * socketRead
 *
 * Reads data from the socket.
 *
 * ptr - The session to read data from.
 *
 * Returns < 0 if failed
 *           0 if blocked or nothing to read
 *         > 0 amount read.
 */
INT32 socketRead(BTSession_t *ptr) {
  INT32 available = 0, read = 0;
  _kernel_oserror *error;

  assert(ptr);
  assert(ptr->socket >= 0);
  assert(ptr->status & BTT_STATUS_CONNECTED);

  // Clear flag
  FD_CLR(ptr->socket, &reading);

  // How much room is there to read into?
  if((available = bufferFree(ptr->work)) == 0) {
    DBUGF("socketRead: No room to read data for session %d", ptr->ID);
    return 0;
  }

  DBUGF("socketRead: Attempting to read up to %d bytes, SSL = %d,write ptr = %p", available,
        (ptr->status & BTT_STATUS_SSL) ? 1 :0, bufferWritePtr(ptr->work));

  // Attempt to read data
  if((error = _swix((ptr->status & BTT_STATUS_SSL) ? Secure_Read : Socket_Read, _INR(0, 2) | _OUT(0),
                    (ptr->status & BTT_STATUS_SSL) ? ptr->ssl : ptr->socket,
                    bufferWritePtr(ptr->work), available, &read)) != NULL)
  {
    int result;

#   ifdef USE_MEMCHECK
     MemCheck_SetReadChecking(0);
#   endif

    // Did connection block?
    if(error->errnum == EWOULDBLOCK) {
      DBUGF("socketRead: Read would have blocked, amount read = %d", read);
      result = 0;
    }
    else {
      DBUGF("socketRead: Read failed, error = %d, \"%s\"", error->errnum, error->errmess);
      result = -1;
    }

#   ifdef USE_MEMCHECK
     MemCheck_SetReadChecking(1);
#   endif

    // return
    return result;
  }

  if(read < 0) {
    DBUGF("socketRead: Read returned %d", read);
    return -1;
  }

  DBUGF("socketRead: Read %d bytes of data out of %d", read, available);

  // Eat the data.
  bufferConsume(ptr->work, read);


  /** Some processing to do now. */
  if(read > 0) {
    // Check to see if more is waiting
    FD_SET(ptr->socket, &reading);
    addCallback(CALLBACK_READ);

    FD_SET(ptr->socket, &working);
    addCallback(CALLBACK_WORK);
  }

  // Return amount read
  return read;
}


