/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <assert.h>
#include <math.h>

#include <Qt/qimage.h>

#include "QCamFrame.hpp"
#include "yuv.hpp"
#include "bayer.hpp"

using namespace std;

QCamFrameCommon::~QCamFrameCommon() {
   free(yFrame_);
   free(uFrame_);
   free(vFrame_);
   clearCache();
}

void QCamFrameCommon::clear() {
   memset(yFrame_,0,size_.width()*size_.height());
   if (getMode()==YuvFrame) {
      memset(uFrame_,128,(size_.width())*(size_.height()));
      memset(vFrame_,128,(size_.width())*(size_.height()));
   }
}

bool QCamFrameCommon::empty() const {
   return size_.width()<=0 || size_.height()<=0;
}

QCamFrameCommon* QCamFrameCommon::clone() {
   QCamFrameCommon* newFrame=new QCamFrameCommon(getMode());
   newFrame->setSize(size_);
   memcpy(newFrame->yFrame_,yFrame_,ySize());
   if (getMode()==YuvFrame) {
      memcpy(newFrame->uFrame_,uFrame_,uSize());
      memcpy(newFrame->vFrame_,vFrame_,vSize());
   }
   return newFrame;
}

void QCamFrameCommon::decRef() {
   --nbRef_;
}

void QCamFrameCommon::allocBuff() {
   free(yFrame_);
   free(uFrame_);
   free(vFrame_);

   ySize_=size_.width()*size_.height();
   uSize_=vSize_=(getMode()!=YuvFrame)?0:(size_.width()*size_.height());
   yFrame_=(unsigned char*)malloc(ySize());
   uFrame_=(getMode()!=YuvFrame)?NULL:(unsigned char*)malloc(uSize());
   vFrame_=(getMode()!=YuvFrame)?NULL:(unsigned char*)malloc(vSize());
}

void QCamFrameCommon::setSize(QSize s) {
   assert(nbRef_<=1);
   if (!s.isValid()) {
      return;
   }
   /* need multiple of 4 */
   int x=s.width();
   int y=s.height();
   if (x%4) {x+=(4-x%4);}
   if (y%4) {y+=(4-y%4);}
   s=QSize(x,y);
   if (size_ != s) {
      size_=s;
      allocBuff();
   }
}

void QCamFrameCommon::clearCache() {
   delete colorImage_;
   colorImage_=NULL;
   delete grayImage_;
   grayImage_=NULL;
   free(colorImageBuff_);
   colorImageBuff_=NULL;
}

const QImage & QCamFrameCommon::colorImage() const {
   switch (getMode()) {
   case GreyFrame:
      return grayImage();
   case YuvFrame:
      if (!colorImage_) {
         colorImageBuff_ =(unsigned char*)malloc(size_.width()*size_.height()*4);
         colorImage_=new QImage(colorImageBuff_,size_.width(),size_.height(),size_.width()*4,QImage::Format_RGB32);
      }
      yuv444_to_bgr32(size_.width(),size_.height(),Y(),U(),V(),(unsigned char*)colorImageBuff_);
      break;
   }
   return *colorImage_;
}

const QImage & QCamFrameCommon::grayImage() const {
   if (!grayImage_) {
      static QVector<QRgb>* grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QVector<QRgb>;
         for (int i=0;i<256;++i) {
            grayTable->insert(i,qRgb(i,i,i));
         }
      }
      grayImage_=new QImage((uchar *)(Y()),size_.width(),size_.height(),size_.width(),QImage::Format_Indexed8);
      grayImage_->setColorTable(*grayTable);
   }
   return *grayImage_;
}

const QImage & QCamFrameCommon::grayImageNegated() const {
   if (!grayImage_) {
      static QVector<QRgb>* grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QVector<QRgb>(256);
         for (int i=0;i<256;++i) {
            grayTable->insert(255-i,qRgb(i,i,i));
         }
      }
      grayImage_=new QImage(const_cast<uchar *>(Y()),size_.width(),size_.height(),size_.width(),QImage::Format_Indexed8);
      grayImage_->setColorTable(*grayTable);
   }
   return *grayImage_;
}

const QImage & QCamFrameCommon::falseColorImage() const {
   if (!grayImage_) {
      static QVector<QRgb>* grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QVector<QRgb>(256);

         for (int i=0;i<256;i+=4) {
            grayTable->insert(i/4,qRgb(0,i,255));
         }
         for (int i=0;i<256;i+=4) {
            grayTable->insert(i/4+64,qRgb(0,255,255-i));
         }
         for (int i=0;i<256;i+=4) {
            grayTable->insert(i/4+128,qRgb(i,255,0));
         }
         for (int i=0;i<256;i+=4) {
            grayTable->insert(i/4+192,qRgb(255,255-i,0));
         }
      }
      grayImage_=new QImage(const_cast<uchar *>(Y()),size_.width(),size_.height(),size_.width(),QImage::Format_Indexed8);
      grayImage_->setColorTable(*grayTable);
   }
   return *grayImage_;
}

static void memswap(unsigned char *dest,const unsigned char *src, size_t n) {
   unsigned i=0,j=n-1;
   while (i<n) {
      dest[i++]=src[j--];
   }
}

void QCamFrameCommon::copy(const QCamFrameCommon & src,
                           int srcX1,int srcY1,
                           int srcX2,int srcY2,
                           int dstX,int dstY,
                           bool swapLeftRight,bool swapUpDown) {

   if (getMode() != src.getMode()) {
      //cerr << "not same mode for frame copy\n";
      return;
   }
   bool colorMode=(getMode()==YuvFrame && src.getMode()==YuvFrame);
   switch (getMode()) {
   case GreyFrame:
   case YuvFrame:
      break;
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      if (srcX1%2 != dstX%2) ++dstX;
      if (srcY1%2 != dstY%2) ++dstY;
      break;
   }

   if (dstX>=size_.width()
       || dstX+(srcX2-srcX1+1) <0
       || dstY>=size_.height()
       || dstY+(srcY2-srcY1+1) <0 ) {
      return;
   }
   if (srcX2>src.size_.width()-1) {
      srcX2=src.size_.width()-1;
   }
   if (srcY2>src.size_.height()-1) {
      srcY2=src.size_.height()-1;
   }
   if (dstX<0) {
      srcX1-=dstX;
      dstX=0;
   }
   if (dstY<0) {
      srcY1-=dstY;
      dstY=0;
   }
   if (srcX1<0) {
      dstX-=srcX1;
      srcX1=0;
   }
   if (srcY1<0) {
      dstY-=srcY1;
      srcY1=0;
   }
   if (srcX1>srcX2) {
      return;
   }
   if (srcY1>srcY2) {
      return;
   }
   int lineSize=srcX2-srcX1+1;
   int zoneHeight=srcY2-srcY1+1;
   if (dstX+lineSize>=size_.width()) {
      lineSize=size_.width()-dstX;
   }
   if (dstY+zoneHeight>=size_.height()) {
      zoneHeight=size_.height()-dstY;
   }
   srcY2=srcY1+zoneHeight-1;

   int jinc,jref;
   if (swapUpDown) {
      jinc=-1;
      jref=dstY+zoneHeight-1;
   } else {
      jinc=1;
      jref=dstY;
   }

   for (int j=srcY1;j<=srcY2;
        ++j) {
      if (!swapLeftRight) {
         memcpy(YLine(jref)+dstX,
                src.YLine(j)+srcX1,lineSize);
         if (colorMode) {
            memcpy(ULine(jref)+dstX,
                   src.ULine(j)+srcX1,lineSize);
            memcpy(VLine(jref)+dstX,
                   src.VLine(j)+srcX1,lineSize);
         }
         jref+=jinc;
      } else {
         memswap(YLine(jref)+dstX,
                src.YLine(j)+srcX1,lineSize);
         if (colorMode) {
            memswap(ULine(jref)+dstX,
                   src.ULine(j)+srcX1,lineSize);
            memswap(VLine(jref)+dstX,
                   src.VLine(j)+srcX1,lineSize);
         }
         jref+=jinc;
      }
   }

   if (swapLeftRight) {
      switch (getMode()) {
      case RawRgbFrame1:
         mode_=RawRgbFrame2;
         break;
      case RawRgbFrame2:
         mode_=RawRgbFrame1;
         break;
      case RawRgbFrame3:
         mode_=RawRgbFrame4;
         break;
      case RawRgbFrame4:
         mode_=RawRgbFrame3;
         break;
      default:
         // mode is preserved
         break;
      }
   }

   if (swapUpDown) {
      switch (getMode()) {
      case RawRgbFrame1:
         mode_=RawRgbFrame3;
         break;
      case RawRgbFrame2:
         mode_=RawRgbFrame4;
         break;
      case RawRgbFrame3:
         mode_=RawRgbFrame1;
         break;
      case RawRgbFrame4:
         mode_=RawRgbFrame2;
         break;
      default:
         // mode is preserved
         break;
      }
   }
}

void QCamFrameCommon::move(int srcX1,int srcY1,
                           int srcX2,int srcY2,
                           int dstX,int dstY) {
   bool colorMode=getMode()==YuvFrame;
   if (dstX>=size_.width()
       || dstX+(srcX2-srcX1+1) <0
       || dstY>=size_.height()
       || dstY+(srcY2-srcY1+1) <0 ) {
      return;
   }
   if (srcX2>size_.width()-1) {
      srcX2=size_.width()-1;
   }
   if (srcY2>size_.height()-1) {
      srcY2=size_.height()-1;
   }
   if (dstX<0) {
      srcX1-=dstX;
      dstX=0;
   }
   if (dstY<0) {
      srcY1-=dstY;
      dstY=0;
   }
   if (srcX1<0) {
      dstX-=srcX1;
      srcX1=0;
   }
   if (srcY1<0) {
      dstY-=srcY1;
      srcY1=0;
   }
   if (srcX1>srcX2) {
      return;
   }
   if (srcY1>srcY2) {
      return;
   }
   int lineSize=srcX2-srcX1+1;
   int zoneHeight=srcY2-srcY1+1;
   if (dstX+lineSize>=size_.width()) {
      lineSize=size_.width()-dstX;
   }
   if (dstY+zoneHeight>=size_.height()) {
      zoneHeight=size_.height()-dstY;
   }
   srcY2=srcY1+zoneHeight-1;
   int jref;
   if (dstY<=srcY1) {
      jref=dstY;
      for (int j=srcY1;j<=srcY2;
           ++j) {
         memmove(YLine(jref)+dstX,
                 YLine(j)+srcX1,lineSize);
         if (colorMode) {
            memmove(ULine(jref)+dstX,
                    ULine(j)+srcX1,lineSize);
            memmove(VLine(jref)+dstX,
                    VLine(j)+srcX1,lineSize);
         }
         ++jref;
      }
   } else {
      jref=dstY+zoneHeight-1;
      for (int j=srcY2;j>=srcY1;
           --j) {
         memmove(YLine(jref)+dstX,
                 YLine(j)+srcX1,lineSize);
         if (colorMode) {
            memmove(ULine(jref)+dstX,
                    ULine(j)+srcX1,lineSize);
            memmove(VLine(jref)+dstX,
                    VLine(j)+srcX1,lineSize);
         }
         --jref;
      }
   }
}

// frame binning
void QCamFrameCommon::binning(const QCamFrameCommon & src, int xFactor, int yFactor) {
   int i,j,k,l,Ysum,Usum,Vsum;
   int xOffset,yOffset;

   allocBuff();
   // luminance plan
   // for each dest. pixel
   for(i=0;i<size_.width();i++) {
      xOffset=i*xFactor;
      for(j=0;j<size_.height();j++) {
         // we sum source pixels arround
         Ysum=0;
         Usum=0;
         Vsum=0;
         yOffset=j*yFactor;
         // lines
         for(k=0;k<yFactor;k++) {
            const unsigned char* Yline=src.YLine(yOffset+k);
            const unsigned char* Uline=src.ULine(yOffset+k);
            const unsigned char* Vline=src.VLine(yOffset+k);
            // rows
            for(l=0;l<xFactor;l++) {
               Ysum+=Yline[xOffset+l];
               if(getMode()==YuvFrame) {
                  Usum+=Uline[xOffset+l];
                  Vsum+=Vline[xOffset+l];
               }
            }
         }
         // outside dynamic range
         if(Ysum>255)
            Ysum=255;
         // update dest pixel
         yFrame_[j*size_.width()+i]=(unsigned char)Ysum;
         if(getMode()==YuvFrame) {
            // outside dynamic range
            Usum-=(xFactor*yFactor-1)*128;
            if(Usum>255)
               Usum=255;
            if(Usum<0)
               Usum=0;
            Vsum-=(xFactor*yFactor-1)*128;
            if(Vsum>255)
               Vsum=255;
            if(Vsum<0)
               Vsum=0;
            uFrame_[j*size_.width()+i]=(unsigned char)Usum;
            vFrame_[j*size_.width()+i]=(unsigned char)Vsum;
         }
      }
   }
}

// bilinear frame scaling
void QCamFrameCommon::scaling(const QCamFrameCommon & src, int xFactor, int yFactor) {
int i,j,k,l,Ysum,Usum,Vsum;
   int xOffset,yOffset;

   allocBuff();
   // luminance plan
   // for each dest. pixel
   for(i=0;i<size_.width();i++) {
      xOffset=i*xFactor;
      for(j=0;j<size_.height();j++) {
         // we sum source pixels arround
         Ysum=0;
         Usum=0;
         Vsum=0;
         yOffset=j*yFactor;
         // lines
         for(k=0;k<yFactor;k++) {
            const unsigned char* Yline=src.YLine(yOffset+k);
            const unsigned char* Uline=src.ULine(yOffset+k);
            const unsigned char* Vline=src.VLine(yOffset+k);
            // rows
            for(l=0;l<xFactor;l++) {
               Ysum+=Yline[xOffset+l];
               if(getMode()==YuvFrame) {
                  Usum+=Uline[xOffset+l];
                  Vsum+=Vline[xOffset+l];
               }
            }
         }
         // outside dynamic range
         Ysum/=xFactor*yFactor;
         if(Ysum>255)
            Ysum=255;
         // update dest pixel
         yFrame_[j*size_.width()+i]=(unsigned char)Ysum;
         // handle colors
         if(getMode()==YuvFrame) {
            // outside dynamic range
            Usum-=(xFactor*yFactor)*128;
            Usum/=xFactor*yFactor;
            Usum+=128;
            if(Usum>255)
               Usum=255;
            if(Usum<0)
               Usum=0;
            // outside dynamic range
            Vsum-=(xFactor*yFactor)*128;
            Vsum/=xFactor*yFactor;
            Vsum+=128;
            if(Vsum>255)
               Vsum=255;
            if(Vsum<0)
               Vsum=0;
            uFrame_[j*size_.width()+i]=(unsigned char)Usum;
            vFrame_[j*size_.width()+i]=(unsigned char)Vsum;
         }
      }
   }
}

// debayer the frame
void QCamFrameCommon::debayer(const QCamFrameCommon & src, ImageMode mode, DebayerMethod method) {
   unsigned char* yTemp=src.Yfree();
   int w=src.sizeXfree();
   int h=src.sizeYfree();
   switch(method) {
      case Luminance :
         setMode(GreyFrame);
         setSize(QSize(w,h));
         raw2luminance(yFrame_,yTemp,size().width(),size().height(),mode);
      case GreenOnly :
         setMode(GreyFrame);
         setSize(QSize(w,h));
         raw2green(yFrame_,yTemp,size().width(),size().height(),mode);
         break;
      case RedOnly :
         setMode(GreyFrame);
         setSize(QSize(w,h));
         raw2red(yFrame_,yTemp,size().width(),size().height(),mode);
         break;
      case BlueOnly :
         setMode(GreyFrame);
         setSize(QSize(w,h));
         raw2blue(yFrame_,yTemp,size().width(),size().height(),mode);
         break;
      case Nearest :
         setMode(YuvFrame);
         setSize(QSize(w,h));
         raw2yuv444_nearest(yFrame_,uFrame_,vFrame_,yTemp,size().width(),size().height(),mode);
         break;
      case Bilinear :
         setMode(YuvFrame);
         setSize(QSize(w,h));
         raw2yuv444_bilinear(yFrame_,uFrame_,vFrame_,yTemp,size().width(),size().height(),mode);
         break;
   }
}

// apply dark frame
void QCamFrameCommon::applyDark(const QCamFrameCommon* dark, double timeFactor=1.0) {
   lum_plan_sub(size().width(),size().height(),yFrame_,dark->Y(), timeFactor);
   if(getMode()==YuvFrame) {
      color_plan_sub(size().width(),size().height(),uFrame_,dark->U(), timeFactor);
      color_plan_sub(size().width(),size().height(),vFrame_,dark->V(), timeFactor);
   }
}

// apply flat frame
void QCamFrameCommon::applyFlat(const QCamFrameCommon* flat, int max) {
   lum_plan_div(size().width(),size().height(),max,yFrame_,flat->Y());
   if(getMode()==YuvFrame) {
      color_plan_div(size().width(),size().height(),max,uFrame_,flat->U());
      color_plan_div(size().width(),size().height(),max,vFrame_,flat->V());
   }
}

// get max value
unsigned char QCamFrameCommon::getMax() {
   unsigned char max=0;
   for(int i=0;i<size().width();i++)
      for(int j=0;j<size().height();j++)
         if(yFrame_[j*size().width()+i]>max) {
            max=yFrame_[j*size().width()+i];
            maxX=i;
            maxY=j;
         }
   return(max);
}

int QCamFrameCommon::brightestX() {
   if(maxX<0)
      getMax();
   return(maxX);
}

int QCamFrameCommon::brightestY() {
   if(maxY<0)
      getMax();
   return(maxY);
}

QCamFrame::QCamFrame(ImageMode mode) {
   common_=new QCamFrameCommon(mode);
   common_->incRef();
}

QCamFrame::QCamFrame(const QCamFrame& other) {
   other.common_->incRef();
   common_ = other.common_;
}

QCamFrame & QCamFrame::operator=(const QCamFrame & other) {
   common_->decRef();
   if(common_->nbRef()==0)
      delete common_;
   other.common_->incRef();
   common_=other.common_;
   return *this;
}

QCamFrame::~QCamFrame() {
   common_->decRef();
   if(common_->nbRef()==0)
      delete common_;
}

QCamFrameCommon * QCamFrame::setCommon() {
   if (common_->nbRef() != 1 ) {
      common_->decRef();
      if(common_->nbRef()==0)
         delete common_;
      QCamFrameCommon * tmpFrame = common_->clone();
      tmpFrame->incRef();
      common_= tmpFrame;
   } else {
      common_->clearCache();
   }
   return common_;
}

void QCamFrame::copy(const QCamFrame & src,
                     int srcX1,int srcY1,
                     int srcX2,int srcY2,
                     int dstX,int dstY,
                     bool mirrorX,bool mirrorY) {
//   if (srcX1==0 && srcY1==0
//       && dstX==0 && dstY==0
//       && srcX2==src.size().width()-1
//       && srcY2==src.size().height()-1
//       && size()==src.size()
//       && !mirrorX
//       && !mirrorY) {
//      *this=src;
//   } else {
      setCommon()->copy(*src.getCommon(),
                        srcX1, srcY1,
                        srcX2, srcY2,
                        dstX, dstY,
                        mirrorX, mirrorY);
//   }
}

/** for modules that don't look if we have a b&w frame */
const unsigned char * QCamFrameCommon::UVGreyBuff() const {
   static unsigned char *emptyBuff=NULL;
   static int size=0;
   if (emptyBuff==NULL || size<ySize() ) {
      delete emptyBuff;
      size=ySize();
      emptyBuff= new unsigned char[size];
      memset(emptyBuff,127,size);
   }
   return emptyBuff;
}

unsigned char * QCamFrameCommon::UVGreyBuff() {
   abort();
   return 0;
}

const string & QCamFrameCommon::getProperty(const string & propName) const {
   static string empty="N/A";
   map<string,string>::const_iterator it=properties_.find(propName);

   if (it != properties_.end()) {
      return it->second;
   } else {
      return empty;
   }
}

void  QCamFrame::exportProperties(map<string,string> & dest) const {
   dest=getCommon()->properties_;
}

void QCamFrame::setAllProperies(const map<string,string> & src) const {
   getCommon()->properties_=src;
}

unsigned char *  QCamFrame::YforOverwrite() {
   if (common_->nbRef() != 1) {
      QCamFrameCommon * newFrame= new QCamFrameCommon(common_->getMode());
      newFrame->incRef();
      newFrame->setSize(common_->size_);
      common_->decRef();
      if(common_->nbRef()==0)
         delete common_;
      common_=newFrame;
   }
   return common_->Y();
}

unsigned char * QCamFrame::UforOverwrite() {
    if (common_->nbRef() != 1) {
      QCamFrameCommon * newFrame= new QCamFrameCommon(common_->getMode());
      newFrame->incRef();
      newFrame->setSize(common_->size_);
      common_->decRef();
      if(common_->nbRef()==0)
         delete common_;
      common_=newFrame;
   }
   return common_->U();
}

unsigned char * QCamFrame::VforOverwrite() {
    if (common_->nbRef() != 1) {
      QCamFrameCommon * newFrame= new QCamFrameCommon(common_->getMode());
      newFrame->incRef();
      newFrame->setSize(common_->size_);
      common_->decRef();
      if(common_->nbRef()==0)
         delete common_;
      common_=newFrame;
   }
   return common_->V();
}

// frame cropping
// l : x start
// t : y start
// w : width
// h : height
void QCamFrame::cropping(const QCamFrame & src, int l, int t, int w, int h) {
   setSize(QSize(w,h));
   setMode(src.getMode());
   copy(src,l,t,w+l-1,h+t-1,0,0,false,false);
}

// frame binning
// w : new target width
// h : new target height
void QCamFrame::binning(const QCamFrame & src, int w, int h) {
   int xFactor, yFactor;

   if((w==0)||(h==0))
      return;
   // compute binning factors
   xFactor=src.size().width()/w;
   yFactor=src.size().height()/h;
   // keep mode and set size
   setSize(QSize(w,h));
   setMode(src.getMode());
   // binning...
   setCommon()->binning(*src.getCommon(),xFactor,yFactor);
}

// bilinear frame scaling
// w : new target width
// h : new target height
void QCamFrame::scaling(const QCamFrame & src, int w, int h) {
   int xFactor, yFactor;

   if((w==0)||(h==0))
      return;
   // compute scaling factors
   xFactor=src.size().width()/w;
   yFactor=src.size().height()/h;
   // keep mode and set size
   setSize(QSize(w,h));
   setMode(src.getMode());
   // scaling...
   setCommon()->scaling(*src.getCommon(),xFactor,yFactor);
}

// debayer the frame
void QCamFrame::debayer(const QCamFrame & src, ImageMode mode, DebayerMethod method) {
   setSize(QSize(0,0));
   setMode((ImageMode)0);
   common_->debayer(*src.getCommon(),mode,method);
}

// apply bias frame
void QCamFrame::applyBias(const QCamFrame & bias) {
   common_->applyDark(bias.getCommon());
}

// apply dark frame
void QCamFrame::applyDark(const QCamFrame & dark, double timeFactor=1.0) {
   common_->applyDark(dark.getCommon(), timeFactor);
}

// apply flat frame
void QCamFrame::applyFlat(const QCamFrame & flat, int max) {
   common_->applyFlat(flat.getCommon(),max);
}

// get max value
unsigned char QCamFrame::getMax() {
   return(common_->getMax());
}

int QCamFrame::brightestX() {
   return(common_->brightestX());
}

int QCamFrame::brightestY() {
   return(common_->brightestY());
}
