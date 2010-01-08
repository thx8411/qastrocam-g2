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

#include <string.h>
#include "yuv.hpp"

// yuv444 conversion tools

// 4:4:4 planar yuv to 4:2:0 planar
void yuv444_to_yuv420(int width, int height, const void* srcY, const void* srcU, const void* srcV, void* dstY, void* dstU, void* dstV) {
   int i,j;
   int maxX=width/2;
   int maxY=height/2;
   unsigned char* uSrcPlan=(unsigned char*)srcU;
   unsigned char* vSrcPlan=(unsigned char*)srcV;
   unsigned char* uDstPlan=(unsigned char*)dstU;
   unsigned char* vDstPlan=(unsigned char*)dstV;
   unsigned char* yDstPlan=(unsigned char*)dstY;
   memcpy(dstY,srcY,width*height);
   for(i=0;i<maxX;i++) {
      for(j=0;j<maxY;j++) {
         uDstPlan[j*maxX+i]=uSrcPlan[(j*width+i)*2];
         vDstPlan[j*maxX+i]=vSrcPlan[(j*width+i)*2];
      }
   }
}
