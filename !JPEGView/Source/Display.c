/******************************************************************************

Program:        JPEGView

File:           Display.c

Function:       Displays JPEGs

Description:    Handles outputting the display

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sat 05th February 2000

******************************************************************************/

// Includes
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "display.h"
#include "wimp.h"

// Global variables

/****************************** Procedures. *********************************/
bool displaySetupPointers(void) {
  // Sets up the pointers to the sprite
  int flags = 0;

  // Get pointer
  if(spriteArea == NULL) {
    spritePointer = NULL;

    // Ensure window is closed
    _swix(Wimp_CloseWindow, _IN(1), &handles[MAIN_WINDOW]);
    statusFlags &= ~FLAG_MAIN_OPEN;
    DBUGF("spriteBase was NULL\n");
    return false;
  }

  // Is it valid memory?
  if(_swix(OS_ValidateAddress, _INR(0,1) | _OUT(_FLAGS), spriteArea, spriteArea, &flags)) {

    // Ensure window is closed
    _swix(Wimp_CloseWindow, _IN(1), &handles[MAIN_WINDOW]);
    statusFlags &= ~FLAG_MAIN_OPEN;
    DBUGF("Invalid memory error.\n");
    return false;
  }

  // Check flags
  if(flags & _C) {
    // Invalid memory range.
    spriteArea = NULL;
    spritePointer = NULL;

    // Ensure window is closed
    _swix(Wimp_CloseWindow, _IN(1), &handles[MAIN_WINDOW]);
    statusFlags &= ~FLAG_MAIN_OPEN;
    DBUGF("Memory area was invalid\n");
    return false;
  }

  // Work out position of sprite
  if(_swix(OS_SpriteOp, _INR(0, 2) | _OUT(2), 0x118, spriteArea, JPEGViewName, &spritePointer)) {
    // Ensure window is closed
    _swix(Wimp_CloseWindow, _IN(1), &handles[MAIN_WINDOW]);
    statusFlags &= ~FLAG_MAIN_OPEN;

    // Failed
    DBUGF("Failed to locate sprite\n");
    return false;
  }

  DBUG(fprintf(stderr, "sprite Area = %p, spritePointer = %p\n", spriteArea, spritePointer));

  // Get the width and height
  if(_swix(OS_SpriteOp, _INR(0,2) | _OUTR(3, 4), 0x228, spriteArea, spritePointer, (int *)&spriteWidth, (int *)&spriteHeight)) {
    // Ensure window is closed
    _swix(Wimp_CloseWindow, _IN(1), &handles[MAIN_WINDOW]);
    statusFlags &= ~FLAG_MAIN_OPEN;

    // Failed
    DBUGF("Failed to get width and height\n");
    return false;
  }

  // Ensure window is of the correct size.
  wimpSetExtent(handles[MAIN_WINDOW], spriteWidth, spriteHeight);

  if(statusFlags & FLAG_MAIN_OPEN)
    wimpOpenWindow(handles[MAIN_WINDOW], 0);

  // Success
  return true;
}

/****************************************************************************/
void displayRedrawMain(CARD32 handle) {
  // Redraws the main window.
  int redrawBlock[11], more, xPos, yPos;

  // Setup block
  redrawBlock[0] = handle;

  // Call swi
  if(_swix(Wimp_RedrawWindow, _IN(1) | _OUT(0), redrawBlock, &more))
    return;

  // Possible rebuild palette here?

  // Main loop
  while(more) {
    // Work out x and y pos
    xPos = redrawBlock[1] - redrawBlock[5];
    yPos = (redrawBlock[4] - redrawBlock[6]) - (spriteHeight << yEig);

    // Plot the sprite
    if(spriteArea)
      _swix(OS_SpriteOp, _INR(0, 7), 0x234, spriteArea, spritePointer, xPos, yPos, (1 << 5) | (1 << 6), spriteScaling, transTable);

    // Any more to do?
    if(_swix(Wimp_GetRectangle, _IN(1) | _OUT(0), redrawBlock, &more))
      return;
  }
}

/****************************************************************************/
void displayUpdateMain(int min, int max) {
  // Redraws the main window.
  int redrawBlock[11], more, xPos, yPos;

  // Check if the window is open
  if(!(statusFlags & FLAG_MAIN_OPEN))
    return;

  // Setup block
    redrawBlock[0] = handles[MAIN_WINDOW];
    redrawBlock[1] = 0;

  if(min != -1 && max != -1) {
    redrawBlock[2] = -(max << yEig);
    redrawBlock[3] = spriteWidth  << xEig;
    redrawBlock[4] = -(min << yEig);
  }
  else {
    // Redraw whole screen
    redrawBlock[2] = -65535;
    redrawBlock[3] = 65535;
    redrawBlock[4] = 0;
  }

  // Call swi
  if(_swix(Wimp_UpdateWindow, _IN(1) | _OUT(0), redrawBlock, &more))
    return;

  // Possible rebuild palette here?

  // Main loop
  while(more) {
    // Work out x and y pos
    xPos = redrawBlock[1] - redrawBlock[5];
    yPos = (redrawBlock[4] - redrawBlock[6]) - (spriteHeight << yEig);

    // Plot the sprite
    if(spriteArea)
      _swix(OS_SpriteOp, _INR(0, 7), 0x234, spriteArea, spritePointer, xPos, yPos, (1 << 5) | (1 << 6), spriteScaling, transTable);

    // Any more to do?
    if(_swix(Wimp_GetRectangle, _IN(1) | _OUT(0), redrawBlock, &more))
      return;
  }
}
