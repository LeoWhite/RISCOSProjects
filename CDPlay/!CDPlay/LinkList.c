/******************************************************************************

Program:        CDPlay

File:           LinkList.c

Function:       CD Player Wimp C Program

Description:    Linked List Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Tue 28th October 1997

******************************************************************************/

/** Includes **/
#include "CDPlay.h"

/**************************************************************************/
void linkListAddToTail(linkListHeader *anchor, linkListHeader  *item)
{
  /** Adds Item To The Tail Of The List **/
  linkListHeader *oldTail;
  
  /** Moves Old Tail **/
  oldTail = anchor->previous;
  
  /** Adds New Item To End Of List **/
  item->next = NULL;
  item->previous = oldTail;
  
  /** Checks if Only Item **/
  if(oldTail == NULL)
    anchor->next = item;
  else
    oldTail->next = item;
  
  /** Sets Pointer To New Tail **/
  anchor->previous = item;
}

/**************************************************************************/
void linkListUnlink(linkListHeader *anchor, linkListHeader *item)
{
  /** Removes A Link From The List **/
  linkListHeader *prev, *next;
  
  /** Sets Up Pointers To Start And End Of Item **/
  next = item->next;
  prev = item->previous;
  
  /** Checks If End Or Middle Of List **/
  if(next == NULL)
    anchor->previous = prev;
  else
    next->previous = prev;
    
  /** Checks If First In List **/
  if(prev == NULL)
    anchor->next = next;
  else 
    prev->next = next;
  
  /** Deallocates Item **/
  free(item);
}
