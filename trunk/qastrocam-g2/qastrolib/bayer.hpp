/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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


#ifndef _bayer_hpp_
#define _bayer_hpp_
#include "QCamFrame.hpp"

// vesta raw to rgb conversion
void raw2rgb(unsigned char * dest,const unsigned char * const data,
               int w, int h,int mode);

// vesta raw to yuv444 conversion
void raw2yuv444(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* input, const int w, const int h, int mode);

// vesta raw to yuv420p conversion
void raw2yuv420p(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* input, const int w, const int h, int mode);

#endif
