#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <zlib.h>

#include "libdeskbar/log.h"

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr)   ((png_ptr)->jmpbuf)
#endif

static png_structp png_ptr = NULL;
static png_infop info_ptr = NULL;

png_uint_32 width, height;

int bit_depth, color_type;
unsigned char	*image_data = NULL;

void
db_lpng_version (void)
{
	db_log_mesg ("Using zlib %s and libpng %s\n",
							 ZLIB_VERSION, PNG_LIBPNG_VER_STRING);
}

int
db_lpng_check_sig (FILE *file, 
									 unsigned long *w, 
									 unsigned long *h)
{
	unsigned char sig[8];

	fread (sig, 1, 8, file);
		
	if (!png_check_sig (sig, 8))
		return (1);   /* bad signature */

	/* could pass pointers to user-defined error handlers instead of NULLs: */
	png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		
	if (!png_ptr)
		return (4);   /* out of memory */

	info_ptr = png_create_info_struct (png_ptr);
		
	if (!info_ptr)
		{
			png_destroy_read_struct (&png_ptr, NULL, NULL);
		
			return (4);   /* out of memory */
		}

	if (setjmp (png_jmpbuf (png_ptr)))
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

			return (2);
    }
			
		png_init_io (png_ptr, file);
		png_set_sig_bytes (png_ptr, 8);  /* we already read the 8 signature bytes */

		png_read_info (png_ptr, info_ptr);  /* read all PNG info up to image data */
    png_get_IHDR (png_ptr, info_ptr, &width, 
			&height, &bit_depth, &color_type,
      NULL, NULL, NULL);
			
    *w = width;
    *h = height;
		
    return (0);
}

db_lpng_load (double display_exponent,
							int *channels,
							unsigned long *bytes)
{
	double gamma;
	
	png_uint_32 i, rowbytes;
	png_bytepp row_pointers = NULL;

	if (setjmp (png_jmpbuf (png_ptr))) {
		png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		
		return (NULL);
	}

	/* Check image palette */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	/* Skip gamma corrections? */
	if (png_get_gAMA (png_ptr, info_ptr, &gamma))
		png_set_gamma (png_ptr, display_exponent, gamma);
		
	/* Get row, channels and alloc the image */
	png_read_update_info (png_ptr, info_ptr);

	rowbytes = png_get_rowbytes (png_ptr, info_ptr);

	*bytes			= rowbytes;
	*channels		= (int) png_get_channels (png_ptr, info_ptr);
	image_data	= (unsigned char *) malloc (rowbytes * height);
		
	if (!image_data)
		{
			png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
			
			return (NULL);
		}
		
	row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep));
	
	if (!row_pointers)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			
			free(image_data);
			image_data = NULL;
			
			return (NULL);
		}

	db_log_debug ("Channels %d, Bytes %ld, Height %ld\n",
								*channels, rowbytes, height);

	for (i = 0;  i < height;  ++i)
		row_pointers[i] = image_data + (i * rowbytes);

	png_read_image (png_ptr, row_pointers);

	free(row_pointers);
	row_pointers = NULL;

	/* Could be omitted.. */
	png_read_end (png_ptr, NULL);

	return (image_data);
}

void 
db_lpng_cleanup (int clean)
{
	if (clean && image_data)
		{
			free (image_data);
		
			image_data = NULL;
		}

	if (png_ptr && info_ptr)
		{
			png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		
			png_ptr		= NULL;
			info_ptr	= NULL;
	}
}
