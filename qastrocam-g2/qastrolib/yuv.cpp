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

// all these conversions use ITU-R formulas

#include <stdlib.h>
#include <string.h>
#include "yuv.hpp"

// yuv444 conversion tool

unsigned char clip(double v) {
   if(v<0) return(0);
   if(v>255) return(255);
   return((unsigned char)v);
}

unsigned char clip(int v) {
   if(v<0) return(0);
   if(v>255) return(255);
   return((unsigned char)v);
}

//
// TO YUV444
//

// bgr32 to yuv444 planar
void bgr32_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV) {
   static int p,size;
   static double R,G,B;
   size=h*w;
   for(p=0;p<size;p++) {
         R=src[p*4];
         G=src[p*4+1];
         B=src[p*4+2];
         dstY[p]=clip(0.299*R+0.587*G+0.114*B);
         dstU[p]=clip(-0.169*R-0.331*G+0.499*B+128);
         dstV[p]=clip(0.499*R-0.418*G-0.0813*B+128);
   }
}

// rgb24 to yuv444 planar
void rgb24_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   static int p,size;
   static double R,G,B;
   size=h*w;
   for(p=0;p<size;p++) {
         R=src[p*3];
         G=src[p*3+1];
         B=src[p*3+2];
         dstY[p]=clip(0.299*R+0.587*G+0.114*B);
         dstU[p]=clip(-0.169*R-0.331*G+0.499*B+128);
         dstV[p]=clip(0.499*R-0.418*G-0.0813*B+128);
   }
}

// uyvy to yuv444 planar
void uyvy_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   static int i;
   static int size;
   size==w*h;
   for(i=0;i<size;i++){
      dstY[i]=src[i*2+1];
      dstU[i]=src[(i/2)*4];
      dstV[i]=src[(i/2)*4+2];
   }
}

// yuyv to yuv444 planar
void yuyv_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   static int i;
   static int size;
   size=w*h;
   for(i=0;i<size;i++){
      dstY[i]=src[i*2];
      dstU[i]=src[(i/2)*4+1];
      dstV[i]=src[(i/2)*4+3];
   }
}

// yuv420 planar to yuv444 planar
void yuv420_to_yuv444(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
   static int i,j;
   memcpy(dstY,srcY,w*h);
   for(i=0;i<w;i++){
      for(j=0;j<h;j++){
         dstU[j*w+i]=srcU[(j/2)*(w/2)+i/2];
         dstV[j*w+i]=srcV[(j/2)*(w/2)+i/2];
      }
   }
}

// YCbCr to yuv444
void ycbcr_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV) {
   static int i;
   static int size;
   size=w*h;
   for(i=0;i<size;i++) {
      dstY[i]=src[i*3];
      dstU[i]=src[i*3+1];
      dstV[i]=src[i*3+2];
   }
}

// s505 (yyuv per line) to yuv444 planar
void s505_to_yuv444(int w, int h, const unsigned char* src, unsigned char* dstY, unsigned char* dstU, unsigned char* dstV){
}

//
// TO Y only
//

// bgr32 to y planar
void bgr32_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   static int p,size;
   static double R,G,B;
   size=h*w;
   for(p=0;p<size;p++) {
         R=src[p*4];
         G=src[p*4+1];
         B=src[p*4+2];
         dstY[p]=clip(0.299*R+0.587*G+0.114*B);
   }
}

// rgb24 to y planar
void rgb24_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   static int p,size;
   static double R,G,B;
   size=h*w;
   for(p=0;p<size;p++) {
         R=src[p*3];
         G=src[p*3+1];
         B=src[p*3+2];
         dstY[p]=clip(0.299*R+0.587*G+0.114*B);
   }
}

// uyvy to y planar
void uyvy_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   static int i;
   static int size;
   size=w*h;
   for(i=0;i<size;i++) dstY[i]=src[i*2+1];
}

// yuyv to y planar
void yuyv_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   static int i;
   static int size;
   size=w*h;
   for(i=0;i<size;i++) dstY[i]=src[i*2];
}

// yuv420 planar to y planar
void yuv420_to_y(int w, int h, const unsigned char* srcY, unsigned char* dstY) {
   memcpy(dstY,srcY,w*h);
}

// YCbCr to y
void ycbcr_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   static int i;
   static int size;
   size=w*h;
   for(i=0;i<size;i++) dstY[i]=src[i*3];
}

// s505 (yyuv per line) to y
void s505_to_y(int w, int h, const unsigned char* src, unsigned char* dstY) {
   // temp
   static int i,j;
   for(i=0;i<h;i+=2) {
      memcpy(dstY+i*w,src+i*3/2*w,w);
      memcpy(dstY+(i+1)*w,src+(i+1)*3/2*w,w);
   }
   for(i=0;i<h*w;i++)
      dstY[i]-=128;
}

//
// FROM YUV444
//

// yuv444 planar to rgb24
void yuv444_to_rgb24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst){
   static int p,size;
   static double Y,U,V;
   size=h*w;
   for(p=0;p<size;p++) {
      Y=srcY[p];
      U=srcU[p]-128;
      V=srcV[p]-128;
      dst[p*3]=clip(Y+1.402*V);
      dst[p*3+1]=clip(Y-0.344*U-0.714*V);
      dst[p*3+2]=clip(Y+1.772*U);
   }
}

// yuv444 planar to bgr24
void yuv444_to_bgr24(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst){
   static int p,size;
   static double Y,U,V;
   size=h*w;
   for(p=0;p<size;p++) {
      Y=srcY[p];
      U=srcU[p]-128;
      V=srcV[p]-128;
      dst[p*3+2]=clip(Y+1.402*V);
      dst[p*3+1]=clip(Y-0.344*U-0.714*V);
      dst[p*3]=clip(Y+1.772*U);
   }
}

// yuv444 planar to rgb32
void yuv444_to_bgr32(int w, int h, const unsigned char* srcY, const unsigned char* srcU, const unsigned char* srcV, unsigned char* dst){
   static int p,size;
   static double Y,U,V;
   size=h*w;
   for(p=0;p<size;p++) {
      Y=srcY[p];
      U=srcU[p]-128;
      V=srcV[p]-128;
      dst[p*4]=clip(Y+1.772*U);
      dst[p*4+1]=clip(Y-0.344*U-0.714*V);
      dst[p*4+2]=clip(Y+1.402*V);
      dst[p*4+3]=0;
   }
}

//
// FROM Y only
//

// y to yuyv
void y_to_yuyv(int w, int h, const unsigned char* src, unsigned char* dst) {
   static int i;
   static int size;
   size=w*h/2;
   for(i=0;i<size;i++) {
      dst[i*4]=src[i*2];
      dst[i*4+1]=128;
      dst[i*4+2]=src[i*2+1];
      dst[i*4+3]=128;
   }
}

//
// TRANSFORMS
//

// swap 8 bit grey (upside down)
void grey_vertical_swap(int w, int h, unsigned char* data){
   unsigned char* tmp;
   static int i;
   tmp=(unsigned char*)malloc(w*h);
   memcpy(tmp,data,w*h);
   for(i=0;i<h;i++) {
      memcpy(&data[i*w],&tmp[(h-i-1)*w],w);
   }
   free(tmp);
}

// swap rgb24 (upside down)
void rgb24_vertical_swap(int w, int h, unsigned char* data){
   unsigned char* tmp;
   static int i;
   tmp=(unsigned char*)malloc(w*h*3);
   memcpy(tmp,data,w*h*3);
   for(i=0;i<h;i++) {
      memcpy(&data[i*w*3],&tmp[(h-i-1)*w*3],w*3);
   }
   free(tmp);
}

// 8 bits luminance plan substraction A=A-B
void lum_plan_sub(int w, int h,unsigned char* A, const unsigned char* B) {
   static int size;
   size=w*h;
   for(int i=0;i<size;i++)
      A[i]=clip((int)A[i]-(int)B[i]);
}

// 8 bits color (U or V) plan substraction A=A-B
void color_plan_sub(int w, int h,unsigned char* A, const unsigned char* B) {
   static int size;
   size=w*h;
   for(int i=0;i<size;i++)
      A[i]=clip((int)A[i]-(int)B[i]+128);
}

// 8 bits luminance plan division A=A/B
void lum_plan_div(int w, int h,int max, unsigned char* A, const unsigned char* B) {
   static int size;
   size=w*h;
   for(int i=0;i<size;i++) {
      if(B[i]!=0)
         A[i]=clip((int)A[i]*max/(int)B[i]);
      else
         A[i]=255;
   }
}

// 8 bits color (U or V) plan division A=A/B
void color_plan_div(int w, int h, int max, unsigned char* A, const unsigned char* B) {
   static int size;
   size=w*h;
   for(int i=0;i<size;i++) {
      if(B[i]!=0)
         A[i]=clip(((int)A[i]-128)*max/(int)B[i]+128);
      else
         A[i]=255;
   }
}
