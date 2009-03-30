/******************************************************************************

Program:        BeyondThought Telnet

File:           BTT_api.h

Function:       A telnet client for RISC OS

Description:    Manages multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Thu 27th April 2000
                     Creation.

******************************************************************************/

// Check if header file is already included.
#ifndef  __BTT_API_H
# define __BTT_API_H

/****************************** Constants ******************************/

// Flags
#define BTT_FLAGS_SSL             (1<<0)    // Use SSL if available
#define BTT_FLAGS_FORCE_SSL       (1<<1)    // Only use SSL
#define BTT_FLAGS_RAW             (1<<2)    // Don't preprocess anything
#define BTT_FLAGS_STRIP_ANSI      (1<<3)    // Strip out ANSI code.
#define BTT_FLAGS_SUPPORT_ECHO    (1<<4)    // Support local echo

// Status
#define BTT_STATUS_RESOLVING      (1<<0)    // Resolving hostname.
#define BTT_STATUS_CREATING       (1<<1)    // Connecting to remote server.
#define BTT_STATUS_CONNECTING     (1<<2)    // Connecting to remote server.
#define BTT_STATUS_NEGOTIATING    (1<<3)    // Negotiating telnet connection.
#define BTT_STATUS_CONNECTED      (1<<4)    // Connected to server.
#define BTT_STATUS_CREATING_SSL   (1<<5)    // Connecting to SSL server
#define BTT_STATUS_CONNECTING_SSL (1<<6)    // Connecting to SSL server
#define BTT_STATUS_SSL            (1<<7)    // SSL connection.
#define BTT_STATUS_OUTGOING       (1<<8)    // Data waiting to be sent.
#define BTT_STATUS_INCOMING       (1<<9)    // Data waiting to be read.
#define BTT_STATUS_LOCAL_ECHO     (1<<10)   // Local echo is enabled

#define BTT_STATUS_CONN_LOST      (1<<28)   // Connection lost.
#define BTT_STATUS_FINISHED       (1<<29)   // Session finished.  (Successful terminate)
#define BTT_STATUS_ABORTED        (1<<30)   // Session was aborted. (Error terminated)


// Error results
#define BTT_ERROR_NO_SESSIONS    (-1)      // No sessions available
#define BTT_ERROR_DNS_FAILED     (-2)      // Failed to resolve hostname
#define BTT_ERROR_SSL_FAILED     (-3)      // Failed to establish SSL connection
#define BTT_ERROR_CONN_FAILED    (-4)      // Failed to connect to server
#define BTT_ERROR_CONN_CLOSED    (-5)      // Server closed the connection
#define BTT_ERROR_CONN_LOST      (-6)      // Connection lost.
#define BTT_ERROR_INVALID_ID     (-7)      // Invalid Session ID
#define BTT_ERROR_INVALID_CONFIG (-8)      // Invalid configuration value
#define BTT_ERROR_NO_MEMORY      (-9)      // Ran out of memory
#define BTT_ERROR_NOT_CONNECTED  (-10)     // Not connected
#define BTT_ERROR_INVALID_ARGS   (-11)     // Invalid arguments passed.

// Pollword data
#define BTT_POLLWORD_READ        (1 << 0)  // Data waiting to be read.

// SWI's
#ifndef BTTelnet_00
#define BTTelnet_00               (0x00054cc0)
#define BTTelnet_Open             (0x00054cc0)
#define BTTelnet_Close            (0x00054cc1)
#define BTTelnet_Read             (0x00054cc2)
#define BTTelnet_Write            (0x00054cc3)
#define BTTelnet_Status           (0x00054cc4)
#define BTTelnet_Configure        (0x00054cc5)
#endif /* BTTelnet_00 */

/****************************** TypeDefines ****************************/
typedef int BTT_ID;

/****************************** Structures *****************************/


/******************************** Globals *****************************/

// End of include file
#endif // __BTT_API_H
