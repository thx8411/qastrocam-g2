/*
 * Convert an image from yuv colourspace to rgb 
 *
 * Code by Tony Hague (C) 2001.
 */

#include <assert.h>

#include "ccvt.h"
#include "ccvt_types.h"

/* by suitable definition of PIXTYPE, can do yuv to rgb or bgr, with or
without word alignment */

/* This doesn't exactly earn a prize in a programming beauty contest. */

#define WHOLE_FUNC2RGB(type) 			\
	const unsigned char *y1, *y2, *u, *v; 	\
	PIXTYPE_##type *l1, *l2;		\
	int r, g, b, cr, cg, cb, yp, j, i;	\
						\
	if ((width & 1) || (height & 1))	\
		return;				\
						\
	l1 = (PIXTYPE_##type *)dst;		\
	l2 = l1 + width;			\
	y1 = (unsigned char *)src;		\
	y2 = y1 + width;			\
	u = (unsigned char *)src + width * height;		\
	v = u + (width * height) / 4;		\
	j = height / 2;				\
	while (j--) {				\
		i = width / 2;			\
		while (i--) {			\
			/* Since U & V are valid for 4 pixels, repeat code 4 	\
			   times for different Y */				\
			cb = ((*u-128) * 454)>>8;				\
			cr = ((*v-128) * 359)>>8;				\
			cg = ((*v-128) * 183 + (*u-128) * 88)>>8;		\
						\
			yp = *(y1++);		\
			r = yp + cr; 		\
			b = yp + cb;		\
			g = yp - cg;            \
			SAT(r);                 \
			SAT(g);                 \
			SAT(b);                 \
			l1->b = b;		\
			l1->g = g;              \
			l1->r = r;              \
			l1++;                   \
                                                \
			yp = *(y1++);           \
			r = yp + cr;            \
			b = yp + cb;            \
			g = yp - cg;            \
			SAT(r);                 \
			SAT(g);                 \
			SAT(b);                 \
			l1->b = b;		\
			l1->g = g;		\
			l1->r = r;		\
			l1++;			\
						\
			yp = *(y2++);		\
			r = yp + cr; 		\
			b = yp + cb;		\
			g = yp - cg;		\
			SAT(r);			\
			SAT(g);			\
			SAT(b);			\
			l2->b = b;		\
			l2->g = g;		\
			l2->r = r;		\
			l2++;			\
						\
			yp = *(y2++);		\
			r = yp + cr; 		\
			b = yp + cb;		\
			g = yp - cg;		\
			SAT(r);			\
			SAT(g);			\
			SAT(b);			\
			l2->b = b;		\
			l2->g = g;		\
			l2->r = r;		\
			l2++;			\
						\
			u++;			\
			v++;			\
		}				\
		y1 = y2;			\
		y2 += width;			\
		l1 = l2;			\
		l2 += width;			\
	}




//void ccvt_420p_bgr32(int width, int height, const void *src, void *dst)
//{
//	WHOLE_FUNC2RGB(bgr32)
//}

void ccvt_420p_bgr24(int width, int height, const void *src, void *dst)
{
	WHOLE_FUNC2RGB(bgr24)
}

void ccvt_420p_rgb32(int width, int height, const void *src, void *dst)
{
	WHOLE_FUNC2RGB(rgb32)
}

void ccvt_420p_rgb24(int width, int height, const void *src, void *dst)
{
	WHOLE_FUNC2RGB(rgb24)
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


/************************************/
/* added by Blaise-Florentin Collin */
/*             (c) 2009             */
/************************************/

void ccvt_bgr24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv) {
   assert(0);
}

/* luminance only */
void ccvt_rgb24_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv) {
   int p,size;
   unsigned char* sPlan;
   unsigned char* yPlan;

   unsigned short R;
   unsigned short G;
   unsigned short B;

   sPlan=(unsigned char*)src;
   yPlan=(unsigned char*)dsty;

   size=height*width;
   for(p=0;p<size;p++) {
         R=sPlan[p*3];
         G=sPlan[p*3+1];
         B=sPlan[p*3+2];

         yPlan[p]=(unsigned char)(((66*R+129*G+25*B+128)>>8)+16);
   }
}

void ccvt_yuyv_420p(int width, int height, const void *src, void *dsty, void *dstu, void *dstv) {
   static int x,y;
   static int y_offset;
   static int uv_offset;
   static int uv_offset2;
   static int s_offset;
   static int s_offset2;
   static int line_offset;
   static int half_width;
   static int half_height;
   static int double_width;
   static unsigned char* sPlan;
   static unsigned char* yPlan;
   static unsigned char* uPlan;
   static unsigned char* vPlan;

   sPlan=(unsigned char*)src;
   yPlan=(unsigned char*)dsty;
   uPlan=(unsigned char*)dstu;
   vPlan=(unsigned char*)dstv;
   line_offset=2*width;
   half_width=width/2;
   half_height=height/2;
   double_width=width*2;
   for(y=0;y<half_height;y++) {
      y_offset=y*double_width;
      uv_offset=y*half_width;
      for(x=0;x<half_width;x++) {
         s_offset2=(y_offset+x*2);
         s_offset=s_offset2*2;
         uv_offset2=uv_offset+x;
         yPlan[s_offset2]=sPlan[s_offset];
         yPlan[s_offset2+width]=sPlan[s_offset+line_offset];
         s_offset++;
         uPlan[uv_offset2]=(sPlan[s_offset]+sPlan[s_offset+line_offset])/2;
         s_offset++;
         s_offset2++;
         yPlan[s_offset2]=sPlan[s_offset];
         yPlan[s_offset2+width]=sPlan[s_offset+line_offset];
         s_offset++;
         vPlan[uv_offset2]=(sPlan[s_offset]+sPlan[s_offset+line_offset])/2;
      }
   }
}

