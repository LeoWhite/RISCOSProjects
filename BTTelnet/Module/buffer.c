/******************************************************************************

Program:        BeyondThought Telnet

File:           buffer.c

Function:       A telnet client module for RISC OS

Description:    Handles multiple telnet connections from this machine

Author:         Leo White

Enviroment:     Norcroft C

Revisions:      0.01 Sun 23rd April 2000
                     Creation.
                0.02 Sat 19th August 2000
                     Redesign to better get the telnet processing code
                     wholly inside the module

******************************************************************************/

// Includes
#include "assert.h"
#include "common.h"
#define  PROTOTYPES_INTERNAL
#include "BTT_buffer.h"

// Debug code?
#ifdef DEBUG
# include "BTDebug:BT_debug.h"
#endif


// Local procedures

/** Procedures --------------------------------------------------------------*/

/**
 * bufferCreate
 *
 * Allocates room for the buffer and sets up
 * initial state
 *
 * size - The size of the buffer in bytes
 * circular - Create a circular buffer, otherwise create linear
 *
 * Returns a pointer to the buffer, or NULL if failed.
 */
BTBuffer_t *bufferCreate(CARD32 size, BOOL circular) {
  BTBuffer_t *buffer = NULL;

  // Attempt to allocate space to hold the buffer
  if((buffer = calloc(1, sizeof(BTBuffer_t))) == NULL) {
    return NULL;
  }

  // Allocate buffer space
  if((buffer->data = calloc(1, size + 1)) == NULL) {
    free(buffer);
    return NULL;
  }

  // Set up values
  buffer->writePos = 0;
  buffer->readPos  = 0;
  buffer->used     = 0;
  buffer->size     = size;
  buffer->circular = circular;

  // Return pointer to buffer
  return buffer;
}

/**
 * bufferDestroy
 *
 * Frees up the memory allocated to a buffer.
 *
 * buffer - Pointer to buffer to free
 */
void bufferDestroy(BTBuffer_t *buffer) {
  // Check buffer is valid
  assert(buffer);

  // Free up data
  if(buffer->data)
    free(buffer->data);

  // Free up structure
  free(buffer);
}

/**
 * bufferReadPtr
 *
 * buffer - The buffer to use.
 *
 * If the buffer is a linear buffer, returns a pointer
 * to the read location.
 * If the buffer is circular, will return NULL
 */
char *bufferReadPtr(BTBuffer_t *buffer) {
  char *ptr = NULL;

  // Check buffer
  assert(buffer);

  // Only return pointer for linear buffer
  if(!buffer->circular)
    ptr = buffer->data + buffer->readPos;

  // Return pointer
  return ptr;
}

/**
 * bufferWritePtr
 *
 * buffer - The buffer to use.
 *
 * If the buffer is a linear buffer, returns a pointer
 * to the write location.
 * If the buffer is circular, will return NULL
 */
char *bufferWritePtr(BTBuffer_t *buffer) {
  char *ptr = NULL;

  // Check buffer
  assert(buffer);

  // Only return pointer for linear buffer
  if(!buffer->circular)
    ptr = buffer->data + buffer->writePos;
  else {
    assert(0);
  }

  // Return pointer
  return ptr;
}

/**
 * bufferDrain
 *
 * Drains the specified amount of data from the front of
 * the buffer.
 *
 * buffer - The buffer to use.
 * amount - Amount of data to drain
 *
 * Returns the number of bytes drained
 */
CARD32 bufferDrain(BTBuffer_t *buffer, CARD32 amount) {
  int available = 0;

  // Check buffer
  assert(buffer);

  // How much to remove
  available = MIN(amount, buffer->used);

  // What do we do?
  if(available == buffer->used) {
    buffer->readPos  = 0;
    buffer->writePos = 0;
    buffer->used     = 0;
  }
  else {
    if(buffer->circular) {
    }
    else {
      memmove(buffer->data, buffer->data + buffer->readPos + available, buffer->used - available);
      buffer->readPos   = 0;
      buffer->writePos -= available;
      buffer->used     -= available;
    }
  }

  // Return amount drained
  return available;
}

/**
 * bufferConsume
 *
 * 'Eats' the specified data from the start of the
 * buffer.
 *
 * buffer - The buffer to use.
 * amount - Amount of data to consume
 *
 * Returns the number of bytes dconsumed
 */
CARD32 bufferConsume(BTBuffer_t *buffer, CARD32 amount) {
  int available = 0;

  // Check buffer
  assert(buffer);

  // How much to add
  available = MIN(amount, bufferFree(buffer));

  // What do we do?
  if(buffer->circular) {
  }
  else {
    buffer->writePos += available;
    buffer->used     += available;
  }

  // Return amount consumed
  return available;
}

/**
 * bufferWrite
 *
 * Adds the specified data to the buffer.
 *
 * buffer - The buffer to add the data too.
 * data   - Pointer to data to add
 * size   - Size of data to add
 *
 * Returns the number of bytes written
 */
CARD32 bufferWrite(BTBuffer_t *buffer, char *data, CARD32 size) {
  CARD32 written = 0;

  // Check buffer is valid
  assert(buffer);
  assert(data);

  // How much can we write?
  written = MIN(size, bufferFree(buffer));

  // Copy data
  if(written) {
    if(buffer->circular) {
      assert(0);
    }
    else {
      memcpy((buffer->data + buffer->writePos), data, written);

      buffer->used += written;
      buffer->writePos += written;
    }
  }

  // Return amount written
  return written;
}

/**
 * bufferRead
 *
 * Copies the number of bytes out of the buffer
 *
 * buffer - The buffer to read from
 * data   - Pointer to area to copy data to
 * size   - max size of data to read
 * drain  - Remove bytes from buffer after use?
 *
 * Returns the number of bytes read
 */
CARD32 bufferRead(BTBuffer_t *buffer, char *data, CARD32 size, BOOL drain) {
  CARD32 read = 0;

  // Check buffer is valid
  assert(buffer);
  assert(data);

  // How much can we write?
  read = MIN(size, buffer->used);

  // Copy data
  if(read) {
    if(buffer->circular) {
    }
    else {
      memcpy(data, buffer->data + buffer->readPos, read);
      if(drain)
        bufferDrain(buffer, read);
    }
  }

  // Return amount read
  return read;
}

/**
 * bufferSize
 *
 * Returns the maximum size of a buffer
 *
 * buffer - The buffer to get size of
 *
 * Returns size of buffer
 */
CARD32 bufferSize(BTBuffer_t *buffer) {
  // Check buffer exists
  assert(buffer);

  // Return size
  return buffer->size;
}

/**
 * bufferFree
 *
 * Returns the amount of free space available in the buffer
 *
 * buffer - The buffer to get free size of
 *
 * Returns amount of free space
 */
CARD32 bufferFree(BTBuffer_t *buffer) {
  // Check buffer exists
  assert(buffer);

  // Return size
  return buffer->size - buffer->used;
}

/**
 * bufferUsed
 *
 * Returns the amount of buffer space used
 *
 * buffer - The buffer to get used size of
 *
 * Returns amount of space used in the buffer
 */
CARD32 bufferUsed(BTBuffer_t *buffer) {
  // Check buffer exists
  assert(buffer);

  // Return size
  return buffer->used;
}
