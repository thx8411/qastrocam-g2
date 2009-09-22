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

#if 0

char headerfit[18][81]={
   "SIMPLE  =                    T                                                  ",
   "BITPIX  =                   16                                                  ", 
   "NAXIS   =                    2                                                  ", 
   "NAXIS1  =                  640                                                  ", 
   "NAXIS2  =                  480                                                  ", 
   "DATE-OBS= 'dd/mm/yy'                                                            ",  
   "UTC-OBS = 'hh:mm:ss.cc'                                                         ", 
   "EXPTIME =                    0                                                  ", 
   "MIPS-HI =                  255                                                  ", 
   "MIPS-LO =                    0                                                  ", 
   "MIPS-X1 =                    0                                                  ", 
   "MIPS-Y1 =                    0                                                  ", 
   "MIPS-X2 =                    0                                                  ", 
   "MIPS-Y2 =                    0                                                  ", 
   "MIPS-BIX=                    1                                                  ", 
   "MIPS-BIY=                    1                                                  ", 
   "MIPS-CCD=                    0                                                  ", 
   "END                                                                             " 
}; 

int FitsImageFM::debug=0;


bool FitsImageFM::load(const string & name) {
   char header[2880];
   int fd, count;

   fd=open(name.c_str(),O_RDONLY);
   if (fd==-1){
      fprintf(stderr, "%s :", name.c_str());
      perror("Fichier raté");
      return false;
   }

   if((count=read(fd, header,2880))!=2880){
      fprintf(stderr,"readfits : %d octets lus\n",count);
      perror("readfits: raté la lecture du header");
      return false;
   }
   sscanf(header+256, "%d", &(rx_));
   sscanf(header+336, "%d", &(ry_));
   sscanf(header+0x290, "%d", &(hi_));
   sscanf(header+0x2e0, "%d", &(lo_));

   int nbdata=(rx_)*(ry_);
   if((f_=(WORD*)realloc(f_,rx_ * ry_*sizeof(WORD)))==NULL){
      fprintf(stderr,"readfits: error realloc\n");
      exit(1);
   }
   read(fd,f_,nbdata*sizeof(WORD));
   swab((char *)f_,(char *)f_,nbdata*sizeof(WORD));

   close(fd);
   return true;
}

bool FitsImageFM::save(const string & name) const {
   char header[2880];
   char buffer[256];
   int fd;
   int i;

   memset(header,0x20,2880);
   for(i=0;i<18;++i){
      memcpy(header+80*i, headerfit[i],80);
   }
   sprintf(buffer,"NAXIS1  =                  %3d                                                       ", rx_); 
   memcpy(header+80*3, buffer, 80);
   
   sprintf(buffer,"NAXIS2  =                  %3d                                                       ", ry_); 
   memcpy(header+80*4, buffer, 80);

   sprintf(buffer,"MIPS-HI =                  %3d                                                       ", hi_); 
   memcpy(header+80*9, buffer, 80);

   sprintf(buffer,"MIPS-LO =                  %3d                                                       ", lo_); 
   memcpy(header+80*10, buffer, 80);

   /*
    snprintf(buff,100,"%04d-%02d-%02d-%02d-%02d-%02d.%06ld",
            TM.tm_year+1900,TM.tm_mon+1,TM.tm_mday,
            TM.tm_hour, TM.tm_min, TM.tm_sec,
            TIMEVAL.tv_usec
      );
   setProperty("TIME",buff);
   */

   map<string,string>::const_iterator it=externalProperties_.find("TIME");
   if (it != externalProperties_.end()) {
      const string & timeStr=it->second;

      int year,month,day;
      int hour,minute,second,csecond;
      
      sscanf(timeStr.c_str(),"%04d-%02d-%02d-%02d-%02d-%02d.%06ld",
             &year,&month,&day,&hour,&minute,&second,&csecond);
      csecond/=1000;

      snprintf(buffer,sizeof(buffer),
               "DATE-OBS= '%02d/%02d/%02d'                                                                                   ",
               day,month,year%1000);
      memcpy(header+80*5, buffer, 80);
      snprintf(buffer,sizeof(buffer),
               "UTC-OBS = '%02d:%02d:%02d.%02d'                                                                                ", 
               hour,minute,second,csecond);
      memcpy(header+80*6, buffer, 80);
   }
   
   fd=open(name.c_str(),O_RDWR|O_CREAT,0644);
   if (fd == -1 ) {
      perror(name.c_str());
      return false;
   }
   write(fd, header,2880);
   int nbdata=(rx_)*(ry_);
   swab((char *)f_,(char *)f_,nbdata*sizeof(WORD));
   write(fd, f_, rx_ * ry_ *sizeof(WORD));
   close(fd);
   if (debug)
      fprintf(stderr, "savefit: %s %d %d\n", name.c_str(), rx_, ry_);
   return true;
}

FitsImageFM::FitsImageFM(const QCamFrame &src) {
   rx_=src.size().width();
   ry_=src.size().height();
   lo_=65536;
   hi_=-65536;
   f_=new WORD[rx_*ry_];
   for (int j=0;j<ry_;j++) {
      for (int i=0;i<rx_;i++) {
         int pix=src.YLine(ry_-1-j)[i];
         f_[i+j*rx_]=pix;
         if (pix<lo_) {
            lo_=pix;
         }
         if (pix>hi_) {
            hi_=pix;
         }
      }
   }
   src.exportProperties(externalProperties_);
}

FitsImageFM::~FitsImageFM() {
   delete f_;
}
#endif


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
