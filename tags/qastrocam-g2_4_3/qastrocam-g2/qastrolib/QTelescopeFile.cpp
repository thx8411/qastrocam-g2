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

void QTelescopeFile::Update(double x, double y)
{
   if(tracking_) {
      double newTime;
      xPosition=x;
      yPosition=y;
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
   char buff[30];

   filePathx=genericFilePath;
   filePathy=genericFilePath;

   time_t timet;
   time(&timet);
   struct tm * t=gmtime(&timet);
   snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);


   sessionTime=0.0;
   xPosition=0.0;
   yPosition=0.0;

   filePathx=filePathx + '-' + buff + '-' + "x.dat";
   filePathy=filePathy + '-' + buff + '-' + "y.dat";

   tracking_=false;
}

void QTelescopeFile::setTrack(bool tracking) {
   tracking_=tracking;
}

QTelescopeFile::QTelescopeFile(const char * filePath) :
   QTelescope() {
   char buff[30];

   genericFilePath=filePath;
   filePathx=genericFilePath;
   filePathy=genericFilePath;

   time_t timet;
   time(&timet);
   struct tm * t=gmtime(&timet);
   snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);


   sessionTime=0.0;
   xPosition=0.0;
   yPosition=0.0;

   filePathx=filePathx + '-' + buff + '-' + "x.dat";
   filePathy=filePathy + '-' + buff + '-' + "y.dat";

   tracking_=false;
}
