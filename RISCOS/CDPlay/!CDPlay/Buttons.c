/******************************************************************************

Program:        CDPlay

File:           Buttons.c

Function:       CD Player Wimp C Program

Description:    Button Click Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 11th September 1996

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/*****************************************************************************/
void buttonsLoadConfig(BOOL update)
{
  /** Loads In The Configuration File **/
  FILE *configData;
  char *error;
  int read, oldDriveNumber;
  BOOL oldShowLengths;
  
  /** Opens Config File **/
  if(update)
  {
    oldDriveNumber = driveNumber;
    oldShowLengths = CDFlags.data.showLengths;
    sprintf(tempData, "%sTempConfig", CDPlayResources);
    configData = fopen(tempData, "r");
  }
  else
    configData = fopen("<CDPlay$Dir>.Choices", "r");
  
  /** Checks If Open **/
  if(configData == NULL)
  {
    /** Checks If Update **/
    if(update)
      return;
      
    /** Sets Default options **/
    CDFlags.value = 0x1FEE0;
    driveNumber = 0;
    CDOffset = 0;
    hotkeyCode = 442;
    timeFormat = 0;
    minX = 132;
    minY = 550;
    maxX = 1048;
    maxY = 714;
    scrollX = 0;
    scrollY = 0;
  }    
  else
  {
    /** Checks If Valid Config File **/
    error = fgets(tempData, 13, configData);
    if(strcmp(tempData, "CDCONFIG V2\n") == 0 && error != NULL)
    {
      /** Reads In Data **/
      read = fscanf(configData, "%i %i %i %i %i\n%i %i %i %i %i %i\n",&CDFlags.value, &driveNumber, 
      &CDOffset, &hotkeyCode, &timeFormat, &minX, &minY, &maxX, &maxY, &scrollX, &scrollY);
 
      /** Checks If Worked **/
      if(read != 11)
      {
        fclose(configData);
        _kernel_oscli("Wipe <CDPlay$Dir>.Choices ~CF { > null: }");
        buttonsLoadConfig(FALSE);
      }
    }
    else
    {
      /** Closes The File And Creates new **/
      fclose(configData);
      _kernel_oscli("Wipe <CDPlay$Dir>.Choices ~CF { > null: }");
      buttonsLoadConfig(FALSE);
    }          
  }
  
  if(update)
  {
    if(oldDriveNumber != driveNumber)
    {
      read = 1;
      if(trackListingAltered && CDFlags.data.autoSave)
        read = reportError();
        
      if(read != 1)
          driveNumber = oldDriveNumber;
      else
      {
        if(CDFlags.data.stopCDOnChange)
        {
          _kernel_swi(XCD_StopDisc, &regs, &regs);
        }
        CDInitialise();
      }
    }
      
    /** Moves the length fields **/
    if(oldShowLengths != CDFlags.data.showLengths)
    {
      read = ((maxTrack - minTrack + 1) * 3 + baseIconNumber);
      while(--read >= baseIconNumber)
        windowDeleteIcon(mainWindow, read);
      
      windowChangeExtent(maxTrack - minTrack + 1, TRUE);

      read = baseIconNumber - 1;
      while(++read <= (maxTrack * 3) + baseIconNumber)
        windowIconSetState(mainWindow, read, 0, 0);
    }
  }
  
  /** Sets Up Hotkeys Variable **/
  sprintf(tempData, "Seteval hotkeys$cddrive %i { > null: }", driveNumber);
  _kernel_oscli(tempData);
  
  /** Closes The File **/
  fclose(configData);
}

/**************************************************************************/
void buttonClick(int *event)
{
  /** Checks Which Button Mouse Click On **/
  if(event[eventMouseButton] != mouseMenu)
  {
    if(event[eventMouseWindow] == mainWindow)
      /** Main Window Click **/
      buttonsWindowClick(event);
    else
      if(event[eventMouseWindow] == programWindow)
        /** Program Window Click **/
        buttonsProgramClick(event);
  }
  else
    if(event[eventMouseWindow] != infoWindow)
    {
      regs.r[0] = 0;
      regs.r[1] = (int)&mainMenuArray[0];
      regs.r[2] = event[eventMousePosX]-64;
      regs.r[3] = event[eventMousePosY];
      _kernel_swi(SYS_XWimp_CreateMenu, &regs, &regs);
      menuOpen = TRUE;
    }
}

/**************************************************************************/
void buttonsWindowClick(int *event)
{
  /** Handles Mouse Clicks On The Main Window **/
  switch(event[eventMouseIcon])
  {
    /** Changes Volume **/
    case iconVolumeArea:buttonsSlider(event);
    break;
    
    /** Swaps Track Time Display **/
    case iconCurrentTime:
      if((++timeFormat) > 3)
        timeFormat = 0;
      CDUpdateDiscInfo();
    break;
    
    /** Go To Start Of Track/Previous Track **/
    case iconLast:
      /** Checks if Wanna Rewind CD **/
      if(event[eventMouseButton] == mouseAdjust)
        CDMoveHead(-256);
      else
        buttonsLast();
    break;
    
    /** Play Next Track **/
    case iconNext:
      /** Checks if Wanna Cue CD **/
      if(event[eventMouseButton] == mouseAdjust)
        CDMoveHead(256);
      else
        buttonsNext();
    break;
    
    /** Plays/pauses CD **/
    case iconPlay:
      if(stopped)
      {
        if(memory)
          buttonsNext();
        else
        {
          stopped = FALSE;
          CDFlags.data.playOverride = TRUE;
          if(currentTrack < minTrack)
            currentTrack = minTrack;
          CDPlayTrack(currentTrack);
        }
      }
      else
        buttonsPause();
    break;
    
    /** Stops CD **/
    case iconStop:
      _kernel_swi(XCD_StopDisc, &regs, &regs);
      CDFlags.data.pause = FALSE;
      stopped = TRUE;
      windowCheckPlayIcon();
    break;
    
    /** Ejects CD **/
    case iconEject:
      if(event[eventMouseButton] == mouseAdjust)
        /** Opens Drawer, Adjust Click Override **/
        CDFlags.data.driveEmpty = FALSE;
      else
        CDCheckIfEmpty();
       
      if(!CDFlags.data.driveEmpty)
        /** Opens Drawer **/
        _kernel_swi(XCD_OpenDrawer, &regs, &regs);
      else
        /** Closes Drawer **/
        _kernel_swi(XCD_CloseDrawer, &regs, &regs);
    break;
    
    /** Sets Random Play **/
    case iconRandom:
      random = !random;
      if(random)
      {
        if(currentTrack < maxTrack)
          startOfNextTrack = CDStartOfTrack(currentTrack + 1);
          
        memory = FALSE;
      }
      else
        windowIconSetState(mainWindow, iconRandom, 0, 1 << 21);
    break;

    /** Sets Memory Play **/
    case iconMemory:
      memory = !memory;
      if(memory)
      {
        if(currentTrack < maxTrack)
          startOfNextTrack = CDStartOfTrack(currentTrack + 1);
          
        random = FALSE;
      }
      else
        windowIconSetState(mainWindow, iconMemory, 0, 1 << 21);
    break;

    /** Checks If Change track Button Pressed **/
    default:buttonsChangeTrack(event);
    break;
  }
}
    
/**************************************************************************/
void buttonsChangeTrack(int *event)
{
  /** Gets Icon Info **/
  int counter, iconBlock[10];
  memoryElement *program;
  
  /** Gets the icon state **/
  windowIconGetState(mainWindow, event[eventMouseIcon], iconBlock);
  
  /** Checks If Right Type Of Button **/
  if(iconBlock[6] == iconOneValue || iconBlock[6] == iconOneSelectValue)
  {
    counter = ((event[eventMouseIcon]) - baseIconNumber) / 3 + 1;
    if(programMode)
    {
      windowIconGetText(programWindow, programDataIcon, tempData);
      if(strlen(tempData) < 156)
      {
        if(memoryItems == 0)
          sprintf(tempData, "");
          
        /** Allocates Memory **/
        program = (memoryElement *) malloc(sizeof(memoryElement));
          
        /** Adds Track Number **/
        program->trackNumber = counter;
        
	/** Adds To Link List **/
	linkListAddToTail(&memoryAnchor, &(program->header));
	
	/** Increases Number Of Memory Items **/
	memoryItems++;
	sprintf(tempData, "%s %i", tempData, counter);
	windowIconSetText(programWindow, programDataIcon, tempData);
      }
    }
    else
    {
      if(CDIsTrackAudio(counter))
      {
        CDFlags.data.playOverride = TRUE;
        CDPlayTrack(counter);
      }
      else
      {
        sprintf(tempData, "Filer_OpenDir CDFS::%i",driveNumber);
        _kernel_oscli(tempData);
  	currentTrack = minTrack;
  	CDFlags.data.pause = FALSE;
  	stopped = TRUE;
  	CDUpdateDiscInfo();
      }
    }
  }
  else
  {
    if(iconBlock[6] == iconTwoValue || iconBlock[6] == iconCDTitleValue)
    {
      /** Change Background Type **/
      windowsUnsetIcons();
      if(iconBlock[6] == iconCDTitleValue)
        windowIconSetState(mainWindow, event[eventMouseIcon], iconCDTitleSelectValue, -1);
      else
        windowIconSetState(mainWindow, event[eventMouseIcon], iconTwoSelectValue, -1);
    }
  }
}
  
/**************************************************************************/
void buttonsLast(void)
{
  /** Moves CD Head To Begining Of Track Or Last Track **/
  int currentPosition = 0, trackStart;
  BOOL skip = FALSE;
  
  /** Reads In CurrentPostion **/
  regs.r[0] = 0;
  _kernel_swi(XCD_EnquireAddress, &regs, &regs);
  currentPosition = regs.r[0];
  
  /** Reads In Start Of Current Track **/
  trackStart = CDStartOfTrack(currentTrack);;
  
  /** Checks If Has To Change Track **/
  if(trackStart > (currentPosition - 100))
  {
    if(random)
    {
      /** Selects Random Track **/
      buttonsNext();
      skip = TRUE;
    }
    else
    {
      if(memory)
      {
        /** Sets Track Number **/
        memoryPosition--;
        currentTrack = listingGetProgramTrack();
        startOfNextTrack = -1;
      }
      else
      {
        /** Sets Previous Track Number **/
        currentTrack--;
        
        /** Checks if gone off 'bottom' of CD **/
        if(currentTrack < minTrack)
          currentTrack = maxTrack;
      }
    }
  }

  if(!skip)
  {
    /** Plays The Track **/
    CDFlags.data.playOverride = TRUE;
    CDPlayTrack(currentTrack);
  }
}
  
/**************************************************************************/
void buttonsNext(void)
{
  /** Plays The Next Track Number In The Play List **/
  int randomTrack;
  
  if(memory)
  {
    /** Sets Track Number **/
    memoryPosition++;
    currentTrack = listingGetProgramTrack();
    startOfNextTrack = -1;
  }
  else
  {
    if(random)
    {
      do
        /** Generates Random Number **/
        randomTrack = rand()%(maxTrack-minTrack+1) + minTrack;
      while(randomTrack == currentTrack);
      currentTrack = randomTrack;
      startOfNextTrack = -1;
    }
    else
    {
      /** Sets Previous Track Number **/
      currentTrack++;
       
      /** Checks if gone off 'top' of CD **/
      if(currentTrack > maxTrack)
        currentTrack = minTrack;
    }
  }
  
  /** Plays Track **/
  CDFlags.data.playOverride = TRUE;
  CDPlayTrack(currentTrack);
}

/**************************************************************************/
void buttonsPause(void)
{
  /** Pauses/Unpauses The CD **/
  listelement *el;
  
  /** Reads In Current Pause Status **/
  _kernel_swi(XCD_AudioStatus, &regs, &regs);
  
  if(regs.r[0] == 0)
  {
    CDFlags.data.pause = TRUE;
    windowIconSetText(mainWindow, iconTrackName, "(Paused)");
  }
  else
  {
    CDFlags.data.pause = FALSE;

    /** Set Track Name **/
    el = (listelement *) listingFindLink(currentTrack);
    windowIconSetText(mainWindow, iconTrackName, el->trackName);
  }
  
  /** Actually Sets Status **/
  windowCheckPlayIcon();
  regs.r[0] = CDFlags.data.pause;
  _kernel_swi(XCD_AudioPause, &regs, &regs);
}
    
/**************************************************************************/
void buttonsProgramClick(int *event)
{
  /** Handles Mouse Clicks In Program Window **/
  switch(event[eventMouseIcon])
  {
    /** Activates Program Mode **/
    case programProgramIcon:
      programMode = !programMode;
    break;

    /** Clears The List **/
    case programClearIcon:
      listingClearList(&memoryAnchor);
      memoryItems = memoryPosition = 0;
      windowIconSetText(programWindow, programDataIcon, "Empty");
    break;

    default:
    break;
  }
}

/**************************************************************************/
void buttonsMenuClick(int *event)
{
  /** Checks Which Button Mouse Click On **/
  switch(event[eventMenuSelection])
  {
    /** Shows Info Window **/
    case menuInfo:windowOpen(infoWindow, TRUE);
    break;
    
    /** Shows Program Window **/
    case menuProgram:
      windowOpen(programWindow, TRUE);
      windowSetCaret(programWindow, programDataIcon);
    break;

    /** Runs Configure Program **/
    case menuChoices:
      if(configtask__handle != NULL)
        break;

      /** Sets up the command line **/
      sprintf(tempData, "Run %sConfig %i %i %i %i %i %i %i %i %i %i %i %i %i\0", CDPlayResources,
              (int)mainWindow, task__handle, CDFlags.value, driveNumber, CDOffset, hotkeyCode, timeFormat,
              minX, minY, maxX, maxY, scrollX, scrollY);

      /** Starts the task **/
      configtask__handle = windowStartTask(tempData);
    break;

    /** Saves CD Details **/
    case menuSave:
        listingSaveData();
        trackListingAltered = FALSE;
    break;

    /** Quits Program **/
    case menuQuit:
      if(shutdown())
        CDFlags.data.quit = TRUE;
    break;

    default:
    break;
  }
  
  /** Checks If Need To Reopen Menu **/
  regs.r[1] = (int)&tempBlock;
  _kernel_swi(XWimp_GetPointerInfo, &regs, &regs);
  if(tempBlock[2] == mouseAdjust)
  {
    regs.r[1] = (int)&mainMenuArray[0];
    _kernel_swi(SYS_XWimp_CreateMenu, &regs, &regs);
  }
  else
    menuOpen = FALSE;
}

/**************************************************************************/
void buttonsSlider(int *event)
{
  /** Adjust The Volume **/
  int temp;
  
  /** Gets Window State **/
  tempBlock[0] = mainWindow;
  regs.r[1] = (int)&tempBlock;
  _kernel_swi(XWimp_GetWindowState, &regs, &regs);
  
  temp = event[eventMousePosX] - tempBlock[1] - minXSlider;
  

  /** Changes Size Of Icon **/
  if(temp > maxXSlider-minXSlider)
    temp = maxXSlider-minXSlider;

  if(temp < minVolume)
    temp = minVolume;
    
  /** Redraws Window **/
  if(currentVolume != temp)
  {
    currentVolume = temp;
    windowIconSetState(mainWindow, iconVolumeArea, 0, 0);
  
    /** Converts To Volume Values **/
    temp *= 207;

    /** Sets Volume Values **/
    tempBlock[0] = tempBlock[1] = temp;
    regs.r[0] = 0;
    regs.r[1] = (int)&tempBlock;
    _kernel_swi(XCD_SetAudioParameters, &regs, &regs);
  }  
}

  
  
  
  
  
