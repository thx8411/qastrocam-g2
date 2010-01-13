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


#ifndef _QCamFrame_hpp_
#define _QCamFrame_hpp_

#include <stdlib.h>

#include <qsize.h>

#include <iostream>
#include <assert.h>
#include <string>
#include <map>

class QImage;

using namespace std;

enum ImageMode {
   GreyFrame,
   /* GR
      BG */
   YuvFrame,
   RawRgbFrame1,
   /* RG
      GB */
   RawRgbFrame2,
   /* BG
      GR */
   RawRgbFrame3,
   /* GB
      RG */
   RawRgbFrame4,
};

/*** to not be used */
class QCamFrameCommon {
protected:
   QCamFrameCommon(ImageMode mode): mode_(mode) {
      init();
   };
   QCamFrameCommon * clone();

   int nbRef() const { return nbRef_;};
   void incRef() {
      assert(this);
      ++nbRef_;
   }

   void decRef();

   const unsigned char * Y() const { return yFrame_;}
   const unsigned char * U() const { return (mode_!=YuvFrame)?UVGreyBuff():uFrame_;}
   const unsigned char * V() const { return (mode_!=YuvFrame)?UVGreyBuff():vFrame_;}

   unsigned char * Y() { return yFrame_;}
   unsigned char * U() { return (mode_!=YuvFrame)?UVGreyBuff():uFrame_;}
   unsigned char * V() { return (mode_!=YuvFrame)?UVGreyBuff():vFrame_;}

   int ySize() const { return ySize_;}
   int uSize() const { return uSize_;}
   int vSize() const { return vSize_;}

   const unsigned char * YLine(int line) const {
      return yFrame_+line*size_.width();
   }
   const unsigned char * ULine(int line) const {
      return (mode_!=YuvFrame)?UVGreyBuff():(uFrame_+line*size_.width());
   }
   const unsigned char * VLine(int line) const {
      return (mode_!=YuvFrame)?UVGreyBuff():(vFrame_+line*size_.width());
   }
   unsigned char * YLine(int line) {
      return yFrame_+line*size_.width();
   }
   unsigned char * ULine(int line) {
      return (mode_!=YuvFrame)?UVGreyBuff():(uFrame_+line*size_.width());
   }
   unsigned char * VLine(int line) {
      return (mode_!=YuvFrame)?UVGreyBuff():(vFrame_+line*size_.width());
   }
   void clear();
   bool empty() const;
   void setSize(QSize s);
   const QSize & size() { return size_;}
   const QImage & colorImage() const;
   const QImage & grayImage() const;
   const QImage & grayImageNegated() const;
   const QImage & falseColorImage() const;
   void clearCache();
   void copy(const QCamFrameCommon & src,
             int srcX1,int srcY1,
             int srcX2,int srcY2,
             int dstX,int dstY,
             bool mirrorX,bool mirrorY);
   void move(int srcX1,int srcY1,
             int srcX2,int srcY2,
             int dstX,int dstY);
   // binning source frame using x and y factors
   void binning(const QCamFrameCommon & src,int xFactor, int yFactor);
   void setMode(ImageMode val) { mode_=val; allocBuff();}
   ImageMode getMode() const { return mode_;}
   // debayer the frame using the known mode
   void debayer();

   friend class QCamFrame;
   void rotate(int center_x, int center_y, double angle);
   void rotatePI(int center_x,int center_y);
   const string & getProperty(const string & propName) const;
private:
   void hShear(int liney, double radang);
   void vShear(int linex, double radang);
   void rasterOpV(int   x, int   w, int   vshift);
   void rasterOpH(int   y, int   h, int   hshift);
   const unsigned char * UVGreyBuff() const;
   unsigned char * UVGreyBuff();
   ~QCamFrameCommon();
   QCamFrameCommon(const QCamFrameCommon&);
   QCamFrameCommon & operator=(const QCamFrameCommon&);
   void init() {
      nbRef_=0;
      yFrame_=uFrame_=vFrame_=NULL;
      size_=QSize(0,0);
      ySize_=uSize_=vSize_=0;
      colorImage_=grayImage_=NULL;
      colorImageBuff_=NULL;
   }
   void allocBuff();
   ImageMode mode_;
   int nbRef_;
   unsigned char *yFrame_;
   unsigned char *uFrame_;
   unsigned char *vFrame_;
   QSize size_;
   int ySize_;
   int uSize_;
   int vSize_;
   mutable QImage * colorImage_;
   mutable uchar * colorImageBuff_;
   mutable QImage * grayImage_;
   mutable map<string,string> properties_;
};

class QCamFrame {
public:
   QCamFrame(ImageMode mode=YuvFrame);
   QCamFrame(const QCamFrame& other);
   QCamFrame & operator=(const QCamFrame & other);
   ~QCamFrame();
   const QCamFrameCommon * getCommon() const { return common_; }
   QCamFrameCommon * setCommon();
   const QSize & size() const { return common_->size_;}
   int ySize() const { return getCommon()->ySize_;}
   int uSize() const { return getCommon()->uSize_;}
   int vSize() const { return getCommon()->vSize_;}
   const unsigned char * Y() const { return getCommon()->Y();}
   const unsigned char * U() const { return getCommon()->U();}
   const unsigned char * V() const { return getCommon()->V();}
   const unsigned char * YLine(int line) const
      { return getCommon()->YLine(line);}
   const unsigned char * ULine(int line) const
      { return getCommon()->ULine(line);}
   const unsigned char * VLine(int line) const
      { return getCommon()->VLine(line);}
   unsigned char * YforUpdate() { return setCommon()->Y();}
   unsigned char * UforUpdate() { return setCommon()->U();}
   unsigned char * VforUpdate() { return setCommon()->V();}
   unsigned char * YforOverwrite();
   unsigned char * UforOverwrite();
   unsigned char * VforOverwrite();
   unsigned char * YLineForUpdate(int line)
      { return setCommon()->YLine(line);}
   unsigned char * ULineForUpdate(int line)
      { return setCommon()->ULine(line);}
   unsigned char * VLineForUpdate(int line)
      { return setCommon()->VLine(line);}
   void setSize(const QSize & s) { if (getCommon()->size_ != s) setCommon()->setSize(s) ;}
   bool empty() const { return getCommon()->empty(); }
   void clear() { setCommon()->clear();}
   const QImage & colorImage() const { return getCommon()->colorImage();}
   const QImage & grayImage() const { return getCommon()->grayImage();}
   const QImage & grayImageNegated() const { return getCommon()->grayImageNegated();}
   const QImage & falseColorImage() const { return getCommon()->falseColorImage();}
   void copy(const QCamFrame & src,
             int srcX1,int srcY1,
             int srcX2,int srcY2,
             int dstX,int dstY,
             bool mirrorX=false,bool mirrorY=false);
   void setMode(ImageMode val) { if (getCommon()->getMode() != val) setCommon()->setMode(val);}
   ImageMode getMode() const { return getCommon()->getMode();}
   void rotate(int centerX,int centerY,double angle) {if (angle !=0.0) setCommon()->rotate(centerX,centerY,angle);}
   const string & getProperty(const string & propName) const { return getCommon()->getProperty(propName);}
   void exportProperties(map<string,string> & dest) const;
   void setAllProperies(const map<string,string> & src) const;

   // cropping the frame l,t = start point
   void cropping(const QCamFrame & src, int l, int t, int w, int h);
   // binning the frame to the new given size
   void binning(const QCamFrame & src, int w, int h);
   // debayer the frame using the known mode
   void debayer();
   // is the frame max luminance bigger than the black level ?
   bool isValide(int level);
private:
   QCamFrameCommon* common_;
};

#endif
