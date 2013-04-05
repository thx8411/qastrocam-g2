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

#include "QCamMovieAviLossless_libav.hpp"
#include "QCamFrame.hpp"
#include "QCam.hpp"


QCamMovieAviLossless::QCamMovieAviLossless() {

}

QCamMovieAviLossless::~QCamMovieAviLossless() {

}

QWidget * QCamMovieAviLossless::buildGUI(QWidget  * father) {
   // nothing to do at this time
   return father;
}

bool QCamMovieAviLossless::openImpl(const string & seqName, const QCam & cam) {
   cam.writeProperties(seqName+".properties");
   //aviFile_ = avm::CreateWriteFile((seqName+".avi").c_str());

   // if grey frame use, yuy2 for smaller resulting file
   //if(cam.yuvFrame().getMode()==GreyFrame) {

   // get framerate
   //int frameRate=atoi(cam.getProperty("FrameRateSecond").c_str());

   // update properties
   cam.writeProperties(seqName+".properties");
   return true;
}

void QCamMovieAviLossless::closeImpl() {

}

bool QCamMovieAviLossless::addImpl(const QCamFrame & newFrame, const QCam & cam) {

   return true;
}

#endif /* HAVE_LIBAV_H */
