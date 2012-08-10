/******************************************************************************

Program:        VNCInput

File:           Sockets.c

Function:       A VNC Input only Client for Risc OS

Description:    Handles socket connections etc.

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Sun 21st November 1999 - Creation

******************************************************************************/

/** Includes. */
#include "VNCInput.h"

/** Global variables. */
socketBuff *memBuffer = NULL;

/*****************************************************************************/
int socketGenerate(char *hostname, int port) {
  /** Attempts to open a connection to the specified host. */
  struct hostent *he;
  struct sockaddr_in servAddr;
  int s, flagData;

  /** Sets up structure. */
  bzero((void *)&servAddr, sizeof(servAddr));
  servAddr.sin_port = htons(port);

  /** Check if a numeric ip, or name. */
  if(sscanf(hostname, "%i.%i.%i.%i", &flagData, &flagData, &flagData, &flagData) == 4) {
    /** Convert to network binary format. */
    if(inet_pton(hostname, (unsigned char *)&servAddr.sin_addr) == 0)
      return -2;

    /** Set address type. */
    servAddr.sin_family = AF_INET;
  }
  else {
    he = resolveName(hostname);

    /** Checks if successful. */
    if(he == NULL)
      return -2;

  /* Memcheck function call... Don't check reads for this section */
#ifdef VNC_MemCheck
  MemCheck_SetReadChecking(0);
#endif
    bcopy(he->h_addr, (char *)&servAddr.sin_addr, he->h_length);
    servAddr.sin_family = he->h_addrtype;
/* Memcheck function call... Turns read checking back on */
#ifdef VNC_MemCheck
  MemCheck_SetReadChecking(1);
#endif

  }

  /** Creates socket. */
  if((s = create(AF_INET, SOCK_STREAM, 0)) < 0) {
    windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Failed to create socket.");
    return -1;
  }

  /** Attempts to connect to the remote end. */
  if(connect(s, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
    close(s);
    windowIconSetText(connectingWindow, CONNECTING_MIDDLE, "Connection failed.");
    return -1;
  }

  /** Disables nagle algorithm. */
  flagData = 0;
  if(disableNagle(s, &flagData, sizeof(flagData)) == -1)
    return -1;

  /** Returns the socket descriptior. */
  return s;
}

struct hostent *resolveName(char *hostname) {
  /** Resolvers a host name. */
  int regs[10];

  /** Sets pointer to host name. */
  regs[0] = (int)hostname;
  if(_swi(Resolver_GetHost, _IN(0) | _OUTR(0, 1), (int)hostname, &regs[0], &regs[1]))
    return NULL;

  switch(regs[0]) {
    /** Lookup was successful. */
    case 0:resolverStatus = 0;
    break;

    /** Host not found. */
    case -1:
    case -2:resolverStatus = 1;
    break;

    /** Waiting for resolve to complete. */
    case EINPROGRESS:resolverStatus = 2;
    break;
  }

  /** Returns. */
  return (struct hostent *)regs[1];
}

int inet_pton(char *src, CARD8 *dst) {
  /** Converts a string into a numeric host name. */
  static const char digits[] = "0123456789";
  int sawDigit, octets, ch;
  unsigned char tmp[4], *tp;

  /** Set up variables. */
  sawDigit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while((ch = *src++) != '\0') {
    const char *pch;

    /** Checks its a valid number. */
    if((pch = strchr(digits, ch)) != NULL) {
      unsigned int new = *tp * 10 + (pch - digits);

      if(new > 255)
        return 0;

      *tp = new;
      if(!sawDigit) {
        if(++octets > 4)
          return 0;
        sawDigit = 1;
      }
    } else if(ch == '.' && sawDigit) {
      if(octets == 4)
        return 0;
      *++tp = 0;
      sawDigit = 0;
    }
    else
      return 0;
  }

  if(octets < 4)
    return 0;

  memcpy(dst, tmp, 4);

  return 1;
}

void socketProcessInitInput(void) {
  /** Checks data when initialising VNC connection. */
  CARD32 waitingData;

  /** Gets size of data in buffer. */
  waitingData = socketGetBufferSize();

  if(waitingData == 0)
    return;

  /** Checks what we are waiting for. */
  switch(networkStatus) {
    /** Waiting for version number. */
    case NS_VERSION:
      if(waitingData >= VNCVersionLength)
        VNCProcessVersion();
    break;

    /** Waiting for password. */
    case NS_PASSWORD:
      if(waitingData >= VNCPasswordSize)
        VNCGetPassword();
    break;

    /** Waiting for response. */
    case NS_PASSWORD_RESPONSE:
      if(waitingData >= sizeOfCARD32)
        VNCPasswordResponse();
    break;

    /** Waiting for server init. */
    case NS_SERVER_INIT:
      if(waitingData >= VNCServerInitSize)
        VNCInitialiseDisplay();
    break;
  }
}

int socketGetBufferSize(void) {
  /** Returns the amount of data waiting in the buffer. */
  int result = 0;
  socketBuff *nextBuff;

  /** Sets up pointer. */
  nextBuff = memBuffer;

  /** Add up values. */
  while(nextBuff != NULL) {
    result += (nextBuff->size - nextBuff->offset);
    nextBuff = nextBuff->next;
  }

  /** Returns result. */
  return result;
}

int socketFillBuffers(void) {
  /** Fills up the buffer. */
  int waitingData = 0;
  socketBuff *nextBuff, *newBuffer;

  /** Checks if there is any data to read. */
  ioctl(VNCSocket, FIONREAD, &waitingData);

  if(waitingData <= 0)
    /** Nothing to read so return. */
    return 0;

  /** Allocate memory for structure and data. */
  newBuffer = (socketBuff *)malloc(sizeof(socketBuff));
  if(newBuffer == NULL)
    return 0;

  newBuffer->data = (char *)malloc(waitingData);

  /** Check if memory was allocated. */
  if(newBuffer->data == NULL)
    return 0;

  /** Read in data. */
  if((waitingData = read(VNCSocket, newBuffer->data, waitingData)) == 0) {
    /** Nothing copied, or error. */
    free(newBuffer->data);
    free(newBuffer);
    return 0;
  }
  else {
    if(waitingData == -1) {
      VNCCloseConnection();
      return -1;
    }
  }

  /** Set up other varaiables. */
  newBuffer->size = waitingData;
  newBuffer->offset = 0;
  newBuffer->next = NULL;

  /** Add to end of list. */
  if(memBuffer == NULL)
    memBuffer = newBuffer;
  else {
    /** Find end of list. */
    nextBuff = memBuffer;

    while(nextBuff->next != NULL)
      nextBuff = nextBuff->next;

    /** Set pointer. */
    nextBuff->next = newBuffer;
  }

  /** Finshed, return amount read. */
  return waitingData;
}

int socketReadData(void *buffer, int amount) {
  /** Reads data from the buffer. */
  int available, left;
  socketBuff *nextBuff;
  char *ptr;

  /** Read in data. */
  left = amount;
  ptr  = buffer;

  /** Main loop. */
  while(left > 0) {
    /** Set to point to start of buffer. */
    nextBuff = memBuffer;
    if(nextBuff == NULL) {
      /** Read in more data. */
      if(socketFillBuffers() < 0)
        break;

      nextBuff = memBuffer;
    }
    else {
      /** Copy data. */
      available = nextBuff->size - nextBuff->offset;
      if(left > available) {
        /** Only copy as much as is in buffer. */
        if(available > 0) {
          memcpy(ptr, &nextBuff->data[nextBuff->offset], available);
          left -= available;
          ptr += available;
        }

        /** Free the buffer. */
        memBuffer = nextBuff->next;
        free(nextBuff->data);
        free(nextBuff);
      }
      else {
        /** Copy as much as needed. */
        memcpy(ptr, &nextBuff->data[nextBuff->offset], left);
        nextBuff->offset += left;
        ptr += left;
        left = 0;
      }
    }
  }

  /** Return amount read. */
  return (amount - left);
}

