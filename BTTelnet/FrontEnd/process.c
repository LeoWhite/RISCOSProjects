/******************************************************************************

Program:        Telnet

File:           process.c

Function:       A telnet client using the BTTelnet module for RISC OS

Description:    An SSL capable telnet client

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sat 29th July 2000
                     Creation.

******************************************************************************/

// Includes
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "process.h"
#include "ctype.h"


// Prototypes
static void process_key(char key, char out);

/** Procedures --------------------------------------------------------------*/

/**
 * process_session
 *
 * Checks status of session, reading data etc.
 *
 * Returns TRUE if all successful,
 *         FALSE if failed (Socket closed etc).
 */
BOOL process_session(void) {
  int status = 0, waiting_data = 0, i, bytes_to_read;
  int read;
  BOOL redraw = FALSE, old_echo = echo;

  // Check the current status
  if(_swix(BTTelnet_Status, _IN(0) | _OUTR(0, 1), session, &status, &waiting_data)) {
    printf("process_session: Failed to read status\n");
    return FALSE;
  }

  // Any errors?
  if(status < 0) {
    printf("process_session: Got error [%d - %s] when getting status\n", status, getError(status));
    return FALSE;
  }

  // Echo mode?
  if(status & BTT_STATUS_LOCAL_ECHO) {
    echo = TRUE;
  }
  else {
    echo = FALSE;
  }

  // Anything to do?
  if(waiting_data == 0)
    return TRUE;


  /** Do we need to erase any keyboard data? */
  if(line_mode && key_size > 0 && echo) {
    int count = 0;

    // How many printable characters?
    for(i = 0; i < key_size; i++) {
      if(isprint(key_buffer[i]))
        count++;
    }

    // Erase data
    for(i = 0; i < count; i++)
      printf("\x7F");

    // Need to redraw later
    redraw = TRUE;
  }

  // Read in data
  while(waiting_data > 0) {
    bytes_to_read = MIN(waiting_data, BUFFER_SIZE);

    // Read in data
    _swix(BTTelnet_Read, _INR(0, 2) | _OUT(0), session, &buffer, bytes_to_read, &read);

    if(read == 0) {
      // Nothing to read or blocked.
      break;
    }

    // Scan through buffer.
    for(i = 0; i < read; i++) {
      // Check character
      switch(buffer[i]) {
        // If Vertical tab or form feed, do nothing
        case VT:
        case FF:
        break;

        // Is it a bell?
        case BEL:printf("\a");
        break;

        // Horizontal tab
        case HT:printf("\t");
        break;

        // Line feed
        case LF:printf("\n");
        break;

        // Default action is to output char
        default:printf("%c", buffer[i]);
        break;
      }
    }

    // Update amount waiting
    waiting_data -= read;

    // read in some input
    old_echo = echo;
    echo = FALSE;
    process_input();
    echo = old_echo;
  }

  // Do we need to redraw the keyboard?
  if(redraw) {
    for(i = 0; i < key_size; i++) {
      printf("%c", key_buffer[i]);
    }

    // Clear redraw
    redraw = FALSE;
  }

  // Return success
  return TRUE;
}

/**
 * process_input
 *
 * Reads data in from the keyboard, and sends to the remote server.
 *
 * Returns TRUE if all is happy,
 *         FALSE if something fails
 */
BOOL process_input(void) {
  int input, read;

  // Any keys pressed?
  _swix(OS_Byte, _INR(0, 2) | _OUTR(1, 2), 129, 0, 0, &input, &read);

  if(read == 0) {
    // What char pressed?
    switch(input) {
      // If NULL, do nothing
      case VT:
      case FF:
      case NULL:
      break;

      // Bell?
      case BEL:process_key('\a', BEL);
      break;

      // Horizontal tab
      case HT:process_key('\t', HT);
      break;

      // Line feed
      case LF:process_key('\n', LF);
      break;

      // Backspace or DEL
      case BS:
      case DEL:
        if(line_mode) {
          if(key_size > 0) {
            if(echo) {
              printf("\x7F");
            }

            key_size--;
          }
        }
        else
          process_key('\x7F', input);
      break;

      // Carriage return
      case CR:
        process_key('\n', CR);
        process_key('\0', LF);

        // Do we need to send the data?
        if(line_mode && key_size) {
          int sent = 0, error = 0;

          while(key_size > 0) {
            sent += _swi(BTTelnet_Write, _INR(0, 2) | _OUT(0) | _RETURN(1), session, (key_buffer + sent),
                                                                            key_size, &error);

            key_size -= sent;

            if(error < 0) {
              printf("Got error [%d - %s] whilst sending data\n", error, getError(error));
              return FALSE;
            }
          }
          key_size = 0;
        }
      break;

      // Erase line
      case eraseline:
        if(line_mode) {
          if(key_size > 0 && echo) {
            int count = 0, i;

            // How many printable characters?
            for(i = 0; i < key_size; i++) {
              if(isprint(key_buffer[i]))
                count++;
            }

            // Erase data
            for(i = 0; i < count; i++)
              printf("\x7F");

            // Mark buffer as empty
            key_size = 0;
          }
        }
        else {
          process_key(input, input);
        }
      break;

      default:process_key(input, input);
      break;
    }
  }

  return TRUE;
}

/**
 * process_key
 *
 * Works out what needs to be done to the key
 *
 * key - echoed chr
 * out - What is output
 */
static void process_key(char key, char out) {
  // In line mode?
  if(line_mode) {
    // Do we need to redraw keys?
    if(out == REDRAW && key_size) {
      int i;
      // Drop onto a new line
      printf("\n");

      // Output buffer
      for(i = 0; i < key_size; i++) {
        printf("%c", key_buffer[i]);
      }
    }
    else {
      if(echo && key != 0) {
        printf("%c", key);
      }

      key_buffer[key_size++] = out;
    }
  }
  else {
    // Send to remote end
    _swix(BTTelnet_Write, _INR(0, 2), session, &out, 1);
  }
}

