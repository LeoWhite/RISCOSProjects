/******************************************************************************

Program:        BeyondThought Telnet

File:           telnet.c

Function:       A telnet client module for RISC OS

Description:    Handles multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 17th September 2000
                     Creation.

******************************************************************************/

/** Includes */
#include "assert.h"
#include "common.h"
#include "BTT_buffer.h"
#include "BTT_socket.h"
#define PROTOTYPES_INTERNAL
#include "BTT_telnet.h"
#include "BTT_utils.h"
#include "BTT_vectors.h"

/** Debug code? */
#include "BTDebug:BT_debug.h"

/** Procedure prototypes */
static BOOL telnetWillOption(BTSession_t *ptr, char *data);
static BOOL telnetWontOption(BTSession_t *ptr, char *data);
static BOOL telnetDoOption(BTSession_t *ptr, char *data);
static BOOL telnetDontOption(BTSession_t *ptr, char *data);
static BOOL telnetSubnegotiation(BTSession_t *ptr, char *data);
static BOOL telnetSEPresent(BTSession_t *ptr, char *data);
static BOOL telnetMessage(BTSession_t *ptr, char result, char option);
static char *telnetLookupOption(char option);
static BOOL telnetAuthSend(BTSession_t *ptr, char *data);
static BOOL telnetAuthReply(BTSession_t *ptr, char *data);
static BOOL telnetTTypeSend(BTSession_t *ptr, char *data);

__swi(XOS_Bit | OS_ReadMonotonicTime) unsigned int OSReadMonotonicTime(void);

/** Procedures --------------------------------------------------------------*/

/**
 * telnetWork
 *
 * Does some processing on a session.
 *
 * ptr - The session to process.
 */
BOOL telnetWork(BTSession_t *ptr) {
  unsigned int uiAvailable = bufferUsed(ptr->work);
  char *data = bufferReadPtr(ptr->work);
  assert(ptr);

  DBUGF("telnetWork: Working on session %d", ptr->ID);

  /** If a RAW buffer, just need to copy data. */
  if(ptr->flags & BTT_FLAGS_RAW) {
    ptr->workProcessed += bufferWrite(ptr->read, &data[ptr->workProcessed],
                                      uiAvailable - ptr->workProcessed);
  }
  else {
    BOOL bProcess = TRUE;
    unsigned int endTime = OSReadMonotonicTime() + TELNET_POLL_LOOP;

    /** What to do? */
    while(bProcess &&
          ptr->workProcessed < uiAvailable &&
          (ptr->status & BTT_STATUS_CONNECTED) &&
          endTime > OSReadMonotonicTime())
    {
      switch(data[ptr->workProcessed]) {
        /** If NULL, then skip it */
        case NULL:
          DBUGF("telnetWork: NULL processed");
          ptr->workProcessed++;

          // Can no longer be negotiating
          ptr->status &= ~BTT_STATUS_NEGOTIATING;
        break;

        /** Convert backspaces and deletes to same */
        case BS:
        case DEL:
          DBUGF("telnetWork: BS/DEL processed");
          if(bufferWrite(ptr->read, "\x7F", 1) == 1) {
            ptr->workProcessed++;
          }
          else {
            bProcess = FALSE;
          }

          // Can no longer be negotiating
          ptr->status &= ~BTT_STATUS_NEGOTIATING;
        break;

        /** Carriage returns */
        case CR:
          /** Sufficent left in buffer? */
          if((ptr->workProcessed + 1) >= uiAvailable) {
            DBUGF("telnetWork: Not enough characters to process CR");
            bProcess = FALSE;
            break;
          }

          /** Do we treat as a carriage return or as a new line? */
          if(data[ptr->workProcessed + 1] == LF) {
            if(bufferWrite(ptr->read, "\n", 1) == 1) {
              ptr->workProcessed += 2;
              DBUGF("telnetWork: Added linefeed");
            }
            else {
              ptr->workProcessed--;
              bProcess = TRUE;
            }
          }
          else if(data[ptr->workProcessed] == 0) {
            if(bufferWrite(ptr->read, "\r", 1) == 1) {
              ptr->workProcessed += 2;
              DBUGF("telnetWork: Added CR");
            }
            else {
              bProcess = FALSE;
            }
          }
          else {
            DBUGF("telnetWork: Unknown character \'%c\' after CR", data[ptr->workProcessed]);
            ptr->workProcessed++;
          }

          // Can no longer be negotiating
          ptr->status &= ~BTT_STATUS_NEGOTIATING;
        break;

        /** Telnet configuration */
        case IAC:
          /** Need at least 2 bytes of data. */
          if((ptr->workProcessed + 1) >= uiAvailable) {
            DBUGF("telnetWork: Not enough data to process IAC,  processed = %d, available = %d",
                  ptr->workProcessed, uiAvailable);
            bProcess = FALSE;
            break;
          }

          /** event is it? */
          switch(data[ptr->workProcessed + 1]) {
            /** Server actually wants us to insert a 255 char */
            case IAC:
              if(bufferWrite(ptr->read, &data[ptr->workProcessed + 1], 1) == 1) {
                ptr->workProcessed += 2;
              }
              else {
                bProcess = FALSE;
              }
            break;

            /** Server will support this option */
            case WILL:bProcess = telnetWillOption(ptr, data);
            break;

            /** Server won't support an option */
            case WONT:bProcess = telnetWontOption(ptr, data);
            break;

            /** Request to do an option */
            case DO:bProcess = telnetDoOption(ptr, data);
            break;

            /** Request to not do an option */
            case DONT:bProcess = telnetDontOption(ptr, data);
            break;

            /** Request to perform subnegotiation */
            case SB:bProcess = telnetSubnegotiation(ptr, data);
            break;

            /** Unknown option */
            default:
              DBUGF("telnetWork: Unknown IAC option 0x%X", data[ptr->workProcessed + 1]);

              /** Ignore it */
              ptr->workProcessed += 2;
            break;
          }
        break;

        /** Default, add to buffer */
        default:
          DBUGF("telnetWork: Unknown character %c", data[ptr->workProcessed]);
          if(bufferWrite(ptr->read, (char *)&data[ptr->workProcessed], 1) == 1) {
            ptr->workProcessed++;

            // Can no longer be negotiating
            ptr->status &= ~BTT_STATUS_NEGOTIATING;
          }
          else {
            bProcess = FALSE;
          }

        break;
      }
    }
  }

  DBUGF("telnetWork: processed %d bytes of data, still used = %d", ptr->workProcessed, bufferUsed(ptr->work));

  /** If more than half the buffer has been processed, release the rest. */
  if((bufferUsed(ptr->work) == ptr->workProcessed) || (ptr->workProcessed > bufferSize(ptr->work) / 2)) {
    DBUGF("telnetWork: Draining buffer");
    bufferDrain(ptr->work, ptr->workProcessed);
    ptr->workProcessed = 0;
  }

  /** Mark socket as having data */
  if(bufferUsed(ptr->read)) {
    DBUGF("Data waiting to be read");
    ptr->status |= BTT_STATUS_INCOMING;
    ptr->pollword |= BTT_POLLWORD_READ;
  }

  /** Mark socket as having data to write*/
  if(bufferUsed(ptr->write)) {
    DBUGF("Data waiting to write");
    FD_SET(ptr->socket, &writing);
    addCallback(CALLBACK_WRITE);
    ptr->status |= BTT_STATUS_OUTGOING;
  }

  /** More work to do? */
  if(bufferUsed(ptr->work) > ptr->workProcessed) {
    DBUGF("Data waiting to be processed");
    FD_SET(ptr->socket, &working);
    addCallback(CALLBACK_WORK);
  }
  else {
    DBUGF("No data waiting, workProcessed = %d, used = %d", ptr->workProcessed, bufferUsed(ptr->work));
  }


  /** Success */
  return TRUE;
}

/**
 * telnetWillOption
 *
 * The server has declared it is willing to supprt an option.
 *
 * ptr  - The session to process
 * data - Points to read buffer
 *
 * Returns TRUE to continue processing, false if not enough data is available
 */
static BOOL telnetWillOption(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  assert(data[ptr->workProcessed] == IAC);

  /** Is there enough data to process IAC? */
  if((ptr->workProcessed + 2) < bufferUsed(ptr->work)) {
    /** Processing data. */
    DBUGF("telnetWillOption: Server willing to support [0x%X], %s", data[ptr->workProcessed + 2],
           telnetLookupOption(data[ptr->workProcessed + 2]));

    switch(data[ptr->workProcessed + 2]) {
      /** Server supports echoing */
      case TELOPT_ECHO:
        // Already disabled?
        if((ptr->status & BTT_STATUS_LOCAL_ECHO) == 0) {
          ptr->workProcessed += 3;
          bResult = TRUE;
          break;
        }

        // We're willing to let them echo
        if(telnetMessage(ptr, DO, TELOPT_ECHO)) {
          ptr->workProcessed += 3;
          bResult = TRUE;
          ptr->status &= ~BTT_STATUS_LOCAL_ECHO;
        }
      break;

      default:
        if(telnetMessage(ptr, DONT, data[ptr->workProcessed + 2])) {
          ptr->workProcessed += 3;
          bResult = TRUE;
        }
      break;
    }
  }

  /** Success */
  return bResult;
}

/**
 * telnetWontOption
 *
 * The server has declared it won't support an option
 *
 * ptr  - The session to process
 * data - Points to read buffer
 *
 * Returns TRUE if success, FALSE if not processed
 */
static BOOL telnetWontOption(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  assert(data[ptr->workProcessed] == IAC);

  /** Is there enough data to process IAC? */
  if((ptr->workProcessed + 2) < bufferUsed(ptr->work)) {
    DBUGF("telnetWontOption: Server won't support [0x%X], %s", data[ptr->workProcessed + 2],
           telnetLookupOption(data[ptr->workProcessed + 2]));

    switch(data[ptr->workProcessed + 2]) {
      /** Server won't echo */
      case TELOPT_ECHO:
        /** Are we already echoing? */
        if(ptr->status & BTT_STATUS_LOCAL_ECHO) {
          DBUGF("We are already echoing");
          // Ignore
          ptr->workProcessed += 3;
          bResult = TRUE;
          break;
        }

        // Enable echoing
        if(telnetMessage(ptr, DONT, TELOPT_ECHO)) {
          ptr->status |= BTT_STATUS_LOCAL_ECHO;

          ptr->workProcessed += 3;
          bResult = TRUE;
          DBUGF("Enabled echoing");
        }
      break;

      default:
        // Ignore the command.
        ptr->workProcessed += 3;
        bResult = TRUE;
      break;
    }
  }

  /** Return result */
  return bResult;
}

/**
 * telnetDoOption
 *
 * The server has declared it is going to do an option
 *
 * ptr  - The session to process
 * data - Points to read buffer
 *
 * Returns TRUE if success, FALSE if not processed
 */
static BOOL telnetDoOption(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  assert(data[ptr->workProcessed] == IAC);

  /** Is there enough data to process IAC? */
  if((ptr->workProcessed + 2) < bufferUsed(ptr->work)) {
    DBUGF("telnetDoOption: Server wants us to do option [0x%X], %s", data[ptr->workProcessed + 2],
           telnetLookupOption(data[ptr->workProcessed + 2]));

    switch(data[ptr->workProcessed + 2]) {
      /** Server wants us to echo */
      case TELOPT_ECHO:
        // We won't echo back to the server
        if(telnetMessage(ptr, WONT, TELOPT_ECHO)) {
         ptr->workProcessed += 3;
         bResult = TRUE;
        }
      break;

      /** Server is willing to negotiate Authentication */
      case TELOPT_AUTH:
         if(telnetMessage(ptr, WILL, TELOPT_AUTH)) {
          ptr->workProcessed += 3;
          bResult = TRUE;
        }
      break;

      /** Server is willing to negotiate terminal type */
      case TELOPT_TTYPE:
         if(telnetMessage(ptr, WILL, TELOPT_TTYPE)) {
          ptr->workProcessed += 3;
          bResult = TRUE;
        }
      break;

      default:
         if(telnetMessage(ptr, WONT, data[ptr->workProcessed + 2])) {
          ptr->workProcessed += 3;
          bResult = TRUE;
        }
      break;
    }
  }

  /** Return result */
  return bResult;
}

/**
 * telnetDontOption
 *
 * The server wants you to stop supporting an opton.
 *
 * ptr  - The session to process
 * data - Points to read buffer
 *
 * Returns TRUE if success, FALSE if not processed
 */
static BOOL telnetDontOption(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  assert(data[ptr->workProcessed] == IAC);

  /** Is there enough data to process IAC? */
  if((ptr->workProcessed + 2) < bufferUsed(ptr->work)) {
    DBUGF("telnetDontOption: Server doesn't want us to support [0x%X], %s", data[ptr->workProcessed + 2],
           telnetLookupOption(data[ptr->workProcessed + 2]));

    switch(data[ptr->workProcessed + 2]) {
      /** Server doens't want you to echo */
      case TELOPT_ECHO:
        // Fine by us...
        ptr->workProcessed += 3;
        bResult = TRUE;
      break;

      default:
        // Ignore it
        ptr->workProcessed += 3;
        bResult = TRUE;
      break;
    }
  }

  /** Return result */
  return bResult;
}

/**
 * telnetSubnegotiation
 *
 * The server wants to sub negotiate an option.
 *
 * ptr  - The session to process
 * data - Points to the read buffer.
 */
static BOOL telnetSubnegotiation(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  assert(data[ptr->workProcessed] == IAC && data[ptr->workProcessed + 1] == SB);

  /** Is there an entire message waiting? */
  if(telnetSEPresent(ptr, data)) {

    switch(data[ptr->workProcessed + 2]) {
      case TELOPT_AUTH:
        DBUGF("telnetSubnegotiation: AUTH Subnegotiation started.");
        switch(data[ptr->workProcessed + 3]) {
          /** Being sent info */
          case SEND:bResult = telnetAuthSend(ptr, data);
          break;

          /** Got a reply messae */
          case REPLY:bResult = telnetAuthReply(ptr, data);
          break;

          default:
            DBUGF("Unknown auth state");
            bResult = TRUE;
          break;
        }
      break;

      case TELOPT_TTYPE:
        DBUGF("telnetSubnegotiation: Terminal type subnegotiation started.");
        switch(data[ptr->workProcessed + 3]) {
          /** Being sent info */
          case SEND:bResult = telnetTTypeSend(ptr, data);
          break;

          default:
            DBUGF("Unknown terminal type state");
            bResult = TRUE;
          break;
        }
      break;

      /** Skip bytes */
      default:
        while(data[ptr->workProcessed] != IAC && data[ptr->workProcessed + 1] == IAC &&
                   data[ptr->workProcessed + 2] == SE)
        {
          ptr->workProcessed++;
        }

        /** Skip final data */
        ptr->workProcessed += 3;


        /** Success */
        bResult = TRUE;
      break;
    }
  }

  /** Return result */
  return bResult;
}


/**
 * telnetSEPresent
 *
 * Looks through the work buffer seeing if
 * an SE code is present.
 *
 * ptr - The session to process
 *
 * Returns TRUE if found, FALSE otherwise.
 */
static BOOL telnetSEPresent(BTSession_t *ptr, char *data) {
  unsigned int i = 0, end = bufferUsed(ptr->work) - 1;

  /** Search through list. */
  for(i = ptr->workProcessed; i < end; i++) {
    if(data[i] == IAC && data[i + 1] == SE) {
      return TRUE;
    }
  }

  /** Failed to find it */
  return FALSE;
}

/**
 * telnetMessage
 *
 * Send a respone to an option
 *
 * ptr    - The session to process.
 * result - The result (DO, DONT, WONT, WILL);
 * option - The option to adjust.
 *
 * Returns TRUE if all successful, FALSE otherwise.
 */
static BOOL telnetMessage(BTSession_t *ptr, char result, char option) {
  char message[3] = {IAC};

  /** Fill in message */
  message[1] = result;
  message[2] = option;

  if(bufferFree(ptr->write) >= 3 && bufferWrite(ptr->write, message, 3) == 3) {
    return TRUE;
  }

  /** failed */
  return FALSE;
}

/**
 * telnetLookupOption
 *
 * Returns the name of the option passed in
 *
 * option - The option to look up
 *
 * Returns pointer to the string.
 */
static char *telnetLookupOption(char option) {
  char *result = "";
#ifdef DEBUG

  switch(option) {
    case TELOPT_ECHO:result = "Echo";
    break;

    case TELOPT_AUTH:result = "Auth";
    break;

    default:result = "Unknown";
    break;
  }

#endif /* DEBUG */

  /** Return result */
  return result;
}

/**
 * telnetAuthSend
 *
 * ptr - Session to process.
 * data - readPointer of work buffer
 *
 * return TRUE if successfulyl processed everything,
 *        FALSE if failed.
 */
static BOOL telnetAuthSend(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  unsigned int i = ptr->workProcessed + 4;
  BOOL bSSLAvailable = FALSE;
  unsigned int requiredSpace = 11;

  /** Is one of the options SSL? */
  while(data[i] != IAC && data[i+1] != SE) {
    DBUGF("Got auth pair 0x%X 0x%X", data[i], data[i+1]);
    if(data[i] == AUTH_SSL && data[i+1] == 0) {
      DBUGF("SSL Auth supported");
      if(ptr->flags & BTT_FLAGS_SSL && IsModulePresent("SecureSockets")) {
        DBUGF("Enabling SSL");
        bSSLAvailable = TRUE;
      }
    }
    i+=2;
  }

  /** Is there a username to send? */
  if(ptr->userName) {
    requiredSpace += 6 + strlen(ptr->userName) + 1;
  }

  /** Send response */
  if(bufferFree(ptr->write) >= requiredSpace) {
    /** Send username if it exists */
    if(ptr->userName) {
      DBUGF("Sending login name \"%s\"", ptr->userName);
      bufferWrite(ptr->write, "\xFF\xFA\x25\x03", 4);
      bufferWrite(ptr->write, ptr->userName, strlen(ptr->userName) + 1);
      bufferWrite(ptr->write, "\xFF\xF0", 2);
    }

    /** Send authentication reply. */
    bufferWrite(ptr->write, "\xFF\xFA\x25\x00", 4);

    if(TRUE == bSSLAvailable) {
      /** Accept SSL */
      bufferWrite(ptr->write, "\x07\x00", 2);

      /** Send AUTH_SSL_START data */
      bufferWrite(ptr->write, "\x01", 1);
    }
    else {
      /** Dont' accept, send NULL */
      bufferWrite(ptr->write, "\x00\x00", 2);
    }

    /** Send end of list */
    bufferWrite(ptr->write, "\xFF\xF0", 2);

    /** Update amount read */
    ptr->workProcessed = i + 2;

    /** Success */
    bResult = TRUE;
  }

  /** Return result */
  return bResult;
}

/**
 * telnetAuthReply
 *
 * Handles the reply of an auth request
 *
 * ptr  - The session to affect
 * data - Pointer to data.
 *
 * Returns TRUE if success, FALSE if not enough data.
 */
static BOOL telnetAuthReply(BTSession_t *ptr, char *data) {
  unsigned int i = ptr->workProcessed + 4;
  BOOL bSSLAccept = FALSE;
  DBUGF("Got an Auth Reply message");

  if(data[i] == AUTH_SSL && data[i+1] == 0) {
    DBUGF("Its an SSL reply.");

    /** Skip start of message */
    i+=2;

    /** Accept or reject? */
    switch(data[i]) {
      case AUTH_SSL_ACCEPT:
        DBUGF("Its an accept!");
        bSSLAccept = TRUE;
        ptr->status |= BTT_STATUS_CREATING_SSL;
        ptr->status &= ~BTT_STATUS_CONNECTED;
      break;

      case AUTH_SSL_REJECT:
        DBUGF("Server rejected SSL connect");
      break;

      default:
        DBUGF("Unknown Auth reply.");
      break;
    }

    /** Skip reply messgage */
    i++;
  }

  /** Skip rest of message */
  while(data[i] != IAC && data[i+1] != SE) {
    DBUGF("Skipping data");
    i+=2;
  }

  /** Update values */
  ptr->workProcessed = i + 2;

  /** Success */
  return TRUE;
}




/**
 * telnetTTypeSend
 *
 * We've been asked to send our terminal type to the remote
 * server.
 *
 * ptr - Session to process.
 * data - readPointer of work buffer
 *
 * return TRUE if successfulyl processed everything,
 *        FALSE if failed.
 */
static BOOL telnetTTypeSend(BTSession_t *ptr, char *data) {
  BOOL bResult = FALSE;
  unsigned int i = ptr->workProcessed + 4;
  unsigned int terminalLength =  strlen(ptr->terminal);

  /** Skip rest of message */
  while(data[i] != IAC && data[i+1] != SE) {
    i+=2;
  }

  /** Send response */
  if(bufferFree(ptr->write) >= (6 + terminalLength)) {
    DBUGF("Sending terminal type [%s]", ptr->terminal);

    bufferWrite(ptr->write, "\xFF\xFA\x18\x00", 4);

    /** Send terminal size. */
    bufferWrite(ptr->write, ptr->terminal, terminalLength);

    /** Send end of list */
    bufferWrite(ptr->write, "\xFF\xF0", 2);

    /** Update amount read */
    ptr->workProcessed = i + 2;

    /** Success */
    bResult = TRUE;
  }

  /** Return result */
  return bResult;
}


/**
 * telnetWrite
 *
 * ptr    - The session to add code to.
 * buffer - Pointer to buffer to add
 * length - Length of data to add
 *
 * Returns the amount of data written
 */
unsigned int telnetWrite(BTSession_t *ptr, char *buffer, unsigned int length) {
  unsigned int uiAvailable = bufferFree(ptr->write);
  unsigned int uiWritten = 0;
  assert(ptr);
  assert(buffer);
  assert(length >= 1);

  DBUGF("telnetWrite: Adding data to session %d", ptr->ID);

  /** If raw, then just write data. */
  if(ptr->flags & BTT_FLAGS_RAW) {
    uiWritten = bufferWrite(ptr->write, buffer, length);
  }
  else {
    unsigned int endTime = OSReadMonotonicTime() + TELNET_POLL_LOOP;

    while(uiAvailable &&
          uiWritten < length &&
          (ptr->status & BTT_STATUS_CONNECTED) &&
          endTime > OSReadMonotonicTime())
    {
      switch(buffer[uiWritten]) {
        case BS:
          // Echo data and send to server
          if(ptr->status & BTT_STATUS_LOCAL_ECHO && ptr->flags & BTT_FLAGS_SUPPORT_ECHO) {
            bufferWrite(ptr->read, "\x7F", 1);
          }

          bufferWrite(ptr->write, &buffer[uiWritten], 1);
          uiAvailable--;
          uiWritten++;
        break;

        // Default
        default:
          // Echo data and send to server
          if(ptr->status & BTT_STATUS_LOCAL_ECHO && ptr->flags & BTT_FLAGS_SUPPORT_ECHO) {
            bufferWrite(ptr->read, &buffer[uiWritten], 1);
          }

          bufferWrite(ptr->write, &buffer[uiWritten], 1);
          uiAvailable--;
          uiWritten++;
        break;
      }
    }
  }

  /** Anything to write? */
  if(bufferUsed(ptr->write)) {
    DBUGF("Data waiting to write");
    FD_SET(ptr->socket, &writing);
    addCallback(CALLBACK_WRITE);
    ptr->status |= BTT_STATUS_OUTGOING;
  }

  /** Return amount written */
  return uiWritten;
}


/**
 * telnetSendNegotiation
 *
 * Sends the initial state of the commands.
 *
 * ptr - The session to setup
 */
void telnetSendNegotiation(BTSession_t *ptr) {
  assert(ptr);

  /** Should we echo? */
  ptr->status |= BTT_STATUS_LOCAL_ECHO;

  /** Send we will do it */
  telnetMessage(ptr, WILL, TELOPT_ECHO);

  /** No need to send GA */
  telnetMessage(ptr, WILL, TELOPT_SGA);

  /** we support authentication */
  telnetMessage(ptr, WILL, TELOPT_AUTH);

  /** We support sending termianl info */
  telnetMessage(ptr, WILL, TELOPT_TTYPE);
}
