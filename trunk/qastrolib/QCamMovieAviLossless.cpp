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


#include "../config.h"

#if HAVE_AVIFILE_H

#include <stdlib.h>

#include "yuv.hpp"

#include "QCamMovieAviLossless.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"

#include <videoencoder.h>
#include <image.h>
#include <avm_fourcc.h>
#include <avm_cpuinfo.h>
#include <utils.h>
#include <avm_creators.h>
#include <avm_except.h>

QCamMovieAviLossless::QCamMovieAviLossless() {
   // init
   aviFile_ = 0;
   aviStream_ = 0;
   deinterlaceBuf_ =NULL;
}

QCamMovieAviLossless::~QCamMovieAviLossless() {
   // free buffer if needed
   free(deinterlaceBuf_);
}

QWidget * QCamMovieAviLossless::buildGUI(QWidget  * father) {
   // nothing to do at this time
   return father;
}

bool QCamMovieAviLossless::openImpl(const string & seqName, const QCam & cam) {
   cam.writeProperties(seqName+".properties");
   aviFile_ = avm::CreateWriteFile((seqName+".avi").c_str());

   // building header
   BITMAPINFOHEADER bi;
   memset(&bi, 0, sizeof(bi));
   bi.biSize = sizeof(bi);
   bi.biWidth = cam.size().width();
   bi.biHeight = cam.size().height();
   // if grey frame use, yuy2 for smaller resulting file
   if(cam.yuvFrame().getMode()==GreyFrame) {
      bi.biSizeImage =bi.biWidth*bi.biHeight*2;
      bi.biPlanes = 3;
      bi.biBitCount = 16;
      bi.biCompression = RIFFINFO_YUY2;
   // else use rgb24
   } else {
      bi.biSizeImage =bi.biWidth*bi.biHeight*3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;
      bi.biCompression = BI_RGB;
   }

   // get framerate
   int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());
   if (frameRate<1) frameRate=1;

   // build stream
   aviStream_=aviFile_->AddVideoStream(RIFFINFO_HFYU, &bi, 1000*1000/frameRate);
   aviStream_->SetQuality(10000);
   aviStream_->Start();

   // alloc frame buffer
   deinterlaceBuf_ =(unsigned char*)malloc(bi.biSizeImage*sizeof(unsigned char));

   // update properties
   cam.writeProperties(seqName+".properties");
   return true;
}

void QCamMovieAviLossless::closeImpl() {
   if(aviStream_) {
      delete aviFile_;
      aviFile_ = NULL;
      aviStream_ = NULL;
      free(deinterlaceBuf_);
      deinterlaceBuf_ =NULL;
   }
}

bool QCamMovieAviLossless::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   // build header
   BITMAPINFOHEADER bi;
   memset(&bi, 0, sizeof(bi));
   bi.biSize = sizeof(bi);
   bi.biWidth = newFrame.size().width();
   bi.biHeight = newFrame.size().height();
   // if grey frame use, yuy2 for smaller resulting file
   if(newFrame.getMode()==GreyFrame) {
      bi.biSizeImage =bi.biWidth*bi.biHeight*2;
      bi.biPlanes = 3;
      bi.biBitCount = 16;
      bi.biCompression = RIFFINFO_YUY2;
   // else use rgb24
   } else {
      bi.biSizeImage =bi.biWidth*bi.biHeight*3;
      bi.biPlanes = 1;
      bi.biBitCount = 24;
      bi.biCompression = BI_RGB;
   }

   // convert the frame
   if(newFrame.getMode()==GreyFrame) {
      y_to_yuyv(bi.biWidth,bi.biHeight,newFrame.Y(),deinterlaceBuf_);
   } else {
      yuv444_to_bgr24(bi.biWidth,bi.biHeight,newFrame.Y(),newFrame.U(),newFrame.V(),deinterlaceBuf_);
      rgb24_vertical_swap(bi.biWidth,bi.biHeight,deinterlaceBuf_);
   }

   // add the frame
   BitmapInfo info(bi);
   CImage img(&info,deinterlaceBuf_, true);
   aviStream_->AddFrame(&img);
   return true;
}

#endif
