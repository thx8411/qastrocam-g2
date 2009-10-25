#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include "FitsImage.hpp"

#define DEFINC 4
#define DEFDEC 20

#include "QCamFrame.hpp"

using namespace std;

FitsImageCFITSIO::FitsImageCFITSIO(const string & fileName,
                                   bool multipleFrame) :
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

#include "ccvt.h"

bool FitsImageCFITSIO::initFits(const QCamFrame& frame) {

   numAxes_=2;
   axesDim_[0]=frame.size().width();
   axesDim_[1]=frame.size().height();
   axesDim_[2]=1;

   switch (frame.getMode()) {
   case GreyFrame:
   case RawRgbFrame1:
   case RawRgbFrame2:
   case RawRgbFrame3:
   case RawRgbFrame4:
      axesDim_[2]=1;
      numAxes_=2;
      break;
   case YuvFrame:
      axesDim_[2]=3;
      numAxes_=3;
      break;
   break;
   }

   fprintf(stdout,"CreateFITS ");
   fits_create_file(&fptr_,fileName_.c_str(),&status_);
   fits_report_error(stdout,status_);

   fprintf(stdout,"createImage ");
   fits_create_img(fptr_,BYTE_IMG /* 8 */, numAxes_, axesDim_,&status_);
   fits_report_error(stdout,status_);
}

bool FitsImageCFITSIO::save(const QCamFrame& frame) {

   if (fptr_==NULL) {
      initFits(frame);
   }
   fprintf(stdout,"WriteImage ");
   switch (numAxes_) {
   case 2:
      fits_write_pix(fptr_,TBYTE, base_,axesDim_[0]*axesDim_[1]*axesDim_[2],(void*)frame.Y(),&status_);
      fits_report_error(stdout,status_);
      break;
   case 3:
   {
      uchar * colorImageBuff = new uchar[axesDim_[0] * axesDim_[1] * 4];
      ccvt_420p_bgr32(axesDim_[0],axesDim_[1],
                      frame.Y(),frame.U(),frame.V(),(void*)colorImageBuff);
      uchar * colorImagePlane= new uchar[axesDim_[0] * axesDim_[1]];
      for (int i=0;i<3;++i) {
         for(int j=0;j<axesDim_[0] * axesDim_[1];++j) {
            colorImagePlane[j]=colorImageBuff[j*4+i];
         }
         base_[2]=3-i;
         fits_write_pix(fptr_,TBYTE, base_,axesDim_[0]*axesDim_[1],(void*)colorImagePlane,&status_);
         fits_report_error(stdout,status_);
      }
      delete colorImageBuff;
      delete colorImagePlane;
   }
   break;
   default:
      assert(0);
   }

   return (status_==0);
}

bool FitsImageCFITSIO::close() {
   if (fptr_ != NULL) {
      fprintf(stdout,"CloseFITS");
      fits_close_file(fptr_,&status_);
      fits_report_error(stdout,status_);
      fprintf(stdout,"\n");
      fptr_=NULL;
      fflush(stdout);
   }
   return (status_==0);
}
