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


#include "bayer.hpp"
#include "yuv.hpp"
//#include "ccvt.h"

using namespace std;

#define LINE4(jj) int LineAddr4=(jj)*w*4;

#define A(ii,jj) ((jj)*w+(ii))

#define P4(ii) int Paddr4=LineAddr4+(ii)*4;

// vesta raw to rgb conversion
void raw2rgb(unsigned char * dest,const unsigned char * const data,int w,
             int h,int mode) {
   int redShiftX,redShiftY;
   int blueShiftX, blueShiftY;
   int greenShiftX,greenShiftY;

   switch (mode) {
   case RawRgbFrame1:
      redShiftX=1;
      redShiftY=0;
      blueShiftX=0;
      blueShiftY=1;
      greenShiftX=0;
      greenShiftY=0;
      break;
   case RawRgbFrame2:
      redShiftX=0;
      redShiftY=0;
      blueShiftX=1;
      blueShiftY=1;
      greenShiftX=0;
      greenShiftY=1;
      break;
   case RawRgbFrame3:
      redShiftX=1;
      redShiftY=1;
      blueShiftX=0;
      blueShiftY=0;
      greenShiftX=0;
      greenShiftY=1;
      break;
   case RawRgbFrame4:
      redShiftX=0;
      redShiftY=1;
      blueShiftX=1;
      blueShiftY=0;
      greenShiftX=0;
      greenShiftY=0;
      break;
   }

   for(int j=2;j<h-2;++j) {
      LINE4(j);
      for(int i=2;i<w-2;++i) {
         int lx,ly;

         lx=(i-redShiftX)%2;
         ly=(j-redShiftY)%2;

         P4(i);

         if (lx==0 && ly==0) {
            dest[Paddr4+2]=data[A(i,j)];
         } else if (lx==0 && ly==1) {
            dest[Paddr4+2]
               =(data[A(i,j-1)]+data[A(i,j+1)])>>1;
         } else if (lx==1 && ly==0) {
            dest[Paddr4+2]
               = (data[A(i-1,j)]+data[A(i+1,j)])>>1;
         } else if (lx==1 && ly==1) {
            dest[Paddr4+2]
               = (data[A(i-1,j-1)]+data[A(i-1,j+1)]
                  + data[A(i+1,j-1)]+data[A(i+1,j+1)])>>2;
         }

         lx=(i-blueShiftX)%2;
         ly=(j-blueShiftY)%2;

         if (lx==0 && ly==0) {
            dest[Paddr4+0]=data[A(i,j)];
         } else if (lx==0 && ly==1) {
            dest[Paddr4+0]
               =(data[A(i,j-1)]+data[A(i,j+1)])>>1;
         } else if (lx==1 && ly==0) {
            dest[Paddr4]
               = (data[A(i-1,j)]+data[A(i+1,j)])>>1;
         } else if (lx==1 && ly==1) {
            dest[Paddr4]
               = (data[A(i-1,j-1)]+data[A(i-1,j+1)]
                  + data[A(i+1,j-1)]+data[A(i+1,j+1)])>>2;
         }

         lx=(i-greenShiftX)%2;
         ly=(j-greenShiftY)%2;

         if ((lx==0 && ly==0)
             || (lx==1 && ly==1)) {
            dest[Paddr4+1]=data[A(i,j)];
         } else {
            dest[Paddr4+1]
               = (data[A(i,j-1)]+data[A(i,j+1)]
                  + data[A(i+1,j)]+data[A(i-1,j)])>>2;
         }
      }
   }
}

// pixel types
#define	RED	0
#define GREEN1	1	// green pixel on a red line
#define GREEN2	2	// green pixel on a blue line
#define	BLUE	3

// color offsets
#define	RED_OFFSET	0
#define	GREEN_OFFSET	1
#define	BLUE_OFFSET	2

// get the bayer pixel color depending on its position
// and bayer pattern
int getPixelColor(int x,int y,int mode) {
   int color;
   int x_odd = x % 2;
   int y_odd = y % 2;
   switch (mode) {
      case RawRgbFrame1:
         if(x_odd==y_odd) {
            // green pixel
            if(y_odd)
               // blue line
               return(GREEN2);
            else
               // red line
               return(GREEN1);
         } else {
            if(y_odd)
               // blue pixel
               return(BLUE);
            else
               // red pixel
               return(RED);
         }
         break;
      case RawRgbFrame2:
         if(x_odd!=y_odd) {
            // green pixel
            if(y_odd)
               // blue line
               return(GREEN2);
            else
               // red line
               return(GREEN1);
         } else {
            if(y_odd)
               // blue pixel
               return(BLUE);
            else
            // red pixel
               return(RED);
         }
         break;
      case RawRgbFrame3:
         if(x_odd!=y_odd) {
            // green pixel
            if(y_odd)
               // red line
               return(GREEN1);
            else
               // blue line
               return(GREEN2);
         } else {
            if(y_odd)
               // red pixel
               return(RED);
            else
               // blue pixel
               return(BLUE);
         }
         break;
      case RawRgbFrame4:
         if(x_odd==y_odd) {
            // green pixel
            if(y_odd)
               // red line
               return(GREEN1);
            else
               // blue line
               return(GREEN2);
         } else {
            if(y_odd)
               // red pixel
               return(RED);
            else
               // blue pixel
               return(BLUE);
         }
         break;
   }
   return(color);
}

// rgb raw to yuv 4:4:4 color conversion
void raw2yuv444(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* data, const int w, const int h, int mode) {
   int red, green, blue;
   int pixelOffset;
   int rowOffset=1;
   int lineOffset=w;
   for(int x=0;x<w;x++) {
      for(int y=0;y<h;y++) {
         pixelOffset=(y*w+x);
         // manage edges
         if((x==0)||(x==w-1)||(y==0)||(y==h-1)) {
            red=0;
            green=0;
            blue=0;
         } else {
            switch(getPixelColor(x,y,mode)) {
               case RED :
                  red=data[pixelOffset];
                  green=(data[pixelOffset-rowOffset]
                     +data[pixelOffset+rowOffset]
                     +data[pixelOffset-lineOffset]
                     +data[pixelOffset+lineOffset])/4;
                  blue=(data[pixelOffset+lineOffset+rowOffset]
                     +data[pixelOffset+lineOffset-rowOffset]
                     +data[pixelOffset-lineOffset+rowOffset]
                     +data[pixelOffset-lineOffset-rowOffset])/4;
                  break;
               case GREEN1 :
                  red=(data[pixelOffset+rowOffset]+data[pixelOffset-rowOffset])/2;
                  green=data[pixelOffset];
                  blue=(data[pixelOffset+lineOffset]+data[pixelOffset-lineOffset])/2;
                  break;
               case GREEN2 :
                  red=(data[pixelOffset+lineOffset]+data[pixelOffset-lineOffset])/2;
                  green=data[pixelOffset];
                  blue=(data[pixelOffset+rowOffset]+data[pixelOffset-rowOffset])/2;
                  break;
               case BLUE :
                  red=(data[pixelOffset+lineOffset+rowOffset]
                     +data[pixelOffset+lineOffset-rowOffset]
                     +data[pixelOffset-lineOffset+rowOffset]
                     +data[pixelOffset-lineOffset-rowOffset])/4;
                  green=(data[pixelOffset-rowOffset]
                     +data[pixelOffset+rowOffset]
                     +data[pixelOffset-lineOffset]
                     +data[pixelOffset+lineOffset])/4;
                  blue=data[pixelOffset];
                  break;
            }
         }
         Y[pixelOffset]=((66*red+129*green+25*blue+128)>>8) + 16;
         U[pixelOffset]=((-38*red-74*green+112*blue+128)>>8) + 128;
         V[pixelOffset]=((112*red-94*green-18*blue+128)>>8) + 128;
      }
   }
}

// vesta raw to yuv420p conversion
void raw2yuv420p(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* input, const int w, const int h, int mode) {
   unsigned char* yTemp;
   unsigned char* uTemp;
   unsigned char* vTemp;
   yTemp=(unsigned char*)malloc(w*h);
   uTemp=(unsigned char*)malloc(w*h);
   vTemp=(unsigned char*)malloc(w*h);
   raw2yuv444(yTemp,uTemp,vTemp,input,w,h,mode);
   yuv444_to_yuv420(w,h,yTemp,uTemp,vTemp,Y,U,V);
   free(yTemp);
   free(uTemp);
   free(vTemp);
}
