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


#include "QCamFindShift.hpp"
#include <iostream>

#include "Vector2D.hpp"
#include "ShiftInfo.hpp"
#include "QCam.hpp"

#include <Qt/qlabel.h>

QCamFindShift::QCamFindShift(){
   scope_=NULL;
}

QCamFindShift::QCamFindShift(QTelescope* scope){
   scope_=scope;
}

void QCamFindShift::reset() {

   cout << "reset tracker"<<endl;
   firstFrameRegistered_=false;
   if (scope_!=NULL) scope_->Reset();
}

void QCamFindShift::camDisconnected() {
}

void QCamFindShift::camConnected() {
   firstFrameRegistered_=false;
   currentFrameSize_=cam().size();
}

void QCamFindShift::newFrame() {
   if (currentFrameSize_!=cam().size()) {
      currentFrameSize_=cam().size();
      // frame size changed, reset tracker
      reset();
   }
   if (firstFrameRegistered_) {
      ShiftInfo theShift;
      if (findShift(theShift)) {
         currentShift_=theShift;
         emit(shift(theShift));
      }
   } else {
      firstFrameRegistered_=registerFirstFrame();
   }
}

QCamFrame QCamFindShift::image() const {
   QCamFrame img;
   if (firstFrameRegistered_) {
      img.setSize(QSize(32,32));
      cout << "copying center "
           <<(int)currentShift().center().x()-15 <<"x"
           <<(int)currentShift().center().y()-15 << " , "
           <<(int)currentShift().center().x()+16 <<"x"
           <<(int)currentShift().center().y()+16 <<endl;

      img.copy(cam().yuvFrame(),
               (int)currentShift().center().x()-15,
               (int)currentShift().center().y()-15,
               (int)currentShift().center().x()+16,
               (int)currentShift().center().y()+16,
               0,0);
   }
   return img;
}

QWidget* QCamFindShift::buildGUI(QWidget *parent) {
   return QCamClient::buildGUI(parent);
}
