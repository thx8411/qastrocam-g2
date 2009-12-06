#include "bayer.hpp"

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
