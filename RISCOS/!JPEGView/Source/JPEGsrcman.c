/******************************************************************************

Program:        JPEGView

File:           JPEGsrcman.c

Function:       Displays JPEGs

Description:    Handles the reading in of a JPEG file

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Tue 23rd May 2000 - Creation

******************************************************************************/

// Includes
#include "common.h"
#include "JPEGdecode.h"

// Defines
#define BUFFER_SIZE (96 * 1024)

// Global variables
typedef struct {
  // Public/shared data
  struct jpeg_source_mgr pub;

  // Private data
  FILE         *file;              // The file being read from
  char         *buffer;            // The data buffer
  int          numBytes;           // We need to keep our own track of whats in the buffer
  size_t       maxSize;            // The size of the buffer

  int skipBytes;                   // Count of how many bytes to skip
} JPEGsrcmanMgr_struct;

// Pointer
typedef JPEGsrcmanMgr_struct *JPEGsrcmanMgr_ptr;

// Procesdure prototypes
void JPEGsrcmanDestroyManager(struct jpeg_source_mgr *srcmam);
void JPEGsrcmanGetInput(JPEGInfo_t *JPEGInfo);

/****************************** Procedures. *********************************/

/**
 * JPEGsrcmanGetInput
 *
 * This routine is the one that actually reads in the input
 */
void JPEGsrcmanGetInput(JPEGInfo_t *JPEGInfo) {
  JPEGsrcmanMgr_ptr srcman = (JPEGsrcmanMgr_ptr)JPEGInfo->cinfo.src;
  int free, read;
  char *input;

  DBUG(fprintf(stderr, "JPEGsrcmanGetInput: Number of bytes in buffer = %d\n", srcman->pub.bytes_in_buffer));

  // Check if we need to shuffle down any data
  if(srcman->pub.bytes_in_buffer < srcman->numBytes) {
    // Need to shuffle down data
    DBUG(fprintf(stderr, "JPEGsrcmanGetInput: Moving %d bytes of data\n", srcman->pub.bytes_in_buffer));
    memmove(srcman->buffer, srcman->pub.next_input_byte, srcman->pub.bytes_in_buffer);
    srcman->numBytes = srcman->pub.bytes_in_buffer;
  }

  // How much room is left in the buffer?
  free = srcman->maxSize - srcman->numBytes - 1;

  DBUG(fprintf(stderr, "JPEGsrcmanGetInput:Amount free in buffer = %d\n", free));

  if(free > 0 && !(JPEGInfo->flags & JV_FLAGS_COMPLETED)) {
    DBUG(fprintf(stderr, "JPEGsrcmanGetInput: Reading in %d bytes of data\n", free));

    // Read in data
    input = srcman->buffer + srcman->numBytes;
    read = fread(input, 1, free, srcman->file);

    DBUG(fprintf(stderr, "JPEGsrcmanGetInput: Read %d bytes\n", read));

    // Was it zero?
    if(read == 0) {
      // End of file
      JPEGInfo->flags |= JV_FLAGS_COMPLETED;

      if(srcman->pub.bytes_in_buffer == 0) {
        DBUGF("Inserting EOI\n");
        // Insert data
        srcman->buffer[0] = (JOCTET) 0xFF;
        srcman->buffer[1] = (JOCTET) JPEG_EOI;
        srcman->numBytes = 2;
      }
    }
    else {
      // Are we supposed to be skipping input?
      if(srcman->skipBytes > 0) {
        if(srcman->skipBytes > read) {
          srcman->skipBytes -= read;
          read = 0;
        }
        else {
          char *dest;

          // Move data down in buffer
          dest = input = srcman->buffer + srcman->numBytes;
          input += srcman->skipBytes;
          read -= srcman->skipBytes;
          srcman->skipBytes = 0;
          memmove(dest, input, read);
        }
      }

      srcman->numBytes += read;
    }
  }
 else {
    if(free > 2 && JPEGInfo->flags & JV_FLAGS_COMPLETED) {
      DBUGF("Inserting EOI\n");
      // Insert data
      srcman->buffer[srcman->numBytes] = (JOCTET) 0xFF;
      srcman->buffer[srcman->numBytes + 1] = (JOCTET) JPEG_EOI;
      srcman->numBytes += 2;
    }
 }

  // Reset pointers
  srcman->pub.next_input_byte = (JOCTET *)srcman->buffer;
  srcman->pub.bytes_in_buffer = srcman->numBytes;

  DBUG(fprintf(stderr, "JPEGsrcmanGetInput: NUmber of bytes in buffer = %d\n", srcman->numBytes));
}

/**
 * JPEGsrcmanInitSource
 *
 * Initialises the JPEG source. For a suspending source manager this doesn't
 * do anything.
 *
 * cinfo - Pointer to the JPEG information structure
 */
void JPEGsrcmanInitSource(j_decompress_ptr cinfo) {
  // Do nothing
  DBUGF("JPEGsrcmanInitSource: Nothing to do.\n");
}

/**
 * JPEGsrcmanFillBuffer
 *
 * Fills in the input buffer. For a suspending source manager this returns FALSE
 * to tell the JPEG decompresser to suspend, or fills the buffer with EOIs (End of Image)
 * tags if there is no more data to fetch.  This allows the decompressor to display as much
 * of the JPEG image as possible.
 *
 * cinfo - Pointer to the JPEG information structure
 *
 * Returns FALSE is no more data has been added,
 *         TRUE if more data has been added.
 */
boolean JPEGsrcmanFillBuffer(j_decompress_ptr cinfo) {
  // Fills in BUFFER with EOIs if no more input is available, otherwise returns FALSE
  DBUGF("JPEGsrcmanFillBuffer: Filling buffer.\n");

  return false;
}

/**
 * JPEGsrcmanSkipInput
 *
 * Skips the specified amounts of bytes.  The JPEG decompressor
 * should not be called again until all the bytes are skipped.
 *
 * cinfo    - Pointer to the JPEG information structure
 * numBytes - The number of bytes to skip
 */
void JPEGsrcmanSkipInput(j_decompress_ptr cinfo, long numBytes) {
  // Fiddle pointers here
  JPEGsrcmanMgr_ptr srcman = (JPEGsrcmanMgr_ptr)cinfo->src;
  char *inptr;
  int left;

  DBUG(fprintf(stderr, "JPEGsrcmanSkipInput: Skipping %ld bytes of data.\n", numBytes));
  DBUG(fprintf(stderr, "JPEGsrcmanSkipInput: Amount in buffer = %d\n", srcman->pub.bytes_in_buffer));
  // Is there enough to handle it?
  if(srcman->pub.bytes_in_buffer >= numBytes) {
    left = srcman->pub.bytes_in_buffer - (int)numBytes;

    DBUG(fprintf(stderr, "JPEGsrcmanSkipInput: Removing %ld bytes from buffer, %d left\n", numBytes, left));
    inptr = ((char *)srcman->pub.next_input_byte) + numBytes;
    memmove(srcman->buffer, inptr, left);
  }
  else {
    DBUGF("Not enough data in buffer\n");
    srcman->skipBytes = (int)numBytes - srcman->pub.bytes_in_buffer;
    DBUG(fprintf(stderr, "Setting skipbytes to %d\n", srcman->skipBytes));
    left = 0;
  }

  // Set up pointers
  srcman->pub.next_input_byte = (JOCTET *)srcman->buffer;
  srcman->pub.bytes_in_buffer = srcman->numBytes = left;
}

/**
 * JPEGsrcmanTermSource
 *
 * Terminates the source and tidies up.  For a suspending source
 * manager this routine does nothing.
 *
 * cinfo    - Pointer to the JPEG information structure
 */
void JPEGsrcmanTermSource(j_decompress_ptr cinfo) {
  // Do nothing
  DBUGF("JPEGsrcmanTermSource: Nothing to do.\n");
}

/**
 * JPEGsrcmanGetManager
 *
 * Creates a new manager object, fills it in and returns a pointer
 * to the structure.
 *
 * cinfo    - Pointer to the JPEG information structure
 * filepath - Filepath of the file to load
 */
void JPEGsrcmanGetManager(j_decompress_ptr cinfo, const char *filepath) {
  JPEGsrcmanMgr_ptr srcman;

  DBUG(fprintf(stderr, "JPEGsrcmanGetManager: Creating manager for file \"%s\".\n", filepath));

  if(cinfo->src != NULL) {
    DBUGF("JPEGsrcmanGetManager: Destroying manager\n");
    JPEGsrcmanDestroyManager(cinfo->src);
  }

  // Create new structure
  if((srcman = calloc(1, sizeof(JPEGsrcmanMgr_struct))) == NULL) {
    DBUGF("JPEGsrcmanGetManager: Failed to allocate space for srcman structure.\n");
    return;
  }

  // Set pointer
  cinfo->src = (struct jpeg_source_mgr *)srcman;

  // Attempt to open file
  if((srcman->file = fopen(filepath, "rb")) == NULL) {
    DBUG(fprintf(stderr, "JPEGsrcmanGetManager: Failed to open file \"%s\".\n", filepath));
    JPEGsrcmanDestroyManager((struct jpeg_source_mgr *)srcman);
    return;
  }

  // Allocate buffer space
  if((srcman->buffer = calloc(1, BUFFER_SIZE)) == NULL) {
    DBUGF("JPEGsrcmanGetManager: Failed to allocate buffer\n");
    JPEGsrcmanDestroyManager((struct jpeg_source_mgr *)srcman);
    return;
  }

  // Fill in function pointers
  srcman->pub.init_source       = JPEGsrcmanInitSource;
  srcman->pub.fill_input_buffer = JPEGsrcmanFillBuffer;
  srcman->pub.skip_input_data   = JPEGsrcmanSkipInput;
  srcman->pub.resync_to_restart = jpeg_resync_to_restart;
  srcman->pub.term_source       = JPEGsrcmanTermSource;

  // Initialise ready for use
  srcman->pub.next_input_byte = NULL;
  srcman->pub.bytes_in_buffer = 0;
  srcman->maxSize = BUFFER_SIZE;

  DBUGF("JPEGsrcmanGetManager: Completed\n");
}

/**
 * JPEGsrcmanDestroyManager
 *
 * Destroys the source manager, closes files, frees memory etc.
 *
 * srcman - The source manager to destroy
 */
void JPEGsrcmanDestroyManager(struct jpeg_source_mgr *data) {
  JPEGsrcmanMgr_ptr srcman = (JPEGsrcmanMgr_ptr)data;

  DBUGF("JPEGsrcmanDestroyManager: Destroying manager\n");

  // Valid pointer
  if(data == NULL) {
    DBUGF("JPEGsrcmanDestroyManager: NULL Pointer passed.\n");
    return;
  }

  // Destroy buffer
  if(srcman->buffer) free(srcman->buffer);

  // Close file
  if(srcman->file) fclose(srcman->file);

  // Destory structure
  free(srcman);
}

