/******************************************************************************

Program:        RiscXFE

File:           Wimp.c

Function:       Frontend program for RISCX

Description:    Wimp Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Tue 01st February 2000

******************************************************************************/

// Includes
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "wimp.h"
#include "display.h"

// Global variables

/****************************** Procedures. *********************************/
bool wimpEnumerateTasks(const char *appName) {
  // Check if there is a copy of appName already running
  CARD32 taskInfo[4], length;
  INT32 index = 0;

  // Is it a valid appname?
  if(appName == NULL || ((length = strlen(appName) + 1) <= 1))
    return false;


  // Goes through all the load tasks
  while(index >= 0) {
    index = _swi(TaskManager_EnumerateTasks, _INR(0, 2) | _RETURN(0), index, taskInfo, 16);

    if(index >= 0 && memcmp((char *)taskInfo[1], appName, length) == 0)
      return true;
  }

  // Failed
  return false;
}

/****************************************************************************/
void wimpTaskCloseDown(CARD32 taskHandle) {
  // Closes down a task.
  _swi(Wimp_CloseDown, _INR(0,1), taskHandle, WIMP_TASK);
}

/****************************************************************************/
CARD32 wimpReportError(const _kernel_oserror *error, const CARD32 errorFlags, const char *appName,
                    const char *spriteName, const CARD32 spriteArea, const char *buttons) {
  // Checks if it should be a old or new Error box
  if(riscosVersion <= 0xA5)
    // Old style boxes.
    return _swi(Wimp_ReportError, _INR(0, 2) | _RETURN(1), error, errorFlags, appName);

  // New style error box.
  return _swi(Wimp_ReportError, _INR(0, 5) | _RETURN(1), error, errorFlags, appName, spriteName,
              spriteArea, buttons);
}

/****************************************************************************/
INT32 *moduleGetPollword(const char *moduleName) {
  // Returns the pollword for this module, or NULL
  INT32 *result;

  if(_swix(OS_Module, _INR(0, 1) | _OUT(4), 18, moduleName, &result)) {
    // Couldn't find the module
    return NULL;
  }

  // Return result
  return result;
}

/****************************************************************************/
CARD32 wimpCreateIconbar(const char *spriteName, const INT16 position) {
  // Creates an icon and places it on the iconbar.
  int iconBlock[9], iconHandle;

  // Set up icon details
  iconBlock[0] = -1;
  iconBlock[1] = iconBlock[2] = 0;
  iconBlock[3] = iconBlock[4] = 68;
  iconBlock[5] = 0x3002;

  // Copy name
  sprintf((char *)&iconBlock[6], "%s\0", spriteName);

  // Return the iconhandle
  if(_swix(Wimp_CreateIcon, _IN( 1) | _OUT(0), iconBlock, &iconHandle)) {
    // Failed
    return -1;
  }

  // Returniconhandle
  return iconHandle;
}

/****************************************************************************/
static CARD32 wimpCalculateBuffer(char *name, bool bufferType) {
  // Returns the size of buffer required to hold the specified window.

  // Checks which buffer size is required.
  if(bufferType)
    // template buffer
    return _swi(Wimp_LoadTemplate, _INR(0, 1) | _INR(4, 6) | _RETURN(1), 0, 0, -1, name, 0);
  else
    // Indirect buffer size
    return _swi(Wimp_LoadTemplate, _INR(0, 1) | _INR(4, 6) | _RETURN(2), 0, 0, -1, name, 0);
}

/****************************************************************************/
static CARD32 wimpCreateWindow(char *name, char *indirectBuffer, char **nextFree, char *endBuffer) {
  // Creates a window, using the passed in bufffers

  // Load the window details out of the template file
  _swi(Wimp_LoadTemplate, _INR(1, 6) | _OUT(2), indirectBuffer, *nextFree, endBuffer, -1, name, 0, nextFree);

  // Create the window
  return _swi(Wimp_CreateWindow,_IN(1) | _RETURN(0), indirectBuffer);
}

/****************************************************************************/
char *wimpLoadTemplate(const char *filePath, const char *names[], CARD32 *handles) {
  // Opens the specified templates file, and creates windows until a NULL is found
  CARD16 bufferSize = 0, tempSize = 0, i = 0;
  char *indirectBuffer = NULL, *templateBuffer = NULL, *nextFree = NULL, *end = NULL;

  // Open the templates file
  if(_swix(Wimp_OpenTemplate, _IN(1), filePath)) {
    // Failed!
    return NULL;
  }

  // Calculate the indirect buffer size.
  while(names[i] != NULL) {
    tempSize = wimpCalculateBuffer((char *)names[i], 0);
    if(tempSize > bufferSize) bufferSize = tempSize;
    i++;
  }

  // Allocate the memory
  if((indirectBuffer = (char *)malloc(bufferSize * 8)) == NULL) {
    // Close template file.
    _swi(Wimp_CloseTemplate, 0);
    return NULL;
  }

  // Calculate template buffersize
  i = 0;
  bufferSize = 0;
  while(names[i++] != NULL) {
    bufferSize += wimpCalculateBuffer((char *)names[i], 1);
    i++;
  }

  // Allocate memory.
  if((templateBuffer = (char *)malloc(bufferSize)) == NULL) {
    // Failed, close template file and free memory
    _swi(Wimp_CloseTemplate, 0);
    free(indirectBuffer);
    return NULL;
  }

  // Creates the windows
  i = 0;
  nextFree = templateBuffer;
  end = templateBuffer;
  end += bufferSize;
  while(names[i] != NULL) {
    handles[i] = wimpCreateWindow((char *)names[i], indirectBuffer, &nextFree, end);
    i++;
  }

  // Frees indirect buffer.
  free(indirectBuffer);

  // Close template file.
  _swi(Wimp_CloseTemplate, 0);

  // Return pointer
  return templateBuffer;
}

/****************************************************************************/
char *wimpGetIconText(CARD32 handle, CARD16 icon) {
  // Returns a pointer to the indirect buffer in the icon
  CARD32 iconBlock[10];

  // Gets Icon information
  wimpGetIconState(handle, icon, iconBlock);

  // Check if the icon is indirected
  if(iconBlock[6] & (1 << 8))
    return (char *)iconBlock[7];

  // Failed
  return NULL;
}

/****************************************************************************/
bool wimpSetIconText(CARD32 handle, CARD16 icon, char *text) {
  // Sets the text in an indirected icon
  CARD32 iconBlock[10];

  // Check if a string was actually passed.
  if(text == NULL) return false;

  // Gets icon info
  wimpGetIconState(handle, icon, iconBlock);

  // Checks if the icon is indirected
  if(iconBlock[6] & (1 << 8)) {
    // Copies the text.
    memcpy((void *)iconBlock[7], (void *)text, iconBlock[9] - 1);

    // Update the icon.
    wimpSetIconState(handle, icon, 0, 0);
  }

  // Success
  return true;
}

/****************************************************************************/
CARD32 wimpGetIconInteger(CARD32 handle, CARD16 icon) {
  // Returns the numeric contents of an icon. */
  return(atoi(wimpGetIconText(handle, icon)));
}

/****************************************************************************/
bool wimpSetIconInteger(CARD32 handle, CARD16 icon, int value) {
  // Writes a 'number' into an icon
  char temp[13];

  // Convert to string
  sprintf(temp, "%i", value);

  // Set icon
  return wimpSetIconText(handle, icon, temp);
}

/****************************************************************************/
void wimpGetIconState(CARD32 handle, CARD16 icon, CARD32 *block) {
  // Returns the state of an icon.
  block[0] = handle;
  block[1] = icon;
  _swix(Wimp_GetIconState, _IN(1), block);
}

/****************************************************************************/
void wimpSetIconState(CARD32 handle, CARD16 icon, CARD32 EORWord, CARD32 clearWord) {
  // Sets the state of an icon
  _swix(Wimp_SetIconState, _BLOCK(1), handle, icon, EORWord, clearWord);
}

/****************************************************************************/
void wimpSetCaret(CARD32 handle, INT16 icon) {
  // Positions the caret
  _swix(Wimp_SetCaretPosition, _INR(0, 1) | _INR(4, 5), handle, icon, -1, 0);
}

/****************************************************************************/
void wimpOpenWindow(CARD32 handle, CARD8 position) {
  // Opens a window in the specified position
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

/****************************************************************************/
void wimpSetExtent(CARD32 handle, CARD16 width, CARD16 height) {
  // Changes the size of a window
  _swix(Wimp_SetExtent, _IN(0) | _BLOCK(1), handle, 0, -(height << yEig), width << xEig, 0);
}

/****************************************************************************/
void wimpModeChange(void) {
  // Handles mode changes
  char *spriteSuffix;
  int numColours, screenMode, size;

  // Recalculate X and Y eig values.
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 4, (int *)&xEig);
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 5, (int *)&yEig);

  // Get screensize
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 11, (int *)&screenX);
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), -1, 12, (int *)&screenY);

  // Check if its a double height mode
  spriteSuffix = (char *)_swi(Wimp_ReadSysInfo, _IN(0), _RETURN(0));
  if(strcmp("24", spriteSuffix) == 0)
    doubleHeightMode = true;
  else
    doubleHeightMode = false;

  // Check if a sprite exits
  if(!displaySetupPointers() || spriteArea == NULL)
    return;

  // Generate trans table
  if(transTable) free(transTable);
  transTable = NULL;

  // Get number of colours
  _swix(OS_SpriteOp, _INR(0, 2) | _OUT(6), 0x228, spriteArea, spritePointer, &screenMode);

  // Check how many colours are in the mode
  _swix(OS_ReadModeVariable, _INR(0, 1) | _OUT(2), screenMode, 3, &numColours);

  if(numColours >= 16) {
    _kernel_oserror *error;

    // Use Wimp tables
    _swix(Wimp_ReadPixTrans, _INR(0, 2) | _INR(6, 7), 0x200, spriteArea, spritePointer, spriteScaling, 0);

    // Get size of tables
    if((error = _swix(ColourTrans_SelectTable, _INR(0, 5) | _OUT(4), spriteArea, spritePointer, -1, -1, 0, (1 << 4) | (1 << 0), &size))) {
      fprintf(stderr, "Failed to get Trans table size, %s,mode = %d\n", error->errmess, screenMode);
      return;
    }


    // Allocate space
    if((transTable = (CARD32 *)malloc(size)) == NULL)
      return;

    // Get size of tables
    if((error = _swix(ColourTrans_SelectTable, _INR(0, 5), spriteArea, spritePointer, -1, -1, transTable, (1 << 4) | (1 << 0)))) {
      fprintf(stderr, "Failed to create Trans table, %s\n", error->errmess);
      return;
    }
  }
  else {
    // Smaller table
    if((transTable = (CARD32 *)malloc(16)) == NULL)
      return;

    // Build table
    _swix(Wimp_ReadPixTrans, _INR(0, 2) | _INR(6, 7), 0x200, spriteArea, spritePointer, spriteScaling, transTable);
  }
}

/****************************************************************************/
void wimpCreateMainMenu(void) {
  /** Creates the main menu. */
  char *menuSpace;
  int *item;

  /** Sets up the pointer. */
  menuSpace = (char *)&mainMenuData[0];

  /** Sets up the menu header. */
  strncpy(menuSpace, JPEGViewName, 8);
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
  item[1] = handles[INFO_WINDOW];
  item[2] = MENU_UNSHADED;
  strcpy((char *)&item[3], "Info");

  /** Data for connect item. */
  item = &mainMenuData[13];
  item[0] = 0x80;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char *)&item[3], "Quit");
}
