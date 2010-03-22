/******************************************************************************

Program:        CDPlay

File:           Window.c

Function:       CD Player Wimp C Program


Description:    Window Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 11th September 1996

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/*************************** Initialisation **********************************/
BOOL windowInitialise(char *filePath)
{
  /** Loads In The Templates etc, **/
  regs.r[0] = 0;
  regs.r[1] = (int)&filePath[0];
  _kernel_swi(SYS_Wimp_OpenTemplate, &regs, &regs);

  /** Creates The Windows **/
  mainWindow = windowCreate("Main");
  infoWindow = windowCreate("Info");
  programWindow = windowCreate("Program");
  hotkeysWindow = windowCreate("Hotkeys");

  /** Closes The Template File **/
  _kernel_swi(SYS_Wimp_CloseTemplate, &regs, &regs);

  /** Open Hotkeys Window **/
  windowOpen(hotkeysWindow, FALSE);
  
  /** Copies The Version Number Into The Info Box **/
  windowIconSetText(infoWindow, 0, task__name);
  windowIconSetText(infoWindow, 1, "Filer Like CD Player");
  windowIconSetText(infoWindow, 2, "Leo White");
  windowIconSetText(infoWindow, 3, task__version);

  /** Returns TRUE **/
  return(TRUE);
}

/*****************************************************************************/
void windowOpen(int handle, BOOL onTop)
{
  /** Handles Opening Windows **/
  int ptrBlock[5];
  
  /** Gets Window State **/
  tempBlock[0] = handle;
  regs.r[1] = (int)&tempBlock;
  _kernel_swi(XWimp_GetWindowState, &regs, &regs);

  /** Sets Window Position **/
  if(!mainOpened && handle == mainWindow)
  {
    tempBlock[1] = minX;
    tempBlock[2] = minY;
    tempBlock[3] = maxX;
    tempBlock[4] = maxY;
    tempBlock[5] = scrollX;
    tempBlock[6] = scrollY;
    mainOpened = TRUE;
  }

  /** Gets Pointer Info **/
  regs.r[1] = (int)&ptrBlock;
  _kernel_swi(XWimp_GetPointerInfo, &regs, &regs);
  
  if(handle == programWindow && !programOpen)
  {
    tempBlock[3] = tempBlock[3] - tempBlock[1] + ptrBlock[0];
    tempBlock[1] = ptrBlock[0] - 30;
    tempBlock[2] = tempBlock[2] - tempBlock[4] + ptrBlock[1];
    tempBlock[4] = ptrBlock[1] + 30;
  }
  
  if(onTop)
    /** Makes Sure Window Appears On Top **/
    tempBlock[7] = -1;

  /** Opens The Window **/
  regs.r[1] = (int)&tempBlock;
  _kernel_swi(XWimp_OpenWindow, &regs, &regs);
}
  
/*****************************************************************************/
int windowCreate(char *name)
{
  /** Creates The Window **/
  
  /** Loads In Template File **/
  regs.r[1] = (int)area;
  regs.r[2] = (int)ws;
  regs.r[3] = (int)wsend;
  regs.r[4] = -1;
  regs.r[5] = (int)&name[0];
  regs.r[6] = 0;
  _kernel_swi(SYS_Wimp_LoadTemplate, &regs, &regs);
  ws = (char *)regs.r[2];
  
  /** Sets The Sprite Area Pointer **/
  strcpy(&area[64],(char *)&area);

  /** Creates Window **/
  _kernel_swi(SYS_Wimp_CreateWindow, &regs, &regs);

  return(regs.r[0]);
}
  
/*****************************************************************************/
void windowChangeExtent(int numberOfTracks, BOOL update)
{
  /** Adjusts The Size Of The Window **/
  int counter, yPosition;
  
  /** Allocates Memory To Make Sure We Have Enough **/
  free(calloc(numberOfTracks, sizeof(tempBlock)));

  /** Creates The Icons **/
  yPosition = windowHeight + iconOffSet;
  for(counter = 1; counter <= numberOfTracks;counter++)
  {
    windowCreateIcon(counter, yPosition, update);
    yPosition += iconHeight + 2 * iconOffSet;
  }

  /** Adjusts Values **/
  tempBlock[0] = 0;
  tempBlock[2] = windowWidth;
  tempBlock[1] = (numberOfTracks * (iconHeight + (2 * iconOffSet))) + windowHeight;
  tempBlock[3] = 0;

  /** Changes Extent **/
  regs.r[0] = mainWindow;
  regs.r[1] = (int)&tempBlock;
  _kernel_swi(XWimp_SetExtent, &regs, &regs);
}

/*****************************************************************************/
void windowCreateIcon(int trackNumber, int yPosition, BOOL update)
{
  /** Creates The Icons Required For Each Track **/
  int iconHandle;
  listelement *el;

  if(!update)
  {
    /** Allocates Memory For Linked List Element **/
    el = (listelement *) calloc(1, sizeof(listelement));

    /** Adds Values **/
    el->trackNumber = trackNumber;

    /** Works Out Play Time Of Track **/
    strcpy(el->trackLength, CDGenerateTrackLength(trackNumber));
    strcpy(el->trackName, "Track Name Unknown\n\0");
    sprintf(el->trackTextNumber, "%i", trackNumber);
  }
  else
  {
    el = (listelement *) listingFindLink(trackNumber);
  }
  
  /** Sets Global Data **/
  regs.r[1] = (int)&tempBlock;
  tempBlock[0] = mainWindow;
  tempBlock[2] = yPosition + iconHeight;
  tempBlock[4] = yPosition;

  /** Sets Up Info For Each Icon And Creates It **/
  tempBlock[1] = iconOneXPosition;
  tempBlock[3] = iconOneWidth + iconOneXPosition;
  tempBlock[6] = (int)(el->trackTextNumber);
  tempBlock[7] = (int)"R5";
  tempBlock[8] = 3;
  tempBlock[5] = iconOneValue;
  regs.r[0] = (int)&iconHandle;
  _kernel_swi(XWimp_CreateIcon, &regs, &regs);

  /** Icon Two **/
  tempBlock[1] = iconTwoXPosition;
  if(CDFlags.data.showLengths)
    tempBlock[3] = iconTwoWidth + iconTwoXPosition;
  else
   tempBlock[3]  = iconTwoFullWidth + iconTwoXPosition;
  tempBlock[6] = (int)(el->trackName);
  tempBlock[7] = (int)"Ktranc";
  tempBlock[8] = maxTrackLength-2;
  tempBlock[5] = iconTwoValue;
  _kernel_swi(XWimp_CreateIcon, &regs, &regs);

  /** Icon Three **/
  if(CDFlags.data.showLengths)
    tempBlock[1] = iconThreeXPosition;
  else
    tempBlock[1] = iconThreeFullXPosition;
  tempBlock[3] = iconThreeWidth + iconThreeXPosition;
  tempBlock[6] = (int)(el->trackLength);
  tempBlock[7] = (int)"";
  tempBlock[8] = 6;
  tempBlock[5] = iconThreeValue;
  _kernel_swi(XWimp_CreateIcon, &regs, &regs);

  /** Add Element To End Of List **/
  if(!update)
    linkListAddToTail(&listAnchor,&(el->header));
}

/*****************************************************************************/
void windowLoadSprites(void)
{
  /** Old Routies For Dynamically Allocationg Memory **/
  /*_kernel_osfile_block osfile;*/
  
  /** Gets File Size **/
  /*_kernel_osfile(17,filePath, &osfile);*/
  
  /** Allocates Memory To Hold Sprites **/
  /* area = (sprite_area) malloc(osfile.start+16); */

  /** Loads In And Stores the Sprite Information **/
  
  /** Loads In Sprite File **/
  regs.r[0] = 2;
  _kernel_swi(SYS_XWimp_ReadSysInfo, &regs, &regs);
  
  regs.r[1] = (int)area;
  sprintf(tempData, "%sSprites", CDPlayResources);

  if(strcmp("22", (char *)regs.r[0]) == 0)
    /** Hi-Res Sprites **/
    strcat(tempData, "22");

  regs.r[0] = 0x10A;
  regs.r[2] = (int)tempData;
  _kernel_swi(SYS_OS_SpriteOp, &regs, &regs);
}  

/*****************************************************************************/
void windowMakeMenu(void)
{
  /** Sets Up The Data Block For The Main Menu **/
  char *menspc;
  int *item;
  
  /** Sets Up Pointers **/
  menspc = (char *)&mainMenuArray[0];
  
  /** Creates Menu **/
  strcpy(menspc, task__name);
  menspc[12] = 7;
  menspc[13] = 2;
  menspc[14] = 7;
  menspc[15] = 0;
  menspc[16] = 10*16+32;
  mainMenuArray[5] = 44;
  mainMenuArray[6] = 1;
    
  /** Data For Info Icon **/
  item = &mainMenuArray[7];
  item[0] = 0;
  item[1] = infoWindow;
  item[2] = 0x07000021;
  strcpy((char *)&item[3],"Info");
  
  /** Data For Program Icon **/
  item = &mainMenuArray[13];
  item[0] = 0;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char*)&item[3],"Program...");

  /** Data For Choices Icon **/
  item = &mainMenuArray[19];
  item[0] = 0;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char *)&item[3],"Choices...");

  /** Data For Save Icon **/
  item = &mainMenuArray[25];
  item[0] = 0;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char *)&item[3],"Save");

  /** Data For Quit Icon **/
  item = &mainMenuArray[31];
  item[0] = 0x80;
  item[1] = NULL;
  item[2] = 0x07000021;
  strcpy((char *)&item[3],"Quit");        

}

/*****************************************************************************/
void windowsRedrawSlider(int handle)
{
  /** Redraws Slider Icon **/
  int newVolume = 0, redraw[11];
  
  /** Reads In Volume **/
  regs.r[0] = 0;
  regs.r[1] = (int)&tempBlock;
  if(_kernel_swi(XCD_GetAudioParameters, &regs, &regs) == NULL)
    newVolume = tempBlock[0]/207;
  else
    newVolume = 0;
  
  /** Checks Volume **/
  if(newVolume > (maxXSlider - minXSlider))
    newVolume = maxXSlider - minXSlider;
    
  /** Sets Up Redraw Loop **/
  redraw[0] = handle;
  regs.r[1] = (int)&redraw;
  _kernel_swi(XWimp_RedrawWindow, &regs, &regs);
  
  while(regs.r[0])
  {
    windowsRedraw(redraw, newVolume);
    regs.r[1] = (int)&redraw;
    _kernel_swi(XWimp_GetRectangle, &regs, &regs);
  }
  
  /** Sets Volume **/
  currentVolume = newVolume;
}

/*****************************************************************************/
BOOL windowsRedraw(int *redraw, int newVolume)
{

  redraw[4] -= redraw[6];
  redraw[1] += redraw[5];  

  if (redraw[2]-140 > redraw[10])  return(FALSE);
  if (redraw[4]-124 < redraw[8])  return(FALSE);

  if (redraw[1]+minXSlider > redraw[9])  return(FALSE);
  if (redraw[3]+maxXSlider < redraw[7])  return(FALSE);

  /** Clears Icon **/
  regs.r[0] = 0;
  _kernel_swi(SYS_XWimp_SetColour, &regs, &regs);
  
  regs.r[0] = 4;
  regs.r[1] = redraw[1] + minXSlider;
  regs.r[2] = redraw[4] - 140;
  _kernel_swi(XOS_Plot, &regs, &regs);

  regs.r[0] = 5+96;
  regs.r[1] = redraw[1]+maxXSlider;
  regs.r[2] = redraw[4]-124;
  _kernel_swi(XOS_Plot, &regs, &regs);

  /** Sets New Rectangle **/
  regs.r[0] = 11;
  _kernel_swi(SYS_XWimp_SetColour, &regs, &regs);

  regs.r[0] = 4;
  regs.r[1] = redraw[1]+minXSlider;
  regs.r[2] = redraw[4]-140;
  _kernel_swi(XOS_Plot, &regs, &regs);

  regs.r[0] = 5+96;
  regs.r[1] = redraw[1] + newVolume+minXSlider;
  regs.r[2] = redraw[4] -124;
  _kernel_swi(XOS_Plot, &regs, &regs);

  return(TRUE);
}

/*****************************************************************************/
void windowsLoseCaret(int *event)
{
  /** Handles Losing The Caret **/
  
  /** Checks If Main Window, Otherwise Ignore **/
  if(event[0] != mainWindow)
    return;
    
  /** Checks Which Icon Number **/
  if(event[eventCaretIcon] == iconCDTitle)
    /** Changes CD Title Icon **/
    windowIconSetState(mainWindow, iconCDTitle, iconCDTitleValue, -1); 
  else
    /** Must Be A Track Icon **/
    windowIconSetState(mainWindow, event[eventCaretIcon], iconTwoValue, -1); 
    
}

/*****************************************************************************/
void windowsGainCaret(int *event)
{
  /** Handles Gaining The Caret **/

  /** Checks If Main Window, Otherwise Ignore **/
  if(event[0] != mainWindow)
    return;
    
  /** Checks Which Icon Number **/
  if(event[eventCaretIcon] == iconCDTitle)
    /** Changes CD Title Icon **/
    windowIconSetState(mainWindow, iconCDTitle, iconCDTitleSelectValue, -1); 
  else
    /** Must Be A Track Icon **/
    windowIconSetState(mainWindow, event[eventCaretIcon], iconTwoSelectValue, -1); 
}

/*****************************************************************************/
void windowsUnsetIcons(void)
{
  /** Clears The Icons **/
  int counter;
  
  /** Checks CD Title Icon **/
  windowIconGetState(mainWindow, iconCDTitle, tempBlock);
  
  /** Restes Icon **/
  if(tempBlock[6] == iconCDTitleSelectValue)
    windowIconSetState(mainWindow, iconCDTitle, iconCDTitleValue, -1);
    
  /** Scans Through Rest Of Valid Icons **/
  counter = baseIconNumber;
  while(++counter <= ((maxTrack - minTrack + 1) * 3) + baseIconNumber)
  {
    windowIconGetState(mainWindow, counter, tempBlock);

    /** Resets Icon **/
    if(tempBlock[6] == iconTwoSelectValue)
      windowIconSetState(mainWindow, counter, iconTwoValue, -1);
  }
}

/*****************************************************************************/
void windowsKeyPress(int keycode)
{
  /** Checks If Need To Redraw Icons **/
  int caretBlock[6], state[9];
  listelement *el;
    
  switch(keycode)
  {
    case keycode_CURSORDOWN:
    case keycode_CURSORUP:
    case keycode_TAB:
    case keycode_SHIFT_TAB:
    case keycode_RETURN:
      /** Reads In Caret Positon **/
      regs.r[1] = (int)&caretBlock;
      _kernel_swi(XWimp_GetCaretPosition, &regs, &regs);

      if(caretBlock[eventCaretIcon] == (((maxTrack - minTrack + 1 ) * 3) - 2 + baseIconNumber))
      {
        /** Checks If On Last One **/
        windowIconGetState(mainWindow, caretBlock[eventCaretIcon], tempBlock);

        if(tempBlock[6] == iconTwoSelectValue)
        {
          /** Checks If Need To Save Listing **/
          if(CDFlags.data.autoSave && trackListingAltered && keycode == keycode_RETURN)
          {
            listingSaveData();
            trackListingAltered = FALSE;
          }
          return;  
        }
      }

      /** Clears Icons **/
      windowsUnsetIcons();
      
      /** Changes New Icon **/
      if(caretBlock[eventCaretIcon] == iconCDTitle)
        windowIconSetState(mainWindow, caretBlock[eventCaretIcon], iconCDTitleSelectValue, -1);
      else
        windowIconSetState(mainWindow, caretBlock[eventCaretIcon], iconTwoSelectValue, -1);
        
      /** Gets Window State **/
      state[0] = mainWindow;
      regs.r[1] = (int)&state;
      _kernel_swi(XWimp_GetWindowState, &regs, &regs);
      
      /** Check If Need To Scroll Window **/
      if(caretBlock[3] < (state[6] - state[4] + state[2]))
      {
        /** Scrolls Window **/
        state[6] = caretBlock[3] + (state[4] - state[2]) - 5;
        _kernel_swi(XWimp_OpenWindow, &regs, &regs);
      }

      if(caretBlock[3] > (state[6] - 40))
      {
        /** Scrolls Window **/
        state[6] = caretBlock[3] + 45;
        _kernel_swi(XWimp_OpenWindow, &regs, &regs);
      }

    break;
    
    case keycode_ESCAPE:windowSetCaret(-1,-1);
    break;
    
    default:
      windowProcessKey(keycode);
      if(keycode <= 256)
        trackListingAltered = TRUE;
    break;
    
  }
  
  /** Checks if need to update current track name **/
  if(trackListingAltered && keycode == keycode_RETURN)
  {
    el = (listelement *) listingFindLink(currentTrack);
    if(strcmp(el->trackName, "\0\1\2") != 0)
      windowIconSetText(mainWindow, iconTrackName, el->trackName);
  }
    
}    
    
/*****************************************************************************/
void windowDeleteIcon(int handle, int icon)
{
  /** Deletes the specified Icon**/
  regs.r[1] = (int)&tempBlock;
  tempBlock[0] = handle;
  tempBlock[1] = icon;
  _kernel_swi(XWimp_DeleteIcon, &regs, &regs);
}

/*****************************************************************************/
void windowProcessKey(int keyCode)
{
  /** Passes The key onto the WIMP **/
  regs.r[0] = keyCode;
  _kernel_swi(XWimp_ProcessKey, &regs, &regs);
}

/*****************************************************************************/
void windowIconGetState(int handle, int icon, int *block)
{
  /** Sets the icons state **/
  regs.r[1] = (int)block;
  block[0] = handle;
  block[1] = icon;
  
  _kernel_swi(XWimp_GetIconState, &regs, &regs);
}
/*****************************************************************************/
void windowIconSetState(int handle, int icon, int EORWord, int clearWord)
{
  /** Sets the icons state **/
  regs.r[1] = (int)&tempBlock;
  tempBlock[0] = handle;
  tempBlock[1] = icon;
  tempBlock[2] = EORWord;
  tempBlock[3] = clearWord;
  
  _kernel_swi(XWimp_SetIconState, &regs, &regs);
}

/*****************************************************************************/
void windowSetCaret(int handle, int icon)
{
  /** Sets the caret position **/
  regs.r[0] = handle;
  regs.r[1] = icon;
  regs.r[4] = -1;
  regs.r[5] = 0;
  
  _kernel_swi(XWimp_SetCaretPosition, &regs, &regs);
}

/*****************************************************************************/
void windowIconGetText(int handle, int icon, char *text)
{
  /** Returns the contents of an icon **/
  
  /** Sets text to NULL **/
  text[0] = NULL;
  
  /** Gets Icon info **/
  windowIconGetState(handle, icon, tempBlock);
  
  /** Checks if Icon is indirected **/
  if(tempBlock[6] & 1 << 8)
    /** Copies the text **/
    strncpy(text, (char *)tempBlock[7], tempBlock[9]);
}
   
/*****************************************************************************/
void windowIconSetText(int handle, int icon, char *text)
{
  /** Returns the contents of an icon **/
  
  /** Checks if Text exists **/
  if(text == NULL)
    return;
  
  /** Gets Icon info **/
  windowIconGetState(handle, icon, tempBlock);
  
  if(handle == mainWindow && icon == iconPlay)
  {
    /** Copies the text **/
    strncpy((char *)tempBlock[8], text, strlen(text) - 1);
  }
  else
  {
    /** Checks if Icon is indirected **/
    if(tempBlock[6] & 1 << 8)
      /** Copies the text **/
      strncpy((char *)tempBlock[7], text, tempBlock[9] - 1);
  }
  
  /** Redraws the icon **/
  windowIconSetState(handle, icon, 0, 0);
  
}

/*****************************************************************************/
int windowIconGetInteger(int handle, int icon)
{
  /** Returns the contents of an icon **/
  windowIconGetText(handle, icon, tempData);
  
  if(tempData != NULL)
    return(atoi(tempData));
    
  /** Default returns **/
  return 0;
}
   
/*****************************************************************************/
void windowIconSetInteger(int handle, int icon, int newContent)
{
  /** Returns the contents of an icon **/
  sprintf(tempData, "%i", newContent);
  windowIconSetText(handle, icon, tempData);
}

/*****************************************************************************/
int windowStartTask(char *commandLine)
{
  /** Starts up a task **/
  int taskHandle;
  
  /** Reads in current size of Wimp Slot **/
  regs.r[0] = regs.r[1] = -1;
  _kernel_swi(SYS_XWimp_SlotSize, &regs, &regs);
  hotkeysWindow = regs.r[1];

  /** Sets 'Next' to 36K **/
  regs.r[1] = 0x9000;
  _kernel_swi(SYS_XWimp_SlotSize, &regs, &regs);

  /** Starts the task **/
  regs.r[0] = (int)commandLine;
  _kernel_swi(SYS_XWimp_StartTask, &regs, &regs);
  taskHandle = regs.r[0];
      
  /** Resets the 'Next' slot size **/
  regs.r[0] = -1;
  regs.r[1] = hotkeysWindow;
  _kernel_swi(SYS_XWimp_SlotSize, &regs, &regs);
  
  /** Returns the task handle **/
  return taskHandle;
}

/*****************************************************************************/
void windowCheckPlayIcon(void)
{
  /** Returns the contents of an icon **/
  if(!CDFlags.data.pause && !stopped)
    windowIconSetText(mainWindow, iconPlay, "R5;Scdp_pause;\n");
  else
    windowIconSetText(mainWindow, iconPlay, "R5;Scdp_play;\n");
      
}

