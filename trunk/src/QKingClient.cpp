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
#include <time.h>

#include "QCamHBox.hpp"
#include "QKingClient.hpp"
#include "QCamUtilities.hpp"


QKingClient::QKingClient() {
   //
   //
   //

   statusBar_=NULL;
   hidePause(true);
}

QKingClient::~QKingClient() {
   kingCam_->disconnectCam();
   delete(kingCam_);
}

QWidget* QKingClient::buildGUI(QWidget* parent) {
   QWidget* w=QCamFindShift_hotSpot::buildGUI(parent);
   w->setWindowTitle("King module");

   QCamUtilities::registerWidget(w);

   QCamHBox* kingButtons=new QCamHBox(w);

   kingStartButton=new QPushButton("start",kingButtons);
   connect(kingStartButton,SIGNAL(pressed()),this,SLOT(kingStart()));

   kingStopButton=new QPushButton("stop",kingButtons);
   kingStopButton->setDisabled(true);
   connect(kingStopButton,SIGNAL(pressed()),this,SLOT(kingStop()));

   QPushButton* kingResetButton=new QPushButton("reset",kingButtons);
   connect(kingResetButton,SIGNAL(pressed()),this,SLOT(kingReset()));

   statusBar_=new QLabel(w);

   QString stat;
   stat.sprintf("Orig:%d,%d time=%d shift=%1.1f,%1.1f move=%1.1f,%1.1f",
                (int)firstHotSpot_.x(),(int)firstHotSpot_.y(),
                 0,0.0,0.0,0.0,0.0);

   kingCam_=new QCamTrans();
   kingCam_->label("King display");
   kingCam_->connectCam(cam());
   kingCam_->hideMode(true);
   kingCam_->hideFile(true);
   kingCam_->buildGUI(w);

   kingReset();

   return w;
}

void QKingClient::kingStart() {
   // update GUI
   if(kingStartButton)
      kingStartButton->setDisabled(true);
   if(kingStopButton)
      kingStopButton->setDisabled(false);

   reset();

   registerFirstFrame();
   firstFrameDate_=time(NULL);
}

void QKingClient::kingStop() {
   // update GUI
   if(kingStartButton)
      kingStartButton->setDisabled(false);
   if(kingStopButton)
      kingStopButton->setDisabled(true);

   if(findShift(frameShift_)) {
      Vector2D correctPos;
      int dt=(time(NULL)-firstFrameDate_);

      // sideral speed in rad/s
      double sideralSpeed=0.0000727;
      double angle=sideralSpeed*dt;
      double dx=frameShift_.shift().x();
      double dy=frameShift_.shift().y();

      if (dt != 0) {
         correctPos=Vector2D(0.5*(dx+dy*sin(angle)/(1.0-cos(angle))),
                             0.5*(dy-dx*sin(angle)/(1-cos(angle))));
      }
      if (statusBar_) {
         QString stat;
         stat.sprintf("Orig:%d,%d time=%d shift=%1.1f,%1.1f move=%1.1f,%1.1f",
                      (int)firstHotSpot_.x(),(int)firstHotSpot_.y(),
                      dt,dx,dy,correctPos.x(),correctPos.y());
         statusBar_->setText(stat);
      }
      kingCam_->annotate(firstHotSpot_-correctPos);
   }
}

void QKingClient::kingReset() {
   // update GUI
   if(kingStartButton)
      kingStartButton->setDisabled(false);
   if(kingStopButton)
      kingStopButton->setDisabled(true);

   reset();

   if (statusBar_) {
         QString stat;
         stat.sprintf("Orig:%d,%d time=%d shift=%1.1f,%1.1f move=%1.1f,%1.1f",
                      (int)firstHotSpot_.x(),(int)firstHotSpot_.y(),
                      0,0.0,0.0,0.0,0.0);
         statusBar_->setText(stat);
   }
   kingCam_->annotate(false);
}
