/******************************************************************************

Program:        Telnet

File:           Main.c

Function:       A telnet client using the BTTelnet module for RISC OS

Description:    An SSL capable telnet client

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sat 29th July 2000
                     Creation.

******************************************************************************/

// includes
#define PROTOTYPES_INTERNAL
#include "common.h"
#include "process.H"

// Local variables
static int *pollword    = NULL;
static int fakePollword = 1;
static int sendUserName = 0;
static int forceSSL     = 0;
static int noSSL        = 0;
static int secureSSL    = 0;

/** Procedures --------------------------------------------------------------*/

static void usage(char *name) {
  printf("Usage: %s [-l <Username>] [-L] [-z <ssl | nossl | secure>] hostname [port]\n",name);
}

/**
 * IsModulePresent
 *
 * Checks if the specified module is present.
 *
 * sName - The name of the module.
 *
 * Returns TRUE if present, FALSE otherwise.
 */
static BOOL IsModulePresent(char *sName) {
  if(_swix(OS_Module, _INR(0, 1), 18, sName)) {
    return FALSE;
  }

  // Success
  return TRUE;
}

/**
 * read_args
 *
 * Reads in arguments from the command line,
 * failing that it will prompt the user for input.
 *
 * argc - Number of arguments
 * argv - List of arguments
 *
 * Returns TRUE if got valid results,
 *         FALSE if failed
 */
static BOOL read_args(int argc, char *argv[]) {
  int i;
  char *colon = NULL;

  // Are there enough arguments?
  if(argc < 2) {
    printf("Host to connect to: ");
    gets(hostname);
  }
  else {
    for(i = 1; i < argc; i++) {
      // Send user name?
      if(strcmp(argv[i], "-l") == 0) {
        // user name
        if((i + 1) < argc) {
          sendUserName = 1;

          strncpy(userName, argv[i+1], USER_NAME_SIZE);

          // Skip extra argument
          i++;
        }
      }
      // Help
      else if(strcmp(argv[i], "-h") == 0) {
        usage(argv[0]);
        return FALSE;
      }
      // Use line mode?
      else if(strcmp(argv[i], "-L") == 0) {
        line_mode = TRUE;
        echo = TRUE;
      }
      else if(strcmp(argv[i], "-z") == 0) {
        // SSL parameter

        if((i + i) < argc) {
          // Which one?
          if(strcmp(argv[i+1], "ssl") == 0) {
            // Force SSL
            forceSSL = 1;
          }
          else if(strcmp(argv[i+1], "nossl") == 0) {
            // Don't use SSL
            noSSL = 1;
          }
          else if(strcmp(argv[i+1], "secure") == 0) {
            // Secure session only
            secureSSL = 1;
          }

          // Skip arg
          i++;
        }
      }
      else {
        // Assume its the name
        if(i == (argc - 1)) {
          // Is there a port number included?
          if(strchr(argv[i], ':') == NULL)
            sprintf(hostname, "%s:23", argv[i]);
          else
            sprintf(hostname, "%s", argv[i]);
        }
        else {
          // Hostname and port?
          if(i+1 == (argc -1)) {
            sprintf(hostname, "%s:%s", argv[i], argv[i+1]);
            i++;
          }
          else {
            printf("Usage: Telnet [-l][-s] <Hostname> <Port>\n");
            return FALSE;
          }
        }
      }
    }
  }

  // Split up into hostname and port.
  if((colon = strrchr(hostname, ':')) != NULL) {
    *colon = '\0';
    port = atoi(colon + 1);
  }
  else if((colon = strrchr(hostname, ' ')) != NULL) {
    *colon = '\0';
    port = atoi(colon + 1);
  }

  // Success
  return TRUE;
}

/**
 * task_exit
 *
 * Called when task is killed off.
 */
void task_exit(void) {
  if(session >= 0)
    _swix(BTTelnet_Close, _IN(0), session);
}

/**
 * setup and connect
 */
int main(int argc, char *argv[]) {
  int status;
  int flags = 0;

  // Set up exit handler
  if(atexit(task_exit))
    return 1;

  // Check for existance of BTTelnet module
  if(_swix(OS_Module, _INR(0, 1), 18, "BTTelnet")) {
    printf("BTTelnet module not found.\n");
    return 1;
  }

  // Check if running in a taskwindow?
  if(_swi(TaskWindow_TaskInfo, _RETURN(0)) == 0)
    task_window = FALSE;
  else
    task_window = TRUE;

  // Read in arguments.
  if(read_args(argc, argv) == FALSE) {
    // read_args will have printed out a valid error message
    return 1;
  }

  printf("Attempting to connect to %s:%d\n", hostname, port == -1 ? 23 : port);

  // Setup default flags
  if(line_mode == FALSE) {
    flags = BTT_FLAGS_SUPPORT_ECHO;
  }

  // Only use secure SSL?
  if(secureSSL) {
    flags |= BTT_FLAGS_FORCE_SSL;
  }

  // Disable SSL?
  if(noSSL == 0) {
    flags |= BTT_FLAGS_SSL;
  }

  // Only use SSL?
  if(forceSSL == 1) {
    flags |= BTT_FLAGS_FORCE_SSL | BTT_FLAGS_SSL | BTT_FLAGS_RAW;
  }

  // Create session
  if(_swix(BTTelnet_Open, _INR(0, 4) | _OUTR(0, 1), hostname, port, flags, "network",
           sendUserName ? userName : NULL, &session, &pollword))
  {
    printf("failed to open connection\n");
    return 1;
  }

  // Successfully create?
  if(session < 0) {
    printf("Failed to create session, error = [%d - %s]\n", session, getError(session));
    return 1;
  }

  // Set up keystatus
  _swix(OS_Byte, _INR(0, 2), 229, 1, 1);

  // Loop until connected
  do {
    if(_swix(BTTelnet_Status, _IN(0) | _OUT(0), session, &status)) {
      printf("Error occured whilst reading status\n");
      return 1;
    }
  } while(status >= 0 && (!(status & BTT_STATUS_CONNECTED) || (status & BTT_STATUS_NEGOTIATING)));

  // Success
  if(status < 0) {
    printf("Connection failed with error [%d - %s]\n", status, getError(status));
    return 1;
  }

  // Connected
  printf("Connected.\n");

  // Output status
  if(status & BTT_STATUS_SSL) {
    printf("[SSL Connection]\n");
  }
  else {
    printf("[Non-SSL Connection]\n");
    if(secureSSL) {
      printf("Closing connection\n");
      return 0;
    }
  }

  // Main loop
  while(TRUE) {
    // Process session data
    if(!process_session())
      break;

    // Process input
    if(!process_input())
      break;

    // If in taskwindow, then sleep
    if(task_window)
      _swix(OS_UpCall, _INR(0, 1), 6, &fakePollword);

    if(_swix(BTTelnet_Status, _IN(0) | _OUT(0), session, &status)) {
      printf("Error occured whilst reading status\n");
      return 1;
    }
    else {
      if(status & BTT_STATUS_CONN_LOST) {
        printf("Connection lost\n");
        break;
      }
      else if(status & BTT_STATUS_FINISHED) {
        break;
      }
      else if(status & BTT_STATUS_ABORTED) {
        printf("Session aborted\n");
        break;
      }
      else if(status < 0) {
        printf("Got error [%d - %s]\n", status, getError(status));
        break;
      }
      else if(status & BTT_STATUS_LOCAL_ECHO) {
        echo = TRUE;
      }
      else {
        echo = FALSE;
      }
    }
  }

  printf("Connection closed\n");

  // Close session
  if(session >= 0) {
    _swix(BTTelnet_Close, _IN(0), session);
    session = -1;
  }

  // Exit successfully
  return 0;
}

/**
 * getErrorText
 *
 * Returns the text message for the specifed error
 * number
 */
char *getError(int error) {

  switch(error) {
    case BTT_ERROR_NO_SESSIONS: return "No sessions available for use.";
    break;

    case BTT_ERROR_DNS_FAILED: return "Failed to resolve hostname";
    break;

    case BTT_ERROR_SSL_FAILED:
      if(TRUE == IsModulePresent("SecureSockets")) {
        return "Failed to establish an SSL connection";
      }
      else{
        return "Failed to establish an SSL connection (SecureSockets module not loaded)";
      }
    break;

    case BTT_ERROR_CONN_FAILED: return "Connection failed";
    break;

    case BTT_ERROR_CONN_CLOSED: return "Connection closed by remote host";
    break;

    case BTT_ERROR_CONN_LOST: return "Connection lost";
    break;

    case BTT_ERROR_INVALID_ID: return "Invalid ID passed to BTTelnet module";
    break;

    case BTT_ERROR_INVALID_CONFIG: return "Invalid configuration value used";
    break;

    case BTT_ERROR_NO_MEMORY:      return "Insufficent memory";
    break;

    case BTT_ERROR_NOT_CONNECTED: return "Not connected";
    break;

    case BTT_ERROR_INVALID_ARGS: return "Invalid arguments passed";
    break;

    default: return "Unknown error occured";
    break;
  }

  return "Unknown error occured";
}

