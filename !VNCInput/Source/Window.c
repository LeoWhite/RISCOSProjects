/******************************************************************************

Program:        VNCInput

File:           window.c

Function:       A VNC Input only Client for Risc OS

Description:    window Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Thu 18th November 1999 - Creation

******************************************************************************/

/** Includes. */
#include "VNCInput.h"

/*****************************************************************************/
int windowCreateIconbar(char *spriteName, int position) {
  /** Creates the bar icon, and put it on the icon bar. */
  int *iconBlock, iconHandle;

  /** Allocates memory. */
  iconBlock = (int *)malloc(33);

  /** Sets up icon details. */
  iconBlock[0] = position;
  iconBlock[1] = iconBlock[2] = 0;
  iconBlock[3] = iconBlock[4] = 68;
  iconBlock[5] = 0x3002;
  sprintf((char *)&iconBlock[6], "%s\0", spriteName);

  /** Get the icon handle. */
  if(_swix(Wimp_CreateIcon, _IN( 1) | _OUT(0), iconBlock, &iconHandle)) {
    // Failed
    return -1;
  }

  /** Free up memory. */
  free(iconBlock);

  /** Return iconhandle. */
  return iconHandle;
}

BOOL windowInit(char *filePath) {
  /** Variables t hold the buffer size. */
  CARD16 bufferSize, tempSize;
  char *indirectBuffer = NULL, *templateBuffer = NULL;
  int regs[10];
  _kernel_oserror *error;

  /** Checks there is no memory already allocated. */
  if(templateBuffer != NULL)
    free(templateBuffer);

  /** Open the template file. */
  if((error = _swix(Wimp_OpenTemplate, _INR(0, 2), 0, (int)filePath, NULL)) != NULL)
  {
    /** Reports the error. */
    mainReportError(error->errmess, error->errnum, 0);
    return FALSE;
  }

  /** Calculates the indirect buffer size required. */
  bufferSize = windowCalculateBuffer("Info", 0);
  tempSize   = windowCalculateBuffer("Connect", 0);
  if(tempSize > bufferSize)
    bufferSize = tempSize;
  tempSize   = windowCalculateBuffer("Connecting", 0);
  if(tempSize > bufferSize)
    bufferSize = tempSize;
  tempSize   = windowCalculateBuffer("Password", 0);
  if(tempSize > bufferSize)
    bufferSize = tempSize;
  tempSize   = windowCalculateBuffer("Main", 0);
  if(tempSize > bufferSize)
    bufferSize = tempSize;

  /** Allocate memory. */
  indirectBuffer = (char *)malloc(bufferSize * 8);

  /** Calculate buffer size. */
  bufferSize  = windowCalculateBuffer("Info", 1);
  bufferSize += windowCalculateBuffer("Connect", 1);
  bufferSize += windowCalculateBuffer("Connecting", 1);
  bufferSize += windowCalculateBuffer("Password", 1);
  bufferSize += windowCalculateBuffer("Main", 1);

  /** Allocates memory. */
  templateBuffer = (char *)malloc(bufferSize);

  /** Checks if the memory was allocated successfully. */
  if(indirectBuffer == NULL || templateBuffer == NULL) {
    mainReportError("Failed to allocate memory for templates.", ERROR_TEMPLATE, 0);
    return FALSE;
  }

  /** Sets up the buffer pointers */
  regs[1] = (int)indirectBuffer;
  regs[2] = (int)templateBuffer;
  regs[3] = (int)(templateBuffer + bufferSize);

  /** Creates the window. */
  infoWindow       = windowCreate("Info", regs);

  connectWindow    = windowCreate("Connect", regs);
  connectingWindow = windowCreate("Connecting", regs);
  passwordWindow   = windowCreate("Password", regs);
  mainWindow       = windowCreate("Main", regs);

  /** Sets up info window. */
  windowIconSetText(infoWindow, INFO_NAME, VNCTaskName);
  windowIconSetText(infoWindow, INFO_PURPOSE, "VNC Input client");
  windowIconSetText(infoWindow, INFO_AUTHOR, "Leo White");
  windowIconSetText(infoWindow, INFO_VERSION, VNCVersion);

  /** Ensures the password is cleared. */
  sprintf(indirectBuffer, "\0\0\0\0\0\0\0\0");
  windowIconSetText(passwordWindow, PASSWORD_INPUT, indirectBuffer);

  /** Frees up indirect buffer. */
  free(indirectBuffer);

  /** Closes the template buffer. */
  _swix(Wimp_CloseTemplate, _IN(1), NULL);

  /** Successfully completed. */
  return TRUE;
}

int windowCalculateBuffer(char *name, BOOL bufferType) {
  // Returns the size of buffer required to hold the specified window.

  // Checks which buffer size is required.
  if(bufferType)
    // template buffer
    return _swi(Wimp_LoadTemplate, _INR(0, 1) | _INR(4, 6) | _RETURN(1), 0, 0, -1, name, 0);
  else
    // Indirect buffer size
    return _swi(Wimp_LoadTemplate, _INR(0, 1) | _INR(4, 6) | _RETURN(2), 0, 0, -1, name, 0);
}

int windowCreate(char *name, int *regs) {
  regs[4] = -1;
  regs[5] = (int)&name[0];
  regs[0] = regs[6] = 0;

  /** Creates the specified window. */
  _swix(Wimp_LoadTemplate, _INR(1, 6) | _OUT(1), regs[1], regs[2], regs[3], regs[4], regs[5], regs[6],
                                                &regs[1]);

  /** Creates the window and returns the window handle. */
  return _swi(Wimp_CreateWindow, _IN(1) | _RETURN(0), regs[1]);
}

char *windowIconGetText(CARD32 handle, CARD16 icon) {
  /** Returns a pointer to the text held in an icon. */
  int iconBlock[10];

  /** Gets icon Info. */
  windowIconGetState(handle, icon, iconBlock);

  /** Checks if icon is indirected. */
  if(iconBlock[6] & (1 << 8))
    return (char *)iconBlock[7];
  else
    return NULL;
}

void windowIconSetText(CARD32 handle, CARD16 icon, char *text) {
  /** Sets the text in an icon. */
  int iconBlock[10];

  /** Checks if a string was actually passed. */
  if(text == NULL)
    return;

  /** Gets icon info. */
  windowIconGetState(handle, icon, iconBlock);

  /** Checks if the icon is indirected. */
  if(iconBlock[6] & 1 << 8) {
    /** Copies the text. */
    strncpy((char *)iconBlock[7], text, iconBlock[9] - 1);

    /** Redraws the icon. */
    windowIconSetState(handle, icon, 0, 0);
  }
}

int windowIconGetInteger(CARD32 handle, CARD16 icon) {
  /** Returns the numeric contents of an icon. */
  return(atoi(windowIconGetText(handle, icon)));
}

void windowIconSetInteger(CARD32 handle, CARD16 icon, int newContent) {
  /** Converts number into a string. */
  char string[13];

  sprintf(string, "%i", newContent);
  windowIconSetText(handle, icon, string);
}

void windowIconGetState(CARD32 handle, CARD16 icon, int *block) {
  /** Gets the icon state. */
  block[0] = handle;
  block[1] = icon;
  _swix(Wimp_GetIconState, _IN(1), block);
}

void windowIconSetState(CARD32 handle, CARD16 icon, CARD32 EORWord, CARD32 clearWord) {
  _swix(Wimp_SetIconState, _BLOCK(1), handle, icon, EORWord, clearWord);
}

void windowCreateMainMenu(void) {
  /** Creates the main menu. */
  char *menuSpace;
  int *item;

  /** Sets up the pointer. */
  menuSpace = (char *)&mainMenuData[0];

  /** Sets up the menu header. */
  strncpy(menuSpace, VNCTaskName, 8);
  menuSpace[12] = 7;
  menuSpace[13] = 2;
  menuSpace[14] = 7;
  menuSpace[15] = 0;
  menuSpace[16] = 10 * 16 + 32;
  mainMenuData[5] = 44;
  mainMenuData[6] = 1;

  /** Data for info item. */
  item = &mainMenuData[7];
  item[0] = 0;
  item[1] = infoWindow;
  item[2] = MENU_UNSHADED;
  strcpy((char *)&item[3], "Info");

  /** Data for connect item. */
  item = &mainMenuData[13];
  item[0] = 0;
  item[1] = NULL;
  item[2] = MENU_UNSHADED;
  strcpy((char *)&item[3], "Connect...");

  /** Data for disconnect item. */
  item = &mainMenuData[19];
  item[0] = 0;
  item[1] = NULL;
  item[2] = MENU_SHADED;
  strcpy((char *)&item[3], "Disconnect");

  /** Data for quit item. */
  item = &mainMenuData[25];
  item[0] = 0x80;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char *)&item[3], "Quit");
}

void windowOpen(CARD32 handle, CARD8 position) {
  /** Opens a window in the specified position. */
  int ptr[5], state[10];

  /** Gets the window state. */
  state[0] = handle;
  _swix(Wimp_GetWindowState, _IN(1), state);

  /** Checks if the window should be centered. */
  if(position & WINDOW_CENTERED) {
    /** Gets X and Y sizes of the screen. */
    ptr[0] = screenX << xEig;
    ptr[1] = screenY << yEig;

    /** Gets center point, by dividing by 2. */
    ptr[0] = ptr[0] >> 1;
    ptr[1] = ptr[1] >> 1;

    /** Calculates window width and height. */
    ptr[2] = state[3] - state[1];
    ptr[3] = state[4] - state[2];

    /** Updates state. */
    state[3] = ptr[0] + (ptr[2] >> 1);
    state[1] = state[3] - ptr[2];
    state[2] = ptr[1] - (ptr[3] >> 1);
    state[4] = state[2] + ptr[3];
  }

  /** Checks if it should be positioned under the pointer. */
  if(position & WINDOW_UNDER_PTR) {
    /** Gets pointer info. */
    _swix(Wimp_GetPointerInfo, _IN(1), ptr);

    /** Adjusts window position. */
    state[3] = state[3] - state[1] + ptr[0];
    state[1] = ptr[0] - 30;
    state[2] = state[2] - state[4] + ptr[1];
    state[4] = ptr[1] + 30;
  }

  /** Checks if should be opened full size. */
  if(position & WINDOW_FULL_SCREEN) {
    /** Sets min x and y. */
    state[1] = screenX << xEig;
    state[2] = 0;
    state[3] = (VNCConfig.width << xEig) + state[1];
    state[4] = (VNCConfig.height << yEig);
    state[5] = 0;
    state[6] = 0;
  }

  /** Checks if the window should appear on top. */
  if(position & WINDOW_ON_TOP)
    state[7] = -1;

  /** Checks if the window should appear on top. */
  if(position & WINDOW_ON_BOTTOM)
    state[7] = -2;

  /** Checks if the window should appear on top. */
  if(position & WINDOW_ON_BEHIND)
    state[7] = -3;

  /** Opens the window. */
  _swix(Wimp_OpenWindow, _IN(1), state);
}

void windowSetCaret(CARD32 handle, int icon) {
  _swix(Wimp_SetCaretPosition, _INR(0, 1) | _INR(4, 5), handle, icon, -1, 0);
}

void windowModeChange(void) {
  /** Handles mode changes. */
  char *spriteSuffix;

  // Recalculate X and Y eig values.
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 4, (int *)&xEig);
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 5, (int *)&yEig);

  // Get screensize
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 11, (int *)&screenX);
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 12, (int *)&screenY);

  /** Checks if a double height mode. */
  spriteSuffix = (char *)_swi(Wimp_ReadSysInfo, _IN(0), _RETURN(0));
  if(strcmp("24", spriteSuffix) == 0)
    doubleHeightMode = TRUE;
  else
    doubleHeightMode = FALSE;

  /** Do we need to reset bounding box? */
  if(VNCFlags & FLAG_WINDOW_OPEN)
    VNCBoundingBox(screenX << xEig, 0, VNCConfig.width << xEig, VNCConfig.height << yEig);
}

void windowChangeExtent(CARD32 handle, CARD16 width, CARD16 height) {
  // Changes the size of a window
  _swix(Wimp_SetExtent, _IN(0) | _BLOCK(1), handle, 0, -(height << yEig), width << xEig, 0);
}

/*BOOL wimpEnumerateTasks(const char *appName) {
  // Check if there is a copy of appName already running
  CARD32 taskInfo[4], length;
  INT32 index = 0;

  // Is it a valid appname?
  if(appName == NULL || ((length = strlen(appName) + 1) <= 1))
    return FALSE;


  // Goes through all the load tasks
  while(index >= 0) {
    index = _swi(TaskManager_EnumerateTasks, _INR(0, 2) | _RETURN(0), index, taskInfo, 16);

    if(index >= 0 && memcmp((char *)taskInfo[1], appName, length) == 0)
      return TRUE;
  }

  // Failed
  return FALSE;
}
*/
