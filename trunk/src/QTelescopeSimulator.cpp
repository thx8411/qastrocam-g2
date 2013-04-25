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

#include <Qt/qlabel.h>
#include <Qt/qmessagebox.h>
#include <Qt/qlayout.h>
#include <Qt/qwidget.h>
#include <Qt/qpixmap.h>

#include <Qt3Support/Q3GridLayout>

#include "QTelescopeSimulator.hpp"
#include "QCamUtilities.hpp"

using namespace std;

QTelescopeSimulator::QTelescopeSimulator() : QTelescope() {
   speedLabel_=NULL;
   setSpeed(0.1);
}

QTelescopeSimulator::~QTelescopeSimulator() {
   QCamUtilities::removeWidget(simulatorWidget_);
}

void QTelescopeSimulator::buildGUI(QWidget* parent) {
   QTelescope::buildGUI(parent);

   QPixmap* tmpIcon;

   QString speed_="Speed : ";
   widget()->setCaption("Simulator");

   simulatorWidget_=new QCamVGroupBox("Telescope Simulator",NULL);

   QCamUtilities::registerWidget(simulatorWidget_);
   QCamUtilities::setQastrocamIcon(simulatorWidget_);

   QWidget* simulatorArrows_=new QWidget(simulatorWidget_);
   Q3GridLayout* simulatorArrowsLayout_=new Q3GridLayout(simulatorArrows_,3,3);

   // up label
   buttonUp_=new QLabel(simulatorArrows_);
   tmpIcon=QCamUtilities::getIcon("north.png");
   if(tmpIcon!=NULL) {
      buttonUp_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      buttonUp_->setText("North");
   buttonUp_->setDisabled(true);
   buttonUp_->setAlignment(Qt::AlignHCenter);
   simulatorArrowsLayout_->addWidget(buttonUp_,0,1);

   // down label
   buttonDown_=new QLabel(simulatorArrows_);
   tmpIcon=QCamUtilities::getIcon("south.png");
   if(tmpIcon!=NULL) {
      buttonDown_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      buttonDown_->setText("South");
   buttonDown_->setDisabled(true);
   buttonDown_->setAlignment(Qt::AlignHCenter);
   simulatorArrowsLayout_->addWidget(buttonDown_,2,1);

   // left label
   buttonLeft_=new QLabel(simulatorArrows_);
   tmpIcon=QCamUtilities::getIcon("east.png");
   if(tmpIcon!=NULL) {
      buttonLeft_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      buttonLeft_->setText("East");
   buttonLeft_->setDisabled(true);
   simulatorArrowsLayout_->addWidget(buttonLeft_,1,0);

   // right label
   buttonRight_=new QLabel(simulatorArrows_);
   tmpIcon=QCamUtilities::getIcon("west.png");
   if(tmpIcon!=NULL) {
      buttonRight_->setPixmap(*tmpIcon);
      delete tmpIcon;
   } else
      buttonRight_->setText("West");
   buttonRight_->setDisabled(true);
   simulatorArrowsLayout_->addWidget(buttonRight_,1,2);

   buttonCenter_=new QLabel(simulatorArrows_);
   tmpIcon=QCamUtilities::getIcon("directions.png");
   if(tmpIcon!=NULL) {
      buttonCenter_->setPixmap(*tmpIcon);
      delete tmpIcon;
   }
   simulatorArrowsLayout_->addWidget(buttonCenter_,1,1);

   speedLabel_=new QLabel(simulatorWidget_);
   speedLabel_->setAlignment(Qt::AlignHCenter);
   speed_+=QString().sprintf("%i %%",(int)(currentSpeed*100));
   speedLabel_->setText(speed_);

   simulatorArrows_->show();
   simulatorWidget_->show();
}

void QTelescopeSimulator::goE(float shift) {
   buttonLeft_->setDisabled(false);
}

void QTelescopeSimulator::goW(float shift) {
   buttonRight_->setDisabled(false);
}

void QTelescopeSimulator::goS(float shift) {
   buttonDown_->setDisabled(false);
}

void QTelescopeSimulator::goN(float shift) {
   buttonUp_->setDisabled(false);
}

void QTelescopeSimulator::stopE() {
   buttonLeft_->setDisabled(true);
}

void QTelescopeSimulator::stopN() {
   buttonUp_->setDisabled(true);
}

void QTelescopeSimulator::stopW() {
   buttonRight_->setDisabled(true);
}

void QTelescopeSimulator::stopS() {
   buttonDown_->setDisabled(true);
}

double QTelescopeSimulator::setSpeed(double speed) {
   QString speed_="Speed : ";
   currentSpeed=speed;
   speed_+=QString().sprintf("%i %%",(int)(currentSpeed*100));
   if(speedLabel_)
      speedLabel_->setText(speed_);
   return(speed);
}

bool QTelescopeSimulator::setTracking(bool activated) {
   // always tracking
   return activated;
}

