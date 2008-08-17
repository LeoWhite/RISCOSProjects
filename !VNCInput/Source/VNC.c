/******************************************************************************

Program:        VNCInput

File:           VNC.c

Function:       A VNC input only client for Risc OS

Description:    Handles VNC specific functions

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 21st November 1999 - Creation

******************************************************************************/

/** Includes. */
#include "VNCInput.h"

/*****************************************************************************/
void VNCOpenConnection(void) {
  /** Attempts to open a connection to the VNC server. */
  int length;
  char output[41], tempString[128];

  /** Read in the server address. */
  strncpy(tempString, windowIconGetText(connectWindow, CONNECT_HOSTNAME), 128);

  /** Checks for the display number. */
  if(strchr(tempString, ':') != NULL) {
    /** Close the connect window. */
    swi_wimp(Wimp_CloseWindow, &connectWindow);

    /** Extracts the port number. */
    length = strcspn(tempString, ":");
    VNCPort = VNCBaseClientPort + atoi(&tempString[length + 1]);
    tempString[length] = 0;
    if(strcmp(tempString, "0") == 0)
      sprintf(tempString, "127.0.0.1");

    /** Sets messages and opens connecting window. */
    sprintf(output, "Attempting to connect to ");
    strncat(output, tempString, (40 - strlen(output)));
    windowIconSetText(connectingWindow, CONNECTING_TOP, output);
    windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Looking up address");
    windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "");
    windowOpen(connectingWindow, WINDOW_ON_TOP | WINDOW_CENTERED);

    /** Attempts to connect. */
    VNCSocket = socketGenerate(tempString, VNCPort);

    /** Checks if successful. */
    if(VNCSocket == -2) {
      /** Waiting on resolver. */
      if(resolverStatus != 2)
        windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Failed to resolve name");

      return;
    }

    if(VNCSocket != -1) {
      /** Sets flags, activates internet event watching and set message. */
      VNCFlags |= FLAG_CONNECTED;
      windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Connection established.");

      /** Set flags to say waiting for protocol version. */
      networkStatus = NS_VERSION;

      /** Adjusts menu entries. */
      mainMenuData[CONNECT_ITEM] = MENU_SHADED;
      mainMenuData[DISCONNECT_ITEM] = MENU_UNSHADED;
    }
    else {
      /** Failed. */
      windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Failed to establish connection.");
    }
  }
  else
    mainReportError("Please use 'Hostname:DisplayNumber'", ERROR_CONNECT, 0);
}

void VNCCloseConnection(void) {
  /** Disconnects from the server and frees up memory used. */
  socketBuff *nextBuff;

  /** Ensure main window is closed. */
  swi_wimp(Wimp_CloseWindow, &mainWindow);

  /** Closes the socket. */
  if(VNCSocket != -1) {
    close(VNCSocket);
    VNCSocket = -1;
  }

  /** Unsets flags. */
  VNCFlags &= ~(FLAG_CONNECTED | FLAG_INITIALISED);
  networkStatus = 0;
  resolverStatus = 0;

  /** Adjusts menu entries. */
  mainMenuData[CONNECT_ITEM] = MENU_UNSHADED;
  mainMenuData[DISCONNECT_ITEM] = MENU_SHADED;

  /** Reset bounding box. */
  if(VNCFlags & FLAG_WINDOW_OPEN)
    VNCProcessEdgeDetect();

  /** Frees up buffer. */
  nextBuff = memBuffer;
  while(nextBuff != NULL) {
    memBuffer = nextBuff->next;

    free(nextBuff->data);
    free(nextBuff);

    nextBuff = memBuffer;
  }

  memBuffer = NULL;
}

void VNCProcessVersion(void) {
  /** Reads in and replies to the process number. */
  int major, minor;
  char protocol[13];

  /** Reads in the data. */
  if((socketReadData(protocol, VNCVersionLength)) <= 0)
    return;

  /** Extracts the protocol numbers. */
  major = atoi(&protocol[4]);
  minor = atoi(&protocol[8]);

  /** Checks the version numbers. */
  if(major > VNCMajorVersion) {
    major = VNCMajorVersion;
    minor = VNCMinorVersion;
  }
  else if(major == VNCMajorVersion && minor > VNCMinorVersion)
    minor = VNCMinorVersion;

  /** Updates the protocol string. */
  sprintf(&protocol[4], "%3i.%3i", major, minor);

  /** Sets zeros. */
  if(major < 100)
    protocol[4] = '0';

  if(major < 10)
    protocol[5] = '0';

  if(minor < 100)
    protocol[8] = '0';

  if(minor < 10)
    protocol[9] = '0';

  /** Sends the data. */
  if((write(VNCSocket, protocol, VNCVersionLength)) <= 0) {
    VNCCloseConnection();
    return;
  }

  windowIconSetText(connectingWindow, CONNECTING_BOTTOM, protocol);

  /** Sets flags. */
  networkStatus = NS_PASSWORD;
}

void VNCGetPassword(void) {
  /** Checks if we need a password, and if so open the window. */
  CARD32 authScheme;
  CARD8 shared = 1;

  /** Sets message. */
  windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Performing authentication");

  /** Reads in data. */
  if((socketReadData(&authScheme, sizeOfCARD32)) <= 0)
    return;

  /** Converts endianess. */
  authScheme = (CARD32)ntohl(authScheme);

  /** Checks result. */
  switch(authScheme) {
    /** No password required. */
    case VNCNoAuthentication:
      windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "No password required");

      /** Sends shared info. */
      if((write(VNCSocket, &shared, sizeOfCARD8)) <= 0) {
        VNCCloseConnection();
        return;
      }

      networkStatus = NS_SERVER_INIT;
    break;

    /** Password required. */
    case VNCAuthenticationReq:
      windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Password required");
      windowOpen(passwordWindow, WINDOW_ON_TOP | WINDOW_CENTERED);
      windowSetCaret(passwordWindow, PASSWORD_INPUT);

      networkStatus = NS_WAITING_FOR_ENCRYPT;;
    break;

    /** Connection failed. */
    case VNCConnectionFailed:
    default:
      windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Connection failed");
      VNCCloseConnection();
    break;
  }
}

void VNCAuthentication(void) {
  /** Handles authentication routines. */
  unsigned char challenge[VNCChallengeSize];
  unsigned char password[VNCPasswordLength + 1];

  /** Reads in challenge. */
  if((socketReadData(challenge, VNCChallengeSize)) <= 0)
    return;

  /** Close window and reads in password. */
  swi_wimp(Wimp_CloseWindow, &passwordWindow);
  strncpy((char *)&password, windowIconGetText(passwordWindow, PASSWORD_INPUT), VNCPasswordLength);

  /** Set zero terminator */
  password[VNCPasswordLength] = '\0';

  /** Converts challenge. */
  VNCEncryptBytes(challenge, password);

  /** Sends results. */
  if((write(VNCSocket, &challenge, VNCChallengeSize)) <= 0) {
    VNCCloseConnection();
    return;
  }

  /** Sets status. */
  networkStatus = NS_PASSWORD_RESPONSE;

  /** Clears the password. */
  bzero(windowIconGetText(passwordWindow, PASSWORD_INPUT), VNCPasswordSize);
}

void VNCEncryptBytes(unsigned char *bytes, unsigned char *password) {
  /** Encryts the bytes using the password as the key. */
  CARD8 counter;

  /** Pads the password with NULLs. */
  bzero(&password[strlen((char *)password)], VNCPasswordLength - strlen((char *)password));

  /** Sends the key. */
  deskey(password, EN0);

  /** Processes the key. */
  for(counter = 0; counter < VNCChallengeSize; counter += 8)
    des(bytes + counter, bytes + counter);
}

void VNCPasswordResponse(void) {
  /** Reads in password response and processes it. */
  CARD32 response;
  CARD8 shared = 1;

  /** Reads in password response. */
  if((socketReadData(&response, sizeOfCARD32)) <= 0)
    return;

  /** Converts to host endianess. */
  response = (CARD32)ntohl(response);

  /** Checks what action to take. */
  switch(response) {
    /** Password accepted. */
    case VNCPasswordOK:
      /** Sends shared flags. */
      if((write(VNCSocket, &shared, sizeOfCARD8)) <= 0) {
        VNCCloseConnection();
        return;
      }

      /** Sets network status. */
      networkStatus = NS_SERVER_INIT;
      windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Password correct");
    break;

    /** Password rejected. */
    default:
      if(response == VNCPasswordTooMany)
        windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Password incorrect - Too many attempts");
      else
        windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "Password incorrect");

      /** Closes connection. */
      VNCCloseConnection();
    break;
  }
}

void VNCInitialiseDisplay(void) {
  /** Gets display information. */
  rfbSize size;
  rfbPixelFormat serverInit;
  CARD32 length;
  char *titleString;

  /* Updates messages. */
  windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Getting server details");
  windowIconSetText(connectingWindow, CONNECTING_BOTTOM, "");

  /** Reads in width and height. */
  if(socketReadData(&size, sizeOfCARD32) <= 0)
    return;

  /** Converts endianess. */
  size.Width  = ntohs(size.Width);
  size.Height = ntohs(size.Height);

  /** Reads in server info. */
  if(socketReadData(&serverInit, VNCServerInitSize) <= 0)
    return;

  /** Reads in title length. */
  if(socketReadData(&length, sizeOfCARD32) <= 0)
    return;
  length = (CARD32)ntohl(length);

  /** Reads in title string. */
  titleString = (char *)malloc(length + 1);
  if(titleString == NULL)
    return;

  if(socketReadData(titleString, length) <= 0) {
    free(titleString);
    return;
  }

  /** Adds null termiantor. */
  titleString[length] = 0;

  /** Record height and width. */
  VNCConfig.width  = size.Width;
  VNCConfig.height = size.Height;

  /** Set network status. */
  networkStatus = 0;

  windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Adjusting window size.");

  /** Adjust size of main window. */
  windowChangeExtent(mainWindow, VNCConfig.width, VNCConfig.height);

  /** Set the encoding types supported. */
  VNCSetEncoding();

  /** Close the connecting window. */
  swi_wimp(Wimp_CloseWindow, &connectingWindow);

  /** Finished initialising. */
  VNCFlags |= FLAG_INITIALISED;

  /** Turn on edge detection. */
  pollword[POLL_CONFIG] |= POLL_EDGE_DETECT;
}

void VNCSetEncoding(void) {
  /** Tells the server what encoding types we support. */
  CARD8 messageType = 2;
  CARD16 numberOfEncoding = 1;
  CARD32 encodingType;

  /** Sends info. */
  /* sends info */
  numberOfEncoding = htons(numberOfEncoding);
  if((write(VNCSocket, &messageType, sizeOfCARD8)) <= 0) {
    VNCCloseConnection();
    return;
  }
  if((write(VNCSocket, &messageType, sizeOfCARD8)) <= 0) {
    VNCCloseConnection();
    return;
  }
  if((write(VNCSocket, &numberOfEncoding, sizeOfCARD16)) <= 0) {
    VNCCloseConnection();
    return;
  }


  /* Always send Raw */
  encodingType = (CARD32)htonl(0x0);
  if((write(VNCSocket, &encodingType, sizeOfCARD32)) <= 0) {
    VNCCloseConnection();
    return;
  }
}

void VNCProcessMouse(void) {
  /** Sends the current mouse position. */
  rfbPointerEvent pointer;
  int regs[10];

  /** Reads in mouse position. */
  swi_wimp(Wimp_GetPointerInfo, &regs);

  /** Checks if its above our window. Shouldn't happen. */
  if(regs[3] != mainWindow)
    return;

  pointer.XPosition = regs[0] - (screenX << xEig);
  pointer.YPosition = regs[1];

  /** Sets buttons. */
  pointer.ButtonMask = 0;
  if(regs[2] & 4)
    pointer.ButtonMask |= 1;
  if(regs[2] & 2)
    pointer.ButtonMask |= 2;
  if(regs[2] & 1)
    pointer.ButtonMask |= 4;

  /** Adjusts for screen changes. */
  pointer.XPosition = pointer.XPosition >> xEig;
  pointer.YPosition = VNCConfig.height - (pointer.YPosition >> yEig);

  /** Checks ifits a double height mode. */
  if(doubleHeightMode)
    pointer.YPosition *= 2;

  /** Set up pointer structure. */
  pointer.MessageType = 5;
  pointer.XPosition = htons(pointer.XPosition);
  pointer.YPosition = htons(pointer.YPosition);

  /** Sends data. */
  if(write(VNCSocket, &pointer, VNCPointerSize) <= 0) {
    VNCCloseConnection();
    return;
  }
}

void VNCProcessWimpKey(int keyCode) {
  /** Process wimp keys. */
  rfbKeyEvent keyEvent;

  /** Checks what key was pressed. */
  switch(keyCode) {
    case keycode_BACKSPACE:keyCode = 0xFF08;
    break;

    case keycode_TAB:keyCode = 0xFF09;
    break;

    case keycode_RETURN:keyCode = 0xFF0D;
    break;

    case keycode_ESCAPE:keyCode = 0xFF1B;
    break;

    case keycode_INSERT:keyCode = 0xFF63;
    break;

    case keycode_DELETE:keyCode = 0xFFFF;
    break;

    case keycode_HOME:keyCode = 0xFF50;
    break;

    case keycode_COPY:keyCode = 0xFF57;
    break;

    case keycode_PAGEUP:keyCode = 0xFF55;
    break;

    case keycode_PAGEDOWN:keyCode = 0xFF56;
    break;

    case keycode_CURSORLEFT:keyCode = 0xFF51;
    break;

    case keycode_CURSORUP:keyCode = 0xFF52;
    break;

    case keycode_CURSORRIGHT:keyCode = 0xFF53;
    break;

    case keycode_CURSORDOWN:keyCode = 0xFF54;
    break;

    case 136:keyCode = 0xFF51;
    break;

    case 139:keyCode = 0xFF52;
    break;

    case 137:keyCode = 0xFF53;
    break;

    case 138:keyCode = 0xFF54;
    break;

    case keycode_F1:keyCode = 0xFFBE;
    break;

    case keycode_F2:keyCode = 0xFFBF;
    break;

    case keycode_F3:keyCode = 0xFFC0;
    break;

    case keycode_F4:keyCode = 0xFFC1;
    break;

    case keycode_F5:keyCode = 0xFFC2;
    break;

    case keycode_F6:keyCode = 0xFFC3;
    break;

    case keycode_F7:keyCode = 0xFFC4;
    break;

    case keycode_F8:keyCode = 0xFFC5;
    break;

    case keycode_F9:keyCode = 0xFFC6;
    break;

    case keycode_F10:keyCode = 0xFFC7;
    break;

    case keycode_F11:keyCode = 0xFFC8;
    break;

    case keycode_F12:keyCode = 0xFFC9;
    break;

    default:
    break;
  }

  /** Down event. */
  keyEvent.MessageType = 4;
  keyEvent.DownFlag    = 1;
  keyEvent.key         = (CARD32)htonl(keyCode);
  if(write(VNCSocket, &keyEvent, VNCKeyEventSize) <= 0) {
    VNCCloseConnection();
    return;
  }

  /** Up event. */
  keyEvent.DownFlag    = 0;
  if(write(VNCSocket, &keyEvent, VNCKeyEventSize) <= 0) {
    VNCCloseConnection();
    return;
  }
}

void VNCProcessKey(int keyNumber, CARD8 keyUpDown) {
  /** Processes info on keypresses. */
  rfbKeyEvent keyEvent;

  /** Checks what key to send. */
  switch(keyNumber) {
    /** Shift pressed. */
    case 0x4C:
    case 0x58:
      keyNumber = 0xFFE1;
    break;

    /** Left alt. */
    case 0x5E:keyNumber = 0xFFE9;
    break;

    /** Right alt. */
    case 0x60:keyNumber = 0xFFE7;
    break;

    /** Control. */
    case 0x3B:
    case 0x61:
      keyNumber = 0xFFE3;
    break;

    /** Delete pessed. */
    case 127:keyNumber = 0xFFFF;
    break;

    default:
      return;
    break;
  }

  /** Sets up structure. */
  keyEvent.MessageType = 4;
  keyEvent.DownFlag    = keyUpDown;
  keyEvent.key         = (CARD32)htonl(keyNumber);

  /** Sends info. */
  if(write(VNCSocket, &keyEvent, VNCKeyEventSize) <=0) {
    VNCCloseConnection();
    return;
  }
}

void VNCProcessEdgeDetect(void) {
  /** Checks what to do when the mouse is moved to the edge. */

  if(VNCFlags & FLAG_WINDOW_OPEN) {
    /** Window is open, so close it. */
    swi_wimp(Wimp_CloseWindow, &mainWindow);

    /** Turn off mouse and key watching. */
    pollword[POLL_CONFIG] &= ~(POLL_MOUSE_WATCH | POLL_KEY_WATCH);

    /** Reset bounding box. */
    VNCBoundingBox(0, 0, screenX << xEig, screenY << yEig);

    /** Unset flag. */
    VNCFlags &= ~FLAG_WINDOW_OPEN;
  }
  else {
    /** Open the main window. */
    windowOpen(mainWindow, WINDOW_ON_TOP | WINDOW_FULL_SCREEN);

    /** Set the caret. */
    windowSetCaret(mainWindow, -1);

    /** Window is closed so open it. */
    VNCBoundingBox(screenX << xEig, 0, VNCConfig.width << xEig, VNCConfig.height << yEig);

    /** Turn on mouse watching. */
    pollword[POLL_CONFIG] |= (POLL_MOUSE_WATCH | POLL_KEY_WATCH);

    /** Set flag. */
    VNCFlags |= FLAG_WINDOW_OPEN;
  }
}

void VNCBoundingBox(int x,  int y, short int width, short int height) {
  /** Marks the area available for the box. */
  char block[9];
  short int *values = (short int *)&block[1];

  /** Set the graphics origin. */
  swi_fast(NULL, NULL, NULL, 256 + 29);
  swi_fast(x & 0xFF, NULL, NULL, OS_WriteC);
  swi_fast((x & 0xFF00) >> 8, NULL, NULL, OS_WriteC);
  swi_fast(y & 0xFF, NULL, NULL, OS_WriteC);
  swi_fast((y & 0xFF00) >> 8, NULL, NULL, OS_WriteC);

  /** Set up values. */
  block[0] = 1;
  values[0] = 0;
  values[1] = 0;
  values[2] = width;
  values[3] = height;

  /** Calls swi. */
  swi_fast(21, (int)block, NULL, OS_Word);

  /** Reset graphics origin. */
  swi_fast(NULL, NULL, NULL, 256 + 29);
  swi_fast(0, NULL, NULL, OS_WriteC);
  swi_fast(0, NULL, NULL, OS_WriteC);
  swi_fast(0, NULL, NULL, OS_WriteC);
  swi_fast(0, NULL, NULL, OS_WriteC);

}
