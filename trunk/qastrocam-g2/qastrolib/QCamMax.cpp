/******************************************************************
Qastrocam
Copyright (C) 2003-2009   Franck Sicard
Qastrocam-g2
Copyright (C) 2009-2012   Blaise-Florentin Collin

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


#include "QCamMax.moc"

#include <qpushbutton.h>
#include <qpixmap.h>

#include "QCamUtilities.hpp"

QCamMax::QCamMax(QCam* cam) {
   cam_=cam;
   connect(cam_,SIGNAL(newFrame()),this,SLOT(addNewFrame()));
   label("Max stacking");
   //initRemoteControl(remoteCTRL_);

   paused_=false;
}

void QCamMax::clear() {
   yuvFrame_.clear();
}

void QCamMax::addNewFrame() {
   if(!paused_) {
      QCamFrame origFrame = cam_->yuvFrame();
      bool colorMode=(origFrame.getMode()==YuvFrame);
      if (origFrame.size() != yuvFrame_.size()) {
         yuvFrame_.setSize(origFrame.size());
         yuvFrame_.clear();
      }
      int frameSize=yuvFrame_.size().height()*yuvFrame_.size().width();
      int frameWidth=yuvFrame_.size().width();
      uchar * locY=yuvFrame_.YforUpdate();
      uchar * locU=yuvFrame_.UforUpdate();
      uchar * locV=yuvFrame_.VforUpdate();
      const uchar * oriY=origFrame.Y();
      const uchar * oriU=NULL;
      const uchar * oriV=NULL;
      if (colorMode) {
         oriU=origFrame.U();
         oriV=origFrame.V();
      }
      int uvLineSize=frameWidth;
      for(int i=0;i<frameSize;++i) {
         if (oriY[i] > locY[i]) {
            locY[i]=oriY[i];
            if (colorMode) {
               int x=i%frameWidth;
               int y=i/frameWidth;
               int shift2=y*uvLineSize+x;
               locU[shift2]=oriU[shift2];
               locV[shift2]=oriV[shift2];
            }
         }
      }
      newFrameAvaible();
   }
}

QWidget* QCamMax::buildGUI(QWidget * parent) {
   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QPushButton* resetBufferFill_= new QPushButton("reset",remoteCTRL);
   connect(resetBufferFill_,SIGNAL(pressed()),this,SLOT(clear()));

   QPushButton* pauseBufferFill_= new QPushButton("",remoteCTRL);
   QPixmap* tmpIcon;
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   pauseBufferFill_->setToggleButton(true);
   if(tmpIcon!=NULL) pauseBufferFill_->setPixmap(*tmpIcon);
   delete tmpIcon;
   connect(pauseBufferFill_,SIGNAL(pressed()),this,SLOT(pause()));

   pauseBufferFill_->show();
   resetBufferFill_->show();
   return remoteCTRL;
}

void QCamMax::pause() {
   if(paused_)
      paused_=false;
   else
      paused_=true;
}
