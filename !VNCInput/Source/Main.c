/******************************************************************************

Program:        VNCInput

File:           Main.c

Function:       A VNC input only client for Risc OS

Description:    Main Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Tue 16th November 1999 - Creation

******************************************************************************/

/** Includes */
#include "VNCInput.h"

/** Global variables. */
char *errorMessage;
int error, VNCFlags, pollTime, VNCTaskHandle, VNCBarIcon, riscosVersion;
int infoWindow, connectWindow, connectingWindow, passwordWindow, mainWindow, mainMenuData[32];
int screenX, screenY, xEig, yEig, *hostMenuData, resolverStatus = 0, networkStatus = 0;
BOOL doubleHeightMode = FALSE, hostOpen = FALSE;
int VNCSocket, VNCPort, *pollword = NULL;

VNCConfig_s VNCConfig;

/****************************** Procedures. *********************************/
void mainTaskDispatch(CARD16 eventType, int *event) {
  /** Work out which WIMP event has occured. */
  switch(eventType) {
    /** Poll word activation. */
    case eventNONZEROPOLLWORD:
      if(pollword[POLL_POLLWORD] & POLL_MOUSE_WATCH) {
        VNCProcessMouse();
      }

      if(pollword[POLL_POLLWORD] & POLL_PRESSED) {
        VNCProcessKey((pollword[POLL_POLLWORD] & POLL_PRESSED) >> 16, 1);
      }

      if(pollword[POLL_POLLWORD] & POLL_RELEASED) {
        VNCProcessKey((pollword[POLL_POLLWORD] & POLL_RELEASED) >> 24, 0);
      }

      if(pollword[POLL_POLLWORD] & POLL_EDGE_DETECT) {
         VNCProcessEdgeDetect();
      }

      pollword[POLL_POLLWORD] = 0;
    break;

    /** Null poll event. */
    case eventNULL:
      /** If connected, then read in socket data. */
      if(VNCFlags & FLAG_CONNECTED) {
        socketFillBuffers();

        /** Are we still initialising. */
        if(!(VNCFlags & FLAG_INITIALISED)) {
          /** Process input. */
          socketProcessInitInput();

          /** Set polltime. */
          pollTime = 5;
        }
        else
          pollTime = 50;
      }
      else
        pollTime = 50;

      /** Update poll time. */
      pollTime += _swi(OS_ReadMonotonicTime, _RETURN(0));
    break;

    /** Handles mouse clicks. */
    case eventBUTTON:buttonClick(event);
    break;

    /** Handles menu selections. */
    case eventMENU:buttonMenuClick(event);
    break;

    /** Handles key presses. */
    case eventKEY:buttonKeyPress(event);
    break;

    /** Handle window open events. */
    case eventOPEN:_swix(Wimp_OpenWindow, _IN(1), &event[0]);
    break;

    /** Handle window closing events. */
    case eventCLOSE:_swix(Wimp_CloseWindow, _IN(1), &event[0]);
    break;

    /** Handles User messages. */
    case eventUSERMESSAGE:
    case eventUSERMESSAGERECORDED:
      switch(event[MESSAGE_CODE]) {
        /** Quit request. */
        case messageQUIT:VNCFlags |= FLAG_QUIT;
        break;

        /* Handles Mode Change */
        case messageMODECHANGE:windowModeChange();
        break;

        /** Default message. */
        default:
        break;
      }
    break;

    /** Ignore all other events. */
    default:
    break;
  }
}

BOOL mainInitialise(void) {
  /** Sets up any resources or values required. */
  int messages[3];
  _kernel_oserror *error;

  /** Check if we are already loaded. */
  if(wimpEnumerateTasks(VNCTaskName))
    return FALSE;

  /** Set up messages we want to receive. */
  messages[0] = messagePREQUIT;
  messages[1] = messageMODECHANGE;
  messages[2] = 0;

  /** Attempts to initialise tasks. */

  if((error = _swix(Wimp_Initialise, _INR(0, 3) | _OUT(1), 310, 0x4B534154,
                    (int)VNCTaskName, (int)messages, &VNCTaskHandle)) != NULL)
  {
     mainReportError("Failed to call wimp_initialise .", ERROR_INIT, 0);
    return FALSE;
  }

  /** Check what version of RISC OS we are using. */
  riscosVersion = _swi(OS_Byte, _INR(0, 2) | _RETURN(0), 129, 0, 0xFF);

  /** Setup exit handler. */
  if(atexit(mainVNCTaskExit))  {
     mainReportError("Failed to to register atexit program.", ERROR_INIT, 0);
    return FALSE;
  }


  /** Create iconbar */
  VNCBarIcon = windowCreateIconbar("!VNCInput", -1);

  /** Initialises windows. */
  if(!windowInit("<VNCInput$Dir>.Resources.Templates")) {
      mainReportError("Failed to to reinit windowsister atexit program.", ERROR_INIT, 0);

    return FALSE;
  }

  /** Creates the main menu. */
  windowCreateMainMenu();

  /** Create the hosts menu. */
  buttonLoadMenu();

  /** Sets up various mode values. */
  windowModeChange();

  /** Setup pollword. */
  if(!mainSetupPollWord()) {
     mainReportError("Failed to to setup pollword program.", ERROR_INIT, 0);

    return FALSE;
  }

  /** Success. */
  return TRUE;
}

void mainVNCTaskExit(void) {
#ifdef VNC_MemCheck
  MemCheck_OutputBlocksInfo();
#endif

  /** Closes down this wimp task. */
  _swix(Wimp_CloseDown, _INR(0, 2), VNCTaskHandle, 0x4B534154, NULL);
}

int mainReportError(char *errorStr, int errorNum, int flags) {
  /** Reports an error. Return value depends on button pressed. */
  int errorBlock[65];

  /** Set up block. */
  errorBlock[0] = errorNum;
  sprintf((char *)&errorBlock[1], "%s\0", errorStr);

  /** Generate error. */
  return _swi(Wimp_ReportError, _INR(0,2) | _RETURN(1), (int)errorBlock, flags, (int)VNCTaskName);
}

BOOL mainSetupPollWord(void) {
  /** Sets up the poll word. */

  /** Gets modules private word address. */
  pollword = (int *)_swi(OS_Module, _INR(0,1) | _RETURN(4), 18, (int)VNCTaskName);
  if(pollword == 0)
    return FALSE;

  /* Memcheck function call... Marks block as legal */
#ifdef VNC_MemCheck
  MemCheck_RegisterMiscBlock((void *)regs[4], (sizeof(int) * 10));
#endif

  /** Sets up initial configuration. */
  pollword[POLL_CONFIG] = 0;
  pollword[POLL_POLLWORD] = 0;

  /** Returns successfully. */
  return TRUE;
}

int main(void) {
  /** Start of program. */
  int eventBlock[64];

  /* Sets up MemCheck stuff */
# ifdef VNC_MemCheck
  MemCheck_Init();
  MemCheck_InterceptSCLStringFunctions();
  MemCheck_SetStoreMallocFunctions(1);
# endif

  /* Sets up HierProf stuff */
# ifdef HierProf_PROFILE
  HierProf_ProfileAllFunctions();
#endif

  /** Perform program initialisation. */
  if(mainInitialise()) {
    /** The main event loop. */
    while(!(VNCFlags & FLAG_QUIT)) {
      /** Calls wimp poll. */
      _swix(Wimp_PollIdle, _INR(0, 3), WIMP_MASK, (int)eventBlock, pollTime, (int)&pollword[POLL_POLLWORD]);
      mainTaskDispatch(WIMP_MASK, eventBlock);
    }
  }
  else {
    /** Report a wimp error. */
    mainReportError("Failed to initialise program.", ERROR_INIT, 0);
    return 1;
  }

  /** Turns off pollword support. */
  if(pollword != NULL) {
    pollword[POLL_CONFIG] = 0;
    pollword[POLL_POLLWORD] = 0;
  }

  /** Closes any sockets. */
  VNCCloseConnection();

  /** Return success. */
  return 0;
}
