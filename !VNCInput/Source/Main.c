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
      pollTime += swi_wimp_r0(OS_ReadMonotonicTime, NULL);
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
    case eventOPEN:swi_wimp(Wimp_OpenWindow, &event[0]);
    break;

    /** Handle window closing events. */
    case eventCLOSE:swi_wimp(Wimp_CloseWindow, &event[0]);
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
  int messages[3], regs[10];

  /** Check if we are already loaded. */
  if(wimpEnumerateTasks(VNCTaskName))
    return FALSE;

  /** Set up messages we want to receive. */
  messages[0] = messagePREQUIT;
  messages[1] = messageMODECHANGE;
  messages[2] = 0;

  /** Set up wimp init block. */
  regs[0] = 310;
  regs[1] = 0x4B534154;
  regs[2] = (int)VNCTaskName;
  regs[3] = (int)messages;

  /** Attempts to initialise tasks. */
  if(!swi_error(Wimp_Initialise, regs)) {
    /** Reads in the task handle. */
    VNCTaskHandle = regs[1];

    /** Check what version of RISC OS we are using. */
    riscosVersion = swi_fast_r0(129, 0, 0xFF, OS_Byte);

    /** Setup exit handler. */
    if(atexit(mainVNCTaskExit))
      return FALSE;

    /** Create iconbar */
    VNCBarIcon = windowCreateIconbar("!VNCInput", -1);

    /** Initialises windows. */
    if(!windowInit("<VNCInput$Dir>.Resources.Templates"))
      return FALSE;

    /** Creates the main menu. */
    windowCreateMainMenu();

    /** Create the hosts menu. */
    buttonLoadMenu();

    /** Sets up various mode values. */
    windowModeChange();

    /** Setup pollword. */
    if(!mainSetupPollWord())
      return FALSE;
  }
  else {
    /** Reports error. */
    errorMessage = swi_get_error(&error);
    mainReportError(errorMessage, error, 0);
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
  swi_fast(VNCTaskHandle, 0x4B534154, NULL, Wimp_CloseDown);
}

int mainReportError(char *errorStr, int errorNum, int flags) {
  /** Reports an error. Return value depends on button pressed. */
  int errorBlock[65];

  /** Set up block. */
  errorBlock[0] = errorNum;
  sprintf((char *)&errorBlock[1], "%s\0", errorStr);

  /** Generate error. */
  return swi_fast_r1((int)errorBlock, flags, (int)VNCTaskName, Wimp_ReportError);
}

BOOL mainSetupPollWord(void) {
  /** Sets up the poll word. */
  int regs[10];

  /** Gets modules private word address. */
  regs[0] = 18;
  regs[1] = (int)VNCTaskName;
  swi(OS_Module, regs);
  if(regs[4] != 0)
    pollword = (void *)regs[4];
  else
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
  int eventBlock[64], regs[10];

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
      regs[0] = WIMP_MASK;
      regs[1] = (int)eventBlock;
      regs[2] = pollTime;
      regs[3] = (int)&pollword[POLL_POLLWORD];
      swi(Wimp_PollIdle, regs);
      mainTaskDispatch(regs[0], eventBlock);
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
