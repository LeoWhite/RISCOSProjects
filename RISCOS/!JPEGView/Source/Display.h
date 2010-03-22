/******************************************************************************

Program:        JPEGView

File:           Display.h

Function:       Header file for the various button and menu calls

Description:    Handles displaying output

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Fri 19th May 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __JV_DISPLAY_H
# define __JV_DISPLAY_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x)   =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

// Includes

/* Constants ---------------------------------------------------------------*/
DECLARE CARD16 spriteWidth  INIT(0);
DECLARE CARD16 spriteHeight INIT(0);


/* Global Variables --------------------------------------------------------*/

/* Procedure Prototypes ----------------------------------------------------*/
DECLARE bool displaySetupPointers(void);
DECLARE void displayRedrawMain(CARD32 handle);
DECLARE void displayUpdateMain(int min, int max);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __JV_DISPLAY_H
