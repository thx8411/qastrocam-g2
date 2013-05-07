/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
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

#include <math.h>

#include "QCamTrans.hpp"
#include "QCam.hpp"
#include "FrameAlgo.hpp"
#include "QCamRadioBox.hpp"

QCamTrans::QCamTrans():
   cam_(NULL),
   algo_(NULL),
   algoWidget_(NULL) {
   mode_=Off;
   hideMode_=false;
   //label("None");
}

void QCamTrans::connectCam(QCam & theCam) {
   if (cam_) {
      disconnectCam();
   }
   cam_=&theCam;
   connect(cam_,SIGNAL(newFrame()),this,SLOT(transNewFrame()));
   mode_=algo_?On:Copy;
}

void QCamTrans::disconnectCam() {
   if (!cam_) {
      return;
   }
   disconnect(cam_,SIGNAL(newFrame()),this,SLOT(transNewFrame()));
   mode_=Off;
   cam_=NULL;
}

void QCamTrans::connectAlgo(FrameAlgo & algo) {
   if (algo_) {
      disconnectAlgo();
   }
   algo_=&algo;
   label(algo.label());
   mode_=On;
   if (guiBuild()) {
      algoWidget_=algo_->allocGui(gui());
      algoWidget_->show();
   }
}

void QCamTrans::disconnectAlgo() {
   if (!algo_) {
      return;
   }
   mode_=cam_?Copy:Off;
   algo_->reset();
   algo_=NULL;
   if (algoWidget_) {
      algoWidget_->hide();
      delete(algoWidget_);
   }
   label("None");
}

void QCamTrans::mode(int intMode) {
   Mode mode=(Mode)intMode;
   switch(mode) {
   case Off:
      mode_=Off;
      break;
   case On:
      if (algo_ != NULL) {
         mode_ = On;
      }
      break;
   case Copy:
      if (cam_ != NULL) {
         mode_=Copy;
      }
      break;
   }
   emit(modeChanged(mode_));
}

void QCamTrans::transNewFrame() {
   switch(mode_) {
   case Off:
      break;
   case On:
      assert(algo_);
      if (algo_->transform(cam_->yuvFrame(),camFrame_)) {
         newFrameAvaible();
      } else {
      }
      break;
   case Copy:
      camFrame_=cam_->yuvFrame();
      newFrameAvaible();
      break;
   };
}

QWidget * QCamTrans::buildGUI(QWidget * parent) {
   QWidget * remoteCTRL=QCam::buildGUI(parent);
   int modeTable[]={Off,Copy,On};
   const char * modeLabel[]={"Off","Copy","On"};
   QCamRadioBox * modeWidget=new QCamRadioBox("Mode",remoteCTRL,
                        3,modeTable,modeLabel,3);
   connect(this,SIGNAL(modeChanged(int)),modeWidget,SLOT(update(int)));
   connect(modeWidget,SIGNAL(change(int)),this,SLOT(mode(int)));
   emit(modeChanged(mode_));
   if (algo_) {
      algoWidget_=algo_->allocGui(gui());
      algoWidget_->show();
   }

   if(hideMode_)
      modeWidget->hide();

   return remoteCTRL;
}
