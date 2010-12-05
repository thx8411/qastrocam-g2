/******************************************************************
Qastrocam-g2
Copyright (C) 2010 Blaise-Florentin Collin

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

#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#include "QCamMovieSer.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"
#include "SettingsBackup.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

QCamMovieSer::QCamMovieSer() {
   // setting the default header
   memset(&header,0,sizeof(serHeader));
   memcpy(header.FileID,"LUCAM-RECORDER",14);
   header.LittleEndian=1;
   header.PixelDepth=8;
}

QCamMovieSer::~QCamMovieSer() {
   // nothing to do yet
}

QWidget * QCamMovieSer::buildGUI(QWidget  * father) {
   // nothing to do yet
   return father;
}

bool QCamMovieSer::openImpl(const string & seqName, const QCam & cam) {
   int ret;
   string fileName;

   frameNumber=0;
   // create the file
   fileName=seqName+".ser";
   fd=creat(fileName.c_str(),S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
   if(fd<0) return(FALSE);
   // update the header
   header.ImageWidth=cam.size().width();
   header.ImageHeight=cam.size().height();
   // write the header
   ret=write(fd,&header,sizeof(serHeader));
   if(ret!=sizeof(serHeader)) return(FALSE);
   // warn the user about mono

   return true;
}

void QCamMovieSer::closeImpl() {
   int ret;
   // update the header
   lseek(fd,0,SEEK_SET);
   header.FrameCount=frameNumber;
   ret=write(fd,&header,sizeof(serHeader));
   // close the file
   close(fd);
}

bool QCamMovieSer::addImpl(const QCamFrame & newFrame, const QCam & cam) {
   int ret;
   int size=newFrame.size().width()*newFrame.size().height();

   ret=write(fd,newFrame.Y(),size);
   if(ret==size) {
      frameNumber++;
      return(TRUE);
   } else
      return(FALSE);
}
