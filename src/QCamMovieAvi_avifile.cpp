/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2013 Blaise-Florentin Collin

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


// only available if have libavifile
#if HAVE_AVIFILE_H

#include <stdlib.h>

#include "yuv.hpp"

#include "QCamMovieAvi_avifile.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"
#include "SettingsBackup.hpp"

#include <videoencoder.h>
#include <image.h>
#include <avm_fourcc.h>
#include <avm_cpuinfo.h>
#include <utils.h>
#include <avm_creators.h>
#include <avm_except.h>

// settings object, needed everywhere
extern settingsBackup settings;

QCamMovieAvi::QCamMovieAvi() {
   // init
   aviFile_ = 0;
   aviStream_ = 0;
   deinterlaceBuf_ =NULL;

   // should the movie be registax compatible ?
   if(settings.haveKey("REGISTAX_AVI"))
      registaxCompatibility=(string(settings.getKey("REGISTAX_AVI"))=="yes");
   else
      registaxCompatibility=false;
}

QCamMovieAvi::~QCamMovieAvi() {
   // free buffer if needed
   free(deinterlaceBuf_);
}

QWidget * QCamMovieAvi::buildGUI(QWidget  * father) {
   // nothing to do yet
   return father;
}

bool QCamMovieAvi::openImpl(const string & seqName, const QCam & cam) {
   cam.writeProperties(seqName+".properties");
   aviFile_ = avm::CreateWriteFile((seqName+".avi").c_str());

   // build header for grey frames (we must add a palette)
   if((cam.yuvFrame().getMode()==GreyFrame)&&!(registaxCompatibility)) {
      BITMAPINFO* bi;
      bi=(BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER)+256*4);
      memset(bi, 0, sizeof(BITMAPINFOHEADER)+256*4);
      bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER)+256*4;
      bi->bmiHeader.biWidth = cam.size().width();
      bi->bmiHeader.biHeight = cam.size().height();
      bi->bmiHeader.biSizeImage =bi->bmiHeader.biWidth*bi->bmiHeader.biHeight;
      bi->bmiHeader.biPlanes = 1;
      bi->bmiHeader.biBitCount = 8;
      bi->bmiHeader.biCompression = BI_RGB;
      // build the palette
      for(int i=0;i<256;i++)
         bi->bmiColors[i]=(i << 16) + (i << 8) + i;

      // get the framerate
      int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());
      if (frameRate<1) frameRate=1;
      // build the stream
      aviStream_ = aviFile_->AddStream(AviStream::Video, bi, sizeof(BITMAPINFOHEADER)+256*4, BI_RGB, 1000*1000/frameRate);
      deinterlaceBuf_ =(unsigned char*)malloc(bi->bmiHeader.biSizeImage*sizeof(unsigned char));
      // release the header
      free(bi);
   // build header for rgb24
   } else {
      BITMAPINFOHEADER bi;
      memset(&bi, 0, sizeof(bi));
      bi.biSize = sizeof(bi);
      bi.biWidth = cam.size().width();
      bi.biHeight = cam.size().height();
      bi.biSizeImage =bi.biWidth*bi.biHeight*3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;
      bi.biCompression = BI_RGB;

      // get the framerate
      int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());
      if (frameRate<1) frameRate=1;
      // build the stream
      aviStream_ = aviFile_->AddStream(AviStream::Video, &bi, sizeof(bi),BI_RGB,1000*1000/frameRate);
      // alloc the frame buffer
      deinterlaceBuf_ =(unsigned char*)malloc(bi.biSizeImage*sizeof(unsigned char));
   }

   cam.writeProperties(seqName+".properties");
   return true;
}

void QCamMovieAvi::closeImpl() {
   // free all
   if(aviStream_) {
      delete aviFile_;
      aviFile_ = NULL;
      aviStream_ = NULL;
      free(deinterlaceBuf_);
      deinterlaceBuf_ =NULL;
   }
}

bool QCamMovieAvi::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   // build the header
   BITMAPINFOHEADER bi;
   memset(&bi, 0, sizeof(bi));
   bi.biSize = sizeof(bi);
   bi.biWidth = newFrame.size().width();
   bi.biHeight = newFrame.size().height();
   // grey frame
   if((newFrame.getMode()==GreyFrame)&&(!registaxCompatibility)) {
      bi.biSizeImage =bi.biWidth*bi.biHeight;
      bi.biPlanes = 1;
      bi.biBitCount = 8;
      bi.biCompression = BI_RGB;
   // rgb24
   } else {
      bi.biSizeImage =bi.biWidth*bi.biHeight*3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;
      bi.biCompression = BI_RGB;
   }

   // transform frames
   if((newFrame.getMode()==GreyFrame)&&(!registaxCompatibility)) {
      memcpy(deinterlaceBuf_,newFrame.Y(),bi.biSizeImage);
      grey_vertical_swap(bi.biWidth,bi.biHeight,deinterlaceBuf_);
   } else {
      // if registax compatible, grey frame stored as raw rgb
      if(registaxCompatibility) {
         y_to_bgr24(bi.biWidth,bi.biHeight,newFrame.Y(),deinterlaceBuf_);
      } else {
      // else yuv to raw rgb
         yuv444_to_bgr24(bi.biWidth,bi.biHeight,newFrame.Y(),newFrame.U(),newFrame.V(),deinterlaceBuf_);
      }
      rgb24_vertical_swap(bi.biWidth,bi.biHeight,deinterlaceBuf_);
   }
   // add the frame
   aviStream_->AddChunk(deinterlaceBuf_,bi.biSizeImage,1);
   return true;
}

#endif /* HAVE_AVIFILE_H */
