#include "QTelescopeFile.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <math.h>

using namespace std;

double QTelescopeFile::getTime()
{
   double t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=(float)tv.tv_usec/(float)1000000;
   t+=tv.tv_sec;
   return(t);
}

void QTelescopeFile::Update()
{
   if(tracking_) {
      double newTime;
      if (sessionTime==0.0) sessionTime=getTime();
      newTime=fabs(getTime()-sessionTime);
      descriptorx_=fopen(filePathx.c_str(),"a");
      if (descriptorx_==NULL) perror(filePathx.c_str());
      fprintf(descriptorx_,"%.3f %.3f\r\n",newTime,xPosition);
      fclose(descriptorx_);
      descriptory_=fopen(filePathy.c_str(),"a");
      if (descriptory_==NULL) perror(filePathy.c_str());
      fprintf(descriptory_,"%.3f %.3f\r\n",newTime,yPosition);
      fclose(descriptory_);
   }
}

void QTelescopeFile::Reset() {
}

void QTelescopeFile::setTrack(bool tracking) {
   tracking_=tracking;
}

QTelescopeFile::QTelescopeFile(const char * filePath) :
   QTelescope() {
   filePathx=filePath;
   filePathy=filePath;

   sessionTime=0.0;
   xPosition=0.0;
   yPosition=0.0;

   filePathx+="x.dat";
   filePathy+="y.dat";

   tracking_=false;
}

void QTelescopeFile::goE(float shift) {
	xPosition=shift;
}

void QTelescopeFile::goW(float shift) {
   goE(shift);
}

void QTelescopeFile::goN(float shift) {
	yPosition=shift;
}

void QTelescopeFile::goS(float shift) {
   goN(shift);
}


