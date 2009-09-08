/*====================================================================*
  -  Copyright (C) 2001 Leptonica.  All rights reserved.
  -  This software is distributed in the hope that it will be
  -  useful, but with NO WARRANTY OF ANY KIND.
  -  No author or distributor accepts responsibility to anyone for the
  -  consequences of using this software, or for whether it serves any
  -  particular purpose or works at all, unless he or she says so in
  -  writing.  Everyone is granted permission to copy, modify and
  -  redistribute this source code, for commercial or non-commercial
  -  purposes, with the following restrictions: (1) the origin of this
  -  source code must not be misrepresented; (2) modified versions must
  -  be plainly marked as such; and (3) this notice may not be removed
  -  or altered from any source or modified source distribution.
  *====================================================================*/

/*
 *  ropiplow.c
 *
 *      Low level in-place full height vertical block transfer
 *
 *           void           rasteropVipLow()
 *
 *      Low level in-place full width horizontal block transfer
 *
 *           void           rasteropHipLow()
 *           void           shiftDataHorizontalLow()
 *
 */


//#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#include "rasterOp.hpp"


#define COMBINE_PARTIAL(d, s, m)     ( ((d) & ~(m)) | ((s) & (m)) )



static const unsigned int lmask32[] = {0x0,
                                       0x80000000, 0xc0000000, 0xe0000000, 0xf0000000,
                                       0xf8000000, 0xfc000000, 0xfe000000, 0xff000000,
                                       0xff800000, 0xffc00000, 0xffe00000, 0xfff00000,
                                       0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000,
                                       0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000,
                                       0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00,
                                       0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0,
                                       0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff};

static const unsigned int rmask32[] = {0x0,
                                       0x00000001, 0x00000003, 0x00000007, 0x0000000f,
                                       0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
                                       0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
                                       0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
                                       0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
                                       0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
                                       0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
                                       0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff};





/*--------------------------------------------------------------------*
 *                 Low-level Vertical In-place Rasterop               *
 *--------------------------------------------------------------------*/
/*
 *  rasteropVipLow()
 *
 *       Input: 8 args
 *              These are
 *                   data   (ptr to image data)
 *                   pixw   (width)
 *                   pixh   (height)
 *                   depth  (depth)
 *                   wpl    (wpl)
 *                   x      (x val of UL corner of rectangle)
 *                   w      (width of rectangle)
 *                   shift  (+ shifts data downward in vertical column)
 *
 *       Return: 0 if OK; 1 on error.
 *
 *       Action: scales width, performs clipping and dispatches
 *
 */
void
rasteropVipLow(unsigned int  *data,
               int    pixw,
               int    pixh,
	       int    depth,
	       int    wpl,
	       int    x,
	       int    w,
               int    shift)
{
   //int    hangw;
   int    fwpartb;    /* boolean (1, 0) if first word is partial */
   int    fwpart2b;   /* boolean (1, 0) if first word is doubly partial */
   unsigned int   fwmask;     /* mask for first partial word */
   int    fwbits;     /* first word bits in ovrhang */
   unsigned int  *pdfwpart;   /* ptr to first partial dest word */
   unsigned int  *psfwpart;   /* ptr to first partial src word */
   int    fwfullb;    /* boolean (1, 0) if there exists a full word */
   int    nfullw;     /* number of full words */
   unsigned int  *pdfwfull;   /* ptr to first full dest word */
   unsigned int  *psfwfull;   /* ptr to first full src word */
   int    lwpartb;    /* boolean (1, 0) if last word is partial */
   unsigned int   lwmask;     /* mask for last partial word */
   int    lwbits;     /* last word bits in ovrhang */
   unsigned int  *pdlwpart;   /* ptr to last partial dest word */
   unsigned int  *pslwpart;   /* ptr to last partial src word */
   int    dirwpl;     /* directed wpl (-wpl * sign(shift)) */
   int    absshift;   /* absolute value of shift; for use in iterator */
   int    i, j;

   /*--------------------------------------------------------*
    *            Scale horizontal dimensions by depth        *
    *--------------------------------------------------------*/
   if (depth != 1) {
      pixw *= depth;
      x *= depth;
      w *= depth;
   }


   /*--------------------------------------------------------*
    *                   Clip horizontally                    *
    *--------------------------------------------------------*/
   if (x < 0) {
      w += x;    /* reduce w */
      x = 0;     /* clip to x = 0 */
   }
   if (x + w > pixw)
      w = pixw - x;   /* clip to x + w = pixw */

   if (w <= 0)  /* no part of vertical slice is in the image */
      return;

   /*--------------------------------------------------------*
    *                Preliminary calculations                *
    *--------------------------------------------------------*/
   /* is the first word partial? */
   if ((x & 31) == 0) {  /* if not */
      fwpartb = 0;
      fwbits = 0;
   }
   else {  /* if so */
      fwpartb = 1;
      fwbits = 32 - (x & 31);
      fwmask = rmask32[fwbits];
      if (shift >= 0) { /* go up from bottom */
         pdfwpart = data + wpl * (pixh - 1) + (x >> 5);
         psfwpart = data + wpl * (pixh - 1 - shift) + (x >> 5);
      }
      else {  /* go down from top */
         pdfwpart = data + (x >> 5);
         psfwpart = data - wpl * shift + (x >> 5);
      }
   }

   /* is the first word doubly partial? */
   if (w >= fwbits)  /* if not */
      fwpart2b = 0;
   else {  /* if so */
      fwpart2b = 1;
      fwmask &= lmask32[32 - fwbits + w];
   }

   /* is there a full dest word? */
   if (fwpart2b == 1) {  /* not */
      fwfullb = 0;
      nfullw = 0;
   }
   else {
      nfullw = (w - fwbits) >> 5;
      if (nfullw == 0)  /* if not */
         fwfullb = 0;
      else {  /* if so */
         fwfullb = 1;
         if (fwpartb) {
            pdfwfull = pdfwpart + 1;
            psfwfull = psfwpart + 1;
         }
         else {
            if (shift >= 0) { /* go up from bottom */
               pdfwfull = data + wpl * (pixh - 1) + (x >> 5);
               psfwfull = data + wpl * (pixh - 1 - shift) + (x >> 5);
            }
            else {  /* go down from top */
               pdfwfull = data + (x >> 5);
               psfwfull = data - wpl * shift + (x >> 5);
            }
         }
      }
   }

   /* is the last word partial? */
   lwbits = (x + w) & 31;
   if (fwpart2b == 1 || lwbits == 0)  /* if not */
      lwpartb = 0;
   else {
      lwpartb = 1;
      lwmask = lmask32[lwbits];
      if (fwpartb) {
         pdlwpart = pdfwpart + 1 + nfullw;
         pslwpart = psfwpart + 1 + nfullw;
      }
      else {
         if (shift >= 0) { /* go up from bottom */
            pdlwpart = data + wpl * (pixh - 1) + (x >> 5) + nfullw;
            pslwpart = data + wpl * (pixh - 1 - shift) + (x >> 5) + nfullw;
         }
         else {  /* go down from top */
            pdlwpart = data + (x >> 5) + nfullw;
            pslwpart = data - wpl * shift + (x >> 5) + nfullw;
         }
      }
   }

   /* determine the direction of flow from the shift
    * If the shift >= 0, data flows downard from src
    * to dest, starting at the bottom and working up.
    * If shift < 0, data flows upward from src to 
    * dest, starting at the top and working down. */
   dirwpl = (shift >= 0) ? -wpl : wpl;
   absshift = ABS(shift);


   /*--------------------------------------------------------*
    *            Now we're ready to do the ops               *
    *--------------------------------------------------------*/

   /* do the first partial word */
   if (fwpartb) {
      for (i = 0; i < pixh - absshift; i++) {
         *pdfwpart = COMBINE_PARTIAL(*pdfwpart, *psfwpart, fwmask);
         pdfwpart += dirwpl;
         psfwpart += dirwpl;
      }
      for (i = pixh - absshift; i < pixh; i++) {
         *pdfwpart = COMBINE_PARTIAL(*pdfwpart, 0x0, fwmask);
         pdfwpart += dirwpl;
      }
   }

   /* do the full words */
   if (fwfullb) {
      for (i = 0; i < pixh - absshift; i++) {
         for (j = 0; j < nfullw; j++)
            *(pdfwfull + j) = *(psfwfull + j);
         pdfwfull += dirwpl;
         psfwfull += dirwpl;
      }
      for (i = pixh - absshift; i < pixh; i++) {
         for (j = 0; j < nfullw; j++)
            *(pdfwfull + j) = 0x0;
         pdfwfull += dirwpl;
      }
   }

   /* do the last partial word */
   if (lwpartb) {
      for (i = 0; i < pixh - absshift; i++) {
         *pdlwpart = COMBINE_PARTIAL(*pdlwpart, *pslwpart, lwmask);
         pdlwpart += dirwpl;
         pslwpart += dirwpl;
      }
      for (i = pixh - absshift; i < pixh; i++) {
         *pdlwpart = COMBINE_PARTIAL(*pdlwpart, 0x0, lwmask);
         pdlwpart += dirwpl;
      }
   } 

   return;
}



/*--------------------------------------------------------------------*
 *                 Low-level Horizontal In-place Rasterop             *
 *--------------------------------------------------------------------*/
/* 
 *  rasteropHipLow()
 */
void
rasteropHipLow(unsigned int  *data,
	       int    pixh,
	       int    depth,
	       int    wpl,
	       int    y,
	       int    h,
	       int    shift)
{
   int     i;
   unsigned int   *line;

   /* clip band if necessary */
   if (y < 0) {
      h += y;  /* reduce h */
      y = 0;   /* clip to y = 0 */
   }
   if (y + h > pixh)
      h = pixh - y;   /* clip to y + h = pixh */

   if (h <= 0)  /* no part of horizontal slice is in the image */
      return;

   for (i = y; i < y + h; i++) {
      line = data + i * wpl;
      shiftDataHorizontalLow(line, wpl, line, wpl, shift * depth);
   }
}


/*
 *  shiftDataHorizontalLow()
 *
 *      Input:  datad  (ptr to beginning of dest line)
 *              wpld   (wpl of dest)
 *              datas  (ptr to beginning of src line)
 *              wpls   (wpl of src)
 *              shift  (horizontal shift of block; >0 is to right)
 *      Return: void
 *      Note: this can be used for in-place operation, as
 *            for example, in rasteropHipLow()
 */
void
shiftDataHorizontalLow(unsigned int  *datad,
                       int    wpld,
		       unsigned int  *datas,
		       int    wpls,
		       int    shift)
{
   int     j, firstdw, wpl, rshift, lshift;
   unsigned int   *lined, *lines;

   lined = datad;
   lines = datas;

   if (shift >= 0) {   /* src shift to right; data flows to
                        * right, starting at right edge and
                        * progressing leftward. */
      firstdw = shift / 32;
      wpl = MIN(wpls, wpld - firstdw);
      lined += firstdw + wpl - 1;
      lines += wpl - 1;
      rshift = shift & 31;
      if (rshift == 0) {
         for (j = 0; j < wpl; j++)
            *lined-- = *lines--;

         /* clear out the rest to the left edge */
         for (j = 0; j < firstdw; j++)
            *lined-- = 0;
      }
      else {
         lshift = 32 - rshift;
         for (j = 1; j < wpl; j++) {
            *lined-- = *(lines - 1) << lshift | *lines >> rshift;
            lines--;
         }
         *lined = *lines >> rshift;  /* partial first */

         /* clear out the rest to the left edge */
         *lined &= ~lmask32[rshift];
         lined--;
         for (j = 0; j < firstdw; j++)
            *lined-- = 0;
      }
   }
   else {  /* src shift to left; data flows to left, starting
            * at left edge and progressing rightward. */
      firstdw = (-shift) / 32;
      wpl = MIN(wpls - firstdw, wpld);
      lines += firstdw;
      lshift = (-shift) & 31;
      if (lshift == 0) {
         for (j = 0; j < wpl; j++)
            *lined++ = *lines++;

         /* clear out the rest to the right edge */
         for (j = 0; j < firstdw; j++)
            *lined++ = 0;
      }
      else {
         rshift = 32 - lshift;
         for (j = 1; j < wpl; j++) {
            *lined++ = *lines << lshift | *(lines + 1) >> rshift;
            lines++;
         }
         *lined = *lines << lshift;  /* partial last */

         /* clear out the rest to the right edge */
         /* first clear the lshift pixels of this partial word */
         *lined &= ~rmask32[lshift];
         lined++;
         /* then the remaining words to the right edge */
         for (j = 0; j < firstdw; j++)
            *lined++ = 0;
      }
   }
   return;
}

