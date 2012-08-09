/******************************************************************************

Program:        VNCInput

File:           Button.c

Function:       A VNC Input only Client for Risc OS

Description:    Handles mouse clicks, menu clicks and keypresses

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sat 20th November 1999 - Creation

******************************************************************************/

/** Includes. */
#include "VNCInput.h"

/** Local variables. */
char **hostNames;

/*****************************************************************************/
void buttonClick(int *event) {
  /** Handles mouse button clicks. */

  /** Checks if its a menu click. */
  if(event[EVENT_MOUSE_BUTTON] == MOUSE_MENU) {
    mainReportError("mouse buttopn", ERROR_TEMPLATE, 0);

    /** Checks if an icon bar click. */
    if(event[EVENT_MOUSE_ICON] == VNCBarIcon && event[EVENT_MOUSE_WINDOW] == -2) {
      /** Open the iconbar menu. */
      hostOpen = FALSE;
      buttonOpenMenu(event);
    }
    else {
      /** Host menu? */
      if(event[EVENT_MOUSE_ICON] == CONNECT_MENU && event[EVENT_MOUSE_WINDOW] == connectWindow) {
        /** Open the iconbar menu. */
        hostOpen = TRUE;
        buttonOpenMenu(event);
      }
    }
  }
  else {
    /** Is it a iconbar click? */
    if(event[EVENT_MOUSE_ICON] == VNCBarIcon && event[EVENT_MOUSE_WINDOW] == -2) {
      /** If not connected then open window. */
      if(!(VNCFlags & FLAG_CONNECTED)) {
        windowOpen(connectWindow, WINDOW_ON_TOP | WINDOW_CENTERED);
        _swi(Wimp_CloseWindow, _IN(0), &connectingWindow);
        windowSetCaret(connectWindow, CONNECT_HOSTNAME);
      }
    }
    else {
      /** Click on connect window. */
      if(event[EVENT_MOUSE_WINDOW] == connectWindow)
        buttonConnectWindow(event);
    }
  }
}

void buttonMenuClick(int *event) {
  /** Checks what menu item has been selected. */
  int ptr[5];

  /** Is it the host menu? */
  if(hostOpen) {
    windowIconSetText(connectWindow, CONNECT_HOSTNAME, (char *)hostNames[event[EVENT_MENU_SELECTION]]);
  }
  else {
    /** Must be main menu click. */
    switch(event[EVENT_MENU_SELECTION]) {
      /** Open the Info Window. */
      case MENU_INFO:windowOpen(infoWindow, WINDOW_UNDER_PTR | WINDOW_ON_TOP);
      break;

      /** Opens the connect window. */
      case MENU_CONNECT:
        windowOpen(connectWindow, WINDOW_CENTERED | WINDOW_ON_TOP);
        _swi(Wimp_CloseWindow, _IN(0), &connectingWindow);
        windowSetCaret(connectWindow, CONNECT_HOSTNAME);
      break;

      /** Disconnects from the server. */
      case MENU_DISCONNECT:VNCCloseConnection();
      break;

      /** Quits the program. */
      case MENU_QUIT:VNCFlags |= FLAG_QUIT;
      break;
    }
  }

  /** Checks if need to reopen meu. */
  _swi(Wimp_GetPointerInfo, _IN(1), &ptr);
  if(ptr[2] == MOUSE_ADJUST) {
    /** Reopen the menu. */
    buttonOpenMenu(event);
  }
}

void buttonConnectWindow(int *event) {
  /** Click on connect window. */
  /** Check which button. */
  switch(event[EVENT_MOUSE_ICON]) {
    /** Start a new session. */
    case CONNECT_CONNECT:VNCOpenConnection();
    break;

    /** Canceled. Close window. */
    case CONNECT_CANCEL:_swi(Wimp_CloseWindow, _IN(0), &connectWindow);
    break;

    /** Click on menu button. */
    case CONNECT_MENU:
      hostOpen = TRUE;
      buttonOpenMenu(event);
    break;
  }
}

void buttonKeyPress(int *event) {
  /** Handles key presses. */

  /** Key press in the connect window. */
  if(event[EVENT_WINDOW_HANDLE] == connectWindow) {
    switch(event[EVENT_KEY_CODE]) {
      /** Attempts to initiate a connection. */
      case 13:VNCOpenConnection();
      break;

      /** Escape pressed. */
      case 27:_swi(Wimp_CloseWindow, _IN(0), &connectWindow);
      break;

      /** Allow wimp to process key. */
      default:
       _swi(Wimp_ProcessKey, _IN(0), event[EVENT_KEY_CODE]);
      break;
    }
  }
  else if(event[EVENT_WINDOW_HANDLE] == passwordWindow) {
    /** Keypress in password window. */
    switch(event[EVENT_KEY_CODE]) {
      /** Return pressed. Continue with authentication. */
      case 13:VNCAuthentication();
      break;

      /** Escape pressed, abort. */
      case 27:
        _swi(Wimp_CloseWindow, _IN(0), &passwordWindow);
        windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Connection Aborted");
        /** VNCCloseConnection(); */
      break;

      /** Default, allow wimp to process key. */
      default:_swi(Wimp_ProcessKey, _IN(0), event[EVENT_KEY_CODE]);
      break;
    }
  }
  else {
    /** If keypress in MainWindow, then send to server. Otherwise process. */
    if(event[EVENT_WINDOW_HANDLE] != mainWindow)
      _swi(Wimp_ProcessKey, _IN(0), event[EVENT_KEY_CODE]);
    else {
      VNCProcessWimpKey(event[EVENT_KEY_CODE]);
    }
  }
}

void buttonOpenMenu(int *event) {
  /** Opens a menu. */
  int regs[10];

  regs[0] = 0;

  if(hostOpen == TRUE) {
    regs[1] = (int)&hostMenuData[0];
    regs[2] = event[EVENT_MOUSE_POS_X] - 44;
    regs[3] = event[EVENT_MOUSE_POS_Y] + 22;
  }
  else {
    regs[1] = (int)&mainMenuData[0];
    regs[2] = event[EVENT_MOUSE_POS_X] - 64;
    regs[3] = 96 + MENU_NUM_ITEMS * 44;
  }

  _swi(Wimp_CreateMenu, _INR(0,3), regs[0], regs[1], regs[2], regs[3]);
}

void buttonLoadMenu(void) {
  /** Loads in the host name menu. */
  FILE *menuData = NULL;
  char *menuSpace;
  int *item, read;
  CARD16 numberOfItems, counter;
  char tempString[128], *readStr = NULL;

  /** Opens the menu file. */
  menuData = fopen("<VNCInput$Dir>.Hosts", "r");

  /** Check if successful. */
  if(menuData == NULL) {
    /** Allocate space for menu. */
    hostMenuData = (int *)malloc(14 * 4);

    /** Set up menu header. */
    menuSpace = (char *)&hostMenuData[0];
    strcpy(menuSpace, "Hosts");
    menuSpace[12] = 7;
    menuSpace[13] = 2;
    menuSpace[14] = 7;
    menuSpace[15] = 0;
    menuSpace[16] = 10 * 16 + 32;
    hostMenuData[5] = 44;
    hostMenuData[6] = 1;

    /** Set up empty item. */
    item = &hostMenuData[7];
    item[0] = 0x80;
    item[1] = NULL;
    item[2] = MENU_SHADED;
    strcpy((char *)&item[3], "Empty");

    /** Menu set up, so exit. */
    return;
  }

  /** File opened, so process it. */
  read = fscanf(menuData, "%i\n", &numberOfItems);

  /** Checks if worked. */
  if(read == 1) {
    /** Malloc space. */
    hostNames = (char **)malloc(sizeof(char *) * numberOfItems);

    /** Allocates space for menu info. */
    hostMenuData = (int *)malloc((8 + (numberOfItems * 6)) * 4);

    /** Set up menu header. */
    menuSpace = (char *)&hostMenuData[0];
    strcpy(menuSpace, "Hosts");
    menuSpace[12] = 7;
    menuSpace[13] = 2;
    menuSpace[14] = 7;
    menuSpace[15] = 0;
    menuSpace[16] = 10 * 16 + 32;
    hostMenuData[5] = 44;
    hostMenuData[6] = 1;

    /** Fill in item information. */
    for(counter = 0; counter < numberOfItems; counter++) {
      /** Read in string. */
      readStr = fgets(tempString, 128, menuData);

      if(readStr != NULL) {
        /** Copies string. */
        hostNames[counter] = (char *)malloc(strlen(tempString) + 1);
        strcpy((char *)hostNames[counter], tempString);

        /** Sets up menu item. */
        item = &hostMenuData[7 + (6 * counter)];
        if(counter == (numberOfItems - 1))
          item[0] = 0x80;
        else
          item[0] = 0;
        item[1] = NULL;
        item[2] = MENU_INDIRECTED;
        item[3] = (int)hostNames[counter];
        item[4] = NULL;
        item[5] = strlen(tempString);
      }
      else {
        /** Invalid input. */
        hostNames[counter] = (char *)malloc(8);
        strcpy((char *)hostNames[counter], "Invlaid");

        /** Sets up menu item. */
        item = &hostMenuData[7 + (6 * counter)];
        if(counter == (numberOfItems - 1))
          item[0] = 0x80;
        else
          item[0] = 0;
        item[1] = NULL;
        item[2] = MENU_INDIRECTED | MENU_SHADED;
        item[3] = (int)hostNames[counter];
        item[4] = NULL;
        item[5] = 8;
      }
    }
  }

  /** Close the file. */
  fclose(menuData);
}



