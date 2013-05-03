/******************************************************************
Qastrocam-g2
Copyright (C) 2009-2013   Blaise-Florentin Collin

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

#include <sys/time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include <iostream>

#include "QTelescopeFile.hpp"

using namespace std;

// get time form the system, in seconds (usec accuracy)
double QTelescopeFile::getTime()
{
   double t;
   struct timeval tv;
   gettimeofday(&tv,NULL);
   t=(float)tv.tv_usec/(float)1000000;
   t+=tv.tv_sec;
   return(t);
}

// called when a new frame with new shifts come
void QTelescopeFile::Update(double x, double y)
{
   if(tracking_) {
      double newTime;
      xPosition=x;
      yPosition=y;
      // get the system time
      if (sessionTime==0.0) sessionTime=getTime();
      newTime=fabs(getTime()-sessionTime);
      descriptorx_=fopen(filePathx.c_str(),"a");
      // add the system time and the shift in the "x" file
      if (descriptorx_==NULL) perror(filePathx.c_str());
      fprintf(descriptorx_,"%.3f %.3f\r\n",newTime,xPosition);
      fclose(descriptorx_);
      // same for "y" file
      descriptory_=fopen(filePathy.c_str(),"a");
      if (descriptory_==NULL) perror(filePathy.c_str());
      fprintf(descriptory_,"%.3f %.3f\r\n",newTime,yPosition);
      fclose(descriptory_);
   }
}

// reset the actual tracking files, creating a new ones
void QTelescopeFile::Reset() {
   char buff[30];

   filePathx=genericFilePath;
   filePathy=genericFilePath;

   // file name are time stamped
   time_t timet;
   time(&timet);
   struct tm * t=gmtime(&timet);
   snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
   // reset current values
   sessionTime=0.0;
   xPosition=0.0;
   yPosition=0.0;
   // new file names
   filePathx=filePathx + '-' + buff + '-' + "x.dat";
   filePathy=filePathy + '-' + buff + '-' + "y.dat";
   // stop tracking
   tracking_=false;
}

// parent class compatibility
void QTelescopeFile::setTrack(bool tracking) {
   tracking_=tracking;
}

// constructor
QTelescopeFile::QTelescopeFile(const char * filePath) :
   QTelescope() {
   char buff[30];

   // setting file name base
   genericFilePath=filePath;
   filePathx=genericFilePath;
   filePathy=genericFilePath;
   // file names are time stamped
   time_t timet;
   time(&timet);
   struct tm * t=gmtime(&timet);
   snprintf(buff,30,"%04d.%02d.%02d-%02dh%02dm%02ds",
            t->tm_year+1900,t->tm_mon+1,t->tm_mday,
            t->tm_hour,t->tm_min,t->tm_sec);
   // reset current values
   sessionTime=0.0;
   xPosition=0.0;
   yPosition=0.0;
   // new file names
   filePathx=filePathx + '-' + buff + '-' + "x.dat";
   filePathy=filePathy + '-' + buff + '-' + "y.dat";
   // tracking set to stop
   tracking_=false;
}

// GUI
void QTelescopeFile::buildGUI(QWidget* parent) {
   // nothing to be done here
   // telescope controls are useless for fifo
}
