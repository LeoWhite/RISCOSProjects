/******************************************************************************

Program:        JPEGView

File:           JPEGDecode.c

Function:       Displays JPEGs

Description:    Handles processing the JPEG files

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Mon 22nd May 2000 - Creation

******************************************************************************/

// Includes
#include "common.h"
#define PROTOTYPES_INTERNAL
#include "JPEGdecode.h"
#include "wimp.h"
#include "display.h"

#ifdef USE_HEAP
#include "heap.h"
#endif

static void JV_DecodeLines(JPEGInfo_t *JPEGInfo);

// Global variables
static char defaultPalette[][3] = { {0x00, 0x00, 0x00},
 {0x11, 0x11, 0x11}, {0x22, 0x22, 0x22}, {0x33, 0x33, 0x33}, {0x44, 0x00, 0x00},
 {0x55, 0x11, 0x11}, {0x66, 0x22, 0x22}, {0x77, 0x33, 0x33}, {0x00, 0x00, 0x44},
 {0x11, 0x11, 0x55}, {0x22, 0x22, 0x66}, {0x33, 0x33, 0x77}, {0x44, 0x00, 0x44},
 {0x55, 0x11, 0x55}, {0x66, 0x22, 0x66}, {0x77, 0x33, 0x77}, {0x88, 0x00, 0x00},
 {0x99, 0x11, 0x11}, {0xAA, 0x22, 0x22}, {0xBB, 0x33, 0x33}, {0xCC, 0x00, 0x00},
 {0xDD, 0x11, 0x11}, {0xEE, 0x22, 0x22}, {0xFF, 0x33, 0x33}, {0x88, 0x00, 0x44},
 {0x99, 0x11, 0x55}, {0xAA, 0x22, 0x66}, {0xBB, 0x33, 0x77}, {0xCC, 0x00, 0x44},
 {0xDD, 0x11, 0x55}, {0xEE, 0x22, 0x66}, {0xFF, 0x33, 0x77}, {0x00, 0x44, 0x00},
 {0x11, 0x55, 0x11}, {0x22, 0x66, 0x22}, {0x33, 0x77, 0x33}, {0x44, 0x44, 0x00},
 {0x55, 0x55, 0x11}, {0x66, 0x66, 0x22}, {0x77, 0x77, 0x33}, {0x00, 0x44, 0x44},
 {0x11, 0x55, 0x55}, {0x22, 0x66, 0x66}, {0x33, 0x77, 0x77}, {0x44, 0x44, 0x44},
 {0x55, 0x55, 0x55}, {0x66, 0x66, 0x66}, {0x77, 0x77, 0x77}, {0x88, 0x44, 0x00},
 {0x99, 0x55, 0x11}, {0xAA, 0x66, 0x22}, {0xBB, 0x77, 0x33}, {0xCC, 0x44, 0x00},
 {0xDD, 0x55, 0x11}, {0xEE, 0x66, 0x22}, {0xFF, 0x77, 0x33}, {0x88, 0x44, 0x44},
 {0x99, 0x55, 0x55}, {0xAA, 0x66, 0x66}, {0xBB, 0x77, 0x77}, {0xCC, 0x44, 0x44},
 {0xDD, 0x55, 0x55}, {0xEE, 0x66, 0x66}, {0xFF, 0x77, 0x77}, {0x00, 0x88, 0x00},
 {0x11, 0x99, 0x11}, {0x22, 0xAA, 0x22}, {0x33, 0xBB, 0x33}, {0x44, 0x88, 0x00},
 {0x55, 0x99, 0x11}, {0x66, 0xAA, 0x22}, {0x77, 0xBB, 0x33}, {0x00, 0x88, 0x44},
 {0x11, 0x99, 0x55}, {0x22, 0xAA, 0x66}, {0x33, 0xBB, 0x77}, {0x44, 0x88, 0x44},
 {0x55, 0x99, 0x55}, {0x66, 0xAA, 0x66}, {0x77, 0xBB, 0x77}, {0x88, 0x88, 0x00},
 {0x99, 0x99, 0x11}, {0xAA, 0xAA, 0x22}, {0xBB, 0xBB, 0x33}, {0xCC, 0x88, 0x00},
 {0xDD, 0x99, 0x11}, {0xEE, 0xAA, 0x22}, {0xFF, 0xBB, 0x33}, {0x88, 0x88, 0x44},
 {0x99, 0x99, 0x55}, {0xAA, 0xAA, 0x66}, {0xBB, 0xBB, 0x77}, {0xCC, 0x88, 0x44},
 {0xDD, 0x99, 0x55}, {0xEE, 0xAA, 0x66}, {0xFF, 0xBB, 0x77}, {0x00, 0xCC, 0x00},
 {0x11, 0xDD, 0x11}, {0x22, 0xEE, 0x22}, {0x33, 0xFF, 0x33}, {0x44, 0xCC, 0x00},
 {0x55, 0xDD, 0x11}, {0x66, 0xEE, 0x22}, {0x77, 0xFF, 0x33}, {0x00, 0xCC, 0x44},
 {0x11, 0xDD, 0x55}, {0x22, 0xEE, 0x66}, {0x33, 0xFF, 0x77}, {0x44, 0xCC, 0x44},
 {0x55, 0xDD, 0x55}, {0x66, 0xEE, 0x66}, {0x77, 0xFF, 0x77}, {0x88, 0xCC, 0x00},
 {0x99, 0xDD, 0x11}, {0xAA, 0xEE, 0x22}, {0xBB, 0xFF, 0x33}, {0xCC, 0xCC, 0x00},
 {0xDD, 0xDD, 0x11}, {0xEE, 0xEE, 0x22}, {0xFF, 0xFF, 0x33}, {0x88, 0xCC, 0x44},
 {0x99, 0xDD, 0x55}, {0xAA, 0xEE, 0x66}, {0xBB, 0xFF, 0x77}, {0xCC, 0xCC, 0x44},
 {0xDD, 0xDD, 0x55}, {0xEE, 0xEE, 0x66}, {0xFF, 0xFF, 0x77}, {0x00, 0x00, 0x88},
 {0x11, 0x11, 0x99}, {0x22, 0x22, 0xAA}, {0x33, 0x33, 0xBB}, {0x44, 0x00, 0x88},
 {0x55, 0x11, 0x99}, {0x66, 0x22, 0xAA}, {0x77, 0x33, 0xBB}, {0x00, 0x00, 0xCC},
 {0x11, 0x11, 0xDD}, {0x22, 0x22, 0xEE}, {0x33, 0x33, 0xFF}, {0x44, 0x00, 0xCC},
 {0x55, 0x11, 0xDD}, {0x66, 0x22, 0xEE}, {0x77, 0x33, 0xFF}, {0x88, 0x00, 0x88},
 {0x99, 0x11, 0x99}, {0xAA, 0x22, 0xAA}, {0xBB, 0x33, 0xBB}, {0xCC, 0x00, 0x88},
 {0xDD, 0x11, 0x99}, {0xEE, 0x22, 0xAA}, {0xFF, 0x33, 0xBB}, {0x88, 0x00, 0xCC},
 {0x99, 0x11, 0xDD}, {0xAA, 0x22, 0xEE}, {0xBB, 0x33, 0xFF}, {0xCC, 0x00, 0xCC},
 {0xDD, 0x11, 0xDD}, {0xEE, 0x22, 0xEE}, {0xFF, 0x33, 0xFF}, {0x00, 0x44, 0x88},
 {0x11, 0x55, 0x99}, {0x22, 0x66, 0xAA}, {0x33, 0x77, 0xBB}, {0x44, 0x44, 0x88},
 {0x55, 0x55, 0x99}, {0x66, 0x66, 0xAA}, {0x77, 0x77, 0xBB}, {0x00, 0x44, 0xCC},
 {0x11, 0x55, 0xDD}, {0x22, 0x66, 0xEE}, {0x33, 0x77, 0xFF}, {0x44, 0x44, 0xCC},
 {0x55, 0x55, 0xDD}, {0x66, 0x66, 0xEE}, {0x77, 0x77, 0xFF}, {0x88, 0x44, 0x88},
 {0x99, 0x55, 0x99}, {0xAA, 0x66, 0xAA}, {0xBB, 0x77, 0xBB}, {0xCC, 0x44, 0x88},
 {0xDD, 0x55, 0x99}, {0xEE, 0x66, 0xAA}, {0xFF, 0x77, 0xBB}, {0x88, 0x44, 0xCC},
 {0x99, 0x55, 0xDD}, {0xAA, 0x66, 0xEE}, {0xBB, 0x77, 0xFF}, {0xCC, 0x44, 0xCC},
 {0xDD, 0x55, 0xDD}, {0xEE, 0x66, 0xEE}, {0xFF, 0x77, 0xFF}, {0x00, 0x88, 0x88},
 {0x11, 0x99, 0x99}, {0x22, 0xAA, 0xAA}, {0x33, 0xBB, 0xBB}, {0x44, 0x88, 0x88},
 {0x55, 0x99, 0x99}, {0x66, 0xAA, 0xAA}, {0x77, 0xBB, 0xBB}, {0x00, 0x88, 0xCC},
 {0x11, 0x99, 0xDD}, {0x22, 0xAA, 0xEE}, {0x33, 0xBB, 0xFF}, {0x44, 0x88, 0xCC},
 {0x55, 0x99, 0xDD}, {0x66, 0xAA, 0xEE}, {0x77, 0xBB, 0xFF}, {0x88, 0x88, 0x88},
 {0x99, 0x99, 0x99}, {0xAA, 0xAA, 0xAA}, {0xBB, 0xBB, 0xBB}, {0xCC, 0x88, 0x88},
 {0xDD, 0x99, 0x99}, {0xEE, 0xAA, 0xAA}, {0xFF, 0xBB, 0xBB}, {0x88, 0x88, 0xCC},
 {0x99, 0x99, 0xDD}, {0xAA, 0xAA, 0xEE}, {0xBB, 0xBB, 0xFF}, {0xCC, 0x88, 0xCC},
 {0xDD, 0x99, 0xDD}, {0xEE, 0xAA, 0xEE}, {0xFF, 0xBB, 0xFF}, {0x00, 0xCC, 0x88},
 {0x11, 0xDD, 0x99}, {0x22, 0xEE, 0xAA}, {0x33, 0xFF, 0xBB}, {0x44, 0xCC, 0x88},
 {0x55, 0xDD, 0x99}, {0x66, 0xEE, 0xAA}, {0x77, 0xFF, 0xBB}, {0x00, 0xCC, 0xCC},
 {0x11, 0xDD, 0xDD}, {0x22, 0xEE, 0xEE}, {0x33, 0xFF, 0xFF}, {0x44, 0xCC, 0xCC},
 {0x55, 0xDD, 0xDD}, {0x66, 0xEE, 0xEE}, {0x77, 0xFF, 0xFF}, {0x88, 0xCC, 0x88},
 {0x99, 0xDD, 0x99}, {0xAA, 0xEE, 0xAA}, {0xBB, 0xFF, 0xBB}, {0xCC, 0xCC, 0x88},
 {0xDD, 0xDD, 0x99}, {0xEE, 0xEE, 0xAA}, {0xFF, 0xFF, 0xBB}, {0x88, 0xCC, 0xCC},
 {0x99, 0xDD, 0xDD}, {0xAA, 0xEE, 0xEE}, {0xBB, 0xFF, 0xFF}, {0xCC, 0xCC, 0xCC},
 {0xDD, 0xDD, 0xDD}, {0xEE, 0xEE, 0xEE}, {0xFF, 0xFF, 0xFF} };

/****************************** Procedures. *********************************/

/**
 * JPEGDataSave
 *
 * Processes Data save messages, checking that the input is a JPEG,
 * allocating space to save the JPEG file into, and actually loading
 * the data.
 *
 * event - The wimp event block.
 */
void JPEGDataSave(int *event) {
  // Check that the input is a JPEG File
  if(event[MESSAGE_FILETYPE] != -1 && event[MESSAGE_FILETYPE] != 0xC85) {
    DBUG(fprintf(stderr, "JPEGDataSave: File is not a JPEG, or wimp$scrap file 0x%.3X\n", event[MESSAGE_FILETYPE]));
    return;
  }

  if(JPEGInfo) {
    DBUGF("JPEGDataSave: Already loading an object.\n");
    return;
  }

  // Allocate space for the file (Looking up the size if needed)
  DBUG(fprintf(stderr, "JPEGDataSave: Filesize = %d\n", event[MESSAGE_FILESIZE]));

  // Send an acknowledgement message
  event[MESSAGE_ACTION] = MESSAGE_DATASAVEACK;
  event[MESSAGE_SIZE]   = 60;

  // Copy your_ref field
  event[MESSAGE_YOUR_REF] = event[MESSAGE_MY_REF];

  // Set size to -1
  event[MESSAGE_FILESIZE] = -1;
  strcpy((char *)&event[MESSAGE_FILENAME], "<Wimp$Scrap>");

  // Send message
  _swix(Wimp_SendMessage, _INR(0, 2), 17, event, event[MESSAGE_SENDER]);

}

/**
 * JPEGDataLoad
 *
 * Processes Dataload messages, checking that the input is a JPEG,
 * allocating space to save the JPEG file into, and actually loading
 * the data.
 *
 * event - The wimp event block.
 */
void JPEGDataLoad(int *event) {
  // Check that the input is a JPEG File
  if(event[MESSAGE_FILETYPE] != -1 && event[MESSAGE_FILETYPE] != 0xC85) {
    DBUG(fprintf(stderr, "JPEGDataLoad: File is not a JPEG, or wimp$scrap file. 0x%.3X\n", event[MESSAGE_FILETYPE]));
    return;
  }

  if(JPEGInfo) {
    DBUGF("JPEGDataLoad: Already loading an object.\n");
    return;
  }

  // Allocate space for the file (Looking up the size if needed)
  DBUG(fprintf(stderr, "JPEGDataLoad: Filesize = %d\n", event[MESSAGE_FILESIZE]));

  // Load the file (Allocates memory for the JPEG file
  if((JPEGInfo = JV_DecodeCreate((char *)&event[MESSAGE_FILENAME])) == NULL) {
    DBUG(fprintf(stderr, "JPEGDataLoad: Failed to load file\n"));
    return;
  }

  // Send acknowledgement message
  event[MESSAGE_ACTION] = MESSAGE_DATALOADACK;

  // Copy your_ref field
  event[MESSAGE_YOUR_REF] = event[MESSAGE_MY_REF];

  // Send message
  _swix(Wimp_SendMessage, _INR(0, 2), 17, event, event[MESSAGE_SENDER]);

}

/**
 * JPEGDataOpen
 *
 * Processes Dataopen messages, checking that the input is a JPEG,
 * allocating space to save the JPEG file into, and actually loading
 * the data.
 *
 * event - The wimp event block.
 */
void JPEGDataOpen(int *event) {
  // Check that the input is a JPEG File
  if(event[MESSAGE_FILETYPE] != 0xC85) {
    DBUG(fprintf(stderr, "JPEGDataLoad: File is not a JPEG file. 0x%.3X\n", event[MESSAGE_FILETYPE]));
    return;
  }

  if(JPEGInfo) {
    DBUGF("JPEGDataLoad: Already loading an object.\n");
    return;
  }

  // Allocate space for the file (Looking up the size if needed)
  DBUG(fprintf(stderr, "JPEGDataLoad: Filesize = %d\n", event[MESSAGE_FILESIZE]));

  // Load the file (Allocates memory for the JPEG file
  if((JPEGInfo = JV_DecodeCreate((char *)&event[MESSAGE_FILENAME])) == NULL) {
    DBUG(fprintf(stderr, "JPEGDataLoad: Failed to load file\n"));
    return;
  }

  // Send acknowledgement message
  event[MESSAGE_ACTION] = MESSAGE_DATALOADACK;

  // Copy your_ref field
  event[MESSAGE_YOUR_REF] = event[MESSAGE_MY_REF];

  // Send message
  _swix(Wimp_SendMessage, _INR(0, 2), 17, event, event[MESSAGE_SENDER]);

}

/**
 * JPEGCreateSprite
 *
 * Creates a sprite of the appropiate size, depth and adds a palette if
 * needed.
 *
 * width   - The width of the sprite
 * height  - The height of the sprite
 * l2bpp   - Depth of the sprite
 * palette - TRUE If a palette should be added
 *
 * Returns TRUE if successful, FALSE otherwise
 */
bool JPEGCreateSprite(int width, int height, int l2bpp, bool palette) {
  int bufferSize, mode;
  sprite_areainfo_ptr areainfo;
  sprite_header_ptr   header;

  DBUGF("JPEGCreateSprite: Creating sprite\n");

  if(width == 0 || height == 0) {
    DBUGF("JPEGCreateSprite: Height or width was zero\n");
    return false;
  }

  // Work out size of buffer
  bufferSize = ((((width  << l2bpp) + 31) & ~31) >> 3) * height;

  // Add room for palette
  if(l2bpp <= 3 && palette)
    bufferSize += 8 << (1 << l2bpp);

  // Add room for the headers
  bufferSize += sizeof(sprite_areainfo_struct) + sizeof(sprite_header_struct);

  DBUG(fprintf(stderr, "JPEGCreateSprite: Buffer size = %d\n", bufferSize));

  // Allocate the memory
  if((spriteArea = JPEGViewAllocateMemory(bufferSize)) == NULL) {
    DBUGF("JPEGCreateSprite: Failed to allocate memory\n");
    return false;
  }

  // Mark as valid memory for MEMCHECK
#ifdef USE_MEMCHECK
  MemCheck_RegisterMiscBlock((void *)spriteArea, bufferSize);
#endif

  // Set up structure pointer
  areainfo = (sprite_areainfo_ptr)spriteArea;

  areainfo->areasize   = bufferSize;
  areainfo->firstoffset = 16;

  // Create the sprite area
  if(_swix(OS_SpriteOp, _INR(0, 1), 0x209, areainfo)) {
    DBUGF("JPEGCreateSprite: Failed to initialise sprite area\n");
    return false;
  }

  // Create sprite
  switch(l2bpp) {
    case 5:
    case 4:
      mode = 1 | (90 << 1) | (90 << 14) | ((l2bpp+1) << 27);
    break;

    case 3:
      mode = 28;
    break;
  }

  if(_swix(OS_SpriteOp, _INR(0, 6), 0x10F, spriteArea, JPEGViewName, 0, width, height, mode)) {
    DBUGF("JPEGCreateSprite: Failed to create the sprite.\n");
    return false;
  }

  // Set up pointer to sprite
  header = (sprite_header_ptr)(spriteArea + sizeof(sprite_areainfo_struct));

  // Do we need to add a palette
  if(l2bpp <=3 && palette) {
    if(_swix(OS_SpriteOp, _INR(0, 3), 0x225, spriteArea, header, 0x80000000)) {
      DBUGF("JPEGCreateSprite: Failed to add palette to the sprite\n");
      return false;
    }

    // Set pointer to palette
    JPEGInfo->palettePointer = (int *)(spriteArea + sizeof(sprite_areainfo_struct) + sizeof(sprite_header_struct));
  }

  // Set pointer to sprite data
  DBUG(fprintf(stderr, "Header = %p, offset = 0x%X\n", header,header->imageoffset));
  JPEGInfo->spriteData = ((char *)header) + header->imageoffset;
  DBUG(fprintf(stderr, "Sprite data = %p\n", JPEGInfo->spriteData));

  // Call wimp stuff
  wimpModeChange();

  // Return true;
  return true;
}

/**
 * JPEGViewAllocateMemory
 *
 * Allocates the memory required to hold the output sprite.
 *
 * size - The size in bytes to allocate
 *
 * Returns - Pointer to memory
 *           NULL if failed
 */
char *JPEGViewAllocateMemory(CARD32 size){
  /* Allocates memory for the screen buffer */
  char *result;

  // Create a dynamic area
  if(dynamicArea > 0) {
    JPEGViewFreeMemory();
  }

  // Create dynamic area
  if(_swix(OS_DynamicArea, _INR(0, 8) | _OUT(1) | _OUT(3), 0, -1, size, -1, (1 << 7), size,
                           NULL, NULL, JPEGViewName, &dynamicArea, &result)) {
    DBUGF("JPEGViewAllocateMemory: Failed to create DA\n");
    return NULL;
  }

  // Return pointer
  return result;
}


/**
 * JPEGViewFreeMemory
 *
 * Frees the dynamic area allocated to hold the sprite.
 */
void JPEGViewFreeMemory(void) {
  if(dynamicArea != 0) {
    _swix(OS_DynamicArea, _INR(0, 1), 1, dynamicArea);
    dynamicArea = 0;
  }
}












/*------------------------------------------------------------------*/

/**
 *  Inside oregano threads are broken up into Create, Work and Destroy sections
 */



/**
 * JV_DecodeCreate
 *
 * Creates the work structure, sets up any info needed.
 */
JPEGInfo_t *JV_DecodeCreate(const char *filepath) {
  JPEGInfo_t *JPEGInfo;

  if(filepath == NULL || *filepath == '\0') {
    DBUGF("JV_DecodeCreate: File path NULL or empty string\n");
    return NULL;
  }

  // Create the JPEG info structure
  if((JPEGInfo = calloc(1, sizeof(JPEGInfo_t))) == NULL) {
    DBUGF("JV_DecodeCreate: Failed to allocate space for the JPEGInfo structure.\n");
    return NULL;
  }

  // Copy the filepath
  JPEGInfo->filepath = calloc(1, strlen(filepath)+1);
  strcpy(JPEGInfo->filepath, filepath);

  // Set status to create
  JPEGInfo->status = JV_STATUS_CREATE;

  // Set delay time
  pollDelay = 1;

  // Return JPEGInfo
  return JPEGInfo;
}


/**
 * JV_DecodeWork
 *
 * Does all the work, reading in data, outputting info etc.
 */
void JV_DecodeWork(JPEGInfo_t *JPEGInfo) {
  // The cinfo pointer is used a lot, so set up a shortcut..
  struct jpeg_decompress_struct *cinfo = &JPEGInfo->cinfo;

  DBUGF("JV_DecodeWork\n");

  // Check what we should be doing
  switch(JPEGInfo->status) {
    // Create a new sprite, JPEG decoder etc.
    case JV_STATUS_CREATE:
    {
      DBUGF("JV_DecodeWork: Creating new JPEG object\n");

      // Work out the current depth of this mode
      JPEGInfo->log2bpp = _swi(OS_ReadModeVariable, _INR(0, 1) | _RETURN(2), -1, 9);

      if(JPEGInfo->log2bpp < 3)
        JPEGInfo->log2bpp = 3;

      // Allocate a decompression object
      cinfo->err = jpeg_std_error(&JPEGInfo->jerr);
      jpeg_create_decompress(cinfo);

      // Specifiy data source
      JPEGsrcmanGetManager(cinfo, JPEGInfo->filepath);

      // Waiting for header
      JPEGInfo->status = JV_STATUS_HEADER;
    }
    break;

    // Processing header
    case JV_STATUS_HEADER:
      DBUGF("JV_DecodeWork: Reading in headers\n");

      // Fill up the buffers
      JPEGsrcmanGetInput(JPEGInfo);

      // Read in the headers
      if(JPEG_SUSPENDED == jpeg_read_header(cinfo, TRUE)) {
        DBUGF("JV_DecodeWork: Not enough data to process headers\n");
        return;
      }

      // If its a grey scale image, then insure we are at 8 BPP
      if(cinfo->out_color_space == JCS_GRAYSCALE)
        JPEGInfo->log2bpp = 3;

      // Set up colour map routines
      if(JPEGInfo->log2bpp == 3) {
        int i;

        DBUGF("JV_DecodeWork: Setting up colourmap\n");

        // Setup colourmap
        cinfo->quantize_colors = TRUE;

        // Only generate the default palette for coloured images
        if(cinfo->out_color_space != JCS_GRAYSCALE && !jpeg_has_multiple_scans(cinfo)) {
          cinfo->colormap = (*cinfo->mem->alloc_sarray)
                            ((j_common_ptr) cinfo, JPOOL_IMAGE, MAXJSAMPLE + 1, 3);

          cinfo->actual_number_of_colors = 256;

          // Fill in colourmap
          i = 256;
          while(i--) {
            cinfo->colormap[0][i] = defaultPalette[i][0];
            cinfo->colormap[1][i] = defaultPalette[i][1];
            cinfo->colormap[2][i] = defaultPalette[i][2];
          }
        }
      }

      // If it a progessive JPEG?
      if(jpeg_has_multiple_scans(cinfo)) {
        DBUGF("JV_DecodeWork: JPEG is a progressive\n");
        cinfo->buffered_image = TRUE;

        if(cinfo->out_color_space != JCS_GRAYSCALE) {
          cinfo->two_pass_quantize = FALSE;
          cinfo->enable_external_quant = TRUE;
        }
        else
          cinfo->enable_2pass_quant = TRUE;
      }

      // Go and start up the decompressor
      JPEGInfo->status = JV_STATUS_START;
    // break;


    // Start decompressing the stream
    case JV_STATUS_START:
      DBUGF("JV_DecodeWork: Getting ready to start the decompressor\n");

      // Read in data
      JPEGsrcmanGetInput(JPEGInfo);

      // Start up the decompressor
      if(FALSE == jpeg_start_decompress(cinfo)) {
        DBUGF("JV_DecodeWork: Ran out of data when starting the decompressor\n");
        return;
      }

      DBUGF("JV_DecodeWork: Allocating buffer\n");
      // Allocates the buffer
      JPEGInfo->buffer = (*cinfo->mem->alloc_sarray)
                         ((j_common_ptr) cinfo, JPOOL_IMAGE,
                         cinfo->output_width * cinfo->output_components,
                         cinfo->rec_outbuf_height);

      // Create sprite
      DBUG(fprintf(stderr, "JV_DecodeWork: Width = %d, height = %d\n", cinfo->output_width, cinfo->output_height));

      if(false == JPEGCreateSprite(cinfo->output_width, cinfo->output_height,
                                   JPEGInfo->log2bpp, cinfo->quantize_colors)) {
        DBUGF("JV_DecodeWork: Failed to create sprite\n");
        JPEGInfo->status = JV_STATUS_FINISHED;
        exit(0);
        return;
      }

      // Set width
      JPEGInfo->imageWidth = cinfo->output_width;
      if(cinfo->output_width % 4 != 0)
        JPEGInfo->imageWidth += 4 - (cinfo->output_width % 4);

      // Copy across palette if needed
      if(cinfo->quantize_colors && cinfo->colormap) {
        int i, j;

        for(i = 0, j = 0; i < 512; i += 2, j++) {
          if((j) < cinfo->desired_number_of_colors ) {
            if(cinfo->out_color_space == JCS_GRAYSCALE) {
              JPEGInfo->palettePointer[i] = (cinfo->colormap[0][j] << 8)  |
                                            (cinfo->colormap[0][j] << 16) |
                                            (cinfo->colormap[0][j] << 24);
            }
            else {
              JPEGInfo->palettePointer[i] = (cinfo->colormap[0][j] << 8)  |
                                            (cinfo->colormap[1][j] << 16) |
                                            (cinfo->colormap[2][j] << 24);
            }

            JPEGInfo->palettePointer[i+1] = JPEGInfo->palettePointer[i];
          }
        }
        wimpModeChange();
      }


      // If it a progessive JPEG?
      if(jpeg_has_multiple_scans(cinfo)) {
        JPEGInfo->status = JV_STATUS_PROGRESSIVE;
        JPEGInfo->progressive = JV_PROGRESSIVE_START;
      }
      else {
        JPEGInfo->status = JV_STATUS_STANDARD_JPEG;
      }

      // Reset base pointer, and window size
      displaySetupPointers();

      // Update window display
      if(statusFlags & FLAG_MAIN_OPEN)
        displayUpdateMain(-1, -1);
      else {
        wimpOpenWindow(handles[MAIN_WINDOW], WINDOW_ON_TOP | WINDOW_CENTERED);
        statusFlags |= FLAG_MAIN_OPEN;
      }

      JPEGInfo->outptr = JPEGInfo->spriteData;

    break;

    // Handle 'normal' JPEGs
    case JV_STATUS_STANDARD_JPEG:
    {
      int min = cinfo->output_scanline;
      DBUGF("JV_DecodeWork: Handling standard JPEG\n");

      // Common routine to read in scanlines?  Shared with progressive process
      JV_DecodeLines(JPEGInfo);

      // Update window display
      if(statusFlags & FLAG_MAIN_OPEN)
        displayUpdateMain(min, cinfo->output_scanline);
      else {
        wimpOpenWindow(handles[MAIN_WINDOW], WINDOW_ON_TOP | WINDOW_CENTERED);
        statusFlags |= FLAG_MAIN_OPEN;
      }
      DBUGF("JV_DecodeWork: FINISHED STANDARD JPEG\n");
    }
    break;

    // Handle progressive JPEGs
    case JV_STATUS_PROGRESSIVE:
      DBUGF("JV_DecodeWork: Handling progressive JPEG\n");

      // What stage are we at?
      switch(JPEGInfo->progressive) {
        // Starting new scan
        case JV_PROGRESSIVE_START:
          DBUGF("JV_DecodeWork: Starting scan of progressive JPEG\n");

          // Fill buffers
          JPEGsrcmanGetInput(JPEGInfo);

          // If the image is downloaded, eat data until EOI
          if(JPEGInfo->flags & JV_FLAGS_COMPLETED && jpeg_consume_input(cinfo) != JPEG_REACHED_EOI) {
            int count = _swi(OS_ReadMonotonicTime, _RETURN(0)) + JV_PROCESS_TIME;
            DBUGF("JV_DecodeWork: File complete, eatting data\n");
            while(jpeg_consume_input(cinfo) != JPEG_REACHED_EOI &&
                  count > _swi(OS_ReadMonotonicTime, _RETURN(0))) {
              // Fill buffers
              JPEGsrcmanGetInput(JPEGInfo);
            }

            // All ready in?
            if(jpeg_consume_input(cinfo) != JPEG_REACHED_EOI)
              return;
          }
          else {
            // Only process every third frame
            if((cinfo->input_scan_number % 3) != 1 && !JPEGInfo->flags & JV_FLAGS_COMPLETED) {
              JPEGInfo->progressive = JV_PROGRESSIVE_SKIP_SCAN;
              return;
            }
          }


          // If image completed start final scan
          if(jpeg_input_complete(cinfo)) {
            int i;

            DBUGF("JV_DecodeWork: Image complete, doing HQ scan\n");
            cinfo->dct_method          = JDCT_ISLOW;
            cinfo->dither_mode         = JDITHER_FS;
            cinfo->do_fancy_upsampling = TRUE;
            cinfo->two_pass_quantize   = FALSE;
            JPEGInfo->flags |= JV_FLAGS_FINAL_SCAN;

            // Select our prefered colourmap
            if(cinfo->quantize_colors && cinfo->colormap) {
              if(cinfo->out_color_space != JCS_GRAYSCALE) {
                cinfo->colormap = (*cinfo->mem->alloc_sarray)
                                  ((j_common_ptr) cinfo, JPOOL_IMAGE, MAXJSAMPLE + 1, 3);

                cinfo->actual_number_of_colors = 256;

                // Fill in colourmap
                i = 256;
                while(i--) {
                  cinfo->colormap[0][i] = defaultPalette[i][0];
                  cinfo->colormap[1][i] = defaultPalette[i][1];
                  cinfo->colormap[2][i] = defaultPalette[i][2];
                }

                jpeg_new_colormap(cinfo);
              }
            }
          }
          else {
            DBUGF("JV_DecodeWork: Doing low quality scan\n");
            cinfo->dct_method          = JDCT_IFAST;
            cinfo->dither_mode         = JDITHER_ORDERED;
            cinfo->do_fancy_upsampling = FALSE;
          }

          // Reset base pointer
          JPEGInfo->outptr = JPEGInfo->spriteData;

          // Start decode
          jpeg_start_output(cinfo, cinfo->input_scan_number);

          // Change state
          JPEGInfo->progressive = JV_PROGRESSIVE_PROCESS;

          // Copy any generated palettes
          if(cinfo->quantize_colors && cinfo->colormap) {
            int i, j;

            for(i = 0, j = 0; i < 512; i += 2, j++) {
              if((j) < cinfo->desired_number_of_colors) {
                if(cinfo->out_color_space == JCS_GRAYSCALE) {
                  JPEGInfo->palettePointer[i] = (cinfo->colormap[0][j] << 8)  |
                                                (cinfo->colormap[0][j] << 16) |
                                                (cinfo->colormap[0][j] << 24);
                }
                else {
                  JPEGInfo->palettePointer[i] = (cinfo->colormap[0][j] << 8)  |
                                                (cinfo->colormap[1][j] << 16) |
                                                (cinfo->colormap[2][j] << 24);
                }

                JPEGInfo->palettePointer[i+1] = JPEGInfo->palettePointer[i];
              }
            }

            // Re generate palette
            wimpModeChange();
          }

        break;

        // Skipping a scan
        case JV_PROGRESSIVE_SKIP_SCAN:
        {
          // Consume data
          int count = _swi(OS_ReadMonotonicTime, _RETURN(0)) + JV_PROCESS_TIME;
          int result;

          while((result = jpeg_consume_input(cinfo)) != JPEG_REACHED_SOS &&
                 result != JPEG_REACHED_EOI && count > _swi(OS_ReadMonotonicTime, _RETURN(0))) {
                // Fill buffers
                JPEGsrcmanGetInput(JPEGInfo);
          }

          if(result == JPEG_REACHED_SOS || result == JPEG_REACHED_EOI)
            JPEGInfo->progressive = JV_PROGRESSIVE_START;
        }
        break;

        // Processing data for a scan
        case JV_PROGRESSIVE_PROCESS:
        {
          int min = cinfo->output_scanline;
          DBUGF("JV_DecodeWork: Processing progressive JPEG\n");
          JV_DecodeLines(JPEGInfo);

          // Update window display
          if(statusFlags & FLAG_MAIN_OPEN) {
            DBUGF("Updating display\n");
            displayUpdateMain(min, cinfo->output_scanline);
          }
        }
        break;

        // Finished scan
        case JV_PROGRESSIVE_FINISH:
          DBUGF("JV_DecodeWork: Finished scan of progressive JPEG\n");

          // We've finished this output
          jpeg_finish_output(cinfo);

          // Was that the final scan
          if(JPEGInfo->flags & JV_FLAGS_FINAL_SCAN) {
            JPEGInfo->status = JV_STATUS_FINISHED;
          }
          else {
            // More scans to perform
            JPEGInfo->progressive = JV_PROGRESSIVE_START;
          }
        break;

        // Unknown state
        default:DBUG(fprintf(stderr, "JV_DecodeWork: Unknown progressive state %d\n", JPEGInfo->progressive));
        break;
      }
    break;

    // Got some data to skip
    case JV_STATUS_SKIPPING_DATA:
      DBUGF("JV_DecodeWork: Skipping data\n");

      // Nothing to do, all handled by buffer filling
    break;

    // Finished
    case JV_STATUS_FINISHED:
      DBUGF("JV_DecodeWork: Finished. Destroying JPEG object\n");

      // Fill buffers
      JPEGsrcmanGetInput(JPEGInfo);

      // Close down
      if(FALSE == jpeg_finish_decompress(cinfo)) {
        DBUGF("JV_DecodeWork: jpeg_finish_decompress returned FALSE.. Suspending\n");
        return;
      }

      // Destroy the source manager
      JPEGsrcmanDestroyManager(cinfo->src);

      // Release the object
      jpeg_destroy_decompress(cinfo);

      // Destroy the object
      JV_DecodeDestroy(JPEGInfo);

      // Shrink heap
#ifdef USE_HEAP
      Heap_Shrink(NULL);
#endif
    break;

    // Unknown state...
    default:DBUG(fprintf(stderr, "JV_DecodeWork: Unknown work stage %d\n", JPEGInfo->status));
    break;
  }
}

/**
 * JV_DecodeDestroy
 *
 * Does all the tidying up.
 */
void JV_DecodeDestroy(JPEGInfo_t *data) {

  // Destroy filepath
  if(data->filepath) free(data->filepath);

  // Destory object
  free(data);
  JPEGInfo = NULL;

  // Shrink the heap
#ifdef USE_HEAP
  Heap_Shrink(NULL);
#endif

  // Reset poll delay
  pollDelay = WIMP_POLL_DELAY;
}

/**
 * JV_DecodeLines
 *
 * Decodes the next few scan lines of the JPEG
 */
static void JV_DecodeLines(JPEGInfo_t *JPEGInfo) {
  // Set the timeout
  int count = _swi(OS_ReadMonotonicTime, _RETURN(0)) + JV_PROCESS_TIME;
  int lines, i;
  JSAMPROW inptr;
  JDIMENSION col;

  DBUGF("JV_DecodeLines: Decoding data\n");

  // Fill the buffers
  JPEGsrcmanGetInput(JPEGInfo);

  DBUG(fprintf(stderr, "JPEGInfo->outptr = %p, image width = %d\n", JPEGInfo->outptr, JPEGInfo->imageWidth));

#ifdef USE_MEMCHECK
  MemCheck_SetChecking(0, 0);
#endif

  // Is there anything to do?
  if(JPEGInfo->cinfo.output_scanline < JPEGInfo->cinfo.output_height) {
    // Process scan lines until the timer runs out, or the image/scan finishes
    while(JPEGInfo->cinfo.output_scanline < JPEGInfo->cinfo.output_height &&
          count > _swi(OS_ReadMonotonicTime, _RETURN(0)))
    {
      // Read in lines
      if((lines = jpeg_read_scanlines(&JPEGInfo->cinfo, JPEGInfo->buffer, JPEGInfo->cinfo.rec_outbuf_height)) == 0) {
        DBUGF("JV_DecodeLines: jpeg_read_scanlines returned 0... Suspending\n");
        return;
      }

      // Process the buffer and output
      for(i = 0; i < lines; i++) {
        // Set up pointer to start of this line
        inptr = JPEGInfo->buffer[i];

        // Check the BPP and do the necessary work
        switch(JPEGInfo->log2bpp) {
          case 3:
            memcpy(JPEGInfo->outptr, inptr, JPEGInfo->cinfo.output_width);
            JPEGInfo->outptr += JPEGInfo->cinfo.output_width;

            // Zero set pad bytes
            for(col = JPEGInfo->cinfo.output_width; col < JPEGInfo->imageWidth; col++)
              *JPEGInfo->outptr++ = 0;
          break;

          case 4:
          {
            short *pixel;

            for(col = 0; col < JPEGInfo->cinfo.output_width; col++) {
              pixel = (short *)JPEGInfo->outptr;
              *pixel = (inptr[0] >> 3) | ((inptr[1] >> 3) << 5) | ((inptr[2] >> 3) << 10);
              inptr += 3;
              JPEGInfo->outptr += 2;
            }

            if(((JPEGInfo->imageWidth - JPEGInfo->cinfo.output_width)% 2) != 0) {
              JPEGInfo->outptr += 2;
            }
          }
          break;

          case 5:
            for(col = 0; col < JPEGInfo->cinfo.output_width; col++) {
              *JPEGInfo->outptr++ = *inptr++;
              *JPEGInfo->outptr++ = *inptr++;
              *JPEGInfo->outptr++ = *inptr++;
              *JPEGInfo->outptr++ = 0;
            }
          break;

          default:
            DBUG(fprintf(stderr, "JV_DecodeLines: Unknown BPP %d\n", JPEGInfo->log2bpp));
            JPEGInfo->status = JV_STATUS_FINISHED;
            return;
          break;
        }
      }
    }
  }
  else {
    DBUGF("JV_DecodeLines; End of scan/image\n");
    // Finshed this scan/image.  So update status
    if(JPEGInfo->status == JV_STATUS_PROGRESSIVE) {
      // Its a progressive JPEG
      JPEGInfo->progressive = JV_PROGRESSIVE_FINISH;
    }
    else
      JPEGInfo->status = JV_STATUS_FINISHED;
  }

#ifdef USE_MEMCHECK
  MemCheck_SetChecking(1, 1);
#endif

  DBUGF("JV_DecodeLines Finished decoding data\n");
}
