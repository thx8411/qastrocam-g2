#include "QTelescopeFile.moc"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <math.h>

using namespace std;

float QTelescopeFile::getTime()
{
   return((float)clock()/(float)CLOCKS_PER_SEC);
}

void QTelescopeFile::writeToFile()
{
   float newTime;
   if (sessionTime==0.0) sessionTime=getTime();
   newTime=fabs(getTime()-sessionTime);
   newTime=(roundf(newTime*100)/100);
   if(newTime!=lastTime)
   {
      lastTime=newTime;
      descriptorx_=fopen(filePathx.c_str(),"a");
      if (descriptorx_==NULL) {
        perror(filePathx.c_str());
      }
      fprintf(descriptorx_,"%.3f %.3f\r\n",newTime,lastxShift);
      fclose(descriptorx_);
      descriptory_=fopen(filePathy.c_str(),"a");
      if (descriptory_==NULL) {
         perror(filePathy.c_str());
      }
      fprintf(descriptory_,"%.3f %.3f\r\n",newTime,lastyShift);
      fclose(descriptory_);
   }
}

QTelescopeFile::QTelescopeFile(const char * filePath) :
   QTelescope() {
   filePathx=filePath;
   filePathy=filePath;

   sessionTime=0.0;

   filePathx+="x.dat";
   filePathy+="y.dat";
   descriptorx_=fopen(filePathx.c_str(),"a");
   if (descriptorx_==NULL) {
      perror(filePathx.c_str());
   }
   descriptory_=fopen(filePathy.c_str(),"a");
   if (descriptory_==NULL) {
      perror(filePathy.c_str());
   }
   fclose(descriptorx_);
   fclose(descriptory_);
}

void QTelescopeFile::goE(float shift) {
	lastxShift=shift;
	writeToFile();
}

void QTelescopeFile::goW(float shift) {
   goE(shift);
}

void QTelescopeFile::goN(float shift) {
	lastyShift=shift;
	writeToFile();
}

void QTelescopeFile::goS(float shift) {
   goN(shift);
}


