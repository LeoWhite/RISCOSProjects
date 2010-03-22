/******************************************************************************

Program:        JPEGView

File:           Main.c

Function:       Displays JPEG files

Description:    Main Program Routines

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 30th January 2000

******************************************************************************/

// Includes
#define PROTOTYPES_INTERNAL
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "JPEGView.h"
#include "wimp.h"
#include "button.h"
#include "display.h"
#include "JPEGdecode.h"

#ifdef USE_HEAP
  #include "heap.h"
  #include "stubshack.h"
#endif

// Global variables
char *windowHandles[] = { "Info", "Main" , NULL};
char *templateBuffer = NULL;

/****************************** Procedures. *********************************/
void mainTaskDispatch(CARD16 eventType, INT32 *event) {

  // Process Wimp events
  switch(eventType) {
    // NULL Event
    case WIMP_NULL:
      if(JPEGInfo) {
        // Process some more of the JPEG
        JV_DecodeWork(JPEGInfo);
      }
#ifdef USE_HEAP
      else {
        // Shrink the heap
        Heap_Shrink(NULL);
      }
#endif

      break;

    // Handles window redraws
    case WIMP_REDRAW:
      if(event[EVENT_WINDOW_HANDLE] == handles[MAIN_WINDOW]) {
        displayRedrawMain(event[EVENT_WINDOW_HANDLE]);
      }
    break;

    // Open window events
    case WIMP_OPEN:
      // Check if its main window
      if(event[EVENT_WINDOW_HANDLE] == handles[MAIN_WINDOW]) {
        statusFlags |= FLAG_MAIN_OPEN;
      }

      // Open the window.
      _swix(Wimp_OpenWindow, _IN(1), &event[0]);
    break;

    // Close window events
    case WIMP_CLOSE:
      _swix(Wimp_CloseWindow, _IN(1), &event[0]);

      // Check if the main window is closed
      if(event[EVENT_WINDOW_HANDLE] == handles[MAIN_WINDOW]) {
        // Mark window as closed
        statusFlags &= ~FLAG_MAIN_OPEN;
      }
    break;

    // Handle button events
    case WIMP_MOUSE_CLICK:buttonClick(event);
    break;

    // Handle menu clicks
    case WIMP_MENU_SELECTION:buttonMenuClick(event);
    break;

    // Handle user messages
    case WIMP_USER_MESSAGE:
    case WIMP_USER_MESSAGE_REC:
    case WIMP_USER_MESSAGE_ACK:
      switch(event[MESSAGE_ACTION]) {
        // Quit message
        case MESSAGE_QUIT:statusFlags |= FLAG_QUIT;
        break;

        // Mode change
        case MESSAGE_MODECHANGE:wimpModeChange();
        break;

        // Dataload messages
        case MESSAGE_DATALOAD:JPEGDataLoad(event);
        break;

        // Datasave messages
        case MESSAGE_DATASAVE:JPEGDataSave(event);
        break;

        // Dataopen message
        case MESSAGE_DATAOPEN:JPEGDataOpen(event);
        break;

        default:
          fprintf(stderr, "Failed to handle message %d\n", event[MESSAGE_ACTION]);
        break;
      }
    break;
  }
}

/***********************************************************************/
_kernel_oserror *mainInitialise(void) {
  // Sets up any resources or values required.
  int messages[6];
  _kernel_oserror *error;

  // Check if we are already loaded.
  if(wimpEnumerateTasks(JPEGViewName)) {
    // Just exit quietly
    exit(0);
  }

  // Set up those messages we wish to receive
  messages[0] = MESSAGE_PREQUIT;
  messages[1] = MESSAGE_MODECHANGE;
  messages[2] = MESSAGE_DATASAVE;
  messages[3] = MESSAGE_DATALOAD;
  messages[4] = MESSAGE_DATAOPEN;
  messages[5] = 0;

  // Attempt to initialise task.
  if((error = _swix(Wimp_Initialise, _INR(0, 3) | _OUT(1), 310, WIMP_TASK, JPEGViewName,
                    messages, &JPEGViewTaskHandle)) != NULL) {
    // Report error
    return error;
  }

  // Initialised okay, so check what version of the OS we are running on.
  riscosVersion = _swi(OS_Byte, _INR(0, 3) | _RETURN(0), 129, 0, 0xFF);

  // Set up exit handler
  if(atexit(mainTaskExit))
    return &errFailedAtExit;


  // Create iconbar
  if((JPEGViewBarIcon = wimpCreateIconbar("!JPEGView", ICONBAR_RIGHT)) == -1) {
    return &errFailedIconbar;
  }

  // Load in the windows
  templateBuffer = wimpLoadTemplate(TEMPLATE_FILE, (const char **)&windowHandles, handles);

  // Create menu
  wimpCreateMainMenu();

  // Perform mode value setup.
  wimpModeChange();

  // Success
  return NULL;
}

/***********************************************************************/
void mainTaskExit(void) {

  // Free up buffers
  if(templateBuffer) free(templateBuffer);
  if(transTable) free(transTable);

  // Free dynamic area
  if(dynamicArea != 0) JPEGViewFreeMemory();

  // Dump the heap
#if defined(DEBUG) && defined(USE_HEAP)
  Heap_StatsAll();
#endif

  // Output debugging logs
# ifdef USE_MEMCHECK
  MemCheck_OutputBlocksInfo();
# endif



  // Closes down the wimp
  wimpTaskCloseDown(JPEGViewTaskHandle);
}

/***********************************************************************/
int main(int argc, char *argv[]) {
  // Main loop
  INT32 eventBlock[64];
  _kernel_oserror *error;
  int delay = 0;

  // Initialise heap
# ifdef USE_HEAP
  Heap_Initialise();
  StubsHack_ReplaceANSIAllocFns(Lib_malloc, Lib_realloc, Lib_calloc, Lib_free,
                                NULL,    NULL,     NULL,    NULL,
                                TRUE);
# endif

  /* Sets up MemCheck stuff */
# ifdef USE_MEMCHECK
  MemCheck_Init();
  MemCheck_InterceptSCLStringFunctions();
  MemCheck_SetStoreMallocFunctions(1);
  MemCheck_RegisterArgs(argc, argv);
# endif

  /* Sets up HierProf stuff */
# ifdef HierProf_PROFILE
  HierProf_ProfileAllFunctions();
#endif

  // Perform program initialisation.
  if((error = mainInitialise()) != NULL) {
    wimpReportError(error, 0, JPEGViewName, NULL, NULL, NULL);
    return 1;
  }

  // Heap finished initialising
# ifdef USE_HEAP
  Heap_InitialisationDone();
# endif

  // Check for a file to load.
  if(argc == 2) {
    // Load the file
    JPEGInfo = JV_DecodeCreate(argv[1]);
  }

  // Loop
  while(!(statusFlags & FLAG_QUIT)) {
    delay = _swi(OS_ReadMonotonicTime, _RETURN(0)) + pollDelay;

    // Perform a wimp poll
    mainTaskDispatch(_swi(Wimp_PollIdle, _INR(0,2) | _RETURN(0), WIMP_MASK, eventBlock,
                          delay), eventBlock);
  }

  // Return
  return 0;
}
