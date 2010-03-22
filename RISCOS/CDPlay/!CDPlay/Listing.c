/******************************************************************************

Program:        CDPlay

File:           Listing.c

Function:       CD Player Wimp C Program

Description:    Program Routines For Handling The Listing

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 13th September 1996

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/*****************************************************************************/
void listingEraseList(void)
{
  /** Erases The List **/
  int counter = baseIconNumber;
  listelement *el;

  /** Sets up the icon delete block**/
  do
  {
    /** Follows Pointer **/
    el = (listelement *) listAnchor.next;

    if(el != NULL)
    {
      /** Deletes Icons **/
      windowDeleteIcon(mainWindow, counter);
      windowDeleteIcon(mainWindow, counter + 1);
      windowDeleteIcon(mainWindow, counter + 2);

      /** Removes From List And Frees Up Memory **/
      linkListUnlink(&listAnchor,(linkListHeader *) el);

      counter += 3;
    }
  } while(el != NULL);
  
}

/*****************************************************************************/
listelement *listingFindLink(int trackNumber)
{
  /** Searchs The Linked List For A Certain Item **/
  listelement *el;

  /** Gets First Element **/
  el = (listelement *) listAnchor.next;

  /** Follows The Pointers Until Reaches Element Or End **/
  while(el->header.next != NULL && el->trackNumber != trackNumber)
    el = (listelement *) el->header.next;

  /** Returns Element **/
  return(el);
}

/*****************************************************************************/
void listingSearchList(void)
{
  /** Searches The File For The CD **/
  int whichDisc = 0, counter = 0, error = NULL, tracks = 0, block[2];
  char *error2 = NULL, header = NULL;
  BOOL match = FALSE;
  FILE *catalogue;
  listelement *el;
  
  /** Opens The File **/
  catalogue = fopen("<CDPlay$Dir>.Listing","r");

  if(catalogue == NULL)
  {
    catalogue = fopen("<CDPlay$Dir>.Listing","w");
    fprintf(catalogue, "CDLISTING\n");
  }

  /** Sets File Position To Start Of File **/
  fseek(catalogue, SEEK_SET, 0);

  /** Works Out CD Total Playtime **/
  regs.r[0] = 0;
  regs.r[1] = (int)&block;
  _kernel_swi(XCD_DiscUsed, &regs, &regs);
  CDDiscID = block[0];

  /** Checks If Valid Listing **/
  error2 = fgets(tempData, 29, catalogue);
  if(strcmp(tempData, "CDLISTING\n") == 0 && error2 != NULL)
  {
    do
    {
      do
      {
        /** Searches For Header Of CD Info **/
        error = fscanf(catalogue, "%c", &header);
      } while((int)header != 137 && error != EOF);

      /** Reads In Playtime **/
      error = fscanf(catalogue, "%i\n", &whichDisc);

      /** Reads In Number Of Tracks **/
      error = fscanf(catalogue, "%i\n", &tracks);

      /** Checks If Correct CD **/
      if((whichDisc >= CDDiscID-CDOffset && whichDisc <= CDDiscID+CDOffset && tracks == maxTrack) 
      && error != EOF)
      {
        /** Reads in CD Title **/
        error2 = fgets(dataCDTitle, maxTrackLength, catalogue);

        /** Reads In Track Names **/
        el = (listelement *) listAnchor.next;
        for(counter = minTrack; counter <= maxTrack; counter++)
        {
          error2 = fgets(tempData, maxTrackLength, catalogue);

          /** Clears Track Name **/
          memset(el->trackName, '\0', maxTrackLength);
          
          /** Adds Data To linked List **/
          strcpy(el->trackName, tempData);
          el = (listelement *) el->header.next;
        }

        /** Sets match to TRUE **/
        match = TRUE;
      }
    } while(!match && error != EOF);
  }

  /** Closes Data File **/
  fclose(catalogue);

  /** Checks If CD Data Was Found **/
  if(error == EOF || error2 == NULL)
  {
    strcpy(dataCDTitle, "Unknown");
    listingNotFound();
  }
  
  /** Sets CD Title **/
  windowIconSetText(mainWindow, iconCDTitle, dataCDTitle);

  /** Sets Track Name **/
  el = (listelement *) listingFindLink(currentTrack);
  windowIconSetText(mainWindow, iconTrackName, el->trackName);
}

/*****************************************************************************/
void listingNotFound(void)
{
  /** Sets Variables If CD Not Found **/
  int counter;
  listelement *el;
  
  /** Marks CD as Unknown **/
  CDFlags.data.CDUnknown = TRUE;

  /** Resets Track Names **/
  el = (listelement *) listAnchor.next;
  for(counter = minTrack; counter <= maxTrack; counter++)
  {
    /** Clears Track Name **/
    memset(el->trackName, '\0', maxTrackLength); 

    /** Adds Data To linked List **/
    strcpy(el->trackName, "\0\1\2");
    el = (listelement *) el->header.next;
  }
}

/*****************************************************************************/
memoryElement *listingFindMemoryItem(int trackNumber, linkListHeader *anchor)
{
  /** Searchs The Linked List For A Certain Item **/
  memoryElement *search;

  /** Gets First Element **/
  search = (memoryElement *) anchor->next;

  /** Follows The Pointers Until Reaches Element Or End **/
  while(search != NULL && search->trackNumber != trackNumber)
    search = (memoryElement *)search->header.next;

  /** Returns Element **/
  return(search);
}

/*****************************************************************************/
void listingClearList(linkListHeader *anchor)
{
  /** Wipes The Linked List **/
  memoryElement *el;

  do
  {
    /** Follows Pointer **/
    el = (memoryElement *) anchor->next;

    if(el != NULL)
      linkListUnlink(anchor,(linkListHeader *) el);

  } while(el != NULL);
}

/*****************************************************************************/
void listingSaveData(void)
{
  /** Saves The Current Data To File **/
  int counter, trackNameLength = 0;
  char *error = &tempData[0];
  FILE *catalogue, *wimpscrap;
  BOOL internalTempFile = FALSE;
  listelement *el;
  
  /** Checks If File Exists **/
  catalogue = fopen("<CDPlay$Dir>.Listing","r");
  if(catalogue == NULL)
  {
    catalogue = fopen("<CDPlay$Dir>.Listing","w");
    fprintf(catalogue, "CDLISTING\n");
    fclose(catalogue);
  }
  else
    /** Closes File **/
    fclose(catalogue);

  /** Gets CD Title **/
  windowIconGetText(mainWindow, iconCDTitle, dataCDTitle);

  /** Sets Write Access To File **/
  regs.r[0] = 4;
  regs.r[1] = (int)"<CDPlay$Dir>.Listing";
  regs.r[5] = FileWriteAccess;
  _kernel_swi(XOS_File, &regs, &regs);
  
  /** Checks If Disc Unknown **/
  if(CDFlags.data.CDUnknown)
  {
    /** Opens File **/
    catalogue = fopen("<CDPlay$Dir>.Listing","a");

    /** Writes Out Header **/
    fprintf(catalogue, "%c%i\n%i\n%s\n", (char)137, CDDiscID, maxTrack, dataCDTitle);

    /** Writes Out Tracks **/
    el = (listelement *) listAnchor.next;
    for(counter = minTrack;counter <= maxTrack;counter++)
    {
      fprintf(catalogue, "%s\n",el->trackName);
      el = (listelement *) el->header.next;
    }

    /** Sets Unknown To False **/
    CDFlags.data.CDUnknown = FALSE;

    /** Closes File **/
    fclose(catalogue);
  }
  else
  {
    /** Opens File **/
    catalogue = fopen("<CDPlay$Dir>.Listing","r");

    /** Opens File In WimpScrap **/
    wimpscrap = fopen("<Wimp$ScrapDir>.CDListing","w");

    if(wimpscrap == NULL)
    {
      /** Opens Temp File Inside Applicatin Directory **/
      wimpscrap = fopen("<CDPlay$Dir>.TempList","w");
      internalTempFile = TRUE;
    }

    /** Turns Hourglass On **/
    _kernel_swi(XHourglass_On, &regs, &regs);

    /** Copies Data **/
    while(fgets(tempData, maxTrackLength, catalogue) && (error != NULL))
    {
      /** Writes Data To Scrap **/
      fputs(tempData, wimpscrap);

      /** Checks If Strings Match **/
      counter = atoi(&tempData[1]);
      if(counter >= CDDiscID-CDOffset && counter <= CDDiscID+CDOffset)
      {
        /* Reads In Track Number */
        error = fgets(tempData, maxTrackLength, catalogue);

        /** Outputs Track Number **/
        fprintf(wimpscrap,"%i\n", maxTrack);

        if(atoi(tempData) == maxTrack)
        {
          /** Reads In CD Title **/
          error = fgets(tempData, maxTrackLength, catalogue);

          /** Writes Out CD Title **/
          windowIconGetText(mainWindow, iconCDTitle, tempData);
          trackNameLength = strlen(tempData);
          if(tempData[trackNameLength-1] != '\n')
            fprintf(wimpscrap, "%s\n", tempData);
          else
            fprintf(wimpscrap, "%s", tempData);

          /** Reads In Track Data **/
          for(counter = minTrack; counter <= maxTrack; counter++)
          {
            error = fgets(tempData, maxTrackLength, catalogue);
            if(error == NULL)
              counter = maxTrack + 1;
          }

          /** Writes Out Track Data **/
          el = (listelement *) listAnchor.next;
          for(counter = minTrack;counter <= maxTrack;counter++)
          {
            trackNameLength = strlen(el->trackName);
            if(el->trackName[trackNameLength-1] != '\n')
              fprintf(wimpscrap, "%s\n",el->trackName);
            else
              fprintf(wimpscrap, "%s",el->trackName);

            el = (listelement *) el->header.next;
          }
        }
      }
    }

    /** Closes File **/
    fclose(wimpscrap);

    /** Closes File **/
    fclose(catalogue);

    /** Copies File Back **/
    if(!internalTempFile)
      _kernel_oscli("Copy <Wimp$ScrapDir>.CDListing <CDPlay$Dir>.Listing D~CF { > null: }");
    else
      _kernel_oscli("Copy <CDPlayDir>.TempList <CDPlay$Dir>.Listing D~CF { > null: }");

    /** Updates Window **/
    CDUpdateDiscInfo();

    /** Turns Hourglass Off **/
    _kernel_swi(XHourglass_Smash, &regs, &regs);

  }

  /** Sets Read Access To File **/
  regs.r[0] = 4;
  regs.r[1] = (int)"<CDPlay$Dir>.Listing";
  regs.r[5] = FileReadAccess;
  _kernel_swi(XOS_File, &regs, &regs);
}

/*****************************************************************************/
int listingGetProgramTrack(void)
{
  /** Gets Specified Program Track Number **/
  int counter;
  memoryElement *el;
  
  if(memoryItems <= 0)
    return 0;
    
  /** Checks Pointer Position **/
  if(memoryPosition > memoryItems)
  {
    memoryPosition = 1;
    if(!CDFlags.data.autoRepeatCD || CDFlags.data.playOverride)
      stopped = TRUE;
  }
  else
    if(memoryPosition < 1)
      memoryPosition = memoryItems;
     
  /** Searches List **/
  el = (memoryElement *) memoryAnchor.next;

  counter = 0;
  while(++counter < memoryPosition)
    if(el->header.next != NULL)
      el = (memoryElement *) el->header.next;
  
  /** Returns Track Number **/
  return(el->trackNumber);
}
      
        


      
      


        
