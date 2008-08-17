/******************************************************************************

Program:        JPEGView

File:           Button.h

Function:       Header file for the various button and menu calls

Description:    Handles mouse clicks

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Fri 19th May 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __JV_BUTTON_H
# define __JV_BUTTON_H
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


/* Global Variables --------------------------------------------------------*/

/* Procedure Prototypes ----------------------------------------------------*/
DECLARE void buttonClick(int *event);
DECLARE void buttonMenuClick(int *event);
DECLARE void buttonOpenMenu(int *event);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __JV_BUTTON_H
