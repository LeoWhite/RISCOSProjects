/******************************************************************************

Program:        VNCInput

File:           VNCInput.h

Function:       Header File For VNCInput

Description:    Contains List Of Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 14th November 1999 - Creation

******************************************************************************/

/* Checks if file already included */
#ifndef __VNC_INPUT_H
#define __VNC_INPUT_H

/* Includes ----------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel.h"
#include "swi.h"
#include "swis.h"
#include "d3des.h"
#include "socket.h"
#include "keycodes.h"

/* Memcheck includes */
#ifdef VNC_MemCheck
#  include "MemCheck:MemCheck.h"
#endif

/* HierProf includes */
#ifdef HierProf_PROFILE
#  include  "HierProf:HierProf.h"
#endif

/* Constants ---------------------------------------------------------------*/

/** Task name. */
#define VNCTaskName              "VNCInput"
#define VNCVersion               "0.02 (29-05-02)"

/** Window positions. */
#define WINDOW_CENTERED          (1 << 0)
#define WINDOW_UNDER_PTR         (1 << 1)
#define WINDOW_ON_TOP            (1 << 2)
#define WINDOW_ON_BOTTOM         (1 << 3)
#define WINDOW_ON_BEHIND         (1 << 4)
#define WINDOW_FULL_SCREEN       (1 << 5)

/* Messages */
#define messageQUIT		 0
#define messageDATASAVE          1
#define messageDATASAVEACK       2
#define messagePREQUIT		 8
#define messageMODECHANGE	 0x400C1

/* Error Messages */
#define ERROR_INIT               (1 << 30)
#define ERROR_TEMPLATE           (1 << 30) + 1
#define ERROR_CONNECT            (1 << 30) + 2

/** Info window icons. */
#define INFO_NAME                0
#define INFO_PURPOSE             1
#define INFO_AUTHOR              2
#define INFO_VERSION             3

/** Password window icons. */
#define PASSWORD_PROMPT          0
#define PASSWORD_INPUT           1

/** Connect window icons. */
#define CONNECT_HOSTNAME         0
#define CONNECT_CONNECT          3
#define CONNECT_CANCEL           4
#define CONNECT_MENU             5

/** Connecting window. */
#define CONNECTING_TOP           0
#define CONNECTING_MIDDLE        1
#define CONNECTING_BOTTOM        2

/** Flags */
#define FLAG_QUIT                (1 << 0)
#define FLAG_CONNECTED           (1 << 1)
#define FLAG_INITIALISED         (1 << 2)
#define FLAG_WINDOW_OPEN         (1 << 3)

/** Menu defines. */
#define MENU_SHADED              0x07400021
#define MENU_UNSHADED            0x07000021
#define MENU_INDIRECTED          0x07000121
#define MENU_NUM_ITEMS           4
#define MENU_INFO                0
#define MENU_CONNECT             1
#define MENU_DISCONNECT          2
#define MENU_QUIT                3

#define CONNECT_ITEM             ((6 * MENU_CONNECT) + 9)
#define DISCONNECT_ITEM          ((6 * MENU_DISCONNECT) + 9)

/* Wimp Events */
#define eventNULL                0
#define eventREDRAW              1
#define eventOPEN		 2
#define eventCLOSE		 3
#define eventPOINTERLEAVE	 4
#define eventPOINTERENTER	 5
#define eventBUTTON 	         6
#define eventDRAGEND             7
#define eventKEY		 8
#define eventMENU		 9
#define eventLOSECARET		 11
#define eventGAINCARET		 12
#define eventNONZEROPOLLWORD	 13
#define eventUSERMESSAGE	 17
#define eventUSERMESSAGERECORDED 18

#define EVENT_WINDOW_HANDLE	0
#define EVENT_MENU_SELECTION	0
#define EVENT_POINTER_WINDOW    0
#define EVENT_MOUSE_POS_X	0
#define EVENT_MOUSE_POS_Y	1
#define EVENT_MOUSE_BUTTON	2
#define EVENT_MOUSE_WINDOW	3
#define EVENT_MOUSE_ICON	4
#define EVENT_KEY_CODE		6
#define MESSAGE_CODE            4

/* Mouse buttons */
#define MOUSE_ADJUST		1
#define MOUSE_MENU		2
#define MOUSE_SELECT		4

/** Key mouse values. */
#define KEY_MOUSE_SELECT        0x70
#define KEY_MOUSE_MENU          0x71
#define KEY_MOUSE_ADJUST        0x72

/* VNC Defines */
#define VNCBaseClientPort	5900
#define VNCChallengeSize	16
#define VNCVersionLength        12
#define VNCPasswordLength       8
#define VNCMajorVersion         003
#define VNCMinorVersion         005
#define VNCServerInitSize       16
#define VNCKeyEventSize         8
#define VNCPointerSize          6
#define VNCUpdateSize           10
#define VNCUpdateBufferSize     12
#define VNCRRESize              8
#define VNCCoRRESize            4

/* VNC Authentication defines */
#define VNCPasswordSize         4
#define VNCConnectionFailed     0
#define VNCNoAuthentication     1
#define VNCAuthenticationReq    2
#define VNCPasswordOK           0
#define VNCPasswordFailed       1
#define VNCPasswordTooMany      2

/* VNC message type */
#define VNCMessageBufferUpdate  0
#define VNCMessageSetColourMap  1
#define VNCMessageBell          2
#define VNCMessageServerCut     3

/* Encoding types */
#define VNCEncodingRaw          0

/** Network status. */
#define NS_VERSION              1
#define NS_PASSWORD             2
#define NS_WAITING_FOR_ENCRYPT  3
#define NS_PASSWORD_RESPONSE    4
#define NS_SERVER_INIT          5

/* Variable sizes */
#define sizeOfCARD8             1
#define sizeOfCARD16            2
#define sizeOfCARD32            4
#define sizeOfINT8              1
#define sizeOfINT16             2
#define sizeOfINT32             4

/** Pollword defines. */
#define POLL_POLLWORD           1
#define POLL_CONFIG             2

#define POLL_MOUSE_WATCH        (1 << 0)
#define POLL_KEY_WATCH          (1 << 1)
#define POLL_EDGE_DETECT        (1 << 2)

#define POLL_PRESSED            0x00FF0000
#define POLL_RELEASED           0xFF000000

/* Misc defines */
#define WIMP_MASK		0x401832    /* Pollword requests */
#define iconInfo		0x7006103
#define BEL                     0x0000007
#define WindowFurniture         0xFF
#define NoWindowFurniture       0x80
#define NoWindowBorder          0xFF

/* TypeDefines -------------------------------------------------------------*/

/* Boolean values */
#ifndef BOOL
#  define BOOL  unsigned int
#  define TRUE  (1==1)
#  define FALSE !TRUE
#endif

typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
typedef char INT8;
typedef short INT16;
typedef int INT32;

/* Pixel format structure */
typedef struct
{
  CARD8 bitsPerPixel;           /* Bits per pixel */
  CARD8 depth;                  /* Depth (Number of colours) */
  CARD8 bigEndian;              /* 0 = little endian, <>0 = Big Endian */
  CARD8 trueColour;             /* 0 = Pseudo colour, <>0 = True Colour */
  CARD16 redMax;                /* Max Red Value */
  CARD16 greenMax;              /* Max Green Value */
  CARD16 blueMax;               /* Max Blue Value */
  CARD8 redShift;               /* Red shift value */
  CARD8 greenShift;             /* Green shift value */
  CARD8 blueShift;              /* Blue shift value */
  CARD8 pad1;                   /* Padding */
  CARD16 pad2;                  /* Padding */
} rfbPixelFormat;

/* Size structure */
typedef struct
{
  CARD16 Width;
  CARD16 Height;
} rfbSize;

/* Update buffer structure */
typedef struct
{
  CARD8 MessageType;
  CARD8 Redraw;
  CARD16 X;
  CARD16 Y;
  CARD16 Width;
  CARD16 Height;
} rfbUpdateRequest;

/* Pointer info */
typedef struct
{
  CARD8  MessageType;           /* Message type */
  CARD8  ButtonMask;            /* Button Mask */
  CARD16 XPosition;             /* X Position */
  CARD16 YPosition;             /* Y Position */
} rfbPointerEvent;

/* Key press info */
typedef struct
{
  CARD8  MessageType;           /* Message Type */
  CARD8  DownFlag;              /* Key up or down */
  CARD16 Padding;               /* Padding */
  CARD32 key;                   /* Key */
} rfbKeyEvent;

/* Encoding Rectangle details */
typedef struct
{
  CARD16 X;
  CARD16 Y;
  CARD16 Width;
  CARD16 Height;
  CARD32 EncodingType;
} rfbUpdateBuffer;

/* RRE Encoding struct */
typedef struct
{
  CARD16 X;
  CARD16 Y;
  CARD16 Width;
  CARD16 Height;
} rfbRRE_s;

/* C0RRE Encoding struct */
typedef struct
{
  CARD8 X;
  CARD8 Y;
  CARD8 Width;
  CARD8 Height;
} rfbCoRRE_s;

/* Configuration details */
typedef struct VNCConfig_s {
  CARD16 width;
  CARD16 height;
} VNCConfig_s;

struct socketBuff_st {
  int size;             /** Amount of data held. */
  int offset;           /** Amount of data already read. */
  char *data;           /** Pointer to the meory. */
  struct socketBuff_st *next;  /** Pointer to the next buffer. */
};

typedef struct socketBuff_st socketBuff;

/* Global Variables --------------------------------------------------------*/
extern char *errorMessage;
extern int error, VNCFlags, VNCTaskHandle, VNCBarIcon, riscosVersion;
extern int infoWindow, connectWindow, connectingWindow, passwordWindow, mainWindow, mainMenuData[32];
extern int screenX, screenY, xEig, yEig, *hostMenuData, resolverStatus, networkStatus;
extern BOOL doubleHeightMode, hostOpen;
extern int VNCSocket, VNCPort, *pollword;
extern socketBuff *memBuffer;

extern VNCConfig_s VNCConfig;

/* Procedure Prototypes ----------------------------------------------------*/

/** Main */
void mainTaskDispatch(CARD16 eventType, int *event);
BOOL mainInitialise(void);
void mainVNCTaskExit(void);
BOOL mainSetupPollWord(void);
int mainReportError(char *errorStr, int errorNum, int flags);

/** Window procedures. */
int windowCreateIconbar(char *spriteName, int position);
BOOL windowInit(char *filePath);
int windowCalculateBuffer(char *name, BOOL bufferType);
int windowCreate(char *name, int *regs);
char *windowIconGetText(CARD32 handle, CARD16 icon);
void windowIconSetText(CARD32 handle, CARD16 icon, char *text);
int windowIconGetInteger(CARD32 handle, CARD16 icon);
void windowIconSetInteger(CARD32 handle, CARD16 icon, int newContent);
void windowIconGetState(CARD32 handle, CARD16 icon, int *block);
void windowIconSetState(CARD32 handle, CARD16 icon, CARD32 EORWord, CARD32 clearWord);
void windowCreateMainMenu(void);
void windowOpen(CARD32 handle, CARD8 position);
void windowSetCaret(CARD32 handle, int icon);
void windowModeChange(void);
void windowChangeExtent(CARD32 handle, CARD16 width, CARD16 height);

/** Button procedures. */
void buttonClick(int *event);
void buttonMenuClick(int *event);
void buttonConnectWindow(int *event);
void buttonKeyPress(int *event);
void buttonOpenMenu(int *event);
void buttonLoadMenu(void);

/** Sockets. */
int socketGenerate(char *hostname, int port);
struct hostent *resolveName(char *hostname);
int inet_pton(char *src, CARD8 *dst);
void socketProcessInitInput(void);
int socketGetBufferSize(void);
int socketFillBuffers(void);
int socketReadData(void *buffer, int amount);

/** VNC procedures. */
void VNCOpenConnection(void);
void VNCCloseConnection(void);
void VNCProcessVersion(void);
void VNCGetPassword(void);
void VNCAuthentication(void);
void VNCEncryptBytes(unsigned char *bytes, unsigned char *password);
void VNCPasswordResponse(void);
void VNCInitialiseDisplay(void);
void VNCSetEncoding(void);
void VNCProcessMouse(void);
void VNCProcessWimpKey(int keyCode);
void VNCProcessKey(int keyNumber, CARD8 keyUpDown);
void VNCProcessEdgeDetect(void);
void VNCBoundingBox(int x,  int y, short int width, short int height);

/* Assembler included calls */
unsigned long int htonl(unsigned long int Value);
unsigned long int ntohl(unsigned long int Value);
unsigned short int htons(unsigned short int Value);
unsigned short int ntohs(unsigned short int Value);
int ioctl(int Socket, int Request, void *Flags);
int disableNagle(int Socket, void *Flags, int flagSize);
int create(int Family, int Type, int Protocol);
int close(int Socket);
int read(int Socket, void *Buffer, int Amount);
int connect(int Socket, struct sockaddr *HostAddr, int NameLength);
int write(int Socket, void *Buffer, int Amount);

int wimpEnumerateTasks(char *taskName);

#endif  /* #ifndef __VNC_INPUT_H */
