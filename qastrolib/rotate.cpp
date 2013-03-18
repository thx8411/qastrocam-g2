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

// to be rewritten with yuv444

#include "QCamFrame.hpp"
#include "rasterOp.hpp"
#include <math.h>

void QCamFrameCommon::rotatePI(int center_x,
                               int center_y) {
   QCamFrameCommon * tmp=clone();
   copy(*tmp,0,0,size_.width()-1,size_.height()-1,0,0,true,true);
   delete tmp;
}

void QCamFrameCommon::rotate(int center_x,
                             int center_y,
                             double angle) {
   double  hangle;

   while(angle<-M_PI) {
      angle+=M_PI*2;
   }
   while (angle>M_PI) {
      angle-=M_PI*2;
   }

   if (angle>M_PI_2) {
      rotatePI(center_x,center_y);
      angle-=M_PI;
   } else if (angle<-M_PI_2) {
      rotatePI(center_x,center_y);
      angle+=M_PI;
   }
   if (empty() || angle==0.0) {
      return;
   }
   hangle = atan(sin(angle));
   hShear(center_y, angle / 2.);
   vShear(center_x, hangle);
   hShear(center_y, angle / 2.);
}

void QCamFrameCommon::hShear(int liney,
                             double radang) {
   int sign, w, h;
   int y, yincr, inityincr, hshift;
   double tanangle, invangle;

   if (empty()) {
      return;
   }

   sign = SIGN(radang);
   w = size_.width();
   h = size_.height();
   tanangle = tan(radang);
   invangle = ABS(1. / tanangle);
   inityincr = (int)(invangle / 2.);
   yincr = (int)invangle;

   rasterOpH(liney - inityincr, 2 * inityincr, 0);

   for (hshift = 1, y = liney + inityincr; y < h; hshift++) {
      yincr = (int)(invangle * (hshift + 0.5) + 0.5) - (y - liney);
      if (h - y < yincr)  /* reduce for last one if req'd */
         yincr = h - y;
      rasterOpH(y, yincr, -sign*hshift);
      y += yincr;
   }

   for (hshift = -1, y = liney - inityincr; y > 0; hshift--) {
      yincr = (y - liney) - (int)(invangle * (hshift - 0.5) + 0.5);
      if (y < yincr)  /* reduce for last one if req'd */
         yincr = y;
      rasterOpH(y - yincr, yincr, -sign*hshift);
      y -= yincr;
   }
}

void QCamFrameCommon::vShear(int     linex,
                             double radang) {
   int sign, w, h;
   int x, xincr, initxincr, vshift;
   double tanangle, invangle;

   if (empty()) {
      return;
   }

   sign = SIGN(radang);
   w = size_.width();
   h = size_.height();
   tanangle = tan(radang);
   invangle = ABS(1. / tanangle);
   initxincr = (int)(invangle / 2.);
   xincr = (int)invangle;

   rasterOpV(linex - initxincr, 2 * initxincr, 0);

   for (vshift = 1, x = linex + initxincr; x < w; vshift++) {
      xincr = (int)(invangle * (vshift + 0.5) + 0.5) - (x - linex);
      if (w - x < xincr)  /* reduce for last one if req'd */
         xincr = w - x;
      rasterOpV(x, xincr, sign*vshift);
      x += xincr;
   }

   for (vshift = -1, x = linex - initxincr; x > 0; vshift--) {
      xincr = (x - linex) - (int)(invangle * (vshift - 0.5) + 0.5);
      if (x < xincr)  /* reduce for last one if req'd */
         xincr = x;
      rasterOpV(x - xincr, xincr, sign*vshift);
      x -= xincr;
   }
}

void QCamFrameCommon::rasterOpV(int   x,
                                int   w,
                                int   vshift) {
#if 1
   move(x,0,x+w-1,size_.height()-1,x,vshift);
#else
   rasteropVipLow((unsigned int*)Y(),
                  size_.width(), size_.height(),
                  8, size_.width()/4,
                  x, w, -vshift);
#endif

}


void QCamFrameCommon::rasterOpH(int   y,
                                int   h,
                                int   hshift) {
#if 1
   move(0,y,size_.width()-1,y+h-1,hshift,y);
#else
   rasteropHipLow((unsigned int*)Y(), size_.height(),
                  8, size_.width()/4,
                  y, h, hshift);
#endif
}
