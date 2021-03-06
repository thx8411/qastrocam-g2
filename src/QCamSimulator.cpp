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

QCamSimulator::QCamSimulator() {
   // set cam label
   label(QString("Simulator"));

   // init
   timer_=NULL;
   sizeTable_=NULL;
   sizeTable=getAllowedSize();
   starPositionX_=_SIMULATOR_WIDTH_/2;
   starPositionY_=_SIMULATOR_HEIGHT_/2;
   raSpeed_=0.0;
   decSpeed_=0.0;
   raMove_=_SIMULATOR_STOP_;
   decMove_=_SIMULATOR_STOP_;
   exposure_=1000;
   progressCounter_=0;

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
   QIcon* tmpIcon;

   QWidget* remoteCTRL=QCam::buildGUI(parent);
   QCamVGroupBox* settingsBox=new QCamVGroupBox(QString("Settings"),remoteCTRL);

   // RA zone
   QCamHGroupBox* raZone=new QCamHGroupBox(QString("RA"),settingsBox);
   raSpeedSlider_=new QCamSlider(QString("Speed : "),false,raZone,0,20,false,false);

   // left button
   raLeft_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("left.png");
   if(tmpIcon!=NULL) {
      raLeft_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      raLeft_->setText("Left");
   raLeft_->setCheckable(true);

   // right button
   raRight_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("right.png");
   if(tmpIcon!=NULL) {
      raRight_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      raRight_->setText("Right");
   raRight_->setCheckable(true);

   // pause button
   raStop_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   if(tmpIcon!=NULL) {
      raStop_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      raStop_->setText("Pause");

   // center button
   raCenter_=new QPushButton(raZone);
   tmpIcon=QCamUtilities::getIcon("target_icon.png");
   if(tmpIcon!=NULL) {
      raCenter_->setIcon(*tmpIcon);
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
      decUp_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      decUp_->setText("Up");
   decUp_->setCheckable(true);

   // down button
   decDown_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("down.png");
   if(tmpIcon!=NULL) {
      decDown_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      decDown_->setText("Down");
   decDown_->setCheckable(true);

   // stop button
   decStop_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("movie_pause.png");
   if(tmpIcon!=NULL) {
      decStop_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      decStop_->setText("Pause");

   // center button
   decCenter_=new QPushButton(decZone);
   tmpIcon=QCamUtilities::getIcon("target_icon.png");
   if(tmpIcon!=NULL) {
      decCenter_->setIcon(*tmpIcon);
      delete tmpIcon;
   } else
      decCenter_->setText("Center");

   // Exposure zone
   QCamHGroupBox* expZone=new QCamHGroupBox(QString("Exposure"),settingsBox);
   expSpeedSlider_=new QCamSlider(QString("Exposure (s) : "),false,expZone,1,60,false,false);
   progress_=new QProgressBar(expZone);
   progress_->setMinimum(0);

   // connexions
   connect(raLeft_,SIGNAL(toggled(bool)),this,SLOT(moveLeft(bool)));
   connect(raRight_,SIGNAL(toggled(bool)),this,SLOT(moveRight(bool)));
   connect(decUp_,SIGNAL(toggled(bool)),this,SLOT(moveUp(bool)));
   connect(decDown_,SIGNAL(toggled(bool)),this,SLOT(moveDown(bool)));
   connect(raSpeedSlider_,SIGNAL(valueChange(int)),this,SLOT(setRaSpeed(int)));
   connect(decSpeedSlider_,SIGNAL(valueChange(int)),this,SLOT(setDecSpeed(int)));
   connect(expSpeedSlider_,SIGNAL(valueChange(int)),this,SLOT(setExpSpeed(int)));
   connect(raStop_,SIGNAL(pressed()),this,SLOT(stopRa()));
   connect(decStop_,SIGNAL(pressed()),this,SLOT(stopDec()));
   connect(raCenter_,SIGNAL(pressed()),this,SLOT(centerRa()));
   connect(decCenter_,SIGNAL(pressed()),this,SLOT(centerDec()));

   // init values
   raSpeedSlider_->setValue(1);
   decSpeedSlider_->setValue(1);
   setRaSpeed(1);
   setDecSpeed(1);
   setExpSpeed(1);

   // tooltips
   raSpeedSlider_->setToolTip(tr("RA speed (pixels/frame)"));
   raLeft_->setToolTip(tr("Move left"));
   raRight_->setToolTip(tr("Move right"));
   raStop_->setToolTip(tr("Stop RA moves"));
   raCenter_->setToolTip(tr("Center the star"));
   decSpeedSlider_->setToolTip(tr("DEC speed (pixels/frame)"));
   decUp_->setToolTip(tr("Move up"));
   decDown_->setToolTip(tr("Move down"));
   decStop_->setToolTip(tr("Stop DEC moves"));
   decCenter_->setToolTip(tr("Center the star"));

   // set the first timer shot
   timer_=new QTimer(this);
   connect(timer_,SIGNAL(timeout()),this,SLOT(updateFrame()));
   timer_->setSingleShot(false);

   // init timer
   expSpeedSlider_->setValue(1);
   setExpSpeed(1);
   progress_->setMaximum(1);

   // progress timer
   progressTimer_=new QTimer(this);
   connect(progressTimer_,SIGNAL(timeout()),this,SLOT(updateProgress()));
   progressTimer_->setSingleShot(false);
   progressTimer_->start(1000);

   return remoteCTRL;
}

bool QCamSimulator::updateFrame() {
   // get the frame buffer
   unsigned char* YBuff=NULL;
   YBuff=(unsigned char*)yuvBuffer_.YforOverwrite();
   // read picture datas
   setTime();
   // gives a new shot for the timer
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

   // progress bar
   if(progress_)
      progress_->setValue(0);
   progressCounter_=0;

   // publish the frame
   newFrameAvaible();
}

//
// private slots
//

void QCamSimulator::moveLeft(bool s) {
   if(s) {
      raRight_->setChecked(false);
      raMove_=_SIMULATOR_LEFT_;
   } else
      raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveRight(bool s) {
   if(s) {
      raLeft_->setChecked(false);
      raMove_=_SIMULATOR_RIGHT_;
   } else
      raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::stopRa() {
   raLeft_->setChecked(false);
   raRight_->setChecked(false);
   raMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveUp(bool s) {
   if(s) {
      decDown_->setChecked(false);
      decMove_=_SIMULATOR_UP_;
   } else
      decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::moveDown(bool s) {
   if(s) {
      decUp_->setChecked(false);
      decMove_=_SIMULATOR_DOWN_;
   } else
      decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::stopDec() {
   decUp_->setChecked(false);
   decDown_->setChecked(false);
   decMove_=_SIMULATOR_STOP_;
}

void QCamSimulator::setRaSpeed(int s) {
   raSpeed_=s;
}

void QCamSimulator::setDecSpeed(int s) {
   decSpeed_=s;
}

void QCamSimulator::setExpSpeed(int s) {
   exposure_=s*1000;
   if(timer_)
      timer_->start(exposure_);
   if(progress_) {
      progress_->setMaximum(s);
      progress_->setValue(0);
   }
   progressCounter_=0;
}

void QCamSimulator::centerRa() {
  starPositionX_=_SIMULATOR_WIDTH_/2;
}

void QCamSimulator::centerDec() {
   starPositionY_=_SIMULATOR_HEIGHT_/2;
}

void QCamSimulator::updateProgress() {
   progressCounter_++;
   if(progress_)
      progress_->setValue(progressCounter_);
}
