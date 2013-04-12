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

#include <qpushbutton.h>
#include <qtooltip.h>

#include "QCamClient.hpp"
#include "QCam.hpp"
#include "QCamUtilities.hpp"

QCamClient::QCamClient(): cam_(NULL) {
   paused_=true;
   cam_=NULL;
}

QCamClient::QCamClient(QCam & theCam) {
   paused_=true;
   cam_=NULL;
   connectCam(theCam);
}

void QCamClient::connectCam(QCam & theCam) {
   if (cam_) {
      disconnectCam();
   }
   cam_=&theCam;
   camConnected();
   connect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   paused_=false;
}

void QCamClient::disconnectCam() {
   if (!cam_) {
      return;
   }
   disconnect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   paused_=true;
   camDisconnected();
   cam_=NULL;
}

void QCamClient::disable(bool notactive) {
   if (!notactive) resume();
   else pause();
}

void QCamClient::pause() {
   if (!paused_) {
      paused_=true;
      disconnect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   }
}
void QCamClient::resume() {
   if (paused_) {
      paused_=false;
      connect(cam_,SIGNAL(newFrame()),this,SLOT(newFrame()));
   }
}

QWidget * QCamClient::buildGUI(QWidget *parent) {
   QPixmap* tmpIcon;
   QPushButton * pauseCapture_=new QPushButton("pause",parent);
   QToolTip::add(pauseCapture_,"Suspend the current capture");
   pauseCapture_->setToggleButton(true);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   if(tmpIcon!=NULL) pauseCapture_->setPixmap(*tmpIcon);
   delete tmpIcon;
   connect(pauseCapture_,SIGNAL(toggled(bool)),this,SLOT(disable(bool)));
   return parent;
}
