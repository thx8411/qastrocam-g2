#include "QCamFrame.hpp"
#include <qimage.h>
#include "ccvt.h"
#include "bayer.hpp"
#include <assert.h>
#include <math.h>

using namespace std;

QCamFrameCommon::~QCamFrameCommon() {
   assert(nbRef_==0);
   delete yFrame_; yFrame_=NULL;
   delete uFrame_; uFrame_=NULL;
   delete vFrame_; vFrame_=NULL;
   clearCache();
}

void QCamFrameCommon::clear() {
   memset(yFrame_,0,size_.width()*size_.height());
   if (getMode()==YuvFrame) {
      memset(uFrame_,128,(size_.width()/2)*(size_.height()/2));
      memset(vFrame_,128,(size_.width()/2)*(size_.height()/2));
   }
}

bool QCamFrameCommon::empty() const {
   return size_.width()<=0 || size_.height()<=0;
}

QCamFrameCommon * QCamFrameCommon::clone() {
   QCamFrameCommon * newFrame=new QCamFrameCommon(getMode());
   newFrame->setSize(size_);
   memcpy(newFrame->yFrame_,yFrame_,ySize());
   if (getMode()==YuvFrame) {
      memcpy(newFrame->uFrame_,uFrame_,uSize());
      memcpy(newFrame->vFrame_,vFrame_,vSize());
   }
   return newFrame;
}

void QCamFrameCommon::decRef() {
   assert(this);
   --nbRef_;
   if (nbRef_<=0) {
      delete this;
   }
}

void QCamFrameCommon::allocBuff() {
   delete yFrame_;
   delete uFrame_;
   delete vFrame_;

   ySize_=size_.width()*size_.height();
   uSize_=vSize_=(getMode()!=YuvFrame)?0:((size_.width()/2)*(size_.height()/2));
   yFrame_ = new unsigned char[ySize()];
   uFrame_ = (getMode()!=YuvFrame)?NULL:new unsigned char[uSize()];
   vFrame_ = (getMode()!=YuvFrame)?NULL:new unsigned char[vSize()];
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
   delete colorImage_; colorImage_=NULL;
   delete grayImage_; grayImage_=NULL;
   delete colorImageBuff_; colorImageBuff_=NULL;
}

const QImage & QCamFrameCommon::colorImage() const {
   switch (getMode()) {
   case GreyFrame:
      return grayImage();
   case YuvFrame:
      if (!colorImage_) {
         colorImageBuff_ = new uchar[ size_.width() * size_.height()* 4];
         colorImage_=new QImage(colorImageBuff_,size_.width(),size_.height(),
                                32,0,0,QImage::BigEndian);
      }
      ccvt_420p_bgr32(size_.width(),size_.height(),
                      Y(),U(),V(),(void*)colorImageBuff_);
      break;
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      if (!colorImage_) {
         colorImageBuff_
            =new unsigned char[size_.width()*size_.height()*4];
         memset(colorImageBuff_,0,size_.width() * size_.height()* 4);
         colorImage_=new QImage(colorImageBuff_,
                                size_.width(),size_.height(),
                                32,0,0,QImage::BigEndian);
      }
      raw2rgb(colorImageBuff_,Y(),size_.width(),size_.height(),getMode());
      break;
   }
   return *colorImage_;
}

const QImage & QCamFrameCommon::grayImage() const {
   if (!grayImage_) {
      static QRgb * grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QRgb[256];
         for (int i=0;i<256;++i) {
            grayTable[i]=qRgb(i,i,i);
         }
      }
      grayImage_=new QImage(const_cast<uchar *>(Y()),
                            size_.width(),size_.height(),
                            8,grayTable,256,QImage::BigEndian);
   }
   return *grayImage_;
}

const QImage & QCamFrameCommon::grayImageNegated() const {
   if (!grayImage_) {
      static QRgb * grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QRgb[256];
         for (int i=0;i<256;++i) {
            grayTable[255-i]=qRgb(i,i,i);
         }
      }
      grayImage_=new QImage(const_cast<uchar *>(Y()),
                            size_.width(),size_.height(),
                            8,grayTable,256,QImage::BigEndian);
   }
   return *grayImage_;
}

const QImage & QCamFrameCommon::falseColorImage() const {
   if (!grayImage_) {
      static QRgb * grayTable=NULL;
      if (grayTable == NULL) {
         grayTable=new QRgb[256];

         for (int i=0;i<256;i+=4) {
            grayTable[i/4]=qRgb(0,i,255);
         }
         for (int i=0;i<256;i+=4) {
            grayTable[i/4+64]=qRgb(0,255,255-i);
         }
         for (int i=0;i<256;i+=4) {
            grayTable[i/4+128]=qRgb(i,255,0);
         }
         for (int i=0;i<256;i+=4) {
            grayTable[i/4+192]=qRgb(255,255-i,0);
         }
      }
      grayImage_=new QImage(const_cast<uchar *>(Y()),
                            size_.width(),size_.height(),
                            8,grayTable,256,QImage::BigEndian);
   }
   return *grayImage_;
}

static void memswap(unsigned char *dest,
                    const unsigned char *src, size_t n) {
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
      //cout << "not same mode for frame copy\n";
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
         if (j%2==0 && colorMode) {
            memcpy(ULine(jref)+dstX/2,
                   src.ULine(j)+srcX1/2,lineSize/2);
            memcpy(VLine(jref)+dstX/2,
                   src.VLine(j)+srcX1/2,lineSize/2);
         }
         jref+=jinc;
      } else {
         memswap(YLine(jref)+dstX,
                src.YLine(j)+srcX1,lineSize);
         if (j%2==0 && colorMode) {
            memswap(ULine(jref)+dstX/2,
                   src.ULine(j)+srcX1/2,lineSize/2);
            memswap(VLine(jref)+dstX/2,
                   src.VLine(j)+srcX1/2,lineSize/2);
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
         if (j%2==0 && colorMode) {
            memmove(ULine(jref)+dstX/2,
                    ULine(j)+srcX1/2,lineSize/2);
            memmove(VLine(jref)+dstX/2,
                    VLine(j)+srcX1/2,lineSize/2);
         }
         ++jref;
      }
   } else {
      jref=dstY+zoneHeight-1;
      for (int j=srcY2;j>=srcY1;
           --j) {
         memmove(YLine(jref)+dstX,
                 YLine(j)+srcX1,lineSize);
         if (j%2==0 && colorMode) {
            memmove(ULine(jref)+dstX/2,
                    ULine(j)+srcX1/2,lineSize/2);
            memmove(VLine(jref)+dstX/2,
                    VLine(j)+srcX1/2,lineSize/2);
         }
         --jref;
      }
   }
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
   other.common_->incRef();
   common_=other.common_;
   return *this;
}

QCamFrame::~QCamFrame() {
   common_->decRef();
}

QCamFrameCommon * QCamFrame::setCommon() {
   if (common_->nbRef() != 1 ) {
      common_->decRef();
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
   if (srcX1==0 && srcY1==0
       && dstX==0 && dstY==0
       && srcX2==src.size().width()-1
       && srcY2==src.size().height()-1
       && size()==src.size()
       && !mirrorX
       && !mirrorY) {
      *this=src;
   } else {
      setCommon()->copy(*src.getCommon(),
                        srcX1, srcY1,
                        srcX2, srcY2,
                        dstX, dstY,
                        mirrorX, mirrorY);
   }
}

/** for modules that don't look if we have a b&w frame */
const unsigned char * QCamFrameCommon::UVGreyBuff() const {
   static unsigned char *emptyBuff=NULL;
   static int size=0;
   if (emptyBuff==NULL || size<(ySize()/4) ) {
      emptyBuff= new unsigned char[size=(ySize()/4)];
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
      common_=newFrame;
   }
   return common_->V();
}

bool QCamFrame::isValide(int level) {
   int i;
   int size;
   int value=0;
   const unsigned char* lum;
   lum=Y();
   size=ySize();
   for(i=0;i<size;i++) {
      value+=lum[i];
   }
   value/=size;
   cout << value << endl;
   if(value>level) {
      return(true);
   }
   return(false);
}
