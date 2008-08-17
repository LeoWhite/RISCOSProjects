/******************************************************************************

Program:        JpegLib

File:           wrsprite.c

Function:       To convert to and from JPEG format files

Description:    Writes out images in Acorn Sprite format.

Author:         Leo White

Enviroment:     ANSI C

Revisions:      0.01 Thu 18th May 2000 - Creation

******************************************************************************/

// Includes
#include "cdjpeg.h"          // Common declarations for cjpeg/djpeg applications

// Only include the contents of this file if they are required
#ifdef SPRITE_SUPPORTED

/*
 * To support 12-bit JPEG data, we'd have to scale output down to 8 bits.
 * This is not yet implemented.
 */

#if BITS_IN_JSAMPLE != 8
  Sorry, this code only copes with 8-bit JSAMPLEs. /* deliberate syntax err */
#endif

// Defines
#define SPRITE_MAXNAME    12
#define BUFFER_SIZE       8192

// Private versions of the data destination object
typedef struct {
  struct djpeg_dest_struct pub; // Public fields

  char       *iobuffer;         // fwrites I/O buffer
  char       *bufferptr;
  size_t     buffer_width;      // Width of I/O buffer
  size_t     row_width;         // Width of a row
  size_t     free;
  JDIMENSION samples_per_row;   // JSAMPLES per output row
} sprite_dest_struct;



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
typedef sprite_dest_struct     *sprite_dest_ptr;
typedef sprite_areainfo_struct *sprite_areainfo_ptr;
typedef sprite_fileinfo_struct *sprite_fileinfo_ptr;
typedef sprite_header_struct   *sprite_header_ptr;


/*********************************  Procedures  ****************************/


/**
 * put_pixel_rows
 *
 * Writes a row of pixels to the output file
 *
 * Only used for > 8bit modes?
 *
 * cinfo         - Pointer to decompression info structure
 * dinfo         - Pointer to the destination structure
 * rows_supplied - The number of rows to be outputted
 */
METHODDEF(void)
put_pixel_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
               JDIMENSION rows_supplied)
{
  // Cast dinfo to our structure
  sprite_dest_ptr dest = (sprite_dest_ptr)dinfo;
  JSAMPROW inptr;
  char *outptr;
  JDIMENSION col;

  // Set up pointer in input data
  inptr = dest->pub.buffer[0];
  outptr = dest->bufferptr;

  // Copy values
  for(col = 0; col < cinfo->output_width; col++) {
    *dest->bufferptr++ = *inptr++;
    *dest->bufferptr++ = *inptr++;
    *dest->bufferptr++ = *inptr++;
    *dest->bufferptr++ = 0;
  }

  // Update free space accordingly
  dest->free -= dest->row_width;

  // If the buffer full?
  if(dest->free < dest->row_width) {
    // Output to the file
    (void) JFWRITE(dest->pub.output_file, dest->iobuffer, (dest->buffer_width - dest->free));
    dest->bufferptr = dest->iobuffer;
    dest->free = dest->buffer_width;
  }

}

/**
 * put_cmap_rows
 *
 * Writes a row of colourmapped or greyscale pixel rows to output.
 *
 * cinfo         - Pointer to decompression info structure
 * dinfo         - Pointer to the destination structure
 * rows_supplied - The number of rows to be outputted
 */
METHODDEF(void)
put_cmap_rows(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
               JDIMENSION rows_supplied)
{
  // Cast dinfo to our structure
  sprite_dest_ptr dest = (sprite_dest_ptr)dinfo;
  int i;

  // Copy the data (Note we dont
  memcpy(dest->bufferptr, dest->pub.buffer[0], cinfo->output_width);

  // Update pointer position, setting pad words to NULL
  dest->bufferptr += cinfo->output_width;
  for(i = cinfo->output_width; i < dest->row_width; i++)
    *dest->bufferptr++ = 0;

  // Update free space accordingly
  dest->free -= dest->row_width;

  // If the buffer full?
  if(dest->free < dest->row_width) {
    // Output to the file
    (void) JFWRITE(dest->pub.output_file, dest->iobuffer, (dest->buffer_width - dest->free));
    dest->bufferptr = dest->iobuffer;
    dest->free = dest->buffer_width;
  }
}


/**
 * start_output_sprite
 *
 * Generates the appropiate sprite header and writes it to disc.
 *
 * cinfo         - Pointer to decompression info structure
 * dinfo         - Pointer to the destination structure
 */
METHODDEF(void)
start_output_sprite(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  // Cast dinfo to our structure
  sprite_dest_ptr        dest   = (sprite_dest_ptr)dinfo;
  sprite_fileinfo_struct sprite_file;
  sprite_header_struct   sprite_header;

  // Set up common values
  sprite_file.numsprites  = 1;
  sprite_file.firstoffset = sizeof(sprite_areainfo_struct);
  sprite_file.freeoffset  = (dest->row_width * cinfo->output_height) + sizeof(sprite_areainfo_struct) +
                            sizeof(sprite_header_struct);

  sprite_header.offset_next = sprite_file.freeoffset - sprite_file.firstoffset;
  strncpy(sprite_header.name, "sprite", SPRITE_MAXNAME);
  sprite_header.width   = (dest->row_width / 4) - 1;
  sprite_header.height  = cinfo->output_height - 1;
  sprite_header.leftbit = 0;
  sprite_header.imageoffset = sizeof(sprite_header_struct);  // Straight after header
  sprite_header.maskoffset  = sizeof(sprite_header_struct);  // No mask

  // Check what depth the image is to be outputted in
  switch(cinfo->out_color_space) {
    // Is it a grey scale image?
    case JCS_GRAYSCALE:
      // Output sprite header for mode 28 with grey level palette
        // Output mode 28 sprite with no palette (Always use the default)
        sprite_header.rightbit   = ((dest->row_width - cinfo->output_width) * 8) - 1;
        sprite_header.screenmode = 28;
    break;

    // RGB output... 24 bit or 8 bit?
    case JCS_RGB:
      if(cinfo->quantize_colors) {
        // Output mode 28 sprite with no palette (Always use the default)
        sprite_header.rightbit   = ((dest->row_width - cinfo->output_width) * 8) - 1;
        sprite_header.screenmode = 28;
      }
      else {
        // 24 bit mode, output 'new mode' sprite
        sprite_header.rightbit   = 0x1F;
        sprite_header.screenmode = (6 << 27) | (90 << 14) | (90 << 1) | 1;
      }
    break;

    // Unsupported colour type
    default:ERREXIT(cinfo, JERR_SPRITE_COLORSPACE);
    break;
  }

  // Do we need to add a palette?
  if(cinfo->quantize_colors) {
    sprite_file.freeoffset += 2048;
    sprite_header.offset_next += 2048;
    sprite_header.imageoffset += 2048;
    sprite_header.maskoffset  += 2048;
  }

  // Output information
  (void)JFWRITE(dest->pub.output_file, &sprite_file,   sizeof(sprite_fileinfo_struct));
  (void)JFWRITE(dest->pub.output_file, &sprite_header, sizeof(sprite_header_struct));

  // Output palette details
  if(cinfo->quantize_colors) {
    // Output palette
    JSAMPROW colormap0 = cinfo->colormap[0];
    JSAMPROW colormap1 = cinfo->colormap[1];
    JSAMPROW colormap2 = cinfo->colormap[2];
    int i = 0, j = 0;
    int palette[512];

    for(i = 0, j = 0; i < 512; i += 2, j++) {
      if((j) < cinfo->desired_number_of_colors) {
        if(cinfo->out_color_space == JCS_GRAYSCALE)
          palette[i]   =   (colormap0[j] << 8) | (colormap0[j] << 16) | (colormap0[j] << 24);
        else
          palette[i]   =   (colormap0[j] << 8) | (colormap1[j] << 16) | (colormap2[j] << 24);

        palette[i+1] = palette[i];
      }
      else {
        palette[i] = 0;
        palette[i+1] = 0;
      }
    }

    // Write palette
    (void)JFWRITE(dest->pub.output_file, &palette, 512 * 4);
  }

}

/**
 * finish_output_sprite
 *
 * Flushes the file to ensure all data is written to disc.  Nothing else to do.
 *
 * cinfo         - Pointer to decompression info structure
 * dinfo         - Pointer to the destination structure
 */
METHODDEF(void)
finish_output_sprite(j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  sprite_dest_ptr        dest   = (sprite_dest_ptr)dinfo;

  // Any data left to send?
  if(dest->free != dest->buffer_width) {
    (void) JFWRITE(dest->pub.output_file, dest->iobuffer, (dest->buffer_width - dest->free));
  }

  // Flush the file
  fflush(dinfo->output_file);

  // Check for any errors.
  if(ferror(dinfo->output_file))
    ERREXIT(cinfo, JERR_FILE_WRITE);
}

/**
 * jinit_write_sprite
 *
 * Sets up all the pointers in the decompession structure to the various
 * functions for processing the output buffer.
 *
 * cinfo         - Pointer to decompression info structure;
 */
GLOBAL(djpeg_dest_ptr)
jinit_write_sprite(j_decompress_ptr cinfo)
{
  // Local variables
  sprite_dest_ptr dest;

  // Allocate space for the structure, and fill in functions pointers
  dest = (sprite_dest_ptr) (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                                                      SIZEOF(sprite_dest_struct));
  dest->pub.start_output  = start_output_sprite;
  dest->pub.finish_output = finish_output_sprite;

  // Work out the size of the image
  jpeg_calc_output_dimensions(cinfo);

  // Allocate the I/O buffers
  dest->samples_per_row = cinfo->output_width * cinfo->out_color_components;

  // Allocate temporay buffer
  dest->row_width    = cinfo->output_width;

  if(!cinfo->quantize_colors) {
    // 24 bit colour
    dest->row_width *= 4;
  }

  dest->row_width    += (dest->row_width % 4);

  if(BUFFER_SIZE > dest->row_width) {
    dest->buffer_width  = BUFFER_SIZE;
  }
  else
    dest->buffer_width  = dest->row_width;

  dest->iobuffer = (char *) (*cinfo->mem->alloc_small)
                            ((j_common_ptr) cinfo, JPOOL_IMAGE, dest->buffer_width);

  // Set the amount of buffer space free
  dest->free          = dest->buffer_width;
  dest->bufferptr     = dest->iobuffer;

  if (cinfo->quantize_colors) {
    /* When quantizing, we need an output buffer for colormap indexes
     * that's separate from the physical I/O buffer.  We also need a
     * separate buffer if pixel format translation must take place.
     */
    dest->pub.buffer = (*cinfo->mem->alloc_sarray) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                       cinfo->output_width * cinfo->output_components, (JDIMENSION) 1);
    dest->pub.buffer_height = 1;
    if (cinfo->out_color_space == JCS_GRAYSCALE)
      dest->pub.put_pixel_rows = put_cmap_rows;
    else
      dest->pub.put_pixel_rows = put_cmap_rows;
  } else {
    /* We will fwrite() directly from decompressor output buffer. */
    /* Synthesize a JSAMPARRAY pointer structure */
    /* Cast here implies near->far pointer conversion on PCs */
    dest->pub.buffer = (*cinfo->mem->alloc_sarray) ((j_common_ptr) cinfo, JPOOL_IMAGE,
                       cinfo->output_width * cinfo->output_components, (JDIMENSION) 1);
    dest->pub.buffer_height = 1;
    dest->pub.put_pixel_rows = put_pixel_rows;
  }

  // Return the pointer to the structure
  return (djpeg_dest_ptr) dest;
}

#endif // SPRITE_SUPPORTED




