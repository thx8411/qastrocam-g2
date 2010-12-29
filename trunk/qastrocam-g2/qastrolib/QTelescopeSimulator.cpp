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

#include <qmessagebox.h>
#include <qvgroupbox.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qpixmap.h>

#include "QTelescopeSimulator.moc"

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

   simulatorWidget_=new QVGroupBox("Telescope Simulator",NULL);

   QCamUtilities::registerWidget(simulatorWidget_);
   QCamUtilities::setQastrocamIcon(simulatorWidget_);

   QWidget* simulatorArrows_=new QWidget(simulatorWidget_);
   QGridLayout* simulatorArrowsLayout_=new QGridLayout(simulatorArrows_,3,3);

   buttonUp_=new QLabel(simulatorArrows_,"U");
   tmpIcon=QCamUtilities::getIcon("north.png");
   buttonUp_->setPixmap(*tmpIcon);
   buttonUp_->setDisabled(true);
   buttonUp_->setAlignment(Qt::AlignHCenter);
   delete tmpIcon;
   simulatorArrowsLayout_->addWidget(buttonUp_,0,1);

   buttonDown_=new QLabel(simulatorArrows_,"D");
   tmpIcon=QCamUtilities::getIcon("south.png");
   buttonDown_->setPixmap(*tmpIcon);
   buttonDown_->setDisabled(true);
   buttonDown_->setAlignment(Qt::AlignHCenter);
   delete tmpIcon;
   simulatorArrowsLayout_->addWidget(buttonDown_,2,1);

   buttonLeft_=new QLabel(simulatorArrows_,"L");
   tmpIcon=QCamUtilities::getIcon("east.png");
   buttonLeft_->setPixmap(*tmpIcon);
   buttonLeft_->setDisabled(true);
   delete tmpIcon;
   simulatorArrowsLayout_->addWidget(buttonLeft_,1,0);

   buttonRight_=new QLabel(simulatorArrows_,"R");
   tmpIcon=QCamUtilities::getIcon("west.png");
   buttonRight_->setPixmap(*tmpIcon);
   buttonRight_->setDisabled(true);
   delete tmpIcon;
   simulatorArrowsLayout_->addWidget(buttonRight_,1,2);

   buttonCenter_=new QLabel(simulatorArrows_,"C");
   tmpIcon=QCamUtilities::getIcon("directions.png");
   buttonCenter_->setPixmap(*tmpIcon);
   delete tmpIcon;
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

