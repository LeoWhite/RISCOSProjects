/******************************************************************************

Program:        BeyondThought Telnet

File:           vectors.c

Function:       A telnet client module for RISC OS

Description:    Manages a number of telnet connections to and from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Fri 15th September 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module.

******************************************************************************/

/** Includes */
#include "assert.h"
#include "common.h"
#include "BTTelnet.h"

#define INTERNAL_PROTOTYPES
#include "BTT_vectors.h"

#include "BTT_session.h"
#include "BTT_socket.h"
#include "BTT_telnet.h"

/** Debug code? */
#include "BTDebug:BT_debug.h"

/** Local variables. */
static CARD32 callbackReason = 0;
static BOOL   callbackWaiting = FALSE;
static fd_set setSockets;

/** Procedures ------------------------------------------------------------- */

/**
 * addCallafter
 *
 * Adds a callafter event, with the appropiate reasons.
 *
 * reasons - The reason for the call after.
 * time    - Time in cs to be calle after.
 */
void addCallafter(CARD32 reasons, CARD32 time) {
  assert(reasons);

  /** Add reason codes. */
  callbackReason |= reasons;

  /** If there is already a callafter or callback in progress then
      nothing more to do. */
  if(!callbackWaiting) {
    /** Add callafter */
    if(_swix(OS_CallAfter, _INR(0, 2), time, callback, pollword) == NULL) {
      callbackWaiting = TRUE;
    }
  }
}

/**
 * addCallback
 *
 * Adds a callback event, with the appropiate reasons.
 *
 * reasons - The reason for the call after.
 */
void addCallback(CARD32 reasons) {
  DBUGF("addCallback: Entered with reason %d", reasons);

  assert(reasons);


  /** Add reason codes. */
  callbackReason |= reasons;

  /** If there is already a callafter or callback in progress then
      nothing more to do. */
  if(!callbackWaiting) {
    DBUGF("addCallback: Callback added.");
    /** Add callafter */
    if(_swix(OS_AddCallBack, _INR(0, 1), callback, pollword) == NULL) {
      callbackWaiting = TRUE;
    }
  }
  else {
    DBUGF("addCallback: Callback already in progress.");
  }
}

/**
 * callbackHandler
 *
 * called under callbacks to process buffers.
 */
_kernel_oserror *callbackHandler(_kernel_swi_regs *r, void *pw) {
  static BOOL inHandler = FALSE;
  static struct timeval tv = {0, 0};
  int timeStart = 0, ready = 0, i;
  BTSession_t *ptr = NULL;
  CARD32 newCallback = 0;

  DBUGF("callbackHandler: Entered");

  /** Are we already inside a callback? */
  if(inHandler) {
    DBUGF("callbackHandler: Attempt to reenter callback!");
    return 0;
  }
  else {
    inHandler = TRUE;
  }


  timeStart = readMonotonic();


  /** Waiting on read request? */
  if(callbackReason & CALLBACK_READ) {
    DBUGF("callbackHandler: Read callback event");
    /** Clear flag. */
    callbackReason &= ~CALLBACK_READ;

    /** Copy readable sockets. */
    FD_COPY(&reading, &setSockets);

    if(_swix(Socket_Select, _INR(0, 4) | _OUT(0), maxSocket + 1, &setSockets, NULL, NULL, &tv, &ready)) {
      DBUGF("callbackHandler: Failed to select any readable sockets.");
      ready = 0;
    }

    DBUGF("callbackHandler: %d sockets ready to be read from.", ready);

    if(ready > 0) {
      for(i = 0; i <= maxSocket; i++) {
        if(FD_ISSET(i, &setSockets) && ((readMonotonic() - timeStart) <= CALLBACK_TIME)) {
        DBUGF("callbackHandler: Socket %d is ready", i);

          if((ptr = sessionFindBySocket(i)) != NULL && ptr->status & BTT_STATUS_CONNECTED) {
            DBUGF("callbackHandler: Successfully found socket");
            if(socketRead(ptr) < 0) {
              DBUGF("callbackHandler: Error occured whilst reading from socket.");
              socketClose(ptr);
            }
          }
        }
      }

      /** Did we process all events? */
      if(i <= maxSocket) {
        newCallback |= CALLBACK_READ;
      }
    }
  }

  /** Waiting on write request? */
  if(callbackReason & CALLBACK_WRITE && ((readMonotonic() - timeStart) <= CALLBACK_TIME)) {
    DBUGF("callbackHandler: Write reason code.");
    /** Clear flag. */
    callbackReason &= ~CALLBACK_WRITE;

    /** Copy readable sockets. */
    FD_COPY(&writing, &setSockets);
    if(_swix(Socket_Select, _INR(0, 4) | _OUT(0), maxSocket + 1, NULL, &setSockets, NULL, &tv, &ready)) {
      DBUGF("callbackHandler: Failed to select any writeable sockets.");
      ready = 0;
    }

    if(ready > 0) {
      for(i = 0; i <= maxSocket; i++) {
        if(FD_ISSET(i, &setSockets) && ((readMonotonic() - timeStart) <= CALLBACK_TIME)) {
          if((ptr = sessionFindBySocket(i)) != NULL && ptr->status & BTT_STATUS_CONNECTED) {
            if(socketWrite(ptr) < 0) {
              DBUGF("callbackHandler: Error occured whilst writing to socket.");
              socketClose(ptr);
            }
          }
        }
      }

      /** Did we process all events? */
      if(i <= maxSocket) {
        newCallback |= CALLBACK_WRITE;
      }
    }
  }

  /** Waiting on work? */
  if(callbackReason & CALLBACK_WORK && ((readMonotonic() - timeStart) <= CALLBACK_TIME)) {
    DBUGF("callbackHandler: Work reason code.");

    /** Clear flag. */
    callbackReason &= ~CALLBACK_WORK;

    /** Copy readable sockets. */
    FD_COPY(&working, &setSockets);

    for(i = 0; i <= maxSocket; i++) {
      if(FD_ISSET(i, &setSockets) && ((readMonotonic() - timeStart) <= CALLBACK_TIME)) {
        DBUGF("callbackHandler: Working on socket %d", i);
        if((ptr = sessionFindBySocket(i)) != NULL && ptr->status & BTT_STATUS_CONNECTED) {
          if(telnetWork(ptr) == FALSE) {
            DBUGF("callbackHandler: Error occured whilst working on socket.");
            socketClose(ptr);
          }
        }
        else {
          DBUGF("callbackHandler: Failed to find work socket");
        }
      }
    }


    /** Did we process all events? */
    if(i <= maxSocket) {
      newCallback |= CALLBACK_WORK;
    }
  }

  /** Anymore to do? */
  callbackWaiting = FALSE;

  if(newCallback || callbackReason) {
    DBUGF("callbackHandler: Still have to process 0x%X", (newCallback | callbackReason));
    addCallafter(newCallback | callbackReason, CALLBACK_TIME);
  }

  /** Leaving callback handler. */
  inHandler = FALSE;

  DBUGF("callbackHandler: Left");
  return 0;
}

/**
 * eventvHandler
 *
 * On Entry: r  = registers on entry (r0-r9)
 *           pw = module private word
 * On Exit:  Return 0 to claim event, non-0 to pass on
 *           Update r to alter return values
 */
int eventvHandler(_kernel_swi_regs *r, void *pw) {
  BTSession_t *ptr = NULL;
  int result = 1;

#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock(r, sizeof(_kernel_swi_regs));
#endif

  /** Is it the internet event? */
  if(r->r[0] == INTERNET_EVENT && FD_ISSET(r->r[2], &connected)) {
    /** Attempt to find session. */
    if((ptr = sessionFindBySocket(r->r[2])) != NULL) {

      /** Check sub readon */
      switch(r->r[1]) {
        /** Data waiting to be read. */
        case 1:
          DBUGF("eventvHandler: Socket read event on socket %d", r->r[2]);
          if(ptr->status & BTT_STATUS_CONNECTED) {
            FD_SET(ptr->socket, &reading);

            /** Add callback. */
            addCallback(CALLBACK_READ);
          }
          else {
            DBUGF("eventvHandler: Socket is not connected.");
          }

          result = 0;
        break;

        /** Urgent event (Out of bound data) */
        case 2:
        break;

        /** Connection lost. */
        case 3:
          DBUGF("eventvHandler: Connection lost.");

          /** Ensure our end is closed */
          socketClose(ptr);

          /** Mark as finished */
          ptr->status &= ~BTT_STATUS_CONNECTED;
          ptr->status |=  BTT_STATUS_CONN_LOST;
          result = 0;
        break;
      }
    }
    else {
      FD_CLR(r->r[2], &connected);
    }
  }

  /** Return result. */
#ifdef USE_MEMCHECK
  MemCheck_UnRegisterMiscBlock(r);
#endif

  return result;
  UNUSED(pw);
  UNUSED(r);
}
