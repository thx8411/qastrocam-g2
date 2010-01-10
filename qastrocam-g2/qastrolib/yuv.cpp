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

int clip(int v) {
   if(v<0) return(0);
   if(v>255) return(255);
   return(v);
}

//
// TO YUV444
//

// rgb24 to yuv444 planar
void rgb24_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   int p,size;
   int R,G,B;
   size=h*w;
   for(p=0;p<size;p++) {
         R=src[p*3];
         G=src[p*3+1];
         B=src[p*3+2];
         dstY[p]=((66*R+129*G+25*B+128)>>8)+16;
         dstU[p]=((-38*R-74*G+112*B+128)>>8)+128;
         dstV[p]=((112*R-94*G-18*B+128)>>8)+128;
   }
}

// yuv422 to yuv444 planar
void yuv422_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   int i;
   int size=w*h;
   for(i=0;i<size;i++){
      dstY[i]=src[i*2];
      dstU[i]=src[(i/2)*4+1];
      dstV[i]=src[(i/2)*4+3];
   }
}

// yuv420 planar to yuv444 planar
void yuv420_to_yuv444(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   int i,j;
   memcpy(dstY,srcY,w*h);
   for(i=0;i<w;i++){
      for(j=0;j<h;j++){
         dstU[j*w+i]=srcU[j*w/4+i/2];
         dstV[j*w+i]=srcV[j*w/4+i/2];
      }
   }
}

//
// FROM YUV444
//

// yuv444 planar to rgb24
void yuv444_to_rgb24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst){
   int p,size;
   int Y,U,V;
   size=h*w;
   for(p=0;p<size;p++) {
      Y=srcY[p]-16;
      U=srcU[p]-128;
      V=srcV[p]-128;
      dst[p*3]=clip((298*Y+ 409*V+128)>>8);
      dst[p*3+1]=clip((298*Y-100*U-208*V+128)>>8);
      dst[p*3+2]=clip((298*Y+516*U+128)>>8);
   }
}

// yuv444 planar to rgb32
void yuv444_to_bgr32(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst){
   int p,size;
   int Y,U,V;
   size=h*w;
   for(p=0;p<size;p++) {
      Y=srcY[p]-16;
      U=srcU[p]-128;
      V=srcV[p]-128;
      dst[p*4]=clip((298*Y+516*U+128)>>8);
      dst[p*4+1]=clip((298*Y-100*U-208*V+128)>>8);
      dst[p*4+2]=clip((298*Y+ 409*V+128)>>8);
      dst[p*4+3]=0;
   }
}

// 4:4:4 planar yuv to 4:2:0 planar
void yuv444_to_yuv420(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV) {
   int i,j;
   int maxX=w/2;
   int maxY=h/2;
   memcpy(dstY,srcY,w*h);
   for(i=0;i<maxX;i++) {
      for(j=0;j<maxY;j++) {
         dstU[j*maxX+i]=srcU[(j*w+i)*2];
         dstV[j*maxX+i]=srcV[(j*w+i)*2];
      }
   }
}
