/******************************************************************
Qastrocam-g2
Copyright (C) 2013 Blaise-Florentin Collin

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


// only available if have libav
#if HAVE_LIBAV_H

#include <stdlib.h>

#include "yuv.hpp"

#include "QCamMovieAvi_libav.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"
#include "SettingsBackup.hpp"


// settings object, needed everywhere
extern settingsBackup settings;

QCamMovieAvi::QCamMovieAvi() {

   // should the movie be registax compatible ?
   if(settings.haveKey("REGISTAX_AVI"))
      registaxCompatibility=(string(settings.getKey("REGISTAX_AVI"))=="yes");
   else
      registaxCompatibility=false;
}

QCamMovieAvi::~QCamMovieAvi() {

}

QWidget * QCamMovieAvi::buildGUI(QWidget  * father) {
   // nothing to do yet
   return father;
}

// open a stream
bool QCamMovieAvi::openImpl(const string & seqName, const QCam & cam) {
   cam.writeProperties(seqName+".properties");
   //aviFile_ = avm::CreateWriteFile((seqName+".avi").c_str());

   // build header for grey frames (we must add a palette)
   //if((cam.yuvFrame().getMode()==GreyFrame)&&!(registaxCompatibility)) {

   // get the framerate
   // int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());

   cam.writeProperties(seqName+".properties");
   return true;
}

// close a stream
void QCamMovieAvi::closeImpl() {

}

// add a frame
bool QCamMovieAvi::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   return true;
}

#endif /* HAVE_LIBAV_H */
