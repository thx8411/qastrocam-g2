/******************************************************************
Qastrocam-g2
Copyright (C) 2009   Blaise-Florentin Collin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License v2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
*******************************************************************/

#ifndef _YUV_HPP_
#define _YUV_HPP_

// yuv444 conversion tools

//
// TO YUV444
//

// rgb24 to yuv444 planar
void rgb24_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// yuv422 to yuv444 planar
void yuv422_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// yuv420 planar to yuv444 planar
void yuv420_to_yuv444(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

//
// FROM YUV444
//

// yuv444 planar to rgb24
void yuv444_to_rgb24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

// yuv444 planar to bgr24
void yuv444_to_bgr24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

// swap rgb24 (upside down)
void rgb24_vertical_swap(int w, int h, unsigned char* data);

// yuv444 planar to rgb32
void yuv444_to_bgr32(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

// 4:4:4 planar yuv to 4:2:0 planar yuv
void yuv444_to_yuv420(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

#endif
