/******************************************************************************

Program:        CDPlay

File:           CDBitz.c

Function:       CD Player Wimp C Program

Description:    CD Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 11th September 1996

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/*****************************************************************************/
void CDInitialise(void)
{
  /** Sets Up CD Block **/
  regs.r[0] = driveNumber;
  _kernel_swi(XCDFS_ConvertDriveToDevice, &regs, &regs);
  CDBlock[0] = (regs.r[1] & 7);
  CDBlock[1] = (regs.r[1] & 0x18) >> 3;
  CDBlock[2] = (regs.r[1] & 0xE0) >> 5;
  CDBlock[3] = (regs.r[1] & 0xFF00) >> 8;
  CDBlock[4] = (regs.r[1] & 0xFFFF0000) >> 16;
  
  /** Sets up registers **/
  regs.r[7] = (int)&CDBlock;
}

/*****************************************************************************/
void CDCheckIfEmpty(void)
{
  /** Checks If The CD Drive Is Empty **/
  BOOL oldState = CDFlags.data.driveEmpty;
  int *save = &mainMenuArray[25];
  
  /** If empty, then grey out the 'Save' option **/
  regs.r[0] = 0;
  regs.r[1] = (int)&tempData;
  
  if(_kernel_swi(XCD_DiscUsed, &regs, &regs))
  {
    save[2] = 0x7400021;    
    CDFlags.data.driveEmpty = TRUE;
  }
  else
  {
    save[2] = 0x7000021;    
    CDFlags.data.driveEmpty = FALSE;
  }
  
  /** Redisplayes Menu **/
  if(menuOpen && oldState != CDFlags.data.driveEmpty)
  {
    regs.r[1] = (int)&mainMenuArray[0];
    _kernel_swi(SYS_XWimp_CreateMenu, &regs, &regs);
  }
  
  /** Updates Icons **/
  if(CDFlags.data.driveEmpty && mainOpen)
  {
    windowIconGetText(mainWindow, iconTrackName, tempData);
    if(strcmp(tempData,"Drive Empty") != 0)
    {
      stopped = TRUE;
      minTrack = 1;
      maxTrack = -1;
      CDDiscID = 0;
      windowChangeExtent(0,FALSE);
      windowIconSetText(mainWindow, iconCDTitle, "Drive Empty");
      windowIconSetText(mainWindow, iconTrackName, "Drive Empty");
      windowIconSetText(mainWindow, iconTrackNumber, "---");
      windowIconSetText(mainWindow, iconCurrentTime, "--:--");
      
      /** Clears memory **/
      if(memoryItems != 0)
      {
        listingClearList(&memoryAnchor);
        memoryPosition = 0;
        windowIconSetText(programWindow, programDataIcon, "Empty");
      }

      /** Sets the icon state **/
      windowCheckPlayIcon();
      windowOpen(mainWindow, FALSE);
    }
  }
}

/*****************************************************************************/
void CDGenerateTrackList(void)
{
  /** Gets Track Informaton For The Current CD **/
  int counter = 0;
  listelement *el;
  
  /** Checks If Drive is Empty **/
  CDCheckIfEmpty();
  
  if(!CDFlags.data.driveEmpty)
  {
    /** Works Out Total Number Of Tracks **/
    regs.r[0] = 0;
    regs.r[1] = (int)&tempBlock;
    _kernel_swi(XCD_EnquireTrack, &regs, &regs);
    
    minTrack = (tempBlock[0] & 0xFF);
    maxTrack = ((tempBlock[0] & 0xFFFF) >> 8);

    /** Resets Values **/
    currentTrack = minTrack - 1;
    dataCD = FALSE;
    stopped = !CDFlags.data.autoPlayCD;
    CDFlags.data.CDUnknown = FALSE;
    CDFlags.data.newCD = TRUE;
    trackListingAltered = FALSE;
    
    /** Erases The Track Names Linked List **/
    listingEraseList();
    
    /** Adjusts Size Of Main Window **/
    windowChangeExtent(maxTrack - minTrack + 1, FALSE);
    
    /** Searches List For A Match **/
    if(!CDFlags.data.disableCDScanning)
      listingSearchList();
    else
    {
      /** Works Out CD Total Playtime **/
      regs.r[0] = 0;
      _kernel_swi(XCD_DiscUsed, &regs, &regs);
      CDDiscID = tempBlock[0];
      strcpy(dataCDTitle, "CD Scanning Disabled");
      listingNotFound();

      /** Sets CD Title **/
      windowIconSetText(mainWindow, iconCDTitle, dataCDTitle);
    }
      
    /** Updates Window **/
    if(mainOpen)  
    {
      /** Forces Redraw Of Icons **/
      windowIconSetState(mainWindow, iconCDTitle, 0, 0);
      counter = baseIconNumber - 1;
      while(++counter <= (maxTrack * 3) + baseIconNumber)
        windowIconSetState(mainWindow, counter, 0, 0);
    }


    /** Forces Redraw Of Track Name **/
    el = (listelement *) listingFindLink(minTrack);
    windowIconSetText(mainWindow, iconTrackName, el->trackName);
    
    /** Erases Program Listing **/
    listingClearList(&memoryAnchor);
    memoryItems = 0;
    memoryPosition = 0;
    windowIconSetText(programWindow, programDataIcon, "Empty");

    /** Checks If CD Is Playing **/
    CDCheckCD();
    
    /** Checks If Data CD **/
    if(maxTrack == minTrack)
    {
      if(!CDIsTrackAudio(maxTrack))
      {
        dataCD = TRUE;
        windowIconSetText(mainWindow, iconTrackName, "Data Track");
        windowIconSetText(mainWindow, iconCurrentTime, "--:--");
        windowIconSetText(mainWindow, iconTrackNumber, "---");
      }
    }
  }

  /** Sets the icon state **/
  windowCheckPlayIcon();
}
 
/*****************************************************************************/
BOOL CDIsTrackAudio(int track)
{
  /** Checks If Specified Track Is Audio **/
  regs.r[0] = track;
  regs.r[1] = (int)&tempData;

  if(CDFlags.data.checkIfAudio && (_kernel_swi(XCD_EnquireTrack, &regs, &regs) == NULL))
    return((int)tempData[4] == 0);
  else
    return TRUE;
}

/*****************************************************************************/
void CDCheckCD(void)
{
  /** Checks If CD Is Playing **/
  int status = 0;
  listelement *el;
  
  /** Checks If CD Has Changed **/
  regs.r[0] = 0;
  regs.r[1] = (int)&tempBlock;
  if(CDFlags.data.autoDetectChange && (_kernel_swi(XCD_DiscUsed, &regs, &regs) == NULL))
    if(CDDiscID != tempBlock[0])
      CDGenerateTrackList();
      
  /** Checks If Not Empty And Not Detected **/
  if(!CDFlags.data.autoDetectChange && !CDFlags.data.driveEmpty && CDDiscID == 0)
    CDGenerateTrackList();

  /** Checks If CD Is Playing **/
  _kernel_swi(XCD_AudioStatus, &regs, &regs);
  status = regs.r[0];
  
  /** Checks if need to switch to random or memory mode **/
  if((random || memory) && (startOfNextTrack > -1))
  { 
    /** Reads In Current Position **/
    regs.r[0] = 2;
    _kernel_swi(XCD_EnquireAddress, &regs, &regs);
    tempBlock[0] = regs.r[0];
    
    /** Checks if current value is larger than stored value **/
    if(startOfNextTrack <= tempBlock[0])
    {
      /** Forces switch to single play mode **/
      status = 3;
      startOfNextTrack = -1;
    }
  }

  /** If CD has Finished Playing, Then Play From Start **/
  switch(status)
  {
    case 0:
      /** CD Is Playing **/
      if(stopped)
      {
        stopped = FALSE;
        /** Sets the icon state **/
        windowCheckPlayIcon();
      }
      
      if(CDFlags.data.pause)
      {
        el = (listelement *) listingFindLink(currentTrack);
        windowIconSetText(mainWindow, iconTrackName, el->trackName);
        CDFlags.data.pause = FALSE;

        /** Sets the icon state **/
        windowCheckPlayIcon();
      }
    break;      
      
    case 1:
      /** Checks if CD Is paused **/
      if(CDFlags.data.pause)
        break;
      
      CDFlags.data.pause = TRUE;
      windowIconSetText(mainWindow, iconTrackName, "(Paused)");

      /** Sets the icon state **/
      windowCheckPlayIcon();
    break;
    
    case 3:
    case 5:
      if(stopped)
        break;
        
      if(random)
        /** Chooses Next Random Track **/
        buttonsNext();
      else
      {
        if(memory)
        {
          /** Sets Next Track Number **/
          memoryPosition++;
          if(memoryItems > 0)
            CDPlayTrack(listingGetProgramTrack());
        }
        else
        {
          /** Reads In Next Track Number To Be Played **/
          if(++currentTrack > maxTrack)
            currentTrack = minTrack;
            
          /** Plays Track **/
          CDPlayTrack(currentTrack);
        }
      }
    break;
            
    default:
    break;
  }
 
  /** Unsets New CD Variable **/
  CDFlags.data.newCD = FALSE;
}
          
/*****************************************************************************/
void CDPlayTrack(int track)
{
  /** Plays The Specified Track **/
  listelement *el;
  
  if(!stopped || CDFlags.data.playOverride)
  {
    if(!CDFlags.data.driveEmpty && CDIsTrackAudio(track))
    {
      if((track > minTrack) || CDFlags.data.autoRepeatCD || CDFlags.data.newCD || CDFlags.data.playOverride)
      {
        /** Checks if only want to play till end of track **/
        if(random || memory)
          regs.r[1] = 0xFE;
        else
          regs.r[1] = 0xFF;
          
        /** Plays The Track **/
        regs.r[0] = track;
        _kernel_swi(XCD_PlayTrack, &regs, &regs);
        currentTrack = track;
        stopped = FALSE;
        CDFlags.data.pause = FALSE;
        
        /** Updates the play icon **/
        windowCheckPlayIcon();
      }
      else
      {
        /** Do Not Restart CD **/
        currentTrack = minTrack;
        stopped = TRUE;
      }    

      /** Updates Window Display **/
      CDUpdateDiscInfo();
      el = (listelement *) listingFindLink(currentTrack);
      if(strcmp(el->trackName, "\0\1\2") == 0)
        windowIconSetText(mainWindow, iconTrackName, "Track Name Unknown");
      else
        windowIconSetText(mainWindow, iconTrackName, el->trackName);
    }
  }

  /** Unsets playOverride **/
  CDFlags.data.playOverride = FALSE;
}

/*****************************************************************************/
void CDUpdateDiscInfo(void)
{
  listelement *el;
  
  /** Checks If Drive Is Empty **/
  CDCheckIfEmpty();
  
  /** Checks If CD Is playing **/
  if(!CDFlags.data.driveEmpty && !CDFlags.data.playOverride)
    CDCheckCD();
    
  /** Redraws Window **/
  if(mainOpen && !CDFlags.data.driveEmpty && !stopped && !CDFlags.data.pause)
  {
    if(!dataCD)
    {
      /** Updates Track Number **/
      regs.r[0] = 0x40;
      regs.r[1] = (int)&tempData;
      if(_kernel_swi(XCD_ReadSubChannel, &regs, &regs) != NULL)
        return;
    
      /** Extracts Track Number **/
      currentTrack = (int)tempData[9];
    
      if(currentTrack > maxTrack || currentTrack < minTrack)
        currentTrack = minTrack;
        
      /** Works Out Track Start Position **/
      tempBlock[1]  = CDStartOfTrack(currentTrack);

      /** Reads In Current Position **/
      regs.r[0] = 2;
      _kernel_swi(XCD_EnquireAddress, &regs, &regs);
      tempBlock[0] = regs.r[0];
    
      switch(timeFormat)
      {
        /** Updates Time From Start Of Track **/
        case 1:strcpy(tempData, CDGenerateTime(tempBlock[0] - tempBlock[1]));
        break;
        
        /** Updates Time From End Of CD **/
        case 2:strcpy(tempData, CDGenerateTime(CDDiscID - tempBlock[0] + 300));
        break;
        
        /** Updates Time From End Of Track **/
        case 3:
          if(currentTrack == maxTrack)
            strcpy(tempData, CDGenerateTime(CDDiscID - tempBlock[0] + 300));
          else
          {
            tempBlock[1]  = CDStartOfTrack(currentTrack+1);
            strcpy(tempData, CDGenerateTime(tempBlock[1] - tempBlock[0] + 300));
          }
        break;
        
        /** Updates Current Time **/
        default:strcpy(tempData, CDGenerateTime(tempBlock[0]));
        break;
      }
      
      /** Updates Time **/
      windowIconSetText(mainWindow, iconCurrentTime, tempData);
    
      /** Gets Displayed Track number **/
      tempBlock[0] = windowIconGetInteger(mainWindow, iconTrackNumber);
      
      if(tempBlock[0] != currentTrack && tempBlock[0] <= maxTrack)
      {
        /** Updates Track Number And Name **/
        windowIconSetInteger(mainWindow, iconTrackNumber, currentTrack);
        el = (listelement *) listingFindLink(currentTrack);
        if(strcmp(el->trackName, "\0\1\2") == 0)
          windowIconSetText(mainWindow, iconTrackName, "Track Name Unknown");
        else
          windowIconSetText(mainWindow, iconTrackName, el->trackName);
      }
      
      /** Updates Volume **/
      regs.r[0] = 0;
      regs.r[1] = (int)&tempBlock;
      
      if((_kernel_swi(XCD_GetAudioParameters, &regs, &regs) == NULL)
         && currentVolume != (tempBlock[0]/207))
      { 
        currentVolume = tempBlock[0]/207;
        windowIconSetState(mainWindow, iconVolumeArea, 0, 0);
      }
    }      
    else
    {
      /** Sets Data CD Icon text **/
      windowIconSetText(mainWindow, iconCDTitle, "Data CD");
      windowIconSetText(mainWindow, iconTrackName, "Data Track");
      windowIconSetText(mainWindow, iconCurrentTime, "--:--");
      windowIconSetText(mainWindow, iconTrackNumber, "---");
      stopped = TRUE;
    }
  }
}
      
/*****************************************************************************/
int CDStartOfTrack(int trackNumber)
{
  /** Calculates The Start Position Of The Track **/
  int trackStart[2];
  
  regs.r[0] = trackNumber;
  regs.r[1] = (int)&trackStart;
  _kernel_swi(XCD_EnquireTrack, &regs, &regs);
  
  /** Returns Track Start Positon **/
  return trackStart[0];
}
  
/*****************************************************************************/
char *CDGenerateTrackLength(int trackNumber)
{
  /** Works Out Length Of Track **/
  int startOfTrack = 0, endOfTrack = 0;

  /** Works Out Track Start And End Position **/
  startOfTrack = CDStartOfTrack(trackNumber);

  if(trackNumber != maxTrack)
    /** Calculates End of Track **/
    endOfTrack = CDStartOfTrack(trackNumber+1);
  else
  {
    /** Works Out End Of Track **/
    regs.r[0] = 0;
    regs.r[1] = (int)&tempBlock;
    _kernel_swi(XCD_DiscUsed, &regs, &regs);
    endOfTrack = tempBlock[0];
  }
    
  /** Creates Text String **/
  strcpy(tempData, CDGenerateTime(endOfTrack - startOfTrack + 180));
  
  /** Returns String **/
  return(tempData);
}
/*****************************************************************************/
char *CDGenerateTime(int currentPosition)
{
  /** Returns The Current Time On The Track **/
  int seconds, minutes, hours;

  regs.r[0] = 2;
  regs.r[1] = currentPosition;
  _kernel_swi(XCD_ConvertToMSF, &regs, &regs);
  currentPosition = regs.r[1];
  
  seconds = (currentPosition >> 8) & 0x000000ff;
  minutes = (currentPosition >> 16) & 0x000000ff;
  hours   = (currentPosition >> 24) & 0x000000ff;
  
  /** Check If Valid Number Of Hours **/
  if (hours>23)
  {
    seconds = 2;
    minutes = 0;
    hours = 0;
  }

  /** Checks If Valid Seconds Parameter **/
  if (seconds > 59)
  {
    seconds = seconds - 60;
    minutes++;
  }

  /** Generates Minutes **/
  minutes = minutes + (hours * 60);

  sprintf(tempData, "%2i:%2i", minutes, seconds);

  /** Adds Extra Zero If Needed **/
  if((minutes < 10) && (seconds < 10))
  {
    tempData[0] = tempData[3] = '0';
  }
  else
    if(seconds < 10)
      tempData[3] ='0';
    else
      if(minutes < 10)
        tempData[0] = '0';

  /** Returns String **/
  return(tempData);
}

/*****************************************************************************/
void CDMoveHead(int amount)
{
  /** Moves The Head To A New Position On The CD And Plays Till End Of Track **/

  /** Gets Keyboard Information **/
  regs.r[0] = 202;
  regs.r[1] = 0;
  regs.r[2] = 255;
  _kernel_swi(XOS_Byte, &regs, &regs);
  
  if(regs.r[1] & 0x8)
    amount += amount;
        
  /** Reads in Current Position **/
  regs.r[0] = 0;
  _kernel_swi(XCD_EnquireAddress, &regs, &regs);
  tempBlock[8] = regs.r[0];
    
  if(tempBlock[8] + amount < CDStartOfTrack(currentTrack) || ((tempBlock[8] + amount) > CDDiscID))
    return;
    
  /** Sets New Position **/
  if((random || memory) && (currentTrack < maxTrack))
    regs.r[2] = CDStartOfTrack(currentTrack+1);
  else
    regs.r[2] = CDDiscID;
    
  regs.r[0] = 0;
  regs.r[1] = tempBlock[8] + amount;
  _kernel_swi(XCD_PlayAudio, &regs, &regs);
      
  /** Updates Window **/
  CDUpdateDiscInfo();  
}  
