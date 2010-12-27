/******************************************************************
Qastrocam-g2
Copyright (C) 2010   Blaise-Florentin Collin

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

#include <qtimer.h>
#include <qvgroupbox.h>
#include <qhbox.h>
#include <qtooltip.h>

#include "SettingsBackup.hpp"

#include "QCamSimulator.moc"

// settings object, needed everywhere
extern settingsBackup settings;

QCamSimulator::QCamSimulator() {
   // set cam label
   label(QString("Simulator"));

   // init
   sizeTable_=NULL;
   sizeTable=getAllowedSize();

   // set frame
   yuvBuffer_.setMode(GreyFrame);
   yuvBuffer_.setSize(QSize(640,480));
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",640,480);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","Simulator");
}

QCamSimulator::~QCamSimulator() {
}

void QCamSimulator::resize(const QSize & s) {
   // NOP
}

const QSize* QCamSimulator::getAllowedSize() const {
   // lists sizes
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[2];
      sizeTable_[0]=QSize(640,480);
      sizeTable_[1]=QSize(0,0);
   }
   return sizeTable_;
}

const QSize& QCamSimulator::size() const {
   return(sizeTable[0]);
}

QWidget* QCamSimulator::buildGUI(QWidget * parent) {
   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QVGroupBox* settingsBox=new QVGroupBox(QString("Settings"),remoteCTRL);

   // set the first timer shot
   timer_=new QTimer(this);
   connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
   timer_->start(1000,true);

   return remoteCTRL;
}

bool QCamSimulator::updateFrame() {
   // get the frame buffer
   void* YBuff=NULL;
   YBuff=yuvBuffer_.YforOverwrite();
   // read picture datas
   setTime();
   // gives a new shot for the timer
   timer_->start(1000,true);
   // fill the frame

   // publish the frame
   newFrameAvaible();
}

