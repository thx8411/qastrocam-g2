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

// clip value between 0 and 255
unsigned char clip(double v);

unsigned char clip(int v);

//
// TO YUV444
//

// bgr32 to yuv444 planar
void bgr32_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// rgb24 to yuv444 planar
void rgb24_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// uyvy to yuv444 planar
void uyvy_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// yuyv to yuv444 planar
void yuyv_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// yuv420 planar to yuv444 planar
void yuv420_to_yuv444(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// YCbCr to yuv444
void ycbcr_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

// s505 (yyuv per lines) to yuv444
void s505_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV);

//
// TO Y only
//

// bgr32 to y planar
void bgr32_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

// rgb24 to y planar
void rgb24_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

// uyvy to y planar
void uyvy_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

// yuyv to y planar
void yuyv_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

// yuv420 planar to y planar
void yuv420_to_y(int w, int h, const unsigned char* srcY, unsigned char* dstY);

// YCbCr to y
void ycbcr_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

// s505 (yyuv per line) to y
void s505_to_y(int w, int h, const unsigned char* src, unsigned char* dstY);

//
// FROM YUV444
//

// yuv444 planar to rgb24
void yuv444_to_rgb24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

// yuv444 planar to bgr24
void yuv444_to_bgr24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

// yuv444 planar to rgb32
void yuv444_to_bgr32(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst);

//
// FROM Y ONLY
//

// y to yuyv
void y_to_yuyv(int w, int h, const unsigned char* src, unsigned char* dst);

// y to bgr24
void y_to_bgr24(int w, int h, const unsigned char* src, unsigned char* dst);

//
// TRANSFORMS
//

// swap 8 bit grey (upside down)
void grey_vertical_swap(int w, int h, unsigned char* data);

// swap rgb24 (upside down)
void rgb24_vertical_swap(int w, int h, unsigned char* data);

// 8 bits luminance plan substraction A=A-B*C
void lum_plan_sub(int w, int h,unsigned char* A, const unsigned char* B, double C);

// 8 bits color (U or V) plan substraction A=A-B*C
void color_plan_sub(int w, int h,unsigned char* A, const unsigned char* B, double C);

// 8 bits luminance plan division A=A*max/B
void lum_plan_div(int w, int h, int max, unsigned char* A, const unsigned char* B);

// 8 bits color (U or V) plan division A=A*max/B
void color_plan_div(int w, int h, int max, unsigned char* A, const unsigned char* B);

#endif
