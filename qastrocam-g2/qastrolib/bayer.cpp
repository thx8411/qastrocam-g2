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

// uses ITU-R formulas

#include "bayer.hpp"
#include "yuv.hpp"

#include "QCamFrame.hpp"

using namespace std;

extern unsigned char clip(int v);

// pixel types
#define	RED	0
#define GREEN1	1	// green pixel on a red line
#define GREEN2	2	// green pixel on a blue line
#define	BLUE	3

// 1st and 6rd values : syncs with the bayer mode enum
int bayerPatterns[6][2][2]={{{0,0},{0,0}},{{0,0},{0,0}},{{GREEN1,BLUE},{RED,GREEN2}},{{RED,GREEN2},{GREEN1,BLUE}},{{BLUE,GREEN1},{GREEN2,RED}},{{GREEN2,RED},{BLUE,GREEN1}}};

// vesta raw to 8 bits green using green pixels binning only (source res must be w*2 x h*2)
void raw2greenBinning(unsigned char* Y, unsigned char* data, const int w, const int h, int mode) {
   for(int i=0; i<w; i++) {
      for(int j=0; j<h; j++) {
         switch(mode) {
            case RawRgbFrame1 :
            case RawRgbFrame4 :
               Y[j*w+i]=clip(data[j*2*w*2+i*2]+data[(j*2+1)*w*2+i*2+1]);
               break;
            case RawRgbFrame2 :
            case RawRgbFrame3 :
               Y[j*w+i]=clip(data[(j*2+1)*w*2+i*2]+data[j*2*w*2+i*2+1]);
               break;
         }
      }
   }
}

// vesta raw to 8 bits green using green pixels only (source res must be w*2 x h*2)
void raw2green(unsigned char* Y, unsigned char* data, const int w, const int h, int mode) {
   for(int i=0; i<w; i++) {
      for(int j=0; j<h; j++) {
         switch(mode) {
            case RawRgbFrame1 :
            case RawRgbFrame4 :
               Y[j*w+i]=clip(data[j*2*w*2+i*2]+data[(j*2+1)*w*2+i*2+1]/2);
               break;
            case RawRgbFrame2 :
            case RawRgbFrame3 :
               Y[j*w+i]=clip(data[(j*2+1)*w*2+i*2]+data[j*2*w*2+i*2+1]/2);
               break;
         }
      }
   }
}

// vesta raw to 8 bits red using red pixels only (source res must be w*2 x h*2)
void raw2red(unsigned char* Y, unsigned char* data, const int w, const int h, int mode) {
   for(int i=0; i<w; i++) {
      for(int j=0; j<h; j++) {
         switch(mode) {
            case RawRgbFrame1 :
               Y[j*w+i]=data[j*2*w*2+i*2+1];
               break;
            case RawRgbFrame2 :
               Y[j*w+i]=data[j*2*w*2+i*2];
               break;
            case RawRgbFrame3 :
               Y[j*w+i]=data[(j*2+1)*w*2+i*2+1];
               break;
            case RawRgbFrame4 :
               Y[j*w+i]=data[(j*2+1)*w*2+i*2];
               break;
         }
      }
   }
}

// vesta raw to 8 bits blue using blue pixels only (source res must be w*2 x h*2)
void raw2blue(unsigned char* Y, unsigned char* data, const int w, const int h, int mode) {
   for(int i=0; i<w; i++) {
      for(int j=0; j<h; j++) {
         switch(mode) {
            case RawRgbFrame1 :
               Y[j*w+i]=data[(j*2+1)*w*2+i*2];
               break;
            case RawRgbFrame2 :
               Y[j*w+i]=data[(j*2+1)*w*2+i*2+1];
               break;
            case RawRgbFrame3 :
               Y[j*w+i]=data[j*2*w*2+i*2];
               break;
            case RawRgbFrame4 :
               Y[j*w+i]=data[j*2*w*2+i*2+1];
               break;
         }
      }
   }
}

// vesta raw to 8 bits grey conversion
void raw2grey(unsigned char* Y, unsigned char* data, const int w, const int h, int mode) {
   /*register int pixelOffset=0;
   for(int y=0;y<h;y++) {
      for(int x=0;x<w;x++) {
         switch(bayerPatterns[mode][x%2][y%2]) {
            case RED :
               //
               Y[pixelOffset]=clip(0.95*(double)data[pixelOffset]);
               break;
            case GREEN1 :
            case GREEN2 :
               Y[pixelOffset]=data[pixelOffset];
               break;
            case BLUE :
               //
               Y[pixelOffset]=clip(1.085*(double)data[pixelOffset]);
               break;
         }
         pixelOffset++;
      }
   }*/
   double red, green, blue;
   register int pixelOffset=0;
   register int rowOffset=1;
   register int lineOffset=w;
   register int maxW=w-1;
   register int maxH=h-1;
   for(int y=0;y<h;y++) {
      for(int x=0;x<w;x++) {
         // manage edges
         if(!x||(x==maxW)||!y||(y==maxH)) {
            red=0.0;
            green=0.0;
            blue=0.0;
         } else {
            switch(bayerPatterns[mode][x%2][y%2]) {
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
         Y[pixelOffset]=clip(0.299*red+0.587*green+0.114*blue);
         pixelOffset++;
      }
   }
}

// rgb raw to yuv 4:4:4 color conversion (nearest)
void raw2yuv444_nearest(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* data, const int w, const int h, int mode) {
   double red, green, blue;
   register int pixelOffset=0;
   register int rowOffset=1;
   register int lineOffset=w;
   register int maxW=w-1;
   register int maxH=h-1;
   for(int y=0;y<h;y++) {
      for(int x=0;x<w;x++) {
         // manage edges
         if(!x||(x==maxW)||!y||(y==maxH)) {
            red=0.0;
            green=0.0;
            blue=0.0;
         } else {
            switch(bayerPatterns[mode][x%2][y%2]) {
               case RED :
                  red=data[pixelOffset];
                  green=data[pixelOffset-rowOffset];
                  blue=data[pixelOffset+lineOffset+rowOffset];
                  break;
               case GREEN1 :
                  red=data[pixelOffset+rowOffset];
                  green=data[pixelOffset];
                  blue=data[pixelOffset+lineOffset];
                  break;
               case GREEN2 :
                  red=data[pixelOffset+lineOffset];
                  green=data[pixelOffset];
                  blue=data[pixelOffset+rowOffset];
                  break;
               case BLUE :
                  red=data[pixelOffset+lineOffset+rowOffset];
                  green=data[pixelOffset-rowOffset];
                  blue=data[pixelOffset];
                  break;
            }
         }
         Y[pixelOffset]=clip(0.299*red+0.587*green+0.114*blue);
         U[pixelOffset]=clip(-0.169*red-0.331*green+0.499*blue+128);
         V[pixelOffset]=clip(0.499*red-0.418*green-0.0813*blue+128);
         pixelOffset++;
      }
   }
}

// rgb raw to yuv 4:4:4 color conversion (bilinear)
void raw2yuv444_bilinear(unsigned char* Y, unsigned char* U, unsigned char* V, unsigned char* data, const int w, const int h, int mode) {
   double red, green, blue;
   register int pixelOffset=0;
   register int rowOffset=1;
   register int lineOffset=w;
   register int maxW=w-1;
   register int maxH=h-1;
   for(int y=0;y<h;y++) {
      for(int x=0;x<w;x++) {
         // manage edges
         if(!x||(x==maxW)||!y||(y==maxH)) {
            red=0.0;
            green=0.0;
            blue=0.0;
         } else {
            switch(bayerPatterns[mode][x%2][y%2]) {
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
         Y[pixelOffset]=clip(0.299*red+0.587*green+0.114*blue);
         U[pixelOffset]=clip(-0.169*red-0.331*green+0.499*blue+128);
         V[pixelOffset]=clip(0.499*red-0.418*green-0.0813*blue+128);
         pixelOffset++;
      }
   }
}

