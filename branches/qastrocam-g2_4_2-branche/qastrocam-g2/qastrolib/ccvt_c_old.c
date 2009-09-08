/*
   Colour conversion routines (RGB <-> YUV) in plain C
   (C) 2000 Nemosoft Unv.    nemosoft@smcc.demon.nl
   (C) 2005 Chris Monico <c.monico@ttu.edu> ccvt_420p_bgr32()
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "ccvt.h"
#include <assert.h>

#define PUSH_RGB24	1
#define PUSH_BGR24	2
#define PUSH_RGB32	3
#define PUSH_BGR32	4

/* This is a really simplistic approach. Speedups are welcomed. */
static void ccvt_420i(int width, int height, unsigned char *src, unsigned char *dst, int push)
{
	int line, col, linewidth;
	int y, u, v, yy, vr, ug, vg, ub;
	int r, g, b;
	unsigned char *py, *pu, *pv;

	linewidth = width + (width >> 1);
	py = src;
	pu = py + 4;
	pv = pu + linewidth;

	y = *py++;
	yy = y << 8;
	u = *pu - 128;
	ug =   88 * u;
	ub =  454 * u;
	v = *pv - 128;
	vg =  183 * v;
	vr =  359 * v;

	/* The biggest problem is the interlaced data, and the fact that odd
	   add even lines have V and U data, resp. 
	 */
	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col++) {
			r = (yy +      vr) >> 8;
			g = (yy - ug - vg) >> 8;
			b = (yy + ub     ) >> 8;
			
			switch(push) {
			case PUSH_RGB24:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				break;

			case PUSH_BGR24:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				break;
			
			case PUSH_RGB32:
				*dst++ = r;
				*dst++ = g;
				*dst++ = b;
				*dst++ = 0;
				break;

			case PUSH_BGR32:
				*dst++ = b;
				*dst++ = g;
				*dst++ = r;
				*dst++ = 0;
				break;
			}
			
			y = *py++;
			yy = y << 8;
			if ((col & 3) == 3)
				py += 2; // skip u/v
			if (col & 1) {
				if ((col & 3) == 3) {
					pu += 4; // skip y
					pv += 4;
				}
				else {
					pu++;
					pv++;
				}
				u = *pu - 128;
				ug =   88 * u;
				ub =  454 * u;
				v = *pv - 128;
				vg =  183 * v;
				vr =  359 * v;
			}
		} /* ..for col */
		if (line & 1) { // odd line: go to next band
			pu += linewidth;
			pv += linewidth;
		}
		else { // rewind u/v pointers
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}

void ccvt_420i_rgb24(int width, int height, const void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_RGB24);
}

void ccvt_420i_bgr24(int width, int height, const void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_BGR24);
}

void ccvt_420i_rgb32(int width, int height, const void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_RGB32);
}

void ccvt_420i_bgr32(int width, int height, const void *src, void *dst)
{
	ccvt_420i(width, height, (unsigned char *)src, (unsigned char *)dst, PUSH_BGR32);
}


void ccvt_420i_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv)
{
	short *s, *dy, *du, *dv;
	int line, col;

	s = (short *)src;
	dy = (short *)dsty;
	du = (short *)dstu;
	dv = (short *)dstv;
	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col += 4) {
			*dy++ = *s++;
			*dy++ = *s++;
			if (line & 1)
				*dv++ = *s++;
			else
				*du++ = *s++;
		} /* ..for col */
	} /* ..for line */
}

void ccvt_420i_yuyv(int width, int height, const void *src, void *dst)
{
	int line, col, linewidth;
	unsigned const char *py, *pu, *pv;
        unsigned char *d;

	linewidth = width + (width >> 1);
	py = (unsigned const char *)src;
	pu = (unsigned const char *)src + 4;
	pv = pu + linewidth;
	d = (unsigned char *)dst;

	for (line = 0; line < height; line++) {
		for (col = 0; col < width; col += 4) {
			/* four pixels in one go */
			*d++ = *py++;
			*d++ = *pu++;
			*d++ = *py++;
			*d++ = *pv++;
			
			*d++ = *py++;
			*d++ = *pu++;
			*d++ = *py++;
			*d++ = *pv++;

			py += 2;
			pu += 4;
			pv += 4;
		} /* ..for col */
		if (line & 1) { // odd line: go to next band
			pu += linewidth;
			pv += linewidth;
		}
		else { // rewind u/v pointers
			pu -= linewidth;
			pv -= linewidth;
		}
	} /* ..for line */
}

/* CJM, 11/5/05: In my environment, these don't seem to be used so I didn't code them.
   But something needs to be linked in, so here we go:
*/
void ccvt_420p_rgb32(int width, int height, const void *srcy, const void *srcu, const void *srcv, void *dst) {
   assert(0);
}
void ccvt_rgb24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv) {
   assert(0);
}
void ccvt_bgr24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv) {
   assert(0);
}

#define MAX(_a, _b) ((_a)>(_b) ? (_a) : (_b))
#define MIN(_a, _b) ((_a)<(_b) ? (_a) : (_b))


/* CJM, 11/5/05 : it's not very fast, but it works. Doing two columns
   per pass doesn't help either, so I just kept it simple.
*/
void ccvt_420p_bgr32(int width, int height, const void *srcy, const void *srcu, const void *srcv, void *dst)
{ unsigned char  *py, *pu, *pv, *lastY, *d;
  int    y, u, v, yy, vr, ug, vg, ub;
  int    r, g, b, row, col, w2;

  py=(unsigned char *)srcy;
  pu=(unsigned char *)srcu; 
  pv=(unsigned char *)srcv;
  d=dst;
  lastY = &py[width*height-1];


  row = col=0;
  w2 = width/2;
  u = *pu - 128; ug = 88*u;  ub = 454*u;
  v = *pv - 128; vg = 183*v; vr = 359 * v;
  col=1; /* Makes the col%2 test easier if we count from 1 instead of 0. */
  while (py<lastY) {
    y = *py - 16;  
    yy = y<<8;
    r = (yy+vr)>>8;
    g = (yy - ug - vg) >> 8;
    b = (yy + ub) >> 8;        
    /* This clamping is expensive, and should be done in a better way. */
    r = MAX(MIN(255, r), 0);
    g = MAX(MIN(255, g), 0);
    b = MAX(MIN(255, b), 0);
    *d++ = (char)b; 
    *d++ = (char)g; 
    *d++ = (char)r; 
    *d++ = 0; /* filler byte for alpha */
    col++;
    py++;

    if (col&1) {
      pu++; pv++;
      u = *pu - 128; 
      v = *pv - 128; 
      ug = 88*u;  
      ub = 454*u;
      vg = 183*v; 
      vr = 359 * v;
    }

    if (col ==width) {
      col=1;
      row++;
      if ((row&1)==0) {
        pu -= w2;
        pv -= w2;
      } else {
        pu++; pv++;
      }
      u = *pu - 128; ug = 88*u;  ub = 454*u;
      v = *pv - 128; vg = 183*v; vr = 359 * v;
    } 
  }
}

