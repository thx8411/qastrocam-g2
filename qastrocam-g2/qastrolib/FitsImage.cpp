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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "FitsImage.hpp"
#include "yuv.hpp"

//#define DEFINC 4
//#define DEFDEC 20

#include "QCamFrame.hpp"

using namespace std;

FitsImageCFITSIO::FitsImageCFITSIO(const string & fileName,bool multipleFrame) :
   FitsImage(fileName),fptr_(NULL),multipleFrame_(multipleFrame) {
   status_=0;
   numAxes_=2;
   axesDim_[0]=axesDim_[1]=axesDim_[2]=1;
   base_[0]=base_[1]=base_[2]=1;
}

FitsImageCFITSIO::~FitsImageCFITSIO() {
   close();
}

bool FitsImageCFITSIO::load(QCamFrame& frame) {
}

bool FitsImageCFITSIO::initFits(const QCamFrame& frame) {
   numAxes_=2;
   axesDim_[0]=frame.size().width();
   axesDim_[1]=frame.size().height();
   axesDim_[2]=1;

   switch (frame.getMode()) {
   case GreyFrame:
      axesDim_[2]=1;
      numAxes_=2;
      break;
   case YuvFrame:
      axesDim_[2]=3;
      numAxes_=3;
      break;
   break;
   }
   fits_create_file(&fptr_,fileName_.c_str(),&status_);
   fits_report_error(stdout,status_);
   fits_create_img(fptr_,BYTE_IMG, numAxes_, axesDim_,&status_);
   fits_report_error(stdout,status_);
}

bool FitsImageCFITSIO::save(const QCamFrame& frame) {

   if (fptr_==NULL)
      initFits(frame);
   switch (numAxes_) {
   case 2:
      fits_write_pix(fptr_,TBYTE, base_,axesDim_[0]*axesDim_[1]*axesDim_[2],(void*)frame.Y(),&status_);
      fits_report_error(stdout,status_);
      break;
   case 3:
      uchar* colorImageBuff = new uchar[axesDim_[0] * axesDim_[1] * 3];
      yuv444_to_rgb24(axesDim_[0],axesDim_[1],frame.Y(),frame.U(),frame.V(),colorImageBuff);
      uchar* colorImagePlane= new uchar[axesDim_[0] * axesDim_[1]];
      for (int i=0;i<3;++i) {
         for(int j=0;j<axesDim_[0] * axesDim_[1];++j) {
            colorImagePlane[j]=colorImageBuff[j*3+i];
         }
         base_[2]=3-i;
         fits_write_pix(fptr_,TBYTE, base_,axesDim_[0]*axesDim_[1],(void*)colorImagePlane,&status_);
         fits_report_error(stdout,status_);
      }
      delete colorImageBuff;
      delete colorImagePlane;
   break;
   }
   return (status_==0);
}

bool FitsImageCFITSIO::close() {
   if (fptr_ != NULL) {
      fits_close_file(fptr_,&status_);
      fits_report_error(stdout,status_);
      fptr_=NULL;
   }
   return (status_==0);
}
