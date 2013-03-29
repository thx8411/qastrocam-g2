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


#include "QTelescope.moc"

#include <iostream>

#include <stdlib.h>
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qhgroupbox.h>

#include "QCamUtilities.hpp"
#include "SettingsBackup.hpp"

// settings object, needed everywhere
extern settingsBackup settings;

QTelescope::QTelescope() {
   mainWidget_=NULL;
   arrowsLayout_=NULL;
   arrows_=NULL;
   upButton_=NULL;
   downButton_=NULL;
   leftButton_=NULL;
   rightButton_=NULL;
   decSwap_=NULL;
   raSwap_=NULL;
}

void QTelescope::buildGUI(QWidget * parent) {
   QPixmap* tmpIcon;
   mainWidget_=new QVGroupBox("Telescope CTRL",parent);

   QCamUtilities::registerWidget(mainWidget_);

   QCamUtilities::setQastrocamIcon(mainWidget_);
   arrows_ = new QWidget(mainWidget_);
   arrowsLayout_=new QGridLayout(arrows_,3,3);
   upButton_=new QPushButton(arrows_,"U");
   tmpIcon=QCamUtilities::getIcon("up.png");
   if(tmpIcon!=NULL) upButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   downButton_=new QPushButton(arrows_,"D");
   tmpIcon=QCamUtilities::getIcon("down.png");
   if(tmpIcon!=NULL) downButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   leftButton_=new QPushButton(arrows_,"L");
   tmpIcon=QCamUtilities::getIcon("left.png");
   if(tmpIcon!=NULL) leftButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   rightButton_=new QPushButton(arrows_,"R");
   tmpIcon=QCamUtilities::getIcon("right.png");
   if(tmpIcon!=NULL) rightButton_->setPixmap(*tmpIcon);
   delete tmpIcon;
   arrowsLayout_->addWidget(upButton_,0,1);
   arrowsLayout_->addWidget(downButton_,2,1);
   arrowsLayout_->addWidget(leftButton_,1,0);
   arrowsLayout_->addWidget(rightButton_,1,2);

   decSwap_=new QCheckBox("Swap North/South",mainWidget_);
   raSwap_=new QCheckBox("Swap East/West",mainWidget_);

   if(settings.haveKey("TELESCOPE_DEC_INVERSION")) {
      if(QString(settings.getKey("TELESCOPE_DEC_INVERSION"))=="yes")
         decSwap_->setChecked(true);
   }
   if(settings.haveKey("TELESCOPE_RA_INVERSION")) {
      if(QString(settings.getKey("TELESCOPE_RA_INVERSION"))=="yes")
         raSwap_->setChecked(true);
   }

   connect(decSwap_,SIGNAL(toggled(bool)),this,SLOT(swapDec(bool)));
   connect(raSwap_,SIGNAL(toggled(bool)),this,SLOT(swapRa(bool)));

   upButton_->show();
   arrows_->show();

   connect(upButton_,SIGNAL(pressed()),this,SLOT(goUp()));
   connect(upButton_,SIGNAL(released()),this,SLOT(stopUp()));
   connect(downButton_,SIGNAL(pressed()),this,SLOT(goDown()));
   connect(downButton_,SIGNAL(released()),this,SLOT(stopDown()));
   connect(leftButton_,SIGNAL(pressed()),this,SLOT(goLeft()));
   connect(leftButton_,SIGNAL(released()),this,SLOT(stopLeft()));
   connect(rightButton_,SIGNAL(pressed()),this,SLOT(goRight()));
   connect(rightButton_,SIGNAL(released()),this,SLOT(stopRight()));

   mainWidget_->show();

   // disable buttons if fifo
   if(telescopeType()==TELESCOPE_FIFO||telescopeType()==TELESCOPE_FILE) {
      upButton_->setEnabled(false);
      downButton_->setEnabled(false);
      leftButton_->setEnabled(false);
      rightButton_->setEnabled(false);
   }

   // speed slider
   double speed;
   QHGroupBox* speedBox;
   speedBox=new QHGroupBox(QString("Speed"),mainWidget_);
   speedSlider_=new QSlider(1,100,1,100,Qt::Horizontal,speedBox);
   speedValue_=new QLabel(speedBox);
   speed=setSpeed(100/100);
   speedValue_->setText(QString().sprintf("%3i%%",(int)(speed*100)));
   if(speed==0) {
      speedSlider_->setEnabled(false);
      speedValue_->setEnabled(false);
   }

   // read the stored speed
   if(settings.haveKey("TELESCOPE_SPEED")) {
      speed=atof(settings.getKey("TELESCOPE_SPEED"));
      if(speed!=0) {
         speed=setSpeed(speed);
         speedSlider_->setValue(speed*100);
         speedValue_->setText(QString().sprintf("%3i%%",(int)(speed*100)));
      }
   }

   connect(speedSlider_,SIGNAL(valueChanged(int)),this,SLOT(speedChanged(int)));
   speedBox->show();
}

QTelescope::~QTelescope() {
   QCamUtilities::removeWidget(mainWidget_);
}

QWidget * QTelescope::widget() {
   return mainWidget_;
}

void QTelescope::speedChanged(int speed) {
   double newSpeed;
   newSpeed=setSpeed((double)speed/100.0);
   speedValue_->setText(QString().sprintf("%3i%%",(int)(newSpeed*100)));

   //  saving speed
   char value[10];
   sprintf(value,"%1.1f",currentSpeed);
   settings.setKey("TELESCOPE_SPEED",value);
}

void QTelescope::goUp() {
   if(decSwap_) {
      if(decSwap_->isChecked())
         goS();
      else
         goN();
   }
}

void QTelescope::goDown() {
   if(decSwap_) {
      if(decSwap_->isChecked())
         goN();
      else
         goS();
   }
}

void QTelescope::goLeft() {
   if(raSwap_) {
      if(raSwap_->isChecked())
         goW();
      else
         goE();
   }
}

void QTelescope::goRight() {
   if(raSwap_) {
      if(raSwap_->isChecked())
         goE();
      else
         goW();
   }
}

void QTelescope::stopUp() {
   if(decSwap_) {
      if(decSwap_->isChecked())
         stopS();
      else
         stopN();
   }
}

void QTelescope::stopDown() {
   if(decSwap_) {
      if(decSwap_->isChecked())
         stopN();
      else
         stopS();
   }
}

void QTelescope::stopLeft() {
   if(raSwap_) {
      if(raSwap_->isChecked())
         stopW();
      else
         stopE();
   }
}

void QTelescope::stopRight() {
   if(raSwap_) {
      if(raSwap_->isChecked())
         stopE();
      else
         stopW();
   }
}

void QTelescope::swapRa(bool s) {
   if(s) {
      settings.setKey("TELESCOPE_RA_INVERSION","yes");
   } else {
      settings.setKey("TELESCOPE_RA_INVERSION","no");
   }
}

void QTelescope::swapDec(bool s) {
   if(s) {
      settings.setKey("TELESCOPE_DEC_INVERSION","yes");
   } else {
      settings.setKey("TELESCOPE_DEC_INVERSION","no");
   }
}
