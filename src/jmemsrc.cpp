#if HAVE_JPEG_H

/*
 * jmemsrc.c
 *
 * This file contains a decompression data source which takes a
 * memory region as source.
 * The only thing these routines really do is tell the library where the
 * data is on construction (jpeg_mem_src()). Everything else is there
 * for error checking purposes.
 * Adapted from jdatasrc.c 9/96 by Ulrich von Zadow.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */

#include <stdio.h>

#include "jmemsrc.hpp"

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

void init_source (j_decompress_ptr cinfo)
{
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * If this procedure gets called, we have a buffer overrun condition -
 * there is not enough data in the buffer to satisfy the decoder.
 * The procedure just generates a warning and feeds the decoder a fake
 * JPEG_EOI marker.
 */

boolean fill_input_buffer (j_decompress_ptr cinfo)
{
  struct jpeg_source_mgr * src = cinfo->src;
  static JOCTET FakeEOI[] = { 0xFF, JPEG_EOI };

  /* Generate warning */
  WARNMS(cinfo, JWRN_JPEG_EOF);
  /* Insert a fake EOI marker */
  src->next_input_byte = FakeEOI;
  src->bytes_in_buffer = 2;

  return TRUE;
}


void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  struct jpeg_source_mgr * src = cinfo->src;
  if(num_bytes >= (long)src->bytes_in_buffer)
  {
    fill_input_buffer(cinfo);
    return;
  }

  src->bytes_in_buffer -= num_bytes;
  src->next_input_byte += num_bytes;
}


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

void term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}


/*
 * Prepare for input. This routine tells the jpeg library where to find
 * the data & sets up the function pointers the library needs.
 */

void jpeg_mem_src (
              j_decompress_ptr cinfo,
              JOCTET * pData,
              int FileSize
             )
{
  struct jpeg_source_mgr * src;

  if (cinfo->src == NULL)
  {   /* first time for this JPEG object? */
      cinfo->src = (struct jpeg_source_mgr *)
        (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                            sizeof(struct jpeg_source_mgr));
  }

  src = cinfo->src;

  /* Set up function pointers */
  src->init_source = init_source;
  src->fill_input_buffer = fill_input_buffer;
  src->skip_input_data = skip_input_data;
  src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->term_source = term_source;

  /* Set up data pointer */
  src->bytes_in_buffer = FileSize;
  src->next_input_byte = pData;
}
/*
/--------------------------------------------------------------------
|
|      $Log: not supported by cvs2svn $
|      Revision 1.4  2004/09/11 12:41:34  uzadow
|      removed plstdpch.h
|
|      Revision 1.3  2001/09/16 20:57:17  uzadow
|      Linux version name prefix changes
|
|      Revision 1.2  2001/09/15 21:02:44  uzadow
|      Cleaned up stdpch.h and config.h to make them internal headers.
|
|      Revision 1.1  2000/10/30 14:32:50  uzadow
|      Removed dependency on jinclude.h
|
|      Revision 1.5  2000/09/01 13:27:07  Administrator
|      Minor bugfixes
|
|
\--------------------------------------------------------------------
*/

#endif /* HAVE_JPEG_H */
