/******************************************************************************

Program:        Yet Another Wimp Library

File:           Wimp.h

Function:       Header file for the various wimp calls

Description:    Contains defines and declarations for Wimp functions

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 30th January 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __YAWL_WIMP_H
# define __YAWL_WIMP_H
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

// Misc defines
#define WIMP_TASK              0x4B534154   // 'TASK'

// WIMP poll event values
#define WIMP_NULL              0
#define WIMP_REDRAW            1
#define WIMP_OPEN              2
#define WIMP_CLOSE             3
#define WIMP_POINTER_LEAVE     4
#define WIMP_POINTER_ENTER     5
#define WIMP_MOUSE_CLICK       6
#define WIMP_USER_DRAG_BOX     7
#define WIMP_KEY_PRESSED       8
#define WIMP_MENU_SELECTION    9
#define WIMP_SCROLL_REQUEST    10
#define WIMP_LOSE_CARET        11
#define WIMP_GAIN_CARET        12
#define WIMP_POLLWORD_NON_ZERO 13
#define WIMP_USER_MESSAGE      17
#define WIMP_USER_MESSAGE_REC  18
#define WIMP_USER_MESSAGE_ACK  19

// Offset into WIMP event buffer
#define EVENT_WINDOW_HANDLE    0
#define EVENT_MENU_SELECTION   0
#define EVENT_POINTER_WINDOW   0
#define EVENT_MOUSE_POS_X      0
#define EVENT_MOUSE_POS_Y      1
#define EVENT_MOUSE_BUTTON     2
#define EVENT_MOUSE_WINDOW     3
#define EVENT_MOUSE_ICON       4
#define EVENT_KEY_CODE         6

// Open window block
#define WIMP_MIN_X             1
#define WIMP_MIN_Y             2
#define WIMP_MAX_X             3
#define WIMP_MAX_Y             4
#define WIMP_SCROLL_X          5
#define WIMP_SCROLL_Y          6

// WIMP message numbers
#define MESSAGE_QUIT           0
#define MESSAGE_DATASAVE       1
#define MESSAGE_DATASAVEACK    2
#define MESSAGE_DATALOAD       3
#define MESSAGE_DATALOADACK    4
#define MESSAGE_DATAOPEN       5
#define MESSAGE_RAMFETCH       6
#define MESSAGE_RAMTRANSMIT    7
#define MESSAGE_PREQUIT        8
#define MESSAGE_PALETTECHANGE  9
#define MESSAGE_SAVEDESKTOP    10
#define MESSAGE_DEVICECLAIM    11
#define MESSAGE_DEVICEINUSE    12
#define MESSAGE_DATASAVED      13
#define MESSAGE_SHUTDOWN       14
#define MESSAGE_MODECHANGED    0x400C1

// Filer messages
#define MESSAGE_FILEROPENDIR            0x400
#define MESSAGE_FILERCLOSEDIR           0x401
#define MESSAGE_FILEROPENDIRAT          0x402
#define MESSAGE_FILERSELECTIONDIRECTORY 0x403
#define MESSAGE_FILERADDSELECTION       0x404
#define MESSAGE_FILERACTION             0x405
#define MESSAGE_FILERCONTROLACTION      0x406
#define MESSAGE_FILERSELECTION          0x407

// WIMP messages
#define MESSAGE_MENUWARNING             0x400C0
#define MESSAGE_MODECHANGE              0x400C1
#define MESSAGE_TASKINITIALISE          0x400C2
#define MESSAGE_TASKCLOSEDOWN           0x400C3
#define MESSAGE_SLOTSIZE                0x400C4
#define MESSAGE_SETSLOT                 0x400C5
#define MESSAGE_TASKNAMERQ              0x400C6
#define MESSAGE_TASKNAMEIS              0x400C7
#define MESSAGE_TASKSTARTED             0x400C8
#define MESSAGE_MENUSDELETED            0x400C9

// Iconbar position
#define ICONBAR_RIGHT                   -1
#define ICONBAR_LEFT                    -2
#define ICONBAR_LEFT_ICON               -3
#define ICONBAR_RIGHT_ICON              -4
#define ICONBAR_LEFT_SCAN_LEFT          -5
#define ICONBAR_LEFT_SCAN_RIGHT         -6
#define ICONBAR_RIGHT_SCAN_LEFT         -7
#define ICONBAR_RIGHT_SCAN_RIGHT        -8

// Window positions
#define WINDOW_CENTERED                 (1 << 0)
#define WINDOW_UNDER_PTR                (1 << 1)
#define WINDOW_ON_TOP                   (1 << 2)
#define WINDOW_ON_BOTTOM                (1 << 3)
#define WINDOW_ON_BEHIND                (1 << 4)
#define WINDOW_FULL_SCREEN              (1 << 5)

/* Mouse buttons */
#define MOUSE_ADJUST		1
#define MOUSE_MENU		2
#define MOUSE_SELECT		4

// Error flags.
#define WIMP_ERROR_OK           (1 << 0)
#define WIMP_ERROR_CANCEL       (1 << 1)
#define WIMP_ERROR_HCANCEL      (1 << 2)
#define WIMP_ERROR_NOSPACE      (1 << 3)
#define WIMP_ERROR_NOPREFIX     (1 << 4)
#define WIMP_ERROR_RETURN       (1 << 5)
#define WIMP_ERROR_SELECT       (1 << 6)
#define WIMP_ERROR_QUIET        (1 << 7)
#define WIMP_ERROR_NEW          (1 << 8)
#define WIMP_ERROR_OLDERROR     (0)
#define WIMP_ERROR_INFOREPORT   (1 << 9)
#define WIMP_ERROR_ERRORREPORT  (1 << 10)
#define WIMP_ERROR_PROGREPORT   (1 << 9 || 1 << 10)
#define WIMP_ERROR_QUESTION     (1 << 11)

// Wimp message structures
#define MESSAGE_SIZE            0
#define MESSAGE_SENDER          1
#define MESSAGE_MY_REF          2
#define MESSAGE_YOUR_REF        3
#define MESSAGE_ACTION          4
#define MESSAGE_DATA            5
#define MESSAGE_WINDOW          5
#define MESSAGE_ICON            6
#define MESSAGE_X               7
#define MESSAGE_Y               8
#define MESSAGE_FILESIZE        9
#define MESSAGE_FILETYPE        10
#define MESSAGE_FILENAME        11

/* Global Variables --------------------------------------------------------*/
DECLARE int  riscosVersion     INIT(0);
DECLARE INT16 xEig             INIT(1);
DECLARE INT16 yEig             INIT(1);
DECLARE INT16 screenX          INIT(0);
DECLARE INT16 screenY          INIT(0);
DECLARE bool doubleHeightMode  INIT(false);

/* Procedure Prototypes ----------------------------------------------------*/
DECLARE bool wimpEnumerateTasks(const char *appName);
DECLARE void wimpTaskCloseDown(CARD32 taskHandle);
DECLARE CARD32 wimpReportError(const _kernel_oserror *error, const CARD32 errorFlags, const char *appName,
                               const char *spriteName, const CARD32 spriteArea, const char *buttons);
DECLARE INT32 *moduleGetPollword(const char *moduleName);
DECLARE CARD32 wimpCreateIconbar(const char *spriteName, const INT16 position);
DECLARE char *wimpLoadTemplate(const char *filePath, const char *names[], CARD32 *handles);
DECLARE char *wimpGetIconText(CARD32 handle, CARD16 icon);
DECLARE bool wimpSetIconText(CARD32 handle, CARD16 icon, char *text);
DECLARE CARD32 wimpGetIconInteger(CARD32 handle, CARD16 icon);
DECLARE bool wimpSetIconInteger(CARD32 handle, CARD16 icon, int value);
DECLARE void wimpGetIconState(CARD32 handle, CARD16 icon, CARD32 *block);
DECLARE void wimpSetIconState(CARD32 handle, CARD16 icon, CARD32 EORWord, CARD32 clearWord);
DECLARE void wimpSetCaret(CARD32 handle, INT16 icon);
DECLARE void wimpOpenWindow(CARD32 handle, CARD8 position);
DECLARE void wimpSetExtent(CARD32 handle, CARD16 width, CARD16 height);
DECLARE void wimpModeChange(void);
DECLARE void wimpCreateMainMenu(void);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __YAWL_WIMP_H
