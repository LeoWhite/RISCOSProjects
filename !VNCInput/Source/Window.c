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
  iconHandle = swi_wimp_r0(Wimp_CreateIcon, iconBlock);

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

  /** Checks there is no memory already allocated. */
  if(templateBuffer != NULL)
    free(templateBuffer);

  /** Open the template file. */
  if(swi_error_fast(0, (int)filePath, NULL, Wimp_OpenTemplate)) {
    /** Reports the error. */
    mainReportError(swi_get_error(&error), error, 0);
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
  swi_wimp(Wimp_CloseTemplate, NULL);

  /** Successfully completed. */
  return TRUE;
}

int windowCalculateBuffer(char *name, BOOL bufferType) {
  /** Returns the size of a templates buffer. */
  int regs[10];

  /** Reads template details. */
  regs[0] = 0;
  regs[1] = 0;
  regs[4] = -1;
  regs[5] = (int)&name[0];
  regs[6] = 0;
  swi(Wimp_LoadTemplate, regs);

  /** Returns indirect size. */
  if(bufferType == 0)
    return regs[2];
  else
    return regs[1];
}

int windowCreate(char *name, int *regs) {
  /** Creates the specified window. */
  regs[0] = regs[6] = 0;
  regs[4] = -1;
  regs[5] = (int)&name[0];
  swi(Wimp_LoadTemplate, regs);

  /** Creates the window and returns the window handle. */
  return swi_fast_r0(NULL, regs[1], NULL, Wimp_CreateWindow);
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
  swi_wimp(Wimp_GetIconState, block);
}

void windowIconSetState(CARD32 handle, CARD16 icon, CARD32 EORWord, CARD32 clearWord) {
  /** Sets the icon state. */
  int block[4];

  block[0] = handle;
  block[1] = icon;
  block[2] = EORWord;
  block[3] = clearWord;
  swi_wimp(Wimp_SetIconState, &block);
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
  swi_wimp(Wimp_GetWindowState, &state);

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
    swi_wimp(Wimp_GetPointerInfo, &ptr);

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
  swi_wimp(Wimp_OpenWindow, &state);
}

void windowSetCaret(CARD32 handle, int icon) {
  /** Sets the caret position. */
  int regs[10];

  /** Sets up info. */
  regs[0] = handle;
  regs[1] = icon;
  regs[4] = -1;
  regs[5] = 0;
  swi(Wimp_SetCaretPosition, regs);
}

void windowModeChange(void) {
  /** Handles mode changes. */
  char *spriteSuffix;

  /** Recalculates X and Y eig values. */
  xEig = swi_fast_r2(-1, 4, NULL, OS_ReadModeVariable);
  yEig = swi_fast_r2(-1, 5, NULL, OS_ReadModeVariable);

  /** Reads in screens X and Y sizes. */
  screenX = swi_fast_r2(-1, 11, NULL, OS_ReadModeVariable) + 1;
  screenY = swi_fast_r2(-1, 12, NULL, OS_ReadModeVariable) + 1;

  /** Checks if a double height mode. */
  spriteSuffix = (char *)swi_fast_r0(2, NULL, NULL, Wimp_ReadSysInfo);
  if(strcmp("24", spriteSuffix) == 0)
    doubleHeightMode = TRUE;
  else
    doubleHeightMode = FALSE;

  /** Do we need to reset bounding box? */
  if(VNCFlags & FLAG_WINDOW_OPEN)
    VNCBoundingBox(screenX << xEig, 0, VNCConfig.width << xEig, VNCConfig.height << yEig);
}

void windowChangeExtent(CARD32 handle, CARD16 width, CARD16 height) {
  /** Adjusts the size of the window. */
  int newExtent[4];

  /** Adjusts sizes. */
  newExtent[0] = 0;
  newExtent[2] = width << xEig;
  newExtent[1] = -(height << yEig);
  newExtent[3] = 0;

  /** Changes the extent. */
  swi_fast(handle, (int)&newExtent, NULL, Wimp_SetExtent);
}


