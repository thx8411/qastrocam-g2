#ifndef _JMEMSRC_HPP_
#define _JMEMSRC_HPP_

extern "C"
{

// jmorecfg.h INT32 definition conflicts with QT.
// we use a trick to avoid this problem.
// B-F. C. 2010

#ifndef XMD_H                   /* X11/xmd.h correctly defines INT16 */
typedef short INT16;
#define XMD_H
#include "jpeglib.h"
#undef XMD_H
#else
#include "jpeglib.h"
#endif

#include "jerror.h"
}

void jpeg_mem_src (j_decompress_ptr cinfo,
              JOCTET * pData,
              int FileSize);

#endif

/*
/--------------------------------------------------------------------
|
|      $Log: not supported by cvs2svn $
|      Revision 1.2  2010/03/30 02:19:06  thx8411
|      trick to avoid INT32 def conflit
|
|      Revision 1.1  2010/02/11 04:09:09  thx8411
|      jpeg palette support almost done
|
|      Revision 1.7  2004/06/06 12:56:38  uzadow
|      Doxygenified documentation.
|
|      Revision 1.6  2000/10/30 21:45:04  uzadow
|      Added again after deleting the file by mistake.
|
|      Revision 1.4  2000/10/26 21:06:17  uzadow
|      Removed dependency on jpegint.h
|
|
\--------------------------------------------------------------------
*/
