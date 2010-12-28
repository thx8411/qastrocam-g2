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

#define _SIMULATOR_WIDTH_	640
#define _SIMULATOR_HEIGHT_	480
#define _SIMULATOR_EXPOSURE_	1000

QCamSimulator::QCamSimulator() {
   // set cam label
   label(QString("Simulator"));

   // init
   sizeTable_=NULL;
   sizeTable=getAllowedSize();
   starPositionX_=_SIMULATOR_WIDTH_/2;
   starPositionY_=_SIMULATOR_HEIGHT_/2;
   raSpeed_=0.0;
   decSpeed_=0.0;
   raMove_=_SIMULATOR_STOP_;
   decMove_=_SIMULATOR_STOP_;

   // set frame
   yuvBuffer_.setMode(GreyFrame);
   yuvBuffer_.setSize(QSize(_SIMULATOR_WIDTH_,_SIMULATOR_HEIGHT_));
   // set prop.
   static char buff[11];
   snprintf(buff,10,"%dx%d",_SIMULATOR_WIDTH_,_SIMULATOR_HEIGHT_);
   setProperty("FrameSize",buff,true);
   setProperty("CameraName","Simulator");
}

QCamSimulator::~QCamSimulator() {
   // NOP
}

void QCamSimulator::resize(const QSize & s) {
   // NOP
}

const QSize* QCamSimulator::getAllowedSize() const {
   // lists sizes
   if (sizeTable_==NULL) {
      sizeTable_=new QSize[2];
      sizeTable_[0]=QSize(_SIMULATOR_WIDTH_,_SIMULATOR_HEIGHT_);
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
   timer_->start(_SIMULATOR_EXPOSURE_,true);

   return remoteCTRL;
}

bool QCamSimulator::updateFrame() {
   // get the frame buffer
   unsigned char* YBuff=NULL;
   YBuff=(unsigned char*)yuvBuffer_.YforOverwrite();
   // read picture datas
   setTime();
   // gives a new shot for the timer
   timer_->start(_SIMULATOR_EXPOSURE_,true);
   // fill the frame
   memset(YBuff,0,_SIMULATOR_WIDTH_*_SIMULATOR_HEIGHT_);
   // compute the new star position
   if(raMove_==_SIMULATOR_LEFT_) starPositionX_-=raSpeed_;
   if(raMove_==_SIMULATOR_RIGHT_) starPositionX_+=raSpeed_;
   if(decMove_==_SIMULATOR_UP_) starPositionY_-=decSpeed_;
   if(decMove_==_SIMULATOR_DOWN_) starPositionY_+=decSpeed_;
   if(starPositionX_<1) starPositionX_=1;
   if(starPositionX_>=(_SIMULATOR_WIDTH_-1)) starPositionX_=_SIMULATOR_WIDTH_-2;
   if(starPositionY_<1) starPositionY_=1;
   if(starPositionY_>=(_SIMULATOR_HEIGHT_-1)) starPositionY_=_SIMULATOR_HEIGHT_-2;

   // draw the star
   YBuff[((int)starPositionY_-1)*_SIMULATOR_WIDTH_+(int)starPositionX_]=0x80;
   YBuff[(int)starPositionY_*_SIMULATOR_WIDTH_+(int)starPositionX_-1]=0x80;
   YBuff[(int)starPositionY_*_SIMULATOR_WIDTH_+(int)starPositionX_]=0xF0;
   YBuff[(int)starPositionY_*_SIMULATOR_WIDTH_+(int)starPositionX_+1]=0x80;
   YBuff[(int)(starPositionY_+1)*_SIMULATOR_WIDTH_+(int)starPositionX_]=0x80;
   YBuff[(int)(starPositionY_-1)*_SIMULATOR_WIDTH_+(int)starPositionX_-1]=0x40;
   YBuff[(int)(starPositionY_-1)*_SIMULATOR_WIDTH_+(int)starPositionX_+1]=0x40;
   YBuff[(int)(starPositionY_+1)*_SIMULATOR_WIDTH_+(int)starPositionX_-1]=0x40;
   YBuff[(int)(starPositionY_+1)*_SIMULATOR_WIDTH_+(int)starPositionX_+1]=0x40;

   // publish the frame
   newFrameAvaible();
}

//
// private slots
//

void QCamSimulator::moveLeft() {
   // add gui controls

   raMove_=_SIMULATOR_LEFT_;
}

void QCamSimulator::moveRight() {
   // add gui controls

   raMove_=_SIMULATOR_RIGHT_;
}

void QCamSimulator::stopRa() {
   // add gui controls

   raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveUp() {
   // add gui controls

   decMove_=_SIMULATOR_UP_;
}

void QCamSimulator::moveDown() {
   // add gui controls

  decMove_=_SIMULATOR_DOWN_;
}

void QCamSimulator::stopDec() {
   // add gui controls

   decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::setRaSpeed() {
   // add gui controls
}

void QCamSimulator::setDecSpeed() {
   // add gui controls
}

void QCamSimulator::centerRa() {
   // add gui controls

  starPositionX_=_SIMULATOR_WIDTH_/2;
}

void QCamSimulator::centerDec() {
   // add gui controls

   starPositionY_=_SIMULATOR_HEIGHT_/2;
}
