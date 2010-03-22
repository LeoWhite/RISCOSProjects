/******************************************************************************

Program:        JPEGView

File:           JPEGView.h

Function:       Header file for the various defines and declarations

Description:    Contains defines and declarations for JPEGView

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Fri 19th May 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __JV_JPEGVIEW_H
# define __JV_JPEGVIEW_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

/* Memcheck includes */
#ifdef USE_MEMCHECK
#  include "MemCheck:MemCheck.h"
#endif

/* HierProf includes */
#ifdef HierProf_PROFILE
#  include  "HierProf:HierProf.h"
#endif

/* Constants ---------------------------------------------------------------*/
#define WIMP_MASK            0x3930
#define TEMPLATE_FILE        "<JPEGView$Dir>.Templates"
#define MESSAGES_FILE        "<JPEGView$Dir>.Messages"

// Error messages
static _kernel_oserror errFailedAtExit    = {JPEGVIEW_ERROR, "JPEGView failed to add atexit handler."};
static _kernel_oserror errFailedIconbar   = {JPEGVIEW_ERROR+1 , "JPEGView failed to create iconbar icon."};

/* Global Variables --------------------------------------------------------*/
DECLARE CARD32 JPEGViewTaskHandle INIT(-1);

/* Procedure Prototypes ----------------------------------------------------*/
DECLARE void mainTaskDispatch(CARD16 eventType, INT32 *event);
DECLARE void mainTaskExit(void);
DECLARE _kernel_oserror *mainInitialise(void);
DECLARE void mainTaskExit(void);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __JV_JPEGVIEW_H
