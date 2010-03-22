/******************************************************************************

Program:        Common

File:           JPEGView.h

Function:       Header file for the various defines and declarations

Description:    Contains defines and declarations for JPEGView

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 21st May 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __JV_COMMON_H
# define __JV_COMMON_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x) =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

// Includes
#include "stddef.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "swis.h"
#include "../../../Libraries/jpeg-6b/jpeglib.h"

/* Memcheck includes */
#ifdef USE_MEMCHECK
#  include "MemCheck:MemCheck.h"
#endif

/* HierProf includes */
#ifdef HierProf_PROFILE
#  include  "HierProf:HierProf.h"
#endif

/* Constants ---------------------------------------------------------------*/
#define JPEGVIEW_ERROR          0x818400
#define WIMP_POLL_DELAY         100

// Flags
#define FLAG_QUIT            (1 << 0)
#define FLAG_MAIN_OPEN       (1 << 1)

// Windows
#define INFO_WINDOW          0
#define MAIN_WINDOW          1

#define NUM_WINDOWS          2

// Menu defines.
#define MENU_SHADED              0x07400021
#define MENU_UNSHADED            0x07000021
#define MENU_INDIRECTED          0x07000121
#define MENU_NUM_ITEMS           2
#define MENU_INFO                0
#define MENU_QUIT                1

// Variable sizes
#define sizeOfCARD8             1
#define sizeOfCARD16            2
#define sizeOfCARD32            4
#define sizeOfINT8              1
#define sizeOfINT16             2
#define sizeOfINT32             4

/* TypeDefines -------------------------------------------------------------*/

/* Boolean values */
#ifndef bool
#  define bool  unsigned char
#  define true  (1==1)
#  define false !true
#endif

typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
typedef char INT8;

#ifndef JPEGLIB_H
typedef short INT16;
typedef int INT32;
#endif

/* Global Variables --------------------------------------------------------*/
DECLARE CARD32     handles[NUM_WINDOWS];
DECLARE int        mainMenuData[20];
DECLARE INT32      spriteScaling[4];
DECLARE char       JPEGViewName[]      INIT("JPEGView");
DECLARE CARD32     JPEGViewBarIcon     INIT(-1);
DECLARE CARD32     pollDelay           INIT(WIMP_POLL_DELAY);
DECLARE CARD32     statusFlags        INIT(0);
DECLARE CARD32     dynamicArea        INIT(0);
DECLARE INT8       *spriteArea        INIT(NULL);
DECLARE INT8       *spriteBase        INIT(NULL);
DECLARE CARD32     *spritePointer     INIT(NULL);
DECLARE CARD32     *transTable        INIT(NULL);
DECLARE INT32      colourtransPalette[256];
#ifdef DEBUG
DECLARE bool       DEBUG_ON           INIT(true);
#else
DECLARE bool       DEBUG_ON           INIT(false);
#endif

/* Macros ------------------------------------------------------------------*/
#define DBUG(x) {if(DEBUG_ON) {x;}}
#define DBUGF(x) {if(DEBUG_ON) {fprintf(stderr, x);}}

/* Procedure Prototypes ----------------------------------------------------*/

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __JV_COMMON_H
