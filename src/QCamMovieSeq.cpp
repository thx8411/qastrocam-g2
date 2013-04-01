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


#include "QCamMovieSeq.hpp"
#include "QCam.hpp"

#include <sys/stat.h>
#include <sys/types.h>

QCamMovieSeq::QCamMovieSeq() {
}

QWidget * QCamMovieSeq::buildGUI(QWidget  * father) {
   return father;
}

bool QCamMovieSeq::openImpl(const string & seqName, const QCam & cam) {
   seqenceFileName_=seqName;
   if (mkdir(seqenceFileName_.c_str(),0777)) {
      perror("QCamMovieSeq::open");
      return false;
   }
   return true;
}

void QCamMovieSeq::closeImpl() {
   seqenceFileName_="/dev/null";
}

bool QCamMovieSeq::addImpl(const QCamFrame & newFrame, const QCam & cam) {
   char number[10];
   snprintf(number,9,"%d",getFrameNumber());
   return cam.saveFrame(seqenceFileName_+'/'+number);
}
