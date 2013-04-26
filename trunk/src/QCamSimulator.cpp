/******************************************************************
Qastrocam-g2
Copyright (C) 2010-2013   Blaise-Florentin Collin

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

#include <Qt/qtimer.h>
#include <Qt/qtooltip.h>
#include <Qt/qpixmap.h>

#include "QCamVGroupBox.hpp"
#include "QCamHGroupBox.hpp"
#include "SettingsBackup.hpp"
#include "QCamUtilities.hpp"
#include "QCamSimulator.hpp"

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
   QPixmap* tmpIcon;

   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QCamVGroupBox* settingsBox=new QCamVGroupBox(QString("Settings"),remoteCTRL);

   // RA zone
   QCamHGroupBox* raZone=new QCamHGroupBox(QString("RA"),settingsBox);
   raSpeedSlider_=new QCamSlider(QString("Speed : "),false,raZone,0,20,false,false);

   // left button
   raLeft_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("left.png");
   if(tmpIcon!=NULL) {
      raLeft_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      raLeft_->setText("Left");
   raLeft_->setToggleButton(true);

   // right button
   raRight_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("right.png");
   if(tmpIcon!=NULL) {
      raRight_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      raRight_->setText("Right");
   raRight_->setToggleButton(true);

   // pause button
   raStop_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   if(tmpIcon!=NULL) {
      raStop_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      raStop_->setText("Pause");

   // center button
   raCenter_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("target_icon.png");
   if(tmpIcon!=NULL) {
      raCenter_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      raCenter_->setText("Center");

   // DEC zone
   QCamHGroupBox* decZone=new QCamHGroupBox(QString("DEC"),settingsBox);
   decSpeedSlider_=new QCamSlider(QString("Speed : "),false,decZone,0,20,false,false);

   // up button
   decUp_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("up.png");
   if(tmpIcon!=NULL) {
      decUp_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      decUp_->setText("Up");
   decUp_->setToggleButton(true);

   // down button
   decDown_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("down.png");
   if(tmpIcon!=NULL) {
      decDown_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      decDown_->setText("Down");
   decDown_->setToggleButton(true);

   // stop button
   decStop_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   if(tmpIcon!=NULL) {
      decStop_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      decStop_->setText("Pause");

   // center button
   decCenter_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("target_icon.png");
   if(tmpIcon!=NULL) {
      decCenter_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      decCenter_->setText("Center");

   // connexions
   connect(raLeft_,SIGNAL(toggled(bool)),this,SLOT(moveLeft(bool)));
   connect(raRight_,SIGNAL(toggled(bool)),this,SLOT(moveRight(bool)));
   connect(decUp_,SIGNAL(toggled(bool)),this,SLOT(moveUp(bool)));
   connect(decDown_,SIGNAL(toggled(bool)),this,SLOT(moveDown(bool)));
   connect(raSpeedSlider_,SIGNAL(valueChange(int)),this,SLOT(setRaSpeed(int)));
   connect(decSpeedSlider_,SIGNAL(valueChange(int)),this,SLOT(setDecSpeed(int)));
   connect(raStop_,SIGNAL(pressed()),this,SLOT(stopRa()));
   connect(decStop_,SIGNAL(pressed()),this,SLOT(stopDec()));
   connect(raCenter_,SIGNAL(pressed()),this,SLOT(centerRa()));
   connect(decCenter_,SIGNAL(pressed()),this,SLOT(centerDec()));

   // init values
   raSpeedSlider_->setValue(1);
   decSpeedSlider_->setValue(1);
   setRaSpeed(1);
   setDecSpeed(1);

   // tooltips
   QToolTip::add(raSpeedSlider_,tr("RA speed (pixels/frame)"));
   QToolTip::add(raLeft_,tr("Move left"));
   QToolTip::add(raRight_,tr("Move right"));
   QToolTip::add(raStop_,tr("Stop RA moves"));
   QToolTip::add(raCenter_,tr("Center the star"));
   QToolTip::add(decSpeedSlider_,tr("DEC speed (pixels/frame)"));
   QToolTip::add(decUp_,tr("Move up"));
   QToolTip::add(decDown_,tr("Move down"));
   QToolTip::add(decStop_,tr("Stop DEC moves"));
   QToolTip::add(decCenter_,tr("Center the star"));

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

void QCamSimulator::moveLeft(bool s) {
   if(s) {
      raRight_->setOn(false);
      raMove_=_SIMULATOR_LEFT_;
   } else
      raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveRight(bool s) {
   if(s) {
      raLeft_->setOn(false);
      raMove_=_SIMULATOR_RIGHT_;
   } else
      raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::stopRa() {
   raLeft_->setOn(false);
   raRight_->setOn(false);
   raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveUp(bool s) {
   if(s) {
      decDown_->setOn(false);
      decMove_=_SIMULATOR_UP_;
   } else
      decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveDown(bool s) {
   if(s) {
      decUp_->setOn(false);
      decMove_=_SIMULATOR_DOWN_;
   } else
      decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::stopDec() {
   decUp_->setOn(false);
   decDown_->setOn(false);
   decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::setRaSpeed(int s) {
   raSpeed_=s;
}

void QCamSimulator::setDecSpeed(int s) {
   decSpeed_=s;
}

void QCamSimulator::centerRa() {
  starPositionX_=_SIMULATOR_WIDTH_/2;
}

void QCamSimulator::centerDec() {
   starPositionY_=_SIMULATOR_HEIGHT_/2;
}
