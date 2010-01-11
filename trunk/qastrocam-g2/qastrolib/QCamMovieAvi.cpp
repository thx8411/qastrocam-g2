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


#include "../config.h"

#if HAVE_AVIFILE_H

#include <stdlib.h>

#include "yuv.hpp"

#include "QCamMovieAvi.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"

#include <videoencoder.h>
#include <image.h>
#include <avm_fourcc.h>
#include <avm_cpuinfo.h>
#include <utils.h>
#include <avm_creators.h>
#include <avm_except.h>

#define _CODEC_	RIFFINFO_I420

QCamMovieAvi::QCamMovieAvi() {
   aviFile_ = 0;
   aviStream_ = 0;
   deinterlaceBuf_ =NULL;
}

QCamMovieAvi::~QCamMovieAvi() {
   if(deinterlaceBuf_!=NULL)
      free(deinterlaceBuf_);
}

QWidget * QCamMovieAvi::buildGUI(QWidget  * father) {
   return father;
}

bool QCamMovieAvi::openImpl(const string & seqName, const QCam & cam) {
   cam.writeProperties(seqName+".properties");
   aviFile_ = avm::CreateWriteFile((seqName+".avi").c_str());

   BITMAPINFOHEADER bi;
   fourcc_t codec = _CODEC_;

   memset(&bi, 0, sizeof(bi));
   bi.biSize = sizeof(bi);
   bi.biWidth = cam.size().width();
   bi.biHeight = cam.size().height();
   bi.biSizeImage = (bi.biWidth * bi.biHeight * 3) / 2;
   bi.biPlanes = 3;
   bi.biBitCount = 12;
   bi.biCompression = codec;

   int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());
   if (frameRate<1) frameRate=1;
   aviStream_ = aviFile_->AddVideoStream(codec, &bi, 1000*1000/frameRate);
   aviStream_->SetQuality(10000);
   aviStream_->Start();

   deinterlaceBuf_ =(unsigned char*)malloc(bi.biSizeImage*sizeof(unsigned char));

   cam.writeProperties(seqName+".properties");
   return true;
}

void QCamMovieAvi::closeImpl() {
   if(aviStream_) {
      aviStream_->Stop();
      delete aviFile_;
      aviFile_ = 0;
      aviStream_ = 0;
      if(deinterlaceBuf_!=NULL)
         free(deinterlaceBuf_);
      deinterlaceBuf_ =NULL;
   }
}

bool QCamMovieAvi::addImpl(const QCamFrame & newFrame, const QCam & cam) {
   BITMAPINFOHEADER bi;

   fourcc_t codec= _CODEC_;

   memset(&bi, 0, sizeof(bi));
   bi.biSize = sizeof(bi);
   bi.biWidth = newFrame.size().width();
   bi.biHeight = newFrame.size().height();
   bi.biSizeImage = (bi.biWidth * bi.biHeight * 3) / 2;
   bi.biPlanes = 3;
   bi.biBitCount = 12;
   bi.biCompression = codec;

   if(deinterlaceBuf_) {
      yuv444_to_yuv420(bi.biWidth,bi.biHeight,newFrame.Y(),newFrame.U(),newFrame.V(),deinterlaceBuf_,
         deinterlaceBuf_ + (bi.biWidth * bi.biHeight),deinterlaceBuf_ + (bi.biWidth * bi.biHeight) + (bi.biWidth * bi.biHeight) / 4);
      bi.biHeight = - bi.biHeight;
      avm::BitmapInfo info(bi);
      avm::CImage img(&info, deinterlaceBuf_, false);
      aviStream_->AddFrame(&img);
      return true;
   } else {
      cerr << "no AVI buffer allocated." << endl;
      return false;
   }
}

#endif
