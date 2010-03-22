/******************************************************************************

Program:        JPEGView

File:           JPEGdecode.h

Function:       Header file for processing of JPEG fils

Description:    Handles displaying output

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Mon 22nd May 2000 - Creation

******************************************************************************/

// Check if the file has already been included
#ifndef __JV_JPEGDECODE_H
# define __JV_JPEGDECODE_H
# undef  DECLARE
# undef  INIT
# ifdef  PROTOTYPES_INTERNAL
#  define DECLARE
#  define INIT(x)   =(x)
# else
#  define DECLARE extern
#  define INIT(x)
# endif

// Includes

/* Constants ---------------------------------------------------------------*/
#define JV_STATUS_CREATE          1
#define JV_STATUS_HEADER          2
#define JV_STATUS_START           3
#define JV_STATUS_STANDARD_JPEG   4
#define JV_STATUS_PROGRESSIVE     5
#define JV_STATUS_SKIPPING_DATA   6
#define JV_STATUS_FINISHED        7

#define JV_PROGRESSIVE_START      1
#define JV_PROGRESSIVE_SKIP_SCAN  2
#define JV_PROGRESSIVE_PROCESS    3
#define JV_PROGRESSIVE_FINISH     4

#define JV_PROCESS_TIME           100

#define JV_FLAGS_COMPLETED        (1 << 0)
#define JV_FLAGS_FINAL_SCAN       (1 << 1)

#define SPRITE_MAXNAME    12

/* Typedefines -------------------------------------------------------------*/
typedef struct JPEGInfo_s {
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr         jerr;


  // Current status
  int status;
  int progressive;
  int flags;

  // Info about the sprite, pointer to sprite data, palette, width and the depth
  char *spriteData;
  int *palettePointer;
  int  imageWidth;
  int log2bpp;

  // Pointer to current output position, and the location of the buffer
  char *outptr;
  JSAMPARRAY buffer;

  // File info
  char *filepath;
} JPEGInfo_t;

// Control block for sprite area
typedef struct {
    unsigned int areasize ;
    unsigned int numsprites ;
    unsigned int firstoffset ;
    unsigned int freeoffset ;
} sprite_areainfo_struct;

// When writing to file, don't include size
typedef struct {
    unsigned int numsprites ;
    unsigned int firstoffset ;
    unsigned int freeoffset ;
} sprite_fileinfo_struct;

// Control block for a sprite
typedef struct {
  int  offset_next;
  char name[SPRITE_MAXNAME];
  int  width;
  int  height;
  int  leftbit;
  int  rightbit;
  int  imageoffset;
  int  maskoffset;
  int  screenmode;
} sprite_header_struct;

// Pointer typedefines
typedef sprite_areainfo_struct *sprite_areainfo_ptr;
typedef sprite_fileinfo_struct *sprite_fileinfo_ptr;
typedef sprite_header_struct   *sprite_header_ptr;

/* Global Variables --------------------------------------------------------*/

/* Procedure Prototypes ----------------------------------------------------*/
DECLARE void JPEGDataSave(int *event);
DECLARE void JPEGDataLoad(int *event);
DECLARE void JPEGDataOpen(int *event);
DECLARE bool JPEGLoadFile(const char *filepath);
DECLARE char *JPEGViewAllocateMemory(CARD32 size);
DECLARE void JPEGViewFreeMemory(void);
DECLARE bool JPEGCreateSprite(int width, int height, int l2bpp, bool palette);
DECLARE FILE        *JPEGFile     INIT(NULL);
DECLARE JPEGInfo_t  *JPEGInfo;
DECLARE void JV_DecodeWork(JPEGInfo_t *JPEGInfo);
DECLARE void JPEGsrcmanGetManager(j_decompress_ptr cinfo, const char *filepath);
DECLARE void JPEGsrcmanDestroyManager(struct jpeg_source_mgr *data);
DECLARE JPEGInfo_t *JV_DecodeCreate(const char *filepath);
DECLARE void JV_DecodeDestroy(JPEGInfo_t *JPEGInfo);
DECLARE void JPEGsrcmanGetInput(JPEGInfo_t *JPEGInfo);

// End of include file
#undef PROTOTYPES_INTERNAL
#undef INIT
#undef DECLARE
#endif // __JV_JPEGDECODE_H
