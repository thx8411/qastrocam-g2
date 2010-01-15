/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2010 Blaise-Florentin Collin

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

#include "QCamFrame.hpp"

using namespace std;

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

