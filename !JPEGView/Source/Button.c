/******************************************************************************

Program:        JPEGView

File:           Button.c

Function:       Displays JPEGs

Description:    Button and menu processing routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 21st May 2000

******************************************************************************/

// Includes
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "button.h"
#include "wimp.h"
#include "display.h"

// Global variables

/****************************** Procedures. *********************************/
void buttonClick(int *event) {
  // Handles the various button clicks.

  // Check if its a menu click
  if(event[EVENT_MOUSE_BUTTON] == MOUSE_MENU) {
    // Is it on the iconbar?
    if(event[EVENT_MOUSE_ICON] == JPEGViewBarIcon && event[EVENT_MOUSE_WINDOW] == -2)  {
      // Open the menu
      buttonOpenMenu(event);
    }
  }
  else if(event[EVENT_MOUSE_ICON] == JPEGViewBarIcon && event[EVENT_MOUSE_WINDOW] == -2) {
    // If the sprite exists, then open the window
    if(displaySetupPointers()) {
      wimpOpenWindow(handles[MAIN_WINDOW], WINDOW_ON_TOP | WINDOW_CENTERED);
      statusFlags |= FLAG_MAIN_OPEN;
    }
  }
  else if(event[EVENT_MOUSE_WINDOW] == handles[MAIN_WINDOW]) {
    // Click on main window, so give it the focus
    wimpSetCaret(handles[MAIN_WINDOW], -1);

    // (Could do with 'faking' the mouse click here.
  }
}

/****************************************************************************/
void buttonMenuClick(int *event) {
  // Check which menu item has been selected
  int ptr[5];

  switch(event[EVENT_MENU_SELECTION]) {
    // Info selected, so open info window
    case MENU_INFO:wimpOpenWindow(handles[INFO_WINDOW], WINDOW_UNDER_PTR | WINDOW_ON_TOP);
    break;

    // Quit
    case MENU_QUIT:statusFlags |= FLAG_QUIT;
    break;
  }

  // Check if we need to reopen the window
  _swix(Wimp_GetPointerInfo, _IN(1), ptr);
  if(ptr[2] == MOUSE_ADJUST)
    buttonOpenMenu(event);
}

/****************************************************************************/
void buttonOpenMenu(int *event) {
  // Opens the menu
  _swix(Wimp_CreateMenu, _INR(0, 3), 0, mainMenuData, event[EVENT_MOUSE_POS_X] - 64,
                         96 + MENU_NUM_ITEMS * 44);
}
