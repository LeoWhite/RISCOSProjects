/******************************************************************************

Program:        CDPlay

File:           CDPlay.h

Function:       Header File For CDPlay

Description:    Contains List Of Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 11th September 1996

******************************************************************************/

/************************ Constant Definitions *******************************/

/** Boolean values **/
#ifndef BOOL
#define BOOL  unsigned

#define FALSE 0
#define TRUE  1

#define ERROR   1
#define NOERROR 0
#endif

/**  Defines **/
#define maxTrackLength	        68
#define XOS_CLI		        0x20005
#define XOS_Byte	        0x20006
#define XOS_File	        0x20008
#define XOS_Module      	0x2001E
#define SYS_OS_SpriteOp       	0x2002E
#define XOS_ReadMonotonicTime 	0x20042
#define XOS_Plot	      	0x20045
#define XWimp_Initialise	0x600C0
#define SYS_Wimp_CreateWindow 	0x600C1
#define XWimp_CreateIcon 	0x600C2
#define XWimp_DeleteIcon      	0x600C4
#define XWimp_OpenWindow      	0x600C5
#define XWimp_CloseWindow     	0x600C6
#define XWimp_RedrawWindow    	0x600C8
#define XWimp_GetRectangle    	0x600CA
#define XWimp_GetWindowState 	0x600CB
#define XWimp_SetIconState    	0x600CD
#define XWimp_GetIconState    	0x600CE
#define XWimp_GetPointerInfo   	0x600CF
#define XWimp_SetCaretPosition 	0x600D2
#define XWimp_GetCaretPosition 	0x600D3
#define SYS_XWimp_CreateMenu  	0x600D4
#define XWimp_SetExtent  	0x600D7
#define SYS_Wimp_OpenTemplate 	0x600D9
#define SYS_Wimp_CloseTemplate 	0x600DA
#define SYS_Wimp_LoadTemplate 	0x600DB
#define XWimp_ProcessKey      	0x600DC
#define XWimp_CloseDown      	0x600DD
#define SYS_XWimp_StartTask   	0x600DE
#define SYS_Wimp_ReportError  	0x600DF
#define XWimp_PollIdle   	0x600E1
#define SYS_XWimp_SetColour   	0x600E6
#define XWimp_SendMessage 	0x600E7
#define SYS_XWimp_SlotSize    	0x600EC
#define SYS_XWimp_ReadSysInfo 	0x600F2
#define XHourglass_On	      	0x606C0
#define XHourglass_Smash      	0x606C2
#define XTaskManager_EnumerateTasks 0x62681
#define FileWriteAccess       	51
#define FileReadAccess	      	25

/**  Main Window Icons **/
#define iconCDTitle         0
#define iconTrackName       1
#define iconCurrentTime     3
#define iconTrackNumber     4
#define iconMemory	    5
#define iconLast            6
#define iconPlay            7
#define iconNext            8
#define iconStop            9
#define iconEject	    10
#define iconRandom	    11
#define iconVolumeArea	    12
#define baseIconNumber	    13

/** Program Window Icons **/
#define programDataIcon	      0
#define programProgramIcon    1
#define programClearIcon      2

/**  Creating Icons Info **/
#define windowWidth         916
#define windowHeight       -164
#define iconOffSet	     -2
#define iconHeight          -44
#define windowExtentChange  -64
#define iconOneWidth	     64
#define iconTwoWidth	    712
#define iconThreeWidth	    116
#define iconOneXPosition      6
#define iconTwoXPosition     72
#define iconThreeXPosition  794
#define iconOneValue        385900861
#define iconTwoValue	    385935633
#define iconThreeValue	    385876249
#define iconOneSelectValue  387998013
#define iconTwoSelectValue  117500213
#define iconCDTitleValue    385933629
#define iconCDTitleSelectValue 117498173
#define iconTwoFullWidth    827
#define iconThreeFullXPosition 1000
#define iconVolumeRedraw    0xB7202187
#define iconVolumeNoRedraw  0xB7202107

/** Menu Options **/
#define menuInfo	0
#define menuProgram	1
#define menuChoices	2
#define menuSave	3
#define menuQuit	4

/** Slider Info **/
#define maxVolume 	65021
#define minVolume 	0
#define minXPosition	566
#define maxXPosition	898
#define minXSlider	574
#define maxXSlider	888

/** Keycodes Used **/
#define keycode_CURSORDOWN  398
#define keycode_CURSORUP    399
#define keycode_RETURN      13
#define keycode_ESCAPE      27
#define keycode_TAB         394
#define keycode_SHIFT_TAB   410

/** Defines for event types **/
#define messageCode		4
#define messageFiletype 	10
#define messageYourRef		3
#define messageMyRef		2
#define messageAction		4
#define messageSender		1
#define messageFilename		11
#define eventKeyCode		6
#define eventWindowHandle	0
#define eventMouseButton	2
#define eventMouseWindow	3
#define mouseAdjust		1
#define mouseMenu		2
#define mouseSelect		4
#define eventMousePosX		0
#define eventMousePosY		1
#define eventMouseIcon		4
#define eventMenuSelection	0
#define eventCaretIcon		1

#define eventNULL		0            
#define eventREDRAW		1
#define eventOPEN		2
#define eventCLOSE		3
#define eventBUTTON 		6
#define eventKEY		8
#define eventMENU		9
#define eventLOSECARET		11
#define eventGAINCARET		12
#define eventNONZEROPOLLWORD	13
#define eventUSERMESSAGE	17
#define eventUSERMESSAGERECORDED 18
#define eventUSERMESSAGEACK	19
#define messageQUIT		0
#define messageDATALOAD		3
#define messagePREQUIT		8
#define messageMENUSDELETED     0x400C9
#define messageTASKCLOSEDOWN    0x400C3
#define messageDATALOADACK	4

/**  Includes **/
#ifndef __cd_cdplay_h

  #include "CDSWI's.h"

  /**  Clib **/
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "Kernel.h"

  /**  Sets Define To Say Have Been Included **/
  #define __cd_cdplay_h

#endif

/** Linked list header **/
typedef struct linkListHeader
{
  struct linkListHeader *next;
  struct linkListHeader *previous;
} linkListHeader;

/**  Linked List Element For The  Track Information **/
typedef struct
{
  /**  List header **/
  linkListHeader header;
  
  /**  My Data **/
  int trackNumber;
  char trackTextNumber[3];
  char trackName[maxTrackLength+1];
  char trackLength[6];
} listelement;

/** Linked List Element For Memory **/
typedef struct
{
  /** List Header **/
  linkListHeader header;

  /** Memory Data **/
  int trackNumber;
} memoryElement;

/** Flags Setup **/
typedef union
{
  unsigned int value;
  struct
  {
    unsigned int quit		     : 1;
    unsigned int newCD		     : 1;
    unsigned int CDUnknown	     : 1;
    unsigned int driveEmpty	     : 1;
    unsigned int playOverride	     : 1;
    unsigned int autoOpenMain        : 1;
    unsigned int autoPlayCD          : 1;
    unsigned int stopCDOnChange      : 1;
    unsigned int disableCDScanning   : 1;
    unsigned int warnIfNotSaved      : 1;
    unsigned int showLengths         : 1;
    unsigned int saveVolume          : 1;
    unsigned int autoSave            : 1;
    unsigned int stopCDOnQuit        : 1;
    unsigned int autoRepeatCD        : 1;
    unsigned int autoDetectChange    : 1;
    unsigned int checkIfAudio        : 1;
    unsigned int pause		     : 1;
  } data;
} configData;

/******************************* Variables ********************************/

/**  Initialisation Variables **/
extern int task__handle, configtask__handle;
extern char tempData[156], windowData[556], spriteArea[1268],
            *ws, *wsend, *area,task__name[7], task__version[16];
extern int mainMenuArray[38], tempBlock[10];
extern char CDPlayResources [24];

/**  Global Variables **/
extern configData CDFlags;
extern BOOL mainOpen, programOpen, programMode, menuOpen, mainOpened, 
       trackListingAltered, stopped, memory, random, dataCD;
extern linkListHeader listAnchor, memoryAnchor;
extern int mainWindow, programWindow, infoWindow, 
              hotkeysWindow;
extern int driveNumber, CDBlock[5], minTrack, maxTrack, currentTrack, CDDiscID,
           memoryItems, memoryPosition, currentVolume, CDOffset, hotkeyCode,
           timeFormat, minX, minY, maxX, maxY, scrollX, scrollY, 
           startOfNextTrack;
extern char dataCDTitle[maxTrackLength];
extern _kernel_swi_regs regs;

/*************************** Procedure List *******************************/

/**  Main Procedures **/
void taskDispatch(int eventType, int *event);
BOOL mainInitialise(void);
BOOL enumerateTasks(void);
void setupPollWord(void);
void taskExit(void);
int reportError(void);
BOOL shutdown(void);
int main(void);

/**  Window Procedures **/
BOOL windowInitialise(char *filePath);
void windowOpen(int handle, BOOL onTop);
int windowCreate(char *name);
void windowChangeExtent(int numberOfTracks, BOOL update);
void windowCreateIcon(int trackNumber, int yPosition, BOOL update);
void windowLoadSprites(void);
void windowMakeMenu(void);
void windowsRedrawSlider(int handle);
BOOL windowsRedraw(int *redraw, int newVolume);
void windowsLoseCaret(int *event);
void windowsGainCaret(int *event);
void windowsUnsetIcons(void);
void windowsKeyPress(int keycode);
void windowDeleteIcon(int handle, int icon);
void windowProcessKey(int keyCode);
void windowIconGetState(int handle, int icon, int *block);
void windowIconSetState(int handle, int icon, int EORWord, int clearWord);
void windowSetCaret(int handle, int icon);
void windowIconGetText(int handle, int icon, char *text);
void windowIconSetText(int handle, int icon, char *text);
int windowIconGetInteger(int handle, int icon);
void windowIconSetInteger(int handle, int icon, int newContent);
int windowStartTask(char *commandLine);
void windowCheckPlayIcon(void);
                     
/**  Buttons Procedures **/
void buttonsLoadConfig(BOOL update);
void buttonClick(int *event);
void buttonsWindowClick(int *event);
void buttonsChangeTrack(int *event);
void buttonsLast(void);
void buttonsNext(void);
void buttonsPause(void);
void buttonsProgramClick(int *event);
void buttonsMenuClick(int *event);
void buttonsSlider(int *event);

/**  CDBitz Functions **/
void CDInitialise(void);
void CDCheckIfEmpty(void);
void CDGenerateTrackList(void);
BOOL CDIsTrackAudio(int track);
char *CDGenerateTime(int currentPosition);
void CDCheckCD(void);
void CDPlayTrack(int track);
void CDUpdateDiscInfo(void);
int CDStartOfTrack(int trackNumber);
char *CDGenerateTrackLength(int trackNumber);
void CDMoveHead(int amount);

/**  Listing Procedures **/
void listingEraseList(void);
listelement *listingFindLink(int trackNumber);
void listingSearchList(void);
void listingNotFound(void);
memoryElement *listingFindMemoryItem(int trackNumber, linkListHeader *anchor);
void listingClearList(linkListHeader *anchor);
void listingSaveData(void);
int listingGetProgramTrack(void);

/** Linked List Procedures **/
void linkListAddToTail(linkListHeader *anchor, linkListHeader *item);
void linkListUnlink(linkListHeader *anchor, linkListHeader *item);

/**  External Procedures **/
void srand(unsigned int);
