/******************************************************************************

Program:        CDPlay

File:           Main.c

Function:       CD Player Wimp C Program

Description:    Main Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 11th September 1996
		1.02 12th December 1996

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/** Initialisation Variables **/
int task__handle = NULL, configtask__handle = NULL;
char task__name[7] = "CDPlay";
char task__version[16] = "1.17 (08-12-97)";
char tempData[156], windowData[556], spriteArea[1268];
int mainMenuArray[38], tempBlock[10];
char *ws = &windowData[0], *wsend = &windowData[556], *area = &spriteArea[0];
char CDPlayResources[24] = "<CDPlay$Dir>.Resources.";

/** Global Variables **/
configData CDFlags;
BOOL mainOpen = FALSE, programOpen = FALSE, programMode = FALSE, menuOpen = FALSE, mainOpened = FALSE,
     trackListingAltered = FALSE, stopped, memory, random, dataCD;
linkListHeader listAnchor = {NULL}, memoryAnchor = {NULL};
int mainWindow = NULL, programWindow = NULL, infoWindow = NULL, hotkeysWindow = NULL,
    driveNumber, CDBlock[5], minTrack, maxTrack, currentTrack, CDDiscID,
    memoryItems, memoryPosition, currentVolume, CDOffset, hotkeyCode,
    timeFormat, minX, minY, maxX, maxY, scrollX, scrollY, startOfNextTrack = -1;
char dataCDTitle[maxTrackLength];
_kernel_swi_regs regs;

/***************************** Task Event Tree *****************************/

void taskDispatch(int eventType, int *event)
{
  /** Sets up pointer to event block **/
  regs.r[1] = (int)&event[0];

  /** Works out what wimp event has happened **/
  switch(eventType)
  {
    /** Handles Null Events **/
    case eventNULL:
      /** Poll Loop **/
      if(!CDFlags.data.driveEmpty)
        CDUpdateDiscInfo();
      else
        CDCheckIfEmpty();
    break;

    /** Handles Open Window Event **/
    case eventOPEN:
      _kernel_swi(XWimp_OpenWindow, &regs, &regs);
    break;

    /** Handles Mouse Button Clicks **/
    case eventBUTTON:buttonClick(event);
    break;

    /** Handles Key Presses **/
    case eventKEY:
      if(event[eventKeyCode] == hotkeyCode)
        *wsend = 1;
      else
      {
        if(event[eventWindowHandle] == mainWindow)
          /** Checks If Special Key **/
          windowsKeyPress(event[eventKeyCode]);
        else
          windowProcessKey(event[eventKeyCode]);
      }
    break;

     /** Handles Menu Clicks **/
    case eventMENU:buttonsMenuClick(event);
    break;

    /** Handles Losing The Caret **/
    case eventLOSECARET:windowsLoseCaret(event);
    break;

    /** Handles Gaining The Caret **/
    case eventGAINCARET:windowsGainCaret(event);
    break;

    /** Handles Close Window Request **/
    case eventCLOSE:
      if(event[0] == mainWindow)
      {
        mainOpen = FALSE;
        windowsUnsetIcons();
      }
      else
        if(event[0] == programWindow)
        {
          /** Sets programOpen to FALSE **/
          programMode = programOpen = FALSE;
          windowIconSetState(programWindow, programProgramIcon, (programMode << 21), (1 << 21));
        }

      _kernel_swi(XWimp_CloseWindow, &regs, &regs);
    break;

    /** Handles Redraw Requests **/
    case eventREDRAW:
      windowsRedrawSlider(event[0]);
    break;

    case eventNONZEROPOLLWORD:
      /** Resets Poll Word **/
      *wsend = 0;

      /** Opens/Closes Main Window **/
      mainOpen = !mainOpen;
      if(mainOpen)
      {
        CDUpdateDiscInfo();
        windowOpen(mainWindow, TRUE);
      }
      else
      {
        regs.r[1] = (int)&mainWindow;
        _kernel_swi(XWimp_CloseWindow, &regs, &regs);
        windowsUnsetIcons();
      }
    break;

    /** User Messages **/
    case eventUSERMESSAGE:
    case eventUSERMESSAGERECORDED:
      switch (event[messageCode])
      {
        /** Quit Request **/
        case messageQUIT:
          /** Sets quit to TRUE **/
          if(shutdown())
            CDFlags.data.quit = TRUE;
        break;

  	case messageDATALOAD:
        switch(event[messageFiletype])
        {
          /** Check If Text File Or Data File **/
          case 0xFFF:
          case 0xFFD:
            /** Replys To Message **/
            event[messageYourRef] = event[messageMyRef];
            event[messageAction] = messageDATALOADACK;
            regs.r[0] = eventUSERMESSAGE;
            regs.r[2] = event[messageSender];
            _kernel_swi(XWimp_SendMessage, &regs, &regs);

	    sprintf(tempData, "%sCDMerge %s\0", CDPlayResources, &event[messageFilename]);
	   windowStartTask(tempData);
          break;

          default:
          break;
        }
        break;

	/** Handles Shutdown Message **/
	case messagePREQUIT:
	  if(trackListingAltered && CDFlags.data.autoSave)
          {
            if(!shutdown())
            {
              /** We Don't Want To Shutdown **/
              event[messageYourRef] = event[messageMyRef];
              regs.r[0] = eventUSERMESSAGEACK;
              regs.r[2] = event[messageSender];
              _kernel_swi(XWimp_SendMessage, &regs, &regs);
            }
          }
	break;

        case messageMENUSDELETED:
          /** Unsets Window Variables **/
          menuOpen = FALSE;
        break;

        /** Checks If Config Program Quitting **/
        case messageTASKCLOSEDOWN:
          if(configtask__handle != NULL && event[messageSender] == configtask__handle)
          {
            /** Loads In New Configuration **/
            configtask__handle = NULL;
            buttonsLoadConfig(TRUE);
            sprintf(tempData, "Wipe %sTempConfig ~CF { > null: }", CDPlayResources);
            _kernel_oscli(tempData);
          }
        break;

        default:
        break;
      }
    break;

    /** Ignores All Other Wimp Events **/
    default:
    break;
  }
}

/************************** Initialisation *********************************/
BOOL mainInitialise(void)
{
  /** Sets Up Resource Area etc. **/
  FILE *newVolume;

  /** Checks if we already exist **/
  if(enumerateTasks())
    return FALSE;

  /** Sets up the messages we want to receive **/
  tempBlock[0] = messageDATALOAD;
  tempBlock[1] = messagePREQUIT;
  tempBlock[2] = messageMENUSDELETED;
  tempBlock[3] = messageTASKCLOSEDOWN;
  tempBlock[4] = NULL;

  /** Sets up Wimp_Initialise block **/
  regs.r[0] = 300;
  regs.r[1] = 0x4B534154;
  regs.r[2] = (int)task__name;
  regs.r[3] = (int)&tempBlock;

  if(!_kernel_swi(XWimp_Initialise, &regs, &regs))
  {
    /** Reads in task__handle etc. **/
    task__handle = regs.r[1];

    /** Initialises Handler And Windows **/
    sprintf(tempData, "%sTemplates", CDPlayResources);
    windowInitialise(tempData);

    /** Loads In Sprites **/
    windowLoadSprites();

    /** Creates Menu **/
    windowMakeMenu();

    /** Loads In Config Data And Updates Window **/
    buttonsLoadConfig(FALSE);

    /** Sets Up Poll Word, And Checks If Already Running **/
    setupPollWord();

    /** Reseeds Random Number Generator (Memory address should be fairly random) **/
    srand(tempBlock[4]);

    /** Sets Up Exit Handler **/
    atexit(taskExit);

    /** Sets Up CD Functions **/
    CDInitialise();
    CDGenerateTrackList();

    /** Sets Up Volume Variable **/
    if(CDFlags.data.saveVolume)
    {
      newVolume = fopen("<CDPlay$Dir>.Volume","r");
      if(newVolume != NULL)
      {
        fscanf(newVolume, "%i", &currentVolume);
        tempBlock[0] = tempBlock[1] = currentVolume*207;
        regs.r[0] = 0;
        regs.r[1] = (int)&tempBlock;
        _kernel_swi(XCD_SetAudioParameters, &regs, &regs);
        fclose(newVolume);
      }
    }
  }
  else
    /** Aborts Program **/
    return(FALSE);


  /** Returns **/
  return(TRUE);
}

/**************************************************************************/
BOOL enumerateTasks(void)
{
  /** Sets Up Poll Word, And Checks If Already Running **/
  regs.r[0] = 0;
  while(regs.r[0] >= 0)
  {
    regs.r[1] = (int)&tempBlock[0];
    regs.r[2] = 20;
    _kernel_swi(XTaskManager_EnumerateTasks, &regs, &regs);
    if(regs.r[0] >= 0 && strcmp((char *)tempBlock[1], task__name) == 0)
      return TRUE;
  }
  /** Returns **/
  return FALSE;
}
/**************************************************************************/
void setupPollWord(void)
{
  /** Sets Up Poll Word, And Checks If Already Running **/
  regs.r[0] = 18;
  regs.r[1] = (int)"CDFilter";
  _kernel_swi(XOS_Module, &regs, &regs);
  tempBlock[4] = regs.r[4];

  if(tempBlock[4] != 0)
    wsend = (void *)tempBlock[4];
}
/**************************************************************************/
void taskExit(void)
{
  /** Kills Off CDFilter **/
  _kernel_oscli("RMKill CDFilter");

  /** Closedown Wimp Task **/
  regs.r[0] = task__handle;
  regs.r[1] = 0x4B534154;
  _kernel_swi(XWimp_CloseDown, &regs, &regs);
}

/**************************************************************************/
int reportError(void)
{
  /** Reports The Error That Listings Not Saved **/
  int errorBlock[15];

  errorBlock[0] = 1<<30;
  sprintf((char *)&errorBlock[1], "The track listing has been modified.  Discard change?\0");

  regs.r[0] = (int)&errorBlock;
  regs.r[1] = 3;
  regs.r[2] = (int)task__name;
  _kernel_swi(SYS_Wimp_ReportError, &regs, &regs);

  /** Returns Result **/
  return (regs.r[1]);
}

/**************************************************************************/
BOOL shutdown(void)
{
  /** Prepares To Shutdown **/
  FILE *volume;

  /** Checks If Need To Save Track Listing **/
  if(trackListingAltered && CDFlags.data.warnIfNotSaved)
  {
    if(reportError() != 1)
      return FALSE;
    else
      trackListingAltered = FALSE;
  }

  if(CDFlags.data.saveVolume)
  {
    /** Saves The Current Volume **/
    volume = fopen("<CDPlay$Dir>.Volume","w");
    if(volume != NULL)
    {
      fprintf(volume, "%i", currentVolume);
      fclose(volume);
    }
  }
  else
  {
    tempBlock[0] = tempBlock[1] = maxVolume;
    regs.r[0] = 0;
    regs.r[1] = (int)&tempBlock;
    _kernel_swi(XCD_SetAudioParameters, &regs, &regs);
  }


  /** Stops CD **/
  if(CDFlags.data.stopCDOnQuit)
    _kernel_swi(XCD_StopDisc, &regs, &regs);

  /** Return **/
  return(TRUE);

}

/**************************************************************************/
int main(void)
{
  /** Main Program Procedure **/
  int event_block[64];

  if(mainInitialise())
  {
    /** Opens Main Window **/
    if(CDFlags.data.autoOpenMain)
    {
      windowOpen(mainWindow, TRUE);
      mainOpen = TRUE;
      CDUpdateDiscInfo();
    }

    /** The Main Event Loop **/
    while(!CDFlags.data.quit)
    {
      _kernel_swi(XOS_ReadMonotonicTime, &regs, &regs);

      regs.r[2] = regs.r[0] + 100;
      regs.r[0] = 0x480030;
      regs.r[1] = (int)&event_block;
      regs.r[3] = (int)wsend;
      _kernel_swi(XWimp_PollIdle, &regs, &regs);
      taskDispatch(regs.r[0], event_block);
    }
  }

  /** Returns **/
  return(0);
}
